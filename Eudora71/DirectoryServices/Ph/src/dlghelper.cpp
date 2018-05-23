// dlghelper.cpp
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <commctrl.h>
#include "DebugNewHelpers.h"
#include "dlghelper.h"
#include "resource.h"
#include "ustring.h"


///////////////////////////////////////////////////////////////////////////////
// Dialog helper functions


// (public)
// Resizes the last column in a ListView control line up exactly with the 
// right edge of the window. No horizontal scroll bar will be displayed 
// unless the start of the last column exceeds the width of the window.
BOOL ListView_AutoResizeLastColumn(HWND hwndListView)
{
  int nColumns = 0;
  int nTotalWidth = 0;
  int nLastColumnStart = 0;
  int nScrollWidth = 0;

  // get number of columns and position of start of last column
  LV_COLUMN lvc;
  lvc.mask = LVCF_WIDTH;
  for (nColumns=0; ListView_GetColumn(hwndListView, nColumns, &lvc); nColumns++)
    nTotalWidth += lvc.cx;
  nLastColumnStart = nTotalWidth - lvc.cx;  // subtract out the last column width

  // get the number of items per page
  int nPageCount = ListView_GetCountPerPage(hwndListView);
  // get the total number of items in list view
  int nItems = ListView_GetItemCount(hwndListView);

  if (nItems > nPageCount)
    nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);

  RECT rc;
  GetWindowRect(hwndListView, &rc);
  int nWidth = rc.right - rc.left - nScrollWidth - 4;

  if (nLastColumnStart < nWidth) {
    ListView_SetColumnWidth(hwndListView, nColumns-1, nWidth-nLastColumnStart);
    return TRUE;
  }

  return FALSE;
}





BOOL CheckEditFieldEmpty(HWND hwndDlg, UINT nIDEdit, UINT nResStringMsg)
{
	HWND hEdit = GetDlgItem(hwndDlg, nIDEdit);
	CUString str(hEdit);
	if (!strlen(str)) {
		CUString strError(nResStringMsg);
		CUString strTitle(IDS_ERRORINFIELD);
		MessageBox(hwndDlg, strError, strTitle, MB_OK);
		SetFocus(hEdit);
		return 1;
	}
	return 0;
}



//In some cases we may need to ignore the min or max.

BOOL CheckEditFieldInt(HWND hwndDlg, UINT nIDEdit, int nMin, int nMax, IGNOREBOUNDS ignore)
{
	BOOL bTrans;
	int nVal = (int)GetDlgItemInt(hwndDlg, nIDEdit, &bTrans, TRUE);

    bool bMaxOK = (ignore == IGNOREBOTH ) || (ignore == IGNOREMAX) || (nVal <= nMax);
	bool bMinOK = (ignore == IGNOREBOTH ) || (ignore == IGNOREMIN) || (nVal >= nMin);


	if (!bTrans || !bMinOK || !bMaxOK) {
		UINT nError = IDS_INTEGERERR;
		int nBound = nMax;

		if (ignore == IGNOREMAX)
			{
		     nError = IDS_INTEGERERRMIN;
			 nBound = nMin;
			}
		else if (ignore == IGNOREMIN)
			{
			nError = IDS_INTEGERERRMAX;
			nBound = nMax;
			}
        
		CUString strError(nError);
		CUString strTitle(IDS_ERRORINFIELD);
		char buffer[100];
		
		if (ignore == IGNORENONE)
			{
			wsprintf(buffer, strError, nMin, nMax);
			}
		else
			{
			wsprintf(buffer, strError, nBound);
			}

		MessageBox(hwndDlg, buffer, strTitle, MB_OK);
		SetFocus(GetDlgItem(hwndDlg, nIDEdit));
		return 1;
	}
	return 0;
}

BOOL SetListViewEntry(HWND hwndList, LPSTR pszAttribName, 
														LPSTR pszPresentName, int nType)
{
	LV_ITEM	lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(hwndList);
	lvi.iSubItem = 0;
	lvi.pszText = pszAttribName;
	lvi.lParam = (LPARAM)nType;
	int iIndex = ListView_InsertItem(hwndList, &lvi);
	ListView_SetItemText(hwndList, iIndex, 1, pszPresentName);
	return TRUE;
}


BOOL GetListViewEntry(HWND hwndList, int iIndex, LPSTR pszAttribName, int cbAttribName,
														 LPSTR pszPresentName, int cbPresentName)
{
	ListView_GetItemText(hwndList, iIndex, 0, pszAttribName, cbAttribName);
	ListView_GetItemText(hwndList, iIndex, 1, pszPresentName, cbPresentName);
	return TRUE;
}


LPARAM ListView_GetItemData(HWND hwndList, int iItem)
{
	LV_ITEM	lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	if (!ListView_GetItem(hwndList, &lvi))
		return 0;

	return lvi.lParam;
}


int ListView_GetFocusedItem(HWND hwndList, LPARAM * plParam/*=NULL*/)
{
	SetFocus(hwndList);
	int iItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL|LVNI_FOCUSED);
	if (iItem >= 0 && plParam)
		*plParam = ListView_GetItemData(hwndList, iItem);
	return iItem;
}


void ListView_EditFocused(HWND hwndList)
{
	int iIndex = ListView_GetFocusedItem(hwndList);
	ListView_EditLabel(hwndList, iIndex);
}


void SetListViewColumn(HWND hwndList, int nColumn, UINT nIDResString, int nWidth)
{
	LV_COLUMN	lvc = {0};
	
	CUString str(nIDResString);			// load the resource string
	lvc.mask			= LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.fmt				= LVCFMT_LEFT;
	lvc.pszText		= str;
	lvc.cx				= nWidth;
	lvc.iSubItem	= nColumn;
	ListView_InsertColumn(hwndList, nColumn, &lvc);
}



