// nicktree.h : header file
//
//
// Copyright (c) 1995-2000 by QUALCOMM, Incorporated
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

#ifndef __NICKTREE_H__
#define __NICKTREE_H__

#include "qctree.h"
#include "QCImageList.h"		// c_dalew 9/30/99 - Added so we can use QCImageList.

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
    int GetImageIndexFromNickname(const CNickname *);
	int GetCustomImageIndex(const char* name);
	BOOL	EditingName() { return m_bInEditMode; }

	enum ItemType
	{
		ITEM_FILE = 0,
		ITEM_NICKNAME,
		ITEM_ADDRESS,
		ITEM_CUSTOM
	};
	static enum FindTextPane
	{
		PANE_NOT_FOUND = -1,
		PANE_ONE,
		PANE_TWO,
		PANE_THREE,
		PANE_FOUR,
		PANE_FIVE
	};

	static enum FindTextField
	{
		FIELD_NOT_FOUND = -1,
		FIELD_NICKNAME,
		FIELD_NAME,
		FIELD_FIRSTNAME,
		FIELD_LASTNAME,
		FIELD_ADDRESSES,
		FIELD_ADDRESS,
		FIELD_CITY,
		FIELD_STATE,
		FIELD_COUNTRY,
		FIELD_ZIP,
		FIELD_PHONE,
		FIELD_FAX,
		FIELD_MOBILE,
		FIELD_WEB,
		FIELD_COMPANY,
		FIELD_TITLE,
		FIELD_ADDRESS2,
		FIELD_CITY2,
		FIELD_STATE2,
		FIELD_COUNTRY2,
		FIELD_ZIP2,
		FIELD_PHONE2,
		FIELD_FAX2,
		FIELD_MOBILE2,
		FIELD_WEB2,
		FIELD_OTHER_EMAIL,
		FIELD_OTHER_PHONE,
		FIELD_OTHER_WEB,
		FIELD_NOTES
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
	BOOL GetSelectedNicknamefiles(CObList& nickfileList);
	CNicknameFile* FindSingleSelectedNicknameFile();
	CNicknameFile* FindRootNicknameFile();
	void MarkSelectedNicknameDirty();
	void BroadcastSelectedNickname();
	void NicknameWasChanged(CNickname* pNickname);
	void CheckForDuplicateNickname(CString* pNewName);
	bool CheckForDuplicateNicknameInTree();
	BOOL RestoreVisualState();
	void RefreshRecipListDisplay();
	void RefreshBPListDisplay();
	void RemoveSelectedItems();
	void SetViewByMode(int nIndex, const char* pFieldName);
	void EvaluateSelection();
	int DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
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
		IMAGE_GROUP,
		IMAGE_HOME,
		IMAGE_PHONE,
		IMAGE_FAX,
		IMAGE_NICKNAME_ON_BP,
		IMAGE_GROUP_ON_BP,
		IMAGE_WEB,
		IMAGE_CLOSED_FOLDER_BP,
		IMAGE_OPEN_FOLDER_BP
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
	UINT EvaluateSelectionForBP();
	BOOL EvaluateSelectionForRename() const;
	BOOL EvaluateSelectionForNewAndDel(bool bIsNew) const;
	HTREEITEM GetFileInsertPos(const char* strName) const;
	void OnNew();
	void OnDelete();
	void OnRename();
	void OnMoveTo();
	void OnCopyTo();
	void OnAddRecipient();
	void OnRemoveRecipient();
	void OnAddBP();
	void OnRemoveBP();
	int Find(CNickname *pNick, const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, int nFieldStart);
	BOOL FindText(CString szField, const char *szSearch, BOOL bMatchCase, BOOL bWholeWord);
	BOOL PopulateNicknameFileNode(HTREEITEM hNicknameFile);
	HTREEITEM GetFirstSelectedNickorFile();
	int m_nFoundField;

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

	QCImageList m_ImageList;			// c_dalew 9/30/99 - Changed CImageList to QCImageList.

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
