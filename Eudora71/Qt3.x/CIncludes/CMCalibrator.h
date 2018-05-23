/*
 	File:		CMCalibrator.h
 
 	Contains:	ColorSync Calibration API
 
 	Version:	Technology:	ColorSync 2.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __CMCALIBRATOR__
#define __CMCALIBRATOR__

#ifndef __CMAPPLICATION__
#include <CMApplication.h>
#endif
#ifndef __DISPLAYS__
#include <Displays.h>
#endif
#ifndef __ERRORS__
#include <Errors.h>
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

typedef CALLBACK_API( void , CalibrateEventProcPtr )(EventRecord *event);
typedef STACK_UPP_TYPE(CalibrateEventProcPtr) 					CalibrateEventUPP;

struct CalibratorInfo {
	AVIDType 						displayID;
	CMProfileLocation 				profileLocation;
	CalibrateEventUPP 				eventProc;
	UInt32 							reserved;
	UInt32 							flags;
	Boolean 						isGood;
	SInt8 							byteFiller;
};
typedef struct CalibratorInfo			CalibratorInfo;
typedef CALLBACK_API( Boolean , CanCalibrateProcPtr )(AVIDType displayID);
typedef CALLBACK_API( OSErr , CalibrateProcPtr )(CalibratorInfo *theInfo);
typedef STACK_UPP_TYPE(CanCalibrateProcPtr) 					CanCalibrateUPP;
typedef STACK_UPP_TYPE(CalibrateProcPtr) 						CalibrateUPP;
enum { uppCalibrateEventProcInfo = 0x000000C0 }; 				/* pascal no_return_value Func(4_bytes) */
enum { uppCanCalibrateProcInfo = 0x000000D0 }; 					/* pascal 1_byte Func(4_bytes) */
enum { uppCalibrateProcInfo = 0x000000E0 }; 					/* pascal 2_bytes Func(4_bytes) */
#define NewCalibrateEventProc(userRoutine) 						(CalibrateEventUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCalibrateEventProcInfo, GetCurrentArchitecture())
#define NewCanCalibrateProc(userRoutine) 						(CanCalibrateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCanCalibrateProcInfo, GetCurrentArchitecture())
#define NewCalibrateProc(userRoutine) 							(CalibrateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCalibrateProcInfo, GetCurrentArchitecture())
#define CallCalibrateEventProc(userRoutine, event) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppCalibrateEventProcInfo, (event))
#define CallCanCalibrateProc(userRoutine, displayID) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppCanCalibrateProcInfo, (displayID))
#define CallCalibrateProc(userRoutine, theInfo) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppCalibrateProcInfo, (theInfo))

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

#endif /* __CMCALIBRATOR__ */

