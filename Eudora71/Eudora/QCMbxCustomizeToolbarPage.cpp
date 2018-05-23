// QCMbxCustomizeToolbarPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCMbxCustomizeToolbarPage.h"
#include "rs.h"


#include "DebugNewHelpers.h"

extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;

static UINT BASED_CODE theMailboxButtons[] =
{
	ID_MAILBOX_USER,
	ID_TRANSFER_USER
};

		
static UINT BASED_CODE theInButtons[] =
{
	ID_MAILBOX_IN,
	ID_TRANSFER_USER
};

		
static UINT BASED_CODE theOutButtons[] =
{
	ID_MAILBOX_OUT,
	ID_TRANSFER_USER
};

		
static UINT BASED_CODE theJunkButtons[] =
{
	ID_MAILBOX_JUNK,
	ID_TRANSFER_USER
};

		
static UINT BASED_CODE theTrashButtons[] =
{
	ID_MAILBOX_TRASH,
	ID_TRANSFER_USER
};

		
/////////////////////////////////////////////////////////////////////////////
// QCMailboxCustomizeToolbarPage property page

IMPLEMENT_DYNCREATE(QCMailboxCustomizeToolbarPage, QCToolBarCmdPage)

QCMailboxCustomizeToolbarPage::QCMailboxCustomizeToolbarPage() : QCToolBarCmdPage(QCMailboxCustomizeToolbarPage::IDD, IDR_MAILBOXES)
{
	//{{AFX_DATA_INIT(QCMailboxCustomizeToolbarPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCMailboxCustomizeToolbarPage::~QCMailboxCustomizeToolbarPage()
{
}

void QCMailboxCustomizeToolbarPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCMailboxCustomizeToolbarPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


UINT QCMailboxCustomizeToolbarPage::TranslateID(UINT uID)
{
	COMMAND_ACTION_TYPE theAction;

	switch (uID)
	{
	case ID_MAILBOX_USER:	theAction = CA_OPEN;		break;
	case ID_TRANSFER_USER:	theAction = CA_TRANSFER_TO;	break;
	default:
		return uID;
	}

	HTREEITEM hItem = m_theTreeCtrl.GetSelectedItem();
	if (!hItem)
	{
		ASSERT(0);
		return 0;
	}

	QCMailboxCommand* pCommand =
		((QCMailboxTreeCtrl::CMboxTreeItemData*)m_theTreeCtrl.GetItemData(hItem))->m_pCommand;
	if (!pCommand)
	{
		ASSERT(0);
		return 0;
	}

	UINT uNewID = g_theCommandStack.FindOrAddCommandID(pCommand, theAction);

	return uNewID;
}


BEGIN_MESSAGE_MAP(QCMailboxCustomizeToolbarPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCMailboxCustomizeToolbarPage)
	//}}AFX_MSG_MAP
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREE_CTRL, OnSelChanged )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCMailboxCustomizeToolbarPage message handlers

BOOL QCMailboxCustomizeToolbarPage::OnInitDialog() 
{
	CStatic*	pLabel;
	CRString	szTitle( IDR_MAILBOXES );

	m_nCurSel = 0;

	DefineBtnGroup(_T("In"), NUMELEMENTS(theInButtons), theInButtons);
	DefineBtnGroup(_T("Out"), NUMELEMENTS(theOutButtons), theOutButtons);
	DefineBtnGroup(_T("Junk"), NUMELEMENTS(theJunkButtons), theJunkButtons);
	DefineBtnGroup(_T("Trash"), NUMELEMENTS(theTrashButtons), theTrashButtons);
	DefineBtnGroup(_T("Mailboxes"), NUMELEMENTS(theMailboxButtons), theMailboxButtons);

	QCToolBarCmdPage::OnInitDialog();
	
	pLabel = ( CStatic* ) GetDlgItem( IDC_TREE_ITEMS_STATIC );
	
	if( pLabel != NULL )
	{
		pLabel->SetWindowText( 	szTitle );	
	}
	
	m_theTreeCtrl.SubclassDlgItem( IDC_TREE_CTRL, this );
	m_theTreeCtrl.Init();
	g_theMailboxDirector.InitializeMailboxTreeControl( &m_theTreeCtrl, 0, NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void QCMailboxCustomizeToolbarPage::OnSelChanged(
NMHDR*		pNMHDR, 
LRESULT*	pResult) 
{
	QCMailboxTreeCtrl::ItemType		theItemType;
	NM_TREEVIEW*					pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	INT								iIndex;
	HTREEITEM						hItem;
	CTreeCtrl*						pTree;
	CString							szName;
	SECCustomToolBar*				pToolBar;
	UINT							nStyle;

	*pResult = 0;

	hItem = pNMTreeView->itemNew.hItem;

	pTree = ( CTreeCtrl* ) GetDlgItem( IDC_TREE_CTRL );
	
	if( pTree == NULL )
	{
		ASSERT( 0 );
		return;
	}

	// Is this really a sel change?
	if (pNMTreeView->action == TVC_UNKNOWN)
		return;

	theItemType = ( ( QCMailboxTreeCtrl::CMboxTreeItemData* ) m_theTreeCtrl.GetItemData( hItem ) )->m_itemType;

	if(m_nCurSel != LB_ERR)
	{
		pToolBar = ((SECBtnGroup*) m_btnGroups[m_nCurSel])->pToolBar;
		
		pToolBar->EnableConfigMode(FALSE);

		for( iIndex = 0; iIndex < pToolBar->GetBtnCount(); iIndex ++ )
		{
			nStyle = pToolBar->GetButtonStyle(iIndex) | TBBS_DISABLED;
			pToolBar->SetButtonStyle( iIndex, nStyle);
		}
		
		m_nCurSel = LB_ERR;
	}

	switch( theItemType )
	{
		case QCMailboxTreeCtrl::ITEM_IN_MBOX:
			szName = "In";	
		break;
		case QCMailboxTreeCtrl::ITEM_OUT_MBOX:
			szName = "Out";	
		break;
		case QCMailboxTreeCtrl::ITEM_JUNK_MBOX:
			szName = "Junk";	
		break;
		case QCMailboxTreeCtrl::ITEM_TRASH_MBOX:
			szName = "Trash";	
		break;
		case QCMailboxTreeCtrl::ITEM_USER_MBOX:
			szName = "Mailboxes";	
		break;
		case QCMailboxTreeCtrl::ITEM_IMAP_MAILBOX:
			szName = "Mailboxes";
		break;

		default:
			return;
	}
	
	// The current selection has changed. I need to show the newly selected
	// toolbar
	for( iIndex = 0; iIndex < m_btnGroups.GetSize(); iIndex ++)
	{
		if( szName == ((SECBtnGroup*) m_btnGroups[iIndex])->lpszTitle )
		{
			break;
		}
	}

	if( iIndex == m_btnGroups.GetSize() )
	{
		ASSERT( 0 );
		return;
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



