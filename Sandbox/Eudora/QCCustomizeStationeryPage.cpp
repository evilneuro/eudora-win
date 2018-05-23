// QCCustomizeStationeryPage.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizeStationeryPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack		g_theCommandStack;
extern QCStationeryDirector	g_theStationeryDirector;

static UINT BASED_CODE theStationeryButtons[] =
{
	ID_MESSAGE_NEWMESSAGE_WITH,
	ID_MESSAGE_REPLY_WITH
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeStationeryPage property page

IMPLEMENT_DYNCREATE(QCCustomizeStationeryPage, QCToolBarCmdPage)

QCCustomizeStationeryPage::QCCustomizeStationeryPage() : QCToolBarCmdPage(QCCustomizeStationeryPage::IDD, IDR_STATIONERY )
{
	//{{AFX_DATA_INIT(QCCustomizeStationeryPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizeStationeryPage::~QCCustomizeStationeryPage()
{
}

void QCCustomizeStationeryPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizeStationeryPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT	QCCustomizeStationeryPage::TranslateID(
UINT	uID )
{
	HTREEITEM				hItem;
	QCStationeryCommand*	pCommand;
	COMMAND_ACTION_TYPE		theAction;
	UINT					uNewID;
	CTreeCtrl*				pTree;

	switch( uID )
	{
		case ID_MESSAGE_NEWMESSAGE_WITH:
			theAction = CA_NEW_MESSAGE_WITH;
		break;

		case ID_MESSAGE_REPLY_WITH:
			theAction = CA_REPLY_WITH;
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

	pCommand = ( QCStationeryCommand* ) ( pTree -> GetItemData( hItem ) );

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


BEGIN_MESSAGE_MAP(QCCustomizeStationeryPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizeStationeryPage)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(wmSECNotifyMsg, OnBarNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizeStationeryPage message handlers

BOOL QCCustomizeStationeryPage::OnInitDialog() 
{
	CStatic*			pLabel;
	CRString			szTitle( IDR_STATIONERY );
	CTreeCtrl*			pTree;
	POSITION			pos;
	QCStationeryCommand*	pCommand;
	HTREEITEM			hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_theStationeryDirector.GetFirstStationeryPosition();

	if( pos == NULL )
	{
		// there is no stationery
		return QCToolBarCmdPage::OnInitDialog();
	}	

	DefineBtnGroup( _T( szTitle), NUMELEMENTS(theStationeryButtons), theStationeryButtons );

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
		pCommand = g_theStationeryDirector.GetNext( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_STATIONERY, QCMailboxImageList::IMAGE_STATIONERY );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	
	pTree->SortChildren( NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
	
}


LONG QCCustomizeStationeryPage::OnBarNotify(UINT nID, LONG lParam)
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
			if( pBar->GetCurBtn() == 2 )
			{
				CRString szDesc( IDS_EDIT_STATIONERY_DESCRIPTION );
				GetDlgItem(IDC_TOOLBAR_DESCRIPTION)->SetWindowText( szDesc );
				return 0;
			}
		}
		break;

		default:
		break;
	}

	return SECToolBarCmdPage::OnBarNotify( nID, lParam );
}

