/*
 	File:		DriverSynchronization.h
 
 	Contains:	Driver Synchronization Interfaces.
 
 	Version:	Technology:	MacOS
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __DRIVERSYNCHRONIZATION__
#define __DRIVERSYNCHRONIZATION__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __MACTYPES__
#include <MacTypes.h>
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

EXTERN_API_C( void )
SynchronizeIO					(void)														ONEWORDINLINE(0x4E71);

EXTERN_API_C( Boolean )
CompareAndSwap					(UInt32 				oldVvalue,
								 UInt32 				newValue,
								 UInt32 *				OldValueAdr);

EXTERN_API_C( Boolean )
TestAndClear					(UInt32 				bit,
								 UInt8 *				startAddress);

EXTERN_API_C( Boolean )
TestAndSet						(UInt32 				bit,
								 UInt8 *				startAddress);

EXTERN_API_C( SInt8 )
IncrementAtomic8				(SInt8 *				value);

EXTERN_API_C( SInt8 )
DecrementAtomic8				(SInt8 *				value);

EXTERN_API_C( SInt8 )
AddAtomic8						(SInt32 				amount,
								 SInt8 *				value);

EXTERN_API_C( UInt8 )
BitAndAtomic8					(UInt32 				mask,
								 UInt8 *				value);

EXTERN_API_C( UInt8 )
BitOrAtomic8					(UInt32 				mask,
								 UInt8 *				value);

EXTERN_API_C( UInt8 )
BitXorAtomic8					(UInt32 				mask,
								 UInt8 *				value);

EXTERN_API_C( SInt16 )
IncrementAtomic16				(SInt16 *				value);

EXTERN_API_C( SInt16 )
DecrementAtomic16				(SInt16 *				value);

EXTERN_API_C( SInt16 )
AddAtomic16						(SInt32 				amount,
								 SInt16 *				value);

EXTERN_API_C( UInt16 )
BitAndAtomic16					(UInt32 				mask,
								 UInt16 *				value);

EXTERN_API_C( UInt16 )
BitOrAtomic16					(UInt32 				mask,
								 UInt16 *				value);

EXTERN_API_C( UInt16 )
BitXorAtomic16					(UInt32 				mask,
								 UInt16 *				value);

EXTERN_API_C( SInt32 )
IncrementAtomic					(SInt32 *				value);

EXTERN_API_C( SInt32 )
DecrementAtomic					(SInt32 *				value);

EXTERN_API_C( SInt32 )
AddAtomic						(SInt32 				amount,
								 SInt32 *				value);

EXTERN_API_C( UInt32 )
BitAndAtomic					(UInt32 				mask,
								 UInt32 *				value);

EXTERN_API_C( UInt32 )
BitOrAtomic						(UInt32 				mask,
								 UInt32 *				value);

EXTERN_API_C( UInt32 )
BitXorAtomic					(UInt32 				mask,
								 UInt32 *				value);



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

#endif /* __DRIVERSYNCHRONIZATION__ */

