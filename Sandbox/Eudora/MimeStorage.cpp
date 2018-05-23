#include "stdafx.h"

#include "etf2html.h"
#include "Eudora.h"
#include "Fileutil.h"
#include "msgutils.h"
#include "MimeStorage.h"
#include "resource.h"
#include "rs.h"
#include "rpc.h"
#include "trnslate.h"
#include "Text2HTML.h"
#include "QCGraphics.h"
#include "utils.h"
#include "msgdoc.h"
#include "summary.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



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
	m_theMess = new char[ strlen( buf ) + 1 ];
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

		fBaptizeBlockMT(m_theMess, szName);
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

RESULT QCMessage::IsRealCID( CString CID )
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
	m_theMap.ResolveURI( id, path, sizeof( path ) );

	if ( *path )
	{
		if ( strncmp( path, "file://", strlen("file://") ) == 0 )
			*ppPart = new QCMimePart( id, &path[strlen("file://")], _O_RDONLY );
		else
			*ppPart = new QCMimePart( id, path, _O_RDONLY );

		if ( *ppPart )
			return SUCCESS;
	}

	return ERR_BAD_CREATE;
}


// where id is a URI, and pPath is filled with a full path to the "guts"
// of the part. ** this hack by Ben **
RESULT QCMessage::GetPartAsFile( PART_ID id, char* pPath, int pathLen )
{
	// lookup the part (ResolveURI won't stuff all over pPath unless
	// it gets somethin' worth sniffin'.)
	if ( m_theMap.ResolveURI( id, pPath, pathLen ) )
		return SUCCESS;
	else
		return ERR_BAD_CREATE;
}


RESULT QCMessage::AddPart( const CString &path,	// use an existing file
                    QCMimePart ** ppPart )
{
	// create a URLMap entry/CID for the part
	char cid[ 256 ];
	if ( m_theMap.AddCIDMap( m_MessageId, path, cid, sizeof( cid )) )
	{
		*ppPart = new QCMimePart( cid, (LPCTSTR)path, _O_RDWR | _O_EXCL );

		if ( *ppPart )
			return SUCCESS;
	}

	return ERR_BAD_CREATE;
}

RESULT QCMessage::AddPart( const char * extension,	// create a new file with this extension
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
	if ( m_theMap.AddCIDMap( m_MessageId, tmpPath, cid, sizeof( cid )) )
	{
		*ppPart = new QCMimePart( cid, tmpPath, _O_CREAT | _O_TRUNC | _O_RDWR );

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

// strips headers and trailers (like attachment converted lines)
RESULT QCMessage::GetBodyAsHTML( CString & Body)
{
	const char*	pStartLine;

	Body.Empty();

	// find the end of the headers
	if ( ! m_theMess )
	{
		ASSERT(0);
		return ERR_NOT_INIT;
	}

	if (m_HasHeaders)
		pStartLine = ::FindBody( m_theMess );
	else
		pStartLine = (LPCTSTR)m_theMess;

	if (!pStartLine)
	{
		//ASSERT(0); //@@@@ SK this one comes up when a new message is open and we process the out.mbx
		return ERR_NOT_INIT;
	}

	::GetBodyAsHTML(Body, pStartLine);

	return SUCCESS;
}


int QCMessage::GetAttachments( CString & Attachments )
{
	CRString ACTrailer( IDS_ATTACH_CONVERTED );

	int num_attachments = 0;
	Attachments.Empty();

	if ( m_theMess )
	{
		char * pLine = m_theMess;
		char * EndLine;
		CString ExcerptOn = "<excerpt>";
		CString ExcerptOff = "</excerpt>";
		int		ExcerptLevel = 0;

		while (*pLine)
		{
			// prepare for the next line
			EndLine = strchr(pLine, '\r');
			if (EndLine)
				EndLine--;
			else
				EndLine = pLine + strlen(pLine) - 1;

			if ( strnicmp( ExcerptOn, pLine, ExcerptOn.GetLength()) == 0 )
				ExcerptLevel++;
			if ( strnicmp( ExcerptOff, pLine, ExcerptOff.GetLength()) == 0 )
				ExcerptLevel--;

			if ( ( strnicmp( ACTrailer, pLine, ACTrailer.GetLength()) == 0 ) && (ExcerptLevel == 0) )
			{
				// extract path
				const char * pPath = pLine + ACTrailer.GetLength();
				while ( *pPath == ' ' || *pPath == '"' )
					pPath++;

				// pPath now points at the path
				char * pEnd = EndLine;
				if ( *pEnd == '"')
					pEnd--;

				// add the path to the return cstring
				if ( ! Attachments.IsEmpty() )
					Attachments += "; ";

				// Little trick here of temporarily creating a null-terminated string
				// out of the line in order to save on an extra memory allocation
				char SaveChar = pEnd[1];
				pEnd[1] = 0;
				Attachments += pPath;
				pEnd[1] = SaveChar;

				num_attachments++;
			}

			pLine = EndLine + 1;
			if (*pLine == '\r')
				pLine++;
			if (*pLine == '\n')
				pLine++;
		}
	}

	return num_attachments;
}


RESULT QCMessage::GetEmbeddedObjectHeaders( CString & Headers )
{
	m_theMap.BuildEmbeddedObjectHeaders( Headers );

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
		if ( SUCCEEDED( m_jjFile.Tell( pOffset )));
			return SUCCESS;
	}

	return ERR_BAD_OPEN;
}


RESULT QCMimePart::Read( CHAR * buf, INT32 bufsize, INT32 * pRead )
{
	*pRead = 0;

	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED(m_jjFile.RawRead( buf, bufsize, pRead )))
		{
			// BOG: yes keith, it was a leap of faith. tried this out, and
			// darn near filled up my disk. RawRead looks good, at least for
			// binary stuff.

//			*pRead = bufsize;	// boy, this looks like a leap of faith
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

RESULT QCMimePart::Get( BOOL * value )
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

RESULT QCMimePart::Get( UINT16 * value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Get( (unsigned short *)value )))
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
		if ( SUCCEEDED( m_jjFile.Get( value )))
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

RESULT QCMimePart::Put( BOOL value )
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

RESULT QCMimePart::Put( UINT16 value )
{
	if ( m_jjFile.IsOpen() == S_OK )
	{
		if ( SUCCEEDED( m_jjFile.Put( (unsigned short)value )))
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
		if ( SUCCEEDED( m_jjFile.Put( value )))
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
