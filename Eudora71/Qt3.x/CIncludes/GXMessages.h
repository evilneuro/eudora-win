/*
 	File:		GXMessages.h
 
 	Contains:	This file contains all of the public data structures,
 
 	Version:	Technology:	Quickdraw GX 1.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __GXMESSAGES__
#define __GXMESSAGES__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
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

#if defined(__MWERKS__) && TARGET_CPU_68K
	#pragma push
	#pragma pointers_in_D0
#endif
 

/*

	>>>>>> CONSTANTS <<<<<<

*/
/* Message Manager Trap */

enum {
	messageManagerTrap			= 0xABFB
};


/* Message Manager Error Result Codes */

enum {
	messageStopLoopingErr		= -5775,
	cantDeleteRunningHandlerErr	= -5776,
	noMessageTableErr			= -5777,
	dupSignatureErr				= -5778,
	messageNotReceivedErr		= -5799
};


/*
	DATA TYPES
*/
typedef CALLBACK_API_C( OSErr , MessageHandlerOverrideProcPtr )(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
typedef STACK_UPP_TYPE(MessageHandlerOverrideProcPtr) 			MessageHandlerOverrideUPP;
enum { uppMessageHandlerOverrideProcInfo = 0x0003FFE1 }; 		/* 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewMessageHandlerOverrideProc(userRoutine) 				(MessageHandlerOverrideUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMessageHandlerOverrideProcInfo, GetCurrentArchitecture())
#define CallMessageHandlerOverrideProc(userRoutine, arg1, arg2, arg3, arg4, arg5, arg6)  CALL_SIX_PARAMETER_UPP((userRoutine), uppMessageHandlerOverrideProcInfo, (arg1), (arg2), (arg3), (arg4), (arg5), (arg6))
typedef CALLBACK_API_C( void , MessageGlobalsInitProcPtr )(void *messageGlobals);
typedef STACK_UPP_TYPE(MessageGlobalsInitProcPtr) 				MessageGlobalsInitUPP;
enum { uppMessageGlobalsInitProcInfo = 0x000000C1 }; 			/* no_return_value Func(4_bytes) */
#define NewMessageGlobalsInitProc(userRoutine) 					(MessageGlobalsInitUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMessageGlobalsInitProcInfo, GetCurrentArchitecture())
#define CallMessageGlobalsInitProc(userRoutine, messageGlobals)  CALL_ONE_PARAMETER_UPP((userRoutine), uppMessageGlobalsInitProcInfo, (messageGlobals))
#if OLDROUTINENAMES
typedef MessageHandlerOverrideProcPtr 	MessageHandlerOverrideProc;
typedef MessageGlobalsInitProcPtr 		MessageGlobalsInitProc;
#endif  /* OLDROUTINENAMES */

typedef struct OpaqueMessageHandler* 	MessageHandler;
typedef struct OpaqueMessageObject* 	MessageObject;
/*

	PUBLIC INTERFACES

	Message Handler API Routines
*/
EXTERN_API_C( long )
CountMessageHandlerInstances	(void)														TWOWORDINLINE(0x7028, 0xABFB);

EXTERN_API_C( void *)
GetMessageHandlerClassContext	(void)														TWOWORDINLINE(0x7029, 0xABFB);

EXTERN_API_C( void *)
SetMessageHandlerClassContext	(void *					anyValue)							TWOWORDINLINE(0x702A, 0xABFB);

EXTERN_API_C( void *)
GetMessageHandlerInstanceContext (void)														TWOWORDINLINE(0x702B, 0xABFB);

EXTERN_API_C( void *)
SetMessageHandlerInstanceContext (void *				anyValue)							TWOWORDINLINE(0x702C, 0xABFB);

EXTERN_API_C( OSErr )
NewMessageGlobals				(long 					messageGlobalsSize,
								 MessageGlobalsInitUPP 	initProc)							TWOWORDINLINE(0x702D, 0xABFB);

EXTERN_API_C( void )
DisposeMessageGlobals			(void)														TWOWORDINLINE(0x702E, 0xABFB);

/*
	Message Sending API Routines
*/


#ifndef BUILDING_GXMESSAGES
EXTERN_API_C( OSErr )
SendObjectMessage				(MessageObject 			msgObject,
								 long 					messageSelector,
								 ...)														FIVEWORDINLINE(0x205F, 0x221F, 0x702F, 0xABFB, 0x518F);

EXTERN_API_C( OSErr )
SendObjectMessageTo				(MessageObject 			msgObject,
								 long 					messageSelector,
								 MessageHandler 		msgHandler,
								 ...)														SEVENWORDINLINE(0x205F, 0x221F, 0x241F, 0x7030, 0xABFB, 0x9EFC, 0x000C);

EXTERN_API_C( OSErr )
SendObjectMessageFor			(MessageObject 			msgObject,
								 long 					messageSelector,
								 MessageHandler 		msgHandler,
								 ...)														SEVENWORDINLINE(0x205F, 0x221F, 0x241F, 0x7037, 0xABFB, 0x9EFC, 0x000C);

EXTERN_API_C( OSErr )
StartObjectMessageAt			(MessageObject 			msgObject,
								 long 					messageSelector,
								 MessageHandler 		msgHandler,
								 ...)														SEVENWORDINLINE(0x205F, 0x221F, 0x241F, 0x7031, 0xABFB, 0x9EFC, 0x000C);

#if TARGET_OS_MAC
	#define MacSendMessage SendMessage
#endif
EXTERN_API_C( OSErr )
MacSendMessage					(long 					messageSelector,
								 ...)														FOURWORDINLINE(0x221F, 0x7032, 0xABFB, 0x598F);

EXTERN_API_C( OSErr )
SendMessageTo					(long 					messageSelector,
								 MessageHandler 		msgHandler,
								 ...)														FIVEWORDINLINE(0x221F, 0x241F, 0x7033, 0xABFB, 0x518F);

EXTERN_API_C( OSErr )
StartMessageAt					(long 					messageSelector,
								 MessageHandler 		msgHandler,
								 ...)														FIVEWORDINLINE(0x221F, 0x241F, 0x7034, 0xABFB, 0x518F);

EXTERN_API_C( OSErr )
ForwardMessage					(long 					messageSelector,
								 ...)														FOURWORDINLINE(0x221F, 0x7035, 0xABFB, 0x598F);

EXTERN_API_C( OSErr )
ForwardThisMessage				(void *					parameter1,
								 ...)														TWOWORDINLINE(0x7036, 0xABFB);

#endif	/* BUILDING_GXMESSAGES */
 
#if defined(__MWERKS__) && TARGET_CPU_68K
	#pragma pop
#endif


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

#endif /* __GXMESSAGES__ */

