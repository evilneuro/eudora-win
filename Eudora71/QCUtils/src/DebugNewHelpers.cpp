// Variations of debugging macros and operator new and delete to support
// file name and line number leak detection in conjuction with calling
// the "no throw" variation of operator new.
//
// Copyright (c) 2005 by QUALCOMM, Incorporated
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


#include "stdafx.h"

#include "DebugNewHelpers.h"


#ifdef _DEBUG

// From afxmem.cpp
extern void* __cdecl operator new(size_t nSize, int nType, LPCSTR lpszFileName, int nLine);
extern void* __cdecl operator new[](size_t nSize, int nType, LPCSTR lpszFileName, int nLine);


void * __cdecl operator new(size_t nSize, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine) throw()
{
	void *	p;

	try
	{
		p = operator new(nSize, nType, lpszFileName, nLine);
	}
	// Catching everything isn't technically what we should do - we should only catch memory errors.
	// However:
	// * MS's implementation in newopnt.cpp catches everything.
	// * MFC's debug new seems to throw a CMemoryException as opposed to std::bad_alloc, but this may change
	// * We're inside of _DEBUG anyway so this is only affecting Debug builds, so even if this is a
	//   little overzealous about catching, it's not such a big deal
	catch (...)
	{
		// We're in a debug build. Be noisy if new throws and we're catching it here.
		ASSERT(0);
		
		p = 0;
	}

	return p;
}


void * __cdecl operator new[](size_t nSize, const std::nothrow_t & x, int nType, const char * lpszFileName, int nLine) throw()
{
	// Just use non-array operator new above
	return operator new(nSize, x, nType, lpszFileName, nLine);
}


void __cdecl operator delete(void * ptr, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine)
{
	// Just call MFC's standard operator delete
	operator delete(ptr, nType, lpszFileName, nLine);
}


void __cdecl operator delete[](void * ptr, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine)
{
	// Just call MFC's standard array operator delete
	operator delete[](ptr, nType, lpszFileName, nLine);
}


#endif	//	_DEBUG
