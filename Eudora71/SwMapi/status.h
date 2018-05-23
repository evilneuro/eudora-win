// status.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog

class CStatusDlg : public CDialog
{
// Construction
public:
	CStatusDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CStatusDlg)
	enum { IDD = IDD_INFO };
	CString	m_Description16;
	CString	m_Description32;
	CString	m_Company16;
	CString	m_Company32;
	CString	m_Directory;
	CString	m_DllName16;
	CString	m_DllName32;
	CString	m_Version16;
	CString	m_Version32;
	CString	m_InUse16;
	CString	m_InUse32;
	CString	m_FileInfo16;
	CString	m_FileInfo32;
	//}}AFX_DATA

private:
	CString GetFileInfo(const CString& fileName);
	void GetVersionInfo
	(
		const CString& fileName,	//(i) pathname to the file
		CString& companyName,		//(o) company name
		CString& fileVersion,		//(o) file version number
		CString& fileDescription	//(o) file description
	);
	BOOL FileExists(const CString& fileName) const;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CStatusDlg)
	afx_msg void OnRadioButtonClicked();
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
