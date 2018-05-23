/*
 	File:		Folders.h
 
 	Contains:	Folder Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __FOLDERS__
#define __FOLDERS__

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
	kOnSystemDisk				= -32768L						/* previously was 0x8000 but that is an unsigned value whereas vRefNum is signed*/
};


enum {
	kCreateFolder				= true,
	kDontCreateFolder			= false
};


enum {
	kSystemFolderType			= FOUR_CHAR_CODE('macs'),		/* the system folder */
	kDesktopFolderType			= FOUR_CHAR_CODE('desk'),		/* the desktop folder; objects in this folder show on the desk top. */
	kTrashFolderType			= FOUR_CHAR_CODE('trsh'),		/* the trash folder; objects in this folder show up in the trash */
	kWhereToEmptyTrashFolderType = FOUR_CHAR_CODE('empt'),		/* the "empty trash" folder; Finder starts empty from here down */
	kPrintMonitorDocsFolderType	= FOUR_CHAR_CODE('prnt'),		/* Print Monitor documents */
	kStartupFolderType			= FOUR_CHAR_CODE('strt'),		/* Finder objects (applications, documents, DAs, aliases, to...) to open at startup go here */
	kShutdownFolderType			= FOUR_CHAR_CODE('shdf'),		/* Finder objects (applications, documents, DAs, aliases, to...) to open at shutdown go here */
	kAppleMenuFolderType		= FOUR_CHAR_CODE('amnu'),		/* Finder objects to put into the Apple menu go here */
	kControlPanelFolderType		= FOUR_CHAR_CODE('ctrl'),		/* Control Panels go here (may contain INITs) */
	kExtensionFolderType		= FOUR_CHAR_CODE('extn'),		/* System extensions go here */
	kFontsFolderType			= FOUR_CHAR_CODE('font'),		/* Fonts go here */
	kPreferencesFolderType		= FOUR_CHAR_CODE('pref'),		/* preferences for applications go here */
	kTemporaryFolderType		= FOUR_CHAR_CODE('temp')		/* temporary files go here (deleted periodically, but don't rely on it.) */
};

/*
	Note: 	The FindFolder trap was not implemented until System 7.  If you want to call FindFolder
			while running on System 6 machines, then define USE_FINDFOLDER_GLUE and link with
			Interface.o which contains glue to implement FindFolder on pre-System 7 machines.
*/
#ifdef USE_FINDFOLDER_GLUE
EXTERN_API( OSErr )
FindFolder						(short 					vRefNum,
								 OSType 				folderType,
								 Boolean 				createFolder,
								 short *				foundVRefNum,
								 long *					foundDirID);

#else
EXTERN_API( OSErr )
FindFolder						(short 					vRefNum,
								 OSType 				folderType,
								 Boolean 				createFolder,
								 short *				foundVRefNum,
								 long *					foundDirID)							TWOWORDINLINE(0x7000, 0xA823);

#endif  /*  defined(USE_FINDFOLDER_GLUE)  */

EXTERN_API( OSErr )
ReleaseFolder					(short 					vRefNum,
								 OSType 				folderType)							TWOWORDINLINE(0x700B, 0xA823);


#if !TARGET_OS_MAC
/* Since non-mac targets don't know about VRef's or DirID's, the Ex version returns
   the found folder path.
 */
EXTERN_API_C( OSErr )
FindFolderEx					(short 					vRefNum,
								 OSType 				folderType,
								 Boolean 				createFolder,
								 short *				foundVRefNum,
								 long *					foundDirID,
								 char *					foundFolder);

#endif  /*  !TARGET_OS_MAC */

/******************************************/
/* Extensible Folder Manager declarations */
/******************************************/

/****************************/
/* Folder Manager constants */
/****************************/


enum {
	kExtensionDisabledFolderType = FOUR_CHAR_CODE('extD'),
	kControlPanelDisabledFolderType = FOUR_CHAR_CODE('ctrD'),
	kSystemExtensionDisabledFolderType = FOUR_CHAR_CODE('macD'),
	kStartupItemsDisabledFolderType = FOUR_CHAR_CODE('strD'),
	kShutdownItemsDisabledFolderType = FOUR_CHAR_CODE('shdD'),
	kApplicationsFolderType		= FOUR_CHAR_CODE('apps'),
	kDocumentsFolderType		= FOUR_CHAR_CODE('docs')
};


enum {
																/* new constants */
	kVolumeRootFolderType		= FOUR_CHAR_CODE('root'),		/* root folder of a volume */
	kChewableItemsFolderType	= FOUR_CHAR_CODE('flnt'),		/* items deleted at boot */
	kApplicationSupportFolderType = FOUR_CHAR_CODE('asup'),		/* third-party items and folders */
	kTextEncodingsFolderType	= FOUR_CHAR_CODE('Ätex'),		/* encoding tables */
	kStationeryFolderType		= FOUR_CHAR_CODE('odst'),		/* stationery */
	kOpenDocFolderType			= FOUR_CHAR_CODE('odod'),		/* OpenDoc root */
	kOpenDocShellPlugInsFolderType = FOUR_CHAR_CODE('odsp'),	/* OpenDoc Shell Plug-Ins in OpenDoc folder */
	kEditorsFolderType			= FOUR_CHAR_CODE('oded'),		/* OpenDoc editors in MacOS Folder */
	kOpenDocEditorsFolderType	= FOUR_CHAR_CODE('Äodf'),		/* OpenDoc subfolder of Editors folder */
	kOpenDocLibrariesFolderType	= FOUR_CHAR_CODE('odlb'),		/* OpenDoc libraries folder */
	kGenEditorsFolderType		= FOUR_CHAR_CODE('Äedi'),		/* CKH general editors folder at root level of Sys folder */
	kHelpFolderType				= FOUR_CHAR_CODE('Ählp'),		/* CKH help folder currently at root of system folder */
	kInternetPlugInFolderType	= FOUR_CHAR_CODE('Änet'),		/* CKH internet plug ins for browsers and stuff */
	kModemScriptsFolderType		= FOUR_CHAR_CODE('Ämod'),		/* CKH modem scripts, get 'em OUT of the Extensions folder */
	kPrinterDescriptionFolderType = FOUR_CHAR_CODE('ppdf'),		/* CKH new folder at root of System folder for printer descs. */
	kPrinterDriverFolderType	= FOUR_CHAR_CODE('Äprd'),		/* CKH new folder at root of System folder for printer drivers */
	kScriptingAdditionsFolderType = FOUR_CHAR_CODE('Äscr'),		/* CKH at root of system folder */
	kSharedLibrariesFolderType	= FOUR_CHAR_CODE('Älib'),		/* CKH for general shared libs. */
	kVoicesFolderType			= FOUR_CHAR_CODE('fvoc'),		/* CKH macintalk can live here */
	kControlStripModulesFolderType = FOUR_CHAR_CODE('sdev'),	/* CKH for control strip modules */
	kAssistantsFolderType		= FOUR_CHAR_CODE('astÄ'),		/* SJF for Assistants (MacOS Setup Assistant, etc) */
	kUtilitiesFolderType		= FOUR_CHAR_CODE('utiÄ'),		/* SJF for Utilities folder */
	kAppleExtrasFolderType		= FOUR_CHAR_CODE('aexÄ'),		/* SJF for Apple Extras folder */
	kContextualMenuItemsFolderType = FOUR_CHAR_CODE('cmnu'),	/* SJF for Contextual Menu items */
	kMacOSReadMesFolderType		= FOUR_CHAR_CODE('morÄ'),		/* SJF for MacOS ReadMes folder */
	kALMModulesFolderType		= FOUR_CHAR_CODE('walk'),		/* EAS for Location Manager Module files except type 'thng' (within kExtensionFolderType) */
	kALMPreferencesFolderType	= FOUR_CHAR_CODE('trip'),		/* EAS for Location Manager Preferences (within kPreferencesFolderType; contains kALMLocationsFolderType) */
	kALMLocationsFolderType		= FOUR_CHAR_CODE('fall'),		/* EAS for Location Manager Locations (within kALMPreferencesFolderType) */
	kColorSyncProfilesFolderType = FOUR_CHAR_CODE('prof'),		/* for ColorSyncª Profiles */
	kThemesFolderType			= FOUR_CHAR_CODE('thme'),		/* for Theme data files */
	kFavoritesFolderType		= FOUR_CHAR_CODE('favs')		/* Favorties folder for Navigation Services */
};


/* FolderDescFlags values */

enum {
	kCreateFolderAtBoot			= 0x00000002,
	kFolderCreatedInvisible		= 0x00000004,
	kFolderCreatedNameLocked	= 0x00000008
};

typedef UInt32 							FolderDescFlags;
/* FolderClass values */

enum {
	kRelativeFolder				= FOUR_CHAR_CODE('relf'),
	kSpecialFolder				= FOUR_CHAR_CODE('spcf')
};

typedef OSType 							FolderClass;
/* special folder locations */

enum {
	kBlessedFolder				= FOUR_CHAR_CODE('blsf'),
	kRootFolder					= FOUR_CHAR_CODE('rotf')
};

typedef OSType 							FolderType;
typedef OSType 							FolderLocation;


struct FolderDesc {
	Size 							descSize;
	FolderType 						foldType;
	FolderDescFlags 				flags;
	FolderClass 					foldClass;
	FolderType 						foldLocation;
	OSType 							badgeSignature;
	OSType 							badgeType;
	UInt32 							reserved;
	StrFileName 					name;						/* Str63 on MacOS*/
};
typedef struct FolderDesc				FolderDesc;
typedef FolderDesc *					FolderDescPtr;


typedef UInt32 							RoutingFlags;

struct FolderRouting {
	Size 							descSize;
	OSType 							fileType;
	FolderType 						routeFromFolder;
	FolderType 						routeToFolder;
	RoutingFlags 					flags;
};
typedef struct FolderRouting			FolderRouting;
typedef FolderRouting *					FolderRoutingPtr;
/* routing constants */
/***************************/
/* Folder Manager routines */
/***************************/
/* Folder Manager administration routines */
EXTERN_API( OSErr )
AddFolderDescriptor				(FolderType 			foldType,
								 FolderDescFlags 		flags,
								 FolderClass 			foldClass,
								 FolderLocation 		foldLocation,
								 OSType 				badgeSignature,
								 OSType 				badgeType,
								 ConstStrFileNameParam 	name,
								 Boolean 				replaceFlag)						TWOWORDINLINE(0x7020, 0xA823);

EXTERN_API( OSErr )
GetFolderDescriptor				(FolderType 			foldType,
								 Size 					descSize,
								 FolderDesc *			foldDesc)							TWOWORDINLINE(0x7023, 0xA823);

EXTERN_API( OSErr )
GetFolderTypes					(UInt32 				requestedTypeCount,
								 UInt32 *				totalTypeCount,
								 FolderType *			theTypes)							TWOWORDINLINE(0x7024, 0xA823);

EXTERN_API( OSErr )
RemoveFolderDescriptor			(FolderType 			foldType)							TWOWORDINLINE(0x7021, 0xA823);

/* legacy routines */
EXTERN_API( OSErr )
GetFolderName					(short 					vRefNum,
								 OSType 				foldType,
								 short *				foundVRefNum,
								 StrFileName 			name)								TWOWORDINLINE(0x700E, 0xA823);

/* routing routines */
EXTERN_API( OSErr )
AddFolderRouting				(OSType 				fileType,
								 FolderType 			routeFromFolder,
								 FolderType 			routeToFolder,
								 RoutingFlags 			flags,
								 Boolean 				replaceFlag)						TWOWORDINLINE(0x7026, 0xA823);

EXTERN_API( OSErr )
RemoveFolderRouting				(OSType 				fileType,
								 FolderType 			routeFromFolder)					TWOWORDINLINE(0x7027, 0xA823);

EXTERN_API( OSErr )
FindFolderRouting				(OSType 				fileType,
								 FolderType 			routeFromFolder,
								 FolderType *			routeToFolder,
								 RoutingFlags *			flags)								TWOWORDINLINE(0x7022, 0xA823);

EXTERN_API( OSErr )
GetFolderRoutings				(UInt32 				requestedRoutingCount,
								 UInt32 *				totalRoutingCount,
								 Size 					routingSize,
								 FolderRouting *		theRoutings)						TWOWORDINLINE(0x701E, 0xA823);

EXTERN_API( OSErr )
InvalidateFolderDescriptorCache	(short 					vRefNum,
								 long 					dirID)								TWOWORDINLINE(0x7025, 0xA823);

EXTERN_API( OSErr )
IdentifyFolder					(short 					vRefNum,
								 long 					dirID,
								 FolderType *			foldType)							TWOWORDINLINE(0x701F, 0xA823);





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

#endif /* __FOLDERS__ */

