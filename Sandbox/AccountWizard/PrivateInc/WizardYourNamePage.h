// WizardYourNamePage.h

#ifndef _WIZARDYOURNAMEPAGE_H_
#define _WIZARDYOURNAMEPAGE_H_

#include "NewIfaceMT.h"

#include "resource.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

class CWizardPropSheet ;

////////////////////////////////////////////////////////////////////
/*** The Real Name of the person ***/

class CWizardYourNamePage : public CPropertyPage
{
// Construction
public:
	CWizardYourNamePage();   // standard constructor
	CWizardYourNamePage(CWizardPropSheet *);

	CWizardPropSheet *m_pParent;

	// Dialog Data
	//{{AFX_DATA(CWizardYourNamePage)
	enum { IDD = IDD_DIALYOURNAME };
	CString	m_stryourname;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardYourNamePage)
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
	//{{AFX_MSG(CWizardYourNamePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditYourname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // _WIZARDYOURNAMEPAGE_H_
