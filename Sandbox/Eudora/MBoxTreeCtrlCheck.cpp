
// MBoxTreeCtrlCheck.cpp : implementation file
//

#include "stdafx.h"
#include "mboxtree.h"
#include <list.h>

#include "rs.h" // For CRString
#include "resource.h" // For the context menu strings

#include "MBoxTreeCtrlCheck.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

UINT msgMBoxTreeCtrlCheck_CheckChange = ::RegisterWindowMessage("msgMBoxTreeCtrlCheck_CheckChange");

//	enum CheckState { TCS_NOSTATE, TCS_UNCHECKED, TCS_PARTIAL_CHECK, TCS_CHECKED };
//	enum CheckAction { TCA_CHECK, TCA_PARTIAL_CHECK, TCA_UNCHECK, TCA_TOGGLE, TCA_REFRESH };

/////////////////////////////////////////////////////////////////////////////
// CMBoxTreeCtrlCheck

CMBoxTreeCtrlCheck::CMBoxTreeCtrlCheck()
	: CMboxTreeCtrl(), m_bInitStateImageList(FALSE), m_bInitItemImageList(FALSE),
	  m_bContainers(true), m_bSingleCheck(false), m_hCheckedItem(NULL)
{
}

CMBoxTreeCtrlCheck::~CMBoxTreeCtrlCheck()
{
}

BEGIN_MESSAGE_MAP(CMBoxTreeCtrlCheck, CMboxTreeCtrl)
	//{{AFX_MSG_MAP(CMBoxTreeCtrlCheck)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMBoxTreeCtrlCheck message handlers

BOOL CMBoxTreeCtrlCheck::InitItemImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask)
{
	ASSERT(!m_bInitItemImageList);

	if (m_bInitItemImageList) // Already init'd, ignore re-init
		return (FALSE);

	if (m_imageItem.Create( nImageBitmapID, cImageX, 0, crImageMask ))
	{
		SetImageList( &m_imageItem, TVSIL_NORMAL );
		m_bInitItemImageList = TRUE;

		return (TRUE);
	}

	m_imageItem.DeleteImageList();

	return (FALSE);
}

BOOL CMBoxTreeCtrlCheck::InitStateImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask)
{
	ASSERT(!m_bInitStateImageList);

	if (m_bInitStateImageList) // Already init'd, ignore re-init
		return (FALSE);

	if (m_imageState.Create( nImageBitmapID, cImageX, 0, crImageMask ))
	{
		SetImageList( &m_imageState, TVSIL_STATE );
		m_bInitStateImageList = TRUE;

		return (TRUE);
	}

	m_imageState.DeleteImageList();

	return (FALSE);
}


BOOL CMBoxTreeCtrlCheck::InitImageLists(UINT nImageBitmapID, int cImageX, COLORREF crImageMask,
										UINT nStateBitmapID, int cStateX, COLORREF crStateMask )
{
	if (InitItemImageList(nImageBitmapID, cImageX, crImageMask) &&
		InitStateImageList(nStateBitmapID, cStateX, crStateMask))
		return (TRUE);

	return (FALSE);
}

// --------------------------------------------------------------------------
// Overrides

// WARNING: Overriding non-virtual function!
HTREEITEM CMBoxTreeCtrlCheck::InsertItem(LPTV_INSERTSTRUCT lpInsertStruct)
{
	// This does not force the checkbox (state image)
	HTREEITEM h = CMboxTreeCtrl::InsertItem(lpInsertStruct);

	if (lpInsertStruct->hParent != TVI_ROOT)
	{
		CRect rct;
		if (GetItemRect(lpInsertStruct->hParent, LPRECT(rct), false))
			InvalidateRect(LPRECT(rct));

		if (DoAction(lpInsertStruct->hParent, TCA_REFRESH))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}
	}

	return h;
}

// WARNING: Overriding non-virtual function!
HTREEITEM CMBoxTreeCtrlCheck::InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage,
											int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,
											HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	// This does not force the checkbox (state image)
	HTREEITEM h = CMboxTreeCtrl::InsertItem(nMask, lpszItem, nImage, nSelectedImage, nState, nStateMask, lParam, hParent, hInsertAfter);
	
	if (hParent != TVI_ROOT)
	{
		CRect rct;
		if (GetItemRect(hParent, LPRECT(rct), false))
			InvalidateRect(LPRECT(rct));

		if (DoAction(hParent, TCA_REFRESH))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}
	}

	return h;
}

// WARNING: Overriding non-virtual function!
HTREEITEM CMBoxTreeCtrlCheck::InsertItem(LPCTSTR lpszItem, HTREEITEM hParent /* = TVI_ROOT */,
						HTREEITEM hInsertAfter /* = TVI_LAST */)
{
	UINT nMask, nState, nStateMask;

	// Forces the checkbox to be present
	nMask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	nStateMask = TVIS_STATEIMAGEMASK;
	nState = INDEXTOSTATEIMAGEMASK(TCS_UNCHECKED);

	HTREEITEM h = CMboxTreeCtrl::InsertItem(nMask, lpszItem, 0, 0, nState, nStateMask, 0, hParent, hInsertAfter);
	
	if (hParent != TVI_ROOT)
	{
		CRect rct;
		if (GetItemRect(hParent, LPRECT(rct), false))
			InvalidateRect(LPRECT(rct));

		if (DoAction(hParent, TCA_REFRESH))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}
	}

	return h;
}

// WARNING: Overriding non-virtual function!
HTREEITEM CMBoxTreeCtrlCheck::InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage,
						HTREEITEM hParent /* = TVI_ROOT */, HTREEITEM hInsertAfter /* = TVI_LAST */)
{
	UINT nMask, nState, nStateMask;

	// Forces the checkbox to be present
	nMask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	nStateMask = TVIS_STATEIMAGEMASK;
	nState = INDEXTOSTATEIMAGEMASK(TCS_UNCHECKED);

	HTREEITEM h = CMboxTreeCtrl::InsertItem(nMask, lpszItem, nImage, nSelectedImage, nState, nStateMask, 0, hParent, hInsertAfter);

	if (hParent != TVI_ROOT)
	{
		CRect rct;
		if (GetItemRect(hParent, LPRECT(rct), false))
			InvalidateRect(LPRECT(rct));

		if (DoAction(hParent, TCA_REFRESH))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}
	}

 	return h;
}

// We trap double clicks to avoid expanding/collapsing tree nodes when over the checkbox
void CMBoxTreeCtrlCheck::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	UINT uHitFlags = 0;
	HTREEITEM hItem = HitTest(point, &uHitFlags);

	if (uHitFlags & TVHT_ONITEMSTATEICON)
	{
		SetFocus();
		SelectItem(hItem);
		if (DoAction(hItem, TCA_TOGGLE))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}

		return;
	}
	
	CMboxTreeCtrl::OnLButtonDblClk(nFlags, point);
}

void CMBoxTreeCtrlCheck::OnLButtonDown(UINT nFlags, CPoint point) 
{	
	UINT uHitFlags = 0;
	HTREEITEM hItem = HitTest(point, &uHitFlags);

	if (uHitFlags & TVHT_ONITEMSTATEICON)
	{
		SetFocus();
		SelectItem(hItem);
		if (DoAction(hItem, TCA_TOGGLE))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}

		return;
	}

	CMboxTreeCtrl::OnLButtonDown(nFlags, point);
}

void CMBoxTreeCtrlCheck::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE)
	{
		HTREEITEM hItem = GetSelectedItem();
		if (DoAction(hItem, TCA_TOGGLE))
		{
			CWnd *pOwner = GetOwner();

			if (pOwner)
				pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
		}
	}

	CMboxTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

bool CMBoxTreeCtrlCheck::DoAction(HTREEITEM hItem, TCT_CheckAction nAction)
{
	ASSERT(hItem);

	TCT_CheckState nState = (TCT_CheckState) STATETOINDEXIMAGEMASK(GetItemState(hItem, TVIS_STATEIMAGEMASK));
	TCT_CheckState nNewState = nState; // default is to do nothing

	if (nAction == TCA_TOGGLE)
	{
		switch (nState)
		{
			case TCS_NOSTATE:
			case TCS_UNCHECKED:
			{
				nAction = TCA_CHECK;
			}
			break;
			
/*			// Uncomment to toggle THREE states
			case TCS_CHECKED:
			{
				if ( (m_bContainers) && (ItemHasChildren(hItem)) )
				{
					BOOL bAnyContainerChildren = false, bAnyLeafChildren = false;
					HTREEITEM hChild = GetChildItem(hItem);

					while (((!bAnyContainerChildren) || (!bAnyLeafChildren)) && (hChild))
					{
						if (ItemHasChildren(hChild))
							bAnyContainerChildren = true;
						else
							bAnyLeafChildren = true;

						hChild = GetNextSiblingItem(hChild);
					}

					if ((bAnyContainerChildren) && (bAnyLeafChildren))
						nAction = TCA_PARTIAL_CHECK;
					else
						nAction = TCA_UNCHECK;
				}
				else
					nAction = TCA_UNCHECK;
			}
			break;
*/

			case TCS_CHECKED:
			case TCS_PARTIAL_CHECK:
			{
				nAction = TCA_UNCHECK;
			}
			break;
		}
	}

	ASSERT(nAction != TCA_TOGGLE);

	switch (nAction)
	{
		case TCA_CHECK:				nNewState = TCS_CHECKED;		break;
		case TCA_PARTIAL_CHECK:		nNewState = TCS_PARTIAL_CHECK;	break;
		case TCA_UNCHECK:			nNewState =	TCS_UNCHECKED;		break;
		// case TCA_TOGGLE: break;

		case TCA_REFRESH:
			nNewState = GetRefreshState(hItem, nNewState);
		break;
	}

	if (nState == nNewState)
		return (false); // No change

	HTREEITEM hOldParent = NULL;

	// If we can have only one check, then uncheck the old check
	if ( (m_bSingleCheck) && (nNewState == TCS_CHECKED) && ((!m_bContainers) || (!ItemHasChildren(hItem))) && (m_hCheckedItem != NULL) )
	{
		SetItemState(m_hCheckedItem, INDEXTOSTATEIMAGEMASK(TCS_UNCHECKED), TVIS_STATEIMAGEMASK);

#ifdef _DEBUG
	UpdateWindow();
#endif // _DEBUG

		hOldParent = GetParentItem(m_hCheckedItem);
		m_hCheckedItem = GetFirstCheckedItem();
	}

	// Now do any work needed to reflect the container condition (to children)
	if ((m_bContainers) && (ItemHasChildren(hItem)))
	{
		switch (nAction)
		{
			case TCA_CHECK: // Check all children recursively
			{
				HTREEITEM hChild = GetChildItem(hItem);
				while (hChild)
				{
					DoAction(hChild, TCA_CHECK);

					if (m_bSingleCheck)
						break; // Only check the first item

					hChild = GetNextSiblingItem(hChild);

				}
			}
			break;

			case TCA_PARTIAL_CHECK: // Check only 1st level 'leaf' children (uncheck rest)
			{
				HTREEITEM hChild = GetChildItem(hItem);
				while (hChild)
				{
					if (ItemHasChildren(hChild))
						DoAction(hChild, TCA_UNCHECK);
					else
					{
						DoAction(hChild, TCA_CHECK);

						if (m_bSingleCheck)
							break; // Only check the first item
					}

					hChild = GetNextSiblingItem(hChild);
				}
			}
			break;

			case TCA_UNCHECK: // Uncheck all children recursively
			{
				HTREEITEM hChild = GetChildItem(hItem);
				while (hChild)
				{
					DoAction(hChild, TCA_UNCHECK);
					hChild = GetNextSiblingItem(hChild);
				}
			}
			break;

			// case TCA_TOGGLE: break;
			// case TCA_REFRESH: break;
		}
	}

	if ( (m_bSingleCheck) && (nNewState == TCS_CHECKED) && (m_bContainers) && (ItemHasChildren(hItem)) )
	{
		nNewState = GetRefreshState(hItem, nNewState);
	}

	SetItemState(hItem, INDEXTOSTATEIMAGEMASK(nNewState), TVIS_STATEIMAGEMASK);

#ifdef _DEBUG
	UpdateWindow();
#endif // _DEBUG

	HTREEITEM hParent = GetParentItem(hItem);

	// hOldParent is set if we unchecked something (single check state)
	if (hOldParent)
		DoAction(hOldParent, TCA_REFRESH);

	// Only do a second refresh if we have a different parent
	if ((hParent) && (hParent != hOldParent))
		DoAction(hParent, TCA_REFRESH);

	m_hCheckedItem = GetFirstCheckedItem();

	return (true);
}

BOOL CMBoxTreeCtrlCheck::SetContainerFlag(BOOL bContainers /* = true */)
{
	if (bContainers == m_bContainers)
		return (m_bContainers);

	BOOL ret = m_bContainers;
	m_bContainers = bContainers;

	RefreshTreeChecks();

	return ret;
}

void CMBoxTreeCtrlCheck::RefreshTreeChecks()
{
	HTREEITEM hRoot = GetRootItem();
	while (hRoot)
	{
		RefreshNode(hRoot);
		hRoot = GetNextSiblingItem(hRoot);
	}
}

void CMBoxTreeCtrlCheck::RefreshNode(HTREEITEM hItem)
{
	ASSERT(hItem);

	if (ItemHasChildren(hItem))
	{
		HTREEITEM hChild = GetChildItem(hItem);
		while (hChild)
		{
			RefreshNode(hChild);
			hChild = GetNextSiblingItem(hChild);
		}

		TCT_CheckState nCurState = (TCT_CheckState) STATETOINDEXIMAGEMASK(GetItemState(hItem, TVIS_STATEIMAGEMASK));
		TCT_CheckState nNewState = GetRefreshState(hItem, nCurState);
		if (nCurState != nNewState)
		{
			SetItemState(hItem, INDEXTOSTATEIMAGEMASK(nNewState), TVIS_STATEIMAGEMASK);
#ifdef _DEBUG
	UpdateWindow();
#endif // _DEBUG
		}
	}
}

CMBoxTreeCtrlCheck::TCT_CheckState CMBoxTreeCtrlCheck::GetRefreshState(HTREEITEM hItem, TCT_CheckState CurrentState /* = TCS_NOSTATE */ )
{
	ASSERT(hItem);

	TCT_CheckState nNewState = CurrentState;

	if (m_bContainers)
	{
		if (ItemHasChildren(hItem))
		{
			BOOL bNoneChecked = true;
			BOOL bAllChecked = true;
			TCT_CheckState nChildState = TCS_NOSTATE;

			HTREEITEM hChild = GetChildItem(hItem);
			while (((bNoneChecked) || (bAllChecked)) && (hChild))
			{
				nChildState = (TCT_CheckState) STATETOINDEXIMAGEMASK(GetItemState(hChild, TVIS_STATEIMAGEMASK));
				switch (nChildState)
				{
					case TCS_NOSTATE:
					case TCS_UNCHECKED:
					{
						bAllChecked = false;
					}
					break;

					case TCS_CHECKED:
					{
						bNoneChecked = false;
					}
					break;

					case TCS_PARTIAL_CHECK:
					{
						bAllChecked = false;
						bNoneChecked = false;
					}
					break;
				}

				hChild = GetNextSiblingItem(hChild);
			}

			if (bNoneChecked)
				nNewState = TCS_UNCHECKED;
			else
			{
				if (bAllChecked)
					nNewState = TCS_CHECKED;
				else
					nNewState = TCS_PARTIAL_CHECK;
			}
		}
	}
	else
	{
		if (nNewState == TCS_PARTIAL_CHECK)
			nNewState = TCS_CHECKED;	// When you turn off containers all partials become checks
	}

	return (nNewState);
}


void CMBoxTreeCtrlCheck::SetCheck(HTREEITEM hItem, BOOL bCheck /* = true */)
{
	ASSERT(hItem);

	if (!DoAction(hItem, bCheck ? TCA_CHECK : TCA_UNCHECK))
		return;

	CWnd *pOwner = GetOwner();

	if (pOwner)
		pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
}

BOOL CMBoxTreeCtrlCheck::IsItemChecked(HTREEITEM hItem) const
{
	ASSERT(hItem);

	if (!hItem)
		return (false);

	// see if this is an IMAP mailbox -jdboyd
	CMboxTreeItemData *pImapItemData = (CMboxTreeItemData *) GetItemData(hItem);
	bool isIMAPBox = false;
	if (pImapItemData) isIMAPBox = (pImapItemData->m_itemType == ITEM_IMAP_MAILBOX);

	// we can skip this item if it's a folder, as long as it's NOT an IMAP mailbox -jdboyd
	if ((m_bContainers) && (ItemHasChildren(hItem)) && (!isIMAPBox))
		return (false);

	TCT_CheckState nState = (TCT_CheckState) STATETOINDEXIMAGEMASK(GetItemState(hItem, TVIS_STATEIMAGEMASK));
	
	// this item is checked if it's fully checked, or is a partially checked IMAP mailbox -jdboyd
	return ((nState == TCS_CHECKED) || (isIMAPBox && nState == TCS_PARTIAL_CHECK));
}

HTREEITEM CMBoxTreeCtrlCheck::GetNextItem(HTREEITEM hItem, UINT nCode) const
{
	ASSERT(hItem);
	return CMboxTreeCtrl::GetNextItem(hItem, nCode);
}

HTREEITEM CMBoxTreeCtrlCheck::GetNextItem(HTREEITEM hItem) const
{
	ASSERT(hItem);

	if (ItemHasChildren(hItem))
		return (GetChildItem(hItem));

	HTREEITEM hSibling;

	while (!(hSibling = GetNextSiblingItem(hItem)))
	{
		if ((hItem = GetParentItem(hItem)) == NULL)
			return NULL;
	}

	return hSibling;
}

HTREEITEM CMBoxTreeCtrlCheck::GetFirstCheckedItem() const
{
	HTREEITEM hItem = GetRootItem();
	
	if ((hItem) && (!IsItemChecked(hItem)))
		hItem = GetNextCheckedItem(hItem);

	return hItem;
}

HTREEITEM CMBoxTreeCtrlCheck::GetNextCheckedItem(HTREEITEM hItem) const
{
	ASSERT(hItem);

	while (hItem = GetNextItem(hItem))
	{
		if (IsItemChecked(hItem))
			break;
	}

	return (hItem);
}

// Returns old value
BOOL CMBoxTreeCtrlCheck::SetSingleCheck(BOOL bSingleCheck /* = true */ )
{
	if (bSingleCheck == m_bSingleCheck)
		return (m_bSingleCheck);

	BOOL bOldValue = m_bSingleCheck;
	m_bSingleCheck = bSingleCheck;

	if (m_bSingleCheck)
	{
		// If we are setting the single-check mode, uncheck all but one
		m_hCheckedItem = GetFirstCheckedItem();

		if (m_hCheckedItem)
		{
			bool bChanged = false;
			HTREEITEM hItem = m_hCheckedItem;
			while (hItem = GetNextCheckedItem(hItem))
			{
				// Turn off the check
				SetItemState(hItem, INDEXTOSTATEIMAGEMASK(TCS_UNCHECKED), TVIS_STATEIMAGEMASK);
				bChanged = true;
#ifdef _DEBUG
	UpdateWindow();
#endif // _DEBUG
//				DoAction(hItem, TCA_UNCHECK);
			}

			if (bChanged)
			{
				RefreshTreeChecks();

				CWnd *pOwner = GetOwner();

				if (pOwner)
					pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
			}
		}
	}

	return (bOldValue);
}

BOOL CMBoxTreeCtrlCheck::ExpandAll()
{
	HTREEITEM hItem;
	
	for (hItem = GetRootItem(); hItem; hItem = GetNextItem(hItem))
	{
		if ((ItemHasChildren(hItem)) && (GetItemState(hItem, TVIF_STATE) != TVIS_EXPANDED))
		{
			Expand(hItem, TVE_EXPAND);
		}
	}

	return (TRUE);
}


BOOL CMBoxTreeCtrlCheck::GetItemStruct(TV_INSERTSTRUCT &tvstruct,
							ItemType itemType,
							const CString& itemName, 
							QCMailboxCommand* pCommand,
							BOOL isChecked)
{
	if (CMboxTreeCtrl::GetItemStruct(tvstruct, itemType, itemName, pCommand, isChecked))
	{
		tvstruct.item.mask |= TVIF_STATE;
		tvstruct.item.state = INDEXTOSTATEIMAGEMASK(TCS_UNCHECKED);
		tvstruct.item.stateMask = TVIS_STATEIMAGEMASK;

		return (TRUE);
	}

	return (FALSE);
}

UINT CMBoxTreeCtrlCheck::GetCheckCount() const
{
	HTREEITEM hItem = GetRootItem();
	UINT nCount = 0;

	while (hItem)
	{
		// add this box to the count if it is an IMAP mailbox -jdboyd
		CMboxTreeItemData *pImapItemData = (CMboxTreeItemData *) GetItemData(hItem);
		bool isIMAPBox = false;
		if (pImapItemData) isIMAPBox = (pImapItemData->m_itemType == ITEM_IMAP_MAILBOX);

		// we can skip this item if it's a folder, unless it's an IMAP mailbox -jdboyd
		if ((!m_bContainers) || (!ItemHasChildren(hItem)) || (isIMAPBox))
		{
			// Counting only the checked items
			if (IsItemChecked(hItem))
			{
				// Because we can have leaf folders (empty folder), we need
				// to make sure we are only counting the mailboxes

				// Grad the data
				CMboxTreeItemData *pItemData = (CMboxTreeItemData *) GetItemData(hItem);
				ASSERT(pItemData);
				if ((pItemData) && (pItemData->IsMbx()))
					nCount++; // Mailbox -- inc count
			}
		}

		hItem = GetNextItem(hItem);
	}

	return (nCount);
}

BOOL CMBoxTreeCtrlCheck::GetSelectedList(list<QCMailboxCommand*> *cmdlist) const
{
	ASSERT(cmdlist);

	if (!cmdlist)
		return (FALSE);

	cmdlist->erase_raw(cmdlist->begin(), cmdlist->end()); // erase the whole list (don't call destructors)

	HTREEITEM hItem = GetRootItem();

	while (hItem)
	{
		// see if this item is an IMAP mailbox -jdboyd
		CMboxTreeItemData *pImapItemData = (CMboxTreeItemData *) GetItemData(hItem);
		bool isIMAPBox = false;
		if (pImapItemData) isIMAPBox = (pImapItemData->m_itemType == ITEM_IMAP_MAILBOX);


		// we can skip this item if it's a folder, unless it's an IMAP folder -jdboyd
		// note, non-selectable IMAP boxes are skipped during the search later.
		if ((!m_bContainers) || (!ItemHasChildren(hItem)) || isIMAPBox)
		{
			if (IsItemChecked(hItem))
			{
				CMboxTreeItemData *pItemData = (CMboxTreeItemData *) GetItemData(hItem);
				ASSERT(pItemData);
				if ((pItemData) && (pItemData->IsMbx()))
					cmdlist->push_back(pItemData->m_pCommand); // Mailbox -- add to list
			}
		}

		hItem = GetNextItem(hItem);
	}

	return (TRUE);
}

void CMBoxTreeCtrlCheck::CheckAll(bool bCheck /* = true */)
{
	HTREEITEM hItem;
	
	for (hItem = GetRootItem(); hItem; hItem = GetNextItem(hItem))
	{
		if (bCheck)
			SetItemState(hItem, INDEXTOSTATEIMAGEMASK(TCS_CHECKED), TVIS_STATEIMAGEMASK);
		else
			SetItemState(hItem, INDEXTOSTATEIMAGEMASK(TCS_UNCHECKED), TVIS_STATEIMAGEMASK);
	}

	CWnd *pOwner = GetOwner();

	if (pOwner)
		pOwner->SendMessage(msgMBoxTreeCtrlCheck_CheckChange, (WPARAM) 0, (LPARAM)0);
}

////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected, virtual]
//
// Called when the user releases the right button in the tree control.
////////////////////////////////////////////////////////////////////////
void CMBoxTreeCtrlCheck::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	if (GetCapture() != this)
		SetCapture();

	// Popup menu stuff
	CMenu menu;
	menu.CreatePopupMenu();

	const UINT nMENUID_SelectAll = 1;
	const UINT nMENUID_SelectNone = 2;

	menu.AppendMenu(MF_STRING, nMENUID_SelectAll, LPCSTR(CRString(IDS_MBOXTREECTRLCHECK_POPUP_SELALL)));
	menu.AppendMenu(MF_STRING, nMENUID_SelectNone, LPCSTR(CRString(IDS_MBOXTREECTRLCHECK_POPUP_SELNONE)));

	DWORD sel = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON
					| TPM_NONOTIFY | TPM_RETURNCMD, ptScreen.x, ptScreen.y, this);

	menu.DestroyMenu();

	// Make sure we selected something
	if (0 != sel)
	{
		switch (sel)
		{
			case nMENUID_SelectAll:
			{
				CheckAll(true);
			}
			break;

			case nMENUID_SelectNone:
			{
				CheckAll(false);
			}
			break;

			default:
				ASSERT(0);
		}
	}
}

HTREEITEM CMBoxTreeCtrlCheck::FindCheck(QCMailboxCommand *pMbxCmd, bool bClearCheck /* = true */)
{
	if (bClearCheck)
		CheckAll(false);

	HTREEITEM hItem;	
	for (hItem = GetRootItem(); hItem; hItem = GetNextItem(hItem))
	{
		CMboxTreeItemData *pItemData = (CMboxTreeItemData *) GetItemData(hItem);
		ASSERT(pItemData);
		if (pItemData)
		{
			if ((pItemData->m_pCommand) == pMbxCmd)
			{
				SetCheck(hItem);
				return (hItem);
			}
		}
	}

	return (NULL);
}
