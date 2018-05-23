// plist_stg.h
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

#ifndef _PLIST_STG_H_
#define _PLIST_STG_H_

#include "plist_mgr.h"

// anything we write to file is part of a "Node". this is the basic plumbing
// for linked structures, although currently we just read/write everything
// sequentially. this may be overkill, but i don't really want to support
// a separate file for every playlist.

struct Node {
    int nodeType;
    int size;

    long nextNode;    // ptr: file *or* memory
};


// Node macros
#define node(p) ((Node*)p);
#define nextnode(p) ((Node*)((Node*)p)->nextNode)
#define set_nextnode(p, n) (((Node*)p)->nextNode = (long)n)
#define nodetype(p) ((int)((Node*)p)->nodeType)
#define set_nodetype(p, t) (((Node*)p)->nodeType = t)
#define nodesize(p) ((int)((Node*)p)->size)
#define set_nodesize(p, s) (((Node*)p)->size = s)
#define nodeid(p) (((Node*)p)->nextNode - nodesize(p))


// this is the playlist header. it contains all the values passed in the XML
// playlist, excluding the entries.

struct PlaylistHeader : public Node {
    int    type;
    int    numEntries;
    char   playlistID[256];
    char   lastModified[64];
};

// here's the entry, with fields for each XML entity contained within the
// "entry" entity.

struct Entry : public Node {
    char   entryID[240];
    int	   shownToday;			// how much facetime today	// not in XML
    time_t lastTime;			// last time we showed it	// not in XML
    int	   userDeleted:1;		// has the user deleted btn	// not in XML
    int    spareFlags:31;		// not in XML
    EntryType	type;
	char   title[128];
    char   src[1024];
	char   href[1024];
	int    width;
	int    height;
    int    showFor;
	int    showForMax;
	int    dayMax;
	int    blackBefore;
	int    blackAfter;
	char   startDT[32];
	char   endDT[32];
	char   nextEntry[223];
	char   checksum[33];

// state -- not part of XML definition

    int    shown;
    long   shownFor;
};

#define RealEntry( pe )	((pe) && *(pe)->entryID!=0)
#define SameEntry( seq1, pe1, seq2, pe2 )	((seq1)==(seq2) && !strcmp((pe1)->entryID,(pe2)->entryID))

#ifdef __cplusplus
extern "C" {
#endif

enum NodeType {
    kPlaylistHeader,
    kEntry,
	kDead
};

int CommitPlaylist( PlaylistHeader** ppHeader );
int OpenPlaylist( const char* dbName, bool bWrite );
void ClosePlaylist( int desc );
void DeletePlaylist( int fd );
int GetFirstEntry( int hPlaylist, Entry* entry );
int SetToZerothEntry( int hPlaylist );
int GetNextEntry( int hPlaylist, Entry* entry, bool bWrap );
int GetEntryAfter( int hPlaylist, Entry* entry, bool bWrap );
int GetFirstPlaylist( int desc, long* seq, char *playlistID );
int GetNextPlaylist( int desc, long* seq, char *playlistID, bool bWrap );
int GetEntry( int hPlaylist, long id, Entry* entry );
int SetEntry( int hPlaylist, Entry* entry );
int DbInit( const char* dbName, const char* workingDir, const char *subDir );
void DbShutdown( int desc );
int DbAddPlaylist( int dbh, int plh );
int DbRemovePlaylist( int dbh, int plh );
int GetPlaylistHeader( int hPlaylist, PlaylistHeader* header );

// Linked-list macros, lifted from Mac Eudora (sorry...)
#ifndef LL_Remove
typedef unsigned long uLong;
#define LL_Remove(head,item,type) 										\
	do { uLong M_T1;													\
	if (head==item) head = (*head)->next; 								\
	else																\
		for (tmp=(uLong)head; tmp; tmp=(uLong)((type*) tmp)->next)		\
		{ 																\
			if (((type*)tmp)->next == item) 							\
			{ 															\
				((type*)tmp)->next = (item)->next; 						\
				break;													\
			} 															\
		} 																\
	} while (0)

#define LL_Push(head,item)												\
	(((item))->next = head, head = item)
	
#define LL_Queue(head,item,type)									\
	do {															\
		void *t = head;												\
		if (head)													\
		{															\
			while (((type*)t)->next) t = ((type*)t)->next;			\
			((type*)t)->next = item;								\
		}															\
		else														\
			head = item;											\
	} while (0)

#define LL_Last(head,item)											\
	do {															\
		item = head;												\
		while (((item))->next) item = ((item))->next;				\
	} while (0)

#define LL_Parent(head,item,parent)									\
	do {															\
		parent = head;												\
		while (parent && (parent)->next!=item)						\
			parent = ((parent))->next;								\
	} while (0)
#endif	// LL_Remove

typedef struct PlaylistRec *PlaylistRecPtr;
typedef struct EntryRec *EntryRecPtr;

PlaylistRecPtr BuildDBLists( int dbh );
EntryRecPtr BuildEntryIndex( PlaylistRecPtr plrp );
PlaylistRecPtr FindPlaylistInt( PlaylistRecPtr headPLR, int playlistID );
PlaylistRecPtr FindPlaylist( PlaylistRecPtr headPLR, const char *plID );
EntryRecPtr FindEntry( PlaylistRecPtr plr, int serverID, int entryID );
int ReadEntry( PlaylistRecPtr plrp, EntryRecPtr erp, Entry *entryP );
int WriteEntry( PlaylistRecPtr plrp, EntryRecPtr erp, Entry *entryP );
int ReadEntryID( PlaylistRecPtr headPLRP, int playlistID, int serverID, int entryID, Entry *entryP );
int WriteEntryID( PlaylistRecPtr headPLRP, int playlistID, int serverID, int entryID, Entry *entryP );
void DisposeDBLists( PlaylistRecPtr plrp );
void DisposeEntryList( EntryRecPtr erp );
void EntryRecEntryCpy( EntryRecPtr erp, Entry *entry );

struct PlaylistRec : Node {
	char plID[128];				// Playlist ID, can derive filename
	long seq;					// sequence number, so we can find the next db
	EntryRecPtr entries;		// List of Entry headers, can browse entries
	PlaylistRecPtr next;		// Next one in list
};

struct EntryRec : Node {
	// database management stuff
	int				serverID;	// use number for compactness
	int				entryID;	// use number for compactness
	EntryRecPtr		next;		// next one in list
	
	// ad scheduling stuff, same as in Entry
    int		showFor;
	int		showForMax;
	int		dayMax;
	time_t	startTime;
	time_t	endTime;
	time_t	lastTime;
	int		shownFor;
	int		shown;
	int		userDeleted:1;
	int		oldNCrufty:1;
	int		spareFlags:30;
	EntryType	type;
	int		blackBefore;
	int		blackAfter;
	int		shownToday;
	int		chkSumFailures;
};

#ifdef __cplusplus
}
#endif    // External "C" linkage

#endif    // _PLIST_STG_H_
