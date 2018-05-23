#ifndef _MODIFYACCTDLGSH__
#define _MODIFYACCTDLGSH__

#include "resource.h"
#include "helpxdlg.h"
#include "ValidEdit.h" // CValidEdit
#include "PersParams.h"

class CModifyAcctSheet ; //forward reference


/////////////////////////////////////////////////////////////////////////////
// PropCHelpxDlg dialog

// Use this class if you want Context Sensitive Help in a PropertySheet

class CPropHelpxDlg : public CPropertyPage
{
// Construction
public:
	CPropHelpxDlg(UINT nID);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHelpxDlg)
//	enum { IDD = nID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

#ifdef WIN32
	afx_msg long OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg long OnHelp(WPARAM wParam, LPARAM lParam);
#endif
	afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM lParam);
	afx_msg long OnContextMenu(WPARAM wParam, LPARAM lParam);
#ifdef WIN32
	afx_msg long OnRightButton(WPARAM wParam, LPARAM lParam);
#endif

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropHelpxDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
 
};

/////////////////////////////////////////////////////////////////////////
/*** Generic Traits Property Page ************/

class CModifyAcct_Generic : public CPropHelpxDlg
{
// Construction
public:
	CModifyAcct_Generic( CModifyAcctSheet *);   
	CModifyAcctSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CModifyAcct_Generic)
	enum { IDD = IDD_PROPPAGE_PERSONALITY1 };
	CValidEdit	m_PersNameEdit;
	CComboBox	m_comboSignature;
	CComboBox	m_comboStationery;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyAcct_Generic)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive() ;
	virtual void OnOK() ;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL SaveDialogSettings(); // Only 'saves' to the parent's CPersParam block, not to file
	BOOL IsNameAcceptable(bool bShowErr);

protected:
	BOOL LoadDialogSettings(); // Loads from parent's CPersParam block

	void PopulateSignatureCombo();
	void PopulateStationeryCombo();
	
	// Generated message map functions
	//{{AFX_MSG(CModifyAcct_Generic)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePersonaNameEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////
/*** Servers Property Page ************/

class CModifyAcct_Servers : public CPropHelpxDlg
{
// Construction
public:
	CModifyAcct_Servers( CModifyAcctSheet *);   
	CModifyAcctSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CModifyAcct_Servers)
	enum { IDD = IDD_PROPPAGE_PERSONALITY2 };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyAcct_Servers)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive() ;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL SaveDialogSettings(); // Only 'saves' to the parent's CPersParam block, not to file
	bool HasActiveTasks();

protected:
//	void LoadControlSetting(CWnd* pWndControl, UINT uIniID, bool bUseDefault = false) ;
	typedef enum { SERV_POP, SERV_IMAP } ServType;

	CString m_OriginalInServer, m_OriginalOutServer;
	int m_nOriginalServTypeID;
	bool m_bInit;

	void ShowServerSpecificControls(ServType nServ);
	void ShowControl(int nID, bool bShow);
	void EnableControl(int nID, bool bShow);
	void VerifyAuthCheck();

	BOOL LoadDialogSettings(); // Loads from parent's CPersParam block

	// Generated message map functions
	//{{AFX_MSG(CModifyAcct_Servers)
	virtual void OnOK() ;
	virtual BOOL OnInitDialog();
	afx_msg void OnPop();
	afx_msg void OnImap() ;
	afx_msg void OnImapTrashSelect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif _MODIFYACCTDLGSH__
