/*
 	File:		CMMComponent.h
 
 	Contains:	ColorSync CMM Component API
 
 	Version:	Technology:	ColorSync 2.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __CMMCOMPONENT__
#define __CMMCOMPONENT__

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __CMAPPLICATION__
#include <CMApplication.h>
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
	CMMInterfaceVersion			= 1
};

/* Component function selectors */
/* Required */

enum {
	kCMMInit					= 0,
	kNCMMInit					= 6,
	kCMMMatchColors				= 1,
	kCMMCheckColors				= 2
};

/* Optional */

enum {
	kCMMValidateProfile			= 8,
	kCMMFlattenProfile			= 14,
	kCMMUnflattenProfile		= 15,
	kCMMMatchBitmap				= 9,
	kCMMCheckBitmap				= 10,
	kCMMMatchPixMap				= 3,
	kCMMCheckPixMap				= 4,
	kCMMConcatenateProfiles		= 5,
	kCMMConcatInit				= 7,
	kCMMNewLinkProfile			= 16,
	kCMMGetPS2ColorSpace		= 11,
	kCMMGetPS2ColorRenderingIntent = 12,
	kCMMGetPS2ColorRendering	= 13,
	kCMMGetPS2ColorRenderingVMSize = 17,
	kCMMGetNamedColorInfo		= 70,
	kCMMGetNamedColorValue		= 71,
	kCMMGetIndNamedColorValue	= 72,
	kCMMGetNamedColorIndex		= 73,
	kCMMGetNamedColorName		= 74
};


EXTERN_API( CMError )
NCMInit							(ComponentInstance 		CMSession,
								 CMProfileRef 			srcProfile,
								 CMProfileRef 			dstProfile)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0006, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMInit							(ComponentInstance 		CMSession,
								 CMProfileHandle 		srcProfile,
								 CMProfileHandle 		dstProfile)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0000, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMatchColors					(ComponentInstance 		CMSession,
								 CMColor *				myColors,
								 unsigned long 			count)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0001, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMCheckColors					(ComponentInstance 		CMSession,
								 CMColor *				myColors,
								 unsigned long 			count,
								 long *					result)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0002, 0x7000, 0xA82A);


/* Optional functions */
EXTERN_API( CMError )
CMMValidateProfile				(ComponentInstance 		CMSession,
								 CMProfileRef 			prof,
								 Boolean *				valid)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0008, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMFlattenProfile				(ComponentInstance 		CMSession,
								 CMProfileRef 			prof,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x000E, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMUnflattenProfile				(ComponentInstance 		CMSession,
								 FSSpec *				resultFileSpec,
								 CMFlattenUPP 			proc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x000F, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMatchBitmap					(ComponentInstance 		CMSession,
								 const CMBitmap *		bitmap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon,
								 CMBitmap *				matchedBitmap)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x0009, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMCheckBitmap					(ComponentInstance 		CMSession,
								 const CMBitmap *		bitmap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon,
								 CMBitmap *				resultBitmap)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x000A, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMatchPixMap					(ComponentInstance 		CMSession,
								 PixMap *				myPixMap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0003, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMCheckPixMap					(ComponentInstance 		CMSession,
								 const PixMap *			myPixMap,
								 CMBitmapCallBackUPP 	progressProc,
								 BitMap *				myBitMap,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0004, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMConcatInit					(ComponentInstance 		CMSession,
								 CMConcatProfileSet *	profileSet)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMNewLinkProfile				(ComponentInstance 		CMSession,
								 CMProfileRef *			prof,
								 const CMProfileLocation * targetLocation,
								 CMConcatProfileSet *	profileSet)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0010, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetPS2ColorSpace				(ComponentInstance 		CMSession,
								 CMProfileRef 			srcProf,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x000B, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetPS2ColorRenderingIntent	(ComponentInstance 		CMSession,
								 CMProfileRef 			srcProf,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x000C, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetPS2ColorRendering			(ComponentInstance 		CMSession,
								 CMProfileRef 			srcProf,
								 CMProfileRef 			dstProf,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0014, 0x000D, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetPS2ColorRenderingVMSize	(ComponentInstance 		CMSession,
								 CMProfileRef 			srcProf,
								 CMProfileRef 			dstProf,
								 unsigned long *		vmSize)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0011, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMConcatenateProfiles			(ComponentInstance 		CMSession,
								 CMProfileHandle 		thru,
								 CMProfileHandle 		dst,
								 CMProfileHandle *		newDst)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0005, 0x7000, 0xA82A);

/* Named Color functions */
EXTERN_API( CMError )
CMMGetNamedColorInfo			(ComponentInstance 		CMSession,
								 CMProfileRef 			srcProf,
								 unsigned long *		deviceChannels,
								 OSType *				deviceColorSpace,
								 OSType *				PCSColorSpace,
								 unsigned long *		count,
								 StringPtr 				prefix,
								 StringPtr 				suffix)								FIVEWORDINLINE(0x2F3C, 0x001C, 0x0046, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetNamedColorValue			(ComponentInstance 		CMSession,
								 CMProfileRef 			prof,
								 StringPtr 				name,
								 CMColor *				deviceColor,
								 CMColor *				PCSColor)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x0047, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetIndNamedColorValue		(ComponentInstance 		CMSession,
								 CMProfileRef 			prof,
								 unsigned long 			index,
								 CMColor *				deviceColor,
								 CMColor *				PCSColor)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x0048, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetNamedColorIndex			(ComponentInstance 		CMSession,
								 CMProfileRef 			prof,
								 StringPtr 				name,
								 unsigned long *		index)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0049, 0x7000, 0xA82A);

EXTERN_API( CMError )
CMMGetNamedColorName			(ComponentInstance 		CMSession,
								 CMProfileRef 			prof,
								 unsigned long 			index,
								 StringPtr 				name)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x004A, 0x7000, 0xA82A);


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

#endif /* __CMMCOMPONENT__ */

