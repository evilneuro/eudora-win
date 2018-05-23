// plist_stg.cpp
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

#include "afx.h"

#include "stdlib.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "errno.h"
#include "fcntl.h"
#include "memory.h"
#include "string.h"
#include "time.h"
#include "assert.h"
#include <direct.h>

#ifdef WIN32
#include "io.h"
#define ACCESS_FLAGS_READ O_BINARY | O_RDONLY
#define ACCESS_FLAGS_WRITE O_BINARY | O_RDWR
#else
#include "unistd.h"
#define ACCESS_FLAGS_READ O_RDONLY
#define ACCESS_FLAGS_WRITE O_RDWR
#endif

#include "plist_debug.h"
#include "plist_stg.h"

#define MACAFEE_SUCKS ".mfs"

#include "DebugNewHelpers.h"

static char g_workingDir[_MAX_PATH + 1];
static int open_dbase_file( const char* fileName, bool bWrite );
void remove_dbase_file( const char* fileName );
bool is_valid_playlist( char* id );
char* playlist_filename( char* id );
time_t interpret_dt( const char *dt );
int make_subdir( const char *workingDir, const char *subDir );
int move_db_files( const char *dbName, const char *origDir );
int file_move_and_munge( const char *filename, const char *origDir, const char *newDir, const char *newExtension );

///////////////////////////////////////////////////////////////////////////////
// Public routines

// CommitPlaylist -- takes a Playlist in memory, creates a file, and writes
// the Playlist out. the memory Playlist is consumed. the out-ptr will be
// set to NULL. all access to the list from that point will be through the
// descriptor that is returned.

// TODO: *** is it possible for db to exist? OpenPlaylist doesn't trunc ***

int CommitPlaylist( PlaylistHeader** ppHeader )
{
    int ret = 0;
    PlaylistHeader* pH = *ppHeader;
    int hpl = OpenPlaylist( pH->playlistID, true );

    if ( hpl != -1 ) {
        int nBytes = 0;
        Node* pN = node(pH);
		Node* tmp = 0;

        for ( ; pN; ) {
			tmp = nextnode(pN);
			pN->nextNode = tell( hpl ) + nodesize(pN);
            nBytes = write( hpl, pN, nodesize(pN) );
			pN->nextNode = (long)tmp;

            if ( nBytes < nodesize(pN) || nBytes == -1 ) {
                close( hpl );
                ret = -1;
                break;
            }
            else
                pN = nextnode(pN);
        }

        if ( !ret ) {
            pN = node(pH);
            Node* pT = NULL;

            for ( ; pN; ) {
                pT = nextnode(pN);
                delete pN;
                pN = pT;
            }

            *ppHeader = NULL;
            ret = hpl;
        }
    }

    return hpl;
}

#ifdef PLIST_LOGGING
int g_OpenPlaylists;
#endif

int OpenPlaylist( const char* plstID, bool bWrite )
{
	if (plstID && *plstID)
	{
		char fileName[_MAX_PATH + 1];
		const char Extension[] = MACAFEE_SUCKS;
		const int MaxBase = sizeof(fileName) - sizeof(Extension) - 1;

		strncpy(fileName, plstID, MaxBase);
		fileName[MaxBase] = 0;
		strcat(fileName, fileName[strlen(fileName) - 1] == '.'? Extension + 1 : Extension);

		int err = open_dbase_file( fileName, bWrite );
		
		if (err<0 || g_OpenPlaylists > 20) SDTrace3("OpenPlaylist %s %d (%d)", fileName, err, ++g_OpenPlaylists);
		
		return err;
	}

	return -1;
}

void ClosePlaylist( int fd )
{
	if (g_OpenPlaylists > 20) SDTrace2( "ClosePlaylist %d (%d)\n", fd, g_OpenPlaylists );
	g_OpenPlaylists--;
	close( fd );
}

void DeletePlaylist( int fd )
{
	PlaylistHeader ph;
	if ( GetPlaylistHeader( fd, &ph ) == 0 ) {
		char fileName[_MAX_PATH + 1];
		const char Extension[] = MACAFEE_SUCKS;
		const int MaxBase = sizeof(fileName) - sizeof(Extension) - 1;

		strncpy(fileName, ph.playlistID, MaxBase);
		fileName[MaxBase] = 0;
		strcat(fileName, fileName[strlen(fileName) - 1] == '.'? Extension + 1 : Extension);

		ClosePlaylist( fd );
		remove_dbase_file( fileName );
	}
}

// TODO: this is kinda broke now that multi-plist is enabled. it is only sanity-
// checking the first idx entry. since we're about to make big changes to the idx
// data structures, we'll just live with it for now.
int DbInit( const char* dbName, const char* workingDir, const char *subDir )
{
	if ( make_subdir( workingDir, subDir ) ) return -1;
	if ( move_db_files( dbName, workingDir ) ) return -1;
	
    char fileName[_MAX_PATH + 1];
    sprintf( fileName, "%s.idx", dbName );

	int fdb;
	if ( (fdb = open_dbase_file( fileName, true )) != -1 ) {
		PlaylistHeader ph;
		// TODO: if first entry is dead, go to next!
		if ( GetPlaylistHeader( fdb, &ph ) == 0 ) {
			bool bValid = (nodetype(&ph) == kPlaylistHeader) &&
				(nodesize(&ph) == sizeof(PlaylistHeader)) &&
				*ph.playlistID &&
				is_valid_playlist(ph.playlistID);

			if ( !bValid ) {
				DbShutdown( fdb );
				remove_dbase_file( fileName );
				fdb = open_dbase_file( fileName, true );
			}
		}
	}

	return fdb;
}

// make our subdirectories, if we need to make them
int make_subdir( const char *workingDir, const char *subDir )
{
	char tmp[_MAX_PATH+1];
	char *token;
	
	strcpy( g_workingDir, workingDir );
	if ( g_workingDir[strlen(g_workingDir)-1] == '/' )
		g_workingDir[strlen(g_workingDir)-1] = '\\';
	strcpy( tmp, subDir );
	for ( token=strtok(tmp,"/\\"); token; token=strtok(NULL,"/\\") )
		if (*token)
		{
			if ( g_workingDir[strlen(g_workingDir)-1] != '\\' )
				strcat( g_workingDir, "\\" );
			strcat( g_workingDir, token );
			mkdir( g_workingDir );
		}
	
	if ( g_workingDir[strlen(g_workingDir)-1] != '\\' )
		strcat( g_workingDir, "\\" );
	
	// mkdir doesn't return what the dox say it returns,
	// so we can't check meaningfully for errors yet
	return 0;
}

// move files into our subdirectories, if we need to do so
int move_db_files( const char *dbName, const char *origDir )
{
	// move the idx file, if it's there
	char tmp[_MAX_PATH+1];
	strcpy( tmp, dbName );
	strcat( tmp, ".idx" );
	int err = file_move_and_munge( tmp, origDir, g_workingDir, NULL );
	if ( err && errno==ENOENT ) err = 0;
	if ( err ) return -1;
	// strictly speaking, these don't belong here, but...
	err = file_move_and_munge( "CInfo.dat", origDir, g_workingDir, NULL );
	if ( err && errno==ENOENT ) err = 0;
	if ( err ) return -1;
	err = file_move_and_munge( "AdCache", origDir, g_workingDir, NULL );
	if ( err && errno==ENOENT ) err = 0;
	if ( err ) return -1;
	
	// find and move the bin files
	WIN32_FIND_DATA findData;
	char starDotBin[_MAX_PATH+1];	strcpy( starDotBin, origDir );
	strcat( starDotBin, "*.bin" );
	HANDLE h = FindFirstFile( starDotBin, &findData );
	if (INVALID_HANDLE_VALUE != h)
	{
		do
		{
			// Skip directories
			if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				continue;
			
			// is it a filename we expect?  Maybe weirdo user has precious files
			// named "whatever.bin" in directory; try to avoid them
			if ( strspn( findData.cFileName, "0123456789" ) == strlen( findData.cFileName )-4 )	// our .bin files are numeric
				// Ok, the filename matches.  Rename it with our new extension
				err = file_move_and_munge( findData.cFileName, origDir, g_workingDir, MACAFEE_SUCKS );
		}
		while ( !err && FindNextFile(h,&findData) );
		FindClose(h);
	}
	
	return err;
}

// move a file, possibly munging its name
int file_move_and_munge( const char *filename, const char *origDir, const char *newDir, const char *newExtension )
{
	char oldPath[_MAX_PATH+1];
	char newPath[_MAX_PATH+1];
	
	// build the old path
	strcpy( oldPath, origDir );
	strcat( oldPath, filename );
	
	// build the new path
	strcpy( newPath, newDir );
	strcat( newPath, filename );
	if ( newExtension )
	{
		char *dot = strrchr( newPath, '.' );
		char *bslash = strrchr( newPath, '\\' );
		if ( dot && dot > bslash )
			strcpy( dot, newExtension );
	}
	
	// if the new path already exists, we're done
	if ( _access( newPath, 0) == 0)
		return 0;

	// and now, do the rename
	return rename( oldPath, newPath );
}

// Get rid of all idx entries marked as "dead"
//
// TODO: rewriting the idx file in-place like this is pretty lame, but i've
// painted myself into a corner, and don't have the information i need to do a
// rename operation! need to switch from using os file handle for db handle, to
// our own opaque handle---after i get the current multi-plist stuff checked-in.
void DbShutdown( int desc )
{
	long nxt = 0;
	PlaylistHeader ph;

	lseek( desc, nxt, SEEK_SET );

	while ( read( desc, &ph, sizeof(PlaylistHeader) ) == sizeof(PlaylistHeader) ) {
		if ( nodetype(&ph) == kDead ) {
			char* filename = playlist_filename( ph.playlistID );
			remove_dbase_file( filename );
			delete [] filename;
		}
		else {
			long curPos = tell( desc );
			lseek( desc, nxt, SEEK_SET );
			write( desc, &ph, sizeof(PlaylistHeader) );
			nxt = tell( desc );
			lseek( desc, curPos, SEEK_SET );
		}
	}

	chsize( desc, nxt );
    close( desc );
}

// just slap it on at the end
// TODO: need a way to get rid of these
int DbAddPlaylist( int dbh, int plh )
{
	PlaylistHeader ph;
	GetPlaylistHeader( plh, &ph );

	lseek( dbh, 0, SEEK_END );
	write( dbh, &ph, sizeof(PlaylistHeader) );
	return 0;
}

int DbRemovePlaylist( int dbh, int plh )
{
	int ret = -1;
	PlaylistHeader ph, idxEntry;

	long curPos = tell( dbh );
	lseek( dbh, 0, SEEK_SET );

	if ( plh == -1 ) {
		for ( ;; ) {
			if ( read( dbh, &idxEntry, sizeof(PlaylistHeader) )
					== sizeof(PlaylistHeader) ) {

				set_nodetype(&idxEntry, kDead);
				lseek( dbh, -nodesize(&idxEntry), SEEK_CUR );
				write( dbh, &idxEntry, nodesize(&idxEntry) );
			}
			else
				break;
		}

		ret = 0;
	}
	else {
		// find the matching db record, and mark it as dead
		GetPlaylistHeader( plh, &ph );

		for ( ;; ) {
			if ( read( dbh, &idxEntry, sizeof(PlaylistHeader) )
					== sizeof(PlaylistHeader) ) {

				if ( strcmp( idxEntry.playlistID, ph.playlistID ) == 0 ) {
					set_nodetype(&idxEntry, kDead);
					lseek( dbh, -nodesize(&idxEntry), SEEK_CUR );
					write( dbh, &idxEntry, nodesize(&idxEntry) );
					ret = 0;
					break;
				}
			}
			else
				break;
		}
	}

	lseek( dbh, curPos, SEEK_SET );
	return ret;
}

int GetFirstPlaylist( int desc, long* seq, char *playlistID )
{
	*seq = 0;
	return GetNextPlaylist( desc, seq, playlistID, false );
}

int GetNextPlaylist( int desc, long* seq, char *playlistID, bool bWrap )
{
	long offset = *seq;
	const int maxpass = bWrap ? 2 : 1;

	// if "bWrap" is true this might take two passes
	for ( int i = 0; i < maxpass; i++ ) {
		if ( lseek( desc, offset, SEEK_SET ) == offset ) {
			PlaylistHeader ph;
			int bytes;

			if ( (bytes = read( desc, &ph, sizeof(PlaylistHeader) )) == -1 )
				break;

			if (  bytes == sizeof(PlaylistHeader) ) {
				*seq = tell( desc );
				if ( playlistID ) strcpy( playlistID, ph.playlistID );
				if ( nodetype( &ph ) == kDead ) return -2;
				return OpenPlaylist( ph.playlistID, false );
			}

			// we should not be hitting eof with leftovers
			assert( bytes == 0 );

			// might be wrapping
			offset = 0;
		}
		else
			break;
	}

	*seq = -1;
	return -1;
}

int GetPlaylistHeader( int hPlaylist, PlaylistHeader* header )
{
	if ( lseek( hPlaylist, 0, SEEK_SET ) == 0 )
		if ( read( hPlaylist, header, sizeof(PlaylistHeader) ) == sizeof(PlaylistHeader) )
			return 0;

	SDTraceLastError(-1);

	return -1;
}

// iterator functions. these just assume that the files are
// sequential and flat. later i'll rewrite these to follow
// the node ptrs (unless i don't ;-)

int SetToZerothEntry( int hPlaylist )
{
	if ( lseek( hPlaylist, sizeof(PlaylistHeader), SEEK_SET ) == sizeof(PlaylistHeader) )
		return 0;
	return -1;
}


int GetFirstEntry( int hPlaylist, Entry* entry )
{
	if ( SetToZerothEntry(hPlaylist) ) return -1;
	// read-up the first node following the header
	return GetNextEntry( hPlaylist, entry, false );
}

// Like GetNextEntry, but seeks the file pointer to after
// the entry that was passed in, to make sure that nothing
// bad has happened to it since we last set it.
int GetEntryAfter ( int hPlaylist, Entry* entry, bool bWrap )
{
	int err;

	if ( RealEntry(entry) )
	{
		long offset = (long) nextnode( entry );
		if ( offset && ( offset != lseek( hPlaylist, offset, SEEK_SET ) ) )
			return -1;
		err = GetNextEntry ( hPlaylist, entry, bWrap );
	}
	else 
		err = GetFirstEntry ( hPlaylist, entry );
	
	if ( !err && !RealEntry ( entry ) ) err = -1;	// bogus stuff
	
	return err;
}

int GetNextEntry( int hPlaylist, Entry* entry, bool bWrap )
{
	const int NumBytesRead = read(hPlaylist, entry, sizeof(Entry));

	if (NumBytesRead >= 0)
	{
		if (NumBytesRead == sizeof(Entry))
			return 0;
		if ( bWrap )
			return GetFirstEntry( hPlaylist, entry );
	}

	return -1;
}

int GetEntry( int hPlaylist, long id, Entry* entry )
{
	long curpos = tell( hPlaylist );

	if ( curpos >= 0 &&
		 lseek( hPlaylist, id, SEEK_SET ) == id &&
		 read(hPlaylist, entry, sizeof(Entry)) == sizeof(Entry) &&
		 lseek( hPlaylist, curpos, SEEK_SET ) == curpos )
	{
		return 0;
	}

	SDTraceLastError(-1);

	return -1;
}

int SetEntry( int hPlaylist, Entry* entry )
{
	long entrySize = nodesize(entry);
	long nodeID = nodeid(entry);
	long curpos = tell( hPlaylist );

	if ( curpos >= 0 &&
		 lseek( hPlaylist, nodeID, SEEK_SET ) == nodeID &&
		 write( hPlaylist, entry, entrySize ) == entrySize &&
		 lseek( hPlaylist, curpos, SEEK_SET ) == curpos )
	{
		return 0;
	}

	return -1;
}


// if "fileName" doesn't exist, a new file is created
int open_dbase_file( const char* fileName, bool bWrite )
{
	if (!fileName)
		return -1;

	char fileNameBuf[_MAX_PATH + 1];

	strcpy(fileNameBuf, g_workingDir);
	strncat(fileNameBuf, fileName, sizeof(fileNameBuf));
	fileNameBuf[sizeof(fileNameBuf) - 1] = 0;

    struct stat st;
	int err = stat(fileNameBuf, &st);
    int open_flags = err ? ( O_CREAT | ACCESS_FLAGS_WRITE )
    					 : ( bWrite ?  ACCESS_FLAGS_WRITE : ACCESS_FLAGS_READ );

	SDTraceLastError(err);
    if (err) SDTrace1("Creating dbase %s",fileNameBuf);

    err = open( fileNameBuf, open_flags, S_IWRITE );

#ifdef _DEBUG
	if (err<0) SDTraceLastError(err);
    SDTrace3("Open dbase %s %c (%d)",fileNameBuf, bWrite ? 'w':'r', err);
#endif

	return err;
}

void remove_dbase_file( const char* fileName )
{
	if ( !fileName )
		return;

	char fileNameBuf[_MAX_PATH + 1];
	strcpy(fileNameBuf, g_workingDir);
	strncat(fileNameBuf, fileName, sizeof(fileNameBuf));
	fileNameBuf[sizeof(fileNameBuf) - 1] = 0;

	int err = unlink( fileNameBuf );
	
	SDTraceLastError(err);
	SDTrace2("Removing %s %d", fileNameBuf, err);
	
	return;
}


char* playlist_filename( char* id )
{
	if ( !id )
		return 0;

	int bufsiz = _MAX_PATH + 1;
	char* fileName = DEBUG_NEW_NOTHROW char[bufsiz];

	if ( fileName ) {
		const char Extension[] = MACAFEE_SUCKS;
		const int MaxBase = bufsiz - sizeof(Extension) - 1;

		strncpy( fileName, id, MaxBase );
		fileName[MaxBase] = 0;
		strcat( fileName, fileName[strlen(fileName) - 1] == '.' ?
					Extension + 1 : Extension);
	}

	return fileName;
}


bool is_valid_playlist( char* id )
{
	bool ret = false;
	char* fnam = playlist_filename( id );

	if ( fnam ) {
		char buf[_MAX_PATH + 1];
		strcpy( buf, g_workingDir );
		strncat( buf, fnam, sizeof(buf) );
		buf[sizeof(buf) - 1] = 0;

		// there's a lot more things we can do to sanity check a
		// playlist, but for now we'll just be glad it's there.
		ret = (access( buf, 0 ) != -1);
		
#ifdef _DEBUG
		if (!ret) SDTrace1("Missing Playlist %s", buf);
#endif

		delete [] fnam;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Indices

/////////////////////////////////////////
// Build the basic lists
PlaylistRecPtr BuildDBLists( int dbh )
{
	if ( !dbh ) return NULL;	// oops
	
	PlaylistRecPtr headplrp = NULL;
	
	int hpl;
	long seq;
	char plID[128];
	
	// Get{First,Next}Playlist return -1 for end of file, -2 for dead playlist
	hpl = GetFirstPlaylist( dbh, &seq, plID );
	while ( hpl != -1 )
	{
		if ( hpl != -2 )
		{
			assert( atoi(plID) );
			if ( atoi(plID) )
			{
				PlaylistRecPtr plrp = DEBUG_NEW_NOTHROW PlaylistRec;
				if ( plrp )
				{
					memset( plrp, '\0', sizeof( *plrp ) );
					plrp->seq = seq;
					strcpy( plrp->plID, plID );
					plrp->entries = BuildEntryIndex( plrp );
					plrp->next = NULL;
					LL_Queue( headplrp, plrp, PlaylistRec );
				}
			}
			ClosePlaylist( hpl );
		}
		hpl = GetNextPlaylist( dbh, &seq, plID, false );
	}
	
	return headplrp;
}

/////////////////////////////////////////
// Build the basic lists
EntryRecPtr BuildEntryIndex( PlaylistRecPtr plrp )
{
	EntryRecPtr headEntry = NULL;
	
	// open the playlist
	int hpl = OpenPlaylist ( plrp->plID, false );
	
	// set to first entry
	if ( hpl >= 0 && !SetToZerothEntry( hpl ) )
	{
		Entry entry;
		
		// read 'em one by one
		while( !GetNextEntry( hpl, &entry, false ) )
		{
			if ( nodetype( &entry ) != kDead )
			{
				EntryRecPtr erp = DEBUG_NEW_NOTHROW EntryRec;
				if ( erp )
				{
					memset( erp, '\0', sizeof( EntryRec ) );
					
					// Copy info
					EntryRecEntryCpy( erp, &entry );
					
					// Make sure id's are not zero
					assert( erp->serverID );
					assert( erp->entryID );
					if ( erp->serverID==0 || erp->entryID==0 )
					{
						delete erp;
						continue;
					}
					
					// now add to list
					LL_Queue( headEntry, erp, EntryRec );
				}
			}
		}
	}
	
	// close it up
	if ( hpl >= 0 ) ClosePlaylist( hpl );
	
	return headEntry;				
}

/////////////////////////////////////////
// Find a particular playlist record

PlaylistRecPtr FindPlaylistInt( PlaylistRecPtr headPLRP, int playlistID )
{
	char plID[128];
	sprintf(plID, "%d", playlistID );
	return FindPlaylist( headPLRP, plID );
}

PlaylistRecPtr FindPlaylist( PlaylistRecPtr headPLRP, const char *plID )
{
	// walk the in-memory list.  So easy, so fun...
	while( headPLRP )
		if ( !strcmp( headPLRP->plID, plID ) ) break;
		else headPLRP = headPLRP->next;
	
	return headPLRP;
}

/////////////////////////////////////////
// Find a particular entry
EntryRecPtr FindEntry( PlaylistRecPtr plrp, int serverID, int entryID )
{
	// in-memory lists again.  Note that we don't have to error-check
	// the next pointers!  Stupid files.  :-)
	for ( EntryRecPtr erp = plrp->entries;
		  erp;
		  erp = erp->next )
		if ( erp->serverID == serverID )
		if ( erp->entryID == entryID )
			return erp;
	
	// oops
	return NULL;
}

/////////////////////////////////////////
// Read an entry, given its ids
int ReadEntryID( PlaylistRecPtr headPLRP, int playlistID, int serverID, int entryID, Entry *entryP )
{
	// make sure it's in our list of playlists
	if (PlaylistRecPtr plrp = FindPlaylistInt( headPLRP, playlistID ) )
	{
		// make sure the entry is in our list of entries
		if ( EntryRecPtr erp = FindEntry( plrp, serverID, entryID ) )
		{
			return ReadEntry( plrp, erp, entryP );
		}
	}
	
	// failed to find it
	return -2;
}

/////////////////////////////////////////
// Write an entry
int WriteEntryID( PlaylistRecPtr headPLRP, int playlistID, int serverID, int entryID, Entry *entryP )
{
	// make sure it's in our list of playlists
	if (PlaylistRecPtr plrp = FindPlaylistInt( headPLRP, playlistID ) )
	{
		// make sure the entry is in our list of entries
		if ( EntryRecPtr erp = FindEntry( plrp, serverID, entryID ) )
		{
			return WriteEntry( plrp, erp, entryP );
		}
	}
	
	// failed to find it
	return -2;
}

/////////////////////////////////////////
// Read an entry, given an erp
int ReadEntry( PlaylistRecPtr plrp, EntryRecPtr erp, Entry *entryP )
{
	int hpl = OpenPlaylist( plrp->plID, false );
	int ret = -1;
	
	// open the playlist
	if ( hpl >= 0 )
	{
		// read the entry
		ret = GetEntry( hpl, nodeid( erp ), entryP );
		
#ifdef _DEBUG
		char id[32];
		sprintf(id,"%d.%d",erp->serverID,erp->entryID);
		assert(!strcmp(id,entryP->entryID));
#endif

		// close it up, since we opened it
		ClosePlaylist( hpl );
	}
	
	return ret;
}

/////////////////////////////////////////
// Write an entry
int WriteEntry( PlaylistRecPtr plrp, EntryRecPtr /*erp*/, Entry *entryP )
{
	int hpl = OpenPlaylist( plrp->plID, true );
	int ret = -1;
	
	// open the playlist
	if ( hpl >= 0 )
	{
		// read the entry
		ret = SetEntry( hpl, entryP );
		
		// close it up, since we opened it
		ClosePlaylist( hpl );
	}
	
	return ret;
}

/////////////////////////////////////////
// KILL THEM ALL!!!!  THEY MUST DIE!!!!
void DisposeDBLists( PlaylistRecPtr plrp )
{
	PlaylistRecPtr next;
	
	while( plrp )
	{
		next = plrp->next;
		
		DisposeEntryList( plrp->entries );
		delete plrp;
		
		plrp = next;
	}
}

/////////////////////////////////////////
// KILL THEM ALL!!!!  THEY MUST DIE!!!!
void DisposeEntryList( EntryRecPtr erp )
{
	EntryRecPtr next;
	
	while( erp )
	{
		next = erp->next;
		delete erp;
		erp = next;
	}
}

/////////////////////////////////////////
// Copy interesting fields from an entry to
// an EntryRecPtr
void EntryRecEntryCpy( EntryRecPtr erp, Entry *entry )
{
	// copy all this stuff.  Difficult programming, this
	erp->serverID = atoi( entry->entryID );
	char *period = strchr( entry->entryID, '.' );
	erp->entryID = period ? atoi( period+1 ) : 0;
	erp->showFor = entry->showFor;
	erp->showForMax = entry->showForMax;
	erp->dayMax = entry->dayMax;
	erp->startTime = interpret_dt( entry->startDT );
	erp->endTime = interpret_dt( entry->endDT );
	erp->lastTime = entry->lastTime;
	erp->shownFor = entry->shownFor;
	erp->shown = entry->shown;
	erp->userDeleted = entry->userDeleted;
	erp->type = entry->type;
	erp->spareFlags = entry->spareFlags;
	erp->blackBefore = entry->blackBefore;
	erp->blackAfter = entry->blackAfter;
	erp->nodeType = entry->nodeType;
	erp->size = entry->size;
	erp->nextNode = entry->nextNode;
	erp->shownToday = entry->shownToday;
}

/////////////////////////////////////////
// Interpret a date-time
// Simplified version of old Mac Eudora date parser
// assumes RFC-822 compliant dates with no tabs, no comments,
// case changes, four-digit years, etc.
#define nextspace( cp ) do { while ( *cp && *cp!=' ' ) cp++; } while(0)
#define skipspace( cp ) do { while ( *cp==' ' ) cp++; } while(0)
time_t CStr2Zone(const char *s);

time_t interpret_dt( const char *dateStr )
{
	const char *cp;
	struct tm tm;
	
	if (!strlen( dateStr )) return 0;

	memset( &tm, '\0', sizeof(tm) );
	
	/*
	 * skip day of week
	 */
	cp=strchr(dateStr,',');
	if (cp) cp++;
	else cp = dateStr;
	skipspace( cp );
		
	/* get day of month */
	tm.tm_mday = atoi( cp );
	nextspace( cp );
	skipspace( cp );

	/* get month */
	if (!strncmp(cp,"Jan",3)) tm.tm_mon = 0;
	else if (!strncmp(cp,"Feb",3)) tm.tm_mon = 1;
	else if (!strncmp(cp,"Mar",3)) tm.tm_mon = 2;
	else if (!strncmp(cp,"Apr",3)) tm.tm_mon = 3;
	else if (!strncmp(cp,"May",3)) tm.tm_mon = 4;
	else if (!strncmp(cp,"Jun",3)) tm.tm_mon = 5;
	else if (!strncmp(cp,"Jul",3)) tm.tm_mon = 6;
	else if (!strncmp(cp,"Aug",3)) tm.tm_mon = 7;
	else if (!strncmp(cp,"Sep",3)) tm.tm_mon = 8;
	else if (!strncmp(cp,"Oct",3)) tm.tm_mon = 9;
	else if (!strncmp(cp,"Nov",3)) tm.tm_mon = 10;
	else if (!strncmp(cp,"Dec",3)) tm.tm_mon = 11;
	else return 0;
	nextspace( cp );
	skipspace( cp );
	
	/* get year */
	tm.tm_year = atoi( cp );
	if ( !tm.tm_year ) return 0;
	if ( tm.tm_year > 1900 ) tm.tm_year -= 1900;
	nextspace( cp );
	skipspace( cp );
	
	/* get time */
	tm.tm_hour = atoi( cp );
	cp=strchr( cp,':' );
	if (cp)
	{
		tm.tm_min = atoi( cp+1 );
	}
	nextspace( cp );
	skipspace( cp );
	
	/* get timezone, if any */
	time_t offset = CStr2Zone( cp );
	
	/* calculate as if local time */
	time_t t = mktime(&tm);
	
	if ( t < 0 )
	{
		SDTrace1("Invalid date: %s",dateStr);
		return 0;	// ignore invalid dates
	}
	
	/* adjust for timezone, if specified */
	if (*cp)
	{
		t += offset - _timezone;
	}
	
	return t;
}

time_t CStr2Zone(const char *s)
{
	time_t offset;

	offset = atoi(s);
	if (offset)
	{
		if (offset>2400 || offset<-2400) offset = _timezone;
		else
		{
			if (*s == '-') offset *= -1;
			offset = 60*((offset/100)*60 + offset%100);
			if (*s == '-') offset *= -1;
		}
	}
	return(offset);
}
