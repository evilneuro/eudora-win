// InsertLinkDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "InsertLinkDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#include "Text2Html.h"

#define MAX_URL_LEN 1030

/////////////////////////////////////////////////////////////////////////////
// CInsertLinkDialog dialog


CInsertLinkDialog::CInsertLinkDialog(const char* szURL /*NULL*/, CWnd* pParent /*=NULL*/)
	: CDialog(CInsertLinkDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsertLinkDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	if (( szURL == NULL ) || ((szURL) && !(*szURL)))
		m_szURL = "http://";
	else
		m_szURL = szURL;
}


void CInsertLinkDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertLinkDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_INSERT_LINK_URL, m_szURL);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsertLinkDialog, CDialog)
	//{{AFX_MSG_MAP(CInsertLinkDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertLinkDialog message handlers

BOOL CInsertLinkDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	InitHtmlStrings();
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInsertLinkDialog::OnOK()
{
	UpdateData(TRUE);

	char buffer[MAX_URL_LEN];

	if (strnicmp(m_szURL, "www.", 4) == 0)
	{
		strcpy(buffer, "http://");
		strncat(buffer, m_szURL, MAX_URL_LEN-8);
		m_szURL = buffer;
	}
	else if (strnicmp(m_szURL, "ftp.", 4) == 0)
	{
		strcpy(buffer, "ftp://");
		strncat(buffer, m_szURL, MAX_URL_LEN-7);
		m_szURL = buffer;
	}
	
	if (IsURLScheme(m_szURL))
		CDialog::OnOK();
	else
		AfxMessageBox(IDS_ERR_INVALID_URL);
}
