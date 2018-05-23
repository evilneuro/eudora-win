/*
 	File:		NameRegistry.h
 
 	Contains:	NameRegistry Interfaces
 
 	Version:	Technology:	MacOS
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1993-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __NAMEREGISTRY__
#define __NAMEREGISTRY__

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
	#pragma options align=power
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

/*******************************************************************************
 * 
 * Foundation Types
 *
 */
/* Value of a property */
typedef void *							RegPropertyValue;
/* Length of property value */
typedef UInt32 							RegPropertyValueSize;
/*******************************************************************************
 * 
 * RegEntryID	:	The Global x-Namespace Entry Identifier
 *
 */

struct RegEntryID {
	UInt32 							contents[4];
};
typedef struct RegEntryID				RegEntryID;
typedef RegEntryID *					RegEntryIDPtr;
/*******************************************************************************
 *
 * Root Entry Name Definitions	(Applies to all Names in the RootNameSpace)
 *
 *	¥ Names are a colon-separated list of name components.  Name components
 *	  may not themselves contain colons.  
 *	¥ Names are presented as null-terminated ASCII character strings.
 *	¥ Names follow similar parsing rules to Apple file system absolute
 *	  and relative paths.  However the '::' parent directory syntax is
 *	  not currently supported.
 */
/* Max length of Entry Name */

enum {
	kRegCStrMaxEntryNameLength	= 47
};

/* Entry Names are single byte ASCII */

typedef char 							RegCStrEntryName;
typedef char *							RegCStrEntryNamePtr;
/* length of RegCStrEntryNameBuf =  kRegCStrMaxEntryNameLength+1*/
typedef char 							RegCStrEntryNameBuf[48];
typedef char 							RegCStrPathName;
typedef UInt32 							RegPathNameSize;

enum {
	kRegPathNameSeparator		= ':',							/* 0x3A */
	kRegEntryNameTerminator		= 0x00,							/* '\0' */
	kRegPathNameTerminator		= 0x00							/* '\0' */
};

/*******************************************************************************
 *
 * Property Name and ID Definitions
 *	(Applies to all Properties Regardless of NameSpace)
 */

enum {
	kRegMaximumPropertyNameLength = 31,							/* Max length of Property Name */
	kRegPropertyNameTerminator	= 0x00							/* '\0' */
};

typedef char 							RegPropertyNameBuf[32];
typedef char 							RegPropertyName;
typedef char *							RegPropertyNamePtr;
/*******************************************************************************
 *
 * Iteration Operations
 *
 *	These specify direction when traversing the name relationships
 */
typedef UInt32 							RegIterationOp;
typedef RegIterationOp 					RegEntryIterationOp;

enum {
																/* Absolute locations*/
	kRegIterRoot				= 0x00000002,					/* "Upward" Relationships	*/
	kRegIterParents				= 0x00000003,					/* include all  parent(s) of entry */
																/* "Downward" Relationships*/
	kRegIterChildren			= 0x00000004,					/* include all children */
	kRegIterSubTrees			= 0x00000005,					/* include all sub trees of entry */
	kRegIterDescendants			= 0x00000005,					/* include all descendants of entry */
																/* "Horizontal" Relationships	*/
	kRegIterSibling				= 0x00000006,					/* include all siblings */
																/* Keep doing the same thing*/
	kRegIterContinue			= 0x00000001
};

/*******************************************************************************
 *
 * Name Entry and Property Modifiers
 *
 *
 *
 * Modifiers describe special characteristics of names
 * and properties.  Modifiers might be supported for
 * some names and not others.
 * 
 * Device Drivers should not rely on functionality
 * specified as a modifier.
 */
typedef UInt32 							RegModifiers;
typedef RegModifiers 					RegEntryModifiers;
typedef RegModifiers 					RegPropertyModifiers;

enum {
	kRegNoModifiers				= 0x00000000,					/* no entry modifiers in place */
	kRegUniversalModifierMask	= 0x0000FFFF,					/* mods to all entries */
	kRegNameSpaceModifierMask	= 0x00FF0000,					/* mods to all entries within namespace */
	kRegModifierMask			= (long)0xFF000000				/* mods to just this entry */
};

/* Universal Property Modifiers */

enum {
	kRegPropertyValueIsSavedToNVRAM = 0x00000020,				/* property is non-volatile (saved in NVRAM) */
	kRegPropertyValueIsSavedToDisk = 0x00000040					/* property is non-volatile (saved on disk) */
};

/* ///////////////////////
//
// The Registry API
//
/////////////////////// */
/* ///////////////////////
//
// Entry Management
//
/////////////////////// */

/*-------------------------------
 * EntryID handling
 */
/*
 * Initialize an EntryID to a known invalid state
 *   note: invalid != uninitialized
 */
EXTERN_API_C( OSStatus )
RegistryEntryIDInit				(RegEntryID *			id);

/*
 * Compare EntryID's for equality or if invalid
 *
 * If a NULL value is given for either id1 or id2, the other id 
 * is compared with an invalid ID.  If both are NULL, the id's 
 * are consided equal (result = true). 
 */
EXTERN_API_C( Boolean )
RegistryEntryIDCompare			(const RegEntryID *		id1,
								 const RegEntryID *		id2);

/*
 * Copy an EntryID
 */
EXTERN_API_C( OSStatus )
RegistryEntryIDCopy				(const RegEntryID *		src,
								 RegEntryID *			dst);

/*
 * Free an ID so it can be reused.
 */
EXTERN_API_C( OSStatus )
RegistryEntryIDDispose			(RegEntryID *			id);

/*-------------------------------
 * Adding and removing entries
 *
 * If (parentEntry) is NULL, the name is assumed
 * to be a rooted path. It is rooted to an anonymous, unnamed root.
 */
EXTERN_API_C( OSStatus )
RegistryCStrEntryCreate			(const RegEntryID *		parentEntry,
								 const RegCStrPathName * name,
								 RegEntryID *			newEntry);

EXTERN_API_C( OSStatus )
RegistryEntryDelete				(const RegEntryID *		id);

EXTERN_API_C( OSStatus )
RegistryEntryCopy				(RegEntryID *			parentEntryID,
								 RegEntryID *			sourceDevice,
								 RegEntryID *			destDevice);

/*---------------------------
 * Traversing the namespace
 *
 * To support arbitrary namespace implementations in the future,
 * I have hidden the form that the place pointer takes.  The previous
 * interface exposed the place pointer by specifying it as a
 * RegEntryID.
 *
 * I have also removed any notion of returning the entries
 * in a particular order, because an implementation might
 * return the names in semi-random order.  Many name service
 * implementations will store the names in a hashed lookup
 * table.
 *
 * Writing code to traverse some set of names consists of
 * a call to begin the iteration, the iteration loop, and
 * a call to end the iteration.  The begin call initializes
 * the iteration cookie data structure.  The call to end the 
 * iteration should be called even in the case of error so 
 * that allocated data structures can be freed.
 *
 *	Create(...)
 *	do {
 *		Iterate(...);
 *	} while (!done);
 *	Dispose(...);
 *
 * This is the basic code structure for callers of the iteration
 * interface.
 */
typedef struct OpaqueRegEntryIter* 		RegEntryIter;
/* 
 * create/dispose the iterator structure
 *   defaults to root with relationship = kRegIterDescendants
 */
EXTERN_API_C( OSStatus )
RegistryEntryIterateCreate		(RegEntryIter *			cookie);

EXTERN_API_C( OSStatus )
RegistryEntryIterateDispose		(RegEntryIter *			cookie);

/* 
 * set Entry Iterator to specified entry
 */
EXTERN_API_C( OSStatus )
RegistryEntryIterateSet			(RegEntryIter *			cookie,
								 const RegEntryID *		startEntryID);

/*
 * Return each value of the iteration
 *
 * return entries related to the current entry
 * with the specified relationship
 */
EXTERN_API_C( OSStatus )
RegistryEntryIterate			(RegEntryIter *			cookie,
								 RegEntryIterationOp 	relationship,
								 RegEntryID *			foundEntry,
								 Boolean *				done);

/*
 * return entries with the specified property
 *
 * A NULL RegPropertyValue pointer will return an
 * entry with the property containing any value.
 */
EXTERN_API_C( OSStatus )
RegistryEntrySearch				(RegEntryIter *			cookie,
								 RegEntryIterationOp 	relationship,
								 RegEntryID *			foundEntry,
								 Boolean *				done,
								 const RegPropertyName * propertyName,
								 const void *			propertyValue,
								 RegPropertyValueSize 	propertySize);

/*--------------------------------
 * Find a name in the namespace
 *
 * This is the fast lookup mechanism.
 * NOTE:  A reverse lookup mechanism
 *	  has not been provided because
 *        some name services may not
 *        provide a fast, general reverse
 *        lookup.
 */
EXTERN_API_C( OSStatus )
RegistryCStrEntryLookup			(const RegEntryID *		searchPointID,
								 const RegCStrPathName * pathName,
								 RegEntryID *			foundEntry);

/*---------------------------------------------
 * Convert an entry to a rooted name string
 *
 * A utility routine to turn an Entry ID
 * back into a name string.
 */
EXTERN_API_C( OSStatus )
RegistryEntryToPathSize			(const RegEntryID *		entryID,
								 RegPathNameSize *		pathSize);

EXTERN_API_C( OSStatus )
RegistryCStrEntryToPath			(const RegEntryID *		entryID,
								 RegCStrPathName *		pathName,
								 RegPathNameSize 		pathSize);

/*
 * Parse a path name.
 *
 * Retrieve the last component of the path, and
 * return a spec for the parent.
 */
EXTERN_API_C( OSStatus )
RegistryCStrEntryToName			(const RegEntryID *		entryID,
								 RegEntryID *			parentEntry,
								 RegCStrEntryName *		nameComponent,
								 Boolean *				done);

/* //////////////////////////////////////////////////////
//
// Property Management
//
////////////////////////////////////////////////////// */
/*-------------------------------
 * Adding and removing properties
 */
EXTERN_API_C( OSStatus )
RegistryPropertyCreate			(const RegEntryID *		entryID,
								 const RegPropertyName * propertyName,
								 const void *			propertyValue,
								 RegPropertyValueSize 	propertySize);

EXTERN_API_C( OSStatus )
RegistryPropertyDelete			(const RegEntryID *		entryID,
								 const RegPropertyName * propertyName);

EXTERN_API_C( OSStatus )
RegistryPropertyRename			(const RegEntryID *		entry,
								 const RegPropertyName * oldName,
								 const RegPropertyName * newName);

/*---------------------------
 * Traversing the Properties of a name
 *
 */
typedef struct OpaqueRegPropertyIter* 	RegPropertyIter;
EXTERN_API_C( OSStatus )
RegistryPropertyIterateCreate	(const RegEntryID *		entry,
								 RegPropertyIter *		cookie);

EXTERN_API_C( OSStatus )
RegistryPropertyIterateDispose	(RegPropertyIter *		cookie);

EXTERN_API_C( OSStatus )
RegistryPropertyIterate			(RegPropertyIter *		cookie,
								 RegPropertyName *		foundProperty,
								 Boolean *				done);

/*
 * Get the value of the specified property for the specified entry.
 *
 */
EXTERN_API_C( OSStatus )
RegistryPropertyGetSize			(const RegEntryID *		entryID,
								 const RegPropertyName * propertyName,
								 RegPropertyValueSize *	propertySize);

/*
 * (*propertySize) is the maximum size of the value returned in the buffer
 * pointed to by (propertyValue).  Upon return, (*propertySize) is the size of the
 * value returned.
 */
EXTERN_API_C( OSStatus )
RegistryPropertyGet				(const RegEntryID *		entryID,
								 const RegPropertyName * propertyName,
								 void *					propertyValue,
								 RegPropertyValueSize *	propertySize);

EXTERN_API_C( OSStatus )
RegistryPropertySet				(const RegEntryID *		entryID,
								 const RegPropertyName * propertyName,
								 const void *			propertyValue,
								 RegPropertyValueSize 	propertySize);

/* //////////////////////////////////////////////////////
//
// Modifier Management
//
////////////////////////////////////////////////////// */
/*
 * Modifiers describe special characteristics of names
 * and properties.  Modifiers might be supported for
 * some names and not others.
 * 
 * Device Drivers should not rely on functionality
 * specified as a modifier.  These interfaces
 * are for use in writing Experts.
 */
/*
 * Get and Set operators for entry modifiers
 */
EXTERN_API_C( OSStatus )
RegistryEntryGetMod				(const RegEntryID *		entry,
								 RegEntryModifiers *	modifiers);

EXTERN_API_C( OSStatus )
RegistryEntrySetMod				(const RegEntryID *		entry,
								 RegEntryModifiers 		modifiers);

/*
 * Get and Set operators for property modifiers
 */
EXTERN_API_C( OSStatus )
RegistryPropertyGetMod			(const RegEntryID *		entry,
								 const RegPropertyName * name,
								 RegPropertyModifiers *	modifiers);

EXTERN_API_C( OSStatus )
RegistryPropertySetMod			(const RegEntryID *		entry,
								 const RegPropertyName * name,
								 RegPropertyModifiers 	modifiers);

/*
 * Iterator operator for entry modifier search
 */
EXTERN_API_C( OSStatus )
RegistryEntryMod				(RegEntryIter *			cookie,
								 RegEntryIterationOp 	relationship,
								 RegEntryID *			foundEntry,
								 Boolean *				done,
								 RegEntryModifiers 		matchingModifiers);

/*
 * Iterator operator for entries with matching 
 * property modifiers
 */
EXTERN_API_C( OSStatus )
RegistryEntryPropertyMod		(RegEntryIter *			cookie,
								 RegEntryIterationOp 	relationship,
								 RegEntryID *			foundEntry,
								 Boolean *				done,
								 RegPropertyModifiers 	matchingModifiers);



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

#endif /* __NAMEREGISTRY__ */

