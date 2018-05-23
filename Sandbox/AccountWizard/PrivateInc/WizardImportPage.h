#ifndef _WIZARDIMPORTPAGE_H_
#define _WIZARDIMPORTPAGE_H_

#include "NewIfaceMT.h"

#include "resource.h"
#include "TreeCtrlCheck.h"
#include "MAPIImport.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

// --------------------------------------------------------------------------

class CWizardPropSheet;

// --------------------------------------------------------------------------

class CWizardImportPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardImportPage)

// Construction
public:
	CWizardImportPage(CWizardPropSheet *parent = NULL);
	~CWizardImportPage();

// Dialog Data
	//{{AFX_DATA(CWizardImportPage)
	enum { IDD = IDD_DIALIMPORT };
	CButton	m_MailChk;
	CTreeCtrlCheck m_ImportTree;
	CStatic	m_TitleStatic;
	CStatic	m_AlsoStatic;
	CButton	m_AddrChk;
	CButton	m_LDIFChk;
	CEdit m_LDIFEdt;
	CButton m_BrowseBtn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardImportPage)
	public:
	virtual BOOL OnKillActive();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateNextBackBtns();
	bool CopySettings();
	void SetupControls();
	bool m_bInitializedTree;
	bool InitTree();
	CWizardPropSheet *m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CWizardImportPage)
	virtual BOOL OnInitDialog();
	afx_msg LONG OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLdifCheck();
	afx_msg void OnBrowseBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _WIZARDIMPORTPAGE_H_
