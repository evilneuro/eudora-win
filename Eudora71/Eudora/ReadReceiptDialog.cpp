

#include "stdafx.h"
#include "eudora.h"
#include "ReadReceiptDialog.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CReadReceiptDlg dialog


CReadReceiptDlg::CReadReceiptDlg(
LPCSTR	szTitle,
CWnd*	pParent /*=NULL*/) : CDialog(CReadReceiptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReadReceiptDlg)
	//}}AFX_DATA_INIT

	m_sReadRecptStatic = szTitle;
}


void CReadReceiptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReadReceiptDlg)
	DDX_Text(pDX, IDC_READ_RECPT_DLG, m_sReadRecptStatic);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReadReceiptDlg, CDialog)
	//{{AFX_MSG_MAP(CReadReceiptDlg)
	ON_BN_CLICKED(IDC_RR_LATER, OnRrLater)
	ON_BN_CLICKED(IDC_RR_NEVER, OnRrNever)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadReceiptDlg message handlers

void CReadReceiptDlg::OnRrLater() 
{
	CDialog::EndDialog(IDRETRY);	
}

void CReadReceiptDlg::OnRrNever() 
{
	CDialog::EndDialog(IDABORT);	
}

void CReadReceiptDlg::OnCancel() 
{	
	CDialog::OnCancel();
}

void CReadReceiptDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

BOOL CReadReceiptDlg::OnInitDialog() 
{
	return CDialog::OnInitDialog();
}

