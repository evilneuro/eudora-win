/*
 	File:		ATSLayoutTypes.h
 
 	Contains:	Apple Text Services layout public structures and constants.
 
 	Version:	Technology:	Allegro
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1994-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __ATSLAYOUTTYPES__
#define __ATSLAYOUTTYPES__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __SFNTTYPES__
#include <SFNTTypes.h>
#endif
#ifndef __SFNTLAYOUTTYPES__
#include <SFNTLayoutTypes.h>
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

/* ----------------------------------------------------------------------------------------- */
/* CONSTANTS */
/* LineOptions flags */

enum {
	kATSLineNoLayoutOptions		= 0x00000000,
	kATSLineIsDisplayOnly		= 0x00000001,
	kATSLineHasNoHangers		= 0x00000002,
	kATSLineHasNoOpticalAlignment = 0x00000004,
	kATSLineKeepSpacesInMargin	= 0x00000008,
	kATSLineTwoLevelReordering	= 0x00000010,
	kATSLineLeftEdgeNotMargin	= 0x00000020,
	kATSLineRightEdgeNotMargin	= 0x00000040,
	kATSLineDoReordering		= 0x00000080,
	kATSLineNoJustPostComp		= 0x00000100
};

/* Run controls flags (in the style rec) */

enum {
	kATSRunNoLigatureSplits		= (long)0x80000000,
	kATSRunNoCaretAngle			= 0x40000000,
	kATSRunImposeWidth			= 0x20000000,
	kATSRunNoCrossKerning		= 0x10000000,
	kATSRunNoOpticalAlignment	= 0x08000000,
	kATSRunForceHanging			= 0x04000000,
	kATSRunNoSpecialJustification = 0x02000000,
	kATSRunIsVertical			= 0x01000000,
	kATSRunDirectionOverrideMask = 0x00000003
};

/* Direction overrides */

enum {
	kATSNoDirectionOverride		= 0,
	kATSImposeLeftToRight		= 1,
	kATSImposeRightToLeft		= 2
};

/* Miscellaneous constants */

enum {
	kATSNoTracking				= (long)0x80000000,				/* negativeInfinity */
	kATSDeletedGlyphcode		= 0xFFFF,
	kATSSelectToEnd				= (long)0xFFFFFFFF,
	kATSOffsetToNoData			= (long)0xFFFFFFFF
};

/* --------------------------------------------------------------------------- */
/* TYPES */
/* --------------------------------------------------------------------------- */
typedef UInt32 							ATSLineLayoutOptions;
/*
	The JustWidthDeltaEntryOverride structure specifies values for the grow and shrink case during
	justification, both on the left and on the right. It also contains flags.  This particular structure
	is used for passing justification overrides to LLC.
*/

struct ATSJustWidthDeltaEntryOverride {
	Fixed 							beforeGrowLimit;			/* ems AW can grow by at most on LT */
	Fixed 							beforeShrinkLimit;			/* ems AW can shrink by at most on LT */
	Fixed 							afterGrowLimit;				/* ems AW can grow by at most on RB */
	Fixed 							afterShrinkLimit;			/* ems AW can shrink by at most on RB */
	JustificationFlags 				growFlags;					/* flags controlling grow case */
	JustificationFlags 				shrinkFlags;				/* flags controlling shrink case */
};
typedef struct ATSJustWidthDeltaEntryOverride ATSJustWidthDeltaEntryOverride;
/* The JustPriorityOverrides type is an array of 4 width delta records, one per priority level override. */

typedef ATSJustWidthDeltaEntryOverride 	ATSJustPriorityWidthDeltaOverrides[4];
/*
	The JustGlyphWidthDeltaOverride structure specifies justification override information for a
	single specified glyph.
*/

struct ATSJustGlyphWidthDeltaOverride {
	UInt16 							glyph;
	ATSJustWidthDeltaEntryOverride 	override;
};
typedef struct ATSJustGlyphWidthDeltaOverride ATSJustGlyphWidthDeltaOverride;
/* --------------------------------------------------------------------------- */

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

#endif /* __ATSLAYOUTTYPES__ */

