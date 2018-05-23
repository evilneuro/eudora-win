/*
 	File:		DriverGestalt.h
 
 	Contains:	Driver Gestalt interfaces
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __DRIVERGESTALT__
#define __DRIVERGESTALT__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
#ifndef __SCSI__
#include <SCSI.h>
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

/*__________________________________________________________________________________*/
/* The Driver Gestalt bit in the dCtlFlags */

enum {
	kbDriverGestaltEnable		= 2,
	kmDriverGestaltEnableMask	= (1 << kbDriverGestaltEnable)
};

/*__________________________________________________________________________________*/
/* Driver Gestalt related csCodes */

enum {
	kDriverGestaltCode			= 43,							/* various uses */
	kDriverConfigureCode		= 43,							/* various uses */
	kdgLowPowerMode				= 70,							/* Sets/Returns the current energy consumption level */
	kdgReturnDeviceID			= 120,							/* returns SCSI DevID in csParam[0] */
	kdgGetCDDeviceInfo			= 121							/* returns CDDeviceCharacteristics in csParam[0] */
};

/*__________________________________________________________________________________*/
/* Driver Gestalt selectors */

enum {
	kdgVersion					= FOUR_CHAR_CODE('vers'),		/* Version number of the driver in standard Apple format */
	kdgDeviceType				= FOUR_CHAR_CODE('devt'),		/* The type of device the driver is driving. */
	kdgInterface				= FOUR_CHAR_CODE('intf'),		/* The underlying interface that the driver is using (if any) */
	kdgSync						= FOUR_CHAR_CODE('sync'),		/* True if driver only behaves synchronously. */
	kdgBoot						= FOUR_CHAR_CODE('boot'),		/* value to place in PRAM for this drive (long) */
	kdgWide						= FOUR_CHAR_CODE('wide'),		/* True if driver supports ioWPosOffset */
	kdgPurge					= FOUR_CHAR_CODE('purg'),		/* Driver purge permission (True = purge; False = no purge) */
	kdgSupportsSwitching		= FOUR_CHAR_CODE('lpwr'),		/* True if driver supports power switching */
	kdgMin3VPower				= FOUR_CHAR_CODE('pmn3'),		/* Minimum 3.3V power consumption in microWatts */
	kdgMin5VPower				= FOUR_CHAR_CODE('pmn5'),		/* Minimum 5V power consumption in microWatts */
	kdgMax3VPower				= FOUR_CHAR_CODE('pmx3'),		/* Maximum 3.3V power consumption in microWatts */
	kdgMax5VPower				= FOUR_CHAR_CODE('pmx5'),		/* Maximum 5V power consumption in microWatts */
	kdgInHighPower				= FOUR_CHAR_CODE('psta'),		/* True if device is currently in high power mode */
	kdgSupportsPowerCtl			= FOUR_CHAR_CODE('psup'),		/* True if driver supports following five calls */
	kdgAPI						= FOUR_CHAR_CODE('dAPI'),		/* API support for PC Exchange */
	kdgEject					= FOUR_CHAR_CODE('ejec'),		/* Eject options for shutdown/restart (Shutdown Mgr) */
	kdgFlush					= FOUR_CHAR_CODE('flus'),		/* Determine if disk driver supports flush and if it needs a flush */
	kdgVMOptions				= FOUR_CHAR_CODE('vmop'),		/* Disk drive's Virtual Memory options */
	kdgMediaInfo				= FOUR_CHAR_CODE('minf')		/* return media specific information */
};

/*__________________________________________________________________________________*/
/* Driver Configure selectors */

enum {
	kdcFlush					= FOUR_CHAR_CODE('flus'),		/* Tell a disk driver to flush its cache and any hardware caches */
	kdcVMOptions				= FOUR_CHAR_CODE('vmop')		/* Change the disk driver's Virtual Memory options */
};

/*__________________________________________________________________________________*/
/* control parameter block for Driver Configure calls */

struct DriverConfigParam {
	QElemPtr 						qLink;
	short 							qType;
	short 							ioTrap;
	Ptr 							ioCmdAddr;
	ProcPtr 						ioCompletion;
	OSErr 							ioResult;
	StringPtr 						ioNamePtr;
	short 							ioVRefNum;
	short 							ioCRefNum;					/* refNum for I/O operation */
	short 							csCode;						/* == kDriverConfigureCode */
	OSType 							driverConfigureSelector;
	UInt32 							driverConfigureParameter;
};
typedef struct DriverConfigParam		DriverConfigParam;
/*__________________________________________________________________________________*/
/* status parameter block for Driver Gestalt calls */

struct DriverGestaltParam {
	QElemPtr 						qLink;
	short 							qType;
	short 							ioTrap;
	Ptr 							ioCmdAddr;
	ProcPtr 						ioCompletion;
	OSErr 							ioResult;
	StringPtr 						ioNamePtr;
	short 							ioVRefNum;
	short 							ioCRefNum;					/* refNum for I/O operation */
	short 							csCode;						/*	== kDriverGestaltCode */
	OSType 							driverGestaltSelector;		/* 'sync', 'vers', etc. */
	UInt32 							driverGestaltResponse;		/* Could be a pointer, bit field or other format */
	UInt32 							driverGestaltResponse1;		/* Could be a pointer, bit field or other format */
	UInt32 							driverGestaltResponse2;		/* Could be a pointer, bit field or other format */
	UInt32 							driverGestaltResponse3;		/* Could be a pointer, bit field or other format */
	UInt16 							driverGestaltfiller;		/* To pad out to the size of a controlPB */
};
typedef struct DriverGestaltParam		DriverGestaltParam;
/* Note that the various response definitions are overlays of the response fields above.
   For instance the deviceType response would be returned in driverGestaltResponse.
   The DriverGestaltPurgeResponse would be in driverGestaltResponse and driverGestaltResponse1
*/
/* GetDriverGestaltDevTResponse(gestaltPtr)->deviceType */
#define GetDriverGestaltDevTResponse(p) ((DriverGestaltDevTResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltIntfResponse(p) ((DriverGestaltIntfResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltPowerResponse(p) ((DriverGestaltPowerResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltSyncResponse(p) ((DriverGestaltSyncResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltBootResponse(p) ((DriverGestaltBootResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltBootResponse(p) ((DriverGestaltBootResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltPurgeResponse(p) ((DriverGestaltPurgeResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltEjectResponse(p) ((DriverGestaltEjectResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltVersionResponse(p) (((NumVersion*)(&((p)->driverGestaltResponse))))
#define GetDriverGestaltAPIResponse(p) ((DriverGestaltAPIResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltBooleanResponse(p) ((Boolean *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltFlushResponse(p) ((DriverGestaltFlushResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltVMOptionsResponse(p) ((DriverGestaltVMOptionsResponse *)(&((p)->driverGestaltResponse)))
#define GetDriverGestaltMediaInfoResponse(p) ((DriverGestaltMediaInfoResponse *)(&((p)->driverGestaltResponse)))
/*__________________________________________________________________________________*/
/* Device Types response */

struct DriverGestaltDevTResponse {
	OSType 							deviceType;
};
typedef struct DriverGestaltDevTResponse DriverGestaltDevTResponse;

enum {
	kdgDiskType					= FOUR_CHAR_CODE('disk'),		/* standard r/w disk drive */
	kdgTapeType					= FOUR_CHAR_CODE('tape'),		/* tape drive */
	kdgPrinterType				= FOUR_CHAR_CODE('prnt'),		/* printer */
	kdgProcessorType			= FOUR_CHAR_CODE('proc'),		/* processor */
	kdgWormType					= FOUR_CHAR_CODE('worm'),		/* write-once */
	kdgCDType					= FOUR_CHAR_CODE('cdrm'),		/* cd-rom drive */
	kdgFloppyType				= FOUR_CHAR_CODE('flop'),		/* floppy disk drive */
	kdgScannerType				= FOUR_CHAR_CODE('scan'),		/* scanner */
	kdgFileType					= FOUR_CHAR_CODE('file'),		/* Logical Partition type based on a file (Drive Container) */
	kdgRemovableType			= FOUR_CHAR_CODE('rdsk')		/* A removable media hard disk drive ie. Syquest, Bernioulli */
};

/*__________________________________________________________________________________*/
/* Device Interfaces response */

struct DriverGestaltIntfResponse {
	OSType 							interfaceType;
};
typedef struct DriverGestaltIntfResponse DriverGestaltIntfResponse;

enum {
	kdgScsiIntf					= FOUR_CHAR_CODE('scsi'),
	kdgPcmciaIntf				= FOUR_CHAR_CODE('pcmc'),
	kdgATAIntf					= FOUR_CHAR_CODE('ata '),
	kdgFireWireIntf				= FOUR_CHAR_CODE('fire'),
	kdgExtBus					= FOUR_CHAR_CODE('card')
};

/*__________________________________________________________________________________*/
/* Power Saving */

struct DriverGestaltPowerResponse {
	unsigned long 					powerValue;					/* Power consumed in µWatts */
};
typedef struct DriverGestaltPowerResponse DriverGestaltPowerResponse;
/*__________________________________________________________________________________*/
/* Disk Specific */

struct DriverGestaltSyncResponse {
	Boolean 						behavesSynchronously;
	UInt8 							pad[3];
};
typedef struct DriverGestaltSyncResponse DriverGestaltSyncResponse;

struct DriverGestaltBootResponse {
	UInt8 							extDev;						/*  Packed target (upper 5 bits) LUN (lower 3 bits) */
	UInt8 							partition;					/*  Unused */
	UInt8 							SIMSlot;					/*  Slot */
	UInt8 							SIMsRSRC;					/*  sRsrcID */
};
typedef struct DriverGestaltBootResponse DriverGestaltBootResponse;

struct DriverGestaltAPIResponse {
	short 							partitionCmds;				/* if bit 0 is nonzero, supports partition control and status calls */
																/*	 	prohibitMounting (control, kProhibitMounting) */
																/* 		partitionToVRef (status, kGetPartitionStatus) */
																/* 		getPartitionInfo (status, kGetPartInfo) */
	short 							unused1;					/* all the unused fields should be zero */
	short 							unused2;
	short 							unused3;
	short 							unused4;
	short 							unused5;
	short 							unused6;
	short 							unused7;
	short 							unused8;
	short 							unused9;
	short 							unused10;
};
typedef struct DriverGestaltAPIResponse	DriverGestaltAPIResponse;

struct DriverGestaltFlushResponse {
	Boolean 						canFlush;					/* Return true if driver supports the */
																/* kdcFlush Driver Configure _Control call */
	Boolean 						needsFlush;					/* Return true if driver/device has data cached */
																/* and needs to be flushed when the disk volume */
																/* is flushed by the File Manager */
	UInt8 							pad[2];
};
typedef struct DriverGestaltFlushResponse DriverGestaltFlushResponse;
/* Flags for purge permissions */

enum {
	kbCloseOk					= 0,							/* Ok to call Close */
	kbRemoveOk					= 1,							/* Ok to call RemoveDrvr */
	kbPurgeOk					= 2,							/* Ok to call DisposePtr */
	kmNoCloseNoPurge			= 0,
	kmOkCloseNoPurge			= (1 << kbCloseOk) + (1 << kbRemoveOk),
	kmOkCloseOkPurge			= (1 << kbCloseOk) + (1 << kbRemoveOk) + (1 << kbPurgeOk)
};

/* Driver purge permission structure */

struct DriverGestaltPurgeResponse {
	UInt16 							purgePermission;			/* 0 = Do not change the state of the driver */
																/* 3 = Do Close() and DrvrRemove() this driver */
																/* but don't deallocate driver code */
																/* 7 = Do Close(), DrvrRemove(), and DisposePtr() */
	UInt16 							purgeReserved;
	Ptr 							purgeDriverPointer;			/* pointer to the start of the driver block (valid */
																/* only of DisposePtr permission is given */
};
typedef struct DriverGestaltPurgeResponse DriverGestaltPurgeResponse;

struct DriverGestaltEjectResponse {
	UInt32 							ejectFeatures;				/*  */
};
typedef struct DriverGestaltEjectResponse DriverGestaltEjectResponse;
/* Flags for Ejection Features field */

enum {
	kRestartDontEject			= 0,							/* Dont Want eject during Restart */
	kShutDownDontEject			= 1,							/* Dont Want eject during Shutdown */
	kRestartDontEject_Mask		= 1 << kRestartDontEject,
	kShutDownDontEject_Mask		= 1 << kShutDownDontEject
};

/*
	The DriverGestaltVMOptionsResponse is returned by a disk driver in response to a
	kdgVMOptions Driver Gestalt request. This allows a disk driver to tell VM a few
	things about a disk drive. For example:
	
	¥ A drive that should never be in the page fault path should return kAllowVMNoneMask.
	  Examples of this are drives that have manual eject buttons that are not disabled by
	  software, drives with very slow throughput, or drives that depend on
	  a network connection.
	¥ A drive that should never be written to but is safe for read-only file mapping
	  should return kAllowVMReadOnlyMask. Examples of this are WORM drives where each write
	  eats write-once space on the disk and CD-ROM drives which are read-only media.
	¥ A drive that should allow VM to create its main backing store file should return
	  kAllowVMReadWriteMask. Examples of this are fast read/write drives that don't allow
	  manual eject and don't use a network connection.
	
	A disk driver must look at the ioVRefNum field of the DriverGestaltParam to determine
	what disk drive this call is for. This is a per-drive call, not a per-driver call.
	
	The only three valid responses to kdgVMOptions at this time are kAllowVMNoneMask,
	kAllowVMReadOnlyMask, and kAllowVMReadWriteMask (i.e., setting only kAllowVMWriteBit
	is not valid).
	
	Important: All bits not defined here are reserved and should be set to zero until
	they are defined for a specific purpose.
	
	The kdcVMOptions Driver Configure _Control call provides the ability to change a driver's
	response to kdgVMOptions Driver Gestalt requests. A driver should return controlErr if
	it doesn't want to provide the ability to change the kdgVMOptions response. If a driver
	supports the kdcVMOptions Driver Configure _Control call, but is asked to set an option bit
	that it doesn't support (for example, if a read-only device is asked to set the kAllowVMWriteBit),
	it should return paramErr.
*/

struct DriverGestaltVMOptionsResponse {
	UInt32 							vmOptions;
};
typedef struct DriverGestaltVMOptionsResponse DriverGestaltVMOptionsResponse;
/* Bits and masks for DriverGestaltVMOptionsResponse.vmOptions field */

enum {
	kAllowVMReadBit				= 0,							/* Allow VM to use this drive for read access */
	kAllowVMWriteBit			= 1,							/* Allow VM to use this drive for write access */
	kAllowVMNoneMask			= 0,
	kAllowVMReadOnlyMask		= 1 << kAllowVMReadBit,
	kAllowVMReadWriteMask		= (1 << kAllowVMReadBit) + (1 << kAllowVMWriteBit)
};

/*
	The DriverGestaltMediaInfoResponse is returned by a disk driver in response to a
	kdgMediaInfo DriverGestalt request. This allows a disk driver to tell callers the
	physical block size, the number of blocks that are of that size, and the media type
	for a given device.
	
	A disk driver must look at the ioVRefNum field of the DriverGestaltParam to determine
	what disk drive this call is for. This is a per-drive call, not a per-driver call.
	
	On drives that support ejectable media, the response can change depending on what
	media is currently in the drive.
*/

struct DriverGestaltMediaInfoResponse {
	UInt32 							numberBlocks;				/* number of blocks */
	UInt32 							blockSize;					/* physical size of blocks */
	SInt16 							mediaType;					/* media type identifier */
};
typedef struct DriverGestaltMediaInfoResponse DriverGestaltMediaInfoResponse;
/* DriverGestaltMediaInfoResponse.mediaType constants */

enum {
	kMediaTypeUnknown			= 128,							/* media type is unknown */
	kMediaTypeCDROM				= 129,							/* media type is a CD-ROM */
	kMediaTypeDVDROM			= 130,							/* media type is a DVD-ROM */
	kMediaTypeNoMedia			= -1							/* no media is present */
};

/*__________________________________________________________________________________*/
/* CD-ROM Specific */
/* The CDDeviceCharacteristics result is returned in csParam[0] and csParam[1] of a 
   standard CntrlParam parameter block called with csCode kdgGetCDDeviceInfo.
*/

struct CDDeviceCharacteristics {
	UInt8 							speedMajor;					/* High byte of fixed point number containing drive speed */
	UInt8 							speedMinor;					/* Low byte of "" CD 300 == 2.2, CD_SC == 1.0 etc. */
	UInt16 							cdFeatures;					/* Flags field for features and transport type of this CD-ROM */
};
typedef struct CDDeviceCharacteristics	CDDeviceCharacteristics;

enum {
	cdFeatureFlagsMask			= 0xFFFC,						/* The Flags are in the first 14 bits of the cdFeatures field */
	cdTransportMask				= 0x0003						/* The transport type is in the last 2 bits of the cdFeatures field */
};


/* Flags for CD Features field */

enum {
	cdMute						= 0,							/* The following flags have the same bit number */
	cdLeftToChannel				= 1,							/* as the Audio Mode they represent.  Don't change */
	cdRightToChannel			= 2,							/* them without changing dControl.c */
	cdLeftPlusRight				= 3,							/* Reserve some space for new audio mixing features (4-7) */
	cdSCSI_2					= 8,							/* Supports SCSI2 CD Command Set */
	cdStereoVolume				= 9,							/* Can support two different volumes (1 on each channel) */
	cdDisconnect				= 10,							/* Drive supports disconnect/reconnect */
	cdWriteOnce					= 11,							/* Drive is a write/once (CD-R?) type drive */
	cdMute_Mask					= 1 << cdMute,
	cdLeftToChannel_Mask		= 1 << cdLeftToChannel,
	cdRightToChannel_Mask		= 1 << cdRightToChannel,
	cdLeftPlusRight_Mask		= 1 << cdLeftPlusRight,
	cdSCSI_2_Mask				= 1 << cdSCSI_2,
	cdStereoVolume_Mask			= 1 << cdStereoVolume,
	cdDisconnect_Mask			= 1 << cdDisconnect,
	cdWriteOnce_Mask			= 1 << cdWriteOnce
};

/* Transport types */

enum {
	cdCaddy						= 0,							/* CD_SC,CD_SC_PLUS,CD-300 etc. */
	cdTray						= 1,							/* CD_300_PLUS etc. */
	cdLid						= 2								/* Power CD - eg no eject mechanism */
};

/* the following are used by PC Exchange (and Apple DOS/PC Compatibility Card)*/

/* Control Codes*/

enum {
	kRegisterPartition			= 50,							/* PCX needs a new Drive (for a non-macintosh partition found on the disk)*/
	OLD_REGISTER_PARTITION		= 301,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card*/
	THE_DRIVE					= 0,							/* DrvQElPtr for the partition to register*/
	THE_PHYS_START				= 1,							/* The start of the partition in logical blocks*/
	THE_PHYS_SIZE				= 2,							/* The size of the partition in logical blocks*/
	kGetADrive					= 51,							/* control call to ask the driver to create a drive*/
	OLD_GET_A_DRIVE				= 302,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card*/
	THE_VAR_QUEL				= 0,							/* a VAR parameter for the returned DrvQElPtr*/
	kProhibitMounting			= 52,							/* Dont allow mounting of the following drives*/
	kOldProhibitMounting		= 2100,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card*/
	kProhibitDevice				= 0,							/* CS Param 0 and 1 (partInfoRecPtr)*/
	kIsContainerMounted			= 53,
	kOldIsContainerMounted		= 2201,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card			*/
	kContainerVRef				= 0,							/* CS Param 0 and 1 (VRefNum)*/
	kContainerParID				= 1,							/* CS Param 2 and 3 (Parent ID)*/
	kContainerName				= 2,							/* CS Param 4 and 5 (File Name)*/
	kContainerResponse			= 3,							/* CS Param 6 and 7 (VAR pointer to short result)*/
	kMountVolumeImg				= 54,
	OLD_MOUNT_VOLUME_IMG		= 2000,
	MV_HOST_VREFNUM				= 0,
	MV_HOST_PAR_ID				= 1,
	MV_HOST_NAME				= 2,
	MV_REQ_PERM					= 3
};

/* Status Codes*/


enum {
	kGetPartitionStatus			= 50,							/* what is the status of this partition?*/
	kOldGetPartitionStatus		= 2200,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card*/
	kDeviceToQuery				= 0,							/* CS Param 0 and 1 (partInfoRecPtr)*/
	kDeviceResponse				= 1,							/* CS Param 2 and 3 (VAR pointer to short result)*/
	kGetPartInfo				= 51,							/* Get a partition info record based on the provided vrefnum*/
	kOldGetPartInfo				= 2300,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card*/
	kPartInfoResponse			= 0,							/* var parameter (pointer to partInfoRec) CSParam [0-1]*/
	kGetContainerAlias			= 52,							/* Get the alias that describes the file this drive was mounted from.*/
	kOldGetContainerAlias		= 2400,							/* left in for compatibility with shipping Apple DOS/PC Compatibility Card*/
	kGetAliasResponse			= 0								/*	var parameter (pointer to a Handle) CSParam [0-1]*/
};

/* the result codes to come from the driver interface */


enum {
	DRIVER_NOT_INSTALLED		= -1,
	DRIVER_BUSY					= -2,
	CANT_MOUNT_WITHIN_THIS_FS	= -3,							/* can only mount container within residing on HFS volume*/
	VOLUME_ALREADY_MOUNTED		= -4							/* Already Mounted*/
};

/* requisite structures for PCX control and status calls*/


enum {
	kMaxProhibted				= 2								/* the max number of volumes the PC can possibly have mounted*/
};

/* GestaltSelector for Finding Driver information*/


enum {
	kGetDriverInfo				= FOUR_CHAR_CODE('vdrc')
};


enum {
	VerifyCmd					= 5,
	FormatCmd					= 6,
	EjectCmd					= 7
};

/* Partition information passed back and forth between PCX and the driver*/

struct partInfoRec {
	DeviceIdent 					SCSIID;						/* DeviceIdent for the device*/
	UInt32 							physPartitionLoc;			/* physical block number of beginning of partition*/
	UInt32 							partitionNumber;			/* the partition number of this partition*/
};
typedef struct partInfoRec				partInfoRec;
typedef partInfoRec *					partInfoRecPtr;

struct vPartInfoRec {
	UInt8 							VPRTVers;					/* Virtual partition version number*/
	UInt8 							VPRTType;					/* virtual partition type (DOS, HFS, etc)*/
	SInt16 							drvrRefNum;					/* Driver Reference number of partition driver*/
};
typedef struct vPartInfoRec				vPartInfoRec;
typedef vPartInfoRec *					vPartInfoRecPtr;
/* Information related to DOS partitions*/

enum {
	kDOSSigLow					= 0x01FE,						/* offset into boot block for DOS signature*/
	kDOSSigHi					= 0x01FF,						/* offset into boot block for DOS signature*/
	kDOSSigValLo				= 0x55,							/* DOS signature value in low byte*/
	kDOSSigValHi				= 0xAA							/* DOS signature value in high byte*/
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

#endif /* __DRIVERGESTALT__ */

