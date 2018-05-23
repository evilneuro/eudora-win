// QCMailboxTreeCtrl.cpp
//
// Smart mailbox-specific tree control that is meant to be embedded
// in a parent "dialog" class.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "resource.h"

#include "QCMailboxTreeCtrl.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "mboxtree.h"
#include "DynamicMailboxMenu.h"
#include "fileutil.h"

#ifdef IMAP4
#include "QCImapMailboxCommand.h"
#include "ImapMailbox.h"
#endif  // IMAP4

#include "DebugNewHelpers.h"

extern CString EudoraDirNoBackslash;		// avoids inclusion of fileutil.h
extern QCMailboxDirector g_theMailboxDirector;


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
	m_hRecentFolderItem = NULL;
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
							const char* itemName, 
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
	tvstruct.item.pszText = (char*)itemName;
	tvstruct.item.cchTextMax = -1;		// unused
	tvstruct.item.iImage = -1;			// normally overwritten later
	tvstruct.item.iSelectedImage = -1;	// normally overwritten later
	tvstruct.item.lParam = NULL;		// normally overwritten later

	//
	// Create an item data object and attach it to the item.
	//
	CMboxTreeItemData* p_itemdata = DEBUG_NEW_NOTHROW CMboxTreeItemData(itemType, pCommand);
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
	case ITEM_JUNK_MBOX:
		tvstruct.hParent = GetRootItem();
		tvstruct.item.iImage = QCMailboxImageList::IMAGE_JUNK_MBOX;
		tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_JUNK_MBOX;
		break;

	case ITEM_FOLDER:	
		//
		// We have to figure out where to place this in the hierarchy.
		//
		tvstruct.hParent = pCommand? GetParentFromPathname(pCommand->GetPathname()) : GetRootItem();
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

	// 
	// find the right place to insert
	//
	
	if( tvstruct.hParent && ( itemType == ITEM_FOLDER || itemType == ITEM_USER_MBOX ) )
	{
		if( ItemHasChildren( tvstruct.hParent ) ) 
		{
			hItem = GetChildItem( tvstruct.hParent );
			
			while( hItem )
			{
				p_itemdata = ( CMboxTreeItemData* ) GetItemData( hItem );
				
				if( itemType == ITEM_FOLDER ) 
				{
					if( p_itemdata->m_itemType != ITEM_FOLDER || !p_itemdata->m_pCommand )
					{
						hItem = GetNextSiblingItem( hItem );
						continue;
					}
				}
				else
				{
					if( p_itemdata->m_itemType == ITEM_FOLDER && p_itemdata->m_pCommand )
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
HTREEITEM QCMailboxTreeCtrl::AddItem(
	ItemType itemType,
	const char* itemName, 
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
			else if (itemType == ITEM_TRASH_MBOX)
				UpdateRecentFolder();

			return hItem;
		}
	}

	ASSERT(0);
	return NULL;
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
// UpdateRecentFolder [public]
//
// Update the list of recent mailboxes
//
////////////////////////////////////////////////////////////////////////
void QCMailboxTreeCtrl::UpdateRecentFolder()
{
	const int MaxRecent = GetIniShort(IDS_INI_MAX_RECENT_MAILBOX);

	if (!MaxRecent)
	{
		if (m_hRecentFolderItem)
		{
			DeleteItemData(m_hRecentFolderItem);
			DeleteItem(m_hRecentFolderItem);
			m_hRecentFolderItem = NULL;
		}
	}
	else
	{
		if (!m_hRecentFolderItem)
		{
			QCMailboxDirector::BuildRecentMailboxesList();

			CRString RecentName(IDS_RECENT_MBOX_MENU);
			RecentName.Remove('&');
			TV_INSERTSTRUCT tvstruct;

			if (GetItemStruct(tvstruct, QCMailboxTreeCtrl::ITEM_FOLDER, RecentName, NULL, FALSE))
			{
				HTREEITEM hItem = GetChildItem(GetRootItem());

				// Insert after Trash mailbox
				while (hItem)
				{
					CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
					if (p_itemdata->m_itemType == ITEM_TRASH_MBOX)
						break;
					hItem = GetNextSiblingItem(hItem);
				}
				tvstruct.hParent = GetRootItem();
				tvstruct.hInsertAfter = hItem;
				VERIFY(m_hRecentFolderItem = InsertItem(&tvstruct));
			}
		}

		HTREEITEM hItem = GetChildItem(m_hRecentFolderItem);
		HTREEITEM hPrevItem = NULL;

		for (std::list<LPCTSTR>::iterator it = QCMailboxDirector::s_RecentMailboxList.begin(); it != QCMailboxDirector::s_RecentMailboxList.end(); ++it)
		{
			QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname(EudoraDir + *it);

			if (pCommand)
			{
				// Get rid of items in the tree that don't match this one, but not if we're at the
				// first item in the recent list as that may have just been added to the front
				CMboxTreeItemData* p_itemdata = hItem? (CMboxTreeItemData *) GetItemData(hItem) : NULL;
				if (it != QCMailboxDirector::s_RecentMailboxList.begin())
				{
					while (hItem)
					{
						if (p_itemdata->m_pCommand == pCommand)
							break;

						HTREEITEM hNextItem = GetNextSiblingItem(hItem);
						DeleteItemData(hItem);
						DeleteItem(hItem);
						hItem = hNextItem;
						p_itemdata = hItem? (CMboxTreeItemData *) GetItemData(hItem) : NULL;
					}
				}

				// If the mailbox on the recent list is already at the right spot in the tree,
				// then don't add anything, just go to the next tree item
				if (!p_itemdata || p_itemdata->m_pCommand != pCommand)
				{
					// Insert the first item from the recent list if it's not already there at the front
					TV_INSERTSTRUCT tvstruct;

					ItemType itemType = ITEM_USER_MBOX;
					switch (pCommand->GetType())
					{
					case MBT_IN:			itemType = ITEM_IN_MBOX; break;
					case MBT_OUT:			itemType = ITEM_OUT_MBOX; break;
					case MBT_JUNK:			itemType = ITEM_JUNK_MBOX; break;
					case MBT_TRASH:			itemType = ITEM_TRASH_MBOX; break;
					case MBT_REGULAR:		itemType = ITEM_USER_MBOX; break;
					case MBT_IMAP_MAILBOX:	itemType = ITEM_IMAP_MAILBOX; break;
					default:
						// Shouldn't get here
						ASSERT(0);
						break;
					}

					BOOL bStatus = FALSE;
					if (pCommand->IsImapType())
						bStatus = GetImapItemStruct(tvstruct, itemType, pCommand->GetName(), pCommand, pCommand->GetStatus() == US_YES);
					else
						bStatus = GetItemStruct(tvstruct, itemType, pCommand->GetName(), pCommand, pCommand->GetStatus() == US_YES);
					if (bStatus)
					{
						tvstruct.hParent = m_hRecentFolderItem;
						tvstruct.hInsertAfter = hPrevItem? hPrevItem : TVI_FIRST;
						VERIFY(hItem = InsertItem(&tvstruct));
					}
				}
			}
			hPrevItem = hItem;
			if (hItem)
				hItem = GetNextSiblingItem(hItem);
		}

		// Remove existing items at the end of the tree control that
		// are no longer on the recent mailbox list
		while (hItem)
		{
			HTREEITEM hNextItem = GetNextSiblingItem(hItem);
			DeleteItemData(hItem);
			DeleteItem(hItem);
			hItem = hNextItem;
		}
	}
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
		case ITEM_JUNK_MBOX:
		case ITEM_USER_MBOX:
			break;
		default:
			ASSERT(0);
			break;
		}
	}
}


HTREEITEM QCMailboxTreeCtrl::GetParentFromPathname(LPCSTR szPathname)
{
	char szChildPath[_MAX_PATH + 1];
	
	strcpy(szChildPath, szPathname);
	char* LastBS = strrchr(szChildPath, '\\');
	if (LastBS)
		*LastBS = 0;
	else
		ASSERT(0);

	return GetParentFromPathname( GetRootItem(), EudoraDirNoBackslash, szChildPath );
}


HTREEITEM QCMailboxTreeCtrl::GetParentFromPathname(HTREEITEM hParent, LPCSTR szParentPath, LPCSTR szChildPath)
{
	const int ChildPathLen = strlen(szChildPath);
	CString Dir;

	while (1)
	{
		if ( stricmp(szChildPath, szParentPath ) == 0 )
			return hParent;

		// This fixes a bug that arises if we have mailbox names like mbox and mbox0!
		const int ParentPathLen = strlen(szParentPath);
		if (ChildPathLen > ParentPathLen &&
			szChildPath[ParentPathLen] == '\\' &&
			strnicmp(szParentPath, szChildPath, ParentPathLen) == 0)
		{
			// its in this sub tree
			hParent = GetChildItem( hParent );
		}			
		else
		{		
			// check the next sibling
			hParent = GetNextSiblingItem( hParent );
		}	

		if (!hParent)
			break;

		CMboxTreeItemData*	pItemData = (CMboxTreeItemData *) GetItemData( hParent );
		
		if (pItemData != NULL && pItemData->m_pCommand)
		{
			if ( (pItemData->m_itemType == ITEM_IMAP_ACCOUNT) ||
				 (pItemData->m_itemType == ITEM_IMAP_MAILBOX) ||
				 (pItemData->m_itemType == ITEM_IMAP_NAMESPACE) )
			{
				((QCImapMailboxCommand *) pItemData->m_pCommand)->GetObjectDirectory (Dir);
				szParentPath = Dir;
			}
			else
			{
				szParentPath = pItemData->m_pCommand->GetPathname();
			}
		}
	}
	
	return NULL;
}


void QCMailboxTreeCtrl::CheckItemByMenuId(const char* itemFilename, BOOL isChecked)
{
}


void QCMailboxTreeCtrl::RenameItemByMenuId(const char* oldItemFilename, const char* newItemFilename)
{
}


void QCMailboxTreeCtrl::RetypeItemByMenuId(const char* itemFilename, ItemType newType)
{
}


#ifdef IMAP4  //=========================================================//

// virtual
BOOL QCMailboxTreeCtrl::GetImapItemStruct(TV_INSERTSTRUCT &tvstruct,
							ItemType itemType,
							const char* itemName, 
							QCMailboxCommand* pCommand,
							BOOL isChecked)
{
	CString szObjectDir;

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
	CMboxTreeItemData* p_itemdata = DEBUG_NEW_NOTHROW CMboxTreeItemData(itemType, pCommand);
	if (p_itemdata != NULL)
		tvstruct.item.lParam = long(p_itemdata);	// cast to long
	else
		return FALSE;		// out of memory

	// Cast this now.
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

	if (!pImapCommand)
	{
		return FALSE;
	}

	//
	// Then, handle item-specific stuff.
	//

	// Bold the entry only if it has unread mail and it isn't the Junk mailbox or the user doesn't
	// want the Junk mailbox to be marked unread.
	if (isChecked && (!IsJunk(pImapCommand->GetName()) || (GetIniShort(IDS_INI_JUNK_NEVER_UNREAD) == 0)))
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
					if (pImapCommand->IsAutoSync())
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_READ_AUTOSYNC;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_READ_AUTOSYNC;
					}
					else
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_READ;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_READ;
					}
				}
				else
				{
					if (pImapCommand->IsAutoSync())
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_AUTOSYNC;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_FOLDER_AUTOSYNC;
					}
					else
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_FOLDER;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_FOLDER;
					}
				}
			}
			else
			{
				if ( pImapCommand->IsReadOnly() )
				{
					if (pImapCommand->IsAutoSync())
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_MBOX_READ_AUTOSYNC;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_MBOX_READ_AUTOSYNC;
					}
					else
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_MBOX_READ;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_MBOX_READ;
					}
				}
				else
				{
					if (pImapCommand->IsAutoSync())
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_MBOX_AUTOSYNC;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_MBOX_AUTOSYNC;
					}
					else
					{
						tvstruct.item.iImage = QCMailboxImageList::IMAGE_IMAP_MBOX;
						tvstruct.item.iSelectedImage = QCMailboxImageList::IMAGE_IMAP_MBOX;
					}
				}
			}
			break;

	default:
			ASSERT(0);
			break;
	}

	return (TRUE);							
}

////////////////////////////////////////////////////////////////////////
// AddImapItem [public]
//
// As AddItem, except that now we may pass the parent tree item
////////////////////////////////////////////////////////////////////////
BOOL QCMailboxTreeCtrl::AddImapItem(
	ItemType itemType,
	const char* itemName, 
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

HTREEITEM QCMailboxTreeCtrl::ImapGetParentFromPathname(LPCSTR szPathname)
{
	char szChildPath[_MAX_PATH + 1];
	
	strcpy(szChildPath, szPathname);
	char* LastBS = strrchr(szChildPath, '\\');
	if (LastBS)
		*LastBS = 0;
	else
		ASSERT(0);

	// Start at the root.
	HTREEITEM hParent = GetRootItem();
	HTREEITEM hItem = NULL;
	CString szParentPath;

	// Loop through top level nodes.
	while (hParent)
	{
		CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hParent);
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
					if (pImapCommand->IsAutoSync())
					{
						if ( pImapCommand->IsReadOnly() )
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_READ_AUTOSYNC, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_READ_AUTOSYNC);
						else
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_AUTOSYNC, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_AUTOSYNC);
					}
					else
					{
						if ( pImapCommand->IsReadOnly() )
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_READ, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN_READ);
						else
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN, QCMailboxImageList::IMAGE_IMAP_FOLDER_OPEN);
					}
				}
				else
				{
					// Is it read-only?
					if (pImapCommand->IsAutoSync())
					{
						if ( pImapCommand->IsReadOnly() )
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_READ_AUTOSYNC, QCMailboxImageList::IMAGE_IMAP_FOLDER_READ_AUTOSYNC);
						else
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_AUTOSYNC, QCMailboxImageList::IMAGE_IMAP_FOLDER_AUTOSYNC);
					}
					else
					{
						if ( pImapCommand->IsReadOnly() )
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER_READ, QCMailboxImageList::IMAGE_IMAP_FOLDER_READ);
						else
							SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_FOLDER, QCMailboxImageList::IMAGE_IMAP_FOLDER);
					}
				}
			}
			// no-inferiors mailbox.
			else
			{
				if (pImapCommand->IsAutoSync())
				{
					if ( pImapCommand->IsReadOnly() )
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_MBOX_READ_AUTOSYNC, QCMailboxImageList::IMAGE_IMAP_MBOX_READ_AUTOSYNC);
					else
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_MBOX_AUTOSYNC, QCMailboxImageList::IMAGE_IMAP_MBOX_AUTOSYNC);
				}
				else
				{
					if ( pImapCommand->IsReadOnly() )
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_MBOX_READ, QCMailboxImageList::IMAGE_IMAP_MBOX_READ);
					else
						SetItemImage(hItem, QCMailboxImageList::IMAGE_IMAP_MBOX, QCMailboxImageList::IMAGE_IMAP_MBOX);
				}
			}

			break;

		default:
		ASSERT(0);
		break;
	}
}



#endif // IMAP4

