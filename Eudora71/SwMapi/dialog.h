// dialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSwitchDialog dialog

class CSwitchDialog : public CDialog
{
// Construction
public:
    CSwitchDialog(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    //{{AFX_DATA(CSwitchDialog)
	enum { IDD = IDD_SWITCH };
	//}}AFX_DATA

// Implementation
private:

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CSwitchDialog)
    virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnClickedAbout();
	virtual void OnCancel();
	afx_msg void OnClickedInstall();
	afx_msg void OnClickedUninstall();
	afx_msg void OnClickedInfo();
	afx_msg void OnInstallEudoraAsNetscapeMailHandler();
	afx_msg void OnUnInstallEudoraAsNetscapeMailHandler();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
