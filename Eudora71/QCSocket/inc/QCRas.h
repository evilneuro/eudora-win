#include "QCRasConnection.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// QCRasAuthDlg dialog

class AFX_EXT_CLASS QCRasAuthDlg : public CDialog
{
// Construction
public:
	//QCRasAuthDlg(CWnd* pParent = NULL);   // standard constructor
	QCRasAuthDlg(CString strEntry, RASDIALPARAMS *pDialParams, CWnd* pParent=NULL);
	void UpdateInfo();
// Dialog Data
	//{{AFX_DATA(QCRasAuthDlg)
	enum { IDD = IDD_AUTO_CONNECTION };
	//CComboBox	m_ConnectionCombo;
	//CString	m_strConnectionName;
	CString	m_strDomain;
	CString	m_strUserName;
	BOOL	m_bSavePassword;
	CString	m_strPassword;
	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QCRasAuthDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	CString m_strTitle;
	RASDIALPARAMS *m_pDialParams;
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	bool GetSavePassFlag();
	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(QCRasAuthDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// QCRasAuthDlg dialog
