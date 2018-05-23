// WizardInServerPage.h

#ifndef _WIZARDINSERVERPAGE_H_
#define _WIZARDINSERVERPAGE_H_

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

/**** The Incoming server settings ****/

class CWizardInServerPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardInServerPage)

// Construction
public:
	CWizardInServerPage();
	CWizardInServerPage( CWizardPropSheet*) ;
	~CWizardInServerPage();

	CWizardPropSheet* m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardInServerPage)
	enum { IDD = IDD_DIALINSERVER };
	CEdit	m_InServerEdit;
	CString	m_strinserver;
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