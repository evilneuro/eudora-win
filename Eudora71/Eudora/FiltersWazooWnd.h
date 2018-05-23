#ifndef __FILTERSWAZOOWND_H__
#define __FILTERSWAZOOWND_H__

// FiltersWazooWnd.h : header file
//
// CFiltersWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "SplitHlp.h"

/////////////////////////////////////////////////////////////////////////////
// CFiltersWazooWnd window

class CFiltersWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CFiltersWazooWnd)

public:
	CFiltersWazooWnd();
	virtual ~CFiltersWazooWnd();

	// Override of base class implementation
	virtual void OnDeactivateWazoo();
	virtual void OnActivateWazoo();

	CSplitterHelpWnd	m_wndSplitter;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFiltersWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	// Generated message map functions
protected:
	//{{AFX_MSG(CFiltersWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	//}}AFX_MSG

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


#endif //__FILTERSWAZOOWND_H__
