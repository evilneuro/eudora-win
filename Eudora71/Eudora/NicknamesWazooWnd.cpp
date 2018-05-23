// NicknamesWazooWnd.cpp : implementation file
//
// CNicknamesWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "doc.h"
#include "resource.h"
#include "utils.h"
#include "summary.h"

#include "3dformv.h"
#include "nickdoc.h"
#include "eudora.h"
#include "urledit.h"	// for NICKPAGE.H
#include "nickpage.h"	// for NICKSHT.H
#include "nicksht.h"	// for NICKVIEW.H
#include "nicktree.h"	// for NICKVIEW.H
#include "nickview.h"
#include "NicknamesWazooWnd.h"
#include "QCProtocol.h"
#include "QCFindMgr.h"
#include <afxdlgs.h> // CFindReplaceDialog
#include "FindDlg.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CNicknamesWazooWnd

IMPLEMENT_DYNCREATE(CNicknamesWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CNicknamesWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CNicknamesWazooWnd)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_MOUSEACTIVATE()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

END_MESSAGE_MAP()


CNicknamesWazooWnd::CNicknamesWazooWnd() : CWazooWnd(IDR_NICKNAMES),
	m_nRHSWidth(0)
{
	m_pWndPreviousFind = NULL;
}

CNicknamesWazooWnd::~CNicknamesWazooWnd()
{
}


////////////////////////////////////////////////////////////////////////
// DestroyWindow [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesWazooWnd::DestroyWindow() 
{
	if (m_wndSplitter.GetSafeHwnd())
	{
		int cxCur, cxMin;
		m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
		if (cxCur >= 0)
		{
			//
			// Empirically, we find that 'cxCur' is -1 if the nicknames
			// window has never been shown during a given Eudora
			// session.  Therefore, we only save the position if 
			// the window has been made visible at least once.
			//
			SetIniLong(IDS_INI_NICKNAMES_WINDOW_SPLITTER, cxCur);
		}
	}
	
	return CWnd::DestroyWindow();
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to change the standard command routing to forward commands from the
// Wazoo control bar all the way to the active splitter pane view.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesWazooWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_wndSplitter.GetSafeHwnd())
	{
		CWnd* pActivePane = m_wndSplitter.GetActivePane();
		if (pActivePane && pActivePane->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return CWazooWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


////////////////////////////////////////////////////////////////////////
// OnDeactivateWazoo [public, virtual]
//
// Override for virtual base class method.  This gets called when the
// filters window tab is deactivated or when the Wazoo container is
// closed.
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnDeactivateWazoo()
{
	ASSERT(::IsWindow(m_wndSplitter.GetSafeHwnd()));

	//
	// Give the parent implementation a chance to save the keyboard focus.
	//
	CWazooWnd::OnDeactivateWazoo();

	//
	// Notify the document that it has been deactivated so that it can
	// prompt the user for saving any unsaved changes.
	//
	ASSERT(g_Nicknames);
	g_Nicknames->CanCloseFrame(NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CNicknamesWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (m_wndSplitter.GetSafeHwnd())
	{
		CRect rectClient;
		GetClientRect(rectClient);

		m_wndSplitter.SetWindowPos(NULL, rectClient.left, rectClient.top,
											rectClient.Width(), rectClient.Height(),
											SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnGetMinMaxInfo [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
#ifdef _DEBUG
	CString msg;
	msg.Format("CNicknamesWazooWnd::OnGetMinMaxInfo: "
				"maxSize(%d,%d) "
				"maxPos(%d,%d) "
				"minTrack(%d,%d) "
				"maxTrack(%d,%d)\n",
				lpMMI->ptMaxSize.x, lpMMI->ptMaxSize.y,
				lpMMI->ptMaxPosition.x, lpMMI->ptMaxPosition.y,
				lpMMI->ptMinTrackSize.x, lpMMI->ptMinTrackSize.y,
				lpMMI->ptMaxTrackSize.x, lpMMI->ptMaxTrackSize.y);
	//TRACE0(msg);
#endif // _DEBUG

	lpMMI->ptMinTrackSize.x = 140;		//FORNOW, hardcoded
	lpMMI->ptMinTrackSize.y = 240;		//FORNOW, hardcoded

	CWnd::OnGetMinMaxInfo(lpMMI);
}


////////////////////////////////////////////////////////////////////////
// ToggleRHSDisplay [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::ToggleRHSDisplay() 
{
	//
	// First thing to do is to figure out whether or the RHS is
	// "open" or "closed".
	//
	int nRHSCurrent;
	int nRHSMin;
	m_wndSplitter.GetColumnInfo(1, nRHSCurrent, nRHSMin);

	//
	// Grab the current LHS pane size.
	//
	int nLHSCurrent;
	int nLHSMin;
	m_wndSplitter.GetColumnInfo(0, nLHSCurrent, nLHSMin);

	//
	// Grab current frame window and client area sizes.
	//
	CRect rect;
	GetWindowRect(&rect);
	CRect clientrect;
	GetClientRect(&clientrect);

	//
	// FORNOW, this is a hardcoded kludge factor.  The real solution
	// is to query the CWndSplitter border metrics.
	//
	const int FUDGE = (IsVersion4() ? 2 : 1);

	if (nRHSCurrent)
	{
//FORNOW		//
//FORNOW		// Shrinking the RHS window...  FORNOW, IsZoomed() probably
//FORNOW		// doesn't work for floating/docked control bars!
//FORNOW		//
//FORNOW		if (IsZoomed())
//FORNOW		{
			m_wndSplitter.SetColumnInfo(0, clientrect.Width(), nLHSMin);
			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}
//FORNOW		else
//FORNOW		{
//FORNOW			int width = nLHSCurrent + 												// LHS pane width
//FORNOW						(clientrect.Width() - nLHSCurrent - nRHSCurrent - FUDGE) +	// splitter bar width
//FORNOW						(rect.Width() - clientrect.Width());						// window borders
//FORNOW			SetWindowPos(0, 0, 0, width, rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
//FORNOW			m_wndSplitter.SetColumnInfo(0, nLHSCurrent, nLHSMin);
//FORNOW			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}

		//
		// Save old RHS width so that we can restore it next time.
		//
		m_nRHSWidth = nRHSCurrent;
	}
	else
	{
		//
		// Growing the RHS window...
		//
		if (0 == m_nRHSWidth)
			m_nRHSWidth = 210;		// FORNOW, use minimum

//FORNOW		if (IsZoomed())
//FORNOW		{
			m_wndSplitter.SetColumnInfo(0, clientrect.Width() - m_nRHSWidth - (clientrect.Width() - nLHSCurrent) - FUDGE, nLHSMin);
			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}
//FORNOW		else
//FORNOW		{
//FORNOW			int width = rect.Width() + m_nRHSWidth + FUDGE;
//FORNOW			SetWindowPos(0, 0, 0, width, rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
//FORNOW			m_wndSplitter.SetColumnInfo(0, nLHSCurrent, nLHSMin);
//FORNOW			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}

		m_nRHSWidth = 0;
	}

	m_wndSplitter.RecalcLayout();
//FORNOW	RecalcLayout();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to left hand Nicknames view.
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnActivateWazoo()
{
	CreateViews();

	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		CWnd* pLeftPane = m_wndSplitter.GetPane(0, 0);

		if (pLeftPane)
			pLeftPane->SendMessage(umsgActivateWazoo);
	}
}

////////////////////////////////////////////////////////////////////////
// This function is called when the find dialog is in front and we 
// click on the text field to change the contents. This function is
// called when the find dialog has *not* been dismissed and the user
// tries to change the found text. This is required because the textfield
// has really never lost focus... in the kill focus of the text field
// we prevent the kill focus when there is a find dialog so that the
// text will remain highlighted when we find the text we search for
////////////////////////////////////////////////////////////////////////

int CNicknamesWazooWnd::OnMouseActivate(CWnd* pDesktopWnd,UINT nHitTest,UINT message) {
	if(m_pWndPreviousFind) {
		m_pWndPreviousFind->SetFocus();
		m_pWndPreviousFind = NULL;
	}
	return CWazooWnd::OnMouseActivate(pDesktopWnd,nHitTest,message);

}


////////////////////////////////////////////////////////////////////////
// This function is called when the find dialog is dismissed.
// we want to set the focus back to the text field where the text was found
// this is because in the kill focus of the text field we actually never kill
// the focus so that the found text can remain highlighted even with the
// find dialog in the front.
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) {
	if(m_pWndPreviousFind) {
		m_pWndPreviousFind->SetFocus();
		m_pWndPreviousFind->SetKillFocus(true);
		m_pWndPreviousFind = NULL;
	}
	CWazooWnd::OnActivate(nState, pWndOther, bMinimized);

}

////////////////////////////////////////////////////////////////////////
// CreateViews [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesWazooWnd::CreateViews()
{
	if (!m_wndSplitter.GetSafeHwnd())
	{
		//
		// We just need a bogus context to keep CWndSplitter::CreateView()
		// happy.
		//
		CCreateContext myBogusContext;
		
		if ((! m_wndSplitter.CreateStatic(this, 1, 2)) ||
			(! m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CNicknamesViewLeft32), CSize(154,154), &myBogusContext)) ||
			(! m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CNicknamesViewRight), CSize(0,0), &myBogusContext)))
		{
			return FALSE;
		}

#ifdef _DEBUG
		{
			// FORNOW, take a looksee at the RHS view
			CView* pViewRight = (CView *) m_wndSplitter.GetPane(0, 1);
			ASSERT(pViewRight);
			ASSERT_KINDOF(CView, pViewRight);
		}
#endif // _DEBUG

		CRect rectClient;
		// I know i could have just taken the ClientRect for myself(i.e 'this'), but then that would be (0,0,0,0) when 'this' is in the process of creation
		::GetClientRect(GetParent()->m_hWnd,rectClient);

		int nSplit;
		int nStaticallyAssignedWidth = 80;
		int nDynamicallyAssignedWidth = rectClient.Width()/4;	
				
		// Restore the previous split window sizes from the INI file.
		nSplit = int(GetIniLong(IDS_INI_NICKNAMES_WINDOW_SPLITTER));
				
		// If value is negative or more than the width of the screen, then just use default
		if (nSplit < 0 || nSplit >= GetSystemMetrics(SM_CXFULLSCREEN))
		{
			ASSERT(0);
			// Lets choose it depending on the mode we are in, anyways it will be atleast 80 units (if not more)
			nSplit = __max(nDynamicallyAssignedWidth,nStaticallyAssignedWidth);
		}
		else	// This means that the value is positive & within the boundaries of acceptance
		{
			// If user is not forcing us to use his splitter window position setting, then lets set the default value
			if (FALSE == GetIniShort(IDS_INI_USE_MY_NICKNAMES_WINDOW_POSITION))
			{
				// The max of these values
				nSplit = __max(nSplit,nStaticallyAssignedWidth);
				// It's better to also compare with width of the client's rectangular area to have better size allocation
				nSplit = __max(nSplit,nDynamicallyAssignedWidth);
			}				
		}

		m_wndSplitter.SetColumnInfo(0, nSplit, 0);
		m_wndSplitter.SetColumnInfo(1, 0, 0);

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());
	}

	return TRUE;
}


// --------------------------------------------------------------------------

//
// FIND TEXT
//

void CNicknamesWazooWnd::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CNicknamesWazooWnd::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else 
		pCmdUI->Enable(FALSE);
}

LONG CNicknamesWazooWnd::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCProtocol *pProtocol = NULL;
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	CFindDlg *pFindDlg = pFindMgr->GetFindDlg();
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

	CWnd *pwndFocus = m_wndSplitter.GetPane(0,0);
	ASSERT(pwndFocus);

	if(!pwndFocus)
		return (EuFIND_ERROR);

	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, pwndFocus );

	ASSERT(pProtocol);


	
	if( ( pProtocol == NULL ))
		return EuFIND_NOTFOUND;
	int iRes = pProtocol->DoFindNext( lpFR->lpstrFindWhat, (lpFR->Flags & FR_MATCHCASE), (lpFR->Flags & FR_WHOLEWORD), TRUE);
	if(iRes == -1)
		return EuFIND_NOTFOUND;
	else
	{
		CString szTerm(lpFR->lpstrFindWhat);

		szTerm.MakeLower();
	
		// when we find a textfield with the text that we are searching for using the
		// find next, we want to kill the focus for the previous textfield...
		// for the current textfield that contains the text we are searching
		// for, we do not kill the focus so that the text will remain highlighted even
		// when the find dialog has the current focus. So we explicitly set the focus
		// back on the previous text field so that on the next kill focus the focus is really lost
		// for the previous textfield since the text is not required to be highlighted anymore
		if(m_pWndPreviousFind) {
			m_pWndPreviousFind->SetSel(0,0);
			m_pWndPreviousFind->SetKillFocus(true);
			m_pWndPreviousFind->SetFocus();
		}

		CNicknamesViewRight * pwndRight = (CNicknamesViewRight * ) m_wndSplitter.GetPane(0,1);
		CURLEdit *pEdit = NULL;
		CNicknamesCustomPage * pPage = NULL;
		
		switch(iRes)
		{
		case CNicknamesTreeCtrl::FIELD_NICKNAME:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_ONE);
			pEdit = (CURLEdit *) &(pwndRight->m_NickNameEdit);	
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;

		case CNicknamesTreeCtrl::FIELD_ADDRESSES:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_ONE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_ADDRESSES);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;

		case CNicknamesTreeCtrl::FIELD_NAME:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_ONE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_NAME);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_FIRSTNAME:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_ONE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_FIRSTNAME);
			if(pEdit)
				SelectField(pEdit, szTerm);			
			break;
		case CNicknamesTreeCtrl::FIELD_LASTNAME:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_ONE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_LASTNAME);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_ADDRESS:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_ADDRESS);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_CITY:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_CITY);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_STATE:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_STATE);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_COUNTRY:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_COUNTRY);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_ZIP:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_ZIP);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_PHONE:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_PHONE);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_FAX:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_FAX);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_MOBILE:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_MOBILE);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_WEB:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_TWO);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_WEB);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_COMPANY:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_COMPANY);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_TITLE:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_TITLE);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_ADDRESS2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_ADDRESS2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_CITY2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_CITY2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_STATE2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_STATE2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_COUNTRY2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_COUNTRY2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_ZIP2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_ZIP2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_PHONE2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_PHONE2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_FAX2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_FAX2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_MOBILE2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_MOBILE2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_WEB2:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_THREE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_WEB2);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;

		case CNicknamesTreeCtrl::FIELD_OTHER_EMAIL:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_FOUR);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_O_EMAIL);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_OTHER_PHONE:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_FOUR);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_O_PHONE);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		case CNicknamesTreeCtrl::FIELD_OTHER_WEB:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_FOUR);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = pPage->GetEditControlByLabel(NICKPAGE_EDIT_O_WEB);
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;

		case CNicknamesTreeCtrl::FIELD_NOTES:
			pwndRight->m_PropSheet.SetActivePage(CNicknamesTreeCtrl::PANE_FIVE);
			pPage = (CNicknamesCustomPage *) pwndRight->m_PropSheet.GetActivePage();
			pEdit = (CURLEdit *) &((CNicknamesNotesPage *)pPage)->m_NotesEdit;
			if(pEdit)
				SelectField(pEdit, szTerm);
			break;
		default:
			m_pWndPreviousFind = NULL;
			return EuFIND_NOTFOUND;

		}
		m_pWndPreviousFind = pEdit;
		// activate the find window
		if(pFindDlg)
			pFindDlg->SetFocus();

		return (EuFIND_OK);
	}
}

void CNicknamesWazooWnd::SelectField(CURLEdit *pEdit, const CString & szTerm) { 
	CString szTmp;
	pEdit->GetWindowText(szTmp);
	szTmp.MakeLower();
	int iPos = szTmp.Find(szTerm);
	pEdit->SetFocus();
	pEdit->SetSel(iPos, strlen(szTerm) + iPos);
	pEdit->SetKillFocus(false);
}