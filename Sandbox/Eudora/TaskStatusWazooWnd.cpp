// TaskStatusWazooWnd.cpp : implementation file
//

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "TaskStatusView.h"
#include "TaskStatusWazooWnd.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CTaskStatusWazooWnd

IMPLEMENT_DYNCREATE(CTaskStatusWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CTaskStatusWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CTaskStatusWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CTaskStatusWazooWnd::CTaskStatusWazooWnd() : CWazooWnd(IDR_TASK_STATUS),
	m_pTaskStatusView(NULL)
{
}

CTaskStatusWazooWnd::~CTaskStatusWazooWnd()
{
	m_pTaskStatusView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to Task Status view.
////////////////////////////////////////////////////////////////////////
void CTaskStatusWazooWnd::OnActivateWazoo()
{
	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pTaskStatusView)
			m_pTaskStatusView->SendMessage(umsgActivateWazoo);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CTaskStatusWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CTaskStatusWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pTaskStatusView);
	m_pTaskStatusView = new CTaskStatusView;
	if (NULL == m_pTaskStatusView)
		return -1;

	//
	// FORNOW, total ugly hack to "register" the one and only Task Status
	// view with the main app window.
	//
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	ASSERT(NULL == pMainWnd->m_pActiveTaskStatusView);			//FORNOW
	pMainWnd->m_pActiveTaskStatusView = m_pTaskStatusView;		//FORNOW

	//
	// For some reason, MFC is really insistent that this CView class
	// get created as a Child window.
	//
	m_pTaskStatusView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL);
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CTaskStatusWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pTaskStatusView);
	m_pTaskStatusView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}
