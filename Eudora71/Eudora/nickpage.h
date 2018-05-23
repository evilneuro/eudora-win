// NICKPAGE.H : header file
//
// Property page classes for the CNicknamesPropSheet property sheet.


#define kABookAddressLabel "addresses"

/////////////////////////////////////////////////////////////////////////////
// CNicknamesPropertyPage dialog
//
// This is an abstract base class which defines a public protocol for
// generically dealing with the controls contained in each derived
// property page.

class CNicknamesPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CNicknamesPropertyPage);
	
// Construction
public:
	CNicknamesPropertyPage(UINT nIDTemplate);
	~CNicknamesPropertyPage() { }

// Dialog Data
	//{{AFX_DATA(CNicknamesPropertyPage)
	//}}AFX_DATA

// Overridables
public:
	virtual void ClearAllControls() = 0;
	virtual void EnableAllControls(BOOL bEnable) = 0;
	virtual void SetEditFont(CFont* pFont) = 0;
	virtual BOOL IsEditControl(int nCtrlID) const = 0;
	virtual BOOL IsEditControlReadOnly(int nCtrlID) const = 0;

	int GetStaticHeight() const { return m_StaticHeight; }

// Overrides

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesPropertyPage)
protected:
	//}}AFX_VIRTUAL

//FORNOW	BOOL m_EditControlHasFocus;
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	CNicknamesPropertyPage() : CPropertyPage(UINT(-1)) { }		// do not use, only for MFC RTTI
	int m_StaticHeight;
	RECT m_rectOldFrame;

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CNicknamesNotesPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CNicknamesNotesPage dialog

class CNicknamesNotesPage : public CNicknamesPropertyPage
{
// Construction
public:
	CNicknamesNotesPage();
	~CNicknamesNotesPage() { }

// Dialog Data
	//{{AFX_DATA(CNicknamesNotesPage)
	enum
	{
#ifdef WIN32
		IDD = IDD_NICKNAME_NOTES_PAGE32
#else
		IDD = IDD_NICKNAME_NOTES_PAGE
#endif // WIN32
	};
	CURLEdit m_NotesEdit;
	//}}AFX_DATA

// Overrides
	virtual void ClearAllControls();
	virtual void EnableAllControls(BOOL bEnable);
	virtual void SetEditFont(CFont* pFont);
	virtual BOOL IsEditControl(int nCtrlID) const;
	virtual BOOL IsEditControlReadOnly(int nCtrlID) const;

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesNotesPage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CNicknamesNotesPage)
		// NOTE: the ClassWizard will add member functions here
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CNicknamesCustomPage dialog

class CNickname;

class CNicknamesCustomPage : public CNicknamesPropertyPage
{
// Construction
public:
	CNicknamesCustomPage(int nDlgTemplateID);
	~CNicknamesCustomPage() { }

	enum
	{
		NUM_FIELDS = 11
	};

	BOOL			 InitPage();
	virtual int		 NumNickNameFields();
	void			 RegisterFields();

	CURLEdit		*CNicknamesCustomPage::GetEditControlByLabel(const char *szLabel);
	virtual void	 PopulateEditControls(CNickname* pNickname);
	BOOL			 SaveModifiedEditControls(CNickname* pNickname);

// Dialog Data
	//{{AFX_DATA(CNicknamesCustomPage)
	//}}AFX_DATA

// Overrides
	virtual void	 ClearAllControls();
	virtual void	 EnableAllControls(BOOL bEnable);
	virtual void	 SetEditFont(CFont* pFont);
	virtual BOOL	 IsEditControl(int nCtrlID) const;
	virtual BOOL	 IsEditControlReadOnly(int nCtrlID) const;

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesCustomPage)
protected:
	virtual void	 DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	 OnInitDialog();
	//}}AFX_VIRTUAL

public:
	int				 m_NumFieldsInUse;
	CStatic			 m_EditLabelArray[NUM_FIELDS];
	CURLEdit		 m_EditArray[NUM_FIELDS];
	CStatic			 m_FieldNameArray[NUM_FIELDS];

// Implementation
protected:
	int				 ComputeStaticHeight() const;

	virtual void	 SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX);
	virtual void	 SizeSpecials(CRect rect, const int MARGIN, int deltaX);
	virtual void	 SizeSpecialField(int fieldNum, CRect rect,
									  const int MARGIN, int deltaX,
									  int leftFactor, int rightFactor);

	// Generated message map functions
	//{{AFX_MSG(CNicknamesCustomPage)
		// NOTE: the ClassWizard will add member functions here
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNicknamesPersonalPage dialog

class CNicknamesPersonalPage : public CNicknamesCustomPage
{
// Construction
public:
	CNicknamesPersonalPage();
	~CNicknamesPersonalPage() { }

// Dialog Data
	//{{AFX_DATA(CNicknamesPersonalPage)
	enum
	{
#ifdef WIN32
		IDD = IDD_NICKNAME_ADDRESS_PAGE32
#else
		IDD = IDD_NICKNAME_ADDRESS_PAGE
#endif // WIN32
	};
	SECBitmapButton m_Swap;
	//}}AFX_DATA

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesPersonalPage)
protected:
	virtual BOOL	 OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// To handle button presses:
	virtual BOOL	 OnCommand(WPARAM wParam, LPARAM lParam);

	// To handle sizing of fields:
	virtual void	 SizeSpecials(CRect rect, const int MARGIN, int deltaX);
	virtual void	 SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX);

	// Generated message map functions
	//{{AFX_MSG(CNicknamesPersonalPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNicknamesHomePage dialog

class CNicknamesWorkPage;

class CNicknamesHomePage : public CNicknamesCustomPage
{
// Construction
public:
	CNicknamesHomePage();
	~CNicknamesHomePage() { }

	CNicknamesWorkPage	*workPage;

	virtual int		 NumNickNameFields();

	virtual void	 PopulateEditControls(CNickname* pNickname);

// Dialog Data
	//{{AFX_DATA(CNicknamesHomePage)
	enum
	{
#ifdef WIN32
		IDD = IDD_NICKNAME_HOME_PAGE32
#else
		IDD = IDD_NICKNAME_HOME_PAGE
#endif // WIN32
	};
	//}}AFX_DATA

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesHomePage)
protected:
	virtual BOOL	 OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// To handle button presses:
	virtual BOOL	 OnCommand(WPARAM wParam, LPARAM lParam);

	// To handle sizing of fields:
	virtual void	 SizeSpecials(CRect rect, const int MARGIN, int deltaX);
	virtual void	 SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX);

	// Generated message map functions
	//{{AFX_MSG(CNicknamesHomePage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNicknamesWorkPage dialog

class CNicknamesWorkPage : public CNicknamesCustomPage
{
// Construction
public:
	CNicknamesWorkPage();
	~CNicknamesWorkPage() { }

	CNicknamesHomePage	*homePage;

	virtual void	 PopulateEditControls(CNickname* pNickname);

// Dialog Data
	//{{AFX_DATA(CNicknamesWorkPage)
	enum
	{
#ifdef WIN32
		IDD = IDD_NICKNAME_WORK_PAGE32
#else
		IDD = IDD_NICKNAME_WORK_PAGE
#endif // WIN32
	};
	//}}AFX_DATA

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesWorkPage)
protected:
	virtual BOOL	 OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// To handle button presses:
	virtual BOOL	 OnCommand(WPARAM wParam, LPARAM lParam);

	// To handle sizing of fields:
	virtual void	 SizeSpecials(CRect rect, const int MARGIN, int deltaX);
	virtual void	 SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX);

	// Generated message map functions
	//{{AFX_MSG(CNicknamesWorkPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
