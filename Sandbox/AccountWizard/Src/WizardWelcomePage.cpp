// WizardWelcomePage.cpp

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardWelcomePage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardWelcomePage property page

IMPLEMENT_DYNCREATE(CWizardWelcomePage, CPropertyPage)

CWizardWelcomePage::CWizardWelcomePage() : CPropertyPage(CWizardWelcomePage::IDD)
{
	//{{AFX_DATA_INIT(CWizardWelcomePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CWizardWelcomePage::CWizardWelcomePage(CWizardPropSheet *parent) : CPropertyPage(CWizardWelcomePage::IDD)
{
	ASSERT(parent);
	m_pParent = parent;
}

CWizardWelcomePage::~CWizardWelcomePage()
{
}

void CWizardWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardWelcomePage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardWelcomePage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardWelcomePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinish message handlers

BOOL CWizardWelcomePage::OnSetActive() 
{
	m_pParent->SetWizardButtons(PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

BOOL CWizardWelcomePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem(IDC_STATWELCOME)->SetFont(&(m_pParent->m_StatFont), FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
