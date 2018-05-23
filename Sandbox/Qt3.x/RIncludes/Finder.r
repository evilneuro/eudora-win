/*
 	File:		Finder.r
 
 	Contains:	Finder flags and container types.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __FINDER_R__
#define __FINDER_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define kCustomIconResource 			(-16455)			/*  Custom icon family resource ID  */
#define kContainerFolderAliasType 		'fdrp'				/*  type for folder aliases  */
#define kContainerTrashAliasType 		'trsh'				/*  type for trash folder aliases  */
#define kContainerHardDiskAliasType 	'hdsk'				/*  type for hard disk aliases  */
#define kContainerFloppyAliasType 		'flpy'				/*  type for floppy aliases  */
#define kContainerServerAliasType 		'srvr'				/*  type for server aliases  */
#define kApplicationAliasType 			'adrp'				/*  type for application aliases  */
#define kContainerAliasType 			'drop'				/*  type for all other containers  */
															/*  types for Special folder aliases  */
#define kSystemFolderAliasType 			'fasy'
#define kAppleMenuFolderAliasType 		'faam'
#define kStartupFolderAliasType 		'fast'
#define kPrintMonitorDocsFolderAliasType  'fapn'
#define kPreferencesFolderAliasType 	'fapf'
#define kControlPanelFolderAliasType 	'fact'
#define kExtensionFolderAliasType 		'faex'				/*  types for AppleShare folder aliases  */
#define kExportedFolderAliasType 		'faet'
#define kDropFolderAliasType 			'fadr'
#define kSharedFolderAliasType 			'fash'
#define kMountedFolderAliasType 		'famn'


/*----------------------------BNDL ¥ Bundle---------------------------------------------*/
type 'BNDL' {
		literal longint;										/* Signature			*/
		integer;												/* Version ID			*/
		integer = $$CountOf(TypeArray) - 1;
		array TypeArray {
				literal longint;								/* Type 				*/
				integer = $$CountOf(IDArray) - 1;
				wide array IDArray {
						integer;								/* Local ID 			*/
						integer;								/* Actual ID			*/
				};
		};
};

/*----------------------------open ¥ FileTypes an app can open--------------------------*/
type 'open' {
		literal longint;										/* app signature		*/
		integer = 0;
		integer = $$CountOf(typeArray);		 					/* Array size			*/
		wide array typeArray {									/* FileTypes that app can open */
				literal longint;								/* FileType				*/
		};
};

/*----------------------------FREF ¥ File Reference-------------------------------------*/
type 'FREF' {
		literal longint;										/* File Type			*/
		integer;												/* Icon ID				*/
		pstring;												/* Filename 			*/
};

/*----------------------------kind ¥ Custom kind strings --------------------------------*/
type 'kind' {
		literal longint;										/* app signature		 */
		integer;												/* region code of kind string localizations */
		integer = 0;
		integer = $$CountOf(kindArray);		 					/* Array size			*/
		wide array kindArray {
				literal longint;								/* File Type			*/
				pstring;										/* custom kind strings	*/
				align	word;
		};
};
/*----------------------------mach ¥ cdev filtering-------------------------------------*/
type 'mach' {
		unsigned hex integer;												/* Softmask	*/
		unsigned hex integer;												/* Hardmask	*/
};
/*----------------------------nrct ¥ Rectangle List-------------------------------------*/
type 'nrct' {
		integer = $$CountOf(RectArray);							/* Number of rectangles	*/
		array RectArray {
			rect;
		};
};


#endif /* __FINDER_R__ */

