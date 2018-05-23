/*
 	File:		Appearance.r
 
 	Contains:	Appearance Manager Interfaces.
 
 	Version:	Technology:	Appearance 1.0.2
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __APPEARANCE_R__
#define __APPEARANCE_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

															/*  Appearance Trap Number  */
#define _AppearanceDispatch 			0xAA74
#define gestaltAppearanceAttr 			'appr'
#define gestaltAppearanceExists 		0
#define gestaltAppearanceCompatMode 	1

#define gestaltAppearanceVersion 		'apvr'
#define kAppearanceEventClass 			'appr'				/*  Event Class  */
#define kAEThemeSwitch 					'thme'				/*  Event ID's: Theme Switched  */

#define appearanceBadBrushIndexErr 		(-30560)			/*  pattern index invalid  */
#define appearanceProcessRegisteredErr 	(-30561)
#define appearanceProcessNotRegisteredErr  (-30562)
#define appearanceBadTextColorIndexErr 	(-30563)
#define appearanceThemeHasNoAccents 	(-30564)
#define appearanceBadCursorIndexErr 	(-30565)

#define kThemeActiveDialogBackgroundBrush  1				/*  Dialogs  */
#define kThemeInactiveDialogBackgroundBrush  2				/*  Dialogs  */
#define kThemeActiveAlertBackgroundBrush  3
#define kThemeInactiveAlertBackgroundBrush  4
#define kThemeActiveModelessDialogBackgroundBrush  5
#define kThemeInactiveModelessDialogBackgroundBrush  6
#define kThemeActiveUtilityWindowBackgroundBrush  7			/*  Miscellaneous  */
#define kThemeInactiveUtilityWindowBackgroundBrush  8		/*  Miscellaneous  */
#define kThemeListViewSortColumnBackgroundBrush  9			/*  Finder  */
#define kThemeListViewBackgroundBrush 	10
#define kThemeIconLabelBackgroundBrush 	11
#define kThemeListViewSeparatorBrush 	12
#define kThemeChasingArrowsBrush 		13
#define kThemeDragHiliteBrush 			14
#define kThemeDocumentWindowBackgroundBrush  15
#define kThemeFinderWindowBackgroundBrush  16

#define kThemeActiveDialogTextColor 	1					/*  Dialogs  */
#define kThemeInactiveDialogTextColor 	2
#define kThemeActiveAlertTextColor 		3
#define kThemeInactiveAlertTextColor 	4
#define kThemeActiveModelessDialogTextColor  5
#define kThemeInactiveModelessDialogTextColor  6
#define kThemeActiveWindowHeaderTextColor  7				/*  Primitives  */
#define kThemeInactiveWindowHeaderTextColor  8
#define kThemeActivePlacardTextColor 	9					/*  Primitives  */
#define kThemeInactivePlacardTextColor 	10
#define kThemePressedPlacardTextColor 	11
#define kThemeActivePushButtonTextColor  12					/*  Primitives  */
#define kThemeInactivePushButtonTextColor  13
#define kThemePressedPushButtonTextColor  14
#define kThemeActiveBevelButtonTextColor  15				/*  Primitives  */
#define kThemeInactiveBevelButtonTextColor  16
#define kThemePressedBevelButtonTextColor  17
#define kThemeActivePopupButtonTextColor  18				/*  Primitives  */
#define kThemeInactivePopupButtonTextColor  19
#define kThemePressedPopupButtonTextColor  20
#define kThemeIconLabelTextColor 		21					/*  Finder  */
#define kThemeListViewTextColor 		22

#define kThemeActiveDocumentWindowTitleTextColor  23
#define kThemeInactiveDocumentWindowTitleTextColor  24
#define kThemeActiveMovableModalWindowTitleTextColor  25
#define kThemeInactiveMovableModalWindowTitleTextColor  26
#define kThemeActiveUtilityWindowTitleTextColor  27
#define kThemeInactiveUtilityWindowTitleTextColor  28
#define kThemeActivePopupWindowTitleColor  29
#define kThemeInactivePopupWindowTitleColor  30
#define kThemeActiveRootMenuTextColor 	31
#define kThemeSelectedRootMenuTextColor  32
#define kThemeDisabledRootMenuTextColor  33
#define kThemeActiveMenuItemTextColor 	34
#define kThemeSelectedMenuItemTextColor  35
#define kThemeDisabledMenuItemTextColor  36
#define kThemeActivePopupLabelTextColor  37
#define kThemeInactivePopupLabelTextColor  38

#define kThemeStateDisabled 			0
#define kThemeStateActive 				1
#define kThemeStatePressed 				2

#define kThemeMenuBarNormal 			0
#define kThemeMenuBarSelected 			1

#define kThemeMenuSquareMenuBar 		0x01
#define kThemeMenuActive 				0
#define kThemeMenuSelected 				1
#define kThemeMenuDisabled 				3

#define kThemeMenuTypePullDown 			0
#define kThemeMenuTypePopUp 			1
#define kThemeMenuTypeHierarchical 		2

#define kThemeMenuItemPlain 			0
#define kThemeMenuItemHierarchical 		1
#define kThemeMenuItemScrollUpArrow 	2
#define kThemeMenuItemScrollDownArrow 	3


#endif /* __APPEARANCE_R__ */

