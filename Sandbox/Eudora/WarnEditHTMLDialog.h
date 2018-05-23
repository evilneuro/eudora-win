// WarnEditHTMLDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWarnEditHTMLDialog dialog

class CWarnEditHTMLDialog : public CDialog
{
// Construction
public:
	CWarnEditHTMLDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWarnEditHTMLDialog)
	enum { IDD = IDD_WARN_EDIT_HTML };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWarnEditHTMLDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWarnEditHTMLDialog)
	afx_msg void OnEditStyledText();
	afx_msg void OnEditHtml();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
