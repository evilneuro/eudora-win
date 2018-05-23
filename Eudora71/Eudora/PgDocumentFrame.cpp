// PgDocumentFrame.cpp : implementation file
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "mainfrm.h"
#include "rs.h"
#include "utils.h"
#include "helpcntx.h"
#include "QCProtocol.h"
#include "PgDocumentFrame.h"

#include "font.h"
#include "QCChildToolBar.h"
#include "TBarMenuButton.h"
#include "TBarCombo.h"
#include "TBarMenuButton.h"
#include "ColorToolbarButton.h"

#include "QCFindMgr.h"
#include "QCSharewareManager.h"
#include "PaigeEdtView.h"

#include "EmoticonToolbarButton.h"

#include "DebugNewHelpers.h"

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

static UINT theFullFeatureFormatButtons[] = {
	ID_FONT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_SIZE,
	ID_SEPARATOR,
	ID_EDIT_TEXT_BOLD,
	ID_EDIT_TEXT_ITALIC,
	ID_EDIT_TEXT_UNDERLINE,
	ID_EDIT_TEXT_STRIKEOUT,
	ID_EDIT_TEXT_LAST_TEXT_COLOR,
	ID_EDIT_TEXT_TT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_LEFT,
	ID_EDIT_TEXT_CENTER,
	ID_EDIT_TEXT_RIGHT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_INDENT_IN,
	ID_EDIT_TEXT_INDENT_OUT,
	ID_SEPARATOR,
	ID_BLKFMT_BULLETTED_LIST,
	ID_EDIT_INSERT_LINK,
	ID_SEPARATOR,
	ID_EDIT_TEXT_CLEAR,
	ID_SEPARATOR,
	ID_EDIT_INSERT,
	ID_SEPARATOR,
	ID_EDIT_LAST_EMOTICON

};

static UINT theReducedFeatureFormatButtons[] = {
	ID_FONT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_SIZE,
	ID_SEPARATOR,
	ID_EDIT_TEXT_BOLD,
	ID_EDIT_TEXT_ITALIC,
	ID_EDIT_TEXT_UNDERLINE,
	ID_SEPARATOR,
	ID_EDIT_TEXT_CLEAR
};


BEGIN_BUTTON_MAP( thePgDocFrameButtonMap )

	STD_BUTTON( ID_EDIT_TEXT_BOLD, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_ITALIC, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_UNDERLINE, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_STRIKEOUT, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_TT, TBBS_INDETERMINATE )

	STD_BUTTON( ID_BLKFMT_BULLETTED_LIST, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_LEFT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_CENTER, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_RIGHT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_INSERT_LINK, TBBS_BUTTON )

	TBARCOMBO_BUTTON( ID_FONT, IDC_FONT_COMBO, 0, WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, 115, 40, 115 )
	
	TBARMENU_BUTTON( ID_EDIT_INSERT, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_TEXT_SIZE, TBBS_BUTTON )

	COLOR_BUTTON(ID_EDIT_TEXT_LAST_TEXT_COLOR, ID_EDIT_TEXT_COLOR, 0, NULL)
	EMOTICON_BUTTON(ID_EDIT_LAST_EMOTICON, ID_EDIT_EMOTICON, 0, NULL)

END_BUTTON_MAP()

/////////////////////////////////////////////////////////////////////////////
// PgDocumentFrame

IMPLEMENT_DYNCREATE(PgDocumentFrame, CMDIChild)

PgDocumentFrame::PgDocumentFrame()
{
	m_pToolBarManager = NULL;
	m_pFormattingToolBar = NULL;
}

PgDocumentFrame::~PgDocumentFrame()
{
	delete m_pToolBarManager;
}


BEGIN_MESSAGE_MAP(PgDocumentFrame, CMDIChild)
	//{{AFX_MSG_MAP(PgDocumentFrame)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_EDIT_CHECKSPELLING, OnCheckSpelling)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PgDocumentFrame message handlers

int PgDocumentFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	DWORD				dwStyle;
	DWORD				dwExStyle;
	INT					i;
	CComboBox*			pCombo;
	CStringArray		theArray;
	CMDIFrameWnd*		pMainFrame;
	BOOL				bMaximized;
	CMenu*				pEditTextMenu;
	CMenu*				pMenu;
	CTBarMenuButton*	pMenuButton;

	if (CMDIChild::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	
	m_pToolBarManager = DEBUG_NEW_MFCOBJ_NOTHROW SECToolBarManager( this );
	m_pFormattingToolBar = DEBUG_NEW_MFCOBJ_NOTHROW QCChildToolBar;  

	if( ( m_pToolBarManager == NULL ) ||
		( m_pFormattingToolBar == NULL ) )
	{
		ASSERT( 0 );
		return FALSE;
	}

	m_pFormattingToolBar->SetManager( m_pToolBarManager );
	m_pFormattingToolBar->m_bAutoDelete = TRUE;
	
	m_pToolBarManager->LoadToolBarResource( MAKEINTRESOURCE( IDR_COMPMESS ), MAKEINTRESOURCE( IDR_COMPMESS ) );
	m_pToolBarManager->SetButtonMap( thePgDocFrameButtonMap );

	if ( GetIniShort( IDS_INI_SHOW_COOLBAR ) )
	{
		m_pToolBarManager->EnableCoolLook( TRUE );
	}

	EnableDocking(CBRS_ALIGN_TOP);
	
	dwStyle = WS_VISIBLE | WS_CHILD | CBRS_TOP | CBRS_SIZE_DYNAMIC | CBRS_FLYBY ;

	if( GetIniShort( IDS_INI_SHOW_TOOLTIPS ) )
	{
		dwStyle |= CBRS_TOOLTIPS;
	}

	dwExStyle = 0L;
	//dwExStyle = CBRS_EX_SIZE_TO_FIT;

	if( m_pToolBarManager->CoolLookEnabled() )
	{
		dwExStyle |= CBRS_EX_COOLBORDERS;
	}
	
	// now create the formatting tool bar
	
	if( ! GetIniShort( IDS_INI_SHOW_STYLED_TEXT_TOOLBAR ) )
	{
		dwStyle = dwStyle & ~WS_VISIBLE;
	}

	if( ! m_pFormattingToolBar->CreateEx( dwExStyle, this, dwStyle, AFX_IDW_TOOLBAR+7, _T( "Format" ) ) )
	{
		ASSERT( 0 );
		return FALSE;
	}

	m_pToolBarManager->SetToolBarInfo( m_pFormattingToolBar );
	m_pFormattingToolBar->EnableDocking(CBRS_ALIGN_TOP);

	// Shareware: In reduced feature mode, you get a less-capable format toolbar
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		m_pFormattingToolBar->SetButtons( theFullFeatureFormatButtons, DIM( theFullFeatureFormatButtons ) );
	}
	else
	{
		// REDUCED FEATURE mode
		m_pFormattingToolBar->SetButtons( theReducedFeatureFormatButtons, DIM( theReducedFeatureFormatButtons ) );
	}

	DockControlBar( m_pFormattingToolBar );

	// get the face names
	EnumFontFaces( theArray );

	pCombo = ( CComboBox* ) ( m_pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ) );

	for( i = 0; i < theArray.GetSize(); i ++ )
	{
		pCombo->AddString( theArray[i] );
	}

	// get the main frame window
	pMainFrame = ( CMDIFrameWnd* ) AfxGetApp()->m_pMainWnd;
	
	// see if it's maximized
	bMaximized = FALSE;
	pMainFrame->MDIGetActive( &bMaximized );
	i = ( bMaximized ? 1 : 0 );

	// get the main window
	VERIFY( pEditTextMenu = pMainFrame->GetMenu() );

	// get the edit menu
	VERIFY( pEditTextMenu = pEditTextMenu->GetSubMenu( 1 + i ) );

	// Shareware: In reduced feature mode, you cannot right-click
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode

		// get the insert menu
		VERIFY( pMenu = pEditTextMenu->GetSubMenu( 11 ) );	
		i = m_pFormattingToolBar->CommandToIndex( ID_EDIT_INSERT );
		VERIFY( pMenuButton = ( CTBarMenuButton* ) ( m_pFormattingToolBar->GetButton( i ) ) );	
		pMenuButton->SetHMenu( pMenu->GetSafeHmenu() );
	}

	// get the text menu
	VERIFY( pEditTextMenu = pEditTextMenu->GetSubMenu( 10 ) );

	// get the size menu
	VERIFY( pMenu = pEditTextMenu->GetSubMenu( 10 ) );
	i = m_pFormattingToolBar->CommandToIndex( ID_EDIT_TEXT_SIZE );
	VERIFY( pMenuButton = ( CTBarMenuButton* ) ( m_pFormattingToolBar->GetButton( i ) ) );	
	pMenuButton->SetHMenu( pMenu->GetSafeHmenu() );

	RecalcLayout();

	return 0;
}

QCChildToolBar* PgDocumentFrame::GetFormatToolbar() 
{
	if( m_pFormattingToolBar )
	{
		return m_pFormattingToolBar; 
	}

	return NULL;
}


// --------------------------------------------------------------------------

//
// FIND TEXT
//

void PgDocumentFrame::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void PgDocumentFrame::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG PgDocumentFrame::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCProtocol *pProtocol;
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (!pFindMgr)
		return (EuFIND_ERROR);

	// This is our internal message to ask if we support find.
	// Return non-zero (TRUE).
	if (pFindMgr->IsAck(wParam, lParam))
		return (EuFIND_ACK_YES);

	LPFINDREPLACE lpFR = (LPFINDREPLACE) lParam;
	ASSERT(lpFR);

	if (!lpFR)
		return (EuFIND_ERROR);

	if (lpFR->Flags & FR_DIALOGTERM)
	{
		ASSERT(0); // Should never fwd a terminating msg
		return (EuFIND_ERROR);
	}

	CWnd *pwndFocus = GetActiveView();

	if(!pwndFocus)
		return (EuFIND_ERROR);

	pProtocol = QCProtocol::QueryProtocol( QCP_FIND, pwndFocus );
	
	if( ( pProtocol != NULL ) &&
		( pProtocol->DoFindNext( lpFR->lpstrFindWhat, (lpFR->Flags & FR_MATCHCASE), (lpFR->Flags & FR_WHOLEWORD), TRUE) ) )
	{
		// activate the window
		pwndFocus->GetParentFrame()->ActivateFrame();
		return (EuFIND_OK);
	}
		

	return (EuFIND_NOTFOUND);
}

void PgDocumentFrame::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = ::QCLoadMenu(IDR_CONTEXT_POPUPS);
	if (!hMenu || !menu.Attach(hMenu))
	{
		ASSERT(0);		// resources hosed?
		return;
	}

	// MP_POPUP_RECEIVED_MSG is the offset for the read message submenu.
	CMenu* pMenuPopup = menu.GetSubMenu(MP_POPUP_COMP_MSG);
	if (!pMenuPopup)
		ASSERT(0);		// resources hosed?
	else
	{
		//
		// Since the popup menu we get from GetSubMenu() is a pointer
		// to a temporary object, let's make a local copy of the
		// object so that we have explicit control over its lifetime.
		//
		// Note that we edit the context menu on-the-fly in order to
		// stick in the latest/greatest Transfer menu, display the
		// edited context menu, then remove the Transfer menu.
		//
		CMenu tempPopupMenu;
		tempPopupMenu.Attach(pMenuPopup->GetSafeHmenu());
		tempPopupMenu.DeleteMenu(ID_EDIT_PASTE_SPECIAL, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(ID_EDIT_PASTEASQUOTATION, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(ID_MESSAGE_ATTACHFILE, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(ID_MESSAGE_SENDAGAIN, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(ID_MESSAGE_CHANGEQUEUEING, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(ID_SEARCH_MAILBOX_FOR_SEL, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(ID_SEARCH_MAILFOLDER_FOR_SEL, MF_BYCOMMAND);
		tempPopupMenu.DeleteMenu(6, MF_BYPOSITION); // a separator...

		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), point);
		CContextMenu(&tempPopupMenu, point.x, point.y);
		
		VERIFY(tempPopupMenu.Detach());
	}

	menu.DestroyMenu();	
}

void PgDocumentFrame::OnCheckSpelling()
{
	// Shareware: In reduced feature mode, you cannot spell check
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		CView* View = GetActiveView();

		if(!View)
			return;			
		
		QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_SPELL, View );
		
		if (pProtocol && ((CPaigeEdtView*)pProtocol)->HasSelection())
		{
			if(pProtocol->CheckSpelling(FALSE)==NO_MISSPELLINGS)
					::MessageBox( NULL, (LPCTSTR)CRString(IDS_SPELL_NO_MISSPELLINGS), 
					(LPCTSTR)CRString(IDS_EUDORA), MB_OK);
		}
		else 
		{
				int nBodyResult = NO_MISSPELLINGS;
				if (pProtocol)
					nBodyResult = pProtocol->CheckSpelling(FALSE);
				if (nBodyResult == NO_MISSPELLINGS)
					AfxMessageBox(IDS_SPELL_NO_MISSPELLINGS);
		}
	}
}