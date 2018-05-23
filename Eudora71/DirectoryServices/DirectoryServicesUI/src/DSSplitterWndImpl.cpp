//////////////////////////////////////////////////////////////////////////////
// DSSplitterWndImpl.cpp
// 
//
// Created: 09/13/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_SPLITTER_WND_INTERFACE_IMPLEMENTATION_
#include "DSSplitterWndImpl.h"

IMPLEMENT_DYNCREATE(DSSplitterWnd, CSplitterWnd)

BEGIN_MESSAGE_MAP(DSSplitterWnd, CSplitterWnd)
    ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

DSSplitterWnd::DSSplitterWnd()
    : nCurWidth(INT_MIN)
{
}

DSSplitterWnd::~DSSplitterWnd()
{
}

void
DSSplitterWnd::PrepareForDestruction()
{
}

BOOL
DSSplitterWnd::SerializeSerializables()
{
    BOOL        bWritten   = FALSE;

    // Write the widht of the right pane.
    CRString    dsSect(IDS_NDS_DIRSERV_SECTION);

    char        buffer[32] = { '\0' };
    const char *section    = dsSect;
    const char *panesX     = "PanesX";
    const char *itoaBuf    = ::itoa(nCurWidth, buffer, 10);
    const char *iniPath    = INIPath;

    if (section && panesX && itoaBuf && iniPath) {
	bWritten = ::WritePrivateProfileString(section, panesX, itoaBuf,
					       iniPath);
    }

    return bWritten;

}

////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void
DSSplitterWnd::OnDestroy()
{
    PrepareForDestruction();
    CSplitterWnd::OnDestroy();
}

void
DSSplitterWnd::OnSize(UINT nType, int cx, int cy)
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
	nCurWidth = cx;
	SerializeSerializables();
    }

    RecalcLayout();
}

int
DSSplitterWnd::SplitterAndBorderHeight()
{
    return(m_cySplitter + (2 * m_cyBorder));
}

LRESULT
DSSplitterWnd::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_CHAR:
    case WM_SYSCHAR:
	if (ShouldPropagateMessageToParent(wMessage, wParam, lParam)) {
	    ::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
	    return(0);
	}
	break;
    default:
	break;
    }

    return(CSplitterWnd::WindowProc(wMessage, wParam, lParam));
} 
