/////////////////////////////////////////////////////////////////////////////
// QCTipOfDayDlg dialog used for Tip Of The Day command

class QCTipOfDayDlg : public SECTipOfDay
{
public:
	QCTipOfDayDlg(const TCHAR * lpszTipFileName,
				  int nTip = 1,
				  BOOL bShowAtStart = 1,
				  CWnd* pParentWnd = NULL);
	~QCTipOfDayDlg();

// Dialog Data
	//{{AFX_DATA(QCTipOfDayDlg)
	//}}AFX_DATA

// Implementation
protected:
	//{{AFX_MSG(QCTipOfDayDlg)
 	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont m_fontLeadIn;
	CFont m_fontTip;
};

