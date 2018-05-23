// SelectMBox.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "eudora.h"
#include "rs.h"
#include "newmbox.h"
#include "QCMailboxTreeCtrl.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "SelectMBox.h"

extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectMBox dialog


CSelectMBox::CSelectMBox(CWnd* pParent /*=NULL*/, BOOL allowNew /*= TRUE*/)
	: CDialog(CSelectMBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMBox)
	m_PromptText = _T("");
	//}}AFX_DATA_INIT
	m_bAllowNew = allowNew;
}


void CSelectMBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMBox)
	DDX_Control(pDX, IDOK, m_OKButton);
	DDX_Text(pDX, IDC_PROMPT_TEXT, m_PromptText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectMBox, CDialog)
	//{{AFX_MSG_MAP(CSelectMBox)
	ON_NOTIFY(TVN_SELCHANGED, IDC_MBOX_TREE, OnSelChanged)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL CSelectMBox::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_MboxTree.SubclassDlgItem( IDC_MBOX_TREE, this );
	m_MboxTree.Init();
	g_theMailboxDirector.InitializeMailboxTreeControl( &m_MboxTree, 0, NULL );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CSelectMBox message handlers
void CSelectMBox::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	QCMailboxTreeCtrl::ItemType		theItemType;
	NM_TREEVIEW*					pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM						hItem;
	CTreeCtrl*						pTree;

	*pResult = 0;

	if (pNMTreeView->action)
	{

		hItem = pNMTreeView->itemNew.hItem;

		pTree = ( CTreeCtrl* ) GetDlgItem( IDC_MBOX_TREE );
		if( pTree == NULL )
		{
			ASSERT( 0 );
			return;
		}

		if ( ( QCMailboxTreeCtrl::CMboxTreeItemData* ) m_MboxTree.GetItemData( hItem ) )
		{
			theItemType = ( ( QCMailboxTreeCtrl::CMboxTreeItemData* ) m_MboxTree.GetItemData( hItem ) )->m_itemType;
			if (theItemType ==QCMailboxTreeCtrl::ITEM_FOLDER)
				m_OKButton.EnableWindow(FALSE);
			else
				m_OKButton.EnableWindow(TRUE);
		}
	}

}

void CSelectMBox::OnOK() 
{
	// Get the selection, find the path
	HTREEITEM hItem = m_MboxTree.GetSelectedItem( );
	if (hItem)
	{
		QCMailboxCommand* pCommand = ( ( QCMailboxTreeCtrl::CMboxTreeItemData* ) m_MboxTree.GetItemData( hItem ) )->m_pCommand;
		if (pCommand)
		{
			m_MailboxPath = pCommand->GetPathname();
		}
	}
	CDialog::OnOK();
}

void CSelectMBox::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (m_bAllowNew)
	{
		HTREEITEM hItem = m_MboxTree.GetSelectedItem();
		if (hItem)
		{
			QCMailboxTreeCtrl::ItemType		theItemType;
			QCMailboxCommand* pCommand = ( ( QCMailboxTreeCtrl::CMboxTreeItemData* ) m_MboxTree.GetItemData( hItem ) )->m_pCommand;
			
			theItemType = ( ( QCMailboxTreeCtrl::CMboxTreeItemData* ) m_MboxTree.GetItemData( hItem ) )->m_itemType;
			if (theItemType == QCMailboxTreeCtrl::ITEM_FOLDER || theItemType == QCMailboxTreeCtrl::ITEM_ROOT)
			{
				UINT	cmdID = 0;
				CMenu		theMenu;
				CRString	menuStr(IDS_MAILBOX_NEW);
		
				theMenu.CreatePopupMenu();
				theMenu.AppendMenu(MF_STRING, 1, menuStr );

				if (cmdID = theMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, pWnd ))
				{
					// Did the user click a new mailbox?
					if (cmdID == 1)
					{
						pCommand = g_theMailboxDirector.CreateTargetMailbox( (QCMailboxCommand *) pCommand, FALSE );
						if (pCommand)
							pCommand->Execute( CA_NEW_MAILBOX, NULL );
						
						m_MboxTree.Reset();
						g_theMailboxDirector.InitializeMailboxTreeControl( &m_MboxTree, 0, NULL );

					}
				}
			}
		}
	}
}
