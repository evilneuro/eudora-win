/*
 	File:		ImageCompression.k.h
 
 	Contains:	QuickTime interfaces
 
 	Version:	Technology:	
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __IMAGECOMPRESSION_K__
#define __IMAGECOMPRESSION_K__

#include <ImageCompression.h>

/*
	Example usage:

		#define GRAPHICSIMPORT_BASENAME()	Fred
		#define GRAPHICSIMPORT_GLOBALS()	FredGlobalsHandle
		#include <ImageCompression.k.h>

	To specify that your component implementation does not use globals, do not #define GRAPHICSIMPORT_GLOBALS
*/
#ifdef GRAPHICSIMPORT_BASENAME
	#ifndef GRAPHICSIMPORT_GLOBALS
		#define GRAPHICSIMPORT_GLOBALS() 
		#define ADD_GRAPHICSIMPORT_COMMA 
	#else
		#define ADD_GRAPHICSIMPORT_COMMA ,
	#endif
	#define GRAPHICSIMPORT_GLUE(a,b) a##b
	#define GRAPHICSIMPORT_STRCAT(a,b) GRAPHICSIMPORT_GLUE(a,b)
	#define ADD_GRAPHICSIMPORT_BASENAME(name) GRAPHICSIMPORT_STRCAT(GRAPHICSIMPORT_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetDataReference) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Handle  dataRef, OSType  dataReType);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetDataReference) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Handle * dataRef, OSType * dataReType);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetDataFile) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const FSSpec * theFile);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetDataFile) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA FSSpec * theFile);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetDataHandle) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Handle  h);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetDataHandle) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Handle * h);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetImageDescription) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA ImageDescriptionHandle * desc);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetDataOffsetAndSize) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA unsigned long * offset, unsigned long * size);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(ReadData) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA void * dataPtr, unsigned long  dataOffset, unsigned long  dataSize);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetClip) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA RgnHandle  clipRgn);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetClip) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA RgnHandle * clipRgn);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetSourceRect) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const Rect * sourceRect);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetSourceRect) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Rect * sourceRect);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetNaturalBounds) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Rect * naturalBounds);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(Draw) (GRAPHICSIMPORT_GLOBALS());

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetGWorld) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA CGrafPtr  port, GDHandle  gd);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetGWorld) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA CGrafPtr * port, GDHandle * gd);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetMatrix) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const MatrixRecord * matrix);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetMatrix) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA MatrixRecord * matrix);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetBoundsRect) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const Rect * bounds);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetBoundsRect) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Rect * bounds);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SaveAsPicture) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const FSSpec * fss, ScriptCode  scriptTag);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetGraphicsMode) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA long  graphicsMode, const RGBColor * opColor);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetGraphicsMode) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA long * graphicsMode, RGBColor * opColor);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetQuality) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA CodecQ  quality);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetQuality) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA CodecQ * quality);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SaveAsQuickTimeImageFile) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const FSSpec * fss, ScriptCode  scriptTag);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetDataReferenceOffsetAndLimit) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA unsigned long  offset, unsigned long  limit);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetDataReferenceOffsetAndLimit) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA unsigned long * offset, unsigned long * limit);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetAliasedDataReference) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Handle * dataRef, OSType * dataRefType);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(Validate) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA Boolean * valid);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetMetaData) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA void * userData);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetMIMETypeList) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA void * qtAtomContainerPtr);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(DoesDrawAllPixels) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA short * drawsAllPixels);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetAsPicture) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA PicHandle * picture);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(ExportImageFile) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA OSType  fileType, OSType  fileCreator, const FSSpec * fss, ScriptCode  scriptTag);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetExportImageTypeList) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA void * qtAtomContainerPtr);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(DoExportImageFileDialog) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA const FSSpec * inDefaultSpec, StringPtr  prompt, ModalFilterYDUPP  filterProc, OSType * outExportedType, FSSpec * outExportedSpec, ScriptCode * outScriptTag);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetExportSettingsAsAtomContainer) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA void * qtAtomContainerPtr);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetExportSettingsFromAtomContainer) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA void * qtAtomContainer);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(SetProgressProc) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA ICMProgressProcRecordPtr  progressProc);

	EXTERN_API( ComponentResult  ) ADD_GRAPHICSIMPORT_BASENAME(GetProgressProc) (GRAPHICSIMPORT_GLOBALS() ADD_GRAPHICSIMPORT_COMMA ICMProgressProcRecordPtr  progressProc);


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppGraphicsImportSetDataReferenceProcInfo = 0x00000FF0,
		uppGraphicsImportGetDataReferenceProcInfo = 0x00000FF0,
		uppGraphicsImportSetDataFileProcInfo = 0x000003F0,
		uppGraphicsImportGetDataFileProcInfo = 0x000003F0,
		uppGraphicsImportSetDataHandleProcInfo = 0x000003F0,
		uppGraphicsImportGetDataHandleProcInfo = 0x000003F0,
		uppGraphicsImportGetImageDescriptionProcInfo = 0x000003F0,
		uppGraphicsImportGetDataOffsetAndSizeProcInfo = 0x00000FF0,
		uppGraphicsImportReadDataProcInfo = 0x00003FF0,
		uppGraphicsImportSetClipProcInfo = 0x000003F0,
		uppGraphicsImportGetClipProcInfo = 0x000003F0,
		uppGraphicsImportSetSourceRectProcInfo = 0x000003F0,
		uppGraphicsImportGetSourceRectProcInfo = 0x000003F0,
		uppGraphicsImportGetNaturalBoundsProcInfo = 0x000003F0,
		uppGraphicsImportDrawProcInfo = 0x000000F0,
		uppGraphicsImportSetGWorldProcInfo = 0x00000FF0,
		uppGraphicsImportGetGWorldProcInfo = 0x00000FF0,
		uppGraphicsImportSetMatrixProcInfo = 0x000003F0,
		uppGraphicsImportGetMatrixProcInfo = 0x000003F0,
		uppGraphicsImportSetBoundsRectProcInfo = 0x000003F0,
		uppGraphicsImportGetBoundsRectProcInfo = 0x000003F0,
		uppGraphicsImportSaveAsPictureProcInfo = 0x00000BF0,
		uppGraphicsImportSetGraphicsModeProcInfo = 0x00000FF0,
		uppGraphicsImportGetGraphicsModeProcInfo = 0x00000FF0,
		uppGraphicsImportSetQualityProcInfo = 0x000003F0,
		uppGraphicsImportGetQualityProcInfo = 0x000003F0,
		uppGraphicsImportSaveAsQuickTimeImageFileProcInfo = 0x00000BF0,
		uppGraphicsImportSetDataReferenceOffsetAndLimitProcInfo = 0x00000FF0,
		uppGraphicsImportGetDataReferenceOffsetAndLimitProcInfo = 0x00000FF0,
		uppGraphicsImportGetAliasedDataReferenceProcInfo = 0x00000FF0,
		uppGraphicsImportValidateProcInfo = 0x000003F0,
		uppGraphicsImportGetMetaDataProcInfo = 0x000003F0,
		uppGraphicsImportGetMIMETypeListProcInfo = 0x000003F0,
		uppGraphicsImportDoesDrawAllPixelsProcInfo = 0x000003F0,
		uppGraphicsImportGetAsPictureProcInfo = 0x000003F0,
		uppGraphicsImportExportImageFileProcInfo = 0x0000BFF0,
		uppGraphicsImportGetExportImageTypeListProcInfo = 0x000003F0,
		uppGraphicsImportDoExportImageFileDialogProcInfo = 0x000FFFF0,
		uppGraphicsImportGetExportSettingsAsAtomContainerProcInfo = 0x000003F0,
		uppGraphicsImportSetExportSettingsFromAtomContainerProcInfo = 0x000003F0,
		uppGraphicsImportSetProgressProcProcInfo = 0x000003F0,
		uppGraphicsImportGetProgressProcProcInfo = 0x000003F0
	};

#endif	/* GRAPHICSIMPORT_BASENAME */

/*
	Example usage:

		#define IMAGETRANSCODER_BASENAME()	Fred
		#define IMAGETRANSCODER_GLOBALS()	FredGlobalsHandle
		#include <ImageCompression.k.h>

	To specify that your component implementation does not use globals, do not #define IMAGETRANSCODER_GLOBALS
*/
#ifdef IMAGETRANSCODER_BASENAME
	#ifndef IMAGETRANSCODER_GLOBALS
		#define IMAGETRANSCODER_GLOBALS() 
		#define ADD_IMAGETRANSCODER_COMMA 
	#else
		#define ADD_IMAGETRANSCODER_COMMA ,
	#endif
	#define IMAGETRANSCODER_GLUE(a,b) a##b
	#define IMAGETRANSCODER_STRCAT(a,b) IMAGETRANSCODER_GLUE(a,b)
	#define ADD_IMAGETRANSCODER_BASENAME(name) IMAGETRANSCODER_STRCAT(IMAGETRANSCODER_BASENAME(),name)

	EXTERN_API( ComponentResult  ) ADD_IMAGETRANSCODER_BASENAME(BeginSequence) (IMAGETRANSCODER_GLOBALS() ADD_IMAGETRANSCODER_COMMA ImageDescriptionHandle  srcDesc, ImageDescriptionHandle * dstDesc, void * data, long  dataSize);

	EXTERN_API( ComponentResult  ) ADD_IMAGETRANSCODER_BASENAME(Convert) (IMAGETRANSCODER_GLOBALS() ADD_IMAGETRANSCODER_COMMA void * srcData, long  srcDataSize, void ** dstData, long * dstDataSize);

	EXTERN_API( ComponentResult  ) ADD_IMAGETRANSCODER_BASENAME(DisposeData) (IMAGETRANSCODER_GLOBALS() ADD_IMAGETRANSCODER_COMMA void * dstData);

	EXTERN_API( ComponentResult  ) ADD_IMAGETRANSCODER_BASENAME(EndSequence) (IMAGETRANSCODER_GLOBALS());


	/* MixedMode ProcInfo constants for component calls */
	enum {
		uppImageTranscoderBeginSequenceProcInfo = 0x0000FFF0,
		uppImageTranscoderConvertProcInfo = 0x0000FFF0,
		uppImageTranscoderDisposeDataProcInfo = 0x000003F0,
		uppImageTranscoderEndSequenceProcInfo = 0x000000F0
	};

#endif	/* IMAGETRANSCODER_BASENAME */


#endif /* __IMAGECOMPRESSION_K__ */

