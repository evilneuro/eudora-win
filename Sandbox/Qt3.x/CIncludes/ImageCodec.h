/*
 	File:		ImageCodec.h
 
 	Contains:	QuickTime Interfaces.
 
 	Version:	Technology:	QuickTime 3.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __IMAGECODEC__
#define __IMAGECODEC__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __IMAGECOMPRESSION__
#include <ImageCompression.h>
#endif
#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __MOVIES__
#include <Movies.h>
#endif
#ifndef __GXTYPES__
#include <GXTypes.h>
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


/*	codec capabilities flags	*/

enum {
	codecCanScale				= 1L << 0,
	codecCanMask				= 1L << 1,
	codecCanMatte				= 1L << 2,
	codecCanTransform			= 1L << 3,
	codecCanTransferMode		= 1L << 4,
	codecCanCopyPrev			= 1L << 5,
	codecCanSpool				= 1L << 6,
	codecCanClipVertical		= 1L << 7,
	codecCanClipRectangular		= 1L << 8,
	codecCanRemapColor			= 1L << 9,
	codecCanFastDither			= 1L << 10,
	codecCanSrcExtract			= 1L << 11,
	codecCanCopyPrevComp		= 1L << 12,
	codecCanAsync				= 1L << 13,
	codecCanMakeMask			= 1L << 14,
	codecCanShift				= 1L << 15,
	codecCanAsyncWhen			= 1L << 16,
	codecCanShieldCursor		= 1L << 17,
	codecCanManagePrevBuffer	= 1L << 18,
	codecHasVolatileBuffer		= 1L << 19,
	codecWantsRegionMask		= 1L << 20,
	codecImageBufferIsOnScreen	= 1L << 21,
	codecWantsDestinationPixels	= 1L << 22,
	codecWantsSpecialScaling	= 1L << 23,
	codecHandlesInputs			= 1L << 24,
	codecCanDoIndirectSurface	= 1L << 25,
	codecIsSequenceSensitive	= 1L << 26,
	codecRequiresOffscreen		= 1L << 27,
	codecRequiresMaskBits		= 1L << 28,
	codecCanRemapResolution		= 1L << 29,
	codecIsDirectToScreenOnly	= 1L << 30,
	codecCanLockSurface			= 1L << 31
};


struct CodecCapabilities {
	long 							flags;
	short 							wantedPixelSize;
	short 							extendWidth;
	short 							extendHeight;
	short 							bandMin;
	short 							bandInc;
	short 							pad;
	unsigned long 					time;
};
typedef struct CodecCapabilities		CodecCapabilities;
/*	codec condition flags	*/

enum {
	codecConditionFirstBand		= 1L << 0,
	codecConditionLastBand		= 1L << 1,
	codecConditionFirstFrame	= 1L << 2,
	codecConditionNewDepth		= 1L << 3,
	codecConditionNewTransform	= 1L << 4,
	codecConditionNewSrcRect	= 1L << 5,
	codecConditionNewMask		= 1L << 6,
	codecConditionNewMatte		= 1L << 7,
	codecConditionNewTransferMode = 1L << 8,
	codecConditionNewClut		= 1L << 9,
	codecConditionNewAccuracy	= 1L << 10,
	codecConditionNewDestination = 1L << 11,
	codecConditionFirstScreen	= 1L << 12,
	codecConditionDoCursor		= 1L << 13,
	codecConditionCatchUpDiff	= 1L << 14,
	codecConditionMaskMayBeChanged = 1L << 15,
	codecConditionToBuffer		= 1L << 16,
	codecConditionCodecChangedMask = 1L << 31
};



enum {
	codecInfoResourceType		= FOUR_CHAR_CODE('cdci'),		/* codec info resource type */
	codecInterfaceVersion		= 2								/* high word returned in component GetVersion */
};


struct CDSequenceDataSourceQueueEntry {
	void *							nextBusy;

	long 							descSeed;
	Handle 							dataDesc;
	void *							data;
	long 							dataSize;

	long 							useCount;

	TimeValue 						frameTime;
	TimeValue 						frameDuration;
	TimeValue 						timeScale;
};
typedef struct CDSequenceDataSourceQueueEntry CDSequenceDataSourceQueueEntry;
typedef CDSequenceDataSourceQueueEntry * CDSequenceDataSourceQueueEntryPtr;

struct CDSequenceDataSource {
	long 							recordSize;

	void *							next;

	ImageSequence 					seqID;
	ImageSequenceDataSource 		sourceID;
	OSType 							sourceType;
	long 							sourceInputNumber;
	void *							dataPtr;
	Handle 							dataDescription;
	long 							changeSeed;
	ICMConvertDataFormatUPP 		transferProc;
	void *							transferRefcon;
	long 							dataSize;

																/* fields available in QT 3 and later */

	QHdrPtr 						dataQueue;					/* queue of CDSequenceDataSourceQueueEntry structures*/

	void *							originalDataPtr;
	long 							originalDataSize;
	Handle 							originalDataDescription;
	long 							originalDataDescriptionSeed;
};
typedef struct CDSequenceDataSource		CDSequenceDataSource;
typedef CDSequenceDataSource *			CDSequenceDataSourcePtr;

struct ICMFrameTimeInfo {
	wide 							startTime;
	long 							scale;
	long 							duration;
};
typedef struct ICMFrameTimeInfo			ICMFrameTimeInfo;
typedef ICMFrameTimeInfo *				ICMFrameTimeInfoPtr;

struct CodecCompressParams {
	ImageSequence 					sequenceID;					/* precompress,bandcompress */
	ImageDescriptionHandle 			imageDescription;			/* precompress,bandcompress */
	Ptr 							data;
	long 							bufferSize;
	long 							frameNumber;
	long 							startLine;
	long 							stopLine;
	long 							conditionFlags;
	CodecFlags 						callerFlags;
	CodecCapabilities *				capabilities;				/* precompress,bandcompress */
	ICMProgressProcRecord 			progressProcRecord;
	ICMCompletionProcRecord 		completionProcRecord;
	ICMFlushProcRecord 				flushProcRecord;

	PixMap 							srcPixMap;					/* precompress,bandcompress */
	PixMap 							prevPixMap;
	CodecQ 							spatialQuality;
	CodecQ 							temporalQuality;
	Fixed 							similarity;
	DataRateParamsPtr 				dataRateParams;
	long 							reserved;

																/* The following fields only exist for QuickTime 2.1 and greater */
	UInt16 							majorSourceChangeSeed;
	UInt16 							minorSourceChangeSeed;
	CDSequenceDataSourcePtr 		sourceData;

																/* The following fields only exit for QuickTime 2.5 and greater */
	long 							preferredPacketSizeInBytes;

																/* The following fields only exit for QuickTime 3.0 and greater */
	long 							requestedBufferWidth;		/* must set codecWantsSpecialScaling to indicate this field is valid*/
	long 							requestedBufferHeight;		/* must set codecWantsSpecialScaling to indicate this field is valid*/
};
typedef struct CodecCompressParams		CodecCompressParams;

struct CodecDecompressParams {
	ImageSequence 					sequenceID;					/* predecompress,banddecompress */
	ImageDescriptionHandle 			imageDescription;			/* predecompress,banddecompress */
	Ptr 							data;
	long 							bufferSize;
	long 							frameNumber;
	long 							startLine;
	long 							stopLine;
	long 							conditionFlags;
	CodecFlags 						callerFlags;
	CodecCapabilities *				capabilities;				/* predecompress,banddecompress */
	ICMProgressProcRecord 			progressProcRecord;
	ICMCompletionProcRecord 		completionProcRecord;
	ICMDataProcRecord 				dataProcRecord;

	CGrafPtr 						port;						/* predecompress,banddecompress */
	PixMap 							dstPixMap;					/* predecompress,banddecompress */
	BitMapPtr 						maskBits;
	PixMapPtr 						mattePixMap;
	Rect 							srcRect;					/* predecompress,banddecompress */
	MatrixRecord *					matrix;						/* predecompress,banddecompress */
	CodecQ 							accuracy;					/* predecompress,banddecompress */
	short 							transferMode;				/* predecompress,banddecompress */
	ICMFrameTimePtr 				frameTime;					/* banddecompress */
	long 							reserved[1];
																/* The following fields only exist for QuickTime 2.0 and greater */
	SInt8 							matrixFlags;				/* high bit set if 2x resize */
	SInt8 							matrixType;
	Rect 							dstRect;					/* only valid for simple transforms */
																/* The following fields only exist for QuickTime 2.1 and greater */
	UInt16 							majorSourceChangeSeed;
	UInt16 							minorSourceChangeSeed;
	CDSequenceDataSourcePtr 		sourceData;

	RgnHandle 						maskRegion;

																/* The following fields only exist for QuickTime 2.5 and greater */

	OSType **						wantedDestinationPixelTypes; /* Handle to 0-terminated list of OSTypes */

	long 							screenFloodMethod;
	long 							screenFloodValue;
	short 							preferredOffscreenPixelSize;

																/* The following fields only exist for QuickTime 3.0 and greater */
	ICMFrameTimeInfoPtr 			syncFrameTime;				/* banddecompress */
	Boolean 						needUpdateOnTimeChange;		/* banddecompress */
	Boolean 						enableBlackLining;
	Boolean 						needUpdateOnSourceChange;	/* band decompress */
	Boolean 						pad;

	long 							unused;

	CGrafPtr 						finalDestinationPort;

	long 							requestedBufferWidth;		/* must set codecWantsSpecialScaling to indicate this field is valid*/
	long 							requestedBufferHeight;		/* must set codecWantsSpecialScaling to indicate this field is valid*/
};
typedef struct CodecDecompressParams	CodecDecompressParams;

enum {
	matrixFlagScale2x			= 1L << 7,
	matrixFlagScale1x			= 1L << 6,
	matrixFlagScaleHalf			= 1L << 5
};


enum {
	kScreenFloodMethodNone		= 0,
	kScreenFloodMethodKeyColor	= 1,
	kScreenFloodMethodAlpha		= 2
};


enum {
	kFlushLastQueuedFrame		= 0,
	kFlushFirstQueuedFrame		= 1
};


enum {
	kNewImageGWorldErase		= 1L << 0
};


struct ImageSubCodecDecompressCapabilities {
	long 							recordSize;					/* sizeof(ImageSubCodecDecompressCapabilities)*/
	long 							decompressRecordSize;		/* size of your codec's decompress record*/
	Boolean 						canAsync;					/* default true*/
	UInt8 							pad[3];
};
typedef struct ImageSubCodecDecompressCapabilities ImageSubCodecDecompressCapabilities;

enum {
	kCodecFrameTypeUnknown		= 0,
	kCodecFrameTypeKey			= 1,
	kCodecFrameTypeDifference	= 2,
	kCodecFrameTypeDroppableDifference = 3
};


struct ImageSubCodecDecompressRecord {
	Ptr 							baseAddr;
	long 							rowBytes;
	Ptr 							codecData;
	ICMProgressProcRecord 			progressProcRecord;
	ICMDataProcRecord 				dataProcRecord;
	void *							userDecompressRecord;		/* pointer to codec-specific per-band data*/
	UInt8 							frameType;
	UInt8 							pad[3];
};
typedef struct ImageSubCodecDecompressRecord ImageSubCodecDecompressRecord;
/* name of parameters or effect -- placed in root container, required */

enum {
	kParameterTitleName			= FOUR_CHAR_CODE('name'),
	kParameterTitleID			= 1
};

/* codec sub-type of parameters or effect -- placed in root container, required */

enum {
	kParameterWhatName			= FOUR_CHAR_CODE('what'),
	kParameterWhatID			= 1
};

/* effect version -- placed in root container, optional, but recommended */

enum {
	kParameterVersionName		= FOUR_CHAR_CODE('vers'),
	kParameterVersionID			= 1
};

/* is effect repeatable -- placed in root container, optional, default is TRUE*/

enum {
	kParameterRepeatableName	= FOUR_CHAR_CODE('pete'),
	kParameterRepeatableID		= 1
};


enum {
	kParameterRepeatableTrue	= 1,
	kParameterRepeatableFalse	= 0
};

/* substitution codec in case effect is missing -- placed in root container, recommended */

enum {
	kParameterAlternateCodecName = FOUR_CHAR_CODE('subs'),
	kParameterAlternateCodecID	= 1
};

/* maximum number of sources -- placed in root container, required */

enum {
	kParameterSourceCountName	= FOUR_CHAR_CODE('srcs'),
	kParameterSourceCountID		= 1
};



enum {
	kParameterDependencyName	= FOUR_CHAR_CODE('deep'),
	kParameterDependencyID		= 1
};


enum {
	kParameterListDependsUponColorProfiles = FOUR_CHAR_CODE('prof'),
	kParameterListDependsUponFonts = FOUR_CHAR_CODE('font')
};


struct ParameterDependancyRecord {
	long 							dependCount;
	OSType 							depends[1];
};
typedef struct ParameterDependancyRecord ParameterDependancyRecord;
/*
   enumeration list in container -- placed in root container, optional unless used by a
   parameter in the list
*/

enum {
	kParameterEnumList			= FOUR_CHAR_CODE('enum')
};


struct EnumValuePair {
	long 							value;
	Str255 							name;
};
typedef struct EnumValuePair			EnumValuePair;

struct EnumListRecord {
	long 							enumCount;					/* number of enumeration items to follow*/
	EnumValuePair 					values[1];					/* values and names for them, packed */
};
typedef struct EnumListRecord			EnumListRecord;
/* atom type of parameter*/

enum {
	kParameterAtomTypeAndID		= FOUR_CHAR_CODE('type')
};


enum {
	kNoAtom						= FOUR_CHAR_CODE('none'),		/* atom type for no data got/set*/
	kAtomNoFlags				= 0x00000000,
	kAtomNotInterpolated		= 0x00000001,					/* atom can never be interpolated*/
	kAtomInterpolateIsOptional	= 0x00000002					/* atom can be interpolated, but it is an advanced user operation*/
};


struct ParameterAtomTypeAndID {
	QTAtomType 						atomType;					/* type of atom this data comes from/goes into*/
	QTAtomID 						atomID;						/* ID of atom this data comes from/goes into*/
	long 							atomFlags;					/* options for this atom*/
	Str255 							atomName;					/* name of this value type*/
};
typedef struct ParameterAtomTypeAndID	ParameterAtomTypeAndID;
/* data type of a parameter*/

enum {
	kParameterDataType			= FOUR_CHAR_CODE('data')
};


enum {
	kParameterTypeDataLong		= kTweenTypeLong,				/* integer value*/
	kParameterTypeDataFixed		= kTweenTypeFixed,				/* fixed point value*/
	kParameterTypeDataRGBValue	= kTweenTypeRGBColor,			/* RGBColor data*/
	kParameterTypeDataDouble	= kTweenTypeQTFloatDouble,		/* IEEE 64 bit floating point value*/
	kParameterTypeDataText		= FOUR_CHAR_CODE('text'),		/* editable text item*/
	kParameterTypeDataEnum		= FOUR_CHAR_CODE('enum'),		/* enumerated lookup value*/
	kParameterTypeDataBitField	= FOUR_CHAR_CODE('bool'),		/* bit field value (something that holds boolean(s))*/
	kParameterTypeDataImage		= FOUR_CHAR_CODE('imag')		/* reference to an image via Picture data*/
};


struct ParameterDataType {
	OSType 							dataType;					/* type of data this item is stored as*/
};
typedef struct ParameterDataType		ParameterDataType;
/*
   alternate (optional) data type -- main data type always required.  
   Must be modified or deleted when modifying main data type.
   Main data type must be modified when alternate is modified.
*/

enum {
	kParameterAlternateDataType	= FOUR_CHAR_CODE('alt1'),
	kParameterTypeDataColorValue = FOUR_CHAR_CODE('cmlr'),		/* CMColor data (supported on machines with ColorSync)*/
	kParameterTypeDataCubic		= FOUR_CHAR_CODE('cubi'),		/* cubic bezier(s) (no built-in support)*/
	kParameterTypeDataNURB		= FOUR_CHAR_CODE('nurb')		/* nurb(s) (no built-in support)*/
};


struct ParameterAlternateDataEntry {
	OSType 							dataType;					/* type of data this item is stored as*/
	QTAtomType 						alternateAtom;				/* where to store*/
};
typedef struct ParameterAlternateDataEntry ParameterAlternateDataEntry;

struct ParameterAlternateDataType {
	long 							numEntries;
	ParameterAlternateDataEntry 	entries[1];
};
typedef struct ParameterAlternateDataType ParameterAlternateDataType;
/* legal values for the parameter*/

enum {
	kParameterDataRange			= FOUR_CHAR_CODE('rang')
};


enum {
	kNoMinimumLongFixed			= 0x7FFFFFFF,					/* ignore minimum/maxiumum values*/
	kNoMaximumLongFixed			= (long)0x80000000,
	kNoScaleLongFixed			= 0,							/* don't perform any scaling of value*/
	kNoPrecision				= (-1)							/* allow as many digits as format*/
};

/* 'text'*/

struct StringRangeRecord {
	long 							maxChars;					/* maximum length of string*/
	long 							maxLines;					/* number of editing lines to use (1 typical, 0 to default)*/
};
typedef struct StringRangeRecord		StringRangeRecord;
/* 'long'*/

struct LongRangeRecord {
	long 							minValue;					/* no less than this*/
	long 							maxValue;					/* no more than this*/
	long 							scaleValue;					/* muliply content by this going in, divide going out*/
	long 							precisionDigits;			/* # digits of precision when editing via typing*/
};
typedef struct LongRangeRecord			LongRangeRecord;
/* 'enum'*/

struct EnumRangeRecord {
	long 							enumID;						/* 'enum' list in root container to search within*/
};
typedef struct EnumRangeRecord			EnumRangeRecord;
/* 'fixd'*/

struct FixedRangeRecord {
	Fixed 							minValue;					/* no less than this*/
	Fixed 							maxValue;					/* no more than this*/
	Fixed 							scaleValue;					/* muliply content by this going in, divide going out*/
	long 							precisionDigits;			/* # digits of precision when editing via typing*/
};
typedef struct FixedRangeRecord			FixedRangeRecord;
/* 'doub'*/

	#define kNoMinimumDouble		(NAN)					// ignore minimum/maxiumum values
	#define kNoMaximumDouble		(NAN)
	#define kNoScaleDouble			(0)						// don't perform any scaling of value
	struct DoubleRangeRecord
		{
		QTFloatDouble		minValue;			// no less than this
		QTFloatDouble		maxValue;			// no more than this
		QTFloatDouble		scaleValue;			// muliply content by this going in, divide going out
		long				precisionDigits;	// # digits of precision when editing via typing
		};
	typedef struct DoubleRangeRecord DoubleRangeRecord;
	
/* 'bool'	*/

struct BooleanRangeRecord {
	long 							maskValue;					/* value to mask on/off to set/clear the boolean*/
};
typedef struct BooleanRangeRecord		BooleanRangeRecord;
/* 'rgb '*/

struct RGBRangeRecord {
	RGBColor 						minColor;
	RGBColor 						maxColor;
};
typedef struct RGBRangeRecord			RGBRangeRecord;
/* 'imag'*/

enum {
	kParameterImageNoFlags		= 0
};


struct ImageRangeRecord {
	long 							imageFlags;
};
typedef struct ImageRangeRecord			ImageRangeRecord;
/* union of all of the above*/

	struct ParameterRangeRecord
		{
		union 
			{
			LongRangeRecord		longRange;
			EnumRangeRecord		enumRange;
			FixedRangeRecord	fixedRange;
			DoubleRangeRecord	doubleRange;
			StringRangeRecord	stringRange;
			BooleanRangeRecord	booleanRange;
			RGBRangeRecord		rgbRange;
			ImageRangeRecord	imageRange;
			} u;
		};
	typedef struct ParameterRangeRecord ParameterRangeRecord;
	
/* UI behavior of a parameter*/

enum {
	kParameterDataBehavior		= FOUR_CHAR_CODE('ditl')
};


enum {
																/* items edited via typing*/
	kParameterItemEditText		= FOUR_CHAR_CODE('edit'),		/* edit text box*/
	kParameterItemEditLong		= FOUR_CHAR_CODE('long'),		/* long number editing box*/
	kParameterItemEditFixed		= FOUR_CHAR_CODE('fixd'),		/* fixed point number editing box*/
	kParameterItemEditDouble	= FOUR_CHAR_CODE('doub'),		/* double number editing box*/
																/* items edited via control(s)*/
	kParameterItemPopUp			= FOUR_CHAR_CODE('popu'),		/* pop up value for enum types*/
	kParameterItemRadioCluster	= FOUR_CHAR_CODE('radi'),		/* radio cluster for enum types*/
	kParameterItemCheckBox		= FOUR_CHAR_CODE('chex'),		/* check box for booleans*/
	kParameterItemControl		= FOUR_CHAR_CODE('cntl'),		/* item controlled via a standard control of some type*/
																/* special user items*/
	kParameterItemLine			= FOUR_CHAR_CODE('line'),		/* line*/
	kParameterItemColorPicker	= FOUR_CHAR_CODE('pick'),		/* color swatch & picker*/
	kParameterItemGroupDivider	= FOUR_CHAR_CODE('divi'),		/* start of a new group of items*/
	kParameterItemStaticText	= FOUR_CHAR_CODE('stat'),		/* display "parameter name" as static text*/
	kParameterItemDragImage		= FOUR_CHAR_CODE('imag'),		/* allow image display, along with drag and drop*/
																/* flags valid for lines and groups*/
	kGraphicsNoFlags			= 0x00000000,					/* no options for graphics*/
	kGraphicsFlagsGray			= 0x00000001,					/* draw lines with gray*/
																/* flags valid for groups*/
	kGroupNoFlags				= 0x00000000,					/* no options for group -- may be combined with graphics options						*/
	kGroupAlignText				= 0x00010000,					/* edit text items in group have the same size*/
	kGroupSurroundBox			= 0x00020000,					/* group should be surrounded with a box*/
	kGroupMatrix				= 0x00040000,					/* side-by-side arrangement of group is okay*/
	kGroupNoName				= 0x00080000,					/* name of group should not be displayed above box*/
																/* flags valid for popup/radiocluster/checkbox/control*/
	kDisableControl				= 0x00000001,
	kDisableWhenNotEqual		= (0x00000000 + kDisableControl),
	kDisableWhenEqual			= (0x00000010 + kDisableControl),
	kDisableWhenLessThan		= (0x00000020 + kDisableControl),
	kDisableWhenGreaterThan		= (0x00000030 + kDisableControl), /* flags valid for popups*/
	kPopupStoreAsString			= 0x00010000
};


struct ControlBehaviors {
	QTAtomID 						groupID;					/* group under control of this item*/
	long 							controlValue;				/* control value for comparison purposes*/
};
typedef struct ControlBehaviors			ControlBehaviors;

struct ParameterDataBehavior {
	OSType 							behaviorType;
	long 							behaviorFlags;
	union {
		ControlBehaviors 				controls;
	} 								u;
};
typedef struct ParameterDataBehavior	ParameterDataBehavior;
/* higher level purpose of a parameter or set of parameters*/

enum {
	kParameterDataUsage			= FOUR_CHAR_CODE('use ')
};


enum {
	kParameterUsagePixels		= FOUR_CHAR_CODE('pixl'),
	kParameterUsageRectangle	= FOUR_CHAR_CODE('rect'),
	kParameterUsagePoint		= FOUR_CHAR_CODE('xy  '),
	kParameterUsage3DPoint		= FOUR_CHAR_CODE('xyz '),
	kParameterUsageDegrees		= FOUR_CHAR_CODE('degr'),
	kParameterUsageRadians		= FOUR_CHAR_CODE('rads'),
	kParameterUsagePercent		= FOUR_CHAR_CODE('pcnt'),
	kParameterUsageSeconds		= FOUR_CHAR_CODE('secs'),
	kParameterUsageMilliseconds	= FOUR_CHAR_CODE('msec'),
	kParameterUsageMicroseconds	= FOUR_CHAR_CODE('µsec'),
	kParameterUsage3by3Matrix	= FOUR_CHAR_CODE('3by3')
};


struct ParameterDataUsage {
	OSType 							usageType;					/* higher level purpose of the data or group*/
};
typedef struct ParameterDataUsage		ParameterDataUsage;
/* default value(s) for a parameter*/

enum {
	kParameterDataDefaultItem	= FOUR_CHAR_CODE('dflt')
};

/* atoms that help to fill in data within the info window */

enum {
	kParameterInfoLongName		= FOUR_CHAR_CODE('©nam'),
	kParameterInfoCopyright		= FOUR_CHAR_CODE('©cpy'),
	kParameterInfoDescription	= FOUR_CHAR_CODE('©inf'),
	kParameterInfoWindowTitle	= FOUR_CHAR_CODE('©wnt'),
	kParameterInfoPicture		= FOUR_CHAR_CODE('©pix'),
	kParameterInfoManufacturer	= FOUR_CHAR_CODE('©man'),
	kParameterInfoIDs			= 1
};

/* flags for ImageCodecValidateParameters */

enum {
	kParameterValidationNoFlags	= 0x00000000,
	kParameterValidationFinalValidation = 0x00000001
};


typedef long 							QTParameterValidationOptions;
/* QTAtomTypes for atoms in image compressor settings containers*/

enum {
	kImageCodecSettingsFieldCount = FOUR_CHAR_CODE('fiel'),		/* Number of fields (UInt8) */
	kImageCodecSettingsFieldOrdering = FOUR_CHAR_CODE('fdom'),	/* Ordering of fields (UInt8)*/
	kImageCodecSettingsFieldOrderingF1F2 = 1,
	kImageCodecSettingsFieldOrderingF2F1 = 2
};

typedef CALLBACK_API( ComponentResult , ImageCodecMPDrawBandProcPtr )(void *refcon, ImageSubCodecDecompressRecord *drp);
typedef STACK_UPP_TYPE(ImageCodecMPDrawBandProcPtr) 			ImageCodecMPDrawBandUPP;
enum { uppImageCodecMPDrawBandProcInfo = 0x000003F0 }; 			/* pascal 4_bytes Func(4_bytes, 4_bytes) */
#define NewImageCodecMPDrawBandProc(userRoutine) 				(ImageCodecMPDrawBandUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppImageCodecMPDrawBandProcInfo, GetCurrentArchitecture())
#define CallImageCodecMPDrawBandProc(userRoutine, refcon, drp) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppImageCodecMPDrawBandProcInfo, (refcon), (drp))
/*	codec selectors 0-127 are reserved by Apple */
/*	codec selectors 128-191 are subtype specific */
/*	codec selectors 192-255 are vendor specific */
/*	codec selectors 256-32767 are available for general use */
/*	negative selectors are reserved by the Component Manager */
EXTERN_API( ComponentResult )
ImageCodecGetCodecInfo			(ComponentInstance 		ci,
								 CodecInfo *			info)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0000, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetCompressionTime	(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 short 					depth,
								 CodecQ *				spatialQuality,
								 CodecQ *				temporalQuality,
								 unsigned long *		time)								FIVEWORDINLINE(0x2F3C, 0x0016, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetMaxCompressionSize	(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 short 					depth,
								 CodecQ 				quality,
								 long *					size)								FIVEWORDINLINE(0x2F3C, 0x0012, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecPreCompress			(ComponentInstance 		ci,
								 CodecCompressParams *	params)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecBandCompress			(ComponentInstance 		ci,
								 CodecCompressParams *	params)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecPreDecompress			(ComponentInstance 		ci,
								 CodecDecompressParams * params)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecBandDecompress		(ComponentInstance 		ci,
								 CodecDecompressParams * params)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecBusy					(ComponentInstance 		ci,
								 ImageSequence 			seq)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetCompressedImageSize (ComponentInstance 	ci,
								 ImageDescriptionHandle  desc,
								 Ptr 					data,
								 long 					bufferSize,
								 ICMDataProcRecordPtr 	dataProc,
								 long *					dataSize)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0008, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetSimilarity			(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 ImageDescriptionHandle  desc,
								 Ptr 					data,
								 Fixed *				similarity)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0009, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecTrimImage				(ComponentInstance 		ci,
								 ImageDescriptionHandle  Desc,
								 Ptr 					inData,
								 long 					inBufferSize,
								 ICMDataProcRecordPtr 	dataProc,
								 Ptr 					outData,
								 long 					outBufferSize,
								 ICMFlushProcRecordPtr 	flushProc,
								 Rect *					trimRect,
								 ICMProgressProcRecordPtr  progressProc)					FIVEWORDINLINE(0x2F3C, 0x0024, 0x000A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecRequestSettings		(ComponentInstance 		ci,
								 Handle 				settings,
								 Rect *					rp,
								 ModalFilterUPP 		filterProc)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x000B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetSettings			(ComponentInstance 		ci,
								 Handle 				settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecSetSettings			(ComponentInstance 		ci,
								 Handle 				settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecFlush					(ComponentInstance 		ci)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecSetTimeCode			(ComponentInstance 		ci,
								 void *					timeCodeFormat,
								 void *					timeCodeTime)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x000F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecIsImageDescriptionEquivalent (ComponentInstance  ci,
								 ImageDescriptionHandle  newDesc,
								 Boolean *				equivalent)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0010, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecNewMemory				(ComponentInstance 		ci,
								 Ptr *					data,
								 Size 					dataSize,
								 long 					dataUse,
								 ICMMemoryDisposedUPP 	memoryGoneProc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0014, 0x0011, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecDisposeMemory			(ComponentInstance 		ci,
								 Ptr 					data)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0012, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecHitTestData			(ComponentInstance 		ci,
								 ImageDescriptionHandle  desc,
								 void *					data,
								 Size 					dataSize,
								 Point 					where,
								 Boolean *				hit)								FIVEWORDINLINE(0x2F3C, 0x0014, 0x0013, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecNewImageBufferMemory	(ComponentInstance 		ci,
								 CodecDecompressParams * params,
								 long 					flags,
								 ICMMemoryDisposedUPP 	memoryGoneProc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0014, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecExtractAndCombineFields (ComponentInstance 	ci,
								 long 					fieldFlags,
								 void *					data1,
								 long 					dataSize1,
								 ImageDescriptionHandle  desc1,
								 void *					data2,
								 long 					dataSize2,
								 ImageDescriptionHandle  desc2,
								 void *					outputData,
								 long *					outDataSize,
								 ImageDescriptionHandle  descOut)							FIVEWORDINLINE(0x2F3C, 0x0028, 0x0015, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetMaxCompressionSizeWithSources (ComponentInstance  ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 short 					depth,
								 CodecQ 				quality,
								 CDSequenceDataSourcePtr  sourceData,
								 long *					size)								FIVEWORDINLINE(0x2F3C, 0x0016, 0x0016, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecSetTimeBase			(ComponentInstance 		ci,
								 void *					base)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0017, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecSourceChanged			(ComponentInstance 		ci,
								 UInt32 				majorSourceChangeSeed,
								 UInt32 				minorSourceChangeSeed,
								 CDSequenceDataSourcePtr  sourceData,
								 long *					flagsOut)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x0018, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecFlushFrame			(ComponentInstance 		ci,
								 UInt32 				flags)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0019, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetSettingsAsText		(ComponentInstance 		ci,
								 Handle *				text)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetParameterListHandle (ComponentInstance 	ci,
								 Handle *				parameterDescriptionHandle)			FIVEWORDINLINE(0x2F3C, 0x0004, 0x001B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetParameterList		(ComponentInstance 		ci,
								 QTAtomContainer *		parameterDescription)				FIVEWORDINLINE(0x2F3C, 0x0004, 0x001C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecCreateStandardParameterDialog (ComponentInstance  ci,
								 QTAtomContainer 		parameterDescription,
								 QTAtomContainer 		parameters,
								 QTParameterDialogOptions  dialogOptions,
								 DialogPtr 				existingDialog,
								 short 					existingUserItem,
								 QTParameterDialog *	createdDialog)						FIVEWORDINLINE(0x2F3C, 0x0016, 0x001D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecIsStandardParameterDialogEvent (ComponentInstance  ci,
								 EventRecord *			pEvent,
								 QTParameterDialog 		createdDialog)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x001E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecDismissStandardParameterDialog (ComponentInstance  ci,
								 QTParameterDialog 		createdDialog)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x001F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecStandardParameterDialogDoAction (ComponentInstance  ci,
								 QTParameterDialog 		createdDialog,
								 long 					action,
								 void *					params)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0020, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecNewImageGWorld		(ComponentInstance 		ci,
								 CodecDecompressParams * params,
								 GWorldPtr *			newGW,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0021, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecDisposeImageGWorld	(ComponentInstance 		ci,
								 GWorldPtr 				theGW)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0022, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecHitTestDataWithFlags	(ComponentInstance 		ci,
								 ImageDescriptionHandle  desc,
								 void *					data,
								 Size 					dataSize,
								 Point 					where,
								 long *					hit,
								 long 					hitFlags)							FIVEWORDINLINE(0x2F3C, 0x0018, 0x0023, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecValidateParameters	(ComponentInstance 		ci,
								 QTAtomContainer 		parameters,
								 QTParameterValidationOptions  validationFlags,
								 StringPtr 				errorString)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0024, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecGetBaseMPWorkFunction	(ComponentInstance 		ci,
								 ComponentMPWorkFunctionUPP * workFunction,
								 void **				refCon,
								 ImageCodecMPDrawBandUPP  drawProc,
								 void *					drawProcRefCon)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x0025, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecPreflight				(ComponentInstance 		ci,
								 CodecDecompressParams * params)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0200, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecInitialize			(ComponentInstance 		ci,
								 ImageSubCodecDecompressCapabilities * cap)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0201, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecBeginBand				(ComponentInstance 		ci,
								 CodecDecompressParams * params,
								 ImageSubCodecDecompressRecord * drp,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0202, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecDrawBand				(ComponentInstance 		ci,
								 ImageSubCodecDecompressRecord * drp)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0203, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecEndBand				(ComponentInstance 		ci,
								 ImageSubCodecDecompressRecord * drp,
								 OSErr 					result,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000A, 0x0204, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecQueueStarting			(ComponentInstance 		ci)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0205, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecQueueStopping			(ComponentInstance 		ci)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0206, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecDroppingFrame			(ComponentInstance 		ci,
								 const ImageSubCodecDecompressRecord * drp)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0207, 0x7000, 0xA82A);




/* selectors for component calls */
enum {
	kImageCodecGetCodecInfoSelect					= 0x0000,
	kImageCodecGetCompressionTimeSelect				= 0x0001,
	kImageCodecGetMaxCompressionSizeSelect			= 0x0002,
	kImageCodecPreCompressSelect					= 0x0003,
	kImageCodecBandCompressSelect					= 0x0004,
	kImageCodecPreDecompressSelect					= 0x0005,
	kImageCodecBandDecompressSelect					= 0x0006,
	kImageCodecBusySelect							= 0x0007,
	kImageCodecGetCompressedImageSizeSelect			= 0x0008,
	kImageCodecGetSimilaritySelect					= 0x0009,
	kImageCodecTrimImageSelect						= 0x000A,
	kImageCodecRequestSettingsSelect				= 0x000B,
	kImageCodecGetSettingsSelect					= 0x000C,
	kImageCodecSetSettingsSelect					= 0x000D,
	kImageCodecFlushSelect							= 0x000E,
	kImageCodecSetTimeCodeSelect					= 0x000F,
	kImageCodecIsImageDescriptionEquivalentSelect	= 0x0010,
	kImageCodecNewMemorySelect						= 0x0011,
	kImageCodecDisposeMemorySelect					= 0x0012,
	kImageCodecHitTestDataSelect					= 0x0013,
	kImageCodecNewImageBufferMemorySelect			= 0x0014,
	kImageCodecExtractAndCombineFieldsSelect		= 0x0015,
	kImageCodecGetMaxCompressionSizeWithSourcesSelect = 0x0016,
	kImageCodecSetTimeBaseSelect					= 0x0017,
	kImageCodecSourceChangedSelect					= 0x0018,
	kImageCodecFlushFrameSelect						= 0x0019,
	kImageCodecGetSettingsAsTextSelect				= 0x001A,
	kImageCodecGetParameterListHandleSelect			= 0x001B,
	kImageCodecGetParameterListSelect				= 0x001C,
	kImageCodecCreateStandardParameterDialogSelect = 0x001D,
	kImageCodecIsStandardParameterDialogEventSelect = 0x001E,
	kImageCodecDismissStandardParameterDialogSelect = 0x001F,
	kImageCodecStandardParameterDialogDoActionSelect = 0x0020,
	kImageCodecNewImageGWorldSelect					= 0x0021,
	kImageCodecDisposeImageGWorldSelect				= 0x0022,
	kImageCodecHitTestDataWithFlagsSelect			= 0x0023,
	kImageCodecValidateParametersSelect				= 0x0024,
	kImageCodecGetBaseMPWorkFunctionSelect			= 0x0025,
	kImageCodecPreflightSelect						= 0x0200,
	kImageCodecInitializeSelect						= 0x0201,
	kImageCodecBeginBandSelect						= 0x0202,
	kImageCodecDrawBandSelect						= 0x0203,
	kImageCodecEndBandSelect						= 0x0204,
	kImageCodecQueueStartingSelect					= 0x0205,
	kImageCodecQueueStoppingSelect					= 0x0206,
	kImageCodecDroppingFrameSelect					= 0x0207
};










enum {
	kMotionJPEGTag				= FOUR_CHAR_CODE('mjpg'),
	kJPEGQuantizationTablesImageDescriptionExtension = FOUR_CHAR_CODE('mjqt'),
	kJPEGHuffmanTablesImageDescriptionExtension = FOUR_CHAR_CODE('mjht'),
	kFieldInfoImageDescriptionExtension = FOUR_CHAR_CODE('fiel') /* image description extension describing the field count and field orderings*/
};


enum {
	kFieldOrderUnknown			= 0,
	kFieldsStoredF1F2DisplayedF1F2 = 1,
	kFieldsStoredF1F2DisplayedF2F1 = 2,
	kFieldsStoredF2F1DisplayedF1F2 = 5,
	kFieldsStoredF2F1DisplayedF2F1 = 6
};


struct MotionJPEGApp1Marker {
	long 							unused;
	long 							tag;
	long 							fieldSize;
	long 							paddedFieldSize;
	long 							offsetToNextField;
	long 							qTableOffset;
	long 							huffmanTableOffset;
	long 							sofOffset;
	long 							sosOffset;
	long 							soiOffset;
};
typedef struct MotionJPEGApp1Marker		MotionJPEGApp1Marker;

struct FieldInfoImageDescriptionExtension {
	UInt8 							fieldCount;
	UInt8 							fieldOrderings;
};
typedef struct FieldInfoImageDescriptionExtension FieldInfoImageDescriptionExtension;

EXTERN_API( ComponentResult )
QTPhotoSetSampling				(ComponentInstance 		codec,
								 short 					yH,
								 short 					yV,
								 short 					cbH,
								 short 					cbV,
								 short 					crH,
								 short 					crV)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTPhotoSetRestartInterval		(ComponentInstance 		codec,
								 unsigned short 		restartInterval)					FIVEWORDINLINE(0x2F3C, 0x0002, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTPhotoDefineHuffmanTable		(ComponentInstance 		codec,
								 short 					componentNumber,
								 Boolean 				isDC,
								 unsigned char *		lengthCounts,
								 unsigned char *		values)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTPhotoDefineQuantizationTable	(ComponentInstance 		codec,
								 short 					componentNumber,
								 unsigned char *		table)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0103, 0x7000, 0xA82A);


/* selectors for component calls */
enum {
	kQTPhotoSetSamplingSelect						= 0x0100,
	kQTPhotoSetRestartIntervalSelect				= 0x0101,
	kQTPhotoDefineHuffmanTableSelect				= 0x0102,
	kQTPhotoDefineQuantizationTableSelect			= 0x0103
};




/* source identifier -- placed in root container of description, one or more required */

enum {
	kEffectSourceName			= FOUR_CHAR_CODE('src ')
};


/* source type -- placed in the input map to identify the source kind */

enum {
	kEffectDataSourceType		= FOUR_CHAR_CODE('dtst')
};

/*  default effect types */

enum {
	kEffectRawSource			= 0,							/* the source is raw image data*/
	kEffectGenericType			= FOUR_CHAR_CODE('geff')		/* generic effect for combining others*/
};

typedef struct EffectSource 			EffectSource;

typedef EffectSource *					EffectSourcePtr;

union SourceData {
	CDSequenceDataSourcePtr 		image;
	EffectSourcePtr 				effect;
};
typedef union SourceData				SourceData;


struct EffectSource {
	long 							effectType;					/* type of effect or kEffectRawSource if raw ICM data*/
	Ptr 							data;						/* track data for this effect*/
	SourceData 						source;						/* source/effect pointers*/
	EffectSourcePtr 				next;						/* the next source for the parent effect*/
};


struct EffectsFrameParams {
	ICMFrameTimeRecord 				frameTime;					/* timing data*/
	long 							effectDuration;				/* the duration of a single effect frame*/
	Boolean 						doAsync;					/* set to true if the effect can go async*/
	unsigned char 					pad[3];
	EffectSourcePtr 				source;						/* ptr to the source input tree*/
	void *							refCon;						/* storage for the effect*/
};
typedef struct EffectsFrameParams		EffectsFrameParams;
typedef EffectsFrameParams *			EffectsFrameParamsPtr;


EXTERN_API( ComponentResult )
ImageCodecEffectSetup			(ComponentInstance 		effect,
								 CodecDecompressParams * p)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0300, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecEffectBegin			(ComponentInstance 		effect,
								 CodecDecompressParams * p,
								 EffectsFrameParamsPtr 	ePtr)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0301, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecEffectRenderFrame		(ComponentInstance 		effect,
								 EffectsFrameParamsPtr 	p)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0302, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecEffectConvertEffectSourceToFormat (ComponentInstance  effect,
								 EffectSourcePtr 		sourceToConvert,
								 ImageDescriptionHandle  requestedDesc)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0303, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecEffectCancel			(ComponentInstance 		effect,
								 EffectsFrameParamsPtr 	p)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0304, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ImageCodecEffectGetSpeed		(ComponentInstance 		effect,
								 QTAtomContainer 		parameters,
								 Fixed *				pFPS)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0305, 0x7000, 0xA82A);



/* selectors for component calls */
enum {
	kImageCodecEffectSetupSelect					= 0x0300,
	kImageCodecEffectBeginSelect					= 0x0301,
	kImageCodecEffectRenderFrameSelect				= 0x0302,
	kImageCodecEffectConvertEffectSourceToFormatSelect = 0x0303,
	kImageCodecEffectCancelSelect					= 0x0304,
	kImageCodecEffectGetSpeedSelect					= 0x0305
};


/* curve atom types and data structures */

enum {
	kCurvePathAtom				= FOUR_CHAR_CODE('path'),
	kCurveEndAtom				= FOUR_CHAR_CODE('zero'),
	kCurveAntialiasControlAtom	= FOUR_CHAR_CODE('anti'),
	kCurveAntialiasOff			= 0,
	kCurveAntialiasOn			= (long)0xFFFFFFFF,
	kCurveFillTypeAtom			= FOUR_CHAR_CODE('fill'),
	kCurvePenThicknessAtom		= FOUR_CHAR_CODE('pent'),
	kCurveMiterLimitAtom		= FOUR_CHAR_CODE('mitr'),
	kCurveJoinAttributesAtom	= FOUR_CHAR_CODE('join'),
	kCurveMinimumDepthAtom		= FOUR_CHAR_CODE('mind'),
	kCurveDepthAlwaysOffscreenMask = (long)0x80000000,
	kCurveTransferModeAtom		= FOUR_CHAR_CODE('xfer'),
	kCurveGradientAngleAtom		= FOUR_CHAR_CODE('angl'),
	kCurveGradientRadiusAtom	= FOUR_CHAR_CODE('radi'),
	kCurveGradientOffsetAtom	= FOUR_CHAR_CODE('cent')
};


enum {
	kCurveARGBColorAtom			= FOUR_CHAR_CODE('argb')
};


struct ARGBColor {
	unsigned short 					alpha;
	unsigned short 					red;
	unsigned short 					green;
	unsigned short 					blue;
};
typedef struct ARGBColor				ARGBColor;

enum {
	kCurveGradientRecordAtom	= FOUR_CHAR_CODE('grad')
};


struct GradientColorRecord {
	ARGBColor 						thisColor;
	Fixed 							endingPercentage;
};
typedef struct GradientColorRecord		GradientColorRecord;

typedef GradientColorRecord *			GradientColorPtr;

enum {
	kCurveGradientTypeAtom		= FOUR_CHAR_CODE('grdt')
};

/* currently supported gradient types */

enum {
	kLinearGradient				= 0,
	kCircularGradient			= 1
};

typedef long 							GradientType;
EXTERN_API( ComponentResult )
CurveGetLength					(ComponentInstance 		effect,
								 gxPaths *				target,
								 long 					index,
								 wide *					wideLength)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveLengthToPoint				(ComponentInstance 		effect,
								 gxPaths *				target,
								 long 					index,
								 Fixed 					length,
								 FixedPoint *			location,
								 FixedPoint *			tangent)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveNewPath					(ComponentInstance 		effect,
								 Handle *				pPath)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveCountPointsInPath			(ComponentInstance 		effect,
								 gxPaths *				aPath,
								 unsigned long 			contourIndex,
								 unsigned long *		pCount)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveGetPathPoint				(ComponentInstance 		effect,
								 gxPaths *				aPath,
								 unsigned long 			contourIndex,
								 unsigned long 			pointIndex,
								 gxPoint *				thePoint,
								 Boolean *				ptIsOnPath)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0104, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveInsertPointIntoPath		(ComponentInstance 		effect,
								 gxPoint *				aPoint,
								 Handle 				thePath,
								 unsigned long 			contourIndex,
								 unsigned long 			pointIndex,
								 Boolean 				ptIsOnPath)							FIVEWORDINLINE(0x2F3C, 0x0012, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveSetPathPoint				(ComponentInstance 		effect,
								 gxPaths *				aPath,
								 unsigned long 			contourIndex,
								 unsigned long 			pointIndex,
								 gxPoint *				thePoint,
								 Boolean 				ptIsOnPath)							FIVEWORDINLINE(0x2F3C, 0x0012, 0x0106, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveGetNearestPathPoint		(ComponentInstance 		effect,
								 gxPaths *				aPath,
								 FixedPoint *			thePoint,
								 unsigned long *		contourIndex,
								 unsigned long *		pointIndex,
								 Fixed *				theDelta)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0107, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurvePathPointToLength			(ComponentInstance 		ci,
								 gxPaths *				aPath,
								 Fixed 					startDist,
								 Fixed 					endDist,
								 FixedPoint *			thePoint,
								 Fixed *				pLength)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0108, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveCreateVectorStream			(ComponentInstance 		effect,
								 Handle *				pStream)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0109, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveAddAtomToVectorStream		(ComponentInstance 		effect,
								 OSType 				atomType,
								 Size 					atomSize,
								 void *					pAtomData,
								 Handle 				vectorStream)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x010A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveAddPathAtomToVectorStream	(ComponentInstance 		effect,
								 Handle 				pathData,
								 Handle 				vectorStream)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x010B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveAddZeroAtomToVectorStream	(ComponentInstance 		effect,
								 Handle 				vectorStream)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x010C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
CurveGetAtomDataFromVectorStream (ComponentInstance 	effect,
								 Handle 				vectorStream,
								 long 					atomType,
								 long *					dataSize,
								 Ptr *					dataPtr)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x010D, 0x7000, 0xA82A);



/* selectors for component calls */
enum {
	kCurveGetLengthSelect							= 0x0100,
	kCurveLengthToPointSelect						= 0x0101,
	kCurveNewPathSelect								= 0x0102,
	kCurveCountPointsInPathSelect					= 0x0103,
	kCurveGetPathPointSelect						= 0x0104,
	kCurveInsertPointIntoPathSelect					= 0x0105,
	kCurveSetPathPointSelect						= 0x0106,
	kCurveGetNearestPathPointSelect					= 0x0107,
	kCurvePathPointToLengthSelect					= 0x0108,
	kCurveCreateVectorStreamSelect					= 0x0109,
	kCurveAddAtomToVectorStreamSelect				= 0x010A,
	kCurveAddPathAtomToVectorStreamSelect			= 0x010B,
	kCurveAddZeroAtomToVectorStreamSelect			= 0x010C,
	kCurveGetAtomDataFromVectorStreamSelect			= 0x010D
};
/* UPP call backs */

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

#endif /* __IMAGECODEC__ */

