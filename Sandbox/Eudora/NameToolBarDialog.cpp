// NameToolBarDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "NameToolBarDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CNameToolBarDialog dialog


CNameToolBarDialog::CNameToolBarDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNameToolBarDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameToolBarDialog)
	m_szName = _T("");
	//}}AFX_DATA_INIT
}


void CNameToolBarDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameToolBarDialog)
	DDX_Text(pDX, IDC_TOOL_BAR_NAME, m_szName);
	DDV_MaxChars(pDX, m_szName, 15);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNameToolBarDialog, CDialog)
	//{{AFX_MSG_MAP(CNameToolBarDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNameToolBarDialog message handlers
