// ListCtrlEx.h : header file
//

#if !defined(AFX_LISTCTRLEX_H__14D0592D_666B_11D1_B5EE_080009BD5569__INCLUDED_)
#define AFX_LISTCTRLEX_H__14D0592D_666B_11D1_B5EE_080009BD5569__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxdisp.h>
#include <list> // STL list

extern UINT msgListCtrlEx_RBtn; // registered Windows message
extern UINT msgListCtrlEx_LBtnDblClk; // registered Windows message
extern UINT msgListCtrlEx_DeleteKey; // registered Windows message
extern UINT msgListCtrlEx_ReturnKey; // registered Windows message

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx window

#define LVEX_HIGHLIGHT_NORMAL       (0)
#define LVEX_HIGHLIGHT_ALLCOLUMNS   (1)
#define LVEX_HIGHLIGHT_ROW          (2)

#define LVEX_DISPLAY_NORMAL     (0)
#define LVEX_DISPLAY_BARGRAPH   (1)
#define LVEX_DISPLAY_IMAGELIST  (2)

typedef struct tagSortInfo
{
	int	nCol;
	bool bAscending;
} SortInfo;
	
class CListCtrlEx : public CListCtrl
{
public:	
	CListCtrlEx();
	virtual ~CListCtrlEx();

// Overrides
	BOOL DeleteItem(int nItem);
	BOOL DeleteAllItems();

// New stuff
	int SetHighlightType(int hilite);
	BOOL SetColumnAlignment(unsigned int nCol, int fmt);
	CImageList * GetColImageList(unsigned int nCol);
	bool SetColImageList(unsigned int nCol, CImageList *pImageList);
	unsigned int SetColDisplayType(unsigned int nCol, unsigned int nType);
	unsigned int GetColDisplayType(unsigned int nCol);
	void SetCurSel(int nIdx, bool bFocus = true);

	void SetGraphBorderColor(COLORREF);
	void SetGraphCompletedColor(COLORREF);
	void SetGraphRemainColor(COLORREF);
	void SetGraphUseBorder(bool);

	void SetLinePerRow(int nLines);

	void SetHorizGrid(bool);

	bool SetEatReturnKey(bool bEat);

	bool GetCurSel(std::list<int> *pSelList);
	bool DeleteItems(const std::list<int>& InList);

	void NotifyInsertedCol(unsigned int nCol);
	void NotifyDeletedCol(unsigned int nCol);

	void ResetSortColumn();

protected:	
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	void CListCtrlEx::DrawItemLabel(CDC *pDC,
									CRect &LabelRct,
									CString &LabelStr,
									UINT nColAlignment,
									unsigned int nColumn);
	void RepaintSelectedItems();
	void InsertArr(unsigned int *& pArr, unsigned int &nLen, unsigned int nDefault, unsigned int nIdx);
	void DeleteArr(unsigned int *& pArr, unsigned int &nLen, unsigned int nIdx);

	int HitTestEx(CPoint point);

	virtual int CompareItems(LPARAM lpOne, LPARAM lpTwo, int nCol);
	static int CALLBACK CompareCB(LPARAM lpOne, LPARAM lpTwo, LPARAM lpArg);
	SortInfo m_SortInfo;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlEx)
	public:
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	int nSortedCol;
	bool bSortAscending;

	bool m_bHorzGrid;
	int m_nLinePerRow;
	int  m_nHighlight;		// Indicate type of selection highlighting
	unsigned int *m_DispTypeArr;
	unsigned int m_DispTypeArrLen;
	unsigned int m_nLabelOffset;
	unsigned int m_MinRowHeight;
	unsigned int m_ImageHeightArrLen;
	unsigned int * m_ImageHeightArr;
	unsigned int m_ImageWidthArrLen;
	unsigned int * m_ImageWidthArr;
	void SetColImageHeight(unsigned int nCol, unsigned int nHeight);
	void SetColImageWidth(unsigned int nCol, unsigned int nWidth);
	unsigned int GetColImageWidth(unsigned int nCol);
	unsigned int GetColImageHeight(unsigned int nCol);
	CImageList ** m_DispImageListArr;
	unsigned int m_DispImageListArrLen;
	unsigned int m_GraphHeight;
	COLORREF m_GraphBorderColor, m_GraphCompletedColor, m_GraphRemainColor;
	bool m_GraphUseBorder;
	bool m_bEatReturnKey;

	//{{AFX_MSG(CListCtrlEx)
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC *);
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnLButtonDblClk(UINT, CPoint);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG

	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM);
	afx_msg void MeasureItem ( LPMEASUREITEMSTRUCT lpMeasureItemStruct );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRLEX_H__14D0592D_666B_11D1_B5EE_080009BD5569__INCLUDED_)
