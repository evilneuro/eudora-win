// plist_privcache.h -- Our own cache handler
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

#ifndef _PLIST_PRIVCACHE_H_
#define _PLIST_PRIVCACHE_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// A few forward-looking declarations
typedef struct PrivCacheStruct *PrivCachePtr;
typedef struct PrivCacheFileStruct *PrivCacheFilePtr;

// Our basic structure
typedef struct PrivCacheStruct
{
	// Public stuff.  Look, but don't touch
	
	// Information about the cache as a whole (call PrivCacheSweep first)
	char *dir;	// The base path for our directory
	int numFiles;		// The number of files in our directory
	size_t totalK;		// The total amount of data, in K
	
	// Cache algorithms
	time_t minAccAge;		// Access age after which files can be removed, or 0
	int sweep;				// How often the cache is swept

	// Private stuff.  Pretend you don't see this
	
	bool initted;			// have we initted this one?
	
} PrivCache;

// The file structure
typedef struct PrivCacheFileStruct
{
	char *url;
	char *tmpFile;
	FILE *fp;
} PrivCacheFile;

////////////////////////////////////////////////////////////////
// Allocate and deallocate

// Allocate.  initPtr can be NULL, or else full of chewy initial options
PrivCachePtr PrivCacheNew(const char *subDir, const char *dir, PrivCachePtr initPtr, int valid);

// Duh.
void PrivCacheDispose(PrivCachePtr pcp);

////////////////////////////////////////////////////////////////
// Routine maintenance

// Set options
int PrivCacheSet(PrivCachePtr pcp,PrivCachePtr optionsPtr, int valid);

// Ask for a cache sweep
// optionsPtr can be used if you want to override normal sweep options
// As a special case, optionsPtr of NULL and valid of kPrCaAllValid
// nukes all the files in the cache
int PrivCacheSweep(PrivCachePtr pcp, PrivCachePtr optionsPtr, int valid);

// Kill a particular file
int PrivCacheKill(PrivCachePtr pcp, const char *url);

////////////////////////////////////////////////////////////////
// Adding files

// Add a file to the cache.  Set killThePig to true if the file should
// be removed after it is successfully added to the cache
int PrivCacheAddFile(PrivCachePtr pcp, const char *url, const char *path, bool killThePig);

////////////////////////////////////////////////////////////////
// Adding data

// Open a file for eventual caching
PrivCacheFilePtr PrivCacheOpen(PrivCachePtr pcp, const char *url);

// Add some data to the cached file
// returns 0 if it writes all the data
int PrivCacheWrite(PrivCachePtr pcp, PrivCacheFilePtr fp, const char *buf, size_t len);

// Close the cached file
int PrivCacheClose(PrivCachePtr pcp, PrivCacheFilePtr fp, bool killThePig);

////////////////////////////////////////////////////////////////
// Did you think we were going to have a write-only cache?

// The big enchilada - do we have the file?
// The okToDownload parameter is IGNORED for now.
// The completion parameter is IGNORED for now.  It will eventually be
// called when the request to download the URL completes
// Returns a file: URL/path or NULL if the file is not available
char *PrivCacheGet(PrivCachePtr pcp, const char *url, bool okToDownload, bool wantURL);

////////////////////////////////////////////////////////////////
// Boring validity flags

#define kPrCaminAccAgeValid (1<<0)
#define kPrCasweepValid 	(1<<1)


////////////////////////////////////////////////////////////////
// Exciting sweep flags

#define kPrCasweepDispose	(1<<0)	// when destroying cache


#ifdef __cplusplus
}
#endif

#endif    // _PLIST_PRIVCACHE_H_
