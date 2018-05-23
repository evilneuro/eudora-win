#ifndef __RESIZBAR_H__
#define __RESIZBAR_H__
// resizbar.h : header file
//
// CResizableDialogBar 
// Encapsulates all the generic "resizing" logic for a resizable,
// dockable dialog-based control bar.  This class is not fully generic
// since it can only be docked on the left or right (it cannot docked
// on the top or bottom.


/////////////////////////////////////////////////////////////////////////////
// CResizableDialogBar window

class CResizableDialogBar : public CDialogBar
{
	DECLARE_DYNAMIC(CResizableDialogBar)
// Construction
public:
	BOOL Init(CWnd* pParentWnd, UINT nIDTemplate);
	void LoadBarSizes(UINT nDockWidthId, UINT nDockHeightId, UINT nFloatWidthId, UINT nFloatHeightId);
	void SaveBarSizes(UINT nDockWidthId, UINT nDockHeightId, UINT nFloatWidthId, UINT nFloatHeightId);

	//redefine undocumented public virtual CControlBar::DoPaint()
	virtual void DoPaint(CDC* pDC);

	CResizableDialogBar();

	enum
	{
		MBOX_BORDER = 8,
		TRACKER_WIDTH = 5
	};

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizableDialogBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResizableDialogBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);

	// Generated message map functions
protected:
	virtual void OnDynamicLayout(const CSize& newSize) = 0;
	BOOL IsDockedOnLeft() const;
	
	//{{AFX_MSG(CResizableDialogBar)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	//
	// Data used while tracking a ghosted a resize bar.  This is
	// used only for resizing while docked -- the floating resize
	// is based more on the native CControlBar notifications.
	//
	// It is important to understand that all values in this
	// group are in terms of docking Frame coordinates.
	//
	struct TrackerData
	{
		BOOL isTracking;
		int leftLimit;
		int rightLimit;
		CSize mboxToFrameOffset;	// relative offset
		CRect rect;
		int startX;
		void MboxToFrame(CPoint& point)
		{
			// do Mbox to Frame coordinate mapping
			point.Offset(mboxToFrameOffset);
		}
	} m_trackerData;

	void DrawTrackerGhost(const CRect& rect);
	BOOL IsOnResizeBar(CPoint pt) const;
	CSize m_floatingSize;
	CSize m_dockedSize;
	CSize m_floatingGhostSize;
	HCURSOR m_hResizeCursor;
};

/////////////////////////////////////////////////////////////////////////////
#endif //__RESIZBAR_H__
