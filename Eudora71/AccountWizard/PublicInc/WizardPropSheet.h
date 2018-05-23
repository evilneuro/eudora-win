// WizardPropSheet.h : header file
//

#if !defined(AFX_WIZARDPROPSHEET_H__F2DE4597_E8E1_11D1_94C7_00805F9BF4D7__INCLUDED_)
#define AFX_WIZARDPROPSHEET_H__F2DE4597_E8E1_11D1_94C7_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

#include "MAPIImport.h"

// --------------------------------------------------------------------------

// Forward declarations for protected member variables.
class CWizardAcapPage;
class CWizardClientPage;
class CWizardConfirmPage;
class CWizardLoginPage;
class CWizardFinishPage;
class CWizardImapDirPage;
class CWizardInServerPage;
class CWizardOutServerPage;
class CWizardPersNamePage;
class CWizardRetAddPage;
class CWizardWelcomePage;
class CWizardYourNamePage;
class CWizardImportPage;

/////////////////////////////////////////////////////////////////////////////
// CWizardPropSheet

class CWizardPropSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CWizardPropSheet)

public:
	CWizardPropSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, bool bDominant = false);
	CWizardPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, bool bDominant = false);
	void StartWizard(); //calls DoModal internally
	virtual ~CWizardPropSheet();

public:
	CFont			m_StatFont; // static font used for page titles
	CWnd*			m_pParentWnd; // the parent window (points to wazoo if creating new personality)
	bool			m_bIsDominant;

	CPersParams *m_pParams; //the current account being processed

	enum { PST_UNKNOWN, PST_NEW, PST_IMPORT, PST_ACAP, PST_ADVANCED } m_SettingType;
	enum { ACH_UNKNOWN, ACH_NO, ACH_YES } m_ConfirmChange; // ACAP confirm?

	CImportMail *m_pImporter;
	
	CImportChild *m_pImportChild;
	bool m_bImportMail, m_bImportAddr, m_bImportLDIF, m_bImportPersona, m_bIsProfile;
	CString m_sImportFileLDIF;

	CString m_OriginalInServer, m_OriginalOutServer;

	//this boolean is set to true if the user chose to get this
	//settings from an ACAP server
	bool m_bIsACAPSettings;

	bool ValidateHost(LPCTSTR pHost, UINT ServiceID);

	// maintain pointers to all possible wizard dialogs
protected:
	CWizardWelcomePage *m_pWelcome;
	CWizardPersNamePage *m_pPersName;
	CWizardClientPage *m_pClientDlg;
	CWizardAcapPage *m_pAcapDlg;
	CWizardConfirmPage *m_pDialConfirm;
	CWizardYourNamePage *m_pYourName;
	CWizardRetAddPage *m_pRetAdd;
	CWizardLoginPage *m_pDialLogin;
	CWizardInServerPage *m_pInServer;
	CWizardOutServerPage *m_pOutServer;
	CWizardFinishPage *m_pFinish;
	CWizardImapDirPage *m_pImapDir;
	CWizardImportPage *m_pImportPage;

	void AddPages(bool);
	void InitializeAll();
	void UpdateWazoo();
	void ClearIniEntries();

	//{{AFX_MSG(CWizardPropSheet)
	virtual void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIZARDPROPSHEET_H__F2DE4597_E8E1_11D1_94C7_00805F9BF4D7__INCLUDED_)
