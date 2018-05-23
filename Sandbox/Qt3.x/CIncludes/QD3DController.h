/*
 	File:		QD3DController.h
 
 	Contains:	Q3Controller methods	
 
 	Version:	Technology:	Quickdraw 3D 1.5.4
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QD3DCONTROLLER__
#define __QD3DCONTROLLER__

#ifndef __QD3D__
#include <QD3D.h>
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
	#pragma options align=power
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint on
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=int
#elif PRAGMA_ENUM_PACK
	#if __option(pack_enums)
		#define PRAGMA_ENUM_PACK__QD3DCONTROLLER__
	#endif
	#pragma options(!pack_enums)
#endif

/******************************************************************************
 **																			 **
 **								Type Definitions							 **
 **																			 **
 *****************************************************************************/
#define kQ3ControllerSetChannelMaxDataSize		256
typedef CALLBACK_API_C( TQ3Status , TQ3ChannelGetMethod )(TQ3ControllerRef controllerRef, unsigned long channel, void *data, unsigned long *dataSize);
typedef CALLBACK_API_C( TQ3Status , TQ3ChannelSetMethod )(TQ3ControllerRef controllerRef, unsigned long channel, const void *data, unsigned long dataSize);

struct TQ3ControllerData {
	char *							signature;
	unsigned long 					valueCount;
	unsigned long 					channelCount;
	TQ3ChannelGetMethod 			channelGetMethod;
	TQ3ChannelSetMethod 			channelSetMethod;
};
typedef struct TQ3ControllerData		TQ3ControllerData;
/******************************************************************************
 **																			 **
 **									 Routines								 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3Status )
Q3Controller_GetListChanged		(TQ3Boolean *			listChanged,
								 unsigned long *		serialNumber);

EXTERN_API_C( TQ3Status )
Q3Controller_Next				(TQ3ControllerRef 		controllerRef,
								 TQ3ControllerRef *		nextControllerRef);

EXTERN_API_C( TQ3ControllerRef )
Q3Controller_New				(const TQ3ControllerData * controllerData);

EXTERN_API_C( TQ3Status )
Q3Controller_Decommission		(TQ3ControllerRef 		controllerRef);

EXTERN_API_C( TQ3Status )
Q3Controller_SetActivation		(TQ3ControllerRef 		controllerRef,
								 TQ3Boolean 			active);

EXTERN_API_C( TQ3Status )
Q3Controller_GetActivation		(TQ3ControllerRef 		controllerRef,
								 TQ3Boolean *			active);

EXTERN_API_C( TQ3Status )
Q3Controller_GetSignature		(TQ3ControllerRef 		controllerRef,
								 char *					signature,
								 unsigned long 			numChars);

EXTERN_API_C( TQ3Status )
Q3Controller_SetChannel			(TQ3ControllerRef 		controllerRef,
								 unsigned long 			channel,
								 const void *			data,
								 unsigned long 			dataSize);

EXTERN_API_C( TQ3Status )
Q3Controller_GetChannel			(TQ3ControllerRef 		controllerRef,
								 unsigned long 			channel,
								 void *					data,
								 unsigned long *		dataSize);

EXTERN_API_C( TQ3Status )
Q3Controller_GetValueCount		(TQ3ControllerRef 		controllerRef,
								 unsigned long *		valueCount);

EXTERN_API_C( TQ3Status )
Q3Controller_SetTracker			(TQ3ControllerRef 		controllerRef,
								 TQ3TrackerObject 		tracker);

EXTERN_API_C( TQ3Status )
Q3Controller_HasTracker			(TQ3ControllerRef 		controllerRef,
								 TQ3Boolean *			hasTracker);

EXTERN_API_C( TQ3Status )
Q3Controller_Track2DCursor		(TQ3ControllerRef 		controllerRef,
								 TQ3Boolean *			track2DCursor);

EXTERN_API_C( TQ3Status )
Q3Controller_Track3DCursor		(TQ3ControllerRef 		controllerRef,
								 TQ3Boolean *			track3DCursor);

EXTERN_API_C( TQ3Status )
Q3Controller_GetButtons			(TQ3ControllerRef 		controllerRef,
								 unsigned long *		buttons);

EXTERN_API_C( TQ3Status )
Q3Controller_SetButtons			(TQ3ControllerRef 		controllerRef,
								 unsigned long 			buttons);

EXTERN_API_C( TQ3Status )
Q3Controller_GetTrackerPosition	(TQ3ControllerRef 		controllerRef,
								 TQ3Point3D *			position);

EXTERN_API_C( TQ3Status )
Q3Controller_SetTrackerPosition	(TQ3ControllerRef 		controllerRef,
								 const TQ3Point3D *		position);

EXTERN_API_C( TQ3Status )
Q3Controller_MoveTrackerPosition (TQ3ControllerRef 		controllerRef,
								 const TQ3Vector3D *	delta);

EXTERN_API_C( TQ3Status )
Q3Controller_GetTrackerOrientation (TQ3ControllerRef 	controllerRef,
								 TQ3Quaternion *		orientation);

EXTERN_API_C( TQ3Status )
Q3Controller_SetTrackerOrientation (TQ3ControllerRef 	controllerRef,
								 const TQ3Quaternion *	orientation);

EXTERN_API_C( TQ3Status )
Q3Controller_MoveTrackerOrientation (TQ3ControllerRef 	controllerRef,
								 const TQ3Quaternion *	delta);

EXTERN_API_C( TQ3Status )
Q3Controller_GetValues			(TQ3ControllerRef 		controllerRef,
								 unsigned long 			valueCount,
								 float *				values,
								 TQ3Boolean *			changed,
								 unsigned long *		serialNumber);

EXTERN_API_C( TQ3Status )
Q3Controller_SetValues			(TQ3ControllerRef 		controllerRef,
								 const float *			values,
								 unsigned long 			valueCount);


/******************************************************************************
 **																			 **
 **									 Routines								 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3ControllerStateObject )
Q3ControllerState_New			(TQ3ControllerRef 		controllerRef);

EXTERN_API_C( TQ3Status )
Q3ControllerState_SaveAndReset	(TQ3ControllerStateObject  controllerStateObject);

EXTERN_API_C( TQ3Status )
Q3ControllerState_Restore		(TQ3ControllerStateObject  controllerStateObject);


/******************************************************************************
 **																			 **
 **								Type Definitions							 **
 **																			 **
 *****************************************************************************/
typedef CALLBACK_API_C( TQ3Status , TQ3TrackerNotifyFunc )(TQ3TrackerObject trackerObject, TQ3ControllerRef controllerRef);
/******************************************************************************
 **																			 **
 **									 Routines								 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3TrackerObject )
Q3Tracker_New					(TQ3TrackerNotifyFunc 	notifyFunc);

EXTERN_API_C( TQ3Status )
Q3Tracker_SetNotifyThresholds	(TQ3TrackerObject 		trackerObject,
								 float 					positionThresh,
								 float 					orientationThresh);

EXTERN_API_C( TQ3Status )
Q3Tracker_GetNotifyThresholds	(TQ3TrackerObject 		trackerObject,
								 float *				positionThresh,
								 float *				orientationThresh);

EXTERN_API_C( TQ3Status )
Q3Tracker_SetActivation			(TQ3TrackerObject 		trackerObject,
								 TQ3Boolean 			active);

EXTERN_API_C( TQ3Status )
Q3Tracker_GetActivation			(TQ3TrackerObject 		trackerObject,
								 TQ3Boolean *			active);

EXTERN_API_C( TQ3Status )
Q3Tracker_GetButtons			(TQ3TrackerObject 		trackerObject,
								 unsigned long *		buttons);

EXTERN_API_C( TQ3Status )
Q3Tracker_ChangeButtons			(TQ3TrackerObject 		trackerObject,
								 TQ3ControllerRef 		controllerRef,
								 unsigned long 			buttons,
								 unsigned long 			buttonMask);

EXTERN_API_C( TQ3Status )
Q3Tracker_GetPosition			(TQ3TrackerObject 		trackerObject,
								 TQ3Point3D *			position,
								 TQ3Vector3D *			delta,
								 TQ3Boolean *			changed,
								 unsigned long *		serialNumber);

EXTERN_API_C( TQ3Status )
Q3Tracker_SetPosition			(TQ3TrackerObject 		trackerObject,
								 TQ3ControllerRef 		controllerRef,
								 const TQ3Point3D *		position);

EXTERN_API_C( TQ3Status )
Q3Tracker_MovePosition			(TQ3TrackerObject 		trackerObject,
								 TQ3ControllerRef 		controllerRef,
								 const TQ3Vector3D *	delta);

EXTERN_API_C( TQ3Status )
Q3Tracker_GetOrientation		(TQ3TrackerObject 		trackerObject,
								 TQ3Quaternion *		orientation,
								 TQ3Quaternion *		delta,
								 TQ3Boolean *			changed,
								 unsigned long *		serialNumber);

EXTERN_API_C( TQ3Status )
Q3Tracker_SetOrientation		(TQ3TrackerObject 		trackerObject,
								 TQ3ControllerRef 		controllerRef,
								 const TQ3Quaternion *	orientation);

EXTERN_API_C( TQ3Status )
Q3Tracker_MoveOrientation		(TQ3TrackerObject 		trackerObject,
								 TQ3ControllerRef 		controllerRef,
								 const TQ3Quaternion *	delta);

EXTERN_API_C( TQ3Status )
Q3Tracker_SetEventCoordinates	(TQ3TrackerObject 		trackerObject,
								 unsigned long 			timeStamp,
								 unsigned long 			buttons,
								 const TQ3Point3D *		position,
								 const TQ3Quaternion *	orientation);

EXTERN_API_C( TQ3Status )
Q3Tracker_GetEventCoordinates	(TQ3TrackerObject 		trackerObject,
								 unsigned long 			timeStamp,
								 unsigned long *		buttons,
								 TQ3Point3D *			position,
								 TQ3Quaternion *		orientation);

/******************************************************************************
 **																			 **
 **								 Types										 **
 **																			 **
 *****************************************************************************/
typedef CALLBACK_API_C( void , TQ3CursorTrackerNotifyFunc )(void );
/******************************************************************************
 **																			 **
 **								 Routines									 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3Status )
Q3CursorTracker_PrepareTracking	(void);

EXTERN_API_C( TQ3Status )
Q3CursorTracker_SetTrackDeltas	(TQ3Boolean 			trackDeltas);

EXTERN_API_C( TQ3Status )
Q3CursorTracker_GetAndClearDeltas (float *				depth,
								 TQ3Quaternion *		orientation,
								 TQ3Boolean *			hasOrientation,
								 TQ3Boolean *			changed,
								 unsigned long *		serialNumber);

EXTERN_API_C( TQ3Status )
Q3CursorTracker_SetNotifyFunc	(TQ3CursorTrackerNotifyFunc  notifyFunc);

EXTERN_API_C( TQ3Status )
Q3CursorTracker_GetNotifyFunc	(TQ3CursorTrackerNotifyFunc * notifyFunc);






#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint reset
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=reset
#elif defined(PRAGMA_ENUM_PACK__QD3DCONTROLLER__)
	#pragma options(pack_enums)
#endif

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

#endif /* __QD3DCONTROLLER__ */

