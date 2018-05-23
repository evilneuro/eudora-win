// ImportMailDlog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImportMailDlog.h"
#include "ImportAdvDlog.h"
#include "MAPIImport.h"
#include "guiutils.h"
#include "utils.h"  //for QCLoadString

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CImportMailDlog dialog


CImportMailDlog::CImportMailDlog(CWnd* pParent /*=NULL*/)
	: CDialog(CImportMailDlog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportMailDlog)
	//}}AFX_DATA_INIT
	m_pImporter = DEBUG_NEW CImportMail;
}
CImportMailDlog::~CImportMailDlog()
{
	delete m_pImporter;

}

void CImportMailDlog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportMailDlog)
//	DDX_Control(pDX, IDC_ADDRESS_CHECK, m_AddressCheck);
//	DDX_Control(pDX, IDC_MAIL_CHECK, m_MailCheck);
//	DDX_Control(pDX, IDC_LDIF_CHECK, m_LDIFCheck);
//	DDX_Control(pDX, IDC_LDIF_EDIT, m_LDIFEditCon);
	DDX_Control(pDX, IDC_IMPORT_TREE, m_Tree);
//	DDX_Control(pDX, IDC_BROWSE_BTN, m_BrowseBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportMailDlog, CDialog)
	//{{AFX_MSG_MAP(CImportMailDlog)
	//ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	ON_BN_CLICKED(IDC_IMPORT_MAIL_CANCEL, OnImportMailCancel)
	ON_BN_CLICKED(IDC_IMPORT_MAIL_OK, OnImportMailOk)
	ON_BN_CLICKED(IDC_IMPORT_MAIL_ADVANCED,OnImportMailAdvanced)
	//ON_BN_CLICKED(IDC_LDIF_CHECK, OnLdifCheck)
	ON_REGISTERED_MESSAGE(msgTreeCtrlCheck_CheckChange, OnMsgTreeCheckChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportMailDlog message handlers

/*void CImportMailDlog::OnBrowseBtn() 
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
}*/

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

	m_Tree.InitStateImageList(IDB_TREECTRL_CHECKMARKS, 16, RGB(0,255,255) );
	m_Tree.SetContainerFlag();
	m_Tree.SetSingleCheck();

	InitTree();
	
//	m_LDIFCheck.ShowWindow(SW_HIDE);
//	m_LDIFEditCon.ShowWindow(SW_HIDE);
//	m_BrowseBtn.ShowWindow(SW_HIDE);

//	m_MailCheck.EnableWindow(FALSE);
//	m_AddressCheck.EnableWindow(FALSE);

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
	CImportChild *pChild = NULL;

	if (hItem)
		pChild = (CImportChild *) m_Tree.GetItemData(hItem);

	// The string called "txt" is carefully updated and then never used. The code is nearly
	// identical to the function by the same name in WizardImportPage.cpp, except that in the
	// latter, the line that actually uses txt isn't commented out.
//	CString txt = "There has been an internal error.";

	//bool bMail = false, bAddr = false, bLDIF = false;
//	bool bLDIF = false;

//	if (NULL == pChild)
//	{
//		m_pParent->SetWizardButtons(PSWIZB_BACK);	
		
//		txt = CString("Please select an account to import.");
		//bMail = bAddr = bLDIF = false;
//		bLDIF = false;
//	}
//	else
	if (pChild)
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
				//bLDIF = pProvider->bHasLdif;

				/*if (bLDIF) 
				{
					if (pChild->lpszPathtoLdif)
					{
						m_LDIFEditCon.SetWindowText(pChild->lpszPathtoLdif);
					}
					else
					{
						m_LDIFEditCon.SetWindowText("");
					}
				}*/

//				if (false == bMail)
//				{	
//					if ((false == bAddr) && (false == bLDIF))
//					{
//						txt.Format("%s does not have any mail or address book entries,"
//							"only your account settings will be imported.", pProviderName);
//					}
//					else if (true == bAddr)
//					{
//						txt.Format("You may also import your address book entries from %s.", pProviderName);
//					}
//					else // (true == bLDIF)
//					{
//						txt.Format("You may also import LDIF entries from %s.", pProviderName);
//					}
//				}
//				else // if (true == bMail)
//				{
//					if ((false == bAddr) && (false == bLDIF))
//					{
//						txt.Format("You may also import your mail from %s.", pProviderName);
//					}
//					else if (true == bAddr)
//					{
//						txt.Format("You may also import mail and address book entries from %s.", pProviderName);
//					}
//					else // (true == bLDIF)
//					{
//						txt.Format("You may also import mail and LDIF entries from %s.", pProviderName);
//					}
//				}
//
//				if (true == bLDIF)
//					txt += " (see manual LDIF help)";
			}
		}
	}

//	m_AlsoStatic.SetWindowText(txt);

/*	m_MailCheck.EnableWindow(bMail);
	m_AddressCheck.EnableWindow(bAddr);*/

	//m_LDIFCheck.EnableWindow(bLDIF);

	// Only show one or the other (the overlap in the dlg)
	//m_AddressCheck.ShowWindow(bLDIF ? SW_HIDE : SW_SHOW);
	
	//m_LDIFCheck.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	//m_LDIFEditCon.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);
	//m_BrowseBtn.ShowWindow(bLDIF ? SW_SHOW : SW_HIDE);

//	UpdateNextBackBtns();
	//OnLdifCheck();
}

void CImportMailDlog::OnImportMailAdvanced() 
{
	CDialog::OnCancel();

	CImportAdvDlog AdvDlog;

	if(AdvDlog.m_Importer.InitPlugins())
	{
		AdvDlog.m_Importer.InitProviders(true);
		AdvDlog.DoModal();
	}
	else
	{
		CString szUrlSite;
		GetJumpURL(&szUrlSite);
		ConstructURLWithQuery(szUrlSite, ACTION_SUPPORT, _T("no-importer"));

		WarnUrlDialog(CRString(IDR_MAINFRAME), CRString(IDS_ERR_IMPORT_DLL_NOTINSTALLED), CRString(IDS_TECHSPPT_TITLE), szUrlSite);
	}
}

bool CImportMailDlog::InitTree()
{

	m_Tree.DeleteAllItems();

	HTREEITEM htiRoot, htiChild;

	CImportProvider * pProvider = m_pImporter->m_newhead;
	CImportChild * pImportChild = NULL;

	CString str;

	while (pProvider)
	{
		pImportChild = pProvider->pFirstChild;

		if (pImportChild)
		{
			VERIFY(htiRoot = m_Tree.InsertItem(pProvider->lpszDisplayName));
			VERIFY(m_Tree.SetItemData(htiRoot, DWORD(pProvider)));
			
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

				VERIFY(htiChild = m_Tree.InsertItem(str, htiRoot));
				VERIFY(m_Tree.SetItemData(htiChild, DWORD(pImportChild)));
				pImportChild = pImportChild->pNext;
			}
		}

		pProvider = pProvider->pNext;
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

//	bool bGetMail = (m_MailCheck.GetCheck() != 0);
//	bool bGetAddr = (m_AddressCheck.GetCheck() != 0);


	bool bGetMail = false;
	bool bGetAddr = false;
	bool bGetPersona = false;
	bool bIsProfile = false;
	
	//bool bGetLDIF = (m_LDIFCheck.GetCheck() != 0);
	//CString Filename;
	//m_LDIFEditCon.GetWindowText(Filename);

	HTREEITEM hItem = m_Tree.GetFirstCheckedItem();
	CImportChild *pChild = NULL;

	if (hItem)
		pChild = (CImportChild *) m_Tree.GetItemData(hItem);

	if (pChild)
	{

		ImportType kImpType = pChild->kImportType;

		switch (kImpType) {
			case MBOX_ABOOK_PERSONA: {
				bGetMail = true;
				bGetAddr = true;
				bGetPersona = true;
				bIsProfile = true;
				break;
			}
			case MBOX_ABOOK: {
				bGetMail = true;
				bGetAddr = true;
				break;
			}
			case PERSONA: {
				bGetPersona = true;
				break;
			}
			case MBOX: {
				bGetMail = true;
				break;
			}
			default: {
				ASSERT(0);
				break;
			}

		}

		if (bGetMail)
		{
			m_pImporter->ImportMail2(pChild, NULL);
		}

		if (bGetAddr)
		{
			m_pImporter->ImportAddresses2(pChild, NULL);
		}
		/*else 
		if (bGetLDIF && Filename.GetLength() > 0)
		{
			pChild->lpszPathToAdrData =  Filename.GetBuffer(Filename.GetLength());

			m_pImporter->ImportAddresses2(pChild, NULL);
			pChild->lpszPathToAdrData = NULL;
	
		}*/

		if (bGetPersona) {
			m_pImporter->ImportPersonalities2(pChild, bIsProfile, NULL);
		}

	}
}

/*void CImportMailDlog::OnLdifCheck() 
{
	// TODO: Add your control notification handler code here
	const bool bChecked = (m_LDIFCheck.GetCheck() != 0);

	m_LDIFEditCon.EnableWindow(bChecked);
	m_BrowseBtn.EnableWindow(bChecked);
}*/

