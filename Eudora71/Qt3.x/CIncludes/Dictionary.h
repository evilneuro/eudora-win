/*
 	File:		Dictionary.h
 
 	Contains:	Dictionary Manager Interfaces
 
 	Version:	Technology:	System 7
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1992-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __DICTIONARY__
#define __DICTIONARY__

#ifndef __MACTYPES__
#include <MacTypes.h>
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
																/* Dictionary data insertion modes */
	kInsert						= 0,							/* Only insert the input entry if there is nothing in the dictionary that matches the key. */
	kReplace					= 1,							/* Only replace the entries which match the key with the input entry. */
	kInsertOrReplace			= 2								/* Insert the entry if there is nothing in the dictionary which matches the key, otherwise replaces the existing matched entries with the input entry. */
};

/* This Was InsertMode */
typedef short 							DictionaryDataInsertMode;

enum {
																/* Key attribute constants */
	kIsCaseSensitive			= 0x10,							/* case sensitive = 16		*/
	kIsNotDiacriticalSensitive	= 0x20							/* diac not sensitive = 32	*/
};


enum {
																/* Registered attribute type constants.	*/
	kNoun						= -1,
	kVerb						= -2,
	kAdjective					= -3,
	kAdverb						= -4
};

/* This Was AttributeType */
typedef SInt8 							DictionaryEntryAttribute;
/* Dictionary information record */

struct DictionaryInformation {
	FSSpec 							dictionaryFSSpec;
	SInt32 							numberOfRecords;
	SInt32 							currentGarbageSize;
	ScriptCode 						script;
	SInt16 							maximumKeyLength;
	SInt8 							keyAttributes;
};
typedef struct DictionaryInformation	DictionaryInformation;

struct DictionaryAttributeTable {
	UInt8 							datSize;
	DictionaryEntryAttribute 		datTable[1];
};
typedef struct DictionaryAttributeTable	DictionaryAttributeTable;
typedef DictionaryAttributeTable *		DictionaryAttributeTablePtr;
EXTERN_API( OSErr )
InitializeDictionary			(const FSSpec *			theFsspecPtr,
								 SInt16 				maximumKeyLength,
								 SInt8 					keyAttributes,
								 ScriptCode 			script)								THREEWORDINLINE(0x303C, 0x0500, 0xAA53);

EXTERN_API( OSErr )
OpenDictionary					(const FSSpec *			theFsspecPtr,
								 SInt8 					accessPermission,
								 SInt32 *				dictionaryReference)				THREEWORDINLINE(0x303C, 0x0501, 0xAA53);

EXTERN_API( OSErr )
CloseDictionary					(SInt32 				dictionaryReference)				THREEWORDINLINE(0x303C, 0x0202, 0xAA53);

EXTERN_API( OSErr )
InsertRecordToDictionary		(SInt32 				dictionaryReference,
								 ConstStr255Param 		key,
								 Handle 				recordDataHandle,
								 DictionaryDataInsertMode  whichMode)						THREEWORDINLINE(0x303C, 0x0703, 0xAA53);

EXTERN_API( OSErr )
DeleteRecordFromDictionary		(SInt32 				dictionaryReference,
								 ConstStr255Param 		key)								THREEWORDINLINE(0x303C, 0x0404, 0xAA53);

EXTERN_API( OSErr )
FindRecordInDictionary			(SInt32 				dictionaryReference,
								 ConstStr255Param 		key,
								 DictionaryAttributeTablePtr  requestedAttributeTablePointer,
								 Handle 				recordDataHandle)					THREEWORDINLINE(0x303C, 0x0805, 0xAA53);

EXTERN_API( OSErr )
FindRecordByIndexInDictionary	(SInt32 				dictionaryReference,
								 SInt32 				recordIndex,
								 DictionaryAttributeTablePtr  requestedAttributeTablePointer,
								 Str255 				recordKey,
								 Handle 				recordDataHandle)					THREEWORDINLINE(0x303C, 0x0A06, 0xAA53);

EXTERN_API( OSErr )
GetDictionaryInformation		(SInt32 				dictionaryReference,
								 DictionaryInformation * theDictionaryInformation)			THREEWORDINLINE(0x303C, 0x0407, 0xAA53);

EXTERN_API( OSErr )
CompactDictionary				(SInt32 				dictionaryReference)				THREEWORDINLINE(0x303C, 0x0208, 0xAA53);




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

#endif /* __DICTIONARY__ */

