// MIME.H
//
// Routines for MIME
//

#ifndef _MIME_H_
#define _MIME_H_

#include "MIMEReader.h"

class LineReader;
class JJFile;
class HeaderDesc;
class Decoder;
class CTLAddress;



class CTranslator;
/*
 * MIME converter state structure
 */
class MIMEState : public CObject
{
public:
	MIMEState(LineReader* lr, HeaderDesc* hd = NULL);
	~MIMEState();

	int Init(HeaderDesc* hd);

	void SetLineReader(LineReader* lr) {m_LineReader = lr;}
	LineReader* GetLineReader() {return m_LineReader;}

	BoundaryType IsBoundaryLine(const char* buf) const;
	BoundaryType HuntBoundary(char* buf, LONG bSize);
	int WriteBoundary(BoundaryType boundaryType) const;

	HeaderDesc*	m_hd;
	Decoder*	m_Coder;
	MIMEReader*	m_Reader;
	LineReader*	m_LineReader;
	
	BOOL IsGrokked();
	char m_boundary[128];

	CTranslator* m_Translator;
};


// this class is used to keep track of MHTML embedded
// objects that are part of a multipart/related
// all non text/xxx parts are stored in attachment files
// the filename and content-id content-base/content-location
// are cached up via a list of these elements
// if a text/html part is found while parsing the multipart/related
// then the list of attachments is added to the message as
// "Embedded Content" lines, else the list of attachments is
// added as "Attachment Converted" lines.
class EmbeddedElement : public CObject
{
public:
	EmbeddedElement()
	{ 
		m_Path = ""; m_CID = "";
		m_MRI = m_CIDHash = m_CLHash = m_MRI_CB_CL_Hash = 0;
	}

	void Init( const char* path, DWORD MRI, const char* CID, const char* CB, const char* CL );

	CString & GetPath() { return m_Path; }
	CString & GetCID() { return m_CID; }
	DWORD GetMRI() { return m_MRI;}
	DWORD GetCIDHash() { return m_CIDHash; }
	DWORD GetCLHash() { return m_CLHash; }
	DWORD GetMRI_CB_CL_Hash() { return m_MRI_CB_CL_Hash; }

	void SetPath( CString Path ) { m_Path = Path; }
	void SetCID( CString CID );
	void SetMRI( DWORD MRI ) { m_MRI = MRI;}
	void SetCIDHash( DWORD CIDHash ) { m_CIDHash = CIDHash; }
	void SetCLHash( DWORD CLHash ) { m_CLHash = CLHash; }
	void SetMRI_CB_CL_Hash( DWORD Hash ) { m_MRI_CB_CL_Hash = Hash; }

private:
	CString m_Path;		// where the content was stored
	CString m_CID;		// for comp message stuff

	DWORD m_MRI;		// multipart/related id (0-indexed order it appears in the message)
	DWORD m_CIDHash;
	DWORD m_CLHash;
	DWORD m_MRI_CB_CL_Hash;
};

int SaveEmbeddedElement( const char * filename );
int WriteEmbeddedElements( BOOL isMHTML );
void FlushEmbeddedElements();



// Function Prototypes

void mimeInit();
CTLAddress* GetEMSHeaders(long diskStart);
void HexToString(char* dest, const char* src, int Num = 4);
JJFile* OpenAttachFile(LPTSTR Name);
JJFile* OpenEmbeddedFile(LPTSTR Name);
int WriteAttachNote(JJFile* AttachFile, UINT labelStrID, LPCTSTR params = NULL);
Decoder* FindMIMEDecoder(LPCTSTR encoding, BOOL AllocateCoder);
short FindMIMECharset(LPCTSTR charSet);
void DeMacifyName32(LPTSTR Name);

#endif // #ifndef _MIME_H_
