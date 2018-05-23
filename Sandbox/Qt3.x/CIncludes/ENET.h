/*
 	File:		ENET.h
 
 	Contains:	Ethernet Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __ENET__
#define __ENET__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __OSUTILS__
#include <OSUtils.h>
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
	ENetSetGeneral				= 253,							/*Set "general" mode*/
	ENetGetInfo					= 252,							/*Get info*/
	ENetRdCancel				= 251,							/*Cancel read*/
	ENetRead					= 250,							/*Read*/
	ENetWrite					= 249,							/*Write*/
	ENetDetachPH				= 248,							/*Detach protocol handler*/
	ENetAttachPH				= 247,							/*Attach protocol handler*/
	ENetAddMulti				= 246,							/*Add a multicast address*/
	ENetDelMulti				= 245							/*Delete a multicast address*/
};


enum {
	EAddrRType					= FOUR_CHAR_CODE('eadr')		/*Alternate address resource type*/
};

typedef struct EParamBlock 				EParamBlock;
typedef EParamBlock *					EParamBlkPtr;
typedef CALLBACK_API( void , ENETCompletionProcPtr )(EParamBlkPtr thePBPtr);
/*
	WARNING: ENETCompletionProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef REGISTER_UPP_TYPE(ENETCompletionProcPtr) 				ENETCompletionUPP;

struct EParamBlock {
	QElem *							qLink;						/*General EParams*/
	short 							qType;						/*queue type*/
	short 							ioTrap;						/*routine trap*/
	Ptr 							ioCmdAddr;					/*routine address*/
	ENETCompletionUPP 				ioCompletion;				/*completion routine*/
	OSErr 							ioResult;					/*result code*/
	StringPtr 						ioNamePtr;					/*->filename*/
	short 							ioVRefNum;					/*volume reference or drive number*/
	short 							ioRefNum;					/*driver reference number*/
	short 							csCode;						/*Call command code*/
	union {
		struct {
			short 							eProtType;			/*Ethernet protocol type*/
			Ptr 							ePointer;			/*No support for PowerPC code*/
			short 							eBuffSize;			/*buffer size*/
			short 							eDataSize;			/*number of bytes read*/
		} 								EParms1;
		struct {
			Byte 							eMultiAddr[6];		/*Multicast Address*/
		} 								EParms2;
	} 								u;
};

enum { uppENETCompletionProcInfo = 0x00009802 }; 				/* register no_return_value Func(4_bytes:A0) */
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter CallENETCompletionProc(__A1, __A0)
	void CallENETCompletionProc(ENETCompletionUPP routine, EParamBlkPtr thePBPtr) = 0x4E91;
#else
	#define CallENETCompletionProc(userRoutine, thePBPtr) 		CALL_ONE_PARAMETER_UPP((userRoutine), uppENETCompletionProcInfo, (thePBPtr))
#endif
#define NewENETCompletionProc(userRoutine) 						(ENETCompletionUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppENETCompletionProcInfo, GetCurrentArchitecture())
EXTERN_API( OSErr )
EWrite							(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
EAttachPH						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
EDetachPH						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
ERead							(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
ERdCancel						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
EGetInfo						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
ESetGeneral						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
EAddMulti						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);

EXTERN_API( OSErr )
EDelMulti						(EParamBlkPtr 			thePBptr,
								 Boolean 				async);




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

#endif /* __ENET__ */

