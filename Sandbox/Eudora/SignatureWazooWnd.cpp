// SignatureWazooWnd.cpp : implementation file
//
// CSignatureWazooWnd
// Specific implementation of a CWazooWnd.

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

extern QCCommandStack		g_theCommandStack;
extern QCSignatureDirector	g_theSignatureDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSignatureWazooWnd

IMPLEMENT_DYNCREATE(CSignatureWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CSignatureWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CSignatureWazooWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)

	ON_COMMAND( IDM_EDIT_SIGNATURE, OnEdit )

#ifdef COMMERCIAL	
	ON_COMMAND( ID_NEW_SIGNATURE, OnNewSignature )
	ON_COMMAND( IDM_DELETE_SIGNATURE, OnDelete )
	ON_COMMAND( IDM_RENAME_SIGNATURE, OnRename )

	ON_UPDATE_COMMAND_UI( IDM_DELETE_SIGNATURE, EnableIfSelected )
	ON_UPDATE_COMMAND_UI( IDM_RENAME_SIGNATURE, EnableIfSelected )
#endif // COMMERCIAL
	ON_UPDATE_COMMAND_UI( IDM_EDIT_SIGNATURE, EnableIfSelected )
	
END_MESSAGE_MAP()


CSignatureWazooWnd::CSignatureWazooWnd() : CWazooWnd(IDR_SIGNATURES),
	m_pSignatureView(NULL)
{
}

CSignatureWazooWnd::~CSignatureWazooWnd()
{
}


/////////////////////////////////////////////////////////////////////////////
// CSignatureWazooWnd message handlers

int CSignatureWazooWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWazooWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pSignatureView = new CContainerView;
	if (m_pSignatureView->Create(this) == FALSE)
	{
		TRACE("Failed to create Signature view");
		return -1;
	}

	if (m_SignatureTree.Create(WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS,
			CRect(0, 0, 0, 0), m_pSignatureView, IDC_TREE1) == FALSE)
	{
		TRACE("Failed to create Signature tree");
		return -1;
	}

	if (!m_SignatureTree.Init())
	{
		TRACE("Failed to initialize Signature tree");
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Hook standard MFC message to initialize Wazoo window focus.
////////////////////////////////////////////////////////////////////////
LRESULT CSignatureWazooWnd::OnInitialUpdate(WPARAM, LPARAM)
{
	CWazooWnd::SaveInitialChildFocus(&m_SignatureTree);
	return TRUE;
}


void CSignatureWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectClient;
	GetClientRect(rectClient);

	m_pSignatureView->SetWindowPos(NULL, rectClient.left, rectClient.top,
									rectClient.Width(), rectClient.Height(),
									SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

	m_SignatureTree.SetWindowPos(NULL, rectClient.left, rectClient.top,
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
		{
			m_SignatureTree.SelectItem( hHitTest );	
		}
	}

	// get the Signature bar specific context menu
	hMenu = QCLoadMenu( MAKEINTRESOURCE( IDR_SIGNATURES ) );

	CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
	DoContextMenu(this, ptScreen, hMenu);
}


void CSignatureWazooWnd::EnableIfSelected(
CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_SignatureTree.GetSelectedItem() != NULL );
}



void CSignatureWazooWnd::OnNewSignature()
{
	// unfortunately, the New.. on the context menu and the New.. on the 
	// main menu can't be the same id -- it causes an infinite loop
	//AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_TOOL_SIGNATURE_NEW );
    CNewSignatureDialog	theDialog;
	QCSignatureCommand*	pCommand;

	while( theDialog.DoModal() == IDOK )
	{
		pCommand = g_theSignatureDirector.AddCommand( theDialog.GetName() );
		
		if( pCommand )		
		{
			pCommand->Execute( CA_OPEN );
			break;
		}
	}

}


QCSignatureCommand* CSignatureWazooWnd::GetSelected()
{
	HTREEITEM				hItem;
	CString					szName;

	hItem = m_SignatureTree.GetSelectedItem();
	
	if( hItem == NULL )
	{
		return NULL;
	}

	szName = m_SignatureTree.GetItemText( hItem );
	
	return g_theSignatureDirector.Find( szName );
}



void CSignatureWazooWnd::OnEdit()
{
	QCSignatureCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
	{
		pCommand->Execute( CA_OPEN );
	}
}

void CSignatureWazooWnd::OnDelete()
{
	QCSignatureCommand*	pCommand;

	pCommand = GetSelected();

	if( pCommand != NULL )
	{
		pCommand->Execute( CA_DELETE );
	}
}

void CSignatureWazooWnd::OnRename()
{
	HTREEITEM	hItem;
	CEdit*		pEdit;

	hItem = m_SignatureTree.GetSelectedItem();
	
	if( hItem == NULL )
	{
		return;
	}

	m_SignatureTree.SendMessage( TVM_EDITLABEL, 0, LPARAM( hItem ) );
	
	pEdit = m_SignatureTree.GetEditControl();
	
	if( pEdit != NULL )
	{
		pEdit->SetSel( 0, -1 );
	}
}



