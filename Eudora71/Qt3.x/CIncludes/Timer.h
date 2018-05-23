/*
 	File:		Timer.h
 
 	Contains:	Time Manager interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1993, 1995-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __TIMER__
#define __TIMER__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
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
																/* high bit of qType is set if task is active */
	kTMTaskActive				= (1L << 15)
};

typedef struct TMTask 					TMTask;
typedef TMTask *						TMTaskPtr;
typedef CALLBACK_API( void , TimerProcPtr )(TMTaskPtr tmTaskPtr);
/*
	WARNING: TimerProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef REGISTER_UPP_TYPE(TimerProcPtr) 						TimerUPP;

struct TMTask {
	QElemPtr 						qLink;
	short 							qType;
	TimerUPP 						tmAddr;
	long 							tmCount;
	long 							tmWakeUp;
	long 							tmReserved;
};

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter InsTime(__A0)
																							#endif
EXTERN_API( void )
InsTime							(QElemPtr 				tmTaskPtr)							ONEWORDINLINE(0xA058);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter InsXTime(__A0)
																							#endif
EXTERN_API( void )
InsXTime						(QElemPtr 				tmTaskPtr)							ONEWORDINLINE(0xA458);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter PrimeTime(__A0, __D0)
																							#endif
EXTERN_API( void )
PrimeTime						(QElemPtr 				tmTaskPtr,
								 long 					count)								ONEWORDINLINE(0xA05A);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter RmvTime(__A0)
																							#endif
EXTERN_API( void )
RmvTime							(QElemPtr 				tmTaskPtr)							ONEWORDINLINE(0xA059);

EXTERN_API( void )
Microseconds					(UnsignedWide *			microTickCount)						FOURWORDINLINE(0xA193, 0x225F, 0x22C8, 0x2280);

enum { uppTimerProcInfo = 0x0000B802 }; 						/* register no_return_value Func(4_bytes:A1) */
#define NewTimerProc(userRoutine) 								(TimerUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTimerProcInfo, GetCurrentArchitecture())
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter CallTimerProc(__A0, __A1)
	void CallTimerProc(TimerUPP routine, TMTaskPtr tmTaskPtr) = 0x4E90;
#else
	#define CallTimerProc(userRoutine, tmTaskPtr) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppTimerProcInfo, (tmTaskPtr))
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

#endif /* __TIMER__ */

