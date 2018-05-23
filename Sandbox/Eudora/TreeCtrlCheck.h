#if !defined(AFX_TREECTRLCHECK_H__2F3939AC_AFAD_11D1_94A9_00805F9BF4D7__INCLUDED_)
#define AFX_TREECTRLCHECK_H__2F3939AC_AFAD_11D1_94A9_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// TreeCtrlCheck.h : header file
//

extern UINT msgTreeCtrlCheck_CheckChange; // registered Windows message

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlCheck window

#define STATETOINDEXIMAGEMASK(i) ((i) >> 12)

class CTreeCtrlCheck : public CTreeCtrl
{
public:
// Construction
	CTreeCtrlCheck();
	virtual ~CTreeCtrlCheck();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeCtrlCheck)
	//}}AFX_VIRTUAL

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
	
	BOOL SetContainerFlag(BOOL bContainers = true);
	BOOL SetSingleCheck(BOOL bSingleCheck = true);

	BOOL ExpandAll();

	BOOL InitImageLists(UINT nImageBitmapID, int cImageX, COLORREF crImageMask,
							UINT nStateBitmapID, int cStateX, COLORREF crStateMask );
	BOOL InitItemImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask);
	BOOL InitStateImageList(UINT nImageBitmapID, int cImageX, COLORREF crImageMask);


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

	//{{AFX_MSG(CTreeCtrlCheck)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREECTRLCHECK_H__2F3939AC_AFAD_11D1_94A9_00805F9BF4D7__INCLUDED_)
