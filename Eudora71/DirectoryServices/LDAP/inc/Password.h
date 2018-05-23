// PASSWORD.HPP
//
// Routines for changing the password
//


#ifndef _PASSWORD_H_
#define _PASSWORD_H_


#include <afxpriv.h>
#include <afxwin.h>					// MFC core and standard components
#include <afxext.h>					// MFC extensions (including VB)
#include <afxcmn.h>					// MFC common controls
#include <afxole.h>					// MFC COleDropTarget, etc.

// Function Prototypes
BOOL GetPassword(CString& NewPassword, UINT Prompt = 0, BOOL ChangePass = FALSE, const char* Login = NULL);


/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog dialog

class CPasswordDialog : public CDialog
{
// Construction
public:
	CPasswordDialog(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPasswordDialog)
	enum { IDD = IDD_GET_PASSWORD };
	CStatic	m_Prompt;
	CString	m_Password;
	CString	m_Account;
	//}}AFX_DATA
	int		m_PromptID;

// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CPasswordDialog)
	virtual void OnOK();
	afx_msg BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog dialog





#endif