// 
// Stingray Software Extension Classes
// Copyright (C) 1996 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the  help files for detailed information
// regarding using these classes.
// 
//  Author:		Dean Hallman

#ifdef WIN32

#ifndef __TRCTLX_H__
#define __TRCTLX_H__

#ifndef __SLSTCTL_H__
#include "slstctl.h"
#endif

#ifndef __TREENODE_H__
#include "TreeNode.h"
#endif

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

// Extended Styles Defined By SECTreeCtrl
#define TVXS_MULTISEL				0x0001
#define TVXS_WORDWRAP				0x0002
#define TVXS_COLUMNHEADER			0x0004
#define TVXS_FLYBYTOOLTIPS			0x0008

// Extra GetNextItem Codes
#define TVGN_FIRSTSELECTED			0x0020
#define TVGN_NEXTSELECTED			0x0021
#define TVGN_PREVIOUSSELECTED		0x0022
#define TVGV_FIRSTSELECTED			TVGN_FIRSTSELECTED
#define TVGV_NEXTSELECTED			TVGN_NEXTSELECTED
#define TVGV_PREVIOUSSELECTED		TVGN_PREVIOUSSELECTED

// HitTest Codes
#define TVHT_COLUMNSEP				0x1000
#define TVHT_COLUMNHEADING			0x2000

// Extra Mask Flags
#define TVIF_EX_STATEEX				0x0001

// Extra State Flags
#define TVIS_EX_DISABLED			0x0100
#define TVIS_EX_HIDDEN				0x0200

typedef struct tagTSMALLVITEMEX
{
	UINT maskEx;
	UINT stateEx;
	UINT stateExMask;
} TSMALLVITEMEX, FAR* LPTSMALLVITEMEX;

#define TV_ITEM_EX TSMALLVITEMEX

////////////////////////////////////////////////////////////////////////////
// class SECTreeCtrl

class SECTreeCtrl : public SECListCtrl
{

		DECLARE_DYNCREATE(SECTreeCtrl)

public:

	// State & Draw info for each node.
	class Node : public SECTreeNodeX
	{
	  public:
		Node();
		virtual ~Node();
		
	  // Attributes
	  public:
  		TV_ITEM m_item;
		BOOL    m_bHangingIndent;
		CString	m_strBeforeIndent,
				m_strAfterIndent;       
		int		m_cxTextBeforeIndent;   
		UINT	stateEx;

	  // Operations
	  public:
		BOOL    IsVisible();
		virtual BOOL OnNextSearchNode( WORD idSearch, SECTreeNode *pNode );
		BOOL    IsDisabled() const;
		BOOL    IsHidden() const;

		virtual BOOL IsExpanded() const;
		virtual void Expand(BOOL bExpand = TRUE);
	};

	class TvPaintContext : public SECListCtrl::LvPaintContext
	{
	public:
		TV_ITEM tvi;
		TV_ITEM_EX tvix;
		Node* pNode;

		TvPaintContext(CDC* pDC);
	};

// Attributes
protected:
	Node *m_pNodeRoot; //the root of all storage
	UINT m_nIndent;
	BOOL m_bHideDisabledItems;
	DWORD m_dwTreeStyle;
	DWORD m_dwTreeStyleEx;

// Operations
public:

	// Construction / Destruction
	SECTreeCtrl();
	virtual ~SECTreeCtrl();

	// Creation Members
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	BOOL Create(DWORD dwStyle, DWORD dwStyleEx, const RECT& rect, CWnd* pParentWnd, UINT nID);
	//Create a SECTreeCtrl on a dialog template. Call this func
	//from OnInitDialog and pass the IDC of MS's tree-ctrl from
	//create in dialog editor.
	BOOL SubclassTreeCtrlId( UINT idc, CWnd *pWndDlg );

	// Position/Dimension Accessors
	BOOL GetItemRect( HTREEITEM hti, LPRECT lpRect, UINT nCode ) const;
	UINT GetIndent() const;
	void SetIndent(UINT nIndent);

	// Tree Item Navigation Members
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const;
	HTREEITEM GetChildItem(HTREEITEM hItem) const;
	HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const;
	HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const;
	HTREEITEM GetParentItem(HTREEITEM hItem) const;
	HTREEITEM GetFirstVisibleItem() const;
    HTREEITEM GetLastVisibleItem(void) const;
	HTREEITEM GetNextVisibleItem(HTREEITEM hItem) const;
	HTREEITEM GetPrevVisibleItem(HTREEITEM hItem) const;
	HTREEITEM GetFirstSelectedItem() const;
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem) const;
	HTREEITEM GetPrevSelectedItem(HTREEITEM hItem) const;
	HTREEITEM GetCaretItem() const;
	HTREEITEM GetSelectedItem() const;
	HTREEITEM GetDropHilightItem() const;
	HTREEITEM GetRootItem(HTREEITEM hti = NULL) const;
	HTREEITEM GetNextItemInDisplayOrder(HTREEITEM hti) const;

	// Tree Item Data Accessors
	virtual BOOL GetItem(TV_ITEM* pItem, BOOL bCopyText = TRUE, BOOL bGetDispInfo = FALSE) const;
	virtual BOOL SetItem(const LV_ITEM* pLVI);
	virtual BOOL SetItemEx(TV_ITEM* pTVI, TV_ITEM_EX* pTVIX);
	virtual BOOL GetItemEx(TV_ITEM* pTVI, TV_ITEM_EX* pTVIX, BOOL bGetDispInfoEx = FALSE) const;
	virtual BOOL GetItem(LV_ITEM* pLVI, BOOL bCopyText = TRUE, BOOL bGetDispInfo = FALSE) const;
	virtual BOOL GetItemEx(LV_ITEM* pLVI, LV_ITEM_EX* pLVIX, BOOL bGetDispInfoEx = FALSE) const;
	virtual BOOL SetItemEx(LV_ITEM* pLVI, const LV_ITEM_EX* pLVIX);
	BOOL GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage) const;
	UINT GetItemState(HTREEITEM hItem, UINT nStateMask) const;
	DWORD GetItemData(HTREEITEM hItem) const;
	BOOL SetItem(TV_ITEM* pItem);
	BOOL SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR lpszItem, int nImage, 
		int nSelectedImage,	UINT nState, UINT nStateMask, LPARAM lParam);
	BOOL SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage);
	BOOL SetItemState(HTREEITEM hItem, UINT nState, UINT nStateMask);
	BOOL SetItemData(HTREEITEM hItem, DWORD dwData);
	Node* GetNode( HTREEITEM hti ) const;
	Node* GetNode( Item* pItem ) const;
	Node* GetNodeAt( int nIndex ) const;
	HTREEITEM GetItemAt(int nIndex) const;
	BOOL IsExpanded(HTREEITEM hItem) const;
	virtual BOOL IsCallbackItem(int nIndex) const;

	// Text accessors
	// WARNING: Be sure to read the comments associated with these
	//   functions in the implementation file.  These functions have
	//   a somewhat different purpose than you're probably assuming.
	CString GetItemText(HTREEITEM hItem, int iSubItem = 0) const;
	BOOL SetItemText(HTREEITEM hItem, LPCTSTR lpszItem);
	BOOL SetItemText(HTREEITEM hItem, int nSubItem, LPCTSTR lpszItem);
	BOOL SetItemString(HTREEITEM hti, int nSubItem, const CString& strItem);
	BOOL GetItemString(HTREEITEM hti, int nSubItem, CString& strItem);

	// Tree Item Management
	HTREEITEM InsertItem(LPTV_INSERTSTRUCT lpInsertStruct);
	HTREEITEM InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage, 
		int nSelectedImage,	UINT nState, UINT nStateMask, LPARAM lParam, 
		HTREEITEM hParent, HTREEITEM hInsertAfter);
	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT, 
		HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage,
		HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	BOOL DeleteItem(HTREEITEM hItem);
	BOOL DeleteAllItems();

	// Tree Item Operations
	BOOL Expand(HTREEITEM hItem, UINT nCode, BOOL bRedraw = TRUE);
	BOOL Select(HTREEITEM hItem, UINT nCode);
	BOOL SelectItem(HTREEITEM hItem);
	BOOL SelectDropTarget(HTREEITEM hItem);
	BOOL SelectSetFirstVisible(HTREEITEM hItem);
	HTREEITEM HitTest(CPoint pt, UINT* pFlags = NULL);
	HTREEITEM HitTest(TV_HITTESTINFO* pHitTestInfo);
	CImageList* CreateDragImage(HTREEITEM hItem);
	BOOL SortChildren(HTREEITEM hItem);
	BOOL EnsureVisible(HTREEITEM hItem, BOOL bParentVisible = FALSE);
	BOOL SortChildrenCB(LPTV_SORTCB pSort);
	CEdit* EditLabel(HTREEITEM hItem);
	BOOL SetFirstVisible(HTREEITEM hti);

	//Multiple Selected
	BOOL SelectAllVisibleChildren(HTREEITEM hti);
	BOOL SelectItemRange( HTREEITEM htiFirst, HTREEITEM htiLast, BOOL bSelect);
	BOOL IsSelected(HTREEITEM hti) const;

	//Hidden items
	BOOL      HideItem( HTREEITEM hti, BOOL bHide );
	BOOL      IsHidden( HTREEITEM hti ) const;
	HTREEITEM GetFirstHiddenItem(void) const;
	HTREEITEM GetNextHiddenItem(HTREEITEM) const;
	BOOL      UnHideAllItems(void);

	//Disable items / shown in disabled text colour
	BOOL      DisableAllItems( BOOL bDisable );
	BOOL      DisableItem( HTREEITEM hti, BOOL bDisable );
	BOOL      IsDisabled( HTREEITEM ) const;
	HTREEITEM GetFirstDisabledItem(void) const;
	HTREEITEM GetNextDisabledItem(HTREEITEM) const;
	void	  HideDisabledItems(BOOL bHide);
	BOOL      IsHideDisabledItems(void) const;

	// Tree Control Queries
	UINT GetCount() const;
	UINT GetVisibleCount() const;
	BOOL ItemHasChildrenOnDemand(HTREEITEM hItem) const;
	BOOL ItemHasChildren(HTREEITEM hItem) const;
	BOOL ItemExists(HTREEITEM hti, BOOL bExpanded=TRUE) const;

	// Style accessors
	virtual BOOL GetTreeCtrlStyles(DWORD& dwStyle, DWORD& dwStyleEx) const;
	virtual BOOL SetTreeCtrlStyles(DWORD dwStyle, DWORD dwStyleEx, BOOL bRedraw = TRUE);
	virtual DWORD GetTreeCtrlStyle() const;
	virtual DWORD GetTreeCtrlStyleEx() const;
	virtual BOOL SetTreeCtrlStyle(DWORD dwStyle, BOOL bRedraw = TRUE);
	virtual BOOL SetTreeCtrlStyleEx(DWORD dwStyleEx, BOOL bRedraw = TRUE);
	virtual BOOL ModifyTreeCtrlStyle(DWORD dwRemove, DWORD dwAdd, BOOL bRedraw = TRUE);
	virtual BOOL ModifyTreeCtrlStyleEx(DWORD dwRemoveEx, DWORD dwAddEx, BOOL bRedraw = TRUE);
	virtual BOOL ModifyTreeCtrlStyles(DWORD dwRemove, DWORD dwAdd,
							DWORD dwRemoveEx, DWORD dwAddEx, BOOL bRedraw = TRUE);

    //Filter a level (0=root, 1=children of root, etc) from visible view
	void      SetFilterLevel(WORD wLevel);
    WORD      GetFilterLevel(void) const;

	// Redraw
	virtual BOOL Update( HTREEITEM hti, BOOL bLabelOnly = FALSE, BOOL bEraseBkgnd = TRUE, BOOL bUpdateBelow = FALSE );
	inline BOOL InvalidateItem(HTREEITEM hti) { Update(hti); return TRUE; };

// Implementation
protected:

	// Image List
	void RemoveImageList(int nImageList);

	// Paint Context members
	virtual BOOL LoadPaintContext(LvPaintContext* pPC);
	virtual LvPaintContext* CreatePaintContext(CDC* pDC);

	// Layout and Owner Draw Members
	void MeasureText(CDC* pDC, Node* pNode, const CString& strText, CRect& rectText) const;
	virtual void DrawHierarchyLines(LvPaintContext* pPC);
	virtual void DrawButton(LvPaintContext* pPC);
	virtual void MeasureSubItem(LvPaintContext* pPC, UINT& cySubItem);
    virtual void DrawSubItem(LvPaintContext* pPC);

	virtual void DrawHorzHierarchyLine(CDC* pDC, BOOL bSelected, int x1, int x2, int y);
	virtual void DrawVertHierarchyLine(CDC* pDC, BOOL bSelected, int x1, int x2, int y);

	// Keyboard handlers
	virtual void OnReturnKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnHomeKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnEndKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnLeftKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnBackKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnRightKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);

	// Tree Control Notifications (sent to parent)
	virtual void NotifyBeginDrag(int nDragItem, POINT ptDrag) const;
	virtual BOOL NotifyBeginLabelEdit(int nEditItem) const;
	virtual void NotifyBeginRDrag(int nDragItem, POINT ptDrag) const;
	virtual void NotifyDeleteItem(int nDelItem) const;
	virtual BOOL NotifyEndLabelEdit(int nEditItem, int iSubItem, CString& strEdit, BOOL bCommit) const;
	virtual void NotifyGetDispInfo(TV_ITEM *pTVI) const;
	virtual void NotifyItemExpanded(HTREEITEM hti, UINT nCode) const;
	virtual BOOL NotifyItemExpanding(HTREEITEM hti, UINT nCode) const;
	virtual void NotifyKeyDown(WORD wVKey) const;
	virtual void NotifySelChanged(int nNewItem, int nOldItem, SelectAction sa) const;
	virtual BOOL NotifySelChanging(int nNewItem, int nOldItem, SelectAction sa) const;
	virtual void NotifySetDispInfo(TV_ITEM* pTVI) const;
	virtual void NotifyGetDispInfoEx(TV_ITEM_EX* pTVIX) const;
	virtual void NotifySetDispInfoEx(const TV_ITEM_EX* pTVIX) const;

// Operations
protected:
	BOOL AddNodeToListBox(   Node *ptcnAdd );
	BOOL DelNodeFromListBox( Node *ptcnDel );
	void InitRootNode(void);
	BOOL DeleteUnChildedItem( HTREEITEM hti, BOOL bDelFromListBox=TRUE );

	BOOL AddChildNodesToListBox( Node *pNode, BOOL bExpand );
	BOOL NodeInListBox( Node *pNode ) const;

    BOOL GetIndexSelItems( CWordArray *pWa ) const;
    void DelNonSiblingsFromListBox( Node *);
    HTREEITEM ExpandSelectedItem( UINT nCode );
    void CollapseSelectedItemThenSelectParent(void);
    void AddSiblingsToListBox( Node *);
    BOOL FilterListBox();
	void AddChildrenToListCtrl(Node* pParent, int& nIndex);
	void RemoveChildrenFromListCtrl(Node* pParent, int nParent);
    int  ItemIndex(HTREEITEM, int nStartFrom = 0) const;
    int  NodeIndex(Node *, int nStartFrom = 0) const;

	void CalcButtonRect( CRect *pRectButton, int iItem, const CRect &rcItem ) const;

	BOOL SelectCaretItem( HTREEITEM hti );

	BOOL ToolTipManager( CPoint pt );
	BOOL ShouldBeInListBox( HTREEITEM hti ) const;

// Oversome overrides
protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECTreeCtrl)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual void NotifyParent_DeleteItem(HTREEITEM hti) const;
    virtual Node *NewRefTreeCtrlNode(void) const { return new Node; }

	// Generated message map functions
protected:
	//{{AFX_MSG(SECTreeCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern void DDX_Control( CDataExchange* pDX, 
		     int            nIDC, 
			 SECTreeCtrl    &TreeCtrlX );

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TRCTLX_H__

#endif // WIN32
