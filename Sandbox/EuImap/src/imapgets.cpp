/////////////////////////////////////////////////////////////////////////////
// 	IMAPGETS.cpp - The CMailGets class imlementation. 


#include "stdafx.h"


#ifdef IMAP4 // Only for IMAP.

#include "resource.h"
#include "password.h"
#include "rs.h"
#include "fileutil.h"
#include "Base64.h"
#include "QP.h"
#include "ImHexbin.h"
#include "ImSingle.h"
#include "ImUudecode.h"
#include "progress.h"

// JOK
#include "ImapExports.h"
#include "imap.h"
#include "imapdlg.h"
#include "imapgets.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "imapfol.h"
#include "imapacct.h"
#include "ImapTypes.h"
// END JOK

// ========== MACROS ===============

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// ========================================================================//



//	================================================================================
//	================================================================================
//	================================================================================
//		IMAPGETS Mail Service Object
//	================================================================================
//	================================================================================
//	================================================================================

//	================================================================================
//////////////////////////////////////////////////////////////////////////////


//========== Implementation of CStringWriter ===================/

CStringWriter::CStringWriter()
{
	m_Buffer.Empty();
	m_CurrentSize = 0;
	m_FullSize = 0;

	// The buffer is allocated during the Write.
	m_nBufferSize	= 0;
	m_pReadBuffer	= NULL;
}

CStringWriter::~CStringWriter()
{
	// Free data
	if (m_pReadBuffer)
	{
		delete[] m_pReadBuffer;
		m_pReadBuffer = NULL;
	}
}


//
// Manually add a set of butes to the internal CString.
//
BOOL CStringWriter::Write (const char *buffer, ULONG nBytes)
{
	BOOL bResult = TRUE;

	if (buffer)
	{
		CString buf(buffer, (int)nBytes);

		// Append to internal.
		m_Buffer += buf;

		// Set length also.
		m_CurrentSize = m_Buffer.GetLength();
	}
	else
		bResult = FALSE;

	return bResult;
}




//
// Method to snarf data into a CString.
//
BOOL CStringWriter::Write (readfn_t readfn, void * read_data, unsigned long size)
{
	BOOL	bResult = FALSE;

	// Sanity:
	if (! (readfn && read_data) )
		return FALSE;

	// Can have a zero size.
	if (size < 0)
		return FALSE;

	// Nothing to read?
	if (size == 0)
		return TRUE;

	// Was our read buffer allocated?
	if (!m_pReadBuffer)
	{
		m_pReadBuffer = new char[BUFLEN + 4];
		if (m_pReadBuffer)
			m_nBufferSize = BUFLEN;
	}

	// Did we get a buffer??
	if (!m_pReadBuffer)
		return FALSE;

	// Set the current and max size of the data.
	m_CurrentSize	= 0;
	m_FullSize		= (long)size;

	// We can now read in chunks.
	long i = 0;
	long inLen;

	while (i < m_FullSize)
	{
		// NUmber of bytes to read..
		inLen = min( m_nBufferSize, (m_FullSize - i));

		// Read now.
		if (!(*readfn)(read_data, inLen, m_pReadBuffer))
			break;

		// Append to internal CString.
		m_Buffer += CString (m_pReadBuffer, inLen);

		// Set length also.
		m_CurrentSize = m_Buffer.GetLength();

		// Next chunk.
		i += inLen;
	}
	
	// Check if we got all the data.
	if (i == m_FullSize )
		bResult = TRUE;

	if (!bResult)
	{
		m_Buffer.Empty();
		m_CurrentSize = 0;
		m_FullSize = 0;

	}

	return bResult;
}




// Interface to caller:	
LPCSTR CStringWriter::GetString()
{
	return (LPCSTR)m_Buffer;
}



//////////////////////////////////////////////////////////////
// If the string ends with a trailing blank line, remove it.
//////////////////////////////////////////////////////////////
void CStringWriter::RemoveTrailingBlankLine ()
{
	long len = m_Buffer.GetLength();

	// Lines MUST be terminated by "\r\n". So a blank line means two sets of
	// consecutive "\r\n"
	if (len < 4)
	{
		// Impossible then.
		return;
	}

	// Get the last 4 chars.
	CString szSub = m_Buffer.Right (4);
	if (szSub.Compare ("\r\n\r\n") == 0)
	{
		// Leave the first end of line.
		m_Buffer = m_Buffer.Left (len - 2);
	}
}






//========== Implementation of CFileWriter ===================/

CFileWriter::CFileWriter()
{
	m_CurrentSize = 0;
	m_FullSize = 0;
	m_Filename.Empty();

	m_Decoder = NULL;

	// The buffer is allocated during the Write.
	m_nBufferSize	= 0;
	m_pReadBuffer	= NULL;
	m_pWriteBuffer	= NULL;

	m_bShowProgress = FALSE;
}

CFileWriter::CFileWriter(const char *Filename)
{
	m_CurrentSize = 0;
	m_FullSize = 0;
	m_Filename.Empty();
	m_Decoder = NULL;

	m_bMustReadSingleLines = FALSE;

	if (Filename)
		m_Filename = Filename;

	// The buffer is allocated during the Write.
	m_nBufferSize	= 0;
	m_pReadBuffer	= NULL;
	m_pWriteBuffer	= NULL;

	m_bShowProgress = FALSE;
}


CFileWriter::~CFileWriter()
{
	// Close file if still open.
	if (m_Jfile.IsOpen() == S_OK)
		m_Jfile.Close();

	if (m_Decoder)
		delete m_Decoder;

	// Free buffer.
	if (m_pReadBuffer)
	{
		delete[] m_pReadBuffer;
		m_pReadBuffer = NULL;
	}

	// Free buffer.
	if (m_pWriteBuffer)
	{
		delete[] m_pWriteBuffer;
		m_pWriteBuffer = NULL;
	}
}





// FUNCTION
// Open the file for writing
// END FUNCTION

BOOL CFileWriter::Open ()
{
	// If file is not yet open, do so now.
	if (m_Jfile.IsOpen() != S_OK)
	{
		m_Jfile.Open((LPCSTR)m_Filename, O_CREAT | O_WRONLY);
	}

	return m_Jfile.IsOpen() == S_OK;
}


// FUNCTION
// Close the file.
// END FUNCTION

BOOL CFileWriter::Close ()
{
	// If file is not yet open, do so now.
	if (m_Jfile.IsOpen() == S_OK)
	{
		m_Jfile.Close();
	}

	return (m_Jfile.IsOpen() != S_OK);
}


//
// Manually add a set of bytes to the file.
//
BOOL CFileWriter::Write (const char *buffer, ULONG nBytes)
{
	long outLen = 0;
	BOOL bResult = TRUE;
		
	// Sanity. 
	if ((buffer == NULL) || (nBytes <= 0))
		return TRUE;

	// Was our read buffer allocated?
	if (!m_pReadBuffer)
	{
		m_pReadBuffer = new char[BUFLEN + 4];
		if (m_pReadBuffer)
			m_nBufferSize = BUFLEN;
	}

	// If file is not yet open, do so now.
	if (m_Jfile.IsOpen() != S_OK)
	{
		m_Jfile.Open((LPCSTR)m_Filename, O_CREAT | O_WRONLY);
	}

	if (m_Jfile.IsOpen() == S_OK)
	{
		// Decode if we need to.
		if (m_Decoder)
		{
			long inLen;

			// Just in case we are passed more bytes than we can handle.
			while (bResult && (nBytes > 0))
			{
				inLen = min (m_nBufferSize, (long)nBytes);
				strncpy (m_pReadBuffer, buffer, inLen);

				// Note: "outLen" is passed as a reference so it can be modified! See
				// declaration of Decoder::Decode()
				m_Decoder->Decode (m_pReadBuffer, inLen, m_pReadBuffer, outLen);
				if ( SUCCEEDED ( m_Jfile.Put (m_pReadBuffer, outLen) ) )
					nBytes -= inLen;
				else
				{
					// We should have some way of backing out of this if we fail after
					// a partial write.
					bResult = FALSE;
					break;	
				}
			}
		}
		else
		{
			// We can do one big write.
			bResult = SUCCEEDED ( m_Jfile.Put (buffer, nBytes) );
		}
	}

	return (bResult);
}




// NOTES
// If we have a decoder set, do a decode and write.
// END NOTES
BOOL CFileWriter::Write (readfn_t readfn, void * read_data, unsigned long size)
{
	long outLen = 0;
	BOOL bResult = TRUE;
	long TotalTranslatedBytes = 0;	// Size of resulting file.

	// Sanity:
	if (! (readfn && read_data) )
		return FALSE;

	// Can have a zero size.
	if (size < 0)
		return FALSE;

	// Nothing to read?
	if (size == 0)
		return TRUE;

	// Was our read buffer allocated?
	if (!m_pReadBuffer)
	{
		m_pReadBuffer = new char[BUFLEN + 4];
		if (m_pReadBuffer)
			m_nBufferSize = BUFLEN;
	}

	// Did we get a buffer??
	if (!m_pReadBuffer)
		return FALSE;

	// Initialize:
	*m_pReadBuffer = 0;

	// Was our write buffer allocated?
	if (!m_pWriteBuffer)
	{
		m_pWriteBuffer = new char[BUFLEN + 4];
	}

	// Did we get a buffer??
	if (!m_pWriteBuffer)
		return FALSE;

	// Nothing in the buffer yet.
	*m_pWriteBuffer = 0;


	// Set the current and max size of the data.
	m_CurrentSize	= 0;
	m_FullSize		= size;

	// Make sure to initialize the decoder, if any.
	if (m_Decoder)
		m_Decoder->Init();

	// If file is not yet open, do so now.
	if (m_Jfile.IsOpen() != S_OK)
	{
		m_Jfile.Open((LPCSTR)m_Filename, O_CREAT | O_WRONLY);
	}

	if (m_Jfile.IsOpen() == S_OK)
	{
		// Truncate the file to zero bytes:
		m_Jfile.ChangeSize (0);

		// Put up progress bar?? Only if one is already up!!
		// Caller must set these internal flags it they want this.
		// We only handle the guage here.
		//
		if ( InProgress && m_bShowProgress )
		{
			Progress(0, NULL, size);
		}

		// Read and decode in chunks.
		long inLen;
		long nBytesHandled = 0;
		LPSTR pBuf = NULL;

		// Instantiate our chunk reader:
		CChunkReader chunkReader (readfn, read_data, size, m_bMustReadSingleLines);

		bResult = TRUE;

		// Read in chunks..
		while ( bResult && (nBytesHandled < m_FullSize) )
		{
			inLen = 0;

			if (!chunkReader.GetNextChunk (&pBuf, &inLen))
			{
				bResult = FALSE;
				break;
			}

			if (!pBuf)
			{
				bResult = FALSE;
				break;
			}

			// Decode if we need to.
			// Note: "outLen" is passed as a reference so it can be modified! See
			// declaration of Decoder::Decode()

			// If we didn't get any chars, a grievous error:
			if (inLen <= 0)
			{
				ASSERT (0);
				bResult = FALSE;
				break;
			}

			// Update bytes we handled.
			nBytesHandled += inLen;

			// Decode if we need to.
			// Note: "outLen" is passed as a reference so it can be modified! See
			// declaration of Decoder::Decode()

			outLen = inLen;

			if (m_Decoder)
			{
				m_Decoder->Decode (pBuf, inLen, m_pWriteBuffer, outLen);

				// pBuf now points to out buffer:
				pBuf = m_pWriteBuffer;
			}
			else
			{
				// Write what pBuf points to!!
				//
				outLen = inLen;
			}

			// Write to the file.
			if (outLen > 0)
			{
				// Keep track of total bytes written:
				TotalTranslatedBytes += outLen;

				if (! SUCCEEDED ( m_Jfile.Put (pBuf, outLen) ) )
				{
					// We should have some way of backing out of this if we fail after
					// a partial write.
					//
					bResult = FALSE;
					break;	
				}

				// Update counter.
				//
				if (InProgress && m_bShowProgress)
				{
					ProgressAdd(inLen);
				}
			}
		}

		// If we succeeded, flush the file.
		if ( bResult && (nBytesHandled > 0) )
			m_Jfile.Flush();
	}

	return (bResult);
}





// FUNCTION
// Write the given text string followed by "\r\n"
BOOL CFileWriter::WriteTextLine (const char *buffer /* = NULL */)
{
	BOOL bResult = FALSE;

	// If file is not yet open, do so now.
	if (S_OK != m_Jfile.IsOpen())
	{
		m_Jfile.Open((LPCSTR)m_Filename, O_CREAT | O_WRONLY);
	}

	if (m_Jfile.IsOpen() == S_OK)
	{
		bResult = TRUE;

		if (buffer)
		{
			bResult = SUCCEEDED ( m_Jfile.Put (buffer, strlen (buffer)) );
		}

		if (bResult)
			bResult = SUCCEEDED ( m_Jfile.Put ("\r\n", 2) );
	}

	return bResult;
}


void CFileWriter::SetFilename (const char *filename)
{
	if (filename)
	{
		m_Filename = filename;
	}
}


// Interface to caller:	
LPCSTR CFileWriter::GetFilename()
{
	return (LPCSTR)m_Filename;
}


void CFileWriter::SetDecoder (unsigned short encoding)
{
	// If one exists, delete it.
	if (m_Decoder)
		delete m_Decoder;
	m_Decoder = NULL;

	// Reset this:
	m_bMustReadSingleLines = FALSE;

	switch (encoding)
	{
		case ENCBASE64:
			m_Decoder = new Base64Decoder;
			break;

		case ENCQUOTEDPRINTABLE:
			m_Decoder = new QPDecoder;
			break;

		case ENCUUENCODE:
			m_Decoder = new ImapDecodeUU;

			// Must set this to TRUE:
			m_bMustReadSingleLines = TRUE;

			break;

		case ENCBINHEX:
			m_Decoder = new ImapDecodeBH;
			
			// Must set this to TRUE:
			m_bMustReadSingleLines = TRUE;

			break;
		
		default:
			// Just stream as is.
			m_Decoder = NULL;
			break;
	}
}




// Special case for application/applefile - yuck.
// 
void CFileWriter::SetApplefileDecoder (unsigned short encoding)
{
	// Get a new AppleSingle decoder and set is as the main decoder.
	//
	ImapDecodeAS *pDc = new ImapDecodeAS;

	m_Decoder = pDc;

	// Set "secondary" decoder based of "encoding"
	//
	if (pDc)
	{
		// If we've got a secondary encoding:
		switch (encoding)
		{
			case ENCBASE64:
				{
					Base64Decoder *pSdc = new Base64Decoder;
						pDc->SetSecondaryDecoder (pSdc);
				}
				break;

		}
	}
}



// Delete any decoder.
void CFileWriter::ResetDecoder ()
{
	// If one exists, delete it.
	if (m_Decoder)
		delete m_Decoder;
	m_Decoder = NULL;

	m_bMustReadSingleLines = FALSE;
}



//================== Implementation of the CImapLogin class. =============/

CImapLogin::CImapLogin ()
{
	m_pAccount = NULL;
}


// Do the real login-in.
BOOL CImapLogin::Login(char *user, char *pwd, long trial)
{

#if 0 // No longer used.

	CString Login, Password;
	BOOL	GETFROMUSER = FALSE;
	BOOL	GOTLOGIN = FALSE;

	// MUST be called in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// Initialize
	Login.Empty();
	Password.Empty();


	// Always get and copy login.
	if (m_pAccount)
	{
		m_pAccount->GetLogin(Login);
	}

	// On first entry, "trial" is 0. THe IMAP DLL allows 3 trials.
	if ( (trial > 0) || !m_pAccount)
	{
		GETFROMUSER = TRUE;
	}

	// Try to determine our password.
	if (!GETFROMUSER)
	{
		if (m_pAccount)
		{
			// Get stored password, if any.
			m_pAccount->GetPassword(Password);
		}

		// If no password, go get any password stored in the INI file.
		if (Password.IsEmpty())
			m_pAccount->GetSavedPassword (Password);
	}

	// If we still don't have a password, go get from user.
	if ( Password.IsEmpty() )
		GETFROMUSER = TRUE;
	else
	{
		strcpy (user, (LPCSTR)Login);
		strcpy (pwd, (LPCSTR)Password);
	}

	// Do we have to put up dialog box??
	if (GETFROMUSER)
	{
		CString szAddress; m_pAccount->GetAddress(szAddress);
		CString szPrompt;
		szPrompt.Format ("%s@%s", Login, szAddress);

		GOTLOGIN = GetPassword(Password, 0, FALSE, szPrompt);

		// If we got login and password strings.
		if (GOTLOGIN)
		{
			// Copy to out parms.
			strcpy (user, (LPCSTR)Login);
			strcpy (pwd, (LPCSTR)Password);

			// Save for future use.
			if (m_pAccount)
			{
				m_pAccount->SetPassword ((LPCSTR)Password);

				// If user wants to save to disk, call this as well.
				if (::IsMainThreadMT())
				{
					int SavePass = GetIniShort(IDS_INI_SAVE_PASSWORD);
					if (SavePass)
						m_pAccount->SavePasswordToDisk ();
				}
			}
		}
		else
		{
			// User cancelled. Make sure these are empty.
			Login.Empty();
			Password.Empty();

			// Clear out parms..
			strcpy (user, (LPCSTR)Login);
			strcpy (pwd, (LPCSTR)Password);
		}
	}
#endif


	return TRUE;
}



//========================= CMboxLister implementation ====================/
CMboxLister::CMboxLister()
{
	m_pAccount = NULL;
}


CMboxLister::CMboxLister(CImapAccount *pAccount)
{
	if (pAccount)
		m_pAccount = pAccount;
	else
		m_pAccount = NULL;
}


// Call the account's "insert" routine to add this mailbox.
BOOL CMboxLister::AddMailbox (char *name, int delimiter, long attributes)
{
	if (m_pAccount == NULL)
		return FALSE;

	// Call the account's "insert" routine.
	return m_pAccount->Insert (name, (TCHAR)delimiter, attributes, IMAP_MAILBOX) >= 0;
}





//========================= CImapNodeLister implementation ====================/
//
CImapNodeLister::CImapNodeLister(ACCOUNT_ID AccountId)
{
	// Must have an account ID.
	m_AccountID			= AccountId;

	// Initialize these.
	m_curLevel			= 0;
	m_curReference.Empty();			// Used in Insert().
	m_curParentMailbox	= NULL;

	// Public:
	m_pTopMailbox	= NULL;		// They all get added to theis list.

	// Set this to point to the address of m_pTopMailbox initially.
	m_curMailboxList = &m_pTopMailbox;
}



CImapNodeLister::~CImapNodeLister()
{
	if (m_pTopMailbox)
	{
		DeleteFolderSubtree (m_pTopMailbox);
		m_pTopMailbox = NULL;
		m_curMailboxList = &m_pTopMailbox;
		m_curReference.Empty();
		m_curLevel = 0;
	}
}



// Create a new QCImapMailboxComamnd object and append to the list;
BOOL CImapNodeLister::AddMailbox (char *name, int delimiter, long attributes)
{
	// BUG: Should determine from name if this is an IMAP_MAILBOX or an IMAP_NAMESPACE.
	return ( Insert(name, (TCHAR)delimiter, attributes, IMAP_MAILBOX) > 0 );
}		



// Insert [PRIVATE]
// Insert a new mailbox into the mailbox tree, at the
// correct level and in the correct order.
// Return the position (in the sibling list at that level) at which it is inserted, or -1.
// 
int CImapNodeLister::Insert(const char *mailbox, TCHAR delimiter, long attributes, ImapMailboxType Type)
{
	short i;
	ImapMailboxNode *pFolder = NULL, *pPrevFolder = NULL;
	CString		str;

	// Sanity
	ASSERT (mailbox != NULL);
	// m_curMailboxList should NOT be NULL!!
	ASSERT (m_curMailboxList != NULL);

	// If the name is the same as the current reference, ignore it.
	if ( m_curReference.Compare (mailbox) == 0)
		return (-1);

	// Ok. Add folder...

	// Create a new ImapFolder structure and insert it into the list at this level.
	pFolder = NewImapMailboxNode();

	if (pFolder)
	{
		// Level and type.
		pFolder->level			= m_curLevel;
		pFolder->Type			= Type;
		pFolder->Delimiter		= delimiter;
		pFolder->AccountID		= m_AccountID;
		pFolder->pParentMailbox = m_curParentMailbox;

		// m_ImapName is a copy of mailbox.
		pFolder->pImapName = new char[strlen(mailbox) + 1];
		if (pFolder->pImapName)
		{
			strcpy (pFolder->pImapName, mailbox);
		}

		pFolder->NoInferiors	= (attributes & LATT_NOINFERIORS) > 0;
		pFolder->NoSelect		= (attributes & LATT_NOSELECT) > 0;
		pFolder->Marked			= (attributes & LATT_MARKED) > 0;
		pFolder->UnMarked		= (attributes & LATT_UNMARKED) > 0;

		// Make sure.						
		pFolder->SiblingList	= pFolder->ChildList = NULL;

		// pFolder->HasUnread   = ?? Won't know this until the folder is selected.
	}

	// Validity check.
	if (!pFolder)
	{
		return (-1);
	} 
	// MUST have ImapName!!
	else if (!(pFolder->pImapName))
	{
		delete pFolder;
		return (-1);
	}
	else
	{
		// Insert this in correct order.
		if (!*m_curMailboxList)
		{
			*m_curMailboxList = pFolder;
			i = 0;		// First position, zero based.
		}
		else if (strcmp ((*m_curMailboxList)->pImapName, pFolder->pImapName) > 0)
		{
			pFolder->SiblingList = *m_curMailboxList;
			*m_curMailboxList = pFolder;
			i = 0;		// First position, zero based.
		}
		else
		{
			pPrevFolder = *m_curMailboxList;
			i = 1;
			while (pPrevFolder->SiblingList)
			{
				if (strcmp (pPrevFolder->SiblingList->pImapName, pFolder->pImapName) > 0)
				{
					pFolder->SiblingList = pPrevFolder->SiblingList;
					pPrevFolder->SiblingList = pFolder;
					break;
				}
				else
				{
					pPrevFolder = pPrevFolder->SiblingList;
					i++;
				}
			}

			// Append to end of list?
			if (!pPrevFolder->SiblingList)
				pPrevFolder->SiblingList = pFolder;
		}

		return i;
	}
}






//======================================= CImapEltWriter class =========================//

CImapEltWriter::CImapEltWriter(CPtrUidList *pUidList)
{
	m_pUidList = pUidList;
}


CImapEltWriter::~CImapEltWriter()
{
}


// Must override this.
BOOL CImapEltWriter::SetFlag (	unsigned long uid,
								unsigned int seen,
								unsigned int deleted,
								unsigned int flagged,
								unsigned int answered,
								unsigned int draft,
								unsigned int recent)
{
	// Must have an internal CPtrUidList.
	if (m_pUidList)
	{
		m_pUidList->OrderedInsert (uid, seen, deleted, flagged, answered, draft, recent, TRUE);
		return TRUE;
	}

	return FALSE;
}



//======================================= CImapEltWriterMap class =========================//

CImapEltWriterMap::CImapEltWriterMap(CUidMap *pUidMap)
{
	m_pUidMap = pUidMap;
}


CImapEltWriterMap::~CImapEltWriterMap()
{
}


// Must override this.
BOOL CImapEltWriterMap::SetFlag (	unsigned long uid,
								unsigned int seen,
								unsigned int deleted,
								unsigned int flagged,
								unsigned int answered,
								unsigned int draft,
								unsigned int recent)
{
	// Must have an internal CPtrUidList.
	if (m_pUidMap)
	{
		return m_pUidMap->OrderedInsert (uid, seen, deleted, flagged, answered, draft, recent, TRUE);
	}

	return FALSE;
}





//====================== CFileReader class ====================/
// This class reads a file in chunks.


//====================== CFileReader class ====================/
// This class reads a file in chunks.

CFileReader::CFileReader(LPCSTR pFilepath)
{
	*m_buffer = 0;
	m_TotalSize = 0;

	// Set thye full path of the file.
	m_Filepath = pFilepath;

	// File not yet open.
	m_fd = -1;
}


CFileReader::~CFileReader()
{
	// Close the file.

	if (IsOpen())
		Close();

	m_fd = -1;
}



BOOL CFileReader::Open()
{
	BOOL bResult = FALSE;
	struct _stat st;

	// Open the file.
	if (m_fd < 0)
		m_fd = _open ( m_Filepath, O_RDONLY | O_BINARY | O_CREAT );

	// Did we succeed?
	if (m_fd < 0)
		return FALSE;

	// Set the file size.
	if (_fstat (m_fd, &st) == 0)
	{
		m_TotalSize = st.st_size;

		bResult = TRUE;
	}

	return bResult;
}


// Call _close() on the open file handle.
BOOL CFileReader::Close()
{
	BOOL bResult = TRUE;

	if (m_fd >= 0)
	{
		// _close() returns 0 if successful.
		bResult = (_close (m_fd) == 0);

		// In any case, set this.
		m_fd = -1;
	}

	return bResult;
}



// NOTES
// The "Open()" method MUST be called before this is called.
// END NOTES

long CFileReader::Read	(char *buffer, ULONG BufferSize)
{
	// Sanity
	if (!buffer)
		return 0;

	// The file MUST be open.
	if (m_fd < 0)
		return -1;

	return (long) _read (m_fd, buffer, BufferSize);
}



long CFileReader::Read()
{
	return Read (m_buffer, BufferSize);
}


BOOL CFileReader::Done ()
{
	if (IsOpen())
		Close();

	return TRUE;
}







//==================== Definition of the CChunkReader class =========//


CChunkReader::CChunkReader (readfn_t readfn, void * read_data, long size,
							 BOOL bMustReadSingleLines)
{
	m_pReadfn	= readfn;
	m_pReadData = read_data;
	m_Size		= size;
	m_bMustReadSingleLines = bMustReadSingleLines;

	// Allocate buffer here:
	m_lBufferSize = 0;

	m_pReadBuffer = new char[BUFLEN + 4];

	if (m_pReadBuffer)
		m_lBufferSize = BUFLEN;

	// Make sure of these initially:
	m_lBytesInBuffer = 0;
	m_lBytesRead	 = 0;

	m_pStart = m_pNext = NULL;
}


CChunkReader::~CChunkReader ()
{
	// Free memory:
	if (m_pReadBuffer)
	{
		delete[] m_pReadBuffer;
		m_pReadBuffer = NULL;
	}
}
	


BOOL CChunkReader::GetNextChunk (LPSTR *pBuf, long *lBytes)
{
	BOOL bResult = FALSE;
	long inLen = 0;

	// Sanity:
	if (! (pBuf && lBytes) )
		return FALSE;

	// Initialize them:
	*pBuf = NULL;
	*lBytes = 0;

	// Must also make sure these have been initialized:
	//
	if (! (m_pReadfn && m_pReadData && (m_Size > 0)) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Note: We MAY loop if we need to read more data:
	//
	while (1)
	{
		// If buffer is empty, we must read a new chunk. This will be the case
		// if we are handling a complete buffer-full of data at a time,
		// of if this is our first entry into the loop.
		//
		if (m_lBytesInBuffer <= 0)
		{
			// Read a full buffer.
			//
			long lBytesToRead = min (m_lBufferSize, (m_Size - m_lBytesRead));

			// If no more bytes to read, some error:
			if (lBytesToRead <= 0)
			{
				break;
			}

			// Read now.
			if (!(*m_pReadfn)(m_pReadData, lBytesToRead, m_pReadBuffer))
			{
				break;
			}

			// Make sure to update these:
			//
			m_lBytesRead	 += lBytesToRead;
			m_lBytesInBuffer  = lBytesToRead;

			// Start at the beginning:
			m_pNext = m_pReadBuffer;
		}
			
		// If we need to send just one line at a time, do so:
		// Binhex requires a line at a time. 
		if (m_bMustReadSingleLines)
		{
			// Must have a next point to start at:
			//
			if (!m_pNext)
			{
				bResult = FALSE;
				break;
			}

			// Advance pointer. First time through, m_pNext == m_pReadBuffer. 
			// 
			m_pStart = m_pNext;

			inLen = 0;

			while ( inLen <= m_lBytesInBuffer)
			{
				if (*m_pNext == '\r')
					break; // Out of inner loop.
				else
				{
					m_pNext++; inLen++;
				}
			}

			// Did we find the end of line??
			if (*m_pNext == '\r' && inLen < m_lBytesInBuffer)
			{
				++m_pNext; inLen++;
					
				// Yep. Eat \n
				//
				if (*m_pNext == '\n' && inLen < m_lBytesInBuffer)
				{
					m_pNext++; inLen++;
				}

				// Update lBytesInBuffer;
				m_lBytesInBuffer -= inLen;

				// Fix up our return values:
				bResult = TRUE;
				*pBuf = m_pStart;
				*lBytes = inLen;

				break;
			}
			else
			{
				// If we have read all the bytes, then perhaps the last line
				// wasn't terminated with /r/n. Just return it:
				//
				if (m_lBytesRead >= m_Size)
				{
					bResult = TRUE;
					*pBuf = m_pStart;
					*lBytes = m_lBytesInBuffer;

					break;
				}

				// Otherwise, need to read a fresh chunk and go through this proceedure again:
				// This means LOOP!!
				//
				// First, Move leftovers to beginning of buffer:
				//
				for (int i = 0; i < m_lBytesInBuffer; i++)
				{
					m_pReadBuffer[i] = *m_pStart++;
				}

				m_pStart = m_pReadBuffer + m_lBytesInBuffer;

				// Read now.
				long lBytesToRead = min ( (m_lBufferSize - m_lBytesInBuffer),
										  (m_Size - m_lBytesRead) );

				if (!(*m_pReadfn)(m_pReadData, lBytesToRead, m_pStart))
				{
					break;
				}

				// Update total number of bytes in buffer:
				m_lBytesInBuffer += lBytesToRead;

				// And total number of bytes read:
				m_lBytesRead += lBytesToRead;

				// We now go through the loop again to get a full line.
				m_pNext = m_pReadBuffer;

				// Note: Don't break;
			}
		}
		else
		{
			// Send the full buffer to be written or decoded:
			*pBuf	= m_pReadBuffer;
			*lBytes = m_lBytesInBuffer;

			// Read a new buffer-full next time:
			m_lBytesInBuffer = 0;

			// We succeeded:
			//
			bResult = TRUE;

			// Get out of loop:
			break;
		}
	} // While

	// Make sure we got good results:
	// 
	if (bResult && *pBuf && (*lBytes > 0 ))
		return TRUE;
	else
		return FALSE;
}

#endif // IMAP4
