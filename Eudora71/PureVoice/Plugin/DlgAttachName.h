// DlgAttachName.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAttachName dialog

class CDlgAttachName : public CDialog
{
// Construction
public:
	CDlgAttachName(CString& attachName, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CDlgAttachName)
	enum { IDD = IDD_ATTACH_NAME };
	CString	m_AttachName;
	//}}AFX_DATA
	
	CString m_FinalAttachName;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAttachName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Attributes
private:
	CString	m_OrigName;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAttachName)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
