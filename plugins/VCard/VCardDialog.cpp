// VCardDialog.cpp : implementation file
//

#include "stdafx.h"
#include "testplugin.h"
#include "VCardDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVCardDialog dialog


CVCardDialog::CVCardDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CVCardDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVCardDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVCardDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVCardDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVCardDialog, CDialog)
	//{{AFX_MSG_MAP(CVCardDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVCardDialog message handlers
