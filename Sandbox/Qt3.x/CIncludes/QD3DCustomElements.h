/*
 	File:		QD3DCustomElements.h
 
 	Contains:	Custom QuickTime Elements in QuickDraw 3D							
 
 	Version:	Technology:	Quickdraw 3D 1.5.4
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QD3DCUSTOMELEMENTS__
#define __QD3DCUSTOMELEMENTS__

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
		#define PRAGMA_ENUM_PACK__QD3DCUSTOMELEMENTS__
	#endif
	#pragma options(!pack_enums)
#endif


/******************************************************************************
 **																			 **
 **						Name Data Structure Definitions						 **
 **																			 **
 *****************************************************************************/
#define	CEcNameElementName	"Apple Computer, Inc.:NameElement"


/******************************************************************************
 **																			 **
 **						Custom Name Element Functions						 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3Status )
CENameElement_SetData			(TQ3Object 				object,
								 const char *			name);

EXTERN_API_C( TQ3Status )
CENameElement_GetData			(TQ3Object 				object,
								 char **				name);

EXTERN_API_C( TQ3Status )
CENameElement_EmptyData			(char **				name);


/******************************************************************************
 **																			 **
 **							URL Data Structure Definitions					 **
 **																			 **
 *****************************************************************************/
#define	CEcUrlElementName		"Apple Computer, Inc.:URLElement"

enum TCEUrlOptions {
	kCEUrlOptionNone			= 0,
	kCEUrlOptionUseMap			= 1
};
typedef enum TCEUrlOptions TCEUrlOptions;


struct TCEUrlData {
	char *							url;
	char *							description;
	TCEUrlOptions 					options;
};
typedef struct TCEUrlData				TCEUrlData;
/******************************************************************************
 **																			 **
 **						Custom URL Element Functions						 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3Status )
CEUrlElement_SetData			(TQ3Object 				object,
								 TCEUrlData *			urlData);

EXTERN_API_C( TQ3Status )
CEUrlElement_GetData			(TQ3Object 				object,
								 TCEUrlData **			urlData);

EXTERN_API_C( TQ3Status )
CEUrlElement_EmptyData			(TCEUrlData **			urlData);




#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint reset
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=reset
#elif defined(PRAGMA_ENUM_PACK__QD3DCUSTOMELEMENTS__)
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

#endif /* __QD3DCUSTOMELEMENTS__ */

