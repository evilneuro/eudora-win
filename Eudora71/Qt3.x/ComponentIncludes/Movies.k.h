/*
 	File:		Movies.k.h
 
 	Contains:	QuickTime interfaces
 
 	Version:	Technology:	
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __MOVIES_K__
#define __MOVIES_K__

#include <Movies.h>

/*
	Example usage:

		#define VIDEOMEDIA_BASENAME()	Fred
		#define VIDEOMEDIA_GLOBALS()	FredGlobalsHandle
		#include <Movies.k.h>

	To specify that your component implementation does not use globals, do not #define VIDEOMEDIA_GLOBALS
*/
#ifdef VIDEOMEDIA_BASENAME
	#ifndef VIDEOMEDIA_GLOBALS
		#define VIDEOMEDIA_GLOBALS() 
		#define ADD_VIDEOMEDIA_COMMA 
	#else
		#define ADD_VIDEOMEDIA_COMMA ,
	#endif
	#define VIDEOMEDIA_GLUE(a,b) a##b
	#define VIDEOMEDIA_STRCAT(a,b) VIDEOMEDIA_GLUE(a,b)
	#define ADD_VIDEOMEDIA_BASENAME(name) VIDEOMEDIA_STRCAT(VIDEOMEDIA_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_VIDEOMEDIA_BASENAME(ResetStatistics) (VIDEOMEDIA_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_VIDEOMEDIA_BASENAME(GetStatistics) (VIDEOMEDIA_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_VIDEOMEDIA_BASENAME(GetStallCount) (VIDEOMEDIA_GLOBALS() ADD_VIDEOMEDIA_COMMA unsigned long * stalls);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppVideoMediaResetStatisticsProcInfo = 0x000000F0,
		uppVideoMediaGetStatisticsProcInfo = 0x000000F0,
		uppVideoMediaGetStallCountProcInfo = 0x000003F0
	};

#endif	/* VIDEOMEDIA_BASENAME */

/*
	Example usage:

		#define TEXTMEDIA_BASENAME()	Fred
		#define TEXTMEDIA_GLOBALS()	FredGlobalsHandle
		#include <Movies.k.h>

	To specify that your component implementation does not use globals, do not #define TEXTMEDIA_GLOBALS
*/
#ifdef TEXTMEDIA_BASENAME
	#ifndef TEXTMEDIA_GLOBALS
		#define TEXTMEDIA_GLOBALS() 
		#define ADD_TEXTMEDIA_COMMA 
	#else
		#define ADD_TEXTMEDIA_COMMA ,
	#endif
	#define TEXTMEDIA_GLUE(a,b) a##b
	#define TEXTMEDIA_STRCAT(a,b) TEXTMEDIA_GLUE(a,b)
	#define ADD_TEXTMEDIA_BASENAME(name) TEXTMEDIA_STRCAT(TEXTMEDIA_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(SetTextProc) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA TextMediaUPP  TextProc, long  refcon);

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(AddTextSample) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA Ptr  text, unsigned long  size, short  fontNumber, short  fontSize, Style  textFace, RGBColor * textColor, RGBColor * backColor, short  textJustification, Rect * textBox, long  displayFlags, TimeValue  scrollDelay, short  hiliteStart, short  hiliteEnd, RGBColor * rgbHiliteColor, TimeValue  duration, TimeValue * sampleTime);

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(AddTESample) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA TEHandle  hTE, RGBColor * backColor, short  textJustification, Rect * textBox, long  displayFlags, TimeValue  scrollDelay, short  hiliteStart, short  hiliteEnd, RGBColor * rgbHiliteColor, TimeValue  duration, TimeValue * sampleTime);

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(AddHiliteSample) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA short  hiliteStart, short  hiliteEnd, RGBColor * rgbHiliteColor, TimeValue  duration, TimeValue * sampleTime);

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(FindNextText) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA Ptr  text, long  size, short  findFlags, TimeValue  startTime, TimeValue * foundTime, TimeValue * foundDuration, long * offset);

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(HiliteTextSample) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA TimeValue  sampleTime, short  hiliteStart, short  hiliteEnd, RGBColor * rgbHiliteColor);

	EXTERN_API( ComponentResult  ) ADD_TEXTMEDIA_BASENAME(SetTextSampleData) (TEXTMEDIA_GLOBALS() ADD_TEXTMEDIA_COMMA void * data, OSType  dataType);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppTextMediaSetTextProcProcInfo = 0x00000FF0,
		uppTextMediaAddTESampleProcInfo = 0x3FAFEFF0,
		uppTextMediaAddHiliteSampleProcInfo = 0x0003FAF0,
		uppTextMediaFindNextTextProcInfo = 0x003FEFF0,
		uppTextMediaHiliteTextSampleProcInfo = 0x0000EBF0,
		uppTextMediaSetTextSampleDataProcInfo = 0x00000FF0
	};

#endif	/* TEXTMEDIA_BASENAME */

#ifdef TEXTMEDIA_BASENAME
enum {
	uppTextMediaAddTextSampleProcInfo = SPECIAL_CASE_PROCINFO(specialTextMediaAddTextSample)
};
#endif	/* TEXTMEDIA_BASENAME */
/*
	Example usage:

		#define SPRITEMEDIA_BASENAME()	Fred
		#define SPRITEMEDIA_GLOBALS()	FredGlobalsHandle
		#include <Movies.k.h>

	To specify that your component implementation does not use globals, do not #define SPRITEMEDIA_GLOBALS
*/
#ifdef SPRITEMEDIA_BASENAME
	#ifndef SPRITEMEDIA_GLOBALS
		#define SPRITEMEDIA_GLOBALS() 
		#define ADD_SPRITEMEDIA_COMMA 
	#else
		#define ADD_SPRITEMEDIA_COMMA ,
	#endif
	#define SPRITEMEDIA_GLUE(a,b) a##b
	#define SPRITEMEDIA_STRCAT(a,b) SPRITEMEDIA_GLUE(a,b)
	#define ADD_SPRITEMEDIA_BASENAME(name) SPRITEMEDIA_STRCAT(SPRITEMEDIA_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(SetProperty) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short  spriteIndex, long  propertyType, void * propertyValue);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetProperty) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short  spriteIndex, long  propertyType, void * propertyValue);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(HitTestSprites) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA long  flags, Point  loc, short * spriteHitIndex);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(CountSprites) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short * numSprites);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(CountImages) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short * numImages);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetIndImageDescription) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short  imageIndex, ImageDescriptionHandle  imageDescription);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetDisplayedSampleNumber) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA long * sampleNum);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetSpriteName) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  spriteID, Str255  spriteName);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetImageName) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short  imageIndex, Str255  imageName);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(SetSpriteProperty) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  spriteID, long  propertyType, void * propertyValue);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetSpriteProperty) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  spriteID, long  propertyType, void * propertyValue);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(HitTestAllSprites) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA long  flags, Point  loc, QTAtomID * spriteHitID);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(HitTestOneSprite) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  spriteID, long  flags, Point  loc, Boolean * wasHit);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(SpriteIndexToID) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short  spriteIndex, QTAtomID * spriteID);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(SpriteIDToIndex) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  spriteID, short * spriteIndex);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetSpriteActionsForQTEvent) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTEventRecordPtr  event, QTAtomID  spriteID, QTAtomContainer * container, QTAtom * atom);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(SetActionVariable) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  variableID, const float * value);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetActionVariable) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA QTAtomID  variableID, float * value);

	EXTERN_API( ComponentResult  ) ADD_SPRITEMEDIA_BASENAME(GetIndImageProperty) (SPRITEMEDIA_GLOBALS() ADD_SPRITEMEDIA_COMMA short  imageIndex, long  imagePropertyType, void * imagePropertyValue);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppSpriteMediaSetPropertyProcInfo = 0x00003EF0,
		uppSpriteMediaGetPropertyProcInfo = 0x00003EF0,
		uppSpriteMediaHitTestSpritesProcInfo = 0x00003FF0,
		uppSpriteMediaCountSpritesProcInfo = 0x000003F0,
		uppSpriteMediaCountImagesProcInfo = 0x000003F0,
		uppSpriteMediaGetIndImageDescriptionProcInfo = 0x00000EF0,
		uppSpriteMediaGetDisplayedSampleNumberProcInfo = 0x000003F0,
		uppSpriteMediaGetSpriteNameProcInfo = 0x00000FF0,
		uppSpriteMediaGetImageNameProcInfo = 0x00000EF0,
		uppSpriteMediaSetSpritePropertyProcInfo = 0x00003FF0,
		uppSpriteMediaGetSpritePropertyProcInfo = 0x00003FF0,
		uppSpriteMediaHitTestAllSpritesProcInfo = 0x00003FF0,
		uppSpriteMediaHitTestOneSpriteProcInfo = 0x0000FFF0,
		uppSpriteMediaSpriteIndexToIDProcInfo = 0x00000EF0,
		uppSpriteMediaSpriteIDToIndexProcInfo = 0x00000FF0,
		uppSpriteMediaGetSpriteActionsForQTEventProcInfo = 0x0000FFF0,
		uppSpriteMediaSetActionVariableProcInfo = 0x00000FF0,
		uppSpriteMediaGetActionVariableProcInfo = 0x00000FF0,
		uppSpriteMediaGetIndImagePropertyProcInfo = 0x00003EF0
	};

#endif	/* SPRITEMEDIA_BASENAME */

/*
	Example usage:

		#define MEDIA3D_BASENAME()	Fred
		#define MEDIA3D_GLOBALS()	FredGlobalsHandle
		#include <Movies.k.h>

	To specify that your component implementation does not use globals, do not #define MEDIA3D_GLOBALS
*/
#ifdef MEDIA3D_BASENAME
	#ifndef MEDIA3D_GLOBALS
		#define MEDIA3D_GLOBALS() 
		#define ADD_MEDIA3D_COMMA 
	#else
		#define ADD_MEDIA3D_COMMA ,
	#endif
	#define MEDIA3D_GLUE(a,b) a##b
	#define MEDIA3D_STRCAT(a,b) MEDIA3D_GLUE(a,b)
	#define ADD_MEDIA3D_BASENAME(name) MEDIA3D_STRCAT(MEDIA3D_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_MEDIA3D_BASENAME(GetNamedObjectList) (MEDIA3D_GLOBALS() ADD_MEDIA3D_COMMA QTAtomContainer * objectList);

	EXTERN_API( ComponentResult  ) ADD_MEDIA3D_BASENAME(GetRendererList) (MEDIA3D_GLOBALS() ADD_MEDIA3D_COMMA QTAtomContainer * rendererList);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppMedia3DGetNamedObjectListProcInfo = 0x000003F0,
		uppMedia3DGetRendererListProcInfo = 0x000003F0
	};

#endif	/* MEDIA3D_BASENAME */

/*
	Example usage:

		#define MC_BASENAME()	Fred
		#define MC_GLOBALS()	FredGlobalsHandle
		#include <Movies.k.h>

	To specify that your component implementation does not use globals, do not #define MC_GLOBALS
*/
#ifdef MC_BASENAME
	#ifndef MC_GLOBALS
		#define MC_GLOBALS() 
		#define ADD_MC_COMMA 
	#else
		#define ADD_MC_COMMA ,
	#endif
	#define MC_GLUE(a,b) a##b
	#define MC_STRCAT(a,b) MC_GLUE(a,b)
	#define ADD_MC_BASENAME(name) MC_STRCAT(MC_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetMovie) (MC_GLOBALS() ADD_MC_COMMA Movie  theMovie, WindowPtr  movieWindow, Point  where);

	EXTERN_API( Movie  ) ADD_MC_BASENAME(GetIndMovie) (MC_GLOBALS() ADD_MC_COMMA short  index);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(RemoveAllMovies) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(RemoveAMovie) (MC_GLOBALS() ADD_MC_COMMA Movie  m);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(RemoveMovie) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(IsPlayerEvent) (MC_GLOBALS() ADD_MC_COMMA const EventRecord * e);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetActionFilter) (MC_GLOBALS() ADD_MC_COMMA MCActionFilterUPP  blob);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(DoAction) (MC_GLOBALS() ADD_MC_COMMA short  action, void * params);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetControllerAttached) (MC_GLOBALS() ADD_MC_COMMA Boolean  attach);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(IsControllerAttached) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetControllerPort) (MC_GLOBALS() ADD_MC_COMMA CGrafPtr  gp);

	EXTERN_API( CGrafPtr  ) ADD_MC_BASENAME(GetControllerPort) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetVisible) (MC_GLOBALS() ADD_MC_COMMA Boolean  visible);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(GetVisible) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(GetControllerBoundsRect) (MC_GLOBALS() ADD_MC_COMMA Rect * bounds);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetControllerBoundsRect) (MC_GLOBALS() ADD_MC_COMMA const Rect * bounds);

	EXTERN_API( RgnHandle  ) ADD_MC_BASENAME(GetControllerBoundsRgn) (MC_GLOBALS());

	EXTERN_API( RgnHandle  ) ADD_MC_BASENAME(GetWindowRgn) (MC_GLOBALS() ADD_MC_COMMA WindowPtr  w);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(MovieChanged) (MC_GLOBALS() ADD_MC_COMMA Movie  m);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetDuration) (MC_GLOBALS() ADD_MC_COMMA TimeValue  duration);

	EXTERN_API( TimeValue  ) ADD_MC_BASENAME(GetCurrentTime) (MC_GLOBALS() ADD_MC_COMMA TimeScale * scale);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(NewAttachedController) (MC_GLOBALS() ADD_MC_COMMA Movie  theMovie, WindowPtr  w, Point  where);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Draw) (MC_GLOBALS() ADD_MC_COMMA WindowPtr  w);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Activate) (MC_GLOBALS() ADD_MC_COMMA WindowPtr  w, Boolean  activate);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Idle) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Key) (MC_GLOBALS() ADD_MC_COMMA SInt8  key, long  modifiers);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Click) (MC_GLOBALS() ADD_MC_COMMA WindowPtr  w, Point  where, long  when, long  modifiers);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(EnableEditing) (MC_GLOBALS() ADD_MC_COMMA Boolean  enabled);

	EXTERN_API( long  ) ADD_MC_BASENAME(IsEditingEnabled) (MC_GLOBALS());

	EXTERN_API( Movie  ) ADD_MC_BASENAME(Copy) (MC_GLOBALS());

	EXTERN_API( Movie  ) ADD_MC_BASENAME(Cut) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Paste) (MC_GLOBALS() ADD_MC_COMMA Movie  srcMovie);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Clear) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Undo) (MC_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(PositionController) (MC_GLOBALS() ADD_MC_COMMA const Rect * movieRect, const Rect * controllerRect, long  someFlags);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(GetControllerInfo) (MC_GLOBALS() ADD_MC_COMMA long * someFlags);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetClip) (MC_GLOBALS() ADD_MC_COMMA RgnHandle  theClip, RgnHandle  movieClip);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(GetClip) (MC_GLOBALS() ADD_MC_COMMA RgnHandle * theClip, RgnHandle * movieClip);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(DrawBadge) (MC_GLOBALS() ADD_MC_COMMA RgnHandle  movieRgn, RgnHandle * badgeRgn);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetUpEditMenu) (MC_GLOBALS() ADD_MC_COMMA long  modifiers, MenuHandle  mh);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(GetMenuString) (MC_GLOBALS() ADD_MC_COMMA long  modifiers, short  item, Str255  aString);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(SetActionFilterWithRefCon) (MC_GLOBALS() ADD_MC_COMMA MCActionFilterWithRefConUPP  blob, long  refCon);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(PtInController) (MC_GLOBALS() ADD_MC_COMMA Point  thePt, Boolean * inController);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(Invalidate) (MC_GLOBALS() ADD_MC_COMMA WindowPtr  w, RgnHandle  invalidRgn);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(AdjustCursor) (MC_GLOBALS() ADD_MC_COMMA WindowPtr  w, Point  where, long  modifiers);

	EXTERN_API( ComponentResult  ) ADD_MC_BASENAME(GetInterfaceElement) (MC_GLOBALS() ADD_MC_COMMA MCInterfaceElement  whichElement, void * element);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppMCSetMovieProcInfo = 0x00003FF0,
		uppMCGetIndMovieProcInfo = 0x000002F0,
		uppMCRemoveAllMoviesProcInfo = 0x000000F0,
		uppMCRemoveAMovieProcInfo = 0x000003F0,
		uppMCRemoveMovieProcInfo = 0x000000F0,
		uppMCIsPlayerEventProcInfo = 0x000003F0,
		uppMCSetActionFilterProcInfo = 0x000003F0,
		uppMCDoActionProcInfo = 0x00000EF0,
		uppMCSetControllerAttachedProcInfo = 0x000001F0,
		uppMCIsControllerAttachedProcInfo = 0x000000F0,
		uppMCSetControllerPortProcInfo = 0x000003F0,
		uppMCGetControllerPortProcInfo = 0x000000F0,
		uppMCSetVisibleProcInfo = 0x000001F0,
		uppMCGetVisibleProcInfo = 0x000000F0,
		uppMCGetControllerBoundsRectProcInfo = 0x000003F0,
		uppMCSetControllerBoundsRectProcInfo = 0x000003F0,
		uppMCGetControllerBoundsRgnProcInfo = 0x000000F0,
		uppMCGetWindowRgnProcInfo = 0x000003F0,
		uppMCMovieChangedProcInfo = 0x000003F0,
		uppMCSetDurationProcInfo = 0x000003F0,
		uppMCGetCurrentTimeProcInfo = 0x000003F0,
		uppMCNewAttachedControllerProcInfo = 0x00003FF0,
		uppMCDrawProcInfo = 0x000003F0,
		uppMCActivateProcInfo = 0x000007F0,
		uppMCIdleProcInfo = 0x000000F0,
		uppMCKeyProcInfo = 0x00000DF0,
		uppMCClickProcInfo = 0x0000FFF0,
		uppMCEnableEditingProcInfo = 0x000001F0,
		uppMCIsEditingEnabledProcInfo = 0x000000F0,
		uppMCCopyProcInfo = 0x000000F0,
		uppMCCutProcInfo = 0x000000F0,
		uppMCPasteProcInfo = 0x000003F0,
		uppMCClearProcInfo = 0x000000F0,
		uppMCUndoProcInfo = 0x000000F0,
		uppMCPositionControllerProcInfo = 0x00003FF0,
		uppMCGetControllerInfoProcInfo = 0x000003F0,
		uppMCSetClipProcInfo = 0x00000FF0,
		uppMCGetClipProcInfo = 0x00000FF0,
		uppMCDrawBadgeProcInfo = 0x00000FF0,
		uppMCSetUpEditMenuProcInfo = 0x00000FF0,
		uppMCGetMenuStringProcInfo = 0x00003BF0,
		uppMCSetActionFilterWithRefConProcInfo = 0x00000FF0,
		uppMCPtInControllerProcInfo = 0x00000FF0,
		uppMCInvalidateProcInfo = 0x00000FF0,
		uppMCAdjustCursorProcInfo = 0x00003FF0,
		uppMCGetInterfaceElementProcInfo = 0x00000FF0
	};

#endif	/* MC_BASENAME */

/*
	Example usage:

		#define MUSICMEDIA_BASENAME()	Fred
		#define MUSICMEDIA_GLOBALS()	FredGlobalsHandle
		#include <Movies.k.h>

	To specify that your component implementation does not use globals, do not #define MUSICMEDIA_GLOBALS
*/
#ifdef MUSICMEDIA_BASENAME
	#ifndef MUSICMEDIA_GLOBALS
		#define MUSICMEDIA_GLOBALS() 
		#define ADD_MUSICMEDIA_COMMA 
	#else
		#define ADD_MUSICMEDIA_COMMA ,
	#endif
	#define MUSICMEDIA_GLUE(a,b) a##b
	#define MUSICMEDIA_STRCAT(a,b) MUSICMEDIA_GLUE(a,b)
	#define ADD_MUSICMEDIA_BASENAME(name) MUSICMEDIA_STRCAT(MUSICMEDIA_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_MUSICMEDIA_BASENAME(GetIndexedTunePlayer) (MUSICMEDIA_GLOBALS() ADD_MUSICMEDIA_COMMA long  sampleDescIndex, ComponentInstance * tp);

#endif	/* MUSICMEDIA_BASENAME */


#endif /* __MOVIES_K__ */

