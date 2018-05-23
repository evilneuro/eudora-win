// FaceNameDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dlgs.h"
#include "FaceNameDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaceNameDialog

IMPLEMENT_DYNAMIC(CFaceNameDialog, CFontDialog)

CFaceNameDialog::CFaceNameDialog(LPLOGFONT lplfInitial, DWORD dwFlags, CDC* pdcPrinter, CWnd* pParentWnd) : 
	CFontDialog(lplfInitial, dwFlags, pdcPrinter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CFaceNameDialog, CFontDialog)
	//{{AFX_MSG_MAP(CFaceNameDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CFaceNameDialog::OnInitDialog() 
{
	CFontDialog::OnInitDialog();
	
	CFontDialog::OnInitDialog();
	
	// hide the sizes static control
	ASSERT( GetDlgItem( stc3 ) );
	GetDlgItem( stc3 )->ShowWindow( SW_HIDE );

	// hide the sizes combo box
	ASSERT( GetDlgItem( cmb3 ) );
	GetDlgItem( cmb3 )->ShowWindow( SW_HIDE );

	// hide the strikeout check box
	ASSERT( GetDlgItem( chx1 ) );
	GetDlgItem( chx1 )->ShowWindow( SW_HIDE );

	// hide the script static control
	ASSERT( GetDlgItem( stc7 ) );
	GetDlgItem( stc7 )->ShowWindow( SW_HIDE );

	// hide the script combo box
	ASSERT( GetDlgItem( cmb5 ) );
	GetDlgItem( cmb5 )->ShowWindow( SW_HIDE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
