#if !defined(AFX_SETTINGSDIALOG_H__D212512C_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
#define AFX_SETTINGSDIALOG_H__D212512C_752B_11D2_8A0E_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SettingsDialog.h : header file
//
class CTestpluginApp;

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog

class CSettingsDialog : public CDialog
{
// Construction
public:
	CSettingsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDialog)
	enum { IDD = IDD_SETTINGS };
	CButton	m_AutoDecodeCheck;
	CButton	m_AddToABCheck;
	CComboBox	m_NickCombo;
	BOOL	m_AddToAddressBook;
	BOOL	m_AutoDecodeToAttachDir;
	//}}AFX_DATA

	CTestpluginApp* m_MainApp;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL CSettingsDialog::OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CSettingsDialog)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void WriteControlsToINI();
	void GetControlsFromINI();
};

BOOL GetIniBool(int Res, char* INIFilename);

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDIALOG_H__D212512C_752B_11D2_8A0E_00805F9B7487__INCLUDED_)
