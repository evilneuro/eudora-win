/*
 	File:		QTML.h
 
 	Contains:	QuickTime Cross-platform specific interfaces
 
 	Version:	Technology:	QuickTime 3.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QTML__
#define __QTML__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MACMEMORY__
#include <MacMemory.h>
#endif
#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif
#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
#ifndef __FILES__
#include <Files.h>
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

EXTERN_API( void )
QTMLYieldCPU					(void);

EXTERN_API( void )
QTMLYieldCPUTime				(long 					milliSeconds,
								 unsigned long 			flags);

#if !TARGET_OS_MAC
typedef long 							QTMLMutex;
typedef struct OpaqueQTMLSyncVar* 		QTMLSyncVar;
typedef QTMLSyncVar *					QTMLSyncVarPtr;

enum {
	kInitializeQTMLNoSoundFlag	= (1L << 0),					/* flag for requesting no sound when calling InitializeQTML*/
	kInitializeQTMLUseGDIFlag	= (1L << 1)						/* flag for requesting GDI when calling InitializeQTML*/
};


enum {
	kQTMLHandlePortEvents		= (1L << 0),					/* flag for requesting requesting QTML to handle events*/
	kQTMLNoIdleEvents			= (1L << 1)						/* flag for requesting requesting QTML not to send Idle Events*/
};

EXTERN_API( OSErr )
InitializeQTML					(long 					flag);

EXTERN_API( void )
TerminateQTML					(void);

EXTERN_API( GrafPtr )
CreatePortAssociation			(void *					theWnd,
								 Ptr 					storage,
								 long 					flags);

EXTERN_API( void )
DestroyPortAssociation			(CGrafPtr 				cgp);

EXTERN_API( void )
QTMLGrabMutex					(QTMLMutex 				mu);

EXTERN_API( void )
QTMLReturnMutex					(QTMLMutex 				mu);

EXTERN_API( QTMLMutex )
QTMLCreateMutex					(void);

EXTERN_API( void )
QTMLDestroyMutex				(QTMLMutex 				mu);

EXTERN_API( QTMLSyncVarPtr )
QTMLCreateSyncVar				(void);

EXTERN_API( void )
QTMLDestroySyncVar				(QTMLSyncVarPtr 		p);

EXTERN_API( long )
QTMLTestAndSetSyncVar			(QTMLSyncVarPtr 		sync);

EXTERN_API( void )
QTMLWaitAndSetSyncVar			(QTMLSyncVarPtr 		sync);

EXTERN_API( void )
QTMLResetSyncVar				(QTMLSyncVarPtr 		sync);

EXTERN_API( void )
InitializeQHdr					(QHdr *					qhdr);

EXTERN_API( void )
TerminateQHdr					(QHdr *					qhdr);

EXTERN_API( void )
QTMLAcquireWindowList			(void);

EXTERN_API( void )
QTMLReleaseWindowList			(void);

/*
   These routines are here to support "interrupt level" code
      These are dangerous routines, only use if you know what you are doing.
*/

EXTERN_API( long )
QTMLRegisterInterruptSafeThread	(unsigned long 			threadID,
								 void *					threadInfo);

EXTERN_API( long )
QTMLUnregisterInterruptSafeThread (unsigned long 		threadID);

EXTERN_API( long )
NativeEventToMacEvent			(void *					nativeEvent,
								 EventRecord *			macEvent);

#if TARGET_OS_WIN32
EXTERN_API( long )
WinEventToMacEvent				(void *					winMsg,
								 EventRecord *			macEvent);

#define WinEventToMacEvent	NativeEventToMacEvent
EXTERN_API( Boolean )
IsTaskBarVisible				(void);

EXTERN_API( void )
ShowHideTaskBar					(Boolean 				showIt);


enum {
	kDDSurfaceLocked			= (1L << 0),
	kDDSurfaceStatic			= (1L << 1)
};

EXTERN_API( OSErr )
QTGetDDObject					(void **				lpDDObject);

EXTERN_API( OSErr )
QTSetDDObject					(void *					lpNewDDObject);

EXTERN_API( OSErr )
QTSetDDPrimarySurface			(void *					lpNewDDSurface,
								 unsigned long 			flags);

EXTERN_API( OSErr )
QTMLGetVolumeRootPath			(char *					fullPath,
								 char *					volumeRootPath,
								 unsigned long 			volumeRootLen);

EXTERN_API( void )
QTMLSetWindowWndProc			(WindowPtr 				theWindow,
								 void *					windowProc);

EXTERN_API( void *)
QTMLGetWindowWndProc			(WindowPtr 				theWindow);

#endif  /* TARGET_OS_WIN32 */

EXTERN_API( OSErr )
QTMLGetCanonicalPathName		(char *					inName,
								 char *					outName,
								 unsigned long 			outLen);


enum {
	kFullNativePath				= 0,
	kFileNameOnly				= (1 << 0),
	kDirectoryPathOnly			= (1 << 1)
};

EXTERN_API( OSErr )
FSSpecToNativePathName			(const FSSpec *			inFile,
								 char *					outName,
								 unsigned long 			outLen,
								 long 					flags);

EXTERN_API( OSErr )
NativePathNameToFSSpec			(char *					inName,
								 FSSpec *				outFile,
								 long 					flags);

#endif  /*  !TARGET_OS_MAC */

#if TARGET_OS_MAC

#define QTMLYieldCPU()
#define QTMLYieldCPUTime(a,b)

#endif  /* TARGET_OS_MAC */


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

#endif /* __QTML__ */

