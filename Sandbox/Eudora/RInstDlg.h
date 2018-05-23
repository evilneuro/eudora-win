// RInstDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRemoteInstance dialog

class CRemoteInstance : public CDialog
{
// Construction
public:
	CRemoteInstance(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRemoteInstance)
	enum { IDD = IDD_REMOTE_INSTANCE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteInstance)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	typedef enum 
	{
		TERMINATE_LOCAL,
		TERMINATE_REMOTE,
		IGNORE_REMOTE
	} RESULT;

	RESULT	m_Result;

protected:

	// Generated message map functions
	//{{AFX_MSG(CRemoteInstance)
	afx_msg void OnTerminateLocal();
	afx_msg void OnTerminateRemote();
	afx_msg void OnContinue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
