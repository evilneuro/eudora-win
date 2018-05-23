// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Author:			Dean Hallman
//  Description:	Declarations for SECMDIFrameWnd	/ SECMDIChildWnd / SECControlBarMDIChildWnd
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __SWINMDI_H__
#define __SWINMDI_H__

#ifndef __AFXPRIV_H__
#include <afxpriv.h>
#endif

#ifndef __SWINFRM_H__
#include "swinfrm.h"
#endif

#ifndef __SBARDOCK_H__
#include "sbardock.h"
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

// Forward Declarations
class SECControlBarMDIChildWnd;
class SECControlBarInfo;
class SECControlBarManager;

/////////////////////////////////////////////////////////////////////////////
// SECMDIFrameWnd

class SECMDIFrameWnd : public CMDIFrameWnd
{
    DECLARE_DYNCREATE(SECMDIFrameWnd)

protected:
    SECMDIFrameWnd(); 

// Attributes
protected:
	CRect m_prevLayout;
	CRuntimeClass* m_pFloatingMDIChildClass;
	static const DWORD dwSECDockBarMap[4][2];
	CPoint m_ptDefaultMDIPos;
	SECFrameWnd* m_pActiveDockableFrame;
	SECControlBarManager* m_pControlBarManager;

	// Gradient Caption Attributes
	UINT m_uiTextAlign;		// DrawText alignment flags for caption
	BOOL m_bNullGetText;	// return nothing for WM_GETTEXT message?
	BOOL m_bHandleCaption;	// is custom caption drawing enabled?

// Operations
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    void EnableDocking(DWORD dwDockStyle);
	virtual void DockControlBarEx(CControlBar* pBar, UINT nDockBarID = 0,
		int nCol = 0, int nRow = 0, float fPctWidth = (float)1.0, int nHeight = 150);
	void FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);
	void DockControlBar(CControlBar* pBar, UINT nDockBarID = 0,
		LPCRECT lpRect = NULL);
	void ReDockControlBar(CControlBar* pBar, CDockBar* pDockBar,
		LPCRECT lpRect = NULL);
	void ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay);
	void FloatControlBarInMDIChild(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);
	void ReFloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);
	virtual CMDIChildWnd* CreateFloatingMDIChild(DWORD dwStyle, CControlBar* pBar = NULL);

	virtual void ActivateDockableFrame(SECFrameWnd* pFrameWnd);
	virtual void OnActivateDockableFrame(SECFrameWnd* pFrame);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	// Active frame (for frames within frames -- MDI)
	virtual CFrameWnd* GetActiveFrame();

	// saving and loading control bar state
	virtual void LoadBarState(LPCTSTR lpszProfileName);
	virtual void SaveBarState(LPCTSTR lpszProfileName) const;
	virtual void SetDockState(SECDockState& state);
	virtual void GetDockState(SECDockState& state) const;

	// Control Bar Manager
	virtual SECControlBarManager* GetControlBarManager() const;
	virtual void SetControlBarManager(SECControlBarManager*);

	// General custom caption members

	BOOL IsCustomCaptionEnabled() const
		{ return m_bHandleCaption; }
	BOOL EnableCustomCaption(BOOL bEnable, BOOL bRedraw = TRUE);
	void ForceCaptionRedraw();

	// Caption text control members

	enum AlignCaption { acLeft, acCenter, acRight };
	AlignCaption GetCaptionTextAlign() const
		{ return (AlignCaption)m_uiTextAlign; }
	void SetCaptionTextAlign(AlignCaption ac, BOOL bRedraw = TRUE);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECMDIFrameWnd)
	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
    //}}AFX_VIRTUAL

// Message handler member functions
protected:
	afx_msg void OnSysColorChange();
    afx_msg LRESULT OnExtendContextMenu(WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	void GetWindowsVersion();
	BOOL m_bIsWin95orAbove;

    virtual ~SECMDIFrameWnd();
public:
	void DockControlBar(CControlBar* pBar, CDockBar* pDockBar,
		LPCRECT lpRect = NULL);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);

protected:

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
    //{{AFX_MSG(SECMDIFrameWnd)
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcPaint();
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// SECMDIChildWnd

class SECMDIChildWnd : public CMDIChildWnd
{
    DECLARE_DYNCREATE(SECMDIChildWnd)

protected:
    SECMDIChildWnd(); 

// Operations
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECMDIChildWnd)
    //}}AFX_VIRTUAL

// Message handler member functions
protected:

// Implementation
protected:
    virtual ~SECMDIChildWnd();

	// Generated message map functions
    //{{AFX_MSG(SECMDIChildWnd)
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnChildActivate();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// SECControlBarMDIChildWnd

class SECControlBarMDIChildWnd : public SECMDIChildWnd
{
	DECLARE_DYNCREATE(SECControlBarMDIChildWnd)

// Constructors
protected:
	SECControlBarMDIChildWnd();           // protected constructor used by dynamic creation

public:
	SECDockBar	m_wndDockBar;		

// Attributes
public:

// Overrides
	void RecalcLayout(BOOL bNotify = TRUE);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECControlBarMDIChildWnd)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~SECControlBarMDIChildWnd();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	// Generated message map functions
	//{{AFX_MSG(SECControlBarMDIChildWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
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

#endif // __SWINMDI_H__

#else // WIN32                       

#define SECMDIFrameWnd CMDIFrameWnd
#define SECMDIChildWnd CMDIChildWnd

#endif // WIN32
