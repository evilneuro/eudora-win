#ifndef __SIGNATUREWAZOOWND_H__
#define __SIGNATUREWAZOOWND_H__

// SignatureWazooWnd.h : header file
//
// CSignatureWazooWnd
// Specific implementation of a CWazooWnd.


#include "WazooWnd.h"
#include "SignatureTree.h"

/////////////////////////////////////////////////////////////////////////////
// CSignatureWazooWnd window

class CContainerView;
class QCSignatureCommand;

class CSignatureWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CSignatureWazooWnd)
	
	QCSignatureCommand*	GetSelected();

public:
	CSignatureWazooWnd();
	virtual ~CSignatureWazooWnd();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSignatureWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CSignatureWazooWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg LRESULT OnInitialUpdate(WPARAM, LPARAM);
	afx_msg void EnableIfSelected(CCmdUI* pCmdUI);
	afx_msg void OnNewSignature();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	DECLARE_MESSAGE_MAP()

	CContainerView*	m_pSignatureView;
	CSignatureTree	m_SignatureTree;
};


#endif //__SIGNATUREWAZOOWND_H__
