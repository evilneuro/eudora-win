/*
 	File:		Devices.r
 
 	Contains:	Device Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __DEVICES_R__
#define __DEVICES_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

/*----------------------------DRVR ¥ Driver---------------------------------------------*/
type 'DRVR' {
		boolean = 0;
		boolean			dontNeedLock, needLock;					/* lock drvr in memory	*/
		boolean			dontNeedTime, needTime;					/* for periodic	action	*/
		boolean			dontNeedGoodbye, needGoodbye;			/* call before heap reinit*/
		boolean			noStatusEnable, statusEnable;			/* responds to status	*/
		boolean			noCtlEnable, ctlEnable;					/* responds to control	*/
		boolean			noWriteEnable, writeEnable;				/* responds to write	*/
		boolean			noReadEnable, readEnable;				/* responds to read		*/
		byte = 0;
		integer;												/* driver delay			*/
		unsigned hex integer;									/* desk acc event mask	*/
		integer;												/* driver menu			*/
		unsigned hex integer;									/* offset to open		*/
		unsigned hex integer;									/* offset to prime		*/
		unsigned hex integer;									/* offset to control	*/
		unsigned hex integer;									/* offset to status		*/
		unsigned hex integer;									/* offset to close		*/
		pstring;												/* driver name			*/
		hex string;												/* driver code			*/
};

#endif /* __DEVICES_R__ */

