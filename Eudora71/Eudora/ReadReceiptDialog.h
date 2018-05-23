#ifndef READRECEIPTDIALOG_H
#define READRECEIPTDIALOG_H


/////////////////////////////////////////////////////////////////////////////
// CReadReceiptDlg dialog

class CReadReceiptDlg : public CDialog
{
// Construction
public:
	CReadReceiptDlg(
	LPCSTR	szTitle,
	CWnd*	pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReadReceiptDlg)
	enum { IDD = IDD_READ_RECEIPT };
	CString	m_sReadRecptStatic;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReadReceiptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReadReceiptDlg)
	afx_msg void OnRrNever(); 
	afx_msg void OnRrLater();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
