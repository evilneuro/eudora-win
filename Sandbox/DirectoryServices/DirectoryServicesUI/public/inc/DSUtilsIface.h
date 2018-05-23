///////////////////////////////////////////////////////////////////////////////
// DSUtilsIface.h
//
// Created: 09/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_UTILS_IFACE_H_
#define __DS_UTILS_IFACE_H_

#include "DirectoryServicesBaseTypes.h"

LPARAM GetListItemLPARAM(void *pv, int nItem);
LPARAM GetListFocusedItemLPARAM(void *pv);
BOOL LookupQuery(const char* Server, const char* Command, BOOL IsPh);
bool ShouldPropagateMessageToParent(UINT wMessage, WPARAM wParam,
				    LPARAM lParam);
bool IsThisAShortcutKeyWeWant(WPARAM wParam);
bool IsThisALeftViewShortcutKey(WPARAM wParam);
bool IsThisARightViewShortcutKey(WPARAM wParam);

#endif // __DS_UTILS_IFACE_H_


