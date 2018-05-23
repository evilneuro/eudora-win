// RInstDlg.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "RInstDlg.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CRemoteInstance dialog


CRemoteInstance::CRemoteInstance(CWnd* pParent /*=NULL*/)
	: CDialog(CRemoteInstance::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRemoteInstance)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRemoteInstance::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRemoteInstance)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRemoteInstance, CDialog)
	//{{AFX_MSG_MAP(CRemoteInstance)
	ON_BN_CLICKED(IDC_TERMINATE_LOCAL, OnTerminateLocal)
	ON_BN_CLICKED(IDC_TERMINATE_REMOTE, OnTerminateRemote)
	ON_BN_CLICKED(IDC_CONTINUE, OnContinue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteInstance message handlers

void CRemoteInstance::OnTerminateLocal() 
{
	m_Result = TERMINATE_LOCAL;

	CDialog::EndDialog( m_Result );
}

void CRemoteInstance::OnTerminateRemote() 
{
	m_Result = TERMINATE_REMOTE;

	CDialog::EndDialog( m_Result );
}

void CRemoteInstance::OnContinue() 
{
	m_Result = IGNORE_REMOTE;

	CDialog::EndDialog( m_Result );
}
