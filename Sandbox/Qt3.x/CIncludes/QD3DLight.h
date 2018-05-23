/*
 	File:		QD3DLight.h
 
 	Contains:	Generic light routines								 			
 
 	Version:	Technology:	Quickdraw 3D 1.5.4
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QD3DLIGHT__
#define __QD3DLIGHT__

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
		#define PRAGMA_ENUM_PACK__QD3DLIGHT__
	#endif
	#pragma options(!pack_enums)
#endif

/******************************************************************************
 **																			 **
 **							Enum Definitions								 **
 **																			 **
 *****************************************************************************/

enum TQ3AttenuationType {
	kQ3AttenuationTypeNone		= 0,
	kQ3AttenuationTypeInverseDistance = 1,
	kQ3AttenuationTypeInverseDistanceSquared = 2
};
typedef enum TQ3AttenuationType TQ3AttenuationType;



enum TQ3FallOffType {
	kQ3FallOffTypeNone			= 0,
	kQ3FallOffTypeLinear		= 1,
	kQ3FallOffTypeExponential	= 2,
	kQ3FallOffTypeCosine		= 3
};
typedef enum TQ3FallOffType TQ3FallOffType;


/******************************************************************************
 **																			 **
 **							Data Structure Definitions						 **
 **																			 **
 *****************************************************************************/

struct TQ3LightData {
	TQ3Boolean 						isOn;
	float 							brightness;
	TQ3ColorRGB 					color;
};
typedef struct TQ3LightData				TQ3LightData;

struct TQ3DirectionalLightData {
	TQ3LightData 					lightData;
	TQ3Boolean 						castsShadows;
	TQ3Vector3D 					direction;
};
typedef struct TQ3DirectionalLightData	TQ3DirectionalLightData;

struct TQ3PointLightData {
	TQ3LightData 					lightData;
	TQ3Boolean 						castsShadows;
	TQ3AttenuationType 				attenuation;
	TQ3Point3D 						location;
};
typedef struct TQ3PointLightData		TQ3PointLightData;

struct TQ3SpotLightData {
	TQ3LightData 					lightData;
	TQ3Boolean 						castsShadows;
	TQ3AttenuationType 				attenuation;
	TQ3Point3D 						location;
	TQ3Vector3D 					direction;
	float 							hotAngle;
	float 							outerAngle;
	TQ3FallOffType 					fallOff;
};
typedef struct TQ3SpotLightData			TQ3SpotLightData;
/******************************************************************************
 **																			 **
 **					Light routines (apply to all TQ3LightObjects)		     **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3ObjectType )
Q3Light_GetType					(TQ3LightObject 		light);

EXTERN_API_C( TQ3Status )
Q3Light_GetState				(TQ3LightObject 		light,
								 TQ3Boolean *			isOn);

EXTERN_API_C( TQ3Status )
Q3Light_GetBrightness			(TQ3LightObject 		light,
								 float *				brightness);

EXTERN_API_C( TQ3Status )
Q3Light_GetColor				(TQ3LightObject 		light,
								 TQ3ColorRGB *			color);

EXTERN_API_C( TQ3Status )
Q3Light_SetState				(TQ3LightObject 		light,
								 TQ3Boolean 			isOn);

EXTERN_API_C( TQ3Status )
Q3Light_SetBrightness			(TQ3LightObject 		light,
								 float 					brightness);

EXTERN_API_C( TQ3Status )
Q3Light_SetColor				(TQ3LightObject 		light,
								 const TQ3ColorRGB *	color);

EXTERN_API_C( TQ3Status )
Q3Light_GetData					(TQ3LightObject 		light,
								 TQ3LightData *			lightData);

EXTERN_API_C( TQ3Status )
Q3Light_SetData					(TQ3LightObject 		light,
								 const TQ3LightData *	lightData);


/******************************************************************************
 **																			 **
 **							Specific Light Routines					 	 	 **
 **																			 **
 *****************************************************************************/
/******************************************************************************
 **																			 **
 **							Ambient Light							 	 	 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3LightObject )
Q3AmbientLight_New				(const TQ3LightData *	lightData);

EXTERN_API_C( TQ3Status )
Q3AmbientLight_GetData			(TQ3LightObject 		light,
								 TQ3LightData *			lightData);

EXTERN_API_C( TQ3Status )
Q3AmbientLight_SetData			(TQ3LightObject 		light,
								 const TQ3LightData *	lightData);


/******************************************************************************
 **																			 **
 **						Directional Light									 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3LightObject )
Q3DirectionalLight_New			(const TQ3DirectionalLightData * directionalLightData);

EXTERN_API_C( TQ3Status )
Q3DirectionalLight_GetCastShadowsState (TQ3LightObject 	light,
								 TQ3Boolean *			castsShadows);

EXTERN_API_C( TQ3Status )
Q3DirectionalLight_GetDirection	(TQ3LightObject 		light,
								 TQ3Vector3D *			direction);

EXTERN_API_C( TQ3Status )
Q3DirectionalLight_SetCastShadowsState (TQ3LightObject 	light,
								 TQ3Boolean 			castsShadows);

EXTERN_API_C( TQ3Status )
Q3DirectionalLight_SetDirection	(TQ3LightObject 		light,
								 const TQ3Vector3D *	direction);

EXTERN_API_C( TQ3Status )
Q3DirectionalLight_GetData		(TQ3LightObject 		light,
								 TQ3DirectionalLightData * directionalLightData);

EXTERN_API_C( TQ3Status )
Q3DirectionalLight_SetData		(TQ3LightObject 		light,
								 const TQ3DirectionalLightData * directionalLightData);


/******************************************************************************
 **																			 **
 **						Point Light									 		 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3LightObject )
Q3PointLight_New				(const TQ3PointLightData * pointLightData);

EXTERN_API_C( TQ3Status )
Q3PointLight_GetCastShadowsState (TQ3LightObject 		light,
								 TQ3Boolean *			castsShadows);

EXTERN_API_C( TQ3Status )
Q3PointLight_GetAttenuation		(TQ3LightObject 		light,
								 TQ3AttenuationType *	attenuation);

EXTERN_API_C( TQ3Status )
Q3PointLight_GetLocation		(TQ3LightObject 		light,
								 TQ3Point3D *			location);

EXTERN_API_C( TQ3Status )
Q3PointLight_GetData			(TQ3LightObject 		light,
								 TQ3PointLightData *	pointLightData);

EXTERN_API_C( TQ3Status )
Q3PointLight_SetCastShadowsState (TQ3LightObject 		light,
								 TQ3Boolean 			castsShadows);

EXTERN_API_C( TQ3Status )
Q3PointLight_SetAttenuation		(TQ3LightObject 		light,
								 TQ3AttenuationType 	attenuation);

EXTERN_API_C( TQ3Status )
Q3PointLight_SetLocation		(TQ3LightObject 		light,
								 const TQ3Point3D *		location);

EXTERN_API_C( TQ3Status )
Q3PointLight_SetData			(TQ3LightObject 		light,
								 const TQ3PointLightData * pointLightData);


/******************************************************************************
 **																			 **
 **						Spot Light									 		 **
 **																			 **
 *****************************************************************************/
EXTERN_API_C( TQ3LightObject )
Q3SpotLight_New					(const TQ3SpotLightData * spotLightData);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetCastShadowsState	(TQ3LightObject 		light,
								 TQ3Boolean *			castsShadows);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetAttenuation		(TQ3LightObject 		light,
								 TQ3AttenuationType *	attenuation);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetLocation			(TQ3LightObject 		light,
								 TQ3Point3D *			location);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetDirection		(TQ3LightObject 		light,
								 TQ3Vector3D *			direction);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetHotAngle			(TQ3LightObject 		light,
								 float *				hotAngle);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetOuterAngle		(TQ3LightObject 		light,
								 float *				outerAngle);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetFallOff			(TQ3LightObject 		light,
								 TQ3FallOffType *		fallOff);

EXTERN_API_C( TQ3Status )
Q3SpotLight_GetData				(TQ3LightObject 		light,
								 TQ3SpotLightData *		spotLightData);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetCastShadowsState	(TQ3LightObject 		light,
								 TQ3Boolean 			castsShadows);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetAttenuation		(TQ3LightObject 		light,
								 TQ3AttenuationType 	attenuation);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetLocation			(TQ3LightObject 		light,
								 const TQ3Point3D *		location);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetDirection		(TQ3LightObject 		light,
								 const TQ3Vector3D *	direction);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetHotAngle			(TQ3LightObject 		light,
								 float 					hotAngle);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetOuterAngle		(TQ3LightObject 		light,
								 float 					outerAngle);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetFallOff			(TQ3LightObject 		light,
								 TQ3FallOffType 		fallOff);

EXTERN_API_C( TQ3Status )
Q3SpotLight_SetData				(TQ3LightObject 		light,
								 const TQ3SpotLightData * spotLightData);





#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint reset
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=reset
#elif defined(PRAGMA_ENUM_PACK__QD3DLIGHT__)
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

#endif /* __QD3DLIGHT__ */

