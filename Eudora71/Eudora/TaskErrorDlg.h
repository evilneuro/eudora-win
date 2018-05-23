#if !defined(AFX_TASKERRORDLG_H__1D2B525B_E386_11D1_94C5_00805F9BF4D7__INCLUDED_)
#define AFX_TASKERRORDLG_H__1D2B525B_E386_11D1_94C5_00805F9BF4D7__INCLUDED_

#include "stdafx.h"
#include "resource.h"
#include "TaskErrorMT.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TaskErrorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorDlg dialog

class CTaskErrorDlg : public CDialog
{
// Construction
public:
	CTaskErrorDlg(CTaskErrorMT *pErr, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTaskErrorDlg)
	enum { IDD = IDD_TASK_ERR_DLG };
	CStatic	m_TimeStatic;
	CButton	m_OKBtn;
	CButton	m_DetailsBtn;
	CStatic	m_TitleStatic;
	CStatic	m_SecTxtStatic;
	CStatic	m_PersonaStatic;
	CStatic	m_MainTxtStatic;
	CRichEditCtrl	m_ErrorRichEdit;
	CRichEditCtrl	m_PersonaRichEdit;
	CRichEditCtrl	m_TitleRichEdit;
	CRichEditCtrl	m_StatusRichEdit;
	CRichEditCtrl	m_InfoRichEdit;
	CRichEditCtrl	m_TimeRichEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskErrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRect m_InitialDlgRct;
	bool m_bInitialized;
	CTaskErrorMT * m_pError;

	// Generated message map functions
	//{{AFX_MSG(CTaskErrorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDetailsBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKERRORDLG_H__1D2B525B_E386_11D1_94C5_00805F9BF4D7__INCLUDED_)
