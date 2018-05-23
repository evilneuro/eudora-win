/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File:           lviewex.h
Description:    Owner-draw extended selection ListView control
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
Revisions:      
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef __LVIEWEX_H__
#define __LVIEWEX_H__


// Style flags for CListViewEx
#define LVX_EXTENDEDSELECTION     0x00000001

class CListViewEx
{
public:
  CListViewEx(HWND hwndListView, DWORD dwStyle=LVX_EXTENDEDSELECTION);

public:
  BOOL    HandleMessage(HWND hwndParent, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult);

protected:
  BOOL    OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam, LRESULT &rlResult);
  void    DrawItem(LPDRAWITEMSTRUCT lpDrawItem);
  void    DrawItemColumn(HDC hdc, LPRECT prcClip);
  BOOL    CalcStringEllipsis(HDC hdc, UINT uColWidth);

protected:
  HWND    m_hWnd;
  DWORD   m_dwStyle;
  TCHAR   m_szText[256];
  TCHAR   m_szScratch[256];
};


///////////////////////////////////////////////////////////////////////////////
// Example usage:

#if 0
LRESULT ParentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // this could be a class data member also...
  static CListViewEx * plv = NULL;
  
  if (plv && plv->HandleMessage(hwnd, uMsg, wParam, lParam, &lResult))
    return lResult;

  switch(uMsg) {
    case WM_CREATE:
      // create the list view and attach it to object...
      hwndListView = CreateListView();
      plv = new CListViewEx(hwndListView);
      fBaptizeBlockMT(plv, "::ParentWndProc");
      break;

    case WM_DESTROY:
      // destroy the list view object...
      if (plv) {
        delete plv;
        plv = NULL;
      }
      break;

    // rest of WndProc...
  }
}

#endif
///////////////////////////////////////////////////////////////////////////////



#endif  //__LVIEWEX_H__

