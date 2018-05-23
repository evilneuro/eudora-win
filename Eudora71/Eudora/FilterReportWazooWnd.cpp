// FilterReportWazooWnd.cpp : implementation file
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
#include "FilterReportView.h"
#include "FilterReportWazooWnd.h"

#include "mainfrm.h"				// FORNOW A TOTAL HACK

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterReportWazooWnd

IMPLEMENT_DYNCREATE(CFilterReportWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CFilterReportWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CFilterReportWazooWnd)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFilterReportWazooWnd* CFilterReportWazooWnd::s_pOneAndOnlyWnd = NULL;

CFilterReportWazooWnd::CFilterReportWazooWnd() : CWazooWnd(IDR_FILTER_REPORT),
	m_pFilterReportView(NULL)
{
	ASSERT(s_pOneAndOnlyWnd == NULL);
	s_pOneAndOnlyWnd = this;
}

CFilterReportWazooWnd::~CFilterReportWazooWnd()
{
	m_pFilterReportView = NULL;			// deleted in CView::OnPostNcDestroy();
	s_pOneAndOnlyWnd = NULL;
}

/*static*/ void CFilterReportWazooWnd::CreateFilterReportView()
{
	if (!s_pOneAndOnlyWnd)
		ASSERT(0);
	else
		s_pOneAndOnlyWnd->CreateView();
}

BOOL CFilterReportWazooWnd::CreateView()
{
	if (!m_pFilterReportView)
	{
		m_pFilterReportView = DEBUG_NEW_MFCOBJ_NOTHROW CFilterReportView;
		if (NULL == m_pFilterReportView)
			return FALSE;

		//
		// FORNOW, total ugly hack to "register" the one and only mailbox
		// tree control with the main app window.
		//
		CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
		ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
		ASSERT(NULL == pMainWnd->m_pActiveFilterReportView);		//FORNOW
		pMainWnd->m_pActiveFilterReportView = m_pFilterReportView;	//FORNOW

		if (!m_pFilterReportView->Create(NULL, NULL, WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1)))
			return FALSE;

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());

		SaveInitialChildFocus(m_pFilterReportView);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to make sure child view is
// created and activated
////////////////////////////////////////////////////////////////////////
void CFilterReportWazooWnd::OnActivateWazoo()
{
	CreateView();
}


/////////////////////////////////////////////////////////////////////////////
// CFilterReportWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CFilterReportWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (m_pFilterReportView)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		m_pFilterReportView->SetWindowPos(NULL, rectClient.left, rectClient.top,
											rectClient.Width(), rectClient.Height(),
											SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}
