#ifndef __NICKTREE_H__
#define __NICKTREE_H__
// nicktree.h : header file
//

#ifndef WIN32
#error This is a WIN32-only file
#endif // WIN32

#include "qctree.h"

class CNickname;
class CNicknameFile;
class CNicknamesTreeItemData;

///////////////////////////////////////////////////////////////////////
// CNicknamesTreeCtrl
//
// The main tree control implementation class.  This is a smart 
// Eudora nicknames-specific tree control that is meant to be embedded
// in a parent "dialog" class.
///////////////////////////////////////////////////////////////////////
class CNicknamesTreeCtrl : public QCTreeCtrl
{
	BOOL	m_bInEditMode;

	DECLARE_DYNAMIC(CNicknamesTreeCtrl)

// Construction
public:

	BOOL	EditingName() { return m_bInEditMode; }

	enum ItemType
	{
		ITEM_FILE = 0,
		ITEM_NICKNAME,
		ITEM_ADDRESS,
		ITEM_CUSTOM
	};

	CNicknamesTreeCtrl();
	~CNicknamesTreeCtrl();

	virtual BOOL Init(void);

	//
	// Clients should NOT call CTreeCtrl::DeleteItem() or
	// CTreeCtrl::DeleteAllItems() directly, under pain of leaky
	// memory!
	//
	BOOL Reset(void);

	BOOL AddFile(CNicknameFile* pNicknameFile);
	BOOL AddNewNickname(CNicknameFile* pNicknameFile, CNickname* pNickname);
	BOOL DeleteExistingNickname(CNicknameFile* pNicknameFile, CNickname* pNickname, BOOL bLastOne = TRUE);
	BOOL GetSelectedNicknames(CObList& nicknameList, BOOL bGetNicknameFiles = FALSE);
	CNicknameFile* FindSingleSelectedNicknameFile();
	void MarkSelectedNicknameDirty();
	void BroadcastSelectedNickname();
	void NicknameWasChanged(CNickname* pNickname);
	void CheckForDuplicateNickname(CString* pNewName);
	BOOL RestoreVisualState();
	void RefreshRecipListDisplay();
	void RemoveSelectedItems();
	void SetViewByMode(int nIndex, const char* pFieldName);
	void EvaluateSelection();

	void OnEscKeyPressed(void);

	// Generated message map functions
protected:
	enum ImageOffset
	{
		IMAGE_CLOSED_FOLDER = 0,
		IMAGE_OPEN_FOLDER,
		IMAGE_CLOSED_FOLDER_READONLY,
		IMAGE_OPEN_FOLDER_READONLY,
		IMAGE_NICKNAME,
		IMAGE_ADDRESS,
		IMAGE_CUSTOM		// this is actually a base index for one or more custom bitmaps
	};

	//{{AFX_MSG(CNicknamesTreeCtrl)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnDestroy();
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg long OnRightButtonDown(WPARAM wParam, LPARAM lParam);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	HTREEITEM AddItem(ItemType itemType,
					  const CString& itemName, 
					  BOOL isRecipList,
					  void* pItemData,
					  HTREEITEM hParentItem = NULL);
	CNickname* GetSafeNickname(HTREEITEM hItem) const;
	CNicknameFile* GetSafeNicknameFile(HTREEITEM hItem) const;
	void GetNicknameItemText(CNickname* pNickname, CString& itemText) const;
	HTREEITEM FindFileItem(CNicknameFile* pNicknameFile) const;
	void DeleteItemData(HTREEITEM hItem);
	void DoRefreshRecipListDisplay(HTREEITEM hItem);
	UINT EvaluateSelectionForRecipients();
	BOOL EvaluateSelectionForRename() const;
	HTREEITEM GetFileInsertPos(const CString& strName) const;
	void OnNew();
	void OnDelete();
	void OnRename();
	void OnMoveTo();
	void OnCopyTo();
	void OnAddRecipient();
	void OnRemoveRecipient();

	BOOL PopulateNicknameFileNode(HTREEITEM hNicknameFile);

	//
	// Support for multi-selection.  Caller's should not
	// use the built-in CTreeCtrl::SelectItem(), CTreeCtrl::Select(),
	// or CTreeCtrl::GetSelectedItem() methods, since those apply
	// to the default single-select behavior of the tree control.
	//
	enum SelectionState
	{
		SELSTATE_UNKNOWN = -1,
		SELSTATE_NONE = 0,
		SELSTATE_SINGLE = 1,
		SELSTATE_MULTI = 2
	} m_LastEvaluatedSelectionState;		// performance hack

	enum EvaluateSelectionHint
	{
		EVALHINT_FULL,				// force full evaluation
		EVALHINT_SINGLE,			// optimize for single selection
		EVALHINT_SKIP				// skip evaluation
	} m_EvaluateSelectionHint;		// performance hack

	BOOL IsItemExpanded(HTREEITEM hItem);
	BOOL IsItemSelected(HTREEITEM hItem);
	void DoSaveSelection(BOOL bNicknamesOnly = FALSE);
	void DoRestoreSelection();
	void DoDeselectAll();
	void DoDeselectAllChildren(HTREEITEM hParent);
	void DoToggleSelection(HTREEITEM hItem);
	void DoSelectItem(HTREEITEM hItem, BOOL isSelected);
	void DoSelectItemRange(HTREEITEM hAnchorItem, HTREEITEM hEndItem);

	CImageList m_ImageList;
	CDWordArray m_SelectedItemArray;	// array of HTREEITEM handles for multi-select
	HTREEITEM m_hAnchorItem;			// anchor point of extended selection
	ItemType m_viewByItemType;
	int m_customImageIndex;				// used only when m_viewByItemType == ITEM_CUSTOM
	CString m_customFieldName;			// used only when m_viewByItemType == ITEM_CUSTOM

	CImageList* m_pDragImage;
	HTREEITEM m_hDragItem;
};


///////////////////////////////////////////////////////////////////////
// CNicknamesTreeItemData
//
// Private class that is stuffed as a pointer into the lParam field of 
// the CTreeCtrl items.
///////////////////////////////////////////////////////////////////////
class CNicknamesTreeItemData
{
	friend class CNicknamesTreeCtrl;		// private class

private:
	CNicknamesTreeItemData(CNicknamesTreeCtrl::ItemType itemType, 
						   void* pItemData) :
		m_itemType(itemType)
	{
		ASSERT(pItemData != NULL);
		if (CNicknamesTreeCtrl::ITEM_FILE == m_itemType)
		{
			m_u.pNicknameFile = (CNicknameFile *) pItemData;
			ASSERT(m_u.pNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));
		}
		else
		{
			m_u.pNickname = (CNickname *) pItemData;
			ASSERT(m_u.pNickname->IsKindOf(RUNTIME_CLASS(CNickname)));
		}
	}

	~CNicknamesTreeItemData(void)
	{
	}

	CNicknamesTreeItemData(void);							// not implemented
	CNicknamesTreeItemData(const CNicknamesTreeItemData&);	// not implemented
	void operator=(const CNicknamesTreeItemData&);			// not implemented

	CNicknamesTreeCtrl::ItemType m_itemType;	// type of tree node
	union
	{
		CNicknameFile* pNicknameFile;			// used when m_itemType == ITEM_FILE
		CNickname* pNickname;					// used when m_itemType != ITEM_FILE
	} m_u;
};


/////////////////////////////////////////////////////////////////////////////
#endif // __NICKTREE_H__
