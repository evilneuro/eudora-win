/*
 	File:		TextEdit.h
 
 	Contains:	TextEdit Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __TEXTEDIT__
#define __TEXTEDIT__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __MIXEDMODE__
#include <MixedMode.h>
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

typedef struct TERec 					TERec;
typedef TERec *							TEPtr;
typedef TEPtr *							TEHandle;
typedef CALLBACK_API( void , HighHookProcPtr )(const Rect *r, TEPtr pTE);
/*
	WARNING: HighHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( Boolean , EOLHookProcPtr )(char theChar, TEPtr pTE, TEHandle hTE);
/*
	WARNING: EOLHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( void , CaretHookProcPtr )(const Rect *r, TEPtr pTE);
/*
	WARNING: CaretHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( unsigned short , WidthHookProcPtr )(unsigned short textLen, unsigned short textOffset, void *textBufferPtr, TEPtr pTE, TEHandle hTE);
/*
	WARNING: WidthHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( unsigned short , TextWidthHookProcPtr )(unsigned short textLen, unsigned short textOffset, void *textBufferPtr, TEPtr pTE, TEHandle hTE);
/*
	WARNING: TextWidthHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( unsigned short , NWidthHookProcPtr )(unsigned short styleRunLen, unsigned short styleRunOffset, short slop, short direction, void *textBufferPtr, short *lineStart, TEPtr pTE, TEHandle hTE);
/*
	WARNING: NWidthHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( void , DrawHookProcPtr )(unsigned short textOffset, unsigned short drawLen, void *textBufferPtr, TEPtr pTE, TEHandle hTE);
/*
	WARNING: DrawHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( Boolean , HitTestHookProcPtr )(unsigned short styleRunLen, unsigned short styleRunOffset, unsigned short slop, void *textBufferPtr, TEPtr pTE, TEHandle hTE, unsigned short *pixelWidth, unsigned short *charOffset, Boolean *pixelInChar);
/*
	WARNING: HitTestHookProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( void , TEFindWordProcPtr )(unsigned short currentPos, short caller, TEPtr pTE, TEHandle hTE, unsigned short *wordStart, unsigned short *wordEnd);
/*
	WARNING: TEFindWordProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( void , TERecalcProcPtr )(TEPtr pTE, unsigned short changeLength, unsigned short *lineStart, unsigned short *firstChar, unsigned short *lastChar);
/*
	WARNING: TERecalcProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( void , TEDoTextProcPtr )(TEPtr pTE, unsigned short firstChar, unsigned short lastChar, short selector, GrafPtr *currentGrafPort, short *charPosition);
/*
	WARNING: TEDoTextProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( Boolean , TEClickLoopProcPtr )(TEPtr pTE);
/*
	WARNING: TEClickLoopProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef CALLBACK_API( Boolean , WordBreakProcPtr )(Ptr text, short charPos);
/*
	WARNING: WordBreakProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
/* 
	Important note about TEClickLoopProcPtr and WordBreakProcPtr

	At one point these were defined as returning the function result in the 
	condition code Z-bit.  This was correct, in that it was what the 68K
	implementation of TextEdit actually tested.  But, MixedMode had a different 
	idea of what returning a boolean in the Z-bit meant.  MixedMode was setting
	the Z-bit the complement of what was wanted.  
	
	Therefore, these ProcPtrs have been changed (back) to return the result in
	register D0.  It turns out that for register based routines, 
	MixedMode sets the Z-bit of the 68K emulator based on the contents 
	of the return result register.  Thus we can get the Z-bit set correctly.  
	
	But, when TextEdit is recoded in PowerPC, if it calls a 68K ClickLoop
	or WordBreak routine, register D0 had better have the result (in addition
	to the Z-bit). Therefore all 68K apps should make sure their ClickLoop or
	WordBreak routines set register D0 at the end.
*/

/* 
	There is no function to get/set the low-mem for FindWordHook at 0x07F8.
	This is because it is not a low-mem ProcPtr. That address is the entry
	in the OS TrapTable for trap 0xA0FE.  You can use Get/SetTrapAddress to 
	acccess it.	
*/

/*
	The following ProcPtrs cannot be written in or called from a high-level 
	language without the help of mixed mode or assembly glue because they 
	use the following parameter-passing conventions:

	typedef pascal void (*HighHookProcPtr)(const Rect *r, TEPtr pTE);
	typedef pascal void (*CaretHookProcPtr)(const Rect *r, TEPtr pTE);

		In:
			=> 	r						on stack
			=>	pTE						A3.L
		Out:
			none

	typedef pascal Boolean (*EOLHookProcPtr)(char theChar, TEPtr pTE, TEHandle hTE);

		In:
			=> 	theChar					D0.B
			=>	pTE						A3.L
			=>	hTE						A4.L
		Out:
			<=	Boolean					Z bit of the CCR

	typedef pascal unsigned short (*WidthHookProcPtr)(unsigned short textLen,
	 unsigned short textOffset, void *textBufferPtr, TEPtr pTE, TEHandle hTE);
	typedef pascal unsigned short (*TextWidthHookProcPtr)(unsigned short textLen,
	 unsigned short textOffset, void *textBufferPtr, TEPtr pTE, TEHandle hTE);

		In:
			=> 	textLen					D0.W
			=>	textOffset				D1.W
			=>	textBufferPtr			A0.L
			=>	pTE						A3.L
			=>	hTE						A4.L
		Out:
			<=	unsigned short			D1.W

	typedef pascal unsigned short (*NWidthHookProcPtr)(unsigned short styleRunLen,
	 unsigned short styleRunOffset, short slop, short direction, void *textBufferPtr, 
	 short *lineStart, TEPtr pTE, TEHandle hTE);

		In:
			=> 	styleRunLen				D0.W
			=>	styleRunOffset			D1.W
			=>	slop					D2.W (low)
			=>	direction				D2.W (high)
			=>	textBufferPtr			A0.L
			=>	lineStart				A2.L
			=>	pTE						A3.L
			=>	hTE						A4.L
		Out:
			<=	unsigned short			D1.W

	typedef pascal void (*DrawHookProcPtr)(unsigned short textOffset, unsigned short drawLen,
	 void *textBufferPtr, TEPtr pTE, TEHandle hTE);

		In:
			=> 	textOffset				D0.W
			=>	drawLen					D1.W
			=>	textBufferPtr			A0.L
			=>	pTE						A3.L
			=>	hTE						A4.L
		Out:
			none

	typedef pascal Boolean (*HitTestHookProcPtr)(unsigned short styleRunLen,
	 unsigned short styleRunOffset, unsigned short slop, void *textBufferPtr,
	 TEPtr pTE, TEHandle hTE, unsigned short *pixelWidth, unsigned short *charOffset, 
	 Boolean *pixelInChar);

		In:
			=> 	styleRunLen				D0.W
			=>	styleRunOffset			D1.W
			=>	slop					D2.W
			=>	textBufferPtr			A0.L
			=>	pTE						A3.L
			=>	hTE						A4.L
		Out:
			<=	pixelWidth				D0.W (low)
			<=	Boolean					D0.W (high)
			<=	charOffset				D1.W
			<=	pixelInChar				D2.W

	typedef pascal void (*TEFindWordProcPtr)(unsigned short currentPos, short caller, 
	 TEPtr pTE, TEHandle hTE, unsigned short *wordStart, unsigned short *wordEnd);

		In:
			=> 	currentPos				D0.W
			=>	caller					D2.W
			=>	pTE						A3.L
			=>	hTE						A4.L
		Out:
			<=	wordStart				D0.W
			<=	wordEnd					D1.W

	typedef pascal void (*TERecalcProcPtr)(TEPtr pTE, unsigned short changeLength,
  	 unsigned short *lineStart, unsigned short *firstChar, unsigned short *lastChar);

		In:
			=> 	pTE						A3.L
			=>	changeLength			D7.W
		Out:
			<=	lineStart				D2.W
			<=	firstChar				D3.W
			<=	lastChar				D4.W

	typedef pascal void (*TEDoTextProcPtr)(TEPtr pTE, unsigned short firstChar, unsigned short lastChar,
	 					short selector, GrafPtr *currentGrafPort, short *charPosition);

		In:
			=> 	pTE						A3.L
			=>	firstChar				D3.W
			=>	lastChar				D4.W
			=>	selector				D7.W
		Out:
			<=	currentGrafPort			A0.L
			<=	charPosition			D0.W
			
*/
typedef REGISTER_UPP_TYPE(HighHookProcPtr) 						HighHookUPP;
typedef REGISTER_UPP_TYPE(EOLHookProcPtr) 						EOLHookUPP;
typedef REGISTER_UPP_TYPE(CaretHookProcPtr) 					CaretHookUPP;
typedef REGISTER_UPP_TYPE(WidthHookProcPtr) 					WidthHookUPP;
typedef REGISTER_UPP_TYPE(TextWidthHookProcPtr) 				TextWidthHookUPP;
typedef REGISTER_UPP_TYPE(NWidthHookProcPtr) 					NWidthHookUPP;
typedef REGISTER_UPP_TYPE(DrawHookProcPtr) 						DrawHookUPP;
typedef REGISTER_UPP_TYPE(HitTestHookProcPtr) 					HitTestHookUPP;
typedef REGISTER_UPP_TYPE(TEFindWordProcPtr) 					TEFindWordUPP;
typedef REGISTER_UPP_TYPE(TERecalcProcPtr) 						TERecalcUPP;
typedef REGISTER_UPP_TYPE(TEDoTextProcPtr) 						TEDoTextUPP;
typedef REGISTER_UPP_TYPE(TEClickLoopProcPtr) 					TEClickLoopUPP;
typedef REGISTER_UPP_TYPE(WordBreakProcPtr) 					WordBreakUPP;

struct TERec {
	Rect 							destRect;
	Rect 							viewRect;
	Rect 							selRect;
	short 							lineHeight;
	short 							fontAscent;
	Point 							selPoint;
	short 							selStart;
	short 							selEnd;
	short 							active;
	WordBreakUPP 					wordBreak;
	TEClickLoopUPP 					clickLoop;
	long 							clickTime;
	short 							clickLoc;
	long 							caretTime;
	short 							caretState;
	short 							just;
	short 							teLength;
	Handle 							hText;
	long 							hDispatchRec;				/* added to replace recalBack & recalLines.  it's a handle anyway */
	short 							clikStuff;
	short 							crOnly;
	short 							txFont;
	StyleField 						txFace;						/*txFace is unpacked byte*/
	short 							txMode;
	short 							txSize;
	GrafPtr 						inPort;
	HighHookUPP 					highHook;
	CaretHookUPP 					caretHook;
	short 							nLines;
	short 							lineStarts[16001];
};


enum {
																/* Justification (word alignment) styles */
	teJustLeft					= 0,
	teJustCenter				= 1,
	teJustRight					= -1,
	teForceLeft					= -2,							/* new names for the Justification (word alignment) styles */
	teFlushDefault				= 0,							/*flush according to the line direction */
	teCenter					= 1,							/*center justify (word alignment) */
	teFlushRight				= -1,							/*flush right for all scripts */
	teFlushLeft					= -2							/*flush left for all scripts */
};


enum {
																/* Set/Replace style modes */
	fontBit						= 0,							/*set font*/
	faceBit						= 1,							/*set face*/
	sizeBit						= 2,							/*set size*/
	clrBit						= 3,							/*set color*/
	addSizeBit					= 4,							/*add size mode*/
	toggleBit					= 5								/*set faces in toggle mode*/
};


enum {
																/* TESetStyle/TEContinuousStyle modes */
	doFont						= 1,							/* set font (family) number*/
	doFace						= 2,							/*set character style*/
	doSize						= 4,							/*set type size*/
	doColor						= 8,							/*set color*/
	doAll						= 15,							/*set all attributes*/
	addSize						= 16,							/*adjust type size*/
	doToggle					= 32							/*toggle mode for TESetStyle*/
};


enum {
																/* offsets into TEDispatchRec */
	EOLHook						= 0,							/*[UniversalProcPtr] TEEOLHook*/
	DRAWHook					= 4,							/*[UniversalProcPtr] TEWidthHook*/
	WIDTHHook					= 8,							/*[UniversalProcPtr] TEDrawHook*/
	HITTESTHook					= 12,							/*[UniversalProcPtr] TEHitTestHook*/
	nWIDTHHook					= 24,							/*[UniversalProcPtr] nTEWidthHook*/
	TextWidthHook				= 28							/*[UniversalProcPtr] TETextWidthHook*/
};


enum {
																/* selectors for TECustomHook */
	intEOLHook					= 0,							/*TEIntHook value*/
	intDrawHook					= 1,							/*TEIntHook value*/
	intWidthHook				= 2,							/*TEIntHook value*/
	intHitTestHook				= 3,							/*TEIntHook value*/
	intNWidthHook				= 6,							/*TEIntHook value for new version of WidthHook*/
	intTextWidthHook			= 7,							/*TEIntHook value for new TextWidthHook*/
	intInlineInputTSMTEPreUpdateHook = 8,						/*TEIntHook value for TSMTEPreUpdateProcPtr callback*/
	intInlineInputTSMTEPostUpdateHook = 9						/*TEIntHook value for TSMTEPostUpdateProcPtr callback*/
};


enum {
																/* feature or bit definitions for TEFeatureFlag */
	teFAutoScroll				= 0,							/*00000001b*/
	teFTextBuffering			= 1,							/*00000010b*/
	teFOutlineHilite			= 2,							/*00000100b*/
	teFInlineInput				= 3,							/*00001000b */
	teFUseWhiteBackground		= 4,							/*00010000b */
	teFUseInlineInput			= 5,							/*00100000b */
	teFInlineInputAutoScroll	= 6								/*01000000b */
};


enum {
																/* action for the new "bit (un)set" interface, TEFeatureFlag */
	teBitClear					= 0,
	teBitSet					= 1,							/*set the selector bit*/
	teBitTest					= -1							/*no change; just return the current setting*/
};


enum {
																/*constants for identifying the routine that called FindWord */
	teWordSelect				= 4,							/*clickExpand to select word*/
	teWordDrag					= 8,							/*clickExpand to drag new word*/
	teFromFind					= 12,							/*FindLine called it ($0C)*/
	teFromRecal					= 16							/*RecalLines called it ($10)      obsolete */
};


enum {
																/*constants for identifying TEDoText selectors */
	teFind						= 0,							/*TEDoText called for searching*/
	teHighlight					= 1,							/*TEDoText called for highlighting*/
	teDraw						= -1,							/*TEDoText called for drawing text*/
	teCaret						= -2							/*TEDoText called for drawing the caret*/
};



typedef char 							Chars[32001];
typedef char *							CharsPtr;
typedef CharsPtr *						CharsHandle;

struct StyleRun {
	short 							startChar;					/*starting character position*/
	short 							styleIndex;					/*index in style table*/
};
typedef struct StyleRun					StyleRun;

struct STElement {
	short 							stCount;					/*number of runs in this style*/
	short 							stHeight;					/*line height*/
	short 							stAscent;					/*font ascent*/
	short 							stFont;						/*font (family) number*/
	StyleField 						stFace;						/*character Style*/
	short 							stSize;						/*size in points*/
	RGBColor 						stColor;					/*absolute (RGB) color*/
};
typedef struct STElement				STElement;

typedef STElement 						TEStyleTable[1777];
typedef STElement *						STPtr;
typedef STPtr *							STHandle;

struct LHElement {
	short 							lhHeight;					/*maximum height in line*/
	short 							lhAscent;					/*maximum ascent in line*/
};
typedef struct LHElement				LHElement;

typedef LHElement 						LHTable[8001];
typedef LHElement *						LHPtr;
typedef LHPtr *							LHHandle;

struct ScrpSTElement {
	long 							scrpStartChar;				/*starting character position*/
	short 							scrpHeight;
	short 							scrpAscent;
	short 							scrpFont;
	StyleField 						scrpFace;					/*unpacked byte*/
	short 							scrpSize;
	RGBColor 						scrpColor;
};
typedef struct ScrpSTElement			ScrpSTElement;
/* ARRAY [0..1600] OF ScrpSTElement */

typedef ScrpSTElement 					ScrpSTTable[1601];

struct StScrpRec {
	short 							scrpNStyles;				/*number of styles in scrap*/
	ScrpSTTable 					scrpStyleTab;				/*table of styles for scrap*/
};
typedef struct StScrpRec				StScrpRec;

typedef StScrpRec *						StScrpPtr;
typedef StScrpPtr *						StScrpHandle;

struct NullStRec {
	long 							teReserved;					/*reserved for future expansion*/
	StScrpHandle 					nullScrap;					/*handle to scrap style table*/
};
typedef struct NullStRec				NullStRec;

typedef NullStRec *						NullStPtr;
typedef NullStPtr *						NullStHandle;

struct TEStyleRec {
	short 							nRuns;						/*number of style runs*/
	short 							nStyles;					/*size of style table*/
	STHandle 						styleTab;					/*handle to style table*/
	LHHandle 						lhTab;						/*handle to line-height table*/
	long 							teRefCon;					/*reserved for application use*/
	NullStHandle 					nullStyle;					/*Handle to style set at null selection*/
	StyleRun 						runs[8001];					/*ARRAY [0..8000] OF StyleRun*/
};
typedef struct TEStyleRec				TEStyleRec;

typedef TEStyleRec *					TEStylePtr;
typedef TEStylePtr *					TEStyleHandle;

struct TextStyle {
	short 							tsFont;						/*font (family) number*/
	StyleField 						tsFace;						/*character Style*/
	short 							tsSize;						/*size in point*/
	RGBColor 						tsColor;					/*absolute (RGB) color*/
};
typedef struct TextStyle				TextStyle;
typedef TextStyle *						TextStylePtr;
typedef TextStylePtr *					TextStyleHandle;

typedef short 							TEIntHook;
enum { uppHighHookProcInfo = 0x0000000F }; 						/* SPECIAL_CASE_PROCINFO(0) */
enum { uppEOLHookProcInfo = 0x0000001F }; 						/* SPECIAL_CASE_PROCINFO(1) */
enum { uppCaretHookProcInfo = 0x0000000F }; 					/* SPECIAL_CASE_PROCINFO(0) */
enum { uppWidthHookProcInfo = 0x0000002F }; 					/* SPECIAL_CASE_PROCINFO(2) */
enum { uppTextWidthHookProcInfo = 0x0000002F }; 				/* SPECIAL_CASE_PROCINFO(2) */
enum { uppNWidthHookProcInfo = 0x0000003F }; 					/* SPECIAL_CASE_PROCINFO(3) */
enum { uppDrawHookProcInfo = 0x0000004F }; 						/* SPECIAL_CASE_PROCINFO(4) */
enum { uppHitTestHookProcInfo = 0x0000005F }; 					/* SPECIAL_CASE_PROCINFO(5) */
enum { uppTEFindWordProcInfo = 0x0000006F }; 					/* SPECIAL_CASE_PROCINFO(6) */
enum { uppTERecalcProcInfo = 0x0000009F }; 						/* SPECIAL_CASE_PROCINFO(9) */
enum { uppTEDoTextProcInfo = 0x000000AF }; 						/* SPECIAL_CASE_PROCINFO(10) */
enum { uppTEClickLoopProcInfo = 0x0000F812 }; 					/* register 1_byte:D0 Func(4_bytes:A3) */
enum { uppWordBreakProcInfo = 0x00029812 }; 					/* register 1_byte:D0 Func(4_bytes:A0, 2_bytes:D0) */
#define NewHighHookProc(userRoutine) 							(HighHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppHighHookProcInfo, GetCurrentArchitecture())
#define NewEOLHookProc(userRoutine) 							(EOLHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppEOLHookProcInfo, GetCurrentArchitecture())
#define NewCaretHookProc(userRoutine) 							(CaretHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppCaretHookProcInfo, GetCurrentArchitecture())
#define NewWidthHookProc(userRoutine) 							(WidthHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppWidthHookProcInfo, GetCurrentArchitecture())
#define NewTextWidthHookProc(userRoutine) 						(TextWidthHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTextWidthHookProcInfo, GetCurrentArchitecture())
#define NewNWidthHookProc(userRoutine) 							(NWidthHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppNWidthHookProcInfo, GetCurrentArchitecture())
#define NewDrawHookProc(userRoutine) 							(DrawHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppDrawHookProcInfo, GetCurrentArchitecture())
#define NewHitTestHookProc(userRoutine) 						(HitTestHookUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppHitTestHookProcInfo, GetCurrentArchitecture())
#define NewTEFindWordProc(userRoutine) 							(TEFindWordUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTEFindWordProcInfo, GetCurrentArchitecture())
#define NewTERecalcProc(userRoutine) 							(TERecalcUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTERecalcProcInfo, GetCurrentArchitecture())
#define NewTEDoTextProc(userRoutine) 							(TEDoTextUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTEDoTextProcInfo, GetCurrentArchitecture())
#define NewTEClickLoopProc(userRoutine) 						(TEClickLoopUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTEClickLoopProcInfo, GetCurrentArchitecture())
#define NewWordBreakProc(userRoutine) 							(WordBreakUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppWordBreakProcInfo, GetCurrentArchitecture())
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallHighHookProc can't be called from classic 68k without glue code */
#else
	#define CallHighHookProc(userRoutine, r, pTE) 				CALL_TWO_PARAMETER_UPP((userRoutine), uppHighHookProcInfo, (r), (pTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallEOLHookProc can't be called from classic 68k without glue code */
#else
	#define CallEOLHookProc(userRoutine, theChar, pTE, hTE) 	CALL_THREE_PARAMETER_UPP((userRoutine), uppEOLHookProcInfo, (theChar), (pTE), (hTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallCaretHookProc can't be called from classic 68k without glue code */
#else
	#define CallCaretHookProc(userRoutine, r, pTE) 				CALL_TWO_PARAMETER_UPP((userRoutine), uppCaretHookProcInfo, (r), (pTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallWidthHookProc can't be called from classic 68k without glue code */
#else
	#define CallWidthHookProc(userRoutine, textLen, textOffset, textBufferPtr, pTE, hTE)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppWidthHookProcInfo, (textLen), (textOffset), (textBufferPtr), (pTE), (hTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallTextWidthHookProc can't be called from classic 68k without glue code */
#else
	#define CallTextWidthHookProc(userRoutine, textLen, textOffset, textBufferPtr, pTE, hTE)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppTextWidthHookProcInfo, (textLen), (textOffset), (textBufferPtr), (pTE), (hTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallNWidthHookProc can't be called from classic 68k without glue code */
#else
	#define CallNWidthHookProc(userRoutine, styleRunLen, styleRunOffset, slop, direction, textBufferPtr, lineStart, pTE, hTE)  CALL_EIGHT_PARAMETER_UPP((userRoutine), uppNWidthHookProcInfo, (styleRunLen), (styleRunOffset), (slop), (direction), (textBufferPtr), (lineStart), (pTE), (hTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallDrawHookProc can't be called from classic 68k without glue code */
#else
	#define CallDrawHookProc(userRoutine, textOffset, drawLen, textBufferPtr, pTE, hTE)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppDrawHookProcInfo, (textOffset), (drawLen), (textBufferPtr), (pTE), (hTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallHitTestHookProc can't be called from classic 68k without glue code */
#else
	#define CallHitTestHookProc(userRoutine, styleRunLen, styleRunOffset, slop, textBufferPtr, pTE, hTE, pixelWidth, charOffset, pixelInChar)  CALL_NINE_PARAMETER_UPP((userRoutine), uppHitTestHookProcInfo, (styleRunLen), (styleRunOffset), (slop), (textBufferPtr), (pTE), (hTE), (pixelWidth), (charOffset), (pixelInChar))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallTEFindWordProc can't be called from classic 68k without glue code */
#else
	#define CallTEFindWordProc(userRoutine, currentPos, caller, pTE, hTE, wordStart, wordEnd)  CALL_SIX_PARAMETER_UPP((userRoutine), uppTEFindWordProcInfo, (currentPos), (caller), (pTE), (hTE), (wordStart), (wordEnd))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallTERecalcProc can't be called from classic 68k without glue code */
#else
	#define CallTERecalcProc(userRoutine, pTE, changeLength, lineStart, firstChar, lastChar)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppTERecalcProcInfo, (pTE), (changeLength), (lineStart), (firstChar), (lastChar))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallTEDoTextProc can't be called from classic 68k without glue code */
#else
	#define CallTEDoTextProc(userRoutine, pTE, firstChar, lastChar, selector, currentGrafPort, charPosition)  CALL_SIX_PARAMETER_UPP((userRoutine), uppTEDoTextProcInfo, (pTE), (firstChar), (lastChar), (selector), (currentGrafPort), (charPosition))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	/* CallTEClickLoopProc can't be called from classic 68k without glue code */
#else
	#define CallTEClickLoopProc(userRoutine, pTE) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppTEClickLoopProcInfo, (pTE))
#endif
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter __D0 CallWordBreakProc(__A1, __A0, __D0)
	Boolean CallWordBreakProc(WordBreakUPP routine, Ptr text, short charPos) = 0x4E91;
#else
	#define CallWordBreakProc(userRoutine, text, charPos) 		CALL_TWO_PARAMETER_UPP((userRoutine), uppWordBreakProcInfo, (text), (charPos))
#endif

enum {
																/* feature bit 4 for TEFeatureFlag no longer in use */
	teFUseTextServices			= 4								/*00010000b */
};


#if OLDROUTINENAMES
/* action for the old C "bit (un)set" interface, TEFeatureFlag */
#define TEBitClear	teBitClear
#define TEBitSet 	teBitSet
#define TEBitTest	teBitTest
#define teFAutoScr	teFAutoScroll
#define toglBit		toggleBit
#endif  /* OLDROUTINENAMES */

EXTERN_API( Handle )
TEScrapHandle					(void)														TWOWORDINLINE(0x2EB8, 0x0AB4);

#if TARGET_CPU_PPC
EXTERN_API( long )
TEGetScrapLength				(void);

#else
#define TEGetScrapLength() ((long) * (unsigned short *) 0x0AB0)
#endif  /* TARGET_CPU_PPC */

EXTERN_API( void )
TEInit							(void)														ONEWORDINLINE(0xA9CC);

EXTERN_API( TEHandle )
TENew							(const Rect *			destRect,
								 const Rect *			viewRect)							ONEWORDINLINE(0xA9D2);

EXTERN_API( void )
TEDispose						(TEHandle 				hTE)								ONEWORDINLINE(0xA9CD);

EXTERN_API( void )
TESetText						(const void *			text,
								 long 					length,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9CF);

EXTERN_API( CharsHandle )
TEGetText						(TEHandle 				hTE)								ONEWORDINLINE(0xA9CB);

EXTERN_API( void )
TEIdle							(TEHandle 				hTE)								ONEWORDINLINE(0xA9DA);

EXTERN_API( void )
TESetSelect						(long 					selStart,
								 long 					selEnd,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9D1);

EXTERN_API( void )
TEActivate						(TEHandle 				hTE)								ONEWORDINLINE(0xA9D8);

EXTERN_API( void )
TEDeactivate					(TEHandle 				hTE)								ONEWORDINLINE(0xA9D9);

EXTERN_API( void )
TEKey							(CharParameter 			key,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9DC);

EXTERN_API( void )
TECut							(TEHandle 				hTE)								ONEWORDINLINE(0xA9D6);

EXTERN_API( void )
TECopy							(TEHandle 				hTE)								ONEWORDINLINE(0xA9D5);

EXTERN_API( void )
TEPaste							(TEHandle 				hTE)								ONEWORDINLINE(0xA9DB);

EXTERN_API( void )
TEDelete						(TEHandle 				hTE)								ONEWORDINLINE(0xA9D7);

EXTERN_API( void )
TEInsert						(const void *			text,
								 long 					length,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9DE);

EXTERN_API( void )
TESetAlignment					(short 					just,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9DF);

EXTERN_API( void )
TEUpdate						(const Rect *			rUpdate,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9D3);

EXTERN_API( void )
TETextBox						(const void *			text,
								 long 					length,
								 const Rect *			box,
								 short 					just)								ONEWORDINLINE(0xA9CE);

EXTERN_API( void )
TEScroll						(short 					dh,
								 short 					dv,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA9DD);

EXTERN_API( void )
TESelView						(TEHandle 				hTE)								ONEWORDINLINE(0xA811);

EXTERN_API( void )
TEPinScroll						(short 					dh,
								 short 					dv,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA812);

EXTERN_API( void )
TEAutoView						(Boolean 				fAuto,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA813);

EXTERN_API( void )
TECalText						(TEHandle 				hTE)								ONEWORDINLINE(0xA9D0);

EXTERN_API( short )
TEGetOffset						(Point 					pt,
								 TEHandle 				hTE)								ONEWORDINLINE(0xA83C);

EXTERN_API( Point )
TEGetPoint						(short 					offset,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0008, 0xA83D);

EXTERN_API( void )
TEClick							(Point 					pt,
								 Boolean 				fExtend,
								 TEHandle 				h)									ONEWORDINLINE(0xA9D4);

EXTERN_API( TEHandle )
TEStyleNew						(const Rect *			destRect,
								 const Rect *			viewRect)							ONEWORDINLINE(0xA83E);

EXTERN_API( void )
TESetStyleHandle				(TEStyleHandle 			theHandle,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0005, 0xA83D);

EXTERN_API( TEStyleHandle )
TEGetStyleHandle				(TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0004, 0xA83D);

EXTERN_API( void )
TEGetStyle						(short 					offset,
								 TextStyle *			theStyle,
								 short *				lineHeight,
								 short *				fontAscent,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0003, 0xA83D);

EXTERN_API( void )
TEStylePaste					(TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0000, 0xA83D);

EXTERN_API( void )
TESetStyle						(short 					mode,
								 const TextStyle *		newStyle,
								 Boolean 				fRedraw,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0001, 0xA83D);

EXTERN_API( void )
TEReplaceStyle					(short 					mode,
								 const TextStyle *		oldStyle,
								 const TextStyle *		newStyle,
								 Boolean 				fRedraw,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0002, 0xA83D);

EXTERN_API( StScrpHandle )
TEGetStyleScrapHandle			(TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0006, 0xA83D);

EXTERN_API( void )
TEStyleInsert					(const void *			text,
								 long 					length,
								 StScrpHandle 			hST,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0007, 0xA83D);

EXTERN_API( long )
TEGetHeight						(long 					endLine,
								 long 					startLine,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x0009, 0xA83D);

EXTERN_API( Boolean )
TEContinuousStyle				(short *				mode,
								 TextStyle *			aStyle,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x000A, 0xA83D);

EXTERN_API( void )
TEUseStyleScrap					(long 					rangeStart,
								 long 					rangeEnd,
								 StScrpHandle 			newStyles,
								 Boolean 				fRedraw,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x000B, 0xA83D);

EXTERN_API( void )
TECustomHook					(TEIntHook 				which,
								 UniversalProcPtr *		addr,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x000C, 0xA83D);

EXTERN_API( long )
TENumStyles						(long 					rangeStart,
								 long 					rangeEnd,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x000D, 0xA83D);

EXTERN_API( short )
TEFeatureFlag					(short 					feature,
								 short 					action,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x000E, 0xA83D);

EXTERN_API( OSErr )
TEGetHiliteRgn					(RgnHandle 				region,
								 TEHandle 				hTE)								THREEWORDINLINE(0x3F3C, 0x000F, 0xA83D);

EXTERN_API( void )
TESetScrapLength				(long 					length);

EXTERN_API( OSErr )
TEFromScrap						(void);

EXTERN_API( OSErr )
TEToScrap						(void);

EXTERN_API( void )
TESetClickLoop					(TEClickLoopUPP 		clikProc,
								 TEHandle 				hTE);

EXTERN_API( void )
TESetWordBreak					(WordBreakUPP 			wBrkProc,
								 TEHandle 				hTE);


#if CGLUESUPPORTED
EXTERN_API_C( void )
teclick							(Point *				pt,
								 Boolean 				fExtend,
								 TEHandle 				h);

#endif  /* CGLUESUPPORTED */

#if OLDROUTINENAMES
#define TESetJust(just, hTE) TESetAlignment(just, hTE)
#define TextBox(text, length, box, just) TETextBox(text, length, box, just)
#define TEStylNew(destRect, viewRect) TEStyleNew(destRect, viewRect)
#define SetStylHandle(theHandle, hTE) TESetStyleHandle(theHandle, hTE)
#define SetStyleHandle(theHandle, hTE) TESetStyleHandle (theHandle, hTE)
#define GetStylHandle(hTE) TEGetStyleHandle(hTE)
#define GetStyleHandle(hTE) TEGetStyleHandle(hTE)
#define TEStylPaste(hTE) TEStylePaste(hTE)
#define GetStylScrap(hTE) TEGetStyleScrapHandle(hTE)
#define GetStyleScrap(hTE) TEGetStyleScrapHandle(hTE)
#define SetStylScrap(rangeStart, rangeEnd, newStyles, redraw, hTE) TEUseStyleScrap(rangeStart, rangeEnd, newStyles, redraw, hTE)
#define SetStyleScrap(rangeStart, rangeEnd, newStyles, redraw, hTE)  TEUseStyleScrap(rangeStart, rangeEnd, newStyles, redraw, hTE)
#define TEStylInsert(text, length, hST, hTE) TEStyleInsert(text, length, hST, hTE)
#define TESetScrapLen(length) TESetScrapLength(length)
#define TEGetScrapLen() TEGetScrapLength()
#define SetClikLoop(clikProc, hTE) TESetClickLoop(clikProc, hTE)
#define SetWordBreak(wBrkProc, hTE) TESetWordBreak(wBrkProc, hTE)
#endif  /* OLDROUTINENAMES */


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

#endif /* __TEXTEDIT__ */

