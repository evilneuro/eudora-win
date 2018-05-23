// WizardImapDirPage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"

#include "WizardImapDirPage.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CWizardImapDirPage dialog


CWizardImapDirPage::CWizardImapDirPage()
	: CPropertyPage(CWizardImapDirPage::IDD), m_bActiveInitialized(false)
{
	//{{AFX_DATA_INIT(CWizardImapDirPage)
	m_strimapdir = _T("");
	//}}AFX_DATA_INIT
}

CWizardImapDirPage::CWizardImapDirPage( CWizardPropSheet *parent)
	: CPropertyPage(CWizardImapDirPage::IDD), m_bActiveInitialized(false)
{
	ASSERT ( parent);
	m_pParent	=	parent ;
}

void CWizardImapDirPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardImapDirPage)
	DDX_Text(pDX, IDC_IMAP_EDIT, m_strimapdir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardImapDirPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardImapDirPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardImapDirPage message handlers

BOOL CWizardImapDirPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	GetDlgItem( IDC_IMAP_CAPTION)->SetFont( &(m_pParent->m_StatFont), FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizardImapDirPage::OnSetActive() 
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

	// Skip when NOT using IMAP
	CPersParams::ServType type;
	if (!m_pParent->m_pParams->GetInServType(type))
		return (FALSE);

	if (type != CPersParams::IST_IMAP)
		return (FALSE);

	//we need to read from the INI file and populate the IMAP Dir prefix only
	//the first time that the dialog is initialized. The next time the dialog is displayed
	//it must not read from the INI file but retain the value
	if(!m_bActiveInitialized) {
		const char* imapDir = GetIniString(IDS_INI_IMAP_PREFIX);
		if(imapDir && *imapDir) 
			SetDlgItemText(IDC_IMAP_EDIT, imapDir);
		m_bActiveInitialized = true;
	}

	m_strimapdir = m_pParent->m_pParams->IMAPPrefix ;
	UpdateData(true);
	//UpdateData( false) ;

	// The IMAP dir is optional, so no check needed for 'NEXT' -- always enabled
	m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	

	return CPropertyPage::OnSetActive();
}

BOOL CWizardImapDirPage::OnKillActive() 
{
	
	return CPropertyPage::OnKillActive();
}

LRESULT CWizardImapDirPage::OnWizardNext() 
{
	UpdateData( true) ;
	m_pParent->m_pParams->IMAPPrefix  = m_strimapdir;
	
	return CPropertyPage::OnWizardNext();
}

LRESULT CWizardImapDirPage::OnWizardBack() 
{

	UpdateData( true) ;
	m_pParent->m_pParams->IMAPPrefix  = m_strimapdir;
	return CPropertyPage::OnWizardBack();
}
