// msgdoc.cpp
//
// Base document class for read and composition messages
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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
#include "QCSharewareManager.h"
#include "PaigeEdtView.h"
#include "JunkMail.h"
#include "ImapFiltersd.h"
#include "..\Version.h"


extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;


#include "DebugNewHelpers.h"

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
	m_bWasTranslated = false;
	m_pAssociatedFiles = NULL;

	// Need to set the filename to something because otherwise MFC will give the
	// Save As file dialog when saving 
	m_strPathName = "A";
}

CMessageDoc::~CMessageDoc()
{
	delete [] m_Text;
	delete m_pAssociatedFiles;
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
	HRESULT		hr = Write();
	if ( FAILED(hr) )
	{
		CString		szMessageSaveErrorReason;

		if ( (HRESULT_FACILITY(hr) == FACILITY_ITF) &&
			 (HRESULT_CODE(hr) == MSE_MESSAGE_MISSING_HEADERS) )
		{
			szMessageSaveErrorReason.LoadString(IDS_ERR_MESSAGE_SAVE_ERROR_HEADERS_MISSING);
		}

		// Display error message with reason for failure if possible.
		ErrorDialog(IDS_ERR_MESSAGE_SAVE_ERROR, GetTitle(), szMessageSaveErrorReason);
		
		return (FALSE);
	}

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
		if (m_Sum->m_TheToc)
			m_Sum->m_TheToc->InvalidateCachedPreviewSummary(m_Sum, true);
	}

	SetModifiedFlag(FALSE);
	SetWasTranslated(false);
	
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

void CMessageDoc::SetAssociatedFiles(CStringList * in_pAssociatedFiles)
{
	if (!in_pAssociatedFiles)
		return;
	
	if (m_pAssociatedFiles)
	{
		m_pAssociatedFiles->AddTail(in_pAssociatedFiles);
		delete in_pAssociatedFiles;
	}
	else
	{
		m_pAssociatedFiles = in_pAssociatedFiles;
	}
}

bool CMessageDoc::GetFullMessage(CString & out_szMessage, BOOL in_bRaw /* = FALSE */, JJFile * in_pMailbox /* = NULL */)
{
#ifdef IMAP4 
	//
	// If we failed last time to download the message, try again.
	//
	if ( m_Sum && m_Sum->IsNotIMAPDownloadedAtAll() )
		m_QCMessage.Flush();

#endif // IMAP4
		

	if ( ! m_QCMessage.IsInit() )
	{
		char *		Message = NULL;
		long		lSaveOffset = 0;
		JJFile		mailbox;
		bool		bUseAlreadyOpenMailbox = false;
		JJFile *	pMailbox = &mailbox;

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
				return false;
		}

#endif // END IMAP4

		int Length = m_Sum->m_Length;

		while (1)
		{
			if ( in_pMailbox && (in_pMailbox->IsOpen() == S_OK) )
			{
				// Save the previous offset
				if ( FAILED(in_pMailbox->Tell(&lSaveOffset)) )
					break;
				pMailbox = in_pMailbox;
				bUseAlreadyOpenMailbox = true;
			}
			else
			{
				// No open mailbox parameter so open it ourselves
				if (FAILED(mailbox.Open(m_Sum->m_TheToc->GetMBFileName(), O_RDONLY)))
					break;
			}
		
			// check if size is zero, so it'll allocate one so it'll be 
			// able to assign m_Text[0]= 0 in next if statement
			Message = DEBUG_NEW_NOTHROW char[Length + 1];
			if (!Message)
				return false;
			
			if ( FAILED(pMailbox->Seek(m_Sum->m_Offset)) )
			{
				// Doesn't matter if we're using an already open mailbox -
				// seeking failed so we don't need to restore to the
				// old file location.
				ASSERT(0);
				break;
			}
			if (FAILED(pMailbox->Read(Message, Length)))
			{
				// Reading failed - restore file position if necessary before we bail.
				if (bUseAlreadyOpenMailbox)
					in_pMailbox->Seek(lSaveOffset);
				ASSERT(0);
				break;
			}
			Message[Length] = 0;

			// We're done reading - restore file position if necessary
			if (bUseAlreadyOpenMailbox)
			{
				HRESULT		hrSeek = in_pMailbox->Seek(lSaveOffset);

				ASSERT( SUCCEEDED(hrSeek) );
			}

			//The last 2 chars are usually the CR LF used to dilineate messages
			//and is not part of the message, so take those off.
			if (Message[Length-2] == '\r' && Message[Length-1] == '\n')
			{
				Length -= 2;
				Message[Length] = 0;
			}
	
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
						memcpy(FirstNULLSpot, m, end - m + 2);
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
		if (in_bRaw)
			m_QCMessage.GetRawMessage(out_szMessage);
		else
			m_QCMessage.GetFullMessage(out_szMessage);

		return true;
	}
	else
	{
		ErrorDialog(IDS_ERR_MSG_READING, m_Sum->m_TheToc->GetMBFileName());
		
		return false;
	}
}

char* CMessageDoc::GetFullMessage( BOOL bRaw /* = FALSE */, JJFile * in_pMailbox /* = NULL */ )
{
	CString		csMessage;
	char *		szMessage = NULL;

	if ( GetFullMessage(csMessage, bRaw, in_pMailbox) )
	{
		UINT Length = csMessage.GetLength();
		szMessage = DEBUG_NEW_NOTHROW char[Length + 1];
		if (!szMessage)
			return NULL;
		strcpy(szMessage, csMessage);
	}

	return szMessage;
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

	if (!m_Text || Length > m_BufSize || Length == 0)
	{
		delete [] m_Text;
		m_Text = DEBUG_NEW_NOTHROW char[Length + 1];
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
		char *catText = DEBUG_NEW char[newLength + 1];
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


BEGIN_MESSAGE_MAP(CMessageDoc, CDoc)
	//{{AFX_MSG_MAP(CMessageDoc)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY, OnUpdateNoComp)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY_ALL, OnUpdateReplyToAll)
	ON_COMMAND(ID_SPECIAL_MAKENICKNAME, OnMakeNickname)
	ON_COMMAND_EX_RANGE(ID_JUNK, ID_NOT_JUNK, OnChangeJunkStatus)
	ON_COMMAND(ID_RECHECK_JUNK, OnRecalculateJunkScore)

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

	ON_UPDATE_COMMAND_UI(ID_JUNK, OnUpdateJunk)
	ON_UPDATE_COMMAND_UI(ID_NOT_JUNK, OnUpdateNotJunk)
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


void CMessageDoc::OnUpdateReplyToAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Sum != NULL);
	
	// This is the Reply menu item, so continue the updating so that the text
	// can be set if necessary
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

        if (pQCProtocol)
            pQCProtocol->GetSelectedHTML(szText);
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
	// Must have valid summary pointer
	if (!m_Sum)
	{
		ASSERT( !"No valid summary pointer" );
		return TRUE;
	}

	// Must have valid toc doc
	CTocDoc *		toc = m_Sum->m_TheToc;
	if (!toc)
	{
		ASSERT( !"No valid toc doc pointer" );
		return TRUE;
	}

	CSumList &		listSums = toc->GetSumList();
	POSITION		pos = listSums.Find(m_Sum);
	
	if (pos)
	{
		BOOL		 bIgnoreDeletedIMAP = toc->HideDeletedIMAPMsgs();
		CSummary	*pSum = NULL;
		if (Direction == ID_NEXT_MESSAGE)
		{
			listSums.GetNext(pos);
			if (pos)
			{
				pSum = listSums.GetAt(pos);
				while (bIgnoreDeletedIMAP && pos && pSum && pSum->IsIMAPDeleted())
				{
					listSums.GetNext(pos);
					if (pos)
					{
						pSum = listSums.GetAt(pos);
					}
				}
			}
		}
		else
		{
			listSums.GetPrev(pos);
			if (pos)
			{
				pSum = listSums.GetAt(pos);
				while (bIgnoreDeletedIMAP && pos && pSum && pSum->IsIMAPDeleted())
				{
					listSums.GetPrev(pos);
					if (pos)
					{
						pSum = listSums.GetAt(pos);
					}
				}
			}
		}
			
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
	if (!m_Sum || !m_Sum->m_TheToc)
	{
		ASSERT(0);
		return;
	}

	CFilterActions		*pFilt = NULL;
	CTocDoc				*pToc = m_Sum->m_TheToc;

	// Note it is vital that we use the appropriate type of filter actions object because if the message
	// is in an IMAP mailbox the filtering code will treat the filtering object like it is a
	// CImapFilterActions object, not just a CFilterActions object.
	if (pToc->IsImapToc())
	{
		pFilt = DEBUG_NEW_MFCOBJ_NOTHROW CImapFilterActions();
	}
	else
	{
		pFilt = DEBUG_NEW_MFCOBJ_NOTHROW CFilterActions();
	}

	if (pFilt && pFilt->StartFiltering())
	{
		CTocView* TocView = (pToc->GetView());

		CObArray	oaABHashes;

		// Generate the address book hashes
		CFilter::GenerateHashes(&oaABHashes);

		if ((pFilt->FilterOne(m_Sum, WTA_MANUAL, &oaABHashes) & FA_TRANSFER) && TocView)
		{
			TocView->m_SumListBox.SetRedraw(TRUE);
		}
		pFilt->EndFiltering();
	}

	delete pFilt;
}

void CMessageDoc::OnMakeNickname()
{
	MakeNickname();
}

//
//	CMessageDoc::OnRecalculateJunkScore()
//
//	Recalculate the junk scores for the selected message.
//
void CMessageDoc::OnRecalculateJunkScore()
{
	// Start filtering in case rescoring needs to use filters.
	CFilterActions	 filt;
	if (filt.StartFiltering())
	{
		CObArray	oaABHashes;

		// Generate the address book hashes
		CFilter::GenerateHashes(&oaABHashes);
		
		// Reprocess the message.
		CJunkMail::ReprocessOne(m_Sum, &filt, &oaABHashes);

		// Done with filtering.
		filt.EndFiltering();
	}
}

//
//	CMessageDoc::OnUpdateJunk()
//
//	Enable the Junk menu item appropriately.  If IDS_INI_ALWAYS_ENABLE_JUNK
//	is set, always enable, otherwise only enable if the message is not
//	currently in the Junk mailbox.
//
void CMessageDoc::OnUpdateJunk(CCmdUI* pCmdUI)
{
	// Junk menus not available in light.
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;
	
	if (m_Sum && !m_Sum->IsComp() && m_Sum->m_TheToc)
	{
		pCmdUI->Enable( !m_Sum->m_TheToc->IsJunk() || GetIniShort(IDS_INI_ALWAYS_ENABLE_JUNK) );
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

//
//	CMessageDoc::OnUpdateNotJunk()
//
//	Enable the Not Junk menu item appropriately.  If IDS_INI_ALWAYS_ENABLE_JUNK
//	is set, always enable, otherwise only enable if the message is
//	currently in the Junk mailbox.
//
void CMessageDoc::OnUpdateNotJunk(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;
	
	if (m_Sum && !m_Sum->IsComp() && m_Sum->m_TheToc)
	{
		pCmdUI->Enable( (m_Sum->m_TheToc->IsJunk()) || GetIniShort(IDS_INI_ALWAYS_ENABLE_JUNK) );
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

//
//	CMessageDoc::OnChangeJunkStatus()
//
//	Change the junk status of this message to the specified state.
//
//	Parameters:
//		nID[in] - Command that was issued (ID_JUNK or ID_NOT_JUNK).
//
BOOL CMessageDoc::OnChangeJunkStatus(UINT nID)
{
	if (m_Sum)
	{
		// Start filtering in case changing status needs to use filters.
		CFilterActions	 filt;
		if (filt.StartFiltering())
		{
			// Hash the address book for translator use and potentially
			// determining if we need to add not junked senders to the AB.
			CObArray	oaABHashes;

			CFilter::GenerateHashes(&oaABHashes);

			// Determine if we're adding Not Junk'ed senders to the address book.
			bool		bAddNotJunkedSendersToAB = (nID == ID_NOT_JUNK) &&
												   GetIniShort(IDS_INI_ADDBOOK_IS_WHITELIST) &&
												   GetIniShort(IDS_INI_ADD_NONJUNK_TO_ADDBOOK);

			// Set the new state for this message.
			CJunkMail::DeclareJunk(m_Sum, (nID == ID_JUNK), bAddNotJunkedSendersToAB, &oaABHashes, &filt);

			// Done with filtering.
			filt.EndFiltering();
		}
	}

	return (m_Sum != NULL);
}


/// Added from doc.h

BOOL CMessageDoc::OnNewDocument()
{
	m_QCMessage.Flush();
	delete [] m_Text;
	m_Text = NULL;
	m_BufSize = 0;

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
				m_Sum->m_TheToc->SetModifiedFlag(TRUE);
				break;
				}
			case IDCANCEL:
				return FALSE;
				break;
			case IDABORT:
				//disable the dialog for this message without sending the Read Receipt
				m_Sum->UnsetFlag(MSF_READ_RECEIPT);
				m_Sum->m_TheToc->SetModifiedFlag(TRUE);
				break;
			case IDRETRY:
				//enable the dialog for this message without sending the Read Receipt
				//But next time this e-mail shows up, it will prompt again.
				m_Sum->SetFlag(MSF_READ_RECEIPT);
				m_Sum->m_TheToc->SetModifiedFlag(TRUE);
				break;
			}
		}
	}

	if ( !IsModified() && !WasTranslated() )
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
		if (m_pAssociatedFiles)
		{
			//	The user has chosen not to save the message - delete the associated
			//	attachment and embedded files.
			while ( !m_pAssociatedFiles->IsEmpty() )
			{
				CString		szPath = m_pAssociatedFiles->RemoveHead();

				if ( !szPath.IsEmpty() )
					DeleteFile(szPath);
			}
		}

		if (m_Sum && m_Sum->m_TheToc)
			m_Sum->m_TheToc->InvalidateCachedPreviewSummary(m_Sum, true);
		break;

	default:
		ASSERT(FALSE);
		return (FALSE);       // don't continue
	}

	// If we get here, it may be the case that the user hit
	// No when asked to save the changes.  If this is so, then the modified
	// flag needs to be cleared so the save prompt doesn't come up again.
	SetModifiedFlag(FALSE);

	// Make sure we know that we are no longer translated (we were either
	// saved or discarded by the user's choice above).
	SetWasTranslated(false);

	// Delete the list of associated files, if any. We've already deleted
	// the files themselves if necessary above.
	delete m_pAssociatedFiles;
	m_pAssociatedFiles = NULL;
	
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
	CMakeFilter		mkFiltDlg;
	CTempSumList	summaryList;

	summaryList.AddTail(m_Sum);

	if (mkFiltDlg.GoFigure(&summaryList))
		mkFiltDlg.DoModal();
}


void CMessageDoc::AssignMessageId( void )
{
	char msgid[ 256 ];
	LPCTSTR MailServer = GetIniString(IDS_INI_RETURN_ADDRESS);

	MailServer = strrchr(MailServer, '@');
	if (MailServer)
		MailServer++;
	else
		MailServer = "wheresmymailserver.com";

	sprintf(msgid, "%s.%d.%s.%08lx@%s", 
		EUDORA_BUILD_VERSION,
		GetSharewareMode(),
		::FormatTimeMT( time( NULL ), "%Y%m%d%H%M%S"),
		(DWORD)this, 
		MailServer);

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
		DoFileCommand(szTempFilename, 0, false);
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
	pCmdUI->Enable(TRUE);
//	pCmdUI->Enable(m_Sum->IsHTML());
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

	ASSERT(m_Sum != NULL);
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


