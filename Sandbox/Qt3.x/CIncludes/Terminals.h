/*
 	File:		Terminals.h
 
 	Contains:	Communications Toolbox Terminal tool Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1998, 1995-1997 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __TERMINALS__
#define __TERMINALS__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
#ifndef __CTBUTILITIES__
#include <CTBUtilities.h>
#endif
#ifndef __CONNECTIONS__
#include <Connections.h>
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
	curTMVersion				= 2								/* current Terminal Manager version */
};


enum {
	curTermEnvRecVers			= 0								/* current Terminal Manager Environment Record version */
};


enum {
																/* error codes    */
	tmGenericError				= -1,
	tmNoErr						= 0,
	tmNotSent					= 1,
	tmEnvironsChanged			= 2,
	tmNotSupported				= 7,
	tmNoTools					= 8
};

typedef OSErr 							TMErr;

enum {
	tmInvisible					= 1 << 0,
	tmSaveBeforeClear			= 1 << 1,
	tmNoMenus					= 1 << 2,
	tmAutoScroll				= 1 << 3,
	tmConfigChanged				= 1 << 4
};

typedef unsigned long 					TMFlags;

enum {
	selTextNormal				= 1 << 0,
	selTextBoxed				= 1 << 1,
	selGraphicsMarquee			= 1 << 2,
	selGraphicsLasso			= 1 << 3,
	tmSearchNoDiacrit			= 1 << 8,
	tmSearchNoCase				= 1 << 9
};

typedef unsigned short 					TMSearchTypes;
typedef short 							TMSelTypes;

enum {
	cursorText					= 1,
	cursorGraphics				= 2
};

typedef unsigned short 					TMCursorTypes;

enum {
	tmTextTerminal				= 1 << 0,
	tmGraphicsTerminal			= 1 << 1
};

typedef unsigned short 					TMTermTypes;

struct TermDataBlock {
	TMTermTypes 					flags;
	Handle 							theData;
	Handle 							auxData;
	long 							reserved;
};
typedef struct TermDataBlock			TermDataBlock;
typedef TermDataBlock *					TermDataBlockPtr;
typedef TermDataBlockPtr *				TermDataBlockH;
typedef TermDataBlockPtr *				TermDataBlockHandle;

struct TermEnvironRec {
	short 							version;
	TMTermTypes 					termType;
	short 							textRows;
	short 							textCols;
	Point 							cellSize;
	Rect 							graphicSize;
	Point 							slop;
	Rect 							auxSpace;
};
typedef struct TermEnvironRec			TermEnvironRec;

typedef TermEnvironRec *				TermEnvironPtr;

union TMSelection {
	Rect 							selRect;
	RgnHandle 						selRgnHandle;
};
typedef union TMSelection				TMSelection;
typedef struct TermRecord 				TermRecord;

typedef TermRecord *					TermPtr;
typedef TermPtr *						TermHandle;
typedef CALLBACK_API( long , TerminalSendProcPtr )(Ptr thePtr, long theSize, long refCon, CMFlags flags);
typedef CALLBACK_API( void , TerminalBreakProcPtr )(long duration, long refCon);
typedef CALLBACK_API( long , TerminalCacheProcPtr )(long refCon, TermDataBlockPtr theTermData);
typedef CALLBACK_API( void , TerminalSearchCallBackProcPtr )(TermHandle hTerm, short refNum, Rect *foundRect);
typedef CALLBACK_API( Boolean , TerminalClikLoopProcPtr )(long refCon);
typedef CALLBACK_API( CMErr , TerminalEnvironsProcPtr )(long refCon, ConnEnvironRec *theEnvirons);
typedef CALLBACK_API( void , TerminalChooseIdleProcPtr )(void );
typedef CALLBACK_API( long , TerminalToolDefProcPtr )(TermHandle hTerm, short msg, long p1, long p2, long p3);
typedef STACK_UPP_TYPE(TerminalSendProcPtr) 					TerminalSendUPP;
typedef STACK_UPP_TYPE(TerminalBreakProcPtr) 					TerminalBreakUPP;
typedef STACK_UPP_TYPE(TerminalCacheProcPtr) 					TerminalCacheUPP;
typedef STACK_UPP_TYPE(TerminalSearchCallBackProcPtr) 			TerminalSearchCallBackUPP;
typedef STACK_UPP_TYPE(TerminalClikLoopProcPtr) 				TerminalClikLoopUPP;
typedef STACK_UPP_TYPE(TerminalEnvironsProcPtr) 				TerminalEnvironsUPP;
typedef STACK_UPP_TYPE(TerminalChooseIdleProcPtr) 				TerminalChooseIdleUPP;
typedef STACK_UPP_TYPE(TerminalToolDefProcPtr) 					TerminalToolDefUPP;
/*    TMTermTypes     */

struct TermRecord {
	short 							procID;
	TMFlags 						flags;
	TMErr 							errCode;
	long 							refCon;
	long 							userData;
	TerminalToolDefUPP 				defProc;
	Ptr 							config;
	Ptr 							oldConfig;
	TerminalEnvironsUPP 			environsProc;
	long 							reserved1;
	long 							reserved2;
	Ptr 							tmPrivate;
	TerminalSendUPP 				sendProc;
	TerminalBreakUPP 				breakProc;
	TerminalCacheUPP 				cacheProc;
	TerminalClikLoopUPP 			clikLoop;
	WindowPtr 						owner;
	Rect 							termRect;
	Rect 							viewRect;
	Rect 							visRect;
	long 							lastIdle;
	TMSelection 					selection;
	TMSelTypes 						selType;
	long 							mluField;
};

enum { uppTerminalSendProcInfo = 0x00002FF0 }; 					/* pascal 4_bytes Func(4_bytes, 4_bytes, 4_bytes, 2_bytes) */
enum { uppTerminalBreakProcInfo = 0x000003C0 }; 				/* pascal no_return_value Func(4_bytes, 4_bytes) */
enum { uppTerminalCacheProcInfo = 0x000003F0 }; 				/* pascal 4_bytes Func(4_bytes, 4_bytes) */
enum { uppTerminalSearchCallBackProcInfo = 0x00000EC0 }; 		/* pascal no_return_value Func(4_bytes, 2_bytes, 4_bytes) */
enum { uppTerminalClikLoopProcInfo = 0x000000D0 }; 				/* pascal 1_byte Func(4_bytes) */
enum { uppTerminalEnvironsProcInfo = 0x000003E0 }; 				/* pascal 2_bytes Func(4_bytes, 4_bytes) */
enum { uppTerminalChooseIdleProcInfo = 0x00000000 }; 			/* pascal no_return_value Func() */
enum { uppTerminalToolDefProcInfo = 0x0000FEF0 }; 				/* pascal 4_bytes Func(4_bytes, 2_bytes, 4_bytes, 4_bytes, 4_bytes) */
#define NewTerminalSendProc(userRoutine) 						(TerminalSendUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalSendProcInfo, GetCurrentArchitecture())
#define NewTerminalBreakProc(userRoutine) 						(TerminalBreakUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalBreakProcInfo, GetCurrentArchitecture())
#define NewTerminalCacheProc(userRoutine) 						(TerminalCacheUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalCacheProcInfo, GetCurrentArchitecture())
#define NewTerminalSearchCallBackProc(userRoutine) 				(TerminalSearchCallBackUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalSearchCallBackProcInfo, GetCurrentArchitecture())
#define NewTerminalClikLoopProc(userRoutine) 					(TerminalClikLoopUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalClikLoopProcInfo, GetCurrentArchitecture())
#define NewTerminalEnvironsProc(userRoutine) 					(TerminalEnvironsUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalEnvironsProcInfo, GetCurrentArchitecture())
#define NewTerminalChooseIdleProc(userRoutine) 					(TerminalChooseIdleUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalChooseIdleProcInfo, GetCurrentArchitecture())
#define NewTerminalToolDefProc(userRoutine) 					(TerminalToolDefUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppTerminalToolDefProcInfo, GetCurrentArchitecture())
#define CallTerminalSendProc(userRoutine, thePtr, theSize, refCon, flags)  CALL_FOUR_PARAMETER_UPP((userRoutine), uppTerminalSendProcInfo, (thePtr), (theSize), (refCon), (flags))
#define CallTerminalBreakProc(userRoutine, duration, refCon) 	CALL_TWO_PARAMETER_UPP((userRoutine), uppTerminalBreakProcInfo, (duration), (refCon))
#define CallTerminalCacheProc(userRoutine, refCon, theTermData)  CALL_TWO_PARAMETER_UPP((userRoutine), uppTerminalCacheProcInfo, (refCon), (theTermData))
#define CallTerminalSearchCallBackProc(userRoutine, hTerm, refNum, foundRect)  CALL_THREE_PARAMETER_UPP((userRoutine), uppTerminalSearchCallBackProcInfo, (hTerm), (refNum), (foundRect))
#define CallTerminalClikLoopProc(userRoutine, refCon) 			CALL_ONE_PARAMETER_UPP((userRoutine), uppTerminalClikLoopProcInfo, (refCon))
#define CallTerminalEnvironsProc(userRoutine, refCon, theEnvirons)  CALL_TWO_PARAMETER_UPP((userRoutine), uppTerminalEnvironsProcInfo, (refCon), (theEnvirons))
#define CallTerminalChooseIdleProc(userRoutine) 				CALL_ZERO_PARAMETER_UPP((userRoutine), uppTerminalChooseIdleProcInfo)
#define CallTerminalToolDefProc(userRoutine, hTerm, msg, p1, p2, p3)  CALL_FIVE_PARAMETER_UPP((userRoutine), uppTerminalToolDefProcInfo, (hTerm), (msg), (p1), (p2), (p3))

EXTERN_API( TMErr )
InitTM							(void);

EXTERN_API( Handle )
TMGetVersion					(TermHandle 			hTerm);

EXTERN_API( short )
TMGetTMVersion					(void);

EXTERN_API( TermHandle )
TMNew							(const Rect *			termRect,
								 const Rect *			viewRect,
								 TMFlags 				flags,
								 short 					procID,
								 WindowPtr 				owner,
								 TerminalSendUPP 		sendProc,
								 TerminalCacheUPP 		cacheProc,
								 TerminalBreakUPP 		breakProc,
								 TerminalClikLoopUPP 	clikLoop,
								 TerminalEnvironsUPP 	environsProc,
								 long 					refCon,
								 long 					userData);

EXTERN_API( void )
TMDispose						(TermHandle 			hTerm);

EXTERN_API( void )
TMKey							(TermHandle 			hTerm,
								 const EventRecord *	theEvent);

EXTERN_API( void )
TMUpdate						(TermHandle 			hTerm,
								 RgnHandle 				visRgn);

EXTERN_API( void )
TMPaint							(TermHandle 			hTerm,
								 const TermDataBlock *	theTermData,
								 const Rect *			theRect);

EXTERN_API( void )
TMActivate						(TermHandle 			hTerm,
								 Boolean 				activate);

EXTERN_API( void )
TMResume						(TermHandle 			hTerm,
								 Boolean 				resume);

EXTERN_API( void )
TMClick							(TermHandle 			hTerm,
								 const EventRecord *	theEvent);

EXTERN_API( void )
TMIdle							(TermHandle 			hTerm);

EXTERN_API( long )
TMStream						(TermHandle 			hTerm,
								 void *					theBuffer,
								 long 					theLength,
								 CMFlags 				flags);

EXTERN_API( Boolean )
TMMenu							(TermHandle 			hTerm,
								 short 					menuID,
								 short 					item);

EXTERN_API( void )
TMReset							(TermHandle 			hTerm);

EXTERN_API( void )
TMClear							(TermHandle 			hTerm);

EXTERN_API( void )
TMResize						(TermHandle 			hTerm,
								 const Rect *			newViewRect);

EXTERN_API( long )
TMGetSelect						(TermHandle 			hTerm,
								 Handle 				theData,
								 ResType *				theType);

EXTERN_API( void )
TMGetLine						(TermHandle 			hTerm,
								 short 					lineNo,
								 TermDataBlock *		theTermData);

EXTERN_API( void )
TMSetSelection					(TermHandle 			hTerm,
								 const TMSelection *	theSelection,
								 TMSelTypes 			selType);

EXTERN_API( void )
TMScroll						(TermHandle 			hTerm,
								 short 					dh,
								 short 					dv);

EXTERN_API( Boolean )
TMValidate						(TermHandle 			hTerm);

EXTERN_API( void )
TMDefault						(Ptr *					theConfig,
								 short 					procID,
								 Boolean 				allocate);

EXTERN_API( Handle )
TMSetupPreflight				(short 					procID,
								 long *					magicCookie);

EXTERN_API( void )
TMSetupSetup					(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 long *					magicCookie);

EXTERN_API( Boolean )
TMSetupFilter					(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 EventRecord *			theEvent,
								 short *				theItem,
								 long *					magicCookie);

EXTERN_API( void )
TMSetupItem						(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 short *				theItem,
								 long *					magicCookie);

EXTERN_API( void )
TMSetupXCleanup					(short 					procID,
								 const void *			theConfig,
								 short 					count,
								 DialogPtr 				theDialog,
								 Boolean 				OKed,
								 long *					magicCookie);

EXTERN_API( void )
TMSetupPostflight				(short 					procID);

EXTERN_API( Ptr )
TMGetConfig						(TermHandle 			hTerm);

EXTERN_API( short )
TMSetConfig						(TermHandle 			hTerm,
								 const void *			thePtr);

EXTERN_API( OSErr )
TMIntlToEnglish					(TermHandle 			hTerm,
								 const void *			inputPtr,
								 Ptr *					outputPtr,
								 short 					language);

EXTERN_API( OSErr )
TMEnglishToIntl					(TermHandle 			hTerm,
								 const void *			inputPtr,
								 Ptr *					outputPtr,
								 short 					language);

EXTERN_API( void )
TMGetToolName					(short 					id,
								 Str255 				name);

EXTERN_API( short )
TMGetProcID						(ConstStr255Param 		name);

EXTERN_API( void )
TMSetRefCon						(TermHandle 			hTerm,
								 long 					refCon);

EXTERN_API( long )
TMGetRefCon						(TermHandle 			hTerm);

EXTERN_API( void )
TMSetUserData					(TermHandle 			hTerm,
								 long 					userData);

EXTERN_API( long )
TMGetUserData					(TermHandle 			hTerm);

EXTERN_API( short )
TMAddSearch						(TermHandle 			hTerm,
								 ConstStr255Param 		theString,
								 const Rect *			where,
								 TMSearchTypes 			searchType,
								 TerminalSearchCallBackUPP  callBack);

EXTERN_API( void )
TMRemoveSearch					(TermHandle 			hTerm,
								 short 					refnum);

EXTERN_API( void )
TMClearSearch					(TermHandle 			hTerm);

EXTERN_API( Point )
TMGetCursor						(TermHandle 			hTerm,
								 TMCursorTypes 			cursType);

EXTERN_API( TMErr )
TMGetTermEnvirons				(TermHandle 			hTerm,
								 TermEnvironRec *		theEnvirons);

EXTERN_API( short )
TMChoose						(TermHandle *			hTerm,
								 Point 					where,
								 TerminalChooseIdleUPP 	idleProc);

EXTERN_API( void )
TMEvent							(TermHandle 			hTerm,
								 const EventRecord *	theEvent);

EXTERN_API( Boolean )
TMDoTermKey						(TermHandle 			hTerm,
								 ConstStr255Param 		theKey);

EXTERN_API( short )
TMCountTermKeys					(TermHandle 			hTerm);

EXTERN_API( void )
TMGetIndTermKey					(TermHandle 			hTerm,
								 short 					id,
								 Str255 				theKey);

EXTERN_API( void )
TMGetErrorString				(TermHandle 			hTerm,
								 short 					id,
								 Str255 				errMsg);


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

#endif /* __TERMINALS__ */

