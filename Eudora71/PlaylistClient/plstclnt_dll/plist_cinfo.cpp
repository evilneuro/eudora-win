// plist_cinfo.cpp -- ClientInfo handling for the playlist manager
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

// basic mfc
#include "afx.h"

#include <stdio.h>

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

#include "plist_cinfo.h"
#include "plist_debug.h"
#include "plist_utils.h"
#include "plist_stg.h"
#include "plist_cmd.h"

#include "DebugNewHelpers.h"

////////////////////////////////////////////////////////////////
// Private declarations
////////////////////////////////////////////////////////////////

#define kCurPlistCInfoVersion	2

#define kSaveInterval			(15*60)

#define kDefaultAdWindowHeight	144
#define kDefaultAdWindowWidth	144
#define kDefaultScheduleType	PlistCInfoSchedLinear
#ifdef FAST_AD_SCHEDULING
#define kDefaultHistoryLength	(30*60)			// 30 minutes
#define kDefaultReqInterval		(10*60)			// 10 minutes
#define kDefaultFaceTimeQuota	(24*2*4)		// 24 ads shown twice for four seconds
#define	kDefaultRerunInterval	(24*60)			// 24 minutes
#define kQuotaSlop				(5)
#else
#define kDefaultHistoryLength	(30*24*60*60)	// 30 days
#define kDefaultReqInterval		( 1*24*60*60)	//  1 day
#define kDefaultFaceTimeQuota	(60*60)			//  1 hour
#define	kDefaultRerunInterval	( 7*24*60*60)	//  7 days
#define kQuotaSlop				( A_MINUTE )
#endif

void PlistCInfoNewDayCheck(PlistCInfoPtr pcip);
void PlistCInfoFaceTimeRoll(PlistCInfoPtr pcip);
PlistCInfoPtr PlistCInfoRead(const char *path);
void PlistCInfoResetDayStuff(PlistCInfoPtr pcip);
int PlistCInfoResetDayProc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, long refCon );

// ugly extern import of cool looper
typedef (*adproc)( void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );
extern int ForEntriesDo( void *dbRef, adproc proc, int startPlaylistID, int startServerID, int startEntryID, void *userRef );
// end ugly
int PlistCInfoResetDayProc(void *dbRef, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr plrpWas, EntryRecPtr erpWas, void *userRef );

// more ugly
extern void mgr_DayEndBookkeeping( void *dbRef, bool startOver );
// end more ugly

////////////////////////////////////////////////////////////////
// Public routines
////////////////////////////////////////////////////////////////

//
// Routines for startup, shutdown, load, save, etc
//

////////////////////////////////////////////////////////////////
// Open or create a new one
PlistCInfoPtr PlistCInfoNew(const char *fName,const char *dirName, plist_callback pfn, void *dbRef, const int adFailure )
{
	PlistCInfoPtr pcip = NULL;
	bool needInit = false;
	char *path = DEBUG_NEW_NOTHROW char[strlen(dirName)+strlen(fName)+1];
	
	if (!path) return NULL;
	strcpy(path,dirName);
	strcat(path,fName);
	
	
	// First, we try to read the old one
	pcip = PlistCInfoRead(path);
	
	//
	// if we failed to read the old one, so make a new one
	//
	if (!pcip)
	{
		SDTrace("Allocating PlistClientInfo");
		
		// allocate the memory
		pcip = DEBUG_NEW_NOTHROW PlistCInfo;
	
		// Can we fail to allocate so few bytes?  Sure...
		if (!pcip) return NULL;
		
		// Set flag so that later on we set defaults
		needInit = true;
		
		// initialize
		memset(pcip,'\0',sizeof(*pcip));
		pcip->dirty = true;
		pcip->version = kCurPlistCInfoVersion;
	}
	
	//
	// now we have one, either new or read from file,
	// certain things need to be re-initialized
	//
	if (pcip)
	{
		pcip->userProc = pfn;			// callback into app so we can calc facetime
		pcip->fmb = NULL;				// We init the fmb later
		pcip->path = path; path=NULL;	// Remember our path, and don't delete it
		pcip->adShowing = false;		// So far as we know, anyway...
		pcip->dbRef = dbRef;			// We need this for new day rollovers
		pcip->commands = 0;             // init from playlist; garbage from file
		pcip->dirty = false;			// We're new, so we're not dirty
		pcip->adFailure = adFailure;	// Slam this value from the app
		pcip->profile  = 0;             // init from playlist; garbage from file
		pcip->nagInfo = 0;              // init from playlist; garbage from file
	}
	
	//
	// Ok, let's make sure that we got everything we wanted to get
	//
	if (pcip)
	{
		// Cool!  Let's get 'em rolling
		if (needInit) PlistCInfoSet(pcip,NULL,0);
	}
	else if (pcip)
	{
		// We failed, but we got partway
		pcip->dirty = false;		// Don't save
		PlistCInfoDispose(pcip);	// Destroy
		delete pcip; pcip = NULL;	// Sadly, we give nothing to the caller
	}
	
	if (path) delete path;	// oops, didn't make it...
	
	// give 'em what we got		
	return pcip;
}

////////////////////////////////////////////////////////////////
// Kill the Pig!
void PlistCInfoDispose(PlistCInfoPtr pcip)
{
	if (pcip)
	{
		PlistCInfoSave(pcip);
		if (pcip->path) delete [] pcip->path;
		if (pcip->fmb) DisposeFaceMeasure(pcip->userProc,pcip->fmb);
		if (pcip->commands) pcip->commands->goPostal();
		delete [] pcip->profile;
		if ( pcip->nagInfo ) {
			// most peculiar mama
			char* s = (char*) pcip->nagInfo[1];
			delete [] s;
			delete [] pcip->nagInfo;
		}
		delete pcip;
	}
}

////////////////////////////////////////////////////////////////
// On idle, save every once in a while
void PlistCInfoIdle(PlistCInfoPtr pcip)
{
	if (!pcip) return;
	
	// Finish up initialization
	// This could go in init, but I moved it here because
	// of some initialization problem that turned out to be
	// bogus, and I don't know that I want to move it back
	// since it seems to work here.  SD 12/25/99
	if (!pcip->fmb)
	{
		pcip->fmb = NewFaceMeasure(pcip->userProc);
		if (pcip->fmb)
		{
			// Total facetime begins now
			FaceMeasureBegin(pcip->userProc,pcip->fmb);
			
			// Figure out what day it is
			PlistCInfoNewDayCheck(pcip);
		}
	}

	// Is it tomorrow yet?
	PlistCInfoNewDayCheck(pcip);
	
	// Do we need to save the data?
	if (time(0) - pcip->lastSaveTime > kSaveInterval)
		PlistCInfoSave(pcip);
}

//
// Global facetime management stuff
//

////////////////////////////////////////////////////////////////
// Accumulate facetime for ads
void PlistCInfoAdStart(PlistCInfoPtr pcip)
{
	if (!pcip) return;
	
	//SDTrace("PlistCInfoAdStart");
	PlistCInfoFaceTimeRoll(pcip);
	pcip->adShowing = true;
}

void PlistCInfoAdEnd(PlistCInfoPtr pcip)
{
	if (!pcip) return;
	
	//SDTrace("PlistCInfoAdEnd");
	PlistCInfoFaceTimeRoll(pcip);
	pcip->adShowing = false;
}

////////////////////////////////////////////////////////////////
// Accept info from our host
int PlistCInfoSet(PlistCInfoPtr pcip,PlistCInfoPtr pciSetp,int validFlags)
{
	if (!pcip) return 1;
	
	// Is this a mind-blow?
	if (!pciSetp)
	{
		pcip->scheduleType = kDefaultScheduleType;
		pcip->historyLength = kDefaultHistoryLength;
		pcip->adWindowHeight = kDefaultAdWindowHeight;
		pcip->adWindowWidth = kDefaultAdWindowWidth;
		pcip->reqInterval = kDefaultReqInterval;
		pcip->lastSaveTime = 0;
		pcip->faceTimeUsedToday = 0;
		pcip->adFaceTimeUsedToday = 0;
		pcip->faceTimeQuota = kDefaultFaceTimeQuota;
		pcip->rerunInterval = kDefaultRerunInterval;
		int i;
		for (i=0;i<7;i++) pcip->faceTimeWeek[i] = 0;
		for (i=0;i<sizeof(pcip->spareStuff)/sizeof(int);i++)
			pcip->spareStuff[i] = 0;
		pcip->lastFetchTime = 0;
		pcip->lastUpdateTime = 0;
		pcip->lastSaveTime = 0;
		pcip->faceTimeAWeekAgo;
		pcip->adShowing = false;
		if (pcip->fmb)
		{
			FaceMeasureReset(pcip->userProc,pcip->fmb);
			FaceMeasureBegin(pcip->userProc,pcip->fmb);
		}
		pcip->dirty = true;
		pcip->dayOfWeek = -1;
		pcip->lastPlaylistID = 0;
		pcip->lastServerID = 0;
		pcip->lastEntryID = 0;
		pcip->checkedMail = 0;
		// Note that we don't blow the ad failure
		// pcip->adFailure = 0; NOT!
		return 0;
	}

// If valid bit and different integers, set and set dirty
#define PLIST_CINFO_SET_IF_VALID_I(x)\
	if ((validFlags&PLCI ## x ## Valid) && \
		(pcip-> ## x != pciSetp-> ## x)) \
	{ \
		pcip-> ## x = pciSetp-> ## x; \
		pcip->dirty = true; \
	}
// If valid bit and different strings, set and set dirty
#define PLIST_CINFO_SET_IF_VALID_S(x)\
	if ((validFlags&PLCI ## x ## Valid) && \
		strcmp((const char *)pcip-> ## x, pciSetp-> ## x)) \
	{ \
		strncpy(pcip-> ## x, pciSetp-> ##x, sizeof(pcip-> ## x)); \
		pcip->dirty = true; \
	}

	// Now we just mention the field names, and the preprocessor
	// does all the tedium for us.
	PLIST_CINFO_SET_IF_VALID_I(faceTimeQuota);
	PLIST_CINFO_SET_IF_VALID_I(adWindowHeight);
	PLIST_CINFO_SET_IF_VALID_I(adWindowWidth);
	PLIST_CINFO_SET_IF_VALID_I(lastFetchTime);
	PLIST_CINFO_SET_IF_VALID_I(lastUpdateTime);
	PLIST_CINFO_SET_IF_VALID_I(scheduleType);
	PLIST_CINFO_SET_IF_VALID_I(historyLength);
	PLIST_CINFO_SET_IF_VALID_I(reqInterval);
	PLIST_CINFO_SET_IF_VALID_I(commands);
	PLIST_CINFO_SET_IF_VALID_S(pastry);
	PLIST_CINFO_SET_IF_VALID_I(clientMode);
	PLIST_CINFO_SET_IF_VALID_I(lastPlaylistID);
	PLIST_CINFO_SET_IF_VALID_I(lastServerID);
	PLIST_CINFO_SET_IF_VALID_I(lastEntryID);
	PLIST_CINFO_SET_IF_VALID_I(rerunInterval);
	PLIST_CINFO_SET_IF_VALID_I(checkedMail);
	PLIST_CINFO_SET_IF_VALID_I(profile);
	PLIST_CINFO_SET_IF_VALID_I(nagInfo);
	
	return 0;
}

////////////////////////////////////////////////////////////////
// Guts
////////////////////////////////////////////////////////////////

//
// Reading, writing, and updating
//
int PlistCInfoSanity(PlistCInfoPtr pcip,size_t size);

////////////////////////////////////////////////////////////////
// Save the current data
int PlistCInfoSave(PlistCInfoPtr pcip)
{
	if (!pcip) return 1;
	
	pcip->lastSaveTime = time(0);
	
	PlistCInfoFaceTimeRoll(pcip);

#ifdef _DEBUG
	CString output;
	PlistCInfoDump(pcip, output);
	OutputDebugStrings("",output);
#endif
	
	if (!pcip->dirty) return 0;

	char *tempfile = TempFile();
	int err = 1;
	
	// make a temp file
	if (tempfile)
	{
		// Open the temp file, write to it, flush it...
		FILE *fp = fopen(tempfile,"wb");
		if (fp)
		{
			if (fwrite(pcip,1,sizeof(*pcip),fp)==sizeof(*pcip))
			{
				if (!fflush(fp))
				{
					fclose(fp); fp=NULL;
					
					// Now, delete & rename the original
					unlink(pcip->path);
					err = rename(tempfile,pcip->path);
					ASSERT(!err);	// did we win?
					if (!err)
					{
						pcip->dirty = false;
						pcip->lastSaveTime = time(NULL);
					}
				}
					
			}
			if (fp) fclose(fp);
		}
		delete [] tempfile;
	}

#ifdef PLIST_LOGGING
	if (err) SDTrace1("Failed to write ClientInfo %s",pcip->path);
#endif
	
	return err;
}

////////////////////////////////////////////////////////////////
// Print the client info for debugging
void PlistCInfoDump(PlistCInfoPtr pcip, CString &output)
{
	char buffer[256];
	
	sprintf(buffer,"PlistCInfo v%d %s %d\n",pcip->version, pcip->path, pcip->dirty);
	output += buffer;
	sprintf(buffer,"adWidth %d adHeight %d schedType %d\n",
		pcip->adWindowWidth, pcip->adWindowHeight,pcip->scheduleType);
	output += buffer;
	sprintf(buffer,"reqInterval %s histLength %s rerunInterval %s\n",
		S2D(pcip->reqInterval), S2D(pcip->historyLength), S2D(pcip->rerunInterval));
	output += buffer;
	sprintf(buffer,"adDay %s totalDay %s lastWeek %s quota %s failure %s\n",
		S2D(pcip->adFaceTimeUsedToday,-1), S2D(pcip->faceTimeUsedToday,-1), S2D(pcip->faceTimeAWeekAgo,-1),
		S2D(pcip->faceTimeQuota), S2D(pcip->adFailure*A_DAY,1));
	output += buffer;
	sprintf(buffer,"      %c      %c      %c      %c      %c      %c      %c\n",
				pcip->dayOfWeek==0 ? '*':'S',
				pcip->dayOfWeek==1 ? '*':'M',
				pcip->dayOfWeek==2 ? '*':'T',
				pcip->dayOfWeek==3 ? '*':'W',
				pcip->dayOfWeek==4 ? '*':'R',
				pcip->dayOfWeek==5 ? '*':'F',
				pcip->dayOfWeek==6 ? '*':'S');
	output += buffer;
	sprintf(buffer," %6s %6s %6s %6s %6s %6s %6s",
		S2D(pcip->faceTimeWeek[0],-1),
		S2D(pcip->faceTimeWeek[1],-1),
		S2D(pcip->faceTimeWeek[2],-1),
		S2D(pcip->faceTimeWeek[3],-1),
		S2D(pcip->faceTimeWeek[4],-1),
		S2D(pcip->faceTimeWeek[5],-1),
		S2D(pcip->faceTimeWeek[6],-1));
	output += buffer;
}

////////////////////////////////////////////////////////////////
// Read the file
PlistCInfoPtr PlistCInfoRead(const char *path)
{
	// We have yet to succeed
	PlistCInfoPtr returnPCIP = NULL;
	
	// Figure out how big the file is
	struct _stat statBuf;
	if (!_stat(path,&statBuf))
	{
		// If it's bigger than it's supposed to be,
		// goodness only knows what's in it--pitch it
		if (statBuf.st_size <= sizeof(PlistCInfo))
		{
			// Allocate the pointer
			PlistCInfoPtr pcip = DEBUG_NEW_NOTHROW PlistCInfo;
			if (pcip)
			{
				memset(pcip,'\0',sizeof(*pcip));
				
				// Open and read
				FILE *fp = fopen(path,"rb");
				if (fp)
				{
					if (fread(pcip,1,statBuf.st_size,fp)==(size_t)statBuf.st_size)
					{
						// Ok, now let's worry about whether or not the
						// data is good
						if (!PlistCInfoSanity(pcip,statBuf.st_size))
						{
							returnPCIP = pcip;
							CString output;
#ifdef _DEBUG
							pcip->path = (char*) path;	// record now, for benefit of dump
							PlistCInfoDump(pcip,output);
							OutputDebugStrings("",output);
#endif
							pcip = NULL;
						}
					}
					
					// We're on our way out...
					fclose(fp);
				}
				
				// If pcip is not NULL here, we have failed
				if (pcip) delete pcip;
			}
		}
	}

#ifdef PLIST_LOGGING
	if (!returnPCIP) SDTrace1("Failed to read ClientInfo %s",path);
#endif
	
	return returnPCIP;
}

////////////////////////////////////////////////////////////////
// Check the reasonableness of what we just read
int PlistCInfoSanity(PlistCInfoPtr pcip,size_t size)
{
	if (!pcip) return 1;
	
	if (pcip->version==kCurPlistCInfoVersion)
	{
		// If it's our version, it better be our size
		if (size!=sizeof(*pcip)) return -1;
	}
	else switch (pcip->version)
	{
		case 1:
			// Update from version 1 is trivial, all we did was add pastry to
			// the end for version 2.
			pcip->version = kCurPlistCInfoVersion;
			break;
		default:
			// We have no idea how to upgrade from this
			// version.  Kill the pig!
			return -1;
	}
		
	// Worry about some values
	if (pcip->lastUpdateTime > time(0))
	{
		SDTrace1("lastUpdateTime %s unreasonable",ctime(&pcip->lastUpdateTime));
		pcip->lastUpdateTime = 0;
	}
	if (pcip->lastFetchTime > time(0))
	{
		SDTrace1("lastFetchTime %s unreasonable",ctime(&pcip->lastUpdateTime));
		pcip->lastFetchTime = 0;
	}
	if (pcip->historyLength==0 || pcip->historyLength>10*kDefaultHistoryLength)
	{
		SDTrace1("historyLength %s unreasonable",S2D(pcip->historyLength));
		pcip->historyLength = kDefaultHistoryLength;
	}
	if (pcip->faceTimeQuota==0 || pcip->faceTimeQuota>10*kDefaultFaceTimeQuota)
	{
		SDTrace1("faceTimeQuota %s unreasonable",S2D(pcip->faceTimeQuota));
		pcip->faceTimeQuota = kDefaultFaceTimeQuota;
	}
	if (pcip->reqInterval==0 || pcip->reqInterval>10*kDefaultReqInterval)
	{
		SDTrace1("reqInterval %s unreasonable",S2D(pcip->reqInterval));
		pcip->reqInterval = kDefaultReqInterval;
	}
	if (pcip->rerunInterval==0 || pcip->rerunInterval>10*kDefaultRerunInterval)
	{
		SDTrace1("rerunInterval %s unreasonable",S2D(pcip->rerunInterval));
		pcip->rerunInterval = kDefaultRerunInterval;
	}
	if (pcip->adWindowHeight==0 || pcip->adWindowHeight>2*kDefaultAdWindowHeight)
	{
		SDTrace1("adWindowHeight %d unreasonable",pcip->adWindowHeight);
		pcip->adWindowHeight = kDefaultAdWindowHeight;
	}
	if (pcip->adWindowWidth==0 || pcip->adWindowWidth>2*kDefaultAdWindowWidth)
	{
		SDTrace1("adWindowWidth %d unreasonable",pcip->adWindowWidth);
		pcip->adWindowWidth = kDefaultAdWindowWidth;
	}
	
	// I guess we're ok
	return 0;
}


////////////////////////////////////////////////////////////////
// Check for a new day
void PlistCInfoNewDayCheck(PlistCInfoPtr pcip)
{
	// Are we setup yet?
	if (!pcip || !pcip->fmb) return;
	
	time_t now;
	time(&now);
	int day = localtime(&now)->tm_wday;
	
#ifdef FAST_AD_SCHEDULING
	day = (localtime(&now)->tm_min/2)%7;
#endif

	if (day != pcip->dayOfWeek)
	{
		SDTrace2("()()()()()()()()()() New Day: %d->%d ()()()()()()()()()()",pcip->dayOfWeek,day);
		
		// Commit facetime from last time
		PlistCInfoFaceTimeRoll(pcip);
		
		// Tell the scheduler it has a new day
		// If we got to the quota, it means we should start over
		// at the beginning of the ads so as to be sure we clear
		// out old ads instead of just showing new ones
		mgr_DayEndBookkeeping( pcip->dbRef, pcip->adFaceTimeUsedToday>=pcip->faceTimeQuota );
		
		// Commit facetime from yesterday
		if (pcip->dayOfWeek >= 0)
			pcip->faceTimeWeek[pcip->dayOfWeek] = pcip->faceTimeUsedToday;
		
#ifdef PLIST_LOGGING
		int oldAdFailure = pcip->adFailure;
#endif
		// Keep track of ad failure
		pcip->adsFailing = false;	// we don't know they're failing yet
		if ( pcip->adFaceTimeUsedToday+kQuotaSlop < pcip->faceTimeUsedToday && // there was face time available
																	   // the +60 allows a minute of slop
																	   // between the ad & total timers
			 pcip->adFaceTimeUsedToday < (pcip->faceTimeQuota*3)/4 )	// but we still didn't make our quota
		{
			if ( pcip->checkedMail &&		// we only count days with mail checks
				 pcip->dayOfWeek >= 0 && 	// yesterday was a valid day
				 pcip->adFaceTimeUsedToday < pcip->faceTimeQuota/2 )	// give 'em half a quota to take the
				 														// heat off our ability to do exact
				 														// accounting, etc.  Better that they
				 														// don't see as many ads as we'd like
				 														// rather than we pester them to death
				 														// because of some minor failur of ours
			{
				pcip->adFailure++;  // we are sad
				if ( pcip->adFailure > 2 ) pcip->adsFailing = true;	// things getting worse
				if ( pcip->adFailure > 127 )
					pcip->adFailure = 127;	// seven bits should do it
			}
		}
		else if ( pcip->adFaceTimeUsedToday > kQuotaSlop )	// don't credit days when the pgm isn't used
															// at least for a minute
		{
			// we are happy!
			pcip->adFailure -= 2;	// add two days to user's happy period
			
			// Not too happy now...
			if ( pcip->adFailure < 0 )
				pcip->adFailure = 0;
			
			// But if we were, like totally bummed, man, then make it 
			// significantly happier.
			if ( pcip->adFailure >= kAdFailureGraceDays )
				pcip->adFailure = kAdFailureGraceDays - 2;
		}
		SDTrace5( "adFace %s ttFace %s quota %s; adFailure %d->%d",
			S2D(pcip->adFaceTimeUsedToday), S2D(pcip->faceTimeUsedToday),
			S2D(pcip->faceTimeQuota), oldAdFailure, pcip->adFailure );
		(*pcip->userProc)( PLIST_SETADFAILURE, 0, pcip->adFailure );
		
		// Clear the counters for the new day
		pcip->faceTimeUsedToday = 0;
		pcip->adFaceTimeUsedToday = 0;
		pcip->dayOfWeek = day;
		pcip->checkedMail = 0;
		
		// Save last week's value
		if (day>=0)
			pcip->faceTimeAWeekAgo = pcip->faceTimeWeek[day];

		// I feel so dirty
		pcip->dirty = true;
		
		// Reset per-day counters in the playlists
		PlistCInfoResetDayStuff(pcip);
	}
}

////////////////////////////////////////////////////////////////
// Reset shown counters for ads
void PlistCInfoResetDayStuff(PlistCInfoPtr pcip)
{
	ForEntriesDo( pcip->dbRef, PlistCInfoResetDayProc, 0, 0, 0, 0 );
}


int PlistCInfoResetDayProc(void* /*dbRef*/, PlaylistRecPtr plrp, EntryRecPtr erp, PlaylistRecPtr /*plrpWas*/, EntryRecPtr /*erpWas*/, void* /*userRef*/ )
{
	Entry entry;
	
	if ( erp->shown || erp->shownToday )
	if ( !ReadEntry( plrp, erp, &entry ) )
	{
		// if it's a runout and was shown for part of its time, now record a full showing
		if ( erp->type==kRunout && erp->shownToday && erp->shownFor < erp->showForMax )
			entry.shownFor += entry.showFor;
			
		// this entry needs to be reset & recorded
		SDTrace2("New day reset %s shown %s->0",entry.entryID,S2D(entry.shown));
		entry.shown = 0;
		entry.shownToday = 0;
		WriteEntry ( plrp, erp, &entry );
		EntryRecEntryCpy( erp, &entry );
	}
	return 1;
}

////////////////////////////////////////////////////////////////
// Commit facetime now
void PlistCInfoFaceTimeRoll(PlistCInfoPtr pcip)
{
	if (!pcip || !pcip->fmb) return;
	
	time_t faceTime = 0;
	
	// Suck the facetime out
	FaceMeasureReport( pcip->userProc, pcip->fmb, &faceTime, NULL, NULL, NULL );	
	
	// Did we get any?
	if (faceTime)
	{
		// Start the counting over
		FaceMeasureReset( pcip->userProc, pcip->fmb );
		FaceMeasureBegin( pcip->userProc, pcip->fmb );

		// Accumulate facetimes
		pcip->faceTimeUsedToday += faceTime;
		if (pcip->adShowing)
			pcip->adFaceTimeUsedToday += faceTime;
		
		// The face time for today is the bigger of
		// today's facetime and last week's facetime
		if (pcip->faceTimeUsedToday > pcip->faceTimeWeek[pcip->dayOfWeek])
			pcip->faceTimeWeek[pcip->dayOfWeek] = pcip->faceTimeUsedToday;
		
		// I feel so dirty...
		pcip->dirty = true;
	}
	
	SDTrace3("FaceTimeRoll: %s ads %s total %s",
		S2D(faceTime),
		S2D(pcip->adFaceTimeUsedToday),
		S2D(pcip->faceTimeUsedToday));
}


////////////////////////////////////////////////////////////////
// Here is where we reinvent the facetime wheel.
FMBHANDLE NewFaceMeasure(plist_callback pfn)
{
	FMBHANDLE fmb;
	// Pretty crude way we have to do things...
	pfn(PLIST_FMBNEW,(long)&fmb,0);
	return fmb;
}

void DisposeFaceMeasure(plist_callback pfn, FMBHANDLE fmb)
{
	// Pretty crude way we have to do things...
	pfn(PLIST_FMBDISP,(long)fmb,0);
}


void FaceMeasureReset(plist_callback pfn, FMBHANDLE fmb)
{
	// Pretty crude way we have to do things...
	pfn(PLIST_FMBRESET,(long)fmb,0);
}

void FaceMeasureBegin(plist_callback pfn, FMBHANDLE fmb)
{	// Pretty crude way we have to do things...
	pfn(PLIST_FMBBEGIN,(long)fmb,0);
}

int FaceMeasureReport(plist_callback pfn, FMBHANDLE fmb, time_t *faceTime, time_t* /*rearTime*/, 
		time_t* /*connectTime*/, time_t* /*totalTime*/ )
{	// Pretty crude way we have to do things...
	pfn(PLIST_FMBREPORT,(long)fmb,(long)faceTime);
	return 0;
}



