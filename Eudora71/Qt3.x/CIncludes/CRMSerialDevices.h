/*
 	File:		CRMSerialDevices.h
 
 	Contains:	Communications Resource Manager Serial Device interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1993, 1995, 1997-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __CRMSERIALDEVICES__
#define __CRMSERIALDEVICES__

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
																/* 	for the crmDeviceType field of the CRMRec data structure	*/
	crmSerialDevice				= 1,							/*	version of the CRMSerialRecord below	*/
	curCRMSerRecVers			= 1
};

/* Maintains compatibility w/ apps & tools that expect an old style icon	*/

struct CRMIconRecord {
	long 							oldIcon[32];				/* ICN#	*/
	long 							oldMask[32];
	Handle 							theSuite;					/* Handle to an IconSuite	*/
	long 							reserved;
};
typedef struct CRMIconRecord			CRMIconRecord;

typedef CRMIconRecord *					CRMIconPtr;
typedef CRMIconPtr *					CRMIconHandle;

struct CRMSerialRecord {
	short 							version;
	StringHandle 					inputDriverName;
	StringHandle 					outputDriverName;
	StringHandle 					name;
	CRMIconHandle 					deviceIcon;
	long 							ratedSpeed;
	long 							maxSpeed;
	long 							reserved;
};
typedef struct CRMSerialRecord			CRMSerialRecord;

typedef CRMSerialRecord *				CRMSerialPtr;

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

#endif /* __CRMSERIALDEVICES__ */

