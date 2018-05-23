// MIME.CPP
//
// Routines for MIME
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
#include "TransReader.h"
#include "LineReader.h"
#include "MIMEmap.h"
#include "mime.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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

	FlushEmbeddedElements();
}


/************************************************************************
 * Other MIME stuff
 ************************************************************************/

MIMEState::MIMEState(LineReader* lr, HeaderDesc* hd/*=NULL*/)
{
	m_hd = NULL;
	m_Coder = NULL;
	m_Reader = NULL;
	m_boundary[0] = 0;
	m_Translator = NULL;
	m_LineReader = NULL;
	
	if (lr)
		SetLineReader(lr);

	if (hd)
		Init(hd);
}

int MIMEState::Init(HeaderDesc* hd)
{
	AttributeElement* AE;
	m_hd = hd;
	CTranslator *trans = NULL;
	long err = EMSR_CANT_TRANS;

	if (!m_hd->isMIME)
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
			m_Reader = new NowTransReader;
			m_Translator = trans;
		}
		else if (err == EMSR_NOT_NOW)
		{
			m_Reader = new NotNowTransReader;
			m_Translator = trans;
		}		
		else
		{
			m_Coder = NULL;
			m_Reader = new TextReader;
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
				wsprintf(m_boundary, "--%s", (const char*)AE->m_Value);
		}
		// Find the reader for this part
		m_Reader = NULL;
	
		// Here's where we can see if translators should do something ...
		CTranslator *trans = NULL;
		long err = EMSR_CANT_TRANS;


		// Get all the addresses
		CTLAddress *addresses = GetEMSHeaders(m_hd->lDiskStart);
//		if (!g_tempaddresses)
//		{
//			g_tempaddresses = GetEMSHeaders(m_hd->lDiskStart);
//		}

		if (g_bAttachmentDecodeEnabled)
			err = GetTransMan()->CanXLateMessage(m_hd->m_TLMime, EMSF_ON_ARRIVAL, &trans, addresses);
		
		if (addresses) delete addresses;

		CRString CTEuuencode(IDS_MIME_UUENCODE);
		CRString CTEuue(IDS_MIME_UUE);
		if(err == EMSR_NOW)
		{
			m_Reader = new NowTransReader;
			m_Translator = trans;
		}
		else if (err == EMSR_NOT_NOW)
		{
			m_Reader = new NotNowTransReader;
			m_Translator = trans;
		}
		// Content-Transfer-Encoding of x-uuencode, x-uue, uuencode, or uue goes
		// to the TextReader, which knows how to deal with uuencoded data
		else if (!stricmp(CTEuuencode, m_hd->contentEnco) ||
				 !stricmp(CTEuue, m_hd->contentEnco) ||
				 !stricmp(((LPCTSTR)CTEuuencode) + 2, m_hd->contentEnco) ||
				 !stricmp(((LPCTSTR)CTEuue) + 2, m_hd->contentEnco))
		{
			m_Reader = new TextReader;
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
					if (m_hd->grokked && FAILED(g_pPOPInboxFile->ChangeSize(m_hd->lDiskStart)))
						return (FALSE);

					m_hd->Init();
					m_hd->isMIME = TRUE;

					if (m_hd->Read(GetLineReader()) != EndOfHeader)
						return (FALSE);

					return (Init(m_hd));
				}
			}
			m_Reader = new TextReader;
		}
		else if (!CompareRStringI(IDS_MIME_TEXT, contentType))
		{
			AE = m_hd->GetAttribute(CRString(IDS_MIME_HEADER_CONTENT_DISP));
			if (AE && !CompareRStringI(IDS_MIME_CONTENT_DISP_ATTACH, AE->m_Value))
				m_Reader = new GenericReader;
			else
				m_Reader = new TextReader;
		}
		else if (!CompareRStringI(IDS_MIME_MULTIPART, contentType))
			m_Reader = new MultiReader;
		else if (!CompareRStringI(IDS_MIME_APPLICATION, contentType))
		{
			if (!CompareRStringI(IDS_MIME_BINHEX, contentSubType))
				m_Reader = new TextReader;
			else if (!CompareRStringI(IDS_MIME_APPLEFILE, contentSubType))
				m_Reader = new SingleReader;
		}
		if (!m_Reader)
			m_Reader = new GenericReader;
		
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
	if (!strncmp(m_boundary, buf, strlen(m_boundary)))
	{
		const char* bEnd = buf + strlen(m_boundary);
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
	int ReadStatus;

	for (ReadStatus = m_LineReader->ReadLine(buf, bSize); ReadStatus > 0;
		ReadStatus = m_LineReader->ReadLine(buf, bSize))
	{
		// write the line
		if (FAILED(g_pPOPInboxFile->PutLine(buf)))
			return (btError);
		
		// is it the first boundary?
		if (boundaryType = IsBoundaryLine(buf))
			return (boundaryType);
	}
	
	return (ReadStatus? btError : btEndOfMessage);
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
		return (AllocateCoder? new Base64Decoder : (Decoder*)TRUE);
	else if (!CompareRStringI(IDS_MIME_QP, encoding))
		return (AllocateCoder? new QPDecoder : (Decoder*)TRUE);

	return (NULL);
}


/************************************************************************
 * FindMIMECharset - find the right xlate table for a particular MIME char set
 ************************************************************************/
short FindMIMECharset(const char* charSet)
{
	if (!CompareRStringI(IDS_MIME_ISO_LATIN1, charSet))
		return (1);
		
	return (0);
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
		if (FAILED(g_pPOPInboxFile->ChangeSize(((MimeStates.GetSize() > 1 && hd->grokked)?
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
			if (!(innerHD = new HeaderDesc(hd->RemainingBytes))) goto error;

			// Content-Base must inherit
			strcpy( innerHD->contentBase, hd->contentBase );

			// If this is multipart/digest, then the default for all subparts
			// is message/rfc822
			if (!CompareRStringI(IDS_MIME_DIGEST, hd->contentSubType))
			{
				GetIniString(IDS_MIME_MESSAGE, innerHD->contentType, sizeof(innerHD->contentType));
				GetIniString(IDS_MIME_RFC822, innerHD->contentSubType, sizeof(innerHD->contentSubType));
			}

			// read the header
			if ((Result = innerHD->Read(ms->GetLineReader())) != EndOfHeader) goto error;

			// All sub parts are considered MIME, too
			innerHD->isMIME = TRUE;

			// extract MIME info
			if (!(innerMS = new MIMEState(ms->GetLineReader(), innerHD)) || !innerMS->m_Reader)
				goto error;
			MimeStates.Add(innerMS);

			if (alternative)
			{ 
				CString newSubtype; 
				if (!CompareRStringI(IDS_MIME_PLAIN, innerHD->contentSubType))
					newSubtype = innerHD->contentSubType;
				else if (!CompareRStringI(IDS_MIME_ENRICHED, innerHD->contentSubType))
					newSubtype = innerHD->contentSubType;
				else if (!CompareRStringI(IDS_MIME_HTML, innerHD->contentSubType))
					newSubtype = innerHD->contentSubType;
				else 
					newSubtype.Empty();
				
				// This is an unknown alternative, so just show it!
				if (newSubtype.IsEmpty())
					eatOffset = 0;


				// Is this something better (text/enriched,text/html, original empty)
				else if (!CompareRStringI(IDS_MIME_ENRICHED, newSubtype) ||
					( !CompareRStringI(IDS_MIME_HTML, newSubtype) &&
					  CompareRStringI(IDS_MIME_ENRICHED, altSubtype) ) ||
					  altSubtype.IsEmpty()) 
				{
					// do we have something better?
					if (altOffset)
					{
						g_pPOPInboxFile->ChangeSize(altOffset);
						innerHD->lDiskEnd = altOffset + innerHD->lDiskEnd - innerHD->lDiskStart;
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

			// If this is an appledouble file, we need to get the filename
            // from the previous part and remove the file created by the
			// first part
			if (wasApplefile)
			{
            	CRString Name(IDS_MIME_NAME);
				AttributeElement* AE = innerHD->GetAttribute(Name);
				char* p = strrchr(ApplefileName, SLASH);
				::FileRemoveMT(ApplefileName);
				if (p)
					strcpy(ApplefileName, p + 1);
				if (AE)
					AE->ChangeValue(ApplefileName);
				else
				{
                	AE = new AttributeElement(Name, ApplefileName); 
					innerHD->Attributes.AddTail(AE);
				}
				// Make sure extension doesn't get reinterpretted by clearing
                // out the MIME type and subtype
				innerHD->contentType[0] = innerHD->contentSubType[0] = 0;
			}

			// read the body of the message
			s_hdEmbeddedPart = innerHD;
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
				if (FAILED(g_pPOPInboxFile->ChangeSize(lOffset)))
					goto error;
			}

			MimeStates.RemoveAt(MimeStates.GetUpperBound());
			delete innerHD; innerHD = NULL;
			delete innerMS; innerMS = NULL;
	
	        // We're really getting rid of this part
			if (eatOffset)
			{	
				g_pPOPInboxFile->ChangeSize(eatOffset);  
				eatOffset = 0; 
			}
		}
	}
	
	if (boundaryType == btOuterBoundary)
	{
		// write out the final boundary
//JES		if (ms->WriteBoundary(/*g_pPOPInboxFile,*/ boundaryType) < 0) goto error;
		
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
		
		g_pPOPInboxFile->ChangeSize(lOffset);		// toss the excess bytes
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



int CheckName(const char* Dir, const char* FName, char* FinalFname, BOOL bLongFileNameSupport)
{
	char DirName[_MAX_PATH + 1];

	strcpy(DirName, Dir);
	if (DirName[strlen(DirName) - 1] == SLASH)
		DirName[strlen(DirName) - 1] = NULL;

	int TryNum = 1, len;
	char tmp[255], *ptr, *dot;
	int BaseLen;
	
	//gets the string of DOS reserved device names
	CString sReservedWords;
	char *pReserved = sReservedWords.GetBuffer(1024);
	GetIniString(IDS_INI_DOS_RESERVED, pReserved, 1024);
	sReservedWords.ReleaseBuffer();

	if ((strlen(DirName) + strlen(FName)) > _MAX_PATH-2)	// The file is too big to create...
	{
		strncpy(FinalFname, DirName, strlen(DirName)+1);
		int endofFname = strlen(FinalFname);
		FinalFname[endofFname] = SLASH;
		FinalFname[endofFname+1] = 0;

		int Difference = (strlen(DirName) + strlen(FName))- (_MAX_PATH - 2);
		strncat(FinalFname, FName, (strlen(FName)-Difference));

		char * floater;
		floater = strrchr(FName, '.');
		
		if (floater)	// There's a period, so there's an extension.
		{
			char *leader;
			leader = FinalFname + (((strlen(DirName) + strlen(FName))-Difference) - strlen(floater));
			while (*floater)
			{
				*leader = *floater;
				leader++;
				floater++;
			}
			*leader = 0;
		}
	}

	else
	{
		wsprintf(FinalFname, "%s%c%s", DirName, SLASH, FName);
	}


	//variables definition
	BOOL isReserved = FALSE;
	pReserved = (char *)(LPCTSTR)sReservedWords;
	CString sResWord;
	char *DOSptr;
	//store in tmp the file name without the extension
	DOSptr = strchr(FName, '.');
	len = (DOSptr? DOSptr - FName : strlen(FName));
	strncpy(tmp, FName, len);
	tmp[len] = 0;

	while (1)
	{
		//loop to see if tmp has one of the DOS reserved device names
		while (*pReserved != NULL)
		{
			if ((DOSptr = strchr(pReserved, ',')) != NULL)
			{
				//store in sResWord one DOS device name at a time so it can be compared
				len = DOSptr - pReserved;
				ASSERT( len < 9);
//				strncpy((char*)(LPCTSTR)sResWord, pReserved, len);
				sResWord = CString(pReserved);
				char* pResWord = (char*)(LPCTSTR)sResWord;
				pResWord[len]='\0';
			}
			else
				//Last device name so copy it
//				strcpy((char*)(LPCTSTR)sResWord, pReserved);
				sResWord = CString(pReserved);
		
			if (stricmp(tmp, sResWord) == 0)	//if match
			{
				isReserved = TRUE;
				break;
			}
			else
				isReserved = FALSE;

			//if there are still device names to compare
			if (DOSptr)
				pReserved = DOSptr + 1;
			else
				break;
		}

		if (!isReserved && !::FileExistsMT(FinalFname))
        	break;

		// point to the beginning of the device string just in case the new namebecomes
		// one of the reserved DOS device names
		pReserved = (char *)(LPCTSTR)sReservedWords;

		// More than 4 characters after the last period is not an extension
		ptr = strrchr(FName, '.');
		if (ptr && strlen(ptr + 1) <= 4)
			len = ptr - FName;
		else
		{
			len = strlen(FName);
			ptr = NULL;
		}
		if (bLongFileNameSupport)
			BaseLen = 255;
		else
			BaseLen = 8;
			
		BaseLen -= (TryNum < 10? 1 : (TryNum < 100? 2 : 3));
		len = min(len, BaseLen);
		strncpy(tmp, FName, len);
		tmp[len] = 0;

		char tmp2[255];
		// do the following to add the number to the reserved name and the following
		// dot seperated section of the name.  THIS IS A CRUCIAL IF statement;
		// IF DELETED IT WILL INFINITLY LOOP 
		if ((dot = strchr(tmp, '.')) && isReserved)
		{
			strcpy(tmp2, dot);
			*dot = 0;
			wsprintf(FinalFname, "%s\\%s%d%s%0.4s", DirName, tmp, TryNum++, tmp2, (ptr? ptr : ""));
		}
		else
			wsprintf(FinalFname, "%s\\%s%d%0.4s", DirName, tmp, TryNum++, (ptr? ptr : ""));
	
		wsprintf(tmp, "%s%d", tmp, TryNum);
	}

	return (TRUE);
}

// OpenAttachFile
//
// Legalizes the file name and opens the attachment file
//
JJFile* OpenAttachFile(char* Name)
{
	// this is part of the uglyness.  OpenAttachFile and WriteAttachNote are
	// called from lots o' places in the code that know not the context in which
	// they work.  With MHTML, sometimes we want to save stuff to the Embedded directory
	// and write "Embedded content:" lines verses saving stuff to the Attach dir and
	// writing "Attachment converted:" lines.  Rather than changing the world I use a
	// static to determine if we are working with MHTML or plain ol' attachments...
	if ( s_bMPRelated )
		return OpenEmbeddedFile( Name );

    JJFile* ATFile;
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
	}

	// Let's get the file
	ATFile = new JJFile;
	if (ATFile)
	{
		if (FAILED(ATFile->Open(buf, O_CREAT | O_WRONLY)))
		{
			delete ATFile;
			ATFile = NULL;
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
    JJFile* EBFile;
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
	}

	// Let's get the file
	EBFile = new JJFile;
	if (!EBFile || EBFile->Open(buf, O_CREAT | O_WRONLY) < 0)
	{
		if (EBFile)
			delete EBFile;
        EBFile = NULL;
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
    char Filename[sizeof(hd->attributeName) + 5]; // Room for extension and 0
	char* NamePtr = Filename;
	char TypeCreator[8];
	long NameOffset = 0x7FFFFFFF, NameEnd = 0, TypeOffset = 0x7FFFFFFF;
	long TypeCreatorOffset = 0, DataOffset = 0;
	int LastType;
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
					*NamePtr++ = *b;
					if (HeaderCount == NameEnd - 1)
					{
						// Put the mapped name in the attribute name field for
						// possible later perusal
						*NamePtr++ = 0;
						strcpy(hd->attributeName, Filename);
					}
				}
				else if (HeaderCount >= TypeCreatorOffset &&
					HeaderCount < TypeCreatorOffset + 8)
				{
					TypeCreator[HeaderCount - TypeCreatorOffset] = *b;
					if (HeaderCount == TypeCreatorOffset + 7 &&
						mm.Find(TypeCreator + 4, TypeCreator))
					{
						mm.SetExtension(hd->attributeName);
					}
				}
				else if (HeaderCount == DataOffset)
				{
					memcpy(buf, b, size);
					if (AttachFile = OpenAttachFile(hd->attributeName))
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

						strcpy(hd->attributeName, strFilename);
						g_pPOPInboxFile->ChangeSize(hd->grokked? hd->lDiskStart : hd->lDiskEnd);
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
			if (FAILED(WriteFile->Put(buf, size)))
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
			mm.SetExtension(fName);
	
		if (AttachFile = OpenAttachFile(fName))
		{
			if (hd->grokked /*&& mm.type != '????'*/)
				g_pPOPInboxFile->ChangeSize(hd->lDiskStart);

			WriteAttachNote(AttachFile, IDS_ATTACH_CONVERTED);

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
					WriteFile->ChangeSize(lCurrentSpot - EOL_LEN);
			}
			break;
		}

		if (decode)
			g_lBadCoding += ms->m_Coder->Decode(buf, size, buf, size);
		
		if (size)
		{
			if (FAILED(WriteFile->Put(buf, size)))
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
CTLAddress *GetEMSHeaders(long diskStart)
{
	CTLAddress *addresses = new CTLAddress;

	BOOL bBasic = GetTransMan()->GetHeaderFlag() & EMSAPI_ARRIVAL_BASIC;
	BOOL bRaw = GetTransMan()->GetHeaderFlag() & EMSAPI_ARRIVAL_RAW;
	
	// Do we need any headers?
	if (!bBasic && !bRaw)
		return addresses;


	// Back up the pointer in the mailbox
	long lCurrent = 0;
	g_pPOPInboxFile->Tell(&lCurrent);
	ASSERT(lCurrent >= 0);
	g_pPOPInboxFile->Seek(diskStart);

	// move what's currently in the message to memory
	long bufSize = lCurrent - diskStart;
	char *msgHeader = new char[bufSize + 1];
	if(msgHeader)
	{
		g_pPOPInboxFile->Read(msgHeader, bufSize);
	
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
		if (to) delete []to;
		if (from) delete []from;
		if (cc) delete []cc;
		if (subject) delete []subject;
		delete[] msgHeader;
	}
	// put file pointer back
	g_pPOPInboxFile->Seek(lCurrent);		

	return addresses;
}

void EmbeddedElement::Init( const char* path, DWORD MRI, const char* CID, const char* CB, const char* CL )
{
	m_Path = path;
	m_CID = (*CID == '<')? CID + 1 : CID;
	if (CID[strlen(CID) - 1] == '>')
		m_CID.ReleaseBuffer(m_CID.GetLength() - 1);
	m_MRI = MRI;
	m_CIDHash = ::HashMT( m_CID );
	m_CLHash = 0;
	m_MRI_CB_CL_Hash = 0;

	if ( CL && *CL )
	{
		int len = 40 + strlen( CB ) + strlen( CL );
		char * buf = new char[ len ];
		char * url = new char[ len ];
		char * tmpcb = new char[ strlen(CB) ];
		char * tmpcl = new char[ strlen(CL) ];

		if ( buf && url && tmpcb && tmpcl )
		{
			// combine the ContentBase and ContentLocation headers into an abs URL
			strcpy( tmpcb, CB );
			strcpy( tmpcl, CL );
			URLCombine( url, tmpcb, tmpcl );
			wsprintf( buf, "%d%s", MRI, url );
			m_MRI_CB_CL_Hash = ::HashMT( buf );
		}

		delete buf;
		delete url;
		delete tmpcb;
		delete tmpcl;
	}
}

void EmbeddedElement::SetCID( CString CID )
{
	m_CID = CID;

	// the sadness of "CStrings"
	char* pCid = CID.GetBuffer( _MAX_PATH );
	int i = ( strnicmp( pCid, "cid:", 4) == 0 ) ? 4 : 0;
	m_CIDHash = ::HashMT( &pCid[i] );
	CID.ReleaseBuffer();
}

int SaveEmbeddedElement( const char * filename )
{
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
