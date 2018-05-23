// MAPICMC.CPP
//
// Routines for handling MAPI and CMC calls
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>

#include "eumapi.h"
#include "eudora.h"
#include "mainfrm.h"
#include "cursor.h"
#include "fileutil.h"
#include "guiutils.h"
#include "resource.h"
#include "summary.h"
#include "doc.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "pop.h"
#include "rs.h"
#include "address.h"
#include "tocdoc.h"
#include "sendmail.h"
#include "3dformv.h"	// for CTocView
#include "tocview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

void StripCrapFromAddress(CString& RawAddress)
{	
	if(RawAddress.Find('[') == -1)
		return;

	char * floater;
	char * locatebracket = NULL;
	char * ReturnString;
	char * PassedAddresses;

	PassedAddresses = new char[RawAddress.GetLength()+1];
	ReturnString = new char[RawAddress.GetLength()+1];
	ReturnString[0] = 0;

	strcpy(PassedAddresses, RawAddress);

	floater = strchr(PassedAddresses, '[');

	if (floater != PassedAddresses)
	{
		*floater = 0;
		strcat(ReturnString, PassedAddresses);
		*floater = '[';
	}

	if (strnicmp(floater, _T("[SMTP:"), 5)== 0)
	{
		floater += 6;
		locatebracket = floater;
		while (*floater == ' ')	// strip spaces from the beginning of the address
			floater++;
		locatebracket = strchr(floater, ']');
		if (locatebracket != NULL)
		{
			while (*locatebracket == ' ')	// strip spaces from the end of the address
				locatebracket--;

			*(locatebracket++) = 0;
		}

		strcat(ReturnString, _T("<"));
		strcat(ReturnString, floater);
		strcat(ReturnString, _T(">"));
		strcat(ReturnString, locatebracket);
	}
	else
	{
		strcat(ReturnString, PassedAddresses);
	}
	RawAddress.Empty();
	RawAddress = ReturnString;

	delete [] PassedAddresses;
	delete [] ReturnString;
}

////////////////////////////////////////////////////////////////////////
// ProcessMAPIData [static]
//
// Processes the raw string data passed across the WM_COPYDATA
// interface from the Eudora MAPI DLL.  The input consists of
// line-oriented keyword/value data and the output consists of
// properly decoded and categorized strings ready to send to Eudora's
// composition window.
//
////////////////////////////////////////////////////////////////////////
static BOOL ProcessMAPIData(
	const char* pszData,		//(i) ptr to string data buffer
	CString& strTo, 			//(o) returned TO: string
	CString& strSubject, 		//(o) returned SUBJECT: string
	CString& strCc, 			//(o) returned CC: string
	CString& strBcc, 			//(o) returned BCC: string
	CString& strAttachments, 	//(o) returned ATTACHMENTS: string
	CString& strBody,			//(o) returned message body string
	BOOL&    autoSend)			//(o) returned "auto send" flag
{
	ASSERT(pszData != NULL);
	ASSERT(strTo.IsEmpty());
	ASSERT(strSubject.IsEmpty());
	ASSERT(strCc.IsEmpty());
	ASSERT(strBcc.IsEmpty());
	ASSERT(strAttachments.IsEmpty());
	ASSERT(strBody.IsEmpty());

	//
	// For processing FILE/PATH pairs.
	//
	CString optional_filename;

	//
	// Start out by assuming no auto send.  This may be set TRUE below.
	//
	autoSend = FALSE;

	//
	// Pre-allocate the strBody buffer for performance reasons.
	//
	strBody.GetBufferSetLength(strlen(pszData));
	strBody.ReleaseBuffer();
	
	//
	// Each pass through this loop processes one "line" of data at a
	// time.  End when we hit an empty line or the end of the string.
	//
	BOOL status = TRUE;		// returned
	while (*pszData)
	{
		//
		// Extract current line from buffer, not including terminating
		// newline char.
		//
		CString current_line;
		{
			char* pszNewline = strchr(pszData, '\n');
			if (pszNewline)
			{
				*pszNewline = '\0';		// temporary NULL termination
				current_line = pszData;
				*pszNewline = '\n';		// undo temporary NULL termination
				pszData = pszNewline + 1;
			}
			else
			{
				ASSERT(0);				// malformed data buffer
				break;
			}
		}

		//
		// Check for the end of data condition -- a blank line.
		//
		if (current_line.GetLength() == 0)
		{
			ASSERT(strlen(pszData) == 0);
			break;
		}

		//
		// Process the current line, looking for a four letter keyword
		// followed by a colon and a space:
		//
		//		"XXXX: "
		//
		if (current_line.GetLength() < 6)
		{
			ErrorDialog(IDS_ERR_BAD_MAPI_DATA);
			status = FALSE;
			break;
		}

		//
		// Extract "keyword" portion of line, then remove the keyword
		// portion from the current line, leaving only the (possibly
		// empty) data.
		//
		CString keyword = current_line.Left(6);
		current_line = current_line.Right(current_line.GetLength() - 6);
		if ("SUBJ: " == keyword)
		{
			//
			// There should only be a single subject line.
			//
			ASSERT(strSubject.IsEmpty());
			strSubject = current_line;
		}
		else if ("BODY: " == keyword)
		{
			//
			// Handle multiple BODY lines by inserting newlines
			// between the lines.
			//
			if (strBody.IsEmpty())
			{
				//
				// Even though this is the first assignment, use the
				// concatenation operator (+=) instead of the
				// assignment operator (=) since the assignment
				// operator simply overwrites the internal CString
				// buffer.  For performance reasons, we want to reuse 
				// the existing, pre-allocated buffer.
				//
				strBody += current_line;
			}
			else
				strBody += "\n" + current_line;
		}
		else if ("TO__: " == keyword)
		{
			//
			// Handle multiple TO lines by inserting commas
			// between the recipients.
			//
			StripCrapFromAddress(current_line);
			if (strTo.IsEmpty())
				strTo = current_line;
			else
				strTo += ", " + current_line;
		}
		else if ("CC__: " == keyword)
		{
			//
			// Handle multiple CC lines by inserting commas
			// between the recipients.
			//
			if (strCc.IsEmpty())
				strCc = current_line;
			else
				strCc += ", " + current_line;
		}
		else if ("BCC_: " == keyword)
		{
			//
			// Handle multiple BCC lines by inserting commas
			// between the recipients.
			//
			if (strBcc.IsEmpty())
				strBcc = current_line;
			else
				strBcc += ", " + current_line;
		}
		else if ("FILE: " == keyword)
		{
			//
			// This optional FILE value is associated with the
			// next required PATH value in the data stream.
			// Therefore, we just save a copy of this value
			// off to the side to be used when the PATH value
			// is processed.
			//
			ASSERT(current_line.Find(';') == -1);
			optional_filename = current_line;
		}
		else if ("PATH: " == keyword)
		{
			//
			// The MAPI client can creates attachment files as
			// temporary files that get cleaned up as soon as this
			// routine returns.  Therefore, we need to make our own
			// copies of the temp files in the Eudora attachment
			// directory.
			//
			// IMPORTANT!  Note that a successful call to
			// CopyAttachmentFile() causes the 'current_line' string
			// to be updated to refer to the *copied* file pathname,
			// rather than the *source* file pathname.
			//
			ASSERT(current_line.Find(';') == -1);
			if (! CopyAttachmentFile(current_line, optional_filename))
			{
				ErrorDialog(IDS_ERR_BAD_MAPI_ATTACHMENTS, (const char *) current_line);
				status = FALSE;
				break;
			}

			//
			// Like with regular attachments, let's go with the Eudora
			// convention of ALWAYS appending a semi-colon at the end
			// of the attachment filename.
			//
			if (strAttachments.IsEmpty())
			{
				strAttachments = current_line;
				strAttachments += ";";
			}
			else
			{
				strAttachments += " ";
				strAttachments += current_line;
				strAttachments += ";";
			}

			//
			// Clear the filename.
			//
			optional_filename.Empty();
		}
		else if ("AUTO: " == keyword)
		{
			autoSend = TRUE;
		}
		else
		{
			ErrorDialog(IDS_ERR_BAD_MAPI_KEYWORD, (const char *) keyword);
			status = FALSE;
			break;
		}
	}

	return status;
}


////////////////////////////////////////////////////////////////////////
// HandleMAPIMessage [extern]
//
// Handler for processing the data forwarded to Eudora from the
// special Eudora MAPI DLL replacement.  This data contains all of the
// stuff Eudora needs to put up a reasonable composition window using
// data extracted from the raw MAPI data structures.
////////////////////////////////////////////////////////////////////////
LRESULT HandleMAPIMessage(PCOPYDATASTRUCT pcds)
{
	switch (pcds->dwData)
	{
	case EUM_CHECK_MAIL:
		ASSERT(pcds->lpData != NULL);
#ifdef FORNOW
		//
		// FORNOW, display raw message data sent across WM_COPYDATA
		// interface as a popup message.
		//
		::AfxMessageBox((const char *) pcds->lpData, MB_OK);
#endif // FORNOW

		//
		// MAPI client wants Eudora to do an automatic check mail.
		//
		ASSERT(strncmp((const char *) pcds->lpData, "CHEK: ", 6) == 0);
		//GetMail(kAutomaticMailCheckBits);
		((CEudoraApp *) AfxGetApp())->SetExternalCheckMail();
		((CEudoraApp *) AfxGetApp())->UpdateMailCheckLabel();
		break;
	case EUM_SEND_DOCUMENTS:
	case EUM_SEND_MAIL:
		{
			ASSERT(pcds->lpData != NULL);
#ifdef FORNOW
			//
			// FORNOW, display raw message data sent across WM_COPYDATA
			// interface as a popup message.
			//
			::AfxMessageBox((const char *) pcds->lpData, MB_OK);
#endif // FORNOW

			//
			// Declare the target strings.
			//
			CString str_to;
			CString str_subject;
			CString str_cc;
			CString str_bcc;
			CString str_attachments;
			CString str_body;
			BOOL auto_send = FALSE;		// auto send flag

			if (ProcessMAPIData((const char *) pcds->lpData,
								str_to, 
								str_subject, 
								str_cc, 
								str_bcc, 
								str_attachments, 
								str_body,
								auto_send))
			{
//FORNOW				//
//FORNOW				// Check for the special case of an empty message
//FORNOW				// body and a single TXT or HTM file attachment.
//FORNOW				//
//FORNOW				CString strInlineTextFile;
//FORNOW
//FORNOW				if (str_body.IsEmpty() && 
//FORNOW					!str_attachments.IsEmpty() &&
//FORNOW					GetIniShort(IDS_INI_INLINE_MAPI_TEXT_ATTACHMENT))
//FORNOW				{
//FORNOW					//
//FORNOW					// Check for a single TXT or HTM file attachment.
//FORNOW					// If found, place the filename in 'strInlineTextFile'.
//FORNOW					//
//FORNOW					int nSemicolon = str_attachments.Find(';');
//FORNOW					if (nSemicolon != -1)
//FORNOW					{
//FORNOW						if (nSemicolon == str_attachments.GetLength() - 1)
//FORNOW						{
//FORNOW							//
//FORNOW							// Found single attachment file, but is it
//FORNOW							// a TXT or HTM file?
//FORNOW							//
//FORNOW							strInlineTextFile = str_attachments.Left(nSemicolon);	// strip off trailing semicolon
//FORNOW
//FORNOW							int nDot = strInlineTextFile.ReverseFind('.');
//FORNOW							CString str_extension(strInlineTextFile.Right(strInlineTextFile.GetLength() - nDot - 1));
//FORNOW
//FORNOW							if (str_extension.CompareNoCase(CRString(IDS_TEXT_EXTENSION)) == 0 ||
//FORNOW								str_extension.CompareNoCase(CRString(IDS_HTM_EXTENSION)) == 0 ||
//FORNOW								str_extension.CompareNoCase(CRString(IDS_HTML_EXTENSION)) == 0)
//FORNOW							{
//FORNOW								//
//FORNOW								// Yes.  We found a single TXT or HTM
//FORNOW								// file attachment.  Instead of
//FORNOW								// attaching it as an external file,
//FORNOW								// let's setup to inline the contents
//FORNOW								// of the file into the message body
//FORNOW								// after the message object has been
//FORNOW								// created below (as opposed to
//FORNOW								// making it a regular attachment).
//FORNOW								//
//FORNOW								if (::FileExistsMT(strInlineTextFile))
//FORNOW								{
//FORNOW									str_attachments.Empty();
//FORNOW								}
//FORNOW								else
//FORNOW								{
//FORNOW									ASSERT(0);
//FORNOW									strInlineTextFile.Empty();
//FORNOW								}
//FORNOW							}
//FORNOW							else
//FORNOW								strInlineTextFile.Empty();
//FORNOW						}
//FORNOW					}
//FORNOW					else
//FORNOW					{
//FORNOW						ASSERT(0);		// huh? missing semi-colon
//FORNOW					}
//FORNOW				}

				//
				// Create Eudora composition window with as many fields
				// filled as possible, then display it.
				//
				CCompMessageDoc* p_comp = NewCompDocument(
					str_to.IsEmpty()		  ? NULL : ( const char* ) str_to, 			// To
					NULL,																// From (FORNOW)
					str_subject.IsEmpty()	  ? NULL : ( const char* ) str_subject,		// Subject
					str_cc.IsEmpty()		  ? NULL : ( const char* ) str_cc,			// Cc
					str_bcc.IsEmpty()		  ? NULL : ( const char* ) str_bcc,			// Bcc
					str_attachments.IsEmpty() ? NULL : ( const char* ) str_attachments,	// Attachments
					str_body.IsEmpty()		  ? NULL : ( const char* ) str_body);		// Body
				if (p_comp)
				{
					p_comp->m_Sum->SetFlag(MSF_MAPI_MESSAGE);

//FORNOW					if (! strInlineTextFile.IsEmpty())
//FORNOW					{
//FORNOW						JJFile textFile;
//FORNOW						if (SUCCEEDED(textFile.Open(strInlineTextFile, O_RDONLY)))
//FORNOW						{
//FORNOW							//
//FORNOW							// Allocate a buffer big enough to store
//FORNOW							// the entire file, then read the contents of
//FORNOW							// the file into the buffer.
//FORNOW							//
//FORNOW							CFileStatus fileStatus;
//FORNOW							CFile::GetStatus(strInlineTextFile, fileStatus);
//FORNOW							char* pszNewBody = new char [fileStatus.m_size + 1];
//FORNOW							memset(pszNewBody, 0, fileStatus.m_size + 1);
//FORNOW							if (pszNewBody && SUCCEEDED(textFile.Read(pszNewBody, fileStatus.m_size)))
//FORNOW							{
//FORNOW								//
//FORNOW								// Replace the m_Text buffer with the
//FORNOW								// new body text and optionally
//FORNOW								// dispose of the file attachment.
//FORNOW								// Yeah, this is technically a little
//FORNOW								// too early since we haven't sent the
//FORNOW								// message yet and we haven't yet
//FORNOW								// emptied it from the trash.  But
//FORNOW								// hey, it's our only shot, and
//FORNOW								// we're taking it.
//FORNOW								//
//FORNOW								if (p_comp->SetText(pszNewBody))
//FORNOW								{
//FORNOW									p_comp->m_QCMessage.Init(p_comp->CCompMessageDoc::m_MessageId, p_comp->GetText(), FALSE);
//FORNOW									if (! GetIniShort(IDS_INI_MAPI_DELETE_NEVER))
//FORNOW									{
//FORNOW										textFile.Close();
//FORNOW										textFile.Delete();
//FORNOW									}
//FORNOW								}
//FORNOW							}
//FORNOW
//FORNOW							if (pszNewBody)
//FORNOW								delete [] pszNewBody;
//FORNOW						}
//FORNOW					}

					//
					// Check for the special case of an empty message
					// body and a single TXT or HTM file attachment.
					//
					if (str_body.IsEmpty() && 
						!str_attachments.IsEmpty() &&
						GetIniShort(IDS_INI_INLINE_MAPI_TEXT_ATTACHMENT))
					{
						//
						// Check for a single TXT or HTM file attachment.
						//
						int nSemicolon = str_attachments.Find(';');
						if (nSemicolon != -1)
						{
							if (nSemicolon == str_attachments.GetLength() - 1)
							{
								//
								// Found single attachment file, but is it
								// a TXT or HTM file?
								//
								CString str_file(str_attachments.Left(nSemicolon));	// strip off trailing semicolon

								int nDot = str_file.ReverseFind('.');
								CString str_extension(str_file.Right(str_file.GetLength() - nDot - 1));

								if (str_extension.CompareNoCase(CRString(IDS_TEXT_EXTENSION)) == 0 ||
									str_extension.CompareNoCase(CRString(IDS_HTM_EXTENSION)) == 0 ||
									str_extension.CompareNoCase(CRString(IDS_HTML_EXTENSION)) == 0)
								{
									//
									// Yes.  We found a single TXT or HTM
									// file attachment.  Change the
									// Text-as-doc setting so that the
									// textual attachment file is sent
									// as an "inline" attachment.
									//
									p_comp->m_Sum->UnsetFlag(MSF_TEXT_AS_DOC);
								}
							}
						}
						else
						{
							ASSERT(0);		// huh? missing semi-colon
						}
					}

					if (auto_send)
					{
						if (GetIniShort(IDS_INI_WARN_MAPI_AUTO_SEND) &&
							WarnYesNoDialog(IDS_INI_WARN_MAPI_AUTO_SEND, IDS_WARN_MAPI_AUTO_SEND) == IDOK)
						{
							auto_send = FALSE;
						}
						//
						// We're in auto-send mode, so let's not bother
						// to open up the composition window or make
						// Eudora the foreground app.  Just queue the
						// message then initiate the send process, if
						// the user has "immediate send" enabled.
						// Note that we're NOT selectively sending only
						// the newly queued MAPI message ... we're sending
						// ALL queued messages.
						//
						else if ( p_comp->Queue(auto_send) == TRUE )
						{
							if (GetIniShort(IDS_INI_IMMEDIATE_SEND))
							{
								//
								// The following check for a NULL NetConnection
								// pointer is a hack to prevent a re-entrancy
								// problem observed when HotSyncing with a
								// Palm Pilot (see Laurence for more info).
								// That is, the Palm Pilot initiates a check mail, 
								// so there is a very high likelihood that Eudora 
								// is in the middle of checking mail at the time 
								// we get here and want to start sending a message 
								// via SMTP.  Since starting a second net connection
								// when there is an existing net connection
								// causes all hell to break loose, we prevent
								// that here by checking the global ::NetConnection.
								//
								// The side effect to the user, of course, is
								// that the Send Immediately option for outgoing
								// mail is randomly disabled for outgoing messages
								// that happen to get queued while a check mail
								// session is active.
								//
								if (NULL == ::NetConnection)
									SendQueuedMessages();
								else
								{
									//
									// There really should be an ASSERT(0)
									// here to indicate a re-entrancy problem,
									// but it would be too darn annoying.
									//
								}
							}
						}
						else
							auto_send = FALSE;
					}
					if (!auto_send)
					{
						//
						// Not auto-sending, so make sure Eudora gets
						// brought to the foreground, even if it is
						// minimized.  This gives a hint to the user
						// that they need to finish addressing the
						// message and then send it on its way.
						//
						CFrameWnd* p_mainframe = (CFrameWnd *) AfxGetMainWnd();
						ASSERT(p_mainframe != NULL);
						p_mainframe->ActivateFrame();
						SetForegroundWindow(p_mainframe->GetSafeHwnd());

						//
						// Display the outgoing message window.
						//
						p_comp->m_Sum->Display();
						p_comp->SetModifiedFlag();
					}
				}
			}
		}
		break;
	}

	return (0L);
}


////////////////////////////////////////////////////////////////////////
// WriteRecipientLines [static]
//
// Given a comma-separated list of recipients (recipientList), write
// the recipient data in 822 format to the given output file.
//
// Return TRUE if successful.
//
////////////////////////////////////////////////////////////////////////
BOOL WriteRecipientLines(JJFile& outFile, UINT keywordId, CString& recipientList)
{
	//
	// Write recipient line(s), one recipient per line.
	//
	ASSERT(! recipientList.IsEmpty());
	CString buf;
	BOOL first_time = TRUE;
	while (! recipientList.IsEmpty())
	{
		if (first_time)
		{
			buf.LoadString(keywordId);
			buf += " ";
			first_time = FALSE;
		}
		else
			buf = "        ";

		const char* p_end = ::FindAddressEnd(recipientList);
		if (p_end && *p_end == ',')
		{
			buf += recipientList.Left(p_end - ((const char *) recipientList));
			recipientList = recipientList.Right(recipientList.GetLength() - (p_end - ((const char *) recipientList)) - 1);
			recipientList.TrimLeft();
			if (! recipientList.IsEmpty())
				buf += ",";
		}
		else
		{
			buf += recipientList;
			recipientList.Empty();
		}
		
		if (FAILED(outFile.PutLine(buf)))
			return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SaveMAPIMessage [extern]
//
// Save a new message to the Eudora inbox to support the
// MAPISaveMail() function.  The message data is encoded into a
// simple, line-oriented textual format -- the same WM_COPYDATA format
// understood by HandleMAPIMessage() when processing MAPISendMail()
// and MAPISendDocuments() calls.
//
// If successful, returns a non-zero "message id" value for the newly
// added message.  A zero return value indicates an error.
//
// NOTE:  The Inbox update code below was lifted and adapted from
// the POP module.
//
////////////////////////////////////////////////////////////////////////
unsigned long SaveMAPIMessage(const CString& messageData)
{
#ifdef FORNOW
	//
	// FORNOW, display raw message data sent across WM_COPYDATA
	// interface as a popup message.
	//
	::AfxMessageBox(messageData, MB_OK);
#endif // FORNOW

	//
	// Declare the target strings.
	//
	CString str_to;
	CString str_subject;
	CString str_cc;
	CString str_bcc;
	CString str_attachments;
	CString str_body;
	BOOL auto_send = FALSE;		// auto send flag

	if (! ProcessMAPIData(messageData,
						str_to, 
						str_subject, 
						str_cc, 
						str_bcc, 
						str_attachments, 
						str_body,
						auto_send))
	{
		return 0;
	}

	//
	// Open the Eudora inbox file.
	//
	CTocDoc* p_inbox = ::GetInToc();
	if (NULL == p_inbox)
	{
		ASSERT(0);
		return 0;
	}
	JJFile inboxfile;
	if (FAILED(inboxfile.Open(p_inbox->MBFilename(), O_RDWR | O_APPEND | O_CREAT)))
		return 0;

	//
	// Save starting point of the write operation (i.e., the end of
	// the file).  We do this for two reasons: 1) the TOC rebuild
	// routine needs to know where the new stuff is so that it can
	// build a CSummary object, and 2) we need to truncate a partially
	// written message from the file if something bad happens.
	//
	long lStartPos = 0;
	inboxfile.Tell(&lStartPos);
    if (lStartPos < 0)
    	return 0;

    //
    // Write the special Eudora message separator to the file.
	//
	time_t now;
	time(&now);
	if (now < 0)
		now = 1;
	struct tm* p_tm_struct = localtime(&now);
	if (NULL == p_tm_struct)
		return 0;

	CRString weekdays(IDS_WEEKDAYS);
	CRString months(IDS_MONTHS);
	CString buf;
	buf.Format(CRString(IDS_FROM_FORMAT), 
				((const char *) weekdays) + p_tm_struct->tm_wday * 3,
				((const char *) months) + p_tm_struct->tm_mon * 3,
				p_tm_struct->tm_mday, 
				p_tm_struct->tm_hour,
				p_tm_struct->tm_min,
				p_tm_struct->tm_sec,
				p_tm_struct->tm_year + 1900);
	if (FAILED(inboxfile.PutLine(buf)))
		return 0;

	//
	// Write Date header line.  The general format is:
	//
	//      Date: Thu, 28 Mar 1996 15:31:26 -0800
	//
	if (! ::ComposeDate(buf.GetBuffer(80), now))
	{
		inboxfile.Truncate(lStartPos);
		return 0;
	}
	buf.ReleaseBuffer();
	if (FAILED(inboxfile.PutLine(buf)))
	{
		inboxfile.Truncate(lStartPos);
		return 0;
	}

	//
	// Write TO: line(s), one recipient per line.
	//
	if (! str_to.IsEmpty())
	{
		if (! ::WriteRecipientLines(inboxfile, IDS_HEADER_TO, str_to))
		{
			inboxfile.Truncate(lStartPos);
			return 0;
		}
	}

	//
	// Write FROM: line.
	//
	buf.LoadString(IDS_HEADER_FROM);
	buf += " ";
	buf += GetReturnAddress();
	if (FAILED(inboxfile.PutLine(buf)))
	{
		inboxfile.Truncate(lStartPos);
		return 0;
	}

	//
	// Write SUBJECT line.
	//
	if (! str_subject.IsEmpty())
	{
		ASSERT(str_subject.Find('\n') == -1);
		buf.LoadString(IDS_HEADER_TO);
		buf += " ";
		buf += str_subject;

		if (FAILED(inboxfile.PutLine(buf)))
		{
			inboxfile.Truncate(lStartPos);
			return 0;
		}
	}

	//
	// Write CC: line(s), one recipient per line.
	//
	if (! str_cc.IsEmpty())
	{
		if (! ::WriteRecipientLines(inboxfile, IDS_HEADER_CC, str_cc))
		{
			inboxfile.Truncate(lStartPos);
			return 0;
		}
	}

	//
	// Write BCC: line(s), one recipient per line.
	//
	if (! str_bcc.IsEmpty())
	{
		if (! ::WriteRecipientLines(inboxfile, IDS_HEADER_BCC, str_bcc))
		{
			inboxfile.Truncate(lStartPos);
			return 0;
		}
	}

	//
	// Write blank separator line.
	//
	if (FAILED(inboxfile.PutLine("")))
	{
		inboxfile.Truncate(lStartPos);
		return 0;
	}

	//
	// Write body text.
	//
	while (! str_body.IsEmpty())
	{
		int idx = str_body.Find('\n');
		if (idx != -1)
		{
			if ((idx > 0) && (str_body[idx - 1] == '\r'))
			{
				buf = str_body.Left(idx - 1);
				str_body = str_body.Right(str_body.GetLength() - idx - 2);
			}
			else
			{
				buf = str_body.Left(idx);
				str_body = str_body.Right(str_body.GetLength() - idx - 1);
			}
		}
		else
		{
			buf = str_body;
			str_body.Empty();
		}
		
		if (FAILED(inboxfile.PutLine(buf)))
		{
			inboxfile.Truncate(lStartPos);
			return 0;
		}
	}

	//
	// Write attachment lines.
	//
	BOOL has_attachment = FALSE;
	while (! str_attachments.IsEmpty())
	{
		buf.LoadString(IDS_ATTACH_CONVERTED);
		int idx = str_attachments.Find(';');
		if (idx != -1)
		{
			buf += str_attachments.Left(idx);
			str_attachments = str_attachments.Right(str_attachments.GetLength() - idx - 1);
			str_attachments.TrimLeft();
		}
		else
		{
			ASSERT(0);
			buf = str_attachments;
			str_attachments.Empty();
		}
		
		if (FAILED(inboxfile.PutLine(buf)))
		{
			inboxfile.Truncate(lStartPos);
			return 0;
		}
		has_attachment = TRUE;
	}

	//
	// Okay, if we get this far, we're done writing the MBX data.
	// Now, update the summary info.
	//
	inboxfile.Seek(lStartPos);
	CSummary* p_sum = p_inbox->UpdateSum(&inboxfile, TRUE);
	if (NULL == p_sum)
	{
		inboxfile.Truncate(lStartPos);
		return 0;
	}

	if (has_attachment)
		p_sum->SetFlag(MSF_HAS_ATTACHMENT);
	p_sum->SetFlag(MSF_MAPI_MESSAGE);

	if (! p_inbox->WriteSum(p_sum))
	{
		inboxfile.Truncate(lStartPos);
		return 0;
	}

	inboxfile.Close();

	//
	// UpdateSum() turns off redraws, so make sure redrawing is
	// re-enabled.
	//
	if (p_inbox->GetView())
		p_inbox->GetView()->m_SumListBox.SetRedraw(TRUE);

	ASSERT(p_sum->GetUniqueMessageId());
	return p_sum->GetUniqueMessageId();
}

