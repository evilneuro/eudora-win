/*
 	File:		DatabaseAccess.r
 
 	Contains:	Database Access Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1989-1998, 1995-1997 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __DATABASEACCESS_R__
#define __DATABASEACCESS_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif


/* types for the Database Access Manager */

/* 'wstr' - Word Length String Resource */
type 'wstr' {
		wstring;												/* string with word length spec. */
};

/* 'qrsc' - Query Resource */
type 'qrsc' {
		integer;												/* version */

		integer;												/* 'qdef' ID */

		integer;												/* STR# ID for ddevName, host,
											  						 user, password, connstr */

		integer;												/* current query */

		/* array of IDs of 'wstr' resources containing queries */
		integer = $$CountOf(QueryArray);						/* array size */
		wide array QueryArray {
			integer;											/* id of 'wstr' resource */
		};

		/* array of types and IDs for other resources for query */
		integer = $$CountOf(ResArray);							/* array size */
		wide array ResArray {
			literal longint;									/* resource type */
			integer;											/* resource ID */
		};
};

/* 'dflg' - ddev Flags */
type 'dflg' {
		longint;												/* version */

		unsigned bitstring[32]									/* ddev flags */
			asyncNotSupp, asyncSupp;
};

#endif /* __DATABASEACCESS_R__ */

