#if !defined(AFX_SSLCONNECTIONDLG_H__C85AAF34_B450_11D4_B57B_0008C7392A1C__INCLUDED_)
#define AFX_SSLCONNECTIONDLG_H__C85AAF34_B450_11D4_B57B_0008C7392A1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SSLConnectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSSLConnectionDlg dialog
struct ConnectionInfo;
class CSSLConnectionDlg : public CDialog
{
// Construction
public:
	CSSLConnectionDlg(CWnd* pParent = NULL);   // standard constructor
	int Run(const CString & person, const CString& protocol);

// Dialog Data
	//{{AFX_DATA(CSSLConnectionDlg)
	enum { IDD = IDD_DIALOG_SSL_CONNECTION };
	CString m_ServerIP;
	int m_ServerPort;
	CString m_SSLVersion;
	CString m_NegoStatus;
	CString m_KEA;
	CString m_EA;
	CString m_HM;
	CString m_Notes;
	CString m_LastTime;
	CString	m_ServerName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSSLConnectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:
	CString m_Persona;
	CString m_Protocol;
	ConnectionInfo *m_pConnectionInfo;
	// Generated message map functions
	//{{AFX_MSG(CSSLConnectionDlg)
	afx_msg void OnButtonCertinfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SSLCONNECTIONDLG_H__C85AAF34_B450_11D4_B57B_0008C7392A1C__INCLUDED_)
