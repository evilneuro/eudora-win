// MimeStorage.cpp
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


#include "stdafx.h"

#include "etf2html.h"
#include "Eudora.h"
#include "Fileutil.h"
#include "msgutils.h"
#include "MimeStorage.h"
#include "ContentConcentrator.h"
#include "resource.h"
#include "rs.h"
#include "rpc.h"
#include "trnslate.h"
#include "Text2HTML.h"
#include "QCGraphics.h"
#include "utils.h"
#include "msgdoc.h"
#include "summary.h"
#include "tocdoc.h"


#include "DebugNewHelpers.h"



/*****
	QCMessage Implementation
*****/

QCMessage::QCMessage(CMessageDoc* ParentDoc)
{
	m_ParentDoc = ParentDoc;

	m_theMess = NULL;
	m_HasHeaders = TRUE;
}

QCMessage::~QCMessage()
{
	delete [] m_theMess;
}

RESULT QCMessage::Init( const char * messageid, const char * buf, BOOL HeadersIncluded )
{
	m_HasHeaders = HeadersIncluded;

	// used as seed for CID generation
	m_MessageId = messageid;

	// make a copy of the message
	delete [] m_theMess;
	m_theMess = DEBUG_NEW_NOTHROW char[ strlen( buf ) + 1 ];
	if ( ! m_theMess )
		return ERR_BAD_NEW;

#ifdef _DEBUG
	{
		char szName[126] = "QCMsg: ";
		char *sub = HeaderContents(IDS_HEADER_SUBJECT, buf);
		if(!sub)
			sub = HeaderContents(IDS_HEADER_FROM, buf);
		
		if(sub)
			strncpy(szName+strlen(szName), sub, 26);
		else
			strncpy(szName+strlen(szName), buf, 26);

		delete [] sub;
	}
#endif

	// save off the message text
	memcpy( m_theMess, buf, strlen( buf ) + 1 );

	// create a list of "Embedded Content"
	m_theMap.BuildURIMap( m_theMess );
	
	return SUCCESS;
}

RESULT QCMessage::InitMap( const char * ECHeaders )
{
	m_theMap.BuildURIMap( ECHeaders );

	return SUCCESS;
}

void QCMessage::InitMap(URIMap & in_sourceMap)
{
	m_theMap.FreeURIMap();
	in_sourceMap.TransferMembers(m_theMap);
	m_theMap.NukeCIDs();
}

//	Empty the dest message's map, then transfer all contents
//	of our map to dest.
void QCMessage::TransferMap(QCMessage & dest)
{
	dest.m_theMap.FreeURIMap();
	m_theMap.TransferMembers(dest.m_theMap);
}

RESULT QCMessage::Flush()
{
	delete [] m_theMess;
	m_theMess = NULL;

	m_MessageId.Empty();
	m_CIDReference = 0;
	m_HasHeaders = TRUE;
	m_theMap.FreeURIMap();

	return SUCCESS;
}

RESULT QCMessage::NukeCIDs()
{
	m_theMap.NukeCIDs();

	return SUCCESS;
}

RESULT QCMessage::IsRealCID( const char * CID )
{
	if( m_theMap.IsRealCID( CID ) )
		return SUCCESS;

	return ERR_NOT_CID;
}

// where id is a URI
RESULT QCMessage::GetPart( PART_ID id, QCMimePart ** ppPart )
{
	*ppPart = NULL;

	// lookup the part
	char path[ MAX_PATH ];
	m_theMap.ResolveURIToPath( id, path, sizeof( path ) );

	if ( *path )
	{
		*ppPart = DEBUG_NEW_MFCOBJ_NOTHROW QCMimePart( id, path, _O_RDONLY );

		if ( *ppPart )
			return SUCCESS;
	}

	return ERR_BAD_CREATE;
}


// where id is a URI, and pPath is filled with a full path to the "guts"
// of the part. ** this hack by Ben **
RESULT QCMessage::GetPartAsFile( PART_ID id, char* pPath, int pathLen, bool bValidateFile )
{
	// lookup the part (ResolveURI won't stuff all over pPath unless
	// it gets somethin' worth sniffin'.)
	if ( m_theMap.ResolveURIToPath( id, pPath, pathLen, bValidateFile ) )
		return SUCCESS;
	else
		return ERR_BAD_CREATE;
}


RESULT
QCMessage::AddPart(
	const char *			path,	// use an existing file
	QCMimePart **			ppPart,
	bool					bIsEmoticon)
{
	// create a URLMap entry/CID for the part
	char cid[ 256 ];
	if ( m_theMap.AddCIDMap(m_MessageId, path, cid, sizeof(cid), bIsEmoticon) )
	{
		*ppPart = DEBUG_NEW_MFCOBJ_NOTHROW QCMimePart( cid, (LPCTSTR)path, _O_RDWR | _O_EXCL );

		if ( *ppPart )
			return SUCCESS;
	}

	return ERR_BAD_CREATE;
}

RESULT QCMessage::RemovePart( PART_ID pid )
{
	RESULT r = ERR_BAD_DELETE;
	QCMimePart* p = 0;

	if ( SUCCEEDED(r = GetPart( pid, &p )) )
	{
		if ( SUCCEEDED(r = p->Delete()) )
		{
			// remove the cid map entry
			m_theMap.RemoveCIDMap( pid );
			delete p;
		}
	}

	return r;
}

RESULT QCMessage::NewPart( const char * extension,	// create a new file with this extension
                    QCMimePart ** ppPart )
{
	*ppPart = NULL;

	// create a temp file
	char EmbeddedDir[_MAX_PATH + 1];
	wsprintf(EmbeddedDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_EMBEDDED_FOLDER));

	// use an extension based on content-type
	char tmpPath[_MAX_PATH];			
	GetTmpFile( EmbeddedDir, extension, tmpPath );

	// create a URLMap entry/CID for the part
	char cid[ 256 ];
	if ( m_theMap.AddCIDMap(m_MessageId, tmpPath, cid, sizeof(cid), false) )
	{
		*ppPart = DEBUG_NEW_MFCOBJ_NOTHROW QCMimePart( cid, tmpPath, _O_CREAT | _O_TRUNC | _O_RDWR );

		if ( *ppPart )
			return SUCCESS;
	}

	return ERR_BAD_CREATE;
}


RESULT QCMessage::GetRawMessage( CString & Message )
{
	// return m_theMess including any X-EmbeddedContent headers
	if ( ! m_theMess )
		return ERR_NOT_INIT;

	Message = m_theMess;

	return SUCCESS;
}

RESULT QCMessage::GetFullMessage( CString & Message )
{
	CRString ECHeader(IDS_HEADER_EMBEDDED_CONTENT);
	BOOL	bInEC = FALSE;

	// Optimization so that only one large memory allocation
	// is done instead of lots of small ones
	Message.ReleaseBuffer(0);
	Message.GetBuffer(::SafeStrlenMT(m_theMess));

	// return m_theMess minus any X-EmbeddedContent headers
	if ( ! m_theMess )
		return ERR_NOT_INIT;

	char * pLine = m_theMess;
	char * NextLine;

	while (*pLine)
	{
		// prepare for the next line -  skip '\r\n'
		NextLine = strchr(pLine, '\r');
		if (!NextLine)
			NextLine = pLine + strlen(pLine);
		else
		{
			if (*++NextLine == '\n')
				NextLine++;
		}

		if ( strnicmp( ECHeader, pLine, ECHeader.GetLength()) == 0 )
			bInEC = TRUE;
		else if ( bInEC && strncmp( "  <", pLine, 3 ) != 0 )
			bInEC = FALSE;

		// Exit out if we hit the body
		if (*NextLine == '\r' || *NextLine == '\n')
		{
			Message += (bInEC? NextLine : pLine);
			break;
		}

		if ( ! bInEC )
		{
			// Little trick here of temporarily creating a null-terminated string
			// out of the line in order to save on an extra memory allocation
			char SaveChar = *NextLine;
			*NextLine = 0;
			Message += pLine;
			*NextLine = SaveChar;
		}

		pLine = NextLine;
	}

	return SUCCESS;
}

RESULT QCMessage::GetHeaders( CString & Headers )
{
	Headers = "";

	if ( ! m_theMess )
		return ERR_NOT_INIT;

	const char * pBody = ::FindBody( m_theMess );
		
	if ( pBody )
	{
		Headers = CString( m_theMess, pBody - m_theMess );
	}
	else
	{
		Headers = m_theMess;
	}

	return SUCCESS;
}

RESULT QCMessage::GetBody( CString & Body )
{
	Body = "";

	if ( ! m_theMess )
		return ERR_NOT_INIT;

	const char * pBody = ::FindBody( m_theMess );
		
	if ( pBody )
		Body = pBody;

	return SUCCESS;
}


RESULT
QCMessage::GetMessageForDisplay(
	ContentConcentrator::ContextT		in_context,
	CString &							out_szBody,
	CString *							out_pszHeaders,
	bool *								out_bWasConcentrated,
	bool								in_bAlwaysStripHTMLCode,
	bool								in_bStripDocumentLevelTags,
	bool								in_bRelaxLocalFileRefStripping,
	bool								in_bMorphHTML )
{
	if (out_pszHeaders)
		out_pszHeaders->Empty();
	out_szBody.Empty();

	if ( ! m_theMess )
	{
		ASSERT(0);
		return ERR_NOT_INIT;
	}
	
	CString			szConcentratedMessage;
	const char *	szMessageForDisplay = NULL;
	
	//	We need to be able to determine when we're dealing with a composition message,
	//	whine like crazy if we don't always have the information necessary!
	ASSERT(m_ParentDoc);
	ASSERT(m_ParentDoc->m_Sum);

	//	Assume that we're not concentrating
	bool			bWasConcentrated = false;
	
	if ( m_ParentDoc && m_ParentDoc->m_Sum )
	{
		//	Assume that we'll allow concentration
		bool	bAllowConcentration = true;

		//	Don't allow concentration for full message view composition windows
		//	or any messages in the Out mailbox.
		if (in_context == ContentConcentrator::kCCFullViewContext)
			bAllowConcentration = !m_ParentDoc->m_Sum->IsComp();
		else
			bAllowConcentration = (m_ParentDoc->m_Sum->m_TheToc->m_Type != MBT_OUT);

		if (bAllowConcentration)
		{
			bWasConcentrated = ContentConcentrator::Instance()->ConcentrateMessage( in_context, m_ParentDoc->m_Sum,
																					m_theMess, szConcentratedMessage );
			if (bWasConcentrated)
				szMessageForDisplay = static_cast<LPCTSTR>(szConcentratedMessage);
		}
	}

	//	Pass back whether or not we concentrated if the caller cares
	if (out_bWasConcentrated)
		*out_bWasConcentrated = bWasConcentrated;

	if (!bWasConcentrated || !szMessageForDisplay)	//	We didn't concentrate - default to full content
		szMessageForDisplay = m_theMess;

	const char *		pStartBody;

	//	Find the end of the headers
	if (m_HasHeaders)
	{
		pStartBody = ::FindBody(szMessageForDisplay);
		if (pStartBody && out_pszHeaders)
		{
			int			nHeadersLength = pStartBody - szMessageForDisplay;
			char *		szHeadersBuffer = out_pszHeaders->GetBuffer(nHeadersLength);

			strncpy(szHeadersBuffer, szMessageForDisplay, nHeadersLength);
			out_pszHeaders->ReleaseBuffer(nHeadersLength);
		}
	}
	else
	{
		pStartBody = szMessageForDisplay;
	}

	if (!pStartBody)
	{
		//	Previously SK said: "this one comes up when a new message is open and we process the out.mbx"
		//	So let's try only ASSERT'ing here when it's not for use in composition 
		ASSERT(in_context == ContentConcentrator::kCCNoConcentrationContext);
		return ERR_NOT_INIT;
	}

	::GetBodyAsHTML( out_szBody, pStartBody, in_bAlwaysStripHTMLCode,
					 in_bStripDocumentLevelTags, in_bRelaxLocalFileRefStripping );

	//	Build the URI map no matter what, because we'll need it even
	//	if conversion from CID's to local file URLs happens later.
	URIMap		uriMap;
	int			nURIs = uriMap.BuildURIMap(m_theMess);

	//	If we have embeded content, then convert cid's to local file URLs (if the
	//	caller wants us to) and add any orphaned URIs to the body as attachments.
	if (nURIs)
	{
		if (in_bMorphHTML)
		{
			//	Convert CIDs to local file URLs
			MorphMHTML(out_szBody, uriMap);
		}
		else
		{
			//	Do much of the work of MorphMHTML, but don't make the results
			//	permanent. Leave out_szBody untouched and discard the cooked
			//	HTML. The only point of this is to figure out which URIs
			//	are orphaned/unreferenced (if any).
			char *		pDiscardCookedHTML = NULL;

			ConvertURIs(out_szBody, &pDiscardCookedHTML, uriMap);

			free(pDiscardCookedHTML);
		}

		//	Add orphaned URIs to the body as attachments.
		//
		//	Fixes bug where Exchange sometimes incorrectly formats messages using
		//	multipart/related instead of multipart/mixed. To handle this we check
		//	for references to the related parts, and display any unreferenced parts
		//	as attachments.
		uriMap.AddOrphanedURIsToBodyAsAttachments(out_szBody);
	}

	return SUCCESS;
}


int QCMessage::GetAttachments( CString & Attachments )
{
	//	Body of routine moved to msgutils.cpp for common use.
	return ::GetAttachments(m_theMess, Attachments);
}


RESULT QCMessage::GetEmbeddedObjectHeaders(CString & Headers, bool bIncludeEmoticons)
{
	m_theMap.BuildEmbeddedObjectHeaders(Headers, bIncludeEmoticons);

	return SUCCESS;
}

RESULT QCMessage::GetEmbeddedObjectPaths( CString & Paths )
{
	m_theMap.GetEmbeddedObjectPaths( Paths );

	return SUCCESS;
}


/*****
	QCMimePart Implementation
*****/

QCMimePart::QCMimePart( const char * cid, const char * path, int mode )
{
	m_csCID = cid;
	m_jjFile.Open( path, mode ); 
}

// use to get at an embedded object
RESULT QCMimePart::GetCID( CString& csCID )
{
	csCID = m_csCID;

	// BOG: SUCCESS == 0
	return (csCID.GetLength() == 0);
}

RESULT QCMimePart::Seek( INT32 offset, INT32 mode /* = SEEK_SET */ )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		char		szLogBuf[256];
		sprintf(szLogBuf, "LOGNULL QCMimePart::Seek() JJFileMT::Seek(%ld %ld)", offset, mode);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		long lNewOffset = 0;
		if ( SUCCEEDED( m_jjFile.Seek( offset, mode, &lNewOffset )))
			return lNewOffset;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Tell( INT32 * pOffset )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		long lOffset;
		if ( SUCCEEDED( m_jjFile.Tell( &lOffset )))
		{
			*pOffset = lOffset;
			return SUCCESS;
		}
	}

	return ERR_BAD_OPEN;
}


RESULT QCMimePart::Read( CHAR * buf, INT32 bufsize, INT32 * pRead )
{
	*pRead = 0;

	if ( m_jjFile.IsOpen() == S_OK )
	{
		long lRead;
		if ( SUCCEEDED(m_jjFile.RawRead( buf, bufsize, &lRead )))
		{
			// BOG: yes keith, it was a leap of faith. tried this out, and
			// darn near filled up my disk. RawRead looks good, at least for
			// binary stuff.

//			*pRead = bufsize;	// boy, this looks like a leap of faith
			*pRead = lRead;
			return SUCCESS;
		}
		else
			return ERR_BAD_READ;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Get( CHAR * value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Get( value )))
			return SUCCESS;
		else
			return ERR_BAD_READ;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Get( INT32 * value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Get( value )))
			return SUCCESS;
		else
			return ERR_BAD_READ;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Get( UINT32 * value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{

		if ( SUCCEEDED( m_jjFile.Get( (INT32*)value )))
			return SUCCESS;
		else
			return ERR_BAD_READ;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Get( CHAR * buf, INT32 bufsize )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Read( buf, bufsize )))
			return SUCCESS;
		else
			return ERR_BAD_READ;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::GetLine( CHAR * str, INT32 bufsize )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.GetLine( str, bufsize )))
			return SUCCESS;
		else
			return ERR_BAD_READ;
	}

	return ERR_BAD_OPEN;
}

// use when creating an embedded part
RESULT QCMimePart::Write( CHAR * buf, INT32 bufsize, INT32 * pWritten )
{
	*pWritten = 0;

	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED(m_jjFile.Put( buf, bufsize )))
		{
			*pWritten = bufsize;	// another leap of faith
			return SUCCESS;
		}
		else
			return ERR_BAD_WRITE;
	}

	return ERR_BAD_OPEN;
}


RESULT QCMimePart::Put( CHAR value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Put( value )))
			return SUCCESS;
		else
			return ERR_BAD_WRITE;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Put( INT32 value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Put( value )))
			return SUCCESS;
		else
			return ERR_BAD_WRITE;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Put( UINT32 value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Put( (INT32)value )))
			return SUCCESS;
		else
			return ERR_BAD_WRITE;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Put( const void * str /* = NULL */, INT32 len /* = -1L */ )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Put( (const char *)str, len )))
			return SUCCESS;
		else
			return ERR_BAD_WRITE;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::PutLine( const void * str /* = NULL */, INT32 len /* = -1L */ )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Put( (const char *)str, len )))
			return SUCCESS;
		else
			return ERR_BAD_WRITE;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Close()
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Close()))
			return SUCCESS;
		else
			return ERR_BAD_CLOSE;
	}

	return ERR_BAD_OPEN;
}

RESULT QCMimePart::Delete( bool bDeleteAny /*= false*/ )
{
	char EmbeddedDir[_MAX_PATH + 1];
	wsprintf(EmbeddedDir,"%s%s",(const char *)EudoraDir,
					(const char *)CRString(IDS_EMBEDDED_FOLDER));

	CString filename;
	BSTR badFilename;
	bool bOurFile = false;

	if ( SUCCEEDED(m_jjFile.GetFName( &badFilename )) ) {
		filename = badFilename;

		if ( strstr( filename, EmbeddedDir ) )
			bOurFile = true;

		::SysFreeString( badFilename );
	}

	if ( !bOurFile && !bDeleteAny )
		return S_OK;
	else if ( SUCCEEDED(m_jjFile.Delete()) )
		return S_OK;
	else
		return ERR_BAD_DELETE;
}
