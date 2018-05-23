/*
 	File:		Icons.h
 
 	Contains:	Icon Utilities Interfaces.
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc. All rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __ICONS__
#define __ICONS__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __FILES__
#include <Files.h>
#endif
#ifndef __CODEFRAGMENTS__
#include <CodeFragments.h>
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
																/* The following are icons for which there are both icon suites and SICNs. */
	kGenericDocumentIconResource = -4000,
	kGenericStationeryIconResource = -3985,
	kGenericEditionFileIconResource = -3989,
	kGenericApplicationIconResource = -3996,
	kGenericDeskAccessoryIconResource = -3991,
	kGenericFolderIconResource	= -3999,
	kPrivateFolderIconResource	= -3994,
	kFloppyIconResource			= -3998,
	kTrashIconResource			= -3993,
	kGenericRAMDiskIconResource	= -3988,
	kGenericCDROMIconResource	= -3987,						/* The following are icons for which there are SICNs only. */
	kDesktopIconResource		= -3992,
	kOpenFolderIconResource		= -3997,
	kGenericHardDiskIconResource = -3995,
	kGenericFileServerIconResource = -3972,
	kGenericSuitcaseIconResource = -3970,
	kGenericMoverObjectIconResource = -3969,					/* The following are icons for which there are icon suites only. */
	kGenericPreferencesIconResource = -3971,
	kGenericQueryDocumentIconResource = -16506,
	kGenericExtensionIconResource = -16415,
	kSystemFolderIconResource	= -3983,
	kAppleMenuFolderIconResource = -3982
};


enum {
																/* Obsolete. Use named constants defined above. */
	genericDocumentIconResource	= kGenericDocumentIconResource,
	genericStationeryIconResource = kGenericStationeryIconResource,
	genericEditionFileIconResource = kGenericEditionFileIconResource,
	genericApplicationIconResource = kGenericApplicationIconResource,
	genericDeskAccessoryIconResource = kGenericDeskAccessoryIconResource,
	genericFolderIconResource	= kGenericFolderIconResource,
	privateFolderIconResource	= kPrivateFolderIconResource,
	floppyIconResource			= kFloppyIconResource,
	trashIconResource			= kTrashIconResource,
	genericRAMDiskIconResource	= kGenericRAMDiskIconResource,
	genericCDROMIconResource	= kGenericCDROMIconResource,
	desktopIconResource			= kDesktopIconResource,
	openFolderIconResource		= kOpenFolderIconResource,
	genericHardDiskIconResource	= kGenericHardDiskIconResource,
	genericFileServerIconResource = kGenericFileServerIconResource,
	genericSuitcaseIconResource	= kGenericSuitcaseIconResource,
	genericMoverObjectIconResource = kGenericMoverObjectIconResource,
	genericPreferencesIconResource = kGenericPreferencesIconResource,
	genericQueryDocumentIconResource = kGenericQueryDocumentIconResource,
	genericExtensionIconResource = kGenericExtensionIconResource,
	systemFolderIconResource	= kSystemFolderIconResource,
	appleMenuFolderIconResource	= kAppleMenuFolderIconResource
};


enum {
	kStartupFolderIconResource	= -3981,
	kOwnedFolderIconResource	= -3980,
	kDropFolderIconResource		= -3979,
	kSharedFolderIconResource	= -3978,
	kMountedFolderIconResource	= -3977,
	kControlPanelFolderIconResource = -3976,
	kPrintMonitorFolderIconResource = -3975,
	kPreferencesFolderIconResource = -3974,
	kExtensionsFolderIconResource = -3973,
	kFontsFolderIconResource	= -3968,
	kFullTrashIconResource		= -3984
};


enum {
																/* Obsolete. Use named constants defined above. */
	startupFolderIconResource	= kStartupFolderIconResource,
	ownedFolderIconResource		= kOwnedFolderIconResource,
	dropFolderIconResource		= kDropFolderIconResource,
	sharedFolderIconResource	= kSharedFolderIconResource,
	mountedFolderIconResource	= kMountedFolderIconResource,
	controlPanelFolderIconResource = kControlPanelFolderIconResource,
	printMonitorFolderIconResource = kPrintMonitorFolderIconResource,
	preferencesFolderIconResource = kPreferencesFolderIconResource,
	extensionsFolderIconResource = kExtensionsFolderIconResource,
	fontsFolderIconResource		= kFontsFolderIconResource,
	fullTrashIconResource		= kFullTrashIconResource
};


enum {
	kLarge1BitMask				= FOUR_CHAR_CODE('ICN#'),
	kLarge4BitData				= FOUR_CHAR_CODE('icl4'),
	kLarge8BitData				= FOUR_CHAR_CODE('icl8'),
	kSmall1BitMask				= FOUR_CHAR_CODE('ics#'),
	kSmall4BitData				= FOUR_CHAR_CODE('ics4'),
	kSmall8BitData				= FOUR_CHAR_CODE('ics8'),
	kMini1BitMask				= FOUR_CHAR_CODE('icm#'),
	kMini4BitData				= FOUR_CHAR_CODE('icm4'),
	kMini8BitData				= FOUR_CHAR_CODE('icm8')
};


enum {
																/* Obsolete. Use names defined above. */
	large1BitMask				= kLarge1BitMask,
	large4BitData				= kLarge4BitData,
	large8BitData				= kLarge8BitData,
	small1BitMask				= kSmall1BitMask,
	small4BitData				= kSmall4BitData,
	small8BitData				= kSmall8BitData,
	mini1BitMask				= kMini1BitMask,
	mini4BitData				= kMini4BitData,
	mini8BitData				= kMini8BitData
};

/*  alignment type values */

enum {
	kAlignNone					= 0x00,
	kAlignVerticalCenter		= 0x01,
	kAlignTop					= 0x02,
	kAlignBottom				= 0x03,
	kAlignHorizontalCenter		= 0x04,
	kAlignAbsoluteCenter		= kAlignVerticalCenter | kAlignHorizontalCenter,
	kAlignCenterTop				= kAlignTop | kAlignHorizontalCenter,
	kAlignCenterBottom			= kAlignBottom | kAlignHorizontalCenter,
	kAlignLeft					= 0x08,
	kAlignCenterLeft			= kAlignVerticalCenter | kAlignLeft,
	kAlignTopLeft				= kAlignTop | kAlignLeft,
	kAlignBottomLeft			= kAlignBottom | kAlignLeft,
	kAlignRight					= 0x0C,
	kAlignCenterRight			= kAlignVerticalCenter | kAlignRight,
	kAlignTopRight				= kAlignTop | kAlignRight,
	kAlignBottomRight			= kAlignBottom | kAlignRight
};


enum {
																/* Obsolete. Use names defined above. */
	atNone						= kAlignNone,
	atVerticalCenter			= kAlignVerticalCenter,
	atTop						= kAlignTop,
	atBottom					= kAlignBottom,
	atHorizontalCenter			= kAlignHorizontalCenter,
	atAbsoluteCenter			= kAlignAbsoluteCenter,
	atCenterTop					= kAlignCenterTop,
	atCenterBottom				= kAlignCenterBottom,
	atLeft						= kAlignLeft,
	atCenterLeft				= kAlignCenterLeft,
	atTopLeft					= kAlignTopLeft,
	atBottomLeft				= kAlignBottomLeft,
	atRight						= kAlignRight,
	atCenterRight				= kAlignCenterRight,
	atTopRight					= kAlignTopRight,
	atBottomRight				= kAlignBottomRight
};

typedef SInt16 							IconAlignmentType;
/*  transform type values  */

enum {
	kTransformNone				= 0x00,
	kTransformDisabled			= 0x01,
	kTransformOffline			= 0x02,
	kTransformOpen				= 0x03,
	kTransformLabel1			= 0x0100,
	kTransformLabel2			= 0x0200,
	kTransformLabel3			= 0x0300,
	kTransformLabel4			= 0x0400,
	kTransformLabel5			= 0x0500,
	kTransformLabel6			= 0x0600,
	kTransformLabel7			= 0x0700,
	kTransformSelected			= 0x4000,
	kTransformSelectedDisabled	= kTransformSelected | kTransformDisabled,
	kTransformSelectedOffline	= kTransformSelected | kTransformOffline,
	kTransformSelectedOpen		= kTransformSelected | kTransformOpen
};


enum {
																/* Obsolete. Use names defined above. */
	ttNone						= kTransformNone,
	ttDisabled					= kTransformDisabled,
	ttOffline					= kTransformOffline,
	ttOpen						= kTransformOpen,
	ttLabel1					= kTransformLabel1,
	ttLabel2					= kTransformLabel2,
	ttLabel3					= kTransformLabel3,
	ttLabel4					= kTransformLabel4,
	ttLabel5					= kTransformLabel5,
	ttLabel6					= kTransformLabel6,
	ttLabel7					= kTransformLabel7,
	ttSelected					= kTransformSelected,
	ttSelectedDisabled			= kTransformSelectedDisabled,
	ttSelectedOffline			= kTransformSelectedOffline,
	ttSelectedOpen				= kTransformSelectedOpen
};

typedef SInt16 							IconTransformType;
/*  Selector mask values  */

enum {
	kSelectorLarge1Bit			= 0x00000001,
	kSelectorLarge4Bit			= 0x00000002,
	kSelectorLarge8Bit			= 0x00000004,
	kSelectorSmall1Bit			= 0x00000100,
	kSelectorSmall4Bit			= 0x00000200,
	kSelectorSmall8Bit			= 0x00000400,
	kSelectorMini1Bit			= 0x00010000,
	kSelectorMini4Bit			= 0x00020000,
	kSelectorMini8Bit			= 0x00040000,
	kSelectorAllLargeData		= 0x000000FF,
	kSelectorAllSmallData		= 0x0000FF00,
	kSelectorAllMiniData		= 0x00FF0000,
	kSelectorAll1BitData		= kSelectorLarge1Bit | kSelectorSmall1Bit | kSelectorMini1Bit,
	kSelectorAll4BitData		= kSelectorLarge4Bit | kSelectorSmall4Bit | kSelectorMini4Bit,
	kSelectorAll8BitData		= kSelectorLarge8Bit | kSelectorSmall8Bit | kSelectorMini8Bit,
	kSelectorAllAvailableData	= (long)0xFFFFFFFF
};


enum {
																/* Obsolete. Use names defined above. */
	svLarge1Bit					= kSelectorLarge1Bit,
	svLarge4Bit					= kSelectorLarge4Bit,
	svLarge8Bit					= kSelectorLarge8Bit,
	svSmall1Bit					= kSelectorSmall1Bit,
	svSmall4Bit					= kSelectorSmall4Bit,
	svSmall8Bit					= kSelectorSmall8Bit,
	svMini1Bit					= kSelectorMini1Bit,
	svMini4Bit					= kSelectorMini4Bit,
	svMini8Bit					= kSelectorMini8Bit,
	svAllLargeData				= kSelectorAllLargeData,
	svAllSmallData				= kSelectorAllSmallData,
	svAllMiniData				= kSelectorAllMiniData,
	svAll1BitData				= kSelectorAll1BitData,
	svAll4BitData				= kSelectorAll4BitData,
	svAll8BitData				= kSelectorAll8BitData,
	svAllAvailableData			= kSelectorAllAvailableData
};

typedef UInt32 							IconSelectorValue;
typedef CALLBACK_API( OSErr , IconActionProcPtr )(ResType theType, Handle *theIcon, void *yourDataPtr);
typedef STACK_UPP_TYPE(IconActionProcPtr) 						IconActionUPP;
enum { uppIconActionProcInfo = 0x00000FE0 }; 					/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes) */
#define NewIconActionProc(userRoutine) 							(IconActionUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppIconActionProcInfo, GetCurrentArchitecture())
#define CallIconActionProc(userRoutine, theType, theIcon, yourDataPtr)  CALL_THREE_PARAMETER_UPP((userRoutine), uppIconActionProcInfo, (theType), (theIcon), (yourDataPtr))
typedef IconActionProcPtr 				IconAction;
typedef CALLBACK_API( Handle , IconGetterProcPtr )(ResType theType, void *yourDataPtr);
typedef STACK_UPP_TYPE(IconGetterProcPtr) 						IconGetterUPP;
enum { uppIconGetterProcInfo = 0x000003F0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes) */
#define NewIconGetterProc(userRoutine) 							(IconGetterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppIconGetterProcInfo, GetCurrentArchitecture())
#define CallIconGetterProc(userRoutine, theType, yourDataPtr) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppIconGetterProcInfo, (theType), (yourDataPtr))
typedef IconGetterProcPtr 				IconGetter;
#if !OLDROUTINELOCATIONS

struct CIcon {
	PixMap 							iconPMap;					/*the icon's pixMap*/
	BitMap 							iconMask;					/*the icon's mask*/
	BitMap 							iconBMap;					/*the icon's bitMap*/
	Handle 							iconData;					/*the icon's data*/
	SInt16 							iconMaskData[1];			/*icon's mask and BitMap data*/
};
typedef struct CIcon					CIcon;
typedef CIcon *							CIconPtr;
typedef CIconPtr *						CIconHandle;
EXTERN_API( CIconHandle )
GetCIcon						(SInt16 				iconID)								ONEWORDINLINE(0xAA1E);

EXTERN_API( void )
PlotCIcon						(const Rect *			theRect,
								 CIconHandle 			theIcon)							ONEWORDINLINE(0xAA1F);

EXTERN_API( void )
DisposeCIcon					(CIconHandle 			theIcon)							ONEWORDINLINE(0xAA25);

#endif  /*  !OLDROUTINELOCATIONS */

EXTERN_API( Handle )
GetIcon							(SInt16 				iconID)								ONEWORDINLINE(0xA9BB);

EXTERN_API( void )
PlotIcon						(const Rect *			theRect,
								 Handle 				theIcon)							ONEWORDINLINE(0xA94B);



/*
	Note:	IconSuiteRef and IconCacheRef should be an abstract types, 
			but too much source code already relies on them being of type Handle.
*/

typedef Handle 							IconSuiteRef;
typedef Handle 							IconCacheRef;
EXTERN_API( OSErr )
PlotIconID						(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 SInt16 				theResID)							THREEWORDINLINE(0x303C, 0x0500, 0xABC9);

EXTERN_API( OSErr )
NewIconSuite					(IconSuiteRef *			theIconSuite)						THREEWORDINLINE(0x303C, 0x0207, 0xABC9);

EXTERN_API( OSErr )
AddIconToSuite					(Handle 				theIconData,
								 IconSuiteRef 			theSuite,
								 ResType 				theType)							THREEWORDINLINE(0x303C, 0x0608, 0xABC9);

EXTERN_API( OSErr )
GetIconFromSuite				(Handle *				theIconData,
								 IconSuiteRef 			theSuite,
								 ResType 				theType)							THREEWORDINLINE(0x303C, 0x0609, 0xABC9);

EXTERN_API( OSErr )
ForEachIconDo					(IconSuiteRef 			theSuite,
								 IconSelectorValue 		selector,
								 IconActionUPP 			action,
								 void *					yourDataPtr)						THREEWORDINLINE(0x303C, 0x080A, 0xABC9);

EXTERN_API( OSErr )
GetIconSuite					(IconSuiteRef *			theIconSuite,
								 SInt16 				theResID,
								 IconSelectorValue 		selector)							THREEWORDINLINE(0x303C, 0x0501, 0xABC9);

EXTERN_API( OSErr )
DisposeIconSuite				(IconSuiteRef 			theIconSuite,
								 Boolean 				disposeData)						THREEWORDINLINE(0x303C, 0x0302, 0xABC9);

EXTERN_API( OSErr )
PlotIconSuite					(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 IconSuiteRef 			theIconSuite)						THREEWORDINLINE(0x303C, 0x0603, 0xABC9);

EXTERN_API( OSErr )
MakeIconCache					(IconCacheRef *			theCache,
								 IconGetterUPP 			makeIcon,
								 void *					yourDataPtr)						THREEWORDINLINE(0x303C, 0x0604, 0xABC9);

EXTERN_API( OSErr )
LoadIconCache					(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 IconCacheRef 			theIconCache)						THREEWORDINLINE(0x303C, 0x0606, 0xABC9);

EXTERN_API( OSErr )
PlotIconMethod					(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 IconGetterUPP 			theMethod,
								 void *					yourDataPtr)						THREEWORDINLINE(0x303C, 0x0805, 0xABC9);

EXTERN_API( OSErr )
GetLabel						(SInt16 				labelNumber,
								 RGBColor *				labelColor,
								 Str255 				labelString)						THREEWORDINLINE(0x303C, 0x050B, 0xABC9);

EXTERN_API( Boolean )
PtInIconID						(Point 					testPt,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 SInt16 				iconID)								THREEWORDINLINE(0x303C, 0x060D, 0xABC9);

EXTERN_API( Boolean )
PtInIconSuite					(Point 					testPt,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 IconSuiteRef 			theIconSuite)						THREEWORDINLINE(0x303C, 0x070E, 0xABC9);

EXTERN_API( Boolean )
PtInIconMethod					(Point 					testPt,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 IconGetterUPP 			theMethod,
								 void *					yourDataPtr)						THREEWORDINLINE(0x303C, 0x090F, 0xABC9);

EXTERN_API( Boolean )
RectInIconID					(const Rect *			testRect,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 SInt16 				iconID)								THREEWORDINLINE(0x303C, 0x0610, 0xABC9);

EXTERN_API( Boolean )
RectInIconSuite					(const Rect *			testRect,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 IconSuiteRef 			theIconSuite)						THREEWORDINLINE(0x303C, 0x0711, 0xABC9);

EXTERN_API( Boolean )
RectInIconMethod				(const Rect *			testRect,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 IconGetterUPP 			theMethod,
								 void *					yourDataPtr)						THREEWORDINLINE(0x303C, 0x0912, 0xABC9);

EXTERN_API( OSErr )
IconIDToRgn						(RgnHandle 				theRgn,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 SInt16 				iconID)								THREEWORDINLINE(0x303C, 0x0613, 0xABC9);

EXTERN_API( OSErr )
IconSuiteToRgn					(RgnHandle 				theRgn,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 IconSuiteRef 			theIconSuite)						THREEWORDINLINE(0x303C, 0x0714, 0xABC9);

EXTERN_API( OSErr )
IconMethodToRgn					(RgnHandle 				theRgn,
								 const Rect *			iconRect,
								 IconAlignmentType 		align,
								 IconGetterUPP 			theMethod,
								 void *					yourDataPtr)						THREEWORDINLINE(0x303C, 0x0915, 0xABC9);

EXTERN_API( OSErr )
SetSuiteLabel					(IconSuiteRef 			theSuite,
								 SInt16 				theLabel)							THREEWORDINLINE(0x303C, 0x0316, 0xABC9);

EXTERN_API( SInt16 )
GetSuiteLabel					(IconSuiteRef 			theSuite)							THREEWORDINLINE(0x303C, 0x0217, 0xABC9);

EXTERN_API( OSErr )
GetIconCacheData				(IconCacheRef 			theCache,
								 void **				theData)							THREEWORDINLINE(0x303C, 0x0419, 0xABC9);

EXTERN_API( OSErr )
SetIconCacheData				(IconCacheRef 			theCache,
								 void *					theData)							THREEWORDINLINE(0x303C, 0x041A, 0xABC9);

EXTERN_API( OSErr )
GetIconCacheProc				(IconCacheRef 			theCache,
								 IconGetterUPP *		theProc)							THREEWORDINLINE(0x303C, 0x041B, 0xABC9);

EXTERN_API( OSErr )
SetIconCacheProc				(IconCacheRef 			theCache,
								 IconGetterUPP 			theProc)							THREEWORDINLINE(0x303C, 0x041C, 0xABC9);

EXTERN_API( OSErr )
PlotIconHandle					(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 Handle 				theIcon)							THREEWORDINLINE(0x303C, 0x061D, 0xABC9);

EXTERN_API( OSErr )
PlotSICNHandle					(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 Handle 				theSICN)							THREEWORDINLINE(0x303C, 0x061E, 0xABC9);

EXTERN_API( OSErr )
PlotCIconHandle					(const Rect *			theRect,
								 IconAlignmentType 		align,
								 IconTransformType 		transform,
								 CIconHandle 			theCIcon)							THREEWORDINLINE(0x303C, 0x061F, 0xABC9);













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

#endif /* __ICONS__ */

