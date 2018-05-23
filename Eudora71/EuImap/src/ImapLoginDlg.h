// ImapLoginDlg.h : header file
//
#ifndef __IMAPDLG_H
#define __IMAPDLG_H

// IMAP defines
#define		MAX_IMAP_LOGIN		20
#define		MAX_IMAP_PASSWORD	20

// For account dialog.
#define MAX_IMAP_FULLNAME		40
#define MAX_IMAP_ACCOUNT		128
#define MAX_IMAP_PORTNUM		20
#define MAX_IMAP_PREFIX			256


/////////////////////////////////////////////////////////////////////////////
// CImapLoginDlg dialog

class CImapLoginDlg : public CDialog
{
// Construction
public:
	CImapLoginDlg(LPCSTR pServer, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImapLoginDlg)
	enum { IDD = IDD_IMAP_LOGIN };
	CString	m_Login;
	CString	m_Password;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImapLoginDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImapLoginDlg)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Initialization:
public:
	void	SetInitialValues (LPCSTR pLogin, LPCSTR pPassword)
		{ if (pLogin)		m_Login = pLogin;
		  if (pPassword)	m_Password = pPassword; }

	void GetResults (CString &Login, CString &Password)
		{ Login = m_Login; Password = m_Password; }

private:
	CString m_Server;

};



#endif // __IMAPDLG_H

