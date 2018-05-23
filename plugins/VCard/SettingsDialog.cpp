// SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "testplugin.h"
#include "SettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog


CSettingsDialog::CSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDialog)
	m_AddToAddressBook = FALSE;
	m_AutoDecodeToAttachDir = FALSE;
	//}}AFX_DATA_INIT
}


void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDialog)
	DDX_Control(pDX, IDS_INI_AUTO_DECODE, m_AutoDecodeCheck);
	DDX_Control(pDX, IDS_INI_AUTO_ADD_TO_AB, m_AddToABCheck);
	DDX_Control(pDX, IDC_NICKNAMES_COMBO, m_NickCombo);
	DDX_Check(pDX, IDS_INI_AUTO_ADD_TO_AB, m_AddToAddressBook);
	DDX_Check(pDX, IDS_INI_AUTO_DECODE, m_AutoDecodeToAttachDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CSettingsDialog)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog message handlers


BOOL CSettingsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();	// Set up our controls if you would maestro.

	m_AddToAddressBook = true;
	
	GetControlsFromINI();

	return true;
} 

void CSettingsDialog::OnOK() 
{
	WriteControlsToINI();

	DestroyWindow();
}

void CSettingsDialog::OnCancel() 
{
	DestroyWindow();
}

void CSettingsDialog::OnDestroy() 
{
	CDialog::OnDestroy();
	m_MainApp->m_ShowingSettingsPanel = false;
}


void CSettingsDialog::WriteControlsToINI()
{
	char* settings = new char[m_NickCombo.GetLBTextLen(m_NickCombo.GetCurSel())+1];
	m_NickCombo.GetLBText(m_NickCombo.GetCurSel(), settings);
	WritePrivateProfileString(_T("Main"), _T("AddVCardsToThisAB"), settings, m_MainApp->m_INIFilename);
	delete [] settings;

	settings = new char[2];
	settings[1]= 0;
	if (m_AutoDecodeCheck.GetCheck())
		settings[0] = '1';
	else
		settings[0] = '0';

	WritePrivateProfileString(_T("Main"), _T("AutoDecodeVCards"), settings, m_MainApp->m_INIFilename);


	if (m_AddToABCheck.GetCheck())
		settings[0] = '1';
	else
		settings[0] = '0';

	WritePrivateProfileString(_T("Main"), _T("AutoAddtoAB"), settings, m_MainApp->m_INIFilename);	

	delete [] settings;
}

void CSettingsDialog::GetControlsFromINI()
{
	ABDataP ABs = m_MainApp->m_EudoraFuncs.EnumAB();
	ABDataP ABData = ABs;

	m_NickCombo.ResetContent();

	while (ABData && ABData->name)
	{
		m_NickCombo.AddString(ABData->name);
		ABData = ABData->next;
	}

	int nSize = 100;
	char * lpReturnedString = new char[nSize +1];

	GetPrivateProfileString(_T("Main"), _T("AddVCardsToThisAB"),_T("Eudora Nicknames"), lpReturnedString, nSize, m_MainApp->m_INIFilename);
	m_NickCombo.SelectString(-1, lpReturnedString);

	delete [] lpReturnedString;


	m_AddToABCheck.SetCheck(GetIniBool(IDS_INI_AUTO_ADD_TO_AB, m_MainApp->m_INIFilename));

	m_AutoDecodeCheck.SetCheck(GetIniBool(IDS_INI_AUTO_DECODE, m_MainApp->m_INIFilename));
}



BOOL GetIniBool(int Res, char* INIFilename)
{
	HINSTANCE hInstance;

	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		hInstance = AfxGetInstanceHandle( );
	}
	char *String = new char[260];
	char *KeyName;
	char *Default;
	bool Answer = true;

	if (!LoadString( hInstance, Res, String, 259))
	{
		delete [] String;
		return false;
	}

	char * division = strstr(String, "\n");
	if (division)
	{
		*division = 0;
		division += 1;
		KeyName = new char[strlen(String)+1];
		strcpy(KeyName, String);

		Default = new char[strlen(division)+1];
		strcpy(Default, division);
	}
	else 
	{
		KeyName = new char[strlen(String)+1];
		strcpy(KeyName, String);

		Default = new char[2];
		strcpy(KeyName, _T("0"));
	}

	delete [] String;


	char * Setting = new char[2];
	GetPrivateProfileString(_T("Main"), KeyName, Default, Setting, 2, INIFilename);
	if (Setting[0] == '0')
		Answer = false;
	delete [] KeyName;
	delete [] Default;
	delete [] Setting;

	return Answer;
}


/*
	GetPrivateProfileString(_T("Main"), _T("AddVCardsToAttachDir"),_T("0"), lpReturnedString, nSize, m_MainApp->m_INIFilename);
	if (lpReturnedString[0] == '0')
		m_AutoDecodeCheck.SetCheck(GetIniBool(IDS_INI_AUTO_ADD_TO_AB));
	else
		m_AutoDecodeCheck.SetCheck(TRUE);

	GetPrivateProfileString(_T("Stuff"), _T("AutoAddVCardstoAB"),_T("0"), lpReturnedString, nSize, m_MainApp->m_INIFilename);
	if (lpReturnedString[0] == '0')
		m_AddToABCheck.SetCheck(FALSE);
	else
		m_AddToABCheck.SetCheck(TRUE);	
*/