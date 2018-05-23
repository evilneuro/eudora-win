// WebWazooWnd.cpp : implementation file
//
// CWebWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "WebWazooWnd.h"
#include "utils.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CWebWazooWnd

IMPLEMENT_DYNCREATE(CWebWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CWebWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CWebWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_FILE_BROWSE_REFRESH, OnCmdRefresh)
	ON_UPDATE_COMMAND_UI(ID_FILE_BROWSE_REFRESH, OnCmdUpdateRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CWebWazooWnd::CWebWazooWnd() : CWazooWnd(IDR_WEB_BROWSER),
	m_pWebView(NULL)
{
}

CWebWazooWnd::~CWebWazooWnd()
{
	m_pWebView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to Web Browser view.
////////////////////////////////////////////////////////////////////////
void CWebWazooWnd::OnActivateWazoo()
{
	if (!m_pWebView)
	{
		m_pWebView = DEBUG_NEW_MFCOBJ_NOTHROW CWebView;
		if (!m_pWebView)
			return;

		//
		// For some reason, MFC is really insistent that this CView class
		// get created as a Child window.
		//
		m_pWebView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL);

		// Need to notify the child wazoos to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
	}

	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pWebView)
			m_pWebView->SendMessage(umsgActivateWazoo);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CWebWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CWebWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CWebWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	if (m_pWebView)
	{
		m_pWebView->SetWindowPos(NULL, rectClient.left, rectClient.top,
											rectClient.Width(), rectClient.Height(),
											SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void CWebWazooWnd::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
//	HMENU hMenu = ::QCLoadMenu(MAKEINTRESOURCE(IDR_FILE_BROWSER));
//
//	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
//	DoContextMenu(this, ptScreen, hMenu);
}


////////////////////////////////////////////////////////////////////////
// OnCmdRefresh [protected]
//
// Delegate to view's command handler...
////////////////////////////////////////////////////////////////////////
void CWebWazooWnd::OnCmdRefresh()
{ 
	m_pWebView->SendMessage(WM_COMMAND, ID_FILE_BROWSE_REFRESH);
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateRefresh [protected]
//
////////////////////////////////////////////////////////////////////////
void CWebWazooWnd::OnCmdUpdateRefresh(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
