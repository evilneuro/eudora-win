// WazooBar.cpp : implementation file
//
// CWazooBar
// Specific implementation of a resizable SECControlBar.

#include "stdafx.h"

#include "ClipboardMgr.h"
#include "QC3DTabWnd.h"
#include "WazooBar.h"
#include "WazooWnd.h"
#include "WazooBarMgr.h"

#include "resource.h"
#include "rs.h"
#include "utils.h"

#include "workbook.h" // QCControlBarWorksheet

#include "MboxWazooWnd.h"
#include "SignatureWazooWnd.h"
#include "StationeryWazooWnd.h"
#include "FilterReportWazooWnd.h"
#include "FileBrowseWazooWnd.h"
#include "FiltersWazooWnd.h"
#include "NicknamesWazooWnd.h"
#include "PersonalityWazooWnd.h"
#include "DirectoryServicesWazooWndNewIface.h"
#include "TaskStatusWazooWnd.h"
#include "TaskErrorWazooWnd.h"
#include "AdWazooWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CWazooBar, SECControlBar)

BEGIN_MESSAGE_MAP(CWazooBar, SECControlBar)
	//{{AFX_MSG_MAP(CWazooBar)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_WINDOWPOSCHANGING()
	ON_MESSAGE(TCM_TABSEL, OnTabSelect)
	ON_COMMAND(ID_SEC_MDIFLOAT, OnCmdSECMDIFloat)
	ON_COMMAND(ID_WAZOO_TAB_ON_TOP, OnTabsOnTop)
	ON_COMMAND(ID_WAZOO_TAB_ON_BOTTOM, OnTabsOnBottom)
	ON_COMMAND(ID_WAZOO_TAB_ON_LEFT, OnTabsOnLeft)
	ON_COMMAND(ID_WAZOO_TAB_ON_RIGHT, OnTabsOnRight)
	ON_COMMAND(ID_WAZOO_TAB_SHOW, OnTabsShowSingleTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CWazooBar

CWazooBar::CWazooBar(CWazooBarMgr* pManager) :
	m_pManager(pManager),
	m_idAutoActivateTimer(0)
{

	ASSERT(pManager);
}

CWazooBar::~CWazooBar()
{
}


////////////////////////////////////////////////////////////////////////
// AddWazooWindow [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::AddWazooWindow(CWazooWnd* pWazooWnd, const CPoint& ptScreen)
{
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));
	ASSERT(pWazooWnd);
	ASSERT_KINDOF(CWazooWnd, pWazooWnd);
	ASSERT(pWazooWnd->GetStyle() & WS_CHILD);
	ASSERT(pWazooWnd->GetParent() == ::AfxGetMainWnd());
	
	int nIndex = -1;
	if (ptScreen != CPoint(-1, -1))
	{
		//
		// Do hit test to see if we're dropping a wazoo over
		// an existing tab.
		//
		BOOL bSelected = FALSE;		// unused
		if (QCTabHitTest(ptScreen, bSelected, nIndex))
		{
			ASSERT(nIndex >= 0 && nIndex < m_wndTab.GetTabCount());
		}
	}

	//
	// Based on the results of the hit test (stored in the nIndex value), 
	// decide whether to insert or append the new wazoo.
	//
	SECTab* pNewTab = NULL;
	if (nIndex != -1)
	{
		//
		// Dropping on an existing tab, so insert the new tab "before"
		// the existing tab.
		//
		pNewTab = m_wndTab.InsertTab(pWazooWnd, nIndex, pWazooWnd->GetTabLabel());
	}
	else
	{
		//
		// Not dropping on an existing tab, so append the new tab as
		// the "last" tab.
		//
		pNewTab = m_wndTab.AddTab(pWazooWnd, pWazooWnd->GetTabLabel());
	}

	if (pNewTab)
	{
		//
		// Lookup the index for the newly-added window in order to set
		// the icon.
		//
		int nTabIndex = -1;
	    if (m_wndTab.FindTab(pWazooWnd, nTabIndex))
	    {
			m_wndTab.SetTabIcon(nTabIndex, pWazooWnd->GetTabIconHandle());
			pWazooWnd->ModifyStyle(0, WS_CHILD);
			pWazooWnd->SetParent(&m_wndTab);
			return TRUE;
	    }
	}

	ASSERT(0);			// something is very wrong...
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// RemoveWazooWindow [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::RemoveWazooWindow(CWazooWnd* pWazooWnd)
{
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));
	ASSERT(pWazooWnd);
	ASSERT_KINDOF(CWazooWnd, pWazooWnd);
	ASSERT(pWazooWnd->GetStyle() & WS_CHILD);

	if (m_wndTab.TabExists(pWazooWnd))
	{
		m_wndTab.RemoveTab(pWazooWnd);
		ASSERT_VALID(pWazooWnd);
		ASSERT(::IsWindow(pWazooWnd->GetSafeHwnd()));
		pWazooWnd->SetParent(::AfxGetMainWnd());
		ASSERT(pWazooWnd->GetOwner() == ::AfxGetMainWnd());
		return TRUE;
	}

	ASSERT(0);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// ShuffleWazooWindow [public]
//
// Move the given 'pWazooWnd' which is already in this container to a
// new location in the tab order, based on the location of 'ptScreen'.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::ShuffleWazooWindow(CWazooWnd* pWazooWnd, const CPoint& ptScreen)
{
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));
	ASSERT(pWazooWnd);
	ASSERT_KINDOF(CWazooWnd, pWazooWnd);
	ASSERT(pWazooWnd->GetStyle() & WS_CHILD);
	ASSERT(pWazooWnd->GetParent() == &m_wndTab);
	ASSERT(ptScreen != CPoint(-1, -1));
	
	int nInsertionIndex = -1;		// tab that we're dropping on
	{
		//
		// Do hit test to see if we're dropping a wazoo over
		// an existing tab.
		//
		BOOL bSelected = FALSE;		// unused
		if (QCTabHitTest(ptScreen, bSelected, nInsertionIndex))
		{
			ASSERT(nInsertionIndex >= 0 && nInsertionIndex < m_wndTab.GetTabCount());
			ASSERT(FALSE == bSelected);
		}
	}

	if (nInsertionIndex != -1)
	{
		//
		// Dropping on an existing tab, so we need to figure out which
		// tab is being moved.
		//
		int nSelectionIndex = -1;		// tab that we're about to shuffle
	    if (! m_wndTab.FindTab(pWazooWnd, nSelectionIndex))
	    {
			ASSERT(0);
			return FALSE;
	    }
		ASSERT(nSelectionIndex >= 0 && nSelectionIndex < m_wndTab.GetTabCount());
		ASSERT(nSelectionIndex != nInsertionIndex);

		//
		// Based on the relative tab positions of the source and target
		// tabs, decide where to surgically re-insert the wazoo.
		//
		SECTab* pNewTab = NULL;
		if (nInsertionIndex < nSelectionIndex)
		{
			//
			// Easy case.  New position is "before" the old position.
			//
			RemoveWazooWindow(pWazooWnd);
			pNewTab = m_wndTab.InsertTab(pWazooWnd, nInsertionIndex, pWazooWnd->GetTabLabel());
		}
		else
		{
			//
			// Hard case.  New position is "after" the old position.  In this
			// case, we need to account for the removal of the old tab
			// in the index computation.
			//
			if (nInsertionIndex < (m_wndTab.GetTabCount() - 1))
			{
				RemoveWazooWindow(pWazooWnd);
				pNewTab = m_wndTab.InsertTab(pWazooWnd, nInsertionIndex, pWazooWnd->GetTabLabel());
			}
			else
			{
				RemoveWazooWindow(pWazooWnd);
				pNewTab = m_wndTab.AddTab(pWazooWnd, pWazooWnd->GetTabLabel());
			}
		}

		if (pNewTab)
		{
			//
			// Lookup the index for the newly-added window in order to set
			// the icon.
			//
			int nTabIndex = -1;
		    if (m_wndTab.FindTab(pWazooWnd, nTabIndex))
		    {
				m_wndTab.SetTabIcon(nTabIndex, pWazooWnd->GetTabIconHandle());
				pWazooWnd->ModifyStyle(0, WS_CHILD);
				pWazooWnd->SetParent(&m_wndTab);
				return TRUE;
		    }
		}
	}
	else
	{
		//
		// Not dropping over a tab...  This means remove it then add it
		// back as the "last" tab.
		//
		RemoveWazooWindow(pWazooWnd);
		AddWazooWindow(pWazooWnd, CPoint(-1, -1));
		return TRUE;
	}

	ASSERT(0);			// something is very wrong...
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// ActivateWazooWindow [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::ActivateWazooWindow(CRuntimeClass* pRuntimeClass)
{
	for (int i = 0; i < m_wndTab.GetTabCount(); i++)
	{
		LPCTSTR lpszLabel = NULL;	// unused
		BOOL bSelected = FALSE;		// unused
		CWnd* pWnd = NULL;
		void* pExtra = NULL;		// unused

		if (m_wndTab.GetTabInfo(i, lpszLabel, bSelected, pWnd, pExtra))
		{
			if (pWnd->IsKindOf(pRuntimeClass))
			{
				int nOldActiveTab = -1;
				m_wndTab.GetActiveTab(nOldActiveTab);
				ASSERT(nOldActiveTab != -1);

				//
				// Found the window to activate, so get on with it
				//
				m_wndTab.ActivateTab(i);

				//
				// In the SEC stuff, the ActivateTab() call doesn't do
				// anything if the tab is ALREADY active, so make sure the 
				// Wazoo gets an initialization notice anyway.  This
				// covers cases like a wazoo container with only
				// one wazoo and the proper handling of Just In Time
				// display initialization for "active" wazoos which
				// have never been displayed before.
				//
				if (nOldActiveTab == i)
					((CWazooWnd *) pWnd)->OnActivateWazoo();

				//
				// Force the keyboard focus to the desired Wazoo window,
				// just in case.
				//
				ASSERT_KINDOF(CWazooWnd, pWnd);
				pWnd->SetFocus();

				return TRUE;
			}
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetActiveWazooWindow [public]
//
////////////////////////////////////////////////////////////////////////
CWazooWnd* CWazooBar::GetActiveWazooWindow()
{
	CWnd* pActiveTabWnd = NULL;
	m_wndTab.GetActiveTab(pActiveTabWnd);

	if (pActiveTabWnd)
	{
		ASSERT_KINDOF(CWazooWnd, pActiveTabWnd);
	}

	return (CWazooWnd *) pActiveTabWnd;
}


////////////////////////////////////////////////////////////////////////
// SetActiveWazooWindow [public]
//
// Decide which tab is the initial "active" tab in this wazoo container.
////////////////////////////////////////////////////////////////////////
void CWazooBar::SetActiveWazooWindow()
{
	//
	// FORNOW, always activate the first tab... In the future, we will
	// want to restore the last previously active tab.
	//
	ASSERT(m_wndTab.GetSafeHwnd());
	if (m_wndTab.GetTabCount() > 0)
	{
		m_wndTab.ActivateTab(0);
		m_wndTab.ScrollToTab(0);
	}
}


////////////////////////////////////////////////////////////////////////
// ContainsWazooWindow [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::ContainsWazooWindow(CRuntimeClass* pRuntimeClass)
{
	for (int i = 0; i < m_wndTab.GetTabCount(); i++)
	{
		LPCTSTR lpszLabel = NULL;	// unused
		BOOL bSelected = FALSE;		// unused
		CWnd* pWnd = NULL;
		void* pExtra = NULL;		// unused

		if (m_wndTab.GetTabInfo(i, lpszLabel, bSelected, pWnd, pExtra))
		{
			if (pWnd->IsKindOf(pRuntimeClass))
				return TRUE;
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// UnlinkFromManager [public]
//
// Returns TRUE if we successfully unlinked from our manager.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::UnlinkFromManager()
{
	ASSERT(m_pManager);

	return m_pManager->UnlinkWazooBar(this);
}


////////////////////////////////////////////////////////////////////////
// GetWazooCount [public]
//
// Returns the number of Wazoos in this container.
////////////////////////////////////////////////////////////////////////
int CWazooBar::GetWazooCount()
{
	return m_wndTab.GetTabCount();
}


////////////////////////////////////////////////////////////////////////
// QCTabHitTest [public]
//
// Does a hit test on the tabs and returns the corresponding window,
// if any.
////////////////////////////////////////////////////////////////////////
CWnd* CWazooBar::QCTabHitTest(const CPoint& ptScreen, BOOL& bSelected, int& nIndex)
{
	nIndex = -1;

	//
	// Loop through the tabs and see if we hit one...
	//
	for (int i = 0; i < m_wndTab.GetTabCount(); i++)
	{
		if (m_wndTab.QCTabHit(i, ptScreen))
		{
			//
			// Hit one, so return the corresponding Wazoo window...
			// 
			LPCTSTR lpszLabel = NULL;	// unused
			CWnd* pWnd = NULL;
			void* pExtra = NULL;		// unused

			bSelected = FALSE;
			if (m_wndTab.GetTabInfo(i, lpszLabel, bSelected, pWnd, pExtra))
			{
				ASSERT(pWnd);
				ASSERT_KINDOF(CWazooWnd, pWnd);
				nIndex = i;				// zero-based index of tab that was hit
				return pWnd;
			}
			else
			{
				ASSERT(0);
			}
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// SetDefaultWazooConfig [public]
//
// Setup some hardcoded default Wazoo configurations.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::SetDefaultWazooConfig(int nBar)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));

	switch (nBar)
	{
	case 0:
		CreateWazooWindow(RUNTIME_CLASS(CMboxWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CFileBrowseWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CSignatureWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CStationeryWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CPersonalityWazooWnd));
		break;
	case 1:
		CreateWazooWindow(RUNTIME_CLASS(DirectoryServicesWazooWndNew));
		CreateWazooWindow(RUNTIME_CLASS(CNicknamesWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CFiltersWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CFilterReportWazooWnd));
		break;
	case 2:
		CreateWazooWindow(RUNTIME_CLASS(CTaskStatusWazooWnd));
		CreateWazooWindow(RUNTIME_CLASS(CTaskErrorWazooWnd));
		break;
	default:
		ASSERT(0);
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// LoadWazooConfigFromIni [public]
//
// Restores Wazoo config for this container from the INI file.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::LoadWazooConfigFromIni()
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));

	//
	// The first thing to do is to create a key name by appending the
	// control bar ID to a "WazooBar" string as in "WazooBar201".
	//
	CString strName;
	strName.Format(CRString(IDS_INI_WAZOOBAR_FORMAT), GetDlgCtrlID());

	//
	// Fetch the INI entry...
	//
	CString strValue;
	::GetPrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), 
								strName, 
								"",
								strValue.GetBuffer(255),
								256, 
								INIPath);
	strValue.ReleaseBuffer();
	if (strValue.IsEmpty())
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Grab the first parameter, which should be a cryptic decimal value that
	// indicates the tab location/visibility.  We save the value here
	// so that we can use it *after* the tab windows have been created.
	//
	int nTabLocation = 0;
	CString strParam;
	if (GetNextParam(strValue, strParam))
		nTabLocation = ::atoi(strParam);

	//
	// At this point, 'strValue' should contain a comma-separated
	// list of Runtime Class names.
	//
	while (GetNextParam(strValue, strParam))
	{
		if (isdigit(strParam[0]))
		{
			int nIndex = atoi(strParam);
			if (nIndex > 0)
				m_wndTab.PostMessage(TCM_TABSEL, nIndex);
		}
		//
		// Okay, do the ugly string compares to figure out what
		// object we're dealing with...  If there's a match, create
		// the Wazoo window.
		//
		else if (strcmp(strParam, RUNTIME_CLASS(CMboxWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CMboxWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CSignatureWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CSignatureWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CFileBrowseWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CFileBrowseWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CFilterReportWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CFilterReportWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CFiltersWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CFiltersWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CNicknamesWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CNicknamesWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CPersonalityWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CPersonalityWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CStationeryWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CStationeryWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(DirectoryServicesWazooWndNew)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(DirectoryServicesWazooWndNew));
		else if (strcmp(strParam, RUNTIME_CLASS(CTaskStatusWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CTaskStatusWazooWnd));
		else if (strcmp(strParam, RUNTIME_CLASS(CTaskErrorWazooWnd)->m_lpszClassName) == 0)
			CreateWazooWindow(RUNTIME_CLASS(CTaskErrorWazooWnd));
		else
		{
			ASSERT(0);			// unrecognized Runtime Class name
		}
	}

	if (IsFloating() && !IsMDIChild())
	{
		//
		// Special case for creation of child Wazoos inside of a
		// floating window...  need to notify the child wazoos to
		// perform their OnInitialUpdate() sequence.
		//
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
	}

	//
	// Now go back and reset the tab location/visibility.
	//
	m_wndTab.SetTabLocation(nTabLocation);

	//
	// If the parent window is an MDI child, then let's also restore the 
	// frame window state and position in the following format:
	//
	//     WazooMDIxxx=left,top,right,bottom,nCmdShow,nFlags
	// 
	// Where "xxx" is the control bar ID and "left/top/right/bottom"
	// are the main window client coordinates for a RESTORED window
	// (not maximized or minimized).  The 'nCmdShow' value indicates 
	// the previous state for this MDI child, while 'nFlags' is a bit
	// strip indicating whether or not this was the active MDI child and
	// whether or not this child window is visible or hidden.
	//
	if (IsMDIChild())
	{
		strName.Format(CRString(IDS_INI_WAZOOMDI_FORMAT), GetDlgCtrlID());

		::GetPrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), 
									strName, 
									"",
									strValue.GetBuffer(255),
									256, 
									INIPath);
		strValue.ReleaseBuffer();
		if (strValue.IsEmpty())
		{
			ASSERT(0);
			return FALSE;
		}

		CRect rectFrame;
		int nCmdShow = -1;
		int nFlags = 0;

		if (sscanf(strValue, "%d,%d,%d,%d,%d,%d", &rectFrame.left,
													&rectFrame.top,
													&rectFrame.right,
													&rectFrame.bottom,
													&nCmdShow,
													&nFlags) != 6)
		{
			ASSERT(0);
			return FALSE;
		}

		CMDIChildWnd* pMDIFrame = (CMDIChildWnd *) GetParentFrame();
		ASSERT_KINDOF(CMDIChildWnd, pMDIFrame);

		pMDIFrame->MoveWindow(rectFrame, FALSE);

		//
		// Ugly, ugly, ugly.  In the Eudora start up sequence, the
		// CMainFrame::LoadOpenWindows() routine runs after we've gone
		// through here, so we need to POST messages so that the main
		// frame can properly activate MDI-mode Wazoo windows.
		//
		if (nFlags & FLAGS_VISIBLE)
		{
			switch (nCmdShow)
			{
			case SW_SHOWMAXIMIZED:
				pMDIFrame->PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				if (nFlags & FLAGS_ACTIVE)
				{
					pMDIFrame->PostMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, 0), 0);
					pMDIFrame->PostMessage(WM_NCACTIVATE, 1, 0);
				}
				break;
			case SW_SHOWNORMAL:
				pMDIFrame->PostMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
				if (nFlags & FLAGS_ACTIVE)
				{
					pMDIFrame->PostMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, 0), 0);
					pMDIFrame->PostMessage(WM_NCACTIVATE, 1, 0);
				}
				break;
			case SW_SHOWMINIMIZED:
				pMDIFrame->PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);

				//
				// Don't bother activating minimized windows.
				//
				break;
			default:
				ASSERT(0);
				break;
			}
		}
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SaveWazooConfigToIni [public]
//
// Saves Wazoo config for this container to the INI file.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::SaveWazooConfigToIni()
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));

	//
	// Assumes that the [WazooBars] section has already been cleared out
	// by the time we get here...  The first thing to do is to create
	// a key name by appending the control bar ID to a "WazooBar"
	// string as in "WazooBar201".
	//
	CString strName;
	strName.Format(CRString(IDS_INI_WAZOOBAR_FORMAT), GetDlgCtrlID());

	//
	// Then, encode the current tab location as a cryptic decimal
	// value.
	//
	CString strValue;
	strValue.Format("%d", int(m_wndTab.GetTabLocation()));

	//
	// Next, create a comma-separated list of Runtime Class names,
	// such as "CMboxWazooWnd,CStationeryWazooWnd,CLookupWazooWnd".
	// Note that the items are written out in left-to-right tab order.
	//
	ASSERT(m_wndTab.GetTabCount() > 0);
	for (int i = 0; i < m_wndTab.GetTabCount(); i++)
	{
		LPCTSTR lpszLabel = NULL;	// unused
		BOOL bSelected = FALSE;		// unused
		CWnd* pWnd = NULL;
		void* pExtra = NULL;		// unused

		if (m_wndTab.GetTabInfo(i, lpszLabel, bSelected, pWnd, pExtra))
		{
			CWazooWnd* pWazooWnd = DYNAMIC_DOWNCAST(CWazooWnd, pWnd);
			if (pWazooWnd)
			{
				//
				// Append the Runtime Class name to the comma-separated string.
				//
				ASSERT_VALID(pWazooWnd);
				ASSERT(::IsWindow(pWazooWnd->GetSafeHwnd()));
				if (! strValue.IsEmpty())
					strValue += ",";
				strValue += pWazooWnd->GetRuntimeClass()->m_lpszClassName;
			}
			else
			{
				ASSERT(0);
				return FALSE;
			}
		}
	}

	int nActiveTab;
	m_wndTab.GetActiveTab(nActiveTab);
	if (nActiveTab > 0)
	{
		char NumStr[16];
		itoa(nActiveTab, NumStr, 10);
		strValue += ',';
		strValue += NumStr;
	}

	//
	// If we get this far, 'strName' contains the key and 'strValue'
	// contains the value, so write it out to the INI file.
	//
	::WritePrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), 
								strName, 
								strValue, 
								INIPath);

	//
	// If the parent window is an MDI child, then let's also restore the 
	// frame window state and position in the following format:
	//
	//     WazooMDIxxx=left,top,right,bottom,nCmdShow,nFlags
	// 
	// Where "xxx" is the control bar ID and "left/top/right/bottom"
	// are the main window client coordinates for a RESTORED window
	// (not maximized or minimized).  The 'nCmdShow' value indicates 
	// the previous state for this MDI child, while 'nFlags' is a bit
	// strip indicating whether or not this was the active MDI child and
	// whether or not this child window is visible or hidden.
	//
	if (IsMDIChild())
	{
		CMDIChildWnd* pParentFrame = (CMDIChildWnd *) GetParentFrame();
		ASSERT_KINDOF(CMDIChildWnd, pParentFrame);

		strName.Format(CRString(IDS_INI_WAZOOMDI_FORMAT), GetDlgCtrlID());

		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (! pParentFrame->GetWindowPlacement(&wp))
		{
			ASSERT(0);
			return FALSE;
		}

		//
		// Determine the nFlags value to save.  The 0x1 bit indicates
		// whether the window is visible (0 for invisible, 1 for
		// visible).  The 0x2 bit indicates whether the window is
		// the active MDI child (0x0 for inactive, 0x2 for active).
		//

		int nFlags = 0;
//		if (pParentFrame->GetStyle() & WS_VISIBLE)
		if (IsWindowVisible())
		{
			nFlags |= FLAGS_VISIBLE;

			CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
			ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

			BOOL bIsMaximized = FALSE;
			CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);

			if (pActiveMDIChild == pParentFrame)
				nFlags |= FLAGS_ACTIVE;
		}

		strValue.Format("%d,%d,%d,%d,%d,%d", wp.rcNormalPosition.left,		// FORNOW, hardcoded
											wp.rcNormalPosition.top,
											wp.rcNormalPosition.right,
											wp.rcNormalPosition.bottom,
											wp.showCmd,
											nFlags);

		//
		// If we get this far, 'strName' contains the key and 'strValue'
		// contains the value, so write it out to the INI file.
		//
		::WritePrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), 
									strName, 
									strValue, 
									INIPath);
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// RefreshWindowTitle [public]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::RefreshWindowTitle()
{
	//
	// Now make sure the window title is updated.
	//
	int nTabIndex = -1;
	if (m_wndTab.GetActiveTab(nTabIndex))
		OnTabSelect(nTabIndex, 0);
}


////////////////////////////////////////////////////////////////////////
// PreTranslateMessage [public, virtual]
//
// FORNOW, experimental hook for dialog messages.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::PreTranslateMessage(MSG* pMsg)
{
#ifdef _DEBUG
	BOOL bGotKey = FALSE;

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		if (VK_TAB == pMsg->wParam || VK_F4 == pMsg->wParam)
		{
			TRACE2("CWazooBar::PreTranslateMessage: got keycode %d (0x%X)\n", pMsg->wParam, pMsg->wParam);
			bGotKey = TRUE;
		}
		break;
	default:
		break;
	}
#endif // _DEBUG

	BOOL bProcessed = SECControlBar::PreTranslateMessage(pMsg);

	if (bProcessed)
	{
#ifdef _DEBUG
		if (bGotKey)
		{
			TRACE2("CWazooBar::PreTranslateMessage: ate keycode %d (0x%X)\n", pMsg->wParam, pMsg->wParam);
		}
#endif // _DEBUG
			
		return TRUE;
	}

	// no default processing
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// CreateWazooWindow [protected]
//
// Creates and adds a new Wazoo window to this Wazoo container.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::CreateWazooWindow(CRuntimeClass* pRuntimeClass)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));
	ASSERT(pRuntimeClass);

	CWazooWnd* pWazooWnd = (CWazooWnd *) pRuntimeClass->CreateObject();
	if (NULL == pWazooWnd)
		return FALSE;

	ASSERT_KINDOF(CWazooWnd, pWazooWnd);

	//
	// Note that unowned wazoo windows are child windows of the
	// main frame window.
	//
	pWazooWnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), ::AfxGetMainWnd(), UINT(-1));
	if (! AddWazooWindow(pWazooWnd, CPoint(-1, -1)))
	{
		delete pWazooWnd;
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetNextParam [private]
//
// Private helper function for processing parameter strings with one
// or more comma-separated values.  The idea is to remove the next
// parameter from the parameter list, stuff it into a string,
// returning TRUE if everything went okay.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::GetNextParam(CString& strParams, CString& strParam) const
{
	strParam.Empty();

	if (strParams.IsEmpty())
		return FALSE;			// can't get juice from a turnip

	int idx = -1;
	if ((idx = strParams.Find(',')) != -1)
	{
		//
		// Strip next parameter from the front of the paramList.
		//
		strParam = strParams.Left(idx);
		strParams = strParams.Right(strParams.GetLength() - idx - 1);
	}
	else
	{
		strParam = strParams;
		strParams.Empty();
	}

	if (strParam.IsEmpty())
	{
		ASSERT(0);				// huh?  empty parameter?
		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CWazooBar message handlers


////////////////////////////////////////////////////////////////////////
// OnUpdateCmdUI [protected, pure virtual]
//
// Dummy implementation required for this pure virtual.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnUpdateCmdUI(CFrameWnd*, BOOL)
{
	// this method intentionally left blank...  ;-)
}


////////////////////////////////////////////////////////////////////////
// OnExtendContextMenu [protected, virtual]
//
// Override base class implementation to provide tab choices.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnExtendContextMenu(CMenu* pMenu)
{
	//
	// Acquire a temporary Wazoo Bar context menu.  This is cleaned
	// up automatically by MFC at the next idle time.
	//
	HMENU hWazooBarMenu = ::QCLoadMenu(IDR_WAZOOBAR_POPUPS);
	CMenu* pWazooBarMenu = CMenu::FromHandle(hWazooBarMenu);
	if (NULL == pWazooBarMenu)
	{
		ASSERT(0);
		return;
	}

	//
	// The CMenu::GetSubMenu() returns a temporary CMenu object that
	// MFC cleans up automatically.
	//
	CMenu* pTabPopupMenu = pWazooBarMenu->GetSubMenu(0);
	if (pTabPopupMenu != NULL)
	{
		//
		// Use a menu "radio button" to show the current tab location.
		//
		UINT uCurrentLocation = UINT(-1);
		switch (m_wndTab.GetTabLocation() & TWS_TAB_ORIENTATIONS)
		{
		case TWS_TABS_ON_TOP:
			uCurrentLocation = ID_WAZOO_TAB_ON_TOP;
			break;
		case TWS_TABS_ON_BOTTOM:
			uCurrentLocation = ID_WAZOO_TAB_ON_BOTTOM;
			break;
		case TWS_TABS_ON_LEFT:
			uCurrentLocation = ID_WAZOO_TAB_ON_LEFT;
			break;
		case TWS_TABS_ON_RIGHT:
			uCurrentLocation = ID_WAZOO_TAB_ON_RIGHT;
			break;
		default:
			ASSERT(0);
			break;
		}

		pTabPopupMenu->CheckMenuRadioItem(ID_WAZOO_TAB_ON_TOP, ID_WAZOO_TAB_ON_RIGHT, uCurrentLocation, MF_BYCOMMAND);

		//
		// Disable the Top/Left/Bottom/Right items only if the tabs are
		// currently hidden.  Also, set the check mark next to the
		// "Hide" menu item.
		//
		if (m_wndTab.TabsAreVisible())
			pTabPopupMenu->CheckMenuItem(ID_WAZOO_TAB_SHOW, MF_BYCOMMAND | MF_CHECKED);
		else
		{
			pTabPopupMenu->EnableMenuItem(ID_WAZOO_TAB_ON_TOP, MF_BYCOMMAND | MF_GRAYED);
			pTabPopupMenu->EnableMenuItem(ID_WAZOO_TAB_ON_BOTTOM, MF_BYCOMMAND | MF_GRAYED);
			pTabPopupMenu->EnableMenuItem(ID_WAZOO_TAB_ON_LEFT, MF_BYCOMMAND | MF_GRAYED);
			pTabPopupMenu->EnableMenuItem(ID_WAZOO_TAB_ON_RIGHT, MF_BYCOMMAND | MF_GRAYED);
			pTabPopupMenu->CheckMenuItem(ID_WAZOO_TAB_SHOW, MF_BYCOMMAND | MF_UNCHECKED);
		}
		
		//
		// Enable the Show menu item only if there is a single tab.
		//
		if (m_wndTab.GetTabCount() != 1)
			pTabPopupMenu->EnableMenuItem(ID_WAZOO_TAB_SHOW, MF_BYCOMMAND | MF_GRAYED);

		//
		// Insert the temp menu in the SEC context menu.
		//
		pMenu->InsertMenu(0, MF_BYPOSITION | MF_SEPARATOR);
		pMenu->InsertMenu(0, MF_BYPOSITION | MF_POPUP, UINT(pTabPopupMenu->GetSafeHmenu()), CRString(IDS_WAZOOBAR_TAB_LOCATION));
	}
}


////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
// Create the 3D tab control window as a child window of this
// control bar.
////////////////////////////////////////////////////////////////////////
int CWazooBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (SECControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_wndTab.Create(this);

	//
	// And what the hell, let's register our OLE drag and drop object here...
	//
	VERIFY(m_dropTarget.Register(this));

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
// Blow away the Wazoo window objects contained in this Wazoo container.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnDestroy()
{
	ASSERT_VALID(&m_wndTab);

	//
	// We have to assume that there is no active view anymore when we
	// destroy a Wazoo container.  We need to cling to this assumption
	// so that the MFC Doc/Frame/View triad code doesn't crash when we
	// destroy views that aren't housed in CFrameWnd's ...  you know,
	// like with docked Wazoo bars.  If the Wazoo bar is floating or
	// in MDI mode, then note that its parent frame window is a mini
	// frame or MDI child frame, respectively.
	//

	CFrameWnd* pFrame = GetParentFrame();
	ASSERT(pFrame);

	pFrame->SetActiveView(NULL);    // deactivate during death

	for (int i = 0; i < m_wndTab.GetTabCount(); i++)
	{
		LPCTSTR lpszLabel = NULL;	// unused
		BOOL bSelected = FALSE;		// unused
		CWnd* pWnd = NULL;
		void* pExtra = NULL;		// unused

		if (m_wndTab.GetTabInfo(i, lpszLabel, bSelected, pWnd, pExtra))
		{
			CWazooWnd* pWazooWnd = DYNAMIC_DOWNCAST(CWazooWnd, pWnd);
			if (pWazooWnd)
			{
				//
				// Destroy the Wazoo windows...
				//
				ASSERT(NULL == pFrame->GetActiveView());
				ASSERT_VALID(pWazooWnd);
				ASSERT(::IsWindow(pWazooWnd->GetSafeHwnd()));
				pWazooWnd->DestroyWindow();
				delete pWazooWnd;
			}
			else
			{
				ASSERT(0);
			}
		}
	}

	SECControlBar::OnDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
// Whenever this parent control bar window is sized, we need to also
// size the child tab control window.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnSize(UINT nType, int cx, int cy)
{
	SECControlBar::OnSize(nType, cx, cy);

	// Don't do anything if the controls aren't created yet, or
	// the window is being minimized
	if (m_wndTab.m_hWnd == NULL || nType == SIZE_MINIMIZED)
		return;

	CRect rectInside;
	GetInsideRect(rectInside);
	m_wndTab.SetWindowPos(NULL, rectInside.left, rectInside.top, 
							rectInside.Width(), rectInside.Height(),
							SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
}


////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd [protected]
//
// Strangely, we need to handle the drawing of the control bar background
// ourselves.  This code is stolen from the SEC "VIZ" sample.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBar::OnEraseBkgnd(CDC* pDC)
{
	ASSERT_VALID(pDC);

	// Fill background with APPWORKSPACE
	CBrush backBrush(::GetSysColor(COLOR_BTNFACE));
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);
	CRect rect;
	pDC->GetClipBox(&rect);     // Erase the area needed

	CRect rectInside;
	GetInsideRect(rectInside);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rectInside.top - rect.top, PATCOPY);
	pDC->PatBlt(rect.left, rectInside.bottom, rect.Width(), rect.bottom - rectInside.bottom, PATCOPY);
	pDC->PatBlt(rect.left, rect.top, rectInside.left - rect.left, rect.Height(), PATCOPY);
	pDC->PatBlt(rectInside.right, rect.top, rect.right - rectInside.right, rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnOK [protected]
//
// Delegate handling to active Wazoo window.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnOK()
{
	CWazooWnd* pActive = NULL;

	if (m_wndTab.GetActiveTab((CWnd*&) pActive))
	{
		//
		// Forward the "IDOK" notification to the active
		// Wazoo window in the hope that it is smart enough
		// to do something with it.
		//
		ASSERT(pActive);
		ASSERT_KINDOF(CWazooWnd, pActive);
		pActive->SendMessage(WM_CHAR, VK_RETURN, 0);
	}
}


////////////////////////////////////////////////////////////////////////
// OnCancel [protected]
//
// Delegate handling to active Wazoo window.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnCancel()
{
	CWazooWnd* pActive = NULL;

	if (m_wndTab.GetActiveTab((CWnd*&) pActive))
	{
		//
		// Forward the "IDCANCEL" notification to the active
		// Wazoo window in the hope that it is smart enough
		// to do something with it.
		//
		ASSERT(pActive);
		ASSERT_KINDOF(CWazooWnd, pActive);
		pActive->SendMessage(WM_CHAR, VK_ESCAPE, 0);
	}
}


////////////////////////////////////////////////////////////////////////
// OnTimer [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnTimer(UINT nIDEvent)
{
	if (nIDEvent != AUTOACTIVATE_TIMER_EVENT_ID)
	{
		//
		// Let the base class deal with timers that we didn't set.
		//
		SECControlBar::OnTimer( nIDEvent );
		return;
	}

	if (0 == m_idAutoActivateTimer)
		return;					// ignore queued leftover timer messages

	CPoint ptScreen;
	::GetCursorPos(&ptScreen);	// screen coordinates

	//
	// Require at least two samples at the same cursor location
	// before doing an auto-activate.
	//
	static CPoint last_pt(-1, -1);
	if ((ptScreen.x == last_pt.x) && (ptScreen.y == last_pt.y))
	{
		//
		// Check to see if we're over a tab button.  If so,
		// then handle the auto-activate behavior here.
		//
		BOOL bSelected = FALSE;
		int nIndex = -1;		// unused
		CWnd* pWnd = QCTabHitTest(ptScreen, bSelected, nIndex);
		if (pWnd && !bSelected)
		{
			ASSERT_KINDOF(CWazooWnd, pWnd);
			m_wndTab.ActivateTab(pWnd);
			m_wndTab.ScrollToTab(pWnd);
		}
	}
	last_pt.x = ptScreen.x;
	last_pt.y = ptScreen.y;
}


////////////////////////////////////////////////////////////////////////
// OnWindowPosChanging [protected]
//
// Hook to generically detect Wazoo container window closure no 
// matter whether it is docked, floating, or in MDI mode.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (lpwndpos->flags & SWP_HIDEWINDOW)
	{
		//
		// Notify active wazoo that it is being deactivated.
		//
		CWazooWnd* pActive = NULL;

		if (m_wndTab.GetActiveTab((CWnd*&) pActive))
		{
			//
			// Forward the "you're about to be deactivated"
			// notification to the active Wazoo window.
			//
			ASSERT(pActive);
			ASSERT_KINDOF(CWazooWnd, pActive);
			pActive->OnDeactivateWazoo();		// virtual function
		}
	}

	SECControlBar::OnWindowPosChanging(lpwndpos);
}


////////////////////////////////////////////////////////////////////////
// OnCmdSECMDIFloat [protected]
//
// We need to hook and handle the generic ID_SEC_MDIFLOAT command in order
// to detect the transition from docked/floating mode to MDI mode.
// Yep.  We just need this hook so that we can set the title on the
// floating mini-frame or the MDI child frame properly.
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnCmdSECMDIFloat()
{ 
	//
	// Call standard base class handler to toggle the window
	// state.
	//
	SECControlBar::OnFloatAsMDIChild();

	RefreshWindowTitle();
}


////////////////////////////////////////////////////////////////////////
// OnTabsOnTop [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnTabsOnTop()
{ 
	m_wndTab.SetTabLocation(TWS_TABS_ON_TOP);
}


////////////////////////////////////////////////////////////////////////
// OnTabsOnBottom [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnTabsOnBottom()
{ 
	m_wndTab.SetTabLocation(TWS_TABS_ON_BOTTOM);
}


////////////////////////////////////////////////////////////////////////
// OnTabsOnLeft [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnTabsOnLeft()
{ 
	m_wndTab.SetTabLocation(TWS_TABS_ON_LEFT);
}


////////////////////////////////////////////////////////////////////////
// OnTabsOnRight [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnTabsOnRight()
{ 
	m_wndTab.SetTabLocation(TWS_TABS_ON_RIGHT);
}


////////////////////////////////////////////////////////////////////////
// OnTabsShowSingleTab [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooBar::OnTabsShowSingleTab()
{
	if (m_wndTab.GetTabCount() != 1)
	{
		ASSERT(0);		// theoretically, UI should never let us get here
		return;
	}

	//
	// Toggle visibility of tab.
	//
	m_wndTab.ShowTabs(! m_wndTab.TabsAreVisible());
}


////////////////////////////////////////////////////////////////////////
// OnTabSelect [protected]
//
// Process notification from child tab window that the selected tab
// changed.  Use this to set the parent window title to match
// the current tab selection.
////////////////////////////////////////////////////////////////////////
LRESULT CWazooBar::OnTabSelect(WPARAM wParam, LPARAM)
{
	LPCTSTR lpszLabel = NULL;
	BOOL bSelected = FALSE;
	CWnd* pWndUnused = NULL;		// unused
	void* pExtraUnused = NULL;		// unused

	if (m_wndTab.GetTabInfo(wParam, lpszLabel, bSelected, pWndUnused, pExtraUnused))
	{
		//TRACE1("CWazooBar::OnTabSelect: selected tab: %s\n", lpszLabel);

		if (IsMDIChild() || IsFloating())
		{
			//
			// Update the MDI child window or floating frame title.
			//
			CFrameWnd* pFrameWnd = (CFrameWnd *) GetParentFrame();
			ASSERT_KINDOF(CFrameWnd, pFrameWnd);
			pFrameWnd->SetWindowText(lpszLabel);

			//
			// Selected a tab, so update the associated "icon" associated
			// with the MDI frame window so that the Auto-Wazoo task
			// bar is drawn correctly.
			//
			if (IsMDIChild())
			{
				QCControlBarWorksheet* pSheet = DYNAMIC_DOWNCAST(QCControlBarWorksheet, pFrameWnd);
				if (pSheet)
				{
					pSheet->QCSetIcon(m_wndTab.QCGetIcon(wParam));
				}
				else
				{
					ASSERT(0);
				}

				//
				// Now, notify the main window so that it can redraw
				// the Auto-Wazoo task bar.
				//
				CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
				pMainFrame->PostMessage(TCM_TABSEL, wParam, 0);
			}
		}
	}
	else
	{
		ASSERT(0);		// uh, oh...
	}

    return 0;
}

void CWazooBar::OnBarFloat()
{
	/////////////////////////////////////////////////////////////////////////
	//
	// HACK ALERT!!
	//
	// Okay, if you have the Wazoo docked and click in a view the CMainFrame
	// gets the SetActiveView and stores a pointer to the view. Now you
	// float the Wazoo. While floating the CMainFrame does NOT get
	// the SetActiveView calls, so when the view is destroyed it cannot
	// update the CMainFrame pointer and that pointer is dangling.
	//
	// Hack Solution (tm) is to clear out the active view when we change
	// states.
	//
	/////////////////////////////////////////////////////////////////////////

	CFrameWnd *pParentFrame = GetParentFrame();
	ASSERT(pParentFrame);

	if (pParentFrame)
	{
		CView *pView = pParentFrame->GetActiveView();
		
		if ((pView) && (IsChild(pView)))
		{
			pParentFrame->SetActiveView(NULL);
		}
	}

	SECControlBar::OnBarFloat();
}

void CWazooBar::OnBarMDIFloat()
{
	/////////////////////////////////////////////////////////////////////////
	//
	// HACK ALERT!!
	//
	// Okay, if you have the Wazoo docked and click in a view the CMainFrame
	// gets the SetActiveView and stores a pointer to the view. Now you
	// float the Wazoo. While floating the CMainFrame does NOT get
	// the SetActiveView calls, so when the view is destroyed it cannot
	// update the CMainFrame pointer and that pointer is dangling.
	//
	// Hack Solution (tm) is to clear out the active view when we change
	// states.
	//
	/////////////////////////////////////////////////////////////////////////

	CFrameWnd *pParentFrame = GetParentFrame();
	ASSERT(pParentFrame);

	if (pParentFrame)
	{
		CView *pView = pParentFrame->GetActiveView();
		
		if ((pView) && (IsChild(pView)))
		{
			pParentFrame->SetActiveView(NULL);
		}
	}

	SECControlBar::OnBarMDIFloat();
}



/**********************************************************************/
/*                    CWazooBarOleDropTarget                          */
/**********************************************************************/


////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
// Handler for registered COleDropTarget::OnDragEnter() callbacks.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CWazooBarOleDropTarget::OnDragEnter(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CWazooBar)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	CWazooBar* pTargetWazooBar = (CWazooBar *) pWnd;		// type cast

	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_QCWILDCARD))
	{
		if (! g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_WAZOO))
		{
			//
			// Start auto-activate timer only if we're not dragging a wazoo
			// window.
			//
			TRACE("CWazooBarOleDropTarget::OnDragEnter: Start timer\n");
			ASSERT(0 == pTargetWazooBar->m_idAutoActivateTimer);
			pTargetWazooBar->m_idAutoActivateTimer = pTargetWazooBar->SetTimer(CWazooBar::AUTOACTIVATE_TIMER_EVENT_ID, 400, NULL);
		}
	}

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
// Handler for registered COleDropTarget::OnDragOver() callbacks.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CWazooBarOleDropTarget::OnDragOver(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CWazooBar)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	CWazooBar* pTargetWazooBar = (CWazooBar *) pWnd;		// type cast

	if (! g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_QCWILDCARD))
	{
		return DROPEFFECT_NONE;
	}

	//
	// Okay, we're not over a tab button, so see if we're dragging
	// a Wazoo window into this container.
	// 
	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_WAZOO))
	{
		// Get global Wazoo clipboard data from COleDataObject
		QCFWAZOO qcfWazoo;
		if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfWazoo))
		{
			ASSERT(0);
			return DROPEFFECT_NONE;
		}

		//
		// Doing wazoo drag, so check to see whether or not we're over
		// a tab in the target wazoo container.  If we're over a tab,
		// 'nIndex' will be a tab index instead of -1.
		//
		int nInsertionIndex = -1;
		{
			BOOL bSelected = FALSE;		// unused
			CPoint ptScreen(point);
			pTargetWazooBar->ClientToScreen(&ptScreen);
			if (pTargetWazooBar->QCTabHitTest(ptScreen, bSelected, nInsertionIndex))	// requires screen coordinates
			{
				ASSERT(nInsertionIndex >= 0 && nInsertionIndex < pTargetWazooBar->m_wndTab.GetTabCount());
			}
		}

		//
		// Check to see whether or not we're doing an Inter-wazoo drag
		// or Intra-wazoo drag.
		//
		ASSERT_KINDOF(CWazooBar, qcfWazoo.m_pSourceWazooBar);
		ASSERT_KINDOF(CWazooWnd, qcfWazoo.m_pSourceWazooWnd);
		if (qcfWazoo.m_pSourceWazooBar != pTargetWazooBar)
		{
			// Inter-wazoo drag
			if (nInsertionIndex != -1)
				return DROPEFFECT_LINK;		// over a tab
			else
				return DROPEFFECT_MOVE;		// not over a tab
		}
		else
		{
			//
			// Intra-wazoo drag -- source and target are the same, so this
			// means just to rearrange the tab order.
			//
			if (pTargetWazooBar->m_wndTab.GetTabCount() == 1)
			{
				//
				// Special case.  Can't do intra-wazoo drag if there's only
				// one wazoo window.  That would result in a no-op.
				//
				return DROPEFFECT_NONE;
			}
			else if (nInsertionIndex != -1)
			{
				//
				// Over tab while doing an intra-wazoo drag.  This gets
				// tricky.  If we're over the selected tab (i.e.  the
				// one that's being dragged), then this would end up
				// being a no-op, so disallow the drop insertion.
				//
				int nSelectionIndex = -1;
			    BOOL bFound = pTargetWazooBar->m_wndTab.FindTab(qcfWazoo.m_pSourceWazooWnd, nSelectionIndex);
				ASSERT(bFound);
				ASSERT(nSelectionIndex >= 0 && nSelectionIndex < pTargetWazooBar->m_wndTab.GetTabCount());
				if (nSelectionIndex == nInsertionIndex)
					return DROPEFFECT_NONE;
				else
					return DROPEFFECT_LINK;
			}
			else
			{
				//
				// Not over a tab, so indicate a "move" for moving a
				// tab to the "end" of the tab order.
				//
				return DROPEFFECT_MOVE;		
			}
		}
	}
	else
	{
		//
		// Okay, clipboard has usable non-wazoo drop data from our
		// instance, so let's do some hit tests to see if we're over a
		// valid drop target -- i.e., a tab button.
		//
		BOOL bSelected = FALSE;
		int nIndex = -1;			// unused
		CPoint ptScreen(point);
		pTargetWazooBar->ClientToScreen(&ptScreen);
		if (pTargetWazooBar->QCTabHitTest(ptScreen, bSelected, nIndex))	// requires screen coordinates
		{
			//
			// Yup.  We're over a tab button.  Use the DROPEFFECT_LINK
			// as a hack indication to the OleDropSource that 
			// auto-activate can happen.
			//
			if (bSelected)
				return DROPEFFECT_NONE;
			else
				return DROPEFFECT_LINK;
		}
	}

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
// Handler for registered COleDropTarget::OnDrop() callbacks.
////////////////////////////////////////////////////////////////////////
BOOL CWazooBarOleDropTarget::OnDrop(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect, 
	CPoint point)
{
	ASSERT_VALID(this);
	TRACE("CWazooBarOleDropTarget::OnDrop\n");

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CWazooBar)))
	{
		ASSERT(0);
		return FALSE;
	}

	CWazooBar* pTargetWazooBar = (CWazooBar *) pWnd;		// type cast

	//
	// First things first ... always kill the auto activate timer.
	//
	if (pTargetWazooBar->m_idAutoActivateTimer)
	{
		pTargetWazooBar->KillTimer(pTargetWazooBar->m_idAutoActivateTimer);
		pTargetWazooBar->m_idAutoActivateTimer = 0;
	}

	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_WAZOO))
	{
		QCFWAZOO qcfWazoo;
		if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfWazoo))
		{
			ASSERT(0);
			return FALSE;
		}

		CPoint ptScreen(point);
		pTargetWazooBar->ClientToScreen(&ptScreen);

		//
		// This is the big moment!  Two cases here -- either the
		// source and target windows are the same or they are
		// different.
		//
		ASSERT_KINDOF(CWazooBar, qcfWazoo.m_pSourceWazooBar);
		ASSERT_KINDOF(CWazooWnd, qcfWazoo.m_pSourceWazooWnd);
		if (qcfWazoo.m_pSourceWazooBar == pTargetWazooBar)
		{
			//
			// Intra-wazoo drag -- source and target are the same, so this
			// just means rearranging the tab order.
			//
			pTargetWazooBar->ShuffleWazooWindow(qcfWazoo.m_pSourceWazooWnd, ptScreen);
		}
		else
		{
			//
			// Inter-wazoo drag -- source and target are different, so
			// this means moving the Wazoo from one container to
			// another.  We don't have to worry about cleaning up empty
			// containers here since the container takes care of that
			// on the source side of the drag and drop handler.
			//
			qcfWazoo.m_pSourceWazooBar->RemoveWazooWindow(qcfWazoo.m_pSourceWazooWnd);
			pTargetWazooBar->AddWazooWindow(qcfWazoo.m_pSourceWazooWnd, ptScreen);
			pTargetWazooBar->m_wndTab.ShowTabs(TRUE);		// make sure tabs are visible
			CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
			ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);
			pMainFrame->RecalcLayout();
		}

		pTargetWazooBar->ActivateWazooWindow(qcfWazoo.m_pSourceWazooWnd->GetRuntimeClass());
		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnDropEx [public, virtual]
//
// Handler for registered COleDropTarget::OnDropEx() callbacks.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CWazooBarOleDropTarget::OnDropEx(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropDefault, 
	DROPEFFECT dropList, 
	CPoint point)
{
	ASSERT_VALID(this);
	TRACE("CWazooBarOleDropTarget::OnDropEx\n");

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CWazooBar)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	//
	// Normally, this would delegate to the CWazooBar::OnDropEx()
	// method.  However, since we're currently not doing anything like
	// right button drags, let's just return a "not implemented" status
	// code to force MFC to call the older OnDrop() method.
	//
	return DROPEFFECT(-1);
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
// Handler for registered COleDropTarget::OnDragLeave() callbacks.
////////////////////////////////////////////////////////////////////////
void CWazooBarOleDropTarget::OnDragLeave(CWnd* pWnd)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CWazooBar)))
	{
		ASSERT(0);
		return;
	}

	// delegates to Wazoo Bar
	CWazooBar* pTargetWazooBar = (CWazooBar *) pWnd;

	//
	// Stop auto-activate timer
	//
	if (pTargetWazooBar->m_idAutoActivateTimer)
	{
		TRACE("CWazooBarOleDropTarget::OnDragLeave: Kill timer\n");
		pTargetWazooBar->KillTimer(pTargetWazooBar->m_idAutoActivateTimer);
		pTargetWazooBar->m_idAutoActivateTimer = 0;
	}
}

