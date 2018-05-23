/*
 	File:		TerminalTools.h
 
 	Contains:	Communications Toolbox Terminal tools Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1998, 1995-1997 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __TERMINALTOOLS__
#define __TERMINALTOOLS__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
#ifndef __TERMINALS__
#include <Terminals.h>
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
	tdefType					= FOUR_CHAR_CODE('tdef'),
	tvalType					= FOUR_CHAR_CODE('tval'),
	tsetType					= FOUR_CHAR_CODE('tset'),
	tlocType					= FOUR_CHAR_CODE('tloc'),
	tscrType					= FOUR_CHAR_CODE('tscr'),
	tbndType					= FOUR_CHAR_CODE('tbnd'),
	tverType					= FOUR_CHAR_CODE('vers')
};


enum {
																/* messages */
	tmInitMsg					= 0,
	tmDisposeMsg				= 1,
	tmSuspendMsg				= 2,
	tmResumeMsg					= 3,
	tmMenuMsg					= 4,
	tmEventMsg					= 5,
	tmActivateMsg				= 6,
	tmDeactivateMsg				= 7,
	tmGetErrorStringMsg			= 8,
	tmIdleMsg					= 50,
	tmResetMsg					= 51,
	tmKeyMsg					= 100,
	tmStreamMsg					= 101,
	tmResizeMsg					= 102,
	tmUpdateMsg					= 103,
	tmClickMsg					= 104,
	tmGetSelectionMsg			= 105,
	tmSetSelectionMsg			= 106,
	tmScrollMsg					= 107,
	tmClearMsg					= 108,
	tmGetLineMsg				= 109,
	tmPaintMsg					= 110,
	tmCursorMsg					= 111,
	tmGetEnvironsMsg			= 112,
	tmDoTermKeyMsg				= 113,
	tmCountTermKeysMsg			= 114,
	tmGetIndTermKeyMsg			= 115
};


enum {
																/* messages for validate DefProc    */
	tmValidateMsg				= 0,
	tmDefaultMsg				= 1
};


enum {
																/* messages for Setup DefProc    */
	tmSpreflightMsg				= 0,
	tmSsetupMsg					= 1,
	tmSitemMsg					= 2,
	tmSfilterMsg				= 3,
	tmScleanupMsg				= 4
};


enum {
																/* messages for scripting defProc    */
	tmMgetMsg					= 0,
	tmMsetMsg					= 1
};


enum {
																/* messages for localization defProc  */
	tmL2English					= 0,
	tmL2Intl					= 1
};


struct TMSearchBlock {
	StringHandle 					theString;
	Rect 							where;
	TMSearchTypes 					searchType;
	TerminalSearchCallBackUPP 		callBack;
	short 							refnum;
	struct TMSearchBlock *			next;
};
typedef struct TMSearchBlock			TMSearchBlock;
typedef TMSearchBlock *					TMSearchBlockPtr;

struct TMSetupStruct {
	DialogPtr 						theDialog;
	short 							count;
	Ptr 							theConfig;
	short 							procID;						/* procID of the tool */
};
typedef struct TMSetupStruct			TMSetupStruct;

typedef TMSetupStruct *					TMSetupPtr;

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

#endif /* __TERMINALTOOLS__ */

