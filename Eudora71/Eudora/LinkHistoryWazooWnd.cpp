//	LinkHistoryWazooWnd.cpp
//
//	Specific implementation of a CWazooWnd for Link History information.
//
//	Copyright (c) 1999 by QUALCOMM, Incorporated
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


#include "stdafx.h"

#include "rs.h"
#include "resource.h"
#include "3dformv.h"
#include "font.h"
#include "helpcntx.h"
#include "LinkHistoryList.h"
#include "LinkHistoryManager.h"
#include "LinkHistoryWazooWnd.h"
#include "QCSharewareManager.h"
#include "utils.h"


#include "DebugNewHelpers.h"


IMPLEMENT_DYNCREATE(CLinkHistoryWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CLinkHistoryWazooWnd, CWazooWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()
	ON_NOTIFY(LVN_ITEMCHANGED, CLinkHistoryWazooWnd::id_LinkHistoryList, OnItemChange)
	ON_BN_CLICKED(CLinkHistoryWazooWnd::id_ViewButton, OnViewLink)
	ON_BN_CLICKED(CLinkHistoryWazooWnd::id_RemoveButton, OnRemoveLink)
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
//		* CLinkHistoryWazooWnd										[Public]
// ---------------------------------------------------------------------------
//	CLinkHistoryWazooWnd constructor

CLinkHistoryWazooWnd::CLinkHistoryWazooWnd() : CWazooWnd(IDR_LINK_HISTORY),
	m_pLinkHistoryList(NULL), m_ViewButton(), m_RemoveButton(), m_iButtonHeight(30)
{
}


// ---------------------------------------------------------------------------
//		* ~CLinkHistoryWazooWnd										[Public]
// ---------------------------------------------------------------------------
//	CLinkHistoryWazooWnd destructor

CLinkHistoryWazooWnd::~CLinkHistoryWazooWnd()
{
}


// ---------------------------------------------------------------------------
//		* OnActivateWazoo											[Public]
// ---------------------------------------------------------------------------
//	Do just-in-time creation of the child controls and the icon image
//	list associated with the LinkHistoryManager and LinkHistoryList.

void CLinkHistoryWazooWnd::OnActivateWazoo()
{
	if (NULL == m_pLinkHistoryList)
	{
		//	Make sure that the LinkHistoryManager creates the icon image list before
		//	the LinkHistoryList needs it. The LinkHistoryList needs it in it's
		//	OnCreate method, which will be called when we create it below.
		LinkHistoryManager::Instance()->CreateIconImageList();
		
		m_pLinkHistoryList = DEBUG_NEW CLinkHistoryList;

		m_pLinkHistoryList->Create(NULL, NULL, WS_VISIBLE | WS_BORDER | WS_TABSTOP, CRect(0,0,0,0), this, id_LinkHistoryList);

		CFont *		pControlFont = m_pLinkHistoryList->GetFont();

		TEXTMETRIC tm;
		::GetTextMetrics(pControlFont, tm);
		m_iButtonHeight = tm.tmHeight + tm.tmExternalLeading + 10;

		m_ViewButton.Create( CRString(IDS_VIEW_BUTTON_NAME), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP | BS_PUSHBUTTON,
							 CRect(0,0,0,0), this, CLinkHistoryWazooWnd::id_ViewButton );
		m_ViewButton.SetFont(pControlFont);
		
		m_RemoveButton.Create( CRString(IDS_REMOVE_BUTTON_NAME), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP | BS_PUSHBUTTON,
							   CRect(0,0,0,0), this, CLinkHistoryWazooWnd::id_RemoveButton );
		m_RemoveButton.SetFont(pControlFont);

		CWazooWnd::SaveInitialChildFocus(m_pLinkHistoryList);
		
		//	Register with the shareware manager so that we know when the mode changes
		QCSharewareManager *	pSWM = GetSharewareManager();
		if (pSWM != NULL)
			pSWM->Register(this);
	}
}


// ---------------------------------------------------------------------------
//		* OnCreate												[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to create the window contents
//	(list and buttons).

int CLinkHistoryWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


// ---------------------------------------------------------------------------
//		* OnDestroy												[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to unregister with the shareware manager.

void CLinkHistoryWazooWnd::OnDestroy() 
{
	if (m_pLinkHistoryList != NULL)
	{
		//	We previously registered with the shareware manager - unregister now.
		QCSharewareManager *	pSWM = GetSharewareManager();
		if (pSWM != NULL)
			pSWM->UnRegister(this);
	}

	CWazooWnd::OnDestroy();
}


// ---------------------------------------------------------------------------
//		* OnEraseBkgnd											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to correctly erase background. In particular
//	this ensures that the background area below the list view (i.e. the
//	space around the buttons) is erased when switching to this wazoo.

BOOL CLinkHistoryWazooWnd::OnEraseBkgnd(CDC *pDC)
{
	CBrush		backBrush;
	backBrush.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	CBrush *	pOldBrush = pDC->SelectObject(&backBrush);
	CRect		rcClip;

	pDC->GetClipBox(&rcClip);
	pDC->PatBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);

	return CWazooWnd::OnEraseBkgnd(pDC);
}


// ---------------------------------------------------------------------------
//		* OnSize												[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to correctly size window contents.

void CLinkHistoryWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	// Don't do anything if the controls aren't created yet, or
	// the window is being minimized
	if (m_pLinkHistoryList == NULL || nType == SIZE_MINIMIZED)
		return;
	
	const int	kHorizontalSpace = 10;
	const int	kVerticalSpace = 6;
	const int	kViewButtonWidth = 70;
	const int	kRemoveButtonWidth = 90;

	CRect	rectClient;
	GetClientRect(rectClient);

	//	Size list to be partly overlapping/under it's parents border. Do this
	//	for left, top, and right but not bottom (hence just + 1 for height
	//	to compensate for top-1, as opposed to + 2 for width, which compensates
	//	for left-1 and overlaps on the right).
	int		listHeight = rectClient.Height() - (kVerticalSpace * 2 + m_iButtonHeight) + 1;
	int		listWidth = rectClient.Width() + 2;
	m_pLinkHistoryList->SetWindowPos( NULL, rectClient.left-1, rectClient.top-1,
									  listWidth, listHeight,
									  SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW );

	int		buttonTop = rectClient.top + listHeight + kVerticalSpace;
	int		buttonLeft = kHorizontalSpace;
	m_ViewButton.SetWindowPos( NULL, buttonLeft, buttonTop,
							   kViewButtonWidth, m_iButtonHeight,
							   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW );
	
	buttonLeft += kViewButtonWidth + kHorizontalSpace;
	m_RemoveButton.SetWindowPos( NULL, buttonLeft, buttonTop,
								 kRemoveButtonWidth, m_iButtonHeight,
								 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW );

	CWazooWnd::OnSize(nType, cx, cy);
}


// ---------------------------------------------------------------------------
//		* OnContextMenu											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to support link history specific context
//	menu.

void CLinkHistoryWazooWnd::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	CListCtrl &		theCtrl = m_pLinkHistoryList->GetListCtrl();

	if ( pWnd->GetSafeHwnd() == theCtrl.GetSafeHwnd() )
	{
		//
		// Hit the list control, so get the Personality-specific
		// context menu.  First, we must get the menu that contains
		// the Personality context popups.
		//
		CMenu	menuBar;
		HMENU	hMenuBar = ::QCLoadMenu(IDR_LINK_HISTORY);
		if (!hMenuBar || !menuBar.Attach(hMenuBar))
		{
			ASSERT(0);
			return;
		}

		CMenu* pTempPopup = menuBar.GetSubMenu(0);
		if (pTempPopup != NULL)
		{
			CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
			DoContextMenu(this, ptScreen, pTempPopup->GetSafeHmenu());
		}
		else
		{
			ASSERT(0);		// resources hosed?
		}

		menuBar.DestroyMenu();
		return;
	}

	CWazooWnd::OnContextMenu(pWnd, ptScreen);
}


// ---------------------------------------------------------------------------
//		* OnCmdMsg													[Public]
// ---------------------------------------------------------------------------
//	Override for virtual CCmdTarget::OnCmdMsg() method. The idea is to
//	change the standard command routing to forward commands from the Wazoo
//	control bar all the way to the embedded view. Necessary to correctly
//	enable the context menu.

BOOL
CLinkHistoryWazooWnd::OnCmdMsg(
	UINT					nID,
	int						nCode,
	void *					pExtra,
	AFX_CMDHANDLERINFO *	pHandlerInfo)
{
	if ( m_pLinkHistoryList && m_pLinkHistoryList->GetSafeHwnd() )
		if ( m_pLinkHistoryList->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
			return TRUE;

	return CWazooWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


// ---------------------------------------------------------------------------
//		* PreTranslateMessage										[Public]
// ---------------------------------------------------------------------------
//	Override to correctly handle dialogish keyboard control (i.e. tabbing
//	among controls, return key with focussed buttons, accelerators).
//	This is needed because we're not using a form view.
//	Implementation adapted from CFormView::PreTranslateMessage.

BOOL CLinkHistoryWazooWnd::PreTranslateMessage(MSG* pMsg) 
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	//	Give parent class the first shot
	if ( CWazooWnd::PreTranslateMessage(pMsg) )
		return TRUE;

	//	Don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	//	Since 'IsDialogMessage' will eat frame window accelerators, we call
	//	all frame windows' PreTranslateMessage first
	pFrameWnd = GetParentFrame();   // start with first parent frame
	while (pFrameWnd != NULL)
	{
		//	Allow owner & frames to translate before IsDialogMessage does
		if ( pFrameWnd->PreTranslateMessage(pMsg) )
			return TRUE;

		//	Try parent frames until there are no parent frames
		pFrameWnd = pFrameWnd->GetParentFrame();
	}

	//	Translate dialogish events, but don't translate non-input events
	if ( ((pMsg->message < WM_KEYFIRST) || (pMsg->message > WM_KEYLAST)) &&
		 ((pMsg->message < WM_MOUSEFIRST) || (pMsg->message > WM_MOUSELAST)) )
	{
		return FALSE;
	}

	return IsDialogMessage(pMsg);
}


// ---------------------------------------------------------------------------
//		* OnSetCursor											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to set the restore the default status text

BOOL CLinkHistoryWazooWnd::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	CFrameWnd *		pFrameWnd = GetTopLevelFrame();
	
	if ( (pFrameWnd != NULL) && (!m_pLinkHistoryList->GetStatusURLShown()) )
	{
		// Restore default status bar prompt.
		pFrameWnd->PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	m_pLinkHistoryList->SetStatusURLShown(false);

	return CWazooWnd::OnSetCursor(pWnd, nHitTest, message);
}


// ---------------------------------------------------------------------------
//		* UpdateButtonsEnabledStates								[Public]
// ---------------------------------------------------------------------------
//	Updates button enabling based on current selection and feature mode.

void CLinkHistoryWazooWnd::UpdateButtonsEnabledStates()
{
    UINT	nSel = m_pLinkHistoryList->GetListCtrl().GetSelectedCount();

	if ( UsingFullFeatureSet() )
	{
		m_ViewButton.EnableWindow(nSel == 1);
		m_RemoveButton.EnableWindow(nSel >= 1);
	}
	else
	{
		m_ViewButton.EnableWindow(FALSE);
		m_RemoveButton.EnableWindow(FALSE);
	}
}


// ---------------------------------------------------------------------------
//		* Notify													[Public]
// ---------------------------------------------------------------------------
//	Notify us when program changes state between Light, Pro, and Adware.

void CLinkHistoryWazooWnd::Notify(
	QCCommandObject*		/*pObject*/,
	COMMAND_ACTION_TYPE		theAction,
	void *					pData)
{
	if (theAction == CA_SWM_CHANGE_FEATURE)
	{
		UpdateButtonsEnabledStates();
	}
}


// ---------------------------------------------------------------------------
//		* OnItemChange											[Protected]
// ---------------------------------------------------------------------------
//	Override (message mapped) to change button enabling.

void CLinkHistoryWazooWnd::OnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateButtonsEnabledStates();
    *pResult = 0;
}


// ---------------------------------------------------------------------------
//		* OnViewLink											[Protected]
// ---------------------------------------------------------------------------
//	Tell the Link History view to launch the selected item.

void CLinkHistoryWazooWnd::OnViewLink()
{
	m_pLinkHistoryList->OnLaunchSelectedItem();
}


// ---------------------------------------------------------------------------
//		* OnRemoveLink											[Protected]
// ---------------------------------------------------------------------------
//	Tell the Link History view to remove the selected item.

void CLinkHistoryWazooWnd::OnRemoveLink()
{
	m_pLinkHistoryList->OnRemoveSelectedItems();
}
