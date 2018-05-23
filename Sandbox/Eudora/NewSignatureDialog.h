#if !defined(AFX_NEWSIGNATUREDIALOG_H__3FBA2B72_BB31_11D0_97C1_00805FD2F268__INCLUDED_)
#define AFX_NEWSIGNATUREDIALOG_H__3FBA2B72_BB31_11D0_97C1_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NewSignatureDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewSignatureDialog dialog

class CNewSignatureDialog : public CDialog
{
// Construction
public:
	CNewSignatureDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewSignatureDialog)
	enum { IDD = IDD_TOOL_NEW_SIGNATURE };
	CString	m_szName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewSignatureDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation


protected:

	// Generated message map functions
	//{{AFX_MSG(CNewSignatureDialog)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	LPCSTR	GetName() { return m_szName; }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWSIGNATUREDIALOG_H__3FBA2B72_BB31_11D0_97C1_00805FD2F268__INCLUDED_)
