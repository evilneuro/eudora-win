#if !defined(AFX_PGDOCUMENTFRAME_H__35E93283_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
#define AFX_PGDOCUMENTFRAME_H__35E93283_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PgDocumentFrame.h : header file
//

#include "mdichild.h"

class QCChildToolBar;
class SECToolBarManager;

/////////////////////////////////////////////////////////////////////////////
// PgDocumentFrame frame

class PgDocumentFrame : public CMDIChild, public QCProtocol
{

	QCChildToolBar*		m_pFormattingToolBar;
	SECToolBarManager*	m_pToolBarManager;

	DECLARE_DYNCREATE(PgDocumentFrame)
protected:
	PgDocumentFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PgDocumentFrame)
	//}}AFX_VIRTUAL
	QCChildToolBar* GetFormatToolbar(); 

// Implementation
protected:
	virtual ~PgDocumentFrame();

	// Generated message map functions
	//{{AFX_MSG(PgDocumentFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	afx_msg void OnCheckSpelling();
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGDOCUMENTFRAME_H__35E93283_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
