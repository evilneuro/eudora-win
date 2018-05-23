#ifndef __FILTERREPORTWAZOOWND_H__
#define __FILTERREPORTWAZOOWND_H__

// FilterReportWazooWnd.h : header file
//
// CFilterReportWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterReportWazooWnd window

class CFilterReportView;

class CFilterReportWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CFilterReportWazooWnd)

public:
	CFilterReportWazooWnd();
	virtual ~CFilterReportWazooWnd();

	CFilterReportView*	m_pFilterReportView;	// MFC expects View objects to be created on heap

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterReportWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CFilterReportWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnInitialUpdate(WPARAM, LPARAM);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif //__FILTERREPORTWAZOOWND_H__
