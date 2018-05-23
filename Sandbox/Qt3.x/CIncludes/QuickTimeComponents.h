/*
 	File:		QuickTimeComponents.h
 
 	Contains:	QuickTime Interfaces.
 
 	Version:	Technology:	QuickTime 3.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __QUICKTIMECOMPONENTS__
#define __QUICKTIMECOMPONENTS__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif
#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __IMAGECOMPRESSION__
#include <ImageCompression.h>
#endif
#ifndef __MOVIES__
#include <Movies.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __VIDEO__
#include <Video.h>
#endif
#ifndef __SOUND__
#include <Sound.h>
#endif
#ifndef __QUICKTIMEMUSIC__
#include <QuickTimeMusic.h>
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
	clockComponentType			= FOUR_CHAR_CODE('clok'),
	systemTickClock				= FOUR_CHAR_CODE('tick'),		/* subtype: 60ths since boot		*/
	systemSecondClock			= FOUR_CHAR_CODE('seco'),		/* subtype: seconds since 1904		*/
	systemMillisecondClock		= FOUR_CHAR_CODE('mill'),		/* subtype: 1000ths since boot		*/
	systemMicrosecondClock		= FOUR_CHAR_CODE('micr')		/* subtype: 1000000ths since boot	*/
};


enum {
	kClockRateIsLinear			= 1,
	kClockImplementsCallBacks	= 2,
	kClockCanHandleIntermittentSound = 4						/* sound clocks only */
};

#if OLDROUTINENAMES
#define GetClockTime(aClock, out) ClockGetTime(aClock, out)
#endif
/** These are Clock procedures **/
EXTERN_API( ComponentResult )
ClockGetTime					(ComponentInstance 		aClock,
								 TimeRecord *			out)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0001, 0x7000, 0xA82A);


EXTERN_API( QTCallBack )
ClockNewCallBack				(ComponentInstance 		aClock,
								 TimeBase 				tb,
								 short 					callBackType)						FIVEWORDINLINE(0x2F3C, 0x0006, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockDisposeCallBack			(ComponentInstance 		aClock,
								 QTCallBack 			cb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockCallMeWhen					(ComponentInstance 		aClock,
								 QTCallBack 			cb,
								 long 					param1,
								 long 					param2,
								 long 					param3)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockCancelCallBack				(ComponentInstance 		aClock,
								 QTCallBack 			cb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockRateChanged				(ComponentInstance 		aClock,
								 QTCallBack 			cb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockTimeChanged				(ComponentInstance 		aClock,
								 QTCallBack 			cb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockSetTimeBase				(ComponentInstance 		aClock,
								 TimeBase 				tb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0008, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockStartStopChanged			(ComponentInstance 		aClock,
								 QTCallBack 			cb,
								 Boolean 				startChanged,
								 Boolean 				stopChanged)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0009, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
ClockGetRate					(ComponentInstance 		aClock,
								 Fixed *				rate)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000A, 0x7000, 0xA82A);





enum {
	StandardCompressionType		= FOUR_CHAR_CODE('scdi'),
	StandardCompressionSubType	= FOUR_CHAR_CODE('imag'),
	StandardCompressionSubTypeSound = FOUR_CHAR_CODE('soun')
};


typedef CALLBACK_API( Boolean , SCModalFilterProcPtr )(DialogPtr theDialog, EventRecord *theEvent, short *itemHit, long refcon);
typedef CALLBACK_API( short , SCModalHookProcPtr )(DialogPtr theDialog, short itemHit, void *params, long refcon);
typedef STACK_UPP_TYPE(SCModalFilterProcPtr) 					SCModalFilterUPP;
typedef STACK_UPP_TYPE(SCModalHookProcPtr) 						SCModalHookUPP;
/*	Preference flags.*/

enum {
	scListEveryCodec			= 1L << 1,
	scAllowZeroFrameRate		= 1L << 2,
	scAllowZeroKeyFrameRate		= 1L << 3,
	scShowBestDepth				= 1L << 4,
	scUseMovableModal			= 1L << 5,
	scDisableFrameRateItem		= 1L << 6
};


/*	Possible test flags for setting test image.*/

enum {
	scPreferCropping			= 1 << 0,
	scPreferScaling				= 1 << 1,
	scPreferScalingAndCropping	= scPreferScaling | scPreferCropping,
	scDontDetermineSettingsFromTestImage = 1 << 2
};


/*	Dimensions of the image preview box.*/

enum {
	scTestImageWidth			= 80,
	scTestImageHeight			= 80
};

/*	Possible items returned by hookProc.*/

enum {
	scOKItem					= 1,
	scCancelItem				= 2,
	scCustomItem				= 3
};

/*	Result returned when user cancelled.*/

enum {
	scUserCancelled				= 1
};



/* Component selectors*/

enum {
	scPositionRect				= 2,
	scPositionDialog			= 3,
	scSetTestImagePictHandle	= 4,
	scSetTestImagePictFile		= 5,
	scSetTestImagePixMap		= 6,
	scGetBestDeviceRect			= 7,
	scRequestImageSettings		= 10,
	scCompressImage				= 11,
	scCompressPicture			= 12,
	scCompressPictureFile		= 13,
	scRequestSequenceSettings	= 14,
	scCompressSequenceBegin		= 15,
	scCompressSequenceFrame		= 16,
	scCompressSequenceEnd		= 17,
	scDefaultPictHandleSettings	= 18,
	scDefaultPictFileSettings	= 19,
	scDefaultPixMapSettings		= 20,
	scGetInfo					= 21,
	scSetInfo					= 22,
	scNewGWorld					= 23
};

/*	Get/SetInfo structures.*/


struct SCSpatialSettings {
	CodecType 						codecType;
	CodecComponent 					codec;
	short 							depth;
	CodecQ 							spatialQuality;
};
typedef struct SCSpatialSettings		SCSpatialSettings;

struct SCTemporalSettings {
	CodecQ 							temporalQuality;
	Fixed 							frameRate;
	long 							keyFrameRate;
};
typedef struct SCTemporalSettings		SCTemporalSettings;

struct SCDataRateSettings {
	long 							dataRate;
	long 							frameDuration;
	CodecQ 							minSpatialQuality;
	CodecQ 							minTemporalQuality;
};
typedef struct SCDataRateSettings		SCDataRateSettings;

struct SCExtendedProcs {
	SCModalFilterUPP 				filterProc;
	SCModalHookUPP 					hookProc;
	long 							refcon;
	Str31 							customName;
};
typedef struct SCExtendedProcs			SCExtendedProcs;
/*	Get/SetInfo selectors*/

enum {
	scSpatialSettingsType		= FOUR_CHAR_CODE('sptl'),		/* pointer to SCSpatialSettings struct*/
	scTemporalSettingsType		= FOUR_CHAR_CODE('tprl'),		/* pointer to SCTemporalSettings struct*/
	scDataRateSettingsType		= FOUR_CHAR_CODE('drat'),		/* pointer to SCDataRateSettings struct*/
	scColorTableType			= FOUR_CHAR_CODE('clut'),		/* pointer to CTabHandle*/
	scProgressProcType			= FOUR_CHAR_CODE('prog'),		/* pointer to ProgressRecord struct*/
	scExtendedProcsType			= FOUR_CHAR_CODE('xprc'),		/* pointer to SCExtendedProcs struct*/
	scPreferenceFlagsType		= FOUR_CHAR_CODE('pref'),		/* pointer to long*/
	scSettingsStateType			= FOUR_CHAR_CODE('ssta'),		/* pointer to Handle*/
	scSequenceIDType			= FOUR_CHAR_CODE('sequ'),		/* pointer to ImageSequence*/
	scWindowPositionType		= FOUR_CHAR_CODE('wndw'),		/* pointer to Point*/
	scCodecFlagsType			= FOUR_CHAR_CODE('cflg'),		/* pointer to CodecFlags*/
	scCodecSettingsType			= FOUR_CHAR_CODE('cdec'),		/* pointer to Handle*/
	scForceKeyValueType			= FOUR_CHAR_CODE('ksim'),		/* pointer to long*/
	scSoundSampleRateType		= FOUR_CHAR_CODE('ssrt'),		/* pointer to UnsignedFixed*/
	scSoundSampleSizeType		= FOUR_CHAR_CODE('ssss'),		/* pointer to short*/
	scSoundChannelCountType		= FOUR_CHAR_CODE('sscc'),		/* pointer to short*/
	scSoundCompressionType		= FOUR_CHAR_CODE('ssct'),		/* pointer to OSType*/
	scCompressionListType		= FOUR_CHAR_CODE('ctyl')		/* pointer to OSType Handle*/
};

/*	scTypeNotFoundErr returned by Get/SetInfo when type cannot be found.*/



struct SCParams {
	long 							flags;
	CodecType 						theCodecType;
	CodecComponent 					theCodec;
	CodecQ 							spatialQuality;
	CodecQ 							temporalQuality;
	short 							depth;
	Fixed 							frameRate;
	long 							keyFrameRate;
	long 							reserved1;
	long 							reserved2;
};
typedef struct SCParams					SCParams;

enum {
	scGetCompression			= 1,
	scShowMotionSettings		= 1L << 0,
	scSettingsChangedItem		= -1
};


enum {
	scCompressFlagIgnoreIdenticalFrames = 1
};

/* QTAtomTypes for atoms found in settings atom containers*/

enum {
	kQTSettingsVideo			= FOUR_CHAR_CODE('vide'),		/* Container for video/image compression related atoms (Get/SetInfo selectors)*/
	kQTSettingsSound			= FOUR_CHAR_CODE('soun')		/* Container for sound compression related atoms (Get/SetInfo selectors)*/
};


#define SCGetCompression(ci, params, where) SCGetCompressionExtended(ci,params,where,0,0,0,0)
/** These are Progress procedures **/
EXTERN_API( ComponentResult )
SCGetCompressionExtended		(ComponentInstance 		ci,
								 SCParams *				params,
								 Point 					where,
								 SCModalFilterUPP 		filterProc,
								 SCModalHookUPP 		hookProc,
								 long 					refcon,
								 StringPtr 				customName)							FIVEWORDINLINE(0x2F3C, 0x0018, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCPositionRect					(ComponentInstance 		ci,
								 Rect *					rp,
								 Point *				where)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCPositionDialog				(ComponentInstance 		ci,
								 short 					id,
								 Point *				where)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCSetTestImagePictHandle		(ComponentInstance 		ci,
								 PicHandle 				testPict,
								 Rect *					testRect,
								 short 					testFlags)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCSetTestImagePictFile			(ComponentInstance 		ci,
								 short 					testFileRef,
								 Rect *					testRect,
								 short 					testFlags)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCSetTestImagePixMap			(ComponentInstance 		ci,
								 PixMapHandle 			testPixMap,
								 Rect *					testRect,
								 short 					testFlags)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCGetBestDeviceRect				(ComponentInstance 		ci,
								 Rect *					r)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
SCRequestImageSettings			(ComponentInstance 		ci)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCCompressImage					(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 ImageDescriptionHandle * desc,
								 Handle *				data)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x000B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCCompressPicture				(ComponentInstance 		ci,
								 PicHandle 				srcPicture,
								 PicHandle 				dstPicture)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x000C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCCompressPictureFile			(ComponentInstance 		ci,
								 short 					srcRefNum,
								 short 					dstRefNum)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCRequestSequenceSettings		(ComponentInstance 		ci)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCCompressSequenceBegin			(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 ImageDescriptionHandle * desc)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x000F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCCompressSequenceFrame			(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 const Rect *			srcRect,
								 Handle *				data,
								 long *					dataSize,
								 short *				notSyncFlag)						FIVEWORDINLINE(0x2F3C, 0x0014, 0x0010, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCCompressSequenceEnd			(ComponentInstance 		ci)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0011, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCDefaultPictHandleSettings		(ComponentInstance 		ci,
								 PicHandle 				srcPicture,
								 short 					motion)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0012, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCDefaultPictFileSettings		(ComponentInstance 		ci,
								 short 					srcRef,
								 short 					motion)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0013, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCDefaultPixMapSettings			(ComponentInstance 		ci,
								 PixMapHandle 			src,
								 short 					motion)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0014, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCGetInfo						(ComponentInstance 		ci,
								 OSType 				infoType,
								 void *					info)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0015, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCSetInfo						(ComponentInstance 		ci,
								 OSType 				infoType,
								 void *					info)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0016, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCNewGWorld						(ComponentInstance 		ci,
								 GWorldPtr *			gwp,
								 Rect *					rp,
								 GWorldFlags 			flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0017, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCSetCompressFlags				(ComponentInstance 		ci,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0018, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCGetCompressFlags				(ComponentInstance 		ci,
								 long *					flags)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0019, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCGetSettingsAsText				(ComponentInstance 		ci,
								 Handle *				text)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCGetSettingsAsAtomContainer	(ComponentInstance 		ci,
								 QTAtomContainer *		settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SCSetSettingsFromAtomContainer	(ComponentInstance 		ci,
								 QTAtomContainer 		settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001C, 0x7000, 0xA82A);





enum {
	TweenComponentType			= FOUR_CHAR_CODE('twen')
};


typedef ComponentInstance 				TweenerComponent;
EXTERN_API( ComponentResult )
TweenerInitialize				(TweenerComponent 		tc,
								 QTAtomContainer 		container,
								 QTAtom 				tweenAtom,
								 QTAtom 				dataAtom)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TweenerDoTween					(TweenerComponent 		tc,
								 TweenRecord *			tr)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TweenerReset					(TweenerComponent 		tc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0003, 0x7000, 0xA82A);




enum {
	TCSourceRefNameType			= FOUR_CHAR_CODE('name')
};


enum {
	tcDropFrame					= 1 << 0,
	tc24HourMax					= 1 << 1,
	tcNegTimesOK				= 1 << 2,
	tcCounter					= 1 << 3
};


struct TimeCodeDef {
	long 							flags;						/* drop-frame, etc.*/
	TimeScale 						fTimeScale;					/* time scale of frameDuration (eg. 2997)*/
	TimeValue 						frameDuration;				/* duration of each frame (eg. 100)*/
	UInt8 							numFrames;					/* frames/sec for timecode (eg. 30) OR frames/tick for counter mode*/
	UInt8 							padding;					/* unused padding byte*/
};
typedef struct TimeCodeDef				TimeCodeDef;

enum {
	tctNegFlag					= 0x80							/* negative bit is in minutes*/
};


struct TimeCodeTime {
	UInt8 							hours;
	UInt8 							minutes;
	UInt8 							seconds;
	UInt8 							frames;
};
typedef struct TimeCodeTime				TimeCodeTime;

struct TimeCodeCounter {
	long 							counter;
};
typedef struct TimeCodeCounter			TimeCodeCounter;

union TimeCodeRecord {
	TimeCodeTime 					t;
	TimeCodeCounter 				c;
};
typedef union TimeCodeRecord			TimeCodeRecord;

struct TimeCodeDescription {
	long 							descSize;					/* standard sample description header*/
	long 							dataFormat;
	long 							resvd1;
	short 							resvd2;
	short 							dataRefIndex;
	long 							flags;						/* timecode specific stuff*/
	TimeCodeDef 					timeCodeDef;
	long 							srcRef[1];
};
typedef struct TimeCodeDescription		TimeCodeDescription;
typedef TimeCodeDescription *			TimeCodeDescriptionPtr;
typedef TimeCodeDescriptionPtr *		TimeCodeDescriptionHandle;

enum {
	tcdfShowTimeCode			= 1 << 0
};



struct TCTextOptions {
	short 							txFont;
	short 							txFace;
	short 							txSize;
	short 							pad;						/* let's make it longword aligned - thanks.. */
	RGBColor 						foreColor;
	RGBColor 						backColor;
};
typedef struct TCTextOptions			TCTextOptions;
typedef TCTextOptions *					TCTextOptionsPtr;
EXTERN_API( HandlerError )
TCGetCurrentTimeCode			(MediaHandler 			mh,
								 long *					frameNum,
								 TimeCodeDef *			tcdef,
								 TimeCodeRecord *		tcrec,
								 UserData *				srcRefH)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x0101, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCGetTimeCodeAtTime				(MediaHandler 			mh,
								 TimeValue 				mediaTime,
								 long *					frameNum,
								 TimeCodeDef *			tcdef,
								 TimeCodeRecord *		tcdata,
								 UserData *				srcRefH)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0102, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCTimeCodeToString				(MediaHandler 			mh,
								 TimeCodeDef *			tcdef,
								 TimeCodeRecord *		tcrec,
								 StringPtr 				tcStr)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0103, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCTimeCodeToFrameNumber			(MediaHandler 			mh,
								 TimeCodeDef *			tcdef,
								 TimeCodeRecord *		tcrec,
								 long *					frameNumber)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0104, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCFrameNumberToTimeCode			(MediaHandler 			mh,
								 long 					frameNumber,
								 TimeCodeDef *			tcdef,
								 TimeCodeRecord *		tcrec)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0105, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCGetSourceRef					(MediaHandler 			mh,
								 TimeCodeDescriptionHandle  tcdH,
								 UserData *				srefH)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0106, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCSetSourceRef					(MediaHandler 			mh,
								 TimeCodeDescriptionHandle  tcdH,
								 UserData 				srefH)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0107, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCSetTimeCodeFlags				(MediaHandler 			mh,
								 long 					flags,
								 long 					flagsMask)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0108, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCGetTimeCodeFlags				(MediaHandler 			mh,
								 long *					flags)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0109, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCSetDisplayOptions				(MediaHandler 			mh,
								 TCTextOptionsPtr 		textOptions)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x010A, 0x7000, 0xA82A);

EXTERN_API( HandlerError )
TCGetDisplayOptions				(MediaHandler 			mh,
								 TCTextOptionsPtr 		textOptions)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x010B, 0x7000, 0xA82A);




typedef ComponentInstance 				MovieImportComponent;
typedef ComponentInstance 				MovieExportComponent;

enum {
	MovieImportType				= FOUR_CHAR_CODE('eat '),
	MovieExportType				= FOUR_CHAR_CODE('spit')
};


enum {
	canMovieImportHandles		= 1 << 0,
	canMovieImportFiles			= 1 << 1,
	hasMovieImportUserInterface	= 1 << 2,
	canMovieExportHandles		= 1 << 3,
	canMovieExportFiles			= 1 << 4,
	hasMovieExportUserInterface	= 1 << 5,
	dontAutoFileMovieImport		= 1 << 6,
	canMovieExportAuxDataHandle	= 1 << 7,
	canMovieImportValidateHandles = 1 << 8,
	canMovieImportValidateFile	= 1 << 9,
	dontRegisterWithEasyOpen	= 1 << 10,
	canMovieImportInPlace		= 1 << 11,
	movieImportSubTypeIsFileExtension = 1 << 12,
	canMovieImportPartial		= 1 << 13,
	hasMovieImportMIMEList		= 1 << 14,
	canMovieExportFromProcedures = 1 << 15,
	canMovieExportValidateMovie	= 1L << 16,
	movieExportNeedsResourceFork = 1L << 17,
	canMovieImportDataReferences = 1L << 18,
	movieExportMustGetSourceMediaType = 1L << 19,
	reservedForUseByGraphicsImporters = 1L << 23
};


enum {
	movieImportCreateTrack		= 1,
	movieImportInParallel		= 2,
	movieImportMustUseTrack		= 4
};


enum {
	movieImportResultUsedMultipleTracks = 8
};


enum {
	kMovieExportTextOnly		= 0,
	kMovieExportAbsoluteTime	= 1,
	kMovieExportRelativeTime	= 2
};


enum {
	kMIDIImportSilenceBefore	= 1 << 0,
	kMIDIImportSilenceAfter		= 1 << 1,
	kMIDIImport20Playable		= 1 << 2,
	kMIDIImportWantLyrics		= 1 << 3
};


enum {
	kMimeInfoMimeTypeTag		= FOUR_CHAR_CODE('mime'),
	kMimeInfoFileExtensionTag	= FOUR_CHAR_CODE('ext '),
	kMimeInfoDescriptionTag		= FOUR_CHAR_CODE('desc'),
	kMimeInfoGroupTag			= FOUR_CHAR_CODE('grop'),
	kMimeInfoDoNotOverrideExistingFileTypeAssociation = FOUR_CHAR_CODE('nofa')
};


enum {
	kQTFileTypeAIFF				= FOUR_CHAR_CODE('AIFF'),
	kQTFileTypeAIFC				= FOUR_CHAR_CODE('AIFC'),
	kQTFileTypeDVC				= FOUR_CHAR_CODE('dvc!'),
	kQTFileTypeMIDI				= FOUR_CHAR_CODE('Midi'),
	kQTFileTypePicture			= FOUR_CHAR_CODE('PICT'),
	kQTFileTypeMovie			= FOUR_CHAR_CODE('MooV'),
	kQTFileTypeText				= FOUR_CHAR_CODE('TEXT'),
	kQTFileTypeWave				= FOUR_CHAR_CODE('WAVE'),
	kQTFileTypeSystemSevenSound	= FOUR_CHAR_CODE('sfil'),
	kQTFileTypeMuLaw			= FOUR_CHAR_CODE('ULAW'),
	kQTFileTypeAVI				= FOUR_CHAR_CODE('VfW '),
	kQTFileTypeSoundDesignerII	= FOUR_CHAR_CODE('Sd2f'),
	kQTFileTypeAudioCDTrack		= FOUR_CHAR_CODE('trak'),
	kQTFileTypePICS				= FOUR_CHAR_CODE('PICS'),
	kQTFileTypeGIF				= FOUR_CHAR_CODE('GIFf'),
	kQTFileTypePhotoShop		= FOUR_CHAR_CODE('8BPS'),
	kQTFileTypeSGIImage			= FOUR_CHAR_CODE('.SGI'),
	kQTFileTypeBMP				= FOUR_CHAR_CODE('BMPf'),
	kQTFileTypeJPEG				= FOUR_CHAR_CODE('JPEG'),
	kQTFileTypeJFIF				= FOUR_CHAR_CODE('JPEG'),
	kQTFileTypeMacPaint			= FOUR_CHAR_CODE('PNTG'),
	kQTFileTypeTargaImage		= FOUR_CHAR_CODE('TPIC'),
	kQTFileTypeQuickDrawGXPicture = FOUR_CHAR_CODE('qdgx'),
	kQTFileTypeQuickTimeImage	= FOUR_CHAR_CODE('qtif'),
	kQTFileType3DMF				= FOUR_CHAR_CODE('3DMF')
};

/* QTAtomTypes for atoms in import/export settings containers*/

enum {
	kQTSettingsEffect			= FOUR_CHAR_CODE('effe'),		/* Parent atom whose contents are atoms of an effects description*/
	kQTSettingsMIDI				= FOUR_CHAR_CODE('MIDI'),		/* MIDI import related container*/
	kQTSettingsMIDISettingFlags	= FOUR_CHAR_CODE('sttg'),		/* MIDI import settings	(UInt32)*/
	kQTSettingsText				= FOUR_CHAR_CODE('text'),		/* Text related container*/
	kQTSettingsTextDescription	= FOUR_CHAR_CODE('desc'),		/* Text settings (TextDescription record)*/
	kQTSettingsTextSize			= FOUR_CHAR_CODE('size'),		/* Width/height to create (FixedPoint)*/
	kQTSettingsTextSettingFlags	= FOUR_CHAR_CODE('sttg'),		/* Text export settings (UInt32)*/
	kQTSettingsTextTimeFraction	= FOUR_CHAR_CODE('timf'),		/* Movie time fraction for export (UInt32)*/
	kQTSettingsTime				= FOUR_CHAR_CODE('time'),		/* Time related container*/
	kQTSettingsAudioCDTrack		= FOUR_CHAR_CODE('trak'),		/* Audio CD track related container*/
	kQTSettingsAudioCDTrackRateShift = FOUR_CHAR_CODE('rshf')	/* Rate shift to be performed (SInt16)*/
};





struct MovieExportGetDataParams {
	long 							recordSize;

	long 							trackID;

	TimeScale 						sourceTimeScale;
	TimeValue 						requestedTime;
	TimeValue 						actualTime;

	Ptr 							dataPtr;
	long 							dataSize;

	SampleDescriptionHandle 		desc;
	OSType 							descType;
	long 							descSeed;

	long 							requestedSampleCount;
	long 							actualSampleCount;
	TimeValue 						durationPerSample;
	long 							sampleFlags;
};
typedef struct MovieExportGetDataParams	MovieExportGetDataParams;
typedef CALLBACK_API( OSErr , MovieExportGetDataProcPtr )(void *refCon, MovieExportGetDataParams *params);
typedef CALLBACK_API( OSErr , MovieExportGetPropertyProcPtr )(void *refcon, long trackID, OSType propertyType, void *propertyValue);
typedef STACK_UPP_TYPE(MovieExportGetDataProcPtr) 				MovieExportGetDataUPP;
typedef STACK_UPP_TYPE(MovieExportGetPropertyProcPtr) 			MovieExportGetPropertyUPP;
enum { uppSCModalFilterProcInfo = 0x00003FD0 }; 				/* pascal 1_byte Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSCModalHookProcInfo = 0x00003EE0 }; 					/* pascal 2_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes) */
enum { uppMovieExportGetDataProcInfo = 0x000003E0 }; 			/* pascal 2_bytes Func(4_bytes, 4_bytes) */
enum { uppMovieExportGetPropertyProcInfo = 0x00003FE0 }; 		/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewSCModalFilterProc(userRoutine) 						(SCModalFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSCModalFilterProcInfo, GetCurrentArchitecture())
#define NewSCModalHookProc(userRoutine) 						(SCModalHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSCModalHookProcInfo, GetCurrentArchitecture())
#define NewMovieExportGetDataProc(userRoutine) 					(MovieExportGetDataUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMovieExportGetDataProcInfo, GetCurrentArchitecture())
#define NewMovieExportGetPropertyProc(userRoutine) 				(MovieExportGetPropertyUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMovieExportGetPropertyProcInfo, GetCurrentArchitecture())
#define CallSCModalFilterProc(userRoutine, theDialog, theEvent, itemHit, refcon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppSCModalFilterProcInfo, (theDialog), (theEvent), (itemHit), (refcon))
#define CallSCModalHookProc(userRoutine, theDialog, itemHit, params, refcon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppSCModalHookProcInfo, (theDialog), (itemHit), (params), (refcon))
#define CallMovieExportGetDataProc(userRoutine, refCon, params)  CALL_TWO_PARAMETER_UPP((userRoutine), uppMovieExportGetDataProcInfo, (refCon), (params))
#define CallMovieExportGetPropertyProc(userRoutine, refcon, trackID, propertyType, propertyValue)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppMovieExportGetPropertyProcInfo, (refcon), (trackID), (propertyType), (propertyValue))
EXTERN_API( ComponentResult )
MovieImportHandle				(MovieImportComponent 	ci,
								 Handle 				dataH,
								 Movie 					theMovie,
								 Track 					targetTrack,
								 Track *				usedTrack,
								 TimeValue 				atTime,
								 TimeValue *			addedDuration,
								 long 					inFlags,
								 long *					outFlags)							FIVEWORDINLINE(0x2F3C, 0x0020, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportFile					(MovieImportComponent 	ci,
								 const FSSpec *			theFile,
								 Movie 					theMovie,
								 Track 					targetTrack,
								 Track *				usedTrack,
								 TimeValue 				atTime,
								 TimeValue *			addedDuration,
								 long 					inFlags,
								 long *					outFlags)							FIVEWORDINLINE(0x2F3C, 0x0020, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetSampleDuration	(MovieImportComponent 	ci,
								 TimeValue 				duration,
								 TimeScale 				scale)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetSampleDescription	(MovieImportComponent 	ci,
								 SampleDescriptionHandle  desc,
								 OSType 				mediaType)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetMediaFile			(MovieImportComponent 	ci,
								 AliasHandle 			alias)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetDimensions		(MovieImportComponent 	ci,
								 Fixed 					width,
								 Fixed 					height)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetChunkSize			(MovieImportComponent 	ci,
								 long 					chunkSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetProgressProc		(MovieImportComponent 	ci,
								 MovieProgressUPP 		proc,
								 long 					refcon)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0008, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetAuxiliaryData		(MovieImportComponent 	ci,
								 Handle 				data,
								 OSType 				handleType)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0009, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetFromScrap			(MovieImportComponent 	ci,
								 Boolean 				fromScrap)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x000A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportDoUserDialog			(MovieImportComponent 	ci,
								 const FSSpec *			theFile,
								 Handle 				theData,
								 Boolean *				canceled)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x000B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetDuration			(MovieImportComponent 	ci,
								 TimeValue 				duration)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportGetAuxiliaryDataType	(MovieImportComponent 	ci,
								 OSType *				auxType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportValidate				(MovieImportComponent 	ci,
								 const FSSpec *			theFile,
								 Handle 				theData,
								 Boolean *				valid)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x000E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportGetFileType			(MovieImportComponent 	ci,
								 OSType *				fileType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportDataRef				(MovieImportComponent 	ci,
								 Handle 				dataRef,
								 OSType 				dataRefType,
								 Movie 					theMovie,
								 Track 					targetTrack,
								 Track *				usedTrack,
								 TimeValue 				atTime,
								 TimeValue *			addedDuration,
								 long 					inFlags,
								 long *					outFlags)							FIVEWORDINLINE(0x2F3C, 0x0024, 0x0010, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportGetSampleDescription	(MovieImportComponent 	ci,
								 SampleDescriptionHandle * desc,
								 OSType *				mediaType)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0011, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportGetMIMETypeList		(MovieImportComponent 	ci,
								 QTAtomContainer *		mimeInfo)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0012, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetOffsetAndLimit	(MovieImportComponent 	ci,
								 unsigned long 			offset,
								 unsigned long 			limit)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0013, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportGetSettingsAsAtomContainer (MovieImportComponent  ci,
								 QTAtomContainer *		settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0014, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieImportSetSettingsFromAtomContainer (MovieImportComponent  ci,
								 QTAtomContainer 		settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0015, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportToHandle				(MovieExportComponent 	ci,
								 Handle 				dataH,
								 Movie 					theMovie,
								 Track 					onlyThisTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0080, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportToFile				(MovieExportComponent 	ci,
								 const FSSpec *			theFile,
								 Movie 					theMovie,
								 Track 					onlyThisTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0081, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportGetAuxiliaryData		(MovieExportComponent 	ci,
								 Handle 				dataH,
								 OSType *				handleType)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0083, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportSetProgressProc		(MovieExportComponent 	ci,
								 MovieProgressUPP 		proc,
								 long 					refcon)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0084, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportSetSampleDescription	(MovieExportComponent 	ci,
								 SampleDescriptionHandle  desc,
								 OSType 				mediaType)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0085, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportDoUserDialog			(MovieExportComponent 	ci,
								 Movie 					theMovie,
								 Track 					onlyThisTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration,
								 Boolean *				canceled)							FIVEWORDINLINE(0x2F3C, 0x0014, 0x0086, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportGetCreatorType		(MovieExportComponent 	ci,
								 OSType *				creator)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0087, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportToDataRef			(MovieExportComponent 	ci,
								 Handle 				dataRef,
								 OSType 				dataRefType,
								 Movie 					theMovie,
								 Track 					onlyThisTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration)							FIVEWORDINLINE(0x2F3C, 0x0018, 0x0088, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportFromProceduresToDataRef (MovieExportComponent  ci,
								 Handle 				dataRef,
								 OSType 				dataRefType)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0089, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportAddDataSource		(MovieExportComponent 	ci,
								 OSType 				trackType,
								 TimeScale 				scale,
								 long *					trackID,
								 MovieExportGetPropertyUPP  getPropertyProc,
								 MovieExportGetDataUPP 	getDataProc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x0018, 0x008A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportValidate				(MovieExportComponent 	ci,
								 Movie 					theMovie,
								 Track 					onlyThisTrack,
								 Boolean *				valid)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x008B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportGetSettingsAsAtomContainer (MovieExportComponent  ci,
								 QTAtomContainer *		settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportSetSettingsFromAtomContainer (MovieExportComponent  ci,
								 QTAtomContainer 		settings)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportGetFileNameExtension	(MovieExportComponent 	ci,
								 OSType *				extension)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportGetShortFileTypeString (MovieExportComponent  ci,
								 Str255 				typeString)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportGetSourceMediaType	(MovieExportComponent 	ci,
								 OSType *				mediaType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0090, 0x7000, 0xA82A);

/* Text Export Display Info data structure*/

struct TextDisplayData {
	long 							displayFlags;
	long 							textJustification;
	RGBColor 						bgColor;
	Rect 							textBox;

	short 							beginHilite;
	short 							endHilite;
	RGBColor 						hiliteColor;
	Boolean 						doHiliteColor;
	SInt8 							filler;
	TimeValue 						scrollDelayDur;
	Point 							dropShadowOffset;
	short 							dropShadowTransparency;
};
typedef struct TextDisplayData			TextDisplayData;

typedef ComponentInstance 				TextExportComponent;
typedef ComponentInstance 				GraphicImageMovieImportComponent;
EXTERN_API( ComponentResult )
TextExportGetDisplayData		(TextExportComponent 	ci,
								 TextDisplayData *		textDisplay)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextExportGetTimeFraction		(TextExportComponent 	ci,
								 long *					movieTimeFraction)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextExportSetTimeFraction		(TextExportComponent 	ci,
								 long 					movieTimeFraction)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextExportGetSettings			(TextExportComponent 	ci,
								 long *					setting)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextExportSetSettings			(TextExportComponent 	ci,
								 long 					setting)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0104, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
MIDIImportGetSettings			(TextExportComponent 	ci,
								 long *					setting)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MIDIImportSetSettings			(TextExportComponent 	ci,
								 long 					setting)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportNewGetDataAndPropertiesProcs (MovieExportComponent  ci,
								 OSType 				trackType,
								 TimeScale *			scale,
								 Movie 					theMovie,
								 Track 					theTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration,
								 MovieExportGetPropertyUPP * getPropertyProc,
								 MovieExportGetDataUPP * getDataProc,
								 void **				refCon)								FIVEWORDINLINE(0x2F3C, 0x0024, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MovieExportDisposeGetDataAndPropertiesProcs (MovieExportComponent  ci,
								 MovieExportGetPropertyUPP  getPropertyProc,
								 MovieExportGetDataUPP 	getDataProc,
								 void *					refCon)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0101, 0x7000, 0xA82A);


enum {
	movieExportWidth			= FOUR_CHAR_CODE('wdth'),		/* pointer to Fixed*/
	movieExportHeight			= FOUR_CHAR_CODE('hegt'),		/* pointer to Fixed*/
	movieExportDuration			= FOUR_CHAR_CODE('dura'),		/* pointer to TimeRecord*/
	movieExportVideoFilter		= FOUR_CHAR_CODE('iflt')		/* pointer to QTAtomContainer*/
};

EXTERN_API( ComponentResult )
GraphicsImageImportSetSequenceEnabled (GraphicImageMovieImportComponent  ci,
								 Boolean 				enable)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
GraphicsImageImportGetSequenceEnabled (GraphicImageMovieImportComponent  ci,
								 Boolean *				enable)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);








/***************

	File Preview Components

***************/
typedef ComponentInstance 				pnotComponent;

enum {
	pnotComponentWantsEvents	= 1,
	pnotComponentNeedsNoCache	= 2
};


enum {
	ShowFilePreviewComponentType = FOUR_CHAR_CODE('pnot'),
	CreateFilePreviewComponentType = FOUR_CHAR_CODE('pmak')
};

EXTERN_API( ComponentResult )
PreviewShowData					(pnotComponent 			p,
								 OSType 				dataType,
								 Handle 				data,
								 const Rect *			inHere)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
PreviewMakePreview				(pnotComponent 			p,
								 OSType *				previewType,
								 Handle *				previewResult,
								 const FSSpec *			sourceFile,
								 ICMProgressProcRecordPtr  progress)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
PreviewMakePreviewReference		(pnotComponent 			p,
								 OSType *				previewType,
								 short *				resID,
								 const FSSpec *			sourceFile)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
PreviewEvent					(pnotComponent 			p,
								 EventRecord *			e,
								 Boolean *				handledEvent)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0004, 0x7000, 0xA82A);



typedef ComponentInstance 				DataCompressorComponent;
typedef ComponentInstance 				DataDecompressorComponent;
typedef ComponentInstance 				DataCodecComponent;

enum {
	DataCompressorComponentType	= FOUR_CHAR_CODE('dcom'),
	DataDecompressorComponentType = FOUR_CHAR_CODE('ddec'),
	AppleDataCompressorSubType	= FOUR_CHAR_CODE('adec'),
	zlibDataCompressorSubType	= FOUR_CHAR_CODE('zlib')
};


/** These are DataCodec procedures **/
EXTERN_API( ComponentResult )
DataCodecDecompress				(DataCodecComponent 	dc,
								 void *					srcData,
								 UInt32 				srcSize,
								 void *					dstData,
								 UInt32 				dstBufferSize)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataCodecGetCompressBufferSize	(DataCodecComponent 	dc,
								 UInt32 				srcSize,
								 UInt32 *				dstSize)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataCodecCompress				(DataCodecComponent 	dc,
								 void *					srcData,
								 UInt32 				srcSize,
								 void *					dstData,
								 UInt32 				dstBufferSize,
								 UInt32 *				actualDstSize,
								 UInt32 *				decompressSlop)						FIVEWORDINLINE(0x2F3C, 0x0018, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataCodecBeginInterruptSafe		(DataCodecComponent 	dc,
								 unsigned long 			maxSrcSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataCodecEndInterruptSafe		(DataCodecComponent 	dc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataCodecDecompressPartial		(DataCodecComponent 	dc,
								 void **				next_in,
								 unsigned long *		avail_in,
								 unsigned long *		total_in,
								 void **				next_out,
								 unsigned long *		avail_out,
								 unsigned long *		total_out,
								 Boolean *				didFinish)							FIVEWORDINLINE(0x2F3C, 0x001C, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataCodecCompressPartial		(DataCodecComponent 	dc,
								 void **				next_in,
								 unsigned long *		avail_in,
								 unsigned long *		total_in,
								 void **				next_out,
								 unsigned long *		avail_out,
								 unsigned long *		total_out,
								 Boolean 				tryToFinish,
								 Boolean *				didFinish)							FIVEWORDINLINE(0x2F3C, 0x001E, 0x0007, 0x7000, 0xA82A);




typedef CALLBACK_API( void , DataHCompletionProcPtr )(Ptr request, long refcon, OSErr err);
typedef STACK_UPP_TYPE(DataHCompletionProcPtr) 					DataHCompletionUPP;


enum {
	kDataHCanRead				= 1L << 0,
	kDataHSpecialRead			= 1L << 1,
	kDataHSpecialReadFile		= 1L << 2,
	kDataHCanWrite				= 1L << 3,
	kDataHSpecialWrite			= 1 << 4,
	kDataHSpecialWriteFile		= 1 << 5,
	kDataHCanStreamingWrite		= 1 << 6,
	kDataHMustCheckDataRef		= 1 << 7
};


struct DataHVolumeListRecord {
	short 							vRefNum;
	long 							flags;
};
typedef struct DataHVolumeListRecord	DataHVolumeListRecord;

typedef DataHVolumeListRecord *			DataHVolumeListPtr;
typedef DataHVolumeListPtr *			DataHVolumeList;

enum {
	kDataHExtendedSchedule		= FOUR_CHAR_CODE('xtnd')
};


struct DataHScheduleRecord {
	TimeRecord 						timeNeededBy;
	long 							extendedID;					/* always is kDataHExtendedSchedule*/
	long 							extendedVers;				/* always set to 0*/
	Fixed 							priority;					/* 100.0 or more means must have. lower numbersÉ*/
};
typedef struct DataHScheduleRecord		DataHScheduleRecord;

typedef DataHScheduleRecord *			DataHSchedulePtr;


EXTERN_API( ComponentResult )
DataHGetData					(DataHandler 			dh,
								 Handle 				h,
								 long 					hOffset,
								 long 					offset,
								 long 					size)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHPutData					(DataHandler 			dh,
								 Handle 				h,
								 long 					hOffset,
								 long *					offset,
								 long 					size)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHFlushData					(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHOpenForWrite				(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHCloseForWrite				(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0006, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
DataHOpenForRead				(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0008, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHCloseForRead				(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0009, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHSetDataRef					(DataHandler 			dh,
								 Handle 				dataRef)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetDataRef					(DataHandler 			dh,
								 Handle *				dataRef)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHCompareDataRef				(DataHandler 			dh,
								 Handle 				dataRef,
								 Boolean *				equal)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x000C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHTask						(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHScheduleData				(DataHandler 			dh,
								 Ptr 					PlaceToPutDataPtr,
								 long 					FileOffset,
								 long 					DataSize,
								 long 					RefCon,
								 DataHSchedulePtr 		scheduleRec,
								 DataHCompletionUPP 	CompletionRtn)						FIVEWORDINLINE(0x2F3C, 0x0018, 0x000E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHFinishData					(DataHandler 			dh,
								 Ptr 					PlaceToPutDataPtr,
								 Boolean 				Cancel)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x000F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHFlushCache					(DataHandler 			dh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0010, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHResolveDataRef				(DataHandler 			dh,
								 Handle 				theDataRef,
								 Boolean *				wasChanged,
								 Boolean 				userInterfaceAllowed)				FIVEWORDINLINE(0x2F3C, 0x000A, 0x0011, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetFileSize				(DataHandler 			dh,
								 long *					fileSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0012, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHCanUseDataRef				(DataHandler 			dh,
								 Handle 				dataRef,
								 long *					useFlags)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0013, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetVolumeList				(DataHandler 			dh,
								 DataHVolumeList *		volumeList)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0014, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHWrite						(DataHandler 			dh,
								 Ptr 					data,
								 long 					offset,
								 long 					size,
								 DataHCompletionUPP 	completion,
								 long 					refCon)								FIVEWORDINLINE(0x2F3C, 0x0014, 0x0015, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHPreextend					(DataHandler 			dh,
								 unsigned long 			maxToAdd,
								 unsigned long *		spaceAdded)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0016, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHSetFileSize				(DataHandler 			dh,
								 long 					fileSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0017, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetFreeSpace				(DataHandler 			dh,
								 unsigned long *		freeSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0018, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHCreateFile					(DataHandler 			dh,
								 OSType 				creator,
								 Boolean 				deleteExisting)						FIVEWORDINLINE(0x2F3C, 0x0006, 0x0019, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetPreferredBlockSize		(DataHandler 			dh,
								 long *					blockSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetDeviceIndex				(DataHandler 			dh,
								 long *					deviceIndex)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x001B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHIsStreamingDataHandler		(DataHandler 			dh,
								 Boolean *				yes)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetDataInBuffer			(DataHandler 			dh,
								 long 					startOffset,
								 long *					size)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x001D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetScheduleAheadTime		(DataHandler 			dh,
								 long *					millisecs)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHSetCacheSizeLimit			(DataHandler 			dh,
								 Size 					cacheSizeLimit)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x001F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetCacheSizeLimit			(DataHandler 			dh,
								 Size *					cacheSizeLimit)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0020, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetMovie					(DataHandler 			dh,
								 Movie *				theMovie,
								 short *				id)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0021, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHAddMovie					(DataHandler 			dh,
								 Movie 					theMovie,
								 short *				id)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0022, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHUpdateMovie				(DataHandler 			dh,
								 Movie 					theMovie,
								 short 					id)									FIVEWORDINLINE(0x2F3C, 0x0006, 0x0023, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHDoesBuffer					(DataHandler 			dh,
								 Boolean *				buffersReads,
								 Boolean *				buffersWrites)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0024, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetFileName				(DataHandler 			dh,
								 Str255 				str)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0025, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetAvailableFileSize		(DataHandler 			dh,
								 long *					fileSize)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0026, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetMacOSFileType			(DataHandler 			dh,
								 OSType *				fileType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0027, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetMIMEType				(DataHandler 			dh,
								 Str255 				mimeType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0028, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHSetDataRefWithAnchor		(DataHandler 			dh,
								 Handle 				anchorDataRef,
								 OSType 				dataRefType,
								 Handle 				dataRef)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0029, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHGetDataRefWithAnchor		(DataHandler 			dh,
								 Handle 				anchorDataRef,
								 OSType 				dataRefType,
								 Handle *				dataRef)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x002A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHSetMacOSFileType			(DataHandler 			dh,
								 OSType 				fileType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x002B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
DataHSetTimeBase				(DataHandler 			dh,
								 TimeBase 				tb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x002C, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
DataHPlaybackHints				(DataHandler 			dh,
								 long 					flags,
								 unsigned long 			minFileOffset,
								 unsigned long 			maxFileOffset,
								 long 					bytesPerSecond)						FIVEWORDINLINE(0x2F3C, 0x0010, 0x0103, 0x7000, 0xA82A);




/* Standard type for video digitizers */

enum {
	videoDigitizerComponentType	= FOUR_CHAR_CODE('vdig'),
	vdigInterfaceRev			= 2
};

/* Input Format Standards */

enum {
	ntscIn						= 0,							/* current input format */
	currentIn					= 0,							/* ntsc input format */
	palIn						= 1,							/* pal input format */
	secamIn						= 2,							/* secam input format */
	ntscReallyIn				= 3								/* ntsc input format */
};

/* Input Formats */

enum {
	compositeIn					= 0,							/* input is composite format */
	sVideoIn					= 1,							/* input is sVideo format */
	rgbComponentIn				= 2,							/* input is rgb component format */
	rgbComponentSyncIn			= 3,							/* input is rgb component format (sync on green?)*/
	yuvComponentIn				= 4,							/* input is yuv component format */
	yuvComponentSyncIn			= 5,							/* input is yuv component format (sync on green?) */
	tvTunerIn					= 6,
	sdiIn						= 7
};


/* Video Digitizer PlayThru States */

enum {
	vdPlayThruOff				= 0,
	vdPlayThruOn				= 1
};

/* Input Color Space Modes */

enum {
	vdDigitizerBW				= 0,							/* black and white */
	vdDigitizerRGB				= 1								/* rgb color */
};

/* Phase Lock Loop Modes */

enum {
	vdBroadcastMode				= 0,							/* Broadcast / Laser Disk video mode */
	vdVTRMode					= 1								/* VCR / Magnetic media mode */
};

/* Field Select Options */

enum {
	vdUseAnyField				= 0,							/* Digitizers choice on field use */
	vdUseOddField				= 1,							/* Use odd field for half size vert and smaller */
	vdUseEvenField				= 2								/* Use even field for half size vert and smaller */
};

/* vdig types */

enum {
	vdTypeBasic					= 0,							/* basic, no clipping */
	vdTypeAlpha					= 1,							/* supports clipping with alpha channel */
	vdTypeMask					= 2,							/* supports clipping with mask plane */
	vdTypeKey					= 3								/* supports clipping with key color(s) */
};



/* Digitizer Input Capability/Current Flags	*/

enum {
	digiInDoesNTSC				= 1L << 0,						/* digitizer supports NTSC input format */
	digiInDoesPAL				= 1L << 1,						/* digitizer supports PAL input format */
	digiInDoesSECAM				= 1L << 2,						/* digitizer supports SECAM input format */
	digiInDoesGenLock			= 1L << 7,						/* digitizer does genlock */
	digiInDoesComposite			= 1L << 8,						/* digitizer supports composite input type */
	digiInDoesSVideo			= 1L << 9,						/* digitizer supports S-Video input type */
	digiInDoesComponent			= 1L << 10,						/* digitizer supports component = rgb, input type */
	digiInVTR_Broadcast			= 1L << 11,						/* digitizer can differentiate between the two */
	digiInDoesColor				= 1L << 12,						/* digitizer supports color */
	digiInDoesBW				= 1L << 13,						/* digitizer supports black & white */
																/* Digitizer Input Current Flags = these are valid only during active operating conditions,	*/
	digiInSignalLock			= 1L << 31						/* digitizer detects input signal is locked, this bit = horiz lock || vertical lock */
};


/* Digitizer Output Capability/Current Flags */

enum {
	digiOutDoes1				= 1L << 0,						/* digitizer supports 1 bit pixels */
	digiOutDoes2				= 1L << 1,						/* digitizer supports 2 bit pixels */
	digiOutDoes4				= 1L << 2,						/* digitizer supports 4 bit pixels */
	digiOutDoes8				= 1L << 3,						/* digitizer supports 8 bit pixels */
	digiOutDoes16				= 1L << 4,						/* digitizer supports 16 bit pixels */
	digiOutDoes32				= 1L << 5,						/* digitizer supports 32 bit pixels */
	digiOutDoesDither			= 1L << 6,						/* digitizer dithers in indexed modes */
	digiOutDoesStretch			= 1L << 7,						/* digitizer can arbitrarily stretch */
	digiOutDoesShrink			= 1L << 8,						/* digitizer can arbitrarily shrink */
	digiOutDoesMask				= 1L << 9,						/* digitizer can mask to clipping regions */
	digiOutDoesDouble			= 1L << 11,						/* digitizer can stretch to exactly double size */
	digiOutDoesQuad				= 1L << 12,						/* digitizer can stretch exactly quadruple size */
	digiOutDoesQuarter			= 1L << 13,						/* digitizer can shrink to exactly quarter size */
	digiOutDoesSixteenth		= 1L << 14,						/* digitizer can shrink to exactly sixteenth size */
	digiOutDoesRotate			= 1L << 15,						/* digitizer supports rotate transformations */
	digiOutDoesHorizFlip		= 1L << 16,						/* digitizer supports horizontal flips Sx < 0 */
	digiOutDoesVertFlip			= 1L << 17,						/* digitizer supports vertical flips Sy < 0 */
	digiOutDoesSkew				= 1L << 18,						/* digitizer supports skew = shear,twist, */
	digiOutDoesBlend			= 1L << 19,
	digiOutDoesWarp				= 1L << 20,
	digiOutDoesHW_DMA			= 1L << 21,						/* digitizer not constrained to local device */
	digiOutDoesHWPlayThru		= 1L << 22,						/* digitizer doesn't need time to play thru */
	digiOutDoesILUT				= 1L << 23,						/* digitizer does inverse LUT for index modes */
	digiOutDoesKeyColor			= 1L << 24,						/* digitizer does key color functions too */
	digiOutDoesAsyncGrabs		= 1L << 25,						/* digitizer supports async grabs */
	digiOutDoesUnreadableScreenBits = 1L << 26,					/* playthru doesn't generate readable bits on screen*/
	digiOutDoesCompress			= 1L << 27,						/* supports alternate output data types */
	digiOutDoesCompressOnly		= 1L << 28,						/* can't provide raw frames anywhere */
	digiOutDoesPlayThruDuringCompress = 1L << 29,				/* digi can do playthru while providing compressed data */
	digiOutDoesCompressPartiallyVisible = 1L << 30				/* digi doesn't need all bits visible on screen to do hardware compress */
};

/* Types */
typedef ComponentInstance 				VideoDigitizerComponent;
typedef ComponentResult 				VideoDigitizerError;

struct DigitizerInfo {
	short 							vdigType;
	long 							inputCapabilityFlags;
	long 							outputCapabilityFlags;
	long 							inputCurrentFlags;
	long 							outputCurrentFlags;
	short 							slot;						/* temporary for connection purposes */
	GDHandle 						gdh;						/* temporary for digitizers that have preferred screen */
	GDHandle 						maskgdh;					/* temporary for digitizers that have mask planes */
	short 							minDestHeight;				/* Smallest resizable height */
	short 							minDestWidth;				/* Smallest resizable width */
	short 							maxDestHeight;				/* Largest resizable height */
	short 							maxDestWidth;				/* Largest resizable height */
	short 							blendLevels;				/* Number of blend levels supported (2 if 1 bit mask) */
	long 							reserved;					/* reserved */
};
typedef struct DigitizerInfo			DigitizerInfo;

struct VdigType {
	long 							digType;
	long 							reserved;
};
typedef struct VdigType					VdigType;

struct VdigTypeList {
	short 							count;
	VdigType 						list[1];
};
typedef struct VdigTypeList				VdigTypeList;

struct VdigBufferRec {
	PixMapHandle 					dest;
	Point 							location;
	long 							reserved;
};
typedef struct VdigBufferRec			VdigBufferRec;

struct VdigBufferRecList {
	short 							count;
	MatrixRecordPtr 				matrix;
	RgnHandle 						mask;
	VdigBufferRec 					list[1];
};
typedef struct VdigBufferRecList		VdigBufferRecList;
typedef VdigBufferRecList *				VdigBufferRecListPtr;
typedef VdigBufferRecListPtr *			VdigBufferRecListHandle;
typedef CALLBACK_API( void , VdigIntProcPtr )(long flags, long refcon);
typedef STACK_UPP_TYPE(VdigIntProcPtr) 							VdigIntUPP;

struct VDCompressionList {
	CodecComponent 					codec;
	CodecType 						cType;
	Str63 							typeName;
	Str63 							name;
	long 							formatFlags;
	long 							compressFlags;
	long 							reserved;
};
typedef struct VDCompressionList		VDCompressionList;
typedef VDCompressionList *				VDCompressionListPtr;
typedef VDCompressionListPtr *			VDCompressionListHandle;

enum {
	dmaDepth1					= 1,
	dmaDepth2					= 2,
	dmaDepth4					= 4,
	dmaDepth8					= 8,
	dmaDepth16					= 16,
	dmaDepth32					= 32,
	dmaDepth2Gray				= 64,
	dmaDepth4Gray				= 128,
	dmaDepth8Gray				= 256
};


enum {
	kVDIGControlledFrameRate	= -1
};


EXTERN_API( VideoDigitizerError )
VDGetMaxSrcRect					(VideoDigitizerComponent  ci,
								 short 					inputStd,
								 Rect *					maxSrcRect)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x0001, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetActiveSrcRect				(VideoDigitizerComponent  ci,
								 short 					inputStd,
								 Rect *					activeSrcRect)						FIVEWORDINLINE(0x2F3C, 0x0006, 0x0002, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetDigitizerRect				(VideoDigitizerComponent  ci,
								 Rect *					digitizerRect)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0003, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetDigitizerRect				(VideoDigitizerComponent  ci,
								 Rect *					digitizerRect)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0004, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetVBlankRect					(VideoDigitizerComponent  ci,
								 short 					inputStd,
								 Rect *					vBlankRect)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x0005, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetMaskPixMap					(VideoDigitizerComponent  ci,
								 PixMapHandle 			maskPixMap)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0006, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetPlayThruDestination		(VideoDigitizerComponent  ci,
								 PixMapHandle *			dest,
								 Rect *					destRect,
								 MatrixRecord *			m,
								 RgnHandle *			mask)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0008, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDUseThisCLUT					(VideoDigitizerComponent  ci,
								 CTabHandle 			colorTableHandle)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0009, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetInputGammaValue			(VideoDigitizerComponent  ci,
								 Fixed 					channel1,
								 Fixed 					channel2,
								 Fixed 					channel3)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x000A, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetInputGammaValue			(VideoDigitizerComponent  ci,
								 Fixed *				channel1,
								 Fixed *				channel2,
								 Fixed *				channel3)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x000B, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetBrightness					(VideoDigitizerComponent  ci,
								 unsigned short *		brightness)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000C, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetBrightness					(VideoDigitizerComponent  ci,
								 unsigned short *		brightness)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000D, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetContrast					(VideoDigitizerComponent  ci,
								 unsigned short *		contrast)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x000E, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetHue						(VideoDigitizerComponent  ci,
								 unsigned short *		hue)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000F, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetSharpness					(VideoDigitizerComponent  ci,
								 unsigned short *		sharpness)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0010, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetSaturation					(VideoDigitizerComponent  ci,
								 unsigned short *		saturation)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0011, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetContrast					(VideoDigitizerComponent  ci,
								 unsigned short *		contrast)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0012, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetHue						(VideoDigitizerComponent  ci,
								 unsigned short *		hue)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0013, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetSharpness					(VideoDigitizerComponent  ci,
								 unsigned short *		sharpness)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0014, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetSaturation					(VideoDigitizerComponent  ci,
								 unsigned short *		saturation)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0015, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGrabOneFrame					(VideoDigitizerComponent  ci)								FIVEWORDINLINE(0x2F3C, 0x0000, 0x0016, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetMaxAuxBuffer				(VideoDigitizerComponent  ci,
								 PixMapHandle *			pm,
								 Rect *					r)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0017, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetDigitizerInfo				(VideoDigitizerComponent  ci,
								 DigitizerInfo *		info)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0019, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetCurrentFlags				(VideoDigitizerComponent  ci,
								 long *					inputCurrentFlag,
								 long *					outputCurrentFlag)					FIVEWORDINLINE(0x2F3C, 0x0008, 0x001A, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetKeyColor					(VideoDigitizerComponent  ci,
								 long 					index)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001B, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetKeyColor					(VideoDigitizerComponent  ci,
								 long *					index)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001C, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDAddKeyColor					(VideoDigitizerComponent  ci,
								 long *					index)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001D, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetNextKeyColor				(VideoDigitizerComponent  ci,
								 long 					index)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001E, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetKeyColorRange				(VideoDigitizerComponent  ci,
								 RGBColor *				minRGB,
								 RGBColor *				maxRGB)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x001F, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetKeyColorRange				(VideoDigitizerComponent  ci,
								 RGBColor *				minRGB,
								 RGBColor *				maxRGB)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0020, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetDigitizerUserInterrupt		(VideoDigitizerComponent  ci,
								 long 					flags,
								 VdigIntUPP 			userInterruptProc,
								 long 					refcon)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0021, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetInputColorSpaceMode		(VideoDigitizerComponent  ci,
								 short 					colorSpaceMode)						FIVEWORDINLINE(0x2F3C, 0x0002, 0x0022, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetInputColorSpaceMode		(VideoDigitizerComponent  ci,
								 short *				colorSpaceMode)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0023, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetClipState					(VideoDigitizerComponent  ci,
								 short 					clipEnable)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x0024, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetClipState					(VideoDigitizerComponent  ci,
								 short *				clipEnable)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0025, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetClipRgn					(VideoDigitizerComponent  ci,
								 RgnHandle 				clipRegion)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0026, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDClearClipRgn					(VideoDigitizerComponent  ci,
								 RgnHandle 				clipRegion)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0027, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetCLUTInUse					(VideoDigitizerComponent  ci,
								 CTabHandle *			colorTableHandle)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0028, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetPLLFilterType				(VideoDigitizerComponent  ci,
								 short 					pllType)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x0029, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetPLLFilterType				(VideoDigitizerComponent  ci,
								 short *				pllType)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x002A, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetMaskandValue				(VideoDigitizerComponent  ci,
								 unsigned short 		blendLevel,
								 long *					mask,
								 long *					value)								FIVEWORDINLINE(0x2F3C, 0x000A, 0x002B, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetMasterBlendLevel			(VideoDigitizerComponent  ci,
								 unsigned short *		blendLevel)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x002C, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetPlayThruDestination		(VideoDigitizerComponent  ci,
								 PixMapHandle 			dest,
								 RectPtr 				destRect,
								 MatrixRecordPtr 		m,
								 RgnHandle 				mask)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x002D, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetPlayThruOnOff				(VideoDigitizerComponent  ci,
								 short 					state)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x002E, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetFieldPreference			(VideoDigitizerComponent  ci,
								 short 					fieldFlag)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x002F, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetFieldPreference			(VideoDigitizerComponent  ci,
								 short *				fieldFlag)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0030, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDPreflightDestination			(VideoDigitizerComponent  ci,
								 Rect *					digitizerRect,
								 PixMap **				dest,
								 RectPtr 				destRect,
								 MatrixRecordPtr 		m)									FIVEWORDINLINE(0x2F3C, 0x0010, 0x0032, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDPreflightGlobalRect			(VideoDigitizerComponent  ci,
								 GrafPtr 				theWindow,
								 Rect *					globalRect)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0033, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetPlayThruGlobalRect			(VideoDigitizerComponent  ci,
								 GrafPtr 				theWindow,
								 Rect *					globalRect)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0034, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetInputGammaRecord			(VideoDigitizerComponent  ci,
								 VDGamRecPtr 			inputGammaPtr)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0035, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetInputGammaRecord			(VideoDigitizerComponent  ci,
								 VDGamRecPtr *			inputGammaPtr)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0036, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetBlackLevelValue			(VideoDigitizerComponent  ci,
								 unsigned short *		blackLevel)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0037, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetBlackLevelValue			(VideoDigitizerComponent  ci,
								 unsigned short *		blackLevel)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0038, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetWhiteLevelValue			(VideoDigitizerComponent  ci,
								 unsigned short *		whiteLevel)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0039, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetWhiteLevelValue			(VideoDigitizerComponent  ci,
								 unsigned short *		whiteLevel)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x003A, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetVideoDefaults				(VideoDigitizerComponent  ci,
								 unsigned short *		blackLevel,
								 unsigned short *		whiteLevel,
								 unsigned short *		brightness,
								 unsigned short *		hue,
								 unsigned short *		saturation,
								 unsigned short *		contrast,
								 unsigned short *		sharpness)							FIVEWORDINLINE(0x2F3C, 0x001C, 0x003B, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetNumberOfInputs				(VideoDigitizerComponent  ci,
								 short *				inputs)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x003C, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetInputFormat				(VideoDigitizerComponent  ci,
								 short 					input,
								 short *				format)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x003D, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetInput						(VideoDigitizerComponent  ci,
								 short 					input)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x003E, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetInput						(VideoDigitizerComponent  ci,
								 short *				input)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x003F, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetInputStandard				(VideoDigitizerComponent  ci,
								 short 					inputStandard)						FIVEWORDINLINE(0x2F3C, 0x0002, 0x0040, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetupBuffers					(VideoDigitizerComponent  ci,
								 VdigBufferRecListHandle  bufferList)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0041, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGrabOneFrameAsync				(VideoDigitizerComponent  ci,
								 short 					buffer)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0042, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDDone							(VideoDigitizerComponent  ci,
								 short 					buffer)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0043, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetCompression				(VideoDigitizerComponent  ci,
								 OSType 				compressType,
								 short 					depth,
								 Rect *					bounds,
								 CodecQ 				spatialQuality,
								 CodecQ 				temporalQuality,
								 long 					keyFrameRate)						FIVEWORDINLINE(0x2F3C, 0x0016, 0x0044, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDCompressOneFrameAsync			(VideoDigitizerComponent  ci)								FIVEWORDINLINE(0x2F3C, 0x0000, 0x0045, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDCompressDone					(VideoDigitizerComponent  ci,
								 Boolean *				done,
								 Ptr *					theData,
								 long *					dataSize,
								 UInt8 *				similarity,
								 TimeRecord *			t)									FIVEWORDINLINE(0x2F3C, 0x0014, 0x0046, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDReleaseCompressBuffer			(VideoDigitizerComponent  ci,
								 Ptr 					bufferAddr)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0047, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetImageDescription			(VideoDigitizerComponent  ci,
								 ImageDescriptionHandle  desc)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0048, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDResetCompressSequence			(VideoDigitizerComponent  ci)								FIVEWORDINLINE(0x2F3C, 0x0000, 0x0049, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetCompressionOnOff			(VideoDigitizerComponent  ci,
								 Boolean 				state)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x004A, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetCompressionTypes			(VideoDigitizerComponent  ci,
								 VDCompressionListHandle  h)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x004B, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetTimeBase					(VideoDigitizerComponent  ci,
								 TimeBase 				t)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x004C, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetFrameRate					(VideoDigitizerComponent  ci,
								 Fixed 					framesPerSecond)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x004D, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetDataRate					(VideoDigitizerComponent  ci,
								 long *					milliSecPerFrame,
								 Fixed *				framesPerSecond,
								 long *					bytesPerSecond)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x004E, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetSoundInputDriver			(VideoDigitizerComponent  ci,
								 Str255 				soundDriverName)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x004F, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetDMADepths					(VideoDigitizerComponent  ci,
								 long *					depthArray,
								 long *					preferredDepth)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0050, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetPreferredTimeScale			(VideoDigitizerComponent  ci,
								 TimeScale *			preferred)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0051, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDReleaseAsyncBuffers			(VideoDigitizerComponent  ci)								FIVEWORDINLINE(0x2F3C, 0x0000, 0x0052, 0x7000, 0xA82A);

/* 83 is reserved for compatibility reasons */
EXTERN_API( VideoDigitizerError )
VDSetDataRate					(VideoDigitizerComponent  ci,
								 long 					bytesPerSecond)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0054, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetTimeCode					(VideoDigitizerComponent  ci,
								 TimeRecord *			atTime,
								 void *					timeCodeFormat,
								 void *					timeCodeTime)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0055, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDUseSafeBuffers				(VideoDigitizerComponent  ci,
								 Boolean 				useSafeBuffers)						FIVEWORDINLINE(0x2F3C, 0x0002, 0x0056, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetSoundInputSource			(VideoDigitizerComponent  ci,
								 long 					videoInput,
								 long *					soundInput)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0057, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetCompressionTime			(VideoDigitizerComponent  ci,
								 OSType 				compressionType,
								 short 					depth,
								 Rect *					srcRect,
								 CodecQ *				spatialQuality,
								 CodecQ *				temporalQuality,
								 unsigned long *		compressTime)						FIVEWORDINLINE(0x2F3C, 0x0016, 0x0058, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetPreferredPacketSize		(VideoDigitizerComponent  ci,
								 long 					preferredPacketSizeInBytes)			FIVEWORDINLINE(0x2F3C, 0x0004, 0x0059, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetPreferredImageDimensions	(VideoDigitizerComponent  ci,
								 long 					width,
								 long 					height)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x005A, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetPreferredImageDimensions	(VideoDigitizerComponent  ci,
								 long *					width,
								 long *					height)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x005B, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDGetInputName					(VideoDigitizerComponent  ci,
								 long 					videoInput,
								 Str255 				name)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x005C, 0x7000, 0xA82A);

EXTERN_API( VideoDigitizerError )
VDSetDestinationPort			(VideoDigitizerComponent  ci,
								 CGrafPtr 				destPort)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x005D, 0x7000, 0xA82A);



/*
	General Sequence Grab stuff
*/

typedef ComponentInstance 				SeqGrabComponent;
typedef ComponentInstance 				SGChannel;

enum {
	SeqGrabComponentType		= FOUR_CHAR_CODE('barg'),
	SeqGrabChannelType			= FOUR_CHAR_CODE('sgch'),
	SeqGrabPanelType			= FOUR_CHAR_CODE('sgpn'),
	SeqGrabCompressionPanelType	= FOUR_CHAR_CODE('cmpr'),
	SeqGrabSourcePanelType		= FOUR_CHAR_CODE('sour')
};


enum {
	seqGrabToDisk				= 1,
	seqGrabToMemory				= 2,
	seqGrabDontUseTempMemory	= 4,
	seqGrabAppendToFile			= 8,
	seqGrabDontAddMovieResource	= 16,
	seqGrabDontMakeMovie		= 32,
	seqGrabPreExtendFile		= 64,
	seqGrabDataProcIsInterruptSafe = 128,
	seqGrabDataProcDoesOverlappingReads = 256
};

typedef unsigned long 					SeqGrabDataOutputEnum;

enum {
	seqGrabRecord				= 1,
	seqGrabPreview				= 2,
	seqGrabPlayDuringRecord		= 4
};

typedef unsigned long 					SeqGrabUsageEnum;

enum {
	seqGrabHasBounds			= 1,
	seqGrabHasVolume			= 2,
	seqGrabHasDiscreteSamples	= 4
};

typedef unsigned long 					SeqGrabChannelInfoEnum;

struct SGOutputRecord {
	long 							data[1];
};
typedef struct SGOutputRecord			SGOutputRecord;

typedef SGOutputRecord *				SGOutput;

struct SeqGrabFrameInfo {
	long 							frameOffset;
	long 							frameTime;
	long 							frameSize;
	SGChannel 						frameChannel;
	long 							frameRefCon;
};
typedef struct SeqGrabFrameInfo			SeqGrabFrameInfo;
typedef SeqGrabFrameInfo *				SeqGrabFrameInfoPtr;

struct SeqGrabExtendedFrameInfo {
	wide 							frameOffset;
	long 							frameTime;
	long 							frameSize;
	SGChannel 						frameChannel;
	long 							frameRefCon;
	SGOutput 						frameOutput;
};
typedef struct SeqGrabExtendedFrameInfo	SeqGrabExtendedFrameInfo;
typedef SeqGrabExtendedFrameInfo *		SeqGrabExtendedFrameInfoPtr;

enum {
	grabPictOffScreen			= 1,
	grabPictIgnoreClip			= 2,
	grabPictCurrentImage		= 4
};


enum {
	sgFlagControlledGrab		= (1 << 0)
};

typedef CALLBACK_API( OSErr , SGDataProcPtr )(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);
typedef STACK_UPP_TYPE(SGDataProcPtr) 							SGDataUPP;

struct SGDeviceName {
	Str63 							name;
	Handle 							icon;
	long 							flags;
	long 							refCon;
	long 							reserved;					/* zero*/
};
typedef struct SGDeviceName				SGDeviceName;

enum {
	sgDeviceNameFlagDeviceUnavailable = (1 << 0)
};


struct SGDeviceListRecord {
	short 							count;
	short 							selectedIndex;
	long 							reserved;					/* zero*/
	SGDeviceName 					entry[1];
};
typedef struct SGDeviceListRecord		SGDeviceListRecord;

typedef SGDeviceListRecord *			SGDeviceListPtr;
typedef SGDeviceListPtr *				SGDeviceList;

enum {
	sgDeviceListWithIcons		= (1 << 0),
	sgDeviceListDontCheckAvailability = (1 << 1)
};


enum {
	seqGrabWriteAppend			= 0,
	seqGrabWriteReserve			= 1,
	seqGrabWriteFill			= 2
};


enum {
	seqGrabUnpause				= 0,
	seqGrabPause				= 1,
	seqGrabPauseForMenu			= 3
};


enum {
	channelFlagDontOpenResFile	= 2,
	channelFlagHasDependency	= 4
};

typedef CALLBACK_API( Boolean , SGModalFilterProcPtr )(DialogPtr theDialog, const EventRecord *theEvent, short *itemHit, long refCon);
typedef STACK_UPP_TYPE(SGModalFilterProcPtr) 					SGModalFilterUPP;

enum {
	sgPanelFlagForPanel			= 1
};


enum {
	seqGrabSettingsPreviewOnly	= 1
};


enum {
	channelPlayNormal			= 0,
	channelPlayFast				= 1,
	channelPlayHighQuality		= 2,
	channelPlayAllData			= 4
};


EXTERN_API( ComponentResult )
SGInitialize					(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetDataOutput					(SeqGrabComponent 		s,
								 const FSSpec *			movieFile,
								 long 					whereFlags)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetDataOutput					(SeqGrabComponent 		s,
								 FSSpec *				movieFile,
								 long *					whereFlags)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetGWorld						(SeqGrabComponent 		s,
								 CGrafPtr 				gp,
								 GDHandle 				gd)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetGWorld						(SeqGrabComponent 		s,
								 CGrafPtr *				gp,
								 GDHandle *				gd)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGNewChannel					(SeqGrabComponent 		s,
								 OSType 				channelType,
								 SGChannel *			ref)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGDisposeChannel				(SeqGrabComponent 		s,
								 SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGStartPreview					(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0010, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGStartRecord					(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0011, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGIdle							(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0012, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGStop							(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0013, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPause							(SeqGrabComponent 		s,
								 Byte 					pause)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0014, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPrepare						(SeqGrabComponent 		s,
								 Boolean 				prepareForPreview,
								 Boolean 				prepareForRecord)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0015, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGRelease						(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0016, 0x7000, 0xA82A);

EXTERN_API( Movie )
SGGetMovie						(SeqGrabComponent 		s)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0017, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetMaximumRecordTime			(SeqGrabComponent 		s,
								 unsigned long 			ticks)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0018, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetMaximumRecordTime			(SeqGrabComponent 		s,
								 unsigned long *		ticks)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0019, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetStorageSpaceRemaining		(SeqGrabComponent 		s,
								 unsigned long *		bytes)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetTimeRemaining				(SeqGrabComponent 		s,
								 long *					ticksLeft)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGrabPict						(SeqGrabComponent 		s,
								 PicHandle *			p,
								 const Rect *			bounds,
								 short 					offscreenDepth,
								 long 					grabPictFlags)						FIVEWORDINLINE(0x2F3C, 0x000E, 0x001C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetLastMovieResID				(SeqGrabComponent 		s,
								 short *				resID)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x001D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetFlags						(SeqGrabComponent 		s,
								 long 					sgFlags)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetFlags						(SeqGrabComponent 		s,
								 long *					sgFlags)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x001F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetDataProc					(SeqGrabComponent 		s,
								 SGDataUPP 				proc,
								 long 					refCon)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0020, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGNewChannelFromComponent		(SeqGrabComponent 		s,
								 SGChannel *			newChannel,
								 Component 				sgChannelComponent)					FIVEWORDINLINE(0x2F3C, 0x0008, 0x0021, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGDisposeDeviceList				(SeqGrabComponent 		s,
								 SGDeviceList 			list)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0022, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAppendDeviceListToMenu		(SeqGrabComponent 		s,
								 SGDeviceList 			list,
								 MenuHandle 			mh)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0023, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetSettings					(SeqGrabComponent 		s,
								 UserData 				ud,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0024, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetSettings					(SeqGrabComponent 		s,
								 UserData *				ud,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0025, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetIndChannel					(SeqGrabComponent 		s,
								 short 					index,
								 SGChannel *			ref,
								 OSType *				chanType)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x0026, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGUpdate						(SeqGrabComponent 		s,
								 RgnHandle 				updateRgn)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0027, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetPause						(SeqGrabComponent 		s,
								 Byte *					paused)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0028, 0x7000, 0xA82A);

typedef const Component *				ConstComponentListPtr;
EXTERN_API( ComponentResult )
SGSettingsDialog				(SeqGrabComponent 		s,
								 SGChannel 				c,
								 short 					numPanels,
								 ConstComponentListPtr 	panelList,
								 long 					flags,
								 SGModalFilterUPP 		proc,
								 long 					procRefNum)							FIVEWORDINLINE(0x2F3C, 0x0016, 0x0029, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetAlignmentProc				(SeqGrabComponent 		s,
								 ICMAlignmentProcRecordPtr  alignmentProc)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x002A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelSettings			(SeqGrabComponent 		s,
								 SGChannel 				c,
								 UserData 				ud,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x002B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelSettings			(SeqGrabComponent 		s,
								 SGChannel 				c,
								 UserData *				ud,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x002C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetMode						(SeqGrabComponent 		s,
								 Boolean *				previewMode,
								 Boolean *				recordMode)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x002D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetDataRef					(SeqGrabComponent 		s,
								 Handle 				dataRef,
								 OSType 				dataRefType,
								 long 					whereFlags)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x002E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetDataRef					(SeqGrabComponent 		s,
								 Handle *				dataRef,
								 OSType *				dataRefType,
								 long *					whereFlags)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x002F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGNewOutput						(SeqGrabComponent 		s,
								 Handle 				dataRef,
								 OSType 				dataRefType,
								 long 					whereFlags,
								 SGOutput *				sgOut)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0030, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGDisposeOutput					(SeqGrabComponent 		s,
								 SGOutput 				sgOut)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0031, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetOutputFlags				(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 long 					whereFlags)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0032, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelOutput				(SeqGrabComponent 		s,
								 SGChannel 				c,
								 SGOutput 				sgOut)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0033, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetDataOutputStorageSpaceRemaining (SeqGrabComponent 	s,
								 SGOutput 				sgOut,
								 unsigned long *		space)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0034, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGHandleUpdateEvent				(SeqGrabComponent 		s,
								 const EventRecord *	event,
								 Boolean *				handled)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0035, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetOutputNextOutput			(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 SGOutput 				nextOut)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0036, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetOutputNextOutput			(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 SGOutput *				nextOut)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0037, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetOutputMaximumOffset		(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 const wide *			maxOffset)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0038, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetOutputMaximumOffset		(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 wide *					maxOffset)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0039, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetOutputDataReference		(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 Handle *				dataRef,
								 OSType *				dataRefType)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x003A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGWriteExtendedMovieData		(SeqGrabComponent 		s,
								 SGChannel 				c,
								 Ptr 					p,
								 long 					len,
								 wide *					offset,
								 SGOutput *				sgOut)								FIVEWORDINLINE(0x2F3C, 0x0014, 0x003B, 0x7000, 0xA82A);

/*
	calls from Channel to seqGrab
*/
EXTERN_API( ComponentResult )
SGWriteMovieData				(SeqGrabComponent 		s,
								 SGChannel 				c,
								 Ptr 					p,
								 long 					len,
								 long *					offset)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAddFrameReference				(SeqGrabComponent 		s,
								 SeqGrabFrameInfoPtr 	frameInfo)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetNextFrameReference			(SeqGrabComponent 		s,
								 SeqGrabFrameInfoPtr 	frameInfo,
								 TimeValue *			frameDuration,
								 long *					frameNumber)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetTimeBase					(SeqGrabComponent 		s,
								 TimeBase *				tb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSortDeviceList				(SeqGrabComponent 		s,
								 SGDeviceList 			list)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0104, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAddMovieData					(SeqGrabComponent 		s,
								 SGChannel 				c,
								 Ptr 					p,
								 long 					len,
								 long *					offset,
								 long 					chRefCon,
								 TimeValue 				time,
								 short 					writeType)							FIVEWORDINLINE(0x2F3C, 0x001A, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGChangedSource					(SeqGrabComponent 		s,
								 SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0106, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAddExtendedFrameReference		(SeqGrabComponent 		s,
								 SeqGrabExtendedFrameInfoPtr  frameInfo)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0107, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetNextExtendedFrameReference	(SeqGrabComponent 		s,
								 SeqGrabExtendedFrameInfoPtr  frameInfo,
								 TimeValue *			frameDuration,
								 long *					frameNumber)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0108, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAddExtendedMovieData			(SeqGrabComponent 		s,
								 SGChannel 				c,
								 Ptr 					p,
								 long 					len,
								 wide *					offset,
								 long 					chRefCon,
								 TimeValue 				time,
								 short 					writeType,
								 SGOutput *				whichOutput)						FIVEWORDINLINE(0x2F3C, 0x001E, 0x0109, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAddOutputDataRefToMedia		(SeqGrabComponent 		s,
								 SGOutput 				sgOut,
								 Media 					theMedia,
								 SampleDescriptionHandle  desc)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x010A, 0x7000, 0xA82A);



/*** Sequence Grab CHANNEL Component Stuff ***/

EXTERN_API( ComponentResult )
SGSetChannelUsage				(SGChannel 				c,
								 long 					usage)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0080, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelUsage				(SGChannel 				c,
								 long *					usage)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0081, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelBounds				(SGChannel 				c,
								 const Rect *			bounds)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0082, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelBounds				(SGChannel 				c,
								 Rect *					bounds)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0083, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelVolume				(SGChannel 				c,
								 short 					volume)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0084, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelVolume				(SGChannel 				c,
								 short *				volume)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0085, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelInfo				(SGChannel 				c,
								 long *					channelInfo)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0086, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelPlayFlags			(SGChannel 				c,
								 long 					playFlags)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0087, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelPlayFlags			(SGChannel 				c,
								 long *					playFlags)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0088, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelMaxFrames			(SGChannel 				c,
								 long 					frameCount)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0089, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelMaxFrames			(SGChannel 				c,
								 long *					frameCount)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelRefCon				(SGChannel 				c,
								 long 					refCon)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x008B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelClip				(SGChannel 				c,
								 RgnHandle 				theClip)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelClip				(SGChannel 				c,
								 RgnHandle *			theClip)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelSampleDescription	(SGChannel 				c,
								 Handle 				sampleDesc)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x008E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelDeviceList			(SGChannel 				c,
								 long 					selectionFlags,
								 SGDeviceList *			list)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x008F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelDevice				(SGChannel 				c,
								 StringPtr 				name)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0090, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetChannelMatrix				(SGChannel 				c,
								 const MatrixRecord *	m)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0091, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelMatrix				(SGChannel 				c,
								 MatrixRecord *			m)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0092, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetChannelTimeScale			(SGChannel 				c,
								 TimeScale *			scale)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0093, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGChannelPutPicture				(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0094, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGChannelSetRequestedDataRate	(SGChannel 				c,
								 long 					bytesPerSecond)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0095, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGChannelGetRequestedDataRate	(SGChannel 				c,
								 long *					bytesPerSecond)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0096, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGChannelSetDataSourceName		(SGChannel 				c,
								 ConstStr255Param 		name,
								 ScriptCode 			scriptTag)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x0097, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGChannelGetDataSourceName		(SGChannel 				c,
								 Str255 				name,
								 ScriptCode *			scriptTag)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0098, 0x7000, 0xA82A);

/*
	calls from seqGrab to Channel
*/
EXTERN_API( ComponentResult )
SGInitChannel					(SGChannel 				c,
								 SeqGrabComponent 		owner)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0180, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGWriteSamples					(SGChannel 				c,
								 Movie 					m,
								 AliasHandle 			theFile)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0181, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetDataRate					(SGChannel 				c,
								 long *					bytesPerSecond)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0182, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAlignChannelRect				(SGChannel 				c,
								 Rect *					r)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0183, 0x7000, 0xA82A);

/*
	Dorky dialog panel calls
*/
EXTERN_API( ComponentResult )
SGPanelGetDitl					(SeqGrabComponent 		s,
								 Handle *				ditl)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0200, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelGetTitle					(SeqGrabComponent 		s,
								 Str255 				title)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0201, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelCanRun					(SeqGrabComponent 		s,
								 SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0202, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelInstall					(SeqGrabComponent 		s,
								 SGChannel 				c,
								 DialogPtr 				d,
								 short 					itemOffset)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x0203, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelEvent					(SeqGrabComponent 		s,
								 SGChannel 				c,
								 DialogPtr 				d,
								 short 					itemOffset,
								 const EventRecord *	theEvent,
								 short *				itemHit,
								 Boolean *				handled)							FIVEWORDINLINE(0x2F3C, 0x0016, 0x0204, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelItem						(SeqGrabComponent 		s,
								 SGChannel 				c,
								 DialogPtr 				d,
								 short 					itemOffset,
								 short 					itemNum)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0205, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelRemove					(SeqGrabComponent 		s,
								 SGChannel 				c,
								 DialogPtr 				d,
								 short 					itemOffset)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x0206, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelSetGrabber				(SeqGrabComponent 		s,
								 SeqGrabComponent 		sg)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0207, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelSetResFile				(SeqGrabComponent 		s,
								 short 					resRef)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0208, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelGetSettings				(SeqGrabComponent 		s,
								 SGChannel 				c,
								 UserData *				ud,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0209, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelSetSettings				(SeqGrabComponent 		s,
								 SGChannel 				c,
								 UserData 				ud,
								 long 					flags)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x020A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelValidateInput			(SeqGrabComponent 		s,
								 Boolean *				ok)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x020B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGPanelSetEventFilter			(SeqGrabComponent 		s,
								 SGModalFilterUPP 		proc,
								 long 					refCon)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x020C, 0x7000, 0xA82A);


/*** Sequence Grab VIDEO CHANNEL Component Stuff ***/
/*
	Video stuff
*/

struct SGCompressInfo {
	Ptr 							buffer;
	unsigned long 					bufferSize;
	UInt8 							similarity;
	UInt8 							reserved;
};
typedef struct SGCompressInfo			SGCompressInfo;
typedef CALLBACK_API( ComponentResult , SGGrabBottleProcPtr )(SGChannel c, short bufferNum, long refCon);
typedef CALLBACK_API( ComponentResult , SGGrabCompleteBottleProcPtr )(SGChannel c, short bufferNum, Boolean *done, long refCon);
typedef CALLBACK_API( ComponentResult , SGDisplayBottleProcPtr )(SGChannel c, short bufferNum, MatrixRecord *mp, RgnHandle clipRgn, long refCon);
typedef CALLBACK_API( ComponentResult , SGCompressBottleProcPtr )(SGChannel c, short bufferNum, long refCon);
typedef CALLBACK_API( ComponentResult , SGCompressCompleteBottleProcPtr )(SGChannel c, short bufferNum, Boolean *done, SGCompressInfo *ci, long refCon);
typedef CALLBACK_API( ComponentResult , SGAddFrameBottleProcPtr )(SGChannel c, short bufferNum, TimeValue atTime, TimeScale scale, const SGCompressInfo *ci, long refCon);
typedef CALLBACK_API( ComponentResult , SGTransferFrameBottleProcPtr )(SGChannel c, short bufferNum, MatrixRecord *mp, RgnHandle clipRgn, long refCon);
typedef CALLBACK_API( ComponentResult , SGGrabCompressCompleteBottleProcPtr )(SGChannel c, Boolean *done, SGCompressInfo *ci, TimeRecord *t, long refCon);
typedef CALLBACK_API( ComponentResult , SGDisplayCompressBottleProcPtr )(SGChannel c, Ptr dataPtr, ImageDescriptionHandle desc, MatrixRecord *mp, RgnHandle clipRgn, long refCon);
typedef STACK_UPP_TYPE(SGGrabBottleProcPtr) 					SGGrabBottleUPP;
typedef STACK_UPP_TYPE(SGGrabCompleteBottleProcPtr) 			SGGrabCompleteBottleUPP;
typedef STACK_UPP_TYPE(SGDisplayBottleProcPtr) 					SGDisplayBottleUPP;
typedef STACK_UPP_TYPE(SGCompressBottleProcPtr) 				SGCompressBottleUPP;
typedef STACK_UPP_TYPE(SGCompressCompleteBottleProcPtr) 		SGCompressCompleteBottleUPP;
typedef STACK_UPP_TYPE(SGAddFrameBottleProcPtr) 				SGAddFrameBottleUPP;
typedef STACK_UPP_TYPE(SGTransferFrameBottleProcPtr) 			SGTransferFrameBottleUPP;
typedef STACK_UPP_TYPE(SGGrabCompressCompleteBottleProcPtr) 	SGGrabCompressCompleteBottleUPP;
typedef STACK_UPP_TYPE(SGDisplayCompressBottleProcPtr) 			SGDisplayCompressBottleUPP;

struct VideoBottles {
	short 							procCount;
	SGGrabBottleUPP 				grabProc;
	SGGrabCompleteBottleUPP 		grabCompleteProc;
	SGDisplayBottleUPP 				displayProc;
	SGCompressBottleUPP 			compressProc;
	SGCompressCompleteBottleUPP 	compressCompleteProc;
	SGAddFrameBottleUPP 			addFrameProc;
	SGTransferFrameBottleUPP 		transferFrameProc;
	SGGrabCompressCompleteBottleUPP  grabCompressCompleteProc;
	SGDisplayCompressBottleUPP 		displayCompressProc;
};
typedef struct VideoBottles				VideoBottles;
EXTERN_API( ComponentResult )
SGGetSrcVideoBounds				(SGChannel 				c,
								 Rect *					r)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetVideoRect					(SGChannel 				c,
								 const Rect *			r)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetVideoRect					(SGChannel 				c,
								 Rect *					r)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetVideoCompressorType		(SGChannel 				c,
								 OSType *				compressorType)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetVideoCompressorType		(SGChannel 				c,
								 OSType 				compressorType)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0104, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetVideoCompressor			(SGChannel 				c,
								 short 					depth,
								 CompressorComponent 	compressor,
								 CodecQ 				spatialQuality,
								 CodecQ 				temporalQuality,
								 long 					keyFrameRate)						FIVEWORDINLINE(0x2F3C, 0x0012, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetVideoCompressor			(SGChannel 				c,
								 short *				depth,
								 CompressorComponent *	compressor,
								 CodecQ *				spatialQuality,
								 CodecQ *				temporalQuality,
								 long *					keyFrameRate)						FIVEWORDINLINE(0x2F3C, 0x0014, 0x0106, 0x7000, 0xA82A);

EXTERN_API( ComponentInstance )
SGGetVideoDigitizerComponent	(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0107, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetVideoDigitizerComponent	(SGChannel 				c,
								 ComponentInstance 		vdig)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0108, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGVideoDigitizerChanged			(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0109, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetVideoBottlenecks			(SGChannel 				c,
								 VideoBottles *			vb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x010A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetVideoBottlenecks			(SGChannel 				c,
								 VideoBottles *			vb)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x010B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGrabFrame						(SGChannel 				c,
								 short 					bufferNum)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x010C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGrabFrameComplete				(SGChannel 				c,
								 short 					bufferNum,
								 Boolean *				done)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x010D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGDisplayFrame					(SGChannel 				c,
								 short 					bufferNum,
								 const MatrixRecord *	mp,
								 RgnHandle 				clipRgn)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x010E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGCompressFrame					(SGChannel 				c,
								 short 					bufferNum)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x010F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGCompressFrameComplete			(SGChannel 				c,
								 short 					bufferNum,
								 Boolean *				done,
								 SGCompressInfo *		ci)									FIVEWORDINLINE(0x2F3C, 0x000A, 0x0110, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGAddFrame						(SGChannel 				c,
								 short 					bufferNum,
								 TimeValue 				atTime,
								 TimeScale 				scale,
								 const SGCompressInfo *	ci)									FIVEWORDINLINE(0x2F3C, 0x000E, 0x0111, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGTransferFrameForCompress		(SGChannel 				c,
								 short 					bufferNum,
								 const MatrixRecord *	mp,
								 RgnHandle 				clipRgn)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x0112, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetCompressBuffer				(SGChannel 				c,
								 short 					depth,
								 const Rect *			compressSize)						FIVEWORDINLINE(0x2F3C, 0x0006, 0x0113, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetCompressBuffer				(SGChannel 				c,
								 short *				depth,
								 Rect *					compressSize)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x0114, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetBufferInfo					(SGChannel 				c,
								 short 					bufferNum,
								 PixMapHandle *			bufferPM,
								 Rect *					bufferRect,
								 GWorldPtr *			compressBuffer,
								 Rect *					compressBufferRect)					FIVEWORDINLINE(0x2F3C, 0x0012, 0x0115, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetUseScreenBuffer			(SGChannel 				c,
								 Boolean 				useScreenBuffer)					FIVEWORDINLINE(0x2F3C, 0x0002, 0x0116, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetUseScreenBuffer			(SGChannel 				c,
								 Boolean *				useScreenBuffer)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0117, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGrabCompressComplete			(SGChannel 				c,
								 Boolean *				done,
								 SGCompressInfo *		ci,
								 TimeRecord *			tr)									FIVEWORDINLINE(0x2F3C, 0x000C, 0x0118, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGDisplayCompress				(SGChannel 				c,
								 Ptr 					dataPtr,
								 ImageDescriptionHandle  desc,
								 MatrixRecord *			mp,
								 RgnHandle 				clipRgn)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x0119, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetFrameRate					(SGChannel 				c,
								 Fixed 					frameRate)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x011A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetFrameRate					(SGChannel 				c,
								 Fixed *				frameRate)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x011B, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
SGSetPreferredPacketSize		(SGChannel 				c,
								 long 					preferredPacketSizeInBytes)			FIVEWORDINLINE(0x2F3C, 0x0004, 0x0121, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetPreferredPacketSize		(SGChannel 				c,
								 long *					preferredPacketSizeInBytes)			FIVEWORDINLINE(0x2F3C, 0x0004, 0x0122, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetUserVideoCompressorList	(SGChannel 				c,
								 Handle 				compressorTypes)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0123, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetUserVideoCompressorList	(SGChannel 				c,
								 Handle *				compressorTypes)					FIVEWORDINLINE(0x2F3C, 0x0004, 0x0124, 0x7000, 0xA82A);

/*** Sequence Grab SOUND CHANNEL Component Stuff ***/

/*
	Sound stuff
*/
EXTERN_API( ComponentResult )
SGSetSoundInputDriver			(SGChannel 				c,
								 ConstStr255Param 		driverName)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0100, 0x7000, 0xA82A);

EXTERN_API( long )
SGGetSoundInputDriver			(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSoundInputDriverChanged		(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetSoundRecordChunkSize		(SGChannel 				c,
								 long 					seconds)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0103, 0x7000, 0xA82A);

EXTERN_API( long )
SGGetSoundRecordChunkSize		(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0104, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetSoundInputRate				(SGChannel 				c,
								 Fixed 					rate)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0105, 0x7000, 0xA82A);

EXTERN_API( Fixed )
SGGetSoundInputRate				(SGChannel 				c)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0106, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetSoundInputParameters		(SGChannel 				c,
								 short 					sampleSize,
								 short 					numChannels,
								 OSType 				compressionType)					FIVEWORDINLINE(0x2F3C, 0x0008, 0x0107, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetSoundInputParameters		(SGChannel 				c,
								 short *				sampleSize,
								 short *				numChannels,
								 OSType *				compressionType)					FIVEWORDINLINE(0x2F3C, 0x000C, 0x0108, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetAdditionalSoundRates		(SGChannel 				c,
								 Handle 				rates)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0109, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetAdditionalSoundRates		(SGChannel 				c,
								 Handle *				rates)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x010A, 0x7000, 0xA82A);

/*
	Text stuff
*/
EXTERN_API( ComponentResult )
SGSetFontName					(SGChannel 				c,
								 StringPtr 				pstr)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetFontSize					(SGChannel 				c,
								 short 					fontSize)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetTextForeColor				(SGChannel 				c,
								 RGBColor *				theColor)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetTextBackColor				(SGChannel 				c,
								 RGBColor *				theColor)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetJustification				(SGChannel 				c,
								 short 					just)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0104, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGGetTextReturnToSpaceValue		(SGChannel 				c,
								 short *				rettospace)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetTextReturnToSpaceValue		(SGChannel 				c,
								 short 					rettospace)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x0106, 0x7000, 0xA82A);

/*
	Music stuff
*/
EXTERN_API( ComponentResult )
SGGetInstrument					(SGChannel 				c,
								 ToneDescription *		td)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0100, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SGSetInstrument					(SGChannel 				c,
								 ToneDescription *		td)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);



enum {
	sgChannelAtom				= FOUR_CHAR_CODE('chan'),
	sgChannelSettingsAtom		= FOUR_CHAR_CODE('ctom'),
	sgChannelDescription		= FOUR_CHAR_CODE('cdsc'),
	sgChannelSettings			= FOUR_CHAR_CODE('cset')
};


enum {
	sgDeviceNameType			= FOUR_CHAR_CODE('name'),
	sgUsageType					= FOUR_CHAR_CODE('use '),
	sgPlayFlagsType				= FOUR_CHAR_CODE('plyf'),
	sgClipType					= FOUR_CHAR_CODE('clip'),
	sgMatrixType				= FOUR_CHAR_CODE('mtrx'),
	sgVolumeType				= FOUR_CHAR_CODE('volu')
};


enum {
	sgPanelSettingsAtom			= FOUR_CHAR_CODE('ptom'),
	sgPanelDescription			= FOUR_CHAR_CODE('pdsc'),
	sgPanelSettings				= FOUR_CHAR_CODE('pset')
};


enum {
	sgcSoundCompressionType		= FOUR_CHAR_CODE('scmp'),
	sgcSoundSampleRateType		= FOUR_CHAR_CODE('srat'),
	sgcSoundChannelCountType	= FOUR_CHAR_CODE('schn'),
	sgcSoundSampleSizeType		= FOUR_CHAR_CODE('ssiz'),
	sgcSoundInputType			= FOUR_CHAR_CODE('sinp'),
	sgcSoundGainType			= FOUR_CHAR_CODE('gain')
};


enum {
	sgcVideoHueType				= FOUR_CHAR_CODE('hue '),
	sgcVideoSaturationType		= FOUR_CHAR_CODE('satr'),
	sgcVideoContrastType		= FOUR_CHAR_CODE('trst'),
	sgcVideoSharpnessType		= FOUR_CHAR_CODE('shrp'),
	sgcVideoBrigtnessType		= FOUR_CHAR_CODE('brit'),
	sgcVideoBlackLevelType		= FOUR_CHAR_CODE('blkl'),
	sgcVideoWhiteLevelType		= FOUR_CHAR_CODE('whtl'),
	sgcVideoInputType			= FOUR_CHAR_CODE('vinp'),
	sgcVideoFormatType			= FOUR_CHAR_CODE('vstd'),
	sgcVideoFilterType			= FOUR_CHAR_CODE('vflt'),
	sgcVideoRectType			= FOUR_CHAR_CODE('vrct'),
	sgcVideoDigitizerType		= FOUR_CHAR_CODE('vdig')
};





typedef ComponentInstance 				QTVideoOutputComponent;
/* Component type and subtype enumerations*/

enum {
	QTVideoOutputComponentType	= FOUR_CHAR_CODE('vout'),
	QTVideoOutputComponentBaseSubType = FOUR_CHAR_CODE('base')
};


/* QTVideoOutput Component flags*/


enum {
	kQTVideoOutputDontDisplayToUser = 1L << 0
};

/* Display mode atom types*/


enum {
	kQTVODisplayModeItem		= FOUR_CHAR_CODE('qdmi'),
	kQTVODimensions				= FOUR_CHAR_CODE('dimn'),		/* atom contains two longs - pixel count - width, height*/
	kQTVOResolution				= FOUR_CHAR_CODE('resl'),		/* atom contains two Fixed - hRes, vRes in dpi*/
	kQTVORefreshRate			= FOUR_CHAR_CODE('refr'),		/* atom contains one Fixed - refresh rate in Hz*/
	kQTVOPixelType				= FOUR_CHAR_CODE('pixl'),		/* atom contains one OSType - pixel format of mode*/
	kQTVOName					= FOUR_CHAR_CODE('name'),		/* atom contains string (no length byte) - name of modefor display to user*/
	kQTVODecompressors			= FOUR_CHAR_CODE('deco'),		/* atom contains other atoms indicating supported decompressors*/
																/* kQTVODecompressors sub-atoms*/
	kQTVODecompressorType		= FOUR_CHAR_CODE('dety'),		/* atom contains one OSType - decompressor type code*/
	kQTVODecompressorContinuous	= FOUR_CHAR_CODE('cont'),		/* atom contains one Boolean - true if this type is displayed continuously*/
	kQTVODecompressorComponent	= FOUR_CHAR_CODE('cmpt')		/* atom contains one Component - component id of decompressor*/
};

/** These are QTVideoOutput procedures **/
EXTERN_API( ComponentResult )
QTVideoOutputGetDisplayModeList	(QTVideoOutputComponent  vo,
								 QTAtomContainer *		outputs)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0001, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetCurrentClientName (QTVideoOutputComponent  vo,
								 Str255 				str)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0002, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputSetClientName		(QTVideoOutputComponent  vo,
								 ConstStr255Param 		str)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetClientName		(QTVideoOutputComponent  vo,
								 Str255 				str)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0004, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputBegin				(QTVideoOutputComponent  vo)								FIVEWORDINLINE(0x2F3C, 0x0000, 0x0005, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputEnd				(QTVideoOutputComponent  vo)								FIVEWORDINLINE(0x2F3C, 0x0000, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputSetDisplayMode		(QTVideoOutputComponent  vo,
								 long 					displayModeID)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetDisplayMode		(QTVideoOutputComponent  vo,
								 long *					displayModeID)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0008, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputCustomConfigureDisplay (QTVideoOutputComponent  vo,
								 ModalFilterUPP 		filter)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0009, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputSaveState			(QTVideoOutputComponent  vo,
								 QTAtomContainer *		state)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputRestoreState		(QTVideoOutputComponent  vo,
								 QTAtomContainer 		state)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetGWorld			(QTVideoOutputComponent  vo,
								 GWorldPtr *			gw)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x000C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetGWorldParameters (QTVideoOutputComponent  vo,
								 Ptr *					baseAddr,
								 long *					rowBytes,
								 CTabHandle *			colorTable)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x000D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetIndSoundOutput	(QTVideoOutputComponent  vo,
								 long 					index,
								 Component *			outputComponent)					FIVEWORDINLINE(0x2F3C, 0x0008, 0x000E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputGetClock			(QTVideoOutputComponent  vo,
								 ComponentInstance *	clock)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x000F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
QTVideoOutputSetEchoPort		(QTVideoOutputComponent  vo,
								 CGrafPtr 				echoPort)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0010, 0x7000, 0xA82A);

/* UPP call backs */
enum { uppDataHCompletionProcInfo = 0x00000BC0 }; 				/* pascal no_return_value Func(4_bytes, 4_bytes, 2_bytes) */
enum { uppVdigIntProcInfo = 0x000003C0 }; 						/* pascal no_return_value Func(4_bytes, 4_bytes) */
enum { uppSGDataProcInfo = 0x003BFFE0 }; 						/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 2_bytes, 4_bytes) */
enum { uppSGModalFilterProcInfo = 0x00003FD0 }; 				/* pascal 1_byte Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSGGrabBottleProcInfo = 0x00000EF0 }; 					/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppSGGrabCompleteBottleProcInfo = 0x00003EF0 }; 			/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes) */
enum { uppSGDisplayBottleProcInfo = 0x0000FEF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSGCompressBottleProcInfo = 0x00000EF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppSGCompressCompleteBottleProcInfo = 0x0000FEF0 }; 		/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSGAddFrameBottleProcInfo = 0x0003FEF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSGTransferFrameBottleProcInfo = 0x0000FEF0 }; 		/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSGGrabCompressCompleteBottleProcInfo = 0x0000FFF0 }; 	/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppSGDisplayCompressBottleProcInfo = 0x0003FFF0 }; 		/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewDataHCompletionProc(userRoutine) 					(DataHCompletionUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDataHCompletionProcInfo, GetCurrentArchitecture())
#define NewVdigIntProc(userRoutine) 							(VdigIntUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppVdigIntProcInfo, GetCurrentArchitecture())
#define NewSGDataProc(userRoutine) 								(SGDataUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGDataProcInfo, GetCurrentArchitecture())
#define NewSGModalFilterProc(userRoutine) 						(SGModalFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGModalFilterProcInfo, GetCurrentArchitecture())
#define NewSGGrabBottleProc(userRoutine) 						(SGGrabBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGGrabBottleProcInfo, GetCurrentArchitecture())
#define NewSGGrabCompleteBottleProc(userRoutine) 				(SGGrabCompleteBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGGrabCompleteBottleProcInfo, GetCurrentArchitecture())
#define NewSGDisplayBottleProc(userRoutine) 					(SGDisplayBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGDisplayBottleProcInfo, GetCurrentArchitecture())
#define NewSGCompressBottleProc(userRoutine) 					(SGCompressBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGCompressBottleProcInfo, GetCurrentArchitecture())
#define NewSGCompressCompleteBottleProc(userRoutine) 			(SGCompressCompleteBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGCompressCompleteBottleProcInfo, GetCurrentArchitecture())
#define NewSGAddFrameBottleProc(userRoutine) 					(SGAddFrameBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGAddFrameBottleProcInfo, GetCurrentArchitecture())
#define NewSGTransferFrameBottleProc(userRoutine) 				(SGTransferFrameBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGTransferFrameBottleProcInfo, GetCurrentArchitecture())
#define NewSGGrabCompressCompleteBottleProc(userRoutine) 		(SGGrabCompressCompleteBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGGrabCompressCompleteBottleProcInfo, GetCurrentArchitecture())
#define NewSGDisplayCompressBottleProc(userRoutine) 			(SGDisplayCompressBottleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSGDisplayCompressBottleProcInfo, GetCurrentArchitecture())
#define CallDataHCompletionProc(userRoutine, request, refcon, err)  CALL_THREE_PARAMETER_UPP((userRoutine), uppDataHCompletionProcInfo, (request), (refcon), (err))
#define CallVdigIntProc(userRoutine, flags, refcon) 			CALL_TWO_PARAMETER_UPP((userRoutine), uppVdigIntProcInfo, (flags), (refcon))
#define CallSGDataProc(userRoutine, c, p, len, offset, chRefCon, time, writeType, refCon)  CALL_EIGHT_PARAMETER_UPP((userRoutine), uppSGDataProcInfo, (c), (p), (len), (offset), (chRefCon), (time), (writeType), (refCon))
#define CallSGModalFilterProc(userRoutine, theDialog, theEvent, itemHit, refCon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppSGModalFilterProcInfo, (theDialog), (theEvent), (itemHit), (refCon))
#define CallSGGrabBottleProc(userRoutine, c, bufferNum, refCon)  CALL_THREE_PARAMETER_UPP((userRoutine), uppSGGrabBottleProcInfo, (c), (bufferNum), (refCon))
#define CallSGGrabCompleteBottleProc(userRoutine, c, bufferNum, done, refCon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppSGGrabCompleteBottleProcInfo, (c), (bufferNum), (done), (refCon))
#define CallSGDisplayBottleProc(userRoutine, c, bufferNum, mp, clipRgn, refCon)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppSGDisplayBottleProcInfo, (c), (bufferNum), (mp), (clipRgn), (refCon))
#define CallSGCompressBottleProc(userRoutine, c, bufferNum, refCon)  CALL_THREE_PARAMETER_UPP((userRoutine), uppSGCompressBottleProcInfo, (c), (bufferNum), (refCon))
#define CallSGCompressCompleteBottleProc(userRoutine, c, bufferNum, done, ci, refCon)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppSGCompressCompleteBottleProcInfo, (c), (bufferNum), (done), (ci), (refCon))
#define CallSGAddFrameBottleProc(userRoutine, c, bufferNum, atTime, scale, ci, refCon)  CALL_SIX_PARAMETER_UPP((userRoutine), uppSGAddFrameBottleProcInfo, (c), (bufferNum), (atTime), (scale), (ci), (refCon))
#define CallSGTransferFrameBottleProc(userRoutine, c, bufferNum, mp, clipRgn, refCon)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppSGTransferFrameBottleProcInfo, (c), (bufferNum), (mp), (clipRgn), (refCon))
#define CallSGGrabCompressCompleteBottleProc(userRoutine, c, done, ci, t, refCon)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppSGGrabCompressCompleteBottleProcInfo, (c), (done), (ci), (t), (refCon))
#define CallSGDisplayCompressBottleProc(userRoutine, c, dataPtr, desc, mp, clipRgn, refCon)  CALL_SIX_PARAMETER_UPP((userRoutine), uppSGDisplayCompressBottleProcInfo, (c), (dataPtr), (desc), (mp), (clipRgn), (refCon))

/* selectors for component calls */
enum {
	kClockGetTimeSelect								= 0x0001,
	kClockNewCallBackSelect							= 0x0002,
	kClockDisposeCallBackSelect						= 0x0003,
	kClockCallMeWhenSelect							= 0x0004,
	kClockCancelCallBackSelect						= 0x0005,
	kClockRateChangedSelect							= 0x0006,
	kClockTimeChangedSelect							= 0x0007,
	kClockSetTimeBaseSelect							= 0x0008,
	kClockStartStopChangedSelect					= 0x0009,
	kClockGetRateSelect								= 0x000A,
	kSCGetCompressionExtendedSelect					= 0x0001,
	kSCPositionRectSelect							= 0x0002,
	kSCPositionDialogSelect							= 0x0003,
	kSCSetTestImagePictHandleSelect					= 0x0004,
	kSCSetTestImagePictFileSelect					= 0x0005,
	kSCSetTestImagePixMapSelect						= 0x0006,
	kSCGetBestDeviceRectSelect						= 0x0007,
	kSCRequestImageSettingsSelect					= 0x000A,
	kSCCompressImageSelect							= 0x000B,
	kSCCompressPictureSelect						= 0x000C,
	kSCCompressPictureFileSelect					= 0x000D,
	kSCRequestSequenceSettingsSelect				= 0x000E,
	kSCCompressSequenceBeginSelect					= 0x000F,
	kSCCompressSequenceFrameSelect					= 0x0010,
	kSCCompressSequenceEndSelect					= 0x0011,
	kSCDefaultPictHandleSettingsSelect				= 0x0012,
	kSCDefaultPictFileSettingsSelect				= 0x0013,
	kSCDefaultPixMapSettingsSelect					= 0x0014,
	kSCGetInfoSelect								= 0x0015,
	kSCSetInfoSelect								= 0x0016,
	kSCNewGWorldSelect								= 0x0017,
	kSCSetCompressFlagsSelect						= 0x0018,
	kSCGetCompressFlagsSelect						= 0x0019,
	kSCGetSettingsAsTextSelect						= 0x001A,
	kSCGetSettingsAsAtomContainerSelect				= 0x001B,
	kSCSetSettingsFromAtomContainerSelect			= 0x001C,
	kTweenerInitializeSelect						= 0x0001,
	kTweenerDoTweenSelect							= 0x0002,
	kTweenerResetSelect								= 0x0003,
	kTCGetCurrentTimeCodeSelect						= 0x0101,
	kTCGetTimeCodeAtTimeSelect						= 0x0102,
	kTCTimeCodeToStringSelect						= 0x0103,
	kTCTimeCodeToFrameNumberSelect					= 0x0104,
	kTCFrameNumberToTimeCodeSelect					= 0x0105,
	kTCGetSourceRefSelect							= 0x0106,
	kTCSetSourceRefSelect							= 0x0107,
	kTCSetTimeCodeFlagsSelect						= 0x0108,
	kTCGetTimeCodeFlagsSelect						= 0x0109,
	kTCSetDisplayOptionsSelect						= 0x010A,
	kTCGetDisplayOptionsSelect						= 0x010B,
	kMovieImportHandleSelect						= 0x0001,
	kMovieImportFileSelect							= 0x0002,
	kMovieImportSetSampleDurationSelect				= 0x0003,
	kMovieImportSetSampleDescriptionSelect			= 0x0004,
	kMovieImportSetMediaFileSelect					= 0x0005,
	kMovieImportSetDimensionsSelect					= 0x0006,
	kMovieImportSetChunkSizeSelect					= 0x0007,
	kMovieImportSetProgressProcSelect				= 0x0008,
	kMovieImportSetAuxiliaryDataSelect				= 0x0009,
	kMovieImportSetFromScrapSelect					= 0x000A,
	kMovieImportDoUserDialogSelect					= 0x000B,
	kMovieImportSetDurationSelect					= 0x000C,
	kMovieImportGetAuxiliaryDataTypeSelect			= 0x000D,
	kMovieImportValidateSelect						= 0x000E,
	kMovieImportGetFileTypeSelect					= 0x000F,
	kMovieImportDataRefSelect						= 0x0010,
	kMovieImportGetSampleDescriptionSelect			= 0x0011,
	kMovieImportGetMIMETypeListSelect				= 0x0012,
	kMovieImportSetOffsetAndLimitSelect				= 0x0013,
	kMovieImportGetSettingsAsAtomContainerSelect	= 0x0014,
	kMovieImportSetSettingsFromAtomContainerSelect = 0x0015,
	kMovieExportToHandleSelect						= 0x0080,
	kMovieExportToFileSelect						= 0x0081,
	kMovieExportGetAuxiliaryDataSelect				= 0x0083,
	kMovieExportSetProgressProcSelect				= 0x0084,
	kMovieExportSetSampleDescriptionSelect			= 0x0085,
	kMovieExportDoUserDialogSelect					= 0x0086,
	kMovieExportGetCreatorTypeSelect				= 0x0087,
	kMovieExportToDataRefSelect						= 0x0088,
	kMovieExportFromProceduresToDataRefSelect		= 0x0089,
	kMovieExportAddDataSourceSelect					= 0x008A,
	kMovieExportValidateSelect						= 0x008B,
	kMovieExportGetSettingsAsAtomContainerSelect	= 0x008C,
	kMovieExportSetSettingsFromAtomContainerSelect = 0x008D,
	kMovieExportGetFileNameExtensionSelect			= 0x008E,
	kMovieExportGetShortFileTypeStringSelect		= 0x008F,
	kMovieExportGetSourceMediaTypeSelect			= 0x0090,
	kTextExportGetDisplayDataSelect					= 0x0100,
	kTextExportGetTimeFractionSelect				= 0x0101,
	kTextExportSetTimeFractionSelect				= 0x0102,
	kTextExportGetSettingsSelect					= 0x0103,
	kTextExportSetSettingsSelect					= 0x0104,
	kMIDIImportGetSettingsSelect					= 0x0100,
	kMIDIImportSetSettingsSelect					= 0x0101,
	kMovieExportNewGetDataAndPropertiesProcsSelect = 0x0100,
	kMovieExportDisposeGetDataAndPropertiesProcsSelect = 0x0101,
	kGraphicsImageImportSetSequenceEnabledSelect	= 0x0100,
	kGraphicsImageImportGetSequenceEnabledSelect	= 0x0101,
	kPreviewShowDataSelect							= 0x0001,
	kPreviewMakePreviewSelect						= 0x0002,
	kPreviewMakePreviewReferenceSelect				= 0x0003,
	kPreviewEventSelect								= 0x0004,
	kDataCodecDecompressSelect						= 0x0001,
	kDataCodecGetCompressBufferSizeSelect			= 0x0002,
	kDataCodecCompressSelect						= 0x0003,
	kDataCodecBeginInterruptSafeSelect				= 0x0004,
	kDataCodecEndInterruptSafeSelect				= 0x0005,
	kDataCodecDecompressPartialSelect				= 0x0006,
	kDataCodecCompressPartialSelect					= 0x0007,
	kDataHGetDataSelect								= 0x0002,
	kDataHPutDataSelect								= 0x0003,
	kDataHFlushDataSelect							= 0x0004,
	kDataHOpenForWriteSelect						= 0x0005,
	kDataHCloseForWriteSelect						= 0x0006,
	kDataHOpenForReadSelect							= 0x0008,
	kDataHCloseForReadSelect						= 0x0009,
	kDataHSetDataRefSelect							= 0x000A,
	kDataHGetDataRefSelect							= 0x000B,
	kDataHCompareDataRefSelect						= 0x000C,
	kDataHTaskSelect								= 0x000D,
	kDataHScheduleDataSelect						= 0x000E,
	kDataHFinishDataSelect							= 0x000F,
	kDataHFlushCacheSelect							= 0x0010,
	kDataHResolveDataRefSelect						= 0x0011,
	kDataHGetFileSizeSelect							= 0x0012,
	kDataHCanUseDataRefSelect						= 0x0013,
	kDataHGetVolumeListSelect						= 0x0014,
	kDataHWriteSelect								= 0x0015,
	kDataHPreextendSelect							= 0x0016,
	kDataHSetFileSizeSelect							= 0x0017,
	kDataHGetFreeSpaceSelect						= 0x0018,
	kDataHCreateFileSelect							= 0x0019,
	kDataHGetPreferredBlockSizeSelect				= 0x001A,
	kDataHGetDeviceIndexSelect						= 0x001B,
	kDataHIsStreamingDataHandlerSelect				= 0x001C,
	kDataHGetDataInBufferSelect						= 0x001D,
	kDataHGetScheduleAheadTimeSelect				= 0x001E,
	kDataHSetCacheSizeLimitSelect					= 0x001F,
	kDataHGetCacheSizeLimitSelect					= 0x0020,
	kDataHGetMovieSelect							= 0x0021,
	kDataHAddMovieSelect							= 0x0022,
	kDataHUpdateMovieSelect							= 0x0023,
	kDataHDoesBufferSelect							= 0x0024,
	kDataHGetFileNameSelect							= 0x0025,
	kDataHGetAvailableFileSizeSelect				= 0x0026,
	kDataHGetMacOSFileTypeSelect					= 0x0027,
	kDataHGetMIMETypeSelect							= 0x0028,
	kDataHSetDataRefWithAnchorSelect				= 0x0029,
	kDataHGetDataRefWithAnchorSelect				= 0x002A,
	kDataHSetMacOSFileTypeSelect					= 0x002B,
	kDataHSetTimeBaseSelect							= 0x002C,
	kDataHPlaybackHintsSelect						= 0x0103,
	kVDGetMaxSrcRectSelect							= 0x0001,
	kVDGetActiveSrcRectSelect						= 0x0002,
	kVDSetDigitizerRectSelect						= 0x0003,
	kVDGetDigitizerRectSelect						= 0x0004,
	kVDGetVBlankRectSelect							= 0x0005,
	kVDGetMaskPixMapSelect							= 0x0006,
	kVDGetPlayThruDestinationSelect					= 0x0008,
	kVDUseThisCLUTSelect							= 0x0009,
	kVDSetInputGammaValueSelect						= 0x000A,
	kVDGetInputGammaValueSelect						= 0x000B,
	kVDSetBrightnessSelect							= 0x000C,
	kVDGetBrightnessSelect							= 0x000D,
	kVDSetContrastSelect							= 0x000E,
	kVDSetHueSelect									= 0x000F,
	kVDSetSharpnessSelect							= 0x0010,
	kVDSetSaturationSelect							= 0x0011,
	kVDGetContrastSelect							= 0x0012,
	kVDGetHueSelect									= 0x0013,
	kVDGetSharpnessSelect							= 0x0014,
	kVDGetSaturationSelect							= 0x0015,
	kVDGrabOneFrameSelect							= 0x0016,
	kVDGetMaxAuxBufferSelect						= 0x0017,
	kVDGetDigitizerInfoSelect						= 0x0019,
	kVDGetCurrentFlagsSelect						= 0x001A,
	kVDSetKeyColorSelect							= 0x001B,
	kVDGetKeyColorSelect							= 0x001C,
	kVDAddKeyColorSelect							= 0x001D,
	kVDGetNextKeyColorSelect						= 0x001E,
	kVDSetKeyColorRangeSelect						= 0x001F,
	kVDGetKeyColorRangeSelect						= 0x0020,
	kVDSetDigitizerUserInterruptSelect				= 0x0021,
	kVDSetInputColorSpaceModeSelect					= 0x0022,
	kVDGetInputColorSpaceModeSelect					= 0x0023,
	kVDSetClipStateSelect							= 0x0024,
	kVDGetClipStateSelect							= 0x0025,
	kVDSetClipRgnSelect								= 0x0026,
	kVDClearClipRgnSelect							= 0x0027,
	kVDGetCLUTInUseSelect							= 0x0028,
	kVDSetPLLFilterTypeSelect						= 0x0029,
	kVDGetPLLFilterTypeSelect						= 0x002A,
	kVDGetMaskandValueSelect						= 0x002B,
	kVDSetMasterBlendLevelSelect					= 0x002C,
	kVDSetPlayThruDestinationSelect					= 0x002D,
	kVDSetPlayThruOnOffSelect						= 0x002E,
	kVDSetFieldPreferenceSelect						= 0x002F,
	kVDGetFieldPreferenceSelect						= 0x0030,
	kVDPreflightDestinationSelect					= 0x0032,
	kVDPreflightGlobalRectSelect					= 0x0033,
	kVDSetPlayThruGlobalRectSelect					= 0x0034,
	kVDSetInputGammaRecordSelect					= 0x0035,
	kVDGetInputGammaRecordSelect					= 0x0036,
	kVDSetBlackLevelValueSelect						= 0x0037,
	kVDGetBlackLevelValueSelect						= 0x0038,
	kVDSetWhiteLevelValueSelect						= 0x0039,
	kVDGetWhiteLevelValueSelect						= 0x003A,
	kVDGetVideoDefaultsSelect						= 0x003B,
	kVDGetNumberOfInputsSelect						= 0x003C,
	kVDGetInputFormatSelect							= 0x003D,
	kVDSetInputSelect								= 0x003E,
	kVDGetInputSelect								= 0x003F,
	kVDSetInputStandardSelect						= 0x0040,
	kVDSetupBuffersSelect							= 0x0041,
	kVDGrabOneFrameAsyncSelect						= 0x0042,
	kVDDoneSelect									= 0x0043,
	kVDSetCompressionSelect							= 0x0044,
	kVDCompressOneFrameAsyncSelect					= 0x0045,
	kVDCompressDoneSelect							= 0x0046,
	kVDReleaseCompressBufferSelect					= 0x0047,
	kVDGetImageDescriptionSelect					= 0x0048,
	kVDResetCompressSequenceSelect					= 0x0049,
	kVDSetCompressionOnOffSelect					= 0x004A,
	kVDGetCompressionTypesSelect					= 0x004B,
	kVDSetTimeBaseSelect							= 0x004C,
	kVDSetFrameRateSelect							= 0x004D,
	kVDGetDataRateSelect							= 0x004E,
	kVDGetSoundInputDriverSelect					= 0x004F,
	kVDGetDMADepthsSelect							= 0x0050,
	kVDGetPreferredTimeScaleSelect					= 0x0051,
	kVDReleaseAsyncBuffersSelect					= 0x0052,
	kVDSetDataRateSelect							= 0x0054,
	kVDGetTimeCodeSelect							= 0x0055,
	kVDUseSafeBuffersSelect							= 0x0056,
	kVDGetSoundInputSourceSelect					= 0x0057,
	kVDGetCompressionTimeSelect						= 0x0058,
	kVDSetPreferredPacketSizeSelect					= 0x0059,
	kVDSetPreferredImageDimensionsSelect			= 0x005A,
	kVDGetPreferredImageDimensionsSelect			= 0x005B,
	kVDGetInputNameSelect							= 0x005C,
	kVDSetDestinationPortSelect						= 0x005D,
	kSGInitializeSelect								= 0x0001,
	kSGSetDataOutputSelect							= 0x0002,
	kSGGetDataOutputSelect							= 0x0003,
	kSGSetGWorldSelect								= 0x0004,
	kSGGetGWorldSelect								= 0x0005,
	kSGNewChannelSelect								= 0x0006,
	kSGDisposeChannelSelect							= 0x0007,
	kSGStartPreviewSelect							= 0x0010,
	kSGStartRecordSelect							= 0x0011,
	kSGIdleSelect									= 0x0012,
	kSGStopSelect									= 0x0013,
	kSGPauseSelect									= 0x0014,
	kSGPrepareSelect								= 0x0015,
	kSGReleaseSelect								= 0x0016,
	kSGGetMovieSelect								= 0x0017,
	kSGSetMaximumRecordTimeSelect					= 0x0018,
	kSGGetMaximumRecordTimeSelect					= 0x0019,
	kSGGetStorageSpaceRemainingSelect				= 0x001A,
	kSGGetTimeRemainingSelect						= 0x001B,
	kSGGrabPictSelect								= 0x001C,
	kSGGetLastMovieResIDSelect						= 0x001D,
	kSGSetFlagsSelect								= 0x001E,
	kSGGetFlagsSelect								= 0x001F,
	kSGSetDataProcSelect							= 0x0020,
	kSGNewChannelFromComponentSelect				= 0x0021,
	kSGDisposeDeviceListSelect						= 0x0022,
	kSGAppendDeviceListToMenuSelect					= 0x0023,
	kSGSetSettingsSelect							= 0x0024,
	kSGGetSettingsSelect							= 0x0025,
	kSGGetIndChannelSelect							= 0x0026,
	kSGUpdateSelect									= 0x0027,
	kSGGetPauseSelect								= 0x0028,
	kSGSettingsDialogSelect							= 0x0029,
	kSGGetAlignmentProcSelect						= 0x002A,
	kSGSetChannelSettingsSelect						= 0x002B,
	kSGGetChannelSettingsSelect						= 0x002C,
	kSGGetModeSelect								= 0x002D,
	kSGSetDataRefSelect								= 0x002E,
	kSGGetDataRefSelect								= 0x002F,
	kSGNewOutputSelect								= 0x0030,
	kSGDisposeOutputSelect							= 0x0031,
	kSGSetOutputFlagsSelect							= 0x0032,
	kSGSetChannelOutputSelect						= 0x0033,
	kSGGetDataOutputStorageSpaceRemainingSelect		= 0x0034,
	kSGHandleUpdateEventSelect						= 0x0035,
	kSGSetOutputNextOutputSelect					= 0x0036,
	kSGGetOutputNextOutputSelect					= 0x0037,
	kSGSetOutputMaximumOffsetSelect					= 0x0038,
	kSGGetOutputMaximumOffsetSelect					= 0x0039,
	kSGGetOutputDataReferenceSelect					= 0x003A,
	kSGWriteExtendedMovieDataSelect					= 0x003B,
	kSGWriteMovieDataSelect							= 0x0100,
	kSGAddFrameReferenceSelect						= 0x0101,
	kSGGetNextFrameReferenceSelect					= 0x0102,
	kSGGetTimeBaseSelect							= 0x0103,
	kSGSortDeviceListSelect							= 0x0104,
	kSGAddMovieDataSelect							= 0x0105,
	kSGChangedSourceSelect							= 0x0106,
	kSGAddExtendedFrameReferenceSelect				= 0x0107,
	kSGGetNextExtendedFrameReferenceSelect			= 0x0108,
	kSGAddExtendedMovieDataSelect					= 0x0109,
	kSGAddOutputDataRefToMediaSelect				= 0x010A,
	kSGSetChannelUsageSelect						= 0x0080,
	kSGGetChannelUsageSelect						= 0x0081,
	kSGSetChannelBoundsSelect						= 0x0082,
	kSGGetChannelBoundsSelect						= 0x0083,
	kSGSetChannelVolumeSelect						= 0x0084,
	kSGGetChannelVolumeSelect						= 0x0085,
	kSGGetChannelInfoSelect							= 0x0086,
	kSGSetChannelPlayFlagsSelect					= 0x0087,
	kSGGetChannelPlayFlagsSelect					= 0x0088,
	kSGSetChannelMaxFramesSelect					= 0x0089,
	kSGGetChannelMaxFramesSelect					= 0x008A,
	kSGSetChannelRefConSelect						= 0x008B,
	kSGSetChannelClipSelect							= 0x008C,
	kSGGetChannelClipSelect							= 0x008D,
	kSGGetChannelSampleDescriptionSelect			= 0x008E,
	kSGGetChannelDeviceListSelect					= 0x008F,
	kSGSetChannelDeviceSelect						= 0x0090,
	kSGSetChannelMatrixSelect						= 0x0091,
	kSGGetChannelMatrixSelect						= 0x0092,
	kSGGetChannelTimeScaleSelect					= 0x0093,
	kSGChannelPutPictureSelect						= 0x0094,
	kSGChannelSetRequestedDataRateSelect			= 0x0095,
	kSGChannelGetRequestedDataRateSelect			= 0x0096,
	kSGChannelSetDataSourceNameSelect				= 0x0097,
	kSGChannelGetDataSourceNameSelect				= 0x0098,
	kSGInitChannelSelect							= 0x0180,
	kSGWriteSamplesSelect							= 0x0181,
	kSGGetDataRateSelect							= 0x0182,
	kSGAlignChannelRectSelect						= 0x0183,
	kSGPanelGetDitlSelect							= 0x0200,
	kSGPanelGetTitleSelect							= 0x0201,
	kSGPanelCanRunSelect							= 0x0202,
	kSGPanelInstallSelect							= 0x0203,
	kSGPanelEventSelect								= 0x0204,
	kSGPanelItemSelect								= 0x0205,
	kSGPanelRemoveSelect							= 0x0206,
	kSGPanelSetGrabberSelect						= 0x0207,
	kSGPanelSetResFileSelect						= 0x0208,
	kSGPanelGetSettingsSelect						= 0x0209,
	kSGPanelSetSettingsSelect						= 0x020A,
	kSGPanelValidateInputSelect						= 0x020B,
	kSGPanelSetEventFilterSelect					= 0x020C,
	kSGGetSrcVideoBoundsSelect						= 0x0100,
	kSGSetVideoRectSelect							= 0x0101,
	kSGGetVideoRectSelect							= 0x0102,
	kSGGetVideoCompressorTypeSelect					= 0x0103,
	kSGSetVideoCompressorTypeSelect					= 0x0104,
	kSGSetVideoCompressorSelect						= 0x0105,
	kSGGetVideoCompressorSelect						= 0x0106,
	kSGGetVideoDigitizerComponentSelect				= 0x0107,
	kSGSetVideoDigitizerComponentSelect				= 0x0108,
	kSGVideoDigitizerChangedSelect					= 0x0109,
	kSGSetVideoBottlenecksSelect					= 0x010A,
	kSGGetVideoBottlenecksSelect					= 0x010B,
	kSGGrabFrameSelect								= 0x010C,
	kSGGrabFrameCompleteSelect						= 0x010D,
	kSGDisplayFrameSelect							= 0x010E,
	kSGCompressFrameSelect							= 0x010F,
	kSGCompressFrameCompleteSelect					= 0x0110,
	kSGAddFrameSelect								= 0x0111,
	kSGTransferFrameForCompressSelect				= 0x0112,
	kSGSetCompressBufferSelect						= 0x0113,
	kSGGetCompressBufferSelect						= 0x0114,
	kSGGetBufferInfoSelect							= 0x0115,
	kSGSetUseScreenBufferSelect						= 0x0116,
	kSGGetUseScreenBufferSelect						= 0x0117,
	kSGGrabCompressCompleteSelect					= 0x0118,
	kSGDisplayCompressSelect						= 0x0119,
	kSGSetFrameRateSelect							= 0x011A,
	kSGGetFrameRateSelect							= 0x011B,
	kSGSetPreferredPacketSizeSelect					= 0x0121,
	kSGGetPreferredPacketSizeSelect					= 0x0122,
	kSGSetUserVideoCompressorListSelect				= 0x0123,
	kSGGetUserVideoCompressorListSelect				= 0x0124,
	kSGSetSoundInputDriverSelect					= 0x0100,
	kSGGetSoundInputDriverSelect					= 0x0101,
	kSGSoundInputDriverChangedSelect				= 0x0102,
	kSGSetSoundRecordChunkSizeSelect				= 0x0103,
	kSGGetSoundRecordChunkSizeSelect				= 0x0104,
	kSGSetSoundInputRateSelect						= 0x0105,
	kSGGetSoundInputRateSelect						= 0x0106,
	kSGSetSoundInputParametersSelect				= 0x0107,
	kSGGetSoundInputParametersSelect				= 0x0108,
	kSGSetAdditionalSoundRatesSelect				= 0x0109,
	kSGGetAdditionalSoundRatesSelect				= 0x010A,
	kSGSetFontNameSelect							= 0x0100,
	kSGSetFontSizeSelect							= 0x0101,
	kSGSetTextForeColorSelect						= 0x0102,
	kSGSetTextBackColorSelect						= 0x0103,
	kSGSetJustificationSelect						= 0x0104,
	kSGGetTextReturnToSpaceValueSelect				= 0x0105,
	kSGSetTextReturnToSpaceValueSelect				= 0x0106,
	kSGGetInstrumentSelect							= 0x0100,
	kSGSetInstrumentSelect							= 0x0101,
	kQTVideoOutputGetDisplayModeListSelect			= 0x0001,
	kQTVideoOutputGetCurrentClientNameSelect		= 0x0002,
	kQTVideoOutputSetClientNameSelect				= 0x0003,
	kQTVideoOutputGetClientNameSelect				= 0x0004,
	kQTVideoOutputBeginSelect						= 0x0005,
	kQTVideoOutputEndSelect							= 0x0006,
	kQTVideoOutputSetDisplayModeSelect				= 0x0007,
	kQTVideoOutputGetDisplayModeSelect				= 0x0008,
	kQTVideoOutputCustomConfigureDisplaySelect		= 0x0009,
	kQTVideoOutputSaveStateSelect					= 0x000A,
	kQTVideoOutputRestoreStateSelect				= 0x000B,
	kQTVideoOutputGetGWorldSelect					= 0x000C,
	kQTVideoOutputGetGWorldParametersSelect			= 0x000D,
	kQTVideoOutputGetIndSoundOutputSelect			= 0x000E,
	kQTVideoOutputGetClockSelect					= 0x000F,
	kQTVideoOutputSetEchoPortSelect					= 0x0010
};


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

#endif /* __QUICKTIMECOMPONENTS__ */

