// plist_cinfo.h -- ClientInfo handling for the playlist manager
//
// Copyright (c) 1999 by QUALCOMM, Incorporated
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

#ifndef _PLIST_CINFO_H_
#define _PLIST_CINFO_H_

#include <afx.h>

#include "plist_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "plist_fast.h"

typedef struct PlistCmd* PlistCmdPtr;
typedef struct PlistCInfoStruct *PlistCInfoPtr;

// Reinvention-of-the-wheel declarations because we
// can't seem to get at facetime cleanly
typedef void *FMBHANDLE;
FMBHANDLE NewFaceMeasure(plist_callback pfn);
void DisposeFaceMeasure(plist_callback pfn,FMBHANDLE fmb);
void FaceMeasureBegin(plist_callback pfn,FMBHANDLE fmb);
void FaceMeasureReset(plist_callback pfn,FMBHANDLE fmb);
int FaceMeasureReport(plist_callback pfn, FMBHANDLE fmb, time_t *faceTime, time_t *rearTime, 
		time_t *connectTime, time_t *totalTime );
		
#define kMaxPastryLen 1024

//
// Oooo, we can make an enum
//
typedef enum {
	PlistCInfoSchedLinear,	// Show the ads in order
	PlistCInfoSchedRandom	// Show the ads at random
} PlistCInfoSchedEnum;

//
// Basic ClientInfo structure
//
typedef struct PlistCInfoStruct
{
	//
	// public stuff, you can look at these values but don't touch
	//
	
	// facetimes
	time_t faceTimeUsedToday;
	time_t adFaceTimeUsedToday;
	time_t faceTimeWeek[7];
	time_t faceTimeQuota;
	
	// ad size
	int adWindowHeight;
	int adWindowWidth;
	
	// playlist fetch info
	time_t lastFetchTime;
	time_t lastUpdateTime;
	time_t reqInterval;

	// scheduling
	PlistCInfoSchedEnum scheduleType;
	time_t historyLength;
	
	// mode the client is in
	// this is just a convenient place to stash
	// it, we aren't actually the ones doing the
	// remembering of it
	int clientMode;
	
	// Whatever we were showing last
	int lastPlaylistID;
	int lastServerID;
	int lastEntryID;
	
	// How long can ads be rerun for?
	time_t rerunInterval;
	
	// How many days of ad failure have there been?
	int adFailure;
	
	// Some flags
	int checkedMail:1;
	int adsFailing:1;
	int spareFlags:30;
	
	//
	// private stuff, pay no attention to the man behind the curtain
	//
	
	// we put the spare stuff in between public and private so
	// we can steal it for either direction
	int spareStuff[29];

	// storage for "nag" stuff (processed like a "command")
	int* nagInfo;

	// new profile id (processed like a "command")
	char* profile;

	// commands (e.g. "flush")
	PlistCmdPtr commands;
	
	void *dbRef;        // db reference, need for new day reset
	
	int version;  		// version number for the structure
	
	int dayOfWeek;		// weekday we're working on; keep it handy
	
	time_t faceTimeAWeekAgo;	// keep the value from a week ago
								// because we want to report the
								// bigger of this week's value and last
								// week's value.
	
	// I/O stuff goes here
	char *path;				// read/write our data here
	bool dirty;				// do we need to save?
	time_t lastSaveTime;	// When did we save our data last?
	
	// building blocks of facetime measurements
	FMBHANDLE fmb;		// facetime measurement
	bool adShowing;			// is the ad showing?
	
	// I wouldn't have to do this if this code wasn't so well "structured"
	plist_callback userProc;
	
	// The cookie
	char pastry[kMaxPastryLen];

} PlistCInfo ;

//
// Routines for startup, shutdown, load, save, etc
//

// Open or create a new one
PlistCInfoPtr PlistCInfoNew(const char *fName,const char *dirName, plist_callback pfn, void *dbRef, const int adFailure );

// Save the current data
int PlistCInfoSave(PlistCInfoPtr pcip);

// Close the database
void PlistCInfoDispose(PlistCInfoPtr pcip);

//
// Here's an idle routine, because we like idle routines
//
void PlistCInfoIdle(PlistCInfoPtr pcip);

//
// Global facetime management stuff
//

// Accumulate facetime for app and for ads
void PlistCInfoAdStart(PlistCInfoPtr pcip);
void PlistCInfoAdEnd(PlistCInfoPtr pcip);

//
// A call to tell us Stuff
// If you make pciSetp NULL, it blows its mind entire; use for reset
//
int PlistCInfoSet(PlistCInfoPtr pcip,PlistCInfoPtr pciSetp,int validFlags);
#define PLCIfaceTimeQuotaValid	(1<<0)
#define	PLCIadWindowHeightValid	(1<<1)
#define	PLCIadWindowWidthValid	(1<<2)
#define	PLCIlastFetchTimeValid	(1<<3)
#define	PLCIlastUpdateTimeValid	(1<<4)
#define	PLCIscheduleTypeValid	(1<<5)
#define PLCIhistoryLengthValid	(1<<6)
#define PLCIreqIntervalValid	(1<<7)
#define PLCIpastryValid			(1<<8)
#define PLCIcommandsValid       (1<<9)
#define PLCIclientModeValid     (1<<10)
#define PLCIlastPlaylistIDValid (1<<11)
#define PLCIlastServerIDValid   (1<<12)
#define PLCIlastEntryIDValid    (1<<13)
#define PLCIrerunIntervalValid	(1<<14)
#define PLCIcheckedMailValid	(1<<15)
#define PLCIprofileValid        (1<<16)
#define PLCInagInfoValid        (1<<17)

void PlistCInfoDump(PlistCInfoPtr pcip, CString &output);

#ifdef __cplusplus
}
#endif

#endif    // _PLIST_CINFO_H_
