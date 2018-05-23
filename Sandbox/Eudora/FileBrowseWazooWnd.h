#ifndef __FILEBROWSEWAZOOWND_H__
#define __FILEBROWSEWAZOOWND_H__

// FileBrowseWazooWnd.h : header file
//
// CFileBrowseWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"

class CFileBrowseView;

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseWazooWnd window

class CFileBrowseWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CFileBrowseWazooWnd)

public:
	CFileBrowseWazooWnd();
	virtual ~CFileBrowseWazooWnd();

	CFileBrowseView*	m_pFileBrowseView;	// MFC expects View objects to be created on heap

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBrowseWazooWnd)
public:
	//}}AFX_VIRTUAL
	virtual void OnActivateWazoo();

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CFileBrowseWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCmdRefresh();
	afx_msg void OnCmdUpdateRefresh(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif //__FILEBROWSEWAZOOWND_H__
