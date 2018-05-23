// WizardClientPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "MAPIImport.h"
#include "TreeCtrlCheck.h"
#include "WizardPropSheet.h"

#include "WizardClientPage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardClientPage dialog



BEGIN_MESSAGE_MAP(CWizardClientPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardClientPage)
	ON_BN_CLICKED(IDC_NEW_RADIO, OnRadioNew)
	ON_BN_CLICKED(IDC_IMPORT_RADIO, OnRadioOld)
	ON_BN_CLICKED(IDC_ACAP_RADIO, OnRadioAcap)
	ON_BN_CLICKED(IDC_ADVANCED_RADIO, OnAdvancedRadio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWizardClientPage::CWizardClientPage(CWizardPropSheet* parent)
	: CPropertyPage(CWizardClientPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardClientPage)
	//}}AFX_DATA_INIT

	m_pParent = parent;
	ASSERT(m_pParent);
}


void CWizardClientPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardClientPage)
	DDX_Control(pDX, IDC_CLIENTS_TITLE_STATIC, m_TitleStatic);
	DDX_Control(pDX, IDC_NEW_RADIO, m_NewRadio);
	DDX_Control(pDX, IDC_IMPORT_RADIO, m_ImportRadio);
	DDX_Control(pDX, IDC_ACAP_RADIO, m_AcapRadio);
	DDX_Control(pDX, IDC_ADVANCED_RADIO, m_AdvancedRadio);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWizardClientPage message handlers


//the very fact that i hit this function means that one or more email
//clients are configured

BOOL CWizardClientPage::OnInitDialog() 
{
	ASSERT(m_pParent);
	CPropertyPage::OnInitDialog();

	m_TitleStatic.SetFont(&(m_pParent->m_StatFont), FALSE);

	// Select 'NEW' radio btn
	m_NewRadio.SetCheck(1);
	OnRadioNew();

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWizardClientPage::OnRadioNew() // New
{
	ASSERT(m_pParent);
	m_pParent->m_SettingType = CWizardPropSheet::PST_NEW;
	SetupBtns();
}

void CWizardClientPage::OnRadioOld() // Import
{
	ASSERT(m_pParent);
	m_pParent->m_SettingType = CWizardPropSheet::PST_IMPORT;
	SetupBtns();
}

void CWizardClientPage::OnRadioAcap() // ACAP
{
	ASSERT(m_pParent);
	m_pParent->m_SettingType = CWizardPropSheet::PST_ACAP;
	SetupBtns();
}

void CWizardClientPage::OnAdvancedRadio() // Advanced
{
	ASSERT(m_pParent);
	m_pParent->m_SettingType = CWizardPropSheet::PST_ADVANCED;
	SetupBtns();
}

BOOL CWizardClientPage::OnKillActive() 
{
	ASSERT(m_pParent);
	UpdateData(true);

	return CPropertyPage::OnKillActive();
}

BOOL CWizardClientPage::OnSetActive() 
{
	ASSERT(m_pParent);
	SetupBtns();

	// No ACAP configuration if this is not the DOMINANT Account
	if (!(m_pParent->m_bIsDominant))
	{
		m_AcapRadio.ShowWindow(SW_HIDE);
	}

	// Check if we have anyting to import
	if (!m_pParent->m_pImporter)
		m_pParent->m_pImporter = new CImportMail;

	CImportMail *pIM = m_pParent->m_pImporter;
	ASSERT(pIM);

	if (pIM->NumOfPersonalities() == 0)
	{
		m_ImportRadio.ShowWindow(SW_HIDE);
	}

	bool bNewCheck = false;
	bool bImportCheck = false;
	bool bAcapCheck = false;
	bool bAdvCheck = false;

	switch (m_pParent->m_SettingType) // PST_UNKNOWN, PST_NEW, PST_IMPORT, PST_ACAP, PST_ADVANCED
	{
		case CWizardPropSheet::PST_IMPORT:		bImportCheck = true;	break;
		case CWizardPropSheet::PST_ACAP:		bAcapCheck = true;		break;
		case CWizardPropSheet::PST_ADVANCED:	bAdvCheck = true;		break;
		
		default:
		case CWizardPropSheet::PST_NEW:
		case CWizardPropSheet::PST_UNKNOWN:
		{
			bNewCheck = true;
		}
		break;
	}

	m_NewRadio.SetCheck(bNewCheck);
	m_ImportRadio.SetCheck(bImportCheck);
	m_AcapRadio.SetCheck(bAcapCheck);
	m_AdvancedRadio.SetCheck(bAdvCheck);


	return CPropertyPage::OnSetActive();
}

LRESULT CWizardClientPage::OnWizardNext() 
{
	ASSERT(m_pParent);
	
	if (m_pParent->m_SettingType == CWizardPropSheet::PST_ADVANCED)
	{
		return (-1);
	}

	return CPropertyPage::OnWizardNext();
}

void CWizardClientPage::SetupBtns()
{
	ASSERT(m_pParent);

	if (m_pParent->m_SettingType == CWizardPropSheet::PST_ADVANCED)
	{
		m_pParent->SetWizardButtons(PSWIZB_FINISH);
	}
	else if (m_pParent->m_bIsDominant)
	{
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	}
	else
	{
		m_pParent->SetWizardButtons(PSWIZB_NEXT);
	}
}
