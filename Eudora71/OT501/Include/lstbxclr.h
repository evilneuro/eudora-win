// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:			John Williston
//  Description:	Declaration for SECColorsListBox
//  Created:		August 1996
//

#ifndef __LSTBXCLR_H__
#define __LSTBXCLR_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

/////////////////////////////////////////////////////////////////////////////
// SECColorsListBox window

class SECColorsListBox : public CListBox
{
// Construction
public:
	SECColorsListBox(BOOL bAllowDblClickEdit = TRUE);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECColorsListBox)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECColorsListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(SECColorsListBox)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	BOOL ReplaceListbox(CListBox* pWnd);
	int AddColor(const TCHAR* pcszName, COLORREF crColor);
	BOOL GetColor(int iIndex, CString& str, COLORREF& cr);
	BOOL SetColor(int iIndex, const TCHAR* pcszName, COLORREF cr);
	BOOL EditSelectedColor();
private:
	BOOL m_bAllowDblClickEdit;
	void FastRect(CDC *pDC, CRect* pRect, COLORREF cr);
};

/////////////////////////////////////////////////////////////////////////////

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif	// __LSTBXCLR_H__
