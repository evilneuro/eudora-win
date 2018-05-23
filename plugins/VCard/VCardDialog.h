#if !defined(AFX_VCARDDIALOG_H__D2125129_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
#define AFX_VCARDDIALOG_H__D2125129_752B_11D2_8A0E_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// VCardDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVCardDialog dialog

class CVCardDialog : public CDialog
{
// Construction
public:
	CVCardDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVCardDialog)
	enum { IDD = IDD_VCARD_VIEW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVCardDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVCardDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VCARDDIALOG_H__D2125129_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
