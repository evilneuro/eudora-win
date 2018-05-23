/*
 	File:		OSAComp.h
 
 	Contains:	AppleScript Component Implementor's Interfaces.
 
 	Version:	Technology:	AppleScript 1.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1992-1995, 1997-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __OSACOMP__
#define __OSACOMP__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __AEDATAMODEL__
#include <AEDataModel.h>
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

/**************************************************************************
	Types and Constants
**************************************************************************/
/**************************************************************************
	Routines for Associating a Storage Type with a Script Data Handle 
**************************************************************************/
EXTERN_API( OSErr )
OSAGetStorageType				(Handle 				scriptData,
								 DescType *				dscType);

EXTERN_API( OSErr )
OSAAddStorageType				(Handle 				scriptData,
								 DescType 				dscType);

EXTERN_API( OSErr )
OSARemoveStorageType			(Handle 				scriptData);



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

#endif /* __OSACOMP__ */

