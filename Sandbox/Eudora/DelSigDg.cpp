// DelSigDg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DelSigDg.h"
#include "QCCommandActions.h"
#include "QCCommandObject.h"
#include "QCSignatureDirector.h"
#include "QCStationeryDirector.h"
#include "rs.h"

extern QCStationeryDirector g_theStationeryDirector;
extern QCSignatureDirector g_theSignatureDirector;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CDeleteDialog dialog


CDeleteDialog::CDeleteDialog( 
BOOL	bDeleteStationery,
CWnd*	pParent /*=NULL*/) : CDialog(CDeleteDialog::IDD, pParent)
{
	m_bDeleteStationery = bDeleteStationery;
	//{{AFX_DATA_INIT(CDeleteDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDeleteDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteDialog, CDialog)
	//{{AFX_MSG_MAP(CDeleteDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteDialog message handlers

BOOL CDeleteDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if( m_bDeleteStationery )
	{
		SetWindowText( CRString( IDS_DELETE_STATIONERY ) );
		g_theStationeryDirector.FillListBox( &SignatureLB() );
	}
	else
	{
		g_theSignatureDirector.FillListBox( &SignatureLB() );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDeleteDialog::OnOK()
{
	int					iCount;
	UINT				nDeleted;
	int					iIndex;
	CString				szName;
	QCCommandObject*	pCommand;

	for(	iIndex = SignatureLB().GetCount() - 1, 
			iCount = SignatureLB().GetSelCount(),
			nDeleted = 0; 
			( iCount > 0 ) && ( iIndex >= 0 );
			iIndex -- )
	{
		// see if the item is selected
		if ( SignatureLB().GetSel( iIndex ) )
		{
			// get the name
			SignatureLB().GetText( iIndex, szName );

			if( m_bDeleteStationery )
			{
				pCommand = ( QCCommandObject* ) g_theStationeryDirector.Find( szName );
			}
			else
			{
				pCommand = ( QCCommandObject* ) g_theSignatureDirector.Find( szName );
			}

			if( pCommand )
			{
				pCommand->Execute( CA_DELETE );
			}

			// increment the delete count
			++nDeleted;
			
			// decrement the selected count
			-- iCount;
		}
	}

	if ( !nDeleted )
	{
		// we haven't done anything -- same thing as cancel
		CDialog::OnCancel();
	}
	else
	{
		// normal ok processing
		CDialog::OnOK();
	}
}
