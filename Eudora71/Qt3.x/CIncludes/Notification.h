/*
 	File:		Notification.h
 
 	Contains:	Notification Manager interfaces
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1989-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __NOTIFICATION__
#define __NOTIFICATION__

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

typedef struct NMRec 					NMRec;
typedef NMRec *							NMRecPtr;
typedef CALLBACK_API( void , NMProcPtr )(NMRecPtr nmReqPtr);
typedef STACK_UPP_TYPE(NMProcPtr) 								NMUPP;

struct NMRec {
	QElemPtr 						qLink;						/* next queue entry*/
	short 							qType;						/* queue type -- ORD(nmType) = 8*/
	short 							nmFlags;					/* reserved*/
	long 							nmPrivate;					/* reserved*/
	short 							nmReserved;					/* reserved*/
	short 							nmMark;						/* item to mark in Apple menu*/
	Handle 							nmIcon;						/* handle to small icon*/
	Handle 							nmSound;					/* handle to sound record*/
	StringPtr 						nmStr;						/* string to appear in alert*/
	NMUPP 							nmResp;						/* pointer to response routine*/
	long 							nmRefCon;					/* for application use*/
};

enum { uppNMProcInfo = 0x000000C0 }; 							/* pascal no_return_value Func(4_bytes) */
#define CallNMProc(userRoutine, nmReqPtr) 						CALL_ONE_PARAMETER_UPP((userRoutine), uppNMProcInfo, (nmReqPtr))
#define NewNMProc(userRoutine) 									(NMUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNMProcInfo, GetCurrentArchitecture())
																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 NMInstall(__A0)
																							#endif
EXTERN_API( OSErr )
NMInstall						(NMRecPtr 				nmReqPtr)							ONEWORDINLINE(0xA05E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 NMRemove(__A0)
																							#endif
EXTERN_API( OSErr )
NMRemove						(NMRecPtr 				nmReqPtr)							ONEWORDINLINE(0xA05F);



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

#endif /* __NOTIFICATION__ */

