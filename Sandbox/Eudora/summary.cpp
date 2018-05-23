// SUMMARY.CPP
//
// Routines for message summaries

#include "stdafx.h"
#include <afxcmn.h>
#include <ctype.h>

#include <QCUtils.h>

#include "cursor.h"
#include "resource.h"
#include "rs.h"
#include "fileutil.h"
#include "utils.h"
#include "guiutils.h"
#include "address.h"
#include "summary.h"
#include "doc.h"
#include "tocview.h"
#include "eudora.h"
#include "msgutils.h"
#include "font.h"
#include "mainfrm.h"
#include "compmsgd.h"
#include "ReadMessageDoc.h"
#include "msgframe.h"
#include "sendmail.h"
#include "jjnet.h"
#include "progress.h"
#include "header.h"
#include "mime.h"

#include "pop.h"
#include "MsgRecord.h"
#include "persona.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCProtocol.h"
#include "TocFrame.h"
#include "AutoCompleteSearcher.h"

#include "trnslate.h"
#include "QCSharewareManager.h"

// IMAP4
#include "imapfol.h"
// END IMAP4

extern QCMailboxDirector	g_theMailboxDirector;

#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CSumList::CSumList(const CSumList& copy)
{
	POSITION pos = copy.GetHeadPosition();
	
	while (pos)
	{
		CSummary* Sum = copy.GetNext(pos);
		Add(Sum);
	}
}

CSumList::~CSumList()
{
	while (!IsEmpty())
		delete RemoveHead();
}

int CSumList::GetIndex(const CSummary* Sum) const
{
	POSITION pos = GetTailPosition();
	
	for (int i = GetCount() - 1; i >= 0 ; i--)
	{
		if (Sum == GetPrev(pos))
			return (i);
	}
	
	return (-1);
}

// Find
//
// Optimized over CObList::Find() in that it starts at the end and searches backward
// through the list.  The reason for this is that most messages that are being used
// tend to be at the end of the summary list.
//
POSITION CSumList::Find(CSummary* searchSum, POSITION startAfter/* = NULL*/) const
{
	if (startAfter != NULL)
		return CObList::Find(searchSum, startAfter);

// WARNING!!!!
// The rest of this code is stolen from CObList::Find(), and slightly changed.
// If MFC ever changes CObList::Find(), then this will need to change as well.

	ASSERT_VALID(this);

	CNode* pNode = m_pNodeTail;  // start at tail

	for (; pNode != NULL; pNode = pNode->pPrev)
		if (pNode->data == searchSum)
			return (POSITION) pNode;
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CSummary

IMPLEMENT_DYNAMIC(CSummary, CObject)

/////////////////////////////////////////////////////////////////////////////
// CSummary diagnostics

#ifdef _DEBUG
void CSummary::AssertValid() const
{
	CObject::AssertValid();
}

void CSummary::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	AFX_DUMP1(dc, "\n\tFrom = ", m_From);
	AFX_DUMP1(dc, "\n\tSubject = ", m_Subject);
	AFX_DUMP1(dc, "\n\tDate = ", m_Date);
}

#endif //_DEBUG


CSummary::CSummary() : CObject(),
	m_SavedPos(-1, -1, -1, -1)
{
	m_Offset = 0L;
	m_Length = 0L;
	m_State = MS_UNREAD;
	m_Label = 0;
	memset(m_From, 0, sizeof(m_From));
	memset(m_Subject, 0, sizeof(m_Subject));
	memset(m_Date, 0, sizeof(m_Date));
	m_Seconds = 0L;
	m_TimeZoneMinutes = 0;
	m_TheToc = NULL;
	m_FrameWnd = NULL;
	//m_Flags = ( unsigned short ) (GetIniShort(IDS_INI_SHOW_ALL_HEADERS)? MSF_SHOW_ALL_HEADERS : 0);
	m_Flags = 0;
	m_Priority = MSP_NORMAL;
	m_Hash = 0L;
	m_UniqueMessageId = 0L;
	m_ResponseSum = m_RespondingToSum = NULL;
	m_TransHdr.Empty();
	m_SigHdr.Empty();
	m_SigSelected.Empty();
	m_Precedence.Empty();
	m_FlagsEx = 0;

	SetPersona( g_Personalities.GetCurrent() );

#ifdef IMAP4
	m_Imflags = 0;
	m_MsgSize = 0;
	m_nUndownloadedAttachments = 0;
#endif

	m_rrDialogOverride = false;	// RR dialog override -jdboyd 8/23/99

}

CSummary::~CSummary()
{
	// Clean up pointers used to undo state changes when a comp is cancelled
	if (m_ResponseSum)
		m_ResponseSum->m_RespondingToSum = NULL;
	if (m_RespondingToSum)
		m_RespondingToSum->m_ResponseSum = NULL;

	if (m_TheToc)
	{
		//
		// Give the TocDoc a chance to cleanup its "previewable" Summary 
		// cache.  In turn, the TocDoc notifies its Frame that its
		// previewed message is being invalidated.
		//
		m_TheToc->InvalidatePreviewableSummary(this);
	}

	// Normally, a window shouldn't still exist for a summary when then
	// summary is being deleted.  But there's a weird case during shutdown
	// where this can happen.  So if the window still exists, we need to
	// reset to NULL the m_Sum member variable of the CMessageDoc object.
	CMessageDoc* Doc = NULL;
	if (Doc = FindMessageDoc())
	{
		// JB and SD say "go away, this is normal"  ASSERT(0);
		if (Doc->IsKindOf(RUNTIME_CLASS(CMessageDoc)))
			Doc->m_Sum = NULL;
	}
}

void CSummary::Copy(CSummary* NewInfo)
{
	m_Length = NewInfo->m_Length;
	m_State = NewInfo->m_State;
	m_Priority = NewInfo->m_Priority;
	m_Label = NewInfo->m_Label;
	m_Seconds = NewInfo->m_Seconds;
	m_TimeZoneMinutes = NewInfo->m_TimeZoneMinutes;
	m_SavedPos = NewInfo->m_SavedPos;
	memcpy(m_Date, NewInfo->m_Date, sizeof(m_Date));
	memcpy(m_From, NewInfo->m_From, sizeof(m_From));
	memcpy(m_Subject, NewInfo->m_Subject, sizeof(m_Subject));
	m_Flags = NewInfo->m_Flags;
	//Make sure Show-all-headers is turned off
	m_Flags &= ~MSF_SHOW_ALL_HEADERS;
	m_FlagsEx = NewInfo->m_FlagsEx;
	m_Hash = NewInfo->m_Hash;
	m_PersonaHash = NewInfo->m_PersonaHash;
	m_Persona = NewInfo->m_Persona;

#ifdef IMAP4
	m_Imflags = NewInfo->m_Imflags;
	m_MsgSize = NewInfo->m_MsgSize;
#endif
	//
	// Intentionally not copying the message id.  The assumption is
	// that we're populating a newly created summary object that has
	// not yet been added to any TocDoc.  The TocDoc should determine
	// how "virgin" summary objects get their message id.
	//
	ASSERT(0 == m_UniqueMessageId);
}

BOOL CSummary::Selected() const
{
	CTocView* TocView;
	
	if (m_TheToc && (TocView = m_TheToc->GetView()))
		return (TocView->Selected(this));
		
	return (FALSE);
}

int CSummary::Select(BOOL bSelect /*= TRUE*/)
{
	CTocView* TocView;
	
	if (m_TheToc && (TocView = m_TheToc->GetView()))
		return (TocView->Select(this, bSelect));
		
	return (LB_ERR);
}


int CSummary::GetSize()
{
	//if POP, get the length of the message in mbx excluding the size of attachments
	
	if( m_TheToc && m_TheToc->IsImapToc())
	{
		return m_MsgSize;
	}
	else
	{
		//POP stores size in bytes
		return (m_Length / 1024); //in KB
	}
}

const char* CSummary::GetFrom()
{
	if (*m_From)
		return (m_From);

	return (strcpy(m_From + 1,
		(m_State < MS_UNSENDABLE? CRString(IDS_NO_SENDER) : CRString(IDS_NO_RECIPIENT))));
}

////////////////////////////////////////////////////////////////////////
// GetPriority [public]
//
// Returns an ASCII string which corresponds to the current priority
// value.
////////////////////////////////////////////////////////////////////////
CString CSummary::GetPriority() const
{
	CString priority;

	switch (m_Priority)
	{
	case MSP_HIGHEST:
		priority.LoadString(IDS_PRIORITY_HIGHEST);
		break;
	case MSP_HIGH:
		priority.LoadString(IDS_PRIORITY_HIGH);
		break;
	case MSP_NORMAL:
		priority.LoadString(IDS_PRIORITY_NORMAL);
		break;
	case MSP_LOW:
		priority.LoadString(IDS_PRIORITY_LOW);
		break;
	case MSP_LOWEST:
		priority.LoadString(IDS_PRIORITY_LOWEST);
		break;
	default:
		ASSERT(0);
		break;
	}

	return priority;
}

const char* CSummary::GetSubject()
{
	if (*m_Subject)
		return (m_Subject);

	return (strcpy(m_Subject + 1, CRString(IDS_NO_SUBJECT)));
}

const char* CSummary::MakeTitle()
{
	m_Title.Format(IDS_SUM_TITLE_FORMAT, GetFrom(),
					(m_Date[0]? ", " : ""), m_Date, GetSubject());

	return (m_Title);
}

// FindMessageDoc
// Returns the exsiting message document associated with this summary, if one exists, otherwise NULL.
//
CMessageDoc* CSummary::FindMessageDoc()
{
	CMultiDocTemplate* MessageTemplate = (IsComp()? CompMessageTemplate : ReadMessageTemplate);
	CMessageDoc* doc = NULL;

	POSITION pos = MessageTemplate->GetFirstDocPosition();
	for (POSITION NextPos = pos; pos != NULL; pos = NextPos)
	{
		doc = (CMessageDoc*)MessageTemplate->GetNextDoc(NextPos);
		if (doc->m_Sum == this)
			break;
    }

	return (pos? doc : NULL);
}

// GetMessageDoc
// Gets the message document associated with this summary.  Will create one if necessary.
//
CMessageDoc* CSummary::GetMessageDoc()
{

	CMessageDoc* doc = FindMessageDoc();
	
	if (doc)
		return (doc);
		
	if (doc = (CMessageDoc*)NewChildDocument(IsComp()? CompMessageTemplate : ReadMessageTemplate))
		doc->m_Sum = this;

	return (doc);
}

// NukeMessageDocIfUnused
// Deletes CMessageDoc if it exists and isn't being viewed
//
BOOL CSummary::NukeMessageDocIfUnused()
{
	CMessageDoc* pMessageDoc = FindMessageDoc();
	if (pMessageDoc)
	{
		//Is it being previewed currently?
		if ( m_TheToc->GetPreviewableSummary() == this )
			return FALSE;

		//Is it being used by Read/Comp views?
		POSITION pos = pMessageDoc->GetFirstViewPosition();
		if (!pos)
		{
			pMessageDoc->OnCloseDocument();
			return TRUE;
		}
	}

	return FALSE;
}

// Display
// Displays a read or composition message, or brings it to the front if already open
//
BOOL CSummary::Display()
{
	if (m_FrameWnd)
	{
		//
		// Bring the existing frame window to the front.
		//
		ASSERT_VALID(m_FrameWnd);
		ASSERT_KINDOF(QCWorksheet, m_FrameWnd);
		if (m_FrameWnd->IsIconic())
			m_FrameWnd->MDIRestore();
		else
			m_FrameWnd->ActivateFrame();
		return TRUE;
	}

#ifdef IMAP4
	// If we're going to display this, if this is part of an IMAP toc,
	// attempt to download the message here before we dig ourselves any deeper.
	//
	if ( m_TheToc && m_TheToc->IsImapToc () )
	{
		if ( !SUCCEEDED (DownloadImapMessage (this)) )
			return FALSE;
	}
#endif // IMAP4

	//
	// If we get this far, we need to load up the doc and
	// create a new frame window for it.
	//
	CMultiDocTemplate* MessageTemplate = (IsComp()? CompMessageTemplate : ReadMessageTemplate);
	CMessageDoc* pMessageDoc = GetMessageDoc();
	if (! pMessageDoc)
		return FALSE;

	// Mark the message as read if it was unread
	if (m_State == MS_UNREAD)
		SetState(MS_READ);

#ifdef IMAP4 // IMAP4
			// Must also set the remote message status.
			// We should do this before we call CSummary::SetState() but this
			// is less disruptive of the code.
			// 
			if (m_TheToc && m_TheToc->IsImapToc())
			{
				m_TheToc->ImapSetMessageState (this, MS_READ);
			}
#endif // IMAP4
	
	return (NewChildFrame(MessageTemplate, pMessageDoc) != NULL);
}


BOOL CSummary::DisplayBelowTopMostMDIChild()
{
	//Find the top level window and save it
	CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	BOOL bIsMaximized = FALSE;
	CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);

	if(!pActiveMDIChild || (pActiveMDIChild && !pActiveMDIChild->IsWindowVisible()) )
	{
		Display();
		return TRUE;
	}


	CMDIChild::m_nCmdShow = SW_HIDE;
	Display();
	if(m_FrameWnd)
		m_FrameWnd->SetWindowPos(pActiveMDIChild, 0,0,0,0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );
	CMDIChild::m_nCmdShow = -1;

	return TRUE;
}




// StripAttachFromBody
// Strips the 'Attachment Converted' lines from the message
BOOL CSummary::StripAttachFromBody(char* message, UINT tag)
{
	if (!message)
	{
		ASSERT(0);
		return FALSE;
	}

	CRString szTag(tag);
	const char* InSpot = message;
	char* OutSpot = message;
	const char* pMatch = NULL;
	int SaveTextLen = 0;

	while (pMatch = strstr(InSpot, szTag))
	{
		const char* EndLine = strchr(pMatch, '\r');
		if (!EndLine)
			EndLine = pMatch + strlen(pMatch) - 1;
		else if (EndLine[1] == '\n')
			EndLine++;

		// Make sure it was only at the beginning of a line
		if (pMatch == InSpot || pMatch[-1] == '\n' || pMatch[-1] == '\r')
			SaveTextLen = pMatch - InSpot;
		else
		{
			//Could have been inside quoted text, usually a msg w/ attachment
			//replied to using Eudora 3.x or Light. 
			//Need to preserve it as part of quoted text.
			SaveTextLen = EndLine - InSpot;

			//Remember to grab the newline too
			if (*EndLine == '\n')
				SaveTextLen++;
		}

		if (InSpot == OutSpot)
		{
			// If first time, then nothing to copy because it's
			// already in the right spot in the buffer
			OutSpot += SaveTextLen;
		}
		else
		{
			strncpy(OutSpot, InSpot, SaveTextLen);
			OutSpot += SaveTextLen;
		}

		//*OutSpot = 0;
		
		InSpot = EndLine + 1;
	}

	if (OutSpot != message)
		strcpy(OutSpot, InSpot);

	return TRUE;
}

// ComposeMessage
// Creates the new composition message based on this summary and
// the type of response (reply, forward, redirect).
//
BOOL CSummary::ComposeMessage
(
	UINT MenuItemID, 
	const char* DefaultTo /*= NULL*/,
	BOOL AutoSend /*=FALSE*/, 
	const char* pszStationery /*= NULL*/,
	const char* pszPersona /*= NULL*/,
	const char* selectedText /*= NULL*/
)
{
    // ReplyAll vs. Reply if the shift key is down
    if ( (MenuItemID == ID_MESSAGE_REPLY) && ShiftDown() )
        MenuItemID = ID_MESSAGE_REPLY_ALL;

    CMessageDoc* doc = m_FrameWnd? (CMessageDoc*)m_FrameWnd->GetActiveDocument() : NULL;
    char* OldSubject = NULL;
    char* NewTo = NULL;
    char* NewFrom = NULL;
    char* NewSubject = NULL;
    char* NewCc = NULL;
    const char* theMessage = NULL;
    char* message = NULL;
    const char* mes2 = NULL;
    char* QuotedBody = NULL;
    CString TempMes;
    BOOL Success = FALSE;
    char ResponseType;
    CCursor cursor;
    char**      pSrcMessage = NULL;
    BOOL bIsHtml = FALSE;
        
    UINT IsRich = 0;
    long lBufLen;

	// Figure out the message state from the ID of the menu item
	switch (MenuItemID)
	{
	case ID_MESSAGE_REPLY:		ResponseType = MS_REPLIED;		break;
	case ID_MESSAGE_REPLY_ALL:	ResponseType = MS_REPLIED;		break;
	case ID_MESSAGE_FORWARD:	ResponseType = MS_FORWARDED;	break;
	case ID_MESSAGE_REDIRECT:	ResponseType = MS_REDIRECT;		break;
	default: ASSERT(FALSE);
	}

	if (IsComp() && (doc = GetMessageDoc()))
	{
		CCompMessageDoc* comp = (CCompMessageDoc*)doc;
		
		// Make sure it's read in
		comp->GetText();

		// Collect all the headers with stuff in it 
		for (int i = 0; i < MaxHeaders; i++)
		{
			const char* contents = comp->GetHeaderLine(i);
			while (isspace((int)(unsigned char)*contents))
				contents++;
			if (*contents)
				TempMes += CRString(IDS_HEADER_TO + i) + ' ' + contents + "\r\n";
		}

		// Add on the body
		TempMes += "\r\n";
		TempMes += comp->GetText();
		message = new char[TempMes.GetLength() + 1];
		strcpy(message, TempMes);
	}
	else
	{
		if (!(doc = GetMessageDoc()))
			return (FALSE);
		if (!doc->IsModified() || doc->SaveModified())
		{
			const char *replySelectionChars = GetIniString(IDS_INI_REPLY_SELECTION_CHARS);

			// If we were told not to reply to selection or
			// if it doesn't have tasty chars, pretend no selection
			if (selectedText && (!GetIniShort(IDS_INI_REPLY_SELECTION) || !strpbrk(selectedText, replySelectionChars)))
				selectedText = NULL;
		    
			// use selection if there is (a suitable) one
			theMessage = selectedText ? (char *)selectedText : doc->GetText();

			// Possible for theMessage to be NULL - e.g. IMAP msg not downloaded and user 
			// cancels login. Must escape.
			//
			if (!theMessage)
			{
				return FALSE;
			}

			message = new char[strlen(theMessage) + 1];
			strcpy(message, theMessage);
		}
		else 
			return (FALSE);

    }

	// we really want a copy of the full message so headers can be dealt with
	char *fullMes = doc->GetFullMessage();
	if (!fullMes)
	{
		delete [] message;
		return (FALSE);
	}

	// Show All Headers flag off may hide some important headers, so let's get the whole thing
	mes2 = message;

#ifdef TODO
	if (!IsComp() && !ShowAllHeaders())
	{
		if (((CReadMessageDoc*)doc)->Read(SAH_YES))
			mes2 = doc->GetText();
		else
			mes2 = message;
	}
#endif

	IsRich = IsFancy(fullMes);
	if (!IsFlowed() && !IsXRich() && IsRich != IS_FLOWED)
		IsRich = IS_ASCII;

	CString sAttach;
	if (ResponseType == MS_FORWARDED || ResponseType == MS_REDIRECT)
	{
		if ( IsComp() )
			sAttach = ((CCompMessageDoc*)doc)->GetHeaderLine(HEADER_ATTACHMENTS);
		else
		{
			doc->m_QCMessage.GetAttachments(sAttach);
			if (sAttach.IsEmpty() == FALSE && ';' != sAttach[sAttach.GetLength() - 1])
				sAttach += ';';
		}

#ifdef IMAP4
		// If we got attachments and this is an IMAP mbox summary, we may have to 
		// download attachments. Ask user and do the download, if necessary.
		//
		if ( m_TheToc && m_TheToc->IsImapToc() )
		{
			if (!m_TheToc->m_pImapMailbox)
			{
				ASSERT (0);
			}
			else
			{
				// If the user doesn't want to download the attachments, "sAttach"
				// is cleared!!
				m_TheToc->m_pImapMailbox->DownloadAttachmentsBeforeCompose (sAttach, ResponseType);
			}
		}
#endif  // IMAP4
	}

	StripAttachFromBody((char*)message, IDS_ATTACH_CONVERTED);

	if (ResponseType == MS_FORWARDED)
	{
		if (! ShowAllHeaders() )
			CReadMessageDoc::StripTabooHeaders( (char *)message );
	}

	CString csPersona;
	if (pszPersona)
	{
		//
		// Use caller-provided Persona.
		//
		csPersona = pszPersona;
	}
	else
	{
		//
		// No Persona provided, so we can either use the explicitly
		// set X-Persona header value, if any, or we can grab the
		// personality from the summary record.  The personality in
		// the summary record takes precedence if it is different than
		// the X-Persona value (i.e., the user explicitly requested a
		// persona change via filters or manually via
		// Edit:Change:Personality).
		//
		CString strSummaryPersona = GetPersona();

		// 
		// Grab the personality from the Persona X-Header, if any.
		//
		CString strXPersona;
		{
			char *pszPersonaHeader = HeaderContents(IDS_PERSONA_XHEADER, fullMes);
			if (pszPersonaHeader)
			{
				char *cp = pszPersonaHeader;

				if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
				{
					cp++;
					*(cp + strlen(cp) - 1) = '\0';
					strXPersona = cp;
				}
				delete [] pszPersonaHeader;
			}
		}
		if (strSummaryPersona != strXPersona)
			csPersona = strSummaryPersona;	// can be empty
		else
			csPersona = strXPersona;		// can be empty
	}

	OldSubject = IsComp() ? ::SafeStrdupMT(((CCompMessageDoc*)doc)->GetHeaderLine(HEADER_SUBJECT)) : HeaderContents(IDS_HEADER_SUBJECT, fullMes);
	if (ResponseType == MS_REPLIED || ResponseType == MS_FORWARDED)
	{
		if (ResponseType == MS_REPLIED && (!OldSubject || RemovePrefixMT("Re:", OldSubject) == OldSubject))
		{
			CRString ReplyPrefix(IDS_REPLY_PREFIX);
			NewSubject = new char[ReplyPrefix.GetLength() + ::SafeStrlenMT(OldSubject) + 1];
			strcpy(NewSubject, ReplyPrefix);
			if (OldSubject)
				strcat(NewSubject, OldSubject);
		}
		if (ResponseType == MS_FORWARDED && (!OldSubject || RemovePrefixMT("Fwd:", OldSubject) == OldSubject))
		{
			CRString ForwardPrefix(IDS_FORWARD_PREFIX);
			NewSubject = new char[ForwardPrefix.GetLength() + ::SafeStrlenMT(OldSubject) + 1];
			strcpy(NewSubject, ForwardPrefix);
			if (OldSubject)
				strcat(NewSubject, OldSubject);
		}
		NewTo = ::SafeStrdupMT(DefaultTo);
		if (ResponseType == MS_FORWARDED)
		{
			const char* MessageText = message;
			if (IsComp() && m_State == MS_SENT && m_Seconds)
			{
				char DateBuf[80];
				ComposeDate(DateBuf, m_Seconds, m_TimeZoneMinutes);
				if (*DateBuf)
				{
					lBufLen = ::SafeStrlenMT(DateBuf) + ::SafeStrlenMT(message)+ 3;
					char* NewBuf = new char[lBufLen+1];
					if (NewBuf)
					{

						*NewBuf = 0;
						strcat(NewBuf,DateBuf);
						strcat(NewBuf,"\r\n");
						if (message)
							strcat(NewBuf, message);
						MessageText = NewBuf;
					}
				}
			}
			
			QuotedBody = QuoteText(MessageText, TRUE, IsRich, "\r\n");
			if (MessageText != message)
				delete [] (char*)MessageText;
		}
		else
		{
			if (!NewTo)
			{
				NewTo = HeaderContents(IDS_HEADER_REPLY_TO, fullMes);
				if (!NewTo)
				{
					NewTo = HeaderContents(IDS_HEADER_FROM, fullMes);
					// And add it to the history list if the INI entry is on.
					if (NewTo && GetIniShort(IDS_INI_ADD_FROM_LINES_TO_HISTORY) != 0 && g_AutoCompleter)
						g_AutoCompleter->Add(NewTo);
				}
			}
				
			// Add in reply intro string
			CString Attribution;
			char szDateBuf[sizeof(m_Date)+32];  // Extra long (+32) for day of week that wasn't counted on when m_date was created.
			SetDateString(szDateBuf, sizeof(m_Date)+32, GetIniShort(IDS_INI_LOCAL_TIME_DISPLAY), TRUE);
			LPCSTR rglpsz[2];
			rglpsz[0] = szDateBuf;
			rglpsz[1] = m_From;
			if (ID_MESSAGE_REPLY_ALL == MenuItemID)
				AfxFormatStrings(Attribution, GetIniString(IDS_INI_REPLY_ALL_ATTRIBUTION), rglpsz, 2);
			else
				AfxFormatStrings(Attribution, GetIniString(IDS_INI_REPLY_ATTRIBUTION), rglpsz, 2);
			Attribution += "\r\n";

			// If we're replying to selected text, by god reply to selected text,
			// and don't try to suck away headers from the selected text  SD 2/19/99
			QuotedBody = QuoteText(selectedText ? mes2:FindBody(mes2), FALSE, IsRich, Attribution);

			if (ID_MESSAGE_REPLY_ALL == MenuItemID)
			{
				char* OldTo = HeaderContents(IDS_HEADER_TO, fullMes);
				NewCc = HeaderContents(IDS_HEADER_CC, fullMes);
				if (OldTo && *OldTo)
				{
					::TrimWhitespaceMT(OldTo);
					if (GetIniShort(IDS_INI_REPLY_TO_TO_CC))
					{
						// Original To: address(es) get moved to Cc:
						char* TmpCc = new char[::SafeStrlenMT(NewCc) + 2 + ::SafeStrlenMT(OldTo) + 1];
						if (TmpCc)
						{
							if (!NewCc || !*NewCc)
								strcpy(TmpCc, OldTo);
							else
								sprintf(TmpCc, "%s, %s", OldTo, NewCc);
							delete [] NewCc;
							NewCc = TmpCc;
						}
					}
					else
					{
						// Original To: address(es) stay in To:
						char* TmpTo = new char[::SafeStrlenMT(NewTo) + 2 + ::SafeStrlenMT(OldTo) + 1];
						if (TmpTo)
						{
							if (!NewTo && !*NewTo)
								strcpy(TmpTo, OldTo);
							else
								sprintf(TmpTo, "%s, %s", NewTo, OldTo);
							delete [] NewTo;
							NewTo = TmpTo;
						}
					}
				}
				delete [] OldTo;
			}
	//		ConvertAttachmentLine(QuotedBody);
		}
	}
	else if (ResponseType == MS_REDIRECT)
		NewTo = ::SafeStrdupMT(DefaultTo);

	const char* Sub = NewSubject? NewSubject : OldSubject;
	const char* Body = QuotedBody? QuotedBody : FindBody(message);
	::TrimWhitespaceMT(NewTo);
	::TrimWhitespaceMT(NewCc);

	// we now know all the defaults - create the message
	CCompMessageDoc* comp = NewCompDocument(NewTo, NewFrom, Sub, NewCc, NULL, sAttach, Body, 
											pszStationery, csPersona, ResponseType);
	if (comp)
	{
		// we now know the Personality for this message...
		// switch to it so the stuff that follows is Personality specific
		CString	homie =	g_Personalities.GetCurrent();
		CString Persona = comp->m_Sum->GetPersona();
		if ( ! Persona.IsEmpty() )
			g_Personalities.SetCurrent( Persona );

		if (sAttach.IsEmpty())
			comp->m_Sum->UnsetFlag(MSF_HAS_ATTACHMENT);
		else
			comp->m_Sum->SetFlag(MSF_HAS_ATTACHMENT);

		if (!m_Precedence.IsEmpty())
			comp->m_Sum->SetPrecedence(m_Precedence);

		switch (IsRich)
		{
		case IS_FLOWED:
			comp->m_Sum->SetFlagEx(MSFEX_FLOWED);
			break;
		case IS_HTML:
			comp->m_Sum->SetFlagEx(MSFEX_HTML);
			// fall through
		case IS_RICH:
			comp->m_Sum->SetFlag(MSF_XRICH);
			break;
		}
		
		if (GetIniShort(IDS_INI_COPY_PRIORITY) || ResponseType == MS_REDIRECT)
		{
			char* OldPrior = HeaderContents(IDS_HEADER_PRIORITY_MATCH, fullMes);
			if (OldPrior)
			{
				comp->m_Sum->GleanPriority(OldPrior);
				delete [] OldPrior;
			}
		}

		// fix up the To: and Cc: lines
		if ( (ResponseType != MS_FORWARDED) && (ID_MESSAGE_REPLY_ALL == MenuItemID) )
		{
			const BOOL bStripAll = !GetIniShort(IDS_INI_INCLUDE_SELF);

			if (bStripAll || GetIniShort(IDS_INI_STRIP_DUPLICATE_ADDRESSES))
			{
				BOOL bFoundOne;

				CString NewTo = comp->m_Headers[ HEADER_TO ];
				bFoundOne = StripMe( NewTo.GetBuffer(0), bStripAll );
				comp->m_Headers[ HEADER_TO ] = NewTo;

				CString NewCc = comp->m_Headers[ HEADER_CC ];
				StripMe( NewCc.GetBuffer(0), bStripAll || bFoundOne );
				comp->m_Headers[ HEADER_CC ] = NewCc;
			}
		}

		// fix up the From: line
		if (ResponseType == MS_REDIRECT)
		{
			char* oldFrom = HeaderContents(IDS_HEADER_FROM, fullMes);
			comp->SetupRedirect(oldFrom);
			delete [] oldFrom;
		}

		// Reply threading stuff
		if (ResponseType == MS_REPLIED)
		{
			// get the pertinent headers from the current message
			char* OldReferences = HeaderContents(IDS_HEADER_REFERENCES, fullMes);
			char* OldInReplyTo = HeaderContents(IDS_HEADER_IN_REPLY_TO, fullMes);
			char* OldMessageId = HeaderContents(IDS_HEADER_MESSAGE_ID, fullMes);

			// generate new headers and add 'em to 
			CString NewInReplyTo = OldMessageId;
			CString NewReferences = OldInReplyTo;

			// guard against bogus InReplyTo headers
			if ( OldInReplyTo && OldInReplyTo[ 0 ] != '\0' )
			{
				if ( OldInReplyTo[ 0 ] != '<' )
				{
					NewReferences = "<";
					NewReferences += OldInReplyTo;
				}
				if ( OldInReplyTo[ strlen( OldInReplyTo ) - 1 ] != '>' )
				{
					NewReferences += ">";
				}
			}

			if ( OldReferences && OldReferences[ 0 ] )
			{
				// append the old references info
				int length = NewReferences.GetLength();
				if ( length )
					NewReferences += " ";	// add some white space

				NewReferences += OldReferences;
			}

			comp->m_Headers[ HEADER_IN_REPLY_TO ] = NewInReplyTo;
			comp->m_Headers[ HEADER_REFERENCES ] = NewReferences;

			delete [] OldMessageId;
			delete [] OldInReplyTo;
			delete [] OldReferences;
		}

		// Shareware: Only allow FCC in FULL FEATURE version.
		if (UsingFullFeatureSet())
		{
			bool bNonSystemToc = false;
			if (m_TheToc->IsImapToc())
			{
				if( IsInbox( m_TheToc->m_pImapMailbox->GetImapName()) == false)
					bNonSystemToc = true;
			}
			else
			{
				//pop
				if(m_TheToc->m_Type == MBT_REGULAR)
					bNonSystemToc = true;
			}

			if (ResponseType == MS_REPLIED && GetIniShort(IDS_INI_REPLY_AUTOFCC) && bNonSystemToc)
							//m_TheToc->m_Type == MBT_REGULAR)
			{	//AutoFCC implementation.
				QCMailboxCommand*	pCommand = g_theMailboxDirector.FindByPathname( m_TheToc->MBFilename() );
				if( pCommand != NULL )
				{
					comp->InsertFCCInBCC( pCommand );
				}
			}
		}

		// Check to see if we need to get rid of the signature
		if ((ResponseType == MS_REPLIED && !GetIniShort(IDS_INI_INCLUDE_SIG_REPLY)) ||
			(ResponseType == MS_FORWARDED && !GetIniShort(IDS_INI_INCLUDE_SIG_FORWARD)) ||
			(ResponseType == MS_REDIRECT && !GetIniShort(IDS_INI_INCLUDE_SIG_REDIRECT)))
		{
			comp->m_Sum->UnsetFlag(MSF_USE_SIGNATURE);
			comp->m_Sum->UnsetFlag(MSF_ALT_SIGNATURE);
		}

		BOOL Turbo = FALSE;
		if (ResponseType == MS_REDIRECT)
		{
			// Do turbo redirect if there's something in the To: header and
			// either exclusively the turbo redirect by default setting is on
			// or the shift key is down
			BOOL TR = GetIniShort(IDS_INI_TURBO_REDIRECT) != 0;
			Turbo = NewTo && *NewTo && (TR != ShiftDown());
		}

		// Change status of non-comp messages, saving a pointer to the orginal message
		// and original state so that it can be undone if the response is cancelled
		if (!IsComp())
		{
			// If we already have a response to this message, make sure the original
			// response doesn't get left with a link back to this message because
			// otherwise Bad Things (TM) will happen if you delete this message first
			// and then try to delete the original response
			if (m_ResponseSum)
				m_ResponseSum->m_RespondingToSum = NULL;
			m_ResponseSum = comp->m_Sum;
			comp->m_Sum->m_RespondingToSum = this;
			comp->m_Sum->m_RespondingToOldState = m_State;
			SetState(ResponseType);

#ifdef IMAP4 // IMAP4
			// Set the remote message's state also.
			if (m_TheToc && m_TheToc->IsImapToc())
			{
				m_TheToc->ImapSetMessageState (this, ResponseType);
			}
#endif // END IMAP4

		}

		if( !AutoSend && !Turbo )
		{
			comp->m_Sum->Display();
		}
		else
		{
			if( IsRich == IS_HTML )
			{
				// show the view as hidden -- this forces html to
				// get cleaned up by going through the save code
				CMDIChild::m_nCmdShow = SW_HIDE;
				comp->m_Sum->Display();
			}
			
			// save & queue the message
			comp->Queue( TRUE );
			CMDIChild::m_nCmdShow = -1;
		}
		
		if (m_TheToc)
		{
			// Delete message if we're doing turbo redirect, but only if it's not already in the Trash
			if (Turbo && m_TheToc->m_Type != MBT_TRASH)
				m_TheToc->Xfer(GetTrashToc(), this);
		
			m_TheToc->SetModifiedFlag();
		}
	
		Success = TRUE;

		// leave the Personality as we found it
		g_Personalities.SetCurrent( homie );
	}

	delete [] OldSubject;
	delete [] NewTo;
	delete [] NewFrom;
	delete [] NewSubject;
	delete [] NewCc;
	delete [] fullMes;
	delete [] QuotedBody;
	delete [] message;
	
	if( pSrcMessage )
	{
		delete [] *pSrcMessage;
	}

	return (Success);
}

CCompMessageDoc* CSummary::SendAgain
(
	BOOL DisplayIt /*= TRUE*/,
	const char* pszPersona /*= NULL*/
)
{
	CMessageDoc* doc = GetMessageDoc();
	CCompMessageDoc* NewDoc = NULL;
	char* Start;
	char* End;
	
	if (!doc)
		return (NULL);

	CString csPersona;
	if (pszPersona)
	{
		//
		// Use the caller-provided personality.
		//
		csPersona = pszPersona;
	}
	else
	{
		//
		// Caller didn't provide a persona, so grab it from the 
		// X-Persona header of the original message's personality.
		// We really want a copy of the full message so headers can 
		// be dealt with.
		//
		char *fullMes = doc->GetFullMessage();
		if ( fullMes )
		{
			// get the Persona X-Header
			char *PersonaHeader = HeaderContents(IDS_PERSONA_XHEADER, fullMes);
			if (PersonaHeader)
			{
				char *cp = PersonaHeader;

				if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
				{
					cp++;
					*(cp + strlen(cp) - 1) = '\0';
					csPersona = cp;
				}
				delete [] PersonaHeader;
			}

			delete [] fullMes;
		}
	}

	if (IsComp())
	{
		CCompMessageDoc* CompDoc = (CCompMessageDoc*)doc;

		// Need to make sure the message is read in if it doesn't have a window
		if (!m_FrameWnd)
			doc->GetText();

		CString theStuff;
		CompDoc->m_QCMessage.GetEmbeddedObjectHeaders( theStuff );
		
		NewDoc = NewCompDocument(CompDoc->GetHeaderLine(HEADER_TO), NULL,
			CompDoc->GetHeaderLine(HEADER_SUBJECT), CompDoc->GetHeaderLine(HEADER_CC),
			CompDoc->GetHeaderLine(HEADER_BCC), CompDoc->GetHeaderLine(HEADER_ATTACHMENTS),
			CompDoc->GetText(), NULL, csPersona, 0, theStuff);
		if (NewDoc)
		{
			NewDoc->m_Sum->SetPriority(m_Priority);
			NewDoc->m_Sum->SetTranslators(GetTranslators(), TRUE);  
			NewDoc->m_Sum->m_SigSelected = m_SigSelected; 
		}
	}
	else
	{
		BOOL gotReject = FALSE;
		const char* message;
		const char* mes2;

		message = doc->GetText();

		if (!(mes2 = FindBody(message)))
		{
			mes2 = message;
		}
		else
		{
			// search for the first Recieved: line
			Start = strstr( mes2, CRString( IDS_NEWLINE_RECEIVED ) );
			
			if( Start != NULL ) 
			{
				// found it
				mes2 = Start + 2;
				gotReject = TRUE;			
			}
			else
			{
				// If this is a mailer-daemon reject, then grab the orginal message
				char RejectStrings[128];
				GetIniString(IDS_REJECT_MESSAGE_STR, RejectStrings, sizeof(RejectStrings));
				Start = RejectStrings;
				
				for (; *Start; Start = End)
				{
					End = strchr(Start, '\n');
					if (End)
						*End++ = 0;
					else
						End = Start + strlen(Start);
					if (Start = strstr(mes2, Start))
					{
						if (Start = strchr(Start, '\n'))
							mes2 = Start + 1;
						gotReject = TRUE;
						break;
					}
				}
			}
		}

		// Reset message back to begining so it'll get all info if
		// its not a reject
		if (!gotReject)
			mes2 = message;

		char* To		= HeaderContents(IDS_HEADER_TO, mes2);
		char* Subject	= HeaderContents(IDS_HEADER_SUBJECT, mes2);
		char* Cc		= HeaderContents(IDS_HEADER_CC, mes2);
		char* Bcc		= HeaderContents(IDS_HEADER_BCC, mes2);
		char* OldPrior	= HeaderContents(IDS_HEADER_PRIORITY_MATCH, mes2);
		char* Attach = NULL;
		if ( IsComp() )
			Attach	= HeaderContents(IDS_HEADER_ATTACHMENTS, mes2);
		else
		{
			CString szAttach;
			doc->m_QCMessage.GetAttachments(szAttach);
			Attach = new char[szAttach.GetLength() + 1];
			strcpy(Attach, szAttach);
		}
	
		//Strip the attach converted lines if its a read message
		char* TempMsg = NULL;
		if (Attach && !IsComp() )
		{
			TempMsg = new char[strlen(mes2) + 1];
			strcpy(TempMsg, mes2);
			StripAttachFromBody(TempMsg, IDS_ATTACH_CONVERTED);
			mes2 = TempMsg;
		}

		NewDoc = NewCompDocument(To, NULL, Subject, Cc, Bcc, Attach, FindBody(mes2), NULL, csPersona);

		if (NewDoc)
		{
			// Grab old priority any way we can
			if (OldPrior)
				NewDoc->m_Sum->GleanPriority(OldPrior);
			else if (IsComp())
				NewDoc->m_Sum->SetPriority(m_Priority);
		}

		delete [] To;
		delete [] Subject;
		delete [] Cc;
		delete [] Bcc;
		delete [] Attach;
		delete [] OldPrior;
		delete [] TempMsg;
	}

    if (NewDoc)
	{
		// If the original message was a composition, then copy all the flags because they'll
		// have info like the encoding type, signature, and all the switches
		if (IsComp()) {
			NewDoc->m_Sum->CopyFlags(this);
			NewDoc->m_Sum->CopyFlagsEx(this);
		}
			
		if (DisplayIt)
			NewDoc->m_Sum->Display();
	}

	return (NewDoc);
}

//------------------------------------------------------------


BOOL IsHeader(const char* Line)
{
	for (const char* s = Line; *s && *s != ' ' && *s != '\t'; s++)
	{
		if (*s == ':')
			return (TRUE);
	}

	return (FALSE);
}

BOOL Weekday(const char* Line)
{
	ASSERT(Line);
	
	CRString Weekdays(IDS_WEEKDAYS);
	char day[4];

	strncpy(day, Line, 3);
	day[3] = '\0';
	
	return (Weekdays.Find(day) < 0? FALSE : TRUE);
}

// GetTime
//
// If FromLine is TRUE and Line is not a valid Unix-style From line, then
// return 0L.  Otherwise, return the time given in the line, if parsable. 
// Here's a sample From line:
//   From beckley@qualcomm.com Thu Oct 15 16:15:08 1992 
//
long GetTime(const char* Line, BOOL FromLine)
{
	// alternate from line
	const char* AltFromScan1 = "%3s %d %d:%d %s %d";
	const char* AltFromScan2 = "%3s %d %d:%d:%d %s %d";
	char timeZone[8];

	struct tm time;
	char month[5];
	int mon;
	int fraction;

	if (!Line) return (0L);

	time.tm_year = time.tm_hour = time.tm_min = time.tm_sec = 0;
	if (FromLine)
	{
		// Check for "From "
		if (Line[0] != 'F' || Line[1] != 'r' || Line[2] != 'o' ||
			Line[3] != 'm' || Line[4] != ' ')
		{
			return (0L);
		}
		Line += 5;

		// Skip address
		while (*Line && (*Line == ' ' || *Line == '\t'))
			Line++;
		while (*Line && *Line != ' ' && *Line != '\t')
			Line++;
		while (*Line && (*Line == ' ' || *Line == '\t'))
			Line++;

		// Verify weekday
		if (!Weekday(Line))
			return (0L);

		// Skip the weekday
		while (*Line && *Line != ' ' && *Line != '\t')
			Line++;
		while (*Line && (*Line == ' ' || *Line == '\t'))
			Line++;

		if (!*Line)
			return (0L);

		// Read in rest of date
		if (sscanf(Line, CRString(IDS_SCAN_DATE_1), month, &time.tm_mday,
			&time.tm_hour, &time.tm_min, &time.tm_sec, &time.tm_year) != 6)
		{
			if (sscanf(Line, AltFromScan1, month, &time.tm_mday, &time.tm_hour,
				&time.tm_min, timeZone, &time.tm_year) != 6)
			{
				sscanf(Line, AltFromScan2, month, &time.tm_mday, &time.tm_hour,
					&time.tm_min, &time.tm_sec, timeZone, &time.tm_year);
			}
		}
	}
	else
	{
		while (*Line && !isdigit((int)(unsigned char)*Line))
			Line++;
		if (!*Line)
			return (0L);
		if ((sscanf(Line, CRString(IDS_SCAN_DATE_2), &time.tm_mday, month,
			&time.tm_year, &time.tm_hour, &time.tm_min, &time.tm_sec) < 5) &&
			(sscanf(Line, CRString(IDS_SCAN_DATE_3), &time.tm_mday, month,
			&time.tm_year, &time.tm_hour, &time.tm_min, &time.tm_sec, &fraction) < 7))
		{
			return (0L);
		}
	}

	// Find corresponding month number
	CRString Months(IDS_MONTHS);
	LPCTSTR MonthIndex = Months;
	mon = -1;
	for (int i = 0; *MonthIndex; MonthIndex += 3, i++)
	{
		if (strnicmp(month, MonthIndex, 3) == 0)
		{
			mon = i;
			break;
		}
	}
	if (mon < 0)
		return (0L);
	time.tm_mon = mon;

	// Get year as number of years past 1900
	if (time.tm_year < 70)
		time.tm_year += 100;
	else if (time.tm_year > 1900)
	{
		time.tm_year -= 1900;
		if (time.tm_year < 70)
			time.tm_year = 70;
	}

	time.tm_isdst = -1;
	time_t seconds = mktime(&time);

	// Ok, we've got this far.  This is probably a "From " line of some
	// variety, but the rest of it may not match up.  So if we get a bogus
	// time, still consider it a "From " line, but we don't know exactly what
	// date/time it represents, so just return 1.

	return (seconds < 0L? (FromLine? 1L : 0L) : seconds);
}

void CSummary::GetStatus(const char *Line)
{
	for (const char* l = Line; *l; l++)
	{
		if (toupper(*l) == 'R')
    	{
			m_State = MS_READ;
			break;
		}
	}
}

static char Buffer[512];
long CSummary::m_lLastTime = 0L;
long CSummary::m_lBegin = 0L;

int CSummary::Build(JJFile* in, BOOL Rebuild)
{
	long lNumBytesRead = 0;		// -1 means error, 0 means EOF
	int IsOut = (m_TheToc->m_Type == MBT_OUT);
	int IsIn  = (m_TheToc->m_Type == MBT_IN);

	// Some strings that will be used in this routine	
	CRString AttachConvert(IDS_ATTACH_CONVERTED);
	int ACLen = AttachConvert.GetLength();
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

#ifdef IMAP4
	// IMAP UID is written to an "X-UID" header field when the MBX file is generated.
	CRString HeaderImapUid(IDS_IMAP_UID_XHEADER);
	CRString HeaderImapFlags(IDS_IMAP_IMFLAGS_XHEADER);
#endif

	if (!CSummary::m_lLastTime)
	{
		if (FAILED(in->GetLine(Buffer, sizeof(Buffer), &lNumBytesRead)) || (0 == lNumBytesRead))
			return int(lNumBytesRead);
		m_Seconds = GetTime(Buffer, TRUE);
	}
	else
		m_Seconds = CSummary::m_lLastTime;

	if (IsOut)
		SetState(MS_UNSENDABLE);
	else if (IsIn && Rebuild)
		SetState(MS_UNREAD);
	else if (Rebuild)
		SetState(MS_READ);

	m_Offset = CSummary::m_lBegin;

	BOOL InHeader = TRUE;
	while (SUCCEEDED(in->GetLine(Buffer, sizeof(Buffer), &lNumBytesRead)) && (lNumBytesRead > 0))
	{
		long time = IsFromLine(Buffer);
		if (time)
		{
			CSummary::m_lLastTime = time;
			lNumBytesRead = 1;
			if (!*m_Date)
				FormatDate();
			m_Length = CSummary::m_lBegin - m_Offset;

			// For IMAP:
			if (m_MsgSize <= 1)
				m_MsgSize = (unsigned short) max( (m_Length/1024 + 1), 1 );

			return (1);
		}

		if (InHeader && *Buffer)
		{
			if (!*m_Date && !strnicmp(Buffer, HeaderDate, 5))
			{
				m_Seconds = GetTime(Buffer + 5, FALSE);
				FormatDate(Buffer);
			}
			else if (!IsOut && !strnicmp(Buffer, HeaderFrom, 5))
				SetFrom(GetRealName(Buffer + 5));
			else if (IsOut && !*m_From &&
				(!strnicmp(Buffer, HeaderTo, 3) ||
				!strnicmp(Buffer, HeaderCc, 3) ||
				!strnicmp(Buffer, HeaderBcc, 4)))
			{
				SetFrom(GetRealName(strchr(Buffer, ':') + 1));
				if (*m_From && !Rebuild)
					SetState(MS_SENDABLE);
			}
			else if (!*m_Subject && !strnicmp(Buffer, HeaderSubject, 8))
			{
				::TrimWhitespaceMT(Buffer + 8);
				SetSubject(Buffer + 8);
			}
			else if (!strnicmp(Buffer, HeaderStatus, 7))
				GetStatus(Buffer + 7);
			else if (!IsOut && !*m_From && !strnicmp(Buffer, HeaderReplyTo, 9))
				SetFrom(GetRealName(Buffer + 9));
			else if (!strnicmp(Buffer, HeaderPriority, 7))
				GleanPriority(Buffer + 11);
//			else if (!strnicmp(Buffer, HeaderAttachments, 14) ||
//				!strnicmp(Buffer, ((const char*)HeaderAttachments) + 2, 12))
//			{
//				if ((strlen(Buffer)) > 16)
//					SetFlag(MSF_HAS_ATTACHMENT);
//			}
			else if (!strnicmp(Buffer, HeaderContentType, 13))
			{
				if (strstr(Buffer, CRString(IDS_MIME_ENRICHED)))
					SetFlag(MSF_XRICH);
				if (strstr(Buffer, CRString(IDS_MIME_HTML)))
				{
					SetFlag(MSF_XRICH);
					SetFlagEx(MSFEX_HTML);
				}
			}
    		else if ( !strnicmp(Buffer, HeaderTrans, 15) )
				SetTranslators(Buffer + 15);
    		else if ( !strnicmp(Buffer, HeaderSig, 20) )
			{
				char *cp = (Buffer + 20);

				if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
				{
					cp++;
					*(cp + strlen(cp) - 1) = '\0';
					m_SigHdr = m_SigSelected = cp;
				}
			}
#ifdef IMAP4	// "X-UID:" header. Note: string includes the colon.
			else if ( !strnicmp(Buffer, HeaderImapUid, 6) )
			{
				CString suid = Buffer + 6;
				suid.TrimLeft();
				suid.TrimRight();
				m_Hash = (unsigned long)atol (suid);
			}
			// X-IMFLAGS: RAFDF for \Read, \Answered, \Flagged, \Deleted and \Draft resp.
    		else if ( !strnicmp(Buffer, HeaderImapFlags, 9) )
			{
				unsigned long ImapParseImflags (LPCSTR pImflags);
				CString Imflags = Buffer + 9;
				Imflags.TrimLeft();
				Imflags.TrimRight();
				unsigned long m_Imflags = ImapParseImflags (Imflags);
			}
#endif // IMAP4
		}
		else
		{
			if (Rebuild)
			{
				if (InHeader)
				{
					if (!m_SigHdr.IsEmpty())
					{
						SetFlag(MSF_ALT_SIGNATURE);
					}
					else if (GetIniShort(IDS_INI_USE_SIGNATURE))
					{
						SetFlag(MSF_USE_SIGNATURE);
					}
					if (GetIniShort(IDS_INI_SEND_MIME))		SetFlag(MSF_MIME);
					if (GetIniShort(IDS_INI_SEND_UUENCODE))	SetFlag(MSF_UUENCODE);
					if (GetIniShort(IDS_INI_USE_QP))		SetFlag(MSF_QUOTED_PRINTABLE);
					if (GetIniShort(IDS_INI_WORD_WRAP))		SetFlag(MSF_WORD_WRAP);
					if (GetIniShort(IDS_INI_TABS_IN_BODY))	SetFlag(MSF_TABS_IN_BODY);
					if (GetIniShort(IDS_INI_KEEP_COPIES))	SetFlag(MSF_KEEP_COPIES);
					if (GetIniShort(IDS_INI_TEXT_AS_DOC))	SetFlag(MSF_TEXT_AS_DOC);
				}
				else if (!strnicmp(Buffer, AttachConvert, ACLen))
					SetFlag(MSF_HAS_ATTACHMENT);
			}

			if (!IsHTML())
			{
				switch (::IsFancy(Buffer))
				{
				case IS_HTML:
					SetFlagEx(MSFEX_HTML);
					// fall through
				case IS_RICH:
					SetFlag(MSF_XRICH);
					break;
				}
			}

			InHeader = FALSE;
		}

		in->Tell(&CSummary::m_lBegin);
		ASSERT(CSummary::m_lBegin >= 0);
	}

	CSummary::m_lLastTime = 0L;
	if (lNumBytesRead < 0 || InHeader)
		return (-1);

	m_Length = CSummary::m_lBegin - m_Offset;
	if (!*m_Date)
		FormatDate();

	// For IMAP:
	if (m_MsgSize <= 1)
		m_MsgSize = (unsigned short) max( (m_Length/1024 + 1), 1 );
		
	return (0);
}


static const int SumUnusedDiskSpace = 8;
 
int CSummary::Read(JJFile* in)
{
	// Default these to some arbitrary values.
	short x1 = 0, y1 = 0, x2 = 400, y2 = 300, tzm = 0;
	unsigned long mask = IMFLAGS_UNDOWNLOADED_ATTACHMENTS;	// Because of goto.

	HRESULT hrRead = S_OK;		// way up here 'cos of the goto's

    if (FAILED(in->Get(&m_Offset))) goto fail;
	if (FAILED(in->Get(&m_Length))) goto fail;
	if (FAILED(in->Get(&m_Seconds))) goto fail;
	if (FAILED(in->Get(&m_State))) goto fail;
	if (FAILED(in->Get(&m_Flags))) goto fail;
	if (FAILED(in->Get(&m_Priority))) goto fail;
	if (FAILED(in->Read(m_Date, sizeof(m_Date)))) goto fail;
	if (FAILED(in->Read(m_From, sizeof(m_From)))) goto fail;
	if (FAILED(in->Read(m_Subject, sizeof(m_Subject)))) goto fail;

	if (FAILED(in->Get(&x1))) goto fail;
	if (FAILED(in->Get(&y1))) goto fail;
	if (FAILED(in->Get(&x2))) goto fail;
	if (FAILED(in->Get(&y2))) goto fail;
	m_SavedPos.SetRect(x1, y1, x2, y2);

	if (FAILED(in->Get(&m_Label))) goto fail;
	if (FAILED(in->Get(&m_Hash))) goto fail;
	if (FAILED(in->Get(&m_UniqueMessageId))) goto fail;
	if (FAILED(in->Get(&m_FlagsEx))) goto fail;
	if (FAILED(in->Get(&m_PersonaHash))) goto fail;
	if (FAILED(in->Get(&tzm))) goto fail;
	m_TimeZoneMinutes = tzm;

	SetDate();

// IMAP4
	if (FAILED(in->Get(&m_Imflags))) goto fail;
	if (FAILED(in->Get(&m_MsgSize))) goto fail;

	// Format the number of un-downloaded attachments into the appropriate bits
	// of m_Imflags.
	//
	m_nUndownloadedAttachments = m_Imflags & IMFLAGS_UNDOWNLOADED_ATTACHMENTS;

	for (mask = IMFLAGS_UNDOWNLOADED_ATTACHMENTS; (mask & 1UL) == 0; mask >>= 1)
	{
		m_nUndownloadedAttachments >>= 1;
	}

	if (m_MsgSize <= 1)
		m_MsgSize = (unsigned short) max( (m_Length/1024 + 1), 1 );
// END IMAP

	hrRead = in->Read(Buffer, SumUnusedDiskSpace);
	if (FAILED(hrRead))
		return -1;
	else if (S_OK == hrRead)
		return SumUnusedDiskSpace;
	else if (S_FALSE == hrRead)
		return 0;		// hit EOF
	else
	{
		ASSERT(0);		// unexpected status
		return -1;
	}

fail:
	return (-1);
}


int CSummary::Write(JJFile *out)
{
	unsigned long mask = IMFLAGS_UNDOWNLOADED_ATTACHMENTS;	// Because of goto.
	unsigned long nUndownloadedAttachments = m_nUndownloadedAttachments;

    if (FAILED(out->Put(m_Offset))) goto fail;
	if (FAILED(out->Put(m_Length))) goto fail;
	if (FAILED(out->Put(m_Seconds))) goto fail;
	if (FAILED(out->Put(m_State))) goto fail;
	if (FAILED(out->Put(m_Flags))) goto fail;
	if (FAILED(out->Put(m_Priority))) goto fail;
	if (FAILED(out->Put(m_Date, sizeof(m_Date)))) goto fail;
	if (FAILED(out->Put(m_From, sizeof(m_From)))) goto fail;
	if (FAILED(out->Put(m_Subject, sizeof(m_Subject)))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.left))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.top))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.right))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.bottom))) goto fail;
	if (FAILED(out->Put(m_Label))) goto fail;
	if (FAILED(out->Put(m_Hash))) goto fail;
	if (FAILED(out->Put(m_UniqueMessageId))) goto fail;
	if (FAILED(out->Put(m_FlagsEx))) goto fail;
	if (FAILED(out->Put(m_PersonaHash))) goto fail;
	if (FAILED(out->Put((short)m_TimeZoneMinutes))) goto fail;

#ifdef IMAP4
	// Format the number of un-downloaded attachments into the appropriate bits
	// of m_Imflags.
	//
	
	nUndownloadedAttachments = m_nUndownloadedAttachments;

	for (mask = IMFLAGS_UNDOWNLOADED_ATTACHMENTS; (mask & 1UL) == 0; mask >>= 1)
	{
		nUndownloadedAttachments <<= 1;
	}

	// Clear it first:
	m_Imflags &= ~IMFLAGS_UNDOWNLOADED_ATTACHMENTS;
	m_Imflags |= nUndownloadedAttachments;

	if (FAILED(out->Put(m_Imflags))) goto fail;
	if (FAILED(out->Put(m_MsgSize))) goto fail;
#endif

	memset(Buffer, 0, SumUnusedDiskSpace);
	if (FAILED(out->Put(Buffer, SumUnusedDiskSpace))) goto fail;

	return (1);

fail:
	return (-1);
}

void CSummary::RedisplayField(int field, BOOL SetDirty /*= TRUE*/)
{
	//
	// The server status bits are actual stored in the LMOS file, so
	// don't mark the TOC as being dirty for server status changes.
	//
    if (SetDirty && ((field != FW_SERVER) 
#ifdef IMAP4
	// Ignore field test if this is an IMAP mailbox:
		|| m_TheToc->IsImapToc()
#endif	
	) )
	{
        m_TheToc->SetModifiedFlag();
	}

	m_TheToc->UpdateAllViews(NULL, field, this);

	if (FW_SERVER == field)
	{
		if (m_FrameWnd)
		{
			// give Frame a chance to display new server status
			extern UINT umsgButtonSetCheck;
			m_FrameWnd->SendMessage(umsgButtonSetCheck, ID_DELETE_FROM_SERVER, IsDeleteFromServerSet());
			m_FrameWnd->SendMessage(umsgButtonSetCheck, ID_DOWNLOAD_FROM_SERVER, IsDownloadFromServerSet());
		}
	}

	// We've changed some summary info in a field, so we need
	// to resort if the mailbox is being sorted by that field
	for (int i = 0; i < m_TheToc->NumSortColumns; i++)
	{
		const SortType st = m_TheToc->GetSortMethod(i);
		if (st == NOT_SORTED)
			break;
		if ((st - (st < BY_REVERSE_STATUS? 1 : BY_REVERSE_STATUS)) == field)
		{
			m_TheToc->SetNeedsSorting();
			break;
		}
	}
}

void CSummary::SetState(char State)
{
	if (m_State != State)
	{
		m_State = State;
		RedisplayField(FW_STATUS);
	}
}

void CSummary::SetTranslators(const char *trans, BOOL bValidate /* = FALSE */)
{ 
    if (*trans == '<') 
        trans++;

    int end = strlen(trans) - 1;

// BOG: so much for "const char" w/this following line. Very ugly coding, imo.
    if (*(trans + end) == '>') 
        *((char *)trans + end) = 0;

    // validating the translators will cause an error message to be displayed
    // for trans ids without assoc translators---the actual plugins can't be
    // identified tho. Bad ids will be stripped out of the list.

    if ( bValidate ) {
        CTranslatorManager* tmgr = ((CEudoraApp*)AfxGetApp())->GetTranslators();
        long lFlags = EMSF_Q4_TRANSMISSION | EMSF_Q4_COMPLETION;
        tmgr->ValidateTransIDs( trans, lFlags, m_TransHdr );
    }
    else
        m_TransHdr = trans;
}



void CSummary::SetPriority(int Priority, BOOL bUpdateFrame /* = TRUE */)
{
	if (m_Priority != Priority)
	{
		m_Priority = (short)Priority;
		RedisplayField(FW_PRIORITY);

		if ((bUpdateFrame) && (m_FrameWnd))
		{
			//
			// Notify the frame window of the priority change so that it
			// can update its UI.
			//
			extern UINT umsgSetPriority;
			m_FrameWnd->SendMessage(umsgSetPriority, 0, LPARAM(this));
		}
	}
}

void CSummary::SetLabel(int Label)
{
	if (m_Label != Label)
	{
		m_Label = (short)Label;
		RedisplayField(FW_LABEL);
	}
}

void CSummary::SetServerStatus(UINT nID)
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();

	if (pMsgRecord)
	{
		if (nID == ID_MESSAGE_SERVER_FETCH || nID == ID_MESSAGE_SERVER_FETCH_DELETE)
			pMsgRecord->SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE /*0*/);
		else
			pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE /*1*/);

		if (pMsgRecord->GetDeleteFlag() != LMOS_DELETE_SENT /*-1*/)
		{
			if (nID == ID_MESSAGE_SERVER_DELETE || nID == ID_MESSAGE_SERVER_FETCH_DELETE)
				pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE /*0*/);
			else
				pMsgRecord->SetDeleteFlag(LMOS_DONOT_DELETE /*1*/);
		}

		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}

void CSummary::GleanPriority(const char* HeaderContents)
{
	int prior = MSP_NORMAL;
	if (HeaderContents)
	{
		prior = atoi(HeaderContents);
		if( (prior < MSP_HIGHEST) || (prior > MSP_LOWEST) )
		{
			prior = MSP_NORMAL;
		}
	}
	SetPriority(prior);
}

void CSummary::SetFrom(const char* NewFrom)
{
    if (NewFrom && !strcmp(m_From, NewFrom))
        return; 
    memset(m_From, 0, sizeof(m_From));
    if (NewFrom)
    	strncpy(m_From, NewFrom, sizeof(m_From) - 1);
    RedisplayField(FW_SENDER);
}

void CSummary::SetSubject(const char* NewSubject)
{
    if (NewSubject && !strcmp(m_Subject, NewSubject))
        return; 
    memset(m_Subject, 0, sizeof(m_Subject));
    if (NewSubject)
    	strncpy(m_Subject, NewSubject, sizeof(m_Subject) - 1);
    RedisplayField(FW_UNUSED);
}


void CSummary::SetPersona(const char *persona)
{
	m_Persona = persona;
	m_PersonaHash = g_Personalities.GetHash( m_Persona );
}

CString CSummary::GetPersona()
{
	if ( m_PersonaHash && m_Persona.IsEmpty() )
	{
		// initialize m_Persona based on m_PersonaHash
		m_Persona = g_Personalities.GetByHash( m_PersonaHash );
	}

	return m_Persona;
}

void CSummary::SetSignature( const char *sig)
{
	m_SigHdr = sig;
	m_SigSelected = sig;
	if (sig && *sig)
	{
		m_Flags |= MSF_ALT_SIGNATURE; 
		m_Flags &= ~MSF_USE_SIGNATURE;
	}

	if (m_FrameWnd)
	{
		//
		// Notify the frame window of the signature change so that it
		// can update its UI.
		//
		extern UINT umsgSetSignature;
		m_FrameWnd->SendMessage(umsgSetSignature, 0, LPARAM(this));
	}
}


// SetDate
//
// Sets the internal date/time/timezone of a summary if a value is specified as an argument,
// which should only happen if this is a composition message.  Also, the m_Date string field
// is updated to contain the correct display for mailboxes.
//
void CSummary::SetDate(time_t Seconds /*= 0L*/)
{
	if (Seconds)
	{
		ASSERT(IsComp());
		m_Seconds = Seconds;
		m_TimeZoneMinutes = -GetGMTOffset();
	}

	char DateBuf[sizeof(m_Date)+32];
	m_Date[sizeof(m_Date) - 1] = 0;

	SetDateString(DateBuf, sizeof(m_Date)+32,
				  GetIniShort(IDS_INI_LOCAL_TIME_DISPLAY), 
				  GetIniShort(IDS_INI_FIXED_DATE_DISPLAY));

	if (strcmp(m_Date, DateBuf))
	{
		strncpy(m_Date, DateBuf, sizeof(m_Date)-1);
		RedisplayField(FW_DATE);
	}
}


////////////////////////////////////////////////////////////////////////
// SetDateString [private]
//
// Helper function for formatting the date string displayed in the TOC.
////////////////////////////////////////////////////////////////////////
void CSummary::SetDateString(char* pszDateBuf, int nSize, BOOL bDisplayLocalTime, BOOL bUseFixedFormat) const
{
	ASSERT(pszDateBuf != NULL);
	ASSERT(nSize > 0);

	::memset(pszDateBuf, 0, nSize);

	if (m_Seconds)
	{
		time_t DisplayDate = m_Seconds + m_TimeZoneMinutes * 60;
		int nDisplayTimeZone = m_TimeZoneMinutes;
		if (bDisplayLocalTime)
		{
			DisplayDate = m_Seconds - (GetGMTOffset() * 60);
			nDisplayTimeZone = INT_MAX;
		}

		if (bUseFixedFormat)
			::TimeDateStringFormatMT(pszDateBuf, DisplayDate, nDisplayTimeZone, GetIniString(IDS_INI_FIXED_DATE_FORMAT));
		else
		{
			time_t Age = time(NULL) - DisplayDate;
			if (Age > 60 * 60 * GetIniLong(IDS_INI_ANCIENT_DATE_HOURS))
				::TimeDateStringFormatMT(pszDateBuf, DisplayDate, nDisplayTimeZone, GetIniString(IDS_INI_ANCIENT_DATE_FORMAT));
			else if (Age > 60 * 60 * GetIniLong(IDS_INI_OLD_DATE_HOURS))
				::TimeDateStringFormatMT(pszDateBuf, DisplayDate, nDisplayTimeZone, GetIniString(IDS_INI_OLD_DATE_FORMAT));
			else
				::TimeDateStringFormatMT(pszDateBuf, DisplayDate, nDisplayTimeZone, GetIniString(IDS_INI_RECENT_DATE_FORMAT));
		}
	}

	ASSERT((int)strlen(pszDateBuf) < nSize); // better late than never
}


void CSummary::FormatDate(const char* GMTOffset)
{
	char GMTbuf[128];

	if (!m_Seconds)
	{
		SetDate();
		return;
	}

	if (GMTOffset)
	{
		strncpy(GMTbuf, GMTOffset, sizeof(GMTbuf) - 1);
		GMTbuf[sizeof(GMTbuf) - 1] = 0;

		// StripAddress() will strip out () comments
		StripAddress(GMTbuf);

		GMTOffset = strrchr(GMTbuf, ' ');
		if (GMTOffset && GMTOffset[1] != '-' && GMTOffset[1] != '+' &&
			!isalpha((int)(unsigned char)(GMTOffset[1])))
		{
			GMTOffset = NULL;
		}
	}

	// Store seconds as GMT time so sorts by date occur correctly
	if (GMTOffset)
	{
		GMTOffset++;
		if (!isdigit((int)(unsigned char)*GMTOffset) && *GMTOffset != '-' && *GMTOffset != '+')
		{
			// Do we have a timezone abbreviation for it
			int len = strlen(GMTOffset);
			CString ZoneInfo;
			for (UINT i = IDS_TZ_FIRST; TRUE; i++)
			{
				if (!ZoneInfo.LoadString(i) || ZoneInfo.IsEmpty())
					break;
				if (!strnicmp(ZoneInfo, GMTOffset, len))
				{
					strcpy((char *)GMTOffset,(const char*)ZoneInfo);
					GMTOffset += len;
					while(!isdigit((int)(unsigned char)*GMTOffset) && *GMTOffset != '-' && *GMTOffset != '+')
						GMTOffset++;
					break;
				}
			}
		}
		int num = atoi(GMTOffset);

		// Check for bogus timezones.  Ignore if bad.
		if (num >= -2400 && num <= 2400)
		{
			m_TimeZoneMinutes = num / 100 * 60;

			// Deal with minutes if there are some
			num %= 100;
			if (num)
				m_TimeZoneMinutes += num;

			m_Seconds -= m_TimeZoneMinutes * 60;
		}
	}

	SetDate();
}

// Abort procedure for printing
//extern BOOL CALLBACK AFX_EXPORT _AfxAbortProc(HDC, int);

void CSummary::PrintSum()
{
	BOOL		bWasOpen;
	QCProtocol*	pProtocol;

	bWasOpen = (m_FrameWnd != NULL);

	CMDIChild::m_nCmdShow = SW_HIDE;
	
	if( !m_FrameWnd )
	{
		Display();
	}
	
	pProtocol = QCProtocol::QueryProtocol( QCP_QUIET_PRINT, m_FrameWnd );
	
	if( pProtocol )
	{
		pProtocol->QuietPrint();
	}

	CMDIChild::m_nCmdShow = -1;

	if( !bWasOpen )
	{
		GetMessageDoc()->OnCloseDocument();
	}

}

void CSummary::ConvertAttachmentLine( char* buf )
{
	CString szPath;
	CRString szAttach(IDS_ATTACH_CONVERTED);
	int szAttachLen = szAttach.GetLength();

	char* pos=buf;

	while ( pos = strstr(buf, szAttach ) )
	{
		pos += szAttachLen +1;
		// Replace first quote with a space
		*pos = ' ';
		// Replace second quote with a space
		for (int i=0; *pos != '\"' && pos; pos++ )
			*pos = ' ';		
	}
}

BOOL CSumList::Print()
{
	CEudoraApp *theApp = (CEudoraApp *)AfxGetApp();
	POSITION pos = GetHeadPosition();

	if (pos)
	{
		// disable main window while printing & init printing status dialog
		AfxGetMainWnd()->EnableWindow(FALSE);
	
		CountdownProgress(CRString(IDS_PRINTING_MESSAGES_LEFT), GetCount());
		
		//BUG FIX 1793.  If no printer is attatched then let's not go any further.  If we allow 
		//this to continue strange stuff happens.
		CPrintInfo printInfo;
		if (!theApp->GetPrinterDeviceDefaults(&printInfo.m_pPD->m_pd))
		{
			CloseProgress();
			AfxGetMainWnd()->EnableWindow(TRUE);
			// bring up dialog to alert the user they need to install a printer.
			if (theApp->DoPrintDialog(printInfo.m_pPD) != IDOK)
				return FALSE;
		}
		
		while (pos)
		{
			CSummary* sum = GetNext(pos);
			if (sum)
			{
				Progress(-1, sum->GetTitle(), -1);
				sum->PrintSum();
			}
			if (EscapePressed(1))
				break;
			
			DecrementCountdownProgress();
		}
		CloseProgress();
		AfxGetMainWnd()->EnableWindow();    // enable main window
	}
	return (TRUE);
}


////////////////////////////////////////////////////////////////////////
// FindNextByMessageId [public]
//
// This is tricky.  Returns the POSITION of the *next* item, if any,
// that matches the search criteria in the list *after* the item with
// the given 'messageId'.  A messageId of 0xFFFFFFFF is a special case
// indicating that we want the first element in the list that meets
// the search criteria.
//
// This function supports Simple MAPI.
//
////////////////////////////////////////////////////////////////////////
POSITION CSumList::FindNextByMessageId(unsigned long messageId, BOOL unreadOnly)
{
	//
	// Get next item in the list matching the search criteria.
	//
	POSITION pos = GetHeadPosition();		// can be NULL
	BOOL return_next_match = (0xFFFFFFFF == messageId) ? TRUE : FALSE;
	while (pos != NULL)
	{
		CSummary* p_sum = GetAt(pos);
		ASSERT(p_sum != NULL);

		if (unreadOnly && (MS_UNREAD != p_sum->m_State))
		{
			GetNext(pos);		// skip it since we want only unread messages
			continue;
		}

		if (return_next_match)
			break;
		else if (p_sum->GetUniqueMessageId() == (long)messageId)
			return_next_match = TRUE;

		//
		// Move ahead to the *next* message, if any.
		//
		GetNext(pos);
	}

	return pos;		// can be NULL
}


////////////////////////////////////////////////////////////////////////
// GetByMessageId [public]
//
// Search the list for a message with a matching messageId and return
// a pointer to the summary object.  Silently return NULL if there is
// no match.
//
// This function supports Simple MAPI.
//
////////////////////////////////////////////////////////////////////////
CSummary* CSumList::GetByMessageId(unsigned long messageId)
{
	if (0 == messageId)
	{
		ASSERT(0);
		return NULL;
	}

	POSITION pos = GetHeadPosition();		// can be NULL
	while (pos != NULL)
	{
		CSummary* p_sum = GetNext(pos);
		ASSERT(p_sum != NULL);

		if (p_sum->GetUniqueMessageId() == (long)messageId)
			return p_sum;
	}

	return NULL;		// can be NULL
}



////////////////////////////////////////////////////////////////////////
// GetByMessageUid [public]
//
// Search the list for a message with a matching hash and return
// a pointer to the summary object.  Silently return NULL if there is
// no match.
//
// This function supports IMAP.
//
////////////////////////////////////////////////////////////////////////
CSummary* CSumList::GetByUid(unsigned long Uid)
{
	if (0 == Uid)
	{
		ASSERT(0);
		return NULL;
	}

	POSITION pos = GetHeadPosition();		// can be NULL
	while (pos != NULL)
	{
		CSummary* p_sum = GetNext(pos);
		ASSERT(p_sum != NULL);

		if (p_sum->GetHash() == Uid)
			return p_sum;
	}

	return NULL;		// can be NULL
}



void CSummary::SetSavedPos( const CRect& thePosition )
{
	m_SavedPos = thePosition;
	m_TheToc->SetModifiedFlag();
}

void CSummary::ToggleDeleteFromServer()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	if ( pMsgRecord )
	{
		//pMsgRecord->SetDeleteFlag(!pMsgRecord->GetDeleteFlag());

		if( pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE)
			pMsgRecord->SetDeleteFlag(LMOS_DONOT_DELETE);
		else
			pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE);

		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::ToggleDownloadFromServer()
{

	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		//pMsgRecord->SetRetrieveFlag(!pMsgRecord->GetRetrieveFlag());
		if(pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE)
			pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE);
		else
			pMsgRecord->SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE);
		
		// If a message gets marked for retrieve and LMOS is off, then set for removal
		// if not already set as such
		if (pMsgRecord->GetRetrieveFlag() == LMOS_DONOT_RETRIEVE && !GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) &&
			pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE /*0*/)
		{
			// CHostList::WriteLMOSFile will get called by this
			pMsgRecord->SetDeleteFlag( LMOS_DONOT_DELETE /*1*/ );
		}

		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


BOOL CSummary::IsMessageIsOnServer()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	return (pMsgRecord)?TRUE:FALSE;
	
}


BOOL CSummary::IsDeleteFromServerSet()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		//return !( pMsgRecord->GetDeleteFlag() );
		return ( pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE);
	}

	return FALSE;
}


BOOL CSummary::IsDownloadFromServerSet()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		//return !( pMsgRecord->GetRetrieveFlag() );
		return ( pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE);
	}

	return FALSE;
}


void CSummary::SetMessageServerLeave()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		pMsgRecord->SetDeleteFlag( LMOS_DONOT_DELETE /*1*/ );
		pMsgRecord->SetRetrieveFlag( LMOS_DONOT_RETRIEVE /*1*/ );
		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::SetMessageServerDelete()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	

	if ( pMsgRecord )
	{
		pMsgRecord->SetDeleteFlag( LMOS_DELETE_MESSAGE /*0*/ );
		pMsgRecord->SetRetrieveFlag( LMOS_DONOT_RETRIEVE /*1*/ );
		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::SetMessageServerDownload()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		pMsgRecord->SetRetrieveFlag( LMOS_RETRIEVE_MESSAGE /*0*/ );
					
		// If a message gets marked for retrieve and LMOS is off, then set for removal
		// if not already set as such
		if ( !GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) )
		{
			// CHostList::WriteLMOSFile will get called by this
			pMsgRecord->SetDeleteFlag( LMOS_DELETE_MESSAGE /*0*/ );
		}
		else
		{
			pMsgRecord->SetDeleteFlag( LMOS_DONOT_DELETE /*1*/ );
		}				

		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::SetMessageServerDownloadDelete()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		pMsgRecord->SetDeleteFlag( LMOS_DELETE_MESSAGE /*0*/ );
		pMsgRecord->SetRetrieveFlag( LMOS_RETRIEVE_MESSAGE /*0*/ );
		//CHostList::WriteLMOSFile();
		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}





// JOK, June 16, 1997.

// ImapParseImflags ()
// FUNCTION
// Extract character flags into an unsigned long.
// END FUNCTION

#ifdef IMAP4

unsigned long ImapParseImflags (LPCSTR pImflags)
{
	unsigned long Flags = 0;
	int c;

	if (!pImflags)
		return 0;

	while (c = *pImflags)
	{
		switch (c)
		{
			case 'R':
			case 'r':
				Flags |= IMFLAGS_SEEN;
				break;
			case 'A':
			case 'a':
				Flags |= IMFLAGS_ANSWERED;
				break;
			case 'F':
			case 'f':
				Flags |= IMFLAGS_FLAGGED;
				break;
			case 'D':
			case 'd':
				Flags |= IMFLAGS_DELETED;
				break;
			case ' T':
			case 't':
				Flags |= IMFLAGS_DRAFT;
				break;
			default:
				break;
		}
		pImflags++;
	}

	return Flags;
}

#endif

///////////////////////////////////////////////////////////////////////
// For IMAP4 (JOK, 9/24/97)
// FUNCTION
// Return TRUE is the given summary has the not-downloaded flag set.
// END FUNCTION
///////////////////////////////////////////////////////////////////////

#ifdef IMAP4

BOOL IsNotDownloaded(CSummary *pSum)
{
	if (!pSum)
		return FALSE;

	return pSum->m_Imflags & IMFLAGS_NOT_DOWNLOADED;
}

#endif

///////////////////////////////////////////////////////////////////////
// DownloadImapMessage [EXPORTED]
// FUNCTION
// If this message's contents are not yet downloaded, ask the Imap mailbox to download
// them now. 
// HISTORY:
// 12/18/97 (JOK) Added bQueueIfBusy argument: Passed to the IMAP reoutine.
// END HISTORY
///////////////////////////////////////////////////////////////////////
#ifdef IMAP4

HRESULT DownloadImapMessage (CSummary *pSum, BOOL bDownloadAttachments /* = FALSE */)
{
	HRESULT hResult = E_FAIL;

	if (!pSum)
		return E_INVALIDARG;

	CTocDoc *pTocDoc = pSum->m_TheToc;
	if ( pTocDoc && pTocDoc->IsImapToc() )
	{
		if (pTocDoc->m_pImapMailbox)
		{
			// Are the contents already down?
			if ( IsNotDownloaded (pSum) )
			{
				hResult = pTocDoc->m_pImapMailbox->OnPreviewSingleMessage (pTocDoc, pSum, bDownloadAttachments);

				if ( SUCCEEDED (hResult) )
				{
			        pTocDoc->SetModifiedFlag();

					CTocView *pTocView = pTocDoc->GetView();
					if (pTocView)
					{
						pSum->RedisplayField (FW_SERVER);
						pSum->RedisplayField (FW_SIZE);
					}
				}
			}
			else if ( pSum->m_Offset < 0 || pSum->m_Length <= 0)
			{
				// Make a check on m_Length.
				hResult = E_FAIL;
			}
			else
			{
				// It's been downloaded
				hResult = S_OK;
			} 
		}
	}

	return hResult;
}

#endif
