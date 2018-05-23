// MIME.CPP
//
// Routines for MIME
//
// Copyright (c) 1993-2000 by QUALCOMM, Incorporated
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

#include "QCUtils.h"

#include "rs.h"
#include "lex822.h"
#include "Base64.h"
#include "QP.h"
#include "pop.h"
#include "hexbin.h"
#include "uudecode.h"
#include "guiutils.h"
#include "progress.h"
#include "eudora.h"
#include "reltoabs.h"
#include "trnslate.h"
#include "msgutils.h"
#include "fileutil.h"
#include "header.h"
#include "GenericReader.h"
#include "MultiReader.h"
#include "SingleReader.h"
#include "TextReader.h"
#include "RegInfoReader.h"
#include "TransReader.h"
#include "LineReader.h"
#include "MIMEmap.h"
#include "mime.h"


#include "DebugNewHelpers.h"

// Used to be in os.h, moved here because this is the only place it is used
#define EOL_LEN 	2

// Yes, I dislike statics too.  But the various Decode() functions write
// Attachment converted line by calling WriteAttachNote() from sundry locations.
// And they have no idea of the mime parsing context.
// To support MHTML we need to postpone WriteAttachNote()s processing until the
// end of the multipart/related's processing.  So what's a man to do?  There's no
// clean linkage to do this.  Plus I really don't want Decode() to care about what state
// mime.cpp is in.  (we could wrap these statics in a class...)
// mimeInit() should be called before processing a message to ensure these are initialized.

static BOOL s_bMPRelated = FALSE;			// inside a multipart/related
static INT s_nMPRelatedID = 0;				// unique id for each m/r within a message
static HeaderDesc* s_hdEmbeddedPart = NULL;	// the current part's headers
static CObList s_EmbeddedElements;			// all the non-text parts that have been cached to disk

//  CTLAddress *g_tempaddresses;

void mimeInit()
{
	s_bMPRelated = FALSE;			// inside a multipart/related
	s_nMPRelatedID = 0;				// unique id for each m/r within a message
	s_hdEmbeddedPart = NULL;		// the current part's headers

	// Initialize the attachment count
	g_nAttachmentCount = 0;

	FlushEmbeddedElements();
}


/************************************************************************
 * Other MIME stuff
 ************************************************************************/

MIMEState::MIMEState(LineReader* lr, int depth, HeaderDesc* hd/*=NULL*/)
{
	m_hd = NULL;
	m_Coder = NULL;
	m_Reader = NULL;
	m_Translator = NULL;
	m_LineReader = NULL;
	
	if (lr)
		SetLineReader(lr);

	if (hd)
		Init(hd,depth);
}

int MIMEState::Init(HeaderDesc* hd, int depth)
{
	AttributeElement* AE;
	m_hd = hd;
	CTranslator *trans = NULL;
	long err = EMSR_CANT_TRANS;

	if (!m_hd->isMIME || depth>=MAX_MIME_DEPTH)
	{
		m_Coder = NULL;

		// Let the plug-in deal with these at text/plain	
		CTLAddress *addresses = GetEMSHeaders(m_hd->lDiskStart);

//		if (!g_tempaddresses)
//		{
//			g_tempaddresses = GetEMSHeaders(m_hd->lDiskStart);
//		}

		if (g_bAttachmentDecodeEnabled)
		{
			CTLMime tlMime;
			tlMime.CreateMimeType();
			err = GetTransMan()->CanXLateMessage(&tlMime, EMSF_ON_ARRIVAL, &trans, addresses);
		}
		if (addresses) delete addresses;
		
		if(err == EMSR_NOW)
		{
			m_Reader = DEBUG_NEW NowTransReader;
			m_Translator = trans;
		}
		else if (err == EMSR_NOT_NOW)
		{
			m_Reader = DEBUG_NEW NotNowTransReader;
			m_Translator = trans;
		}		
		else
		{
			m_Coder = NULL;
			m_Reader = DEBUG_NEW TextReader;
		}

	}
	else
	{	
		const char* contentType = m_hd->contentType;
		const char* contentSubType = m_hd->contentSubType;
		
		// Is it a multipart message?  Worry about the boundary
		if (!CompareRStringI(IDS_MIME_MULTIPART, contentType))
		{
			AE = m_hd->GetAttribute(CRString(IDS_MIME_BOUNDARY));
			if (AE)
				m_boundary = "--" + AE->m_Value;
		}
		// Find the reader for this part
		m_Reader = NULL;
	
		if ( !CompareRStringI(IDS_MIME_EUDORA_DATA, contentSubType) &&
			 !CompareRStringI(IDS_MIME_APPLICATION, contentType) )
		{
			m_Reader = DEBUG_NEW RegInfoReader;
			err = EMSR_NOT_NOW;
		}
		else
		{
			// Here's where we can see if translators should do something ...
			CTranslator *trans = NULL;

			// Get all the addresses
			CTLAddress *addresses = GetEMSHeaders(m_hd->lDiskStart);
	//		if (!g_tempaddresses)
	//		{
	//			g_tempaddresses = GetEMSHeaders(m_hd->lDiskStart);
	//		}

			if (g_bAttachmentDecodeEnabled)
				err = GetTransMan()->CanXLateMessage(m_hd->m_TLMime, EMSF_ON_ARRIVAL, &trans, addresses);
			
			if (addresses) delete addresses;

			if (err == EMSR_NOW)
			{
				m_Reader = DEBUG_NEW NowTransReader;
				m_Translator = trans;
			}
			else if (err == EMSR_NOT_NOW)
			{
				m_Reader = DEBUG_NEW NotNowTransReader;
				m_Translator = trans;
			}
		}

		if (m_Reader == NULL)
		{
			// Not registration information and not recognized by any plugin. Check
			// the other possibilities.
			CRString CTEuuencode(IDS_MIME_UUENCODE);
			CRString CTEuue(IDS_MIME_UUE);

			// Content-Transfer-Encoding of x-uuencode, x-uue, uuencode, or uue goes
			// to the TextReader, which knows how to deal with uuencoded data
			if (!stricmp(CTEuuencode, m_hd->contentEnco) ||
					 !stricmp(CTEuue, m_hd->contentEnco) ||
					 !stricmp(((LPCTSTR)CTEuuencode) + 2, m_hd->contentEnco) ||
					 !stricmp(((LPCTSTR)CTEuue) + 2, m_hd->contentEnco))
			{
				m_Reader = DEBUG_NEW TextReader;
			}
			else if (!CompareRStringI(IDS_MIME_MESSAGE, contentType))
			{
				if (g_bAttachmentDecodeEnabled)
				{
					if (!CompareRStringI(IDS_MIME_RFC822, contentSubType) ||
						!CompareRStringI(IDS_MIME_NEWS, contentSubType))
					{
						// just skip by message/[rfc822|news] part and treat contents of it
						// as if it were a normal part of the multipart
						char		szLogBuf[256];
						sprintf(szLogBuf, "LOGNULL MIMEState::Init() JJFileMT::Truncate(%d)", m_hd->lDiskStart);
						PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

						if (m_hd->grokked && FAILED(g_pPOPInboxFile->Truncate(m_hd->lDiskStart)))
							return (FALSE);

						m_hd->Init();
						m_hd->isMIME = TRUE;

						// We expect to see EndOfHeader, but EndOfMessage is good enough that
						// we shouldn't give up parsing the message because of it.
						int		nHeaderReadResult = m_hd->Read( GetLineReader() );

						if ( (nHeaderReadResult != EndOfHeader) && (nHeaderReadResult != EndOfMessage) )
							return (FALSE);

						return (Init(m_hd,depth));
					}
				}
				m_Reader = DEBUG_NEW TextReader;
			}
			else if (!CompareRStringI(IDS_MIME_TEXT, contentType))
			{
				AE = m_hd->GetAttribute(CRString(IDS_MIME_HEADER_CONTENT_DISP));
				if ((AE && !CompareRStringI(IDS_MIME_CONTENT_DISP_ATTACH, AE->m_Value)) || (contentSubType && !CompareRStringI(IDS_MIME_CONTENT_CALENDAR, contentSubType)))
					m_Reader = DEBUG_NEW GenericReader;
				else
					m_Reader = DEBUG_NEW TextReader;
			}
			else if (!CompareRStringI(IDS_MIME_MULTIPART, contentType))
				m_Reader = DEBUG_NEW MultiReader;
			else if (!CompareRStringI(IDS_MIME_APPLICATION, contentType))
			{
				if (!CompareRStringI(IDS_MIME_BINHEX, contentSubType))
					m_Reader = DEBUG_NEW TextReader;
				else if (!CompareRStringI(IDS_MIME_APPLEFILE, contentSubType))
					m_Reader = DEBUG_NEW SingleReader;
			}
			if (!m_Reader)
				m_Reader = DEBUG_NEW GenericReader;
		}
		
		// Is it encoded in a way that we can decode?
		m_Coder = FindMIMEDecoder(m_hd->contentEnco, TRUE);
	}
	
	if (!m_Reader)
	{
		// Uh oh...
		ASSERT(FALSE);
		return (FALSE);
	}
		
	return (TRUE);
}

MIMEState::~MIMEState()
{
	delete m_Coder;
	delete m_Reader;
//	if (g_tempaddresses) 
//	{
//		delete g_tempaddresses;
//		g_tempaddresses = NULL;
//	}


}

/************************************************************************
 * IsBoundaryLine: is a given line a MIME boundary?
 ************************************************************************/
BoundaryType MIMEState::IsBoundaryLine(const char* buf) const
{
	if (!strncmp(m_boundary, buf, m_boundary.GetLength()))
	{
		const char* bEnd = buf + m_boundary.GetLength();
		if (bEnd[0] == '\r')
			return (btInnerBoundary);
		else if (bEnd[0] == '-' && bEnd[1] == '-' && bEnd[2] == '\r')
			return (btOuterBoundary);
	}

	return (btNotBoundary);
}

/************************************************************************
 * HuntBoundary - hunt for a given boundary
 ************************************************************************/
BoundaryType MIMEState::HuntBoundary(char* buf, LONG bSize)
{
	BoundaryType boundaryType;
	int size = -1;

	while ((size = m_LineReader->ReadLine(buf, bSize)) > 0)
	{
		// write the line
		if (FAILED(PutWithEscapedFileMarkers(buf, size)))
			return (btError);
		
		// is it the first boundary?
		if (boundaryType = IsBoundaryLine(buf))
			return (boundaryType);
	}
	
	return (size? btError : btEndOfMessage);
}

/************************************************************************
 * WriteBoundary - write out a boundary
 ************************************************************************/
int MIMEState::WriteBoundary(BoundaryType boundaryType) const
{
	if (FAILED(g_pPOPInboxFile->EnsureNewline()))
		return (-1);
	if (FAILED(g_pPOPInboxFile->Put(m_boundary)))
		return (-1);
	if (boundaryType == btOuterBoundary && FAILED(g_pPOPInboxFile->Put("--")))
		return (-1);
	if (FAILED(g_pPOPInboxFile->PutLine()))
		return (-1);
		
	return (1);
}

/************************************************************************
 * IsGrokked - Do we understand the header we just wrote?
 ************************************************************************/
BOOL MIMEState::IsGrokked()
{
	if (m_hd->grokked == FALSE)
		return FALSE;
	else
		return TRUE;
}

/************************************************************************
 * FindMIMEDecoder - find the decoder for a given content-transfer-encoding
 ************************************************************************/
Decoder* FindMIMEDecoder(const char* encoding, int AllocateCoder)
{
	if (!CompareRStringI(IDS_MIME_BASE64, encoding))
		return (AllocateCoder? DEBUG_NEW_NOTHROW Base64Decoder : (Decoder*)TRUE);
	else if (!CompareRStringI(IDS_MIME_QP, encoding))
		return (AllocateCoder? DEBUG_NEW_NOTHROW QPDecoder : (Decoder*)TRUE);

	return (NULL);
}


/************************************************************************
 * FindMIMECharset - find the right xlate table for a particular MIME char set
 ************************************************************************/
int FindMIMECharset(const char* charSet)
{
	int		iCharSet = -1;

	if (!strnicmp("windows-", charSet, 8))
	{
		// Windows character sets are assigned an index of 0.
		iCharSet = 0;
	}
	else
	{
		iCharSet = FindRStringIndexI(IDS_MIME_US_ASCII, IDS_MIME_UTF_8,	charSet, -1);
		if (iCharSet >= 0)
		{
			// Bump index to avoid conflict with windows character set index.
			++iCharSet;
		}
	}

	return iCharSet;
}

// Params will be put on the attachment line in <...> ... this should not be displayed
int WriteAttachNote(JJFile* AttachFile, UINT labelStrID, LPCTSTR params /*= NULL*/)
{
	if (!AttachFile || !g_pPOPInboxFile)
		return (0);

	if ( s_bMPRelated )
	{
		CString strFilename("???");
		{
			BSTR bstrFilename = NULL;
			if (SUCCEEDED(AttachFile->GetFName(&bstrFilename)))
			{
				strFilename = bstrFilename;
				::SysFreeString(bstrFilename);
			}
		}
		
		return SaveEmbeddedElement(strFilename);
	}

	long lOffset = 0;
	if (FAILED(g_pPOPInboxFile->EnsureNewline())) 
		goto fail;

	g_pPOPInboxFile->Tell(&lOffset);
	if ((g_bHasAttachment = lOffset) < 0L) 
		goto fail;

	if (FAILED(g_pPOPInboxFile->Put(CRString(labelStrID)))) goto fail;
	
	if (FAILED(g_pPOPInboxFile->Put("\""))) goto fail;

	{
		CString strFilename("???");
		BSTR bstrFilename = NULL;
		if (SUCCEEDED(AttachFile->GetFName(&bstrFilename)))
		{
			strFilename = bstrFilename;
			::SysFreeString(bstrFilename);
		}
		if (FAILED(g_pPOPInboxFile->Put(strFilename))) goto fail;
	}
		
	if (params)
	{
		if (FAILED(g_pPOPInboxFile->Put(" "))) goto fail;
		if (FAILED(g_pPOPInboxFile->Put(params))) goto fail;
	}
	if (FAILED(g_pPOPInboxFile->Put("\""))) goto fail;

	if (FAILED(g_pPOPInboxFile->PutLine())) goto fail;

	// If we come here, then lets increment the attachment count
	g_nAttachmentCount++;
	return (1);

fail:
	return (-1);

}

/************************************************************************
 * The readers
 ************************************************************************/
/************************************************************************
 * MultiReader - read a multipart message body
 ************************************************************************/
MultiReader::MultiReader()
{
	m_bExplodeDigest = false;

}
BoundaryType MultiReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	BoundaryType boundaryType;
	MIMEState* ms;
	HeaderDesc* hd;
	HeaderDesc* innerHD = NULL;
	MIMEState* innerMS = NULL;
	BOOL wasApplefile = FALSE;
	char ApplefileName[128];
	long lOffset;
	int Result = ErrorToken;
	BOOL alternative = FALSE;
	long altOffset = 0;   
	long eatOffset = 0;
	CString altSubtype;altSubtype.Empty();
	bool bInDigest = false;
	JJFile *pTmp = NULL;
	JJFile * pFile = NULL;
	
	ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];
	hd = ms->m_hd;
	

	alternative = !CompareRStringI(IDS_MIME_ALTERNATIVE, hd->contentSubType);

	// this stuff is called recursively.  The ugly static should indicate if we are
	// processing a multipart/related, even if it contains a multipart/alternative
	// so save the state on the stack and only assign s_bMPRelated to TRUE.
	// setting back to false will occur when the stack unwinds...
	BOOL bOldMPRelated = s_bMPRelated;
	if ( !CompareRStringI(IDS_MIME_RELATED, hd->contentSubType) )
	{
		s_bMPRelated = TRUE;
		s_nMPRelatedID++;
	}
	
	// Skip introductory text
	boundaryType = ms->HuntBoundary(buf, bSize);
	if (boundaryType == btInnerBoundary)
	{
		// we have found the first boundary

		// remove text between header and multipart intro
		// or, if we understood the whole header, remove it
		char		szLogBuf[256];
		sprintf(szLogBuf, "LOGNULL MultiReader::ReadIt() JJFileMT::Truncate(%d)", ((MimeStates.GetSize() > 1 && hd->grokked)?
			hd->lDiskStart : hd->lDiskEnd));
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		if (FAILED(g_pPOPInboxFile->Truncate(((MimeStates.GetSize() > 1 && hd->grokked)?
			hd->lDiskStart : hd->lDiskEnd))))
		{
			goto error;
		}

		// we have read a boundary.  What follows will be a message header
		// (probably; else we have to infer one :-().  Read the header, and
		// [re]curse.  We continue with this until we find the end of the
		// message or a terminal boundary
		while (boundaryType == btInnerBoundary)
		{
			// Save for remaining bytes calculation, and applefile
			g_pPOPInboxFile->Tell(&lOffset);
			ASSERT(lOffset >= 0);
				
			// write out the boundary
//			if (ms->WriteBoundary(boundaryType) < 0) goto error;

			// prepare a new header desc
			if (!(innerHD = DEBUG_NEW_NOTHROW HeaderDesc(hd->RemainingBytes))) goto error;

			// Content-Base must inherit
			strcpy( innerHD->contentBase, hd->contentBase );

			// If this is multipart/digest, then the default for all subparts
			// is message/rfc822
			if (!CompareRStringI(IDS_MIME_DIGEST, hd->contentSubType))
			{
				GetIniString(IDS_MIME_MESSAGE, innerHD->contentType, sizeof(innerHD->contentType));
				GetIniString(IDS_MIME_RFC822, innerHD->contentSubType, sizeof(innerHD->contentSubType));
				
				if(m_bExplodeDigest)
				{
					char *tmp = GetMessageSeparatorLine();

					if (FAILED(g_pPOPInboxFile->PutLine(tmp)))
					{
						delete [] tmp;
						break;
					}
					delete [] tmp;

				}
				else
				// Process MIME digest as attachment to the message in mbx format
				if(GetIniShort(IDS_INI_EXPLODE_DIGEST))
				{
					char fName[MAX_PATH];

					if(!bInDigest)
					{
						AttributeElement *AE = hd->GetAttribute(CRString(IDS_MIME_CONTENT_DISP_FILENAME));
						if (!AE)
							AE = hd->GetAttribute(CRString(IDS_MIME_NAME));
						if (AE)
							strcpy(fName, AE->m_Value);
						else
						{
							//Bad digest, no filename generic is "digest"
							CRString szDigest(IDS_MIME_DIGEST);
							strcpy(fName, szDigest);
						}
						char *ext = strrchr(fName, '.');
						if(ext)
						{
							if(stricmp(ext, ".mbx") != 0)
								strcat(fName, ".mbx");
						}
						else
						{
							strcat(fName, ".mbx");
						}

						pFile = OpenAttachFile(fName, TRUE);
						if (!pFile)
							break;

						pTmp = g_pPOPInboxFile;
						g_pPOPInboxFile = pFile;
					}

					bInDigest = true;
					char *tmp = GetMessageSeparatorLine();

					if (FAILED(g_pPOPInboxFile->PutLine(tmp)))
					{
						delete [] tmp;
						break;
					}
					delete [] tmp;

				}
			}

			// read the header
			if ((Result = innerHD->Read(ms->GetLineReader())) != EndOfHeader) goto error;

			// All sub parts are considered MIME, too
			innerHD->isMIME = TRUE;

			// extract MIME info
			if (!(innerMS = DEBUG_NEW_MFCOBJ_NOTHROW MIMEState(ms->GetLineReader(), MimeStates.GetSize(),innerHD)) || !innerMS->m_Reader)
				goto error;
			MimeStates.Add(innerMS);

			if (alternative)
			{ 
				CString newSubtype;

				// Stuff we recognize:
				//		text/plain
				//		text/enriched
				//		text/html
				//		mulitpart/related
				if ((!CompareRStringI(IDS_MIME_TEXT, innerHD->contentType) &&
						(!CompareRStringI(IDS_MIME_PLAIN, innerHD->contentSubType) ||
						!CompareRStringI(IDS_MIME_ENRICHED, innerHD->contentSubType) ||
						!CompareRStringI(IDS_MIME_HTML, innerHD->contentSubType))) ||
					(!CompareRStringI(IDS_MIME_MULTIPART, innerHD->contentType) && 
						!CompareRStringI(IDS_MIME_RELATED, innerHD->contentSubType)))
				{
					newSubtype = innerHD->contentSubType;
				}

				// The point to multipart/alternative is to present alternative views
				// of the same data.  If we have an acceptable part already, stick with
				// it and dump the others.  It might be a reasonable thing to put them
				// in attachments, but I'm not up to that this morning, so for now I'm
				// going to find a comfy spot on the floor and drop them there.
				//
				// At least, for text parts.  Things that will go as attachments anyway
				// we may as well leave alone.
				//
				// SD 12/21/99

				//NOT // This is an unknown alternative, so just show it!
				//NOT if (newSubtype.IsEmpty())
				//NOT	eatOffset = 0;
				
				// we recognize text/calendar attachements from Exchange
				// so we preserve this as an .ics attachment
				// If this is an unknown text/* part and we already have one we know
				// we want to drop it on the floor.  If it's non-text, we'll keep it,
				// god only knows what it might be, and it will just be an attachment anyway
				if ( newSubtype.IsEmpty() && ( CompareRStringI(IDS_MIME_TEXT, innerHD->contentType) || !CompareRStringI(IDS_MIME_CONTENT_CALENDAR, innerHD->contentSubType) ) )
					eatOffset = 0;

				// Is this something better (original empty, multipart/related, text/enriched, text/html)
				else if (altSubtype.IsEmpty() ||
						!CompareRStringI(IDS_MIME_RELATED, newSubtype) ||
						(CompareRStringI(IDS_MIME_RELATED, altSubtype) &&
							(!CompareRStringI(IDS_MIME_ENRICHED, newSubtype) ||
							!CompareRStringI(IDS_MIME_HTML, newSubtype)))						)
				{
					// do we have something better?
					if (altOffset)
					{
						char		szLogBuf[256];
						sprintf(szLogBuf, "LOGNULL MultiReader::ReadIt() JJFileMT::Truncate(%d) 2", altOffset);
						PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

						g_pPOPInboxFile->Truncate(altOffset);

						// We set lDiskEnd to the same value as lDiskStart, because
						// we just truncated the file. No good will come from
						// remembering an end offset that is now past EOF.
						// Later code could attempt to Truncate the file to lDiskEnd,
						// which would fail in turn causing MIME processing to fail.
						innerHD->lDiskEnd = altOffset;
						innerHD->lDiskStart = altOffset;
					}
					altOffset = lOffset;
					altSubtype = newSubtype;
				}
				else
				{
					// we have what we want, let's eat the next section
					eatOffset = lOffset;
				}
			}
			else
				altOffset = 0;

			if (wasApplefile)
			{
				// Check for slash and get just the final name portion
				char* szAppleFileNameNoSlash = strrchr(ApplefileName, SLASH);

				if (szAppleFileNameNoSlash)
				{
					// A slash in the name means that it was written out (SingleReader provides
					// the full path when it writes out).
					// The first part of apple double was not expected to be written out -
					// ASSERT to notify us that we may need to re-add a file remove call.
					ASSERT(0);
				}
				else
				{
					szAppleFileNameNoSlash = ApplefileName;
				}

				// If there isn't a value for the name, then use the name from the first part
            	CRString			Name(IDS_MIME_NAME);
				AttributeElement*	AE = innerHD->GetAttribute(Name);
				if (AE)
				{
					if ( AE->m_Value.IsEmpty() )
						AE->ChangeValue(szAppleFileNameNoSlash);
				}
				else
				{
					AE = DEBUG_NEW AttributeElement(Name, szAppleFileNameNoSlash); 
					innerHD->Attributes.AddTail(AE);
				}

				// Make sure extension doesn't get reinterpretted by clearing
                // out the MIME type and subtype
				innerHD->contentType[0] = innerHD->contentSubType[0] = 0;
			}

			// read the body of the message
			s_hdEmbeddedPart = innerHD;

			// Give the subpart some Content-* header values of the multipart if the subpart
			// doesn't have them.  This is done to handle cases like multipart/appledouble
			// where the whole multipart refers to one object (a file), but the Content-*
			// headers for the object are at the multipart level.
			if (!s_hdEmbeddedPart->contentID[0])
				strcpy(s_hdEmbeddedPart->contentID, hd->contentID);
			if (!s_hdEmbeddedPart->contentLocation[0])
				strcpy(s_hdEmbeddedPart->contentLocation, hd->contentLocation);
			if (!s_hdEmbeddedPart->contentBase[0])
				strcpy(s_hdEmbeddedPart->contentBase, hd->contentBase);

			char		szLogBuf[256];
			sprintf(szLogBuf, "LOGNULL MultiReader::ReadIt() calling ::ReadIt()");
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

			boundaryType = innerMS->m_Reader->ReadIt(MimeStates, buf, bSize);  
			s_hdEmbeddedPart = NULL;

			// if the inner part contains text/html then mark the multipart/related
			// as MHTML
			if ( s_bMPRelated && innerHD->isMHTML )
				hd->isMHTML = TRUE;

			{
				long lCurrent = 0;
				g_pPOPInboxFile->Tell(&lCurrent);
				ASSERT(lCurrent >= 0);
				hd->RemainingBytes -= lCurrent - lOffset;
			}

			// clean up
			wasApplefile =
				!CompareRStringI(IDS_MIME_APPLEFILE, innerHD->contentSubType) &&
				(!CompareRStringI(IDS_MIME_APPLEDOUBLE, hd->contentSubType) ||
				!CompareRStringI(IDS_MIME_HEADERSET, hd->contentSubType));
			if (wasApplefile)
			{
				strcpy(ApplefileName, innerHD->attributeName);

				char		szLogBuf[256];
				sprintf(szLogBuf, "LOGNULL MultiReader::ReadIt() JJFileMT::Truncate(%d) 3", lOffset);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

				if (FAILED(g_pPOPInboxFile->Truncate(lOffset)))
					goto error;
			}

			MimeStates.RemoveAt(MimeStates.GetUpperBound());
			delete innerHD; innerHD = NULL;
			delete innerMS; innerMS = NULL;
	
	        // We're really getting rid of this part
			if (eatOffset)
			{	
				char		szLogBuf[256];
				sprintf(szLogBuf, "LOGNULL MultiReader::ReadIt() JJFileMT::Truncate(%d) 4", eatOffset);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

				g_pPOPInboxFile->Truncate(eatOffset);  
				eatOffset = 0; 
			}
		}
	}

	// If we are in a digest we need to set the inbox back
	// And write the attach note for the .mbx we saved
	if(bInDigest)
	{
		g_pPOPInboxFile = pTmp;
		WriteAttachNote(pFile, IDS_ATTACH_CONVERTED);
		delete pFile;
		pFile = NULL;
		bInDigest = false;
	}

	if (boundaryType == btOuterBoundary)
	{
		g_pPOPInboxFile->EnsureNewline();
		
		// we have finished our multipart.
		// if we are part of a larger multipart, then we must now hunt for the
		// larger multipart's boundary.  If not, we hunt for the end of the
		// message.  In either case, HuntBoundary will do it for us.
		MIMEState *pMS = ms;
		if (MimeStates.GetSize() > 1)
			pMS = (MIMEState*)MimeStates[MimeStates.GetUpperBound() - 1];
		g_pPOPInboxFile->Tell(&lOffset);		// remember where we started tossing
		ASSERT(lOffset >= 0);
		do
		{
			boundaryType = pMS->HuntBoundary(buf, bSize);
		} while (pMS == ms && boundaryType < btEndOfMessage);
		// this condition is the way it is because, if we are the outermost
		// multipart, we need to keep reading until end of message
		
		char		szLogBuf[256];
		sprintf(szLogBuf, "LOGNULL MultiReader::ReadIt() JJFileMT::Truncate(%d) 5", lOffset);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		g_pPOPInboxFile->Truncate(lOffset);		// toss the excess bytes
	}

	if ( s_bMPRelated && ! bOldMPRelated )
	{
		// ending up with a multipart/related - save 'em parts
		WriteEmbeddedElements( hd->isMHTML );
		FlushEmbeddedElements();
	}
	s_bMPRelated = bOldMPRelated;

	delete innerHD;
	delete innerMS;

	return (boundaryType);

error:
	FlushEmbeddedElements();
	s_bMPRelated = bOldMPRelated;

	delete innerHD;
	delete innerMS;
	
	return (Result == EndOfMessage? btEndOfMessage : btError);
}

// GetMessageSeparatorLine
// 
// This line is the From ???@???etc... to separate messages in an mbx
// Please delete the returned buffer
char * GetMessageSeparatorLine()
{
	char *buf = DEBUG_NEW char [1024];

	*buf = 0;

	time_t currentTime;
	time(&currentTime);
	if (currentTime < 0)
		currentTime = 1;
	struct tm* pTime = localtime(&currentTime);

	const char *Weekdays = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat\0";
	const char *Months = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec\0";
	wsprintf(buf, CRString(IDS_FROM_FORMAT), 
						((const char *) Weekdays) + pTime->tm_wday * 4,
						((const char *) Months) + pTime->tm_mon * 4, 
						pTime->tm_mday, 
						pTime->tm_hour,
						pTime->tm_min, 
						pTime->tm_sec, 
						pTime->tm_year + 1900);
	return buf;
}
// DeMacifyName
//
// Take the name given to us and figure out how to map it
// to something more useful.
// Mac names can contain funny characters, spaces, be up to 32
// characters long etc, etc. We need 8 plus an extention.
//

void DeMacifyName(char* Name)
{
	static const char Illegal[] = " \t\r\n\".*+,/:;<=>?[\\]|";
	char pcname[15];
	int len = 8;
	char *s, *d, *ext;
	
	if (s = strpbrk(Name, "\r\n"))
		*s = 0;
	ext = strrchr(Name, '.');

	for (d = pcname, s = Name; *s && s != ext && len; s++)
	{
		if ((*s & 0x80) || (*s < 32) || strchr((LPCSTR)Illegal, *s))
			continue;
		*d++ = *s;
		len--;
	}
	*d = 0;

	if (ext)
	{
		len = 8;
		*d++ = *ext++;
		for (s = ext; *s && len; s++)
		{
			if ((*s & 0x80) || (*s < 32) || strchr((LPCSTR)Illegal, *s))
				continue;
			*d++ = *s;
			len--;
		}
		*d = 0;
	}

	strcpy(Name, pcname);
}

void DeMacifyName32(char* Name)
{
	static const char Illegal[] = "\t\r\n\"\\/:*?<>|";

	char pcname[255];
	int len = 255;
	char *s, *d;
	
	if (s = strpbrk(Name, "\r\n"))
		*s = 0;

	for (d = pcname, s = Name; *s  && len; s++)
	{
		if (/*(*s & 0x80) ||*/ ((unsigned char)*s < 32) || strchr((LPCSTR)Illegal, *s))
			continue;
		*d++ = *s;
		len--;
	}
	*d = 0;

	strcpy(Name, pcname);
}


// CheckName
//
// Checks to see if the file name already exists or will conflict with a DOS reserved
// file name. Searches for a unique file name by tacking on a number after the name, but
// before the extension(s). Returns whether or not a unique name was found. Last name that it
// checked (whether or not successful) is placed in FinalFname.
//
// Unique number is either placed before the first valid extension separating dot
// (e.g. SomeName.NotAnExtension.tar.gz - the second dot before tar), or before the
// first dot if the name before the first dot is DOS reserved
// (e.g. com1.NotAnExtension.tar.gz - the first dot).
//
// The path is truncated if the path is going to be too long, but the file extension is maintained.
// When the path is truncated the code does not check for DOS reserved names, because the
// name that would need to be checked against changes based upon how much truncation occurs
// (i.e. it would require code that reparses for the last back slash path separator after each
// truncation). Instead the name is immediately truncated to the DOS reserved portion of the name
// in the hopes that that will allow us to avoid DOS reserved names (i.e. com1.badname is truncated
// to com1).
//
BOOL CheckName(const char* Dir, const char* FName, char* FinalFname, BOOL bLongFileNameSupport)
{
	BOOL	bFoundUniqueName = FALSE;
	char	DirName[_MAX_PATH];
	
	//	By default we'll check the DOS reserved file name at least once
	bool	bCheckDOSReservedNameAtLeastOnce = true;

	//	Copy the directory name and add a trailing backslash if necessary
	strncpy(DirName, Dir, _MAX_PATH-2);
	DirName[_MAX_PATH-1] = '\0';
	int		nDirLength = strlen(DirName);
	if (DirName[nDirLength-1] != SLASH)
	{
		DirName[nDirLength++] = SLASH;
		DirName[nDirLength] = '\0';
	}
	
	//	Get the string of DOS reserved device names
	CString sReservedWords;
	char *pReserved = sReservedWords.GetBuffer(1024);
	GetIniString(IDS_INI_DOS_RESERVED, pReserved, 1024);
	sReservedWords.ReleaseBuffer();

	CString			sFileName = FName;

	//	Trim any leading or trailing spaces or periods to make sure that what
	//	we think the name is matches up with what Windows uses (leading spaces,
	//	trailing spaces, and trailing periods are ignored; leading periods
	//	seem to be illegal).
	sFileName.TrimLeft(" .");
	sFileName.TrimRight(" .");

	//	Now that we're done modifying sFileName grab the pointer to save time later
	const char *	szFileName = sFileName;

	//	We might need to truncate if we aren't sure that we'll have enough
	//	room for both the full path and a 4 digit unique number
	bool			bMightNeedToTruncate = ( (nDirLength + sFileName.GetLength() + 4) > (_MAX_PATH - 1) );

	//	pDOSReservedExtension and pExtension point to the extension(s) (including the dot)
	//	in szFileName, while sOtherName and sBaseName copy the portion of the name before the extension(s).
	//	sOtherName is intially used for the DOS reserved name and is reused in the truncation code.
	CString			sOtherName, sBaseName;

	//	Find the first dot for the DOS reserved base name (portion of name before any dots)
	const char *	pDOSReservedExtension = strchr(szFileName, '.');
	const char *	pExtension = NULL;

	//	Initialize the DOS reserved base name
	sOtherName = sFileName;
	if (pDOSReservedExtension)
	{
		//	Chop off the DOS reserved extension
		sOtherName.GetBufferSetLength(pDOSReservedExtension - szFileName);
		
		//	Look for the first dot that separates valid extensions
		//	(e.g. with SomeName.NotAnExtension.tar.gz it would be the second dot)
		pExtension = szFileName + sFileName.GetLength();
		const char *	pNameChar = pExtension - 1;

		//	With long file names the dot must not be more than 5 characters
		//	from the next dot or the end of the name.
		//	With short file names the dot must not be more than 4 characters
		//	from the end of the name.
		int				nMaxExtensionLimit = bLongFileNameSupport ? 6 : 5;

		//	Support only one extension (e.g. just the .zip in SomeName.tar.zip) if:
		//	* we are not supporting long file names (so we don't violate 8.3)
		//	* we might need to truncate
		bool			bOnlySupportOneExtension = !bLongFileNameSupport || bMightNeedToTruncate;

		//	There must be at least one character and the dot must be close enough
		while ( (pNameChar > szFileName) && ((pExtension - pNameChar) < nMaxExtensionLimit) )
		{
			//	Remember the location of any dots we find
			if (*pNameChar == '.')
			{
				pExtension = pNameChar;

				//	If we only support one extension stop looking
				if (bOnlySupportOneExtension)
					break;
			}

			--pNameChar;
		}

		//	Did we ever found a valid extension dot?
		if ( pExtension == (szFileName + sFileName.GetLength()) )
			pExtension = NULL;
	}

	if (bMightNeedToTruncate)
	{
		//	The file name length truncation code doesn't support checking for
		//	DOS reserved names (neither did the previous version of this routine).
		bCheckDOSReservedNameAtLeastOnce = false;
		
		//	We might need to truncate so use the portion of the name before the first dot.
		//	This accomplishes two things:
		//	* It sometimes truncates the name a little bit immediately (com1.badname.txt would
		//	  normally result in an sBaseName of com1.badname, but instead becomes com1).
		//
		//	* It makes sure that we're adding the number to the portion of the name that could
		//	  be DOS reserved. Even though we don't support checking for DOS reserved names,
		//	  in testing it appears that a DOS reserved name will fail the !::FileExistsMT
		//	  check. By adding the number to the DOS reserved portion of the name, we'll likely
		//	  still be able to avoid DOS reserved names.
		sBaseName = sOtherName;
	}
	else
	{
		//	Stash the normal base name (possibly the same as the DOS reserved base name)
		sBaseName = sFileName;
		if (pExtension)
		{
			//	Chop off the normal extension
			sBaseName.GetBufferSetLength(pExtension - szFileName);
		}
	}

	//	Check to see if we'll be using the DOS reserved base name the whole time.
	//	We'll use the DOS reservered base name the whole time if the dots are at
	//	the same location. This is because while we are attempting to find a unique name
	//	before the first dot, we may accidently stumble across a DOS reserved name
	//	even if the initial name is not a DOS reserved name.
	bool			bUseDOSReservedBaseName = bCheckDOSReservedNameAtLeastOnce &&
											  (pDOSReservedExtension == pExtension);
	

	//	Determine the base file name length we support
	int				nBaseLen = bLongFileNameSupport ? 255 : 8;

	//	If the file name (by itself without any directories) is too long, then truncate it
	if ( nBaseLen < sBaseName.GetLength() )
		sBaseName.GetBufferSetLength(nBaseLen);

	//	nFullPathLengthWithoutExtension is the length of the full path
	//	except for the length of the file extension
	int				nFullPathLengthWithoutExtension = nDirLength + sBaseName.GetLength();
	int				nExtensionLength = pExtension ? strlen(pExtension) : 0;
	int				nMaxLengthAllowed = _MAX_PATH - 1 - nExtensionLength;
	if (nFullPathLengthWithoutExtension > nMaxLengthAllowed)
	{
		//	The full path is too long
		//	Let's just go the easy route and truncate while maintaining the correct extension
		CString		sFullBlown;
		sFullBlown.Format("%s%s", DirName, sBaseName);
		strncpy(FinalFname, sFullBlown, nMaxLengthAllowed);
		FinalFname[nMaxLengthAllowed] = '\0';

		//	We left space for the extension - cat it on now
		if (pExtension)
			strcat(FinalFname, pExtension);
	}
	else
	{
		//	Not too long, just format the full name for the first unique check
		wsprintf( FinalFname, "%s%s%s",
				  DirName, sBaseName, (pExtension ? pExtension : "") );
	}

	CString			sResWord;
	bool			bIsReserved = false;
	int				nTryNum = 1;
	int				nNumberLength;

	//	Look for a unique name up to 9999 times
	while (nTryNum < 10000)
	{
		if (bCheckDOSReservedNameAtLeastOnce || bUseDOSReservedBaseName)
		{
			//	Loop to see if sOtherName is one of the DOS reserved device names
			char *		pCommaDelimeter;
			pReserved = (char *) (LPCTSTR) sReservedWords;

			while (*pReserved != NULL)
			{
				if ( (pCommaDelimeter = strchr(pReserved, ',')) != NULL )
				{
					//	Store in sResWord one DOS device name at a time so it can be compared
					int		nResWordLen = pCommaDelimeter - pReserved;
					ASSERT(nResWordLen < 9);
					sResWord = pReserved;
					sResWord.GetBufferSetLength(nResWordLen);
				}
				else
				{
					//	Last device name so copy it
					sResWord = pReserved;
				}
			
				bIsReserved = (sOtherName.CompareNoCase(sResWord) == 0);
				if (bIsReserved)
				{
					if (bCheckDOSReservedNameAtLeastOnce && !bUseDOSReservedBaseName)
					{
						//	It's our first time through the loop and the names weren't
						//	previously the same. Make sure that we use the DOS reserved
						//	base name and extension.
						sBaseName = sOtherName;
						pExtension = pDOSReservedExtension;

						//	We must use the DOS reserved base name from here on out so that we can
						//	continue to check any unique number names against our list of reserved names
						bUseDOSReservedBaseName = true;
					}

					//	We found a match - stop looking
					break;
				}

				//	If there are still device names to compare
				if (pCommaDelimeter)
					pReserved = pCommaDelimeter + 1;
				else	//	Stop looking
					break;
			}

			bCheckDOSReservedNameAtLeastOnce = false;
		}

		if ( !bIsReserved && !::FileExistsMT(FinalFname) )
		{
			bFoundUniqueName = TRUE;
        	break;
		}

		//	Figure out how many digits the number will take up
		if (nTryNum < 10)
			nNumberLength = 1;
		else if (nTryNum < 100)
			nNumberLength = 2;
		else if (nTryNum < 1000)
			nNumberLength = 3;
		else
			nNumberLength = 4;

		//	If the file name (by itself without any directories) is too long, then truncate it
		if ( (nBaseLen-nNumberLength) < sBaseName.GetLength() )
			sBaseName.GetBufferSetLength(nBaseLen-nNumberLength);

		nFullPathLengthWithoutExtension = nDirLength + sBaseName.GetLength();

		//	Is the file name is too long?
		if ( nFullPathLengthWithoutExtension > (nMaxLengthAllowed-nNumberLength) )
		{
			//	Let's just go the easy route and truncate
			sOtherName.Format("%s%s", DirName, sBaseName);
			strncpy(FinalFname, sOtherName, nMaxLengthAllowed-nNumberLength);
			FinalFname[nMaxLengthAllowed-nNumberLength] = '\0';

			//	Maintain the unique number and the correct extension.
			//	We left space for them - cat them on now.
			sOtherName.Format( "%d%s", nTryNum, (pExtension ? pExtension : "") );
			strcat(FinalFname, sOtherName);

			//	The file name length truncation code doesn't support checking for
			//	DOS reserved names (neither did the previous version of this routine).
			bUseDOSReservedBaseName = false;
		}
		else
		{
			//	Format our next attempt at a valid name
			wsprintf( FinalFname, "%s%s%d%s",
					  DirName, sBaseName, nTryNum, (pExtension ? pExtension : "") );

			//	If we're still checking for DOS reserved base names format it too
			if (bUseDOSReservedBaseName)
				sOtherName.Format("%s%d", sBaseName, nTryNum);
		}

		++nTryNum;
	}

	return bFoundUniqueName;
}

// OpenAttachFile
//
// Legalizes the file name and opens the attachment file
//
JJFile* OpenAttachFile(LPTSTR Name, BOOL bAllowRead /*= FALSE*/)
{
	// this is part of the uglyness.  OpenAttachFile and WriteAttachNote are
	// called from lots o' places in the code that know not the context in which
	// they work.  With MHTML, sometimes we want to save stuff to the Embedded directory
	// and write "Embedded content:" lines verses saving stuff to the Attach dir and
	// writing "Attachment converted:" lines.  Rather than changing the world I use a
	// static to determine if we are working with MHTML or plain ol' attachments...
	if ( s_bMPRelated )
		return OpenEmbeddedFile( Name );

    JJFile* ATFile = NULL;
	char buf[_MAX_PATH+1];
    char FinalFname[_MAX_PATH+1];
    char DirName[_MAX_PATH+1];
	CString attachDir;
	char* FileName = NULL;
	
    *buf = 0;
	GetIniString(IDS_INI_AUTO_RECEIVE_DIR, DirName, sizeof(DirName));
	if ( (DirName[0] == 0) || !::FileExistsMT(DirName, TRUE))
	{ 
		WIN32_FIND_DATA wfd;
		attachDir = EudoraDir + CRString(IDS_ATTACH_FOLDER); 
		attachDir.MakeLower();
		strcpy(DirName, attachDir);

		// see if we need to create "attach" directory
		HANDLE findHandle;
		findHandle = FindFirstFile((const char *)attachDir,&wfd);
		if ( findHandle == INVALID_HANDLE_VALUE)
		{
			// if we can't make the directory, reset it to to old way
			if (mkdir(attachDir) < 0)
				if (errno != EACCES)
					strcpy(DirName, EudoraDir);
		}
		FindClose(findHandle);
	}


	// Turn the macintosh name into something more useful
	BOOL bLongFileNameSupport = ::LongFileSupportMT(DirName);
	if (bLongFileNameSupport)
		DeMacifyName32(Name);
	else
		DeMacifyName(Name);

	if (CheckName(DirName, Name, FinalFname, bLongFileNameSupport))
	{
		FileName = strrchr(FinalFname, SLASH) + 1;
		wsprintf(buf, "%s%s%s", DirName,
		((DirName[strlen(DirName) - 1] == SLASH)? "" : SLASHSTR), FileName);

		// Let's get the file
		ATFile = DEBUG_NEW_NOTHROW JJFile;
		if (ATFile)
		{
			int mode = bAllowRead? _O_RDWR : O_WRONLY;
			if (FAILED(ATFile->Open(buf, O_CREAT | mode)))
			{
				delete ATFile;
				ATFile = NULL;
			}
		}
	}

	return (ATFile);
}

// OpenEmbeddedFile
//
// Legalizes the file name and opens the file that holds embedded (mhtml m/r)
// parts.  (They're opened in <eudoradir>\Embedded\)
//
JJFile* OpenEmbeddedFile(char* Name)
{
    JJFile* EBFile = NULL;
	char buf[_MAX_PATH+1];
    char FinalFname[_MAX_PATH+1];
    char DirName[_MAX_PATH+1];
	CString embeddedDir;
	char* FileName = NULL;
	
    *buf = 0;

	WIN32_FIND_DATA wfd;
	embeddedDir = EudoraDir + CRString(IDS_EMBEDDED_FOLDER); 
	strcpy(DirName, embeddedDir);

	// see if we need to create "embedded" directory
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)embeddedDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
	{
		// if we can't make the directory, reset it to to old way
		if (mkdir(embeddedDir) < 0)
			if (errno != EACCES)
				strcpy(DirName, EudoraDir);
	}
	FindClose(findHandle);


	// Turn the macintosh name into something more useful
	BOOL bLongFileNameSupport = ::LongFileSupportMT(DirName);
	if (bLongFileNameSupport)
		DeMacifyName32(Name);
	else
		DeMacifyName(Name);

	if (CheckName(DirName, Name, FinalFname, bLongFileNameSupport))
	{
		FileName = strrchr(FinalFname, SLASH) + 1;
		wsprintf(buf, "%s%s%s", DirName,
		((DirName[strlen(DirName) - 1] == SLASH)? "" : SLASHSTR), FileName);

		// Let's get the file
		EBFile = DEBUG_NEW_NOTHROW JJFile;
		if (!EBFile || EBFile->Open(buf, O_CREAT | O_WRONLY) < 0)
		{
			delete EBFile;
			EBFile = NULL;
		}
	}

	return (EBFile);
}


// SingleReader
// All it really does it grab the creator and type
// 
BoundaryType SingleReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	MIMEState* ms;
	MIMEState* parentMS = NULL;
	HeaderDesc* hd;
	MIMEMap mm;
	BoundaryType boundaryType = btEndOfMessage;
	BOOL decode = FALSE;
	LONG size;
	JJFile* WriteFile = g_pPOPInboxFile;
	JJFile* AttachFile = NULL;
	char ProgBuf[128];
	long HeaderCount = 0;
    char Filename[_MAX_PATH + 1];
	char* NamePtr = Filename;
	char TypeCreator[8];
	long NameOffset = 0x7FFFFFFF, NameEnd = 0;
	long TypeCreatorOffset = 0, DataOffset = 0;
	int LastType = 0;
	long number = 0;
	
	// grab descriptors for our message
	if (!MimeStates.GetSize())
		return (btError);
	ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];
	hd = ms->m_hd;
	
	// find our birth mother
	if (MimeStates.GetSize() > 1)
		parentMS = (MIMEState*)MimeStates[MimeStates.GetUpperBound() - 1];

	// initialize the decoder
	decode = g_bAttachmentDecodeEnabled && ms->m_Coder;
	if (decode && ms->m_Coder->Init())
		return (btError);
	
	for (size = ms->m_LineReader->ReadLine(buf, bSize); size > 0; size = ms->m_LineReader->ReadLine(buf, bSize))
	{
		if (parentMS && (boundaryType = parentMS->IsBoundaryLine(buf)))
			break;

		if (decode)
			g_lBadCoding += ms->m_Coder->Decode(buf, size, buf, size);

		// Go through the header to find pertinent info about file
		if (g_bAttachmentDecodeEnabled && !AttachFile)
		{
			for (BYTE* b = (BYTE*)buf; size; HeaderCount++, b++, size--)
			{
				if (HeaderCount <= 0x19)
					;
				else if (HeaderCount < NameOffset)
				{
					int MapOffset = (int)((HeaderCount - 0x19 - 1) % 12 );
					int NumByte = MapOffset % 4;
					if (!NumByte)
						number = 0L;
					number |= ((long)*b) << ((3 - NumByte) * 8);
					if (MapOffset == 3)			// type
						LastType = (int)number;
					else if (MapOffset == 7)	// offset
					{
						switch (LastType)
						{
						case 1: DataOffset = number; break;
						case 3: NameOffset = number; break;
						case 9: TypeCreatorOffset = number; break;
						}
					}
					else if (MapOffset == 11)	// length
					{
						if (LastType == 3)
							NameEnd = NameOffset + number;
					}
				}
				else if (HeaderCount >= NameOffset && HeaderCount < NameEnd)
				{
					if ( (NamePtr - Filename) < (sizeof(Filename) - 1) )
					{
						*NamePtr++ = *b;
					}
					else
					{
						// We ran out of space in our Filename buffer
						// This is not the best way to handle this (e.g. file
						// file extensions could get chopped off), but it should
						// be good enough and avoid buffer overflows.

						// Alert developers running debug builds
						ASSERT(0);

						// Jump ahead to the end of the name
						b += NameEnd - HeaderCount - 1;
						HeaderCount = NameEnd - 1;

						// This shouldn't be necessary, but let's be sure
						NamePtr = Filename + (sizeof(Filename) - 1);
					}

					if (HeaderCount == NameEnd - 1)
					{
						// NULL terminate the file name
						*NamePtr++ = 0;
					}
				}
				else if (HeaderCount >= TypeCreatorOffset &&
					HeaderCount < TypeCreatorOffset + 8)
				{
					TypeCreator[HeaderCount - TypeCreatorOffset] = *b;
					if (HeaderCount == TypeCreatorOffset + 7 &&
						mm.Find(TypeCreator + 4, TypeCreator))
					{
						mm.SetExtension(Filename);
					}
				}
				else if (HeaderCount == DataOffset)
				{
					memcpy(buf, b, size);
					if (AttachFile = OpenAttachFile(Filename))
					{
						CString strFilename("???");
						{
							BSTR bstrFilename = NULL;
							if (SUCCEEDED(AttachFile->GetFName(&bstrFilename)))
							{
								strFilename = bstrFilename;
								::SysFreeString(bstrFilename);
							}
						}

						strncpy( Filename, strFilename, sizeof(Filename) );
						Filename[sizeof(Filename)-1] = '\0';

						char		szLogBuf[256];
						sprintf(szLogBuf, "LOGNULL SingleReader::ReadIt() JJFileMT::Truncate(%d)", hd->lDiskEnd);
						PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

						g_pPOPInboxFile->Truncate(hd->grokked? hd->lDiskStart : hd->lDiskEnd);
						WriteAttachNote(AttachFile, IDS_ATTACH_CONVERTED);
						// Start progress
						PushProgress();

						CString ("???");
						{
							BSTR bstrFilename = NULL;
							if (SUCCEEDED(AttachFile->GetFName(&bstrFilename)))
							{
								strFilename = bstrFilename;
								::SysFreeString(bstrFilename);
							}
						}
						wsprintf(ProgBuf, CRString(IDS_DECODING_ATTACH), strrchr(strFilename, '\\') + 1);
						ASSERT(strlen(ProgBuf) < sizeof(ProgBuf));
						Progress(0, ProgBuf, hd->RemainingBytes);
						WriteFile = AttachFile;
					}
					break;
				}
			}
		}
		
		if (size)
		{
			HRESULT hr = (WriteFile == g_pPOPInboxFile)?
								PutWithEscapedFileMarkers(buf, size) : WriteFile->Put(buf, size);
			if (FAILED(hr))
			{
				size = -1;
				break;
			}

			hd->RemainingBytes -= size;
			Progress(g_lEstimatedMsgSize - hd->RemainingBytes);
		}
	}

	if (size < 0)
		boundaryType = btError;

	if (AttachFile)
	{
		Progress(1, NULL, 1);
		delete AttachFile;
		PopProgress();
	}

	return (boundaryType);
}


/************************************************************************
 * ReadGeneric
 ************************************************************************/
BoundaryType GenericReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	MIMEState* ms = NULL;
	MIMEState* parentMS = NULL;
	HeaderDesc* hd;
	MIMEMap mm;
	BoundaryType boundaryType = btEndOfMessage;
	BOOL decode = FALSE;
	char fName[_MAX_PATH + 1];
	JJFile* AttachFile = NULL;
	JJFile* WriteFile;
	LONG size;
	
	// grab descriptors for our message
	if (!MimeStates.GetSize())
		return (btError);
	ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];
	hd = ms->m_hd;
	
	// find our birth mother
	if (MimeStates.GetSize() > 1)
		parentMS = (MIMEState*)MimeStates[MimeStates.GetUpperBound() - 1];

	// initialize the decoder
	decode = g_bAttachmentDecodeEnabled && ms->m_Coder;
	if (decode && ms->m_Coder->Init())
		return (btError);
	
	BOOL		 bDeleteAttachment = FALSE;

	if (g_bAttachmentDecodeEnabled)
	{
		// figure out the suggested filename
		*fName = 0;
		AttributeElement *AE = hd->GetAttribute(CRString(IDS_MIME_CONTENT_DISP_FILENAME));
		if (!AE)
			AE = hd->GetAttribute(CRString(IDS_MIME_NAME));
		if (AE)
			strncpy(fName, AE->m_Value, _MAX_PATH - 1);
		if (!*fName)
			strncpy(fName, ((MIMEState*)MimeStates[0])->m_hd->subj, _MAX_PATH - 1);
		if (!*fName)
			strncpy(fName, CRString(IDS_UNTITLED), _MAX_PATH - 1);
	
		if (mm.Find(ms))
		{
			mm.SetExtension(fName);
			if (mm.m_bDelete)
				bDeleteAttachment = TRUE;
		}
	
		if (GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_VCF))
		{
			int			 fNameLen = strlen(fName);
			if (fNameLen > 4)
			{
				bDeleteAttachment = (stricmp(fName + (fNameLen - 4), ".vcf") == 0);
			}
		}

		if (AttachFile = OpenAttachFile(fName))
		{
			if (hd->grokked /*&& mm.type != '????'*/)
			{
				char		szLogBuf[256];
				sprintf(szLogBuf, "LOGNULL GenericReader::ReadIt() JJFileMT::Truncate(%d)", hd->lDiskStart);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

				g_pPOPInboxFile->Truncate(hd->lDiskStart);
			}

			if (!bDeleteAttachment)
			{
				WriteAttachNote(AttachFile, IDS_ATTACH_CONVERTED);
			}

			// Start progress
			PushProgress();

			CString strFilename("???");
			{
				BSTR bstrFilename = NULL;
				if (SUCCEEDED(AttachFile->GetFName(&bstrFilename)))
				{
					strFilename = bstrFilename;
					::SysFreeString(bstrFilename);
				}
			}

			wsprintf(buf, CRString(IDS_DECODING_ATTACH), (const char*) strFilename);
			ASSERT(long(strlen(buf)) < bSize);
			Progress(0, buf, hd->RemainingBytes);
		}
	}

	WriteFile = AttachFile? AttachFile : g_pPOPInboxFile;
	
	for (size = ms->m_LineReader->ReadLine(buf, bSize); size > 0; size = ms->m_LineReader->ReadLine(buf, bSize))
	{
		if (parentMS && (boundaryType = parentMS->IsBoundaryLine(buf)))
		{
			// The last CR-LF was not actually part of the body,
			// so remove it if we're not doing Base64
			if (CompareRStringI(IDS_MIME_BASE64, hd->contentEnco))
			{
				long lCurrentSpot = 0;
				WriteFile->Tell(&lCurrentSpot);
				if (lCurrentSpot > 0)
				{
					char		szLogBuf[256];
					sprintf(szLogBuf, "LOGNULL GenericReader::ReadIt() JJFileMT::Truncate(%d) 2", lCurrentSpot - EOL_LEN);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

					WriteFile->Truncate(lCurrentSpot - EOL_LEN);
				}
			}
			break;
		}

		if (decode)
			g_lBadCoding += ms->m_Coder->Decode(buf, size, buf, size);
		
		if (size)
		{
			HRESULT hr = (WriteFile == g_pPOPInboxFile)?
								PutWithEscapedFileMarkers(buf, size) : WriteFile->Put(buf, size);
			if (FAILED(hr))
			{
				size = -1;
				break;
			}
			hd->RemainingBytes -= size;
			Progress(g_lEstimatedMsgSize - hd->RemainingBytes);
		}
	}
	if (size < 0)
	{
		boundaryType = btError;

		// get rid of the attachment
		if (AttachFile)
			AttachFile->Delete();
	}
	else if (bDeleteAttachment && AttachFile)
	{
		// get rid of the attachment
		AttachFile->Delete();
	}

	if (AttachFile)
	{
		Progress(1, NULL, 1);
		delete AttachFile;
		PopProgress();
	}

	return (boundaryType);
}

/************************************************************************
 * StripRich - remove rich text junk from a TE
 ************************************************************************/
/*
void StripRich(Handle text,long offset)
{
	Str31 nl,cmnt,lt,para,xrich;
	UPtr spot,end,cmStart,cmEnd,copySpot;
	short inComment = 0;
	Boolean pitch = False;
	Boolean neg;
	short inRich = 0;
	
	GetRString(nl,MIMERICH_NL);
	GetRString(lt,MIMERICH_LT);
	GetRString(cmnt,MIMERICH_COMMENT);
	GetRString(para,MIMERICH_PARA);
	GetRString(xrich,MIME_XRICH);

#define SETTOKEN do{cmStart=spot+1;if(neg=*cmStart=='/')cmStart++;for(cmEnd=cmStart;cmEnd<end && *cmEnd!='>';cmEnd++);}while(0)
#define	MATCH(s) !strincmp(cmStart,s+1,MAX(cmEnd-cmStart,*s))
#define COPY *copySpot++ = *spot++
#define SKIPCMD spot=cmEnd+1

	copySpot = spot = LDRef(text)+offset;
	end = spot + GetHandleSize(text)-offset;
	
	while (spot<end)
	{
		// copy bytes until rich text is turned on
		while (spot<end)
		{
			if (*spot=='<')
			{
				SETTOKEN;
				if (!neg && MATCH(xrich))
				{
					inRich++;
					SKIPCMD;
					break;
				}
			}
			COPY;
		}
		
		// copy the rich text segment
		while (spot<end && inRich)
		{
			// copy up to comment
			while (inRich && spot<end)
			{
				if (*spot=='\n')
				{
					if (copySpot==*text || copySpot[-1]=='\n')
					{
						spot++;
						continue;
					}
					*spot = ' ';
				}
				if (*spot != '<') COPY;
				else
				{
					SETTOKEN;
					if (MATCH(cmnt))
					{
						if (!neg)
						{
							inComment++;
							SKIPCMD;
							break;
						}
					}
					else if (MATCH(para))
					{
						if (neg)
						{
							*copySpot++ = '\n';
							*copySpot++ = '\n';
						}
					}
					else if (MATCH(lt)) *copySpot++ = '<';
					else if (MATCH(nl)) *copySpot++ = '\n';
					else if (MATCH(xrich)) if (neg) inRich--; else inRich++;
					SKIPCMD;
				}
			}	

			// ignore comments
			while (inComment && spot<end)
			{
				if (*spot != '<') spot++;
				else
				{
					SETTOKEN;
					if (MATCH(cmnt))
					{
						if (neg) inComment--; else inComment++;
					}
					SKIPCMD;
				}
			}	
		}
	}	
	
	SetHandleBig(text,copySpot-*text);
	UL(text);
}
*/


inline char HEX(char c)
{
	if (c >= '0' && c <= '9')
		return ((char)(c - '0'));
	if (c >= 'A' && c <= 'F')
		return ((char)(c - 'A' + 10));
	if (c >= 'a' && c <= 'f')
		return ((char)(c - 'a' + 10));
	return (-1);
}

void HexToString(char* dest, const char* src, int Num /*= 4*/)
{
	for (int i = 0; i < Num; i++)
	{
		int high = HEX(*src++) << 4;
		int low = HEX(*src++);
		*dest++ = (char)(low + high);
	}
}

// This will back in the in mailbox, get the header, parse out the header fields and return
// the address datastructure
CTLAddress *GetEMSHeaders(long diskStart, JJFileMT * in_mbox)
{
	CTLAddress *addresses = DEBUG_NEW CTLAddress;

	BOOL bBasic = GetTransMan()->GetHeaderFlag() & EMSAPI_ARRIVAL_BASIC;
	BOOL bRaw = GetTransMan()->GetHeaderFlag() & EMSAPI_ARRIVAL_RAW;
	
	// Do we need any headers?
	if (!bBasic && !bRaw)
		return addresses;

	// If we weren't provided a mailbox default to the POP inbox
	if (!in_mbox)
		in_mbox = g_pPOPInboxFile;


	// Back up the pointer in the mailbox
	long lCurrent = 0;
	in_mbox->Tell(&lCurrent);
	if (lCurrent < 0)
	{
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char szLogBuf[64];
			sprintf(szLogBuf, "LOGNULL GetEMSHeaders() Bad Tell");
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		}
	}
	else
	{
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char szLogBuf[64];
			sprintf(szLogBuf, "LOGNULL GetEMSHeaders() 1 JJFileMT::Seek(%ld)", diskStart);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		}

		HRESULT hr = in_mbox->Seek(diskStart);

		if (FAILED(hr))
		{
			char szLogBuf[64];
			sprintf(szLogBuf, "LOGNULL GetEMSHeaders() Bad Seek(0x%lx)", hr);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		}
		else
		{
			// Move what's currently in the message to memory, but only bother
			// doing this if there's something to read (i.e. bufSize ends
			// up being greater than zero).
			char *msgHeader = NULL;
			long bufSize = lCurrent - diskStart;
			if (bufSize > 0)
				msgHeader = DEBUG_NEW_NOTHROW char[bufSize + 1];

			if(msgHeader)
			{
				hr = in_mbox->Read(msgHeader, bufSize);
				if (FAILED(hr))
				{
					char szLogBuf[64];
					sprintf(szLogBuf, "LOGNULL GetEMSHeaders() Bad Read(0x%lx)", hr);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
				}
				else
				{
					char *to = NULL, *from = NULL, *cc = NULL, *subject = NULL, *bcc = NULL;
					if (bBasic)
					{
						to = HeaderContents(IDS_HEADER_TO, msgHeader);	
						from = HeaderContents(IDS_HEADER_FROM, msgHeader);
						cc = HeaderContents(IDS_HEADER_CC, msgHeader);	
						subject = HeaderContents(IDS_HEADER_SUBJECT, msgHeader);
					}

					addresses->CreateAddressList(to, 
												from, 
												subject, 
												cc, 
												bcc,
												bRaw ? msgHeader : NULL);
					delete [] to;
					delete [] from;
					delete [] cc;
					delete [] subject;
				}
				delete [] msgHeader;
			}

			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
			{
				char szLogBuf[64];
				sprintf(szLogBuf, "LOGNULL GetEMSHeaders() 2 JJFileMT::Seek(%ld)", lCurrent);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
			}

			// put file pointer back
			hr = in_mbox->Seek(lCurrent);
			if (FAILED(hr))
			{
				char szLogBuf[64];
				sprintf(szLogBuf, "LOGNULL GetEMSHeaders() Bad Seek(0x%lx)", hr);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
			}
		}
	}

	return addresses;
}


EmbeddedElement::EmbeddedElement()
	:	m_bWasReferenced(false), m_bIsEmoticon(false),
		m_Path(), m_CID(), m_MRI(0),
		m_CIDHash(0), m_CLHash(0), m_MRI_CB_CL_Hash(0)
{ 
}


void EmbeddedElement::Init( const char* path, DWORD MRI, const char* CID, const char* CB, const char* CL )
{
	m_Path = path;

	if (CID && *CID)
	{
		m_CID = (*CID == '<')? CID + 1 : CID;
		if (CID[strlen(CID) - 1] == '>')
			m_CID.ReleaseBuffer(m_CID.GetLength() - 1);
	}
	m_MRI = MRI;
	m_CIDHash = ::HashMT( m_CID );
	m_CLHash = 0;
	m_MRI_CB_CL_Hash = 0;

	if (CL && *CL)
	{
		int len = 40 + SafeStrlenMT(CB) + SafeStrlenMT(CL);
		char * buf = DEBUG_NEW_NOTHROW char[ len ];
		char * url = DEBUG_NEW_NOTHROW char[ len ];
		char * tmpcb = DEBUG_NEW_NOTHROW char[ SafeStrlenMT(CB) + 1];
		char * tmpcl = DEBUG_NEW_NOTHROW char[ SafeStrlenMT(CL) + 1];

		if ( buf && url && tmpcb && tmpcl )
		{
			// combine the ContentBase and ContentLocation headers into an abs URL
			if (CB)
				strcpy( tmpcb, CB );
			else
				*tmpcb = 0;
			strcpy( tmpcl, CL );
			URLCombine( url, tmpcb, tmpcl );
			wsprintf( buf, "%d%s", MRI, url );
			m_MRI_CB_CL_Hash = ::HashMT( buf );
		}

		delete [] buf;
		delete [] url;
		delete [] tmpcb;
		delete [] tmpcl;
	}
}

void EmbeddedElement::SetCID( const char* CID )
{
	m_CID = CID;

	int i = ( strnicmp( CID, "cid:", 4) == 0 ) ? 4 : 0;
	m_CIDHash = ::HashMT( CID + i );
}

int SaveEmbeddedElement( const char * filename )
{
	// currently processing a multipart/related which ^could^ be MHTML.
	// so cache the attached file info until the end of the mp/related
	// params == cid OR CB/CL
	EmbeddedElement* EE = DEBUG_NEW_MFCOBJ_NOTHROW EmbeddedElement();
	if ( EE )
	{
		// we always save to the <eudoradir>\Embedded directory
		const char * pName = strrchr( filename, '\\' );
		if ( pName )
			pName++;			// skip the backslash
		else
			pName = filename;	// no slash; use the whole thing

		EE->Init( pName, s_nMPRelatedID, 
			s_hdEmbeddedPart->contentID,
			s_hdEmbeddedPart->contentBase, 
			s_hdEmbeddedPart->contentLocation );

		if ( s_EmbeddedElements.AddTail(EE) )
			return 1;
	}

	return -1;
}

int WriteEmbeddedElements( BOOL isMHTML )
{
	if (!g_pPOPInboxFile)
		return 0;

	POSITION pos = s_EmbeddedElements.GetHeadPosition();
	while (pos)
	{
		EmbeddedElement* EE = (EmbeddedElement*)s_EmbeddedElements.GetNext(pos);

		if (FAILED(g_pPOPInboxFile->EnsureNewline())) goto fail;
		if ( isMHTML )
		{
			if (FAILED(g_pPOPInboxFile->Put(CRString(IDS_EMBEDDED_CONTENT)))) goto fail;
		}
		else
		{
			if (FAILED(g_pPOPInboxFile->Put(CRString(IDS_ATTACH_CONVERTED)))) goto fail;
		}
	
		if (FAILED(g_pPOPInboxFile->Put(EE->GetPath()))) goto fail;
		if (FAILED(g_pPOPInboxFile->Put(":"))) goto fail;

		char buf[ 128 ];
		wsprintf( buf, "%0.8x,%0.8x,%0.8x,%0.8x",
			EE->GetMRI(), EE->GetCIDHash(), EE->GetCLHash(), EE->GetMRI_CB_CL_Hash() );
		
		if (FAILED(g_pPOPInboxFile->Put(" "))) goto fail;
		if (FAILED(g_pPOPInboxFile->Put( buf ))) goto fail;
 

		if (FAILED(g_pPOPInboxFile->PutLine())) goto fail;
	}

	return 1;

fail:

	return -1;
}

void FlushEmbeddedElements()
{
	// clean the list
	while (s_EmbeddedElements.IsEmpty() == FALSE)
		delete s_EmbeddedElements.RemoveHead();
}

// Routine to write out some bytes to g_pPOPInboxFile while checking for
// spoofed Attachment Converted: and Embedded Content: lines
HRESULT PutWithEscapedFileMarkers(const char* pBuffer /*= NULL*/, long lNumBytesToWrite /*= -1L*/, bool bIsHTML /*= false*/)
{
	HRESULT		hr = S_OK;
	
	//	Check for NULL pBuffer - if NULL, we'll just return success (which is
	//	what calling g_pPOPInboxFile->Put with NULL would have done)
	if (pBuffer)
	{
		//	String resources for file marker lines
		CRString	szACMarker(IDS_ATTACH_CONVERTED);
		CRString	szEmbeddedMarker(IDS_EMBEDDED_CONTENT);

		//	Whether or not the current line needs to be escaped
		bool		bNeedsEscaped;
		
		//	Handle lNumBytesToWrite value of -1 in the same way that JJFileMT::Write_
		//	would have - by determining the string length of the buffer.
		if (lNumBytesToWrite < 0)
			lNumBytesToWrite = strlen(pBuffer);

		//	Start of the next line (if any)
		const char *	pNextLine = NULL;

		//	Points to just after the end of the buffer
		const char *	pEndBuffer = pBuffer + lNumBytesToWrite;
			
		while (pBuffer < pEndBuffer)
		{
			//	Check for a line break after which extra sneaky spoofed file
			//	marker lines could be hiding. Note that without any line breaks
			//	this just simplifies down to writing out the entire buffer like
			//	we did previously.
			pNextLine = static_cast<const char *>( memchr(pBuffer, '\r', pEndBuffer-pBuffer) );
			if (pNextLine)
			{
				//	Include the line breaks in the current line - skip past the '\r\n'
				if ( *pNextLine == '\r' || *pNextLine == '\n' ) pNextLine++;
				if ( *pNextLine == '\r' || *pNextLine == '\n' ) pNextLine++;
			}
			else
			{
				//	No line break - point just after the end of the buffer
				pNextLine = pEndBuffer;
			}

			//	Calculate how many bytes to write out for the current line
			lNumBytesToWrite = pNextLine - pBuffer;

			// Check for file marker lines
			bNeedsEscaped = ( lNumBytesToWrite > szACMarker.GetLength() ) &&
							( strnicmp(pBuffer, szACMarker, szACMarker.GetLength()) == 0 );

			if (!bNeedsEscaped)
			{
				bNeedsEscaped = ( lNumBytesToWrite > szEmbeddedMarker.GetLength() ) &&
								( strnicmp(pBuffer, szEmbeddedMarker, szEmbeddedMarker.GetLength()) == 0 );
			}
				
			if (bNeedsEscaped)
			{
				ASSERT(TRUE);	// Caught ya!

				//	If we're dealing with HTML, find the colon
				char *	pColon = bIsHTML ? static_cast<char *>(memchr(pBuffer, ':', lNumBytesToWrite)) : NULL;
				
				if (pColon)
				{
					//	We found the colon
					long	lNumBytesBeforeColon = pColon - pBuffer;
					
					//	Write out the buffer up to, but not including, the colon
					hr = g_pPOPInboxFile->Put(pBuffer, lNumBytesBeforeColon);
					if (FAILED(hr))
						break;

					//	Write out the HTML numeric entity for a colon.
					//	If we're legitimately still in HTML, this has the advantage of
					//	appearing exactly like the sender intended. If we're not still
					//	in HTML because someone sent HTML with an x-html end tag in it,
					//	then we're dealing with a spoof attempt, and I don't really care
					//	if the user sees the "&#058;".
					hr = g_pPOPInboxFile->Put("&#058;", 6);
					if (FAILED(hr))
						break;

					//	Move past the colon and adjust the number of bytes to write
					pBuffer = pColon + 1;
					lNumBytesToWrite -= lNumBytesBeforeColon + 1;
				}
				else
				{
					//	Write out '#' to escape the file marker
					hr = g_pPOPInboxFile->Put("#", 1);
					if (FAILED(hr))
						break;
				}
			}

			//	Write out the buffer up to and including any line breaks
			hr = g_pPOPInboxFile->Put(pBuffer, lNumBytesToWrite);
			if (FAILED(hr))
				break;

			//	Move on to the next line (if any)
			pBuffer = pNextLine;
		}
	}

	return hr;
}
