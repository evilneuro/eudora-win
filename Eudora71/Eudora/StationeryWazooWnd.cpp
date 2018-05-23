// StationeryWazooWnd.cpp : implementation file
//
// CStationeryWazooWnd
// Specific implementation of a CWazooWnd.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include "eudora.h"

#include "rs.h"
#include "utils.h"
#include "helpcntx.h"
#include "StationeryWazooWnd.h"
#include "ContainerView.h"
#include "mdichild.h"
#include "mainfrm.h"
#include "compmsgd.h"
#include "summary.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"

#include "QCSharewareManager.h"

extern QCCommandStack		g_theCommandStack;
extern QCStationeryDirector	g_theStationeryDirector;

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CStationeryWazooWnd

IMPLEMENT_DYNCREATE(CStationeryWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CStationeryWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CStationeryWazooWnd)
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP

	ON_COMMAND( IDM_NEW_STATIONERY, OnNewStationery )
	ON_COMMAND( IDM_NEW_MESSAGE_WITH_STATIONERY, OnNewMessageWith )
	ON_COMMAND( IDM_EDIT_STATIONERY, OnEdit )
	ON_COMMAND( IDM_DELETE_STATIONERY, OnDelete )
	ON_COMMAND( IDM_RENAME_STATIONERY, OnRename )
	ON_COMMAND( IDM_REPLY_WITH_STATIONERY, OnReplyWith )
	ON_COMMAND( IDM_REPLY_TO_ALL_WITH_STATIONERY, OnReplyToAllWith )

	ON_UPDATE_COMMAND_UI( IDM_NEW_STATIONERY, OnUpdateNew )
	ON_UPDATE_COMMAND_UI( IDM_NEW_MESSAGE_WITH_STATIONERY, EnableIfSelected )
	ON_UPDATE_COMMAND_UI( IDM_EDIT_STATIONERY, OnUpdateEdit )
	ON_UPDATE_COMMAND_UI( IDM_DELETE_STATIONERY, EnableIfSelected )
	ON_UPDATE_COMMAND_UI( IDM_RENAME_STATIONERY, EnableIfSelected )

	ON_UPDATE_COMMAND_UI( IDM_REPLY_WITH_STATIONERY, OnUpdateReplyWith )
	ON_UPDATE_COMMAND_UI( IDM_REPLY_TO_ALL_WITH_STATIONERY, OnUpdateReplyToAllWith )
END_MESSAGE_MAP()


CStationeryWazooWnd::CStationeryWazooWnd() : CWazooWnd(IDR_STATIONERY),
	m_pStationeryView(NULL)
{
}

CStationeryWazooWnd::~CStationeryWazooWnd()
{
}


BOOL CStationeryWazooWnd::CreateView() 
{
	if (!m_pStationeryView)
	{
		m_pStationeryView = DEBUG_NEW_MFCOBJ_NOTHROW CContainerView;
		if (!m_pStationeryView || m_pStationeryView->Create(this) == FALSE)
			return FALSE;

		if (m_StationeryTree.Create(WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS,
				CRect(0, 0, 0, 0), m_pStationeryView, IDC_TREE1) == FALSE)
		{
			return FALSE;
		}

		if (!m_StationeryTree.Init())
			return FALSE;

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());

		SaveInitialChildFocus(&m_StationeryTree);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to allow JIT creation of view
////////////////////////////////////////////////////////////////////////
void CStationeryWazooWnd::OnActivateWazoo()
{
	CreateView();
}


/////////////////////////////////////////////////////////////////////////////
// CStationeryWazooWnd message handlers

void CStationeryWazooWnd::OnSize(UINT nType, int cx, int cy)
{
	if (m_pStationeryView)
	{
		CRect rectClient;
		GetClientRect(rectClient);
		
		m_pStationeryView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

		m_StationeryTree.SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}


////////////////////////////////////////////////////////////////////////
// OnChar [protected]
//
// Scan for VK_RETURN and VK_ESCAPE key codes and forward them to
// the tree control.
////////////////////////////////////////////////////////////////////////
void CStationeryWazooWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_RETURN:
	case VK_ESCAPE:
		m_StationeryTree.SendMessage(WM_CHAR, nChar, MAKELPARAM(nRepCnt, nFlags));
		break;
	default:
		CWazooWnd::OnChar(nChar, nRepCnt, nFlags);
		break;
	}
}

void CStationeryWazooWnd::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	HMENU		hMenu;	
	HTREEITEM	hGetSelectedItem;
	HTREEITEM	hHitTest;
	UINT		uFlags;
	CPoint		thePoint( ptScreen );

	hGetSelectedItem = m_StationeryTree.GetSelectedItem();
	m_StationeryTree.ScreenToClient( &thePoint );
	hHitTest = m_StationeryTree.HitTest( thePoint, &uFlags );

	if( ( hHitTest != hGetSelectedItem ) && ( hHitTest != NULL ) )
	{
		if( uFlags & TVHT_ONITEM )
			m_StationeryTree.SelectItem( hHitTest );	
	}

	// get the Stationery bar specific context menu
	hMenu = QCLoadMenu( MAKEINTRESOURCE( IDR_STATIONERY ) );

	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
	DoContextMenu(this, ptScreen, hMenu);
}


void CStationeryWazooWnd::OnUpdateNew(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);
}

void CStationeryWazooWnd::OnUpdateEdit(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (UsingFullFeatureSet())
		EnableIfSelected(pCmdUI);
}

void CStationeryWazooWnd::EnableIfSelected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_StationeryTree.GetSelectedItem() != NULL );
}


void CStationeryWazooWnd::OnNewStationery()
{
	if (!UsingFullFeatureSet())
		FeatureNotInFree();
	else
	{
		CCompMessageDoc *pCompMsgDoc = NewCompDocument();

		if (pCompMsgDoc)
		{
			pCompMsgDoc->SetIsStationery();
			//The comp doc knows where to get the title from and what format to set it in,
			pCompMsgDoc->SetTitle("");
			pCompMsgDoc->m_Sum->Display();
		}
	}
}


QCStationeryCommand* CStationeryWazooWnd::GetSelected()
{
	HTREEITEM				hItem;
	CString					szName;

	hItem = m_StationeryTree.GetSelectedItem();
	
	if( hItem == NULL )
		return NULL;

	szName = m_StationeryTree.GetItemText( hItem );
	
	return g_theStationeryDirector.Find( szName );
}


void CStationeryWazooWnd::OnNewMessageWith()
{
	QCStationeryCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
		pCommand->Execute( CA_NEW_MESSAGE_WITH );
}


void CStationeryWazooWnd::OnEdit()
{
	if (!UsingFullFeatureSet())
		FeatureNotInFree();
	else
	{
		QCStationeryCommand*	pCommand;

		pCommand = GetSelected();

		if( pCommand != NULL )
			pCommand->Execute( CA_OPEN );
	}
}

void CStationeryWazooWnd::OnDelete()
{
	QCStationeryCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
		pCommand->Execute( CA_DELETE );
}

void CStationeryWazooWnd::OnRename()
{
	HTREEITEM	hItem;
	CEdit*		pEdit;

	hItem = m_StationeryTree.GetSelectedItem();
	
	if( hItem == NULL )
		return;

	m_StationeryTree.SendMessage( TVM_EDITLABEL, 0, LPARAM( hItem ) );
	
	pEdit = m_StationeryTree.GetEditControl();
	
	if( pEdit != NULL )
		pEdit->SetSel( 0, -1 );
}


void CStationeryWazooWnd::OnReplyToAllWith()
{
	DoReply( CA_REPLY_TO_ALL_WITH );
}


void CStationeryWazooWnd::OnReplyWith()
{
	DoReply( CA_REPLY_WITH );
}


void CStationeryWazooWnd::DoReply(
COMMAND_ACTION_TYPE theAction )
{
	SECControlBar*			pParentBar;
	CMainFrame*				pMainFrame;
	CFrameWnd*				pFrame;
	QCStationeryCommand*	pCommand;
 	WORD					wID;

	pCommand = GetSelected();
	
	if( pCommand == NULL )
		return;

 	wID = g_theCommandStack.FindOrAddCommandID( pCommand, theAction );

	if( wID == 0 )
		return;
	
	g_theCommandStack.SaveCommand( wID );

	//
	// Hunt for the "next" frame window which contains a message view
	// that can be replied to.
	//
	// If we're checking this from an MDI mode Wazoo container, we
	// need to check the "next" MDI child in the Z-order with a known 
	// non-Wazoo type.  Otherwise, check the topmost MDI window.
	//
	pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMainFrame, pMainFrame);

	pParentBar = (SECControlBar *) GetParentControlBar();
	ASSERT_KINDOF(SECControlBar, pParentBar);

	if( pParentBar->IsMDIChild() )
		pFrame = (CFrameWnd *) GetParentFrame()->GetNextWindow();
	else
		pFrame = pMainFrame->GetActiveFrame();

	if( pFrame == NULL )
		return;

	pFrame->SendMessage( WM_COMMAND, wID, 0 );
}


void CStationeryWazooWnd::OnUpdateReplyWith(CCmdUI* pCmdUI)
{	
	DoUpdateReply( CA_REPLY_WITH, pCmdUI );
}


void CStationeryWazooWnd::OnUpdateReplyToAllWith(CCmdUI* pCmdUI)
{	
	DoUpdateReply( CA_REPLY_TO_ALL_WITH, pCmdUI );
}


void CStationeryWazooWnd::DoUpdateReply(COMMAND_ACTION_TYPE theAction, CCmdUI* pCmdUI)
{	
	SECControlBar*			pParentBar;
	CMainFrame*				pMainFrame;
	CFrameWnd*				pFrame;
	QCStationeryCommand*	pCommand;
 	WORD					wID;
	UINT					uSaveID;

	pCmdUI->Enable( FALSE );

	pCommand = GetSelected();
	
	if( pCommand == NULL )
		return;

 	wID = g_theCommandStack.FindOrAddCommandID( pCommand, theAction );

	if( wID == 0 )
		return;
	
	//
	// Hunt for the "next" frame window which contains a message view
	// that can be replied to.
	//
	// If we're checking this from an MDI mode Wazoo container, we
	// need to check the "next" MDI child in the Z-order with a known 
	// non-Wazoo type.  Otherwise, check the topmost MDI window.
	//
	pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMainFrame, pMainFrame);

	pParentBar = (SECControlBar *) GetParentControlBar();
	ASSERT_KINDOF(SECControlBar, pParentBar);

	if( pParentBar->IsMDIChild() )
		pFrame = (CFrameWnd *) GetParentFrame()->GetNextWindow();
	else
		pFrame = pMainFrame->GetActiveFrame();

	if( pFrame == NULL )
		return;

	uSaveID = pCmdUI->m_nID;
	
	// substitute stationery specific id
	pCmdUI->m_nID = wID;
	
	( ( CCmdTarget* ) pFrame )->OnCmdMsg( wID, CN_UPDATE_COMMAND_UI, pCmdUI, NULL );

	// restore the id
	pCmdUI->m_nID = uSaveID;
}
