// APPEND.CPP
//
// Routines to append a message to a remot eIMAP mailbox. 
// Patterned after the Sendmail/SMTP routines
//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "summary.h"
#include "sendmail.h"
#include "progress.h"
#include "resource.h"
#include "rs.h"
#include "tocdoc.h"
#include "guiutils.h"
#include "MIMEMap.h"
#include "QP.h"
#include "Base64.h"

// IMAP4 
#include "ImapExports.h"
#include "imap.h"
#include "imapgets.h"
#include "imapjournal.h"
#include "imapresync.h"
#include "imapfol.h"
#include "imapopt.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imapmlst.h"
#include "append.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define		PREFERED_BUFFER_SIZE	10000
#define		MINIMUM_BUFFER_SIZE		2048

// JOK - Can't write a header line longer than this.
#define		MAX_HEADER_LINELEN		998
#define		MAXBUF					1024

static LONG		lBufSize = PREFERED_BUFFER_SIZE;

// Headers to ignore when reformating a header:
//
// Ignore any header starting with "Content-".
CString MimeHeaderPrefix;
CString XuidHeader;
CString XImflagsHeader;
CString MimeVersion;
CString HeaderBcc;



// Static resource strings:
CString DateHeader;


// ========== Internal data structures ===========

// STRINGDRIVER for sending a spool find to c-client as part of an APPEND.

static void SpoolDriverInit (STRING *s, void *data, unsigned long size);
static char SpoolDriverNext (STRING *s);
static void SpoolDriverSetpos (STRING *s,unsigned long i);

STRINGDRIVER FileStringDriver = {
	SpoolDriverInit,
	SpoolDriverNext,
	SpoolDriverSetpos
};


// This driver reads chunck from a mailbox on one server.
static void StreamAppendDriverInit (STRING *s, void *data, unsigned long size);
static char StreamAppendDriverNext (STRING *s);
static void StreamAppendDriverSetpos (STRING *s, unsigned long i);

STRINGDRIVER StreamAppendDriver = {
	StreamAppendDriverInit,
	StreamAppendDriverNext,
	StreamAppendDriverSetpos
};



// ===============================================================//
// Statics
void APPENDError(const char* Command, const char* Message);

// For Apppend:
void	NewMimeBoundary ( CString& Boundary);
long	APPENDEncodingType ( const char* fname, MIMEMap& mm );
int		AppendMIME(const char* Filename, MIMEMap& mm, long EncoType, CFileWriter *pBodyFileWriter);
int		AppendContentType(short tableID, WORD flag, CFileWriter *pBodyFileWriter, MIMEMap* mm = NULL);
int		AppendDataText (const char* text, CFileWriter *pBodyFileWriter, int length = -1, int InitCharsOnLine = -1);
BOOL	UseQPToAppend( const char* text1, const char* text2, short etid, BOOL bIsRich );
int		AppendMIMEMessageHeaders(LPCSTR pBoundary, int flag, JJFileMT *pOpenFile);
BOOL	IsContinuationChar (TCHAR c);
BOOL	IsHeaderToInclude (LPCSTR t);
BOOL	ContainsNonAsciiChar (LPCSTR t);

// These are in sendmail.cpp:
extern void NameCharset(char* charset, int tid);
extern BOOL AnyFunny(const char* text);
extern BOOL LongerThan(const char* text, short len);

// ==============================================================//

// APPENDError
// Show the user a dialog box with the error message on command
void APPENDError(const char* Command, const char* Message)
{
#if 0 // JOK - Must fix if we need it.
	ErrorDialog(	IDS_ERR_TRANSFER_MAIL, 
					Command, 
					"APPEND", 
					( const char* ) CurSMTPServer, 
					Message);
#endif 
}






// SpoolDriver functions. These are static and operate on the given STRING.
static void SpoolDriverInit (STRING *s, void *data, unsigned long size)
{
	CFileReader *pFileReader = NULL;

	// Sanity.
	if (!s)
		return;

	// Get the CFileReader object from the string.
	pFileReader = (CFileReader *) s->data;

	if (!pFileReader)
		return;

	// Initialize
	*pFileReader->m_buffer = 0;

	// Set STRING to point to the buffer in the file reader.
	s->chunk = s->curpos = pFileReader->m_buffer;

	// Nothing in the buffer yet.
	s->chunksize = s->cursize = 0;

	// Open the file reader and get the full size of the file.
	// Open() also sets the size of the file.
	if ( pFileReader->Open () )
	{
		s->size = pFileReader->GetTotalSize();
		
		// Go read a chunk-full from the file.
		s->chunksize = s->cursize = pFileReader->Read();

		// Catch error.
		if (s->cursize < 0)
		{
			s->cursize = 0;
			s->chunksize = 0;
		}
	}

	// never any offset 
	s->data1 = s->offset = 0;
}



static char SpoolDriverNext (STRING *s)
{
	CFileReader *pFileReader = NULL;

	// Sanity.
	if (!s)
		return 0;

	// Get the CFileReader object from the string.
	pFileReader = (CFileReader *) s->data;

	if (!pFileReader)
		return 0;

	// Initialize
	*pFileReader->m_buffer = 0;

	// Always reset these.
	s->chunk = s->curpos = pFileReader->m_buffer;

	// Go read another chunk-full from the file.
	s->chunksize = s->cursize = pFileReader->Read();

	// Did we read correctly?
	if ( s->cursize < 0 )
	{
		// Error.
		s->cursize = 0;
		s->chunksize = 0;
	}

	return *(pFileReader->m_buffer);
}


static void SpoolDriverSetpos (STRING *s,unsigned long i)
{
	s->curpos = s->chunk + i;
	s->cursize = s->chunksize - i;
}




//============== Functions for the StreamAppendDriver streain driver ===========/


// StreamAppendDriver functions. These are static and operate on the given STRING.
static void StreamAppendDriverInit (STRING *s, void *data, unsigned long size)
{
	CImapAppend *pImapAppend = NULL;

	// Sanity.
	if (!s)
		return;

	// The append object is the strings data.
	pImapAppend = (CImapAppend *)s->data;
	if (pImapAppend == NULL)
		return;

	// Must have the source CIMAP object.
	CIMAP *pSrcImap = pImapAppend->m_pSrcImap;
	if (!pSrcImap)
		return;

	// "size" is the rfc822 size of the message. Set it and don't change it
	s->size = size;

	// Set STRING to point to the buffer in the file reader.
	s->chunk = s->curpos = pImapAppend->m_pBuffer;

	// Nothing in the buffer yet.
	s->chunksize = s->cursize = 0;

	// Go read a chunk-full from the message.
	unsigned long first, nBytes;
	unsigned long chunksize = 0;
	first	= 0;
	nBytes	= min (s->size, pImapAppend->m_nBufferSize);

	// Read:
	pSrcImap->UIDFetchPartialContentsToBuffer (pImapAppend->m_SrcUid,
			 NULL, first, nBytes, pImapAppend->m_pBuffer, pImapAppend->m_nBufferSize, &chunksize);

	s->cursize = s->chunksize = chunksize;

	// Catch error.
	if (s->cursize <= 0)
	{
		s->cursize = 0;
		s->chunksize = 0;
	}

	// Increment bytes read
	pImapAppend->m_nBytesRead += s->cursize;

	// never any offset 
	s->data1 = s->offset = 0;
}



static char StreamAppendDriverNext (STRING *s)
{
	CImapAppend *pImapAppend = NULL;

	// Sanity.
	if (!s)
		return 0;

	// The append object is the strings data.
	pImapAppend = (CImapAppend *)s->data;

	if (pImapAppend == NULL)
		return 0;

	// Must have the source CIMAP object.
	CIMAP* pSrcImap = pImapAppend->m_pSrcImap;

	if (!pSrcImap)
		return 0;

	// Re-initiallize these..
	// Set STRING to point to the buffer in the file reader.
	s->chunk = s->curpos = pImapAppend->m_pBuffer;
	s->cursize = 0;
	*s->chunk = 0;

	// Fetch next chunk. 
	unsigned long first, nBytes;

	first = pImapAppend->m_nBytesRead;
	nBytes = min (pImapAppend->m_nBufferSize, s->size - pImapAppend->m_nBytesRead);

	if (nBytes <= 0)
	{
		// Error.
		s->cursize = 0;
		s->curpos = NULL;

		return 0;
	}

#ifdef _DEBUG
	if (nBytes < pImapAppend->m_nBufferSize)
	{
		int tt = nBytes;
	}
#endif // _DEBUG

	// Go read another chunk-full from the message.
	pSrcImap->UIDFetchPartialContentsToBuffer ( pImapAppend->m_SrcUid, NULL, first, nBytes,
							 pImapAppend->m_pBuffer, pImapAppend->m_nBufferSize,
							 &(s->cursize) );

	// Did we read correctly?
	if ( s->cursize < 0 )
	{
		// Error.
		s->cursize = 0;
		s->curpos = NULL;
	}

	// Increment bytes read
	pImapAppend->m_nBytesRead += s->cursize;

	return *s->chunk;
}


static void StreamAppendDriverSetpos (STRING *s, unsigned long i)
{
	if (s)
	{
		s->curpos = s->chunk + i;
		s->cursize = s->chunksize - i;
	}
}




//================================== CImapAppend imaplementation =======================//

CImapAppend::CImapAppend (CIMAP *pDstImap, CIMAP *pSrcImap, CSummary *pSum, unsigned long nSrcUid)
{
	// Append to this
	m_pDstImap = pDstImap;

	// This can be NULL;
	m_pSrcImap = pSrcImap;

	m_pSum = pSum;

	// This can be 0.
	m_SrcUid = nSrcUid;

	m_pStartHeader = m_pStartBody = NULL;

	// Clear the IMAPFLAGS structure.
	memset ( (void *)&m_ImapFlags, 0, sizeof (IMAPFLAGS) );

	// Don't know if it will be multipart.
	m_bIsMultipart = FALSE;

	// Scratch buffer. Allocated when needed.
	m_nBufferSize	= 0;
	m_pBuffer		= NULL;

	// Keep track of how many bytes left ot download during a remote-to-remote append.
	m_nBytesRead = 0;

	// Time of source summary.
	m_Seconds = 0;

	// If these haven't been initialized yet, do so:
	// Note: These can't be made static!
	if (MimeHeaderPrefix.IsEmpty())
		MimeHeaderPrefix = CRString (IDS_MIME_HEADER_PREFIX);

	if (XuidHeader.IsEmpty())
		XuidHeader = CRString (IDS_IMAP_UID_XHEADER);

	if (XImflagsHeader.IsEmpty())
		XImflagsHeader = CRString (IDS_IMAP_IMFLAGS_XHEADER);

	if (MimeVersion.IsEmpty())
		MimeVersion = CRString (IDS_MIME_HEADER_VERSION);

	if (DateHeader.IsEmpty())
		DateHeader = CRString (IDS_HEADER_DATE);

	// Nedd this later:
	//
	if (HeaderBcc.IsEmpty())
		HeaderBcc = CRString (IDS_HEADER_BCC);

}


CImapAppend::~CImapAppend()
{
	m_pSrcImap = m_pDstImap = NULL;

	// Free buffer.
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
	}

	m_pBuffer = NULL;
	m_nBufferSize = 0;
}



// SetFlags
// NOTES
// ulFlags have bits set as defined in summary.h
// END NOTES

void CImapAppend::SetFlags (unsigned long Flags)
{
	memset ( (void *)&m_ImapFlags, 0, sizeof (IMAPFLAGS) );

	m_ImapFlags.DELETED	= Flags & IMFLAGS_DELETED;
	m_ImapFlags.SEEN		= Flags & IMFLAGS_SEEN;
	m_ImapFlags.FLAGGED	= Flags & IMFLAGS_FLAGGED;
	m_ImapFlags.ANSWERED	= Flags & IMFLAGS_ANSWERED;
	m_ImapFlags.DRAFT		= Flags & IMFLAGS_DRAFT;
}


// FUNCTION
// Copy the string.
// END FUNCTION
void CImapAppend::SetMbxStub (LPCSTR p)
{
	m_MbxStub = p;
}

	
// Main entry point to performing the append.
HRESULT CImapAppend::AppendMessage (BOOL Spool, BOOL InBackground)
{
	HRESULT		hResult = E_FAIL;

	// Sanity. Verify internal state.
	if (m_MbxStub.IsEmpty())
	{
		ASSERT (0);
		return E_FAIL;
	}

	if (m_SrcAttachDir.IsEmpty())
	{
		ASSERT (0);
		return E_FAIL;
	}

	if (m_SpoolDir.IsEmpty())
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Must have an open destination CIMAP
	if (!m_pDstImap)
		return E_FAIL;

	if (Spool)
	{
		CString SpoolFilepath;

		// Return the full path to the spool file in SpoolFilepath.
		if (SpoolMessageToFile (SpoolFilepath))
		{
			// Now append the file.
			hResult = AppendSpoolFile (SpoolFilepath);
		}

		// Delete the spool file if it exists.
		DeleteLocalFile ( SpoolFilepath );
	}
	else
	{
		// Do converions on the fly.
		hResult = E_FAIL;
	}

	return hResult;
}




//
// FUNCTION
// Append the given spool file to the remote mailbox. 
HRESULT CImapAppend::AppendSpoolFile (LPCSTR pSpoolFilepath)
{
	STRING	ms;
	HRESULT	hResult = E_FAIL;

	if (!pSpoolFilepath)
		return E_INVALIDARG;

	// We should have an open IMAP connection.
	if (!m_pDstImap)
		return E_FAIL;

	// Instantiate a CFileWriter and attach it to the STRING.
	CFileReader SpoolFileReader (pSpoolFilepath);

	// Initialize the STRING.
	memset ((void *)&ms, 0, sizeof (STRING));

	// Attach our "FileStringDriver" to the STRING.
	ms.dtb = &FileStringDriver;

	// The File Reader becomes the STRING's "data".
	ms.data = (void *)&SpoolFileReader;

	// Initialize the STRING. THe STRING's init() function will set the full
	// file's size, read the first chunk and set the chunksize.

	(ms.dtb)->init ( &ms, SpoolFileReader.m_buffer, SpoolFileReader.GetTotalSize () );

	// Launch the APPEND.

	hResult = m_pDstImap->AppendMessage ( &m_ImapFlags, m_Seconds, &ms );

	return hResult;

}




// SpoolMessageToFile [PRIVATE]
// NOTES
// Return the full path to the spool file in "SpoolFilepath".
// NOTE: We create 2 temporary files, one for the header and the other for
// the body. We need to do this because we won't know any of the Content-*
// values until we've done constructing the message's body.
// END NOTES

BOOL CImapAppend::SpoolMessageToFile (CString& SpoolFilepath)
{
	BOOL			bResult = FALSE;
	CFileWriter		HeaderFileWriter;
	CFileWriter		BodyFileWriter;
	CString			str;

	// Get a unique filename in our spool directory for the header component of the message.

	if ( !GetUniqueTempFilepath (m_SpoolDir, CRString (IDS_IMAP_APPEND_FILENAME), str) )
	{
		// BUG: We should pop up an error message instead of asserting here.
		ASSERT (0);
		return FALSE;
	}

	// Create the empty file.
	if ( !CreateLocalFile (str, TRUE) )
	{
	    ErrorDialog( IDS_ERR_FILE_OPEN, str, CRString (IDS_ERR_FILE_OPEN_WRITING) );
		return FALSE;
	}

	// Set this as our header spool file.
	m_HeaderSpoolPath = str;

	// The complete message eventually gets copied to this so return this to
	// caller as our spool file..
	SpoolFilepath = str;

	// Now go do the same for m_BodySpoolFile.

	if ( !GetUniqueTempFilepath (m_SpoolDir, CRString (IDS_IMAP_APPEND_FILENAME), str) )
	{
		// BUG: We should pop up an error message instead of asserting here.
		ASSERT (0);
		return FALSE;
	}

	if ( !CreateLocalFile (str, TRUE) )
	{
	    ErrorDialog( IDS_ERR_FILE_OPEN, str, CRString (IDS_ERR_FILE_OPEN_WRITING) );
		return FALSE;
	}

	// Set this as our body spool file.
	m_BodySpoolPath = str;

	// Go write the message now.
	bResult = ReformatMessage ();

	return bResult;
}



// FUNCTION
// Do the grunge work for properly formatting the message to be appended.
// END FUNCTION
// NOTES
// This accumulates the message into m_SpoolFilepath.
// END FUNCTION

BOOL CImapAppend::ReformatMessage ()
{
	BOOL			bResult = FALSE;

	// Make sure we have a string.
	if (m_MbxStub.IsEmpty())
		return FALSE;

	// Write headers except "Content-Type:". We have to re-write it to include our
	// own boundary.
	// Note: This operates on the string pointed to by m_pStartHeader and sets the m_pStartBody
	// pointer before returning.

	m_pStartHeader = (LPCSTR)m_MbxStub;

	bResult = ReformatHeader ();

	// Format the body?
	if (bResult)
	{
		// If there is a body, go format it..
		if (m_pStartBody)
		{
			bResult = ReformatBody ();
		}
	}

	// If we succeeded, we now have to add the messaage's Content-type headers.
	if (bResult)
	{
		JJFileMT	jFile;
		CString		cHeader;

		bResult = SUCCEEDED( jFile.Open (m_HeaderSpoolPath, O_WRONLY | O_APPEND ) );
		if (bResult)
		{
			if (!m_bIsMultipart)
				bResult = ( AppendMIMEMessageHeaders( NULL, 0, &jFile) >= 0 );
			else
				bResult = ( AppendMIMEMessageHeaders( (LPCSTR)m_Boundary, 0, &jFile) >= 0 );
						
			if (!bResult)
				return bResult;

			// Blank header/body separator line.
			jFile.PutLine ();

			// Flush the headers, otherwise the above gets written to the end of the file!!.
			jFile.Flush();

			// Now append the body.
			JJFileMT jSrcFile;
			bResult = SUCCEEDED ( jSrcFile.Open (m_BodySpoolPath, O_RDONLY) );
			if (bResult)
			{
				struct stat stat_info;
				bResult = SUCCEEDED ( jSrcFile.Stat(&stat_info) );
				if (bResult)
					bResult = SUCCEEDED ( jSrcFile.JJBlockMove(0L, stat_info.st_size, &jFile) );

				jSrcFile.Close();
			}
			jFile.Close();
		}
	}

	// Remove the body spool file here.
	DeleteLocalFile (m_BodySpoolPath);

	// We may not have succeeded!.
	return bResult;
}


// FUNCTION
// Extract and write headers except Content-Type: 
// This must be treated specially.
// END FUNCTION
// NOTES
// Don't copy the following header lines:
// 1. IMAP_UID_XHEADER.
// 2. All Content-* lines
// 3. IMAP_IMFLAGS_XHEADER.
// END NOTES

BOOL CImapAppend::ReformatHeader ()
{
	BOOL			bResult = FALSE;
	const char		*p, *t;
//	CString			MimeHeaderPrefix;
	CFileWriter		HeaderFileWriter;
	size_t			len;
	BOOL			bDateHeaderFound = FALSE;

	// Initialize our file writer. 
	HeaderFileWriter.SetFilename (m_HeaderSpoolPath);

	if (!HeaderFileWriter.Open())
	{
	    ErrorDialog( IDS_ERR_FILE_OPEN, m_HeaderSpoolPath, CRString (IDS_ERR_FILE_OPEN_WRITING) );
		return FALSE;
	}

	// Ignore any header starting with "Content-".
//	MimeHeaderPrefix = CRString (IDS_MIME_HEADER_PREFIX);

	// Ignore these as well.
//	CRString XuidHeader		( IDS_IMAP_UID_XHEADER );
//	CRString XImflagsHeader ( IDS_IMAP_IMFLAGS_XHEADER ); 
//	CRString MimeVersion	(IDS_MIME_HEADER_VERSION);

	// Check if we have this. If not, generate one:
//	CRString DateHeader		( IDS_HEADER_DATE );

	// For continuation lline handling.
	//
	BOOL bPrevGoodHeader = FALSE;
	BOOL bThisGoodHeader = FALSE;

	// Loop through the header string.

	t = m_pStartHeader;
	bResult = TRUE;

	while (t && *t && bResult)
	{
		// Look for the blank line separating header from body.
		if (*t == '\r')
		{
			t++;
			// Could we have "\r\r"? It should be "\r\n".
			if (*t == '\r' || *t == '\n')
				t++;

			break;
		}
		else if (*t == '\n')
		{
			// Probably meant we had "\n\n"
			t++;
			break;
		}

		bThisGoodHeader = FALSE;

		// This is a header line. If it's not a "Content-*" header, write it to the file.
		p = strpbrk (t, "\r\n");
		if (!p)
		{
			// Some grievous error. If it's a valid header, print it and quit.

			// Continuation line??
			//
			if ( IsContinuationChar (*t) )
			{
				if (bPrevGoodHeader)
				{
					bThisGoodHeader = TRUE;
				}
			}
			else
			{
				p = strchr (t, ':');

				if (p)
				{
					// Don't print Content-* headers, X-UID or Imflags.
					//
					if ( IsHeaderToInclude (t) )
					{
						bThisGoodHeader = TRUE;

						if ( strnicmp (t, (LPCSTR)HeaderBcc, HeaderBcc.GetLength()) == 0 )
						{
							bThisGoodHeader = !ContainsNonAsciiChar (t);
						}
					}
				}
			}

			if ( bThisGoodHeader )
			{
				bPrevGoodHeader = TRUE;

				// Can this be a long line??
				len = strlen (t);

				// Print it.
				if (len <= MAX_HEADER_LINELEN)
				{
					HeaderFileWriter.Write (t, len);
					HeaderFileWriter.Write ("\r\n", 2);

					if (strnicmp ( t, (LPCSTR) DateHeader, DateHeader.GetLength() ) == 0)
					{
						bDateHeaderFound = TRUE;
					}
				}
			}
			else
			{
				bPrevGoodHeader = FALSE;
			}

			// This will be set into m_pStartBody.
			t = NULL;
		}
		else
		{
			len = p - t;

			if ( (len > 0) && (len <= MAX_HEADER_LINELEN) )
			{
				// Continuation line??
				//
				if ( IsContinuationChar (*t) )
				{
					if (bPrevGoodHeader)
					{
						bThisGoodHeader = TRUE;
					}
				}
				else if ( NULL != strchr (t, ':') )
				{
					// This is a valid. Print it if it's not Content-Type.
					if ( IsHeaderToInclude (t) )
					{
						bThisGoodHeader = TRUE;

						if ( strnicmp (t, (LPCSTR)HeaderBcc, HeaderBcc.GetLength()) == 0 )
						{
							bThisGoodHeader = !ContainsNonAsciiChar (t);
						}

					}
				}
			}

			if ( bThisGoodHeader )
			{
				bPrevGoodHeader = TRUE;

				HeaderFileWriter.Write (t, len);
				HeaderFileWriter.Write ("\r\n", 2);

				if (strnicmp ( t, (LPCSTR) DateHeader, DateHeader.GetLength() ) == 0)
				{
					bDateHeaderFound = TRUE;
				}
			}
			else
			{
				bPrevGoodHeader = FALSE;
			}

			// Advance over the "\r", "\n" or "\r\n".
			t = p;

			if (*t == '\r')
			{
				t++;
				if (*t == '\n')
					t++;
			}
			else
				t++;
		}
	}

	// Setup for possible call of ReformatBody.
	m_pStartBody = t;

	// 
	// If we didn't find a Date: header, add one now.
	//
	if (!bDateHeaderFound)
	{
		unsigned long Seconds = m_Seconds;

		//
		// If this is zero, use current time.
		//
		if (0 == Seconds)
			Seconds = time (0);

		TCHAR DateBuf [64];

		ComposeDate(DateBuf, Seconds);

		// Write this.
		size_t dLen = strlen (DateBuf);
		if (dLen != 0)
		{
			HeaderFileWriter.Write (DateBuf, dLen);
			HeaderFileWriter.Write ("\r\n", 2);
		}
	}

	// Cleanup.
	HeaderFileWriter.Close();

	return bResult;
}

			

// FUNCTION
// Do the real work of reformating a body.
// This is real nasty because we have tolool for "Attachment-Converted lines.
// END FUNCTION
// NOTES
// We'd never know if this body is multipart unless we wade through the body text
// looking for Attachment Converted lines. 
// NOTE: We make this message multipart if we find AT LEAST 1 Attachment Converted line!!!
// END NOTES

BOOL CImapAppend::ReformatBody ()
{
	char			ACStr[64];
	int				ACLen;
	BOOL			bResult = FALSE;
	LPCSTR			p;
	LPCSTR			t;
	CFileWriter		BodyFileWriter;
	CString			ContentType;
	size_t			len;

	// Do we really have a body? OK if we don't.
	if (m_pStartBody == NULL)
		return TRUE;

	// Initialize our file writer. 
	BodyFileWriter.SetFilename (m_BodySpoolPath);

	if (!BodyFileWriter.Open())
	{
	    ErrorDialog( IDS_ERR_FILE_OPEN, m_BodySpoolPath, CRString (IDS_ERR_FILE_OPEN_WRITING) );
		return FALSE;
	}


	// COnstruct ACStr.
	// "Attachment converted:" string.
	GetIniString(IDS_ATTACH_CONVERTED, ACStr, sizeof(ACStr));
	ACLen = strlen(ACStr);


	// Now the fun begins. Loop through the stub

	t = p = m_pStartBody;

	// bResult will be FALSE at the end only a write failed or some such grievous error.
	bResult = TRUE;
	BOOL bPreviousWritten = FALSE;
	
	while (t && *t && bResult)
	{
		// Is this an Attachment Converted line??
		if ( !strncmp(ACStr, t, ACLen) )
		{
			CString	szAttachmentPathname;
			short	tNext;

			// get the full path name to the attachment
			tNext = StripAttachmentName( t + ACLen, szAttachmentPathname, m_SrcAttachDir );

			// Note: tNext is non-negative;
			if( tNext > 0 )
			{
				// Check to see if the file exists before we commit to creating a MIME part.
				if ( FileExistsMT (szAttachmentPathname) )
				{
					// Ok. ReformatAttachment() will write a full body part. Let's
					// make the text we now have into a previous part.

					bResult = bPreviousWritten = AddTextBodyPart (p, t, &BodyFileWriter);

					// NOTE: Don't write the terminating boundary.
					// ReformatAttachment () will create a new body part.
					if (bResult)
					{
						bResult = ReformatAttachment (szAttachmentPathname, &BodyFileWriter);
					}
				}
			}
		}

		if (bResult)
		{
			// Skip to the end of the line.
			t = strchr (t, '\n');

			if (t)
				t++;

			// Did we write the previous text?
			if (bPreviousWritten)
			{
				p = t;
				// Reset this.
				bPreviousWritten = FALSE;
			}
		}

	} // while.

	// Did we succeed? There might be left over chars, write them.
	if (bResult && p && *p)
	{
		if (m_bIsMultipart)
		{
			bResult = AddTextBodyPart (p, NULL, &BodyFileWriter);
		}
		else
		{
			// Just Write it out.
			len = strlen ( p );

			if (len > 0)
			{
				bResult = BodyFileWriter.Write ( p, len );

				// Add a blank line if this doesn't end in one.
				if ( bResult &&  (p[len - 1] != '\n') && (p[len - 1] != '\n') )
					bResult = BodyFileWriter.Write ( "\r\n", 2 );
			}
		}
	}

	// Do we add the terminating MIME boundary?
	if (bResult && m_bIsMultipart)
	{
		// Append the terminating boundary cookie.
		BodyFileWriter.Write (m_Boundary, m_Boundary.GetLength());
		BodyFileWriter.Write ("--\r\n",4);
	}

	// Cleanup.
	BodyFileWriter.Close();

	return bResult;
}




// FUNCTION
// If we're not yet in MULTIPART mode, generate a boundary, append it to the
// file and add the text between p and t.
// Don't add a trailing boundary - the next part will do that.
// END FUNCTION
// NOTES
// t can be NULL, in which case, p must be nul-terminated.

// NOTE: We MUST wade through the given text and make sure lines end in \r\n!!!
// END NOTES

BOOL CImapAppend::AddTextBodyPart (LPCSTR p, LPCSTR t, CFileWriter *pBodyFileWriter)
{
	long	len;
	BOOL	bResult = FALSE;
	BOOL	bIsBlank = TRUE;

	// Sanity.
	if ( !(p && pBodyFileWriter))
		return FALSE;

	// If we have nothing to write, just return.
	if (t)
		len = (t - p);
	else
		len = strlen ( p );

	if (len <= 0)
		return TRUE;

	// If the text is all blank, don't bother.
	for (int i=0; i < len; i++)
	{
		if ( (p[i]  != ' ') && (p[i] != '\t') && (p[i] != '\r') && (p[i] != '\n') )
		{
			bIsBlank = FALSE;
			break;
		}
	}

	if (bIsBlank)
		return TRUE;

	// Have we already decided to go multipart?
	if (!m_bIsMultipart) 
	{
		m_bIsMultipart = TRUE;

	}
	
	// Generate a boundary.
	NewMimeBoundary ( m_Boundary );
					
	// Write the boundary.
	bResult = pBodyFileWriter->WriteTextLine (m_Boundary);

	// Write the Content-Type:Text/Plain header.
	CString szHeader;
	szHeader.Format ("%s%s/%s", CRString(IDS_MIME_HEADER_CONTENT_TYPE),
				 CRString(IDS_MIME_TEXT), CRString (IDS_MIME_PLAIN) );

	bResult = pBodyFileWriter->WriteTextLine (szHeader);
	if (!bResult)
		return FALSE;

	// Write blank line separator.
	bResult = pBodyFileWriter->Write ("\r\n", 2);

	// Write the text.
	if (bResult)
	{
		// Verify that lines end in \r\n. Yuck!!!
		// Carefull! p is NOT nul-terminated.

		long i =0;
		const char *q = p;

		while (bResult && i < len)
		{
			if (*q == '\r')
			{
				if (i < (len - 1))
				{
					q++; i++;

					if (*q != '\n')
					{
						// Write properly.
						bResult = pBodyFileWriter->Write( p, (long) (q-p));
						if (bResult)
							bResult = pBodyFileWriter->Write( "\n", 1);

						// Next iteration will look at this \r.
						p = q;
					}
					else
					{
						// This is a proper termination. Skip over \n.
						q++; i++;

						// However, if this was the last char, we have to write stuff out!!
						if ( i == len)
						{
							bResult = pBodyFileWriter->Write( p, (long) (q - p));
						}
					}
				}
				else
				{
					// Last char. Append \n.

					q++; i++;	// Carefull!! q now points beyond len!! But this will stop the while.

					bResult = pBodyFileWriter->Write( p, (long) (q - p));
					if (bResult)
						bResult = pBodyFileWriter->Write( "\n", 1);
				}
			}
			else if (*q == '\n')
			{
				// Means this wasn't preceeded by \r. Write out previous.

				bResult = pBodyFileWriter->Write( p, (long) (q - p));

				// Write \r\n and ignore the \n.
				if (bResult)
					bResult = pBodyFileWriter->Write( "\r\n", 2);

				// Skip over \n
				q++; i++;

				p = q;
			}
			else
			{
				// Ordinary letter: 
				q++; i++;

				// If this is the last char, write out and append \r\n
				if (i == len)
				{
					bResult = pBodyFileWriter->Write( p, (long) (q - p));
					if (bResult)
						bResult = pBodyFileWriter->Write( "\r\n", 2);
				}

			}
		} // While.
	} // If 

	return bResult;
}



BOOL CImapAppend::ReformatAttachment (LPCSTR pAttachmentPathname, CFileWriter *pBodyFileWriter)
{
	BOOL bResult = FALSE;
	long et;
	MIMEMap mm;

	// Sanity.
	if ( !(pAttachmentPathname && pBodyFileWriter))
		return FALSE;

	// Make sure the attachment file exists and we can read from it.
	DWORD Attributes = GetFileAttributes (pAttachmentPathname);
	if (Attributes != 0xFFFFFFFF)	// Exists.
	{
		// Make sure it's a file.
		if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			ErrorDialog(IDS_ERR_OPEN_ATTACH, pAttachmentPathname);
			return FALSE;
		}
	}

	// Have we already decided to go multipart?
	if (!m_bIsMultipart) 
	{
		m_bIsMultipart = TRUE;

		// Generate a boundary.
		NewMimeBoundary ( m_Boundary );
	}

	// Write the boundary.
	pBodyFileWriter->WriteTextLine ( m_Boundary );

	// Determine what encoding type we need.

	if ((et = APPENDEncodingType(pAttachmentPathname, mm)) < 0)
		return FALSE;
		
	if (!mm.m_Creator[0])
		strcpy(mm.m_Creator, CRString(et? IDS_CREATOR_TEXT : IDS_CREATOR_UNKNOWN));

	if (!mm.m_Type[0])
		strcpy(mm.m_Type, CRString(et? IDS_ATTACH_TEXT_TYPE : IDS_TYPE_UNKNOWN));
		
	if (AppendMIME(pAttachmentPathname, mm, et, pBodyFileWriter) < 0)
		return FALSE;

	return TRUE;
}




//
// FUNCTION
// Set up a "STRING" structure that will stream a message from a remote source 
// mailbox to another remote mailbox on a different server.
// END FUNCTION

// NOTE:
// (JOK - 11/14/97) Added "Seconds". If no Date: field found in
// the message header, use this (if non-zero) to create a Date: header.
// END NOTE

HRESULT CImapAppend::AppendMessageAcrossRemotes (BOOL InBackground /* = FALSE */)
{
	STRING	ms;
	HRESULT	hResult = E_FAIL;
	unsigned long nTotSize = 0;

	// Caller MUST have set these: 
	if (! (m_pSrcImap && m_pDstImap) )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Both mailboxes must be open and selected!
	if (! (m_pSrcImap->IsSelected() && m_pDstImap->IsSelected()) )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Initialize the STRING.
	memset ((void *)&ms, 0, sizeof (STRING));

	// Get the total size of the source message text.
	nTotSize = 0;
	m_pSrcImap->GetRfc822Size (m_SrcUid, &nTotSize);

	// The STRING driver is the StreamAppendDriver.
	ms.dtb = &StreamAppendDriver;

	// Set this append object as the STRING's data.
	ms.data = (void *)this;

	// Allocate buffer within the CImapApend object.
	m_nBufferSize	= 4096;
	m_pBuffer		= new TCHAR[m_nBufferSize + 4];

	if (!m_pBuffer)
		return FALSE;

	// No bytes read yet.
	m_nBytesRead	 = 0;
	
	// Initialize the STRING. THe STRING's init() function will set the full
	// data size, read the first chunk and set the chunksize.

	(ms.dtb)->init ( &ms, this, nTotSize);

	// Launch the APPEND.

	hResult = m_pDstImap->AppendMessage ( &m_ImapFlags, m_Seconds, &ms );

	// Free memory.
	delete[] m_pBuffer;

	m_pBuffer = NULL;
	m_nBufferSize = 0;

	return hResult;
}



// Create a new message ID and return it in "szBuffer".
//
void CImapAppend::NewMessageID (CString& szBuffer)
{
	TCHAR	szMyBuf[2048];  
	LPSTR	pPtr = NULL;
	CRString Version(IDS_VERSION);

	// Fill with nuls.
	memset ( (void *)szMyBuf, 0, sizeof(szMyBuf) );

	pPtr = szMyBuf;

	*pPtr++ = '<';

	LPSTR pVer = strchr(Version, ' ');
	if (pVer)
		pVer++;
	pVer = strchr (pVer, ' ');
	if (pVer)
		pVer++;

	if (pVer)
	{
		strcpy(pPtr, strchr(strchr(Version, ' ') + 1, ' ') + 1);

		pPtr += strlen (pPtr);
	}

	*pPtr++ = '.';

	time_t Seconds;

	if (m_Seconds)
		Seconds = m_Seconds;
	else
		Seconds = time(0);

	strcat (pPtr, ::FormatTimeMT(Seconds, "%Y%m%d%H%M%S"));

	pPtr += strlen (pPtr);

	*pPtr++ = '.';

	sprintf(pPtr, "%08lx", (DWORD)this);

	pPtr += strlen (pPtr);

	const char* At = strrchr(GetIniString(IDS_INI_POP_ACCOUNT), '@');
	if (At)
	{
		strcat (pPtr, At);

		pPtr += strlen (pPtr);
	}

	*pPtr++ = '>';

	// Return result in szBuffer;

	szBuffer = szMyBuf;
}





//===================== Utility functions ===============================/
// Most of these were derived from similar "sendmail.cpp" functions. 
//

// FUNCTION
// Generate a new unique MIME boundary string and write it to the given
// CString.
// END FUNCTION

void NewMimeBoundary ( CString& Boundary)
{
	char buf [ 128 ];

	sprintf (buf, CRString(IDS_MIME_BOUNDARY_FORMAT), time(0));

	// Copy to the CString.
	Boundary = buf;  
}



// This is taken from sendmail.cpp.

enum {
	ET_BINARY = 0,
	ET_ASCII,
	ET_QP
};



// EncodingType
//
// Reads over the file and does some manipulation on it to determine
// what encoding type is needed.
// Returns -1 for error, ET_BINARY for binary, ET_ASCII for regular text,
// ET_QP for quoted-printable, and if the message is being sent as BinHex
// the return is the number of LFs that need to be stripped
//
long APPENDEncodingType ( const char* fname, MIMEMap& mm )
{
	JJFileMT InFile;
	long ByteCnt = 0L;
	int QPable = FALSE;
	BOOL IsTypeText = FALSE;
	unsigned long NumCharsOnLine = 0;
	BYTE ch, LastCh;
	char* p;

	p = strrchr(fname, '.');
	if (p)
		mm.Find(p + 1);
	if (!strncmp(mm.m_Type, CRString(IDS_ATTACH_TEXT_TYPE), 4))
		IsTypeText = TRUE;
	else if (mm.m_Type[0])
	{
		// Assume binary on known mappings that aren't type TEXT
		return (ET_BINARY);
	}

	if ( !SUCCEEDED ( InFile.Open(fname, O_RDONLY) ) )
		 return (-1L);

	HRESULT hrGet = S_OK;
	for (LastCh = 0; SUCCEEDED(hrGet = InFile.Get((char *)&ch)) && (hrGet != S_FALSE); LastCh = ch, NumCharsOnLine++)
	{
		// Only count it when we see a CR-LF pair, since that's the
		// only time a character will get stripped
		if (ch == '\n' && LastCh == '\r')
		{
			ByteCnt++;
			if (NumCharsOnLine >= 1000)
				QPable = TRUE;
			NumCharsOnLine = 0;
			continue;
		}

		// Check for '\r' without '\n' - like in PDF files
		if (LastCh == '\r')
		{
			mm.m_Type[0] = 0;
			return (ET_BINARY);
		}

		// Check for binary data
		// User may think it's TEXT, but maybe it's not
//		if (!IsTypeText)
		{
			if (ch < 32 && ch != 0x0A && ch != 0x0D &&
				ch != 0x0C && ch != 0x09 && ch != 0x1A && ch != 0x04)
			{
				mm.m_Type[0] = 0;
				return (ET_BINARY);
			}
		}
		if (ch == 0x04 || ch & 0x80)
			QPable = TRUE;
	}
	if (NumCharsOnLine >= 1000)
		QPable = TRUE;

	// Check for '\r' without '\n' - like in PDF files
	if (LastCh == '\r')
	{
		mm.m_Type[0] = 0;
		return (ET_BINARY);
	}

	if (!mm.m_Mimetype[0] || !mm.m_Subtype[0])
	{
		GetIniString(IDS_MIME_TEXT, mm.m_Mimetype, sizeof(mm.m_Mimetype));
		GetIniString(IDS_MIME_PLAIN, mm.m_Subtype, sizeof(mm.m_Subtype));
	}
	GetIniString(IDS_ATTACH_TEXT_TYPE, mm.m_Type, sizeof(mm.m_Type));
	if (!mm.m_Creator[0])
		GetIniString(IDS_CREATOR_TEXT, mm.m_Creator, sizeof(mm.m_Creator));

	return (QPable? ET_QP : ET_ASCII);
}




// Taken from Sendmail.cpp;

class EncodeCopy : public Encoder
{
public:
	int Encode(const char* In, LONG InLen, char* Out, LONG &OutLen)
		{ memcpy(Out, In, InLen); OutLen = InLen; return (0); }
};





// 
// FUNCTION
// - Write the body's MIME header
// - Encode the file if necessary and append it.
// Patterned after AttachMIME() in sendmail.cpp.
// END FUNCTION

// NOTES
// This does it's own encoding. There is no encoder set in 
// the CFileWriter.
// END NOTES

int AppendMIME(const char* Filename, MIMEMap& mm, long EncoType, CFileWriter *pBodyFileWriter)
{
	Encoder* TheEncoder = NULL;
	JJFileMT	InFile;
	char*	FileBuf = NULL;
	LONG	NumRead;
	int		BlockSize;
	BOOL	IsQP = FALSE;
	const char* fname = strrchr(Filename, '\\');
	char	*buf;

	// New way of getting the buffer.
	InFile.GetBuf(&FileBuf);
	if (!FileBuf)
		return -1;

	// Allocate our buffer.
	buf = new char[lBufSize + 4];
	if (!buf)
		return -1;

	// JOK - The while is used just so we can go to the function's end easily.
	while (1)
	{
		if (fname)
			fname++;
		else
			fname = Filename;

		// If there are funny characters, check whether mostly text, so use QP
		if (EncoType == ET_ASCII)
		{
			TheEncoder = new EncodeCopy;
			if (!TheEncoder || AppendContentType(FALSE,0, pBodyFileWriter, &mm) < 0) break;
		}
		else
		{
			if (EncoType == ET_QP)
			{
				// SendContentType returns:
				//	-1 : error
				//	 0 : no QP
				//	 1 : QP
				int Result = AppendContentType(TRUE, 0, pBodyFileWriter, &mm);
				if (Result < 0) break;
				if (Result)
				{
					TheEncoder = new QPEncoder;
					IsQP = TRUE;
				}
				else
					TheEncoder = new EncodeCopy;
			}
			else
			{
				TheEncoder = new Base64Encoder;

				if (!TheEncoder) break;
		
				// If we don't have a full mime type/subtype, then assume application/octet-stream
				if (!mm.m_Mimetype[0] || !mm.m_Subtype[0])
				{
					GetIniString(IDS_MIME_APPLICATION, mm.m_Mimetype, sizeof(mm.m_Mimetype));
					GetIniString(IDS_MIME_OCTET_STREAM, mm.m_Subtype, sizeof(mm.m_Subtype));
				}
					
				// Content-Type
				CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
				CRString MIMEName(IDS_MIME_NAME);
				sprintf(buf, "%s %s/%s; %s=\"%s\"", (const char*)ContentType,
					mm.m_Mimetype, mm.m_Subtype, (const char*)MIMEName, fname);

				if ( !pBodyFileWriter->Write ( buf, strlen(buf) ) ) break;

				// Add Mac info if present
				if ((mm.m_Type[0] || mm.m_Creator[0]) &&
					(strcmp(mm.m_Type, CRString(IDS_TYPE_UNKNOWN)) ||
					strcmp(mm.m_Creator, CRString(IDS_CREATOR_UNKNOWN))))
				{
					if (!pBodyFileWriter->WriteTextLine(";")) break;

					CRString MacType(IDS_MIME_MAC_TYPE);
					CRString MacCreator(IDS_MIME_MAC_CREATOR);
					sprintf(buf, " %s=\"%02X%02X%02X%02X\"; %s=\"%02X%02X%02X%02X\"",
						(const char*)MacType, mm.m_Type[0], mm.m_Type[1], mm.m_Type[2], mm.m_Type[3],
						(const char*)MacCreator, mm.m_Creator[0], mm.m_Creator[1], mm.m_Creator[2], mm.m_Creator[3]);
					if ( !pBodyFileWriter->Write(buf, strlen (buf)) ) break;
				}

				if ( !pBodyFileWriter->WriteTextLine(NULL)) break;
				
				// Content-Transfer-Encoding
				CRString ContentEncoding(IDS_MIME_HEADER_CONTENT_ENCODING);
				CRString CTEName(IDS_MIME_BASE64);
				sprintf(buf, "%s %s", (const char*)ContentEncoding, (const char*)CTEName);

				if ( !pBodyFileWriter->WriteTextLine(buf) ) break;
			}
		}
		
		// (JOK - always set displostion as attachment!!
		// Content-Disposition
		//
		CRString ContentDisp(IDS_MIME_HEADER_CONTENT_DISP);
		CRString ContentDispAttach(IDS_MIME_CONTENT_DISP_ATTACH);
		CRString ContentDispFilename(IDS_MIME_CONTENT_DISP_FILENAME);
		sprintf(buf, "%s %s; %s=\"%s\"",
				(const char*)ContentDisp, (const char*)ContentDispAttach,
				(const char*)ContentDispFilename, fname);
		if ( !pBodyFileWriter->WriteTextLine(buf) ) break;
		
		// Blank line between MIME header and body
		if ( !pBodyFileWriter->WriteTextLine(NULL) ) break;
		
		// Put out beginning bytes.  Put filename in buffer for those encodings that need it.
		strcpy(buf, fname);
		NumRead = 0;
		int flag = IsQP ? ENCODE_QP : ENCODE_NONE;
		if (TheEncoder && TheEncoder->Init(buf, NumRead, flag)) break;
		
		// Init CharsOnLine variable of SendDataText()
		AppendDataText(NULL, pBodyFileWriter, -1, 0);
		
		if (AppendDataText(buf, pBodyFileWriter, NumRead) < 0) break;
		
		// Now on to the attachment
		if (!SUCCEEDED ( InFile.Open(Filename, O_RDONLY) ) ) break;
		if (IsQP)
		{
			char* buf2 = new char[lBufSize * 3];
			if (!buf2) break;
			buf[lBufSize - 1] = 0;
			ASSERT( lBufSize > 0 && lBufSize < 32767L );

			// New way of reading (JOK)
			NumRead = 0;

			while (SUCCEEDED ( InFile.RawRead(buf, (int)(lBufSize - 1), &NumRead) ) && (NumRead > 0) )
			{
				LONG OutLen;
				TheEncoder->Encode(buf, NumRead, buf2, OutLen);
				if (AppendDataText(buf2, pBodyFileWriter, OutLen) < 0)
				{
					delete[] buf2;
					NumRead = -1;
					break;
				}
				ProgressAdd(NumRead);
			}
			delete[] buf2;
			if (NumRead < 0) break;
		}
		else
		{
			BlockSize = lBufSize;
			if (EncoType != ET_ASCII)
				BlockSize /= 2;
			
			// New way of doing this (JOK)
			while ( SUCCEEDED (InFile.GetNextBlock(&NumRead)) && (NumRead > 0))
			{
				for (int i = 0; i < NumRead; i += BlockSize)
				{
					LONG InLen = min(BlockSize, NumRead - i);
					LONG OutLen;
					TheEncoder->Encode(FileBuf + i, InLen, buf, OutLen);
					if (AppendDataText(buf, pBodyFileWriter, OutLen) < 0)
						NumRead = -1;
					else
						ProgressAdd(InLen);
				}
				if (NumRead < 0)
					break;
			}
			if (NumRead < 0)
			{
				ErrorDialog(IDS_ERR_BINHEX_READ, Filename);
				break;
			}
		}
		
		// Finish up encoding and write out remaining bytes
		if (TheEncoder->Done(buf, NumRead)) break;
		if (AppendDataText(buf, pBodyFileWriter, NumRead) < 0) break;
		
		delete TheEncoder;

		// JOK
		delete[] buf;

		return (1);
	}

	delete TheEncoder;

	// JOK
	delete[] buf;

	return (-1);
}




// AppendContentType
// deduce and send the appropriate content-type
// (and CTE) for two blocks of text (body and signature, typically)
//	text1 - one block of text (may be null)
//  text2 - second block of text (may be null)
//  tableID - xlate table id
// Returns -1 for error, 0 for normal text, 1 for QP text

int AppendContentType(short tableID, WORD flag, CFileWriter *pBodyFileWriter, MIMEMap* mm /* = NULL */)
{
	short etid = tableID;
	char scratch[64];
	char buf[1024];

	if (!pBodyFileWriter)
		return -1;
	
	if ( UseQPToAppend(NULL, NULL, etid, ( flag & MSF_XRICH ) ? TRUE : FALSE ) )
	{
		etid = 1;
	}

	NameCharset(scratch, etid);
	
	 // send the content type
	CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
	CRString MimeText(IDS_MIME_TEXT);
	CRString MimePlain(IDS_MIME_PLAIN);
	CRString MimeEnriched(IDS_MIME_ENRICHED);

	const char* Type = MimeText;
	const char* Subtype;
#ifdef WIN32
	if (flag & MSF_XRICH )
		Subtype = MimeEnriched;
	else
#endif
		Subtype = MimePlain;	

	if (mm)
	{
		Type = mm->m_Mimetype;
		Subtype = mm->m_Subtype;
	}
	sprintf(buf, "%s %s/%s%s", (const char*)ContentType, Type, Subtype, scratch);

	if ( !pBodyFileWriter->WriteTextLine(buf)) return (-1);

	if (!etid)
		return (0);

	// content-transfer-encoding
	// JOK : Do QP.
	CRString ContentEncoding(IDS_MIME_HEADER_CONTENT_ENCODING);
	CRString CTEName(IDS_MIME_QP);
	sprintf(buf, "%s %s", (const char*)ContentEncoding, (const char*)CTEName);
	if ( !pBodyFileWriter->WriteTextLine(buf)) return (-1);

	return (1);
}



// SendDataText
// Writes out text during the DATA command.  Need to check for things
// like leading periods, and very long lines (> 1000 chars).
//
// text: text to be sent
// length: length of the text (-1 if NULL terminated)
// InitCharsOnLine: initializes the CharsOnLine static variable
//                  (-1 means no initialization)
//
// Returns -1 on error, positive otherwise
//

int AppendDataText (const char* text, CFileWriter *pBodyFileWriter, int length /*  = -1 */, int InitCharsOnLine /* = -1 */)
{
	// BUG:: JOK - This is NOT re-entrant!!
	static int CharsOnLine = 0;

	if (!pBodyFileWriter)
		return -1;

	if (InitCharsOnLine != -1)
		CharsOnLine = InitCharsOnLine;

	if (!text || !*text)
		return (1);

	if (length == -1)
		length = strlen(text);

	while (length)
	{
		const char* start = text;

		// Leading periods must be quoted, i.e. an extra period must be added
		if (!CharsOnLine && *text == '.')
		{
			if (!pBodyFileWriter->Write(".", 1))
				goto fail;
		}

		int count = 0;
		const int MagicLength = 995;
		while (length && *text != '\n' && count < MagicLength)
		{
			text++; length--; count++;
		}
		CharsOnLine = count;

		if (length)
		{
			// Special case of truncating a long line at a CR, let it pass
			if (count == MagicLength && *text == '\r')
			{
				text++; length--; count++;
			}
			if (*text == '\n')
			{
				CharsOnLine = 0;
				text++; length--; count++;
			}
		}

		// Send what we got
		if (!pBodyFileWriter->Write (start, count)) goto fail;

		// Line was too long, so insert newline to break it up
		if (count >= MagicLength && text[-1] != '\n')
		{
			if (!pBodyFileWriter->WriteTextLine(NULL)) goto fail;
			CharsOnLine = 0;
		}
	}

	return (1);

fail:
	return (-1);
}



// UseQP
// Should quoted printable encoding be done?
BOOL UseQPToAppend( const char* text1, const char* text2, short etid, BOOL bIsRich )
{
	if(	etid || 
		AnyFunny(text1) || 
		AnyFunny(text2) ||
		(	(	bIsRich  ) &&
			(	LongerThan(text1, 1000) || LongerThan(text2, 1000) ) ) )
	{
		return (TRUE);
	}
	
	return (FALSE);
}



// AppendMIMEMessageHeaders
// Append the requisite MIME headers to the open file.
// If Boundary is non-NULL, write the contentype header as well.
// Return -1 if error, else 0.
//
int AppendMIMEMessageHeaders(LPCSTR pBoundary, int flag, JJFileMT *pOpenFile)
{
	char buf [2048];

	// NOTE: pBoundary CAN be NULL.
	if (! pOpenFile )
		return -1;

	if (S_OK != pOpenFile->IsOpen())
		return -1;

	if (FAILED ( pOpenFile->Put(CRString(IDS_MIME_HEADER_VERSION)) )  ) return (-1);
	if (FAILED ( pOpenFile->Put(' ') ) ) return (-1);
	if (FAILED ( pOpenFile->PutLine(CRString(IDS_MIME_VERSION)) ) ) return (-1);
	
	// Write these if we have a boundary.
	if (pBoundary)
	{
		// Build Content-Type and append Multipart/Mixed.
		sprintf(buf, "%s %s/%s; %s=\"%s\"",
			(const char*)CRString(IDS_MIME_HEADER_CONTENT_TYPE),
			(const char*)CRString(IDS_MIME_MULTIPART),
			(const char*)CRString(IDS_MIME_MIXED),
			(const char*)CRString(IDS_MIME_BOUNDARY),
			pBoundary + 4);
		if (FAILED (pOpenFile->PutLine(buf) ) ) return (-1);
		return (0);
	}

	return 0;
}


BOOL IsContinuationChar (TCHAR c)
{
	return (c == ' ' || c == '\t');
}



BOOL IsHeaderToInclude (LPCSTR t)
{
	return ( (strnicmp ( t, (LPCSTR)MimeHeaderPrefix, MimeHeaderPrefix.GetLength() ) != 0) &&
			 (strnicmp ( t, (LPCSTR)XuidHeader, XuidHeader.GetLength() ) != 0) &&
			 (strnicmp ( t, (LPCSTR)MimeVersion, MimeVersion.GetLength() ) != 0) &&
			 (strnicmp ( t, (LPCSTR)XImflagsHeader, XImflagsHeader.GetLength() ) != 0) );
}



BOOL ContainsNonAsciiChar (LPCSTR t)
{
	// If this is a Bcc header and it contains the weird FCC non-ascii character,
	// ignore it.
	//
	LPCSTR s = t;

	while ( s && *s && (*s != '\r') && (*s != '\n') )
	{
		if ((unsigned char)*s > 127)
		{
			return TRUE;
		}

		s++;
	}

	return FALSE;
}



#endif // IMAP4
