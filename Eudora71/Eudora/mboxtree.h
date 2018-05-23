// mboxtree.h
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

#ifndef __MBOXTREE_H__
#define __MBOXTREE_H__

#include "QCMailboxTreeCtrl.h"
#include "QICommandClient.h"
#include "QCImageList.h"

#include "QCFindMgr.h"

class QCMailboxCommand;
class CMboxTreeItemData;

#ifdef IMAP4
class QCImapMailboxCommand;
#endif

///////////////////////////////////////////////////////////////////////
// CMboxTreeCtrl
//
// The main tree control implementation class.  This is a smart 
// Eudora mailbox-specific tree control that is meant to be embedded
// in a parent container window of any sort.
///////////////////////////////////////////////////////////////////////
class CMboxTreeCtrl : public QCMailboxTreeCtrl, public QICommandClient
{
	DECLARE_DYNAMIC(CMboxTreeCtrl)

	HTREEITEM FindPathname(HTREEITEM hItem, LPCSTR szPathname, BOOL bInRecent = FALSE);

// Construction
public:

	//
	// Clients should NOT call CTreeCtrl::DeleteItem() or
	// CTreeCtrl::DeleteAllItems() directly, under pain of leaky
	// memory!
	//
	void CheckItemByMenuId(const char* itemFilename, BOOL isChecked);
	void RenameItemByMenuId(const char* itemFilename, const char* newItemLabel);
	void RetypeItemByMenuId(const char* itemFilename, ItemType newType);

	void OnEnterKeyPressed(void);
	void OnEscKeyPressed(void);

	HTREEITEM GetNextWrapItem(HTREEITEM hItem) const;


//FORNOW	void SaveVisualState(void);
//FORNOW	void RestoreVisualState(void);
//FORNOW	void SetSelectedItemFilename(const CString& newFilename);

	CMboxTreeCtrl();

#ifdef IMAP4
	BOOL CMboxTreeCtrl::ImapNotify( 
			QCCommandObject*	pObject,
			COMMAND_ACTION_TYPE	theAction,
			void*				pData);

#endif // IMAP4

	virtual void Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

// Attributes
public:

// Operations
public:
	// drag drop implementation (parallels CView implementation)
	virtual BOOL OnDrop(COleDataObject* pDataObject,
						DROPEFFECT dropEffect, 
						CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject,
								   DWORD dwKeyState, 
								   CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
								  DWORD dwKeyState, 
								  CPoint point);
	virtual void OnDragLeave();

	// Auto-close operation to support drag and drop
	BOOL AutoCloseOpenedFolders();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMboxTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMboxTreeCtrl();


	// Generated message map functions
protected:

	//
	// Override base class hook to perform auto-open action.
	//
	virtual void DoSomethingWhileUserPausedMouseAtPoint(CPoint pt);

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	//{{AFX_MSG(CMboxTreeCtrl)
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg long OnRightButtonDown(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCmdOpen();
	afx_msg void OnCmdFindMsgs();
	afx_msg void OnCmdNew();
	afx_msg void OnCmdDelete();
	afx_msg void OnCmdRename();
	afx_msg void OnUpdateCmdEmptyTrash(CCmdUI* pCmdUI);
	afx_msg void OnCmdEmptyTrash();
	afx_msg void OnUpdateCmdTrimJunk(CCmdUI* pCmdUI);
	afx_msg void OnCmdTrimJunk();
	afx_msg void OnCmdUpdateBySelection(CCmdUI* pCmdUI);
	afx_msg BOOL OnDynamicCommand(UINT uID);
	afx_msg void OnUpdateDynamicCommand(CCmdUI* pCmdUI);
	//}}AFX_MSG

#ifdef IMAP4
	// Command handlers for IMAP stuff.
	void OnCmdModifyImapAccount();
	void OnCmdImapRefresh();
	void OnCmdImapResync();
	void OnCmdImapResyncTree();
	void OnCmdImapAutoSync();
	void OnCmdImapShowDeleted();
	void OnCmdImapFilters();

	// Helpers for IMAP command handlers.
	void DoIMAPCreate(CMboxTreeItemData *p_itemdata);
	BOOL DoIMAPRename(CMboxTreeItemData *p_itemdata, LPCSTR newname);
#endif // IMAP4

	DECLARE_MESSAGE_MAP()
private:
	BOOL DoCheckItemByMenuId(HTREEITEM hItem, const char* itemFilename, BOOL isChecked);
	BOOL DoRenameItemByMenuId(HTREEITEM hItem, const char* itemFilename, const char* newItemLabel);
	BOOL DoRetypeItemByMenuId(HTREEITEM hItem, const char* itemFilename, ItemType newType);
//	HTREEITEM GetLastParentItemForDepth(UINT itemDepth);
	BOOL SiblingHasSameName(HTREEITEM hItem, const char* labelName);
//FORNOW	void DoSaveFolderStates(HTREEITEM hItem);
//FORNOW	BOOL DoRestoreFolderStates(HTREEITEM hItem, const CString& fileName);
//FORNOW	BOOL DoRestoreSelection(HTREEITEM hItem, const CString& fileName);
	BOOL IsRecentMailboxItem(HTREEITEM hItem) const;

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#ifdef IMAP4
	// private
	void		ImapHandleDblclkFolder(HTREEITEM h_selitem, UINT nFlags, CPoint point, CMboxTreeItemData* p_itemdata);
	void		ImapHandleDblclkAccount(HTREEITEM h_selitem, UINT nFlags, CPoint point, CMboxTreeItemData* p_itemdata);

	// Called internally from OnCmdDelete().
	void		OnCmdImapDelete(void);
	// Called internally from OnMouseMove():
	void		HandleImapMouseMove(UINT nFlags, CPoint point, HTREEITEM h_target);
#endif // IMAP4
	
	HTREEITEM m_hDragMbox;
	CImageList* m_pDragImage;

//FORNOW	//
//FORNOW	// Used to save/restore the selection state, folder state, and
//FORNOW	// keyboard focus during the menu rebuild, new mailbox, and rename
//FORNOW	// mailbox operations.
//FORNOW	//
//FORNOW	CStringList m_openFolderList;
//FORNOW	CString m_selectedItemFilename;
//FORNOW	BOOL m_bHadFocusWhenSavingState;
};


/////////////////////////////////////////////////////////////////////////////
#endif // __MBOXTREE_H__
