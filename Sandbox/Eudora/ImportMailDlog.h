#if !defined(AFX_IMPORTMAILDLOG_H__7B6E802D_6E95_11D2_8A05_00805F9B7487__INCLUDED_)
#define AFX_IMPORTMAILDLOG_H__7B6E802D_6E95_11D2_8A05_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ImportMailDlog.h : header file
//
#include "TreeCtrlCheck.h"
#include "MAPIImport.h"

//class CImportMail;
/////////////////////////////////////////////////////////////////////////////
// CImportMailDlog dialog

class CImportMailDlog : public CDialog
{
// Construction
public:
	CImportMailDlog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImportMailDlog)
	enum { IDD = IDD_IMPORT_MAIL };
	CButton	m_AddressCheck;
	CButton	m_MailCheck;
	CButton	m_LDIFCheck;
	CEdit	m_LDIFEditCon;
	CTreeCtrlCheck	m_Tree;
	CButton	m_BrowseBtn;
	//}}AFX_DATA
	CImportMail *m_pImporter;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportMailDlog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportMailDlog)
	afx_msg void OnBrowseBtn();
	afx_msg void OnImportMailCancel();
	afx_msg void OnImportMailOk();
	virtual BOOL OnInitDialog();
	afx_msg LONG OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLdifCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	bool InitTree();
	void SetupControls();

	//}}AFX_DATA


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTMAILDLOG_H__7B6E802D_6E95_11D2_8A05_00805F9B7487__INCLUDED_)
