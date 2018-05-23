#if !defined(AFX_ADWAZOOWND_H__0BCF1F02_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_)
#define AFX_ADWAZOOWND_H__0BCF1F02_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AdWazooWnd.h : header file
//

#include "WazooWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CAdWazooWnd window

class CAdView;

class CAdWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CAdWazooWnd)

// Construction
public:
	CAdWazooWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdWazooWnd)
	//}}AFX_VIRTUAL
	virtual void OnActivateWazoo();

// Implementation
public:
	virtual ~CAdWazooWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAdWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CAdView* m_pAdView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADWAZOOWND_H__0BCF1F02_A1A6_11D1_92DB_0060082B6AF8__INCLUDED_)
