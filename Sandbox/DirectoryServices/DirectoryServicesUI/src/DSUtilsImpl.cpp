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
    LPARAM lParam  = 0;
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

    switch((BYTE) wParam) {
    case 'q': // Query Input EditBox.
    case 's': // Start/Stop button.
    case 't': // To button.
    case 'c': // Cc button.
    case 'b': // Bcc button.
    case 'k': // Keep On Top check box.
	retVal = true;
	break;
    default:
	break;
    }

    return(retVal);
}

bool IsThisARightViewShortcutKey(WPARAM wParam)
{
    bool retVal = false;

    switch((BYTE) wParam) {
    case 'n': // New Database button.
    case 'm': // Modify Database button.
    case 'd': // Delete Database button.
	retVal = true;
	break;
    default:
	break;
    }

    return(retVal);
}
