/*
 	File:		Gestalt.r
 
 	Contains:	Gestalt Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1998 by Apple Computer, Inc.  All rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __GESTALT_R__
#define __GESTALT_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define gestaltAddressingModeAttr 		'addr'				/*  addressing mode attributes  */
#define gestalt32BitAddressing 			0					/*  using 32-bit addressing mode  */
#define gestalt32BitSysZone 			1					/*  32-bit compatible system zone  */
#define gestalt32BitCapable 			2					/*  Machine is 32-bit capable  */

#define gestaltAFPClient 				'afps'
#define gestaltAFPClientVersionMask 	0x0000FFFF			/*  low word of long is the  */
															/*  client version 0x0001 -> 0x0007 */
#define gestaltAFPClient3_5 			0x0001
#define gestaltAFPClient3_6 			0x0002
#define gestaltAFPClient3_6_1 			0x0003
#define gestaltAFPClient3_6_2 			0x0004
#define gestaltAFPClient3_6_3 			0x0005				/*  including 3.6.4, 3.6.5 */
#define gestaltAFPClient3_7 			0x0006				/*  including 3.7.1 */
#define gestaltAFPClient3_7_2 			0x0007				/*  including 3.7.3 */
#define gestaltAFPClientAttributeMask 	0xFFFF0000			/*  high word of long is a  */
															/*  set of attribute bits */
#define gestaltAFPClientCfgRsrc 		16					/*  Client uses config resources */
#define gestaltAFPClientSupportsIP 		29					/*  Client supports AFP over TCP/IP */
#define gestaltAFPClientVMUI 			30					/*  Client can put up UI from the PBVolMount trap */
#define gestaltAFPClientMultiReq 		31					/*  Client supports multiple outstanding requests */

#define gestaltAliasMgrAttr 			'alis'				/*  Alias Mgr Attributes  */
#define gestaltAliasMgrPresent 			0					/*  True if the Alias Mgr is present  */
#define gestaltAliasMgrSupportsRemoteAppletalk  1			/*  True if the Alias Mgr knows about Remote Appletalk  */
#define gestaltAliasMgrSupportsAOCEKeychain  2				/*  True if the Alias Mgr knows about the AOCE Keychain  */
#define gestaltAliasMgrResolveAliasFileWithMountOptions  3	/*  True if the Alias Mgr implements gestaltAliasMgrResolveAliasFileWithMountOptions() and IsAliasFile()  */

#define gestaltArbitorAttr 				'arb '
#define gestaltSerialArbitrationExists 	0					/*  this bit if the serial port arbitrator exists */

#define gestaltAppleScriptVersion 		'ascv'				/*  AppleScript version */
#define gestaltAppleScriptAttr 			'ascr'				/*  AppleScript attributes */
#define gestaltAppleScriptPresent 		0
#define gestaltAppleScriptPowerPCSupport  1

#define gestaltATAAttr 					'ata '				/*  ATA is the driver to support IDE hard disks  */
#define gestaltATAPresent 				0					/*  if set, ATA Manager is present  */

#define gestaltATalkVersion 			'atkv'				/*  Detailed AppleTalk version; see comment above for format  */
#define gestaltAppleTalkVersion 		'atlk'				/*  appletalk version  */
#define gestaltAUXVersion 				'a/ux'				/*  a/ux version, if present  */
#define gestaltBusClkSpeed 				'bclk'				/*  main I/O bus clock speed in hertz  */
#define gestaltCloseViewAttr 			'BSDa'				/*  CloseView attributes  */
#define gestaltCloseViewEnabled 		0					/*  Closeview enabled (dynamic bit - returns current state)  */
#define gestaltCloseViewDisplayMgrFriendly  1				/*  Closeview compatible with Display Manager (FUTURE)  */

#define gestaltCFMAttr 					'cfrg'				/*  returns information about the Code Fragment Manager  */
#define gestaltCFMPresent 				0					/*  true if the Code Fragment Manager is present  */

#define gestaltCollectionMgrVersion 	'cltn'				/*  Collection Manager version  */
#define gestaltColorMatchingAttr 		'cmta'				/*  ColorSync attributes  */
#define gestaltHighLevelMatching 		0
#define gestaltColorMatchingLibLoaded 	1

#define gestaltColorMatchingVersion 	'cmtc'
#define gestaltColorSync10 				0x0100				/*  0x0100 & 0x0110 _Gestalt versions for 1.0-1.0.3 product  */
#define gestaltColorSync11 				0x0110				/*    0x0100 == low-level matching only  */
#define gestaltColorSync104 			0x0104				/*  Real version, by popular demand  */
#define gestaltColorSync105 			0x0105
#define gestaltColorSync20 				0x0200				/*  ColorSync 2.0  */
#define gestaltColorSync21 				0x0210
#define gestaltColorSync211 			0x0211
#define gestaltColorSync212 			0x0212
#define gestaltColorSync213 			0x0213
#define gestaltColorSync25 				0x0250

#define gestaltConnMgrAttr 				'conn'				/*  connection mgr attributes     */
#define gestaltConnMgrPresent 			0
#define gestaltConnMgrCMSearchFix 		1					/*  Fix to CMAddSearch?      */
#define gestaltConnMgrErrorString 		2					/*  has CMGetErrorString()  */
#define gestaltConnMgrMultiAsyncIO 		3					/*  CMNewIOPB, CMDisposeIOPB, CMPBRead, CMPBWrite, CMPBIOKill  */

#define gestaltColorPickerVersion 		'cpkr'				/*  returns version of ColorPicker  */
#define gestaltColorPicker 				'cpkr'				/*  gestaltColorPicker is old name for gestaltColorPickerVersion  */

#define gestaltComponentMgr 			'cpnt'				/*  Component Mgr version  */
#define gestaltNativeCPUtype 			'cput'				/*  Native CPU type 									   */
#define gestaltNativeCPUfamily 			'cpuf'				/*  Native CPU family								   */
#define gestaltCPU68000 				0					/*  Various 68k CPUs... 	 */
#define gestaltCPU68010 				1
#define gestaltCPU68020 				2
#define gestaltCPU68030 				3
#define gestaltCPU68040 				4
#define gestaltCPU601 					0x0101				/*  IBM 601 												 */
#define gestaltCPU603 					0x0103
#define gestaltCPU604 					0x0104
#define gestaltCPU603e 					0x0106
#define gestaltCPU603ev 				0x0107
#define gestaltCPU750 					0x0108				/*  Also 740 - "G3"  */
#define gestaltCPU604e 					0x0109
#define gestaltCPU604ev 				0x010A				/*  Mach 5, 250Mhz and up  */

#define gestaltCRMAttr 					'crm '				/*  comm resource mgr attributes  */
#define gestaltCRMPresent 				0
#define gestaltCRMPersistentFix 		1					/*  fix for persistent tools  */
#define gestaltCRMToolRsrcCalls 		2					/*  has CRMGetToolResource/ReleaseToolResource  */

#define gestaltControlStripVersion 		'csvr'				/*  Control Strip version (was 'sdvr')  */
#define gestaltCTBVersion 				'ctbv'				/*  CommToolbox version  */
#define gestaltDBAccessMgrAttr 			'dbac'				/*  Database Access Mgr attributes  */
#define gestaltDBAccessMgrPresent 		0					/*  True if Database Access Mgr present  */

#define gestaltSDPFindVersion 			'dfnd'				/*  OCE Standard Directory Panel */
#define gestaltDictionaryMgrAttr 		'dict'				/*  Dictionary Manager attributes  */
#define gestaltDictionaryMgrPresent 	0					/*  Dictionary Manager attributes  */

#define gestaltDITLExtAttr 				'ditl'				/*  AppenDITL, etc. calls from CTB  */
#define gestaltDITLExtPresent 			0					/*  True if calls are present  */
#define gestaltDITLExtSupportsIctb 		1					/*  True if AppendDITL, ShortenDITL support 'ictb's  */

#define gestaltDesktopPicturesAttr 		'dkpx'				/*  Desktop Pictures attributes  */
#define gestaltDesktopPicturesInstalled  0					/*  True if control panel is installed  */
#define gestaltDesktopPicturesDisplayed  1					/*  True if a picture is currently displayed  */

#define gestaltDisplayMgrVers 			'dplv'				/*  Display Manager version  */
#define gestaltDisplayMgrAttr 			'dply'				/*  Display Manager attributes  */
#define gestaltDisplayMgrPresent 		0					/*  True if Display Mgr is present  */
#define gestaltDisplayMgrCanSwitchMirrored  2				/*  True if Display Mgr can switch modes on mirrored displays  */
#define gestaltDisplayMgrSetDepthNotifies  3				/*  True SetDepth generates displays mgr notification  */
#define gestaltDisplayMgrCanConfirm 	4					/*  True Display Manager supports DMConfirmConfiguration  */

#define gestaltDragMgrAttr 				'drag'				/*  Drag Manager attributes  */
#define gestaltDragMgrPresent 			0					/*  Drag Manager is present  */
#define gestaltDragMgrFloatingWind 		1					/*  Drag Manager supports floating windows  */
#define gestaltPPCDragLibPresent 		2					/*  Drag Manager PPC DragLib is present  */
#define gestaltDragMgrHasImageSupport 	3					/*  Drag Manager allows SetDragImage call  */
#define gestaltCanStartDragInFloatWindow  4					/*  Drag Manager supports starting a drag in a floating window  */

#define gestaltDigitalSignatureVersion 	'dsig'				/*  returns Digital Signature Toolbox version in low-order word */
#define gestaltEasyAccessAttr 			'easy'				/*  Easy Access attributes  */
#define gestaltEasyAccessOff 			0					/*  if Easy Access present, but off (no icon)  */
#define gestaltEasyAccessOn 			1					/*  if Easy Access "On"  */
#define gestaltEasyAccessSticky 		2					/*  if Easy Access "Sticky"  */
#define gestaltEasyAccessLocked 		3					/*  if Easy Access "Locked"  */

#define gestaltEditionMgrAttr 			'edtn'				/*  Edition Mgr attributes  */
#define gestaltEditionMgrPresent 		0					/*  True if Edition Mgr present  */
#define gestaltEditionMgrTranslationAware  1				/*  True if edition manager is translation manager aware  */

#define gestaltAppleEventsAttr 			'evnt'				/*  Apple Events attributes  */
#define gestaltAppleEventsPresent 		0					/*  True if Apple Events present  */
#define gestaltScriptingSupport 		1
#define gestaltOSLInSystem 				2					/*  OSL is in system so don’t use the one linked in to app  */

#define gestaltExtensionTableVersion 	'etbl'				/*  ExtensionTable version  */
#define gestaltFloppyAttr 				'flpy'				/*  Floppy disk drive/driver attributes  */
#define gestaltFloppyIsMFMOnly 			0					/*  Floppy driver only supports MFM disk formats  */
#define gestaltFloppyIsManualEject 		1					/*  Floppy drive, driver, and file system are in manual-eject mode  */
#define gestaltFloppyUsesDiskInPlace 	2					/*  Floppy drive must have special DISK-IN-PLACE output; standard DISK-CHANGED not used  */

#define gestaltFinderAttr 				'fndr'				/*  Finder attributes  */
#define gestaltFinderDropEvent 			0					/*  Finder recognizes drop event  */
#define gestaltFinderMagicPlacement 	1					/*  Finder supports magic icon placement  */
#define gestaltFinderCallsAEProcess 	2					/*  Finder calls AEProcessAppleEvent  */
#define gestaltOSLCompliantFinder 		3					/*  Finder is scriptable and recordable  */
#define gestaltFinderSupports4GBVolumes  4					/*  Finder correctly handles 4GB volumes  */
#define gestaltFinderHasClippings 		6					/*  Finder supports Drag Manager clipping files  */
#define gestaltFinderFullDragManagerSupport  7				/*  Finder accepts 'hfs ' flavors properly  */
#define gestaltFinderFloppyRootComments  8					/*  in MacOS 8 and later, will be set if Finder ever supports comments on Floppy icons  */
#define gestaltFinderLargeAndNotSavedFlavorsOK  9			/*  in MacOS 8 and later, drags with >1024-byte flavors and flavorNotSaved flavors work reliably  */

#define gestaltFindFolderAttr 			'fold'				/*  Folder Mgr attributes  */
#define gestaltFindFolderPresent 		0					/*  True if Folder Mgr present  */
#define gestaltFolderDescSupport 		1					/*  Tru if Folder Mgr has FolderDesc calls  */

#define gestaltFontMgrAttr 				'font'				/*  Font Mgr attributes  */
#define gestaltOutlineFonts 			0					/*  True if Outline Fonts supported  */

#define gestaltFPUType 					'fpu '				/*  fpu type  */
#define gestaltNoFPU 					0					/*  no FPU  */
#define gestalt68881 					1					/*  68881 FPU  */
#define gestalt68882 					2					/*  68882 FPU  */
#define gestalt68040FPU 				3					/*  68040 built-in FPU  */

#define gestaltFSAttr 					'fs  '				/*  file system attributes  */
#define gestaltFullExtFSDispatching 	0					/*  has really cool new HFSDispatch dispatcher  */
#define gestaltHasFSSpecCalls 			1					/*  has FSSpec calls  */
#define gestaltHasFileSystemManager 	2					/*  has a file system manager  */
#define gestaltFSMDoesDynamicLoad 		3					/*  file system manager supports dynamic loading  */
#define gestaltFSSupports4GBVols 		4					/*  file system supports 4 gigabyte volumes  */
#define gestaltFSSupports2TBVols 		5					/*  file system supports 2 terabyte volumes  */
#define gestaltHasExtendedDiskInit 		6					/*  has extended Disk Initialization calls  */
#define gestaltDTMgrSupportsFSM 		7					/*  Desktop Manager support FSM-based foreign file systems  */

#define gestaltFSMVersion 				'fsm '				/*  returns version of HFS External File Systems Manager (FSM)  */
#define gestaltFXfrMgrAttr 				'fxfr'				/*  file transfer manager attributes  */
#define gestaltFXfrMgrPresent 			0
#define gestaltFXfrMgrMultiFile 		1					/*  supports FTSend and FTReceive  */
#define gestaltFXfrMgrErrorString 		2					/*  supports FTGetErrorString  */
#define gestaltFXfrMgrAsync 			3					/* supports FTSendAsync, FTReceiveAsync, FTCompletionAsync */

#define gestaltGraphicsAttr 			'gfxa'				/*  Quickdraw GX attributes selector  */
#define gestaltGraphicsIsDebugging 		0x00000001
#define gestaltGraphicsIsLoaded 		0x00000002
#define gestaltGraphicsIsPowerPC 		0x00000004

#define gestaltGraphicsVersion 			'grfx'				/*  Quickdraw GX version selector  */
#define gestaltCurrentGraphicsVersion 	0x00010200			/*  the version described in this set of headers  */

#define gestaltHardwareAttr 			'hdwr'				/*  hardware attributes  */
#define gestaltHasVIA1 					0					/*  VIA1 exists  */
#define gestaltHasVIA2 					1					/*  VIA2 exists  */
#define gestaltHasASC 					3					/*  Apple Sound Chip exists  */
#define gestaltHasSCC 					4					/*  SCC exists  */
#define gestaltHasSCSI 					7					/*  SCSI exists  */
#define gestaltHasSoftPowerOff 			19					/*  Capable of software power off  */
#define gestaltHasSCSI961 				21					/*  53C96 SCSI controller on internal bus  */
#define gestaltHasSCSI962 				22					/*  53C96 SCSI controller on external bus  */
#define gestaltHasUniversalROM 			24					/*  Do we have a Universal ROM?  */
#define gestaltHasEnhancedLtalk 		30					/*  Do we have Enhanced LocalTalk?  */

#define gestaltHelpMgrAttr 				'help'				/*  Help Mgr Attributes  */
#define gestaltHelpMgrPresent 			0					/*  true if help mgr is present  */
#define gestaltHelpMgrExtensions 		1					/*  true if help mgr extensions are installed  */
#define gestaltAppleGuideIsDebug 		30
#define gestaltAppleGuidePresent 		31					/*  true if AppleGuide is installed  */

#define gestaltHardwareVendorCode 		'hrad'				/*  Returns hardware vendor information  */
#define gestaltHardwareVendorApple 		'Appl'				/*  Hardware built by Apple  */

#define gestaltCompressionMgr 			'icmp'				/*  returns version of the Image Compression Manager  */
#define gestaltIconUtilitiesAttr 		'icon'				/*  Icon Utilities attributes  (Note: available in System 7.0, despite gestalt)  */
#define gestaltIconUtilitiesPresent 	0					/*  true if icon utilities are present  */

#define gestaltInternalDisplay 			'idsp'				/*  slot number of internal display location  */
#define gestaltKeyboardType 			'kbd '				/*  keyboard type  */
#define gestaltMacKbd 					1
#define gestaltMacAndPad 				2
#define gestaltMacPlusKbd 				3
#define gestaltExtADBKbd 				4
#define gestaltStdADBKbd 				5
#define gestaltPrtblADBKbd 				6
#define gestaltPrtblISOKbd 				7
#define gestaltStdISOADBKbd 			8
#define gestaltExtISOADBKbd 			9
#define gestaltADBKbdII 				10
#define gestaltADBISOKbdII 				11
#define gestaltPwrBookADBKbd 			12
#define gestaltPwrBookISOADBKbd 		13
#define gestaltAppleAdjustKeypad 		14
#define gestaltAppleAdjustADBKbd 		15
#define gestaltAppleAdjustISOKbd 		16
#define gestaltJapanAdjustADBKbd 		17					/*  Japan Adjustable Keyboard  */
#define gestaltPwrBkExtISOKbd 			20					/*  PowerBook Extended International Keyboard with function keys  */
#define gestaltPwrBkExtJISKbd 			21					/*  PowerBook Extended Japanese Keyboard with function keys 		 */
#define gestaltPwrBkExtADBKbd 			24					/*  PowerBook Extended Domestic Keyboard with function keys 		 */
#define gestaltPS2Keyboard 				27					/*  PS2 keyboard  */
#define gestaltPwrBkSubDomKbd 			28					/*  PowerBook Subnote Domestic Keyboard with function keys w/  inverted T 	 */
#define gestaltPwrBkSubISOKbd 			29					/*  PowerBook Subnote International Keyboard with function keys w/  inverted T 	 */
#define gestaltPwrBkSubJISKbd 			30					/*  PowerBook Subnote Japanese Keyboard with function keys w/ inverted T 		 */
#define gestaltPwrBkEKDomKbd 			195					/*  (0xC3) PowerBook Domestic Keyboard with Embedded Keypad, function keys & inverted T 	 */
#define gestaltPwrBkEKISOKbd 			196					/*  (0xC4) PowerBook International Keyboard with Embedded Keypad, function keys & inverted T 	 */
#define gestaltPwrBkEKJISKbd 			197					/*  (0xC5) PowerBook Japanese Keyboard with Embedded Keypad, function keys & inverted T 		 */

#define gestaltLowMemorySize 			'lmem'				/*  size of low memory area  */
#define gestaltLogicalRAMSize 			'lram'				/*  logical ram size  */
#define gestaltMachineType 				'mach'				/*  machine type  */
#define gestaltClassic 					1
#define gestaltMacXL 					2
#define gestaltMac512KE 				3
#define gestaltMacPlus 					4
#define gestaltMacSE 					5
#define gestaltMacII 					6
#define gestaltMacIIx 					7
#define gestaltMacIIcx 					8
#define gestaltMacSE030 				9
#define gestaltPortable 				10
#define gestaltMacIIci 					11
#define gestaltPowerMac8100_120 		12
#define gestaltMacIIfx 					13
#define gestaltMacClassic 				17
#define gestaltMacIIsi 					18
#define gestaltMacLC 					19
#define gestaltMacQuadra900 			20
#define gestaltPowerBook170 			21
#define gestaltMacQuadra700 			22
#define gestaltClassicII 				23
#define gestaltPowerBook100 			24
#define gestaltPowerBook140 			25
#define gestaltMacQuadra950 			26
#define gestaltMacLCIII 				27
#define gestaltPerforma450 				27
#define gestaltPowerBookDuo210 			29
#define gestaltMacCentris650 			30
#define gestaltPowerBookDuo230 			32
#define gestaltPowerBook180 			33
#define gestaltPowerBook160 			34
#define gestaltMacQuadra800 			35
#define gestaltMacQuadra650 			36
#define gestaltMacLCII 					37
#define gestaltPowerBookDuo250 			38
#define gestaltAWS9150_80 				39
#define gestaltPowerMac8100_110 		40
#define gestaltAWS8150_110 				40
#define gestaltPowerMac5200 			41
#define gestaltPowerMac5260 			41
#define gestaltPerforma5300 			41
#define gestaltPowerMac6200 			42
#define gestaltPerforma6300 			42
#define gestaltMacIIvi 					44
#define gestaltMacIIvm 					45
#define gestaltPerforma600 				45
#define gestaltPowerMac7100_80 			47
#define gestaltMacIIvx 					48
#define gestaltMacColorClassic 			49
#define gestaltPerforma250 				49
#define gestaltPowerBook165c 			50
#define gestaltMacCentris610 			52
#define gestaltMacQuadra610 			53
#define gestaltPowerBook145 			54
#define gestaltPowerMac8100_100 		55
#define gestaltMacLC520 				56
#define gestaltAWS9150_120 				57
#define gestaltPowerMac6400 			58
#define gestaltPerforma6400 			58
#define gestaltPerforma6360 			58
#define gestaltMacCentris660AV 			60
#define gestaltMacQuadra660AV 			60
#define gestaltPerforma46x 				62
#define gestaltPowerMac8100_80 			65
#define gestaltAWS8150_80 				65
#define gestaltPowerMac9500 			67
#define gestaltPowerMac9600 			67
#define gestaltPowerMac7500 			68
#define gestaltPowerMac7600 			68
#define gestaltPowerMac8500 			69
#define gestaltPowerMac8600 			69
#define gestaltAWS8550 					68
#define gestaltPowerBook180c 			71
#define gestaltPowerBook520 			72
#define gestaltPowerBook520c 			72
#define gestaltPowerBook540 			72
#define gestaltPowerBook540c 			72
#define gestaltPowerMac5400 			74
#define gestaltPowerMac6100_60 			75
#define gestaltAWS6150_60 				75
#define gestaltPowerBookDuo270c 		77
#define gestaltMacQuadra840AV 			78
#define gestaltPerforma550 				80
#define gestaltPowerBook165 			84
#define gestaltPowerBook190 			85
#define gestaltMacTV 					88
#define gestaltMacLC475 				89
#define gestaltPerforma47x 				89
#define gestaltMacLC575 				92
#define gestaltMacQuadra605 			94
#define gestaltMacQuadra630 			98
#define gestaltMacLC580 				99
#define gestaltPerforma580 				99
#define gestaltPowerMac6100_66 			100
#define gestaltAWS6150_66 				100
#define gestaltPowerBookDuo280 			102
#define gestaltPowerBookDuo280c 		103
#define gestaltPowerMacLC475 			104					/*  Mac LC 475 & PPC Processor Upgrade Card */
#define gestaltPowerMacPerforma47x 		104
#define gestaltPowerMacLC575 			105					/*  Mac LC 575 & PPC Processor Upgrade Card  */
#define gestaltPowerMacPerforma57x 		105
#define gestaltPowerMacQuadra630 		106					/*  Quadra 630 & PPC Processor Upgrade Card */
#define gestaltPowerMacLC630 			106					/*  Mac LC 630 & PPC Processor Upgrade Card */
#define gestaltPowerMacPerforma63x 		106					/*  Performa 63x & PPC Processor Upgrade Card */
#define gestaltPowerMac7200 			108
#define gestaltPowerMac7300 			109
#define gestaltPowerMac7100_66 			112
#define gestaltPowerBook150 			115
#define gestaltPowerMacQuadra700 		116					/*  Quadra 700 & Power PC Upgrade Card */
#define gestaltPowerMacQuadra900 		117					/*  Quadra 900 & Power PC Upgrade Card  */
#define gestaltPowerMacQuadra950 		118					/*  Quadra 950 & Power PC Upgrade Card  */
#define gestaltPowerMacCentris610 		119					/*  Centris 610 & Power PC Upgrade Card  */
#define gestaltPowerMacCentris650 		120					/*  Centris 650 & Power PC Upgrade Card  */
#define gestaltPowerMacQuadra610 		121					/*  Quadra 610 & Power PC Upgrade Card  */
#define gestaltPowerMacQuadra650 		122					/*  Quadra 650 & Power PC Upgrade Card  */
#define gestaltPowerMacQuadra800 		123					/*  Quadra 800 & Power PC Upgrade Card  */
#define gestaltPowerBookDuo2300 		124
#define gestaltPowerBook500PPCUpgrade 	126
#define gestaltPowerBook5300 			128
#define gestaltPowerBook1400 			310
#define gestaltPowerBook3400 			306
#define gestaltPowerBook2400 			307
#define gestaltPowerBookG3 				313
#define gestaltPowerMac5500 			512
#define gestalt20thAnniversary 			512
#define gestaltPowerMac6500 			513
#define gestaltPowerMac4400_160 		514					/*  slower machine has different machine ID */
#define gestaltPowerMac4400 			515

#define gestaltQuadra605 				94
#define gestaltQuadra610 				53
#define gestaltQuadra630 				98
#define gestaltQuadra650 				36
#define gestaltQuadra660AV 				60
#define gestaltQuadra700 				22
#define gestaltQuadra800 				35
#define gestaltQuadra840AV 				78
#define gestaltQuadra900 				20
#define gestaltQuadra950 				26

#define kMachineNameStrID 				(-16395)
#define gestaltSMPMailerVersion 		'malr'				/*  OCE StandardMail */
#define gestaltMediaBay 				'mbeh'				/*  media bay driver type  */
#define gestaltMBLegacy 				0					/*  media bay support in PCCard 2.0  */
#define gestaltMBSingleBay 				1					/*  single bay media bay driver  */
#define gestaltMBMultipleBays 			2					/*  multi-bay media bay driver  */

#define gestaltMessageMgrVersion 		'mess'				/*  GX Printing Message Manager Gestalt Selector  */
#define gestaltMachineIcon 				'micn'				/*  machine icon  */
#define gestaltMiscAttr 				'misc'				/*  miscellaneous attributes  */
#define gestaltScrollingThrottle 		0					/*  true if scrolling throttle on  */
#define gestaltSquareMenuBar 			2					/*  true if menu bar is square  */

#define gestaltMixedModeVersion 		'mixd'				/*  returns version of Mixed Mode  */
#define gestaltMixedModeAttr 			'mixd'				/*  returns Mixed Mode attributes  */
#define gestaltMixedModePowerPC 		0					/*  true if Mixed Mode supports PowerPC ABI calling conventions  */
#define gestaltPowerPCAware 			0					/*  old name for gestaltMixedModePowerPC  */
#define gestaltMixedModeCFM68K 			1					/*  true if Mixed Mode supports CFM-68K calling conventions  */
#define gestaltMixedModeCFM68KHasTrap 	2					/*  true if CFM-68K Mixed Mode implements _MixedModeDispatch (versions 1.0.1 and prior did not)  */
#define gestaltMixedModeCFM68KHasState 	3					/*  true if CFM-68K Mixed Mode exports Save/RestoreMixedModeState  */

#define gestaltQuickTimeConferencing 	'mtlk'				/*  returns QuickTime Conferencing version  */
#define gestaltMemoryMapAttr 			'mmap'				/*  Memory map type  */
#define gestaltMemoryMapSparse 			0					/*  Sparse memory is on  */

#define gestaltMMUType 					'mmu '				/*  mmu type  */
#define gestaltNoMMU 					0					/*  no MMU  */
#define gestaltAMU 						1					/*  address management unit  */
#define gestalt68851 					2					/*  68851 PMMU  */
#define gestalt68030MMU 				3					/*  68030 built-in MMU  */
#define gestalt68040MMU 				4					/*  68040 built-in MMU  */
#define gestaltEMMU1 					5					/*  Emulated MMU type 1   */

#define gestaltStdNBPAttr 				'nlup'				/*  standard nbp attributes  */
#define gestaltStdNBPPresent 			0
#define gestaltStdNBPSupportsAutoPosition  1				/*  StandardNBP takes (-1,-1) to mean alert position main screen  */

#define gestaltNotificationMgrAttr 		'nmgr'				/*  notification manager attributes  */
#define gestaltNotificationPresent 		0					/*  notification manager exists  */

#define gestaltNameRegistryVersion 		'nreg'				/*  NameRegistryLib version number, for System 7.5.2+ usage  */
#define gestaltNuBusSlotCount 			'nubs'				/*  count of logical NuBus slots present  */
#define gestaltOCEToolboxVersion 		'ocet'				/*  OCE Toolbox version  */
#define gestaltOCETB 					0x0102				/*  OCE Toolbox version 1.02  */
#define gestaltSFServer 				0x0100				/*  S&F Server version 1.0  */

#define gestaltOCEToolboxAttr 			'oceu'				/*  OCE Toolbox attributes  */
#define gestaltOCETBPresent 			0x01				/*  OCE toolbox is present, not running  */
#define gestaltOCETBAvailable 			0x02				/*  OCE toolbox is running and available  */
#define gestaltOCESFServerAvailable 	0x04				/*  S&F Server is running and available  */
#define gestaltOCETBNativeGlueAvailable  0x10				/*  Native PowerPC Glue routines are availible  */

#define gestaltOpenFirmwareInfo 		'opfw'				/*  Open Firmware info  */
#define gestaltOSAttr 					'os  '				/*  o/s attributes  */
#define gestaltSysZoneGrowable 			0					/*  system heap is growable  */
#define gestaltLaunchCanReturn 			1					/*  can return from launch  */
#define gestaltLaunchFullFileSpec 		2					/*  can launch from full file spec  */
#define gestaltLaunchControl 			3					/*  launch control support available  */
#define gestaltTempMemSupport 			4					/*  temp memory support  */
#define gestaltRealTempMemory 			5					/*  temp memory handles are real  */
#define gestaltTempMemTracked 			6					/*  temporary memory handles are tracked  */
#define gestaltIPCSupport 				7					/*  IPC support is present  */
#define gestaltSysDebuggerSupport 		8					/*  system debugger support is present  */

#define gestaltOSTable 					'ostt'				/*   OS trap table base   */
#define gestaltPCCard 					'pccd'				/* 	PC Card attributes */
#define gestaltCardServicesPresent 		0					/* 	PC Card 2.0 (68K) API is present */
#define gestaltPCCardFamilyPresent 		1					/* 	PC Card 3.x (PowerPC) API is present */
#define gestaltPCCardHasPowerControl 	2					/* 	PCCardSetPowerLevel is supported */
#define gestaltPCCardSupportsCardBus 	3					/* 	CardBus is supported */

#define gestaltProcClkSpeed 			'pclk'				/*  processor clock speed in hertz  */
#define gestaltPCXAttr 					'pcxg'				/*  PC Exchange attributes  */
#define gestaltPCXHas8and16BitFAT 		0					/*  PC Exchange supports both 8 and 16 bit FATs  */
#define gestaltPCXHasProDOS 			1					/*  PC Exchange supports ProDOS  */

#define gestaltLogicalPageSize 			'pgsz'				/*  logical page size  */
#define gestaltScreenCaptureMain 		'pic1'				/*  Zero, or vRefNum of disk to hold picture  */
#define gestaltScreenCaptureDir 		'pic2'				/*  Directory ID of folder to hold picture  */

#define gestaltGXPrintingMgrVersion 	'pmgr'				/*  QuickDraw GX Printing Manager Version */
#define gestaltPopupAttr 				'pop!'				/*  popup cdef attributes  */
#define gestaltPopupPresent 			0

#define gestaltPowerMgrAttr 			'powr'				/*  power manager attributes  */
#define gestaltPMgrExists 				0
#define gestaltPMgrCPUIdle 				1
#define gestaltPMgrSCC 					2
#define gestaltPMgrSound 				3
#define gestaltPMgrDispatchExists 		4
#define gestaltPMgrSupportsAVPowerStateAtSleepWake  5

#define gestaltPPCToolboxAttr 			'ppc '				/*  PPC toolbox attributes  */
#define gestaltPPCToolboxPresent 		0x0000				/*  PPC Toolbox is present  Requires PPCInit to be called  */
#define gestaltPPCSupportsRealTime 		0x1000				/*  PPC Supports real-time delivery  */
#define gestaltPPCSupportsIncoming 		0x0001				/*  PPC will deny incoming network requests  */
#define gestaltPPCSupportsOutGoing 		0x0002				/*  PPC will deny outgoing network requests  */

#define gestaltProcessorType 			'proc'				/*  processor type  */
#define gestalt68000 					1
#define gestalt68010 					2
#define gestalt68020 					3
#define gestalt68030 					4
#define gestalt68040 					5

#define gestaltSDPPromptVersion 		'prpv'				/*  OCE Standard Directory Panel */
#define gestaltParityAttr 				'prty'				/*  parity attributes  */
#define gestaltHasParityCapability 		0					/*  has ability to check parity  */
#define gestaltParityEnabled 			1					/*  parity checking enabled  */

#define gestaltQD3DVersion 				'q3v '				/*  Quickdraw 3D version in pack BCD */
#define gestaltQD3DViewer 				'q3vc'				/*  Quickdraw 3D viewer attributes */
#define gestaltQD3DViewerPresent 		0					/*  bit 0 set if QD3D Viewer is available */

#if OLDROUTINENAMES
#define gestaltQD3DViewerNotPresent 	0x00
#define gestaltQD3DViewerAvailable 		0x01

#endif
#define gestaltQuickdrawVersion 		'qd  '				/*  quickdraw version  */
#define gestaltOriginalQD 				0x0000				/*  original 1-bit QD  */
#define gestalt8BitQD 					0x0100				/*  8-bit color QD  */
#define gestalt32BitQD 					0x0200				/*  32-bit color QD  */
#define gestalt32BitQD11 				0x0201				/*  32-bit color QDv1.1  */
#define gestalt32BitQD12 				0x0220				/*  32-bit color QDv1.2  */
#define gestalt32BitQD13 				0x0230				/*  32-bit color QDv1.3  */

#define gestaltQD3D 					'qd3d'				/*  Quickdraw 3D attributes */
#define gestaltQD3DPresent 				0					/*  bit 0 set if QD3D available */

#if OLDROUTINENAMES
#define gestaltQD3DNotPresent 			0x00
#define gestaltQD3DAvailable 			0x01

#endif
#define gestaltGXVersion 				'qdgx'				/*  Overall QuickDraw GX Version */
#define gestaltQuickdrawFeatures 		'qdrw'				/*  quickdraw features  */
#define gestaltHasColor 				0					/*  color quickdraw present  */
#define gestaltHasDeepGWorlds 			1					/*  GWorlds can be deeper than 1-bit  */
#define gestaltHasDirectPixMaps 		2					/*  PixMaps can be direct (16 or 32 bit)  */
#define gestaltHasGrayishTextOr 		3					/*  supports text mode grayishTextOr  */
#define gestaltSupportsMirroring 		4					/*  Supports video mirroring via the Display Manager.  */

#define gestaltQDTextVersion 			'qdtx'				/*  QuickdrawText version  */
#define gestaltOriginalQDText 			0x0000				/*  up to and including 8.1  */
#define gestaltAllegroQDText 			0x0100				/*  starting with 8.2 (?)  */

#define gestaltQDTextFeatures 			'qdtf'				/*  QuickdrawText features  */
#define gestaltWSIISupportBit 			0					/*  bit 0: WSII support included  */
#define gestaltSbitFontsBit 			1					/*  sbit-only fonts supported  */
#define gestaltAntiAliasBit 			2					/*  capably of antialiased text  */
#define gestaltOFA2availableBit 		3					/*  OFA2 available  */

#define gestaltQuickTimeConferencingInfo  'qtci'			/*  returns pointer to QuickTime Conferencing information  */
#define gestaltQuickTimeVersion 		'qtim'				/*  returns version of QuickTime  */
#define gestaltQuickTime 				'qtim'				/*  gestaltQuickTime is old name for gestaltQuickTimeVersion  */

#define gestaltQuickTimeFeatures 		'qtrs'
#define gestaltPPCQuickTimeLibPresent 	0					/*  PowerPC QuickTime glue library is present  */

#define gestaltQTVRMgrAttr 				'qtvr'				/*  QuickTime VR attributes                                */
#define gestaltQTVRMgrPresent 			0					/*  QTVR API is present                                    */
#define gestaltQTVRObjMoviesPresent 	1					/*  QTVR runtime knows about object movies                 */
#define gestaltQTVRCylinderPanosPresent  2					/*  QTVR runtime knows about cylindrical panoramic movies  */

#define gestaltQTVRMgrVers 				'qtvv'				/*  QuickTime VR version                                   */
#define gestaltPhysicalRAMSize 			'ram '				/*  physical RAM size  */
#define gestaltRBVAddr 					'rbv '				/*  RBV base address   */
#define gestaltROMSize 					'rom '				/*  rom size  */
#define gestaltROMVersion 				'romv'				/*  rom version  */
#define gestaltResourceMgrAttr 			'rsrc'				/*  Resource Mgr attributes  */
#define gestaltPartialRsrcs 			0					/*  True if partial resources exist  */

#define gestaltRealtimeMgrAttr 			'rtmr'				/*  Realtime manager attributes			 */
#define gestaltRealtimeMgrPresent 		0					/*  true if the Realtime manager is present 	 */

#define gestaltSCCReadAddr 				'sccr'				/*  scc read base address   */
#define gestaltSCCWriteAddr 			'sccw'				/*  scc read base address   */
#define gestaltScrapMgrAttr 			'scra'				/*  Scrap Manager attributes  */
#define gestaltScrapMgrTranslationAware  0					/*  True if scrap manager is translation aware  */

#define gestaltScriptMgrVersion 		'scri'				/*  Script Manager version number      */
#define gestaltScriptCount 				'scr#'				/*  number of active script systems    */
#define gestaltSCSI 					'scsi'				/*  SCSI Manager attributes  */
#define gestaltAsyncSCSI 				0					/*  Supports Asynchronous SCSI  */
#define gestaltAsyncSCSIINROM 			1					/*  Async scsi is in ROM (available for booting)  */
#define gestaltSCSISlotBoot 			2					/*  ROM supports Slot-style PRAM for SCSI boots (PDM and later)  */
#define gestaltSCSIPollSIH 				3					/*  SCSI Manager will poll for interrupts if Secondary Interrupts are busy.  */

#define gestaltControlStripAttr 		'sdev'				/*  Control Strip attributes  */
#define gestaltControlStripExists 		0					/*  Control Strip is installed  */
#define gestaltControlStripVersionFixed  1					/*  Control Strip version Gestalt selector was fixed  */
#define gestaltControlStripUserFont 	2					/*  supports user-selectable font/size  */
#define gestaltControlStripUserHotKey 	3					/*  support user-selectable hot key to show/hide the window  */

#define gestaltSDPStandardDirectoryVersion  'sdvr'			/*  OCE Standard Directory Panel */
#define gestaltSerialAttr 				'ser '				/*  Serial attributes  */
#define gestaltHasGPIaToDCDa 			0					/*  GPIa connected to DCDa */
#define gestaltHasGPIaToRTxCa 			1					/*  GPIa connected to RTxCa clock input */
#define gestaltHasGPIbToDCDb 			2					/*  GPIb connected to DCDb  */

#define gestaltShutdownAttributes 		'shut'				/*  ShutDown Manager Attributes  */
#define gestaltShutdownHassdOnBootVolUnmount  0				/*  True if ShutDown Manager unmounts boot & VM volume at shutdown time.  */

#define gestaltNuBusConnectors 			'sltc'				/*  bitmap of NuBus connectors */
#define gestaltSlotAttr 				'slot'				/*  slot attributes   */
#define gestaltSlotMgrExists 			0					/*  true is slot mgr exists   */
#define gestaltNuBusPresent 			1					/*  NuBus slots are present   */
#define gestaltSESlotPresent 			2					/*  SE PDS slot present   */
#define gestaltSE30SlotPresent 			3					/*  SE/30 slot present   */
#define gestaltPortableSlotPresent 		4					/*  Portable’s slot present   */

#define gestaltFirstSlotNumber 			'slt1'				/*  returns first physical slot  */
#define gestaltSoundAttr 				'snd '				/*  sound attributes  */
#define gestaltStereoCapability 		0					/*  sound hardware has stereo capability  */
#define gestaltStereoMixing 			1					/*  stereo mixing on external speaker  */
#define gestaltSoundIOMgrPresent 		3					/*  The Sound I/O Manager is present  */
#define gestaltBuiltInSoundInput 		4					/*  built-in Sound Input hardware is present  */
#define gestaltHasSoundInputDevice 		5					/*  Sound Input device available  */
#define gestaltPlayAndRecord 			6					/*  built-in hardware can play and record simultaneously  */
#define gestalt16BitSoundIO 			7					/*  sound hardware can play and record 16-bit samples  */
#define gestaltStereoInput 				8					/*  sound hardware can record stereo  */
#define gestaltLineLevelInput 			9					/*  sound input port requires line level  */
															/*  the following bits are not defined prior to Sound Mgr 3.0  */
#define gestaltSndPlayDoubleBuffer 		10					/*  SndPlayDoubleBuffer available, set by Sound Mgr 3.0 and later  */
#define gestaltMultiChannels 			11					/*  multiple channel support, set by Sound Mgr 3.0 and later  */
#define gestalt16BitAudioSupport 		12					/*  16 bit audio data supported, set by Sound Mgr 3.0 and later  */

#define gestaltSplitOSAttr 				'spos'
#define gestaltSplitOSAware 			1					/*  the system includes the code to deal with a split os situation.  */
#define gestaltSplitOSEnablerVolumeIsDifferentFromBootVolume  2 /*  the active enabler is on a different volume than the system file.  */

#define gestaltSMPSPSendLetterVersion 	'spsl'				/*  OCE StandardMail */
#define gestaltSpeechRecognitionAttr 	'srta'				/*  speech recognition attributes  */
#define gestaltDesktopSpeechRecognition  1					/*  recognition thru the desktop microphone is available  */
#define gestaltTelephoneSpeechRecognition  2				/*  recognition thru the telephone is available  */

#define gestaltSpeechRecognitionVersion  'srtb'				/*  speech recognition version (0x0150 is the first version that fully supports the API)  */
#define gestaltSoftwareVendorCode 		'srad'				/*  Returns system software vendor information  */
#define gestaltSoftwareVendorApple 		'Appl'				/*  System software sold by Apple  */
#define gestaltSoftwareVendorLicensee 	'Lcns'				/*  System software sold by licensee  */

#define gestaltStandardFileAttr 		'stdf'				/*  Standard File attributes  */
#define gestaltStandardFile58 			0					/*  True if selectors 5-8 (StandardPutFile-CustomGetFile) are supported  */
#define gestaltStandardFileTranslationAware  1				/*  True if standard file is translation manager aware  */
#define gestaltStandardFileHasColorIcons  2					/*  True if standard file has 16x16 color icons  */
#define gestaltStandardFileUseGenericIcons  3				/*  Standard file LDEF to use only the system generic icons if true  */
#define gestaltStandardFileHasDynamicVolumeAllocation  4	/*  True if standard file supports more than 20 volumes  */

#define gestaltSysArchitecture 			'sysa'				/*  Native System Architecture  */
#define gestalt68k 						1					/*  Motorola MC68k architecture  */
#define gestaltPowerPC 					2					/*  IBM PowerPC architecture  */

#define gestaltSystemUpdateVersion 		'sysu'				/*  System Update version  */
#define gestaltSystemVersion 			'sysv'				/*  system version */
#define gestaltToolboxTable 			'tbtt'				/*   OS trap table base   */
#define gestaltTextEditVersion 			'te  '				/*  TextEdit version number  */
#define gestaltTE1 						1					/*  TextEdit in MacIIci ROM  */
#define gestaltTE2 						2					/*  TextEdit with 6.0.4 Script Systems on MacIIci (Script bug fixes for MacIIci)  */
#define gestaltTE3 						3					/*  TextEdit with 6.0.4 Script Systems all but MacIIci  */
#define gestaltTE4 						4					/*  TextEdit in System 7.0  */
#define gestaltTE5 						5					/*  TextWidthHook available in TextEdit  */

#define gestaltTEAttr 					'teat'				/*  TextEdit attributes  */
#define gestaltTEHasGetHiliteRgn 		0					/*  TextEdit has TEGetHiliteRgn  */
#define gestaltTESupportsInlineInput 	1					/*  TextEdit does Inline Input  */
#define gestaltTESupportsTextObjects 	2					/*  TextEdit does Text Objects  */
#define gestaltTEHasWhiteBackground 	3					/*  TextEdit supports overriding the TERec's background to white  */

#define gestaltTeleMgrAttr 				'tele'				/*  Telephone manager attributes  */
#define gestaltTeleMgrPresent 			0
#define gestaltTeleMgrPowerPCSupport 	1
#define gestaltTeleMgrSoundStreams 		2
#define gestaltTeleMgrAutoAnswer 		3
#define gestaltTeleMgrIndHandset 		4
#define gestaltTeleMgrSilenceDetect 	5
#define gestaltTeleMgrNewTELNewSupport 	6

#define gestaltTermMgrAttr 				'term'				/*  terminal mgr attributes  */
#define gestaltTermMgrPresent 			0
#define gestaltTermMgrErrorString 		2

#define gestaltThreadMgrAttr 			'thds'				/*  Thread Manager attributes  */
#define gestaltThreadMgrPresent 		0					/*  bit true if Thread Mgr is present  */
#define gestaltSpecificMatchSupport 	1					/*  bit true if Thread Mgr supports exact match creation option  */
#define gestaltThreadsLibraryPresent 	2					/*  bit true if Thread Mgr shared library is present  */

#define gestaltTimeMgrVersion 			'tmgr'				/*  time mgr version  */
#define gestaltStandardTimeMgr 			1					/*  standard time mgr is present  */
#define gestaltRevisedTimeMgr 			2					/*  revised time mgr is present  */
#define gestaltExtendedTimeMgr 			3					/*  extended time mgr is present  */

#define gestaltTSMTEVersion 			'tmTV'
#define gestaltTSMTE1 					0x0100
#define gestaltTSMTE15 					0x0150
#define gestaltTSMTE2 					0x0200

#define gestaltTSMTEAttr 				'tmTE'
#define gestaltTSMTEPresent 			0
#define gestaltTSMTE 					0					/*  gestaltTSMTE is old name for gestaltTSMTEPresent  */

#define gestaltALMAttr 					'trip'				/*  Settings Manager attributes (see also gestaltALMVers)  */
#define gestaltALMPresent 				0					/*  bit true if ALM is available  */
#define gestaltALMHasSFGroup 			1					/*  bit true if Put/Get/Merge Group calls are implmented  */
#define gestaltALMHasCFMSupport 		2					/*  bit true if CFM-based modules are supported  */
#define gestaltALMHasRescanNotifiers 	3					/*  bit true if Rescan notifications/events will be sent to clients  */

#define gestaltALMHasSFLocation 		1
#define gestaltTSMgrVersion 			'tsmv'				/*  Text Services Mgr version, if present  */
#define gestaltTSMgr15 					0x0150
#define gestaltTSMgr2 					0x0200

#define gestaltTSMgrAttr 				'tsma'				/*  Text Services Mgr attributes, if present  */
#define gestaltTSMDisplayMgrAwareBit 	0					/*  TSM knows about display manager  */
#define gestaltTSMdoesTSMTEBit 			1					/*  TSM has integrated TSMTE  */

#define gestaltSpeechAttr 				'ttsc'				/*  Speech Manager attributes  */
#define gestaltSpeechMgrPresent 		0					/*  bit set indicates that Speech Manager exists  */
#define gestaltSpeechHasPPCGlue 		1					/*  bit set indicates that native PPC glue for Speech Manager API exists  */

#define gestaltTVAttr 					'tv  '				/*  TV version  */
#define gestaltHasTVTuner 				0					/*  supports Philips FL1236F video tuner  */
#define gestaltHasSoundFader 			1					/*  supports Philips TEA6330 Sound Fader chip  */
#define gestaltHasHWClosedCaptioning 	2					/*  supports Philips SAA5252 Closed Captioning  */
#define gestaltHasIRRemote 				3					/*  supports CyclopsII Infra Red Remote control  */
#define gestaltHasVidDecoderScaler 		4					/*  supports Philips SAA7194 Video Decoder/Scaler  */
#define gestaltHasStereoDecoder 		5					/*  supports Sony SBX1637A-01 stereo decoder  */
#define gestaltHasSerialFader 			6					/*  has fader audio in serial with system audio  */
#define gestaltHasFMTuner 				7					/*  has FM Tuner from donnybrook card  */
#define gestaltHasSystemIRFunction 		8					/*  Infra Red button function is set up by system and not by Video Startup  */
#define gestaltIRDisabled 				9					/*  Infra Red remote is not disabled.  */
#define gestaltINeedIRPowerOffConfirm 	10					/*  Need IR power off confirm dialog.  */
#define gestaltHasZoomedVideo 			11					/*  Has Zoomed Video PC Card video input.  */

#define gestaltVersion 					'vers'				/*  gestalt version  */
#define gestaltValueImplementedVers 	5					/*  version of gestalt where gestaltValue is implemented.  */

#define gestaltVIA1Addr 				'via1'				/*  via 1 base address   */
#define gestaltVIA2Addr 				'via2'				/*  via 2 base address   */
#define gestaltVMAttr 					'vm  '				/*  virtual memory attributes  */
#define gestaltVMPresent 				0					/*  true if virtual memory is present  */
#define gestaltVMHasLockMemoryForOutput  1					/*  true if LockMemoryForOutput is available  */
#define gestaltVMFilemappingOn 			3					/*  true if filemapping is available  */
#define gestaltVMHasPagingControl 		4					/*  true if MakeMemoryResident, MakeMemoryNonResident, FlushMemory, and ReleaseMemoryData are available  */

#define gestaltALMVers 					'walk'				/*  Settings Manager version (see also gestaltALMAttr)  */
#define gestaltTranslationAttr 			'xlat'				/*  Translation Manager attributes  */
#define gestaltTranslationMgrExists 	0					/*  True if translation manager exists  */
#define gestaltTranslationMgrHintOrder 	1					/*  True if hint order reversal in effect  */
#define gestaltTranslationPPCAvail 		2
#define gestaltTranslationGetPathAPIAvail  3

#define gestaltExtToolboxTable 			'xttt'				/*  Extended Toolbox trap table base  */

#endif /* __GESTALT_R__ */

