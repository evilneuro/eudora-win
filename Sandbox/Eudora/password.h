// PASSWORD.HPP
//
// Routines for changing the password
//


#ifndef _PASSWORD_H_
#define _PASSWORD_H_

#include "helpxdlg.h"

// Data Items
extern CString POPPassword;


// Function Prototypes
void ClearPassword();
BOOL GetPassword(CString& NewPassword, UINT Prompt = 0, BOOL ChangePass = FALSE, const char* Login = NULL);
BOOL GetPasswordMP(CString& NewPassword, CString& Persona, UINT Prompt = 0);
BOOL ChangePassword();
BOOL ForgetPasswordByDialog();

//const char* EncodePassword(const char* ClearText);
//const char* DecodePassword(const char* EncodedText);

CString EncodePassword(const char* ClearText);
CString DecodePassword(const char* EncodedText);


/////////////////////////////////////////////////////////////////////////////
// CGetPasswordMP dialog

class CGetPasswordMP : public CHelpxDlg
{
// Construction
public:
	CGetPasswordMP(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGetPasswordMP)
	enum { IDD = IDD_GET_PASSWORD_MP };
	CComboBox	m_PersonaCombo;
	CString	m_Password;
	CString	m_Persona;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetPasswordMP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FillInPersonalities(void );

	// Generated message map functions
	//{{AFX_MSG(CGetPasswordMP)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CForgetPassword dialog

class CForgetPassword : public CHelpxDlg
{
// Construction
public:
	CForgetPassword(CWnd* pParent = NULL);   // standard constructor
	~CForgetPassword();
	LPSTR GetPersonas(){ return m_lpNameList; }
// Dialog Data
	//{{AFX_DATA(CForgetPassword)
	enum { IDD = IDD_FORGET_PASSWORD };
	CListBox	m_PersonaList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CForgetPassword)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	LPSTR	m_lpNameList; 
	void FillInPersonalities(void );

	// Generated message map functions
	//{{AFX_MSG(CForgetPassword)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
