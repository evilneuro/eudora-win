// MboxWazooWnd.cpp : implementation file
//
// CMboxWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "utils.h"

#include "MboxWazooWnd.h"
#include "ContainerView.h"

#include "mainfrm.h"			// FORNOW UGLY HACK

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMboxWazooWnd

IMPLEMENT_DYNCREATE(CMboxWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CMboxWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CMboxWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()


CMboxWazooWnd::CMboxWazooWnd() : CWazooWnd(IDR_MAILBOXES),
	m_pMailboxesView(NULL)
{
}

CMboxWazooWnd::~CMboxWazooWnd()
{
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to change the standard command routing to forward commands from the
// Wazoo control bar all the way to the embedded tree control.
////////////////////////////////////////////////////////////////////////
BOOL CMboxWazooWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_MboxTreeCtrl.GetSafeHwnd())
		if (m_MboxTreeCtrl.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	return CWazooWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


/////////////////////////////////////////////////////////////////////////////
// CMboxWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CMboxWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pMailboxesView = new CContainerView;
	if (m_pMailboxesView->Create(this) == FALSE)
	{
		TRACE("Failed to create Mailboxes view");
		return -1;
	}

	if (m_MboxTreeCtrl.Create(WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS,
			CRect(0, 0, 0, 0), m_pMailboxesView, IDC_TREE1) == FALSE)
	{
		TRACE("Failed to create Mailboxes tree");
		return -1;
	}

	//
	// FORNOW, total ugly hack to "register" the one and only mailbox
	// tree control with the main app window.
	//
	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	ASSERT(NULL == pMainWnd->m_pActiveMboxTreeCtrl);			//FORNOW
	pMainWnd->m_pActiveMboxTreeCtrl = &m_MboxTreeCtrl;			//FORNOW

	if (! m_MboxTreeCtrl.Init())
	{
		TRACE("Failed to initialize Mailboxes tree");
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Hook standard MFC message to initialize Wazoo window focus.
////////////////////////////////////////////////////////////////////////
LRESULT CMboxWazooWnd::OnInitialUpdate(WPARAM, LPARAM)
{
	CWazooWnd::SaveInitialChildFocus(&m_MboxTreeCtrl);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CMboxWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	m_pMailboxesView->SetWindowPos(NULL, rectClient.left, rectClient.top,
									rectClient.Width(), rectClient.Height(),
									SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

	m_MboxTreeCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top,
									rectClient.Width(), rectClient.Height(),
									SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnChar [protected]
//
// Scan for VK_RETURN and VK_ESCAPE key codes and forward them to
// the tree control.
////////////////////////////////////////////////////////////////////////
void CMboxWazooWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_RETURN:
		m_MboxTreeCtrl.OnEnterKeyPressed();
		break;
	case VK_ESCAPE:
		m_MboxTreeCtrl.OnEscKeyPressed();
		break;
	default:
		CWazooWnd::OnChar(nChar, nRepCnt, nFlags);
		break;
	}
}

