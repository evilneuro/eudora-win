// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Author:			Dean Hallman
//  Description:	Declarations for SECDockBar / SECMiniDockFrameWnd
//  Created:		May 1996
//

#ifdef WIN32

#ifndef __SBARDOCK_H__
#define __SBARDOCK_H__

#ifndef __AFXPRIV_H__
#include "afxpriv.h"  // CDockBar is a private, undocumented MFC class that
                      // we subclass.  Hence, the necessity to include afxpriv.h
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

class SECControlBar;
class SECControlBarInfo;

// Operation types for SECDockBar::SetControlBarWidthsInRow
#define SEC_DOCKBAR_MAX_CONTROL_BAR				1
#define SEC_DOCKBAR_EVEN_SPACE_CONTROL_BARS		2

/////////////////////////////////////////////////////////////////////////////
// SECDockBar 

class SECDockBar : public CDockBar
{
// Construction
public:
	DECLARE_DYNAMIC(SECDockBar)
	SECDockBar(BOOL bFloating = FALSE, BOOL bMDIChild = FALSE);
	// bFloating is true if attached to SECMiniDockFrameWnd
	// bMDIChild is true if attached to SECControlBarMDIChildWnd

// Attributes
public:

 	static BOOL m_bBorderClientEdge;

	class Splitter : public CObject	{

		public:
			enum Orientation { Horizontal, Vertical };
			enum Type { RowSplitter, BarSplitter };

		// Construction
		public:
			Splitter(Type type, Orientation orientation, const RECT & rect);

		// Attributes
		public:
			static const int cx;
			static const int cy;
			enum Orientation m_orientation;
			enum Type m_type;
			CRect m_rect;
			int m_nPos;
			BOOL m_bInUse;
			int m_nMin, m_nMax;

		protected:
			CRect m_rectLast;
			CSize m_sizeLast;
			BOOL m_bErase;          // TRUE if DrawTrackerRect is called for erasing
			BOOL m_bFinalErase;     // TRUE if DragTrackerRect called for final erase

 		// Implementation
		protected:
			void DrawTrackerRect(LPCRECT lpRect,
				CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd);


		// Operations
		public:
			virtual void Draw(CDC *pDC);
			int Track(CWnd* pWnd, CPoint point, CWnd* pWndClipTo);
	};

	class ClientEdge : public CObject	{

		public:
			enum Orientation { Horizontal, Vertical };

		// Construction
		public:
			ClientEdge(Orientation orientation, const RECT & rect);

		// Attributes
		public:
			static const int cx;
			static const int cy;
			enum Orientation m_orientation;
			CRect m_rect;
			BOOL m_bInUse;

		// Operations
		public:
			virtual void Draw(CDC *pDC);
	};

	CPtrArray		m_arrInvalidBars; // Array of control bars needing a repaint
	CObArray		m_arrEdges;
    CObArray        m_arrSplitters;
	BOOL			m_bMDIChild;

protected:
// Attributes
	AFX_SIZEPARENTPARAMS *m_pLayout;     
	CControlBar* m_pBarDocked;
	BOOL m_bProcessingDelayedInvalidates;
	BOOL m_bOptimizeNextRedraw;
	float m_fAdjustedMinPctWidth;

public:
	virtual BOOL IsControlBarAtMaxWidthInRow(SECControlBar* pBar);
	virtual BOOL IsOnlyControlBarInRow(SECControlBar* pBar);

public:
// Operations
	void DockControlBar(CControlBar* pBar, LPCRECT lpRect = NULL);
	virtual BOOL GetControlBarRow(CPtrList& rowList,SECControlBar* pBar);
	virtual void SetControlBarWidthsInRow(SECControlBar* pBar,USHORT uOperationType);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(SECDockBar)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~SECDockBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Drawing
	virtual void DoPaint(CDC* pDC);
	void InvalidateBar(CControlBar* pBar);
	void InvalidateBar(int nPos);
	void InvalidateToRow(int nPosRow);
	virtual void OnBarHideShow(CControlBar* pBar);
	virtual void ProcessDelayedInvalidates();

	// Splitter/Client Edge Related function
    Splitter * HitTest(CPoint pt);
    void StartTracking(Splitter* pSplit, CPoint pt);
    Splitter * GetSplitter(int i)
		{ return ((Splitter *)(m_arrSplitters[i])); };
    virtual void AddSplitter(Splitter::Type type, Splitter::Orientation orientation,
			int x1, int y1, int x2, int y2, int nPos);
	virtual void CalcTrackingLimits(Splitter* pSplitter);
    void DeleteAllSplitters();
	void BeginRecycleSplitters();
	void EndRecycleSplitters();
    virtual void AddClientEdge(ClientEdge::Orientation orientation,
			int x1, int y1, int x2, int y2);
    void DeleteAllEdges();
	void BeginRecycleEdges();
	void EndRecycleEdges();
	virtual void InvalidateCustomToolBarsInRow(SECControlBar* pBar);

    // Queries / Attributes
	BOOL IsNewBar(CControlBar* pBarToTest) const;
	CControlBar* NextBarThisRow(int nPos);
	CControlBar* NextVisibleBarThisRow(int nPos);
	CControlBar* PrevBarThisRow(int nPos);
	CControlBar* PrevVisibleBarThisRow(int nPos);
    SECControlBar * GetFirstControlBar ();
	BOOL BarIsNewToThisRow(CControlBar* pBarToTest, int nCurrentRow) const;
	int GetRowHeight(int nPos) const;
	void GetBarInfo(SECControlBarInfo* pInfo);
	void SetBarInfo(SECControlBarInfo* pInfo, CFrameWnd* pFrameWnd);

    // Layout
protected:
	virtual void NormalizeRow(int nPos, CControlBar* pBarDocked,
		int& nBarsBidirectional, int& nBarsUnidirectional);
	int PredictInsertPosition(CControlBar* pBarIns, CRect rect, CPoint ptMid);

	// implementation helpers
	virtual int Insert(CControlBar* pBarIns, int nInsCol, int nInsRow);
	int Insert(CControlBar* pBar, CRect rect, CPoint ptMid);

public:
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CRect CalcDockingLayout(CControlBar* pBarToDock, CRect& rectBar,
		CPoint pt, int& nPosDockingRow, CRect& prevFocusRect, CPoint& prevPt);
	void SetRowHeight(int nPos, int nRowHeight);
	void AdjustRowHeight(int nPos, int nWidth);
	virtual void DockControlBar(CControlBar* pBar, int nCol, int nRow);
	void ReDockControlBar(CControlBar* pBar, LPCRECT lpRect = NULL);
#if _MFC_VER >= 0x0420
	BOOL RemoveControlBar(CControlBar*, int nPosExclude = -1, BOOL bAddPlaceHolder = FALSE);
#else
	BOOL RemoveControlBar(CControlBar*, int nPosExclude = -1);
#endif
	virtual void OnSplitterMoved(Splitter* pSplitter, int nDelta);

    // Generated message map functions
protected:
    //{{AFX_MSG(SECDockBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

	friend class SECMiniDockFrameWnd;
	friend class SECDockContext;
};


class SECMiniDockFrameWnd : public CMiniDockFrameWnd
{
	DECLARE_DYNCREATE(SECMiniDockFrameWnd)

public:
// Construction
	SECMiniDockFrameWnd();
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);

// Implementation
public:
	SECDockBar m_wndSECDockBar;

// Operations
	virtual void RecalcLayout(CPoint point, BOOL bNotify = TRUE);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

	//{{AFX_MSG(CMiniFrameWnd)
	afx_msg void OnClose();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
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

#endif // __SBARDOCK_H__

#endif // WIN32