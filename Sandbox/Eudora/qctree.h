#ifndef __QCTREE_H__
#define __QCTREE_H__


// qctree.h : header file
//

#include "SafetyPal.h"

///////////////////////////////////////////////////////////////////////
// QCTreeOleDropTarget
//
// Private class that overrides CView-based implementation of 
// COleDropTarget to work with the QCTreeCtrl.
///////////////////////////////////////////////////////////////////////
class QCTreeOleDropTarget : public COleDropTarget
{
	//
	// This is a private class (i.e., a class with private constructors
	// and destructors).  So we need to declare somebody as a friend
	// in order to instantiate ourselves.
	//
	friend class QCTreeCtrl;

private:
	QCTreeOleDropTarget()
	{
	}
	virtual ~QCTreeOleDropTarget()
	{
	}

public:
	// Overridables
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
								   DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
								  DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
						DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
								DROPEFFECT dropDefault, DROPEFFECT dropList,
								CPoint point);
	/* FORNOW
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point);
	FORNOW */

private:
	QCTreeOleDropTarget(const QCTreeOleDropTarget&);	// not implemented
	void operator=(const QCTreeOleDropTarget&);		// not implemented
};


class CWazooWnd;

///////////////////////////////////////////////////////////////////////
// QCTreeCtrl
//
// The main tree control implementation class.  This is a smart 
// Eudora mailbox-specific tree control that is meant to be embedded
// in a parent "dialog" class.
///////////////////////////////////////////////////////////////////////
class QCTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(QCTreeCtrl)

public:
	QCTreeCtrl();
	virtual ~QCTreeCtrl();

	virtual BOOL Init(void);		// must be called to initialize drag and drop support

	CWazooWnd* GetParentWazooWnd() const;


// Attributes
public:

// Operations
public:
	//
	// Overridables for drag drop implementation, which is modeled
	// after the CView implementation.  Subclasses should override
	// these methods if it is a drop target for drag and drop
	// operations.  These base class methods are stubs which don't do
	// anything.
	//
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

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QCTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:

protected:
	//
	// AutoScroll support.
	//
	BOOL IsAutoScrollTimerRunning() const;
	BOOL StartAutoScrollTimer();
	void KillAutoScrollTimer();

	UINT m_uAutoScrollTimerInterval;

	virtual void DoSomethingWhileUserPausedMouseAtPoint(CPoint point);

	//{{AFX_MSG(QCTreeCtrl)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	//
	// Auto-scroll support
	//
	enum AutoScrollZone
	{
		ZONE_NONE = 0,		// not on auto-scroll edge
		ZONE_TOP,
		ZONE_TOP_LEFT,
		ZONE_TOP_RIGHT,
		ZONE_BOTTOM,
		ZONE_BOTTOM_LEFT,
		ZONE_BOTTOM_RIGHT,
		ZONE_LEFT,
		ZONE_RIGHT
	};

	enum
	{
		AUTOSCROLL_TIMER_EVENT_ID = 1000
	};

	AutoScrollZone GetAutoScrollZone(const CPoint& clientPt) const;

	UINT m_idAutoScrollTimer;
	CPoint m_PreviousAutoScrollPoint;

	//
	// Drop target support.
	//
	QCTreeOleDropTarget m_dropTarget;
	CSafetyPalette m_Palette;
};


/////////////////////////////////////////////////////////////////////////////
#endif // __QCTREE_H__
