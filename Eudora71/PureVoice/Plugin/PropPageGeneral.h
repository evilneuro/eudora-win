// PropPageGeneral.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropPageGeneral dialog

class CPropPageGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageGeneral)

// Construction
public:
	CPropPageGeneral();
	~CPropPageGeneral();

	CString m_strOutboxDir;			// out-box directory

// Dialog Data
	//{{AFX_DATA(CPropPageGeneral)
	enum { IDD = IDD_PROPPAGE_SMALL };
	BOOL	mb_ImmediateRecord;
	BOOL	mb_AttachReadMe;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageGeneral)
	afx_msg void OnOutboxDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
