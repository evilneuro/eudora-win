// RegistrationCodeDlg.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "guiutils.h"

#include "RegistrationCodeDlg.h"

#include "QCSharewareManager.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CRegistrationCodeDlg dialog


CRegistrationCodeDlg::CRegistrationCodeDlg(CWnd* pParent /*=NULL*/)
	:	CDialog(CRegistrationCodeDlg::IDD, pParent),
		m_MessageTitle(IDS_REG_DIALOG_MSG_TITLE_THANKS),
		m_Message(IDS_REG_DIALOG_MSG_NORMAL)
{
	//{{AFX_DATA_INIT(CRegistrationCodeDlg)
	m_FirstName = QCSharewareManager::GetFirstNameForCurrentMode();
	m_LastName = QCSharewareManager::GetLastNameForCurrentMode();
	m_Code = QCSharewareManager::GetRegCodeForCurrentMode();
	//}}AFX_DATA_INIT
}


CRegistrationCodeDlg::CRegistrationCodeDlg(
	const char *	szFirstName,
	const char *	szLastName,
	const char *	szRegCode,
	UINT			nMessageTitleStringID,
	UINT			nMessageStringID,
	CWnd *			pParent /*=NULL*/)
	:	CDialog(CRegistrationCodeDlg::IDD, pParent),
		m_MessageTitle(nMessageTitleStringID), m_Message(nMessageStringID)
{
	//{{AFX_DATA_INIT(CRegistrationCodeDlg)
	m_FirstName = szFirstName;
	m_LastName = szLastName;
	m_Code = szRegCode;
	//}}AFX_DATA_INIT
}


void CRegistrationCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegistrationCodeDlg)
	DDX_Text(pDX, IDC_MESSAGE_TITLE, m_MessageTitle);
	DDX_Text(pDX, IDC_MESSAGE, m_Message);
	DDX_Text(pDX, IDC_FIRST_NAME, m_FirstName);
	DDX_Text(pDX, IDC_LAST_NAME, m_LastName);
	DDX_Text(pDX, IDC_CODE, m_Code);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegistrationCodeDlg, CDialog)
	//{{AFX_MSG_MAP(CRegistrationCodeDlg)
	ON_BN_CLICKED(IDC_LOST_CODE, OnLostCode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegistrationCodeDlg message handlers

void CRegistrationCodeDlg::OnOK() 
{
	//	Call UpdateData first to get the information into our fields
	UpdateData();

	SharewareModeType	newRegMode;
	if ( !QCSharewareManager::IsValidRegistrationInfo(m_FirstName, m_LastName, m_Code, &newRegMode) )
	{
		// Invalid reg code
		ErrorDialog(IDS_ERR_INVALID_REGCODE);
		return;
	}

	CDialog::OnOK();
	
	QCSharewareManager::SetRegistrationInfo(m_FirstName, m_LastName, m_Code, newRegMode);
}

void CRegistrationCodeDlg::OnLostCode()
{
	LaunchURLWithQuery(NULL, ACTION_LOST_CODE);
}
