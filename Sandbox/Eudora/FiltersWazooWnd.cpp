// FiltersWazooWnd.cpp : implementation file
//
// CFiltersWazooWnd
// Specific implementation of a CWazooWnd.

#include "stdafx.h"

#include "rs.h"
#include "doc.h"
#include "resource.h"
#include "utils.h"
#include "summary.h"

#include "tocdoc.h"			// for FILTERSD.H
#include "3dformv.h"		// for FILTERSV.H
#include "controls.h"
#include "filtersd.h"		// for FILTERSV.H
#include "filtersv.h"
#include "FiltersWazooWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFiltersWazooWnd

IMPLEMENT_DYNCREATE(CFiltersWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CFiltersWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CFiltersWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CFiltersWazooWnd::CFiltersWazooWnd() : CWazooWnd(IDR_FILTERS)
{
}

CFiltersWazooWnd::~CFiltersWazooWnd()
{
}


////////////////////////////////////////////////////////////////////////
// DestroyWindow [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CFiltersWazooWnd::DestroyWindow() 
{
	int cxCur, cxMin;
	m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
	if (cxCur >= 0)
	{
		//
		// Empirically, we find that 'cxCur' is -1 if the filters
		// window has never been shown during a given Eudora
		// session.  Therefore, we only save the position if 
		// the window has been made visible at least once.
		//
		SetIniLong(IDS_INI_FILTERS_WINDOW_SPLITER, cxCur);
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
BOOL CFiltersWazooWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
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
void CFiltersWazooWnd::OnDeactivateWazoo()
{
	ASSERT(::IsWindow(m_wndSplitter.GetSafeHwnd()));

	//
	// Give the parent implementation a chance to save the keyboard focus.
	//
	CWazooWnd::OnDeactivateWazoo();

	//
	// Notify the document that it has been deactivated so that it can
	// prompt the user for saving any unsaved changes.  FORNOW, this is
	// totally too late since by the time we get here, the deactivation
	// has already occurred.
	//
	GetFiltersDoc()->CanCloseFrame(NULL);
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Override for virtual base class method.  This gets called when the
// filters window tab is activated
////////////////////////////////////////////////////////////////////////
void CFiltersWazooWnd::OnActivateWazoo()
{
	GetFiltersDoc();
}


/////////////////////////////////////////////////////////////////////////////
// CFiltersWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
////////////////////////////////////////////////////////////////////////
int CFiltersWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	//
	// We just need a bogus context to keep CWndSplitter::CreateView()
	// happy.
	//
	CCreateContext myBogusContext;
	
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((! m_wndSplitter.CreateStatic(this, 1, 2)) ||
		(! m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CFiltersViewLeft), CSize(130,130), &myBogusContext)) ||
		(! m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CFiltersViewRight), CSize(0,0), &myBogusContext)))
	{
		return -1;
	}

	int nSplit = int(GetIniLong(IDS_INI_FILTERS_WINDOW_SPLITER));
	if (nSplit < 0)
	{
		ASSERT(0);
		nSplit = 140;
	}
	m_wndSplitter.SetColumnInfo(0, nSplit, 0);
	m_wndSplitter.SetColumnInfo(1, 0, 0);
//FORNOW	m_wndSplitter.SetActivePane(0, 0);

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CFiltersWazooWnd::OnSize(UINT nType, int cx, int cy) 
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
void CFiltersWazooWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	lpMMI->ptMinTrackSize.x += 420; 
	lpMMI->ptMinTrackSize.y += 360;

	CWnd::OnGetMinMaxInfo(lpMMI);
}
