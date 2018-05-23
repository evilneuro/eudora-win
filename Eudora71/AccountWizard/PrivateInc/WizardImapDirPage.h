// WizardImapDirPage.h

#ifndef _WIZARDIMAPDIRPAGE_H_
#define _WIZARDIMAPDIRPAGE_H_

#include "resource.h"

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////

/*** The IMAP Directory Prefix if required ***/

class CWizardImapDirPage : public CPropertyPage
{
// Construction
public:
	CWizardImapDirPage();   // standard constructor
	CWizardImapDirPage( CWizardPropSheet *);   // standard constructor
	
	CWizardPropSheet *m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardImapDirPage)
	enum { IDD = IDD_DIALIMAPDIR };
	CString	m_strimapdir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardImapDirPage)
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
	//{{AFX_MSG(CWizardImapDirPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	//this holds the value if the dialog has been initialized already or not
	//this helps us preventing initializing the dialog with the INI values only
	//for the first time the dialog is displayed
	bool m_bActiveInitialized;
};

#endif // _WIZARDIMAPDIRPAGE_H_