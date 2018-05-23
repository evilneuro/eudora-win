// bmpcombo.h
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
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
DAMAGE. */

//

#ifndef _BMPCOMBO_H_
#define _BMPCOMBO_H_

#include "font.h"

class CBitmapComboItem
{
public:
	CBitmapComboItem(UINT BitmapID, UINT StringID, int ID = 0);
	CBitmapComboItem(UINT BitmapID, const char* String, int ID = 0);
	
	CString m_Text;
	CBitmap m_Bitmap;
	int m_BitmapWidth, m_BitmapHeight;
	int m_ID;

protected:
	void LoadBitmap(UINT BitmapID);
};

/////////////////////////////////////////////////////////////////////////////
// CBitmapCombo window

class CBitmapCombo : public CComboBox
{
	DECLARE_DYNCREATE(CBitmapCombo)

	CWnd*	m_pLastFocus;

// Construction
public:
	CBitmapCombo();

// Attributes
public:
	int m_Width, m_Height, m_ItemHeight;
	UINT	m_EditWidth;

// Operations
public:

// Implementation
public:
	virtual ~CBitmapCombo();
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	int Add(CBitmapComboItem* Item);
	CBitmapComboItem* GetItemDataPtr(int nIndex) const
		{ return ((CBitmapComboItem*)CComboBox::GetItemDataPtr(nIndex)); }

	void RestoreFocus();
	void SetEditWidth( INT iWidth ) { m_EditWidth = iWidth; }

protected:
	// Generated message map functions
	//{{AFX_MSG(CBitmapCombo)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	virtual void PostNcDestroy();

protected:
	static CFontInfo m_Font;
	static int m_Count;
	enum {SPACE = 3};
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CPriorityCombo window

class CPriorityCombo : public CBitmapCombo
{
// Construction
public:
	CPriorityCombo();

// Attributes
public:

// Operations
public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Implementation
public:
	~CPriorityCombo();

protected:
	// Generated message map functions
	//{{AFX_MSG(CPriorityCombo)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif	// _BMPCOMBO_H_
