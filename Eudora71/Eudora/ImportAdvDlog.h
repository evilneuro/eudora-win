#if !defined(AFX_IMPORTADVDLOG_H__7B6E802D_6E95_11D2_8A05_00805F9B7487__INCLUDED_)
#define AFX_IMPORTADVDLOG_H__7B6E802D_6E95_11D2_8A05_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ImportAdvDlog.h : header file
//
#include "MAPIImport.h"

/////////////////////////////////////////////////////////////////////////////
// CImportAdvDlog dialog

class CImportAdvDlog : public CDialog
{
// Construction
public:
	CImportAdvDlog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImportAdvDlog)
	enum { IDD = IDD_IMPORT_ADV };
	CListBox	m_ProgramListBox;
	CEdit	m_MailEdit;
	CEdit	m_AddrEdit;
	CButton	m_MailBrowseBtn;
	CButton	m_AddrBrowseBtn;
	//}}AFX_DATA
	CImportMail m_Importer;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportAdvDlog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportAdvDlog)
	afx_msg void OnMailBrowseBtn();
	afx_msg void OnAddrBrowseBtn();
	afx_msg void OnImportAdvCancel();
	afx_msg void OnImportAdvOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeImportProgramList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	//}}AFX_DATA


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTADVDLOG_H__7B6E802D_6E95_11D2_8A05_00805F9B7487__INCLUDED_)
