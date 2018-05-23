// WebBrowserWazooWnd.h : header file
//
// CWebBrowserWazooWnd
// Specific implementation of a CWazooWnd.

#ifdef WEB_BROWSER_WAZOO

#ifndef __WEBBROWSERWAZOOWND_H__
#define __WEBBROWSERWAZOOWND_H__


#include "WazooWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CWebBrowserWazooWnd window

class CWebBrowserView;

class CWebBrowserWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CWebBrowserWazooWnd)

public:
	CWebBrowserWazooWnd();
	virtual ~CWebBrowserWazooWnd();

	CWebBrowserView*	m_pWebBrowserView;	// MFC expects View objects to be created on heap

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebBrowserWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CWebBrowserWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnInitialUpdate(WPARAM, LPARAM);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
#ifdef ADWARE_PROTOTYPE
private:
	void OnTimer(UINT);
#endif
};


#endif //__WEBBROWSERWAZOOWND_H__

#endif //WEB_BROWSER_WAZOO
