// PersonalityWazooWnd.cpp : implementation file
//
// CPersonalityWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "helpcntx.h"
#include "PersonalityView.h"
#include "PersonalityWazooWnd.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPersonalityWazooWnd

IMPLEMENT_DYNCREATE(CPersonalityWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CPersonalityWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CPersonalityWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)
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
		if (m_pPersonalityView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	return CWazooWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


/////////////////////////////////////////////////////////////////////////////
// CPersonalityWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CPersonalityWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(NULL == m_pPersonalityView);
	m_pPersonalityView = new CPersonalityView;
	if (NULL == m_pPersonalityView)
		return -1;
	//
	// FORNOW, total ugly hack to "register" the one and only personality
	// view with the main app window.
	//
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	ASSERT(NULL == pMainWnd->m_pActivePersonalityView);		//FORNOW
	pMainWnd->m_pActivePersonalityView = m_pPersonalityView;	//FORNOW

	m_pPersonalityView->Create(NULL, NULL, WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1));
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Hook standard MFC message to initialize Wazoo window focus.
////////////////////////////////////////////////////////////////////////
LRESULT CPersonalityWazooWnd::OnInitialUpdate(WPARAM, LPARAM)
{
	CWazooWnd::SaveInitialChildFocus(m_pPersonalityView);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CPersonalityWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_pPersonalityView);
	m_pPersonalityView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
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
		if (pTempPopup != NULL)
		{
			CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
			DoContextMenu(this, ptScreen, pTempPopup->GetSafeHmenu());
		}
		else
		{
			ASSERT(0);		// resources hosed?
		}

		menuBar.DestroyMenu();
		return;
	}

	CWazooWnd::OnContextMenu(pWnd, ptScreen);
}


