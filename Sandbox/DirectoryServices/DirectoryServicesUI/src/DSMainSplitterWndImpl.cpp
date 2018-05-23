//////////////////////////////////////////////////////////////////////////////
// DSMainSplitterWndImpl.cpp
// 
//
// Created: 10/09/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_MAIN_SPLITTER_WND_INTERFACE_IMPLEMENTATION_
#include "DSMainSplitterWndImpl.h"

IMPLEMENT_DYNCREATE(DSMainSplitterWnd, CSplitterWnd)

BEGIN_MESSAGE_MAP(DSMainSplitterWnd, CSplitterWnd)
    ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

DSMainSplitterWnd::DSMainSplitterWnd()
    : nCurHeight(INT_MIN)
{
}

BOOL
DSMainSplitterWnd::SerializeSerializables()
{
    BOOL        bWritten   = FALSE;

    // Write the widht of the right pane.
    CRString    dsSect(IDS_NDS_DIRSERV_SECTION),
	        dsPanesY(IDS_NDS_DIRSERV_PANES_Y);

    char        buffer[32] = { '\0' };
    const char *section    = dsSect;
    const char *panesY     = dsPanesY;
    const char *itoaBuf    = ::itoa(nCurHeight, buffer, 10);
    const char *iniPath    = INIPath;

    if (section && panesY && itoaBuf && iniPath) {
	bWritten = ::WritePrivateProfileString(section, panesY, itoaBuf,
					       iniPath);
    }

    return bWritten;
}

void
DSMainSplitterWnd::PrepareForDestruction()
{
}

DSMainSplitterWnd::~DSMainSplitterWnd()
{
}

////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void
DSMainSplitterWnd::OnDestroy()
{
    PrepareForDestruction();
    CSplitterWnd::OnDestroy();
}

void
DSMainSplitterWnd::OnSize(UINT nType, int cx, int cy)
{
    CSplitterWnd::OnSize(nType, cx, cy);

    // Don't resize if the controls aren't created yet, or the window
    // is being minimized.
    CWnd *cWndList = GetDlgItem(IdFromRowCol(0, 0));

    if (cWndList == NULL) {
	return;
    }

    if ((cWndList->m_hWnd == NULL) || (nType == SIZE_MINIMIZED ||
	 cx == -1 || cy == -1)) {
	return;
    }

    if (::ShouldUpdateIni() && (nType != SIZE_MINIMIZED)) {
	nCurHeight = cy;
	SerializeSerializables();
    }

    RecalcLayout();
}

int
DSMainSplitterWnd::SplitterAndBorderWidth()
{
    return(m_cxSplitter + (2 * m_cxBorder));
}

LRESULT
DSMainSplitterWnd::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_SYSCHAR:
	if (lParam & 0x20000000) {
	    if (IsThisALeftViewShortcutKey(wParam)) {
		::SendMessage(GetPane(0, 0)->m_hWnd, wMessage, wParam, lParam);
		return(0);
	    }
	    else if (IsThisARightViewShortcutKey(wParam)) {
		::SendMessage(GetPane(0, 1)->m_hWnd, wMessage, wParam, lParam);
		return(0);
	    }		
	}
	break;
     case WM_WINDOWPOSCHANGED:
 	{
	    // lPos->flags is 0x101E when the DS Window is about
	    // to be activated.
 	    LPWINDOWPOS lPos = (LPWINDOWPOS)lParam;
 	    UINT flags       = lPos->flags;
 	    UINT testFlag    = 0x101E;
 	    if (flags == testFlag) {
		::ShouldUpdateIni(0);
 	    }
 	}
	break;
    default:
	break;
    }

    return(CSplitterWnd::WindowProc(wMessage, wParam, lParam));
}


void
DSMainSplitterWnd::ResetResize()
{
    ::ShouldUpdateIni(1);
}
