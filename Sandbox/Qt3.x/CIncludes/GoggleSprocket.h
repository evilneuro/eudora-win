/*
 	File:		GoggleSprocket.h
 
 	Contains:	Games Sprockets: GoggleSprocket interfaces
 
 	Version:	Technology:	Goggle Sprocket 1.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1996-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __GOGGLESPROCKET__
#define __GOGGLESPROCKET__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __EVENTS__
#include <Events.h>
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

#if TARGET_CPU_PPC
/*
********************************************************************************
** constants & data types
********************************************************************************
*/
typedef CALLBACK_API_C( Boolean , GSpEventProcPtr )(EventRecord *inEvent);
/*
********************************************************************************
** prototypes for application level calls
********************************************************************************
*/
/* general */
EXTERN_API_C( OSStatus )
GSpStartup						(UInt32 				inReserved);

EXTERN_API_C( OSStatus )
GSpShutdown						(UInt32 				inReserved);

/* configuration */
EXTERN_API_C( OSStatus )
GSpConfigure					(GSpEventProcPtr 		inEventProc,
								 Point *				inUpperLeft);

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

#endif /* __GOGGLESPROCKET__ */

