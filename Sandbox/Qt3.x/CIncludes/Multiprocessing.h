/*
 	File:		Multiprocessing.h
 
 	Contains:	Multiprocessing interfaces
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1996-1998 by Apple Computer, Inc. and © 1995-1997 DayStar Digital, Inc.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __MULTIPROCESSING__
#define __MULTIPROCESSING__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __CODEFRAGMENTS__
#include <CodeFragments.h>
#endif

#include <stdarg.h>


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
	#pragma options align=power
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

#if TARGET_CPU_PPC
#define MPCopyrightNotice								\
	"Copyright © 1995, 1996 DayStar Digital, Inc.\n"			\
	"Portions Copyright © 1995 Apple Computer, Inc.\n"
#define MPLibraryName "MPLibrary"
#define MPLibraryCName MPLibraryName
#define MPLibraryPName "\p" MPLibraryName
#define MP_API_Version "1.4"

enum {
	MPLibrary_MajorVersion		= 1,
	MPLibrary_MinorVersion		= 4,
	MPLibrary_Release			= 1,
	MPLibrary_DevelopmentRevision = 2
};

typedef struct OpaqueMPTaskID* 			MPTaskID;
typedef struct OpaqueMPQueueID* 		MPQueueID;
typedef struct OpaqueMPSemaphoreID* 	MPSemaphoreID;
typedef struct OpaqueMPCriticalRegionID*  MPCriticalRegionID;
typedef UInt32 							MPSemaphoreCount;
typedef UInt32 							MPTaskOptions;
typedef CALLBACK_API_C( OSStatus , TaskProc )(void *parameter);
typedef CALLBACK_API_C( void *, MPRemoteProcedure )(void *parameter);
typedef CALLBACK_API_C( void , MPPrintfHandler )(MPTaskID taskID, const char *format, va_list args);

enum {
	kDurationImmediate			= 0L,
	kDurationForever			= 0x7FFFFFFF
};


enum {
	kMPNoID						= 0
};


#define MPLibraryIsLoaded()		\
  ( ( (UInt32)_MPIsFullyInitialized != (UInt32)kUnresolvedCFragSymbolAddress ) \
   && _MPIsFullyInitialized() )


EXTERN_API_C( UInt32 )
MPProcessors					(void);

EXTERN_API_C( OSStatus )
MPCreateTask					(TaskProc 				entryPoint,
								 void *					parameter,
								 ByteCount 				stackSize,
								 MPQueueID 				notifyQueue,
								 void *					terminationParameter1,
								 void *					terminationParameter2,
								 MPTaskOptions 			options,
								 MPTaskID *				task);


EXTERN_API_C( OSStatus )
MPTerminateTask					(MPTaskID 				task,
								 OSStatus 				terminationStatus);


EXTERN_API_C( void )
MPExit							(OSStatus 				status);


EXTERN_API_C( MPTaskID )
MPCurrentTaskID					(void);


EXTERN_API_C( void )
MPYield							(void);


EXTERN_API_C( OSStatus )
MPCreateQueue					(MPQueueID *			queue);


EXTERN_API_C( OSStatus )
MPDeleteQueue					(MPQueueID 				queue);


EXTERN_API_C( OSStatus )
MPNotifyQueue					(MPQueueID 				queue,
								 void *					param1,
								 void *					param2,
								 void *					param3);


EXTERN_API_C( OSStatus )
MPWaitOnQueue					(MPQueueID 				queue,
								 void **				param1,
								 void **				param2,
								 void **				param3,
								 Duration 				timeout);


EXTERN_API_C( OSStatus )
MPCreateSemaphore				(MPSemaphoreCount 		maximumValue,
								 MPSemaphoreCount 		initialValue,
								 MPSemaphoreID *		semaphore);


#define MPCreateBinarySemaphore(semaphore) \
        MPCreateSemaphore(1, 1, (semaphore))

EXTERN_API_C( OSStatus )
MPWaitOnSemaphore				(MPSemaphoreID 			semaphore,
								 Duration 				timeout);


EXTERN_API_C( OSStatus )
MPSignalSemaphore				(MPSemaphoreID 			semaphore);


EXTERN_API_C( OSStatus )
MPDeleteSemaphore				(MPSemaphoreID 			semaphore);


EXTERN_API_C( OSStatus )
MPCreateCriticalRegion			(MPCriticalRegionID *	criticalRegion);


EXTERN_API_C( OSStatus )
MPEnterCriticalRegion			(MPCriticalRegionID 	criticalRegion,
								 Duration 				timeout);


EXTERN_API_C( OSStatus )
MPExitCriticalRegion			(MPCriticalRegionID 	criticalRegion);


EXTERN_API_C( OSStatus )
MPDeleteCriticalRegion			(MPCriticalRegionID 	criticalRegion);


EXTERN_API_C( LogicalAddress )
MPAllocate						(ByteCount 				size);

EXTERN_API_C( void )
MPFree							(LogicalAddress 		object);


EXTERN_API_C( void )
MPBlockCopy						(LogicalAddress 		sourcePtr,
								 LogicalAddress 		destPtr,
								 ByteCount 				blockSize);


/**************************************************************************
 *
 *	MPTaskIsToolboxSafe() and MPRPC() were functions added by DayStar. 
 *	The 1.4 MPLibrary exports the names with an underscore prefix. 
 *	To work around this, #defines have been added to automatically
 *	add the underscore.
 *
 */

#define	MPTaskIsToolboxSafe		_MPTaskIsToolboxSafe
#define	MPRPC					_MPRPC

/* 
	MPTaskIsToolboxSafe() allows routines which are otherwise unaware that 
	they are being called from an MP task to check to see if it is permissible
	to make a call to the Macintosh toolbox.  (It is okay to make toolbox 
	calls only if the routine is not being called from an MP task).
*/
EXTERN_API_C( Boolean )
MPTaskIsToolboxSafe				(MPTaskID 				task);

/*
	MPRPC() calls its MPRemoteProcedure parameter in the application's main 
	thread when the app next calls WaitNextEvent, EventAvail, SystemTask, 
	MPYield, MPWaitOnQueue, MPWaitOnSemaphore, or MPEnterCriticalRegion. The  
	return value of the MPRemoteProcedure is returned as the result of MPRPC.  
	The MPRemoteProcedure function can call any toolbox function except  
	SystemTask (or anything that calls it).
*/
EXTERN_API_C( void *)
MPRPC							(MPRemoteProcedure 		theProc,
								 void *					parameter);



/**************************************************************************
 *
 *	The following routines were added by DayStar for debugging purposes.
 *	You should not use these in shipping products.  You can tell which
 *	functions are for debugging only because they begin with an underscore
 *
 */
EXTERN_API_C( Boolean )
_MPIsFullyInitialized			(void);


/*
	MPAllocateSys() does the same thing as MPAllocate() except the memory
	is allocated from the system heap.
*/
EXTERN_API_C( LogicalAddress )
_MPAllocateSys					(ByteCount 				size);


EXTERN_API_C( Boolean )
_MPLibraryIsCompatible			(const char *			versionCString,
								 UInt32 				major,
								 UInt32 				minor,
								 UInt32 				release,
								 UInt32 				revision);

/*
	MPLibraryVersion() retrieves the hardcoded version information built into
	the currently active Multiprocessing API Library.
*/
EXTERN_API_C( void )
_MPLibraryVersion				(const char **			versionCString,
								 UInt32 *				major,
								 UInt32 *				minor,
								 UInt32 *				release,
								 UInt32 *				revision);

EXTERN_API_C( void )
_MPInitializePrintf				(MPPrintfHandler 		pfn);

EXTERN_API_C( void )
_MPPrintf						(const char *			format,
								 ...);


/*
 	MPDebugStr() works just like DebugStr() except that it is safe
	to call it from an MP task.
*/
EXTERN_API_C( void )
_MPDebugStr						(ConstStr255Param 		msg);

/*

	MPStatusPString() and MPStatusCString() provide a way to translate an OSStatus
	value returned from one of the MP API calls into either a Pascal string or a
	C string.  Thus, if an MPLibrary function returns an error then the application
	(not a task) could use the following:

    status = MPxxx( function_params );
    DebugStr( MPStatusPString( status ) );
	
*/
EXTERN_API_C( StringPtr )
_MPStatusPString				(OSStatus 				status);

EXTERN_API_C( const char *)
_MPStatusCString				(OSStatus 				status);


#endif  /* TARGET_CPU_PPC */


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

#endif /* __MULTIPROCESSING__ */

