// ImapTypes.h
//
// Miscellaneous utility classes and functions.
//
// Copyright (c) 1997-2003 by QUALCOMM, Incorporated
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

#ifndef __IMAPTYPES_H
#define __IMAPTYPES_H

// Global constants.
const	long			MsgByteLimit 			= 54*1024L;	// 54K for each message -- MAX
const	long			MsgByteLimit32			= 1024*1024L; // Let's limit message to 1 Meg
const	unsigned long	_MAGICNUMBER			= 98754908;	// Arbitrary.

const int				 iStateOnline = 0;
const int				 iStateStayOfflineDisallow = 1;
const int				 iStateStayOfflineAllow = 2;
const int				 iStateGoOnline = 3;
const int				 iStateGoOnlineForThis = 4;

const bool				 bDoAllowOffline = true;
const bool				 bDontAllowOffline = false;


#pragma warning(disable : 4663)
#pragma warning(disable : 4244)

// #include <map>

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include <algorithm>
#include <map>
#pragma warning (default: 4663 4244)


using namespace std;


// CPtrUidList below contains a list of objects of this type:
class CImapFlags
{
public:
	CImapFlags (unsigned long msgno = 0, unsigned long uid = 0,
				unsigned long Imflags = 0, BOOL IsNew = FALSE);

	~CImapFlags() {}

	// Data:
	unsigned long m_Uid;
	unsigned long m_Imflags;
	BOOL		  m_IsNew;
};

//=============== CPtrUidList interface ==========================//
class CPtrUidList  : public CPtrList
{
public:
	CPtrUidList();
	~CPtrUidList();

	// Insert in ascending order.

	// If we already have flags as bitflags.
	POSITION OrderedInsert(unsigned long uid, unsigned long Imflags, BOOL IsNew);
	// Have BOOLS.
	POSITION OrderedInsert(	unsigned long uid,
							BOOL seen,
							BOOL deleted,
							BOOL flagged,
							BOOL answered,
							BOOL draft,
							BOOL recent,
							BOOL IsNew);

	// Delete data plus do a RemoveAll().
	void DeleteAll();
};

class CUidMap : public map<unsigned long, CImapFlags *, less<unsigned long> >
{
public:
	virtual ~CUidMap();

	void DeleteAll ();
	void RemoveAll ();

	BOOL OrderedInsert(unsigned long uid, unsigned long Imflags, BOOL IsNew);
	BOOL OrderedInsert(unsigned long uid,
									BOOL seen,
									BOOL deleted,
									BOOL flagged,
									BOOL answered,
									BOOL draft,
									BOOL recent,
									BOOL IsNew);

	// Utility:
	//
	unsigned long GetHighestUid ();

	int GetCount ()
		{ return size(); }
		
};

typedef CUidMap::iterator			UidIteratorType;
typedef CUidMap::reverse_iterator	ReverseUidIteratorType;


//================================================================//

// Use a MAP to store case sensitive generic strings. We key on the string.
// Don't store any data.
//

// #include "cstring.h"

class CStringMap : public map<CString, unsigned long, less<CString> >
{
public:
	virtual ~CStringMap();

	void DeleteAll ();
	void RemoveAll ();

	int GetCount ()
		{ return size(); }
};

typedef CStringMap::iterator			StringMapIteratorType;
typedef CStringMap::reverse_iterator	ReverseStringMapUidIteratorType;


// Exported functions.
//

int GetConnectionState(bool bAllowOffline);


void MergeUidMapsMT (CUidMap& CurrentUidMap, CUidMap& NewUidMap, CUidMap& NewOldUidMap);
void RemoveSubUidMapMT (CUidMap& MainUidMap,CUidMap& SubUidMap);
void CommaListToUidMap (LPCSTR pStr, CUidMap& DstUidMap);

void __ImapDebugLogger (char *string,long errflg);

#endif // __IMAPTYPES_H