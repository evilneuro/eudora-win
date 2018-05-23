/*
 	File:		PPCToolbox.r
 
 	Contains:	Program-Program Communications Toolbox Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1989-1993, 1995-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __PPCTOOLBOX_R__
#define __PPCTOOLBOX_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

/*------------------------ppcc ¥ PPC Browser Configuration Resource---------------------------*/

	type 'ppcc' {
		unsigned byte;	// NBP lookup interval
		unsigned byte;	// NBP lookup count
		integer;		// NBP maximum lives an entry has before deletion
		integer;		// NBP maximum number of entities
		integer;		// NBP idle time in ticks between lookups
		integer;		// PPC maximum number of ports
		integer;		// PPC idle time in ticks between list ports
	};


#endif /* __PPCTOOLBOX_R__ */

