// TextReader.cpp
//
// MIMEReader class for reading in text/* MIME parts

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "Html2Text.h"
#include "TextReader.h"
#include "LineReader.h"
#include "header.h"
#include "mime.h"
#include "uudecode.h"
#include "hexbin.h"
#include "pop.h"
#include "fileutil.h"
#include "progress.h"
#include "StripRich.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// StripRich
//
// Strips out formatting commands in text/richtext and text/enriched
// body parts.  Returns the new length of the buffer.
//
LONG TextReader::StripRich(LPTSTR buf, LONG size)
{
	CStripRichString szStripped;

	buf[size] = 0;		// make sure input is NULL delimited
	szStripped.Strip(buf);
	ASSERT(szStripped.GetLength() <= size);	// it should be same or smaller...
	strncpy(buf, szStripped, size);
	buf[size] = 0;		// make sure it's NULL delimited

	return strlen(buf);
}

// size is equiv to strlen( buf ) 'cept buf might not be NULL delimited on input
// this assumes that buf has at least [ size + 1 ] characters ( [0..size] )
LONG TextReader::StripHtml(LPTSTR buf, LONG size)
{
	buf[size] = 0;		// make sure input is NULL delimited
	CString szStripped(Html2Text(buf));
	ASSERT(szStripped.GetLength() <= size);	// it should be same or smaller...
	strncpy(buf, szStripped, size);
	buf[size] = 0;		// make sure it's NULL delimited

	return strlen(buf);
}

// ReadText - read a plain text MIME message body
//
BoundaryType TextReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	MIMEState* ms = (MIMEState *)MimeStates[MimeStates.GetUpperBound()];
	const BOOL bRichtext	= !CompareRStringI(IDS_MIME_RICHTEXT, ms->m_hd->contentSubType);
	const BOOL bEnriched	= !CompareRStringI(IDS_MIME_ENRICHED, ms->m_hd->contentSubType);
	const BOOL bHtml		= !CompareRStringI(IDS_MIME_HTML,	  ms->m_hd->contentSubType);
	BOOL bFormatFlowed = FALSE;
	BoundaryType boundaryType = btEndOfMessage;
	AttributeElement* charset;
	MIMEState* parentMS;
	UINT XlateIndex;
	BOOL hexing = FALSE, uuing = FALSE, uuingLastLine = FALSE;
	DecodeUU UUDecoder(ms);
	DecodeBH BHDecoder;
	BOOL attach;
	BOOL decode;
	LONG size;
	long lHeaderEnd;

	attach = g_bAttachmentDecodeEnabled && !ms->m_hd->isPartial;
	if (charset = ms->m_hd->GetAttribute(CRString(IDS_MIME_CHARSET)))
	{
		XlateIndex = FindRStringIndexI(IDS_MIME_US_ASCII, IDS_MIME_ISO_LATIN1,
			charset->m_Value, -1);
	}
	
	// initialize the decoder
	decode = g_bAttachmentDecodeEnabled && ms->m_Coder;
	if (decode && ms->m_Coder->Init())
		return (btError);
	
	// is the header superfluous?
	if (MimeStates.GetSize() > 1 && ms->m_hd->grokked)
		g_pPOPInboxFile->ChangeSize(ms->m_hd->lDiskStart);
	g_pPOPInboxFile->Tell(&lHeaderEnd);
	ASSERT(lHeaderEnd >= 0);
	
	// write out rich text delimiter, if need be
//	if (ms->m_hd->hasRich)
	if (bRichtext || bEnriched) 
	{
		wsprintf(buf, CRString(IDS_MIME_RICH_ON), (const char*)CRString(IDS_MIME_XRICH));
		g_pPOPInboxFile->Put(buf);
	}
	else if (bHtml)
	{
		if ( ms->m_hd->contentBase[ 0 ] )
		{
			// <x-html content-base>
			//KCM	this was something Steve wanted in but we don't use it and it 
			//		causes trouble, so nix it for now...
			// wsprintf(buf, CRString(IDS_MIME_HTML_ON), (const char*)CRString(IDS_MIME_XHTML),
			//	(const char*) ms->m_hd->contentBase);

			// <x-html>
			// <BASE HREF="the content-base header string">
			wsprintf(buf, CRString(IDS_MIME_RICH_ON), (const char*)CRString(IDS_MIME_XHTML));
			g_pPOPInboxFile->PutLine(buf);
			wsprintf(buf, "<BASE HREF=%s>", (const char*) ms->m_hd->contentBase);
		}
		else
		{
			// <x-html>
			wsprintf(buf, CRString(IDS_MIME_RICH_ON), (const char*)CRString(IDS_MIME_XHTML));
		}
		g_pPOPInboxFile->PutLine(buf);
	}
	else
	{
		// Plain text, check for format=flowed
		AttributeElement* Format = ms->m_hd->GetAttribute(CRString(IDS_MIME_FORMAT));
		if (Format && !CompareRStringI(IDS_MIME_FLOWED, Format->m_Value))
		{
			bFormatFlowed = TRUE;
			wsprintf(buf, CRString(IDS_MIME_RICH_ON), (const char*)CRString(IDS_MIME_XFLOWED));
			g_pPOPInboxFile->Put(buf);
		}
	}
	
	// find our birth mother
	parentMS = (MIMEState*)(MimeStates.GetSize() <= 1?
		NULL : MimeStates[MimeStates.GetUpperBound() - 1]);
	
	// now, read the message
	for (size = ms->m_LineReader->ReadLine(buf, bSize); size > 0; size = ms->m_LineReader->ReadLine(buf, bSize))
	{
		if (parentMS && (boundaryType = parentMS->IsBoundaryLine(buf)))
		{
			// We don't want to strip off the last CRLF if we just ended the
			// uuencoded part because it will wind up stripping off the CRLF
			// of the Attachment Converted line, which causes Very Bad Things.
			if (!uuingLastLine)
			{
				// The last CR-LF was not actually part of the body,
				// so remove it
				long lCurrentSpot = 0;
				g_pPOPInboxFile->Tell(&lCurrentSpot);
				if (lCurrentSpot > 0)
					g_pPOPInboxFile->ChangeSize(lCurrentSpot - 2);
			}
			break;
		}

		// See hack a little bit above
		uuingLastLine = uuing;

		// decode
		if (decode)
			g_lBadCoding += ms->m_Coder->Decode(buf, size, buf, size);

		ms->m_hd->RemainingBytes -= size;
		if (!hexing && !uuing)
			Progress(g_lEstimatedMsgSize - ms->m_hd->RemainingBytes);
		
		// give each converter a crack at the line
		if (attach)
		{
			if (!uuing)
				hexing = BHDecoder.Decode(buf, size, buf, size);
			if (!hexing)
				uuing = UUDecoder.Decode(buf, size, buf, size);
		}
		
		// convert
//		if (XlateIndex > 0)
//		{
//			// XlateIndex = 0 is US ASCII which is not translated.  The
//			// first translation table is actually as position.
//			ISOTranslate(buf, size, XlateTables[XlateIndex - 1][ISO_TO_WIN]);
//		}

		// write the line
		if (size && FAILED(g_pPOPInboxFile->Put(buf, size)))
		{
			size = -1;
			break;
		}
	}
	if (size >= 0)
	{
		// write out rich text delimiter, if need be
	//	if (ms->m_hd->hasRich)
		if (bRichtext || bEnriched)
		{
			wsprintf(buf, CRString(IDS_MIME_RICH_OFF), (const char*)CRString(IDS_MIME_XRICH));
			if (FAILED(g_pPOPInboxFile->PutLine(buf)))
				size = -1;
		}
		else if (bHtml)
		{
			wsprintf(buf, CRString(IDS_MIME_HTML_OFF), (const char*)CRString(IDS_MIME_XHTML));
			if (FAILED(g_pPOPInboxFile->PutLine(buf)))
				size = -1;
		}
		else if (bFormatFlowed)
		{
			if (FAILED(g_pPOPInboxFile->EnsureNewline())) 
				size = -1;
			wsprintf(buf, CRString(IDS_MIME_RICH_OFF), (const char*)CRString(IDS_MIME_XFLOWED));
			if (FAILED(g_pPOPInboxFile->PutLine(buf)))
				size = -1;
		}
		if (!CompareRStringI(IDS_MIME_MESSAGE, ms->m_hd->contentType) &&
			!CompareRStringI(IDS_MIME_EXTERNAL_BODY, ms->m_hd->contentSubType))
		{
			AttributeElement* AE = ms->m_hd->GetAttribute(CRString(IDS_MIME_ACCESS_TYPE));
			if (AE)
			{
				if (!CompareRStringI(IDS_MIME_AT_FTP, AE->m_Value) ||
					!CompareRStringI(IDS_MIME_AT_TFTP, AE->m_Value) ||
					!CompareRStringI(IDS_MIME_AT_ANON_FTP, AE->m_Value))
				{
					g_pPOPInboxFile->ChangeSize(lHeaderEnd);
					strcpy(buf, "\r\n<ftp:");
					AE = ms->m_hd->GetAttribute(CRString(IDS_MIME_EB_SITE));
					if (AE && AE->m_Value[0])
					{
						strcat(buf, "//");
						strcat(buf, AE->m_Value);
					}
					AE = ms->m_hd->GetAttribute(CRString(IDS_MIME_EB_DIRECTORY));
					if (AE && AE->m_Value[0])
					{
						if (buf[strlen(buf) - 1] != '/')
							strcat(buf, "/");
						strcat(buf, AE->m_Value);
					}
					AE = ms->m_hd->GetAttribute(CRString(IDS_MIME_EB_NAME));
					if (AE && AE->m_Value[0])
					{
						if (buf[strlen(buf) - 1] != '/')
							strcat(buf, "/");
						strcat(buf, AE->m_Value);
					}
					strcat(buf, ">\r\n");
					if (FAILED(g_pPOPInboxFile->Put(buf)))
						size = -1;
				}
				else if (!CompareRStringI(IDS_MIME_AT_LOCAL_FILE, AE->m_Value))
				{
					g_pPOPInboxFile->ChangeSize(lHeaderEnd);
					AE = ms->m_hd->GetAttribute(CRString(IDS_MIME_EB_NAME));
					if (AE && AE->m_Value[0])
					{
						sprintf(buf, "\r\n%s\"%s\"\r\n", (const char*)CRString(IDS_ATTACH_CONVERTED),
							(const char*)AE->m_Value);
						if (FAILED(g_pPOPInboxFile->Put(buf)))
							size = -1;
						g_bHasAttachment = TRUE;
					}
				}
			}
		}
	}
	
	return (size < 0 ? btError : boundaryType);
}
