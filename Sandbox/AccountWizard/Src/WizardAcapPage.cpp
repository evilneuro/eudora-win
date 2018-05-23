// WizardAcapPage.cpp
//

#include "stdafx.h"

#ifdef _DEBUG
#ifdef new
#undef new
#endif
#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif
#endif

#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardAcapPage.h"

#ifdef _DEBUG
	#undef new
        #ifndef DEBUG_NEW
        #define DEBUG_NEW new(__FILE__, __LINE__)
        #endif
	#define new DEBUG_NEW
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWizardAcapPage dialog


CWizardAcapPage::CWizardAcapPage(CWizardPropSheet *parent)
	: CPropertyPage(CWizardAcapPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardAcapPage)
	//}}AFX_DATA_INIT

	ASSERT ( parent);
	m_pParent = parent;
}


void CWizardAcapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardAcapPage)
	DDX_Text(pDX, IDS_INI_ACAP_SERVER, m_stracapserver);
	DDX_Text(pDX, IDS_INI_ACAP_USER_ID, m_stracapusername);
	DDX_Text(pDX, IDS_INI_ACAP_PASSWORD, m_stracappassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardAcapPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardAcapPage)
	ON_EN_CHANGE(IDS_INI_ACAP_PASSWORD, OnChangeIniAcapPassword)
	ON_EN_CHANGE(IDS_INI_ACAP_SERVER, OnChangeIniAcapServer)
	ON_EN_CHANGE(IDS_INI_ACAP_USER_ID, OnChangeIniAcapUserId)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardAcapPage message handlers

BOOL CWizardAcapPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem( IDC_STATACAP)->SetFont( &(m_pParent->m_StatFont), FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CWizardAcapPage::OnSetActive() 
{
	if (m_pParent->m_SettingType != CWizardPropSheet::PST_ACAP)
		return false;

	UpdateButtons(); // This will set the next/back buttons

	return CPropertyPage::OnSetActive();

}

LRESULT CWizardAcapPage::OnWizardNext() 
{
	UpdateData( true) ;

	// initiate the ACAP protocol
	CAcapSettings AcapSet ;
	
	CWaitCursor wait; // display wait cursor
	
	if (AcapSet.RetrieveSettings( m_stracapserver, m_stracapusername, m_stracappassword))
	{
		// All settings from the ACAP server are directly written to the INI (is this a good thing?)

//		m_pParent->m_pCurrentAccount->m_bUsesAcap	=	true;

		// go and initialise all data members of m_pParams, to what
		// we just got from the ACAP Server
		char szPOPUserName[ 128];
		::GetIniString(IDS_INI_POP_ACCOUNT, szPOPUserName, sizeof(szPOPUserName));
		char* pszAtSign = strrchr(szPOPUserName, '@');
		if( pszAtSign) {
				char szTemp[ 128];
				strcpy( szTemp, pszAtSign + 1) ;
				*pszAtSign = 0 ;
				//SetIniString( IDS_INI_LOGIN_NAME, szPOPUserName) ;
				m_pParent->m_pParams->LoginName	=	szPOPUserName;
				//SetIniString( IDS_INI_POP_SERVER, szTemp) ;
				m_pParent->m_pParams->InServer	=	szTemp;
				m_pParent->m_pParams->ReturnAddress	=	GetIniString(IDS_INI_RETURN_ADDRESS) ;
				m_pParent->m_pParams->RealName	=	GetIniString(IDS_INI_REAL_NAME) ;
		}

		// all is cool
		return CPropertyPage::OnWizardNext();
	}
	else {
			return -1 ; // something went wrong. EEEEEEEEEKKKKK!!!
	}
}

void CWizardAcapPage::OnChangeIniAcapPassword() 
{
	UpdateButtons();
}

void CWizardAcapPage::OnChangeIniAcapServer() 
{
	UpdateButtons();
}

void CWizardAcapPage::OnChangeIniAcapUserId() 
{
	UpdateButtons();
}

void CWizardAcapPage::UpdateButtons()
{
	// Only check on the server, allow empty username and password
	if (GetDlgItem(IDS_INI_ACAP_SERVER)->GetWindowTextLength() != 0)
	{
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
	}
	else
	{
		// Something is missing, so only BACK available
		m_pParent->SetWizardButtons(PSWIZB_BACK);
	}
}
