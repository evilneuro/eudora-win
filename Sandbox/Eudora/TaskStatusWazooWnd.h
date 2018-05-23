#if !defined(AFX_TASKSTATUSWAZOOWND_H__0BCF1F02_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_)
#define AFX_TASKSTATUSWAZOOWND_H__0BCF1F02_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TaskStatusWazooWnd.h : header file
//

#include "WazooWnd.h"

class CTaskStatusView;

/////////////////////////////////////////////////////////////////////////////
// CTaskStatusWazooWnd window

class CTaskStatusWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CTaskStatusWazooWnd)

// Construction
public:
	CTaskStatusWazooWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskStatusWazooWnd)
	//}}AFX_VIRTUAL
	virtual void OnActivateWazoo();

// Implementation
public:
	virtual ~CTaskStatusWazooWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTaskStatusWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CTaskStatusView* m_pTaskStatusView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKSTATUSWAZOOWND_H__0BCF1F02_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_)
