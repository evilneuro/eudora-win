/*
 	File:		TSMTE.h
 
 	Contains:	Text Services Managerfor TextEdit Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	©1991-1998 Apple Technology, Inc. All rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __TSMTE__
#define __TSMTE__

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif
#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
#ifndef __TEXTSERVICES__
#include <TextServices.h>
#endif
/* #include <Gestalt.i>*/




#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

/* signature, interface types*/

enum {
	kTSMTESignature				= FOUR_CHAR_CODE('tmTE'),
	kTSMTEInterfaceType			= FOUR_CHAR_CODE('tmTE'),
	kTSMTEDialog				= FOUR_CHAR_CODE('tmDI')
};


/* update flag for TSMTERec*/

enum {
	kTSMTEAutoScroll			= 1
};


/* callback procedure definitions*/

typedef CALLBACK_API( void , TSMTEPreUpdateProcPtr )(TEHandle textH, long refCon);
typedef CALLBACK_API( void , TSMTEPostUpdateProcPtr )(TEHandle textH, long fixLen, long inputAreaStart, long inputAreaEnd, long pinStart, long pinEnd, long refCon);
typedef STACK_UPP_TYPE(TSMTEPreUpdateProcPtr) 					TSMTEPreUpdateUPP;
typedef STACK_UPP_TYPE(TSMTEPostUpdateProcPtr) 					TSMTEPostUpdateUPP;


/* data types*/

struct TSMTERec {
	TEHandle 						textH;
	TSMTEPreUpdateUPP 				preUpdateProc;
	TSMTEPostUpdateUPP 				postUpdateProc;
	long 							updateFlag;
	long 							refCon;
};
typedef struct TSMTERec					TSMTERec;
typedef TSMTERec *						TSMTERecPtr;
typedef TSMTERecPtr *					TSMTERecHandle;

struct TSMDialogRecord {
	DialogRecord 					fDialog;
	TSMDocumentID 					fDocID;
	TSMTERecHandle 					fTSMTERecH;
	long 							fTSMTERsvd[3];				/* reserved*/
};
typedef struct TSMDialogRecord			TSMDialogRecord;

typedef TSMDialogRecord *				TSMDialogPeek;
enum { uppTSMTEPreUpdateProcInfo = 0x000003C0 }; 				/* pascal no_return_value Func(4_bytes, 4_bytes) */
enum { uppTSMTEPostUpdateProcInfo = 0x000FFFC0 }; 				/* pascal no_return_value Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewTSMTEPreUpdateProc(userRoutine) 						(TSMTEPreUpdateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTSMTEPreUpdateProcInfo, GetCurrentArchitecture())
#define NewTSMTEPostUpdateProc(userRoutine) 					(TSMTEPostUpdateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTSMTEPostUpdateProcInfo, GetCurrentArchitecture())
#define CallTSMTEPreUpdateProc(userRoutine, textH, refCon) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppTSMTEPreUpdateProcInfo, (textH), (refCon))
#define CallTSMTEPostUpdateProc(userRoutine, textH, fixLen, inputAreaStart, inputAreaEnd, pinStart, pinEnd, refCon)  CALL_SEVEN_PARAMETER_UPP((userRoutine), uppTSMTEPostUpdateProcInfo, (textH), (fixLen), (inputAreaStart), (inputAreaEnd), (pinStart), (pinEnd), (refCon))

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TSMTE__ */

