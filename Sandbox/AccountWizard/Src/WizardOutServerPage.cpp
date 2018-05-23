// WizardOutServerPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardOutServerPage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardOutServerPage property page

IMPLEMENT_DYNCREATE(CWizardOutServerPage, CPropertyPage)

CWizardOutServerPage::CWizardOutServerPage() : CPropertyPage(CWizardOutServerPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardOutServerPage)
	m_stroutserver = _T("");
	//}}AFX_DATA_INIT
}


CWizardOutServerPage::CWizardOutServerPage( CWizardPropSheet* parent)
	: CPropertyPage(CWizardOutServerPage::IDD)
{
	ASSERT( parent);
	m_pParent = parent ;
}


CWizardOutServerPage::~CWizardOutServerPage()
{
}

void CWizardOutServerPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardOutServerPage)
	DDX_Text(pDX, IDC_EDITOUTSERVER, m_stroutserver);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardOutServerPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardOutServerPage)
	ON_EN_CHANGE(IDC_EDITOUTSERVER, OnChangeEditoutserver)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardOutServerPage message handlers

BOOL CWizardOutServerPage::OnSetActive() 
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

	m_stroutserver	=	m_pParent->m_pParams->OutServer;
	UpdateData( false) ;
	
	OnChangeEditoutserver(); // This will set the next/back buttons

	return CPropertyPage::OnSetActive();
}

BOOL CWizardOutServerPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem( IDC_STATOUTSERVER)->SetFont( &(m_pParent->m_StatFont), FALSE);
	GetDlgItem( IDC_EDITOUTSERVER)->SetFocus() ;

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CWizardOutServerPage::OnWizardNext() 
{
	UpdateData( true) ;
	if( m_stroutserver.IsEmpty()) {
		::ErrorDialog( IDS_ERR_WIZARD_NOOUTSERVER) ;
		GetDlgItem( IDC_EDITOUTSERVER)->SetFocus() ;
		return -1 ;
	}
	m_pParent->m_pParams->OutServer  = m_stroutserver;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardOutServerPage::OnWizardBack() 
{
	UpdateData( true) ;
	m_pParent->m_pParams->OutServer  = m_stroutserver;	
	return CPropertyPage::OnWizardBack();
}

// Diable the 'NEXT' button until all fields are complete
void CWizardOutServerPage::OnChangeEditoutserver() 
{
	if (GetDlgItem(IDC_EDITOUTSERVER)->GetWindowTextLength() == 0)
		m_pParent->SetWizardButtons(PSWIZB_BACK);
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
}
