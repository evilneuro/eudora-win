// Imapldlg.cpp : implementation file
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

// #include "eudora.h"
#include "resource.h"
#include "Imapdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CImapLoginDlg dialog


CImapLoginDlg::CImapLoginDlg(LPCSTR pServer, CWnd* pParent /*=NULL*/)
	: CDialog(CImapLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImapLoginDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_Login.Empty();
	m_Password.Empty();
	m_Server.Empty();

	if (pServer)
		m_Server = pServer;
}


void CImapLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImapLoginDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
		DDX_Text (pDX, IDC_IMAP_LOGIN, m_Login);
		DDV_MaxChars (pDX, m_Login, MAX_IMAP_LOGIN);
		DDV_MaxChars (pDX, m_Password, MAX_IMAP_PASSWORD);
		DDX_Text (pDX, IDC_IMAP_PASSWORD, m_Password);
	//}}AFX_DATA_MAP
}



BOOL CImapLoginDlg::OnInitDialog()
{

	// Initialize control text:

	if (!m_Server.IsEmpty())
		GetDlgItem(IDC_IMAP_SERVER)->SetWindowText (m_Server);

	if (!m_Login.IsEmpty())
	{
		GetDlgItem(IDC_IMAP_LOGIN)->SetWindowText (m_Login);
		GetDlgItem (IDC_IMAP_PASSWORD)->SetFocus();
	}
	else
	{
		// Set the focus to the login control.
		GetDlgItem (IDC_IMAP_LOGIN)->SetFocus();
	}

	// We set the focus, so return FALSE;
	return FALSE;
}



BEGIN_MESSAGE_MAP(CImapLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CImapLoginDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CImapLoginDlg message handlers


#endif //IMAP4


