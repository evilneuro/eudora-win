// PersonalityView.cpp : implementation file
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

#include "eudora.h"
#include "resource.h"
#include "rs.h"
#include "WazooWnd.h"
#include "persona.h"
#include "mdichild.h"
#include "3dformv.h"
#include "mainfrm.h"
#include "summary.h"	// for COMPMSGD.H
#include "compmsgd.h"
#include "MsgDoc.h"
#include "tocview.h"
#include "pop.h"
#include "guiutils.h"
#include "ClipboardMgr.h"
#include "PgReadMsgPreview.h"
#include "TridentPreviewView.h"
#include "TocFrame.h"
#include "WizardPropSheet.h"
#include "ModifyAcctSheet.h"
#include "EudoraMsgs.h"
#include "QCSharewareManager.h"

// IMAP4
#include "ImapAccountMgr.h"
// END IMAP4

#include "PersonalityView.h"

#include "DebugNewHelpers.h"



/////////////////////////////////////////////////////////////////////////////
// CPersonalityView

IMPLEMENT_DYNCREATE(CPersonalityView, CListView)

CPersonalityView::CPersonalityView() :
	m_nSortIndex(0)
{
}

CPersonalityView::~CPersonalityView()
{
}


BEGIN_MESSAGE_MAP(CPersonalityView, CListView)
	//{{AFX_MSG_MAP(CPersonalityView)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClick)

	ON_COMMAND_EX(IDM_CHECK_MAIL_PERSONALITY, OnCmdTransferMail)
	ON_COMMAND_EX(IDM_SEND_MAIL_PERSONALITY, OnCmdTransferMail)
	ON_COMMAND_EX(IDM_VALIDATE_PERSONALITY, OnCmdTransferMail)

	ON_UPDATE_COMMAND_UI(IDM_CHECK_MAIL_PERSONALITY, OnUpdateCheckMail)
	ON_UPDATE_COMMAND_UI(IDM_SEND_MAIL_PERSONALITY, OnUpdateSendMail)
	ON_UPDATE_COMMAND_UI(IDM_VALIDATE_PERSONALITY, OnUpdateNeedSelection)

	ON_COMMAND(IDM_NEW_MESSAGE_AS_PERSONALITY, OnCmdNewMessageAs)
	ON_COMMAND_EX(IDM_REPLY_AS_PERSONALITY, OnCmdMultiReplyAs)
	ON_COMMAND_EX(IDM_REPLY_TO_ALL_AS_PERSONALITY, OnCmdMultiReplyAs)
	ON_COMMAND_EX(IDM_FORWARD_AS_PERSONALITY, OnCmdMultiReplyAs)
	ON_COMMAND_EX(IDM_REDIRECT_AS_PERSONALITY, OnCmdMultiReplyAs)
	ON_COMMAND_EX(IDM_SEND_AGAIN_AS_PERSONALITY, OnCmdMultiReplyAs)

	ON_UPDATE_COMMAND_UI(IDM_NEW_MESSAGE_AS_PERSONALITY, OnUpdateNeedSingleSelection)
	ON_UPDATE_COMMAND_UI(IDM_REPLY_AS_PERSONALITY, OnUpdateMultiReply)
	ON_UPDATE_COMMAND_UI(IDM_REPLY_TO_ALL_AS_PERSONALITY, OnUpdateMultiReply)
	ON_UPDATE_COMMAND_UI(IDM_FORWARD_AS_PERSONALITY, OnUpdateMultiReply)
	ON_UPDATE_COMMAND_UI(IDM_REDIRECT_AS_PERSONALITY, OnUpdateMultiReply)
	ON_UPDATE_COMMAND_UI(IDM_SEND_AGAIN_AS_PERSONALITY, OnUpdateMultiReply)

	ON_COMMAND(IDM_NEW_PERSONALITY, OnCmdNewPersonality)
	ON_COMMAND(IDM_MODIFY_PERSONALITY, OnCmdModifyPersonality)
	ON_COMMAND(IDM_DELETE_PERSONALITY, OnCmdDeletePersonality)

	ON_UPDATE_COMMAND_UI(IDM_NEW_PERSONALITY, OnUpdateNewPersonality)
	ON_UPDATE_COMMAND_UI(IDM_MODIFY_PERSONALITY, OnUpdateNeedSingleSelection)
	ON_UPDATE_COMMAND_UI(IDM_DELETE_PERSONALITY, OnUpdateDeletePersonality)

	ON_MESSAGE(msgPersViewAdvNew, OnMsgAdvNew)
	ON_MESSAGE(msgPersViewEditDom, OnMsgAdvEditDom)

	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

  

/////////////////////////////////////////////////////////////////////////////
// CPersonalityView drawing

void CPersonalityView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CPersonalityView diagnostics

#ifdef _DEBUG
void CPersonalityView::AssertValid() const
{
	CListView::AssertValid();
}

void CPersonalityView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

  

/////////////////////////////////////////////////////////////////////////////
// CPersonalityView message handlers

void CPersonalityView::OnInitialUpdate() 
{
	CListCtrl& theCtrl = GetListCtrl();

	if (theCtrl.GetImageList(LVSIL_NORMAL))
	{
		ASSERT(0);
		return;			// guard against bogus double initializations
	}

	CListView::OnInitialUpdate();

	// 
	// Set the control to the Report style with icons.
	//
	{
		LONG lStyle = GetWindowLong( theCtrl.GetSafeHwnd(), GWL_STYLE );
		lStyle &= ~( LVS_ICON | LVS_SMALLICON | LVS_LIST );
		lStyle |= LVS_REPORT | LVS_ICON;
		lStyle |= LVS_NOSORTHEADER;		// FORNOW, until we figure out what we want to do about sorting
		SetWindowLong( theCtrl.GetSafeHwnd(), GWL_STYLE, lStyle );
	}
	
	// create the imagelist
	if (! m_ImageList.Load())
	{
		ASSERT(0);
		return;
	}

	theCtrl.SetImageList(&m_ImageList, LVSIL_NORMAL);
	theCtrl.SetImageList(&m_ImageList, LVSIL_SMALL);

	//
	// FORNOW, hardcode columns for the report view...
	//
	theCtrl.InsertColumn(0, CRString(IDS_PERSONA_VIEW_NAME_LABEL), LVCFMT_LEFT, GetIniShort(IDS_INI_PERSONA_VIEW_NAME_WIDTH));
	theCtrl.InsertColumn(1, CRString(IDS_PERSONA_VIEW_ACCOUNT_LABEL), LVCFMT_LEFT, GetIniShort(IDS_INI_PERSONA_VIEW_ACCOUNT_WIDTH));

	//
	// Refresh the contents.
	//
	PopulateView();

	//
	// Register ourselves as an OLE drop target.
	//
	VERIFY(m_dropTarget.Register(this));
} 


////////////////////////////////////////////////////////////////////////
// PopulateView [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CPersonalityView::PopulateView()
{
	//
	// First things first.  Blow everything away.
	//
	CListCtrl& theCtrl = GetListCtrl();
	theCtrl.DeleteAllItems();

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	//
	// Enumerate the list of personalities and each corresponding account.
	//
	for (LPSTR pszNameList = g_Personalities.List();
		 pszNameList && *pszNameList;
		 pszNameList += strlen(pszNameList) + 1)
	{
		LV_ITEM theItem;
		theItem.mask = LVIF_TEXT | LVIF_IMAGE;
		theItem.iItem = theCtrl.GetItemCount();	
		theItem.iSubItem = 0;
		theItem.pszText = (char *) (const char *) pszNameList;
		theItem.iImage = QCMailboxImageList::IMAGE_PERSONALITY;
		theItem.lParam = NULL;

		// insert the item
		int nIndex = theCtrl.InsertItem(&theItem);
		if (nIndex < 0)
		{
			g_Personalities.SetCurrent(strCurrentPersona);
			return FALSE;
		}

		//
		// Temporarily switch to this personality so that we can get
		// the account name.
		//
		CString strPersona(pszNameList);
		if (g_Personalities.SetCurrent(strPersona))
			theCtrl.SetItemText(theItem.iItem, 1, GetIniString(IDS_INI_POP_ACCOUNT));
		else
		{
			ASSERT(0);
		}
	}

	//
	// Forcibly set the "focus" to the first persona in the list.  The
	// list control needs a focus item so that activated Wazoos
	// display a focus rect on the selected item.
	//
	theCtrl.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);

	//
	// Last but not least, restore the current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetSelectedPersonalities [protected]
//
// Helper function which returns a list of selected personality names in 
// the caller-provided CStringList object.
////////////////////////////////////////////////////////////////////////
int CPersonalityView::GetSelectedPersonalities
(
	CStringList& strListPersonalities		//(i) must be empty on entry
)
{
	if (! strListPersonalities.IsEmpty())
	{
		ASSERT(0);
		return 0;
	}

	//
	// Walk the selected items...
	//
	CListCtrl& theCtrl = GetListCtrl();
	UINT nNumSelected = 0;
	int nIndex = -1;
	while ((nIndex = theCtrl.GetNextItem(nIndex, LVNI_SELECTED)) != -1)
	{
		//
		// Grab the item name and stuff it in the caller's string
		// list.
		//
		strListPersonalities.AddTail(theCtrl.GetItemText(nIndex, 0));
		nNumSelected++;
	}

	ASSERT(theCtrl.GetSelectedCount() == nNumSelected);
	return nNumSelected;
}


////////////////////////////////////////////////////////////////////////
// MapToGlobalCommandId [protected]
//
// Map "persona" commands to corresponding global commands as understood
// by the ComposeMessage() functions.
////////////////////////////////////////////////////////////////////////
UINT CPersonalityView::MapToGlobalCommandId(UINT uPersonaCmdId)
{
	switch (uPersonaCmdId)
	{
	case IDM_REPLY_AS_PERSONALITY:
		return ID_MESSAGE_REPLY;
	case IDM_REPLY_TO_ALL_AS_PERSONALITY:
		return ID_MESSAGE_REPLY_ALL;
	case IDM_FORWARD_AS_PERSONALITY:
		return ID_MESSAGE_FORWARD;
	case IDM_REDIRECT_AS_PERSONALITY:
		return ID_MESSAGE_REDIRECT;
	case IDM_SEND_AGAIN_AS_PERSONALITY:
		return ID_MESSAGE_SENDAGAIN;
	default:
		ASSERT(0);
		return 0;
	}
}


void CPersonalityView::GetCommandTarget(CSummary *& out_pSummary, CTocFrame *& out_pTocFrame)
{
	out_pSummary = NULL;		// non-NULL for read message case
	out_pTocFrame = NULL;		// non-NULL for TOC window case
	
	CWazooWnd *			pParentWazoo = reinterpret_cast<CWazooWnd *>( GetParent() );
	ASSERT_KINDOF(CWazooWnd, pParentWazoo);

	SECControlBar *		pParentBar = NULL;
	if (pParentWazoo)
	{
		pParentBar = reinterpret_cast<SECControlBar *>( pParentWazoo->GetParentControlBar() );
		ASSERT_KINDOF(SECControlBar, pParentBar);
	}
	
	//	Make sure we have the pParentBar as expected
	if (!pParentBar)
	{
		ASSERT(0);
		return;
	}

	//	Hunt for either "next" frame window which contains a read
	//	message view (which contains a single summary) or for a 
	//	TOC view (which contains a selection of zero or more summaries).
	//
	//	If we're checking this from an MDI mode Wazoo container, we
	//	need to check the "next" MDI child in the Z-order with a known 
	//	non-Wazoo type.  Otherwise, check the topmost MDI window.
	CFrameWnd *		pFrame;

	if ( pParentBar->IsMDIChild() )
	{
		//	Hunt for "next" MDI window, if any, in the Z-order
		pFrame = (CFrameWnd *) GetParentFrame()->GetNextWindow();

		//	If we got a frame, then it should be a CMDIChildWnd
		ASSERT( !pFrame || pFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)) );
	}
	else
	{
		//	Hunt for "topmost" MDI window, if any
		CMainFrame *	pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
		ASSERT_KINDOF(CMainFrame, pMainFrame);

		pFrame = pMainFrame->GetActiveFrame();
		if (pFrame == pMainFrame)
			pFrame = NULL;
	}

	if (pFrame)
	{
		//	1st chance, check for TOC window
		if ( pFrame->IsKindOf( RUNTIME_CLASS(CTocFrame)) )
		{
			if ( reinterpret_cast<CTocFrame *>(pFrame)->IsReplyable() )
				out_pTocFrame = reinterpret_cast<CTocFrame *>(pFrame);
		}
		
		//	2nd chance, check for read msg window
		else if ( pFrame->IsKindOf(RUNTIME_CLASS(CMDIChild)) && (IDR_READMESS == ((CMDIChild *) pFrame)->GetTemplateID()) )
		{
			CMessageDoc *	pReadMessageDoc = (CMessageDoc *) pFrame->GetActiveDocument();
			if (pReadMessageDoc)
			{
				ASSERT_KINDOF(CMessageDoc, pReadMessageDoc);
				out_pSummary = pReadMessageDoc->m_Sum;
			}
			else
			{
				ASSERT(0);
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnDestroy()
{
	//
	// Save column widths to INI file.
	//
	CListCtrl& theCtrl = GetListCtrl();
	ASSERT(::IsWindow(theCtrl.GetSafeHwnd()));

	LV_COLUMN col;
	col.mask = LVCF_WIDTH;
	for (int i = 0; theCtrl.GetColumn(i, &col); i++)
	{
		ASSERT(col.cx > 0 && col.cx < 30000);
		switch (i)
		{
		case 0:
			// Name column
			SetIniShort(IDS_INI_PERSONA_VIEW_NAME_WIDTH, short(col.cx));
			break;
		case 1:
			// Account column
			SetIniShort(IDS_INI_PERSONA_VIEW_ACCOUNT_WIDTH, short(col.cx));
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	CListView::OnDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnColumnClick [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	::MessageBeep(MB_OK);		// FORNOW, not implemented

//FORNOW	CListCtrl&		theCtrl = GetListCtrl();
//FORNOW	NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)pNMHDR;
//FORNOW	BOOL			bResult;
//FORNOW	BOOL			bSaveOrder;
//FORNOW
//FORNOW	if( pNMListView->iItem == -1 )
//FORNOW	{
//FORNOW		// the user clicked on a header button
//FORNOW			
//FORNOW		bResult = FALSE;
//FORNOW		
//FORNOW		bSaveOrder = m_bAscendingOrder;
//FORNOW
//FORNOW		if(	m_iSortIndex == pNMListView->iSubItem )
//FORNOW		{
//FORNOW			// if the index is the same, toggle between 
//FORNOW			// ascending and descending order
//FORNOW			m_bAscendingOrder = !m_bAscendingOrder;
//FORNOW		}
//FORNOW		else
//FORNOW		{					
//FORNOW			// always do ascending first
//FORNOW			m_bAscendingOrder = TRUE;
//FORNOW		}
//FORNOW
//FORNOW		if( pNMListView->iSubItem == 0 )
//FORNOW		{
//FORNOW			// sort by mailbox
//FORNOW			PFNLVCOMPARE fn = SortByMailbox;
//FORNOW
//FORNOW			bResult = theCtrl.SortItems( SortByMailbox, m_bAscendingOrder );
//FORNOW		}
//FORNOW		else if( pNMListView->iSubItem == 1 )
//FORNOW		{
//FORNOW			// sort by count
//FORNOW			bResult = theCtrl.SortItems( SortByCount, m_bAscendingOrder );
//FORNOW		}
//FORNOW		else if( pNMListView->iSubItem == 2 )
//FORNOW		{
//FORNOW			// sort by time
//FORNOW			bResult = theCtrl.SortItems( SortByTime, m_bAscendingOrder );
//FORNOW		}
//FORNOW
//FORNOW		if( bResult )
//FORNOW		{
//FORNOW			// set the sort index
//FORNOW			m_iSortIndex = pNMListView->iSubItem;
//FORNOW		}
//FORNOW		else
//FORNOW		{
//FORNOW			// restore the order
//FORNOW			m_bAscendingOrder = bSaveOrder;
//FORNOW		}
//FORNOW	}
	
	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// PreTranslateMessage [public, virtual]
//
// Pressing the Enter key is a shortcut for running the New Message 
// As command on the selected personality.
////////////////////////////////////////////////////////////////////////
BOOL CPersonalityView::PreTranslateMessage(MSG* pMsg)
{
	if ((WM_KEYDOWN == pMsg->message) && (VK_RETURN == pMsg->wParam))
	{
		//
		// Only support a single-selection since that is the corresponding
		// restriction in the context menu Cmd UI stuff.
		//
		if (GetListCtrl().GetSelectedCount() == 1)
			OnCmdNewMessageAs();
		else
			::MessageBeep(MB_OK);
		return TRUE;
	}

	if ((VK_RETURN == pMsg->wParam) && (GetKeyState(VK_MENU) < 0))
	{
		//
		// Only supports single selection...
		//
		if (GetListCtrl().GetSelectedCount() == 1)
			OnCmdModifyPersonality();
		else
			::MessageBeep(MB_OK);
		return TRUE;
	}


	// no default processing
	return CListView::PreTranslateMessage(pMsg);
}


////////////////////////////////////////////////////////////////////////
// OnDblClick [protected]
//
// Double-click is a shortcut for running the New Message As command
// on the selected personalities.
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnDblClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//
	// Only support a single-selection since that is the corresponding
	// restriction in the context menu Cmd UI stuff.
	//
	if (GetListCtrl().GetSelectedCount() == 1)
		OnCmdNewMessageAs();
	else
		::MessageBeep(MB_OK);

	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// OnKeyUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DELETE:
		{
			// The Del key is a shortcut for the Delete command.
			PostMessage(WM_COMMAND, IDM_DELETE_PERSONALITY);
		}
		break;
	default:
		CListView::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnCmdTransferMail [protected]
//
// Do a "normal" manual mail check or mail send for the selected personalities.
////////////////////////////////////////////////////////////////////////
BOOL CPersonalityView::OnCmdTransferMail(UINT nID)
{
	if (!UsingFullFeatureSet())
		return TRUE;

	//
	// Grab a list of the selected personality names.
	//
	CStringList strListPersonalities;
	if (GetSelectedPersonalities(strListPersonalities))
	{
		//
		// Convert the CStringList into the NULL-terminated list 
		// of NULL-terminated strings that ::GetMail() wants to see.
		//
		const int nLEN = (strListPersonalities.GetCount() * MAX_PERSONA_LEN) + 2;	// allocate for worst case
		char* pszNameList = DEBUG_NEW_NOTHROW char[ nLEN ];
		if (pszNameList)
		{
			char* pszName = pszNameList;
			while (! strListPersonalities.IsEmpty())
			{
				CString strName = strListPersonalities.RemoveHead();

				if (((pszName - pszNameList) + strName.GetLength() + 1) < nLEN)
				{
					strcpy(pszName, strName);
					pszName += strName.GetLength() + 1;
				}
				else
				{
					//
					// In theory, buffer overflow should never happen 
					// since we overallocated... but we check for
					// overflow anyway as a Paranoid Thing(tm).
					//
					ASSERT(0);		
					break;
				}
			}

			// NULL-terminate the list
			*pszName++ = '\0';
			*pszName   = '\0';		// in case there are no members

			switch (nID)
			{
			case IDM_CHECK_MAIL_PERSONALITY:
				::GetMail(kManualMailCheckBits, pszNameList);
				break;
			case IDM_SEND_MAIL_PERSONALITY:
				::GetMail(kSendMailBit, pszNameList);
				break;
			case IDM_VALIDATE_PERSONALITY:
				::GetMail(kValidateLoginOnly, pszNameList);
				break;
			default:
				ASSERT(0);
				break;
			}


			delete [] pszNameList;
			pszNameList = NULL;
		}
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnCmdNewMessageAs [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnCmdNewMessageAs()
{
	if (!UsingFullFeatureSet())
		return;

	//
	// Grab a list of selected personality names.
	//
	CStringList strListPersonalities;
	if (GetSelectedPersonalities(strListPersonalities))
	{
		while (! strListPersonalities.IsEmpty())
		{
			CString strName = strListPersonalities.RemoveHead();

			CCompMessageDoc* pCompMessage = ::NewCompDocumentAs(strName);
			if (pCompMessage)
				pCompMessage->m_Sum->Display();
		}
	}
	else
	{
		ASSERT(0);		// should never get here unless CmdUI is screwed up
	}
}


////////////////////////////////////////////////////////////////////////
// OnCmdMultiReplyAs [protected]
//
// Common handler for Reply As, Reply To All As, Forward As,
// Redirect As, and Send Again As commands.
////////////////////////////////////////////////////////////////////////
BOOL CPersonalityView::OnCmdMultiReplyAs(UINT nID)
{
	if (!UsingFullFeatureSet())
		return TRUE;
	
	CSummary *		pSummary = NULL;		// non-NULL for read message case
	CTocFrame *		pTocFrame = NULL;		// non-NULL for TOC window case

	GetCommandTarget(pSummary, pTocFrame);

	//
	// At this point, at most one of 'pSummary' or 'pTocView' should
	// be set.
	//
	CString strPersona;
	if (pSummary || pTocFrame)
	{
		CStringList strListPersonalities;
		if (GetSelectedPersonalities(strListPersonalities))
		{
			ASSERT(strListPersonalities.GetCount() == 1);
			strPersona = strListPersonalities.RemoveHead();
		}
		else
		{
			ASSERT(0);
		}
	}

	UINT uGlobalCmdID = MapToGlobalCommandId(nID);
	if (pSummary)
	{
		ASSERT(NULL == pTocFrame);
		if (ID_MESSAGE_SENDAGAIN == uGlobalCmdID)
		{
			pSummary->SendAgain(TRUE, strPersona);
		}
		else
		{
			// We didn't find a TocFrame for our command handling, so look for the
			// message doc (we really should find one, but we'll be ok if we don't)
			// so that we can properly handle passing the selected text (if any).
			CString			szText = "";
			const char *	pSelText = NULL;
			CMessageDoc *	pMessageDoc = pSummary->FindMessageDoc();

			ASSERT(pMessageDoc);

			if ( pMessageDoc && pMessageDoc->GetSelectedText(szText) )
				pSelText = szText;

			pSummary->ComposeMessage(uGlobalCmdID, NULL, FALSE, NULL, strPersona, pSelText);
		}
	}
	else if (pTocFrame)
	{
		ASSERT(NULL == pSummary);
		if (ID_MESSAGE_SENDAGAIN == uGlobalCmdID)
			pTocFrame->SendAgain(strPersona);
		else
			pTocFrame->ComposeMessage(uGlobalCmdID, NULL, NULL, strPersona);
	}
	else
	{
		// Shouldn't get here unless the Command UI stuff is screwed up
		ASSERT(0);
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnUpdateCheckMail [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateCheckMail(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (UsingFullFeatureSet())
		OnUpdateNeedSelection(pCmdUI);
}


////////////////////////////////////////////////////////////////////////
// OnUpdateSendMail [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateSendMail(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (UsingFullFeatureSet())
	{
		if (GetListCtrl().GetSelectedCount() == 0)
		{
			pCmdUI->Enable(FALSE);
			return;
		}

		pCmdUI->Enable((QueueStatus & QS_READY_TO_BE_SENT) == QS_READY_TO_BE_SENT);
	}
}


////////////////////////////////////////////////////////////////////////
// OnUpdateMultiReply [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateMultiReply(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

	if (GetListCtrl().GetSelectedCount() != 1)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	CSummary *	pSummary = NULL;		// non-NULL for read message case
	CTocFrame *	pTocFrame = NULL;		// non-NULL for TOC window case

	//	Get the "replyable" target window, if any
	GetCommandTarget(pSummary, pTocFrame);

	BOOL	bEnable = ( pTocFrame || (pSummary && !pSummary->IsComp()) );

	pCmdUI->Enable(bEnable);
}

LONG CPersonalityView::OnMsgAdvNew(WPARAM, LPARAM)
{
	CModifyAcctSheet dlg(CRString(IDS_PERS_CREATE_NEW_ACCT), this, 0, "", true); // Create new account (not modify)'
	dlg.Create();

	return (0);
}

LONG CPersonalityView::OnMsgAdvEditDom(WPARAM, LPARAM)
{
	CListCtrl& theCtrl = GetListCtrl();
	const CString strName = theCtrl.GetItemText(0, 0);

	CString strTemp ;
	strTemp.Format( IDS_PERS_ACCT_SETTINGS_FORMAT, strName) ;

	CModifyAcctSheet dlg( strTemp, this, 0, strName) ;
	dlg.Create();

	return (0);
}

////////////////////////////////////////////////////////////////////////
// OnCmdNewPersonality [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnCmdNewPersonality()
{
	if (!UsingFullFeatureSet())
		return;

	if (ShiftDown())
	{
		CModifyAcctSheet dlg(CRString(IDS_PERS_CREATE_NEW_ACCT), this, 0, "", true); // Create new account (not modify)'
		dlg.Create();
	}
	else
	{
		CWizardPropSheet dlg("", this) ;
		dlg.StartWizard() ;
	}
}


////////////////////////////////////////////////////////////////////////
// OnCmdModifyPersonality [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnCmdModifyPersonality()
{
	if (!UsingFullFeatureSet())
		return;

	//
	// Grab a list of selected personality names.  There should always 
	// be exactly one item selected, unless the CmdUI stuff is screwed
	// up.
	//
	CStringList strListPersonalities;
	if (1 == GetSelectedPersonalities(strListPersonalities))
	{
		CString strName = strListPersonalities.RemoveHead();

		CString strTemp ;
		strTemp.Format( IDS_PERS_ACCT_SETTINGS_FORMAT, strName) ;
		CModifyAcctSheet dlg( strTemp, this, 0, strName) ;
		dlg.Create();
	}
	else
	{
		ASSERT(0);		// only support single selection
	}
}


////////////////////////////////////////////////////////////////////////
// OnCmdDeletePersonality [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnCmdDeletePersonality()
{
	if (!UsingFullFeatureSet())
		return;

	//
	// Grab a list of selected personality names.
	//
	CStringList strListPersonalities;
	if (GetSelectedPersonalities(strListPersonalities))
	{
		CListCtrl& theCtrl = GetListCtrl();
		LV_FINDINFO lvFindInfo;
		lvFindInfo.flags = LVFI_STRING;
		lvFindInfo.lParam = NULL;
		while (! strListPersonalities.IsEmpty())
		{
			CString strName = strListPersonalities.RemoveHead();

			if (CRString(IDS_DOMINANT) == strName)
			{
				ASSERT(0);		// cannot allow deletion of dominant persona
				::MessageBeep(MB_ICONEXCLAMATION);
				continue;
			}

			//
			// Warn the user that we're about to delete a Persona
			// and that this cannot be undone.
			//
			CString msg;
			msg.Format(CRString(IDS_PERSONA_PERMANENT_DELETE), (const char *) strName);
			if (IDYES == ::AfxMessageBox(msg, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
			{
				//
				// First, remove the persona data.
				//
				if (g_Personalities.Remove(strName))
				{
					//
					// Then, if successful, fixup the UI.
					//
					lvFindInfo.psz = strName;
					int nIndex = theCtrl.FindItem(&lvFindInfo);
					ASSERT(nIndex != -1);
					theCtrl.DeleteItem(nIndex);
				}
				else
				{
					ASSERT(0);
				}
			}
		}
	}
	else
	{
		ASSERT(0);
	}
}


////////////////////////////////////////////////////////////////////////
// OnUpdateNewPersonality [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateNewPersonality(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);
}


////////////////////////////////////////////////////////////////////////
// OnUpdateDeletePersonality [protected]
//
// Don't allow user to delete dominant personality.
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateDeletePersonality(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (!UsingFullFeatureSet())
		return;

	//
	// Grab a list of selected personality names.
	//
	CStringList strListPersonalities;
	if (GetSelectedPersonalities(strListPersonalities))
	{
		while (! strListPersonalities.IsEmpty())
		{
			if (CRString(IDS_DOMINANT) == strListPersonalities.RemoveHead())
			{
				pCmdUI->Enable(FALSE);		// cannot allow deletion of dominant persona
				return;
			}
		}

		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);			// nothing selected
	}
}


////////////////////////////////////////////////////////////////////////
// OnUpdateNeedSelection [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
	if (GetListCtrl().GetSelectedCount() > 0)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


////////////////////////////////////////////////////////////////////////
// OnUpdateNeedSingleSelection [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityView::OnUpdateNeedSingleSelection(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (UsingFullFeatureSet())
	{
		if (GetListCtrl().GetSelectedCount() == 1)
			pCmdUI->Enable(TRUE);
		else
			pCmdUI->Enable(FALSE);
	}
}


//FORNOWint CALLBACK SortByMailbox(
//FORNOWLPARAM lParam1,
//FORNOWLPARAM lParam2, 
//FORNOWLPARAM lParamSort)
//FORNOW{
//FORNOW	CPersonalityView::CEntry*	pEntry1;
//FORNOW	CPersonalityView::CEntry*	pEntry2;
//FORNOW	
//FORNOW	pEntry1 = ( CPersonalityView::CEntry* ) lParam1;
//FORNOW	pEntry2 = ( CPersonalityView::CEntry* ) lParam2;
//FORNOW	
//FORNOW	if( lParamSort )
//FORNOW	{
//FORNOW		return ( stricmp( pEntry1->m_szMailbox, pEntry2->m_szMailbox ) );
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		return ( stricmp( pEntry2->m_szMailbox, pEntry1->m_szMailbox ) );
//FORNOW	}
//FORNOW}
//FORNOW
//FORNOW
//FORNOWint CALLBACK SortByCount(
//FORNOWLPARAM lParam1,
//FORNOWLPARAM lParam2, 
//FORNOWLPARAM lParamSort)
//FORNOW{
//FORNOW	CPersonalityView::CEntry*	pEntry1;
//FORNOW	CPersonalityView::CEntry*	pEntry2;
//FORNOW	
//FORNOW	pEntry1 = ( CPersonalityView::CEntry* ) lParam1;
//FORNOW	pEntry2 = ( CPersonalityView::CEntry* ) lParam2;
//FORNOW	
//FORNOW	if( lParamSort )
//FORNOW	{
//FORNOW		return ( pEntry1->m_uCount - pEntry2->m_uCount );
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		return ( pEntry2->m_uCount - pEntry1->m_uCount );
//FORNOW	}
//FORNOW}
//FORNOW
//FORNOW
//FORNOWint CALLBACK SortByTime(
//FORNOWLPARAM lParam1,
//FORNOWLPARAM lParam2, 
//FORNOWLPARAM lParamSort)
//FORNOW{
//FORNOW	CPersonalityView::CEntry*	pEntry1;
//FORNOW	CPersonalityView::CEntry*	pEntry2;
//FORNOW	
//FORNOW	pEntry1 = ( CPersonalityView::CEntry* ) lParam1;
//FORNOW	pEntry2 = ( CPersonalityView::CEntry* ) lParam2;
//FORNOW	
//FORNOW	CTime	time1( pEntry1->m_Time );
//FORNOW	CTime	time2( pEntry2->m_Time );
//FORNOW	
//FORNOW	if( !lParamSort )
//FORNOW	{
//FORNOW		// swap 'em
//FORNOW		CTime time3;
//FORNOW
//FORNOW		time3 = time2;
//FORNOW		time2 = time1;
//FORNOW		time1 = time3;
//FORNOW	}
//FORNOW
//FORNOW	if( time1 < time2 )
//FORNOW	{
//FORNOW		return -1;
//FORNOW	}
//FORNOW	else if ( time2 < time1 )
//FORNOW	{
//FORNOW		return 1;
//FORNOW	}
//FORNOW	
//FORNOW	return 0;
//FORNOW}
//FORNOW
//FORNOW
//FORNOWBOOL CPersonalityView::AddEntry( 
//FORNOWUINT	uCount,
//FORNOWLPCSTR	szMailbox,
//FORNOWtime_t	tNow )
//FORNOW{
//FORNOW	CEntry*		pEntry;
//FORNOW
//FORNOW	pEntry = new CEntry;
//FORNOW
//FORNOW	if( pEntry == NULL )
//FORNOW	{
//FORNOW		return FALSE;
//FORNOW	}
//FORNOW
//FORNOW	pEntry->m_szMailbox = szMailbox;
//FORNOW	pEntry->m_uCount = uCount;
//FORNOW	pEntry->m_Time = tNow;
//FORNOW
//FORNOW	( ( CEudoraApp* ) AfxGetApp() )->AddFilterListEntry( pEntry );
//FORNOW	
//FORNOW	return AddEntry( pEntry );
//FORNOW}
//FORNOW
//FORNOW
//FORNOW
//FORNOWBOOL CPersonalityView::AddEntry(
//FORNOWCEntry*	pEntry )
//FORNOW{							
//FORNOW	CListCtrl&	theCtrl = GetListCtrl();
//FORNOW	char		szTime[256];
//FORNOW	LV_ITEM		theItem;
//FORNOW	INT			iRet;
//FORNOW	CString		szCount;
//FORNOW
//FORNOW	// format the strings
//FORNOW	*szTime = 0;
//FORNOW
//FORNOW	if( pEntry->m_Time > 0L )
//FORNOW	{
//FORNOW		TimeDateString( szTime, pEntry->m_Time, TRUE );
//FORNOW	}
//FORNOW
//FORNOW	szCount.Format( "%u", pEntry->m_uCount );
//FORNOW
//FORNOW	// setup the item structure
//FORNOW	theItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
//FORNOW	theItem.iItem = theCtrl.GetItemCount();	
//FORNOW	theItem.iSubItem = 0;
//FORNOW	theItem.pszText = ( char* ) ( const char* ) ( pEntry->m_szMailbox );
//FORNOW	theItem.iImage = QCMailboxImageList::IMAGE_NORMAL_MBOX;
//FORNOW	theItem.lParam = ( LPARAM ) pEntry;
//FORNOW
//FORNOW	// to do:	
//FORNOW	// IMAGE_TRASH_MBOX
//FORNOW	// IMAGE_IN_MBOX;
//FORNOW	//IMAGE_OUT_MBOX;
//FORNOW	
//FORNOW	// insert the item
//FORNOW	iRet = theCtrl.InsertItem( &theItem );
//FORNOW
//FORNOW	if( iRet < 0 )
//FORNOW	{
//FORNOW		return FALSE;
//FORNOW	}
//FORNOW
//FORNOW	//
//FORNOW	// Set the "focus" to the newly added item and make sure it
//FORNOW	// is visible in the display.
//FORNOW	//
//FORNOW	theCtrl.SetItemState(iRet, LVIS_FOCUSED, LVIS_FOCUSED);
//FORNOW	theCtrl.EnsureVisible(iRet, TRUE);
//FORNOW
//FORNOW	// set the message column
//FORNOW	theCtrl.SetItemText( theItem.iItem, 1, szCount );
//FORNOW
//FORNOW
//FORNOW	// set the time column
//FORNOW	theCtrl.SetItemText( theItem.iItem, 2, szTime );
//FORNOW
//FORNOW	return TRUE;
//FORNOW}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
////////////////////////////////////////////////////////////////////////
DROPEFFECT CPersonalityView::OnDragOver
(
	COleDataObject* pDataObject,
	DWORD dwKeyState, 
	CPoint point
)
{
	//
	// Nuke all drop target highlighting.
	//
	CListCtrl& theCtrl = GetListCtrl();
	int nIndex = -1;
	while ((nIndex = theCtrl.GetNextItem(nIndex, LVNI_DROPHILITED)) != -1)
		theCtrl.SetItemState(nIndex, 0, LVIS_DROPHILITED);

	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_TRANSFER))
	{
		//
		// Let's do a hit test to see if we're over a valid 
		// personality target.
		//
		UINT uFlags = LVHT_ONITEM;
		nIndex = theCtrl.HitTest(point, &uFlags);
		if (nIndex != -1)
		{

			//
			// Set drop target "hilite" to the drop target item.
			//
			theCtrl.SetItemState(nIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
			if (dwKeyState & MK_CONTROL)
				return DROPEFFECT_COPY;
			else
				return DROPEFFECT_MOVE;
		}
	}

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CPersonalityView::OnDrop
(
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect, 
	CPoint point
)
{
	//
	// Nuke all drop target highlighting.
	//
	CListCtrl& theCtrl = GetListCtrl();
	int nIndex = -1;
	while ((nIndex = theCtrl.GetNextItem(nIndex, LVNI_DROPHILITED)) != -1)
		theCtrl.SetItemState(nIndex, 0, LVIS_DROPHILITED);

	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_TRANSFER))
	{
		//
		// Let's do a hit test to see if we're over a valid 
		// personality target.
		//
		UINT uFlags = LVHT_ONITEM;
		nIndex = theCtrl.HitTest(point, &uFlags);
		if (nIndex != -1)
		{
			// Get global TOC clipboard data from COleDataObject
			QCFTRANSFER qcfTransfer;
			if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfTransfer))
			{
				ASSERT(0);
				return FALSE;
			}

			ASSERT_KINDOF(CTocDoc, qcfTransfer.m_pSourceTocDoc);

			// Grab the hilited persona name
			CString strPersona(theCtrl.GetItemText(nIndex, 0));
			ASSERT(! strPersona.IsEmpty());

			//
			// Determine which kind of command to perform.
			//
			UINT uCmdType = 0;
			if (DROPEFFECT_MOVE == (dropEffect & DROPEFFECT_MOVE))
				uCmdType = ID_MESSAGE_REPLY;		// Execute "Reply As" command.
			else if (DROPEFFECT_COPY == (dropEffect & DROPEFFECT_COPY))
				uCmdType = ID_MESSAGE_REPLY_ALL;	// Execute "Reply To All As" command.
			else
			{
				ASSERT(0);							// unknown drop effect
			}

			if (uCmdType)
			{
				//
				// Determine whether we're dealing with a single Summary
				// (probably tow trucked) or an entire TOC selection.
				//
				if (qcfTransfer.m_pSingleSummary)
				{
					// Do the single Summary thing
					ASSERT_KINDOF(CSummary, qcfTransfer.m_pSingleSummary);
					qcfTransfer.m_pSingleSummary->ComposeMessage(uCmdType, NULL, FALSE, NULL, strPersona);
				}
				else
				{
					// Do the TOC thing
					CTocView* pTocView = qcfTransfer.m_pSourceTocDoc->GetView();
					ASSERT_KINDOF(CTocView, pTocView);

					if (pTocView)
					{
						CTocFrame *	pTocFrame = reinterpret_cast<CTocFrame *>( pTocView->GetParentFrame() );
						if (pTocFrame)
							pTocFrame->ComposeMessage(uCmdType, NULL, NULL, strPersona);
					}
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}


