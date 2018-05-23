// MimeStorage.h
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


#ifndef __MIMESTORAGE_H__
#define __MIMESTORAGE_H__

#include "QCUtils.h"
#include "Convhtml.h"
#include "ContentConcentrator.h"

typedef CString			PART_ID;
typedef INT32			RESULT;
#define SUCCESS			0
#define ERR_BAD_CREATE	-1
#define ERR_BAD_OPEN	-2
#define ERR_BAD_READ	-3
#define ERR_BAD_WRITE	-4
#define ERR_BAD_CLOSE	-5
#define ERR_BAD_DELETE  -6
#define ERR_BAD_NEW		-7
#define ERR_NOT_INIT	-8
#define ERR_NOT_CID		-9

class CMessageDoc;
class QCMimePart;




class QCMessage : public CObject
{
public:	
	QCMessage(CMessageDoc* ParentDoc);
	~QCMessage();

	RESULT Init( const char * messageid, const char * buf, BOOL HasHeaders = TRUE );
	RESULT InitMap( const char * ECHeaders );
	void   InitMap(URIMap & in_sourceMap);
	void   TransferMap(QCMessage & dest);
	BOOL   IsInit(){ return m_theMess != NULL; }
	RESULT Flush();
	RESULT NukeCIDs();
	RESULT IsRealCID( const char * CID );

	RESULT GetRawMessage( CString & Message );
	RESULT GetFullMessage( CString & Message );
	RESULT GetHeaders( CString & Headers );
	RESULT GetBody( CString & Body );
	RESULT GetMessageForDisplay(
					ContentConcentrator::ContextT		in_context,
					CString &							out_szBody,
					CString *							out_pszHeaders = NULL,
					bool *								out_bWasConcentrated = NULL,
					bool								in_bAlwaysStripHTMLCode = false,
					bool								in_bStripDocumentLevelTags = false,
					bool								in_bRelaxLocalFileRefStripping = false,
					bool								in_bMorphHTML = true );
	
	//RESULT GetAttachments( CString & Attachments );

	//more useful if it tells how many attahcments it got.
	int GetAttachments( CString & Attachments );
	RESULT GetPart( PART_ID id, QCMimePart ** ppPart );	// where id is a URI
	RESULT GetPartAsFile( PART_ID id, char* pPath, int pathLen, bool bValidateFile = true );

	// use when creating messages
	RESULT				AddPart(
							const char *		path,				// use an existing file
							QCMimePart **		ppPart,
							bool				bIsEmoticon = false);

	RESULT NewPart( const char * extension,			// create a new file with this extension
                    QCMimePart ** ppPart );

	// when deleting embedded objects from a message
	RESULT RemovePart( PART_ID pid );

	RESULT GetEmbeddedObjectHeaders(CString & Headers, bool bIncludeEmoticons = true);
	RESULT GetEmbeddedObjectPaths( CString & Paths );

protected:
	CMessageDoc* m_ParentDoc;
	CString		m_MessageId;
	UINT		m_CIDReference;
	char *		m_theMess;
	URIMap		m_theMap;
	BOOL		m_HasHeaders;
};

class QCMimePart : public CObject
{
public:
	QCMimePart( const char * cid, const char * path, int mode );

	// use to get at an embedded object
	RESULT GetCID( CString& csCID );

	RESULT Seek( INT32 offset, INT32 mode = SEEK_SET );
	RESULT Tell( INT32 * pOffset );

	RESULT Read( CHAR * buf, INT32 bufsize, INT32 * pRead );

	RESULT Get( CHAR * value );
	RESULT Get( INT32 * value );
	RESULT Get( UINT32 * value );
	RESULT Get( CHAR * str, INT32 bufsize );
	RESULT GetLine( CHAR * str, INT32 bufsize );

	// use when creating an embedded part
	RESULT Write( CHAR * but, INT32 bufsize, INT32 * pWritten );

	RESULT Put( CHAR value );
	RESULT Put( INT32 value );
	RESULT Put( UINT32 value );
	RESULT Put( const void * str = NULL, INT32 len = -1L );
	RESULT PutLine( const void * str = NULL, INT32 len = -1L );

	RESULT Close();
	RESULT Delete(bool bDeleteAny = false);

protected:
	CString		m_csCID;
	JJFileMT	m_jjFile;		// object to access the local storage
};

#endif // __MIMESTORAGE_H__
