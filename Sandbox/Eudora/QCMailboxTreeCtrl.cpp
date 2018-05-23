// mboxtree.cpp: implementation
//
// QCMailboxTreeCtrl
// Smart mailbox-specific tree control that is meant to be embedded
// in a parent "dialog" class.

#include "stdafx.h"
#include "resource.h"


#include "QCMailboxTreeCtrl.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "mboxtree.h"

#ifdef IMAP4
#include "QCImapMailboxCommand.h"
#endif  // IMAP4

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern CString EudoraDir;		// avoids inclusion of fileutil.h

IMPLEMENT_DYNAMIC( QCMailboxTreeCtrl, QCTreeCtrl )

BEGIN_MESSAGE_MAP(QCMailboxTreeCtrl, QCTreeCtrl)
	//{{AFX_MSG_MAP(QCMailboxTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// QCMailboxTreeCtrl

QCMailboxTreeCtrl::QCMailboxTreeCtrl()
{
}

QCMailboxTreeCtrl::~QCMailboxTreeCtrl()
{
}


////////////////////////////////////////////////////////////////////////
// Init [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL QCMailboxTreeCtrl::Init()
{
	if (! QCTreeCtrl::Init())
		return FALSE;

	//
	// Add the bitmap images for tree control.
	//
	if (! m_ImageList.Load())
	{
		return FALSE;		// mangled resources?
	}

	SetImageList( &m_ImageList, TVSIL_NORMAL);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// Reset [public]
//
// Delete all tree items and their lParam-based storage from this
// tree control.  Clients should not call the DeleteItem() or 
// DeleteAllItems() methods directly.
//
////////////////////////////////////////////////////////////////////////
BOOL QCMailboxTreeCtrl::Reset(void)
{
	//
	// First, loop through all items and delete the lParam-based storage.
	//
	HTREEITEM h_item = GetRootItem();
	if (h_item != NULL)
	{
		//
		// Recursively process the entire tree.
		//
		DeleteItemData(h_item);

#ifdef IMAP4
		// Prevent undesirable "OnEndLabelEdit()"
		//
		SendMessage(TVM_ENDEDITLABELNOW, TRUE, 0);

		// May have siblings at level 0 - delete them too.
		HTREEITEM h_sibling = GetNextSiblingItem(h_item);
		while (h_sibling)
		{
			DeleteItemData(h_sibling);
			h_sibling = GetNextSiblingItem(h_sibling);
		}
#endif // IMAP4

	}

	DeleteAllItems();
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// DeleteItemData [private]
//
// Recursively delete all lParam-based storage from this tree item
// and all of its children.  Does not delete the items themselves.
//
////////////////////////////////////////////////////////////////////////
void QCMailboxTreeCtrl::DeleteItemData(HTREEITEM hItem)
{
	//
	// First, process this item (the "root").
	//
	ASSERT(hItem != NULL);
	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
	ASSERT(p_itemdata != NULL);
	delete p_itemdata;
	p_itemdata = NULL;		// good hygiene

	HTREEITEM h_child = GetChildItem(hItem);
	while (h_child != NULL)
	{
		// cleanup subtrees recursively!
		DeleteItemData(h_child);
		h_child = GetNextSiblingItem(h_child);
	}
}

// virtual
BOOL QCMailboxTreeCtrl::GetItemStruct(TV_INSERTSTRUCT &tvstruct,
							ItemType itemType,
							const CString& itemName, 
							QCMailboxCommand* pCommand,
							BOOL isChecked)
{
	HTREEITEM		hItem;
	CString			szSibling;

	//
	// Initialize record.
	//
	tvstruct.hParent = NULL;
				// normally overwritten later
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
	tvstruct.item.hItem = 0;
	tvstruct.item.state = 0;			// may be overwritten later
	tvstruct.item.stateMask = TVIS_BOLD;	
	tvstruct.item.pszText = ( char* ) ( const char* ) itemName;
	tvstruct.item.cchTextMax = -1;		// unused
	tvstruct.item.iImage = -1;			// normally overwritten later
	tvstruct.item.iSelectedImage = -1;	// normally overwritten later
	tvstruct.item.lParam = NULL;		// normally overwritten later

	//
	// Create an item data object and attach it to the item.
	//
	CMboxTreeItemData* p_itemdata = new CMboxTreeItemData(itemType, pCommand);
	if (p_itemdata != NULL)
		tvstruct.item.lParam = long(p_itemdata);	// cast to long
	else
		return FALSE;		// out of memory

	//
	// Then, handle item-specific stuff.
	//
	if (isChecked)
		tvstruct.item.state = TVIS_BOLD;

	switch (itemType)
	{
	case ITEM_ROOT:
		tvstruct.hParent = TVI_ROOT;
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_EUDORA;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_EUDORA;
		break;
	case ITEM_IN_MBOX:
		tvstruct.hParent = GetRootItem();
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_IN_MBOX;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IN_MBOX;
		break;
	case ITEM_OUT_MBOX:
		tvstruct.hParent = GetRootItem();
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_OUT_MBOX;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_OUT_MBOX;
		break;
	case ITEM_TRASH_MBOX:
		tvstruct.hParent = GetRootItem();
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_TRASH_MBOX;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_TRASH_MBOX;
		break;

	case ITEM_FOLDER:	
		//
		// We have to figure out where to place this in the hierarchy.
		//
		tvstruct.hParent = GetParentFromPathname( pCommand->GetPathname() );
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_CLOSED_FOLDER;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_CLOSED_FOLDER;
		break;
	case ITEM_USER_MBOX:
		//
		// We have to figure out where to place this in the hierarchy.
		//
		tvstruct.hParent = GetParentFromPathname( pCommand->GetPathname() );
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_NORMAL_MBOX;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_NORMAL_MBOX;
		break;
	default:
		ASSERT(0);
		break;
	}

	ASSERT(tvstruct.hParent != NULL);

	// 
	// find the right place to insert
	//
	
	if( ( itemType == ITEM_FOLDER ) || ( itemType == ITEM_USER_MBOX ) )
	{
		if( ItemHasChildren( tvstruct.hParent ) ) 
		{
			hItem = GetChildItem( tvstruct.hParent );
			
			while( hItem )
			{
				p_itemdata = ( CMboxTreeItemData* ) GetItemData( hItem );
				
				if( itemType == ITEM_FOLDER ) 
				{
					if( p_itemdata->m_itemType != ITEM_FOLDER )
					{
						hItem = GetNextSiblingItem( hItem );
						continue;
					}
				}
				else
				{
					if( p_itemdata->m_itemType == ITEM_FOLDER )
					{
						hItem = GetPrevSiblingItem( hItem );
						
						if( hItem == NULL )
						{
							hItem = TVI_FIRST;
						}
						
						break;
					}
					else if( p_itemdata->m_itemType != ITEM_USER_MBOX )
					{
						hItem = GetNextSiblingItem( hItem );
						continue;
					}
				}
				
				szSibling = GetItemText( hItem );
				
				if(  szSibling.CompareNoCase( itemName ) < 0 )
				{
					hItem = GetNextSiblingItem( hItem );
				}
				else
				{
					hItem = GetPrevSiblingItem( hItem );
					
					if( hItem == NULL )
					{
						hItem = TVI_FIRST;
					}
					
					break;
				}
			}

			if( hItem )
			{
				tvstruct.hInsertAfter = hItem;
			}
		}
	}

	return (TRUE);
}

////////////////////////////////////////////////////////////////////////
// AddItem [public]
//
// The 'itemType' and 'itemDepth' parameters determines the level of
// the tree where the new item gets added.  The root item is defined
// to be at level 0, the top-level mailboxes/folders at are level 1,
// and so on.
////////////////////////////////////////////////////////////////////////
QCMailboxTreeCtrl::AddItem(
	ItemType itemType,
	const CString& itemName, 
	QCMailboxCommand* pCommand,
	BOOL isChecked)
{
	TV_INSERTSTRUCT tvstruct;

	if (GetItemStruct(tvstruct, itemType, itemName, pCommand, isChecked))
	{
		// Do the insert...
		HTREEITEM hItem = InsertItem(&tvstruct);

		if (hItem != NULL)
		{
			if( itemType == ITEM_ROOT )
			{
				PostMessage( TVM_EXPAND, WPARAM( TVE_EXPAND ), LPARAM( hItem ) ); 			
			}
			
			return TRUE;
		}
	}

	ASSERT(0);
	return FALSE;
}



/////////////////////////////////////////////////////////////////////////////
// QCMailboxTreeCtrl message handlers


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
// Handles QCTreeCtrl HWND destruction by calling the Reset() method
// to cleanup all the heap-allocated memory.
//
////////////////////////////////////////////////////////////////////////
void QCMailboxTreeCtrl::OnDestroy()
{
	Reset();		// make sure heap allocations are cleared up
	QCTreeCtrl::OnDestroy();
}


////////////////////////////////////////////////////////////////////////
// AutoCloseOpenedFolders [public]
//
// Process the list of auto-opened folders by closing them in reverse
// order.
//
////////////////////////////////////////////////////////////////////////
BOOL QCMailboxTreeCtrl::AutoCloseOpenedFolders()
{
	TRACE0("QCMailboxTreeCtrl::AutoCloseOpenedFolders()\n");
	while (! m_autoOpenFolderList.IsEmpty())
	{
		HTREEITEM h_item = HTREEITEM(m_autoOpenFolderList.RemoveTail());
		Expand(h_item, TVE_COLLAPSE);

		//
		// Okay, the TVM_EXPAND message does not send the corresponding
		// TVN_ITEMEXPANDED notification message like you might
		// expect, so we have to do it ourselves.  Whatta pain.
		//
		NM_TREEVIEW nmtv;
		nmtv.action = TVE_COLLAPSE;
		nmtv.itemNew.mask = TVIF_HANDLE | TVIF_PARAM;
		nmtv.itemNew.hItem = h_item;
		nmtv.itemNew.lParam = GetItemData(h_item);
		LRESULT unused;
		OnItemExpanded((NMHDR* ) &nmtv, &unused);
	}

	return TRUE;
}



////////////////////////////////////////////////////////////////////////
// OnItemExpanded [protected]
//
// Handles QCTreeCtrl expanded/collapsed notification message by updating
// the graphic associated with the changed tree item.
//
////////////////////////////////////////////////////////////////////////
void QCMailboxTreeCtrl::OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* p_treeview = (NM_TREEVIEW * )pNMHDR;	// type cast
	ASSERT(p_treeview);

	CMboxTreeItemData* p_dataitem = (CMboxTreeItemData *) (p_treeview->itemNew.lParam);
	ASSERT(p_dataitem != NULL);
	switch (p_treeview->action)
	{
	case TVE_COLLAPSE:
		if (ITEM_ROOT == p_dataitem->m_itemType)
			SetItemImage(p_treeview->itemNew.hItem, QCMailboxImageList::IMAGE_EUDORA, QCMailboxImageList::IMAGE_EUDORA);
		else if (ITEM_FOLDER == p_dataitem->m_itemType)
			SetItemImage(p_treeview->itemNew.hItem, QCMailboxImageList::IMAGE_CLOSED_FOLDER, QCMailboxImageList::IMAGE_CLOSED_FOLDER);
#ifdef IMAP4
		else if ( (ITEM_IMAP_ACCOUNT == p_dataitem->m_itemType)		||
				  (ITEM_IMAP_NAMESPACE == p_dataitem->m_itemType)	||
				  (ITEM_IMAP_MAILBOX == p_dataitem->m_itemType) )
		{
			UpdateImapItemImage (p_treeview->itemNew.hItem, p_treeview->action);

		}
#endif // IMAP4

		else
			SetItemImage(p_treeview->itemNew.hItem, QCMailboxImageList::IMAGE_NORMAL_MBOX, QCMailboxImageList::IMAGE_NORMAL_MBOX);
		break;
	case TVE_EXPAND:
		if (ITEM_ROOT == p_dataitem->m_itemType)
			SetItemImage(p_treeview->itemNew.hItem, QCMailboxImageList::IMAGE_EUDORA, QCMailboxImageList::IMAGE_EUDORA);
		else if (ITEM_FOLDER == p_dataitem->m_itemType)
			SetItemImage(p_treeview->itemNew.hItem, QCMailboxImageList::IMAGE_OPEN_FOLDER, QCMailboxImageList::IMAGE_OPEN_FOLDER);
#ifdef IMAP4

		else if ( (ITEM_IMAP_ACCOUNT == p_dataitem->m_itemType)		||
				  (ITEM_IMAP_NAMESPACE == p_dataitem->m_itemType)	||
				  (ITEM_IMAP_MAILBOX == p_dataitem->m_itemType) )
		{
			UpdateImapItemImage (p_treeview->itemNew.hItem, p_treeview->action);

		}

#endif // IMAP4
		else
			SetItemImage(p_treeview->itemNew.hItem, QCMailboxImageList::IMAGE_NORMAL_MBOX, QCMailboxImageList::IMAGE_NORMAL_MBOX);
		break;
	default:
		ASSERT(0);
		break;
	}

	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// DoSomethingWhileUserPausedMouseAtPoint [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void QCMailboxTreeCtrl::DoSomethingWhileUserPausedMouseAtPoint(CPoint pt)
{
	//
	// Check to see if we hit a folder item.  If so, then handle
	// the auto-open behavior here.
	//
	UINT flags = TVHT_ONITEM;
	HTREEITEM h_target = HitTest(pt, &flags);
	if (h_target != NULL)
	{
		CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_target);
		ASSERT(p_itemdata != NULL);
		switch (p_itemdata->m_itemType)
		{
		case ITEM_ROOT:
		case ITEM_FOLDER:
			{
				//
				// If we're doing a mailbox move operation via
				// an internal drag and drop, then don't execute
				// the auto-open action.  The problem seems to be
				// that the auto-close will crash due to stale 
				// HTREEITEM handles!
				//
				UINT state = GetItemState(h_target, TVIS_EXPANDED | TVIS_DROPHILITED);
				if (state & TVIS_DROPHILITED)
				{
					if (0 == (state & TVIS_EXPANDED))
					{
						//
						// User has paused over a closed folder, so let's
						// auto-open it for them.
						//
						CImageList::DragShowNolock(FALSE);		// allow window updates

						Expand(h_target, TVE_EXPAND);

						//
						// Okay, the TVM_EXPAND message does not send the corresponding
						// TVN_ITEMEXPANDED notification message like you might
						// expect, so we have to do it ourselves.  Whatta pain.
						//
						NM_TREEVIEW nmtv;
						nmtv.action = TVE_EXPAND;
						nmtv.itemNew.mask = TVIF_HANDLE | TVIF_PARAM;
						nmtv.itemNew.hItem = h_target;
						nmtv.itemNew.lParam = GetItemData(h_target);
						LRESULT unused;
						OnItemExpanded((NMHDR* ) &nmtv, &unused);

						CImageList::DragShowNolock(TRUE);	// lock window updates

						//
						// Keep track of which folders we've auto-opened 
						// so that we can auto-close them when we're done.
						//
						m_autoOpenFolderList.AddTail((void *) h_target);
					}
				}
			}
			break;
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
		case ITEM_USER_MBOX:
			break;
		default:
			ASSERT(0);
			break;
		}
	}
}


HTREEITEM QCMailboxTreeCtrl::GetParentFromPathname(
LPCSTR	szPathname )
{
	HTREEITEM	hParent;
	CString		szParentPath;
	CString		szChildPath;
	char		szDrive[ _MAX_DRIVE ];
	char		szDir[ _MAX_DIR ];
	char		szFname[ _MAX_FNAME ];
	char		szExt[ _MAX_EXT ];
	
	VERIFY( hParent = GetRootItem() );
	
	szParentPath = EudoraDir.Left( EudoraDir.GetLength() - 1 );

	_splitpath( szPathname, szDrive, szDir, szFname, szExt );

	szChildPath = szDrive;
	szChildPath += szDir;
	szChildPath = szChildPath.Left( szChildPath.GetLength() - 1 );

	return GetParentFromPathname( hParent, szParentPath, szChildPath );
}


HTREEITEM QCMailboxTreeCtrl::GetParentFromPathname(
HTREEITEM		hParent,
LPCSTR			szParentPath,
const CString&	szChildPath )
{
	CMboxTreeItemData*	pItemData;

	if( szChildPath.CompareNoCase( szParentPath ) == 0 )
	{
		return hParent;
	}

#ifdef IMAP4
	// This fixes a bug that arises if we have mailbox names like mbox and mbox0!
	if( ( (size_t) szChildPath.GetLength() > strlen ( szParentPath ) ) &&
		( strnicmp( szParentPath, szChildPath, strlen( szParentPath ) ) == 0 ) &&
		( szChildPath.GetAt( strlen( szParentPath ) ) == '\\' ) )

#else // Original
	if( strnicmp( szParentPath, szChildPath, strlen( szParentPath ) ) == 0 )
#endif // IMAP4
	{
		// its in this sub tree
		hParent = GetChildItem( hParent );
	}			
	else
	{		
		// check the next sibling
		hParent = GetNextSiblingItem( hParent );
	}	

	if( hParent )
	{
		pItemData = (CMboxTreeItemData *) GetItemData( hParent );
		
		if( ( pItemData != NULL ) && ( pItemData->m_pCommand != NULL ) )
		{
#ifdef IMAP4
			// Special for IMAP
			if ( (pItemData->m_itemType == ITEM_IMAP_ACCOUNT) ||
				 (pItemData->m_itemType == ITEM_IMAP_MAILBOX) ||
				 (pItemData->m_itemType == ITEM_IMAP_NAMESPACE) )
			{
				CString Dir;
				((QCImapMailboxCommand *) pItemData->m_pCommand)->GetObjectDirectory (Dir);

				return GetParentFromPathname( hParent, Dir, szChildPath );
			}
#endif // IMAP4	
			return GetParentFromPathname( hParent, pItemData->m_pCommand->GetPathname(), szChildPath );
		}
		else
		{
			ASSERT( 0 );
		}
	}
	
	return NULL;
}


void QCMailboxTreeCtrl::CheckItemByMenuId(const CString& itemFilename, BOOL isChecked)
{
}


#ifdef IMAP4  //=========================================================//

// virtual
BOOL QCMailboxTreeCtrl::GetImapItemStruct(TV_INSERTSTRUCT &tvstruct,
							ItemType itemType,
							const CString& itemName, 
							QCMailboxCommand* pCommand,
							BOOL isChecked)
{
	HTREEITEM	newItem = NULL;
	CString		szObjectDir;

	// Must have a valid command object.
	if (!pCommand)
		return FALSE;

	//
	// Initialize record.
	//
	tvstruct.hParent = NULL;			// normally overwritten later
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
	tvstruct.item.hItem = 0;
	tvstruct.item.state = 0;			// may be overwritten later
	tvstruct.item.stateMask = TVIS_BOLD;	
	tvstruct.item.pszText = (char *) ((const char *) itemName);
	tvstruct.item.cchTextMax = -1;		// unused
	tvstruct.item.iImage = -1;			// normally overwritten later
	tvstruct.item.iSelectedImage = -1;	// normally overwritten later
	tvstruct.item.lParam = NULL;		// normally overwritten later

	//
	// Create an item data object and attach it to the item.
	//
	CMboxTreeItemData* p_itemdata = new CMboxTreeItemData(itemType, pCommand);
	if (p_itemdata != NULL)
		tvstruct.item.lParam = long(p_itemdata);	// cast to long
	else
		return FALSE;		// out of memory

	// Cast this now.
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

	//
	// Then, handle item-specific stuff.
	//
	if (isChecked)
		tvstruct.item.state = TVIS_BOLD;

	switch (itemType)
	{
		case ITEM_IMAP_ACCOUNT:		// Can now be at any depth.
			tvstruct.hParent = TVI_ROOT;

			tvstruct.item.iImage = QCMailboxImageList::IMAGE_EUDORA;	// JOK - change to image of a server.
			tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_EUDORA;
			break;

		case ITEM_IMAP_NAMESPACE:
			pImapCommand->GetObjectDirectory(szObjectDir);
			tvstruct.hParent = ImapGetParentFromPathname( szObjectDir );

			tvstruct.item.iImage = QCMailboxImageList::IMAGE_CLOSED_FOLDER;
			tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_CLOSED_FOLDER;
			break;


		case ITEM_IMAP_MAILBOX:
			pImapCommand->GetObjectDirectory(szObjectDir);
			tvstruct.hParent = ImapGetParentFromPathname( szObjectDir );

			if (pImapCommand->CanHaveChildren())
			{
				if ( pImapCommand->IsReadOnly() )
				{
					tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_READ;
					tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_READ;
				}
				else
				{
					tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_FOLDER;
					tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_FOLDER;
				}
			}
			else
			{
				if ( pImapCommand->IsReadOnly() )
				{
					tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_MBOX_READ;
					tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_MBOX_READ;
				}
				else
				{
					tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_MBOX;
					tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_MBOX;
				}
			}
			break;

	default:
			ASSERT(0);
			break;
	}

	ASSERT(tvstruct.hParent != NULL);

	return (TRUE);							
}

////////////////////////////////////////////////////////////////////////
// AddImapItem [public]
//
// As AddItem, except that now we may pass the parent tree item
////////////////////////////////////////////////////////////////////////
BOOL QCMailboxTreeCtrl::AddImapItem(
	ItemType itemType,
	const CString& itemName, 
	QCMailboxCommand* pCommand,
	BOOL isChecked)
{
	TV_INSERTSTRUCT tvstruct;
	if (GetImapItemStruct(tvstruct, itemType, itemName, pCommand, isChecked))
	{
		// Do the insert...
		HTREEITEM newItem = InsertItem(&tvstruct);

		// Expand accounts.
		if( newItem != NULL)
		{
			if( itemType == ITEM_IMAP_ACCOUNT )
			{
				PostMessage( TVM_EXPAND, WPARAM( TVE_EXPAND ), LPARAM( newItem ) ); 			
			}
		}
		else
			ASSERT(0);
	}

	return TRUE;
}



// ImapGetParentFromPathname
// NOTES
// This is different from GetParentFromPathname() in that it searches IMAP trees only.
// END NOTES

HTREEITEM QCMailboxTreeCtrl::ImapGetParentFromPathname(
LPCSTR			szPathname )
{
	HTREEITEM hParent, hItem;
	CMboxTreeItemData* p_itemdata;
	CString		szParentPath;
	CString		szChildPath;
	char		szDrive[ _MAX_DRIVE ];
	char		szDir[ _MAX_DIR ];
	char		szFname[ _MAX_FNAME ];
	char		szExt[ _MAX_EXT ];

	// Start at the root.
	VERIFY( hParent = GetRootItem() );

	_splitpath( szPathname, szDrive, szDir, szFname, szExt );

	szChildPath = szDrive;
	szChildPath += szDir;
	szChildPath = szChildPath.Left( szChildPath.GetLength() - 1 );

	// Loop through top level nodes.
	hItem = NULL;

	while (hParent)
	{
		p_itemdata = (CMboxTreeItemData *) GetItemData(hParent);
		if (p_itemdata && p_itemdata->m_pCommand)
		{
			if ( (p_itemdata->m_itemType == ITEM_IMAP_ACCOUNT) )
			{
				((QCImapMailboxCommand *) p_itemdata->m_pCommand)->GetObjectDirectory( szParentPath );
				hItem = GetParentFromPathname(hParent, szParentPath, szChildPath);
				if (hItem)
					break;
			}
		}

		hParent = GetNextSiblingItem ( hParent );
	}
	
	return hItem;
}



////////////////////////////////////////////////////////////////////////
// OnItemExpanded [protected]
//
// Handles QCTreeCtrl expanded/collapsed notification message by updating
// the graphic associated with the changed tree item.
//
////////////////////////////////////////////////////////////////////////
void QCMailboxTreeCtrl::UpdateImapItemImage (HTREEITEM hItem, UINT action)
{
	// Must have a non-zero item
	if (!hItem)
	{
		ASSERT (0);
		return;
	}

	//
	// Get the item's data.
	//
	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
	if (!p_itemdata)
	{
		ASSERT (0);
		return;
	}

	//
	// Must have a command object.
	//
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;

	if (! ( pImapCommand && hItem) )
	{
		ASSERT (0);
		return;
	}

	//
	// Verify that it's of the right class type.
	//
	if( ! ( pImapCommand->IsKindOf( RUNTIME_CLASS ( QCImapMailboxCommand ) ) ) )
	{
		ASSERT (0);
		return;
	}

	//
	// Type of IMAP mailbox/object
	//
	switch (p_itemdata->m_itemType)
	{
		// ACCOUNTS are easy. 
		case ITEM_IMAP_ACCOUNT:
			SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_ACCOUNT, QCMailboxImageList::IMAGE_IMAP_ACCOUNT);
			break;

		// Name spaces not yet handled.
		case ITEM_IMAP_NAMESPACE:
			SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER, QCMailboxImageList::IMAGE_IMAP_FOLDER);
			break;

		case  ITEM_IMAP_MAILBOX:
			//
			// expandable mailbox?
			//
			if ( pImapCommand->CanHaveChildren () )
			{
				// Is it expanded??
				BOOL bIsExpanded = GetItemState( hItem, TVIS_EXPANDED ) & TVIS_EXPANDED;

				if ( (action == TVE_EXPAND) || ( (action == 0) && bIsExpanded)  )
				{
					// Is it read-only??:
					if ( pImapCommand->IsReadOnly() )
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_READ, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_READ);
					else
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN);
				}
				else
				{
					// Is it read-only?
					if ( pImapCommand->IsReadOnly() )
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_READ, QCMailboxImageList::IMAGE_IMAP_FOLDER_READ);
					else
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER, QCMailboxImageList::IMAGE_IMAP_FOLDER);
				}
			}
			// no-inferiors mailbox.
			else
			{
				if ( pImapCommand->IsReadOnly() )
					SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_MBOX_READ, QCMailboxImageList::IMAGE_IMAP_MBOX_READ);
				else
					SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_MBOX, QCMailboxImageList::IMAGE_IMAP_MBOX);
			}

			break;

		default:
		ASSERT(0);
		break;
	}
}



#endif // IMAP4

