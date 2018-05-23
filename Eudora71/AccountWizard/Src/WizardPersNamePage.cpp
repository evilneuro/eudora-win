// WizardPersNamePage.cpp
//

#include "stdafx.h"
#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "WizardPropSheet.h"
#include "ValidEdit.h" // CValidEdit

#include "WizardPersNamePage.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CWizardPersNamePage dialog


CWizardPersNamePage::CWizardPersNamePage()
	: CPropertyPage(CWizardPersNamePage::IDD)
{
	//{{AFX_DATA_INIT(CWizardPersNamePage)
	m_strpersname = _T("");
	//}}AFX_DATA_INIT
}


CWizardPersNamePage::CWizardPersNamePage( CWizardPropSheet *parent)
	: CPropertyPage(CWizardPersNamePage::IDD)
{
	ASSERT( parent);
	m_pParent = parent ;
}

void CWizardPersNamePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardPersNamePage)
	DDX_Control(pDX, IDC_EDITPERSNAME, m_NameEdit);
	DDX_Text(pDX, IDC_EDITPERSNAME, m_strpersname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardPersNamePage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardPersNamePage)
	ON_EN_CHANGE(IDC_EDITPERSNAME, OnChangeEditPersName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizardPersNamePage message handlers

BOOL CWizardPersNamePage::OnSetActive() 
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

	m_strpersname = m_pParent->m_pParams->PersName;
	UpdateData(false);

	OnChangeEditPersName(); // This will enable/disable the next button

	return CPropertyPage::OnSetActive();
}

BOOL CWizardPersNamePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	GetDlgItem( IDC_STATICPERSNAME)->SetFont( &(m_pParent->m_StatFont), FALSE);
	m_NameEdit.SetInvalid(CRString(IDS_PERSONA_EXCLUDE_CHARS));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWizardPersNamePage::OnChangeEditPersName() 
{
	if (GetDlgItem(IDC_EDITPERSNAME)->GetWindowTextLength() == 0)
		m_pParent->SetWizardButtons(PSWIZB_BACK);
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);	
}

LRESULT CWizardPersNamePage::OnWizardNext() 
{
	UpdateData( true) ;
	m_strpersname.TrimLeft();
	m_strpersname.TrimRight();


	if (m_strpersname.GetLength() >= MAX_PERSONA_LEN)
	{
		//
		// Whoops.  Name too long.
		//
		::ErrorDialog(IDS_ERR_PERSONA_NAME_TOO_LONG, MAX_PERSONA_LEN);
		GetDlgItem( IDC_EDITPERSNAME)->SetFocus () ;
		return (-1);
	}

	if( m_strpersname.IsEmpty()) {
		::ErrorDialog( IDS_ERR_WIZARD_NOPERSNAME);
		GetDlgItem( IDC_EDITPERSNAME)->SetFocus () ;
		return (-1) ;
	}

	//
	// Make sure the new name is unique.
	//
	for (LPSTR pszList = g_Personalities.List();
		 pszList && *pszList;
		 pszList += strlen(pszList) + 1)
	{
		if (m_strpersname.CompareNoCase(pszList) == 0)
		{
			//
			// Whoops.  Found duplicate.
			//
			::ErrorDialog(IDS_ERR_DUPLICATE_PERSONA, (const char *) m_strpersname);
			GetDlgItem( IDC_EDITPERSNAME)->SetFocus () ;
			return (-1);
		}
	}

	// the new personality has passed the valid name test
	m_pParent->m_pParams->PersName  = m_strpersname;	

	return CPropertyPage::OnWizardNext();
}
