// WebBrowserWazooWnd.cpp : implementation file
//
// CWebBrowserWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#ifdef WEB_BROWSER_WAZOO

#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "WebBrowserView.h"
#include "WebBrowserWazooWnd.h"
#ifdef ADWARE_PROTOTYPE
#include "WazooBar.h"
#endif //ADWARE_PROTOTYPE

#include "mainfrm.h"				// FORNOW A TOTAL HACK

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CWebBrowserWazooWnd

IMPLEMENT_DYNCREATE(CWebBrowserWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CWebBrowserWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CWebBrowserWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)
#ifdef ADWARE_PROTOTYPE
    ON_WM_TIMER()
#endif //ADWARE_PROTOTYPE
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CWebBrowserWazooWnd::CWebBrowserWazooWnd() : CWazooWnd(IDR_WEB_BROWSER),
	m_pWebBrowserView(NULL)
{
}

CWebBrowserWazooWnd::~CWebBrowserWazooWnd()
{
	m_pWebBrowserView = NULL;			// deleted in CView::OnPostNcDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CWebBrowserWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CWebBrowserWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pWebBrowserView);
	m_pWebBrowserView = new CWebBrowserView;
	if (NULL == m_pWebBrowserView)
		return -1;

	//
	// FORNOW, total ugly hack to "register" the one and only mailbox
	// tree control with the main app window.
	//
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	ASSERT(NULL == pMainWnd->m_pActiveWebBrowserView);			//FORNOW
	pMainWnd->m_pActiveWebBrowserView = m_pWebBrowserView;		//FORNOW

	m_pWebBrowserView->Create(NULL, NULL, WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1));
	
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Hook standard MFC message to initialize Wazoo window focus.
////////////////////////////////////////////////////////////////////////
LRESULT CWebBrowserWazooWnd::OnInitialUpdate(WPARAM, LPARAM)
{
	CWazooWnd::SaveInitialChildFocus(m_pWebBrowserView);

#ifdef ADWARE_PROTOTYPE
	// modify this wazoo not to have a close box
	CWazooBar* hoosierDaddy = DYNAMIC_DOWNCAST(CWazooBar,GetParentControlBar());
	if (hoosierDaddy)
		hoosierDaddy->ModifyBarStyleEx(CBRS_EX_GRIPPER_CLOSE,0,TRUE);
	SetTimer(0xadadadad,GetIniLong(IDS_INI_AD_KFI)*500,NULL);
#endif //ADWARE_PROTOTYPE

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CWebBrowserWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pWebBrowserView);
	m_pWebBrowserView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}

////////////////////////////////////////////////////////////////////////
// OnTimer - just make sure it's showing
//
////////////////////////////////////////////////////////////////////////
#ifdef ADWARE_PROTOTYPE
void CWebBrowserWazooWnd::OnTimer(UINT)
{
	// show the window if not visible
	CWazooBar* hoosierDaddy = DYNAMIC_DOWNCAST(CWazooBar,GetParentControlBar());
	if (hoosierDaddy && !hoosierDaddy->IsWindowVisible())
		CMainFrame::QCGetMainFrame()->SendMessage(WM_COMMAND, IDM_VIEW_WEB_BROWSER);
}
#endif //ADWARE_PROTOTYPE

#endif //WEB_BROWSER_WAZOO
