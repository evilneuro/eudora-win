// plist_privcache.h -- Our own cache handler
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

#include "string.h"
#include "assert.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include <direct.h>

#ifdef WIN32
#include "io.h"
#else
#include "unistd.h"
#endif

#include "plist_privcache.h"
#include "plist_utils.h"
#include "plist_debug.h"

#include "DebugNewHelpers.h"

////////////////////////////////////////////////////////////////
// Private declarations
////////////////////////////////////////////////////////////////

#define kDefaultSweep	(kPrCasweepDispose)
#define kDefaultAccAge	(3*24*3600)	// keep files three days
char *PrivCacheURL2Path(PrivCachePtr pcp, const char *url, char *path);
unsigned int PrivCacheHash(const char* pszStr);
int PrivCacheMakeDir(PrivCachePtr pcp);
size_t PrivCacheK (const char *path);

////////////////////////////////////////////////////////////////
// Public Stuff
////////////////////////////////////////////////////////////////

//
// Allocate and deallocate
//

////////////////////////////////////////////////////////////////
// Allocate.  initPtr can be NULL, or else full of chewy initial options
PrivCachePtr PrivCacheNew(const char *subDir, const char *dir, PrivCachePtr initPtr, int valid)
{
	PrivCachePtr pcp = DEBUG_NEW_NOTHROW PrivCache;
	int err = 1;
	
	if (pcp)
	{
		// wipe to zero to begin
		memset(pcp,'\0',sizeof(*pcp));
		
		// allocate the directory path
		pcp->dir = DEBUG_NEW_NOTHROW char[strlen(dir)+strlen(subDir)+1];
		if (pcp->dir)
		{
			strcpy(pcp->dir,dir);
			strcat(pcp->dir,subDir);
			
			// does the directory actually exist?
			err = PrivCacheMakeDir(pcp);
			
			// init values	
			pcp->minAccAge = kDefaultAccAge;
			pcp->sweep = kDefaultSweep;
			
			if (valid) PrivCacheSet(pcp,initPtr,valid);
		}
	}
	
	// Did we win?
	if (err)
	{
		PrivCacheDispose(pcp);
		pcp = NULL;
	}
	else
	{
		pcp->initted = true;
		// Do an inital sweep just to get our numbers right,
		// But don't delete any files just yet
		PrivCache pc;
		pc.minAccAge = 0;
		PrivCacheSweep(pcp,&pc,kPrCaminAccAgeValid);
	}
	
	return pcp;
}

////////////////////////////////////////////////////////////////
// Duh.
void PrivCacheDispose(PrivCachePtr pcp)
{
	if (!pcp) return;
	
	// Do a final sweep if the user is so inclined
	if (pcp->initted && pcp->sweep&kPrCasweepDispose)
		PrivCacheSweep(pcp, NULL, 0);
	
	// Die die die
	if (pcp->dir) delete [] pcp->dir;
	delete pcp;
}

//
// Routine maintenance
//

////////////////////////////////////////////////////////////////
// Set options
int PrivCacheSet(PrivCachePtr pcp,PrivCachePtr optionsPtr, int valid)
{
#define PRIV_CACHE_SET_IF_VALID(x)\
	do { if (valid&kPrCa ## x ## Valid) \
		pcp-> ## x = optionsPtr-> ## x; } while(0)
	
	PRIV_CACHE_SET_IF_VALID(minAccAge);
	PRIV_CACHE_SET_IF_VALID(sweep);

	return 0;
}

////////////////////////////////////////////////////////////////
// Ask for a cache sweep
int PrivCacheSweep(PrivCachePtr pcp, PrivCachePtr optionsPtr, int valid)
{
	// calculate mod times less than which we will dump the file
	time_t age = pcp->minAccAge;
	time_t absAge = 0;
	if (valid & kPrCaminAccAgeValid) age = optionsPtr->minAccAge;
	if (age) absAge = time(0) - age;
	
	// make copy of the string and add *.* to it
	char starDotStar[MAX_PATH+1];
	strncpy(starDotStar,pcp->dir,sizeof(starDotStar)-4);
	strcat(starDotStar,"\\*.*");
	
	// walk the directory
	WIN32_FIND_DATA findData;
	HANDLE h = FindFirstFile(starDotStar,&findData);
	if (INVALID_HANDLE_VALUE != h)
	{
		// Reset the values
		pcp->totalK = 0;
		pcp->numFiles = 0;
		
		// figure out what time it is
		
		do
		{
			// Skip directories
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			
			if (age)
			{
				// Build the pathname
				char path[MAX_PATH+1];
				sprintf(path,"%s\\%s",pcp->dir,findData.cFileName);
				
				// How old is the file?
				struct stat info;
				if (stat(path,&info)) continue;
				
				if (age)	// age==0 means don't delete anything, just count
				if (info.st_atime<absAge)
				{
					SDTrace2("%s is %d too old, unlinking", path, absAge - info.st_atime);
					if (!unlink(path))
						continue;
				}
			}
			
			// accumulate filesize & count
			pcp->totalK += (findData.nFileSizeLow+512)/1024;
			pcp->numFiles++;
		}
		while (FindNextFile(h,&findData));
		FindClose(h);
	}

	SDTrace3("PrCaSweep age %dh #%d %dK", age/3600, pcp->numFiles, pcp->totalK);

	// are we done now?
	return 0;
}

////////////////////////////////////////////////////////////////
// Kill a particular file
int PrivCacheKill(PrivCachePtr pcp, const char *url)
{
	char path[MAX_PATH+1];
	
	// turn into filename & figure out size
	size_t k = PrivCacheK(PrivCacheURL2Path(pcp,url,path));
	
	// remove file
	int err = unlink(path);
	
	// if we've removed it, count it no more
	if (!err)
	{
		pcp->totalK -= k;
		pcp->numFiles--;
	}

	SDTrace5("PrCaKill %d %d %dK %s as %s",
		err, pcp->numFiles, pcp->totalK,
		url, path);

	return err;
}

////////////////////////////////////////////////////////////////
// Adding files

////////////////////////////////////////////////////////////////
// Add a file to the cache.
int PrivCacheAddFile(PrivCachePtr pcp, const char *url, const char *path, bool killThePig)
{
	char newPath[MAX_PATH+1];
	
	// we fail unless killThePig
	if (!killThePig) return 1;
	
	// let's start by making sure we have a directory
	PrivCacheMakeDir(pcp);
	
	// first, kill the old one if we had one
	PrivCacheKill(pcp,url);

	// rename is evidently some sort of magic
	int err = rename(path,PrivCacheURL2Path(pcp,url,newPath));
	
	// if it worked, count it
	if (!err)
	{
		pcp->totalK += PrivCacheK(newPath);
		pcp->numFiles++;
	}
	
	SDTraceLastError(err);
	SDTrace4("PrCaAddFile %d %d %dK %s", err, pcp->numFiles, pcp->totalK, url);
	SDTrace2("\t%s as %s", path, newPath);

	return err;
}

//
// Adding data
//

////////////////////////////////////////////////////////////////
// Open a file for eventual caching
PrivCacheFilePtr PrivCacheOpen(PrivCachePtr /*pcp*/, const char *url)
{
	PrivCacheFilePtr fp = DEBUG_NEW_NOTHROW PrivCacheFile;
	
	if (fp)
	{
		fp->url = DupeString(url);
		if (fp->url)
		{
			fp->tmpFile = TempFile();
			if (fp->tmpFile)
			{
				fp->fp = fopen(fp->tmpFile,"wb");
				if (fp->fp)
				{
					SDTrace2("PrCaOpen %s %s",url,fp->tmpFile);
					return fp;
				}
				delete [] fp->tmpFile;
			}
			delete [] fp->url;
		}
		delete fp;
	}
	
	SDTrace1("PrCaOpen failed %s",url);
	
	return NULL;
}

////////////////////////////////////////////////////////////////
// Add some data to the cached file
int PrivCacheWrite(PrivCachePtr /*pcp*/, PrivCacheFilePtr fp, const char *buf, size_t len)
{
	return (len==fwrite(buf,1,len,fp->fp)) ? 0:1;
}

////////////////////////////////////////////////////////////////
// Close the cached file
int PrivCacheClose(PrivCachePtr pcp, PrivCacheFilePtr fp, bool /*killThePig*/)
{
	int err = fclose(fp->fp);
	
	// if it worked...
	if (!err)
	{
		// move it into place
		err = PrivCacheAddFile(pcp,fp->url,fp->tmpFile,true);
	}
	
	// delete the file if we failed
	if (err) unlink(fp->tmpFile);
	
	// whether or not we succeeded, deallocate memory
	delete [] fp->tmpFile;
	delete [] fp->url;
	delete fp;
	
	return err;
}

//
// Did you think we were going to have a write-only cache?
//

////////////////////////////////////////////////////////////////
// The big enchilada - do we have the file?
char *PrivCacheGet(PrivCachePtr pcp, const char *url, bool /*okToDownload*/, bool wantURL)
{
	char path[MAX_PATH+1];
	struct stat info;
	char *retString = NULL;
	
	// check for file
	if (!stat(PrivCacheURL2Path(pcp,url,path),&info))
	
	// Ok, turn into URL
	retString = DEBUG_NEW_NOTHROW char[strlen(path) + (wantURL?7:0) + 1];
	if (retString)
	{
		*retString = 0;
		if (wantURL) strcpy(retString,"file://");
		strcat(retString,path);
	}
	
	//SDTrace2("PrCaGet %s %s",url,retString?retString:"FAIL");
#ifdef PLIST_LOGGING
	if (!retString && wantURL) SDTrace2("PrCaGet %s %s",url,"FAIL");
#endif
	
	return retString;
}

////////////////////////////////////////////////////////////////
// Guts - what little of them there are
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Turn a URL into a pathname
char *PrivCacheURL2Path(PrivCachePtr pcp, const char *url, char *path)
{
	unsigned int hash = PrivCacheHash(url);
	
	char *extension = strrchr(url,'.');
	if (!extension || strlen(extension)>5) extension = ".dat";
	
	sprintf(path,"%s\\%08x%s",pcp->dir,hash,extension);
	
	return path;
}	
	
////////////////////////////////////////////////////////////////////////
// PrivCacheHash
//
// The following hashing algorithm, KRHash, is derived from Karp & Rabin,
// Harvard Center for Research in Computing Technology Tech. Report TR-31-81
// The constant prime number in use is kKRHashPrime.  It happens to be the largest
// prime number that will fit in 31 bits, except for 2^31-1 itself....
////////////////////////////////////////////////////////////////////////
unsigned int PrivCacheHash(const char* pszStr)
{
	const unsigned int kKRHashPrime = 2147483629;

	unsigned int ulSum = 0;		// returned

	for (; *pszStr; pszStr++)
	{
		for (int nBit = 0x80; nBit != 0; nBit >>= 1)
		{
			ulSum += ulSum;
			if (ulSum >= kKRHashPrime)
				ulSum -= kKRHashPrime;
			if ((*pszStr) & nBit)
				++ulSum;
			if (ulSum>= kKRHashPrime)
				ulSum -= kKRHashPrime;
		}
	}

	return (ulSum + 1);
}

////////////////////////////////////////////////////////////////
// Make the directory
int PrivCacheMakeDir(PrivCachePtr pcp)
{
	int err;
	struct stat info;

	err = stat(pcp->dir,&info);
	if (err)
	{
		err = mkdir(pcp->dir);	// take a stab at creating it
		SDTrace2("PrCaMakeDir: %s %d",pcp->dir,err);
	}
	return err;
}


////////////////////////////////////////////////////////////////
// How big is a file in K?
size_t PrivCacheK (const char *path)
{
	struct stat info;

	if (!stat(path,&info))
		return (info.st_size+512)/1024;
	else
		return 0;
}

