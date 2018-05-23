/*
 	File:		ColorPickerComponents.h
 
 	Contains:	Color Picker Component Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __COLORPICKERCOMPONENTS__
#define __COLORPICKERCOMPONENTS__

#ifndef __COLORPICKER__
#include <ColorPicker.h>
#endif
#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif
#ifndef __BALLOONS__
#include <Balloons.h>
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
	kPickerComponentType		= FOUR_CHAR_CODE('cpkr')
};


enum {
	kPickerInit					= 0,
	kPickerTestGraphicsWorld	= 1,
	kPickerGetDialog			= 2,
	kPickerGetItemList			= 3,
	kPickerGetColor				= 4,
	kPickerSetColor				= 5,
	kPickerEvent				= 6,
	kPickerEdit					= 7,
	kPickerSetVisibility		= 8,
	kPickerDisplay				= 9,
	kPickerItemHit				= 10,
	kPickerSetBaseItem			= 11,
	kPickerGetProfile			= 12,
	kPickerSetProfile			= 13,
	kPickerGetPrompt			= 14,
	kPickerSetPrompt			= 15,
	kPickerGetIconData			= 16,
	kPickerGetEditMenuState		= 17,
	kPickerSetOrigin			= 18,
	kPickerExtractHelpItem		= 19,
	kPickerSetColorChangedProc	= 20,
	kNPickerGetColor			= 21,
	kNPickerSetColor			= 22,
	kNPickerGetProfile			= 23,
	kNPickerSetProfile			= 24,
	kNPickerSetColorChangedProc	= 25
};

/* These structs were moved here from the ColorPicker header.*/

enum {
	kPickerDidNothing			= 0,							/* was kDidNothing */
	kPickerColorChanged			= 1,							/* was kColorChanged */
	kPickerOkHit				= 2,							/* was kOkHit */
	kPickerCancelHit			= 3,							/* was kCancelHit */
	kPickerNewPickerChosen		= 4,							/* was kNewPickerChosen */
	kPickerApplItemHit			= 5								/* was kApplItemHit */
};

typedef short 							PickerAction;

enum {
	kOriginalColor				= 0,
	kNewColor					= 1
};

typedef short 							PickerColorType;

enum {
	kPickerCut					= 0,							/* was kCut */
	kPickerCopy					= 1,							/* was kCopy */
	kPickerPaste				= 2,							/* was kPaste */
	kPickerClear				= 3,							/* was kClear */
	kPickerUndo					= 4								/* was kUndo */
};

typedef short 							PickerEditOperation;

enum {
	kPickerMouseDown			= 0,							/* was kMouseDown */
	kPickerKeyDown				= 1,							/* was kKeyDown */
	kPickerFieldEntered			= 2,							/* was kFieldEntered */
	kPickerFieldLeft			= 3,							/* was kFieldLeft */
	kPickerCutOp				= 4,							/* was kCutOp */
	kPickerCopyOp				= 5,							/* was kCopyOp */
	kPickerPasteOp				= 6,							/* was kPasteOp */
	kPickerClearOp				= 7,							/* was kClearOp */
	kPickerUndoOp				= 8								/* was kUndoOp */
};

typedef short 							PickerItemModifier;
/* These are for the flags field in the picker's 'thng' resource. */

enum {
	kPickerCanDoColor			= 1,							/* was CanDoColor */
	kPickerCanDoBlackWhite		= 2,							/* was CanDoBlackWhite */
	kPickerAlwaysModifiesPalette = 4,							/* was AlwaysModifiesPalette */
	kPickerMayModifyPalette		= 8,							/* was MayModifyPalette */
	kPickerIsColorSyncAware		= 16,							/* was PickerIsColorSyncAware */
	kPickerCanDoSystemDialog	= 32,							/* was CanDoSystemDialog */
	kPickerCanDoApplDialog		= 64,							/* was CanDoApplDialog */
	kPickerHasOwnDialog			= 128,							/* was HasOwnDialog */
	kPickerCanDetach			= 256,							/* was CanDetach */
	kPickerIsColorSync2Aware	= 512							/* was PickerIsColorSync2Aware */
};


enum {
	kPickerNoForcast			= 0,							/* was kNoForcast */
	kPickerMenuChoice			= 1,							/* was kMenuChoice */
	kPickerDialogAccept			= 2,							/* was kDialogAccept */
	kPickerDialogCancel			= 3,							/* was kDialogCancel */
	kPickerLeaveFocus			= 4,							/* was kLeaveFocus */
	kPickerSwitch				= 5,
	kPickerNormalKeyDown		= 6,							/* was kNormalKeyDown */
	kPickerNormalMouseDown		= 7								/* was kNormalMouseDown */
};

typedef short 							PickerEventForcaster;

struct PickerIconData {
	short 							scriptCode;
	short 							iconSuiteID;
	ResType 						helpResType;
	short 							helpResID;
};
typedef struct PickerIconData			PickerIconData;

struct PickerInitData {
	DialogPtr 						pickerDialog;
	DialogPtr 						choicesDialog;
	long 							flags;
	Picker 							yourself;
};
typedef struct PickerInitData			PickerInitData;

struct PickerMenuState {
	Boolean 						cutEnabled;
	Boolean 						copyEnabled;
	Boolean 						pasteEnabled;
	Boolean 						clearEnabled;
	Boolean 						undoEnabled;
	SInt8 							filler;
	Str255 							undoString;
};
typedef struct PickerMenuState			PickerMenuState;

struct SystemDialogInfo {
	long 							flags;
	long 							pickerType;
	DialogPlacementSpec 			placeWhere;
	Point 							dialogOrigin;
	PickerMenuItemInfo 				mInfo;
};
typedef struct SystemDialogInfo			SystemDialogInfo;

struct PickerDialogInfo {
	long 							flags;
	long 							pickerType;
	Point *							dialogOrigin;
	PickerMenuItemInfo 				mInfo;
};
typedef struct PickerDialogInfo			PickerDialogInfo;

struct ApplicationDialogInfo {
	long 							flags;
	long 							pickerType;
	DialogPtr 						theDialog;
	Point 							pickerOrigin;
	PickerMenuItemInfo 				mInfo;
};
typedef struct ApplicationDialogInfo	ApplicationDialogInfo;

struct PickerEventData {
	EventRecord *					event;
	PickerAction 					action;
	short 							itemHit;
	Boolean 						handled;
	SInt8 							filler;
	ColorChangedUPP 				colorProc;
	long 							colorProcData;
	PickerEventForcaster 			forcast;
};
typedef struct PickerEventData			PickerEventData;

struct PickerEditData {
	PickerEditOperation 			theEdit;
	PickerAction 					action;
	Boolean 						handled;
	SInt8 							filler;
};
typedef struct PickerEditData			PickerEditData;

struct PickerItemHitData {
	short 							itemHit;
	PickerItemModifier 				iMod;
	PickerAction 					action;
	ColorChangedUPP 				colorProc;
	long 							colorProcData;
	Point 							where;
};
typedef struct PickerItemHitData		PickerItemHitData;

struct PickerHelpItemInfo {
	long 							options;
	Point 							tip;
	Rect 							altRect;
	short 							theProc;
	short 							helpVariant;
	HMMessageRecord 				helpMessage;
};
typedef struct PickerHelpItemInfo		PickerHelpItemInfo;
#if OLDROUTINENAMES

enum {
	kInitPicker					= kPickerInit,
	kTestGraphicsWorld			= kPickerTestGraphicsWorld,
	kGetDialog					= kPickerGetDialog,
	kGetItemList				= kPickerGetItemList,
	kGetColor					= kPickerGetColor,
	kSetColor					= kPickerSetColor,
	kEvent						= kPickerEvent,
	kEdit						= kPickerEdit,
	kSetVisibility				= kPickerSetVisibility,
	kDrawPicker					= kPickerDisplay,
	kItemHit					= kPickerItemHit,
	kSetBaseItem				= kPickerSetBaseItem,
	kGetProfile					= kPickerGetProfile,
	kSetProfile					= kPickerSetProfile,
	kGetPrompt					= kPickerGetPrompt,
	kSetPrompt					= kPickerSetPrompt,
	kGetIconData				= kPickerGetIconData,
	kGetEditMenuState			= kPickerGetEditMenuState,
	kSetOrigin					= kPickerSetOrigin,
	kExtractHelpItem			= kPickerExtractHelpItem
};


enum {
	kDidNothing					= kPickerDidNothing,
	kColorChanged				= kPickerColorChanged,
	kOkHit						= kPickerOkHit,
	kCancelHit					= kPickerCancelHit,
	kNewPickerChosen			= kPickerNewPickerChosen,
	kApplItemHit				= kPickerApplItemHit
};


enum {
	kCut						= kPickerCut,
	kCopy						= kPickerCopy,
	kPaste						= kPickerPaste,
	kClear						= kPickerClear,
	kUndo						= kPickerUndo
};


enum {
	kMouseDown					= kPickerMouseDown,
	kKeyDown					= kPickerKeyDown,
	kFieldEntered				= kPickerFieldEntered,
	kFieldLeft					= kPickerFieldLeft,
	kCutOp						= kPickerCutOp,
	kCopyOp						= kPickerCopyOp,
	kPasteOp					= kPickerPasteOp,
	kClearOp					= kPickerClearOp,
	kUndoOp						= kPickerUndoOp
};


enum {
	kNoForcast					= kPickerNoForcast,
	kMenuChoice					= kPickerMenuChoice,
	kDialogAccept				= kPickerDialogAccept,
	kDialogCancel				= kPickerDialogCancel,
	kLeaveFocus					= kPickerLeaveFocus,
	kNormalKeyDown				= kPickerNormalKeyDown,
	kNormalMouseDown			= kPickerNormalMouseDown
};


typedef short 							ColorType;
typedef short 							EditOperation;
typedef short 							ItemModifier;
typedef short 							EventForcaster;

struct EventData {
	EventRecord *					event;
	PickerAction 					action;
	short 							itemHit;
	Boolean 						handled;
	SInt8 							filler;
	ColorChangedUPP 				colorProc;
	long 							colorProcData;
	EventForcaster 					forcast;
};
typedef struct EventData				EventData;

struct EditData {
	EditOperation 					theEdit;
	PickerAction 					action;
	Boolean 						handled;
	SInt8 							filler;
};
typedef struct EditData					EditData;

struct ItemHitData {
	short 							itemHit;
	ItemModifier 					iMod;
	PickerAction 					action;
	ColorChangedUPP 				colorProc;
	long 							colorProcData;
	Point 							where;
};
typedef struct ItemHitData				ItemHitData;

struct HelpItemInfo {
	long 							options;
	Point 							tip;
	Rect 							altRect;
	short 							theProc;
	short 							helpVariant;
	HMMessageRecord 				helpMessage;
};
typedef struct HelpItemInfo				HelpItemInfo;
#endif  /* OLDROUTINENAMES */

typedef CALLBACK_API( ComponentResult , PickerOpenProcPtr )(long storage, ComponentInstance self);
typedef CALLBACK_API( ComponentResult , PickerCloseProcPtr )(long storage, ComponentInstance self);
typedef CALLBACK_API( ComponentResult , PickerCanDoProcPtr )(long storage, short selector);
typedef CALLBACK_API( ComponentResult , PickerVersionProcPtr )(long storage);
typedef CALLBACK_API( ComponentResult , PickerRegisterProcPtr )(long storage);
typedef CALLBACK_API( ComponentResult , PickerSetTargetProcPtr )(long storage, ComponentInstance topOfCallChain);
typedef STACK_UPP_TYPE(PickerOpenProcPtr) 						PickerOpenUPP;
typedef STACK_UPP_TYPE(PickerCloseProcPtr) 						PickerCloseUPP;
typedef STACK_UPP_TYPE(PickerCanDoProcPtr) 						PickerCanDoUPP;
typedef STACK_UPP_TYPE(PickerVersionProcPtr) 					PickerVersionUPP;
typedef STACK_UPP_TYPE(PickerRegisterProcPtr) 					PickerRegisterUPP;
typedef STACK_UPP_TYPE(PickerSetTargetProcPtr) 					PickerSetTargetUPP;
EXTERN_API( ComponentResult )
PickerInit						(long 					storage,
								 PickerInitData *		data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0000, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerInitProcPtr )(long storage, PickerInitData *data);
EXTERN_API( ComponentResult )
PickerTestGraphicsWorld			(long 					storage,
								 PickerInitData *		data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0001, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerTestGraphicsWorldProcPtr )(long storage, PickerInitData *data);
EXTERN_API( ComponentResult )
PickerGetDialog					(long 					storage)							FIVEWORDINLINE(0x2F3C, 0x0000, 0x0002, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetDialogProcPtr )(long storage);
EXTERN_API( ComponentResult )
PickerGetItemList				(long 					storage)							FIVEWORDINLINE(0x2F3C, 0x0000, 0x0003, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetItemListProcPtr )(long storage);
EXTERN_API( ComponentResult )
PickerGetColor					(long 					storage,
								 PickerColorType 		whichColor,
								 PMColorPtr 			color)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0004, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetColorProcPtr )(long storage, PickerColorType whichColor, PMColorPtr color);
EXTERN_API( ComponentResult )
PickerSetColor					(long 					storage,
								 PickerColorType 		whichColor,
								 PMColorPtr 			color)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0005, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetColorProcPtr )(long storage, PickerColorType whichColor, PMColorPtr color);
EXTERN_API( ComponentResult )
PickerEvent						(long 					storage,
								 PickerEventData *		data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0006, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerEventProcPtr )(long storage, PickerEventData *data);
EXTERN_API( ComponentResult )
PickerEdit						(long 					storage,
								 PickerEditData *		data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerEditProcPtr )(long storage, PickerEditData *data);
EXTERN_API( ComponentResult )
PickerSetVisibility				(long 					storage,
								 Boolean 				visible)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x0008, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetVisibilityProcPtr )(long storage, Boolean visible);
EXTERN_API( ComponentResult )
PickerDisplay					(long 					storage)							FIVEWORDINLINE(0x2F3C, 0x0000, 0x0009, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerDisplayProcPtr )(long storage);
EXTERN_API( ComponentResult )
PickerItemHit					(long 					storage,
								 PickerItemHitData *	data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000A, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerItemHitProcPtr )(long storage, PickerItemHitData *data);
EXTERN_API( ComponentResult )
PickerSetBaseItem				(long 					storage,
								 short 					baseItem)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x000B, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetBaseItemProcPtr )(long storage, short baseItem);
EXTERN_API( ComponentResult )
PickerGetProfile				(long 					storage)							FIVEWORDINLINE(0x2F3C, 0x0000, 0x000C, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetProfileProcPtr )(long storage);
EXTERN_API( ComponentResult )
PickerSetProfile				(long 					storage,
								 CMProfileHandle 		profile)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000D, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetProfileProcPtr )(long storage, CMProfileHandle profile);
EXTERN_API( ComponentResult )
PickerGetPrompt					(long 					storage,
								 Str255 				prompt)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000E, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetPromptProcPtr )(long storage, Str255 prompt);
EXTERN_API( ComponentResult )
PickerSetPrompt					(long 					storage,
								 ConstStr255Param 		prompt)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000F, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetPromptProcPtr )(long storage, ConstStr255Param prompt);
EXTERN_API( ComponentResult )
PickerGetIconData				(long 					storage,
								 PickerIconData *		data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0010, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetIconDataProcPtr )(long storage, PickerIconData *data);
EXTERN_API( ComponentResult )
PickerGetEditMenuState			(long 					storage,
								 PickerMenuState *		mState)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0011, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerGetEditMenuStateProcPtr )(long storage, PickerMenuState *mState);
EXTERN_API( ComponentResult )
PickerSetOrigin					(long 					storage,
								 Point 					where)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0012, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetOriginProcPtr )(long storage, Point where);
/*	Below are the ColorPicker 2.1 routines.*/


EXTERN_API( ComponentResult )
PickerSetColorChangedProc		(long 					storage,
								 ColorChangedUPP 		colorProc,
								 long 					colorProcData)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0014, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerSetColorChangedProcProcPtr )(long storage, NColorChangedUPP colorProc, long colorProcData);
/* New Color Picker 2.1 messages.  If you don't wish to support these you should already be... */
/* returning a badComponentSelector in your main entry routine.  They have new selectors*/
EXTERN_API( ComponentResult )
NPickerGetColor					(long 					storage,
								 PickerColorType 		whichColor,
								 NPMColor *				color)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0015, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , NPickerGetColorProcPtr )(long storage, PickerColorType whichColor, NPMColor *color);
EXTERN_API( ComponentResult )
NPickerSetColor					(long 					storage,
								 PickerColorType 		whichColor,
								 NPMColor *				color)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0016, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , NPickerSetColorProcPtr )(long storage, PickerColorType whichColor, NPMColor *color);
EXTERN_API( ComponentResult )
NPickerGetProfile				(long 					storage,
								 CMProfileRef *			profile)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0017, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , NPickerGetProfileProcPtr )(long storage, CMProfileRef *profile);
EXTERN_API( ComponentResult )
NPickerSetProfile				(long 					storage,
								 CMProfileRef 			profile)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0018, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , NPickerSetProfileProcPtr )(long storage, CMProfileRef profile);
EXTERN_API( ComponentResult )
NPickerSetColorChangedProc		(long 					storage,
								 NColorChangedUPP 		colorProc,
								 long 					colorProcData)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0019, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , NPickerSetColorChangedProcProcPtr )(long storage, NColorChangedUPP colorProc, long colorProcData);
EXTERN_API( ComponentResult )
PickerExtractHelpItem			(long 					storage,
								 short 					itemNo,
								 short 					whichMsg,
								 PickerHelpItemInfo *	helpInfo)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0013, 0x7000, 0xA82A);

typedef CALLBACK_API( ComponentResult , PickerExtractHelpItemProcPtr )(long storage, short itemNo, short whichMsg, PickerHelpItemInfo *helpInfo);
typedef STACK_UPP_TYPE(PickerInitProcPtr) 						PickerInitUPP;
typedef STACK_UPP_TYPE(PickerTestGraphicsWorldProcPtr) 			PickerTestGraphicsWorldUPP;
typedef STACK_UPP_TYPE(PickerGetDialogProcPtr) 					PickerGetDialogUPP;
typedef STACK_UPP_TYPE(PickerGetItemListProcPtr) 				PickerGetItemListUPP;
typedef STACK_UPP_TYPE(PickerGetColorProcPtr) 					PickerGetColorUPP;
typedef STACK_UPP_TYPE(PickerSetColorProcPtr) 					PickerSetColorUPP;
typedef STACK_UPP_TYPE(PickerEventProcPtr) 						PickerEventUPP;
typedef STACK_UPP_TYPE(PickerEditProcPtr) 						PickerEditUPP;
typedef STACK_UPP_TYPE(PickerSetVisibilityProcPtr) 				PickerSetVisibilityUPP;
typedef STACK_UPP_TYPE(PickerDisplayProcPtr) 					PickerDisplayUPP;
typedef STACK_UPP_TYPE(PickerItemHitProcPtr) 					PickerItemHitUPP;
typedef STACK_UPP_TYPE(PickerSetBaseItemProcPtr) 				PickerSetBaseItemUPP;
typedef STACK_UPP_TYPE(PickerGetProfileProcPtr) 				PickerGetProfileUPP;
typedef STACK_UPP_TYPE(PickerSetProfileProcPtr) 				PickerSetProfileUPP;
typedef STACK_UPP_TYPE(PickerGetPromptProcPtr) 					PickerGetPromptUPP;
typedef STACK_UPP_TYPE(PickerSetPromptProcPtr) 					PickerSetPromptUPP;
typedef STACK_UPP_TYPE(PickerGetIconDataProcPtr) 				PickerGetIconDataUPP;
typedef STACK_UPP_TYPE(PickerGetEditMenuStateProcPtr) 			PickerGetEditMenuStateUPP;
typedef STACK_UPP_TYPE(PickerSetOriginProcPtr) 					PickerSetOriginUPP;
typedef STACK_UPP_TYPE(PickerSetColorChangedProcProcPtr) 		PickerSetColorChangedProcUPP;
typedef STACK_UPP_TYPE(NPickerGetColorProcPtr) 					NPickerGetColorUPP;
typedef STACK_UPP_TYPE(NPickerSetColorProcPtr) 					NPickerSetColorUPP;
typedef STACK_UPP_TYPE(NPickerGetProfileProcPtr) 				NPickerGetProfileUPP;
typedef STACK_UPP_TYPE(NPickerSetProfileProcPtr) 				NPickerSetProfileUPP;
typedef STACK_UPP_TYPE(NPickerSetColorChangedProcProcPtr) 		NPickerSetColorChangedProcUPP;
typedef STACK_UPP_TYPE(PickerExtractHelpItemProcPtr) 			PickerExtractHelpItemUPP;
enum { uppPickerOpenProcInfo = 0x000003F0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerCloseProcInfo = 0x000003F0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerCanDoProcInfo = 0x000002F0 }; 					/* pascal 4_bytes Func(4_bytes, 2_bytes) */
enum { uppPickerVersionProcInfo = 0x000000F0 }; 				/* pascal 4_bytes Func(4_bytes) */
enum { uppPickerRegisterProcInfo = 0x000000F0 }; 				/* pascal 4_bytes Func(4_bytes) */
enum { uppPickerSetTargetProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerInitProcInfo = 0x000003F0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerTestGraphicsWorldProcInfo = 0x000003F0 }; 		/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerGetDialogProcInfo = 0x000000F0 }; 				/* pascal 4_bytes Func(4_bytes) */
enum { uppPickerGetItemListProcInfo = 0x000000F0 }; 			/* pascal 4_bytes Func(4_bytes) */
enum { uppPickerGetColorProcInfo = 0x00000EF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppPickerSetColorProcInfo = 0x00000EF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppPickerEventProcInfo = 0x000003F0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerEditProcInfo = 0x000003F0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerSetVisibilityProcInfo = 0x000001F0 }; 			/* pascal 4_bytes Func(4_bytes, 1_byte) */
enum { uppPickerDisplayProcInfo = 0x000000F0 }; 				/* pascal 4_bytes Func(4_bytes) */
enum { uppPickerItemHitProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerSetBaseItemProcInfo = 0x000002F0 }; 			/* pascal 4_bytes Func(4_bytes, 2_bytes) */
enum { uppPickerGetProfileProcInfo = 0x000000F0 }; 				/* pascal 4_bytes Func(4_bytes) */
enum { uppPickerSetProfileProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerGetPromptProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerSetPromptProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerGetIconDataProcInfo = 0x000003F0 }; 			/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerGetEditMenuStateProcInfo = 0x000003F0 }; 		/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerSetOriginProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppPickerSetColorChangedProcProcInfo = 0x00000FF0 }; 	/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes) */
enum { uppNPickerGetColorProcInfo = 0x00000EF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppNPickerSetColorProcInfo = 0x00000EF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppNPickerGetProfileProcInfo = 0x000003F0 }; 			/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppNPickerSetProfileProcInfo = 0x000003F0 }; 			/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppNPickerSetColorChangedProcProcInfo = 0x00000FF0 }; 	/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes) */
enum { uppPickerExtractHelpItemProcInfo = 0x00003AF0 }; 		/* pascal 4_bytes Func(4_bytes, 2_bytes, 2_bytes, 4_bytes) */
#define NewPickerOpenProc(userRoutine) 							(PickerOpenUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerOpenProcInfo, GetCurrentArchitecture())
#define NewPickerCloseProc(userRoutine) 						(PickerCloseUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerCloseProcInfo, GetCurrentArchitecture())
#define NewPickerCanDoProc(userRoutine) 						(PickerCanDoUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerCanDoProcInfo, GetCurrentArchitecture())
#define NewPickerVersionProc(userRoutine) 						(PickerVersionUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerVersionProcInfo, GetCurrentArchitecture())
#define NewPickerRegisterProc(userRoutine) 						(PickerRegisterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerRegisterProcInfo, GetCurrentArchitecture())
#define NewPickerSetTargetProc(userRoutine) 					(PickerSetTargetUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetTargetProcInfo, GetCurrentArchitecture())
#define NewPickerInitProc(userRoutine) 							(PickerInitUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerInitProcInfo, GetCurrentArchitecture())
#define NewPickerTestGraphicsWorldProc(userRoutine) 			(PickerTestGraphicsWorldUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerTestGraphicsWorldProcInfo, GetCurrentArchitecture())
#define NewPickerGetDialogProc(userRoutine) 					(PickerGetDialogUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetDialogProcInfo, GetCurrentArchitecture())
#define NewPickerGetItemListProc(userRoutine) 					(PickerGetItemListUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetItemListProcInfo, GetCurrentArchitecture())
#define NewPickerGetColorProc(userRoutine) 						(PickerGetColorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetColorProcInfo, GetCurrentArchitecture())
#define NewPickerSetColorProc(userRoutine) 						(PickerSetColorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetColorProcInfo, GetCurrentArchitecture())
#define NewPickerEventProc(userRoutine) 						(PickerEventUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerEventProcInfo, GetCurrentArchitecture())
#define NewPickerEditProc(userRoutine) 							(PickerEditUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerEditProcInfo, GetCurrentArchitecture())
#define NewPickerSetVisibilityProc(userRoutine) 				(PickerSetVisibilityUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetVisibilityProcInfo, GetCurrentArchitecture())
#define NewPickerDisplayProc(userRoutine) 						(PickerDisplayUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerDisplayProcInfo, GetCurrentArchitecture())
#define NewPickerItemHitProc(userRoutine) 						(PickerItemHitUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerItemHitProcInfo, GetCurrentArchitecture())
#define NewPickerSetBaseItemProc(userRoutine) 					(PickerSetBaseItemUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetBaseItemProcInfo, GetCurrentArchitecture())
#define NewPickerGetProfileProc(userRoutine) 					(PickerGetProfileUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetProfileProcInfo, GetCurrentArchitecture())
#define NewPickerSetProfileProc(userRoutine) 					(PickerSetProfileUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetProfileProcInfo, GetCurrentArchitecture())
#define NewPickerGetPromptProc(userRoutine) 					(PickerGetPromptUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetPromptProcInfo, GetCurrentArchitecture())
#define NewPickerSetPromptProc(userRoutine) 					(PickerSetPromptUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetPromptProcInfo, GetCurrentArchitecture())
#define NewPickerGetIconDataProc(userRoutine) 					(PickerGetIconDataUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetIconDataProcInfo, GetCurrentArchitecture())
#define NewPickerGetEditMenuStateProc(userRoutine) 				(PickerGetEditMenuStateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerGetEditMenuStateProcInfo, GetCurrentArchitecture())
#define NewPickerSetOriginProc(userRoutine) 					(PickerSetOriginUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetOriginProcInfo, GetCurrentArchitecture())
#define NewPickerSetColorChangedProcProc(userRoutine) 			(PickerSetColorChangedProcUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerSetColorChangedProcProcInfo, GetCurrentArchitecture())
#define NewNPickerGetColorProc(userRoutine) 					(NPickerGetColorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNPickerGetColorProcInfo, GetCurrentArchitecture())
#define NewNPickerSetColorProc(userRoutine) 					(NPickerSetColorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNPickerSetColorProcInfo, GetCurrentArchitecture())
#define NewNPickerGetProfileProc(userRoutine) 					(NPickerGetProfileUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNPickerGetProfileProcInfo, GetCurrentArchitecture())
#define NewNPickerSetProfileProc(userRoutine) 					(NPickerSetProfileUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNPickerSetProfileProcInfo, GetCurrentArchitecture())
#define NewNPickerSetColorChangedProcProc(userRoutine) 			(NPickerSetColorChangedProcUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNPickerSetColorChangedProcProcInfo, GetCurrentArchitecture())
#define NewPickerExtractHelpItemProc(userRoutine) 				(PickerExtractHelpItemUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPickerExtractHelpItemProcInfo, GetCurrentArchitecture())
#define CallPickerOpenProc(userRoutine, storage, self) 			CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerOpenProcInfo, (storage), (self))
#define CallPickerCloseProc(userRoutine, storage, self) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerCloseProcInfo, (storage), (self))
#define CallPickerCanDoProc(userRoutine, storage, selector) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerCanDoProcInfo, (storage), (selector))
#define CallPickerVersionProc(userRoutine, storage) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppPickerVersionProcInfo, (storage))
#define CallPickerRegisterProc(userRoutine, storage) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppPickerRegisterProcInfo, (storage))
#define CallPickerSetTargetProc(userRoutine, storage, topOfCallChain)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerSetTargetProcInfo, (storage), (topOfCallChain))
#define CallPickerInitProc(userRoutine, storage, data) 			CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerInitProcInfo, (storage), (data))
#define CallPickerTestGraphicsWorldProc(userRoutine, storage, data)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerTestGraphicsWorldProcInfo, (storage), (data))
#define CallPickerGetDialogProc(userRoutine, storage) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppPickerGetDialogProcInfo, (storage))
#define CallPickerGetItemListProc(userRoutine, storage) 		CALL_ONE_PARAMETER_UPP((userRoutine), uppPickerGetItemListProcInfo, (storage))
#define CallPickerGetColorProc(userRoutine, storage, whichColor, color)  CALL_THREE_PARAMETER_UPP((userRoutine), uppPickerGetColorProcInfo, (storage), (whichColor), (color))
#define CallPickerSetColorProc(userRoutine, storage, whichColor, color)  CALL_THREE_PARAMETER_UPP((userRoutine), uppPickerSetColorProcInfo, (storage), (whichColor), (color))
#define CallPickerEventProc(userRoutine, storage, data) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerEventProcInfo, (storage), (data))
#define CallPickerEditProc(userRoutine, storage, data) 			CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerEditProcInfo, (storage), (data))
#define CallPickerSetVisibilityProc(userRoutine, storage, visible)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerSetVisibilityProcInfo, (storage), (visible))
#define CallPickerDisplayProc(userRoutine, storage) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppPickerDisplayProcInfo, (storage))
#define CallPickerItemHitProc(userRoutine, storage, data) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerItemHitProcInfo, (storage), (data))
#define CallPickerSetBaseItemProc(userRoutine, storage, baseItem)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerSetBaseItemProcInfo, (storage), (baseItem))
#define CallPickerGetProfileProc(userRoutine, storage) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppPickerGetProfileProcInfo, (storage))
#define CallPickerSetProfileProc(userRoutine, storage, profile)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerSetProfileProcInfo, (storage), (profile))
#define CallPickerGetPromptProc(userRoutine, storage, prompt) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerGetPromptProcInfo, (storage), (prompt))
#define CallPickerSetPromptProc(userRoutine, storage, prompt) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerSetPromptProcInfo, (storage), (prompt))
#define CallPickerGetIconDataProc(userRoutine, storage, data) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerGetIconDataProcInfo, (storage), (data))
#define CallPickerGetEditMenuStateProc(userRoutine, storage, mState)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerGetEditMenuStateProcInfo, (storage), (mState))
#define CallPickerSetOriginProc(userRoutine, storage, where) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppPickerSetOriginProcInfo, (storage), (where))
#define CallPickerSetColorChangedProcProc(userRoutine, storage, colorProc, colorProcData)  CALL_THREE_PARAMETER_UPP((userRoutine), uppPickerSetColorChangedProcProcInfo, (storage), (colorProc), (colorProcData))
#define CallNPickerGetColorProc(userRoutine, storage, whichColor, color)  CALL_THREE_PARAMETER_UPP((userRoutine), uppNPickerGetColorProcInfo, (storage), (whichColor), (color))
#define CallNPickerSetColorProc(userRoutine, storage, whichColor, color)  CALL_THREE_PARAMETER_UPP((userRoutine), uppNPickerSetColorProcInfo, (storage), (whichColor), (color))
#define CallNPickerGetProfileProc(userRoutine, storage, profile)  CALL_TWO_PARAMETER_UPP((userRoutine), uppNPickerGetProfileProcInfo, (storage), (profile))
#define CallNPickerSetProfileProc(userRoutine, storage, profile)  CALL_TWO_PARAMETER_UPP((userRoutine), uppNPickerSetProfileProcInfo, (storage), (profile))
#define CallNPickerSetColorChangedProcProc(userRoutine, storage, colorProc, colorProcData)  CALL_THREE_PARAMETER_UPP((userRoutine), uppNPickerSetColorChangedProcProcInfo, (storage), (colorProc), (colorProcData))
#define CallPickerExtractHelpItemProc(userRoutine, storage, itemNo, whichMsg, helpInfo)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppPickerExtractHelpItemProcInfo, (storage), (itemNo), (whichMsg), (helpInfo))

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

#endif /* __COLORPICKERCOMPONENTS__ */

