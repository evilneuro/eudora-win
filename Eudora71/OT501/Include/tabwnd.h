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
//  Author:			Dean Hallman
//  Description:	SECTabWnd declaration
//

#ifndef __TABWND_H__
#define __TABWND_H__

#ifndef __TABCTRL_H__
#include "tabctrl.h"
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

// Tab Window Styles
#define TWS_LEFTRIGHTSCROLL	0x0001
#define TWS_FULLSCROLL		0x0002

#define TWS_TABS_ON_BOTTOM	0x0010
#define TWS_TABS_ON_TOP		0x0020
#define TWS_TABS_ON_LEFT	0x0040
#define TWS_TABS_ON_RIGHT	0x0080

#define TWS_TAB_ORIENTATIONS 0x00f0

#define TWS_NOACTIVE_TAB_ENLARGED	0x0100
#define TWS_TAB_DRAW_STYLES			0x0f00

class SECTabWnd : public SECTabWndBase
{
    DECLARE_DYNCREATE(SECTabWnd)
    
// Construction
public:
    SECTabWnd();
    virtual ~SECTabWnd();

    BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE |
    		WS_HSCROLL | WS_VSCROLL | TWS_FULLSCROLL | TWS_TABS_ON_BOTTOM,
			UINT nID = AFX_IDW_PANE_FIRST);

protected:    
	virtual BOOL CreateTabCtrl(DWORD dwStyle, UINT nID);

// Operations
public:
    virtual SECTab* InsertTab(CWnd* pWnd, int nIndex, LPCTSTR lpszLabel);
	virtual SECTab* InsertTab(CRuntimeClass* pViewClass, int nIndex,
		LPCTSTR lpszLabel, CCreateContext* pContext = NULL, UINT nID = -1)
	{
		return SECTabWndBase::InsertTab(pViewClass,
			nIndex, lpszLabel, pContext, nID);
	}

	virtual void RemoveTab(int nIndex);
	virtual void SetScrollStyle(int nTab, DWORD dwStyle);
    virtual void ActivateTab(CWnd* pWnd, int nIndex);
    virtual void ActivateTab(CWnd* pWnd) { SECTabWndBase::ActivateTab(pWnd); };
    virtual void ActivateTab(int nIndex) { SECTabWndBase::ActivateTab(nIndex); };
    virtual CScrollBar* GetScrollBar(int nBar, const CWnd* const pWnd = NULL) const;

	// font support
	BOOL SetFontSelectedTab(CFont* pFont,BOOL bRedraw=TRUE);
	BOOL SetFontUnselectedTab(CFont* pFont,BOOL bRedraw=TRUE); 	
	BOOL SetFontActiveTab(CFont* pFont,BOOL bRedraw=TRUE); 	
	CFont* GetFontSelectedTab(); 			
	CFont* GetFontUnselectedTab(); 			
	CFont* GetFontActiveTab();

// Overrides5
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECTabWnd)
    //}}AFX_VIRTUAL
    
// Implementation
protected:
    int m_cxTabCtrl;
    int m_cyTabCtrl;
    int m_cxVScroll;
    int m_cyHScroll;
    int	m_cxSplitter;
    int m_nTrackOffset;
    BOOL m_bTracking;
    BOOL m_bSplitterHit;
    BOOL m_bFirstSize;
    HCURSOR m_hcurSplit;
    HCURSOR m_hcurArrow;
	DWORD m_dwTabCtrlStyle;
#ifdef WIN32
	CPtrArray m_arrHScrollInfo;
	CPtrArray m_arrVScrollInfo;
#endif
	CDWordArray m_arrScrollStyles;

    void GetSplitterRect(CRect& rectSplit);
    void OnDrawSplitter(CDC* pDC, const CRect& rect);
    void rgbFill(CDC* pDC,int x,int y,int cx,int cy, COLORREF rgb);
    void GetInsideRect(CRect& rect) const;
    void RecalcLayout();

    // Generated message map functions
protected:
    //{{AFX_MSG(SECTabWnd)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnTabSelect(WPARAM wParam, LPARAM lParam);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

// For naming uniformity and backward compatibility,
// you can call it SECTabWnd or SEC2DTabWnd.
class SEC2DTabWnd : public SECTabWnd
{
    DECLARE_DYNCREATE(SEC2DTabWnd)
    
// Construction
public:
    SEC2DTabWnd();
    virtual ~SEC2DTabWnd();
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TABWND_H__
