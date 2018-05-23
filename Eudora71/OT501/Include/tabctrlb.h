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
//  Description:  Declarations for SECTab and SECTabControlBase
//

#ifndef __TABCTRLB_H__
#define __TABCTRLB_H__

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
// SECTab

class SECTab : public CObject
{
    DECLARE_DYNAMIC(SECTab);
	
// Attributes
public:
    CString m_csLabel;
    int m_nWidth;
	int m_nHeight;
    BOOL m_bSelected;
    CObject* m_pClient; // Object associated with this tab
    HMENU m_hMenu;
    void* m_pExtra;  // Optionally used to associate extra data

    SECTab();

// Implementation
protected:
    ~SECTab();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// SECTabControlBase window

// Tab Control Messages
#define TCM_TABSEL		(WM_USER+1000)
#define TCM_TABDBLCLK		(WM_USER+1001)
#define TCM_TABSELCLR		(WM_USER+1002)

// Tab Control Styles
#define TCS_TABS_ON_BOTTOM	0x0010
#define TCS_TABS_ON_TOP		0x0020
#define TCS_TABS_ON_LEFT	0x0040
#define TCS_TABS_ON_RIGHT	0x0080
#define TCS_TAB_ORIENTATIONS 0x00f0

class SECTabControlBase : public CWnd
{
    DECLARE_DYNAMIC(SECTabControlBase);

// Construction
public:
    SECTabControlBase();
    ~SECTabControlBase();

// Overridables
    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
    virtual void Initialize(DWORD dwStyle);
	virtual SECTab* CreateNewTab() const;
	virtual void RecalcLayout();
	virtual void OnActivateTab(int nTab);
	virtual void DestroyGDIObjects(){ /*place holder for SEC3DTabControl for now*/ }

public:

// Operations
    virtual SECTab* InsertTab(int nIndex,
							  LPCTSTR lpszLabel = NULL,
							  CObject *pClient = NULL,
							  HMENU hMenu = 0,
							  void* pExtra = NULL);
    virtual SECTab* AddTab(LPCTSTR lpszLabel = NULL,
						   CObject* pClient = NULL,
						   HMENU hMenu = 0,
						   void* pExtra = NULL);
	virtual void SetTabIcon(int nIndex, HICON hIcon);
	virtual void SetTabIcon(int nIndex, UINT nIDIcon);
	virtual void SetTabIcon(int nIndex, LPCTSTR lpszResourceName);
    virtual void DeleteTab(int nTab);
    virtual void RenameTab(int nTab, LPCTSTR lpszLabel);
    virtual void ActivateTab(int nTab);  // The tab appearing on top of all others
    virtual void SelectTab(int nTab);
    virtual void ClearSelection();
    virtual void ScrollToTab(int nTab);
	virtual void InvalidateTab(int nTab);

// Queries
    int GetTabCount() const;
    BOOL GetTabInfo(int nTab, LPCTSTR& lpszLabel, BOOL& bSelected,
    			CObject*& pClient, HMENU& hMenu, void*& pExtra) const;
    BOOL FindTab(const CObject* const pClient, int& nTab) const;
    BOOL GetActiveTab(int& nTab) const;
    BOOL TabExists(CObject* pClient) const;
    BOOL TabExists(int nTab) const;

// Implementation
protected:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    CObArray *m_pTabArray;
    int m_nActiveTab;
    BOOL m_bActiveTab;
    HCURSOR m_hcurArrow;

    SECTab& GetTab(int nTab) const;
	SECTab* GetTabPtr(int nTab) const;
    
    // Generated message map functions
protected:
    //{{AFX_MSG(SECTabControlBase)
    afx_msg void OnDestroy();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
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

#endif // __TABCTRLB_H__
/////////////////////////////////////////////////////////////////////////////
