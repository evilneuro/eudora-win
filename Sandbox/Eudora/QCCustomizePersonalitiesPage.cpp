// QCCustomizePersonalitiesPage.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizePersonalitiesPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPersonalityCommand.h"
#include "QCPersonalityDirector.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern QCCommandStack			g_theCommandStack;
extern QCPersonalityDirector	g_thePersonalityDirector;

static UINT BASED_CODE s_PersonalityButtons[] =
{
	ID_CHANGE_PERSONA
};

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePersonalitiesPage property page

IMPLEMENT_DYNCREATE(QCCustomizePersonalitiesPage, QCToolBarCmdPage)

QCCustomizePersonalitiesPage::QCCustomizePersonalitiesPage() : QCToolBarCmdPage(QCCustomizePersonalitiesPage::IDD, IDR_PERSONALITIES )
{
	//{{AFX_DATA_INIT(QCCustomizePersonalitiesPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizePersonalitiesPage::~QCCustomizePersonalitiesPage()
{
}

void QCCustomizePersonalitiesPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizePersonalitiesPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT	QCCustomizePersonalitiesPage::TranslateID(
UINT	uID )
{
	HTREEITEM				hItem;
	QCPersonalityCommand*	pCommand;
	COMMAND_ACTION_TYPE		theAction;
	UINT					uNewID;
	CTreeCtrl*				pTree;

	switch( uID )
	{
		case ID_CHANGE_PERSONA:
			theAction = CA_CHANGE_PERSONA;
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

	pCommand = ( QCPersonalityCommand* ) ( pTree -> GetItemData( hItem ) );

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



BEGIN_MESSAGE_MAP(QCCustomizePersonalitiesPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizePersonalitiesPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePersonalitiesPage message handlers

BOOL QCCustomizePersonalitiesPage::OnInitDialog() 
{
	CStatic*				pLabel;
	CRString				szTitle( IDR_PERSONALITIES );
	CTreeCtrl*				pTree;
	POSITION				pos;
	QCPersonalityCommand*	pCommand;
	HTREEITEM				hItem;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_thePersonalityDirector.GetFirstPersonalityPosition();

	if( pos == NULL )
	{
		// there are no personalities
		return QCToolBarCmdPage::OnInitDialog();
	}
		
	DefineBtnGroup(_T( szTitle), NUMELEMENTS(s_PersonalityButtons), s_PersonalityButtons);

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

	pos = g_thePersonalityDirector.GetFirstPersonalityPosition();

	while( pos != NULL )
	{
		pCommand = g_thePersonalityDirector.GetNextPersonality( pos );
		hItem = pTree->InsertItem( pCommand->GetName() , QCMailboxImageList::IMAGE_PERSONALITY, QCMailboxImageList::IMAGE_PERSONALITY );
		pTree->SetItemData( hItem, ( DWORD ) pCommand );
	}
	
	pTree->SortChildren( NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
