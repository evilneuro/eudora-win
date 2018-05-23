#ifndef __WEBWAZOOWND_H__
#define __WEBWAZOOWND_H__

// WebWazooWnd.h : header file
//
// CWebWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "WebView.h"

/////////////////////////////////////////////////////////////////////////////
// CWebWazooWnd window

class CWebWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CWebWazooWnd)

public:
	CWebWazooWnd();
	virtual ~CWebWazooWnd();

	CWebView*	m_pWebView;	// MFC expects View objects to be created on heap

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebWazooWnd)
public:
	//}}AFX_VIRTUAL
	virtual void OnActivateWazoo();

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CWebWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCmdRefresh();
	afx_msg void OnCmdUpdateRefresh(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif //__WEBWAZOOWND_H__
