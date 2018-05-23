/*
 	File:		Displays.h
 
 	Contains:	Display Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1993-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __DISPLAYS__
#define __DISPLAYS__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __VIDEO__
#include <Video.h>
#endif

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
#ifndef __EVENTS__
#include <Events.h>
#endif
#ifndef __PROCESSES__
#include <Processes.h>
#endif
#ifndef __DIALOGS__
#include <Dialogs.h>
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

#ifndef FOR_GLUE_LIB
#define FOR_GLUE_LIB 0
#endif  /*  ! defined(FOR_GLUE_LIB)  */

#ifndef FOR_68kGLUE_LIB
#define FOR_68kGLUE_LIB 0
#endif  /*  ! defined(FOR_68kGLUE_LIB)  */


enum {
																/* AppleEvents Core Suite */
	kAESystemConfigNotice		= FOUR_CHAR_CODE('cnfg'),		/* Core Suite types */
	kAEDisplayNotice			= FOUR_CHAR_CODE('dspl'),
	kAEDisplaySummary			= FOUR_CHAR_CODE('dsum'),
	keyDMConfigVersion			= FOUR_CHAR_CODE('dmcv'),
	keyDMConfigFlags			= FOUR_CHAR_CODE('dmcf'),
	keyDMConfigReserved			= FOUR_CHAR_CODE('dmcr'),
	keyDisplayID				= FOUR_CHAR_CODE('dmid'),
	keyDisplayComponent			= FOUR_CHAR_CODE('dmdc'),
	keyDisplayDevice			= FOUR_CHAR_CODE('dmdd'),
	keyDisplayFlags				= FOUR_CHAR_CODE('dmdf'),
	keyDisplayMode				= FOUR_CHAR_CODE('dmdm'),
	keyDisplayModeReserved		= FOUR_CHAR_CODE('dmmr'),
	keyDisplayReserved			= FOUR_CHAR_CODE('dmdr'),
	keyDisplayMirroredId		= FOUR_CHAR_CODE('dmmi'),
	keyDeviceFlags				= FOUR_CHAR_CODE('dddf'),
	keyDeviceDepthMode			= FOUR_CHAR_CODE('dddm'),
	keyDeviceRect				= FOUR_CHAR_CODE('dddr'),
	keyPixMapRect				= FOUR_CHAR_CODE('dpdr'),
	keyPixMapHResolution		= FOUR_CHAR_CODE('dphr'),
	keyPixMapVResolution		= FOUR_CHAR_CODE('dpvr'),
	keyPixMapPixelType			= FOUR_CHAR_CODE('dppt'),
	keyPixMapPixelSize			= FOUR_CHAR_CODE('dpps'),
	keyPixMapCmpCount			= FOUR_CHAR_CODE('dpcc'),
	keyPixMapCmpSize			= FOUR_CHAR_CODE('dpcs'),
	keyPixMapAlignment			= FOUR_CHAR_CODE('dppa'),
	keyPixMapResReserved		= FOUR_CHAR_CODE('dprr'),
	keyPixMapReserved			= FOUR_CHAR_CODE('dppr'),
	keyPixMapColorTableSeed		= FOUR_CHAR_CODE('dpct'),
	keySummaryMenubar			= FOUR_CHAR_CODE('dsmb'),
	keySummaryChanges			= FOUR_CHAR_CODE('dsch'),
	keyDisplayOldConfig			= FOUR_CHAR_CODE('dold'),
	keyDisplayNewConfig			= FOUR_CHAR_CODE('dnew')
};


enum {
	dmOnlyActiveDisplays		= true,
	dmAllDisplays				= false
};



enum {
																/* DMSendDependentNotification notifyClass */
	kDependentNotifyClassShowCursor = FOUR_CHAR_CODE('shcr'),	/* When display mgr shows a hidden cursor during an unmirror */
	kDependentNotifyClassDriverOverride = FOUR_CHAR_CODE('ndrv'), /* When a driver is overridden */
	kDependentNotifyClassDisplayMgrOverride = FOUR_CHAR_CODE('dmgr'), /* When display manager is upgraded */
	kDependentNotifyClassProfileChanged = FOUR_CHAR_CODE('prof') /* When DMSetProfileByAVID is called */
};



enum {
																/* Switch Flags */
	kNoSwitchConfirmBit			= 0,							/* Flag indicating that there is no need to confirm a switch to this mode */
	kDepthNotAvailableBit		= 1,							/* Current depth not available in new mode */
	kShowModeBit				= 3,							/* Show this mode even though it requires a confirm. */
	kModeNotResizeBit			= 4,							/* Do not use this mode to resize display (for cards that mode drives a different connector). */
	kNeverShowModeBit			= 5								/* This mode should not be shown in the user interface. */
};

/*	Summary Change Flags (sticky bits indicating an operation was performed)
	For example, moving a display then moving it back will still set the kMovedDisplayBit.
*/

enum {
	kBeginEndConfigureBit		= 0,
	kMovedDisplayBit			= 1,
	kSetMainDisplayBit			= 2,
	kSetDisplayModeBit			= 3,
	kAddDisplayBit				= 4,
	kRemoveDisplayBit			= 5,
	kNewDisplayBit				= 6,
	kDisposeDisplayBit			= 7,
	kEnabledDisplayBit			= 8,
	kDisabledDisplayBit			= 9,
	kMirrorDisplayBit			= 10,
	kUnMirrorDisplayBit			= 11
};



enum {
																/* Notification Messages for extended call back routines */
	kDMNotifyInstalled			= 1,							/* At install time */
	kDMNotifyEvent				= 2,							/* Post change time */
	kDMNotifyRemoved			= 3,							/* At remove time */
	kDMNotifyPrep				= 4,							/* Pre change time */
	kDMNotifyExtendEvent		= 5,							/* Allow registrees to extend apple event before it is sent */
	kDMNotifyDependents			= 6,							/* Minor notification check without full update */
	kDMNotifySuspendConfigure	= 7,							/* Temporary end of configuration */
	kDMNotifyResumeConfigure	= 8,							/* Resume configuration */
	kDMNotifyRequestDisplayProbe = 9,							/* Request smart displays re-probe (used in sleep and hot plugging) */
																/* Notification Flags */
	kExtendedNotificationProc	= (1L << 16)
};


/* types for notifyType */

enum {
	kFullNotify					= 0,							/* This is the appleevent whole nine yards notify */
	kFullDependencyNotify		= 1								/* Only sends to those who want to know about interrelated functionality (used for updating UI) */
};

/* DisplayID/DeviceID constants */

enum {
	kDummyDeviceID				= 0x00FF,						/* This is the ID of the dummy display, used when the last ÒrealÓ display is disabled.*/
	kInvalidDisplayID			= 0x0000,						/* This is the invalid ID*/
	kFirstDisplayID				= 0x0100
};


enum {
																/* bits for panelListFlags */
	kAllowDuplicatesBit			= 0
};


enum {
																/* bits for nameFlags */
	kSuppressNumberBit			= 0,
	kSuppressNumberMask			= 1,
	kForceNumberBit				= 1,
	kForceNumberMask			= 2,
	kSuppressNameBit			= 2,
	kSuppressNameMask			= 4
};



/* Constants for fidelity checks */

enum {
	kNoFidelity					= 0,
	kMinimumFidelity			= 1,
	kDefaultFidelity			= 500,							/* I'm just picking a number for Apple default panels and engines*/
	kDefaultManufacturerFidelity = 1000							/* I'm just picking a number for Manufacturer's panels and engines (overrides apple defaults)*/
};


enum {
	kAnyPanelType				= 0,							/* Pass to DMNewEngineList for list of all panels (as opposed to specific types)*/
	kAnyEngineType				= 0,							/* Pass to DMNewEngineList for list of all engines*/
	kAnyDeviceType				= 0,							/* Pass to DMNewDeviceList for list of all devices*/
	kAnyPortType				= 0								/* Pass to DMNewDevicePortList for list of all devices*/
};

/* portListFlags for DM_NewDevicePortList */

enum {
																/* Should offline devices be put into the port list (such as dummy display) */
	kPLIncludeOfflineDevicesBit	= 0
};


/* confirmFlags for DMConfirmConfiguration */

enum {
	kForceConfirmBit			= 0,							/* Force a confirm dialog */
	kForceConfirmMask			= (1 << kForceConfirmBit)
};


/* Flags for displayModeFlags */

enum {
	kDisplayModeListNotPreferredBit = 0,
	kDisplayModeListNotPreferredMask = (1 << kDisplayModeListNotPreferredBit)
};


/* Flags for itemFlags */

enum {
	kComponentListNotPreferredBit = 0,
	kComponentListNotPreferredMask = (1 << kComponentListNotPreferredBit)
};


enum {
	kDisplayTimingInfoVersionZero = 1,
	kDisplayTimingInfoReservedCountVersionZero = 16,
	kDisplayModeEntryVersionZero = 0,							/* displayModeVersion - original version*/
	kDisplayModeEntryVersionOne	= 1								/* displayModeVersion - added displayModeOverrideInfo*/
};



typedef unsigned long 					DMFidelityType;
/*
   AVID is an ID for ports and devices the old DisplayID type
  	is carried on for compatibility
*/

typedef unsigned long 					AVIDType;
typedef AVIDType 						DisplayIDType;
typedef void *							DMListType;
typedef unsigned long 					DMListIndexType;
typedef VDPowerStateRec 				AVPowerStateRec;
typedef VDPowerStateRec *				AVPowerStatePtr;

struct DMDisplayTimingInfoRec {
	UInt32 							timingInfoVersion;
	UInt32 							timingInfoAttributes;		/* Flags */
	SInt32 							timingInfoRelativeQuality;	/* quality of the timing */
	SInt32 							timingInfoRelativeDefault;	/* relative default of the timing */

	UInt32 							timingInfoReserved[16];		/* Reserved */
};
typedef struct DMDisplayTimingInfoRec	DMDisplayTimingInfoRec;

typedef DMDisplayTimingInfoRec *		DMDisplayTimingInfoPtr;


struct DMComponentListEntryRec {
	DisplayIDType 					itemID;						/* DisplayID Manager*/
	Component 						itemComponent;				/* Component Manager*/
	ComponentDescription 			itemDescription;			/* We can always construct this if we use something beyond the compontent mgr.*/

	ResType 						itemClass;					/* Class of group to put this panel (eg geometry/color/etc for panels, brightness/contrast for engines, video out/sound/etc for devices)*/
	DMFidelityType 					itemFidelity;				/* How good is this item for the specified search?*/
	ResType 						itemSubClass;				/* Subclass of group to put this panel.  Can use to do sub-grouping (eg volume for volume panel and mute panel)*/
	Point 							itemSort;					/* Set to 0 - future to sort the items in a sub group.*/

	unsigned long 					itemFlags;					/* Set to 0 (future expansion)*/
	ResType 						itemReserved;				/* What kind of code does the itemReference point to  (right now - kPanelEntryTypeComponentMgr only)*/
	unsigned long 					itemFuture1;				/* Set to 0 (future expansion - probably an alternate code style)*/
	unsigned long 					itemFuture2;				/* Set to 0 (future expansion - probably an alternate code style)*/
	unsigned long 					itemFuture3;				/* Set to 0 (future expansion - probably an alternate code style)*/
	unsigned long 					itemFuture4;				/* Set to 0 (future expansion - probably an alternate code style)*/
};
typedef struct DMComponentListEntryRec	DMComponentListEntryRec;

typedef DMComponentListEntryRec *		DMComponentListEntryPtr;
/* ¥¥¥ Move AVLocationRec to AVComponents.i AFTER AVComponents.i is created*/

struct AVLocationRec {
	unsigned long 					locationConstant;			/* Set to 0 (future expansion - probably an alternate code style)*/
};
typedef struct AVLocationRec			AVLocationRec;

typedef AVLocationRec *					AVLocationPtr;

struct DMDepthInfoRec {
	VDSwitchInfoPtr 				depthSwitchInfo;			/* This is the switch mode to choose this timing/depth */
	VPBlockPtr 						depthVPBlock;				/* VPBlock (including size, depth and format) */
	UInt32 							depthFlags;					/* VDVideoParametersInfoRec.csDepthFlags  */
	UInt32 							depthReserved1;				/* Reserved */
	UInt32 							depthReserved2;				/* Reserved */
};
typedef struct DMDepthInfoRec			DMDepthInfoRec;

typedef DMDepthInfoRec *				DMDepthInfoPtr;

struct DMDepthInfoBlockRec {
	unsigned long 					depthBlockCount;			/* How many depths are there? */
	DMDepthInfoPtr 					depthVPBlock;				/* Array of DMDepthInfoRec */
	unsigned long 					depthBlockFlags;			/* Reserved */
	unsigned long 					depthBlockReserved1;		/* Reserved */
	unsigned long 					depthBlockReserved2;		/* Reserved */
};
typedef struct DMDepthInfoBlockRec		DMDepthInfoBlockRec;

typedef DMDepthInfoBlockRec *			DMDepthInfoBlockPtr;

struct DMDisplayModeListEntryRec {
	UInt32 							displayModeFlags;
	VDSwitchInfoPtr 				displayModeSwitchInfo;
	VDResolutionInfoPtr 			displayModeResolutionInfo;
	VDTimingInfoPtr 				displayModeTimingInfo;
	DMDepthInfoBlockPtr 			displayModeDepthBlockInfo;	/* Information about all the depths*/
	UInt32 							displayModeVersion;			/* What version is this record (now kDisplayModeEntryVersionOne)*/
	StringPtr 						displayModeName;			/* Name of the timing mode*/
	DMDisplayTimingInfoPtr 			displayModeDisplayInfo;		/* Information from the display.*/
};
typedef struct DMDisplayModeListEntryRec DMDisplayModeListEntryRec;

typedef DMDisplayModeListEntryRec *		DMDisplayModeListEntryPtr;


struct DependentNotifyRec {
	ResType 						notifyType;					/* What type was the engine that made the change (may be zero)*/
	ResType 						notifyClass;				/* What class was the change (eg geometry, color etc)*/
	DisplayIDType 					notifyPortID;				/* Which device was touched (kInvalidDisplayID -> all or none)*/
	ComponentInstance 				notifyComponent;			/* What engine did it (may be 0)?*/

	unsigned long 					notifyVersion;				/* Set to 0 (future expansion)*/
	unsigned long 					notifyFlags;				/* Set to 0 (future expansion)*/
	unsigned long 					notifyReserved;				/* Set to 0 (future expansion)*/
	unsigned long 					notifyFuture;				/* Set to 0 (future expansion)*/
};
typedef struct DependentNotifyRec		DependentNotifyRec;

typedef DependentNotifyRec *			DependentNotifyPtr;
/* Exports to support Interfaces library containing unused calls */
#if !FOR_GLUE_LIB
#endif  /*  !FOR_GLUE_LIB */

typedef CALLBACK_API( void , DMNotificationProcPtr )(AppleEvent *theEvent);
typedef CALLBACK_API( void , DMExtendedNotificationProcPtr )(void *userData, short theMessage, void *notifyData);
typedef CALLBACK_API( void , DMComponentListIteratorProcPtr )(void *userData, DMListIndexType itemIndex, DMComponentListEntryPtr componentInfo);
typedef CALLBACK_API( void , DMDisplayModeListIteratorProcPtr )(void *userData, DMListIndexType itemIndex, DMDisplayModeListEntryPtr displaymodeInfo);
typedef STACK_UPP_TYPE(DMNotificationProcPtr) 					DMNotificationUPP;
typedef STACK_UPP_TYPE(DMExtendedNotificationProcPtr) 			DMExtendedNotificationUPP;
typedef STACK_UPP_TYPE(DMComponentListIteratorProcPtr) 			DMComponentListIteratorUPP;
typedef STACK_UPP_TYPE(DMDisplayModeListIteratorProcPtr) 		DMDisplayModeListIteratorUPP;
enum { uppDMNotificationProcInfo = 0x000000C0 }; 				/* pascal no_return_value Func(4_bytes) */
enum { uppDMExtendedNotificationProcInfo = 0x00000EC0 }; 		/* pascal no_return_value Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppDMComponentListIteratorProcInfo = 0x00000FC0 }; 		/* pascal no_return_value Func(4_bytes, 4_bytes, 4_bytes) */
enum { uppDMDisplayModeListIteratorProcInfo = 0x00000FC0 }; 	/* pascal no_return_value Func(4_bytes, 4_bytes, 4_bytes) */
#define NewDMNotificationProc(userRoutine) 						(DMNotificationUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDMNotificationProcInfo, GetCurrentArchitecture())
#define NewDMExtendedNotificationProc(userRoutine) 				(DMExtendedNotificationUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDMExtendedNotificationProcInfo, GetCurrentArchitecture())
#define NewDMComponentListIteratorProc(userRoutine) 			(DMComponentListIteratorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDMComponentListIteratorProcInfo, GetCurrentArchitecture())
#define NewDMDisplayModeListIteratorProc(userRoutine) 			(DMDisplayModeListIteratorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDMDisplayModeListIteratorProcInfo, GetCurrentArchitecture())
#define CallDMNotificationProc(userRoutine, theEvent) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppDMNotificationProcInfo, (theEvent))
#define CallDMExtendedNotificationProc(userRoutine, userData, theMessage, notifyData)  CALL_THREE_PARAMETER_UPP((userRoutine), uppDMExtendedNotificationProcInfo, (userData), (theMessage), (notifyData))
#define CallDMComponentListIteratorProc(userRoutine, userData, itemIndex, componentInfo)  CALL_THREE_PARAMETER_UPP((userRoutine), uppDMComponentListIteratorProcInfo, (userData), (itemIndex), (componentInfo))
#define CallDMDisplayModeListIteratorProc(userRoutine, userData, itemIndex, displaymodeInfo)  CALL_THREE_PARAMETER_UPP((userRoutine), uppDMDisplayModeListIteratorProcInfo, (userData), (itemIndex), (displaymodeInfo))

/* Trap interfaces */

#if !FOR_GLUE_LIB
EXTERN_API( GDHandle )
DMGetFirstScreenDevice			(Boolean 				activeOnly)							TWOWORDINLINE(0x7000, 0xABEB);

EXTERN_API( GDHandle )
DMGetNextScreenDevice			(GDHandle 				theDevice,
								 Boolean 				activeOnly)							TWOWORDINLINE(0x7001, 0xABEB);

EXTERN_API( void )
DMDrawDesktopRect				(Rect *					globalRect)							TWOWORDINLINE(0x7002, 0xABEB);

EXTERN_API( void )
DMDrawDesktopRegion				(RgnHandle 				globalRgn)							TWOWORDINLINE(0x7003, 0xABEB);



EXTERN_API( OSErr )
DMBeginConfigureDisplays		(Handle *				displayState)						THREEWORDINLINE(0x303C, 0x0206, 0xABEB);

EXTERN_API( OSErr )
DMEndConfigureDisplays			(Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0207, 0xABEB);

EXTERN_API( OSErr )
DMAddDisplay					(GDHandle 				newDevice,
								 short 					driver,
								 unsigned long 			mode,
								 unsigned long 			reserved,
								 unsigned long 			displayID,
								 Component 				displayComponent,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0D08, 0xABEB);

EXTERN_API( OSErr )
DMMoveDisplay					(GDHandle 				moveDevice,
								 short 					x,
								 short 					y,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0609, 0xABEB);

EXTERN_API( OSErr )
DMDisableDisplay				(GDHandle 				disableDevice,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x040A, 0xABEB);

EXTERN_API( OSErr )
DMEnableDisplay					(GDHandle 				enableDevice,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x040B, 0xABEB);

EXTERN_API( OSErr )
DMRemoveDisplay					(GDHandle 				removeDevice,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x040C, 0xABEB);




EXTERN_API( OSErr )
DMSetMainDisplay				(GDHandle 				newMainDevice,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0410, 0xABEB);

EXTERN_API( OSErr )
DMSetDisplayMode				(GDHandle 				theDevice,
								 unsigned long 			mode,
								 unsigned long *		depthMode,
								 unsigned long 			reserved,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0A11, 0xABEB);

EXTERN_API( OSErr )
DMCheckDisplayMode				(GDHandle 				theDevice,
								 unsigned long 			mode,
								 unsigned long 			depthMode,
								 unsigned long *		switchFlags,
								 unsigned long 			reserved,
								 Boolean *				modeOk)								THREEWORDINLINE(0x303C, 0x0C12, 0xABEB);

EXTERN_API( OSErr )
DMGetDeskRegion					(RgnHandle *			desktopRegion)						THREEWORDINLINE(0x303C, 0x0213, 0xABEB);

EXTERN_API( OSErr )
DMRegisterNotifyProc			(DMNotificationUPP 		notificationProc,
								 ProcessSerialNumberPtr  whichPSN)							THREEWORDINLINE(0x303C, 0x0414, 0xABEB);

EXTERN_API( OSErr )
DMRemoveNotifyProc				(DMNotificationUPP 		notificationProc,
								 ProcessSerialNumberPtr  whichPSN)							THREEWORDINLINE(0x303C, 0x0415, 0xABEB);


EXTERN_API( OSErr )
DMQDIsMirroringCapable			(Boolean *				qdIsMirroringCapable)				THREEWORDINLINE(0x303C, 0x0216, 0xABEB);

EXTERN_API( OSErr )
DMCanMirrorNow					(Boolean *				canMirrorNow)						THREEWORDINLINE(0x303C, 0x0217, 0xABEB);

EXTERN_API( OSErr )
DMIsMirroringOn					(Boolean *				isMirroringOn)						THREEWORDINLINE(0x303C, 0x0218, 0xABEB);

EXTERN_API( OSErr )
DMMirrorDevices					(GDHandle 				gD1,
								 GDHandle 				gD2,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0619, 0xABEB);

EXTERN_API( OSErr )
DMUnmirrorDevice				(GDHandle 				gDevice,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x041A, 0xABEB);

EXTERN_API( OSErr )
DMGetNextMirroredDevice			(GDHandle 				gDevice,
								 GDHandle *				mirroredDevice)						THREEWORDINLINE(0x303C, 0x041B, 0xABEB);

EXTERN_API( OSErr )
DMBlockMirroring				(void)														TWOWORDINLINE(0x701C, 0xABEB);

EXTERN_API( OSErr )
DMUnblockMirroring				(void)														TWOWORDINLINE(0x701D, 0xABEB);

EXTERN_API( OSErr )
DMGetDisplayMgrA5World			(Ptr *					dmA5)								THREEWORDINLINE(0x303C, 0x021E, 0xABEB);

EXTERN_API( OSErr )
DMGetDisplayIDByGDevice			(GDHandle 				displayDevice,
								 DisplayIDType *		displayID,
								 Boolean 				failToMain)							THREEWORDINLINE(0x303C, 0x051F, 0xABEB);

EXTERN_API( OSErr )
DMGetGDeviceByDisplayID			(DisplayIDType 			displayID,
								 GDHandle *				displayDevice,
								 Boolean 				failToMain)							THREEWORDINLINE(0x303C, 0x0520, 0xABEB);

EXTERN_API( OSErr )
DMSetDisplayComponent			(GDHandle 				theDevice,
								 Component 				displayComponent)					THREEWORDINLINE(0x303C, 0x0421, 0xABEB);

EXTERN_API( OSErr )
DMGetDisplayComponent			(GDHandle 				theDevice,
								 Component *			displayComponent)					THREEWORDINLINE(0x303C, 0x0422, 0xABEB);

EXTERN_API( OSErr )
DMNewDisplay					(GDHandle *				newDevice,
								 short 					driverRefNum,
								 unsigned long 			mode,
								 unsigned long 			reserved,
								 DisplayIDType 			displayID,
								 Component 				displayComponent,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0D23, 0xABEB);

EXTERN_API( OSErr )
DMDisposeDisplay				(GDHandle 				disposeDevice,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x0424, 0xABEB);

EXTERN_API( OSErr )
DMResolveDisplayComponents		(void)														TWOWORDINLINE(0x7025, 0xABEB);

#endif  /*  !FOR_GLUE_LIB */

#if !FOR_68kGLUE_LIB
EXTERN_API( OSErr )
DMRegisterExtendedNotifyProc	(DMExtendedNotificationUPP  notifyProc,
								 void *					notifyUserData,
								 unsigned short 		nofifyOnFlags,
								 ProcessSerialNumberPtr  whichPSN)							THREEWORDINLINE(0x303C, 0x07EF, 0xABEB);

EXTERN_API( OSErr )
DMRemoveExtendedNotifyProc		(DMExtendedNotificationUPP  notifyProc,
								 void *					notifyUserData,
								 ProcessSerialNumberPtr  whichPSN,
								 unsigned short 		removeFlags)						THREEWORDINLINE(0x303C, 0x0726, 0xABEB);

#endif  /*  !FOR_68kGLUE_LIB */

EXTERN_API( OSErr )
DMNewAVPanelList				(DisplayIDType 			displayID,
								 ResType 				panelType,
								 DMFidelityType 		minimumFidelity,
								 unsigned long 			panelListFlags,
								 unsigned long 			reserved,
								 DMListIndexType *		thePanelCount,
								 DMListType *			thePanelList)						THREEWORDINLINE(0x303C, 0x0C27, 0xABEB);

EXTERN_API( OSErr )
DMNewAVEngineList				(DisplayIDType 			displayID,
								 ResType 				engineType,
								 DMFidelityType 		minimumFidelity,
								 unsigned long 			engineListFlags,
								 unsigned long 			reserved,
								 DMListIndexType *		engineCount,
								 DMListType *			engineList)							THREEWORDINLINE(0x303C, 0x0C28, 0xABEB);

EXTERN_API( OSErr )
DMNewAVDeviceList				(ResType 				deviceType,
								 unsigned long 			deviceListFlags,
								 unsigned long 			reserved,
								 DMListIndexType *		deviceCount,
								 DMListType *			deviceList)							THREEWORDINLINE(0x303C, 0x0A29, 0xABEB);

EXTERN_API( OSErr )
DMNewAVPortListByPortType		(ResType 				subType,
								 unsigned long 			portListFlags,
								 unsigned long 			reserved,
								 DMListIndexType *		devicePortCount,
								 DMListType *			theDevicePortList)					THREEWORDINLINE(0x303C, 0x0A2A, 0xABEB);

EXTERN_API( OSErr )
DMGetIndexedComponentFromList	(DMListType 			panelList,
								 DMListIndexType 		itemIndex,
								 unsigned long 			reserved,
								 DMComponentListIteratorUPP  listIterator,
								 void *					userData)							THREEWORDINLINE(0x303C, 0x0A2B, 0xABEB);

#if !FOR_68kGLUE_LIB
EXTERN_API( OSErr )
DMDisposeList					(DMListType 			panelList)							THREEWORDINLINE(0x303C, 0x022C, 0xABEB);

#endif  /*  !FOR_68kGLUE_LIB */

EXTERN_API( OSErr )
DMGetNameByAVID					(AVIDType 				theID,
								 unsigned long 			nameFlags,
								 Str255 				name)								THREEWORDINLINE(0x303C, 0x062D, 0xABEB);

EXTERN_API( OSErr )
DMNewAVIDByPortComponent		(Component 				thePortComponent,
								 ResType 				portKind,
								 unsigned long 			reserved,
								 AVIDType *				newID)								THREEWORDINLINE(0x303C, 0x082E, 0xABEB);

EXTERN_API( OSErr )
DMGetPortComponentByAVID		(DisplayIDType 			thePortID,
								 Component *			thePortComponent,
								 ComponentDescription *	theDesciption,
								 ResType *				thePortKind)						THREEWORDINLINE(0x303C, 0x082F, 0xABEB);

#if !FOR_68kGLUE_LIB
EXTERN_API( OSErr )
DMSendDependentNotification		(ResType 				notifyType,
								 ResType 				notifyClass,
								 AVIDType 				displayID,
								 ComponentInstance 		notifyComponent)					THREEWORDINLINE(0x303C, 0x0830, 0xABEB);

#endif  /*  !FOR_68kGLUE_LIB */

EXTERN_API( OSErr )
DMDisposeAVComponent			(Component 				theAVComponent)						THREEWORDINLINE(0x303C, 0x0231, 0xABEB);

EXTERN_API( OSErr )
DMSaveScreenPrefs				(unsigned long 			reserved1,
								 unsigned long 			saveFlags,
								 unsigned long 			reserved2)							THREEWORDINLINE(0x303C, 0x0632, 0xABEB);

EXTERN_API( OSErr )
DMNewAVIDByDeviceComponent		(Component 				theDeviceComponent,
								 ResType 				portKind,
								 unsigned long 			reserved,
								 DisplayIDType *		newID)								THREEWORDINLINE(0x303C, 0x0833, 0xABEB);

EXTERN_API( OSErr )
DMNewAVPortListByDeviceAVID		(AVIDType 				theID,
								 DMFidelityType 		minimumFidelity,
								 unsigned long 			portListFlags,
								 unsigned long 			reserved,
								 DMListIndexType *		devicePortCount,
								 DMListType *			theDevicePortList)					THREEWORDINLINE(0x303C, 0x0C34, 0xABEB);

EXTERN_API( OSErr )
DMGetDeviceComponentByAVID		(AVIDType 				theDeviceID,
								 Component *			theDeviceComponent,
								 ComponentDescription *	theDesciption,
								 ResType *				theDeviceKind)						THREEWORDINLINE(0x303C, 0x0835, 0xABEB);

EXTERN_API( OSErr )
DMNewDisplayModeList			(DisplayIDType 			displayID,
								 unsigned long 			modeListFlags,
								 unsigned long 			reserved,
								 DMListIndexType *		thePanelCount,
								 DMListType *			thePanelList)						THREEWORDINLINE(0x303C, 0x0A36, 0xABEB);

EXTERN_API( OSErr )
DMGetIndexedDisplayModeFromList	(DMListType 			panelList,
								 DMListIndexType 		itemIndex,
								 unsigned long 			reserved,
								 DMDisplayModeListIteratorUPP  listIterator,
								 void *					userData)							THREEWORDINLINE(0x303C, 0x0A37, 0xABEB);

EXTERN_API( OSErr )
DMGetGraphicInfoByAVID			(AVIDType 				theID,
								 PicHandle *			theAVPcit,
								 Handle *				theAVIconSuite,
								 AVLocationRec *		theAVLocation)						THREEWORDINLINE(0x303C, 0x0838, 0xABEB);

EXTERN_API( OSErr )
DMGetAVPowerState				(AVIDType 				theID,
								 AVPowerStatePtr 		getPowerState,
								 unsigned long 			reserved1)							THREEWORDINLINE(0x303C, 0x0839, 0xABEB);

EXTERN_API( OSErr )
DMSetAVPowerState				(AVIDType 				theID,
								 AVPowerStatePtr 		setPowerState,
								 unsigned long 			powerFlags,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x083A, 0xABEB);

EXTERN_API( OSErr )
DMGetDeviceAVIDByPortAVID		(AVIDType 				portAVID,
								 AVIDType *				deviceAVID)							THREEWORDINLINE(0x303C, 0x043B, 0xABEB);

EXTERN_API( OSErr )
DMGetEnableByAVID				(AVIDType 				theAVID,
								 Boolean *				isAVIDEnabledNow,
								 Boolean *				canChangeEnableNow)					THREEWORDINLINE(0x303C, 0x063C, 0xABEB);

EXTERN_API( OSErr )
DMSetEnableByAVID				(AVIDType 				theAVID,
								 Boolean 				doEnable,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x053D, 0xABEB);

EXTERN_API( OSErr )
DMGetDisplayMode				(GDHandle 				theDevice,
								 VDSwitchInfoPtr 		switchInfo)							THREEWORDINLINE(0x303C, 0x043E, 0xABEB);

EXTERN_API( OSErr )
DMConfirmConfiguration			(ModalFilterUPP 		filterProc,
								 UInt32 				confirmFlags,
								 UInt32 				reserved,
								 Handle 				displayState)						THREEWORDINLINE(0x303C, 0x083F, 0xABEB);



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

#endif /* __DISPLAYS__ */

