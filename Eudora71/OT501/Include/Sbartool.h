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
//  Description:	Declarations for SECToolBar
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __SBARTOOL_H__
#define __SBARTOOL_H__

#ifndef __SBARCORE_H__
#include "sbarcore.h"
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

/////////////////////////////////////////////////////////////////////////////
// SECToolBar control

//HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc, BOOL bMono = FALSE);

class CToolBarCtrl; // forward reference (see afxcmn.h for definition)

class SECToolBar : public SECControlBar
{
	DECLARE_DYNAMIC(SECToolBar)

// Construction
public:
	SECToolBar();
	BOOL Create(CWnd* pParentWnd,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		UINT nID = AFX_IDW_TOOLBAR);
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszWindowName,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		DWORD dwExStyle = CBRS_EX_UNIDIRECTIONAL,
		UINT nID = AFX_IDW_TOOLBAR,	CCreateContext* pContext = NULL);


	void SetSizes(SIZE sizeButton, SIZE sizeImage);
		// button size should be bigger than image
	void SetHeight(int cyHeight);
		// call after SetSizes, height overrides bitmap size
	BOOL LoadToolBar(LPCTSTR lpszResourceName);
	BOOL LoadToolBar(UINT nIDResource)
			{ return LoadToolBar(MAKEINTRESOURCE(nIDResource)); };
	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource)
		{ return LoadBitmap(MAKEINTRESOURCE(nIDResource)); };
	BOOL SetBitmap(HBITMAP hbmImageWell);
	BOOL SetButtons(const UINT* lpIDArray, int nIDCount);
		// lpIDArray can be NULL to allocate empty buttons

// Attributes
public:
	// standard control bar things
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;
	UINT GetButtonStyle(int nIndex) const;
	void SetButtonStyle(int nIndex, UINT nStyle);

	// for changing button info
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);
	BOOL SetButtonText(int nIndex, LPCTSTR lpszText);
	CString GetButtonText(int nIndex) const;
	void GetButtonText(int nIndex, CString& rString) const;

	// for direct access to the underlying common control
	CToolBarCtrl& GetToolBarCtrl() const
		{ return *(CToolBarCtrl*)this; };

// Implementation
public:
	virtual ~SECToolBar();
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	void SetOwner(CWnd* pOwnerWnd);
	BOOL AddReplaceBitmap(HBITMAP hbmImageWell);
	virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);

#ifdef _MAC
	virtual BOOL CheckMonochrome();
#endif

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HRSRC m_hRsrcImageWell; // handle to loaded resource for image well
	HINSTANCE m_hInstImageWell; // instance handle to load image well from
	HBITMAP m_hbmImageWell; // contains color mapped button images
	BOOL m_bDelayedButtonLayout; // used to manage when button layout should be done

	BOOL m_bFloatSizeUnknown;
	BOOL m_bHorzDockSizeUnknown;
	BOOL m_bVertDockSizeUnknown;

	CSize m_sizeImage;  // current image size
	CSize m_sizeButton; // current button size

	CMapStringToPtr* m_pStringMap;  // used as CMapStringToUInt

	// implementation helpers
	void _GetButton(int nIndex, TBBUTTON* pButton) const;
	void _SetButton(int nIndex, TBBUTTON* pButton);
	CSize CalcLayout(DWORD nMode, int nLength = -1);
	CSize CalcSize(TBBUTTON* pData, int nCount);
	int WrapToolBar(TBBUTTON* pData, int nCount, int nWidth);
	void SizeToolBar(TBBUTTON* pData, int nCount, int nLength, BOOL bVert = FALSE);

	//{{AFX_MSG(SECToolBar)
	afx_msg UINT OnNcHitTest(CPoint);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS);
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnSetButtonSize(WPARAM, LPARAM);
	afx_msg LRESULT OnSetBitmapSize(WPARAM, LPARAM);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// Styles for toolbar buttons
#define TBBS_BUTTON     MAKELONG(TBSTYLE_BUTTON, 0) // this entry is button
#define TBBS_SEPARATOR  MAKELONG(TBSTYLE_SEP, 0)    // this entry is a separator
#define TBBS_CHECKBOX   MAKELONG(TBSTYLE_CHECK, 0)  // this is an auto check button
#define TBBS_GROUP      MAKELONG(TBSTYLE_GROUP, 0)  // marks the start of a group
#define TBBS_CHECKGROUP (TBBS_GROUP|TBBS_CHECKBOX)  // normal use of TBBS_GROUP

// styles for display states
#define TBBS_CHECKED    MAKELONG(0, TBSTATE_CHECKED)    // button is checked/down
#define TBBS_PRESSED    MAKELONG(0, TBSTATE_PRESSED)    // button is being depressed
#define TBBS_DISABLED   MAKELONG(0, TBSTATE_ENABLED)    // button is disabled
#define TBBS_INDETERMINATE  MAKELONG(0, TBSTATE_INDETERMINATE)  // third state
#define TBBS_HIDDEN     MAKELONG(0, TBSTATE_HIDDEN) // button is hidden
#define TBBS_WRAPPED    MAKELONG(0, TBSTATE_WRAP)   // button is wrapped at this point

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SBARTOOL_H__

#else // WIN32

#define SECToolBar CToolBar

#endif // WIN32