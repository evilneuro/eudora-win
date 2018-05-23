// plist_cache.h
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

#ifndef _PLIST_CACHE_H_
#define _PLIST_CACHE_H_

#include "plist_inet.h"
#include "plist_privcache.h"


////////////////////////////////////////////////////////////////////////////////
// prefetch_queue -- private list of items to be fetched

struct prefetch_item;
typedef prefetch_item* prefetch_queue;


////////////////////////////////////////////////////////////////////////////////
// PrefetchContext -- the inet context used for caching ads

class PrefetchContext : public InetContext
{
public:
	char* url;
	bool bLocalFile;
	int plist;
	long eid;
	long getfile_thread;
	PrivCachePtr pcp;
	PrivCacheFilePtr fp;

	// inet context protocol
	void InetComplete( InetResult* /*pRslt*/ ){}

public:
	PrefetchContext(){kind = 'ftch'; url = 0; bLocalFile = false; plist = 0; eid = 0;
		getfile_thread = -1; pcp = NULL; fp = NULL; }
	virtual ~PrefetchContext();

	void exec()
		{ getfile_thread = _beginthread( getfile_thread_proc, 0, this ); }

private:
	static void getfile_thread_proc( void* p );
};


int QueueUnfetchedEntries( PrivCachePtr pcp, prefetch_queue* ppfq, PlaylistRecPtr plrp );
int QueueUnfetchedEntry( prefetch_queue* ppfq, Entry* pe );
bool GetCachedEntryThang( PrivCachePtr pcp, const char* url, char** filePath, const char checksum[33], bool wantURL, int *chkSumFailures );
#define GetCachedEntryPath( pcp, u, cs, fp ) GetCachedEntryThang( (pcp), (u), (fp), (cs), false, NULL )
#define GetCachedEntryURL( pcp, u, u2 ) GetCachedEntryThang( (pcp), (u), (u2), 0, true, NULL )
#define GetCachedEntryPathCarefully(  pcp, u, cs, fp, fs ) GetCachedEntryThang( (pcp), (u), (fp), (cs), false, fs )
PrefetchContext* FetchCachedEntry( PrivCachePtr pcp, prefetch_queue* ppq );

#endif   // _PLIST_CACHE_H_
