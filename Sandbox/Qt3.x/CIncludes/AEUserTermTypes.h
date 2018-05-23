/*
 	File:		AEUserTermTypes.h
 
 	Contains:	AppleEvents AEUT resource format Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1991-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __AEUSERTERMTYPES__
#define __AEUSERTERMTYPES__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __MACTYPES__
#include <MacTypes.h>
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
	kAEUserTerminology			= FOUR_CHAR_CODE('aeut'),		/*  0x61657574  */
	kAETerminologyExtension		= FOUR_CHAR_CODE('aete'),		/*  0x61657465  */
	kAEScriptingSizeResource	= FOUR_CHAR_CODE('scsz')		/*  0x7363737a  */
};


enum {
	kAEUTHasReturningParam		= 31,							/* if event has a keyASReturning param */
	kAEUTOptional				= 15,							/* if something is optional */
	kAEUTlistOfItems			= 14,							/* if property or reply is a list. */
	kAEUTEnumerated				= 13,							/* if property or reply is of an enumerated type. */
	kAEUTReadWrite				= 12,							/* if property is writable. */
	kAEUTChangesState			= 12,							/* if an event changes state. */
	kAEUTTightBindingFunction	= 12,							/* if this is a tight-binding precedence function. */
	kAEUTApostrophe				= 3,							/* if a term contains an apostrophe. */
	kAEUTFeminine				= 2,							/* if a term is feminine gender. */
	kAEUTMasculine				= 1,							/* if a term is masculine gender. */
	kAEUTPlural					= 0								/* if a term is plural. */
};


struct TScriptingSizeResource {
	short 							scriptingSizeFlags;
	unsigned long 					minStackSize;
	unsigned long 					preferredStackSize;
	unsigned long 					maxStackSize;
	unsigned long 					minHeapSize;
	unsigned long 					preferredHeapSize;
	unsigned long 					maxHeapSize;
};
typedef struct TScriptingSizeResource	TScriptingSizeResource;

enum {
	kLaunchToGetTerminology		= (1 << 15),					/*	If kLaunchToGetTerminology is 0, 'aete' is read directly from res file.  If set to 1, then launch and use 'gdut' to get terminology. */
	kDontFindAppBySignature		= (1 << 14),					/*	If kDontFindAppBySignature is 0, then find app with signature if lost.  If 1, then don't */
	kAlwaysSendSubject			= (1 << 13)						/* 	If kAlwaysSendSubject 0, then send subject when appropriate. If 1, then every event has Subject Attribute */
};

/* old names for above bits. */

enum {
	kReadExtensionTermsMask		= (1 << 15)
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

#endif /* __AEUSERTERMTYPES__ */

