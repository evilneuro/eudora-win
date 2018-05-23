// mboxtree.cpp: implementation
//
// CMboxTreeCtrl
// Smart mailbox-specific tree control that is meant to be embedded
// in a parent "dialog" class.

#include "stdafx.h"
#include "resource.h"

extern CString EudoraDir;		// avoids inclusion of fileutil.h

#include "eudora.h"
#include "ClipboardMgr.h"
#include "mboxtree.h"
#include "cursor.h"
#include "mainfrm.h"
#include "guiutils.h"
#include "helpcntx.h"
#include "resource.h"
#include "3dformv.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "fileutil.h"
#include "helpxdlg.h"
#include "newmbox.h"
#include "rs.h"
#include "utils.h"
#include "workbook.h"
#include "WazooWnd.h"
#include "newmbox.h"

#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

#include "QCFindMgr.h"
#include "SearchEngine.h"
#include "SearchView.h"
#include "SearchDoc.h"
#include "EudoraMsgs.h"

#ifdef IMAP4
#include "imapfol.h"
#include "QCImapMailboxCommand.h"
#endif

extern QCMailboxDirector	g_theMailboxDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static WNDPROC s_pOldEditProc = NULL;
static CTreeCtrl* s_pTreeCtrl = NULL;

IMPLEMENT_DYNAMIC(CMboxTreeCtrl, QCMailboxTreeCtrl)

BEGIN_MESSAGE_MAP(CMboxTreeCtrl, QCMailboxTreeCtrl)
	//{{AFX_MSG_MAP(CMboxTreeCtrl)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_RBUTTONDOWN, OnRightButtonDown)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_KEYUP()
	ON_UPDATE_COMMAND_UI(ID_MBOXTREE_DELETE, OnCmdUpdateBySelection)
	ON_UPDATE_COMMAND_UI(ID_MBOXTREE_RENAME, OnCmdUpdateBySelection)
	ON_UPDATE_COMMAND_UI(ID_MBOXTREE_NEW, OnCmdUpdateBySelection)
	ON_COMMAND(ID_MBOXTREE_OPEN, OnCmdOpen)
	ON_COMMAND(ID_MBOXTREE_FINDMESSAGES, OnCmdFindMsgs)
	ON_COMMAND(ID_MBOXTREE_DELETE, OnCmdDelete)
	ON_COMMAND(ID_MBOXTREE_RENAME, OnCmdRename)
	ON_COMMAND(ID_MBOXTREE_NEW, OnCmdNew)
	ON_COMMAND(ID_MBOXTREE_EMPTY_TRASH, OnCmdEmptyTrash)
	//}}AFX_MSG_MAP

#ifdef IMAP4
	// Updates
	ON_UPDATE_COMMAND_UI(ID_MBOXTREE_IMAP_REFRESH, OnCmdUpdateBySelection)
	ON_UPDATE_COMMAND_UI(ID_MBOXTREE_IMAP_RESYNC, OnCmdUpdateBySelection)

	// Commands
	ON_COMMAND(ID_MBOXTREE_IMAP_REFRESH, OnCmdImapRefresh)
	ON_COMMAND(ID_MBOXTREE_IMAP_RESYNC, OnCmdImapResync)
#endif

END_MESSAGE_MAP()

#ifdef IMAP4 // 
BOOL IsValidImapName (QCMailboxCommand* pCommand, LPCSTR pNewname);
BOOL ImapMboxIsDragable (QCMailboxCommand *pCommand, int itemType);
BOOL ImapTargetIsDroppable (QCMailboxCommand *pSourceCommand, int iSourceItemType,
							QCMailboxCommand *pTargetCommand, int iTargetItemType);
#endif 

/////////////////////////////////////////////////////////////////////////////
// CMboxTreeCtrl

CMboxTreeCtrl::CMboxTreeCtrl() :
	m_hDragMbox(NULL),
	m_pDragImage(NULL)/*FORNOW,
	m_bHadFocusWhenSavingState(FALSE)FORNOW*/
{
//FORNOW	m_selectedItemFilename.Empty();		// good hygiene
	g_theMailboxDirector.Register( this );
}

CMboxTreeCtrl::~CMboxTreeCtrl()
{
	//
	// Note:  clients should call Reset() before
	// we get here...  Otherwise, we will have
	// memory leaks on the heap-allocated 
	// CMboxTreeItemData objects.
	//
	g_theMailboxDirector.UnRegister( this );
}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// SetSelectedItemFilename [public]
//FORNOW//
//FORNOW// This is used during "rename" and "new" operations to update the internal
//FORNOW// pathname to the "selected" item.  In this case, the selected item
//FORNOW// (and its corresponding disk file) has just been renamed, so it doesn't
//FORNOW// make sense to use the old pathname to the selected item anymore.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMboxTreeCtrl::SetSelectedItemFilename(const CString& newFilename)
//FORNOW{
//FORNOW	ASSERT(m_selectedItemFilename.IsEmpty());
//FORNOW	ASSERT(! newFilename.IsEmpty());
//FORNOW
//FORNOW	m_selectedItemFilename = newFilename;
//FORNOW}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// SaveVisualState [public]
//FORNOW//
//FORNOW// Save the list of current "open" folders, in preparation for a tree
//FORNOW// rebuild.  We only need to save the "open" folders, since the tree
//FORNOW// control initializes all folders to the closed state.
//FORNOW//
//FORNOW// Also, save the currently selected item in the tree.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMboxTreeCtrl::SaveVisualState(void)
//FORNOW{
//FORNOW	//
//FORNOW	// Empty out the existing strings.
//FORNOW	//
//FORNOW	ASSERT(m_openFolderList.IsEmpty());
//FORNOW
//FORNOW	//
//FORNOW	// Save the fact whether we had focus or not.
//FORNOW	//
//FORNOW	ASSERT(! m_bHadFocusWhenSavingState);
//FORNOW	if (GetFocus() == this)
//FORNOW		m_bHadFocusWhenSavingState = TRUE;
//FORNOW
//FORNOW	//
//FORNOW	// Okay, hack alert.  In the normal case where we're just
//FORNOW	// saving/restoring the visual state of the tree without
//FORNOW	// actually renaming an item, then the 'm_selectedItemFilename'
//FORNOW	// member will be empty, which means that it's okay to go
//FORNOW	// ahead and overwrite it.  However, if we get here and
//FORNOW	// somebody has already set it (e.g., see the Rename and Delete
//FORNOW	// code), then honor it and don't overwrite it.  Don't worry,
//FORNOW	// it'll get cleared during the restore procedure anyway.
//FORNOW	//
//FORNOW	if (m_selectedItemFilename.IsEmpty())
//FORNOW	{
//FORNOW		//
//FORNOW		// Save the file pathname associated with the selected
//FORNOW		// item, if any.
//FORNOW		//
//FORNOW		HTREEITEM h_selitem = GetSelectedItem();
//FORNOW		if (h_selitem != NULL)
//FORNOW		{
//FORNOW			//
//FORNOW			// Save the filename associated with the selected item.
//FORNOW			//
//FORNOW			CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_selitem);
//FORNOW			ASSERT(p_itemdata != NULL);
//FORNOW			if (p_itemdata->m_pCommand != NULL)
//FORNOW				SetSelectedItemFilename( p_itemdata->m_pCommand->GetPathname() );
//FORNOW		}
//FORNOW	}
//FORNOW
//FORNOW	//
//FORNOW	// Well, just do a recursive, exhaustive walk of the tree in search
//FORNOW	// of open "folder" items and save the file pathname for each such
//FORNOW	// folder item.  For the MDI child version of this tree control,
//FORNOW	// it may be in the process of being initialized for the first time,
//FORNOW	// so silently cut it some slack if the tree control doesn't contain
//FORNOW	// any items.
//FORNOW	//
//FORNOW	HTREEITEM h_root = GetRootItem();
//FORNOW	if (h_root != NULL)
//FORNOW		DoSaveFolderStates(h_root);
//FORNOW}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// DoSaveFolderStates [private]
//FORNOW//
//FORNOW// Recursively process the subtree rooted at the given item node, in
//FORNOW// search of "open" folders.  Note:  This doesn't work for empty folders
//FORNOW// which are visually "open", but which actually contain no children.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMboxTreeCtrl::DoSaveFolderStates(HTREEITEM hItem)
//FORNOW{
//FORNOW	//
//FORNOW	// Check the item itself to see if it is an open folder.
//FORNOW	//
//FORNOW	ASSERT(hItem != NULL);
//FORNOW	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
//FORNOW	ASSERT(p_itemdata != NULL);
//FORNOW	
//FORNOW	switch (p_itemdata->m_itemType)
//FORNOW	{
//FORNOW	case ITEM_ROOT:
//FORNOW		//
//FORNOW		// The root item doesn't have a pointer to the menu object, therefore 
//FORNOW		// it doesn't contain a pathname to the Eudora mail directory.  So,
//FORNOW		// skip it since there's nothing else we can do with it.  At restore 
//FORNOW		// time, we *always* open the root item anyway.
//FORNOW		//
//FORNOW		break;
//FORNOW	case ITEM_FOLDER:
//FORNOW		{
//FORNOW			UINT state = GetItemState(hItem, TVIS_EXPANDED);
//FORNOW			if (state & TVIS_EXPANDED)
//FORNOW			{
//FORNOW				// Found an open folder, so save the corresponding folder file pathname
//FORNOW				ASSERT(p_itemdata->m_pCommand );
//FORNOW				m_openFolderList.AddTail( p_itemdata->m_pCommand->GetPathname() );
//FORNOW			}
//FORNOW			break;
//FORNOW		}
//FORNOW	case ITEM_IN_MBOX:
//FORNOW	case ITEM_OUT_MBOX:
//FORNOW	case ITEM_TRASH_MBOX:
//FORNOW	case ITEM_USER_MBOX:
//FORNOW		return;
//FORNOW
//FORNOW#ifdef IMAP4
//FORNOW	// Treat these like folders..
//FORNOW	case ITEM_IMAP_ACCOUNT:
//FORNOW	case ITEM_IMAP_NAMESPACE:
//FORNOW		{
//FORNOW			UINT state = GetItemState(hItem, TVIS_EXPANDED);
//FORNOW			if (state & TVIS_EXPANDED)
//FORNOW			{
//FORNOW				// Found an open folder, so save the corresponding folder file pathname
//FORNOW				ASSERT(p_itemdata->m_pCommand );
//FORNOW				m_openFolderList.AddTail( p_itemdata->m_pCommand->GetPathname() );
//FORNOW			}
//FORNOW			break;
//FORNOW		}
//FORNOW	// This is complicated.
//FORNOW	case ITEM_IMAP_MAILBOX:
//FORNOW		{
//FORNOW			if (p_itemdata->m_pCommand->CanHaveChildren())
//FORNOW			{
//FORNOW				// Treat as a folder.
//FORNOW				UINT state = GetItemState(hItem, TVIS_EXPANDED);
//FORNOW				if (state & TVIS_EXPANDED)
//FORNOW				{
//FORNOW					// Found an open folder, so save the corresponding folder file pathname
//FORNOW					ASSERT(p_itemdata->m_pCommand );
//FORNOW					m_openFolderList.AddTail( p_itemdata->m_pCommand->GetPathname() );
//FORNOW				}
//FORNOW				break;
//FORNOW			}
//FORNOW			else
//FORNOW				return;
//FORNOW		}
//FORNOW#endif // IMAP
//FORNOW
//FORNOW	default:
//FORNOW		ASSERT(0);
//FORNOW		return;
//FORNOW	}
//FORNOW
//FORNOW	//
//FORNOW	// If we get this far, then recursively process the children of
//FORNOW	// this folder node.
//FORNOW	//
//FORNOW	if (ItemHasChildren(hItem))
//FORNOW	{
//FORNOW		HTREEITEM h_child = GetChildItem(hItem);
//FORNOW		while (h_child != NULL)
//FORNOW		{
//FORNOW			DoSaveFolderStates(h_child);
//FORNOW			h_child = GetNextSiblingItem(h_child);
//FORNOW		}
//FORNOW	}
//FORNOW}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// RestoreVisualState [public]
//FORNOW//
//FORNOW// Restore the saved folder open/close states, presumably just after
//FORNOW// the tree has been rebuilt.  If there are any errors encountered
//FORNOW// during the restore process (e.g, an entire subtree is missing
//FORNOW// or something drastic like that) then don't sweat it.  It's not
//FORNOW// that big a deal.
//FORNOW//
//FORNOW// Also, restore the previous selection, if any.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMboxTreeCtrl::RestoreVisualState(void)
//FORNOW{
//FORNOW	HTREEITEM h_root = GetRootItem();
//FORNOW
//FORNOW	if (h_root != NULL)
//FORNOW	{
//FORNOW		//
//FORNOW		// Special processing for the root node -- we *always*
//FORNOW		// open it.
//FORNOW		//
//FORNOW		Expand(h_root, TVE_EXPAND);
//FORNOW
//FORNOW		//
//FORNOW		// Now, process the previously stored list of open
//FORNOW		// folders.
//FORNOW		//
//FORNOW		while (! m_openFolderList.IsEmpty())
//FORNOW		{
//FORNOW			CString pathname = m_openFolderList.RemoveHead();
//FORNOW
//FORNOW			DoRestoreFolderStates(h_root, pathname);
//FORNOW		}
//FORNOW
//FORNOW		//
//FORNOW		// Finally, restore the selection to the previously
//FORNOW		// selected item.
//FORNOW		//
//FORNOW		if (! m_selectedItemFilename.IsEmpty())
//FORNOW		{
//FORNOW			DoRestoreSelection(h_root, m_selectedItemFilename);
//FORNOW			m_selectedItemFilename.Empty();
//FORNOW		}
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		//
//FORNOW		// Something is wrong, but make sure we reset the 
//FORNOW		// visual state items anyway.
//FORNOW		//
//FORNOW		ASSERT(0);
//FORNOW		while (! m_openFolderList.IsEmpty())
//FORNOW			m_openFolderList.RemoveHead();
//FORNOW		m_selectedItemFilename.Empty();
//FORNOW	}
//FORNOW
//FORNOW	if (m_bHadFocusWhenSavingState)
//FORNOW		SetFocus();
//FORNOW	m_bHadFocusWhenSavingState = FALSE;
//FORNOW}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// DoRestoreFolderStates [private]
//FORNOW//
//FORNOW// Recursively search the tree rooted at the given HTREEITEM for an
//FORNOW// item with a matching file pathname.  Return TRUE as soon as a match is
//FORNOW// found for the given pathname.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWBOOL CMboxTreeCtrl::DoRestoreFolderStates(HTREEITEM hItem, const CString& fileName)
//FORNOW{
//FORNOW	//
//FORNOW	// Check the item itself to see if it is an open folder.
//FORNOW	//
//FORNOW	ASSERT(hItem != NULL);
//FORNOW	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
//FORNOW	ASSERT(p_itemdata != NULL);
//FORNOW	
//FORNOW	switch (p_itemdata->m_itemType)
//FORNOW	{
//FORNOW	case ITEM_ROOT:
//FORNOW		//
//FORNOW		// The root item doesn't have a pointer to the menu object, therefore 
//FORNOW		// it doesn't contain a pathname to the Eudora mail directory.  So,
//FORNOW		// be sure to skip it.
//FORNOW		//
//FORNOW		break;
//FORNOW	case ITEM_FOLDER:
//FORNOW		ASSERT( p_itemdata->m_pCommand );
//FORNOW		if ( fileName.CompareNoCase( p_itemdata->m_pCommand->GetPathname() ) == 0)
//FORNOW		{
//FORNOW			Expand(hItem, TVE_EXPAND);
//FORNOW			return TRUE;
//FORNOW		}
//FORNOW		break;
//FORNOW	case ITEM_IN_MBOX:
//FORNOW	case ITEM_OUT_MBOX:
//FORNOW	case ITEM_TRASH_MBOX:
//FORNOW	case ITEM_USER_MBOX:
//FORNOW		return FALSE;
//FORNOW
//FORNOW#ifdef IMAP4
//FORNOW	case ITEM_IMAP_ACCOUNT:
//FORNOW	case ITEM_IMAP_NAMESPACE:
//FORNOW		ASSERT( p_itemdata->m_pCommand );
//FORNOW		if ( fileName.CompareNoCase( p_itemdata->m_pCommand->GetPathname() ) == 0)
//FORNOW		{
//FORNOW			Expand(hItem, TVE_EXPAND);
//FORNOW			return TRUE;
//FORNOW		}
//FORNOW		break;
//FORNOW	case ITEM_IMAP_MAILBOX:
//FORNOW		ASSERT( p_itemdata->m_pCommand );
//FORNOW		if ( p_itemdata->m_pCommand->CanHaveChildren())
//FORNOW		{
//FORNOW			if ( fileName.CompareNoCase( p_itemdata->m_pCommand->GetPathname() ) == 0)
//FORNOW			{
//FORNOW				Expand(hItem, TVE_EXPAND);
//FORNOW				return TRUE;
//FORNOW			}
//FORNOW			break;
//FORNOW		}
//FORNOW		else
//FORNOW			return FALSE;
//FORNOW#endif // IMAP4
//FORNOW
//FORNOW	default:
//FORNOW		ASSERT(0);
//FORNOW		return FALSE;
//FORNOW	}
//FORNOW
//FORNOW	//
//FORNOW	// If we get this far, then recursively process the children of
//FORNOW	// this root/folder node.
//FORNOW	//
//FORNOW	if (ItemHasChildren(hItem))
//FORNOW	{
//FORNOW		HTREEITEM h_child = GetChildItem(hItem);
//FORNOW		while (h_child != NULL)
//FORNOW		{
//FORNOW			if (DoRestoreFolderStates(h_child, fileName))
//FORNOW				return TRUE;
//FORNOW			h_child = GetNextSiblingItem(h_child);
//FORNOW		}
//FORNOW	}
//FORNOW	return FALSE;
//FORNOW}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// DoRestoreSelection [private]
//FORNOW//
//FORNOW// Recursively search the tree rooted at the given HTREEITEM for an
//FORNOW// item with a matching file pathname.  Return TRUE as soon as a match is
//FORNOW// found for the given pathname.  When a match is found, set the
//FORNOW// selection to that item.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWBOOL CMboxTreeCtrl::DoRestoreSelection(HTREEITEM hItem, const CString& fileName)
//FORNOW{
//FORNOW	//
//FORNOW	// Check the item itself to see if it is the one we're looking for.
//FORNOW	//
//FORNOW	ASSERT(! fileName.IsEmpty());
//FORNOW	ASSERT(hItem != NULL);
//FORNOW	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
//FORNOW	ASSERT(p_itemdata != NULL);
//FORNOW	
//FORNOW	switch (p_itemdata->m_itemType)
//FORNOW	{
//FORNOW	case ITEM_ROOT:
//FORNOW		//
//FORNOW		// The root item doesn't have a pointer to the menu object, therefore 
//FORNOW		// it doesn't contain a pathname to the Eudora mail directory.  So,
//FORNOW		// be sure to skip it.
//FORNOW		//
//FORNOW		break;
//FORNOW	case ITEM_FOLDER:
//FORNOW	case ITEM_IN_MBOX:
//FORNOW	case ITEM_OUT_MBOX:
//FORNOW	case ITEM_TRASH_MBOX:
//FORNOW	case ITEM_USER_MBOX:
//FORNOW#ifdef IMAP4
//FORNOW	case ITEM_IMAP_ACCOUNT:
//FORNOW	case ITEM_IMAP_NAMESPACE:
//FORNOW	case ITEM_IMAP_MAILBOX:
//FORNOW#endif // IMAP4
//FORNOW		ASSERT(p_itemdata->m_pCommand);
//FORNOW		if (fileName.CompareNoCase( p_itemdata->m_pCommand->GetPathname() ) == 0)
//FORNOW		{
//FORNOW			SelectItem(hItem);
//FORNOW			EnsureVisible(hItem);
//FORNOW			return TRUE;
//FORNOW		}
//FORNOW		break;
//FORNOW	default:
//FORNOW		ASSERT(0);
//FORNOW		return FALSE;
//FORNOW	}
//FORNOW
//FORNOW	//
//FORNOW	// If we get this far, then recursively process the children of
//FORNOW	// this root/folder node.
//FORNOW	//
//FORNOW	if (ItemHasChildren(hItem))
//FORNOW	{
//FORNOW		HTREEITEM h_child = GetChildItem(hItem);
//FORNOW		while (h_child != NULL)
//FORNOW		{
//FORNOW			if (DoRestoreSelection(h_child, fileName))
//FORNOW				return TRUE;
//FORNOW			h_child = GetNextSiblingItem(h_child);
//FORNOW		}
//FORNOW	}
//FORNOW	return FALSE;
//FORNOW}

// --------------------------------------------------------------------------

HTREEITEM CMboxTreeCtrl::GetNextWrapItem(HTREEITEM hItem) const
{
	ASSERT(hItem);

	if (ItemHasChildren(hItem))
		return (GetChildItem(hItem));

	HTREEITEM hSibling;

	while (!(hSibling = GetNextSiblingItem(hItem)))
	{
		if ((hItem = GetParentItem(hItem)) == NULL)
		{
			// Got to bottom, now wrap to top
			return GetRootItem();
		}
	}

	return hSibling;
}

// --------------------------------------------------------------------------

//
// FIND TEXT
//

void CMboxTreeCtrl::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CMboxTreeCtrl::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG CMboxTreeCtrl::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (!pFindMgr)
		return (EuFIND_ERROR);

	// This is our internal message to ask if we support find.
	// Return non-zero (TRUE).
	if (pFindMgr->IsAck(wParam, lParam))
		return (EuFIND_ACK_YES);

	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if (!lpFR)
		return (EuFIND_ERROR);

	if (lpFR->Flags & FR_DIALOGTERM)
	{
		ASSERT(0); // Should never fwd a terminating msg
		return (EuFIND_ERROR);
	}

	// Do search from selected item forward
	HTREEITEM hStartItem = GetSelectedItem();
	
	// If nothing selected, use root
	if (NULL == hStartItem)
		hStartItem = GetRootItem();

	// Get next item (find starts AFTER selection)
	hStartItem = GetNextWrapItem(hStartItem);

	HTREEITEM hItem = hStartItem;

	while (hItem)
	{
		if( QCStrstr(GetItemText(hItem),
				lpFR->lpstrFindWhat,  
				((lpFR->Flags & FR_MATCHCASE) != 0), 
				(((lpFR->Flags & FR_WHOLEWORD) != 0)?QCSTR_WORD_SEARCH:QCSTR_SUBSTR_SEARCH) ) )
		{
			SelectItem(hItem);
			EnsureVisible(hItem);

			return (EuFIND_OK);
		}

		hItem = GetNextWrapItem(hItem);

		if (hItem == hStartItem)
		{
			// Gone all the way around without a match
			return (EuFIND_NOTFOUND);
		}
	}

	return (EuFIND_NOTFOUND);
}

// --------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////
// CheckItemByMenuId [public]
//
// For tree controls, we define "checking" an item to mean setting
// the label text to bold.  Unchecking means to set the label text
// back to a normal (non-bold) font.
//
// This routine initiates a recursive, depth-first search of the tree 
// hierarchy, using the private DoCheckItemByMenuId() method to
// do all the work.
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::CheckItemByMenuId(const CString& itemFilename, BOOL isChecked)
{
	VERIFY(DoCheckItemByMenuId(GetRootItem(), itemFilename, isChecked));
}


////////////////////////////////////////////////////////////////////////
// DoCheckItemByMenuId [private]
//
// This routine does a recursive, in-order search of the tree 
// hierarchy rooted at the given 'hItem', returning TRUE as soon
// as it processes an item with a matching menu ID.
//
////////////////////////////////////////////////////////////////////////
BOOL CMboxTreeCtrl::DoCheckItemByMenuId(HTREEITEM hItem, const CString& itemFilename, BOOL isChecked)
{
	//
	// Check the item itself to see if has a matching menu ID.
	//
	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(hItem);
	ASSERT(p_itemdata != NULL);
	
	if (p_itemdata->m_pCommand != NULL)
	{
		if(itemFilename.CompareNoCase(p_itemdata->m_pCommand->GetPathname() ) == 0)
		{
			//
			// Found matching menuID key, so do the "check" vs. "uncheck" 
			// processing and return.
			//
			TV_ITEM tv_item;
			tv_item.mask = TVIF_HANDLE | TVIF_STATE;
			tv_item.hItem = hItem;
			if (isChecked)
				tv_item.state = TVIS_BOLD;
			else
				tv_item.state = 0;
			tv_item.stateMask = TVIS_BOLD;
			SetItem(&tv_item);
			return TRUE;
		}
	}

	//
	// If we get this far, extend the search to all child items.
	//
	if (ItemHasChildren(hItem))
	{
		HTREEITEM h_child = GetChildItem(hItem);
		while (h_child != NULL)
		{
			if (DoCheckItemByMenuId(h_child, itemFilename, isChecked))
				return TRUE;
			h_child = GetNextSiblingItem(h_child);
		}
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// SiblingHasSameName [private]
//
// Returns TRUE if any sibling of the given item has the same label
// as the given name.
////////////////////////////////////////////////////////////////////////
BOOL CMboxTreeCtrl::SiblingHasSameName(HTREEITEM hItem, const CString& labelName)
{
	//
	// The fastest way to find the "first" sibling is probably
	// to start at the parent and walk the list of children.
	//
	ASSERT(hItem != NULL);
	HTREEITEM h_parent = GetParentItem(hItem);
	if (NULL == h_parent)
	{
		ASSERT(0);
		return TRUE;	// most conservative answer, given the circumstances
	}

	//
	// Now, walk the list of children searching for a matching name.
	// Note that we don't worry about the optimization of checking
	// against the hItem that we stared with in the first place.
	//
	HTREEITEM h_child = GetChildItem(h_parent);		// get first child
	while (h_child != NULL)
	{
		CString name = GetItemText(h_child);
		if (labelName.CompareNoCase(name) == 0)
			return TRUE;
		h_child = GetNextSiblingItem(h_child);
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CMboxTreeCtrl message handlers


////////////////////////////////////////////////////////////////////////
// TreeEditLabelProc [extern]
//
// This is the window procedure used to subclass the temporary edit
// control created for editing tree control labels.  This subclassing
// is necessary to work around a known bug where tree control edit
// controls do not receive the Enter and Esc keystrokes since they
// are hooked by the parent dialog Proc.
//
////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK TreeEditLabelProc(HWND hWnd, WORD wMessage, WORD wParam, LONG lParam)
{
	switch (wMessage)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_CHAR:
		if (VK_RETURN == wParam)
		{
			//
			// Accept the edit.
			//
			ASSERT(s_pTreeCtrl != NULL);
			s_pTreeCtrl->SendMessage(TVM_ENDEDITLABELNOW, FALSE, 0);
			return 0;		// indicates keystroke was handled
		}
		else if (VK_ESCAPE == wParam)
		{
			//
			// Cancel the edit.
			//
			ASSERT(s_pTreeCtrl != NULL);
			s_pTreeCtrl->SendMessage(TVM_ENDEDITLABELNOW, TRUE, 0);
			return 0;		// indicates keystroke was handled
		}
		// else fall through...
	default:
		ASSERT(s_pOldEditProc != NULL);
		return CallWindowProc(s_pOldEditProc, hWnd, wMessage, wParam, lParam);
	}
} 


////////////////////////////////////////////////////////////////////////
// OnEnterKeyPressed [public]
//
// Since this tree control is embedded in a "dialog-like" window,
// the parent window traps "Enter" key presses as IDOK commands.
// Therefore, this is just a publically available hook for
// parent windows to pass the IDOK command messages to the
// child tree control.
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnEnterKeyPressed(void)
{
	if (IsWinNT() && GetEditControl())
	{
		//
		// We don't do the wicked edit control subclassing thing under
		// NT, so use the IDOK hook from the Enter key to commit and
		// terminate the label edit mode.
		//
		SendMessage(TVM_ENDEDITLABELNOW, FALSE, 0);
		return;
	}

	//
	// Win 95 intercepts the Enter key when we're label edit mode,
	// so we can assume that we never get here when we're in
	// label editing mode.
	//
	ASSERT(NULL == GetEditControl());
	OnLButtonDblClk(0, CPoint(-1, -1));
}


////////////////////////////////////////////////////////////////////////
// OnEscKeyPressed [public]
//
// Since this tree control is embedded in a "dialog-like" window,
// the parent window traps "Esc" key presses as IDCANCEL commands.
// Therefore, this is just a publically available hook for
// parent windows to pass the IDCANCEL command messages to the
// child tree control.
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnEscKeyPressed(void)
{
	if (IsWinNT() && GetEditControl())
	{
		//
		// We don't do the wicked edit control subclassing thing under
		// NT, so use the IDCANCEL hook from the Esc key to cancel and
		// terminate the label edit mode.
		//
		SendMessage(TVM_ENDEDITLABELNOW, TRUE, 0);
		return;
	}

	ASSERT(NULL == GetEditControl());
	if (m_pDragImage)
	{
		m_pDragImage->DragLeave(this);
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage = NULL;

		if (GetCapture() == this)
			ReleaseCapture();

		KillAutoScrollTimer();

		SelectDropTarget(NULL);
		ASSERT(m_hDragMbox != NULL);
		m_hDragMbox = NULL;
		::SetCursor(::LoadCursor(0, IDC_ARROW));

		AutoCloseOpenedFolders();
	}
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CMboxTreeCtrl::OnDrop(COleDataObject* pDataObject,
						   DROPEFFECT dropEffect, 
						   CPoint point)
{
	BOOL	bReturn;

	ASSERT_VALID(this);

	bReturn = FALSE;

	KillAutoScrollTimer();

	//
	// Do a hit test to see if we're over a valid mailbox target.
	//
	UINT flags = TVHT_ONITEM;
	HTREEITEM h_target = HitTest(point, &flags);
	if (h_target != NULL)
	{
		CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_target);
		ASSERT(p_itemdata != NULL);
		switch (p_itemdata->m_itemType)
		{
		case ITEM_ROOT:
		case ITEM_FOLDER:
			break;

#ifdef IMAP4
		case ITEM_IMAP_MAILBOX:
		{
			// Must "CanCantainMessages".
			QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;
			if (! (pImapCommand && pImapCommand->CanContainMessages()) )
			{
				break;
			}
			// Fall through if selectable:
		}
#endif   // IMAP4
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
		case ITEM_USER_MBOX:
			{
				// make sure that we're dragging and dropping in the same copy of Eudora
				if (! g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_TRANSFER))
				{
					ASSERT(0);
					break;
				}

				// Get global TOC clipboard data from COleDataObject
				QCFTRANSFER qcfTransfer;
				if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfTransfer))
				{
					ASSERT(0);
					break;
				}

				ASSERT_KINDOF(CTocDoc, qcfTransfer.m_pSourceTocDoc);

				// Get drop target item data
				CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_target);
				ASSERT(p_itemdata != NULL);
				ASSERT(p_itemdata->m_pCommand != NULL);

				//
				// Determine what type of command to perform.
				//
				COMMAND_ACTION_TYPE cmdType = CA_NONE;
				if( DROPEFFECT_COPY == ( dropEffect & DROPEFFECT_COPY ) )
					cmdType = CA_COPY;
				else if( DROPEFFECT_MOVE == ( dropEffect & DROPEFFECT_MOVE ) )
					cmdType = CA_TRANSFER_TO;
				else
				{
					ASSERT(0);	// unknown drop effect
				}				

				// Do the transfer!
				if (cmdType != CA_NONE)
				{
					if (qcfTransfer.m_pSingleSummary)
					{
						// Transfer/Copy a single message (typically tow trucked)
						ASSERT_KINDOF(CSummary, qcfTransfer.m_pSingleSummary);
						ASSERT(qcfTransfer.m_pSourceTocDoc == qcfTransfer.m_pSingleSummary->m_TheToc);
						p_itemdata->m_pCommand->Execute( cmdType, qcfTransfer.m_pSingleSummary );
					}
					else
					{
						// Transfer/Copy one or more selected messages from a TOC
						p_itemdata->m_pCommand->Execute( cmdType, qcfTransfer.m_pSourceTocDoc );
					}
				}

				bReturn = TRUE;
			}
			break;
#ifdef IMAP4	// Not allowed.
		case ITEM_IMAP_ACCOUNT:
		case ITEM_IMAP_NAMESPACE:
			break;
#endif 
		default:
			ASSERT(0);
			break;
		}
	}
	
	// turn off the drop target
	::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_DROPHILITE, NULL );

	AutoCloseOpenedFolders();

	return bReturn;
}


////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMboxTreeCtrl::OnDragEnter(COleDataObject* pDataObject,
									  DWORD dwKeyState, 
									  CPoint point)
{
	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_TRANSFER))
	{
		StartAutoScrollTimer();
	}

	//
	// Reset the list of auto-opened folders.
	//
	m_autoOpenFolderList.RemoveAll();

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMboxTreeCtrl::OnDragOver(COleDataObject* pDataObject,
									 DWORD dwKeyState, 
									 CPoint point)
{
	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_TRANSFER))
	{
		//
		// Let's do a hit test to see if we're over a valid 
		// mailbox target.
		//
		UINT flags = TVHT_ONITEM | TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT;
		HTREEITEM h_target = HitTest(point, &flags);
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
					// Hit folder item, so select it using bogus "link" cursor
					// to give user visual feedback regarding "auto-open"
					// capability.
					//
					SelectDropTarget(h_target);
					return DROPEFFECT_LINK;
				}
				break;
#ifdef IMAP4
			case ITEM_IMAP_MAILBOX:
			{
				QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;
				if (!pImapCommand)
				{
					ASSERT (0);
					break;
				}
				// Fall through if selectable:
				else if ( pImapCommand->CanContainMessages() )
				{			
					; // Fall through.
				}
				// If not selectable but can have children, treat as "FOLDER".
				else if ( pImapCommand->CanHaveChildren() )
				{
					SelectDropTarget(h_target);
					return DROPEFFECT_LINK;
				}
				else
				{
					break;
				}
			}
			// Otherwise, fall through.
#endif	// IMAP4
			case ITEM_IN_MBOX:
			case ITEM_OUT_MBOX:
			case ITEM_TRASH_MBOX:
			case ITEM_USER_MBOX:
				SelectDropTarget(h_target);
				if ((dwKeyState & MK_CONTROL) || ShiftDown())
					return DROPEFFECT_COPY;
				else
					return DROPEFFECT_MOVE;
				break;
#ifdef IMAP4
			case ITEM_IMAP_ACCOUNT:
			case ITEM_IMAP_NAMESPACE:
				SelectDropTarget(h_target);
				return DROPEFFECT_LINK;
#endif
			default:
				ASSERT(0);
				break;
			}
		}
	}

	SelectDropTarget(NULL);
	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnDragLeave()
{
	KillAutoScrollTimer();
	
	// turn off the drop target
	::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_DROPHILITE, NULL );

	AutoCloseOpenedFolders();
}


////////////////////////////////////////////////////////////////////////
// AutoCloseOpenedFolders [public]
//
// Process the list of auto-opened folders by closing them in reverse
// order.
//
////////////////////////////////////////////////////////////////////////
BOOL CMboxTreeCtrl::AutoCloseOpenedFolders()
{
	TRACE0("CMboxTreeCtrl::AutoCloseOpenedFolders()\n");
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
// OnLButtonDblClk [protected]
//
// Handle mouse double-click notification message by opening the
// selected mailbox item, if any.
//  
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HTREEITEM h_selitem = GetSelectedItem();

#ifdef _DEBUG
	if (0 == nFlags)
	{
		// nFlags should only be zero for "fake" mouse double-click cases
		ASSERT(-1 == point.x && -1 == point.y);
	}
#endif // DEBUG

	if (h_selitem != NULL)
	{
		//
		// Do a hit test to see where the user landed the double click.
		// Only do this if we're handling a bona-fide mouse click.  The
		// pNMHDR is NULL if we're handling an Enter key shortcut.
		//
		HTREEITEM h_hititem = NULL;
		if (nFlags)
		{
			TV_HITTESTINFO tvhit;
			tvhit.flags = 0;
			tvhit.hItem = 0;
			tvhit.pt = point;		// client coordinates
			HitTest(&tvhit);
			if ((tvhit.flags & TVHT_ONITEM) && (tvhit.hItem != NULL))
				h_hititem = tvhit.hItem;
		}

		CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_selitem);
		ASSERT(p_itemdata != NULL);

		switch (p_itemdata->m_itemType)
		{
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
		case ITEM_USER_MBOX:
			if ((0 == nFlags) || (h_hititem == h_selitem))
			{
				ASSERT(p_itemdata->m_pCommand != NULL);
				p_itemdata->m_pCommand->Execute( CA_OPEN );
			}
			else
				QCMailboxTreeCtrl::OnLButtonDblClk(nFlags, point);
			break;
		case ITEM_ROOT:
		case ITEM_FOLDER:
			if (ItemHasChildren(h_selitem))
			{
				if (nFlags)
				{
					//
					// This is a hack.  If there is a non-zero
					// nFlags, then we know that the MFC
					// framework called us to handle a real
					// double-click message.  In this case, let the
					// tree control do its default double-click
					// behavior of expanding/collapsing parent items.
					//
					QCMailboxTreeCtrl::OnLButtonDblClk(nFlags, point);
					break;
				}

				//
				// If we get this far, this is the other half of the
				// hack.  If we get a zero nFlags value, then we know
				// we're faking a double-click via the Enter key or
				// the Open context menu command.  So, fool the
				// control into thinking it gets an expand/collapse
				// command.
				//

				//
				// First, save the item's current expand/collapse state, then
				// change the item's expand/collapse state.
				//
				UINT oldstate = GetItemState(h_selitem, TVIS_EXPANDED);
				SendMessage(TVM_EXPAND, WPARAM(TVE_TOGGLE), LPARAM(h_selitem)); 

				//
				// Okay, the TVM_EXPAND message does not send the corresponding
				// TVN_ITEMEXPANDED notification message like you might
				// expect, so we have to do it ourselves.  Whatta pain.
				//
				NM_TREEVIEW nmtv;
				if (oldstate & TVIS_EXPANDED)
					nmtv.action = TVE_COLLAPSE;
				else
					nmtv.action = TVE_EXPAND;
				nmtv.itemNew.mask = TVIF_HANDLE | TVIF_PARAM;
				nmtv.itemNew.hItem = h_selitem;
				nmtv.itemNew.lParam = GetItemData(h_selitem);
				LRESULT unused;
				OnItemExpanded((NMHDR* ) &nmtv, &unused);
			}
			else
			{
				//
				// User wants to open or close an empty folder.  This
				// case requires special handling since the default behavior
				// of the tree control is to sit there and do nothing.
				//
				int image_idx = -1;
				int unused;
				GetItemImage(h_selitem, image_idx, unused);
				switch (image_idx)
				{
				case QCMailboxImageList::IMAGE_OPEN_FOLDER:
					SetItemImage(h_selitem, QCMailboxImageList::IMAGE_CLOSED_FOLDER, QCMailboxImageList::IMAGE_CLOSED_FOLDER);
					break;
				case QCMailboxImageList::IMAGE_CLOSED_FOLDER:
					SetItemImage(h_selitem, QCMailboxImageList::IMAGE_OPEN_FOLDER, QCMailboxImageList::IMAGE_OPEN_FOLDER);
					break;
				default:
					ASSERT(0);
					break;
				}
			}
			break;

#ifdef IMAP4
		case ITEM_IMAP_MAILBOX:
			if ((0 == nFlags) || (h_hititem == h_selitem))
			{
				// Handling an IMAP folder is more complicated. See "imaptree.cpp"...
				ImapHandleDblclkFolder (h_selitem, nFlags, point, p_itemdata);
			}
			break;

		case ITEM_IMAP_NAMESPACE:
			// Not handled yet.
			break;

		case ITEM_IMAP_ACCOUNT:
			if ((0 == nFlags) || (h_hititem == h_selitem))
			{
				// Handling an IMAP folder is more complicated. See "imaptree.cpp"...
				ImapHandleDblclkAccount (h_selitem, nFlags, point, p_itemdata);
			}
			break;
#endif // IMAP4

		default:
			ASSERT(0);
			break;
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnBeginLabelEdit [protected]
//
// Handles CTreeCtrl notification that the label is about to be edited.
// We do not allow "persistent" nodes such as the In, Out, and Trash
// mailboxes to be edited.
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* p_dispinfo = (TV_DISPINFO * )pNMHDR;	// type cast
	ASSERT(p_dispinfo);

	CMboxTreeItemData* p_dataitem = (CMboxTreeItemData *) (p_dispinfo->item.lParam);
	ASSERT(p_dataitem != NULL);
	switch (p_dataitem->m_itemType)
	{
	case ITEM_ROOT:
	case ITEM_IN_MBOX:
	case ITEM_OUT_MBOX:
	case ITEM_TRASH_MBOX:
		::MessageBeep(MB_OK);
		*pResult = 1;		// cancel editing
		break;
	case ITEM_FOLDER:
	case ITEM_USER_MBOX:
#ifdef IMAP4 // IMAP4
	case ITEM_IMAP_MAILBOX:
#endif // END IMAP4
		{
			//
			// Workaround known bug with Tree Controls used in dialogs ...
			// must temporarily subclass edit control to intercept the 
			// Enter and Esc keystrokes to commit/cancel label editing.
			//
			if (IsWinNT())
			{
				//
				// This workaround causes crashes under Windows NT 3.x
				// and Windows NT 4.x beta, so don't do it.  The
				// bummer is that Enter/Esc won't work under NT until
				// 1) Microsoft fixes the tree control for NT 4.x, or
				// 2) we figure out another workaround for NT.
				//
				ASSERT(NULL == s_pOldEditProc);
				ASSERT(NULL == s_pTreeCtrl);
			}
			else
			{
				HWND h_editctrl = HWND(SendMessage(TVM_GETEDITCONTROL));
				ASSERT(h_editctrl != NULL);
				ASSERT(NULL == s_pOldEditProc);
				s_pOldEditProc = WNDPROC(SetWindowLong(h_editctrl, GWL_WNDPROC, long(TreeEditLabelProc)));
				ASSERT(s_pOldEditProc != NULL);
				ASSERT(NULL == s_pTreeCtrl);
				s_pTreeCtrl = this;
			}
			*pResult = 0;		// allow editing
		}
		break;

#ifdef IMAP4
	case ITEM_IMAP_ACCOUNT:
	case ITEM_IMAP_NAMESPACE:
		// Bug - Can't rename these.
		::MessageBeep(MB_OK);
		*pResult = 1;		// cancel editing
		break;
#endif // IMAP4

	default:
		ASSERT(0);
		*pResult = 0;
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnEndLabelEdit [protected]
//
// Handles CTreeCtrl notification that the label editing is complete.
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
    TV_DISPINFO* p_dispinfo = (TV_DISPINFO * )pNMHDR;   // type cast
    ASSERT(p_dispinfo);

    CString newname;
    BOOL is_newname_ok = TRUE;          // innocent until proven guilty

    CMboxTreeItemData* p_dataitem = (CMboxTreeItemData *) (p_dispinfo->item.lParam);
    ASSERT(p_dataitem != NULL);
    switch (p_dataitem->m_itemType)
    {
    case ITEM_ROOT:
    case ITEM_IN_MBOX:
    case ITEM_OUT_MBOX:
    case ITEM_TRASH_MBOX:
        ASSERT(0);
        break;
    case ITEM_FOLDER:
    case ITEM_USER_MBOX:
#ifdef IMAP4
    case ITEM_IMAP_MAILBOX:
#endif
    {
        if (p_dispinfo->item.pszText)
        {
            newname = p_dispinfo->item.pszText;

            if (newname.IsEmpty())
            {
                ::MessageBeep(MB_OK);
                is_newname_ok = FALSE;
            }
            else if (newname.GetLength() >= 32)
            {
                ErrorDialog(IDS_ERR_MAILBOX_NAME_TOO_LONG);
                is_newname_ok = FALSE;
            }
            else
            {
                CString oldname = GetItemText(p_dispinfo->item.hItem);

                // Treat same name just like cancel
                if (newname != oldname)
                {
                    // Commas and &s are a no-no
                    if (newname.FindOneOf(",&\\") >= 0)
                    {
                        ErrorDialog(IDS_MAILBOXES_BAD_CHARACTER);
                        is_newname_ok = FALSE;
                    }

#ifdef IMAP4    // don't allow the account's hierarchy delimiter.
                    else if ( (p_dataitem->m_itemType == ITEM_IMAP_MAILBOX) && 
                              ( !IsValidImapName (p_dataitem->m_pCommand, newname) ) )
                    {
                        is_newname_ok = FALSE;
                    }
#endif                  // IMAP4

                    // You can rename to the same name with different capitalization
                    else if (newname.CompareNoCase(oldname) && SiblingHasSameName(p_dispinfo->item.hItem, newname))
                    {
                        ErrorDialog(IDS_ERR_MAILBOX_EXISTS, (const char*) newname);
                        is_newname_ok = FALSE;
                    }
                    else
                    {
                        //
                        // The moment we have all been waiting for...
                        // do the rename and rebuild the user menus.
                        //
                        CMboxTreeItemData* p_itemdata = (CMboxTreeItemData  *) GetItemData(p_dispinfo->item.hItem);
                        ASSERT(p_itemdata != NULL);
                        ASSERT(p_itemdata->m_pCommand != NULL);

//FORNOW                                                        //
//FORNOW                                                        // Okay, we pass our 'this' pointer to the global
//FORNOW                                                        // rename routine so that it can tell us what the
//FORNOW                                                        // NEW pathname to the file is going to be.  Since
//FORNOW                                                        // the re-selection logic works strictly off of pathnames,
//FORNOW                                                        // the default behavior of storing the OLD pathname won't 
//FORNOW                                                        // work since the newly-built tree will already contain 
//FORNOW                                                        // the NEW pathname.
//FORNOW                                                        //
//FORNOW                                                        ASSERT(m_selectedItemFilename.IsEmpty());
                        p_itemdata->m_pCommand->Execute( CA_RENAME, ( void* ) ( LPCSTR ) newname );
                    }
                }
            }
        }

        //
        // Workaround known bug with Tree Controls used in dialogs ...
        // Restore the temporarily subclassed edit control.
        //
        if (IsWinNT())
        {
                                //
                                // This workaround causes crashes under Windows NT 3.x
                                // and Windows NT 4.x beta, so don't do it.
                                //
            ASSERT(NULL == s_pOldEditProc);
            ASSERT(NULL == s_pTreeCtrl);
        }
        else
        {
            HWND h_editctrl = HWND(SendMessage(TVM_GETEDITCONTROL));
            ASSERT(h_editctrl != NULL);
            ASSERT(s_pOldEditProc != NULL);
            SetWindowLong(h_editctrl, GWL_WNDPROC, long(s_pOldEditProc));
            s_pOldEditProc = NULL;
            ASSERT(s_pTreeCtrl != NULL);
            s_pTreeCtrl = NULL;
        }
    }
    break;

#ifdef IMAP4
    case ITEM_IMAP_ACCOUNT:
    case ITEM_IMAP_NAMESPACE:

        // Can't rename these.
        break;
#endif // IMAP4
 
    default:
        ASSERT(0);
        break;
    }

    if ( !is_newname_ok ) {

        // BOG: Ok, here's my hack for the "overly friendly dialog" problem. If the focus has
        // changed, it means the user is ending an editing session by switching windows. There
        // are a lot of weird interactions going on here (MDI vs. Wazoo vs. Common Control vs.
        // MFC CmdUI stuff), and if we can't subclass the edit control without crashing Eudora,
        // then we have no options and are just gonna have to punt. By not executing the code
        // below, the mailbox name simply reverts back to the original, and whatever window
        // they just clicked on gets the focus.

        if ( GetFocus() == this ) {

            // keep the tree control in edit mode, using the edited name
            SendMessage(TVM_EDITLABEL, 0, LPARAM(p_dispinfo->item.hItem));
            CEdit* p_edit = GetEditControl();

            if ( p_edit ) {
                p_edit->SetWindowText(newname);
                p_edit->SetSel(0, -1);
            }
            else
                ASSERT(0);
        }
    }

    *pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// OnRightButtonDown [protected]
//
////////////////////////////////////////////////////////////////////////
long CMboxTreeCtrl::OnRightButtonDown(WPARAM wParam, LPARAM lParam)
{
	//
	// Kill any drags in progress, then keep going.
	//
	OnEscKeyPressed();

	//
	// Cancel any label edits in progress, then keep going.
	// If we don't do this here, then the SelectItem() call
	// below will GPF.  We could *accept*, rather than cancel,
	// any edits in progress, but this gets messy when the
	// edited name is illegal since it generates multiple 
	// (identical) error messages for the illegal name.
 	//
	if (GetEditControl())
		SendMessage(TVM_ENDEDITLABELNOW, TRUE, 0);

	CPoint client_pt(LOWORD(lParam), HIWORD(lParam));

	//
	// Do a hit test to see where the user landed the right click.
	//
	TV_HITTESTINFO tvhit;
	tvhit.pt = client_pt;
    tvhit.flags = 0;
    tvhit.hItem = 0;

	HitTest(&tvhit);

#ifdef FORNOW_DEBUG
	CString msg;
	static unsigned count = 0;
	msg.Format("CMboxTreeCtrl::OnRButtonDown(%d, %d), hItem=%d", client_pt.x, client_pt.y, tvhit.hItem);
	if (tvhit.flags & TVHT_ABOVE)
		msg += ", ABOVE";
	if (tvhit.flags & TVHT_BELOW)
		msg += ", BELOW";
	if (tvhit.flags & TVHT_NOWHERE)
		msg += ", NOWHERE";
	if (tvhit.flags & TVHT_ONITEM)
		msg += ", ONITEM";
	if (tvhit.flags & TVHT_ONITEMBUTTON)
		msg += ", ONITEMBUTTON";
	if (tvhit.flags & TVHT_ONITEMICON)
		msg += ", ONITEMICON";
	if (tvhit.flags & TVHT_ONITEMINDENT)
		msg += ", ONITEMINDENT";
	if (tvhit.flags & TVHT_ONITEMLABEL)
		msg += ", ONITEMLABEL";
	if (tvhit.flags & TVHT_ONITEMRIGHT)
		msg += ", ONITEMRIGHT";
	if (tvhit.flags & TVHT_ONITEMSTATEICON)
		msg += ", ONITEMSTATEICON";
	if (tvhit.flags & TVHT_TOLEFT)
		msg += ", TOLEFT";
	if (tvhit.flags & TVHT_TORIGHT)
		msg += ", TORIGHT";
	msg += ")\n";
	TRACE0((const char *) msg);
#endif //FORNOW_DEBUG

	SetFocus();
	if ((tvhit.flags & TVHT_ONITEM) && (tvhit.hItem != NULL))
	{
		SelectItem(tvhit.hItem);
	}
	else
		SelectItem(0);

	if (GetCapture() != this)
		SetCapture();
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnBeginDrag [protected]
//
// Handles CTreeCtrl notification that the user has started dragging
// a tree item. 
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* p_nmtv = (NM_TREEVIEW * )pNMHDR;	// type cast
	ASSERT(p_nmtv);

	HTREEITEM h_item = p_nmtv->itemNew.hItem;
	ASSERT(h_item != NULL);
	CMboxTreeItemData* p_dataitem = (CMboxTreeItemData *) (p_nmtv->itemNew.lParam);
	ASSERT(p_dataitem != NULL);
	Select(h_item, TVGN_CARET);
	SelectItem(NULL);	// clear the selection, if any
	UpdateWindow();
	switch (p_dataitem->m_itemType)
	{
	case ITEM_ROOT:
	case ITEM_IN_MBOX:
	case ITEM_OUT_MBOX:
	case ITEM_TRASH_MBOX:
	case ITEM_FOLDER:
		::MessageBeep(MB_OK);
		break;
	case ITEM_USER_MBOX:
#ifdef IMAP4
	case ITEM_IMAP_MAILBOX:
#endif // IMAP4
		{
#ifdef IMAP4
			// For IMAP, determine if this object is dragable:
			if (p_dataitem->m_itemType == ITEM_IMAP_MAILBOX)
			{
				if (! ImapMboxIsDragable (p_dataitem->m_pCommand, p_dataitem->m_itemType) )
				{
					::MessageBeep(MB_OK);
					break;
				}
			}
#endif // IMAP4					

			//
			// Setup drag image.
			//
			ASSERT(NULL == m_pDragImage);
			ASSERT(NULL == m_hDragMbox);
			if ((m_pDragImage = CreateDragImage(h_item)) != NULL)
			{
				SetFocus();			// grab keyboard input focus for Esc key
				::SetCursor(::LoadCursor(0, IDC_ARROW));
				VERIFY(m_pDragImage->SetDragCursorImage(0, CPoint(0, 0)));
				m_pDragImage->BeginDrag(0, CPoint(0, 0));
				m_pDragImage->DragMove(p_nmtv->ptDrag);
				m_pDragImage->DragEnter(this, p_nmtv->ptDrag);

				if (GetCapture() != this)
					SetCapture();
				m_hDragMbox = h_item;

				StartAutoScrollTimer();
			}
		}
		break;
#ifdef IMAP4
	case ITEM_IMAP_ACCOUNT:
	case ITEM_IMAP_NAMESPACE:
		// Bug - Not yet handled.
		break;
#endif // IMAP4
	default:
		ASSERT(0);
		break;
	}

	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// OnMouseMove [protected]
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_pDragImage && (MK_LBUTTON & nFlags))
	{
		ASSERT(this == GetCapture());
		ASSERT(m_hDragMbox != NULL);
		VERIFY(m_pDragImage->DragMove(point));

		UINT flags = 0;
		HTREEITEM h_target = HitTest(point, &flags);
		m_pDragImage->DragLeave(this);			// allow window updates
		if (h_target != NULL)
		{
#ifdef IMAP4
			// If the drag source is an IMAP object, it's easier just to
			// handle it in a separate method:
			//
			if (m_hDragMbox != NULL)
			{
				CMboxTreeItemData* pSourceItemData = (CMboxTreeItemData *) GetItemData(m_hDragMbox);
				if (  pSourceItemData && 
					 (pSourceItemData->m_itemType == ITEM_IMAP_ACCOUNT ||
					  pSourceItemData->m_itemType == ITEM_IMAP_MAILBOX ||
					  pSourceItemData->m_itemType == ITEM_IMAP_NAMESPACE) )
				{
					HandleImapMouseMove (nFlags, point, h_target);
					return;
				}
			}
#endif // IMAP4

			CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_target);
			ASSERT(p_itemdata != NULL);
			switch (p_itemdata->m_itemType)
			{
			case ITEM_ROOT:
			case ITEM_FOLDER:
				//
				// Highlight the drop target directly if it is a folder.
				//
				if (GetParentItem(m_hDragMbox) == h_target)
				{
					::SetCursor(::LoadCursor(0, IDC_NO));
					SelectDropTarget(NULL);
				}
				else
				{
					::SetCursor(::LoadCursor(0, IDC_ARROW));
					SelectDropTarget(h_target);
				}
				break;
			case ITEM_IN_MBOX:
			case ITEM_OUT_MBOX:
			case ITEM_TRASH_MBOX:
			case ITEM_USER_MBOX:
				{
					//
					// If the drop target is a mailbox, highlight the
					// parent folder, if it's not its own parent.
					//
					HTREEITEM h_targetparent = GetParentItem(h_target);
					ASSERT(h_targetparent != NULL);

					if (GetParentItem(m_hDragMbox) != h_targetparent)
					{
						::SetCursor(::LoadCursor(0, IDC_ARROW));
						SelectDropTarget(h_targetparent);
					}
					else
					{
						::SetCursor(::LoadCursor(0, IDC_NO));
						SelectDropTarget(NULL);
					}
				}
				break;
#ifdef IMAP4
			case ITEM_IMAP_ACCOUNT:
			case ITEM_IMAP_MAILBOX:
			case ITEM_IMAP_NAMESPACE:
				// Don't allow these:
				::SetCursor(::LoadCursor(0, IDC_NO));
				SelectDropTarget(NULL);
				break;
#endif // IMAP4

			default:
				ASSERT(0);
				::SetCursor(::LoadCursor(0, IDC_NO));
				SelectDropTarget(NULL);
				break;
			}
		}
		else
		{
			::SetCursor(::LoadCursor(0, IDC_NO));
			SelectDropTarget(NULL);
		}
		m_pDragImage->DragEnter(this, point);	// lock window updates
	}

	QCMailboxTreeCtrl::OnMouseMove(nFlags, point);
}




#ifdef IMAP4 //

// HandleImapMouseMove [protected]
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::HandleImapMouseMove(UINT nFlags, CPoint point, HTREEITEM h_target)
{
	// No need to check these because it's been done above, but I'm paranoid.
	//
	if (m_hDragMbox == NULL)
	{
		ASSERT (0);
		::SetCursor(::LoadCursor(0, IDC_NO));
		SelectDropTarget(NULL);

		return;
	}

	// Verify that the source object is an IMAP object!!
	//
	CMboxTreeItemData* pSourceItemData = (CMboxTreeItemData *) GetItemData(m_hDragMbox);
	if (!pSourceItemData)
	{
		ASSERT (0);
		::SetCursor(::LoadCursor(0, IDC_NO));
		SelectDropTarget(NULL);

		return;
	}

	if (! (pSourceItemData->m_itemType == ITEM_IMAP_ACCOUNT ||
		   pSourceItemData->m_itemType == ITEM_IMAP_MAILBOX ||
		   pSourceItemData->m_itemType == ITEM_IMAP_NAMESPACE) )
	{
		::SetCursor(::LoadCursor(0, IDC_NO));
		SelectDropTarget(NULL);

		return;
	}

	if (h_target != NULL)
	{
		CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_target);
		if (!p_itemdata)
		{
			ASSERT(0);

			::SetCursor(::LoadCursor(0, IDC_NO));
			SelectDropTarget(NULL);

			return;
		}

		switch (p_itemdata->m_itemType)
		{
			// Don't allow a drop to a local (POP) mailbox.
			case ITEM_ROOT:
			case ITEM_FOLDER:
			case ITEM_IN_MBOX:
			case ITEM_OUT_MBOX:
			case ITEM_TRASH_MBOX:
			case ITEM_USER_MBOX:
				::SetCursor(::LoadCursor(0, IDC_NO));
				SelectDropTarget(NULL);

				break;

			case ITEM_IMAP_ACCOUNT:
			case ITEM_IMAP_MAILBOX:
				// If the target and source are the same, don't allow:
				//
				if (m_hDragMbox == h_target)
				{
					::SetCursor(::LoadCursor(0, IDC_NO));
					SelectDropTarget(NULL);
				}
				// If the target is the source's parent, don't allow.
				//
				else if (GetParentItem(m_hDragMbox) == h_target)
				{
					::SetCursor(::LoadCursor(0, IDC_NO));
					SelectDropTarget(NULL);
				}
				else
				{
					// Cannot allow a mailbox to be dropped onto one of it's 
					// descendents:
					//
					BOOL		bTargetIsDescendent = FALSE;
					HTREEITEM	hParent = GetParentItem(h_target);

					while (hParent)
					{
						if (hParent == m_hDragMbox)
						{
							bTargetIsDescendent = TRUE;
							break;
						}

						hParent = GetParentItem(hParent);
					}

					if (bTargetIsDescendent)
					{
						::SetCursor(::LoadCursor(0, IDC_NO));
						SelectDropTarget(NULL);
					}
					// The source and target objects must be within the same IMAP account.
					// However, the target CANNOT be the immediate parent tree item of
					// the source.
					// Also, if the destination is of type ITEM_IMAP_MAILBOX, it must
					// NOT have the \NOINFERIORS flag.
					//
					else if ( ImapTargetIsDroppable (pSourceItemData->m_pCommand,
											 pSourceItemData->m_itemType,
											 p_itemdata->m_pCommand,
					 						 p_itemdata->m_itemType) )
					{
						::SetCursor(::LoadCursor(0, IDC_ARROW));
						SelectDropTarget(h_target);
					}
					else
					{
						::SetCursor(::LoadCursor(0, IDC_NO));
						SelectDropTarget(NULL);
					}
				}
	
				break;

			case ITEM_IMAP_NAMESPACE:
				// Not yet handled.
				::SetCursor(::LoadCursor(0, IDC_NO));
				SelectDropTarget(NULL);
				break;

			default:
				ASSERT(0);
				::SetCursor(::LoadCursor(0, IDC_NO));
				SelectDropTarget(NULL);
				break;
		} // switch
	} // if
	else
	{
		::SetCursor(::LoadCursor(0, IDC_NO));
		SelectDropTarget(NULL);
	}

	m_pDragImage->DragEnter(this, point);	// lock window updates

	QCMailboxTreeCtrl::OnMouseMove(nFlags, point);
}

#endif // IMAP4

  

////////////////////////////////////////////////////////////////////////
// OnLButtonUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_pDragImage)
	{
		HTREEITEM h_target = GetDropHilightItem();

		m_pDragImage->DragLeave(this);
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage = NULL;

		if (GetCapture() == this)
			ReleaseCapture();

		KillAutoScrollTimer();

		SelectDropTarget(NULL);

		//
		// Perform mailbox move step.
		//
		ASSERT(m_hDragMbox != NULL);
		if (h_target != NULL)
		{
			CMboxTreeItemData* p_dragitem = (CMboxTreeItemData * ) GetItemData(m_hDragMbox);
			ASSERT(p_dragitem != NULL);

#ifdef IMAP4
			ASSERT(p_dragitem->m_itemType == ITEM_USER_MBOX ||
				   p_dragitem->m_itemType == ITEM_IMAP_MAILBOX );
#else
			ASSERT(p_dragitem->m_itemType == ITEM_USER_MBOX);
#endif // IMAP4

			ASSERT(p_dragitem->m_pCommand != NULL);

			CMboxTreeItemData* p_targetitem = (CMboxTreeItemData * ) GetItemData(h_target);
			ASSERT(p_targetitem != NULL);

			if (ITEM_FOLDER == p_targetitem->m_itemType)
			{
				p_dragitem->m_pCommand->Execute( CA_GRAFT, p_targetitem->m_pCommand );
			}
			else if (ITEM_ROOT == p_targetitem->m_itemType)
			{
				ASSERT(NULL == p_targetitem->m_pCommand);
				p_dragitem->m_pCommand->Execute( CA_GRAFT, NULL );
			}
#ifdef IMAP4 // These cases go to the CImapMailboxCommand derived object.
			else if (ITEM_IMAP_ACCOUNT == p_targetitem->m_itemType || 
				     ITEM_IMAP_MAILBOX == p_targetitem->m_itemType)
			{
				ASSERT(NULL != p_targetitem->m_pCommand);
				p_dragitem->m_pCommand->Execute( CA_IMAP_GRAFT, p_targetitem->m_pCommand );
			}
#endif // IMAP4
		}

		m_hDragMbox = NULL;

		AutoCloseOpenedFolders();
	}

	::SetCursor(::LoadCursor(0, IDC_ARROW));

	QCMailboxTreeCtrl::OnLButtonUp(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// DoSomethingWhileUserPausedMouseAtPoint [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::DoSomethingWhileUserPausedMouseAtPoint(CPoint pt)
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
#ifdef IMAP4
		case ITEM_IMAP_MAILBOX:
		{
			// If this has children, treat as a ITEM_FOLDER"
			//
			QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;
			if (! (pImapCommand && pImapCommand->CanHaveChildren()) )
			{			
				break;
			}
		}
		// Fall through otherwise.
		case ITEM_IMAP_ACCOUNT:
#endif
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
				if (m_pDragImage != NULL)
					break;

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

#ifdef IMAP4
	// These not yet implemented.
		case ITEM_IMAP_NAMESPACE:
			break;
#endif // IMAP4

		default:
			ASSERT(0);
			break;
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnKeyUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DELETE:
		// The Del key is a shortcut for the Delete command.
		OnCmdDelete();
		break;
	case VK_F2:
		{
			//
			// Shortcut for the rename command.
			//
			HTREEITEM h_selitem = GetSelectedItem();
			if (h_selitem != NULL)
				PostMessage(TVM_EDITLABEL, 0, LPARAM(h_selitem)); 
			QCMailboxTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		}
		break;
	default:
		QCMailboxTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected, virtual]
//
// Called when the user releases the right button in the tree control.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	if (GetCapture() == this)
		ReleaseCapture();

	//
	// If we're in the middle of editing a label, don't bother with
	// the context menu.
	//
	if (GetEditControl())
		return;

  	// Get the menu that contains all the CMboxTree context popups
	HTREEITEM h_selitem = GetSelectedItem();
	if (h_selitem != NULL)
	{
		CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_selitem);
		ASSERT(p_itemdata != NULL);

		HMENU hMenu = NULL;

		if ((ITEM_ROOT == p_itemdata->m_itemType) || (ITEM_FOLDER == p_itemdata->m_itemType))
		{
			hMenu = QCLoadMenu(IDR_MAILBOXES_FOLDER);
		}
		else
		{
			hMenu = QCLoadMenu(IDR_MAILBOXES);
		}

		if (hMenu)
		{
			//
			// Note that we edit the context menu on-the-fly in order to
			// stick in the "Empty Trash" item when the user selects
			// the Trash mailbox.
			//
			if (hMenu && (ITEM_TRASH_MBOX == p_itemdata->m_itemType))
				::InsertMenu(hMenu, 1, MF_BYPOSITION, ID_MBOXTREE_EMPTY_TRASH, CRString(IDS_MBOXTREE_EMPTY_TRASH));
			::SetMenuDefaultItem(hMenu, 0, TRUE);

#ifdef IMAP4	
			// Append the IMAP menu items only to imap objects.
			if (p_itemdata && (ITEM_IMAP_ACCOUNT   == p_itemdata->m_itemType ||
							   ITEM_IMAP_NAMESPACE == p_itemdata->m_itemType ||
							   ITEM_IMAP_MAILBOX   == p_itemdata->m_itemType)  )
			{
				// This is over in "imaptree.cpp" which is #included below!!
				void AddImapMenus (HMENU hMenu);

				AddImapMenus (hMenu);
			}
#endif // IMAP4

			//
			// Get parent Wazoo Window.
			//
			CWazooWnd* p_wndWazoo = GetParentWazooWnd();
			if (p_wndWazoo)
			{
				CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
				p_wndWazoo->DoContextMenu(this, ptScreen, hMenu);
				return;
			}
		}
		ASSERT(0);
	}

	QCMailboxTreeCtrl::OnContextMenu(pWnd, ptScreen);
}


////////////////////////////////////////////////////////////////////////
// OnCmdOpen [public]
//
// Context menu command handler for the Open command.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdOpen(void)
{
	//
	// Execute "open" item by simulating mouse double-click on 
	// selected item.
	//
	OnLButtonDblClk(0, CPoint(-1, -1));
}


// OnCmdFindMsgs [public]
//
// Context menu command handler for the "Find Messages..." command.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdFindMsgs(void)
{
	// Open the find msg window
	CMainFrame::QCGetMainFrame()->SendMessage(WM_COMMAND, ID_EDIT_FIND_FINDMSG);

	// Now the global doc should be avail
	ASSERT(gSearchDoc);
	if (!gSearchDoc)
		return;

	// Get selected item
	HTREEITEM h_item = GetSelectedItem();
	ASSERT(h_item);
	if (NULL == h_item)
		return;

	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_item);
	ASSERT(p_itemdata);
	if (NULL == p_itemdata)
		return;

	// Get the SearchView
	POSITION pos = gSearchDoc->GetFirstViewPosition();
	CSearchView *pSearchView = (CSearchView *) gSearchDoc->GetNextView(pos);
	ASSERT(pSearchView);
	if (NULL == pSearchView)
		return;

	// Select the appropriate mbx
	// Needs to be posted for the item to be scrolled in to view correctly
	pSearchView->PostMessage(msgFindMsgMaiboxSel, (WPARAM) (p_itemdata->m_pCommand));
}


////////////////////////////////////////////////////////////////////////
// OnCmdNew [public]
//
// Context menu command handler for the New command.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdNew(void)
{
	HTREEITEM h_item = GetSelectedItem();
	if (NULL == h_item)
		return;

	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_item);
	ASSERT(p_itemdata != NULL);

	//
	// Can only create new items underneath "folders".
	//
	switch (p_itemdata->m_itemType)
	{
		case ITEM_ROOT:
			ASSERT( NULL == p_itemdata->m_pCommand );
		break;

		case ITEM_FOLDER:
			ASSERT(p_itemdata->m_pCommand != NULL);
		break;
	
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
		case ITEM_USER_MBOX:
			ASSERT(0);		// should never get here
			return;

#ifdef IMAP4
		// Allow some to pass through.
		case ITEM_IMAP_ACCOUNT:
			break;

		case ITEM_IMAP_MAILBOX:
		case ITEM_IMAP_NAMESPACE:
		{
			// Enable only if the mailbox can have inferiors and is not read-only.
			QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;
			if (pImapCommand && pImapCommand->CanHaveChildren())
			{
				break;
			}

			// Shouldn't get here.
			ASSERT (0);
			return;
		}

#endif // IMAP4
	
		default:
			ASSERT(0);
			return;
	}

	g_theMailboxDirector.CreateTargetMailbox( p_itemdata->m_pCommand, FALSE );
	
	SetFocus();			// grab the keyboard focus back
}


////////////////////////////////////////////////////////////////////////
// OnCmdDelete [public]
//
// Context menu command handler for the Delete command.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdDelete(void)
{
	HTREEITEM h_item = GetSelectedItem();
	if (NULL == h_item)
		return;

	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_item);
	ASSERT(p_itemdata != NULL);

#ifdef IMAP4
	// If this is an IMAP object, pass the buck.
	if (p_itemdata->m_pCommand)
	{
		if ( p_itemdata->m_pCommand->IsImapType() )
		{
			OnCmdImapDelete ();
			return;
		}
	}
#endif // IMAP4

	//
	// Can only delete user-defined items.
	//
	switch (p_itemdata->m_itemType)
	{
		case ITEM_ROOT:
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
			::MessageBeep(MB_OK);		// not allowed to delete these
			return;
		
		case ITEM_FOLDER:
		case ITEM_USER_MBOX:
			ASSERT(p_itemdata->m_pCommand != NULL);
		break;
		
		default:
			ASSERT(0);
			return;
	}

	if ( p_itemdata->m_pCommand->IsEmpty() == FALSE )
	{
		short OldAutoOK = GetIniShort(IDS_INI_AUTO_OK);
		SetIniShort(IDS_INI_AUTO_OK, 0);
		
		int result = AlertDialog( ( ( p_itemdata->m_pCommand->GetType() == MBT_FOLDER ) ? IDD_DELETE_MBOX_FOLDER : IDD_DELETE_MAILBOX), p_itemdata->m_pCommand->GetName() );
		SetIniShort(IDS_INI_AUTO_OK, OldAutoOK);
		if (IDCANCEL == result)
			return;
	}
	
			
	// Actually delete file(s)
	p_itemdata->m_pCommand->Execute( CA_DELETE );	
	
	SetFocus();			// grab keyboard focus back
}


////////////////////////////////////////////////////////////////////////
// OnCmdRename [public]
//
// Context menu command handler for the Rename command.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdRename(void)
{
	HTREEITEM h_selitem = GetSelectedItem();
	if (h_selitem != NULL)
	{
		// Make sure label editing is not already in progress
		if (NULL == GetEditControl())
			EditLabel(h_selitem);
	}
}


////////////////////////////////////////////////////////////////////////
// OnCmdEmptyTrash [public]
//
// Context menu command handler for the Empty Trash command.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdEmptyTrash(void)
{
	EmptyTrash();
}


////////////////////////////////////////////////////////////////////////
// OnCmdUpdateBySelection [protected]
//
// UI handler for enabling/disabling menu items on the context popup
// menu based on the current selection in the tree control.
////////////////////////////////////////////////////////////////////////
void CMboxTreeCtrl::OnCmdUpdateBySelection(CCmdUI* pCmdUI)
{
	HTREEITEM h_item = GetSelectedItem();
	if (NULL == h_item)
	{
		pCmdUI->Enable(FALSE);  // something's wrong
		return;
	}
	CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_item);
	ASSERT(p_itemdata != NULL);

	switch (pCmdUI->m_nID)
	{
	case ID_MBOXTREE_DELETE:
	case ID_MBOXTREE_RENAME:
		//
		// These only apply to user-defined items.
		//
		switch (p_itemdata->m_itemType)
		{
		case ITEM_ROOT:
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
			pCmdUI->Enable(FALSE);
			break;
		case ITEM_FOLDER:
		case ITEM_USER_MBOX:
			pCmdUI->Enable(TRUE);
			break;

#ifdef IMAP4
		case ITEM_IMAP_MAILBOX:
			pCmdUI->Enable(TRUE);
			break;

		// Can't delete an account from the mailbox tree.
		case ITEM_IMAP_ACCOUNT:
		case ITEM_IMAP_NAMESPACE:
			pCmdUI->Enable(FALSE);
			break;

#endif // IMAP4

		default:
			ASSERT(0);
			pCmdUI->Enable(FALSE);
			break;
		}
		break;
	case ID_MBOXTREE_NEW:
		//
		// These only apply to "folder" items.
		//
		switch (p_itemdata->m_itemType)
		{
		case ITEM_ROOT:
		case ITEM_FOLDER:
			pCmdUI->Enable(TRUE);
			break;
		case ITEM_IN_MBOX:
		case ITEM_OUT_MBOX:
		case ITEM_TRASH_MBOX:
		case ITEM_USER_MBOX:
			pCmdUI->Enable(FALSE);
			break;

#ifdef IMAP4
		// Create mailbox.
		case ITEM_IMAP_ACCOUNT:
			pCmdUI->Enable(TRUE);
			break;

		case ITEM_IMAP_MAILBOX:
		case ITEM_IMAP_NAMESPACE:
		{
			// Enable only if the mailbox can have inferiors and is not read-only.
			QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;
			if (pImapCommand && pImapCommand->CanHaveChildren())
			{
				pCmdUI->Enable(TRUE);
			}
			else
			{
				pCmdUI->Enable(FALSE);
			}
			break;
		}
#endif // IMAP4

		default:
			ASSERT(0);
			pCmdUI->Enable(FALSE);
			break;
		}
		break;

#ifdef IMAP4
	case ID_MBOXTREE_IMAP_REFRESH:
		//
		// These only apply to "account" items for now.
		//
		switch (p_itemdata->m_itemType)
		{
			case ITEM_IMAP_ACCOUNT:
				pCmdUI->Enable(TRUE);
				break;

			case ITEM_IMAP_MAILBOX:
				// Enable if this can have child mailboxes.
				if( p_itemdata->m_pCommand && p_itemdata->m_pCommand->CanHaveChildren () )
					pCmdUI->Enable(TRUE);
				else
					pCmdUI->Enable(FALSE);
				break;

			default:
				pCmdUI->Enable(FALSE);
				break;
		}
		break;

	case ID_MBOXTREE_IMAP_RESYNC:
		//
		// These apply to IMAP items only.
		//
		switch (p_itemdata->m_itemType)
		{
			// Only of mailboxes, and if selectable.
			case ITEM_IMAP_MAILBOX:
				{
					QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)p_itemdata->m_pCommand;
					if (pImapCommand && !pImapCommand->IsNoSelect())
					{
						pCmdUI->Enable(TRUE);
						break;
					}
				}

				// Anything else:
				pCmdUI->Enable(FALSE);

				break;
			default:
				pCmdUI->Enable(FALSE);
				break;
		}
		break;

#endif // IMAP4

	default:
		ASSERT(0);
		break;
	}
}




HTREEITEM	CMboxTreeCtrl::FindPathname(
HTREEITEM	hItem,
LPCSTR		szPathname )
{
	CMboxTreeItemData*	pItemData;

	if( hItem == NULL )
	{
		return NULL;
	}
	
	pItemData = (CMboxTreeItemData *) GetItemData( hItem );
	
	if( pItemData == NULL ) 
	{
		return NULL;
	}

	if( pItemData->m_pCommand == NULL )
	{
#ifdef IMAP4
		// Need to do this so sibling IMAP trees would be searched as well.
		// we're in the local root -- search the children
		HTREEITEM hRes = FindPathname( GetChildItem( hItem ), szPathname );
		if (hRes)
			return hRes;
		else
		{
			// Search the IMAP mailboxes.
			return FindPathname( GetNextSiblingItem( hItem ), szPathname );
		}
#else // Original

		// we're in the root -- search the children
		return FindPathname( GetChildItem( hItem ), szPathname );
#endif // IMAP4
	}

	if( stricmp( szPathname, pItemData->m_pCommand->GetPathname() ) == 0 )
	{
		return hItem;
	}

#ifndef IMAP4  // Original
	if( strnicmp( szPathname, pItemData->m_pCommand->GetPathname(), strlen( pItemData->m_pCommand->GetPathname() ) ) == 0 )
	{
		// its in this subtree
		return FindPathname( GetChildItem( hItem ), szPathname );
	}
#else // Do IMAP differently.
	CString szParentPath = pItemData->m_pCommand->GetPathname();

	// This fixes a bug that arises if we have mailbox names like mbox and mbox0!
	if( ( strlen (szPathname) > strlen (szParentPath) ) &&
		( strnicmp( szPathname, szParentPath, strlen( szParentPath ) ) == 0 ) &&
		( szPathname [szParentPath.GetLength()] == '\\' ) )
	{
		// its in this subtree
		return FindPathname( GetChildItem( hItem ), szPathname );
	}
	else if ( pItemData->m_itemType == ITEM_IMAP_MAILBOX )
	{
		// Need to modify this to account for the fact that pCurrent->GetPathname()
		// may have a .mbx extension but the mailbox may still contain child mailboxes.
		CString CurMailboxPath = pItemData->m_pCommand->GetPathname();
		MbxFilePathToMailboxDir (pItemData->m_pCommand->GetPathname(), CurMailboxPath);
		
		if( ( (size_t) CurMailboxPath.GetLength() < strlen (szPathname) ) &&
			( strnicmp( CurMailboxPath, szPathname, CurMailboxPath.GetLength() )  == 0 ) &&
			( szPathname [CurMailboxPath.GetLength()] == '\\' ) )
		{
			// its in this subtree
			return FindPathname( GetChildItem( hItem ), szPathname );
		}
	}
#endif  // IMAP4

	// check sibling items 
	return FindPathname( GetNextSiblingItem( hItem ), szPathname );
}


////////////////////////////////////////////////////////////////////////
// CompareFunc [private, static]
//
// Handles on-the-fly re-sorting necessary after a mailbox or folder 
// has been renamed.
////////////////////////////////////////////////////////////////////////
int CALLBACK CMboxTreeCtrl::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CMboxTreeItemData* pItemData1 = (CMboxTreeItemData *) lParam1;
	CMboxTreeItemData* pItemData2 = (CMboxTreeItemData *) lParam2;

	ASSERT(pItemData1 != pItemData2);

#ifdef IMAP4
	// For IMAP, an account is at the same level as "ITEM_ROOT, the
	// root for "local" (read POP) mailboxes.
	// We can therefore be comparing the ITEM_ROOT with an account.
	// Catch that here and always put ITEM_ROOT first.
	//
	if (pItemData1->m_itemType == ITEM_ROOT)
	{
		return -1;
	}
	else if (pItemData2->m_itemType == ITEM_ROOT)
	{
		return 1;
	}
#endif // IMAP4


#ifndef IMAP4
	ASSERT(pItemData1->m_itemType == ITEM_FOLDER ||
		   pItemData1->m_itemType == ITEM_USER_MBOX ||
		   pItemData1->m_itemType == ITEM_IN_MBOX ||
		   pItemData1->m_itemType == ITEM_OUT_MBOX ||
		   pItemData1->m_itemType == ITEM_TRASH_MBOX);

	ASSERT(pItemData2->m_itemType == ITEM_FOLDER ||
		   pItemData2->m_itemType == ITEM_USER_MBOX ||
		   pItemData2->m_itemType == ITEM_IN_MBOX ||
		   pItemData2->m_itemType == ITEM_OUT_MBOX ||
		   pItemData2->m_itemType == ITEM_TRASH_MBOX);

#else // IMAP4 now:

	ASSERT(pItemData1->m_itemType == ITEM_FOLDER ||
		   pItemData1->m_itemType == ITEM_USER_MBOX ||
		   pItemData1->m_itemType == ITEM_IN_MBOX ||
		   pItemData1->m_itemType == ITEM_OUT_MBOX ||
		   pItemData1->m_itemType == ITEM_TRASH_MBOX  ||
		   pItemData1->m_itemType == ITEM_IMAP_ACCOUNT ||
		   pItemData1->m_itemType == ITEM_IMAP_MAILBOX );

	ASSERT(pItemData2->m_itemType == ITEM_FOLDER ||
		   pItemData2->m_itemType == ITEM_USER_MBOX ||
		   pItemData2->m_itemType == ITEM_IN_MBOX ||
		   pItemData2->m_itemType == ITEM_OUT_MBOX ||
		   pItemData2->m_itemType == ITEM_TRASH_MBOX ||
		   pItemData1->m_itemType == ITEM_IMAP_ACCOUNT ||
		   pItemData1->m_itemType == ITEM_IMAP_MAILBOX );
#endif

	// 
	// Handle the cases by item type...
	//
	if (pItemData1->m_itemType == ITEM_IN_MBOX)
	{
		return -1;
	}
	else if (pItemData2->m_itemType == ITEM_IN_MBOX)
	{
		return 1;
	}
	else if (pItemData1->m_itemType == ITEM_OUT_MBOX &&
			 pItemData2->m_itemType != ITEM_IN_MBOX)
	{
		return -1;
	}
	else if (pItemData2->m_itemType == ITEM_OUT_MBOX &&
			 pItemData1->m_itemType != ITEM_IN_MBOX)
	{
		return 1;
	}
	else if (pItemData1->m_itemType == ITEM_TRASH_MBOX &&
			 (pItemData2->m_itemType != ITEM_IN_MBOX &&
			  pItemData2->m_itemType != ITEM_OUT_MBOX))
	{
		return -1;
	}
	else if (pItemData2->m_itemType == ITEM_TRASH_MBOX &&
			 (pItemData1->m_itemType != ITEM_IN_MBOX &&
			  pItemData1->m_itemType != ITEM_OUT_MBOX))
	{
		return 1;
	}
	else if (pItemData1->m_itemType == ITEM_USER_MBOX &&
			 pItemData2->m_itemType == ITEM_FOLDER)
	{
		return -1;
	}
	else if (pItemData2->m_itemType == ITEM_USER_MBOX &&
			 pItemData1->m_itemType == ITEM_FOLDER)
	{
		return 1;
	}

	//
	// If we get this far, both items should either be a user mailbox
	// or a user folder, or, for IMAP, an account.
	// The types, however, should be the same.
	//

#ifndef IMAP4

	ASSERT(pItemData1->m_itemType == ITEM_USER_MBOX ||
		   pItemData1->m_itemType == ITEM_FOLDER);

#else // IMAP4 now

	ASSERT(pItemData1->m_itemType == ITEM_USER_MBOX ||
		   pItemData1->m_itemType == ITEM_FOLDER ||
		   pItemData1->m_itemType == ITEM_IMAP_ACCOUNT ||
		   pItemData1->m_itemType == ITEM_IMAP_MAILBOX );

	// Must have these now.
	//
	if ( !(pItemData1->m_pCommand && pItemData2->m_pCommand) )
	{
		ASSERT (0);
		return -1;
	}

	// If IMAP mailboxes and one is Inbox, put that first.
	//
	if ( pItemData1->m_itemType == ITEM_IMAP_MAILBOX &&
			IsInbox (pItemData1->m_pCommand->GetName()) )
	{
		return -1;
	}
	else if ( pItemData2->m_itemType == ITEM_IMAP_MAILBOX &&
			IsInbox (pItemData2->m_pCommand->GetName()) )
	{
		return 1;
	}

#endif

	//
	// If we get this far, both items should either be a user mailbox
	// or a user folder, or for IMAP, an account.
	//  Since the types are the same, do a simple string compare.
	//

	ASSERT(pItemData1->m_itemType == pItemData2->m_itemType);

	return ::stricmp(pItemData1->m_pCommand->GetName(), pItemData2->m_pCommand->GetName());
}


void CMboxTreeCtrl::Notify( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction,
void*				pData)
{
	HTREEITEM			hItem;
	HTREEITEM			hDestItem;
	QCMailboxCommand*	pCommand;
	TV_INSERTSTRUCT		tvstruct;
	char				szBuf[256];
	UINT				theState;
	UnreadStatusType	theStatus;

	if( pObject->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) == FALSE )
	{
		return;
	}

	pCommand = ( QCMailboxCommand* ) pObject;

#ifdef IMAP4
	// Handle new IMAP items.
	if ( pCommand->IsImapType() )
	{
		//
		// Note: ImapNotify modified (JOK, 10/6/97) to return FALSE if
		// it did not handle the notification. In that case, fall through
		// to the main notification handler.
		//
		if (ImapNotify (pObject, theAction, pData) == TRUE)
			return;
		// else fall through.
	}
#endif // IMAP4

	if( theAction == CA_NEW )
	{
		if( pCommand->GetType() == MBT_FOLDER )
		{
			AddItem( ITEM_FOLDER, pCommand->GetName(), pCommand, FALSE );
		}
		else
		{
			AddItem( ITEM_USER_MBOX, pCommand->GetName(), pCommand, FALSE );
		}

		//
		// Select the newly-added item.
		//
		HTREEITEM hNewItem = FindPathname( GetRootItem(), pCommand->GetPathname() );
		if (hNewItem)
		{
			SelectItem(hNewItem);
			EnsureVisible(hNewItem);
		}
		return;
	}

	hItem = FindPathname( GetRootItem(), pCommand->GetPathname() );
		
	if( hItem == NULL )
	{
		ASSERT( 0 );
		return;
	}

	//
	// The Rename command for mailboxes and folders is really whacked.
	// We get the CA_RENAME notification here BEFORE the rename of the
	// mailbox or folder actually takes place in the file system.  It has
	// to be done this way since we're looking up the OLD name in the 
	// tree.
	//
	// AFTER the rename has taken place, the contents of the 'pCommand'
	// object has been updated with the NEW mailbox or folder name.
	// Therefore, we rely on the CA_SORT_AFTER_RENAME notification to
	// notify us again and re-sort the view based on the NEW name.
	//
	if( theAction == CA_RENAME ) 
	{
		//
		// Rename - Phase 1.  Just update the tree label UI with the new name.
		//
		SetItemText( hItem, ( LPCSTR ) pData );
		return;
	}

	if( theAction == CA_SORT_AFTER_RENAME ) 
	{
		//
		// Rename - Phase 2.  Now re-sort the tree items based on the
		// new name of the mailbox or folder, using a custom sort function.
		//
		TV_SORTCB sortStruct;
		sortStruct.hParent = GetParentItem(hItem);

#ifndef IMAP4 // Can now have this being NULL:
		ASSERT(sortStruct.hParent);
#endif
		sortStruct.lpfnCompare = CMboxTreeCtrl::CompareFunc;
		sortStruct.lParam = NULL;		// unused

		SortChildrenCB(&sortStruct);
		return;
	}

	if( theAction == CA_DELETE ) 
	{
		DeleteItemData( hItem );
		DeleteItem( hItem );	
		return;
	}


	if( theAction == CA_GRAFT ) 
	{
		if( pData != NULL )
		{
			hDestItem = FindPathname( GetRootItem(), ( ( QCMailboxCommand* ) pData ) ->GetPathname() );
		}
		else
		{
			hDestItem = GetRootItem();
		}

		if( hDestItem == NULL ) 
		{
			return;
		}
		
		tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
		tvstruct.item.hItem = hItem;
		tvstruct.item.state = 0;
		tvstruct.item.stateMask = TVIS_BOLD | TVIS_SELECTED;
		tvstruct.item.pszText = szBuf;
		tvstruct.item.cchTextMax = 256;
		tvstruct.item.iImage = 0;
		tvstruct.item.iSelectedImage = 0;
		tvstruct.item.cChildren = 0;
		tvstruct.item.lParam = 0;

		if( GetItem( &( tvstruct.item ) ) )
		{
			DeleteItem( hItem );
		
			tvstruct.hParent = hDestItem;
			tvstruct.hInsertAfter = TVI_LAST;

			hItem = InsertItem( &( tvstruct ) );

			//
			// Do a smart re-sort after adding the new item.
			//
			TV_SORTCB sortStruct;
			sortStruct.hParent = GetParentItem(hItem);
			ASSERT(sortStruct.hParent);
			sortStruct.lpfnCompare = CMboxTreeCtrl::CompareFunc;
			sortStruct.lParam = NULL;		// unused
			SortChildrenCB(&sortStruct);

			//
			// Select the newly-added item.
			//
			SelectItem(hItem);
			EnsureVisible(hItem);
		}
		return;
	}

	if( theAction == CA_UPDATE_STATUS )
	{
		theStatus = ( UnreadStatusType ) ( ULONG ) pData;
		if( theStatus == US_YES )
		{
			theState = TVIS_BOLD;
		}
		else
		{
			theState = 0;
		}

		SetItem( hItem, TVIF_STATE, NULL, 0, 0, theState, TVIS_BOLD, 0 );		
	}
}

#ifdef IMAP4
//
// Verify that during a rename, the new IMAP mailbox name contains valid chars.
//
BOOL IsValidImapName (QCMailboxCommand* pCommand, LPCSTR pNewname)
{
	BOOL bResult = TRUE;

	// Sanity.
	if (! (pCommand && pNewname) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Must be an IMAP comamnd object.
	if( pCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) == FALSE )
	{
		ASSERT (0);
		return FALSE;
	}

	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

	// Must NOT contain delimiter char.
	TCHAR cDelimiter = pImapCommand->GetDelimiter();

	if (cDelimiter && (strchr ( pNewname, cDelimiter ) != NULL) )
	{
		ErrorDialog(IDS_MBOX_RENAME_HAS_DELIMITER, cDelimiter);
		bResult = FALSE;
	}

	return bResult;
}



// ImapMboxIsDragable [INTERNAL]
//
// Return TRUE if this mailbox's name is NOT "inbox" (case-insensitive),
// or if it's NOT flagged as \NOSELECT.
//

BOOL ImapMboxIsDragable (QCMailboxCommand *pCommand, int itemType)
{
	// Must be of type ITEM_IMAP_MAILBOX
	//
	if (itemType != CMboxTreeCtrl::ITEM_IMAP_MAILBOX)
		return FALSE;

	// Sanity:
	if (!pCommand)
	{
		ASSERT (0);
		return FALSE;
	}

	// Must be an IMAP comamnd object.
	if( pCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) == FALSE )
	{
		ASSERT (0);
		return FALSE;
	}

	// Can now Cast:
	QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

	// Don't allow drag of a no-select mailbox.
	if ( pImapCommand->IsNoSelect () )
	{
		return FALSE;
	}

	// Can't drag inbox:
	CString szImapName = pImapCommand->GetImapName();

	if (szImapName.CompareNoCase (CRString (IDS_IMAP_RAW_INBOX_NAME)) == 0)
		return FALSE;

	// Accept all others:
	return TRUE;
}




// ImapTargetIsDroppable [INTERNAL]
//
// Return TRUE if the command object is an account, or, if it's
// a mailbox, it is NOT flagged as \NOINFERIORS.
// Note: Both source and target objects must be within the same account.
//
BOOL ImapTargetIsDroppable (QCMailboxCommand *pSourceCommand, int iSourceItemType,
							QCMailboxCommand *pTargetCommand, int iTargetItemType)
{
	QCImapMailboxCommand *pImapSourceCommand = NULL;
	QCImapMailboxCommand *pImapTargetCommand = NULL;

	// Must have these:
	if (! (pTargetCommand && pSourceCommand) )
		return FALSE;

	// Command targets must be IMAP type!!
	if( pSourceCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) == FALSE )
	{
		ASSERT (0);
		return FALSE;
	}

	if( pTargetCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) == FALSE )
	{
		ASSERT (0);
		return FALSE;
	}

	// Cast the command objects to IMAP command objects and use them from now on:
	pImapSourceCommand = (QCImapMailboxCommand *) pSourceCommand;
	pImapTargetCommand = (QCImapMailboxCommand *) pTargetCommand;

	// Make sure:
	ASSERT (pImapSourceCommand->GetAccountID () != 0);
	ASSERT (pImapTargetCommand->GetAccountID () != 0);

	// Both source and target command objects must be in the same IMAP account.
	//
	if (pImapSourceCommand->GetAccountID () != pImapTargetCommand->GetAccountID () )
		return FALSE;

	// TArget must be of type ITEM_IMAP_MAILBOX or ITEM_IMAP_ACCOUNT.
	//
	if ( iTargetItemType == CMboxTreeCtrl::ITEM_IMAP_ACCOUNT )
	{
		return TRUE;
	}
	else if ( iTargetItemType == CMboxTreeCtrl::ITEM_IMAP_MAILBOX ) 
	{
		// Don't allow drag to a no-inferiors mailbox.
		if ( !pImapTargetCommand->IsNoInferiors () )
		{
			return TRUE;
		}
	}

	// Reject all others:
	return FALSE;
}




// Major mods needed for IMAP, put in this file so we can do updates easily.
#include "imaptree.cpp"

#endif // IMAP4

