// plist_cache.cpp
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
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "process.h"

#ifdef WIN32
#include "io.h"
#else
#include "unistd.h"
#endif

#include "plist_stg.h"
#include "plist_utils.h"
#include "plist_inet.h"
#include "plist_html.h"
#include "plist_debug.h"
#include "plist_md5.h"
#include "plist_secret.h"

// our own lazy ass
#include "plist_cache.h"

#include "DebugNewHelpers.h"

////////////////////////////////////////////////////////////////////////////////

#define ods(s) OutputDebugString(s"\n")

void fetch_subreferences( char* localFile, char* contentLoc );
int fetch_subref_callback( char* in, long ref, char* out, int maxOut );

void queue_for_prefetch( prefetch_queue* pfq, const char* purl );
bool next_prefetch_item( prefetch_queue* pfq, char** purl );


int QueueUnfetchedEntries( PrivCachePtr pcp, prefetch_queue* ppfq, PlaylistRecPtr plrp )
{
    Entry e;
    EntryRecPtr erp = plrp->entries;

    for ( erp=plrp->entries; erp; erp = erp->next )
    	if ( !erp->oldNCrufty )
    	if ( !ReadEntry( plrp, erp, &e ) ) {
			if ( !GetCachedEntryPath( pcp, e.src, e.checksum, 0 ) ) {
				queue_for_prefetch( ppfq, e.src );
				SDTrace1("Queueing unfetched %s\n",e.src);
			}
		}

    return 0;
}

int QueueUnfetchedEntry( prefetch_queue* ppfq, Entry* pe )
{
	ods( "QueueUnfetchedEntry:" );
	queue_for_prefetch( ppfq, pe->src );
	SDTrace1("Queueing unfetched %s\n", pe->src );
	return 0;
}

bool is_valid_entry( const char* filename, const char* chksm )
{
	// no checksum is ok?
	if ( !(chksm && *chksm) )
		return true;

	char newChksm[33] = "";
	MD5Secret itsTrue;
	if ( GenerateMD5DigestFromFile( itsTrue, filename, newChksm ) )
	{
		bool ret = strcmp( chksm, newChksm ) == 0;
		if ( !ret ) SDTrace1( "Checksum failed: %s", filename );
		return ret;
	}

	return false;
}

// maps a url to an entry in the local cache. a new checksum is computed, and
// compared with caller's -- we delete cache entries that fail.
bool GetCachedEntryThang( PrivCachePtr pcp, const char *url, char** filePath,
			const char checksum[33], bool wantURL, int *chkSumFailures )
{
	bool bRet = false;
	char *thang = PrivCacheGet( pcp, url, false, wantURL );

	if ( thang ) {
		if ( is_valid_entry( wantURL ? thang+7 : thang, checksum ) ) {
			bRet = true;

			if ( filePath )
				*filePath = thang;
		}
		else
		{
			if ( chkSumFailures ) ++*chkSumFailures;
			PrivCacheKill( pcp, url );
		}

		if ( !bRet || !filePath )
			delete [] thang;
	}
	
    return bRet;
}

PrefetchContext* FetchCachedEntry( PrivCachePtr pcp, prefetch_queue* ppq )
{
	PrefetchContext* pctx = 0;
	char* purl = 0;

	if ( next_prefetch_item( ppq, &purl ) ) {
		OutputDebugString( purl );
		ods( "" );

		pctx = DEBUG_NEW PrefetchContext;
		pctx->url = purl;
		pctx->status = kNetOK;
		pctx->bLocalFile = (strnicmp( "file://", purl, 7 ) == 0);
		pctx->pcp = pcp;

		pctx->exec();
	}

	return pctx;
}


// TODO: fix ParseURLs to use "new" instead of malloc so we can use
// "delete" instead of "free" here.
void fetch_subreferences( char* localFile, char* contentLoc )
{
	int fd = open( localFile, O_RDONLY );

	if ( fd != -1 ) {
		long nBytes = lseek( fd, 0, SEEK_END );
		lseek( fd, 0, SEEK_SET );
		char* srcBuf = DEBUG_NEW char[nBytes + 1];
		read( fd, srcBuf, nBytes );

		char* out = 0;
		ParseURLs( srcBuf, fetch_subref_callback, (long)contentLoc, 0 );

		delete [] srcBuf;
		free( out );
	}
}

int fetch_subref_callback( char* /*in*/, long /*ref*/, char* /*out*/, int /*maxOut*/ )
{
	// TODO: fetch all urls for images---anyting else?---resolving them as
	// needed with the content location passed in "ref".
	// ParseURLs should be fixed to not do allocations unless we want.
	// It would be nice if ParseURLs would tell us the mime type that a URL
	// references---but I think that'll have to wait.

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// prefetch queue

extern unsigned int PrivCacheHash( const char* pstr );

struct prefetch_item {
	unsigned uid;
	char* url;
	prefetch_item* next;
};

bool prefetch_item_exists( prefetch_queue* pfq, unsigned uid )
{
	prefetch_item* pi = *pfq;
	for ( ; pi; pi = pi->next )
		if ( pi->uid == uid )
			return true;

	return false;
}

void queue_for_prefetch( prefetch_queue* pfq, const char* purl )
{
	int uid = PrivCacheHash( purl );
	if ( prefetch_item_exists( pfq, uid ) )
		return;

	prefetch_item* pi = DEBUG_NEW_NOTHROW prefetch_item;

    if ( pi ) {
		pi->uid = uid;
		pi->url = DupeString( purl );
        pi->next = 0;

        if ( *pfq ) {
            prefetch_item* pTrv;
            for ( pTrv = *pfq; pTrv->next; pTrv = pTrv->next )
                ;

            pTrv->next = pi;
        }
        else
            *pfq = pi;

		ods( "queued unfetched entry" );
    }
}

bool next_prefetch_item( prefetch_queue* pfq, char** purl )
{
    bool bRet = false;
	prefetch_item* pi = *pfq;

    if ( pi ) {
		*pfq = pi->next;
		*purl = pi->url;
		delete pi;
        bRet = true;
    }

    return bRet;
}


////////////////////////////////////////////////////////////////////////////////
// PrefetchContext -- the inet context used for caching ads

PrefetchContext::~PrefetchContext()
{
	delete [] url;
}

void PrefetchContext::getfile_thread_proc( void* p )
{
	PrefetchContext* pctx = (PrefetchContext*) p;
	inet_handle hInternet = NetGetSessionHandle();
	inet_handle hResource = InternetOpenUrl( hInternet, pctx->url, NULL, 0, 0, 0 );

	if ( hResource ) {
		ods( "open url succeeded" );

		BOOL bOK;
		DWORD nRead = 0;
		char buf[2048];
		
		pctx->fp = PrivCacheOpen( pctx->pcp, pctx->url );
		if ( pctx->fp ) {

			for ( ;; ) {
				bOK = InternetReadFile( hResource, buf, sizeof(buf), &nRead );
	
				if ( !bOK ) {
					// Boo: don't think we'll ever get a pending error in non-async
					// mode. if the assert never fires, i'll clean this up.
	
					if ( GetLastError() != ERROR_IO_PENDING ) {
						ods( "failed fetching ad" );
	
						pctx->status = kNetError;
						char* errMsg = NetErrorDesc( GetLastError() );
						OutputDebugString( errMsg );
						NetFreeError( errMsg );
						break;
					}
					else {
						ods( "ERROR_IO_PENDING" );
						assert( 0 );
						_sleep( 1000 );
					}
				}
				else if ( nRead == 0 ) {
					ods( "done fetching ad" );
					
					if (PrivCacheClose(pctx->pcp, pctx->fp, false))
					{
						ods ( "priv cache close failed" );
						pctx->status = kNetError;
					}
					else
						pctx->status = kNetComplete;
					pctx->fp = NULL;	// it went bye-bye
					break;
				}
				else {
					if (PrivCacheWrite(pctx->pcp, pctx->fp, buf, nRead))
					{
						ods ( "priv cache write failed" );
						pctx->status = kNetError;
						break;
					}
				}
			}
		}
		else {
			ods( "priv cache open failed" );
			pctx->status = kNetError;
		}
		InternetCloseHandle( hResource );
	}
	else {
		ods( "open url failed" );
		pctx->status = kNetError;
	}
	
	// if left open file, destroy
	if (pctx->fp) PrivCacheClose(pctx->pcp, pctx->fp, true);
}

