// MBoxTreeCtrlCheck.h : header file
//

#ifndef _MBOXTREECTRLCHECK_H_
#define _MBOXTREECTRLCHECK_H_

#include "mboxtree.h"
#include <list.h>

extern UINT msgMBoxTreeCtrlCheck_CheckChange; // registered Windows message

/////////////////////////////////////////////////////////////////////////////
// CMBoxTreeCtrlCheck window

#define STATETOINDEXIMAGEMASK(i) ((i) >> 12)

class CMBoxTreeCtrlCheck : public CMboxTreeCtrl
{
public:
// Construction
	CMBoxTreeCtrlCheck();
	virtual ~CMBoxTreeCtrlCheck();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMBoxTreeCtrlCheck)
	//}}AFX_VIRTUAL

	// Override from QCMailboxTreeCtrl
	virtual BOOL GetItemStruct(TV_INSERTSTRUCT &tvstruct,
								ItemType itemType,
								const CString& itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);

	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(LPTV_INSERTSTRUCT lpInsertStruct);
	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage,
							int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,
							HTREEITEM hParent, HTREEITEM hInsertAfter);
	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT,
							HTREEITEM hInsertAfter = TVI_LAST);
	// WARNING: Overriding non-virtual function!
	HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage,
							HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const;
	HTREEITEM GetNextItem(HTREEITEM hItem) const;

	BOOL IsItemChecked(HTREEITEM hItem) const;
	HTREEITEM GetFirstCheckedItem() const;
	HTREEITEM GetNextCheckedItem(HTREEITEM hItem) const;
	void SetCheck(HTREEITEM hItem, BOOL bCheck = true);
	void CheckAll(bool bCheck = true);
	UINT GetCheckCount() const;
	BOOL GetSelectedList(list<QCMailboxCommand*> *hlist) const;

	BOOL SetContainerFlag(BOOL bContainers = true);
	BOOL SetSingleCheck(BOOL bSingleCheck = true);

	BOOL ExpandAll();

	BOOL InitImageLists(UINT nImageBitmapID, int cImageX, COLORREF crImageMask,
							UINT nStateBitmapID, int cStateX, COLORREF crStateMask );
	BOOL InitItemImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask);
	BOOL InitStateImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask);

	HTREEITEM FindCheck(QCMailboxCommand *pMbxCmd, bool bClearCheck = true);

// Generated message map functions
protected:
	typedef enum TCT_CheckState_tag { TCS_NOSTATE = 0, TCS_UNCHECKED, TCS_PARTIAL_CHECK, TCS_CHECKED } TCT_CheckState;
	typedef enum TCT_CheckAction_tag { TCA_CHECK = 0, TCA_PARTIAL_CHECK, TCA_UNCHECK, TCA_TOGGLE, TCA_REFRESH } TCT_CheckAction;

	HTREEITEM m_hCheckedItem; // Only valid if m_bSingleCheck == true
	BOOL m_bSingleCheck;

	void RefreshTreeChecks();
	bool DoAction(HTREEITEM hItem, TCT_CheckAction nAction);
	void RefreshNode(HTREEITEM hItem);
	TCT_CheckState GetRefreshState(HTREEITEM hItem, TCT_CheckState CurrentState = TCS_NOSTATE);
	BOOL m_bContainers;

	BOOL m_bInitStateImageList;
	CImageList m_imageState; // This contains the image list of bitmaps for the state (checkboxes)

	BOOL m_bInitItemImageList;
	CImageList m_imageItem; // This contains the image list of bitmaps for each item

	//{{AFX_MSG(CMBoxTreeCtrlCheck)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _MBOXTREECTRLCHECK_H_

