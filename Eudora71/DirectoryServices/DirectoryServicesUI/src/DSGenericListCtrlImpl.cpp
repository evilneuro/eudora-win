//////////////////////////////////////////////////////////////////////////////
// DSGenericListCtrlImpl.cpp
// 
//
// Created: 10/25/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_GENERIC_LIST_CTRL_INTERFACE_IMPLEMENTATION_
#include "DSGenericListCtrlImpl.h"

IMPLEMENT_DYNCREATE(DSGenericListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(DSGenericListCtrl, CListCtrl)
END_MESSAGE_MAP()

BOOL
DSGenericListCtrl::PreTranslateMessage(MSG *pMsg)
{
    BOOL retVal = FALSE;
    ASSERT_VALID(this);

    switch(pMsg->message) {
    case WM_SYSCHAR:
	// A shortcut key.
	if (pMsg->lParam & 0x20000000) {
	    // Do we want this shortcut.
	    if (IsThisAShortcutKeyWeWant(pMsg->wParam)) {
		::SendMessage(GetParent()->m_hWnd, pMsg->message,
			      pMsg->wParam, pMsg->lParam);
		retVal = TRUE;
	    }
	}
	break;
    default:
	break;
    }

    return(retVal);
}

LRESULT
DSGenericListCtrl::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_GETDLGCODE:
	return(DLGC_WANTALLKEYS);
    case WM_CHAR:    // We are interested only in Tab key here.
	if (ShouldPropagateMessageToParent(wMessage, wParam, lParam)) {
	   ::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
	   return(0);
	}
	break;
    case WM_KEYUP:
	::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
	break;
    default:
	break;
    }
    return(CListCtrl::WindowProc(wMessage, wParam, lParam));
} 
