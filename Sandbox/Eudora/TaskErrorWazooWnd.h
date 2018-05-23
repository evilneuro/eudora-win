// TaskErrorWazooWnd.h : header file
//

#ifndef _TASKERRORWAZOOWND_H_
#define _TASKERRORWAZOOWND_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "WazooWnd.h"

class CTaskErrorView;

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorWazooWnd window

class CTaskErrorWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CTaskErrorWazooWnd)

// Construction
public:
	CTaskErrorWazooWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskErrorWazooWnd)
	//}}AFX_VIRTUAL
	virtual void OnActivateWazoo();

// Implementation
public:
	virtual ~CTaskErrorWazooWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTaskErrorWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CTaskErrorView* m_pTaskErrorView;
};

/////////////////////////////////////////////////////////////////////////////

#endif // _TASKERRORWAZOOWND_H_
