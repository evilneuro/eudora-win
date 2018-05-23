#if !defined(AFX_SELECTMBOX_H__8A293951_254E_11D1_9CFE_00805F4A39FE__INCLUDED_)
#define AFX_SELECTMBOX_H__8A293951_254E_11D1_9CFE_00805F4A39FE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SelectMBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectMBox dialog

class CSelectMBox : public CDialog
{
// Construction
public:
	CSelectMBox(CWnd* pParent = NULL, BOOL allowNew = TRUE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectMBox)
	enum { IDD = IDD_SELECT_MBOX };
	QCMailboxTreeCtrl	m_MboxTree;
	CButton	m_OKButton;
	CString	m_PromptText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectMBox)
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();

public:
	CString	m_MailboxPath;
	BOOL	m_bAllowNew;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMBOX_H__8A293951_254E_11D1_9CFE_00805F4A39FE__INCLUDED_)
