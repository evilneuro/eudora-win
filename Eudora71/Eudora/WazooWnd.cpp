// WazooWnd.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "rs.h"
#include "utils.h"
#include "WazooWnd.h"
#include "WazooBar.h"

#include "DebugNewHelpers.h"

UINT umsgActivateWazoo = ::RegisterWindowMessage("umsgActivateWazoo");

/////////////////////////////////////////////////////////////////////////////
// CWazooWnd

IMPLEMENT_DYNAMIC(CWazooWnd, CWnd)

BEGIN_MESSAGE_MAP(CWazooWnd, CWnd)
	//{{AFX_MSG_MAP(CWazooWnd)
	ON_WM_MENUSELECT()
	//}}AFX_MSG_MAP
	ON_WM_INITMENUPOPUP()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_SEC_ALLOWDOCKING, OnCmdSECAllowDocking)
	ON_COMMAND(ID_SEC_HIDE, OnCmdSECHide)
	ON_COMMAND(ID_SEC_MDIFLOAT, OnCmdSECMDIFloat)
	ON_UPDATE_COMMAND_UI(ID_SEC_ALLOWDOCKING, OnCmdUpdateSECAllowDocking)
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()


CWazooWnd::CWazooWnd(UINT nID) :
	m_nID(nID),
	m_hwndFocus(NULL)
{
	//
	// Initialize standard data members, including the handle to the
	// 16x16 small icon and the tab label.
	//
	HINSTANCE hResInst = QCFindResourceHandle(MAKEINTRESOURCE(m_nID), RT_GROUP_ICON);
	m_hIcon = HICON(::LoadImage(hResInst, MAKEINTRESOURCE(m_nID), IMAGE_ICON, 16, 16, 0));
	ASSERT(m_hIcon);

	m_strTabLabel = CRString(m_nID);
}

CWazooWnd::~CWazooWnd()
{
}


////////////////////////////////////////////////////////////////////////
// HasActiveFocus [public]
//
// Return TRUE if this is the active Wazoo window in the parent control
// bar AND a child of the Wazoo window currently owns the keyboard
// focus.
////////////////////////////////////////////////////////////////////////
BOOL CWazooWnd::HasActiveFocus() const
{
	CWazooBar* pParent = (CWazooBar *) GetParentControlBar();

	if (NULL == pParent)
	{
		ASSERT(0);
		return FALSE;
	}

	ASSERT_KINDOF(CWazooBar, pParent);
	if (this == pParent->GetActiveWazooWindow())
	{
		CWnd* pFocusWnd = CWnd::GetFocus();
		if (IsChild(pFocusWnd))
			return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// DoContextMenu [public]
//
////////////////////////////////////////////////////////////////////////
void CWazooWnd::DoContextMenu(
CWnd*	pWnd, 
CPoint	point,
HMENU	hMenu )
{
	DWORD		dwStyle;	
	CMenu		menu;
	CMenu		menuSrc;
	INT			i;
	INT			iMax;
	UINT		uID;
	CString		szString;
	CRString	szAllowDocking( ID_SEC_ALLOWDOCKING );
	CRString	szHide( ID_SEC_HIDE );
	CRString	szFloat( ID_SEC_MDIFLOAT);
	
	CWazooBar*	pBar = DYNAMIC_DOWNCAST(CWazooBar, GetParentControlBar());	
	if( pBar == NULL )
	{
		// this should never happen
		ASSERT( 0 );
		return;
	}

	if( !menu.CreatePopupMenu() )
	{
		ASSERT( 0 );
		return;
	}
	
	if( hMenu == NULL )
	{
		ASSERT( 0 );

		menu.DestroyMenu();
		return;
	}
		
	// attach to the menu object
	menuSrc.Attach( hMenu );

	iMax = menuSrc.GetMenuItemCount();

	// loop through the entries and add them to the context menu

	for( i = 0; i < iMax ; i++ )
	{
		// get the source menu id & string
		uID = menuSrc.GetMenuItemID( i );
		
		if( uID == 0 )
		{
			// add a separator
			menu.InsertMenu( i, MF_SEPARATOR );

		}
		else if ( uID == -1 )
		{
			// add a popup
			CMenu* pPopupMenu = menuSrc.GetSubMenu( i );
			menuSrc.GetMenuString( i, szString, MF_BYPOSITION );			
			menu.InsertMenu( i, MF_POPUP|MF_BYPOSITION, UINT(pPopupMenu->GetSafeHmenu()), szString);
		}
		else
		{
			// add the item
			menuSrc.GetMenuString( i, szString, MF_BYPOSITION );			
			menu.InsertMenu( i, MF_STRING, uID, szString );
		}	 		

	}

	if( pBar != NULL )
	{
		// add the Allow Docking string
		dwStyle = MF_STRING;
		if (pBar->m_dwDockStyle & CBRS_ALIGN_ANY)
			dwStyle |= MF_CHECKED;
		menu.AppendMenu( dwStyle, ID_SEC_ALLOWDOCKING, szAllowDocking );

		// add the Hide string
		menu.AppendMenu( MF_STRING, ID_SEC_HIDE, szHide );
		menu.AppendMenu( MF_SEPARATOR );
		
		// add the float string
		dwStyle = MF_STRING;
		if( pBar->IsMDIChild() )
			dwStyle |= MF_CHECKED;
		menu.AppendMenu( dwStyle, ID_SEC_MDIFLOAT, szFloat );
	}

	menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);

	// destroy the source menu
	menuSrc.DestroyMenu();
}


////////////////////////////////////////////////////////////////////////
// GetParentControlBar [public]
//
// Returns a pointer to the parent control bar object, if any.  Otherwise,
// return NULL if this tree control is not a child of a control bar.
////////////////////////////////////////////////////////////////////////
CControlBar* CWazooWnd::GetParentControlBar() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	CWnd* pParentWnd = GetParent();  // start with one parent up
	while (pParentWnd != NULL)
	{
		if (pParentWnd->IsKindOf(RUNTIME_CLASS(CControlBar)))
			return (CControlBar *) pParentWnd;
		pParentWnd = pParentWnd->GetParent();
	}
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// SaveInitialChildFocus [public, static]
//
////////////////////////////////////////////////////////////////////////
void CWazooWnd::SaveInitialChildFocus(const CWnd* pChildWnd)
{
	ASSERT(pChildWnd);
	ASSERT(::IsWindow(pChildWnd->GetSafeHwnd()));

	for (CWnd* pWnd = pChildWnd->GetParent(); pWnd; pWnd = pWnd->GetParent())
	{
		CWazooWnd* pWazooWnd = DYNAMIC_DOWNCAST(CWazooWnd, pWnd);
		if (pWazooWnd)
		{
			ASSERT(NULL == pWazooWnd->m_hwndFocus);
			pWazooWnd->m_hwndFocus = pChildWnd->GetSafeHwnd();
			return;
		}
	}

	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// This is called when this Wazoo window is about to be "activated" (displayed).
// This method is meant to be implemented in each derived class -- this
// base class implementation does nothing.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnActivateWazoo()
{
	//
	// If we get here, then tsk tsk, this object doesn't support 
	// Just In Time display initialization.
	//
	TRACE1("CWazooWnd::OnActivateWazoo: Warning: called base implementation for: %s\n", GetRuntimeClass()->m_lpszClassName);
}


////////////////////////////////////////////////////////////////////////
// OnDeactivateWazoo [public, virtual]
//
// This is called when this Wazoo window is about to be "deactivated".
// In practice, this means either when the user selects another tab in
// a Wazoo group, when the user activates a Wazoo via the menu, or
// when the user closes the Wazoo container.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnDeactivateWazoo()
{
	SaveChildWithFocus();
}


////////////////////////////////////////////////////////////////////////
// SaveChildWithFocus [protected]
//
// Modeled after the MFC implementation in CFormView::SaveFocusControl(),
// except this works with Wazoo windows.
////////////////////////////////////////////////////////////////////////
BOOL CWazooWnd::SaveChildWithFocus()
{
	CWnd* pFocusWnd = CWnd::GetFocus();
	if (pFocusWnd && IsChild(pFocusWnd))
	{
		pFocusWnd->SendMessage(WM_KILLFOCUS);
		m_hwndFocus = pFocusWnd->GetSafeHwnd();
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CWazooWnd message handlers


////////////////////////////////////////////////////////////////////////
// OnInitMenuPopup [protected]
//
// WARNING!  HACK ALERT!  Since a WazooWnd is derived from CWnd, we
// don't inherit that neat-o CmdUI stuff that comes for free with
// CFrameWnd.  Therefore, we stole the CmdUI implementation directly
// from CFrameWnd::OnInitMenuPopup() pretty much verbatim.  If MFC
// ever changes, we could be in deep doo-doo.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnInitMenuPopup(CMenu* pMenu, UINT, BOOL bSysMenu)
{
//	extern void AFXAPI AfxCancelModes(HWND);
//	AfxCancelModes(m_hWnd);

	if (bSysMenu)
		return;     // don't support system menu

	ASSERT(pMenu != NULL);
	// check the enabled state of various menu items

	CCmdUI state;
	state.m_pMenu = pMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// determine if menu is popup in top-level menu and set m_pOther to
	//  it if so (m_pParentMenu == NULL indicates that it is secondary popup)
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pMenu->m_hMenu)
		state.m_pParentMenu = pMenu;    // parent == child for tracking popup
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = GetTopLevelParent();
			// child windows don't have menus -- need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pMenu->m_hMenu)
				{
					// when popup is found, m_pParentMenu is containing menu
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		state.m_nID = pMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // menu separator or invalid cmd - ignore it

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// possibly a popup menu, route to first item of that popup
			state.m_pSubMenu = pMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // first item of popup can't be routed to
			}
			state.DoUpdate(this, FALSE);    // popups are never auto disabled
		}
		else
		{
			// normal menu item
			// Auto enable/disable if frame window has 'm_bAutoMenuEnable'
			//    set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, /*m_bAutoMenuEnable &&*/ state.m_nID < 0xF000);
		}

		// adjust for menu deletions and additions
		UINT nCount = pMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}


////////////////////////////////////////////////////////////////////////
// OnMenuSelect [protected]
//
// Do the status bar prompt "fly-bys" for our home grown context menus.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
	CMenu		theMenu;
	CFrameWnd*	pFrame;

	CWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);
	
	pFrame = ( CFrameWnd* ) AfxGetMainWnd();
	
	if( ( nFlags & 0xFFFF ) == 0xFFFF )
	{
		// menu was closed -- restore the prompt
		// Restore default status bar prompt.
		pFrame->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		return;
	}
	
	if( nFlags & MF_HILITE ) 
	{
		CRString	szString( nItemID );
	
		pFrame->SetMessageText( szString );
		pFrame->GetMessageBar()->UpdateWindow();
	}
}


////////////////////////////////////////////////////////////////////////
// OnSetFocus [protected]
//
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnSetFocus(CWnd*)
{
	// Make sure the "Active View" is set correctly
	CWnd* pWnd = CWnd::FromHandle(m_hwndFocus);
	CFrameWnd* pFrame = GetParentFrame();
	CView* pActiveView = NULL;

	if (pWnd && pFrame && IsChild(pWnd))
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

		pFrame->SetActiveView(pActiveView);
	}

	if (!::IsWindow(m_hwndFocus))
	{
		// invalid or unknown focus window... let windows handle it
		m_hwndFocus = NULL;
		Default();
		return;
	}

	// otherwise, set focus to the last known focus window
	::SetFocus(m_hwndFocus);
}


////////////////////////////////////////////////////////////////////////
// OnCmdSECMDIFloat [protected]
//
// Since we're rolling our own context menu and sticking some
// SECControlBar commands onto our menu ...  guess what?  Yep.  We
// need to provide our own command handler proxy that just forwards
// the command to the SECControlBar implementation.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnCmdSECMDIFloat()
{ 
	SECControlBar* p_ctrlbar = (SECControlBar *) GetParentControlBar();
	if (p_ctrlbar)
	{
		ASSERT_KINDOF(SECControlBar, p_ctrlbar);
		p_ctrlbar->SendMessage(WM_COMMAND, ID_SEC_MDIFLOAT, 0);
		return;
	}
	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnCmdSECAllowDocking [protected]
//
// Since we're rolling our own context menu and sticking some
// SECControlBar commands onto our menu ...  guess what?  Yep.  We
// need to provide our own command handler proxy that just forwards
// the command to the SECControlBar implementation.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnCmdSECAllowDocking()
{ 
	SECControlBar* p_ctrlbar = (SECControlBar *) GetParentControlBar();
	if (p_ctrlbar)
	{
		ASSERT_KINDOF(SECControlBar, p_ctrlbar);
		p_ctrlbar->SendMessage(WM_COMMAND, ID_SEC_ALLOWDOCKING, 0);
		return;
	}
	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnCmdSECHide [protected]
//
// Since we're rolling our own context menu and sticking some
// SECControlBar commands onto our menu ...  guess what?  Yep.  We
// need to provide our own command handler proxy that just forwards
// the command to the SECControlBar implementation.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnCmdSECHide()
{ 
	SECControlBar* p_ctrlbar = (SECControlBar *) GetParentControlBar();
	if (p_ctrlbar)
	{
		ASSERT_KINDOF(SECControlBar, p_ctrlbar);
		p_ctrlbar->SendMessage(WM_COMMAND, ID_SEC_HIDE, 0);
		return;
	}
	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateSECAllowDocking [protected]
//
// Since we're rolling our own context menu and sticking some
// SECControlBar commands onto our menu ...  guess what?  Yep.  We
// need to provide our own command UI handler proxy that mimics
// the SECControlBar implementation.
////////////////////////////////////////////////////////////////////////
void CWazooWnd::OnCmdUpdateSECAllowDocking(CCmdUI* pCmdUI)
{
	SECControlBar* p_ctrlbar = (SECControlBar *) GetParentControlBar();
	if (p_ctrlbar)
	{
		ASSERT_KINDOF(SECControlBar, p_ctrlbar);
		pCmdUI->Enable(! p_ctrlbar->IsMDIChild());
		return;
	}
	ASSERT(0);
}

////////////////////////////////////////////////////////////////////////
// OnCommandHelp [protected]
//
// Wazoo windows act like a frame window, but are derived from a plain
// CWnd, so we need to fake what a frame window would do in the case of
// being asked to handle ID_HELP, which is call WinHelp() with the
// HID_BASE_RESOURCE + the ID of the window.
////////////////////////////////////////////////////////////////////////
LRESULT CWazooWnd::OnCommandHelp(WPARAM, LPARAM)
{
	AfxGetApp()->CWinApp::WinHelp(HID_BASE_RESOURCE + m_nID);

	return TRUE;
}
