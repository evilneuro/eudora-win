//////////////////////////////////////////////////////////////////////////////
// DSUtilsImpl.cpp
// 
//
// Created: 09/13/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_UTILS_INTERFACE_IMPLEMENTATION_
#include "DSUtilsImpl.h"

unsigned char DSSHORTCUT[NO_OF_SHORTCUTS];



LPARAM
GetListItemLPARAM(void *pv, int nItem)
{
    LV_ITEM item;

    memset(&item, 0, sizeof(LV_ITEM));

    item.mask = LVIF_PARAM;
    item.iItem = nItem;
    if ((static_cast<CListCtrl *>(pv))->GetItem(&item) == TRUE) {
	return(item.lParam);
    }
    return(0);
}

LPARAM
GetListFocusedItemLPARAM(void *pv)
{
    int idx        = -1;
    CListCtrl *pLC = static_cast<CListCtrl *>(pv);

    if ((idx = pLC->GetNextItem(-1, LVNI_FOCUSED)) != -1) {
	return(GetListItemLPARAM(pv, idx));
    }

    return(0);
}

BOOL LookupQuery(const char*, const char*, BOOL)
{
    return(FALSE);
}

bool
ShouldPropagateMessageToParent(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    bool retVal = false;

    switch(wMessage) {
    case WM_SYSCHAR:
	if (lParam & 0x20000000) {
	    if (IsThisAShortcutKeyWeWant(wParam)) {
		retVal = true;
	    }
	}
	break;
    case WM_CHAR:
	if (wParam == VK_TAB) { // Tab traversal.
	    retVal = true;
	}
	break;
    default:
	break;
    }

    return(retVal);
}

bool
IsThisAShortcutKeyWeWant(WPARAM wParam)
{
    bool retVal = false;

    if (IsThisALeftViewShortcutKey(wParam) ||
	IsThisARightViewShortcutKey(wParam)) {
	retVal = true;
    }

    return(retVal);
}

bool IsThisALeftViewShortcutKey(WPARAM wParam)
{
    bool retVal = false;
    unsigned char sc = (BYTE) wParam;

    retVal = ((sc == DSSHORTCUT[SC_LV_QUERYINPUT]) ||// Query Input EditBox.
             (sc == DSSHORTCUT[SC_LV_STARTSTOP])   || // Start/Stop button.
             (sc == DSSHORTCUT[SC_LV_TO])          ||// To button.
             (sc == DSSHORTCUT[SC_LV_CC])          || // Cc button.
             (sc == DSSHORTCUT[SC_LV_BCC])         || // Bcc button.
             (sc == DSSHORTCUT[SC_LV_KEEPONTOP]));// Keep On Top check box.

    return(retVal);
}

bool IsThisARightViewShortcutKey(WPARAM wParam)
{
    bool retVal = false;
    unsigned char sc = (BYTE) wParam;
    retVal = ((sc == DSSHORTCUT[SC_RV_NEWDATABASE])     || // New Database button.
              (sc == DSSHORTCUT[SC_RV_MODIFYDATABASE])  ||// Modify Database button.
              (sc == DSSHORTCUT[SC_RV_DELETEDATABASE])); // Delete Database button.
    return(retVal);
}

//this function is called only once when the directory services window is opened
unsigned char GetDlgShortcutChar(CWnd* pWnd, unsigned long nCtlId)
{
	if(!pWnd)
		return 0;

    CString csText;
	unsigned char sc =0;
	pWnd->GetDlgItemText(nCtlId, csText);
	csText.MakeLower();

	int amppos = csText.Find('&');
	if (amppos >= 0 && (amppos < csText.GetLength()-1))
	{
		sc = csText[amppos +1];
	}

	return sc ;
}
