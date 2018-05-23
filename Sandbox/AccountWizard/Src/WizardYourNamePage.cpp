// WizardYourNamePage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardYourNamePage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardYourNamePage dialog


CWizardYourNamePage::CWizardYourNamePage()
	: CPropertyPage(CWizardYourNamePage::IDD)
{
	//{{AFX_DATA_INIT(CWizardYourNamePage)
	m_stryourname = _T("");
	//}}AFX_DATA_INIT
}

CWizardYourNamePage::CWizardYourNamePage( CWizardPropSheet *parent)
	: CPropertyPage(CWizardYourNamePage::IDD)
{
	ASSERT ( parent);
	m_pParent	=	parent ;
}

void CWizardYourNamePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardYourNamePage)
	DDX_Text(pDX, IDC_EDIT_YOURNAME, m_stryourname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardYourNamePage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardYourNamePage)
	ON_EN_CHANGE(IDC_EDIT_YOURNAME, OnChangeEditYourname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardYourNamePage message handlers

BOOL CWizardYourNamePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	GetDlgItem( IDC_STATICYOURNAME)->SetFont( &(m_pParent->m_StatFont), FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizardYourNamePage::OnSetActive()
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

	m_stryourname = m_pParent->m_pParams->RealName;
	UpdateData(false);

	OnChangeEditYourname(); // This will set the next/back buttons

	return CPropertyPage::OnSetActive();
}

BOOL CWizardYourNamePage::OnKillActive() 
{
	
	return CPropertyPage::OnKillActive();
}

LRESULT CWizardYourNamePage::OnWizardNext() 
{
	UpdateData( true) ;
	if( m_stryourname.IsEmpty()) {
		::ErrorDialog( IDS_ERR_WIZARD_NONAME);
		GetDlgItem( IDC_EDIT_YOURNAME)->SetFocus () ;
		return -1 ;
	}
	m_pParent->m_pParams->RealName  = m_stryourname;

	
	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardYourNamePage::OnWizardBack() 
{

	UpdateData( true) ;
	m_pParent->m_pParams->RealName  = m_stryourname;
	return CPropertyPage::OnWizardBack();
}

// Diable the 'NEXT' button until all fields are complete
void CWizardYourNamePage::OnChangeEditYourname() 
{
	if (GetDlgItem(IDC_EDIT_YOURNAME)->GetWindowTextLength() == 0)
		m_pParent->SetWizardButtons(PSWIZB_BACK);	
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
}
