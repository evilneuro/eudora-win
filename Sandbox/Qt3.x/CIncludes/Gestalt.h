/*
 	File:		Gestalt.h
 
 	Contains:	Gestalt Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1998 by Apple Computer, Inc.  All rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __GESTALT__
#define __GESTALT__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
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



typedef CALLBACK_API( OSErr , SelectorFunctionProcPtr )(OSType selector, long *response);
typedef STACK_UPP_TYPE(SelectorFunctionProcPtr) 				SelectorFunctionUPP;
enum { uppSelectorFunctionProcInfo = 0x000003E0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes) */
#define NewSelectorFunctionProc(userRoutine) 					(SelectorFunctionUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSelectorFunctionProcInfo, GetCurrentArchitecture())
#define CallSelectorFunctionProc(userRoutine, selector, response)  CALL_TWO_PARAMETER_UPP((userRoutine), uppSelectorFunctionProcInfo, (selector), (response))



/*
	Note: 	The Gestalt trap was not implemented until System 6.0.5.  If you want to call Gestalt
			while running on System 6.0 machines, then define USE_GESTALT_GLUE and link with
			Interface.o which contains glue to implement Gestalt on pre-System 6.0.5 machines.
*/
#ifdef USE_GESTALT_GLUE
EXTERN_API( OSErr )
Gestalt							(OSType 				selector,
								 long *					response);

#else
																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 Gestalt(__D0, __A1)
																							#endif
EXTERN_API( OSErr )
Gestalt							(OSType 				selector,
								 long *					response)							TWOWORDINLINE(0xA1AD, 0x2288);

#endif  /*  defined(USE_GESTALT_GLUE)  */

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 ReplaceGestalt(__D0, __A0, __A1)
																							#endif
EXTERN_API( OSErr )
ReplaceGestalt					(OSType 				selector,
								 SelectorFunctionUPP 	gestaltFunction,
								 SelectorFunctionUPP *	oldGestaltFunction)					FOURWORDINLINE(0x2F09, 0xA5AD, 0x225F, 0x2288);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 NewGestalt(__D0, __A0)
																							#endif
EXTERN_API( OSErr )
NewGestalt						(OSType 				selector,
								 SelectorFunctionUPP 	gestaltFunction)					ONEWORDINLINE(0xA3AD);

/*
  	These functions (and SetGestaltValue) are built into System 7.5,
  	but not on earlier systems
*/

EXTERN_API( OSErr )
NewGestaltValue					(OSType 				selector,
								 long 					newValue)							THREEWORDINLINE(0x303C, 0x0401, 0xABF1);

EXTERN_API( OSErr )
ReplaceGestaltValue				(OSType 				selector,
								 long 					replacementValue)					THREEWORDINLINE(0x303C, 0x0402, 0xABF1);

EXTERN_API( OSErr )
SetGestaltValue					(OSType 				selector,
								 long 					newValue)							THREEWORDINLINE(0x303C, 0x0404, 0xABF1);

EXTERN_API( OSErr )
DeleteGestaltValue				(OSType 				selector)							THREEWORDINLINE(0x303C, 0x0203, 0xABF1);



/* Environment Selectors */

enum {
	gestaltAddressingModeAttr	= FOUR_CHAR_CODE('addr'),		/* addressing mode attributes */
	gestalt32BitAddressing		= 0,							/* using 32-bit addressing mode */
	gestalt32BitSysZone			= 1,							/* 32-bit compatible system zone */
	gestalt32BitCapable			= 2								/* Machine is 32-bit capable */
};


enum {
	gestaltAFPClient			= FOUR_CHAR_CODE('afps'),
	gestaltAFPClientVersionMask	= 0x0000FFFF,					/* low word of long is the */
																/* client version 0x0001 -> 0x0007*/
	gestaltAFPClient3_5			= 0x0001,
	gestaltAFPClient3_6			= 0x0002,
	gestaltAFPClient3_6_1		= 0x0003,
	gestaltAFPClient3_6_2		= 0x0004,
	gestaltAFPClient3_6_3		= 0x0005,						/* including 3.6.4, 3.6.5*/
	gestaltAFPClient3_7			= 0x0006,						/* including 3.7.1*/
	gestaltAFPClient3_7_2		= 0x0007,						/* including 3.7.3*/
	gestaltAFPClientAttributeMask = (long)0xFFFF0000,			/* high word of long is a */
																/* set of attribute bits*/
	gestaltAFPClientCfgRsrc		= 16,							/* Client uses config resources*/
	gestaltAFPClientSupportsIP	= 29,							/* Client supports AFP over TCP/IP*/
	gestaltAFPClientVMUI		= 30,							/* Client can put up UI from the PBVolMount trap*/
	gestaltAFPClientMultiReq	= 31							/* Client supports multiple outstanding requests*/
};


enum {
	gestaltAliasMgrAttr			= FOUR_CHAR_CODE('alis'),		/* Alias Mgr Attributes */
	gestaltAliasMgrPresent		= 0,							/* True if the Alias Mgr is present */
	gestaltAliasMgrSupportsRemoteAppletalk = 1,					/* True if the Alias Mgr knows about Remote Appletalk */
	gestaltAliasMgrSupportsAOCEKeychain = 2,					/* True if the Alias Mgr knows about the AOCE Keychain */
	gestaltAliasMgrResolveAliasFileWithMountOptions = 3			/* True if the Alias Mgr implements gestaltAliasMgrResolveAliasFileWithMountOptions() and IsAliasFile() */
};


enum {
	gestaltArbitorAttr			= FOUR_CHAR_CODE('arb '),
	gestaltSerialArbitrationExists = 0							/* this bit if the serial port arbitrator exists*/
};


enum {
	gestaltAppleScriptVersion	= FOUR_CHAR_CODE('ascv')		/* AppleScript version*/
};


enum {
	gestaltAppleScriptAttr		= FOUR_CHAR_CODE('ascr'),		/* AppleScript attributes*/
	gestaltAppleScriptPresent	= 0,
	gestaltAppleScriptPowerPCSupport = 1
};


enum {
	gestaltATAAttr				= FOUR_CHAR_CODE('ata '),		/* ATA is the driver to support IDE hard disks */
	gestaltATAPresent			= 0								/* if set, ATA Manager is present */
};


enum {
	gestaltATalkVersion			= FOUR_CHAR_CODE('atkv')		/* Detailed AppleTalk version; see comment above for format */
};


enum {
	gestaltAppleTalkVersion		= FOUR_CHAR_CODE('atlk')		/* appletalk version */
};

/*
	FORMAT OF gestaltATalkVersion RESPONSE
	--------------------------------------
	The version is stored in the high three bytes of the response value.  Let us number
	the bytes in the response value from 0 to 3, where 0 is the least-significant byte.

		Byte#:	   3 2 1 0
		Value:	0xMMNNRR00

	Byte 3 (MM) contains the major revision number, byte 2 (NN) contains the minor
	revision number, and byte 1 (RR) contains a constant that represents the release
	stage.  Byte 0 always contains 0x00.  The constants for the release stages are:

		development = 0x20
		alpha		= 0x40
		beta		= 0x60
		final		= 0x80
		release		= 0x80

	For example, if you call Gestalt with the 'atkv' selector when AppleTalk version 57
	is loaded, you receive the long integer response value 0x39008000.
*/

enum {
	gestaltAUXVersion			= FOUR_CHAR_CODE('a/ux')		/* a/ux version, if present */
};


enum {
	gestaltBusClkSpeed			= FOUR_CHAR_CODE('bclk')		/* main I/O bus clock speed in hertz */
};


enum {
	gestaltCloseViewAttr		= FOUR_CHAR_CODE('BSDa'),		/* CloseView attributes */
	gestaltCloseViewEnabled		= 0,							/* Closeview enabled (dynamic bit - returns current state) */
	gestaltCloseViewDisplayMgrFriendly = 1						/* Closeview compatible with Display Manager (FUTURE) */
};


enum {
	gestaltCFMAttr				= FOUR_CHAR_CODE('cfrg'),		/* returns information about the Code Fragment Manager */
	gestaltCFMPresent			= 0								/* true if the Code Fragment Manager is present */
};


enum {
	gestaltCollectionMgrVersion	= FOUR_CHAR_CODE('cltn')		/* Collection Manager version */
};


enum {
	gestaltColorMatchingAttr	= FOUR_CHAR_CODE('cmta'),		/* ColorSync attributes */
	gestaltHighLevelMatching	= 0,
	gestaltColorMatchingLibLoaded = 1
};


enum {
	gestaltColorMatchingVersion	= FOUR_CHAR_CODE('cmtc'),
	gestaltColorSync10			= 0x0100,						/* 0x0100 & 0x0110 _Gestalt versions for 1.0-1.0.3 product */
	gestaltColorSync11			= 0x0110,						/*   0x0100 == low-level matching only */
	gestaltColorSync104			= 0x0104,						/* Real version, by popular demand */
	gestaltColorSync105			= 0x0105,
	gestaltColorSync20			= 0x0200,						/* ColorSync 2.0 */
	gestaltColorSync21			= 0x0210,
	gestaltColorSync211			= 0x0211,
	gestaltColorSync212			= 0x0212,
	gestaltColorSync213			= 0x0213,
	gestaltColorSync25			= 0x0250
};


enum {
	gestaltConnMgrAttr			= FOUR_CHAR_CODE('conn'),		/* connection mgr attributes    */
	gestaltConnMgrPresent		= 0,
	gestaltConnMgrCMSearchFix	= 1,							/* Fix to CMAddSearch?     */
	gestaltConnMgrErrorString	= 2,							/* has CMGetErrorString() */
	gestaltConnMgrMultiAsyncIO	= 3								/* CMNewIOPB, CMDisposeIOPB, CMPBRead, CMPBWrite, CMPBIOKill */
};


enum {
	gestaltColorPickerVersion	= FOUR_CHAR_CODE('cpkr'),		/* returns version of ColorPicker */
	gestaltColorPicker			= FOUR_CHAR_CODE('cpkr')		/* gestaltColorPicker is old name for gestaltColorPickerVersion */
};


enum {
	gestaltComponentMgr			= FOUR_CHAR_CODE('cpnt')		/* Component Mgr version */
};

/*
	The gestaltNativeCPUtype ('cput') selector can be used to determine the
	native CPU type for all Macs running System 7.5 or later.

	The 'cput' selector is not available when running System 7.0 (or earlier)
	on most 68K machines.  If 'cput' is not available, then the 'proc' selector
	should be used to determine the processor type.

	An application should always try the 'cput' selector first.  This is because,
	on PowerPC machines, the 'proc' selector will reflect the CPU type of the
	emulator's "virtual processor" rather than the native CPU type.

	The values specified below are accurate.  Prior versions of the Gestalt
	interface file contained values that were off by one.

	The Quadra 840AV and the Quadra 660AV contain a bug in the ROM code that
	causes the 'cput' selector to respond with the value 5.  This behavior
	occurs only when running System 7.1.  System 7.5 fixes the bug by replacing
	the faulty 'cput' selector function with the correct one.

	The gestaltNativeCPUfamily ('cpuf') selector can be used to determine the
	general family the native CPU is in. This can be helpful for determing how
	blitters and things should be written. In general, it is smarter to use this
	selector (when available) than gestaltNativeCPUtype since newer processors
	in the same family can be handled without revising your code.

	gestaltNativeCPUfamily uses the same results as gestaltNativeCPUtype, but
	will only return certain CPU values.
*/

enum {
	gestaltNativeCPUtype		= FOUR_CHAR_CODE('cput'),		/* Native CPU type 									  */
	gestaltNativeCPUfamily		= FOUR_CHAR_CODE('cpuf'),		/* Native CPU family								  */
	gestaltCPU68000				= 0,							/* Various 68k CPUs... 	*/
	gestaltCPU68010				= 1,
	gestaltCPU68020				= 2,
	gestaltCPU68030				= 3,
	gestaltCPU68040				= 4,
	gestaltCPU601				= 0x0101,						/* IBM 601 												*/
	gestaltCPU603				= 0x0103,
	gestaltCPU604				= 0x0104,
	gestaltCPU603e				= 0x0106,
	gestaltCPU603ev				= 0x0107,
	gestaltCPU750				= 0x0108,						/* Also 740 - "G3" */
	gestaltCPU604e				= 0x0109,
	gestaltCPU604ev				= 0x010A						/* Mach 5, 250Mhz and up */
};


enum {
	gestaltCRMAttr				= FOUR_CHAR_CODE('crm '),		/* comm resource mgr attributes */
	gestaltCRMPresent			= 0,
	gestaltCRMPersistentFix		= 1,							/* fix for persistent tools */
	gestaltCRMToolRsrcCalls		= 2								/* has CRMGetToolResource/ReleaseToolResource */
};


enum {
	gestaltControlStripVersion	= FOUR_CHAR_CODE('csvr')		/* Control Strip version (was 'sdvr') */
};


enum {
	gestaltCTBVersion			= FOUR_CHAR_CODE('ctbv')		/* CommToolbox version */
};


enum {
	gestaltDBAccessMgrAttr		= FOUR_CHAR_CODE('dbac'),		/* Database Access Mgr attributes */
	gestaltDBAccessMgrPresent	= 0								/* True if Database Access Mgr present */
};


enum {
	gestaltSDPFindVersion		= FOUR_CHAR_CODE('dfnd')		/* OCE Standard Directory Panel*/
};


enum {
	gestaltDictionaryMgrAttr	= FOUR_CHAR_CODE('dict'),		/* Dictionary Manager attributes */
	gestaltDictionaryMgrPresent	= 0								/* Dictionary Manager attributes */
};


enum {
	gestaltDITLExtAttr			= FOUR_CHAR_CODE('ditl'),		/* AppenDITL, etc. calls from CTB */
	gestaltDITLExtPresent		= 0,							/* True if calls are present */
	gestaltDITLExtSupportsIctb	= 1								/* True if AppendDITL, ShortenDITL support 'ictb's */
};


enum {
	gestaltDesktopPicturesAttr	= FOUR_CHAR_CODE('dkpx'),		/* Desktop Pictures attributes */
	gestaltDesktopPicturesInstalled = 0,						/* True if control panel is installed */
	gestaltDesktopPicturesDisplayed = 1							/* True if a picture is currently displayed */
};


enum {
	gestaltDisplayMgrVers		= FOUR_CHAR_CODE('dplv')		/* Display Manager version */
};


enum {
	gestaltDisplayMgrAttr		= FOUR_CHAR_CODE('dply'),		/* Display Manager attributes */
	gestaltDisplayMgrPresent	= 0,							/* True if Display Mgr is present */
	gestaltDisplayMgrCanSwitchMirrored = 2,						/* True if Display Mgr can switch modes on mirrored displays */
	gestaltDisplayMgrSetDepthNotifies = 3,						/* True SetDepth generates displays mgr notification */
	gestaltDisplayMgrCanConfirm	= 4								/* True Display Manager supports DMConfirmConfiguration */
};


enum {
	gestaltDragMgrAttr			= FOUR_CHAR_CODE('drag'),		/* Drag Manager attributes */
	gestaltDragMgrPresent		= 0,							/* Drag Manager is present */
	gestaltDragMgrFloatingWind	= 1,							/* Drag Manager supports floating windows */
	gestaltPPCDragLibPresent	= 2,							/* Drag Manager PPC DragLib is present */
	gestaltDragMgrHasImageSupport = 3,							/* Drag Manager allows SetDragImage call */
	gestaltCanStartDragInFloatWindow = 4						/* Drag Manager supports starting a drag in a floating window */
};


enum {
	gestaltDigitalSignatureVersion = FOUR_CHAR_CODE('dsig')		/* returns Digital Signature Toolbox version in low-order word*/
};


enum {
	gestaltEasyAccessAttr		= FOUR_CHAR_CODE('easy'),		/* Easy Access attributes */
	gestaltEasyAccessOff		= 0,							/* if Easy Access present, but off (no icon) */
	gestaltEasyAccessOn			= 1,							/* if Easy Access "On" */
	gestaltEasyAccessSticky		= 2,							/* if Easy Access "Sticky" */
	gestaltEasyAccessLocked		= 3								/* if Easy Access "Locked" */
};


enum {
	gestaltEditionMgrAttr		= FOUR_CHAR_CODE('edtn'),		/* Edition Mgr attributes */
	gestaltEditionMgrPresent	= 0,							/* True if Edition Mgr present */
	gestaltEditionMgrTranslationAware = 1						/* True if edition manager is translation manager aware */
};


enum {
	gestaltAppleEventsAttr		= FOUR_CHAR_CODE('evnt'),		/* Apple Events attributes */
	gestaltAppleEventsPresent	= 0,							/* True if Apple Events present */
	gestaltScriptingSupport		= 1,
	gestaltOSLInSystem			= 2								/* OSL is in system so donÕt use the one linked in to app */
};


enum {
	gestaltExtensionTableVersion = FOUR_CHAR_CODE('etbl')		/* ExtensionTable version */
};


enum {
	gestaltFloppyAttr			= FOUR_CHAR_CODE('flpy'),		/* Floppy disk drive/driver attributes */
	gestaltFloppyIsMFMOnly		= 0,							/* Floppy driver only supports MFM disk formats */
	gestaltFloppyIsManualEject	= 1,							/* Floppy drive, driver, and file system are in manual-eject mode */
	gestaltFloppyUsesDiskInPlace = 2							/* Floppy drive must have special DISK-IN-PLACE output; standard DISK-CHANGED not used */
};


enum {
	gestaltFinderAttr			= FOUR_CHAR_CODE('fndr'),		/* Finder attributes */
	gestaltFinderDropEvent		= 0,							/* Finder recognizes drop event */
	gestaltFinderMagicPlacement	= 1,							/* Finder supports magic icon placement */
	gestaltFinderCallsAEProcess	= 2,							/* Finder calls AEProcessAppleEvent */
	gestaltOSLCompliantFinder	= 3,							/* Finder is scriptable and recordable */
	gestaltFinderSupports4GBVolumes = 4,						/* Finder correctly handles 4GB volumes */
	gestaltFinderHasClippings	= 6,							/* Finder supports Drag Manager clipping files */
	gestaltFinderFullDragManagerSupport = 7,					/* Finder accepts 'hfs ' flavors properly */
	gestaltFinderFloppyRootComments = 8,						/* in MacOS 8 and later, will be set if Finder ever supports comments on Floppy icons */
	gestaltFinderLargeAndNotSavedFlavorsOK = 9					/* in MacOS 8 and later, drags with >1024-byte flavors and flavorNotSaved flavors work reliably */
};


enum {
	gestaltFindFolderAttr		= FOUR_CHAR_CODE('fold'),		/* Folder Mgr attributes */
	gestaltFindFolderPresent	= 0,							/* True if Folder Mgr present */
	gestaltFolderDescSupport	= 1								/* Tru if Folder Mgr has FolderDesc calls */
};



enum {
	gestaltFontMgrAttr			= FOUR_CHAR_CODE('font'),		/* Font Mgr attributes */
	gestaltOutlineFonts			= 0								/* True if Outline Fonts supported */
};


enum {
	gestaltFPUType				= FOUR_CHAR_CODE('fpu '),		/* fpu type */
	gestaltNoFPU				= 0,							/* no FPU */
	gestalt68881				= 1,							/* 68881 FPU */
	gestalt68882				= 2,							/* 68882 FPU */
	gestalt68040FPU				= 3								/* 68040 built-in FPU */
};


enum {
	gestaltFSAttr				= FOUR_CHAR_CODE('fs  '),		/* file system attributes */
	gestaltFullExtFSDispatching	= 0,							/* has really cool new HFSDispatch dispatcher */
	gestaltHasFSSpecCalls		= 1,							/* has FSSpec calls */
	gestaltHasFileSystemManager	= 2,							/* has a file system manager */
	gestaltFSMDoesDynamicLoad	= 3,							/* file system manager supports dynamic loading */
	gestaltFSSupports4GBVols	= 4,							/* file system supports 4 gigabyte volumes */
	gestaltFSSupports2TBVols	= 5,							/* file system supports 2 terabyte volumes */
	gestaltHasExtendedDiskInit	= 6,							/* has extended Disk Initialization calls */
	gestaltDTMgrSupportsFSM		= 7								/* Desktop Manager support FSM-based foreign file systems */
};


enum {
	gestaltFSMVersion			= FOUR_CHAR_CODE('fsm ')		/* returns version of HFS External File Systems Manager (FSM) */
};


enum {
	gestaltFXfrMgrAttr			= FOUR_CHAR_CODE('fxfr'),		/* file transfer manager attributes */
	gestaltFXfrMgrPresent		= 0,
	gestaltFXfrMgrMultiFile		= 1,							/* supports FTSend and FTReceive */
	gestaltFXfrMgrErrorString	= 2,							/* supports FTGetErrorString */
	gestaltFXfrMgrAsync			= 3								/*supports FTSendAsync, FTReceiveAsync, FTCompletionAsync*/
};


enum {
	gestaltGraphicsAttr			= FOUR_CHAR_CODE('gfxa'),		/* Quickdraw GX attributes selector */
	gestaltGraphicsIsDebugging	= 0x00000001,
	gestaltGraphicsIsLoaded		= 0x00000002,
	gestaltGraphicsIsPowerPC	= 0x00000004
};


enum {
	gestaltGraphicsVersion		= FOUR_CHAR_CODE('grfx'),		/* Quickdraw GX version selector */
	gestaltCurrentGraphicsVersion = 0x00010200					/* the version described in this set of headers */
};


enum {
	gestaltHardwareAttr			= FOUR_CHAR_CODE('hdwr'),		/* hardware attributes */
	gestaltHasVIA1				= 0,							/* VIA1 exists */
	gestaltHasVIA2				= 1,							/* VIA2 exists */
	gestaltHasASC				= 3,							/* Apple Sound Chip exists */
	gestaltHasSCC				= 4,							/* SCC exists */
	gestaltHasSCSI				= 7,							/* SCSI exists */
	gestaltHasSoftPowerOff		= 19,							/* Capable of software power off */
	gestaltHasSCSI961			= 21,							/* 53C96 SCSI controller on internal bus */
	gestaltHasSCSI962			= 22,							/* 53C96 SCSI controller on external bus */
	gestaltHasUniversalROM		= 24,							/* Do we have a Universal ROM? */
	gestaltHasEnhancedLtalk		= 30							/* Do we have Enhanced LocalTalk? */
};


enum {
	gestaltHelpMgrAttr			= FOUR_CHAR_CODE('help'),		/* Help Mgr Attributes */
	gestaltHelpMgrPresent		= 0,							/* true if help mgr is present */
	gestaltHelpMgrExtensions	= 1,							/* true if help mgr extensions are installed */
	gestaltAppleGuideIsDebug	= 30,
	gestaltAppleGuidePresent	= 31							/* true if AppleGuide is installed */
};


enum {
	gestaltHardwareVendorCode	= FOUR_CHAR_CODE('hrad'),		/* Returns hardware vendor information */
	gestaltHardwareVendorApple	= FOUR_CHAR_CODE('Appl')		/* Hardware built by Apple */
};


enum {
	gestaltCompressionMgr		= FOUR_CHAR_CODE('icmp')		/* returns version of the Image Compression Manager */
};


enum {
	gestaltIconUtilitiesAttr	= FOUR_CHAR_CODE('icon'),		/* Icon Utilities attributes  (Note: available in System 7.0, despite gestalt) */
	gestaltIconUtilitiesPresent	= 0								/* true if icon utilities are present */
};


enum {
	gestaltInternalDisplay		= FOUR_CHAR_CODE('idsp')		/* slot number of internal display location */
};

/*
	To obtain information about the connected keyboard(s), one should
	use the ADB Manager API.  See Technical Note OV16 for details.
*/

enum {
	gestaltKeyboardType			= FOUR_CHAR_CODE('kbd '),		/* keyboard type */
	gestaltMacKbd				= 1,
	gestaltMacAndPad			= 2,
	gestaltMacPlusKbd			= 3,
	gestaltExtADBKbd			= 4,
	gestaltStdADBKbd			= 5,
	gestaltPrtblADBKbd			= 6,
	gestaltPrtblISOKbd			= 7,
	gestaltStdISOADBKbd			= 8,
	gestaltExtISOADBKbd			= 9,
	gestaltADBKbdII				= 10,
	gestaltADBISOKbdII			= 11,
	gestaltPwrBookADBKbd		= 12,
	gestaltPwrBookISOADBKbd		= 13,
	gestaltAppleAdjustKeypad	= 14,
	gestaltAppleAdjustADBKbd	= 15,
	gestaltAppleAdjustISOKbd	= 16,
	gestaltJapanAdjustADBKbd	= 17,							/* Japan Adjustable Keyboard */
	gestaltPwrBkExtISOKbd		= 20,							/* PowerBook Extended International Keyboard with function keys */
	gestaltPwrBkExtJISKbd		= 21,							/* PowerBook Extended Japanese Keyboard with function keys 		*/
	gestaltPwrBkExtADBKbd		= 24,							/* PowerBook Extended Domestic Keyboard with function keys 		*/
	gestaltPS2Keyboard			= 27,							/* PS2 keyboard */
	gestaltPwrBkSubDomKbd		= 28,							/* PowerBook Subnote Domestic Keyboard with function keys w/  inverted T 	*/
	gestaltPwrBkSubISOKbd		= 29,							/* PowerBook Subnote International Keyboard with function keys w/  inverted T 	*/
	gestaltPwrBkSubJISKbd		= 30,							/* PowerBook Subnote Japanese Keyboard with function keys w/ inverted T 		*/
	gestaltPwrBkEKDomKbd		= 195,							/* (0xC3) PowerBook Domestic Keyboard with Embedded Keypad, function keys & inverted T 	*/
	gestaltPwrBkEKISOKbd		= 196,							/* (0xC4) PowerBook International Keyboard with Embedded Keypad, function keys & inverted T 	*/
	gestaltPwrBkEKJISKbd		= 197							/* (0xC5) PowerBook Japanese Keyboard with Embedded Keypad, function keys & inverted T 		*/
};


enum {
	gestaltLowMemorySize		= FOUR_CHAR_CODE('lmem')		/* size of low memory area */
};


enum {
	gestaltLogicalRAMSize		= FOUR_CHAR_CODE('lram')		/* logical ram size */
};

/*
	MACHINE TYPE CONSTANTS NAMING CONVENTION

		All future machine type constant names take the following form:

			gestalt<lineName><modelNumber>

	Line Names

		The following table contains the lines currently produced by Apple and the
		lineName substrings associated with them:

			Line						lineName
			-------------------------	------------
			Macintosh LC				"MacLC"
			Macintosh Performa			"Performa"
			Macintosh PowerBook			"PowerBook"
			Macintosh PowerBook Duo		"PowerBookDuo"
			Power Macintosh				"PowerMac"
			Apple Workgroup Server		"AWS"

		The following table contains lineNames for some discontinued lines:

			Line						lineName
			-------------------------	------------
			Macintosh Quadra			"MacQuadra" (preferred)
										"Quadra" (also used, but not preferred)
			Macintosh Centris			"MacCentris"

	Model Numbers

		The modelNumber is a string representing the specific model of the machine
		within its particular line.  For example, for the Power Macintosh 8100/80,
		the modelNumber is "8100".

		Some Performa & LC model numbers contain variations in the rightmost 1 or 2
		digits to indicate different RAM and Hard Disk configurations.  A single
		machine type is assigned for all variations of a specific model number.  In
		this case, the modelNumber string consists of the constant leftmost part
		of the model number with 0s for the variant digits.  For example, the
		Performa 6115 and Performa 6116 are both return the same machine type
		constant:  gestaltPerforma6100.


	OLD NAMING CONVENTIONS

	The "Underscore Speed" suffix

		In the past, Apple differentiated between machines that had the same model
		number but different speeds.  For example, the Power Macintosh 8100/80 and
		Power Macintosh 8100/100 return different machine type constants.  This is
		why some existing machine type constant names take the form:

			gestalt<lineName><modelNumber>_<speed>

		e.g.

			gestaltPowerMac8100_110
			gestaltPowerMac7100_80
			gestaltPowerMac7100_66

		It is no longer necessary to use the "underscore speed" suffix.  Starting with
		the Power Surge machines (Power Macintosh 7200, 7500, 8500 and 9500), speed is
		no longer used to differentiate between machine types.  This is why a Power
		Macintosh 7200/75 and a Power Macintosh 7200/90 return the same machine type
		constant:  gestaltPowerMac7200.

	The "Screen Type" suffix

		All PowerBook models prior to the PowerBook 190, and all PowerBook Duo models
		before the PowerBook Duo 2300 take the form:

			gestalt<lineName><modelNumber><screenType>

		Where <screenType> is "c" or the empty string.

		e.g.

			gestaltPowerBook100
			gestaltPowerBookDuo280
			gestaltPowerBookDuo280c
			gestaltPowerBook180
			gestaltPowerBook180c

		Starting with the PowerBook 190 series and the PowerBook Duo 2300 series, machine
		types are no longer differentiated based on screen type.  This is why a PowerBook
		5300cs/100 and a PowerBook 5300c/100 both return the same machine type constant:
		gestaltPowerBook5300.

		Macintosh LC 630				gestaltMacLC630
		Macintosh Performa 6200			gestaltPerforma6200
		Macintosh Quadra 700			gestaltQuadra700
		Macintosh PowerBook 5300		gestaltPowerBook5300
		Macintosh PowerBook Duo 2300	gestaltPowerBookDuo2300
		Power Macintosh 8500			gestaltPowerMac8500
*/


enum {
	gestaltMachineType			= FOUR_CHAR_CODE('mach'),		/* machine type */
	gestaltClassic				= 1,
	gestaltMacXL				= 2,
	gestaltMac512KE				= 3,
	gestaltMacPlus				= 4,
	gestaltMacSE				= 5,
	gestaltMacII				= 6,
	gestaltMacIIx				= 7,
	gestaltMacIIcx				= 8,
	gestaltMacSE030				= 9,
	gestaltPortable				= 10,
	gestaltMacIIci				= 11,
	gestaltPowerMac8100_120		= 12,
	gestaltMacIIfx				= 13,
	gestaltMacClassic			= 17,
	gestaltMacIIsi				= 18,
	gestaltMacLC				= 19,
	gestaltMacQuadra900			= 20,
	gestaltPowerBook170			= 21,
	gestaltMacQuadra700			= 22,
	gestaltClassicII			= 23,
	gestaltPowerBook100			= 24,
	gestaltPowerBook140			= 25,
	gestaltMacQuadra950			= 26,
	gestaltMacLCIII				= 27,
	gestaltPerforma450			= gestaltMacLCIII,
	gestaltPowerBookDuo210		= 29,
	gestaltMacCentris650		= 30,
	gestaltPowerBookDuo230		= 32,
	gestaltPowerBook180			= 33,
	gestaltPowerBook160			= 34,
	gestaltMacQuadra800			= 35,
	gestaltMacQuadra650			= 36,
	gestaltMacLCII				= 37,
	gestaltPowerBookDuo250		= 38,
	gestaltAWS9150_80			= 39,
	gestaltPowerMac8100_110		= 40,
	gestaltAWS8150_110			= gestaltPowerMac8100_110,
	gestaltPowerMac5200			= 41,
	gestaltPowerMac5260			= gestaltPowerMac5200,
	gestaltPerforma5300			= gestaltPowerMac5200,
	gestaltPowerMac6200			= 42,
	gestaltPerforma6300			= gestaltPowerMac6200,
	gestaltMacIIvi				= 44,
	gestaltMacIIvm				= 45,
	gestaltPerforma600			= gestaltMacIIvm,
	gestaltPowerMac7100_80		= 47,
	gestaltMacIIvx				= 48,
	gestaltMacColorClassic		= 49,
	gestaltPerforma250			= gestaltMacColorClassic,
	gestaltPowerBook165c		= 50,
	gestaltMacCentris610		= 52,
	gestaltMacQuadra610			= 53,
	gestaltPowerBook145			= 54,
	gestaltPowerMac8100_100		= 55,
	gestaltMacLC520				= 56,
	gestaltAWS9150_120			= 57,
	gestaltPowerMac6400			= 58,
	gestaltPerforma6400			= gestaltPowerMac6400,
	gestaltPerforma6360			= gestaltPerforma6400,
	gestaltMacCentris660AV		= 60,
	gestaltMacQuadra660AV		= gestaltMacCentris660AV,
	gestaltPerforma46x			= 62,
	gestaltPowerMac8100_80		= 65,
	gestaltAWS8150_80			= gestaltPowerMac8100_80,
	gestaltPowerMac9500			= 67,
	gestaltPowerMac9600			= gestaltPowerMac9500,
	gestaltPowerMac7500			= 68,
	gestaltPowerMac7600			= gestaltPowerMac7500,
	gestaltPowerMac8500			= 69,
	gestaltPowerMac8600			= gestaltPowerMac8500,
	gestaltAWS8550				= gestaltPowerMac7500,
	gestaltPowerBook180c		= 71,
	gestaltPowerBook520			= 72,
	gestaltPowerBook520c		= gestaltPowerBook520,
	gestaltPowerBook540			= gestaltPowerBook520,
	gestaltPowerBook540c		= gestaltPowerBook520,
	gestaltPowerMac5400			= 74,
	gestaltPowerMac6100_60		= 75,
	gestaltAWS6150_60			= gestaltPowerMac6100_60,
	gestaltPowerBookDuo270c		= 77,
	gestaltMacQuadra840AV		= 78,
	gestaltPerforma550			= 80,
	gestaltPowerBook165			= 84,
	gestaltPowerBook190			= 85,
	gestaltMacTV				= 88,
	gestaltMacLC475				= 89,
	gestaltPerforma47x			= gestaltMacLC475,
	gestaltMacLC575				= 92,
	gestaltMacQuadra605			= 94,
	gestaltMacQuadra630			= 98,
	gestaltMacLC580				= 99,
	gestaltPerforma580			= gestaltMacLC580,
	gestaltPowerMac6100_66		= 100,
	gestaltAWS6150_66			= gestaltPowerMac6100_66,
	gestaltPowerBookDuo280		= 102,
	gestaltPowerBookDuo280c		= 103,
	gestaltPowerMacLC475		= 104,							/* Mac LC 475 & PPC Processor Upgrade Card*/
	gestaltPowerMacPerforma47x	= gestaltPowerMacLC475,
	gestaltPowerMacLC575		= 105,							/* Mac LC 575 & PPC Processor Upgrade Card */
	gestaltPowerMacPerforma57x	= gestaltPowerMacLC575,
	gestaltPowerMacQuadra630	= 106,							/* Quadra 630 & PPC Processor Upgrade Card*/
	gestaltPowerMacLC630		= gestaltPowerMacQuadra630,		/* Mac LC 630 & PPC Processor Upgrade Card*/
	gestaltPowerMacPerforma63x	= gestaltPowerMacQuadra630,		/* Performa 63x & PPC Processor Upgrade Card*/
	gestaltPowerMac7200			= 108,
	gestaltPowerMac7300			= 109,
	gestaltPowerMac7100_66		= 112,
	gestaltPowerBook150			= 115,
	gestaltPowerMacQuadra700	= 116,							/* Quadra 700 & Power PC Upgrade Card*/
	gestaltPowerMacQuadra900	= 117,							/* Quadra 900 & Power PC Upgrade Card */
	gestaltPowerMacQuadra950	= 118,							/* Quadra 950 & Power PC Upgrade Card */
	gestaltPowerMacCentris610	= 119,							/* Centris 610 & Power PC Upgrade Card */
	gestaltPowerMacCentris650	= 120,							/* Centris 650 & Power PC Upgrade Card */
	gestaltPowerMacQuadra610	= 121,							/* Quadra 610 & Power PC Upgrade Card */
	gestaltPowerMacQuadra650	= 122,							/* Quadra 650 & Power PC Upgrade Card */
	gestaltPowerMacQuadra800	= 123,							/* Quadra 800 & Power PC Upgrade Card */
	gestaltPowerBookDuo2300		= 124,
	gestaltPowerBook500PPCUpgrade = 126,
	gestaltPowerBook5300		= 128,
	gestaltPowerBook1400		= 310,
	gestaltPowerBook3400		= 306,
	gestaltPowerBook2400		= 307,
	gestaltPowerBookG3			= 313,
	gestaltPowerMac5500			= 512,
	gestalt20thAnniversary		= gestaltPowerMac5500,
	gestaltPowerMac6500			= 513,
	gestaltPowerMac4400_160		= 514,							/* slower machine has different machine ID*/
	gestaltPowerMac4400			= 515
};



enum {
	gestaltQuadra605			= gestaltMacQuadra605,
	gestaltQuadra610			= gestaltMacQuadra610,
	gestaltQuadra630			= gestaltMacQuadra630,
	gestaltQuadra650			= gestaltMacQuadra650,
	gestaltQuadra660AV			= gestaltMacQuadra660AV,
	gestaltQuadra700			= gestaltMacQuadra700,
	gestaltQuadra800			= gestaltMacQuadra800,
	gestaltQuadra840AV			= gestaltMacQuadra840AV,
	gestaltQuadra900			= gestaltMacQuadra900,
	gestaltQuadra950			= gestaltMacQuadra950
};


enum {
	kMachineNameStrID			= -16395
};


enum {
	gestaltSMPMailerVersion		= FOUR_CHAR_CODE('malr')		/* OCE StandardMail*/
};


enum {
	gestaltMediaBay				= FOUR_CHAR_CODE('mbeh'),		/* media bay driver type */
	gestaltMBLegacy				= 0,							/* media bay support in PCCard 2.0 */
	gestaltMBSingleBay			= 1,							/* single bay media bay driver */
	gestaltMBMultipleBays		= 2								/* multi-bay media bay driver */
};


enum {
	gestaltMessageMgrVersion	= FOUR_CHAR_CODE('mess')		/* GX Printing Message Manager Gestalt Selector */
};


enum {
	gestaltMachineIcon			= FOUR_CHAR_CODE('micn')		/* machine icon */
};


enum {
	gestaltMiscAttr				= FOUR_CHAR_CODE('misc'),		/* miscellaneous attributes */
	gestaltScrollingThrottle	= 0,							/* true if scrolling throttle on */
	gestaltSquareMenuBar		= 2								/* true if menu bar is square */
};


/*
	The name gestaltMixedModeVersion for the 'mixd' selector is semantically incorrect.
	The same selector has been renamed gestaltMixedModeAttr to properly reflect the
	Inside Mac: PowerPC System Software documentation.  The gestaltMixedModeVersion
	symbol has been preserved only for backwards compatibility.

	Developers are forewarned that gestaltMixedModeVersion has a limited lifespan and
	will be removed in a future release of the Interfaces.

	For the first version of Mixed Mode, both meanings of the 'mixd' selector are
	functionally identical.  They both return 0x00000001.  In subsequent versions
	of Mixed Mode, however, the 'mixd' selector will not respond with an increasing
	version number, but rather, with 32 attribute bits with various meanings.
*/

enum {
	gestaltMixedModeVersion		= FOUR_CHAR_CODE('mixd')		/* returns version of Mixed Mode */
};


enum {
	gestaltMixedModeAttr		= FOUR_CHAR_CODE('mixd'),		/* returns Mixed Mode attributes */
	gestaltMixedModePowerPC		= 0,							/* true if Mixed Mode supports PowerPC ABI calling conventions */
	gestaltPowerPCAware			= 0,							/* old name for gestaltMixedModePowerPC */
	gestaltMixedModeCFM68K		= 1,							/* true if Mixed Mode supports CFM-68K calling conventions */
	gestaltMixedModeCFM68KHasTrap = 2,							/* true if CFM-68K Mixed Mode implements _MixedModeDispatch (versions 1.0.1 and prior did not) */
	gestaltMixedModeCFM68KHasState = 3							/* true if CFM-68K Mixed Mode exports Save/RestoreMixedModeState */
};


enum {
	gestaltQuickTimeConferencing = FOUR_CHAR_CODE('mtlk')		/* returns QuickTime Conferencing version */
};


enum {
	gestaltMemoryMapAttr		= FOUR_CHAR_CODE('mmap'),		/* Memory map type */
	gestaltMemoryMapSparse		= 0								/* Sparse memory is on */
};


enum {
	gestaltMMUType				= FOUR_CHAR_CODE('mmu '),		/* mmu type */
	gestaltNoMMU				= 0,							/* no MMU */
	gestaltAMU					= 1,							/* address management unit */
	gestalt68851				= 2,							/* 68851 PMMU */
	gestalt68030MMU				= 3,							/* 68030 built-in MMU */
	gestalt68040MMU				= 4,							/* 68040 built-in MMU */
	gestaltEMMU1				= 5								/* Emulated MMU type 1  */
};


enum {
	gestaltStdNBPAttr			= FOUR_CHAR_CODE('nlup'),		/* standard nbp attributes */
	gestaltStdNBPPresent		= 0,
	gestaltStdNBPSupportsAutoPosition = 1						/* StandardNBP takes (-1,-1) to mean alert position main screen */
};


enum {
	gestaltNotificationMgrAttr	= FOUR_CHAR_CODE('nmgr'),		/* notification manager attributes */
	gestaltNotificationPresent	= 0								/* notification manager exists */
};


enum {
	gestaltNameRegistryVersion	= FOUR_CHAR_CODE('nreg')		/* NameRegistryLib version number, for System 7.5.2+ usage */
};


enum {
	gestaltNuBusSlotCount		= FOUR_CHAR_CODE('nubs')		/* count of logical NuBus slots present */
};


enum {
	gestaltOCEToolboxVersion	= FOUR_CHAR_CODE('ocet'),		/* OCE Toolbox version */
	gestaltOCETB				= 0x0102,						/* OCE Toolbox version 1.02 */
	gestaltSFServer				= 0x0100						/* S&F Server version 1.0 */
};


enum {
	gestaltOCEToolboxAttr		= FOUR_CHAR_CODE('oceu'),		/* OCE Toolbox attributes */
	gestaltOCETBPresent			= 0x01,							/* OCE toolbox is present, not running */
	gestaltOCETBAvailable		= 0x02,							/* OCE toolbox is running and available */
	gestaltOCESFServerAvailable	= 0x04,							/* S&F Server is running and available */
	gestaltOCETBNativeGlueAvailable = 0x10						/* Native PowerPC Glue routines are availible */
};


enum {
	gestaltOpenFirmwareInfo		= FOUR_CHAR_CODE('opfw')		/* Open Firmware info */
};


enum {
	gestaltOSAttr				= FOUR_CHAR_CODE('os  '),		/* o/s attributes */
	gestaltSysZoneGrowable		= 0,							/* system heap is growable */
	gestaltLaunchCanReturn		= 1,							/* can return from launch */
	gestaltLaunchFullFileSpec	= 2,							/* can launch from full file spec */
	gestaltLaunchControl		= 3,							/* launch control support available */
	gestaltTempMemSupport		= 4,							/* temp memory support */
	gestaltRealTempMemory		= 5,							/* temp memory handles are real */
	gestaltTempMemTracked		= 6,							/* temporary memory handles are tracked */
	gestaltIPCSupport			= 7,							/* IPC support is present */
	gestaltSysDebuggerSupport	= 8								/* system debugger support is present */
};


enum {
	gestaltOSTable				= FOUR_CHAR_CODE('ostt')		/*  OS trap table base  */
};


enum {
	gestaltPCCard				= FOUR_CHAR_CODE('pccd'),		/*	PC Card attributes*/
	gestaltCardServicesPresent	= 0,							/*	PC Card 2.0 (68K) API is present*/
	gestaltPCCardFamilyPresent	= 1,							/*	PC Card 3.x (PowerPC) API is present*/
	gestaltPCCardHasPowerControl = 2,							/*	PCCardSetPowerLevel is supported*/
	gestaltPCCardSupportsCardBus = 3							/*	CardBus is supported*/
};


enum {
	gestaltProcClkSpeed			= FOUR_CHAR_CODE('pclk')		/* processor clock speed in hertz */
};


enum {
	gestaltPCXAttr				= FOUR_CHAR_CODE('pcxg'),		/* PC Exchange attributes */
	gestaltPCXHas8and16BitFAT	= 0,							/* PC Exchange supports both 8 and 16 bit FATs */
	gestaltPCXHasProDOS			= 1								/* PC Exchange supports ProDOS */
};


enum {
	gestaltLogicalPageSize		= FOUR_CHAR_CODE('pgsz')		/* logical page size */
};

/*	System 7.6 and later.  If gestaltScreenCaptureMain is not implemented,
	PictWhap proceeds with screen capture in the usual way.

	The high word of gestaltScreenCaptureMain is reserved (use 0).

	To disable screen capture to disk, put zero in the low word.  To
	specify a folder for captured pictures, put the vRefNum in the
	low word of gestaltScreenCaptureMain, and put the directory ID in
	gestaltScreenCaptureDir.
*/

enum {
	gestaltScreenCaptureMain	= FOUR_CHAR_CODE('pic1'),		/* Zero, or vRefNum of disk to hold picture */
	gestaltScreenCaptureDir		= FOUR_CHAR_CODE('pic2')		/* Directory ID of folder to hold picture */
};


enum {
	gestaltGXPrintingMgrVersion	= FOUR_CHAR_CODE('pmgr')		/* QuickDraw GX Printing Manager Version*/
};


enum {
	gestaltPopupAttr			= FOUR_CHAR_CODE('pop!'),		/* popup cdef attributes */
	gestaltPopupPresent			= 0
};


enum {
	gestaltPowerMgrAttr			= FOUR_CHAR_CODE('powr'),		/* power manager attributes */
	gestaltPMgrExists			= 0,
	gestaltPMgrCPUIdle			= 1,
	gestaltPMgrSCC				= 2,
	gestaltPMgrSound			= 3,
	gestaltPMgrDispatchExists	= 4,
	gestaltPMgrSupportsAVPowerStateAtSleepWake = 5
};

/*
 * PPC will return the combination of following bit fields.
 * e.g. gestaltPPCSupportsRealTime +gestaltPPCSupportsIncoming + gestaltPPCSupportsOutGoing
 * indicates PPC is cuurently is only supports real time delivery
 * and both incoming and outgoing network sessions are allowed.
 * By default local real time delivery is supported as long as PPCInit has been called.*/

enum {
	gestaltPPCToolboxAttr		= FOUR_CHAR_CODE('ppc '),		/* PPC toolbox attributes */
	gestaltPPCToolboxPresent	= 0x0000,						/* PPC Toolbox is present  Requires PPCInit to be called */
	gestaltPPCSupportsRealTime	= 0x1000,						/* PPC Supports real-time delivery */
	gestaltPPCSupportsIncoming	= 0x0001,						/* PPC will deny incoming network requests */
	gestaltPPCSupportsOutGoing	= 0x0002						/* PPC will deny outgoing network requests */
};


enum {
	gestaltProcessorType		= FOUR_CHAR_CODE('proc'),		/* processor type */
	gestalt68000				= 1,
	gestalt68010				= 2,
	gestalt68020				= 3,
	gestalt68030				= 4,
	gestalt68040				= 5
};


enum {
	gestaltSDPPromptVersion		= FOUR_CHAR_CODE('prpv')		/* OCE Standard Directory Panel*/
};


enum {
	gestaltParityAttr			= FOUR_CHAR_CODE('prty'),		/* parity attributes */
	gestaltHasParityCapability	= 0,							/* has ability to check parity */
	gestaltParityEnabled		= 1								/* parity checking enabled */
};


enum {
	gestaltQD3DVersion			= FOUR_CHAR_CODE('q3v ')		/* Quickdraw 3D version in pack BCD*/
};


enum {
	gestaltQD3DViewer			= FOUR_CHAR_CODE('q3vc'),		/* Quickdraw 3D viewer attributes*/
	gestaltQD3DViewerPresent	= 0								/* bit 0 set if QD3D Viewer is available*/
};

#if OLDROUTINENAMES

enum {
	gestaltQD3DViewerNotPresent	= (0 << gestaltQD3DViewerPresent),
	gestaltQD3DViewerAvailable	= (1 << gestaltQD3DViewerPresent)
};

#endif  /* OLDROUTINENAMES */


enum {
	gestaltQuickdrawVersion		= FOUR_CHAR_CODE('qd  '),		/* quickdraw version */
	gestaltOriginalQD			= 0x0000,						/* original 1-bit QD */
	gestalt8BitQD				= 0x0100,						/* 8-bit color QD */
	gestalt32BitQD				= 0x0200,						/* 32-bit color QD */
	gestalt32BitQD11			= 0x0201,						/* 32-bit color QDv1.1 */
	gestalt32BitQD12			= 0x0220,						/* 32-bit color QDv1.2 */
	gestalt32BitQD13			= 0x0230						/* 32-bit color QDv1.3 */
};


enum {
	gestaltQD3D					= FOUR_CHAR_CODE('qd3d'),		/* Quickdraw 3D attributes*/
	gestaltQD3DPresent			= 0								/* bit 0 set if QD3D available*/
};

#if OLDROUTINENAMES

enum {
	gestaltQD3DNotPresent		= (0 << gestaltQD3DPresent),
	gestaltQD3DAvailable		= (1 << gestaltQD3DPresent)
};

#endif  /* OLDROUTINENAMES */


enum {
	gestaltGXVersion			= FOUR_CHAR_CODE('qdgx')		/* Overall QuickDraw GX Version*/
};


enum {
	gestaltQuickdrawFeatures	= FOUR_CHAR_CODE('qdrw'),		/* quickdraw features */
	gestaltHasColor				= 0,							/* color quickdraw present */
	gestaltHasDeepGWorlds		= 1,							/* GWorlds can be deeper than 1-bit */
	gestaltHasDirectPixMaps		= 2,							/* PixMaps can be direct (16 or 32 bit) */
	gestaltHasGrayishTextOr		= 3,							/* supports text mode grayishTextOr */
	gestaltSupportsMirroring	= 4								/* Supports video mirroring via the Display Manager. */
};


enum {
	gestaltQDTextVersion		= FOUR_CHAR_CODE('qdtx'),		/* QuickdrawText version */
	gestaltOriginalQDText		= 0x0000,						/* up to and including 8.1 */
	gestaltAllegroQDText		= 0x0100						/* starting with 8.2 (?) */
};


enum {
	gestaltQDTextFeatures		= FOUR_CHAR_CODE('qdtf'),		/* QuickdrawText features */
	gestaltWSIISupportBit		= 0,							/* bit 0: WSII support included */
	gestaltSbitFontsBit			= 1,							/* sbit-only fonts supported */
	gestaltAntiAliasBit			= 2,							/* capably of antialiased text */
	gestaltOFA2availableBit		= 3								/* OFA2 available */
};



enum {
	gestaltQuickTimeConferencingInfo = FOUR_CHAR_CODE('qtci')	/* returns pointer to QuickTime Conferencing information */
};


enum {
	gestaltQuickTimeVersion		= FOUR_CHAR_CODE('qtim'),		/* returns version of QuickTime */
	gestaltQuickTime			= FOUR_CHAR_CODE('qtim')		/* gestaltQuickTime is old name for gestaltQuickTimeVersion */
};


enum {
	gestaltQuickTimeFeatures	= FOUR_CHAR_CODE('qtrs'),
	gestaltPPCQuickTimeLibPresent = 0							/* PowerPC QuickTime glue library is present */
};


enum {
	gestaltQTVRMgrAttr			= FOUR_CHAR_CODE('qtvr'),		/* QuickTime VR attributes                               */
	gestaltQTVRMgrPresent		= 0,							/* QTVR API is present                                   */
	gestaltQTVRObjMoviesPresent	= 1,							/* QTVR runtime knows about object movies                */
	gestaltQTVRCylinderPanosPresent = 2							/* QTVR runtime knows about cylindrical panoramic movies */
};


enum {
	gestaltQTVRMgrVers			= FOUR_CHAR_CODE('qtvv')		/* QuickTime VR version                                  */
};


enum {
	gestaltPhysicalRAMSize		= FOUR_CHAR_CODE('ram ')		/* physical RAM size */
};


enum {
	gestaltRBVAddr				= FOUR_CHAR_CODE('rbv ')		/* RBV base address  */
};


enum {
	gestaltROMSize				= FOUR_CHAR_CODE('rom ')		/* rom size */
};


enum {
	gestaltROMVersion			= FOUR_CHAR_CODE('romv')		/* rom version */
};


enum {
	gestaltResourceMgrAttr		= FOUR_CHAR_CODE('rsrc'),		/* Resource Mgr attributes */
	gestaltPartialRsrcs			= 0								/* True if partial resources exist */
};


enum {
	gestaltRealtimeMgrAttr		= FOUR_CHAR_CODE('rtmr'),		/* Realtime manager attributes			*/
	gestaltRealtimeMgrPresent	= 0								/* true if the Realtime manager is present 	*/
};


enum {
	gestaltSCCReadAddr			= FOUR_CHAR_CODE('sccr')		/* scc read base address  */
};


enum {
	gestaltSCCWriteAddr			= FOUR_CHAR_CODE('sccw')		/* scc read base address  */
};


enum {
	gestaltScrapMgrAttr			= FOUR_CHAR_CODE('scra'),		/* Scrap Manager attributes */
	gestaltScrapMgrTranslationAware = 0							/* True if scrap manager is translation aware */
};


enum {
	gestaltScriptMgrVersion		= FOUR_CHAR_CODE('scri')		/* Script Manager version number     */
};


enum {
	gestaltScriptCount			= FOUR_CHAR_CODE('scr#')		/* number of active script systems   */
};


enum {
	gestaltSCSI					= FOUR_CHAR_CODE('scsi'),		/* SCSI Manager attributes */
	gestaltAsyncSCSI			= 0,							/* Supports Asynchronous SCSI */
	gestaltAsyncSCSIINROM		= 1,							/* Async scsi is in ROM (available for booting) */
	gestaltSCSISlotBoot			= 2,							/* ROM supports Slot-style PRAM for SCSI boots (PDM and later) */
	gestaltSCSIPollSIH			= 3								/* SCSI Manager will poll for interrupts if Secondary Interrupts are busy. */
};


enum {
	gestaltControlStripAttr		= FOUR_CHAR_CODE('sdev'),		/* Control Strip attributes */
	gestaltControlStripExists	= 0,							/* Control Strip is installed */
	gestaltControlStripVersionFixed = 1,						/* Control Strip version Gestalt selector was fixed */
	gestaltControlStripUserFont	= 2,							/* supports user-selectable font/size */
	gestaltControlStripUserHotKey = 3							/* support user-selectable hot key to show/hide the window */
};


enum {
	gestaltSDPStandardDirectoryVersion = FOUR_CHAR_CODE('sdvr')	/* OCE Standard Directory Panel*/
};


enum {
	gestaltSerialAttr			= FOUR_CHAR_CODE('ser '),		/* Serial attributes */
	gestaltHasGPIaToDCDa		= 0,							/* GPIa connected to DCDa*/
	gestaltHasGPIaToRTxCa		= 1,							/* GPIa connected to RTxCa clock input*/
	gestaltHasGPIbToDCDb		= 2								/* GPIb connected to DCDb */
};


enum {
	gestaltShutdownAttributes	= FOUR_CHAR_CODE('shut'),		/* ShutDown Manager Attributes */
	gestaltShutdownHassdOnBootVolUnmount = 0					/* True if ShutDown Manager unmounts boot & VM volume at shutdown time. */
};


enum {
	gestaltNuBusConnectors		= FOUR_CHAR_CODE('sltc')		/* bitmap of NuBus connectors*/
};


enum {
	gestaltSlotAttr				= FOUR_CHAR_CODE('slot'),		/* slot attributes  */
	gestaltSlotMgrExists		= 0,							/* true is slot mgr exists  */
	gestaltNuBusPresent			= 1,							/* NuBus slots are present  */
	gestaltSESlotPresent		= 2,							/* SE PDS slot present  */
	gestaltSE30SlotPresent		= 3,							/* SE/30 slot present  */
	gestaltPortableSlotPresent	= 4								/* PortableÕs slot present  */
};


enum {
	gestaltFirstSlotNumber		= FOUR_CHAR_CODE('slt1')		/* returns first physical slot */
};


enum {
	gestaltSoundAttr			= FOUR_CHAR_CODE('snd '),		/* sound attributes */
	gestaltStereoCapability		= 0,							/* sound hardware has stereo capability */
	gestaltStereoMixing			= 1,							/* stereo mixing on external speaker */
	gestaltSoundIOMgrPresent	= 3,							/* The Sound I/O Manager is present */
	gestaltBuiltInSoundInput	= 4,							/* built-in Sound Input hardware is present */
	gestaltHasSoundInputDevice	= 5,							/* Sound Input device available */
	gestaltPlayAndRecord		= 6,							/* built-in hardware can play and record simultaneously */
	gestalt16BitSoundIO			= 7,							/* sound hardware can play and record 16-bit samples */
	gestaltStereoInput			= 8,							/* sound hardware can record stereo */
	gestaltLineLevelInput		= 9,							/* sound input port requires line level */
																/* the following bits are not defined prior to Sound Mgr 3.0 */
	gestaltSndPlayDoubleBuffer	= 10,							/* SndPlayDoubleBuffer available, set by Sound Mgr 3.0 and later */
	gestaltMultiChannels		= 11,							/* multiple channel support, set by Sound Mgr 3.0 and later */
	gestalt16BitAudioSupport	= 12							/* 16 bit audio data supported, set by Sound Mgr 3.0 and later */
};


enum {
	gestaltSplitOSAttr			= FOUR_CHAR_CODE('spos'),
	gestaltSplitOSAware			= 1,							/* the system includes the code to deal with a split os situation. */
	gestaltSplitOSEnablerVolumeIsDifferentFromBootVolume = 2	/* the active enabler is on a different volume than the system file. */
};


enum {
	gestaltSMPSPSendLetterVersion = FOUR_CHAR_CODE('spsl')		/* OCE StandardMail*/
};


enum {
	gestaltSpeechRecognitionAttr = FOUR_CHAR_CODE('srta'),		/* speech recognition attributes */
	gestaltDesktopSpeechRecognition = 1,						/* recognition thru the desktop microphone is available */
	gestaltTelephoneSpeechRecognition = 2						/* recognition thru the telephone is available */
};


enum {
	gestaltSpeechRecognitionVersion = FOUR_CHAR_CODE('srtb')	/* speech recognition version (0x0150 is the first version that fully supports the API) */
};


enum {
	gestaltSoftwareVendorCode	= FOUR_CHAR_CODE('srad'),		/* Returns system software vendor information */
	gestaltSoftwareVendorApple	= FOUR_CHAR_CODE('Appl'),		/* System software sold by Apple */
	gestaltSoftwareVendorLicensee = FOUR_CHAR_CODE('Lcns')		/* System software sold by licensee */
};


enum {
	gestaltStandardFileAttr		= FOUR_CHAR_CODE('stdf'),		/* Standard File attributes */
	gestaltStandardFile58		= 0,							/* True if selectors 5-8 (StandardPutFile-CustomGetFile) are supported */
	gestaltStandardFileTranslationAware = 1,					/* True if standard file is translation manager aware */
	gestaltStandardFileHasColorIcons = 2,						/* True if standard file has 16x16 color icons */
	gestaltStandardFileUseGenericIcons = 3,						/* Standard file LDEF to use only the system generic icons if true */
	gestaltStandardFileHasDynamicVolumeAllocation = 4			/* True if standard file supports more than 20 volumes */
};


enum {
	gestaltSysArchitecture		= FOUR_CHAR_CODE('sysa'),		/* Native System Architecture */
	gestalt68k					= 1,							/* Motorola MC68k architecture */
	gestaltPowerPC				= 2								/* IBM PowerPC architecture */
};


enum {
	gestaltSystemUpdateVersion	= FOUR_CHAR_CODE('sysu')		/* System Update version */
};


enum {
	gestaltSystemVersion		= FOUR_CHAR_CODE('sysv')		/* system version*/
};


enum {
	gestaltToolboxTable			= FOUR_CHAR_CODE('tbtt')		/*  OS trap table base  */
};


enum {
	gestaltTextEditVersion		= FOUR_CHAR_CODE('te  '),		/* TextEdit version number */
	gestaltTE1					= 1,							/* TextEdit in MacIIci ROM */
	gestaltTE2					= 2,							/* TextEdit with 6.0.4 Script Systems on MacIIci (Script bug fixes for MacIIci) */
	gestaltTE3					= 3,							/* TextEdit with 6.0.4 Script Systems all but MacIIci */
	gestaltTE4					= 4,							/* TextEdit in System 7.0 */
	gestaltTE5					= 5								/* TextWidthHook available in TextEdit */
};


enum {
	gestaltTEAttr				= FOUR_CHAR_CODE('teat'),		/* TextEdit attributes */
	gestaltTEHasGetHiliteRgn	= 0,							/* TextEdit has TEGetHiliteRgn */
	gestaltTESupportsInlineInput = 1,							/* TextEdit does Inline Input */
	gestaltTESupportsTextObjects = 2,							/* TextEdit does Text Objects */
	gestaltTEHasWhiteBackground	= 3								/* TextEdit supports overriding the TERec's background to white */
};


enum {
	gestaltTeleMgrAttr			= FOUR_CHAR_CODE('tele'),		/* Telephone manager attributes */
	gestaltTeleMgrPresent		= 0,
	gestaltTeleMgrPowerPCSupport = 1,
	gestaltTeleMgrSoundStreams	= 2,
	gestaltTeleMgrAutoAnswer	= 3,
	gestaltTeleMgrIndHandset	= 4,
	gestaltTeleMgrSilenceDetect	= 5,
	gestaltTeleMgrNewTELNewSupport = 6
};


enum {
	gestaltTermMgrAttr			= FOUR_CHAR_CODE('term'),		/* terminal mgr attributes */
	gestaltTermMgrPresent		= 0,
	gestaltTermMgrErrorString	= 2
};


enum {
	gestaltThreadMgrAttr		= FOUR_CHAR_CODE('thds'),		/* Thread Manager attributes */
	gestaltThreadMgrPresent		= 0,							/* bit true if Thread Mgr is present */
	gestaltSpecificMatchSupport	= 1,							/* bit true if Thread Mgr supports exact match creation option */
	gestaltThreadsLibraryPresent = 2							/* bit true if Thread Mgr shared library is present */
};


enum {
	gestaltTimeMgrVersion		= FOUR_CHAR_CODE('tmgr'),		/* time mgr version */
	gestaltStandardTimeMgr		= 1,							/* standard time mgr is present */
	gestaltRevisedTimeMgr		= 2,							/* revised time mgr is present */
	gestaltExtendedTimeMgr		= 3								/* extended time mgr is present */
};


enum {
	gestaltTSMTEVersion			= FOUR_CHAR_CODE('tmTV'),
	gestaltTSMTE1				= 0x0100,
	gestaltTSMTE15				= 0x0150,
	gestaltTSMTE2				= 0x0200
};


enum {
	gestaltTSMTEAttr			= FOUR_CHAR_CODE('tmTE'),
	gestaltTSMTEPresent			= 0,
	gestaltTSMTE				= 0								/* gestaltTSMTE is old name for gestaltTSMTEPresent */
};


enum {
	gestaltALMAttr				= FOUR_CHAR_CODE('trip'),		/* Settings Manager attributes (see also gestaltALMVers) */
	gestaltALMPresent			= 0,							/* bit true if ALM is available */
	gestaltALMHasSFGroup		= 1,							/* bit true if Put/Get/Merge Group calls are implmented */
	gestaltALMHasCFMSupport		= 2,							/* bit true if CFM-based modules are supported */
	gestaltALMHasRescanNotifiers = 3							/* bit true if Rescan notifications/events will be sent to clients */
};


enum {
	gestaltALMHasSFLocation		= gestaltALMHasSFGroup
};


enum {
	gestaltTSMgrVersion			= FOUR_CHAR_CODE('tsmv'),		/* Text Services Mgr version, if present */
	gestaltTSMgr15				= 0x0150,
	gestaltTSMgr2				= 0x0200
};


enum {
	gestaltTSMgrAttr			= FOUR_CHAR_CODE('tsma'),		/* Text Services Mgr attributes, if present */
	gestaltTSMDisplayMgrAwareBit = 0,							/* TSM knows about display manager */
	gestaltTSMdoesTSMTEBit		= 1								/* TSM has integrated TSMTE */
};


enum {
	gestaltSpeechAttr			= FOUR_CHAR_CODE('ttsc'),		/* Speech Manager attributes */
	gestaltSpeechMgrPresent		= 0,							/* bit set indicates that Speech Manager exists */
	gestaltSpeechHasPPCGlue		= 1								/* bit set indicates that native PPC glue for Speech Manager API exists */
};


enum {
	gestaltTVAttr				= FOUR_CHAR_CODE('tv  '),		/* TV version */
	gestaltHasTVTuner			= 0,							/* supports Philips FL1236F video tuner */
	gestaltHasSoundFader		= 1,							/* supports Philips TEA6330 Sound Fader chip */
	gestaltHasHWClosedCaptioning = 2,							/* supports Philips SAA5252 Closed Captioning */
	gestaltHasIRRemote			= 3,							/* supports CyclopsII Infra Red Remote control */
	gestaltHasVidDecoderScaler	= 4,							/* supports Philips SAA7194 Video Decoder/Scaler */
	gestaltHasStereoDecoder		= 5,							/* supports Sony SBX1637A-01 stereo decoder */
	gestaltHasSerialFader		= 6,							/* has fader audio in serial with system audio */
	gestaltHasFMTuner			= 7,							/* has FM Tuner from donnybrook card */
	gestaltHasSystemIRFunction	= 8,							/* Infra Red button function is set up by system and not by Video Startup */
	gestaltIRDisabled			= 9,							/* Infra Red remote is not disabled. */
	gestaltINeedIRPowerOffConfirm = 10,							/* Need IR power off confirm dialog. */
	gestaltHasZoomedVideo		= 11							/* Has Zoomed Video PC Card video input. */
};



enum {
	gestaltVersion				= FOUR_CHAR_CODE('vers'),		/* gestalt version */
	gestaltValueImplementedVers	= 5								/* version of gestalt where gestaltValue is implemented. */
};


enum {
	gestaltVIA1Addr				= FOUR_CHAR_CODE('via1')		/* via 1 base address  */
};


enum {
	gestaltVIA2Addr				= FOUR_CHAR_CODE('via2')		/* via 2 base address  */
};


enum {
	gestaltVMAttr				= FOUR_CHAR_CODE('vm  '),		/* virtual memory attributes */
	gestaltVMPresent			= 0,							/* true if virtual memory is present */
	gestaltVMHasLockMemoryForOutput = 1,						/* true if LockMemoryForOutput is available */
	gestaltVMFilemappingOn		= 3,							/* true if filemapping is available */
	gestaltVMHasPagingControl	= 4								/* true if MakeMemoryResident, MakeMemoryNonResident, FlushMemory, and ReleaseMemoryData are available */
};


enum {
	gestaltALMVers				= FOUR_CHAR_CODE('walk')		/* Settings Manager version (see also gestaltALMAttr) */
};


enum {
	gestaltTranslationAttr		= FOUR_CHAR_CODE('xlat'),		/* Translation Manager attributes */
	gestaltTranslationMgrExists	= 0,							/* True if translation manager exists */
	gestaltTranslationMgrHintOrder = 1,							/* True if hint order reversal in effect */
	gestaltTranslationPPCAvail	= 2,
	gestaltTranslationGetPathAPIAvail = 3
};


enum {
	gestaltExtToolboxTable		= FOUR_CHAR_CODE('xttt')		/* Extended Toolbox trap table base */
};




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

#endif /* __GESTALT__ */

