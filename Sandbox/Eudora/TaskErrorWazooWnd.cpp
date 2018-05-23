// TaskErrorWazooWnd.cpp : implementation file
//

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "TaskErrorView.h"
#include "TaskErrorWazooWnd.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorWazooWnd

IMPLEMENT_DYNCREATE(CTaskErrorWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CTaskErrorWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CTaskErrorWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CTaskErrorWazooWnd::CTaskErrorWazooWnd() : CWazooWnd(IDR_TASK_ERROR),
	m_pTaskErrorView(NULL)
{
}

CTaskErrorWazooWnd::~CTaskErrorWazooWnd()
{
	m_pTaskErrorView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to Task Status view.
////////////////////////////////////////////////////////////////////////
void CTaskErrorWazooWnd::OnActivateWazoo()
{
	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pTaskErrorView)
			m_pTaskErrorView->SendMessage(umsgActivateWazoo);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CTaskErrorWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CTaskErrorWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pTaskErrorView);
	m_pTaskErrorView = new CTaskErrorView;
	if (NULL == m_pTaskErrorView)
		return -1;
/*
	//
	// FORNOW, total ugly hack to "register" the one and only Task Status
	// view with the main app window.
	//
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	ASSERT(NULL == pMainWnd->m_pActiveTaskErrorView);			//FORNOW
	pMainWnd->m_pActiveTaskErrorView = m_pTaskErrorView;		//FORNOW
*/

	//
	// For some reason, MFC is really insistent that this CView class
	// get created as a Child window.
	//
	m_pTaskErrorView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL);
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CTaskErrorWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pTaskErrorView);
	m_pTaskErrorView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}
