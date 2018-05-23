/*
 	File:		Palettes.r
 
 	Contains:	Palette Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1987-1993, 1995-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __PALETTES_R__
#define __PALETTES_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

/*----------------------------pltt ¥ Color Palette--------------------------------------*/
type 'pltt' {
		integer = $$CountOf(ColorInfo); 						/* Color table count	*/
		fill long;												/* Reserved				*/
		fill word;												/* Reserved				*/
		fill long;												/* Reserved				*/
		fill long;												/* Reserved				*/
		wide array ColorInfo {
			unsigned integer;									/* RGB: red				*/
			unsigned integer;									/*		green			*/
			unsigned integer;									/* 		blue			*/
			integer		pmCourteous, pmDithered, pmTolerant,	/* Color usage			*/
						pmAnimated = 4, pmExplicit = 8;
			integer;											/* Tolerance value		*/
			fill word;											/* Private flags		*/
			fill long;											/* Private				*/
		};
};

#endif /* __PALETTES_R__ */

