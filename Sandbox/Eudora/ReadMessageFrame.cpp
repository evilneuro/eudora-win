// ReadMessageFrame.cpp: implementation of the CReadMessageFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PgReadMsgView.h"
#undef SubclassWindow			// Nasty!

#include "resource.h"
#include "msgutils.h"
#include "utils.h"

#include "BlahBlahBlahView.h"
#include "ClipboardMgr.h"
#include "QCChildToolBar.h"
#include "font.h"
#include "helpcntx.h"
#include "mainfrm.h"
#include "ReadMessageFrame.h"
#include "ReadMessageDoc.h"
#include "ReadMessageToolBar.h"
#include "rs.h"
#include "Summary.h"
#include "pop.h"
#include "MsgRecord.h"

#include "TBarMenuButton.h"
#include "TBarBmpCombo.h"
#include "TBarCombo.h"
#include "TBarEdit.h"
#include "TBarStatic.h"
#include "BidentReadMessageView.h"
#include "TridentReadMessageView.h"
#include "TocDoc.h"
#include "TocView.h"
#include "QCProtocol.h"

#include "QCCommandActions.h"
#include "QCCommandObject.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"

#include "QCFindMgr.h"
#include "QCSharewareManager.h"


#define DIM( a ) ( sizeof( a )/sizeof( a[0] ) )

extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;
extern UINT					umsgSetPriority;		// see CompMessageFrame

UINT	umsgButtonSetCheck = RegisterWindowMessage( "umsgButtonSetCheck" );

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


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

// --------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CReadMessageFrame

IMPLEMENT_DYNCREATE(CReadMessageFrame, CMessageFrame)

BEGIN_MESSAGE_MAP(CReadMessageFrame, CMessageFrame)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

	ON_BN_CLICKED( ID_DELETE_FROM_SERVER, OnDeleteFromServer )
	ON_BN_CLICKED( ID_DOWNLOAD_FROM_SERVER, OnDownloadFromServer )
	ON_BN_CLICKED( ID_EDIT_MESSAGE, OnEditMode )

	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_SIZE, OnUpdateAlwaysEnabled )
	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_COLOR, OnUpdateAlwaysEnabled )
	ON_UPDATE_COMMAND_UI( ID_EDIT_INSERT, OnUpdateAlwaysEnabled )
	
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )

	ON_CBN_SELCHANGE( IDC_PRIORITY_COMBO, OnPriorityChange )

	ON_EN_CHANGE(IDC_SUBJECT, OnChangeSubject)
	ON_WM_SIZE()
	
	ON_REGISTERED_MESSAGE(umsgSetPriority, OnSetPriority)
	ON_REGISTERED_MESSAGE(umsgButtonSetCheck, OnButtonSetCheck)

	ON_COMMAND(ID_MESSAGE_DELETE, OnDelete)
	ON_WM_CONTEXTMENU()

#ifdef IMAP4
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_UNDELETE, OnUpdateUndelete)
	ON_COMMAND(ID_MESSAGE_UNDELETE, UnDelete)
#endif

END_MESSAGE_MAP()

BEGIN_BUTTON_MAP( theReadMessageButtonMap )
	STD_BUTTON( ID_EDIT_TEXT_BOLD, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_ITALIC, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_UNDERLINE, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_TT, TBBS_INDETERMINATE )

	STD_BUTTON( ID_BLKFMT_BULLETTED_LIST, TBBS_CHECKBOX )

	STD_BUTTON( ID_EDIT_TEXT_LEFT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_CENTER, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_RIGHT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_INSERT_LINK, TBBS_BUTTON )

	STD_BUTTON( ID_USE_FIXED_FONT, TBBS_CHECKBOX )
	STD_BUTTON( ID_BLAHBLAHBLAH, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_MESSAGE, TBBS_CHECKBOX )
	STD_BUTTON( ID_DELETE_FROM_SERVER, TBBS_CHECKBOX )
	STD_BUTTON( ID_DOWNLOAD_FROM_SERVER, TBBS_CHECKBOX )

	STATIC_BUTTON( ID_TOW_TRUCK, IDC_TOW_TRUCK, 0, SS_NOTIFY | SS_ICON | SS_CENTERIMAGE, 25, 25, 25 )
	STATIC_BUTTON( ID_SUBJECT_STATIC, ID_SUBJECT_STATIC, 0, 0, 70, 30, 20 )
	EDIT_BUTTON( ID_SUBJECT, IDC_SUBJECT, 0, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 200, 30, 20 )

	BMPCOMBO_BUTTON( ID_PRIORITY, IDC_PRIORITY_COMBO, 0, CBS_DROPDOWN, 150, 40, 150 )

	TBARCOMBO_BUTTON( ID_FONT, IDC_FONT_COMBO, 0, WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, 115, 40, 115 )
	
	TBARMENU_BUTTON( ID_EDIT_INSERT, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_TEXT_SIZE, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_TEXT_COLOR, TBBS_BUTTON )

END_BUTTON_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

//////////////////////////////////////////////////////////////////////

CReadMessageFrame::CReadMessageFrame()
{
	m_iMajorVersion = 0;
	m_iMinorVersion = 0;
	m_pToolBarManager = NULL;
	m_pToolBar = NULL;
}

CReadMessageFrame::~CReadMessageFrame()
{
	delete m_pToolBarManager;
}


BOOL CReadMessageFrame::OnCreateClient( 
LPCREATESTRUCT lpcs,
CCreateContext* pContext)
{
	BOOL				fRet;
	CRect				MainWindowRect;
	CRect				FrameInitSize;
	CSummary*			pSummary;
	DWORD				dwStyle;
	DWORD				dwExStyle;
	char*				pFullMessage;
	CBitmapCombo*		pBitmapCombo;
	CComboBox*			pCombo;
	CStatic*			pStatic;
	CEdit*				pEdit;
	INT					i;
	CStringArray		theArray;
	CTBarMenuButton*	pMenuButton;
	CMDIFrameWnd*		pMainFrame;
	BOOL				bMaximized;
	CMenu*				pMenu;
	CMenu*				pEditTextMenu;
	CTBarStaticBtn*		pStaticBtn;
	INT					nButtons;
	INT					j;
	UINT				theReadMessageButtons[] = {
							ID_TOW_TRUCK,
							ID_SEPARATOR,
							ID_USE_FIXED_FONT,
							ID_SEPARATOR,
							ID_BLAHBLAHBLAH,
							ID_EDIT_MESSAGE,
							ID_DELETE_FROM_SERVER,
							ID_DOWNLOAD_FROM_SERVER,
							ID_SEPARATOR,
							ID_PRIORITY,
							ID_SEPARATOR,
							ID_SUBJECT_STATIC,
							ID_SUBJECT };

	CReadMessageDoc* Doc = (CReadMessageDoc*)pContext->m_pCurrentDoc;
	ASSERT_KINDOF(CReadMessageDoc, Doc);

	pSummary = Doc->m_Sum;

	// changed the default behavior to use mshtml if it's found and
	// the ini entry is not turned off.

	if ( GetMSHTMLVersion( &m_iMajorVersion, &m_iMinorVersion) &&
		 GetIniShort( IDS_INI_USE_BIDENT_ALWAYS ) )
	{
		if( ( m_iMajorVersion > 4 ) || ( m_iMinorVersion >= 71 ) )
		{
			pContext->m_pNewViewClass = RUNTIME_CLASS( CTridentReadMessageView );
		}
		else if( ( pSummary->IsHTML() ||
				 ( pSummary->IsXRich() && (pFullMessage = Doc->GetText()) && ::IsFancy( pFullMessage ) == IS_HTML ) ) )
		{
			pContext->m_pNewViewClass = RUNTIME_CLASS( CBidentReadMessageView );
		}
		else
		{
			pContext->m_pNewViewClass = RUNTIME_CLASS( PgReadMsgView );
		}
	}
	else
	{
		pContext->m_pNewViewClass = RUNTIME_CLASS( PgReadMsgView );
	}

	fRet = m_wndSplitter.Create( this, 2, 1, CSize( 1, 1) , pContext );
	m_wndSplitter.SetScrollStyle( 0 );
	m_wndSplitter.RecalcLayout();

	// set up the toolbar manager

	m_pToolBarManager = new SECToolBarManager( this );
	m_pToolBar = new CReadMessageToolBar;  
	m_pFormattingToolBar = new QCChildToolBar;  

	if( ( m_pToolBarManager == NULL ) ||
		( m_pToolBar == NULL ) ||
		( m_pFormattingToolBar == NULL ) )
	{
		ASSERT( 0 );
		return FALSE;
	}

	m_pToolBar->SetManager( m_pToolBarManager );
	m_pToolBar->m_bAutoDelete = TRUE;
	
	m_pFormattingToolBar->SetManager( m_pToolBarManager );
	m_pFormattingToolBar->m_bAutoDelete = TRUE;
	
	m_pToolBarManager->LoadToolBarResource( MAKEINTRESOURCE( IDR_READMESS ), MAKEINTRESOURCE( IDR_READMESS ) );
	m_pToolBarManager->SetButtonMap( theReadMessageButtonMap );

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
	
	// create the normal toolbar
	if( ! m_pToolBar->CreateEx( dwExStyle, this, dwStyle, AFX_IDW_TOOLBAR+6, _T( "Read Message" ) ) )
	{
		ASSERT( 0 );
		return FALSE;
	}

	//pMsgRecord = GetMsgByHash( pSummary );
	CLMOSRecord LMOSRecord(pSummary);
	CMsgRecord* pMsgRecord = LMOSRecord.GetMsgRecord();
	
	nButtons = DIM( theReadMessageButtons );
	
	if( ( pMsgRecord == NULL ) || ( pMsgRecord->GetDeleteFlag() == LMOS_DELETE_SENT /*-1*/ ) )
	{
		// delete both and separator
		
		for( i = 0; i < nButtons; i++ )
		{
			if( ( theReadMessageButtons[i] == ID_DELETE_FROM_SERVER ) ||
				( theReadMessageButtons[i] == ID_DOWNLOAD_FROM_SERVER ) )
			{
				for( j = i + 1; j < nButtons; j++ )
				{
					theReadMessageButtons[j-1] = theReadMessageButtons[j];
				}
				-- i;
				-- nButtons;
			}
		}
	}
	else if( pMsgRecord->GetSkippedFlag() != LMOS_SKIP_MESSAGE /*0*/ )
	{
		for( i = 0; i < nButtons; i++ )
		{
			if( theReadMessageButtons[i] == ID_DOWNLOAD_FROM_SERVER ) 
			{
				for( j = i + 1; j < nButtons; j++ )
				{
					theReadMessageButtons[j-1] = theReadMessageButtons[j];
				}
				-- nButtons;
				break;
			}
		}
	}


	m_pToolBarManager->SetToolBarInfo( m_pToolBar );
	m_pToolBar->EnableDocking(CBRS_ALIGN_TOP);
	m_pToolBar->SetButtons( theReadMessageButtons, nButtons );
	DockControlBar( m_pToolBar );

	

	// now create the formatting tool bar
	
	dwStyle = dwStyle & ~WS_VISIBLE;
	
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

	// initialize the tow truck
	
	pStatic = ( CStatic* ) ( m_pToolBar->GetDlgItem( IDC_TOW_TRUCK ) );
	{
		HICON hIcon = ::QCLoadIcon( IDI_TOWTRUCK );
		pStatic->SetIcon( hIcon );
	}


	// initialize the fixed width font button
	if( GetIniShort( IDS_INI_USE_PROPORTIONAL_AS_DEFAULT ) == 0 )
	{
		OnButtonSetCheck( ID_USE_FIXED_FONT, TRUE );
	}

	// initialize the label next to the subject edit
	
	pStaticBtn = ( CTBarStaticBtn* ) ( m_pToolBar->GetDlgItem( ID_SUBJECT_STATIC ) );
	pStaticBtn->SetWindowText( "Subject: " );
	pStaticBtn->AdjustSize();

	// initialize the subject edit control

	pEdit = ( CEdit* )  ( m_pToolBar->GetDlgItem( IDC_SUBJECT ) );
	pEdit->SetWindowText( pSummary->m_Subject );


	// initialize the priority combo
	
	pBitmapCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_PRIORITY_COMBO ) );
	
	for (i = IDB_PRIOR_HIGHEST; i <= IDB_PRIOR_LOWEST; i++)
		pBitmapCombo->Add(new CBitmapComboItem(i, i - IDB_PRIOR_HIGHEST + IDS_PRIORITY_HIGHEST));

	pBitmapCombo->SetCurSel( pSummary->m_Priority - 1 );
	
	
	EnumFontFaces( theArray );

	pCombo = ( CComboBox* ) ( m_pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ) );

	for( i = 0; i < theArray.GetSize(); i ++ )
	{
		pCombo->AddString( theArray[i] );
	}

	// initialize the delete from server button
	
	if( pSummary->IsDeleteFromServerSet() )
	{
		OnButtonSetCheck( ID_DELETE_FROM_SERVER, TRUE );
	}


	// initialize the download from server button
	
	if( pSummary->IsDownloadFromServerSet() )
	{
		OnButtonSetCheck( ID_DOWNLOAD_FROM_SERVER, TRUE );
	}

	// Initialize the Blah Blah Blah button with the ShowAllHeaders INI entry
	if (GetIniShort(IDS_INI_SHOW_ALL_HEADERS))
	{
		OnButtonSetCheck( ID_BLAHBLAHBLAH, TRUE );
		if (RUNTIME_CLASS( PgReadMsgView ) == pContext->m_pNewViewClass)
			PostMessage(WM_COMMAND, ID_BLAHBLAHBLAH);
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

	// Shareware: In reduced feature mode, you get a less-capable format toolbar
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

	// force the toolbar to recalculate the button sizes
	m_pToolBarManager->SetToolBarInfo( m_pToolBar );
	RecalcLayout();


	// Size parent window
	((CMainFrame*)AfxGetMainWnd())->GetRealClientRect(&MainWindowRect);

	FrameInitSize = pSummary->GetSavedPos();
	
	// If the message doesn't have a saved window size and isn't a Trident view, then size it
	if (FrameInitSize.IsRectEmpty() == FALSE)
		MoveWindow(&FrameInitSize, FALSE);
	else
	{
		const CFontInfo& TheMessageFont = GetMessageFont();
		int CW = TheMessageFont.CharWidth();
		int CH = TheMessageFont.CellHeight();
		int CascadeX = (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXSIZE)) * MessageCascadeSpot;
		int CascadeY = (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYSIZE)) * MessageCascadeSpot;
		
		// Start with toolbar size, which also initializes upper-left corner to 0,0
		m_pToolBar->GetClientRect(&FrameInitSize);
		
		// Add height of caption and top and bottom window borders
		FrameInitSize.bottom += GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) * 2;
		
		// Use Message Width setting plus left and right window borders plus scrollbar width
		// plus left and right margins
		FrameInitSize.right = FrameInitSize.left + GetIniShort(IDS_INI_MESSAGE_WIDTH) * CW +
			GetSystemMetrics(SM_CXFRAME) * 2 + GetSystemMetrics(SM_CXVSCROLL) + 20;
		if (GetIniShort(IDS_INI_ZOOM_WINDOWS))
		{
			// If the start right side is beyond the column-sized right side, then use it
			int ZoomedRight = FrameInitSize.left + lpcs->cx;
			if (ZoomedRight > FrameInitSize.right)
				FrameInitSize.right = ZoomedRight;
		}
			
		// If cascading windows, see if this one would fit width-wise
		if (MessageCascadeSpot > 0)
		{
			if (FrameInitSize.right + CascadeX > MainWindowRect.right)
				MessageCascadeSpot = 0;
			else
			{
				FrameInitSize.left   += CascadeX;
				FrameInitSize.right  += CascadeX;
				FrameInitSize.top    += CascadeY;
				FrameInitSize.bottom += CascadeY;
			}
		}
		
		// Now figure out what the height of the text area should be
		if (GetIniShort(IDS_INI_ZOOM_WINDOWS))
		{
			if (pContext->m_pNewViewClass != RUNTIME_CLASS( PgReadMsgView ))
				FrameInitSize.bottom = FrameInitSize.top + lpcs->cy;
		}
		else
		{
			// Use Message Lines setting
			FrameInitSize.bottom += GetIniShort(IDS_INI_MESSAGE_LINES) * CH;

			// If this window's height is too big, then move it up to the top
			if (MessageCascadeSpot > 0 && FrameInitSize.bottom > MainWindowRect.bottom)
			{
				MessageCascadeSpot = 0;
				FrameInitSize.left   -= CascadeX;
				FrameInitSize.right  -= CascadeX;
				FrameInitSize.top    -= CascadeY;
				FrameInitSize.bottom -= CascadeY;
			}
		}
	
		if (MessageCascadeSpot >= 0)
			MessageCascadeSpot++;

		MoveWindow(&FrameInitSize, FALSE);
	}


	return fRet;
}

void CReadMessageFrame::ZoomWindow()
{
	if (GetIniShort(IDS_INI_ZOOM_WINDOWS))
	{
		CWnd* View = m_wndSplitter.GetPane(0, 0);
		QCProtocol* ViewProtocol = QCProtocol::QueryProtocol(QCP_METRICS, View);
		if (ViewProtocol)
		{
			// Figure out what the height of the window should be to just fit the text
			CRect rect;
			m_pToolBar->GetClientRect(&rect);
			
			// Add height of caption and top and bottom window borders
			rect.bottom += GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) * 2;

			int PixelHeight = ViewProtocol->GetTotalHeight();
			rect.bottom += PixelHeight;

			// mmmm, mmmm, mmmm!  Love that fudge!
			rect.bottom += 2;

			SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		}
	}
}

void CReadMessageFrame::OnDeleteFromServer()
{
	CReadMessageDoc*	pDoc;

	pDoc = (CReadMessageDoc*)GetActiveDocument();

	if( pDoc && pDoc->m_Sum )
		pDoc->m_Sum->ToggleDeleteFromServer();
}


void CReadMessageFrame::OnDownloadFromServer()
{
	CReadMessageDoc*	pDoc;

	pDoc = (CReadMessageDoc*)GetActiveDocument();
	
	if( pDoc && pDoc->m_Sum )
		pDoc->m_Sum->ToggleDownloadFromServer();
}


void CReadMessageFrame::OnBlahBlahBlah()
{
	if( GetCheck( ID_BLAHBLAHBLAH ) == FALSE )
	{
		m_wndSplitter.SplitRow( 5 );
		m_wndSplitter.DeleteRow( 0 );	
	
		if( GetCheck( ID_EDIT_MESSAGE ) &&
			m_pFormattingToolBar->GetSafeHwnd() )
		{
			m_pFormattingToolBar->ShowWindow( SW_SHOW );
		}				
	}
	else
	{
		m_wndSplitter.SetViewType( RUNTIME_CLASS( CBlahBlahBlahView ) );
		m_wndSplitter.SplitRow( 5 );
		m_wndSplitter.DeleteRow( 0 );			
		m_wndSplitter.SetViewType( NULL );
				
		if( m_pFormattingToolBar->GetSafeHwnd() )
		{
			m_pFormattingToolBar->ShowWindow( SW_HIDE );
		}
	}

	RecalcLayout();
}


void CReadMessageFrame::OnEditMode()
{
	CView*		pView;

	pView = ( CView* ) m_wndSplitter.GetPane( 0, 0 );

	if( pView->IsKindOf( RUNTIME_CLASS( CBlahBlahBlahView ) ) == 0 )
	{
		// toggle the formatting toolbar if we aren't in blah blah blah mode

		if( GetCheck( ID_EDIT_MESSAGE ) )
		{
			if( m_pFormattingToolBar->GetSafeHwnd() )
			{
				m_pFormattingToolBar->ShowWindow( SW_SHOW );
				RecalcLayout();
			}
		}
		else
		{
			if( m_pFormattingToolBar->GetSafeHwnd() )
			{
				m_pFormattingToolBar->ShowWindow( SW_HIDE );
				RecalcLayout();
			}
		}
	}	
}



BOOL CReadMessageFrame::OnCmdMsg(
UINT				nID, 
int					nCode, 
void*				pExtra, 
AFX_CMDHANDLERINFO*	pHandlerInfo )
{
	QCFTRANSFER qcfTransfer;
	CWnd*		pWnd;

	if( ( nID == ID_EDIT_MESSAGE ) && ( nCode == 0 ) )
	{
		OnEditMode();
	}
//	else if( ( nID == ID_EDIT_MESSAGE ) && ( nCode == CN_UPDATE_COMMAND_UI ) )
//	{
//		ASSERT( pExtra );
//
//		if( pExtra )
//		{
//			( ( CCmdUI* ) pExtra )->Enable( FALSE );
//		}
//
//		return TRUE;
//	}
	
	pWnd = GetFocus();
	
	if( IsChild( pWnd ) && pWnd->IsKindOf( RUNTIME_CLASS( CEdit ) ) )
	{
		// give the edit control on the toolbar first crack at it
		if( pWnd->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
		{
			return TRUE;
		}
	}

	if( ( nID != IDC_TOW_TRUCK ) || ( nCode != STN_CLICKED ) )
	{
		return CMessageFrame::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
	}

	//
	// Grab the source TocDoc for this message and
	// save it in the clipboard for use by the drop target.
	//
	CReadMessageDoc* pDoc = (CReadMessageDoc*) GetActiveDocument();
	ASSERT_KINDOF(CReadMessageDoc, pDoc);
	CSummary* pSum = pDoc->m_Sum;
	ASSERT(pSum);
	ASSERT_KINDOF(CSummary, pSum);
	qcfTransfer.m_pSourceTocDoc = pSum->m_TheToc;
	ASSERT(qcfTransfer.m_pSourceTocDoc);
	ASSERT_KINDOF(CTocDoc, qcfTransfer.m_pSourceTocDoc);
	qcfTransfer.m_pSingleSummary = pSum;

	// Cache the HINSTANCE data
	g_theClipboardMgr.CacheGlobalData(m_OleDataSource, qcfTransfer);

	//
	// Do the drag and drop!
	//
	QCOleDropSource oleDropSource;
	DROPEFFECT dropEffect = m_OleDataSource.DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK, NULL, &oleDropSource);

	if ((dropEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
	{
		//
		// The OLE docs say that we should process
		// deletion of the selected moved items here.
		// However, since Eudora itself is the only
		// thing that handles transfers from TOCs,
		// we'll just let the Transfer code called
		// from the Drop *Target* side take care of
		// deleting the selection.
		//
	}

	// Clear the Data Source's cache
	m_OleDataSource.Empty();

	return TRUE;
}


BOOL CReadMessageFrame::OnDynamicCommand(
UINT uID )
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	
	TRANSLATE_DATA		theData;


	if( !g_theCommandStack.GetCommand( ( WORD ) uID, &pCommand, &theAction ) )
	{
		return FALSE;
	}

	if( ( pCommand == NULL ) || !theAction )
	{
		return FALSE;
	}

	if( theAction == CA_TRANSLATE_PLUGIN )
	{		
		CView*	pView = ( CView* ) m_wndSplitter.GetPane( 0, 0 ); 
		theData.m_pProtocol = QCProtocol::QueryProtocol( QCP_TRANSLATE, pView );
		
		if( theData.m_pProtocol )
		{		
			theData.m_pView = NULL;
			theData.m_bBuildAddresses = FALSE;			
			pCommand->Execute( theAction, &theData );
			return TRUE;
		}
	}
	
	return FALSE;
}


void CReadMessageFrame::OnUpdateDynamicCommand(
CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) pCmdUI->m_nID, &pCommand, &theAction ) )
		{
			if( theAction == CA_TRANSLATE_PLUGIN )
			{
				if( ( m_wndSplitter.GetPane( 0, 0 ) )->IsKindOf( RUNTIME_CLASS( PgReadMsgView ) ) )
				{
					pCmdUI->Enable( TRUE );
					return;
				}
				else if( ( m_iMajorVersion > 4 ) ||
						 (	( m_iMajorVersion >= 4 ) && 
							( m_iMinorVersion >= 71 ) ) )
				{
					// Always Enable for read messages 
					//if( GetCheck( ID_EDIT_MESSAGE ) )
					{
						pCmdUI->Enable( TRUE );
						return;
					}
				}
			}
		}
	}

	pCmdUI->ContinueRouting();
}

void CReadMessageFrame::OnPriorityChange()
{
	CSummary*			pSummary;
	CReadMessageDoc*	pDoc;
	CComboBox*			pCombo;
	INT					iSel;
	CView*				pView;

	VERIFY( pDoc = ( CReadMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_PRIORITY_COMBO ) ) );
	
	iSel = pCombo->GetCurSel();
	pSummary->SetPriority(iSel + 1, FALSE); // Change the summary info, but don't update the frame (that's us!)
	pView = ( CView* ) m_wndSplitter.GetPane( 0, 0 ); 
	pView->SetFocus();
}


////////////////////////////////////////////////////////////////////////
// OnSetPriority [protected]
//
// Update the UI when somebody changes the priority setting in the
// Summary object.
////////////////////////////////////////////////////////////////////////
long CReadMessageFrame::OnSetPriority(WPARAM, LPARAM lParam)
{
	CSummary* pSummary = (CSummary *) lParam;
	ASSERT_KINDOF(CSummary, pSummary);
	ASSERT(this == pSummary->m_FrameWnd);

	CComboBox* pCombo = (CBitmapCombo *) m_pToolBar->GetDlgItem(IDC_PRIORITY_COMBO);
	ASSERT(pCombo);

	//
	// Assume that the priority is defined as:
	//
	// 1 - highest
	// 2 - high
	// 3 - normal
	// 4 - low
	// 5 - lowest
	//
	ASSERT(pSummary->m_Priority >= 1 && pSummary->m_Priority <= 5);
	pCombo->SetCurSel(pSummary->m_Priority - 1);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnButtonSetCheck [protected]
//
// Update the UI when somebody changes the Server Status settings in
// the Summary object.
////////////////////////////////////////////////////////////////////////
long CReadMessageFrame::OnButtonSetCheck
(
	WPARAM wParam,	// button command ID
	LPARAM lParam	// TRUE if checked, FALSE if unchecked
)
{
	INT		iIndex;
	UINT	uStyle;

	iIndex = m_pToolBar->CommandToIndex( wParam );

	if( iIndex >= 0 )
	{
		uStyle = m_pToolBar->GetButtonStyle( iIndex );

		if( lParam )
		{
			uStyle |= TBBS_CHECKED;
		}
		else
		{
			uStyle &= ~TBBS_CHECKED;
		}

		m_pToolBar->SetButtonStyle( iIndex, uStyle );
	}

	return TRUE;
}



BOOL CReadMessageFrame::GetCheck( UINT nCommandID )
{
	INT		iIndex;
	UINT	uStyle;

	iIndex = m_pToolBar->CommandToIndex( nCommandID );
	
	if( iIndex >= 0 )
	{
		uStyle = m_pToolBar->GetButtonStyle( iIndex );
	}
	else
	{
		uStyle = 0;
	}
	
	return( ( uStyle & TBBS_CHECKED ) != 0 );
}


BOOL CReadMessageFrame::GetFormattingCheck( UINT nCommandID )
{
	INT		iIndex;
	UINT	uStyle;

	iIndex = m_pFormattingToolBar->CommandToIndex( nCommandID );
	
	if( iIndex >= 0 )
	{
		uStyle = m_pFormattingToolBar->GetButtonStyle( iIndex );
	}
	else
	{
		uStyle = 0;
	}
	
	return( ( uStyle & TBBS_CHECKED ) != 0 );
}


CComboBox* CReadMessageFrame::GetFontCombo() 
{
	if( m_pFormattingToolBar )
	{
		return ( CComboBox* ) ( m_pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ) );
	}

	return NULL;
}

void CReadMessageFrame::OnUpdateAlwaysEnabled(
CCmdUI*	pCmdUI)
{
	pCmdUI->Enable( TRUE );
}


void CReadMessageFrame::OnSize(UINT nType, int cx, int cy) 
{
	if( m_pToolBar )
	{
		m_pToolBar->OnSize( nType, cx, cy );
	}

	CMessageFrame::OnSize( nType, cx, cy );
}


BOOL CReadMessageFrame::QuietPrint()
{
	QCProtocol*	pProtocol;
	CView*		pView;
	BOOL		bSwappedViews;
	BOOL		bRet;

	bRet = FALSE;
	pView = ( CView* ) m_wndSplitter.GetPane( 0, 0 );
	bSwappedViews = FALSE;

	if( pView && pView->IsKindOf( RUNTIME_CLASS( CBidentReadMessageView ) ) )
	{
		pView = KludgeInAFindView();
		bSwappedViews = TRUE;
	}
	
	pProtocol = QCProtocol::QueryProtocol( QCP_QUIET_PRINT, pView );

	if( pProtocol )
	{
		bRet = pProtocol->QuietPrint();
	}

	if( bSwappedViews )
	{
		KillTheKludgyView();
	}

	return bRet;
}



void CReadMessageFrame::EnableTooltips( BOOL bEnable )
{
	CWnd*	pWnd;

	pWnd = m_wndSplitter.GetPane( 0, 0 );
	
	if( pWnd )
	{
		pWnd->EnableToolTips( bEnable );
	}
	
	if( m_pToolBar )
	{
		m_pToolBar->EnableToolTips( bEnable );
	}

	if( m_pFormattingToolBar )
	{
		m_pFormattingToolBar->EnableToolTips( bEnable );
	}

}


void CReadMessageFrame::OnChangeSubject()
{
	CReadMessageDoc* pDoc;
	
	pDoc  = (CReadMessageDoc*) GetActiveDocument();
	
	if( pDoc )
	{
		ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CReadMessageDoc)));
		// If the editable subject is changing, then update the subject
		CSummary* Sum = pDoc->m_Sum;
		CString NewSubject;

		GetSubject(NewSubject);
		Sum->SetSubject(NewSubject);
		pDoc->ReallySetTitle(Sum->MakeTitle());
		//pDoc->SetModifiedFlag( TRUE );
	}
}


BOOL CReadMessageFrame::GetSubject(
CString& szText )
{
	CEdit* pEdit;

	pEdit = ( CEdit* ) ( m_pToolBar->GetDlgItem( IDC_SUBJECT ) );

	if( pEdit )
	{
		pEdit->GetWindowText( szText );
		return TRUE;
	}

	return FALSE;
}


void CReadMessageFrame::OnDelete()
{
	CSummary*			pSummary;
	CTocDoc*			pToc;
	CReadMessageDoc*	pDoc;

	VERIFY( pDoc = ( CReadMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pToc = pSummary->m_TheToc );

	if ( pDoc->IsModified() )
		pDoc->SaveModified();

//	if( pDoc->OnSaveDocument( NULL ) )
//	{
#ifdef IMAP4 //
		//
		// Deleting messages from an IMAP mailbox just flags them.
		//
		if ( pSummary && pToc && pToc->IsImapToc () )
		{
			QCMailboxCommand* pSrcCommand = g_theMailboxDirector.FindByPathname( (const char *) pToc->MBFilename() );
			if (pSrcCommand)
			{
				//
				// This will call the Execute() method in the inherited
				// "QCImapMailboxCommand" class.
				//
				pSrcCommand->Execute (CA_DELETE_MESSAGE, pSummary);
			}
			else
			{
				ASSERT (0);
			}

			//
			// Don't continue.
			//
			return;
		}
#endif // IMAP4

		//
		// We no longer call Xfer() here since we want to go through
		// the command director logic for CA_TRANSFER_TO.  The command
		// director notifies the TOC window containing this message
		// that it is okay to invoke the "auto-mark-as-read" logic
		// since the mailbox was changed due to an explicit user
		// action.  The Xfer() function does not notify the TOC
		// window.
		//
		//pToc->Xfer( GetTrashToc(), pSummary );
		CTocDoc* pTrashToc = ::GetTrashToc();
		if (pTrashToc)
		{
			QCMailboxCommand* pFolder = g_theMailboxDirector.FindByPathname( (const char *) pTrashToc->MBFilename() );
			if (pFolder != NULL)
				pFolder->Execute(CA_TRANSFER_TO, pSummary);
			else
			{
				ASSERT(0);
			}
		}
//	}
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Common implementation for the Trident and Paige preview panes.
//
// FORNOW, this is a duplicate of code in CTocFrame.  By putting it in
// both places, we allow the future possibility of having different
// context menus in previews vs real views (Christie made me do it).
////////////////////////////////////////////////////////////////////////
void CReadMessageFrame::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = ::QCLoadMenu(IDR_CONTEXT_POPUPS);
	if ( ! hMenu || ! menu.Attach( hMenu ) )
	{
		ASSERT(0);		// resources hosed?
		return;
	}

	// MP_POPUP_RECEIVED_MSG is the offset for the read message submenu.
	CMenu* pMenuPopup = menu.GetSubMenu(MP_POPUP_RECEIVED_MSG);
	if (pMenuPopup != NULL)
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

		CMenu theTransferMenu;
		theTransferMenu.CreatePopupMenu();

		extern QCMailboxDirector g_theMailboxDirector;
		extern QCPluginDirector g_thePluginDirector;

		//
		// Populate the new Transfer popup menu.
		//
		g_theMailboxDirector.NewMessageCommands( CA_TRANSFER_TO, &theTransferMenu, CA_TRANSFER_NEW );
		::WrapMenu( theTransferMenu.GetSafeHmenu() );
		theTransferMenu.InsertMenu( 3, MF_BYPOSITION, ID_TRANSFER_NEW_MBOX_IN_ROOT, CRString( IDS_MAILBOX_NEW ) );
		theTransferMenu.InsertMenu( 3, MF_BYPOSITION | MF_SEPARATOR );

		tempPopupMenu.InsertMenu(	MP_TRANSFER_READMSG,
									MF_BYPOSITION | MF_POPUP,
									(UINT) theTransferMenu.GetSafeHmenu(), 
									CRString( IDS_TRANSFER_NAME ) );

		::WrapMenu( tempPopupMenu.GetSafeHmenu() );

		//
		// Insert the Message Plug-Ins sub-menu.
		//
		CMenu theMessagePluginsMenu;
		theMessagePluginsMenu.CreatePopupMenu();
		g_thePluginDirector.NewMessageCommands( CA_TRANSLATE_PLUGIN, &theMessagePluginsMenu );
		int nMessagePluginsPosition = 0;
		if (theMessagePluginsMenu.GetMenuItemCount() > 0)
		{
			nMessagePluginsPosition = tempPopupMenu.GetMenuItemCount();
			::WrapMenu( theMessagePluginsMenu.GetSafeHmenu() );
			tempPopupMenu.InsertMenu(	nMessagePluginsPosition - 1,
										MF_BYPOSITION | MF_POPUP,
										(UINT) theMessagePluginsMenu.GetSafeHmenu(),
										CRString(IDS_MESSAGE_PLUGINS));
			tempPopupMenu.InsertMenu(nMessagePluginsPosition, MF_BYPOSITION|MF_SEPARATOR);
		}
		
		CContextMenu::MatchCoordinatesToWindow(pWnd->GetSafeHwnd(), ptScreen);
		CContextMenu(&tempPopupMenu, ptScreen.x, ptScreen.y);
		

		// Remove the Message Plug-Ins sub-menu.
		//
		g_thePluginDirector.RemoveMessageCommands( CA_TRANSLATE_PLUGIN, &theMessagePluginsMenu );
		if (nMessagePluginsPosition)
			tempPopupMenu.RemoveMenu( nMessagePluginsPosition, MF_BYPOSITION );

		tempPopupMenu.RemoveMenu(MP_TRANSFER_READMSG, MF_BYPOSITION);

		// Remove the Transfer sub-menu
		//
		g_theMailboxDirector.RemoveMessageCommands(CA_TRANSFER_TO, &theTransferMenu);
		g_theMailboxDirector.RemoveMessageCommands(CA_TRANSFER_NEW, &theTransferMenu);
		
		VERIFY(tempPopupMenu.Detach());
	}
	else
	{
		ASSERT(0);		// resources hosed?
	}

	menu.DestroyMenu();
} 


QCChildToolBar* CReadMessageFrame::GetFormatToolbar() 
{
	if( m_pFormattingToolBar )
	{
		return m_pFormattingToolBar; 
	}

	return NULL;
}

QCChildToolBar* CReadMessageFrame::GetToolbar() 
{
	if( m_pToolBar )
	{
		return m_pToolBar; 
	}

	return NULL;
}


BOOL CReadMessageFrame::UseFixedWidthFont()
{
	return GetCheck( ID_USE_FIXED_FONT );
}


CView* CReadMessageFrame::KludgeInAFindView()
{
	SetRedraw( FALSE );
	m_wndSplitter.SetViewType( RUNTIME_CLASS( PgReadMsgView ) );
	m_wndSplitter.SplitRow( 5 );
	return ( CView* )( m_wndSplitter.GetPane( 1, 0 ) );
}


void CReadMessageFrame::KillTheKludgyView()
{
	m_wndSplitter.DeleteRow( 1 );			
	SetRedraw( TRUE );
}


void CReadMessageFrame::KillOriginalView()
{
	m_wndSplitter.DeleteRow( 0 );			
	SetRedraw( TRUE );
}

// --------------------------------------------------------------------------

//
// FIND TEXT
//

void CReadMessageFrame::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CReadMessageFrame::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG CReadMessageFrame::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
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

	CReadMessageDoc *pDoc = (CReadMessageDoc *) GetActiveDocument();
	ASSERT(pDoc);
	
	if (!pDoc)
		return (EuFIND_ERROR);

	if (pDoc->FindNext(lpFR->lpstrFindWhat, (lpFR->Flags & FR_MATCHCASE), (lpFR->Flags & FR_WHOLEWORD)))
		return (EuFIND_OK);

	return (EuFIND_NOTFOUND);
}

// --------------------------------------------------------------------------

#ifdef IMAP4 // IMAP4
//
// IMAP-specific stuff.
//
void CReadMessageFrame::OnUpdateUndelete(CCmdUI* pCmdUI)
{
	CSummary*			pSummary;
	CTocDoc*			pToc;
	CReadMessageDoc*	pDoc;

	//
	// Enable undelete if this is an IMAP mailbox and this message 
	// is marked with the IMAP \delete flag.
	//

	VERIFY( pDoc = ( CReadMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pToc = pSummary->m_TheToc );

	if ( pToc && pToc->IsImapToc() && pSummary && (pSummary->m_Imflags & IMFLAGS_DELETED) )
	{
		pCmdUI->Enable(TRUE);

		return;
	}

	// If we get here.
	pCmdUI->Enable(FALSE);
}


void CReadMessageFrame::UnDelete()
{
	CSummary*			pSummary;
	CTocDoc*			pToc;
	CReadMessageDoc*	pDoc;

	VERIFY( pDoc = ( CReadMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pToc = pSummary->m_TheToc );

	//
	// Remove the \deleted flags from the message.
	//
	if (pSummary && pToc && pToc->IsImapToc () )
	{
		QCMailboxCommand* pSrcCommand = g_theMailboxDirector.FindByPathname( (const char *) pToc->MBFilename() );
		if (pSrcCommand)
		{
			//
			// This will call the Execute() method in the inherited
			// "QCImapMailboxCommand" class.
			//
			pSrcCommand->Execute (CA_UNDELETE_MESSAGE, pSummary);
		}
		else
		{
			ASSERT (0);
		}
	}
}

//
#endif // IMAP4

