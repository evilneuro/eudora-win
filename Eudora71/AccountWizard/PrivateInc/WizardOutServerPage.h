// WizardOutServerPage.h

#ifndef _WIZARDOUTSERVERPAGE_H_
#define _WIZARDOUTSERVERPAGE_H_

#include "resource.h"

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////////////

/**** The Outgoing server settings ****/

class CWizardOutServerPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardOutServerPage)

// Construction
public:
	CWizardOutServerPage( CWizardPropSheet* parent = NULL) ;
	~CWizardOutServerPage();

	CWizardPropSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardOutServerPage)
	enum { IDD = IDD_DIALOUTSERVER };
	CString	m_stroutserver;
	BOOL	m_bAllowSMTPAuth;
	BOOL	m_ValidateServer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardOutServerPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//this holds the value if the dialog has been initialized already or not
	//this helps us preventing initializing the dialog with the INI values only
	//for the first time the dialog is displayed
	bool m_bActiveInitialized;
	// Generated message map functions
	//{{AFX_MSG(CWizardOutServerPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditoutserver();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDOUTSERVERPAGE_H_
