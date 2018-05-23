//////////////////////////////////////////////////////////////////////////////
// DSRECtrlImpl.cpp
// 
//
// Created: 09/13/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_RE_CTRL_INTERFACE_IMPLEMENTATION_
#include "DSRECtrlImpl.h"

#include "DebugNewHelpers.h"

IMPLEMENT_DYNCREATE(DSRECtrl, C3DFormView)

BEGIN_MESSAGE_MAP(DSRECtrl, C3DFormView)
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_WM_SETCURSOR()
END_MESSAGE_MAP()

DSRECtrl::DSRECtrl()
    : C3DFormView(IDD_NDS_DIRSERV_RESULTS_LONG), h_inst(NULL),
      nCurHeight(INT_MIN)
{
    h_inst = ::LoadLibrary("RICHED32.DLL");
    ASSERT(h_inst != NULL);

    htfrl.htfrl = 0;
    htfrl.data  = 0;
    m_hIBeam = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);
    m_hArrowHour = AfxGetApp()->LoadStandardCursor(IDC_APPSTARTING);
}

DSRECtrl::~DSRECtrl()
{
    if (h_inst != NULL)
	::FreeLibrary(h_inst);
}

BOOL
DSRECtrl::SerializeSerializables()
{
    BOOL        bWritten   = FALSE;

    // Write the widht of the right pane.
    CRString    dsSect(IDS_NDS_DIRSERV_SECTION);

    char        buffer[32]  = { '\0' };
    const char *section     = dsSect;
    const char *bottomPaneY = "BottomPaneY";
    const char *itoaBuf     = ::itoa(nCurHeight, buffer, 10);
    const char *iniPath     = INIPath;

    if (section && bottomPaneY && itoaBuf && iniPath) {
	bWritten = ::WritePrivateProfileString(section, bottomPaneY, itoaBuf,
					       iniPath);
    }

    return bWritten;
    
}

BOOL
DSRECtrl::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg)
{
    if (nHitTest == HTCLIENT) {
	if (htfrl.data) {
	    ::SetCursor((DSQueryParentQueryInProgress(htfrl.data)) ?
			m_hArrowHour : m_hIBeam);
	}
	else {
	    ::SetCursor(m_hIBeam);
	}
	return(TRUE);
    }

    return(CWnd::OnSetCursor(pWnd, nHitTest, msg));
}

void
DSRECtrl::PrepareForDestruction()
{
}

void
DSRECtrl::DoDataExchange(CDataExchange *pDX)
{
    C3DFormView::DoDataExchange(pDX);

    // Results
    DDX_Control(pDX, IDD_NDS_RE_RESULTS_LONG, m_RE_ResultsLong);
}

void
DSRECtrl::OnInitialUpdate()
{
    C3DFormView::OnInitialUpdate();

    m_RE_ResultsLong.SetFont(&GetMessageFixedFont(), FALSE);

    SetScaleToFitSize(CSize(1, 1));
    RECT rect;
    GetClientRect(&rect);
    OnSize(SIZE_RESTORED, rect.right - rect.left, rect.bottom - rect.top);
}

void
DSRECtrl::OnSize(UINT nType, int cx, int cy)
{
    C3DFormView::OnSize(nType, cx, cy);

    // Don't resize if the controls aren't created yet, or the window
    // is being minimized.
    if ((m_RE_ResultsLong.m_hWnd == NULL) || (nType == SIZE_MINIMIZED ||
	 cx == -1 || cy == -1)) {
	return;
    }

    // Size the details.
    CRect rectREResultsLong;
    rectREResultsLong.TopLeft().x = 0;
    rectREResultsLong.TopLeft().y = 0;
    rectREResultsLong.BottomRight().x = cx;
    rectREResultsLong.BottomRight().y = cy;
    m_RE_ResultsLong.MoveWindow(rectREResultsLong);

    if (::ShouldUpdateIni() && (nType != SIZE_MINIMIZED)) {
	nCurHeight = cy;
	SerializeSerializables();
    }
}

////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void
DSRECtrl::OnDestroy()
{
    PrepareForDestruction();
    ClearContents();
    C3DFormView::OnDestroy();
}

void
DSRECtrl::ClearContents()
{
    m_RE_ResultsLong.SetSel(0, -1);
    m_RE_ResultsLong.ReplaceSel("");
    contentCleared = true;
}

void
DSRECtrl::InitializeHandleTab(HTFRL htfrl_, void *data)
{
    htfrl.htfrl = htfrl_;
    htfrl.data  = data;
}

void
DSRECtrl::AddStringNoNewLine(char *data)
{
    m_RE_ResultsLong.SetSel(-1, -1);
    m_RE_ResultsLong.ReplaceSel(data);
}

void
DSRECtrl::NewRecord()
{
    if (contentCleared) {
	contentCleared = false;
    }
    else {
	CRString phDivider(IDS_PH_DIVIDER);
	m_RE_ResultsLong.SetSel(-1, -1);
	m_RE_ResultsLong.ReplaceSel(phDivider);
    }
}

LRESULT
DSRECtrl::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_GETDLGCODE:
	return(DLGC_WANTALLKEYS);
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

    return(C3DFormView::WindowProc(wMessage, wParam, lParam));
} 
