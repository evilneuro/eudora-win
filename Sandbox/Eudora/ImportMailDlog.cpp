// ImportMailDlog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImportMailDlog.h"
#include "MAPIImport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportMailDlog dialog


CImportMailDlog::CImportMailDlog(CWnd* pParent /*=NULL*/)
	: CDialog(CImportMailDlog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportMailDlog)
	//}}AFX_DATA_INIT
}


void CImportMailDlog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportMailDlog)
	DDX_Control(pDX, IDC_ADDRESS_CHECK, m_AddressCheck);
	DDX_Control(pDX, IDC_MAIL_CHECK, m_MailCheck);
	DDX_Control(pDX, IDC_LDIF_CHECK, m_LDIFCheck);
	DDX_Control(pDX, IDC_LDIF_EDIT, m_LDIFEditCon);
	DDX_Control(pDX, IDC_IMPORT_TREE, m_Tree);
	DDX_Control(pDX, IDC_BROWSE_BTN, m_BrowseBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportMailDlog, CDialog)
	//{{AFX_MSG_MAP(CImportMailDlog)
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	ON_BN_CLICKED(IDC_IMPORT_MAIL_CANCEL, OnImportMailCancel)
	ON_BN_CLICKED(IDC_IMPORT_MAIL_OK, OnImportMailOk)
	ON_BN_CLICKED(IDC_LDIF_CHECK, OnLdifCheck)
	ON_REGISTERED_MESSAGE(msgTreeCtrlCheck_CheckChange, OnMsgTreeCheckChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportMailDlog message handlers

void CImportMailDlog::OnBrowseBtn() 
{
	CString str;
	m_LDIFEditCon.GetWindowText(str);
	
	CFileDialog dlg(TRUE, "LDIF", ((LPCSTR)str), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, "LDIF (*.LDIF)|*.LDIF|All Files (*.*)|*.*|", this);
	
	if (IDOK == dlg.DoModal())
	{
		CString pathname = dlg.GetPathName();

		if (!pathname.IsEmpty())
		{
			m_LDIFEditCon.SetWindowText(dlg.GetPathName());
//			UpdateNextBackBtns();
		}
	}
}

BOOL CImportMailDlog::OnInitDialog()
{

	CDialog::OnInitDialog();

	CFont ANSIFont, StatFont;
	ANSIFont.CreateStockObject(ANSI_VAR_FONT); // Get the stock variable-width font
	
	LOGFONT fontdata;
	ANSIFont.GetObject(sizeof(LOGFONT), &fontdata); // Get the font metrics

	fontdata.lfWeight = FW_BOLD; // Make it bold

	StatFont.CreateFontIndirect(&fontdata); // Create the static font

	GetDlgItem(IDC_IMPORT_TITLE_STATIC)->SetFont(&StatFont, FALSE);

	m_Tree.InitStateImageList(IDB_TREECTRL_CHECKMARKS, 16, RGB(255,255,255) );
	m_Tree.SetContainerFlag();
	m_Tree.SetSingleCheck();

	InitTree();
	
	m_LDIFCheck.ShowWindow(SW_HIDE);
	m_LDIFEditCon.ShowWindow(SW_HIDE);
	m_BrowseBtn.ShowWindow(SW_HIDE);

	m_MailCheck.EnableWindow(FALSE);
	m_AddressCheck.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}

LONG CImportMailDlog::OnMsgTreeCheckChange(WPARAM wParam, LPARAM lParam)
{
	SetupControls();

	return (0);
}

// --------------------------------------------------------------------------

void CImportMailDlog::SetupControls()
{

	HTREEITEM hItem = m_Tree.GetFirstCheckedItem();
	ImportChild *pChild = NULL;

	if (hItem)
		pChild = (ImportChild *) m_Tree.GetItemData(hItem);

	CString txt = "There has been an internal error.";
	bool bMail = false, bAddr = false, bLDIF = false;

	if (NULL == pChild)
	{
//		m_pParent->SetWizardButtons(PSWIZB_BACK);	
		
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
						m_LDIFEditCon.SetWindowText(pChild->PathtoLdif);
					}
					else
					{
						m_LDIFEditCon.SetWindowText("");
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

//	m_AlsoStatic.SetWindowText(txt);
	m_MailCheck.EnableWindow(bMail);
	m_AddressCheck.EnableWindow(bAddr);
	m_LDIFCheck.EnableWindow(bLDIF);

	// Only show one or the other (the overlap in the dlg)
	m_AddressCheck.ShowWindow(bLDIF ? SW_HIDE : SW_SHOW);
	
	m_LDIFCheck.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	m_LDIFEditCon.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	m_BrowseBtn.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);

//	UpdateNextBackBtns();
	OnLdifCheck();
}


bool CImportMailDlog::InitTree()
{
	m_pImporter = new CImportMail;

	m_Tree.DeleteAllItems();

	HTREEITEM htiRoot, htiChild;

	ImportProvider *pProvider = m_pImporter->m_head;
	ImportChild *pImportChild = NULL;

	CString str;

	while (pProvider)
	{
		pImportChild = pProvider->FirstChild;

		if (pImportChild)
		{
			VERIFY(htiRoot = m_Tree.InsertItem(pProvider->DisplayName));
			VERIFY(m_Tree.SetItemData(htiRoot, DWORD(pProvider)));
			
			while (pImportChild)
			{
				str = CString(pImportChild->AccountDisplayName);
				str += CString(" (") +  CString(pImportChild->EmailAddress) + CString(")");

				VERIFY(htiChild = m_Tree.InsertItem(str, htiRoot));
				VERIFY(m_Tree.SetItemData(htiChild, DWORD(pImportChild)));
				pImportChild = pImportChild->next;
			}
		}

		pProvider = pProvider->next;
	}

	m_Tree.ExpandAll();

	return true;
}

void CImportMailDlog::OnImportMailCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	
}

void CImportMailDlog::OnImportMailOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	bool bGetMail = (m_MailCheck.GetCheck() != 0);
	bool bGetAddr = (m_AddressCheck.GetCheck() != 0);
	bool bGetLDIF = (m_LDIFCheck.GetCheck() != 0);
	CString Filename;
	m_LDIFEditCon.GetWindowText(Filename);

	HTREEITEM hItem = m_Tree.GetFirstCheckedItem();
	ImportChild *pChild = NULL;

	if (hItem)
		pChild = (ImportChild *) m_Tree.GetItemData(hItem);

	if (pChild)
	{
		if (bGetMail)
			m_pImporter->ImportMail(pChild, NULL);

		if (bGetAddr)
			m_pImporter->ImportAddresses(pChild, NULL);
		else if (bGetLDIF && Filename.GetLength() > 0)
			m_pImporter->ImportLdif(pChild, Filename, NULL);
	}
}

void CImportMailDlog::OnLdifCheck() 
{
	// TODO: Add your control notification handler code here
	const bool bChecked = (m_LDIFCheck.GetCheck() != 0);

	m_LDIFEditCon.EnableWindow(bChecked);
	m_BrowseBtn.EnableWindow(bChecked);
}

