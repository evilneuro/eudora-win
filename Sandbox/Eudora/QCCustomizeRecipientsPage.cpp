// QCCustomizeRecipientsPage.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizeRecipientsPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern QCCommandStack		g_theCommandStack;
extern QCRecipientDirector	g_theRecipientDirector;

static UINT BASED_CODE theRecipientButtons[] =
{
	ID_RECIPIENT_NEW_MESSAGE,
	ID_RECIPIENT_FORWARD,
	ID_RECIPIENT_REDIRECT,
	ID_RECIPIENT_INSERT
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeRecipientsPage property page

IMPLEMENT_DYNCREATE(QCCustomizeRecipientsPage, QCToolBarCmdPage)

QCCustomizeRecipientsPage::QCCustomizeRecipientsPage() : QCToolBarCmdPage(QCCustomizeRecipientsPage::IDD, IDS_RECIPIENTS_TITLE )
{
	//{{AFX_DATA_INIT(QCCustomizeRecipientsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizeRecipientsPage::~QCCustomizeRecipientsPage()
{
}

void QCCustomizeRecipientsPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizeRecipientsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT	QCCustomizeRecipientsPage::TranslateID(
UINT	uID )
{
	HTREEITEM				hItem;
	QCRecipientCommand*		pCommand;
	COMMAND_ACTION_TYPE		theAction;
	UINT					uNewID;
	CTreeCtrl*				pTree;

	switch( uID )
	{
		case ID_RECIPIENT_NEW_MESSAGE:
			theAction = CA_NEW_MESSAGE_TO;
		break;

		case ID_RECIPIENT_FORWARD:
			theAction = CA_FORWARD_TO;
		break;

		case ID_RECIPIENT_REDIRECT:
			theAction = CA_REDIRECT_TO;
		break;

		case ID_RECIPIENT_INSERT:
			theAction = CA_INSERT_RECIPIENT;
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

	pCommand = ( QCRecipientCommand* ) ( pTree -> GetItemData( hItem ) );

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



BEGIN_MESSAGE_MAP(QCCustomizeRecipientsPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizeRecipientsPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeRecipientsPage message handlers

BOOL QCCustomizeRecipientsPage::OnInitDialog() 
{
	CStatic*			pLabel;
	CRString			szTitle( IDS_RECIPIENTS_TITLE );
	CTreeCtrl*			pTree;
	POSITION			pos;
	QCRecipientCommand*	pCommand;
	HTREEITEM			hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_theRecipientDirector.GetFirstRecipientPosition();

	if( pos == NULL )
	{
		// there are no recipients
		return QCToolBarCmdPage::OnInitDialog();
	}
		
	DefineBtnGroup(_T( szTitle), NUMELEMENTS(theRecipientButtons), theRecipientButtons);

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

	pos = g_theRecipientDirector.GetFirstRecipientPosition();

	while( pos != NULL )
	{
		pCommand = g_theRecipientDirector.GetNextRecipient( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_RECIPIENT, QCMailboxImageList::IMAGE_RECIPIENT );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	
	pTree->SortChildren( NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
