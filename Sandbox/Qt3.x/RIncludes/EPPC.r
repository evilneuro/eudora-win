/*
 	File:		EPPC.r
 
 	Contains:	High Level Event Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1988-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __EPPC_R__
#define __EPPC_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif


/*----------------------------------------eppc -----------------------------------------*/
type 'eppc' {
	unsigned longint;	/* flags word	*/
	unsigned longint;	/* reserved 	*/
	unsigned integer;	/* scriptCode	*/
	pstring[32];
};

#endif /* __EPPC_R__ */

