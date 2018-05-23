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

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardInServerPage property page

IMPLEMENT_DYNCREATE(CWizardInServerPage, CPropertyPage)

CWizardInServerPage::CWizardInServerPage()
	: CPropertyPage(CWizardInServerPage::IDD), m_bActiveInitialized(false)
{
	//{{AFX_DATA_INIT(CWizardInServerPage)
	m_strinserver = _T("");
	//}}AFX_DATA_INIT
}


CWizardInServerPage::CWizardInServerPage(CWizardPropSheet* parent)
	: CPropertyPage(CWizardInServerPage::IDD), m_bActiveInitialized(false)
{
	ASSERT(parent);
	m_pParent = parent;
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

	if (!m_bActiveInitialized)
	{
		// Do one-time dialog initialization here. Try to avoid using OnInitDialog() for as much as possible, because
		// all pages get OnInitDialog(), even the ones that are never shown.

		GetDlgItem(IDC_STATINSERVER)->SetFont(&(m_pParent->m_StatFont), FALSE);

		SetRadioPOP(); //default

		// special check if ACAP
		if (m_pParent->m_SettingType == CWizardPropSheet::PST_ACAP)
		{
			if(GetIniShort(IDS_INI_USES_IMAP) == 1)
			{
				SetRadioIMAP();
			}
		}

		GetDlgItem(IDC_EDITINSERVER)->SetFocus();

		m_bActiveInitialized = true;
	}

	// -----

	m_strinserver	=	m_pParent->m_pParams->InServer;
	UpdateData(false);

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

	CPersParams::ServType type;
	VERIFY(m_pParent->m_pParams->GetInServType(type));

	if(m_strinserver.IsEmpty()) {
		::ErrorDialog(IDS_ERR_WIZARD_NOINSERVER);
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

