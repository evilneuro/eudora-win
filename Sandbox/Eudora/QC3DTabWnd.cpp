// QC3DTabWnd.cpp : implementation file
//
// QC3DTabWnd
// QC customizations of the SEC3DTabWnd.

#include "stdafx.h"

#include "ClipboardMgr.h"
#include "QC3DTabWnd.h"
#include "QCOleDropSource.h"
#include "WazooWnd.h"
#include "WazooBar.h"
#include "eudora.h"
#include "workbook.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(QC3DTabControl, SEC3DTabControl)
IMPLEMENT_DYNCREATE(QC3DTabWnd, SEC3DTabWnd)


/**********************************************************************/
/*                           QC3DTabCtrl                              */
/**********************************************************************/

BEGIN_MESSAGE_MAP(QC3DTabControl, SEC3DTabControl)
	//{{AFX_MSG_MAP(QC3DTabControl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// QC3DTabControl

QC3DTabControl::QC3DTabControl() :
	m_SavedMouseDownPoint(-1, -1),
	m_MouseState(MOUSE_IDLE),
	m_nSavedTabIndex(-1)
{

}

QC3DTabControl::~QC3DTabControl()
{
}


////////////////////////////////////////////////////////////////////////
// QCGetIcon [public]
//
// Given a tab index, fetch the HICON associated with that tab.
////////////////////////////////////////////////////////////////////////
HICON QC3DTabControl::QCGetIcon(int nTabIndex) const
{
	ASSERT(nTabIndex >= 0 && nTabIndex < GetTabCount());
	SEC3DTab* pTab = GetTabPtr(nTabIndex);
	if (pTab)
		return pTab->m_hIcon;

	ASSERT(0);
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// ActivateTab [public, virtual]
//
// Override base class implementation.
////////////////////////////////////////////////////////////////////////
void QC3DTabControl::ActivateTab(int nTab)
{
	//
	// Before calling the base class implementation to switch to the
	// new active tab, give the old active tab, if any, a chance to
	// do some processing.
	//
	int nActiveTab = -1;
	if (GetActiveTab(nActiveTab))
	{
		if (nTab != nActiveTab)
		{
			LPCTSTR lpszUnused;
			CWazooWnd* pWazooWnd = NULL;
			BOOL bUnused;
			HMENU hUnused;
			void* pUnused;
			GetTabInfo(nActiveTab,
						 lpszUnused, 
						 bUnused, 
						 (CObject *&) pWazooWnd, 			// this is what we want
						 hUnused,
						 pUnused);

			ASSERT(pWazooWnd);
			ASSERT_KINDOF(CWazooWnd, pWazooWnd);

			pWazooWnd->OnDeactivateWazoo();
		}
	}

	//
	// Give the wazoo that's about to be activated a chance to
	// initialize its display.  This implements the Just In Time wazoo
	// display strategy -- that is, wazoo windows should not perform
	// their initial display initializations until the wazoo window is
	// displayed for the first time.
	//
	{
		LPCTSTR lpszUnused;
		CWazooWnd* pWazooWnd = NULL;
		BOOL bUnused;
		HMENU hUnused;
		void* pUnused;
		GetTabInfo(nTab,
					 lpszUnused, 
					 bUnused, 
					 (CObject *&) pWazooWnd, 			// this is what we want
					 hUnused,
					 pUnused);

		ASSERT(pWazooWnd);
		ASSERT_KINDOF(CWazooWnd, pWazooWnd);

		pWazooWnd->OnActivateWazoo();
	}

	SEC3DTabControl::ActivateTab(nTab);
}


/////////////////////////////////////////////////////////////////////////////
// QC3DTabControl message handlers


////////////////////////////////////////////////////////////////////////
// OnLButtonDown [protected]
//
////////////////////////////////////////////////////////////////////////
void QC3DTabControl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//
	// Do all the normal button down stuff first, such as activating
	// the tab window.
	//
	SEC3DTabControl::OnLButtonDown(nFlags, point);

	m_MouseState = MOUSE_IDLE;
	m_SavedMouseDownPoint = CPoint(-1, -1);

	//
	// Do a hit test to see if we landed on a tab.
	//
	for (int i = 0; i < GetTabCount(); i++)
	{
		if (TabHit(i, point))
		{
			//TRACE1("QC3DTabControl::OnLButtonDown: Hit tab %d\n", i);
			m_MouseState = MOUSE_DOWN_ON_TAB;
			m_nSavedTabIndex = i;
			m_SavedMouseDownPoint = point;
			break;
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnLButtonUp [protected]
//
////////////////////////////////////////////////////////////////////////
void QC3DTabControl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//TRACE("QC3DTabControl::OnLButtonUp\n");
	SEC3DTabControl::OnLButtonUp(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// OnMouseMove [protected]
//
////////////////////////////////////////////////////////////////////////
void QC3DTabControl::OnMouseMove(UINT nFlags, CPoint pt) 
{
	if ((nFlags & MK_LBUTTON) && (m_MouseState != MOUSE_IDLE))
	{
		switch (m_MouseState)
		{
		case MOUSE_DOWN_ON_TAB:
			//
			// User is starting a drag operation on a tab.
			// In this case, we bypass the normal selection action and just
			// start the drag operation.
			//
			ASSERT(m_SavedMouseDownPoint != CPoint(-1, -1));
			ASSERT(m_nSavedTabIndex != -1);
			m_MouseState = MOUSE_IS_MOVING;			// indicate we're in the drag part of a click and drag action
			SEC3DTabControl::OnMouseMove(nFlags, pt);
			return;
		case MOUSE_IS_MOVING:
			//
			// User has started a click and drag action.  Check to see if
			// the user has dragged far enough to initiate a drag and
			// drop action.
			//
			ASSERT(m_SavedMouseDownPoint != CPoint(-1, -1));
			if (abs(pt.x - m_SavedMouseDownPoint.x) > 6 ||
				abs(pt.y - m_SavedMouseDownPoint.y) > 6)
			{
				break;		// fall through to drag and drop processing below...
			}
			return;
		case MOUSE_IS_DRAGGING:
			// silently return since OleDataSource is in control
			return;
		default:
			ASSERT(0);
			return;
		}

		//
		// If we get this far, initiate a drag and drop operation.
		// Create global memory for drag 'n drop data.  Since Eudora
		// is both the drop source and the drop target, we put
		// Eudora's HINSTANCE and source Wazoo window information in
		// the buffer to make sure that we don't drag and drop between
		// two different instances of Eudora.
		//
		QC3DTabWnd* pTabWnd = (QC3DTabWnd *) GetParent();
		ASSERT(pTabWnd != NULL);
		ASSERT_KINDOF(QC3DTabWnd, pTabWnd);

		CWazooBar* pWazooBar = (CWazooBar *) pTabWnd->GetParent();
		ASSERT(pWazooBar);
		ASSERT_KINDOF(CWazooBar, pWazooBar);

		LPCTSTR lpszUnused;
		CWazooWnd* pWazooWnd = NULL;
		BOOL bUnused;
		HMENU hUnused;
		void* pUnused;
		GetTabInfo(m_nSavedTabIndex,
					 lpszUnused, 
					 bUnused, 
					 (CObject *&) pWazooWnd, 			// this is what we want
					 hUnused,
					 pUnused);

		TRACE1("QC3DTabControl::OnMouseMove: begin drag: %s\n", lpszUnused);

		ASSERT(pWazooWnd);
		ASSERT_KINDOF(CWazooWnd, pWazooWnd);

		QCFWAZOO qcfWazoo;
		qcfWazoo.m_pSourceWazooBar = pWazooBar;
		qcfWazoo.m_pSourceWazooWnd = pWazooWnd;

		// Cache the HINSTANCE data
		g_theClipboardMgr.CacheGlobalData(m_OleDataSource, qcfWazoo);

		m_MouseState = MOUSE_IS_DRAGGING;
                                             
		//
		// Do the drag and drop!
		//
		QCOleDropSource oleDropSource(TRUE);
		DROPEFFECT dropEffect = m_OleDataSource.DoDragDrop(DROPEFFECT_MOVE|DROPEFFECT_LINK, NULL, &oleDropSource);
 
		// Clear the Data Source's cache
		m_OleDataSource.Empty();

		if (((dropEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE) ||
			((dropEffect & DROPEFFECT_LINK) == DROPEFFECT_LINK))
		{
			//
			// The OLE docs say that this is the place to delete the
			// selected items that have just been moved.
			//
			if (pWazooBar->GetWazooCount() == 0)
			{
				pWazooBar->OnBarFloat();
				VERIFY(pWazooBar->UnlinkFromManager());
				SECDockBar* pDockBar = (SECDockBar *) pWazooBar->m_pDockBar;
				ASSERT_KINDOF(SECDockBar, pDockBar);

				//
				// After much experimentation and guessing about how
				// the SEC docking window code works :-), I'm hopeful
				// that the following calls are sufficient to actually
				// blow away a control bar.  There are no known examples
				// of how to do this anywhere that I could find.
				//
				pDockBar->RemoveControlBar(pWazooBar, -1, FALSE);
				pDockBar->RemovePlaceHolder(pWazooBar);

				//
				// Here's an ugly hack to remove the to-be-deleted control bar
				// from the SECDockBar cache of invalidated control bars.
				// If we don't do this, then an invalid (deleted) pointer
				// remains in the cache and causes a crash later.
				//
				for (int i = 0; i < pDockBar->m_arrInvalidBars.GetSize(); i++) 
				{
					if (pWazooBar == (CWazooBar *) pDockBar->m_arrInvalidBars[i])
					{
						//
						// Oh, my.  There's a big assumption here that there
						// is at most one copy of this pointer in this cache.
						// If there are actually multiple copies of the
						// pointers in the cache, then this loop won't work 
						// since it only removes the first occurrence.
						//
						pDockBar->m_arrInvalidBars.RemoveAt(i);
						break;
					}
				}

				//
				// Need to set the dock bar pointer to NULL as we destroy
				// it, or else bad things will happen in the destructor
				// chains.
				//
				pWazooBar->m_pDockBar = NULL;

				pWazooBar->DestroyWindow();
				delete pWazooBar;
				return;
			}
		}
	}

	m_MouseState = MOUSE_IDLE;
	m_SavedMouseDownPoint = CPoint(-1, -1);
	m_nSavedTabIndex = -1;
	SEC3DTabControl::OnMouseMove(nFlags, pt);
}



/**********************************************************************/
/*                              QC3DTabWnd                            */
/**********************************************************************/

BEGIN_MESSAGE_MAP(QC3DTabWnd, SEC3DTabWnd)
	//{{AFX_MSG_MAP(QC3DTabWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// QC3DTabWnd

QC3DTabWnd::QC3DTabWnd() :
	m_bShowTabs(TRUE),
	m_Draw3DBorder(TRUE)
{
}

QC3DTabWnd::~QC3DTabWnd()
{
}


////////////////////////////////////////////////////////////////////////
// QCGetIcon [public]
//
// Given a tab index, fetch the HICON associated with that tab.
////////////////////////////////////////////////////////////////////////
HICON QC3DTabWnd::QCGetIcon(int nTabIndex) const
{
	if (m_pTabCtrl)
	{
		ASSERT_KINDOF(QC3DTabControl, m_pTabCtrl);
		return ((QC3DTabControl *) m_pTabCtrl)->QCGetIcon(nTabIndex);
	}

	ASSERT(0);
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// QCTabHit [public]
//
// Wrapper for the SEC3DTabControl::TabHit() method.
////////////////////////////////////////////////////////////////////////
BOOL QC3DTabWnd::QCTabHit(int nTabIndex, const CPoint& ptScreen)
{
	if (m_pTabCtrl)
	{
		ASSERT_KINDOF(QC3DTabControl, m_pTabCtrl);
		CPoint ptClient(ptScreen);
		((QC3DTabControl *) m_pTabCtrl)->ScreenToClient(&ptClient);
		return ((QC3DTabControl *) m_pTabCtrl)->TabHit(nTabIndex, ptClient);
	}

	ASSERT(0);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// SetTabLocation [public]
//
// Set the location and visibility of the tabs.  The 0x00F0 bits
// determine the location and the 0x0001 bit determines the visibility
// (0x0001 means hidden).
////////////////////////////////////////////////////////////////////////
void QC3DTabWnd::SetTabLocation(DWORD dwTabLocation)
{
	const BOOL bHideSingleTab = dwTabLocation & 0x1;

	switch (dwTabLocation & TWS_TAB_ORIENTATIONS)
	{
	case TWS_TABS_ON_TOP:
	case TWS_TABS_ON_BOTTOM:
	case TWS_TABS_ON_LEFT:
	case TWS_TABS_ON_RIGHT:
		break;
	default:
		ASSERT(0);					// caller passed bogus value
		dwTabLocation = TWS_TABS_ON_BOTTOM;
		break;
	}

	DWORD dwStyle = GetTabStyle();
	dwStyle &= ~TWS_TAB_ORIENTATIONS;					// clear out old style
	dwStyle |= (dwTabLocation & TWS_TAB_ORIENTATIONS);	// set new style
	SetTabStyle(dwStyle);

	//
	// Hide tab window, if necessary.
	//
	if (bHideSingleTab && (GetTabCount() == 1))
	{
		ShowTabs(FALSE);
	}
	else
		ShowTabs(TRUE);

	RecalcLayout();
}


////////////////////////////////////////////////////////////////////////
// GetTabLocation [public]
//
// Fetch the current tab location and visibility.  The 0x00F0 bits
// denote the location and the 0x0001 bit denotes the visibility
// (0x0001 means hidden).
////////////////////////////////////////////////////////////////////////
DWORD QC3DTabWnd::GetTabLocation() const
{
	DWORD dwStyle = GetTabStyle();
	dwStyle &= TWS_TAB_ORIENTATIONS;		// mask out extraneous bits

	switch (dwStyle)
	{
	case TWS_TABS_ON_TOP:			// 0x0020
	case TWS_TABS_ON_BOTTOM:		// 0x0010
	case TWS_TABS_ON_LEFT:			// 0x0040
	case TWS_TABS_ON_RIGHT:			// 0x0080
		break;
	default:
		ASSERT(0);					// huh? conflicting tab locations?
		return TWS_TABS_ON_BOTTOM;
	}

	//
	// If this is a single, hidden tab, then set the 0x1 bit.
	//
	ASSERT((dwStyle & 0xF) == 0);	// assume 0x1 bit is unused
	if (! m_bShowTabs)
	{
		ASSERT(m_pTabCtrl && (m_pTabCtrl->GetTabCount() == 1));
		dwStyle |= 0x0001;			// set "hidden" bit
	}

	return dwStyle;
}


////////////////////////////////////////////////////////////////////////
// ShowTabs [public]
//
////////////////////////////////////////////////////////////////////////
void QC3DTabWnd::ShowTabs(BOOL bShowTabs)
{
	ASSERT(m_pTabCtrl->GetSafeHwnd());

	if ((! bShowTabs) && (GetTabCount() != 1))
	{
		ASSERT(0);		// can't hide if there are more than one tab
		return;
	}

	if (m_bShowTabs != bShowTabs)
	{
		m_bShowTabs = bShowTabs;
		Invalidate();
		RecalcLayout();
	}
}


////////////////////////////////////////////////////////////////////////
// TabsAreVisible [public]
//
////////////////////////////////////////////////////////////////////////
BOOL QC3DTabWnd::TabsAreVisible() const
{
	return m_bShowTabs;
}


////////////////////////////////////////////////////////////////////////
// CreateTabCtrl [protected, virtual]
//
// Override of implementation of base class virtual.
////////////////////////////////////////////////////////////////////////
BOOL QC3DTabWnd::CreateTabCtrl(DWORD dwStyle, UINT nID)
{
    ASSERT_VALID(this);

	m_pTabCtrl = new QC3DTabControl();
    ASSERT_VALID(this);
	VERIFY(m_pTabCtrl->Create(WS_VISIBLE | m_dwTabCtrlStyle,
		CRect(0, 0, 0, 0), this, nID));

	dwStyle; // UNUSED
	return (m_pTabCtrl != NULL);
}


////////////////////////////////////////////////////////////////////////
// RecalcLayout [protected, virtual]
//
// Override of implementation of base class virtual to hide tabs.
////////////////////////////////////////////////////////////////////////
void QC3DTabWnd::RecalcLayout()
{
    ASSERT_VALID(this);

	if (m_bShowTabs)
	{
		SEC3DTabWnd::RecalcLayout();
		return;
	}

	//
	// m_cxTabCtrl and m_cyTabCtrl are used by
	// SEC3DTabWnd::GetInsideRect(), so they must be computed here as
	// in the base class SEC3DTabWnd::RecalcLayout() implementation.
	//
	if (m_dwTabCtrlStyle & TCS_TABS_ON_BOTTOM)
	{
		m_cyTabCtrl = 3;
		m_cxTabCtrl = 0;
	}
	else if (m_dwTabCtrlStyle & TCS_TABS_ON_TOP)
	{
		m_cyTabCtrl = 3;
		m_cxTabCtrl = 0;
	}
	else if (m_dwTabCtrlStyle & TCS_TABS_ON_LEFT)
	{
		m_cyTabCtrl = 0;
		m_cxTabCtrl = 5;
	}
	else // if (m_dwTabCtrlStyle & TCS_TABS_ON_RIGHT)
	{
		m_cyTabCtrl = 0;
		m_cxTabCtrl = 5;
	}

    CRect rectClient;
    GetClientRect(rectClient);

	rectClient.top += 4;
	rectClient.bottom -= 4;
	rectClient.left += 6;
	rectClient.right -= 7;

    HDWP hDWP = ::BeginDeferWindowPos(4);

    // Resize tab control
    // Note, the tab control draws it's own border, so
    // no need to subtract border pixels
    CWnd* pTabCtrl = GetDlgItem(SEC_IDW_TABCTRL);     
    ASSERT(pTabCtrl != NULL);
    hDWP = ::DeferWindowPos(hDWP, pTabCtrl->m_hWnd, NULL, 0, 0, 0, 0, 
    						SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);

	if (m_pActiveWnd)
		hDWP = ::DeferWindowPos(hDWP, m_pActiveWnd->m_hWnd, NULL,
								rectClient.left, rectClient.top,
								rectClient.Width()-1, rectClient.Height(), 
								SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

    ::EndDeferWindowPos(hDWP);
}


/////////////////////////////////////////////////////////////////////////////
// QC3DTabWnd message handlers

void QC3DTabWnd::OnPaint()
{
	if (m_Draw3DBorder)
		SEC3DTabWnd::OnPaint();
	else
		Default();
}
