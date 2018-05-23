// WizardInServerPage.h

#ifndef _WIZARDINSERVERPAGE_H_
#define _WIZARDINSERVERPAGE_H_

#include "resource.h"

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////////////

/**** The Incoming server settings ****/

class CWizardInServerPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardInServerPage)

// Construction
public:
	CWizardInServerPage(CWizardPropSheet* parent = NULL) ;
	~CWizardInServerPage();

	CWizardPropSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardInServerPage)
	enum { IDD = IDD_DIALINSERVER };
	CEdit	m_InServerEdit;
	CString	m_strinserver;
	BOOL	m_ValidateServer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardInServerPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bActiveInitialized;
	void SetRadioPOP();
	void SetRadioIMAP();
	// Generated message map functions
	//{{AFX_MSG(CWizardInServerPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditinserver();
	afx_msg void OnRadioIMAP();
	afx_msg void OnRadioPOP();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDINSERVERPAGE_H_
