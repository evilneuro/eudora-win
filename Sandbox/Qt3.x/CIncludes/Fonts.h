/*
 	File:		Fonts.h
 
 	Contains:	Font Manager Interfaces.
 
 	Version:	Technology:	MacOS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __FONTS__
#define __FONTS__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
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
	systemFont					= 0,
	applFont					= 1
};

/* kPlatformDefaultGuiFontID is used in QuickTime 3.0 */
#if TARGET_OS_MAC

enum {
	kPlatformDefaultGuiFontID	= applFont
};

#else

enum {
	kPlatformDefaultGuiFontID	= -1
};

#endif  /* TARGET_OS_MAC */

/*
	The following font constants are deprecated.  
	Please use GetFNum() to look up the font ID by name.
*/

enum {
	kFontIDNewYork				= 2,
	kFontIDGeneva				= 3,
	kFontIDMonaco				= 4,
	kFontIDVenice				= 5,
	kFontIDLondon				= 6,
	kFontIDAthens				= 7,
	kFontIDSanFrancisco			= 8,
	kFontIDToronto				= 9,
	kFontIDCairo				= 11,
	kFontIDLosAngeles			= 12,
	kFontIDTimes				= 20,
	kFontIDHelvetica			= 21,
	kFontIDCourier				= 22,
	kFontIDSymbol				= 23,
	kFontIDMobile				= 24
};

#if OLDROUTINENAMES

enum {
	newYork						= kFontIDNewYork,
	geneva						= kFontIDGeneva,
	monaco						= kFontIDMonaco,
	venice						= kFontIDVenice,
	london						= kFontIDLondon,
	athens						= kFontIDAthens,
	sanFran						= kFontIDSanFrancisco,
	toronto						= kFontIDToronto,
	cairo						= kFontIDCairo,
	losAngeles					= kFontIDLosAngeles,
	times						= kFontIDTimes,
	helvetica					= kFontIDHelvetica,
	courier						= kFontIDCourier,
	symbol						= kFontIDSymbol,
	mobile						= kFontIDMobile
};

#endif  /* OLDROUTINENAMES */


enum {
	commandMark					= 17,
	checkMark					= 18,
	diamondMark					= 19,
	appleMark					= 20
};


enum {
	propFont					= 36864L,
	prpFntH						= 36865L,
	prpFntW						= 36866L,
	prpFntHW					= 36867L,
	fixedFont					= 45056L,
	fxdFntH						= 45057L,
	fxdFntW						= 45058L,
	fxdFntHW					= 45059L,
	fontWid						= 44208L
};


struct FMInput {
	short 							family;
	short 							size;
	Style 							face;
	Boolean 						needBits;
	short 							device;
	Point 							numer;
	Point 							denom;
};
typedef struct FMInput					FMInput;

struct FMOutput {
	short 							errNum;
	Handle 							fontHandle;					/* the previous "privateFontResult" was a mistake! */
	UInt8 							boldPixels;
	UInt8 							italicPixels;
	UInt8 							ulOffset;
	UInt8 							ulShadow;
	UInt8 							ulThick;
	UInt8 							shadowPixels;
	SInt8 							extra;
	UInt8 							ascent;
	UInt8 							descent;
	UInt8 							widMax;
	SInt8 							leading;
	SInt8 							curStyle;
	Point 							numer;
	Point 							denom;
};
typedef struct FMOutput					FMOutput;

typedef FMOutput *						FMOutPtr;
typedef FMOutput *						FMOutputPtr;

struct FontRec {
	short 							fontType;					/*font type*/
	short 							firstChar;					/*ASCII code of first character*/
	short 							lastChar;					/*ASCII code of last character*/
	short 							widMax;						/*maximum character width*/
	short 							kernMax;					/*negative of maximum character kern*/
	short 							nDescent;					/*negative of descent*/
	short 							fRectWidth;					/*width of font rectangle*/
	short 							fRectHeight;				/*height of font rectangle*/
	unsigned short 					owTLoc;						/*offset to offset/width table*/
	short 							ascent;						/*ascent*/
	short 							descent;					/*descent*/
	short 							leading;					/*leading*/
	short 							rowWords;					/*row width of bit image / 2 */
};
typedef struct FontRec					FontRec;
typedef FontRec *						FontRecPtr;
typedef FontRecPtr *					FontRecHdl;

struct FMetricRec {
	Fixed 							ascent;						/*base line to top*/
	Fixed 							descent;					/*base line to bottom*/
	Fixed 							leading;					/*leading between lines*/
	Fixed 							widMax;						/*maximum character width*/
	Handle 							wTabHandle;					/*handle to font width table*/
};
typedef struct FMetricRec				FMetricRec;
/*
   typedef struct FMetricRec FMetricRec, *FMetricRecPtr;
   typedef FMetricRecPtr *FMetricRecHandle;
*/
typedef FMetricRec *					FMetricRecPtr;
typedef FMetricRecPtr *					FMetricRecHandle;

struct WidEntry {
	short 							widStyle;					/*style entry applies to*/
};
typedef struct WidEntry					WidEntry;

struct WidTable {
	short 							numWidths;					/*number of entries - 1*/
};
typedef struct WidTable					WidTable;

struct AsscEntry {
	short 							fontSize;
	short 							fontStyle;
	short 							fontID;						/*font resource ID*/
};
typedef struct AsscEntry				AsscEntry;

struct FontAssoc {
	short 							numAssoc;					/*number of entries - 1*/
};
typedef struct FontAssoc				FontAssoc;

struct StyleTable {
	short 							fontClass;
	long 							offset;
	long 							reserved;
	char 							indexes[48];
};
typedef struct StyleTable				StyleTable;

struct NameTable {
	short 							stringCount;
	Str255 							baseFontName;
};
typedef struct NameTable				NameTable;

struct KernPair {
	char 							kernFirst;					/*1st character of kerned pair*/
	char 							kernSecond;					/*2nd character of kerned pair*/
	short 							kernWidth;					/*kerning in 1pt fixed format*/
};
typedef struct KernPair					KernPair;

struct KernEntry {
	short 							kernStyle;					/*style the entry applies to*/
	short 							kernLength;					/*length of this entry*/
};
typedef struct KernEntry				KernEntry;

struct KernTable {
	short 							numKerns;					/*number of kerning entries*/
};
typedef struct KernTable				KernTable;

struct WidthTable {
	Fixed 							tabData[256];				/*character widths*/
	Handle 							tabFont;					/*font record used to build table - the previous FontResult was a mistake!*/
	long 							sExtra;						/*space extra used for table*/
	long 							style;						/*extra due to style*/
	short 							fID;						/*font family ID*/
	short 							fSize;						/*font size request*/
	short 							face;						/*style (face) request*/
	short 							device;						/*device requested*/
	Point 							inNumer;					/*scale factors requested*/
	Point 							inDenom;					/*scale factors requested*/
	short 							aFID;						/*actual font family ID for table*/
	Handle 							fHand;						/*family record used to build up table*/
	Boolean 						usedFam;					/*used fixed point family widths*/
	UInt8 							aFace;						/*actual face produced*/
	short 							vOutput;					/*vertical scale output value*/
	short 							hOutput;					/*horizontal scale output value*/
	short 							vFactor;					/*vertical scale output value*/
	short 							hFactor;					/*horizontal scale output value*/
	short 							aSize;						/*actual size of actual font used*/
	short 							tabSize;					/*total size of table*/
};
typedef struct WidthTable				WidthTable;
typedef WidthTable *					WidthTablePtr;
typedef WidthTablePtr *					WidthTableHdl;


struct FamRec {
	short 							ffFlags;					/*flags for family*/
	short 							ffFamID;					/*family ID number*/
	short 							ffFirstChar;				/*ASCII code of 1st character*/
	short 							ffLastChar;					/*ASCII code of last character*/
	short 							ffAscent;					/*maximum ascent for 1pt font*/
	short 							ffDescent;					/*maximum descent for 1pt font*/
	short 							ffLeading;					/*maximum leading for 1pt font*/
	short 							ffWidMax;					/*maximum widMax for 1pt font*/
	long 							ffWTabOff;					/*offset to width table*/
	long 							ffKernOff;					/*offset to kerning table*/
	long 							ffStylOff;					/*offset to style mapping table*/
	short 							ffProperty[9];				/*style property info*/
	short 							ffIntl[2];					/*for international use*/
	short 							ffVersion;					/*version number*/
};
typedef struct FamRec					FamRec;

typedef SInt16 							FontPointSize;
typedef SInt16 							FontFamilyID;
EXTERN_API( void )
InitFonts						(void)														ONEWORDINLINE(0xA8FE);

EXTERN_API( void )
GetFontName						(short 					familyID,
								 Str255 				name)								ONEWORDINLINE(0xA8FF);

EXTERN_API( void )
GetFNum							(ConstStr255Param 		name,
								 short *				familyID)							ONEWORDINLINE(0xA900);

EXTERN_API( Boolean )
RealFont						(short 					fontNum,
								 short 					size)								ONEWORDINLINE(0xA902);

EXTERN_API( void )
SetFontLock						(Boolean 				lockFlag)							ONEWORDINLINE(0xA903);

EXTERN_API( FMOutPtr )
FMSwapFont						(const FMInput *		inRec)								ONEWORDINLINE(0xA901);

EXTERN_API( void )
SetFScaleDisable				(Boolean 				fscaleDisable)						ONEWORDINLINE(0xA834);

EXTERN_API( void )
FontMetrics						(FMetricRecPtr 			theMetrics)							ONEWORDINLINE(0xA835);

EXTERN_API( void )
SetFractEnable					(Boolean 				fractEnable)						ONEWORDINLINE(0xA814);

EXTERN_API( short )
GetDefFontSize					(void)														FIVEWORDINLINE(0x3EB8, 0x0BA8, 0x6604, 0x3EBC, 0x000C);

EXTERN_API( Boolean )
IsOutline						(Point 					numer,
								 Point 					denom)								TWOWORDINLINE(0x7000, 0xA854);

EXTERN_API( void )
SetOutlinePreferred				(Boolean 				outlinePreferred)					TWOWORDINLINE(0x7001, 0xA854);

EXTERN_API( Boolean )
GetOutlinePreferred				(void)														TWOWORDINLINE(0x7009, 0xA854);

EXTERN_API( OSErr )
OutlineMetrics					(short 					byteCount,
								 const void *			textPtr,
								 Point 					numer,
								 Point 					denom,
								 short *				yMax,
								 short *				yMin,
								 FixedPtr 				awArray,
								 FixedPtr 				lsbArray,
								 RectPtr 				boundsArray)						TWOWORDINLINE(0x7008, 0xA854);

EXTERN_API( void )
SetPreserveGlyph				(Boolean 				preserveGlyph)						TWOWORDINLINE(0x700A, 0xA854);

EXTERN_API( Boolean )
GetPreserveGlyph				(void)														TWOWORDINLINE(0x700B, 0xA854);

EXTERN_API( OSErr )
FlushFonts						(void)														TWOWORDINLINE(0x700C, 0xA854);

#if CGLUESUPPORTED
EXTERN_API_C( void )
getfnum							(const char *			theName,
								 short *				familyID);

EXTERN_API_C( void )
getfontname						(short 					familyID,
								 char *					theName);

#endif  /* CGLUESUPPORTED */

EXTERN_API( short )
GetSysFont						(void)														TWOWORDINLINE(0x3EB8, 0x0BA6);

EXTERN_API( short )
GetAppFont						(void)														TWOWORDINLINE(0x3EB8, 0x0984);


EXTERN_API( void )
AntiTextSetApplicationAware		(Boolean 				aware)								TWOWORDINLINE(0x7024, 0xA854);

EXTERN_API( Boolean )
AntiTextGetApplicationAware		(void)														TWOWORDINLINE(0x7025, 0xA854);

EXTERN_API( void )
AntiTextSetEnabled				(Boolean 				enabled)							TWOWORDINLINE(0x7026, 0xA854);

EXTERN_API( Boolean )
AntiTextGetEnabled				(void)														TWOWORDINLINE(0x7027, 0xA854);

EXTERN_API( Boolean )
AntiTextIsAntiAliased			(Point 					numer,
								 Point 					denom)								TWOWORDINLINE(0x7028, 0xA854);



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

#endif /* __FONTS__ */

