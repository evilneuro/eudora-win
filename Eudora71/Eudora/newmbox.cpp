// NEWMBOX.CPP
//

#include "stdafx.h"
#include "eudora.h"

#include "guiutils.h"
#include "newmbox.h"
#include "QCMailboxDirector.h"
#include "rs.h"

#include "DebugNewHelpers.h"

extern QCMailboxDirector	g_theMailboxDirector;

/////////////////////////////////////////////////////////////////////////////
// CNewMailboxDlg dialog


CNewMailboxDlg::CNewMailboxDlg(LPCSTR szParentName, CPtrList* pList, BOOL IsTransfer)
	: CHelpxDlg(CNewMailboxDlg::IDD, AfxGetMainWnd()), m_IsTransfer(IsTransfer)
{
	//{{AFX_DATA_INIT(CNewMailboxDlg)
	m_MakeFolder = FALSE;
	m_DontTransfer = FALSE;
	m_Prompt = "";
	m_Name = "";
	//}}AFX_DATA_INIT
	m_szParentName = szParentName;
	m_pChildList = pList;
}

BOOL CNewMailboxDlg::OnInitDialog()
{
	char Prompt[128];
				
	GetDlgItem(IDC_MB_NEW_PROMPT)->GetWindowText(m_Prompt);

	if( m_szParentName != NULL )
	{
		wsprintf(Prompt, m_Prompt, m_szParentName );
	}
	else
	{
		strcpy(Prompt, CRString(IDS_MAILBOXES_CREATE_TOP));
	}
	m_Prompt = Prompt;

	// Hide the "Don't transfer" checkbox if not doing a transfer
	if (!m_IsTransfer)
	{
		GetDlgItem(IDC_DONT_TRANSFER)->ShowWindow(SW_HIDE);
	}
		                                                    
	((CEdit *)GetDlgItem(IDC_MB_NEW_EDIT))->LimitText(31);
	
	CenterWindow();

	return(CHelpxDlg::OnInitDialog());
}

void CNewMailboxDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	
	//{{AFX_DATA_MAP(CNewMailboxDlg)
	DDX_Check(pDX, IDC_MAKE_FOLDER, m_MakeFolder);
	DDX_Check(pDX, IDC_DONT_TRANSFER, m_DontTransfer);
	DDX_Text(pDX, IDC_MB_NEW_PROMPT, m_Prompt);
	DDX_Text(pDX, IDC_MB_NEW_EDIT, m_Name);
	DDV_MaxChars(pDX, m_Name, 31);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewMailboxDlg, CHelpxDlg)
	//{{AFX_MSG_MAP(CNewMailboxDlg)
	ON_BN_CLICKED(IDC_MAKE_FOLDER, OnMakeFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNewMailboxDlg message handlers

void CNewMailboxDlg::OnOK()
{
	// Get entered name into m_Name
	if (!UpdateData())
		return;
	
	// Empty name, don't exit
	if (m_Name.IsEmpty())
		return;

	// Commas, backslashes and &s are a no-no
	if (m_Name.FindOneOf(",&\\") >= 0)
	{
		ErrorDialog(IDS_MAILBOXES_BAD_CHARACTER);
		return;
	}

	if( g_theMailboxDirector.FindByName( m_pChildList, m_Name ) != NULL ) 
	{
		ErrorDialog(IDS_ERR_MAILBOX_EXISTS, (const char*)m_Name);
		return;
    }
    
	CHelpxDlg::OnOK();
}

void CNewMailboxDlg::OnMakeFolder()
{
	if (IsDlgButtonChecked(IDC_MAKE_FOLDER))
	{
		CheckDlgButton(IDC_DONT_TRANSFER, TRUE);
		GetDlgItem(IDC_DONT_TRANSFER)->EnableWindow(FALSE);
	}
	else
		GetDlgItem(IDC_DONT_TRANSFER)->EnableWindow(TRUE);
}


