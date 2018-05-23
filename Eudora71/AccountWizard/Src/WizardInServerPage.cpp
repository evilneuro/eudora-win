// WizardInServerPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"
#include "PersParams.h"

#include "WizardInServerPage.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CWizardInServerPage property page

IMPLEMENT_DYNCREATE(CWizardInServerPage, CPropertyPage)

CWizardInServerPage::CWizardInServerPage(CWizardPropSheet* parent)
	: CPropertyPage(CWizardInServerPage::IDD), m_bActiveInitialized(false)
{
	ASSERT(parent);
	m_pParent = parent;

	//{{AFX_DATA_INIT(CWizardInServerPage)
	m_strinserver = _T("");
	m_ValidateServer = GetIniShort(IDS_INI_WARN_VALIDATE_HOST);
	//}}AFX_DATA_INIT
}


CWizardInServerPage::~CWizardInServerPage()
{
}

void CWizardInServerPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardInServerPage)
	DDX_Control(pDX, IDC_EDITINSERVER, m_InServerEdit);
	DDX_Text(pDX, IDC_EDITINSERVER, m_strinserver);
	DDX_Check(pDX, IDS_INI_WARN_VALIDATE_HOST, m_ValidateServer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardInServerPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardInServerPage)
	ON_EN_CHANGE(IDC_EDITINSERVER, OnChangeEditinserver)
	ON_BN_CLICKED(IDC_RADIOIMAP, OnRadioIMAP)
	ON_BN_CLICKED(IDC_RADIOPOP, OnRadioPOP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardInServerPage message handlers

BOOL CWizardInServerPage::OnSetActive() 
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

	//used to get the setting for the incoming server
	//if one exists.
	//sites like to install INI files with the server settings
	//built in. this will read the setting if present and display
	//the value in the new account wizard
	//this is done only the dialog is initialized for the first time
	const char* inServer = NULL;

	if (!m_bActiveInitialized)
	{
		// Do one-time dialog initialization here. Try to avoid using OnInitDialog() for as much as possible, because
		// all pages get OnInitDialog(), even the ones that are never shown.

		GetDlgItem(IDC_STATINSERVER)->SetFont(&(m_pParent->m_StatFont), FALSE);

		// Default to IMAP radio button if:
		// params specify it OR
		// special check if ACAP
		if ( m_pParent->m_pParams->bIMAP || (GetIniShort(IDS_INI_USES_IMAP) == 1) ||
			 ((m_pParent->m_SettingType == CWizardPropSheet::PST_ACAP) && (GetIniShort(IDS_INI_USES_IMAP) == 1)) )
		{
			SetRadioIMAP();
			inServer = GetIniString(IDS_INI_IMAP_SERVER);
		}
		else
		{
			SetRadioPOP();
			inServer = GetIniString(IDS_INI_POP_SERVER);

		}

		if(inServer && *inServer) 
			SetDlgItemText(IDC_EDITINSERVER, inServer);	

		GetDlgItem(IDC_EDITINSERVER)->SetFocus();

		m_bActiveInitialized = true;
	}

	// -----

	m_strinserver	=	m_pParent->m_pParams->InServer;

	//UpdateData(false);
	UpdateData(inServer!=NULL);

	OnChangeEditinserver(); // This will set the next/back buttons

	return CPropertyPage::OnSetActive();
}

BOOL CWizardInServerPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizardInServerPage::OnKillActive() 
{
	
	return CPropertyPage::OnKillActive();
}

LRESULT CWizardInServerPage::OnWizardNext() 
{
	ASSERT(m_pParent);
	ASSERT(m_pParent->m_pParams);

	UpdateData(true);

	const BOOL bIsPOP = m_pParent->m_pParams->bPOP;

	if(m_strinserver.IsEmpty()) {
		::ErrorDialog(IDS_ERR_WIZARD_NOINSERVER);
		GetDlgItem(IDC_EDITINSERVER)->SetFocus();
		return -1;
	}
	else if (m_ValidateServer && m_pParent->ValidateHost(m_strinserver, bIsPOP? IDS_POP_SERVICE : IDS_IMAP_SERVICE) == false)
	{
		if (GetIniShort(IDS_INI_WARN_VALIDATE_HOST) == FALSE)
		{
			m_ValidateServer = FALSE;
			CheckDlgButton(IDS_INI_WARN_VALIDATE_HOST, FALSE);
		}
		GetDlgItem(IDC_EDITINSERVER)->SetFocus();
		return -1;
	}
	m_pParent->m_pParams->InServer  = m_strinserver;

	// equate the smtp server (if empty) to this
	if(m_pParent->m_pParams->OutServer.IsEmpty())
		m_pParent->m_pParams->OutServer	=	m_strinserver;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardInServerPage::OnWizardBack() 
{
	UpdateData(true);
	m_pParent->m_pParams->InServer  = m_strinserver;	
	return CPropertyPage::OnWizardBack();
}

// Diable the 'NEXT' button until all fields are complete
void CWizardInServerPage::OnChangeEditinserver() 
{
	if (GetDlgItem(IDC_EDITINSERVER)->GetWindowTextLength() == 0)
		m_pParent->SetWizardButtons(PSWIZB_BACK);	
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
}

void CWizardInServerPage::OnRadioIMAP() 
{
	VERIFY(m_pParent->m_pParams->SetInServType(CPersParams::IST_IMAP));
}

void CWizardInServerPage::OnRadioPOP() 
{
	VERIFY(m_pParent->m_pParams->SetInServType(CPersParams::IST_POP));
}

void CWizardInServerPage::SetRadioIMAP()
{
	((CButton*)(GetDlgItem(IDC_RADIOIMAP)))->SetCheck(1);
	OnRadioIMAP();
}

void CWizardInServerPage::SetRadioPOP()
{
	((CButton*)(GetDlgItem(IDC_RADIOPOP)))->SetCheck(1);
	OnRadioPOP();
}

