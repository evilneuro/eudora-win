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
//  Description:	Declarations for SECControlBar
//  Created:		May 1996
//
//	Modified:	 	March 1997 (Mark Isham)
//	Description: 	Revamped gripper support
//	

#ifdef WIN32

#ifndef __SBARCORE_H__
#define __SBARCORE_H__

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
// SECControlBar

// Forward Declarations
class SECControlBarInfo;
class SECControlBarManager;

// Constants
#define CX_BORDER   1
#define CY_BORDER   1

// User-defined messages
#define WM_EXTENDCONTEXTMENU	(WM_USER+800)

// Extended Styles
#define CBRS_EX_STDCONTEXTMENU		0x0001L  // Standard context menu items
#define CBRS_EX_STRETCH_ON_SIZE		0x0002L  // Control bar is stretched on resize
#define CBRS_EX_UNIDIRECTIONAL		0x0004L  // Bar can be sized in one direction
                                             //    at a time (i.e. toolbar)
#define CBRS_EX_DRAWBORDERS			0x0008L  // Draw a border around the bar
#define CBRS_EX_BORDERSPACE			0x0010L  // Leave border space for ease of dragging
#define CBRS_EX_ALLOW_MDI_FLOAT		0x0020L  // Control bar can be re-parented
											 //    by an MDI Child window
#define CBRS_EX_SIZE_TO_FIT			0x0040L  // Size the (single) child to fit

// Not yet implemented
#define CBRS_EX_DISALLOW_FLOAT		0x0080L  // Control bar can only be docked

// You probably just want to set the CBRS_EX_COOL style in your app,
// but you can also play around with these styles to customize
// Note: As of OT 5.0, gripper requires coolboders, and close reqs gripper.
#define CBRS_EX_COOLBORDERS			0x0100L  // floating buttons, no border
#define CBRS_EX_GRIPPER				0x0200L	 // draw dragging "gripper"
#define CBRS_EX_GRIPPER_CLOSE		0x0400L	 // draw close button on gripper
#define CBRS_EX_GRIPPER_EXPAND		0x0800L	 // draw expand/contract button
#define CBRS_EX_TRANSPARENT			0x1000L	 // toolbar buttons drawn transparently
#define CBRS_EX_COOL				CBRS_EX_COOLBORDERS | CBRS_EX_GRIPPER | \
									CBRS_EX_GRIPPER_CLOSE | CBRS_EX_GRIPPER_EXPAND

////////////////////////////////////////////////////////////////////////
// SECGripperInfo
//
class SECGripperInfo : public CObject {
public:
	SECGripperInfo();
	~SECGripperInfo();

	// override these for your own custom gripper
	virtual int GetWidth();
	virtual int GetHeight();

public:
	// horizontal spacing
	int m_cxPad1;
	int m_cxWidth1;
	int m_cxPad2;
	int m_cxWidth2;
	int m_cxPad3;

	// vertical spacing
	int m_cyPad1;
	int m_cyWidth1;
	int m_cyPad2;
	int m_cyWidth2;
	int m_cyPad3;

	// padding between border and client area for all non-gripper sides
	// when gripper style enabled
	int m_nGripperOffSidePadding;

	// Tooltip text 
	CString m_strCloseTipText;
	CString m_strExpandTipText;
	CString m_strContractTipText;
};


/////////////////////////////////////////////////////////////////////////
// SECControlBar
//

class SECControlBar : public CControlBar
{
	DECLARE_DYNCREATE(SECControlBar)

// Construction
public:
    SECControlBar();

	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, UINT nID,
		DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd = NULL,
		CCreateContext* pContext = NULL);
	virtual BOOL Create(CWnd* pParentWnd, LPCTSTR lpszWindowName, DWORD dwStyle,
		DWORD dwExStyle, UINT nID, CCreateContext* pContext = NULL);

// Attributes
public:

 	static BOOL m_bOptimizedRedrawEnabled;  // TRUE if optimized redraw is in effect;

  	CSize m_szDockHorz; // Dimensions when docked horizontally
	CPoint m_ptDockHorz;
	CSize m_szDockVert; // Dimensions when docked vertically
	CSize m_szFloat;    // Dimensions when floating
	DWORD m_dwMRUDockingState; // Remember docking state when "Allow Docking" unchecked
	float m_fPctWidth;
	float m_fDockedPctWidth;			// pct width when docked (to reenable after float)
	DWORD m_dwExStyle;  				// extended style bits

protected:

	CRect m_rcBorderSpace; 				// White space around bar used for dragging
	SECControlBarManager* m_pManager;	// Control bar manager
	SECGripperInfo m_GripperInfo;
	CRect m_rcGripperCloseButton;		
	BOOL  m_bClickingGripperClose;		// left button is depressed on gripper close
	CRect m_rcGripperExpandButton;		
	BOOL  m_bClickingGripperExpand;		// left button is depressed on gripper expand
	BOOL  m_bGripperExpandEnabled;
	BOOL  m_bGripperExpandExpanding;
	BOOL  m_bGripperExpandHorz;

// Attribute Retrieval
public:
	virtual void GetInsideRect(CRect& rectInside) const;
	BOOL IsMDIChild() const;

protected:

	struct LayoutInfo {
		HWND m_hWnd;
		CRect m_rect;
	};

	CPtrArray* m_pArrLayoutInfo;

// Operations
public:
    void EnableDocking(DWORD dwDockStyle);
	DWORD GetExBarStyle() const
		{ return m_dwExStyle; };
	virtual void SetExBarStyle(DWORD dwExStyle,BOOL bAutoUpdate=FALSE);
	virtual void ModifyBarStyleEx(DWORD dwRemove,DWORD dwAdd,BOOL bAutoUpdate=FALSE);
	
// Overrides
	virtual int OnToolHitTest(CPoint point,TOOLINFO* pTI) const;

	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECControlBar)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~SECControlBar();
 
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
	virtual void DoPaint(CDC* pDC);
	virtual void DrawBorders(CDC* pDC, CRect& rect);
	void EraseNonClient();
	void GetBarInfo(SECControlBarInfo* pInfo);
	void SetBarInfo(SECControlBarInfo* pInfo, CFrameWnd* pFrameWnd);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	void CalcInsideRect(CRect& rect, BOOL bHorz, BOOL bVert = FALSE) const;
	inline SECControlBarManager* GetManager() const;
	inline void SetManager(SECControlBarManager*);

// Overidables
public:
	virtual void OnBarDock();
	virtual void OnBarFloat();
	virtual void OnBarMDIFloat();
	virtual void GetBarInfoEx(SECControlBarInfo* pInfo);
	virtual void SetBarInfoEx(SECControlBarInfo* pInfo, CFrameWnd* pFrameWnd);

protected:
	void InitLayoutInfo();
	void DeleteLayoutInfo();
	virtual void OnExtendContextMenu(CMenu* pMenu);

	//
	// gripper drawing functions
	//
	// override *both* of these if you want to draw your own custom gripper
	virtual void AdjustInsideRectForGripper(CRect& rect,BOOL bHorz);
	virtual void DrawGripper(CDC* pDC, CRect& rect);

	// utility functions to draw the gripper "close" button
	virtual void DrawGripperCloseButton(CDC* pDC, CRect& rect,BOOL bHorz);
	virtual void DrawGripperCloseButtonDepressed(CDC* pDC);
	virtual void DrawGripperCloseButtonRaised(CDC* pDC);

	// utility functions to draw the gripper "expand" button
	virtual void DrawGripperExpandButton(CDC* pDC, CRect& rect,BOOL bHorz);
	virtual void DrawGripperExpandButtonDepressed(CDC* pDC);
	virtual void DrawGripperExpandButtonRaised(CDC* pDC);
	virtual void SetGripperExpandButtonState(BOOL bHorz);

// Command Handlers
protected:

	afx_msg void OnHide();
	afx_msg void OnToggleAllowDocking();
	afx_msg void OnFloatAsMDIChild();

// Generated message map functions
protected:
	
    //{{AFX_MSG(SECControlBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt );
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt );
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// inline functions

inline SECControlBarManager* SECControlBar::GetManager() const
{
	return m_pManager;
}

inline void SECControlBar::SetManager(SECControlBarManager* pManager)
{
	m_pManager = pManager;
}

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SBARCORE_H__

#else // WIN32

#define SECControlBar CControlBar

#endif // WIN32