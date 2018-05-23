// helpcntx.cpp : implementation file
//		  

#include "stdafx.h"
#include "eudora.h"
#include "helpcntx.h"
#include "utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



/////////////////////////////////////////////////////////////////////////////
// CContextHelp


CContextHelp::~CContextHelp()
{
}

CContextHelp::CContextHelp()
{
}

#ifdef WIN32
CContextHelp::CContextHelp(HWND hWnd)
{
	// Display help for the HELP_WM_HELP

	// Builds an array of paired control IDs and Help IDs 
	// for a particular window.  WinHelp uses this array
	// to determine what help topic to display. 

	DWORD* paIDs=m_aHelpID;

	// Make sure window has a parent
	if ((GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) != 0)
	{
		CWnd* pWnd = CWnd::FromHandle(hWnd);
		pWnd= pWnd->GetParent();

		// walk through all child windows
		CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
		while (pWndChild != NULL)
		{
			UINT nID =  pWndChild->GetDlgCtrlID();

			// WinHelp ends on Ids that eqaul zero
			// So skip 'em.
			if ( nID != 0 )
			{	
				*paIDs++ = (DWORD)nID;	 
				*paIDs++ = HID_OFFSET+nID;
			}
			pWndChild = pWndChild->GetWindow( GW_HWNDNEXT);
		}
		// Terminating Ids.
		*paIDs++ = 0;
		*paIDs = 0;
	}		   
	if(!::WinHelp(hWnd, AfxGetApp()->m_pszHelpFilePath, HELP_WM_HELP, (DWORD) (LPVOID) m_aHelpID))
		AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP); 
		
	return;

}
#endif

CContextHelp::CContextHelp(HWND hWnd, CPoint point, BOOL bClientCoords /*=TRUE*/ )
	// bClientCoords tells whether the CPoint is in client or screen coordinates
{
	if (!bClientCoords)
		ScreenToClient(hWnd, &point);

	HWND hWndFound = hWnd;

	while (1)
	{
		HWND hWndChild = ChildWindowFromPointEx(hWndFound, point, CWP_SKIPINVISIBLE);
		if (!hWndChild || hWndChild == hWndFound)
			break;
		MapWindowPoints(hWndFound, hWndChild, &point, 1);
		hWndFound = hWndChild;
	}

	if (hWndFound != hWnd)
		AfxGetApp()->WinHelp(GetDlgCtrlID(hWndFound) + HID_OFFSET);
}


/////////////////////////////////////////////////////////////////////////////
// CContextMenu

BOOL g_bPopupOn=FALSE;

CContextMenu::~CContextMenu()
{
}

CContextMenu::CContextMenu()
{
}



CContextMenu::CContextMenu(UINT nMenuPos, int xPos, int yPos, CWnd* ParentWnd /*=NULL*/ )
{
	if ( ParentWnd == NULL )
		ParentWnd = AfxGetMainWnd();

	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = QCLoadMenu(IDR_CONTEXT_POPUPS);
	if ( ! hMenu || !menu.Attach( hMenu ) )
	   return ;
	
	// SubMenu(x) is the specific popup.
	CMenu* menuPopup = menu.GetSubMenu(nMenuPos);

	g_bPopupOn=TRUE;
	menuPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,xPos,yPos,ParentWnd,NULL); 
	g_bPopupOn=FALSE;

	menu.DestroyMenu();
}

CContextMenu::CContextMenu(	CMenu* pPopupMenu, int xPos, int yPos, CWnd* ParentWnd /*=NULL*/ )
{
	if ( ParentWnd == NULL )
		ParentWnd = AfxGetMainWnd();

 	g_bPopupOn=TRUE;
	pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,xPos,yPos,ParentWnd,NULL); 
	g_bPopupOn=FALSE;

}


////////////////////////////////////////////////////////////////////////
// MatchCoordinatesToWindow [public, static]
//
// This is the MakeSchmookie(tm) command for context menu coordinates.
// When you press the Shift+F10 shortcut to launch a context menu
// you tend to either get (0,0) or (0xFFFF,0xFFFF) as the screen
// coordinates for the menu.  This is not good.  But luckily, the
// WM_CONTEXTMENU message includes the HWND of the window that
// is posting the context menu, so we can use that to make sure the
// coordinates are somewhere in the window.  If the coordinates are
// outside the window, we move the coordinates to the upper left corner
// of the window -- you know, make it schmookie.
////////////////////////////////////////////////////////////////////////
void CContextMenu::MatchCoordinatesToWindow(HWND hWnd, CPoint& ptScreen)
{
	ASSERT(::IsWindow(hWnd));

	//
	// Grab the screen coordinates of the window and do a hit test to
	// see if the given coordinates are in the window.  If so, we're
	// already Schmookie and we can leave the coordinates alone.
	//
	CRect rectWnd;
	::GetWindowRect(hWnd, rectWnd);		// screen coordinates

	if (rectWnd.PtInRect(ptScreen))
		return;							// point is already schmookie

	//
	// If we get this far, the point is outside the window, so make
	// the point schmookie by moving it to the upper left corner of 
	// the window rect.
	//
	ptScreen.x = rectWnd.left + 2;
	ptScreen.y = rectWnd.top + 2;
}
