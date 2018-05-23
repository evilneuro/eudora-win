// addrdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddressDlg dialog

class CAddressDlg : public CDialog
{
// Construction
public:
	CAddressDlg(const CString& dlgCaption, ULONG editFieldCode, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAddressDlg)
	enum { IDD = IDD_ADDRESS };
	CString	m_Bcc;
	CString	m_Cc;
	CString	m_To;
	//}}AFX_DATA

// Implementation
private:
	int GetSelectedNicknames(CStringList& selectedNicknames);
	BOOL ExpandNicknames(CStringList& theNicknames);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	enum EditFieldCode
	{
		EDIT_BROWSE_ONLY,
		EDIT_TO,
		EDIT_TO_CC,
		EDIT_TO_CC_BCC
	};

	CString m_DlgCaption;
	EditFieldCode m_EditFieldCode;

	// Generated message map functions
	//{{AFX_MSG(CAddressDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBccButton();
	afx_msg void OnCcButton();
	afx_msg void OnToButton();
	afx_msg void SetControlStates();
	afx_msg void OnDblClickNicknames();
	afx_msg void SetOKButtonState();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
