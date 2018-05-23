/*
 	File:		SegLoad.h
 
 	Contains:	Segment Loader Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1993, 1995-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __SEGLOAD__
#define __SEGLOAD__

#ifndef __MACTYPES__
#include <MacTypes.h>
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

#if TARGET_CPU_68K && !TARGET_RT_MAC_CFM
/*
   CountAppFiles, GetAppFiles, ClrAppFiles, GetAppParms, getappparms, 
   and the AppFile data structure and enums are obsolete. 
   They are still supported for writing old style 68K apps, 
   but they are not supported for CFM-based apps.
   Use AppleEvents to determine which files are to be 
   opened or printed from the Finder.
*/

enum {
	appOpen						= 0,							/*Open the Document (s)*/
	appPrint					= 1								/*Print the Document (s)*/
};


struct AppFile {
	short 							vRefNum;
	OSType 							fType;
	short 							versNum;					/*versNum in high byte*/
	Str255 							fName;
};
typedef struct AppFile					AppFile;
EXTERN_API( void )
CountAppFiles					(short *				message,
								 short *				count);

EXTERN_API( void )
GetAppFiles						(short 					index,
								 AppFile *				theFile);

EXTERN_API( void )
ClrAppFiles						(short 					index);

EXTERN_API( void )
GetAppParms						(Str255 				apName,
								 short *				apRefNum,
								 Handle *				apParam)							ONEWORDINLINE(0xA9F5);

#if CGLUESUPPORTED
EXTERN_API_C( void )
getappparms						(char *					apName,
								 short *				apRefNum,
								 Handle *				apParam);

#endif  /* CGLUESUPPORTED */

#endif  /* TARGET_CPU_68K &&  !TARGET_RT_MAC_CFM */

 
/*
   Because PowerPC applications don’t have segments.
   But, in order to allow applications to not have conditionalized
   source code, UnloadSeg is macro'ed away for PowerPC.
*/
#if TARGET_CPU_68K
EXTERN_API( void )
UnloadSeg						(void *					routineAddr)						ONEWORDINLINE(0xA9F1);

#else
#define UnloadSeg(x)
#endif  /* TARGET_CPU_68K */




#if OLDROUTINELOCATIONS
EXTERN_API( void )
ExitToShell						(void)														ONEWORDINLINE(0xA9F4);

#endif  /* OLDROUTINELOCATIONS */




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

#endif /* __SEGLOAD__ */

