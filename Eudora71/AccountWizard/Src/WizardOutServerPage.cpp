// WizardOutServerPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardOutServerPage.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CWizardOutServerPage property page

IMPLEMENT_DYNCREATE(CWizardOutServerPage, CPropertyPage)

CWizardOutServerPage::CWizardOutServerPage( CWizardPropSheet* parent)
	: CPropertyPage(CWizardOutServerPage::IDD), m_bActiveInitialized(false)
{
	ASSERT(parent);
	m_pParent = parent;

	//{{AFX_DATA_INIT(CWizardOutServerPage)
	m_stroutserver = _T("");
	m_bAllowSMTPAuth = TRUE;
	m_ValidateServer = GetIniShort(IDS_INI_WARN_VALIDATE_HOST);
	//}}AFX_DATA_INIT
}


CWizardOutServerPage::~CWizardOutServerPage()
{
}

void CWizardOutServerPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardOutServerPage)
	DDX_Text(pDX, IDC_EDITOUTSERVER, m_stroutserver);
	DDX_Check(pDX, IDC_ALLOW_SMTP_AUTH, m_bAllowSMTPAuth);
	DDX_Check(pDX, IDS_INI_WARN_VALIDATE_HOST, m_ValidateServer);
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
	m_bAllowSMTPAuth = 	m_pParent->m_pParams->bSMTPAuthAllowed;

	if (!m_bActiveInitialized && GetIniShort(IDS_INI_WARN_VALIDATE_HOST) == FALSE)
	{
		m_ValidateServer = FALSE;
		CheckDlgButton(IDS_INI_WARN_VALIDATE_HOST, FALSE);
	}

	//UpdateData( false) ;
	UpdateData( !m_bActiveInitialized ) ;
	m_bActiveInitialized = true;

	OnChangeEditoutserver(); // This will set the next/back buttons

	return CPropertyPage::OnSetActive();
}

BOOL CWizardOutServerPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	//used to get the setting for the outgoing server
	//if one exists.
	//sites like to install INI files with the server settings
	//built in. this will read the setting if present and display
	//the value in the new account wizard
	//this is done only the dialog is initialized for the first time

	if(!m_bActiveInitialized) {
		const char* outServer = GetIniString(IDS_INI_SMTP_SERVER);
		if(outServer && *outServer) 
			SetDlgItemText(IDC_EDITOUTSERVER, outServer);
		else
			m_bActiveInitialized = true;
	}
	
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
	else if (m_ValidateServer && m_pParent->ValidateHost(m_stroutserver, IDS_SMTP_SERVICE) == false)
	{
		if (GetIniShort(IDS_INI_WARN_VALIDATE_HOST) == FALSE)
		{
			m_ValidateServer = FALSE;
			CheckDlgButton(IDS_INI_WARN_VALIDATE_HOST, FALSE);
		}
		GetDlgItem(IDC_EDITOUTSERVER)->SetFocus();
		return -1;
	}
	m_pParent->m_pParams->OutServer  = m_stroutserver;
	m_pParent->m_pParams->bSMTPAuthAllowed = m_bAllowSMTPAuth;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardOutServerPage::OnWizardBack() 
{
	UpdateData( true) ;
	m_pParent->m_pParams->OutServer  = m_stroutserver;	
	m_pParent->m_pParams->bSMTPAuthAllowed = m_bAllowSMTPAuth;
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
