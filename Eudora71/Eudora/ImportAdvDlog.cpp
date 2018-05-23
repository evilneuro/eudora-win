// ImportMailDlog.cpp : implementation file
//
//includes yeah!
#include "stdafx.h"
#include "resource.h"
#include "ImportAdvDlog.h"
#include "MAPIImport.h"
#include "ImportMailDlog.h"
#include "guiutils.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CImportAdvDlog dialog


CImportAdvDlog::CImportAdvDlog(CWnd* pParent /*=NULL*/)
	: CDialog(CImportAdvDlog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportAdvDlog)
	//}}AFX_DATA_INIT
}

// DDX Stuff
void CImportAdvDlog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportAdvDlog)
	DDX_Control(pDX, IDC_IMPORT_PROGRAM_LISTBOX, m_ProgramListBox);
	DDX_Control(pDX, IDC_ADV_IMPORT_MAIL, m_MailEdit);
	DDX_Control(pDX, IDC_ADV_IMPORT_ADDR, m_AddrEdit);
	DDX_Control(pDX, IDC_ADV_IMPORT_BROWSE1, m_MailBrowseBtn);
	DDX_Control(pDX, IDC_ADV_IMPORT_BROWSE2, m_AddrBrowseBtn);
	//}}AFX_DATA_MAP
}

// Yeay Message Map
BEGIN_MESSAGE_MAP(CImportAdvDlog, CDialog)
	//{{AFX_MSG_MAP(CImportAdvDlog)
	ON_BN_CLICKED(IDC_ADV_IMPORT_BROWSE1, OnMailBrowseBtn)
	ON_BN_CLICKED(IDC_ADV_IMPORT_BROWSE2, OnAddrBrowseBtn)
	ON_BN_CLICKED(IDC_IMPORT_ADV_CANCEL, OnImportAdvCancel)
	ON_BN_CLICKED(IDC_IMPORT_ADV_OK, OnImportAdvOk)
	ON_LBN_SELCHANGE(IDC_IMPORT_PROGRAM_LISTBOX, OnSelchangeImportProgramList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportAdvDlog message handlers

// Initialization
BOOL CImportAdvDlog::OnInitDialog()
{

	CDialog::OnInitDialog();

	ImportDllStruct *pImportStruct = m_Importer.GetImportDllStruct();
	if(! pImportStruct)
	{
		m_Importer.InitPlugins();

	}

	pImportStruct = m_Importer.GetImportDllStruct();
	int iImportStructSize = m_Importer.GetDllStructSize();

	for(int i = 0; i < iImportStructSize; i++)
	{
		if(pImportStruct->pQueryFunction)
			m_ProgramListBox.AddString(pImportStruct[i].szProviderName);

	}
	// Both Edits are directories
	m_MailEdit.SetLimitText(_MAX_PATH);
	m_AddrEdit.SetLimitText(_MAX_PATH);

	if(m_ProgramListBox.GetCount() > 0)
		m_ProgramListBox.SetCurSel(0);
	
	OnSelchangeImportProgramList();

	m_ProgramListBox.SetFocus();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}
// When we hit the Mail browse button
void CImportAdvDlog::OnMailBrowseBtn()
{
	CString szSel;
	CString szMailPath;


	for (int i=0; i < m_ProgramListBox.GetCount(); i++)
	{
		if(m_ProgramListBox.GetSel(i) > 0)
			m_ProgramListBox.GetText(i, szSel);
	}

	ImportDllStruct *pImportStruct = m_Importer.GetImportDllStruct();
	if(! pImportStruct)
	{
		m_Importer.InitPlugins();

	}

	pImportStruct = m_Importer.GetImportDllStruct();
	int iImportStructSize = m_Importer.GetDllStructSize();

	for(i = 0; i < iImportStructSize; i++)
	{
		if(szSel.Compare(pImportStruct[i].szProviderName) == 0)
			break;

	}
	CFileDialog OFNdlg(TRUE,NULL,NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, pImportStruct[i].pMailExtensionFilter());

	if(OFNdlg.DoModal() == IDOK)
	{
		szMailPath = OFNdlg.GetPathName();
		GetDlgItem(IDC_ADV_IMPORT_MAIL)->SetWindowText(szMailPath);
	}

}
// Same as mail browse button above
void CImportAdvDlog::OnAddrBrowseBtn()
{
	CString szSel;
	CString szAddrPath;


	for (int i=0; i < m_ProgramListBox.GetCount(); i++)
	{
		if(m_ProgramListBox.GetSel(i) > 0)
			m_ProgramListBox.GetText(i, szSel);
	}

	ImportDllStruct *pImportStruct = m_Importer.GetImportDllStruct();
	if(! pImportStruct)
	{
		m_Importer.InitPlugins();

	}

	pImportStruct = m_Importer.GetImportDllStruct();
	int iImportStructSize = m_Importer.GetDllStructSize();

	for(i = 0; i < iImportStructSize; i++)
	{
		if(szSel.Compare(pImportStruct[i].szProviderName) == 0)
			break;

	}
	CFileDialog OFNdlg(TRUE,NULL,NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, pImportStruct[i].pAddrExtensionFilter());

	if(OFNdlg.DoModal() == IDOK)
	{
		szAddrPath = OFNdlg.GetPathName();
		GetDlgItem(IDC_ADV_IMPORT_ADDR)->SetWindowText(szAddrPath);
	}

}


// Cancel, outta here.
void CImportAdvDlog::OnImportAdvCancel()
{
	CDialog::OnCancel();
	
}
// Okay, we import stuff
void CImportAdvDlog::OnImportAdvOk()
{
	// bybye dialog
	CDialog::OnOK();
	// Both are directories
	char szMailPath[_MAX_PATH + 1];
	char szAddrPath[_MAX_PATH + 1];

	CString szSel;

	for (int i=0; i < m_ProgramListBox.GetCount(); i++)
	{
		if(m_ProgramListBox.GetSel(i) > 0)
			m_ProgramListBox.GetText(i, szSel);
	}

	ImportDllStruct *pImportStruct = m_Importer.GetImportDllStruct();
	if(! pImportStruct)
	{
		m_Importer.InitPlugins();

	}

	pImportStruct = m_Importer.GetImportDllStruct();
	int iImportStructSize = m_Importer.GetDllStructSize();

	for(i = 0; i < iImportStructSize; i++)
	{
		if(szSel.Compare(pImportStruct[i].szProviderName) == 0)
			break;

	}
	GetDlgItem(IDC_ADV_IMPORT_MAIL)->GetWindowText(szMailPath, _MAX_PATH);
	GetDlgItem(IDC_ADV_IMPORT_ADDR)->GetWindowText(szAddrPath, _MAX_PATH);
	if(strlen(szMailPath) > 0)
	{
		struct _stat st;

		if(_stat(szMailPath, &st) == -1)
		{
			ErrorDialog(8502); // file or path not found
			return;
		}
		//ImportMail2
		CImportChild Child;
		CImportProvider Provider;

		Provider.bHasAddresses = 0;
		Provider.bHasLdif = 0;
		Provider.bHasMail = 1;
		Provider.iNumChildren = 1;
		Provider.lpszDisplayName = DEBUG_NEW char [strlen(szSel) + 1];
		strcpy(Provider.lpszDisplayName, szSel);
		Provider.pFirstChild = &Child;
		Provider.pNext = NULL;

		Child.bHasAddresses = 0;
		Child.bHasLdif = 0;
		Child.bHasMail = 1;
		Child.bIsIMAP = 0;
		Child.bLMOS = 0;
		Child.iAccountID = 0;
		Child.lpszAccountDisplayName = NULL;
		Child.lpszEmailAddress = NULL;
		Child.lpszIdentityName = NULL;
		Child.lpszIncomingServer = NULL;
		Child.lpszIncomingUserName = NULL;
		Child.lpszPathToAdrData = NULL;
		Child.lpszPathtoLdif = NULL;
		Child.lpszPersonalityName = NULL;
		Child.lpszRealName = NULL;
		Child.lpszSMTPServer = NULL;
		Child.m_szIdentityUID[0] = 0;
		Child.pNext = NULL;
		Child.pParent = &Provider;

		Child.lpszPathToMailData = DEBUG_NEW char[strlen(szMailPath) + 1];
		strcpy(Child.lpszPathToMailData, szMailPath);
		//char *floater = strrchr(Child.lpszPathToMailData, '\\');
		//if(floater)
		//	floater[1] = 0;

		m_Importer.ImportMail2(&Child, NULL);
		delete [] Child.lpszPathToMailData;
		Child.lpszPathToMailData = NULL;

		delete [] Provider.lpszDisplayName;
		Provider.lpszDisplayName = NULL;

	}
	if(strlen(szAddrPath) > 0)
	{
		struct _stat st;

		if(_stat(szAddrPath, &st) == -1)
		{
			ErrorDialog(8502); // file or path not found
			return;
		}

		// Import addressbook
		CImportChild Child;
		CImportProvider Provider;

		Provider.bHasAddresses = 0;
		Provider.bHasLdif = 0;
		Provider.bHasMail = 1;
		Provider.iNumChildren = 1;
		Provider.lpszDisplayName = DEBUG_NEW char [strlen(szSel) + 1];
		strcpy(Provider.lpszDisplayName, szSel);
		Provider.pFirstChild = &Child;
		Provider.pNext = NULL;
		
		Child.bHasAddresses = 0;
		Child.bHasLdif = 0;
		Child.bHasMail = 1;
		Child.bIsIMAP = 0;
		Child.bLMOS = 0;
		Child.iAccountID = 0;
		Child.lpszAccountDisplayName = NULL;
		Child.lpszEmailAddress = NULL;
		Child.lpszIdentityName = NULL;
		Child.lpszIncomingServer = NULL;
		Child.lpszIncomingUserName = NULL;
		Child.lpszPathtoLdif = NULL;
		Child.lpszPersonalityName = NULL;
		Child.lpszRealName = NULL;
		Child.lpszSMTPServer = NULL;
		Child.m_szIdentityUID[0] = 0;
		Child.pNext = NULL;
		Child.lpszPathToMailData = NULL;
		Child.pParent = &Provider;

		Child.lpszPathToAdrData = DEBUG_NEW char[strlen(szAddrPath) + 1];
		strcpy(Child.lpszPathToAdrData, szAddrPath);

		m_Importer.ImportAddresses2(&Child, NULL);

		delete [] Provider.lpszDisplayName;
		Provider.lpszDisplayName= NULL;

		delete [] Child.lpszPathToAdrData;
		Child.lpszPathToAdrData = NULL;
		

	}

}



void CImportAdvDlog::OnSelchangeImportProgramList() 
{
	CString szSel;

	for (int i=0; i < m_ProgramListBox.GetCount(); i++)
	{
		if(m_ProgramListBox.GetSel(i) > 0)
			m_ProgramListBox.GetText(i, szSel);
	}

	ImportDllStruct *pImportStruct = m_Importer.GetImportDllStruct();

	if(! pImportStruct)
	{
		m_Importer.InitPlugins();

	}

	pImportStruct = m_Importer.GetImportDllStruct();

	int iImportStructSize = m_Importer.GetDllStructSize();

	for(i = 0; i < iImportStructSize; i++)
	{
		if(szSel.Compare(pImportStruct[i].szProviderName) == 0)
			break;

	}
	CString strTemp ;
	strTemp.Format( IDS_IMPORT_ADV_LOCATE_FILE, pImportStruct[i].pAddrExtension());
	GetDlgItem(IDC_IMPORT_ADV_ADDR_STATIC)->SetWindowText(strTemp);

	strTemp.Empty();
	strTemp.Format( IDS_IMPORT_ADV_LOCATE_FILE, pImportStruct[i].pMailExtension());
	GetDlgItem(IDC_IMPORT_ADV_MAIL_STATIC)->SetWindowText(strTemp);

	GetDlgItem(IDC_ADV_IMPORT_ADDR)->SetWindowText("");
	GetDlgItem(IDC_ADV_IMPORT_MAIL)->SetWindowText("");

		
}
