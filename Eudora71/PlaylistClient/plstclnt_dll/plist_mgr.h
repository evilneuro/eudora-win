// plist_mgr.h -- the playlist manager. user level access and scheduling.
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

#ifndef _PLIST_MGR_H_
#define _PLIST_MGR_H_

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

// How long is the grace period?
#define kAdFailureGraceDays (14)

// this ain't too exciting right now...
typedef enum EntryType {
    kAd,
    kRunout,
    kSponsor,
    kButton,
    kDeadEntry,
} EntryType;

// stuff the user might do to ads
typedef enum TweakType
{
	kTweakUserDelete,
	kTweakUserRestore,
	kTweakLimit
} TweakType;

// user-provided callback func. "lParam" varies by command---for
// "MGR_SHOWTHIS," it is ptr to an ENTRY_INFO struct.
typedef int (* plist_callback)(int command, long userRef, long lParam);

enum MGR_COMMAND {
    PLIST_BLANK,       // nothing to see, blank the screen (window)
    PLIST_SHOWTHIS,    // url to be loaded, and displayed
    PLIST_FADEIN,      // TODO: this is lame, i think
    PLIST_FADEOUT,     // called between showings
    PLIST_DELETE,      // called to take down a toolbar or sponsor ad
    PLIST_GET_INI_STRING,	// callback to get an INI string
	PLIST_SET_PROFILEID,    // callback to set the profileid
    PLIST_DELETE_PROFILEID,	// callback to delete the profileid.  Could have been just a SET_INI, but prefer specific callback
	PLIST_SET_CLIENTMODE,   // callback to set the clientmode from the server
	PLIST_NAG_USER,         // callback to display a nag dialog
    PLIST_SETADFAILURE,	// stash our ad failure thing in a protected spot
    // Our code is "structured" so "well" that we have to reinvent
    // the face measurement API here.  Wonderful.
    PLIST_FMBNEW,
    PLIST_FMBDISP,
    PLIST_FMBBEGIN,
    PLIST_FMBRESET,
    PLIST_FMBREPORT,
    PLIST_MONITOR,		// For debugging
    PLIST_LOG			// For debugging
};

enum LH_COMMAND	 {
	PLIST_ADD_AD_LINK
};

typedef struct ENTRY_INFO
{
	EntryType entry_type;  // unused, for now
	char* mime_type;   // text/html, image/jpg, etc.
	char* id;          // global unique id of playlist entry
	char* title;       // title text for entry (put in "link window")
	char* src;         // source url of entry; usually a local path
	char* href;        // target url; for when the user clicks
	int   width;       // width of item in pixels
	int   height;      // height of item in pixels

} ENTRY_INFO;

typedef struct NAG_INFO
{
	int   level;   // 0=simple nag, 1=downgrade dlg
	char* text;    // the message text (only for level=0?)

} NAG_INFO;

// call this to open database files, provide callback ptr. "userRef"
// is your own little piece of whatever.
long MGR_Init( const char* dbName, const char* workingDir, const char *subDir, long userRef, plist_callback pfn, int clientMode, const int adFailure );

// synchronus fetching of playlists [TODO: is this needed?]
void MGR_UpdatePlaylists( long dbRef, bool forceUpdate );

// start processing the playlists. callbacks commence at this point---as
// long as "MGR_Idle" is getting called. suspend, resume, and cancel
// do nothing until this is called.
void MGR_BeginScheduling( long dbRef );

// cancels the current selection. this does not suspend scheduling, it
// only causes remaining time for the current item to be skipped. In the
// short term, at least, i don't think we really need this.
void MGR_Cancel( long dbRef );

// call this to shutdown the database. forget to do it and you might
// lose state information. only MGR_Init has any meaning after this.
void MGR_Shutdown( long dbRef );

// call when there's time. the manager runs on the caller's thread,
// except during some http operations. if this doesn't get called
// playlist scheduling will *not* happen.
void MGR_Idle( long dbRef, bool bCanSwitchAd, bool bConnected, int clientMode );

// Are we a happy ad camper?
bool MGR_GetClientUpdateStatus( long dbRef, time_t* lastUpdate );

// use to get rid of ENTRY_INFO block passed with PLIST_SHOWTHIS
void MGR_DisposeEntryInfo( ENTRY_INFO* pei );

// Let the ads know that mail is being checked
void MGR_CheckingMail( long dbRef );

// Dump the ad state to the debug log
void MGR_DumpState( long dbRef );

// Return ads of various types to the client
void MGR_UserTweaked( long dbRef, const char *entryID, TweakType tweak );

// semi-private stuff.  Should be used only by playlist routines

// remove a playlist without knowing the file handle
int mgr_RemovePlaylist( long dbRef, int hpl );

#ifdef __cplusplus
}
#endif

#endif    // _PLIST_MGR_H_
