#ifndef __MBOXTREE_H__
#define __MBOXTREE_H__

//
// This whole file is applicable only to Win32 builds.
//
#ifdef WIN32

// mboxtree.h : header file
//

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

	HTREEITEM	FindPathname(
	HTREEITEM	hItem,
	LPCSTR		szPathname );

// Construction
public:

	//
	// Clients should NOT call CTreeCtrl::DeleteItem() or
	// CTreeCtrl::DeleteAllItems() directly, under pain of leaky
	// memory!
	//
	void CheckItemByMenuId(const CString& itemFilename, BOOL isChecked);

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
	afx_msg void OnCmdEmptyTrash();
	afx_msg void OnCmdUpdateBySelection(CCmdUI* pCmdUI);
	//}}AFX_MSG

#ifdef IMAP4
	// Command handlers for IMAP stuff.
	void OnCmdModifyImapAccount();
	void OnCmdImapRefresh();
	void OnCmdImapResync();
	void OnCmdImapFilters();
	// Special Updates:
#endif // IMAP4

	DECLARE_MESSAGE_MAP()
private:
	BOOL DoCheckItemByMenuId(HTREEITEM hItem, const CString& itemFilename, BOOL isChecked);
//	HTREEITEM GetLastParentItemForDepth(UINT itemDepth);
	BOOL SiblingHasSameName(HTREEITEM hItem, const CString& labelName);
//FORNOW	void DoSaveFolderStates(HTREEITEM hItem);
//FORNOW	BOOL DoRestoreFolderStates(HTREEITEM hItem, const CString& fileName);
//FORNOW	BOOL DoRestoreSelection(HTREEITEM hItem, const CString& fileName);

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



#endif // WIN32

/////////////////////////////////////////////////////////////////////////////
#endif // __MBOXTREE_H__
