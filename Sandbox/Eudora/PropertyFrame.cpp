// PropertyFrame.cpp : implementation file
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>

#include "resource.h"

#include "rs.h"
#include "doc.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "msgdoc.h"
#include "readmsgd.h"
#include "PropertyFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyFrame

IMPLEMENT_DYNCREATE(CPropertyFrame, CMiniFrameWnd)

//CPropertyFrame::CPropertyFrame()
CPropertyFrame::CPropertyFrame(CDocument * pDoc)
{
	m_pModelessPropSheet = NULL;
	m_pDoc = pDoc;
}

CPropertyFrame::~CPropertyFrame()
{
/*	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// or CMDIChildWnd *pChild = pFrame->MDIGetActive();

	// Get the active view attached to the active MDI child
	// window.
	CSampleView *pView = (CSampleView *) pChild->GetActiveView();
	(pView->GetDocument())->m_pPropFrame = NULL;
*/
	if (m_pDoc)
		((CReadMessageDoc*)m_pDoc)->m_pPropFrame = NULL;
}


BEGIN_MESSAGE_MAP(CPropertyFrame, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CPropertyFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATE()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyFrame message handlers

int CPropertyFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pModelessPropSheet = new CPropertyPanel(this, m_pDoc);
	if (!m_pModelessPropSheet->Create(this, 
		WS_CHILD | WS_VISIBLE, 0))
	{
		delete m_pModelessPropSheet;
		m_pModelessPropSheet = NULL;
		return -1;
	}

	// Resize the mini frame so that it fits around the child property
	// sheet.
	CRect rectClient, rectWindow;
	m_pModelessPropSheet->GetWindowRect(rectClient);

	rectWindow = rectClient;

	// CMiniFrameWnd::CalcWindowRect adds the extra width and height
	// needed from the mini frame.
	CalcWindowRect(rectWindow);
	SetWindowPos(NULL, rectWindow.left, rectWindow.top,
		rectWindow.Width(), rectWindow.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE);
	m_pModelessPropSheet->SetWindowPos(NULL, 0, 0,
		rectClient.Width(), rectClient.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE);

	CWnd * pTemp = GetWindow(GW_OWNER);
	SetWindowPos(NULL, 15, 15,
		0, 0,
		SWP_NOZORDER | SWP_NOSIZE);

	return 0;
}

void CPropertyFrame::OnClose() 
{
	if (m_pModelessPropSheet->m_bMultiWin)
	{
//		SendMessage(WM_COMMAND, ID_PROPFRAME_PTR, 0L);
		CMiniFrameWnd::OnClose();
	}
	else
		// Instead of closing the modeless property sheet, just hide it.
		ShowWindow(SW_HIDE);
}

void CPropertyFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMiniFrameWnd::OnSetFocus(pOldWnd);
}

void CPropertyFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CMiniFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	// Forward any WM_ACTIVATEs to the property sheet...
	// Like the dialog manager itself, it needs them to save/restore the focus.
	ASSERT_VALID(m_pModelessPropSheet);

	// Use GetCurrentMessage to get unmodified message data.
	const MSG* pMsg = GetCurrentMessage();
	ASSERT(pMsg->message == WM_ACTIVATE);
	m_pModelessPropSheet->SendMessage(pMsg->message, pMsg->wParam,
		pMsg->lParam);

//	if ((nState == WA_INACTIVE) && !(m_pModelessPropSheet->m_bPressedTack))
//		OnClose();
}

void CPropertyFrame::OnKillFocus(CWnd* pNewWnd) 
{
	CMiniFrameWnd::OnKillFocus(pNewWnd);
	
//	if (!m_pModelessPropSheet->m_bPressedTack)
//		OnClose();
}

