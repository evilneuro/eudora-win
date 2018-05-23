/*
 	File:		Packages.h
 
 	Contains:	Package Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1993, 1995, 1997-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __PACKAGES__
#define __PACKAGES__

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


enum {
	listMgr						= 0,							/* list manager */
	dskInit						= 2,							/* Disk Initializaton */
	stdFile						= 3,							/* Standard File */
	flPoint						= 4,							/* Floating-Point Arithmetic */
	trFunc						= 5,							/* Transcendental Functions */
	intUtil						= 6,							/* International Utilities */
	bdConv						= 7,							/* Binary/Decimal Conversion */
	editionMgr					= 11							/* Edition Manager */
};

EXTERN_API( void )
InitPack						(short 					packID)								ONEWORDINLINE(0xA9E5);

EXTERN_API( void )
InitAllPacks					(void)														ONEWORDINLINE(0xA9E6);



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

#endif /* __PACKAGES__ */

