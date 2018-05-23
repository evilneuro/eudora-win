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
//  Description:	Declarations for SECFDIFrameWnd
//

#ifndef __FDIMAIN_H__
#define __FDIMAIN_H__

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

class SECFDIFrameWnd : public SECFrameWnd
{
    DECLARE_DYNCREATE(SECFDIFrameWnd);

// Construction
public:
    SECFDIFrameWnd();

// Attributes
protected:
	CPtrList m_children;
	CWnd* m_pActiveWnd;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECFDIFrameWnd)
	public:
	virtual CFrameWnd* GetActiveFrame();
	//}}AFX_VIRTUAL

	public:
    ~SECFDIFrameWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

// Operations
public:
	void AddWindow(CWnd* pWnd);
	void RemoveWindow(CWnd* pWnd);
	void SetActiveWindow(CWnd* pWnd);
	CWnd* GetActiveWindow() const;

// Generated message map functions
protected:
    //{{AFX_MSG(SECFDIFrameWnd)
	afx_msg BOOL OnActivateWindow(UINT nID);
	afx_msg void OnUpdateWindowMenu(CCmdUI* pCmdUI);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};

/////////////////////////////////////////////////////////////////////////////
// SECWindowsDialog dialog

class SECWindowsDialog : public CDialog
{
// Construction
public:
	SECWindowsDialog(CWnd* pParent = NULL, CPtrList* pChildren = NULL);
	
// Attributes
public:
	CPtrList* m_pChildren;

// Dialog Data
	//{{AFX_DATA(SECWindowsDialog)
	enum { IDD = IDD_WINDOWS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECWindowsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CObList* m_pWindowList;

	// Generated message map functions
	//{{AFX_MSG(SECWindowsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseWindow();
	virtual void OnOK();
	afx_msg void OnSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __FDIMAIN_H__
