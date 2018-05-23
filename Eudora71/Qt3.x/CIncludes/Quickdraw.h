/*
 	File:		Quickdraw.h
 
 	Contains:	Interface to Quickdraw Graphics
 
 	Version:	Technology:	Mac OS 8.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QUICKDRAW__
#define __QUICKDRAW__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif
#ifndef __QUICKDRAWTEXT__
#include <QuickdrawText.h>
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
	invalColReq					= -1							/*invalid color table request*/
};


enum {
																/* transfer modes */
	srcCopy						= 0,							/*the 16 transfer modes*/
	srcOr						= 1,
	srcXor						= 2,
	srcBic						= 3,
	notSrcCopy					= 4,
	notSrcOr					= 5,
	notSrcXor					= 6,
	notSrcBic					= 7,
	patCopy						= 8,
	patOr						= 9,
	patXor						= 10,
	patBic						= 11,
	notPatCopy					= 12,
	notPatOr					= 13,
	notPatXor					= 14,
	notPatBic					= 15,							/* Special Text Transfer Mode */
	grayishTextOr				= 49,
	hilitetransfermode			= 50,
	hilite						= 50,							/* Arithmetic transfer modes */
	blend						= 32,
	addPin						= 33,
	addOver						= 34,
	subPin						= 35,
	addMax						= 37,
	adMax						= 37,
	subOver						= 38,
	adMin						= 39,
	ditherCopy					= 64,							/* Transparent mode constant */
	transparent					= 36
};


enum {
	italicBit					= 1,
	ulineBit					= 2,
	outlineBit					= 3,
	shadowBit					= 4,
	condenseBit					= 5,
	extendBit					= 6
};


enum {
																/* QuickDraw color separation constants */
	normalBit					= 0,							/*normal screen mapping*/
	inverseBit					= 1,							/*inverse screen mapping*/
	redBit						= 4,							/*RGB additive mapping*/
	greenBit					= 3,
	blueBit						= 2,
	cyanBit						= 8,							/*CMYBk subtractive mapping*/
	magentaBit					= 7,
	yellowBit					= 6,
	blackBit					= 5
};


enum {
	blackColor					= 33,							/*colors expressed in these mappings*/
	whiteColor					= 30,
	redColor					= 205,
	greenColor					= 341,
	blueColor					= 409,
	cyanColor					= 273,
	magentaColor				= 137,
	yellowColor					= 69
};


enum {
	picLParen					= 0,							/*standard picture comments*/
	picRParen					= 1,
	clutType					= 0,							/*0 if lookup table*/
	fixedType					= 1,							/*1 if fixed table*/
	directType					= 2,							/*2 if direct values*/
	gdDevType					= 0								/*0 = monochrome 1 = color*/
};


enum {
	interlacedDevice			= 2,							/* 1 if single pixel lines look bad */
	roundedDevice				= 5,							/* 1 if device has been “rounded” into the GrayRgn */
	hasAuxMenuBar				= 6,							/* 1 if device has an aux menu bar on it */
	burstDevice					= 7,
	ext32Device					= 8,
	ramInit						= 10,							/*1 if initialized from 'scrn' resource*/
	mainScreen					= 11,							/* 1 if main screen */
	allInit						= 12,							/* 1 if all devices initialized */
	screenDevice				= 13,							/*1 if screen device [not used]*/
	noDriver					= 14,							/* 1 if no driver for this GDevice */
	screenActive				= 15,							/*1 if in use*/
	hiliteBit					= 7,							/*flag bit in HiliteMode (lowMem flag)*/
	pHiliteBit					= 0,							/*flag bit in HiliteMode used with BitClr procedure*/
	defQDColors					= 127,							/*resource ID of clut for default QDColors*/
																/* pixel type */
	RGBDirect					= 16,							/* 16 & 32 bits/pixel pixelType value */
																/* pmVersion values */
	baseAddr32					= 4								/*pixmap base address is 32-bit address*/
};



enum {
	sysPatListID				= 0,
	iBeamCursor					= 1,
	crossCursor					= 2,
	plusCursor					= 3,
	watchCursor					= 4
};


enum {
	kQDGrafVerbFrame			= 0,
	kQDGrafVerbPaint			= 1,
	kQDGrafVerbErase			= 2,
	kQDGrafVerbInvert			= 3,
	kQDGrafVerbFill				= 4
};

#if OLDROUTINENAMES

enum {
	frame						= kQDGrafVerbFrame,
	paint						= kQDGrafVerbPaint,
	erase						= kQDGrafVerbErase,
	invert						= kQDGrafVerbInvert,
	fill						= kQDGrafVerbFill
};

#endif  /* OLDROUTINENAMES */

typedef SInt8 							GrafVerb;

enum {
	chunky						= 0,
	chunkyPlanar				= 1,
	planar						= 2
};

typedef SInt8 							PixelType;
typedef short 							Bits16[16];

/***************   IMPORTANT NOTE REGARDING Pattern  **************************************
   Patterns were originally defined as:
   
		C: 			typedef unsigned char Pattern[8];
		Pascal:		Pattern = PACKED ARRAY [0..7] OF 0..255;
		
   The old array definition of Pattern would cause 68000 based CPU's to crash in certain circum-
   stances. The new struct definition is safe, but may require source code changes to compile.
	
*********************************************************************************************/

struct Pattern {
	UInt8 							pat[8];
};
typedef struct Pattern					Pattern;
/*
 ConstPatternParam is no longer needed.  It was first created when Pattern was an array.
 Now that Pattern is a struct, it is more straight forward to just add the "const" qualifier
 on the parameter type (e.g. "const Pattern * pat" instead of "ConstPatternParam pat").
*/

typedef const Pattern *					ConstPatternParam;
typedef Pattern *						PatPtr;
typedef PatPtr *						PatHandle;
typedef SignedByte 						QDByte;
typedef QDByte *						QDPtr;
typedef QDPtr *							QDHandle;
typedef short 							QDErr;

enum {
	singleDevicesBit			= 0,
	dontMatchSeedsBit			= 1,
	allDevicesBit				= 2
};


enum {
	singleDevices				= 1 << singleDevicesBit,
	dontMatchSeeds				= 1 << dontMatchSeedsBit,
	allDevices					= 1 << allDevicesBit
};

typedef unsigned long 					DeviceLoopFlags;

struct BitMap {
	Ptr 							baseAddr;
	short 							rowBytes;
	Rect 							bounds;
};
typedef struct BitMap					BitMap;
typedef BitMap *						BitMapPtr;
typedef BitMapPtr *						BitMapHandle;

struct Cursor {
	Bits16 							data;
	Bits16 							mask;
	Point 							hotSpot;
};
typedef struct Cursor					Cursor;

typedef Cursor *						CursPtr;
typedef CursPtr *						CursHandle;

struct PenState {
	Point 							pnLoc;
	Point 							pnSize;
	short 							pnMode;
	Pattern 						pnPat;
};
typedef struct PenState					PenState;

struct MacRegion {
	unsigned short 					rgnSize;					/*size in bytes*/
	Rect 							rgnBBox;					/*enclosing rectangle*/
};
typedef struct MacRegion				MacRegion;
/*
	The type name "Region" has a name space collision on Win32.
	Use MacRegion to be cross-platfrom safe.
*/
#if TARGET_OS_MAC

typedef MacRegion 						Region;
#endif  /* TARGET_OS_MAC */

typedef MacRegion *						RgnPtr;
typedef RgnPtr *						RgnHandle;

struct Picture {
	short 							picSize;
	Rect 							picFrame;
};
typedef struct Picture					Picture;

typedef Picture *						PicPtr;
typedef PicPtr *						PicHandle;

struct MacPolygon {
	short 							polySize;
	Rect 							polyBBox;
	Point 							polyPoints[1];
};
typedef struct MacPolygon				MacPolygon;
/*
	The type name "Polygon" has a name space collision on Win32.
	Use MacPolygon to be cross-platfrom safe.
*/
#if TARGET_OS_MAC

typedef MacPolygon 						Polygon;
#endif  /* TARGET_OS_MAC */

typedef MacPolygon *					PolyPtr;
typedef PolyPtr *						PolyHandle;
typedef CALLBACK_API( void , QDTextProcPtr )(short byteCount, Ptr textBuf, Point numer, Point denom);
typedef CALLBACK_API( void , QDLineProcPtr )(Point newPt);
typedef CALLBACK_API( void , QDRectProcPtr )(GrafVerb verb, Rect *r);
typedef CALLBACK_API( void , QDRRectProcPtr )(GrafVerb verb, Rect *r, short ovalWidth, short ovalHeight);
typedef CALLBACK_API( void , QDOvalProcPtr )(GrafVerb verb, Rect *r);
typedef CALLBACK_API( void , QDArcProcPtr )(GrafVerb verb, Rect *r, short startAngle, short arcAngle);
typedef CALLBACK_API( void , QDPolyProcPtr )(GrafVerb verb, PolyHandle poly);
typedef CALLBACK_API( void , QDRgnProcPtr )(GrafVerb verb, RgnHandle rgn);
typedef CALLBACK_API( void , QDBitsProcPtr )(BitMap *srcBits, Rect *srcRect, Rect *dstRect, short mode, RgnHandle maskRgn);
typedef CALLBACK_API( void , QDCommentProcPtr )(short kind, short dataSize, Handle dataHandle);
typedef CALLBACK_API( short , QDTxMeasProcPtr )(short byteCount, Ptr textAddr, Point *numer, Point *denom, FontInfo *info);
typedef CALLBACK_API( void , QDGetPicProcPtr )(Ptr dataPtr, short byteCount);
typedef CALLBACK_API( void , QDPutPicProcPtr )(Ptr dataPtr, short byteCount);
typedef CALLBACK_API( void , QDOpcodeProcPtr )(Rect *fromRect, Rect *toRect, short opcode, short version);
typedef CALLBACK_API_C( OSStatus , QDStdGlyphsProcPtr )(void *dataStream, ByteCount size);
typedef CALLBACK_API( void , QDJShieldCursorProcPtr )(short left, short top, short right, short bottom);
typedef STACK_UPP_TYPE(QDTextProcPtr) 							QDTextUPP;
typedef STACK_UPP_TYPE(QDLineProcPtr) 							QDLineUPP;
typedef STACK_UPP_TYPE(QDRectProcPtr) 							QDRectUPP;
typedef STACK_UPP_TYPE(QDRRectProcPtr) 							QDRRectUPP;
typedef STACK_UPP_TYPE(QDOvalProcPtr) 							QDOvalUPP;
typedef STACK_UPP_TYPE(QDArcProcPtr) 							QDArcUPP;
typedef STACK_UPP_TYPE(QDPolyProcPtr) 							QDPolyUPP;
typedef STACK_UPP_TYPE(QDRgnProcPtr) 							QDRgnUPP;
typedef STACK_UPP_TYPE(QDBitsProcPtr) 							QDBitsUPP;
typedef STACK_UPP_TYPE(QDCommentProcPtr) 						QDCommentUPP;
typedef STACK_UPP_TYPE(QDTxMeasProcPtr) 						QDTxMeasUPP;
typedef STACK_UPP_TYPE(QDGetPicProcPtr) 						QDGetPicUPP;
typedef STACK_UPP_TYPE(QDPutPicProcPtr) 						QDPutPicUPP;
typedef STACK_UPP_TYPE(QDOpcodeProcPtr) 						QDOpcodeUPP;
typedef STACK_UPP_TYPE(QDStdGlyphsProcPtr) 						QDStdGlyphsUPP;
typedef STACK_UPP_TYPE(QDJShieldCursorProcPtr) 					QDJShieldCursorUPP;

struct QDProcs {
	QDTextUPP 						textProc;
	QDLineUPP 						lineProc;
	QDRectUPP 						rectProc;
	QDRRectUPP 						rRectProc;
	QDOvalUPP 						ovalProc;
	QDArcUPP 						arcProc;
	QDPolyUPP 						polyProc;
	QDRgnUPP 						rgnProc;
	QDBitsUPP 						bitsProc;
	QDCommentUPP 					commentProc;
	QDTxMeasUPP 					txMeasProc;
	QDGetPicUPP 					getPicProc;
	QDPutPicUPP 					putPicProc;
};
typedef struct QDProcs					QDProcs;
typedef QDProcs *						QDProcsPtr;
enum { uppQDTextProcInfo = 0x00003F80 }; 						/* pascal no_return_value Func(2_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppQDLineProcInfo = 0x000000C0 }; 						/* pascal no_return_value Func(4_bytes) */
enum { uppQDRectProcInfo = 0x00000340 }; 						/* pascal no_return_value Func(1_byte, 4_bytes) */
enum { uppQDRRectProcInfo = 0x00002B40 }; 						/* pascal no_return_value Func(1_byte, 4_bytes, 2_bytes, 2_bytes) */
enum { uppQDOvalProcInfo = 0x00000340 }; 						/* pascal no_return_value Func(1_byte, 4_bytes) */
enum { uppQDArcProcInfo = 0x00002B40 }; 						/* pascal no_return_value Func(1_byte, 4_bytes, 2_bytes, 2_bytes) */
enum { uppQDPolyProcInfo = 0x00000340 }; 						/* pascal no_return_value Func(1_byte, 4_bytes) */
enum { uppQDRgnProcInfo = 0x00000340 }; 						/* pascal no_return_value Func(1_byte, 4_bytes) */
enum { uppQDBitsProcInfo = 0x0000EFC0 }; 						/* pascal no_return_value Func(4_bytes, 4_bytes, 4_bytes, 2_bytes, 4_bytes) */
enum { uppQDCommentProcInfo = 0x00000E80 }; 					/* pascal no_return_value Func(2_bytes, 2_bytes, 4_bytes) */
enum { uppQDTxMeasProcInfo = 0x0000FFA0 }; 						/* pascal 2_bytes Func(2_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppQDGetPicProcInfo = 0x000002C0 }; 						/* pascal no_return_value Func(4_bytes, 2_bytes) */
enum { uppQDPutPicProcInfo = 0x000002C0 }; 						/* pascal no_return_value Func(4_bytes, 2_bytes) */
enum { uppQDOpcodeProcInfo = 0x00002BC0 }; 						/* pascal no_return_value Func(4_bytes, 4_bytes, 2_bytes, 2_bytes) */
enum { uppQDStdGlyphsProcInfo = 0x000003F1 }; 					/* 4_bytes Func(4_bytes, 4_bytes) */
enum { uppQDJShieldCursorProcInfo = 0x00002A80 }; 				/* pascal no_return_value Func(2_bytes, 2_bytes, 2_bytes, 2_bytes) */
#define NewQDTextProc(userRoutine) 								(QDTextUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDTextProcInfo, GetCurrentArchitecture())
#define NewQDLineProc(userRoutine) 								(QDLineUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDLineProcInfo, GetCurrentArchitecture())
#define NewQDRectProc(userRoutine) 								(QDRectUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDRectProcInfo, GetCurrentArchitecture())
#define NewQDRRectProc(userRoutine) 							(QDRRectUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDRRectProcInfo, GetCurrentArchitecture())
#define NewQDOvalProc(userRoutine) 								(QDOvalUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDOvalProcInfo, GetCurrentArchitecture())
#define NewQDArcProc(userRoutine) 								(QDArcUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDArcProcInfo, GetCurrentArchitecture())
#define NewQDPolyProc(userRoutine) 								(QDPolyUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDPolyProcInfo, GetCurrentArchitecture())
#define NewQDRgnProc(userRoutine) 								(QDRgnUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDRgnProcInfo, GetCurrentArchitecture())
#define NewQDBitsProc(userRoutine) 								(QDBitsUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDBitsProcInfo, GetCurrentArchitecture())
#define NewQDCommentProc(userRoutine) 							(QDCommentUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDCommentProcInfo, GetCurrentArchitecture())
#define NewQDTxMeasProc(userRoutine) 							(QDTxMeasUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDTxMeasProcInfo, GetCurrentArchitecture())
#define NewQDGetPicProc(userRoutine) 							(QDGetPicUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDGetPicProcInfo, GetCurrentArchitecture())
#define NewQDPutPicProc(userRoutine) 							(QDPutPicUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDPutPicProcInfo, GetCurrentArchitecture())
#define NewQDOpcodeProc(userRoutine) 							(QDOpcodeUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDOpcodeProcInfo, GetCurrentArchitecture())
#define NewQDStdGlyphsProc(userRoutine) 						(QDStdGlyphsUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDStdGlyphsProcInfo, GetCurrentArchitecture())
#define NewQDJShieldCursorProc(userRoutine) 					(QDJShieldCursorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQDJShieldCursorProcInfo, GetCurrentArchitecture())
#define CallQDTextProc(userRoutine, byteCount, textBuf, numer, denom)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppQDTextProcInfo, (byteCount), (textBuf), (numer), (denom))
#define CallQDLineProc(userRoutine, newPt) 						CALL_ONE_PARAMETER_UPP((userRoutine), uppQDLineProcInfo, (newPt))
#define CallQDRectProc(userRoutine, verb, r) 					CALL_TWO_PARAMETER_UPP((userRoutine), uppQDRectProcInfo, (verb), (r))
#define CallQDRRectProc(userRoutine, verb, r, ovalWidth, ovalHeight)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppQDRRectProcInfo, (verb), (r), (ovalWidth), (ovalHeight))
#define CallQDOvalProc(userRoutine, verb, r) 					CALL_TWO_PARAMETER_UPP((userRoutine), uppQDOvalProcInfo, (verb), (r))
#define CallQDArcProc(userRoutine, verb, r, startAngle, arcAngle)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppQDArcProcInfo, (verb), (r), (startAngle), (arcAngle))
#define CallQDPolyProc(userRoutine, verb, poly) 				CALL_TWO_PARAMETER_UPP((userRoutine), uppQDPolyProcInfo, (verb), (poly))
#define CallQDRgnProc(userRoutine, verb, rgn) 					CALL_TWO_PARAMETER_UPP((userRoutine), uppQDRgnProcInfo, (verb), (rgn))
#define CallQDBitsProc(userRoutine, srcBits, srcRect, dstRect, mode, maskRgn)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppQDBitsProcInfo, (srcBits), (srcRect), (dstRect), (mode), (maskRgn))
#define CallQDCommentProc(userRoutine, kind, dataSize, dataHandle)  CALL_THREE_PARAMETER_UPP((userRoutine), uppQDCommentProcInfo, (kind), (dataSize), (dataHandle))
#define CallQDTxMeasProc(userRoutine, byteCount, textAddr, numer, denom, info)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppQDTxMeasProcInfo, (byteCount), (textAddr), (numer), (denom), (info))
#define CallQDGetPicProc(userRoutine, dataPtr, byteCount) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppQDGetPicProcInfo, (dataPtr), (byteCount))
#define CallQDPutPicProc(userRoutine, dataPtr, byteCount) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppQDPutPicProcInfo, (dataPtr), (byteCount))
#define CallQDOpcodeProc(userRoutine, fromRect, toRect, opcode, version)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppQDOpcodeProcInfo, (fromRect), (toRect), (opcode), (version))
#define CallQDStdGlyphsProc(userRoutine, dataStream, size) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppQDStdGlyphsProcInfo, (dataStream), (size))
#define CallQDJShieldCursorProc(userRoutine, left, top, right, bottom)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppQDJShieldCursorProcInfo, (left), (top), (right), (bottom))



struct GrafPort {
	short 							device;
	BitMap 							portBits;
	Rect 							portRect;
	RgnHandle 						visRgn;
	RgnHandle 						clipRgn;
	Pattern 						bkPat;
	Pattern 						fillPat;
	Point 							pnLoc;
	Point 							pnSize;
	short 							pnMode;
	Pattern 						pnPat;
	short 							pnVis;
	short 							txFont;
	StyleField 						txFace;						/*StyleField occupies 16-bits, but only first 8-bits are used*/
	short 							txMode;
	short 							txSize;
	Fixed 							spExtra;
	long 							fgColor;
	long 							bkColor;
	short 							colrBit;
	short 							patStretch;
	Handle 							picSave;
	Handle 							rgnSave;
	Handle 							polySave;
	QDProcsPtr 						grafProcs;
};
typedef struct GrafPort					GrafPort;

typedef GrafPort *						GrafPtr;

/*
 *	This set of definitions "belongs" in Windows.
 *	But, there is a circularity in the headers where Windows includes Controls and
 *	Controls includes Windows. To break the circle, the information
 *	needed by Controls is moved from Windows to Quickdraw.
 */
typedef GrafPtr 						WindowPtr;
typedef WindowPtr 						DialogPtr;
typedef WindowPtr 						WindowRef;
/* DragConstraint constants to pass to DragGray,DragTheRgn, or ConstrainedDragRgn*/
typedef UInt16 							DragConstraint;

enum {
	kNoConstraint				= 0,
	kVerticalConstraint			= 1,
	kHorizontalConstraint		= 2
};


typedef CALLBACK_API( void , DragGrayRgnProcPtr )(void );
/*
 *	Here ends the list of things that "belong" in Windows.
 */



struct RGBColor {
	unsigned short 					red;						/*magnitude of red component*/
	unsigned short 					green;						/*magnitude of green component*/
	unsigned short 					blue;						/*magnitude of blue component*/
};
typedef struct RGBColor					RGBColor;
typedef RGBColor *						RGBColorPtr;
typedef RGBColorPtr *					RGBColorHdl;
typedef CALLBACK_API( Boolean , ColorSearchProcPtr )(RGBColor *rgb, long *position);
typedef CALLBACK_API( Boolean , ColorComplementProcPtr )(RGBColor *rgb);
typedef STACK_UPP_TYPE(DragGrayRgnProcPtr) 						DragGrayRgnUPP;
typedef STACK_UPP_TYPE(ColorSearchProcPtr) 						ColorSearchUPP;
typedef STACK_UPP_TYPE(ColorComplementProcPtr) 					ColorComplementUPP;
enum { uppDragGrayRgnProcInfo = 0x00000000 }; 					/* pascal no_return_value Func() */
enum { uppColorSearchProcInfo = 0x000003D0 }; 					/* pascal 1_byte Func(4_bytes, 4_bytes) */
enum { uppColorComplementProcInfo = 0x000000D0 }; 				/* pascal 1_byte Func(4_bytes) */
#define NewDragGrayRgnProc(userRoutine) 						(DragGrayRgnUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDragGrayRgnProcInfo, GetCurrentArchitecture())
#define NewColorSearchProc(userRoutine) 						(ColorSearchUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppColorSearchProcInfo, GetCurrentArchitecture())
#define NewColorComplementProc(userRoutine) 					(ColorComplementUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppColorComplementProcInfo, GetCurrentArchitecture())
#define CallDragGrayRgnProc(userRoutine) 						CALL_ZERO_PARAMETER_UPP((userRoutine), uppDragGrayRgnProcInfo)
#define CallColorSearchProc(userRoutine, rgb, position) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppColorSearchProcInfo, (rgb), (position))
#define CallColorComplementProc(userRoutine, rgb) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppColorComplementProcInfo, (rgb))

struct ColorSpec {
	short 							value;						/*index or other value*/
	RGBColor 						rgb;						/*true color*/
};
typedef struct ColorSpec				ColorSpec;
typedef ColorSpec *						ColorSpecPtr;

typedef ColorSpec 						CSpecArray[1];

struct ColorTable {
	long 							ctSeed;						/*unique identifier for table*/
	short 							ctFlags;					/*high bit: 0 = PixMap; 1 = device*/
	short 							ctSize;						/*number of entries in CTTable*/
	CSpecArray 						ctTable;					/*array [0..0] of ColorSpec*/
};
typedef struct ColorTable				ColorTable;

typedef ColorTable *					CTabPtr;
typedef CTabPtr *						CTabHandle;

struct xColorSpec {
	short 							value;						/*index or other value*/
	RGBColor 						rgb;						/*true color*/
	short 							xalpha;
};
typedef struct xColorSpec				xColorSpec;
typedef xColorSpec *					xColorSpecPtr;

typedef xColorSpec 						xCSpecArray[1];

struct MatchRec {
	unsigned short 					red;
	unsigned short 					green;
	unsigned short 					blue;
	long 							matchData;
};
typedef struct MatchRec					MatchRec;
/*
	QuickTime 3.0 makes PixMap data structure available on non-Mac OS's.
	In order to implement PixMap in these alternate environments, the PixMap
	had to be extended. The pmReserved field was changed to pmExt which is
	a Handle to extra info.  The planeBytes field was changed to pixelFormat.
*/
#ifndef OLDPIXMAPSTRUCT
#if TARGET_OS_MAC
#define OLDPIXMAPSTRUCT 1
#else
#define OLDPIXMAPSTRUCT 0
#endif  /* TARGET_OS_MAC */

#endif  /*  ! defined(OLDPIXMAPSTRUCT)  */


#if OLDPIXMAPSTRUCT
	#define GETPIXMAPPIXELFORMAT(pm)		( (pm)->pixelSize )
#else
	#define GETPIXMAPPIXELFORMAT(pm)		( ((pm)->pixelFormat != 0) ? (pm)->pixelFormat : (pm)->pixelSize )
#endif


#if TARGET_OS_MAC || (NeXT && ppc)
    #define NON_MAC_PIXEL_FORMATS   0
#else
    #define NON_MAC_PIXEL_FORMATS   1
#endif

/* pixel formats*/

enum {
	k1MonochromePixelFormat		= 0x00000001,					/* 1 bit indexed*/
	k2IndexedPixelFormat		= 0x00000002,					/* 2 bit indexed*/
	k4IndexedPixelFormat		= 0x00000004,					/* 4 bit indexed*/
	k8IndexedPixelFormat		= 0x00000008,					/* 8 bit indexed*/
	k16BE555PixelFormat			= 0x00000010,					/* 16 bit BE rgb 555 (Mac)*/
	k24RGBPixelFormat			= 0x00000018,					/* 24 bit rgb */
	k32ARGBPixelFormat			= 0x00000020,					/* 32 bit argb	(Mac)*/
	k1IndexedGrayPixelFormat	= 0x00000021,					/* 1 bit indexed gray*/
	k2IndexedGrayPixelFormat	= 0x00000022,					/* 2 bit indexed gray*/
	k4IndexedGrayPixelFormat	= 0x00000024,					/* 4 bit indexed gray*/
	k8IndexedGrayPixelFormat	= 0x00000028					/* 8 bit indexed gray*/
};


/* values for PixMap.pixelFormat*/

enum {
	k16LE555PixelFormat			= FOUR_CHAR_CODE('L555'),		/* 16 bit LE rgb 555 (PC)*/
	k16BE565PixelFormat			= FOUR_CHAR_CODE('B565'),		/* 16 bit BE rgb 565*/
	k16LE565PixelFormat			= FOUR_CHAR_CODE('L565'),		/* 16 bit LE rgb 565*/
	k24BGRPixelFormat			= FOUR_CHAR_CODE('24BG'),		/* 24 bit bgr */
	k32BGRAPixelFormat			= FOUR_CHAR_CODE('BGRA'),		/* 32 bit bgra	(Matrox)*/
	k32ABGRPixelFormat			= FOUR_CHAR_CODE('ABGR'),		/* 32 bit abgr	*/
	k32RGBAPixelFormat			= FOUR_CHAR_CODE('RGBA'),		/* 32 bit rgba	*/
	kYUVSPixelFormat			= FOUR_CHAR_CODE('yuvs'),		/* YUV 4:2:2 byte ordering 16-unsigned = 'YUY2'*/
	kYUVUPixelFormat			= FOUR_CHAR_CODE('yuvu'),		/* YUV 4:2:2 byte ordering 16-signed*/
	kYVU9PixelFormat			= FOUR_CHAR_CODE('YVU9'),		/* YVU9 Planar	9*/
	kYUV411PixelFormat			= FOUR_CHAR_CODE('Y411'),		/* YUV 4:1:1 Interleaved	16*/
	kYVYU422PixelFormat			= FOUR_CHAR_CODE('YVYU'),		/* YVYU 4:2:2 byte ordering	16*/
	kUYVY422PixelFormat			= FOUR_CHAR_CODE('UYVY'),		/* UYVY 4:2:2 byte ordering	16*/
	kYUV211PixelFormat			= FOUR_CHAR_CODE('Y211')		/* YUV 2:1:1 Packed	8*/
};


#if !OLDPIXMAPSTRUCT

struct PixMapExtension {
	long 							extSize;					/*size of struct, duh!*/
	unsigned long 					pmBits;						/*pixmap attributes bitfield*/
	void *							pmGD;						/*this is a GDHandle*/
	long 							pmSeed;
	unsigned long 					reserved0;					/*reserved for future use*/
	unsigned long 					reserved1;
	unsigned long 					reserved2;
};
typedef struct PixMapExtension			PixMapExtension;

typedef PixMapExtension *				PixMapExtPtr;
typedef PixMapExtPtr *					PixMapExtHandle;
#endif  /*  !OLDPIXMAPSTRUCT */


struct PixMap {
	Ptr 							baseAddr;					/*pointer to pixels*/
	short 							rowBytes;					/*offset to next line*/
	Rect 							bounds;						/*encloses bitmap*/
	short 							pmVersion;					/*pixMap version number*/
	short 							packType;					/*defines packing format*/
	long 							packSize;					/*length of pixel data*/
	Fixed 							hRes;						/*horiz. resolution (ppi)*/
	Fixed 							vRes;						/*vert. resolution (ppi)*/
	short 							pixelType;					/*defines pixel type*/
	short 							pixelSize;					/*# bits in pixel*/
	short 							cmpCount;					/*# components in pixel*/
	short 							cmpSize;					/*# bits per component*/
#if OLDPIXMAPSTRUCT
	long 							planeBytes;					/*offset to next plane*/
	CTabHandle 						pmTable;					/*color map for this pixMap*/
	long 							pmReserved;
#else
	OSType 							pixelFormat;				/*fourCharCode representation*/
	CTabHandle 						pmTable;					/*color map for this pixMap*/
	PixMapExtHandle					pmExt;						/*Handle to pixMap extension*/
#endif
};
typedef struct PixMap					PixMap;
typedef PixMap *						PixMapPtr;
typedef PixMapPtr *						PixMapHandle;

struct PixPat {
	short 							patType;					/*type of pattern*/
	PixMapHandle 					patMap;						/*the pattern's pixMap*/
	Handle 							patData;					/*pixmap's data*/
	Handle 							patXData;					/*expanded Pattern data*/
	short 							patXValid;					/*flags whether expanded Pattern valid*/
	Handle 							patXMap;					/*Handle to expanded Pattern data*/
	Pattern 						pat1Data;					/*old-Style pattern/RGB color*/
};
typedef struct PixPat					PixPat;
typedef PixPat *						PixPatPtr;
typedef PixPatPtr *						PixPatHandle;

struct CCrsr {
	short 							crsrType;					/*type of cursor*/
	PixMapHandle 					crsrMap;					/*the cursor's pixmap*/
	Handle 							crsrData;					/*cursor's data*/
	Handle 							crsrXData;					/*expanded cursor data*/
	short 							crsrXValid;					/*depth of expanded data (0 if none)*/
	Handle 							crsrXHandle;				/*future use*/
	Bits16 							crsr1Data;					/*one-bit cursor*/
	Bits16 							crsrMask;					/*cursor's mask*/
	Point 							crsrHotSpot;				/*cursor's hotspot*/
	long 							crsrXTable;					/*private*/
	long 							crsrID;						/*private*/
};
typedef struct CCrsr					CCrsr;
typedef CCrsr *							CCrsrPtr;
typedef CCrsrPtr *						CCrsrHandle;
#if OLDROUTINELOCATIONS

struct CIcon {
	PixMap 							iconPMap;					/*the icon's pixMap*/
	BitMap 							iconMask;					/*the icon's mask*/
	BitMap 							iconBMap;					/*the icon's bitMap*/
	Handle 							iconData;					/*the icon's data*/
	short 							iconMaskData[1];			/*icon's mask and BitMap data*/
};
typedef struct CIcon					CIcon;
typedef CIcon *							CIconPtr;
typedef CIconPtr *						CIconHandle;
#endif  /* OLDROUTINELOCATIONS */


struct GammaTbl {
	short 							gVersion;					/*gamma version number*/
	short 							gType;						/*gamma data type*/
	short 							gFormulaSize;				/*Formula data size*/
	short 							gChanCnt;					/*number of channels of data*/
	short 							gDataCnt;					/*number of values/channel*/
	short 							gDataWidth;					/*bits/corrected value (data packed to next larger byte size)*/
	short 							gFormulaData[1];			/*data for formulas followed by gamma values*/
};
typedef struct GammaTbl					GammaTbl;
typedef GammaTbl *						GammaTblPtr;
typedef GammaTblPtr *					GammaTblHandle;

struct ITab {
	long 							iTabSeed;					/*copy of CTSeed from source CTable*/
	short 							iTabRes;					/*bits/channel resolution of iTable*/
	Byte 							iTTable[1];					/*byte colortable index values*/
};
typedef struct ITab						ITab;
typedef ITab *							ITabPtr;
typedef ITabPtr *						ITabHandle;

struct SProcRec {
	Handle 							nxtSrch;					/*SProcHndl Handle to next SProcRec*/
	ColorSearchUPP 					srchProc;					/*search procedure proc ptr*/
};
typedef struct SProcRec					SProcRec;

typedef SProcRec *						SProcPtr;
typedef SProcPtr *						SProcHndl;

struct CProcRec {
	Handle 							nxtComp;					/*CProcHndl Handle to next CProcRec*/
	ColorComplementUPP 				compProc;					/*complement procedure proc ptr*/
};
typedef struct CProcRec					CProcRec;

typedef CProcRec *						CProcPtr;
typedef CProcPtr *						CProcHndl;
/*
	QuickTime 3.0 makes GDevice data structure available on non-Mac OS's.
	In order to implement GDevice in these alternate environments, the GDevice
	had to be extended. The gdReserved field was changed to gdExt which is
	a Handle to extra info.  
*/
#ifndef OLDGDEVICESTRUCT
#if TARGET_OS_MAC
#define OLDGDEVICESTRUCT 1
#else
#define OLDGDEVICESTRUCT 0
#endif  /* TARGET_OS_MAC */

#endif  /*  ! defined(OLDGDEVICESTRUCT)  */


struct GDevice {
	short 							gdRefNum;					/*driver's unit number*/
	short 							gdID;						/*client ID for search procs*/
	short 							gdType;						/*fixed/CLUT/direct*/
	ITabHandle 						gdITable;					/*Handle to inverse lookup table*/
	short 							gdResPref;					/*preferred resolution of GDITable*/
	SProcHndl 						gdSearchProc;				/*search proc list head*/
	CProcHndl 						gdCompProc;					/*complement proc list*/
	short 							gdFlags;					/*grafDevice flags word*/
	PixMapHandle 					gdPMap;						/*describing pixMap*/
	long 							gdRefCon;					/*reference value*/
	Handle 							gdNextGD;					/*GDHandle Handle of next gDevice*/
	Rect 							gdRect;						/* device's bounds in global coordinates*/
	long 							gdMode;						/*device's current mode*/
	short 							gdCCBytes;					/*depth of expanded cursor data*/
	short 							gdCCDepth;					/*depth of expanded cursor data*/
	Handle 							gdCCXData;					/*Handle to cursor's expanded data*/
	Handle 							gdCCXMask;					/*Handle to cursor's expanded mask*/
#if OLDGDEVICESTRUCT
	long 							gdReserved;					/*future use. MUST BE 0*/
#else
	Handle 							gdExt;						/*QuickTime 3.0 private info*/
#endif
};
typedef struct GDevice					GDevice;

typedef GDevice *						GDPtr;
typedef GDPtr *							GDHandle;

struct GrafVars {
	RGBColor 						rgbOpColor;					/*color for addPin  subPin and average*/
	RGBColor 						rgbHiliteColor;				/*color for hiliting*/
	Handle 							pmFgColor;					/*palette Handle for foreground color*/
	short 							pmFgIndex;					/*index value for foreground*/
	Handle 							pmBkColor;					/*palette Handle for background color*/
	short 							pmBkIndex;					/*index value for background*/
	short 							pmFlags;					/*flags for Palette Manager*/
};
typedef struct GrafVars					GrafVars;

typedef GrafVars *						GVarPtr;
typedef GVarPtr *						GVarHandle;

struct CQDProcs {
	QDTextUPP 						textProc;
	QDLineUPP 						lineProc;
	QDRectUPP 						rectProc;
	QDRRectUPP 						rRectProc;
	QDOvalUPP 						ovalProc;
	QDArcUPP 						arcProc;
	QDPolyUPP 						polyProc;
	QDRgnUPP 						rgnProc;
	QDBitsUPP 						bitsProc;
	QDCommentUPP 					commentProc;
	QDTxMeasUPP 					txMeasProc;
	QDGetPicUPP 					getPicProc;
	QDPutPicUPP 					putPicProc;
	QDOpcodeUPP 					opcodeProc;
	UniversalProcPtr 				newProc1;					/* this is the StdPix bottleneck -- see ImageCompression.h */
	QDStdGlyphsUPP 					glyphsProc;					/* was newProc2; now used in Unicode text drawing */
	UniversalProcPtr 				newProc3;
	UniversalProcPtr 				newProc4;
	UniversalProcPtr 				newProc5;
	UniversalProcPtr 				newProc6;
};
typedef struct CQDProcs					CQDProcs;
typedef CQDProcs *						CQDProcsPtr;

struct CGrafPort {
	short 							device;
	PixMapHandle 					portPixMap;					/*port's pixel map*/
	short 							portVersion;				/*high 2 bits always set*/
	Handle 							grafVars;					/*Handle to more fields*/
	short 							chExtra;					/*character extra*/
	short 							pnLocHFrac;					/*pen fraction*/
	Rect 							portRect;
	RgnHandle 						visRgn;
	RgnHandle 						clipRgn;
	PixPatHandle 					bkPixPat;					/*background pattern*/
	RGBColor 						rgbFgColor;					/*RGB components of fg*/
	RGBColor 						rgbBkColor;					/*RGB components of bk*/
	Point 							pnLoc;
	Point 							pnSize;
	short 							pnMode;
	PixPatHandle 					pnPixPat;					/*pen's pattern*/
	PixPatHandle 					fillPixPat;					/*fill pattern*/
	short 							pnVis;
	short 							txFont;
	StyleField 						txFace;						/*StyleField occupies 16-bits, but only first 8-bits are used*/
	short 							txMode;
	short 							txSize;
	Fixed 							spExtra;
	long 							fgColor;
	long 							bkColor;
	short 							colrBit;
	short 							patStretch;
	Handle 							picSave;
	Handle 							rgnSave;
	Handle 							polySave;
	CQDProcsPtr 					grafProcs;
};
typedef struct CGrafPort				CGrafPort;

typedef CGrafPort *						CGrafPtr;
typedef CGrafPtr 						CWindowPtr;

struct ReqListRec {
	short 							reqLSize;					/*request list size*/
	short 							reqLData[1];				/*request list data*/
};
typedef struct ReqListRec				ReqListRec;

struct OpenCPicParams {
	Rect 							srcRect;
	Fixed 							hRes;
	Fixed 							vRes;
	short 							version;
	short 							reserved1;
	long 							reserved2;
};
typedef struct OpenCPicParams			OpenCPicParams;

enum {
	kCursorImageMajorVersion	= 0x0001,
	kCursorImageMinorVersion	= 0x0000
};


struct CursorImageRec {
	UInt16 							majorVersion;
	UInt16 							minorVersion;
	PixMapHandle 					cursorPixMap;
	BitMapHandle 					cursorBitMask;
};
typedef struct CursorImageRec			CursorImageRec;

typedef CursorImageRec *				CursorImagePtr;
typedef CALLBACK_API( void , DeviceLoopDrawingProcPtr )(short depth, short deviceFlags, GDHandle targetDevice, long userData);
typedef STACK_UPP_TYPE(DeviceLoopDrawingProcPtr) 				DeviceLoopDrawingUPP;
enum { uppDeviceLoopDrawingProcInfo = 0x00003E80 }; 			/* pascal no_return_value Func(2_bytes, 2_bytes, 4_bytes, 4_bytes) */
#define NewDeviceLoopDrawingProc(userRoutine) 					(DeviceLoopDrawingUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDeviceLoopDrawingProcInfo, GetCurrentArchitecture())
#define CallDeviceLoopDrawingProc(userRoutine, depth, deviceFlags, targetDevice, userData)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppDeviceLoopDrawingProcInfo, (depth), (deviceFlags), (targetDevice), (userData))

struct QDGlobals {
	char 							privates[76];
	long 							randSeed;
	BitMap 							screenBits;
	Cursor 							arrow;
	Pattern 						dkGray;
	Pattern 						ltGray;
	Pattern 						gray;
	Pattern 						black;
	Pattern 						white;
	GrafPtr 						thePort;
};
typedef struct QDGlobals				QDGlobals;
typedef QDGlobals *						QDGlobalsPtr;
typedef QDGlobalsPtr *					QDGlobalsHdl;
extern QDGlobals qd;



EXTERN_API( void )
InitGraf						(void *					globalPtr)							ONEWORDINLINE(0xA86E);

EXTERN_API( void )
OpenPort						(GrafPtr 				port)								ONEWORDINLINE(0xA86F);

EXTERN_API( void )
InitPort						(GrafPtr 				port)								ONEWORDINLINE(0xA86D);

EXTERN_API( void )
ClosePort						(GrafPtr 				port)								ONEWORDINLINE(0xA87D);

#if !TARGET_OS_MAC
EXTERN_API( OSErr )
LockPortBits					(GrafPtr 				port);

EXTERN_API( OSErr )
UnlockPortBits					(GrafPtr 				port);

EXTERN_API( OSErr )
UpdatePort						(GrafPtr 				port);

EXTERN_API( void *)
GetPortNativeWindow				(GrafPtr 				macPort);

EXTERN_API( GrafPtr )
GetNativeWindowPort				(void *					nativeWindow);

EXTERN_API( void *)
MacRegionToNativeRegion			(RgnHandle 				macRegion);

EXTERN_API( RgnHandle )
NativeRegionToMacRegion			(void *					nativeRegion);

#if TARGET_OS_WIN32
EXTERN_API( void *)
GetPortHWND						(GrafPtr 				port);

EXTERN_API( GrafPtr )
GetHWNDPort						(void *					theHWND);

#define GetPortHWND(port)  GetPortNativeWindow(port)
#define GetHWNDPort(theHWND) GetNativeWindowPort(theHWND)
EXTERN_API( void *)
GetPortHDC						(GrafPtr 				port);

EXTERN_API( void *)
GetPortHBITMAP					(GrafPtr 				port);

EXTERN_API( void *)
GetPortHPALETTE					(GrafPtr 				port);

EXTERN_API( void *)
GetPortHFONT					(GrafPtr 				port);

EXTERN_API( void *)
GetDIBFromPICT					(PicHandle 				hPict);

EXTERN_API( PicHandle )
GetPICTFromDIB					(void *					h);

#endif  /* TARGET_OS_WIN32 */

#endif  /*  !TARGET_OS_MAC */

#if TARGET_OS_MAC
	#define MacSetPort SetPort
#endif
EXTERN_API( void )
MacSetPort						(GrafPtr 				port)								ONEWORDINLINE(0xA873);

EXTERN_API( void )
GetPort							(GrafPtr *				port)								ONEWORDINLINE(0xA874);

EXTERN_API( void )
GrafDevice						(short 					device)								ONEWORDINLINE(0xA872);

EXTERN_API( void )
SetPortBits						(const BitMap *			bm)									ONEWORDINLINE(0xA875);

EXTERN_API( void )
PortSize						(short 					width,
								 short 					height)								ONEWORDINLINE(0xA876);

EXTERN_API( void )
MovePortTo						(short 					leftGlobal,
								 short 					topGlobal)							ONEWORDINLINE(0xA877);

EXTERN_API( void )
SetOrigin						(short 					h,
								 short 					v)									ONEWORDINLINE(0xA878);

EXTERN_API( void )
SetClip							(RgnHandle 				rgn)								ONEWORDINLINE(0xA879);

EXTERN_API( void )
GetClip							(RgnHandle 				rgn)								ONEWORDINLINE(0xA87A);

EXTERN_API( void )
ClipRect						(const Rect *			r)									ONEWORDINLINE(0xA87B);

EXTERN_API( void )
BackPat							(const Pattern *		pat)								ONEWORDINLINE(0xA87C);

EXTERN_API( void )
InitCursor						(void)														ONEWORDINLINE(0xA850);

#if TARGET_OS_MAC
	#define MacSetCursor SetCursor
#endif
EXTERN_API( void )
MacSetCursor					(const Cursor *			crsr)								ONEWORDINLINE(0xA851);

EXTERN_API( void )
HideCursor						(void)														ONEWORDINLINE(0xA852);

#if TARGET_OS_MAC
	#define MacShowCursor ShowCursor
#endif
EXTERN_API( void )
MacShowCursor					(void)														ONEWORDINLINE(0xA853);

EXTERN_API( void )
ObscureCursor					(void)														ONEWORDINLINE(0xA856);

EXTERN_API( void )
HidePen							(void)														ONEWORDINLINE(0xA896);

EXTERN_API( void )
ShowPen							(void)														ONEWORDINLINE(0xA897);

EXTERN_API( void )
GetPen							(Point *				pt)									ONEWORDINLINE(0xA89A);

EXTERN_API( void )
GetPenState						(PenState *				pnState)							ONEWORDINLINE(0xA898);

EXTERN_API( void )
SetPenState						(const PenState *		pnState)							ONEWORDINLINE(0xA899);

EXTERN_API( void )
PenSize							(short 					width,
								 short 					height)								ONEWORDINLINE(0xA89B);

EXTERN_API( void )
PenMode							(short 					mode)								ONEWORDINLINE(0xA89C);

EXTERN_API( void )
PenPat							(const Pattern *		pat)								ONEWORDINLINE(0xA89D);

EXTERN_API( void )
PenNormal						(void)														ONEWORDINLINE(0xA89E);

EXTERN_API( void )
MoveTo							(short 					h,
								 short 					v)									ONEWORDINLINE(0xA893);

EXTERN_API( void )
Move							(short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA894);

#if TARGET_OS_MAC
	#define MacLineTo LineTo
#endif
EXTERN_API( void )
MacLineTo						(short 					h,
								 short 					v)									ONEWORDINLINE(0xA891);

EXTERN_API( void )
Line							(short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA892);

EXTERN_API( void )
ForeColor						(long 					color)								ONEWORDINLINE(0xA862);

EXTERN_API( void )
BackColor						(long 					color)								ONEWORDINLINE(0xA863);

EXTERN_API( void )
ColorBit						(short 					whichBit)							ONEWORDINLINE(0xA864);

#if TARGET_OS_MAC
	#define MacSetRect SetRect
#endif
EXTERN_API( void )
MacSetRect						(Rect *					r,
								 short 					left,
								 short 					top,
								 short 					right,
								 short 					bottom)								ONEWORDINLINE(0xA8A7);

#if TARGET_OS_MAC
	#define MacOffsetRect OffsetRect
#endif
EXTERN_API( void )
MacOffsetRect					(Rect *					r,
								 short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA8A8);

#if TARGET_OS_MAC
	#define MacInsetRect InsetRect
#endif
EXTERN_API( void )
MacInsetRect					(Rect *					r,
								 short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA8A9);

EXTERN_API( Boolean )
SectRect						(const Rect *			src1,
								 const Rect *			src2,
								 Rect *					dstRect)							ONEWORDINLINE(0xA8AA);

#if TARGET_OS_MAC
	#define MacUnionRect UnionRect
#endif
EXTERN_API( void )
MacUnionRect					(const Rect *			src1,
								 const Rect *			src2,
								 Rect *					dstRect)							ONEWORDINLINE(0xA8AB);

#if TARGET_OS_MAC
	#define MacEqualRect EqualRect
#endif
EXTERN_API( Boolean )
MacEqualRect					(const Rect *			rect1,
								 const Rect *			rect2)								ONEWORDINLINE(0xA8A6);

EXTERN_API( Boolean )
EmptyRect						(const Rect *			r)									ONEWORDINLINE(0xA8AE);

#if TARGET_OS_MAC
	#define MacFrameRect FrameRect
#endif
EXTERN_API( void )
MacFrameRect					(const Rect *			r)									ONEWORDINLINE(0xA8A1);

EXTERN_API( void )
PaintRect						(const Rect *			r)									ONEWORDINLINE(0xA8A2);

EXTERN_API( void )
EraseRect						(const Rect *			r)									ONEWORDINLINE(0xA8A3);

#if TARGET_OS_MAC
	#define MacInvertRect InvertRect
#endif
EXTERN_API( void )
MacInvertRect					(const Rect *			r)									ONEWORDINLINE(0xA8A4);

#if TARGET_OS_MAC
	#define MacFillRect FillRect
#endif
EXTERN_API( void )
MacFillRect						(const Rect *			r,
								 const Pattern *		pat)								ONEWORDINLINE(0xA8A5);

EXTERN_API( void )
FrameOval						(const Rect *			r)									ONEWORDINLINE(0xA8B7);

EXTERN_API( void )
PaintOval						(const Rect *			r)									ONEWORDINLINE(0xA8B8);

EXTERN_API( void )
EraseOval						(const Rect *			r)									ONEWORDINLINE(0xA8B9);

EXTERN_API( void )
InvertOval						(const Rect *			r)									ONEWORDINLINE(0xA8BA);

EXTERN_API( void )
FillOval						(const Rect *			r,
								 const Pattern *		pat)								ONEWORDINLINE(0xA8BB);

EXTERN_API( void )
FrameRoundRect					(const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight)							ONEWORDINLINE(0xA8B0);

EXTERN_API( void )
PaintRoundRect					(const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight)							ONEWORDINLINE(0xA8B1);

EXTERN_API( void )
EraseRoundRect					(const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight)							ONEWORDINLINE(0xA8B2);

EXTERN_API( void )
InvertRoundRect					(const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight)							ONEWORDINLINE(0xA8B3);

EXTERN_API( void )
FillRoundRect					(const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight,
								 const Pattern *		pat)								ONEWORDINLINE(0xA8B4);

EXTERN_API( void )
FrameArc						(const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle)							ONEWORDINLINE(0xA8BE);

EXTERN_API( void )
PaintArc						(const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle)							ONEWORDINLINE(0xA8BF);

EXTERN_API( void )
EraseArc						(const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle)							ONEWORDINLINE(0xA8C0);

EXTERN_API( void )
InvertArc						(const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle)							ONEWORDINLINE(0xA8C1);

EXTERN_API( void )
FillArc							(const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle,
								 const Pattern *		pat)								ONEWORDINLINE(0xA8C2);

EXTERN_API( RgnHandle )
NewRgn							(void)														ONEWORDINLINE(0xA8D8);

EXTERN_API( void )
OpenRgn							(void)														ONEWORDINLINE(0xA8DA);

EXTERN_API( void )
CloseRgn						(RgnHandle 				dstRgn)								ONEWORDINLINE(0xA8DB);

EXTERN_API( OSErr )
BitMapToRegion					(RgnHandle 				region,
								 const BitMap *			bMap)								ONEWORDINLINE(0xA8D7);

EXTERN_API( void )
DisposeRgn						(RgnHandle 				rgn)								ONEWORDINLINE(0xA8D9);

#if TARGET_OS_MAC
	#define MacCopyRgn CopyRgn
#endif
EXTERN_API( void )
MacCopyRgn						(RgnHandle 				srcRgn,
								 RgnHandle 				dstRgn)								ONEWORDINLINE(0xA8DC);

EXTERN_API( void )
SetEmptyRgn						(RgnHandle 				rgn)								ONEWORDINLINE(0xA8DD);

#if TARGET_OS_MAC
	#define MacSetRectRgn SetRectRgn
#endif
EXTERN_API( void )
MacSetRectRgn					(RgnHandle 				rgn,
								 short 					left,
								 short 					top,
								 short 					right,
								 short 					bottom)								ONEWORDINLINE(0xA8DE);

EXTERN_API( void )
RectRgn							(RgnHandle 				rgn,
								 const Rect *			r)									ONEWORDINLINE(0xA8DF);

#if TARGET_OS_MAC
	#define MacOffsetRgn OffsetRgn
#endif
EXTERN_API( void )
MacOffsetRgn					(RgnHandle 				rgn,
								 short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA8E0);

EXTERN_API( void )
InsetRgn						(RgnHandle 				rgn,
								 short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA8E1);

EXTERN_API( void )
SectRgn							(RgnHandle 				srcRgnA,
								 RgnHandle 				srcRgnB,
								 RgnHandle 				dstRgn)								ONEWORDINLINE(0xA8E4);

#if TARGET_OS_MAC
	#define MacUnionRgn UnionRgn
#endif
EXTERN_API( void )
MacUnionRgn						(RgnHandle 				srcRgnA,
								 RgnHandle 				srcRgnB,
								 RgnHandle 				dstRgn)								ONEWORDINLINE(0xA8E5);

EXTERN_API( void )
DiffRgn							(RgnHandle 				srcRgnA,
								 RgnHandle 				srcRgnB,
								 RgnHandle 				dstRgn)								ONEWORDINLINE(0xA8E6);

#if TARGET_OS_MAC
	#define MacXorRgn XorRgn
#endif
EXTERN_API( void )
MacXorRgn						(RgnHandle 				srcRgnA,
								 RgnHandle 				srcRgnB,
								 RgnHandle 				dstRgn)								ONEWORDINLINE(0xA8E7);

EXTERN_API( Boolean )
RectInRgn						(const Rect *			r,
								 RgnHandle 				rgn)								ONEWORDINLINE(0xA8E9);

#if TARGET_OS_MAC
	#define MacEqualRgn EqualRgn
#endif
EXTERN_API( Boolean )
MacEqualRgn						(RgnHandle 				rgnA,
								 RgnHandle 				rgnB)								ONEWORDINLINE(0xA8E3);

EXTERN_API( Boolean )
EmptyRgn						(RgnHandle 				rgn)								ONEWORDINLINE(0xA8E2);

#if TARGET_OS_MAC
	#define MacFrameRgn FrameRgn
#endif
EXTERN_API( void )
MacFrameRgn						(RgnHandle 				rgn)								ONEWORDINLINE(0xA8D2);

#if TARGET_OS_MAC
	#define MacPaintRgn PaintRgn
#endif
EXTERN_API( void )
MacPaintRgn						(RgnHandle 				rgn)								ONEWORDINLINE(0xA8D3);

EXTERN_API( void )
EraseRgn						(RgnHandle 				rgn)								ONEWORDINLINE(0xA8D4);

#if TARGET_OS_MAC
	#define MacInvertRgn InvertRgn
#endif
EXTERN_API( void )
MacInvertRgn					(RgnHandle 				rgn)								ONEWORDINLINE(0xA8D5);

#if TARGET_OS_MAC
	#define MacFillRgn FillRgn
#endif
EXTERN_API( void )
MacFillRgn						(RgnHandle 				rgn,
								 const Pattern *		pat)								ONEWORDINLINE(0xA8D6);

EXTERN_API( void )
ScrollRect						(const Rect *			r,
								 short 					dh,
								 short 					dv,
								 RgnHandle 				updateRgn)							ONEWORDINLINE(0xA8EF);

EXTERN_API( void )
CopyBits						(const BitMap *			srcBits,
								 const BitMap *			dstBits,
								 const Rect *			srcRect,
								 const Rect *			dstRect,
								 short 					mode,
								 RgnHandle 				maskRgn)							ONEWORDINLINE(0xA8EC);

EXTERN_API( void )
SeedFill						(const void *			srcPtr,
								 void *					dstPtr,
								 short 					srcRow,
								 short 					dstRow,
								 short 					height,
								 short 					words,
								 short 					seedH,
								 short 					seedV)								ONEWORDINLINE(0xA839);

EXTERN_API( void )
CalcMask						(const void *			srcPtr,
								 void *					dstPtr,
								 short 					srcRow,
								 short 					dstRow,
								 short 					height,
								 short 					words)								ONEWORDINLINE(0xA838);

EXTERN_API( void )
CopyMask						(const BitMap *			srcBits,
								 const BitMap *			maskBits,
								 const BitMap *			dstBits,
								 const Rect *			srcRect,
								 const Rect *			maskRect,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA817);

EXTERN_API( PicHandle )
OpenPicture						(const Rect *			picFrame)							ONEWORDINLINE(0xA8F3);

EXTERN_API( void )
PicComment						(short 					kind,
								 short 					dataSize,
								 Handle 				dataHandle)							ONEWORDINLINE(0xA8F2);

EXTERN_API( void )
ClosePicture					(void)														ONEWORDINLINE(0xA8F4);

EXTERN_API( void )
DrawPicture						(PicHandle 				myPicture,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA8F6);

EXTERN_API( void )
KillPicture						(PicHandle 				myPicture)							ONEWORDINLINE(0xA8F5);

EXTERN_API( PolyHandle )
OpenPoly						(void)														ONEWORDINLINE(0xA8CB);

EXTERN_API( void )
ClosePoly						(void)														ONEWORDINLINE(0xA8CC);

EXTERN_API( void )
KillPoly						(PolyHandle 			poly)								ONEWORDINLINE(0xA8CD);

EXTERN_API( void )
OffsetPoly						(PolyHandle 			poly,
								 short 					dh,
								 short 					dv)									ONEWORDINLINE(0xA8CE);

EXTERN_API( void )
FramePoly						(PolyHandle 			poly)								ONEWORDINLINE(0xA8C6);

EXTERN_API( void )
PaintPoly						(PolyHandle 			poly)								ONEWORDINLINE(0xA8C7);

EXTERN_API( void )
ErasePoly						(PolyHandle 			poly)								ONEWORDINLINE(0xA8C8);

EXTERN_API( void )
InvertPoly						(PolyHandle 			poly)								ONEWORDINLINE(0xA8C9);

EXTERN_API( void )
FillPoly						(PolyHandle 			poly,
								 const Pattern *		pat)								ONEWORDINLINE(0xA8CA);

EXTERN_API( void )
SetPt							(Point *				pt,
								 short 					h,
								 short 					v)									ONEWORDINLINE(0xA880);

EXTERN_API( void )
LocalToGlobal					(Point *				pt)									ONEWORDINLINE(0xA870);

EXTERN_API( void )
GlobalToLocal					(Point *				pt)									ONEWORDINLINE(0xA871);

EXTERN_API( short )
Random							(void)														ONEWORDINLINE(0xA861);

EXTERN_API( void )
StuffHex						(void *					thingPtr,
								 ConstStr255Param 		s)									ONEWORDINLINE(0xA866);

#if TARGET_OS_MAC
	#define MacGetPixel GetPixel
#endif
EXTERN_API( Boolean )
MacGetPixel						(short 					h,
								 short 					v)									ONEWORDINLINE(0xA865);

EXTERN_API( void )
ScalePt							(Point *				pt,
								 const Rect *			srcRect,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA8F8);

EXTERN_API( void )
MapPt							(Point *				pt,
								 const Rect *			srcRect,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA8F9);

EXTERN_API( void )
MapRect							(Rect *					r,
								 const Rect *			srcRect,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA8FA);

EXTERN_API( void )
MapRgn							(RgnHandle 				rgn,
								 const Rect *			srcRect,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA8FB);

EXTERN_API( void )
MapPoly							(PolyHandle 			poly,
								 const Rect *			srcRect,
								 const Rect *			dstRect)							ONEWORDINLINE(0xA8FC);

EXTERN_API( void )
SetStdProcs						(QDProcs *				procs)								ONEWORDINLINE(0xA8EA);

EXTERN_API( void )
StdRect							(GrafVerb 				verb,
								 const Rect *			r)									ONEWORDINLINE(0xA8A0);

EXTERN_API( void )
StdRRect						(GrafVerb 				verb,
								 const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight)							ONEWORDINLINE(0xA8AF);

EXTERN_API( void )
StdOval							(GrafVerb 				verb,
								 const Rect *			r)									ONEWORDINLINE(0xA8B6);

EXTERN_API( void )
StdArc							(GrafVerb 				verb,
								 const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle)							ONEWORDINLINE(0xA8BD);

EXTERN_API( void )
StdPoly							(GrafVerb 				verb,
								 PolyHandle 			poly)								ONEWORDINLINE(0xA8C5);

EXTERN_API( void )
StdRgn							(GrafVerb 				verb,
								 RgnHandle 				rgn)								ONEWORDINLINE(0xA8D1);

EXTERN_API( void )
StdBits							(const BitMap *			srcBits,
								 const Rect *			srcRect,
								 const Rect *			dstRect,
								 short 					mode,
								 RgnHandle 				maskRgn)							ONEWORDINLINE(0xA8EB);

EXTERN_API( void )
StdComment						(short 					kind,
								 short 					dataSize,
								 Handle 				dataHandle)							ONEWORDINLINE(0xA8F1);

EXTERN_API( void )
StdGetPic						(void *					dataPtr,
								 short 					byteCount)							ONEWORDINLINE(0xA8EE);

EXTERN_API( void )
StdPutPic						(const void *			dataPtr,
								 short 					byteCount)							ONEWORDINLINE(0xA8F0);

EXTERN_API( void )
StdOpcode						(const Rect *			fromRect,
								 const Rect *			toRect,
								 UInt16 				opcode,
								 SInt16 				version)							ONEWORDINLINE(0xABF8);

EXTERN_API( void )
AddPt							(Point 					src,
								 Point *				dst)								ONEWORDINLINE(0xA87E);

EXTERN_API( Boolean )
EqualPt							(Point 					pt1,
								 Point 					pt2)								ONEWORDINLINE(0xA881);

#if TARGET_OS_MAC
	#define MacPtInRect PtInRect
#endif
EXTERN_API( Boolean )
MacPtInRect						(Point 					pt,
								 const Rect *			r)									ONEWORDINLINE(0xA8AD);

EXTERN_API( void )
Pt2Rect							(Point 					pt1,
								 Point 					pt2,
								 Rect *					dstRect)							ONEWORDINLINE(0xA8AC);

EXTERN_API( void )
PtToAngle						(const Rect *			r,
								 Point 					pt,
								 short *				angle)								ONEWORDINLINE(0xA8C3);

EXTERN_API( void )
SubPt							(Point 					src,
								 Point *				dst)								ONEWORDINLINE(0xA87F);

EXTERN_API( Boolean )
PtInRgn							(Point 					pt,
								 RgnHandle 				rgn)								ONEWORDINLINE(0xA8E8);

EXTERN_API( void )
StdLine							(Point 					newPt)								ONEWORDINLINE(0xA890);

EXTERN_API( void )
OpenCPort						(CGrafPtr 				port)								ONEWORDINLINE(0xAA00);

EXTERN_API( void )
InitCPort						(CGrafPtr 				port)								ONEWORDINLINE(0xAA01);

EXTERN_API( void )
CloseCPort						(CGrafPtr 				port)								ONEWORDINLINE(0xAA02);

EXTERN_API( PixMapHandle )
NewPixMap						(void)														ONEWORDINLINE(0xAA03);

EXTERN_API( void )
DisposePixMap					(PixMapHandle 			pm)									ONEWORDINLINE(0xAA04);

EXTERN_API( void )
CopyPixMap						(PixMapHandle 			srcPM,
								 PixMapHandle 			dstPM)								ONEWORDINLINE(0xAA05);

EXTERN_API( PixPatHandle )
NewPixPat						(void)														ONEWORDINLINE(0xAA07);

EXTERN_API( void )
DisposePixPat					(PixPatHandle 			pp)									ONEWORDINLINE(0xAA08);

EXTERN_API( void )
CopyPixPat						(PixPatHandle 			srcPP,
								 PixPatHandle 			dstPP)								ONEWORDINLINE(0xAA09);

EXTERN_API( void )
PenPixPat						(PixPatHandle 			pp)									ONEWORDINLINE(0xAA0A);

EXTERN_API( void )
BackPixPat						(PixPatHandle 			pp)									ONEWORDINLINE(0xAA0B);

EXTERN_API( PixPatHandle )
GetPixPat						(short 					patID)								ONEWORDINLINE(0xAA0C);

EXTERN_API( void )
MakeRGBPat						(PixPatHandle 			pp,
								 const RGBColor *		myColor)							ONEWORDINLINE(0xAA0D);

EXTERN_API( void )
FillCRect						(const Rect *			r,
								 PixPatHandle 			pp)									ONEWORDINLINE(0xAA0E);

EXTERN_API( void )
FillCOval						(const Rect *			r,
								 PixPatHandle 			pp)									ONEWORDINLINE(0xAA0F);

EXTERN_API( void )
FillCRoundRect					(const Rect *			r,
								 short 					ovalWidth,
								 short 					ovalHeight,
								 PixPatHandle 			pp)									ONEWORDINLINE(0xAA10);

EXTERN_API( void )
FillCArc						(const Rect *			r,
								 short 					startAngle,
								 short 					arcAngle,
								 PixPatHandle 			pp)									ONEWORDINLINE(0xAA11);

EXTERN_API( void )
FillCRgn						(RgnHandle 				rgn,
								 PixPatHandle 			pp)									ONEWORDINLINE(0xAA12);

EXTERN_API( void )
FillCPoly						(PolyHandle 			poly,
								 PixPatHandle 			pp)									ONEWORDINLINE(0xAA13);

EXTERN_API( void )
RGBForeColor					(const RGBColor *		color)								ONEWORDINLINE(0xAA14);

EXTERN_API( void )
RGBBackColor					(const RGBColor *		color)								ONEWORDINLINE(0xAA15);

EXTERN_API( void )
SetCPixel						(short 					h,
								 short 					v,
								 const RGBColor *		cPix)								ONEWORDINLINE(0xAA16);

EXTERN_API( void )
SetPortPix						(PixMapHandle 			pm)									ONEWORDINLINE(0xAA06);

EXTERN_API( void )
GetCPixel						(short 					h,
								 short 					v,
								 RGBColor *				cPix)								ONEWORDINLINE(0xAA17);

EXTERN_API( void )
GetForeColor					(RGBColor *				color)								ONEWORDINLINE(0xAA19);

EXTERN_API( void )
GetBackColor					(RGBColor *				color)								ONEWORDINLINE(0xAA1A);

EXTERN_API( void )
SeedCFill						(const BitMap *			srcBits,
								 const BitMap *			dstBits,
								 const Rect *			srcRect,
								 const Rect *			dstRect,
								 short 					seedH,
								 short 					seedV,
								 ColorSearchUPP 		matchProc,
								 long 					matchData)							ONEWORDINLINE(0xAA50);

EXTERN_API( void )
CalcCMask						(const BitMap *			srcBits,
								 const BitMap *			dstBits,
								 const Rect *			srcRect,
								 const Rect *			dstRect,
								 const RGBColor *		seedRGB,
								 ColorSearchUPP 		matchProc,
								 long 					matchData)							ONEWORDINLINE(0xAA4F);

EXTERN_API( PicHandle )
OpenCPicture					(const OpenCPicParams *	newHeader)							ONEWORDINLINE(0xAA20);

EXTERN_API( void )
OpColor							(const RGBColor *		color)								ONEWORDINLINE(0xAA21);

EXTERN_API( void )
HiliteColor						(const RGBColor *		color)								ONEWORDINLINE(0xAA22);

EXTERN_API( void )
DisposeCTable					(CTabHandle 			cTable)								ONEWORDINLINE(0xAA24);

EXTERN_API( CTabHandle )
GetCTable						(short 					ctID)								ONEWORDINLINE(0xAA18);

EXTERN_API( CCrsrHandle )
GetCCursor						(short 					crsrID)								ONEWORDINLINE(0xAA1B);

EXTERN_API( void )
SetCCursor						(CCrsrHandle 			cCrsr)								ONEWORDINLINE(0xAA1C);

EXTERN_API( void )
AllocCursor						(void)														ONEWORDINLINE(0xAA1D);

EXTERN_API( void )
DisposeCCursor					(CCrsrHandle 			cCrsr)								ONEWORDINLINE(0xAA26);

#if OLDROUTINELOCATIONS
EXTERN_API( CIconHandle )
GetCIcon						(short 					iconID)								ONEWORDINLINE(0xAA1E);

EXTERN_API( void )
PlotCIcon						(const Rect *			theRect,
								 CIconHandle 			theIcon)							ONEWORDINLINE(0xAA1F);

EXTERN_API( void )
DisposeCIcon					(CIconHandle 			theIcon)							ONEWORDINLINE(0xAA25);

#endif  /* OLDROUTINELOCATIONS */

EXTERN_API( void )
SetStdCProcs					(CQDProcs *				procs)								ONEWORDINLINE(0xAA4E);

EXTERN_API( GDHandle )
GetMaxDevice					(const Rect *			globalRect)							ONEWORDINLINE(0xAA27);

EXTERN_API( long )
GetCTSeed						(void)														ONEWORDINLINE(0xAA28);

EXTERN_API( GDHandle )
GetDeviceList					(void)														ONEWORDINLINE(0xAA29);

EXTERN_API( GDHandle )
GetMainDevice					(void)														ONEWORDINLINE(0xAA2A);

EXTERN_API( GDHandle )
GetNextDevice					(GDHandle 				curDevice)							ONEWORDINLINE(0xAA2B);

EXTERN_API( Boolean )
TestDeviceAttribute				(GDHandle 				gdh,
								 short 					attribute)							ONEWORDINLINE(0xAA2C);

EXTERN_API( void )
SetDeviceAttribute				(GDHandle 				gdh,
								 short 					attribute,
								 Boolean 				value)								ONEWORDINLINE(0xAA2D);

EXTERN_API( void )
InitGDevice						(short 					qdRefNum,
								 long 					mode,
								 GDHandle 				gdh)								ONEWORDINLINE(0xAA2E);

EXTERN_API( GDHandle )
NewGDevice						(short 					refNum,
								 long 					mode)								ONEWORDINLINE(0xAA2F);

EXTERN_API( void )
DisposeGDevice					(GDHandle 				gdh)								ONEWORDINLINE(0xAA30);

EXTERN_API( void )
SetGDevice						(GDHandle 				gd)									ONEWORDINLINE(0xAA31);

EXTERN_API( GDHandle )
GetGDevice						(void)														ONEWORDINLINE(0xAA32);

EXTERN_API( long )
Color2Index						(const RGBColor *		myColor)							ONEWORDINLINE(0xAA33);

EXTERN_API( void )
Index2Color						(long 					index,
								 RGBColor *				aColor)								ONEWORDINLINE(0xAA34);

EXTERN_API( void )
InvertColor						(RGBColor *				myColor)							ONEWORDINLINE(0xAA35);

EXTERN_API( Boolean )
RealColor						(const RGBColor *		color)								ONEWORDINLINE(0xAA36);

EXTERN_API( void )
GetSubTable						(CTabHandle 			myColors,
								 short 					iTabRes,
								 CTabHandle 			targetTbl)							ONEWORDINLINE(0xAA37);

EXTERN_API( void )
MakeITable						(CTabHandle 			cTabH,
								 ITabHandle 			iTabH,
								 short 					res)								ONEWORDINLINE(0xAA39);

EXTERN_API( void )
AddSearch						(ColorSearchUPP 		searchProc)							ONEWORDINLINE(0xAA3A);

EXTERN_API( void )
AddComp							(ColorComplementUPP 	compProc)							ONEWORDINLINE(0xAA3B);

EXTERN_API( void )
DelSearch						(ColorSearchUPP 		searchProc)							ONEWORDINLINE(0xAA4C);

EXTERN_API( void )
DelComp							(ColorComplementUPP 	compProc)							ONEWORDINLINE(0xAA4D);

EXTERN_API( void )
SetClientID						(short 					id)									ONEWORDINLINE(0xAA3C);

EXTERN_API( void )
ProtectEntry					(short 					index,
								 Boolean 				protect)							ONEWORDINLINE(0xAA3D);

EXTERN_API( void )
ReserveEntry					(short 					index,
								 Boolean 				reserve)							ONEWORDINLINE(0xAA3E);

EXTERN_API( void )
SetEntries						(short 					start,
								 short 					count,
								 CSpecArray 			aTable)								ONEWORDINLINE(0xAA3F);

EXTERN_API( void )
SaveEntries						(CTabHandle 			srcTable,
								 CTabHandle 			resultTable,
								 ReqListRec *			selection)							ONEWORDINLINE(0xAA49);

EXTERN_API( void )
RestoreEntries					(CTabHandle 			srcTable,
								 CTabHandle 			dstTable,
								 ReqListRec *			selection)							ONEWORDINLINE(0xAA4A);

EXTERN_API( short )
QDError							(void)														ONEWORDINLINE(0xAA40);

EXTERN_API( void )
CopyDeepMask					(const BitMap *			srcBits,
								 const BitMap *			maskBits,
								 const BitMap *			dstBits,
								 const Rect *			srcRect,
								 const Rect *			maskRect,
								 const Rect *			dstRect,
								 short 					mode,
								 RgnHandle 				maskRgn)							ONEWORDINLINE(0xAA51);

EXTERN_API( void )
DeviceLoop						(RgnHandle 				drawingRgn,
								 DeviceLoopDrawingUPP 	drawingProc,
								 long 					userData,
								 DeviceLoopFlags 		flags)								ONEWORDINLINE(0xABCA);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __A0 GetMaskTable
																							#endif
EXTERN_API( Ptr )
GetMaskTable					(void)														ONEWORDINLINE(0xA836);

EXTERN_API( PatHandle )
GetPattern						(short 					patternID)							ONEWORDINLINE(0xA9B8);

#if TARGET_OS_MAC
	#define MacGetCursor GetCursor
#endif
EXTERN_API( CursHandle )
MacGetCursor					(short 					cursorID)							ONEWORDINLINE(0xA9B9);

EXTERN_API( PicHandle )
GetPicture						(short 					pictureID)							ONEWORDINLINE(0xA9BC);

EXTERN_API( long )
DeltaPoint						(Point 					ptA,
								 Point 					ptB)								ONEWORDINLINE(0xA94F);

EXTERN_API( void )
ShieldCursor					(const Rect *			shieldRect,
								 Point 					offsetPt)							ONEWORDINLINE(0xA855);

EXTERN_API( void )
ScreenRes						(short *				scrnHRes,
								 short *				scrnVRes)							SIXWORDINLINE(0x225F, 0x32B8, 0x0102, 0x225F, 0x32B8, 0x0104);

EXTERN_API( void )
GetIndPattern					(Pattern *				thePat,
								 short 					patternListID,
								 short 					index);

#if CGLUESUPPORTED
EXTERN_API_C( Boolean )
ptinrect						(const Point *			pt,
								 const Rect *			r);

EXTERN_API_C( void )
pt2rect							(const Point *			pt1,
								 const Point *			pt2,
								 Rect *					destRect);

EXTERN_API_C( void )
pttoangle						(const Rect *			r,
								 const Point *			pt,
								 short *				angle);

EXTERN_API_C( Boolean )
ptinrgn							(const Point *			pt,
								 RgnHandle 				rgn);

EXTERN_API_C( void )
addpt							(const Point *			src,
								 Point *				dst);

EXTERN_API_C( void )
subpt							(const Point *			src,
								 Point *				dst);

EXTERN_API_C( Boolean )
equalpt							(const Point *			pt1,
								 const Point *			pt2);

EXTERN_API_C( void )
stuffhex						(void *					thingPtr,
								 const char *			s);

EXTERN_API_C( void )
stdline							(const Point *			newPt);

EXTERN_API_C( void )
shieldcursor					(const Rect *			shieldRect,
								 Point *				offsetPt);

EXTERN_API_C( long )
deltapoint						(Point *				ptA,
								 Point *				ptB);

#endif  /* CGLUESUPPORTED */

#if OLDROUTINENAMES
#define DisposPixMap(pm) DisposePixMap(pm)
#define DisposPixPat(pp) DisposePixPat(pp)
#define DisposCTable(cTable) DisposeCTable(cTable)
#define DisposCCursor(cCrsr) DisposeCCursor(cCrsr)
#if OLDROUTINELOCATIONS
#define DisposCIcon(theIcon) DisposeCIcon(theIcon)
#endif  /* OLDROUTINELOCATIONS */

#define DisposGDevice(gdh) DisposeGDevice(gdh)
#endif  /* OLDROUTINENAMES */

/*
	From ToolUtils.i
*/
EXTERN_API( void )
PackBits						(Ptr *					srcPtr,
								 Ptr *					dstPtr,
								 short 					srcBytes)							ONEWORDINLINE(0xA8CF);

EXTERN_API( void )
UnpackBits						(Ptr *					srcPtr,
								 Ptr *					dstPtr,
								 short 					dstBytes)							ONEWORDINLINE(0xA8D0);

EXTERN_API( Fixed )
SlopeFromAngle					(short 					angle)								ONEWORDINLINE(0xA8BC);

EXTERN_API( short )
AngleFromSlope					(Fixed 					slope)								ONEWORDINLINE(0xA8C4);













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

#endif /* __QUICKDRAW__ */

