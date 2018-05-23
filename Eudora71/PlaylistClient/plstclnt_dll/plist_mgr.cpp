// plist_mgr.cpp -- public interface for the playlist subsystem.
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "string.h"
#include "assert.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"

#ifdef WIN32
#include "io.h"
#else
#include "unistd.h"
#endif

#include "plist_stg.h"
#include "plist_parser.h"
#include "plist_xmlrpc.h"
#include "plist_cache.h"
#include "plist_inet.h"
#include "plist_utils.h"
#include "plist_html.h"
#include "plist_cinfo.h"

#include "plist_debug.h"

#include "../../Version.h"
#include "../../Eudora/resource.h"

#include "EuLang.h"

// our own sorry ass
#include "plist_mgr.h"

//	Disable warnings for:
//	4100: "unreferenced formal parameter"
//	4706: "assignment within conditional expression"
#pragma warning(disable : 4100 4706)

#include "DebugNewHelpers.h"

#ifdef FAST_AD_SCHEDULING
static const time_t kRetryInterval = A_MINUTE;
static const time_t kLowAdRetryInterval = 4 * A_MINUTE;
static const time_t kMinReasonableShowFor = 1;
#else
static const time_t kRetryInterval = 30 * A_MINUTE;
static const time_t kLowAdRetryInterval = 4 * AN_HOUR;
static const time_t kMinReasonableShowFor = 15;
#endif

#ifdef PLIST_LOGGING
plist_callback g_DebugUserProc;
long g_DebugUserRef;
#endif
////////////////////////////////////////////////////////////////////////////////
// update status --- this really needs to be in the db_reference, but for now
// this is an easier hack since nags don't have the db handle.

static time_t g_lastUpdate = 0;
static time_t g_lastFetch = 0;

// They're still hacky since they use globals and such, but they now
// use the definitely un-hacky clientinfo.  It's ok to build here, probably.

void get_last_update_hack( PlistCInfoPtr pcip, time_t* lastUpdate, time_t* lastFetch ) {
	*lastUpdate = pcip->lastUpdateTime;
	*lastFetch = pcip->lastFetchTime;
}

void set_last_update_hack( PlistCInfoPtr pcip, time_t lastUpdate, time_t lastFetch ) {
	PlistCInfo pci;
	
	pci.lastUpdateTime = lastUpdate;
	pci.lastFetchTime = lastFetch;
	PlistCInfoSet(pcip, &pci, PLCIlastFetchTimeValid|PLCIlastUpdateTimeValid);
}

// For remembering tweaks
typedef struct {
	TweakType tweak;
	int serverID;
	int entryID;
} TweakRecord;
#define MAX_TWEAKS_CUZ_IM_LAZY (10)

///////////////////////////////////////////////////////////////////////////////
// private state -- the user totes this around, but doesn't know it!

struct db_reference {
    char* dbName;
    long userRef;
    plist_callback userProc;

	int dbHdl;
	PlaylistRecPtr playlists;
    int curPlaylistID;
    int curServerID;
    int curEntryID;
    Entry curEntry;
	prefetch_queue pfq;
	InetContext* pNetContext;
    time_t curStartTime;
	time_t blankTime;
    time_t timeRemaining;
	time_t lastUpdate;
    int nStatus;
	bool doUpdate;
	bool forceUpdate;
	char* playlistServer;
	PlistCInfoPtr pcip;
	PrivCachePtr pcp;
	TweakRecord tweaks[MAX_TWEAKS_CUZ_IM_LAZY];
	int nTweaks;
	bool rerun;
	FMBHANDLE adFMB;
};

enum _status_ {
    kInitialized,
    kRunning,
    kSuspended,
    kWaitingToShow,
    kBlank,
    kBlankish,
    kScheduleNext
};

#ifdef PLIST_LOGGING
char *g_statusNames[] = {
    "kInitialized",
    "kRunning",
    "kSuspended",
    "kWaitingToShow",
    "kBlank",
    "kBlankish",
    "kScheduleNext"
};
#endif

///////////////////////////////////////////////////////////////////////////////
// sooper dooper looper
// this actually gets used outside this file, but we admit defeat in modularizing
// the code and just declare it extern where we need it.
typedef (*adproc)( void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
int ForEntriesDo( void *dbRef, adproc proc, int startPlaylistID, int startServerID, int startEntryID, void *userRef );


///////////////////////////////////////////////////////////////////////////////
// top secret intelligence

static void schedule_next( db_reference* pRef, bool forceNew );
static get_entry_info( PrivCachePtr pcp, Entry* pe, ENTRY_INFO** pei );
static char* prep_html_source( PrivCachePtr pcp, char* url );
static int ad_end_bookkeeping( db_reference *pRef, int playlistID, int serverID, int entryID, Entry *pe );
static int valid_entry_for_display( EntryRecPtr erp, enum EntryType desiredType, bool today, bool rerun, time_t rerunAge );
static int user_show_entry ( db_reference *pRef, Entry *pE );
static int user_delete_entry ( db_reference *pRef, Entry *pE );
static void schedule_specials( db_reference* pRef );
static void process_tweaks( db_reference* pRef );
static void process_plist_commands( db_reference* pRef );
static void change_clientmode( db_reference *pRef, int clientMode );
static void setup_for_show( db_reference *pRef, PlaylistRecPtr plrp, EntryRecPtr erp, EntryRecPtr erpWas, Entry *entry );
static void schedule_regular( db_reference *pRef );
static void schedule_runout( db_reference *pRef );
static void schedule_rerun( db_reference *pRef );
static int regular_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int runout_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int rerun_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int tweak_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int special_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int flush_entry_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp,
		PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static void dr_k( db_reference* pRef );
static void send_home_the_clones( db_reference* pRef );
static bool low_on_ads( db_reference* pRef );
static void mark_old_entries( db_reference* pRef );
static int mark_old_entries_proc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int dumper_proc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static void flush_erp( PlaylistRecPtr plrp, EntryRecPtr erp );
static bool entry_in_cache( PrivCachePtr pcp, PlaylistRecPtr plrp, EntryRecPtr erp, Entry *newEntry, prefetch_queue* ppfq );
static void get_server_name( db_reference *pRef );

// need-to-know intelligence
void mgr_DayEndBookkeeping( void *dbRef, bool startOver );

////////////////////////////////////////////////////////////////////////////////
// the never-changing default playlist server url

const char* g_defaultPlaylistServer = 
	"//adserver.eudora.com/adjoin/playlists";


///////////////////////////////////////////////////////////////////////////////
// public interfaces -- all MGR routines have standard 'C' linkage.
long MGR_Init( const char* dbName, const char* workingDir, const char *subDir, long userRef, plist_callback pfn, int clientMode, const int adFailure )
{
    long ret = 0;
	db_reference* pRef = DEBUG_NEW_NOTHROW db_reference;

    if ( pRef ) {
        memset( pRef, '\0', sizeof(db_reference) );
        pRef->dbName = DupeString( dbName );
        pRef->userRef = userRef;
        pRef->userProc = pfn;
        pRef->nTweaks = 0;

#ifdef PLIST_LOGGING
		g_DebugUserProc = pfn;
		g_DebugUserRef = userRef;
#endif

		get_server_name( pRef );
		
		SDTrace1("Server: %s",pRef->playlistServer);

        // now start up the user's database
        int fh = DbInit( pRef->dbName, workingDir, subDir );
        if ( fh >= 0 ) pRef->playlists = BuildDBLists( fh );
        
        // make sure subsystems live in subdirectory
        char realWorkingDir[_MAX_PATH+1];;
        strcpy(realWorkingDir,workingDir);strcat(realWorkingDir,subDir);
		pRef->pcip = PlistCInfoNew("CInfo.dat",realWorkingDir,pfn,pRef,adFailure);       
        pRef->pcp = PrivCacheNew("AdCache",realWorkingDir,NULL,0);
        
        // start up facetime
        pRef->adFMB = NewFaceMeasure(pfn);

        if ( fh != -1 && pRef->pcip && pRef->pcp && pRef->adFMB) {
        	change_clientmode( pRef, clientMode );
			pRef->dbHdl = fh;
            pRef->nStatus = kInitialized;
            
            // delete duplicate ads
            send_home_the_clones( pRef );
        	
        	// kill old playlists
        	dr_k( pRef );
        	
        	// mark entries we don't need to fetch images for
        	mark_old_entries( pRef );
        	
        	// resume where we left off, more or less
        	if ( !ReadEntryID( pRef->playlists,
        					   pRef->pcip->lastPlaylistID,
        					   pRef->pcip->lastServerID,
        					   pRef->pcip->lastEntryID,
        					   &pRef->curEntry ) )
        	{
	            pRef->curPlaylistID = pRef->pcip->lastPlaylistID;
	            pRef->curServerID = pRef->pcip->lastServerID;
	            pRef->curEntryID = pRef->pcip->lastEntryID;
	        }
            
			NetStartup();
			
			get_last_update_hack( pRef->pcip, &g_lastUpdate, &g_lastFetch );

#ifdef PLIST_LOGGING
			time_t nextUpdate = g_lastUpdate + pRef->pcip->reqInterval;
			SDTrace1("Next playlist update: %s",S2D(time(0)-nextUpdate,-1));
#endif

			if ( !pRef->playlists ) {
				pRef->doUpdate = true;
			}
			else {
				PlaylistRecPtr plrp;
				for ( plrp=pRef->playlists ; plrp ; plrp=plrp->next )
					QueueUnfetchedEntries( pRef->pcp, &pRef->pfq, plrp );
			}

            ret = (long) pRef;
        }
    }

    return ret;
}


// after init
void MGR_UpdatePlaylists( long dbRef, bool forceUpdate )
{
	if (!dbRef)
		return;

    db_reference* pRef = (db_reference*) dbRef;
	pRef->doUpdate = true;
	pRef->forceUpdate = true;
}

void priv_UpdatePlaylists( db_reference* pRef, bool forceUpdate )
{
    OutputDebugString( "UpdatePlaylists\n" );

 	get_server_name( pRef );
 	
	ClientUpdateParams* cup = DEBUG_NEW ClientUpdateParams;
    strcpy( cup->userAgent, "Eudora/" EUDORA_BUILD_VERSION " (Win)" );
    strcpy( cup->playlistID, "bogus_playlist_identifier" );
	LNG_GetLanguageInfo( LANG_INFO_RFC1766, cup->langID );
	cup->faceTime = cup->faceTimeLeft = 0;
	cup->pcip = pRef->pcip;
	cup->dbHdl = pRef->dbHdl;
	cup->dbRef = pRef;
	cup->profile = GetIniStringNotStatic( pRef->userProc, IDS_INI_PROFILE );
	if (cup->profile && !*cup->profile)
	{
		delete [] cup->profile;
		cup->profile = NULL;
	}
	cup->distributor = GetIniStringNotStatic( pRef->userProc, IDS_INI_DISTRIBUTOR );
	if (cup->distributor && !*cup->distributor)
	{
		delete [] cup->distributor;
		cup->distributor = NULL;
	}
#ifdef _DEBUG
	cup->ignoreCurrent = forceUpdate;
#endif

	pRef->lastUpdate = time(0);

    assert( !pRef->pNetContext );
    pRef->pNetContext = DoClientUpdate( pRef->playlistServer, cup );
        
    // now's a good time to sweep the cache
    PrivCacheSweep(pRef->pcp,NULL,0);
}

// after init
void MGR_BeginScheduling( long dbRef )
{
	if (!dbRef)
		return;

    db_reference* pRef = (db_reference*) dbRef;
    assert( pRef->nStatus == kInitialized );

    schedule_next ( pRef, true );
    schedule_specials ( pRef );
}


//
// Cancel can be called only after "BeginScheduling"
// has been called.
//

void MGR_Cancel( long dbRef )
{
	if (!dbRef)
		return;

    db_reference* pRef = (db_reference*) dbRef;
    schedule_next( pRef, true );
}


// any time after init
void MGR_Shutdown( long dbRef )
{
	if (!dbRef)
		return;

    db_reference* pRef = (db_reference*) dbRef;
	mgr_DayEndBookkeeping( pRef, false );	// record stuff for ad being shown if runout
	set_last_update_hack( pRef->pcip, g_lastUpdate, g_lastFetch );
    PrivCacheDispose( pRef->pcp );
    PlistCInfoDispose( pRef->pcip );
	DbShutdown( pRef->dbHdl );
    NetShutdown();
    DisposeDBLists( pRef->playlists );
    DisposeFaceMeasure( pRef->userProc, pRef->adFMB );
	delete [] pRef->dbName;
	delete [] pRef->playlistServer;
    delete pRef;

// TODO: multiple playlist support
// TODO: persistent scheduling info
}

// any time after init, regardless of suspended states, etc. if -1 is
// returned, "MGR_GetLastError" can be called to see whutz'up.
void MGR_Idle( long dbRef, bool bCanSwitchAd, bool bConnected, int clientMode )
{
	if (!dbRef)
		return;

    db_reference* pRef = (db_reference*) dbRef;
    int nStatus = pRef->nStatus;
	InetContext* netContext = pRef->pNetContext;

	// Make sure we're in the right mode, unless
	// we're in the middle of a download, in which case
	// let's wait for that to complete, so that we can
	// stir our brains with a stick and not worry about
	// cleaning up the mess
	if ( !netContext ) change_clientmode( pRef, clientMode );
	
	// Let clientinfo do its thing
	PlistCInfoIdle(pRef->pcip);

	// background processing of playlist requests, and fetching of playlist
	// entries continues regardless of "nStatus."
	if ( netContext ) {
		if ( netContext->status == kNetComplete ) {
			SDTrace1("Idle: Net Op '%c%c%c%c' succeeded.",SDCharify(netContext->kind));
			if ( netContext->kind == 'updt' ) {
				// process any client commands (e.g. "flush")
				process_plist_commands( pRef );

				// add playlist to database
				int hplst = ((ClientUpdateContext*)netContext)->m_hplst;
				if ( hplst >= 0 ) {
				
					// add to our database
					DbAddPlaylist( pRef->dbHdl, hplst );
					bool wasFirst = !pRef->playlists;
					
					// No longer need it to be open
					ClosePlaylist( hplst );
					
					// rebuild indices
					DisposeDBLists( pRef->playlists );
					pRef->playlists = BuildDBLists( pRef->dbHdl );
					
					// queue unfetched entries
					QueueUnfetchedEntries( pRef->pcp, &pRef->pfq, pRef->playlists );

					// Boo: is this the first one? if so, we set it as current and
					// (re)begin scheduling.
					if ( !wasFirst ) {
						pRef->nStatus = kInitialized;
						MGR_BeginScheduling( (long)pRef );
					}
					
					// make sure the app knows about any toolbar ads, etc
					schedule_specials( pRef );
				}
				
				// Reset fetch time on "success"
				if ( hplst >=0 ||	// we got a playlist
					 hplst == -2 ) 	// we got valid response but empty playlist
				{
					pRef->lastUpdate = g_lastUpdate = time(0);
					set_last_update_hack( pRef->pcip, g_lastUpdate, g_lastFetch );
				}
			}
			else if ( netContext->kind == 'ftch' )
			{
				// if all done fetching images, do another
				// round of toolbar/sponsor scheduling, just
				// for grins
				if ( !pRef->pfq ) schedule_specials( pRef );
			}
			delete netContext;
			pRef->pNetContext = 0;
		}
		else if ( netContext->status == kNetError ) {
			// TODO: update dead-beat loser log
			SDTrace1("Idle: Net Op '%c%c%c%c' failed.",SDCharify(netContext->kind));

			// if we failed a client update, retry after "kRetryInterval"
			if ( netContext->kind == 'updt' )
				pRef->doUpdate = true;

			delete netContext;
			pRef->pNetContext = 0;
		}
	}
	else if ( bConnected ) {
		if ( pRef->pfq ) {
			pRef->pNetContext = FetchCachedEntry( pRef->pcp, &pRef->pfq );
		}
		else if ( pRef->forceUpdate || ((time(0) - pRef->lastUpdate) > kRetryInterval) ) {
			if ( pRef->doUpdate ) {
				SDTrace2("IDLE: Retrying playlist after %d mins (%d)",pRef->lastUpdate ? (time(0) - pRef->lastUpdate)/60 : 0,pRef->lastUpdate);
				priv_UpdatePlaylists( pRef, pRef->forceUpdate );
				pRef->doUpdate = pRef->forceUpdate = false;
			}
			else {
				assert(!pRef->forceUpdate);

				// Request an update if we don't have a playlist or
				// if the request interval has passed or
				// if we're low on ads and the low ad interval has passed
				pRef->doUpdate = ( !pRef->playlists ) ||
								 ( time(0) - g_lastUpdate >= pRef->pcip->reqInterval ) ||
								 ( time(0) - g_lastUpdate >= kLowAdRetryInterval && low_on_ads( pRef ) );
			}
		}
	}

#ifdef PLIST_LOGGING
	if ( !pRef->playlists )
		SDTrace("MGR_Idle: No Playlist!");
#endif
	
	// This seems like a handy spot to mark that user has been bad
	if ( pRef->nTweaks ) process_tweaks( pRef );
		
    if ( nStatus == kSuspended || nStatus == kInitialized ) {
        return;
    }
    
    // now do whatever scheduling tasks need doing
	// SDTrace1( "MGR_Idle: nStatus %s", nStatus>kBlankish ? "out of range" : g_statusNames[nStatus] );

    if ( nStatus == kWaitingToShow ) {
		if ( pRef->blankTime ) {
			pRef->nStatus = kBlank;
			FaceMeasureReset( pRef->userProc, pRef->adFMB );
			FaceMeasureBegin( pRef->userProc, pRef->adFMB );
		}
		else {
			if ( *(pRef->curEntry.src) && GetCachedEntryPath( pRef->pcp,
						pRef->curEntry.src, pRef->curEntry.checksum, NULL ) ) {

				if (bCanSwitchAd)
				{
					user_show_entry ( pRef, &pRef->curEntry );
					pRef->nStatus = kRunning;
				}
			}
			else
			{
				// Well, we do get here.  One case is if you don't have a playlist
				// and so have spawned a background thread to get it, but it hasn't
				// returned yet.  We'll just stay in the kWaitingToShow state so
				// that eventually when the playlist is retrieved it will attempt
				// to show the ad.

				schedule_next( pRef, true );
			}
		}
    }
    else if ( nStatus == kBlank ) {
		if (bCanSwitchAd)
		{
			pRef->userProc( PLIST_BLANK, pRef->userRef, NULL );
			pRef->nStatus = kBlankish;
		}
    }
    else if ( nStatus == kBlankish ) {
		if ( pRef->blankTime != -1 ) {
			time_t face;
			FaceMeasureReport( pRef->userProc, pRef->adFMB, &face, NULL, NULL, NULL );
			if ( face > pRef->blankTime ) {
				pRef->blankTime = 0;
				pRef->nStatus = kWaitingToShow;
			}
		}
    }
    else {
        schedule_next( pRef, nStatus==kScheduleNext );
    }
// TODO: what routines are OK to call during "userProc"? Any? user should
// be able to call at least the suspend, resume, and shutdown routines.
}


// get rid of an ENTRY_INFO block passed in PLIST_SHOWTHIS
void MGR_DisposeEntryInfo( ENTRY_INFO* pei )
{
	// "prep_html_source" temp files must be disposed of
	if ( pei->src && (pei->mime_type != NULL) && (strcmp( "text/html", pei->mime_type ) == 0) )
		unlink( pei->src );

	delete [] pei->mime_type;
	delete [] pei->id;
	delete [] pei->title;
	if ( pei->src ) delete [] pei->src;
	delete [] pei->href;

	delete pei;
}

// Is it safe?
bool MGR_GetClientUpdateStatus( long dbRef, time_t* lastUpdate )
{
	if (!dbRef)
		return false;

	db_reference *pRef = (db_reference *) dbRef;

	*lastUpdate = time(0) - pRef->pcip->adFailure * A_DAY;
	return pRef->pcip->adsFailing || pRef->pcip->adFailure>=kAdFailureGraceDays;
}

// Let the ads know that mail is being checked
void MGR_CheckingMail( long dbRef )
{
	if (!dbRef)
		return;

	db_reference *pRef = (db_reference *) dbRef;

	// make sure clientinfo knows we did this
	PlistCInfo pci;
	pci.checkedMail = true;
	PlistCInfoSet( pRef->pcip, &pci, PLCIcheckedMailValid );
	
	// TODO: if we're low on ads and it's been a while since
	// our last check, fetch a playlist
}

// Dump the state of the system to the log, just for grins
struct dumpparms
{
	CString output;
	int oldPLID;
	int oldStyle;
};

void MGR_DumpState( long dbRef )
{
	if (!dbRef)
		return;

	db_reference *pRef = (db_reference *) dbRef;
	dumpparms parms;

	if ( pRef->userProc( PLIST_MONITOR, pRef->userRef, NULL ) ) return;
	
	parms.oldPLID = 0;
	parms.oldStyle = 0;
	parms.output.Empty();
		
	parms.output = "<HTML><pre>\n";
	ForEntriesDo( pRef, dumper_proc, 0, 0, 0, (void*)&parms );
	if ( parms.oldStyle ) parms.output += "</font>\n\n";

	char buffer[256];
	time_t nextUpdate = g_lastUpdate + pRef->pcip->reqInterval;
	sprintf(buffer, "Last Update: %s",ctime(&nextUpdate)); parms.output += buffer;
	nextUpdate = pRef->lastUpdate + kRetryInterval;
	sprintf(buffer, "May Update:  %s",ctime(&nextUpdate)); parms.output += buffer;
	sprintf(buffer, "Will Update: %c\n",pRef->doUpdate ? 'Y' : 'N' ); parms.output += buffer;
	PlistCInfoDump( pRef->pcip, parms.output );

	parms.output += "</pre></HTML>";
	
	pRef->userProc( PLIST_MONITOR, pRef->userRef, (long)(LPCTSTR)parms.output );
}

static int dumper_proc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	char buffer[256];
	dumpparms *pParms = (dumpparms*)userRef;
	db_reference *pRef = (db_reference *) dbRef;
	Entry entry;
	char finger[16];
	char status;
	
	bool isRegular = erp->type==kAd || erp->type==kRunout;
	bool isOut = erp->shownFor >= erp->showForMax;
	bool isDateBad = erp->endTime && time(0) > erp->endTime || erp->startTime && time(0) < erp->startTime;
	char tipe;
	
	memset( (void*) &entry, '\0', sizeof( entry ) );
	ReadEntry( plrp, erp, &entry );
	
	// output playlist id if it has changed
	if ( atoi( plrp->plID ) != pParms->oldPLID )
	{
		if ( pParms->oldStyle )
		{
			pParms->output += "</font>";
			pParms->oldStyle = 0;
		}
		pParms->oldPLID = atoi( plrp->plID );
		pParms->output += "Active?  Image?    Total -------------- ";
		sprintf( buffer, "Playlist %s\n", plrp->plID );
		pParms->output += buffer;
		pParms->output += "  |  ID    | Today   |   Age Title\n";
	}
	
	bool isTheOne = false;
	// Is this the current ad?
	if ( atoi(plrp->plID)==pRef->curPlaylistID && erp->serverID==pRef->curServerID && erp->entryID==pRef->curEntryID )
	{
		if ( pRef->rerun )
			strcpy( finger, "R>");
		else
		{
			strcpy( finger, "->");
			isTheOne = true;
		}
	}
	else
		strcpy( finger, "  " );
	
	// What's its status?
	if ( (isRegular && isOut) || isDateBad )
	{
		if (erp->type==kAd && !isDateBad && time(0) - erp->lastTime < pRef->pcip->rerunInterval )
			status = 'R';
		else
			status = 'N';
	}
	else if ( erp->type==kButton && erp->userDeleted )
		status = 'N';
	else if ( erp->type==kDeadEntry )
		status = 'X';
	else
		status = 'Y';
	
	// Type?
	switch( erp->type )
	{
		case kAd: tipe = ' '; break;
		case kRunout: tipe = 'R'; break;
		case kSponsor: tipe = 'C'; break;
		case kButton: tipe = 'B'; break;
		case kDeadEntry: tipe = 'X'; break;
		default: tipe = '?'; break;
	}
	
	// Here's the output...
	sprintf(buffer,
"%s%c %1d.%03d%c %c %2d/%-2d %2d/%-2d %3s" " \"%s\"\n",
	finger,
	status,
	erp->serverID, erp->entryID,
	tipe,
	GetCachedEntryPath( pRef->pcp, entry.src, NULL, NULL ) ? 'Y' : 'N',
	erp->shown, erp->dayMax,
	erp->showFor ? erp->shownFor/erp->showFor:0, erp->showFor ? erp->showForMax/erp->showFor:0,
	S2D( time(0)-erp->lastTime, 1 ),
	
	entry.title,
	0);
	
	if ( isTheOne )
	{
		if ( pParms->oldStyle ) pParms->output += "</font>";
		pParms->output += "<font color=\"#FF0000\">";
		pParms->oldStyle = 1;
	}
	else if ( status == 'R' )
	{
		if ( pParms->oldStyle != 2 )
		{
			if (pParms->oldStyle) pParms->output += "</font>";
			pParms->output += "<font color=\"#0000AA\">";
			pParms->oldStyle = 2;
		}
	}
	else if ( status == 'N' )
	{
		if ( pParms->oldStyle != 3 )
		{
			if (pParms->oldStyle) pParms->output += "</font>";
			pParms->output += "<font color=\"#AAAAAA\">";
			pParms->oldStyle = 3;
		}
	}
	else
	{
		if ( pParms->oldStyle ) pParms->output += "</font>";
		pParms->oldStyle = 0;
	}
	
	pParms->output += buffer;
	
	return 1;
}
	
	
// Let the application notify us that the user has deleted something
// We do minimal processing here, and munge the playlist at idle
void MGR_UserTweaked( long dbRef, const char *entryID, TweakType tweak )
{
	if (!dbRef)
		return;

	db_reference *pRef = (db_reference *)dbRef;

	if ( !pRef ) return;

	assert( pRef->nTweaks < MAX_TWEAKS_CUZ_IM_LAZY );
	if ( pRef->nTweaks == MAX_TWEAKS_CUZ_IM_LAZY ) return;
	
	pRef->tweaks[pRef->nTweaks].tweak = tweak;
	pRef->tweaks[pRef->nTweaks].serverID = atoi( entryID );
	char *period = strchr( entryID, '.' );
	pRef->tweaks[pRef->nTweaks].entryID = period ? atoi( period+1 ) : 0;
	
	pRef->nTweaks++;
}

//////////////////////////////////////////////////////////////////////
// Get the name of the server we're supposed to use
static void get_server_name( db_reference *pRef )
{
	if ( pRef->playlistServer ) delete [] pRef->playlistServer;
	
	// KEEP_TOGETHER
	char* evar = 0;
	char* svName = GetIniStringNotStatic( pRef->userProc, IDS_INI_DEBUG_PLAYLIST_SERVER );
	if ( svName && *svName )
		pRef->playlistServer = DupeString( svName );
#if defined( DEBUG_ENV )
	else if ( evar = getenv( "playlist_server" ) )
		pRef->playlistServer = DupeString( evar );
#endif
	else
		pRef->playlistServer = DupeString( g_defaultPlaylistServer );
	delete [] svName;
	// END KEEP_TOGETHER
}

//////////////////////////////////////////////////////////////////////
// delete old copies of ads
typedef struct
{
	PlaylistRecPtr plrp;
	EntryRecPtr erp;
	int n;
} EntryArrayElement;
static int count_entries_proc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
static int fill_entry_array_proc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
int entry_compare( const void *arg1, const void *arg2 );

static void send_home_the_clones( db_reference* pRef )
{
	int n=0;
	
	// count all the entries
	ForEntriesDo( pRef, count_entries_proc, 0, 0, 0, (void*)&n );

	if ( n > 1 )
	{
		// allocate an array for all of the entries
		EntryArrayElement *entries = DEBUG_NEW_NOTHROW EntryArrayElement[ n ] ;
		if ( entries )
		{
			// copy values into our array
			fill_entry_array_proc( NULL, NULL, NULL, NULL, NULL, NULL );
			ForEntriesDo( pRef, fill_entry_array_proc, 0, 0, 0, (void*)entries);
			
			// sort the array
			qsort ( (void*)entries, (size_t)n, sizeof( EntryArrayElement ), entry_compare );
			
			// kill the earlier of any two identical entries
			while ( --n )
			{
				if ( entries[n].erp->serverID==entries[n-1].erp->serverID )
				if ( entries[n].erp->entryID==entries[n-1].erp->entryID )
					flush_erp ( entries[n-1].plrp, entries[n-1].erp );
			}
			
		}
		delete [] entries;
	}
}

static int count_entries_proc(void* /*dbRef*/, PlaylistRecPtr /*plrp*/, EntryRecPtr /*erp*/, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	++*(int*)userRef;	// count it
	return 1;			// and move on
}

static int fill_entry_array_proc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	static int n;
	
	if ( !dbRef ) n = 0;
	else
	{
		EntryArrayElement *eElement = (EntryArrayElement*) userRef;
		eElement[n].plrp = plrp;
		eElement[n].erp = erp;
		eElement[n].n = n;
		n++;
	}
	return 1;	// keep going
}
	
int entry_compare( const void *arg1, const void *arg2 )
{
	EntryArrayElement *eElement1 = (EntryArrayElement*) arg1;
	EntryArrayElement *eElement2 = (EntryArrayElement*) arg2;
	int sDiff = eElement1->erp->serverID - eElement2->erp->serverID;
	if ( sDiff ) return sDiff;
	int eDiff = eElement1->erp->entryID - eElement2->erp->entryID;
	if ( eDiff ) return eDiff;
	return eElement1->n - eElement2->n;
}

//////////////////////////////////////////////////////////////////////
// delete any playlists that have no entries or only old entries in them
static void dr_k( db_reference* pRef )
{
	PlaylistRecPtr plrp;
	EntryRecPtr erp;
	time_t cutoff;
	bool killedOne = false;
	
	// the cutoff time is now minus our historylength
	cutoff = time(0)-pRef->pcip->historyLength;

	// loop through the playlists
	for( plrp = pRef->playlists; plrp ; plrp = plrp->next )
	{
		// loop through any entries in the playlist
		bool geezer = true;
		for ( erp = plrp->entries; erp ; erp = erp->next )
			if ( erp->type == kDeadEntry )
			{
				SDTrace3( "Entry %s.%d.%d already has passed on",plrp->plID,erp->serverID,erp->entryID );
			}
			else if ( erp->lastTime > cutoff )
			{
				// we found one that wasn't too old
				SDTrace4( "Playlist %s not geezer, entry %d.%d %s newer than cutoff",plrp->plID,erp->serverID,erp->entryID, S2D( erp->lastTime - cutoff, -1 ) );
				geezer = false;
				break;
			}
			else if ( !erp->lastTime && (!erp->startTime || time(0)>erp->startTime) &&
					  valid_entry_for_display( erp, erp->type, false, false, 0 ) )
			{
				// we found one that wasn't too old
				SDTrace3( "Playlist %s not geezer, entry %d.%d never shown but still good",plrp->plID,erp->serverID,erp->entryID );
				geezer = false;
				break;
			}
			else
				SDTrace4( "Entry %s.%d.%d geezer by %s",plrp->plID,erp->serverID,erp->entryID, S2D( cutoff-erp->lastTime, -1 ) );
		
		// if we didn't find any entries younger than historylength, kill the playlist
		if( geezer )
		{
			SDTrace1( "Dr K strikes - Playlist %s is toast", plrp->plID );
			int hpl = OpenPlaylist( plrp->plID, true );
			if ( hpl >= 0 )
			{
				DbRemovePlaylist( pRef->dbHdl, hpl );
				ClosePlaylist( hpl );
				killedOne = true;
			}
		}
	}
	
	// if we killed any, remake the indices
	if ( killedOne )
	{
		DisposeDBLists( pRef->playlists );
		pRef->playlists = BuildDBLists( pRef->dbHdl );
	}
	
	// ta da
}


//////////////////////////////////////////////////////////////////////
// determine if we're low on ads

static int low_on_ads_proc(void* /*dbRef*/, PlaylistRecPtr /*plrp*/, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	time_t *pTimeLeft = (time_t*)userRef;
	
	if ( (!erp->endTime || time(0) < erp->endTime) &&
		 erp->type == kAd &&
		 erp->showFor < erp->showForMax )
		*pTimeLeft += erp->showForMax - erp->shownFor;
	
	return 1;	// must keep going!
}

static bool low_on_ads( db_reference* pRef )
{
	static time_t last_check;
	static bool am_low;
	
	if (time(0)-last_check < 15*A_MINUTE) return am_low;
	
	last_check = time(0);
	time_t time_left = 0;
	
	// count up remaining facetime, sloppily
	ForEntriesDo( pRef, low_on_ads_proc, 0, 0, 0, (void*)&time_left );
	
	// if it's less than half a quota, ask for more soon!
	SDTrace3( "low_on_ads?  %d (%d < %d/2)",time_left < pRef->pcip->faceTimeQuota/2, time_left/60, pRef->pcip->faceTimeQuota/60 );
	am_low = time_left < pRef->pcip->faceTimeQuota/2;
	
	return am_low;
}

//////////////////////////////////////////////////////////////////////
// mark very old entries so we don't try to cache them

static int mark_old_entries_proc(void* /*dbRef*/, PlaylistRecPtr /*plrp*/, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	time_t old = (long)userRef;
	time_t now = time(0);
	
	// if endtime passed, we're definitely crufty
	if ( erp->endTime && erp->endTime < now )
		erp->oldNCrufty = 1;

	// if start time hasn't come, we're not crufty
	else if ( now < erp->startTime )
		;

	// if there is time left, we're not crufty
	else if ( ( erp->type==kAd || erp->type==kRunout ) && erp->shownFor < erp->showForMax )
		;

	// ok, at this point all we're worried about is if we're young enough to rerun
	else if ( erp->lastTime < old )
		erp->oldNCrufty = 1;
	
	return 1;	// must keep going!
}

static void mark_old_entries( db_reference* pRef )
{
	time_t old = time(0)-pRef->pcip->rerunInterval;
	
	// go do it
	ForEntriesDo( pRef, mark_old_entries_proc, 0, 0, 0, (void*)old );
}

//////////////////////////////////////////////////////////////////////
// schedule_next -- the new way
void schedule_next( db_reference* pRef, bool forceNew )
{
	// should old ad continue to run?
	time_t face;
	if ( !forceNew && pRef->curPlaylistID )
	{
		FaceMeasureReport( pRef->userProc, pRef->adFMB, &face, NULL, NULL, NULL );
		if ( pRef->curEntry.type==kRunout ) face += pRef->curEntry.shownToday;
		if ( face < pRef->curEntry.showFor ) return;
	}
	
	SDTrace( "++++++++++++schedule_next++++++++++++" );
	
	// do bookkeeping on old ad, if need be
	if ( pRef->curPlaylistID )
		ad_end_bookkeeping( pRef, pRef->curPlaylistID, pRef->curServerID, pRef->curEntryID, &pRef->curEntry );
	
	// TODO: block processing
	
	pRef->rerun = false;
	
	// are we within quota?
	if ( pRef->pcip->adFaceTimeUsedToday < pRef->pcip->faceTimeQuota )
		schedule_regular( pRef );
	
	// nope, runout or rerun
	else
		schedule_runout( pRef );
	
	SDTrace( "------------schedule_next------------" );
}

//////////////////////////////////////////////////////////////////////
// ForEntriesDo - call a function on every ad in our database
// returns 0 if the called function ever returns 0, returns -2 if makes full circuit of ads
// called function takes pRef, plrp and erp, plrpWas and erpWas, returns 1 to continue, 0 to exit
int ForEntriesDo( void *dbRef, adproc proc, int startPlaylistID, int startServerID, int startEntryID, void *userRef )
{
	db_reference *pRef = (db_reference *)dbRef;
	
	// find our current spot
	PlaylistRecPtr plrpWas = FindPlaylistInt( pRef->playlists, startPlaylistID );
	EntryRecPtr erpWas = plrpWas ? FindEntry( plrpWas, startServerID, startEntryID ) : NULL;
	if ( !erpWas ) plrpWas = NULL;
	
	// point at the prior ad
	PlaylistRecPtr plrp = plrpWas ? plrpWas : pRef->playlists;
	if ( !plrp ) return -2;	// no playlists!?
	EntryRecPtr erp = erpWas;
	
	// loop through them all
	do
	{		
		//////////////////////////////
		// find the next ad to examine
		
		// advance to next entry
		if ( erp ) erp = erp->next;
		else erp = plrp->entries;
		
		// done with this playlist?
		if ( !erp )
		{
			plrp = plrp->next;
			if ( !plrp )
			{
				// we've come to the end of the list of playlists
				if ( !plrpWas ) return -2;			// we started at the top, we finish at the top
				else plrp = pRef->playlists;	// wrap to first playlist
			}
			continue;	// with next playlist
		}
		
		// are we done?
		if ( erp==erpWas ) return -2;	// came to the end
		
		// call the user's function during the test
	}
	while ( !erp || (*proc)( pRef, plrp, erp, plrpWas, erpWas, userRef ) );
	
	return 0;
}


//////////////////////////////////////////////////////////////////////
// little stub routines that just call the ad looper
// with adprocs that do the work

void schedule_regular( db_reference *pRef )
{
	// First, we try to schedule from where we are
	if ( ForEntriesDo( pRef, regular_adproc, pRef->curPlaylistID, pRef->curServerID, pRef->curEntryID, NULL ) )
	// Now we make a second pass, taking it from the top
	// This will allow the same ad to be shown again just in
	// Case it's the only ad we have that fits the regular stage
	if ( ForEntriesDo( pRef, regular_adproc, 0, 0, 0, NULL ) )
		// failed--go to runout
		schedule_runout( pRef );
}

void schedule_runout( db_reference *pRef )
{
	SDTrace( "Going to runout!" );
	
	// Make sure clientinfo understands ads ain't happening
	PlistCInfoAdEnd( pRef->pcip );
	
	if ( ForEntriesDo( pRef, runout_adproc, pRef->curPlaylistID, pRef->curServerID, pRef->curEntryID, NULL ) )
	// Now we make a second pass, taking it from the top
	// This will allow the same ad to be shown again just in
	// Case it's the only ad we have that fits the runout stage
	if ( ForEntriesDo( pRef, runout_adproc, 0, 0, 0, NULL ) )
		// failed--go to rerun
		schedule_rerun( pRef );
}

void schedule_rerun( db_reference *pRef )
{
	SDTrace( "Going to rerun!" );
	pRef->rerun = true;
	if ( ForEntriesDo( pRef, rerun_adproc, pRef->curPlaylistID, pRef->curServerID, pRef->curEntryID, NULL ) )
	// Now we make a second pass, taking it from the top
	// This will allow the same ad to be shown again just in
	// Case it's the only ad we have that fits the rerun stage
	if ( ForEntriesDo( pRef, rerun_adproc, 0, 0, 0, NULL ) )
	{
		// if we get here, we have failed
		SDTrace( "Nothing to show!" );
		
		// go blank for a couple of seconds, then we'll retry
		pRef->blankTime = 2;
		pRef->nStatus = kWaitingToShow;
		
		// clear out the old ad
		memset( &pRef->curEntry, '\0', sizeof(Entry) );
		pRef->curPlaylistID = pRef->curServerID = pRef->curEntryID = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// adproc for regular ads
static int regular_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr erpWas, void* /*userRef*/ )
{
	db_reference *pRef = (db_reference *)dbRef;

	Entry newEntry;
	
	// is it ok in general?
	if ( valid_entry_for_display( erp, kAd, true, false, 0 ) )
	
	// is it in the cache with ok checksum?
	if ( entry_in_cache( pRef->pcp, plrp, erp, &newEntry, &pRef->pfq ) )
	{
		// we found one!
		setup_for_show( pRef, plrp, erp, erpWas, &newEntry );
		return( 0 );
	}
	
	return 1;	// must keep going!
}

//////////////////////////////////////////////////////////////////////
// read an entry, find out if it's ok, get it or flush it
static bool entry_in_cache( PrivCachePtr pcp, PlaylistRecPtr plrp, EntryRecPtr erp, Entry *newEntry, prefetch_queue* ppfq )
{	
	// read it in
	if ( !ReadEntry( plrp, erp, newEntry ) )
	
	// have we fetched it?
	if ( GetCachedEntryPathCarefully( pcp, newEntry->src, newEntry->checksum, NULL, &erp->chkSumFailures ) )
	{		
		return true;
	}
	else if ( erp->chkSumFailures > 1 )
	{
		SDTrace3( "Killing %d.%d after %d checksum failures", erp->serverID, erp->entryID, erp->chkSumFailures );
		flush_erp( plrp, erp );
	}
	else
		QueueUnfetchedEntry( ppfq, newEntry );
	return false;
}

//////////////////////////////////////////////////////////////////////
// adproc for runouts
static int runout_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr erpWas, void* /*userRef*/ )
{
	db_reference *pRef = (db_reference *)dbRef;

	Entry newEntry;
	
	// is it ok in general?
	if ( valid_entry_for_display( erp, kRunout, true, false, 0 ) )
	
	// is it in the cache with ok checksum?
	if ( entry_in_cache( pRef->pcp, plrp, erp, &newEntry, &pRef->pfq ) )
	{
		// we found one!
		setup_for_show( pRef, plrp, erp, erpWas, &newEntry );
		return( 0 );
	}
	
	return 1;	// must keep going!
}

//////////////////////////////////////////////////////////////////////
// adproc for reruns
static int rerun_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr erpWas, void* /*userRef*/ )
{
	db_reference *pRef = (db_reference *)dbRef;
	Entry newEntry;
	
	// is it ok in general?
	if ( valid_entry_for_display( erp, kAd, false, true, pRef->pcip->rerunInterval ) )
	
	// is it in the cache with ok checksum?
	if ( entry_in_cache( pRef->pcp, plrp, erp, &newEntry, &pRef->pfq ) )
	{
		// we found one!
		setup_for_show( pRef, plrp, erp, erpWas, &newEntry );
		return( 0 );
	}		
	
	return 1;	// must keep going!
}

//////////////////////////////////////////////////////////////////////
// do setup stuff so that Idle can show the ad
void setup_for_show( db_reference *pRef, PlaylistRecPtr plrp, EntryRecPtr erp, EntryRecPtr erpWas, Entry *entry )
{
	pRef->curPlaylistID = atoi( plrp->plID );
	pRef->curServerID = erp->serverID;
	pRef->curEntryID = erp->entryID;
	pRef->curEntry = *entry;
	time( &pRef->curStartTime );
	if (!pRef->rerun) pRef->curEntry.lastTime = pRef->curStartTime;
	pRef->blankTime = erpWas ? max( erpWas->blackAfter, erp->blackBefore ) : erp->blackBefore;
	pRef->nStatus = kWaitingToShow;
#ifdef PLIST_LOGGING
	SDTrace3("Next Up: %d.%d %s",erp->serverID,erp->entryID,entry->title);
#endif
}


//////////////////////////////////////////////////////////////////////
// schedule_specials -- schedule special ads
static void schedule_specials( db_reference* pRef )
{
	// bad not to have any state!
	if (!pRef) return;
	
	SDTrace( "++++++++++++schedule_specials++++++++++++" );
	
	// use our handy-dandy looper!
	ForEntriesDo( pRef, special_adproc, 0, 0, 0, 0 );
	
	SDTrace( "------------schedule_specials-----------" );
}

static int special_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void* /*userRef*/ )
{
	db_reference *pRef = (db_reference *)dbRef;
	Entry newEntry;
	
	if ( erp->type == kButton || erp->type == kSponsor )
	{
		if ( valid_entry_for_display( erp, erp->type, true, false, 0 ) )
		{
			// is it in the cache with ok checksum?
			if ( entry_in_cache( pRef->pcp, plrp, erp, &newEntry, &pRef->pfq ) )
			{		
				// we found one!
				user_show_entry( pRef, &newEntry );
				
				// do a little accounting...
				newEntry.lastTime = time(0);
				WriteEntry( plrp, erp, &newEntry );
				EntryRecEntryCpy( erp, &newEntry );
			}
		}
		else
		{
			if ( 0 <= ReadEntry( plrp, erp, &newEntry ) )
				user_delete_entry( pRef, &newEntry );
		}
	}

	return 1;
}


//////////////////////////////////////////////////////////////////////
// schedule_specials -- schedule special ads
static int user_show_entry ( db_reference *pRef, Entry *pE )
{
	ENTRY_INFO* entryInfo = 0;
	get_entry_info( pRef->pcp, pE, &entryInfo );

	// do we have the file?
	char *fileURL;
	if ( entryInfo && GetCachedEntryURL( pRef->pcp, pE->src, &fileURL ) )
	{
		// swap in the file URL
		delete [] entryInfo->src;
		entryInfo->src = fileURL;
	
		// tell the app to show it
		SDTrace5("Showing %s %d/%d %d/%d\n",pE->entryID,pE->shown,pE->dayMax,pE->shownFor,pE->showForMax);
		pRef->userProc( PLIST_SHOWTHIS, pRef->userRef, (long)entryInfo );
		if (pE->type==kAd && !pRef->rerun) PlistCInfoAdStart(pRef->pcip);
		FaceMeasureReset(pRef->userProc,pRef->adFMB);
		FaceMeasureBegin(pRef->userProc,pRef->adFMB);
		MGR_DumpState( (long) pRef );
	}
	else
	{
		delete entryInfo;
		return -1;
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Tell the app to stop showing a sponsor or button ad
static int user_delete_entry ( db_reference *pRef, Entry *pE )
{
	ENTRY_INFO* entryInfo = 0;
	get_entry_info( pRef->pcp, pE, &entryInfo );

	SDTrace1( "Deleting %s", pE->entryID );
	pRef->userProc( PLIST_DELETE, pRef->userRef, (long)entryInfo );

	return 0;
}


//////////////////////////////////////////////////////////////////////
// record that we showed an ad
static int ad_end_bookkeeping( db_reference *pRef, int playlistID, int serverID, int entryID, Entry *pe )
{
	PlistCInfo plci;

	// Have clientinfo remember where we were
	plci.lastPlaylistID = playlistID;
	plci.lastServerID = serverID;
	plci.lastEntryID = entryID;
	PlistCInfoSet( pRef->pcip, &plci, PLCIlastPlaylistIDValid|PLCIlastServerIDValid|PLCIlastEntryIDValid );
	
	// how much face time did we get?
	time_t face;
	FaceMeasureReport( pRef->userProc, pRef->adFMB, &face, NULL, NULL, NULL );
	if ( pe->type==kRunout ) face += pe->shownToday;	// add in last facetime
	
	// record values into the ad
	
	// Only do this if a) it's a runout that has been shown even for a second
	// or b) it's been shown for at least its showfor time
	if ( face && pe->type==kRunout || face >= pe->showFor )
	
	// Do the big bite-me on time for toolbar or sponsor ads
	if ( pe->type != kSponsor && pe->type != kButton )
	
	// And only do it if it's now being rerun, and it's within its daymax and showformax
	if ( !pRef->rerun && pe->shown < pe->dayMax && pe->shownFor < pe->showForMax )
	{
		// do funky accounting if it's a runout and has only been partially shown
		if ( pe->type==kRunout && face < pe->showFor )
		{
			pe->shownToday = face;
			SDTrace4( "Runout %s face %d/%d/%d", pe->entryID, pe->shownToday, pe->showFor, pe->showForMax );
		}
		// do normal accounting
		else
		{
			pe->shown++;
			pe->shownFor += pe->showFor;
			SDTrace4( "Ad %s adding face %d for total %d/%d", pe->entryID, face, pe->shownFor, pe->showForMax );
			// normal accounting clears "shownToday" because "shownToday"
			// is used only for partial showings of runouts, and we now
			// know that we have a full showing
			pe->shownToday = 0;
		}
	
		// Find its momma
		PlaylistRecPtr plrp = FindPlaylistInt( pRef->playlists, playlistID );
		if ( plrp )
		{
			EntryRecPtr erp = FindEntry( plrp, serverID, entryID );
			
			if ( erp )
			{
				EntryRecEntryCpy( erp, pe );
				return WriteEntry( plrp, erp, pe );
			}
		}
		
		// oops
		return -1;
	}

	// did nothing, but did it well
	return 0;
}

//////////////////////////////////////////////////////////////////////
// a new day has dawned, change the current ad
// the startover parameter tells us to not remember our place
// in the list of ads.  This should help us clear old ads more
// quickly.
void mgr_DayEndBookkeeping( void *dbRef, bool startOver )
{
	db_reference *pRef = (db_reference *) dbRef;
	
	if ( pRef->curPlaylistID )
	if ( pRef->curEntry.type == kRunout || startOver )
		ad_end_bookkeeping( pRef, pRef->curPlaylistID, pRef->curServerID, pRef->curEntryID, &pRef->curEntry );
	if ( startOver ) pRef->curPlaylistID = 0;
	pRef->nStatus = kScheduleNext; // reschedule
}

//////////////////////////////////////////////////////////////////////
// is an ad ok to show?
static int valid_entry_for_display( EntryRecPtr erp, enum EntryType desiredType, bool today, bool rerun, time_t rerunAge )
{
	// Check that it's a regular ad
	if ( erp->type != desiredType )
		VSDTrace4("Ad %d.%d type %d not type %d",erp->serverID,erp->entryID,erp->type,desiredType);

	// If a timed ad, make sure showFor isn't silly
	else if ( (erp->type==kAd || erp->type==kRunout) && erp->showFor<kMinReasonableShowFor )
		VSDTrace4("Ad %d.%d showFor %d < %d",erp->serverID,erp->entryID,erp->showFor,kMinReasonableShowFor);

	// check for dayMax expired
	else if ( (erp->type==kAd || erp->type==kRunout) && today && !rerun && erp->dayMax && erp->shown >= erp->dayMax )
		VSDTrace4("Ad %d.%d shown %d dayMax %d",erp->serverID,erp->entryID,erp->shown,erp->dayMax);
	
	// check for showForMax expired
	else if ( (erp->type==kAd || erp->type==kRunout) && !rerun && erp->showForMax && erp->shownFor >= erp->showForMax )
		VSDTrace4("Ad %d.%d shownFor %d showForMax %d",erp->serverID,erp->entryID,erp->shownFor,erp->showForMax);
	
	// check for start/end dates
	else if ( erp->startTime && time(0)<erp->startTime )
		VSDTrace3("Ad %d.%d %s too early",erp->serverID,erp->entryID,S2D(erp->startTime-time(0)));
	else if ( erp->endTime && time(0)>erp->endTime )
		VSDTrace3("Ad %d.%d %s too late",erp->serverID,erp->entryID,S2D(time(0)-erp->startTime));

	// check for rerun age
	else if ( (erp->type==kAd || erp->type==kRunout) &&  rerun && time(0) - erp->lastTime > rerunAge )
		VSDTrace3("Ad %d.%d %s too old to rerun",erp->serverID,erp->entryID,S2D(time(0) - erp->lastTime - rerunAge,-1));

	// user can delete toolbar ads
	else if ( erp->type==kButton && erp->userDeleted )
		VSDTrace2( "Ad %d.%d deleted by user", erp->serverID, erp->entryID );

	else
		return true; // We win

	// Nope.
	return false;
}

// We have been tweaked.  Respond.
static void process_tweaks( db_reference* pRef )
{
	// bad not to have any state!
	if (!pRef) return;
	
	// oh that magic feeling
	if ( !pRef->nTweaks ) return;
	
	// looper dooper!
	ForEntriesDo( pRef, tweak_adproc, 0, 0, 0, 0 );

	// We're done
	pRef->nTweaks = 0;
}

static int tweak_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void* /*userRef*/ )
{
	db_reference *pRef = (db_reference *)dbRef;
	Entry entry;
	int i = pRef->nTweaks;
	while ( i-- )
		// does it match?
		if ( erp->serverID==pRef->tweaks[i].serverID && erp->entryID==pRef->tweaks[i].entryID )
		if ( !ReadEntry( plrp, erp, &entry ) )
		{
			// do the tweak
			switch ( pRef->tweaks[i].tweak )
			{
				case kTweakUserDelete:
					entry.userDeleted = 1;
					break;
				case kTweakUserRestore:
					entry.userDeleted = 0;
					break;
			}
			// store the entry
			WriteEntry( plrp, erp, &entry );
			EntryRecEntryCpy( erp, &entry );
		}
	return 1;
}

// remove a playlist if you know the pRef but not the dbHdl
int mgr_RemovePlaylist( long dbRef, int hpl )
{
	if (!dbRef)
		return false;

	db_reference *pRef = (db_reference *)dbRef;
	
	return DbRemovePlaylist( pRef->dbHdl, hpl );
}


// fill out an ENTRY_INFO block (passed in PLIST_SHOWTHIS)
int get_entry_info( PrivCachePtr pcp, Entry* pe, ENTRY_INFO** pei )
{
	int ret = -1;
	ENTRY_INFO* ei = DEBUG_NEW_NOTHROW ENTRY_INFO;

	if ( ei /* eio */ ) {
		ei->entry_type = pe->type;
		ei->mime_type = MimeString( pe->src );
		ei->id = DupeString( pe->entryID );
		ei->title = DupeString( pe->title );

		// TODO: need to figure content-location into this
		if ( ei->mime_type && strcmp( "text/html", ei->mime_type ) == 0 )
			ei->src = prep_html_source( pcp, pe->src );
		else
			ei->src = DupeString( pe->src );

		ei->href = DupeString( pe->href );
		ei->width = pe->width;
		ei->height = pe->height;
		*pei = ei;
		ret = 0;
	}

	return ret;
}

// change our mode
static void change_clientmode( db_reference *pRef, int clientMode )
{
	if ( pRef->pcip->clientMode==clientMode) return;	// already there
	
	// TODO: a reset command should go here, to toss out
	// all our current info and make us fresh and clean
	
	// After the reset, set the client mode
	PlistCInfo pci;
	pci.clientMode = clientMode;
	PlistCInfoSet( pRef->pcip, &pci, PLCIclientModeValid );
}

// make a fully resolved copy of html ad source
char* prep_html_source( PrivCachePtr pcp, char* url )
{
// TODO: "plist_html" uses malloc, which is why we are "free"ing the memory
// allocated by MungeURLs---it should be using "new" instead.

// TODO: this isn't using a checksum on the cache entry -- is that a problem?

	char* prpFileName = 0;
	char* pstr = 0;
	char  no_checksum[33] = "";

	GetCachedEntryPath( pcp, url, no_checksum, &pstr );

	if ( pstr ) {
		prpFileName = TempFile();

		if ( prpFileName ) {
			int fdPrp = open( prpFileName, O_RDWR | O_CREAT, S_IWRITE );
			int fdSrc = open( pstr, O_RDONLY );
			long nBytes = lseek( fdSrc, 0, SEEK_END );
			lseek( fdSrc, 0, SEEK_SET );

			char* pbuf = DEBUG_NEW_NOTHROW char[nBytes + 1];

			if ( pbuf ) {
				read( fdSrc, pbuf, nBytes );
				pbuf[nBytes] = 0;

				char* out = 0;
				MungeURLs( pbuf, "http://adserver.eudora.com", &out );
				write( fdPrp, out, strlen(out) );
				delete [] pbuf;
				free( out );
			}

			close( fdPrp );
			close( fdSrc );
		}

		delete [] pstr;
	}

	return prpFileName;
}


void process_plist_commands( db_reference* pRef )
{
	PlistCInfoPtr pcip = pRef->pcip;

	PlistCmdPtr pcmd;
	int cmd;
	char** args;

	for ( pcmd = pcip->commands; pcmd; pcmd = pcmd->next ) {
		cmd = pcmd->cmd;
		args = pcmd->args;

		if ( cmd == kCmdFlush ) {
			// a flush command has two args: what to flush (e.g. playlist), and
			// the id of what we are flushing. The arg-order is not guaranteed.

			enum things_that_flush {kFlushPlaylist, kFlushEntry, kFlushProfileID, kFlushLimit};
			const char* strWhat[kFlushLimit] = {"playlist", "entry", "profile"};

			int i, j;
			int what = -1;

			for ( i = 0, j = 1; i < kFlushLimit; i++, j = 1 ) {
				if ( stricmp( strWhat[i], args[j] ) == 0 ||
						stricmp( strWhat[i], args[j+=2] ) == 0 ) {

					what = i;
					break;
				}
			} 

			// this is only good if we found something
			char* id = (j == 1) ? args[3] : args[1];

			switch ( what ) {
				case kFlushPlaylist:
				{
					int plst = 0;

					if ( (plst = OpenPlaylist( id, true )) != -1 ) {
						DbRemovePlaylist( pRef->dbHdl, plst );
						ClosePlaylist( plst );
						DisposeDBLists( pRef->playlists );

						// now rebuild the indices
						pRef->playlists = BuildDBLists( pRef->dbHdl );
					}
				}
					break;
				case kFlushEntry:
				{
					char* dot = strchr( id, '.' );
					int eid = dot ? atoi( dot + 1 ) : 0;

					// loop through db looking for instances of "id"
					if ( ForEntriesDo( pRef, flush_entry_adproc, pRef->curPlaylistID,
							pRef->curServerID, pRef->curEntryID, &eid ) != -2 ) {

						SDTrace1( "Failed trying to flush entry: %i", eid );
					}
				}
					break;
				
				case kFlushProfileID:
				{
					pRef->userProc( PLIST_DELETE_PROFILEID, pRef->userRef, (long)id );
					break;
				}

				default:
					assert(0);
			}
		}
		else if ( cmd == kCmdReset ) {
			// blow away the entire playlist database
			DbRemovePlaylist( pRef->dbHdl, -1 );

			// smoke the indices
			DisposeDBLists( pRef->playlists );
			pRef->playlists = NULL;

			// blow away all client info and facetime
			PlistCInfoSet( pcip, 0, 0 );

			// force an update
			pRef->doUpdate = true;
		}
		else
			SDTrace( "What's this? An unhandled command!\n" );
	}

	// now that we're done with the commands, kill them
	if ( pcip && pcip->commands )
	{
		pcip->commands->goPostal();
		pcip->commands = 0;
		PlistCInfoSet( pcip, pcip, PLCIcommandsValid );
	}

	// do we need to set the profile id?
	if ( pcip && pcip->profile ) {
		pRef->userProc( PLIST_SET_PROFILEID, pRef->userRef, (long)pcip->profile );
		delete [] pcip->profile;
		pcip->profile = 0;
	}

	// how about the client mode? [only if high bit is set]
	if ( pcip && (pcip->clientMode & 0x80000000) ) {
		int newMode = pcip->clientMode;
		newMode ^= 0x80000000;
		pRef->userProc( PLIST_SET_CLIENTMODE, pRef->userRef, (long)newMode );
		change_clientmode( pRef, newMode );
 		// reset ad failure
 		if ( newMode == 0 ) 
 		{
 			pcip->adFailure = 0;
			pcip->adsFailing = 0;
 			pRef->userProc( PLIST_SETADFAILURE, 0, (long) 0);
 		}
	}

	// nag somebody?
	if ( pcip && pcip->nagInfo ) {
		NAG_INFO ni;
		memcpy( &ni, pcip->nagInfo, sizeof(NAG_INFO) );
		pRef->userProc( PLIST_NAG_USER, pRef->userRef, (long)&ni );

		// kill it
		delete [] ni.text;
		delete [] pcip->nagInfo;
		pcip->nagInfo = NULL;
	}
}

// "flush" all instances of an entry
static int flush_entry_adproc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp,
			PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	int eid = *(int*)userRef;
	db_reference *pRef = (db_reference *)dbRef;

	if ( nodetype(erp) != kDead && erp->entryID == eid ) {
		Entry e;

		if ( !ReadEntry( plrp, erp, &e ) ) {
		
			if ( erp->type==kButton || erp->type==kSponsor )
				user_delete_entry( pRef, &e );
				
			if ( nodetype(&e) != kDead ) {
				set_nodetype(&e, kDead);

				if ( !WriteEntry( plrp, erp, &e ) )
					SDTrace1( "Flushed entry: %i", eid );
			}
			else
				SDTrace1( "Tried to flush dead entry: %i", eid );

			set_nodetype(erp, kDead);
		}
	}

	// we do the whole db, as there may be multiple instances of
	// the same ad---ergo, we never return zero.
	return 1;
}

// "flush" a single entry
static void flush_erp( PlaylistRecPtr plrp, EntryRecPtr erp )
{
	Entry e;

	if ( !ReadEntry( plrp, erp, &e ) ) {
		if ( nodetype(&e) != kDead ) {
			set_nodetype(&e, kDead);

			if ( !WriteEntry( plrp, erp, &e ) )
				SDTrace2( "Flushed entry: %d.%d", erp->serverID, erp->entryID );
		}
		else
			SDTrace2( "Tried to flush dead entry: %d.%d", erp->serverID, erp->entryID );

		set_nodetype(erp, kDead);
		erp->type = kDeadEntry;
	}
}

#ifdef PLIST_LOGGING
// Output a whole bunch of debug strings in a buffer
void OutputDebugStrings(const char* /*pfx*/, const char *s)
{
	const char *l1, *l2, *theEnd;
	char t[255];
	long len=0;
	theEnd = s + strlen(s);
	for (l1=s;l1<theEnd;l1+=len)
	{
		l2 = strchr(l1,'\n');
		if (!l2) l2 = theEnd; else l2++;
		len = l2 - l1;
		if (len>sizeof(t)-1) len = sizeof(t)-1;
		strncpy(t,l1,len);
		t[len-1] = 0;
		LogAString(t);
	}
}

// Return GetLastErorr as string.
char *GetLastErrorStr(void)
{
	static char msg[1024];
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    NULL,
	    		GetLastError(),
	    		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    		(LPTSTR) &lpMsgBuf,    0,    NULL );
	strcpy(msg,(char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return msg;
}

void LogAString(const char *logMe)
{
	if (g_DebugUserProc) (g_DebugUserProc)(PLIST_LOG,g_DebugUserRef,(long)logMe);
}
#endif

