// nicktree.cpp: implementation
//
// CNicknamesTreeCtrl
// Smart nicknames-specific tree control that is meant to be embedded
// in a parent "dialog" class.

#include "stdafx.h"

#include <afxpriv.h>			// for MFC-defined WM_SETMESSAGESTRING

#ifndef WIN32
#error This is a WIN32-only file
#endif

extern CString EudoraDir;		// avoids inclusion of fileutil.h

#include "eudora.h"
#include "doc.h"
#include "nickdoc.h"
#include "nicktree.h"
#include "cursor.h"
#include "guiutils.h"
#include "helpcntx.h"
#include "resource.h"
#include "fileutil.h"
#include "urledit.h"	// for URLEDIT.H
#include "nickpage.h"	// for NICKSHT.H
#include "nicksht.h"	// for NICKVIEW.H
#include "nickview.h"
#include "helpxdlg.h"
#include "rs.h"
#include "headertype.h"

#include "QCCommandActions.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"

#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static WNDPROC s_pOldEditProc = NULL;
static CTreeCtrl* s_pTreeCtrl = NULL;

extern QCRecipientDirector	g_theRecipientDirector;

IMPLEMENT_DYNAMIC(CNicknamesTreeCtrl, QCTreeCtrl)

BEGIN_MESSAGE_MAP(CNicknamesTreeCtrl, QCTreeCtrl)
	//{{AFX_MSG_MAP(CNicknamesTreeCtrl)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_INITMENUPOPUP()
	ON_MESSAGE(WM_RBUTTONDOWN, OnRightButtonDown)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnLButtonDblClk)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_COMMAND(ID_POPUP_NEW, OnNew)
	ON_COMMAND(ID_POPUP_DELETE, OnDelete)
	ON_COMMAND(ID_POPUP_RENAME, OnRename)
	ON_COMMAND(ID_POPUP_MOVE_TO, OnMoveTo)
	ON_COMMAND(ID_POPUP_COPY_TO, OnCopyTo)
	ON_COMMAND(ID_POPUP_ADD_RECIPIENT, OnAddRecipient)
	ON_COMMAND(ID_POPUP_REMOVE_RECIPIENT, OnRemoveRecipient)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNicknamesTreeCtrl

CNicknamesTreeCtrl::CNicknamesTreeCtrl() :
	m_hAnchorItem(NULL),
	m_EvaluateSelectionHint(EVALHINT_FULL),
	m_LastEvaluatedSelectionState(SELSTATE_UNKNOWN),
	m_customImageIndex(-1),
	m_pDragImage(NULL),			// intra-tree drag and drop
	m_hDragItem(NULL),			// intra-tree drag and drop
	m_viewByItemType(ITEM_NICKNAME),
	m_bInEditMode( FALSE )
{

}

CNicknamesTreeCtrl::~CNicknamesTreeCtrl()
{
	//
	// Note: clients should call Reset() before we get here...
	// Otherwise, we will have memory leaks on the heap-allocated
	// CNicknamesTreeItemData objects.
	//
}


////////////////////////////////////////////////////////////////////////
// Init [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::Init()
{
	if (! QCTreeCtrl::Init())
		return FALSE;

	BOOL found = FALSE;

	//
	// First, check resource DLLs for IDB_NICKTREE bitmap override.
	//
	CPtrList* p_res_list = ((CEudoraApp *) AfxGetApp())->GetResListPtr();
	POSITION pos = p_res_list->GetHeadPosition(); 
	while (pos != NULL)
	{
#ifdef WIN32
		HINSTANCE hResInst = (HINSTANCE) p_res_list->GetNext(pos);
#else
		HINSTANCE hResInst = (HINSTANCE)(WORD)(DWORD) p_res_list->GetNext(pos);
#endif

		if (hResInst && ::FindResource(hResInst, MAKEINTRESOURCE(IDB_NICKTREE), RT_BITMAP))
		{
			HINSTANCE h_temp = AfxGetInstanceHandle();		// FORNOW, for debugging
			HINSTANCE h_save_inst = AfxGetResourceHandle();
			AfxSetResourceHandle(hResInst);
			// Load the bitmap images for tree control.
			BOOL status = m_ImageList.Create(IDB_NICKTREE, 16, 0, RGB (0,255,0));
			AfxSetResourceHandle(h_save_inst);
			if (! status)
				return FALSE;
			found = TRUE;
			break;
		}
	}

	if (! found)
	{
		//
		// Load default IDB_NICKTREE bitmap.
		//
		if (! m_ImageList.Create(IDB_NICKTREE, 16, 0, RGB (0,255,0)))
			return FALSE;		// mangled resources?
	}

	SetImageList(&m_ImageList, TVSIL_NORMAL);
	
	//
	// Tune size of selected item array.
	//
	m_SelectedItemArray.SetSize(0, 100);

	ASSERT(NULL == GetRootItem());

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
// Handles CTreeCtrl HWND destruction by calling the Reset() method
// to cleanup all the heap-allocated memory.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnDestroy()
{
	CCursor();		// this could take awhile
	Reset();		// make sure heap allocations are cleared up
	QCTreeCtrl::OnDestroy();
}


////////////////////////////////////////////////////////////////////////
// Reset [public]
//
// Delete all tree items and their lParam-based storage from this
// tree control.  Clients should not call the DeleteItem() or 
// DeleteAllItems() methods directly.
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::Reset(void)
{
	//
	// First, loop through all items and delete the lParam-based storage.
	//
	HTREEITEM h_item = GetRootItem();
	while (h_item != NULL)
	{
		//
		// Recursively process the subtree rooted at 'h_item'.
		//
		DeleteItemData(h_item);
		h_item = GetNextSiblingItem(h_item);
	}

	//
	// Set the hint to SKIP to prevent evaluation of any (stale) 
	// selections while in the guts of the DeleteAllItems() call.
	// 
	m_EvaluateSelectionHint = EVALHINT_SKIP;
	DeleteAllItems();

	m_SelectedItemArray.RemoveAll();
	m_hAnchorItem = NULL;
	m_EvaluateSelectionHint = EVALHINT_FULL;
	m_LastEvaluatedSelectionState = SELSTATE_UNKNOWN;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// AddFile [public]
//
// Adds an entire nickname file to the tree control.
// Returns TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::AddFile(CNicknameFile* pNicknameFile)
{
	// let the internal routine to all the work.
	ASSERT(pNicknameFile != NULL);
	HTREEITEM h_nickfile = AddItem(ITEM_FILE, pNicknameFile->m_Name, FALSE, pNicknameFile);
	if (NULL == h_nickfile)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// AddNewNickname [public]
//
// Adds a new nickname to an *existing* file in the tree control.
// Returns TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::AddNewNickname(CNicknameFile* pNicknameFile, CNickname* pNickname)
{
	ASSERT(pNickname != NULL);
	ASSERT(pNicknameFile != NULL);
 	ASSERT(! pNicknameFile->m_ReadOnly);

	//
	// Find the nickname file item which matches the
	// given 'pNicknameFile'.  We need its HTREEITEM handle to pass to
	// the internal AddItem() routine.
	//
	HTREEITEM h_nickfile = FindFileItem(pNicknameFile);
	if (NULL == h_nickfile)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Determine whether or not this is the first time this node is
	// being expanded.  This is important since the nickname item is
	// added to the nicknames database *before* we get here.  If this
	// is "first time" expansion, then the PopulateNicknameFileNode()
	// call below will automatically add the new nickname item.  But,
	// if the tree node has already been populated, then we have the
	// burden of adding the new item to the tree control ourselves.
	//
	BOOL expanded_once = FALSE;
	{
		UINT state = GetItemState(h_nickfile, TVIS_EXPANDEDONCE);
		if (state & TVIS_EXPANDEDONCE)
			 expanded_once = TRUE;
	}

	//
	// Force the "visual" expansion of the nickname file item.
	// Remember whether or not this is the first time this node is
	// being expanded.
	//
	Expand(h_nickfile, TVE_EXPAND);
	SetItemImage(h_nickfile, IMAGE_OPEN_FOLDER, IMAGE_OPEN_FOLDER);

	if (expanded_once)
	{
		//
		// Take the current "View By" (display mode) into account in
		// order to determine the item text.  For all of the nickname
		// fields other than the nickname name itself, we need to
		// force the nickname data to be loaded into memory from the
		// nickname file.
		// 
		CString item_text;
		if (m_viewByItemType != ITEM_NICKNAME)
		{
			if (pNickname->NeedsReadIn())
				pNicknameFile->ReadNickname(pNickname);
		}
		GetNicknameItemText(pNickname, item_text);

		HTREEITEM h_nickname = AddItem(m_viewByItemType, item_text, pNickname->IsRecipient(), pNickname, h_nickfile);
		if (NULL == h_nickname)
			return FALSE;

		SortChildren(h_nickfile);
	}
	else
	{
		PopulateNicknameFileNode(h_nickfile);
	}

	//
	// Now that we have opened the nickname file containing the new
	// nickname, select the new nickname.
	//
	HTREEITEM h_nickname = GetChildItem(h_nickfile);
	while (h_nickname != NULL)
	{
		if (GetSafeNickname(h_nickname) == pNickname)
		{
			//
			// Force a single selection.
			//
			DoDeselectAll();
			SelectItem(h_nickname);
			break;		// we're outta here
		}

		h_nickname = GetNextVisibleItem(h_nickname);
	}
	ASSERT(h_nickname != NULL);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// DeleteExistingNickname [public]
//
// Deletes an existing nickname item from the tree control and then
// from the nicknames database.  We must do it in that order to
// guarantee that the CNickname pointer remains valid throughout the
// processing.  If 'bLastOne' is TRUE, then re-evaluate the selection
// and cleanup the recipient list.  Returns TRUE if successful.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::DeleteExistingNickname(CNicknameFile* pNicknameFile, CNickname* pNickname, BOOL bLastOne /*=TRUE*/)
{
	//
	// Step 1.  Walk the list of parent nickname file items, looking
	// for an item which matches the given 'pNicknameFile'.
	//
	ASSERT(pNicknameFile && !pNicknameFile->m_ReadOnly);
	HTREEITEM h_nickfile = FindFileItem(pNicknameFile);
	if (NULL == h_nickfile)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Step 2.  Walk the list of child nickname items in that file,
	// looking for the item which matches the given 'pNickname'.
	//
	ASSERT(pNickname != NULL);
	HTREEITEM h_nickname = GetChildItem(h_nickfile);
	while (h_nickname != NULL)
	{
		if (GetSafeNickname(h_nickname) == pNickname)
			break;			// found it
		h_nickname = GetNextSiblingItem(h_nickname);
	}

	if (NULL == h_nickname)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Step 3.  Remove the nickname item and its associated lParam
	// storage from the tree control.
	//
	CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_nickname);
	ASSERT(p_itemdata != NULL);
	delete p_itemdata;
	DeleteItem(h_nickname);

	//
	// Now, remove the nickname from the database.  The CNicknameFile::RemoveAt()
	// method takes care of cleaning up the Recipient menu.
	//
	pNicknameFile->RemoveAt(pNicknameFile->CObList::Find(pNickname));
	pNicknameFile->SetModified(TRUE);

	if (bLastOne)
	{
		//
		// Step 4.  At this point, we have no idea what state the selection
		// is in, so force a full evaluation.
		//
		m_hAnchorItem = NULL;
		m_EvaluateSelectionHint = EVALHINT_FULL;
		EvaluateSelection();

		//
		// Fixup recipient menu, if necessary.
		//

	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// RestoreVisualState [public]
//
// Restores the visual state of the tree control back to some previously
// saved state.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::RestoreVisualState()
{
	//FORNOW, just open the main item.
	if (GetRootItem())
	{
		PopulateNicknameFileNode(GetRootItem());
		Expand(GetRootItem(), TVE_EXPAND);
		SelectItem(GetRootItem());	//FORNOW
		//FORNOW EvaluateSelection();
		m_hAnchorItem = GetRootItem();
		return TRUE;
	}

	return FALSE;		// FORNOW
}


////////////////////////////////////////////////////////////////////////
// GetSelectedNicknames [public]
//
// Fetches a list of selected nicknames from the tree control, returning
// the list in the caller-provided CObject list.  Returns TRUE if
// successful.  If the 'bGetNicknameFiles' flag is enabled, then also
// include the CNicknameFile in the returned list as follows:
//
//		CNickname1
//		CNicknameFile1
//		CNickname2
//		CNicknameFile2
//		...
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::GetSelectedNicknames(CObList& nicknameList, BOOL bGetNicknameFiles /*=FALSE*/)
{
	ASSERT(nicknameList.IsEmpty());

	HTREEITEM h_parent = GetRootItem();
	while (h_parent != NULL)
	{
		if (IsItemExpanded(h_parent))
		{
			HTREEITEM h_child = GetChildItem(h_parent);
			while (h_child != NULL)
			{
				if (IsItemSelected(h_child))
				{
					CNickname* p_nickname = GetSafeNickname(h_child);
					ASSERT(p_nickname != NULL);
					nicknameList.AddTail(p_nickname);
					if (bGetNicknameFiles)
					{
						CNicknameFile* p_nickfile = GetSafeNicknameFile(h_parent);
						ASSERT(p_nickfile != NULL);

						//
						// WARNING! WARNING!  The CObList::AddTail() method is
						// overloaded to accept CObList ptrs as well as CObject
						// ptrs, with decidely different behaviors.  Therefore,
						// we need to cast the CObList to a CObject to force
						// the behavior we want.
						//
						nicknameList.AddTail((CObject *) p_nickfile);
					}
				}

				h_child = GetNextSiblingItem(h_child);
			}
		}

		h_parent = GetNextSiblingItem(h_parent);
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// FindSingleSelectedNicknameFile [public]
//
// If there is a single-selected item and it happens to be a nickname
// file item, then return the corresponding CNicknameFile pointer.
// Otherwise, if it is a single-selected nickname item, then find the
// corresponding CNicknameFile parent.  Otherwise, harmlessly return
// NULL.
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesTreeCtrl::FindSingleSelectedNicknameFile()
{
	if (SELSTATE_SINGLE == m_LastEvaluatedSelectionState)
	{
		HTREEITEM h_selitem = GetSelectedItem();
		ASSERT(h_selitem != NULL);

		if (GetSafeNickname(h_selitem))
		{
			//
			// Convert singly-selected nickname item into nickname
			// file item.
			//
			HTREEITEM h_parent = GetParentItem(h_selitem);
			ASSERT(h_parent != NULL);
			h_selitem = h_parent;
		}

		return GetSafeNicknameFile(h_selitem);		// can be NULL
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// MarkSelectedNicknameDirty [public]
//
// We've been notified that at least one field of the singly-selected
// nickname has been modified.  Therefore, we need to determine which
// file contains the nickname and mark the file as dirty.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::MarkSelectedNicknameDirty()
{
	//
	// Fetch the singly-selected item and verify that it is a nickname.
	//
	ASSERT(SELSTATE_SINGLE == m_LastEvaluatedSelectionState);
	HTREEITEM h_selitem = GetSelectedItem();
	ASSERT(h_selitem != NULL);
	ASSERT(GetSafeNickname(h_selitem) != NULL);

	//
	// Then, get the parent nickname file and mark it as dirty.
	//
	HTREEITEM h_parent = GetParentItem(h_selitem);
	ASSERT(h_parent != NULL);
	CNicknameFile* p_nickfile = GetSafeNicknameFile(h_parent);
	ASSERT(p_nickfile != NULL);
	p_nickfile->SetModified(TRUE);
}


////////////////////////////////////////////////////////////////////////
// BroadcastSelectedNickname [public]
//
// Broadcast the current, singly-selected nickname to all interested
// parties.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::BroadcastSelectedNickname()
{
	//
	// Fetch the singly-selected item and verify that it is a nickname.
	//
	ASSERT(SELSTATE_SINGLE == m_LastEvaluatedSelectionState);
	HTREEITEM h_selitem = GetSelectedItem();
	ASSERT(h_selitem != NULL);
	CNickname* p_nickname = GetSafeNickname(h_selitem);		// can be NULL if nickname *file* is selected

	if (g_Nicknames && p_nickname)
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_SENDING_NICKNAME, p_nickname);
}


////////////////////////////////////////////////////////////////////////
// NicknameWasChanged [public]
//
// Nickname that we're displaying was changed externally, so find its
// corresponding tree item and fix up its label.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::NicknameWasChanged(CNickname* pNickname)
{
	ASSERT(pNickname != NULL);
	HTREEITEM h_parent = GetRootItem();
	while (h_parent != NULL)
	{
		if (IsItemExpanded(h_parent))
		{
			HTREEITEM h_child = GetChildItem(h_parent);
			while (h_child != NULL)
			{
				if (GetSafeNickname(h_child) == pNickname)
				{
					ASSERT(! pNickname->NeedsReadIn());
					CString item_text;
					GetNicknameItemText(pNickname, item_text);

					TV_ITEM tv_item;
					tv_item.mask = TVIF_HANDLE | TVIF_TEXT;
					tv_item.hItem = h_child;
					tv_item.pszText = (char *) ((const char *) item_text);
					SetItem(&tv_item);

					//
					// While we're at it, force a single selection.
					//
					if (m_LastEvaluatedSelectionState != SELSTATE_SINGLE)
						DoDeselectAll();
					SelectItem(h_child);
					m_LastEvaluatedSelectionState = SELSTATE_SINGLE;
					return;		// we're outta here
				}

				h_child = GetNextVisibleItem(h_child);
			}
		}

		h_parent = GetNextSiblingItem(h_parent);
	}
}


////////////////////////////////////////////////////////////////////////
// CheckForDuplicateNickname [public]
//
// Check to see if a duplicate nickname exists in the same nickname file
// as the currently selected nickname.  If so, then set the 'pNewName'
// value to empty to indicate a duplicate.  Otherwise, leave the
// 'pNewName' value unchanged.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::CheckForDuplicateNickname(CString* pNewName)
{
	ASSERT(pNewName != NULL);
	ASSERT(! pNewName->IsEmpty());

	//
	// Fetch the singly-selected item and verify that it is a nickname.
	//
	ASSERT(SELSTATE_SINGLE == m_LastEvaluatedSelectionState);
	HTREEITEM h_selitem = GetSelectedItem();
	ASSERT(h_selitem != NULL);
	CNickname* p_selected_nickname = GetSafeNickname(h_selitem);
	ASSERT(p_selected_nickname != NULL);

	//
	// Then, get the parent nickname file and check for a duplicate.
	//
	HTREEITEM h_parent = GetParentItem(h_selitem);
	ASSERT(h_parent != NULL);
	CNicknameFile* p_nickfile = GetSafeNicknameFile(h_parent);
	ASSERT(p_nickfile != NULL);
	CNickname* p_dupe_nickname = p_nickfile->FindNickname(*pNewName);

	if ((p_dupe_nickname != NULL) && (p_dupe_nickname != p_selected_nickname))
	{
		//
		// Found a duplicate nickname.
		//
		pNewName->Empty();
	}
}


////////////////////////////////////////////////////////////////////////
// RefreshRecipListDisplay [public]
//
// Refreshes bold/non-bold state of each nickname item in the tree to
// reflect the current "recipient list" setting for each nickname.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::RefreshRecipListDisplay()
{
	SetRedraw(FALSE);
	HTREEITEM h_item = GetRootItem();
	while (h_item != NULL)
	{
		//
		// Recursively process the subtree rooted at 'h_item'.
		//
		DoRefreshRecipListDisplay(h_item);
		h_item = GetNextSiblingItem(h_item);
	}
	SetRedraw(TRUE);
}


////////////////////////////////////////////////////////////////////////
// RemoveSelectedItems [public]
//
// Removes the selected items from the tree control.  This has to 
// account for several special cases: 1) you can't delete the main
// Eudora nicknames file, 2) you can't delete a read-only file, 3) you
// can't delete items from a read-only file, and 4) if the deleted
// nickname is on the recipient menu, it needs to be deleted from the
// recipient menu.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::RemoveSelectedItems()
{
	//
	// Step 1.  Save a "smart" snapshot of the selected items in the
	// tree.  Only save nickname items if they are not in a read-only
	// file and they are not in a file selected for deletion.
	//
	// We need to save a snapshot because the tree control is smart
	// enough to auto-select new items after one or more selected
	// items are deleted.  Therefore, we have to gather the items
	// which were selected first, and then process the selection
	// second.  If we don't do this, then the selection will mutate
	// while we're pruning the tree, leading to very broken results if
	// we try to process an ever-changing "selection".
	//
	HTREEITEM h_nickfile = GetRootItem();
	CPtrList selected_nickfile_handles;
	CPtrList selected_nickname_handles;
	BOOL displayed_readonly_alert = FALSE;
	CCursor wait_cursor;
	while (h_nickfile != NULL)
	{
		CNicknameFile* p_nickfile = GetSafeNicknameFile(h_nickfile);
		ASSERT(p_nickfile != NULL);
		if (IsItemSelected(h_nickfile))
			selected_nickfile_handles.AddTail(h_nickfile);
		else
		{
			// Process children in unselected files, if the children are visible
			if (TVIS_EXPANDED & GetItemState(h_nickfile, TVIS_EXPANDED))
			{
				HTREEITEM h_nickname = GetChildItem(h_nickfile);
				while (h_nickname != NULL)
				{
					if (IsItemSelected(h_nickname))
					{
						if (p_nickfile->m_ReadOnly)
						{
							if (! displayed_readonly_alert)
							{
								//
								// Inform the user that trying to remove items
								// from read-only files is futile... Remind them
								// only once, and give them a chance to bail the
								// entire remove action.
								//
								if (AlertDialog(IDD_REMOVE_NICK) != IDOK)
									return;		// user wants to bail
								displayed_readonly_alert = TRUE;
							}
						}
						else
						{
							//
							// Save nickname item that is selected, not
							// in a selected file, and not in a read-only 
							// file.
							//
							selected_nickname_handles.AddTail(h_nickname);
						}
					}
					h_nickname = GetNextSiblingItem(h_nickname);
				}
			}
		}

		h_nickfile = GetNextSiblingItem(h_nickfile);
	}

	SetRedraw(FALSE);

	//
	// Step 2.  Process list of selected nickname files, removing them
	// from the tree, the nicknames database, and deleting the files.
	// This only works for files that are not the main Eudora nicknames
	// file and are not marked as read-only.  Ask for confirmation of
	// removal for *each* nickname file, since this is destructive and
	// not undoable.
	//
	while (! selected_nickfile_handles.IsEmpty())
	{
		h_nickfile = HTREEITEM(selected_nickfile_handles.RemoveHead());
		ASSERT(h_nickfile != NULL);

		CNicknameFile* p_nickfile = GetSafeNicknameFile(h_nickfile);
		ASSERT(p_nickfile != NULL);

		if (GetRootItem() == h_nickfile)
		{
			//
			// We've hit the special case where you can't delete the
			// main Eudora Nicknames file.
			//
			::ErrorDialog(IDS_ERR_NICK_DELETING_MAIN);
		}
		else if (p_nickfile->m_ReadOnly)
		{
			//
			// Another special case.  Tell the user that we can't
			// delete read-only nickname files.
			//
			::ErrorDialog(IDS_ERR_NICK_DELETING_READONLY, (const char *) p_nickfile->m_Name);
		}
		else
		{
			//
			// Warn the user that we're about to delete a nickname
			// file and that this cannot be undone.
			//
			CString msg;
			msg.Format(CRString(IDS_NICK_PERMANENT_DELETE), (const char *) p_nickfile->m_Name);
			if (IDYES == ::AfxMessageBox(msg, MB_ICONQUESTION | MB_YESNO))
			{
				//
				// Walk the list of nicknames in the file searching
				// for items on the recipient menu.  If found, remove
				// the corresponding item from the recipient menu.
				// Then, delete the nickname item from the tree.
				//
				HTREEITEM h_nickname = GetChildItem(h_nickfile);
				while (h_nickname != NULL)
				{
					CNickname* p_nickname = GetSafeNickname(h_nickname);
					ASSERT(p_nickname != NULL);
					if (p_nickname->IsRecipient())
#ifdef OLDSTUFF
						g_RecipientList.Remove(p_nickname->GetName());
#else
					{
						QCRecipientCommand*	pCommand;

						pCommand = g_theRecipientDirector.Find( p_nickname->GetName() );

						if( pCommand != NULL )
						{
							pCommand->Execute( CA_DELETE );
						}
					}
#endif
					//
					// Delete the nickname item and its associated
					// lParam storage from the tree control.  We don't
					// need to delete the CNickname objects from the
					// nickname database since that automatically
					// happens when we delete the CNicknameFile
					// object.
					//
					HTREEITEM h_delete = h_nickname;
					h_nickname = GetNextSiblingItem(h_nickname);
					CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_delete);
					ASSERT(p_itemdata != NULL);
					delete p_itemdata;
					DeleteItem(h_delete);
				}

				//
				// Do the file removal.  We need to remove both
				// the nickname file (*.txt) and its TOC file (*.toc).
				//
				CString filename(p_nickfile->m_Filename);
				::FileRemoveMT(filename);		// remove the nickname file
				::SetFileExtensionMT((char *) ((const char *) filename), CRString(IDS_TOC_EXTENSION));
				::FileRemoveMT(filename);		// remove the associated TOC

				//
				// Finally, remove the nickname file from the nickname
				// database, then remove the nickname file item from the
				// tree control.
				//
				ASSERT(g_Nicknames != NULL);
				VERIFY(g_Nicknames->RemoveNicknameFile(p_nickfile));
				CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_nickfile);
				ASSERT(p_itemdata != NULL);
				delete p_itemdata;
				DeleteItem(h_nickfile);
			}
		}
	}

	//
	// Step 3.  Process the list of selected nicknames, removing them
	// from the tree control, and the nicknames database.  Mark each
	// modified nickname file as "dirty", therefore triggering the
	// "file needs to be saved" action. 
	//
	while (! selected_nickname_handles.IsEmpty())
	{
		//
		// Fetch the nickname and its parent nickname file.
		//
		HTREEITEM h_nickname = HTREEITEM(selected_nickname_handles.RemoveHead());
		ASSERT(h_nickname != NULL);
		CNickname* p_nickname = GetSafeNickname(h_nickname);
		ASSERT(p_nickname != NULL);

		h_nickfile = GetParentItem(h_nickname);
		ASSERT(h_nickfile != NULL);
		CNicknameFile* p_nickfile = GetSafeNicknameFile(h_nickfile);
		ASSERT(p_nickfile != NULL);
		ASSERT(! p_nickfile->m_ReadOnly);

		//
		// Delete the nickname item and its associated lParam storage
		// from the tree control.
		//
		CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_nickname);
		ASSERT(p_itemdata != NULL);
		delete p_itemdata;
		DeleteItem(h_nickname);

		//
		// Now, remove the nickname from the database.  The CNicknameFile::RemoveAt()
		// method takes care of cleaning up the Recipient menu.
		//
		p_nickfile->RemoveAt(p_nickfile->CObList::Find(p_nickname));
		p_nickfile->SetModified(TRUE);
	}

	SetRedraw(TRUE);

	//
	// Step 4.  At this point, we have no idea what state the selection
	// is in, so force a full evaluation.
	//
	m_hAnchorItem = NULL;
	m_EvaluateSelectionHint = EVALHINT_FULL;
	EvaluateSelection();

}


////////////////////////////////////////////////////////////////////////
// SetViewByMode [public]
//
// Convert a field name into a "view by" mode that the tree control
// understands.  FORNOW, it would be nice if this could be reworked
// into a common 16/32 implementation so that we don't have the same
// code in two places.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::SetViewByMode(int nIndex, const char* pFieldName)
{
	ASSERT(pFieldName != NULL);

	ItemType old_type = m_viewByItemType;


	CString old_custom_fieldname = m_customFieldName;

	int image_idx = -1;

	if (stricmp(pFieldName, NICKNAME_FIELD_NICKNAME) == 0)
	{
		m_viewByItemType = ITEM_NICKNAME; 
		image_idx = IMAGE_NICKNAME;
	}
	else if (stricmp(pFieldName, NICKNAME_FIELD_ADDRESSES) == 0)
	{
		m_viewByItemType = ITEM_ADDRESS; 
		image_idx = IMAGE_ADDRESS;
	}
	else
	{
		m_viewByItemType = ITEM_CUSTOM;
		ASSERT(nIndex >= 2);
		//
		// FORNOW, not doing any range checking to see if there is
		// actually a bitmap at the image_idx position.
		//
		m_customImageIndex = IMAGE_CUSTOM + nIndex - 2;
		image_idx = m_customImageIndex;

		m_customFieldName = pFieldName;
	}

/*
	if (old_type == m_viewByItemType)
	{
		//
		// Do an extra check for custom fields.
		//
		if (ITEM_CUSTOM == m_viewByItemType)
		{
			if (old_custom_fieldname.CompareNoCase(m_customFieldName) == 0)
				return;		// no change
		}
		else
			return;		// no change
	}
*/

	// Get main window so that we can access the global status bar.
	CFrameWnd* p_mainframe = (CFrameWnd *) AfxGetMainWnd();
	ASSERT(p_mainframe != NULL);
	ASSERT(p_mainframe->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	CRString fmt(IDS_LOADING_NICKNAMES_PERCENT);

	//
	// Now run through all nickname items (whether visible or not)
	// and change the nickname item text to match the current display 
	// mode.
	//
	CCursor wait_cursor;
	HTREEITEM h_nickfile = GetRootItem();
	while (h_nickfile != NULL)
	{
		CNicknameFile* p_nickfile = GetSafeNicknameFile(h_nickfile);
		ASSERT(p_nickfile != NULL);

		unsigned TOTAL = p_nickfile->GetCount();
		unsigned count = 0;

		HTREEITEM h_nickname = GetChildItem(h_nickfile);
		while (h_nickname != NULL)
		{
			CNickname* p_nickname = GetSafeNickname(h_nickname);
			ASSERT(p_nickname != NULL);

			//
			// Change the tree item text and the associated bitmap.
			//
			CString item_text;
			if (m_viewByItemType != ITEM_NICKNAME)
			{
				if (p_nickname->NeedsReadIn())
					p_nickfile->ReadNickname(p_nickname);
			}
			GetNicknameItemText(p_nickname, item_text);

			TV_ITEM tv_item;
			tv_item.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tv_item.hItem = h_nickname;
			tv_item.pszText = (char *) ((const char *) item_text);
			tv_item.iImage = image_idx;
			tv_item.iSelectedImage = image_idx;
			SetItem(&tv_item);

			//
			// Change the internal "item type" value.
			//
			CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_nickname);
			ASSERT(p_itemdata != NULL);
			p_itemdata->m_itemType = m_viewByItemType;

			h_nickname = GetNextSiblingItem(h_nickname);

			//
			// Periodically update progress pacifier on main status bar...
			//
			if ((++count & 0x003F) == 0x003F)
			{
				char prompt[200];
				wsprintf(prompt, fmt, p_nickfile->m_Name, (count * 100) / TOTAL);
				ASSERT(strlen(prompt) < sizeof(prompt));	// better late than never
				p_mainframe->SetMessageText(prompt);
				p_mainframe->GetMessageBar()->UpdateWindow();
			}
		}

		//
		// Now re-sort the nicknames by the new item text.
		//
		CString prompt;
		prompt.Format(IDS_LOADING_NICKNAMES_SORTING, (const char*)p_nickfile->m_Name);
		p_mainframe->SetMessageText(prompt);
		p_mainframe->GetMessageBar()->UpdateWindow();
		SortChildren(h_nickfile);

		h_nickfile = GetNextSiblingItem(h_nickfile);
	}

	// Restore default status bar prompt.
	p_mainframe->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
}


////////////////////////////////////////////////////////////////////////
// EvaluateSelection [public]
//
// Fetches the current selection from the tree control and evaluates
// the net effect of the current selection on the rest of the
// display.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::EvaluateSelection()
{
	//
	// This is the One True Place where the m_hAnchorItem member
	// is supposed to be set to reflect the "current selected item".  The
	// only exception to this is when we handle keyboard multi-selection.
	// In the latter case, the "anchor" item doesn't change even when
	// the "current selected item" changes.
	//
	m_hAnchorItem = GetSelectedItem();

	//
	// First things first.  If there's nobody to receive the broadcast
	// of the selection results, or if we're given a hint that
	// an evaluation is unnecessary, then we can go home early.
	//
	if (NULL == g_Nicknames)
		return;

	if (EVALHINT_SKIP == m_EvaluateSelectionHint)
	{
		// Performance hack!
		TRACE0("CNicknamesTreeCtrl::EvaluateSelection: skipping traversal\n");
		return;
	}

	//
	// PART ONE.  Setup for evaluating the selection, then do the analysis.
	// When we're done, 'selcount' contains the number of selected items and
	// hint contains the overall result to pass on to the UpdateAllViews routine.
	// If we determine that we have a single selection, 'h_single_nickname' is
	// non-NULL for a singly selected nickname item, or NULL for a singly
	// selected nickname file item.  Got all that?
	//
	int selcount = 0;
	HTREEITEM h_single_nickname = NULL;		// handle for singly-selected nickname, if any
	NicknamesHintType hint = NICKNAME_HINT_MULTISELECT_RECIPLIST_DISABLE;

	if (EVALHINT_SINGLE == m_EvaluateSelectionHint)
	{
		//
		// Performance hack.  We're given a hint that there is only one 
		// item selected, so just find the singly-selected item and 
		// we're done.  If it is a nickname item, then saves its handle
		// in 'h_single_nickname'.
		//
		h_single_nickname = GetSelectedItem();
		ASSERT(h_single_nickname != NULL);
		if (NULL == GetSafeNickname(h_single_nickname))
			h_single_nickname = NULL;		// whoops, got singly selected Nickname File item

		selcount = 1;
	}
	else
	{
		//
		// Do a full blown analysis by walking the tree items in search
		// of selected items.  We can stop early if we have seen enough
		// to determine which combination of items have been selected.
		//
		BOOL stop = FALSE;
		HTREEITEM h_parent = GetRootItem();
		while ((h_parent != NULL) && (! stop))
		{
			if (IsItemSelected(h_parent))
				selcount++;

			if (IsItemExpanded(h_parent))
			{
				//
				// Walk the nicknames in the nicknames file.
				//
				HTREEITEM h_child = GetChildItem(h_parent);
				while ((h_child != NULL) && (! stop))
				{
					if (IsItemSelected(h_child))
					{
						selcount++;

						if (1 == selcount)
						{
							//
							// Potential single-selection case, so save the 
							// handle to the item if it is a nickname.
							//
							ASSERT(GetSafeNickname(h_child) != NULL);
							h_single_nickname = h_child;
						}
						else
						{
							//
							// False alarm.  Never mind.
							//
							h_single_nickname = NULL;
						}

						//
						// Process selected nickname item using a state
						// machine that terminates when it sees at least two
						// items and one or more of those items were
						// "unchecked".
						//
						CNickname* p_nickname = GetSafeNickname(h_child);
						ASSERT(p_nickname != NULL);
						switch (hint)
						{
						case NICKNAME_HINT_MULTISELECT_RECIPLIST_DISABLE:
							if (p_nickname->IsRecipient())
								hint = NICKNAME_HINT_MULTISELECT_RECIPLIST_CHECK;
							else
								hint = NICKNAME_HINT_MULTISELECT_RECIPLIST_UNCHECK;
							break;
						case NICKNAME_HINT_MULTISELECT_RECIPLIST_CHECK:
							if (! p_nickname->IsRecipient())
								hint = NICKNAME_HINT_MULTISELECT_RECIPLIST_UNCHECK;
							break;
						case NICKNAME_HINT_MULTISELECT_RECIPLIST_UNCHECK:
							ASSERT(selcount > 1);
							stop = TRUE;		// FORNOW, comment out to see accurate selcount
							break;
						default:
							ASSERT(0);
							break;
						}
					}

					h_child = GetNextSiblingItem(h_child);
				}
			}

			h_parent = GetNextSiblingItem(h_parent);
		}
		
		//FORNOWTRACE1("CNicknamesTreeCtrl::EvaluateSelection: selcount = %d\n", selcount);
		//FORNOWTRACE0("CNicknamesTreeCtrl::EvaluateSelection: finished traversal\n");
	}

	//
	// PART TWO.  Okay, now we need to figure out whether we're 
	// dealing with a multi-select or a single select case, then
	// call UpdateAllViews() to properly broadcast the results of 
	// the evaluation.
	//
	switch (selcount)
	{
	case 0:
		//
		// Default is to broadcast "unknown" multi-selection state.
		//
		m_LastEvaluatedSelectionState = SELSTATE_NONE;
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MULTISELECT_RECIPLIST_DISABLE, NULL);
		break;
	case 1:
		m_LastEvaluatedSelectionState = SELSTATE_SINGLE;
		if (h_single_nickname != NULL)
		{
			// Get CNickname object
			CNickname* p_nickname = GetSafeNickname(h_single_nickname);
			ASSERT(p_nickname != NULL);

			// Get CNicknameFile object (parent)
			HTREEITEM h_parent = GetParentItem(h_single_nickname);
			ASSERT(h_parent != NULL);
			CNicknameFile* p_nickfile = GetSafeNicknameFile(h_parent);
			ASSERT(p_nickfile != NULL);

			if (p_nickname->NeedsReadIn())
				p_nickfile->ReadNickname(p_nickname);

			//
			// Now update the display based on the singly selected nickname.
			//
			if (p_nickfile->m_ReadOnly)
				g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_DISPLAYNICKNAME_READONLY, p_nickname);
			else
				g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_DISPLAYNICKNAME, p_nickname);
		}
		else
		{
			//
			// Broadcast default "unknown" selection state for singly 
			// selected nickname file item.
			//
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MULTISELECT_RECIPLIST_DISABLE, NULL);
		}
		break;
	default:
		//
		// Now update the display based on the multiple selection.
		//
		m_LastEvaluatedSelectionState = SELSTATE_MULTI;
		g_Nicknames->UpdateAllViews(NULL, hint, NULL);
		break;
	}

#ifdef FORNOW_DEBUG
	{
		HTREEITEM h_selitem = GetSelectedItem();
		if (h_selitem)
		{
			CNickname* p_nickname = GetSafeNickname(h_selitem);
			if (p_nickname)
				TRACE2("CNicknamesTreeCtrl::EvaluateSelection: selection=%s, state=%d\n", (const char *) p_nickname->GetName(), m_LastEvaluatedSelectionState);
			else
			{
				CNicknameFile* p_nickfile = GetSafeNicknameFile(h_selitem);
				ASSERT(p_nickfile);
				TRACE2("CNicknamesTreeCtrl::EvaluateSelection: selection=%s, state=%d\n", (const char *) p_nickfile->m_Name, m_LastEvaluatedSelectionState);
			}
		}
		else
		{
			TRACE1("CNicknamesTreeCtrl::EvaluateSelection: selection=<NADA>, state=%d\n", m_LastEvaluatedSelectionState);
		}
	}
#endif // FORNOW_DEBUG
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
void CNicknamesTreeCtrl::OnEscKeyPressed(void)
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

	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		if (m_pDragImage)
		{
			m_pDragImage->DragLeave(this);
			m_pDragImage->EndDrag();

			if (GetCapture() == this)
				ReleaseCapture();

			KillAutoScrollTimer();

			SelectDropTarget(NULL);
			m_pDragImage = NULL;
			ASSERT(m_hDragItem != NULL);
			m_hDragItem = NULL;
			::SetCursor(::LoadCursor(0, IDC_ARROW));
		}

		m_SelectedItemArray.RemoveAll();
	}
}


////////////////////////////////////////////////////////////////////////
// EvaluateSelectionForRecipients [private]
//
// Analyze the selection and return a status code based on whether
// the selected items are on the quick recipient list.
//
// Returns:
//		RL_NONE 	No items selected
//		RL_ON		All selected items are already on the list
//		RL_OFF		None of the selected items are on the list
//		RL_BOTH		Some are on, some are not.  
//
////////////////////////////////////////////////////////////////////////
UINT CNicknamesTreeCtrl::EvaluateSelectionForRecipients()
{
	ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);

	switch (m_LastEvaluatedSelectionState)
	{
	case SELSTATE_NONE:
		return RL_NONE;
	case SELSTATE_SINGLE:
		{
			CNickname* p_nickname = GetSafeNickname(GetSelectedItem());
			if (p_nickname != NULL)
			{
				if (p_nickname->IsRecipient())
					return RL_ON;
				else
					return RL_OFF;
			}
			return RL_NONE;		// singly-selected nickname file
		}
	case SELSTATE_UNKNOWN:
	case SELSTATE_MULTI:
		{
			//
			// Hard case.  Do a full blown analysis by walking the
			// tree in search of selected items.  We can stop early if
			// we find a multi selection that contains items that are
			// both on and off the recipient menu.
			//
			UINT flag = RL_NONE;
			BOOL stop = FALSE;
			HTREEITEM h_nickfile = GetRootItem();
			while ((h_nickfile != NULL) && (flag != RL_BOTH))
			{
				if (IsItemExpanded(h_nickfile))
				{
					//
					// Walk the nicknames in the nicknames file.
					//
					HTREEITEM h_nickname = GetChildItem(h_nickfile);
					while ((h_nickname != NULL) && (flag != RL_BOTH))
					{
						if (IsItemSelected(h_nickname))
						{
							//
							// Process selected nickname item using a state
							// machine that terminates when it sees at least two
							// items and one or more of those items were
							// "unchecked".
							//
							CNickname* p_nickname = GetSafeNickname(h_nickname);
							ASSERT(p_nickname != NULL);
							if (p_nickname->IsRecipient())
								flag |= RL_ON;
							else
								flag |= RL_OFF;
						}

						h_nickname = GetNextSiblingItem(h_nickname);
					}
				}

				h_nickfile = GetNextSiblingItem(h_nickfile);
			}
			return flag;
		}
		ASSERT(0);		// should never get here
	default:
		ASSERT(0);
		return RL_NONE;
	}

	ASSERT(0);			// should never get here
	return RL_NONE;
} 


////////////////////////////////////////////////////////////////////////
// EvaluateSelectionForRename [private]
//
// Returns TRUE if the current tree selection supports the Rename
// operation.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::EvaluateSelectionForRename() const
{
	ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);

	switch (m_LastEvaluatedSelectionState)
	{
	case SELSTATE_NONE:
		return FALSE;
	case SELSTATE_SINGLE:
		{
			HTREEITEM h_selitem = GetSelectedItem();
			CNickname* p_nickname = GetSafeNickname(h_selitem);
			if (p_nickname != NULL)
			{
				//
				// Check to see if the nickname is in a read-only file.
				//
				HTREEITEM h_parent = GetParentItem(h_selitem);
				ASSERT(h_parent != NULL);
				CNicknameFile* p_nickfile = GetSafeNicknameFile(h_parent);
				ASSERT(p_nickfile != NULL);
				if (! p_nickfile->m_ReadOnly)
					return TRUE;
			}
		}

		//
		// If we get this far, we've got a nickname file, or a 
		// nickname in a read-only nickname file.
		//
		return FALSE;
	case SELSTATE_UNKNOWN:
	case SELSTATE_MULTI:
		return FALSE;
	default:
		ASSERT(0);
		return FALSE;
	}

	ASSERT(0);			// should never get here
	return FALSE;
} 


////////////////////////////////////////////////////////////////////////
// OnItemExpanding [protected]
//
// Handles CTreeCtrl "is about to be expanded/collapsed" notification
// message by checking to see if the nickname item data needs to get
// loaded into the tree control.  For performance and memory reasons,
// the nickname items in a nickname file are loaded only "on demand".
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* p_treeview = (NM_TREEVIEW * )pNMHDR;	// type cast
	ASSERT(p_treeview);

	CNicknameFile* p_nickfile = GetSafeNicknameFile(p_treeview->itemNew.hItem);
	ASSERT(p_nickfile != NULL);

	switch (p_treeview->action)
	{
	case TVE_COLLAPSE:
		break;
	case TVE_EXPAND:
		PopulateNicknameFileNode(p_treeview->itemNew.hItem);
		break;
	default:
		ASSERT(0);
		break;
	}

	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// OnItemExpanded [protected]
//
// Handles CTreeCtrl expanded/collapsed notification message by updating
// the graphic associated with the changed tree item.  The BIG assumption
// here is that only "parent" items (i.e. nickname file items) will get
// expand/collapse messages.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* p_treeview = (NM_TREEVIEW * )pNMHDR;	// type cast
	ASSERT(p_treeview);

	CNicknameFile* p_nickfile = GetSafeNicknameFile(p_treeview->itemNew.hItem);
	ASSERT(p_nickfile != NULL);

	switch (p_treeview->action)
	{
	case TVE_COLLAPSE:
		if (p_nickfile->m_ReadOnly)
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_CLOSED_FOLDER_READONLY, IMAGE_CLOSED_FOLDER_READONLY);
		else
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_CLOSED_FOLDER, IMAGE_CLOSED_FOLDER);
		DoDeselectAllChildren(p_treeview->itemNew.hItem);
		EvaluateSelection();
		break;
	case TVE_EXPAND:
		if (p_nickfile->m_ReadOnly)
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_OPEN_FOLDER_READONLY, IMAGE_OPEN_FOLDER_READONLY);
		else
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_OPEN_FOLDER, IMAGE_OPEN_FOLDER);
		break;
	default:
		ASSERT(0);
		break;
	}

	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// OnSelChanged [protected]
//
// Handles CTreeCtrl "selection has already changed" notification message
// by analyzing which items are selected and then broadcasting the 
// appropriate "update" notification to the view at large.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	//
	// The following is a hack to grab focus away from any RHS edit
	// controls.  Under NT 4.0, there is a race condition where the
	// RHS edit controls don't receive a kill focus notification until
	// after the LHS selection processing is complete.  I don't know
	// why, but this race condition doesn't seem to exist under previous
	// versions of Windows.
	//
	SetFocus();

	EvaluateSelection();
	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// OnLButtonDown [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	//
	// Do hit test based on cursor position.
	//
	UINT flags = 0;
	HTREEITEM h_item = HitTest(point, &flags);
	if ((h_item != NULL) && (flags & TVHT_ONITEM))
	{
		// Shift-select takes precedence over Ctrl-select
		if (nFlags & MK_SHIFT)
		{
			//
			// First, deselect everything.
			//
			HTREEITEM h_save_anchor = m_hAnchorItem;
			DoDeselectAll();
			ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
			m_EvaluateSelectionHint = EVALHINT_SKIP;
			QCTreeCtrl::OnLButtonDown(nFlags, point);
			m_EvaluateSelectionHint = EVALHINT_FULL;

			//
			// Then, select everything between the anchor point and
			// the current point.
			//
			if (h_save_anchor)
			{
				SetRedraw(FALSE);
				DoSelectItemRange(h_save_anchor, h_item);
				SetRedraw(TRUE);
				EvaluateSelection();		// same sob story about doing work twice

				//
				// Save a list of selected items in preparation
				// for a potential drag and drop operation.
				//
				DoSaveSelection(TRUE);

				return;
			}
			ASSERT(0);		// should this ever happen?
		}
		else if (nFlags & MK_CONTROL)
		{
			//
			// Toggle the selection state.
			//
			if (IsItemSelected(h_item))
			{
				//
				// Save the handle to the item which the tree control
				// believes to be the singly-selected item, if any.  Note that
				// GetSelectedItem() returns the item with the focus
				// caret, even if it doesn't have a selection "highlight".
				//
				HTREEITEM h_selitem = GetSelectedItem();
				if (! IsItemSelected(h_selitem))
					h_selitem = NULL;

				DoSelectItem(h_item, FALSE);
				ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
				m_EvaluateSelectionHint = EVALHINT_SKIP;
				QCTreeCtrl::OnLButtonDown(nFlags, point);
				m_EvaluateSelectionHint = EVALHINT_FULL;

				//
				// Something very weird happens when calling
				// CTreeCtrl::OnLButtonDown().  That is, this call
				// doesn't return until *after* the user has *released* the
				// mouse button.  Plus, to make things even worse, the
				// tree control processes the TVN_SELCHANGED
				// notification before OnLButtonDown() returns.  Since
				// the TVN_SELCHANGED processing occurs before we have
				// a chance to restore the selection, it produces the
				// wrong results!  Therefore, we must call the
				// TVN_SELCHANGED handler (EvaluateSelection()) again.
				// Whatta waste.
				//
				if (h_selitem && (h_selitem != h_item))
					DoSelectItem(h_selitem, TRUE);			// restores lost "single selection"
				DoSelectItem(h_item, FALSE);				// turn off selection of a selected item
				EvaluateSelection();
			}
			else
			{
				//
				// Save the handle to the item which the tree control
				// believes to be the singly-selected item, if any.  Note that
				// GetSelectedItem() returns the item with the focus
				// caret, even if it doesn't have a selection "highlight".
				//
				HTREEITEM h_selitem = GetSelectedItem();
				if (! IsItemSelected(h_selitem))
					h_selitem = NULL;

				ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
				m_EvaluateSelectionHint = EVALHINT_SKIP;
				QCTreeCtrl::OnLButtonDown(nFlags, point);
				m_EvaluateSelectionHint = EVALHINT_FULL;

				//
				// Something very weird happens when calling
				// CTreeCtrl::OnLButtonDown().  That is, this call
				// doesn't return until *after* the user has *released* the
				// mouse button.  Plus, to make things even worse, the
				// tree control processes the TVN_SELCHANGED
				// notification before OnLButtonDown() returns.  Since
				// the TVN_SELCHANGED processing occurs before we have
				// a chance to restore the selection, it produces the
				// wrong results!  Therefore, we must call the
				// TVN_SELCHANGED handler (EvaluateSelection()) again.
				// Whatta waste.
				//
				if (h_selitem && (h_selitem != h_item))
					DoSelectItem(h_selitem, TRUE);			// restores lost "single selection"
				EvaluateSelection();
			}

			//
			// Save a list of selected items in preparation
			// for a potential drag and drop operation.
			//
			DoSaveSelection(TRUE);

			return;
		}
		else
		{
			BOOL need_fake_selchange = FALSE;

			//
			// Decide whether or not we hit an existing selection or not.
			// If we did, then save a list of selected items in preparation
			// for a potential drag and drop operation.
			//
			if (IsItemSelected(h_item))
				DoSaveSelection(TRUE);

			if (SELSTATE_MULTI == m_LastEvaluatedSelectionState)
			{
				DoDeselectAll();
				HTREEITEM h_selitem = GetSelectedItem();
				if (h_selitem == h_item)
				{
					//
					// Here's a good one.  If we have a multiple selection
					// and the user happens to click on the item that
					// the tree control thinks is the single-selected
					// item, then processing the left button click doesn't
					// generate a TVN_SELCHANGED message -- as far as the
					// tree control knows, there was no selection change
					// caused by reselecting the already selected item.
					// However, we rely on the generated TVN_SELCHANGED 
					// notice to update the state of the RHS view when
					// switching back to a single selection state.  So,
					// we set a flag to note the fact that we need to
					// manually evaluate the selection state.  Got that?
					//
					need_fake_selchange = TRUE;
				}
				DoSelectItem(h_item, TRUE);					// reselect item first
			}
			else if (SELSTATE_NONE == m_LastEvaluatedSelectionState)
			{
				//
				// This the case where we select an item that appears to
				// be deselected, but the tree control really thinks that
				// it is selected since it has the focus caret.  Therefore,
				// we need to toggle the selection state ourselves.
				//
				HTREEITEM h_selitem = GetSelectedItem();
				if (h_selitem == h_item)
				{
					//
					// Here's a good one.  If we have no selection
					// and the user happens to click on the item that
					// the tree control thinks is the single-selected
					// item, then processing the left button click doesn't
					// generate a TVN_SELCHANGED message -- as far as the
					// tree control knows, there was no selection change
					// caused by reselecting the already selected item.
					// However, we rely on the generated TVN_SELCHANGED 
					// notice to update the state of the RHS view when
					// switching back to a single selection state.  So,
					// we set a flag to note the fact that we need to
					// manually evaluate the selection state.  Got that?
					//
					need_fake_selchange = TRUE;
				}
				DoSelectItem(h_selitem, TRUE);
			}

			ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
			m_EvaluateSelectionHint = EVALHINT_SINGLE;		// optimize for single selection
			QCTreeCtrl::OnLButtonDown(nFlags, point);
			if (need_fake_selchange)
				EvaluateSelection();
			m_EvaluateSelectionHint = EVALHINT_FULL;
			return;
		}
	}

	//
	// Didn't hit anything, so interpret this as a deselect.
	//
	DoDeselectAll();
	QCTreeCtrl::OnLButtonDown(nFlags, point);

	//
	// Empirical studies show that the tree control doesn't
	// generate a TVN_SELCHANGE notification unless you hit an
	// item.  Therefore, we need to process a fake selection
	// selection change notice ourselves.
	//
	ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
	LRESULT unused;
	OnSelChanged(NULL, &unused);
}


////////////////////////////////////////////////////////////////////////
// OnLButtonDblClk [protected]
//
// Handle mouse double-click notification message.
//  
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnLButtonDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
	HTREEITEM h_item = GetSelectedItem();

	if (h_item != NULL)
	{
		CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_item);
		ASSERT(p_itemdata != NULL);

		switch (p_itemdata->m_itemType)
		{
		case ITEM_FILE:
			{
				if (! ItemHasChildren(h_item))
				{
					//
					// User wants to open or close an empty folder.  This
					// case requires special handling since the default behavior
					// of the tree control is to sit there and do nothing.
					//
					int image_idx = -1;
					int unused;
					GetItemImage(h_item, image_idx, unused);
					switch (image_idx)
					{
					case IMAGE_OPEN_FOLDER:
						SetItemImage(h_item, IMAGE_CLOSED_FOLDER, IMAGE_CLOSED_FOLDER);
						break;
					case IMAGE_CLOSED_FOLDER:
						{
							SetItemImage(h_item, IMAGE_OPEN_FOLDER, IMAGE_OPEN_FOLDER);
							PopulateNicknameFileNode(h_item);
						}
						break;
					case IMAGE_OPEN_FOLDER_READONLY:
						SetItemImage(h_item, IMAGE_CLOSED_FOLDER_READONLY, IMAGE_CLOSED_FOLDER_READONLY);
						break;
					case IMAGE_CLOSED_FOLDER_READONLY:
						{
							SetItemImage(h_item, IMAGE_OPEN_FOLDER_READONLY, IMAGE_OPEN_FOLDER_READONLY);
							PopulateNicknameFileNode(h_item);
						}
						break;
					default:
						ASSERT(0);
						break;
					}
				}
			}
			break;
		case ITEM_NICKNAME:
		case ITEM_ADDRESS:
		case ITEM_CUSTOM: // Should only be FULL FEATURE fo custom items
			{
				//
				// Double-clicking on a nickname item is a shortcut for
				// adding the nickname to the currently selected
				// header.
				//
				CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
				if (p_view)
				{
					ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));
					//p_view->OnInsertNickname(HEADER_CURRENT);
					p_view->PostMessage(WM_COMMAND, ID_DBLCLK_NICKNAME);
				}
			}
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////
// OnMouseMove [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// Shareware: Reduced feature mode only allows one nickname file, do drag-n-drop
	if (UsingFullFeatureSet())
	{
		if (m_pDragImage && (MK_LBUTTON & nFlags))
		{
			ASSERT(this == GetCapture());
			ASSERT(m_hDragItem != NULL);
			VERIFY(m_pDragImage->DragMove(point));

			UINT flags = 0;
			HTREEITEM h_target = HitTest(point, &flags);
			m_pDragImage->DragLeave(this);			// allow window updates
			if (h_target != NULL)
			{
				CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_target);
				ASSERT(p_itemdata != NULL);
				switch (p_itemdata->m_itemType)
				{
				case ITEM_FILE:
					{
						//
						// Highlight the drop target directly.  If the target
						// folder is writable, then display a normal cursor,
						// otherwise display a no-drop cursor.
						//
						ASSERT(p_itemdata->m_u.pNicknameFile != NULL);
						ASSERT(p_itemdata->m_u.pNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

						if (p_itemdata->m_u.pNicknameFile->m_ReadOnly)
							::SetCursor(::LoadCursor(0, IDC_NO));
						else
							::SetCursor(::LoadCursor(0, IDC_ARROW));
						SelectDropTarget(h_target);
					}
					break;
				case ITEM_NICKNAME:
				case ITEM_ADDRESS:
				case ITEM_CUSTOM:
					{
						//
						// If the drop target is a nickname, highlight the
						// parent folder.  If the target folder is writable
						// then display a normal cursor, otherwise display
						// a no-drop cursor.
						//
						HTREEITEM h_targetparent = GetParentItem(h_target);
						ASSERT(h_targetparent != NULL);

						CNicknamesTreeItemData* p_targetparent = (CNicknamesTreeItemData * ) GetItemData(h_targetparent);
						ASSERT(p_targetparent != NULL);
						ASSERT(ITEM_FILE == p_targetparent->m_itemType);
						ASSERT(p_targetparent->m_u.pNicknameFile != NULL);
						ASSERT(p_targetparent->m_u.pNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

						if (p_targetparent->m_u.pNicknameFile->m_ReadOnly)
							::SetCursor(::LoadCursor(0, IDC_NO));
						else
							::SetCursor(::LoadCursor(0, IDC_ARROW));
						SelectDropTarget(h_targetparent);
					}
					break;
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
	}

	QCTreeCtrl::OnMouseMove(nFlags, point);
}
  
////////////////////////////////////////////////////////////////////////
// OnLButtonUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		if (m_pDragImage)
		{
			HTREEITEM h_target = GetDropHilightItem();

			m_pDragImage->DragLeave(this);
			m_pDragImage->EndDrag();

			if (GetCapture() == this)
				ReleaseCapture();

			KillAutoScrollTimer();

			SelectDropTarget(NULL);
			m_pDragImage = NULL;

			ASSERT(m_hDragItem != NULL);
			if (h_target != NULL)
			{
				//
				// Validate that the drop target is a writable nickname file item.
				//
				CNicknamesTreeItemData* p_targetitem = (CNicknamesTreeItemData * ) GetItemData(h_target);
				ASSERT(p_targetitem != NULL);
				ASSERT(p_targetitem->m_u.pNicknameFile != NULL);
				ASSERT(p_targetitem->m_u.pNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

				if (! p_targetitem->m_u.pNicknameFile->m_ReadOnly)
				{
					switch (p_targetitem->m_itemType)
					{
					case ITEM_FILE:
						{
							//
							// If there was no saved selection, then use the
							// drag item.
							//
							if (-1 == m_SelectedItemArray.GetUpperBound())
								m_SelectedItemArray.Add(DWORD(m_hDragItem));

							//
							// Populate a list containing nickname/nickname file
							// pairs, based on the selected tree items in
							// m_SelectedItemArray.
							//
							CObList nickname_list;
							for (int i = 0; i <= m_SelectedItemArray.GetUpperBound(); i++)
							{
								HTREEITEM h_dragitem = HTREEITEM(m_SelectedItemArray.GetAt(i));

								//
								// Convert the dragged item handle into a corresponding
								// CNickname pointer.  We are guaranteed that the type
								// of the dragged item is not a nickname file item (see
								// OnBeginDrag()).
								//
								CNicknamesTreeItemData* p_dragitem = (CNicknamesTreeItemData *) GetItemData(h_dragitem);
								ASSERT(p_dragitem != NULL);
								ASSERT(p_dragitem->m_itemType != ITEM_FILE);
								ASSERT(p_dragitem->m_u.pNickname != NULL);
								ASSERT(p_dragitem->m_u.pNickname->IsKindOf(RUNTIME_CLASS(CNickname)));

								//
								// We are guaranteed that the m_SelectedItemArray does not
								// contain any nickname file handles ... see OnLButtonDown().
								// So, grab the parent CNicknameFile item for the dragged CNickname.
								//
								HTREEITEM h_dragparentitem = GetParentItem(h_dragitem);
								ASSERT(h_dragparentitem);
								CNicknamesTreeItemData* p_dragparentitem = (CNicknamesTreeItemData *) GetItemData(h_dragparentitem);
								ASSERT(p_dragparentitem != NULL);
								ASSERT(p_dragparentitem->m_itemType == ITEM_FILE);
								ASSERT(p_dragparentitem->m_u.pNicknameFile != NULL);
								ASSERT(p_dragparentitem->m_u.pNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

								//
								// Create a CNickname/CNicknameFile pair inside an CObList
								// wrapper, as expected by the move/copy nickname function.
								//
								// WARNING! WARNING!  The CObList::AddTail() method is
								// overloaded to accept CObList ptrs as well as CObject
								// ptrs, with decidely different behaviors.  Therefore,
								// we need to cast the CNicknameFile to a CObject to force
								// the behavior we want.
								//
								nickname_list.AddTail(p_dragitem->m_u.pNickname);
								nickname_list.AddTail((CObject *) p_dragparentitem->m_u.pNicknameFile);
							}

							//
							// Let the parent view do the actual nicknames move/copy.
							//
							CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
							if (p_view)
							{
								ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));
								p_view->MoveOrCopyNicknames((nFlags & MK_CONTROL) == 0 && (nFlags & MK_SHIFT) == 0, 
															nickname_list,
															p_targetitem->m_u.pNicknameFile);
							}
						}
						break;
					case ITEM_NICKNAME:
					case ITEM_ADDRESS:
					case ITEM_CUSTOM:
						ASSERT(0);
						::MessageBeep(MB_ICONASTERISK);
						break;
					default:
						ASSERT(0);
						break;
					}
				}
				else
				{
					// beep on attempts to move/copy to read-only files
					::MessageBeep(MB_OK);
				}
			}

			m_hDragItem = NULL;
		}
	}

	TRACE0("CNicknamesTreeCtrl::OnLButtonUp: clearing m_SelectedItemArray\n");
	m_SelectedItemArray.RemoveAll();

	::SetCursor(::LoadCursor(0, IDC_ARROW));

	QCTreeCtrl::OnLButtonUp(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// OnInitMenuPopup [protected]
//
// Set the enable/disable state of the context menu items, based on
// whether the selected nicknames are already on or off the quick
// recipient menu.
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	// By default, all menu items are enabled.

	//CTreeCtrl::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	UINT nFlag = EvaluateSelectionForRecipients();
	
	switch (nFlag)
	{
	case RL_BOTH:
		break;
	case RL_ON:
		pPopupMenu->EnableMenuItem(ID_POPUP_ADD_RECIPIENT, MF_GRAYED | MF_BYCOMMAND);
		break;
	case RL_OFF:
		pPopupMenu->EnableMenuItem(ID_POPUP_REMOVE_RECIPIENT, MF_GRAYED | MF_BYCOMMAND);
		break;
	case RL_NONE:
		pPopupMenu->EnableMenuItem(ID_POPUP_ADD_RECIPIENT, MF_GRAYED | MF_BYCOMMAND);
		pPopupMenu->EnableMenuItem(ID_POPUP_REMOVE_RECIPIENT, MF_GRAYED | MF_BYCOMMAND);
		break;
	default:
		ASSERT(0);
		break;
	}

	// Disable the Rename menu item if the selection is inapproppriate
	if (! EvaluateSelectionForRename())
		pPopupMenu->EnableMenuItem(ID_POPUP_RENAME, MF_GRAYED | MF_BYCOMMAND);

	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE
		if (g_Nicknames)
		{
			const int nNUM_WRITABLE = g_Nicknames->GetNumWritableFiles();
			
			if (nNUM_WRITABLE < 2)
				pPopupMenu->EnableMenuItem(ID_POPUP_MOVE_TO, MF_GRAYED | MF_BYCOMMAND);
			if (nNUM_WRITABLE < 1)
				pPopupMenu->EnableMenuItem(ID_POPUP_COPY_TO, MF_GRAYED | MF_BYCOMMAND);
		}
	}
	else
	{
		// RECDUCED FEATURE -- no COPY/MOVE TO
		pPopupMenu->EnableMenuItem(ID_POPUP_MOVE_TO, MF_GRAYED | MF_BYCOMMAND);
		pPopupMenu->EnableMenuItem(ID_POPUP_COPY_TO, MF_GRAYED | MF_BYCOMMAND);
	}
}


////////////////////////////////////////////////////////////////////////
// OnRightButtonDown [protected]
//
////////////////////////////////////////////////////////////////////////
long CNicknamesTreeCtrl::OnRightButtonDown(WPARAM wParam, LPARAM lParam)
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

	SetFocus();

	if ((tvhit.hItem != NULL) && (tvhit.flags & TVHT_ONITEM))
	{
		if (! IsItemSelected(tvhit.hItem))
		{
			//
			// We hit an unselected item, so select it first
			// before processing the context menu stuff.
			//
			DoDeselectAll();
			SelectItem(tvhit.hItem);
		}
	}
	else
	{
		//
		// Didn't hit anything, so deselect everything.
		//
		DoDeselectAll();

		//
		// Empirical studies show that the tree control doesn't
		// generate a TVN_SELCHANGE notification unless you hit an
		// item.  Therefore, we need to process a fake selection
		// selection change notice ourselves.
		//
		ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
		LRESULT unused;
		OnSelChanged(NULL, &unused);
	}

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
void CNicknamesTreeCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE
		NM_TREEVIEW* p_nmtv = (NM_TREEVIEW * )pNMHDR;	// type cast
		ASSERT(p_nmtv);

		HTREEITEM h_item = p_nmtv->itemNew.hItem;
		ASSERT(h_item != NULL);
		CNicknamesTreeItemData* p_dataitem = (CNicknamesTreeItemData *) (p_nmtv->itemNew.lParam);
		ASSERT(p_dataitem != NULL);
		Select(h_item, TVGN_CARET);
		m_EvaluateSelectionHint = EVALHINT_SKIP;
		SelectItem(NULL);	// clear the selection, if any
		m_EvaluateSelectionHint = EVALHINT_FULL;
		UpdateWindow();
		switch (p_dataitem->m_itemType)
		{
		case ITEM_FILE:
			::MessageBeep(MB_OK);			// FORNOW, should disallow only if single-selection
			break;
		case ITEM_NICKNAME:
		case ITEM_ADDRESS:
		case ITEM_CUSTOM:
			{
				//
				// Setup drag image.
				//
				ASSERT(NULL == m_pDragImage);
				ASSERT(NULL == m_hDragItem);
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
					m_hDragItem = h_item;

					StartAutoScrollTimer();
				}
			}
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	if (GetCapture() == this)
		ReleaseCapture();

	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
	CContextMenu(MP_POPUP_NICKNAME_LIST, ptScreen.x, ptScreen.y, FromHandle(m_hWnd));
}


////////////////////////////////////////////////////////////////////////
// OnKeyDown [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DOWN:		// up arrow
	case VK_UP:			// down arrow
	case VK_PRIOR:		// PgUp
	case VK_NEXT:		// PgDn
	case VK_HOME:		// Home
	case VK_END:		// End
		if (ShiftDown())
		{
			//
			// Extended selection case.
			//
			HTREEITEM h_save_anchor = m_hAnchorItem;
			DoDeselectAll();		// clears selection
			ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
			m_EvaluateSelectionHint = EVALHINT_SKIP;
			QCTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			m_EvaluateSelectionHint = EVALHINT_FULL;
			if (h_save_anchor)
			{
				SetRedraw(FALSE);
				DoSelectItemRange(h_save_anchor, GetSelectedItem());
				SetRedraw(TRUE);
				EvaluateSelection();		// same sob story about doing work twice

				//
				// As a kludge for keyboard-based selections, must 
				// reset the anchor item to be the previous anchor, 
				// overriding the normal mouse-driven behavior of 
				// EvaluateSelection().
				// 
				m_hAnchorItem = h_save_anchor;
			}
			else
			{
				ASSERT(0);		// in theory, this shouldn't happen
			}
		}
		else
		{
			//
			// Single-selection case.
			//
			if (SELSTATE_MULTI == m_LastEvaluatedSelectionState)
				DoDeselectAll();
			
			ASSERT(EVALHINT_FULL == m_EvaluateSelectionHint);
			m_EvaluateSelectionHint = EVALHINT_SINGLE;		// optimize for single selection
			QCTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			m_EvaluateSelectionHint = EVALHINT_FULL;
		}
		break;
	case VK_LEFT:		// left arrow
	case VK_RIGHT:		// right arrow
		//
		// Do special keystroke handling for empty nickname files.
		//
		if (SELSTATE_SINGLE == m_LastEvaluatedSelectionState)
		{
			HTREEITEM h_item = GetSelectedItem();
			ASSERT(h_item != NULL);
			CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(h_item);
			ASSERT(p_itemdata != NULL);
			if ((ITEM_FILE == p_itemdata->m_itemType) &&
				(! ItemHasChildren(h_item)))
			{
				//
				// User wants to open or close an empty folder.  This
				// case requires special handling since the default behavior
				// of the tree control is to sit there and do nothing.
				//
				int image_idx = -1;
				int unused;
				GetItemImage(h_item, image_idx, unused);
				switch (image_idx)
				{
				case IMAGE_OPEN_FOLDER:
					if (VK_LEFT == nChar)
						SetItemImage(h_item, IMAGE_CLOSED_FOLDER, IMAGE_CLOSED_FOLDER);
					break;
				case IMAGE_OPEN_FOLDER_READONLY:
					if (VK_LEFT == nChar)
						SetItemImage(h_item, IMAGE_CLOSED_FOLDER_READONLY, IMAGE_CLOSED_FOLDER_READONLY);
					break;
				case IMAGE_CLOSED_FOLDER:
					if (VK_RIGHT == nChar)
					{
						SetItemImage(h_item, IMAGE_OPEN_FOLDER, IMAGE_OPEN_FOLDER);
						PopulateNicknameFileNode(h_item);
					}
					break;
				case IMAGE_CLOSED_FOLDER_READONLY:
					if (VK_RIGHT == nChar)
					{
						SetItemImage(h_item, IMAGE_OPEN_FOLDER_READONLY, IMAGE_OPEN_FOLDER_READONLY);
						PopulateNicknameFileNode(h_item);
					}
					break;
				default:
					ASSERT(0);
					break;
				}
			}
		}

		//
		// Do default keystroke handling.
		//
		QCTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	default:
		QCTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnKeyUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//
	// Do default keystroke handling.
	//
	QCTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);

	if (VK_DELETE == nChar)
	{
		//
		// Pressing the Del key is a shortcut for clicking the Remove
		// button.
		//
		CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
		if (p_view)
		{
			ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));
			p_view->AutoClickRemoveButton();
		}
	}
	else if (VK_F2 == nChar)
	{
		//
		// Shortcut for the rename command.
		//
		OnRename();
	}
}


////////////////////////////////////////////////////////////////////////
// NicknamesTreeEditLabelProc [extern]
//
// This is the window procedure used to subclass the temporary edit
// control created for editing tree control labels.  This subclassing
// is necessary to work around a known bug where tree control edit
// controls do not receive the Enter and Esc keystrokes since they
// are hooked by the parent dialog Proc.
//
////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK NicknamesTreeEditLabelProc(HWND hWnd, WORD wMessage, WORD wParam, LONG lParam)
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
// OnBeginLabelEdit [protected]
//
// Handles CTreeCtrl notification that the label is about to be edited.
// We do not allow "persistent" nodes such as the In, Out, and Trash
// mailboxes to be edited.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* p_dispinfo = (TV_DISPINFO * )pNMHDR;	// type cast
	ASSERT(p_dispinfo);

	CNicknamesTreeItemData* p_dataitem = (CNicknamesTreeItemData *) (p_dispinfo->item.lParam);
	ASSERT(p_dataitem != NULL);
	switch (p_dataitem->m_itemType)
	{
	case ITEM_FILE:			// FORNOW, don't allow filenames to be edited
	case ITEM_ADDRESS:
	case ITEM_CUSTOM: // Custom should only be FULL FEATURE mode
		::MessageBeep(MB_OK);
		*pResult = 1;		// cancel editing
		break;
	case ITEM_NICKNAME:
		{
			// Get CNicknameFile object (parent)
			HTREEITEM h_parent = GetParentItem(p_dispinfo->item.hItem);
			ASSERT(h_parent != NULL);
			CNicknameFile* p_nickfile = GetSafeNicknameFile(h_parent);
			ASSERT(p_nickfile != NULL);

			//
			// If the nickname is in a read-only file, then don't
			// allow a rename action.
			//
			if (p_nickfile->m_ReadOnly)
			{
				::MessageBeep(MB_OK);
				*pResult = 1;		// cancel editing
			}
			else
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
					// and Windows NT 4.x, so don't do it.  The
					// bummer is that Enter/Esc won't work under NT until
					// 1) Microsoft fixes the tree control for NT 4.x, or
					// 2) we figure out another workaround for NT.
					//
					ASSERT(NULL == s_pOldEditProc);
					ASSERT(NULL == s_pTreeCtrl);
					m_bInEditMode = TRUE;
				}
				else
				{
					HWND h_editctrl = HWND(SendMessage(TVM_GETEDITCONTROL));
					ASSERT(h_editctrl != NULL);
					ASSERT(NULL == s_pOldEditProc);
					s_pOldEditProc = WNDPROC(SetWindowLong(h_editctrl, GWL_WNDPROC, long(NicknamesTreeEditLabelProc)));
					ASSERT(s_pOldEditProc != NULL);
					ASSERT(NULL == s_pTreeCtrl);
					s_pTreeCtrl = this;
				}
				
				*pResult = 0;		// allow editing
			}
		}
		break;
	default:
		ASSERT(0);
		*pResult = 1;		// disallow editing
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnEndLabelEdit [protected]
//
// Handles CTreeCtrl notification that the label editing is complete.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* p_dispinfo = (TV_DISPINFO * )pNMHDR;	// type cast
	ASSERT(p_dispinfo);

	CString newname;
	BOOL is_newname_ok = TRUE;		// innocent until proven guilty

	CNicknamesTreeItemData* p_dataitem = (CNicknamesTreeItemData *) (p_dispinfo->item.lParam);
	ASSERT(p_dataitem != NULL);
	switch (p_dataitem->m_itemType)
	{
	case ITEM_FILE:
	case ITEM_ADDRESS:
	case ITEM_CUSTOM: // Custom should only be FULL FEATURE mode
		ASSERT(0);
		break;
	case ITEM_NICKNAME:
		{
			if (p_dispinfo->item.pszText)
			{
				newname = p_dispinfo->item.pszText;

				if (newname.IsEmpty())
				{
					::MessageBeep(MB_OK);
					is_newname_ok = FALSE;
				}
				else if (newname.GetLength() > 32)
				{
					ErrorDialog(IDS_ERR_NICKNAME_NAME_TOO_LONG);
					is_newname_ok = FALSE;
				}
				else
				{
					CString oldname = GetItemText(p_dispinfo->item.hItem);

					// Treat same name just like cancel
					if (newname != oldname)
					{
						//
						// Create a dummy nickname object in order to
						// check the validity of the proposed name.
						//
						CNickname dummy(newname);
						if (! dummy.HasValidName())
						{
							ErrorDialog(IDS_ERR_NICK_INVALID_CHAR);
							is_newname_ok = FALSE;
						}
						else
						{
							// Test for duplicate nickname in this nickfile file
							CString duplicate(newname);
							CheckForDuplicateNickname(&duplicate);
							if (duplicate.IsEmpty())
							{
								ErrorDialog(IDS_ERR_NICK_NAME_USED);
								is_newname_ok = FALSE;
							}
							else
							{
								//
								// The moment we have all been waiting for...
								// do the rename and update the RHS.
								//
								// Handle recipient list status, if necessary, by removing
								// the old name then adding it back with the new name.
								// Then, rebuild the recipient menu if necessary.
								//
								// FORNOW, this code was copied from 
								// CNicknamesViewRight::OnUpdate().  This should probably
								// be placed in a central spot to ease future maintenance.
								//
								CNickname* p_nickname = GetSafeNickname(p_dispinfo->item.hItem);
								ASSERT(p_nickname != NULL);
								ASSERT(p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));
								QCRecipientCommand*	pCommand;

								pCommand = g_theRecipientDirector.Find( p_nickname->GetName() );

								if( pCommand != NULL )
								{
									pCommand->Execute( CA_RENAME, ( void* ) (LPCSTR) newname );
								}
								//
								// Rename the nickname.
								//
								p_nickname->SetName(newname);				
								MarkSelectedNicknameDirty();
								if (g_Nicknames)
								{
									g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_NICKNAME_WAS_CHANGED, p_nickname);
									g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_DISPLAYNICKNAME, p_nickname);
								}
							}
						}
					}
				}
			}

			
			//
			// Workaround known bug with Tree Controls used in dialogs ...
			// Restore the temporarily subclassed edit control.
			//
			if ( IsWinNT() ) 
			{
				m_bInEditMode = FALSE;
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
	default:
		ASSERT(0);
		break;
	}

	if (! is_newname_ok)
	{
		// keep the tree control in edit mode, using the edited name
		SendMessage(TVM_EDITLABEL, 0, LPARAM(p_dispinfo->item.hItem));
		CEdit* p_edit = (CEdit *) FromHandle(HWND(SendMessage(TVM_GETEDITCONTROL)));
		p_edit->SetWindowText(newname);
		p_edit->SetSel(0, -1);
	}

	*pResult = 0;
}


////////////////////////////////////////////////////////////////////////
// AddItem [private]
//
// The 'itemType' and 'hParentItem' parameters determine where the new
// item gets added to the tree.  Returns handle of newly added item
// if successful, else NULL on error.
////////////////////////////////////////////////////////////////////////
HTREEITEM CNicknamesTreeCtrl::AddItem
(
	ItemType itemType,					//(i) item type
	const CString& itemName,			//(i) item name
	BOOL isRecipList,					//(i) should be FALSE for ITEM_FILE
	void* pItemData,					//(i) for CNicknamesTreeItemData record
	HTREEITEM hParentItem /*=NULL*/		//(i) used only if itemType != ITEM_FILE
)		
{
	TV_INSERTSTRUCT tvstruct;

	//
	// Initialize record.
	//
	tvstruct.hParent = NULL;			// normally overwritten later
	tvstruct.hInsertAfter = TVI_LAST/*FORNOWTVI_SORT*/;	// can be overwritten later
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
	CNicknamesTreeItemData* p_itemdata = new CNicknamesTreeItemData(itemType, pItemData);
	if (p_itemdata != NULL)
		tvstruct.item.lParam = long(p_itemdata);	// cast to long
	else
		return FALSE;		// out of memory

	//
	// Then, handle item-specific stuff.
	//
	if (isRecipList)
		tvstruct.item.state = TVIS_BOLD;

	switch (itemType)
	{
	case ITEM_FILE:
		ASSERT(FALSE == isRecipList);
		ASSERT(NULL == hParentItem);
		tvstruct.hParent = TVI_ROOT;
		tvstruct.hInsertAfter = GetFileInsertPos(itemName);
		{
			CNicknameFile* p_nickfile = (CNicknameFile *) pItemData;	// type cast
			ASSERT(p_nickfile && p_nickfile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));
			if (p_nickfile->m_ReadOnly)
			{
				tvstruct.item.iImage = IMAGE_CLOSED_FOLDER_READONLY;
				tvstruct.item.iSelectedImage = IMAGE_CLOSED_FOLDER_READONLY;
			}
			else
			{
				tvstruct.item.iImage = IMAGE_CLOSED_FOLDER;
				tvstruct.item.iSelectedImage = IMAGE_CLOSED_FOLDER;
			}
		}
		break;
	case ITEM_NICKNAME:
		ASSERT(hParentItem != NULL);
		tvstruct.hParent = hParentItem;
		tvstruct.item.iImage = IMAGE_NICKNAME;
		tvstruct.item.iSelectedImage = IMAGE_NICKNAME;
		break;
	case ITEM_ADDRESS:
		ASSERT(hParentItem != NULL);
		tvstruct.hParent = hParentItem;
		tvstruct.item.iImage = IMAGE_ADDRESS;
		tvstruct.item.iSelectedImage = IMAGE_ADDRESS;
		break;
	case ITEM_CUSTOM: // Custom should only be FULL FEATURE mode
		{
			// Shareware: Reduced feature mode only allows one nickname file
			if (UsingFullFeatureSet())
			{
				// FULL FEATURE mode
				ASSERT(hParentItem != NULL);
				ASSERT(m_customImageIndex >= IMAGE_CUSTOM);
				tvstruct.hParent = hParentItem;
				tvstruct.item.iImage = m_customImageIndex;
				tvstruct.item.iSelectedImage = m_customImageIndex;
			}
			else
			{
				// REDUCED FEATURE mode
				ASSERT(0);
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	if (NULL == tvstruct.hParent)
	{
		ASSERT(0);		// caller's probably not checking our return values
		return FALSE;
	}

	//
	// Do the insert...
	//
    return InsertItem(&tvstruct);
}


////////////////////////////////////////////////////////////////////////
// PopulateNicknameFileNode [public]
//
// Populates the given nickname file item with the nickname child items.
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::PopulateNicknameFileNode(HTREEITEM hNicknameFile)
{
	//
	// Check to see if this is the first time this nickname file
	// is being opened.
	//
	UINT state = GetItemState(hNicknameFile, TVIS_EXPANDEDONCE);
	if (state & TVIS_EXPANDEDONCE)
		return TRUE;		// been here, done this (as long as the tree control bits are correct)
	else if (ItemHasChildren(hNicknameFile))
		return TRUE;		// been here, done this

	CNicknameFile* p_nickfile = GetSafeNicknameFile(hNicknameFile);
	ASSERT(p_nickfile != NULL);

	// Get main window so that we can access the global status bar.
	CFrameWnd* p_mainframe = (CFrameWnd *) AfxGetMainWnd();
	ASSERT(p_mainframe != NULL);
	ASSERT(p_mainframe->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	CRString fmt(IDS_LOADING_NICKNAMES_PERCENT);

	CCursor wait_cursor;
	unsigned TOTAL = p_nickfile->GetCount();
	unsigned count = 0;
	POSITION pos = p_nickfile->GetHeadPosition();
	while (pos)
	{
		CNickname* p_nickname = p_nickfile->GetNext(pos);
		ASSERT(p_nickname != NULL);

		//
		// Take the current "View By" (display mode) into account in
		// order to determine the item text.  For all of the nickname
		// fields other than the nickname name itself, we need to
		// force the nickname data to be loaded into memory from the
		// nickname file.
		// 
		CString item_text;
		if (m_viewByItemType != ITEM_NICKNAME)
		{
			if (p_nickname->NeedsReadIn())
				p_nickfile->ReadNickname(p_nickname);
		}
		GetNicknameItemText(p_nickname, item_text);

		if (NULL == AddItem(m_viewByItemType, item_text, p_nickname->IsRecipient(), p_nickname, hNicknameFile))
			return FALSE;

		//
		// Periodically update progress pacifier on main status bar...
		//
		if ((++count & 0x003F) == 0x003F)
		{
			char prompt[100];
			wsprintf(prompt, fmt, p_nickfile->m_Name, (count * 100) / TOTAL);
			ASSERT(strlen(prompt) < sizeof(prompt));	// better late than never
			p_mainframe->SetMessageText(prompt);
			p_mainframe->GetMessageBar()->UpdateWindow();
		}
	}

	//
	// Now re-sort the nicknames by the new item text.
	//
	CString prompt;
	prompt.Format(IDS_LOADING_NICKNAMES_SORTING, (const char*)p_nickfile->m_Name);
	p_mainframe->SetMessageText(prompt);
	p_mainframe->GetMessageBar()->UpdateWindow();
	SortChildren(hNicknameFile);

	// Restore default status bar prompt.
	p_mainframe->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetSafeNickname [private]
//
// Returns a pointer to the CNickname object, if any, associated
// with the given tree item.  Otherwise, returns NULL if there is no
// associated CNickname object -- i.e., the item is a nickname file.
// In that case, use GetSafeNicknameFile() instead.
//
////////////////////////////////////////////////////////////////////////
CNickname* CNicknamesTreeCtrl::GetSafeNickname(HTREEITEM hItem) const
{
	if (NULL == hItem)
	{
		ASSERT(0);
		return NULL;
	}

	CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(hItem);
	ASSERT(p_itemdata != NULL);
	if (p_itemdata->m_itemType != ITEM_FILE)
	{
		ASSERT(p_itemdata->m_u.pNickname != NULL);
		ASSERT(p_itemdata->m_u.pNickname->IsKindOf(RUNTIME_CLASS(CNickname)));
		return p_itemdata->m_u.pNickname;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// GetSafeNicknameFile [private]
//
// Returns a pointer to the CNicknameFile object, if any, associated
// with the given tree item.  Otherwise, returns NULL if there is no
// associated CNicknameFile object -- i.e., the item is a nickname.
// In that case, use GetSafeNickname() instead.
//
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesTreeCtrl::GetSafeNicknameFile(HTREEITEM hItem) const
{
	if (NULL == hItem)
	{
		ASSERT(0);
		return NULL;
	}

	CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(hItem);
	ASSERT(p_itemdata != NULL);
	if (ITEM_FILE == p_itemdata->m_itemType)
	{
		ASSERT(p_itemdata->m_u.pNicknameFile != NULL);
		ASSERT(p_itemdata->m_u.pNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));
		return p_itemdata->m_u.pNicknameFile;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// GetNicknameItemText [private]
//
// Taking the current tree "view by mode" (m_viewByItemType) into account,
// stuff the appropriate nickname item name into the caller-provided
// string object (itemText).
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::GetNicknameItemText(CNickname* pNickname, CString& itemText) const
{
	ASSERT(pNickname != NULL);

	//
	// Take the current "View By" display mode into account.
	// 
	switch (m_viewByItemType)
	{
	case ITEM_NICKNAME:
		itemText = pNickname->GetName();
		break;
	case ITEM_ADDRESS:
		ASSERT(! pNickname->NeedsReadIn());
		itemText = pNickname->GetAddresses();
		break;
	case ITEM_CUSTOM:
		ASSERT(! pNickname->NeedsReadIn());
		ASSERT(! m_customFieldName.IsEmpty());
		if (! pNickname->GetNotesFieldValue(m_customFieldName, itemText))
			itemText.Empty();
		break;
	default:
		//
		// We are being very lenient here ... "unrecognized" display
		// modes are treated as normal nicknames.  This is necessary
		// since the user may happen to run Light using an INI file
		// created by Pro.  In this case, we need to map the Pro
		// display modes back to something Light can understand.
		//
		itemText = pNickname->GetName();
		break;
	}

	//
	// If the item text is empty, fix it up as a nickname wrapped
	// in funky braces that work with the MS Sans Serif font
	// (the funny octal codes below).
	//
	if (itemText.IsEmpty())
		itemText = '\253' + pNickname->GetName() + '\273';

	//
	// If the item text contains CR-LF, then replace them with
	// commas.
	//
	if (itemText.FindOneOf("\r\n") != -1)
		::ConvertEndOfLinesToCommas(itemText);
}


////////////////////////////////////////////////////////////////////////
// FindFileItem [private]
//
// Search the tree for a nickname file item which matches the given
// 'pNicknameFile'.  We need its HTREEITEM handle to pass to internal
// internal routines.
//
////////////////////////////////////////////////////////////////////////
HTREEITEM CNicknamesTreeCtrl::FindFileItem(CNicknameFile* pNicknameFile) const
{
	ASSERT(pNicknameFile);

	HTREEITEM h_nickfile = GetRootItem();
	while (h_nickfile != NULL)
	{
		CNicknameFile* p_nickfile = GetSafeNicknameFile(h_nickfile);
		ASSERT(p_nickfile != NULL);
		if (p_nickfile == pNicknameFile)
			return h_nickfile;		// found it!

		h_nickfile = GetNextSiblingItem(h_nickfile);
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// DeleteItemData [private]
//
// Recursively delete all lParam-based storage from this tree item
// and all of its children.  Does not delete the items themselves.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DeleteItemData(HTREEITEM hItem)
{
	//
	// First, process this item (the "root").
	//
	ASSERT(hItem != NULL);
	CNicknamesTreeItemData* p_itemdata = (CNicknamesTreeItemData *) GetItemData(hItem);
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


////////////////////////////////////////////////////////////////////////
// DoRefreshRecipListDisplay [private]
//
// Assumes a two-level hierarchy where the 'hItem' is a "root" item
// which corresponds to a nickname file.  The child items are
// therefore the nickname entries in that file.
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoRefreshRecipListDisplay(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);
	ASSERT(GetSafeNicknameFile(hItem) != NULL);
	HTREEITEM h_child = GetChildItem(hItem);
	while (h_child != NULL)
	{
		CNickname* p_nickname = GetSafeNickname(h_child);
		ASSERT(p_nickname != NULL);

		TV_ITEM tv_item;
		tv_item.mask = TVIF_HANDLE | TVIF_STATE;
		tv_item.hItem = h_child;
		if (p_nickname->IsRecipient())
			tv_item.state = TVIS_BOLD;
		else
			tv_item.state = 0;
		tv_item.stateMask = TVIS_BOLD;
		SetItem(&tv_item);

		h_child = GetNextSiblingItem(h_child);
	}
}


////////////////////////////////////////////////////////////////////////
// IsItemSelected [private]
//
// Returns TRUE if the item is selected.
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::IsItemSelected(HTREEITEM hItem)
{
	TV_ITEM tv_item;
	tv_item.mask = TVIF_HANDLE | TVIF_STATE;
	tv_item.hItem = hItem;
	tv_item.stateMask = TVIS_SELECTED;
	GetItem(&tv_item);

	if (tv_item.state & TVIS_SELECTED)
		return TRUE;
	else
		return FALSE;
}


////////////////////////////////////////////////////////////////////////
// IsItemExpanded [private]
//
// Returns TRUE if the item has children and it is expanded.
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesTreeCtrl::IsItemExpanded(HTREEITEM hItem)
{
	if (! ItemHasChildren(hItem))
		return FALSE;
		
	TV_ITEM tv_item;
	tv_item.mask = TVIF_HANDLE | TVIF_STATE;
	tv_item.hItem = hItem;
	tv_item.stateMask = TVIS_EXPANDED;
	GetItem(&tv_item);

	if (tv_item.state & TVIS_EXPANDED)
		return TRUE;
	else
		return FALSE;
}


////////////////////////////////////////////////////////////////////////
// DoDeselectAll [private]
//
// Clears selected state from all visible items in the tree control.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoDeselectAll()
{
	CCursor wait_cursor;
	SetRedraw(FALSE);
	HTREEITEM h_parent = GetRootItem();
	while (h_parent != NULL)
	{
		if (IsItemSelected(h_parent))
			DoSelectItem(h_parent, FALSE);

		if (IsItemExpanded(h_parent))
		{
			HTREEITEM h_child = GetChildItem(h_parent);
			while (h_child != NULL)
			{
				if (IsItemSelected(h_child))
					DoSelectItem(h_child, FALSE);

				h_child = GetNextSiblingItem(h_child);
			}
		}

		h_parent = GetNextSiblingItem(h_parent);
	}
	SetRedraw(TRUE);
}


////////////////////////////////////////////////////////////////////////
// DoDeselectAllChildren [private]
//
// Deselect all child items of the given parent item.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoDeselectAllChildren(HTREEITEM hParent)
{
	CCursor wait_cursor;
	SetRedraw(FALSE);
	HTREEITEM h_child = GetChildItem(hParent);
	while (h_child != NULL)
	{
		DoSelectItem(h_child, FALSE);
		h_child = GetNextSiblingItem(h_child);
	}
	SetRedraw(TRUE);
}


////////////////////////////////////////////////////////////////////////
// DoToggleSelection [private]
//
// Toggles the selection state for the given item.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoToggleSelection(HTREEITEM hItem)
{
	TV_ITEM tv_item;
	tv_item.mask = TVIF_HANDLE | TVIF_STATE;
	tv_item.hItem = hItem;
	tv_item.stateMask = TVIS_SELECTED;
	GetItem(&tv_item);

	// toggle the selection state bit
	if (tv_item.state & TVIS_SELECTED)
		tv_item.state &= ~TVIS_SELECTED;
	else
		tv_item.state |= TVIS_SELECTED;

	SetItem(&tv_item);
}


////////////////////////////////////////////////////////////////////////
// DoSelectItem [private]
//
// Sets the selection bit for the item.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoSelectItem(HTREEITEM hItem, BOOL isSelected)
{
	TV_ITEM tv_item;
	tv_item.mask = TVIF_HANDLE | TVIF_STATE;
	tv_item.hItem = hItem;
	tv_item.stateMask = TVIS_SELECTED;

	if (isSelected)
		tv_item.state = TVIS_SELECTED;
	else
		tv_item.state = 0;

	SetItem(&tv_item);
}


////////////////////////////////////////////////////////////////////////
// DoSelectItemRange [private]
//
// Select/deselect a contiguous range of items.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoSelectItemRange(HTREEITEM hAnchorItem, HTREEITEM hEndItem)
{
	ASSERT(hAnchorItem != NULL);
	ASSERT(hEndItem != NULL);

	CCursor wait_cursor;

	RECT anchor_rect;
	RECT end_rect;
	GetItemRect(hAnchorItem, &anchor_rect, FALSE);
	GetItemRect(hEndItem, &end_rect, FALSE);

	HTREEITEM h_item = hAnchorItem;
	if (anchor_rect.top < end_rect.top)
	{
		//
		// Anchor is above, so start at anchor and work
		// down.
		//
		while (h_item != hEndItem)
		{
			DoSelectItem(h_item, TRUE);
			h_item = GetNextVisibleItem(h_item);
			ASSERT(h_item != NULL);
		}
	}
	else
	{
		//
		// Anchor is below, so start at anchor and work up.
		//
		while (h_item != hEndItem)
		{
			DoSelectItem(h_item, TRUE);
			h_item = GetPrevVisibleItem(h_item);
			ASSERT(h_item != NULL);
		}
	}

	//
	// Select the last item itself.
	//
	DoSelectItem(hEndItem, TRUE);
}


////////////////////////////////////////////////////////////////////////
// DoSaveSelection [private]
//
// Saves handles for all selected items.
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoSaveSelection(BOOL bNicknamesOnly /*=FALSE*/)
{
	CCursor wait_cursor;
	m_SelectedItemArray.RemoveAll();

	HTREEITEM h_parent = GetRootItem();
	int FORNOW = 0;
	while (h_parent != NULL)
	{
		if ((! bNicknamesOnly) && IsItemSelected(h_parent))
		{
			m_SelectedItemArray.Add(DWORD(h_parent));
			FORNOW++;
		}

		if (IsItemExpanded(h_parent))
		{
			HTREEITEM h_child = GetChildItem(h_parent);
			while (h_child != NULL)
			{
				if (IsItemSelected(h_child))
				{
					m_SelectedItemArray.Add(DWORD(h_child));
					FORNOW++;
				}
				h_child = GetNextSiblingItem(h_child);
			}
		}

		h_parent = GetNextSiblingItem(h_parent);
	}
	TRACE1("CNicknamesTreeCtrl::DoSaveSelection: saved %d items\n", FORNOW);
}


////////////////////////////////////////////////////////////////////////
// DoRestoreSelection [private]
//
// Restores selection to previously saved state.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::DoRestoreSelection()
{
	CCursor wait_cursor;
	SetRedraw(FALSE);
	int FORNOW = 0;
	for (int i = 0; i <= m_SelectedItemArray.GetUpperBound(); i++)
	{
		DoSelectItem(HTREEITEM(m_SelectedItemArray.GetAt(i)), TRUE);
		FORNOW++;
	}
	m_SelectedItemArray.RemoveAll();
	SetRedraw(TRUE);

	TRACE1("CNicknamesTreeCtrl::DoRestoreSelection: restored %d items\n", FORNOW);
}


////////////////////////////////////////////////////////////////////////
// OnNew [private]
//
// Sends message to parent view to add a new nickname.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnNew()
{
	CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
	if (p_view)
	{
		ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));
		p_view->AutoClickNewButton();
	}
}


////////////////////////////////////////////////////////////////////////
// OnDelete [private]
//
// Sends message to parent view to delete the selected nicknames.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnDelete()
{
	CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
	if (p_view)
	{
		ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));
		p_view->AutoClickRemoveButton();
	}
}


////////////////////////////////////////////////////////////////////////
// OnRename [private]
//
// Enter rename mode on the selected nickname.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnRename()
{
	if (SELSTATE_SINGLE == m_LastEvaluatedSelectionState)
	{
		HTREEITEM h_selitem = GetSelectedItem();
		if (h_selitem != NULL)
		{
			// Make sure label editing is not already in progress
			if (NULL == GetEditControl())
				EditLabel(h_selitem);
		}
	}
	else
	{
		::MessageBeep(MB_OK);
	}
}

////////////////////////////////////////////////////////////////////////
// OnMoveTo [private]
//
// Move the selected nicknames to a nickname file.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnMoveTo()
{
	// Shareware: Reduced feature mode only allows one nickname file, no MOVE TO
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE mode
		ASSERT(0);
		return;
	}

	CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
	if (p_view)
	{
		ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));

		//
		// Gather up the currently selected nicknames and their
		// corresponding nickname files.  Package them into a collection
		// object containing a series of CNickname/CNicknameFile pointer
		// pairs.
		//
		CObList nickname_list;
		if (GetSelectedNicknames(nickname_list, TRUE))
			p_view->MoveOrCopyNicknames(TRUE, nickname_list);
	}
}

////////////////////////////////////////////////////////////////////////
// OnCopyTo [private]
//
// Copy the selected nicknames to a nickfile file.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnCopyTo()
{
	// Shareware: Reduced feature mode only allows one nickname file, no COPY TO
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE mode
		ASSERT(0);
		return;
	}

	CNicknamesViewLeft* p_view = (CNicknamesViewLeft *) GetParent();
	if (p_view)
	{
		ASSERT(p_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewLeft)));

		//
		// Gather up the currently selected nicknames and their
		// corresponding nickname files.  Package them into a collection
		// object containing a series of CNickname/CNicknameFile pointer
		// pairs.
		//
		CObList nickname_list;
		if (GetSelectedNicknames(nickname_list, TRUE))
			p_view->MoveOrCopyNicknames(FALSE, nickname_list);
	}
}

////////////////////////////////////////////////////////////////////////
// OnAddRecipient [private]
//
// Sends message to parent view to add the selected nicknames to the
// quick recipient list.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnAddRecipient()
{
	if (g_Nicknames)
	{
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_RECIPLIST_ADD, NULL);
		// FORNOW, do we need some hints here?
		EvaluateSelection();	// update RHS view recip list checkbox
	}
}


////////////////////////////////////////////////////////////////////////
// OnRemoveRecipient [private]
//
// Sends message to parent view to removed the selected nicknames from the
// quick recipient list.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesTreeCtrl::OnRemoveRecipient()
{
	if (g_Nicknames)
	{
		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_RECIPLIST_REMOVE, NULL);
		// FORNOW, do we need some hints here?
		EvaluateSelection();	// update RHS view recip list checkbox
	}
}


////////////////////////////////////////////////////////////////////////
// GetFileInsertPos [private]
//
// Figures out where to insert the new nickname file in the existing
// tree item list.  The main "Eudora Nicknames" file always goes first,
// and the rest are inserted in sorted order by filename.
////////////////////////////////////////////////////////////////////////
HTREEITEM CNicknamesTreeCtrl::GetFileInsertPos(const CString& strName) const
{
	CRString strEudoraNicknames(IDS_MAIN_NICK_FILE_NAME);

	HTREEITEM hCurrent = GetRootItem();
	if (NULL == hCurrent)
		return TVI_LAST;		// degenerate case - empty tree
	else if (strEudoraNicknames == strName)
		return TVI_FIRST;		// special case - main nicknames file always goes first

	//
	// Walk the list of nickname files in search of the place to insert.
	//
	HTREEITEM hInsertAfter = hCurrent;
	while (hCurrent != NULL)
	{
		CNicknameFile* pNickFile = GetSafeNicknameFile(hCurrent);
		ASSERT(pNickFile != NULL);

		if (strEudoraNicknames != pNickFile->m_Name)
		{
			if (strName.CompareNoCase(pNickFile->m_Name) < 0)
				return hInsertAfter;
		}
		else
		{
			ASSERT(hCurrent == GetRootItem());
		}

		hInsertAfter = hCurrent;
		hCurrent = GetNextSiblingItem(hCurrent);
	}

	return TVI_LAST;
}
