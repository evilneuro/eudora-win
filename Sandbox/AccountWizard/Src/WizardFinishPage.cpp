// WizardFinishPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardFinishPage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardFinishPage property page

IMPLEMENT_DYNCREATE(CWizardFinishPage, CPropertyPage)

CWizardFinishPage::CWizardFinishPage() : CPropertyPage(CWizardFinishPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardFinishPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CWizardFinishPage::CWizardFinishPage( CWizardPropSheet *parent) : CPropertyPage(CWizardFinishPage::IDD)
{
	ASSERT( parent) ;
	m_pParent = parent ;
}

CWizardFinishPage::~CWizardFinishPage()
{
}

void CWizardFinishPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardFinishPage)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardFinishPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardFinishPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardFinishPage message handlers

BOOL CWizardFinishPage::OnSetActive() 
{
	ASSERT(m_pParent);

	if (m_pParent)
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);

	return CPropertyPage::OnSetActive();
}

BOOL CWizardFinishPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem( IDC_STATICCOOL)->SetFont( &(m_pParent->m_StatFont), FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
