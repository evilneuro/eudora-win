// WizardImportPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"

#include "rs.h"
#include "persona.h"
#include "guiutils.h"
#include "acapdlg.h"
#include "MAPIImport.h"
#include "TreeCtrlCheck.h"
#include "WizardPropSheet.h"
#include "utils.h"  //for QCLoadString


#include "WizardImportPage.h"


#include "DebugNewHelpers.h"


// --------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CWizardImportPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CWizardImportPage, CPropertyPage)
	//{{AFX_MSG_MAP(CWizardImportPage)
	ON_REGISTERED_MESSAGE(msgTreeCtrlCheck_CheckChange, OnMsgTreeCheckChange)
	ON_BN_CLICKED(IDC_LDIF_CHECK, OnLdifCheck)
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

CWizardImportPage::CWizardImportPage(CWizardPropSheet *parent /* = NULL */)
	: CPropertyPage(CWizardImportPage::IDD),
	  m_pParent(parent), m_bInitializedTree(false)
{
	//{{AFX_DATA_INIT(CWizardImportPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// --------------------------------------------------------------------------

CWizardImportPage::~CWizardImportPage()
{
}

// --------------------------------------------------------------------------

void CWizardImportPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardImportPage)
	//DDX_Control(pDX, IDC_MAIL_CHECK, m_MailChk);
	DDX_Control(pDX, IDC_IMPORT_TREE, m_ImportTree);
	//DDX_Control(pDX, IDC_IMPORT_TITLE_STATIC, m_TitleStatic);
	//DDX_Control(pDX, IDC_ALSO_STATIC, m_AlsoStatic);
	//DDX_Control(pDX, IDC_ADDRESS_CHECK, m_AddrChk);
	DDX_Control(pDX, IDC_LDIF_CHECK, m_LDIFChk);
	DDX_Control(pDX, IDC_LDIF_EDIT, m_LDIFEdt);
	DDX_Control(pDX, IDC_BROWSE_BTN, m_BrowseBtn);
	//}}AFX_DATA_MAP
}

// --------------------------------------------------------------------------

BOOL CWizardImportPage::OnInitDialog() 
{
	ASSERT(m_pParent);
	CPropertyPage::OnInitDialog();
	
	GetDlgItem(IDC_IMPORT_TITLE_STATIC)->SetFont(&(m_pParent->m_StatFont), FALSE);

	m_ImportTree.InitStateImageList(IDB_TREECTRL_CHECKMARKS, 16, RGB(255,255,255) );
	m_ImportTree.SetContainerFlag();
	m_ImportTree.SetSingleCheck();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// --------------------------------------------------------------------------

// [PROTECTED] InitTree
//
// 
bool CWizardImportPage::InitTree()
{
	if (m_bInitializedTree)
		return (true);

	ASSERT(m_pParent);

//	if (!m_pParent->m_pImporter)
//		m_pParent->m_pImporter = new CImportMail;

	// Check if we have anything to import
	if (!m_pParent->m_pImporter) {
	
		m_pParent->m_pImporter = DEBUG_NEW CImportMail;

		if(m_pParent->m_pImporter->InitPlugins())
			m_pParent->m_pImporter->InitProviders(true);
	}

	const CImportMail *pIM = m_pParent->m_pImporter;
	ASSERT(pIM);

	m_ImportTree.DeleteAllItems();

	HTREEITEM htiRoot, htiChild;

	CImportProvider *pProvider = pIM->m_newhead;
	CImportChild *pImportChild = NULL;

	CString str;

	while (pProvider)
	{
		pImportChild = pProvider->pFirstChild;

		if (pImportChild)
		{
			VERIFY(htiRoot = m_ImportTree.InsertItem(pProvider->lpszDisplayName));
			VERIFY(m_ImportTree.SetItemData(htiRoot, DWORD(pProvider)));
			
			while (pImportChild)
			{
				//str = CString(pImportChild->lpszIdentityName);
				//str += CString(" (") +  CString(pImportChild->lpszEmailAddress) + CString(")");


				ImportType kImpType = pImportChild->kImportType;
				char szImportStr[256];
				switch (kImpType) {

					case MBOX_ABOOK_PERSONA: {
						if(!QCLoadString(IDS_IMPORT_MBOX_ABOOK_PERSONA, szImportStr, sizeof(szImportStr)))
							ASSERT(0);
						break;
					}
					case MBOX_ABOOK: {
						if(!QCLoadString(IDS_IMPORT_MBOX_ABOOK, szImportStr, sizeof(szImportStr)))
							ASSERT(0);
						break;
					}
					case PERSONA: {
						if(!QCLoadString(IDS_IMPORT_PERSONA, szImportStr, sizeof(szImportStr)))
							ASSERT(0);
						break;
					}
					case MBOX: {
						if(!QCLoadString(IDS_IMPORT_MBOX, szImportStr, sizeof(szImportStr)))
							ASSERT(0);
						break;
					} default: {
						ASSERT(0);
						break;
					}

				}
				
				str = CString(szImportStr);
				str += CString(pImportChild->lpszIdentityName);
				if((pImportChild->lpszEmailAddress) && ((*(pImportChild->lpszEmailAddress)) != '\0'))
					str += CString(" (") +  CString(pImportChild->lpszEmailAddress) + CString(")");

				VERIFY(htiChild = m_ImportTree.InsertItem(str, htiRoot));
				VERIFY(m_ImportTree.SetItemData(htiChild, DWORD(pImportChild)));
				pImportChild = pImportChild->pNext;
			}
		}

//		if (pProvider->hasMail)
//		{
//			VERIFY(htiChild = m_ImportTree.InsertItem("Mail", htiRoot));
//			VERIFY(m_ImportTree.SetItemData(htiChild, DWORD(NULL)));
//		}

//		if (pProvider->hasAddresses)
//		{
//			VERIFY(htiChild = m_ImportTree.InsertItem("Address Book", htiRoot));
//			VERIFY(m_ImportTree.SetItemData(htiChild, DWORD(NULL)));
//		}

		pProvider = pProvider->pNext;
	}

	m_ImportTree.ExpandAll();

	m_bInitializedTree = true;

	return (true);
}

// --------------------------------------------------------------------------

BOOL CWizardImportPage::OnSetActive() 
{
	ASSERT(m_pParent);

	if (m_pParent->m_SettingType != CWizardPropSheet::PST_IMPORT)
		return (false);

	if (!InitTree())
		return (false);

	SetupControls();

	return CPropertyPage::OnSetActive();
}

// --------------------------------------------------------------------------

BOOL CWizardImportPage::OnKillActive() 
{
	ASSERT(m_pParent);
	// TODO: Add your specialized code here and/or call the base class
	
	return CPropertyPage::OnKillActive();
}

// --------------------------------------------------------------------------

LRESULT CWizardImportPage::OnWizardNext() 
{
	ASSERT(m_pParent);

	if (!CopySettings())
		return (-1);
	
	return CPropertyPage::OnWizardNext();
}

// --------------------------------------------------------------------------

LONG CWizardImportPage::OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pParent);
	SetupControls();

	return (0);
}

// --------------------------------------------------------------------------

void CWizardImportPage::SetupControls()
{
	ASSERT(m_pParent);

	HTREEITEM hItem = m_ImportTree.GetFirstCheckedItem();
	CImportChild *pChild = NULL;

	if (hItem)
		pChild = (CImportChild *) m_ImportTree.GetItemData(hItem);

	//CString txt = CRString(IDS_ACCT_WIZ_INTERR);
	//bool bMail = false, bAddr = false, bLDIF = false;
	bool bLDIF = false;

	if (NULL == pChild)
	{
		m_pParent->SetWizardButtons(PSWIZB_BACK);	
		
		//txt = "";		// used to be a redundant string here - JR 4/18/00
		//bMail = bAddr = bLDIF = false;
		bLDIF = false;
	}
	else
	{
		const CImportProvider *pProvider = pChild->pParent;
		ASSERT(pProvider);

		if (pProvider)
		{
			const char *pProviderName = pProvider->lpszDisplayName;
			ASSERT(pProviderName);

			if (pProviderName)
			{
				//bMail = pProvider->bHasMail;
				//bAddr = pProvider->bHasAddresses;

				bLDIF = pProvider->bHasLdif;

				if (bLDIF) 
				{
					if (pChild->lpszPathtoLdif)
					{
						m_LDIFEdt.SetWindowText(pChild->lpszPathtoLdif);
					}
					else
					{
						m_LDIFEdt.SetWindowText("");
					}
				}

				/*if (false == bMail)
				{
					if ((false == bAddr) && (false == bLDIF))
					{
						txt.Format(IDS_ONLY_SETTINGS, pProviderName);
					}
					else if (true == bAddr)
					{
						txt.Format(IDS_ALSO_ADDR_BOOK, pProviderName);
					}
					else // (true == bLDIF)
					{
						txt.Format(IDS_ALSO_LDIF, pProviderName);
					}
				}
				else // if (true == bMail)
				{
					if ((false == bAddr) && (false == bLDIF))
					{
						txt.Format(IDS_ALSO_MAIL, pProviderName);
					}
					else if (true == bAddr)
					{
						txt.Format(IDS_ALSO_MAIL_AND_AB, pProviderName);
					}
					else // (true == bLDIF)
					{
						txt.Format(IDS_ALSO_MAIL_AND_LDIF, pProviderName);
					}
				}

				if (true == bLDIF)
					txt += CRString(IDS_SEE_LDIF_HELP);*/
			}
		}
	}

	//m_AlsoStatic.SetWindowText(txt);
	//m_MailChk.EnableWindow(bMail);
	//m_AddrChk.EnableWindow(bAddr);
	m_LDIFChk.EnableWindow(bLDIF);

	// Only show one or the other (the overlap in the dlg)
	//m_AddrChk.ShowWindow(bLDIF ? SW_HIDE : SW_SHOW);
	
	m_LDIFChk.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	m_LDIFEdt.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	m_BrowseBtn.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);

	UpdateNextBackBtns();	// leaves Next enabled inappropriately sometimes
	OnLdifCheck();
}

// --------------------------------------------------------------------------

bool CWizardImportPage::CopySettings()
{
	ASSERT(m_pParent);
	ASSERT(m_pParent->m_pParams);

	bool bMail = false;
	bool bAddr = false;
	bool bPersona = false;
	bool bIsProfile = false;

	HTREEITEM hItem = m_ImportTree.GetFirstCheckedItem();

	if (!hItem)
		return (false);
	
	CImportChild *pChild = (CImportChild *) m_ImportTree.GetItemData(hItem);
	
	//bool bMail = ((m_MailChk.IsWindowVisible()) && (m_MailChk.IsWindowEnabled()) && (m_MailChk.GetCheck() != 0));
	//bool bAddr = ((m_AddrChk.IsWindowVisible()) && (m_AddrChk.IsWindowEnabled()) && (m_AddrChk.GetCheck() != 0));
	bool bLDIF = ((m_LDIFChk.IsWindowVisible()) && (m_LDIFChk.IsWindowEnabled()) && (m_LDIFChk.GetCheck() != 0));
	

	if (pChild)
	{

		ImportType kImpType = pChild->kImportType;

		switch (kImpType) {
			case MBOX_ABOOK_PERSONA: {
				bMail = true;
				bAddr = true;
				bPersona = true;
				bIsProfile = true;
				break;
			}
			case MBOX_ABOOK: {
				bMail = true;
				bAddr = true;
				break;
			}
			case PERSONA: {
				bPersona = true;
				break;
			}
			case MBOX: {
				bMail = true;
				break;
			}
			default: {
				ASSERT(0);
				break;
			}

		}
	}


	CString sFileLDIF;
	sFileLDIF.Empty();

	if (bLDIF)
		m_LDIFEdt.GetWindowText(sFileLDIF);

	m_pParent->m_pParams->PersName       = CString(pChild->lpszPersonalityName);
	m_pParent->m_pParams->RealName       = CString(pChild->lpszRealName);
	m_pParent->m_pParams->ReturnAddress  = CString(pChild->lpszEmailAddress);
	m_pParent->m_pParams->LoginName      = CString(pChild->lpszIncomingUserName);
	m_pParent->m_pParams->InServer       = CString(pChild->lpszIncomingServer);
	m_pParent->m_pParams->OutServer      = CString(pChild->lpszSMTPServer);
	m_pParent->m_pParams->IMAPPrefix     = CString("");

	CPersParams::ServType type;
	type = (pChild->bIsIMAP) ? (CPersParams::IST_IMAP) : (CPersParams::IST_POP);

	VERIFY(m_pParent->m_pParams->SetInServType(type));

	m_pParent->m_pImportChild = pChild;
	m_pParent->m_bImportMail = bMail;
	m_pParent->m_bImportAddr = bAddr;
	m_pParent->m_bImportPersona = bPersona;
	m_pParent->m_bIsProfile = bIsProfile;
	m_pParent->m_bImportLDIF = bLDIF;
	m_pParent->m_sImportFileLDIF = sFileLDIF;

	return (true);
}

// --------------------------------------------------------------------------

void CWizardImportPage::UpdateNextBackBtns()
{
	if ((m_LDIFEdt.IsWindowVisible()) && (m_LDIFChk.GetCheck() != 0) && (m_LDIFEdt.GetWindowTextLength() == 0))
		m_pParent->SetWizardButtons(PSWIZB_BACK);
	else
		m_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}

// --------------------------------------------------------------------------

void CWizardImportPage::OnLdifCheck() 
{
	UpdateNextBackBtns();

	const bool bChecked = (m_LDIFChk.GetCheck() != 0);

	m_LDIFEdt.EnableWindow(bChecked);
	m_BrowseBtn.EnableWindow(bChecked);
}

// --------------------------------------------------------------------------

void CWizardImportPage::OnBrowseBtn() 
{
	CString str;
	m_LDIFEdt.GetWindowText(str);
	
	CFileDialog dlg(TRUE, "LDIF", ((LPCSTR)str), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, "LDIF (*.LDIF)|*.LDIF|All Files (*.*)|*.*|", this);
	
	if (IDOK == dlg.DoModal())
	{
		CString pathname = dlg.GetPathName();

		if (!pathname.IsEmpty())
		{
			m_LDIFEdt.SetWindowText(dlg.GetPathName());
			UpdateNextBackBtns();
		}
	}
}
