// WizardDialLogin.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardLoginPage.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CWizardLoginPage property page

IMPLEMENT_DYNCREATE(CWizardLoginPage, CPropertyPage)

CWizardLoginPage::CWizardLoginPage() : CPropertyPage(CWizardLoginPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardLoginPage)
	m_strlogin = _T("");
	//}}AFX_DATA_INIT
}

CWizardLoginPage::CWizardLoginPage( CWizardPropSheet *parent)
	: CPropertyPage(CWizardLoginPage::IDD)
{
	ASSERT( parent);
	m_pParent	=	parent;
}


CWizardLoginPage::~CWizardLoginPage()
{
}

void CWizardLoginPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardLoginPage)
	DDX_Text(pDX, IDC_EDITLOGINNAME, m_strlogin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardLoginPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardLoginPage)
	ON_EN_CHANGE(IDC_EDITLOGINNAME, OnChangeEditloginname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardLoginPage message handlers

BOOL CWizardLoginPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem( IDC_STATICLOGIN)->SetFont( &(m_pParent->m_StatFont), FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizardLoginPage::OnSetActive() 
{
	// Skip when UNKNOWN
	if (m_pParent->m_SettingType == CWizardPropSheet::PST_UNKNOWN)
		return (FALSE);

	// Skip when ACAP and NO confirm
	if ( (m_pParent->m_SettingType == CWizardPropSheet::PST_ACAP) && (m_pParent->m_ConfirmChange == CWizardPropSheet::ACH_NO) )
		return (FALSE);

	// Skip when IMPORT
	if (m_pParent->m_SettingType == CWizardPropSheet::PST_IMPORT)
		return (FALSE);

	m_strlogin	=	m_pParent->m_pParams->LoginName ;
	UpdateData( false);

	OnChangeEditloginname(); // This will set the next/back buttons

	return CPropertyPage::OnSetActive();
}

BOOL CWizardLoginPage::OnKillActive() 
{	
	return CPropertyPage::OnKillActive();
}

LRESULT CWizardLoginPage::OnWizardNext() 
{
	UpdateData( true) ;
	if( m_strlogin.IsEmpty() ) {
		::ErrorDialog( IDS_ERR_WIZARD_NOLOGIN) ;
		GetDlgItem( IDC_EDITLOGINNAME)->SetFocus() ;
		return -1;
	}
	m_pParent->m_pParams->LoginName  = m_strlogin	;

	
	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardLoginPage::OnWizardBack() 
{
	UpdateData( true) ;
	m_pParent->m_pParams->LoginName  = m_strlogin	;
	
	return CPropertyPage::OnWizardBack();
}

// Diable the 'NEXT' button until all fields are complete
void CWizardLoginPage::OnChangeEditloginname() 
{
	if (GetDlgItem(IDC_EDITLOGINNAME)->GetWindowTextLength() == 0)
		m_pParent->SetWizardButtons(PSWIZB_BACK);	
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
}
