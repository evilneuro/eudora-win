// FilterReportWazooWnd.cpp : implementation file
//
// CFilterReportWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "FilterReportView.h"
#include "FilterReportWazooWnd.h"

#include "mainfrm.h"				// FORNOW A TOTAL HACK

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterReportWazooWnd

IMPLEMENT_DYNCREATE(CFilterReportWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CFilterReportWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CFilterReportWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CFilterReportWazooWnd::CFilterReportWazooWnd() : CWazooWnd(IDR_FILTER_REPORT),
	m_pFilterReportView(NULL)
{
}

CFilterReportWazooWnd::~CFilterReportWazooWnd()
{
	m_pFilterReportView = NULL;			// deleted in CView::OnPostNcDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CFilterReportWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CFilterReportWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pFilterReportView);
	m_pFilterReportView = new CFilterReportView;
	if (NULL == m_pFilterReportView)
		return -1;

	//
	// FORNOW, total ugly hack to "register" the one and only mailbox
	// tree control with the main app window.
	//
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	ASSERT(NULL == pMainWnd->m_pActiveFilterReportView);		//FORNOW
	pMainWnd->m_pActiveFilterReportView = m_pFilterReportView;	//FORNOW

	m_pFilterReportView->Create(NULL, NULL, WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1));
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Hook standard MFC message to initialize Wazoo window focus.
////////////////////////////////////////////////////////////////////////
LRESULT CFilterReportWazooWnd::OnInitialUpdate(WPARAM, LPARAM)
{
	CWazooWnd::SaveInitialChildFocus(m_pFilterReportView);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CFilterReportWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pFilterReportView);
	m_pFilterReportView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}


