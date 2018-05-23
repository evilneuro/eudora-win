// SignatureWazooWnd.cpp : implementation file
//
// CSignatureWazooWnd
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
#include "SignatureWazooWnd.h"
#include "ContainerView.h"
#include "mdichild.h"

#include "NewSignatureDialog.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"

#include "QCSharewareManager.h"

extern QCCommandStack		g_theCommandStack;
extern QCSignatureDirector	g_theSignatureDirector;

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CSignatureWazooWnd

IMPLEMENT_DYNCREATE(CSignatureWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CSignatureWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CSignatureWazooWnd)
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP

	ON_COMMAND( IDM_EDIT_SIGNATURE, OnEdit )
	ON_COMMAND( ID_NEW_SIGNATURE, OnNewSignature )
	ON_COMMAND( IDM_DELETE_SIGNATURE, OnDelete )
	ON_COMMAND( IDM_RENAME_SIGNATURE, OnRename )

	ON_UPDATE_COMMAND_UI( IDM_DELETE_SIGNATURE, EnableIfSelectedFullFeature )
	ON_UPDATE_COMMAND_UI( ID_NEW_SIGNATURE, EnableIfFullFeature )
	ON_UPDATE_COMMAND_UI( IDM_RENAME_SIGNATURE, EnableIfSelectedFullFeature )
	ON_UPDATE_COMMAND_UI( IDM_EDIT_SIGNATURE, EnableIfSelected )
	
END_MESSAGE_MAP()


CSignatureWazooWnd::CSignatureWazooWnd() : CWazooWnd(IDR_SIGNATURES),
	m_pSignatureView(NULL)
{
}

CSignatureWazooWnd::~CSignatureWazooWnd()
{
}


BOOL CSignatureWazooWnd::CreateView() 
{
	if (!m_pSignatureView)
	{
		m_pSignatureView = DEBUG_NEW_MFCOBJ_NOTHROW CContainerView;
		if (!m_pSignatureView || m_pSignatureView->Create(this) == FALSE)
			return FALSE;

		if (m_SignatureTree.Create(WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS,
				CRect(0, 0, 0, 0), m_pSignatureView, IDC_TREE1) == FALSE)
		{
			return FALSE;
		}

		if (!m_SignatureTree.Init())
			return FALSE;

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());

		SaveInitialChildFocus(&m_SignatureTree);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to allow JIT creation of view
////////////////////////////////////////////////////////////////////////
void CSignatureWazooWnd::OnActivateWazoo()
{
	CreateView();
}


/////////////////////////////////////////////////////////////////////////////
// CSignatureWazooWnd message handlers

void CSignatureWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (m_pSignatureView)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		m_pSignatureView->SetWindowPos(NULL, rectClient.left, rectClient.top,
										rectClient.Width(), rectClient.Height(),
										SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

		m_SignatureTree.SetWindowPos(NULL, rectClient.left, rectClient.top,
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
void CSignatureWazooWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_RETURN:
	case VK_ESCAPE:
		m_SignatureTree.SendMessage(WM_CHAR, nChar, MAKELPARAM(nRepCnt, nFlags));
		break;
	default:
		CWazooWnd::OnChar(nChar, nRepCnt, nFlags);
		break;
	}
}


void CSignatureWazooWnd::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	HMENU		hMenu;	
	HTREEITEM	hGetSelectedItem;
	HTREEITEM	hHitTest;
	UINT		uFlags;
	CPoint		thePoint( ptScreen );

	hGetSelectedItem = m_SignatureTree.GetSelectedItem();
	m_SignatureTree.ScreenToClient( &thePoint );
	hHitTest = m_SignatureTree.HitTest( thePoint, &uFlags );

	if( ( hHitTest != hGetSelectedItem ) && ( hHitTest != NULL ) )
	{
		if( uFlags & TVHT_ONITEM )
			m_SignatureTree.SelectItem( hHitTest );	
	}

	// get the Signature bar specific context menu
	hMenu = QCLoadMenu( MAKEINTRESOURCE( IDR_SIGNATURES ) );

	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
	DoContextMenu(this, ptScreen, hMenu);
}


void CSignatureWazooWnd::EnableIfSelected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_SignatureTree.GetSelectedItem() != NULL );
}

void CSignatureWazooWnd::EnableIfSelectedFullFeature(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);

	if (UsingFullFeatureSet())
		pCmdUI->Enable( m_SignatureTree.GetSelectedItem() != NULL );
}


void CSignatureWazooWnd::EnableIfFullFeature(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);
}

void CSignatureWazooWnd::OnNewSignature()
{
	BOOL bCanceled = FALSE;

	if (!UsingFullFeatureSet())
		return;

    CNewSignatureDialog	theDialog;
	QCSignatureCommand*	pCommand;

	while( theDialog.DoModal() == IDOK )
	{
		while(g_theSignatureDirector.Find(theDialog.GetName()))
		{
			if(theDialog.DoModal() != IDOK)
			{
				bCanceled = TRUE;
				break;
			}
		}

		if(!bCanceled)
		{
			pCommand = g_theSignatureDirector.AddCommand( theDialog.GetName() );

		
			if( pCommand )		
			{
				pCommand->Execute( CA_OPEN );
				break;
			}
		}
	}

}


QCSignatureCommand* CSignatureWazooWnd::GetSelected()
{
	HTREEITEM				hItem;
	CString					szName;

	hItem = m_SignatureTree.GetSelectedItem();
	
	if( hItem == NULL )
		return NULL;

	szName = m_SignatureTree.GetItemText( hItem );
	
	return g_theSignatureDirector.Find( szName );
}



void CSignatureWazooWnd::OnEdit()
{
	QCSignatureCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
		pCommand->Execute( CA_OPEN );
}

void CSignatureWazooWnd::OnDelete()
{
	if (!UsingFullFeatureSet())
		return;

	QCSignatureCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
		pCommand->Execute( CA_DELETE );
}

void CSignatureWazooWnd::OnRename()
{
	if (!UsingFullFeatureSet())
		return;

	HTREEITEM	hItem;
	CEdit*		pEdit;

	hItem = m_SignatureTree.GetSelectedItem();
	
	if( hItem == NULL )
		return;

	m_SignatureTree.SendMessage( TVM_EDITLABEL, 0, LPARAM( hItem ) );
	
	pEdit = m_SignatureTree.GetEditControl();
	
	if( pEdit != NULL )
		pEdit->SetSel( 0, -1 );
}
