/*
 	File:		Power.h
 
 	Contains:	Power Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc.  All rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __POWER__
#define __POWER__

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


enum {
																/* Bit positions for ModemByte */
	modemOnBit					= 0,
	ringWakeUpBit				= 2,
	modemInstalledBit			= 3,
	ringDetectBit				= 4,
	modemOnHookBit				= 5
};


enum {
																/* masks for ModemByte */
	modemOnMask					= 0x01,
	ringWakeUpMask				= 0x04,
	modemInstalledMask			= 0x08,
	ringDetectMask				= 0x10,
	modemOnHookMask				= 0x20
};


enum {
																/* bit positions for BatteryByte */
	chargerConnBit				= 0,
	hiChargeBit					= 1,
	chargeOverFlowBit			= 2,
	batteryDeadBit				= 3,
	batteryLowBit				= 4,
	connChangedBit				= 5
};


enum {
																/* masks for BatteryByte */
	chargerConnMask				= 0x01,
	hiChargeMask				= 0x02,
	chargeOverFlowMask			= 0x04,
	batteryDeadMask				= 0x08,
	batteryLowMask				= 0x10,
	connChangedMask				= 0x20
};


enum {
																/* bit positions for SoundMixerByte */
	MediaBaySndEnBit			= 0,
	PCISndEnBit					= 1,
	ZVSndEnBit					= 2,
	PCCardSndEnBit				= 3
};


enum {
																/* masks for SoundMixerByte */
	MediaBaySndEnMask			= 0x01,
	PCISndEnMask				= 0x02,
	ZVSndEnMask					= 0x04,
	PCCardSndEnMask				= 0x08
};


enum {
																/* commands to SleepQRec sleepQProc */
	sleepRequest				= 1,
	sleepDemand					= 2,
	sleepWakeUp					= 3,
	sleepRevoke					= 4,
	sleepUnlock					= 4,
	sleepDeny					= 5,
	sleepNow					= 6,
	dozeDemand					= 7,
	dozeWakeUp					= 8,
	dozeRequest					= 9
};


enum {
																/* SleepQRec.sleepQFlags */
	noCalls						= 1,
	noRequest					= 2,
	slpQType					= 16,
	sleepQType					= 16
};

/* bits in bitfield returned by PMFeatures */

enum {
	hasWakeupTimer				= 0,							/* 1=wakeup timer is supported								*/
	hasSharedModemPort			= 1,							/* 1=modem port shared by SCC and internal modem			*/
	hasProcessorCycling			= 2,							/* 1=processor cycling is supported							*/
	mustProcessorCycle			= 3,							/* 1=processor cycling should not be turned off				*/
	hasReducedSpeed				= 4,							/* 1=processor can be started up at reduced speed			*/
	dynamicSpeedChange			= 5,							/* 1=processor speed can be switched dynamically			*/
	hasSCSIDiskMode				= 6,							/* 1=SCSI Disk Mode is supported							*/
	canGetBatteryTime			= 7,							/* 1=battery time can be calculated							*/
	canWakeupOnRing				= 8,							/* 1=can wakeup when the modem detects a ring				*/
	hasDimmingSupport			= 9,							/* 1=has dimming support built in (DPMS standby by default)	*/
	hasStartupTimer				= 10,							/* 1=startup timer is supported								*/
	hasChargeNotification		= 11,							/* 1=client can determine of charge connect status change notifications available */
	hasDimSuspendSupport		= 12							/* 1=supports dimming LCD and CRT to DPMS suspend state		*/
};

/* bits in bitfield returned by GetIntModemInfo and set by SetIntModemState */

enum {
	hasInternalModem			= 0,							/* 1=internal modem installed						*/
	intModemRingDetect			= 1,							/* 1=internal modem has detected a ring				*/
	intModemOffHook				= 2,							/* 1=internal modem is off hook						*/
	intModemRingWakeEnb			= 3,							/* 1=wakeup on ring is enabled						*/
	extModemSelected			= 4,							/* 1=external modem selected						*/
	modemSetBit					= 15							/* 1=set bit, 0=clear bit (SetIntModemState)		*/
};

/* bits in BatteryInfo.flags 									*/
/* ("chargerConnected" doesn't mean the charger is plugged in)	*/

enum {
	batteryInstalled			= 7,							/* 1=battery is currently connected					*/
	batteryCharging				= 6,							/* 1=battery is being charged						*/
	chargerConnected			= 5								/* 1=charger is connected to the PowerBook			*/
};


enum {
	HDPwrQType					= 0x4844,						/* 'HD' hard disk spindown queue element type		*/
	PMgrStateQType				= 0x504D						/* 'PM' Power Manager state queue element type		*/
};

/* client notification bits in PMgrQueueElement.pmNotifyBits */

enum {
	pmSleepTimeoutChanged		= 0,
	pmSleepEnableChanged		= 1,
	pmHardDiskTimeoutChanged	= 2,
	pmHardDiskSpindownChanged	= 3,
	pmDimmingTimeoutChanged		= 4,
	pmDimmingEnableChanged		= 5,
	pmDiskModeAddressChanged	= 6,
	pmProcessorCyclingChanged	= 7,
	pmProcessorSpeedChanged		= 8,
	pmWakeupTimerChanged		= 9,
	pmStartupTimerChanged		= 10,
	pmHardDiskPowerRemovedbyUser = 11,
	pmChargeStatusChanged		= 12,
	pmPowerLevelChanged			= 13
};

/* System Activity Selectors */

enum {
	OverallAct					= 0,							/* general type of activity							*/
	UsrActivity					= 1,							/* user specific type of activity					*/
	NetActivity					= 2,							/* network specific activity						*/
	HDActivity					= 3								/* Hard Drive activity								*/
};

/* Storage Media sleep mode defines */

enum {
	kMediaModeOn				= 0,							/* Media active (Drive spinning and at full power)	*/
	kMediaModeStandBy			= 1,							/* Media standby (not implemented)	*/
	kMediaModeSuspend			= 2,							/* Media Idle (not implemented)	*/
	kMediaModeOff				= 3								/* Media Sleep (Drive not spinning and at min power, max recovery time)	*/
};


enum {
	kMediaPowerCSCode			= 70
};


/* definitions for HDQueueElement.hdFlags	*/

enum {
	kHDQueuePostBit				= 0,							/* 1 = call this routine on the second pass		*/
	kHDQueuePostMask			= (1 << kHDQueuePostBit)
};


struct ActivityInfo {
	short 							ActivityType;				/* Type of activity to be fetched.  Same as UpdateSystemActivity Selectors */
	unsigned long 					ActivityTime;				/* Time of last activity (in ticks) of specified type. */
};
typedef struct ActivityInfo				ActivityInfo;
/* information returned by GetScaledBatteryInfo */

struct BatteryInfo {
	UInt8 							flags;						/* misc flags (see below)							*/
	UInt8 							warningLevel;				/* scaled warning level (0-255)						*/
	UInt8 							reserved;					/* reserved for internal use						*/
	UInt8 							batteryLevel;				/* scaled battery level (0-255)						*/
};
typedef struct BatteryInfo				BatteryInfo;

typedef SInt8 							ModemByte;
typedef SInt8 							BatteryByte;
typedef SInt8 							SoundMixerByte;
typedef long 							PMResultCode;
typedef struct SleepQRec 				SleepQRec;
typedef SleepQRec *						SleepQRecPtr;
typedef struct HDQueueElement 			HDQueueElement;
typedef struct PMgrQueueElement 		PMgrQueueElement;
typedef CALLBACK_API( long , SleepQProcPtr )(long message, SleepQRecPtr qRecPtr);
/*
	WARNING: SleepQProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( void , HDSpindownProcPtr )(HDQueueElement *theElement);
typedef CALLBACK_API( void , PMgrStateChangeProcPtr )(PMgrQueueElement *theElement, long stateBits);
typedef REGISTER_UPP_TYPE(SleepQProcPtr) 						SleepQUPP;
typedef STACK_UPP_TYPE(HDSpindownProcPtr) 						HDSpindownUPP;
typedef STACK_UPP_TYPE(PMgrStateChangeProcPtr) 					PMgrStateChangeUPP;

struct SleepQRec {
	SleepQRecPtr 					sleepQLink;					/* pointer to next queue element				*/
	short 							sleepQType;					/* queue element type (must be SleepQType)		*/
	SleepQUPP 						sleepQProc;					/* pointer to sleep universal proc ptr			*/
	short 							sleepQFlags;				/* flags										*/
};


struct HDQueueElement {
	struct HDQueueElement *			hdQLink;					/* pointer to next queue element				*/
	short 							hdQType;					/* queue element type (must be HDPwrQType)		*/
	short 							hdFlags;					/* miscellaneous flags							*/
	HDSpindownUPP 					hdProc;						/* pointer to routine to call					*/
	long 							hdUser;						/* user-defined (variable storage, etc.)		*/
};


struct PMgrQueueElement {
	struct PMgrQueueElement *		pmQLink;					/* pointer to next queue element				*/
	short 							pmQType;					/* queue element type (must be PMgrStateQType)	*/
	short 							pmFlags;					/* miscellaneous flags							*/
	long 							pmNotifyBits;				/* bitmap of which changes to be notified for	*/
	PMgrStateChangeUPP 				pmProc;						/* pointer to routine to call					*/
	long 							pmUser;						/* user-defined (variable storage, etc.)		*/
};



struct BatteryTimeRec {
	unsigned long 					expectedBatteryTime;		/* estimated battery time remaining (seconds)	*/
	unsigned long 					minimumBatteryTime;			/* minimum battery time remaining (seconds)		*/
	unsigned long 					maximumBatteryTime;			/* maximum battery time remaining (seconds)		*/
	unsigned long 					timeUntilCharged;			/* time until battery is fully charged (seconds)*/
};
typedef struct BatteryTimeRec			BatteryTimeRec;


struct WakeupTime {
	unsigned long 					wakeTime;					/* wakeup time (same format as current time)		*/
	Boolean 						wakeEnabled;				/* 1=enable wakeup timer, 0=disable wakeup timer	*/
	SInt8 							filler;
};
typedef struct WakeupTime				WakeupTime;


struct StartupTime {
	unsigned long 					startTime;					/* startup time (same format as current time)		*/
	Boolean 						startEnabled;				/* 1=enable startup timer, 0=disable startup timer	*/
	SInt8 							filler;
};
typedef struct StartupTime				StartupTime;
EXTERN_API( OSErr )
DisableWUTime					(void);

EXTERN_API( OSErr )
SetWUTime						(long 					WUTime);

EXTERN_API( OSErr )
GetWUTime						(long *					WUTime,
								 Byte *					WUFlag);

EXTERN_API( OSErr )
BatteryStatus					(Byte *					Status,
								 Byte *					Power);

EXTERN_API( OSErr )
ModemStatus						(Byte *					Status);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 IdleUpdate
																							#endif
EXTERN_API( long )
IdleUpdate						(void)														ONEWORDINLINE(0xA285);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetCPUSpeed
																							#endif
EXTERN_API( long )
GetCPUSpeed						(void)														TWOWORDINLINE(0x70FF, 0xA485);

EXTERN_API( void )
EnableIdle						(void)														TWOWORDINLINE(0x7000, 0xA485);

EXTERN_API( void )
DisableIdle						(void)														TWOWORDINLINE(0x7001, 0xA485);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SleepQInstall(__A0)
																							#endif
EXTERN_API( void )
SleepQInstall					(SleepQRecPtr 			qRecPtr)							ONEWORDINLINE(0xA28A);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SleepQRemove(__A0)
																							#endif
EXTERN_API( void )
SleepQRemove					(SleepQRecPtr 			qRecPtr)							ONEWORDINLINE(0xA48A);

EXTERN_API( void )
AOn								(void)														TWOWORDINLINE(0x7004, 0xA685);

EXTERN_API( void )
AOnIgnoreModem					(void)														TWOWORDINLINE(0x7005, 0xA685);

EXTERN_API( void )
BOn								(void)														TWOWORDINLINE(0x7000, 0xA685);

EXTERN_API( void )
AOff							(void)														TWOWORDINLINE(0x7084, 0xA685);

EXTERN_API( void )
BOff							(void)														TWOWORDINLINE(0x7080, 0xA685);


/* Public Power Management API (NEW!) */

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PMSelectorCount
																							#endif
EXTERN_API( short )
PMSelectorCount					(void)														TWOWORDINLINE(0x7000, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PMFeatures
																							#endif
EXTERN_API( unsigned long )
PMFeatures						(void)														TWOWORDINLINE(0x7001, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetSleepTimeout
																							#endif
EXTERN_API( UInt8 )
GetSleepTimeout					(void)														TWOWORDINLINE(0x7002, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetSleepTimeout(__D0)
																							#endif
EXTERN_API( void )
SetSleepTimeout					(UInt8 					timeout)							FOURWORDINLINE(0x4840, 0x303C, 0x0003, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetHardDiskTimeout
																							#endif
EXTERN_API( UInt8 )
GetHardDiskTimeout				(void)														TWOWORDINLINE(0x7004, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetHardDiskTimeout(__D0)
																							#endif
EXTERN_API( void )
SetHardDiskTimeout				(UInt8 					timeout)							FOURWORDINLINE(0x4840, 0x303C, 0x0005, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 HardDiskPowered
																							#endif
EXTERN_API( Boolean )
HardDiskPowered					(void)														TWOWORDINLINE(0x7006, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SpinDownHardDisk
																							#endif
EXTERN_API( void )
SpinDownHardDisk				(void)														TWOWORDINLINE(0x7007, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 IsSpindownDisabled
																							#endif
EXTERN_API( Boolean )
IsSpindownDisabled				(void)														TWOWORDINLINE(0x7008, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetSpindownDisable(__D0)
																							#endif
EXTERN_API( void )
SetSpindownDisable				(Boolean 				setDisable)							FOURWORDINLINE(0x4840, 0x303C, 0x0009, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 HardDiskQInstall(__A0)
																							#endif
EXTERN_API( OSErr )
HardDiskQInstall				(HDQueueElement *		theElement)							TWOWORDINLINE(0x700A, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 HardDiskQRemove(__A0)
																							#endif
EXTERN_API( OSErr )
HardDiskQRemove					(HDQueueElement *		theElement)							TWOWORDINLINE(0x700B, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter GetScaledBatteryInfo(__D0, __A0)
																							#endif
EXTERN_API( void )
GetScaledBatteryInfo			(short 					whichBattery,
								 BatteryInfo *			theInfo)							FIVEWORDINLINE(0x4840, 0x303C, 0x000C, 0xA09E, 0x2080);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter AutoSleepControl(__D0)
																							#endif
EXTERN_API( void )
AutoSleepControl				(Boolean 				enableSleep)						FOURWORDINLINE(0x4840, 0x303C, 0x000D, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetIntModemInfo
																							#endif
EXTERN_API( unsigned long )
GetIntModemInfo					(void)														TWOWORDINLINE(0x700E, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetIntModemState(__D0)
																							#endif
EXTERN_API( void )
SetIntModemState				(short 					theState)							FOURWORDINLINE(0x4840, 0x303C, 0x000F, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MaximumProcessorSpeed
																							#endif
EXTERN_API( short )
MaximumProcessorSpeed			(void)														TWOWORDINLINE(0x7010, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 CurrentProcessorSpeed
																							#endif
EXTERN_API( short )
CurrentProcessorSpeed			(void)														TWOWORDINLINE(0x7011, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 FullProcessorSpeed
																							#endif
EXTERN_API( Boolean )
FullProcessorSpeed				(void)														TWOWORDINLINE(0x7012, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 SetProcessorSpeed(__D0)
																							#endif
EXTERN_API( Boolean )
SetProcessorSpeed				(Boolean 				fullSpeed)							FOURWORDINLINE(0x4840, 0x303C, 0x0013, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetSCSIDiskModeAddress
																							#endif
EXTERN_API( short )
GetSCSIDiskModeAddress			(void)														TWOWORDINLINE(0x7014, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetSCSIDiskModeAddress(__D0)
																							#endif
EXTERN_API( void )
SetSCSIDiskModeAddress			(short 					scsiAddress)						FOURWORDINLINE(0x4840, 0x303C, 0x0015, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter GetWakeupTimer(__A0)
																							#endif
EXTERN_API( void )
GetWakeupTimer					(WakeupTime *			theTime)							TWOWORDINLINE(0x7016, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetWakeupTimer(__A0)
																							#endif
EXTERN_API( void )
SetWakeupTimer					(WakeupTime *			theTime)							TWOWORDINLINE(0x7017, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 IsProcessorCyclingEnabled
																							#endif
EXTERN_API( Boolean )
IsProcessorCyclingEnabled		(void)														TWOWORDINLINE(0x7018, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter EnableProcessorCycling(__D0)
																							#endif
EXTERN_API( void )
EnableProcessorCycling			(Boolean 				enable)								FOURWORDINLINE(0x4840, 0x303C, 0x0019, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 BatteryCount
																							#endif
EXTERN_API( short )
BatteryCount					(void)														TWOWORDINLINE(0x701A, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetBatteryVoltage(__D0)
																							#endif
EXTERN_API( Fixed )
GetBatteryVoltage				(short 					whichBattery)						FOURWORDINLINE(0x4840, 0x303C, 0x001B, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter GetBatteryTimes(__D0, __A0)
																							#endif
EXTERN_API( void )
GetBatteryTimes					(short 					whichBattery,
								 BatteryTimeRec *		theTimes)							FOURWORDINLINE(0x4840, 0x303C, 0x001C, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetDimmingTimeout
																							#endif
EXTERN_API( UInt8 )
GetDimmingTimeout				(void)														TWOWORDINLINE(0x701D, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetDimmingTimeout(__D0)
																							#endif
EXTERN_API( void )
SetDimmingTimeout				(UInt8 					timeout)							FOURWORDINLINE(0x4840, 0x303C, 0x001E, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter DimmingControl(__D0)
																							#endif
EXTERN_API( void )
DimmingControl					(Boolean 				enableSleep)						FOURWORDINLINE(0x4840, 0x303C, 0x001F, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 IsDimmingControlDisabled
																							#endif
EXTERN_API( Boolean )
IsDimmingControlDisabled		(void)														TWOWORDINLINE(0x7020, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 IsAutoSlpControlDisabled
																							#endif
EXTERN_API( Boolean )
IsAutoSlpControlDisabled		(void)														TWOWORDINLINE(0x7021, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PMgrStateQInstall(__A0)
																							#endif
EXTERN_API( OSErr )
PMgrStateQInstall				(PMgrQueueElement *		theElement)							TWOWORDINLINE(0x7022, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PMgrStateQRemove(__A0)
																							#endif
EXTERN_API( OSErr )
PMgrStateQRemove				(PMgrQueueElement *		theElement)							TWOWORDINLINE(0x7023, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 UpdateSystemActivity(__D0)
																							#endif
EXTERN_API( OSErr )
UpdateSystemActivity			(UInt8 					activity)							FOURWORDINLINE(0x4840, 0x303C, 0x0024, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 DelaySystemIdle
																							#endif
EXTERN_API( OSErr )
DelaySystemIdle					(void)														TWOWORDINLINE(0x7025, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetStartupTimer(__A0)
																							#endif
EXTERN_API( OSErr )
GetStartupTimer					(StartupTime *			theTime)							TWOWORDINLINE(0x7026, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 SetStartupTimer(__A0)
																							#endif
EXTERN_API( OSErr )
SetStartupTimer					(StartupTime *			theTime)							TWOWORDINLINE(0x7027, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetLastActivity(__A0)
																							#endif
EXTERN_API( OSErr )
GetLastActivity					(ActivityInfo *			theActivity)						TWOWORDINLINE(0x7028, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetSoundMixerState(__A0)
																							#endif
EXTERN_API( OSErr )
GetSoundMixerState				(SoundMixerByte *		theSoundMixerByte)					TWOWORDINLINE(0x7029, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 SetSoundMixerState(__A0)
																							#endif
EXTERN_API( OSErr )
SetSoundMixerState				(SoundMixerByte *		theSoundMixerByte)					TWOWORDINLINE(0x702A, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetDimSuspendState
																							#endif
EXTERN_API( Boolean )
GetDimSuspendState				(void)														TWOWORDINLINE(0x702B, 0xA09E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetDimSuspendState(__D0)
																							#endif
EXTERN_API( void )
SetDimSuspendState				(Boolean 				dimSuspendState)					FOURWORDINLINE(0x4840, 0x303C, 0x002C, 0xA09E);

enum { uppSleepQProcInfo = 0x00131832 }; 						/* register 4_bytes:D0 Func(4_bytes:D0, 4_bytes:A0) */
enum { uppHDSpindownProcInfo = 0x000000C0 }; 					/* pascal no_return_value Func(4_bytes) */
enum { uppPMgrStateChangeProcInfo = 0x000003C0 }; 				/* pascal no_return_value Func(4_bytes, 4_bytes) */
#define NewSleepQProc(userRoutine) 								(SleepQUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSleepQProcInfo, GetCurrentArchitecture())
#define NewHDSpindownProc(userRoutine) 							(HDSpindownUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppHDSpindownProcInfo, GetCurrentArchitecture())
#define NewPMgrStateChangeProc(userRoutine) 					(PMgrStateChangeUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPMgrStateChangeProcInfo, GetCurrentArchitecture())
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter __D0 CallSleepQProc(__A1, __D0, __A0)
	long CallSleepQProc(SleepQUPP routine, long message, SleepQRecPtr qRecPtr) = 0x4E91;
#else
	#define CallSleepQProc(userRoutine, message, qRecPtr) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppSleepQProcInfo, (message), (qRecPtr))
#endif
#define CallHDSpindownProc(userRoutine, theElement) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppHDSpindownProcInfo, (theElement))
#define CallPMgrStateChangeProc(userRoutine, theElement, stateBits)  CALL_TWO_PARAMETER_UPP((userRoutine), uppPMgrStateChangeProcInfo, (theElement), (stateBits))


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

#endif /* __POWER__ */

