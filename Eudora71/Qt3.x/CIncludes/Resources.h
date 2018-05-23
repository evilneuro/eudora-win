/*
 	File:		Resources.h
 
 	Contains:	Resource Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __RESOURCES__
#define __RESOURCES__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif
#ifndef __FILES__
#include <Files.h>
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
	resSysHeap					= 64,							/*System or application heap?*/
	resPurgeable				= 32,							/*Purgeable resource?*/
	resLocked					= 16,							/*Load it in locked?*/
	resProtected				= 8,							/*Protected?*/
	resPreload					= 4,							/*Load in on OpenResFile?*/
	resChanged					= 2,							/*Resource changed?*/
	mapReadOnly					= 128,							/*Resource file read-only*/
	mapCompact					= 64,							/*Compact resource file*/
	mapChanged					= 32							/*Write map out at update*/
};


enum {
	resSysRefBit				= 7,							/*reference to system/local reference*/
	resSysHeapBit				= 6,							/*In system/in application heap*/
	resPurgeableBit				= 5,							/*Purgeable/not purgeable*/
	resLockedBit				= 4,							/*Locked/not locked*/
	resProtectedBit				= 3,							/*Protected/not protected*/
	resPreloadBit				= 2,							/*Read in at OpenResource?*/
	resChangedBit				= 1,							/*Existing resource changed since last update*/
	mapReadOnlyBit				= 7,							/*is this file read-only?*/
	mapCompactBit				= 6,							/*Is a compact necessary?*/
	mapChangedBit				= 5								/*Is it necessary to write map?*/
};


enum {
	kResFileNotOpened			= -1,							/*ref num return as error when opening a resource file*/
	kSystemResFile				= 0								/*this is the default ref num to the system file*/
};


typedef CALLBACK_API( void , ResErrProcPtr )(OSErr thErr);
/*
	WARNING: ResErrProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef REGISTER_UPP_TYPE(ResErrProcPtr) 						ResErrUPP;
enum { uppResErrProcInfo = 0x00001002 }; 						/* register no_return_value Func(2_bytes:D0) */
#define NewResErrProc(userRoutine) 								(ResErrUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppResErrProcInfo, GetCurrentArchitecture())
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter CallResErrProc(__A0, __D0)
	void CallResErrProc(ResErrUPP routine, OSErr thErr) = 0x4E90;
#else
	#define CallResErrProc(userRoutine, thErr) 					CALL_ONE_PARAMETER_UPP((userRoutine), uppResErrProcInfo, (thErr))
#endif
#if !TARGET_OS_MAC
/* QuickTime 3.0*/
typedef CALLBACK_API( OSErr , ResourceEndianFilterPtr )(Handle theResource, Boolean currentlyNativeEndian);
#endif  /*  !TARGET_OS_MAC */

EXTERN_API( short )
InitResources					(void)														ONEWORDINLINE(0xA995);

EXTERN_API( void )
RsrcZoneInit					(void)														ONEWORDINLINE(0xA996);

EXTERN_API( void )
CloseResFile					(short 					refNum)								ONEWORDINLINE(0xA99A);

EXTERN_API( OSErr )
ResError						(void)														ONEWORDINLINE(0xA9AF);

EXTERN_API( short )
CurResFile						(void)														ONEWORDINLINE(0xA994);

EXTERN_API( short )
HomeResFile						(Handle 				theResource)						ONEWORDINLINE(0xA9A4);

EXTERN_API( void )
CreateResFile					(ConstStr255Param 		fileName)							ONEWORDINLINE(0xA9B1);

EXTERN_API( short )
OpenResFile						(ConstStr255Param 		fileName)							ONEWORDINLINE(0xA997);

EXTERN_API( void )
UseResFile						(short 					refNum)								ONEWORDINLINE(0xA998);

EXTERN_API( short )
CountTypes						(void)														ONEWORDINLINE(0xA99E);

EXTERN_API( short )
Count1Types						(void)														ONEWORDINLINE(0xA81C);

EXTERN_API( void )
GetIndType						(ResType *				theType,
								 short 					index)								ONEWORDINLINE(0xA99F);

EXTERN_API( void )
Get1IndType						(ResType *				theType,
								 short 					index)								ONEWORDINLINE(0xA80F);

EXTERN_API( void )
SetResLoad						(Boolean 				load)								ONEWORDINLINE(0xA99B);

EXTERN_API( short )
CountResources					(ResType 				theType)							ONEWORDINLINE(0xA99C);

EXTERN_API( short )
Count1Resources					(ResType 				theType)							ONEWORDINLINE(0xA80D);

EXTERN_API( Handle )
GetIndResource					(ResType 				theType,
								 short 					index)								ONEWORDINLINE(0xA99D);

EXTERN_API( Handle )
Get1IndResource					(ResType 				theType,
								 short 					index)								ONEWORDINLINE(0xA80E);

EXTERN_API( Handle )
GetResource						(ResType 				theType,
								 short 					theID)								ONEWORDINLINE(0xA9A0);

EXTERN_API( Handle )
Get1Resource					(ResType 				theType,
								 short 					theID)								ONEWORDINLINE(0xA81F);

EXTERN_API( Handle )
GetNamedResource				(ResType 				theType,
								 ConstStr255Param 		name)								ONEWORDINLINE(0xA9A1);

EXTERN_API( Handle )
Get1NamedResource				(ResType 				theType,
								 ConstStr255Param 		name)								ONEWORDINLINE(0xA820);

#if TARGET_OS_MAC
	#define MacLoadResource LoadResource
#endif
EXTERN_API( void )
MacLoadResource					(Handle 				theResource)						ONEWORDINLINE(0xA9A2);

EXTERN_API( void )
ReleaseResource					(Handle 				theResource)						ONEWORDINLINE(0xA9A3);

EXTERN_API( void )
DetachResource					(Handle 				theResource)						ONEWORDINLINE(0xA992);

EXTERN_API( short )
UniqueID						(ResType 				theType)							ONEWORDINLINE(0xA9C1);

EXTERN_API( short )
Unique1ID						(ResType 				theType)							ONEWORDINLINE(0xA810);

EXTERN_API( short )
GetResAttrs						(Handle 				theResource)						ONEWORDINLINE(0xA9A6);

EXTERN_API( void )
GetResInfo						(Handle 				theResource,
								 short *				theID,
								 ResType *				theType,
								 Str255 				name)								ONEWORDINLINE(0xA9A8);

EXTERN_API( void )
SetResInfo						(Handle 				theResource,
								 short 					theID,
								 ConstStr255Param 		name)								ONEWORDINLINE(0xA9A9);

EXTERN_API( void )
AddResource						(Handle 				theData,
								 ResType 				theType,
								 short 					theID,
								 ConstStr255Param 		name)								ONEWORDINLINE(0xA9AB);

EXTERN_API( long )
GetResourceSizeOnDisk			(Handle 				theResource)						ONEWORDINLINE(0xA9A5);

EXTERN_API( long )
GetMaxResourceSize				(Handle 				theResource)						ONEWORDINLINE(0xA821);

EXTERN_API( long )
RsrcMapEntry					(Handle 				theResource)						ONEWORDINLINE(0xA9C5);

EXTERN_API( void )
SetResAttrs						(Handle 				theResource,
								 short 					attrs)								ONEWORDINLINE(0xA9A7);

EXTERN_API( void )
ChangedResource					(Handle 				theResource)						ONEWORDINLINE(0xA9AA);

EXTERN_API( void )
RemoveResource					(Handle 				theResource)						ONEWORDINLINE(0xA9AD);

EXTERN_API( void )
UpdateResFile					(short 					refNum)								ONEWORDINLINE(0xA999);

EXTERN_API( void )
WriteResource					(Handle 				theResource)						ONEWORDINLINE(0xA9B0);

EXTERN_API( void )
SetResPurge						(Boolean 				install)							ONEWORDINLINE(0xA993);

EXTERN_API( short )
GetResFileAttrs					(short 					refNum)								ONEWORDINLINE(0xA9F6);

EXTERN_API( void )
SetResFileAttrs					(short 					refNum,
								 short 					attrs)								ONEWORDINLINE(0xA9F7);

EXTERN_API( short )
OpenRFPerm						(ConstStr255Param 		fileName,
								 short 					vRefNum,
								 SInt8 					permission)							ONEWORDINLINE(0xA9C4);

EXTERN_API( Handle )
RGetResource					(ResType 				theType,
								 short 					theID)								ONEWORDINLINE(0xA80C);

/*
	Note: 	The HOpenResFile trap was not implemented until System 7.  If you want to call HOpenResFile
			while running on System 6 machines, then define USE_HOPENRESFILE_GLUE and link with
			Interface.o which contains glue to implement HOpenResFile on pre-System 7 machines.
*/
#ifdef USE_HOPENRESFILE_GLUE
EXTERN_API( short )
HOpenResFile					(short 					vRefNum,
								 long 					dirID,
								 ConstStr255Param 		fileName,
								 SInt8 					permission);

#else
EXTERN_API( short )
HOpenResFile					(short 					vRefNum,
								 long 					dirID,
								 ConstStr255Param 		fileName,
								 SInt8 					permission)							ONEWORDINLINE(0xA81A);

#endif  /*  defined(USE_HOPENRESFILE_GLUE)  */

/*
	Note: 	The HCreateResFile trap was not implemented until System 7.  If you want to call HCreateResFile
			while running on System 6 machines, then define USE_HCREATERESFILE_GLUE and link with
			Interface.o which contains glue to implement HCreateResFile on pre-System 7 machines.
*/
#ifdef USE_HCREATERESFILE_GLUE
EXTERN_API( void )
HCreateResFile					(short 					vRefNum,
								 long 					dirID,
								 ConstStr255Param 		fileName);

#else
EXTERN_API( void )
HCreateResFile					(short 					vRefNum,
								 long 					dirID,
								 ConstStr255Param 		fileName)							ONEWORDINLINE(0xA81B);

#endif  /*  defined(USE_HCREATERESFILE_GLUE)  */

EXTERN_API( short )
FSpOpenResFile					(const FSSpec *			spec,
								 SignedByte 			permission)							TWOWORDINLINE(0x700D, 0xAA52);

EXTERN_API( void )
FSpCreateResFile				(const FSSpec *			spec,
								 OSType 				creator,
								 OSType 				fileType,
								 ScriptCode 			scriptTag)							TWOWORDINLINE(0x700E, 0xAA52);

EXTERN_API( void )
ReadPartialResource				(Handle 				theResource,
								 long 					offset,
								 void *					buffer,
								 long 					count)								TWOWORDINLINE(0x7001, 0xA822);

EXTERN_API( void )
WritePartialResource			(Handle 				theResource,
								 long 					offset,
								 const void *			buffer,
								 long 					count)								TWOWORDINLINE(0x7002, 0xA822);

EXTERN_API( void )
SetResourceSize					(Handle 				theResource,
								 long 					newSize)							TWOWORDINLINE(0x7003, 0xA822);

EXTERN_API( Handle )
GetNextFOND						(Handle 				fondHandle)							TWOWORDINLINE(0x700A, 0xA822);


#if !TARGET_OS_MAC
/* QuickTime 3.0*/
EXTERN_API_C( OSErr )
RegisterResourceEndianFilter	(ResType 				theType,
								 ResourceEndianFilterPtr  theFilterProc);

#endif  /*  !TARGET_OS_MAC */

/* Use TempInsertROMMap to force the ROM resource map to be
   inserted into the chain in front of the system. Note that
   this call is only temporary - the modified resource chain
   is only used for the next call to the resource manager.
   See IM IV 19 for more information. 
*/
EXTERN_API( void )
TempInsertROMMap				(Boolean 				tempResLoad)						FIVEWORDINLINE(0x70FF, 0x4A1F, 0x56C0, 0x31C0, 0x0B9E);


#if CGLUESUPPORTED
EXTERN_API_C( Handle )
getnamedresource				(ResType 				theType,
								 const char *			name);

EXTERN_API_C( Handle )
get1namedresource				(ResType 				theType,
								 const char *			name);

EXTERN_API_C( short )
openrfperm						(const char *			fileName,
								 short 					vRefNum,
								 char 					permission);

EXTERN_API_C( short )
openresfile						(const char *			fileName);

EXTERN_API_C( void )
createresfile					(const char *			fileName);

EXTERN_API_C( void )
getresinfo						(Handle 				theResource,
								 short *				theID,
								 ResType *				theType,
								 char *					name);

EXTERN_API_C( void )
setresinfo						(Handle 				theResource,
								 short 					theID,
								 const char *			name);

EXTERN_API_C( void )
addresource						(Handle 				theResource,
								 ResType 				theType,
								 short 					theID,
								 const char *			name);

#endif  /* CGLUESUPPORTED */

#if OLDROUTINENAMES
#define SizeResource(theResource) GetResourceSizeOnDisk(theResource)
#define MaxSizeRsrc(theResource) GetMaxResourceSize(theResource)
#define RmveResource(theResource) RemoveResource(theResource)

#endif  /* OLDROUTINENAMES */

/*
	These typedefs were originally created for the Copland Resource Mangager
*/
typedef short 							ResFileRefNum;
typedef short 							ResID;
typedef short 							ResAttributes;
typedef short 							ResFileAttributes;



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

#endif /* __RESOURCES__ */

