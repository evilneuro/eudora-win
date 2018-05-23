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
#include "utils.h"


#include "DebugNewHelpers.h"


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
	const bool bRichtext	= !CompareRStringI(IDS_MIME_RICHTEXT, ms->m_hd->contentSubType);
	const bool bEnriched	= !CompareRStringI(IDS_MIME_ENRICHED, ms->m_hd->contentSubType);
	const bool bHtml		= !CompareRStringI(IDS_MIME_HTML,	  ms->m_hd->contentSubType);
	BOOL bFormatFlowed = FALSE;
	BoundaryType boundaryType = btEndOfMessage;
	AttributeElement* charset;
	AttributeElement* AEFilename = ms->m_hd->GetAttribute(CRString(IDS_MIME_CONTENT_DISP_FILENAME));
	MIMEState* parentMS;
	int iCharsetIdx = 0;
	BOOL hexing = FALSE, uuing = FALSE, uuingLastLine = FALSE;
	DecodeUU UUDecoder(ms);
	DecodeBH BHDecoder(AEFilename? (LPCTSTR)AEFilename->m_Value : NULL);
	BOOL attach;
	BOOL decode;
	LONG size;
	long lHeaderEnd;

	attach = g_bAttachmentDecodeEnabled && !ms->m_hd->isPartial;

	// Determine the index of the message's charset for translation below.
	if (charset = ms->m_hd->GetAttribute(CRString(IDS_MIME_CHARSET)))
	{
		iCharsetIdx = FindMIMECharset(charset->m_Value);
	}

	// initialize the decoder
	decode = g_bAttachmentDecodeEnabled && ms->m_Coder;
	if (decode && ms->m_Coder->Init())
		return (btError);
	
	// is the header superfluous?
	if (MimeStates.GetSize() > 1 && ms->m_hd->grokked)
	{
		char		szLogBuf[256];
		sprintf(szLogBuf, "LOGNULL TextReader::ReadIt() JJFileMT::Truncate(%d)", ms->m_hd->lDiskStart);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		g_pPOPInboxFile->Truncate(ms->m_hd->lDiskStart);
	}
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
			g_pPOPInboxFile->PutLine(buf);
		}
	}
	
	// find our birth mother
	parentMS = (MIMEState*)(MimeStates.GetSize() <= 1?
		NULL : MimeStates[MimeStates.GetUpperBound() - 1]);
	
	// now, read the message
	BOOL bGotSomething = FALSE;

	CString				szGatheredTextBuffer;
	bool				bSupportGatheredBuffer = true;
	bool				bLineDidNotEndWithCRLF = true;

	try
	{
		// Preallocate the gathered text buffer to be as big as the decode buffer
		szGatheredTextBuffer.GetBuffer(bSize);
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException in TextReader::ReadIt" );
		bSupportGatheredBuffer = false;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception in TextReader::ReadIt" );
		bSupportGatheredBuffer = false;
	}

	// Adjust downward by one byte the buffer that we pass to ReadLine, so that
	// we can be guaranteed that there's space for ISOTranslate to NULL terminate
	// the buffer. It's safer to make ourselves leave space for ISOTranslate than
	// to change ISOTranslate.
	bSize--;

	for (size = ms->m_LineReader->ReadLine(buf, bSize); size > 0; size = ms->m_LineReader->ReadLine(buf, bSize))
	{
		if (parentMS && (boundaryType = parentMS->IsBoundaryLine(buf)))
		{
			if ( bSupportGatheredBuffer && !szGatheredTextBuffer.IsEmpty() )
			{
				// If we had any left over text left in our gathered text buffer
				// better write it out now
				if ( FAILED(PutWithEscapedFileMarkers(szGatheredTextBuffer, szGatheredTextBuffer.GetLength(), bHtml)) )
				{
					size = -1;
					break;
				}

				szGatheredTextBuffer.Empty();
			}
			
			// We don't want to strip off the last CRLF if we just ended the
			// uuencoded part because it will wind up stripping off the CRLF
			// of the Attachment Converted line, which causes Very Bad Things.
			//
			// We also don't want to remove the last CRLF if the part was
			// completely empty, which can also cause VBT.
			if (!bLineDidNotEndWithCRLF && !uuingLastLine && bGotSomething)
			{
				// The last CR-LF was not actually part of the body, so remove it
				long lCurrentSpot = 0;
				g_pPOPInboxFile->Tell(&lCurrentSpot);
				if (lCurrentSpot > 0)
				{
					char		szLogBuf[256];
					sprintf(szLogBuf, "LOGNULL TextReader::ReadIt() JJFileMT::Truncate(%d) 2", lCurrentSpot - 2);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

					g_pPOPInboxFile->Truncate(lCurrentSpot - 2);
				}
			}
			break;
		}

		bGotSomething = TRUE;

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
		
		// iCharsetIdx = 0 is Windows, 1 is US ASCII and 2 is Latin1 which are
		// not translated so only translate if the index is greater than 2.
		if (iCharsetIdx > 2)
		{
			// As a first pass at handling other charsets we pass the text
			// through a translator function.  A more elegant solution would
			// be to create a decoder for other charsets.
			ISOTranslate(buf, size, iCharsetIdx);
		}

		if (size)
		{
			// Check for sneaky Attachment Converted: lines trying to fake the user out.
			// Escape them if found.

			// Check to see if the buffer ends with a CRLF or not.
			bLineDidNotEndWithCRLF = (size < 2) || (buf[size-2] != '\r') || (buf[size-1] != '\n');

			// Check to see if the buffer contains a NULL character
			bool	bContainsNULLChar = (memchr(buf, 0, size) != NULL);

			// Go into our gathering up a buffer behavior if:
			// * We already have a gathered up buffer with stuff in it
			// * The current decoded buffer does not end with CRLF (could be attempting to slip a file marker past us)
			// * The current decoded buffer contains a NULL (again could be attempting to slip a file marker past us)
			if ( bSupportGatheredBuffer && (!szGatheredTextBuffer.IsEmpty() || bLineDidNotEndWithCRLF || bContainsNULLChar) )
			{
				// Remember the old length of the gathered buffer
				int		nLength = szGatheredTextBuffer.GetLength();

				// Calculate where to tack on the new buffer
				char *	pGatheredTextBuffer = szGatheredTextBuffer.GetBuffer(nLength + size) + nLength;

				// Tack on the new buffer and set the CString back to normal operation with the new length
				memcpy(pGatheredTextBuffer, buf, size);
				szGatheredTextBuffer.ReleaseBuffer(nLength + size);

				// Remove any NULL characters
				if (bContainsNULLChar)
					szGatheredTextBuffer.Remove(0);
				
				// Check to see if there's any line endings so that we can write out at least part of the buffer
				int		nSizeWithTerminatingCRLF = szGatheredTextBuffer.ReverseFind('\r');
				
				if (nSizeWithTerminatingCRLF != -1)
				{
					// The size we want to write out includes the CR
					nSizeWithTerminatingCRLF++;
					
					if ( ( nSizeWithTerminatingCRLF < szGatheredTextBuffer.GetLength() ) &&
						 (szGatheredTextBuffer[nSizeWithTerminatingCRLF] == '\n') )
					{
						// The size we want to write out includes the LF
						nSizeWithTerminatingCRLF++;
					}
					
					// Write out everything up to and including the last CRLF
					if ( FAILED(PutWithEscapedFileMarkers(szGatheredTextBuffer, nSizeWithTerminatingCRLF, bHtml)) )
					{
						size = -1;
						break;
					}
					
					// Keep anything after the last CRLF
					szGatheredTextBuffer = szGatheredTextBuffer.Right(szGatheredTextBuffer.GetLength() - nSizeWithTerminatingCRLF);
				}
			}
			// Else no reason to do buffer gathering - just write the line escaping file markers
			else if (FAILED(PutWithEscapedFileMarkers(buf, size, bHtml)))
			{
				size = -1;
				break;
			}
		}
	}

	// If we had any left over text left in our gathered text buffer
	// better write it out now
	if ( (size >= 0) && bSupportGatheredBuffer && !szGatheredTextBuffer.IsEmpty() )
	{
		if ( FAILED(PutWithEscapedFileMarkers(szGatheredTextBuffer, szGatheredTextBuffer.GetLength(), bHtml)) )
			size = -1;
	}

	if (size >= 0)
	{
		// write out rich text delimiter, if need be
	//	if (ms->m_hd->hasRich)

		// Always ensure a new line. We needed this anyway for any of the styled
		// text variants below.
		// We need the new line for plain text too to avoid sneaky attempts to
		// break up a spoofed Attachment Converted: line into two separate inline
		// text attachments. Without the new line, the broken up Attachment Converted:
		// becomes one whole spoofed Attachment Converted:. With the new line it's
		// just another failed spoof attempt.
		if ( FAILED(g_pPOPInboxFile->EnsureNewline()) ) 
			size = -1;

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
					char		szLogBuf[256];
					sprintf(szLogBuf, "LOGNULL TextReader::ReadIt() JJFileMT::Truncate(%d) 3", lHeaderEnd);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

					g_pPOPInboxFile->Truncate(lHeaderEnd);
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
					char		szLogBuf[256];
					sprintf(szLogBuf, "LOGNULL TextReader::ReadIt() JJFileMT::Truncate(%d) 4", lHeaderEnd);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

					g_pPOPInboxFile->Truncate(lHeaderEnd);
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
