// StationeryWazooWnd.cpp : implementation file
//
// CStationeryWazooWnd
// Specific implementation of a CWazooWnd.

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

extern QCCommandStack		g_theCommandStack;
extern QCStationeryDirector	g_theStationeryDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CStationeryWazooWnd

IMPLEMENT_DYNCREATE(CStationeryWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CStationeryWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CStationeryWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)

	ON_COMMAND( IDM_NEW_STATIONERY, OnNewStationery )
	ON_COMMAND( IDM_NEW_MESSAGE_WITH_STATIONERY, OnNewMessageWith )
	ON_COMMAND( IDM_EDIT_STATIONERY, OnEdit )
	ON_COMMAND( IDM_DELETE_STATIONERY, OnDelete )
	ON_COMMAND( IDM_RENAME_STATIONERY, OnRename )
	ON_COMMAND( IDM_REPLY_WITH_STATIONERY, OnReplyWith )
	ON_COMMAND( IDM_REPLY_TO_ALL_WITH_STATIONERY, OnReplyToAllWith )

	ON_UPDATE_COMMAND_UI( IDM_NEW_MESSAGE_WITH_STATIONERY, EnableIfSelected )
	ON_UPDATE_COMMAND_UI( IDM_EDIT_STATIONERY, EnableIfSelected )
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


/////////////////////////////////////////////////////////////////////////////
// CStationeryWazooWnd message handlers

int CStationeryWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pStationeryView = new CContainerView;
	if (m_pStationeryView->Create(this) == FALSE)
	{
		TRACE("Failed to create Stationery view");
		return -1;
	}

	if (m_StationeryTree.Create(WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS,
			CRect(0, 0, 0, 0), m_pStationeryView, IDC_TREE1) == FALSE)
	{
		TRACE("Failed to create Stationery tree");
		return -1;
	}

	if (!m_StationeryTree.Init())
	{
		TRACE("Failed to initialize Stationery tree");
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Hook standard MFC message to initialize Wazoo window focus.
////////////////////////////////////////////////////////////////////////
LRESULT CStationeryWazooWnd::OnInitialUpdate(WPARAM, LPARAM)
{
	CWazooWnd::SaveInitialChildFocus(&m_StationeryTree);
	return TRUE;
}


void CStationeryWazooWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect rectClient;
	GetClientRect(rectClient);
	
	m_pStationeryView->SetWindowPos(NULL, rectClient.left, rectClient.top,
									rectClient.Width(), rectClient.Height(),
									SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

	m_StationeryTree.SetWindowPos(NULL, rectClient.left, rectClient.top,
									rectClient.Width(), rectClient.Height(),
									SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

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
		{
			m_StationeryTree.SelectItem( hHitTest );	
		}
	}

	// get the Stationery bar specific context menu
	hMenu = QCLoadMenu( MAKEINTRESOURCE( IDR_STATIONERY ) );

	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
	DoContextMenu(this, ptScreen, hMenu);
}


void CStationeryWazooWnd::EnableIfSelected(
CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_StationeryTree.GetSelectedItem() != NULL );
}



void CStationeryWazooWnd::OnNewStationery()
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


QCStationeryCommand* CStationeryWazooWnd::GetSelected()
{
	HTREEITEM				hItem;
	CString					szName;

	hItem = m_StationeryTree.GetSelectedItem();
	
	if( hItem == NULL )
	{
		return NULL;
	}

	szName = m_StationeryTree.GetItemText( hItem );
	
	return g_theStationeryDirector.Find( szName );
}


void CStationeryWazooWnd::OnNewMessageWith()
{
	QCStationeryCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
	{
		pCommand->Execute( CA_NEW_MESSAGE_WITH );
	}
}


void CStationeryWazooWnd::OnEdit()
{
	QCStationeryCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
	{
		pCommand->Execute( CA_OPEN );
	}
}

void CStationeryWazooWnd::OnDelete()
{
	QCStationeryCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
	{
		pCommand->Execute( CA_DELETE );
	}
}

void CStationeryWazooWnd::OnRename()
{
	HTREEITEM	hItem;
	CEdit*		pEdit;

	hItem = m_StationeryTree.GetSelectedItem();
	
	if( hItem == NULL )
	{
		return;
	}

	m_StationeryTree.SendMessage( TVM_EDITLABEL, 0, LPARAM( hItem ) );
	
	pEdit = m_StationeryTree.GetEditControl();
	
	if( pEdit != NULL )
	{
		pEdit->SetSel( 0, -1 );
	}
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
	CView*					pView;
	QCStationeryCommand*	pCommand;
 	WORD					wID;

	pCommand = GetSelected();
	
	if( pCommand == NULL )
	{
		return;
	}

 	wID = g_theCommandStack.FindCommandID( pCommand, theAction );

	if( wID == 0 )
	{
		return;
	}
	
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
	{
		pFrame = (CFrameWnd *) GetParentFrame()->GetNextWindow();
	}
	else
	{
		pFrame = pMainFrame->GetActiveFrame();
	}

	if( pFrame == NULL )
	{
		return;
	}

	pView = pFrame->GetActiveView();

	if( pView == NULL )
	{
		return;
	}

	pView->SendMessage( WM_COMMAND, wID, 0 );
}


void CStationeryWazooWnd::OnUpdateReplyWith(
CCmdUI* pCmdUI)
{	
	DoUpdateReply( CA_REPLY_WITH, pCmdUI );
}


void CStationeryWazooWnd::OnUpdateReplyToAllWith(
CCmdUI* pCmdUI)
{	
	DoUpdateReply( CA_REPLY_TO_ALL_WITH, pCmdUI );
}


void CStationeryWazooWnd::DoUpdateReply(
COMMAND_ACTION_TYPE	theAction,
CCmdUI*				pCmdUI)
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
	{
		return;
	}

 	wID = g_theCommandStack.FindCommandID( pCommand, theAction );

	if( wID == 0 )
	{
		return;
	}
	
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
	{
		pFrame = (CFrameWnd *) GetParentFrame()->GetNextWindow();
	}
	else
	{
		pFrame = pMainFrame->GetActiveFrame();
	}

	if( pFrame == NULL )
	{
		return;
	}

	uSaveID = pCmdUI->m_nID;
	
	// substitute stationery specific id
	pCmdUI->m_nID = wID;
	
	( ( CCmdTarget* ) pFrame )->OnCmdMsg( wID, CN_UPDATE_COMMAND_UI, pCmdUI, NULL );

	// restore the id
	pCmdUI->m_nID = uSaveID;
}
