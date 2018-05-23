/*
 	File:		MacMemory.h
 
 	Contains:	Memory Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __MACMEMORY__
#define __MACMEMORY__


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
	maxSize						= 0x00800000					/*Max data block size is 8 megabytes*/
};


enum {
	defaultPhysicalEntryCount	= 8
};


enum {
																/* values returned from the GetPageState function */
	kPageInMemory				= 0,
	kPageOnDisk					= 1,
	kNotPaged					= 2
};


enum {
																/* masks for Zone->heapType field */
	k32BitHeap					= 1,							/* valid in all Memory Managers */
	kNewStyleHeap				= 2,							/* true if new Heap Manager is present */
	kNewDebugHeap				= 4								/* true if new Heap Manager is running in debug mode on this heap */
};

/* Note: The type "Size" moved to Types.h */


typedef CALLBACK_API( long , GrowZoneProcPtr )(Size cbNeeded);
typedef CALLBACK_API( void , PurgeProcPtr )(Handle blockToPurge);
typedef CALLBACK_API( void , UserFnProcPtr )(void *parameter);
/*
	WARNING: UserFnProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef STACK_UPP_TYPE(GrowZoneProcPtr) 						GrowZoneUPP;
typedef STACK_UPP_TYPE(PurgeProcPtr) 							PurgeUPP;
typedef REGISTER_UPP_TYPE(UserFnProcPtr) 						UserFnUPP;

struct Zone {
	Ptr 							bkLim;
	Ptr 							purgePtr;
	Ptr 							hFstFree;
	long 							zcbFree;
	GrowZoneUPP 					gzProc;
	short 							moreMast;
	short 							flags;
	short 							cntRel;
	short 							maxRel;
	short 							cntNRel;
	SInt8 							heapType;					/* previously "maxNRel", now holds flags (e.g. k32BitHeap)*/
	SInt8 							unused;
	short 							cntEmpty;
	short 							cntHandles;
	long 							minCBFree;
	PurgeUPP 						purgeProc;
	Ptr 							sparePtr;
	Ptr 							allocPtr;
	short 							heapData;
};
typedef struct Zone						Zone;

typedef Zone *							THz;

struct MemoryBlock {
	void *							address;
	unsigned long 					count;
};
typedef struct MemoryBlock				MemoryBlock;

struct LogicalToPhysicalTable {
	MemoryBlock 					logical;
	MemoryBlock 					physical[8];
};
typedef struct LogicalToPhysicalTable	LogicalToPhysicalTable;

typedef short 							PageState;
typedef short 							StatusRegisterContents;
enum { uppGrowZoneProcInfo = 0x000000F0 }; 						/* pascal 4_bytes Func(4_bytes) */
enum { uppPurgeProcInfo = 0x000000C0 }; 						/* pascal no_return_value Func(4_bytes) */
enum { uppUserFnProcInfo = 0x00009802 }; 						/* register no_return_value Func(4_bytes:A0) */
#define NewGrowZoneProc(userRoutine) 							(GrowZoneUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppGrowZoneProcInfo, GetCurrentArchitecture())
#define NewPurgeProc(userRoutine) 								(PurgeUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppPurgeProcInfo, GetCurrentArchitecture())
#define NewUserFnProc(userRoutine) 								(UserFnUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppUserFnProcInfo, GetCurrentArchitecture())
#define CallGrowZoneProc(userRoutine, cbNeeded) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppGrowZoneProcInfo, (cbNeeded))
#define CallPurgeProc(userRoutine, blockToPurge) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppPurgeProcInfo, (blockToPurge))
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter CallUserFnProc(__A1, __A0)
	void CallUserFnProc(UserFnUPP routine, void * parameter) = 0x4E91;
#else
	#define CallUserFnProc(userRoutine, parameter) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppUserFnProcInfo, (parameter))
#endif


EXTERN_API( Ptr )
GetApplLimit					(void)														TWOWORDINLINE(0x2EB8, 0x0130);

EXTERN_API( THz )
SystemZone						(void)														TWOWORDINLINE(0x2EB8, 0x02A6);

EXTERN_API( THz )
ApplicationZone					(void)														TWOWORDINLINE(0x2EB8, 0x02AA);

EXTERN_API( Handle )
GZSaveHnd						(void)														TWOWORDINLINE(0x2EB8, 0x0328);

EXTERN_API( Ptr )
TopMem							(void)														TWOWORDINLINE(0x2EB8, 0x0108);

EXTERN_API( OSErr )
MemError						(void)														TWOWORDINLINE(0x3EB8, 0x0220);



																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 GetZone
																							#endif
EXTERN_API( THz )
GetZone							(void)														ONEWORDINLINE(0xA11A);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewHandle(__D0)
																							#endif
EXTERN_API( Handle )
NewHandle						(Size 					byteCount)							ONEWORDINLINE(0xA122);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewHandleSys(__D0)
																							#endif
EXTERN_API( Handle )
NewHandleSys					(Size 					byteCount)							ONEWORDINLINE(0xA522);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewHandleClear(__D0)
																							#endif
EXTERN_API( Handle )
NewHandleClear					(Size 					byteCount)							ONEWORDINLINE(0xA322);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewHandleSysClear(__D0)
																							#endif
EXTERN_API( Handle )
NewHandleSysClear				(Size 					byteCount)							ONEWORDINLINE(0xA722);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 HandleZone(__A0)
																							#endif
EXTERN_API( THz )
HandleZone						(Handle 				h)									ONEWORDINLINE(0xA126);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 RecoverHandle(__A0)
																							#endif
EXTERN_API( Handle )
RecoverHandle					(Ptr 					p)									ONEWORDINLINE(0xA128);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 RecoverHandleSys(__A0)
																							#endif
EXTERN_API( Handle )
RecoverHandleSys				(Ptr 					p)									ONEWORDINLINE(0xA528);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewPtr(__D0)
																							#endif
EXTERN_API( Ptr )
NewPtr							(Size 					byteCount)							ONEWORDINLINE(0xA11E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewPtrSys(__D0)
																							#endif
EXTERN_API( Ptr )
NewPtrSys						(Size 					byteCount)							ONEWORDINLINE(0xA51E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewPtrClear(__D0)
																							#endif
EXTERN_API( Ptr )
NewPtrClear						(Size 					byteCount)							ONEWORDINLINE(0xA31E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewPtrSysClear(__D0)
																							#endif
EXTERN_API( Ptr )
NewPtrSysClear					(Size 					byteCount)							ONEWORDINLINE(0xA71E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 PtrZone(__A0)
																							#endif
EXTERN_API( THz )
PtrZone							(Ptr 					p)									ONEWORDINLINE(0xA148);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MaxBlock
																							#endif
EXTERN_API( long )
MaxBlock						(void)														ONEWORDINLINE(0xA061);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MaxBlockSys
																							#endif
EXTERN_API( long )
MaxBlockSys						(void)														ONEWORDINLINE(0xA461);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 StackSpace
																							#endif
EXTERN_API( long )
StackSpace						(void)														ONEWORDINLINE(0xA065);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewEmptyHandle
																							#endif
EXTERN_API( Handle )
NewEmptyHandle					(void)														ONEWORDINLINE(0xA166);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 NewEmptyHandleSys
																							#endif
EXTERN_API( Handle )
NewEmptyHandleSys				(void)														ONEWORDINLINE(0xA566);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HLock(__A0)
																							#endif
EXTERN_API( void )
HLock							(Handle 				h)									ONEWORDINLINE(0xA029);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HUnlock(__A0)
																							#endif
EXTERN_API( void )
HUnlock							(Handle 				h)									ONEWORDINLINE(0xA02A);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HPurge(__A0)
																							#endif
EXTERN_API( void )
HPurge							(Handle 				h)									ONEWORDINLINE(0xA049);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HNoPurge(__A0)
																							#endif
EXTERN_API( void )
HNoPurge						(Handle 				h)									ONEWORDINLINE(0xA04A);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HLockHi(__A0)
																							#endif
EXTERN_API( void )
HLockHi							(Handle 				h)									TWOWORDINLINE(0xA064, 0xA029);

EXTERN_API( Handle )
TempNewHandle					(Size 					logicalSize,
								 OSErr *				resultCode)							THREEWORDINLINE(0x3F3C, 0x001D, 0xA88F);

EXTERN_API( Size )
TempMaxMem						(Size *					grow)								THREEWORDINLINE(0x3F3C, 0x0015, 0xA88F);

EXTERN_API( long )
TempFreeMem						(void)														THREEWORDINLINE(0x3F3C, 0x0018, 0xA88F);

EXTERN_API( void )
InitZone						(GrowZoneUPP 			pgrowZone,
								 short 					cmoreMasters,
								 void *					limitPtr,
								 void *					startPtr);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetZone(__A0)
																							#endif
EXTERN_API( void )
SetZone							(THz 					hz)									ONEWORDINLINE(0xA01B);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 CompactMem(__D0)
																							#endif
EXTERN_API( Size )
CompactMem						(Size 					cbNeeded)							ONEWORDINLINE(0xA04C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 CompactMemSys(__D0)
																							#endif
EXTERN_API( Size )
CompactMemSys					(Size 					cbNeeded)							ONEWORDINLINE(0xA44C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter PurgeMem(__D0)
																							#endif
EXTERN_API( void )
PurgeMem						(Size 					cbNeeded)							ONEWORDINLINE(0xA04D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter PurgeMemSys(__D0)
																							#endif
EXTERN_API( void )
PurgeMemSys						(Size 					cbNeeded)							ONEWORDINLINE(0xA44D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 FreeMem
																							#endif
EXTERN_API( long )
FreeMem							(void)														ONEWORDINLINE(0xA01C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 FreeMemSys
																							#endif
EXTERN_API( long )
FreeMemSys						(void)														ONEWORDINLINE(0xA41C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter ReserveMem(__D0)
																							#endif
EXTERN_API( void )
ReserveMem						(Size 					cbNeeded)							ONEWORDINLINE(0xA040);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter ReserveMemSys(__D0)
																							#endif
EXTERN_API( void )
ReserveMemSys					(Size 					cbNeeded)							ONEWORDINLINE(0xA440);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MaxMem(__A1)
																							#endif
EXTERN_API( Size )
MaxMem							(Size *					grow)								TWOWORDINLINE(0xA11D, 0x2288);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MaxMemSys(__A1)
																							#endif
EXTERN_API( Size )
MaxMemSys						(Size *					grow)								TWOWORDINLINE(0xA51D, 0x2288);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetGrowZone(__A0)
																							#endif
EXTERN_API( void )
SetGrowZone						(GrowZoneUPP 			growZone)							ONEWORDINLINE(0xA04B);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter MoveHHi(__A0)
																							#endif
EXTERN_API( void )
MoveHHi							(Handle 				h)									ONEWORDINLINE(0xA064);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter DisposePtr(__A0)
																							#endif
EXTERN_API( void )
DisposePtr						(Ptr 					p)									ONEWORDINLINE(0xA01F);

EXTERN_API( Size )
GetPtrSize						(Ptr 					p);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetPtrSize(__A0, __D0)
																							#endif
EXTERN_API( void )
SetPtrSize						(Ptr 					p,
								 Size 					newSize)							ONEWORDINLINE(0xA020);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter DisposeHandle(__A0)
																							#endif
EXTERN_API( void )
DisposeHandle					(Handle 				h)									ONEWORDINLINE(0xA023);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetHandleSize(__A0, __D0)
																							#endif
EXTERN_API( void )
SetHandleSize					(Handle 				h,
								 Size 					newSize)							ONEWORDINLINE(0xA024);

/* 
	NOTE
	
	GetHandleSize and GetPtrSize are documented in Inside Mac as returning 0 
	in case of an error, but the traps actually return an error code in D0.
	The glue sets D0 to 0 if an error occured.
*/
EXTERN_API( Size )
GetHandleSize					(Handle 				h);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 InlineGetHandleSize(__A0)
																							#endif
EXTERN_API( Size )
InlineGetHandleSize				(Handle 				h)									ONEWORDINLINE(0xA025);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter ReallocateHandle(__A0, __D0)
																							#endif
EXTERN_API( void )
ReallocateHandle				(Handle 				h,
								 Size 					byteCount)							ONEWORDINLINE(0xA027);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter ReallocateHandleSys(__A0, __D0)
																							#endif
EXTERN_API( void )
ReallocateHandleSys				(Handle 				h,
								 Size 					byteCount)							ONEWORDINLINE(0xA427);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter EmptyHandle(__A0)
																							#endif
EXTERN_API( void )
EmptyHandle						(Handle 				h)									ONEWORDINLINE(0xA02B);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HSetRBit(__A0)
																							#endif
EXTERN_API( void )
HSetRBit						(Handle 				h)									ONEWORDINLINE(0xA067);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HClrRBit(__A0)
																							#endif
EXTERN_API( void )
HClrRBit						(Handle 				h)									ONEWORDINLINE(0xA068);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 HGetState(__A0)
																							#endif
EXTERN_API( SInt8 )
HGetState						(Handle 				h)									ONEWORDINLINE(0xA069);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter HSetState(__A0, __D0)
																							#endif
EXTERN_API( void )
HSetState						(Handle 				h,
								 SInt8 					flags)								ONEWORDINLINE(0xA06A);

EXTERN_API( void )
PurgeSpace						(long *					total,
								 long *					contig);

#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
/*
	PurgeSpaceTotal and PurgeSpaceContiguous are currently only implement
	on classic 68K.  The are the same as PurgeSpace() but return just
	one value (either total space purgable or contiguous space purgable).
*/
																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 PurgeSpaceTotal
																							#endif
EXTERN_API( long )
PurgeSpaceTotal					(void)														ONEWORDINLINE(0xA062);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PurgeSpaceContiguous
																							#endif
EXTERN_API( long )
PurgeSpaceContiguous			(void)														ONEWORDINLINE(0xA062);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 PurgeSpaceSysTotal
																							#endif
EXTERN_API( long )
PurgeSpaceSysTotal				(void)														ONEWORDINLINE(0xA562);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PurgeSpaceSysContiguous
																							#endif
EXTERN_API( long )
PurgeSpaceSysContiguous			(void)														ONEWORDINLINE(0xA562);

#endif  /* TARGET_OS_MAC && TARGET_CPU_68K &&  !TARGET_RT_MAC_CFM */

/*****************************************************************************

	The routines: 

		BlockMoveUncached
		BlockMoveDataUncached
		BlockZero
		BlockZeroUncached
	
	are intended for developers writing drivers. They do not exist in 
	“InterfaceLib.” You must link with “DriverServicesLib” to access them.
	
	But since BlockMove and BlockMoveData exist in both “InterfaceLib” and
	“DriverServicesLib”, you cannot link with both libraries.
	
*****************************************************************************/
																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter BlockMove(__A0, __A1, __D0)
																							#endif
EXTERN_API( void )
BlockMove						(const void *			srcPtr,
								 void *					destPtr,
								 Size 					byteCount)							ONEWORDINLINE(0xA02E);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter BlockMoveData(__A0, __A1, __D0)
																							#endif
EXTERN_API( void )
BlockMoveData					(const void *			srcPtr,
								 void *					destPtr,
								 Size 					byteCount)							ONEWORDINLINE(0xA22E);

EXTERN_API_C( void )
BlockMoveUncached				(const void *			srcPtr,
								 void *					destPtr,
								 Size 					byteCount);

EXTERN_API_C( void )
BlockMoveDataUncached			(const void *			srcPtr,
								 void *					destPtr,
								 Size 					byteCount);

EXTERN_API_C( void )
BlockZero						(void *					destPtr,
								 Size 					byteCount);

EXTERN_API_C( void )
BlockZeroUncached				(void *					destPtr,
								 Size 					byteCount);


EXTERN_API( void )
MaxApplZone						(void)														ONEWORDINLINE(0xA063);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetApplBase(__A0)
																							#endif
EXTERN_API( void )
SetApplBase						(void *					startPtr)							ONEWORDINLINE(0xA057);

EXTERN_API( void )
MoreMasters						(void)														ONEWORDINLINE(0xA036);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter SetApplLimit(__A0)
																							#endif
EXTERN_API( void )
SetApplLimit					(void *					zoneLimit)							ONEWORDINLINE(0xA02D);



EXTERN_API( void )
InitApplZone					(void)														ONEWORDINLINE(0xA02C);


/*  Temporary Memory routines renamed, but obsolete, in System 7.0 and later.  */
EXTERN_API( void )
TempHLock						(Handle 				h,
								 OSErr *				resultCode)							THREEWORDINLINE(0x3F3C, 0x001E, 0xA88F);

EXTERN_API( void )
TempHUnlock						(Handle 				h,
								 OSErr *				resultCode)							THREEWORDINLINE(0x3F3C, 0x001F, 0xA88F);

EXTERN_API( void )
TempDisposeHandle				(Handle 				h,
								 OSErr *				resultCode)							THREEWORDINLINE(0x3F3C, 0x0020, 0xA88F);

EXTERN_API( Ptr )
TempTopMem						(void)														THREEWORDINLINE(0x3F3C, 0x0016, 0xA88F);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 HoldMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
HoldMemory						(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7000, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 UnholdMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
UnholdMemory					(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7001, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 LockMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
LockMemory						(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7002, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 LockMemoryForOutput(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
LockMemoryForOutput				(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x700A, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 LockMemoryContiguous(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
LockMemoryContiguous			(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7004, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 UnlockMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
UnlockMemory					(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7003, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MakeMemoryResident(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
MakeMemoryResident				(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x700B, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 ReleaseMemoryData(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
ReleaseMemoryData				(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x700C, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 MakeMemoryNonResident(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
MakeMemoryNonResident			(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x700D, 0xA05C);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 FlushMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
FlushMemory						(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x700E, 0xA05C);

EXTERN_API( OSErr )
GetPhysical						(LogicalToPhysicalTable * addresses,
								 unsigned long *		physicalEntryCount);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 DeferUserFn(__A0, __D0)
																							#endif
EXTERN_API( OSErr )
DeferUserFn						(UserFnUPP 				userFunction,
								 void *					argument)							ONEWORDINLINE(0xA08F);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 DebuggerGetMax
																							#endif
EXTERN_API( long )
DebuggerGetMax					(void)														TWOWORDINLINE(0x7000, 0xA08D);

EXTERN_API( void )
DebuggerEnter					(void)														TWOWORDINLINE(0x7001, 0xA08D);

EXTERN_API( void )
DebuggerExit					(void)														TWOWORDINLINE(0x7002, 0xA08D);

EXTERN_API( void )
DebuggerPoll					(void)														TWOWORDINLINE(0x7003, 0xA08D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 GetPageState(__A0)
																							#endif
EXTERN_API( PageState )
GetPageState					(const void *			address)							TWOWORDINLINE(0x7004, 0xA08D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PageFaultFatal
																							#endif
EXTERN_API( Boolean )
PageFaultFatal					(void)														TWOWORDINLINE(0x7005, 0xA08D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 DebuggerLockMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
DebuggerLockMemory				(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7006, 0xA08D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 DebuggerUnlockMemory(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
DebuggerUnlockMemory			(void *					address,
								 unsigned long 			count)								TWOWORDINLINE(0x7007, 0xA08D);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 EnterSupervisorMode
																							#endif
EXTERN_API( StatusRegisterContents )
EnterSupervisorMode				(void)														TWOWORDINLINE(0x7008, 0xA08D);


/* 
	StripAddress and Translate24To32 are traps on classic 68K,
	but macro to the identity function on PowerMacs or other OS's.
*/
#if TARGET_OS_MAC && TARGET_CPU_68K
																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 StripAddress(__D0)
																							#endif
EXTERN_API( Ptr )
StripAddress					(void *					theAddress)							ONEWORDINLINE(0xA055);

#else
#define StripAddress(x)   	((Ptr)(x))
#endif  /* TARGET_OS_MAC && TARGET_CPU_68K */


#if TARGET_OS_MAC && TARGET_CPU_68K
																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 Translate24To32(__D0)
																							#endif
EXTERN_API( Ptr )
Translate24To32					(void *					addr24)								ONEWORDINLINE(0xA091);

#else
#define Translate24To32(x)  ((Ptr)(x))
#endif  /* TARGET_OS_MAC && TARGET_CPU_68K */

EXTERN_API( OSErr )
HandToHand						(Handle *				theHndl);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PtrToXHand(__A0, __A1, __D0)
																							#endif
EXTERN_API( OSErr )
PtrToXHand						(const void *			srcPtr,
								 Handle 				dstHndl,
								 long 					size)								ONEWORDINLINE(0xA9E2);

EXTERN_API( OSErr )
PtrToHand						(const void *			srcPtr,
								 Handle *				dstHndl,
								 long 					size);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 HandAndHand(__A0, __A1)
																							#endif
EXTERN_API( OSErr )
HandAndHand						(Handle 				hand1,
								 Handle 				hand2)								ONEWORDINLINE(0xA9E4);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 PtrAndHand(__A0, __A1, __D0)
																							#endif
EXTERN_API( OSErr )
PtrAndHand						(const void *			ptr1,
								 Handle 				hand2,
								 long 					size)								ONEWORDINLINE(0xA9EF);

#if OLDROUTINENAMES
#define ApplicZone() ApplicationZone()
#define MFTempNewHandle(logicalSize, resultCode) TempNewHandle(logicalSize, resultCode)
#define MFMaxMem(grow) TempMaxMem(grow)
#define MFFreeMem() TempFreeMem()
#define MFTempHLock(h, resultCode) TempHLock(h, resultCode)
#define MFTempHUnlock(h, resultCode) TempHUnlock(h, resultCode)
#define MFTempDisposHandle(h, resultCode) TempDisposeHandle(h, resultCode)
#define MFTopMem() TempTopMem()
#define ResrvMem(cbNeeded) ReserveMem(cbNeeded)
#define DisposPtr(p) DisposePtr(p)
#define DisposHandle(h) DisposeHandle(h)
#define ReallocHandle(h, byteCount) ReallocateHandle(h, byteCount)
#endif  /* OLDROUTINENAMES */


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

#endif /* __MACMEMORY__ */

