/*////////////////////////////////////////////////////////////////////////////

NAME:
	CMainFrame - 

FILE:		MainFrm.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	CMainFrame
	CMainStatusBar
	CMainToolBar

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/??/96   lss     -Initial

/*////////////////////////////////////////////////////////////////////////////

#ifndef _MAINFRM_H_
#define _MAINFRM_H_

#include "NameToolBar.h"

class CMainStatusBar : public CStatusBar
{
public:
    CProgressCtrl   m_wndProgress;
    BOOL Init(CWnd* pParentWnd);
    CProgressCtrl* SetUpProgress();
    BOOL RemoveProgress();
};

class CMainToolBar : public CToolBar
{
public:
#ifdef LS_TOOLBAR_COMBOBOX
 	CComboBox	m_cmbCodec;
#endif
	BOOL Init(CWnd* pParentWnd);
};

class CMainFrame : public CFrameWnd
{
protected:
//	LIFECYCLE
	// create from serialization only
	DECLARE_DYNCREATE(CMainFrame)
	CMainFrame();
public:
	virtual ~CMainFrame();

//	OPERATIONS
	void SendFileName2Eudora( CString& fileName, BOOL bAttachDesc = FALSE );

//	ACCESS
#ifndef LS_PLAYER_ONLY
	CMainStatusBar  m_wndStatusBar;
	CMainToolBar    m_wndToolBar;
	CNameToolBar	m_wndNameBar;
#endif

//////////////////////////////////////////////////////////////////////////////
protected: 
//	METHODS

//	ATTRIBUTES
	BOOL	mb_StayOnTop;
	HWND	m_hWndEudoraFileNameReq; // Eudora hWnd to pass file name

//	DEBUG STUFF
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//////////////////////////////////////////////////////////////////////////////
// ClassWizard and Window Msg stuff
//////////////////////////////////////////////////////////////////////////////

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnUserFileName(WPARAM wParam, LPARAM lParam);
	afx_msg void OnStayOntop();
	afx_msg void OnUpdateStayOntop(CCmdUI* pCmdUI);
	afx_msg void OnShowname();
	afx_msg void OnUpdateShowname(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
