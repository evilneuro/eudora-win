// CONVHTML.H
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//


/*==================================================================*/
/* CONVHTML.H - HTML Conversion Routine                             */
/* ------------------------------------                             */
/* Version 1.0 Rev 7/14/97                                          */
/* Author V. Leamons                                                */
/*                                                                  */
/* Description:                                                     */
/*    This file contains the ConvertURIs() routine to be included in */
/* Eudora 4.0.                                                      */
/*                                                                  */
/* History (all by V. Leamons unless otherwise marked):             */
/* 7/14/97  Created.                                                */
/*==================================================================*/

#ifndef __CONVHTML_H__
#define __CONVHTML_H__

class URIMap;

// *** Prototypes ***
int ConvertURIs(const char * pszIn, char ** ppszOut, const URIMap & uriMap);


// used to interpret "Embedded content" lines within messages
class URIMap
{
public:
	URIMap();
	~URIMap();

	int BuildURIMap( const char * pMessage, bool bFreePrevious = true );	// must include "Embedded content" lines

	void			ResolvePathToFullPath(
							CString &		szPath) const;
	
	int ResolveURIToPath( const char * pszInURI, char * pszOutFilename, int nBufSize, bool bValidateFile = true ) const;
	int ResolveURIToFileURL( const char * pszInURI, char * pszOutFilename, int nBufSize ) const;


	void			AddOrphanedURIsToBodyAsAttachments(
							CString &		szBody);

	void FreeURIMap( void );
	void TransferMembers(URIMap & dest);

	void NukeCIDs( void );
	BOOL IsRealCID( const char * TargetCID );

	// used when creating messages - generates a CID based on a messageid
	int							AddCIDMap(
									const char *			pInMessageId,
									const char *			pInPath, 
									char *					pszOutCID,
									int						nCIDSize,
									bool					bIsEmoticon);

	void RemoveCIDMap( const char* cid );

	// used when creating messages
	void BuildEmbeddedObjectHeaders(CString & Header, bool bIncludeEmoticons);
	void GetEmbeddedObjectPaths( CString & Paths );

protected:
	CObList m_EmbeddedElements;
};

#endif __CONVHTML_H__
