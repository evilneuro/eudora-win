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

#include "WizardImportPage.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

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
	DDX_Control(pDX, IDC_MAIL_CHECK, m_MailChk);
	DDX_Control(pDX, IDC_IMPORT_TREE, m_ImportTree);
	DDX_Control(pDX, IDC_IMPORT_TITLE_STATIC, m_TitleStatic);
	DDX_Control(pDX, IDC_ALSO_STATIC, m_AlsoStatic);
	DDX_Control(pDX, IDC_ADDRESS_CHECK, m_AddrChk);
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

	if (!m_pParent->m_pImporter)
		m_pParent->m_pImporter = new CImportMail;

	const CImportMail *pIM = m_pParent->m_pImporter;
	ASSERT(pIM);

	m_ImportTree.DeleteAllItems();

	HTREEITEM htiRoot, htiChild;

	ImportProvider *pProvider = pIM->m_head;
	ImportChild *pImportChild = NULL;

	CString str;

	while (pProvider)
	{
		pImportChild = pProvider->FirstChild;

		if (pImportChild)
		{
			VERIFY(htiRoot = m_ImportTree.InsertItem(pProvider->DisplayName));
			VERIFY(m_ImportTree.SetItemData(htiRoot, DWORD(pProvider)));
			
			while (pImportChild)
			{
				str = CString(pImportChild->AccountDisplayName);
				str += CString(" (") +  CString(pImportChild->EmailAddress) + CString(")");

				VERIFY(htiChild = m_ImportTree.InsertItem(str, htiRoot));
				VERIFY(m_ImportTree.SetItemData(htiChild, DWORD(pImportChild)));
				pImportChild = pImportChild->next;
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

		pProvider = pProvider->next;
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
	ImportChild *pChild = NULL;

	if (hItem)
		pChild = (ImportChild *) m_ImportTree.GetItemData(hItem);

	CString txt = "There has been an internal error.";
	bool bMail = false, bAddr = false, bLDIF = false;

	if (NULL == pChild)
	{
		m_pParent->SetWizardButtons(PSWIZB_BACK);	
		
		txt = CString("Please select an account to import.");
		bMail = bAddr = bLDIF = false;
	}
	else
	{
		const ImportProvider *pProvider = pChild->parent;
		ASSERT(pProvider);

		if (pProvider)
		{
			const char *pProviderName = pProvider->DisplayName;
			ASSERT(pProviderName);

			if (pProviderName)
			{
				bMail = pProvider->hasMail;
				bAddr = pProvider->hasAddresses;
				bLDIF = pProvider->hasLdif;

				if (bLDIF) 
				{
					if (pChild->PathtoLdif)
					{
						m_LDIFEdt.SetWindowText(pChild->PathtoLdif);
					}
					else
					{
						m_LDIFEdt.SetWindowText("");
					}
				}

				if (false == bMail)
				{
					if ((false == bAddr) && (false == bLDIF))
					{
						txt.Format("%s does not have any mail or address book entries,"
							"only your account settings will be imported.", pProviderName);
					}
					else if (true == bAddr)
					{
						txt.Format("You may also import your address book entries from %s.", pProviderName);
					}
					else // (true == bLDIF)
					{
						txt.Format("You may also import LDIF entries from %s.", pProviderName);
					}
				}
				else // if (true == bMail)
				{
					if ((false == bAddr) && (false == bLDIF))
					{
						txt.Format("You may also import your mail from %s.", pProviderName);
					}
					else if (true == bAddr)
					{
						txt.Format("You may also import mail and address book entries from %s.", pProviderName);
					}
					else // (true == bLDIF)
					{
						txt.Format("You may also import mail and LDIF entries from %s.", pProviderName);
					}
				}

				if (true == bLDIF)
					txt += " (see manual LDIF help)";
			}
		}
	}

	m_AlsoStatic.SetWindowText(txt);
	m_MailChk.EnableWindow(bMail);
	m_AddrChk.EnableWindow(bAddr);
	m_LDIFChk.EnableWindow(bLDIF);

	// Only show one or the other (the overlap in the dlg)
	m_AddrChk.ShowWindow(bLDIF ? SW_HIDE : SW_SHOW);
	
	m_LDIFChk.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	m_LDIFEdt.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	m_BrowseBtn.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);

	UpdateNextBackBtns();
	OnLdifCheck();
}

// --------------------------------------------------------------------------

bool CWizardImportPage::CopySettings()
{
	ASSERT(m_pParent);
	ASSERT(m_pParent->m_pParams);

	HTREEITEM hItem = m_ImportTree.GetFirstCheckedItem();

	if (!hItem)
		return (false);
	
	ImportChild *pChild = (ImportChild *) m_ImportTree.GetItemData(hItem);
	
	bool bMail = ((m_MailChk.IsWindowVisible()) && (m_MailChk.IsWindowEnabled()) && (m_MailChk.GetCheck() != 0));
	bool bAddr = ((m_AddrChk.IsWindowVisible()) && (m_AddrChk.IsWindowEnabled()) && (m_AddrChk.GetCheck() != 0));
	bool bLDIF = ((m_LDIFChk.IsWindowVisible()) && (m_LDIFChk.IsWindowEnabled()) && (m_LDIFChk.GetCheck() != 0));
	
	CString sFileLDIF;
	sFileLDIF.Empty();

	if (bLDIF)
		m_LDIFEdt.GetWindowText(sFileLDIF);

	m_pParent->m_pParams->PersName       = CString(pChild->PersonalityName);
	m_pParent->m_pParams->RealName       = CString(pChild->RealName);
	m_pParent->m_pParams->ReturnAddress  = CString(pChild->EmailAddress);
	m_pParent->m_pParams->LoginName      = CString(pChild->IncomingUserName);
	m_pParent->m_pParams->InServer       = CString(pChild->IncomingServer);
	m_pParent->m_pParams->OutServer      = CString(pChild->SMTPServer);
	m_pParent->m_pParams->IMAPPrefix     = CString("");

	CPersParams::ServType type;
	type = (pChild->IsIMAP) ? (CPersParams::IST_IMAP) : (CPersParams::IST_POP);

	VERIFY(m_pParent->m_pParams->SetInServType(type));

	m_pParent->m_pImportChild = pChild;
	m_pParent->m_bImportMail = bMail;
	m_pParent->m_bImportAddr = bAddr;
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
