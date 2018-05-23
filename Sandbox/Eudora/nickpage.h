// NICKPAGE.H : header file
//
// Property page classes for the CNicknamesPropSheet property sheet.

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

	// Generated message map functions
	//{{AFX_MSG(CNicknamesPropertyPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CNicknamesAddressPage dialog

class CNicknamesAddressPage : public CNicknamesPropertyPage
{
// Construction
public:
	CNicknamesAddressPage();
	~CNicknamesAddressPage() { }

// Dialog Data
	//{{AFX_DATA(CNicknamesAddressPage)
	enum
	{
#ifdef WIN32
		IDD = IDD_NICKNAME_ADDRESS_PAGE32
#else
		IDD = IDD_NICKNAME_ADDRESS_PAGE
#endif // WIN32
	};
	CURLEdit m_AddressEdit;
	//}}AFX_DATA


// Overrides
	virtual void ClearAllControls();
	virtual void EnableAllControls(BOOL bEnable);
	virtual void SetEditFont(CFont* pFont);
	virtual BOOL IsEditControl(int nCtrlID) const;
	virtual BOOL IsEditControlReadOnly(int nCtrlID) const;

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesAddressPage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CNicknamesAddressPage)
		// NOTE: the ClassWizard will add member functions here
	afx_msg void OnSize(UINT nType, int cx, int cy);
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
		NUM_FIELDS = 10
	};

	BOOL InitPage();
	void RegisterFields();

	void PopulateEditControls(CNickname* pNickname);
	BOOL SaveModifiedEditControls(CNickname* pNickname);

// Dialog Data
	//{{AFX_DATA(CNicknamesCustomPage)
	//}}AFX_DATA

// Overrides
	virtual void ClearAllControls();
	virtual void EnableAllControls(BOOL bEnable);
	virtual void SetEditFont(CFont* pFont);
	virtual BOOL IsEditControl(int nCtrlID) const;
	virtual BOOL IsEditControlReadOnly(int nCtrlID) const;

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNicknamesCustomPage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_NumFieldsInUse;
	CStatic m_EditLabelArray[NUM_FIELDS];
	CURLEdit m_EditArray[NUM_FIELDS];
	CStatic m_FieldNameArray[NUM_FIELDS];
	// Generated message map functions
	//{{AFX_MSG(CNicknamesCustomPage)
		// NOTE: the ClassWizard will add member functions here
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int ComputeStaticHeight() const;
};

