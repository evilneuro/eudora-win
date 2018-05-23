/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File:           lviewex.cpp
Description:    Subclassed extended selection ListView control
Date:           9/11/97
Version:        1.0 
Notice:         Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Portions of this code were included (and modified) from ODLISTVW sample 
  distributed by Microsoft Corporation.

  Portions Copyright (C) 1993-1995  Microsoft Corporation.  All Rights Reserved.

  Microsoft Corporation's disclaimer notice:
    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <commctrl.h>
#include <assert.h>
#include "DebugNewHelpers.h"
#include "lviewex.h"


// local function prototypes
static void DrawItem(LPDRAWITEMSTRUCT lpDrawItem);
static BOOL CalcStringEllipsis(HDC hdc, LPTSTR lpszString, int cchMax, UINT uColWidth);


///////////////////////////////////////////////////////////////////////////////

CListViewEx::CListViewEx(HWND hwndListView, DWORD dwStyle)
{
  assert(IsWindow(hwndListView));
  m_hWnd = hwndListView;
  m_dwStyle = dwStyle;

  // force the LVS_OWNERDRAWFIXED style in the listview control
  // otherwise, there is no point to our existence
  DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
  style |= LVS_OWNERDRAWFIXED;
  SetWindowLong(m_hWnd, GWL_STYLE, style);
}


BOOL CListViewEx::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &rlResult)
{
  rlResult = 0;
  switch(uMsg) {
    case WM_DRAWITEM:
      return OnDrawItem(hwnd, wParam, lParam, rlResult);
  }
  return FALSE;
}


// WM_DRAWITEM handler
BOOL CListViewEx::OnDrawItem(HWND, WPARAM, LPARAM lParam, LRESULT &rlResult)
{
  LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
  
  // Make sure the control is from our listview control
  if (lpDrawItem->CtlType != ODT_LISTVIEW || 
      lpDrawItem->hwndItem != m_hWnd)
    return FALSE;

  // we are handling this message
  rlResult = 1;
  DrawItem(lpDrawItem);
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  DrawItem
//
//  Draws one item in the listview control.
//
//  lpDrawItem    Pointer to the information needed to draw the item.  The
//                item number is in the itemID member.
//
void CListViewEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
  HIMAGELIST  himl;
  LV_ITEM     lvi;
  int         cxImage = 0, cyImage = 0;
  RECT        rcClip;
  UINT        uiFlags = ILD_TRANSPARENT;

  // Get the item image to be displayed
  lvi.mask = LVIF_IMAGE | LVIF_STATE;
  lvi.iItem = lpDrawItem->itemID;
  lvi.iSubItem = 0;
  ListView_GetItem(lpDrawItem->hwndItem, &lvi);
  
  // Check to see if this item is selected
  if (lpDrawItem->itemState & ODS_SELECTED) {
    // Set the text background and foreground colors
    SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHT));
    // Also add the ILD_BLEND50 so the images come out selected
    uiFlags |= ILD_BLEND50;
  }
  else {
    // Set the text bkgnd and foregnd colors to the standard window colors
    SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
  }
  
  // Get the image list and draw the image
  himl = ListView_GetImageList(lpDrawItem->hwndItem, LVSIL_SMALL);
  if (himl) {
    ImageList_Draw(himl, lvi.iImage, lpDrawItem->hDC,
                   lpDrawItem->rcItem.left, lpDrawItem->rcItem.top,
                   uiFlags);
    // Find out how big the image we just drew was
    ImageList_GetIconSize(himl, &cxImage, &cyImage);
  }
  
  // set up the clip rect for column text drawing
  rcClip.top    = lpDrawItem->rcItem.top;
  rcClip.bottom = lpDrawItem->rcItem.bottom;
  
  rcClip.left   = lpDrawItem->rcItem.left + cxImage;
  rcClip.right  = lpDrawItem->rcItem.left;
  
  // do for all of the columns...
  LV_COLUMN lvc;
  lvc.mask = LVCF_WIDTH;
  for (int i=0; ListView_GetColumn(lpDrawItem->hwndItem, i, &lvc); i++) {
    // Set up the new clipping rect for the column text and draw it
    if (i != 0)
      rcClip.left = rcClip.right;
    rcClip.right += lvc.cx;
  
    // get the column text
    ListView_GetItemText(lpDrawItem->hwndItem, lpDrawItem->itemID, i, m_szText, sizeof(m_szText));
    // draw the colum text
    DrawItemColumn(lpDrawItem->hDC, &rcClip);
  }
  
  // If we changed the colors for the selected item, undo it
  if (lpDrawItem->itemState & ODS_SELECTED) {
    // Set the text background and foreground colors
    SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
  }
  
  // If the item is focused, now draw a focus rect around the entire row
  if (lpDrawItem->itemState & ODS_FOCUS) {
    // Adjust the left edge to exclude the image
    rcClip = lpDrawItem->rcItem;
    rcClip.left += cxImage;
    // Draw the focus rect
    DrawFocusRect(lpDrawItem->hDC, &rcClip);
  }
}


///////////////////////////////////////////////////////////////////////////////
//  DrawItemColumn
//
//  Draws the text for one of the columns in the list view.
//
//  hdc       Handle of the DC to draw the text into.
//  lpsz      String to draw.
//  prcClip   Rectangle to clip the string to.
//
void CListViewEx::DrawItemColumn(HDC hdc, LPRECT prcClip)
{
  CalcStringEllipsis(hdc, prcClip->right - prcClip->left);
  // print the text
  ExtTextOut(hdc, prcClip->left + 2, prcClip->top + 1, ETO_CLIPPED | ETO_OPAQUE,
             prcClip, m_szText, lstrlen(m_szText), NULL);
}


///////////////////////////////////////////////////////////////////////////////
//  CalcStringEllipsis
//
//  Determines whether the specified string is too wide to fit in
//  an allotted space, and if not truncates the string and adds some
//  points of ellipsis to the end of the string.
//
BOOL CListViewEx::CalcStringEllipsis(HDC hdc, UINT uColWidth)
{
  const TCHAR szEllipsis[] = TEXT("...");
  
  // Adjust the column width to take into account the edges
  uColWidth -= 4;
  
  strcpy(m_szScratch, m_szText);

  SIZE sizeString;
  SIZE sizeEllipsis;
  int cbString = lstrlen(m_szScratch);
  // Get the width of the string in pixels
  if (!GetTextExtentPoint32(hdc, m_szScratch, cbString, &sizeString))
    return FALSE;

  // If the width of the string is less than the column width we are done
  if ((ULONG)sizeString.cx <= uColWidth)
    return TRUE;

  if (!GetTextExtentPoint32(hdc, szEllipsis, lstrlen(szEllipsis), &sizeEllipsis))
    return FALSE;

  // keep truncating the string until the string + ellipsis fits the column width
  while (cbString > 1) {
    m_szScratch[--cbString] = 0;
    if (!GetTextExtentPoint32(hdc, m_szScratch, cbString, &sizeString))
      return FALSE;
    if ((ULONG)(sizeString.cx + sizeEllipsis.cx) <= uColWidth)
      break;    // The string with the ellipsis finally fits, break out of loop
  }

  lstrcat(m_szScratch, szEllipsis);
  lstrcpy(m_szText, m_szScratch);
  return TRUE;
}

