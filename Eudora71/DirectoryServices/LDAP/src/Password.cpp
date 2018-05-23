// PASSWORD.CPP
//
// Routines for changing the password
//

//#include "stdafx.h"
#include "afx.h"

#include "resource.h"
#include "Password.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define MAX_PASSWORD_SIZE			128
#define MAX_ENCODED_PASSWORD_SIZE	(MAX_PASSWORD_SIZE * 4 / 3)

CPasswordDialog::CPasswordDialog(CWnd* pParent /*=NULL*/)
	:  CDialog(CPasswordDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPasswordDialog)
	m_Password = "";
	m_Account = "";
	//}}AFX_DATA_INIT
}

void CPasswordDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPasswordDialog)
	DDX_Control(pDX, IDC_PASSWORD_PROMPT, m_Prompt);
	DDX_Text(pDX, IDC_PASSWORD, m_Password);
	DDV_MaxChars(pDX, m_Password, MAX_PASSWORD_SIZE - 1);
	DDX_Text(pDX, IDC_PASSWORD_ACCOUNT, m_Account);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasswordDialog, CDialog)
	//{{AFX_MSG_MAP(CPasswordDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog message handlers

BOOL CPasswordDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	if (m_PromptID != 0)
		m_Prompt.SetWindowText("Test");
	
	CenterWindow(AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPasswordDialog::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

// GetPassword
// Get the POP account password from the user
//
BOOL GetPassword(CString& NewPassword, UINT Prompt /*= 0*/, BOOL ChangePass /*= FALSE*/, const char* Login /*= NULL*/)
{
	// Must call AFX_MANAGE_STATE before dialog is loaded so that MFC
	// can find resources in the current DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	CPasswordDialog PassDialog;
	int Result;
	
	PassDialog.m_Account = Login;
	PassDialog.m_PromptID = Prompt;
		
	Result = PassDialog.DoModal();
        
	if (Result == IDOK && !PassDialog.m_Password.IsEmpty())
	{
		NewPassword = PassDialog.m_Password;
		return (TRUE);
	}

	return (FALSE);
}


