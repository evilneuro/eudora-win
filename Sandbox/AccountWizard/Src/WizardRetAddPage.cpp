// WizardRetAddPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardRetAddPage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardRetAddPage dialog


CWizardRetAddPage::CWizardRetAddPage()
	: CPropertyPage(CWizardRetAddPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardRetAddPage)
	m_strretadd = _T("");
	//}}AFX_DATA_INIT
}


CWizardRetAddPage::CWizardRetAddPage( CWizardPropSheet *parent)
	: CPropertyPage(CWizardRetAddPage::IDD)
{
	ASSERT( parent);
	m_pParent	=	parent ;
}

void CWizardRetAddPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardRetAddPage)
	DDX_Text(pDX, IDC_EDITRETADD, m_strretadd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardRetAddPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardRetAddPage)
	ON_EN_CHANGE(IDC_EDITRETADD, OnChangeEditretadd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardRetAddPage message handlers

BOOL CWizardRetAddPage::OnSetActive() 
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

	m_strretadd = m_pParent->m_pParams->ReturnAddress;
	UpdateData( false);

	OnChangeEditretadd(); // This will set the next/back buttons
	
	return CPropertyPage::OnSetActive();
}

BOOL CWizardRetAddPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem( IDC_STATICRETADD)->SetFont( &(m_pParent->m_StatFont), FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizardRetAddPage::OnKillActive() 
{
	return CPropertyPage::OnKillActive();
}

LRESULT CWizardRetAddPage::OnWizardNext() 
{
	UpdateData( true);
	if( m_strretadd.IsEmpty() ) {
		::ErrorDialog( IDS_ERR_WIZARD_NORETADDR) ;
		GetDlgItem(IDC_EDITRETADD)->SetFocus() ;
		return -1;
	}
	m_pParent->m_pParams->ReturnAddress  = m_strretadd	;
	
	//parse the return address and fill in the login field
	// only if this is not using ACAP.
	// If using ACAP then life is good
	if (m_pParent->m_SettingType != CWizardPropSheet::PST_ACAP)
	{
		char szLoginName[ 128] ;
		strcpy( szLoginName, m_strretadd) ;
		char *pszAtSign	=	strrchr( szLoginName, '@') ;
		if( pszAtSign) {
			*pszAtSign = 0 ;
			m_pParent->m_pParams->LoginName	=	szLoginName ;
		}
	}

	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardRetAddPage::OnWizardBack() 
{
	UpdateData( true);
	m_pParent->m_pParams->ReturnAddress  = m_strretadd	;
	
	return CPropertyPage::OnWizardBack();
}

// Diable the 'NEXT' button until all fields are complete
void CWizardRetAddPage::OnChangeEditretadd() 
{
	if (GetDlgItem(IDC_EDITRETADD)->GetWindowTextLength() == 0)
		m_pParent->SetWizardButtons(PSWIZB_BACK);	
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
}
