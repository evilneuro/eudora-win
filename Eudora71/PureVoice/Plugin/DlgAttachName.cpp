// DlgAttachName.cpp : implementation file
//
//  03.24.97   lss   Initial

#include "stdafx.h"
#include "qvoice32.h"
#include "DlgAttachName.h"
/*
#include "DebugNewHelpers.h"
*/
/////////////////////////////////////////////////////////////////////////////
// CDlgAttachName dialog


CDlgAttachName::CDlgAttachName(CString& attachName, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAttachName::IDD, pParent), m_OrigName(attachName)
{
	//{{AFX_DATA_INIT(CDlgAttachName)
	m_AttachName = _T("");
	//}}AFX_DATA_INIT
}


void CDlgAttachName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAttachName)
	DDX_Text(pDX, IDC_ATTACH_NAME, m_AttachName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAttachName, CDialog)
	//{{AFX_MSG_MAP(CDlgAttachName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAttachName message handlers

void CDlgAttachName::OnOK() 
{
	CDialog::OnOK();
	int	ret = IDOK;
	CString pathName = m_OrigName.Left( m_OrigName.ReverseFind( '\\' ) );
	m_FinalAttachName = m_AttachName;
	// strip path name if necessary since path name is not valid
	if ( m_FinalAttachName.ReverseFind( '\\' ) > -1 )
	 {
		m_FinalAttachName = m_FinalAttachName.Right(
							m_FinalAttachName.GetLength()
							- m_FinalAttachName.ReverseFind( '\\' )-1 );
	 }
	// do we need to add ".qcp" extension?
	CString tmp = m_FinalAttachName.Right(4);
	tmp.MakeUpper();
	if ( CString(".QCP") == tmp )
		m_FinalAttachName.Format( "%s\\%s", LPCTSTR(pathName), 
														m_FinalAttachName );
	else
		m_FinalAttachName.Format( "%s\\%s.QCP", LPCTSTR(pathName), 
														m_FinalAttachName );

	// check if filename already exist and
	// ask user if they want to replace it or not
	if ( m_FinalAttachName != m_OrigName ) 
	 {
		FILE* fp;
		if ( fp = fopen( LPCTSTR(m_FinalAttachName), "r" ) )
		 {
			fclose( fp );
			ret = AfxMessageBox( 
					"This name already exists. Do you wish to replace it?",
					MB_YESNO );
			if ( IDNO == ret )
				ret = IDRETRY;
			else
				ret = IDOK;
		 }	
	 }
	EndDialog( ret );
}

BOOL CDlgAttachName::OnInitDialog() 
{
	// get rid of path name
	m_AttachName = m_OrigName.Right( m_OrigName.GetLength()
					- m_OrigName.ReverseFind( '\\' )-1 );
	// also get rid of .qcp extension
	m_AttachName = m_AttachName.Left( m_AttachName.GetLength() - 4 );

	CDialog::OnInitDialog();

	SetWindowText( APP_TITLE );

	// select the text
	((CEdit*)GetDlgItem(IDC_ATTACH_NAME))->SetSel( 0, -1 );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
