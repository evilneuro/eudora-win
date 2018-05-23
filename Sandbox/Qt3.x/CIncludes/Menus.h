/*
 	File:		Menus.h
 
 	Contains:	Menu Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __MENUS__
#define __MENUS__

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __EVENTS__
#include <Events.h>
#endif
#ifndef __PROCESSES__
#include <Processes.h>
#endif
#ifndef __TEXTCOMMON__
#include <TextCommon.h>
#endif
#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
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

/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/*	¥ Menu Types (for Appearance 1.0 and later)											*/
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/

enum {
	kMenuStdMenuProc			= 63,
	kMenuStdMenuBarProc			= 63
};


enum {
	kMenuNoModifiers			= 0,							/* Mask for no modifiers*/
	kMenuShiftModifier			= (1 << 0),						/* Mask for shift key modifier*/
	kMenuOptionModifier			= (1 << 1),						/* Mask for option key modifier*/
	kMenuControlModifier		= (1 << 2),						/* Mask for control key modifier*/
	kMenuNoCommandModifier		= (1 << 3)						/* Mask for no command key modifier*/
};


enum {
	kMenuNoIcon					= 0,							/* No icon*/
	kMenuIconType				= 1,							/* Type for ICON*/
	kMenuShrinkIconType			= 2,							/* Type for ICON plotted 16 x 16*/
	kMenuSmallIconType			= 3,							/* Type for SICN*/
	kMenuColorIconType			= 4,							/* Type for cicn*/
	kMenuIconSuiteType			= 5,							/* Type for Icon Suite*/
	kMenuIconRefType			= 6								/* Type for Icon Ref*/
};

/* ÑÑ end of Appearance 1.0 types*/


enum {
	noMark						= 0								/*mark symbol for MarkItem*/
};


enum {
																/* menu defProc messages */
	kMenuDrawMsg				= 0,
	kMenuChooseMsg				= 1,
	kMenuSizeMsg				= 2,
	kMenuDrawItemMsg			= 4,
	kMenuCalcItemMsg			= 5,
	kMenuThemeSavvyMsg			= 7,							/* is your MDEF theme-savvy?  If so, return hex 7473 in the whichItem parameter*/
	mDrawMsg					= 0,
	mChooseMsg					= 1,
	mSizeMsg					= 2,
	mDrawItemMsg				= 4,
	mCalcItemMsg				= 5
};


enum {
	kThemeSavvyMenuResponse		= 0x7473
};


enum {
	textMenuProc				= 0,
	hMenuCmd					= 27,							/*itemCmd == 0x001B ==> hierarchical menu*/
	hierMenu					= -1,							/*a hierarchical menu - for InsertMenu call*/
	mPopUpMsg					= 3,							/*menu defProc messages - place yourself*/
	mctAllItems					= -98,							/*search for all Items for the given ID*/
	mctLastIDIndic				= -99							/*last color table entry has this in ID field*/
};



struct MenuInfo {
	short 							menuID;
	short 							menuWidth;
	short 							menuHeight;
	Handle 							menuProc;
	long 							enableFlags;
	Str255 							menuData;
};
typedef struct MenuInfo					MenuInfo;

typedef MenuInfo *						MenuPtr;
typedef MenuPtr *						MenuHandle;
/* MenuRef is obsolete.  Use MenuHandle. */
typedef MenuHandle 						MenuRef;

struct MCEntry {
	short 							mctID;						/*menu ID.  ID = 0 is the menu bar*/
	short 							mctItem;					/*menu Item. Item = 0 is a title*/
	RGBColor 						mctRGB1;					/*usage depends on ID and Item*/
	RGBColor 						mctRGB2;					/*usage depends on ID and Item*/
	RGBColor 						mctRGB3;					/*usage depends on ID and Item*/
	RGBColor 						mctRGB4;					/*usage depends on ID and Item*/
	short 							mctReserved;				/*reserved for internal use*/
};
typedef struct MCEntry					MCEntry;
typedef MCEntry *						MCEntryPtr;

typedef MCEntry 						MCTable[1];
typedef MCEntry *						MCTablePtr;
typedef MCTablePtr *					MCTableHandle;

struct MenuCRsrc {
	short 							numEntries;					/*number of entries*/
	MCTable 						mcEntryRecs;				/*ARRAY [1..numEntries] of MCEntry*/
};
typedef struct MenuCRsrc				MenuCRsrc;
typedef MenuCRsrc *						MenuCRsrcPtr;
typedef MenuCRsrcPtr *					MenuCRsrcHandle;
#if TARGET_OS_WIN32
/* QuickTime 3.0 */

struct MenuAccessKeyRec {
	short 							count;
	long 							flags;
	unsigned char 					keys[1];
};
typedef struct MenuAccessKeyRec			MenuAccessKeyRec;

typedef MenuAccessKeyRec *				MenuAccessKeyPtr;
typedef MenuAccessKeyPtr *				MenuAccessKeyHandle;
EXTERN_API( void )
SetMenuItemHotKey				(MenuRef 				menuRef,
								 short 					itemID,
								 char 					hotKey,
								 long 					flags);

#endif  /* TARGET_OS_WIN32 */

typedef CALLBACK_API( void , MenuDefProcPtr )(short message, MenuHandle theMenu, Rect *menuRect, Point hitPt, short *whichItem);
typedef CALLBACK_API( long , MenuBarDefProcPtr )(short selector, short message, short parameter1, long parameter2);
typedef CALLBACK_API( void , MenuHookProcPtr )(void );
typedef CALLBACK_API( short , MBarHookProcPtr )(Rect *menuRect);
/*
	WARNING: MBarHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef STACK_UPP_TYPE(MenuDefProcPtr) 							MenuDefUPP;
typedef STACK_UPP_TYPE(MenuBarDefProcPtr) 						MenuBarDefUPP;
typedef STACK_UPP_TYPE(MenuHookProcPtr) 						MenuHookUPP;
typedef REGISTER_UPP_TYPE(MBarHookProcPtr) 						MBarHookUPP;
enum { uppMenuDefProcInfo = 0x0000FF80 }; 						/* pascal no_return_value Func(2_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppMenuBarDefProcInfo = 0x00003AB0 }; 					/* pascal 4_bytes Func(2_bytes, 2_bytes, 2_bytes, 4_bytes) */
enum { uppMenuHookProcInfo = 0x00000000 }; 						/* pascal no_return_value Func() */
enum { uppMBarHookProcInfo = 0x000000CF }; 						/* SPECIAL_CASE_PROCINFO(12) */
#define NewMenuDefProc(userRoutine) 							(MenuDefUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuDefProcInfo, GetCurrentArchitecture())
#define NewMenuBarDefProc(userRoutine) 							(MenuBarDefUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuBarDefProcInfo, GetCurrentArchitecture())
#define NewMenuHookProc(userRoutine) 							(MenuHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuHookProcInfo, GetCurrentArchitecture())
#define NewMBarHookProc(userRoutine) 							(MBarHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMBarHookProcInfo, GetCurrentArchitecture())
#define CallMenuDefProc(userRoutine, message, theMenu, menuRect, hitPt, whichItem)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppMenuDefProcInfo, (message), (theMenu), (menuRect), (hitPt), (whichItem))
#define CallMenuBarDefProc(userRoutine, selector, message, parameter1, parameter2)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppMenuBarDefProcInfo, (selector), (message), (parameter1), (parameter2))
#define CallMenuHookProc(userRoutine) 							CALL_ZERO_PARAMETER_UPP((userRoutine), uppMenuHookProcInfo)
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallMBarHookProc can't be called from classic 68k without glue code */
#else
	#define CallMBarHookProc(userRoutine, menuRect) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppMBarHookProcInfo, (menuRect))
#endif

EXTERN_API( short )
GetMBarHeight					(void)														TWOWORDINLINE(0x3EB8, 0x0BAA);

EXTERN_API( void )
InitMenus						(void)														ONEWORDINLINE(0xA930);

EXTERN_API( MenuHandle )
NewMenu							(short 					menuID,
								 ConstStr255Param 		menuTitle)							ONEWORDINLINE(0xA931);

#if TARGET_OS_MAC
	#define MacGetMenu GetMenu
#endif
EXTERN_API( MenuHandle )
MacGetMenu						(short 					resourceID)							ONEWORDINLINE(0xA9BF);

EXTERN_API( void )
DisposeMenu						(MenuHandle 			theMenu)							ONEWORDINLINE(0xA932);

#if TARGET_OS_MAC
	#define MacAppendMenu AppendMenu
#endif
EXTERN_API( void )
MacAppendMenu					(MenuHandle 			menu,
								 ConstStr255Param 		data)								ONEWORDINLINE(0xA933);

EXTERN_API( void )
InsertResMenu					(MenuHandle 			theMenu,
								 ResType 				theType,
								 short 					afterItem)							ONEWORDINLINE(0xA951);

#if TARGET_OS_MAC
	#define MacInsertMenu InsertMenu
#endif
EXTERN_API( void )
MacInsertMenu					(MenuHandle 			theMenu,
								 short 					beforeID)							ONEWORDINLINE(0xA935);

#if TARGET_OS_MAC
	#define MacDeleteMenu DeleteMenu
#endif
EXTERN_API( void )
MacDeleteMenu					(short 					menuID)								ONEWORDINLINE(0xA936);

EXTERN_API( void )
AppendResMenu					(MenuHandle 			theMenu,
								 ResType 				theType)							ONEWORDINLINE(0xA94D);

#if TARGET_OS_MAC
	#define MacInsertMenuItem InsertMenuItem
#endif
EXTERN_API( void )
MacInsertMenuItem				(MenuHandle 			theMenu,
								 ConstStr255Param 		itemString,
								 short 					afterItem)							ONEWORDINLINE(0xA826);

EXTERN_API( void )
DeleteMenuItem					(MenuHandle 			theMenu,
								 short 					item)								ONEWORDINLINE(0xA952);

EXTERN_API( long )
MenuKey							(CharParameter 			ch)									ONEWORDINLINE(0xA93E);

EXTERN_API( void )
HiliteMenu						(short 					menuID)								ONEWORDINLINE(0xA938);

EXTERN_API( void )
SetMenuItemText					(MenuHandle 			theMenu,
								 short 					item,
								 ConstStr255Param 		itemString)							ONEWORDINLINE(0xA947);

EXTERN_API( void )
GetMenuItemText					(MenuHandle 			theMenu,
								 short 					item,
								 Str255 				itemString)							ONEWORDINLINE(0xA946);

EXTERN_API( void )
SetItemMark						(MenuHandle 			theMenu,
								 short 					item,
								 CharParameter 			markChar)							ONEWORDINLINE(0xA944);

EXTERN_API( void )
GetItemMark						(MenuHandle 			theMenu,
								 short 					item,
								 CharParameter *		markChar)							ONEWORDINLINE(0xA943);

EXTERN_API( void )
SetItemCmd						(MenuHandle 			theMenu,
								 short 					item,
								 CharParameter 			cmdChar)							ONEWORDINLINE(0xA84F);

EXTERN_API( void )
GetItemCmd						(MenuHandle 			theMenu,
								 short 					item,
								 CharParameter *		cmdChar)							ONEWORDINLINE(0xA84E);

EXTERN_API( void )
SetItemIcon						(MenuHandle 			theMenu,
								 short 					item,
								 short 					iconIndex)							ONEWORDINLINE(0xA940);

EXTERN_API( void )
GetItemIcon						(MenuHandle 			theMenu,
								 short 					item,
								 short *				iconIndex)							ONEWORDINLINE(0xA93F);

EXTERN_API( void )
SetItemStyle					(MenuHandle 			theMenu,
								 short 					item,
								 StyleParameter 		chStyle)							ONEWORDINLINE(0xA942);

EXTERN_API( void )
GetItemStyle					(MenuHandle 			theMenu,
								 short 					item,
								 Style *				chStyle);

EXTERN_API( MenuHandle )
GetMenuHandle					(short 					menuID)								ONEWORDINLINE(0xA949);

EXTERN_API( void )
CalcMenuSize					(MenuHandle 			theMenu)							ONEWORDINLINE(0xA948);

EXTERN_API( void )
DisableItem						(MenuHandle 			theMenu,
								 short 					item)								ONEWORDINLINE(0xA93A);

EXTERN_API( void )
EnableItem						(MenuHandle 			theMenu,
								 short 					item)								ONEWORDINLINE(0xA939);

EXTERN_API( void )
FlashMenuBar					(short 					menuID)								ONEWORDINLINE(0xA94C);

EXTERN_API( long )
PopUpMenuSelect					(MenuHandle 			menu,
								 short 					top,
								 short 					left,
								 short 					popUpItem)							ONEWORDINLINE(0xA80B);

EXTERN_API( long )
MenuChoice						(void)														ONEWORDINLINE(0xAA66);

EXTERN_API( void )
DeleteMCEntries					(short 					menuID,
								 short 					menuItem)							ONEWORDINLINE(0xAA60);

EXTERN_API( MCTableHandle )
GetMCInfo						(void)														ONEWORDINLINE(0xAA61);

EXTERN_API( void )
SetMCInfo						(MCTableHandle 			menuCTbl)							ONEWORDINLINE(0xAA62);

EXTERN_API( void )
DisposeMCInfo					(MCTableHandle 			menuCTbl)							ONEWORDINLINE(0xAA63);

EXTERN_API( MCEntryPtr )
GetMCEntry						(short 					menuID,
								 short 					menuItem)							ONEWORDINLINE(0xAA64);

EXTERN_API( void )
SetMCEntries					(short 					numEntries,
								 MCTablePtr 			menuCEntries)						ONEWORDINLINE(0xAA65);

#if TARGET_OS_MAC
	#define MacDrawMenuBar DrawMenuBar
#endif
EXTERN_API( void )
MacDrawMenuBar					(void)														ONEWORDINLINE(0xA937);

EXTERN_API( void )
InvalMenuBar					(void)														ONEWORDINLINE(0xA81D);

EXTERN_API( void )
InitProcMenu					(short 					resID)								ONEWORDINLINE(0xA808);

EXTERN_API( Handle )
GetMenuBar						(void)														ONEWORDINLINE(0xA93B);

EXTERN_API( void )
SetMenuBar						(Handle 				menuList)							ONEWORDINLINE(0xA93C);

EXTERN_API( Boolean )
SystemEdit						(short 					editCmd)							ONEWORDINLINE(0xA9C2);

EXTERN_API( void )
SystemMenu						(long 					menuResult)							ONEWORDINLINE(0xA9B5);

EXTERN_API( Handle )
GetNewMBar						(short 					menuBarID)							ONEWORDINLINE(0xA9C0);

EXTERN_API( void )
ClearMenuBar					(void)														ONEWORDINLINE(0xA934);

EXTERN_API( void )
CheckItem						(MenuHandle 			theMenu,
								 short 					item,
								 Boolean 				checked)							ONEWORDINLINE(0xA945);

EXTERN_API( short )
CountMItems						(MenuHandle 			theMenu)							ONEWORDINLINE(0xA950);

EXTERN_API( void )
SetMenuFlash					(short 					count)								ONEWORDINLINE(0xA94A);

EXTERN_API( long )
MenuSelect						(Point 					startPt)							ONEWORDINLINE(0xA93D);

EXTERN_API( void )
InsertFontResMenu				(MenuHandle 			theMenu,
								 short 					afterItem,
								 short 					scriptFilter)						THREEWORDINLINE(0x303C, 0x0400, 0xA825);

EXTERN_API( void )
InsertIntlResMenu				(MenuHandle 			theMenu,
								 ResType 				theType,
								 short 					afterItem,
								 short 					scriptFilter)						THREEWORDINLINE(0x303C, 0x0601, 0xA825);


/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/*	¥ Appearance 1.0 and later Menu Manager routines									*/
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
EXTERN_API( UInt32 )
MenuEvent						(const EventRecord *	inEvent)							THREEWORDINLINE(0x303C, 0x020C, 0xA825);

EXTERN_API( OSErr )
SetMenuItemCommandID			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt32 				inCommandID)						THREEWORDINLINE(0x303C, 0x0502, 0xA825);

EXTERN_API( OSErr )
GetMenuItemCommandID			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt32 *				outCommandID)						THREEWORDINLINE(0x303C, 0x0503, 0xA825);

EXTERN_API( OSErr )
SetMenuItemModifiers			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt8 					inModifiers)						THREEWORDINLINE(0x303C, 0x0404, 0xA825);

EXTERN_API( OSErr )
GetMenuItemModifiers			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt8 *				outModifiers)						THREEWORDINLINE(0x303C, 0x0505, 0xA825);

EXTERN_API( OSErr )
SetMenuItemIconHandle			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt8 					inIconType,
								 Handle 				inIconHandle)						THREEWORDINLINE(0x303C, 0x0606, 0xA825);

EXTERN_API( OSErr )
GetMenuItemIconHandle			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt8 *				outIconType,
								 Handle *				outIconHandle)						THREEWORDINLINE(0x303C, 0x0707, 0xA825);

EXTERN_API( OSErr )
SetMenuItemTextEncoding			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 TextEncoding 			inScriptID)							THREEWORDINLINE(0x303C, 0x0408, 0xA825);

EXTERN_API( OSErr )
GetMenuItemTextEncoding			(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 TextEncoding *			outScriptID)						THREEWORDINLINE(0x303C, 0x0509, 0xA825);

EXTERN_API( OSErr )
SetMenuItemHierarchicalID		(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 SInt16 				inHierID)							THREEWORDINLINE(0x303C, 0x040D, 0xA825);

EXTERN_API( OSErr )
GetMenuItemHierarchicalID		(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 SInt16 *				outHierID)							THREEWORDINLINE(0x303C, 0x050E, 0xA825);

EXTERN_API( OSErr )
SetMenuItemFontID				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 SInt16 				inFontID)							THREEWORDINLINE(0x303C, 0x040F, 0xA825);

EXTERN_API( OSErr )
GetMenuItemFontID				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 SInt16 *				outFontID)							THREEWORDINLINE(0x303C, 0x0510, 0xA825);

EXTERN_API( OSErr )
SetMenuItemRefCon				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt32 				inRefCon)							THREEWORDINLINE(0x303C, 0x050A, 0xA825);

EXTERN_API( OSErr )
GetMenuItemRefCon				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt32 *				outRefCon)							THREEWORDINLINE(0x303C, 0x050B, 0xA825);

EXTERN_API( OSErr )
SetMenuItemRefCon2				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt32 				inRefCon2)							THREEWORDINLINE(0x303C, 0x0511, 0xA825);

EXTERN_API( OSErr )
GetMenuItemRefCon2				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 UInt32 *				outRefCon2)							THREEWORDINLINE(0x303C, 0x0512, 0xA825);

EXTERN_API( OSErr )
SetMenuItemKeyGlyph				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 SInt16 				inGlyph)							THREEWORDINLINE(0x303C, 0x0513, 0xA825);

EXTERN_API( OSErr )
GetMenuItemKeyGlyph				(MenuHandle 			inMenu,
								 SInt16 				inItem,
								 SInt16 *				outGlyph)							THREEWORDINLINE(0x303C, 0x0514, 0xA825);

/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/*	¥ Contextual Menu routines and constants											*/
/*	available with Conxtextual Menu extension 1.0 and later								*/
/*ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ*/
/* Gestalt Selector for classic 68K apps only. */
/* CFM apps should weak link and check the symbols. */

enum {
	gestaltContextualMenuAttr	= FOUR_CHAR_CODE('cmnu'),
	gestaltContextualMenuUnusedBit = 0,
	gestaltContextualMenuTrapAvailable = 1
};

/* Values indicating what kind of help the application supports */

enum {
	kCMHelpItemNoHelp			= 0,
	kCMHelpItemAppleGuide		= 1,
	kCMHelpItemOtherHelp		= 2
};

/* Values indicating what was chosen from the menu */

enum {
	kCMNothingSelected			= 0,
	kCMMenuItemSelected			= 1,
	kCMShowHelpSelected			= 3
};

EXTERN_API( OSStatus )
InitContextualMenus				(void)														TWOWORDINLINE(0x7001, 0xAA72);

EXTERN_API( Boolean )
IsShowContextualMenuClick		(const EventRecord *	inEvent)							TWOWORDINLINE(0x7002, 0xAA72);

EXTERN_API( OSStatus )
ContextualMenuSelect			(MenuHandle 			inMenu,
								 Point 					inGlobalLocation,
								 Boolean 				inReserved,
								 UInt32 				inHelpType,
								 ConstStr255Param 		inHelpItemString,
								 const AEDesc *			inSelection,
								 UInt32 *				outUserSelectionType,
								 SInt16 *				outMenuID,
								 UInt16 *				outMenuItem)						TWOWORDINLINE(0x7003, 0xAA72);

EXTERN_API( Boolean )
ProcessIsContextualMenuClient	(ProcessSerialNumber *	inPSN)								TWOWORDINLINE(0x7004, 0xAA72);




/* 
	CheckItem, CountMItems, and SetMenuFlash should have better names.
	But the shipping InterfaceLib has the old names, so we need
	to define the new and old names and use macros to map the 
	new names to old names for linking with InterfaceLib.
*/
#if TARGET_OS_MAC
	#define CheckMenuItem MacCheckMenuItem
#endif
#define MacCheckMenuItem( menu, item, checked ) CheckItem( menu, item, checked )
#define CountMenuItems( menu ) 					CountMItems( menu )
#define SetMenuFlashCount( count ) 				SetMenuFlash( count )


#if CGLUESUPPORTED
EXTERN_API_C( MenuHandle )
newmenu							(short 					menuID,
								 const char *			menuTitle);

EXTERN_API_C( void )
appendmenu						(MenuHandle 			menu,
								 const char *			data);

EXTERN_API_C( void )
insertmenuitem					(MenuHandle 			theMenu,
								 const char *			itemString,
								 short 					afterItem);

EXTERN_API_C( long )
menuselect						(const Point *			startPt);

EXTERN_API_C( void )
setmenuitemtext					(MenuHandle 			menu,
								 short 					item,
								 const char *			itemString);

EXTERN_API_C( void )
getmenuitemtext					(MenuHandle 			menu,
								 short 					item,
								 char *					itemString);

#endif  /* CGLUESUPPORTED */

#if OLDROUTINENAMES
#define AddResMenu(theMenu, theType) AppendResMenu(theMenu, theType)
#define InsMenuItem(theMenu, itemString, afterItem)	InsertMenuItem(theMenu, itemString, afterItem)
#define DelMenuItem( theMenu, item ) DeleteMenuItem( theMenu, item )
#if TARGET_OS_MAC
#define SetItem MacSetItem
#define GetItem MacGetItem
#endif
#define MacSetItem(theMenu, item, itemString) SetMenuItemText(theMenu, item, itemString)
#define MacGetItem(theMenu, item, itemString) GetMenuItemText(theMenu, item, itemString)
#define GetMHandle(menuID) GetMenuHandle(menuID)
#define DelMCEntries(menuID, menuItem) DeleteMCEntries(menuID, menuItem)
#define DispMCInfo(menuCTbl) DisposeMCInfo(menuCTbl)
#if CGLUESUPPORTED
#define addresmenu(menu, data) appendresmenu(menu, data)
#define getitem(menu, item, itemString) getmenuitemtext(menu, item, itemString)
#define setitem(menu, item, itemString) setmenuitemtext(menu, item, itemString)
#define insmenuitem(theMenu, itemString, afterItem)	insertmenuitem(theMenu, itemString, afterItem)
#endif	/* CGLUESUPPORTED */
#endif  /* OLDROUTINENAMES */



#if TARGET_OS_WIN32
#endif  /* TARGET_OS_WIN32 */


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

#endif /* __MENUS__ */

