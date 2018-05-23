// FindDlg.cpp: implementation of the CFindDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxdlgs.h> // CFindReplaceDialog
#include "QCFindMgr.h"

#include "FindDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

BEGIN_MESSAGE_MAP(CFindDlg, CFindReplaceDialog)
    //{{AFX_MSG_MAP(CFindDlg)
    ON_WM_ACTIVATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDlg message handlers

void CFindDlg::DoDataExchange(CDataExchange* pDX) 
{
	CFindReplaceDialog::DoDataExchange(pDX);
}

// --------------------------------------------------------------------------

void CFindDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	// We have to be careful here, we only want to change the FindWnd when appropriate.
	if (((WA_ACTIVE == nState) || (WA_CLICKACTIVE == nState)) && (pWndOther) && (!IsChild(pWndOther)))
	{
		// Only windows under the MainFrame are valid
		CWnd *pMainWnd = AfxGetMainWnd();
		if ((pMainWnd != pWndOther) && (!pMainWnd->IsChild(pWndOther)))
			return;

		QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
		ASSERT(pFindMgr);

		if (!pFindMgr)
			return;

		// Get the focus from where the user is coming from (before find)
		pFindMgr->UpdateFindFocus();
	}

	CFindReplaceDialog::OnActivate(nState, pWndOther, bMinimized);
}


BOOL CFindDlg::PreTranslateMessage( MSG* pMsg )
{
    // BOG: This code makes the F3 key simulate the user clicking the "Find next" button.
    // I tried simply forwarding the keystroke to Eudora's MainWnd, but that failed
    // because we (the find dialog) have the focus and the F3-handling code assumes that the
    // Wnd w/focus is the one to search in. We need to call "CanFindAgain" first since F3 is
    // disabled until the user "finds first".

    if ( pMsg->message == WM_KEYDOWN ) {
        if ( ((int)pMsg->wParam) == VK_F3 ) {
            QCFindMgr* pFindMgr = QCFindMgr::GetFindMgr();

            if ( pFindMgr && pFindMgr->CanFindAgain() )
                AfxGetMainWnd()->PostMessage( WM_FINDREPLACE, 0, (LPARAM)&m_fr );

            return TRUE;
        }
    }

    return CFindReplaceDialog::PreTranslateMessage( pMsg );
}

