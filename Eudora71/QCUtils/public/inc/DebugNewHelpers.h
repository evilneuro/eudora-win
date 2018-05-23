#ifndef __DEBUG_NEW_HELPERS_H__
#define __DEBUG_NEW_HELPERS_H__

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


#include <new>
#include <crtdbg.h>

#ifdef _DEBUG

// Private MFC internal definition from "afxpriv.h"
#ifndef _AFX_CLIENT_BLOCK
// Special _CLIENT_BLOCK type to identifiy CObjects.
#define _AFX_CLIENT_BLOCK (_CLIENT_BLOCK|(0xc0<<16))
#endif

// Debug only variations of operator new and delete that support both std::nothrow
// and file name and line number tracking.
__declspec(dllexport) void * __cdecl operator new(size_t nSize, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine) throw();
__declspec(dllexport) void * __cdecl operator new[](size_t nSize, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine) throw();
__declspec(dllexport) void __cdecl operator delete(void * ptr, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine) throw();
__declspec(dllexport) void __cdecl operator delete[](void * ptr, const std::nothrow_t&, int nType, const char * lpszFileName, int nLine) throw();

// Debugging macros similar to DEBUG_NEW that support file name and line number
// tracking combined with std::nothrow. Two variations to support tagging
// the block correctly as well as avoiding CObject's implementation of
// operator new.
#define DEBUG_NEW_NOTHROW new(std::nothrow, _NORMAL_BLOCK, THIS_FILE, __LINE__)
#define DEBUG_NEW_MFCOBJ_NOTHROW ::new(std::nothrow, _AFX_CLIENT_BLOCK, THIS_FILE, __LINE__)

#else

#define DEBUG_NEW_NOTHROW new(std::nothrow)
#define DEBUG_NEW_MFCOBJ_NOTHROW ::new(std::nothrow)

#endif


#endif		//	__DEBUG_NEW_HELPERS_H__
