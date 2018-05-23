/*
 	File:		Threads.h
 
 	Contains:	Thread Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1991-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __THREADS__
#define __THREADS__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif
#ifndef __ERRORS__
#include <Errors.h>
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

/* Thread states */
typedef unsigned short 					ThreadState;

enum {
	kReadyThreadState			= 0,
	kStoppedThreadState			= 1,
	kRunningThreadState			= 2
};

/* Error codes have been meoved to Errors.(pah) */
/* Thread environment characteristics */
typedef void *							ThreadTaskRef;
/* Thread characteristics */
typedef unsigned long 					ThreadStyle;

enum {
	kCooperativeThread			= 1L << 0,
	kPreemptiveThread			= 1L << 1
};

/* Thread identifiers */
typedef unsigned long 					ThreadID;

enum {
	kNoThreadID					= 0,
	kCurrentThreadID			= 1,
	kApplicationThreadID		= 2
};

/* Options when creating a thread */
typedef unsigned long 					ThreadOptions;

enum {
	kNewSuspend					= (1 << 0),
	kUsePremadeThread			= (1 << 1),
	kCreateIfNeeded				= (1 << 2),
	kFPUNotNeeded				= (1 << 3),
	kExactMatchThread			= (1 << 4)
};

/* Information supplied to the custom scheduler */

struct SchedulerInfoRec {
	unsigned long 					InfoRecSize;
	ThreadID 						CurrentThreadID;
	ThreadID 						SuggestedThreadID;
	ThreadID 						InterruptedCoopThreadID;
};
typedef struct SchedulerInfoRec			SchedulerInfoRec;
typedef SchedulerInfoRec *				SchedulerInfoRecPtr;

#if TARGET_CPU_68K && TARGET_RT_MAC_CFM
/*
	The following UniversalProcPtrs are for CFM-68k compatiblity with
	the implementation of the Thread Manager.
*/

typedef UniversalProcPtr 				ThreadEntryProcPtr;
typedef UniversalProcPtr 				ThreadSchedulerProcPtr;
typedef UniversalProcPtr 				ThreadSwitchProcPtr;
typedef UniversalProcPtr 				ThreadTerminationProcPtr;
typedef UniversalProcPtr 				DebuggerNewThreadProcPtr;
typedef UniversalProcPtr 				DebuggerDisposeThreadProcPtr;
typedef UniversalProcPtr 				DebuggerThreadSchedulerProcPtr;
enum {
	uppThreadEntryProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(void*)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(void*))),
	uppThreadSchedulerProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ThreadID)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(SchedulerInfoRecPtr))),
	uppThreadSwitchProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ThreadID)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void*))),
	uppThreadTerminationProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ThreadID)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void*))),
	uppDebuggerNewThreadProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ThreadID))),
	uppDebuggerDisposeThreadProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ThreadID))),
	uppDebuggerThreadSchedulerProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ThreadID)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(SchedulerInfoRecPtr)))
};
#define NewThreadEntryProc(userRoutine)				(ThreadEntryProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppThreadEntryProcInfo, GetCurrentArchitecture())
#define NewThreadSchedulerProc(userRoutine)			(ThreadSchedulerProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppThreadSchedulerProcInfo, GetCurrentArchitecture())
#define NewThreadSwitchProc(userRoutine)			(ThreadSwitchProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppThreadSwitchProcInfo, GetCurrentArchitecture())
#define NewThreadTerminationProc(userRoutine)		(ThreadTerminationProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppThreadTerminationProcInfo, GetCurrentArchitecture())
#define NewDebuggerNewThreadProc(userRoutine)		(DebuggerNewThreadProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDebuggerNewThreadProcInfo, GetCurrentArchitecture())
#define NewDebuggerDisposeThreadProc(userRoutine)	(DebuggerDisposeThreadProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDebuggerDisposeThreadProcInfo, GetCurrentArchitecture())
#define NewDebuggerThreadSchedulerProc(userRoutine)	(DebuggerThreadSchedulerProcPtr) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDebuggerThreadSchedulerProcInfo, GetCurrentArchitecture())
#else
/*
	The following ProcPtrs cannot be interchanged with UniversalProcPtrs because
	of differences between 680x0 and PowerPC runtime architectures with regard to
	the implementation of the Thread Manager.
 */
/* Prototype for thread's entry (main) routine */
typedef void *							voidPtr;
typedef CALLBACK_API( voidPtr , ThreadEntryProcPtr )(void *threadParam);
/* Prototype for custom thread scheduler routine */
typedef CALLBACK_API( ThreadID , ThreadSchedulerProcPtr )(SchedulerInfoRecPtr schedulerInfo);
/* Prototype for custom thread switcher routine */
typedef CALLBACK_API( void , ThreadSwitchProcPtr )(ThreadID threadBeingSwitched, void *switchProcParam);
/* Prototype for thread termination notification routine */
typedef CALLBACK_API( void , ThreadTerminationProcPtr )(ThreadID threadTerminated, void *terminationProcParam);
/* Prototype for debugger NewThread notification */
typedef CALLBACK_API( void , DebuggerNewThreadProcPtr )(ThreadID threadCreated);
/* Prototype for debugger DisposeThread notification */
typedef CALLBACK_API( void , DebuggerDisposeThreadProcPtr )(ThreadID threadDeleted);
/* Prototype for debugger schedule notification */
typedef CALLBACK_API( ThreadID , DebuggerThreadSchedulerProcPtr )(SchedulerInfoRecPtr schedulerInfo);
#endif  /* TARGET_CPU_68K && TARGET_RT_MAC_CFM */


/* Thread Manager routines */
EXTERN_API( OSErr )
CreateThreadPool				(ThreadStyle 			threadStyle,
								 short 					numToCreate,
								 Size 					stackSize)							THREEWORDINLINE(0x303C, 0x0501, 0xABF2);

EXTERN_API( OSErr )
GetFreeThreadCount				(ThreadStyle 			threadStyle,
								 short *				freeCount)							THREEWORDINLINE(0x303C, 0x0402, 0xABF2);

EXTERN_API( OSErr )
GetSpecificFreeThreadCount		(ThreadStyle 			threadStyle,
								 Size 					stackSize,
								 short *				freeCount)							THREEWORDINLINE(0x303C, 0x0615, 0xABF2);

EXTERN_API( OSErr )
GetDefaultThreadStackSize		(ThreadStyle 			threadStyle,
								 Size *					stackSize)							THREEWORDINLINE(0x303C, 0x0413, 0xABF2);

EXTERN_API( OSErr )
ThreadCurrentStackSpace			(ThreadID 				thread,
								 unsigned long *		freeStack)							THREEWORDINLINE(0x303C, 0x0414, 0xABF2);

EXTERN_API( OSErr )
NewThread						(ThreadStyle 			threadStyle,
								 ThreadEntryProcPtr 	threadEntry,
								 void *					threadParam,
								 Size 					stackSize,
								 ThreadOptions 			options,
								 void **				threadResult,
								 ThreadID *				threadMade)							THREEWORDINLINE(0x303C, 0x0E03, 0xABF2);

EXTERN_API( OSErr )
DisposeThread					(ThreadID 				threadToDump,
								 void *					threadResult,
								 Boolean 				recycleThread)						THREEWORDINLINE(0x303C, 0x0504, 0xABF2);

EXTERN_API( OSErr )
YieldToThread					(ThreadID 				suggestedThread)					THREEWORDINLINE(0x303C, 0x0205, 0xABF2);

EXTERN_API( OSErr )
YieldToAnyThread				(void)														FOURWORDINLINE(0x42A7, 0x303C, 0x0205, 0xABF2);

#if TARGET_OS_MAC
	#define MacGetCurrentThread GetCurrentThread
#endif
EXTERN_API( OSErr )
MacGetCurrentThread				(ThreadID *				currentThreadID)					THREEWORDINLINE(0x303C, 0x0206, 0xABF2);

EXTERN_API( OSErr )
GetThreadState					(ThreadID 				threadToGet,
								 ThreadState *			threadState)						THREEWORDINLINE(0x303C, 0x0407, 0xABF2);

EXTERN_API( OSErr )
SetThreadState					(ThreadID 				threadToSet,
								 ThreadState 			newState,
								 ThreadID 				suggestedThread)					THREEWORDINLINE(0x303C, 0x0508, 0xABF2);

EXTERN_API( OSErr )
SetThreadStateEndCritical		(ThreadID 				threadToSet,
								 ThreadState 			newState,
								 ThreadID 				suggestedThread)					THREEWORDINLINE(0x303C, 0x0512, 0xABF2);

EXTERN_API( OSErr )
SetThreadScheduler				(ThreadSchedulerProcPtr  threadScheduler)					THREEWORDINLINE(0x303C, 0x0209, 0xABF2);

EXTERN_API( OSErr )
SetThreadSwitcher				(ThreadID 				thread,
								 ThreadSwitchProcPtr 	threadSwitcher,
								 void *					switchProcParam,
								 Boolean 				inOrOut)							THREEWORDINLINE(0x303C, 0x070A, 0xABF2);

EXTERN_API( OSErr )
SetThreadTerminator				(ThreadID 				thread,
								 ThreadTerminationProcPtr  threadTerminator,
								 void *					terminationProcParam)				THREEWORDINLINE(0x303C, 0x0611, 0xABF2);

EXTERN_API( OSErr )
ThreadBeginCritical				(void)														THREEWORDINLINE(0x303C, 0x000B, 0xABF2);

EXTERN_API( OSErr )
ThreadEndCritical				(void)														THREEWORDINLINE(0x303C, 0x000C, 0xABF2);

EXTERN_API( OSErr )
SetDebuggerNotificationProcs	(DebuggerNewThreadProcPtr  notifyNewThread,
								 DebuggerDisposeThreadProcPtr  notifyDisposeThread,
								 DebuggerThreadSchedulerProcPtr  notifyThreadScheduler)		THREEWORDINLINE(0x303C, 0x060D, 0xABF2);

EXTERN_API( OSErr )
GetThreadCurrentTaskRef			(ThreadTaskRef *		threadTRef)							THREEWORDINLINE(0x303C, 0x020E, 0xABF2);

EXTERN_API( OSErr )
GetThreadStateGivenTaskRef		(ThreadTaskRef 			threadTRef,
								 ThreadID 				threadToGet,
								 ThreadState *			threadState)						THREEWORDINLINE(0x303C, 0x060F, 0xABF2);

EXTERN_API( OSErr )
SetThreadReadyGivenTaskRef		(ThreadTaskRef 			threadTRef,
								 ThreadID 				threadToSet)						THREEWORDINLINE(0x303C, 0x0410, 0xABF2);


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

#endif /* __THREADS__ */

