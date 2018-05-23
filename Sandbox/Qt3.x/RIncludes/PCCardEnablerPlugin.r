/*
 	File:		PCCardEnablerPlugin.r
 
 	Contains:	???
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1997-1998 by Apple Computer, Inc. and SystemSoft Corporation.  All rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __PCCARDENABLERPLUGIN_R__
#define __PCCARDENABLERPLUGIN_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#ifndef __CARDSERVICES__
type 'pccd' {
   longint;										/* MUST BE ZERO */
	integer	dontShowIcon = -1, noCustomIcon = 0;	/* customIconID				*/
	integer	noCustomStrings = 0;					/* customStringsID			*/
	unsigned integer;								/*	customTypeIndex			*/
	unsigned integer;								/*	customHelpIndex			*/
	literal longint	noCustomAction = 0;				/* customAction				*/
	longint;										/*	customActionParam1		*/
	longint;										/*	customActionParam2		*/
};
#endif

#endif /* __PCCARDENABLERPLUGIN_R__ */

