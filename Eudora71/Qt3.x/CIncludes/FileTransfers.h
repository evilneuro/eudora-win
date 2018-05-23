/*
 	File:		FileTransfers.h
 
 	Contains:	CommToolbox File Transfer Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __FILETRANSFERS__
#define __FILETRANSFERS__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __STANDARDFILE__
#include <StandardFile.h>
#endif
#ifndef __CTBUTILITIES__
#include <CTBUtilities.h>
#endif
#ifndef __CONNECTIONS__
#include <Connections.h>
#endif
#ifndef __FILES__
#include <Files.h>
#endif
#ifndef __TERMINALS__
#include <Terminals.h>
#endif



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


enum {
	curFTVersion				= 2								/* current file transfer manager version*/
};


enum {
																/* FTErr    */
	ftGenericError				= -1,
	ftNoErr						= 0,
	ftRejected					= 1,
	ftFailed					= 2,
	ftTimeOut					= 3,
	ftTooManyRetry				= 4,
	ftNotEnoughDSpace			= 5,
	ftRemoteCancel				= 6,
	ftWrongFormat				= 7,
	ftNoTools					= 8,
	ftUserCancel				= 9,
	ftNotSupported				= 10
};

typedef OSErr 							FTErr;

enum {
	ftIsFTMode					= 1 << 0,
	ftNoMenus					= 1 << 1,
	ftQuiet						= 1 << 2,
	ftConfigChanged				= 1 << 4,
	ftSucc						= 1 << 7
};

typedef unsigned long 					FTFlags;

enum {
	ftSameCircuit				= 1 << 0,
	ftSendDisable				= 1 << 1,
	ftReceiveDisable			= 1 << 2,
	ftTextOnly					= 1 << 3,
	ftNoStdFile					= 1 << 4,
	ftMultipleFileSend			= 1 << 5
};

typedef unsigned short 					FTAttributes;

enum {
	ftReceiving					= 0,
	ftTransmitting				= 1,
	ftFullDuplex				= 2								/* (16) added ftFullDuplex bit.*/
};

typedef unsigned short 					FTDirection;
typedef CALLBACK_API( long , FileTransferDefProcPtr )(TermHandle hTerm, short msg, long p1, long p2, long p3);
/*	application routines type definitions */
typedef struct FTRecord 				FTRecord;
typedef FTRecord *						FTPtr;
typedef FTPtr *							FTHandle;
typedef CALLBACK_API( OSErr , FileTransferReadProcPtr )(unsigned long *count, Ptr pData, long refCon, short fileMsg);
typedef CALLBACK_API( OSErr , FileTransferWriteProcPtr )(unsigned long *count, Ptr pData, long refCon, short fileMsg);
typedef CALLBACK_API( Size , FileTransferSendProcPtr )(Ptr thePtr, long theSize, long refCon, CMChannel channel, CMFlags flag);
typedef CALLBACK_API( Size , FileTransferReceiveProcPtr )(Ptr thePtr, long theSize, long refCon, CMChannel channel, CMFlags *flag);
typedef CALLBACK_API( OSErr , FileTransferEnvironsProcPtr )(long refCon, ConnEnvironRec *theEnvirons);
typedef CALLBACK_API( void , FileTransferNotificationProcPtr )(FTHandle hFT, const FSSpec *pFSSpec)/*  (15) added const */;
typedef CALLBACK_API( void , FileTransferChooseIdleProcPtr )(void );
typedef STACK_UPP_TYPE(FileTransferDefProcPtr) 					FileTransferDefUPP;
typedef STACK_UPP_TYPE(FileTransferReadProcPtr) 				FileTransferReadUPP;
typedef STACK_UPP_TYPE(FileTransferWriteProcPtr) 				FileTransferWriteUPP;
typedef STACK_UPP_TYPE(FileTransferSendProcPtr) 				FileTransferSendUPP;
typedef STACK_UPP_TYPE(FileTransferReceiveProcPtr) 				FileTransferReceiveUPP;
typedef STACK_UPP_TYPE(FileTransferEnvironsProcPtr) 			FileTransferEnvironsUPP;
typedef STACK_UPP_TYPE(FileTransferNotificationProcPtr) 		FileTransferNotificationUPP;
typedef STACK_UPP_TYPE(FileTransferChooseIdleProcPtr) 			FileTransferChooseIdleUPP;

struct FTRecord {
	short 							procID;
	FTFlags 						flags;
	FTErr 							errCode;
	long 							refCon;
	long 							userData;
	FileTransferDefUPP 				defProc;
	Ptr 							config;
	Ptr 							oldConfig;
	FileTransferEnvironsUPP 		environsProc;
	long 							reserved1;
	long 							reserved2;
	Ptr 							ftPrivate;
	FileTransferSendUPP 			sendProc;
	FileTransferReceiveUPP 			recvProc;
	FileTransferWriteUPP 			writeProc;
	FileTransferReadUPP 			readProc;
	WindowPtr 						owner;
	FTDirection 					direction;
	SFReply 						theReply;
	long 							writePtr;
	long 							readPtr;
	Ptr 							theBuf;
	long 							bufSize;
	Str255 							autoRec;
	FTAttributes 					attributes;
};

typedef unsigned short 					FTCompletionKind;

enum {
																/* FTReadProc messages */
	ftReadOpenFile				= 0,							/* count = forkFlags, buffer = pblock from PBGetFInfo */
	ftReadDataFork				= 1,
	ftReadRsrcFork				= 2,
	ftReadAbort					= 3,
	ftReadComplete				= 4,
	ftReadSetFPos				= 6,							/* count = forkFlags, buffer = pBlock same as PBSetFPos */
	ftReadGetFPos				= 7								/* count = forkFlags, buffer = pBlock same as PBGetFPos */
};


enum {
																/* FTWriteProc messages */
	ftWriteOpenFile				= 0,							/* count = forkFlags, buffer = pblock from PBGetFInfo */
	ftWriteDataFork				= 1,
	ftWriteRsrcFork				= 2,
	ftWriteAbort				= 3,
	ftWriteComplete				= 4,
	ftWriteFileInfo				= 5,
	ftWriteSetFPos				= 6,							/* count = forkFlags, buffer = pBlock same as PBSetFPos */
	ftWriteGetFPos				= 7								/* count = forkFlags, buffer = pBlock same as PBGetFPos */
};


enum {
																/*	fork flags */
	ftOpenDataFork				= 1,
	ftOpenRsrcFork				= 2
};

enum { uppFileTransferDefProcInfo = 0x0000FEF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppFileTransferReadProcInfo = 0x00002FE0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 2_bytes) */
enum { uppFileTransferWriteProcInfo = 0x00002FE0 }; 			/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 2_bytes) */
enum { uppFileTransferSendProcInfo = 0x0000AFF0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 2_bytes, 2_bytes) */
enum { uppFileTransferReceiveProcInfo = 0x0000EFF0 }; 			/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 2_bytes, 4_bytes) */
enum { uppFileTransferEnvironsProcInfo = 0x000003E0 }; 			/* pascal 2_bytes Func(4_bytes, 4_bytes) */
enum { uppFileTransferNotificationProcInfo = 0x000003C0 }; 		/* pascal no_return_value Func(4_bytes, 4_bytes) */
enum { uppFileTransferChooseIdleProcInfo = 0x00000000 }; 		/* pascal no_return_value Func() */
#define NewFileTransferDefProc(userRoutine) 					(FileTransferDefUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferDefProcInfo, GetCurrentArchitecture())
#define NewFileTransferReadProc(userRoutine) 					(FileTransferReadUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferReadProcInfo, GetCurrentArchitecture())
#define NewFileTransferWriteProc(userRoutine) 					(FileTransferWriteUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferWriteProcInfo, GetCurrentArchitecture())
#define NewFileTransferSendProc(userRoutine) 					(FileTransferSendUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferSendProcInfo, GetCurrentArchitecture())
#define NewFileTransferReceiveProc(userRoutine) 				(FileTransferReceiveUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferReceiveProcInfo, GetCurrentArchitecture())
#define NewFileTransferEnvironsProc(userRoutine) 				(FileTransferEnvironsUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferEnvironsProcInfo, GetCurrentArchitecture())
#define NewFileTransferNotificationProc(userRoutine) 			(FileTransferNotificationUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferNotificationProcInfo, GetCurrentArchitecture())
#define NewFileTransferChooseIdleProc(userRoutine) 				(FileTransferChooseIdleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferChooseIdleProcInfo, GetCurrentArchitecture())
#define CallFileTransferDefProc(userRoutine, hTerm, msg, p1, p2, p3)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppFileTransferDefProcInfo, (hTerm), (msg), (p1), (p2), (p3))
#define CallFileTransferReadProc(userRoutine, count, pData, refCon, fileMsg)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppFileTransferReadProcInfo, (count), (pData), (refCon), (fileMsg))
#define CallFileTransferWriteProc(userRoutine, count, pData, refCon, fileMsg)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppFileTransferWriteProcInfo, (count), (pData), (refCon), (fileMsg))
#define CallFileTransferSendProc(userRoutine, thePtr, theSize, refCon, channel, flag)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppFileTransferSendProcInfo, (thePtr), (theSize), (refCon), (channel), (flag))
#define CallFileTransferReceiveProc(userRoutine, thePtr, theSize, refCon, channel, flag)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppFileTransferReceiveProcInfo, (thePtr), (theSize), (refCon), (channel), (flag))
#define CallFileTransferEnvironsProc(userRoutine, refCon, theEnvirons)  CALL_TWO_PARAMETER_UPP((userRoutine), uppFileTransferEnvironsProcInfo, (refCon), (theEnvirons))
#define CallFileTransferNotificationProc(userRoutine, hFT, pFSSpec)  CALL_TWO_PARAMETER_UPP((userRoutine), uppFileTransferNotificationProcInfo, (hFT), (pFSSpec))
#define CallFileTransferChooseIdleProc(userRoutine) 			CALL_ZERO_PARAMETER_UPP((userRoutine), uppFileTransferChooseIdleProcInfo)

EXTERN_API( FTErr )
InitFT							(void);

EXTERN_API( Handle )
FTGetVersion					(FTHandle 				hFT);

EXTERN_API( short )
FTGetFTVersion					(void);

EXTERN_API( FTHandle )
FTNew							(short 					procID,
								 FTFlags 				flags,
								 FileTransferSendUPP 	sendProc,
								 FileTransferReceiveUPP  recvProc,
								 FileTransferReadUPP 	readProc,
								 FileTransferWriteUPP 	writeProc,
								 FileTransferEnvironsUPP  environsProc,
								 WindowPtr 				owner,
								 long 					refCon,
								 long 					userData);

EXTERN_API( void )
FTDispose						(FTHandle 				hFT);

EXTERN_API( FTErr )
FTStart							(FTHandle 				hFT,
								 FTDirection 			direction,
								 const SFReply *		fileInfo);

EXTERN_API( FTErr )
FTAbort							(FTHandle 				hFT);

EXTERN_API( FTErr )
FTSend							(FTHandle 				hFT,
								 short 					numFiles,
								 FSSpecArrayPtr 		pFSSpec,
								 FileTransferNotificationUPP  notifyProc);

EXTERN_API( FTErr )
FTReceive						(FTHandle 				hFT,
								 FSSpecPtr 				pFSSpec,
								 FileTransferNotificationUPP  notifyProc);

EXTERN_API( void )
FTExec							(FTHandle 				hFT);

EXTERN_API( void )
FTActivate						(FTHandle 				hFT,
								 Boolean 				activate);

EXTERN_API( void )
FTResume						(FTHandle 				hFT,
								 Boolean 				resume);

EXTERN_API( Boolean )
FTMenu							(FTHandle 				hFT,
								 short 					menuID,
								 short 					item);

EXTERN_API( short )
FTChoose						(FTHandle *				hFT,
								 Point 					where,
								 FileTransferChooseIdleUPP  idleProc);

EXTERN_API( void )
FTEvent							(FTHandle 				hFT,
								 const EventRecord *	theEvent);

EXTERN_API( Boolean )
FTValidate						(FTHandle 				hFT);

EXTERN_API( void )
FTDefault						(Ptr *					theConfig,
								 short 					procID,
								 Boolean 				allocate);

EXTERN_API( Handle )
FTSetupPreflight				(short 					procID,
								 long *					magicCookie);

EXTERN_API( void )
FTSetupSetup					(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 long *					magicCookie);

EXTERN_API( Boolean )
FTSetupFilter					(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 EventRecord *			theEvent,
								 short *				theItem,
								 long *					magicCookie);

EXTERN_API( void )
FTSetupItem						(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 short *				theItem,
								 long *					magicCookie);

EXTERN_API( void )
FTSetupXCleanup					(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 Boolean 				OKed,
								 long *					magicCookie);

EXTERN_API( void )
FTSetupPostflight				(short 					procID);

EXTERN_API( Ptr )
FTGetConfig						(FTHandle 				hFT);

EXTERN_API( short )
FTSetConfig						(FTHandle 				hFT,
								 const void *			thePtr);

EXTERN_API( FTErr )
FTIntlToEnglish					(FTHandle 				hFT,
								 const void *			inputPtr,
								 Ptr *					outputPtr,
								 short 					language);

/* (16) chenged OSErr to FTErr*/
EXTERN_API( FTErr )
FTEnglishToIntl					(FTHandle 				hFT,
								 const void *			inputPtr,
								 Ptr *					outputPtr,
								 short 					language);

/* (16) chenged OSErr to FTErr*/
EXTERN_API( void )
FTGetToolName					(short 					procID,
								 Str255 				name);

EXTERN_API( short )
FTGetProcID						(ConstStr255Param 		name);

EXTERN_API( void )
FTSetRefCon						(FTHandle 				hFT,
								 long 					refCon);

EXTERN_API( long )
FTGetRefCon						(FTHandle 				hFT);

EXTERN_API( void )
FTSetUserData					(FTHandle 				hFT,
								 long 					userData);

EXTERN_API( long )
FTGetUserData					(FTHandle 				hFT);

EXTERN_API( void )
FTGetErrorString				(FTHandle 				hFT,
								 short 					id,
								 Str255 				errMsg);

/*
	These Async routines were added to InterfaceLib in System 7.5
*/
EXTERN_API( FTErr )
FTSendAsync						(FTHandle 				hFT,
								 short 					numFiles,
								 FSSpecArrayPtr 		pFSSpec,
								 FileTransferNotificationUPP  notifyProc);

EXTERN_API( FTErr )
FTReceiveAsync					(FTHandle 				hFT,
								 FSSpecPtr 				pFSSpec,
								 FileTransferNotificationUPP  notifyProc);

EXTERN_API( FTErr )
FTCompletionAsync				(FTHandle 				hFT,
								 FTCompletionKind 		completionCall);



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

#endif /* __FILETRANSFERS__ */

