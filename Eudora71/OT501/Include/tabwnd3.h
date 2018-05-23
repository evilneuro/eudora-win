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
//  Author:      Dean Hallman
//  Description: SEC3DTabWnd declaration
//  Created:     July 20, 1996
//
//	Modified:	  Feb. 1997 (Kim Starks)
//	Description:  Added tab disable feature			
//	

#ifndef __TABWND3_H__
#define __TABWND3_H__

#ifndef __TABCTRL3_H__
#include "tabctrl3.h"
#endif

#ifndef __TABWNDB_H__
#include "tabwndb.h"
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

class SEC3DTabWnd : public SECTabWndBase
{
    DECLARE_DYNCREATE(SEC3DTabWnd)
    
// Construction
public:
    SEC3DTabWnd();
    virtual ~SEC3DTabWnd();

    BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | TWS_TABS_ON_BOTTOM,
		UINT nID = AFX_IDW_PANE_FIRST);

protected:    
	virtual BOOL CreateTabCtrl(DWORD dwStyle, UINT nID);

// Operations
public:
	DWORD GetTabStyle() const { return m_dwTabCtrlStyle; }
	DWORD SetTabStyle(DWORD dwTabStyle);
    void EnableTab(CWnd* pWnd, BOOL bEnable = TRUE);
    void EnableTab(int nIndex, BOOL bEnable = TRUE);
	BOOL SetFontActiveTab(CFont* pFont,BOOL bRedraw=TRUE);
	BOOL SetFontInactiveTab(CFont* pFont,BOOL bRedraw=TRUE);
	CFont* GetFontActiveTab();
	CFont* GetFontInactiveTab(); 			

// Attributes
	BOOL IsTabEnabled(CWnd* pWnd);
	BOOL IsTabEnabled(int nIndex);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SEC3DTabWnd)
    //}}AFX_VIRTUAL
    
// Implementation
private:
	void VLine3D(CDC* pDC, int x, int y, int nHeight);
	void HLine3D(CDC* pDC, int x, int y, int nWidth, BOOL bFlip = FALSE);

protected:
    int m_cyTabCtrl;
	int m_cxTabCtrl;
	DWORD m_dwTabCtrlStyle;

    void rgbFill(CDC* pDC,int x,int y,int cx,int cy, COLORREF rgb);
    void GetInsideRect(CRect& rect) const;
    virtual void RecalcLayout();
    
    // Generated message map functions
protected:
    //{{AFX_MSG(SEC3DTabWnd)
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnTabSelect(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
	//}}AFX_MSG
	afx_msg void OnSysColorChange();
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

#endif // __TABWND3_H__
