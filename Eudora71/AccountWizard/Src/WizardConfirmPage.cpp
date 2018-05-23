// WizardConfirmPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardConfirmPage.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CWizardConfirmPage property page

IMPLEMENT_DYNCREATE(CWizardConfirmPage, CPropertyPage)

CWizardConfirmPage::CWizardConfirmPage(CWizardPropSheet *parent) : CPropertyPage(CWizardConfirmPage::IDD)
{

	ASSERT(parent);
	m_pParent	=	parent;
}

CWizardConfirmPage::CWizardConfirmPage() : CPropertyPage(CWizardConfirmPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardConfirmPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}

CWizardConfirmPage::~CWizardConfirmPage()
{
}

void CWizardConfirmPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardConfirmPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardConfirmPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardConfirmPage)
	ON_BN_CLICKED(IDC_RADIOCNO, OnRadioNo)
	ON_BN_CLICKED(IDC_RADIOCYES, OnRadioYes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardConfirmPage message handlers

BOOL CWizardConfirmPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem(IDC_STATACSET)->SetFont(&(m_pParent->m_StatFont), FALSE);
	SetRadioYes();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizardConfirmPage::OnKillActive() 
{

	return CPropertyPage::OnKillActive();
}

BOOL CWizardConfirmPage::OnSetActive() 
{
	if (m_pParent->m_SettingType != CWizardPropSheet::PST_ACAP)
		return false;

	m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	//set all static text fields
	GetDlgItem(IDC_STACLOGIN)->SetWindowText(m_pParent->m_pParams->LoginName);
	GetDlgItem(IDC_STATCINSERVER)->SetWindowText(m_pParent->m_pParams->InServer);
	GetDlgItem(IDC_STATCRETADDR)->SetWindowText(m_pParent->m_pParams->ReturnAddress);
	GetDlgItem(IDC_STATCRNAME)->SetWindowText(m_pParent->m_pParams->RealName);
	return CPropertyPage::OnSetActive();
}

LRESULT CWizardConfirmPage::OnWizardNext() 
{
	if (m_pParent->m_ConfirmChange == CWizardPropSheet::ACH_NO)
	{
		//make sure that at least the popserver and login fields are present
		if(m_pParent->m_pParams->InServer.IsEmpty() || 
				m_pParent->m_pParams->LoginName.IsEmpty() )
		{
			//puke
			ErrorDialog(IDS_ERR_POP_SERVER);
			SetRadioYes();
			return -1; //force users to choose something
		}
	}

	return CPropertyPage::OnWizardNext();
}


void CWizardConfirmPage::OnRadioNo() 
{
	m_pParent->m_ConfirmChange = CWizardPropSheet::ACH_NO;
}

void CWizardConfirmPage::OnRadioYes() 
{
	m_pParent->m_ConfirmChange = CWizardPropSheet::ACH_YES;
}

void CWizardConfirmPage::SetRadioNo()
{
	((CButton*)(GetDlgItem(IDC_RADIOCYES)))->SetCheck(0);
	((CButton*)(GetDlgItem(IDC_RADIOCNO)))->SetCheck(1);
	OnRadioNo();
}

void CWizardConfirmPage::SetRadioYes()
{
	((CButton*)(GetDlgItem(IDC_RADIOCYES)))->SetCheck(1);
	((CButton*)(GetDlgItem(IDC_RADIOCNO)))->SetCheck(0);
	OnRadioYes();
}
