// plist_xmlrpc.cpp -- "procedure calls" for talking to server
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

// basic mfc, and mfc/wininet
#include "afx.h"
#include "afxwin.h"
#include "afxinet.h"

// the usual
#include "assert.h"
#include "string.h"

#include "plist_stg.h"
#include "plist_debug.h"
#include "plist_cinfo.h"
#include "plist_md5.h"
#include "plist_secret.h"

// our own lazy ass
#include "plist_xmlrpc.h"


#ifdef WIN32
#define snprintf _snprintf
#endif

// this sucks
#include "../../Version.h"

#include "DebugNewHelpers.h"

// Our element map.  This should probably be merged with the parser's element map, but I chickened out
#include "plist_xmlmap.h"
#define PLIST_XML_DEC_MAP
#include "plist_xmlmap.h"

static void add_http_header( CHttpFile* req, const char* nam, const char* val );
static bool is_valid_response( const char* rsp, const char* chksm );
static int get_screen_depth();
static int BuildClientUpdate(ClientUpdateParams *p,CString &theCmd);
static int PlistXMLStart(int &level, CString &theCmd, PlistXMLEnum theTag, const char *attributes);
static int PlistXMLEnd(int &level, CString &theCmd, PlistXMLEnum theTag);
static int PlistXMLTag(int level, CString &theCmd, PlistXMLEnum theTag, const char *attributes, const char *fmt, ...);
static int PlistAddPlaylists(int &level,CString &theCmd,ClientUpdateParams *p,time_t &faceTimeLeft);
static int PlistAdd1Entry(int &level,CString &theCmd,EntryRecPtr erp,time_t &faceTimeLeft, time_t cutoff);
static int PlistAddScreenInfo(int &level,CString &theCmd);

////////////////////////////////////////////////////////////////////////////////
// ClientUpdateContext

ClientUpdateContext::~ClientUpdateContext()
{
	delete [] m_xml;
	delete [] m_pURL;
	if ( m_pParams->profile ) delete [] m_pParams->profile;
	if ( m_pParams->distributor ) delete [] m_pParams->distributor;
	delete m_pParams;
}

/*static*/ void ClientUpdateContext::update_thread_proc( void* p )
{
	ClientUpdateContext* pctx = (ClientUpdateContext*) p;
	if ( _DoClientUpdate( pctx->m_pURL, pctx->m_pParams, &pctx->m_xml ) )
	{
		pctx->status = kNetError;
	}
	else
	{
		PlaylistParser pp;
		if ( (pctx->m_hplst = pp.ImportPlaylist( pctx->m_xml, pctx->m_pParams->pcip )) != -1 )
			pctx->status = kNetComplete;
		else
			pctx->status = kNetError;
	}
}


////////////////////////////////////////////////////////////////////////////////
// ClientUpdateSession

// we use our own InternetSession object, instead of "plist_inet" since we
// haven't converted from mfc yet---allows us to cancel the operation.

// TODO: cancel op isn't impl
class ClientUpdateSession : public CInternetSession
{
	ClientUpdateSession(){}
public:
	ClientUpdateSession( char* userAgent ) : CInternetSession( userAgent ){}

#if 0
	void OnStatusCallback( DWORD dwContext, DWORD dwInternetStatus,
		LPVOID lpvStatusInfo, DWORD dwStatusInfoLength )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );
		IInetContext* pctx = (IInetContext*) dwContext;

		if ( dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE ) {
			LPINTERNET_ASYNC_RESULT prslt =
					(LPINTERNET_ASYNC_RESULT) lpvStatusInfo;

			InetResult ir;
			ir.result = prslt->dwResult;
			ir.error = prslt->dwError;

			pctx->InetComplete( &ir );
		}
		else
			pctx->InetStatus( dwInternetStatus, lpvStatusInfo, dwStatusInfoLength );
	}
#endif
};


///////////////////////////////////////////////////////////////////////////////
// public interfaces
ClientUpdateContext* DoClientUpdate( char* pURL, ClientUpdateParams* p )
{
	ClientUpdateContext* pctx = DEBUG_NEW ClientUpdateContext( pURL, p );
	pctx->exec();
	return pctx;
}

int _DoClientUpdate(/*InetContext* pctx,*/ char* pURL, ClientUpdateParams* p, char** xml )
{
    int ret = -1;
    int nPort;
    char* pHost,* pResponder;

	SDTrace1("_DoClientUpdate %s",pURL);
	
    MunchURL( pURL, &pHost, &pResponder, &nPort );

    // make the xml code for this procedure
    CString theCmd = "";
	char checksum[33];
	MD5Secret itsTrue;
    
    if (theCmd && !BuildClientUpdate(p,theCmd) &&
				GenerateMD5Digest( itsTrue, theCmd, checksum ))
    {
		ClientUpdateSession s( "Eudora/" EUDORA_BUILD_VERSION " (Win)" );
	
	    try
	    {
	        CHttpConnection* pConn = s.GetHttpConnection( pHost, (INTERNET_PORT)nPort, "", "" );
	        CHttpFile* pFile = pConn->OpenRequest( "POST", pResponder, NULL, 1,
					NULL, NULL, INTERNET_FLAG_DONT_CACHE );

			add_http_header( pFile, "Content-Type", "text/xml" );
			add_http_header( pFile, "MessageType", "Call" );
			add_http_header( pFile, "Checksum", checksum );
			OutputDebugString("Requesting playlist...\n");
			OutputDebugStrings("req->",theCmd);

	        pFile->SendRequest( NULL, 0, (void*)((const char*)theCmd), theCmd.GetLength() );
	
	        DWORD dwRet = 0;
	        pFile->QueryInfoStatusCode( dwRet );
	
	        if ( dwRet == HTTP_STATUS_OK ) {
	            DWORD dwContentLength = 0;
	            ULONG len = sizeof(dwContentLength);

				pFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH |
							HTTP_QUERY_FLAG_NUMBER, &dwContentLength, &len );

				len = sizeof(checksum);
				memset( checksum, '\0', sizeof(checksum) );
				strcpy( checksum, "Checksum" );

				// MFC asserts on HTTP_QUERY_CUSTOM, even though it ends up
				// working just fine. Go figure. Use WinInet straight.
				HttpQueryInfo( *pFile, HTTP_QUERY_CUSTOM, checksum, &len, 0 );

				if ( dwContentLength && *checksum ) {
					char* pResult = DEBUG_NEW_NOTHROW char[dwContentLength + 1];

					if ( pResult ) {
						UINT nRead = pFile->Read( pResult, dwContentLength );
						pResult[nRead] = 0;

						if ( is_valid_response( pResult, checksum ) ) {
							*xml = pResult;
							ret = 0;
						}
						else
						{
							SDTrace1( "Playlist checksum %s bad", checksum );
							delete [] pResult;
						}
					}
				}
			}
	
	        pFile->Close();
	        pConn->Close();
	        delete pFile;
	        delete pConn;
	    }
	    catch ( CInternetException* pEx )
	    {
	        char eBuf[256] = "";
	        pEx->GetErrorMessage( eBuf, sizeof(eBuf) );
	        printf( eBuf );
	        pEx->Delete();
	    }
	
		s.Close();
	}

    delete [] pHost;
    delete [] pResponder;
        
    return ret;
}

////////////////////////////////////////////////////////////////
// Build the client update request
static int BuildClientUpdate(ClientUpdateParams *p,CString &theCmd)
{
    int level = 0;
    int err = 1;
    time_t faceTimeLeft = 0;
	TCHAR attrs[32];
	int effectiveMode = p->pcip->clientMode;

	// we don't have an explicit deadbeat mode, so gen one up here
	if  (effectiveMode == 1 /* light */ && p->pcip->adFailure>=kAdFailureGraceDays)
		effectiveMode = 3; /* deadbeat */

    theCmd = "<?xml version=\"1.0\"?>\n\n";
    
    // Open the clientupdate parameter
    if (!PlistXMLStart(level,theCmd,kXMLclientUpdate,NULL))
    
    // Add the playlist info, and compute faceTimeLeft
    if (
#ifdef _DEBUG
		p->ignoreCurrent ||
#endif
    	!PlistAddPlaylists(level,theCmd,p,faceTimeLeft))
	    
    // Open the clientinfo parameter
    if (!PlistXMLStart(level,theCmd,kXMLclientInfo,NULL))

	// The version of Playlists.txt that we support.  Sorta.
	if (!PlistXMLTag(level,theCmd,kXMLplaylistVersion,NULL,"%d",22))

	// add locale info
	if ( sprintf(attrs,"language=\"%s\"", p->langID) &&
			!PlistXMLTag(level, theCmd, kXMLlocale, attrs, 0) )
	
	// Add screen information
	if (!PlistAddScreenInfo(level,theCmd))

	// The mode the client is running in.
	if (!PlistXMLTag(level,theCmd,kXMLclientMode,NULL,"%d",effectiveMode))

	// The profile.
	if (!p->profile || !PlistXMLTag(level,theCmd,kXMLprofile,NULL,"%s",p->profile))
	    
	// The distributor id.
	if (!p->distributor || !PlistXMLTag(level,theCmd,kXMLdistributor,NULL,"%s",p->distributor))
	    
    // Add the useragent
    if (!PlistXMLTag(level,theCmd,kXMLuserAgent,NULL,"%s",p->userAgent))
    
    // Now we add facetimes for the last week
    // Note that we store values in seconds, but give them in minutes
    if (!PlistXMLTag(level,theCmd,kXMLfaceTime,NULL,"%d,%d,%d,%d,%d,%d,%d",
    	p->pcip->faceTimeWeek[0]/D_SIXTY,
    	p->pcip->faceTimeWeek[1]/D_SIXTY,
    	p->pcip->faceTimeWeek[2]/D_SIXTY,
    	p->pcip->faceTimeWeek[3]/D_SIXTY,
    	p->pcip->faceTimeWeek[4]/D_SIXTY,
    	p->pcip->faceTimeWeek[5]/D_SIXTY,
    	p->pcip->faceTimeWeek[6]/D_SIXTY))
    
    // The ad time we've used today, in case the server wants to
    // deliver an ad it knows has to be shown today
    if (!PlistXMLTag(level,theCmd,kXMLfaceTimeUsedToday,NULL,"%d",p->pcip->adFaceTimeUsedToday/D_SIXTY))
    
    // The ad time we have left in our ad cache
    if (!PlistXMLTag(level,theCmd,kXMLfaceTimeLeft,NULL,"%d",faceTimeLeft/60))
    
    // The silly pastry
    if (!*p->pcip->pastry || !PlistXMLTag(level,theCmd,kXMLpastry,p->pcip->pastry,NULL))
    
    // Ok, that was fun; close it up
    if (!PlistXMLEnd(level,theCmd,kXMLclientInfo))
    if (!PlistXMLEnd(level,theCmd,kXMLclientUpdate))
    	err = 0;
    
    // Did we win?
    return err;	 
}

#ifdef _DEBUG
char *DBBuildClientUpdate(ClientUpdateParams *p)
{
	CString cmdCString;
	char *cmd = NULL;
	
	if ( !BuildClientUpdate(p,cmdCString) )
	{
		cmd = DEBUG_NEW_NOTHROW char[cmdCString.GetLength()+1];
		if ( cmd ) strcpy( cmd, (LPCTSTR)cmdCString );
	}
	return cmd;
}
#endif

struct PLLoop {
	CString *theCmd;
	char *plID;
	time_t faceTimeLeft;
	int level;
	time_t cutoff;
};

// ugly extern import of cool looper
typedef (*adproc)( void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
extern int ForEntriesDo( void *dbRef, adproc proc, int startPlaylistID, int startServerID, int startEntryID, void *userRef );
// end ugly
static int AddPlaylistsProc( void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );

////////////////////////////////////////////////////////////////
// Add the playlists to the command, and compute faceTimeLeft for all
static int PlistAddPlaylists(int &level,CString &theCmd,ClientUpdateParams *p,time_t &faceTimeLeft)
{
	PLLoop theStuff;
	
	theStuff.theCmd = &theCmd;
	theStuff.plID = NULL;
	theStuff.faceTimeLeft = 0;
	theStuff.level = level;
	theStuff.cutoff = time(0) - p->pcip->historyLength;
	
	// remember how long it was
	int oldLen = theCmd.GetLength();
	
	// add all the playlists and entries
	ForEntriesDo( p->dbRef, AddPlaylistsProc, 0, 0, 0, &theStuff );
	
	// record the amount of facetime we had
	faceTimeLeft = theStuff.faceTimeLeft;
	
	// if it got longer, we must have added at least one playlist, so close the last one.
	if ( oldLen != theCmd.GetLength() )
	{
		level++;	// we are actually inside this level
		PlistXMLEnd( level, theCmd, kXMLplaylist );
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////
// the adproc that does the work
static int AddPlaylistsProc( void* /*dbRef*/, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void *userRef )
{
	CString attributes="";
	PLLoop *theStuff = (PLLoop *) userRef;
	
	// if the current playlist has changed, generate a new playlist tag
	if ( theStuff->plID != plrp->plID )
	{
		// if we had a playlist before, close it out
		if ( theStuff->plID ) PlistXMLEnd(theStuff->level,*theStuff->theCmd,kXMLplaylist);
		
		theStuff->plID = plrp->plID;
		attributes.Format("id=\"%s\"",plrp->plID);
		PlistXMLStart(theStuff->level,*theStuff->theCmd,kXMLplaylist,(LPCTSTR)attributes);
	}

	// Now add this entry
	PlistAdd1Entry(theStuff->level, *theStuff->theCmd, erp, theStuff->faceTimeLeft, theStuff->cutoff);
	
	// keep going...
	return 1;
}

////////////////////////////////////////////////////////////////
// Add a single entry to the command
static int PlistAdd1Entry(int &level,CString &theCmd,EntryRecPtr erp, time_t &faceTimeLeft, time_t cutoff)
{
	CString attributes;
	int err = -1;
	
	// Is it dead?
	if ( erp->type == kDeadEntry ) return 1;
		
	// Ask the entry how much time it has left
	time_t left = 0;
	if ( erp->endTime && time(0)>erp->endTime )
		left = 0;
	else if ( !erp->showForMax )
		left = erp->showFor ? erp->showFor : 1;
	else if ( erp->shownFor < erp->showForMax )
		left = erp->showForMax - erp->shownFor;
	
	// is it too old to list?
	if ( !left && erp->lastTime<cutoff )
		return 0;
	
	// Format the entry ID part
	attributes.Format("id=\"%d.%d\"",erp->serverID, erp->entryID);
	
	// Is ad active?
	char s[32];
	attributes += ' ';
	attributes += gXMLTags[kXMLactive].name;
	sprintf(s,"=\"%d\"",erp->showFor ? left/erp->showFor : left);
	attributes += s;
	
	// Is Ad runout?
	if (erp->type==kRunout)
	{
		attributes += ' ';
		attributes += gXMLTags[kXMLisRunout].name;
		attributes += "=\"1\"";
	}
	
	// Is Ad button?
	if (erp->type==kButton)
	{
		attributes += ' ';
		attributes += gXMLTags[kXMLisButton].name;
		attributes += "=\"1\"";
	}
	
	// Is Ad sponsor?
	if (erp->type==kSponsor)
	{
		attributes += ' ';
		attributes += gXMLTags[kXMLisSponsor].name;
		attributes += "=\"1\"";
	}
	
	// Has ad been deleted?
	if (erp->userDeleted)
	{
		attributes += ' ';
		attributes += gXMLTags[kXMLdeletedByUser].name;
		attributes += "=\"1\"";
	}
	
	// finally, build the darn tag
	err = PlistXMLTag(level,theCmd,kXMLentry,(LPCTSTR)attributes,NULL);
	
	// And count up the facetime
	if ( erp->type==kAd ) faceTimeLeft += left;
	
	return err;
}

////////////////////////////////////////////////////////////////
// add screen info
static int PlistAddScreenInfo(int &level,CString &theCmd)
{
	CString attributes;
	int width = GetSystemMetrics(SM_CXFULLSCREEN);
	int height = GetSystemMetrics(SM_CYFULLSCREEN);
	int depth = get_screen_depth();
	
	if (height>0 && width>0 && depth>0)
	{
		attributes.Format("width=\"%d\" height=\"%d\" depth=\"%d\"",width,height,depth);
		return PlistXMLTag(level,theCmd,kXMLscreen,(LPCTSTR)attributes,NULL);
	}

	return 0;
}

////////////////////////////////////////////////////////////////
// Start an xml block
static int PlistXMLStart(int &level, CString &theCmd, PlistXMLEnum theTag, const char *attributes)
{
	// Better living through indent
	for (int i=0; i<level; i++) theCmd += "  ";
	
	// The next callers will get to add another tab!
	level++;
	
	// Open command
	theCmd += '<';
	theCmd += gXMLTags[theTag].name;
	
	// Add attributes
	if (attributes)
	{
		theCmd += ' ';
		theCmd += attributes;
	}
	
	// Close tag
	theCmd += ">\n";

	
	// We're just making believe that worked for now
	return 0;
}

////////////////////////////////////////////////////////////////
// End an xml block
static int PlistXMLEnd(int &level, CString &theCmd, PlistXMLEnum theTag)
{
	// The next callers will add one less tab!
	level--;
	
	// Better living through indent
	for (int i=0; i<level; i++) theCmd += "  ";
	
	// Here is the deep, difficult, significant work
	char tagStr[256]; // really, this is enough for a tag name
	sprintf(tagStr,"</%s>\n",gXMLTags[theTag].name);
	theCmd += tagStr;
		
	// We're just making believe that worked for now
	return 0;
}

////////////////////////////////////////////////////////////////
// Output a complete tag
static int PlistXMLTag(int level, CString &theCmd, PlistXMLEnum theTag, const char *attributes, const char *fmt, ...)
{
	// No body?
	if (!fmt)
	{
		// Better living through indent
		for (int i=0; i<level; i++) theCmd += "  ";
		
		// Open the tag
		theCmd += '<';
		theCmd += gXMLTags[theTag].name;

	
		// Add attributes
		if (attributes)
		{
			theCmd += ' ';
			theCmd += attributes;
		}
		
		// Close the tag
		theCmd += "/>\n";
	}
	else
	{
		// Issue the beginning of the tag
	    int fakeLevel = level;
		PlistXMLStart(fakeLevel,theCmd,theTag,attributes);
		theCmd.TrimRight();	// kill the newline we just added
		
		// Good ol' vsprintf...
		char s[2048];	// we're in trouble if doing a simple tag with more than 2K chars
						// I am not afraid.
		va_list marker;
		va_start( marker, fmt );
	    vsprintf( s, fmt, marker);
	    va_end( marker );
	    
	    // Add the body to the tag
	    theCmd += s;
	    
	    // And now issue the end of the tag.
	    fakeLevel = 1;	// no indents here
	    PlistXMLEnd(fakeLevel,theCmd,theTag);
    }
    
    // we continue to whistle in the dark regarding errors
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// santa's little helpers

inline void trim( char* s )
{
	char* c;
	for ( c = s; *c == ' '; c++ )
		;
	if ( c != s )
		strcpy( s, c );
	for ( c = &s[strlen(s)-1]; *c == ' '; c-- )
		;
	*(c+1) = 0;
}

// add an http header to the request -- if header exists, it is replaced
void add_http_header( CHttpFile* req, const char* nam, const char* val )
{
	char hdr[1025] = "";

	snprintf( hdr, sizeof(hdr) - 1, "%s: %s", nam, val );

	req->AddRequestHeaders( hdr, HTTP_ADDREQ_FLAG_REPLACE |
				HTTP_ADDREQ_FLAG_ADD );
}

// make sure checksum in header matches payload
bool is_valid_response( const char* rsp, const char* chksm )
{
	char newChksm[33] = "";
	MD5Secret itsTrue;
	if ( GenerateMD5Digest( itsTrue, rsp, newChksm ) )
		return (strcmp( chksm, newChksm ) == 0);

	return false;
}

static int get_screen_depth()
{
	HDC hdc;
	hdc = CreateCompatibleDC(NULL);
	int iDeviceBits = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
	DeleteDC(hdc);
	return iDeviceBits;
}
