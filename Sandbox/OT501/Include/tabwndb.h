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
//  Description:	SECTabWndBase declaration
//

#ifndef __TABWNDB_H__
#define __TABWNDB_H__

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

// Tab Control Window ID
#define SEC_IDW_TABCTRL                 0xEA30
#define SEC_IDW_TABCLIENT_FIRST         0xEB00  // 256 tabs max
#define SEC_IDW_TABCLIENT_LAST          0xEBff
#define SEC_IDW_HORZ_SCROLLBAR          0xEC00  // Horz scrollbar
#define SEC_IDW_VERT_SCROLLBAR          0xECff  // Vert scrollbar

// Default tab styles
#define SEC_WS_DEFAULT_TABCLIENT	(WS_CHILD | WS_VISIBLE)

class SECTabWndBase : public CWnd
{
    DECLARE_DYNAMIC(SECTabWndBase)
    
// Construction
public:
    SECTabWndBase();
    virtual ~SECTabWndBase();

	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL);

    virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE |
    		WS_HSCROLL | WS_VSCROLL, UINT nID = AFX_IDW_PANE_FIRST);

protected:    
    CWnd* CreateView(CRuntimeClass* pViewClass,
			 CCreateContext* pContext = NULL, UINT nID = -1);
    virtual BOOL CreateScrollBarCtrl(DWORD dwStyle, UINT nID);
	virtual BOOL CreateTabCtrl(DWORD dwStyle, UINT nID);

public:

// Operations
    virtual SECTab* InsertTab(CWnd* pWnd,
							  int nIndex,
							  LPCTSTR lpszLabel);
    virtual SECTab* InsertTab(CRuntimeClass* pViewClass,
							  int nIndex,
							  LPCTSTR lpszLabel,
							  CCreateContext* pContext = NULL,
							  UINT nID = -1);
    virtual SECTab* AddTab(CWnd* pWnd,
						   LPCTSTR lpszLabel);
	virtual SECTab* AddTab(CRuntimeClass* pViewClass,
						   LPCTSTR lpszLabel,
						   CCreateContext* pContext = NULL,
						   UINT nID = -1);
	virtual void SetTabIcon(int nIndex, HICON hIcon);
	virtual void SetTabIcon(int nIndex, UINT nIDIcon);
	virtual void SetTabIcon(int nIndex, LPCTSTR lpszResourceName);
	virtual void SetScrollStyle(int nIndex, DWORD dwStyle);
    virtual void RemoveTab(CWnd* pWnd);
    virtual void RemoveTab(int nIndex);
    virtual void RenameTab(CWnd* pWnd, LPCTSTR lpszLabel);
    virtual void RenameTab(int nIndex, LPCTSTR lpszLabel);
    virtual void ActivateTab(CWnd* pWnd, int nIndex);
    virtual void ActivateTab(CWnd* pWnd);
    virtual void ActivateTab(int nIndex);
    virtual void ClearSelection();
    virtual void ScrollToTab(CWnd* pWnd);
    virtual void ScrollToTab(int nIndex);

// Queries
    int GetTabCount();
    BOOL GetTabInfo(int nIndex, LPCTSTR& lpszLabel, BOOL& bSelected, CWnd*& pWnd, void*& pExtra);
    BOOL FindTab(const CWnd* const pWnd, int& nTab) const;
    BOOL GetActiveTab(CWnd*& pWnd);
    BOOL GetActiveTab(int& nIndex);
    BOOL TabExists(CWnd* pClient);
    BOOL TabExists(int nTab);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECTabWndBase)
    //}}AFX_VIRTUAL
    
// Implementation
protected:
    SECTabControlBase *m_pTabCtrl;
    CWnd* m_pActiveWnd;
	CWnd* m_pWndCreating;

    virtual void GetInsideRect(CRect& rect) const;
    virtual void RecalcLayout();
    
    // Generated message map functions
protected:
    //{{AFX_MSG(SECTabWndBase)
    afx_msg void OnDestroy();
    afx_msg LRESULT OnTabSelect(WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

// C Function
SECTabWndBase* GetParentTabWnd(const CWnd* const pWnd);

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TABWNDB_H__
