/*
 	File:		QD3DAcceleration.h
 
 	Contains:	Header file for low-level 3D driver API							
 
 	Version:	Technology:	Quickdraw 3D 1.5.4
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QD3DACCELERATION__
#define __QD3DACCELERATION__

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
		#define PRAGMA_ENUM_PACK__QD3DACCELERATION__
	#endif
	#pragma options(!pack_enums)
#endif

/******************************************************************************
 **																			 **
 ** 						Vendor ID definitions							 **
 **																			 **
 *****************************************************************************/
/*
 * If kQAVendor_BestChoice is used, the system chooses the "best" drawing engine
 * available for the target device. This should be used for the default.
 */
#define kQAVendor_BestChoice		(-1)
/*
 * The other definitions (kQAVendor_Apple, etc.) identify specific vendors
 * of drawing engines. When a vendor ID is used in conjunction with a
 * vendor-defined engine ID, a specific drawing engine can be selected.
 */
#define kQAVendor_Apple			0
#define kQAVendor_ATI			1
#define kQAVendor_Radius		2
#define kQAVendor_Mentor		3		/* Mentor Software, Inc. */
#define kQAVendor_Matrox		4
#define kQAVendor_Yarc			5
#define kQAVendor_DiamondMM		6
#define kQAVendor_3DLabs		7
#define kQAVendor_D3DAdaptor	8
#define kQAVendor_IXMicro		9
/******************************************************************************
 **																			 **
 **						 Apple's engine ID definitions						 **
 **																			 **
 *****************************************************************************/
#define kQAEngine_AppleSW		0		/* Default software rasterizer */
#define kQAEngine_AppleHW		(-1)	/* Apple accelerator */
#define kQAEngine_AppleHW2		1		/* Another Apple accelerator */
#define kQAEngine_AppleHW3		2		/* Another Apple accelerator */


#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint reset
#elif PRAGMA_ENUM_OPTIONS
	#pragma option enum=reset
#elif defined(PRAGMA_ENUM_PACK__QD3DACCELERATION__)
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

#endif /* __QD3DACCELERATION__ */

