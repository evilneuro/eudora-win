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
//  Author:       Dean Hallman
//  Description:  Declarations for SEC3DTab and SEC3DTabControl
//  Created:      July 20, 1996
//
//	Modified:	  Feb. 1997 (Kim Starks)
//	Description:  Added tab disable feature			
//	

#ifndef __TABCTRL3_H__
#define __TABCTRL3_H__

#ifndef __TABCTRLB_H__
#include "tabctrlb.h"
#endif

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


// Array indexes for GDI objects
#define SEC3DTAB_INACTIVE	0
#define SEC3DTAB_ACTIVE		1

/////////////////////////////////////////////////////////////////////////////
// SEC3DTab

class SEC3DTab : public SECTab
{
    DECLARE_DYNAMIC(SEC3DTab);

public:
    SEC3DTab();

// Attributes
public:
	CRect m_rect;
	HICON m_hIcon;
	BOOL m_bEnabled;

// Implementation
protected:
    ~SEC3DTab();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};


/////////////////////////////////////////////////////////////////////////////
// SEC3DTabControl window

class SEC3DTabControl : public SECTabControlBase
{
    DECLARE_DYNAMIC(SEC3DTabControl);

// Construction
public:
    SEC3DTabControl();
    ~SEC3DTabControl();

// Overrides
    virtual void Initialize(DWORD dwStyle);
	virtual SECTab* CreateNewTab() const;
	virtual void RecalcLayout();
	virtual void InitializeFonts(DWORD dwStyle);
	virtual void OnActivateTab(int nTab);
	virtual void DestroyGDIObjects();

#ifdef WIN32
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
#endif

// Operations

	// Font support
	BOOL SetFontActiveTab(CFont* pFont,BOOL bRedraw=TRUE)
		{ return SetFontTab(pFont,SEC3DTAB_ACTIVE,bRedraw); }
	BOOL SetFontInactiveTab(CFont* pFont,BOOL bRedraw=TRUE) 	
		{ return SetFontTab(pFont,SEC3DTAB_INACTIVE,bRedraw); }
	CFont* GetFontActiveTab() 				{ return &m_fonts[SEC3DTAB_ACTIVE]; }
	CFont* GetFontInactiveTab() 			{ return &m_fonts[SEC3DTAB_INACTIVE]; }

	virtual void SetTabIcon(int nIndex, HICON hIcon);
	virtual void SetTabIcon(int nIndex, UINT nIDIcon);
	virtual void SetTabIcon(int nIndex, LPCTSTR lpszResourceName);
	virtual void InvalidateTab(int nTab);
	virtual void EnableTab(int nIndex, BOOL bEnable=TRUE);
	virtual BOOL IsTabEnabled(int nIndex);

#ifdef WIN32
	virtual BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
#endif

// Implementation
protected:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

	// GDI Objects
    CFont m_fonts[2];
    CPen m_pens[2];
    CPen m_penShadow;
    CPen m_penWhite;
    CBrush m_brushes[2];
    COLORREF m_rgbText[2];
	int m_bShowTabLabels;	// show labels since tabs all tabs fit
#ifdef WIN32
	CToolTipCtrl* m_pToolTip;
#endif
	virtual BOOL SetFontTab(CFont* pFont,int nTabType=SEC3DTAB_ACTIVE,BOOL bRedraw=TRUE);

// Queries
	BOOL TabHit(int nTab, CPoint& point) const;
    SEC3DTab& GetTab(int nTab);
	SEC3DTab* GetTabPtr(int nTab) const;
	virtual void GetTabRect(int nTab, CRect &tabRect) const;
	int GetLabelWidth(int nTab);

// Layout
	void SetTabPos(int nTab, int x, int y, DWORD dwStyle);

// Drawing
	virtual void DrawTab(CDC* pDC, int nTab);
    virtual void rgbFill(CDC* pDC,int x,int y,int cx,int cy, COLORREF rgb);
	virtual void DrawTabBottom(CDC* pDC, SEC3DTab* pTab, DWORD dwStyle);
	virtual void DrawTabTop(CDC* pDC, SEC3DTab* pTab, DWORD dwStyle);
	virtual void DrawTabLeft(CDC* pDC, SEC3DTab* pTab, DWORD dwStyle);
	virtual void DrawTabRight(CDC* pDC, SEC3DTab* pTab, DWORD dwStyle);
	virtual void DrawInterior(CDC* pDC, SEC3DTab* pTab, DWORD dwStyle);
	virtual void DrawDisabled(CDC* pDC, int nWidth, int nHeight);

    // Generated message map functions
protected:
    //{{AFX_MSG(SEC3DTabControl)
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TABCTRL3_H__
/////////////////////////////////////////////////////////////////////////////
