/*
 	File:		DiskInit.h
 
 	Contains:	Disk Initialization Package ('PACK' 2) Interfaces.
 
 	Version:	Technology:	System 8.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1995, 1997-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __DISKINIT__
#define __DISKINIT__

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


struct HFSDefaults {
	char 							sigWord[2];					/* signature word */
	long 							abSize;						/* allocation block size in bytes */
	long 							clpSize;					/* clump size in bytes */
	long 							nxFreeFN;					/* next free file number */
	long 							btClpSize;					/* B-Tree clump size in bytes */
	short 							rsrv1;						/* reserved */
	short 							rsrv2;						/* reserved */
	short 							rsrv3;						/* reserved */
};
typedef struct HFSDefaults				HFSDefaults;

enum {
	kHFSPlusDefaultsVersion		= 1
};


struct HFSPlusDefaults {
	UInt16 							version;					/* version of this structure */
	UInt16 							flags;						/* currently undefined; pass zero */
	UInt32 							blockSize;					/* allocation block size in bytes */
	UInt32 							rsrcClumpSize;				/* clump size for resource forks */
	UInt32 							dataClumpSize;				/* clump size for data forks */
	UInt32 							nextFreeFileID;				/* next free file number */
	UInt32 							catalogClumpSize;			/* clump size for catalog B-tree */
	UInt32 							catalogNodeSize;			/* node size for catalog B-tree */
	UInt32 							extentsClumpSize;			/* clump size for extents B-tree */
	UInt32 							extentsNodeSize;			/* node size for extents B-tree */
	UInt32 							attributesClumpSize;		/* clump size for attributes B-tree */
	UInt32 							attributesNodeSize;			/* node size for attributes B-tree */
	UInt32 							allocationClumpSize;		/* clump size for allocation bitmap file */
};
typedef struct HFSPlusDefaults			HFSPlusDefaults;

EXTERN_API( void )
DILoad							(void)														THREEWORDINLINE(0x7002, 0x3F00, 0xA9E9);

EXTERN_API( void )
DIUnload						(void)														THREEWORDINLINE(0x7004, 0x3F00, 0xA9E9);

EXTERN_API( short )
DIBadMount						(Point 					where,
								 long 					evtMessage)							THREEWORDINLINE(0x7000, 0x3F00, 0xA9E9);

EXTERN_API( OSErr )
DIFormat						(short 					drvNum)								THREEWORDINLINE(0x7006, 0x3F00, 0xA9E9);

EXTERN_API( OSErr )
DIVerify						(short 					drvNum)								THREEWORDINLINE(0x7008, 0x3F00, 0xA9E9);

EXTERN_API( OSErr )
DIZero							(short 					drvNum,
								 ConstStr255Param 		volName)							THREEWORDINLINE(0x700A, 0x3F00, 0xA9E9);

/*
	DIXFormat, DIXZero, and DIReformat are only available when FSM (File System Manager) is installed.
	FSM is part of Macintosh PC Exchange and System 7.5.
*/
EXTERN_API( OSErr )
DIXFormat						(short 					drvNum,
								 Boolean 				fmtFlag,
								 unsigned long 			fmtArg,
								 unsigned long *		actSize)							THREEWORDINLINE(0x700C, 0x3F00, 0xA9E9);

EXTERN_API( OSErr )
DIXZero							(short 					drvNum,
								 ConstStr255Param 		volName,
								 short 					fsid,
								 short 					mediaStatus,
								 short 					volTypeSelector,
								 unsigned long 			volSize,
								 void *					extendedInfoPtr)					THREEWORDINLINE(0x700E, 0x3F00, 0xA9E9);

EXTERN_API( OSErr )
DIReformat						(short 					drvNum,
								 short 					fsid,
								 ConstStr255Param 		volName,
								 ConstStr255Param 		msgText)							THREEWORDINLINE(0x7010, 0x3F00, 0xA9E9);


#if CGLUESUPPORTED
EXTERN_API_C( OSErr )
dibadmount						(Point *				where,
								 long 					evtMessage);

EXTERN_API_C( OSErr )
dizero							(short 					drvnum,
								 const char *			volName);

#endif  /* CGLUESUPPORTED */



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

#endif /* __DISKINIT__ */

