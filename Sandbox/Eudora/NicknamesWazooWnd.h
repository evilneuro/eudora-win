#ifndef __NICKNAMESWAZOOWND_H__
#define __NICKNAMESWAZOOWND_H__

// NicknamesWazooWnd.h : header file
//
// CNicknamesWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "SplitHlp.h"

/////////////////////////////////////////////////////////////////////////////
// CNicknamesWazooWnd window

class CNicknamesWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CNicknamesWazooWnd)

public:
	CNicknamesWazooWnd();
	virtual ~CNicknamesWazooWnd();

	// Override of base class implementation
	virtual void OnDeactivateWazoo();

	void ToggleRHSDisplay();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnActivateWazoo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNicknamesWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CSplitterHelpWnd	m_wndSplitter;

	//
	// Cached width of the RHS pane.
	//
	int m_nRHSWidth;
};


#endif //__NICKNAMESWAZOOWND_H__
