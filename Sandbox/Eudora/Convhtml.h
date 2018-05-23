#ifndef __CONVHTML_H__
#define __CONVHTML_H__

/*==================================================================*/
/* CONVHTML.H - HTML Conversion Routine                             */
/* ------------------------------------                             */
/* Version 1.0 Rev 7/14/97                                          */
/* Author V. Leamons for Qualcomm, Inc. All Rights Reserved.        */
/*                                                                  */
/* Description:                                                     */
/*    This file contains the ConvertURIs() routine to be included in */
/* Eudora 4.0.                                                      */
/*                                                                  */
/* History (all by V. Leamons unless otherwise marked):             */
/* 7/14/97  Created.                                                */
/*==================================================================*/

// *** Prototypes ***
int ConvertURIs(char * pszIn, char ** ppszOut);


// used to interpret "Embedded content" lines within messages
class URIMap
{
public:
	URIMap();
	~URIMap();

	int BuildURIMap( const char * pMessage );	// must include "Embedded content" lines
	int ResolveURI( const char * pszInURI, char * pszOutFilename, int nBufSize );
	void FreeURIMap( void );

	void NukeCIDs( void );
	BOOL IsRealCID( CString TargetCID );

	// used when creating messages - generates a CID based on a messageid
	int AddCIDMap( const char * pInMessageId, const char * pInPath, 
				   char * pszOutCID, int nCIDSize );

	// used when creating messages
	void BuildEmbeddedObjectHeaders( CString & Header );
	void GetEmbeddedObjectPaths( CString & Paths );

protected:
	CObList m_EmbeddedElements;
};

#endif __CONVHTML_H__

