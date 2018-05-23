#include "stdafx.h"
#include "aboutdlg.h"

CAboutDlg::CAboutDlg(CWnd* pParentWnd) : CDialog(CAboutDlg::IDD, pParentWnd)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
        // No message handlers
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////
// OnInitDialog
//
////////////////////////////////////////////////////////////////////////
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//
	// Display the windows version as a static string.
	//
	char buf[80];
	DWORD version = ::GetVersion();
	CString format;
	format.LoadString( IDS_WIN_VER_FORMAT );
	
	wsprintf(buf, format, LOBYTE(LOWORD(version)), HIBYTE(LOWORD(version)));
	
	CStatic* p_text = (CStatic *) GetDlgItem(IDC_WINVERSION);
	ASSERT(p_text);
	p_text->SetWindowText(buf);

	return TRUE;	// return TRUE  unless you set the focus to a control
}


