////////////////////////////////////////////////////////////////////////
// File: workbook.cpp
//
// These are overrides of the standard SECWorkbook and SECWorksheet 
// classes which implement some Eudora-specific behavior.
////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "resource.h"
#include "eudora.h"		// for IsWin32s()
#include "helpcntx.h"	// for CContextMenu
#include "workbook.h"
#include "rs.h"
#include "QCGraphics.h"
#include "utils.h"
#include "WazooBar.h"
#include "guiutils.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(QCWorksheet, SECWorksheet);
IMPLEMENT_DYNCREATE(QCControlBarWorksheet, SECControlBarWorksheet);
IMPLEMENT_DYNCREATE(QCMiniDockFrameWnd, SECMiniDockFrameWnd);
IMPLEMENT_DYNCREATE(QCWorkbook, SECWorkbook);
  

/*****************************************************************************/
/*                              QCWorksheet                                  */
/*****************************************************************************/

BEGIN_MESSAGE_MAP(QCWorksheet, SECWorksheet)
	//{{AFX_MSG_MAP(QCWorksheet)
	ON_COMMAND(ID_MDI_TASKBAR_RESTORE, OnCmdMdiRestore)
	ON_COMMAND(ID_MDI_TASKBAR_MINIMIZE, OnCmdMdiMinimize)
	ON_COMMAND(ID_MDI_TASKBAR_MAXIMIZE, OnCmdMdiMaximize)
	ON_COMMAND(ID_MDI_TASKBAR_CLOSE, OnCmdMdiClose)
	ON_UPDATE_COMMAND_UI(ID_MDI_TASKBAR_RESTORE, OnCmdUpdateMdiRestore)
	ON_UPDATE_COMMAND_UI(ID_MDI_TASKBAR_MINIMIZE, OnCmdUpdateMdiMinimize)
	ON_UPDATE_COMMAND_UI(ID_MDI_TASKBAR_MAXIMIZE, OnCmdUpdateMdiMaximize)
	ON_WM_MDIACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

QCWorksheet::QCWorksheet()
{
}

QCWorksheet::~QCWorksheet()
{
}


////////////////////////////////////////////////////////////////////////
// LoadFrame [public, virtual]
//
// Override base class implementation to reset the 'm_hIcon' member to
// reference the 16x16 frame window icon, rather than the 32x32 default
// frame window icon.  This gives much better display results.
////////////////////////////////////////////////////////////////////////
BOOL QCWorksheet::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
			     CWnd* pParentWnd, CCreateContext* pContext)
{
    BOOL bRtn;

    bRtn = SECWorksheet::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);

	{
		//
		// Load the 16x16 small icon, if any.
		//
		HINSTANCE hResInst = QCFindResourceHandle(MAKEINTRESOURCE(nIDResource), RT_GROUP_ICON);
		HICON hSmallIcon = HICON(::LoadImage(hResInst, MAKEINTRESOURCE(nIDResource), IMAGE_ICON, 16, 16,0));
		if (hSmallIcon)
		{
			//
			// Overwrite the standard 32x32 icon only if we successfully
			// find a small icon.  See the base class implementation of
			// SECWorksheet::LoadFrame() for details on loading the
			// standard icon.
			//
			m_hIcon = hSmallIcon;
		}
	}
    
    return bRtn;
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateMdiRestore [protected]
//
////////////////////////////////////////////////////////////////////////
void QCWorksheet::OnCmdUpdateMdiRestore(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsIconic() || IsZoomed());
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateMdiMinimize [protected]
//
////////////////////////////////////////////////////////////////////////
void QCWorksheet::OnCmdUpdateMdiMinimize(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(! IsIconic());
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateMdiMaximize [protected]
//
////////////////////////////////////////////////////////////////////////
void QCWorksheet::OnCmdUpdateMdiMaximize(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(! IsZoomed());
}


////////////////////////////////////////////////////////////////////////
// OnMDIActivate [protected]
//
// Yuck.  Override base class implementation to hook MDI window
// activations.  All we're trying to do here is to invalidate the "tab
// buttons" so that they get repainted correctly by the QCWorkbook
// paint code.  The yucky thing here is that we're completely
// bypassing the base class implementation.  The reason is that we need
// to inflate the tab button rects a bit so that the repainting goes okay.
////////////////////////////////////////////////////////////////////////
void QCWorksheet::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
#ifdef _DEBUG
	if (GetSafeHwnd())
	{
		CString strTitle;
		GetWindowText(strTitle);
		//TRACE2("QCWorksheet::OnMDIActivate(%d) - %s\n", bActivate, (const char *) strTitle);
	}
#endif // _DEBUG

	//
	// HACK ALERT.  Deliberately bypass the parent class implementation in
	// favor of the grandparent class implementation.
	//
	SECMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	QCWorkbook* p_workbook = (QCWorkbook *) GetWorkbook();
	ASSERT_KINDOF(QCWorkbook, p_workbook);
	CRect tabRect;

    if (pDeactivateWnd && pDeactivateWnd->IsKindOf(RUNTIME_CLASS(SECWorksheet)))
    {
		p_workbook->QCGetTabRect((SECWorksheet *) pDeactivateWnd, tabRect);
		tabRect.InflateRect(2, 2);
		p_workbook->InvalidateRect(tabRect, TRUE);
    }
    if (pActivateWnd && pActivateWnd->IsKindOf(RUNTIME_CLASS(SECWorksheet)))
    {
		p_workbook->QCGetTabRect((SECWorksheet *) pActivateWnd, tabRect);
		tabRect.InflateRect(2, 2);
		p_workbook->InvalidateRect(tabRect, TRUE);
    }
}


/*****************************************************************************/
/*                         QCControlBarWorksheet                             */
/*****************************************************************************/

BEGIN_MESSAGE_MAP(QCControlBarWorksheet, SECControlBarWorksheet)
	//{{AFX_MSG_MAP(QCControlBarWorksheet)
	ON_COMMAND(ID_FILE_CLOSE, OnCmdMdiClose)
	ON_COMMAND(ID_MDI_TASKBAR_RESTORE, OnCmdMdiRestore)
	ON_COMMAND(ID_MDI_TASKBAR_MINIMIZE, OnCmdMdiMinimize)
	ON_COMMAND(ID_MDI_TASKBAR_MAXIMIZE, OnCmdMdiMaximize)
	ON_COMMAND(ID_MDI_TASKBAR_CLOSE, OnCmdMdiClose)
	ON_UPDATE_COMMAND_UI(ID_MDI_TASKBAR_RESTORE, OnCmdUpdateMdiRestore)
	ON_UPDATE_COMMAND_UI(ID_MDI_TASKBAR_MINIMIZE, OnCmdUpdateMdiMinimize)
	ON_UPDATE_COMMAND_UI(ID_MDI_TASKBAR_MAXIMIZE, OnCmdUpdateMdiMaximize)
	ON_WM_MDIACTIVATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

QCControlBarWorksheet::QCControlBarWorksheet() :
	m_bFirstActivationAfterClose(TRUE)
{
}

QCControlBarWorksheet::~QCControlBarWorksheet()
{
}

////////////////////////////////////////////////////////////////////////
// QCSetIcon [public]
//
// Set the icon of the controlbar worksheet.  Need to update the
// window itself so that the icon in the upper-left corner of the
// window will show what the current tab is.
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::QCSetIcon(HICON hIcon)
{
	m_hIcon = hIcon;

	SetIcon(m_hIcon, FALSE);
}

////////////////////////////////////////////////////////////////////////
// RecalcLayout [protected]
//
// Override annoying base class behavior where it constantly resets 
// the frame window title.
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::RecalcLayout(BOOL bNotify /*=TRUE*/)
{
	if (! m_bInRecalcLayout)
	{
		//
		// UGLY HACK to bypass parent implementation in favor of
		// grandparent implementation.
		//
		SECWorksheet::RecalcLayout(bNotify);

//FORNOW		// syncronize window text of frame window with dockbar itself
//FORNOW		CString strTitle;
//FORNOW		m_wndDockBar.GetWindowText(strTitle);
//FORNOW		AfxSetWindowText(m_hWnd, strTitle);
	}
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateMdiRestore [protected]
//
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::OnCmdUpdateMdiRestore(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsIconic() || IsZoomed());
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateMdiMinimize [protected]
//
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::OnCmdUpdateMdiMinimize(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(! IsIconic());
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateMdiMaximize [protected]
//
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::OnCmdUpdateMdiMaximize(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(! IsZoomed());
}


////////////////////////////////////////////////////////////////////////
// OnMDIActivate [protected]
//
// Yuck.  Override SECWorksheet base class implementation to hook MDI
// window activations.  All we're trying to do here is to invalidate
// the "tab buttons" so that they get repainted correctly by the
// QCWorkbook paint code.  The yucky thing here is that we're
// completely bypassing the base class implementation.  The reason is
// that we need to inflate the tab button rects a bit so that the
// repainting goes okay.
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
#ifdef _DEBUG
	if (GetSafeHwnd())
	{
		CString strTitle;
		GetWindowText(strTitle);
		//TRACE2("QCControlBarWorksheet::OnMDIActivate(%d) - %s\n", bActivate, (const char *) strTitle);
	}
#endif // _DEBUG

	//
	// HACK ALERT.  Deliberately bypass the parent class implementation in
	// favor of the great-grandparent class implementation.  :-)
	//
	SECMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	QCWorkbook* p_workbook = (QCWorkbook *) GetWorkbook();
	ASSERT_KINDOF(QCWorkbook, p_workbook);

	if (bActivate)
	{
		//
		// Force the active wazoo to take the focus upon MDI activation.
		//
		if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
	    {
			SECControlBar* pBar = ((SECDockBar *) &m_wndDockBar)->GetFirstControlBar();
			if (pBar != NULL) 
			{
				CWazooBar* pWazooBar = DYNAMIC_DOWNCAST(CWazooBar, pBar);
				ASSERT(pWazooBar);

				CWnd* pActiveWazoo = (CWnd *) pWazooBar->GetActiveWazooWindow();
				if (pActiveWazoo)
					pActiveWazoo->SetFocus();
			}
		}

		//
		// HACK ALERT.  Check to see if current MDI window is maximized or not so
		// that we can properly restore the maximized state of the MDI child
		// being activated.
		//
		ASSERT(pActivateWnd);
		ASSERT(pActivateWnd == this);
		CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
		ASSERT(pMainFrame);
		ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

		BOOL bIsMaximized = FALSE;
		CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);
		if (pActiveMDIChild && bIsMaximized)
		{
//			ShowWindow(SW_SHOWMAXIMIZED);
//			RecalcLayout();
//			pMainFrame->PostMessage(WM_MDIRESTORE, WPARAM(GetSafeHwnd()), 0);
//			pMainFrame->PostMessage(WM_MDIMAXIMIZE, WPARAM(GetSafeHwnd()), 0);
//			BOOL FORNOW = IsZoomed();
//			MDIRestore();
//			FORNOW = IsZoomed();
			if (m_bFirstActivationAfterClose)
			{
				//
				// HACK ALERT.  This is an ugly hack to fix
				// maximized child window redraw problems.
				//
//				pMainFrame->m_pWBClient->SendMessage(WM_MDIRESTORE, WPARAM(GetSafeHwnd()), 0);
//				pMainFrame->m_pWBClient->SendMessage(WM_MDIMAXIMIZE, WPARAM(GetSafeHwnd()), 0);
				MDIRestore();
				PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
			}
//			ActivateFrame(SW_SHOWMAXIMIZED);
//			MDIMaximize();
//			FORNOW = IsZoomed();
//			RecalcLayout();
//			CRect rectClient;
//			GetClientRect(rectClient);
//			OnSize(SIZE_MAXIMIZED, rectClient.Width(), rectClient.Height());
//			pMainFrame->RecalcLayout();
		}

		if (GetStyle() & WS_VISIBLE)
			m_bFirstActivationAfterClose = FALSE;
	}

	CRect tabRect;
    if (pDeactivateWnd && pDeactivateWnd->IsKindOf(RUNTIME_CLASS(SECWorksheet)))
    {
		p_workbook->QCGetTabRect((SECWorksheet *) pDeactivateWnd, tabRect);
		tabRect.InflateRect(2, 2);
		p_workbook->InvalidateRect(tabRect, TRUE);
    }
    if (pActivateWnd && pActivateWnd->IsKindOf(RUNTIME_CLASS(SECWorksheet)))
    {
		p_workbook->QCGetTabRect((SECWorksheet *) pActivateWnd, tabRect);
		tabRect.InflateRect(2, 2);
		p_workbook->InvalidateRect(tabRect, TRUE);
    }
}


////////////////////////////////////////////////////////////////////////
// OnClose [protected]
//
// Yuck.  Override SECControlBarWorksheet base class implementation to
// handle closing MDI child windows.  We do this since we don't want
// to destroy the window.  We really just hide it.
////////////////////////////////////////////////////////////////////////
void QCControlBarWorksheet::OnClose() 
{
	if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
    {
		SECControlBar* pBar = ((SECDockBar *) &m_wndDockBar)->GetFirstControlBar();
		if (pBar != NULL) 
		{
			CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
			ASSERT(pMainFrame);
			ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

			BOOL bWasMaximized = FALSE;
			if (IsZoomed())
			{
//FORNOW				pMainFrame->SetRedraw(FALSE);
				MDIRestore();  // If MDI child is maximized, must restore before hiding.
				bWasMaximized = TRUE;
			}

			//
			// Determine whether or not this was the active MDI window.
			// If so, then activate the next MDI window, if any.
			// 
			CMDIChildWnd* pNextActive = NULL;
			if (this == pMainFrame->GetActiveFrame())
				pNextActive = (CMDIChildWnd *) GetNextWindow();		// can be NULL

			//
			// Hide the control bar window and its frame.
			//
			ShowControlBar(pBar, FALSE, FALSE);
			m_bFirstActivationAfterClose = TRUE;

			{
				//
				// Make sure we properly redraw the "tab buttons" on
				// the MDI task bar to account for this newly hidden 
				// window.
				//
				QCWorkbook* p_workbook = (QCWorkbook *) GetWorkbook();
				p_workbook->ResetTaskBar();
			}

			if (pNextActive && (pNextActive->GetStyle() & WS_VISIBLE))
			{
				//
				// Since we're merely hiding the active window, we
				// need to inform the main frame that it needs to
				// activate a new child window.
				//
				ASSERT_KINDOF(CMDIChildWnd, pNextActive);
				pNextActive->CMDIChildWnd::ActivateFrame(bWasMaximized ? SW_SHOWMAXIMIZED : -1);
			}
//FORNOW			if (bWasMaximized)
//FORNOW			{
//FORNOW				pMainFrame->SetRedraw(TRUE);
//FORNOW				pMainFrame->UpdateWindow();
//FORNOW			}
			return;
		}
	}

	//
	// Intentionally bypass parent implementation in favor of grandparent
	// implementation.
	//
	SECWorksheet::OnClose();
}


/*****************************************************************************/
/*                          QCMiniDockFrameWnd                               */
/*****************************************************************************/


BEGIN_MESSAGE_MAP(QCMiniDockFrameWnd, SECMiniDockFrameWnd)
	//{{AFX_MSG_MAP(QCMiniDockFrameWnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


QCMiniDockFrameWnd::QCMiniDockFrameWnd()
{
}

QCMiniDockFrameWnd::~QCMiniDockFrameWnd()
{
}


////////////////////////////////////////////////////////////////////////
// RecalcLayout [protected, virtual]
//
// HACK ALERT.  Note that we're intentionally bypassing and completely 
// replacing the badly behaved SEC parent implementation here.
////////////////////////////////////////////////////////////////////////
void QCMiniDockFrameWnd::RecalcLayout(BOOL bNotify)
{
	if (!m_bInRecalcLayout)
	{
		CMiniFrameWnd::RecalcLayout(bNotify);

//BADSECSTUFF		// syncronize window text of frame window with dockbar itself
//BADSECSTUFF		TCHAR szTitle[_MAX_PATH];
//BADSECSTUFF		m_wndSECDockBar.GetWindowText(szTitle, _countof(szTitle));
//BADSECSTUFF		AfxSetWindowText(m_hWnd, szTitle);
		if ((m_wndSECDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)			//GOODQCSTUFF
		{																	//GOODQCSTUFF
			SECControlBar* pBar = m_wndSECDockBar.GetFirstControlBar();		//GOODQCSTUFF
			ASSERT(pBar != NULL);											//GOODQCSTUFF
			CWazooBar* pWazooBar = DYNAMIC_DOWNCAST(CWazooBar, pBar);		//GOODQCSTUFF
			if (pWazooBar)													//GOODQCSTUFF
				pWazooBar->RefreshWindowTitle();							//GOODQCSTUFF
			else															//GOODQCSTUFF
			{																//GOODQCSTUFF
				// syncronize frame window title with dockbar itself		//GOODQCSTUFF
				TCHAR szTitle[_MAX_PATH];									//GOODQCSTUFF
				m_wndSECDockBar.GetWindowText(szTitle, sizeof(szTitle));	//GOODQCSTUFF
				SetWindowText(szTitle);										//GOODQCSTUFF
			}																//GOODQCSTUFF
		}																	//GOODQCSTUFF
	}
}


////////////////////////////////////////////////////////////////////////
// RecalcLayout [protected, virtual]
//
// HACK ALERT.  Note that we're intentionally bypassing and completely 
// replacing the badly behaved SEC parent implementation here.
////////////////////////////////////////////////////////////////////////
void QCMiniDockFrameWnd::RecalcLayout(CPoint point, BOOL bNotify)
{
	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;
	// clear idle flags for recalc layout if called elsewhere
	if (m_nIdleFlags & idleNotify)
		bNotify = TRUE;
	m_nIdleFlags &= ~(idleLayout|idleNotify);

	// reposition all the child windows (regardless of ID)
	if (GetStyle() & FWS_SNAPTOBARS)
	{
		CRect rect(0, 0, 32767, 32767);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery,
			&rect, &rect, FALSE);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra,
			&m_rectBorder, &rect, TRUE);
		CalcWindowRect(&rect);
		SetWindowPos(NULL, point.x, point.y, rect.Width(), rect.Height(),
			SWP_NOACTIVATE|SWP_NOZORDER);
	}
	else
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra, &m_rectBorder);

//BADSECSTUFF	// syncronize window text of frame window with dockbar itself
//BADSECSTUFF	TCHAR szTitle[_MAX_PATH];
//BADSECSTUFF	m_wndSECDockBar.GetWindowText(szTitle, _countof(szTitle));
//BADSECSTUFF	AfxSetWindowText(m_hWnd, szTitle);

	if ((m_wndSECDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)			//GOODQCSTUFF
	{																	//GOODQCSTUFF
		SECControlBar* pBar = m_wndSECDockBar.GetFirstControlBar();		//GOODQCSTUFF
		ASSERT(pBar != NULL);											//GOODQCSTUFF
		CWazooBar* pWazooBar = DYNAMIC_DOWNCAST(CWazooBar, pBar);		//GOODQCSTUFF
		if (pWazooBar)													//GOODQCSTUFF
			pWazooBar->RefreshWindowTitle();							//GOODQCSTUFF
		else															//GOODQCSTUFF
		{																//GOODQCSTUFF
			// syncronize frame window title with dockbar itself		//GOODQCSTUFF
			TCHAR szTitle[_MAX_PATH];									//GOODQCSTUFF
			m_wndSECDockBar.GetWindowText(szTitle, sizeof(szTitle));	//GOODQCSTUFF
			SetWindowText(szTitle);										//GOODQCSTUFF
		}																//GOODQCSTUFF
	}																	//GOODQCSTUFF

	m_bInRecalcLayout = FALSE;
}


/*****************************************************************************/
/*                               QCWorkbook                                  */
/*****************************************************************************/

BEGIN_MESSAGE_MAP(QCWorkbook, SECWorkbook)
	//{{AFX_MSG_MAP(QCWorkbook)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(TCM_TABSEL, OnTabSelect)
	ON_WM_ACTIVATE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
    
/////////////////////////////////////////////////////////////////////////////
// QCWorkbook construction/initialization

QCWorkbook::QCWorkbook() :
	m_hWndTooltip(NULL),
	m_hBmpLogoOld(NULL),
	m_hBmpMaskOld(NULL)
{
	//
	// Override values set in base class constructor.
	//
    m_cxFold = 0;
	m_cxActive = 0;
	m_cyActive = 0;
	m_nMaxTabWidth = 130;
	ASSERT(m_pFloatingMDIChildClass == RUNTIME_CLASS(SECControlBarWorksheet));
	m_pFloatingMDIChildClass = RUNTIME_CLASS(QCControlBarWorksheet);

	//
	// Initialize "logo" and "mask" bitmaps.
	//
	::QCLoadBitmap(IDB_QCLOGO, m_LogoBitmap);
	BITMAP bm;
	m_LogoBitmap.GetObject(sizeof(bm), &bm);
	m_nLogoBitmapWidth = bm.bmWidth;
	m_nLogoBitmapHeight = bm.bmHeight;

	::QCLoadBitmap(IDB_QCLOGOMASK, m_MaskBitmap);
	m_MaskBitmap.GetObject(sizeof(bm), &bm);
	ASSERT(bm.bmWidth == m_nLogoBitmapWidth);
	ASSERT(bm.bmHeight == m_nLogoBitmapHeight);
}

QCWorkbook::~QCWorkbook()
{
	m_penHilight.DeleteObject();
	m_fontTabBold.DeleteObject();

	ASSERT(m_hBmpLogoOld);
	m_dcLogo.SelectObject(CBitmap::FromHandle(m_hBmpLogoOld));
	m_LogoBitmap.DeleteObject();		// cleanup loaded bitmap

	ASSERT(m_hBmpMaskOld);
	m_dcMask.SelectObject(CBitmap::FromHandle(m_hBmpMaskOld));
	m_MaskBitmap.DeleteObject();		// cleanup loaded bitmap
}


////////////////////////////////////////////////////////////////////////
// ShowMDITaskBar [public]
//
// Wrapper for base class SetWorkbookMode() method.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::ShowMDITaskBar(BOOL bShow)
{
	SetWorkbookMode(bShow);
	if (m_bWorkbookMode)
	{
		//
		// Tweak default margins set by SetWorkbookMode() function.
		//
		if (m_pWBClient)
			m_pWBClient->SetMargins(0, 0, 0, 30);
		else
		{
			ASSERT(0);
		}
		RecalcLayout();
		ResetTaskBar();
	}
}


////////////////////////////////////////////////////////////////////////
// QCGetTabRect [public]
//
// Public wrapper for protected GetTabPts() method in base class.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::QCGetTabRect(SECWorksheet* pSheet, CRect& rectTab)
{
	ASSERT_VALID(pSheet);
	ASSERT_KINDOF(SECWorksheet, pSheet);

	CPoint* tab_pts = NULL;
	int count = 0;

	GetTabPts(pSheet, tab_pts, count);
	ASSERT(count >= 5);
	rectTab.SetRect(tab_pts[0].x, tab_pts[0].y, tab_pts[4].x, tab_pts[4].y);

	delete [] tab_pts;
}


////////////////////////////////////////////////////////////////////////
// QCUpdateTab [public]
//
// Invalidates tab region.  Use when changing window title of a specific
// MDI child window.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::QCUpdateTab(SECWorksheet* pSheet)
{
	if (DYNAMIC_DOWNCAST(SECWorksheet, pSheet))
	{
		CRect rectTab;
		QCGetTabRect(pSheet, rectTab);
		rectTab.InflateRect(2, 2);
		InvalidateRect(&rectTab, TRUE);
	}
}


////////////////////////////////////////////////////////////////////////
// ResetTaskBar [public]
//
// Public wrapper for some commonly used protected methods.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::ResetTaskBar()
{
	if (m_bWorkbookMode)
		QCInvalidateAllTabs();
	RecalcToolTipRects();
}


////////////////////////////////////////////////////////////////////////
// CreateClient [public, virtual]
//
// Override base class implementation to initialize additional GDI 
// objects used by this class.
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu)
{
    BOOL rtn = SECWorkbook::CreateClient(lpCreateStruct, pWindowMenu);
	if (-1 == rtn)
		return -1;			// failure, so no point in continuing

	// Create the fonts and pens needed to draw the pushed-in (active) button
	m_fontTabBold.CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
			 ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			 DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_penHilight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNHILIGHT));

	ASSERT_VALID(&m_fontTabBold);
	ASSERT_VALID(&m_penHilight);

	//
	// Now create a temporary DC for this window, create a compatible
	// memory DC to store the bitmap bits, then select the bitmap into
	// the memory DC.  Get a (temporary) pointer to the old bitmap,
	// then save the old bitmap's handle.  We can't save the *pointer*
	// to the old bitmap because the pointer is temporary.
	//
	// Note that local CClientDC destructor for 'dc' will release the
	// device context.
	//
	{
		CClientDC dc(this);
		if (m_dcLogo.CreateCompatibleDC(&dc))
		{
			CBitmap* p_oldbitmap = m_dcLogo.SelectObject(&m_LogoBitmap);
			ASSERT(p_oldbitmap != NULL);
			m_hBmpLogoOld = HBITMAP(p_oldbitmap->GetSafeHandle());
			if (! m_hBmpLogoOld)
			{
				m_LogoBitmap.DeleteObject();
				return -1;			// indicate failure
			}
		}

		if (m_dcMask.CreateCompatibleDC(&dc))
		{
			CBitmap* p_oldbitmap = m_dcMask.SelectObject(&m_MaskBitmap);
			ASSERT(p_oldbitmap != NULL);
			m_hBmpMaskOld = HBITMAP(p_oldbitmap->GetSafeHandle());
			if (! m_hBmpMaskOld)
			{
				m_MaskBitmap.DeleteObject();
				return -1;			// indicate failure
			}
		}
	}

	return rtn;
}


////////////////////////////////////////////////////////////////////////
// InitMDITaskBarTooltips [public]
//
// One-time initialization to create built-in tooltip control for 
// the tab "buttons".
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::InitMDITaskBarTooltips()
{
	//
	// Create tooltip control.
	//
	ASSERT(NULL == m_hWndTooltip);
	m_hWndTooltip = ::CreateWindow(TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, 
									CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
									GetSafeHwnd(), (HMENU) NULL, ::AfxGetInstanceHandle(), NULL); 

	if (m_hWndTooltip) 
	{
		RecalcToolTipRects();
		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// ShutdownMDITaskBarTooltips [public]
//
// Cleanup the tooltip control.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::ShutdownMDITaskBarTooltips()
{
	//
	// Geez.  That's great.  By the time we get here, the
	// tooltip window has already been destroyed.  All
	// that's left to do is to set the HWND to NULL so that it
	// is no longer used internally.
	//
	ASSERT(! ::IsWindow(m_hWndTooltip));
	m_hWndTooltip = NULL;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
// Resize the MDI task bar tooltip rects whenever the main window size
// changes.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::OnSize(UINT nType, int cx, int cy)
{
	SECWorkbook::OnSize(nType, cx, cy);

	// Don't do anything if the controls aren't created yet, or
	// the window is being minimized
	if (m_hWndTooltip == NULL || nType == SIZE_MINIMIZED)
		return;

	RecalcToolTipRects();
}


////////////////////////////////////////////////////////////////////////
// QCGetTaskBarRect [protected]
//
// Return a rectangle that is roughly the worst case size of the
// entire tab button region.  This rect can be used to invalidate
// the region for painting and for drag and drop hit tests.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::QCGetTaskBarRect(CRect& rectTaskBar) const
{
	ASSERT(m_bWorkbookMode);

	if (m_pWBClient)
	{
		CRect rectMDIClient;
		CRect rectClient;

		GetClientRect(rectClient);
		m_pWBClient->GetClientRect(rectMDIClient);
		m_pWBClient->MapWindowPoints((CWnd *) this, rectMDIClient);

		rectTaskBar.left = rectClient.left;
		rectTaskBar.top = rectMDIClient.bottom + 2;
		rectTaskBar.right = rectClient.right;
		rectTaskBar.bottom = rectTaskBar.top + m_cyTab + 6;
	}
	else
		rectTaskBar.SetRectEmpty();
}


////////////////////////////////////////////////////////////////////////
// QCInvalidateAllTabs [protected]
//
// HACK ALERT.  The base class implementation of InvalidateAllTabs() 
// specifies an invalid region which is a couple of pixels too small.
// This is not an override of the base class implementation since the
// base class implementation is not virtual.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::QCInvalidateAllTabs()
{
	ASSERT(m_bWorkbookMode);

	CRect rectInvalid;
	QCGetTaskBarRect(rectInvalid);
	InvalidateRect(rectInvalid, TRUE);
}


////////////////////////////////////////////////////////////////////////
// GetTabPts [protected, virtual]
//
// Override base class implementation so that all tabs are the same
// size.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::GetTabPts(SECWorksheet* pSheet, CPoint*& pts, int& count)
{
	SECWorkbook::GetTabPts(pSheet, pts, count);
	for (int i = 0; i < count; i++)
	{
		pts[i].x += 2;
		pts[i].y -= 2;
	}

	ASSERT(count > 5);
	pts[3].x -= 1;
	pts[4].x -= 1;
	pts[5].x -= 1;
}


////////////////////////////////////////////////////////////////////////
// WindowProc [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
LRESULT QCWorkbook::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE: 
	case WM_LBUTTONDOWN: 
	case WM_LBUTTONUP: 
	case WM_RBUTTONDOWN: 
	case WM_RBUTTONUP: 
		if (m_bWorkbookMode && m_hWndTooltip)
		{ 
			//
			// Check position of cursor ... if it is over the Auto-Wazoo
			// bar, then relay mouse events to the tooltip control 
			// so that it can do its thing...
			//
			MSG msg; 

			msg.lParam = lParam; 
			msg.wParam = wParam; 
			msg.message = message; 
			msg.hwnd = GetSafeHwnd(); 
			::SendMessage(m_hWndTooltip, TTM_RELAYEVENT, 0, LPARAM(&msg)); 
		} 
		break; 
	default: 
		break; 
	} 

	return SECWorkbook::WindowProc(message, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////
// OnLButtonDown [protected, virtual]
//
// Yuck.  Completely override the base class version so that we can
// do a proper MDI activation when the user clicks a tab.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rectTab;
    
	ASSERT_VALID(this);

	if (m_bWorkbookMode) 
	{
		SECWorksheet* pSheet = TabHitTest(point, FALSE);
		if (pSheet)
		{
			//
			// Handle inactive child window case.
			//
			pSheet->SetSelected(TRUE);
				
			//
			// Unlike the SECWorkbook::OnLButtonDown() implementation,
			// we're not calling pSheet->ActivateFrame() here since
			// that conflicts with the logic in Eudora's 
			// CMDIChild::ActivateFrame() method.  Here, we're using
			// MDIActivate() instead, which seems like the better
			// way to go anyway.
			//
			MDIActivate(pSheet);

			//
			// Make sure we "restore" the newly-activated window...
			// Curiously, we only need to do this if the MDI child
			// is not already maximized.
			//
			BOOL bIsMaximized = FALSE;
			CMDIChildWnd* pActiveMDIChild = MDIGetActive(&bIsMaximized);
			ASSERT(pSheet == pActiveMDIChild);
			if (pActiveMDIChild && !bIsMaximized)
				MDIRestore(pActiveMDIChild);
		}
		else if (CalcLogoTopLeft(NULL, &point))
		{
			// Launch the logo URL if clicking on the logo
			LaunchURL(CRString(IDS_MAIN_WEB_URL));
		}
		else
		{
			//
			// Check for special case of restoring the active window
			// that is currently minimized.
			//
			pSheet = TabHitTest(point, TRUE);
			if (pSheet)
			{
				//
				// Make sure we "restore" the newly-activated window...
				// Curiously, we only need to do this if the MDI child
				// is not already maximized.
				//
				BOOL bIsMaximized = FALSE;
				CMDIChildWnd* pActiveMDIChild = MDIGetActive(&bIsMaximized);
				ASSERT(pSheet == pActiveMDIChild);
				if (pActiveMDIChild && !bIsMaximized)
					MDIRestore(pActiveMDIChild);
			}
		}
	}
    
	//
	// Note that we're intentionally bypassing our parent's
	// implementation here and going to our grandparent's
	// implementation.
	//
	SECMDIFrameWnd::OnLButtonDown(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Note that we use a raw ON_MESSAGE handler here rather than the
// ON_WM_CONTEXTMENU handler defined by CWnd.  The reason is that the
// message is actually routed to us from the derived class 
// implementation.  We return TRUE here to indicate to the derived
// class that we handled the message.
////////////////////////////////////////////////////////////////////////
long QCWorkbook::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	CPoint ptScreen(LOWORD(lParam), HIWORD(lParam));
 
	//
	// See if we landed on a tab button...  If so, then display the
	// "system menu" for the corresponding frame window.
	//
	if (m_bWorkbookMode) 
	{
		CPoint client_pt(ptScreen);
		ScreenToClient(&client_pt);

		SECWorksheet* pSheet = TabHitTest(client_pt);
		if (pSheet)
		{
			//
			// Get the menu containing the Auto-Wazoo context popups.
			//
			CMenu popup_menus;
			HMENU hMenu = QCLoadMenu(IDR_MDI_TASKBAR_POPUPS);
			if ( ! hMenu || ! popup_menus.Attach( hMenu ) )
				return FALSE;
			CMenu* p_temp_popup_menu = NULL;
			VERIFY((p_temp_popup_menu = popup_menus.GetSubMenu(0)) != NULL);
			if (p_temp_popup_menu != NULL)
			{
				CContextMenu::MatchCoordinatesToWindow(HWND(wParam), ptScreen);
				CContextMenu(p_temp_popup_menu, ptScreen.x, ptScreen.y, pSheet);
			}

			popup_menus.DestroyMenu();

			return TRUE;
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// AddSheet [protected, virtual]
//
// Override base class implementation to reset tab "button" positions
// for tooltips and to do a QC-specific invalidation of the MDI Task Bar,
// affectionately known as the Auto-Wazoo.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::AddSheet(SECWorksheet* pSheet)
{
	SECWorkbook::AddSheet(pSheet);

	ResetTaskBar();
}


////////////////////////////////////////////////////////////////////////
// RemoveSheet [protected, virtual]
//
// Override base class implementation to reset tab "button" positions
// for tooltips and to do a QC-specific invalidation of the MDI Task Bar,
// affectionately known as the Auto-Wazoo.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::RemoveSheet(SECWorksheet* pSheet)
{
	SECWorkbook::RemoveSheet(pSheet);

	ResetTaskBar();
}


////////////////////////////////////////////////////////////////////////
// OnDrawTab [protected, virtual]
//
// Override base class implementation to draw buttons that look like 
// Version 4 shell task bars.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::OnDrawTab(CDC* pDC, SECWorksheet* pSheet)
{
	CPen *pOldPen = NULL;
	CRgn rgn;
	CPoint* tab_pts = NULL;
	int count = 0;

	ASSERT_VALID(pDC);

	ASSERT_KINDOF(SECWorksheet, pSheet);
	GetTabPts(pSheet, tab_pts, count);
	rgn.CreateRectRgn(tab_pts[0].x + 1, tab_pts[0].y + 1, tab_pts[4].x - 1, tab_pts[4].y);

	// If this is the active (or selected) tab, draw it differently
	ASSERT_KINDOF(SECWorksheet, pSheet);

	if (pSheet->IsSelected() || pSheet == (SECWorksheet*)GetActiveFrame()) 
	{
		//
		// Draw active button in "down" state.
		//

		CBrush br;
		br.CreateSolidBrush(::GetSysColor(COLOR_BTNHILIGHT));
		pDC->FillRgn(&rgn, &br);
		br.DeleteObject();

		// Paint shadows
		pOldPen = pDC->SelectObject(&m_penFace);
		pDC->MoveTo(tab_pts[1].x + 2, tab_pts[1].y - 2);
		pDC->LineTo(tab_pts[4].x - 3, tab_pts[4].y - 2);
		pDC->LineTo(tab_pts[5].x - 3, tab_pts[5].y);

		pOldPen = pDC->SelectObject(&m_penShadow);
		pDC->MoveTo(tab_pts[1].x + 2, tab_pts[1].y - 3);
		pDC->LineTo(tab_pts[0].x + 2, tab_pts[0].y + 2);
		pDC->LineTo(tab_pts[5].x - 3, tab_pts[5].y + 2);

		// Paint tab outline
		pDC->SelectObject(&m_penBlack);
		pDC->MoveTo(tab_pts[1].x + 1, tab_pts[1].y - 1);
		pDC->LineTo(tab_pts[0].x + 1, tab_pts[0].y + 1);
		pDC->LineTo(tab_pts[5].x - 2, tab_pts[5].y + 1);

		pDC->SelectObject(&m_penHilight);
		pDC->MoveTo(tab_pts[2].x + 1, tab_pts[2].y - 1);
		pDC->LineTo(tab_pts[4].x - 2, tab_pts[4].y - 1);
		pDC->LineTo(tab_pts[5].x - 2, tab_pts[5].y);
	}
	else
	{
		//
		// Draw inactive button in "up" state.
		//

		CBrush br;
		br.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
		pDC->FillRgn(&rgn, &br);
		br.DeleteObject();

		// Paint shadows
		pOldPen = pDC->SelectObject(&m_penShadow);
		pDC->MoveTo(tab_pts[1].x + 2, tab_pts[1].y - 2);
		pDC->LineTo(tab_pts[4].x - 3, tab_pts[4].y - 2);
		pDC->LineTo(tab_pts[5].x - 3, tab_pts[5].y);

		// Paint tab outline
		pDC->SelectObject(&m_penHilight);
		pDC->MoveTo(tab_pts[1].x + 1, tab_pts[1].y - 1);
		pDC->LineTo(tab_pts[0].x + 1, tab_pts[0].y + 1);
		pDC->LineTo(tab_pts[5].x - 2, tab_pts[5].y + 1);

		pDC->SelectObject(&m_penBlack);
		pDC->MoveTo(tab_pts[2].x + 1, tab_pts[2].y - 1);
		pDC->LineTo(tab_pts[4].x - 2, tab_pts[4].y - 1);
		pDC->LineTo(tab_pts[5].x - 2, tab_pts[5].y);
	}

	// Cleanup
	pDC->SelectObject(pOldPen);    
	rgn.DeleteObject();
	delete [] tab_pts;
}


////////////////////////////////////////////////////////////////////////
// OnDrawTabIconAndLabel [protected, virtual]
//
// Override base class implementation to draw 16x16 icons and to draw
// active button with "bold" font using left-justified text rather
// than centered.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::OnDrawTabIconAndLabel(CDC* pDC, SECWorksheet* pSheet)
{
	ASSERT_VALID(pDC);

	CPoint ptIcon;
	CRect rectText;
	GetIconAndTextPositions(pSheet, ptIcon, rectText);

	// Draw icon
	HICON hIcon = GetTabIcon(pSheet);
	if (hIcon) 
		::DrawIconEx(pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, hIcon, 
						16, 16, 0, 0, DI_NORMAL);

	const TCHAR* pTabLabel = GetTabLabel(pSheet);
	if (pTabLabel != NULL) 
	{
		TCHAR* p_label = _tcsdup(pTabLabel);

		//
		// Decide whether or not to use the normal font or the bold font
		//
		ASSERT_KINDOF(SECWorksheet, pSheet);

		CFont* pOldFont = NULL;
		if (pSheet->IsSelected() || pSheet == (SECWorksheet*)GetActiveFrame())
			pOldFont = pDC->SelectObject(&m_fontTabBold);
		else
			pOldFont = pDC->SelectObject(&m_fontTab);

		CSize textSize = pDC->GetTextExtent(p_label, _tcsclen(p_label));
    
		// Draw the tab label using "..." if label is too lengthy
		int len = _tcsclen(p_label);
		while (len >= 4 && textSize.cx > rectText.Width()) 
		{
			p_label[len - 4] = '.';
			p_label[len - 3] = '.';
			p_label[len - 2] = '.';
			p_label[len - 1] = '\0';
			len = _tcsclen(p_label);
			textSize = pDC->GetTextExtent(p_label, len);
		}

		if (len > 0)
		{
			pDC->DrawText(p_label, -1, &rectText, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
		}
        
		// Cleanup
		pDC->SelectObject(pOldFont);
		free(p_label);
	}
}

////////////////////////////////////////////////////////////////////////
// CalcLogoTopLeft [protected]
//
// Utility function to figure out where the logo on the Auto-Wazoo
// should go.  Returns TRUE if the logo is being show, and fills in
// the TopLeft parameter.  Returns FALSE if the logo isn't being shown
// (Auto-Wazoo turned off or too many tabs to display the logo) and if
// pIntersectPoint is not in that region.  Need to optimize this last
// case because it will be called frquently (during mouse moves).
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::CalcLogoTopLeft(CPoint* TopLeft, CPoint* pIntersectPoint /*= NULL*/)
{
	CRect rectTaskBar;
	QCGetTaskBarRect(rectTaskBar);

	ASSERT((TopLeft != NULL) ^ (pIntersectPoint != NULL));

	//
	// The task bar rect stretches all the way to the edge of the
	// frame window, so we can't use the right hand edge of that rect.
	// Instead, use the right hand edge of the "workbook client"
	// window.
	//
	int nRightEdge = -1;
	{
		ASSERT(m_pWBClient);
		CRect rectWBClient;
		m_pWBClient->GetWindowRect(rectWBClient);
		ScreenToClient(rectWBClient);
		nRightEdge = rectWBClient.right;
	}

	int nLeftEdge = nRightEdge - m_nLogoBitmapWidth - 3;
	if (nLeftEdge > 0 && (!pIntersectPoint || pIntersectPoint->x >= nLeftEdge))
	{
		//
		// Walk backwards through the list of worksheets, searching
		// for the last visible one.  When found, determine its "tab
		// rect" on the MDI task bar.  If there is enough room left to
		// render the logo bitmap, then go for it.  Otherwise, don't
		// draw it.
		//
		CRect rectLastVisibleTab;
		rectLastVisibleTab.SetRect(0, 0, 0, 0);
		for (int i = m_worksheets.GetSize() - 1; i >= 0; i--)
		{
			SECWorksheet* pSheet = (SECWorksheet*) m_worksheets.GetAt(i);
			ASSERT_KINDOF(SECWorksheet, pSheet);
			if (pSheet->GetStyle() & WS_VISIBLE)
			{
				QCGetTabRect(pSheet, rectLastVisibleTab);
				break;
			}
		}

		if (nLeftEdge > rectLastVisibleTab.right + 2)
		{
			if (TopLeft)
			{
				TopLeft->x = nLeftEdge;
				TopLeft->y = rectTaskBar.top + 2;
			}
			return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// OnSetCursor [protected]
//
// Need to determine if the mouse is over the logo so we can change the
// cursor to a launch browser cursor to give the user the feedback that
// clicking here will launch a URL.
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT && pWnd == this)
	{
		CPoint point;

		if (GetCursorPos(&point))
		{
			ScreenToClient(&point);
			if (CalcLogoTopLeft(NULL, &point))
			{
				SetCursor(::QCLoadCursor(IDC_APP_LAUNCH));
				return TRUE;
			}
		}
	}

	return SECWorkbook::OnSetCursor(pWnd, nHitTest, message);
}

////////////////////////////////////////////////////////////////////////
// OnDrawBorder [protected, virtual]
//
// Override default base class implementation to draw the QC logo
// on the Auto-Wazoo bar.  Note that we completely override the SEC
// base class implementation here.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::OnDrawBorder(CDC* pDC)
{
	CPoint TopLeft;

	if (CalcLogoTopLeft(&TopLeft))
	{
		pDC->BitBlt(TopLeft.x, TopLeft.y, m_nLogoBitmapWidth, m_nLogoBitmapHeight, &m_dcMask, 0, 0, SRCAND);
		pDC->BitBlt(TopLeft.x, TopLeft.y, m_nLogoBitmapWidth, m_nLogoBitmapHeight, &m_dcLogo, 0, 0, SRCPAINT);
	}
}


////////////////////////////////////////////////////////////////////////
// IsTabLabelTruncated [protected, virtual]
//
// Returns TRUE if "button text" is too long to fit in the tab button.
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::IsTabLabelTruncated(SECWorksheet* pSheet)
{
	CPaintDC dc(this);		// device context for painting
    
	CPoint ptIcon;			// unused
	CRect rectText;
	GetIconAndTextPositions(pSheet, ptIcon, rectText);
    
	BOOL retval = FALSE;
	const TCHAR* pTabLabel = GetTabLabel(pSheet);
	if (pTabLabel != NULL) 
	{
		//
		// Decide whether or not to use the normal font or the bold font
		//
		ASSERT_KINDOF(SECWorksheet, pSheet);

		CFont* pOldFont = NULL;
		if (pSheet->IsSelected() || pSheet == (SECWorksheet *) GetActiveFrame())
			pOldFont = dc.SelectObject(&m_fontTabBold);
		else
			pOldFont = dc.SelectObject(&m_fontTab);

		CSize textSize = dc.GetTextExtent(pTabLabel, strlen(pTabLabel));
		if (textSize.cx > rectText.Width()) 
			retval = TRUE;

		// Cleanup
		dc.SelectObject(pOldFont);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////
// GetIconAndTextPositions [protected]
//
// Return positions of icon and text for the "tab button" in client
// coordinates.
////////////////////////////////////////////////////////////////////////
void QCWorkbook::GetIconAndTextPositions
(
	SECWorksheet* pSheet, 		//(i) worksheet for "tab button"
	CPoint& ptIcon, 			//(o) upper left coordinates of icon position
	CRect& rectText				//(o) rect for rendering text
)
{
	CPoint* pts = NULL;
	int count = 0;
	GetTabPts(pSheet, pts, count);
    
	// Determine icon position
	int icon_width = 0;
	if (GetTabIcon(pSheet))
	{
		icon_width = 16;
		ptIcon.x = pts[2].x + 5;
		ptIcon.y = pts[0].y + 4;
	}
	else
	{
		icon_width = 5;
		ptIcon.x = ptIcon.y = -1;			// icon not used
	}

	//
	// Determine text area, taking icon position into account.
	//
	rectText.SetRect(pts[2].x + 5 + icon_width + 3, pts[0].y + 1, pts[4].x - 3, pts[3].y + 1);

	delete [] pts;
}


////////////////////////////////////////////////////////////////////////
// OnNotify [protected, virtual]
//
// Custom handling for TTN_NEEDTEXT notification messages.
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*) lParam;

	if (m_bWorkbookMode && GetIniShort(IDS_INI_SHOW_TOOLTIPS) && (pNMHDR->hwndFrom == m_hWndTooltip))
	{
		switch (pNMHDR->code)
		{
		case TTN_NEEDTEXTA:
		case TTN_NEEDTEXTW:
			{
				//
				// The tool id is stuffed into the 'idFrom' field.
				// If uFlags doesn't have the IDISHWND bit set,
				// then this means we're getting a NEEDTEXT
				// notification from a "tool rect" rather than
				// a child window.
				//
				UINT nID = pNMHDR->idFrom;
				ASSERT(nID >= 0 && nID < UINT(GetSheetCount()));
				SECWorksheet* pSheet = GetWorksheet(nID);
				if (pSheet && IsTabLabelTruncated(pSheet))
				{
					CString tooltip(GetTabLabel(pSheet));
					tooltip = tooltip.Left(79);					// make sure it is less than 80 characters

					if (TTN_NEEDTEXTW == pNMHDR->code)
					{
						TOOLTIPTEXTW *pTTT = (TOOLTIPTEXTW *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));
						USES_CONVERSION;
						wcscpy(pTTT->szText, A2W(tooltip));
					}
					else
					{
						TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));
						strcpy(pTTT->szText, tooltip);
					}
					return TRUE;
				}
			}
			break;
		default:
			break;
		}
	}

	return SECWorkbook::OnNotify(wParam, lParam, pResult);
}


////////////////////////////////////////////////////////////////////////
// RecalcToolTipRects [protected]
//
// Recalculate tooltip regions to reflect current positions of all tab
// "buttons".
////////////////////////////////////////////////////////////////////////
void QCWorkbook::RecalcToolTipRects()
{
	if (m_hWndTooltip)
	{
		TOOLINFO ti;    // tool information 
		ti.cbSize = sizeof(TOOLINFO); 
		ti.uFlags = 0; 
		ti.hwnd = GetSafeHwnd(); 
		ti.hinst = 0;
		ti.uId = 0; 
		ti.lpszText = LPSTR_TEXTCALLBACK;

		//
		// Blow away all existing tools.
		//
		LRESULT nNumTools = ::SendMessage(m_hWndTooltip, TTM_GETTOOLCOUNT, 0, 0);
		for (int i = 0; i < nNumTools; i++)
		{
			ti.uId = i;
			::SendMessage(m_hWndTooltip, TTM_DELTOOL, 0, LPARAM(&ti));
		}

		if (m_bWorkbookMode && m_pWBClient)
		{
			//TRACE("QCWorkbook::RecalcToolTipRects\n");

			//
			// Recalculate tools based on the current "tab button" 
			// locations.
			//
			for (i = 0; i < GetSheetCount(); i++)
			{
				SECWorksheet* pSheet = GetWorksheet(i);
				ASSERT(pSheet);

				if (pSheet->GetStyle() & WS_VISIBLE)
				{
					CRect tabRect;
					QCGetTabRect(pSheet, tabRect);

					ti.uId = i;
					ti.rect.top = tabRect.top;
					ti.rect.left = tabRect.left;
					ti.rect.right = tabRect.right;
					ti.rect.bottom = tabRect.bottom;

//FORNOW#ifdef _DEBUG
//FORNOW				CString msg;
//FORNOW				msg.Format("toolrect = (%d,%d,%d,%d)", ti.rect.left, ti.rect.top, ti.rect.right, ti.rect.bottom);
//FORNOW				TRACE1("QCWorkbook::RecalcToolTipRects: %s\n", (const char *) msg);
//FORNOW#endif // _DEBUG

					ASSERT(::IsWindow(m_hWndTooltip));
					LRESULT lResult = ::SendMessage(m_hWndTooltip, TTM_ADDTOOL, 0, LPARAM(&ti));
					ASSERT(lResult);
				}
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////
// TabHitTest [protected]
//
// Recalculate tooltip regions to reflect current positions of all tab
// "buttons".
////////////////////////////////////////////////////////////////////////
SECWorksheet* QCWorkbook::TabHitTest(const CPoint& ptClient, BOOL bWantActive /*=TRUE*/)
{
	ASSERT(m_bWorkbookMode);

	//
	// Set up filter for "active" MDI child window.  Sometimes the
	// caller wants the active window and sometimes they don't.
	//
	SECWorksheet* pActive = NULL;
	if (! bWantActive)
	{
		pActive = (SECWorksheet*) GetActiveFrame();
		ASSERT(pActive);

		if (this == (QCWorkbook *) pActive)
			return NULL;			// no active MDI child window
		ASSERT_KINDOF(SECWorksheet, pActive);
	}

	//
	// Walk through the list of worksheets and do an individual hit
	// test on each one visible worksheet.  If we hit one, then return
	// it right away, unless of course it's the active worksheet and
	// the caller doesn't want active worksheets.
	//
	CRect rectTab;
	for (int i = 0; i < m_worksheets.GetSize(); i++)
	{
		SECWorksheet* pSheet = (SECWorksheet*) m_worksheets.GetAt(i);
		ASSERT_KINDOF(SECWorksheet, pSheet);
		if (pSheet->GetStyle() & WS_VISIBLE)
		{
			QCGetTabRect(pSheet, rectTab);
			if (pSheet != pActive && rectTab.PtInRect(ptClient)) 
			{
				return pSheet;
			}
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// IsOnMDIClientAreaBlankSpot [protected]
//
// Returns TRUE if we're over a blank area of the MDI client region.
// If the current MDI child is maximized, then by definition, there is
// no blank area.
////////////////////////////////////////////////////////////////////////
BOOL QCWorkbook::IsOnMDIClientAreaBlankSpot(const CPoint& ptClient)
{
	//
	// First, determine whether we're over the MDI client area.
	//
	ASSERT(m_pWBClient);
	{
		CRect rectMDIClient;

		m_pWBClient->GetWindowRect(rectMDIClient);
		ScreenToClient(rectMDIClient);
		if (! rectMDIClient.PtInRect(ptClient))
			return FALSE;
	}

	//
	// Second, check to see if the active MDI child, if any, is maximized.
	//
	{
		BOOL bIsMaximized = FALSE;
		CMDIChildWnd* pActiveMDIChild = MDIGetActive(&bIsMaximized);
		if (pActiveMDIChild && bIsMaximized)
			return FALSE;
	}

	//
	// Walk through the list of worksheets and do an individual hit
	// test on each visible worksheet frame window.  If we hit one,
	// then return FALSE right away.
	//
	CRect rectMDIFrame;
	for (int i = 0; i < m_worksheets.GetSize(); i++)
	{
		SECWorksheet* pSheet = (SECWorksheet*) m_worksheets.GetAt(i);
		ASSERT_KINDOF(SECWorksheet, pSheet);
		if (pSheet->GetStyle() & WS_VISIBLE)
		{
			pSheet->GetWindowRect(rectMDIFrame);
			ScreenToClient(rectMDIFrame);
			if (rectMDIFrame.PtInRect(ptClient)) 
				return FALSE;
		}
	}

	//
	// Congratulations ... if you make it this far, then we must be
	// over a blank spot.
	//
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnTabSelect [protected]
//
// Process notification from MDI child window that the selected tab
// changed.  Use this to set the tab button text on the Auto-Wazoo 
// task bar match the current MDI window title.
////////////////////////////////////////////////////////////////////////
LRESULT QCWorkbook::OnTabSelect(WPARAM, LPARAM)
{
	if (m_bWorkbookMode)
	{
		//
		// To be safe, invalidate all tab buttons for all MDI mode Wazoos
		//
		for (int i = 0; i < m_worksheets.GetSize(); i++)
		{
			SECWorksheet* pSheet = (SECWorksheet*) m_worksheets.GetAt(i);
			ASSERT_KINDOF(SECWorksheet, pSheet);
			if (DYNAMIC_DOWNCAST(QCControlBarWorksheet, pSheet))
			{
				if (pSheet->GetStyle() & WS_VISIBLE)
				{
					QCUpdateTab(pSheet);
				}
			}
		}
	}

    return 0;
}

////////////////////////////////////////////////////////////////////////
// OnActivate [protected]
//
// Save off last active window when being deactivated
////////////////////////////////////////////////////////////////////////
void QCWorkbook::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_INACTIVE)
	{
		CWnd* pWnd = GetFocus();
		CView* pActiveView = NULL;

		if (pWnd && pWnd->GetParentFrame() == this)
		{
			do
			{
				if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
				{
					pActiveView = (CView*)pWnd;
					break;
				}
			}
			while ((pWnd = pWnd->GetParent()) && pWnd != this);
		}

		SetActiveView(pActiveView);
	}

	SECWorkbook::OnActivate(nState, pWndOther, bMinimized);
}
