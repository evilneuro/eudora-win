// WizardClientPage.h

#ifndef _WIZARDCLIENTPAGE_H_
#define _WIZARDCLIENTPAGE_H_

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

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////////
/**** If multiple email clients are configured ***/

class CWizardClientPage : public CPropertyPage
{
// Construction
public:
	CWizardClientPage( CWizardPropSheet*);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWizardClientPage)
	enum { IDD = IDD_DIALCLIENTS };
	CStatic	m_TitleStatic;
	CButton m_NewRadio;
	CButton m_ImportRadio;
	CButton m_AcapRadio;
	CButton m_AdvancedRadio;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardClientPage)
	public:
	virtual BOOL OnKillActive();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	private:
		CWizardPropSheet *m_pParent;

// Implementation
protected:
	void SetupBtns();
	CImportMail *m_pImportMail;

	// Generated message map functions
	//{{AFX_MSG(CWizardClientPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioNew();
	afx_msg void OnRadioOld();
	afx_msg void OnRadioAcap();
	afx_msg void OnAdvancedRadio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // _WIZARDCLIENTPAGE_H_
