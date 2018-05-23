// QCCustomizePluginsPage.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizePluginsPage.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack	g_theCommandStack;
extern QCPluginDirector	g_thePluginDirector;


/////////////////////////////////////////////////////////////////////////////
// QCCustomizePluginsPage property page

IMPLEMENT_DYNCREATE(QCCustomizePluginsPage, QCToolBarCmdPage)

QCCustomizePluginsPage::QCCustomizePluginsPage() : QCToolBarCmdPage(QCCustomizePluginsPage::IDD, IDS_PLUGINS_TITLE )
{
	//{{AFX_DATA_INIT(QCCustomizePluginsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_IDs = NULL;
}

QCCustomizePluginsPage::~QCCustomizePluginsPage()
{
	delete [] m_IDs;
}


void QCCustomizePluginsPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizePluginsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(QCCustomizePluginsPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizePluginsPage)
	//}}AFX_MSG_MAP
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREE_CTRL, OnSelChanged )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCCustomizePluginsPage message handlers

BOOL QCCustomizePluginsPage::OnInitDialog() 
{
	CStatic*			pLabel;
	CRString			szTitle( IDS_PLUGINS_TITLE );
	CTreeCtrl*			pTree;
	POSITION			pos;
	QCPluginCommand*	pCommand;
	HTREEITEM			hItem;
	INT					i;

	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	pos = g_thePluginDirector.GetFirstPluginPosition();

	m_nCurSel = LB_ERR;

	if( pos == NULL ) 
	{
		// there are no plugins
		return QCToolBarCmdPage::OnInitDialog();
	}

	VERIFY( m_IDs = new UINT [ g_thePluginDirector.GetCount() ] );
	
	if( !m_IDs )
	{
		return FALSE;
	}
	
	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return TRUE;
	}


	pTree->SetImageList( &g_thePluginDirector.m_ImageList, TVSIL_NORMAL );
	
	//
	// Set "thin" font for old Win 3.x GUI.
	//
	if (! IsVersion4() )
	{
		pTree->SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));
	}


	i = 0;

	while( pos != NULL )
	{
		pCommand = g_thePluginDirector.GetNextPlugin( pos );

		if( pCommand->GetType() != CA_TRANSMISSION_PLUGIN )
		{
			m_IDs[ i ] = g_theCommandStack.FindCommandID( pCommand, pCommand->GetType() );
			if( m_IDs[ i ] )
			{
				DefineBtnGroup(_T( pCommand->GetName() ), 1, &( m_IDs[ i ] ) );
				i++;
				hItem = pTree->InsertItem( pCommand->GetName() , pCommand->m_ImageOffset, pCommand->m_ImageOffset );
				pTree->SetItemData( hItem, ( DWORD ) pCommand );
			}
		}
	}
	
	QCToolBarCmdPage::OnInitDialog();	
		
	//pTree->SortChildren( NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void QCCustomizePluginsPage::SetDescription(int nID)
{
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction;
				
	if( g_theCommandStack.Lookup( (unsigned short)nID, &pObject, &theAction ) == TRUE )
	{
		switch (theAction)
		{
			case CA_ATTACH_PLUGIN:
				nID = ID_MESSAGE_ATTACH_PLUGIN;
				break;
			case CA_SPECIAL_PLUGIN:
				nID = ID_SPECIAL_PLUGIN;
				break;
			case CA_TRANSLATE_PLUGIN:
				nID = ID_TRANSLATOR;
				break;
			default:
				break;
		}
	}
	
	SECToolBarCmdPage::SetDescription(nID);
}

void QCCustomizePluginsPage::OnSelChanged(
NMHDR*		pNMHDR, 
LRESULT*	pResult) 
{
	QCPluginCommand*	pCommand;
	NM_TREEVIEW*		pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	INT					iIndex;
	HTREEITEM			hItem;
	CTreeCtrl*			pTree;
	CString				szName;

	*pResult = 0;

	hItem = pNMTreeView->itemNew.hItem;

	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return;
	}

	pCommand = ( QCPluginCommand* ) ( pTree -> GetItemData( hItem ) );

	if( pCommand == NULL )
	{
		ASSERT( 0 );
		return;
	}
		
	
	// The current selection has changed. I need to show the newly selected
	// toolbar
	for( iIndex = 0; iIndex < m_btnGroups.GetSize(); iIndex ++)
	{
		if( pCommand->GetName() == ((SECBtnGroup*) m_btnGroups[iIndex])->lpszTitle )
		{
			break;
		}
	}

	if( iIndex == m_btnGroups.GetSize() )
	{
		ASSERT( 0 );
		return;
	}


	if(m_nCurSel != LB_ERR)
	{
		((SECBtnGroup*) 
			m_btnGroups[m_nCurSel])->pToolBar->EnableConfigMode(FALSE);
		m_nCurSel = LB_ERR;
	}

	for(int i=0; i<m_btnGroups.GetSize(); i++)
	{
		SECCustomizeToolBar* pBar = ((SECBtnGroup*) m_btnGroups[i])->pToolBar;
		if(i == iIndex)
		{
			pBar->EnableConfigMode(TRUE);
			pBar->ShowWindow(SW_SHOW);
			m_nCurSel = i;
		}
		else
			pBar->ShowWindow(SW_HIDE);
	}
}

