/*
 	File:		Appearance.h
 
 	Contains:	Appearance Manager Interfaces.
 
 	Version:	Technology:	Appearance 1.0.2
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __APPEARANCE__
#define __APPEARANCE__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif
#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/* Appearance Manager constants, etc.												*/
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/


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
																/* Appearance Trap Number */
	_AppearanceDispatch			= 0xAA74
};

/* Gestalt selector and values for the Appearance Manager */

enum {
	gestaltAppearanceAttr		= FOUR_CHAR_CODE('appr'),
	gestaltAppearanceExists		= 0,
	gestaltAppearanceCompatMode	= 1
};

/* Gestalt selector for determining Appearance Manager version 	*/
/* If this selector does not exist, but gestaltAppearanceAttr	*/
/* does, it indicates that the 1.0 version is installed. This	*/
/* gestalt returns a BCD number representing the version of the	*/
/* Appearance Manager that is currently running, e.g. 0x0101 for*/
/* version 1.0.1.												*/

enum {
	gestaltAppearanceVersion	= FOUR_CHAR_CODE('apvr')
};

/* Appearance Manager Apple Events (1.0.1 and later) */

enum {
	kAppearanceEventClass		= FOUR_CHAR_CODE('appr'),		/* Event Class */
	kAEThemeSwitch				= FOUR_CHAR_CODE('thme')		/* Event ID's: Theme Switched */
};

/* Appearance Manager Error Codes */

enum {
	appearanceBadBrushIndexErr	= -30560,						/* pattern index invalid */
	appearanceProcessRegisteredErr = -30561,
	appearanceProcessNotRegisteredErr = -30562,
	appearanceBadTextColorIndexErr = -30563,
	appearanceThemeHasNoAccents	= -30564,
	appearanceBadCursorIndexErr	= -30565
};


enum {
	kThemeActiveDialogBackgroundBrush = 1,						/* Dialogs */
	kThemeInactiveDialogBackgroundBrush = 2,					/* Dialogs */
	kThemeActiveAlertBackgroundBrush = 3,
	kThemeInactiveAlertBackgroundBrush = 4,
	kThemeActiveModelessDialogBackgroundBrush = 5,
	kThemeInactiveModelessDialogBackgroundBrush = 6,
	kThemeActiveUtilityWindowBackgroundBrush = 7,				/* Miscellaneous */
	kThemeInactiveUtilityWindowBackgroundBrush = 8,				/* Miscellaneous */
	kThemeListViewSortColumnBackgroundBrush = 9,				/* Finder */
	kThemeListViewBackgroundBrush = 10,
	kThemeIconLabelBackgroundBrush = 11,
	kThemeListViewSeparatorBrush = 12,
	kThemeChasingArrowsBrush	= 13,
	kThemeDragHiliteBrush		= 14,
	kThemeDocumentWindowBackgroundBrush = 15,
	kThemeFinderWindowBackgroundBrush = 16
};

typedef SInt16 							ThemeBrush;

enum {
	kThemeActiveDialogTextColor	= 1,							/* Dialogs */
	kThemeInactiveDialogTextColor = 2,
	kThemeActiveAlertTextColor	= 3,
	kThemeInactiveAlertTextColor = 4,
	kThemeActiveModelessDialogTextColor = 5,
	kThemeInactiveModelessDialogTextColor = 6,
	kThemeActiveWindowHeaderTextColor = 7,						/* Primitives */
	kThemeInactiveWindowHeaderTextColor = 8,
	kThemeActivePlacardTextColor = 9,							/* Primitives */
	kThemeInactivePlacardTextColor = 10,
	kThemePressedPlacardTextColor = 11,
	kThemeActivePushButtonTextColor = 12,						/* Primitives */
	kThemeInactivePushButtonTextColor = 13,
	kThemePressedPushButtonTextColor = 14,
	kThemeActiveBevelButtonTextColor = 15,						/* Primitives */
	kThemeInactiveBevelButtonTextColor = 16,
	kThemePressedBevelButtonTextColor = 17,
	kThemeActivePopupButtonTextColor = 18,						/* Primitives */
	kThemeInactivePopupButtonTextColor = 19,
	kThemePressedPopupButtonTextColor = 20,
	kThemeIconLabelTextColor	= 21,							/* Finder */
	kThemeListViewTextColor		= 22
};

/* Text Colors available in Appearance 1.0.1 or later */

enum {
	kThemeActiveDocumentWindowTitleTextColor = 23,
	kThemeInactiveDocumentWindowTitleTextColor = 24,
	kThemeActiveMovableModalWindowTitleTextColor = 25,
	kThemeInactiveMovableModalWindowTitleTextColor = 26,
	kThemeActiveUtilityWindowTitleTextColor = 27,
	kThemeInactiveUtilityWindowTitleTextColor = 28,
	kThemeActivePopupWindowTitleColor = 29,
	kThemeInactivePopupWindowTitleColor = 30,
	kThemeActiveRootMenuTextColor = 31,
	kThemeSelectedRootMenuTextColor = 32,
	kThemeDisabledRootMenuTextColor = 33,
	kThemeActiveMenuItemTextColor = 34,
	kThemeSelectedMenuItemTextColor = 35,
	kThemeDisabledMenuItemTextColor = 36,
	kThemeActivePopupLabelTextColor = 37,
	kThemeInactivePopupLabelTextColor = 38
};

typedef SInt16 							ThemeTextColor;
/* States to draw primitives: disabled, active, and pressed (hilited) */

enum {
	kThemeStateDisabled			= 0,
	kThemeStateActive			= 1,
	kThemeStatePressed			= 2
};

typedef UInt32 							ThemeDrawState;
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/* Theme menu bar drawing states													*/
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/

enum {
	kThemeMenuBarNormal			= 0,
	kThemeMenuBarSelected		= 1
};

typedef SInt16 							ThemeMenuBarState;
/* attributes */

enum {
	kThemeMenuSquareMenuBar		= (1 << 0)
};

/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/* Theme menu drawing states													    */
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/

enum {
	kThemeMenuActive			= 0,
	kThemeMenuSelected			= 1,
	kThemeMenuDisabled			= 3
};

typedef SInt16 							ThemeMenuState;

enum {
	kThemeMenuTypePullDown		= 0,
	kThemeMenuTypePopUp			= 1,
	kThemeMenuTypeHierarchical	= 2
};

typedef SInt16 							ThemeMenuType;

enum {
	kThemeMenuItemPlain			= 0,
	kThemeMenuItemHierarchical	= 1,
	kThemeMenuItemScrollUpArrow	= 2,
	kThemeMenuItemScrollDownArrow = 3
};

typedef SInt16 							ThemeMenuItemType;
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/* Menu Drawing callbacks														    */
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
typedef CALLBACK_API( void , MenuTitleDrawingProcPtr )(const Rect *inBounds, SInt16 inDepth, Boolean inIsColorDevice, SInt32 inUserData);
typedef CALLBACK_API( void , MenuItemDrawingProcPtr )(const Rect *inBounds, SInt16 inDepth, Boolean inIsColorDevice, SInt32 inUserData);
typedef STACK_UPP_TYPE(MenuTitleDrawingProcPtr) 				MenuTitleDrawingUPP;
typedef STACK_UPP_TYPE(MenuItemDrawingProcPtr) 					MenuItemDrawingUPP;
enum { uppMenuTitleDrawingProcInfo = 0x000036C0 }; 				/* pascal no_return_value Func(4_bytes, 2_bytes, 1_byte, 4_bytes) */
enum { uppMenuItemDrawingProcInfo = 0x000036C0 }; 				/* pascal no_return_value Func(4_bytes, 2_bytes, 1_byte, 4_bytes) */
#define NewMenuTitleDrawingProc(userRoutine) 					(MenuTitleDrawingUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuTitleDrawingProcInfo, GetCurrentArchitecture())
#define NewMenuItemDrawingProc(userRoutine) 					(MenuItemDrawingUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuItemDrawingProcInfo, GetCurrentArchitecture())
#define CallMenuTitleDrawingProc(userRoutine, inBounds, inDepth, inIsColorDevice, inUserData)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppMenuTitleDrawingProcInfo, (inBounds), (inDepth), (inIsColorDevice), (inUserData))
#define CallMenuItemDrawingProc(userRoutine, inBounds, inDepth, inIsColorDevice, inUserData)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppMenuItemDrawingProcInfo, (inBounds), (inDepth), (inIsColorDevice), (inUserData))


/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/*	Appearance Manager APIs															*/
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/* Registering Appearance-Savvy Applications */
EXTERN_API( OSStatus )
RegisterAppearanceClient		(void)														THREEWORDINLINE(0x303C, 0x0015, 0xAA74);

EXTERN_API( OSStatus )
UnregisterAppearanceClient		(void)														THREEWORDINLINE(0x303C, 0x0016, 0xAA74);

/*****************************************************************************
	NOTES ON THEME BRUSHES
	Theme brushes can be either colors or patterns, depending on the theme.
	Because of this, you should be prepared to handle the case where a brush
	is a pattern and save and restore the pnPixPat and bkPixPat fields of
	your GrafPorts when saving the fore and back colors. Also, since patterns
	in bkPixPat override the background color of the window, you should use
	BackPat to set your background pattern to a normal white pattern. This
	will ensure that you can use RGBBackColor to set your back color to white,
	call EraseRect and get the expected results.
*****************************************************************************/

EXTERN_API( OSStatus )
SetThemePen						(ThemeBrush 			inBrush,
								 SInt16 				inDepth,
								 Boolean 				inIsColorDevice)					THREEWORDINLINE(0x303C, 0x0001, 0xAA74);

EXTERN_API( OSStatus )
SetThemeBackground				(ThemeBrush 			inBrush,
								 SInt16 				inDepth,
								 Boolean 				inIsColorDevice)					THREEWORDINLINE(0x303C, 0x0002, 0xAA74);

EXTERN_API( OSStatus )
SetThemeTextColor				(ThemeTextColor 		inColor,
								 SInt16 				inDepth,
								 Boolean 				inIsColorDevice)					THREEWORDINLINE(0x303C, 0x0003, 0xAA74);

EXTERN_API( OSStatus )
SetThemeWindowBackground		(WindowPtr 				inWindow,
								 ThemeBrush 			inBrush,
								 Boolean 				inUpdate)							THREEWORDINLINE(0x303C, 0x0004, 0xAA74);

/* Window Placards, Headers and Frames */
EXTERN_API( OSStatus )
DrawThemeWindowHeader			(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x0005, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeWindowListViewHeader	(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x0006, 0xAA74);

EXTERN_API( OSStatus )
DrawThemePlacard				(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x0007, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeEditTextFrame			(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x0009, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeListBoxFrame			(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x000A, 0xAA74);

/* Keyboard Focus Drawing */
EXTERN_API( OSStatus )
DrawThemeFocusRect				(const Rect *			inRect,
								 Boolean 				inHasFocus)							THREEWORDINLINE(0x303C, 0x000B, 0xAA74);

/* Dialog Group Boxes and Separators */
EXTERN_API( OSStatus )
DrawThemePrimaryGroup			(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x000C, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeSecondaryGroup			(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x000D, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeSeparator				(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x000E, 0xAA74);

/* -------------------- BEGIN APPEARANCE 1.0.1 -------------------------------------*/
/* The following Appearance Manager APIs are only available */
/* in Appearance 1.0.1 or later 							*/
EXTERN_API( OSStatus )
DrawThemeModelessDialogFrame	(const Rect *			inRect,
								 ThemeDrawState 		inState)							THREEWORDINLINE(0x303C, 0x0008, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeGenericWell			(const Rect *			inRect,
								 ThemeDrawState 		inState,
								 Boolean 				inFillCenter)						THREEWORDINLINE(0x303C, 0x0022, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeFocusRegion			(RgnHandle 				inRegion,
								 Boolean 				inHasFocus)							THREEWORDINLINE(0x303C, 0x0023, 0xAA74);

EXTERN_API( Boolean )
IsThemeInColor					(SInt16 				inDepth,
								 Boolean 				inIsColorDevice)					THREEWORDINLINE(0x303C, 0x0024, 0xAA74);

/* IMPORTANT: GetThemeAccentColors will only work in the platinum theme. Any other theme will */
/* most likely return an error */
EXTERN_API( OSStatus )
GetThemeAccentColors			(CTabHandle *			outColors)							THREEWORDINLINE(0x303C, 0x0025, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeMenuBarBackground		(const Rect *			inBounds,
								 ThemeMenuBarState 		inState,
								 UInt32 				inAttributes)						THREEWORDINLINE(0x303C, 0x0018, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeMenuTitle				(const Rect *			inMenuBarRect,
								 const Rect *			inTitleRect,
								 ThemeMenuState 		inState,
								 UInt32 				inAttributes,
								 MenuTitleDrawingUPP 	inTitleProc,
								 UInt32 				inTitleData)						THREEWORDINLINE(0x303C, 0x0019, 0xAA74);

EXTERN_API( OSStatus )
GetThemeMenuBarHeight			(SInt16 *				outHeight)							THREEWORDINLINE(0x303C, 0x001A, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeMenuBackground			(const Rect *			inMenuRect,
								 ThemeMenuType 			inMenuType)							THREEWORDINLINE(0x303C, 0x001B, 0xAA74);

EXTERN_API( OSStatus )
GetThemeMenuBackgroundRegion	(const Rect *			inMenuRect,
								 ThemeMenuType 			menuType,
								 RgnHandle 				region)								THREEWORDINLINE(0x303C, 0x001C, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeMenuItem				(const Rect *			inMenuRect,
								 const Rect *			inItemRect,
								 SInt16 				inVirtualMenuTop,
								 SInt16 				inVirtualMenuBottom,
								 ThemeMenuState 		inState,
								 ThemeMenuItemType 		inItemType,
								 MenuItemDrawingUPP 	inDrawProc,
								 UInt32 				inUserData)							THREEWORDINLINE(0x303C, 0x001D, 0xAA74);

EXTERN_API( OSStatus )
DrawThemeMenuSeparator			(const Rect *			inItemRect)							THREEWORDINLINE(0x303C, 0x001E, 0xAA74);

EXTERN_API( OSStatus )
GetThemeMenuSeparatorHeight		(SInt16 *				outHeight)							THREEWORDINLINE(0x303C, 0x001F, 0xAA74);

EXTERN_API( OSStatus )
GetThemeMenuItemExtra			(ThemeMenuItemType 		inItemType,
								 SInt16 *				outHeight,
								 SInt16 *				outWidth)							THREEWORDINLINE(0x303C, 0x0020, 0xAA74);

EXTERN_API( OSStatus )
GetThemeMenuTitleExtra			(SInt16 *				outWidth,
								 Boolean 				inIsSquished)						THREEWORDINLINE(0x303C, 0x0021, 0xAA74);


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

#endif /* __APPEARANCE__ */

