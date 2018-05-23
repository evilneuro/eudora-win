// AdWazooBar.cpp : implementation file
//
// CAdWazooBar
// Class that handles the particular wazoo container that holds the Ad window

#include "stdafx.h"

#include "resource.h"
#include "mainfrm.h"
#include "guiutils.h"

#include "AdWazooBar.h"
#include "AdWazooWnd.h"
#include "QCSharewareManager.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CAdWazooBar, CWazooBar)

BEGIN_MESSAGE_MAP(CAdWazooBar, CWazooBar)
	//{{AFX_MSG_MAP(CAdWazooBar)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SEC_MDIFLOAT, OnCmdSECMDIFloat)
	ON_COMMAND(ID_WAZOO_TAB_SHOW, OnTabsShowSingleTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CAdWazooBar
CAdWazooBar::~CAdWazooBar()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
		pSWM->UnRegister(this);
}


////////////////////////////////////////////////////////////////////////
// Create [public]
//
// We need to specify some particular parameters to the Create() call
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::Create(CWnd* pParentWnd)
{
	return CWazooBar::Create(pParentWnd, "AdWazooBar",
								CBRS_LEFT | CBRS_BORDER_ANY | CBRS_BORDER_3D | WS_VISIBLE | CBRS_SIZE_DYNAMIC,
								0, IDC_AD_WAZOO_BAR);
}


////////////////////////////////////////////////////////////////////////
// SetDefaultWazooConfig [public, virtual]
//
// Set up the Ad wazoo bar in the default state
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::SetDefaultWazooConfig(int /*nBar*/)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));

	m_ImageWidth = 128;
	m_ImageHeight = 144;

	CreateWazooWindow(RUNTIME_CLASS(CAdWazooWnd));

	if (IsFloating() && !IsMDIChild())
	{
		// Right now we're not planning on allowing the Ad window to float, but if we
		// do at least it will be taken care of, and this ASSERT can be removed.
		ASSERT(0);

		//
		// Special case for creation of child Wazoos inside of a
		// floating window...  need to notify the child wazoos to
		// perform their OnInitialUpdate() sequence.
		//
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
	}

	// Hide tabs and tell tab window not to draw 3D borders
	m_wndTab.ShowTabs(FALSE);
	m_wndTab.SetDraw3DBorder(FALSE);

	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
		pSWM->Register(this);

	UpdateState();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// LoadWazooConfigFromIni [public, virtual]
//
// We don't really load up the Ad wazoo bar from the INI file, but
// instead just create the Ad window and some default parameters.
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::LoadWazooConfigFromIni()
{
	QCWorkbook *pMainFrame = (QCWorkbook*)AfxGetMainWnd();
	pMainFrame->DockControlBarEx(this, AFX_IDW_DOCKBAR_LEFT, 100, 0, (float)0.25, 160);

	SetDefaultWazooConfig(0);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SaveWazooConfigToIni [public, virtual]
//
// Do nothing since we don't save the Ad wazoo container to the
// [WazooBars] section of the INI file.
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::SaveWazooConfigToIni()
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CalcDynamicLayout [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
CSize CAdWazooBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	CSize theSize = SECControlBar::CalcDynamicLayout(nLength, dwMode);

	// Ad window has minimum horizontal and vertical sizes
	theSize.cx = __max(theSize.cx, m_rcBorderSpace.left + m_rcBorderSpace.right + m_ImageWidth);
	theSize.cy = __max(theSize.cy, m_rcBorderSpace.top + m_rcBorderSpace.bottom + m_ImageHeight);

	return theSize;
}

////////////////////////////////////////////////////////////////////////
// CalcFixedLayout [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
CSize CAdWazooBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize theSize = SECControlBar::CalcFixedLayout(bStretch, bHorz);

	// Ad window has minimum horizontal and vertical sizes
	theSize.cx = __max(theSize.cx, m_rcBorderSpace.left + m_rcBorderSpace.right + m_ImageWidth);
	theSize.cy = __max(theSize.cy, m_rcBorderSpace.top + m_rcBorderSpace.bottom + m_ImageHeight);

	return theSize;
}

////////////////////////////////////////////////////////////////////////
// UpdateState [protected]
//
// Show/Hide Ad wazoo bar based on current state of program.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::UpdateState()
{
	if (GetSharewareMode() == SWM_MODE_ADWARE)
		m_pManager->ActivateWazooWindow((CMainFrame*)AfxGetMainWnd(), RUNTIME_CLASS(CAdWazooWnd));
	else
		OnHide();
}

////////////////////////////////////////////////////////////////////////
// Notify [public, virtual]
//
// Notify us when program changes state between Light, Pro, and Adware.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::Notify(QCCommandObject* /*pObject*/, COMMAND_ACTION_TYPE theAction, void* pData /*= NULL*/)
{
	if (theAction == CA_SWM_CHANGE_FEATURE)
	{
		const BOOL bCurrentModeIsAdware = GetSharewareMode() == SWM_MODE_ADWARE;
		const BOOL bOldModeWasAdware = pData? (*(SharewareModeType*)pData == SWM_MODE_ADWARE) : !bCurrentModeIsAdware;

		if (bOldModeWasAdware != bCurrentModeIsAdware)
			UpdateState();
	}
}


////////////////////////////////////////////////////////////////////////
// OnExtendContextMenu [protected, virtual]
//
// Override base class implementation to provide tab choices.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnExtendContextMenu(CMenu* /*pMenu*/)
{
	// Should never get here!
	ASSERT(0);
}

////////////////////////////////////////////////////////////////////////
// OnUpdateCmdUI [protected, virtual]
//
// Use this to check if the Ad window is being hidden.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
	if (GetSharewareMode() != SWM_MODE_ADWARE)
		return;

	static LastCheckTime = 0;

	// Only check every 60 seconds
	if (time(NULL) > LastCheckTime + 60)
	{
		if (LastCheckTime)
		{
			// This code assumes that all UI elements get created from the main thread,
			// which is a pretty safe assumption for good coding practices.
			HWND hWnd = ::GetForegroundWindow();
			if (hWnd && GetWindowThreadProcessId(hWnd, NULL) == GetCurrentThreadId())
			{
				CRect rect;
				GetWindowRect(rect);
				HWND hWndAtCenter = ::WindowFromPoint(rect.CenterPoint());
				if (hWndAtCenter != m_hWnd && !::IsChild(m_hWnd, hWndAtCenter))
				{
					// Somebody's trying to pull a fast one!
					ErrorDialog(IDS_ERR_HIDDEN_AD_WINDOW);
				}
			}
		}

		LastCheckTime = time(NULL);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CAdWazooBar message handlers


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*ptScreen*/)
{
	// Just eat right-click context menus in Ad wazoo bar because we don't want the
	// default behavior of the wazoo bar allowing things like undocking, hiding, etc.
}


////////////////////////////////////////////////////////////////////////
// OnCmdSECMDIFloat [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnCmdSECMDIFloat()
{
	// Ad window can't be an MDI child window!
	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnTabsShowSingleTab [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnTabsShowSingleTab()
{
	// Shouldn't be able to get here, but just in case don't do anything.
	ASSERT(0);
}
