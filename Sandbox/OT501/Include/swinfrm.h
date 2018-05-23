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
//  Description:	Declarations for SECFrameWnd
//  Created:		May 1996
//
//	Modified:		Feb 1997 (Kenn Scribner)
//	Description:	Added italicized text to inactive windows
//					Fixed MDI-derived class behavior (parent
//					and children window captions are activated/
//					de-activated properly)
//
//

#ifdef WIN32

#ifndef __SWINFRM_H__
#define __SWINFRM_H__

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
// SECFrameWnd frame

// Forward Declarations
class SECDockState;
class SECControlBarManager;

class SECFrameWnd : public CFrameWnd
{
    DECLARE_DYNCREATE(SECFrameWnd)

protected:
    SECFrameWnd();

// Attributes
public:
	BOOL GetActiveState() { return m_bActive; }
protected:
	CRect m_prevLayout;

	// Gradient Caption Attributes
	UINT m_uiTextAlign;		// DrawText alignment flags for caption
	BOOL m_bNullGetText;	// return nothing for WM_GETTEXT message?
	BOOL m_bHandleCaption;	// is custom caption drawing enabled?
	BOOL m_bActive;			// WM_ACTIVATE status

	SECControlBarManager* m_pControlBarManager;

// Operations
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    void EnableDocking(DWORD dwDockStyle);
	void FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);
	virtual void DockControlBarEx(CControlBar* pBar, UINT nDockBarID = 0,
		int nCol = 0, int nRow = 0, float fPctWidth = (float)1.0, int nHeight = 150);
	void DockControlBar(CControlBar* pBar, UINT nDockBarID = 0,
		LPCRECT lpRect = NULL);
	void ReDockControlBar(CControlBar* pBar, CDockBar* pDockBar,
		LPCRECT lpRect = NULL);
	void ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay);

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
	void ForceCaptionRedraw(BOOL bImmediate = FALSE);

	// Caption text control members

	enum AlignCaption { acLeft, acCenter, acRight };
	AlignCaption GetCaptionTextAlign() const
		{ return (AlignCaption)m_uiTextAlign; }
	void SetCaptionTextAlign(AlignCaption ac, BOOL bRedraw = TRUE);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECFrameWnd)
 	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
   //}}AFX_VIRTUAL

// Message handler member functions
protected:
	afx_msg void OnSysColorChange();

// Implementation
protected:
    virtual ~SECFrameWnd();
	static const DWORD dwSECDockBarMap[4][2];
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	void GetWindowsVersion();
	BOOL m_bIsWin95orAbove;

public:
	void DockControlBar(CControlBar* pBar, CDockBar* pDockBar,
		LPCRECT lpRect = NULL);
	virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActivateWnd,
		HMENU hMenuAlt);

    // Generated message map functions
    //{{AFX_MSG(SECFrameWnd)
	afx_msg void OnNcPaint();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	//}}AFX_MSG
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
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

#endif // __SWINFRM_H__

#else // WIN32                       

#define SECFrameWnd CFrameWnd

#endif // WIN32