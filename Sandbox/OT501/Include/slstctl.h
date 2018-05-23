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

#ifndef __SLSTCTL_H__
#define __SLSTCTL_H__

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

// Extended Styles Defined By SECListCtrl
#define LVXS_FLYBYTOOLTIPS			0x0001
#define LVXS_OWNERDRAWVARIABLE		0x0002
#define LVXS_LINESBETWEENITEMS		0x0004
#define LVXS_LINESBETWEENCOLUMNS	0x0008
#define LVXS_WORDWRAP				0x0010
#define LVXS_HILIGHTSUBITEMS		0x0020
#define LVXS_FITCOLUMNSONSIZE		0x0040

// Extra Mask Flags
#define LVIF_EX_STATEEX				0x0001

// Extra State Flags
#define LVIS_EX_DISABLED			0x0100
#define LVIS_EX_HIDDEN				0x0200

// Extra Hit Test Codes
#define LVHT_COLUMNSEP				0x0800
#define LVHT_COLUMNHEADING			0x1000
#define LVHT_INSIDEITEMBOUNDS		0x2000
#define LVHT_ONITEMLEFT				0x4000
#define LVHT_ONITEMRIGHT			0x8000

#define LVXC_INTERNALBUFFERSIZE		2048

// Forward Declarations
class SECListCtrlTip;

// CEdit derivative to allow needed VK_* keyboard handlers
class SECListCtrlEdit : public CEdit
{
protected:
	//{{AFX_MSG(SECListCtrlEdit)
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};


//
// LV_ITEM_EX
//
//	We'd like to add disabled item and hidden item support but
//  the state field of the LV_ITEM structure has no more state
//	bits left.  They're all used.  So, to add disabled or hidden
//	states to items, we introduce an LV_ITEM_EX structure that
//	adds "stateEx" for extra states associated with each item.
//

typedef struct tagLVITEMEX
{
	UINT maskEx;
	UINT stateEx;
	UINT stateExMask;
} LVITEMEX, FAR* LPLVITEMEX;

#define LV_ITEM_EX LVITEMEX

///////////////////////////////////////////////////////////////////////
//
// SECListCtrl
//
//		The SECListCtrl is intended to be a drop-in replacement
//		for CListCtrl, but it isn't there yet.  Only the subset
//		of functionality required to implement the SECTreeCtrl
//		derivative is implemented at this time.  For this reason,
//		this class is not yet intended for general use.  A
//		future release of Objective Toolkit will implement the
//		remaining functionality.
//

class SECListCtrl : public CWnd
{

	DECLARE_DYNCREATE(SECListCtrl)

public:

	class SubItem
	{
	public:
		LPTSTR pszText;
		int cchTextMax;
		CRect rcText;

	public:

		SubItem();
		virtual ~SubItem();
	};
		
	class Item
	{
		friend class SECListCtrl;

	protected:
	    CPtrArray m_arrSubItems;
		LPTSTR pszText;
		UINT state;
		int iImage;
		LPARAM lParam;
		UINT m_cy;
		UINT stateEx;

	public:
		CRect rcText;
		CRect rcIcon;

		Item();
		virtual ~Item();

	    // SubItem data
		BOOL AddSubItem();
		BOOL DeleteSubItem(int iSubItem);
		BOOL GetSubItemCount(int iSubItem) const;
		int GetSubItemCount() const;
		SubItem* GetSubItem(int iSubItem) const;
		BOOL SetItemData(DWORD dwData) { lParam = (LPARAM)dwData; return TRUE; }
		DWORD GetItemData() const { return (DWORD)lParam; }
	};

	class Column
	{
	public:
		UINT mask;
		int fmt;
		int cx;
		CString	strText;
		int cchTextMax;
		int iSubItem;

		Column();
	};

	enum SelectAction
	{
		ByKeyboard,
		ByMouse,
		UnKnown
	};

	enum DrawAction
	{
		DrawEntire,
		DrawFocus,
		DrawSelect
	};

	friend class SECListCtrlEdit;

// Attributes
protected:

	COLORREF m_rgbText;
	COLORREF m_rgbTextBk;
	COLORREF m_rgbSelText;
	COLORREF m_rgbSelTextBk;
	COLORREF m_rgbDisabledText;
	COLORREF m_rgbIconBk;
	COLORREF m_rgbSelIconBk;
	COLORREF m_rgbToolTipBk;
	COLORREF m_rgbToolTipText;
	COLORREF m_rgbBk;
	CFont* m_pLabelFont;
	CFont* m_pHeaderFont;
    CImageList  *m_pGridImageList;
    BOOL m_bShowGridMode;
    BOOL m_bPaddingMode;
    BOOL m_bSameHeightMode;
    int m_heightMax;
	BOOL m_bShowHeader;
	int m_nStretchingColumn;
	int m_nTopIndex;
	int m_cyHeader;
	CPoint m_ptViewportOrg;
    HCURSOR m_hcurSplit;
	BOOL m_bColSepHit;
	DWORD m_dwListStyle;
	DWORD m_dwListStyleEx;
	CPtrArray m_arrColumns;
	CPtrArray m_arrItems;
	CDWordArray m_arrSelItems;
	int m_nPendingSelect;
	int m_nPendingEdit;
	int m_nActiveColumn;
	int m_nEditItem;
	UINT m_idEditLabelTimer;
	SECListCtrlEdit m_EditCtrl;
	CImageList *m_pImageList;
	CPoint m_ptButtonDown;
	SECListCtrlTip  *m_pToolTipWnd;
	BOOL m_bAbortPendingSelect;
	BOOL m_bDidProcessAndDestroyEditLabel;
	CString m_strOldEditText;
	UINT m_uiCallbackMask;
	UINT m_uiCallbackMaskEx;
	int m_nSubItemCount;
	LPTSTR m_pszInternalBuffer;
	DWORD m_dwScrollStyle;
	int m_nFocusIndex;
	int m_nDropHilite;
	

	//
	// "class LvPaintContext"
	//
	// This is an important class to understand.  The paint
	// context contains all information used in the process of
	// rendering a list view item.  It is initialized as the
	// first step of drawing an item (LoadPaintContext), and
	// referenced by all subsequent routines which carry out
	// the rendering.  There are a number of advantages to this
	// approach:
	//
	//  1) LoadPaintContext() is virtual, so derived classes can
	//     add paint context members into the derived class and
	//     initialize the derived paint context too.  This gaurantees
	//     all display info needed for the item about to be
	//     drawn is loaded before the painting begins.
	//  2) Eliminates the need for each drawing function to have
	//     many arguments.
	//  3) Prevents the various draw routines from recomputing
	//     the draw info again and again.  This is particularly
	//     important because each requery for drawing info may
	//     send another LVN_GETDISPINFO message to the parent.
	//  4) This is the most important justification for the
	//     paint context but is very difficult to state clearly.
	//     So bear with me.  There is an inherint ambiguity
	//     associated with having both an LVN_GETDISPINFO and
	//     a TVN_GETDISPINFO message (in SECTreeCtrl which
	//     derives from us).  When painting an item, if the
	//     item is a tree node, TVN_GETDISPINFO should be sent
	//     and a TV_ITEM structure filled out.  Otherwise,
	//     LVN_GETDISPINFO should be sent and LV_ITEM filled out.
	//     However, many of the routines defined here will only
	//     understand LV_ITEM and it's legal values.  For example,
	//     if you call PickTextColors() it will choose a color
	//     based on the presence of the LVIS_SELECTED state,
	//     But if you retrieved a TV_ITEM (which uses
	//     TVIS_SELECTED instead), you'll not be able to call
	//     PickTextColors() since TVIS_SELECTED has no meaning
	//     in the list view base class.  So, the real purpose of
	//     this class is it affords us a convenient way of
	//     translating derived display info to base class display
	//     info.  That is, the LoadPaintContext() override can send
	//     a LVN_GETDISPINFO or a TVN_GETDISPINFO as long as it
	//     also properly initalizes the paint context based upon the
	//     notification results.
	//

public:
	class LvPaintContext
	{
	public:
		LV_ITEM lvi;
		LV_ITEM_EX lvix;
		CDC* pdc;
		CRect rect;
		int iCol;
		DrawAction da;
		CFont* pFont;
		Item* pItem;
		SubItem* pSubItem;
		Column* pCol;
		COLORREF rgbText;
		COLORREF rgbTextBkgnd;
		COLORREF rgbItemBkgnd;
		COLORREF rgbIconBkgnd;

		LvPaintContext(CDC* pDC);
	};
	
// Operations
public:

	// Construction / Destruction
	SECListCtrl();
	virtual ~SECListCtrl();

	// Creation Members
	virtual BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	virtual BOOL Create( DWORD dwStyle, DWORD dwStyleEx, const RECT& rect, CWnd* pParentWnd, UINT nID );

	// Visual Attribute Accessors
	virtual COLORREF GetBkColor() const;
	virtual COLORREF GetTextColor() const;
	virtual COLORREF GetTextBkColor() const;
	virtual COLORREF GetSelTextColor() const;
	virtual COLORREF GetSelTextBkColor() const;
	virtual COLORREF GetDisabledTextColor() const;
	virtual COLORREF GetIconBkColor() const;
	virtual COLORREF GetSelIconBkColor() const;
	virtual COLORREF GetToolTipBkColor() const;
	virtual COLORREF GetToolTipTextColor() const;
	virtual BOOL SetBkColor(COLORREF rgbBk);
	virtual BOOL SetTextColor(COLORREF rgbText);
	virtual BOOL SetTextBkColor(COLORREF rgbTextBk);
	virtual BOOL SetSelTextColor(COLORREF rgbSelText);
	virtual BOOL SetSelTextBkColor(COLORREF rgbSelTextBk);
	virtual BOOL SetDisabledTextColor(COLORREF rgbDisabledText);
	virtual BOOL SetIconBkColor(COLORREF rgbIconBk);
	virtual BOOL SetSelIconBkColor(COLORREF rgbSelIconBk);
	virtual BOOL SetToolTipBkColor(COLORREF rgbToolTipBk);
	virtual BOOL SetToolTipTextColor(COLORREF rgbToolTipText);
	virtual CFont* GetLabelFont() const;
	virtual CFont* GetHeaderFont() const;
	virtual BOOL SetLabelFont(CFont* pFont);
	virtual BOOL SetHeaderFont(CFont* pFont);

	// Drawing Members
	virtual BOOL Update( int nItem, BOOL bLabelOnly = FALSE, BOOL bEraseBkgnd = TRUE, BOOL bUpdateBelow = FALSE );
	virtual BOOL RedrawItems( int nFirst, int nLast );

	// Position/Dimension Accessors
	virtual int GetTopIndex() const;
	virtual int ItemsThisPage();
	virtual int ItemsPrevPage();
	virtual int GetFirstItemLastPage() const;
	virtual int GetLastFullyVisibleItem();
	virtual int GetFirstItemNextPage() const;
	virtual int GetFirstItemPrevPage();
	virtual int GetItemAtPoint(const CPoint& pt);
	virtual void GetInsideRect(LPRECT lpRect) const;
	virtual BOOL SetItemHeight(int nIndex, UINT cyItemHeight) const;
	virtual BOOL GetOrigin(LPPOINT lpPoint) const;
	virtual int CalcRangeHeight(int nFirst, int nLast, BOOL bInclusive);
	virtual int CalcItemsInRange(int cyRange, int nStartItem, BOOL bDown);
	virtual void ReMeasureAllItems(); 

	// Column Management & Accessors
    virtual BOOL DeleteColumn( int nCol  );
	virtual BOOL DeleteColumn( const CString& strColumnHeading );
	virtual BOOL DeleteAllColumns();
    virtual BOOL AddColumn( const CString& strHeader, int fmt=LVCFMT_LEFT, int widthCol=-1, int iSubItem = -1);
    virtual BOOL AddColumn( Column* pCol );
    virtual BOOL InsertColumn( int nCol, Column* pCol, BOOL bUpdate = TRUE);
	virtual BOOL InsertColumn( int nCol, const CString& strHeader, int nFormat=LVCFMT_LEFT, int wWidth=-1, int iSubItem = -1, BOOL bUpdate = TRUE );
	virtual int GetColumnLeft( int nCol ) const;
	virtual int GetColumnRight( int nCol ) const;
	virtual void SetColumnWidth( int nCol, int width );
	virtual int GetColumnWidth( int nCol ) const;
	virtual void SetColumnFormat( int nCol, int fmt );
	virtual int GetColumnFormat( int nCol ) const;
	virtual void SetColumnHeading( int nCol, const CString& strHeading );
	virtual CString GetColumnHeading( int nCol ) const;
    virtual BOOL ColumnExists( const CString &strColTitle ) const;
	virtual int GetColumnCount() const;
    virtual BOOL FindColumn( const CString &strColumnHeading, int& nCol, int& iSubItem ) const;
	virtual void ShowHeader(BOOL b); //off by default
	virtual BOOL IsShowHeader() const;
	virtual Column* GetColumnAt(int nCol) const;

	// Items/SubItem Management & Accessors
	Item* GetItemAt(int nIndex) const;
	virtual int GetItemCount() const;
	virtual int GetSubItemCount() const;
	virtual BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam);
	virtual BOOL SetItem(const LV_ITEM* pLVI);
	virtual BOOL SetItemEx(LV_ITEM* pLVI, const LV_ITEM_EX* pLVIX);
	virtual BOOL GetItem(LV_ITEM* pLVI, BOOL bCopyText = TRUE, BOOL bGetDispInfo = FALSE) const;
	virtual BOOL GetItemEx(LV_ITEM* pLVI, LV_ITEM_EX* pLVIX, BOOL bGetDispInfo = FALSE) const;
	virtual BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);
	virtual int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const;
	virtual CString GetItemText(int nItem, int nSubItem) const;
	virtual BOOL DeleteItem( int nItem, int cCount = 1 );
	virtual BOOL DeleteAllItems();
	virtual UINT GetSelectedCount( ) const;
	virtual BOOL GetItemRect( int nItem, LPRECT lpRect, UINT nCode ) const;
	virtual BOOL GetItemRect( int nItem, int nCol, LPRECT lpRect, UINT nCode ) const;
	virtual int InsertItem(int nItem, const Item* pItem);
	virtual int InsertItem(int nItem, const CString& strItem);
	virtual BOOL IsSelected(int nIndex) const;
	virtual BOOL SelectFocusItem(int nIndex, BOOL bSelected = TRUE);
	virtual BOOL DeselectAllItems(int iExclude = -1);
	virtual BOOL SelectItemRange(int nFirst, int nLast, BOOL bSelect = TRUE);
	virtual int GetFocusIndex() const;
	virtual CSize GetLogicalSize();
	virtual BOOL SetItemState(int nItem, UINT nState, UINT nMask);
	virtual UINT GetItemState(int nItem, UINT nMask) const;
	virtual BOOL SetItemData(int nItem, DWORD dwData);
	virtual DWORD GetItemData(int nItem) const;
	virtual BOOL SelectItem(int nIndex, BOOL bSelect = TRUE);
	virtual UINT GetCallbackMask() const;
	virtual BOOL SetCallbackMask(UINT nMask);
	virtual UINT GetCallbackMaskEx() const;
	virtual BOOL SetCallbackMaskEx(UINT nMaskEx);
	virtual BOOL IsCallbackItem(int nItem) const;

	// Item Traversal
	//
	// There are two ways to traverse the selected items.  You can use
	// GetNextItem() passing LVNI_SELECTED or you can retrieve the
	// selection array.  GetNextItem() is useful when you want to find
	// items that have more than one state bit set concurrently.  If
	// you are only interested in selected items, GetSelectionArray
	// is faster.
	virtual int GetNextItem(int nItem, int nFlags = LVNI_ALL) const;
	inline CDWordArray* GetSelectionArray() { return &m_arrSelItems; };

	// Drag and Drop
	virtual BOOL SelectDropTarget(int nIndex);
	virtual int GetDropHilightIndex() const;

	// Image List
	virtual CImageList* GetImageList(UINT nImageList) const;
	virtual CImageList* SetImageList(CImageList* pImageList, int nImageListType);

	// Hit Testing
	virtual int HitTest(CPoint pt, UINT* pFlags, int& nHitCol) const;

	// Scrolling Members
	virtual void SetFirstVisible(int nIndex);
	virtual void ScrollHorz(int nPixels);
	virtual void ScrollVert(int nScrollItems, BOOL bFromBottom = FALSE);
	virtual BOOL EnsureVisible(int nItem, BOOL bPartialOk = FALSE);
	virtual BOOL EnsureRangeVisible(int nFirst, int nLast, BOOL bPartialOk = FALSE);

	// In-Place Editing Members
	virtual CEdit* EditLabel( int nItem, int nCol = 0 );
	virtual CEdit* GetEditControl();
	virtual BOOL KillEditControl();

	// Style accessors
	virtual BOOL GetListCtrlStyles(DWORD& dwStyle, DWORD& dwStyleEx) const;
	virtual BOOL SetListCtrlStyles(DWORD dwStyle, DWORD dwStyleEx, BOOL bRedraw = TRUE);
	virtual DWORD GetListCtrlStyle() const;
	virtual DWORD GetListCtrlStyleEx() const;
	virtual BOOL SetListCtrlStyle(DWORD dwStyle, BOOL bRedraw = TRUE);
	virtual BOOL SetListCtrlStyleEx(DWORD dwStyleEx, BOOL bRedraw = TRUE);
	virtual BOOL ModifyListCtrlStyle(DWORD dwRemove, DWORD dwAdd, BOOL bRedraw = TRUE);
	virtual BOOL ModifyListCtrlStyleEx(DWORD dwRemoveEx, DWORD dwAddEx, BOOL bRedraw = TRUE);
	virtual BOOL ModifyListCtrlStyles(DWORD dwRemove, DWORD dwAdd,
							DWORD dwRemoveEx, DWORD dwAddEx, BOOL bRedraw = TRUE);

	// Feature and Viewing Mode Enable/Disable Members
	virtual void EnableWordWrap(BOOL bEnable = TRUE);
	virtual BOOL IsWordWrapEnabled() const;
	virtual void EnableToolTips(BOOL bEnable = TRUE);
	virtual BOOL AreToolTipsEnabled() const;
	virtual void EnableHeaderCtrl(BOOL bEnable = TRUE);
	virtual BOOL IsHeaderCtrlEnabled() const;
	virtual void EnableMultiSelect(BOOL bEnable = TRUE);
	virtual BOOL IsMultiSelectEnabled() const;

	// Misc View Modes
    virtual void SetShowGridMode(BOOL b);
    virtual BOOL IsShowGridMode(void) const;
    virtual void SetPaddingMode(BOOL b);
    virtual BOOL IsPaddingMode(void) const;
    virtual void SetSameHeightMode(BOOL b);
    virtual BOOL IsSameHeightMode(void) const;

// Implementation
protected:

	// Allocation & Initialization
	virtual BOOL CreateScrollBarCtrl(DWORD dwStyle, UINT nID);
    virtual BOOL AllocateSubItems( Item* pItem ) const;
	virtual BOOL DeallocateSubItems( Item* pItem ) const;
	virtual int InitNewItem(Item* pItem) const;
	virtual Item* CreateNewItem();
	virtual void ShowScrollBarCtrl(int nBar, BOOL bShow = TRUE, LPSCROLLINFO lpScrollInfo = NULL);
	BOOL AddToSelItemsArray(int iItem);
	BOOL RemoveFromSelItemsArray(int iItem);
	BOOL FindInSelItemsArray(int iItem, int& nIndex) const;

	// Color/Font/Pen Pick Members
	virtual void PickItemBkgnd(LvPaintContext* pPC);
	virtual void PickIconBkgnd(LvPaintContext* pPC);
	virtual void PickTextColors(LvPaintContext* pPC);
	virtual void PickTextFont(LvPaintContext* pPC);
	virtual void PickItemLinePen(CPen*& pPen);
	virtual void PickColumnLinePen(CPen*& pPen);

	// Layout Members
	virtual void MeasureSubItem(LvPaintContext* pPC, UINT& cySubItem);
	virtual void MeasureItem(LvPaintContext* pPC);
	virtual void MeasureItem(int nItem);
	virtual void RecalcLayout();

	// Column Resize Tracking
	virtual int TrackColumnResize(CPoint point, int nCol);

	// Owner Draw Members
	// Note, the draw members below reference the paint context for
	// display info needed for the item rendering.
	virtual BOOL LoadPaintContext(LvPaintContext* pPC);
	virtual LvPaintContext* CreatePaintContext(CDC* pDC);

	virtual void DrawItemLines(CDC* pDC, int nItem, const CRect& rcRow);
	virtual void DrawColumnLines(CDC* pDC, int nItem, const CRect& rcRow);
	virtual void DrawSubItem(LvPaintContext* pPC);
	virtual void DrawItem(LvPaintContext* pPC);
	virtual void DrawInvalidItems(LvPaintContext* pPC);
	virtual void DrawHeader(LvPaintContext* pPC);

	// In-Place Editing Implementation Members
	virtual BOOL ProcessAndDestroyEditLabel(BOOL bSaveLabel=TRUE);

	// Scrolling Implementation Members
	virtual void RecalcScrollBars();

	// Keyboard handlers
	virtual void OnUpKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnDownKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnF2KeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnReturnKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnHomeKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnEndKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnLeftKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnBackKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);
	virtual void OnRightKeyPressed(int nItem, int nSubItem, BOOL bShift, BOOL bCtrl, UINT nRepCnt, UINT nFlags);

	// List Control Notifications (sent to parent)
	virtual void NotifyNMHDR( UINT wNotify ) const;
	virtual void NotifyBeginDrag(int nDragItem, POINT ptDrag) const;
	virtual BOOL NotifyBeginLabelEdit(int nEditItem) const;
	virtual void NotifyBeginRDrag(int nDragItem, POINT ptDrag) const;
	virtual void NotifyDeleteItem(int nDelItem) const;
	virtual BOOL NotifyEndLabelEdit(int nEditItem, int iSubItem, CString& strEdit, BOOL bCommit) const;
	virtual void NotifyKeyDown(WORD wVKey) const;
	virtual void NotifySelChanged(int nNewItem, int nOldItem, SelectAction sa) const;
	virtual BOOL NotifySelChanging(int nNewItem, int nOldItem, SelectAction sa) const;
	virtual void NotifyGetDispInfo(LV_ITEM* pLVI) const;
	virtual void NotifySetDispInfo(const LV_ITEM* pLVI) const;
	virtual void NotifyGetDispInfoEx(LV_ITEM_EX* pLVIX) const;
	virtual void NotifySetDispInfoEx(const LV_ITEM_EX* pLVIX) const;
	virtual void NotifyInsertItem(int nItem) const;

// MFC Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECListCtrl)
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	//}}AFX_VIRTUAL

// MFC Generated message map functions
protected:
	//{{AFX_MSG(SECListCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void EditLabel_OnKillFocus(void);
	afx_msg void EditLabel_OnUpdate(void);
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus( CWnd *pwndNew );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SLSTCTL_H__

#endif // WIN32
