/*
 	File:		Folders.r
 
 	Contains:	Folder Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __FOLDERS_R__
#define __FOLDERS_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define kOnSystemDisk 					(-32768)			/*  previously was 0x8000 but that is an unsigned value whereas vRefNum is signed */
#define kCreateFolder 					1
#define kDontCreateFolder 				0

#define kSystemFolderType 				'macs'				/*  the system folder  */
#define kDesktopFolderType 				'desk'				/*  the desktop folder; objects in this folder show on the desk top.  */
#define kTrashFolderType 				'trsh'				/*  the trash folder; objects in this folder show up in the trash  */
#define kWhereToEmptyTrashFolderType 	'empt'				/*  the "empty trash" folder; Finder starts empty from here down  */
#define kPrintMonitorDocsFolderType 	'prnt'				/*  Print Monitor documents  */
#define kStartupFolderType 				'strt'				/*  Finder objects (applications, documents, DAs, aliases, to...) to open at startup go here  */
#define kShutdownFolderType 			'shdf'				/*  Finder objects (applications, documents, DAs, aliases, to...) to open at shutdown go here  */
#define kAppleMenuFolderType 			'amnu'				/*  Finder objects to put into the Apple menu go here  */
#define kControlPanelFolderType 		'ctrl'				/*  Control Panels go here (may contain INITs)  */
#define kExtensionFolderType 			'extn'				/*  System extensions go here  */
#define kFontsFolderType 				'font'				/*  Fonts go here  */
#define kPreferencesFolderType 			'pref'				/*  preferences for applications go here  */
#define kTemporaryFolderType 			'temp'				/*  temporary files go here (deleted periodically, but don't rely on it.)  */

#define kExtensionDisabledFolderType 	'extD'
#define kControlPanelDisabledFolderType  'ctrD'
#define kSystemExtensionDisabledFolderType  'macD'
#define kStartupItemsDisabledFolderType  'strD'
#define kShutdownItemsDisabledFolderType  'shdD'
#define kApplicationsFolderType 		'apps'
#define kDocumentsFolderType 			'docs'

															/*  new constants  */
#define kVolumeRootFolderType 			'root'				/*  root folder of a volume  */
#define kChewableItemsFolderType 		'flnt'				/*  items deleted at boot  */
#define kApplicationSupportFolderType 	'asup'				/*  third-party items and folders  */
#define kTextEncodingsFolderType 		'Ätex'				/*  encoding tables  */
#define kStationeryFolderType 			'odst'				/*  stationery  */
#define kOpenDocFolderType 				'odod'				/*  OpenDoc root  */
#define kOpenDocShellPlugInsFolderType 	'odsp'				/*  OpenDoc Shell Plug-Ins in OpenDoc folder  */
#define kEditorsFolderType 				'oded'				/*  OpenDoc editors in MacOS Folder  */
#define kOpenDocEditorsFolderType 		'Äodf'				/*  OpenDoc subfolder of Editors folder  */
#define kOpenDocLibrariesFolderType 	'odlb'				/*  OpenDoc libraries folder  */
#define kGenEditorsFolderType 			'Äedi'				/*  CKH general editors folder at root level of Sys folder  */
#define kHelpFolderType 				'Ählp'				/*  CKH help folder currently at root of system folder  */
#define kInternetPlugInFolderType 		'Änet'				/*  CKH internet plug ins for browsers and stuff  */
#define kModemScriptsFolderType 		'Ämod'				/*  CKH modem scripts, get 'em OUT of the Extensions folder  */
#define kPrinterDescriptionFolderType 	'ppdf'				/*  CKH new folder at root of System folder for printer descs.  */
#define kPrinterDriverFolderType 		'Äprd'				/*  CKH new folder at root of System folder for printer drivers  */
#define kScriptingAdditionsFolderType 	'Äscr'				/*  CKH at root of system folder  */
#define kSharedLibrariesFolderType 		'Älib'				/*  CKH for general shared libs.  */
#define kVoicesFolderType 				'fvoc'				/*  CKH macintalk can live here  */
#define kControlStripModulesFolderType 	'sdev'				/*  CKH for control strip modules  */
#define kAssistantsFolderType 			'astÄ'				/*  SJF for Assistants (MacOS Setup Assistant, etc)  */
#define kUtilitiesFolderType 			'utiÄ'				/*  SJF for Utilities folder  */
#define kAppleExtrasFolderType 			'aexÄ'				/*  SJF for Apple Extras folder  */
#define kContextualMenuItemsFolderType 	'cmnu'				/*  SJF for Contextual Menu items  */
#define kMacOSReadMesFolderType 		'morÄ'				/*  SJF for MacOS ReadMes folder  */
#define kALMModulesFolderType 			'walk'				/*  EAS for Location Manager Module files except type 'thng' (within kExtensionFolderType)  */
#define kALMPreferencesFolderType 		'trip'				/*  EAS for Location Manager Preferences (within kPreferencesFolderType; contains kALMLocationsFolderType)  */
#define kALMLocationsFolderType 		'fall'				/*  EAS for Location Manager Locations (within kALMPreferencesFolderType)  */
#define kColorSyncProfilesFolderType 	'prof'				/*  for ColorSyncª Profiles  */
#define kThemesFolderType 				'thme'				/*  for Theme data files  */
#define kFavoritesFolderType 			'favs'				/*  Favorties folder for Navigation Services  */

#define kCreateFolderAtBoot 			0x00000002
#define kFolderCreatedInvisible 		0x00000004
#define kFolderCreatedNameLocked 		0x00000008

#define kRelativeFolder 				'relf'
#define kSpecialFolder 					'spcf'

#define kBlessedFolder 					'blsf'
#define kRootFolder 					'rotf'


/* fld# ¥ list of folder names for Folder Mgr */

	type 'fld#' {
		array {
			literal		longint;				// folder type
			integer		inSystemFolder = 0;		// version
			fill byte;							// high byte of data length
			pstring;							// folder name
			align word;
		};
	};

#endif /* __FOLDERS_R__ */

