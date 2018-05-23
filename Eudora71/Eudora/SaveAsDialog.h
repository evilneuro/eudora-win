#ifndef SAVEASDIALOG_H
#define SAVEASDIALOG_H

class CSaveAsDialog : public CFileDialog
{
	CString	m_szStatDir;
	
// Construction
public:
	CSaveAsDialog(const char* Filename, BOOL IsMessage, BOOL IsStationery,
		const char* DefaultExtension, const char* FileFilter, CWnd* pParent);
		
// Operations
public:
	BOOL OnInitDialog();
	virtual void OnTypeChange();


// Implementation
protected:
	//{{AFX_MSG(CSaveAsDialog)
#ifdef WIN32
	afx_msg void ToggleInc() {m_Inc = !m_Inc;}
	afx_msg void ToggleGuess() {m_Guess = !m_Guess;}
#endif // WIN32
	afx_msg void ToggleStat(); 
	afx_msg void StatDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
	BOOL	m_IsMessage;
	CButton	m_IncludeHeaders, m_GuessParagraphs, m_Stationery;

public:
#ifdef WIN32
	//
	// Under Win32 and the Version 4 shell, the OnOK() method never
	// gets called for some reason.  Therefore, we need to keep track
	// of the toggle state of the Include Headers and Guess Paragraphs
	// checkboxes ourselves.  Whatta hack.
	// 
	BOOL m_Inc, m_Guess;		// These tell which selection is made
#endif
	BOOL m_IsStat;				// Be careful...two meanings
								//	1) How to init view
								//  2) after init whether it's in stationery mode  
	BOOL m_ChangingDir;

protected: 
	virtual void OnOK();
	void GetFileNameFromDialog(char *buf, int bufLen);
	void SetFileNameInDialog(const char *buf);
};
                                              
#endif
