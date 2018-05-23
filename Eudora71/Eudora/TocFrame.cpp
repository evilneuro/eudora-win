// TocFrame.cpp : implementation file
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include <map>
#include <set>

#include "resource.h"
#include "mainfrm.h"
#include "rs.h"
#include "utils.h"
#include "fileutil.h"
#include "guiutils.h"
#include "helpcntx.h"
#include "msgutils.h"
#include "TocFrame.h"
#include "TocView.h"
#include "compmsgd.h"
#include "ReadMessageDoc.h"
#include "PgReadMsgPreview.h"
#include "TridentPreviewView.h"
#include "SearchDoc.h"
#include "SearchView.h"
#include "msgframe.h"
#include "trnslate.h"
#include "eudora.h"
#include "cursor.h"
#include "FileBrowseView.h"
#include "changeq.h"
#include "filtersd.h"
#include "JunkMail.h"
#include "MakeFilter.h"
#include "namenick.h"
#include "persona.h"
#include "progress.h"
#include "SaveAsDialog.h"
#include "Sendmail.h"
#include "QCFindMgr.h"
#include "address.h"
#include "AutoCompleteSearcher.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCPersonalityCommand.h"
#include "QCPluginCommand.h"
#include "QCRecipientCommand.h"
#include "QCStationeryCommand.h"


// FOR IMAP4 - yuck. Shouldn't need to include this.

#include "ImapMailbox.h"

// END IMAP4

extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;

#include "DebugNewHelpers.h"

//	Constants
const UINT		CTocFrame::kCheckPreviewSelection	= 10;
const UINT		CTocFrame::kCheckAutoMarkAsRead		= 20;

UINT umsgLoadNewPreview = ::RegisterWindowMessage("umsgLoadNewPreview");

// Registered message for checking the preview selection after we've been notified
// that it may have changed via NotifyPreviewSelectionMayHaveChanged. Note that this
// is intentionally static. It is NOT meant to be used as a messaging mechanism
// from any other source code file.
static UINT umsgCheckPreviewSelection = RegisterWindowMessage( "umsgCheckPreviewSelection" );

static const int	kFillEntireWindowHeight = 0x7FFFFFFF;


/////////////////////////////////////////////////////////////////////////////
// CTocFrame

IMPLEMENT_DYNCREATE(CTocFrame, CMDIChildTarget)

CTocFrame::CTocFrame()
	:	m_dwStartPreviewTime(0), m_nTocListHeight(kFillEntireWindowHeight),
		m_PeanutToolBar(NULL), m_PeanutTabCtrl(NULL), m_previewSummaryArray(),
		m_emptySummaryArray(), m_pSumListBox(NULL), m_nCheckPreviewSelectionTimerID(0),
		m_nCheckAutoMarkAsReadTimerID(0), m_nLastSplitterPos(-1),
		m_bInsideDoPreviewDisplay(false),
		m_bIgnorePreviewSelectionChanges(false), m_bPreviewSelectionChangesAttempted(false),
		m_bCacheInvalidated(false), m_bPreviewAllowed(false), m_bDisplayingPreview(false),
		m_bSelectionResultDirectUserAction(false), m_wndSplitter()
{
}

CTocFrame::~CTocFrame()
{
	delete m_PeanutTabCtrl;
	delete m_PeanutToolBar;
}


BEGIN_MESSAGE_MAP(CTocFrame, CMDIChildTarget)
	//{{AFX_MSG_MAP(CTocFrame)
	ON_REGISTERED_MESSAGE( umsgCheckPreviewSelection, OnCheckPreviewSelection )
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_CLEAR, OnCmdDelete)
	ON_COMMAND(ID_MESSAGE_DELETE, OnCmdDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateCmdDelete)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_DELETE, OnUpdateCmdDelete)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(TCN_SELCHANGE, IDC_PEANUT_TAB, OnTabSelchange)
	ON_COMMAND(ID_PEANUT_ACTIONS, OnPeanutActions)
	ON_UPDATE_COMMAND_UI(ID_PEANUT_ACTIONS, OnUpdatePeanutActions)
	ON_WM_CLOSE()
	ON_COMMAND(IDA_TOGGLEHEADER, OnToggleHeader)
	ON_COMMAND(ID_EDIT_FIND_FINDMSG, OnEditFindFindMsg)
	ON_WM_CHILDACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_UNDELETE, OnUpdateCmdUndelete)
	ON_COMMAND(ID_MESSAGE_UNDELETE, OnCmdUnDelete)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PURGE, OnUpdateCmdImapExpunge)
	ON_COMMAND(ID_MESSAGE_PURGE, OnCmdImapExpunge)
	//}}AFX_MSG_MAP

	// Commands not handled by ClassWizard
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

	ON_UPDATE_COMMAND_UI(ID_SPECIAL_MAKENICKNAME, OnUpdateNeedSel)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY_ALL, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_FORWARD, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REDIRECT, OnUpdateResponse)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SENDAGAIN, OnUpdateResponse)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SENDIMMEDIATELY, OnUpdateSendImmediately)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_CHANGEQUEUEING, OnUpdateChangeQueueing)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNREAD, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_READ, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REPLIED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_FORWARDED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REDIRECTED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_RECOVERED, OnUpdateNeedSelNoOut)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TOGGLE, OnUpdateToggleStatus)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENDABLE, OnUpdateSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_QUEUED, OnUpdateSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TIME_QUEUED, OnUpdateSendable)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENT, OnUpdateSentUnsent)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNSENT, OnUpdateSentUnsent)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_HIGHEST, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_HIGH, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_NORMAL, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_LOW, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PRIOR_LOWEST, OnUpdateNeedSel)
	
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_NONE, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_1, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_2, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_3, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_4, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_5, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_6, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_7, OnUpdateNeedSel)

	ON_UPDATE_COMMAND_UI(ID_RECHECK_JUNK, OnUpdateJunkCommands)
	ON_UPDATE_COMMAND_UI(ID_JUNK, OnUpdateJunkCommands)
	ON_UPDATE_COMMAND_UI(ID_NOT_JUNK, OnUpdateJunkCommands)

	ON_UPDATE_COMMAND_UI(ID_SPECIAL_MAKE_FILTER, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_FILTERMESSAGES, OnUpdateNeedSel)

	// Added by JOK - 4/20/98
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_LEAVE, OnUpdateServerStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_FETCH, OnUpdateServerStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_DELETE, OnUpdateServerStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_FETCH_DELETE, OnUpdateServerStatus)

	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrintOne)
	ON_COMMAND(ID_RECHECK_JUNK, OnRecheckMessagesForJunk)
	ON_COMMAND(ID_JUNK, OnDeclareMessagesJunk)
	ON_COMMAND(ID_NOT_JUNK, OnDeclareMessagesNotJunk)
	ON_COMMAND(ID_SPECIAL_MAKENICKNAME, OnMakeNickname)
	
	ON_COMMAND_EX(ID_MESSAGE_REPLY, OnComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_REPLY_ALL, OnComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_FORWARD, OnComposeMessage)
	ON_COMMAND_EX(ID_MESSAGE_REDIRECT, OnComposeMessage)
	ON_COMMAND(ID_MESSAGE_SENDAGAIN, OnSendAgain)
	
	ON_COMMAND(ID_MESSAGE_SENDIMMEDIATELY, OnSendImmediately)
	ON_COMMAND(ID_MESSAGE_CHANGEQUEUEING, OnChangeQueueing)

	ON_COMMAND_EX(ID_MESSAGE_STATUS_UNREAD, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_READ, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_REPLIED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_FORWARDED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_REDIRECTED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_SENDABLE, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_QUEUED, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_SENT, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_UNSENT, SetStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_RECOVERED, SetStatus)

	ON_COMMAND(ID_MESSAGE_STATUS_TOGGLE, OnToggleStatus)

	ON_COMMAND_EX(ID_MESSAGE_PRIOR_HIGHEST, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_HIGH, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_NORMAL, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_LOW, SetPriority)
	ON_COMMAND_EX(ID_MESSAGE_PRIOR_LOWEST, SetPriority)
	
	ON_COMMAND_EX(ID_MESSAGE_LABEL_NONE, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_1, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_2, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_3, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_4, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_5, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_6, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_7, SetLabel)

	ON_COMMAND_EX(ID_MESSAGE_SERVER_LEAVE, SetServerStatus)
	ON_COMMAND_EX(ID_MESSAGE_SERVER_FETCH, SetServerStatus)
	ON_COMMAND_EX(ID_MESSAGE_SERVER_DELETE, SetServerStatus)
	ON_COMMAND_EX(ID_MESSAGE_SERVER_FETCH_DELETE, SetServerStatus)

	ON_COMMAND(ID_SPECIAL_FILTERMESSAGES, OnFilterMessages)
	ON_COMMAND(ID_SPECIAL_MAKE_FILTER, OnMakeFilter)

	ON_UPDATE_COMMAND_UI( ID_TRANSFER_NEW_MBOX_IN_ROOT, OnUpdateNeedSel )
	ON_COMMAND( ID_TRANSFER_NEW_MBOX_IN_ROOT, OnTransferNewInRoot )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )

#ifdef IMAP4
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_UNDELETE, OnUpdateUndelete)
	ON_COMMAND(ID_MESSAGE_UNDELETE, UnDelete)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_PURGE, OnUpdateImapExpunge)
	ON_COMMAND(ID_MESSAGE_PURGE, ImapExpunge)
	ON_COMMAND(ID_MESSAGE_IMAP_CLEAR_CACHED, OnImapClearCached)
	ON_COMMAND(ID_MBOX_IMAP_RESYNC, OnImapResync)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_REDOWNLOAD_FULL, OnUpdateImapRedownload)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT, OnUpdateImapRedownload)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_FETCH_FULL, OnUpdateImapFetch)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_FETCH_DEFAULT, OnUpdateImapFetch)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_IMAP_CLEAR_CACHED, OnUpdateImapClearCached)

	ON_COMMAND_EX(ID_MESSAGE_IMAP_REDOWNLOAD_FULL, OnImapRedownload)
	ON_COMMAND_EX(ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT, OnImapRedownload)
	ON_COMMAND_EX(ID_MESSAGE_IMAP_FETCH_FULL, OnImapFetch)
	ON_COMMAND_EX(ID_MESSAGE_IMAP_FETCH_DEFAULT, OnImapFetch)
#endif

END_MESSAGE_MAP()

BOOL CTocFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
	BOOL fRet;

	fRet = m_wndSplitter.CreateStatic( this, 2, 1, WS_CHILD | WS_VISIBLE );
	ASSERT(fRet);

	if (fRet)
	{
		fRet = m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CTocView ),
					CSize( 315, 140 ), pContext );
		ASSERT(fRet);
	}

	if (fRet)
	{
		CRuntimeClass* pPreviewRuntimeClass = RUNTIME_CLASS(PgReadMsgPreview);

		if ( UsingTrident() )
			pPreviewRuntimeClass = RUNTIME_CLASS(CTridentPreviewView);

		fRet = m_wndSplitter.CreateView(1, 0, pPreviewRuntimeClass, CSize(10, 600), pContext);
		m_wndSplitter.SetViewType(pPreviewRuntimeClass);

		ASSERT(fRet);
	}

	return fRet;
}

BOOL CTocFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Pass sort-related commands to the TocView.  Kind of a hack, but allows a good
	// separation of layers in not having all the sort-related functions being in CTocFrame.
	if ((nID >= ID_EDIT_SORT_SORTBYSTATUS && nID <= ID_EDIT_SORT_SORTBYSUBJECT) ||
		nID == ID_EDIT_SORT_GROUPBYSUBJECT)
	{
		CTocView *	pTocView = GetTocView();
		if (pTocView)
		{
			if (pTocView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
				return TRUE;
		}
	}
	else if (nID == ID_FILE_PRINT || nID == ID_FILE_PRINT_DIRECT || nID == ID_FILE_PRINT_PREVIEW)
	{
		// Only try to route commands toward the preview pane if we're actually previewing
		if (ShouldPreview())
		{
			// If we're dealing with printing and there's only one message selected,
			// then route the command toward the preview pane so that it can handle it
			CTocView *	pTocView = GetTocView();
			if (pTocView && pTocView->GetSelCount() == 1)
			{
				CWnd* pPreviewPane = m_wndSplitter.GetPane(1, 0);
				if (pPreviewPane && pPreviewPane->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
					return TRUE;
			}
		}
	}

	return CMDIChildTarget::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/*virtual*/ BOOL CTocFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle /*= WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE*/,
	CWnd* pParentWnd /*= NULL*/, CCreateContext* pContext /*= NULL*/)
{
	if (CMDIChildTarget::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		CTocDoc* pTocDoc = GetTocDoc();
		if (pTocDoc && pTocDoc->IsPeanutTagged())
		{
			m_PeanutToolBar = DEBUG_NEW_MFCOBJ_NOTHROW CToolBar;
			if (m_PeanutToolBar)
			{
				m_PeanutToolBar->CreateEx(this, TBSTYLE_FLAT,
					WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY);
				m_PeanutToolBar->LoadToolBar(IDC_PEANUT_TOOLBAR);

				m_PeanutTabCtrl = DEBUG_NEW_MFCOBJ_NOTHROW CTabCtrl;
				if (m_PeanutTabCtrl)
				{
					CRect Crect;
					m_PeanutTabCtrl->Create(WS_VISIBLE | WS_TABSTOP, Crect, m_PeanutToolBar, IDC_PEANUT_TAB);
					m_PeanutTabCtrl->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

					TC_ITEM TabCtrlItem;
					TabCtrlItem.mask = TCIF_TEXT;

					CRString Mailbox(IDS_PEANUT_TAB_MAILBOX);
					TabCtrlItem.pszText = (LPTSTR)(LPCTSTR)Mailbox;
					m_PeanutTabCtrl->InsertItem(0, &TabCtrlItem);

					CRString Workgroup(IDS_PEANUT_TAB_FOLDER);
					TabCtrlItem.pszText = (LPTSTR)(LPCTSTR)Workgroup;
					m_PeanutTabCtrl->InsertItem(1, &TabCtrlItem);

					Crect.SetRectEmpty();
					for (int i = 0; i < m_PeanutTabCtrl->GetItemCount(); i++)
					{
						CRect ItemRect;
						if (m_PeanutTabCtrl->GetItemRect(i, &ItemRect))
						{
							Crect.right += ItemRect.Width() + 2;
							Crect.bottom = __max(Crect.bottom, Crect.top + ItemRect.Height());
						}
					}

					if (pTocDoc->ShowFileBrowser())
						m_PeanutTabCtrl->SetCurSel(1);

					m_PeanutToolBar->SetButtonInfo(0, IDC_PEANUT_TAB, TBBS_SEPARATOR, Crect.Width());
					m_PeanutToolBar->GetItemRect(0, &Crect);
					m_PeanutTabCtrl->MoveWindow(&Crect);
				}

				ShowControlBar(m_PeanutToolBar, TRUE, FALSE);
			}
		}

		return TRUE;
	}

	return FALSE;
}


// every time the mdi child is moved, resized, clicked-to-front...
void CTocFrame::OnChildActivate()
{
    CMDIChildTarget::OnChildActivate();

	//	Set timer to check the preview selection (using timer instead of message may help
	//	avoid bug where initially opened windows ended up being blank at times - bug
	//	that existed in previous code before Content Concentrator rewrite).
	m_nCheckPreviewSelectionTimerID = SetTimer(kCheckPreviewSelection, 200, NULL);

	ASSERT(m_pSumListBox);
}


void CTocFrame::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CTocFrame::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


LONG CTocFrame::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCProtocol *pProtocol;
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (!pFindMgr)
		return (EuFIND_ERROR);

	// This is our internal message to ask if we support find.
	// Return non-zero (TRUE).
	if (pFindMgr->IsAck(wParam, lParam))
		return (EuFIND_ACK_YES);

	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if (!lpFR)
		return (EuFIND_ERROR);

	if (lpFR->Flags & FR_DIALOGTERM)
	{
		ASSERT(0); // Should never fwd a terminating msg
		return (EuFIND_ERROR);
	}

	CWnd *pwndFocus = m_wndSplitter.GetActivePane();

	if(!pwndFocus)
		return (EuFIND_ERROR);

	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, pwndFocus );
	
	if( ( pProtocol != NULL ) &&
		( pProtocol->DoFindNext( lpFR->lpstrFindWhat, (lpFR->Flags & FR_MATCHCASE), (lpFR->Flags & FR_WHOLEWORD), TRUE) ) )
	{
		// activate the window
		pwndFocus->GetParentFrame()->ActivateFrame();
		return (EuFIND_OK);
	}
		

	return (EuFIND_NOTFOUND);
}


void CTocFrame::OnUpdateNeedSel(CCmdUI * pCmdUI)
{
	pCmdUI->Enable( m_pSumListBox && (m_pSumListBox->GetSelCount() > 0) );
	
	if (pCmdUI->m_nID >= ID_MESSAGE_LABEL_1 && pCmdUI->m_nID <= ID_MESSAGE_LABEL_7)
		pCmdUI->ContinueRouting();
}


void CTocFrame::OnUpdateResponse(CCmdUI * pCmdUI)
{
	BOOL EnableIt = FALSE;
	
	if ( m_pSumListBox && (m_pSumListBox->GetSelCount() > 0) )
	{
		// Enable all menu items except Reply and Redirect for the Out mailbox
		// Allow Reply to All because it can be useful and we handle it for
		// sent messages in every other mailbox.
		EnableIt = TRUE;

		CTocDoc *	pTocDoc = GetTocDoc();

		if ( pTocDoc && pTocDoc->m_Type == MBT_OUT &&
			 (pCmdUI->m_nID == ID_MESSAGE_REPLY || pCmdUI->m_nID == ID_MESSAGE_REDIRECT) )
		{
			EnableIt = FALSE;
		}
	}
	
	pCmdUI->Enable(EnableIt);
	
	// If this is the Reply menu item, then continue the updating so that the text
	// can be set if necessary
	if (pCmdUI->m_nID == ID_MESSAGE_REPLY)
		pCmdUI->ContinueRouting();
	if (pCmdUI->m_nID == ID_MESSAGE_REPLY_ALL)
		pCmdUI->ContinueRouting();
}


void CTocFrame::OnUpdateSendImmediately(CCmdUI * pCmdUI)
{
	BOOL		bEnableIt = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && m_pSumListBox)
	{
		int		nSelCount = m_pSumListBox->GetSelCount();

		// Send Immediate/Queue for Delivery can only be done on composition messages that are sendable
		if ( (pTocDoc->m_Type == MBT_OUT) && (nSelCount > 0) )
		{
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
					if (Sum && Sum->IsSendable())
					{
						bEnableIt = TRUE;
						break;
					}
					nSelCount--;
				}
			}
		}
	}
	
	pCmdUI->Enable(bEnableIt);

	// Need to continue routing so that the main window can set the text of the menu item
	// depending on the Send Immediate switch
	pCmdUI->ContinueRouting();
}


void CTocFrame::OnUpdateChangeQueueing(CCmdUI * pCmdUI)
{
	BOOL		bEnableIt = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && m_pSumListBox)
	{
		int		nSelCount = m_pSumListBox->GetSelCount();

		// Change Queueing can only be done on composition messages that are sendable
		if ( (pTocDoc->m_Type == MBT_OUT) && (nSelCount > 0) )
		{
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					if (m_pSumListBox->GetItemDataPtr(i)->IsSendable())
					{
						bEnableIt = TRUE;
						break;
					}
					nSelCount--;
				}
			}
		}
	}
	
	pCmdUI->Enable(bEnableIt);
}


void CTocFrame::OnUpdateNeedSelNoOut(CCmdUI * pCmdUI)
{
	BOOL		bEnable = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && m_pSumListBox)
	{
		int		nSelCount = m_pSumListBox->GetSelCount();
		int		state = -1;
		
		switch (pCmdUI->m_nID)
		{
			case ID_MESSAGE_STATUS_UNREAD:		state = MS_UNREAD;		break;
			case ID_MESSAGE_STATUS_READ:		state = MS_READ;		break;
			case ID_MESSAGE_STATUS_REPLIED:		state = MS_REPLIED;		break;
			case ID_MESSAGE_STATUS_FORWARDED:	state = MS_FORWARDED;	break;
			case ID_MESSAGE_STATUS_REDIRECTED:	state = MS_REDIRECT;	break;
			case ID_MESSAGE_STATUS_RECOVERED:	state = MS_RECOVERED;	break;
			default: ASSERT(FALSE);
		}
		
		if ( (state != -1) && (pTocDoc->m_Type != MBT_OUT) && (nSelCount > 0) )
		{
			bEnable = TRUE;
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
					if (Sum && Sum->m_State != state)
					{
						if (Sum->CantEdit())
						{
							bEnable = FALSE;
							break;
						}
					}
					nSelCount--;
				}
			}
		}
	}

	pCmdUI->Enable(bEnable);
}


void CTocFrame::OnUpdateToggleStatus(CCmdUI * pCmdUI)
{
	BOOL		bEnable = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && m_pSumListBox)
	{
		int		nSelCount = m_pSumListBox->GetSelCount();
		
		if ( (pTocDoc->m_Type != MBT_OUT) && (nSelCount > 0) )
		{
			bEnable = TRUE;
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
					if (Sum && Sum->m_State != MS_UNREAD && Sum->m_State != MS_READ)
					{
						bEnable = FALSE;
						break;
					}
					nSelCount--;
				}
			}
		}
	}

	pCmdUI->Enable(bEnable);
}


void CTocFrame::OnUpdateSendable(CCmdUI* pCmdUI)
{
	BOOL		bEnable = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && 	m_pSumListBox)
	{
		int		nSelCount = m_pSumListBox->GetSelCount();
		int		state = -1;
		
		switch (pCmdUI->m_nID)
		{
			case ID_MESSAGE_STATUS_SENDABLE:	state = MS_SENDABLE;	break;
			case ID_MESSAGE_STATUS_QUEUED:		state = MS_QUEUED;		break;
			case ID_MESSAGE_STATUS_TIME_QUEUED:	state = MS_TIME_QUEUED;	break;
			default: ASSERT(FALSE);
		}
		
		if ( (state != -1) && (state != MS_TIME_QUEUED) &&
			 (pTocDoc->m_Type == MBT_OUT) && (nSelCount > 0) )
		{
			bEnable = TRUE;
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
					if (Sum && Sum->m_State != state)
					{
						if (Sum->IsSendable() == FALSE)
						{
							bEnable = FALSE;
							break;
						}
					}
					nSelCount--;
				}
			}
		}
	}

	pCmdUI->Enable(bEnable);
}


void CTocFrame::OnUpdateSentUnsent(CCmdUI* pCmdUI)
{
	BOOL		bEnable = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && 	m_pSumListBox)
	{
		int		nSelCount = m_pSumListBox->GetSelCount();
		int		state = -1;

		switch (pCmdUI->m_nID)
		{
			case ID_MESSAGE_STATUS_SENT:		state = MS_SENT;		break;
			case ID_MESSAGE_STATUS_UNSENT:		state = MS_UNSENT;		break;
			default: ASSERT(FALSE);
		}

		if ( (state != -1) && (nSelCount > 0) )
		{
			bEnable = TRUE;
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
					if (Sum && Sum->m_State != state)
					{
						// Allow recovered messages to be changed
						if (Sum->m_State != MS_RECOVERED && Sum->CantEdit() == FALSE)
						{
							bEnable = FALSE;
							break;
						}
					}
					nSelCount--;
				}
			}
		}
	}

	pCmdUI->Enable(bEnable);
}


void CTocFrame::OnUpdateJunkCommands(CCmdUI* pCmdUI)
{
	// Junk menus not available in light.
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;
	
	BOOL	bEnable = FALSE;

	if (m_pSumListBox)
	{
		//	Make sure that some messages are selected
		int		nSelCount = m_pSumListBox->GetSelCount();
		bEnable = (nSelCount > 0);

		if ( bEnable && (pCmdUI->m_nID != ID_RECHECK_JUNK) )
		{
			//	Make sure that at least one of the messages selected is not a composition
			bEnable = FALSE;
			for (int i = m_pSumListBox->GetCount() - 1; nSelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary *		pSum = m_pSumListBox->GetItemDataPtr(i);
					if ( pSum && !pSum->IsComp() )
					{
						bEnable = TRUE;
						break;
					}
					nSelCount--;
				}
			}

			if ( bEnable && !GetIniShort(IDS_INI_ALWAYS_ENABLE_JUNK) )
			{
				CTocDoc *	pTocDoc = GetTocDoc();
				
				if (pTocDoc)
				{
					if ( (pCmdUI->m_nID == ID_JUNK) && (pTocDoc->IsJunk()) )
					{
						// We are in the Junk mailbox: disable "Junk" menu.
						bEnable = FALSE;
					}
					else if ( (pCmdUI->m_nID == ID_NOT_JUNK) && (!pTocDoc->IsJunk()) )
					{
						// We are not in the Junk mailbox: disable "Not Junk" menu.
						bEnable = FALSE;
					}
				}
			}
		}
	}

	pCmdUI->Enable(bEnable);
}


//
// Added by JOK - 4/20/98
// Disable these for IMAP!!
//
void CTocFrame::OnUpdateServerStatus(CCmdUI* pCmdUI)
{
	CTocDoc *	pTocDoc = GetTocDoc();

	if ( pTocDoc && pTocDoc->IsImapToc() )
		pCmdUI->Enable(FALSE);
	else
		OnUpdateNeedSel(pCmdUI);
}


void CTocFrame::Print(BOOL bShowDialog)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	INT				nToPrint;
	INT				i;
	BOOL			bWasOpen;
	INT				nTotalCount;
	CSummary*		pSummary;
	QCProtocol*		pProtocol;
	BOOL			bError;
	const DWORD		dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION |
								(bShowDialog? 0 : PD_RETURNDEFAULT);
	CPrintDialog	theDialog(TRUE, dwFlags, AfxGetMainWnd());
	
	nTotalCount = m_pSumListBox->GetCount();
	nToPrint = m_pSumListBox->GetSelCount();
	
	if (!nToPrint)
		return;

	const int PDResult = AfxGetApp()->DoPrintDialog(&theDialog);
	if (bShowDialog && PDResult != IDOK)
		return;

	EnableWindow(FALSE);

	CountdownProgress( CRString( IDS_PRINTING_MESSAGES_LEFT ), nToPrint );
	bError = FALSE;

	CMDIChild::m_nCmdShow = SW_HIDE;

	// Start of the main loop going through all of the list items
	for( i = 0; !bError && nToPrint &&  ( i < nTotalCount ); i++ )
	{
		if ( m_pSumListBox->GetSel(i) <= 0 )
		{
			continue;
		}

		nToPrint --;

		pSummary = m_pSumListBox->GetItemDataPtr(i);
		
		if ( !pSummary )
		{
			continue;
		}

		// If the message window was already open, then we don't want to Display() it because
		// that will bring it to the front of the window list.  However, if the message window
		// isn't open, then Display() will create one for us, and the assignment above to set
		// CMDIChild::m_nCmdShow to SW_HIDE will make sure that the window doesn't get shown.
		
		bWasOpen =( pSummary->m_FrameWnd != NULL );
		
		if( !bWasOpen )
		{
			pSummary->Display();
		}

		pProtocol = QCProtocol::QueryProtocol( QCP_QUIET_PRINT, pSummary->m_FrameWnd );
		
		if(GetIniLong(IDS_INI_PRINT_DELAY_BETWEEN))
			Sleep(GetIniLong(IDS_INI_PRINT_DELAY_BETWEEN));
		
		// Ok now that we have a valid view...let get on with the printing...
		if( pProtocol )
		{
			Progress( -1, pSummary->GetTitle(), -1 );
			bError = !pProtocol->QuietPrint();			
		}
				
		// If we were forced to create the message window becuase it wasn't open before we
		// started printing, then shut it down now
		if ( !bWasOpen )
		{
			pSummary->GetMessageDoc()->OnCloseDocument();
		}

		DecrementCountdownProgress();
	}
	
	CMDIChild::m_nCmdShow = -1;

	EnableWindow(TRUE);
	CloseProgress();
	m_pSumListBox->SetFocus();
}


void CTocFrame::OnFilePrint()
{
	Print(TRUE);
}


void CTocFrame::OnFilePrintOne()
{
	Print(FALSE);
}


//
//	CTocFrame::RecheckMessagesForJunk()
//
//	Iterate over the selected messages and check each message for junk.
//
void CTocFrame::OnRecheckMessagesForJunk()
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	CTocDoc		*pTocDocSrc = GetTocDoc();
	CTocDoc		*pTocDocDest = NULL;
	CTocView	*pTocViewDest = NULL;

	// Get the count of selected messages.
	int			 iSelCount = m_pSumListBox->GetSelCount();
	if (iSelCount == 0)
	{
		return;
	}

	if (pTocDocSrc->IsJunk())
	{
		// Since we might potentially be filtering some messages we don't really
		// have a single destination mailbox, but the In mailbox is the default
		// destination location so note that here.
		pTocDocDest = GetInToc();
		pTocViewDest = pTocDocDest->GetView();
	}
	else
	{
		// Destination is the Junk mailbox.
		pTocDocDest = GetJunkToc();
		pTocViewDest = pTocDocDest->GetView();
	}

	// Turn off redrawing.
	m_pSumListBox->SetRedraw(FALSE);
	if (pTocViewDest)
	{
		pTocViewDest->m_SumListBox.SetRedraw(FALSE);
	}

	// Start filtering in case rescoring needs to use filters.
	CFilterActions	 filt;
	if (!filt.StartFiltering())
	{
		return;
	}
		
	CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), iSelCount);

	CObArray	oaABHashes;

	// Generate the address book hashes once for the entire loop
	CFilter::GenerateHashes(&oaABHashes);

	// Iterate over the selection and rescore each message.
	for (int i = m_pSumListBox->GetCount() - 1; (i >= 0) && iSelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			CSummary		*pSum = m_pSumListBox->GetItemDataPtr(i);

			// Why would anyone ever declare an outgoing message junk?
			if (pTocDocSrc->m_Type == MBT_OUT)
			{
				CCompMessageDoc	*pDoc = (CCompMessageDoc*)pSum->FindMessageDoc();
				if (pDoc && pDoc->m_HasBeenSaved == FALSE)
				{
					pDoc->OnSaveDocument(NULL);
				}
			}

			// Set the new state for this message.
			if (pTocDocSrc->IsImapToc())
			{
				pTocDocSrc->ImapRecheckMessageForJunk(pSum, &oaABHashes);
			}
			else
			{
				CJunkMail::ReprocessOne(pSum, &filt, &oaABHashes);
			}
			iSelCount--;

			DecrementCountdownProgress();
		}
	}

	// Done with filtering.
	filt.EndFiltering();

	// Turn redrawing back on.
	m_pSumListBox->SetRedraw(TRUE);
	if (pTocViewDest)
	{
		pTocViewDest->m_SumListBox.SetRedraw(TRUE);
	}

	// Save the appropriate TOCs.
	pTocDocSrc->Write();
	pTocDocDest->Write();
}

//
//	CTocFrame::OnDeclareMessagesJunk()
//
//	Call the function that iterates over the selected messages
//	specifying that the selected messages are junk.
//
void CTocFrame::OnDeclareMessagesJunk()
{
	SetJunkStatus(true);
}

//
//	CTocFrame::OnDeclareMessagesNotJunk()
//
//	Call the function that iterates over the selected messages
//	specifying that the selected messages are not junk.
//
void CTocFrame::OnDeclareMessagesNotJunk()
{
	SetJunkStatus(false);
}


//
//	CTocView::SetJunkStatus()
//
//	Iterate over the selected messages and if they are in the junk
//	mailbox declare them not junk, otherwise declare them junk.
//
//	Parameters:
//		bJunk[in] - Flag to indicate if the selection should be
//			considered junk.
//
void CTocFrame::SetJunkStatus(bool bJunk)
{
	if ( !UsingFullFeatureSet() )
		return;
	
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	// Get the count of selected messages.
	int			 iSelCount = m_pSumListBox->GetSelCount();
	if (iSelCount == 0)
	{
		return;
	}

	CTocDoc		*pTocDocSrc = GetTocDoc();

	if (pTocDocSrc->IsImapToc())
	{
		// This is an IMAP mailbox: hand off to IMAP code.
#ifdef IMAP4
		// Pass this to the IMAP command object.
		QCMailboxCommand	*pImapFolder = g_theMailboxDirector.FindByPathname((const char*)pTocDocSrc->GetMBFileName());
		if (pImapFolder)
		{
			pImapFolder->Execute(bJunk ? CA_JUNK : CA_NOT_JUNK, NULL);
		}
		else
		{
			ASSERT(0);
		}

		return;
#endif // IMAP4
	}
	else
	{
		// This is a regular mailbox: handle it here.
		CTocDoc		*pTocDocDest = NULL;
		CTocView	*pTocViewDest = NULL;

		if (pTocDocSrc->m_Type == MBT_JUNK)
		{
			// Since we might potentially be filtering some messages we don't really
			// have a single destination mailbox, but the In mailbox is the default
			// destination location so note that here.
			pTocDocDest = GetInToc();
			pTocViewDest = pTocDocDest->GetView();
		}
		else
		{
			// We are unjunking: destination is the Junk mailbox.
			pTocDocDest = GetJunkToc();
			pTocViewDest = pTocDocDest->GetView();
		}

		// Turn off redrawing.
		m_pSumListBox->SetRedraw(FALSE);
		if (pTocViewDest)
		{
			pTocViewDest->m_SumListBox.SetRedraw(FALSE);
		}

		// Start filtering in case changing status needs to use filters.
		CFilterActions	 filt;
		if (!filt.StartFiltering())
		{
			return;
		}
			
		CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), iSelCount);

		// Hash the address book for translator use and potentially
		// determining if we need to add not junked senders to the AB.
		CObArray	oaABHashes;

		CFilter::GenerateHashes(&oaABHashes);

		// Determine if we're adding Not Junk'ed senders to the address book.
		bool		bAddNotJunkedSendersToAB = !bJunk &&
											   GetIniShort(IDS_INI_ADDBOOK_IS_WHITELIST) &&
											   GetIniShort(IDS_INI_ADD_NONJUNK_TO_ADDBOOK);

		// Iterate over the selection and individually change the junk status
		// of each message in the selection.
		for (int i = m_pSumListBox->GetCount() - 1; (i >= 0) && iSelCount; i--)
		{
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary		*pSum = m_pSumListBox->GetItemDataPtr(i);

				if ( !pSum->IsComp() )
				{
					// Set the new state for this message.
					CJunkMail::DeclareJunk(pSum, bJunk, bAddNotJunkedSendersToAB, &oaABHashes, &filt);
				}

				iSelCount--;

				DecrementCountdownProgress();
			}
		}

		// Done with filtering.
		filt.EndFiltering();

		// Turn redrawing back on.
		m_pSumListBox->SetRedraw(TRUE);
		if (pTocViewDest)
		{
			pTocViewDest->m_SumListBox.SetRedraw(TRUE);
		}

		// Save the appropriate TOCs.
		pTocDocSrc->Write();
		pTocDocDest->Write();
	}
}


void CTocFrame::OnMakeNickname()
{
	MakeNickname();
}


////////////////////////////////////////////////////////////////////////
// ComposeMessage
//
// Helper routines, etc..
////////////////////////////////////////////////////////////////////////

//	Typedef for CTocFrame::ComposeMessage and helper routines
typedef std::set<CString>					UniqueAddressesT;

//	Helper routine for CTocFrame::ComposeMessage which adds
//	a given address to the recipients string if it isn't already
//	in the recipients set.
void
AddAddressIfUnique(
	const char *			in_szAddress,
	CString &				io_szRecipients,
	UniqueAddressesT &		io_recipientsSet)
{
	//	Avoid duplicates
	CString		szBareAddress = in_szAddress;
	szBareAddress.MakeLower();
	szBareAddress = StripAddress( szBareAddress.GetBuffer(0) );
	
	UniqueAddressesT::iterator		recipientsIterator = io_recipientsSet.find(szBareAddress);

	if ( recipientsIterator == io_recipientsSet.end() )
	{
		//	Separate addresses with ", "
		if ( !io_szRecipients.IsEmpty() )
			io_szRecipients += ", ";

		//	Add on this next addresses
		io_szRecipients += in_szAddress;

		//	Add it to our set so that we can avoid duplicates
		io_recipientsSet.insert(szBareAddress);
	}
}

//	Helper routine for CTocFrame::ComposeMessage which adds
//	addesses from a header field to a list of addresses.
void
AddAddressesToList(
	const char *			in_szAddressSource,
	CStringList &			io_recipientsList)
{
	if (!in_szAddressSource)
		return;
	
	const char *					szAddrStart = in_szAddressSource;
	const char *					szAddrEnd;
	
	while (*szAddrStart)
	{
		szAddrEnd = FindAddressEnd(szAddrStart, FALSE);
		if (!szAddrEnd)
			break;
		
		CString		szAddress(szAddrStart, szAddrEnd - szAddrStart);
		szAddress.TrimLeft();
		szAddress.TrimRight();

		io_recipientsList.AddTail(szAddress);

		if (*szAddrEnd)
			szAddrStart = szAddrEnd + 1;
		else
			break;
	}
}

//	Helper class for CTocFrame::ComposeMessage which keeps track of the number of
//	times a given string (e.g. subject or personality) appears.
//	The reason to use this rather than a raw int, is that you can directly
//	use the map's [] without worrying whether or not the entry previously
//	existed. The StringCounter constructor will ensure that a new count
//	is initialized to 0.
class StringCounter
{
  public:
					StringCounter() : nCount(0) {}
	void			operator++(int) { nCount++; }
					operator int() { return nCount; }

  protected:
	int				nCount;
};


////////////////////////////////////////////////////////////////////////
// ComposeMessage [public]
//
// Public method for doing a Reply, Forward, or Redirect.
////////////////////////////////////////////////////////////////////////
void CTocFrame::ComposeMessage
(
	UINT nID, 
	const char* pszDefaultTo, 
	const char* pszStationery /*= NULL*/,
	const char* pszPersona /*= NULL*/
)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}
	
	CCursor cursor;
	int		nSelected = m_pSumListBox->GetSelCount();

	bool	bIsReply = (nID == ID_MESSAGE_REPLY) || (nID == ID_MESSAGE_REPLY_ALL);

	if ( (nSelected > 1) && bIsReply &&
		 UsingFullFeatureSet() && !GetIniShort(IDS_INI_MULT_REPLIES_FOR_MULT_SEL) )
	{
		//	Get the selected summaries
		SummaryArrayT		currentPreviewSummaryArray;
		GetSelectedSummaries(currentPreviewSummaryArray);

		int					nNumSummaries = currentPreviewSummaryArray.GetSize();

		if (nNumSummaries > 0)
		{
			//	Check the number of summaries against the warn threshold
			if ( nNumSummaries > GetIniLong(IDS_INI_CC_MULTIPLE_REPLY_WARN_THRESHOLD) )
			{
				//	Warn the user if replying to lots of messages - by default
				//	the threshold is 50 (well above default max number of
				//	messages to concentrate at once, which is 20).
				//
				//	I'm not bothering with a way to turn this warning off
				//	(for now), although the user can always increase the
				//	ConConMultipleReplyWarnThreshold. I don't expect that this
				//	will be a common or annoying warning since it should be rare
				//	to reply to lots of messages all at once (except by
				//	accident - in which case the warning is justified).
				if ( WarnYesNoDialog(0, IDS_CON_CON_WARN_BIG_REPLY, nNumSummaries) != IDOK )
					return;
			}

			//	Message data used for each of the selected messages
			CSummary *				pSum;
			CMessageDoc *			pMessageDoc;
			bool					bDocWasCreated;
			char *					szFullMessage;
			char *					szQuotedBody;
			CString					szBodyToQuote;
			const char *			pStartBody;
			char *					szAddress;
			CString					szBareAddress;
			char *					szOldReferences;
			char *					szOldMessageId;

			//	Record keeping to track the use of subjects, recipients, priority, and References
			typedef std::map<CString, StringCounter>	StringCountsT;
			StringCountsT								subjectCounts;
			UniqueAddressesT							recipientsSet;
			UniqueAddressesT::iterator					recipientsIterator;
			CStringList									potentialCcRecipientsList;
			int											nPriorityCountsArr[MSP_LOWEST] = { 0 };
			CString										szAllReferences;
			CStringArray								szMessageIdsArr;
			CStringArray								szReferencesArr;

			//	Message data for the composition message that we'll be creating
			CString					szPersona;
			CString					szToRecipients;
			CString					szSubject;
			CString					szQuotedBodies;
			CString					szNewInReplyTo;
			URIMap					newEmbeddedImageMap;

			//	x-html tag
			CString					szXHtmlTag;
			CRString				szXHtml(IDS_MIME_XHTML);

			//	Cache a bunch of settings that we'll need
			bool					bReplyToBecomesCc = (GetIniShort(IDS_INI_REPLY_TO_TO_CC) != 0);
			bool					bAddFromLinesToHistory = (GetIniShort(IDS_INI_ADD_FROM_LINES_TO_HISTORY) != 0);
			BOOL					bDisplayLocalTime = GetIniShort(IDS_INI_LOCAL_TIME_DISPLAY);
			bool					bStripMe = (GetIniShort(IDS_INI_INCLUDE_SELF) == 0);
			bool					bCopyPriority = (GetIniShort(IDS_INI_COPY_PRIORITY) != 0);
			
			CString					szReplyAllAttribution;
			GetIniString(IDS_INI_REPLY_ALL_ATTRIBUTION, szReplyAllAttribution);

			CString					szSubjectSuffix;
			GetIniString(IDS_INI_CC_MULTIPLE_SUBJECTS_SUFFIX, szSubjectSuffix);

			//	Format the x-html tag
			szXHtmlTag.Format("<%s>\r\n", szXHtml);

			//	Initialize the body with the x-html tag
			szQuotedBodies = szXHtmlTag;

			//	Watch for user cancelling via escape key
			BOOL					bEscapePressed = FALSE;
			
			if (pszPersona)
			{
				szPersona = pszPersona;
			}
			else
			{
				//	No personality was specified by the caller, so determine the most
				//	used personality among the selected messages.
				StringCountsT				personalityCounts;
				
				for (int i = 0; (i < nNumSummaries) && !bEscapePressed; i++)
				{
					pSum = currentPreviewSummaryArray[i];
					
					//	Count the number of times each personality is used.
					//	The personality in the summary record takes precedence
					//	over any X-Persona value, so just use that.
					personalityCounts[ pSum->GetPersona() ]++;

					//	Check for escape key presses - important to use EscapePressed
					//	and not AsyncEscapePressed so that progress and other windows
					//	update correctly.
					bEscapePressed = EscapePressed();
				}

				if (!bEscapePressed)
				{
					if (personalityCounts.size() == 1)
					{
						//	There's only one personality - so just use the personality from the first summary
						szPersona = currentPreviewSummaryArray[0]->GetPersona();
					}
					else
					{
						//	Find the largest personality count
						int				nLargestPersonalityCount = 0;
						for ( StringCountsT::iterator persCountsIterator = personalityCounts.begin();
							  persCountsIterator != personalityCounts.end();
							  persCountsIterator++ )
						{
							if ( (*persCountsIterator).second > nLargestPersonalityCount )
								nLargestPersonalityCount = (*persCountsIterator).second;
						}
						
						//	Find the first personality that was used the most times
						//	(i.e. first wins if there was a tie)
						for (int i = 0; i < nNumSummaries; i++)
						{
							szPersona = currentPreviewSummaryArray[i]->GetPersona();
							
							if (personalityCounts[szPersona] == nLargestPersonalityCount)
								break;
						}
					}
				}
			}

			//	We now know the Personality for this message...
			//	Switch to it so the stuff that follows is Personality specific
			CString		szSavePersonality = g_Personalities.GetCurrent();
			g_Personalities.SetCurrent(szPersona);

			//	Start the progress (will only be displayed if enough time elapses)
			CountdownProgress(CRString(IDS_CON_CON_REPLY_MESSAGES_LEFT), nNumSummaries);

			//	Prepare the Message Ids and References arrays
			szMessageIdsArr.SetSize(nNumSummaries);
			szReferencesArr.SetSize(nNumSummaries);

			//	Preallocate space in szAllReferences that will likely fit 20 messages.
			//	This is solely for better efficiency. CString will still grow as necessary.
			szAllReferences.GetBufferSetLength(1024);

			for (int i = 0; (i < nNumSummaries) && !bEscapePressed; i++)
			{
				//	Get the summary - we know it will be non-NULL because of the
				//	way GetSelectedSummaries operates.
				pSum = currentPreviewSummaryArray[i];

				//	Get the CMessageDoc if we can. Remember if we created it so
				//	that we can clean up after ourselves.
				pMessageDoc = pSum->GetMessageDoc(&bDocWasCreated);
				
				if (pMessageDoc)
				{
					//	Get the full message (headers and all)
					szFullMessage = pMessageDoc->GetFullMessage();

					//	Release the message doc if we created it
					if (bDocWasCreated)
						pSum->NukeMessageDocIfUnused(pMessageDoc);
					
					if (szFullMessage)
					{
						//	Find the address that we're supposed to reply to for this message
						szAddress = HeaderContents(IDS_HEADER_REPLY_TO, szFullMessage);
						if (!szAddress)
						{
							szAddress = HeaderContents(IDS_HEADER_FROM, szFullMessage);

							//	Add it to the history list if the INI entry is on
							if (szAddress && bAddFromLinesToHistory)
							{
								AutoCompList* pACL = GetAutoCompList();
								if (pACL)
									pACL->Add(szAddress, false, true);
							}
						}

						//	Add the address to the To: recipients if it's not already there
						if (szAddress)
							AddAddressIfUnique(szAddress, szToRecipients, recipientsSet);

						//	Free the memory allocated by HeaderContents
						delete [] szAddress;

						if (ID_MESSAGE_REPLY_ALL == nID)
						{
							CStringList		recipientsList;
							
							//	We're doing Reply to All, add the contents of the To: field
							char *			szOldTo = HeaderContents(IDS_HEADER_TO, szFullMessage);

							if (szOldTo && *szOldTo)
							{
								::TrimWhitespaceMT(szOldTo);

								if (bReplyToBecomesCc)
								{
									//	To: becomes Cc: so add the To: recipients to our
									//	list of Cc: recipients that we'll process later
									//	(allows us to prefer inclusion in To: and avoid
									//	duplicates).
									AddAddressesToList(szOldTo, potentialCcRecipientsList);
								}
								else
								{
									//	To: stays To:, so build the list of recipients
									//	add any unique ones to our To: recipient string.
									AddAddressesToList(szOldTo, recipientsList);
									while ( !recipientsList.IsEmpty() )
										AddAddressIfUnique(recipientsList.RemoveHead(), szToRecipients, recipientsSet);
								}
							}
							
							//	Free the memory allocated by HeaderContents
							delete [] szOldTo;

							//	We're doing Reply to All, add the contents of the Cc: field
							char *		szOldCc = HeaderContents(IDS_HEADER_CC, szFullMessage);

							if (szOldCc && *szOldCc)
							{
								::TrimWhitespaceMT(szOldCc);

								//	Add the Cc: recipients to our list of Cc: recipients
								//	that we'll process later (allows us to prefer
								//	inclusion in To: and avoid duplicates).
								AddAddressesToList(szOldCc, potentialCcRecipientsList);
							}

							//	Free the memory allocated by HeaderContents
							delete [] szOldCc;
						}

						//	Get the message Id of the current message
						szOldMessageId = HeaderContents(IDS_HEADER_MESSAGE_ID, szFullMessage);
						if (szOldMessageId)
						{
							//	In-Reply-To: is easy - just accumulate all the message Ids
							//	separated by spaces
							if ( !szNewInReplyTo.IsEmpty() )
								szNewInReplyTo += " ";
							szNewInReplyTo += szOldMessageId;

							//	Stash the Message Id so that we can format the new References later
							szMessageIdsArr[i] = szOldMessageId;
						}

						//	Free the memory allocated by HeaderContents
						delete [] szOldMessageId;

						//	Get the References of the current message
						szOldReferences = HeaderContents(IDS_HEADER_REFERENCES, szFullMessage);
						if (szOldReferences)
						{
							//	Stash the old References so that we can format the new References later
							szReferencesArr[i] = szOldReferences;
						}
						else
						{
							//	No "References:" field in the original message
							//
							//	RFC 2822 says:
							//	If the parent message does not contain a "References:" field
							//	but does have an "In-Reply-To:" field containing a single message
							//	identifier, then the "References:" field will contain the contents
							//	of the parent's "In-Reply-To:" field followed by the contents of
							//	the parent's "Message-ID:" field (if any).
							szOldReferences = HeaderContents(IDS_HEADER_IN_REPLY_TO, szFullMessage);

							if (szOldReferences)
							{
								char *		pStartMessageId = strchr(szOldReferences, '<');
								
								//	Use the "In-Reply-To:" field from the original message if
								//	it contains exactly one Message Id (as determined by only
								//	one occurence of '<').
								//	Stash so that we can format the new References later.
								if ( pStartMessageId && !strchr(pStartMessageId+1, '<') )
									szReferencesArr[i] = szOldReferences;
							}
						}

						if ( !szReferencesArr[i].IsEmpty() )
						{
							//	We found something that is acting as the References of the
							//	current message. Accumulate all the References into one
							//	big fat string that contains all References to enable
							//	easy uniqueness detection later.
							if ( !szAllReferences.IsEmpty() )
								szAllReferences += " ";
							szAllReferences += szReferencesArr[i];
						}

						//	Free the memory allocated by HeaderContents
						delete [] szOldReferences;
						
						//	Count the number of times each Subject: occurs.
						//	Process it so that prefixes and case don't interfere
						//	with our ability to correctly match.
						szSubject = RemoveSubjectPrefixMT(currentPreviewSummaryArray[i]->m_Subject);
						szSubject.MakeLower();
						subjectCounts[szSubject]++;

						if (bCopyPriority)
						{
							//	Count the number of times each priority occurs
							int			nPriority = MSP_NORMAL;
							char *		szOldPrior = HeaderContents(IDS_HEADER_PRIORITY_MATCH, szFullMessage);
							
							if (szOldPrior)
							{
								nPriority = CSummary::ParsePriority(szOldPrior);

								//	Free the memory allocated by HeaderContents
								delete [] szOldPrior;
							}
							else if ((szOldPrior = HeaderContents(IDS_HEADER_IMPORTANCE_MATCH, szFullMessage)))
							{
								nPriority = CSummary::ParseImportance(szOldPrior);

								//	Free the memory allocated by HeaderContents
								delete [] szOldPrior;
							}

							//	ParsePriority makes sure that the value is legal
							nPriorityCountsArr[nPriority-MSP_HIGHEST]++;
						}

						//	Add any embedded image information onto our URIMap
						newEmbeddedImageMap.BuildURIMap(szFullMessage, false);

						//	Replying does not include the attachment
						StripAttachFromBody(szFullMessage, IDS_ATTACH_CONVERTED);

						//	Concentrate with the special multiple reply context
						bool	bWasConcentrated = ContentConcentrator::Instance()->ConcentrateMessage(
															ContentConcentrator::kCCMultipleReplyContext, pSum,
															szFullMessage, szBodyToQuote );
						
						//	Find the start of the body in either the concentrated
						//	body or the original body depending on whether or not
						//	we concentrated.
						if (bWasConcentrated)
							pStartBody = ::FindBody(szBodyToQuote);
						else
							pStartBody = ::FindBody(szFullMessage);

						//	Get the body as HTML stripping HTML, stripping document
						//	level tags, and not relaxing local file ref stripping.
						//	Converting everything to HTML allows us to combine the
						//	multiple quotes messages in a common format.
						::GetBodyAsHTML(szBodyToQuote, pStartBody, true, true, false);

						//	Get the date for the attribution
						CString		szAttribution;
						char		szDateBuf[64];
						pSum->SetDateString(szDateBuf, sizeof(szDateBuf), bDisplayLocalTime, TRUE);

						LPCSTR		rglpsz[2];
						rglpsz[0] = szDateBuf;
						rglpsz[1] = pSum->m_From;

						CString		szFromSelf;

						if ( pSum->IsComp() )
						{
							//	It's a comp so the attribution should actually be the
							//	current personality's realname rather than the "From"
							//	field (which actually contains the To: field contents).
							GetPersRealName(szFromSelf);
							rglpsz[1] = szFromSelf;
						}

						//	Use Reply All style attributions to identify quoted material
						AfxFormatStrings(szAttribution, szReplyAllAttribution, rglpsz, 2);
						szAttribution += "\r\n";

						//	Quote the HTML body that we retrieved with the attribution
						//	that we just created. Pass in false, false as the last two
						//	parameters so that QuoteText trusts that we're giving it
						//	HTML and so that it doesn't add x-html tags itself.
						szQuotedBody = ::QuoteText(szBodyToQuote, FALSE, IS_HTML, szAttribution, false, false);

						if (szQuotedBody)
						{
							//	Separate quoted bodies with a blank line
							if (i > 0)
								szQuotedBodies += "\r\n<br>\r\n";

							//	Append the current quoted body
							szQuotedBodies += szQuotedBody;
						}

						//	Free the memory allocated by GetFullMessage
						delete [] szFullMessage;

						//	Free the memory allocated by QuoteText
						delete [] szQuotedBody;
					}
				}

				//	We've completed one message - update the progress
				DecrementCountdownProgress();
				
				//	Check for the escape key and update windows
				bEscapePressed = EscapePressed();
			}

			//	Now calculate the References
			CString		szNewReferences;
			if (!bEscapePressed)
			{
				for (int i = 0; i < nNumSummaries; i++)
				{
					if ( !szMessageIdsArr[i].IsEmpty() )
					{
						int		nFoundIndex = szAllReferences.Find( szMessageIdsArr[i] );
						
						if (nFoundIndex == -1)
						{
							//	The current message is not redundant - it is not contained
							//	in the references of any other message.
							if ( !szReferencesArr[i].IsEmpty() )
							{
								//	Append the References of the current message
								if ( !szNewReferences.IsEmpty() )
									szNewReferences += " ";
								szNewReferences += szReferencesArr[i];
							}
							
							//	Append the Message Id of the current message
							if ( !szNewReferences.IsEmpty() )
								szNewReferences += " ";
							szNewReferences += szMessageIdsArr[i];
						}
					}

					//	Check for the escape key and update windows
					bEscapePressed = EscapePressed();
				}
			}

			if (!bEscapePressed)
			{
				//	Update the progress to indicate that we're
				//	"Preparing reply for display...". Unfortunately there's no way
				//	I can think of at the moment to update the progress bar further.
				Progress( CRString(IDS_CON_CON_COMPOSING_REPLY) );
				
				//	Now that the final contents of To: is determined, add any
				//	that we wanted to put in Cc: if they don't already appear
				//	in our recipients.
				CString			szCcRecipients;
				while ( !potentialCcRecipientsList.IsEmpty() )
					AddAddressIfUnique(potentialCcRecipientsList.RemoveHead(), szCcRecipients, recipientsSet);
				
				//	Strip any references to the user if appropriate (with helper routines
				//	it's easier to do this after the fact like this, rather than above).
				if (bStripMe)
				{
					StripMe(szToRecipients.GetBuffer(0), TRUE);
					szToRecipients.ReleaseBuffer();
					
					StripMe(szCcRecipients.GetBuffer(0), TRUE);
					szCcRecipients.ReleaseBuffer();
				}
				
				if (subjectCounts.size() == 1)
				{
					//	There's only one subject - so just use the subject from the first summary
					szSubject = currentPreviewSummaryArray[0]->m_Subject;
				}
				else
				{
					//	Find the largest subject count
					int				nLargestSubjectCount = 0;
					for ( StringCountsT::iterator subCountsIterator = subjectCounts.begin();
						  subCountsIterator != subjectCounts.end();
						  subCountsIterator++ )
					{
						if ( (*subCountsIterator).second > nLargestSubjectCount )
							nLargestSubjectCount = (*subCountsIterator).second;
					}
					
					//	Find the first subject that was used the most times
					//	(i.e. first wins if there was a tie)
					for (int i = 0; i < nNumSummaries; i++)
					{
						szSubject = RemoveSubjectPrefixMT(currentPreviewSummaryArray[i]->m_Subject);
						szSubject.MakeLower();
						
						if (subjectCounts[szSubject] == nLargestSubjectCount)
						{
							szSubject = currentPreviewSummaryArray[i]->m_Subject;
							break;
						}
					}
					
					//	Append ", etc."
					szSubject += szSubjectSuffix;
				}

				int		nPriority = MSP_NORMAL;
				if (bCopyPriority)
				{
					//	Find the most used priority (highest priority wins in case of tie)
					int		nLargestPriorityCount = 0;
					for (int i = 0; i < (MSP_LOWEST-1); i++)
					{
						if (nPriorityCountsArr[i] > nLargestPriorityCount)
						{
							nLargestPriorityCount = nPriorityCountsArr[i];
							nPriority = i + 1;
						}
					}
				}
				
				//	Prepend the subject with "Re:" if the subject is empty or
				//	if the subject is non-empty, but doesn't start with Re:
				if ( szSubject.IsEmpty() ||
					 (RemovePrefixMT("Re:", szSubject) == static_cast<LPCTSTR>(szSubject)) )
				{
					CRString	szReplyPrefix(IDS_REPLY_PREFIX);
					
					if ( !szReplyPrefix.IsEmpty() )
						szSubject.Insert(0, szReplyPrefix);
				}
				
				//	Format the ending x-html tag
				szXHtmlTag.Format("\r\n</%s>", szXHtml);

				//	Append the ending x-html tag to the quoted bodies
				szQuotedBodies += szXHtmlTag;
				
				//	Get the CTocDoc of the original messages
				CTocDoc *			pTocDoc = GetTocDoc();
				
				//	Created the reply with all the information that we've gathered
				CCompMessageDoc *	comp = CSummary::ComposeMessage( pTocDoc, true, szToRecipients, szSubject,
																	 szCcRecipients, NULL, szQuotedBodies,
																	 pszStationery, szPersona, MS_REPLIED,
																	 NULL, NULL, IS_HTML, nPriority );

				if (comp)
				{
					//	Init the message map for the newly created message by transferring
					//	ownership of the embedded elements that we gathered for each message
					//	to which the user is replying
					comp->m_QCMessage.InitMap(newEmbeddedImageMap);
					
					//	Set the In-Reply-To and References headers for the newly created message
					//
					//	Calling SetHeaderLine instead of just setting the header directly doesn't
					//	really matter here because it only makes a difference when the header is
					//	visible, but just in case either of these is visible in the future go
					//	ahead and use SetHeaderLine just in case.
					comp->SetHeaderLine(HEADER_IN_REPLY_TO, szNewInReplyTo);
					comp->SetHeaderLine(HEADER_REFERENCES, szNewReferences);
				}
			}

			//	We're all done - close the progress
			CloseProgress();
			
			//	Restore the personality to the one before we started
			g_Personalities.SetCurrent(szSavePersonality);
		}
	}
	else
	{
		StartGroup();

		CString				szText;
		const char *		pSelText = NULL;

		//	Check to see if a single message is selected, because we can't support
		//	getting the selected text if multiple messages are selected.
		if (nSelected == 1)
		{
			CView *		pView = GetActiveView();

			if ( pView == m_wndSplitter.GetPane(1, 0) )
			{
				//	Query the protocol and get the selected text
				QCProtocol *	pQCProtocol = QCProtocol::QueryProtocol(QCP_GET_MESSAGE, pView);

				if (pQCProtocol)
				{
					if ( pQCProtocol->GetSelectedHTML(szText) )
						pSelText = szText;
				}
			}
		}

		MessageCascadeSpot = 0;
		::AsyncEscapePressed(TRUE);				// reset Escape key logic
		for (int i = m_pSumListBox->GetCount() - 1; nSelected && i >= 0; i--)
		{
			if (::AsyncEscapePressed())
				break;
			if (m_pSumListBox->GetSel(i) > 0)
			{
				m_pSumListBox->GetItemDataPtr(i)->ComposeMessage(nID, pszDefaultTo, FALSE, pszStationery, pszPersona, pSelText);
				nSelected--;
			}
		}
		MessageCascadeSpot = -1;

		EndGroup();
	}
}


////////////////////////////////////////////////////////////////////////
// OnComposeMessage [protected]
//
// Internal command handler for message map.
////////////////////////////////////////////////////////////////////////
BOOL CTocFrame::OnComposeMessage(UINT nID)
{
	ComposeMessage(nID, NULL);
	
	return (TRUE);
}


////////////////////////////////////////////////////////////////////////
// SendAgain [public]
//
// Public method for doing the Send Again command.
////////////////////////////////////////////////////////////////////////
void CTocFrame::SendAgain(const char* pszPersona /*= NULL*/)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}
	
	int Selected = m_pSumListBox->GetSelCount();

	StartGroup();
	
	MessageCascadeSpot = 0;
	::AsyncEscapePressed(TRUE);
	for (int i = m_pSumListBox->GetCount() - 1; Selected && i >= 0; i--)
	{
		if (::AsyncEscapePressed())
			break;
		if (m_pSumListBox->GetSel(i) > 0)
		{
			m_pSumListBox->GetItemDataPtr(i)->SendAgain(TRUE, pszPersona);
			Selected--;
		}
	}
	MessageCascadeSpot = -1;

	EndGroup();
}


////////////////////////////////////////////////////////////////////////
// OnSendAgain [public]
//
// Internal command handler for message map.
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnSendAgain()
{
	SendAgain();
}


void CTocFrame::OnSendImmediately()
{
	CTocDoc *	pTocDoc = GetTocDoc();
	
	if (!m_pSumListBox || !pTocDoc)
	{
		ASSERT(0);
		return;
	}
	
	BOOL ReverseSendQueue = ShiftDown();
	int SelCount;
	
	// Send Immediate/Queue for Delivery can only be done on messages that are sendable
	if ( (pTocDoc->m_Type == MBT_OUT) && ((SelCount = m_pSumListBox->GetSelCount()) > 0) )
	{
		for (int i = m_pSumListBox->GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
				int State = Sum->m_State;
				if ((State == MS_SENDABLE)||(State == MS_TIME_QUEUED))
					Sum->SetState(MS_QUEUED);
				SelCount--;
			}
		}
		
		FlushQueue = GetIniShort(IDS_INI_IMMEDIATE_SEND);
		if (ReverseSendQueue)
			FlushQueue = !FlushQueue;
		
		SetQueueStatus();

		if (FlushQueue)
			SendQueuedMessages();
	}
}


void CTocFrame::OnChangeQueueing()
{
	CTocDoc *	pTocDoc = GetTocDoc();
	
	if (!m_pSumListBox || !pTocDoc)
	{
		ASSERT(0);
		return;
	}
	
	long Seconds = 0;
	int SelCount;
	
	// Change Queueing can only be done on composition messages that are sendable or queued
	if ( (pTocDoc->m_Type == MBT_OUT) && ((SelCount = m_pSumListBox->GetSelCount()) > 0) )
	{
		int i;
		for (i = m_pSumListBox->GetCount() - 1; SelCount && i >= 0; i--)
		{
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
				if (Sum->IsSendable())
				{
					if (Sum->m_State == MS_TIME_QUEUED && Sum->m_Seconds)
					{
						Seconds = Sum->m_Seconds;
						break;
					}
				}
				SelCount--;
			}
		}
		
		CChangeQueueing dlg(Seconds);
		if (dlg.DoModal() == IDOK)
		{
			SelCount = m_pSumListBox->GetSelCount();
			for (i = m_pSumListBox->GetCount() - 1; SelCount && i >= 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);
					if (Sum->IsSendable())
						dlg.ChangeSummary(Sum);
					SelCount--;
				}
			}
			if (FlushQueue)
				SendQueuedMessages();
		}
	}
}


// SetStatus
// Set all of the selected summaries to the specified state
//
BOOL CTocFrame::SetStatus(UINT nID)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return FALSE;
	}

	int SelCount = m_pSumListBox->GetSelCount();
	char ThisState;
	
	switch (nID)
	{
	case ID_MESSAGE_STATUS_UNREAD:		ThisState = MS_UNREAD;		break;
	case ID_MESSAGE_STATUS_READ:		ThisState = MS_READ;		break;
	case ID_MESSAGE_STATUS_REPLIED:		ThisState = MS_REPLIED;		break;
	case ID_MESSAGE_STATUS_FORWARDED:	ThisState = MS_FORWARDED;	break;
	case ID_MESSAGE_STATUS_REDIRECTED:	ThisState = MS_REDIRECT;	break;
	case ID_MESSAGE_STATUS_SENDABLE:	ThisState = MS_SENDABLE;	break;
	case ID_MESSAGE_STATUS_QUEUED:		ThisState = MS_QUEUED;		break;
	case ID_MESSAGE_STATUS_SENT:		ThisState = MS_SENT;		break;
	case ID_MESSAGE_STATUS_UNSENT:		ThisState = MS_UNSENT;		break;
	case ID_MESSAGE_STATUS_RECOVERED:	ThisState = MS_RECOVERED;	break;
	default: ASSERT(FALSE); return (FALSE);
	}

#ifdef IMAP4
	// If this is an IMAP toc, get the document to handle this because we've
	// got lots to do.
	//
	CTocDoc* toc = GetTocDoc();

	if ( toc && toc->IsImapToc() )
	{
		toc->ImapSetStatusOfSelectedMessages (ThisState);

		if (ThisState == MS_SENDABLE || ThisState == MS_QUEUED)
			SetQueueStatus();

		// We handled everything.
		//
		return TRUE;
	}
#endif // IMAP4
	
	for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			CSummary* pSummary = m_pSumListBox->GetItemDataPtr(i);
			ASSERT_KINDOF(CSummary, pSummary);
			pSummary->SetState(ThisState);
			SelCount--;
		}
	}

	if (ThisState == MS_SENDABLE || ThisState == MS_QUEUED)
		SetQueueStatus();
	
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnToggleStatus [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnToggleStatus()
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	int nSelCount = m_pSumListBox->GetSelCount();

#ifdef IMAP4
	// If an IMAP mailbox, pass the buck.
	CTocDoc* toc = GetTocDoc();

	if ( toc && toc->IsImapToc() )
	{
		toc->ImapToggleStatusOfSelectedMessages ();

		// We handled everything.
		//
		return;
	}
#endif // IMAP4
	
	for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && nSelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			CSummary* pSummary = m_pSumListBox->GetItemDataPtr(i);
			ASSERT_KINDOF(CSummary, pSummary);
			if (pSummary->IsComp())
			{
				//
				// Special case.  The Shift+Space shortcut bypasses
				// the normal CmdUI handlers and directly generates
				// an ID_MESSAGE_STATUS_TOGGLE command, so we have to
				// make sure we don't mark comp messages with a bogus
				// "unread" status.
				//
			}
			else if (MS_UNREAD == pSummary->m_State)
				pSummary->SetState(MS_READ);
			else if (MS_READ == pSummary->m_State)
				pSummary->SetState(MS_UNREAD);

			nSelCount--;
		}
	}
}


// SetPrioriy
// Set all of the selected summaries to the specified priority
//
BOOL CTocFrame::SetPriority(UINT nID)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return FALSE;
	}

	int Priority = (int)(nID - ID_MESSAGE_PRIOR_HIGHEST) + MSP_HIGHEST;
	int SelCount = m_pSumListBox->GetSelCount();
	
	for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			m_pSumListBox->GetItemDataPtr(i)->SetPriority(Priority);
			SelCount--;
		}
	}
	
	return TRUE;
}

// SetLabel
// Set all of the selected summaries to the specified label
//
BOOL CTocFrame::SetLabel(UINT nID)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return FALSE;
	}

	int Label = (int)(nID - ID_MESSAGE_LABEL_NONE);
	int SelCount = m_pSumListBox->GetSelCount();
	
	for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			m_pSumListBox->GetItemDataPtr(i)->SetLabel(Label);
			SelCount--;
		}
	}
	
	return TRUE;
}


// SetServerStatus
// Set all of the selected summaries to the specified server status
//
BOOL CTocFrame::SetServerStatus(UINT nID)
{
	CTocDoc *	pTocDoc = GetTocDoc();
	
	if (!m_pSumListBox || !pTocDoc)
	{
		ASSERT(0);
		return FALSE;
	}

#ifdef IMAP4 // IMAP4
	//
	// If this is an IMAP mailbox, do special things for Fetch and Delete.
	// Otherwise, fall through.
	if ( pTocDoc && pTocDoc->IsImapToc() )
	{
		//	This is an IMAP mailbox - we disable set server status menu items
		//	for IMAP mailboxes. We should never get here.
		ASSERT(0);

		//	When this method was in CTocView it called ImapHandleServerStatus,
		//	which in turn called doc->ImapFetchMessages() or
		//	doc->ImapDeleteMessages(NULL, FALSE, FALSE) as appropriate (with
		//	a comment that mentioned that it would never be called anymore).
		//	If we ever want to support these commands for IMAP again, that
		//	would be the way to do it.
	
		return TRUE;
	}

#endif // END IMAP4

	int SelCount = m_pSumListBox->GetSelCount();

	// Maybe do some special handling for junk.
	if ((nID == ID_MESSAGE_SERVER_FETCH) && GetIniShort(IDS_INI_DELETE_FETCHED_JUNK))
	{
		// We are fetching and user wants to delete fetched junk: see if it is junk.
		if (pTocDoc->m_Type == MBT_JUNK)
		{
			// Yup, it's junk: delete it.
			nID = ID_MESSAGE_SERVER_DELETE;
		}
	}
	
	CountdownProgress(CRString(IDS_IMAP_MSGS_LEFTTOPROCESS), SelCount);

	for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			m_pSumListBox->GetItemDataPtr(i)->SetServerStatus(nID);
			SelCount--;

			DecrementCountdownProgress();
		}
	}

	CloseProgress();
	
	return TRUE;
}


void CTocFrame::OnFilterMessages()
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

#ifdef IMAP4
	// Do IMAP differently.
	CTocDoc* Doc = GetTocDoc();

	if (Doc && Doc->IsImapToc ())
	{
		Doc->ImapFilterMessages();

		return;
	}
#endif // IMAP4	

	int TotalCount = m_pSumListBox->GetCount();
	int SelCount = m_pSumListBox->GetSelCount();
	int DidTransfer = FALSE;

	CFilterActions filt;

	if (!filt.StartFiltering())
		return;
		
	CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), SelCount);

	CObArray	oaABHashes;

	// Generate the address book hashes once for the entire filtering loop
	CFilter::GenerateHashes(&oaABHashes);

	// process things from highest index to lowest
	// if messages are transfered out the lower selection indexes remain valid
	// if messages are transfered back in (circular filter actions) they are added at the end
	// so lower selection indexes remain valid
	for (int i = TotalCount - 1; i >= 0 && SelCount; i--)
	{
		if (EscapePressed())
			break;
			
		if (m_pSumListBox->GetSel(i) > 0)
		{
			if (filt.FilterOne(m_pSumListBox->GetItemDataPtr(i), WTA_MANUAL, &oaABHashes) & FA_TRANSFER)
				DidTransfer = TRUE;

			SelCount--;			// we can probably get out early...

			DecrementCountdownProgress();
		}
	}
	
	if (DidTransfer)
	{
		// The listbox redraw got turned off because a transfer was done, so turn it on
		m_pSumListBox->SetRedraw(TRUE);

		// Notify ourselves that the preview selection was changed as a result
		// of direct user action.
		NotifyPreviewSelectionMayHaveChanged();
	}
	
	filt.EndFiltering();
}


void CTocFrame::OnMakeFilter()
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	CMakeFilter		mkFiltDlg;
	CTempSumList	summaryList;

	int SelCount = m_pSumListBox->GetSelCount();

	for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && SelCount; i--)
	{
		if (m_pSumListBox->GetSel(i) > 0)
		{
			CSummary *selSum = m_pSumListBox->GetItemDataPtr(i);
			summaryList.AddTail(selSum);
			SelCount--;
		}
	}

	if (mkFiltDlg.GoFigure(&summaryList))
		mkFiltDlg.DoModal();
}


void CTocFrame::OnTransferNewInRoot()
{
	CTocDoc *		pTocDoc = GetTocDoc();
	if (!pTocDoc)
	{
		ASSERT(0);
		return;
	}

	QCMailboxCommand*	pCommand;
	pCommand = g_theMailboxDirector.CreateTargetMailbox( NULL, TRUE );
	if ( pCommand )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		ASSERT( pCommand->GetType() == MBT_REGULAR );

		if ( ShiftDown() )
			pCommand->Execute(CA_COPY, pTocDoc);
		else
			pCommand->Execute(CA_TRANSFER_TO, pTocDoc);
	}
}


void CTocFrame::OnUpdateDynamicCommand(CCmdUI * pCmdUI)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}
	
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( ( theAction == CA_REPLY_WITH ) ||
				( theAction == CA_REPLY_TO_ALL_WITH ) || 
				( theAction == CA_REDIRECT_TO ) )
			{				
				// Enable all menu items except Reply and Redirect for the Out mailbox
				pCmdUI->Enable( IsReplyable() );
				return;
			}	
			
			if ( ( theAction == CA_TRANSFER_TO ) || 
				( theAction == CA_TRANSFER_NEW ) ||
				( theAction == CA_FORWARD_TO ) )

			{
				pCmdUI->Enable(m_pSumListBox->GetSelCount() > 0);
				return;
			}

			if (theAction == CA_TRANSLATE_PLUGIN)
			{
				CTocDoc *		pTocDoc = GetTocDoc();
				CWnd *			pPreviewPane = m_wndSplitter.GetPane(1, 0);
				int				nNumSelected = m_pSumListBox->GetSelCount();
				
				if ( pTocDoc && pTocDoc->UsingPreviewPane() && m_bDisplayingPreview &&
					 pPreviewPane && (nNumSelected == 1) )
				{
					pCmdUI->Enable(TRUE);
					return;
				}
			}

			if ( theAction == CA_CHANGE_PERSONA )
			{
				int nNumSelected = m_pSumListBox->GetSelCount();
				BOOL bEnable = TRUE;
				if (nNumSelected > 0)
				{
					#define NO_MATCH_HASH		0xFFFFFFFF
					#define MULTIPLE_MATCH_HASH	0xFFFFFFFE

					unsigned long CommonHash = NO_MATCH_HASH;
					for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && nNumSelected; i--)
					{
						if (m_pSumListBox->GetSel(i) > 0)
						{
							CSummary* pSummary = m_pSumListBox->GetItemDataPtr(i);
							if ( pSummary->IsComp() )
								bEnable = FALSE;
							if (CommonHash == NO_MATCH_HASH)
								CommonHash = pSummary->GetPersonaHash();
							else if (CommonHash != pSummary->GetPersonaHash())
								CommonHash = MULTIPLE_MATCH_HASH;
							nNumSelected--;
						}
					}

					pCmdUI->Enable(bEnable);

					BOOL bIsMatchingPersona = FALSE;
					if (CommonHash != NO_MATCH_HASH && CommonHash != MULTIPLE_MATCH_HASH)
					{
						QCPersonalityCommand* pPC = DYNAMIC_DOWNCAST(QCPersonalityCommand, pCommand);
						LPCTSTR CommandPersona = pPC->GetName();
						CString ThisPersona = g_Personalities.GetByHash(CommonHash);

						if (ThisPersona.CompareNoCase(CommandPersona) == 0 ||
							(ThisPersona.IsEmpty() && g_Personalities.IsDominant(CommandPersona)))
						{
							bIsMatchingPersona = TRUE;
						}
					}
					pCmdUI->SetRadio(bIsMatchingPersona);
					return;
				}
				else
				{
					pCmdUI->Enable( FALSE );		// nothing selected
					return;
				}
			}
		}
	}

	pCmdUI->ContinueRouting();
}


BOOL CTocFrame::OnDynamicCommand(UINT uID )
{
	CTocDoc *		pTocDoc = GetTocDoc();
	
	if (!m_pSumListBox || !pTocDoc)
	{
		ASSERT(0);
		return FALSE;
	}
	
	QCCommandObject*	pCommand;
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

#ifdef IMAP4 // Must also work if this is an IMAP mailbox. Sorry.

		ASSERT( ( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_REGULAR ||
				( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_IMAP_MAILBOX );

#else // Original->

		ASSERT( ( ( QCMailboxCommand* ) pCommand)->GetType() == MBT_REGULAR );

#endif // END IMAP4

		theAction = CA_TRANSFER_TO;
	}

	if ( pCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
	{
		if ( theAction == CA_TRANSFER_TO )
		{
			if ( ShiftDown() )
				pCommand->Execute(CA_COPY, pTocDoc);
			else
				pCommand->Execute(CA_TRANSFER_TO, pTocDoc);
			return TRUE;
		}
	}

	switch (theAction)
	{
		case CA_REPLY_WITH:
		case CA_REPLY_TO_ALL_WITH:
			{
				// Rather than executing the command, handle it ourselves after extracting
				// the useful information from the command. That way we can handle getting
				// the selected text if appropriate.
				UINT	nID = (theAction == CA_REPLY_WITH) ? ID_MESSAGE_REPLY : ID_MESSAGE_REPLY_ALL;
				ComposeMessage( nID, NULL, reinterpret_cast<QCStationeryCommand *>(pCommand)->GetPathname() );
			}
			return TRUE;

		case CA_FORWARD_TO:
		case CA_REDIRECT_TO:
			{
				// Rather than executing the command, handle it ourselves after extracting
				// the useful information from the command. That way we can handle getting
				// the selected text if appropriate.
				UINT	nID = (theAction == CA_FORWARD_TO) ? ID_MESSAGE_FORWARD : ID_MESSAGE_REDIRECT;
				ComposeMessage( nID, reinterpret_cast<QCRecipientCommand *>(pCommand)->GetName() );
			}
			return TRUE;

		case CA_TRANSLATE_PLUGIN:
			{
				CTocDoc *		pTocDoc = GetTocDoc();
				CWnd *			pPreviewPane = m_wndSplitter.GetPane(1, 0);
				int				nNumSelected = m_pSumListBox->GetSelCount();
				
				if ( pTocDoc && pTocDoc->UsingPreviewPane() && m_bDisplayingPreview &&
					 pPreviewPane && (nNumSelected == 1) )
				{
					TRANSLATE_DATA		theData;

					theData.m_pProtocol = QCProtocol::QueryProtocol(QCP_TRANSLATE, pPreviewPane);
					
					if (theData.m_pProtocol)
					{		
						theData.m_pView = NULL;
						theData.m_bBuildAddresses = FALSE;			
						pCommand->Execute( theAction, &theData );
					}
				}
			}
			return TRUE;

		case CA_CHANGE_PERSONA:
			{
				CCursor		theCursor;
				int			nSelected;
				
				nSelected = m_pSumListBox->GetSelCount();
				
				StartGroup();

				MessageCascadeSpot = 0;
			
				::AsyncEscapePressed(TRUE);				// reset Escape key logic
				
				for (int i = m_pSumListBox->GetCount() - 1; nSelected && i >= 0; i-- )
				{
					if (::AsyncEscapePressed())
					{
						break;
					}

					if (m_pSumListBox->GetSel(i) > 0)
					{
						pCommand->Execute( theAction, m_pSumListBox->GetItemDataPtr(i) );
						nSelected--;
					}
				}
				
				MessageCascadeSpot = -1;

				EndGroup();
			}
			return TRUE;
	}

	return FALSE;
}


#ifdef IMAP4 // IMAP4
//
void CTocFrame::OnUpdateUndelete(CCmdUI* pCmdUI)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	//
	// Enable undelete if this is an IMAP mailbox and there are messages 
	// marked with the IMAP \delete flag.
	//

	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc ())
	{
		for (int i = 0; i < m_pSumListBox->GetCount(); i++)
		{
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);

				if (Sum && (Sum->m_Imflags & IMFLAGS_DELETED))
				{
					// If at least 1 message has this flag.
					pCmdUI->Enable(TRUE);

					return;
				}
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


void CTocFrame::UnDelete()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc())
	{
		pTocDoc->ImapUnDeleteMessages(NULL, FALSE);
	}
}


void CTocFrame::OnUpdateImapExpunge (CCmdUI* pCmdUI)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}
	
	//
	// Enable purge if this is an IMAP mailbox and there are messages 
	// marked with the IMAP \delete flag.
	//

	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc ())
	{
		for (int i = 0; i < m_pSumListBox->GetCount(); i++)
		{
			CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);

			if (Sum && (Sum->m_Imflags & IMFLAGS_DELETED))
			{
				// If at least 1 message has this flag.
				pCmdUI->Enable(TRUE);

				return;
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


void CTocFrame::ImapExpunge()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc())
	{
		pTocDoc->ImapExpunge (FALSE);
	}
}


void CTocFrame::OnUpdateImapRedownload (CCmdUI* pCmdUI)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	//
	// Enable if this is an IMAP mailbox.
	//

	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc ())
	{
		for (int i = 0; i < m_pSumListBox->GetCount(); i++)
		{
			// Is selected:
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);

				if (Sum)
				{
					// If at least 1 message has this flag, enable "Use defaults".
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT)
					{
						pCmdUI->Enable(TRUE);
						return;
					}

					// Enable "Include Attachments" only if a message has attachments.
					//
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_REDOWNLOAD_FULL)
					{
						if (Sum->HasAttachment())
						{
							pCmdUI->Enable(TRUE);
							return;
						}
					}
				}
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


BOOL CTocFrame::OnImapRedownload(UINT nID)
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc())
	{
		//
		// Fetch the selected messages from the server.
		// Fetch attachments also.
		//
		BOOL bIncludeAttachments	= TRUE;
		if (nID == ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT)
			bIncludeAttachments = FALSE;

		pTocDoc->ImapFetchMessages(bIncludeAttachments, FALSE/*bOnlyIfNotDownloaded*/);
	}

	return TRUE;
}


// This only fetches if it's not yet downloaded
// So if none of the selected messages is un-downloaded, disable.
//
void CTocFrame::OnUpdateImapFetch (CCmdUI* pCmdUI)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc ())
	{
		const BOOL bShiftDown = ShiftDown();

		for (int i = 0; i < m_pSumListBox->GetCount(); i++)
		{
			// Is selected:
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);

				// Shift + Fetch does redownload, which can always be done
				if ( bShiftDown || (Sum && Sum->IsNotIMAPDownloadedAtAll()) )
				{
					// If at least 1 message has this flag, enable "Use defaults".
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_FETCH_DEFAULT)
					{
						pCmdUI->Enable(TRUE);
						return;
					}
				}

				if ( bShiftDown || (Sum && Sum->IsNotFullyIMAPDownloaded()) )
				{
					// Enable "Include Attachments" only if a message has attachments.
					//
					if (pCmdUI->m_nID == ID_MESSAGE_IMAP_FETCH_FULL)
					{
						if (Sum->HasAttachment())
						{
							pCmdUI->Enable(TRUE);
							return;
						}
					}
				}
			}
		}
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


BOOL CTocFrame::OnImapFetch(UINT nID)
{
	// Shift + Fetch performs a redownload
	if (ShiftDown())
		return OnImapRedownload(nID == ID_MESSAGE_IMAP_FETCH_FULL? ID_MESSAGE_IMAP_REDOWNLOAD_FULL : ID_MESSAGE_IMAP_REDOWNLOAD_DEFAULT);


	// If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc())
	{
		//
		// Fetch the selected messages from the server.
		// Don't fetch attachments necessarily (unless
		// his options to omit only large attachments is set).
		//
		BOOL bIncludeAttachments = FALSE;
		if (nID == ID_MESSAGE_IMAP_FETCH_FULL)
			bIncludeAttachments = TRUE;

		pTocDoc->ImapFetchMessages(bIncludeAttachments);
	}

	return TRUE;
}


// Clear cached contents of selected messages and reset the summaries to
// Un-Downloaded.
//
void CTocFrame::OnUpdateImapClearCached(CCmdUI* pCmdUI)
{
	if (!m_pSumListBox)
	{
		ASSERT(0);
		return;
	}

	// If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc ())
	{
		for (int i = 0; i < m_pSumListBox->GetCount(); i++)
		{
			// Is selected:
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary* Sum = m_pSumListBox->GetItemDataPtr(i);

				// Enable if it's downloaded.
				if ( Sum && Sum->IsIMAPMessageBodyDownloaded() )
				{
					pCmdUI->Enable(TRUE);
					return;
				}
			}
		}
	}

	pCmdUI->Enable(FALSE);
}


void CTocFrame::OnImapClearCached()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc())
	{
		// Clear the cache for the selected messages.
		pTocDoc->ImapFetchMessages(FALSE/*bDownloadAttachments*/,
								   TRUE/*bOnlyIfNotDownloaded*/,
								   TRUE/*bClearCacheOnly*/);
	}
}


// OnImapResync [PUBLIC]
//
// Resynchronize a mailbox.
// This is called after eudora has started so IMAP mboxes that open by default
// would get resync'd, but can also be called manually.
//
void CTocFrame::OnImapResync ()
{
	CTocDoc *	pTocDoc = GetTocDoc();

	if ( !pTocDoc->IsImapToc() )
	{
		ASSERT (0);
		return;
	}

	// Pass this to the IMAP command object.
	//
	QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) pTocDoc->GetMBFileName() );
	if( pImapFolder != NULL )
		pImapFolder->Execute( CA_IMAP_RESYNC, NULL);
	else
	{
		ASSERT(0);
	}
}

//
#endif // IMAP4


INT CTocFrame::GetCurrentSplitterPos()
{
	INT cyCur;
	INT cyMin;
	m_wndSplitter.GetRowInfo(1, cyCur, cyMin);

	if ((0 == cyCur) && (m_nTocListHeight != kFillEntireWindowHeight))
	{
		//
		// Preview pane is currently closed due to F7 toggle, so return
		// the last known splitter position instead of the current 
		// splitter position.
		//
		return m_nTocListHeight;
	}

	m_wndSplitter.GetRowInfo(0, cyCur, cyMin);
	return cyCur;
}


void CTocFrame::ShowHideSplitter()
{
	CTocDoc* TocDoc = GetTocDoc();

	if (m_PeanutToolBar)
	{
		for (int i = m_PeanutToolBar->GetToolBarCtrl().GetButtonCount() - 1; i > 1; i--)
		{
			UINT Style = m_PeanutToolBar->GetButtonStyle(i);
			if (TocDoc && TocDoc->ShowFileBrowser())
				Style &= ~(TBBS_HIDDEN | TBSTATE_HIDDEN);
			else
				Style |= (TBBS_HIDDEN | TBSTATE_HIDDEN);
			m_PeanutToolBar->SetButtonStyle(i, Style);
		}
		m_PeanutToolBar->Invalidate();
	}

	if (TocDoc->ShowFileBrowser())
	{
		//
		// Pin the splitter to the top of the window.
		//
		m_wndSplitter.SetRowInfo(0, 0, kFillEntireWindowHeight);
		m_wndSplitter.SetRowInfo(1, 0, 0);
		m_wndSplitter.GetPane(0, 0)->EnableWindow(FALSE);
		m_wndSplitter.GetPane(1, 0)->EnableWindow(TRUE);
		m_wndSplitter.RecalcLayout();
	}
	else if (!TocDoc->UsingPreviewPane())
	{
		//
		// Pin the splitter to the bottom of the window.
		//
		m_wndSplitter.SetRowInfo(0, 0, 0);
		m_wndSplitter.SetRowInfo(1, 0, kFillEntireWindowHeight);
		m_wndSplitter.GetPane(0, 0)->EnableWindow(TRUE);
		m_wndSplitter.GetPane(1, 0)->EnableWindow(FALSE);
		m_wndSplitter.RecalcLayout();
	}
	else
	{
		m_wndSplitter.GetPane(0, 0)->EnableWindow(TRUE);
		m_wndSplitter.GetPane(1, 0)->EnableWindow(TRUE);

		//
		// Do some analysis to see if the splitter will be positioned
		// in some reasonable spot.  Remember that this calculation
		// happens before the CMDIChild::ActivateFrame() stuff mangles
		// the TocFrame window position, so the calculation below
		// is, at best, only a guess.
		//
		int nFrameClientHeight = 0;
		{
			CRect rectSplitter;
			m_wndSplitter.GetWindowRect(rectSplitter);

			CRect rectMDIClient;
			{
				CMainFrame* pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
				ASSERT_KINDOF(CMainFrame, pMainFrame);
				pMainFrame->GetRealClientRect(rectMDIClient);
				pMainFrame->ClientToScreen(rectMDIClient);
			}

			nFrameClientHeight = rectMDIClient.bottom - rectSplitter.top;	// can be negative
		}

		int nSplitterPos = TocDoc->GetSplitterPos();
		if (nSplitterPos > 0 && nSplitterPos < nFrameClientHeight)
		{
			//
			// Only change the splitter position if it looks like it has
			// a chance of being positioned in the middle of the frame. 
			// 
			m_wndSplitter.SetRowInfo(0, nSplitterPos, 0);
			m_wndSplitter.SetRowInfo(1, kFillEntireWindowHeight, 0);
			m_wndSplitter.RecalcLayout();
			m_wndSplitter.GetPane(1, 0)->InvalidateRect(NULL);

			//	Use same method as OnToggleHeader to make sure that the selected
			//	item is visible when the user turns on the preview pane.
			if (m_pSumListBox)
				m_pSumListBox->SetTopIndex( m_pSumListBox->GetCurSel() );

			//	Make sure that we update our preview selection and display
			InvalidateCachedPreviewSummary(NULL, true);
		}
	}

	//	Now that we're done showing or hiding the splitter, remember the current position
	//	so that when Eudora quits we can save the correct value for the splitter position.
	RememberSplitterPos();
}

void CTocFrame::SwitchPane(int Index)
{
	CRuntimeClass* pOldPreviewRuntimeClass = m_wndSplitter.GetViewType();

	if ((Index == 0 && pOldPreviewRuntimeClass == RUNTIME_CLASS(CFileBrowseView)) ||
		(Index == 1 && pOldPreviewRuntimeClass != RUNTIME_CLASS(CFileBrowseView)))
	{
		CTocDoc* pTocDoc = GetTocDoc();
		if (pTocDoc)
		{
			pTocDoc->SetShowFileBrowser(Index == 1);

			if (Index == 0)
			{
				CFileBrowseView* pFBView = DYNAMIC_DOWNCAST(CFileBrowseView, m_wndSplitter.GetPane(1, 0));
				if (pFBView)
					pTocDoc->SetFileBrowserViewState(pFBView->GetViewState());
			}
		}

		if (Index == 1)
			SwitchViewClass(RUNTIME_CLASS(CFileBrowseView));
		else
		{
			// Make sure preview happens right away
			m_wndSplitter.SetViewType(NULL);

			// Invalidate cached preview summaries and immediately update based on current preview selection
			InvalidateCachedPreviewSummary();
			CheckPreviewSelection();
		}

		ShowHideSplitter();

		m_wndSplitter.SetActivePane(Index, 0);
	}
}

////////////////////////////////////////////////////////////////////////
// GetTocDoc [private]
//
// Helper function to get the CTocDoc associated to a mailbox window
////////////////////////////////////////////////////////////////////////
CTocDoc* CTocFrame::GetTocDoc()
{
	CView* pView = DYNAMIC_DOWNCAST(CView, m_wndSplitter.GetPane(0, 0));
	CTocDoc* pTocDoc = NULL;

	if (pView)
	{
		pTocDoc = DYNAMIC_DOWNCAST(CTocDoc, pView->GetDocument());
		ASSERT(pTocDoc);
	}

	return pTocDoc;
}

////////////////////////////////////////////////////////////////////////
// InvalidateCachedPreviewSummary [public]
//
// Notifies us that a Summary object that we might be caching is being
// invalidated for one reason or another (possibly deleted, possibly
// associated message needs to be reloaded). Also tells us whether or
// not to update the preview display if the summary was in fact cached.
////////////////////////////////////////////////////////////////////////
void CTocFrame::InvalidateCachedPreviewSummary(CSummary * in_pInvalidSummary, bool in_bRefreshPreviewDisplay)
{
	int		nSelectedItems = m_previewSummaryArray.GetSize();
	
	//	We need to invalidate if we were told to invalidate no matter what
	bool	bNeedToInvalidate = (in_pInvalidSummary == NULL);

	if (!bNeedToInvalidate)
	{
		//	We weren't told to invalidate no matter what, so check to see if we
		//	have the summary specified.
		int		nSelectedItems = m_previewSummaryArray.GetSize();
		for (int i = 0; i < nSelectedItems; i++)
		{
			if (m_previewSummaryArray[i] == in_pInvalidSummary)
			{
				bNeedToInvalidate = true;
				break;
			}
		}
	}
	
	if (bNeedToInvalidate)
	{
		m_bPreviewSelectionChangesAttempted = true;
		
		//	If we're currently ignoring any preview selection changes, just return.
		//	Typically used during execution of some code that is in the middle of using
		//	m_previewSummaryArray. Such code should call InvalidateCachedPreviewSummary
		//	when it finishes if m_bPreviewSelectionChangesAttempted is true (after setting
		//	m_bIgnorePreviewSelectionChanges back to false).
		if (m_bIgnorePreviewSelectionChanges)
			return;

		m_bCacheInvalidated = true;

		if (m_nCheckAutoMarkAsReadTimerID)
		{
			//	The cache selection has been invalidated - kill the auto mark as read timer
			KillTimer(m_nCheckAutoMarkAsReadTimerID);
			m_nCheckAutoMarkAsReadTimerID = 0;
		}
		
		if (nSelectedItems > 0)
			m_previewSummaryArray.RemoveAll();

		if (in_bRefreshPreviewDisplay)
		{
			//	Kill any previous timer
			if (m_nCheckPreviewSelectionTimerID)
			{
				KillTimer(m_nCheckPreviewSelectionTimerID);
				m_nCheckPreviewSelectionTimerID = 0;
			}
			
			//	We'd prefer to use SetTimer, because it allows us to delay slightly so
			//	that if the user was actively doing something like pressing an arrow
			//	key or delete key, we can let the user finish.
			//
			//	This is particularly important with Trident where we bail during
			//	preview displaying if there's mouse or key input. Otherwise we could
			//	get into a loop where DoPreviewDisplay would bail and call us, and
			//	we would post an event that would mean that DoPreviewDisplay would
			//	be called again too quickly - without any real chance to react to
			//	the user input.
			m_nCheckPreviewSelectionTimerID = SetTimer(kCheckPreviewSelection, 10, NULL);

			//	If SetTimer fails (which although rare, can occur - timers are a limited
			//	global resource), then we'll fall back on using PostMessage and not checking
			//	for arrow keys, etc.
			if (m_nCheckPreviewSelectionTimerID == 0)
				PostMessage(umsgCheckPreviewSelection);
		}
	}
}


void CTocFrame::NotifyPreviewSelectionMayHaveChanged(bool in_bRefreshASAP, bool bSelectionResultDirectUserAction)
{
	//	Kill any previous timer
	if (m_nCheckPreviewSelectionTimerID)
	{
		KillTimer(m_nCheckPreviewSelectionTimerID);
		m_nCheckPreviewSelectionTimerID = 0;
	}

	//	Remember whether or not the current selection is the result of direct user action
	m_bSelectionResultDirectUserAction = bSelectionResultDirectUserAction;
	
	//	Check to see how long we should wait before potentially loading
	//	based on wether we're dealing with single or multi selection
	//	and the appropriate user pref.
	short	nDelay = 250;
	if ( m_pSumListBox && (m_pSumListBox->GetSelCount() == 1) )
		nDelay = GetIniShort(IDS_INI_PREVIEW_SINGLE_DELAY);
	else
		nDelay = GetIniShort(IDS_INI_PREVIEW_MULTI_DELAY);
	
	if ( in_bRefreshASAP || (nDelay == 0) )
	{
		//	We were told to refresh ASAP. Don't bother using a timer - just post a message
		//	that will be processed soon. (We still use PostMessage because we want to
		//	make sure that when we check the preview selection that we're doing so at
		//	a good time).
		PostMessage(umsgCheckPreviewSelection);
	}
	else
	{
		//	We'd prefer to use SetTimer, because it allows us to delay a 1/4 of a second
		//	and then check to see if the user has the arrow key down (checking when
		//	a processing a posted message is too soon - event processing is too fast
		//	for the user to have had a chance to release the key).
		m_nCheckPreviewSelectionTimerID = SetTimer(kCheckPreviewSelection, nDelay, NULL);

		//	If SetTimer fails (which although rare, can occur - timers are a limited
		//	global resource), then we'll fall back on using PostMessage and not checking
		//	for arrow keys, etc.
		if (m_nCheckPreviewSelectionTimerID == 0)
			PostMessage(umsgCheckPreviewSelection);
	}
}


CTocView * CTocFrame::GetTocView() const
{
	return DYNAMIC_DOWNCAST( CTocView, m_wndSplitter.GetPane(0, 0) );
}


BOOL CTocFrame::IsReplyable()
{	
	CTocDoc *		pTocDoc = GetTocDoc();
	
	if (!m_pSumListBox || !pTocDoc)
	{
		ASSERT(0);
		return FALSE;
	}
	
	if ( (m_pSumListBox->GetSelCount() > 0) && (pTocDoc->m_Type != MBT_OUT) )
	{
		return TRUE;
	}
	
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetSelectedSummaries [private]
//
// Fills out_summaryArray with the selected summaries
////////////////////////////////////////////////////////////////////////
void CTocFrame::GetSelectedSummaries(SummaryArrayT & out_summaryArray)
{
	if (m_pSumListBox)
	{
		int			nSelectedItems = m_pSumListBox->GetSelCount();

		if (nSelectedItems > 0)
		{
			//	Get the indexes of all the selected items
			CArray<int, int>	arrayListBoxSel;
			
			arrayListBoxSel.SetSize(nSelectedItems);
			m_pSumListBox->GetSelItems( nSelectedItems, arrayListBoxSel.GetData() );

			//	Fill the summary array with the selected summaries
			int				nCopiedGoodSums = 0;
			CSummary *		pSummary;
			out_summaryArray.SetSize(nSelectedItems);
			for (int i = 0; i < nSelectedItems; i++)
			{
				pSummary = m_pSumListBox->GetItemDataPtr( arrayListBoxSel[i] );

				//	I don't expect this to be NULL, but let's be sure
				if (pSummary)
				{
					out_summaryArray[nCopiedGoodSums] = pSummary;
					++nCopiedGoodSums;
				}
			}

			if (nCopiedGoodSums < nSelectedItems)
			{
				//	Yuck, we got one or more NULL Sums - how'd that happen! Whine in debug builds
				//	and fix the size of the array.
				ASSERT(0);
				out_summaryArray.SetSize(nCopiedGoodSums);
			}
		}
	}
}


bool CTocFrame::CheckPreviewSelection()
{
	CTocDoc *		pTocDoc = GetTocDoc();

	//	Check for special handling for ESP windows with file browsers
	if ( pTocDoc && pTocDoc->IsPeanutTagged() && pTocDoc->ShowFileBrowser() ) 
	{
		SwitchPane(1);
		return true;
	}

	//	Make sure that we should be doing preview code at all
	if ( !ShouldPreview() )
		return true;

	//	Get the current list of previewable summaries
	SummaryArrayT		currentPreviewSummaryArray;
	GetSelectedSummaries(currentPreviewSummaryArray);

	//	Check to see if the selection has changed or not
	int		nSelectedItems = currentPreviewSummaryArray.GetSize();
	bool	bSelectionChanged = m_bCacheInvalidated || (m_previewSummaryArray.GetSize() != nSelectedItems);
	if (!bSelectionChanged)
	{
		for (int i = 0; i < nSelectedItems; i++)
		{
			if (currentPreviewSummaryArray[i] != m_previewSummaryArray[i])
			{
				bSelectionChanged = true;
				break;
			}
		}
	}

	if (bSelectionChanged)
		m_bPreviewSelectionChangesAttempted = true;
	
	//	If we're currently ignoring any preview selection changes, just return.
	//	Typically used during execution of some code that is in the middle of using
	//	m_previewSummaryArray. Such code should call InvalidateCachedPreviewSummary
	//	when it finishes if m_bPreviewSelectionChangesAttempted is true (after setting
	//	m_bIgnorePreviewSelectionChanges back to false).
	if (m_bIgnorePreviewSelectionChanges)
		return true;

	if (bSelectionChanged)
	{
		//	Invalidate whatever preview summaries we had before
		InvalidateCachedPreviewSummary();

		//	Keep track of the new list of preview summaries
		m_previewSummaryArray.SetSize(nSelectedItems);
		for (int i = 0; i < nSelectedItems; i++)
			m_previewSummaryArray[i] = currentPreviewSummaryArray[i];

		//	We just refreshed the cache so it's no longer invalidated
		m_bCacheInvalidated = false;

		//	Handle display of the new list of preview summaries
		m_bDisplayingPreview = DoPreviewDisplay();

		//	Update the profile combo box
		CTocView *	pTocView = GetTocView();
		if (pTocView)
			pTocView->UpdateProfileComboBox( m_previewSummaryArray.GetSize() );
	}

	//	Set the auto mark as read timer if:
	//	* We're currently displaying a preview
	//	* We're not already waiting for an auto mark as read timer
	//	* The selection was changed as the result of direct user action
	//	* The auto mark as read setting is on
	//
	//	Note that this code will be hit even if the selection hasn't changed.
	//	That's because the previous time might not have invoked auto-mark-as-read
	//	behavior (e.g. FUMLUB), but this time might (e.g. user click).
	if ( m_bDisplayingPreview && (m_nCheckAutoMarkAsReadTimerID == 0) &&
		 m_bSelectionResultDirectUserAction && (GetIniShort(IDS_INI_SET_PREVIEW_READ) != 0) )
	{	
		UINT	nAutoMarkAsReadTime = GetIniLong(IDS_INI_SET_PREVIEW_READ_SECONDS) * 1000;
		m_nCheckAutoMarkAsReadTimerID = SetTimer(kCheckAutoMarkAsRead, nAutoMarkAsReadTime, NULL);
	}

	return true;
}


void CTocFrame::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == kCheckPreviewSelection)
	{
		//	Don't bother checking for preview change if the user is currently doing
		//	something, such as using navigation keys, delete keys, or either mouse button.
		short	keyState = static_cast<short>( GetAsyncKeyState(VK_UP) | GetAsyncKeyState(VK_DOWN) |
											   GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState(VK_RIGHT) |
											   GetAsyncKeyState(VK_PRIOR) | GetAsyncKeyState(VK_NEXT) |
											   GetAsyncKeyState(VK_BACK) | GetAsyncKeyState(VK_DELETE) |
											   GetAsyncKeyState(VK_LBUTTON) | GetAsyncKeyState(VK_RBUTTON) );

		//	Be sure to only check the most significant bit. The least significant bit can also be set
		//	to indicate that the key state has changed since the last call to GetAsyncKeyState.
		if ( (keyState & 0x8000) == 0 )
		{
			KillTimer(m_nCheckPreviewSelectionTimerID);
			m_nCheckPreviewSelectionTimerID = 0;
			CheckPreviewSelection();
		}
	}
	else if (nIDEvent == kCheckAutoMarkAsRead)
	{
		//	We got the timer message, kill the auto mark as read timer
		KillTimer(m_nCheckAutoMarkAsReadTimerID);
		m_nCheckAutoMarkAsReadTimerID = 0;
		
		if (m_bPreviewAllowed)
		{
			int		nNumSummaries = m_previewSummaryArray.GetSize();
			
			for (int i = 0; i < nNumSummaries; i++)
			{
				if (m_previewSummaryArray[i]->m_State == MS_UNREAD)
				{
					m_previewSummaryArray[i]->SetState(MS_READ);

					//	If it's an IMAP TOC then we must also set the remote message status
					if ( m_previewSummaryArray[i]->m_TheToc && m_previewSummaryArray[i]->m_TheToc->IsImapToc() )
						m_previewSummaryArray[i]->m_TheToc->ImapSetMessageState(m_previewSummaryArray[i], MS_READ);
				}
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CTocFrame message handlers

////////////////////////////////////////////////////////////////////////
// OnCheckPreviewSelection [protected]
//
// 
////////////////////////////////////////////////////////////////////////
long CTocFrame::OnCheckPreviewSelection(WPARAM wParam, LPARAM)
{
	//	Just call CheckPreviewSelection without bothering to check for navigation keys because
	//	it's too soon for the user to have released them yet (verified through testing).
	CheckPreviewSelection();

	return 0;
}


////////////////////////////////////////////////////////////////////////
// SwitchViewClass [protected]
//
// Used when the view under the message summary list changes.
// This could be from one preview view type to another, or from a
// a preview view type to the file browse view (or vice versa).
////////////////////////////////////////////////////////////////////////
void CTocFrame::SwitchViewClass(CRuntimeClass* pNewViewRuntimeClass)
{
	INT cyCur;
	INT cxCur;
	INT cyMin;
	INT cxMin;

	m_wndSplitter.GetRowInfo( 1, cyCur, cyMin);
	m_wndSplitter.GetColumnInfo( 0, cxCur, cxMin);

	m_wndSplitter.SetViewType(pNewViewRuntimeClass);

	//
	// As an optimization for switching between previewed messages, we
	// don't normally repaint the TOC list part of the window.
	// However, in the case where this mailbox was previously obscured
	// behind some other window that just got nuked (before we have a
	// chance to repaint), then we *do* want to repaint the TOC list.
	//
	BOOL bTOCListNeedsFullRepaint = FALSE;
	{
		CRect rectUpdate;
		CRect rectClient;
		m_wndSplitter.GetPane(0, 0)->GetUpdateRect(rectUpdate);
		m_wndSplitter.GetPane(0, 0)->GetClientRect(rectClient);
		if (rectUpdate == rectClient)
			bTOCListNeedsFullRepaint = TRUE;
	}
	
	//
	// The following code sequence to switch out the old preview window
	// and splice in a new preview window was formulated after hours
	// and hours of experimentation and twiddling.  Please don't
	// mess with it unless you really know what you're doing.  :-)
	// WKS 97.10.22
	//
	m_wndSplitter.SetRedraw( FALSE );
	m_wndSplitter.DeleteView(1, 0);
	m_wndSplitter.CreateView( 1, 0, NULL, CSize( cxCur, cyCur ), NULL );
	m_wndSplitter.SetRowInfo( 1, cyCur, cyMin );
	m_wndSplitter.SetColumnInfo( 0, cxCur, cxMin );
	m_wndSplitter.SetRedraw( TRUE );
	m_wndSplitter.GetPane( 1, 0 )->Invalidate();
	m_wndSplitter.RecalcLayout();
	if (! bTOCListNeedsFullRepaint)
		m_wndSplitter.GetPane( 0, 0 )->ValidateRect(NULL);		// optimization to avoid WM_ERASEBKGND flicker

	CFileBrowseView* pFBView = DYNAMIC_DOWNCAST(CFileBrowseView, m_wndSplitter.GetPane(1, 0));
	if (pFBView)
	{
		// Set the file browser to be just the list control,
		// and limit it to only to the workgroup folder
		CString FolderPath;
		g_pApp->GetTranslators()->MboxContextFolder(GetTocDoc(), FolderPath);
		pFBView->SetFileListOnly(FolderPath);

		// Set the View (Large Icon, Small Icon, List, Details) to what it was before
		CTocDoc* pTocDoc = GetTocDoc();
		if (pTocDoc)
			pFBView->SetViewState(pTocDoc->GetFileBrowserViewState());

		// Need to empty out preview info so if we return to the
		// preview view the message will get reloaded
		InvalidateCachedPreviewSummary(NULL, true);
	}
}


////////////////////////////////////////////////////////////////////////
// DoPreviewDisplay [protected]
//
// Common routine for displaying a new Preview Pane of the appropriate
// type. This all keys off of m_previewSummaryArray, which can be
// empty when there's no selection of when the mailbox is empty.
////////////////////////////////////////////////////////////////////////
bool CTocFrame::DoPreviewDisplay()
{
	CTocDoc* pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->ShowFileBrowser())
		return false;

	//
	// The following flag is designed to prevent this
	// routine from becoming re-entrant.  The re-entrancy can occur
	// because of the PeekMessage() loop below which waits for
	// the Trident view to initialize.  NEVER leave this routine
	// before setting this flag back to FALSE.
	//
	if (m_bInsideDoPreviewDisplay)
		return true;

	m_bInsideDoPreviewDisplay = true;

	//	Make sure no one changes our preview selection and watch to see if anyone tries to
	m_bIgnorePreviewSelectionChanges = true;
	m_bPreviewSelectionChangesAttempted = false;

	//	Determine whether or not preview is allowed for multiple messages.
	m_bPreviewAllowed = true;
	int		nNumSummaries = m_previewSummaryArray.GetSize();

	if (nNumSummaries > 1)
	{
		if ( UsingFullFeatureSet() )		
		{
			//	We need to check Content Concentrator conditions for whether or not
			//	multiple messages will be allowed.
			short	nMaxSummariesAllowed = GetIniShort(IDS_INI_CC_MULTIPLE_MAX_NUM);
			
			if ( (nNumSummaries > nMaxSummariesAllowed) ||
				!ContentConcentrator::Instance()->ShouldConcentrate(ContentConcentrator::kCCMultipleContext, pTocDoc) )
			{
				//	Too many summaries or not Content Concentrating multiple messages
				//	(i.e. "None" profile or unknown profile).
				m_bPreviewAllowed = false;
			}
			else
			{
				//	Should we check for matching subjects or senders?
				bool	bRequireRelated = (GetIniShort(IDS_INI_CC_MULTIPLE_REQUIRE_RELATED) != 0);
				if (bRequireRelated)
				{
					//	Make sure every message matches either the from or subject of the first message
					CString		szFirstMessageSubject = RemoveSubjectPrefixMT(m_previewSummaryArray[0]->m_Subject);
					CString		szFirstMessageFrom = m_previewSummaryArray[0]->m_From;
					
					for (int i = 1; i < nNumSummaries; i++)
					{
						if ( (stricmp(szFirstMessageSubject, RemoveSubjectPrefixMT(m_previewSummaryArray[i]->m_Subject)) != 0) &&
							(stricmp(szFirstMessageFrom, m_previewSummaryArray[i]->m_From) != 0) )
						{
							m_bPreviewAllowed = false;
							break;
						}
					}
				}
			}
		}
		else
		{
			//	Display of multiple messages not allowed in Light
			m_bPreviewAllowed = false;
		}
	}

	int		nNumIMAPSummariesDownloaded = 0;
	if (m_bPreviewAllowed)
	{
		// If it's an IMAP mailbox, then we need to download the message before we preview it
		if ( (nNumSummaries > 0) && pTocDoc && pTocDoc->IsImapToc() )
		{		
			m_bPreviewAllowed = false;
			
			CImapMailbox *	pImapMailbox = pTocDoc->m_pImapMailbox;
			
			if ( pImapMailbox && pImapMailbox->PreviewAllowed() )
			{
				//	Fetch the selected messages from the server
				nNumIMAPSummariesDownloaded = pTocDoc->ImapFetchMessages(FALSE/*bDownloadAttachments*/,
																		 TRUE/*bOnlyIfNotDownloaded*/,
																		 FALSE/*bClearCacheOnly*/,
																		 FALSE/*bInvalidateCachedPreviewSums*/,
																		 TRUE/*bDoItNow*/);

				m_bPreviewAllowed = (nNumIMAPSummariesDownloaded == nNumSummaries);
			}
		}
	}

	CRuntimeClass* pOldPreviewRuntimeClass = m_wndSplitter.GetViewType();
	CRuntimeClass* pNewPreviewRuntimeClass = RUNTIME_CLASS(PgReadMsgPreview);
	BOOL bPaige = TRUE;
	BOOL bTrident = FALSE;

	if( UsingTrident() )
	{
		bTrident = TRUE;
		bPaige = FALSE;
		pNewPreviewRuntimeClass = RUNTIME_CLASS(CTridentPreviewView);
	}

	BOOL	bLoaded = FALSE;

	if (pOldPreviewRuntimeClass == pNewPreviewRuntimeClass)
	{
		bool	bContinue = true;
		
		if (bTrident)
		{
			MSG		msg;
			PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

			UINT	nMessage = msg.message;

			//	Check for fundamental user input - i.e. mouse clicks and key presses
			//	*before* we even continue with loading.
			if ( ((nMessage >= WM_LBUTTONDOWN) && (nMessage <= WM_MOUSELAST)) ||
				 ((nMessage >= WM_KEYFIRST) && (nMessage <= WM_KEYLAST)) )
			{
				//	There's already more user input, bail and wait for the user
				//	to be idle before we continue with Trident preview display.
				//
				//	Avoids two issues:
				//	* Eudora not reacting to user during Trident display
				//	* Crash that could occur if delete was processed while waiting
				//	  for Trident display to complete
				bContinue = false;
			}
		}

		if (bContinue)
			bLoaded = m_wndSplitter.GetPane(1, 0)->SendMessage(umsgLoadNewPreview);

		//	If we didn't load correctly, then remember to invalidate our cached preview
		//	summary, but don't actually generate an immediate refresh because loading
		//	might fail again and that would mean infinite attempts. By just invalidating
		//	our cached preview summary we allow ourselves to try again when something
		//	else happens (like OnChildActivate). This seems to fix a bug where preview
		//	displays would occasionally be blank when a mailbox is initially opened.
		if (m_bPreviewAllowed && !bLoaded)
			m_bPreviewSelectionChangesAttempted = true;
	}
	else
	{
		SwitchViewClass(pNewPreviewRuntimeClass);
	}

	if (m_bPreviewAllowed && bLoaded)
	{
		if (!bPaige)
		{
			//	There used to be three options, Paige, Bident (MSHTML 4.7), & Trident (MSHTML 4.7.1 & up),
			//	but we have since removed Bident support. Therefore if we're not dealing with Paige, we
			//	must be dealing with Trident.
			ASSERT(bTrident);
			
			//	Make sure something gets the focus back from those nasty Microsoft controls.
			CTridentPreviewView *	pTridentView = (CTridentPreviewView *) m_wndSplitter.GetPane(1, 0);

			if (pTridentView)
			{
				MSG		msg;
				UINT	nMessage;
				bool	bHandleMessage;

				while ( !pTridentView->IsReady() && PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
				{
					bHandleMessage = true;

					nMessage = msg.message;

					//	Check for fundamental user input - i.e. mouse clicks and key presses.
					if ( ((nMessage >= WM_LBUTTONDOWN) && (nMessage <= WM_MOUSELAST)) ||
						 ((nMessage >= WM_KEYFIRST) && (nMessage <= WM_KEYLAST)) )
					{
						//	There's already more user input. Stop waiting for Trident
						//	and get out of here so that we can react properly to it.
						//
						//	Avoids two issues:
						//	* Eudora not reacting to user during Trident display
						//	* Crash that could occur if delete was processed while waiting
						//	  for Trident display to complete
						break;
					}

					//	Now that we've established that the message is not user mouse clicks
					//	or key presses - go ahead and remove it so that we can process it
					//	while we wait for Trident to be ready.
					PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

					//	Avoid handling messages that are for us and are irrelevant given that
					//	we're still currently doing the preview display.
					if ( m_hWnd && (msg.hwnd == m_hWnd) &&
						 ((msg.message == umsgCheckPreviewSelection) || (msg.message == WM_TIMER)) )
					{
						bHandleMessage = false;
					}

					//	We're already in the middle of previewing - don't try to initiate previewing
					if (bHandleMessage)
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				
				pTridentView->EnableWindowWithDelay();
			}
		}
	}

	// If this was a preview in an IMAP mailbox, download status would not have
	// been updated because the view was disabled.
	// We need to update the status now (in every case, unfortunately).
	if (nNumIMAPSummariesDownloaded > 0)
	{
		//	Update the status now for each of the selected messages
		for (int i = 0; i < nNumIMAPSummariesDownloaded; i++)
			m_previewSummaryArray[i]->RedisplayField(FW_SERVER);
	}

	//	Be sure to allow cache invalidations again
	m_bIgnorePreviewSelectionChanges = false;

	//	Make sure that we update our preview display if appropriate
	if (m_bPreviewSelectionChangesAttempted)
		InvalidateCachedPreviewSummary(NULL, true);

	m_bInsideDoPreviewDisplay = false;		// clear re-entrancy semaphore

	return (m_bPreviewAllowed && bLoaded);
}


void CTocFrame::OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult)
{
	const int CurrentTabIndex = TabCtrl_GetCurSel(pNMHDR->hwndFrom);
	SwitchPane(CurrentTabIndex);
}

void CTocFrame::OnPeanutActions()
{
	if (m_PeanutToolBar)
	{
		const int Index = m_PeanutToolBar->CommandToIndex(ID_PEANUT_ACTIONS);
		if (Index >= 0)
		{
			CRect rect;

			m_PeanutToolBar->GetItemRect(Index, &rect);
			m_PeanutToolBar->ClientToScreen(&rect);
			g_pApp->GetTranslators()->MboxContextMenu(GetTocDoc(), rect);	
		}
	}
}

void CTocFrame::OnUpdatePeanutActions(CCmdUI* pCmdUI)
{
	CTocDoc* pTocDoc = GetTocDoc();
	pCmdUI->Enable(pTocDoc && pTocDoc->IsPeanutTagged());
}

////////////////////////////////////////////////////////////////////////
// OnClose [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnClose()
{
	// Make sure we have a wait cursor in case this takes a while
	CCursor		waitCursor;
	
	//
	// At this point, we're definitely going away, so
	// nuke any view-less docs associated with the old summary.
	//
	InvalidateCachedPreviewSummary();

	//	Kill our timers - we don't need them anymore
	if (m_nCheckPreviewSelectionTimerID)
	{
		KillTimer(m_nCheckPreviewSelectionTimerID);
		m_nCheckPreviewSelectionTimerID = 0;
	}
	if (m_nCheckAutoMarkAsReadTimerID)
	{
		KillTimer(m_nCheckAutoMarkAsReadTimerID);
		m_nCheckAutoMarkAsReadTimerID = 0;
	}
	
	CTocDoc* pTocDoc = GetTocDoc();
	if (pTocDoc)
		pTocDoc->NotifyFrameClosing();

	CFileBrowseView* pFBView = DYNAMIC_DOWNCAST(CFileBrowseView, m_wndSplitter.GetPane(1, 0));
	if (pFBView)
		pTocDoc->SetFileBrowserViewState(pFBView->GetViewState());

	CMDIChildTarget::OnClose();
}


////////////////////////////////////////////////////////////////////////
// OnCmdDelete [protected]
//
// Handles delete for both TOC list and preview pane (either Paige or
// Trident). This handler is here in the frame so that it can be shared
// by everything that the frame contains no matter where the darn focus
// is.
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnCmdDelete()
{
	CTocDoc *	pTocDoc = GetTocDoc();
	
	if (!m_pSumListBox || !pTocDoc)
	{
		ASSERT(0);
		return;
	}

	if (m_bInsideDoPreviewDisplay)
	{
		//	Return immediately if we're currently doing preview display.
		//	
		//	In bug #613: "Crash deleting messages from an IMAP mailbox" it
		//	appears that at least part of what was happening is that when
		//	doing Trident preview display we do some message pumping in order
		//	to make sure that Trident doesn't steal the focus. During that
		//	message pumping the user had the capability to initiate another
		//	delete, which allowed the user to delete a message that was
		//	currently being previewed - which in turn caused a crash.
		//
		//	Other fixes which were made to avoid this problem:
		//	* Delete and backspace keys now count as input that causes us
		//	  to delay updating the preview selection.
		//	* We check for user input during Trident message pumping
		//	  and bail if mouse clicks or key presses are detected.
		//
		//	Even so, it is almost certainly always bad if we end up here
		//	with m_bInsideDoPreviewDisplay still set to true.
		//
		//	Beep and bail.
		//	(The beep is optional and can be removed if this is deemed annoying -
		//	I was just attempting to provide some feedback.)
		::MessageBeep(MB_OK);
		return;	
	}
	
	//	Adapted from code that was previously in CTocView.
	//
	//	Previously code that handled multiple selections was in CTocView, and code that
	//	handled only the previewed summary was here. To further complicate matters multiple
	//	selections would end up displaying a single preview summary. Thus if the focus
	//	was in the TOC list multiple deletes would be done and if the preview pane was the
	//	active view, a single delete would be done there. Bleah.
	//
	//	Now command delete for TOC views is handled here whether the focus is in the TOC
	//	list or the preview pane (either Paige or Trident).
	int			nSelectedItems = m_pSumListBox->GetSelCount();

	//	If there's no selection, there's nothing to do
	if (nSelectedItems == 0)
		return;

	bool		bWarnDeleteUnread = (GetIniShort(IDS_INI_WARN_DELETE_UNREAD) != 0);
	bool		bWarnDeleteQueued = (GetIniShort(IDS_INI_WARN_DELETE_QUEUED) != 0);
	bool		bWarnDeleteUnsent = (GetIniShort(IDS_INI_WARN_DELETE_UNSENT) != 0);
	bool		bEasyDelete = !bWarnDeleteUnread && !bWarnDeleteQueued && !bWarnDeleteUnsent;
	int			mbType = pTocDoc->m_Type;
	CCursor		cursor;
	CSummary *	pSum;
	
	if (!bEasyDelete)
	{
		for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && nSelectedItems > 0; i--)
		{
			if (m_pSumListBox->GetSel(i) > 0)
			{
				pSum = m_pSumListBox->GetItemDataPtr(i);
				
				if (pSum)
				{
					if ( (pSum->m_State == MS_UNREAD) && bWarnDeleteUnread )
					{
						if (WarnDialog(IDS_INI_WARN_DELETE_UNREAD, IDS_WARN_DELETE_UNREAD) == IDOK)
							break;
						return;
					}
					else if (pSum->IsQueued() && bWarnDeleteQueued)
					{
						if (WarnDialog(IDS_INI_WARN_DELETE_QUEUED, IDS_WARN_DELETE_QUEUED) == IDOK)
							break;
						return;
					}
					else if (pSum->IsSendable() && bWarnDeleteUnsent)
					{
						if (WarnDialog(IDS_INI_WARN_DELETE_UNSENT, IDS_WARN_DELETE_UNSENT) == IDOK)
							break;
						return;
					}
				}

				nSelectedItems--;
			}
		}
	}

	//	Ok, we're sure that we're continuing - make sure that the cache doesn't get invalidated
	//	while we're in the middle of our work. This previously could have happened in the
	//	call to pMsgDoc->OnSaveDocument call made during the non-IMAP case below, but it
	//	seems likely that with fancy trash etc. that we should take similar precautions with IMAP.
	m_bIgnorePreviewSelectionChanges = true;
	m_bPreviewSelectionChangesAttempted = false;

	//	If this is an IMAP mailbox, just flag messages and get out.
	if ( pTocDoc->IsImapToc() )
	{
		//	Pass this to the IMAP command object.
		QCMailboxCommand *		pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) pTocDoc->GetMBFileName() );
		if (pImapFolder)
			pImapFolder->Execute(CA_DELETE_MESSAGE, NULL);
		else
			ASSERT(0);


	}
	else
	{		
		//	Clean up any unsaved open composition windows
		if (mbType == MBT_OUT)
		{
			CCompMessageDoc *	pMsgDoc;

			nSelectedItems = m_pSumListBox->GetSelCount();
			
			for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && nSelectedItems; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					pSum = m_pSumListBox->GetItemDataPtr(i);

					if (pSum)
					{
						pMsgDoc = reinterpret_cast<CCompMessageDoc *>( pSum->FindMessageDoc() );
						if (pMsgDoc && !pMsgDoc->m_HasBeenSaved)
							pMsgDoc->OnSaveDocument(NULL);
					}
					
					nSelectedItems--;
				}
			}
		}
		
		CTocDoc *	pTrashToc = GetTrashToc();
		if (pTrashToc)
		{
			// We no longer call Xfer() here since we want to go through
			// the command director logic for CA_TRANSFER_TO.  The command
			// director notifies the TOC window containing this message
			// that it is okay to invoke the "auto-mark-as-read" logic
			// since the mailbox was changed due to an explicit user
			// action.  The Xfer() function does not notify the TOC
			// window.
			//
			//doc->Xfer(TrashToc);
			QCMailboxCommand *		pFolder = g_theMailboxDirector.FindByPathname( (const char *) pTrashToc->GetMBFileName() );
			if (pFolder)
				pFolder->Execute(CA_TRANSFER_TO, pTocDoc);
			else
				ASSERT(0);
		}
	}

	//	Be sure to allow cache invalidations again
	m_bIgnorePreviewSelectionChanges = false;

	//	Make sure that we update our preview display if appropriate
	if (m_bPreviewSelectionChangesAttempted)
		InvalidateCachedPreviewSummary(NULL, true);
}


void CTocFrame::OnUpdateCmdDelete(CCmdUI * pCmdUI)
{
	//	Adapted from code that was previously in CTocView.
	//
	//	Previously code that handled multiple selections was in CTocView, and code that
	//	handled only the previewed summary was here. To further complicate matters multiple
	//	selections would end up displaying a single preview summary. Thus if the focus
	//	was in the TOC list multiple deletes would be done and if the preview pane was the
	//	active view, a single delete would be done here. Bleah.
	//
	//	Now all updates of command delete for TOC views are handled here whether the focus
	//	is in the TOC list or the preview pane (either Paige or Trident).
	BOOL		bShouldEnable = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && m_pSumListBox)
	{
		int			nSelectedItems = m_pSumListBox->GetSelCount();

		if ( pTocDoc->IsImapToc () )
		{
			for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && nSelectedItems > 0; i--)
			{
				if (m_pSumListBox->GetSel(i) > 0)
				{
					CSummary *	pSum = m_pSumListBox->GetItemDataPtr(i);

					if ( pSum && !(pSum->m_Imflags & IMFLAGS_DELETED) )
					{
						//	We found a message that is not deleted, so we should allow delete
						bShouldEnable = TRUE;
						return;
					}

					nSelectedItems--;
				}
			}
		}
		else
		{
			bShouldEnable = (nSelectedItems > 0);
		}
	}

	pCmdUI->Enable(bShouldEnable);
}


BOOL CTocFrame::PreTranslateMessage(MSG* pMsg) 
{
	UINT	nChar;

	if( pMsg->message == WM_CHAR )
	{
		nChar = pMsg->wParam;
	
		if( ( nChar == VK_TAB ) && GetIniShort( IDS_INI_SWITCH_PREVIEW_WITH_TAB))
		{
			if ( ShouldPreview() )
			{
				if (ShiftDown())
					SendMessage(WM_COMMAND, ID_PREV_PANE);
				else
					SendMessage(WM_COMMAND, ID_NEXT_PANE);

				return TRUE;
			}
		}
	}

	return CMDIChildTarget::PreTranslateMessage(pMsg);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Common implementation for the Trident and Paige preview panes.  The
// CTocListBox has its own context menu handler, so this should only
// get called if the user clicks in the preview pane.
//
// FORNOW, this is a duplicate of code in CReadMessageFrame.  By
// putting it in both places, we allow the future possibility of
// having different context menus in previews vs real views (Christie
// made me do it).
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = ::QCLoadMenu(IDR_CONTEXT_POPUPS);
	if (!hMenu || !menu.Attach(hMenu))
	{
		ASSERT(0);		// resources hosed?
		return;
	}

	// MP_POPUP_RECEIVED_MSG is the offset for the read message submenu.
	CMenu* pMenuPopup = menu.GetSubMenu(MP_POPUP_RECEIVED_MSG);
	if (!pMenuPopup)
		ASSERT(0);		// resources hosed?
	else
	{
		//
		// Since the popup menu we get from GetSubMenu() is a pointer
		// to a temporary object, let's make a local copy of the
		// object so that we have explicit control over its lifetime.
		//
		// Note that we edit the context menu on-the-fly in order to
		// stick in the latest/greatest Transfer menu, display the
		// edited context menu, then remove the Transfer menu.
		//
		CMenu tempPopupMenu;
		tempPopupMenu.Attach(pMenuPopup->GetSafeHmenu());

		// If there is selected text generate the "Transfer to" menus.  Do this before
		// inserting the menus below because this may change the number of items on this menu.

		// Get the selected text from the appropriate view.
		CString		strText;
		CWnd *		pPreviewPane = m_wndSplitter.GetPane(1, 0);

		if (pPreviewPane)
		{
			if ( UsingTrident() )
			{
				CTridentView *		pTridentView = DYNAMIC_DOWNCAST(CTridentView, pPreviewPane);
				if (pTridentView)
					pTridentView->GetSelectedTextAndTrim(strText, false/*in_bAllowMultipleLines*/);
			}
			else
			{
				CPaigeEdtView *		pPgEditView = DYNAMIC_DOWNCAST(CPaigeEdtView, pPreviewPane);
				if (pPgEditView)
					pPgEditView->GetSelectedTextAndTrim(strText, false/*in_bAllowMultipleLines*/);
			}
		}

		// If we have a selection generate the menus.
		CObArray	 oaTransferMatches;
		if (!strText.IsEmpty() && (GetSharewareMode() != SWM_MODE_LIGHT))
		{
			GenerateTransferToMenus(&tempPopupMenu, strText, &oaTransferMatches);
		}

		tempPopupMenu.InsertMenu(MP_TRANSFER_READMSG, MF_BYPOSITION | MF_POPUP,
									(UINT) CMainFrame::QCGetMainFrame()->GetTransferMenu()->GetSafeHmenu(),
									CRString(IDS_TRANSFER_NAME));

		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
		CContextMenu(&tempPopupMenu, ptScreen.x, ptScreen.y);
		
		// Free up the array of "Transfer to" matches.
		CMailboxScore		*pMailboxScore = NULL;
		while (oaTransferMatches.GetSize() > 0)
		{
			pMailboxScore = (CMailboxScore*)oaTransferMatches.GetAt(0);
			oaTransferMatches.RemoveAt(0);
			delete pMailboxScore;
		}

		tempPopupMenu.RemoveMenu(MP_TRANSFER_READMSG, MF_BYPOSITION);

		VERIFY(tempPopupMenu.Detach());
	}

	menu.DestroyMenu();
}


////////////////////////////////////////////////////////////////////////
// OnToggleHeader [protected]
//
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnToggleHeader() 
{
	CTocDoc* TocDoc = GetTocDoc();

	if (!TocDoc || !TocDoc->UsingPreviewPane())
	{
		::MessageBeep(MB_OK);
		return;
	}

	//
	// Grab current TOC list height.
	//
	int nTocListCurrent, nTocListMin;
	m_wndSplitter.GetRowInfo( 0, nTocListCurrent, nTocListMin );

	//
	// Grab current Preview Pane height.
	//
	int nPreviewCurrent, nPreviewMin;
	m_wndSplitter.GetRowInfo( 1, nPreviewCurrent, nPreviewMin );

	//
	// Save current TOC list height so that we can restore it next time.
	//
	m_nTocListHeight = nPreviewCurrent ? nTocListCurrent : m_nTocListHeight;

	//
	// New header height is either saved value or very large.
	//
	nTocListCurrent = nPreviewCurrent ? kFillEntireWindowHeight : m_nTocListHeight;
	m_wndSplitter.SetRowInfo( 0, nTocListCurrent, nTocListMin );
	m_wndSplitter.RecalcLayout();

	// BOG - Hack to make sure the selected item is visible when the user
	// opens the preview pane. This should probably check to see if the item
	// is allready visible. Maybe later.
	if (m_pSumListBox)
		m_pSumListBox->SetTopIndex( m_pSumListBox->GetCurSel() );
	
	if (nPreviewCurrent)
	{
		m_bDisplayingPreview = false;
	}
	else
	{
		//	We're turning on preview display so make sure that we update
		//	our selection and the preview display itself
		InvalidateCachedPreviewSummary(NULL, true);
	}
}

////////////////////////////////////////////////////////////////////////
// OnEditFindFindMsg [protected]
//
// If the Find Messages window isn't open, then open it up and select
// this mailbox in the Mailboxes tree control of the Find Messages
// window.  If the Find Messages window is alreay open, then just bring
// it to the front.
////////////////////////////////////////////////////////////////////////
void CTocFrame::OnEditFindFindMsg()
{
	const char *	szMailboxName = NULL;

	CTocDoc* pTocDoc = GetTocDoc();
	if (pTocDoc)
		szMailboxName = pTocDoc->GetMBFileName();
	
	OpenFindMessagesWindow(szMailboxName);
}


#ifdef IMAP4 // IMAP4

void CTocFrame::OnUpdateCmdUndelete(CCmdUI * pCmdUI)
{
	// Enable undelete if this is an IMAP mailbox and there are messages 
	// marked with the IMAP \delete flag.
	BOOL		bShouldEnable = FALSE;
	CTocDoc *	pTocDoc = GetTocDoc();

	if ( m_pSumListBox && pTocDoc && pTocDoc->IsImapToc() )
	{
		int				nSelectedItems = m_pSumListBox->GetSelCount();

		for (int i = m_pSumListBox->GetCount() - 1; i >= 0 && nSelectedItems > 0; i--)
		{
			if (m_pSumListBox->GetSel(i) > 0)
			{
				CSummary *	pSum = m_pSumListBox->GetItemDataPtr(i);

				if ( pSum && (pSum->m_Imflags & IMFLAGS_DELETED) )
				{
					//	We found a message that is deleted, so we should allow undelete
					bShouldEnable = TRUE;
					return;
				}

				nSelectedItems--;
			}
		}
	}

	pCmdUI->Enable(bShouldEnable);
}


//
// Enable purge if this is an IMAP mailbox and there are messages 
// marked with the IMAP \delete flag.
//
void CTocFrame::OnUpdateCmdImapExpunge (CCmdUI* pCmdUI)
{
	CTocDoc* Doc = GetTocDoc();

	if (Doc && Doc->IsImapToc())
		pCmdUI->Enable( ImapTocHasDeletedMsgs (Doc) );
	else
		pCmdUI->Enable(FALSE);
}


void CTocFrame::OnCmdUnDelete()
{
	//	If this is an IMAP mailbox, do special things.
	CTocDoc *	pTocDoc = GetTocDoc();

	if (pTocDoc && pTocDoc->IsImapToc())
	{
		//	Passing a NULL "SingleSum" to ImapUnDeleteMessages will undelete all selected
		//	messages, which is what we want.
		pTocDoc->ImapUnDeleteMessages(NULL, FALSE);

		//	Redisplay 
		InvalidateRect (NULL);
	}
}


void CTocFrame::OnCmdImapExpunge()
{
	// If this is an IMAP mailbox, do special things.
	CTocDoc* Doc = GetTocDoc();

	if (Doc && Doc->IsImapToc())
		Doc->ImapExpunge (FALSE);
}


//
#endif // IMAP4


// ShouldPreview - this routine is an extension of "UsingPreviewPane" which
// takes into account the current disposition of the splitter. It is used prior
// to displaying messages, etc., in case the user has minimized the pane.
//
// Note: this routine cannot reliably tell you if the preview option is enabled
// or disabled; use "UsingPreviewPane()" in TocDoc for that.

bool CTocFrame::ShouldPreview()
{
	CTocDoc* TocDoc = GetTocDoc();

	if ( !TocDoc || TocDoc->UsingPreviewPane() == FALSE ) {

		// user has disabled preview pane globally
		return false;
	}

	int nPreviewCurrent, nPreviewMin;
	m_wndSplitter.GetRowInfo( 1, nPreviewCurrent, nPreviewMin );

	if ( nPreviewCurrent >= 5 /*a little slop*/ ) {

		// preview pane is open more that just a crack
		return true;
	}

	// preview pane must be closed
	return false;
}

