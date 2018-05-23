// qctree.h : header file
//
// Copyright (c) 1995-2001 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#ifndef __QCTREE_H__
#define __QCTREE_H__

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
	HTREEITEM				CopyItem(
									HTREEITEM			in_hItemToCopy,
									HTREEITEM			in_hNewParent,
									HTREEITEM			in_hAfter = TVI_LAST );
	HTREEITEM				CopyBranch(
									HTREEITEM			in_hBranchToCopy,
									HTREEITEM			in_hNewParent,
									HTREEITEM			in_hAfter = TVI_LAST );

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
	UINT m_uAutoExpandDelay;
	UINT m_uLastUniqueDropPointTime;
	HTREEITEM m_hLastDropPointItem;

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
