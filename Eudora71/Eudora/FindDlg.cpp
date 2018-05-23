// FindDlg.cpp: implementation of the CFindDlg class.
//
// Copyright (c) 1998-2000 by QUALCOMM, Incorporated
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxdlgs.h> // CFindReplaceDialog
#include "QCFindMgr.h"

#include "FindDlg.h"


#include "DebugNewHelpers.h"


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

