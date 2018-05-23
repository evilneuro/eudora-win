#ifndef __QC3DTABWND_H__
#define __QC3DTABWND_H__

// QC3DTabWnd.h : header file
//
// QC3DTabWnd
// QC customizations of the SEC3DTabWnd.


///////////////////////////////////////////////////////////////////////
// QC3DTabControl
//
///////////////////////////////////////////////////////////////////////
class QC3DTabControl : public SEC3DTabControl
{
	friend class QC3DTabWnd;
	DECLARE_DYNCREATE(QC3DTabControl)

// Construction
public:
	QC3DTabControl();
	virtual ~QC3DTabControl();

	HICON QCGetIcon(int nTabIndex) const;

	// Override virtuals from base class...
	virtual void ActivateTab(int nTab);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QC3DTabControl)
protected:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:

	// required implementation of a pure virtual
	//{{AFX_MSG(QC3DTabControl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:    
    // Needed to make this control an OLE data SOURCE (see OnMouseMove)
    COleDataSource m_OleDataSource;
	CPoint m_SavedMouseDownPoint;
	int m_nSavedTabIndex;
	enum
	{
		MOUSE_IDLE,
		MOUSE_DOWN_ON_TAB,
		MOUSE_IS_MOVING,
		MOUSE_IS_DRAGGING
	} m_MouseState;
};


///////////////////////////////////////////////////////////////////////
// QC3DTabWnd
//
///////////////////////////////////////////////////////////////////////
class QC3DTabWnd : public SEC3DTabWnd
{
	DECLARE_DYNCREATE(QC3DTabWnd)

// Construction
public:
	QC3DTabWnd();
	virtual ~QC3DTabWnd();

// Attributes
public:

// Operations
public:

	HICON QCGetIcon(int nTabIndex) const;
	BOOL QCTabHit(int nTabIndex, const CPoint& ptScreen);

	void SetTabLocation(DWORD dwTabLocation);
	DWORD GetTabLocation() const;
	void ShowTabs(BOOL bShowTabs);
	BOOL TabsAreVisible() const;
	void SetDraw3DBorder(BOOL bNew) { m_Draw3DBorder = bNew; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QC3DTabWnd)
protected:
	//}}AFX_VIRTUAL

	virtual BOOL CreateTabCtrl(DWORD dwStyle, UINT nID);
    virtual void RecalcLayout();

// Implementation
public:

	// Generated message map functions
protected:
	BOOL m_bShowTabs;
	BOOL m_Draw3DBorder;

	// required implementation of a pure virtual
	//{{AFX_MSG(QC3DTabWnd)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif //__QC3DTABWND_H__
