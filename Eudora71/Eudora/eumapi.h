// EuMapi.h
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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

#ifndef __EUMAPI_H__
#define __EUMAPI_H__
// EUMAPI.H
//
// Definitions for Eudora MAPI DLL
//


typedef unsigned long ULONG;	// needed by protos in MAPI.H

//
// Will the real MAPI header please stand up?
//
#include "mapi.h"

//
// Okay, now some hacks to #undef the entry point names.
//
#undef MAPILogon
#undef MAPISendMail
#undef MAPISendDocuments
#undef MAPIFindNext
#undef MAPIReadMail
#undef MAPISaveMail
#undef MAPIDeleteMail
#undef MAPIFreeBuffer
#undef MAPIAddress
#undef MAPIDetails
#undef MAPIResolveName

#define EudoraMainWindowClass "EudoraMainWindow"

// 16-bit Windows doesn't have the WM_COPYDATA stuff defined
#ifndef WIN32
#define WM_COPYDATA                     0x004A

typedef struct tagCOPYDATASTRUCT {  // cds  
    DWORD dwData;
	DWORD cbData;
	LPVOID lpData;
} COPYDATASTRUCT, FAR * PCOPYDATASTRUCT;
#endif

LRESULT HandleMAPIMessage(PCOPYDATASTRUCT pcds);
unsigned long SaveMAPIMessage(const char* messageData);

//
// These are the message ID's to identify which MAPI action is being
// requested in the WM_COPYDATA data across the DLL-to-Eudora
// interface.  FORNOW, should probably be C++ enum.
//
const DWORD MAPI_MSG_FIRST =		100;
const DWORD EUM_SEND_DOCUMENTS =	100;
const DWORD EUM_SEND_MAIL =			101;
const DWORD EUM_CHECK_MAIL = 		102;
const DWORD MAPI_MSG_LAST =			200;

#endif // __EUMAPI_H__
