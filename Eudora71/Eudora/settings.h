// SETTINGS.H
//
// Routines for the Settings dialog
//
class CMemoryMappedFile;

/////////////////////////////////////////////////////////////////////////////
// CSettingsListBox window

class CSettingsListBox : public CListBox
{
// Construction
public:
	CSettingsListBox();

// Operations
public:

// Implementation
public:
	~CSettingsListBox();
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

protected:
	// Generated message map functions
	//{{AFX_MSG(CSettingsListBox)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
protected:
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CLabelColorButton button

class CLabelColorButton : public CButton
{
// Construction
public:
	CLabelColorButton();

// Attributes
public:
	COLORREF m_Color;

// Operations
public:

// Implementation
public:
	~CLabelColorButton();
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	// Generated message map functions
	//{{AFX_MSG(CLabelColorButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog

class CSettingsDialog : public CHelpxDlg
{
// Construction
public:
	CSettingsDialog(CWnd* pParent = NULL);	// standard constructor
	~CSettingsDialog();

// Dialog Data
	//{{AFX_DATA(CSettingsDialog)
	enum { IDD = IDD_SETTINGS };
	CSettingsListBox	m_Categories;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	typedef enum { SERV_POP, SERV_IMAP } ServType;
	BOOL OnInitDialog();
	bool HasActiveTasks(ServType nServ);
	bool SmartSetSel(int NewSel);
	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CSettingsDialog)
	afx_msg void OnSelChange();
	afx_msg void OnStationerySelChange();
	afx_msg void OnRelaySelChange();
	afx_msg void OnAcapButton();
	afx_msg void OnPopServer();
	afx_msg void OnImapServer();
	afx_msg void OnMoodMailCheck();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void SetCategoryHeight();
	void FillInCombo(CComboBox* Combo, UINT IniID, UINT StringID);
	void FillInSignatures( CComboBox* Combo );
	void FillInStationery( CComboBox* Combo, UINT id );
	void FillInPersonalities( CComboBox* Combo );

	int m_nOriginalServType;
	bool m_bInitedServType;


	void ShowServerSpecificControls(ServType nServ);
	void ShowControl(int nID, bool bShow);
	void EnableControl(int nID, bool bShow);
	void EnableMoodControls(BOOL bStatus);
	void VerifyAuthCheck();
	BOOL SelectTrashMailbox();

	static int			m_Sel;
	CPoint				m_CtrlOffset;
	HWND				m_DialogHWND;
	CLabelColorButton	m_LabelColor[7];
	CLabelColorButton	m_GraphColor[3];
	CComboBox*			m_StatCombo;
	CStatic	*			m_Static;
	CString				m_NoDefaultStr;
	CString				m_NoRelayStr;
	bool				m_bOrigPOP ; // Does the dominant account use POP or IMAP (before modification)
	bool				m_bCallImapFunctions ;
	bool				m_bACAPBusy;
	int					m_VoiceIndex;
	CString				m_VoiceGUID;
	bool				m_bHasSpeechEngine;
	HWND				m_hNewMailSound;

private:
	short bHideABookFromVirusBefore;
	
};
