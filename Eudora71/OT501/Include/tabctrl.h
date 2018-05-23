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
//  Description:  Declarations for SECTabControl
//

#ifndef __TABCTRL_H__
#define __TABCTRL_H__

#ifndef __TABCTRLB_H__
#include "tabctrlb.h"
#endif //__TABCTRLB_H__

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
// SECTabControl window

// Tab Control Styles
#define TCS_LEFTRIGHTSCROLL	0x0001
#define TCS_FULLSCROLL		0x0002

// Tab Control Button Masks
#define TCBM_NONE		0x00
#define TCBM_FIRST		0x01
#define TCBM_LEFT		0x02
#define TCBM_RIGHT		0x04
#define TCBM_LAST		0x08

// Array indexes for GDI objects
#define SECTAB_UNSELECTED		0
#define SECTAB_SELECTED			1
#define SECTAB_ACTIVE			2

class SECTabControl : public SECTabControlBase
{
    DECLARE_DYNAMIC(SECTabControl);

// Construction
public:
    SECTabControl();
    ~SECTabControl();

public:

// Operations
    void ScrollToTab(int nTab);

	// Font support
	BOOL SetFontSelectedTab(CFont* pFont,BOOL bRedraw=TRUE)
		{ return SetFontTab(pFont,SECTAB_SELECTED,bRedraw); }
	BOOL SetFontUnselectedTab(CFont* pFont,BOOL bRedraw=TRUE) 	
		{ return SetFontTab(pFont,SECTAB_UNSELECTED,bRedraw); }
	BOOL SetFontActiveTab(CFont* pFont,BOOL bRedraw=TRUE) 	
		{ return SetFontTab(pFont,SECTAB_ACTIVE,bRedraw); }
	CFont* GetFontSelectedTab() 			{ return &m_fonts[SECTAB_SELECTED]; }
	CFont* GetFontUnselectedTab() 			{ return &m_fonts[SECTAB_UNSELECTED]; }
	CFont* GetFontActiveTab() 				{ return &m_fonts[SECTAB_ACTIVE]; }

// Overridables
    virtual void Initialize(DWORD dwStyle);
    virtual void DrawTab(CDC *pDC, int nTab, CRgn &tabRgn);
	virtual void InvalidateTab(int nTab);
	virtual void RecalcLayout();

// Queries
	BOOL TabHit(int nTab, CPoint& point) const;

// Implementation
protected:
	virtual BOOL SetFontTab(CFont* pFont,int nTabType=SECTAB_ACTIVE,BOOL bRedraw=TRUE);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    int		m_nScrollOffset;
    int		m_nHeight;

    // GDI Objects
    CFont	m_fonts[3];
    CPen	m_pens[3];
    CPen	m_penShadow;
    CBrush	m_brushes[3];
    COLORREF	m_rgbText[3];
        
    // Button attributes
    int		m_nButtons, m_nButtonPressed, m_nDesensedButtons;
    CSize	m_sizeBtn;
    
    void rgbFill(CDC* pDC,int x,int y,int cx,int cy, COLORREF rgb);
    void DrawButton(CDC *pDC, int btn);
    void GetTabRgn(int nTab, CRgn &tabRgn) const;
    void GetTabsRect(CRect& tabArea) const;
    void GetButtonsRect(CRect& btnArea);
    void GetButtonRect(int btn, CRect& rect);
    void RecalcButtonSensitivity();
    void OnButtonPress(int nButtonCode);
	int GetLabelWidth(int nTab);
    
    // Generated message map functions
protected:
    //{{AFX_MSG(SECTabControl)
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
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

#endif // __TABCTRL_H__
/////////////////////////////////////////////////////////////////////////////
