/*
 	File:		GXEnvironment.h
 
 	Contains:	QuickDraw GX environment constants and interfaces
 
 	Version:	Technology:	Quickdraw GX 1.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __GXENVIRONMENT__
#define __GXENVIRONMENT__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __GXTYPES__
#include <GXTypes.h>
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

#if defined(__MWERKS__) && TARGET_CPU_68K
	#pragma push
	#pragma pointers_in_D0
#endif
 
/* old header = graphics macintosh */
 

enum {
	defaultPollingHandlerFlags	= 0x00,
	okToSwitchDuringPollFlag	= 0x00,
	dontSwitchDuringPollFlag	= 0x01
};

typedef long 							gxPollingHandlerFlags;
typedef CALLBACK_API_C( void , gxPollingHandlerProcPtr )(long reference, gxPollingHandlerFlags flags);
typedef STACK_UPP_TYPE(gxPollingHandlerProcPtr) 				gxPollingHandlerUPP;
enum { uppgxPollingHandlerProcInfo = 0x000003C1 }; 				/* no_return_value Func(4_bytes, 4_bytes) */
#define NewgxPollingHandlerProc(userRoutine) 					(gxPollingHandlerUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppgxPollingHandlerProcInfo, GetCurrentArchitecture())
#define CallgxPollingHandlerProc(userRoutine, reference, flags)  CALL_TWO_PARAMETER_UPP((userRoutine), uppgxPollingHandlerProcInfo, (reference), (flags))
EXTERN_API_C( gxPollingHandlerUPP )
GXGetGraphicsPollingHandler		(long *					reference)							THREEWORDINLINE(0x303C, 0x0245, 0xA832);

EXTERN_API_C( void )
GXSetGraphicsPollingHandler		(gxPollingHandlerUPP 	handler,
								 long 					reference)							THREEWORDINLINE(0x303C, 0x0246, 0xA832);

/* old header = graphics toolbox */
 

/* QD to QD GX Translator typedefs */

enum {
	gxDefaultOptionsTranslation	= 0x0000,
	gxOptimizedTranslation		= 0x0001,
	gxReplaceLineWidthTranslation = 0x0002,
	gxSimpleScalingTranslation	= 0x0004,
	gxSimpleGeometryTranslation	= 0x0008,						/* implies simple scaling */
	gxSimpleLinesTranslation	= 0x000C,						/* implies simple geometry & scaling */
	gxLayoutTextTranslation		= 0x0010,						/* turn on gxLine layout (normally off) */
	gxRasterTargetTranslation	= 0x0020,
	gxPostScriptTargetTranslation = 0x0040,
	gxVectorTargetTranslation	= 0x0080,
	gxPDDTargetTranslation		= 0x0100,
	gxDontConvertPatternsTranslation = 0x1000,
	gxDontSplitBitmapsTranslation = 0x2000
};

typedef long 							gxTranslationOption;

enum {
	gxContainsFormsBegin		= 0x0001,
	gxContainsFormsEnd			= 0x0002,
	gxContainsPostScript		= 0x0004,
	gxContainsEmptyPostScript	= 0x0008
};

typedef long 							gxTranslationStatistic;

enum {
	gxQuickDrawPictTag			= FOUR_CHAR_CODE('pict')
};


struct gxQuickDrawPict {
																/* translator inputs */
	gxTranslationOption 			options;
	Rect 							srcRect;
	Point 							styleStretch;

																/* size of quickdraw picture data */
	unsigned long 					dataLength;

																/* file alias */
	gxBitmapDataSourceAlias 		alias;
};
typedef struct gxQuickDrawPict			gxQuickDrawPict;
/* WindowRecord utilities */
EXTERN_API_C( gxViewPort )
GXNewWindowViewPort				(WindowPtr 				qdWindow)							THREEWORDINLINE(0x303C, 0x0236, 0xA832);

EXTERN_API_C( gxViewPort )
GXGetWindowViewPort				(WindowPtr 				qdWindow)							THREEWORDINLINE(0x303C, 0x0237, 0xA832);

EXTERN_API_C( WindowPtr )
GXGetViewPortWindow				(gxViewPort 			portOrder)							THREEWORDINLINE(0x303C, 0x0238, 0xA832);

/* GDevice utilities */
EXTERN_API_C( GDHandle )
GXGetViewDeviceGDevice			(gxViewDevice 			theDevice)							THREEWORDINLINE(0x303C, 0x0239, 0xA832);

EXTERN_API_C( gxViewDevice )
GXGetGDeviceViewDevice			(GDHandle 				qdGDevice)							THREEWORDINLINE(0x303C, 0x023A, 0xA832);

/* gxPoint utilities */
EXTERN_API_C( void )
GXConvertQDPoint				(const Point *			shortPt,
								 gxViewPort 			portOrder,
								 gxPoint *				fixedPt)							THREEWORDINLINE(0x303C, 0x023B, 0xA832);

typedef CALLBACK_API_C( OSErr , gxShapeSpoolProcPtr )(gxShape toSpool, long refCon);
/* printing utilities typedef */
typedef CALLBACK_API_C( void , gxUserViewPortFilterProcPtr )(gxShape toFilter, gxViewPort portOrder, long refCon);
typedef CALLBACK_API_C( long , gxConvertQDFontProcPtr )(gxStyle dst, long txFont, long txFace);
typedef STACK_UPP_TYPE(gxShapeSpoolProcPtr) 					gxShapeSpoolUPP;
typedef STACK_UPP_TYPE(gxUserViewPortFilterProcPtr) 			gxUserViewPortFilterUPP;
typedef STACK_UPP_TYPE(gxConvertQDFontProcPtr) 					gxConvertQDFontUPP;
enum { uppgxShapeSpoolProcInfo = 0x000003E1 }; 					/* 2_bytes Func(4_bytes, 4_bytes) */
enum { uppgxUserViewPortFilterProcInfo = 0x00000FC1 }; 			/* no_return_value Func(4_bytes, 4_bytes, 4_bytes) */
enum { uppgxConvertQDFontProcInfo = 0x00000FF1 }; 				/* 4_bytes Func(4_bytes, 4_bytes, 4_bytes) */
#define NewgxShapeSpoolProc(userRoutine) 						(gxShapeSpoolUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppgxShapeSpoolProcInfo, GetCurrentArchitecture())
#define NewgxUserViewPortFilterProc(userRoutine) 				(gxUserViewPortFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppgxUserViewPortFilterProcInfo, GetCurrentArchitecture())
#define NewgxConvertQDFontProc(userRoutine) 					(gxConvertQDFontUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppgxConvertQDFontProcInfo, GetCurrentArchitecture())
#define CallgxShapeSpoolProc(userRoutine, toSpool, refCon) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppgxShapeSpoolProcInfo, (toSpool), (refCon))
#define CallgxUserViewPortFilterProc(userRoutine, toFilter, portOrder, refCon)  CALL_THREE_PARAMETER_UPP((userRoutine), uppgxUserViewPortFilterProcInfo, (toFilter), (portOrder), (refCon))
#define CallgxConvertQDFontProc(userRoutine, dst, txFont, txFace)  CALL_THREE_PARAMETER_UPP((userRoutine), uppgxConvertQDFontProcInfo, (dst), (txFont), (txFace))

typedef gxShapeSpoolProcPtr 			gxShapeSpoolFunction;
typedef gxUserViewPortFilterProcPtr 	gxUserViewPortFilter;
typedef gxConvertQDFontProcPtr 			gxConvertQDFontFunction;
/* mouse utilities */
/* return mouse location in fixed-gxPoint global space */
EXTERN_API_C( void )
GXGetGlobalMouse				(gxPoint *				globalPt)							THREEWORDINLINE(0x303C, 0x023C, 0xA832);

/* return fixed-gxPoint local mouse (gxViewPort == 0 --> default) */
EXTERN_API_C( void )
GXGetViewPortMouse				(gxViewPort 			portOrder,
								 gxPoint *				localPt)							THREEWORDINLINE(0x303C, 0x023D, 0xA832);

/* printing utilities */
EXTERN_API_C( gxUserViewPortFilterUPP )
GXGetViewPortFilter				(gxViewPort 			portOrder,
								 long *					refCon)								THREEWORDINLINE(0x303C, 0x025E, 0xA832);

EXTERN_API_C( void )
GXSetViewPortFilter				(gxViewPort 			portOrder,
								 gxUserViewPortFilterUPP  filter,
								 long 					refCon)								THREEWORDINLINE(0x303C, 0x023E, 0xA832);

/* QD to QD GX Translator functions */
EXTERN_API_C( void )
GXInstallQDTranslator			(GrafPtr 				port,
								 gxTranslationOption 	options,
								 const Rect *			srcRect,
								 const Rect *			dstRect,
								 Point 					styleStrech,
								 gxShapeSpoolUPP 		userFunction,
								 void *					reference)							THREEWORDINLINE(0x303C, 0x023F, 0xA832);

EXTERN_API_C( gxTranslationStatistic )
GXRemoveQDTranslator			(GrafPtr 				port,
								 gxTranslationStatistic * statistic)						THREEWORDINLINE(0x303C, 0x0240, 0xA832);

EXTERN_API_C( gxShape )
GXConvertPICTToShape			(PicHandle 				pict,
								 gxTranslationOption 	options,
								 const Rect *			srcRect,
								 const Rect *			dstRect,
								 Point 					styleStretch,
								 gxShape 				destination,
								 gxTranslationStatistic * stats)							THREEWORDINLINE(0x303C, 0x0241, 0xA832);

/* Find the best GX style given a QD font and face. Called by the QD->GX translator */
EXTERN_API_C( long )
GXConvertQDFont					(gxStyle 				theStyle,
								 long 					txFont,
								 long 					txFace)								THREEWORDINLINE(0x303C, 0x0242, 0xA832);

EXTERN_API_C( gxConvertQDFontUPP )
GXGetConvertQDFont				(void)														THREEWORDINLINE(0x303C, 0x0243, 0xA832);

EXTERN_API_C( void )
GXSetConvertQDFont				(gxConvertQDFontUPP 	userFunction)						THREEWORDINLINE(0x303C, 0x0244, 0xA832);

typedef unsigned long 					gxProfilePoolAttributes;

struct gxFlatProfileListItem {
	gxProfilePoolAttributes 		attributes;					/* information about this particular profile's source*/
	CMProfileRef 					profileRef;					/* reference to profile, only valid before shape is disposed*/
	CMProfileIdentifier 			identifier;					/* information on how to find the profile upon unflattening*/
};
typedef struct gxFlatProfileListItem	gxFlatProfileListItem;

 
#if defined(__MWERKS__) && TARGET_CPU_68K
	#pragma pop
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

#endif /* __GXENVIRONMENT__ */

