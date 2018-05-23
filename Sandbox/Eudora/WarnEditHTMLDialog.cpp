// WarnEditHTMLDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "WarnEditHTMLDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CWarnEditHTMLDialog dialog


CWarnEditHTMLDialog::CWarnEditHTMLDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CWarnEditHTMLDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWarnEditHTMLDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWarnEditHTMLDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWarnEditHTMLDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWarnEditHTMLDialog, CDialog)
	//{{AFX_MSG_MAP(CWarnEditHTMLDialog)
	ON_BN_CLICKED(IDC_EDIT_STYLED_TEXT, OnEditStyledText)
	ON_BN_CLICKED(IDC_EDIT_HTML, OnEditHtml)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWarnEditHTMLDialog message handlers

void CWarnEditHTMLDialog::OnEditStyledText() 
{
	EndDialog( IDC_EDIT_STYLED_TEXT );
}

void CWarnEditHTMLDialog::OnEditHtml() 
{
	EndDialog( IDC_EDIT_HTML );
}
