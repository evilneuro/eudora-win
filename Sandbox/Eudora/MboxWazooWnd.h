#ifndef __MBOXWAZOOWND_H__
#define __MBOXWAZOOWND_H__

// MboxWazooWnd.h : header file
//
// CMboxWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "mboxtree.h"

/////////////////////////////////////////////////////////////////////////////
// CMboxWazooWnd window

class CContainerView;

class CMboxWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CMboxWazooWnd)

public:
	CMboxWazooWnd();
	virtual ~CMboxWazooWnd();

	// override base class implementation
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMboxWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CMboxWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnInitialUpdate(WPARAM, LPARAM);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CContainerView*	m_pMailboxesView;
	CMboxTreeCtrl	m_MboxTreeCtrl;
};


#endif //__MBOXWAZOOWND_H__
