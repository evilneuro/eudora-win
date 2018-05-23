// AdWazooWnd.cpp : implementation file
//

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "utils.h"

#include "AdWazooWnd.h"
#include "AdView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CAdWazooWnd

IMPLEMENT_DYNCREATE(CAdWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CAdWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CAdWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
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
	m_pAdView = new CAdView;
	if (NULL == m_pAdView)
		return -1;

	//
	// For some reason, MFC is really insistent that this CView class
	// get created as a Child window.
	//
	m_pAdView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL);
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pAdView);
	m_pAdView->SetWindowPos(NULL, rectClient.left, rectClient.top,
							rectClient.Width(), rectClient.Height(),
							SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*ptScreen*/) 
{
	// Just eat right-click context menus because we don't want the default
	// behavior of the wazoo bar allowing things like undocking, hiding, etc.
}

