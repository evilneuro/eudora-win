// NicknamesWazooWnd.cpp : implementation file
//
// CNicknamesWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "doc.h"
#include "resource.h"
#include "utils.h"
#include "summary.h"

#include "3dformv.h"
#include "nickdoc.h"
#include "eudora.h"
#include "urledit.h"	// for NICKPAGE.H
#include "nickpage.h"	// for NICKSHT.H
#include "nicksht.h"	// for NICKVIEW.H
#include "nicktree.h"	// for NICKVIEW.H
#include "nickview.h"
#include "NicknamesWazooWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CNicknamesWazooWnd

IMPLEMENT_DYNCREATE(CNicknamesWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CNicknamesWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CNicknamesWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CNicknamesWazooWnd::CNicknamesWazooWnd() : CWazooWnd(IDR_NICKNAMES),
	m_nRHSWidth(0)
{
}

CNicknamesWazooWnd::~CNicknamesWazooWnd()
{
}


////////////////////////////////////////////////////////////////////////
// DestroyWindow [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesWazooWnd::DestroyWindow() 
{
	int cxCur, cxMin;
	m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
	if (cxCur >= 0)
	{
		//
		// Empirically, we find that 'cxCur' is -1 if the nicknames
		// window has never been shown during a given Eudora
		// session.  Therefore, we only save the position if 
		// the window has been made visible at least once.
		//
		SetIniLong(IDS_INI_NICKNAMES_WINDOW_SPLITTER, cxCur);
	}
	
	return CWnd::DestroyWindow();
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override for virtual CCmdTarget::OnCmdMsg() method.  The idea is
// to change the standard command routing to forward commands from the
// Wazoo control bar all the way to the active splitter pane view.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesWazooWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_wndSplitter.GetSafeHwnd())
	{
		CWnd* pActivePane = m_wndSplitter.GetActivePane();
		if (pActivePane)
			return pActivePane->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		return FALSE;
	}

	ASSERT(0);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnDeactivateWazoo [public, virtual]
//
// Override for virtual base class method.  This gets called when the
// filters window tab is deactivated or when the Wazoo container is
// closed.
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnDeactivateWazoo()
{
	ASSERT(::IsWindow(m_wndSplitter.GetSafeHwnd()));

	//
	// Give the parent implementation a chance to save the keyboard focus.
	//
	CWazooWnd::OnDeactivateWazoo();

	//
	// Notify the document that it has been deactivated so that it can
	// prompt the user for saving any unsaved changes.
	//
	ASSERT(g_Nicknames);
	g_Nicknames->CanCloseFrame(NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CNicknamesWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CNicknamesWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	//
	// We just need a bogus context to keep CWndSplitter::CreateView()
	// happy.
	//
	CCreateContext myBogusContext;
	
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((! m_wndSplitter.CreateStatic(this, 1, 2)) ||
		(! m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CNicknamesViewLeft32), CSize(154,154), &myBogusContext)) ||
		(! m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CNicknamesViewRight), CSize(0,0), &myBogusContext)))
	{
		return -1;
	}

#ifdef _DEBUG
	{
		// FORNOW, take a looksee at the RHS view
		CView* pViewRight = (CView *) m_wndSplitter.GetPane(0, 1);
		ASSERT(pViewRight);
		ASSERT_KINDOF(CView, pViewRight);
	}
#endif // _DEBUG

	// Restore the previous split window sizes from the INI file.
	int nSplit = int(GetIniLong(IDS_INI_NICKNAMES_WINDOW_SPLITTER));
	if (nSplit < 0)
	{
		ASSERT(0);
		nSplit = 80;
	}
	m_wndSplitter.SetColumnInfo(0, nSplit, 0);
	m_wndSplitter.SetColumnInfo(1, 0, 0);

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	ASSERT(m_wndSplitter.GetSafeHwnd());
	m_wndSplitter.SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnGetMinMaxInfo [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
#ifdef _DEBUG
	CString msg;
	msg.Format("CNicknamesWazooWnd::OnGetMinMaxInfo: "
				"maxSize(%d,%d) "
				"maxPos(%d,%d) "
				"minTrack(%d,%d) "
				"maxTrack(%d,%d)\n",
				lpMMI->ptMaxSize.x, lpMMI->ptMaxSize.y,
				lpMMI->ptMaxPosition.x, lpMMI->ptMaxPosition.y,
				lpMMI->ptMinTrackSize.x, lpMMI->ptMinTrackSize.y,
				lpMMI->ptMaxTrackSize.x, lpMMI->ptMaxTrackSize.y);
	//TRACE0(msg);
#endif // _DEBUG

	lpMMI->ptMinTrackSize.x = 140;		//FORNOW, hardcoded
	lpMMI->ptMinTrackSize.y = 240;		//FORNOW, hardcoded

	CWnd::OnGetMinMaxInfo(lpMMI);
}


////////////////////////////////////////////////////////////////////////
// ToggleRHSDisplay [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::ToggleRHSDisplay() 
{
	//
	// First thing to do is to figure out whether or the RHS is
	// "open" or "closed".
	//
	int nRHSCurrent;
	int nRHSMin;
	m_wndSplitter.GetColumnInfo(1, nRHSCurrent, nRHSMin);

	//
	// Grab the current LHS pane size.
	//
	int nLHSCurrent;
	int nLHSMin;
	m_wndSplitter.GetColumnInfo(0, nLHSCurrent, nLHSMin);

	//
	// Grab current frame window and client area sizes.
	//
	CRect rect;
	GetWindowRect(&rect);
	CRect clientrect;
	GetClientRect(&clientrect);

	//
	// FORNOW, this is a hardcoded kludge factor.  The real solution
	// is to query the CWndSplitter border metrics.
	//
	const int FUDGE = (IsVersion4() ? 2 : 1);

	if (nRHSCurrent)
	{
//FORNOW		//
//FORNOW		// Shrinking the RHS window...  FORNOW, IsZoomed() probably
//FORNOW		// doesn't work for floating/docked control bars!
//FORNOW		//
//FORNOW		if (IsZoomed())
//FORNOW		{
			m_wndSplitter.SetColumnInfo(0, clientrect.Width(), nLHSMin);
			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}
//FORNOW		else
//FORNOW		{
//FORNOW			int width = nLHSCurrent + 												// LHS pane width
//FORNOW						(clientrect.Width() - nLHSCurrent - nRHSCurrent - FUDGE) +	// splitter bar width
//FORNOW						(rect.Width() - clientrect.Width());						// window borders
//FORNOW			SetWindowPos(0, 0, 0, width, rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
//FORNOW			m_wndSplitter.SetColumnInfo(0, nLHSCurrent, nLHSMin);
//FORNOW			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}

		//
		// Save old RHS width so that we can restore it next time.
		//
		m_nRHSWidth = nRHSCurrent;
	}
	else
	{
		//
		// Growing the RHS window...
		//
		if (0 == m_nRHSWidth)
			m_nRHSWidth = 210;		// FORNOW, use minimum

//FORNOW		if (IsZoomed())
//FORNOW		{
			m_wndSplitter.SetColumnInfo(0, clientrect.Width() - m_nRHSWidth - (clientrect.Width() - nLHSCurrent) - FUDGE, nLHSMin);
			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}
//FORNOW		else
//FORNOW		{
//FORNOW			int width = rect.Width() + m_nRHSWidth + FUDGE;
//FORNOW			SetWindowPos(0, 0, 0, width, rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
//FORNOW			m_wndSplitter.SetColumnInfo(0, nLHSCurrent, nLHSMin);
//FORNOW			m_wndSplitter.SetColumnInfo(1, 0, nRHSMin);
//FORNOW		}

		m_nRHSWidth = 0;
	}

	m_wndSplitter.RecalcLayout();
//FORNOW	RecalcLayout();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to left hand Nicknames view.
////////////////////////////////////////////////////////////////////////
void CNicknamesWazooWnd::OnActivateWazoo()
{
	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		CWnd* pLeftPane = m_wndSplitter.GetPane(0, 0);

		if (pLeftPane)
			pLeftPane->SendMessage(umsgActivateWazoo);
	}
}


