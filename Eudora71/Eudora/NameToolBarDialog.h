#if !defined(AFX_NAMETOOLBARDIALOG_H__74A8B311_75A7_11D1_A828_0060972F7459__INCLUDED_)
#define AFX_NAMETOOLBARDIALOG_H__74A8B311_75A7_11D1_A828_0060972F7459__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NameToolBarDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNameToolBarDialog dialog

class CNameToolBarDialog : public CDialog
{
// Construction
public:
	CNameToolBarDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNameToolBarDialog)
	enum { IDD = IDD_NAME_TOOLBAR };
	CString	m_szName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNameToolBarDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNameToolBarDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NAMETOOLBARDIALOG_H__74A8B311_75A7_11D1_A828_0060972F7459__INCLUDED_)
