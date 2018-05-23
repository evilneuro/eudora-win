// qctree.cpp: implementation
//
// QCTreeCtrl
// Base class for "smart" tree control which supports generic auto-scroll
// and forwards OLE drop target messages.
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

#include "stdafx.h"

#include "qctree.h"
#include "eudora.h"		// for IsVersion4()
#include "rs.h"

#include "WazooWnd.h"
#include "MainFrm.h"

#include "DebugNewHelpers.h"

IMPLEMENT_DYNAMIC(QCTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(QCTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(QCTreeCtrl)
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER_QUERY_PALETTE, OnMyQueryNewPalette)
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// QCTreeCtrl

QCTreeCtrl::QCTreeCtrl() :
	m_idAutoScrollTimer(0),
	m_uAutoScrollTimerInterval(GetIniLong(IDS_INI_TREE_CONTROL_AUTO_SCROLL_DELAY)),
	m_uAutoExpandDelay(GetIniLong(IDS_INI_TREE_CONTROL_AUTO_EXPAND_DELAY)),
	m_uLastUniqueDropPointTime(0),
	m_hLastDropPointItem(NULL),
	m_PreviousAutoScrollPoint(-1, -1)
{
}

QCTreeCtrl::~QCTreeCtrl()
{
}


////////////////////////////////////////////////////////////////////////
// Init [public, virtual]
//
// Must be called after window is created to register this window
// as an OLE drop target.
////////////////////////////////////////////////////////////////////////
BOOL QCTreeCtrl::Init()
{
	ASSERT_VALID(this);
	ASSERT(GetSafeHwnd() != NULL);
	
	//
	// Set "thin" font for old Win 3.x GUI.
	//
	if (! IsVersion4())
		SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));

	// register drop target
	VERIFY(m_dropTarget.Register(this));

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetParentWazooWnd [public]
//
// Returns a pointer to the parent WazooWnd object, if any.  Otherwise,
// return NULL if this tree control is not a child of a WazooWnd.
////////////////////////////////////////////////////////////////////////
CWazooWnd* QCTreeCtrl::GetParentWazooWnd() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	CWnd* pParentWnd = GetParent();  // start with one parent up
	while (pParentWnd != NULL)
	{
		if (pParentWnd->IsKindOf(RUNTIME_CLASS(CWazooWnd)))
			return (CWazooWnd *) pParentWnd;
		pParentWnd = pParentWnd->GetParent();
	}
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// GetAutoScrollZone [private]
//
// Given a point in the client area, returns a status code
// based on the position of the cursor within the client area.
////////////////////////////////////////////////////////////////////////
QCTreeCtrl::AutoScrollZone QCTreeCtrl::GetAutoScrollZone(const CPoint& clientPt) const
{
	const int DEFAULT_ZONE_WIDTH = 24;
	int zone_width = DEFAULT_ZONE_WIDTH;

	CRect client_rect;
	GetClientRect(&client_rect);
	if (client_rect.PtInRect(clientPt))
	{
		//
		// If necessary, adjust zone width for ridiculously small
		// client areas.
		//
		if ((DEFAULT_ZONE_WIDTH*2 > client_rect.Height()) ||
			(DEFAULT_ZONE_WIDTH*2 > client_rect.Width()))
		{
			zone_width = 3;
		}

		//
		// Check y-dimension first.
		//
		if (clientPt.y <= zone_width)
		{
			if (clientPt.x <= zone_width)
				return ZONE_TOP_LEFT;
			else if (clientPt.x >= client_rect.Width() - zone_width)
				return ZONE_TOP_RIGHT;
			else
				return ZONE_TOP;
		}
		else if (clientPt.y >= client_rect.Height() - zone_width)
		{
			if (clientPt.x <= zone_width)
				return ZONE_BOTTOM_LEFT;
			else if (clientPt.x >= client_rect.Width() - zone_width)
				return ZONE_BOTTOM_RIGHT;
			else
				return ZONE_BOTTOM;
		}

		//
		// Check x-dimension second.
		//
		if (clientPt.x <= zone_width)
			return ZONE_LEFT;
		else if (clientPt.x >= client_rect.Width() - zone_width)
			return ZONE_RIGHT;
	}

	//
	// Outside of client rect, or not in hot zone.
	//
	return ZONE_NONE;
}


/////////////////////////////////////////////////////////////////////////////
// QCTreeCtrl message handlers


////////////////////////////////////////////////////////////////////////
// IsAutoScrollTimerRunning [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL QCTreeCtrl::IsAutoScrollTimerRunning() const
{
	return (m_idAutoScrollTimer != 0);
}


////////////////////////////////////////////////////////////////////////
// StartAutoScrollTimer [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL QCTreeCtrl::StartAutoScrollTimer()
{
	if (IsAutoScrollTimerRunning())
	{
		ASSERT(0);			// whoops, timer already running
		return TRUE;
	}

	m_idAutoScrollTimer = SetTimer(AUTOSCROLL_TIMER_EVENT_ID, m_uAutoScrollTimerInterval, NULL);

	return IsAutoScrollTimerRunning();
}


////////////////////////////////////////////////////////////////////////
// KillAutoScrollTimer [protected]
//
////////////////////////////////////////////////////////////////////////
void QCTreeCtrl::KillAutoScrollTimer()
{
	if (m_idAutoScrollTimer)
	{
		KillTimer(m_idAutoScrollTimer);
		m_idAutoScrollTimer = 0;
	}
}


////////////////////////////////////////////////////////////////////////
// DoSomethingWhileUserPausedMouseAtPoint [protected, virtual]
//
// Stub implementation.  Subclasses can optional override this method
// to do some processing while the user has paused in the tree
// control while dragging the mouse.  The 'point' argument is in
// ready-to-use client coordinates.
////////////////////////////////////////////////////////////////////////
void QCTreeCtrl::DoSomethingWhileUserPausedMouseAtPoint(CPoint point)
{
	// this function intentionally left blank :-)
}


////////////////////////////////////////////////////////////////////////
// CopyItem [public]
//
// Copies an item to a new location in the tree.
////////////////////////////////////////////////////////////////////////
HTREEITEM QCTreeCtrl::CopyItem(
	HTREEITEM			in_hItemToCopy,
	HTREEITEM			in_hNewParent,
	HTREEITEM			in_hAfter /*= TVI_LAST*/)
{
	TV_INSERTSTRUCT		tvstruct;
	HTREEITEM			hNewItem = NULL;

	//	Init the struct
	memset( &tvstruct, 0, sizeof(TV_INSERTSTRUCT) );

	//	Get most of the information that we want (everything except text)
	tvstruct.item.hItem = in_hItemToCopy;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM |
						 TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvstruct.item.stateMask = TVIS_BOLD | TVIS_SELECTED;

	if ( GetItem(&tvstruct.item) )
	{
		//	Get the text
		CString		szText = GetItemText(in_hItemToCopy);
		
		//	Fill out the text in the struct
		tvstruct.item.cchTextMax = szText.GetLength();
		tvstruct.item.pszText = szText.LockBuffer();
		
		//	Insert the item at proper location
		tvstruct.hParent = in_hNewParent;
		tvstruct.hInsertAfter = in_hAfter;
		tvstruct.item.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_STATE;
		hNewItem = InsertItem(&tvstruct);
	}

	return hNewItem;
}


////////////////////////////////////////////////////////////////////////
// CopyBranch [public]
//
// Copies a branch (item and all children) to a new location
// in the tree.
////////////////////////////////////////////////////////////////////////
HTREEITEM QCTreeCtrl::CopyBranch(
	HTREEITEM			in_hBranchToCopy,
	HTREEITEM			in_hNewParent,
	HTREEITEM			in_hAfter /*= TVI_LAST*/)
{
	// Detect attempts to copy an item to itself
	if (in_hBranchToCopy == in_hNewParent)
	{
		ASSERT(0);
		return NULL;
	}
	
	HTREEITEM	hNewItem = CopyItem(in_hBranchToCopy, in_hNewParent, in_hAfter);
	HTREEITEM	hChild = GetChildItem(in_hBranchToCopy);

	while (hChild != NULL)
	{
		//	Recursively transfer all the children, and their children, etc.
		CopyBranch(hChild, hNewItem);  
		hChild = GetNextSiblingItem(hChild);
	}

	return hNewItem;
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
// Stub implementation only.  Should be overriden by subclasses.
////////////////////////////////////////////////////////////////////////
BOOL QCTreeCtrl::OnDrop(COleDataObject* pDataObject,
						   DROPEFFECT dropEffect, 
						   CPoint point)
{
	ASSERT_VALID(this);
	KillAutoScrollTimer();

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
// Stub implementation only.  Should be overridden by subclasses.
////////////////////////////////////////////////////////////////////////
DROPEFFECT QCTreeCtrl::OnDragEnter(COleDataObject* pDataObject,
									  DWORD dwKeyState, 
									  CPoint point)
{
	StartAutoScrollTimer();

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
// Stub implementation only.  Should be overridden by subclasses.
////////////////////////////////////////////////////////////////////////
DROPEFFECT QCTreeCtrl::OnDragOver(COleDataObject* pDataObject,
									 DWORD dwKeyState, 
									 CPoint point)
{
	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
// Stub implementation only.  Should be overridden by subclasses.
////////////////////////////////////////////////////////////////////////
void QCTreeCtrl::OnDragLeave()
{
	KillAutoScrollTimer();
}


////////////////////////////////////////////////////////////////////////
// OnTimer [protected]
//
// Really interested in auto-scroll timer events.  Other timers are
// processed by the base class implementation.
////////////////////////////////////////////////////////////////////////
void QCTreeCtrl::OnTimer(UINT nIDEvent)
{
	if (nIDEvent != AUTOSCROLL_TIMER_EVENT_ID)
	{
		//
		// Let the base class deal with timers that we didn't set.
		//
		CTreeCtrl::OnTimer( nIDEvent );
		return;
	}

	if (! IsAutoScrollTimerRunning())
		return;				// ignore queued leftover timer messages

	CPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);

	//
	// Require at least two samples at the same cursor location
	// before doing an auto-scroll.
	//
	if ((pt.x != m_PreviousAutoScrollPoint.x) ||
		(pt.y != m_PreviousAutoScrollPoint.y))
	{
		m_PreviousAutoScrollPoint.x = pt.x;
		m_PreviousAutoScrollPoint.y = pt.y;

		m_uLastUniqueDropPointTime = GetTickCount();
		m_hLastDropPointItem = NULL;
	}
	else
	{
		HTREEITEM hSelItem = GetSelectedItem();

		if (m_hLastDropPointItem != hSelItem)
			m_hLastDropPointItem = hSelItem;
		else
		{
			//
			// Invoke virtual method to allow subclass implementations
			// to do some timer-based processing here...
			//
			if (GetTickCount() >= m_uLastUniqueDropPointTime + m_uAutoExpandDelay)
				DoSomethingWhileUserPausedMouseAtPoint(pt);
		}

		//
		// Now carry on with the generic auto-scroll behavior...
		//
		switch (GetAutoScrollZone(pt))
		{
		case ZONE_NONE:
			break;
		case ZONE_TOP:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_BOTTOM:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_LEFT:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_RIGHT:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_TOP_LEFT:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_TOP_RIGHT:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_BOTTOM_LEFT:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		case ZONE_BOTTOM_RIGHT:
			CImageList::DragShowNolock(FALSE);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			CImageList::DragShowNolock(TRUE);
			break;
		default:
			ASSERT(0);
			break;
		}
	}
}

LRESULT QCTreeCtrl::OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam)
{            
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void QCTreeCtrl::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged((CWnd*)this, pFocusWnd);
}

void QCTreeCtrl::OnSysColorChange()
{
	m_Palette.DoSysColorChange((CWnd*)this);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////   QCTreeOleDropTarget    /////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
// Delegate handling to the QCTreeCtrl.
////////////////////////////////////////////////////////////////////////
DROPEFFECT QCTreeOleDropTarget::OnDragEnter(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(QCTreeCtrl)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	// delegates to tree control
	QCTreeCtrl* pTreeCtrl = (QCTreeCtrl *) pWnd;
	ASSERT_VALID(pTreeCtrl);
	return pTreeCtrl->OnDragEnter(pDataObject, dwKeyState, point);
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
// Delegate handling to the QCTreeCtrl.
////////////////////////////////////////////////////////////////////////
DROPEFFECT QCTreeOleDropTarget::OnDragOver(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(QCTreeCtrl)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	// delegates to tree control
	QCTreeCtrl* pTreeCtrl = (QCTreeCtrl *)pWnd;
	ASSERT_VALID(pTreeCtrl);
	return pTreeCtrl->OnDragOver(pDataObject, dwKeyState, point);
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
// Delegate handling to the QCTreeCtrl.
////////////////////////////////////////////////////////////////////////
BOOL QCTreeOleDropTarget::OnDrop(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(QCTreeCtrl)))
	{
		ASSERT(0);
		return FALSE;
	}

	// delegates to tree control
	QCTreeCtrl* pTreeCtrl = (QCTreeCtrl*) pWnd;
	ASSERT_VALID(pTreeCtrl);
	return pTreeCtrl->OnDrop(pDataObject, dropEffect, point);
}


////////////////////////////////////////////////////////////////////////
// OnDropEx [public, virtual]
//
////////////////////////////////////////////////////////////////////////
DROPEFFECT QCTreeOleDropTarget::OnDropEx(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropDefault, 
	DROPEFFECT dropList, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(QCTreeCtrl)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	//
	// Normally, this would delegate to the QCTreeCtrl::OnDropEx()
	// method.  However, since we're currently not doing anything like
	// right button drags, let's just return a "not implemented" status
	// code to force MFC to call the older OnDrop() method.
	//
	return DROPEFFECT(-1);
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
// Delegate handling to the QCTreeCtrl.
////////////////////////////////////////////////////////////////////////
void QCTreeOleDropTarget::OnDragLeave(CWnd* pWnd)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(QCTreeCtrl)))
	{
		ASSERT(0);
		return;
	}

	// delegates to tree control
	QCTreeCtrl* pTreeCtrl = (QCTreeCtrl *) pWnd;
	ASSERT_VALID(pTreeCtrl);
	pTreeCtrl->OnDragLeave();
}
