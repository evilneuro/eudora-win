 
// AboutEMS.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CPlugInListBox window

class CPlugInListBox : public CListBox
{
// Construction
public:
	CPlugInListBox(){};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlugInListBox)
public:
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	//}}AFX_VIRTUAL

public:
	virtual ~CPlugInListBox(){};

protected:
	//{{AFX_MSG(CPlugInListBox)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG


	DECLARE_MESSAGE_MAP()

public:
	void Configure();
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CAboutPlugInDlg dialog

class CAboutPlugInDlg : public CDialog
{
// Construction
public:
	CAboutPlugInDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAboutPlugInDlg)
	enum { IDD = IDD_ABOUT_EMS_PLUGINS };
	CButton			m_Settings;
	CPlugInListBox	m_PlugInList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutPlugInDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAboutPlugInDlg)
	afx_msg void OnSettings();
	afx_msg void OnSelChange();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
