// NewSignatureDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "NewSignatureDialog.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewSignatureDialog dialog


CNewSignatureDialog::CNewSignatureDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNewSignatureDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewSignatureDialog)
		m_szName = _T("");
	//}}AFX_DATA_INIT
}


void CNewSignatureDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSignatureDialog)
	DDX_Text(pDX, IDC_TOOL_SIGNATURE_NEW, m_szName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewSignatureDialog, CDialog)
	//{{AFX_MSG_MAP(CNewSignatureDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSignatureDialog message handlers

void CNewSignatureDialog::OnOK() 
{
	UpdateData(TRUE);

	if ( m_szName.IsEmpty() )
		AfxMessageBox(IDS_ERR_SIGNATURE_NAME);
	else
		CDialog::OnOK();
}
