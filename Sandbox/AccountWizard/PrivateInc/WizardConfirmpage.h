// WizardConfirmPage.h

#ifndef _WIZARDCONFIRMPAGE_H_
#define _WIZARDCONFIRMPAGE_H_

#include "NewIfaceMT.h"

#include "resource.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

class CWizardPropSheet ;

//////////////////////////////////////////////////////////////////////

/*** To show the user the imported settings ****/
class CWizardConfirmPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardConfirmPage)

// Construction
public:
	CWizardConfirmPage();
	CWizardConfirmPage( CWizardPropSheet *);
	~CWizardConfirmPage();

	CWizardPropSheet *m_pParent;

// Dialog Data
	//{{AFX_DATA(CWizardConfirmPage)
	enum { IDD = IDD_DIALCONFIRM };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardConfirmPage)
	public:
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetRadioYes();
	void SetRadioNo();
	// Generated message map functions
	//{{AFX_MSG(CWizardConfirmPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioNo();
	afx_msg void OnRadioYes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDCONFIRMPAGE_H_
