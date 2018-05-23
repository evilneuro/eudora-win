// AdWazooWnd.cpp : implementation file
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "AdWazooWnd.h"
#include "AdView.h"
#include "AdWazooBar.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CAdWazooWnd

IMPLEMENT_DYNCREATE(CAdWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CAdWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CAdWazooWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()

	ON_COMMAND(ID_SEC_MDIFLOAT, OnDisabled)
	ON_COMMAND(ID_WAZOO_TAB_SHOW, OnDisabled)
	ON_COMMAND(ID_SEC_HIDE, OnDisabled)

	ON_UPDATE_COMMAND_UI(ID_SEC_MDIFLOAT, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI(ID_WAZOO_TAB_SHOW, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI(ID_SEC_HIDE, OnUpdateDisable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CAdWazooWnd::CAdWazooWnd() : CWazooWnd(IDR_AD_WINDOW),
	m_pAdView(NULL)
{
}

CAdWazooWnd::~CAdWazooWnd()
{
	m_pAdView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to Ad view.
////////////////////////////////////////////////////////////////////////
void CAdWazooWnd::OnActivateWazoo()
{
	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pAdView)
			m_pAdView->SendMessage(umsgActivateWazoo);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CAdWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CAdWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pAdView);
	m_pAdView = DEBUG_NEW_MFCOBJ_NOTHROW CAdView;
	if (NULL == m_pAdView)
		return -1;

	//
	// For some reason, MFC is really insistent that this CView class
	// get created as a Child window.
	//
	if (m_pAdView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL) == FALSE)
	{
		// When Create() fails with a CView, the CView gets deleted, so let's NULL out the pointer
		m_pAdView = NULL;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooWnd::OnEraseBkgnd(CDC *pDC)
{
	CBrush backBrush;

	backBrush.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);
	CRect rect;

	pDC->GetClipBox(&rect);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rect(0, 0, cx, cy);

	CAdWazooBar* pWazooBar = DYNAMIC_DOWNCAST(CAdWazooBar, GetParentControlBar());
	if (pWazooBar)
	{
		// Center ad view window within wazoo window
		CPoint Center(rect.CenterPoint());
		int Width = pWazooBar->ImageWidth() + 1;
		int Height = pWazooBar->ImageHeight() + 1;
		rect.left = Center.x - Width / 2;
		rect.top = Center.y - Height / 2;
		rect.right = rect.left + Width;
		rect.bottom = rect.top + Height;
	}

	if (m_pAdView)
	{
		m_pAdView->SetWindowPos(NULL, rect.left, rect.top,
								rect.Width(), rect.Height(),
								SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnDisabled [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooWnd::OnDisabled()
{
	// Disabled function for Ad window
	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnUpdateDisable [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooWnd::OnUpdateDisable(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}
