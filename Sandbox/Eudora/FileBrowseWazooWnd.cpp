// FileBrowseWazooWnd.cpp : implementation file
//
// CFileBrowseWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "3dformv.h"
#include "helpcntx.h"
#include "FileBrowseView.h"
#include "FileBrowseWazooWnd.h"
#include "utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseWazooWnd

IMPLEMENT_DYNCREATE(CFileBrowseWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CFileBrowseWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CFileBrowseWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_FILE_BROWSE_REFRESH, OnCmdRefresh)
	ON_UPDATE_COMMAND_UI(ID_FILE_BROWSE_REFRESH, OnCmdUpdateRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CFileBrowseWazooWnd::CFileBrowseWazooWnd() : CWazooWnd(IDR_FILE_BROWSER),
	m_pFileBrowseView(NULL)
{
}

CFileBrowseWazooWnd::~CFileBrowseWazooWnd()
{
	m_pFileBrowseView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to File Browser view.
////////////////////////////////////////////////////////////////////////
void CFileBrowseWazooWnd::OnActivateWazoo()
{
	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pFileBrowseView)
			m_pFileBrowseView->SendMessage(umsgActivateWazoo);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CFileBrowseWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CFileBrowseWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pFileBrowseView);
	m_pFileBrowseView = new CFileBrowseView;
	if (NULL == m_pFileBrowseView)
		return -1;

	//
	// For some reason, MFC is really insistent that this CView class
	// get created as a Child window.
	//
	m_pFileBrowseView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL);
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pFileBrowseView);
	m_pFileBrowseView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseWazooWnd::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	HMENU hMenu = ::QCLoadMenu(MAKEINTRESOURCE(IDR_FILE_BROWSER));

	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
	DoContextMenu(this, ptScreen, hMenu);
}


////////////////////////////////////////////////////////////////////////
// OnCmdRefresh [protected]
//
// Delegate to view's command handler...
////////////////////////////////////////////////////////////////////////
void CFileBrowseWazooWnd::OnCmdRefresh()
{ 
	m_pFileBrowseView->SendMessage(WM_COMMAND, ID_FILE_BROWSE_REFRESH);
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateRefresh [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseWazooWnd::OnCmdUpdateRefresh(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
