/*
 	File:		CMApplication.h
 
 	Contains:	ColorSync API
 
 	Version:	Technology:	ColorSync 2.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1992-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __CMAPPLICATION__
#define __CMAPPLICATION__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __FILES__
#include <Files.h>
#endif
#ifndef __PRINTING__
#include <Printing.h>
#endif
#ifndef __CMICCPROFILE__
#include <CMICCProfile.h>
#endif
#ifndef __DISPLAYS__
#include <Displays.h>
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
	kDefaultCMMSignature		= FOUR_CHAR_CODE('appl')
};

/* Macintosh 68K trap word */

enum {
	cmTrap						= 0xABEE
};


/* PicComment IDs */

enum {
	cmBeginProfile				= 220,
	cmEndProfile				= 221,
	cmEnableMatching			= 222,
	cmDisableMatching			= 223,
	cmComment					= 224
};

/* PicComment selectors for cmComment */

enum {
	cmBeginProfileSel			= 0,
	cmContinueProfileSel		= 1,
	cmEndProfileSel				= 2,
	cmProfileIdentifierSel		= 3
};


/* Defines for version 1.0 CMProfileSearchRecord.fieldMask */

enum {
	cmMatchCMMType				= 0x00000001,
	cmMatchApplProfileVersion	= 0x00000002,
	cmMatchDataType				= 0x00000004,
	cmMatchDeviceType			= 0x00000008,
	cmMatchDeviceManufacturer	= 0x00000010,
	cmMatchDeviceModel			= 0x00000020,
	cmMatchDeviceAttributes		= 0x00000040,
	cmMatchFlags				= 0x00000080,
	cmMatchOptions				= 0x00000100,
	cmMatchWhite				= 0x00000200,
	cmMatchBlack				= 0x00000400
};

/* Defines for version 2.0 CMSearchRecord.searchMask */

enum {
	cmMatchAnyProfile			= 0x00000000,
	cmMatchProfileCMMType		= 0x00000001,
	cmMatchProfileClass			= 0x00000002,
	cmMatchDataColorSpace		= 0x00000004,
	cmMatchProfileConnectionSpace = 0x00000008,
	cmMatchManufacturer			= 0x00000010,
	cmMatchModel				= 0x00000020,
	cmMatchAttributes			= 0x00000040,
	cmMatchProfileFlags			= 0x00000080
};

/* Result codes */

enum {
																/* General Errors */
	cmProfileError				= -170,
	cmMethodError				= -171,
	cmMethodNotFound			= -175,							/* CMM not present */
	cmProfileNotFound			= -176,							/* Responder error */
	cmProfilesIdentical			= -177,							/* Profiles the same */
	cmCantConcatenateError		= -178,							/* Profile can't be concatenated */
	cmCantXYZ					= -179,							/* CMM cant handle XYZ space */
	cmCantDeleteProfile			= -180,							/* Responder error */
	cmUnsupportedDataType		= -181,							/* Responder error */
	cmNoCurrentProfile			= -182,							/* Responder error */
																/* Profile Access Errors */
	cmElementTagNotFound		= -4200,
	cmIndexRangeErr				= -4201,						/* Tag index out of range */
	cmCantDeleteElement			= -4202,
	cmFatalProfileErr			= -4203,
	cmInvalidProfile			= -4204,						/* A Profile must contain a 'cs1 ' tag to be valid */
	cmInvalidProfileLocation	= -4205,						/* Operation not supported for this profile location */
	cmCantCopyModifiedV1Profile	= -4215,						/* Illegal to copy version 1 profiles that have been modified */
																/* Profile Search Errors */
	cmInvalidSearch				= -4206,						/* Bad Search Handle */
	cmSearchError				= -4207,
	cmErrIncompatibleProfile	= -4208,						/* Other ColorSync Errors */
	cmInvalidColorSpace			= -4209,						/* Profile colorspace does not match bitmap type */
	cmInvalidSrcMap				= -4210,						/* Source pix/bit map was invalid */
	cmInvalidDstMap				= -4211,						/* Destination pix/bit map was invalid */
	cmNoGDevicesError			= -4212,						/* Begin/End Matching -- no gdevices available */
	cmInvalidProfileComment		= -4213,						/* Bad Profile comment during drawpicture */
	cmRangeOverFlow				= -4214,						/* Color conversion warning that some output color values over/underflowed and were clipped */
	cmNamedColorNotFound		= -4216,						/* NamedColor not found */
	cmCantGamutCheckError		= -4217							/* Gammut checking not supported by this ColorWorld */
};

/* deviceType values for ColorSync 1.0 Device Profile access */

enum {
	cmSystemDevice				= FOUR_CHAR_CODE('sys '),
	cmGDevice					= FOUR_CHAR_CODE('gdev')
};

/* Commands for CMFlattenUPP(É) */

enum {
	cmOpenReadSpool				= 1,
	cmOpenWriteSpool			= 2,
	cmReadSpool					= 3,
	cmWriteSpool				= 4,
	cmCloseSpool				= 5
};

/* Flags for PostScript-related functions */

enum {
	cmPS7bit					= 1,
	cmPS8bit					= 2
};

/* Flags for profile embedding functions */

enum {
	cmEmbedWholeProfile			= 0x00000000,
	cmEmbedProfileIdentifier	= 0x00000001
};

/* Commands for CMAccessUPP(É) */

enum {
	cmOpenReadAccess			= 1,
	cmOpenWriteAccess			= 2,
	cmReadAccess				= 3,
	cmWriteAccess				= 4,
	cmCloseAccess				= 5,
	cmCreateNewAccess			= 6,
	cmAbortWriteAccess			= 7,
	cmBeginAccess				= 8,
	cmEndAccess					= 9
};


/* Abstract data type for memory-based Profile */
typedef struct OpaqueCMProfileRef* 		CMProfileRef;
/* Abstract data type for Profile search result */
typedef struct OpaqueCMProfileSearchRef*  CMProfileSearchRef;
/* Abstract data type for BeginMatching(É) reference */
typedef struct OpaqueCMMatchRef* 		CMMatchRef;
/* Abstract data type for ColorWorld reference */
typedef struct OpaqueCMWorldRef* 		CMWorldRef;
/* Caller-supplied flatten function */
typedef CALLBACK_API( OSErr , CMFlattenProcPtr )(long command, long *size, void *data, void *refCon);
/* Caller-supplied progress function for Bitmap & PixMap matching routines */
typedef CALLBACK_API( Boolean , CMBitmapCallBackProcPtr )(long progress, void *refCon);
/* Caller-supplied filter function for Profile search */
typedef CALLBACK_API( Boolean , CMProfileFilterProcPtr )(CMProfileRef prof, void *refCon);
/* Caller-supplied function for profile access */
typedef CALLBACK_API( OSErr , CMProfileAccessProcPtr )(long command, long offset, long *size, void *data, void *refCon);
typedef STACK_UPP_TYPE(CMFlattenProcPtr) 						CMFlattenUPP;
typedef STACK_UPP_TYPE(CMBitmapCallBackProcPtr) 				CMBitmapCallBackUPP;
typedef STACK_UPP_TYPE(CMProfileFilterProcPtr) 					CMProfileFilterUPP;
typedef STACK_UPP_TYPE(CMProfileAccessProcPtr) 					CMProfileAccessUPP;
enum { uppCMFlattenProcInfo = 0x00003FE0 }; 					/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppCMBitmapCallBackProcInfo = 0x000003D0 }; 				/* pascal 1_byte Func(4_bytes, 4_bytes) */
enum { uppCMProfileFilterProcInfo = 0x000003D0 }; 				/* pascal 1_byte Func(4_bytes, 4_bytes) */
enum { uppCMProfileAccessProcInfo = 0x0000FFE0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewCMFlattenProc(userRoutine) 							(CMFlattenUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCMFlattenProcInfo, GetCurrentArchitecture())
#define NewCMBitmapCallBackProc(userRoutine) 					(CMBitmapCallBackUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCMBitmapCallBackProcInfo, GetCurrentArchitecture())
#define NewCMProfileFilterProc(userRoutine) 					(CMProfileFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCMProfileFilterProcInfo, GetCurrentArchitecture())
#define NewCMProfileAccessProc(userRoutine) 					(CMProfileAccessUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCMProfileAccessProcInfo, GetCurrentArchitecture())
#define CallCMFlattenProc(userRoutine, command, size, data, refCon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppCMFlattenProcInfo, (command), (size), (data), (refCon))
#define CallCMBitmapCallBackProc(userRoutine, progress, refCon)  CALL_TWO_PARAMETER_UPP((userRoutine), uppCMBitmapCallBackProcInfo, (progress), (refCon))
#define CallCMProfileFilterProc(userRoutine, prof, refCon) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppCMProfileFilterProcInfo, (prof), (refCon))
#define CallCMProfileAccessProc(userRoutine, command, offset, size, data, refCon)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppCMProfileAccessProcInfo, (command), (offset), (size), (data), (refCon))
typedef long 							CMError;
/* For 1.0 and 2.0 profile header variants */
/* CMAppleProfileHeader */

union CMAppleProfileHeader {
	CMHeader 						cm1;
	CM2Header 						cm2;
};
typedef union CMAppleProfileHeader		CMAppleProfileHeader;
/* Param for CWConcatColorWorld(É) */

struct CMConcatProfileSet {
	unsigned short 					keyIndex;					/* Zero-based */
	unsigned short 					count;						/* Min 1 */
	CMProfileRef 					profileSet[1];				/* Variable. Ordered from Source -> Dest */
};
typedef struct CMConcatProfileSet		CMConcatProfileSet;
/* ColorSync color data types */

struct CMRGBColor {
	unsigned short 					red;						/* 0..65535 */
	unsigned short 					green;
	unsigned short 					blue;
};
typedef struct CMRGBColor				CMRGBColor;

struct CMCMYKColor {
	unsigned short 					cyan;						/* 0..65535 */
	unsigned short 					magenta;
	unsigned short 					yellow;
	unsigned short 					black;
};
typedef struct CMCMYKColor				CMCMYKColor;

struct CMCMYColor {
	unsigned short 					cyan;						/* 0..65535 */
	unsigned short 					magenta;
	unsigned short 					yellow;
};
typedef struct CMCMYColor				CMCMYColor;

struct CMHLSColor {
	unsigned short 					hue;						/* 0..65535. Fraction of circle. Red at 0 */
	unsigned short 					lightness;					/* 0..65535 */
	unsigned short 					saturation;					/* 0..65535 */
};
typedef struct CMHLSColor				CMHLSColor;

struct CMHSVColor {
	unsigned short 					hue;						/* 0..65535. Fraction of circle. Red at 0 */
	unsigned short 					saturation;					/* 0..65535 */
	unsigned short 					value;						/* 0..65535 */
};
typedef struct CMHSVColor				CMHSVColor;

struct CMLabColor {
	unsigned short 					L;							/* 0..65535 maps to 0..100 */
	unsigned short 					a;							/* 0..65535 maps to -128..127.996 */
	unsigned short 					b;							/* 0..65535 maps to -128..127.996 */
};
typedef struct CMLabColor				CMLabColor;

struct CMLuvColor {
	unsigned short 					L;							/* 0..65535 maps to 0..100 */
	unsigned short 					u;							/* 0..65535 maps to -128..127.996 */
	unsigned short 					v;							/* 0..65535 maps to -128..127.996 */
};
typedef struct CMLuvColor				CMLuvColor;

struct CMYxyColor {
	unsigned short 					capY;						/* 0..65535 maps to 0..1 */
	unsigned short 					x;							/* 0..65535 maps to 0..1 */
	unsigned short 					y;							/* 0..65535 maps to 0..1 */
};
typedef struct CMYxyColor				CMYxyColor;

struct CMGrayColor {
	unsigned short 					gray;						/* 0..65535 */
};
typedef struct CMGrayColor				CMGrayColor;

struct CMMultichannel5Color {
	unsigned char 					components[5];				/* 0..255 */
};
typedef struct CMMultichannel5Color		CMMultichannel5Color;

struct CMMultichannel6Color {
	unsigned char 					components[6];				/* 0..255 */
};
typedef struct CMMultichannel6Color		CMMultichannel6Color;

struct CMMultichannel7Color {
	unsigned char 					components[7];				/* 0..255 */
};
typedef struct CMMultichannel7Color		CMMultichannel7Color;

struct CMMultichannel8Color {
	unsigned char 					components[8];				/* 0..255 */
};
typedef struct CMMultichannel8Color		CMMultichannel8Color;

struct CMNamedColor {
	unsigned long 					namedColorIndex;			/* 0..a lot */
};
typedef struct CMNamedColor				CMNamedColor;

union CMColor {
	CMRGBColor 						rgb;
	CMHSVColor 						hsv;
	CMHLSColor 						hls;
	CMXYZColor 						XYZ;
	CMLabColor 						Lab;
	CMLuvColor 						Luv;
	CMYxyColor 						Yxy;
	CMCMYKColor 					cmyk;
	CMCMYColor 						cmy;
	CMGrayColor 					gray;
	CMMultichannel5Color 			mc5;
	CMMultichannel6Color 			mc6;
	CMMultichannel7Color 			mc7;
	CMMultichannel8Color 			mc8;
	CMNamedColor 					namedColor;
};
typedef union CMColor					CMColor;

struct CMProfileSearchRecord {
	CMHeader 						header;
	unsigned long 					fieldMask;
	unsigned long 					reserved[2];
};
typedef struct CMProfileSearchRecord	CMProfileSearchRecord;
typedef CMProfileSearchRecord *			CMProfileSearchRecordPtr;
typedef CMProfileSearchRecordPtr *		CMProfileSearchRecordHandle;
/* Search definition for 2.0 */

struct CMSearchRecord {
	OSType 							CMMType;
	OSType 							profileClass;
	OSType 							dataColorSpace;
	OSType 							profileConnectionSpace;
	unsigned long 					deviceManufacturer;
	unsigned long 					deviceModel;
	unsigned long 					deviceAttributes[2];
	unsigned long 					profileFlags;
	unsigned long 					searchMask;
	CMProfileFilterUPP 				filter;
};
typedef struct CMSearchRecord			CMSearchRecord;
/* GetCWInfo structures */

struct CMMInfoRecord {
	OSType 							CMMType;
	long 							CMMVersion;
};
typedef struct CMMInfoRecord			CMMInfoRecord;

struct CMCWInfoRecord {
	unsigned long 					cmmCount;
	CMMInfoRecord 					cmmInfo[2];
};
typedef struct CMCWInfoRecord			CMCWInfoRecord;
/* profile identifier structures */

struct CMProfileIdentifier {
	CM2Header 						profileHeader;
	CMDateTime 						calibrationDate;
	unsigned long 					ASCIIProfileDescriptionLen;
	char 							ASCIIProfileDescription[1];	/* variable length */
};
typedef struct CMProfileIdentifier		CMProfileIdentifier;
typedef CMProfileIdentifier *			CMProfileIdentifierPtr;
/* packing formats */

enum {
	cmNoColorPacking			= 0x0000,
	cmAlphaSpace				= 0x0080,
	cmWord5ColorPacking			= 0x0500,
	cmLong8ColorPacking			= 0x0800,
	cmLong10ColorPacking		= 0x0A00,
	cmAlphaFirstPacking			= 0x1000,
	cmOneBitDirectPacking		= 0x0B00,
	cmAlphaLastPacking			= 0x0000,
	cm24_8ColorPacking			= 0x2100,
	cm32_8ColorPacking			= cmLong8ColorPacking,
	cm40_8ColorPacking			= 0x2200,
	cm48_8ColorPacking			= 0x2300,
	cm56_8ColorPacking			= 0x2400,
	cm64_8ColorPacking			= 0x2500,
	cm32_16ColorPacking			= 0x2600,
	cm48_16ColorPacking			= 0x2900,
	cm64_16ColorPacking			= 0x2A00,
	cm32_32ColorPacking			= 0x2700
};

/* general colorspaces */

enum {
	cmNoSpace					= 0,
	cmRGBSpace					= 1,
	cmCMYKSpace					= 2,
	cmHSVSpace					= 3,
	cmHLSSpace					= 4,
	cmYXYSpace					= 5,
	cmXYZSpace					= 6,
	cmLUVSpace					= 7,
	cmLABSpace					= 8,
	cmReservedSpace1			= 9,
	cmGraySpace					= 10,
	cmReservedSpace2			= 11,
	cmGamutResultSpace			= 12,
	cmNamedIndexedSpace			= 16,
	cmMCFiveSpace				= 17,
	cmMCSixSpace				= 18,
	cmMCSevenSpace				= 19,
	cmMCEightSpace				= 20,
	cmRGBASpace					= cmRGBSpace + cmAlphaSpace,
	cmGrayASpace				= cmGraySpace + cmAlphaSpace
};

/* supported CMBitmapColorSpaces - each of the following is a */
/* combination of a general colospace and a packing formats */

enum {
	cmGray16Space				= cmGraySpace,
	cmGrayA32Space				= cmGrayASpace,
	cmRGB16Space				= cmWord5ColorPacking + cmRGBSpace,
	cmRGB24Space				= cm24_8ColorPacking + cmRGBSpace,
	cmRGB32Space				= cm32_8ColorPacking + cmRGBSpace,
	cmRGB48Space				= cm48_16ColorPacking + cmRGBSpace,
	cmARGB32Space				= cm32_8ColorPacking + cmAlphaFirstPacking + cmRGBASpace,
	cmRGBA32Space				= cm32_8ColorPacking + cmAlphaLastPacking + cmRGBASpace,
	cmCMYK32Space				= cm32_8ColorPacking + cmCMYKSpace,
	cmCMYK64Space				= cm64_16ColorPacking + cmCMYKSpace,
	cmHSV32Space				= cmLong10ColorPacking + cmHSVSpace,
	cmHLS32Space				= cmLong10ColorPacking + cmHLSSpace,
	cmYXY32Space				= cmLong10ColorPacking + cmYXYSpace,
	cmXYZ32Space				= cmLong10ColorPacking + cmXYZSpace,
	cmLUV32Space				= cmLong10ColorPacking + cmLUVSpace,
	cmLAB24Space				= cm24_8ColorPacking + cmLABSpace,
	cmLAB32Space				= cmLong10ColorPacking + cmLABSpace,
	cmLAB48Space				= cm48_16ColorPacking + cmLABSpace,
	cmGamutResult1Space			= cmOneBitDirectPacking + cmGamutResultSpace,
	cmNamedIndexed32Space		= cm32_32ColorPacking + cmNamedIndexedSpace,
	cmMCFive8Space				= cm40_8ColorPacking + cmMCFiveSpace,
	cmMCSix8Space				= cm48_8ColorPacking + cmMCSixSpace,
	cmMCSeven8Space				= cm56_8ColorPacking + cmMCSevenSpace,
	cmMCEight8Space				= cm64_8ColorPacking + cmMCEightSpace
};



typedef unsigned long 					CMBitmapColorSpace;

struct CMBitmap {
	char *							image;
	long 							width;
	long 							height;
	long 							rowBytes;
	long 							pixelSize;
	CMBitmapColorSpace 				space;
	long 							user1;
	long 							user2;
};
typedef struct CMBitmap					CMBitmap;

/* Classic Print Manager Stuff */

enum {
	enableColorMatchingOp		= 12,
	registerProfileOp			= 13
};


enum {
	cmNoProfileBase				= 0,
	cmFileBasedProfile			= 1,
	cmHandleBasedProfile		= 2,
	cmPtrBasedProfile			= 3,
	cmProcedureBasedProfile		= 4
};


struct CMFileLocation {
	FSSpec 							spec;
};
typedef struct CMFileLocation			CMFileLocation;

struct CMHandleLocation {
	Handle 							h;
};
typedef struct CMHandleLocation			CMHandleLocation;

struct CMPtrLocation {
	Ptr 							p;
};
typedef struct CMPtrLocation			CMPtrLocation;

struct CMProcedureLocation {
	CMProfileAccessUPP 				proc;
	void *							refCon;
};
typedef struct CMProcedureLocation		CMProcedureLocation;


union CMProfLoc {
	CMFileLocation 					fileLoc;
	CMHandleLocation 				handleLoc;
	CMPtrLocation 					ptrLoc;
	CMProcedureLocation 			procLoc;
};
typedef union CMProfLoc					CMProfLoc;

struct CMProfileLocation {
	short 							locType;
	CMProfLoc 						u;
};
typedef struct CMProfileLocation		CMProfileLocation;

enum {
	cmOriginalProfileLocationSize = 72,
	cmCurrentProfileLocationSize = 72
};


/* Struct and enums used for Profile iteration */

enum {
	cmProfileIterateDataVersion1 = 0x00010000
};


struct CMProfileIterateData {
	unsigned long 					dataVersion;				/* cmProfileIterateDataVersion1 */
	CM2Header 						header;
	ScriptCode 						code;
	Str255 							name;
	CMProfileLocation 				location;
};
typedef struct CMProfileIterateData		CMProfileIterateData;
/* Caller-supplied callback function for Profile iteration */
typedef CALLBACK_API( OSErr , CMProfileIterateProcPtr )(CMProfileIterateData *iterateData, void *refCon);
typedef STACK_UPP_TYPE(CMProfileIterateProcPtr) 				CMProfileIterateUPP;
enum { uppCMProfileIterateProcInfo = 0x000003E0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes) */
#define NewCMProfileIterateProc(userRoutine) 					(CMProfileIterateUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCMProfileIterateProcInfo, GetCurrentArchitecture())
#define CallCMProfileIterateProc(userRoutine, iterateData, refCon)  CALL_TWO_PARAMETER_UPP((userRoutine), uppCMProfileIterateProcInfo, (iterateData), (refCon))
/* Profile file and element access */
EXTERN_API( CMError )
CMNewProfile					(CMProfileRef *			prof,
								 const CMProfileLocation * theProfile)						FOURWORDINLINE(0x203C, 0x0008, 0x001B, 0xABEE);

EXTERN_API( CMError )
CMOpenProfile					(CMProfileRef *			prof,
								 const CMProfileLocation * theProfile)						FOURWORDINLINE(0x203C, 0x0008, 0x001C, 0xABEE);

EXTERN_API( CMError )
CMCloseProfile					(CMProfileRef 			prof)								FOURWORDINLINE(0x203C, 0x0004, 0x001D, 0xABEE);

EXTERN_API( CMError )
CMUpdateProfile					(CMProfileRef 			prof)								FOURWORDINLINE(0x203C, 0x0004, 0x0034, 0xABEE);

EXTERN_API( CMError )
CMCopyProfile					(CMProfileRef *			targetProf,
								 const CMProfileLocation * targetLocation,
								 CMProfileRef 			srcProf)							FOURWORDINLINE(0x203C, 0x000C, 0x0025, 0xABEE);

EXTERN_API( CMError )
CMValidateProfile				(CMProfileRef 			prof,
								 Boolean *				valid,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x000C, 0x0026, 0xABEE);

EXTERN_API( CMError )
CMGetProfileLocation			(CMProfileRef 			prof,
								 CMProfileLocation *	theProfile)							FOURWORDINLINE(0x203C, 0x0008, 0x003C, 0xABEE);

EXTERN_API( CMError )
NCMGetProfileLocation			(CMProfileRef 			prof,
								 CMProfileLocation *	theProfile,
								 unsigned long *		locationSize)						FOURWORDINLINE(0x203C, 0x000C, 0x0059, 0xABEE);

EXTERN_API( CMError )
CMFlattenProfile				(CMProfileRef 			prof,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0014, 0x0031, 0xABEE);

EXTERN_API( CMError )
CMUnflattenProfile				(FSSpec *				resultFileSpec,
								 CMFlattenUPP 			proc,
								 void *					refCon,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0010, 0x0032, 0xABEE);

EXTERN_API( CMError )
CMGetProfileHeader				(CMProfileRef 			prof,
								 CMAppleProfileHeader *	header)								FOURWORDINLINE(0x203C, 0x0008, 0x0039, 0xABEE);

EXTERN_API( CMError )
CMSetProfileHeader				(CMProfileRef 			prof,
								 const CMAppleProfileHeader * header)						FOURWORDINLINE(0x203C, 0x0008, 0x003A, 0xABEE);

EXTERN_API( CMError )
CMProfileElementExists			(CMProfileRef 			prof,
								 OSType 				tag,
								 Boolean *				found)								FOURWORDINLINE(0x203C, 0x000C, 0x001E, 0xABEE);

EXTERN_API( CMError )
CMCountProfileElements			(CMProfileRef 			prof,
								 unsigned long *		elementCount)						FOURWORDINLINE(0x203C, 0x0008, 0x001F, 0xABEE);

EXTERN_API( CMError )
CMGetProfileElement				(CMProfileRef 			prof,
								 OSType 				tag,
								 unsigned long *		elementSize,
								 void *					elementData)						FOURWORDINLINE(0x203C, 0x0010, 0x0020, 0xABEE);

EXTERN_API( CMError )
CMSetProfileElement				(CMProfileRef 			prof,
								 OSType 				tag,
								 unsigned long 			elementSize,
								 void *					elementData)						FOURWORDINLINE(0x203C, 0x0010, 0x0023, 0xABEE);

EXTERN_API( CMError )
CMSetProfileElementSize			(CMProfileRef 			prof,
								 OSType 				tag,
								 unsigned long 			elementSize)						FOURWORDINLINE(0x203C, 0x000C, 0x0038, 0xABEE);

EXTERN_API( CMError )
CMSetProfileElementReference	(CMProfileRef 			prof,
								 OSType 				elementTag,
								 OSType 				referenceTag)						FOURWORDINLINE(0x203C, 0x000C, 0x0035, 0xABEE);

EXTERN_API( CMError )
CMGetPartialProfileElement		(CMProfileRef 			prof,
								 OSType 				tag,
								 unsigned long 			offset,
								 unsigned long *		byteCount,
								 void *					elementData)						FOURWORDINLINE(0x203C, 0x0014, 0x0036, 0xABEE);

EXTERN_API( CMError )
CMSetPartialProfileElement		(CMProfileRef 			prof,
								 OSType 				tag,
								 unsigned long 			offset,
								 unsigned long 			byteCount,
								 void *					elementData)						FOURWORDINLINE(0x203C, 0x0014, 0x0037, 0xABEE);

EXTERN_API( CMError )
CMGetIndProfileElementInfo		(CMProfileRef 			prof,
								 unsigned long 			index,
								 OSType *				tag,
								 unsigned long *		elementSize,
								 Boolean *				refs)								FOURWORDINLINE(0x203C, 0x0014, 0x0021, 0xABEE);

EXTERN_API( CMError )
CMGetIndProfileElement			(CMProfileRef 			prof,
								 unsigned long 			index,
								 unsigned long *		elementSize,
								 void *					elementData)						FOURWORDINLINE(0x203C, 0x0010, 0x0022, 0xABEE);

EXTERN_API( CMError )
CMRemoveProfileElement			(CMProfileRef 			prof,
								 OSType 				tag)								FOURWORDINLINE(0x203C, 0x0008, 0x0024, 0xABEE);

EXTERN_API( CMError )
CMGetScriptProfileDescription	(CMProfileRef 			prof,
								 Str255 				name,
								 ScriptCode *			code)								FOURWORDINLINE(0x203C, 0x000C, 0x003E, 0xABEE);

EXTERN_API( CMError )
CMCloneProfileRef				(CMProfileRef 			prof)								FOURWORDINLINE(0x203C, 0x0004, 0x0042, 0xABEE);

EXTERN_API( CMError )
CMGetProfileRefCount			(CMProfileRef 			prof,
								 long *					count)								FOURWORDINLINE(0x203C, 0x0008, 0x0043, 0xABEE);

EXTERN_API( CMError )
CMProfileModified				(CMProfileRef 			prof,
								 Boolean *				modified)							FOURWORDINLINE(0x203C, 0x0008, 0x0044, 0xABEE);


/* named Color access functions */
EXTERN_API( CMError )
CMGetNamedColorInfo				(CMProfileRef 			prof,
								 unsigned long *		deviceChannels,
								 OSType *				deviceColorSpace,
								 OSType *				PCSColorSpace,
								 unsigned long *		count,
								 StringPtr 				prefix,
								 StringPtr 				suffix)								FOURWORDINLINE(0x203C, 0x001C, 0x0046, 0xABEE);

EXTERN_API( CMError )
CMGetNamedColorValue			(CMProfileRef 			prof,
								 StringPtr 				name,
								 CMColor *				deviceColor,
								 CMColor *				PCSColor)							FOURWORDINLINE(0x203C, 0x0010, 0x0047, 0xABEE);

EXTERN_API( CMError )
CMGetIndNamedColorValue			(CMProfileRef 			prof,
								 unsigned long 			index,
								 CMColor *				deviceColor,
								 CMColor *				PCSColor)							FOURWORDINLINE(0x203C, 0x0010, 0x0048, 0xABEE);

EXTERN_API( CMError )
CMGetNamedColorIndex			(CMProfileRef 			prof,
								 StringPtr 				name,
								 unsigned long *		index)								FOURWORDINLINE(0x203C, 0x000C, 0x0049, 0xABEE);

EXTERN_API( CMError )
CMGetNamedColorName				(CMProfileRef 			prof,
								 unsigned long 			index,
								 StringPtr 				name)								FOURWORDINLINE(0x203C, 0x000C, 0x004A, 0xABEE);


/* Low-level matching functions */
EXTERN_API( CMError )
NCWNewColorWorld				(CMWorldRef *			cw,
								 CMProfileRef 			src,
								 CMProfileRef 			dst)								FOURWORDINLINE(0x203C, 0x000C, 0x0014, 0xABEE);

EXTERN_API( CMError )
CWConcatColorWorld				(CMWorldRef *			cw,
								 CMConcatProfileSet *	profileSet)							FOURWORDINLINE(0x203C, 0x0008, 0x0015, 0xABEE);

EXTERN_API( CMError )
CWNewLinkProfile				(CMProfileRef *			prof,
								 const CMProfileLocation * targetLocation,
								 CMConcatProfileSet *	profileSet)							FOURWORDINLINE(0x203C, 0x000C, 0x0033, 0xABEE);

EXTERN_API( void )
CWDisposeColorWorld				(CMWorldRef 			cw)									FOURWORDINLINE(0x203C, 0x0004, 0x0001, 0xABEE);

EXTERN_API( CMError )
CWMatchColors					(CMWorldRef 			cw,
								 CMColor *				myColors,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0002, 0xABEE);

EXTERN_API( CMError )
CWCheckColors					(CMWorldRef 			cw,
								 CMColor *				myColors,
								 unsigned long 			count,
								 long *					result)								FOURWORDINLINE(0x203C, 0x0010, 0x0003, 0xABEE);


/* Bitmap matching */
EXTERN_API( CMError )
CWMatchBitmap					(CMWorldRef 			cw,
								 CMBitmap *				bitmap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon,
								 CMBitmap *				matchedBitmap)						FOURWORDINLINE(0x203C, 0x0010, 0x002C, 0xABEE);

EXTERN_API( CMError )
CWCheckBitmap					(CMWorldRef 			cw,
								 const CMBitmap *		bitmap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon,
								 CMBitmap *				resultBitmap)						FOURWORDINLINE(0x203C, 0x0014, 0x002D, 0xABEE);


/* Quickdraw-specific matching */
EXTERN_API( CMError )
CWMatchPixMap					(CMWorldRef 			cw,
								 PixMap *				myPixMap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon)								FOURWORDINLINE(0x203C, 0x0010, 0x0004, 0xABEE);

EXTERN_API( CMError )
CWCheckPixMap					(CMWorldRef 			cw,
								 PixMap *				myPixMap,
								 CMBitmapCallBackUPP 	progressProc,
								 void *					refCon,
								 BitMap *				resultBitMap)						FOURWORDINLINE(0x203C, 0x0014, 0x0007, 0xABEE);

EXTERN_API( CMError )
NCMBeginMatching				(CMProfileRef 			src,
								 CMProfileRef 			dst,
								 CMMatchRef *			myRef)								FOURWORDINLINE(0x203C, 0x000C, 0x0016, 0xABEE);

EXTERN_API( void )
CMEndMatching					(CMMatchRef 			myRef)								FOURWORDINLINE(0x203C, 0x0004, 0x000B, 0xABEE);

EXTERN_API( void )
NCMDrawMatchedPicture			(PicHandle 				myPicture,
								 CMProfileRef 			dst,
								 Rect *					myRect)								FOURWORDINLINE(0x203C, 0x000C, 0x0017, 0xABEE);

EXTERN_API( void )
CMEnableMatchingComment			(Boolean 				enableIt)							FOURWORDINLINE(0x203C, 0x0002, 0x000D, 0xABEE);

EXTERN_API( CMError )
NCMUseProfileComment			(CMProfileRef 			prof,
								 unsigned long 			flags)								FOURWORDINLINE(0x203C, 0x0008, 0x003B, 0xABEE);

EXTERN_API( CMError )
CMCreateProfileIdentifier		(CMProfileRef 			prof,
								 CMProfileIdentifierPtr  ident,
								 unsigned long *		size)								FOURWORDINLINE(0x203C, 0x000C, 0x0041, 0xABEE);


/* System Profile access */
EXTERN_API( CMError )
CMGetSystemProfile				(CMProfileRef *			prof)								FOURWORDINLINE(0x203C, 0x0004, 0x0018, 0xABEE);

EXTERN_API( CMError )
CMSetSystemProfile				(const FSSpec *			profileFileSpec)					FOURWORDINLINE(0x203C, 0x0004, 0x0019, 0xABEE);

EXTERN_API( CMError )
CMGetDefaultProfileBySpace		(OSType 				dataColorSpace,
								 CMProfileRef *			prof)								FOURWORDINLINE(0x203C, 0x0008, 0x005A, 0xABEE);

EXTERN_API( CMError )
CMSetDefaultProfileBySpace		(OSType 				dataColorSpace,
								 CMProfileRef 			prof)								FOURWORDINLINE(0x203C, 0x0008, 0x005B, 0xABEE);

EXTERN_API( CMError )
CMGetProfileByAVID				(AVIDType 				theAVID,
								 CMProfileRef *			prof)								FOURWORDINLINE(0x203C, 0x0008, 0x005C, 0xABEE);

EXTERN_API( CMError )
CMSetProfileByAVID				(AVIDType 				theAVID,
								 CMProfileRef 			prof)								FOURWORDINLINE(0x203C, 0x0008, 0x005D, 0xABEE);


/* External Profile Management */
EXTERN_API( CMError )
CMNewProfileSearch				(CMSearchRecord *		searchSpec,
								 void *					refCon,
								 unsigned long *		count,
								 CMProfileSearchRef *	searchResult)						FOURWORDINLINE(0x203C, 0x0010, 0x0027, 0xABEE);

EXTERN_API( CMError )
CMUpdateProfileSearch			(CMProfileSearchRef 	search,
								 void *					refCon,
								 unsigned long *		count)								FOURWORDINLINE(0x203C, 0x000C, 0x0028, 0xABEE);

EXTERN_API( void )
CMDisposeProfileSearch			(CMProfileSearchRef 	search)								FOURWORDINLINE(0x203C, 0x0004, 0x0029, 0xABEE);

EXTERN_API( CMError )
CMSearchGetIndProfile			(CMProfileSearchRef 	search,
								 unsigned long 			index,
								 CMProfileRef *			prof)								FOURWORDINLINE(0x203C, 0x000C, 0x002A, 0xABEE);

EXTERN_API( CMError )
CMSearchGetIndProfileFileSpec	(CMProfileSearchRef 	search,
								 unsigned long 			index,
								 FSSpec *				profileFile)						FOURWORDINLINE(0x203C, 0x000C, 0x002B, 0xABEE);

EXTERN_API( CMError )
CMProfileIdentifierFolderSearch	(CMProfileIdentifierPtr  ident,
								 unsigned long *		matchedCount,
								 CMProfileSearchRef *	searchResult)						FOURWORDINLINE(0x203C, 0x000C, 0x003F, 0xABEE);

EXTERN_API( CMError )
CMProfileIdentifierListSearch	(CMProfileIdentifierPtr  ident,
								 CMProfileRef *			profileList,
								 unsigned long 			listSize,
								 unsigned long *		matchedCount,
								 CMProfileRef *			matchedList)						FOURWORDINLINE(0x203C, 0x0014, 0x0040, 0xABEE);

EXTERN_API( CMError )
CMIterateColorSyncFolder		(CMProfileIterateUPP 	proc,
								 unsigned long *		seed,
								 unsigned long *		count,
								 void *					refCon)								FOURWORDINLINE(0x203C, 0x0010, 0x0058, 0xABEE);


/* Utilities */
EXTERN_API( CMError )
CMGetColorSyncFolderSpec		(short 					vRefNum,
								 Boolean 				createFolder,
								 short *				foundVRefNum,
								 long *					foundDirID)							FOURWORDINLINE(0x203C, 0x000C, 0x0011, 0xABEE);

EXTERN_API( CMError )
CMGetCWInfo						(CMWorldRef 			cw,
								 CMCWInfoRecord *		info)								FOURWORDINLINE(0x203C, 0x0008, 0x001A, 0xABEE);

EXTERN_API( CMError )
CMConvertProfile2to1			(CMProfileRef 			profv2,
								 CMProfileHandle *		profv1)								FOURWORDINLINE(0x203C, 0x0008, 0x0045, 0xABEE);

EXTERN_API( CMError )
CMGetPreferredCMM				(OSType *				cmmType,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0008, 0x005E, 0xABEE);



/* ColorSpace conversion functions */
EXTERN_API( CMError )
CMConvertXYZToLab				(const CMColor *		src,
								 const CMXYZColor *		white,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x0010, 0x004B, 0xABEE);

EXTERN_API( CMError )
CMConvertLabToXYZ				(const CMColor *		src,
								 const CMXYZColor *		white,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x0010, 0x004C, 0xABEE);

EXTERN_API( CMError )
CMConvertXYZToLuv				(const CMColor *		src,
								 const CMXYZColor *		white,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x0010, 0x004D, 0xABEE);

EXTERN_API( CMError )
CMConvertLuvToXYZ				(const CMColor *		src,
								 const CMXYZColor *		white,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x0010, 0x004E, 0xABEE);

EXTERN_API( CMError )
CMConvertXYZToYxy				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x004F, 0xABEE);

EXTERN_API( CMError )
CMConvertYxyToXYZ				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0050, 0xABEE);

EXTERN_API( CMError )
CMConvertRGBToHLS				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0051, 0xABEE);

EXTERN_API( CMError )
CMConvertHLSToRGB				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0052, 0xABEE);

EXTERN_API( CMError )
CMConvertRGBToHSV				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0053, 0xABEE);

EXTERN_API( CMError )
CMConvertHSVToRGB				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0054, 0xABEE);

EXTERN_API( CMError )
CMConvertRGBToGray				(const CMColor *		src,
								 CMColor *				dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0055, 0xABEE);

EXTERN_API( CMError )
CMConvertXYZToFixedXYZ			(const CMXYZColor *		src,
								 CMFixedXYZColor *		dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0056, 0xABEE);

EXTERN_API( CMError )
CMConvertFixedXYZToXYZ			(const CMFixedXYZColor * src,
								 CMXYZColor *			dst,
								 unsigned long 			count)								FOURWORDINLINE(0x203C, 0x000C, 0x0057, 0xABEE);

/* PS-related */
EXTERN_API( CMError )
CMGetPS2ColorSpace				(CMProfileRef 			srcProf,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0014, 0x002E, 0xABEE);

EXTERN_API( CMError )
CMGetPS2ColorRenderingIntent	(CMProfileRef 			srcProf,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0014, 0x002F, 0xABEE);

EXTERN_API( CMError )
CMGetPS2ColorRendering			(CMProfileRef 			srcProf,
								 CMProfileRef 			dstProf,
								 unsigned long 			flags,
								 CMFlattenUPP 			proc,
								 void *					refCon,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0018, 0x0030, 0xABEE);

EXTERN_API( CMError )
CMGetPS2ColorRenderingVMSize	(CMProfileRef 			srcProf,
								 CMProfileRef 			dstProf,
								 unsigned long *		vmSize,
								 Boolean *				preferredCMMnotfound)				FOURWORDINLINE(0x203C, 0x0010, 0x003D, 0xABEE);


/* ColorSync 1.0 functions which have parallel 2.0 counterparts */
EXTERN_API( CMError )
CWNewColorWorld					(CMWorldRef *			cw,
								 CMProfileHandle 		src,
								 CMProfileHandle 		dst)								FOURWORDINLINE(0x203C, 0x000C, 0x0000, 0xABEE);

EXTERN_API( CMError )
ConcatenateProfiles				(CMProfileHandle 		thru,
								 CMProfileHandle 		dst,
								 CMProfileHandle *		newDst)								FOURWORDINLINE(0x203C, 0x000C, 0x000C, 0xABEE);

EXTERN_API( CMError )
CMBeginMatching					(CMProfileHandle 		src,
								 CMProfileHandle 		dst,
								 CMMatchRef *			myRef)								FOURWORDINLINE(0x203C, 0x000C, 0x000A, 0xABEE);

EXTERN_API( void )
CMDrawMatchedPicture			(PicHandle 				myPicture,
								 CMProfileHandle 		dst,
								 Rect *					myRect)								FOURWORDINLINE(0x203C, 0x000C, 0x0009, 0xABEE);

EXTERN_API( CMError )
CMUseProfileComment				(CMProfileHandle 		profile)							FOURWORDINLINE(0x203C, 0x0004, 0x0008, 0xABEE);

EXTERN_API( void )
CMGetProfileName				(CMProfileHandle 		myProfile,
								 CMIString *			IStringResult)						FOURWORDINLINE(0x203C, 0x0008, 0x000E, 0xABEE);

EXTERN_API( long )
CMGetProfileAdditionalDataOffset (CMProfileHandle 		myProfile)							FOURWORDINLINE(0x203C, 0x0004, 0x000F, 0xABEE);


/* ProfileResponder functions */
EXTERN_API( CMError )
GetProfile						(OSType 				deviceType,
								 long 					refNum,
								 CMProfileHandle 		aProfile,
								 CMProfileHandle *		returnedProfile)					FOURWORDINLINE(0x203C, 0x0010, 0x0005, 0xABEE);

EXTERN_API( CMError )
SetProfile						(OSType 				deviceType,
								 long 					refNum,
								 CMProfileHandle 		newProfile)							FOURWORDINLINE(0x203C, 0x000C, 0x0006, 0xABEE);

EXTERN_API( CMError )
SetProfileDescription			(OSType 				deviceType,
								 long 					refNum,
								 long 					deviceData,
								 CMProfileHandle 		hProfile)							FOURWORDINLINE(0x203C, 0x0010, 0x0010, 0xABEE);

EXTERN_API( CMError )
GetIndexedProfile				(OSType 				deviceType,
								 long 					refNum,
								 CMProfileSearchRecordHandle  search,
								 CMProfileHandle *		returnProfile,
								 long *					index)								FOURWORDINLINE(0x203C, 0x0014, 0x0012, 0xABEE);

EXTERN_API( CMError )
DeleteDeviceProfile				(OSType 				deviceType,
								 long 					refNum,
								 CMProfileHandle 		deleteMe)							FOURWORDINLINE(0x203C, 0x000C, 0x0013, 0xABEE);


#if OLDROUTINENAMES
/* constants */

typedef CMFlattenProcPtr 				CMFlattenProc;
typedef CMBitmapCallBackProcPtr 		CMBitmapCallBackProc;
typedef CMProfileFilterProcPtr 			CMProfileFilterProc;

enum {
	CMTrap						= cmTrap,
	CMBeginProfile				= cmBeginProfile,
	CMEndProfile				= cmEndProfile,
	CMEnableMatching			= cmEnableMatching,
	CMDisableMatching			= cmDisableMatching
};

/* 1.0 Error codes, for compatibility with older applications. 1.0 CMM's may return obsolete error codes */

enum {
	CMNoError					= 0,							/*	obsolete name, use noErr */
	CMProfileError				= cmProfileError,
	CMMethodError				= cmMethodError,
	CMMemFullError				= -172,							/*	obsolete, 2.0 uses memFullErr */
	CMUnimplementedError		= -173,							/*	obsolete, 2.0 uses unimpErr */
	CMParamError				= -174,							/*	obsolete, 2.0 uses paramErr */
	CMMethodNotFound			= cmMethodNotFound,
	CMProfileNotFound			= cmProfileNotFound,
	CMProfilesIdentical			= cmProfilesIdentical,
	CMCantConcatenateError		= cmCantConcatenateError,
	CMCantXYZ					= cmCantXYZ,
	CMCantDeleteProfile			= cmCantDeleteProfile,
	CMUnsupportedDataType		= cmUnsupportedDataType,
	CMNoCurrentProfile			= cmNoCurrentProfile
};


enum {
	qdSystemDevice				= cmSystemDevice,
	qdGDevice					= cmGDevice
};


enum {
	kMatchCMMType				= cmMatchCMMType,
	kMatchApplProfileVersion	= cmMatchApplProfileVersion,
	kMatchDataType				= cmMatchDataType,
	kMatchDeviceType			= cmMatchDeviceType,
	kMatchDeviceManufacturer	= cmMatchDeviceManufacturer,
	kMatchDeviceModel			= cmMatchDeviceModel,
	kMatchDeviceAttributes		= cmMatchDeviceAttributes,
	kMatchFlags					= cmMatchFlags,
	kMatchOptions				= cmMatchOptions,
	kMatchWhite					= cmMatchWhite,
	kMatchBlack					= cmMatchBlack
};

/* types */
typedef CMCMYKColor 					CMYKColor;
typedef CMWorldRef 						CWorld;
typedef long *							CMGamutResult;
/* functions */
#define EndMatching(myRef)														CMEndMatching(myRef)
#define EnableMatching(enableIt)												CMEnableMatchingComment(enableIt)
#define GetColorSyncFolderSpec(vRefNum, createFolder, foundVRefNum, foundDirID)	CMGetColorSyncFolderSpec(vRefNum, createFolder, foundVRefNum, foundDirID)
#define BeginMatching(src, dst, myRef)											CMBeginMatching(src, dst, myRef)
#define DrawMatchedPicture(myPicture, dst, myRect)								CMDrawMatchedPicture(myPicture, dst, myRect)
#define UseProfile(profile)														CMUseProfileComment(profile)
#define GetProfileName(myProfile, IStringResult)								CMGetProfileName(myProfile, IStringResult)
#define GetProfileAdditionalDataOffset(myProfile)								CMGetProfileAdditionalDataOffset(myProfile)
#endif  /* OLDROUTINENAMES */


/* Deprecated stuff*/

/* PrGeneral parameter blocks */

struct TEnableColorMatchingBlk {
	short 							iOpCode;
	short 							iError;
	long 							lReserved;
	THPrint 						hPrint;
	Boolean 						fEnableIt;
	SInt8 							filler;
};
typedef struct TEnableColorMatchingBlk	TEnableColorMatchingBlk;

struct TRegisterProfileBlk {
	short 							iOpCode;
	short 							iError;
	long 							lReserved;
	THPrint 						hPrint;
	Boolean 						fRegisterIt;
	SInt8 							filler;
};
typedef struct TRegisterProfileBlk		TRegisterProfileBlk;

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

#endif /* __CMAPPLICATION__ */

