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
//  Description:	Declarations for Worksheet, Workbook, and WorkbookClient
//

#ifndef __SECWB_H__
#define __SECWB_H__

#ifndef __SWINMDI_H__
#include "swinmdi.h"
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

class SECWorkbook;

class SECWorksheet : public SECMDIChildWnd
{
    friend class SECWorkbook;
    DECLARE_DYNCREATE(SECWorksheet);
    
// Construction
public:
    SECWorksheet();
    
	// Public accessor functions for protected data in base class		//QUALCOMM
	BOOL IsSelected() const												//QUALCOMM
		{ return m_bSelected; }											//QUALCOMM
	void SetSelected(BOOL bSelected)									//QUALCOMM
		{ m_bSelected = bSelected; }									//QUALCOMM
	int GetPosition() const												//QUALCOMM
		{ return m_nPosition; }											//QUALCOMM

// Implementation
protected:
    HICON	m_hIcon;	// Icon for this window
    int		m_nPosition;	// Position of tab within array
    BOOL	m_bSelected;    // True if the tab is selected but
				//   the frame isn't yet active

    SECWorkbook* GetWorkbook();
    virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
			   CWnd* pParentWnd, CCreateContext* pContext = NULL);
    
public:
    virtual ~SECWorksheet();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    //{{AFX_MSG(SECWorksheet)
    afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
    afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

#ifndef _DEBUG  // debug version in secwb.cpp
inline SECWorkbook* SECWorksheet::GetWorkbook()
   { return (SECWorkbook*)GetMDIFrame(); };
#endif

class SECWorkbookClient : public CWnd
{
    DECLARE_DYNCREATE(SECWorkbookClient);
	
// Construction
public:
    SECWorkbookClient();
    
// Operations
public:
    void SetMargins(int left, int right, int top, int bottom);
    void GetMargins(int& left, int& right, int& top, int& bottom);
    
// Implementation
protected:
    CRect m_margins;	// Space between frame border and MDI Client
    virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType);
    
public:
    virtual ~SECWorkbookClient();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    
// Generated message map functions
protected:
    //{{AFX_MSG(SECWorkbookClient)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

class SECWorkbook : public SECMDIFrameWnd
{
    friend class SECWorksheet;
    DECLARE_DYNCREATE(SECWorkbook);

// Construction
public:
    SECWorkbook();

// Attributes
public:

    CObArray m_worksheets;		// List of worksheets
    BOOL	m_bWorkbookMode;	// Are we in workbook mode?
    BOOL	m_bShowIcons;		// Show document icons on the tabs?
    CFont	m_fontTab;		// The font to use for tab labels
    int		m_nScrollOffset;	// Amount to displace tab offset by
    SECWorkbookClient* m_pWBClient;	// The MDI client
    CPen	m_penWhite, m_penBlack, m_penShadow, m_penFace;
    int		m_cxTab;		// Width of tab 
    int		m_cyTab;		// Height of tab
    int		m_cxFold;		// Width of fold in the tab
	int		m_cxActive;		// Extra tab width when active
	int		m_cyActive;		// Extra tab height when active
	int		m_nMaxTabWidth;
    CRect	m_rectClientLast;	// Redraw hint    
// Implementation
protected:

    void GetTabRgn(SECWorksheet* pSheet, CRgn& rgn);
    void InvalidateTab(SECWorksheet* pSheet, BOOL bInvalidAfter = FALSE);
    void InvalidateAllTabs();
    int recalcTabWidth();
	inline int GetIconWidth() { return m_cyTab-8; }

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECWorkbook)
public:
    virtual BOOL CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu);
    virtual void RecalcLayout(BOOL bNotify = TRUE);
    //}}AFX_VIRTUAL

#ifdef WIN32
	// tooltip support
protected:
	CToolTipCtrl* m_pToolTips;
public:
	virtual int OnToolHitTest(CPoint point,TOOLINFO* pTI) const;
#endif 

public:
    ~SECWorkbook();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Overridables
protected:

    virtual void AddSheet(SECWorksheet* pSheet);
    virtual void RemoveSheet(SECWorksheet* pSheet);
	virtual BOOL IsFullTabTextVisible(SECWorksheet* pSheet);
    virtual void GetTabPts(SECWorksheet* pSheet, CPoint*& pts, int& count);
    virtual const TCHAR* GetTabLabel(SECWorksheet* pSheet) const;
	virtual BOOL GetTextRect(CPoint* pts,int ciIconWidth,CRect& rectText);
    virtual void OnDrawTab(CDC* pDC, SECWorksheet* pSheet);
	virtual void OnDrawBorder(CDC* pDC);
    virtual void OnDrawTabIconAndLabel(CDC* pDC, SECWorksheet* pSheet);

// Operations
public:

    void SetWorkbookMode(BOOL bEnabled = TRUE);
    void SetShowIcons(BOOL bShowIcons = TRUE);
    virtual HICON GetTabIcon(SECWorksheet* pSheet) const;
	virtual BOOL SetTabIcon(SECWorksheet* pSheet,HICON hIcon,BOOL bRedraw=TRUE);

// Queries
public:

    BOOL LookupSheet(SECWorksheet* pSheet, int& nIndex);
    SECWorksheet* GetWorksheet(int nSheet) const;
    int GetSheetCount() const;

// Generated message map functions
protected:
    //{{AFX_MSG(SECWorkbook)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnViewWorkbook();
    afx_msg void OnUpdateViewWorkbook(CCmdUI* pCmdUI);
    afx_msg void OnOptionsToggleIcons();
    afx_msg void OnUpdateOptionsToggleIcons(CCmdUI* pCmdUI);
    afx_msg void OnDestroy();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

/////////////////////////////////////////////////////////////////////////////
// SECControlBarWorksheet

class SECControlBarWorksheet : public SECWorksheet
{
	DECLARE_DYNCREATE(SECControlBarWorksheet)

// Constructors
protected:
	SECControlBarWorksheet();           // protected constructor used by dynamic creation

public:
#ifdef WIN32
	SECDockBar	m_wndDockBar;		
#endif

// Attributes
public:

// Overrides
	void RecalcLayout(BOOL bNotify = TRUE);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECControlBarWorksheet)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~SECControlBarWorksheet();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	// Generated message map functions
	//{{AFX_MSG(SECControlBarWorksheet)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// For backward compatibility...
#define SECDDIWorksheet SECWorksheet
#define SECDDIWorkbookClient SECWorkbookClient
#define SECDDIWorkbook SECWorkbook

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECWB_H__
