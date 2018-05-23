// QCCustomizeSignaturesPage.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizeSignaturesPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack		g_theCommandStack;
extern QCSignatureDirector	g_theSignatureDirector;

static UINT BASED_CODE theSignatureButtons[] =
{
	ID_WILDCARD
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeSignaturesPage property page

IMPLEMENT_DYNCREATE(QCCustomizeSignaturesPage, QCToolBarCmdPage)

QCCustomizeSignaturesPage::QCCustomizeSignaturesPage() : QCToolBarCmdPage(QCCustomizeSignaturesPage::IDD, IDS_SIGNATURES )
{
	//{{AFX_DATA_INIT(QCCustomizeSignaturesPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizeSignaturesPage::~QCCustomizeSignaturesPage()
{
}

void QCCustomizeSignaturesPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizeSignaturesPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT	QCCustomizeSignaturesPage::TranslateID(
UINT	uID )
{
	HTREEITEM				hItem;
	QCSignatureCommand*		pCommand;
	COMMAND_ACTION_TYPE		theAction;
	UINT					uNewID;
	CTreeCtrl*				pTree;

	switch( uID )
	{
		case ID_WILDCARD:
			theAction = CA_OPEN;
		break;

		default:
			return uID;
	}

	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return 0;
	}

	hItem = pTree->GetSelectedItem();
	
	if( hItem == NULL )
	{
		ASSERT( 0 );
		return 0;
	}

	pCommand = ( QCSignatureCommand* ) ( pTree -> GetItemData( hItem ) );

	if( pCommand == NULL )
	{
		ASSERT( 0 );
		return 0;
	}

	uNewID = g_theCommandStack.FindCommandID( pCommand, theAction );
	
	if( uNewID == 0 )
	{
		ASSERT( 0 );
	}

	return uNewID;
}


BEGIN_MESSAGE_MAP(QCCustomizeSignaturesPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizeSignaturesPage)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(wmSECNotifyMsg, OnBarNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeSignaturesPage message handlers

BOOL QCCustomizeSignaturesPage::OnInitDialog() 
{	
	CStatic*			pLabel;
	CRString			szTitle( IDS_SIGNATURES );
	CTreeCtrl*			pTree;
	POSITION			pos;
	QCSignatureCommand*	pCommand;
	HTREEITEM			hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_theSignatureDirector.GetFirstSignaturePosition();

	if( pos == NULL )
	{
		// there are no signatures
		return QCToolBarCmdPage::OnInitDialog();
	}
		
	DefineBtnGroup( _T( szTitle), NUMELEMENTS(theSignatureButtons), theSignatureButtons );

	QCToolBarCmdPage::OnInitDialog();
	
	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return TRUE;
	}

	//
	// Add the bitmap images for tree control.
	//
	
	if (! m_ImageList.Load() )
	{
		return TRUE;		// mangled resources?
	}

	pTree->SetImageList( &m_ImageList, TVSIL_NORMAL );
	
	//
	// Set "thin" font for old Win 3.x GUI.
	//
	if (! IsVersion4() )
	{
		pTree->SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));
	}

	while( pos != NULL )
	{
		pCommand = g_theSignatureDirector.GetNext( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_SIGNATURE, QCMailboxImageList::IMAGE_SIGNATURE );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	
	pTree->SortChildren( NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LONG QCCustomizeSignaturesPage::OnBarNotify(UINT nID, LONG lParam)
{
	
	// A toolbar is notifying us of an event.
	if(m_pManager == NULL)
		return 0L;

	SECCustomToolBar* pBar = GetControlBar(nID);
	if(pBar == NULL)
		return 0L;

	switch(lParam)
	{
		case SECCustomToolBar::BarDesChange:
		{
			// The selected button has changed - so need to changed the
			// description text.
			if( pBar->GetCurBtn() != -1 )
			{
				CRString szDesc( IDS_EDIT_SIGNATURE_DESCRIPTION );
				GetDlgItem(IDC_TOOLBAR_DESCRIPTION)->SetWindowText( szDesc );
			}
			else
			{
				GetDlgItem(IDC_TOOLBAR_DESCRIPTION)->SetWindowText( "" );
			}
			
			return 0;
		}

		default:
			break;
	}

	return SECToolBarCmdPage::OnBarNotify( nID, lParam );
}

