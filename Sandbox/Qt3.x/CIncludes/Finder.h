/*
 	File:		Finder.h
 
 	Contains:	Finder flags and container types.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __FINDER__
#define __FINDER__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
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


enum {
	kCustomIconResource			= -16455						/* Custom icon family resource ID */
};


enum {
	kContainerFolderAliasType	= FOUR_CHAR_CODE('fdrp'),		/* type for folder aliases */
	kContainerTrashAliasType	= FOUR_CHAR_CODE('trsh'),		/* type for trash folder aliases */
	kContainerHardDiskAliasType	= FOUR_CHAR_CODE('hdsk'),		/* type for hard disk aliases */
	kContainerFloppyAliasType	= FOUR_CHAR_CODE('flpy'),		/* type for floppy aliases */
	kContainerServerAliasType	= FOUR_CHAR_CODE('srvr'),		/* type for server aliases */
	kApplicationAliasType		= FOUR_CHAR_CODE('adrp'),		/* type for application aliases */
	kContainerAliasType			= FOUR_CHAR_CODE('drop'),		/* type for all other containers */
																/* types for Special folder aliases */
	kSystemFolderAliasType		= FOUR_CHAR_CODE('fasy'),
	kAppleMenuFolderAliasType	= FOUR_CHAR_CODE('faam'),
	kStartupFolderAliasType		= FOUR_CHAR_CODE('fast'),
	kPrintMonitorDocsFolderAliasType = FOUR_CHAR_CODE('fapn'),
	kPreferencesFolderAliasType	= FOUR_CHAR_CODE('fapf'),
	kControlPanelFolderAliasType = FOUR_CHAR_CODE('fact'),
	kExtensionFolderAliasType	= FOUR_CHAR_CODE('faex'),		/* types for AppleShare folder aliases */
	kExportedFolderAliasType	= FOUR_CHAR_CODE('faet'),
	kDropFolderAliasType		= FOUR_CHAR_CODE('fadr'),
	kSharedFolderAliasType		= FOUR_CHAR_CODE('fash'),
	kMountedFolderAliasType		= FOUR_CHAR_CODE('famn')
};


enum {
																/* Finder Flags */
	kIsOnDesk					= 0x0001,
	kColor						= 0x000E,
	kIsShared					= 0x0040,						/* bit 0x0080 is hasNoINITS */
	kHasBeenInited				= 0x0100,						/* bit 0x0200 was the letter bit for AOCE, but is now reserved for future use */
	kHasCustomIcon				= 0x0400,
	kIsStationery				= 0x0800,
	kNameLocked					= 0x1000,
	kHasBundle					= 0x2000,
	kIsInvisible				= 0x4000,
	kIsAlias					= 0x8000
};

#if OLDROUTINENAMES

enum {
	kIsStationary				= kIsStationery
};

#endif  /* OLDROUTINENAMES */

/*	
	The following declerations used to be in Files.Å, 
	but are Finder specific and were moved here.
*/

enum {
																/* Finder Constants */
	fOnDesk						= 1,
	fHasBundle					= 8192,
	fTrash						= -3,
	fDesktop					= -2,
	fDisk						= 0
};


struct FInfo {
	OSType 							fdType;						/*the type of the file*/
	OSType 							fdCreator;					/*file's creator*/
	unsigned short 					fdFlags;					/*flags ex. hasbundle,invisible,locked, etc.*/
	Point 							fdLocation;					/*file's location in folder*/
	short 							fdFldr;						/*folder containing file*/
};
typedef struct FInfo					FInfo;

struct FXInfo {
	short 							fdIconID;					/*Icon ID*/
	short 							fdUnused[3];				/*unused but reserved 6 bytes*/
	SInt8 							fdScript;					/*Script flag and number*/
	SInt8 							fdXFlags;					/*More flag bits*/
	short 							fdComment;					/*Comment ID*/
	long 							fdPutAway;					/*Home Dir ID*/
};
typedef struct FXInfo					FXInfo;

struct DInfo {
	Rect 							frRect;						/*folder rect*/
	unsigned short 					frFlags;					/*Flags*/
	Point 							frLocation;					/*folder location*/
	short 							frView;						/*folder view*/
};
typedef struct DInfo					DInfo;

struct DXInfo {
	Point 							frScroll;					/*scroll position*/
	long 							frOpenChain;				/*DirID chain of open folders*/
	SInt8 							frScript;					/*Script flag and number*/
	SInt8 							frXFlags;					/*More flag bits*/
	short 							frComment;					/*comment*/
	long 							frPutAway;					/*DirID*/
};
typedef struct DXInfo					DXInfo;

/* Values of the 'message' parameter to a Control Panel 'cdev' */

enum {
	initDev						= 0,							/*Time for cdev to initialize itself*/
	hitDev						= 1,							/*Hit on one of my items*/
	closeDev					= 2,							/*Close yourself*/
	nulDev						= 3,							/*Null event*/
	updateDev					= 4,							/*Update event*/
	activDev					= 5,							/*Activate event*/
	deactivDev					= 6,							/*Deactivate event*/
	keyEvtDev					= 7,							/*Key down/auto key*/
	macDev						= 8,							/*Decide whether or not to show up*/
	undoDev						= 9,
	cutDev						= 10,
	copyDev						= 11,
	pasteDev					= 12,
	clearDev					= 13,
	cursorDev					= 14
};


/* Special values a Control Panel 'cdev' can return */

enum {
	cdevGenErr					= -1,							/*General error; gray cdev w/o alert*/
	cdevMemErr					= 0,							/*Memory shortfall; alert user please*/
	cdevResErr					= 1,							/*Couldn't get a needed resource; alert*/
	cdevUnset					= 3								/* cdevValue is initialized to this*/
};

/* Control Panel Default Proc */
typedef CALLBACK_API( long , ControlPanelDefProcPtr )(short message, short item, short numItems, short cPanelID, EventRecord *theEvent, long cdevValue, DialogPtr cpDialog);
typedef STACK_UPP_TYPE(ControlPanelDefProcPtr) 					ControlPanelDefUPP;
enum { uppControlPanelDefProcInfo = 0x000FEAB0 }; 				/* pascal 4_bytes Func(2_bytes, 2_bytes, 2_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewControlPanelDefProc(userRoutine) 					(ControlPanelDefUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppControlPanelDefProcInfo, GetCurrentArchitecture())
#define CallControlPanelDefProc(userRoutine, message, item, numItems, cPanelID, theEvent, cdevValue, cpDialog)  CALL_SEVEN_PARAMETER_UPP((userRoutine), uppControlPanelDefProcInfo, (message), (item), (numItems), (cPanelID), (theEvent), (cdevValue), (cpDialog))





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

#endif /* __FINDER__ */

