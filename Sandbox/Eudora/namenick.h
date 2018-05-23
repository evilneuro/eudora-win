// NAMENICK.H
//
// Code for handling the Nickname naming dialog, and Make Nickname


/////////////////////////////////////////////////////////////////////////////
// CNameNickDialog dialog

class CNicknameFile;
class CNickname;

class CNameNickDialog : public CHelpxDlg
{
// Construction
public:
	CNameNickDialog(UINT Type,
					CNicknameFile* pSelectedNicknameFile = NULL,
					CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CNameNickDialog)
	enum { IDD = IDD_NICKNAME_NAME };
	CComboBox	m_Filename;
	CString	m_NewName;
	BOOL	m_PutOnRecipientList;
	BOOL	m_CreateNicknameFile;
	//}}AFX_DATA
	
	UINT			m_Type;
	CNicknameFile*	m_pNicknameFile;
	BOOL			m_MakeNickAdd;

// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void OnOK();
    
	// Generated message map functions
	//{{AFX_MSG(CNameNickDialog)
	afx_msg BOOL OnInitDialog();
	afx_msg void OnMakeFileClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CNicknameFile*	m_pSelectedNicknameFile;
};


/////////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg dialog

class CChooseNicknameFileDlg : public CHelpxDlg
{
// Construction
public:
	CChooseNicknameFileDlg(CNicknameFile* pSourceNicknameFile = NULL,
							CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CNameNickDialog)
	enum { IDD = IDD_NICKNAME_CHOOSE_FILE };
	CListBox	m_FileListBox;
	//}}AFX_DATA
	
	//
	// Caller uses this to determine which nickname file was chosen
	// by the user.
	//
	CNicknameFile* GetTargetNicknameFile() const
		{ return m_pTargetNicknameFile; }

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnOK();
    
	// Generated message map functions
	//{{AFX_MSG(CChooseNicknameFileDlg)
	afx_msg BOOL OnInitDialog();
	afx_msg void OnFileListBoxDblClk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	//
	// The 'pSourceNicknameFile' gives us a hint which nickname file
	// contains the singly-selected "source" nickname.  That way, we
	// can inhibit the display of the source nickname file -- i.e., it
	// doesn't make sense to list the source nickname file as a
	// choosable target.
	//
	CNicknameFile*	m_pSourceNicknameFile;		// single-selected source nickname file
	CNicknameFile*	m_pTargetNicknameFile;		// target nickname file chosen by user
};

// Function Prototypes

BOOL MakeNickname();

BOOL NicknameFileExists(const CString& nickFilename);
BOOL CreateNewNicknameFile(const CString& newFilename);
