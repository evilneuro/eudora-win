// helpxdlg.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "helpxdlg.h"
#include "helpcntx.h"

#ifdef WIN32
#include <winuser.h>
#endif

#include <afxpriv.h>

#include "DebugNewHelpers.h"

// Necessary for 16-bit context menus
#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU WM_RBUTTONUP
#endif

/////////////////////////////////////////////////////////////////////////////
// CHelpxDlg dialog


CHelpxDlg::CHelpxDlg(UINT nID,  CWnd* pParent /*=NULL*/)
	: CDialog(nID, pParent)
{
	//{{AFX_DATA_INIT(CHelpxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void CHelpxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHelpxDlg, CDialog)
	//{{AFX_MSG_MAP(CHelpxDlg)
	//}}AFX_MSG_MAP

#ifdef WIN32
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_MESSAGE(WM_HELP, OnHelp)	
#endif
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
#ifdef WIN32
	ON_MESSAGE(WM_RBUTTONUP, OnRightButton)
#endif
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
long CHelpxDlg::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
	// Called when the user enters help mode and hits a toolbar button.
	// Returns the Help ID of the selected button.

	// Not implemented since a modal dialog doesn't allow for this help mode
	return 0;
}

long CHelpxDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
	// Called when the user presses F1 in a control.

		HELPINFO* lphi;
		lphi = (HELPINFO*)lParam;
		if ( lphi->iContextType == HELPINFO_WINDOW)
			CContextHelp((HWND)lphi->hItemHandle);

	return TRUE;	
} 

#endif

long CHelpxDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	// Called when the user right clicks on a control.

	// Invoke help
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	CPoint point(x,y);
	
	CContextHelp( m_hWnd, point, FALSE);

	return TRUE;
}

#ifdef WIN32
long CHelpxDlg::OnRightButton(WPARAM wParam, LPARAM lParam)
{
	if (IsWin32s() || (IsWinNT() &&	GetMajorVersion() == 3 && GetMinorVersion() < 51))
		return (OnContextMenu(wParam, lParam));
	return (CDialog::DefWindowProc(WM_RBUTTONUP, wParam, lParam));
}
#endif



LRESULT CHelpxDlg::OnCommandHelp(WPARAM, LPARAM lParam)
{  
#ifndef WIN32
	CWnd* pWnd = GetFocus();
	int nID = pWnd->GetDlgCtrlID();
	WinHelp(nID+HID_OFFSET,HELP_CONTEXTPOPUP);
#endif
	// 32bit version eats this message.  OnHelp covers for it.
 	return TRUE;
}            



 
