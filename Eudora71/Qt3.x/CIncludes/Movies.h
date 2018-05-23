/*
 	File:		Movies.h
 
 	Contains:	QuickTime Interfaces.
 
 	Version:	Technology:	QuickTime 3.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __MOVIES__
#define __MOVIES__

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __ALIASES__
#include <Aliases.h>
#endif
#ifndef __EVENTS__
#include <Events.h>
#endif
#ifndef __MENUS__
#include <Menus.h>
#endif
#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __IMAGECOMPRESSION__
#include <ImageCompression.h>
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


/*  "kFix1" is defined in FixMath as "fixed1"  */
/* error codes are in Errors.[haa] */
/* gestalt codes are in Gestalt.[hpa] */

enum {
	MovieFileType				= FOUR_CHAR_CODE('MooV'),
	MovieScrapType				= FOUR_CHAR_CODE('moov')
};


enum {
	MovieResourceType			= FOUR_CHAR_CODE('moov'),
	MovieForwardPointerResourceType = FOUR_CHAR_CODE('fore'),
	MovieBackwardPointerResourceType = FOUR_CHAR_CODE('back')
};


enum {
	MovieResourceAtomType		= FOUR_CHAR_CODE('moov'),
	MovieDataAtomType			= FOUR_CHAR_CODE('mdat'),
	FreeAtomType				= FOUR_CHAR_CODE('free'),
	SkipAtomType				= FOUR_CHAR_CODE('skip')
};


enum {
	MediaHandlerType			= FOUR_CHAR_CODE('mhlr'),
	DataHandlerType				= FOUR_CHAR_CODE('dhlr')
};


enum {
	VideoMediaType				= FOUR_CHAR_CODE('vide'),
	SoundMediaType				= FOUR_CHAR_CODE('soun'),
	TextMediaType				= FOUR_CHAR_CODE('text'),
	BaseMediaType				= FOUR_CHAR_CODE('gnrc'),
	MPEGMediaType				= FOUR_CHAR_CODE('MPEG'),
	MusicMediaType				= FOUR_CHAR_CODE('musi'),
	TimeCodeMediaType			= FOUR_CHAR_CODE('tmcd'),
	SpriteMediaType				= FOUR_CHAR_CODE('sprt'),
	TweenMediaType				= FOUR_CHAR_CODE('twen'),
	ThreeDeeMediaType			= FOUR_CHAR_CODE('qd3d'),
	HandleDataHandlerSubType	= FOUR_CHAR_CODE('hndl'),
	ResourceDataHandlerSubType	= FOUR_CHAR_CODE('rsrc'),
	URLDataHandlerSubType		= FOUR_CHAR_CODE('url ')
};


enum {
	VisualMediaCharacteristic	= FOUR_CHAR_CODE('eyes'),
	AudioMediaCharacteristic	= FOUR_CHAR_CODE('ears'),
	kCharacteristicCanSendVideo	= FOUR_CHAR_CODE('vsnd'),
	kCharacteristicProvidesActions = FOUR_CHAR_CODE('actn'),
	kCharacteristicNonLinear	= FOUR_CHAR_CODE('nonl')
};


enum {
	kUserDataMovieControllerType = FOUR_CHAR_CODE('ctyp'),
	kUserDataName				= FOUR_CHAR_CODE('name'),
	kUserDataTextFullName		= FOUR_CHAR_CODE('©nam'),
	kUserDataTextCopyright		= FOUR_CHAR_CODE('©cpy'),
	kUserDataTextInformation	= FOUR_CHAR_CODE('©inf')
};


enum {
	DoTheRightThing				= 0
};



struct MovieRecord {
	long 							data[1];
};
typedef struct MovieRecord				MovieRecord;

typedef MovieRecord *					Movie;

struct TrackRecord {
	long 							data[1];
};
typedef struct TrackRecord				TrackRecord;

typedef TrackRecord *					Track;

struct MediaRecord {
	long 							data[1];
};
typedef struct MediaRecord				MediaRecord;

typedef MediaRecord *					Media;

struct UserDataRecord {
	long 							data[1];
};
typedef struct UserDataRecord			UserDataRecord;

typedef UserDataRecord *				UserData;

struct TrackEditStateRecord {
	long 							data[1];
};
typedef struct TrackEditStateRecord		TrackEditStateRecord;

typedef TrackEditStateRecord *			TrackEditState;

struct MovieEditStateRecord {
	long 							data[1];
};
typedef struct MovieEditStateRecord		MovieEditStateRecord;

typedef MovieEditStateRecord *			MovieEditState;

struct SpriteWorldRecord {
	long 							data[1];
};
typedef struct SpriteWorldRecord		SpriteWorldRecord;

typedef SpriteWorldRecord *				SpriteWorld;

struct SpriteRecord {
	long 							data[1];
};
typedef struct SpriteRecord				SpriteRecord;

typedef SpriteRecord *					Sprite;

struct QTTweenerRecord {
	long 							data[1];
};
typedef struct QTTweenerRecord			QTTweenerRecord;

typedef QTTweenerRecord *				QTTweener;

struct SampleDescription {
	long 							descSize;
	long 							dataFormat;
	long 							resvd1;
	short 							resvd2;
	short 							dataRefIndex;
};
typedef struct SampleDescription		SampleDescription;
typedef SampleDescription *				SampleDescriptionPtr;
typedef SampleDescriptionPtr *			SampleDescriptionHandle;

typedef Handle 							QTAtomContainer;
typedef long 							QTAtom;
typedef long 							QTAtomType;
typedef long 							QTAtomID;
/* QTFloatDouble is the 64-bit IEEE-754 standard*/
typedef Float64 						QTFloatDouble;
/* QTFloatSingle is the 32-bit IEEE-754 standard*/
typedef Float32 						QTFloatSingle;



struct SoundDescription {
	long 							descSize;					/* total size of SoundDescription including extra data */
	long 							dataFormat;					/* sound format */
	long 							resvd1;						/* reserved for apple use. set to zero */
	short 							resvd2;						/* reserved for apple use. set to zero */
	short 							dataRefIndex;
	short 							version;					/* which version is this data */
	short 							revlevel;					/* what version of that codec did this */
	long 							vendor;						/* whose  codec compressed this data */
	short 							numChannels;				/* number of channels of sound */
	short 							sampleSize;					/* number of bits per sample */
	short 							compressionID;				/* unused. set to zero. */
	short 							packetSize;					/* unused. set to zero. */
	UnsignedFixed 					sampleRate;					/* sample rate sound is captured at */
};
typedef struct SoundDescription			SoundDescription;
typedef SoundDescription *				SoundDescriptionPtr;
typedef SoundDescriptionPtr *			SoundDescriptionHandle;
/* version 1 of the SoundDescription record*/

struct SoundDescriptionV1 {
																/* original fields*/
	SoundDescription 				desc;
																/* fixed compression ratio information*/
	unsigned long 					samplesPerPacket;
	unsigned long 					bytesPerPacket;
	unsigned long 					bytesPerFrame;
	unsigned long 					bytesPerSample;
																/* additional atom based fields ([long size, long type, some data], repeat)*/
};
typedef struct SoundDescriptionV1		SoundDescriptionV1;
typedef SoundDescriptionV1 *			SoundDescriptionV1Ptr;
typedef SoundDescriptionV1Ptr *			SoundDescriptionV1Handle;

struct TextDescription {
	long 							descSize;					/* Total size of TextDescription*/
	long 							dataFormat;					/* 'text'*/

	long 							resvd1;
	short 							resvd2;
	short 							dataRefIndex;

	long 							displayFlags;				/* see enum below for flag values*/

	long 							textJustification;			/* Can be: teCenter,teFlush -Default,-Right,-Left*/

	RGBColor 						bgColor;					/* Background color*/

	Rect 							defaultTextBox;				/* Location to place the text within the track bounds*/
	ScrpSTElement 					defaultStyle;				/* Default style (struct defined in TextEdit.h)*/
	char 							defaultFontName[1];			/* Font Name (pascal string - struct extended to fit) */
};
typedef struct TextDescription			TextDescription;
typedef TextDescription *				TextDescriptionPtr;
typedef TextDescriptionPtr *			TextDescriptionHandle;

struct SpriteDescription {
	long 							descSize;					/* total size of SpriteDescription including extra data */
	long 							dataFormat;					/*  */
	long 							resvd1;						/* reserved for apple use */
	short 							resvd2;
	short 							dataRefIndex;
	long 							version;					/* which version is this data */
	OSType 							decompressorType;			/* which decompressor to use, 0 for no decompression */
	long 							sampleFlags;				/* how to interpret samples */
};
typedef struct SpriteDescription		SpriteDescription;
typedef SpriteDescription *				SpriteDescriptionPtr;
typedef SpriteDescriptionPtr *			SpriteDescriptionHandle;

struct ThreeDeeDescription {
	long 							descSize;					/* total size of ThreeDeeDescription including extra data */
	long 							dataFormat;					/*  */
	long 							resvd1;						/* reserved for apple use */
	short 							resvd2;
	short 							dataRefIndex;
	long 							version;					/* which version is this data */
	long 							rendererType;				/* which renderer to use, 0 for default */
	long 							decompressorType;			/* which decompressor to use, 0 for default */
};
typedef struct ThreeDeeDescription		ThreeDeeDescription;
typedef ThreeDeeDescription *			ThreeDeeDescriptionPtr;
typedef ThreeDeeDescriptionPtr *		ThreeDeeDescriptionHandle;

struct DataReferenceRecord {
	OSType 							dataRefType;
	Handle 							dataRef;
};
typedef struct DataReferenceRecord		DataReferenceRecord;

typedef DataReferenceRecord *			DataReferencePtr;
/*--------------------------
  Music Sample Description
--------------------------*/

struct MusicDescription {
	long 							descSize;
	long 							dataFormat;					/* 'musi' */

	long 							resvd1;
	short 							resvd2;
	short 							dataRefIndex;

	long 							musicFlags;
	unsigned long 					headerData[1];				/* variable size! */

};
typedef struct MusicDescription			MusicDescription;
typedef MusicDescription *				MusicDescriptionPtr;
typedef MusicDescriptionPtr *			MusicDescriptionHandle;

enum {
	kMusicFlagDontPlay2Soft		= 1L << 0,
	kMusicFlagDontSlaveToMovie	= 1L << 1
};



enum {
	dfDontDisplay				= 1 << 0,						/* Don't display the text*/
	dfDontAutoScale				= 1 << 1,						/* Don't scale text as track bounds grows or shrinks*/
	dfClipToTextBox				= 1 << 2,						/* Clip update to the textbox*/
	dfUseMovieBGColor			= 1 << 3,						/* Set text background to movie's background color*/
	dfShrinkTextBoxToFit		= 1 << 4,						/* Compute minimum box to fit the sample*/
	dfScrollIn					= 1 << 5,						/* Scroll text in until last of text is in view */
	dfScrollOut					= 1 << 6,						/* Scroll text out until last of text is gone (if both set, scroll in then out)*/
	dfHorizScroll				= 1 << 7,						/* Scroll text horizontally (otherwise it's vertical)*/
	dfReverseScroll				= 1 << 8,						/* vert: scroll down rather than up; horiz: scroll backwards (justfication dependent)*/
	dfContinuousScroll			= 1 << 9,						/* new samples cause previous samples to scroll out */
	dfFlowHoriz					= 1 << 10,						/* horiz scroll text flows in textbox rather than extend to right */
	dfContinuousKaraoke			= 1 << 11,						/* ignore begin offset, hilite everything up to the end offset(karaoke)*/
	dfDropShadow				= 1 << 12,						/* display text with a drop shadow */
	dfAntiAlias					= 1 << 13,						/* attempt to display text anti aliased*/
	dfKeyedText					= 1 << 14,						/* key the text over background*/
	dfInverseHilite				= 1 << 15,						/* Use inverse hiliting rather than using hilite color*/
	dfTextColorHilite			= 1 << 16						/* changes text color in place of hiliting. */
};


enum {
	searchTextDontGoToFoundTime	= 1L << 16,
	searchTextDontHiliteFoundText = 1L << 17,
	searchTextOneTrackOnly		= 1L << 18,
	searchTextEnabledTracksOnly	= 1L << 19
};


enum {
	k3DMediaRendererEntry		= FOUR_CHAR_CODE('rend'),
	k3DMediaRendererName		= FOUR_CHAR_CODE('name'),
	k3DMediaRendererCode		= FOUR_CHAR_CODE('rcod')
};

/* progress messages */

enum {
	movieProgressOpen			= 0,
	movieProgressUpdatePercent	= 1,
	movieProgressClose			= 2
};

/* progress operations */

enum {
	progressOpFlatten			= 1,
	progressOpInsertTrackSegment = 2,
	progressOpInsertMovieSegment = 3,
	progressOpPaste				= 4,
	progressOpAddMovieSelection	= 5,
	progressOpCopy				= 6,
	progressOpCut				= 7,
	progressOpLoadMovieIntoRam	= 8,
	progressOpLoadTrackIntoRam	= 9,
	progressOpLoadMediaIntoRam	= 10,
	progressOpImportMovie		= 11,
	progressOpExportMovie		= 12
};


enum {
	mediaQualityDraft			= 0x0000,
	mediaQualityNormal			= 0x0040,
	mediaQualityBetter			= 0x0080,
	mediaQualityBest			= 0x00C0
};

/*****
	Interactive Sprites Support
*****/

struct QTEventRecord {
	long 							version;
	OSType 							eventType;
	Point 							where;
	long 							flags;
};
typedef struct QTEventRecord			QTEventRecord;
typedef QTEventRecord *					QTEventRecordPtr;

struct QTAtomSpec {
	QTAtomContainer 				container;
	QTAtom 							atom;
};
typedef struct QTAtomSpec				QTAtomSpec;
typedef QTAtomSpec *					QTAtomSpecPtr;

struct ResolvedQTEventSpec {
	QTAtomSpec 						actionAtom;
	Track 							targetTrack;
	long 							targetRefCon;
};
typedef struct ResolvedQTEventSpec		ResolvedQTEventSpec;
typedef ResolvedQTEventSpec *			ResolvedQTEventSpecPtr;

/* action constants */

enum {
	kActionMovieSetVolume		= 1024,							/* (short movieVolume) */
	kActionMovieSetRate			= 1025,							/* (Fixed rate) */
	kActionMovieSetLoopingFlags	= 1026,							/* (long loopingFlags) */
	kActionMovieGoToTime		= 1027,							/* (TimeValue time) */
	kActionMovieGoToTimeByName	= 1028,							/* (Str255 timeName) */
	kActionMovieGoToBeginning	= 1029,							/* no params */
	kActionMovieGoToEnd			= 1030,							/* no params */
	kActionMovieStepForward		= 1031,							/* no params */
	kActionMovieStepBackward	= 1032,							/* no params */
	kActionMovieSetSelection	= 1033,							/* (TimeValue startTime, TimeValue endTime) */
	kActionMovieSetSelectionByName = 1034,						/* (Str255 startTimeName, Str255 endTimeName) */
	kActionMoviePlaySelection	= 1035,							/* (Boolean selectionOnly) */
	kActionMovieSetLanguage		= 1036,							/* (long language) */
	kActionMovieChanged			= 1037,							/* no params */
	kActionTrackSetVolume		= 2048,							/* (short volume) */
	kActionTrackSetBalance		= 2049,							/* (short balance) */
	kActionTrackSetEnabled		= 2050,							/* (Boolean enabled) */
	kActionTrackSetMatrix		= 2051,							/* (MatrixRecord matrix) */
	kActionTrackSetLayer		= 2052,							/* (short layer) */
	kActionTrackSetClip			= 2053,							/* (RgnHandle clip) */
	kActionSpriteSetMatrix		= 3072,							/* (MatrixRecord matrix) */
	kActionSpriteSetImageIndex	= 3073,							/* (short imageIndex) */
	kActionSpriteSetVisible		= 3074,							/* (short visible) */
	kActionSpriteSetLayer		= 3075,							/* (short layer) */
	kActionSpriteSetGraphicsMode = 3076,						/* (ModifierTrackGraphicsModeRecord graphicsMode) */
	kActionSpritePassMouseToCodec = 3078,						/* no params */
	kActionSpriteClickOnCodec	= 3079,							/* Point localLoc */
	kActionSpriteTranslate		= 3080,							/* (Fixed x, Fixed y, Boolean isAbsolute) */
	kActionSpriteScale			= 3081,							/* (Fixed xScale, Fixed yScale) */
	kActionSpriteRotate			= 3082,							/* (Fixed degrees) */
	kActionSpriteStretch		= 3083,							/* (Fixed p1x, Fixed p1y, Fixed p2x, Fixed p2y, Fixed p3x, Fixed p3y, Fixed p4x, Fixed p4y) */
	kActionQTVRSetPanAngle		= 4096,							/* (float panAngle) */
	kActionQTVRSetTiltAngle		= 4097,							/* (float tiltAngle) */
	kActionQTVRSetFieldOfView	= 4098,							/* (float fieldOfView) */
	kActionQTVRShowDefaultView	= 4099,							/* no params */
	kActionQTVRGoToNodeID		= 4100,							/* (UInt32 nodeID) */
	kActionMusicPlayNote		= 5120,							/* (long sampleDescIndex, long partNumber, long delay, long pitch, long velocity, long duration) */
	kActionMusicSetController	= 5121,							/* (long sampleDescIndex, long partNumber, long delay, long controller, long value) */
	kActionCase					= 6144,							/* [(CaseStatementActionAtoms)] */
	kActionWhile				= 6145,							/* [(WhileStatementActionAtoms)] */
	kActionGoToURL				= 6146,							/* (C string urlLink) */
	kActionSendQTEventToSprite	= 6147,							/* ([(SpriteTargetAtoms)], QTEventRecord theEvent) */
	kActionDebugStr				= 6148,							/* (Str255 theString) */
	kActionPushCurrentTime		= 6149,							/* no params */
	kActionPushCurrentTimeWithLabel = 6150,						/* (Str255 theLabel) */
	kActionPopAndGotoTopTime	= 6151,							/* no params */
	kActionPopAndGotoLabeledTime = 6152,						/* (Str255 theLabel) */
	kActionSpriteTrackSetVariable = 7168,						/* (QTAtomID variableID, float value) */
	kActionApplicationNumberAndString = 8192					/* (long aNumber, Str255 aString ) */
};


enum {
	kOperandExpression			= 1,
	kOperandConstant			= 2,
	kOperandMovieVolume			= 1024,
	kOperandMovieRate			= 1025,
	kOperandMovieIsLooping		= 1026,
	kOperandMovieLoopIsPalindrome = 1027,
	kOperandMovieTime			= 1028,
	kOperandTrackVolume			= 2048,
	kOperandTrackBalance		= 2049,
	kOperandTrackEnabled		= 2050,
	kOperandTrackLayer			= 2051,
	kOperandTrackWidth			= 2052,
	kOperandTrackHeight			= 2053,
	kOperandSpriteBoundsLeft	= 3072,
	kOperandSpriteBoundsTop		= 3073,
	kOperandSpriteBoundsRight	= 3074,
	kOperandSpriteBoundsBottom	= 3075,
	kOperandSpriteImageIndex	= 3076,
	kOperandSpriteVisible		= 3077,
	kOperandSpriteLayer			= 3078,
	kOperandSpriteTrackVariable	= 3079,							/* [QTAtomID variableID] */
	kOperandSpriteTrackNumSprites = 3080,
	kOperandSpriteTrackNumImages = 3081,
	kOperandSpriteID			= 3082,
	kOperandSpriteIndex			= 3083,
	kOperandSpriteFirstCornerX	= 3084,
	kOperandSpriteFirstCornerY	= 3085,
	kOperandSpriteSecondCornerX	= 3086,
	kOperandSpriteSecondCornerY	= 3087,
	kOperandSpriteThirdCornerX	= 3088,
	kOperandSpriteThirdCornerY	= 3089,
	kOperandSpriteFourthCornerX	= 3090,
	kOperandSpriteFourthCornerY	= 3091,
	kOperandSpriteImageRegistrationPointX = 3092,
	kOperandSpriteImageRegistrationPointY = 3093,
	kOperandQTVRPanAngle		= 4096,
	kOperandQTVRTiltAngle		= 4097,
	kOperandQTVRFieldOfView		= 4098,
	kOperandQTVRNodeID			= 4099,
	kOperandMouseLocalHLoc		= 5120,							/* [TargetAtoms aTrack] */
	kOperandMouseLocalVLoc		= 5121,							/* [TargetAtoms aTrack] */
	kOperandKeyIsDown			= 5122,							/* [short modKeys, char asciiValue] */
	kOperandRandom				= 5123							/* [short min, short max] */
};


enum {
	kFirstMovieAction			= kActionMovieSetVolume,
	kLastMovieAction			= kActionMovieChanged,
	kFirstTrackAction			= kActionTrackSetVolume,
	kLastTrackAction			= kActionTrackSetClip,
	kFirstSpriteAction			= kActionSpriteSetMatrix,
	kLastSpriteAction			= kActionSpriteStretch,
	kFirstQTVRAction			= kActionQTVRSetPanAngle,
	kLastQTVRAction				= kActionQTVRGoToNodeID,
	kFirstMusicAction			= kActionMusicPlayNote,
	kLastMusicAction			= kActionMusicSetController,
	kFirstSystemAction			= kActionCase,
	kLastSystemAction			= kActionPopAndGotoLabeledTime,
	kFirstSpriteTrackAction		= kActionSpriteTrackSetVariable,
	kLastSpriteTrackAction		= kActionSpriteTrackSetVariable,
	kFirstApplicationAction		= kActionApplicationNumberAndString,
	kLastApplicationAction		= kActionApplicationNumberAndString,
	kFirstAction				= kFirstMovieAction,
	kLastAction					= kActionApplicationNumberAndString
};


/* target atom types*/

enum {
	kTargetMovie				= FOUR_CHAR_CODE('moov'),		/* no data */
	kTargetTrackName			= FOUR_CHAR_CODE('trna'),		/* (PString trackName) */
	kTargetTrackID				= FOUR_CHAR_CODE('trid'),		/* (long trackID) */
	kTargetTrackType			= FOUR_CHAR_CODE('trty'),		/* (OSType trackType) */
	kTargetTrackIndex			= FOUR_CHAR_CODE('trin'),		/* (long trackIndex) */
	kTargetSpriteName			= FOUR_CHAR_CODE('spna'),		/* (PString spriteName) */
	kTargetSpriteID				= FOUR_CHAR_CODE('spid'),		/* (QTAtomID spriteID) */
	kTargetSpriteIndex			= FOUR_CHAR_CODE('spin')		/* (short spriteIndex) */
};

/* action container atom types*/

enum {
	kQTEventType				= FOUR_CHAR_CODE('evnt'),
	kAction						= FOUR_CHAR_CODE('actn'),
	kWhichAction				= FOUR_CHAR_CODE('whic'),
	kActionParameter			= FOUR_CHAR_CODE('parm'),
	kActionTarget				= FOUR_CHAR_CODE('targ'),
	kActionFlags				= FOUR_CHAR_CODE('flag'),
	kActionParameterMinValue	= FOUR_CHAR_CODE('minv'),
	kActionParameterMaxValue	= FOUR_CHAR_CODE('maxv'),
	kActionListAtomType			= FOUR_CHAR_CODE('list'),
	kExpressionContainerAtomType = FOUR_CHAR_CODE('expr'),
	kConditionalAtomType		= FOUR_CHAR_CODE('test'),
	kOperatorAtomType			= FOUR_CHAR_CODE('oper'),
	kOperandAtomType			= FOUR_CHAR_CODE('oprn'),
	kCommentAtomType			= FOUR_CHAR_CODE('why ')
};

/* QTEvent types */

enum {
	kQTEventMouseClick			= FOUR_CHAR_CODE('clik'),
	kQTEventMouseClickEnd		= FOUR_CHAR_CODE('cend'),
	kQTEventMouseClickEndTriggerButton = FOUR_CHAR_CODE('trig'),
	kQTEventMouseEnter			= FOUR_CHAR_CODE('entr'),
	kQTEventMouseExit			= FOUR_CHAR_CODE('exit'),
	kQTEventFrameLoaded			= FOUR_CHAR_CODE('fram'),
	kQTEventIdle				= FOUR_CHAR_CODE('idle'),
	kQTEventRequestToModifyMovie = FOUR_CHAR_CODE('reqm')
};

/* flags for the kActionFlags atom */

enum {
	kActionFlagActionIsDelta	= 1L << 1,
	kActionFlagParameterWrapsAround = 1L << 2,
	kActionFlagActionIsToggle	= 1L << 3
};

/* constants for kOperatorAtomType IDs (operator types)*/

enum {
	kOperatorAdd				= FOUR_CHAR_CODE('add '),
	kOperatorSubtract			= FOUR_CHAR_CODE('sub '),
	kOperatorMultiply			= FOUR_CHAR_CODE('mult'),
	kOperatorDivide				= FOUR_CHAR_CODE('div '),
	kOperatorOr					= FOUR_CHAR_CODE('or  '),
	kOperatorAnd				= FOUR_CHAR_CODE('and '),
	kOperatorNot				= FOUR_CHAR_CODE('not '),
	kOperatorLessThan			= FOUR_CHAR_CODE('<   '),
	kOperatorLessThanEqualTo	= FOUR_CHAR_CODE('<=  '),
	kOperatorEqualTo			= FOUR_CHAR_CODE('=   '),
	kOperatorNotEqualTo			= FOUR_CHAR_CODE('!=  '),
	kOperatorGreaterThan		= FOUR_CHAR_CODE('>   '),
	kOperatorGreaterThanEqualTo	= FOUR_CHAR_CODE('>=  '),
	kOperatorModulo				= FOUR_CHAR_CODE('mod '),
	kOperatorIntegerDivide		= FOUR_CHAR_CODE('idiv'),
	kOperatorAbsoluteValue		= FOUR_CHAR_CODE('abs '),
	kOperatorNegate				= FOUR_CHAR_CODE('neg ')
};

/* constants for MediaPropertiesAtom*/

enum {
	kMediaPropertyNonLinearAtomType = FOUR_CHAR_CODE('nonl'),
	kMediaPropertyHasActions	= 105
};

typedef CALLBACK_API( OSErr , MovieRgnCoverProcPtr )(Movie theMovie, RgnHandle changedRgn, long refcon);
typedef CALLBACK_API( OSErr , MovieProgressProcPtr )(Movie theMovie, short message, short whatOperation, Fixed percentDone, long refcon);
typedef CALLBACK_API( OSErr , MovieDrawingCompleteProcPtr )(Movie theMovie, long refCon);
typedef CALLBACK_API( OSErr , TrackTransferProcPtr )(Track t, long refCon);
typedef CALLBACK_API( OSErr , GetMovieProcPtr )(long offset, long size, void *dataPtr, void *refCon);
typedef CALLBACK_API( Boolean , MoviePreviewCallOutProcPtr )(long refcon);
typedef CALLBACK_API( OSErr , TextMediaProcPtr )(Handle theText, Movie theMovie, short *displayFlag, long refcon);
typedef CALLBACK_API( OSErr , ActionsProcPtr )(void *refcon, Track targetTrack, long targetRefCon, QTEventRecordPtr theEvent);
typedef CALLBACK_API( void , MoviesErrorProcPtr )(OSErr theErr, long refcon);
typedef STACK_UPP_TYPE(MoviesErrorProcPtr) 						MoviesErrorUPP;
enum { uppMoviesErrorProcInfo = 0x00000380 }; 					/* pascal no_return_value Func(2_bytes, 4_bytes) */
#define NewMoviesErrorProc(userRoutine) 						(MoviesErrorUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMoviesErrorProcInfo, GetCurrentArchitecture())
#define CallMoviesErrorProc(userRoutine, theErr, refcon) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppMoviesErrorProcInfo, (theErr), (refcon))
typedef STACK_UPP_TYPE(MovieRgnCoverProcPtr) 					MovieRgnCoverUPP;
typedef STACK_UPP_TYPE(MovieProgressProcPtr) 					MovieProgressUPP;
typedef STACK_UPP_TYPE(MovieDrawingCompleteProcPtr) 			MovieDrawingCompleteUPP;
typedef STACK_UPP_TYPE(TrackTransferProcPtr) 					TrackTransferUPP;
typedef STACK_UPP_TYPE(GetMovieProcPtr) 						GetMovieUPP;
typedef STACK_UPP_TYPE(MoviePreviewCallOutProcPtr) 				MoviePreviewCallOutUPP;
typedef STACK_UPP_TYPE(TextMediaProcPtr) 						TextMediaUPP;
typedef STACK_UPP_TYPE(ActionsProcPtr) 							ActionsUPP;

typedef ComponentInstance 				MediaHandler;
typedef ComponentInstance 				DataHandler;
typedef Component 						MediaHandlerComponent;
typedef Component 						DataHandlerComponent;
typedef ComponentResult 				HandlerError;
/* TimeBase equates */
typedef long 							TimeValue;
typedef long 							TimeScale;
typedef wide 							CompTimeValue;

enum {
	loopTimeBase				= 1,
	palindromeLoopTimeBase		= 2,
	maintainTimeBaseZero		= 4
};

typedef unsigned long 					TimeBaseFlags;

struct TimeBaseRecord {
	long 							data[1];
};
typedef struct TimeBaseRecord			TimeBaseRecord;

typedef TimeBaseRecord *				TimeBase;

struct CallBackRecord {
	long 							data[1];
};
typedef struct CallBackRecord			CallBackRecord;

typedef CallBackRecord *				QTCallBack;

struct TimeRecord {
	CompTimeValue 					value;						/* units */
	TimeScale 						scale;						/* units per second */
	TimeBase 						base;
};
typedef struct TimeRecord				TimeRecord;
/* CallBack equates */

enum {
	triggerTimeFwd				= 0x0001,						/* when curTime exceeds triggerTime going forward */
	triggerTimeBwd				= 0x0002,						/* when curTime exceeds triggerTime going backwards */
	triggerTimeEither			= 0x0003,						/* when curTime exceeds triggerTime going either direction */
	triggerRateLT				= 0x0004,						/* when rate changes to less than trigger value */
	triggerRateGT				= 0x0008,						/* when rate changes to greater than trigger value */
	triggerRateEqual			= 0x0010,						/* when rate changes to equal trigger value */
	triggerRateLTE				= triggerRateLT | triggerRateEqual,
	triggerRateGTE				= triggerRateGT | triggerRateEqual,
	triggerRateNotEqual			= triggerRateGT | triggerRateEqual | triggerRateLT,
	triggerRateChange			= 0,
	triggerAtStart				= 0x0001,
	triggerAtStop				= 0x0002
};


typedef unsigned short 					QTCallBackFlags;

enum {
	timeBaseBeforeStartTime		= 1,
	timeBaseAfterStopTime		= 2
};

typedef unsigned long 					TimeBaseStatus;

enum {
	callBackAtTime				= 1,
	callBackAtRate				= 2,
	callBackAtTimeJump			= 3,
	callBackAtExtremes			= 4,
	callBackAtInterrupt			= 0x8000,
	callBackAtDeferredTask		= 0x4000
};

typedef unsigned short 					QTCallBackType;
typedef CALLBACK_API( void , QTCallBackProcPtr )(QTCallBack cb, long refCon);
typedef STACK_UPP_TYPE(QTCallBackProcPtr) 						QTCallBackUPP;

enum {
	qtcbNeedsRateChanges		= 1,							/* wants to know about rate changes */
	qtcbNeedsTimeChanges		= 2,							/* wants to know about time changes */
	qtcbNeedsStartStopChanges	= 4								/* wants to know when TimeBase start/stop is changed*/
};


struct QTCallBackHeader {
	long 							callBackFlags;
	long 							reserved1;
	SInt8 							qtPrivate[40];
};
typedef struct QTCallBackHeader			QTCallBackHeader;
typedef CALLBACK_API( void , QTSyncTaskProcPtr )(void *task);
typedef STACK_UPP_TYPE(QTSyncTaskProcPtr) 						QTSyncTaskUPP;

struct QTSyncTaskRecord {
	void *							qLink;
	QTSyncTaskUPP 					proc;
};
typedef struct QTSyncTaskRecord			QTSyncTaskRecord;

typedef QTSyncTaskRecord *				QTSyncTaskPtr;

enum {
	keepInRam					= 1 << 0,						/* load and make non-purgable*/
	unkeepInRam					= 1 << 1,						/* mark as purgable*/
	flushFromRam				= 1 << 2,						/* empty those handles*/
	loadForwardTrackEdits		= 1 << 3,						/*	load track edits into ram for playing forward*/
	loadBackwardTrackEdits		= 1 << 4						/*	load track edits into ram for playing in reverse*/
};


enum {
	newMovieActive				= 1 << 0,
	newMovieDontResolveDataRefs	= 1 << 1,
	newMovieDontAskUnresolvedDataRefs = 1 << 2,
	newMovieDontAutoAlternates	= 1 << 3,
	newMovieDontUpdateForeBackPointers = 1 << 4
};

/* track usage bits */

enum {
	trackUsageInMovie			= 1 << 1,
	trackUsageInPreview			= 1 << 2,
	trackUsageInPoster			= 1 << 3
};

/* Add/GetMediaSample flags */

enum {
	mediaSampleNotSync			= 1 << 0,						/* sample is not a sync sample (eg. is frame differenced */
	mediaSampleShadowSync		= 1 << 1						/* sample is a shadow sync */
};


enum {
	pasteInParallel				= 1 << 0,
	showUserSettingsDialog		= 1 << 1,
	movieToFileOnlyExport		= 1 << 2,
	movieFileSpecValid			= 1 << 3
};


enum {
	nextTimeMediaSample			= 1 << 0,
	nextTimeMediaEdit			= 1 << 1,
	nextTimeTrackEdit			= 1 << 2,
	nextTimeSyncSample			= 1 << 3,
	nextTimeStep				= 1 << 4,
	nextTimeEdgeOK				= 1 << 14,
	nextTimeIgnoreActiveSegment	= 1 << 15
};

typedef unsigned short 					nextTimeFlagsEnum;

enum {
	createMovieFileDeleteCurFile = 1L << 31,
	createMovieFileDontCreateMovie = 1L << 30,
	createMovieFileDontOpenFile	= 1L << 29,
	createMovieFileDontCreateResFile = 1L << 28
};

typedef unsigned long 					createMovieFileFlagsEnum;

enum {
	flattenAddMovieToDataFork	= 1L << 0,
	flattenActiveTracksOnly		= 1L << 2,
	flattenDontInterleaveFlatten = 1L << 3,
	flattenFSSpecPtrIsDataRefRecordPtr = 1L << 4,
	flattenCompressMovieResource = 1L << 5,
	flattenForceMovieResourceBeforeMovieData = 1L << 6
};

typedef unsigned long 					movieFlattenFlagsEnum;

enum {
	movieInDataForkResID		= -1							/* magic res ID */
};


enum {
	mcTopLeftMovie				= 1 << 0,						/* usually centered */
	mcScaleMovieToFit			= 1 << 1,						/* usually only scales down */
	mcWithBadge					= 1 << 2,						/* give me a badge */
	mcNotVisible				= 1 << 3,						/* don't show controller */
	mcWithFrame					= 1 << 4						/* gimme a frame */
};


enum {
	movieScrapDontZeroScrap		= 1 << 0,
	movieScrapOnlyPutMovie		= 1 << 1
};


enum {
	dataRefSelfReference		= 1 << 0,
	dataRefWasNotResolved		= 1 << 1
};

typedef unsigned long 					dataRefAttributesFlags;

enum {
	hintsScrubMode				= 1 << 0,						/* mask == && (if flags == scrub on, flags != scrub off) */
	hintsLoop					= 1 << 1,
	hintsDontPurge				= 1 << 2,
	hintsUseScreenBuffer		= 1 << 5,
	hintsAllowInterlace			= 1 << 6,
	hintsUseSoundInterp			= 1 << 7,
	hintsHighQuality			= 1 << 8,						/* slooooow */
	hintsPalindrome				= 1 << 9,
	hintsInactive				= 1 << 11,
	hintsOffscreen				= 1 << 12,
	hintsDontDraw				= 1 << 13,
	hintsAllowBlacklining		= 1 << 14
};

typedef unsigned long 					playHintsEnum;

enum {
	mediaHandlerFlagBaseClient	= 1
};

typedef unsigned long 					mediaHandlerFlagsEnum;

enum {
	movieTrackMediaType			= 1 << 0,
	movieTrackCharacteristic	= 1 << 1,
	movieTrackEnabledOnly		= 1 << 2
};


struct SampleReferenceRecord {
	long 							dataOffset;
	long 							dataSize;
	TimeValue 						durationPerSample;
	long 							numberOfSamples;
	short 							sampleFlags;
};
typedef struct SampleReferenceRecord	SampleReferenceRecord;

typedef SampleReferenceRecord *			SampleReferencePtr;

/*************************
* Initialization Routines 
**************************/
EXTERN_API( void )
CheckQuickTimeRegistration		(void *					param1,
								 long 					param2)								THREEWORDINLINE(0x303C, 0x02DA, 0xAAAA);

EXTERN_API( OSErr )
EnterMovies						(void)														TWOWORDINLINE(0x7001, 0xAAAA);

EXTERN_API( void )
ExitMovies						(void)														TWOWORDINLINE(0x7002, 0xAAAA);

/*************************
* Error Routines 
**************************/

EXTERN_API( OSErr )
GetMoviesError					(void)														TWOWORDINLINE(0x7003, 0xAAAA);

EXTERN_API( void )
ClearMoviesStickyError			(void)														THREEWORDINLINE(0x303C, 0x00DE, 0xAAAA);

EXTERN_API( OSErr )
GetMoviesStickyError			(void)														TWOWORDINLINE(0x7004, 0xAAAA);

EXTERN_API( void )
SetMoviesErrorProc				(MoviesErrorUPP 		errProc,
								 long 					refcon)								THREEWORDINLINE(0x303C, 0x00EF, 0xAAAA);

/*************************
* Idle Routines 
**************************/
EXTERN_API( void )
MoviesTask						(Movie 					theMovie,
								 long 					maxMilliSecToUse)					TWOWORDINLINE(0x7005, 0xAAAA);

EXTERN_API( OSErr )
PrerollMovie					(Movie 					theMovie,
								 TimeValue 				time,
								 Fixed 					Rate)								TWOWORDINLINE(0x7006, 0xAAAA);

EXTERN_API( OSErr )
LoadMovieIntoRam				(Movie 					theMovie,
								 TimeValue 				time,
								 TimeValue 				duration,
								 long 					flags)								TWOWORDINLINE(0x7007, 0xAAAA);

EXTERN_API( OSErr )
LoadTrackIntoRam				(Track 					theTrack,
								 TimeValue 				time,
								 TimeValue 				duration,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x016E, 0xAAAA);

EXTERN_API( OSErr )
LoadMediaIntoRam				(Media 					theMedia,
								 TimeValue 				time,
								 TimeValue 				duration,
								 long 					flags)								TWOWORDINLINE(0x7008, 0xAAAA);

EXTERN_API( void )
SetMovieActive					(Movie 					theMovie,
								 Boolean 				active)								TWOWORDINLINE(0x7009, 0xAAAA);

EXTERN_API( Boolean )
GetMovieActive					(Movie 					theMovie)							TWOWORDINLINE(0x700A, 0xAAAA);

/*************************
* calls for playing movies, previews, posters
**************************/
EXTERN_API( void )
StartMovie						(Movie 					theMovie)							TWOWORDINLINE(0x700B, 0xAAAA);

EXTERN_API( void )
StopMovie						(Movie 					theMovie)							TWOWORDINLINE(0x700C, 0xAAAA);

EXTERN_API( void )
GoToBeginningOfMovie			(Movie 					theMovie)							TWOWORDINLINE(0x700D, 0xAAAA);

EXTERN_API( void )
GoToEndOfMovie					(Movie 					theMovie)							TWOWORDINLINE(0x700E, 0xAAAA);

EXTERN_API( Boolean )
IsMovieDone						(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00DD, 0xAAAA);

EXTERN_API( Boolean )
GetMoviePreviewMode				(Movie 					theMovie)							TWOWORDINLINE(0x700F, 0xAAAA);

EXTERN_API( void )
SetMoviePreviewMode				(Movie 					theMovie,
								 Boolean 				usePreview)							TWOWORDINLINE(0x7010, 0xAAAA);

EXTERN_API( void )
ShowMoviePoster					(Movie 					theMovie)							TWOWORDINLINE(0x7011, 0xAAAA);

EXTERN_API( void )
PlayMoviePreview				(Movie 					theMovie,
								 MoviePreviewCallOutUPP  callOutProc,
								 long 					refcon)								THREEWORDINLINE(0x303C, 0x00F2, 0xAAAA);

/*************************
* calls for controlling movies & tracks which are playing
**************************/
EXTERN_API( TimeBase )
GetMovieTimeBase				(Movie 					theMovie)							TWOWORDINLINE(0x7012, 0xAAAA);

EXTERN_API( void )
SetMovieMasterTimeBase			(Movie 					theMovie,
								 TimeBase 				tb,
								 const TimeRecord *		slaveZero)							THREEWORDINLINE(0x303C, 0x0167, 0xAAAA);

EXTERN_API( void )
SetMovieMasterClock				(Movie 					theMovie,
								 Component 				clockMeister,
								 const TimeRecord *		slaveZero)							THREEWORDINLINE(0x303C, 0x0168, 0xAAAA);

EXTERN_API( void )
GetMovieGWorld					(Movie 					theMovie,
								 CGrafPtr *				port,
								 GDHandle *				gdh)								TWOWORDINLINE(0x7015, 0xAAAA);

EXTERN_API( void )
SetMovieGWorld					(Movie 					theMovie,
								 CGrafPtr 				port,
								 GDHandle 				gdh)								TWOWORDINLINE(0x7016, 0xAAAA);


enum {
	movieDrawingCallWhenChanged	= 0,
	movieDrawingCallAlways		= 1
};

EXTERN_API( void )
SetMovieDrawingCompleteProc		(Movie 					theMovie,
								 long 					flags,
								 MovieDrawingCompleteUPP  proc,
								 long 					refCon)								THREEWORDINLINE(0x303C, 0x01DE, 0xAAAA);


EXTERN_API( void )
GetMovieNaturalBoundsRect		(Movie 					theMovie,
								 Rect *					naturalBounds)						THREEWORDINLINE(0x303C, 0x022C, 0xAAAA);

EXTERN_API( Track )
GetNextTrackForCompositing		(Movie 					theMovie,
								 Track 					theTrack)							THREEWORDINLINE(0x303C, 0x01FA, 0xAAAA);

EXTERN_API( Track )
GetPrevTrackForCompositing		(Movie 					theMovie,
								 Track 					theTrack)							THREEWORDINLINE(0x303C, 0x01FB, 0xAAAA);

EXTERN_API( void )
SetTrackGWorld					(Track 					theTrack,
								 CGrafPtr 				port,
								 GDHandle 				gdh,
								 TrackTransferUPP 		proc,
								 long 					refCon)								THREEWORDINLINE(0x303C, 0x009D, 0xAAAA);

EXTERN_API( PicHandle )
GetMoviePict					(Movie 					theMovie,
								 TimeValue 				time)								TWOWORDINLINE(0x701D, 0xAAAA);

EXTERN_API( PicHandle )
GetTrackPict					(Track 					theTrack,
								 TimeValue 				time)								TWOWORDINLINE(0x701E, 0xAAAA);

EXTERN_API( PicHandle )
GetMoviePosterPict				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00F7, 0xAAAA);

/* called between Begin & EndUpdate */
EXTERN_API( OSErr )
UpdateMovie						(Movie 					theMovie)							TWOWORDINLINE(0x701F, 0xAAAA);

EXTERN_API( OSErr )
InvalidateMovieRegion			(Movie 					theMovie,
								 RgnHandle 				invalidRgn)							THREEWORDINLINE(0x303C, 0x022A, 0xAAAA);

/**** spatial movie routines ****/
EXTERN_API( void )
GetMovieBox						(Movie 					theMovie,
								 Rect *					boxRect)							THREEWORDINLINE(0x303C, 0x00F9, 0xAAAA);

EXTERN_API( void )
SetMovieBox						(Movie 					theMovie,
								 const Rect *			boxRect)							THREEWORDINLINE(0x303C, 0x00FA, 0xAAAA);

/** movie display clip */
EXTERN_API( RgnHandle )
GetMovieDisplayClipRgn			(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00FC, 0xAAAA);

EXTERN_API( void )
SetMovieDisplayClipRgn			(Movie 					theMovie,
								 RgnHandle 				theClip)							THREEWORDINLINE(0x303C, 0x00FD, 0xAAAA);

/** movie src clip */
EXTERN_API( RgnHandle )
GetMovieClipRgn					(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x0100, 0xAAAA);

EXTERN_API( void )
SetMovieClipRgn					(Movie 					theMovie,
								 RgnHandle 				theClip)							THREEWORDINLINE(0x303C, 0x0101, 0xAAAA);

/** track src clip */
EXTERN_API( RgnHandle )
GetTrackClipRgn					(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0102, 0xAAAA);

EXTERN_API( void )
SetTrackClipRgn					(Track 					theTrack,
								 RgnHandle 				theClip)							THREEWORDINLINE(0x303C, 0x0103, 0xAAAA);

/** bounds in display space (not clipped by display clip) */
EXTERN_API( RgnHandle )
GetMovieDisplayBoundsRgn		(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00FB, 0xAAAA);

EXTERN_API( RgnHandle )
GetTrackDisplayBoundsRgn		(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0112, 0xAAAA);

/** bounds in movie space */
EXTERN_API( RgnHandle )
GetMovieBoundsRgn				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00FE, 0xAAAA);

EXTERN_API( RgnHandle )
GetTrackMovieBoundsRgn			(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x00FF, 0xAAAA);

/** bounds in track space */
EXTERN_API( RgnHandle )
GetTrackBoundsRgn				(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0111, 0xAAAA);

/** mattes - always in track space */
EXTERN_API( PixMapHandle )
GetTrackMatte					(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0115, 0xAAAA);

EXTERN_API( void )
SetTrackMatte					(Track 					theTrack,
								 PixMapHandle 			theMatte)							THREEWORDINLINE(0x303C, 0x0116, 0xAAAA);

EXTERN_API( void )
DisposeMatte					(PixMapHandle 			theMatte)							THREEWORDINLINE(0x303C, 0x014A, 0xAAAA);

/*************************
* calls for getting/saving movies
**************************/
EXTERN_API( Movie )
NewMovie						(long 					flags)								THREEWORDINLINE(0x303C, 0x0187, 0xAAAA);

EXTERN_API( OSErr )
PutMovieIntoHandle				(Movie 					theMovie,
								 Handle 				publicMovie)						TWOWORDINLINE(0x7022, 0xAAAA);

EXTERN_API( OSErr )
PutMovieIntoDataFork			(Movie 					theMovie,
								 short 					fRefNum,
								 long 					offset,
								 long 					maxSize)							THREEWORDINLINE(0x303C, 0x01B4, 0xAAAA);

EXTERN_API( void )
DisposeMovie					(Movie 					theMovie)							TWOWORDINLINE(0x7023, 0xAAAA);

/*************************
* Movie State Routines
**************************/
EXTERN_API( unsigned long )
GetMovieCreationTime			(Movie 					theMovie)							TWOWORDINLINE(0x7026, 0xAAAA);

EXTERN_API( unsigned long )
GetMovieModificationTime		(Movie 					theMovie)							TWOWORDINLINE(0x7027, 0xAAAA);

EXTERN_API( TimeScale )
GetMovieTimeScale				(Movie 					theMovie)							TWOWORDINLINE(0x7029, 0xAAAA);

EXTERN_API( void )
SetMovieTimeScale				(Movie 					theMovie,
								 TimeScale 				timeScale)							TWOWORDINLINE(0x702A, 0xAAAA);

EXTERN_API( TimeValue )
GetMovieDuration				(Movie 					theMovie)							TWOWORDINLINE(0x702B, 0xAAAA);

EXTERN_API( Fixed )
GetMovieRate					(Movie 					theMovie)							TWOWORDINLINE(0x702C, 0xAAAA);

EXTERN_API( void )
SetMovieRate					(Movie 					theMovie,
								 Fixed 					rate)								TWOWORDINLINE(0x702D, 0xAAAA);

EXTERN_API( Fixed )
GetMoviePreferredRate			(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00F3, 0xAAAA);

EXTERN_API( void )
SetMoviePreferredRate			(Movie 					theMovie,
								 Fixed 					rate)								THREEWORDINLINE(0x303C, 0x00F4, 0xAAAA);

EXTERN_API( short )
GetMoviePreferredVolume			(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00F5, 0xAAAA);

EXTERN_API( void )
SetMoviePreferredVolume			(Movie 					theMovie,
								 short 					volume)								THREEWORDINLINE(0x303C, 0x00F6, 0xAAAA);

EXTERN_API( short )
GetMovieVolume					(Movie 					theMovie)							TWOWORDINLINE(0x702E, 0xAAAA);

EXTERN_API( void )
SetMovieVolume					(Movie 					theMovie,
								 short 					volume)								TWOWORDINLINE(0x702F, 0xAAAA);

EXTERN_API( void )
GetMovieMatrix					(Movie 					theMovie,
								 MatrixRecord *			matrix)								TWOWORDINLINE(0x7031, 0xAAAA);

EXTERN_API( void )
SetMovieMatrix					(Movie 					theMovie,
								 const MatrixRecord *	matrix)								TWOWORDINLINE(0x7032, 0xAAAA);

EXTERN_API( void )
GetMoviePreviewTime				(Movie 					theMovie,
								 TimeValue *			previewTime,
								 TimeValue *			previewDuration)					TWOWORDINLINE(0x7033, 0xAAAA);

EXTERN_API( void )
SetMoviePreviewTime				(Movie 					theMovie,
								 TimeValue 				previewTime,
								 TimeValue 				previewDuration)					TWOWORDINLINE(0x7034, 0xAAAA);

EXTERN_API( TimeValue )
GetMoviePosterTime				(Movie 					theMovie)							TWOWORDINLINE(0x7035, 0xAAAA);

EXTERN_API( void )
SetMoviePosterTime				(Movie 					theMovie,
								 TimeValue 				posterTime)							TWOWORDINLINE(0x7036, 0xAAAA);

EXTERN_API( void )
GetMovieSelection				(Movie 					theMovie,
								 TimeValue *			selectionTime,
								 TimeValue *			selectionDuration)					TWOWORDINLINE(0x7037, 0xAAAA);

EXTERN_API( void )
SetMovieSelection				(Movie 					theMovie,
								 TimeValue 				selectionTime,
								 TimeValue 				selectionDuration)					TWOWORDINLINE(0x7038, 0xAAAA);

EXTERN_API( void )
SetMovieActiveSegment			(Movie 					theMovie,
								 TimeValue 				startTime,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x015C, 0xAAAA);

EXTERN_API( void )
GetMovieActiveSegment			(Movie 					theMovie,
								 TimeValue *			startTime,
								 TimeValue *			duration)							THREEWORDINLINE(0x303C, 0x015D, 0xAAAA);

EXTERN_API( TimeValue )
GetMovieTime					(Movie 					theMovie,
								 TimeRecord *			currentTime)						TWOWORDINLINE(0x7039, 0xAAAA);

EXTERN_API( void )
SetMovieTime					(Movie 					theMovie,
								 const TimeRecord *		newtime)							TWOWORDINLINE(0x703C, 0xAAAA);

EXTERN_API( void )
SetMovieTimeValue				(Movie 					theMovie,
								 TimeValue 				newtime)							TWOWORDINLINE(0x703D, 0xAAAA);


EXTERN_API( UserData )
GetMovieUserData				(Movie 					theMovie)							TWOWORDINLINE(0x703E, 0xAAAA);


/*************************
* Track/Media finding routines
**************************/
EXTERN_API( long )
GetMovieTrackCount				(Movie 					theMovie)							TWOWORDINLINE(0x703F, 0xAAAA);

EXTERN_API( Track )
GetMovieTrack					(Movie 					theMovie,
								 long 					trackID)							TWOWORDINLINE(0x7040, 0xAAAA);

EXTERN_API( Track )
GetMovieIndTrack				(Movie 					theMovie,
								 long 					index)								THREEWORDINLINE(0x303C, 0x0117, 0xAAAA);

EXTERN_API( Track )
GetMovieIndTrackType			(Movie 					theMovie,
								 long 					index,
								 OSType 				trackType,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x0208, 0xAAAA);

EXTERN_API( long )
GetTrackID						(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0127, 0xAAAA);

EXTERN_API( Movie )
GetTrackMovie					(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x00D0, 0xAAAA);

/*************************
* Track creation routines
**************************/
EXTERN_API( Track )
NewMovieTrack					(Movie 					theMovie,
								 Fixed 					width,
								 Fixed 					height,
								 short 					trackVolume)						THREEWORDINLINE(0x303C, 0x0188, 0xAAAA);

EXTERN_API( void )
DisposeMovieTrack				(Track 					theTrack)							TWOWORDINLINE(0x7042, 0xAAAA);

/*************************
* Track State routines
**************************/
EXTERN_API( unsigned long )
GetTrackCreationTime			(Track 					theTrack)							TWOWORDINLINE(0x7043, 0xAAAA);

EXTERN_API( unsigned long )
GetTrackModificationTime		(Track 					theTrack)							TWOWORDINLINE(0x7044, 0xAAAA);


EXTERN_API( Boolean )
GetTrackEnabled					(Track 					theTrack)							TWOWORDINLINE(0x7045, 0xAAAA);

EXTERN_API( void )
SetTrackEnabled					(Track 					theTrack,
								 Boolean 				isEnabled)							TWOWORDINLINE(0x7046, 0xAAAA);

EXTERN_API( long )
GetTrackUsage					(Track 					theTrack)							TWOWORDINLINE(0x7047, 0xAAAA);

EXTERN_API( void )
SetTrackUsage					(Track 					theTrack,
								 long 					usage)								TWOWORDINLINE(0x7048, 0xAAAA);

EXTERN_API( TimeValue )
GetTrackDuration				(Track 					theTrack)							TWOWORDINLINE(0x704B, 0xAAAA);

EXTERN_API( TimeValue )
GetTrackOffset					(Track 					theTrack)							TWOWORDINLINE(0x704C, 0xAAAA);

EXTERN_API( void )
SetTrackOffset					(Track 					theTrack,
								 TimeValue 				movieOffsetTime)					TWOWORDINLINE(0x704D, 0xAAAA);

EXTERN_API( short )
GetTrackLayer					(Track 					theTrack)							TWOWORDINLINE(0x7050, 0xAAAA);

EXTERN_API( void )
SetTrackLayer					(Track 					theTrack,
								 short 					layer)								TWOWORDINLINE(0x7051, 0xAAAA);

EXTERN_API( Track )
GetTrackAlternate				(Track 					theTrack)							TWOWORDINLINE(0x7052, 0xAAAA);

EXTERN_API( void )
SetTrackAlternate				(Track 					theTrack,
								 Track 					alternateT)							TWOWORDINLINE(0x7053, 0xAAAA);

EXTERN_API( void )
SetAutoTrackAlternatesEnabled	(Movie 					theMovie,
								 Boolean 				enable)								THREEWORDINLINE(0x303C, 0x015E, 0xAAAA);

EXTERN_API( void )
SelectMovieAlternates			(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x015F, 0xAAAA);

EXTERN_API( short )
GetTrackVolume					(Track 					theTrack)							TWOWORDINLINE(0x7054, 0xAAAA);

EXTERN_API( void )
SetTrackVolume					(Track 					theTrack,
								 short 					volume)								TWOWORDINLINE(0x7055, 0xAAAA);

EXTERN_API( void )
GetTrackMatrix					(Track 					theTrack,
								 MatrixRecord *			matrix)								TWOWORDINLINE(0x7056, 0xAAAA);

EXTERN_API( void )
SetTrackMatrix					(Track 					theTrack,
								 const MatrixRecord *	matrix)								TWOWORDINLINE(0x7057, 0xAAAA);

EXTERN_API( void )
GetTrackDimensions				(Track 					theTrack,
								 Fixed *				width,
								 Fixed *				height)								TWOWORDINLINE(0x705D, 0xAAAA);

EXTERN_API( void )
SetTrackDimensions				(Track 					theTrack,
								 Fixed 					width,
								 Fixed 					height)								TWOWORDINLINE(0x705E, 0xAAAA);

EXTERN_API( UserData )
GetTrackUserData				(Track 					theTrack)							TWOWORDINLINE(0x705F, 0xAAAA);

EXTERN_API( OSErr )
GetTrackDisplayMatrix			(Track 					theTrack,
								 MatrixRecord *			matrix)								THREEWORDINLINE(0x303C, 0x0263, 0xAAAA);

EXTERN_API( OSErr )
GetTrackSoundLocalizationSettings (Track 				theTrack,
								 Handle *				settings)							THREEWORDINLINE(0x303C, 0x0282, 0xAAAA);

EXTERN_API( OSErr )
SetTrackSoundLocalizationSettings (Track 				theTrack,
								 Handle 				settings)							THREEWORDINLINE(0x303C, 0x0283, 0xAAAA);

/*************************
* get Media routines
**************************/
EXTERN_API( Media )
NewTrackMedia					(Track 					theTrack,
								 OSType 				mediaType,
								 TimeScale 				timeScale,
								 Handle 				dataRef,
								 OSType 				dataRefType)						THREEWORDINLINE(0x303C, 0x018E, 0xAAAA);

EXTERN_API( void )
DisposeTrackMedia				(Media 					theMedia)							TWOWORDINLINE(0x7061, 0xAAAA);

EXTERN_API( Media )
GetTrackMedia					(Track 					theTrack)							TWOWORDINLINE(0x7062, 0xAAAA);

EXTERN_API( Track )
GetMediaTrack					(Media 					theMedia)							THREEWORDINLINE(0x303C, 0x00C5, 0xAAAA);



/*************************
* Media State routines
**************************/
EXTERN_API( unsigned long )
GetMediaCreationTime			(Media 					theMedia)							TWOWORDINLINE(0x7066, 0xAAAA);

EXTERN_API( unsigned long )
GetMediaModificationTime		(Media 					theMedia)							TWOWORDINLINE(0x7067, 0xAAAA);

EXTERN_API( TimeScale )
GetMediaTimeScale				(Media 					theMedia)							TWOWORDINLINE(0x7068, 0xAAAA);

EXTERN_API( void )
SetMediaTimeScale				(Media 					theMedia,
								 TimeScale 				timeScale)							TWOWORDINLINE(0x7069, 0xAAAA);

EXTERN_API( TimeValue )
GetMediaDuration				(Media 					theMedia)							TWOWORDINLINE(0x706A, 0xAAAA);

EXTERN_API( short )
GetMediaLanguage				(Media 					theMedia)							TWOWORDINLINE(0x706B, 0xAAAA);

EXTERN_API( void )
SetMediaLanguage				(Media 					theMedia,
								 short 					language)							TWOWORDINLINE(0x706C, 0xAAAA);

EXTERN_API( short )
GetMediaQuality					(Media 					theMedia)							TWOWORDINLINE(0x706D, 0xAAAA);

EXTERN_API( void )
SetMediaQuality					(Media 					theMedia,
								 short 					quality)							TWOWORDINLINE(0x706E, 0xAAAA);

EXTERN_API( void )
GetMediaHandlerDescription		(Media 					theMedia,
								 OSType *				mediaType,
								 Str255 				creatorName,
								 OSType *				creatorManufacturer)				TWOWORDINLINE(0x706F, 0xAAAA);

EXTERN_API( UserData )
GetMediaUserData				(Media 					theMedia)							TWOWORDINLINE(0x7070, 0xAAAA);

EXTERN_API( OSErr )
GetMediaInputMap				(Media 					theMedia,
								 QTAtomContainer *		inputMap)							THREEWORDINLINE(0x303C, 0x0249, 0xAAAA);

EXTERN_API( OSErr )
SetMediaInputMap				(Media 					theMedia,
								 QTAtomContainer 		inputMap)							THREEWORDINLINE(0x303C, 0x024A, 0xAAAA);

/*************************
* Media Handler routines
**************************/
EXTERN_API( MediaHandler )
GetMediaHandler					(Media 					theMedia)							TWOWORDINLINE(0x7071, 0xAAAA);

EXTERN_API( OSErr )
SetMediaHandler					(Media 					theMedia,
								 MediaHandlerComponent 	mH)									THREEWORDINLINE(0x303C, 0x0190, 0xAAAA);


/*************************
* Media's Data routines
**************************/
EXTERN_API( OSErr )
BeginMediaEdits					(Media 					theMedia)							TWOWORDINLINE(0x7072, 0xAAAA);

EXTERN_API( OSErr )
EndMediaEdits					(Media 					theMedia)							TWOWORDINLINE(0x7073, 0xAAAA);

EXTERN_API( OSErr )
SetMediaDefaultDataRefIndex		(Media 					theMedia,
								 short 					index)								THREEWORDINLINE(0x303C, 0x01E0, 0xAAAA);

EXTERN_API( void )
GetMediaDataHandlerDescription	(Media 					theMedia,
								 short 					index,
								 OSType *				dhType,
								 Str255 				creatorName,
								 OSType *				creatorManufacturer)				THREEWORDINLINE(0x303C, 0x019E, 0xAAAA);

EXTERN_API( DataHandler )
GetMediaDataHandler				(Media 					theMedia,
								 short 					index)								THREEWORDINLINE(0x303C, 0x019F, 0xAAAA);

EXTERN_API( OSErr )
SetMediaDataHandler				(Media 					theMedia,
								 short 					index,
								 DataHandlerComponent 	dataHandler)						THREEWORDINLINE(0x303C, 0x01A0, 0xAAAA);

EXTERN_API( Component )
GetDataHandler					(Handle 				dataRef,
								 OSType 				dataHandlerSubType,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x01ED, 0xAAAA);


/*************************
* Media Sample Table Routines
**************************/
EXTERN_API( long )
GetMediaSampleDescriptionCount	(Media 					theMedia)							TWOWORDINLINE(0x7077, 0xAAAA);

EXTERN_API( void )
GetMediaSampleDescription		(Media 					theMedia,
								 long 					index,
								 SampleDescriptionHandle  descH)							TWOWORDINLINE(0x7078, 0xAAAA);

EXTERN_API( OSErr )
SetMediaSampleDescription		(Media 					theMedia,
								 long 					index,
								 SampleDescriptionHandle  descH)							THREEWORDINLINE(0x303C, 0x01D0, 0xAAAA);

EXTERN_API( long )
GetMediaSampleCount				(Media 					theMedia)							TWOWORDINLINE(0x7079, 0xAAAA);

EXTERN_API( long )
GetMediaSyncSampleCount			(Media 					theMedia)							THREEWORDINLINE(0x303C, 0x02B2, 0xAAAA);

EXTERN_API( void )
SampleNumToMediaTime			(Media 					theMedia,
								 long 					logicalSampleNum,
								 TimeValue *			sampleTime,
								 TimeValue *			sampleDuration)						TWOWORDINLINE(0x707A, 0xAAAA);

EXTERN_API( void )
MediaTimeToSampleNum			(Media 					theMedia,
								 TimeValue 				time,
								 long *					sampleNum,
								 TimeValue *			sampleTime,
								 TimeValue *			sampleDuration)						TWOWORDINLINE(0x707B, 0xAAAA);


EXTERN_API( OSErr )
AddMediaSample					(Media 					theMedia,
								 Handle 				dataIn,
								 long 					inOffset,
								 unsigned long 			size,
								 TimeValue 				durationPerSample,
								 SampleDescriptionHandle  sampleDescriptionH,
								 long 					numberOfSamples,
								 short 					sampleFlags,
								 TimeValue *			sampleTime)							TWOWORDINLINE(0x707C, 0xAAAA);

EXTERN_API( OSErr )
AddMediaSampleReference			(Media 					theMedia,
								 long 					dataOffset,
								 unsigned long 			size,
								 TimeValue 				durationPerSample,
								 SampleDescriptionHandle  sampleDescriptionH,
								 long 					numberOfSamples,
								 short 					sampleFlags,
								 TimeValue *			sampleTime)							TWOWORDINLINE(0x707D, 0xAAAA);

EXTERN_API( OSErr )
AddMediaSampleReferences		(Media 					theMedia,
								 SampleDescriptionHandle  sampleDescriptionH,
								 long 					numberOfSamples,
								 SampleReferencePtr 	sampleRefs,
								 TimeValue *			sampleTime)							THREEWORDINLINE(0x303C, 0x01F7, 0xAAAA);

EXTERN_API( OSErr )
GetMediaSample					(Media 					theMedia,
								 Handle 				dataOut,
								 long 					maxSizeToGrow,
								 long *					size,
								 TimeValue 				time,
								 TimeValue *			sampleTime,
								 TimeValue *			durationPerSample,
								 SampleDescriptionHandle  sampleDescriptionH,
								 long *					sampleDescriptionIndex,
								 long 					maxNumberOfSamples,
								 long *					numberOfSamples,
								 short *				sampleFlags)						TWOWORDINLINE(0x707E, 0xAAAA);

EXTERN_API( OSErr )
GetMediaSampleReference			(Media 					theMedia,
								 long *					dataOffset,
								 long *					size,
								 TimeValue 				time,
								 TimeValue *			sampleTime,
								 TimeValue *			durationPerSample,
								 SampleDescriptionHandle  sampleDescriptionH,
								 long *					sampleDescriptionIndex,
								 long 					maxNumberOfSamples,
								 long *					numberOfSamples,
								 short *				sampleFlags)						TWOWORDINLINE(0x707F, 0xAAAA);

EXTERN_API( OSErr )
GetMediaSampleReferences		(Media 					theMedia,
								 TimeValue 				time,
								 TimeValue *			sampleTime,
								 SampleDescriptionHandle  sampleDescriptionH,
								 long *					sampleDescriptionIndex,
								 long 					maxNumberOfEntries,
								 long *					actualNumberofEntries,
								 SampleReferencePtr 	sampleRefs)							THREEWORDINLINE(0x303C, 0x0235, 0xAAAA);

EXTERN_API( OSErr )
SetMediaPreferredChunkSize		(Media 					theMedia,
								 long 					maxChunkSize)						THREEWORDINLINE(0x303C, 0x01F8, 0xAAAA);

EXTERN_API( OSErr )
GetMediaPreferredChunkSize		(Media 					theMedia,
								 long *					maxChunkSize)						THREEWORDINLINE(0x303C, 0x01F9, 0xAAAA);

EXTERN_API( OSErr )
SetMediaShadowSync				(Media 					theMedia,
								 long 					frameDiffSampleNum,
								 long 					syncSampleNum)						THREEWORDINLINE(0x303C, 0x0121, 0xAAAA);

EXTERN_API( OSErr )
GetMediaShadowSync				(Media 					theMedia,
								 long 					frameDiffSampleNum,
								 long *					syncSampleNum)						THREEWORDINLINE(0x303C, 0x0122, 0xAAAA);

/*************************
* Editing Routines
**************************/
EXTERN_API( OSErr )
InsertMediaIntoTrack			(Track 					theTrack,
								 TimeValue 				trackStart,
								 TimeValue 				mediaTime,
								 TimeValue 				mediaDuration,
								 Fixed 					mediaRate)							THREEWORDINLINE(0x303C, 0x0183, 0xAAAA);

EXTERN_API( OSErr )
InsertTrackSegment				(Track 					srcTrack,
								 Track 					dstTrack,
								 TimeValue 				srcIn,
								 TimeValue 				srcDuration,
								 TimeValue 				dstIn)								THREEWORDINLINE(0x303C, 0x0085, 0xAAAA);

EXTERN_API( OSErr )
InsertMovieSegment				(Movie 					srcMovie,
								 Movie 					dstMovie,
								 TimeValue 				srcIn,
								 TimeValue 				srcDuration,
								 TimeValue 				dstIn)								THREEWORDINLINE(0x303C, 0x0086, 0xAAAA);

EXTERN_API( OSErr )
InsertEmptyTrackSegment			(Track 					dstTrack,
								 TimeValue 				dstIn,
								 TimeValue 				dstDuration)						THREEWORDINLINE(0x303C, 0x0087, 0xAAAA);

EXTERN_API( OSErr )
InsertEmptyMovieSegment			(Movie 					dstMovie,
								 TimeValue 				dstIn,
								 TimeValue 				dstDuration)						THREEWORDINLINE(0x303C, 0x0088, 0xAAAA);

EXTERN_API( OSErr )
DeleteTrackSegment				(Track 					theTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x0089, 0xAAAA);

EXTERN_API( OSErr )
DeleteMovieSegment				(Movie 					theMovie,
								 TimeValue 				startTime,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x008A, 0xAAAA);

EXTERN_API( OSErr )
ScaleTrackSegment				(Track 					theTrack,
								 TimeValue 				startTime,
								 TimeValue 				oldDuration,
								 TimeValue 				newDuration)						THREEWORDINLINE(0x303C, 0x008B, 0xAAAA);

EXTERN_API( OSErr )
ScaleMovieSegment				(Movie 					theMovie,
								 TimeValue 				startTime,
								 TimeValue 				oldDuration,
								 TimeValue 				newDuration)						THREEWORDINLINE(0x303C, 0x008C, 0xAAAA);


/*************************
* Hi-level Editing Routines
**************************/
EXTERN_API( Movie )
CutMovieSelection				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x008D, 0xAAAA);

EXTERN_API( Movie )
CopyMovieSelection				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x008E, 0xAAAA);

EXTERN_API( void )
PasteMovieSelection				(Movie 					theMovie,
								 Movie 					src)								THREEWORDINLINE(0x303C, 0x008F, 0xAAAA);

EXTERN_API( void )
AddMovieSelection				(Movie 					theMovie,
								 Movie 					src)								THREEWORDINLINE(0x303C, 0x0152, 0xAAAA);

EXTERN_API( void )
ClearMovieSelection				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00E1, 0xAAAA);

EXTERN_API( OSErr )
PasteHandleIntoMovie			(Handle 				h,
								 OSType 				handleType,
								 Movie 					theMovie,
								 long 					flags,
								 ComponentInstance 		userComp)							THREEWORDINLINE(0x303C, 0x00CB, 0xAAAA);

EXTERN_API( OSErr )
PutMovieIntoTypedHandle			(Movie 					theMovie,
								 Track 					targetTrack,
								 OSType 				handleType,
								 Handle 				publicMovie,
								 TimeValue 				start,
								 TimeValue 				dur,
								 long 					flags,
								 ComponentInstance 		userComp)							THREEWORDINLINE(0x303C, 0x01CD, 0xAAAA);

EXTERN_API( Component )
IsScrapMovie					(Track 					targetTrack)						THREEWORDINLINE(0x303C, 0x00CC, 0xAAAA);

/*************************
* Middle-level Editing Routines
**************************/
EXTERN_API( OSErr )
CopyTrackSettings				(Track 					srcTrack,
								 Track 					dstTrack)							THREEWORDINLINE(0x303C, 0x0153, 0xAAAA);

EXTERN_API( OSErr )
CopyMovieSettings				(Movie 					srcMovie,
								 Movie 					dstMovie)							THREEWORDINLINE(0x303C, 0x0154, 0xAAAA);

EXTERN_API( OSErr )
AddEmptyTrackToMovie			(Track 					srcTrack,
								 Movie 					dstMovie,
								 Handle 				dataRef,
								 OSType 				dataRefType,
								 Track *				dstTrack)							TWOWORDINLINE(0x7074, 0xAAAA);

/*************************
* movie & track edit state routines
**************************/
EXTERN_API( MovieEditState )
NewMovieEditState				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x0104, 0xAAAA);

EXTERN_API( OSErr )
UseMovieEditState				(Movie 					theMovie,
								 MovieEditState 		toState)							THREEWORDINLINE(0x303C, 0x0105, 0xAAAA);

EXTERN_API( OSErr )
DisposeMovieEditState			(MovieEditState 		state)								THREEWORDINLINE(0x303C, 0x0106, 0xAAAA);

EXTERN_API( TrackEditState )
NewTrackEditState				(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0107, 0xAAAA);

EXTERN_API( OSErr )
UseTrackEditState				(Track 					theTrack,
								 TrackEditState 		state)								THREEWORDINLINE(0x303C, 0x0108, 0xAAAA);

EXTERN_API( OSErr )
DisposeTrackEditState			(TrackEditState 		state)								THREEWORDINLINE(0x303C, 0x0109, 0xAAAA);

/*************************
* track reference routines
**************************/
EXTERN_API( OSErr )
AddTrackReference				(Track 					theTrack,
								 Track 					refTrack,
								 OSType 				refType,
								 long *					addedIndex)							THREEWORDINLINE(0x303C, 0x01F0, 0xAAAA);

EXTERN_API( OSErr )
DeleteTrackReference			(Track 					theTrack,
								 OSType 				refType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x01F1, 0xAAAA);

EXTERN_API( OSErr )
SetTrackReference				(Track 					theTrack,
								 Track 					refTrack,
								 OSType 				refType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x01F2, 0xAAAA);

EXTERN_API( Track )
GetTrackReference				(Track 					theTrack,
								 OSType 				refType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x01F3, 0xAAAA);

EXTERN_API( OSType )
GetNextTrackReferenceType		(Track 					theTrack,
								 OSType 				refType)							THREEWORDINLINE(0x303C, 0x01F4, 0xAAAA);

EXTERN_API( long )
GetTrackReferenceCount			(Track 					theTrack,
								 OSType 				refType)							THREEWORDINLINE(0x303C, 0x01F5, 0xAAAA);


/*************************
* high level file conversion routines
**************************/
EXTERN_API( OSErr )
ConvertFileToMovieFile			(const FSSpec *			inputFile,
								 const FSSpec *			outputFile,
								 OSType 				creator,
								 ScriptCode 			scriptTag,
								 short *				resID,
								 long 					flags,
								 ComponentInstance 		userComp,
								 MovieProgressUPP 		proc,
								 long 					refCon)								THREEWORDINLINE(0x303C, 0x01CB, 0xAAAA);

EXTERN_API( OSErr )
ConvertMovieToFile				(Movie 					theMovie,
								 Track 					onlyTrack,
								 FSSpec *				outputFile,
								 OSType 				fileType,
								 OSType 				creator,
								 ScriptCode 			scriptTag,
								 short *				resID,
								 long 					flags,
								 ComponentInstance 		userComp)							THREEWORDINLINE(0x303C, 0x01CC, 0xAAAA);


enum {
	kGetMovieImporterValidateToFind = 1L << 0,
	kGetMovieImporterAllowNewFile = 1L << 1,
	kGetMovieImporterDontConsiderGraphicsImporters = 1L << 2
};

EXTERN_API( OSErr )
GetMovieImporterForDataRef		(OSType 				dataRefType,
								 Handle 				dataRef,
								 long 					flags,
								 Component *			importer)							THREEWORDINLINE(0x303C, 0x02C7, 0xAAAA);

/*************************
* Movie Timebase Conversion Routines
**************************/
EXTERN_API( TimeValue )
TrackTimeToMediaTime			(TimeValue 				value,
								 Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0096, 0xAAAA);

EXTERN_API( Fixed )
GetTrackEditRate				(Track 					theTrack,
								 TimeValue 				atTime)								THREEWORDINLINE(0x303C, 0x0123, 0xAAAA);


/*************************
* Miscellaneous Routines
**************************/

EXTERN_API( long )
GetMovieDataSize				(Movie 					theMovie,
								 TimeValue 				startTime,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x0098, 0xAAAA);

EXTERN_API( long )
GetTrackDataSize				(Track 					theTrack,
								 TimeValue 				startTime,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x0149, 0xAAAA);

EXTERN_API( long )
GetMediaDataSize				(Media 					theMedia,
								 TimeValue 				startTime,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x0099, 0xAAAA);

EXTERN_API( Boolean )
PtInMovie						(Movie 					theMovie,
								 Point 					pt)									THREEWORDINLINE(0x303C, 0x009A, 0xAAAA);

EXTERN_API( Boolean )
PtInTrack						(Track 					theTrack,
								 Point 					pt)									THREEWORDINLINE(0x303C, 0x009B, 0xAAAA);


/*************************
* Group Selection Routines
**************************/

EXTERN_API( void )
SetMovieLanguage				(Movie 					theMovie,
								 long 					language)							THREEWORDINLINE(0x303C, 0x009C, 0xAAAA);


/*************************
* User Data
**************************/

EXTERN_API( OSErr )
GetUserData						(UserData 				theUserData,
								 Handle 				data,
								 OSType 				udType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x009E, 0xAAAA);

EXTERN_API( OSErr )
AddUserData						(UserData 				theUserData,
								 Handle 				data,
								 OSType 				udType)								THREEWORDINLINE(0x303C, 0x009F, 0xAAAA);

EXTERN_API( OSErr )
RemoveUserData					(UserData 				theUserData,
								 OSType 				udType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x00A0, 0xAAAA);

EXTERN_API( short )
CountUserDataType				(UserData 				theUserData,
								 OSType 				udType)								THREEWORDINLINE(0x303C, 0x014B, 0xAAAA);

EXTERN_API( long )
GetNextUserDataType				(UserData 				theUserData,
								 OSType 				udType)								THREEWORDINLINE(0x303C, 0x01A5, 0xAAAA);

EXTERN_API( OSErr )
GetUserDataItem					(UserData 				theUserData,
								 void *					data,
								 long 					size,
								 OSType 				udType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x0126, 0xAAAA);

EXTERN_API( OSErr )
SetUserDataItem					(UserData 				theUserData,
								 void *					data,
								 long 					size,
								 OSType 				udType,
								 long 					index)								THREEWORDINLINE(0x303C, 0x012E, 0xAAAA);

EXTERN_API( OSErr )
AddUserDataText					(UserData 				theUserData,
								 Handle 				data,
								 OSType 				udType,
								 long 					index,
								 short 					itlRegionTag)						THREEWORDINLINE(0x303C, 0x014C, 0xAAAA);

EXTERN_API( OSErr )
GetUserDataText					(UserData 				theUserData,
								 Handle 				data,
								 OSType 				udType,
								 long 					index,
								 short 					itlRegionTag)						THREEWORDINLINE(0x303C, 0x014D, 0xAAAA);

EXTERN_API( OSErr )
RemoveUserDataText				(UserData 				theUserData,
								 OSType 				udType,
								 long 					index,
								 short 					itlRegionTag)						THREEWORDINLINE(0x303C, 0x014E, 0xAAAA);

EXTERN_API( OSErr )
NewUserData						(UserData *				theUserData)						THREEWORDINLINE(0x303C, 0x012F, 0xAAAA);

EXTERN_API( OSErr )
DisposeUserData					(UserData 				theUserData)						THREEWORDINLINE(0x303C, 0x0130, 0xAAAA);

EXTERN_API( OSErr )
NewUserDataFromHandle			(Handle 				h,
								 UserData *				theUserData)						THREEWORDINLINE(0x303C, 0x0131, 0xAAAA);

EXTERN_API( OSErr )
PutUserDataIntoHandle			(UserData 				theUserData,
								 Handle 				h)									THREEWORDINLINE(0x303C, 0x0132, 0xAAAA);

EXTERN_API( void )
GetMediaNextInterestingTime		(Media 					theMedia,
								 short 					interestingTimeFlags,
								 TimeValue 				time,
								 Fixed 					rate,
								 TimeValue *			interestingTime,
								 TimeValue *			interestingDuration)				THREEWORDINLINE(0x303C, 0x016D, 0xAAAA);

EXTERN_API( void )
GetTrackNextInterestingTime		(Track 					theTrack,
								 short 					interestingTimeFlags,
								 TimeValue 				time,
								 Fixed 					rate,
								 TimeValue *			interestingTime,
								 TimeValue *			interestingDuration)				THREEWORDINLINE(0x303C, 0x00E2, 0xAAAA);

EXTERN_API( void )
GetMovieNextInterestingTime		(Movie 					theMovie,
								 short 					interestingTimeFlags,
								 short 					numMediaTypes,
								 const OSType *			whichMediaTypes,
								 TimeValue 				time,
								 Fixed 					rate,
								 TimeValue *			interestingTime,
								 TimeValue *			interestingDuration)				THREEWORDINLINE(0x303C, 0x010E, 0xAAAA);


EXTERN_API( OSErr )
CreateMovieFile					(const FSSpec *			fileSpec,
								 OSType 				creator,
								 ScriptCode 			scriptTag,
								 long 					createMovieFileFlags,
								 short *				resRefNum,
								 Movie *				newmovie)							THREEWORDINLINE(0x303C, 0x0191, 0xAAAA);

EXTERN_API( OSErr )
OpenMovieFile					(const FSSpec *			fileSpec,
								 short *				resRefNum,
								 SInt8 					permission)							THREEWORDINLINE(0x303C, 0x0192, 0xAAAA);

EXTERN_API( OSErr )
CloseMovieFile					(short 					resRefNum)							THREEWORDINLINE(0x303C, 0x00D5, 0xAAAA);

EXTERN_API( OSErr )
DeleteMovieFile					(const FSSpec *			fileSpec)							THREEWORDINLINE(0x303C, 0x0175, 0xAAAA);

EXTERN_API( OSErr )
NewMovieFromFile				(Movie *				theMovie,
								 short 					resRefNum,
								 short *				resId,
								 StringPtr 				resName,
								 short 					newMovieFlags,
								 Boolean *				dataRefWasChanged)					THREEWORDINLINE(0x303C, 0x00F0, 0xAAAA);

EXTERN_API( OSErr )
NewMovieFromHandle				(Movie *				theMovie,
								 Handle 				h,
								 short 					newMovieFlags,
								 Boolean *				dataRefWasChanged)					THREEWORDINLINE(0x303C, 0x00F1, 0xAAAA);

EXTERN_API( OSErr )
NewMovieFromDataFork			(Movie *				theMovie,
								 short 					fRefNum,
								 long 					fileOffset,
								 short 					newMovieFlags,
								 Boolean *				dataRefWasChanged)					THREEWORDINLINE(0x303C, 0x01B3, 0xAAAA);

EXTERN_API( OSErr )
NewMovieFromUserProc			(Movie *				m,
								 short 					flags,
								 Boolean *				dataRefWasChanged,
								 GetMovieUPP 			getProc,
								 void *					refCon,
								 Handle 				defaultDataRef,
								 OSType 				dataRefType)						THREEWORDINLINE(0x303C, 0x01EC, 0xAAAA);

EXTERN_API( OSErr )
NewMovieFromDataRef				(Movie *				m,
								 short 					flags,
								 short *				id,
								 Handle 				dataRef,
								 OSType 				dataRefType)						THREEWORDINLINE(0x303C, 0x0220, 0xAAAA);

EXTERN_API( OSErr )
AddMovieResource				(Movie 					theMovie,
								 short 					resRefNum,
								 short *				resId,
								 ConstStr255Param 		resName)							THREEWORDINLINE(0x303C, 0x00D7, 0xAAAA);

EXTERN_API( OSErr )
UpdateMovieResource				(Movie 					theMovie,
								 short 					resRefNum,
								 short 					resId,
								 ConstStr255Param 		resName)							THREEWORDINLINE(0x303C, 0x00D8, 0xAAAA);

EXTERN_API( OSErr )
RemoveMovieResource				(short 					resRefNum,
								 short 					resId)								THREEWORDINLINE(0x303C, 0x0176, 0xAAAA);

EXTERN_API( Boolean )
HasMovieChanged					(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x00D9, 0xAAAA);

EXTERN_API( void )
ClearMovieChanged				(Movie 					theMovie)							THREEWORDINLINE(0x303C, 0x0113, 0xAAAA);

EXTERN_API( OSErr )
SetMovieDefaultDataRef			(Movie 					theMovie,
								 Handle 				dataRef,
								 OSType 				dataRefType)						THREEWORDINLINE(0x303C, 0x01C1, 0xAAAA);

EXTERN_API( OSErr )
GetMovieDefaultDataRef			(Movie 					theMovie,
								 Handle *				dataRef,
								 OSType *				dataRefType)						THREEWORDINLINE(0x303C, 0x01D2, 0xAAAA);

EXTERN_API( OSErr )
SetMovieColorTable				(Movie 					theMovie,
								 CTabHandle 			ctab)								THREEWORDINLINE(0x303C, 0x0205, 0xAAAA);

EXTERN_API( OSErr )
GetMovieColorTable				(Movie 					theMovie,
								 CTabHandle *			ctab)								THREEWORDINLINE(0x303C, 0x0206, 0xAAAA);

EXTERN_API( void )
FlattenMovie					(Movie 					theMovie,
								 long 					movieFlattenFlags,
								 const FSSpec *			theFile,
								 OSType 				creator,
								 ScriptCode 			scriptTag,
								 long 					createMovieFileFlags,
								 short *				resId,
								 ConstStr255Param 		resName)							THREEWORDINLINE(0x303C, 0x019B, 0xAAAA);

EXTERN_API( Movie )
FlattenMovieData				(Movie 					theMovie,
								 long 					movieFlattenFlags,
								 const FSSpec *			theFile,
								 OSType 				creator,
								 ScriptCode 			scriptTag,
								 long 					createMovieFileFlags)				THREEWORDINLINE(0x303C, 0x019C, 0xAAAA);

EXTERN_API( void )
SetMovieProgressProc			(Movie 					theMovie,
								 MovieProgressUPP 		p,
								 long 					refcon)								THREEWORDINLINE(0x303C, 0x019A, 0xAAAA);

EXTERN_API( OSErr )
MovieSearchText					(Movie 					theMovie,
								 Ptr 					text,
								 long 					size,
								 long 					searchFlags,
								 Track *				searchTrack,
								 TimeValue *			searchTime,
								 long *					searchOffset)						THREEWORDINLINE(0x303C, 0x0207, 0xAAAA);

EXTERN_API( void )
GetPosterBox					(Movie 					theMovie,
								 Rect *					boxRect)							THREEWORDINLINE(0x303C, 0x016F, 0xAAAA);

EXTERN_API( void )
SetPosterBox					(Movie 					theMovie,
								 const Rect *			boxRect)							THREEWORDINLINE(0x303C, 0x0170, 0xAAAA);

EXTERN_API( RgnHandle )
GetMovieSegmentDisplayBoundsRgn	(Movie 					theMovie,
								 TimeValue 				time,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x016C, 0xAAAA);

EXTERN_API( RgnHandle )
GetTrackSegmentDisplayBoundsRgn	(Track 					theTrack,
								 TimeValue 				time,
								 TimeValue 				duration)							THREEWORDINLINE(0x303C, 0x016B, 0xAAAA);

EXTERN_API( void )
SetMovieCoverProcs				(Movie 					theMovie,
								 MovieRgnCoverUPP 		uncoverProc,
								 MovieRgnCoverUPP 		coverProc,
								 long 					refcon)								THREEWORDINLINE(0x303C, 0x0179, 0xAAAA);

EXTERN_API( OSErr )
GetMovieCoverProcs				(Movie 					theMovie,
								 MovieRgnCoverUPP *		uncoverProc,
								 MovieRgnCoverUPP *		coverProc,
								 long *					refcon)								THREEWORDINLINE(0x303C, 0x01DD, 0xAAAA);

EXTERN_API( ComponentResult )
GetTrackStatus					(Track 					theTrack)							THREEWORDINLINE(0x303C, 0x0172, 0xAAAA);

EXTERN_API( ComponentResult )
GetMovieStatus					(Movie 					theMovie,
								 Track *				firstProblemTrack)					THREEWORDINLINE(0x303C, 0x0173, 0xAAAA);

/****
	Movie Controller support routines
****/
EXTERN_API( ComponentInstance )
NewMovieController				(Movie 					theMovie,
								 const Rect *			movieRect,
								 long 					someFlags)							THREEWORDINLINE(0x303C, 0x018A, 0xAAAA);

EXTERN_API( void )
DisposeMovieController			(ComponentInstance 		mc)									THREEWORDINLINE(0x303C, 0x018B, 0xAAAA);

EXTERN_API( void )
ShowMovieInformation			(Movie 					theMovie,
								 ModalFilterUPP 		filterProc,
								 long 					refCon)								THREEWORDINLINE(0x303C, 0x0209, 0xAAAA);


/*****
	Scrap routines
*****/
EXTERN_API( OSErr )
PutMovieOnScrap					(Movie 					theMovie,
								 long 					movieScrapFlags)					THREEWORDINLINE(0x303C, 0x018C, 0xAAAA);

EXTERN_API( Movie )
NewMovieFromScrap				(long 					newMovieFlags)						THREEWORDINLINE(0x303C, 0x018D, 0xAAAA);


/*****
	DataRef routines
*****/

EXTERN_API( OSErr )
GetMediaDataRef					(Media 					theMedia,
								 short 					index,
								 Handle *				dataRef,
								 OSType *				dataRefType,
								 long *					dataRefAttributes)					THREEWORDINLINE(0x303C, 0x0197, 0xAAAA);

EXTERN_API( OSErr )
SetMediaDataRef					(Media 					theMedia,
								 short 					index,
								 Handle 				dataRef,
								 OSType 				dataRefType)						THREEWORDINLINE(0x303C, 0x01C9, 0xAAAA);

EXTERN_API( OSErr )
SetMediaDataRefAttributes		(Media 					theMedia,
								 short 					index,
								 long 					dataRefAttributes)					THREEWORDINLINE(0x303C, 0x01CA, 0xAAAA);

EXTERN_API( OSErr )
AddMediaDataRef					(Media 					theMedia,
								 short *				index,
								 Handle 				dataRef,
								 OSType 				dataRefType)						THREEWORDINLINE(0x303C, 0x0198, 0xAAAA);

EXTERN_API( OSErr )
GetMediaDataRefCount			(Media 					theMedia,
								 short *				count)								THREEWORDINLINE(0x303C, 0x0199, 0xAAAA);

EXTERN_API( OSErr )
QTNewAlias						(const FSSpec *			fss,
								 AliasHandle *			alias,
								 Boolean 				minimal)							THREEWORDINLINE(0x303C, 0x0201, 0xAAAA);

/*****
	Playback hint routines
*****/
EXTERN_API( void )
SetMoviePlayHints				(Movie 					theMovie,
								 long 					flags,
								 long 					flagsMask)							THREEWORDINLINE(0x303C, 0x01A1, 0xAAAA);

EXTERN_API( void )
SetMediaPlayHints				(Media 					theMedia,
								 long 					flags,
								 long 					flagsMask)							THREEWORDINLINE(0x303C, 0x01A2, 0xAAAA);

EXTERN_API( void )
GetMediaPlayHints				(Media 					theMedia,
								 long *					flags)								THREEWORDINLINE(0x303C, 0x02CE, 0xAAAA);

/*****
	Load time track hints
*****/

enum {
	preloadAlways				= 1L << 0,
	preloadOnlyIfEnabled		= 1L << 1
};

EXTERN_API( void )
SetTrackLoadSettings			(Track 					theTrack,
								 TimeValue 				preloadTime,
								 TimeValue 				preloadDuration,
								 long 					preloadFlags,
								 long 					defaultHints)						THREEWORDINLINE(0x303C, 0x01E3, 0xAAAA);

EXTERN_API( void )
GetTrackLoadSettings			(Track 					theTrack,
								 TimeValue *			preloadTime,
								 TimeValue *			preloadDuration,
								 long *					preloadFlags,
								 long *					defaultHints)						THREEWORDINLINE(0x303C, 0x01E4, 0xAAAA);

/*****
	Big screen TV
*****/

enum {
	fullScreenHideCursor		= 1L << 0,
	fullScreenAllowEvents		= 1L << 1,
	fullScreenDontChangeMenuBar	= 1L << 2,
	fullScreenPreflightSize		= 1L << 3
};

EXTERN_API( OSErr )
BeginFullScreen					(Ptr *					restoreState,
								 GDHandle 				whichGD,
								 short *				desiredWidth,
								 short *				desiredHeight,
								 WindowPtr *			newWindow,
								 RGBColor *				eraseColor,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x0233, 0xAAAA);

EXTERN_API( OSErr )
EndFullScreen					(Ptr 					fullState,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x0234, 0xAAAA);

/*****
	Sprite Toolbox
*****/

enum {
	kBackgroundSpriteLayerNum	= 32767
};

/*  Sprite Properties*/

enum {
	kSpritePropertyMatrix		= 1,
	kSpritePropertyImageDescription = 2,
	kSpritePropertyImageDataPtr	= 3,
	kSpritePropertyVisible		= 4,
	kSpritePropertyLayer		= 5,
	kSpritePropertyGraphicsMode	= 6,
	kSpritePropertyImageDataSize = 7,
	kSpritePropertyImageIndex	= 100,
	kSpriteTrackPropertyBackgroundColor = 101,
	kSpriteTrackPropertyOffscreenBitDepth = 102,
	kSpriteTrackPropertySampleFormat = 103,
	kSpriteTrackPropertyScaleSpritesToScaleWorld = 104,
	kSpriteTrackPropertyHasActions = 105,
	kSpriteTrackPropertyVisible	= 106,
	kSpriteTrackPropertyQTIdleEventsFrequency = 107,
	kSpriteImagePropertyRegistrationPoint = 1000,
	kSpriteImagePropertyGroupID	= 1001
};

/* special value for kSpriteTrackPropertyQTIdleEventsFrequency (the default)*/

enum {
	kNoQTIdleEvents				= -1
};

/* flagsIn for SpriteWorldIdle*/

enum {
	kOnlyDrawToSpriteWorld		= 1L << 0,
	kSpriteWorldPreflight		= 1L << 1
};

/* flagsOut for SpriteWorldIdle*/

enum {
	kSpriteWorldDidDraw			= 1L << 0,
	kSpriteWorldNeedsToDraw		= 1L << 1
};

/* flags for sprite track sample format*/

enum {
	kKeyFrameAndSingleOverride	= 1L << 1,
	kKeyFrameAndAllOverrides	= 1L << 2
};

/* sprite world flags*/

enum {
	kScaleSpritesToScaleWorld	= 1L << 1,
	kSpriteWorldHighQuality		= 1L << 2,
	kSpriteWorldDontAutoInvalidate = 1L << 3,
	kSpriteWorldInvisible		= 1L << 4
};

EXTERN_API( OSErr )
NewSpriteWorld					(SpriteWorld *			newSpriteWorld,
								 GWorldPtr 				destination,
								 GWorldPtr 				spriteLayer,
								 RGBColor *				backgroundColor,
								 GWorldPtr 				background)							THREEWORDINLINE(0x303C, 0x0239, 0xAAAA);

EXTERN_API( void )
DisposeSpriteWorld				(SpriteWorld 			theSpriteWorld)						THREEWORDINLINE(0x303C, 0x023A, 0xAAAA);

EXTERN_API( OSErr )
SetSpriteWorldClip				(SpriteWorld 			theSpriteWorld,
								 RgnHandle 				clipRgn)							THREEWORDINLINE(0x303C, 0x023B, 0xAAAA);

EXTERN_API( OSErr )
SetSpriteWorldMatrix			(SpriteWorld 			theSpriteWorld,
								 const MatrixRecord *	matrix)								THREEWORDINLINE(0x303C, 0x023C, 0xAAAA);

EXTERN_API( OSErr )
SetSpriteWorldGraphicsMode		(SpriteWorld 			theSpriteWorld,
								 long 					mode,
								 const RGBColor *		opColor)							THREEWORDINLINE(0x303C, 0x02D9, 0xAAAA);

EXTERN_API( OSErr )
SpriteWorldIdle					(SpriteWorld 			theSpriteWorld,
								 long 					flagsIn,
								 long *					flagsOut)							THREEWORDINLINE(0x303C, 0x023D, 0xAAAA);

EXTERN_API( OSErr )
InvalidateSpriteWorld			(SpriteWorld 			theSpriteWorld,
								 Rect *					invalidArea)						THREEWORDINLINE(0x303C, 0x023E, 0xAAAA);

EXTERN_API( OSErr )
SpriteWorldHitTest				(SpriteWorld 			theSpriteWorld,
								 long 					flags,
								 Point 					loc,
								 Sprite *				spriteHit)							THREEWORDINLINE(0x303C, 0x0246, 0xAAAA);

EXTERN_API( OSErr )
SpriteHitTest					(Sprite 				theSprite,
								 long 					flags,
								 Point 					loc,
								 Boolean *				wasHit)								THREEWORDINLINE(0x303C, 0x0247, 0xAAAA);

EXTERN_API( void )
DisposeAllSprites				(SpriteWorld 			theSpriteWorld)						THREEWORDINLINE(0x303C, 0x023F, 0xAAAA);

EXTERN_API( OSErr )
SetSpriteWorldFlags				(SpriteWorld 			spriteWorld,
								 long 					flags,
								 long 					flagsMask)							THREEWORDINLINE(0x303C, 0x02C2, 0xAAAA);

EXTERN_API( OSErr )
NewSprite						(Sprite *				newSprite,
								 SpriteWorld 			itsSpriteWorld,
								 ImageDescriptionHandle  idh,
								 Ptr 					imageDataPtr,
								 MatrixRecord *			matrix,
								 Boolean 				visible,
								 short 					layer)								THREEWORDINLINE(0x303C, 0x0240, 0xAAAA);

EXTERN_API( void )
DisposeSprite					(Sprite 				theSprite)							THREEWORDINLINE(0x303C, 0x0241, 0xAAAA);

EXTERN_API( void )
InvalidateSprite				(Sprite 				theSprite)							THREEWORDINLINE(0x303C, 0x0242, 0xAAAA);

EXTERN_API( OSErr )
SetSpriteProperty				(Sprite 				theSprite,
								 long 					propertyType,
								 void *					propertyValue)						THREEWORDINLINE(0x303C, 0x0243, 0xAAAA);

EXTERN_API( OSErr )
GetSpriteProperty				(Sprite 				theSprite,
								 long 					propertyType,
								 void *					propertyValue)						THREEWORDINLINE(0x303C, 0x0244, 0xAAAA);

/*****
	QT Atom Data Support
*****/

enum {
	kParentAtomIsContainer		= 0
};

/* create and dispose QTAtomContainer objects*/

EXTERN_API( OSErr )
QTNewAtomContainer				(QTAtomContainer *		atomData)							THREEWORDINLINE(0x303C, 0x020C, 0xAAAA);

EXTERN_API( OSErr )
QTDisposeAtomContainer			(QTAtomContainer 		atomData)							THREEWORDINLINE(0x303C, 0x020D, 0xAAAA);

/* locating nested atoms within QTAtomContainer container*/
EXTERN_API( QTAtomType )
QTGetNextChildType				(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtomType 			currentChildType)					THREEWORDINLINE(0x303C, 0x020E, 0xAAAA);

EXTERN_API( short )
QTCountChildrenOfType			(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtomType 			childType)							THREEWORDINLINE(0x303C, 0x020F, 0xAAAA);

EXTERN_API( QTAtom )
QTFindChildByIndex				(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtomType 			atomType,
								 short 					index,
								 QTAtomID *				id)									THREEWORDINLINE(0x303C, 0x0210, 0xAAAA);

EXTERN_API( QTAtom )
QTFindChildByID					(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtomType 			atomType,
								 QTAtomID 				id,
								 short *				index)								THREEWORDINLINE(0x303C, 0x021D, 0xAAAA);

EXTERN_API( OSErr )
QTNextChildAnyType				(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtom 				currentChild,
								 QTAtom *				nextChild)							THREEWORDINLINE(0x303C, 0x0200, 0xAAAA);

/* set a leaf atom's data*/
EXTERN_API( OSErr )
QTSetAtomData					(QTAtomContainer 		container,
								 QTAtom 				atom,
								 long 					dataSize,
								 void *					atomData)							THREEWORDINLINE(0x303C, 0x0211, 0xAAAA);

/* extracting data*/
EXTERN_API( OSErr )
QTCopyAtomDataToHandle			(QTAtomContainer 		container,
								 QTAtom 				atom,
								 Handle 				targetHandle)						THREEWORDINLINE(0x303C, 0x0212, 0xAAAA);

EXTERN_API( OSErr )
QTCopyAtomDataToPtr				(QTAtomContainer 		container,
								 QTAtom 				atom,
								 Boolean 				sizeOrLessOK,
								 long 					size,
								 void *					targetPtr,
								 long *					actualSize)							THREEWORDINLINE(0x303C, 0x0213, 0xAAAA);

EXTERN_API( OSErr )
QTGetAtomTypeAndID				(QTAtomContainer 		container,
								 QTAtom 				atom,
								 QTAtomType *			atomType,
								 QTAtomID *				id)									THREEWORDINLINE(0x303C, 0x0232, 0xAAAA);

/* extract a copy of an atom and all of it's children, caller disposes*/
EXTERN_API( OSErr )
QTCopyAtom						(QTAtomContainer 		container,
								 QTAtom 				atom,
								 QTAtomContainer *		targetContainer)					THREEWORDINLINE(0x303C, 0x0214, 0xAAAA);

/* obtaining direct reference to atom data*/
EXTERN_API( OSErr )
QTLockContainer					(QTAtomContainer 		container)							THREEWORDINLINE(0x303C, 0x0215, 0xAAAA);

EXTERN_API( OSErr )
QTGetAtomDataPtr				(QTAtomContainer 		container,
								 QTAtom 				atom,
								 long *					dataSize,
								 Ptr *					atomData)							THREEWORDINLINE(0x303C, 0x0216, 0xAAAA);

EXTERN_API( OSErr )
QTUnlockContainer				(QTAtomContainer 		container)							THREEWORDINLINE(0x303C, 0x0217, 0xAAAA);

/*
   building QTAtomContainer trees
   creates and inserts new atom at specified index, existing atoms at or after index are moved toward end of list
   used for Top-Down tree creation
*/
EXTERN_API( OSErr )
QTInsertChild					(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtomType 			atomType,
								 QTAtomID 				id,
								 short 					index,
								 long 					dataSize,
								 void *					data,
								 QTAtom *				newAtom)							THREEWORDINLINE(0x303C, 0x0218, 0xAAAA);

/* inserts children from childrenContainer as children of parentAtom*/
EXTERN_API( OSErr )
QTInsertChildren				(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 QTAtomContainer 		childrenContainer)					THREEWORDINLINE(0x303C, 0x0219, 0xAAAA);

/* destruction*/
EXTERN_API( OSErr )
QTRemoveAtom					(QTAtomContainer 		container,
								 QTAtom 				atom)								THREEWORDINLINE(0x303C, 0x021A, 0xAAAA);

EXTERN_API( OSErr )
QTRemoveChildren				(QTAtomContainer 		container,
								 QTAtom 				atom)								THREEWORDINLINE(0x303C, 0x021B, 0xAAAA);

/* replacement must be same type as target*/
EXTERN_API( OSErr )
QTReplaceAtom					(QTAtomContainer 		targetContainer,
								 QTAtom 				targetAtom,
								 QTAtomContainer 		replacementContainer,
								 QTAtom 				replacementAtom)					THREEWORDINLINE(0x303C, 0x021C, 0xAAAA);

EXTERN_API( OSErr )
QTSwapAtoms						(QTAtomContainer 		container,
								 QTAtom 				atom1,
								 QTAtom 				atom2)								THREEWORDINLINE(0x303C, 0x01FF, 0xAAAA);

EXTERN_API( OSErr )
QTSetAtomID						(QTAtomContainer 		container,
								 QTAtom 				atom,
								 QTAtomID 				newID)								THREEWORDINLINE(0x303C, 0x0231, 0xAAAA);

EXTERN_API( OSErr )
SetMediaPropertyAtom			(Media 					theMedia,
								 QTAtomContainer 		propertyAtom)						THREEWORDINLINE(0x303C, 0x022E, 0xAAAA);

EXTERN_API( OSErr )
GetMediaPropertyAtom			(Media 					theMedia,
								 QTAtomContainer *		propertyAtom)						THREEWORDINLINE(0x303C, 0x022F, 0xAAAA);

/*****
	Tween Support
*****/
typedef struct TweenRecord 				TweenRecord;
typedef CALLBACK_API( ComponentResult , TweenerDataProcPtr )(TweenRecord *tr, void *tweenData, long tweenDataSize, long dataDescriptionSeed, Handle dataDescription, ICMCompletionProcRecordPtr asyncCompletionProc, UniversalProcPtr transferProc, void *refCon);
typedef STACK_UPP_TYPE(TweenerDataProcPtr) 						TweenerDataUPP;

struct TweenRecord {
	long 							version;

	QTAtomContainer 				container;
	QTAtom 							tweenAtom;
	QTAtom 							dataAtom;
	Fixed 							percent;

	TweenerDataUPP 					dataProc;

	void *							private1;
	void *							private2;
};


struct TweenV1Record {
	long 							version;

	QTAtomContainer 				container;
	QTAtom 							tweenAtom;
	QTAtom 							dataAtom;
	Fixed 							percent;

	TweenerDataUPP 					dataProc;

	void *							private1;
	void *							private2;

	Fract 							fractPercent;
};
typedef struct TweenV1Record			TweenV1Record;
EXTERN_API( OSErr )
QTNewTween						(QTTweener *			tween,
								 QTAtomContainer 		container,
								 QTAtom 				tweenAtom,
								 TimeValue 				maxTime)							THREEWORDINLINE(0x303C, 0x029D, 0xAAAA);

EXTERN_API( OSErr )
QTDisposeTween					(QTTweener 				tween)								THREEWORDINLINE(0x303C, 0x029F, 0xAAAA);

EXTERN_API( OSErr )
QTDoTween						(QTTweener 				tween,
								 TimeValue 				atTime,
								 Handle 				result,
								 long *					resultSize,
								 TweenerDataUPP 		tweenDataProc,
								 void *					tweenDataRefCon)					THREEWORDINLINE(0x303C, 0x029E, 0xAAAA);

/*****
	Sound Description Manipulations
*****/
EXTERN_API( OSErr )
AddSoundDescriptionExtension	(SoundDescriptionHandle  desc,
								 Handle 				extension,
								 OSType 				idType)								THREEWORDINLINE(0x303C, 0x02CF, 0xAAAA);

EXTERN_API( OSErr )
GetSoundDescriptionExtension	(SoundDescriptionHandle  desc,
								 Handle *				extension,
								 OSType 				idType)								THREEWORDINLINE(0x303C, 0x02D0, 0xAAAA);

EXTERN_API( OSErr )
RemoveSoundDescriptionExtension	(SoundDescriptionHandle  desc,
								 OSType 				idType)								THREEWORDINLINE(0x303C, 0x02D1, 0xAAAA);

/*****
	Preferences
*****/
EXTERN_API( OSErr )
GetQuickTimePreference			(OSType 				preferenceType,
								 QTAtomContainer *		preferenceAtom)						THREEWORDINLINE(0x303C, 0x02D4, 0xAAAA);

EXTERN_API( OSErr )
SetQuickTimePreference			(OSType 				preferenceType,
								 QTAtomContainer 		preferenceAtom)						THREEWORDINLINE(0x303C, 0x02D5, 0xAAAA);

/*****
	Effects and dialog Support
*****/
/* atom types for entries in the effects list*/

enum {
	kEffectNameAtom				= FOUR_CHAR_CODE('name'),		/* name of effect */
	kEffectTypeAtom				= FOUR_CHAR_CODE('type'),		/* codec sub-type for effect */
	kEffectManufacturerAtom		= FOUR_CHAR_CODE('manu')		/* codec manufacturer for effect */
};


struct QTParamPreviewRecord {
	long 							sourceID;					/* 1 based source identifier*/
	PicHandle 						sourcePicture;				/* picture for preview, must not dispose until dialog is disposed*/
};
typedef struct QTParamPreviewRecord		QTParamPreviewRecord;

typedef QTParamPreviewRecord *			QTParamPreviewPtr;

struct QTParamDialogEventRecord {
	EventRecord *					theEvent;					/* Event received by the dialog */
	DialogPtr 						whichDialog;				/* dialog that event was directed towards */
	short 							itemHit;					/* dialog item which was hit */
};
typedef struct QTParamDialogEventRecord	QTParamDialogEventRecord;

typedef QTParamDialogEventRecord *		QTParamDialogEventPtr;

enum {
	pdActionConfirmDialog		= 1,							/* no param*/
	pdActionSetAppleMenu		= 2,							/* param is MenuHandle*/
	pdActionSetEditMenu			= 3,							/* param is MenuHandle*/
	pdActionGetDialogValues		= 4,							/* param is QTAtomContainer*/
	pdActionSetPreviewUserItem	= 5,							/* param is long*/
	pdActionSetPreviewPicture	= 6,							/* param is QTParamPreviewPtr;*/
	pdActionSetColorPickerEventProc = 7,						/* param is UserEventUPP*/
	pdActionSetDialogTitle		= 8,							/* param is StringPtr */
	pdActionGetSubPanelMenu		= 9,							/* param is MenuHandle* */
	pdActionActivateSubPanel	= 10,							/* param is long */
	pdActionConductStopAlert	= 11,							/* param is StringPtr */
	pdActionModelessCallback	= 12							/* param is QTParamDialogEventPtr */
};

typedef long 							QTParameterDialog;

enum {
	elOptionsIncludeNoneInList	= 0x00000001					/* "None" effect is included in list */
};

typedef long 							QTEffectListOptions;

enum {
	pdOptionsCollectOneValue	= 0x00000001,					/* should collect a single value only*/
	pdOptionsAllowOptionalInterpolations = 0x00000002,			/* non-novice interpolation options are shown */
	pdOptionsModalDialogBox		= 0x00000004					/* dialog box should be modal */
};

typedef long 							QTParameterDialogOptions;

enum {
	effectIsRealtime			= 0								/* effect can be rendered in real time */
};

EXTERN_API( OSErr )
QTGetEffectsList				(QTAtomContainer *		returnedList,
								 long 					minSources,
								 long 					maxSources,
								 QTEffectListOptions 	getOptions)							THREEWORDINLINE(0x303C, 0x02C9, 0xAAAA);

EXTERN_API( OSErr )
QTCreateStandardParameterDialog	(QTAtomContainer 		effectList,
								 QTAtomContainer 		parameters,
								 QTParameterDialogOptions  dialogOptions,
								 QTParameterDialog *	createdDialog)						THREEWORDINLINE(0x303C, 0x02CA, 0xAAAA);

EXTERN_API( OSErr )
QTIsStandardParameterDialogEvent (EventRecord *			pEvent,
								 QTParameterDialog 		createdDialog)						THREEWORDINLINE(0x303C, 0x02CB, 0xAAAA);

EXTERN_API( OSErr )
QTDismissStandardParameterDialog (QTParameterDialog 	createdDialog)						THREEWORDINLINE(0x303C, 0x02CC, 0xAAAA);

EXTERN_API( OSErr )
QTStandardParameterDialogDoAction (QTParameterDialog 	createdDialog,
								 long 					action,
								 void *					params)								THREEWORDINLINE(0x303C, 0x02CD, 0xAAAA);

EXTERN_API( OSErr )
QTGetEffectSpeed				(QTAtomContainer 		parameters,
								 Fixed *				pFPS)								THREEWORDINLINE(0x303C, 0x02D2, 0xAAAA);

/*****
	Access Keys
*****/

enum {
	kAccessKeyAtomType			= FOUR_CHAR_CODE('acky')
};


enum {
	kAccessKeySystemFlag		= 1L << 0
};

EXTERN_API( OSErr )
QTGetAccessKeys					(Str255 				accessKeyType,
								 long 					flags,
								 QTAtomContainer *		keys)								THREEWORDINLINE(0x303C, 0x02B3, 0xAAAA);

EXTERN_API( OSErr )
QTRegisterAccessKey				(Str255 				accessKeyType,
								 long 					flags,
								 Handle 				accessKey)							THREEWORDINLINE(0x303C, 0x02B4, 0xAAAA);

EXTERN_API( OSErr )
QTUnregisterAccessKey			(Str255 				accessKeyType,
								 long 					flags,
								 Handle 				accessKey)							THREEWORDINLINE(0x303C, 0x02B5, 0xAAAA);

/*****
	Time table
*****/
EXTERN_API( OSErr )
MakeTrackTimeTable				(Track 					trackH,
								 long **				offsets,
								 TimeValue 				startTime,
								 TimeValue 				endTime,
								 TimeValue 				timeIncrement,
								 short 					firstDataRefIndex,
								 short 					lastDataRefIndex,
								 long *					retdataRefSkew)						THREEWORDINLINE(0x303C, 0x02BE, 0xAAAA);

EXTERN_API( OSErr )
MakeMediaTimeTable				(Media 					theMedia,
								 long **				offsets,
								 TimeValue 				startTime,
								 TimeValue 				endTime,
								 TimeValue 				timeIncrement,
								 short 					firstDataRefIndex,
								 short 					lastDataRefIndex,
								 long *					retdataRefSkew)						THREEWORDINLINE(0x303C, 0x02BF, 0xAAAA);

EXTERN_API( OSErr )
GetMaxLoadedTimeInMovie			(Movie 					theMovie,
								 TimeValue *			time)								THREEWORDINLINE(0x303C, 0x02C0, 0xAAAA);

EXTERN_API( OSErr )
QTMovieNeedsTimeTable			(Movie 					theMovie,
								 Boolean *				needsTimeTable)						THREEWORDINLINE(0x303C, 0x02C3, 0xAAAA);

EXTERN_API( OSErr )
QTGetDataRefMaxFileOffset		(Movie 					movieH,
								 OSType 				dataRefType,
								 Handle 				dataRef,
								 long *					offset)								THREEWORDINLINE(0x303C, 0x02C6, 0xAAAA);



/*****
	QT International Text Atom Support
*****/

enum {
	kITextRemoveEverythingBut	= 0 << 1,
	kITextRemoveLeaveSuggestedAlternate = 1 << 1
};


enum {
	kITextAtomType				= FOUR_CHAR_CODE('itxt'),
	kITextStringAtomType		= FOUR_CHAR_CODE('text')
};

EXTERN_API( OSErr )
ITextAddString					(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 RegionCode 			theRegionCode,
								 ConstStr255Param 		theString)							THREEWORDINLINE(0x303C, 0x027A, 0xAAAA);

EXTERN_API( OSErr )
ITextRemoveString				(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 RegionCode 			theRegionCode,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x027B, 0xAAAA);

EXTERN_API( OSErr )
ITextGetString					(QTAtomContainer 		container,
								 QTAtom 				parentAtom,
								 RegionCode 			requestedRegion,
								 RegionCode *			foundRegion,
								 StringPtr 				theString)							THREEWORDINLINE(0x303C, 0x027C, 0xAAAA);

EXTERN_API( OSErr )
QTTextToNativeText				(Handle 				theText,
								 long 					encoding,
								 long 					flags)								THREEWORDINLINE(0x303C, 0x02DB, 0xAAAA);

/*************************
* track reference types
**************************/

enum {
	kTrackReferenceChapterList	= FOUR_CHAR_CODE('chap'),
	kTrackReferenceTimeCode		= FOUR_CHAR_CODE('tmcd'),
	kTrackReferenceModifier		= FOUR_CHAR_CODE('ssrc')
};

/*************************
* modifier track types
**************************/

enum {
	kTrackModifierInput			= 0x696E,						/* is really 'in'*/
	kTrackModifierType			= 0x7479,						/* is really 'ty'*/
	kTrackModifierReference		= FOUR_CHAR_CODE('ssrc'),
	kTrackModifierObjectID		= FOUR_CHAR_CODE('obid'),
	kTrackModifierInputName		= FOUR_CHAR_CODE('name')
};


enum {
	kInputMapSubInputID			= FOUR_CHAR_CODE('subi')
};


enum {
	kTrackModifierTypeMatrix	= 1,
	kTrackModifierTypeClip		= 2,
	kTrackModifierTypeGraphicsMode = 5,
	kTrackModifierTypeVolume	= 3,
	kTrackModifierTypeBalance	= 4,
	kTrackModifierTypeImage		= FOUR_CHAR_CODE('vide'),		/* was kTrackModifierTypeSpriteImage*/
	kTrackModifierObjectMatrix	= 6,
	kTrackModifierObjectGraphicsMode = 7,
	kTrackModifierType3d4x4Matrix = 8,
	kTrackModifierCameraData	= 9,
	kTrackModifierSoundLocalizationData = 10,
	kTrackModifierObjectImageIndex = 11,
	kTrackModifierObjectLayer	= 12,
	kTrackModifierObjectVisible	= 13,
	kTrackModifierPanAngle		= FOUR_CHAR_CODE('pan '),
	kTrackModifierTiltAngle		= FOUR_CHAR_CODE('tilt'),
	kTrackModifierVerticalFieldOfViewAngle = FOUR_CHAR_CODE('fov '),
	kTrackModifierObjectQTEventSend = FOUR_CHAR_CODE('evnt')
};


struct ModifierTrackGraphicsModeRecord {
	long 							graphicsMode;
	RGBColor 						opColor;
};
typedef struct ModifierTrackGraphicsModeRecord ModifierTrackGraphicsModeRecord;

/*************************
* tween track types
**************************/

enum {
	kTweenTypeShort				= 1,
	kTweenTypeLong				= 2,
	kTweenTypeFixed				= 3,
	kTweenTypePoint				= 4,
	kTweenTypeQDRect			= 5,
	kTweenTypeQDRegion			= 6,
	kTweenTypeMatrix			= 7,
	kTweenTypeRGBColor			= 8,
	kTweenTypeGraphicsModeWithRGBColor = 9,
	kTweenTypeQTFloatSingle		= 10,
	kTweenTypeQTFloatDouble		= 11,
	kTweenTypeFixedPoint		= 12,
	kTweenType3dScale			= FOUR_CHAR_CODE('3sca'),
	kTweenType3dTranslate		= FOUR_CHAR_CODE('3tra'),
	kTweenType3dRotate			= FOUR_CHAR_CODE('3rot'),
	kTweenType3dRotateAboutPoint = FOUR_CHAR_CODE('3rap'),
	kTweenType3dRotateAboutAxis	= FOUR_CHAR_CODE('3rax'),
	kTweenType3dRotateAboutVector = FOUR_CHAR_CODE('3rvc'),
	kTweenType3dQuaternion		= FOUR_CHAR_CODE('3qua'),
	kTweenType3dMatrix			= FOUR_CHAR_CODE('3mat'),
	kTweenType3dCameraData		= FOUR_CHAR_CODE('3cam'),
	kTweenType3dSoundLocalizationData = FOUR_CHAR_CODE('3slc'),
	kTweenTypePathToMatrixTranslation = FOUR_CHAR_CODE('gxmt'),
	kTweenTypePathToMatrixRotation = FOUR_CHAR_CODE('gxpr'),
	kTweenTypePathToMatrixTranslationAndRotation = FOUR_CHAR_CODE('gxmr'),
	kTweenTypePathToFixedPoint	= FOUR_CHAR_CODE('gxfp'),
	kTweenTypePathXtoY			= FOUR_CHAR_CODE('gxxy'),
	kTweenTypePathYtoX			= FOUR_CHAR_CODE('gxyx'),
	kTweenTypeAtomList			= FOUR_CHAR_CODE('atom'),
	kTweenTypePolygon			= FOUR_CHAR_CODE('poly'),
	kTweenTypeMultiMatrix		= FOUR_CHAR_CODE('mulm'),
	kTweenTypeSpin				= FOUR_CHAR_CODE('spin'),
	kTweenType3dMatrixNonLinear	= FOUR_CHAR_CODE('3nlr'),
	kTweenType3dVRObject		= FOUR_CHAR_CODE('3vro')
};


enum {
	kTweenEntry					= FOUR_CHAR_CODE('twen'),
	kTweenData					= FOUR_CHAR_CODE('data'),
	kTweenType					= FOUR_CHAR_CODE('twnt'),
	kTweenStartOffset			= FOUR_CHAR_CODE('twst'),
	kTweenDuration				= FOUR_CHAR_CODE('twdu'),
	kTweenFlags					= FOUR_CHAR_CODE('flag'),
	kTweenOutputMin				= FOUR_CHAR_CODE('omin'),
	kTweenOutputMax				= FOUR_CHAR_CODE('omax'),
	kTweenSequenceElement		= FOUR_CHAR_CODE('seqe'),
	kTween3dInitialCondition	= FOUR_CHAR_CODE('icnd'),
	kTweenInterpolationID		= FOUR_CHAR_CODE('intr'),
	kTweenRegionData			= FOUR_CHAR_CODE('qdrg'),
	kTweenPictureData			= FOUR_CHAR_CODE('PICT'),
	kListElementType			= FOUR_CHAR_CODE('type'),
	kListElementDataType		= FOUR_CHAR_CODE('daty'),
	kNameAtom					= FOUR_CHAR_CODE('name'),
	kInitialRotationAtom		= FOUR_CHAR_CODE('inro'),
	kNonLinearTweenHeader		= FOUR_CHAR_CODE('nlth')
};

/* kTweenFlags*/

enum {
	kTweenReturnDelta			= 1L << 0
};


struct TweenSequenceEntryRecord {
	Fixed 							endPercent;
	QTAtomID 						tweenAtomID;
	QTAtomID 						dataAtomID;
};
typedef struct TweenSequenceEntryRecord	TweenSequenceEntryRecord;

#ifdef __QD3D__

struct ThreeDeeVRObjectSample {
	long			rows;  
	long    		columns;
	TQ3Matrix4x4  	calib1;
	TQ3Matrix4x4  	calib2;
	long    		reserved1;
	long    		reserved2;
};

typedef struct ThreeDeeVRObjectSample ThreeDeeVRObjectSample;

struct ThreeDeeNonLinearSample {
	float			DurFromLastSample;  // 0 to 1
	TQ3Matrix4x4  	matrix;
};

typedef struct ThreeDeeNonLinearSample ThreeDeeNonLinearSample;

struct ThreeDeeNonLinearTweenHeaderAtom {
	long    number;
	long	dataSize;
	float	tensionFactor;  // default is 0
	long    reserved1;
	long    reserved2;
};

typedef struct ThreeDeeNonLinearTweenHeaderAtom ThreeDeeNonLinearTweenHeaderAtom;

#endif



#if OLDROUTINENAMES
#include <MediaHandlers.h>

/*************************
* Video Media routines
**************************/

#define GetVideoMediaGraphicsMode		MediaGetGraphicsMode
#define SetVideoMediaGraphicsMode		MediaSetGraphicsMode

// use these two routines at your own peril
#define ResetVideoMediaStatistics		VideoMediaResetStatistics
#define GetVideoMediaStatistics			VideoMediaGetStatistics

/*************************
* Sound Media routines
**************************/

#define GetSoundMediaBalance			MediaGetSoundBalance
#define SetSoundMediaBalance			MediaSetSoundBalance

/*************************
* Text Media routines
**************************/

#define SetTextProc			TextMediaSetTextProc
#define AddTextSample		TextMediaAddTextSample
#define AddTESample			TextMediaAddTESample
#define AddHiliteSample		TextMediaAddHiliteSample
#define FindNextText		TextMediaFindNextText
#define HiliteTextSample	TextMediaHiliteTextSample
#define SetTextSampleData	TextMediaSetTextSampleData

/*************************
* Sprite Media routines
**************************/

#define SetSpriteMediaSpriteProperty	SpriteMediaSetProperty
#define GetSpriteMediaSpriteProperty	SpriteMediaGetProperty
#define HitTestSpriteMedia				SpriteMediaHitTestSprites
#define CountSpriteMediaSprites			SpriteMediaCountSprites
#define CountSpriteMediaImages			SpriteMediaCountImages
#define GetSpriteMediaIndImageDescription	SpriteMediaGetIndImageDescription
#define GetDisplayedSampleNumber		SpriteMediaGetDisplayedSampleNumber
#endif /* OLDROUTINENAMES */






/* they are for non-MacOS file manager*/

enum {
	pathTooLongErr				= -2110,
	emptyPathErr				= -2111,
	noPathMappingErr			= -2112,
	pathNotVerifiedErr			= -2113,
	unknownFormatErr			= -2114,
	wackBadFileErr				= -2115,
	wackForkNotFoundErr			= -2116,
	wackBadMetaDataErr			= -2117,
	qfcbNotFoundErr				= -2118,
	qfcbNotCreatedErr			= -2119,
	AAPNotCreatedErr			= -2120,
	AAPNotFoundErr				= -2121,
	ASDBadHeaderErr				= -2122,
	ASDBadForkErr				= -2123,
	ASDEntryNotFoundErr			= -2124
};





/*************************
* Video Media routines
**************************/



enum {
	videoFlagDontLeanAhead		= 1L << 0
};




/* use these three routines at your own peril*/
EXTERN_API( ComponentResult )
VideoMediaResetStatistics		(MediaHandler 			mh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
VideoMediaGetStatistics			(MediaHandler 			mh)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0106, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
VideoMediaGetStallCount			(MediaHandler 			mh,
								 unsigned long *		stalls)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x010E, 0x7000, 0xA82A);



/*************************
* Text Media routines
**************************/



/* Return displayFlags for TextProc */

enum {
	txtProcDefaultDisplay		= 0,							/*	Use the media's default*/
	txtProcDontDisplay			= 1,							/*	Don't display the text*/
	txtProcDoDisplay			= 2								/*	Do display the text*/
};

EXTERN_API( ComponentResult )
TextMediaSetTextProc			(MediaHandler 			mh,
								 TextMediaUPP 			TextProc,
								 long 					refcon)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextMediaAddTextSample			(MediaHandler 			mh,
								 Ptr 					text,
								 unsigned long 			size,
								 short 					fontNumber,
								 short 					fontSize,
								 Style 					textFace,
								 RGBColor *				textColor,
								 RGBColor *				backColor,
								 short 					textJustification,
								 Rect *					textBox,
								 long 					displayFlags,
								 TimeValue 				scrollDelay,
								 short 					hiliteStart,
								 short 					hiliteEnd,
								 RGBColor *				rgbHiliteColor,
								 TimeValue 				duration,
								 TimeValue *			sampleTime)							FIVEWORDINLINE(0x2F3C, 0x0034, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextMediaAddTESample			(MediaHandler 			mh,
								 TEHandle 				hTE,
								 RGBColor *				backColor,
								 short 					textJustification,
								 Rect *					textBox,
								 long 					displayFlags,
								 TimeValue 				scrollDelay,
								 short 					hiliteStart,
								 short 					hiliteEnd,
								 RGBColor *				rgbHiliteColor,
								 TimeValue 				duration,
								 TimeValue *			sampleTime)							FIVEWORDINLINE(0x2F3C, 0x0026, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextMediaAddHiliteSample		(MediaHandler 			mh,
								 short 					hiliteStart,
								 short 					hiliteEnd,
								 RGBColor *				rgbHiliteColor,
								 TimeValue 				duration,
								 TimeValue *			sampleTime)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x0104, 0x7000, 0xA82A);


enum {
	findTextEdgeOK				= 1 << 0,						/* Okay to find text at specified sample time*/
	findTextCaseSensitive		= 1 << 1,						/* Case sensitive search*/
	findTextReverseSearch		= 1 << 2,						/* Search from sampleTime backwards*/
	findTextWrapAround			= 1 << 3,						/* Wrap search when beginning or end of movie is hit*/
	findTextUseOffset			= 1 << 4						/* Begin search at the given character offset into sample rather than edge*/
};

EXTERN_API( ComponentResult )
TextMediaFindNextText			(MediaHandler 			mh,
								 Ptr 					text,
								 long 					size,
								 short 					findFlags,
								 TimeValue 				startTime,
								 TimeValue *			foundTime,
								 TimeValue *			foundDuration,
								 long *					offset)								FIVEWORDINLINE(0x2F3C, 0x001A, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
TextMediaHiliteTextSample		(MediaHandler 			mh,
								 TimeValue 				sampleTime,
								 short 					hiliteStart,
								 short 					hiliteEnd,
								 RGBColor *				rgbHiliteColor)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0106, 0x7000, 0xA82A);


enum {
	dropShadowOffsetType		= FOUR_CHAR_CODE('drpo'),
	dropShadowTranslucencyType	= FOUR_CHAR_CODE('drpt')
};

EXTERN_API( ComponentResult )
TextMediaSetTextSampleData		(MediaHandler 			mh,
								 void *					data,
								 OSType 				dataType)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0107, 0x7000, 0xA82A);





/*************************
* Sprite Media routines
**************************/
/* flags for sprite hit test routines */

enum {
	spriteHitTestBounds			= 1L << 0,						/*	point must only be within sprite's bounding box*/
	spriteHitTestImage			= 1L << 1,						/*  point must be within the shape of the sprite's image*/
	spriteHitTestInvisibleSprites = 1L << 2,					/*  invisible sprites may be hit tested*/
	spriteHitTestIsClick		= 1L << 3,						/*  for codecs that want mouse events*/
	spriteHitTestLocInDisplayCoordinates = 1L << 4				/*	set if you want to pass a display coordiate point to SpriteHitTest*/
};

/* atom types for sprite media */

enum {
	kSpriteAtomType				= FOUR_CHAR_CODE('sprt'),
	kSpriteImagesContainerAtomType = FOUR_CHAR_CODE('imct'),
	kSpriteImageAtomType		= FOUR_CHAR_CODE('imag'),
	kSpriteImageDataAtomType	= FOUR_CHAR_CODE('imda'),
	kSpriteImageGroupIDAtomType	= FOUR_CHAR_CODE('imgr'),
	kSpriteImageRegistrationAtomType = FOUR_CHAR_CODE('imrg'),
	kSpriteSharedDataAtomType	= FOUR_CHAR_CODE('dflt'),
	kSpriteNameAtomType			= FOUR_CHAR_CODE('name'),
	kSpriteImageNameAtomType	= FOUR_CHAR_CODE('name'),
	kSpriteUsesImageIDsAtomType	= FOUR_CHAR_CODE('uses')		/* leaf data is an array of QTAtomID's, one per image used*/
};

EXTERN_API( ComponentResult )
SpriteMediaSetProperty			(MediaHandler 			mh,
								 short 					spriteIndex,
								 long 					propertyType,
								 void *					propertyValue)						FIVEWORDINLINE(0x2F3C, 0x000A, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetProperty			(MediaHandler 			mh,
								 short 					spriteIndex,
								 long 					propertyType,
								 void *					propertyValue)						FIVEWORDINLINE(0x2F3C, 0x000A, 0x0102, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaHitTestSprites		(MediaHandler 			mh,
								 long 					flags,
								 Point 					loc,
								 short *				spriteHitIndex)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x0103, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaCountSprites			(MediaHandler 			mh,
								 short *				numSprites)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0104, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaCountImages			(MediaHandler 			mh,
								 short *				numImages)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0105, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetIndImageDescription (MediaHandler 		mh,
								 short 					imageIndex,
								 ImageDescriptionHandle  imageDescription)					FIVEWORDINLINE(0x2F3C, 0x0006, 0x0106, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetDisplayedSampleNumber (MediaHandler 		mh,
								 long *					sampleNum)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0107, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetSpriteName		(MediaHandler 			mh,
								 QTAtomID 				spriteID,
								 Str255 				spriteName)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0108, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetImageName			(MediaHandler 			mh,
								 short 					imageIndex,
								 Str255 				imageName)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x0109, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaSetSpriteProperty	(MediaHandler 			mh,
								 QTAtomID 				spriteID,
								 long 					propertyType,
								 void *					propertyValue)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x010A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetSpriteProperty	(MediaHandler 			mh,
								 QTAtomID 				spriteID,
								 long 					propertyType,
								 void *					propertyValue)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x010B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaHitTestAllSprites	(MediaHandler 			mh,
								 long 					flags,
								 Point 					loc,
								 QTAtomID *				spriteHitID)						FIVEWORDINLINE(0x2F3C, 0x000C, 0x010C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaHitTestOneSprite		(MediaHandler 			mh,
								 QTAtomID 				spriteID,
								 long 					flags,
								 Point 					loc,
								 Boolean *				wasHit)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x010D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaSpriteIndexToID		(MediaHandler 			mh,
								 short 					spriteIndex,
								 QTAtomID *				spriteID)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x010E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaSpriteIDToIndex		(MediaHandler 			mh,
								 QTAtomID 				spriteID,
								 short *				spriteIndex)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x010F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetSpriteActionsForQTEvent (MediaHandler 	mh,
								 QTEventRecordPtr 		event,
								 QTAtomID 				spriteID,
								 QTAtomContainer *		container,
								 QTAtom *				atom)								FIVEWORDINLINE(0x2F3C, 0x0010, 0x0110, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaSetActionVariable	(MediaHandler 			mh,
								 QTAtomID 				variableID,
								 const float *			value)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0111, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetActionVariable	(MediaHandler 			mh,
								 QTAtomID 				variableID,
								 float *				value)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x0112, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
SpriteMediaGetIndImageProperty	(MediaHandler 			mh,
								 short 					imageIndex,
								 long 					imagePropertyType,
								 void *					imagePropertyValue)					FIVEWORDINLINE(0x2F3C, 0x000A, 0x0113, 0x7000, 0xA82A);


/*************************
* 3D Media routines
**************************/
EXTERN_API( ComponentResult )
Media3DGetNamedObjectList		(MediaHandler 			mh,
								 QTAtomContainer *		objectList)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0101, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
Media3DGetRendererList			(MediaHandler 			mh,
								 QTAtomContainer *		rendererList)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0102, 0x7000, 0xA82A);



/****************************************
*										*
*  	M O V I E   C O N T R O L L E R		*
*										*
****************************************/

enum {
	MovieControllerComponentType = FOUR_CHAR_CODE('play')
};



enum {
	kMovieControllerQTVRFlag	= 1 << 0,
	kMovieControllerDontDisplayToUser = 1 << 1
};


typedef ComponentInstance 				MovieController;

enum {
	mcActionIdle				= 1,							/* no param*/
	mcActionDraw				= 2,							/* param is WindowPtr*/
	mcActionActivate			= 3,							/* no param*/
	mcActionDeactivate			= 4,							/* no param*/
	mcActionMouseDown			= 5,							/* param is pointer to EventRecord*/
	mcActionKey					= 6,							/* param is pointer to EventRecord*/
	mcActionPlay				= 8,							/* param is Fixed, play rate*/
	mcActionGoToTime			= 12,							/* param is TimeRecord*/
	mcActionSetVolume			= 14,							/* param is a short*/
	mcActionGetVolume			= 15,							/* param is pointer to a short*/
	mcActionStep				= 18,							/* param is number of steps (short)*/
	mcActionSetLooping			= 21,							/* param is Boolean*/
	mcActionGetLooping			= 22,							/* param is pointer to a Boolean*/
	mcActionSetLoopIsPalindrome	= 23,							/* param is Boolean*/
	mcActionGetLoopIsPalindrome	= 24,							/* param is pointer to a Boolean*/
	mcActionSetGrowBoxBounds	= 25,							/* param is a Rect*/
	mcActionControllerSizeChanged = 26,							/* no param*/
	mcActionSetSelectionBegin	= 29,							/* param is TimeRecord*/
	mcActionSetSelectionDuration = 30,							/* param is TimeRecord, action only taken on set-duration*/
	mcActionSetKeysEnabled		= 32,							/* param is Boolean*/
	mcActionGetKeysEnabled		= 33,							/* param is pointer to Boolean*/
	mcActionSetPlaySelection	= 34,							/* param is Boolean*/
	mcActionGetPlaySelection	= 35,							/* param is pointer to Boolean*/
	mcActionSetUseBadge			= 36,							/* param is Boolean*/
	mcActionGetUseBadge			= 37,							/* param is pointer to Boolean*/
	mcActionSetFlags			= 38,							/* param is long of flags*/
	mcActionGetFlags			= 39,							/* param is pointer to a long of flags*/
	mcActionSetPlayEveryFrame	= 40,							/* param is Boolean*/
	mcActionGetPlayEveryFrame	= 41,							/* param is pointer to Boolean*/
	mcActionGetPlayRate			= 42,							/* param is pointer to Fixed*/
	mcActionShowBalloon			= 43,							/* param is a pointer to a boolean. set to false to stop balloon*/
	mcActionBadgeClick			= 44,							/* param is pointer to Boolean. set to false to ignore click*/
	mcActionMovieClick			= 45,							/* param is pointer to event record. change "what" to nullEvt to kill click*/
	mcActionSuspend				= 46,							/* no param*/
	mcActionResume				= 47,							/* no param*/
	mcActionSetControllerKeysEnabled = 48,						/* param is Boolean*/
	mcActionGetTimeSliderRect	= 49,							/* param is pointer to rect*/
	mcActionMovieEdited			= 50,							/* no param*/
	mcActionGetDragEnabled		= 51,							/* param is pointer to Boolean*/
	mcActionSetDragEnabled		= 52,							/* param is Boolean*/
	mcActionGetSelectionBegin	= 53,							/* param is TimeRecord*/
	mcActionGetSelectionDuration = 54,							/* param is TimeRecord*/
	mcActionPrerollAndPlay		= 55,							/* param is Fixed, play rate*/
	mcActionGetCursorSettingEnabled = 56,						/* param is pointer to Boolean*/
	mcActionSetCursorSettingEnabled = 57,						/* param is Boolean*/
	mcActionSetColorTable		= 58,							/* param is CTabHandle*/
	mcActionLinkToURL			= 59,							/* param is Handle to URL*/
	mcActionCustomButtonClick	= 60,							/* param is pointer to EventRecord*/
	mcActionForceTimeTableUpdate = 61,							/* no param*/
	mcActionSetControllerTimeLimits = 62,						/* param is pointer to 2 time values min/max. do no send this message to controller. used internally only.*/
	mcActionExecuteAllActionsForQTEvent = 63,					/* param is ResolvedQTEventSpecPtr*/
	mcActionExecuteOneActionForQTEvent = 64,					/* param is ResolvedQTEventSpecPtr*/
	mcActionAdjustCursor		= 65,							/* param is pointer to EventRecord (WindowPtr is in message parameter)*/
	mcActionUseTrackForTimeTable = 66,							/* param is pointer to {long trackID; Boolean useIt}. do not send this message to controller. */
	mcActionClickAndHoldPoint	= 67,							/* param is point (local coordinates). return true if point has click & hold action (e.g., VR object movie autorotate spot)*/
	mcActionShowMessageString	= 68							/* param is a StringPtr*/
};

typedef short 							mcAction;

enum {
	mcFlagSuppressMovieFrame	= 1 << 0,
	mcFlagSuppressStepButtons	= 1 << 1,
	mcFlagSuppressSpeakerButton	= 1 << 2,
	mcFlagsUseWindowPalette		= 1 << 3,
	mcFlagsDontInvalidate		= 1 << 4,
	mcFlagsUseCustomButton		= 1 << 5
};



enum {
	mcPositionDontInvalidate	= 1 << 5
};

typedef unsigned long 					mcFlags;

enum {
	kMCIEEnabledButtonPicture	= 1,
	kMCIEDisabledButtonPicture	= 2,
	kMCIEDepressedButtonPicture	= 3,
	kMCIEEnabledSizeBoxPicture	= 4,
	kMCIEDisabledSizeBoxPicture	= 5,
	kMCIEEnabledUnavailableButtonPicture = 6,
	kMCIEDisabledUnavailableButtonPicture = 7,
	kMCIESoundSlider			= 128,
	kMCIESoundThumb				= 129,
	kMCIEColorTable				= 256,
	kMCIEIsFlatAppearance		= 257,
	kMCIEDoButtonIconsDropOnDepress = 258
};

typedef unsigned long 					MCInterfaceElement;
typedef CALLBACK_API( Boolean , MCActionFilterProcPtr )(MovieController mc, short *action, void *params);
typedef CALLBACK_API( Boolean , MCActionFilterWithRefConProcPtr )(MovieController mc, short action, void *params, long refCon);
typedef STACK_UPP_TYPE(MCActionFilterProcPtr) 					MCActionFilterUPP;
typedef STACK_UPP_TYPE(MCActionFilterWithRefConProcPtr) 		MCActionFilterWithRefConUPP;
/*
	menu related stuff
*/

enum {
	mcInfoUndoAvailable			= 1 << 0,
	mcInfoCutAvailable			= 1 << 1,
	mcInfoCopyAvailable			= 1 << 2,
	mcInfoPasteAvailable		= 1 << 3,
	mcInfoClearAvailable		= 1 << 4,
	mcInfoHasSound				= 1 << 5,
	mcInfoIsPlaying				= 1 << 6,
	mcInfoIsLooping				= 1 << 7,
	mcInfoIsInPalindrome		= 1 << 8,
	mcInfoEditingEnabled		= 1 << 9,
	mcInfoMovieIsInteractive	= 1 << 10
};

/* menu item codes*/

enum {
	mcMenuUndo					= 1,
	mcMenuCut					= 3,
	mcMenuCopy					= 4,
	mcMenuPaste					= 5,
	mcMenuClear					= 6
};




/* target management */
EXTERN_API( ComponentResult )
MCSetMovie						(MovieController 		mc,
								 Movie 					theMovie,
								 WindowPtr 				movieWindow,
								 Point 					where)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0002, 0x7000, 0xA82A);

EXTERN_API( Movie )
MCGetIndMovie					(MovieController 		mc,
								 short 					index)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x0005, 0x7000, 0xA82A);


#define MCGetMovie(mc) MCGetIndMovie(mc, 0)
EXTERN_API( ComponentResult )
MCRemoveAllMovies				(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0006, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCRemoveAMovie					(MovieController 		mc,
								 Movie 					m)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0003, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCRemoveMovie					(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0006, 0x7000, 0xA82A);

/* event handling etc. */
EXTERN_API( ComponentResult )
MCIsPlayerEvent					(MovieController 		mc,
								 const EventRecord *	e)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0007, 0x7000, 0xA82A);

/* obsolete. use MCSetActionFilterWithRefCon instead. */
EXTERN_API( ComponentResult )
MCSetActionFilter				(MovieController 		mc,
								 MCActionFilterUPP 		blob)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0008, 0x7000, 0xA82A);

/*
	proc is of the form:
		Boolean userPlayerFilter(MovieController mc, short *action, void *params) =
	proc returns TRUE if it handles the action, FALSE if not
	action is passed as a VAR so that it could be changed by filter
	this is consistent with the current dialog manager stuff
	params is any potential parameters that go with the action
		such as set playback rate to xxx.
*/
EXTERN_API( ComponentResult )
MCDoAction						(MovieController 		mc,
								 short 					action,
								 void *					params)								FIVEWORDINLINE(0x2F3C, 0x0006, 0x0009, 0x7000, 0xA82A);

/* state type things */
EXTERN_API( ComponentResult )
MCSetControllerAttached			(MovieController 		mc,
								 Boolean 				attach)								FIVEWORDINLINE(0x2F3C, 0x0002, 0x000A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCIsControllerAttached			(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCSetControllerPort				(MovieController 		mc,
								 CGrafPtr 				gp)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x000C, 0x7000, 0xA82A);

EXTERN_API( CGrafPtr )
MCGetControllerPort				(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCSetVisible					(MovieController 		mc,
								 Boolean 				visible)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x000E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCGetVisible					(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x000F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCGetControllerBoundsRect		(MovieController 		mc,
								 Rect *					bounds)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0010, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCSetControllerBoundsRect		(MovieController 		mc,
								 const Rect *			bounds)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0011, 0x7000, 0xA82A);

EXTERN_API( RgnHandle )
MCGetControllerBoundsRgn		(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0012, 0x7000, 0xA82A);

EXTERN_API( RgnHandle )
MCGetWindowRgn					(MovieController 		mc,
								 WindowPtr 				w)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0013, 0x7000, 0xA82A);


/* other stuff */
EXTERN_API( ComponentResult )
MCMovieChanged					(MovieController 		mc,
								 Movie 					m)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0014, 0x7000, 0xA82A);


/*
	called when the app has changed thing about the movie (like bounding rect) or rate. So that we
		can update our graphical (and internal) state accordingly.
*/
EXTERN_API( ComponentResult )
MCSetDuration					(MovieController 		mc,
								 TimeValue 				duration)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0015, 0x7000, 0xA82A);

/*
	duration to use for time slider -- will be reset next time MCMovieChanged is called
		or MCSetMovie is called
*/
EXTERN_API( TimeValue )
MCGetCurrentTime				(MovieController 		mc,
								 TimeScale *			scale)								FIVEWORDINLINE(0x2F3C, 0x0004, 0x0016, 0x7000, 0xA82A);

/*
	returns the time value and the time scale it is on. if there are no movies, the
		time scale is passed back as 0. scale is an optional parameter

*/
EXTERN_API( ComponentResult )
MCNewAttachedController			(MovieController 		mc,
								 Movie 					theMovie,
								 WindowPtr 				w,
								 Point 					where)								FIVEWORDINLINE(0x2F3C, 0x000C, 0x0017, 0x7000, 0xA82A);

/*
	makes theMovie the only movie attached to the controller. makes the controller visible.
	the window and where parameters are passed a long to MCSetMovie and behave as
	described there
*/
EXTERN_API( ComponentResult )
MCDraw							(MovieController 		mc,
								 WindowPtr 				w)									FIVEWORDINLINE(0x2F3C, 0x0004, 0x0018, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCActivate						(MovieController 		mc,
								 WindowPtr 				w,
								 Boolean 				activate)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x0019, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCIdle							(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x001A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCKey							(MovieController 		mc,
								 SInt8 					key,
								 long 					modifiers)							FIVEWORDINLINE(0x2F3C, 0x0006, 0x001B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCClick							(MovieController 		mc,
								 WindowPtr 				w,
								 Point 					where,
								 long 					when,
								 long 					modifiers)							FIVEWORDINLINE(0x2F3C, 0x0010, 0x001C, 0x7000, 0xA82A);


/*
	calls for editing
*/
EXTERN_API( ComponentResult )
MCEnableEditing					(MovieController 		mc,
								 Boolean 				enabled)							FIVEWORDINLINE(0x2F3C, 0x0002, 0x001D, 0x7000, 0xA82A);

EXTERN_API( long )
MCIsEditingEnabled				(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x001E, 0x7000, 0xA82A);

EXTERN_API( Movie )
MCCopy							(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x001F, 0x7000, 0xA82A);

EXTERN_API( Movie )
MCCut							(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0020, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCPaste							(MovieController 		mc,
								 Movie 					srcMovie)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0021, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCClear							(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0022, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCUndo							(MovieController 		mc)									FIVEWORDINLINE(0x2F3C, 0x0000, 0x0023, 0x7000, 0xA82A);


/*
 *	somewhat special stuff
 */
EXTERN_API( ComponentResult )
MCPositionController			(MovieController 		mc,
								 const Rect *			movieRect,
								 const Rect *			controllerRect,
								 long 					someFlags)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0024, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
MCGetControllerInfo				(MovieController 		mc,
								 long *					someFlags)							FIVEWORDINLINE(0x2F3C, 0x0004, 0x0025, 0x7000, 0xA82A);



EXTERN_API( ComponentResult )
MCSetClip						(MovieController 		mc,
								 RgnHandle 				theClip,
								 RgnHandle 				movieClip)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0028, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCGetClip						(MovieController 		mc,
								 RgnHandle *			theClip,
								 RgnHandle *			movieClip)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0029, 0x7000, 0xA82A);


EXTERN_API( ComponentResult )
MCDrawBadge						(MovieController 		mc,
								 RgnHandle 				movieRgn,
								 RgnHandle *			badgeRgn)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x002A, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCSetUpEditMenu					(MovieController 		mc,
								 long 					modifiers,
								 MenuHandle 			mh)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x002B, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCGetMenuString					(MovieController 		mc,
								 long 					modifiers,
								 short 					item,
								 Str255 				aString)							FIVEWORDINLINE(0x2F3C, 0x000A, 0x002C, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCSetActionFilterWithRefCon		(MovieController 		mc,
								 MCActionFilterWithRefConUPP  blob,
								 long 					refCon)								FIVEWORDINLINE(0x2F3C, 0x0008, 0x002D, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCPtInController				(MovieController 		mc,
								 Point 					thePt,
								 Boolean *				inController)						FIVEWORDINLINE(0x2F3C, 0x0008, 0x002E, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCInvalidate					(MovieController 		mc,
								 WindowPtr 				w,
								 RgnHandle 				invalidRgn)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x002F, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCAdjustCursor					(MovieController 		mc,
								 WindowPtr 				w,
								 Point 					where,
								 long 					modifiers)							FIVEWORDINLINE(0x2F3C, 0x000C, 0x0030, 0x7000, 0xA82A);

EXTERN_API( ComponentResult )
MCGetInterfaceElement			(MovieController 		mc,
								 MCInterfaceElement 	whichElement,
								 void *					element)							FIVEWORDINLINE(0x2F3C, 0x0008, 0x0031, 0x7000, 0xA82A);





/****************************************
*										*
*  		T  I  M  E  B  A  S  E			*
*										*
****************************************/
EXTERN_API( TimeBase )
NewTimeBase						(void)														THREEWORDINLINE(0x303C, 0x00A5, 0xAAAA);

EXTERN_API( void )
DisposeTimeBase					(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x00B6, 0xAAAA);

EXTERN_API( TimeValue )
GetTimeBaseTime					(TimeBase 				tb,
								 TimeScale 				s,
								 TimeRecord *			tr)									THREEWORDINLINE(0x303C, 0x00A6, 0xAAAA);

EXTERN_API( void )
SetTimeBaseTime					(TimeBase 				tb,
								 const TimeRecord *		tr)									THREEWORDINLINE(0x303C, 0x00A7, 0xAAAA);

EXTERN_API( void )
SetTimeBaseValue				(TimeBase 				tb,
								 TimeValue 				t,
								 TimeScale 				s)									THREEWORDINLINE(0x303C, 0x00A8, 0xAAAA);

EXTERN_API( Fixed )
GetTimeBaseRate					(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x00A9, 0xAAAA);

EXTERN_API( void )
SetTimeBaseRate					(TimeBase 				tb,
								 Fixed 					r)									THREEWORDINLINE(0x303C, 0x00AA, 0xAAAA);

EXTERN_API( TimeValue )
GetTimeBaseStartTime			(TimeBase 				tb,
								 TimeScale 				s,
								 TimeRecord *			tr)									THREEWORDINLINE(0x303C, 0x00AB, 0xAAAA);

EXTERN_API( void )
SetTimeBaseStartTime			(TimeBase 				tb,
								 const TimeRecord *		tr)									THREEWORDINLINE(0x303C, 0x00AC, 0xAAAA);

EXTERN_API( TimeValue )
GetTimeBaseStopTime				(TimeBase 				tb,
								 TimeScale 				s,
								 TimeRecord *			tr)									THREEWORDINLINE(0x303C, 0x00AD, 0xAAAA);

EXTERN_API( void )
SetTimeBaseStopTime				(TimeBase 				tb,
								 const TimeRecord *		tr)									THREEWORDINLINE(0x303C, 0x00AE, 0xAAAA);

EXTERN_API( long )
GetTimeBaseFlags				(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x00B1, 0xAAAA);

EXTERN_API( void )
SetTimeBaseFlags				(TimeBase 				tb,
								 long 					timeBaseFlags)						THREEWORDINLINE(0x303C, 0x00B2, 0xAAAA);

EXTERN_API( void )
SetTimeBaseMasterTimeBase		(TimeBase 				slave,
								 TimeBase 				master,
								 const TimeRecord *		slaveZero)							THREEWORDINLINE(0x303C, 0x00B4, 0xAAAA);

EXTERN_API( TimeBase )
GetTimeBaseMasterTimeBase		(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x00AF, 0xAAAA);

EXTERN_API( void )
SetTimeBaseMasterClock			(TimeBase 				slave,
								 Component 				clockMeister,
								 const TimeRecord *		slaveZero)							THREEWORDINLINE(0x303C, 0x00B3, 0xAAAA);

EXTERN_API( ComponentInstance )
GetTimeBaseMasterClock			(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x00B0, 0xAAAA);

EXTERN_API( void )
ConvertTime						(TimeRecord *			inout,
								 TimeBase 				newBase)							THREEWORDINLINE(0x303C, 0x00B5, 0xAAAA);

EXTERN_API( void )
ConvertTimeScale				(TimeRecord *			inout,
								 TimeScale 				newScale)							THREEWORDINLINE(0x303C, 0x00B7, 0xAAAA);

EXTERN_API( void )
AddTime							(TimeRecord *			dst,
								 const TimeRecord *		src)								THREEWORDINLINE(0x303C, 0x010C, 0xAAAA);

EXTERN_API( void )
SubtractTime					(TimeRecord *			dst,
								 const TimeRecord *		src)								THREEWORDINLINE(0x303C, 0x010D, 0xAAAA);

EXTERN_API( long )
GetTimeBaseStatus				(TimeBase 				tb,
								 TimeRecord *			unpinnedTime)						THREEWORDINLINE(0x303C, 0x010B, 0xAAAA);

EXTERN_API( void )
SetTimeBaseZero					(TimeBase 				tb,
								 TimeRecord *			zero)								THREEWORDINLINE(0x303C, 0x0128, 0xAAAA);

EXTERN_API( Fixed )
GetTimeBaseEffectiveRate		(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x0124, 0xAAAA);


/****************************************
*										*
*  		C  A  L  L  B  A  C  K 			*
*										*
****************************************/
EXTERN_API( QTCallBack )
NewCallBack						(TimeBase 				tb,
								 short 					cbType)								THREEWORDINLINE(0x303C, 0x00EB, 0xAAAA);

EXTERN_API( void )
DisposeCallBack					(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x00EC, 0xAAAA);

EXTERN_API( short )
GetCallBackType					(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x00ED, 0xAAAA);

EXTERN_API( TimeBase )
GetCallBackTimeBase				(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x00EE, 0xAAAA);

EXTERN_API( OSErr )
CallMeWhen						(QTCallBack 			cb,
								 QTCallBackUPP 			callBackProc,
								 long 					refCon,
								 long 					param1,
								 long 					param2,
								 long 					param3)								THREEWORDINLINE(0x303C, 0x00B8, 0xAAAA);

EXTERN_API( void )
CancelCallBack					(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x00B9, 0xAAAA);


/****************************************
*										*
*  		C L O C K   C A L L B A C K  	*
*  		      S U P P O R T  			*
*										*
****************************************/
EXTERN_API( OSErr )
AddCallBackToTimeBase			(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x0129, 0xAAAA);

EXTERN_API( OSErr )
RemoveCallBackFromTimeBase		(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x012A, 0xAAAA);

EXTERN_API( QTCallBack )
GetFirstCallBack				(TimeBase 				tb)									THREEWORDINLINE(0x303C, 0x012B, 0xAAAA);

EXTERN_API( QTCallBack )
GetNextCallBack					(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x012C, 0xAAAA);

EXTERN_API( void )
ExecuteCallBack					(QTCallBack 			cb)									THREEWORDINLINE(0x303C, 0x012D, 0xAAAA);





EXTERN_API( ComponentResult )
MusicMediaGetIndexedTunePlayer	(ComponentInstance 		ti,
								 long 					sampleDescIndex,
								 ComponentInstance *	tp)									FIVEWORDINLINE(0x2F3C, 0x0008, 0x0101, 0x7000, 0xA82A);



/* UPP call backs */
enum { uppMovieRgnCoverProcInfo = 0x00000FE0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes) */
enum { uppMovieProgressProcInfo = 0x0000FAE0 }; 				/* pascal 2_bytes Func(4_bytes, 2_bytes, 2_bytes, 4_bytes, 4_bytes) */
enum { uppMovieDrawingCompleteProcInfo = 0x000003E0 }; 			/* pascal 2_bytes Func(4_bytes, 4_bytes) */
enum { uppTrackTransferProcInfo = 0x000003E0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes) */
enum { uppGetMovieProcInfo = 0x00003FE0 }; 						/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppMoviePreviewCallOutProcInfo = 0x000000D0 }; 			/* pascal 1_byte Func(4_bytes) */
enum { uppTextMediaProcInfo = 0x00003FE0 }; 					/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppActionsProcInfo = 0x00003FE0 }; 						/* pascal 2_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppQTCallBackProcInfo = 0x000003C0 }; 					/* pascal no_return_value Func(4_bytes, 4_bytes) */
enum { uppQTSyncTaskProcInfo = 0x000000C0 }; 					/* pascal no_return_value Func(4_bytes) */
enum { uppTweenerDataProcInfo = 0x003FFFF0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppMCActionFilterProcInfo = 0x00000FD0 }; 				/* pascal 1_byte Func(4_bytes, 4_bytes, 4_bytes) */
enum { uppMCActionFilterWithRefConProcInfo = 0x00003ED0 }; 		/* pascal 1_byte Func(4_bytes, 2_bytes, 4_bytes, 4_bytes) */
#define NewMovieRgnCoverProc(userRoutine) 						(MovieRgnCoverUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMovieRgnCoverProcInfo, GetCurrentArchitecture())
#define NewMovieProgressProc(userRoutine) 						(MovieProgressUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMovieProgressProcInfo, GetCurrentArchitecture())
#define NewMovieDrawingCompleteProc(userRoutine) 				(MovieDrawingCompleteUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMovieDrawingCompleteProcInfo, GetCurrentArchitecture())
#define NewTrackTransferProc(userRoutine) 						(TrackTransferUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTrackTransferProcInfo, GetCurrentArchitecture())
#define NewGetMovieProc(userRoutine) 							(GetMovieUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppGetMovieProcInfo, GetCurrentArchitecture())
#define NewMoviePreviewCallOutProc(userRoutine) 				(MoviePreviewCallOutUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMoviePreviewCallOutProcInfo, GetCurrentArchitecture())
#define NewTextMediaProc(userRoutine) 							(TextMediaUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTextMediaProcInfo, GetCurrentArchitecture())
#define NewActionsProc(userRoutine) 							(ActionsUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppActionsProcInfo, GetCurrentArchitecture())
#define NewQTCallBackProc(userRoutine) 							(QTCallBackUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQTCallBackProcInfo, GetCurrentArchitecture())
#define NewQTSyncTaskProc(userRoutine) 							(QTSyncTaskUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppQTSyncTaskProcInfo, GetCurrentArchitecture())
#define NewTweenerDataProc(userRoutine) 						(TweenerDataUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTweenerDataProcInfo, GetCurrentArchitecture())
#define NewMCActionFilterProc(userRoutine) 						(MCActionFilterUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMCActionFilterProcInfo, GetCurrentArchitecture())
#define NewMCActionFilterWithRefConProc(userRoutine) 			(MCActionFilterWithRefConUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMCActionFilterWithRefConProcInfo, GetCurrentArchitecture())
#define CallMovieRgnCoverProc(userRoutine, theMovie, changedRgn, refcon)  CALL_THREE_PARAMETER_UPP((userRoutine), uppMovieRgnCoverProcInfo, (theMovie), (changedRgn), (refcon))
#define CallMovieProgressProc(userRoutine, theMovie, message, whatOperation, percentDone, refcon)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppMovieProgressProcInfo, (theMovie), (message), (whatOperation), (percentDone), (refcon))
#define CallMovieDrawingCompleteProc(userRoutine, theMovie, refCon)  CALL_TWO_PARAMETER_UPP((userRoutine), uppMovieDrawingCompleteProcInfo, (theMovie), (refCon))
#define CallTrackTransferProc(userRoutine, t, refCon) 			CALL_TWO_PARAMETER_UPP((userRoutine), uppTrackTransferProcInfo, (t), (refCon))
#define CallGetMovieProc(userRoutine, offset, size, dataPtr, refCon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppGetMovieProcInfo, (offset), (size), (dataPtr), (refCon))
#define CallMoviePreviewCallOutProc(userRoutine, refcon) 		CALL_ONE_PARAMETER_UPP((userRoutine), uppMoviePreviewCallOutProcInfo, (refcon))
#define CallTextMediaProc(userRoutine, theText, theMovie, displayFlag, refcon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppTextMediaProcInfo, (theText), (theMovie), (displayFlag), (refcon))
#define CallActionsProc(userRoutine, refcon, targetTrack, targetRefCon, theEvent)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppActionsProcInfo, (refcon), (targetTrack), (targetRefCon), (theEvent))
#define CallQTCallBackProc(userRoutine, cb, refCon) 			CALL_TWO_PARAMETER_UPP((userRoutine), uppQTCallBackProcInfo, (cb), (refCon))
#define CallQTSyncTaskProc(userRoutine, task) 					CALL_ONE_PARAMETER_UPP((userRoutine), uppQTSyncTaskProcInfo, (task))
#define CallTweenerDataProc(userRoutine, tr, tweenData, tweenDataSize, dataDescriptionSeed, dataDescription, asyncCompletionProc, transferProc, refCon)  CALL_EIGHT_PARAMETER_UPP((userRoutine), uppTweenerDataProcInfo, (tr), (tweenData), (tweenDataSize), (dataDescriptionSeed), (dataDescription), (asyncCompletionProc), (transferProc), (refCon))
#define CallMCActionFilterProc(userRoutine, mc, action, params)  CALL_THREE_PARAMETER_UPP((userRoutine), uppMCActionFilterProcInfo, (mc), (action), (params))
#define CallMCActionFilterWithRefConProc(userRoutine, mc, action, params, refCon)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppMCActionFilterWithRefConProcInfo, (mc), (action), (params), (refCon))

/* selectors for component calls */
enum {
	kVideoMediaResetStatisticsSelect				= 0x0105,
	kVideoMediaGetStatisticsSelect					= 0x0106,
	kVideoMediaGetStallCountSelect					= 0x010E,
	kTextMediaSetTextProcSelect						= 0x0101,
	kTextMediaAddTextSampleSelect					= 0x0102,
	kTextMediaAddTESampleSelect						= 0x0103,
	kTextMediaAddHiliteSampleSelect					= 0x0104,
	kTextMediaFindNextTextSelect					= 0x0105,
	kTextMediaHiliteTextSampleSelect				= 0x0106,
	kTextMediaSetTextSampleDataSelect				= 0x0107,
	kSpriteMediaSetPropertySelect					= 0x0101,
	kSpriteMediaGetPropertySelect					= 0x0102,
	kSpriteMediaHitTestSpritesSelect				= 0x0103,
	kSpriteMediaCountSpritesSelect					= 0x0104,
	kSpriteMediaCountImagesSelect					= 0x0105,
	kSpriteMediaGetIndImageDescriptionSelect		= 0x0106,
	kSpriteMediaGetDisplayedSampleNumberSelect		= 0x0107,
	kSpriteMediaGetSpriteNameSelect					= 0x0108,
	kSpriteMediaGetImageNameSelect					= 0x0109,
	kSpriteMediaSetSpritePropertySelect				= 0x010A,
	kSpriteMediaGetSpritePropertySelect				= 0x010B,
	kSpriteMediaHitTestAllSpritesSelect				= 0x010C,
	kSpriteMediaHitTestOneSpriteSelect				= 0x010D,
	kSpriteMediaSpriteIndexToIDSelect				= 0x010E,
	kSpriteMediaSpriteIDToIndexSelect				= 0x010F,
	kSpriteMediaGetSpriteActionsForQTEventSelect	= 0x0110,
	kSpriteMediaSetActionVariableSelect				= 0x0111,
	kSpriteMediaGetActionVariableSelect				= 0x0112,
	kSpriteMediaGetIndImagePropertySelect			= 0x0113,
	kMedia3DGetNamedObjectListSelect				= 0x0101,
	kMedia3DGetRendererListSelect					= 0x0102,
	kMCSetMovieSelect								= 0x0002,
	kMCGetIndMovieSelect							= 0x0005,
	kMCRemoveAllMoviesSelect						= 0x0006,
	kMCRemoveAMovieSelect							= 0x0003,
	kMCRemoveMovieSelect							= 0x0006,
	kMCIsPlayerEventSelect							= 0x0007,
	kMCSetActionFilterSelect						= 0x0008,
	kMCDoActionSelect								= 0x0009,
	kMCSetControllerAttachedSelect					= 0x000A,
	kMCIsControllerAttachedSelect					= 0x000B,
	kMCSetControllerPortSelect						= 0x000C,
	kMCGetControllerPortSelect						= 0x000D,
	kMCSetVisibleSelect								= 0x000E,
	kMCGetVisibleSelect								= 0x000F,
	kMCGetControllerBoundsRectSelect				= 0x0010,
	kMCSetControllerBoundsRectSelect				= 0x0011,
	kMCGetControllerBoundsRgnSelect					= 0x0012,
	kMCGetWindowRgnSelect							= 0x0013,
	kMCMovieChangedSelect							= 0x0014,
	kMCSetDurationSelect							= 0x0015,
	kMCGetCurrentTimeSelect							= 0x0016,
	kMCNewAttachedControllerSelect					= 0x0017,
	kMCDrawSelect									= 0x0018,
	kMCActivateSelect								= 0x0019,
	kMCIdleSelect									= 0x001A,
	kMCKeySelect									= 0x001B,
	kMCClickSelect									= 0x001C,
	kMCEnableEditingSelect							= 0x001D,
	kMCIsEditingEnabledSelect						= 0x001E,
	kMCCopySelect									= 0x001F,
	kMCCutSelect									= 0x0020,
	kMCPasteSelect									= 0x0021,
	kMCClearSelect									= 0x0022,
	kMCUndoSelect									= 0x0023,
	kMCPositionControllerSelect						= 0x0024,
	kMCGetControllerInfoSelect						= 0x0025,
	kMCSetClipSelect								= 0x0028,
	kMCGetClipSelect								= 0x0029,
	kMCDrawBadgeSelect								= 0x002A,
	kMCSetUpEditMenuSelect							= 0x002B,
	kMCGetMenuStringSelect							= 0x002C,
	kMCSetActionFilterWithRefConSelect				= 0x002D,
	kMCPtInControllerSelect							= 0x002E,
	kMCInvalidateSelect								= 0x002F,
	kMCAdjustCursorSelect							= 0x0030,
	kMCGetInterfaceElementSelect					= 0x0031,
	kMusicMediaGetIndexedTunePlayerSelect			= 0x0101
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

#endif /* __MOVIES__ */

