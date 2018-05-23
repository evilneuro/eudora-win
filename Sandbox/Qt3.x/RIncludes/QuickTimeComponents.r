/*
 	File:		QuickTimeComponents.r
 
 	Contains:	QuickTime Interfaces.
 
 	Version:	Technology:	QuickTime 3.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __QUICKTIMECOMPONENTS_R__
#define __QUICKTIMECOMPONENTS_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define canMovieImportHandles 			0x01
#define canMovieImportFiles 			0x02
#define hasMovieImportUserInterface 	0x04
#define canMovieExportHandles 			0x08
#define canMovieExportFiles 			0x10
#define hasMovieExportUserInterface 	0x20
#define dontAutoFileMovieImport 		0x40
#define canMovieExportAuxDataHandle 	0x80
#define canMovieImportValidateHandles 	0x0100
#define canMovieImportValidateFile 		0x0200
#define dontRegisterWithEasyOpen 		0x0400
#define canMovieImportInPlace 			0x0800
#define movieImportSubTypeIsFileExtension  0x1000
#define canMovieImportPartial 			0x2000
#define hasMovieImportMIMEList 			0x4000
#define canMovieExportFromProcedures 	0x8000
#define canMovieExportValidateMovie 	0x00010000
#define movieExportNeedsResourceFork 	0x00020000
#define canMovieImportDataReferences 	0x00040000
#define movieExportMustGetSourceMediaType  0x00080000
#define reservedForUseByGraphicsImporters  0x00800000

#define kMimeInfoMimeTypeTag 			'mime'
#define kMimeInfoFileExtensionTag 		'ext '
#define kMimeInfoDescriptionTag 		'desc'
#define kMimeInfoGroupTag 				'grop'
#define kMimeInfoDoNotOverrideExistingFileTypeAssociation  'nofa'


type 'mime' {
	// 10 bytes of reserved
	longint = 0;
	longint = 0;
	integer = 0;
	// 2 bytes of lock count
	integer = 0;
	
	// size of this atom
	parentStart:
	longint = ( (parentEnd - parentStart) / 8 );
	
	// atom type
	literal longint = 'sean';
	
	// atom id
	longint = 1;
	integer = 0;
	integer =  $$CountOf(AtomArray);
	longint = 0;
	
	array AtomArray {
		atomStart:
		// size of this atom
		longint = ((atomEnd[$$ArrayIndex(AtomArray)] - atomStart[$$ArrayIndex(AtomArray)]) / 8);
		
		// atom type
		literal longint;
		
		// atom id
		longint;
		integer = 0;
		integer = 0; // no children
		longint = 0;
		string;
		atomEnd:
		};
	parentEnd:
		
};


#endif /* __QUICKTIMECOMPONENTS_R__ */

