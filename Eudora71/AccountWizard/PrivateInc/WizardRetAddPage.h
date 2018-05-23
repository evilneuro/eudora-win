// WizardRetAddPage.h

#ifndef _WIZARDRETADDPAGE_H_
#define _WIZARDRETADDPAGE_H_

#include "resource.h"

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////

/*** The Persons Return Address ***/

class CWizardRetAddPage : public CPropertyPage
{
// Construction
public:
	CWizardRetAddPage();   // standard constructor
	CWizardRetAddPage( CWizardPropSheet *);   // standard constructor

	CWizardPropSheet *m_pParent;

// Dialog Data
	//{{AFX_DATA(CWizardRetAddPage)
	enum { IDD = IDD_DIALRETADD };
	CString	m_strretadd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardRetAddPage)
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
	//{{AFX_MSG(CWizardRetAddPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditretadd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // _WIZARDRETADDPAGE_H_
