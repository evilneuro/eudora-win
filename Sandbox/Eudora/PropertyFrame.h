// PropertyFrame.h : header file
//
// This file contains the mini-frame that controls modeless 
// property sheet CPropertyPanel.

#ifndef __PROPERTYFRAME_H__
#define __PROPERTYFRAME_H__

#include "PropertyPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyFrame frame

class CPropertyFrame : public CMiniFrameWnd
{
	DECLARE_DYNCREATE(CPropertyFrame)
//Construction
public:
	CPropertyFrame(CDocument* pDoc=NULL);

// Attributes
public:
	CPropertyPanel* m_pModelessPropSheet;
	CDocument * m_pDoc;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyFrame();

	// Generated message map functions
	//{{AFX_MSG(CPropertyFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif	// __PROPERTYFRAME_H__
