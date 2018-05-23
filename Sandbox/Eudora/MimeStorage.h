#ifndef __MIMESTORAGE_H__
#define __MIMESTORAGE_H__

#include "QCUtils.h"
#include "Convhtml.h"

typedef unsigned long	UINT32;
typedef CString			PART_ID;
typedef INT32			RESULT;
#define SUCCESS			0
#define ERR_BAD_CREATE	-1
#define ERR_BAD_OPEN	-2
#define ERR_BAD_READ	-3
#define ERR_BAD_WRITE	-4
#define ERR_BAD_CLOSE	-5
#define ERR_BAD_NEW		-6
#define ERR_NOT_INIT	-7
#define ERR_NOT_CID		-8

class CMessageDoc;
class QCMimePart;

class QCMessage : public CObject
{
public:
	QCMessage(CMessageDoc* ParentDoc);
	~QCMessage();

	RESULT Init( const char * messageid, const char * buf, BOOL HasHeaders = TRUE );
	RESULT InitMap( const char * ECHeaders );
	BOOL   IsInit(){ return m_theMess != NULL; }
	RESULT Flush();
	RESULT NukeCIDs();
	RESULT IsRealCID( CString CID );

	RESULT GetRawMessage( CString & Message );
	RESULT GetFullMessage( CString & Message );
	RESULT GetHeaders( CString & Headers );
	RESULT GetBody( CString & Body );
	RESULT GetBodyAsHTML( CString & Body );
	
	//RESULT GetAttachments( CString & Attachments );

	//more useful if it tells how many attahcments it got.
	int GetAttachments( CString & Attachments );
	RESULT GetPart( PART_ID id, QCMimePart ** ppPart );	// where id is a URI
	RESULT GetPartAsFile( PART_ID id, char* pPath, int pathLen );

	// use when creating messages
	RESULT AddPart( const CString &path,				// use an existing file
                    QCMimePart ** ppPart );

	RESULT AddPart( const char * extension,				// create a new file with this extension
                    QCMimePart ** ppPart );

	RESULT GetEmbeddedObjectHeaders( CString & Headers );
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
	RESULT Get( BOOL * value );
	RESULT Get( UINT16 * value );
	RESULT Get( INT32 * value );
	RESULT Get( UINT32 * value );
	RESULT Get( CHAR * str, INT32 bufsize );
	RESULT GetLine( CHAR * str, INT32 bufsize );

	// use when creating an embedded part
	RESULT Write( CHAR * but, INT32 bufsize, INT32 * pWritten );

	RESULT Put( CHAR value );
	RESULT Put( BOOL value );
	RESULT Put( UINT16 value );
	RESULT Put( INT32 value );
	RESULT Put( UINT32 value );
	RESULT Put( const void * str = NULL, INT32 len = -1L );
	RESULT PutLine( const void * str = NULL, INT32 len = -1L );

	RESULT Close();

protected:
	CString		m_csCID;
	JJFileMT	m_jjFile;		// object to access the local storage
};

#endif // __MIMESTORAGE_H__
