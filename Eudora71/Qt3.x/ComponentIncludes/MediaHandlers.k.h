/*
 	File:		MediaHandlers.k.h
 
 	Contains:	QuickTime interfaces
 
 	Version:	Technology:	
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __MEDIAHANDLERS_K__
#define __MEDIAHANDLERS_K__

#include <MediaHandlers.h>

/*
	Example usage:

		#define MEDIA_BASENAME()	Fred
		#define MEDIA_GLOBALS()	FredGlobalsHandle
		#include <MediaHandlers.k.h>

	To specify that your component implementation does not use globals, do not #define MEDIA_GLOBALS
*/
#ifdef MEDIA_BASENAME
	#ifndef MEDIA_GLOBALS
		#define MEDIA_GLOBALS() 
		#define ADD_MEDIA_COMMA 
	#else
		#define ADD_MEDIA_COMMA ,
	#endif
	#define MEDIA_GLUE(a,b) a##b
	#define MEDIA_STRCAT(a,b) MEDIA_GLUE(a,b)
	#define ADD_MEDIA_BASENAME(name) MEDIA_STRCAT(MEDIA_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(Initialize) (MEDIA_GLOBALS() ADD_MEDIA_COMMA GetMovieCompleteParams * gmc);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetHandlerCapabilities) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  flags, long  flagsMask);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(Idle) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeValue  atMediaTime, long  flagsIn, long * flagsOut, const TimeRecord * movieTime);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetMediaInfo) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Handle  h);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(PutMediaInfo) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Handle  h);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetActive) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Boolean  enableMedia);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetRate) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Fixed  rate);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GGetStatus) (MEDIA_GLOBALS() ADD_MEDIA_COMMA ComponentResult * statusErr);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(TrackEdited) (MEDIA_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetMediaTimeScale) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeScale  newTimeScale);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetMovieTimeScale) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeScale  newTimeScale);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetGWorld) (MEDIA_GLOBALS() ADD_MEDIA_COMMA CGrafPtr  aPort, GDHandle  aGD);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetDimensions) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Fixed  width, Fixed  height);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetClip) (MEDIA_GLOBALS() ADD_MEDIA_COMMA RgnHandle  theClip);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetMatrix) (MEDIA_GLOBALS() ADD_MEDIA_COMMA MatrixRecord * trackMovieMatrix);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetTrackOpaque) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Boolean * trackIsOpaque);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetGraphicsMode) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  mode, const RGBColor * opColor);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetGraphicsMode) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long * mode, RGBColor * opColor);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GSetVolume) (MEDIA_GLOBALS() ADD_MEDIA_COMMA short  volume);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetSoundBalance) (MEDIA_GLOBALS() ADD_MEDIA_COMMA short  balance);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetSoundBalance) (MEDIA_GLOBALS() ADD_MEDIA_COMMA short * balance);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetNextBoundsChange) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeValue * when);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetSrcRgn) (MEDIA_GLOBALS() ADD_MEDIA_COMMA RgnHandle  rgn, TimeValue  atMediaTime);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(Preroll) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeValue  time, Fixed  rate);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SampleDescriptionChanged) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  index);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(HasCharacteristic) (MEDIA_GLOBALS() ADD_MEDIA_COMMA OSType  characteristic, Boolean * hasIt);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetOffscreenBufferSize) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Rect * bounds, short  depth, CTabHandle  ctab);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetHints) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  hints);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetName) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Str255  name, long  requestedLanguage, long * actualLanguage);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(ForceUpdate) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  forceUpdateFlags);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetDrawingRgn) (MEDIA_GLOBALS() ADD_MEDIA_COMMA RgnHandle * partialRgn);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GSetActiveSegment) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeValue  activeStart, TimeValue  activeDuration);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(InvalidateRegion) (MEDIA_GLOBALS() ADD_MEDIA_COMMA RgnHandle  invalRgn);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetNextStepTime) (MEDIA_GLOBALS() ADD_MEDIA_COMMA short  flags, TimeValue  mediaTimeIn, TimeValue * mediaTimeOut, Fixed  rate);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetNonPrimarySourceData) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  inputIndex, long  dataDescriptionSeed, Handle  dataDescription, void * data, long  dataSize, ICMCompletionProcRecordPtr  asyncCompletionProc, UniversalProcPtr  transferProc, void * refCon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(ChangedNonPrimarySource) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  inputIndex);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(TrackReferencesChanged) (MEDIA_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetSampleDataPointer) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  sampleNum, Ptr * dataPtr, long * dataSize, long * sampleDescIndex);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(ReleaseSampleDataPointer) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  sampleNum);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(TrackPropertyAtomChanged) (MEDIA_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetTrackInputMapReference) (MEDIA_GLOBALS() ADD_MEDIA_COMMA QTAtomContainer  inputMap);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetVideoParam) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  whichParam, unsigned short * value);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetVideoParam) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  whichParam, unsigned short * value);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(Compare) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Boolean * isOK, Media  srcMedia, ComponentInstance  srcMediaComponent);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetClock) (MEDIA_GLOBALS() ADD_MEDIA_COMMA ComponentInstance * clock);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetSoundOutputComponent) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Component  outputComponent);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetSoundOutputComponent) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Component * outputComponent);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetSoundLocalizationData) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Handle  data);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetInvalidRegion) (MEDIA_GLOBALS() ADD_MEDIA_COMMA RgnHandle  rgn);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SampleDescriptionB2N) (MEDIA_GLOBALS() ADD_MEDIA_COMMA SampleDescriptionHandle  sampleDescriptionH);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SampleDescriptionN2B) (MEDIA_GLOBALS() ADD_MEDIA_COMMA SampleDescriptionHandle  sampleDescriptionH);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(QueueNonPrimarySourceData) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  inputIndex, long  dataDescriptionSeed, Handle  dataDescription, void * data, long  dataSize, ICMCompletionProcRecordPtr  asyncCompletionProc, const ICMFrameTimeRecord * frameTime, UniversalProcPtr  transferProc, void * refCon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(FlushNonPrimarySourceData) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  inputIndex);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetURLLink) (MEDIA_GLOBALS() ADD_MEDIA_COMMA Point  displayWhere, Handle * urlLink);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(MakeMediaTimeTable) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long ** offsets, TimeValue  startTime, TimeValue  endTime, TimeValue  timeIncrement, short  firstDataRefIndex, short  lastDataRefIndex, long * retDataRefSkew);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(HitTestForTargetRefCon) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  flags, Point  loc, long * targetRefCon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(HitTestTargetRefCon) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  targetRefCon, long  flags, Point  loc, Boolean * wasHit);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(GetActionsForQTEvent) (MEDIA_GLOBALS() ADD_MEDIA_COMMA QTEventRecordPtr  event, long  targetRefCon, QTAtomContainer * container, QTAtom * atom);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(DisposeTargetRefCon) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  targetRefCon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(TargetRefConsEqual) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long  firstRefCon, long  secondRefCon, Boolean * equal);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(SetActionsCallback) (MEDIA_GLOBALS() ADD_MEDIA_COMMA ActionsUPP  actionsCallbackProc, void * refcon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(PrePrerollBegin) (MEDIA_GLOBALS() ADD_MEDIA_COMMA TimeValue  time, Fixed  rate, PrePrerollCompleteUPP  completeProc, void * refcon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(PrePrerollCancel) (MEDIA_GLOBALS() ADD_MEDIA_COMMA void * refcon);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(ExecuteOneAction) (MEDIA_GLOBALS() ADD_MEDIA_COMMA QTAtomContainer  container, QTAtom  actionAtom);

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(EnterEmptyEdit) (MEDIA_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MEDIA_BASENAME(CurrentMediaQueuedData) (MEDIA_GLOBALS() ADD_MEDIA_COMMA long * milliSecs);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppMediaInitializeProcInfo = 0x000003F0,
		uppMediaSetHandlerCapabilitiesProcInfo = 0x00000FF0,
		uppMediaIdleProcInfo = 0x0000FFF0,
		uppMediaGetMediaInfoProcInfo = 0x000003F0,
		uppMediaPutMediaInfoProcInfo = 0x000003F0,
		uppMediaSetActiveProcInfo = 0x000001F0,
		uppMediaSetRateProcInfo = 0x000003F0,
		uppMediaGGetStatusProcInfo = 0x000003F0,
		uppMediaTrackEditedProcInfo = 0x000000F0,
		uppMediaSetMediaTimeScaleProcInfo = 0x000003F0,
		uppMediaSetMovieTimeScaleProcInfo = 0x000003F0,
		uppMediaSetGWorldProcInfo = 0x00000FF0,
		uppMediaSetDimensionsProcInfo = 0x00000FF0,
		uppMediaSetClipProcInfo = 0x000003F0,
		uppMediaSetMatrixProcInfo = 0x000003F0,
		uppMediaGetTrackOpaqueProcInfo = 0x000003F0,
		uppMediaSetGraphicsModeProcInfo = 0x00000FF0,
		uppMediaGetGraphicsModeProcInfo = 0x00000FF0,
		uppMediaGSetVolumeProcInfo = 0x000002F0,
		uppMediaSetSoundBalanceProcInfo = 0x000002F0,
		uppMediaGetSoundBalanceProcInfo = 0x000003F0,
		uppMediaGetNextBoundsChangeProcInfo = 0x000003F0,
		uppMediaGetSrcRgnProcInfo = 0x00000FF0,
		uppMediaPrerollProcInfo = 0x00000FF0,
		uppMediaSampleDescriptionChangedProcInfo = 0x000003F0,
		uppMediaHasCharacteristicProcInfo = 0x00000FF0,
		uppMediaGetOffscreenBufferSizeProcInfo = 0x00003BF0,
		uppMediaSetHintsProcInfo = 0x000003F0,
		uppMediaGetNameProcInfo = 0x00003FF0,
		uppMediaForceUpdateProcInfo = 0x000003F0,
		uppMediaGetDrawingRgnProcInfo = 0x000003F0,
		uppMediaGSetActiveSegmentProcInfo = 0x00000FF0,
		uppMediaInvalidateRegionProcInfo = 0x000003F0,
		uppMediaGetNextStepTimeProcInfo = 0x0000FEF0,
		uppMediaSetNonPrimarySourceDataProcInfo = 0x00FFFFF0,
		uppMediaChangedNonPrimarySourceProcInfo = 0x000003F0,
		uppMediaTrackReferencesChangedProcInfo = 0x000000F0,
		uppMediaGetSampleDataPointerProcInfo = 0x0000FFF0,
		uppMediaReleaseSampleDataPointerProcInfo = 0x000003F0,
		uppMediaTrackPropertyAtomChangedProcInfo = 0x000000F0,
		uppMediaSetTrackInputMapReferenceProcInfo = 0x000003F0,
		uppMediaSetVideoParamProcInfo = 0x00000FF0,
		uppMediaGetVideoParamProcInfo = 0x00000FF0,
		uppMediaCompareProcInfo = 0x00003FF0,
		uppMediaGetClockProcInfo = 0x000003F0,
		uppMediaSetSoundOutputComponentProcInfo = 0x000003F0,
		uppMediaGetSoundOutputComponentProcInfo = 0x000003F0,
		uppMediaSetSoundLocalizationDataProcInfo = 0x000003F0,
		uppMediaGetInvalidRegionProcInfo = 0x000003F0,
		uppMediaSampleDescriptionB2NProcInfo = 0x000003F0,
		uppMediaSampleDescriptionN2BProcInfo = 0x000003F0,
		uppMediaQueueNonPrimarySourceDataProcInfo = 0x03FFFFF0,
		uppMediaFlushNonPrimarySourceDataProcInfo = 0x000003F0,
		uppMediaGetURLLinkProcInfo = 0x00000FF0,
		uppMediaMakeMediaTimeTableProcInfo = 0x003AFFF0,
		uppMediaHitTestForTargetRefConProcInfo = 0x00003FF0,
		uppMediaHitTestTargetRefConProcInfo = 0x0000FFF0,
		uppMediaGetActionsForQTEventProcInfo = 0x0000FFF0,
		uppMediaDisposeTargetRefConProcInfo = 0x000003F0,
		uppMediaTargetRefConsEqualProcInfo = 0x00003FF0,
		uppMediaSetActionsCallbackProcInfo = 0x00000FF0,
		uppMediaPrePrerollBeginProcInfo = 0x0000FFF0,
		uppMediaPrePrerollCancelProcInfo = 0x000003F0,
		uppMediaExecuteOneActionProcInfo = 0x00000FF0,
		uppMediaEnterEmptyEditProcInfo = 0x000000F0,
		uppMediaCurrentMediaQueuedDataProcInfo = 0x000003F0
	};

#endif	/* MEDIA_BASENAME */

#ifdef MEDIA_BASENAME
enum {
	uppMediaSTSampleTableGetChunkNumProcInfo = SPECIAL_CASE_PROCINFO(specialMediaSTSampleTableGetChunkNum),
	uppMediaSTPrivateSampleTableGetSampleProcInfo = SPECIAL_CASE_PROCINFO(specialMediaSTPrivateSampleTableGetSample)
};
#endif	/* MEDIA_BASENAME */

#endif /* __MEDIAHANDLERS_K__ */

