// WizardOutServerPage.h

#ifndef _WIZARDOUTSERVERPAGE_H_
#define _WIZARDOUTSERVERPAGE_H_

#include "NewIfaceMT.h"

#include "resource.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////////////

/**** The Outgoing server settings ****/

class CWizardOutServerPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardOutServerPage)

// Construction
public:
	CWizardOutServerPage();
	CWizardOutServerPage( CWizardPropSheet*) ;
	~CWizardOutServerPage();

	CWizardPropSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardOutServerPage)
	enum { IDD = IDD_DIALOUTSERVER };
	CString	m_stroutserver;
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
	// Generated message map functions
	//{{AFX_MSG(CWizardOutServerPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditoutserver();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDOUTSERVERPAGE_H_
