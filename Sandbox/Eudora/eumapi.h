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
unsigned long SaveMAPIMessage(const CString& messageData);

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
