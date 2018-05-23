/*
 	File:		QD3DString.h
 
 	Contains:	Q3CString methods
 
 	Version:	Technology:	Quickdraw 3D 1.5.4
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QD3DSTRING__
#define __QD3DSTRING__

#ifndef __QD3D__
#include <QD3D.h>
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
	#pragma options align=power
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint on
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=int
#elif PRAGMA_ENUM_PACK
	#if __option(pack_enums)
		#define PRAGMA_ENUM_PACK__QD3DSTRING__
	#endif
	#pragma options(!pack_enums)
#endif

/******************************************************************************
 **																			 **
 **								String Routines								 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3ObjectType )
Q3String_GetType				(TQ3StringObject 		stringObj);


/******************************************************************************
 **																			 **
 **						C String Routines									 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3StringObject )
Q3CString_New					(const char *			str);

EXTERN_API_C( TQ3Status )
Q3CString_GetLength				(TQ3StringObject 		stringObj,
								 unsigned long *		length);

EXTERN_API_C( TQ3Status )
Q3CString_SetString				(TQ3StringObject 		stringObj,
								 const char *			str);

EXTERN_API_C( TQ3Status )
Q3CString_GetString				(TQ3StringObject 		stringObj,
								 char **				str);

EXTERN_API_C( TQ3Status )
Q3CString_EmptyData				(char **				str);



#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint reset
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=reset
#elif defined(PRAGMA_ENUM_PACK__QD3DSTRING__)
	#pragma options(pack_enums)
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

#endif /* __QD3DSTRING__ */

