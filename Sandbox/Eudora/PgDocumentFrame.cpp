// PgDocumentFrame.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include "QCProtocol.h"
#include "PgDocumentFrame.h"

#include "font.h"
#include "QCChildToolBar.h"
#include "TBarMenuButton.h"
#include "TBarCombo.h"
#include "TBarMenuButton.h"

#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

static UINT theFullFeatureFormatButtons[] = {
	ID_FONT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_SIZE,
	ID_SEPARATOR,
	ID_EDIT_TEXT_BOLD,
	ID_EDIT_TEXT_ITALIC,
	ID_EDIT_TEXT_UNDERLINE,
	ID_EDIT_TEXT_COLOR,
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
	ID_EDIT_INSERT
};

static UINT theReducedFeatureFormatButtons[] = {
	ID_FONT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_SIZE,
	ID_SEPARATOR,
	ID_EDIT_TEXT_BOLD,
	ID_EDIT_TEXT_ITALIC,
	ID_EDIT_TEXT_UNDERLINE,
	ID_EDIT_TEXT_COLOR,
	ID_EDIT_TEXT_TT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_LEFT,
	ID_EDIT_TEXT_CENTER,
	ID_EDIT_TEXT_RIGHT,
	ID_SEPARATOR,
	ID_EDIT_TEXT_CLEAR
};


BEGIN_BUTTON_MAP( thePgDocFrameButtonMap )

	STD_BUTTON( ID_EDIT_TEXT_BOLD, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_ITALIC, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_UNDERLINE, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_TT, TBBS_INDETERMINATE )

	STD_BUTTON( ID_BLKFMT_BULLETTED_LIST, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_LEFT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_CENTER, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_RIGHT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_INSERT_LINK, TBBS_BUTTON )

	TBARCOMBO_BUTTON( ID_FONT, IDC_FONT_COMBO, 0, WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, 115, 40, 115 )
	
	TBARMENU_BUTTON( ID_EDIT_INSERT, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_TEXT_SIZE, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_TEXT_COLOR, TBBS_BUTTON )

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
	//}}AFX_MSG_MAP
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
	
	
	m_pToolBarManager = new SECToolBarManager( this );
	m_pFormattingToolBar = new QCChildToolBar;  

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
	VERIFY( pMenu = pEditTextMenu->GetSubMenu( 9 ) );
	i = m_pFormattingToolBar->CommandToIndex( ID_EDIT_TEXT_SIZE );
	VERIFY( pMenuButton = ( CTBarMenuButton* ) ( m_pFormattingToolBar->GetButton( i ) ) );	
	pMenuButton->SetHMenu( pMenu->GetSafeHmenu() );

	// get the color menu
	VERIFY( pMenu = pEditTextMenu->GetSubMenu( 10 ) );
	i = m_pFormattingToolBar->CommandToIndex( ID_EDIT_TEXT_COLOR );
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
