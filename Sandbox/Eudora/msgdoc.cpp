// MSGDOC.CPP
//
// Base document class for read and composition messages
//

#include "stdafx.h"
#include <afxcmn.h>

#include "resource.h"
#include "rs.h"	
#include "helpxdlg.h"
#include "doc.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "msgdoc.h"
#include "rs.h"
#include "tocdoc.h"
#include "3dformv.h"
#include "tocview.h"
#include "guiutils.h"
#include "eudora.h"
#include "utils.h"
#include "mdichild.h"
#include "msgframe.h"
#include "mainfrm.h"
#include "namenick.h"
#include "filtersd.h"
#include "compmsgd.h"
#include "ReadReceiptDialog.h"
#include "mime.h"
#include "msgutils.h"
#include "persona.h"
#include "address.h"
#include "controls.h"
#include "MakeFilter.h"
#include "newmbox.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxDirector.h"

#include "PaigeEdtView.h"


extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessageDoc

IMPLEMENT_DYNAMIC(CMessageDoc, CDoc)

CMessageDoc::CMessageDoc() :
	m_QCMessage(this),
	m_FindIndex(-1),			// -1 indicates that no Find actions have been executed
	m_DidFindOpenView(FALSE)
{
	m_Sum = NULL;
	m_Text = NULL;
	m_BufSize = 0;

	// Need to set the filename to something because otherwise MFC will give the
	// Save As file dialog when saving 
	m_strPathName = "A";
}

CMessageDoc::~CMessageDoc()
{
	delete [] m_Text;
}

BOOL CMessageDoc::OnOpenDocument(const char* Sum)
{	
	// A bit of a kludge, but it allows the use the dynamic creation routines
	m_Sum = (CSummary*)Sum;
	ASSERT(m_Sum != NULL);
	ASSERT(m_Sum->IsKindOf(RUNTIME_CLASS(CSummary)));
	
	if (!Read())
		return (FALSE);
		
	SetModifiedFlag(FALSE);
	
	return (TRUE);
}

BOOL CMessageDoc::OnSaveDocument(const char*)
{
	if (!Write())
		return (FALSE);

	//
	// Flush the content of the QCMessage so that subsequent calls to
	// CMessageDoc::GetFullMessage() re-read the newly saved message
	// content back from disk.  FORNOW, could this cause the
	// memory-resident m_Text data to get slightly out of synch with
	// the QCMessage object?
	//
	m_QCMessage.Flush();

	if (m_Sum)
	{
		//
		// Tell the TocDoc to cleanup its "previewable" Summary 
		// cache.  In turn, the TocDoc notifies its Frame that its
		// previewed message is being invalidated.  Then, reset the TocDoc
		// "preview" Summary so that the Frame redisplays the previewed
		// message properly.
		//
		ASSERT(m_Sum->m_TheToc);
		if (m_Sum->m_TheToc->GetPreviewableSummary() == m_Sum)
		{
			m_Sum->m_TheToc->InvalidatePreviewableSummary(m_Sum);
			m_Sum->m_TheToc->SetPreviewableSummary(m_Sum);
		}
	}

	SetModifiedFlag(FALSE);
	
	return (TRUE);
}
   
// Closing down the frame window, so null out the member that points to it
void CMessageDoc::PreCloseFrame(CFrameWnd* pFrame)
{
	//
	// FORNOW, this is also in CMessageFrame::OnDestroy() since it
	// was causing shutdown GPFs ...  it looks like there is a race
	// condition between the time the CSummary object is deleted and
	// the corresponding frame window is destroyed.
	//
	if (m_Sum)
		m_Sum->m_FrameWnd = NULL;

	CDoc::PreCloseFrame(pFrame);
}

char* CMessageDoc::GetFullMessage( BOOL bRaw /* = FALSE */ )
{
	CString csMessage;
	char *	Message = NULL;

#ifdef IMAP4 
	//
	// If we failed last time to download the message, try again.
	//
	if (IsNotDownloaded (m_Sum))
		m_QCMessage.Flush();

#endif // IMAP4
		

	if ( ! m_QCMessage.IsInit() )
	{
		JJFile in;

		// BOG: the QCMessage object needs to be initialized regardless of
		// message length, so it is better to just go ahead and let things run
		// their course; everything appears to fail gracefully.
		if (!m_Sum || !m_Sum->m_TheToc /*|| !m_Sum->m_Length*/)
			return (NULL);

#ifdef IMAP4 // IMAP4 - If thyis is an IMAP toc, make sure the message has been downloaded. If not,
			// IMAP4 - Go download it now. If we fail, get out.

		if ( m_Sum->m_TheToc->IsImapToc () )
		{
			if ( ! SUCCEEDED (DownloadImapMessage (m_Sum)) )
				return NULL;
		}

#endif // END IMAP4

		int Length = m_Sum->m_Length;

		while (1)
		{
			if (FAILED(in.Open(m_Sum->m_TheToc->MBFilename(), O_RDONLY)))
				break;
		
			// check if size is zero, so it'll allocate one so it'll be 
			// able to assign m_Text[0]= 0 in next if statement
			Message = new char[Length + 1];
			if (!Message)
				return NULL;
		
			in.Seek(m_Sum->m_Offset);
			if (FAILED(in.Read(Message, Length)))
				break;
			Message[Length] = 0;

			//The last 2 chars are usually the CR LF used to dilineate messages
			//and is not part of the message, so take those off.
			if (Message[Length-2] == '\r' && Message[Length-1] == '\n')
				Message[Length-2] = 0;
	
			// If NULL bytes are in the message, get rid of them
			char* end = Message + Length - 1;
			char* FirstNULLSpot = NULL;
			for (char* m = Message; m < end; m++)
			{
				if (!*m)
				{
					if (!FirstNULLSpot)
						FirstNULLSpot = m;
				}
				else
				{
					if (FirstNULLSpot)
					{
						int NumNULLs = m - FirstNULLSpot;
						strcpy(FirstNULLSpot, m);
						m -= NumNULLs;
						end -= NumNULLs;
						FirstNULLSpot = NULL;
					}
				}
			}

			// Get rid of all but one trailing CRLF in Read messages
			if (m_Sum->IsComp() == FALSE)
			{
				const char* Body = FindBody(Message);
				if (Body && *Body)
				{
					while (end >= Body && (*end == '\r' || *end == '\n'))
						end--;
					if (end[1] == '\r' && end[2] == '\n')
						end[3] = 0;
				}
			}
		
			// Skip past the Sendmail From line
			char* t;
			t = strchr(Message, '\n');
			if (t)
				strcpy(Message, t + 1);

			// get the Message-Id header
			char* msgid = HeaderContents(IDS_HEADER_MESSAGE_ID, Message);
			if (msgid && ( *msgid != '\0' ) )
			{
				// delete '<' and '>' characters
				msgid[ strlen( msgid ) - 1 ] = '\0';
				m_MessageId = &msgid[ 1 ];
				delete [] msgid;
			}
			else
			{
				// must be a pre-existing message
				AssignMessageId();
			}

			m_QCMessage.Init( m_MessageId, Message );
			delete [] Message;

			break;	// this is the ok case...
		}
	}

	if ( m_QCMessage.IsInit() )
	{
		if ( bRaw )
			m_QCMessage.GetRawMessage( csMessage );
		else
			m_QCMessage.GetFullMessage( csMessage );

		UINT Length = csMessage.GetLength();
		Message = new char[Length + 1];
		if (!Message)
			return NULL;
		strcpy( Message, csMessage );
		return (Message);
	}

	// bad read or message...
	if (Message)
		delete [] Message;

	ErrorDialog(IDS_ERR_MSG_READING, m_Sum->m_TheToc->MBFilename());

	return (NULL);
}
						 
// Read
// Reads in a message from the mailbox that this document belongs to
//
BOOL CMessageDoc::Read()
{
	m_BufSize = 0;
	delete[] m_Text;
	m_Text = GetFullMessage();
	
	//Empty Stationeries will not have m_Text
	if (!m_Text)
		return (FALSE);
	//New comp messages will be allocated atleast one byte in 
	//SetText or QCMessage::Init, so if the message is empty 
	//return FALSE
	if (!*m_Text)
		return (FALSE);
	
	m_BufSize = strlen(m_Text);

	ReallySetTitle(m_Sum->MakeTitle());

/*
	// get the Message-Id header
	char* msgid = HeaderContents(IDS_HEADER_MESSAGE_ID, m_Text);
	if (msgid)
	{
		// delete '<' and '>' characters
		msgid[ strlen( msgid ) - 1 ] = '\0';
		m_MessageId = &msgid[ 1 ];
		delete [] msgid;
	}
	else
	{
		// must be a pre-existing message
		AssignMessageId();
	}
*/

	return (TRUE);
}

char* CMessageDoc::GetText()
{
	// failing this is a *total* failure; if we succeed, we initialize our funky
	// little mimestore thingy and flush it so we don't leave copies of the
	// message layin' all over the place.
	if ( !m_Text && !Read() )
		return (NULL);

	// however it is likely that the doc was "zeroed" to save on the ol' memory
	// footprint, causing the message to be re-read every time it is "browsed"
	// in the toc, even if it is open in a separate read-view.
	if ( !(*m_Text) )
		Read();
	
	return (m_Text);
}

const char* CMessageDoc::SetText(const char* NewText)
{
	UINT Length = ::SafeStrlenMT(NewText);

	if (Length > m_BufSize || Length == 0)
	{
		delete [] m_Text;
		m_Text = new char[Length + 1];
		m_BufSize = m_Text? Length : 0;
	}

	if (m_Text)
	{
		if (NewText)
			strcpy(m_Text, NewText);
		else
			*m_Text = 0;
	}

	return (m_Text);
}

const char* CMessageDoc::CatText(const char* NewText)
{
	UINT newLength = m_BufSize + ::SafeStrlenMT(NewText);

	if (newLength > m_BufSize)
	{
		char *catText = new char[newLength + 1];
		strcpy(catText, m_Text);
		delete [] m_Text;
		m_Text = catText;
		
		m_BufSize = m_Text? newLength : 0;
	}

	if (m_Text)
	{
		if (NewText)
			strcat(m_Text, NewText);
	}

	return (m_Text);
}


const char* CMessageDoc::SetText(CEdit* Ctrl)
{
	int Length = Ctrl->GetWindowTextLength();

	if ((UINT)Length > m_BufSize || Length == 0)
	{
		delete m_Text;
		m_Text = new char[Length + 1];
		m_BufSize = m_Text? Length : 0;
	}

	if (m_Text)
		Ctrl->GetWindowText(m_Text, Length + 1);

	return (m_Text);
}

BEGIN_MESSAGE_MAP(CMessageDoc, CDoc)
	//{{AFX_MSG_MAP(CMessageDoc)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY, OnUpdateNoComp)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY_ALL, OnUpdateNoComp)
	ON_COMMAND(ID_SPECIAL_MAKENICKNAME, OnMakeNickname)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REDIRECT, OnUpdateNoComp)
	ON_COMMAND(ID_SPECIAL_FILTERMESSAGES, OnFilterMessages)
	ON_COMMAND(ID_SPECIAL_MAKE_FILTER, OnMakeFilter)

	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_VIEW_SOURCE, OnCmdViewSource)
	ON_UPDATE_COMMAND_UI(ID_SEND_TO_BROWSER, OnUpdateSendToBrowser)
	ON_COMMAND(ID_SEND_TO_BROWSER, OnSendToBrowser)
	//}}AFX_MSG_MAP
	// Last five above added from doc.h

	ON_COMMAND_EX(ID_MESSAGE_REPLY, ComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_REPLY_ALL, ComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_FORWARD, ComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_REDIRECT, ComposeMessage)
	ON_COMMAND(ID_MESSAGE_SENDAGAIN, OnSendAgain)

	ON_COMMAND_EX(ID_PREVIOUS_MESSAGE, OnSwitchMessage)
	ON_COMMAND_EX(ID_NEXT_MESSAGE, OnSwitchMessage)

	ON_COMMAND_EX(ID_MESSAGE_LABEL_NONE, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_1, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_2, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_3, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_4, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_5, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_6, OnLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_7, OnLabel)

	ON_COMMAND_EX(ID_MESSAGE_PRIOR_HIGHEST, OnSetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_HIGH, OnSetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_NORMAL, OnSetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_LOW, OnSetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_LOWEST, OnSetPriority)

	ON_COMMAND( ID_TRANSFER_NEW_MBOX_IN_ROOT, OnTransferNewInRoot )

	ON_COMMAND_EX_RANGE(QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand)
	ON_UPDATE_COMMAND_UI_RANGE(QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMessageDoc commands

void CMessageDoc::OnUpdateNoComp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Sum && (m_Sum->IsComp() == FALSE));
	
	// If this is the Reply menu item, then continue the updating so that the text
	// can be set if necessary
	if (pCmdUI->m_nID == ID_MESSAGE_REPLY)
		pCmdUI->ContinueRouting();
	if (pCmdUI->m_nID == ID_MESSAGE_REPLY_ALL)
		pCmdUI->ContinueRouting();
}


void CMessageDoc::ComposeMessage(UINT MenuItemID, const char* DefaultTo, const char* stationery /*= NULL*/,
				 const char* pSelText /*= NULL*/)
{
    if ( m_Sum )
	m_Sum->ComposeMessage( MenuItemID, DefaultTo, FALSE, stationery, NULL, pSelText );
}


BOOL CMessageDoc::ComposeMessage(UINT MenuItemID)
{
    CString szText = "";
    const char* pSelText = GetSelectedText( szText ) ? (const char*)szText : NULL;
    ComposeMessage( MenuItemID, NULL, NULL, pSelText );
    return TRUE;
}


unsigned CMessageDoc::GetSelectedText( CString& szText )
{
    if ( m_Sum ) {
        szText = "";
        CView* pView = NULL;
        QCProtocol* pQCProtocol = NULL;
        CDocument* pDoc = this;            // first sign that something ugly is going on!
        POSITION pos = pDoc->GetFirstViewPosition();

        for ( int i = 0; (i < 2) && !pQCProtocol; i++ ) {
            if ( pos ) {
                do {
                    pView = pDoc->GetNextView( pos );
                    pQCProtocol = QCProtocol::QueryProtocol( QCP_GET_MESSAGE, pView );

                    if ( pQCProtocol )
                        break;

                } while ( pos );
            }
            else {
                // BOG: if we're being invoked from a Preview Pane, then there are no views
                // attached to this doc---we'll try to get the active view and use that

                CWnd* pWnd = AfxGetMainWnd();

                if ( pWnd ) {
                    if ( pWnd->IsKindOf( RUNTIME_CLASS( CMDIFrameWnd ) ) ) {
                        CMDIChildWnd* pMDIChild = ((CMDIFrameWnd*)pWnd)->MDIGetActive();
                        if ( pMDIChild ) {
                            pDoc = pMDIChild->GetActiveDocument();
                            if ( pDoc )
                                pos = pDoc->GetFirstViewPosition();
                        }
                    }
                }

                // now *that* was ugly!
            }
        }

        if ( pQCProtocol ) {
                pQCProtocol->GetSelectedHTML( szText );
        }
    }

    return szText.GetLength();
}



void CMessageDoc::OnSendAgain()
{
	if (m_Sum)
		m_Sum->SendAgain();
}

BOOL CMessageDoc::OnSwitchMessage(UINT Direction)
{
	POSITION pos;
	CTocDoc* toc;
	
	if (m_Sum && (toc = m_Sum->m_TheToc) && (pos = toc->m_Sums.Find(m_Sum)))
	{
		if (Direction == ID_NEXT_MESSAGE)
			toc->m_Sums.GetNext(pos);
		else
			toc->m_Sums.GetPrev(pos);
			
		// If there is a message to switch to then do it,
		// otherwise this message just needs to be closed
		if (pos)
			toc->DoSwitchMessage(Direction, pos);
		else if (m_Sum->m_FrameWnd)
			m_Sum->m_FrameWnd->SendMessage(WM_CLOSE);
	}
	
	return (TRUE);
}

BOOL CMessageDoc::OnLabel(UINT LabelMenuID)
{
	if (m_Sum)
		m_Sum->SetLabel(LabelMenuID - ID_MESSAGE_LABEL_NONE);

	return (TRUE);
}



////////////////////////////////////////////////////////////////////////
// OnSetPriority [protected]
//
// Handler for ID_MESSAGE_PRIOR_xxx commands.
////////////////////////////////////////////////////////////////////////
BOOL CMessageDoc::OnSetPriority(UINT uPriorityCmdID)
{
	if (m_Sum)
	{
		int nPriority = MSP_NORMAL;

		switch (uPriorityCmdID)
		{
		case ID_MESSAGE_PRIOR_HIGHEST:	nPriority = MSP_HIGHEST;	break;
		case ID_MESSAGE_PRIOR_HIGH:	  	nPriority = MSP_HIGH;		break;
		case ID_MESSAGE_PRIOR_NORMAL: 	nPriority = MSP_NORMAL;		break;
		case ID_MESSAGE_PRIOR_LOW:	  	nPriority = MSP_LOW;		break;
		case ID_MESSAGE_PRIOR_LOWEST: 	nPriority = MSP_LOWEST;		break;
		default:
			ASSERT(0);
			return FALSE;
		}

		m_Sum->SetPriority(nPriority);
	}

	return TRUE;
}


void CMessageDoc::OnFilterMessages()
{
	CFilterActions filt;
	if (filt.StartFiltering())
	{
		CTocDoc* Toc = m_Sum->m_TheToc;
		CTocView* TocView = (Toc? Toc->GetView() : NULL);
		if ((filt.FilterOne(m_Sum, WTA_MANUAL) & FA_TRANSFER) && TocView)
		{
			TocView->m_SumListBox.SetRedraw(TRUE);
		}
		filt.EndFiltering();
	}
}

void CMessageDoc::OnMakeNickname()
{
	MakeNickname();
}

/// Added from doc.h

BOOL CMessageDoc::OnNewDocument()
{
	return (TRUE);
}

void CMessageDoc::SetTitle(const char* pszTitle)
{
	// The title should only have to be set once.  This prevents screen flicker when saving.
	if (m_strTitle.IsEmpty())
		CDocument::SetTitle(pszTitle);
}

// Overridden so that the title gets used in the dialog that asks if changes need to be saved,
// rather than what MFC uses, the filename.
BOOL CMessageDoc::SaveModified()
{
	if (m_Sum)
	{
		ASSERT(AfxIsValidAddress(m_Sum, sizeof(CSummary)));
		ASSERT(m_Sum->IsKindOf(RUNTIME_CLASS(CSummary)));
		
		if (AfxIsValidAddress(m_Sum, sizeof(CSummary)) &&
			m_Sum->IsKindOf(RUNTIME_CLASS(CSummary)) &&
			m_Sum->IsComp() == FALSE &&
			m_Sum->ReadReceipt())
		{
			BOOL WasAuto = FALSE;
			int rc = 0;

			// RR Dialog Override - jdboyd 8/23/99
			if (m_Sum->m_rrDialogOverride)
			{
				// If we've already prompted for a return receipt, don't ask again.
				rc = IDRETRY;
			}
			else if (GetIniShort(IDS_INI_READ_RECEIPT_ASK))
			{
				CReadReceiptDlg dlgRR( m_Sum->GetTitle() );
				rc = dlgRR.DoModal();

				// RR Dialog override -jdboyd 8/23/99
				// Make sure the RR dialog does not show up again until after this message has been closed.
				if (rc==IDRETRY) m_Sum->m_rrDialogOverride = true;
			}
			else if (GetIniShort(IDS_INI_READ_RECEIPT_YES))
			{
				rc = IDOK;
				WasAuto = TRUE;
			}
			else
				rc = IDABORT;

			switch (rc)
			{
			case IDOK:
				{
				//disable the dialog for this message after sending the Read Receipt
				WriteReadReceipt(WasAuto);
				m_Sum->UnsetFlag(MSF_READ_RECEIPT);
				break;
				}
			case IDCANCEL:
				return FALSE;
				break;
			case IDABORT:
				//disable the dialog for this message without sending the Read Receipt
				m_Sum->UnsetFlag(MSF_READ_RECEIPT);
				break;
			case IDRETRY:
				//enable the dialog for this message without sending the Read Receipt
				//But next time this e-mail shows up, it will prompt again.
				m_Sum->SetFlag(MSF_READ_RECEIPT);
				break;
			}
		}
	}

	if (!IsModified())
		return (TRUE);

	CString prompt;
	INT nResult;
	//Code obsolete, the ini setting has been removed.
	//if(GetIniShort(IDS_INI_WARN_CHANGED_MESSAGE_READONLY))
	//{
		AfxFormatString1(prompt, IDS_SAVE_CHANGES, m_strTitle);
		nResult = AfxMessageBox(prompt, MB_YESNOCANCEL);
	//}
	//else
	//	nResult = IDYES;
	


	switch (nResult)
	{
	case IDCANCEL:
		
		// RR Dialog Override	-jdboyd 8/23/99
		// User has cancelled save, close will cancel.  Forget about asking about the RR.
		m_Sum->m_rrDialogOverride = false;
	
		return (FALSE);       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoSave(m_strPathName))
			return (FALSE);       // don't continue
		break;
	
	case IDNO:
		break;

	default:
		ASSERT(FALSE);
		return (FALSE);       // don't continue
	}

	// If we get here, it may be the case that the user hit
	// No when asked to save the changes.  If this is so, then the modified
	// flag needs to be cleared so the save prompt doesn't come up again.
	SetModifiedFlag(FALSE);
	
	return (TRUE);    // keep going

}

/////////////////////////////////////////////////////////////////////////////
// CDoc commands

void CMessageDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsModified());
}

void CMessageDoc::OnFileSave()
{
	static BOOL AlreadyIn = FALSE;
	if (AlreadyIn)
		return;
	AlreadyIn = TRUE;

	OnSaveDocument(NULL);
	if (ShiftDown())
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_SAVE_ALL);

	AlreadyIn = FALSE;
}

void CMessageDoc::OnFileClose()
{
	if (ShiftDown())
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_CLOSE_ALL);
	else
	{
		// MFC bug.  Should just act like a normal window close.
		((CFrameWnd*)AfxGetMainWnd())->GetActiveFrame()->SendMessage(WM_CLOSE);
	}
}


// If we get here looking for whether or not to enable the File->Save As...
// menu item, then we don't want to enable it.  This has to be done because
// CDocument has a default handler that we don't want called.
void CMessageDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CMessageDoc::OnFileSaveAs()
{
	ASSERT(FALSE);
}



BOOL CMessageDoc::FindFirst(const CString& searchStr, BOOL matchCase, BOOL wholeWord, BOOL bOpenWnd /* = TRUE */)
{
	return FALSE;
}

BOOL CMessageDoc::FindNext(const CString& searchStr, BOOL matchCase, BOOL wholeWord)
{
	return FALSE;
}
BOOL CMessageDoc::FindAgain(const CString& searchStr, BOOL matchCase, BOOL wholeWord)
{
	return FALSE;
}


void CMessageDoc::WriteReadReceipt(BOOL WasAuto)
{
	JJFile* pFile = OpenAttachFile((char *)(const char*)CRString(IDS_READ_RECEIPT_FILE));
	
	ASSERT(pFile);
	
	if (!pFile)
		return;
	
	char *pFullMsg = GetFullMessage();

	// get the Persona X-Header, if any
	CString old_persona = g_Personalities.GetCurrent();
	BOOL changed_persona = FALSE;

	char *p_persona = HeaderContents(IDS_PERSONA_XHEADER, pFullMsg);
	if (p_persona)
	{
		CString persona(p_persona);

		if (persona[0] == '<' && persona[persona.GetLength() - 1] == '>')
		{

			persona = persona.Left(persona.GetLength() - 1);
			persona = persona.Right(persona.GetLength() - 1);
			if (old_persona != persona)
			{
				g_Personalities.SetCurrent(persona);
				changed_persona = TRUE;
			}
		}

		CString XPersonaString;
		XPersonaString.Format(CRString(IDS_PERSONA_XHEADER_FORMAT), (const char *) persona);
		pFile->PutLine( XPersonaString );

		delete [] p_persona;
	}

	// To: <...>
	char *pToHdr = HeaderContents(IDS_MIME_HEADER_READR, pFullMsg);
	pFile->Put(CRString(IDS_HEADER_TO));
	pFile->Put(' ');
	pFile->PutLine(pToHdr);
	delete [] pToHdr;

	// Subject: Notification for "..."
	char NewSubject[256];
	pFile->Put(CRString(IDS_HEADER_SUBJECT));
	pFile->Put(' ');
	sprintf(NewSubject, CRString(IDS_MIME_SUBJECT_RETURN), m_Sum->GetSubject());
	pFile->PutLine(NewSubject);

	// MIME-Version: 1.0
	pFile->Put(CRString(IDS_MIME_HEADER_VERSION));
	pFile->Put(' ');
	pFile->PutLine(CRString(IDS_MIME_VERSION));

	// Content-Type: multipart/report; boundary="_"; report-type=disposition-notification
	pFile->Put(CRString(IDS_MIME_HEADER_CONTENT_TYPE));
	pFile->Put(' ');
	pFile->Put(CRString(IDS_MIME_MULTIPART));
	pFile->Put('/');
	pFile->Put(CRString(IDS_MIME_REPORT));
	pFile->Put("; ");
	pFile->Put(CRString(IDS_MIME_BOUNDARY));
	pFile->Put("=\"_\"; ");
	pFile->Put(CRString(IDS_MIME_REPORT_TYPE));
	pFile->Put('=');
	pFile->PutLine(CRString(IDS_MIME_DISPOSITION_NOTIFICATION));

	// Blank line separating headers from body
	pFile->PutLine();

	// Opening boundary
	pFile->PutLine("--_");

	// Content-Type: text/plain
	pFile->Put(CRString(IDS_MIME_HEADER_CONTENT_TYPE));
	pFile->Put(' ');
	pFile->Put(CRString(IDS_MIME_TEXT));
	pFile->Put('/');
	pFile->PutLine(CRString(IDS_MIME_PLAIN));

	// Blank line separation
	pFile->PutLine();

	// Compose Textual message, with proper persona
	{

		char RealName[128];
		char* Subject = HeaderContents(IDS_HEADER_SUBJECT, pFullMsg);
		char* Date = HeaderContents(IDS_HEADER_DATE, pFullMsg);
		CString msg_body;
		GetIniString(IDS_INI_REAL_NAME, RealName, sizeof(RealName));
		msg_body.Format(CRString(IDS_READ_RECEIPT_BODY), (Date? Date : ""), (Subject? Subject : ""),
						RealName);
		pFile->PutLine(msg_body);
		pFile->PutLine();
		delete [] Subject;
		delete [] Date;

	}

	// Blank line separation
	pFile->PutLine();

	// Boundary separating text message from notification part
	pFile->PutLine("--_");

	// Notification
	pFile->Put(CRString(IDS_MIME_HEADER_CONTENT_TYPE));
	pFile->Put(' ');
	pFile->Put(CRString(IDS_MIME_MESSAGE));
	pFile->Put('/');
	pFile->PutLine(CRString(IDS_MIME_DISPOSITION_NOTIFICATION));

	// Blank line separation
	pFile->PutLine();

	// Reporting-UA: ...
	char VersionBuf[256];
	pFile->Put(CRString(IDS_MIME_REPORTING_UA));
	sprintf(VersionBuf, CRString(IDS_SMTP_HEADER_VERSION), (const char*)CRString(IDS_VERSION));
	pFile->PutLine(strchr(VersionBuf, ' ') + 1);

	// Final-Recipient: rfc822; <x@y.com>
	pFile->Put(CRString(IDS_MIME_FINAL_RECIPIENT));
	char *pRecipientHdr = ::SafeStrdupMT(GetReturnAddress());
	if(pRecipientHdr)
	{
		StripAddress(pRecipientHdr);
		pFile->Put('<');
		pFile->Put(pRecipientHdr);
		pFile->PutLine(">");
		delete [] pRecipientHdr;
	}

	// Original-Message-ID: <.....>
	char *pMsgIDHdr = HeaderContents(IDS_POP_MESSAGE_ID_HEADER, pFullMsg);
	if (pMsgIDHdr && *pMsgIDHdr)
	{
		pFile->Put(CRString(IDS_MIME_ORIG_MSG_ID));
		pFile->Put(' ');
		pFile->PutLine(pMsgIDHdr);
	}
	delete [] pMsgIDHdr;

	// Disposition: manual-action/<sending-mode>; displayed
	pFile->Put(CRString(IDS_MIME_DISPOSITION));
	pFile->Put(' ');
	pFile->Put(CRString(IDS_MDN_MANUAL_ACTION));
	pFile->Put('/');
	pFile->Put(CRString(WasAuto? IDS_MDN_SENT_AUTOMATIC : IDS_MDN_SENT_MANUAL));
	pFile->Put("; ");
	pFile->PutLine(CRString(IDS_MIME_DISPOSITION_DISPLAYED));

	// Blank line separation
	pFile->PutLine();

	// Closing boundary
	pFile->PutLine("--_--");
	
	pFile->Close();

	delete [] pFullMsg;

	// Reading and sending the msg file created above
	CString strFilename("???");
	{
		BSTR bstrFilename = NULL;
		if (SUCCEEDED(pFile->GetFName(&bstrFilename)))
		{
			strFilename = bstrFilename;
			::SysFreeString(bstrFilename);
		}
	}
	CSummary* Sum = NewMessageFromFile(strFilename);
	if (Sum)
		Sum->SetFlagEx(MSFEX_MDN);

	// deleting the msg file created above
	pFile->Delete();
	delete pFile;

	if (changed_persona)
		g_Personalities.SetCurrent(old_persona);
}


void CMessageDoc::OnMakeFilter()
{
	CMakeFilter	mkFiltDlg;
	CSumList	summaryList;

	summaryList.Add(m_Sum);

	if (mkFiltDlg.GoFigure(&summaryList))
		mkFiltDlg.DoModal();

	summaryList.RemoveAll();
}


void CMessageDoc::AssignMessageId( void )
{
	CRString Version(IDS_VERSION);
	char version[ 128 ];
	char msgid[ 256 ];

	const char* VersionNum = strchr(Version, ' ');
	if (VersionNum)
		VersionNum++;
	else
		VersionNum = Version;
	strcpy(version, VersionNum);

	char* Space = strchr(version, ' ');
	if (Space)
		*Space =0;

	const char* At = strrchr(GetIniString(IDS_INI_POP_ACCOUNT), '@');

	sprintf(msgid, "%s.%s.%08lx%s", 
		version, 
		::FormatTimeMT( time( NULL ), "%Y%m%d%H%M%S"),
		(DWORD)this, 
		At);

	m_MessageId = msgid;
}

////////////////////////////////////////////////////////////////////////
// ViewMessageExternally [protected]
//
// Helper functon for doing View Source and Send to Browser.
////////////////////////////////////////////////////////////////////////
void CMessageDoc::ViewMessageExternally(BOOL ForceText)
{
	ASSERT(::IsMainThreadMT());

	//
	// Load the entire message into a big memory buffer.
	//
	const char* pszMessageText = GetText();
	const char* pszBodyText = FindBody(pszMessageText);

	//
	// Write the message into a tmp file.
	//
	char szTempFilename[_MAX_PATH] = { '\0' };
	{
		//extern CString TempDir -  declared in fileutl.h
		if ( (!TempDir.IsEmpty()) && ::GetTempFileName(TempDir, "eud", 0, szTempFilename))
		{
			//
			// ::GetTempFileName will always give us a ".tmp" extension.
			// If we're doing a View Source (ForceText == TRUE), then
			// we use a ".txt" extension.
			// If we're doing a Send to Browser (Forcetext == FALSE),
			// then we use the appropriate extension for the type of
			// text in the message (.txt or .htm)
			//
			int nLength = ::strlen(szTempFilename);
			ASSERT(nLength > 4);
			ASSERT(::stricmp(szTempFilename + nLength - 4, ".tmp") == 0);
			if (!ForceText && m_Sum->IsHTML())
				::strcpy(szTempFilename + nLength - 4, ".htm");
			else
				::strcpy(szTempFilename + nLength - 4, ".txt");

			JJFile theFile;
			
			if (SUCCEEDED(theFile.Open( szTempFilename, O_CREAT | O_TRUNC | O_WRONLY)))
			{
				if (FAILED(theFile.Put(pszBodyText)))
				{
					//
					// Indicate failure by truncating filename.
					//
					szTempFilename[0] = '\0';
				}
			}
		}
	}

	//
	// Pass the responsibility for launching the file to the shell.
	//
	if (szTempFilename[0])
	{
		::ShellExecute(::AfxGetMainWnd()->GetSafeHwnd(),	// HWND
						NULL,								// operation
						szTempFilename,						// filename
						NULL,								// exe parameters
						NULL,								// default directory
						SW_SHOWNORMAL);						// show command (as in ShowWindow() call)
	}
}

////////////////////////////////////////////////////////////////////////
// OnCmdViewSource [protected]
//
// Command handler for ID_VIEW_SOURCE command.
////////////////////////////////////////////////////////////////////////
void CMessageDoc::OnCmdViewSource()
{
	ViewMessageExternally(TRUE);
}

////////////////////////////////////////////////////////////////////////
// OnUpdateSendToBrowser [protected]
//
// Command Update handler for ID_SEND_TO_BROWSER command.
////////////////////////////////////////////////////////////////////////
void CMessageDoc::OnUpdateSendToBrowser(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Sum->IsHTML());
}

////////////////////////////////////////////////////////////////////////
// OnSendToBrowser [protected]
//
// Command handler for ID_SEND_TO_BROWSER command.
////////////////////////////////////////////////////////////////////////
void CMessageDoc::OnSendToBrowser()
{
	ViewMessageExternally(FALSE);
}

////////////////////////////////////////////////////////////////////////
// OnDynamicCommand [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CMessageDoc::OnDynamicCommand(UINT uID)
{
	QCCommandObject*	pCommand = NULL;
	COMMAND_ACTION_TYPE	theAction;

	if( !g_theCommandStack.GetCommand( ( WORD ) uID, &pCommand, &theAction ) )
	{
		return FALSE;
	}

	if( ( pCommand == NULL ) || !theAction )
	{
		return FALSE;
	}

	if( theAction == CA_TRANSFER_NEW )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		pCommand = g_theMailboxDirector.CreateTargetMailbox( ( QCMailboxCommand* ) pCommand, TRUE );
		if ( NULL == pCommand )
			return TRUE;		// user didn't want to transfer after all

		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		ASSERT( ( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_REGULAR );
		theAction = CA_TRANSFER_TO;
	}

	VERIFY(m_Sum != NULL);
	if ( pCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
	{
		if ( theAction == CA_TRANSFER_TO )
		{
			if ( ShiftDown() )
				pCommand->Execute( CA_COPY, m_Sum );
			else
				pCommand->Execute( CA_TRANSFER_TO, m_Sum );
			return TRUE;
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnUpdateDynamicCommand [protected]
//
////////////////////////////////////////////////////////////////////////
void CMessageDoc::OnUpdateDynamicCommand(CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if ( ( theAction == CA_TRANSFER_TO ) || 
				( theAction == CA_TRANSFER_NEW ) )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->ContinueRouting();
}


void CMessageDoc::OnTransferNewInRoot()
{
	QCMailboxCommand*	pCommand;
	pCommand = g_theMailboxDirector.CreateTargetMailbox( NULL, TRUE );
	if ( pCommand )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		ASSERT( pCommand->GetType() == MBT_REGULAR );
		ASSERT( m_Sum );

		if ( ShiftDown() )
			pCommand->Execute( CA_COPY, m_Sum );
		else
			pCommand->Execute( CA_TRANSFER_TO, m_Sum );
	}
}


