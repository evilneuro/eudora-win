// Dummy.cpp : implementation file
//

#include "stdafx.h"
#include "qvoice32.h"
#include "Dummy.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CDummy dialog


CDummy::CDummy(CWnd* pParent /*=NULL*/)
	: CDialog(CDummy::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDummy)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDummy::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDummy)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDummy, CDialog)
	//{{AFX_MSG_MAP(CDummy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDummy message handlers

BOOL CDummy::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
