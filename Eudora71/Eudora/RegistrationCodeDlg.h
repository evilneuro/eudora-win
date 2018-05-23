// RegistrationCodeDlg.h : header file
//

#if !defined(AFX_REGISTRATIONCODEDLG_H__4EB6FF26_6B27_11D3_88CD_0060082B6AF8__INCLUDED_)
#define AFX_REGISTRATIONCODEDLG_H__4EB6FF26_6B27_11D3_88CD_0060082B6AF8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CRegistrationCodeDlg dialog

class CRegistrationCodeDlg : public CDialog
{
// Construction
public:
	CRegistrationCodeDlg(CWnd* pParent = NULL);		// standard constructor

	CRegistrationCodeDlg(							// constructor with reg info and message string IDs specified
		const char *	szFirstName,
		const char *	szLastName,
		const char *	szRegCode,
		UINT			nMessageTitleStringID,
		UINT			nMessageStringID,
		CWnd *			pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CRegistrationCodeDlg)
	enum { IDD = IDD_REGISTRATION_CODE };
	CString	m_FirstName;
	CString	m_LastName;
	CString	m_Code;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegistrationCodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRString	m_MessageTitle;
	CRString	m_Message;

	// Generated message map functions
	//{{AFX_MSG(CRegistrationCodeDlg)
	virtual void OnOK();
	afx_msg void OnLostCode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGISTRATIONCODEDLG_H__4EB6FF26_6B27_11D3_88CD_0060082B6AF8__INCLUDED_)
