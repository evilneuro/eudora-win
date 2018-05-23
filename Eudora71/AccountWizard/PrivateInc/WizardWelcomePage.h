// WizardWelcomePage.h

#ifndef _WIZARDWELCOMEPAGE_H_
#define _WIZARDWELCOMEPAGE_H_

#include "resource.h"

class CWizardPropSheet;

/////////////////////////////////////////////////////////////////////////
//Welcome Dialog

class CWizardWelcomePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardWelcomePage)

// Construction
public:
	CWizardWelcomePage();
	CWizardWelcomePage( CWizardPropSheet *) ;
	~CWizardWelcomePage();

	CWizardPropSheet *m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardWelcomePage)
	enum { IDD = IDD_DIALWELCOME };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardWelcomePage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWizardWelcomePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDWELCOMEPAGE_H_
