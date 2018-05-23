// IMAPHEADER.CPP
//
// Routines for looking at mail headers - proper version (JOK).
//

#include "stdafx.h"


#ifdef IMAP4 // Only for IMAP.


#include <QCUtils.h>
#include "resource.h"
#include "ems-wglu.h"
#include "trnslate.h"

#include "cursor.h"
#include "fileutil.h"
#include "header.h"
#include "mime.h"
#include "pop.h"
#include "resource.h"
#include "rs.h"
#include "address.h"


#include "ImapExports.h"
#include "imap.h"
#include "imapgets.h"
#include "ImapLex822.h"
#include "imaputil.h"
#include "ImapDownload.h"
#include "ImapHeader.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


ImapHeaderDesc::ImapHeaderDesc()
{

	// This is for the translator
	m_TLMime = new CTLMime();


	Init();
}

ImapHeaderDesc::~ImapHeaderDesc()
{
	while (Attributes.IsEmpty() == FALSE)
		delete Attributes.RemoveHead();

	delete (m_TLMime);
}

void ImapHeaderDesc::Init( )
{
	m_state = ExpectHeaderName;
	m_hFound = hLimit;
	*m_contentEnco = *m_receivedDate = *m_messageID = *m_status = *m_subj = *m_who = 0;

	// Initialize the type and subtype, version
	GetIniString(IDS_MIME_TEXT,	 m_contentType,    sizeof(m_contentType));
	GetIniString(IDS_MIME_PLAIN, m_contentSubType, sizeof(m_contentSubType));
	GetIniString(IDS_MIME_VERSION, m_mimeVersion,  sizeof(m_mimeVersion));

	// Not yet set..
	m_pMbxFile = NULL;
	
	// Initialize this.

#if 0 // Can't initialize this yet!!
	m_pMbxFile->Tell(&m_lDiskStart);
	ASSERT(m_lDiskStart >= 0);
#endif 

	// Don;t know this yet.
	m_lDiskEnd = m_lDiskStart = 0;

	m_grokked = TRUE;
	m_isMIME = FALSE;
	m_isMHTML = FALSE;
	m_hasRich = FALSE;
	m_isPartial = FALSE;
	m_isDispNotifyTo = FALSE;

	m_contentID[0] = '\0';
	m_contentLocation[0] = '\0';
	m_contentBase[0] = '\0';

	// Initialize Plain
	m_TLMime->CreateMimeType();

	// Set this to zero initially.
	m_Uid = 0;
}

int ImapHeaderDesc::Read(CImapDownloader *pRW)
{
	const int IntHeaderSize = 32;
	char interesting[hLimit][IntHeaderSize];
	short h;
	ImapLex822State l822s(pRW);
	Token822Enum tokenType;
	long lLastNewline;
	long lastParam;

#define EXPECT_TEXT l822s.m_State = CollectText; m_state = ExpectText;

	// Grab the interesting header labels
	GetIniString(IDS_MIME_HEADER_CONTENT_TYPE,		interesting[hContentType], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_CONTENT_ENCODING,	interesting[hContentEncoding], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_CONTENT_DISP,		interesting[hContentDisposition], IntHeaderSize);
	GetIniString(IDS_MIME_CONTENT_DESCRIPTION,		interesting[hContentDescription], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_CONTENT_BASE,		interesting[hContentBase], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_CONTENT_ID,		interesting[hContentID], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_CONTENT_LOCATION,	interesting[hContentLocation], IntHeaderSize);
	GetIniString(IDS_HEADER_STATUS,					interesting[hStatus], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_VERSION,			interesting[hMimeVersion], IntHeaderSize);
	GetIniString(IDS_POP_RECEIVED_HEADER,			interesting[hReceived], IntHeaderSize);
	GetIniString(IDS_POP_MESSAGE_ID_HEADER,			interesting[hMessageID], IntHeaderSize);
	GetIniString(IDS_HEADER_SUBJECT,				interesting[hSubject], IntHeaderSize);
	GetIniString(IDS_HEADER_FROM,					interesting[hWho], IntHeaderSize);
	GetIniString(IDS_MIME_HEADER_READR,				interesting[hDispNotifyTo], IntHeaderSize);

	
    // Remove the colons
	for (h = hContentType; h < hLimit; h++)
    	interesting[h][strlen(interesting[h]) - 1] = 0;

	// Initialize this here!!.
	m_pMbxFile = &pRW->m_mbxFile;;
	
	m_pMbxFile->Tell(&m_lDiskStart);
	ASSERT(m_lDiskStart >= 0);
	lLastNewline = m_lDiskEnd = m_lDiskStart;

	// Read the header
	for (tokenType = l822s.Lex822(); ; tokenType = l822s.Lex822())
	{
		// Write the token to disk
		tokenType = l822s.WriteHeaderToken(tokenType);
		if (tokenType == Comment)
			tokenType = LinearWhite;
		
		// First, we handle a few special token types
		switch (tokenType)
		{
		// any of these things terminate the header collection
		case EndOfHeader:
		case EndOfMessage:
		case ErrorToken:
			goto out;
			break;
			
		// If we have a return, we've hit the end of a header field the header
		// can only legally end in ExpectText or ExpectSem or ExpectVersion
		case Special:
			if (l822s.m_Token[0] == '\r')
				break;		// Skip first part of newline
			else if (l822s.m_Token[0] == '\n')
			{
				if (m_state != ExpectText && m_state != ExpectSem && m_state != ExpectVersion)
					m_grokked = FALSE;
				m_state = ExpectHeaderName;


				m_pMbxFile->Tell(&lLastNewline);
				ASSERT(lLastNewline >= 2);
				lLastNewline -= 2;

				break;
			}
			// Fall through
				
		default:
			// We handle the remaining tokens on a state-by-state basis
			switch (m_state)
			{
			//********************************
			case ExpectHeaderName:
				switch (tokenType)
				{
				case LinearWhite:
					break;	// Ignore
							
				case Atom:
					for (h = 0; h < hLimit; h++)
						if (!stricmp(interesting[h], (char*)l822s.m_Token))
							break;
					m_hFound = (InterestHeaderEnum)h;
					m_state = ExpectColon;
					if (h > hMimeVersion)
						m_grokked = FALSE;
					break;
							
				default:
					m_grokked = FALSE;
					EXPECT_TEXT;
					break;
				}
				break;
						
			//********************************
			case ExpectColon:
				switch (tokenType)
				{
				case LinearWhite:
					break; // ignore
							
				case Special:
					if (l822s.m_Token[0] == ':')
					{
						switch (m_hFound)
						{
						case hContentType:
							m_state = ExpectType;
							break;
						case hContentEncoding:
							m_state = ExpectEnco;
							break;
						case hContentDisposition:
							strncpy(m_attributeName,
								CRString(IDS_MIME_HEADER_CONTENT_DISP),
								sizeof(m_attributeName) - 1);

							m_state = ExpectValue;
							break;
						case hMimeVersion:
							m_state = ExpectVersion;
							break;
						default:
							if (m_hFound > hMimeVersion)
								m_grokked = FALSE;
							EXPECT_TEXT;
							break;
						}
					}
					break;
							
				default:
					m_grokked = FALSE;
					EXPECT_TEXT;
					break;
				}
				break;
			//********************************
			case ExpectText:
				switch (m_hFound)
				{
				case hStatus:
					strncpy(m_status, ::TrimWhitespaceMT((char*)l822s.m_Token), sizeof(m_status) - 1);
					m_status[sizeof(m_status) - 1] = 0;
					break;
				case hReceived:
					{
						char* comma = strchr((const char*)l822s.m_Token, ',');
						if (comma && !*m_receivedDate)
						{
							strncpy(m_receivedDate, ::TrimWhitespaceMT(comma + 1), sizeof(m_receivedDate) - 1);
							m_receivedDate[sizeof(m_receivedDate) - 1] = 0;
						}
					}
					break;
				case hMessageID:
					strncpy(m_messageID, ::TrimWhitespaceMT((char*)l822s.m_Token), sizeof(m_messageID) - 1);
					m_messageID[sizeof(m_messageID) - 1] = 0;
					break;
				case hSubject:
					strncpy(m_subj, ::TrimWhitespaceMT((char*)l822s.m_Token), sizeof(m_subj) - 1);
					m_subj[sizeof(m_subj) - 1] = 0;
					break;
				case hWho:
					strncpy(m_who, GetRealName((char*)l822s.m_Token), sizeof(m_who) - 1);
					m_who[sizeof(m_who) - 1] = 0;
					break;
				case hDispNotifyTo:
					m_isDispNotifyTo = TRUE;
					break;
				case hContentBase:
					strncpy(m_contentBase, ::TrimWhitespaceMT((char*)l822s.m_Token), sizeof(m_contentBase) - 1);
					m_contentBase[sizeof(m_contentBase) - 1] = 0;
					break;
				case hContentID:
					strncpy(m_contentID, ::TrimWhitespaceMT((char*)l822s.m_Token), sizeof(m_contentID) - 1);
					m_contentID[sizeof(m_contentID) - 1] = 0;
					break;
				case hContentLocation:
					strncpy(m_contentLocation, ::TrimWhitespaceMT((char*)l822s.m_Token), sizeof(m_contentLocation) - 1);
					m_contentLocation[sizeof(m_contentLocation) - 1] = 0;
					break;
				}
				break;

			//********************************
			case ExpectEnco:
				switch (tokenType)
				{
				case LinearWhite:
					break;	// Ignore
				default:
					strncpy(m_contentEnco, ::TrimWhitespaceMT((char*)l822s.m_Token),
						sizeof(m_contentEnco) - 1);
					m_contentEnco[sizeof(m_contentEnco) - 1] = 0;
					// If we know the encoding, get rid of it
					// that is, if it's safe to do so.
					if (g_bAttachmentDecodeEnabled && FindMIMEDecoder(m_contentEnco, FALSE))
						 m_pMbxFile->Seek(lLastNewline);
					EXPECT_TEXT;
					break;
				}
				break;

			//********************************
			case ExpectVersion:
				switch (tokenType)
				{
				case LinearWhite:
					break;	// Ignore
				default:
					m_isMIME = CompareRStringI(IDS_MIME_VERSION, (char*)l822s.m_Token, 0) == 0;
					EXPECT_TEXT;
					break;
				}
				break;
		
			//********************************
			case ExpectType:
				if (tokenType == LinearWhite)
					break;	// skip this
				if (tokenType == Atom || tokenType == QText)
				{
					strncpy(m_contentType, ::TrimWhitespaceMT((char*)l822s.m_Token),
						sizeof(m_contentType) - 1);
					m_contentType[sizeof(m_contentType) - 1] = 0;
					
					m_TLMime->ChangeInfo(NULL, m_contentType, NULL);

					m_state = ExpectSlash;
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
							
			//********************************
			case ExpectSlash:
				if (tokenType == LinearWhite)
					break;	// Ignore
				if (tokenType == Special && l822s.m_Token[0] == '/')
				{
					m_state = ExpectSubType;
					break;
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
						
			//********************************
			case ExpectSubType:
				if (tokenType == LinearWhite)
					break;	// Skip this
				if (tokenType == Atom || tokenType == QText)
				{
					strncpy(m_contentSubType, ::TrimWhitespaceMT((char*)l822s.m_Token),
						sizeof(m_contentSubType) - 1);
					m_contentSubType[sizeof(m_contentSubType) - 1] = 0;

					m_TLMime->ChangeInfo(NULL,NULL,m_contentSubType);

					m_state = ExpectSem;
					if (!CompareRStringI(IDS_MIME_PARTIAL, m_contentSubType))
						m_isPartial = TRUE;
					else
						m_isPartial = FALSE;
					if (!CompareRStringI(IDS_MIME_RICHTEXT, m_contentSubType) ||
						!CompareRStringI(IDS_MIME_ENRICHED, m_contentSubType) )
							m_hasRich = TRUE;  //m_hasRich = g_AnyRich = TRUE;
					if (!CompareRStringI(IDS_MIME_HTML, m_contentSubType) )
						m_isMHTML = TRUE;  //m_isMHTML = g_AnyHTML = TRUE;


					m_pMbxFile->Tell(&lastParam);


					ASSERT(lastParam >= 0);
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
					
			//********************************
			case ExpectSem:
				if (tokenType == LinearWhite)
					break;
				if (tokenType == Special && l822s.m_Token[0] == ';')
				{
					m_state = ExpectAttribute;
					break;
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
						
			//********************************
			case ExpectAttribute:
				if (tokenType == LinearWhite)
					break;	// Skip this
				if (tokenType == Atom)
				{
					strncpy(m_attributeName, ::TrimWhitespaceMT((char*)l822s.m_Token),
						sizeof(m_attributeName) - 1);
					m_attributeName[sizeof(m_attributeName) - 1] = 0;
					m_state = ExpectEqual;
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
		
			//********************************
			case ExpectEqual:
				if (tokenType == LinearWhite)
					break;
				if (tokenType == Special && l822s.m_Token[0] == '=')
				{
					m_state = ExpectValue;
					break;
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
						
			//********************************
			case ExpectValue:
				if (tokenType == LinearWhite)
					break;	// skip this
				if (tokenType == Atom || tokenType == QText)
				{
					if (!AddAttribute((char *)l822s.m_Token))
					{
						tokenType = ErrorToken;
						goto out;
					}
					m_state = ExpectSem;

					m_pMbxFile->Tell(&lastParam);
					ASSERT(lastParam >= 0);
				}
				else
				{
					m_grokked = FALSE;
					EXPECT_TEXT;
				}
				break;
			}
		}
	}
	
out:
	// did we finish cleanly?
	if (m_state != ExpectHeaderName && m_state != ExpectSem && m_state != ExpectText)
		m_grokked = FALSE;
	
	// where we ended up
	m_pMbxFile->Tell(&m_lDiskEnd);
	ASSERT(m_lDiskEnd >= 0);
	
	return (tokenType);
}

int ImapHeaderDesc::AddAttribute(const char* Value)
{

	AttributeElement* AE = new AttributeElement(m_attributeName, Value);
	if (!AE)
		return (FALSE);
 
	
	// This is a hack, we really should be parsing out the disposition into its own field
	if (!CompareRStringI(IDS_MIME_HEADER_CONTENT_DISP, m_attributeName))
		m_TLMime->ChangeInfo(NULL, NULL, NULL, Value);

	// This is a hack, we really should be parsing out the disposition into its own field
	if (!CompareRStringI(IDS_MIME_CONTENT_DISP_FILENAME, m_attributeName))
		m_TLMime->AddContentParam(m_attributeName, Value);
	else
		m_TLMime->AddParam(m_attributeName, Value);


	if (Attributes.AddTail(AE))
		return (TRUE);
	else
		return (FALSE);
}

AttributeElement* ImapHeaderDesc::GetAttribute(const char* Name)
{
	if (!Name || !*Name)
	{
		ASSERT(FALSE);
		return (NULL);
	}

	POSITION pos = Attributes.GetHeadPosition();
	while (pos)
	{
		AttributeElement* AE = (AttributeElement*)Attributes.GetNext(pos);
		if (AE->m_Name.CompareNoCase(Name) == 0)
			return (AE);
	}

	return (NULL);
}


#endif // IMAP4
