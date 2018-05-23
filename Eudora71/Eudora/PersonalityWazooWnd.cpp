// PersonalityWazooWnd.cpp : implementation file
//
// CPersonalityWazooWnd
// Specific implementation of a CWazooWnd.
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

#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "helpcntx.h"
#include "PersonalityView.h"
#include "PersonalityWazooWnd.h"
#include "mainfrm.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CPersonalityWazooWnd

IMPLEMENT_DYNCREATE(CPersonalityWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CPersonalityWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CPersonalityWazooWnd)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CPersonalityWazooWnd::CPersonalityWazooWnd() : CWazooWnd(IDR_PERSONALITIES),
	m_pPersonalityView(NULL)
{
}

CPersonalityWazooWnd::~CPersonalityWazooWnd()
{
	m_pPersonalityView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to change the standard command routing to forward commands from the
// Wazoo control bar all the way to the embedded view.
////////////////////////////////////////////////////////////////////////
BOOL CPersonalityWazooWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_pPersonalityView && m_pPersonalityView->GetSafeHwnd())
	{
		if (m_pPersonalityView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return CWazooWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CPersonalityWazooWnd::CreateView() 
{
	if (!m_pPersonalityView)
	{
		m_pPersonalityView = DEBUG_NEW_MFCOBJ_NOTHROW CPersonalityView;
		if (!m_pPersonalityView)
			return FALSE;

		//
		// FORNOW, total ugly hack to "register" the one and only personality
		// view with the main app window.
		//
		CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
		ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
		ASSERT(NULL == pMainWnd->m_pActivePersonalityView);			//FORNOW
		pMainWnd->m_pActivePersonalityView = m_pPersonalityView;	//FORNOW

		if (!m_pPersonalityView->Create(NULL, NULL, WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1)))
			return FALSE;

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());

		CWazooWnd::SaveInitialChildFocus(m_pPersonalityView);
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to allow JIT creation of view
////////////////////////////////////////////////////////////////////////
void CPersonalityWazooWnd::OnActivateWazoo()
{
	CreateView();
}


/////////////////////////////////////////////////////////////////////////////
// CPersonalityWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (m_pPersonalityView)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		m_pPersonalityView->SetWindowPos(NULL, rectClient.left, rectClient.top,
											rectClient.Width(), rectClient.Height(),
											SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityWazooWnd::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	CListCtrl& theCtrl = m_pPersonalityView->GetListCtrl();

	if (pWnd->GetSafeHwnd() == theCtrl.GetSafeHwnd())
	{
		//
		// Hit the list control, so get the Personality-specific
		// context menu.  First, we must get the menu that contains
		// the Personality context popups.
		//
		CMenu menuBar;
		HMENU hMenuBar = ::QCLoadMenu(IDR_PERSONALITIES);
		if (!hMenuBar || !menuBar.Attach(hMenuBar))
		{
			ASSERT(0);
			return;
		}

		CMenu* pTempPopup = menuBar.GetSubMenu(0);
		if (!pTempPopup)
			ASSERT(0);		// resources hosed?
		else
		{
			CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
			DoContextMenu(this, ptScreen, pTempPopup->GetSafeHmenu());
		}

		menuBar.DestroyMenu();
		return;
	}

	CWazooWnd::OnContextMenu(pWnd, ptScreen);
}
