////////////////////////////////////////////////////////////////////////
// File: workbook.h
//
// These are overrides of the standard SECWorkbook and SECWorksheet 
// classes which implement some Eudora-specific behavior.
//
// Copyright (c) 1996-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//
////////////////////////////////////////////////////////////////////////

#ifndef __WORKBOOK_H__
#define __WORKBOOK_H__

///////////////////////////////////////////////////////////////////////
// QCWorksheet
//
// Override of SECWorksheet implementation to implement some custom
// command handlers and trap window activations for updating the 
// Auto-Wazoo MDI task bar.
///////////////////////////////////////////////////////////////////////
class QCWorksheet : public SECWorksheet
{
	friend class QCWorkbook;
	DECLARE_DYNCREATE(QCWorksheet);

// Construction
public:
	QCWorksheet();

// Implementation
public:
    virtual ~QCWorksheet();

	// Non-virtual override of MFC 7.1's initial buggy implementation of
	// destroying dock bars. Must be called from a QCWorksheet (or subclass)
	// method in order to work correctly.
	void DestroyDockBars();

// Generated message map functions
protected:
    //{{AFX_MSG(QCWorksheet)
	afx_msg void OnCmdMdiRestore()
		{ PostMessage(WM_SYSCOMMAND, SC_RESTORE, 0); }
	afx_msg void OnCmdMdiMinimize()
		{ PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); }
	afx_msg void OnCmdMdiMaximize()
		{ PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); }
	afx_msg void OnCmdMdiClose()
		{ PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0); }
	afx_msg void OnCmdUpdateMdiRestore(CCmdUI* pCmdUI);
	afx_msg void OnCmdUpdateMdiMinimize(CCmdUI* pCmdUI);
	afx_msg void OnCmdUpdateMdiMaximize(CCmdUI* pCmdUI);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP();
};


///////////////////////////////////////////////////////////////////////
// QCControlBarWorksheet
//
// Enhance default SEC implementation of SECControlBarWorksheet to get
// some the MDI child window behaviors that we need for the QCWorkbook
// implementation.  Unfortunately, this is largely a parallel implementation
// of all the same stuff from QCWorksheet.
///////////////////////////////////////////////////////////////////////
class QCControlBarWorksheet : public SECControlBarWorksheet
{
	DECLARE_DYNCREATE(QCControlBarWorksheet)

// Constructors
protected:
	QCControlBarWorksheet();           // protected constructor used by dynamic creation

public:
	void QCSetIcon(HICON hIcon);

	BOOL m_bFirstActivationAfterClose;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QCControlBarWorksheet)
protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~QCControlBarWorksheet();
	virtual void RecalcLayout(BOOL bNotify = TRUE);

	// Generated message map functions
	//{{AFX_MSG(QCControlBarWorksheet)
	afx_msg void OnCmdMdiRestore()
		{ PostMessage(WM_SYSCOMMAND, SC_RESTORE, 0); }
	afx_msg void OnCmdMdiMinimize()
		{ PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); }
	afx_msg void OnCmdMdiMaximize()
		{ PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); }
	afx_msg void OnCmdMdiClose()
		{ PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0); }
	afx_msg void OnCmdUpdateMdiRestore(CCmdUI* pCmdUI);
	afx_msg void OnCmdUpdateMdiMinimize(CCmdUI* pCmdUI);
	afx_msg void OnCmdUpdateMdiMaximize(CCmdUI* pCmdUI);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


///////////////////////////////////////////////////////////////////////
// QCMiniDockFrameWnd
//
// Override annoying behavior of SECMiniDockFrameWnd RecalcLayout()
// methods which constantly reset the window frame titles.
///////////////////////////////////////////////////////////////////////
class QCMiniDockFrameWnd : public SECMiniDockFrameWnd
{
	DECLARE_DYNCREATE(QCMiniDockFrameWnd)

// Constructors
protected:
	QCMiniDockFrameWnd();           // protected constructor used by dynamic creation

public:

// Attributes
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QCMiniDockFrameWnd)
protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~QCMiniDockFrameWnd();
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual void RecalcLayout(CPoint point, BOOL bNotify = TRUE);

	// Generated message map functions
	//{{AFX_MSG(QCMiniDockFrameWnd)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


///////////////////////////////////////////////////////////////////////
// QCWorkbook
//
// Extension of standard SECWorkbook to get the MDI task bar and
// drag and drop behaviors that we want.
///////////////////////////////////////////////////////////////////////
class QCWorkbook : public SECWorkbook
{
	DECLARE_DYNCREATE(QCWorkbook);

// Construction
public:
	QCWorkbook();

// Attributes
public:

// Implementation
protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECWorkbook)
public:
	virtual BOOL CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu);
	//}}AFX_VIRTUAL
    
public:
	~QCWorkbook();

	void ShowMDITaskBar(BOOL bShow);
	BOOL InitMDITaskBarTooltips();
	void ShutdownMDITaskBarTooltips();

	void QCGetTabRect(SECWorksheet* pSheet, CRect& rectTab);
	void QCGetTaskBarRect(CRect& rectTaskBar) const;
	void QCUpdateTab(SECWorksheet* pSheet);

	void ResetTaskBar();

// Overridables
protected:

	//
	// Override default implementations in base class.
	//
	virtual void AddSheet(SECWorksheet* pSheet);
	virtual void RemoveSheet(SECWorksheet* pSheet);
	virtual void GetTabPts(SECWorksheet* pSheet, CPoint*& pts, int& count);
	virtual void OnDrawTab(CDC* pDC, SECWorksheet* pSheet);
	virtual void OnDrawTabIconAndLabel(CDC* pDC, SECWorksheet* pSheet);
	virtual void OnDrawBorder(CDC* pDC);
	virtual int recalcTabWidth();	// they weren't expect us to override this!  SD 12/22/99

	virtual void LogoClicked();
	virtual CString GetLogoMessageText();

	//
	// Hook mouse messages for tooltip processing.
	//
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	//
	// Custom tooltip support.
	//
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Operations
public:

// Queries
public:

// Generated message map functions
protected:
	//{{AFX_MSG(QCWorkbook)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTabSelect(WPARAM wParam, LPARAM lParam);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();

	void GetIconAndTextPositions(SECWorksheet* pSheet, CPoint& ptIcon, CRect& rectText);
	BOOL IsTabLabelTruncated(SECWorksheet* pSheet);
	void QCInvalidateAllTabs();
	void RecalcToolTipRects();
	SECWorksheet* TabHitTest(const CPoint& ptClient, BOOL bWantActive = TRUE);
	BOOL IsOnMDIClientAreaBlankSpot(const CPoint& ptClient);

	CFont	m_fontTabBold;		// The font to use for bold tab labels
	CPen	m_penHilight;		// For drawing tab "buttons"
	HWND	m_hWndTooltip;		// Tooltip control for tab "buttons"
	
	int		m_lastVisCount;		// How many tabs were visible last time
	int		CountVisibleTabs();	// How many tabs are visible now

	//
	// Task Bar bitmap support.
	//
	CBitmap m_LogoBitmap;
	CBitmap m_MaskBitmap;
	CDC m_dcLogo;				// compatible Memory DC for storing logo bitmap bits
	CDC m_dcMask;				// compatible Memory DC for storing mask bitmap bits
	HBITMAP m_hBmpLogoOld;		// handle of old bitmap from Logo Memory DC
	HBITMAP m_hBmpMaskOld;		// handle of old bitmap from Mask Memory DC
	int m_nLogoBitmapWidth;
	int m_nLogoBitmapHeight;

	// DRW - Made this virtual so subclasses can override and do something different
	// with the logo.
	virtual BOOL CalcLogoTopLeft(CPoint* TopLeft, CPoint* pIntersectPoint = NULL);
};



#endif // __WORKBOOK_H__
