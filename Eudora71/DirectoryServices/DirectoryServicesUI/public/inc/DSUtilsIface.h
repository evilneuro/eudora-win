///////////////////////////////////////////////////////////////////////////////
// DSUtilsIface.h
//
// Created: 09/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_UTILS_IFACE_H_
#define __DS_UTILS_IFACE_H_

#include "DSBaseTypes.h"

#define NO_OF_SHORTCUTS 9

enum DSSHORTCUTS 
{
    SC_LV_QUERYINPUT,
    SC_LV_STARTSTOP,
    SC_LV_TO,
    SC_LV_CC,
    SC_LV_BCC,
    SC_LV_KEEPONTOP,
	SC_RV_NEWDATABASE,
    SC_RV_MODIFYDATABASE,
    SC_RV_DELETEDATABASE
};

LPARAM GetListItemLPARAM(void *pv, int nItem);
LPARAM GetListFocusedItemLPARAM(void *pv);
BOOL LookupQuery(const char* Server, const char* Command, BOOL IsPh);
bool ShouldPropagateMessageToParent(UINT wMessage, WPARAM wParam,
				    LPARAM lParam);
bool IsThisAShortcutKeyWeWant(WPARAM wParam);
bool IsThisALeftViewShortcutKey(WPARAM wParam);
bool IsThisARightViewShortcutKey(WPARAM wParam);
unsigned char GetDlgShortcutChar(CWnd*, unsigned long nCtlId);

#endif // __DS_UTILS_IFACE_H_


