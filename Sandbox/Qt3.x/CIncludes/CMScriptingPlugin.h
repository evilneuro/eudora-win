/*
 	File:		CMScriptingPlugin.h
 
 	Contains:	ColorSync Scripting Plugin API
 
 	Version:	Technology:	ColorSync 2.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __CMSCRIPTINGPLUGIN__
#define __CMSCRIPTINGPLUGIN__

#ifndef __FILES__
#include <Files.h>
#endif
#ifndef __CMAPPLICATION__
#include <CMApplication.h>
#endif
#ifndef __CODEFRAGMENTS__
#include <CodeFragments.h>
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
																/* ColorSync Scripting AppleEvent Errors */
	cmspInvalidImageFile		= -4220,						/* Plugin cannot handle this image file type */
	cmspInvalidImageSpace		= -4221,						/* Plugin cannot create an image file of this colorspace */
	cmspInvalidProfileEmbed		= -4222,						/* Specific invalid profile errors */
	cmspInvalidProfileSource	= -4223,
	cmspInvalidProfileDest		= -4224,
	cmspInvalidProfileProof		= -4225,
	cmspInvalidProfileLink		= -4226
};


/**** embedFlags field  ****/
/* reserved for future use: currently 0 */

/**** matchFlags field  ****/

enum {
	cmspFavorEmbeddedMask		= 0x00000001					/* if bit 0 is 0 then use srcProf profile, if 1 then use profile embedded in image if present*/
};


typedef CALLBACK_API_C( CMError , ValidateImageProcPtr )(const FSSpec *spec);
typedef CALLBACK_API_C( CMError , GetImageSpaceProcPtr )(const FSSpec *spec, OSType *space);
typedef CALLBACK_API_C( CMError , ValidateSpaceProcPtr )(const FSSpec *spec, OSType *space);
typedef CALLBACK_API_C( CMError , EmbedImageProcPtr )(const FSSpec *specFrom, const FSSpec *specInto, CMProfileRef embedProf, UInt32 embedFlags);
typedef CALLBACK_API_C( CMError , MatchImageProcPtr )(const FSSpec *specFrom, const FSSpec *specInto, UInt32 qual, UInt32 srcIntent, CMProfileRef srcProf, CMProfileRef dstProf, CMProfileRef prfProf, UInt32 matchFlags);
typedef STACK_UPP_TYPE(ValidateImageProcPtr) 					ValidateImageUPP;
typedef STACK_UPP_TYPE(GetImageSpaceProcPtr) 					GetImageSpaceUPP;
typedef STACK_UPP_TYPE(ValidateSpaceProcPtr) 					ValidateSpaceUPP;
typedef STACK_UPP_TYPE(EmbedImageProcPtr) 						EmbedImageUPP;
typedef STACK_UPP_TYPE(MatchImageProcPtr) 						MatchImageUPP;
enum { uppValidateImageProcInfo = 0x000000F1 }; 				/* 4_bytes Func(4_bytes) */
enum { uppGetImageSpaceProcInfo = 0x000003F1 }; 				/* 4_bytes Func(4_bytes, 4_bytes) */
enum { uppValidateSpaceProcInfo = 0x000003F1 }; 				/* 4_bytes Func(4_bytes, 4_bytes) */
enum { uppEmbedImageProcInfo = 0x00003FF1 }; 					/* 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes) */
enum { uppMatchImageProcInfo = 0x003FFFF1 }; 					/* 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewValidateImageProc(userRoutine) 						(ValidateImageUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppValidateImageProcInfo, GetCurrentArchitecture())
#define NewGetImageSpaceProc(userRoutine) 						(GetImageSpaceUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppGetImageSpaceProcInfo, GetCurrentArchitecture())
#define NewValidateSpaceProc(userRoutine) 						(ValidateSpaceUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppValidateSpaceProcInfo, GetCurrentArchitecture())
#define NewEmbedImageProc(userRoutine) 							(EmbedImageUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppEmbedImageProcInfo, GetCurrentArchitecture())
#define NewMatchImageProc(userRoutine) 							(MatchImageUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppMatchImageProcInfo, GetCurrentArchitecture())
#define CallValidateImageProc(userRoutine, spec) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppValidateImageProcInfo, (spec))
#define CallGetImageSpaceProc(userRoutine, spec, space) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppGetImageSpaceProcInfo, (spec), (space))
#define CallValidateSpaceProc(userRoutine, spec, space) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppValidateSpaceProcInfo, (spec), (space))
#define CallEmbedImageProc(userRoutine, specFrom, specInto, embedProf, embedFlags)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppEmbedImageProcInfo, (specFrom), (specInto), (embedProf), (embedFlags))
#define CallMatchImageProc(userRoutine, specFrom, specInto, qual, srcIntent, srcProf, dstProf, prfProf, matchFlags)  CALL_EIGHT_PARAMETER_UPP((userRoutine), uppMatchImageProcInfo, (specFrom), (specInto), (qual), (srcIntent), (srcProf), (dstProf), (prfProf), (matchFlags))

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

#endif /* __CMSCRIPTINGPLUGIN__ */

