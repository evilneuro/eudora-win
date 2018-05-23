////////////////////////////////////////////////////////////////////////
// File: ImapDownload.CPP
//
//
//
////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "address.h"
#include "summary.h"
#include "tocdoc.h"
#include "progress.h"
#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "Base64.h"
#include "QP.h"
#include "mime.h"
#include "ImUudecode.h"
#include "ImHexbin.h"
#include "persona.h"
#include "fileutil.h"

#include "eudora.h"

#include "imap.h"
#include "imapgets.h"
#include "imapjournal.h"
#include "imapresync.h"
#include "imapfol.h"
#include "imapopt.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imapmime.h"
#include "ImapHeader.h"
#include "ImapLex822.h"
#include "ImapDownload.h"
#include "ImapSum.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// Internal magic number constants. These are arbitrary.
const	unsigned long	_MAGICNUMBER1	= 98754908;	
const	unsigned long	_MAGICNUMBER2	= 98754909;	
const	unsigned long	_MAGICNUMBER3	= 98754910;	
const	unsigned long	_MAGICNUMBER4	= 98754911;	

//
// Sizes of fields in an attachment stub file.
//
const long	AtfImapNameSize			= 256;		// Full imap name
const long	AtfImapSectionSize		= 256;		// IMAP part specifier.
const long	AtfImapTypeSize			= 48;		// Body type string.
const long	AtfImapSubtypeSize		= 48;		// Body subtype string.
const long	AtfImapEncSize			= 48;		// Encoding string.
const long	AtfImapIDSize			= 256;		// Body ID..
const long	AtfImapDescSize			= 256;		// Body decsription..

//
// Internal class for managing attachment stub files.
//
class CAttachStubFile
{

public:	
	CAttachStubFile(LPCSTR pFilePath);
	~CAttachStubFile();

	// Public Methods.

	// Write the file.
	BOOL WriteBodyPartPlaceHolder ();

	// Read the file.
	BOOL ReadBodyPartPlaceHolder ();

// Parameters (public)

	unsigned long m_Uid;
	unsigned long m_Uidvalidity;
	unsigned long m_AccountID;
	unsigned long m_SizeInBytes;
	unsigned long m_SizeInLines;

	CString		m_szType;
	CString		m_szSubType;
	CString		m_szEncoding;
	CString		m_szImapName;
	CString		m_szBodyId;
	CString		m_szBodyDesc;
	CString		m_szSection;

	// The full path to the file.
	CString		m_szFilePath;
};



////////////////////////////////////////////////////
// EXTERN

extern char *pImapShortHeaderFields;

// END EXTERN 

// STATIC 
static char *pAdditionalHeaders = "X-Priority,Content-Type";

// END STATIC
/////////////////////////////////////////////////////

// Internal functions //////////

BOOL	FillSummaryFromEnvelope (ENVELOPE *pEnvelope, CImapSum *pSum);
void	GetFilenameParameter (BODY *pBody, CString& szFilename);
void	DoWeAddXrichOrXhtmlTokens (BODY *pBody, BOOL& bFoundRich, BOOL& bFoundHtml, BOOL bIsTopBody = TRUE);
void	BodyTypeCodeToString (short Type, CString& szType);
void	BodyEncodingCodeToString (short Encoding, CString& szEncoding);
short	BodyEncodingStringToCode (LPCSTR pEncoding);
DISPOSITION_TYPE DispositionTypeFromStr (char *Value);
static BOOL IsMPAlternativeOrRelated (const char *Buffer);

//////////////////////////////////////////////////////////////////////////////////////////////////




//====================== CImapDownloader class ===============================/
CImapDownloader::CImapDownloader(unsigned long AccountID, CIMAP* pImap, LPCSTR szMbxFilePath)
{
	// Initialilze from parameters:
	//
	m_AccountID		= AccountID;
	m_Pathname		= szMbxFilePath;	

	m_CurrentSize	= 0;
	m_Decoder		= NULL;

	// TRUE for BINHEX:
	m_bMustReadSingleLines = FALSE;

	// The buffer is allocated during the Write.
	m_nBufferSize	= 0;
	m_pReadBuffer	= NULL;
	m_pWriteBuffer	= NULL;

	m_Ptr			= NULL;
	m_Vbytes		= 0;

	m_DownloadMode	= DOWNLOADING_NONE;

	// Set pointer to the IMAP object.
	m_pImap = pImap;

	m_pReadFn	= NULL;
	m_pReadData = NULL;

	// This is allocated later.
	m_pHd = NULL;

	// Initialize the to 0. Every M/R block will increment it.
	m_iLastMRelatedID	= 0;

	m_pImapSum			= NULL;

	// Initialize to TYPETEXT.

	m_CurrentBodyType	= TYPETEXT;

	// Until proven guilty.
	m_bIsMhtml			= FALSE;
	m_bHasAttachment	= FALSE;

	// Default to attachment:
	m_Disposition = DISPOSITION_UNKNOWN;

	m_bUseEnvelope = FALSE;

	m_bForceAttachDownload = FALSE;
}



CImapDownloader::~CImapDownloader()
{
	// Close file if still open.
	if (m_mbxFile.IsOpen() == S_OK)
		m_mbxFile.Close();

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

	// Delete the header desc
	if (m_pHd)
	{
		delete m_pHd;
		m_pHd = NULL;
	}

	// If we have any un-flushed embedded elements, flush them now.
	FlushEmbeddedElements();
}






// DownloadSingleMessage [PUBLIC] //////////////////////////////////////////////////////////
// FUNCTION
// Go fetch the message given by Uid into the mbx file.
// If the file's not there, create it, otherwise, append the mesage.
// Return the starting and ending file offsets in MStart and MEnd.
// Return FALSE if an error occurred in fetching the message.
// END FUNCTION
////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapDownloader::DownloadSingleMessage (CImapSum *pSum, BOOL bDownloadAttachments)
{
	BODY*			pBody	= NULL;
	HRESULT			hResult	= E_FAIL;
	long			MStart = 0, MEnd = 0;
	IMAPUID			Uid = 0;
	BOOL			bWeOpenedMbxFile = FALSE;

	// SAnity:
	if (! pSum )
		return E_INVALIDARG;

	// It must also be open.
	if ( ! SUCCEEDED (m_mbxFile.IsOpen()) )
	{
		OpenMbxFile (FALSE);

		// Flag that we opened it.
		bWeOpenedMbxFile = TRUE;
	}

	// Did we succeed?
	if (! SUCCEEDED ( m_mbxFile.IsOpen() ) )
		return E_FAIL;

	// Broadcase whether we are forcing attachment downloads.
	//
	m_bForceAttachDownload = bDownloadAttachments;

	// Set out current summary so internal methods will be able to modify
	// it's flags as we unravel the download.
	//
	m_pImapSum = pSum;

	//
	// NOTE!!! IMPORTANT!!! If we fail at any point below this, don't return NULL!! 
	// Instead, write a dummy message to the MBX file and set the summary to point to this 
	// dummy message and return TRUE. Keep the "IMFLAGS_NOT_DOWNLOADED" set to TRUE so
	// we will try the download again next time.
	//


	// Use a for so we can break out easily. Note - we're not looping!!

	hResult = E_FAIL;

	// Depending on where we fail, we might have written stuff to the MBX file.
	// We'd have to remove it.
	//
	BOOL  bMustTruncate = FALSE;

	for (int i = 0; i < 1; i++)
	{
		// We must have a valid IMAP object
		if (!m_pImap)
		{
			ASSERT (0);
			break;
		}

		// Our IMAP connection must be open.
		if ( ! m_pImap->IsSelected() )
			break;

		// If the header object has not been allocated, do so now.
		if (!m_pHd)
		{
			m_pHd = new ImapHeaderDesc;
		}

		if (!m_pHd)
			break;

		// Make sure get to end of file for writing!
		//
		m_mbxFile.Seek(0L, SEEK_END);

		// Always initialize header data structure.
		m_pHd->Init ();

		// We MUST be at the end of the file.
		//
		m_mbxFile.Tell(&MStart);

		// Initialize MEnd to MStart in case we fail.
		MEnd = MStart;

		Uid = pSum->GetHash();

		// Tell headerdesc what our UID is.
		m_pHd->m_Uid = Uid;

		// Get the body now then retrieve selectively.
		// Note: We now have to free the body when we're done!!!

		pBody = NULL; m_pImap->UIDFetchStructure (Uid, &pBody);

		if ( pBody )
		{
			// If we het here, we may have written stuff to the MBX file. We'd 
			// have to remove it belowif we fail.
			//
			bMustTruncate = TRUE;

			//
			// We may have set this flag when we did the minimal download. It will be reset
			// here so clear it before we proceed.
			//
			pSum->UnsetFlag(MSF_HAS_ATTACHMENT);

			// Append message delimiter line.
			DownloadingNone();
			AddMessageSeparator ();

			// Tell writer that we are download header now. It will turnoff decoder, etc.
			DownloadingHeader();

			// Always download full header here. Pass it through hd.read.

			hResult = m_pImap->UIDFetchHeaderFull (Uid, this);

			if ( !SUCCEEDED (hResult) )
				break;

			// Flush the file JJFile buffer because we need to move around in the file.
			if ( FAILED (m_mbxFile.Flush()) )
			{
				break;
			}

			// Go read the newly-written header from the MBX file to get the info.
			// Move the file pointer temporarily back to the beginning of the header.
			// But, save the current offset first.

			long lCurrentOffset;

			m_mbxFile.Tell(&lCurrentOffset);

			if (lCurrentOffset >= 0)
			{
				if (SUCCEEDED ( m_mbxFile.Seek (m_pHd->m_lDiskStart) ) )
				{
					ImapRebuildSummary (pSum, m_pHd, &m_mbxFile, (lCurrentOffset - m_pHd->m_lDiskStart) );

					// Reset file offset.
					m_mbxFile.Seek (lCurrentOffset);
				}
				else
				{
					ASSERT (0);
					break;
				}
			}
			else
			{
				ASSERT(lCurrentOffset >= 0);
				break;
			}

			// Set certain info from the HeaderDesc.
			SetSummaryInfoFromHD ( pSum, m_pHd);
	
			// Do the following for when we have to rebuild the summaries.

			// Do we set the fancy flags in the summary?

			BOOL	bFoundRich = FALSE;
			BOOL	bFoundHtml = FALSE;

			DoWeAddXrichOrXhtmlTokens (pBody, bFoundRich, bFoundHtml, TRUE);

			if (bFoundRich)
				pSum->SetFlag(MSF_XRICH);

			if (bFoundHtml)
			{
				pSum->SetFlag(MSF_XRICH);
				pSum->SetFlagEx(MSFEX_HTML);

				// Set this as well.
				m_pHd->m_isMHTML = TRUE;
			}

			// Fetch the body now,

			// Broadcast that that's what we're doing.
			DownloadingBody();

			// Is this a multipart body?
			if (pBody->type == TYPEMULTIPART)
			{
				if ( DownloadMultipartBody (Uid, pBody, NULL) )
				{
					hResult = S_OK;
				}

				// If we saved embedded content lines, go write those out.
				WriteEmbeddedElements( m_pHd->m_isMHTML );
			}
			// Unlikely but it's possible to have a message right within the
			// top message.
			//
			else if (pBody->type == TYPEMESSAGE)
			{
				if ( DownloadSubMessage (Uid, pBody, "1") )
				{
					hResult = S_OK;
				}
			}
			else
			{
				if ( DownloadSimpleBody (Uid, pBody, "1") )
				{
					hResult = S_OK;
				}
			}

			// Important!! We now have to free the body!!
			m_pImap->FreeBodyStructure	(pBody);
			pBody = NULL;

			// Always terminate a message with a blank line!!!
			m_mbxFile.PutLine();

			// If we succeeded, Find MEnd:
			m_mbxFile.Flush ();
			m_mbxFile.Tell(&MEnd);

		}	// If pBody.

		// Break out of the for.
		break;

	} // For.


	// If we didn't succeed, truncate the file.
	if ( !SUCCEEDED (hResult) )
	{
		if (bMustTruncate)		
			m_mbxFile.ChangeSize (MStart);
	}
	else
	{
		// We succeeded. Update the summary.
		pSum->SetOffset ( MStart );
		pSum->m_Length = MEnd - MStart;

		// For messages downloaded before we added m_MsgSize:
		if (pSum->m_MsgSize <= 1)
			pSum->m_MsgSize = (unsigned short) max (pSum->m_Length/1024, 1);

		// Now signal to the summary that this message is now fully downloaded.
		pSum->m_Imflags &= ~IMFLAGS_NOT_DOWNLOADED;

		// If any data writers found a "fancy" part, tell summary.
		if (m_bIsMhtml)
		{
			pSum->SetFlag(MSF_XRICH);
			pSum->SetFlagEx(MSFEX_HTML);
		}

		// If we broke any attachments out to files:
		if ( m_bHasAttachment )
		{
			pSum->SetFlag (MSF_HAS_ATTACHMENT);
		}
	}

	//
	// Close the mbx file if we opened it.
	//
	if (bWeOpenedMbxFile)
		CloseMbxFile();

	// Current IMAP summary may no longer be valid!!
	//
	m_pImapSum = NULL;

	
	return hResult;
}



// DoMinimalDownload [PUBLIC]
//
// Public interface to creating a new CImapSum and filling it with stuff
// without downloading the whole message.
// 
///////////////////////////////////////////////////////////////////////////
HRESULT CImapDownloader::DoMinimalDownload (IMAPUID Uid, CImapSum **ppSum)
{
	// This attempts to use headers, unless the m_bUseEnvelope flag is set.
	// This flag gets set if headers returns blank.
	//

	if (m_bUseEnvelope)
	{
		return DoMinimalDownloadUsingEnvelope (Uid, ppSum);
	}
	else

	{
		return DoMinimalDownloadUsingHeaders (Uid, ppSum);
	}
}



//
// WriteOfflineMessage [PUBLIC]
//
// FUNCTION
// Append a dummy summary message to the MBX file. This will be called if
// the download failed.
// END FUNCTION.

HRESULT CImapDownloader::WriteOfflineMessage (CImapSum *pSum)
{
	HRESULT			hResult	= S_OK;
	long			MStart = 0, MEnd = 0;
	BOOL			bWeOpenedMbxFile = FALSE;

	// Sanity.
	if (!pSum)
		return E_INVALIDARG;

	// Make sure our MBX file is open.
	// It must also be open.
	if ( ! SUCCEEDED (m_mbxFile.IsOpen()) )
	{
		OpenMbxFile (FALSE);

		// Flag that we opened it.
		bWeOpenedMbxFile = TRUE;
	}

	// Did we succeed?
	if (! SUCCEEDED ( m_mbxFile.IsOpen() ) )
		return E_FAIL;

	// Make sure get to end of file for writing!
	//
	m_mbxFile.Seek(0L, SEEK_END);

	// Save our starting point.
	m_mbxFile.Tell(&MStart);

	if (MStart < 0)
		return E_FAIL;

	// Write the message separator. 
	AddMessageSeparator ();

	// Write a fake message header using stuff from the summary.
	CString str;

	// Date: 
	//
	if ( !pSum->m_SummaryDate.IsEmpty() )
	{
		hResult = m_mbxFile.PutLine (CRString (IDS_HEADER_DATE) + " " + pSum->m_SummaryDate);
	}
	else if ( !pSum->m_RawDateString.IsEmpty() )
	{
		hResult = m_mbxFile.PutLine (pSum->m_RawDateString);
	}
	else
	{
		// Just continue writing.
		//
		hResult = S_OK;
	}
		

	// From:
	if ( SUCCEEDED (hResult) )
		hResult = m_mbxFile.PutLine (CRString (IDS_HEADER_FROM) + " " + pSum->m_From);

	// Subject:
	if ( SUCCEEDED (hResult) )
		hResult = m_mbxFile.PutLine (CRString (IDS_HEADER_SUBJECT) + " " + pSum->m_Subject);

	// X-UID header:
	if ( SUCCEEDED (hResult) )
	{
		str.Format (CRString (IDS_IMAP_UID_XHEADER) + " %ul", pSum->GetHash());
		hResult = m_mbxFile.PutLine (str);
	}

	// Blank line.
	if ( SUCCEEDED (hResult) )
		hResult = m_mbxFile.PutLine ();

	// Fake body.
	if ( SUCCEEDED (hResult) )
		hResult = m_mbxFile.PutLine ( CRString(IDS_IMAP_FAKE_BODY) );

	// If we succeeded, Find MEnd:
	if ( SUCCEEDED (hResult) )
		hResult = m_mbxFile.Flush ();

	if ( SUCCEEDED (hResult) )
	{
		m_mbxFile.Tell(&MEnd);

		pSum->SetOffset ( MStart );
		pSum->m_Length = MEnd - MStart;
	}
	else
	{
		// Truncate back to original size.
		m_mbxFile.ChangeSize (MStart);
		pSum->m_Length = 0;
	}

	if (bWeOpenedMbxFile)
		CloseMbxFile();

	return hResult;
}





//=============================== PRIVATE METHODS ======================================//


/////////////////////////////////////////////////////////////////////////////////////////////////
// DoMinimalDownloadUsingHeaders [PRIVATE]
//
// FUNCTION
// Do a minimal download of some header fields just to get the message's attributes,
//  fill in a new CImapSum and return it.
// Mark the new CImapSum with the flag to indicate that the message is only partially downloaded.
// The new CImapSum is returned, or NULL.
// END FUNCTION
////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT CImapDownloader::DoMinimalDownloadUsingHeaders (IMAPUID Uid, CImapSum **ppSum)
{
	BOOL			bWeOpenedOurConnection = FALSE;
	CImapSum*		pSum = NULL;
	HRESULT			hResult = E_FAIL;

	// Sanity. Must have a valid Uid. 
	if (Uid == 0 || (NULL == ppSum))
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	/// Initialize:
	*ppSum = NULL;

	//
	// Our CIMAP member reference MUST NOT BE NULL!
	//
	if (NULL == m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// We MUST be connected and the mailbox must be selected.
	if ( !m_pImap->IsSelected() )
		return E_FAIL;

	// Instantiate a memory writer to fetch the minimal headers.
	CStringWriter	stringWriter;

	// Get the header text into a string.
	hResult = m_pImap->UIDFetchRFC822HeaderFields (Uid, NULL, pImapShortHeaderFields, &stringWriter);

	if ( SUCCEEDED (hResult) )
	{
		// Get a pointer to the in-memory headers. Note, these would be separated by
		// "\r\n" just like if they were writte to a file.
		LPCSTR pHeader = stringWriter.GetString();

		// Did we get any stuff??
		if (pHeader && *pHeader)
		{
			// Allocate a new CImapSum that will be returned.
			pSum = new CImapSum;

			if (pSum)
			{
				// Set the Uid value:
				pSum->SetHash ( Uid );

				// Parse the headers and fill in the summary info into the new summary.
				if (!ParseMinimalHeaderInMemory (pSum, pHeader))
				{
					delete pSum;
					pSum = NULL;
				}
				else
				{
					//SK Get the message size and store it.  It is inefficient but the best way todo it for now
					unsigned long ulSize = 0;
					m_pImap->GetRfc822Size(Uid, &ulSize);
					
					pSum->m_MsgSize = (unsigned short) (ulSize/1024 + 1);

#if 0 // What does this do??
					if( (ulSize % 1024) >= 512)
						(pSum->m_MsgSize)++;
					if(pSum->m_MsgSize == 0)
						pSum->m_MsgSize = 1;
#endif
					

				}
			}
		}
	}

	// If we didn't get a summary or if pSum contain's blank from and
	// date fields, do this again with ENVELOPE.
	// 
	BOOL bUseEnvelope = FALSE;

	if (!pSum)
		bUseEnvelope = TRUE;
	else
	{
		// Must have at least from and Date:
		//
		if ( (!*pSum->m_From) && (!pSum->m_Seconds) )
		{
			delete pSum;
			pSum = NULL;

			bUseEnvelope = TRUE;
		}
	}

	if (bUseEnvelope)
	{
		m_bUseEnvelope = TRUE;

		return DoMinimalDownloadUsingEnvelope (Uid, ppSum);
	}
	else
	{
		// Output parm:
		*ppSum = pSum;
	}

	return hResult;
}





/////////////////////////////////////////////////////////////////////////////////////////////////
// DoMinimalDownloadUsingEnvelope [PRIVATE]
//
// FUNCTION
// Download the envelope for this message and extract summary info from it.
// Still do a HEADER.FIELDS to fetch X-Priority. 
// Note that this may slow thing down a bit.
// END FUNCTION

// HISTORY
// END HISTORY
//
////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapDownloader::DoMinimalDownloadUsingEnvelope (IMAPUID Uid, CImapSum **ppSum)
{
	BOOL			bWeOpenedOurConnection = FALSE;
	CImapSum*		pSum = NULL;
	HRESULT			hResult = E_FAIL;

	// Sanity. Must have a valid Uid. 
	if (Uid == 0 || (NULL == ppSum))
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	/// Initialize:
	*ppSum = NULL;

	//
	// Our CIMAP member reference MUST NOT BE NULL!
	//
	if (NULL == m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// We MUST be connected and the mailbox must be selected.
	if ( !m_pImap->IsSelected() )
		return E_FAIL;

	// We now fetch envelopes!! (2/12/98)
	//

	ENVELOPE *pEnvelope = NULL;

#if 0 // Fetch envelope then extra headers separately because if the server
	  // doesn't support HEADER.FIELDS, it would probably reject a combination
	  // of ENVELOPE and HEADER.FIELDS as well.
	  //

	// Instantiate a memory writer to fetch the minimal headers.
	CStringWriter	stringWriter;

	hResult = m_pImap->UIDFetchEnvelopeAndExtraHeaders (Uid, &pEnvelope,
							pAdditionalHeaders, &stringWriter );
#endif //

	hResult = m_pImap->UIDFetchEnvelope (Uid, &pEnvelope);

	if ( pEnvelope && SUCCEEDED (hResult) )
	{
		// Allocate a new CImapSum that will be returned.
		pSum = new CImapSum;

		if (pSum)
		{
			// Set the Uid value:
			pSum->SetHash ( Uid );

			// Extract stuff the ENVELOPE into our summary.
			//
			if ( !FillSummaryFromEnvelope (pEnvelope, pSum) )
			{
				delete pSum;
				pSum = NULL;
			}
		}

		// If we still have a summary, parse info from additional 
		// headers, (possibly priority, etc).
		//
		if (pSum)
		{
			DoAddAdditionalHeaders (Uid, pSum);

#if 0 // SEE ABOVE NOTE:
			// Get a pointer to the in-memory headers. Note, these would be separated by
			// "\r\n" just like if they were writte to a file.
			LPCSTR pHeader = stringWriter.GetString();

			// Did we get any stuff??
			if (pHeader && *pHeader)
			{
				// "pHeader" should contain priority and content-type stuff.
				// Don't care if we failed.
				ParseAdditionalHeadersInMemory (pSum, pHeader);
			}
#endif
		}

		// Make sure we free envelopw now:
		//
		m_pImap->DestroyEnvelope ( &pEnvelope );
		pEnvelope = NULL;
	}

	// Output parm:
	*ppSum = pSum;

	if (pSum)
		return S_OK;
	else
		return E_FAIL;
}




// DoAddAdditionalHeaders [PRIVATE]
//
void CImapDownloader::DoAddAdditionalHeaders (IMAPUID Uid, CImapSum *pSum)
{
	// Sanity:
	if (!pSum)
		return;

	// Instantiate a memory writer to fetch the minimal headers.
	CStringWriter	stringWriter;

	// Get the header text into a string.
	HRESULT hResult = m_pImap->UIDFetchRFC822HeaderFields (Uid, NULL, pAdditionalHeaders, &stringWriter);

	if ( SUCCEEDED (hResult) )
	{
		// Get a pointer to the in-memory headers. Note, these would be separated by
		// "\r\n" just like if they were writte to a file.
		LPCSTR pHeader = stringWriter.GetString();

		// Did we get any stuff??
		if (pHeader && *pHeader)
		{
			// "pHeader" should contain priority and content-type stuff.
			// Don't care if we failed.
			ParseAdditionalHeadersInMemory (pSum, pHeader);
		}
		else
		{
			//SK Get the message size and store it.  It is inefficient but the best way todo it for now
			unsigned long ulSize = 0;
			m_pImap->GetRfc822Size(Uid, &ulSize);
			pSum->m_MsgSize = (unsigned short) (ulSize/1024 + 1);

#if 0 // ??
			if( (ulSize % 1024) >= 512)
				(pSum->m_MsgSize)++;
			if(pSum->m_MsgSize == 0)
				pSum->m_MsgSize = 1;
#endif
			
		}
	}
}




/////////////////////////////////////////////////////////////////////////////
// ParseMinimalHeaderInMemory [PRIVATE]
//
// FUNCTION
// Parse headers in a memory buffer and fill in the summary info into pSum.
// The headers are in lines separated by "\r\n"
// ENDS FUNCTION

// NOTES
// The header buffer may contain any number of header lines. This function can therefore 
// be used to parse a minimal set of headers or the full set.
// Note: The header fields we check for in this routine must correspond with
// the fields we download ducing a minimal header download.
//
// END NOTES
/////////////////////////////////////////////////////////////////////////////

BOOL CImapDownloader::ParseMinimalHeaderInMemory (CImapSum* pSum, LPCSTR pHeader)
{
	BOOL bResult = TRUE;
	char Buffer [2048];

	// Sanity:
	if (! (pSum && pHeader) )
	{
		ASSERT (0);
		return FALSE;
	}

	if (!*pHeader)
		return FALSE;

	// This is similar to CImapSum::Build.

	// Some strings that will be used in this routine
	CRString HeaderFrom(IDS_HEADER_FROM);
	CRString HeaderSubject(IDS_HEADER_SUBJECT);
	CRString HeaderDate(IDS_HEADER_DATE);
	CRString HeaderPriority(IDS_HEADER_PRIORITY);
	CRString HeaderReplyTo(IDS_HEADER_REPLY_TO);
	CRString HeaderContentType(IDS_MIME_HEADER_CONTENT_TYPE);

	// Initialize
	Buffer[0] = '\0';

	while ( bResult && pHeader && *pHeader )
	{
		Buffer[0] = '\0';

		LPCSTR p = strstr (pHeader, "\r\n");
		LPCSTR q;

		// Is this the blank separator line??
		if ( p && (p == pHeader) )
		{
			// Blank line. This is the end of the header.
			pHeader = NULL;
			Buffer[0] = '\0';

			break;
		}
		else if (p)
		{
			// Did we get a line? Loop through possible multiple lines making up the header
			// field.
			do 
			{
				//
				// See if the next line is a continuation line.
				//
				q = p + 2;

				if ( !(*q == ' ' || *q == '\t') )
				{
					// No more continuation lines. Broeak out of do{}.
					break;
				}
				else
				{
					// This IS continuation line. Look for end of it.
					p = strstr (q, "\r\n");
				}
			} while (p);
		}
		
		// Copy what we got to the buffer. Don't overfill. If header line is too
		// long, reject it.				

		long len;

		*Buffer = 0;

		if (p)
		{
			len = (p - pHeader);

			if ( (len + 2) < sizeof (Buffer) )
			{
				strncpy (Buffer, pHeader, len);

				// Tie off.
				Buffer[len] = '\0';
			}

			// Skip over \r\n and continue to loop.
			pHeader = p + 2;
		}
		else
		{
			// Terminated by nul, not \r\n. Must be last line.
			len = strlen (pHeader);

			if ( (len + 2) < sizeof (Buffer) )
				strcpy (Buffer, pHeader);

			// Terminate loop.
			pHeader = NULL;
		}

		// Did we get anything in our buffer?
		if (*Buffer)
		{
			// Translate any encoded header fields as per RFC2047. 
			// Note: This is done in-place! AND, "len" is passed as a reference.
			//
			int len = strlen (Buffer);

			if (ImapFix2047(Buffer, len))
			{
				if ( len >= 0 && len < sizeof (Buffer) )
					Buffer[len] = '\0';
			}
			
			//
			if (!strnicmp(Buffer, HeaderDate, 5))
			{
				// Remove any line breaks in case we had a multiline date field.
				//
				LPSTR ptr = strstr (Buffer, "\r\n");
				while (ptr)
				{
					// Copy the nul also.
					memcpy ( ptr, ptr + 2, strlen(ptr + 2) + 1 );

					ptr = strstr (ptr, "\r\n");
				}

				pSum->m_Seconds = GetTime(Buffer + 5, FALSE);

				// Stores the raw date header value so "FormaatDate()" can extract the
				// timezone.
				//
				pSum->m_RawDateString = Buffer;
			}
			else if (!strnicmp(Buffer, HeaderFrom, 5))
				pSum->SetFrom(GetRealName(Buffer + 5));
			else if (!*pSum->m_Subject && !strnicmp(Buffer, HeaderSubject, 8))
			{
				// (JOK - 7/10/98) Stop at any \r\n within the sbjetc itself instead of 
				// removing them as we used to do.
				//
				LPSTR ptr = strstr (Buffer, "\r\n");
				if (ptr)
					*ptr = 0;

				::TrimWhitespaceMT(Buffer + 8);
				pSum->SetSubject(Buffer + 8);
			}
			else if (!*pSum->m_From && !strnicmp(Buffer, HeaderReplyTo, 9))
				pSum->SetFrom(GetRealName(Buffer + 9));
			else if (!strnicmp(Buffer, HeaderPriority, 7))
				pSum->GleanPriority(Buffer + 11);
			// Download Content-Type:. At this point, we DO display the HAS_ATTACHMENT
			// icon. It may disappear after we download the message!!
			else if (!strnicmp(Buffer, HeaderContentType, 13))
			{
				if (strstr(Buffer, CRString(IDS_MIME_MULTIPART)))
				{
					if (!IsMPAlternativeOrRelated (Buffer))
						pSum->SetFlag(MSF_HAS_ATTACHMENT);
				}
			}

		} // End parsing this line.
	} // While.

	return bResult;
}



/////////////////////////////////////////////////////////////////////////////
// ParseAdditionalHeadersInMemory [PRIVATE]
//
// FUNCTION
// Parse header string in a memory buffer looking specifically for the priority
// (X-Priority) and Content-Type: headers. Since we don't fetch the bodystructure
// until the user says so, we nee Content-Type to tell us if the message is
// multipart.
// ENDS FUNCTION

// NOTES
//
// END NOTES
/////////////////////////////////////////////////////////////////////////////

BOOL CImapDownloader::ParseAdditionalHeadersInMemory (CImapSum* pSum, LPCSTR pHeader)
{
	BOOL bResult = TRUE;
	char Buffer [2048];

	// Sanity:
	if (! (pSum && pHeader) )
	{
		ASSERT (0);
		return FALSE;
	}

	if (!*pHeader)
		return FALSE;

	// This is similar to CSummary::Build.
	//

	// Some strings that will be used in this routine
	CRString HeaderPriority(IDS_HEADER_PRIORITY);
	CRString HeaderContentType(IDS_MIME_HEADER_CONTENT_TYPE);

	// Initialize
	Buffer[0] = '\0';

	while ( bResult && pHeader && *pHeader )
	{
		Buffer[0] = '\0';

		LPCSTR p = strstr (pHeader, "\r\n");
		LPCSTR q;

		// Is this the blank separator line??
		if ( p && (p == pHeader) )
		{
			// Blank line. This is the end of the header.
			pHeader = NULL;
			Buffer[0] = '\0';

			break;
		}
		else if (p)
		{
			// Did we get a line? Loop through possible multiple lines making up the header
			// field.
			do 
			{
				//
				// See if the next line is a continuation line.
				//
				q = p + 2;

				if ( !(*q == ' ' || *q == '\t') )
				{
					// No more continuation lines. Broeak out of do{}.
					break;
				}
				else
				{
					// This IS continuation line. Look for end of it.
					p = strstr (q, "\r\n");
				}
			} while (p);
		}
		
		// Copy what we got to the buffer. Don't overfill. If header line is too
		// long, reject it.				

		long len;

		*Buffer = 0;

		if (p)
		{
			len = (p - pHeader);

			if ( (len + 2) < sizeof (Buffer) )
			{
				strncpy (Buffer, pHeader, len);

				// Tie off.
				Buffer[len] = '\0';
			}

			// Skip over \r\n and continue to loop.
			pHeader = p + 2;
		}
		else
		{
			// Terminated by nul, not \r\n. Must be last line.
			len = strlen (pHeader);

			if ( (len + 2) < sizeof (Buffer) )
				strcpy (Buffer, pHeader);

			// Terminate loop.
			pHeader = NULL;
		}

		// Did we get anything in our buffer?
		if (*Buffer)
		{
			// Replace any \r\n by spaces;
			LPSTR ptr = strstr (Buffer, "\r\n");
			while (ptr)
			{
				*ptr++ = ' '; *ptr++ = ' ';

				ptr = strstr (ptr, "\r\n");
			}

			if (!strnicmp(Buffer, HeaderPriority, 7))
			{
				pSum->GleanPriority(Buffer + 11);
			}
			// Download Content-Type:. At this point, we DO display the HAS_ATTACHMENT
			// icon. It may disappear after we download the message!!
			else if (!strnicmp(Buffer, HeaderContentType, 13))
			{
				if (strstr(Buffer, CRString(IDS_MIME_MULTIPART)))
				{
					if (!IsMPAlternativeOrRelated (Buffer))
						pSum->SetFlag(MSF_HAS_ATTACHMENT);
				}
			}

		} // End parsing this line.
	} // While.

	return bResult;
}





/////////////////////////////////////////////////////////////////////////////
// ParseHeaderInMemory [PRIVATE]
//
// FUNCTION
// Parse headers in a memory buffer and fill in the summary info into pSum.
// The headers are in lines separated by "\r\n"
// ENDS FUNCTION

// NOTES
// The header buffer may contain any number of header lines. This function can therefore 
// be used to parse a minimal set of headers or the full set.
// END NOTES
/////////////////////////////////////////////////////////////////////////////

BOOL CImapDownloader::ParseHeaderInMemory (CImapSum* pSum, LPCSTR pHeader)
{
	BOOL bResult = TRUE;
	char Buffer [512];

	// Sanity:
	if (! (pSum && pHeader) )
	{
		ASSERT (0);
		return FALSE;
	}

	if (!*pHeader)
		return FALSE;

	// This is similar to CSummary::Build.

	// Some strings that will be used in this routine	
	CRString HeaderTo(IDS_HEADER_TO);
	CRString HeaderFrom(IDS_HEADER_FROM);
	CRString HeaderSubject(IDS_HEADER_SUBJECT);
	CRString HeaderCc(IDS_HEADER_CC);
	CRString HeaderBcc(IDS_HEADER_BCC);
	CRString HeaderAttachments(IDS_HEADER_ATTACHMENTS);
	CRString HeaderDate(IDS_HEADER_DATE);
	CRString HeaderStatus(IDS_HEADER_STATUS);
	CRString HeaderPriority(IDS_HEADER_PRIORITY);
	CRString HeaderReplyTo(IDS_HEADER_REPLY_TO);
	CRString HeaderContentType(IDS_MIME_HEADER_CONTENT_TYPE);
   	CRString HeaderTrans(IDS_TRANS_XHEADER);
   	CRString HeaderSig(IDS_SIGNATURE_XHEADER);

	// Initialize
	Buffer[0] = '\0';

	while ( bResult && pHeader && *pHeader )
	{
		Buffer[0] = '\0';

		LPCSTR p = strstr (pHeader, "\r\n");
		LPCSTR q;

		// Is this the blank separator line??
		if ( p && (p == pHeader) )
		{
			// Blank line. This is the end of the header.
			pHeader = NULL;
			Buffer[0] = '\0';

			break;
		}
		else if (p)
		{
			// Did we get a line? Loop through possible multiple lines making up the header
			// field.
			do 
			{
				//
				// See if the next line is a continuation line.
				//
				q = p + 2;

				if ( !(*q == ' ' || *q == '\t') )
				{
					// No more continuation lines. Broeak out of do{}.
					break;
				}
				else
				{
					// This IS continuation line. Look for end of it.
					p = strstr (q, "\r\n");
				}
			} while (p);
		}
		
		// Copy what we got to the buffer. Don't overfill. If header line is too
		// long, reject it.				

		long len;

		*Buffer = 0;

		if (p)
		{
			len = (p - pHeader);

			if ( (len + 2) < sizeof (Buffer) )
			{
				strncpy (Buffer, pHeader, len);

				// Tie off.
				Buffer[len] = '\0';
			}

			// Skip over \r\n and continue to loop.
			pHeader = p + 2;
		}
		else
		{
			// Terminated by nul, not \r\n. Must be last line.
			len = strlen (pHeader);

			if ( (len + 2) < sizeof (Buffer) )
				strcpy (Buffer, pHeader);

			// Terminate loop.
			pHeader = NULL;
		}

		// Did we get anything in our buffer?
		if (*Buffer)
		{
			if (!strnicmp(Buffer, HeaderDate, 5))
			{
				pSum->m_Seconds = GetTime(Buffer + 5, FALSE);

				// Stores the raw date header value so "FormaatDate()" can extract the
				// timezone.
				//
				pSum->m_RawDateString = Buffer;
			}
			else if (!strnicmp(Buffer, HeaderFrom, 5))
				pSum->SetFrom(GetRealName(Buffer + 5));
			else if (!*pSum->m_Subject && !strnicmp(Buffer, HeaderSubject, 8))
			{
				// (JOK - 7/10/98) Stop at any \r\n within the sbjetc itself instead of 
				// removing them as we used to do.
				//
				LPSTR ptr = strstr (Buffer, "\r\n");
				if (ptr)
					*ptr = 0;

				::TrimWhitespaceMT(Buffer + 8);
				pSum->SetSubject(Buffer + 8);
			}
			else if (!strnicmp(Buffer, HeaderStatus, 7))
			{
				// Don't do status. We already know it anyway.
			}
			else if (!*pSum->m_From && !strnicmp(Buffer, HeaderReplyTo, 9))
				pSum->SetFrom(GetRealName(Buffer + 9));
			else if (!strnicmp(Buffer, HeaderPriority, 7))
				pSum->GleanPriority(Buffer + 11);
			else if (!strnicmp(Buffer, HeaderContentType, 13))
			{
				if (strstr(Buffer, CRString(IDS_MIME_ENRICHED)))
					pSum->SetFlag(MSF_XRICH);
				if (strstr(Buffer, CRString(IDS_MIME_HTML)))
				{
					pSum->SetFlag(MSF_XRICH);
					pSum->SetFlagEx(MSFEX_HTML);
				}
			}
		} // End parsing this line.
	} // While.

	return bResult;
}




// DownloadHeaderToMbxFile [PRIVATE]
// NOTES
// Fetch full header text and append to MBX file.
// END NOTES
BOOL CImapDownloader::DownloadFullHeaderToMbxFile (IMAPUID uid)
{
	LPCSTR			pText = NULL;
	CStringWriter	stringWriter;
	BOOL			pResult = FALSE;

	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// No decoder for header.
	ResetDecoder();					// No decoder.

	// Get the header text into a string.
	return SUCCEEDED (m_pImap->UIDFetchHeaderFull (uid, this));
}





// DownloadHeaderToMbxFile [PRIVATE]
// NOTES
// Fetch full header text and append to MBX file.
// END NOTES
BOOL CImapDownloader::DownloadShortHeaderToMbxFile (IMAPUID uid)
{
	LPCSTR			pText = NULL;
	CStringWriter	stringWriter;
	BOOL			pResult = FALSE;

	if (!uid)
	{
		ASSERT (0);
		return FALSE;
	}

	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// No decoder for header.
	ResetDecoder();					// No decoder.

	// Fetch individual headers, if they exist.
	// Note: When the MIME store comes, we'd fetch the envelope and
	// additional headers instead of doing this.

	// Get the header text into a string.
	return SUCCEEDED (m_pImap->UIDFetchRFC822HeaderFields (uid, NULL, pImapShortHeaderFields, this));
}




// DownloadSimpleBody [PRIVATE]
// NOTES
// If it's text, append to MBX file, else break out.
// END NOTES
//

BOOL CImapDownloader::DownloadSimpleBody (IMAPUID uid, BODY *pBody, LPSTR pSection)
{
	BOOL bResult = FALSE;
	DISPOSITION_TYPE DispType;

	// Sanity:
	if (!(uid && pBody && pSection))
	{
		ASSERT (0);
		return FALSE;
	}

	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Tell everyone what body type we're downloading.
	m_CurrentBodyType = pBody->type;
	m_szCurrentBodySubtype = pBody->subtype;

	// If this body has a disposition, use use that internally:
	// Default to the "current" disposition type.
	//
	DispType = GetDisposition ();

	if (pBody->disposition.type)
	{
		DispType = DispositionTypeFromStr (pBody->disposition.type);
	}

	// Depends on body type...
	switch (pBody->type)
	{
		case TYPETEXT:
			//
			// Treat as attachment only if too large or we were asked to by
			// a Content-Disposition: Attachment
			//
			if ( (DispType == DISPOSITION_ATTACHMENT) || (pBody->size.bytes > MsgByteLimit32) )
			{
				// Treat as a body part object.
				bResult = BreakoutBodyToFile (uid, pBody, pSection); 
			}
			// If Content-Disposition said inline, append to file.
			else
			{
				// Get body text!!
				bResult = AppendBodyTextToMbxFile (uid, pSection, pBody);
			}
			break;

		case TYPEMESSAGE:
		case TYPEMULTIPART:
			// Error if we get here.
			ASSERT (0);
			return FALSE;
			break;

		//
		// For all others, break out to a separate file.
		//
		default:
			bResult = BreakoutBodyToFile (uid, pBody, pSection); 
			break;
	} // Switch

	return bResult;
}




// DownloadMultipartBody [PRIVATE]
//
// FUNCTION
// Main multipart body dispatcher. Handles Multipart/Alternative,
// Multipart/Related specially.
// END FUNCTION

// NOTES
// 
// END NOTES
//
BOOL CImapDownloader::DownloadMultipartBody (IMAPUID uid, BODY *pParentBody, LPSTR pParentSection)
{
	BOOL		bResult = FALSE;

	// Make sure...
	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Must have a body.
	if ( !pParentBody )
		return FALSE;

	// The body MUST have a subtype, otherwise treat as plain text.
	if (NULL == pParentBody->subtype)
	{
		char *pThisSection;

		if (pParentSection && *pParentSection)
		{
			pThisSection = pParentSection;
		}
		else 
		{
			pThisSection = "1";
		}
			

		return  AppendBodyTextToMbxFile (uid, pThisSection, pParentBody);
	}

	//
	// If the multipart body has a disposition type, record it's current type (in
	// our private member variable), since it should be inherited by body parts.
	//
	if (pParentBody->disposition.type)
	{
		DISPOSITION_TYPE dType = DispositionTypeFromStr (pParentBody->disposition.type);
		if (dType != DISPOSITION_UNKNOWN)
			SetDisposition (dType);
	}

	// Is this a Multipart/Related multipart body?
	// Is it multipart/appledouble??
	//
	BOOL bDoingMAlternative = FALSE;
	BOOL bDoingAppleDouble = FALSE;

	if ( stricmp (pParentBody->subtype, CRString (IDS_MIME_ALTERNATIVE) ) == 0)
	{
		bDoingMAlternative = TRUE;
	}
	else if (stricmp (pParentBody->subtype, CRString (IDS_MIME_APPLEDOUBLE) ) == 0)
	{
		bDoingAppleDouble = TRUE;
	}
		
	// If this is a multipart/alternative, try to handle it the proper way for
	// multipart/alternative, If that failed, we'd have to loop through
	// all the parts.
	//
	BOOL bHandled = FALSE;

	if (bDoingMAlternative)
	{
		bHandled = HandleMultipartAlternative (uid, pParentBody, pParentSection);
	}
	else if (bDoingAppleDouble)
	{
		bHandled = HandleAppleDouble (uid, pParentBody, pParentSection);
	}

	// Do we have to loop through all the parts??
	//
	if (bHandled)
	{
		bResult = TRUE;
	}
	else
	{
		// DownloadAllParts:
		bResult = DownloadAllParts (uid, pParentBody, pParentSection);
	}

	return bResult;
}


// DownloadAllParts [PRIVATE]
//
// Loop through all body parts and download all of them.
//

BOOL CImapDownloader::DownloadAllParts (IMAPUID uid, BODY *pParentBody, LPCSTR pParentSection)
{
	char		section[1024];		// Current section.
	BOOL		bResult = FALSE;
	BOOL		TopLevel = TRUE;			// Top level of the message.
	PART		*part = NULL;
	BODY		*body = NULL;
	int			iMRelatedID = 0;

	// Make sure...
	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Must have a body.
	if ( !pParentBody )
		return FALSE;

	// Copy parent section string.
	//
	if (pParentSection == NULL)
		TopLevel = TRUE;
	else if (*pParentSection == 0)
		TopLevel = TRUE;
	else
		TopLevel = FALSE;

	// If the subtype is multipart related, send contents to the "Embedded"
	// directory and write an Embedded-content line instead of an "Attachment
	// Converted" line.

	BOOL bDoingMRelated = FALSE;
	if ( stricmp (pParentBody->subtype, CRString (IDS_MIME_RELATED) ) == 0)
	{
		// Get  unique ID for this M/R block.
		iMRelatedID = ++m_iLastMRelatedID;

		bDoingMRelated = TRUE;
	}

	// Is this a Multipart/Related multipart body?
	// Shouldn't be, because M/A is handled elsewhere, buth tat may fail.
	//

	BOOL bDoingMAlternative = FALSE;
	if ( stricmp (pParentBody->subtype, CRString (IDS_MIME_ALTERNATIVE) ) == 0)
	{
		bDoingMAlternative = TRUE;
	}

	// Loop through all parts:
	int partNum = 0;

	part = pParentBody->nested.part;
	while (part)
	{
		// Is this the first part??
		if (TopLevel)
		{
			wsprintf (section, "%d", ++partNum);	// No dot.
		}
		else
		{
			// Copy parent section first. MUST have a non-NULL parent section if not top level!
			strcpy (section, pParentSection);
			wsprintf (section + strlen(section), ".%d", ++partNum);
		}
			
		body = &part->body;

		// Tell everyone what body type and subtype we're downloading.
		m_CurrentBodyType		= body->type;
		m_szCurrentBodySubtype	= body->subtype;

		switch (body->type)
		{
			case TYPETEXT:
				// If we are in a multipart/related or multipart/alternative, download
				// inline.
				if (bDoingMRelated || bDoingMAlternative)
				{
					// Get body text!!
					bResult = AppendBodyTextToMbxFile (uid, section, body);
				}
				else
				{
					// Let "DownloadSimpleBody" decide:

					bResult = DownloadSimpleBody (uid, body, section); 
				}
				break;

			case TYPEMESSAGE:
			{
				// Get message body and recurse through that.
				//

				// Note: Save m_Disposition because this may change it's value:
				//

				DISPOSITION_TYPE SavedDisp;
				SavedDisp = GetDisposition ();

				bResult = DownloadSubMessage (uid, body, section);

				// Reset disposition:
				SetDisposition (SavedDisp);
				
				break;
			}
				
			case TYPEMULTIPART:
			{
				// Recurse.

				// Note: Save m_Disposition because this may change it's value:
				//
				DISPOSITION_TYPE SavedDisp = GetDisposition ();

				bResult =  DownloadMultipartBody (uid, body, section);

				// Reset disposition:
				SetDisposition (SavedDisp);

				break;
			}

			// For all others, break out to file.
			default:
				// If we are in a multipart/related, breakout as an embedded content.
				if (bDoingMRelated)
				{
					// Get body text!!
					bResult = BreakoutBodyToEmbeddedFile (uid, pParentBody, body, section, iMRelatedID);
				}
				else
				{
					// An ordinary attachment-converted line.
					bResult = BreakoutBodyToFile (uid, body, section); 
				}
				break;
		}  // switch

			// If any of these failed, get out.
		if (!bResult)
			break; // while

		// Next part.
		part = part->next;
	}

	return bResult;
}



// HandleMultipartAlternative [PRIVATE]
//
// Loop through all body parts looking for the best we can handle and download that.
//
BOOL CImapDownloader::HandleMultipartAlternative (IMAPUID uid, BODY *pParentBody, LPCSTR pParentSection)
{
	BOOL		bResult = FALSE;
	BOOL		TopLevel = TRUE;			// Top level of the message.
	PART		*part = NULL;
	BODY		*body = NULL;

	// Make sure...
	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Must have a body.
	if ( !pParentBody )
		return FALSE;

	// Copy parent section string.
	//
	if (pParentSection == NULL)
		TopLevel = TRUE;
	else if (*pParentSection == 0)
		TopLevel = TRUE;
	else
		TopLevel = FALSE;

	// Loop through all parts, looking for the one we should download.
	// When done, these should contain the one to downoad.
	//
	BODY		*pBodyToDownload = NULL;
	char		section[1024];
	*section = 0;

	// Utility variables.
	//
	CString		szAltSybtype;
	int partNum = 0;

	szAltSybtype.Empty();
	part = pParentBody->nested.part;

	while (part)
	{
		// Is this the first part??
		if (TopLevel)
		{
			wsprintf (section, "%d", ++partNum);	// No dot.
		}
		else
		{
			// Copy parent section first. MUST have a non-NULL parent section if not top level!
			strcpy (section, pParentSection);
			wsprintf (section + strlen(section), ".%d", ++partNum);
		}
			
		body = &part->body;

		// Make sure we have at least one.
		//
		if (!pBodyToDownload)
		{
			pBodyToDownload = body;
			szAltSybtype = body->subtype;
		}
		else
		{
			// Is this a better part to download?
			//
			CString		szCurSubtype;

			szCurSubtype = body->subtype;
					
			if (szAltSybtype.IsEmpty ())
			{
				// Accept this.
				pBodyToDownload = body;
				szAltSybtype = body->subtype;
			}
			else
			{
				CString newSubtype; 

				if ( ( CompareRStringI(IDS_MIME_PLAIN, szCurSubtype)	== 0 ) ||
					 ( CompareRStringI(IDS_MIME_ENRICHED, szCurSubtype) == 0 ) ||
					 ( CompareRStringI(IDS_MIME_HTML, szCurSubtype)		== 0 ) )
				{
					newSubtype = szCurSubtype;
				}
				else 
				{
					newSubtype.Empty();
				}
				
				// If we got one of our better types, make that the new body to
				// download and set current and alternate types.
				//
				if (!newSubtype.IsEmpty())
				{
					// Is this something better? We choose mime/enriched over mime/html
					// or plain text.
					//
					if ( !CompareRStringI(IDS_MIME_ENRICHED, newSubtype) ||
					     ( !CompareRStringI(IDS_MIME_HTML, newSubtype) &&
					        CompareRStringI(IDS_MIME_ENRICHED, szAltSybtype) ) )
					{
						// Accept this.
						pBodyToDownload = body;
						szAltSybtype = newSubtype;
					}
				}
			}
		}

		// Next part.
		part = part->next;
	}


	// Did we get a part to download??
	if (!pBodyToDownload || (*section == 0))
	{
		return FALSE;
	}

	// Ok. Download this part.
	//

	// Tell everyone what body type and subtype we're downloading.
	m_CurrentBodyType		= pBodyToDownload->type;
	m_szCurrentBodySubtype	= pBodyToDownload->subtype;

	switch (pBodyToDownload->type)
	{
		case TYPETEXT:
			// Download inline.
			// Get body text!!
			bResult = AppendBodyTextToMbxFile (uid, section, pBodyToDownload);

			break;

		// Shouldn't really have this!!
		case TYPEMESSAGE:
		{
			// Get message body and recurse through that.
			//

			// Note: Save m_Disposition because this may change it's value:
			//

			DISPOSITION_TYPE SavedDisp;
			SavedDisp = GetDisposition ();

			bResult = DownloadSubMessage (uid, pBodyToDownload, section);

			// Reset disposition:
			SetDisposition (SavedDisp);
				
			break;
		}
	
		// Shouldn't have this either!!			
		case TYPEMULTIPART:
		{
			// Recurse.

			// Note: Save m_Disposition because this may change it's value:
			//
			DISPOSITION_TYPE SavedDisp = GetDisposition ();

			bResult =  DownloadMultipartBody (uid, pBodyToDownload, section);

			// Reset disposition:
			SetDisposition (SavedDisp);

			break;
		}

		// For all others, break out to file.
		default:
			// Download as an ordinary attachment-converted line.

			bResult = BreakoutBodyToFile (uid, pBodyToDownload, section); 

			break;
	}  // switch

	// OK. We handled it.
	//
	return TRUE;
}





// HandleAppleDouble [PRIVATE]
//
// Appledouble has an application/Applefile part containing the resource fork,
// followed by the data fork asother part.
// We need to get the filename from the Applefile part for use in storing the data fork.
// 
// NOTE: "pParentBody" points to the multipart/appledouble body.
//
//
BOOL CImapDownloader::HandleAppleDouble (IMAPUID uid, BODY *pParentBody, LPCSTR pParentSection)
{
	BOOL		bResult = FALSE;

	// Make sure...
	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Must have a body.
	if ( !pParentBody )
		return FALSE;

	// First part must be Application/Applefile:.
	//
	PART		*pPart = NULL;
	BODY		*pBody = NULL;

	pPart = pParentBody->nested.part;

	if (!pPart)
	{
		// Grievous error:
		return FALSE;
	}

	// Part must be Applefile:
	//
	pBody = &pPart->body;
	CString szSubtype = pBody->subtype;

	// Test for Applefile subtype:
	if ( szSubtype.IsEmpty() || (szSubtype.CompareNoCase (CRString (IDS_MIME_APPLEFILE)) != 0) )
	{
		// Error
		return FALSE;
	}

	// Get the filename: This checks the Content-Disposition parameter list for a 
	// filename parameter. If that's not found, it will check the Content-Type
	//  parameter list.
	// 
	CString szFilename; szFilename.Empty();
	GetFilenameParameter (pBody, szFilename);

	// The next part should be the data fork:
	//
	pPart = pPart->next;

	if (!pPart)
	{
		// Grievous error:
		return FALSE;
	}

	// 
	pBody = &pPart->body;

	// Format section string:
	//
	char		section[1024];
	*section = 0;

	if (pParentSection && *pParentSection)
	{
		// Copy parent section first. MUST have a non-NULL parent section if not top level!
		strcpy (section, pParentSection);
		wsprintf (section, "%s.2", pParentSection);
	}
	else
	{
		wsprintf (section, "2");
	}

	// Ok, Can now break this body out to a file.
	// Pass "szFilename as the suggested filename.
	//
	return BreakoutBodyToFile (uid, pBody, section, szFilename);
}





// BreakoutBodyToFile [PRIVATE]
// NOTES
// Append an "Attachment Converted:" line to the MBX file and break out the body contents,
// possibly decoding it, into a file in the "Attach" subdirectory.
// END NOTES

BOOL CImapDownloader::BreakoutBodyToFile (IMAPUID uid, BODY *pBody, LPSTR pSection, LPCSTR pSuggestedFilename /* = NULL */)
{
	CString ExternalPathname;
	BOOL	bResult = FALSE;

	// Sanity 
	if (!(uid && pBody && pSection))
	{
		ASSERT (0);
		return FALSE;
	}

	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Get a filename in the "Attach" directory. THis also writes the "Attachment Converted"
	// line to the MBX file.
	//
	// If "pSuggestedFilename" IS NON-null, pass that to "CreateAttachFile".
	//
	if (pSuggestedFilename && *pSuggestedFilename)
		ExternalPathname = pSuggestedFilename;

	// Note: "CreateAttachFile()" gets a unique filename in our attach directory 
	// and creates a blank file. The full pathname of the created file is returned
	// in "ExternalPathname"
	//

	if (CreateAttachFile (pBody, ExternalPathname))
	{
		// If the body is larger than the user specified size, write a stub file,
		// otherwise download full contents.

		// Read the maximum size from the INI file (for this account).
		//
		CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

		// Use dominant if there's an error.
		if ( strPersona.IsEmpty() )
			strPersona = CRString (IDS_DOMINANT);

		CString maxSizeKey		= g_Personalities.GetIniKeyName( IDS_INI_IMAP_MAXSIZE );
		CString fullDownloadKey = g_Personalities.GetIniKeyName( IDS_INI_IMAP_OMITATTACH );

		// TRUE if user has "Full Message except attachments larger than xxx k" checked.
		BOOL	bFullDownload = FALSE;

		TCHAR szValue [512];
		*szValue = 0;

		g_Personalities.GetProfileString(strPersona, maxSizeKey, "", szValue, sizeof(szValue));
		long lMaxsize = atol (szValue);

		// See if user wants full message download:
		//
		bFullDownload = (g_Personalities.GetProfileInt(strPersona, fullDownloadKey, 0) != 0);

		// 
		// Make sure this value isn't negative.
		if ( lMaxsize < 0 )
			lMaxsize = 0;

		// Note: the INI setting setting is in kilobytes.
		lMaxsize *= 1000;

		// Must we force attachment download?
		//
		if (m_bForceAttachDownload)
		{
			// Route the body's contents into the external file.
			//
			bResult = SUCCEEDED (m_pImap->FetchAttachmentContentsToFile (uid, pSection, (LPCSTR)ExternalPathname, pBody->encoding, pBody->subtype));
		}
		// It is now safe to cast lMaxsize to an unsigned long.
		//
		else if ( bFullDownload && (pBody->size.bytes <= (unsigned long) lMaxsize) )
		{
			// Download small body.
			// Route the body's contents into the external file.
			//
			bResult = SUCCEEDED (m_pImap->FetchAttachmentContentsToFile (uid, pSection, (LPCSTR)ExternalPathname, pBody->encoding, pBody->subtype));
		}
		else
		{
			// Write stub.
			bResult = WriteBodyPartPlaceHolder (m_pImap, uid, pBody, pSection, (LPCSTR) ExternalPathname);

			// Increment counter of undownloaded attachments.
			// 
			if (bResult && m_pImapSum)
				m_pImapSum->m_nUndownloadedAttachments++;
		}

		// We can now write the "Attachment Converted" line.
		//
		if (bResult)
			WriteAttachmentLink ( ExternalPathname );
	}

	// Tell that we broke one out.
	if (bResult)
		m_bHasAttachment = TRUE;

	return bResult;
}






// BreakoutBodyToEmbeddedFile [PRIVATE]
// NOTES
// Append an "Attachment Converted:" line to the MBX file and break out the body contents,
// possibly decoding it, into a file in the "Attach" subdirectory.
// END NOTES

BOOL CImapDownloader::BreakoutBodyToEmbeddedFile (IMAPUID uid, BODY *pParentBody, BODY *pBody, LPSTR pSection, int iMRelatedID)
{
	CString ExternalPathname;
	BOOL	bResult = FALSE;
	ImapHeaderDesc hd;

	// Sanity 
	if (!(uid && pBody && pSection))
	{
		ASSERT (0);
		return FALSE;
	}

	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Ask the server for the contentID if we don't know it already.
	// jdboyd 8/24/99
	if (NULL == pBody->id)
	{
		if (!ContentIdOfSection(uid, pSection, &(pBody->id)))
		{
			// failed to get the content header
			pBody->id = NULL;
		}
	}

	// The body MUST contain a contentID, otherwise save this to an ordinary
	// Attachment Converted line.
	if (NULL == pBody->id)
	{
		return BreakoutBodyToFile (uid, pBody, pSection);
	}
	else if ( strlen (pBody->id) >= sizeof (hd.m_contentID) )
	{
		// content-ID too long.
		return BreakoutBodyToFile (uid, pBody, pSection);
	}

	// figure out the suggested filename
	CString szFilename; szFilename.Empty();
	GetFilenameParameter (pBody, szFilename);

	// Need the name into an array!!
	//
	char fName[_MAX_PATH + 1];

	*fName = 0;

	// Did we get anything??
	//
	if (!szFilename.IsEmpty())
	{
		if ( szFilename.GetLength() < sizeof (fName) )
			strcpy(fName, szFilename);
	}

	// Still nothing??
	//
	if (!*fName)
		strcpy(fName, CRString(IDS_UNTITLED));

	// Create
	JJFileMT* pEmbFile = OpenEmbeddedFile(fName);

	if (!pEmbFile)
	{
		return BreakoutBodyToFile (uid, pBody, pSection);
	}

	// Get the filename.
	CString strFilename ("????");
	BSTR bstrFilename = NULL;
	if (SUCCEEDED(pEmbFile->GetFName(&bstrFilename)))
	{
		strFilename = bstrFilename;
		::SysFreeString(bstrFilename);
		bstrFilename = NULL;
	}

	// Close the file.
	pEmbFile->Close();
	delete pEmbFile;
	pEmbFile = NULL;

	// Go write the contents to the file.
	bResult = SUCCEEDED (m_pImap->FetchAttachmentContentsToFile (uid, pSection, strFilename, pBody->encoding, pBody->subtype));

	// Set some members of the headerdesc so we can pass it to the EmbeddedElement creation 
	// routine.

	strcpy (hd.m_contentID, pBody->id);

	// Add the embedded content line.
	if (bResult)
	{
		SaveEmbeddedElement( strFilename, &hd, iMRelatedID );
	}
		
	return bResult;
}






// AppendBodyTextToMbxFile [PRIVATE]
// NOTES
// Download the contents of the body section as text and append it to the MBX file.
// END NOTES

BOOL CImapDownloader::AppendBodyTextToMbxFile (IMAPUID uid, LPSTR pSection, BODY *pBody)
{
	LPCSTR pSubtype = pBody->subtype;
	short BodyEncoding = pBody->encoding;
	BOOL bResult = FALSE;

	if (!(uid && pSection))
	{
		ASSERT (0);
		return FALSE;
	}

	// Make sure we have a CIMAP object.
	if (! (m_pImap && m_pImap->IsSelected()) )
	{
		ASSERT (0);
		return FALSE;
	}

	BOOL bIsEnriched = pSubtype && (stricmp (pSubtype, "ENRICHED") == 0);
	BOOL bIsHtml	 = pSubtype && (stricmp (pSubtype, "HTML") == 0);
	BOOL bIsFormatFlowed = FALSE;
	if (pSubtype && (stricmp (pSubtype, "PLAIN") == 0))
	{
		PARAMETER* p = pBody->parameter;
		for (; p; p = p->next)
		{
			if (p->attribute && stricmp (p->attribute, "FORMAT") == 0)
			{
				if (p->value && stricmp (p->value, "FLOWED") == 0)
				{
					bIsFormatFlowed = TRUE;
					break;
				}
			}
		}
	}

	if (bIsFormatFlowed) 
	{
		Write ("<x-flowed>\r\n", 12);

		// Tell summary!
		//
		if (m_pImapSum)
		{
			m_pImapSum->SetFlagEx(MSFEX_FLOWED);
		}
	}
	else if (bIsEnriched) 
	{
		Write ("<x-rich>\r\n", 10);

		// Tell summary!
		//
		if (m_pImapSum)
		{
			m_pImapSum->SetFlag(MSF_XRICH);
		}
	}
	else if (bIsHtml) 
	{
		Write ("<x-html>\r\n", 10);

		if (m_pImapSum)
		{
			m_pImapSum->SetFlag(MSF_XRICH);
			m_pImapSum->SetFlagEx(MSFEX_HTML);
		}
	}

	// Set the decoder based on the body type and encoding.
	SetDecoder (BodyEncoding);

	bResult = SUCCEEDED (m_pImap->DownloadAttachmentContents (uid, pSection, this));

	ResetDecoder();

	if (bIsFormatFlowed) 
	{
		Write ("</x-flowed>\r\n", 13);
	}
	if (bIsHtml) 
	{
		Write ("</x-html>\r\n", 11);
	}
	else if (bIsEnriched) 
	{
		Write ("</x-rich>\r\n", 11);
	}

	return bResult;
}



// DownloadSubMessage [PRIVATE]
// NOTES
// Handle downloading an embedded message.
// END NOTES.

BOOL CImapDownloader::DownloadSubMessage (IMAPUID uid, BODY *pBody, LPSTR pParentSection)
{
	BODY		*l_body;
	BOOL		bResult = TRUE;
	char sequence [1024];
	CString		buf;

	//
	// Note: Since this is a sub-message, pParentSection cannot be NULL.
	//
	if (! (pBody && pParentSection) )
		return FALSE;


	// Save the current ImapHeaderDesc object in the ImapDownloader and set a 
	// new one during the download of the submessage's header.
	// Make sure to reset it afterwards.
	//
	ImapHeaderDesc *pOldHeaderDesc = m_pHd;

	m_pHd = new ImapHeaderDesc;

	if (!m_pHd)
	{
		ASSERT (0);

		m_pHd = pOldHeaderDesc;
		return FALSE;
	}

	// Ok. We got one. Initialize it.
	m_pHd->Init ();

	//
	// Download the sub-message's full header to the MBX file.
	//
	// Tell writer that we are download header now. It will turnoff decoder, etc.
	DownloadingHeader();

	bResult = SUCCEEDED (m_pImap->UIDFetchRFC822Header (uid, pParentSection, this) );

	// If we succeeded, update original header if necessary.
	if (bResult)
	{
		// If any HTML found in the 
		if (m_pHd->m_isMHTML)
			pOldHeaderDesc->m_isMHTML = TRUE;
	}

	// Don't need current headerdesc any more:
	delete m_pHd;

	// Make sure to reset the header desc. object.
	m_pHd = pOldHeaderDesc;

	if (!bResult)
		return FALSE;

	// Flush the file JJFile buffer because we need to move around in the file.
	if ( FAILED (m_mbxFile.Flush()) )
	{
		return FALSE;
	}

	//
	// If there is a Content-Disposition in the Message/Rfc822 body, set that into
	// the ImapDownloader object so children can inherit it.
	// I hope the one who called this method saved the previous disposition value!!
	//
	if (pBody->disposition.type)
	{
		DISPOSITION_TYPE dType = DispositionTypeFromStr (pBody->disposition.type);
		if (dType != DISPOSITION_UNKNOWN)
			SetDisposition (dType);
	}

	// Broadcast that that's what we're doing.
	DownloadingBody();

	// See if this body has a "msg" submessage.	
	if ( pBody->nested.msg && ((pBody->nested.msg)->body) )
	{
		//
		// Recurse through the message body.
		//
		l_body = (pBody->nested.msg)->body;

		// What to do with this body??
		switch (l_body->type)
		{
			case TYPEMESSAGE:
			default:
				//
				// In either of these cases, the message/rfc822's body would have to be
				// part 1.
				//

				buf.Format ("%s.1", pParentSection);

				// Make sure we don't overrun "sequence".
				if ( buf.GetLength () >= sizeof (sequence) )
				{
					ASSERT (0);
					bResult = FALSE;
					break;
				}

				strcpy (sequence, buf);

				if (l_body->type == TYPEMESSAGE)
				{
					// Get message body and recurse through that.
					bResult = DownloadSubMessage (uid, l_body, sequence);
				}
				else
				{
					// Simple body type. Treat as first part of this message.
					DownloadSimpleBody (uid, l_body, sequence);
				}

				break;
				
			case TYPEMULTIPART:
				//
				// Note that the sub-message's body takes the same IMAP sequence 
				// as the message/rfc822 part.
				//

				DownloadMultipartBody (uid, l_body, pParentSection);
				break;
		} // switch
	}
	else // No nested "msg" or msg->body.
	{
		// Just download the message's contents as-is.
		bResult = AppendBodyTextToMbxFile (uid, pParentSection, pBody);
	}


	return bResult;
}



// CreateAttachFile [PRIVATE]
// FUNCTION
// Format an "Attachment Converted:" line and append to the MBX file.
// END FUNCTION
// NOTES
// The full pathname of the file that's parent of the "Attachment Converted:" line is
// returned in "Pathname".
// END NOTES.
BOOL CImapDownloader::CreateAttachFile (BODY *pBody, CString &Pathname)
{
	char			tmp[2048];
	char			buf[2048];
	JJFileMT*		AttachFile = NULL;
	BOOL			bResult = FALSE;

	if ( ! pBody )
		return NULL;

	// Initialize 
	*tmp = 0;


	CString szFilename; szFilename.Empty();

	// First see if a suggested filename is attached to the body part:
	//
	GetFilenameParameter (pBody, szFilename);

	// If we failed there and a filename was passed in through "Pathname",
	// use that name, otherwise, it's "Untitled".
	//

	if ( szFilename.IsEmpty() && !Pathname.IsEmpty() )
	{
		szFilename = Pathname;
	}

	// Did we get anythying??
	//
	if (!szFilename.IsEmpty())
	{
		if ( szFilename.GetLength() < sizeof (tmp) )
			strcpy(tmp, szFilename);
	}

	if (!*tmp)
		strcpy(tmp, CRString(IDS_UNTITLED));

	// Get the directory housing the MBX file so we can access it's Attach subdirectory.
	CString szMailboxDir;
	MbxFilePathToMailboxDir (GetMbxPathname (), szMailboxDir);

	// Call "OpenLocalAttachFile()" to get a unique filename..

	// Need to do this for "OpenLocalAttachFile()!!!"
	if (szMailboxDir.Right ( 1 ) != DirectoryDelimiter)
	{
		wsprintf (buf, "%s%c", szMailboxDir, DirectoryDelimiter);
	}
	else
	{
		strcpy (buf, (LPCSTR)szMailboxDir);
	}

	AttachFile = OpenLocalAttachFile(buf, tmp, true);
	if (AttachFile)
	{
		BSTR pAllocStr = NULL;

		// Put the filename into "Pathname".
		if ( SUCCEEDED (AttachFile->GetFName(&pAllocStr) ) && pAllocStr != NULL )
		{
			Pathname = pAllocStr;  // Copies;

			// Must free pAllocStr.
			SysFreeString (pAllocStr);
			pAllocStr = NULL;
		}

		AttachFile->Close();
		delete AttachFile;

		bResult = TRUE;
	}

	return bResult;
}




// WriteAttachmentLink [PRIVATE]
//
// Append an "Attachment Converted" or similar line to the MBX file.
// "pPathname" is the full path to the attachment file.
//
BOOL CImapDownloader::WriteAttachmentLink ( LPCSTR pPathname)
{
	CString szBuf;

	if (! (pPathname && *pPathname) )
		return FALSE;

	szBuf.Format ("\r\n%s\"%s\"\r\n", (const char*)CRString(IDS_ATTACH_CONVERTED), pPathname);

	// Append this string to the MBX file.
	return Write ( szBuf, szBuf.GetLength() );
}




//================ MBX file manipulation routines. ======================/


// Open file for appending. If the file doesn't exist try to create it.
// If "Truncate" is TRUE and the file exists, truncate contents before 
// reopening.
BOOL CImapDownloader::OpenMbxFile (BOOL Truncate)
{
	BOOL bResult = FALSE;

	if (m_Pathname.IsEmpty())
		return FALSE;

	// If the IMAPFile is open, close it.
	if (m_mbxFile.IsOpen() == S_OK)
		m_mbxFile.Close();

	// Re-open.
	bResult = SUCCEEDED ( m_mbxFile.Open(m_Pathname, O_RDWR | O_APPEND | O_CREAT) );

	// Truncate.
	if (bResult && Truncate)
	{
		bResult = SUCCEEDED (m_mbxFile.ChangeSize ( 0 ) );
	}

	return bResult;
}



// Close the MBX file.
BOOL CImapDownloader::CloseMbxFile ()
{
	// If the IMAPFile is open, close it.
	if (m_mbxFile.IsOpen() == S_OK)
		m_mbxFile.Close();

	return TRUE;
}

	

void CImapDownloader::SetDecoder (unsigned short Encoding)
{
	// If one exists, delete it.
	if (m_Decoder)
		delete m_Decoder;
	m_Decoder = NULL;

	// Reset this:
	m_bMustReadSingleLines = FALSE;

	switch (Encoding)
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

	// Initialize the decoder before we leave.
	if (m_Decoder)
		m_Decoder->Init();
}



// Delete any decoder.
void CImapDownloader::ResetDecoder ()
{
	// If one exists, delete it.
	if (m_Decoder)
		delete m_Decoder;
	m_Decoder = NULL;

	m_bMustReadSingleLines = FALSE;
}



//
// Manually add a set of bytes to the file.
//
BOOL CImapDownloader::Write (const char *buffer, ULONG nBytes)
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

	// Was our read buffer allocated?
	if (!m_pWriteBuffer)
	{
		m_pWriteBuffer = new char[BUFLEN + 4];
	}

	// Must have buffers:
	if (! (m_pReadBuffer && m_pWriteBuffer) )
	{
		bResult = FALSE;
		return bResult;
	}

	// If file is not yet open, do so now.
	if (m_mbxFile.IsOpen() != S_OK)
	{
		m_mbxFile.Open((LPCSTR)m_Pathname, O_CREAT | O_WRONLY);
	}

	if (m_mbxFile.IsOpen() == S_OK)
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
				if ( SUCCEEDED ( m_mbxFile.Put (m_pReadBuffer, outLen) ) )
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
			bResult = SUCCEEDED ( m_mbxFile.Put (buffer, nBytes) );
		}
	}

	return (bResult);
}


// FUNCTION
// Write the given text string followed by "\r\n"
BOOL CImapDownloader::WriteTextLine (const char *buffer /* = NULL */)
{
	BOOL bResult = FALSE;

	// If file is not yet open, do so now.
	if (S_OK != m_mbxFile.IsOpen())
	{
		m_mbxFile.Open((LPCSTR)m_Pathname, O_CREAT | O_WRONLY);
	}

	if (m_mbxFile.IsOpen() == S_OK)
	{
		bResult = TRUE;

		if (buffer)
		{
			bResult = SUCCEEDED ( m_mbxFile.Put (buffer, strlen (buffer)) );
		}

		if (bResult)
			bResult = SUCCEEDED ( m_mbxFile.Put ("\r\n", 2) );
	}

	return bResult;
}




//
// This is calls the IMAP DLL's "readfn_t" function to slurp the data, either from
// the network or from a buffer (in the case of a parenthesized string).
//
BOOL CImapDownloader::Write (readfn_t readfn, void * read_data, unsigned long size)
{
	long outLen = 0;
	BOOL bResult = TRUE;
	int nReadStatus = -1;
	BOOL bIsFirstLine = TRUE;

	// Sanity:
	if (! (readfn && read_data) )
		return FALSE;

	// Set the class reader function.
	m_pReadFn = readfn;
	m_pReadData = read_data;

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

		m_nBufferSize = BUFLEN;
	}

	// Did we get a buffer??
	if (!m_pReadBuffer)
		return FALSE;

	// Nothing in the buffer yet.
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

	// If file is not yet open, do so now.
	if (m_mbxFile.IsOpen() != S_OK)
	{
		m_mbxFile.Open((LPCSTR)m_Pathname, O_CREAT | O_WRONLY);
	}

	nReadStatus = -1;

	// What are we in the process of writing now??
	if (m_DownloadMode == DOWNLOADING_HEADER)
	{
		if (m_pHd)
		{
			// THe header should have already been initialize, Don't 
			// re-initialize it..

			// Read the header.
			nReadStatus = m_pHd->Read(this);
		}
	}
	else if ( m_DownloadMode == DOWNLOADING_BODY)
	{
		// Make sure to initialize the decoder, if any.
		if (m_Decoder)
			m_Decoder->Init();

		// Read and decode in chunks.
		long inLen;
		long nBytesHandled = 0;
		LPSTR pBuf = NULL;

		// Call IsFancy () on the first line of the text.
		bIsFirstLine = TRUE;

		// If the current subtype is TEXT and there's no decoder specified,
		// then this MAY contain embedded uuencoded or binhexed attachments (YUCK!!).
		// In that case, we need to check each line for the uuencode and binhex banners.
		// 

		// May have to do embedded uuing or hexing - i.e., we may have to search
		// for embedded uuencoded or binhexed stuff inside a TEXT part.
		//
		BOOL bCheckForEmbedded	= FALSE;
		BOOL bHexing			= FALSE;
		BOOL bUuing				= FALSE;

		// Create uudecode and binhex decoders in case we have embedded
		// stuff of one of those types.
		//
		ImapDecodeBH IMAPBHDecoder; IMAPBHDecoder.Init();
		ImapDecodeUU IMAPUUDecoder; IMAPUUDecoder.Init();

		// Tell them that they have to write the file for us:
		//
		IMAPBHDecoder.SetMustOutputFile (TRUE);
		IMAPUUDecoder.SetMustOutputFile (TRUE);

		// Tell them what our mailbox directory is:
		//
		CString szMailboxDir;
		MbxFilePathToMailboxDir (GetMbxPathname (), szMailboxDir);

		IMAPBHDecoder.SetMailboxDirectory (szMailboxDir);
		IMAPUUDecoder.SetMailboxDirectory (szMailboxDir);

		// Always check for uuing/hexing if this is a text/plain part.
		//
		CRString szPlain (IDS_MIME_PLAIN);

		if ( (m_CurrentBodyType == TYPETEXT) &&
			 (m_szCurrentBodySubtype.CompareNoCase (szPlain) == 0) )
		{
			bCheckForEmbedded = TRUE;
			m_bMustReadSingleLines = TRUE;
		}

		// Instantiate our chunk reader:
		CChunkReader chunkReader (readfn, read_data, size, m_bMustReadSingleLines);

		// Read in chunks..
		while ( nBytesHandled < m_FullSize )
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
				break;
			}

			// Update bytes we handled.
			nBytesHandled += inLen;

			// Just go handle bytes now:
			//
			outLen = inLen;

			// Note: We are using a different write buffer!!!
			// pBuf is set to point to the one to save from.
			//

			// Pass data through any decoder specified by Content-Encoding.
			//
			if (m_Decoder)
			{
				m_Decoder->Decode (pBuf, inLen, m_pWriteBuffer, outLen);

				// Use m_pReadBuffer for our second decoder (possibly).
				// Note that m_pReadBuffer is of size BUFLEN, the same size as the buffer
				// passed from GetNextChunk, so we can't overflow m_pReadBuffer.
				//
				if (outLen > 0)
				{
					memcpy (m_pReadBuffer, m_pWriteBuffer, outLen);
					pBuf = m_pReadBuffer;

					inLen = outLen;
				}
				else
				{
					pBuf = m_pWriteBuffer;
				}
			}
			else
			{
				// Write what pBuf points to!!
				//
				outLen = inLen;
			}

			// Hack alert.
			// If this is the first line of data, copy the first 20 bytes to a buffer
			// and pass it to IsFancy.
			//
			if (bIsFirstLine && (m_CurrentBodyType == TYPETEXT) )
			{
				TCHAR buf [24];
	
				memcpy (buf, pBuf, 20);
				buf[20] = '\0';

				if ( IsFancy (buf) == IS_HTML )
					m_bIsMhtml = TRUE;

				// Don't check again.
				bIsFirstLine = FALSE;
			}


			// We may have to pass our data though uudecode and binhex decoders:
			//
			// Do we need to check for embedded binhex or uuencode?
			//
			if ( bCheckForEmbedded )
			{
				if (!bUuing)
					bHexing = IMAPBHDecoder.Decode(pBuf, inLen, m_pWriteBuffer, outLen);
					
				if (!bHexing)
					bUuing = IMAPUUDecoder.Decode(pBuf, inLen, m_pWriteBuffer, outLen);

				// If we succeed in any of these, we need to

				// If we'er uuing or hexing, don't check anymore.
				//
				if (bUuing || bHexing)
				{
					bCheckForEmbedded = FALSE;

					// Chuck the current line and read the next.
					continue;
				}
				else
					outLen = inLen;
			}

			// If we'er uuing or hexing, those decoders write out the decoded contents
			// themselves.
			// Note: When they're finished decoding, they return FALSE so we can 
			// write any left over text to the MBX file.
			//
			if (bUuing)
			{
				bUuing = IMAPUUDecoder.Decode(pBuf, inLen, m_pWriteBuffer, outLen);

				// If we're no longer uuing, means we're done. Write out an
				// "Attachment converted" line.
				//
				if (!bUuing)
				{
					CString szEmbeddedFilePath = IMAPUUDecoder.GetEmbeddedFilename ();
					WriteAttachmentLink ( szEmbeddedFilePath );
				}
			}
			else if (bHexing)
			{
				bHexing = IMAPBHDecoder.Decode(pBuf, inLen, m_pWriteBuffer, outLen);

				// If we're no longer uuing, means we're done. Write out an
				// "Attachment converted" line.
				//
				if (!bHexing)
				{
					CString szEmbeddedFilePath = IMAPBHDecoder.GetEmbeddedFilename ();
					WriteAttachmentLink ( szEmbeddedFilePath );
				}
			}
			// Do our own writing.
			else
			{
				// Write to the file.

				if ( SUCCEEDED ( m_mbxFile.Put (pBuf, outLen) ) )
				{
					// If this is the last chunk and this was text, see if we wrote 
					// a \r\n. If not, do it now.
					if ( (nBytesHandled == m_FullSize) && (outLen > 0) && !m_Decoder)
					{
						if (pBuf [outLen - 1] != '\n' && pBuf [outLen - 1] != '\r')
						{
							bResult = SUCCEEDED ( m_mbxFile.Put ("\r\n", 2 ) );
						}
					}
				}
				else
				{
					// We should have some way of backing out of this if we fail after
					// a partial write.
					bResult = FALSE;
					break;	
				}
			}
		}
	}
	else
	{
		// THis is an error. Flush network.
		// Read and decode in chunks.
		long inLen;
		long i = 0;

		// Read in chunks..
		while ( i < m_FullSize )
		{
			inLen = min (m_nBufferSize, (m_FullSize - i));

			// Read and discard.
			if (!(*readfn)(read_data, inLen, m_pReadBuffer))
				break;

			// Next chunk.
			i += inLen;
		}

		bResult = FALSE;
	}

	return (bResult);
}






//
// Read a line of text from the network. Go read from the internal buffer first.
// Return the number of bytes read.
// NOTE: "Buffer" should be zeroed before passing in here.
//
int	CImapDownloader::ReadLine (char *Buffer, int nBufSize)
{
	int n;
	char *st;
	char c = '\0';
	char d;

	// Sanity
	if (!Buffer)
		return FALSE;

	// Must have space left. Otherwise, flag an error.
	if (nBufSize <= 0)
		return -1;

	// make sure have data.
	if (!Getdata ())
	{
		// No more bytes to read.
		return 0;
	}

	st = m_Ptr;				// save start of string 
	n = 0;					// init string count.

	// look for end of line.
	while (m_Vbytes--)
	{	
	    d = *m_Ptr++;	// slurp another character

		if ((c == '\015') && (d == '\012'))
		{
			// Add the \r\n also. This skips over the \012.
			n++;

			// Make sure we don't overrrun the given buffer.
			if (n + 1 > nBufSize)
				return 0;

			// Otherwise, fill the buffer.
			memcpy ( (void *)Buffer, st, n);		// Copy to buffer.

			// Make space to tie off.
			Buffer[n] = '\0';			// tie off string with null.
			return n;
		}

		n++;							// count another character searched.
		c = d;							// remember previous character.
	}

	// If we get here, we didn't find the end of line!!

	// Do we have space in the buffer??

	if (n + 1 > nBufSize)
		return 0;

	// Otherwise, write to buffer.
	memcpy (Buffer, st, n);		// Copy to buffer.

	// get more data from the net.
	if (!Getdata ())
	{
		 return -1;
	}
	else if ((c == '\015') && (*m_Ptr == '\012'))
	{
		// special case of newline broken by buffer.
		// Add \012.

		Buffer[n++] = *m_Ptr++;

		m_Vbytes--;

		Buffer [n] = '\0';					// tie off string with null

		return n;
	}
	// else recurse to get remainder
	else
	{
		return n + ReadLine (Buffer + n, nBufSize - n);
	}
}




// Getdata [PRIVATE]
// Read data into our internal buffer
//
BOOL CImapDownloader::Getdata()
{
	BOOL bResult = TRUE;
	int inLen = 0;
	    
	// If there's stuff in the buffer, return immediately.
	if (m_Vbytes > 0)
		return TRUE;

	// If we have read all bytes, just return FALSE;
	if (m_CurrentSize >= m_FullSize)
		return FALSE;

	// Otherwise, we have to read from the network.

	inLen = min (m_nBufferSize, (m_FullSize - m_CurrentSize));

	if (! (*m_pReadFn) (m_pReadData, inLen, m_pReadBuffer) )
		return FALSE;

	// Success.
	m_Vbytes = inLen;

	// Increment current size.
	m_CurrentSize += inLen;

	// Rewind pointer.
	m_Ptr = m_pReadBuffer;

	return (TRUE);
}   




//
// AddMessageSeparator [PRIVATE]
//
BOOL CImapDownloader::AddMessageSeparator ()
{
	time_t counter;
	struct tm* Time;
	char buf[2048];

	// If file is not open, some error ocurred.
	if (S_OK != m_mbxFile.IsOpen())
		return FALSE;

    // Put envelope to file
	time(&counter);
	if (counter < 0)
		counter = 1;

	Time = localtime(&counter);

	CRString Weekdays(IDS_WEEKDAYS);
	CRString Months(IDS_MONTHS);
	wsprintf(buf, CRString(IDS_FROM_FORMAT), 
			((const char*)Weekdays) + Time->tm_wday * 3,
			((const char*)Months) + Time->tm_mon * 3, Time->tm_mday, Time->tm_hour,
			Time->tm_min, Time->tm_sec, Time->tm_year + 1900);

	return SUCCEEDED (m_mbxFile.PutLine(buf));
}



// WriteXHEADERLine [PRIVATE]
// FUNCTION
// We use a X-UID header field to store the UID for a message.
// If the current m_pHd is a top-level header, it should contain a non-zero UID valule.
// If it does, append it to the MBX file.
// END FUNCTION
//
BOOL CImapDownloader::WriteXHEADERLine ()
{
	char buf[2048];

	// If file is not open, some error ocurred.
	if (S_OK != m_mbxFile.IsOpen())
		return FALSE;

	if (m_pHd && m_pHd->m_Uid != 0)
	{
		//
		// NOTE: ALways put a space after the colon.
		//
		wsprintf(buf, "%s %lu\r\n", CRString(IDS_IMAP_UID_XHEADER), m_pHd->m_Uid);

		return SUCCEEDED (m_mbxFile.Put(buf) );
	}

	// OK if we don't have a UID - means it's not a top-level header.
	return TRUE;
}



//
// Copy some info from the headerdesc into the summary.
//
void CImapDownloader::SetSummaryInfoFromHD ( CImapSum* pSum, ImapHeaderDesc *pHd)
{
	if (pSum && pHd)
	{
		//
		// Annotate various summary properties.
		//	
		if (pHd->m_hasRich)
			pSum->SetFlag(MSF_XRICH);

		if (pHd->m_isMHTML)
			pSum->SetFlagEx(MSFEX_HTML);

		if (pHd->m_isDispNotifyTo)
			pSum->SetFlag(MSF_READ_RECEIPT);
	}
}




BOOL CImapDownloader::SaveEmbeddedElement( const char * filename, ImapHeaderDesc *pHd, int nMPRelatedID )
{
	// Sanity:
	if (! (filename && pHd) )
		return FALSE;

	// currently processing a multipart/related which ^could^ be MHTML.
	// so cache the attached file info until the end of the mp/related
	// params == cid OR CB/CL
	EmbeddedElement* EE = new EmbeddedElement();
	if ( EE )
	{
		// we always save to the <eudoradir>\Embedded directory
		const char * pName = strrchr( filename, '\\' );
		if ( pName )
			pName++;			// skip the backslash
		else
			pName = filename;	// no slash; use the whole thing

		EE->Init( pName, nMPRelatedID, 
					pHd->m_contentID,
					pHd->m_contentBase, 
					pHd->m_contentLocation );

		if ( m_EmbeddedElements.AddTail(EE) )
			return TRUE;
	}

	return FALSE;
}

//
// Write the saved  embedded element lines to the MBX file.
//
BOOL CImapDownloader::WriteEmbeddedElements( BOOL isMHTML )
{
	if (! SUCCEEDED (m_mbxFile.IsOpen()) )
		return FALSE;

	POSITION pos = m_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)m_EmbeddedElements.GetNext(pos);

		if (FAILED(m_mbxFile.EnsureNewline())) goto fail;
		if ( isMHTML )
		{
			if (FAILED(m_mbxFile.Put(CRString(IDS_EMBEDDED_CONTENT)))) goto fail;
		}
		else
		{
			if (FAILED(m_mbxFile.Put(CRString(IDS_ATTACH_CONVERTED)))) goto fail;
		}
	
		if (FAILED(m_mbxFile.Put(EE->GetPath()))) goto fail;
		if (FAILED(m_mbxFile.Put(":"))) goto fail;

		char buf[ 128 ];
		wsprintf( buf, "%0.8x,%0.8x,%0.8x,%0.8x",
			EE->GetMRI(), EE->GetCIDHash(), EE->GetCLHash(), EE->GetMRI_CB_CL_Hash() );
		
		if (FAILED(m_mbxFile.Put(" "))) goto fail;
		if (FAILED(m_mbxFile.Put( buf ))) goto fail;
 

		if (FAILED(m_mbxFile.PutLine())) goto fail;
	}

	return TRUE;

fail:

	return FALSE;
}


void CImapDownloader::FlushEmbeddedElements()
{
	// clean the list
	while (m_EmbeddedElements.IsEmpty() == FALSE)
		delete m_EmbeddedElements.RemoveHead();
}




// ImapRebuildSummary [PRIVATE]
//
// FUNCTION
// Tell a CImapSum to read a newly-written header from an MBX file.
// Start reading at the current file pointer.
// END FUNCTION
//
BOOL CImapDownloader::ImapRebuildSummary (CImapSum *pSum, ImapHeaderDesc *pHd, JJFileMT *pJJFile, long lHeaderLength)
{
	BOOL bResult = FALSE;

	// Sanity
	if (! (pSum && pHd && pJJFile) )
	{
		ASSERT (0);
		return FALSE;
	}

	if (lHeaderLength <= 0)
		return FALSE;

	// Suck the whole header into a chunk of memory.
	LPTSTR pHeader = new TCHAR [lHeaderLength + 4];

	if (pHeader)
	{
		*pHeader = 0;
		long nBytes = 0;

		pJJFile->RawRead (pHeader, lHeaderLength, &nBytes);

		// Did we get any?
		// Pass the header through our in-memory parser.
		if (nBytes > 0)
		{
			// Tie off;
			pHeader[nBytes] = '\0';

			bResult = ParseHeaderInMemory (pSum, pHeader);
		}

		// Free memory.
		delete[] pHeader;

		pHeader = NULL;
	}

	return bResult;
}




// ContentIdOfSection [PRIVATE]
//
// FUNCTION
// Return the content id string describing the given section of the message uid.
// Useful when receiving embedded parts from servers other than UofW.
// END FUNCTION
//
// jdboyd 8/24/99
BOOL CImapDownloader::ContentIdOfSection(IMAPUID uid, char *sequence, char **ContentID)
{
	BOOL bResult;
	CStringWriter stringWriter;

	bResult = SUCCEEDED (m_pImap->FetchMIMEHeader (uid, sequence, &stringWriter));
	if (bResult)
	{
		// Get a pointer to the in-memory headers. Note, these areseparated by "\r\n"
		LPCSTR pHeader = stringWriter.GetString();
		if (pHeader)
		{
			CRString contentIDHeader(IDS_MIME_HEADER_CONTENT_ID);
			CString pLowerHeader(pHeader);

			contentIDHeader.MakeLower();
			pLowerHeader.MakeLower();

			// find the first occurence of Content-ID in the lower case version of the headers:
			char *scan = strstr(pLowerHeader, contentIDHeader);
			if (scan)
			{
				// point to the Content-ID: header in the real headers now
				scan = (char *)pHeader + (scan - pLowerHeader);

				// skip over the Content-ID header ...
				scan += contentIDHeader.GetLength();

				// skip over any white space ...
				while (isspace(*scan)) scan++;

				// find the end of the Content-ID: string ...
				char *end = strstr(scan, "\r\n");
				if (end)
				{
					// copy the Content-ID string to a new string to be passed back
					int len = end - scan;

					*ContentID = (char *)calloc (1, len);	// this is the way the imap code gets memory for the BODY structure parts.
					strncpy(*ContentID, scan, (len - 1));
					(*ContentID)[len-1] = NULL;
				}
			}
		}
	}

	return (bResult);
}

#endif // IMAp4





//=========================== CAttachStubFile class implementation =============================//

CAttachStubFile::CAttachStubFile (LPCSTR pFilePath)
{
	m_szFilePath = pFilePath;

	// Initialize numeric types.
	m_Uid			= 0;
	m_Uidvalidity	= 0;
	m_AccountID		= 0;
	m_SizeInBytes	= 0;
	m_SizeInLines	= 0;
}

CAttachStubFile::~CAttachStubFile ()
{

}



// WriteBodyPartPlaceHolder [PUBLIC]
//
// FUNCTION
// Open the given file and write the data that would allow us to retrieve 
// the contents of the body part from the IMAP server.
//
// The data is as foll:
// - 4 arbitrary (magic) long integers,		16 bytes.
// - Personality hash (account ID)			4 bytes.
// - FULL Imap name of the mailbox			256 bytes.
// - Mailbox current UIDVALIDITY			4 bytes
// - Message UID value						4 bytes
// - Body part spec (e.g. 1.2.HEADER)		256 bytes.
// - Body type string (text, image, etc.)   48 bytes
// - Body subtype string					48 bytes
// - Encoding as a string (base64, etc)		48 bytes
// - Body size in lines (text only)			4 bytes
// - Body size in bytes						4 bytes
// - Body ID as a string					256 bytes
// - Body description						256 bytes.
// 
// Returns TRUE if success.
// END FUNCTION
//
// 
BOOL CAttachStubFile::WriteBodyPartPlaceHolder ()
{
	char			buf[1024];
	JJFileMT		AttachFile;
	BOOL			bResult = FALSE;

	// Initialize 
	*buf = 0;

	//
	// Verify that the info is valid.
	//
	if ( m_szFilePath.IsEmpty()	||
		 m_AccountID == 0		||
		 m_szImapName.IsEmpty() || 
		 (0 == m_Uidvalidity)	||
		 (0 == m_Uid)			||
		 m_szType.IsEmpty()		|| 
		 m_szEncoding.IsEmpty() || 
		 m_szSubType.IsEmpty()
	   )
	{
		ASSERT (0);
		return FALSE;
	}

	// Try to open the file.
	if ( FAILED (AttachFile.Open (m_szFilePath, O_WRONLY | O_CREAT) ) )
		return FALSE;

	// Make sure it's empty.
	AttachFile.ChangeSize (0);

	//
	// Write the contents. Note: we use the for() so we can break out easily.
	// No looping.
	//
	for (int i = 0; i < 1; i++)
	{
		// Magic numbers as the first 16 bytes.
		if (FAILED ( AttachFile.Put (_MAGICNUMBER1) ) )
			break;
		if (FAILED ( AttachFile.Put (_MAGICNUMBER2) ) )
			break;
		if (FAILED ( AttachFile.Put (_MAGICNUMBER3) ) )
			break;
		if (FAILED ( AttachFile.Put (_MAGICNUMBER4) ) )
			break;

		// Personality hash. This is same as the account ID.
		if (FAILED ( AttachFile.Put (m_AccountID) ) )
			break;

		// Full IMAP name.
		memset(buf, 0, AtfImapNameSize);
		if ( m_szImapName.GetLength() < sizeof (buf) )
		{
			strcpy (buf, m_szImapName);
		}
		if ( FAILED (AttachFile.Put(buf, AtfImapNameSize) ) )
			break;

		// UIDVALIDITY.
		if (FAILED ( AttachFile.Put (m_Uidvalidity) ) )
			break;

		// MESSAGE UID.
		if (FAILED ( AttachFile.Put (m_Uid) ) )
			break;

		// Body part spec.
		memset(buf, 0, AtfImapSectionSize);

		if ( strlen (m_szSection) < sizeof (buf) )
		{
			strcpy (buf, m_szSection);
		}
		if ( FAILED (AttachFile.Put(buf, AtfImapSectionSize) ) )
			break;

		// Body type.
		memset(buf, 0, AtfImapTypeSize);

		if ( m_szType.GetLength() < sizeof (buf) )
		{
			strcpy (buf, m_szType);
		}
		if ( FAILED (AttachFile.Put(buf, AtfImapTypeSize) ) )
			break;

		// Body subtype.
		memset(buf, 0, AtfImapSubtypeSize);

		if ( m_szSubType.GetLength() < sizeof (buf) )
		{
			strcpy (buf, m_szSubType);
		}
		if ( FAILED (AttachFile.Put(buf, AtfImapSubtypeSize) ) )
			break;

		// Body encoding.
		memset(buf, 0, AtfImapEncSize);

		if ( m_szEncoding.GetLength() < sizeof (buf) )
		{
			strcpy (buf, m_szEncoding);
		}
		if ( FAILED (AttachFile.Put(buf, AtfImapEncSize) ) )
			break;

		// Body size in bytes.
		if (FAILED ( AttachFile.Put (m_SizeInBytes) ) )
			break;

		// Body size in lines.
		if (FAILED ( AttachFile.Put (m_SizeInLines) ) )
			break;

		// Body ID. Note: this may be blank if no body->id.
		memset(buf, 0, AtfImapIDSize);
		
		if ( m_szBodyId.GetLength() < sizeof (buf) )
		{
			strcpy (buf, m_szBodyId);
		}
		if ( FAILED (AttachFile.Put(buf, AtfImapIDSize) ) )
			break;

		// Body description. Note: this may be blank if no body->description.
		memset(buf, 0, AtfImapDescSize);
		
		if ( m_szBodyDesc.GetLength() < sizeof (buf) )
		{
			strcpy (buf, m_szBodyDesc);
		}

		if ( FAILED (AttachFile.Put(buf, AtfImapDescSize) ) )
			break;


		// If we get here, we're OK.
		bResult = TRUE;

		// Don't loop.
		break;
	}

	// Flush and close the file.
	AttachFile.Close();

	return bResult;
}



//////////////////////////////////////////////////////////////
// ReadBodyPartPlaceHolder [PUBLIC]
// 
// FUNCTION
// Read from the file into the internal member variables
// END FUNCTION
//////////////////////////////////////////////////////////////

BOOL CAttachStubFile::ReadBodyPartPlaceHolder ()
{
	char			buf[1024];
	JJFileMT		AttachFile;
	BOOL			bResult = FALSE;

	// Initialize 
	*buf = 0;

	// Make sure the file exists.
	if ( !SUCCEEDED ( AttachFile.Open(m_szFilePath, O_RDONLY) ) )
		return FALSE;

	// 
	// Read from the file. Again, we're NOT looping.
	//

	for (int i = 0; i < 1; i++)
	{
		// Read the four magic numbers and make sure they are valid.
		unsigned long ulMagic1, ulMagic2, ulMagic3, ulMagic4;

		// Magic numbers
		if (!SUCCEEDED ( AttachFile.Get(&ulMagic1) ) ) break;
		if (!SUCCEEDED ( AttachFile.Get(&ulMagic2) ) ) break;
		if (!SUCCEEDED ( AttachFile.Get(&ulMagic3) ) ) break;
		if (!SUCCEEDED ( AttachFile.Get(&ulMagic4) ) ) break;

		// Verify:
		if (_MAGICNUMBER1 != ulMagic1) break;
		if (_MAGICNUMBER2 != ulMagic2) break;
		if (_MAGICNUMBER3 != ulMagic3) break;
		if (_MAGICNUMBER4 != ulMagic4) break;

		// AccountID:
		if (!SUCCEEDED ( AttachFile.Get(&m_AccountID) ) ) break;

		// Imap name. 256 bytes.
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapNameSize) ) ) break;
		m_szImapName = buf;

		// Uidvalidity
		if (!SUCCEEDED ( AttachFile.Get(&m_Uidvalidity) ) ) break;

		// Uid
		if (!SUCCEEDED ( AttachFile.Get(&m_Uid) ) ) break;

		// Body part section (256 bytes)
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapSectionSize) ) ) break;
		m_szSection = buf;

		// Body type. 48 bytes.
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapTypeSize) ) ) break;
		m_szType = buf;

		// Body subtype. 48 bytes.
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapSubtypeSize) ) ) break;
		m_szSubType = buf;

		// Body encoding. 48 bytes.
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapEncSize) ) ) break;
		m_szEncoding = buf;

		// Size in bytes.
		if (!SUCCEEDED ( AttachFile.Get(&m_SizeInBytes) ) ) break;

		// Size in lines.
		if (!SUCCEEDED ( AttachFile.Get(&m_SizeInLines) ) ) break;

		// Body ID. 256 bytes.
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapIDSize) ) ) break;
		m_szBodyId = buf;

		// Body description. 256 bytes.
		if (! SUCCEEDED (AttachFile.Read(buf, AtfImapDescSize) ) ) break;
		m_szBodyDesc = buf;

		// Seems like we succeeded.
		bResult = TRUE;

		break;
	}

	// Close the file.
	AttachFile.Close();

	return bResult;
}




// ================= Exported functions.===========================//



////////////////////////////////////////////////////////////////////////////////////
// WriteBodyPartPlaceHolder [PRIVATE]
//
// FUNCTION
// Open the given file  and write the data that would allow us to retrieve 
// the contents of the body part from the IMAP server.
//
// The data is as foll:
// - 4 arbitrary (magic) long integers,		16 bytes.
// - Personality hash (account ID)			4 bytes.
// - FULL Imap name of the mailbox			256 bytes.
// - Mailbox current UIDVALIDITY			4 bytes
// - Message UID value						4 bytes
// - Body part spec (e.g. 1.2.HEADER)		256 bytes.
// - Body type string (text, image, etc.)   48 bytes
// - Body subtype string					48 bytes
// - Encoding as a string (base64, etc)		48 bytes
// - Body size in lines (text only)			4 bytes
// - Body size in bytes						4 bytes
// - Body ID as a string					256 bytes
// - Body description						256 bytes.
// 
// Returns TRUE if success.
// END FUNCTION
//
// 
////////////////////////////////////////////////////////////////////////////////////// 
BOOL WriteBodyPartPlaceHolder (CIMAP *pImap, IMAPUID Uid, BODY *pBody, LPCSTR pSection, LPCSTR pFilePath)
{
	BOOL			bResult = FALSE;

	// Sanity.
	if (!(pImap && pSection && pBody && pFilePath))
	{
		ASSERT (0);
		return FALSE;
	}

	//
	// Instantiate a CAttachStubFile to do the writing.
	//
	CAttachStubFile StubFile (pFilePath);

	//
	// Set info into the object.
	//

	// Uid
	StubFile.m_Uid = Uid;

	// Section
	StubFile.m_szSection = pSection;

	//
	// Fetch stuff from the CIMAP object.
	//
	StubFile.m_AccountID		= pImap->GetAccountID ();
	StubFile.m_szImapName		= pImap->GetImapName ();

	if ( !SUCCEEDED ( pImap->GetUidValidity(StubFile.m_Uidvalidity) ) )
	{
		return FALSE;
	}

	//
	// Stuff from the body.
	//

	// Body type.
	BodyTypeCodeToString (pBody->type, StubFile.m_szType);

	// Get a string equivalent of the encoding.
	BodyEncodingCodeToString (pBody->encoding, StubFile.m_szEncoding);

	//
	// Subtype.
	if (pBody->subtype)
		StubFile.m_szSubType = pBody->subtype;

	// Body id.
	if (pBody->id)
		StubFile.m_szBodyId = pBody->id;

	// Body description.
	if (pBody->description)
		StubFile.m_szBodyDesc = pBody->description;

	// Size in bytes and lines
	StubFile.m_SizeInBytes = pBody->size.bytes;
	StubFile.m_SizeInLines = pBody->size.lines;

	return StubFile.WriteBodyPartPlaceHolder();
}



//////////////////////////////////////////////////////////////////////////////////////////
// ImapDownloaderFetchAttachment
//
// FUNCTION
// Read the first 4 bytes of the given file to see if it's an IMAP attachment file.
// If it is, read the rest of the data into a CAttachStubFile object, then retrieve the
// contents from the server.
// NOTE: Return FALSE only if the download failed.
// END FUNCTION
//////////////////////////////////////////////////////////////////////////////////////////
BOOL ImapDownloaderFetchAttachment (LPCSTR Filename)
{
	JJFileMT	AttachFile;
	BOOL		bResult = FALSE;

	// This must be called in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	if (!Filename)
	{
		ASSERT (0);
		return FALSE;
	}

	// Put up wait cursor during this.
	CWaitCursor wc;

	if ( ::IsMainThreadMT() )
	{
		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}

	// Attempt to open the file.
	// Make sure the file exists.
	if ( !SUCCEEDED ( AttachFile.Open(Filename, O_RDONLY | O_BINARY) ) )
		return TRUE;

	// Reads first 4 bytes.
	unsigned long ulMagic1, ulMagic2, ulMagic3, ulMagic4;

	// Magic numbers
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic1) ) )	return TRUE;
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic2) ) )	return TRUE;
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic3) ) )	return TRUE;;
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic4) ) )	return TRUE;;

	// Verify:
	if (_MAGICNUMBER1 != ulMagic1) return TRUE;
	if (_MAGICNUMBER2 != ulMagic2) return TRUE;
	if (_MAGICNUMBER3 != ulMagic3) return TRUE;
	if (_MAGICNUMBER4 != ulMagic4) return TRUE;

	//
	// Ok. It's one of ours. Close the file back and instantiate a CAttachStubFile
	// object to get all it's info.
	//
	AttachFile.Close();

	CAttachStubFile StubFile (Filename);

	// Read the info.
	if ( !StubFile.ReadBodyPartPlaceHolder() )
		return FALSE;

	// Get encoding as a code.
	short encoding = BodyEncodingStringToCode (StubFile.m_szEncoding);

	// See if a toc corresponding to the Imap name and account
	// is loaded and viewable.  
	//
	CTocDoc* doc = NULL;
	POSITION pos = TocTemplate->GetFirstDocPosition();
	while (pos != NULL)
	{
		doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc->m_pImapMailbox && doc->m_pImapMailbox->GetImapName())
		{
			// Must make sure the account's are the same!!
			if ( StubFile.m_AccountID == doc->m_pImapMailbox->GetAccountID() )
			{
				// Compare filename. There is a chance for error here because we do
				// a case-sensitive compare. We are screwed whether we do case-sensitive or
				// case-insensitive because servers can be either.
				//
				if (StubFile.m_szImapName.Compare (doc->m_pImapMailbox->GetImapName()) == 0)
				{
					// Found it.
					break;
				}
			}
		
			doc = NULL;
		}
    }

	// If we found one, ask it's IMAP object to fetch the attachment for us.
	if (doc != NULL)
	{
		if (doc->m_pImapMailbox)
		{
			BOOL bWasInProgress = FALSE;

			// If we get here, put up a progress bar to show downloading of contents.
			//
			if ( ::IsMainThreadMT() )
			{
				if (InProgress)
				{
					bWasInProgress = TRUE;
					PushProgress();
				}

				MainProgress(CRString(IDS_IMAP_DOWNLOADING_ATTACHMENT));
			}

			// We need to do this. Yuck.
			char buf [256];
			*buf = 0;

			if ( ! (StubFile.m_szSection.IsEmpty() ) )
			{
				strcpy (buf, StubFile.m_szSection);
			}

			bResult = SUCCEEDED ( doc->m_pImapMailbox->FetchAttachmentContentsToFile (
							StubFile.m_Uid,
							buf,
							StubFile.m_szFilePath,
							encoding, 
							StubFile.m_szSubType) );

			// If we failed, go re-write the mail stub so we can try next time:
			//
			if (!bResult)
			{
				StubFile.WriteBodyPartPlaceHolder();
			}

			// Close progress window if we had one.
			//
			if ( ::IsMainThreadMT() )
			{
				if (bWasInProgress)
					PopProgress ();
				else
					CloseProgress();
			}
		}
	}
	else
	{
		// Instantiate a CImapMaibox ourselves.
	}

	return bResult;
}





//////////////////////////////////////////////////////////////////////////////////////////
// IsImapDownloaded
//
// FUNCTION
// Return TRUE if the attachment has been downloaded.
// END FUNCTION
//////////////////////////////////////////////////////////////////////////////////////////
BOOL IsImapDownloaded (LPCSTR Filename)
{
	JJFileMT	AttachFile;
	BOOL		bResult = FALSE;

	if (!Filename)
	{
		ASSERT (0);
		return FALSE;
	}

	// Attempt to open the file.
	// Make sure the file exists.
	if ( !SUCCEEDED ( AttachFile.Open(Filename, O_RDONLY | O_BINARY) ) )
		return FALSE;

	// Reads first 4 bytes.
	unsigned long ulMagic1, ulMagic2, ulMagic3, ulMagic4;

	// Magic numbers
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic1) ) )	return TRUE;
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic2) ) )	return TRUE;
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic3) ) )	return TRUE;;
	if (!SUCCEEDED ( AttachFile.Get(&ulMagic4) ) )	return TRUE;;

	// Verify:
	if (_MAGICNUMBER1 != ulMagic1) return TRUE;
	if (_MAGICNUMBER2 != ulMagic2) return TRUE;
	if (_MAGICNUMBER3 != ulMagic3) return TRUE;
	if (_MAGICNUMBER4 != ulMagic4) return TRUE;

	// If we get here, then we found all our magic number, so the attachment's NOT
	// been downloaded.

	return FALSE;
}



//======================== Internal functions =================================//

// GetFilenameParameter [INTERNAL]
//
// Search the Content-Disposition parameter list for a "filename" parameter,
// and if found, return that. If not, search in the Content-Type 
// parameter list.
//
void GetFilenameParameter (BODY *pBody, CString& szFilename)
{
	CRString szMimeName (IDS_MIME_NAME);
	CRString szMimeFilename(IDS_MIME_CONTENT_DISP_FILENAME);

	// Initialize in case we fail:
	szFilename.Empty();

	// Must have a body:
	if (!pBody)
		return;


	PARAMETER *pParam = NULL;

	// If there is a content-disposition, use that.
	pParam = pBody->disposition.parameter;
	while (pParam)
	{
		if (  pParam->attribute && 
			 (stricmp ( pParam->attribute, szMimeFilename ) == 0) )
		{
			if (pParam->value)
			{
				// Copy:
				szFilename = pParam->value;
				break;
			}
		}

		pParam = pParam->next;
	}

	// Didi we get a name?
	if ( szFilename.IsEmpty() )
	{
		// If there is a parameter list from content-type:
		pParam = pBody->parameter;
		while (pParam)
		{
			if ( pParam->attribute && 
				 ( stricmp (pParam->attribute, szMimeName) == 0 ) )
			{
				if (pParam->value)
				{
					szFilename = pParam->value;
					break;
				}
			}

			pParam = pParam->next;
		}
	}

	// 
	if (!szFilename.IsEmpty())
	{
		szFilename.TrimRight();
		szFilename.TrimLeft();
	}
}





BOOL FillSummaryFromEnvelope (ENVELOPE *pEnv, CImapSum *pSum)
{
	char *p = NULL;

	// Sanity:
	if (! (pEnv && pSum) )
		return FALSE;


	// Do we have a date??
	if (pEnv->date)
	{
		pSum->m_Seconds = GetTime(pEnv->date, FALSE);

		// Stores the raw date header value so "FormaatDate()" can extract the
		// timezone.
		//
		pSum->m_RawDateString = pEnv->date;
	}

	// From:
	if (pEnv->from)
	{
		// Find a field to use.
		p = pEnv->from->personal;
		if (!p)
			p = pEnv->from->mailbox;

		if (p)
		{
			pSum->SetFrom(p);
		}
	}

	// If no from and there's a reply-to, use that:
	if (pEnv->reply_to && !*pSum->m_From)
	{
		// Find a field to use.
		p = pEnv->reply_to->personal;
		if (!p)
			p = pEnv->reply_to->mailbox;

		if (p)
		{
			pSum->SetFrom(p);
		}
	}

	// Subject
	if (pEnv->subject && !*pSum->m_Subject)
	{
		pSum->SetSubject(pEnv->subject);
	}

	return TRUE;
}





// FUNCTION
// Given a body structure, determine if it contains a text/html or text/exriched part.
// It we find a text/html part, return.
// If we find a text enriched, or in any other case, we have to wade through.
// Return FALSE if we need to recurse further into the body structure.
// END FUNCTION

void  DoWeAddXrichOrXhtmlTokens (BODY *pBody, BOOL& bFoundRich, BOOL& bFoundHtml, BOOL bIsTopBody /* = TRUE */)
{
	// Must have a body.
	if (!pBody)
		return;

	// Set defaults on first entry
	if (bIsTopBody)
	{
		bFoundHtml = FALSE;
		bFoundRich = FALSE;
	}

	switch (pBody->type)
	{
		case TYPETEXT:
			if ( pBody->subtype && stricmp (pBody->subtype, CRString(IDS_MIME_HTML)) == 0)
			{
				bFoundHtml = TRUE;
				return;
			}
			else if (pBody->subtype &&
				stricmp (pBody->subtype, CRString(IDS_MIME_RICHTEXT)) == 0)
			{
				bFoundRich = TRUE;
			}

			break;

		case TYPEMULTIPART:
			PART *pPart = pBody->nested.part;
			while (pPart)
			{
				BODY *pSubBody = &pPart->body;

				DoWeAddXrichOrXhtmlTokens (pSubBody, bFoundRich, bFoundHtml, FALSE);

				// If not, loop through siblings.
				pPart = pPart->next;
			}

			break;

	}

	// If we get here, we need to recurse,
	return;
}



//
// Write the text equivalent of a body type code into szType.
// An unknown type code will leave the string blank.
// 
void BodyTypeCodeToString (short Type, CString& szType)
{
	// Do this first.
	szType.Empty();

	switch (Type)
	{
		case TYPETEXT:
			szType = "TEXT";
			break;

		case TYPEIMAGE:
			szType = "IMAGE";
			break;

		case TYPEAUDIO:
			szType = "AUDIO";
			break;

		case TYPEVIDEO:
			szType = "VIDEO";
			break;

		case TYPEAPPLICATION:
			szType = "APPLICATION";
			break;

		case TYPEMESSAGE:
			szType = "MESSAGE";
			break;

		case TYPEMULTIPART:
			szType = "MULTIPART";
			break;

		case TYPEMODEL:
			szType = "MODEL";
			break;
	}
}


//
// Write the text equivalent of a body encoding code into szEncoding.
// An unknown encoding will leave the string blank.
//
void BodyEncodingCodeToString (short Encoding, CString& szEncoding)
{
	// Do this first.
	szEncoding.Empty();

	switch (Encoding)
	{
		case ENC7BIT:
		default:
			szEncoding = "7BIT";
			break;

		case ENC8BIT:
			szEncoding = "8BIT";
			break;

		case ENCBINARY:
			szEncoding = "BINARY";
			break;

		case ENCBASE64:
			szEncoding = "BASE64";
			break;

		case ENCQUOTEDPRINTABLE:
			szEncoding = "QUOTEDPRINTABLE";
			break;

		case ENCUUENCODE:
			szEncoding = "UUENCODE";
			break;

		case ENCBINHEX:
			szEncoding = "BINHEX";
			break;
	}
}



//
// Return the code equivalent to the given encoding string.
// The default is ENC7BIT.
//
short BodyEncodingStringToCode (LPCSTR pEncoding)
{
	short encoding = ENC7BIT;

	if (!pEncoding)
		return encoding;

	// Do string compares.
	CString szEncoding = pEncoding;

	if (szEncoding.CompareNoCase ("8BIT") == 0)
		encoding = ENC8BIT;
	else if (szEncoding.CompareNoCase ("BINARY") == 0)
		encoding = ENCBINARY;
	else if (szEncoding.CompareNoCase ("BASE64") == 0)
		encoding = ENCBASE64;
	else if (szEncoding.CompareNoCase ("QUOTEDPRINTABLE") == 0)
		encoding = ENCQUOTEDPRINTABLE;
	else if (szEncoding.CompareNoCase ("UUENCODE") == 0)
		encoding = ENCUUENCODE;
	else if (szEncoding.CompareNoCase ("BINHEX") == 0)
		encoding = ENCBINHEX;

	return encoding;
}



// DispositionTypeFromStr 
// 
// Set the current enumerated Disposition type that will apply to downstream
// body parts unless overridden.
//
DISPOSITION_TYPE DispositionTypeFromStr (char *Value)
{
	CString Val = Value;
	DISPOSITION_TYPE Ret = DISPOSITION_UNKNOWN;

	// Strip leading and trailing space.
	Val.TrimRight ();
	Val.TrimLeft ();

	// Did we get anything?
	if ( Val.IsEmpty () )
		return Ret;

	// These are the only values we consider:
	CRString szInline (IDS_MIME_CONTENT_DISP_INLINE);
	CRString szAttachment (IDS_MIME_CONTENT_DISP_ATTACH);

	if ( Val.CompareNoCase (szInline) == 0 )
		Ret = DISPOSITION_INLINE;
	else if ( Val.CompareNoCase (szAttachment) == 0 )
		Ret = DISPOSITION_ATTACHMENT;

	// else we don't know.

	return Ret;
}



// Given a Multipart/xxx header line, return TRUE if the subtype is 
// alternative or related.
//
static BOOL IsMPAlternativeOrRelated (const char *Buffer)
{
	if (!Buffer)
		return FALSE;

	char *p = strstr( Buffer, CRString(IDS_MIME_MULTIPART) );

	if (!p)
		return FALSE;

	p = strchr (Buffer, '/');
	if (!p)
		return FALSE;

	// Skip it.
	p++;

	// Skip leading while spaces;
	while ( *p && (*p == ' ' || *p == '\t') )
		p++;

	CRString szRelated (IDS_MIME_RELATED);
	CRString szAlternative (IDS_MIME_ALTERNATIVE);

	if (!strnicmp ( p, szRelated, szRelated.GetLength() ) )
		return TRUE;

	if (!strnicmp ( p, szAlternative, szAlternative.GetLength() ) )
		return TRUE;

	return FALSE;
}


