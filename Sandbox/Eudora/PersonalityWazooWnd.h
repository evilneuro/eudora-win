#ifndef __PERSONALITYWAZOOWND_H__
#define __PERSONALITYWAZOOWND_H__

// PersonalityWazooWnd.h : header file
//
// CPersonalityWazooWnd
// Specific implementation of a CWazooWnd.

#include "WazooWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPersonalityWazooWnd window

class CPersonalityView;

class CPersonalityWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CPersonalityWazooWnd)

public:
	CPersonalityWazooWnd();
	virtual ~CPersonalityWazooWnd();


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPersonalityWazooWnd)
public:
	//}}AFX_VIRTUAL

	// override base class implementation
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CPersonalityWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnInitialUpdate(WPARAM, LPARAM);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CPersonalityView*	m_pPersonalityView;	// MFC expects View objects to be created on heap
};


#endif //__PERSONALITYWAZOOWND_H__
