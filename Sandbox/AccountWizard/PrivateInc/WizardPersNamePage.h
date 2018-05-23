// WizardPersNamePage.h

#ifndef _WIZARDPERSNAMEPAGE_H_
#define _WIZARDPERSNAMEPAGE_H_

#include "resource.h"
#include "NewIfaceMT.h"

#include "ValidEdit.h" // CValidEdit

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

class CWizardPropSheet ;

//////////////////////////////////////////////////////////////////////////////////////
/*** Personality Name Dialog ************/

class CWizardPersNamePage : public CPropertyPage
{
// Construction
public:
	CWizardPersNamePage();   // standard constructor
	CWizardPersNamePage( CWizardPropSheet *);

	CWizardPropSheet * m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardPersNamePage)
	enum { IDD = IDD_DIALPERSNAME };
	CValidEdit	m_NameEdit;
	CString	m_strpersname;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardPersNamePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWizardPersNamePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditPersName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // _WIZARDPERSNAMEPAGE_H_
