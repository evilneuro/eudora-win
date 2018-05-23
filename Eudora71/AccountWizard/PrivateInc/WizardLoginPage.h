// WizardLoginPage.h

#ifndef _WIZARDLOGINPAGE_H_
#define _WIZARDLOGINPAGE_H_

#include "resource.h"

class CWizardPropSheet ;

///////////////////////////////////////////////////////////////
/*** The persons login name ***/

class CWizardLoginPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardLoginPage)

// Construction
public:
	CWizardLoginPage();
	CWizardLoginPage( CWizardPropSheet *);
	~CWizardLoginPage();

	CWizardPropSheet *m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardLoginPage)
	enum { IDD = IDD_DIALLOGIN };
	CString	m_strlogin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardLoginPage)
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
	// Generated message map functions
	//{{AFX_MSG(CWizardLoginPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditloginname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDLOGINPAGE_H_
