// WizardFinishPage.h

#ifndef _WIZARDFINISHPAGE_H_
#define _WIZARDFINISHPAGE_H_

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
/** OK finally come to an end, inform the user that he/she's accomplished 
	a real great task and exit the personality wizard
***/

class CWizardFinishPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CWizardFinishPage)

// Construction
public:
	CWizardFinishPage();
	CWizardFinishPage( CWizardPropSheet *) ;
	~CWizardFinishPage();

	CWizardPropSheet *m_pParent ;

// Dialog Data
	//{{AFX_DATA(CWizardFinishPage)
	enum { IDD = IDD_DIALFINISH };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWizardFinishPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWizardFinishPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _WIZARDFINISHPAGE_H_
