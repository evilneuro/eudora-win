// CompMessageFrame.cpp: implementation of the CCompMessageFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include "mainfrm.h"
#include "tocdoc.h"

#include "CompMessageFrame.h"
#include "CompMsgD.h"
#include "HeaderVW.h"
#include "Summary.h"
#include "utils.h"

#include "QCChildToolBar.h"
#include "TBarMenuButton.h"
#include "TBarBmpCombo.h"
#include "TBarCombo.h"
#include "TBarMenuButton.h"
#include "TBarSendButton.h"

#include "PgCompMsgView.h"
#include "PgFixedCompMsgView.h"




#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "QCToolBarManager.h"

#include "QCFindMgr.h"
#include "QCSharewareManager.h"


#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern QCCommandStack		g_theCommandStack;
extern QCPluginDirector		g_thePluginDirector;
extern QCSignatureDirector	g_theSignatureDirector;

//YUCK!!!
extern int MessageCascadeSpot;


UINT	umsgAddAttachment = RegisterWindowMessage( "umsgAddAttachment" );
UINT	umsgSetSignature = RegisterWindowMessage( "umsgSetSignature" );
UINT	umsgSetPriority = RegisterWindowMessage( "umsgSetPriority" );

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


BEGIN_BUTTON_MAP( theCompMessageButtonMap )
	STD_BUTTON( ID_QUOTED_PRINTABLE, TBBS_CHECKBOX )
	STD_BUTTON( ID_TEXT_AS_DOCUMENT, TBBS_CHECKBOX )
	STD_BUTTON( ID_WORD_WRAP, TBBS_CHECKBOX )
	STD_BUTTON( ID_TABS_IN_BODY, TBBS_CHECKBOX )
	STD_BUTTON( ID_KEEP_COPIES, TBBS_CHECKBOX )
	
	STD_BUTTON( ID_RETURN_RECEIPT, TBBS_CHECKBOX )
	
	STD_BUTTON( ID_BLKFMT_BULLETTED_LIST, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_INSERT_LINK, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_INSERT, TBBS_BUTTON )

	STD_BUTTON(ID_INVISIBLES, TBBS_CHECKBOX )

	STD_BUTTON( ID_EDIT_TEXT_BOLD, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_ITALIC, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_UNDERLINE, TBBS_INDETERMINATE )
	STD_BUTTON( ID_EDIT_TEXT_TT, TBBS_INDETERMINATE )

	
	STD_BUTTON( ID_EDIT_TEXT_LEFT, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_CENTER, TBBS_CHECKBOX )
	STD_BUTTON( ID_EDIT_TEXT_RIGHT, TBBS_CHECKBOX )

	WND_BUTTON(BUTTON_CLASS(TBarSendButton), ID_MESSAGE_SENDIMMEDIATELY, 0, 0, 0, 0, 0, 0 )

	BMPCOMBO_BUTTON( ID_PRIORITY, IDC_PRIORITY_COMBO, 0, CBS_DROPDOWN, 150, 40, 150 )
	BMPCOMBO_BUTTON( ID_SIGNATURE, IDC_SIGNATURE_COMBO, 0, CBS_DROPDOWN, 150, 40, 150 )
	BMPCOMBO_BUTTON( ID_ENCODING, IDC_ENCODING_COMBO, 0, CBS_DROPDOWN, 150, 40, 150 )

	TBARCOMBO_BUTTON( ID_FONT, IDC_FONT_COMBO, 0, WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, 115, 40, 115 )
	
	TBARMENU_BUTTON( ID_EDIT_TEXT_SIZE, TBBS_BUTTON )
	TBARMENU_BUTTON( ID_EDIT_TEXT_COLOR, TBBS_BUTTON )
	
	TBARMENU_BUTTON( ID_TRANSLATORS_QUEUE, TBBS_BUTTON )

END_BUTTON_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttachDlg

// this was living in compmsgd.cpp, now it is living here. It really should
// be living in its own modules. (maybe later.)


class CAttachDlg : public CFileDialog
{
public:
	CAttachDlg(CWnd* pParentWnd);

	BOOL OnInitDialog();

protected:
	CString m_Title;
};

CAttachDlg::CAttachDlg(CWnd* pParentWnd) :
	CFileDialog(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST 
		| OFN_HIDEREADONLY 
		| OFN_ALLOWMULTISELECT
		| OFN_SHAREAWARE,
		CRString(IDS_ALL_FILES_FILTER), pParentWnd)
{
	m_Title = CRString(IDS_ATTACH_FILE);
		
	m_ofn.lpstrTitle = m_Title;
	m_ofn.Flags &= ~OFN_SHOWHELP;
}

BOOL CAttachDlg::OnInitDialog()
{
	CWnd* OKButton = GetDlgItem(IDOK);
	
	if (OKButton)
		OKButton->SetWindowText(CRString(IDS_ATTACH));
	
	return (TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CCompMessageFrame

IMPLEMENT_DYNCREATE(CCompMessageFrame, CMessageFrame)

BEGIN_MESSAGE_MAP(CCompMessageFrame, CMessageFrame)
	//{{AFX_MSG_MAP(CCompMessageFrame)
	ON_WM_DESTROY()
	ON_COMMAND(ID_MESSAGE_ATTACHFILE, OnAttachFile)
	ON_WM_DROPFILES()
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS_STATIONERY, OnUpdateFileSaveAsStationery)
	ON_COMMAND(ID_EDIT_CHECKSPELLING, OnCheckSpelling)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CHECKSPELLING, OnUpdateCheckSpelling)
	ON_UPDATE_COMMAND_UI(IDM_SYSTEM_CONFIGURATION, OnUpdateInsertSystemConfiguration)
	ON_COMMAND(IDM_SYSTEM_CONFIGURATION, OnInsertSystemConfiguration)
  	//}}AFX_MSG_MAP

	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)

	ON_UPDATE_COMMAND_UI(ID_TRANSLATORS_QUEUE, OkIfEditable)

	ON_CBN_SELCHANGE( IDC_PRIORITY_COMBO, OnPriorityChange )
	ON_CBN_SELCHANGE( IDC_SIGNATURE_COMBO, OnSignatureChange )
	ON_CBN_SELCHANGE( IDC_ENCODING_COMBO, OnEncodingChange )

	ON_BN_CLICKED( ID_QUOTED_PRINTABLE, OnQuotedPrintable )
	ON_BN_CLICKED( ID_TEXT_AS_DOCUMENT, OnTextAsDocument )
	ON_BN_CLICKED( ID_WORD_WRAP, OnWordWrap )
	ON_BN_CLICKED( ID_TABS_IN_BODY, OnTabsInBody )
	ON_BN_CLICKED( ID_KEEP_COPIES, OnKeepCopies )
	ON_BN_CLICKED( ID_RETURN_RECEIPT, OnReturnReceipt )

	ON_REGISTERED_MESSAGE( umsgAddAttachment, OnAddAttachment )
	ON_REGISTERED_MESSAGE( umsgSetSignature, OnSetSignature )
	ON_REGISTERED_MESSAGE( umsgSetPriority, OnSetPriority )

	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )

	ON_UPDATE_COMMAND_UI( ID_QUOTED_PRINTABLE, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_TEXT_AS_DOCUMENT, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_WORD_WRAP, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_TABS_IN_BODY, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_KEEP_COPIES, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_RETURN_RECEIPT, OkIfEditable )
	ON_UPDATE_COMMAND_UI( IDC_PRIORITY_COMBO, OkIfEditable )
	ON_UPDATE_COMMAND_UI( IDC_SIGNATURE_COMBO, OkIfEditable )
	ON_UPDATE_COMMAND_UI( IDC_ENCODING_COMBO, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_TRANSLATORS_QUEUE, OkIfEditable )
	ON_UPDATE_COMMAND_UI( ID_MESSAGE_SENDIMMEDIATELY, OkIfNotStationery )

//	ON_UPDATE_COMMAND_UI( IDC_FONT_COMBO, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_SIZE, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_BOLD, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_ITALIC, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_UNDERLINE, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_COLOR, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_LEFT, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_CENTER, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_RIGHT, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_INDENT_IN, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_INDENT_OUT, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_CLEAR, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_INSERT, OkIfEditable )
//	ON_UPDATE_COMMAND_UI( ID_EDIT_INSERT_LINK, OkIfEditable )

	ON_MESSAGE( WM_USER_UPDATE_IMMEDIATE_SEND, OnUserUpdateImmediateSend )

	ON_COMMAND(IDA_TOGGLEHEADER, OnToggleHeader)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompMessageFrame::CCompMessageFrame()
{
	m_pToolBarManager = NULL;
	m_pToolBar = NULL;
	m_pFormattingToolBar = NULL;
	g_theSignatureDirector.Register( this );
	m_nHeaderHeight = 0;
}

CCompMessageFrame::~CCompMessageFrame()
{
	g_theSignatureDirector.UnRegister( this );
	delete m_pToolBarManager;
}

//HBRUSH CCompMessageFrame::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
//{
//	if (CTLCOLOR_LISTBOX == nCtlColor)
//	{
//		pDC->SetBkColor((COLORREF)0x00E7FFFF);	
//	}
//	else
//		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
//	pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
//	return (GetSysColorBrush(COLOR_WINDOW));
//}

BOOL CCompMessageFrame::OnCreateClient( 
LPCREATESTRUCT lpcs,
CCreateContext* pContext)
{
	BOOL				fRet;
	CCompMessageDoc*	pDoc;
	DWORD				dwStyle;
	DWORD				dwExStyle;
	CBitmapCombo*		pBitmapCombo;
	int					i;
	CSummary*			pSummary;
	CComboBox*			pCombo;
	CStringArray		theArray;
	CMainFrame*			pMainFrame;
	BOOL				bMaximized;
	CMenu*				pEditTextMenu;
	CMenu*				pMenu;
	CTBarMenuButton*	pMenuButton;
	POSITION			pos;
	CBitmapComboItem*	pBitmapComboItem;
	QCSignatureCommand*	pSignatureCommand;
	CString				szCurrentSig;
	UINT				nButtons;

	// Major Caution!!

	//when ever a new button is added to this list, make sure the old works right by using the right
	//offsets.  
	UINT				theCompMessageButtons[] = {
							ID_PRIORITY,
							ID_SIGNATURE,
							ID_ENCODING,
							ID_SEPARATOR,
							ID_QUOTED_PRINTABLE,
							ID_TEXT_AS_DOCUMENT,
							ID_WORD_WRAP,
							ID_TABS_IN_BODY,
							ID_KEEP_COPIES,
							ID_RETURN_RECEIPT,
							ID_SEPARATOR,
							ID_INVISIBLES,
							ID_SEPARATOR,
							ID_TRANSLATORS_QUEUE,   //This offset used in GetButton(13) below.  Chnage it if this list changes
							ID_SEPARATOR,
							ID_MESSAGE_SENDIMMEDIATELY
						};

	fRet = m_wndSplitter.CreateStatic( this, 2, 1, WS_CHILD | WS_VISIBLE );

	if ( fRet ) {
		fRet = m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS(CHeaderView),
					CSize( 315, 88 ), pContext );
	}

	if ( fRet ) 
	{
		if(GetIniShort(IDS_INI_WORD_WRAP_ON_SCREEN) && GetIniShort(IDS_INI_WORD_WRAP))
		{
			fRet = m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS(PgFixedCompMsgView),
					CSize( 10, 600 ), pContext );
		}
		else
		{
			fRet = m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS(PgCompMsgView),
					CSize( 10, 600 ), pContext );
		}
	}

	pDoc = ( CCompMessageDoc* ) ( pContext->m_pCurrentDoc );

	// force the read
	pDoc->GetText();

	// see if the message has been addressed & has a subject
	if (	( pDoc ->m_Headers[ HEADER_TO ] != "" ) &&
			( pDoc ->m_Headers[ HEADER_SUBJECT ] != "" ) ) {

		// Set the active view to the body
		m_wndSplitter.SetActivePane( 1, 0 );
	}
	else {
		// HACK - Paige view must be "active" for a moment following creation
		// or things get kinda fubar. Strange goings-on indeed.
		SetActiveView( (CView*) m_wndSplitter.GetPane( 1, 0 ) );
		SetActiveView( (CView*) m_wndSplitter.GetPane( 0, 0 ) );
	}

	pSummary = pDoc->m_Sum;
	
	// build the translators menu
	m_theTranslatorMenu.CreatePopupMenu();

	g_thePluginDirector.NewMessageCommands( CA_TRANSMISSION_PLUGIN, &m_theTranslatorMenu );
	
	if( ( i = m_theTranslatorMenu.GetMenuItemCount() ) != 0 )
	{
		while ( i-- )
		{
			m_thePropertyList.Add( "" );
		}
	}

	::WrapMenu( m_theTranslatorMenu.GetSafeHmenu() );

	m_pToolBarManager = new QCToolBarManagerWithBM( this );
	m_pToolBar = new QCChildToolBar;  
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
	

	m_pToolBarManager->LoadToolBarResource( MAKEINTRESOURCE( IDR_COMPMESS ), MAKEINTRESOURCE( IDR_COMPMESS ) );
	m_pToolBarManager->SetButtonMap( theCompMessageButtonMap );



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

	m_pToolBarManager->SetToolBarInfo( m_pToolBar );
	m_pToolBar->EnableDocking(CBRS_ALIGN_TOP);

	nButtons = DIM( theCompMessageButtons );
	m_pToolBar->SetButtons( theCompMessageButtons,  nButtons );

	int transCount = m_theTranslatorMenu.GetMenuItemCount();
	
	// See if we need to get rid of xlator dropdown and maybe replace it with icons
	if (!transCount || (!GetIniShort(IDS_INI_COMP_COMPACT_TRANSLTORS) && transCount > 0))
	{
		for (i = 0; (UINT)i < nButtons; i++)
		{
			if (theCompMessageButtons[i] == ID_TRANSLATORS_QUEUE)
			{
				// Hide this instead of remove because some sentinal memory problem
				m_pToolBar->SetButtonStyle(i, (unsigned int)~WS_VISIBLE);
				if (transCount)
					m_pToolBar->AddTranslatorButtons(++i);
				else
					m_pToolBar->RemoveButton(i+1, TRUE);

				break;
			}
		}
	}
	
	
	// force the toolbar to recalculate the button sizes
	m_pToolBarManager->SetToolBarInfo( m_pToolBar );
	DockControlBar( m_pToolBar );
	

	// The formatting toolbar should be hidden if one of the following occurs:
	//   1. The user asked not to show it
	//   2. The message can't be edited
	//   3. The default action is to send only plain parts without asking
	if (!GetIniShort(IDS_INI_SHOW_STYLED_TEXT_TOOLBAR) ||
		pSummary->CantEdit() ||
		(GetIniShort(IDS_INI_SEND_PLAIN_ONLY) && !GetIniShort(IDS_INI_WARN_QUEUE_STYLED_TEXT)))
	{
		dwStyle = dwStyle & ~WS_VISIBLE;
	}

	// now create the formatting tool bar
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
	
	// initialize the priority combo
	pBitmapCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_PRIORITY_COMBO ) );
	
	for (i = IDB_PRIOR_HIGHEST; i <= IDB_PRIOR_LOWEST; i++)
		pBitmapCombo->Add(new CBitmapComboItem(i, i - IDB_PRIOR_HIGHEST + IDS_PRIORITY_HIGHEST));

	pBitmapCombo->SetCurSel( pSummary->m_Priority - 1 );

	// initialize the signature combo
	pBitmapCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_SIGNATURE_COMBO ) );
	pBitmapCombo->SetEditWidth( 100 );

	pBitmapCombo->Add( new CBitmapComboItem( 0, IDS_SIGNATURE_NONE, 0 ) );

	pos = g_theSignatureDirector.GetFirstSignaturePosition();

	if ( pSummary->UseSignature() == MSF_USE_SIGNATURE )
	{
		// look for the standard signature file
		szCurrentSig = CRString( IDS_STANDARD_SIGNATURE );
	}
	else if ( pSummary->UseSignature() ==  MSF_ALT_SIGNATURE )
	{
		// user defined signature
		szCurrentSig = (const char *)pSummary->m_SigSelected;
	}
	else if( pSummary->UseSignature() )
	{
		// look for the alternate signature file
		szCurrentSig = CRString( IDS_ALTERNATE_SIGNATURE32  );
	}

	while( pos != NULL )
	{
		pSignatureCommand = g_theSignatureDirector.GetNext( pos );
					
		pBitmapComboItem = new CBitmapComboItem( 0, pSignatureCommand->GetName(), 0);
		
		if ( pBitmapComboItem )
		{
			i = pBitmapCombo->Add( pBitmapComboItem );

			if( ( i != CB_ERR ) && ( szCurrentSig.CompareNoCase( pBitmapComboItem->m_Text ) == 0 ) )
			{
				pBitmapCombo->SetCurSel( i );
			}
		}
	}
	
	if( pBitmapCombo->GetCurSel() == CB_ERR )
	{
		pBitmapCombo->SetCurSel( 0 );
	}

	// initialize the encoding combo
	pBitmapCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_ENCODING_COMBO ) );
	pBitmapCombo->Add( new CBitmapComboItem(IDB_ENCODING_MIME, IDS_ENCODING_MIME, MSF_MIME));
	pBitmapCombo->Add( new CBitmapComboItem(IDB_ENCODING_BINHEX, IDS_ENCODING_BINHEX, MSF_BINHEX));
	pBitmapCombo->Add( new CBitmapComboItem(IDB_ENCODING_UUENCODE, IDS_ENCODING_UUENCODE, MSF_UUENCODE));

	if( pSummary->Encoding() == MSF_BINHEX )
	{
		pBitmapCombo->SetCurSel( 1 );
	}
	else if( pSummary->Encoding() == MSF_UUENCODE )
	{
		pBitmapCombo->SetCurSel( 2 );
	}
	else
	{
		pBitmapCombo->SetCurSel( 0 );
	}


	if( m_theTranslatorMenu.GetMenuItemCount() )
	{
		VERIFY( pMenuButton = ( CTBarMenuButton* ) ( m_pToolBar->GetButton(13)));	//change 13 when a new button is added before ID_TRANSLATORS_QUEUE
		pMenuButton->SetHMenu( m_theTranslatorMenu.GetSafeHmenu() );
		SelectTranslators( pSummary->GetTranslators() );

// BOG: This was incomplete protection against "dead" trans ids in the message
// summary---we're now doing it another way.
//		pSummary->SetTranslators(GetSelectedTranslators());
	}

	// get the face names
	EnumFontFaces( theArray );

	pCombo = ( CComboBox* ) ( m_pFormattingToolBar->GetDlgItem( IDC_FONT_COMBO ) );

	for( i = 0; i < theArray.GetSize(); i ++ )
	{
		pCombo->AddString( theArray[i] );
	}

	// get the main frame window
	pMainFrame = ( CMainFrame* ) AfxGetApp()->m_pMainWnd;
	
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

	SetCheck( ID_QUOTED_PRINTABLE, pSummary->UseQP() );
	SetCheck( ID_TEXT_AS_DOCUMENT, pSummary->TextAsDoc() );
	SetCheck( ID_WORD_WRAP, pSummary->WordWrap() );
	SetCheck( ID_TABS_IN_BODY, pSummary->TabsInBody() );
	SetCheck( ID_KEEP_COPIES, pSummary->KeepCopies() );
	SetCheck( ID_RETURN_RECEIPT, (pSummary->ReadReceipt() || pSummary->ReturnReceipt()) );

//	m_ToolBar->SetSignature(Sum);
//	m_ToolBar->SetEncoding(Sum);

	// FUBAR -- Gotta do this after the message was read in, so we will need to
	// provide an "accessor" routine to the view (perhaps ;-)
//	m_ToolBar->SelectTranslators(Sum->GetTranslators());
	
	if (pSummary->CantEdit()) {
//		m_ToolBar->GetDlgItem(IDC_SIGNATURE_COMBO)->EnableWindow(FALSE);
//		m_ToolBar->GetDlgItem(IDC_ENCODING_COMBO)->EnableWindow(FALSE);
//		m_ToolBar->GetDlgItem(ID_MESSAGE_SENDIMMEDIATELY)->EnableWindow(FALSE);
//		m_ToolBar->EnableTransMenu(FALSE);

// FUBAR
//		GetEditCtrl().SetReadOnly( TRUE );
//		m_bIsReadOnly = TRUE;
	}
	else
		DragAcceptFiles();

	// force the toolbar to recalculate the button sizes
	m_pToolBarManager->SetToolBarInfo( m_pToolBar );
	RecalcLayout();

	// Size parent window
	CRect MainWindowRect;
	pMainFrame->GetRealClientRect(&MainWindowRect);
 
	if (pSummary->GetSavedPos().IsRectEmpty() == FALSE) {
		// Window has been sized before, so use it
		m_InitialSize = pSummary->GetSavedPos();
	}
	else
	{
		const CFontInfo& TheMessageFont = GetMessageFont();
		int CH = TheMessageFont.CellHeight();
		int CW = TheMessageFont.CharWidth();

		int CascadeX = (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXSIZE)) *
			MessageCascadeSpot;
		int CascadeY = (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYSIZE)) *
			MessageCascadeSpot;
		
		// Start with toolbar size
		m_pToolBar->GetClientRect(&m_InitialSize);
		
		// Add height of caption and top and bottom window borders
		m_InitialSize.bottom += GetSystemMetrics(SM_CYCAPTION) +
			GetSystemMetrics(SM_CYFRAME) * 2 + 6;
		
		// Use Message Width setting plus left and right window borders plus scrollbar width.
		// Make sure window is at least as wide as the toolbars
		CRect FTBRect;
		m_pFormattingToolBar->GetClientRect(&FTBRect);
		int MW = GetIniShort(IDS_INI_MESSAGE_WIDTH) * CW + GetSystemMetrics(SM_CXVSCROLL);
		if (m_InitialSize.right < MW)
			m_InitialSize.right = MW;
		if (m_InitialSize.right < FTBRect.right)
			m_InitialSize.right = FTBRect.right;
		m_InitialSize.right += GetSystemMetrics(SM_CXFRAME) * 2 + 6;
		
		// Use Message Lines setting
		m_InitialSize.bottom += GetIniShort(IDS_INI_MESSAGE_LINES) * CH;

		// If cascading windows, see if this one would fit
		if (MessageCascadeSpot > 0) {
			if (m_InitialSize.right + CascadeX > MainWindowRect.right ||
				m_InitialSize.bottom + CascadeY > MainWindowRect.bottom) {
				MessageCascadeSpot = 0;
			}
			else {
				m_InitialSize.left   += CascadeX;
				m_InitialSize.right  += CascadeX;
				m_InitialSize.top    += CascadeY;
				m_InitialSize.bottom += CascadeY;
			}
		}

		if (MessageCascadeSpot >= 0)
			MessageCascadeSpot++;
	}
	
	// place the window
	MoveWindow(&m_InitialSize, FALSE);

	// *guarantee* this is off initially
	pDoc->SetModifiedFlag(FALSE);
	return fRet;
}


void CCompMessageFrame::OnDestroy()
{
	CCompMessageDoc* doc = (CCompMessageDoc*) GetActiveDocument();
	CSummary* Sum = (doc? doc->m_Sum : NULL);
	
	if (Sum)
	{
//		m_ToolBar->SaveInfo(Sum);

		if (stricmp((const char *)Sum->m_SigSelected, (const char *)Sum->m_SigHdr))
		{
			doc->SetModifiedFlag(TRUE);
			Sum->m_SigHdr = Sum->m_SigSelected;
		}

		CTocDoc* Toc = Sum->m_TheToc;
		if (Toc)
		{
			Toc->SetModifiedFlag(TRUE);

			// If we've never saved this message, and nothing was done to it, then
			// just get rid of it without leaving a trace
			if (!doc->m_HasBeenSaved)
			{
				Toc->RemoveSum(Sum);
				doc->m_Sum = NULL;
			}
		}
	}
	
	CMessageFrame::OnDestroy();
}


CHeaderView* CCompMessageFrame::GetHeaderView()
{
	CHeaderView* pHV = (CHeaderView*) m_wndSplitter.GetPane( 0, 0 );
	VERIFY( pHV );
	ASSERT( pHV->IsKindOf(RUNTIME_CLASS(CHeaderView)) );
	return pHV;
}


void CCompMessageFrame::OnAttachFile()
{
	CAttachDlg dlg( this );
	
	if ( dlg.DoModal() == IDOK ) {
		CCompMessageDoc* pMD = (CCompMessageDoc*) GetActiveDocument();
		VERIFY( pMD );
		ASSERT( pMD->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)) );
		POSITION pos = dlg.GetStartPosition();
		while ( pos )
			pMD->AddAttachment( dlg.GetNextPathName( pos ) );
	}
}


void CCompMessageFrame::OnDropFiles( HDROP hDropInfo )
{
	int Count = DragQueryFile( hDropInfo, (UINT)(-1), NULL, 0 );
	char Filename[_MAX_PATH + 1];
	CCompMessageDoc* pMD = (CCompMessageDoc*) GetActiveDocument();
	VERIFY( pMD );
	ASSERT( pMD->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)) );

	for ( int i = 0; i < Count; i++ ) {
		
		if ( DragQueryFile( hDropInfo, i, Filename, sizeof(Filename) ) > 0 ) {
			if ( pMD )
				pMD->AddAttachment( Filename );
		}
	}
}


void CCompMessageFrame::EnableTooltips( BOOL bEnable )
{
}

void CCompMessageFrame::OnUpdateFileSaveAsStationery(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}


void CCompMessageFrame::SetCheck( UINT nCommandID, BOOL bChecked )
{
	INT		iIndex;
	UINT	uStyle;

	iIndex = m_pToolBar->CommandToIndex( nCommandID );

	if( iIndex >= 0 )
	{
		uStyle = m_pToolBar->GetButtonStyle( iIndex );

		if( bChecked )
		{
			uStyle |= TBBS_CHECKED;
		}
		else
		{
			uStyle &= ~TBBS_CHECKED;
		}

		m_pToolBar->SetButtonStyle( iIndex, uStyle );
	}
}



BOOL CCompMessageFrame::GetCheck( UINT nCommandID )
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


void CCompMessageFrame::OnCheckSpelling()
{
	// Shareware: In reduced feature mode, you cannot spell check
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		CView* View = (CView*) m_wndSplitter.GetPane( 1, 0 );
		QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_SPELL, View );

		if( pProtocol != NULL ) 
			pProtocol->CheckSpelling(FALSE);
	}
}

void CCompMessageFrame::OnUpdateCheckSpelling(CCmdUI* pCmdUI)
{
	// Shareware: In reduced feature mode, you cannot spell check
	pCmdUI->Enable(UsingFullFeatureSet());
}

void CCompMessageFrame::OnUpdateInsertSystemConfiguration(CCmdUI* pCmdUI)
{
	CCompMessageDoc* doc = DYNAMIC_DOWNCAST(CCompMessageDoc, GetActiveDocument());

	pCmdUI->Enable(doc && doc->m_Sum && doc->m_Sum->CantEdit() == FALSE);
}

void CCompMessageFrame::OnInsertSystemConfiguration()
{
	CCompMessageDoc* doc = DYNAMIC_DOWNCAST(CCompMessageDoc, GetActiveDocument());

	if (doc == NULL || doc->m_Sum == NULL)
		return;

	PgCompMsgView* View = DYNAMIC_DOWNCAST(PgCompMsgView, m_wndSplitter.GetPane( 1, 0 ));

	if (!View)
		return;

	View->SaveInfo();

	CString sAttach(doc->GetHeaderLine(HEADER_ATTACHMENTS));
	CString sBody;
	doc->ReadSystemInfo(sAttach, sBody);
	doc->SetHeaderLine(HEADER_ATTACHMENTS, sAttach);
	View->SetSelectedText(sBody,FALSE);

	doc->m_Sum->UnsetFlag(MSF_TEXT_AS_DOC);
	doc->m_Sum->SetFlagEx(MSFEX_AUTO_ATTACHED);
	SetCheck(ID_TEXT_AS_DOCUMENT, TRUE);

	doc->SetModifiedFlag(TRUE);
}


void CCompMessageFrame::OnPriorityChange()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;
	CComboBox*			pCombo;
	INT					iSel;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_PRIORITY_COMBO ) ) );
	
	iSel = pCombo->GetCurSel();
	pSummary->m_Priority = ( short ) ( iSel + 1 );
	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::OnSignatureChange()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;
	CComboBox*			pCombo;
	INT					iSel;
	CBitmapComboItem*	pData;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_SIGNATURE_COMBO ) ) );
	
	iSel = pCombo->GetCurSel();

	if( iSel < 1 ) 
	{
		if ( !( pSummary->m_SigSelected.IsEmpty() ) )
		{
			pSummary->UnsetFlag( MSF_USE_SIGNATURE | MSF_ALT_SIGNATURE );
			pSummary->m_SigSelected.Empty();
			pSummary->m_SigHdr.Empty();
			pSummary->SetFlag(0); // I left this in, but it looks like a bug
			pDoc->SetModifiedFlag( TRUE );
		}
	}
	else
	{		
		pData = ( CBitmapComboItem* ) pCombo->GetItemDataPtr( iSel );
		if ( stricmp(pSummary->m_SigSelected, pData->m_Text) != 0 )
		{
			pSummary->UnsetFlag( MSF_USE_SIGNATURE | MSF_ALT_SIGNATURE );
			pSummary->m_SigSelected = pData->m_Text;
			pSummary->SetFlag( MSF_ALT_SIGNATURE );
			pDoc->SetModifiedFlag( TRUE );
		}
	}
}



void CCompMessageFrame::OnEncodingChange()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;
	CComboBox*			pCombo;
	INT					iSel;
	CBitmapComboItem*	pData;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );
	VERIFY( pCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_ENCODING_COMBO ) ) );
	
	iSel = pCombo->GetCurSel();
	
	// Encoding
	pSummary->UnsetFlag(MSF_ENCODE0 | MSF_ENCODE1);

	if( iSel >= 0 ) 
	{
		VERIFY( pData = ( CBitmapComboItem* ) pCombo->GetItemDataPtr( iSel ) );
		pSummary->SetFlag( ( WORD )( pData->m_ID ) );
	}

	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::OnQuotedPrintable()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( GetCheck( ID_QUOTED_PRINTABLE ) )
	{
		pSummary->SetFlag( MSF_QUOTED_PRINTABLE );
	}
	else
	{
		pSummary->UnsetFlag( MSF_QUOTED_PRINTABLE );
	}

	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::OnTextAsDocument()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( GetCheck( ID_TEXT_AS_DOCUMENT ) )
	{
		pSummary->SetFlag( MSF_TEXT_AS_DOC );
	}
	else
	{
		pSummary->UnsetFlag( MSF_TEXT_AS_DOC );
	}

	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::OnWordWrap()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( GetCheck( ID_WORD_WRAP ) )
	{
		pSummary->SetFlag( MSF_WORD_WRAP );
	}
	else
	{
		pSummary->UnsetFlag( MSF_WORD_WRAP );
	}

	pDoc->SetModifiedFlag( TRUE );
}



void CCompMessageFrame::OnTabsInBody()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( GetCheck( ID_TABS_IN_BODY ) )
	{
		pSummary->SetFlag( MSF_TABS_IN_BODY );
	}
	else
	{
		pSummary->UnsetFlag( MSF_TABS_IN_BODY );
	}

	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::OnKeepCopies()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( GetCheck( ID_KEEP_COPIES ) )
	{
		pSummary->SetFlag( MSF_KEEP_COPIES );
	}
	else
	{
		pSummary->UnsetFlag( MSF_KEEP_COPIES );
	}

	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::OnReturnReceipt()
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( GetCheck( ID_RETURN_RECEIPT ) )
	{
		pSummary->SetFlag( MSF_READ_RECEIPT );

		if( GetIniShort( IDS_INI_RETURN_RECEIPT ) )
		{
			pSummary->SetFlag( MSF_RETURN_RECEIPT );
		}
	}
	else
	{
		pSummary->UnsetFlag( MSF_READ_RECEIPT | MSF_RETURN_RECEIPT );
	}

	pDoc->SetModifiedFlag( TRUE );
}


void CCompMessageFrame::Notify( 
QCCommandObject*	pCommand,
COMMAND_ACTION_TYPE	theAction,
void*				pData) 
{
	INT					iSel;
	INT					i;
	CBitmapComboItem*	pItem;
	BOOL				bSelectIt;
	CBitmapCombo*		pCombo;

	if( ( pCommand->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) == FALSE ) ||
		( ( theAction != CA_NEW ) && ( theAction != CA_RENAME ) && ( theAction != CA_DELETE ) ) )
	{
		return;
	}

	bSelectIt = FALSE;

	VERIFY( pCombo = ( CBitmapCombo* ) ( m_pToolBar->GetDlgItem( IDC_SIGNATURE_COMBO ) ) );

	if( ( theAction == CA_DELETE ) || ( theAction == CA_RENAME ) )
	{
		// find the entry
		for( i = 0; i < pCombo->GetCount(); i++ )
		{
			pItem = ( CBitmapComboItem* ) pCombo->GetItemDataPtr( i );
			
			if( stricmp( pItem->m_Text, ( ( QCSignatureCommand* ) pCommand )->GetName() ) == 0 )
			{
				break;
			}
		}

		if( i != pCombo->GetCount() )
		{			
			// get the current selection
			iSel = pCombo->GetCurSel();
			pCombo->DeleteString( i );
	
			if( i == iSel ) 
			{
				if( theAction == CA_RENAME ) 
				{
					bSelectIt = TRUE;
				}
				else
				{
					pCombo->SetCurSel( 0 );
				}
			}
		}
	}

	if( theAction == CA_NEW ) 
	{
		pItem = new CBitmapComboItem( 0, ( ( QCSignatureCommand* ) pCommand )->GetName(), 0);
	}
	else if( theAction == CA_RENAME )
	{
		pItem = new CBitmapComboItem( 0, ( LPCSTR ) pData, 0);
	}
	else
	{
		pItem = NULL;
	}

	if( pItem != NULL )
	{
		i = pCombo->Add( pItem );
		
		if( bSelectIt )
		{
			pCombo->SetCurSel( i );
		}
	}
}


BOOL CCompMessageFrame::OnDynamicCommand(
UINT uID )
{
	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	
	struct TOGGLE_DATA		theData;
	INT						i;
	CSummary*				pSummary;
	CCompMessageDoc*		pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	VERIFY( pSummary = pDoc->m_Sum );

	if( ! g_theCommandStack.Lookup( (WORD)uID, &pCommand, &theAction ) ||	
		( pCommand == NULL ) || 
		!theAction )
	{
		return FALSE;
	}

	if( theAction == CA_TRANSMISSION_PLUGIN )
	{
		for( i = m_theTranslatorMenu.GetMenuItemCount() - 1; i >= 0; i-- )
		{
			if( m_theTranslatorMenu.GetMenuItemID( i ) == uID )
			{
				break;
			}
		}

		ASSERT( i >= 0 );

		theData.m_ForceCheck = FALSE;
		theData.m_pMenu = &m_theTranslatorMenu;
		theData.m_uID = uID;
		theData.m_uFlags = MF_BYCOMMAND;
		theData.m_pszProperties = & ( m_thePropertyList[i] );
		pCommand->Execute( CA_TOGGLE, &theData );

		pSummary->SetTranslators(GetSelectedTranslators());
		pDoc->SetModifiedFlag( TRUE );
		return TRUE;
	}

	return FALSE;
}


void CCompMessageFrame::OnUpdateDynamicCommand(
CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( theAction == CA_TRANSMISSION_PLUGIN ) 
			{
				VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
				VERIFY( pSummary = pDoc->m_Sum );
				pCmdUI->Enable( pSummary->CantEdit() == FALSE );
				return;
			}
		}
	}

	pCmdUI->ContinueRouting();
}



void CCompMessageFrame::SelectTranslators(const char *hdr) 
{
	long				ModuleID = 0, TransID = 0;
	char*				dot = NULL;
	char*				comma = NULL;
	char*				space = NULL;
	QCPluginCommand*	pCommand;
	struct TOGGLE_DATA	theData;
	INT					i;
	UINT				uID;
	COMMAND_ACTION_TYPE	theAction;
	LONG				lMID;
	LONG				lTID;

	theData.m_pMenu = &m_theTranslatorMenu;
	theData.m_uFlags = MF_BYCOMMAND;
	theData.m_ForceCheck = TRUE;

	// The format of the translor line is <modID.transID Properties, modID2.transID Properties...
	while (hdr)
	{
		// get rid of the first char <xxxxx.yyyyy>
		if (hdr[0] == '<')
			hdr += 1;
		ModuleID = atoi(hdr);
		dot = strchr(hdr,'.');
		if (dot)
		{
			hdr = dot+1;
			TransID = atoi(hdr);
		}
		else
			return;

		comma = strchr(hdr,',');
		space = strchr(hdr,' ');
		
		pCommand = NULL;

		for( i = 0; (UINT) i < m_theTranslatorMenu.GetMenuItemCount(); i++ )
		{
			uID = m_theTranslatorMenu.GetMenuItemID( i );
		
			VERIFY( g_theCommandStack.Lookup( ( WORD ) uID, ( QCCommandObject** ) &pCommand, &theAction ) ); 			
			
			if (pCommand && pCommand->GetType() == CA_TRANSMISSION_PLUGIN && theAction == CA_TRANSMISSION_PLUGIN)
			{
				pCommand->GetTransmissionID( &lMID, &lTID);
							
				if( ( lMID == ModuleID ) && ( lTID == TransID ) )
				{
					break;
				}
			}

			pCommand = NULL;
		}

		if( pCommand )
		{
			UINT uCommandID = g_theCommandStack.FindCommandID( pCommand, pCommand->GetType() );
			
			theData.m_uID = uCommandID;
			pCommand->Execute( CA_TOGGLE, &theData );
			SetCheck( uCommandID, TRUE );
			
			if (comma && space)
			{
				space++;
				while (space < comma)
					m_thePropertyList[i] += *space++;
			}
			else if (space)
				m_thePropertyList[i] = ++space;


		}
		if (comma)
			hdr = comma+1;
		else 
			return;
	}
}


long CCompMessageFrame::OnAddAttachment(
WPARAM	wParam, 
LPARAM	lParam )
{
	char* szFilename;
	
	szFilename = ( char* ) lParam;

	GetHeaderView()->AddAttachment( szFilename );

	return 1;
}


long CCompMessageFrame::OnSetSignature(
WPARAM	wParam, 
LPARAM	lParam )
{
	CSummary*		pSummary;
	CString			szSig;
	INT				i;
	CBitmapCombo*	pCombo;

	szSig = "";
	pSummary = ( CSummary* ) lParam;

	VERIFY( pCombo = ( CBitmapCombo* ) m_pToolBar->GetDlgItem( IDC_SIGNATURE_COMBO ) );
	ASSERT_KINDOF(CBitmapCombo, pCombo);
	
	if ( pSummary->UseSignature() == MSF_USE_SIGNATURE )
	{
		// look for the standard signature file
		szSig = CRString( IDS_STANDARD_SIGNATURE );
	}
	else if ( pSummary->UseSignature() ==  MSF_ALT_SIGNATURE )
	{
		// user defined signature
		szSig = (const char *)pSummary->m_SigSelected;
	}
	else if( pSummary->UseSignature() )
	{
		// look for the alternate signature file
		szSig = CRString( IDS_ALTERNATE_SIGNATURE32  );
	}

	for( i = pCombo->GetCount() - 1; i >= 0; i -- )
	{
		CBitmapComboItem* pItem = pCombo->GetItemDataPtr( i );
		ASSERT( pItem );
		if ( pItem )
		{
			if( szSig.CompareNoCase( pItem->m_Text ) == 0 )
			{
				pCombo->SetCurSel( i );
				return 1;
			}
		}
	}

	pCombo->SetCurSel( 0 );

	return 1;
}


////////////////////////////////////////////////////////////////////////
// OnSetPriority [protected]
//
// Update the UI when somebody changes the priority setting in the
// Summary object.
////////////////////////////////////////////////////////////////////////
long CCompMessageFrame::OnSetPriority(WPARAM, LPARAM lParam)
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
// OnUserUpdateImmediateSend [protected]
//
// Update the UI when somebody changes the "send immediate" INI setting.
////////////////////////////////////////////////////////////////////////
LRESULT CCompMessageFrame::OnUserUpdateImmediateSend(WPARAM, LPARAM)
{            
	//
	// Just causing a repaint should do the trick.
	//
	m_pToolBar->InvalidateRect(NULL);
	return 0;
}


void CCompMessageFrame::OkIfEditable( CCmdUI* pCmdUI )
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	pSummary = pDoc->m_Sum;

	pCmdUI->Enable( pSummary && !pSummary->CantEdit() );
}


void CCompMessageFrame::OkIfNotStationery( CCmdUI* pCmdUI )
{
	CSummary*			pSummary;
	CCompMessageDoc*	pDoc;

	VERIFY( pDoc = ( CCompMessageDoc* ) GetActiveDocument() );
	pSummary = pDoc->m_Sum;

	if ( pDoc->IsStationery() )
		pCmdUI->Enable(FALSE);
	else 
		pCmdUI->Enable( pSummary && !pSummary->CantEdit() );
}

CString CCompMessageFrame::GetSelectedTranslators()
{
	CString				trans;
	CString				wholeStr;
	UINT				State = 0;
	
	UINT				uID;
	COMMAND_ACTION_TYPE	theAction;
	QCPluginCommand*	pCommand;
	LONG				lMID;
	LONG				lTID;

	trans.Empty();
	wholeStr.Empty();
	
	
	for (UINT i = 0; i < m_theTranslatorMenu.GetMenuItemCount(); i++)
	{
		State = m_theTranslatorMenu.GetMenuState( i, MF_BYPOSITION );
		
		if (State & MF_CHECKED)
		{
			uID = m_theTranslatorMenu.GetMenuItemID( i );
			VERIFY( g_theCommandStack.Lookup( ( WORD ) uID, ( QCCommandObject** ) &pCommand, &theAction ) ); 			
			
			if( ( pCommand->GetType() == CA_TRANSMISSION_PLUGIN ) && 
				( theAction == CA_TRANSMISSION_PLUGIN ) )
			{
				pCommand->GetTransmissionID( &lMID, &lTID);
											
				trans.Format("%04ld.%04ld", lMID,lTID);

				if( m_thePropertyList[i] != "" )
				{
					trans += " ";
					trans += m_thePropertyList[i];
				}


				if (wholeStr.IsEmpty())
					wholeStr = trans;
				else
				{
					wholeStr += ",";
					wholeStr += trans;
				}
			}
		}
	}
	return wholeStr;
}

QCChildToolBar* CCompMessageFrame::GetFormatToolbar() 
{
	if( m_pFormattingToolBar )
	{
		return m_pFormattingToolBar; 
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// OnToggleHeader [public]
//
////////////////////////////////////////////////////////////////////////
void CCompMessageFrame::OnToggleHeader() 
{
	//
	// Grab current header height.
	//
	int nHeaderCurrent, nHeaderMin;
	m_wndSplitter.GetRowInfo( 0, nHeaderCurrent, nHeaderMin );

	//
	// Save current header height so that we can restore it next time.
	//
	m_nHeaderHeight = nHeaderCurrent ? nHeaderCurrent : m_nHeaderHeight;

	//
	// New header height is either saved value or zero.
	//
	nHeaderCurrent = nHeaderCurrent ? 0 : m_nHeaderHeight;
	m_wndSplitter.SetRowInfo( 0, nHeaderCurrent, nHeaderMin );
	m_wndSplitter.RecalcLayout();
}


BOOL CCompMessageFrame::QuietPrint()
{
	QCProtocol*	pProtocol;
	CView*		pView;
	BOOL		bRet;

	bRet = FALSE;
	pView = ( CView* ) m_wndSplitter.GetPane( 1, 0 );
	
	pProtocol = QCProtocol::QueryProtocol( QCP_QUIET_PRINT, pView );

	if( pProtocol )
	{
		bRet = pProtocol->QuietPrint();
	}

	return bRet;
}

// --------------------------------------------------------------------------

//
// FIND TEXT
//

void CCompMessageFrame::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find (Ctrl-F)
{
	pCmdUI->Enable(TRUE);
}

void CCompMessageFrame::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if ((pFindMgr) && (pFindMgr->CanFindAgain()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

LONG CCompMessageFrame::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
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

	CCompMessageDoc *pDoc = (CCompMessageDoc *) GetActiveDocument();
	ASSERT(pDoc);
	
	if (!pDoc)
		return (EuFIND_ERROR);

	if (pDoc->FindNext(lpFR->lpstrFindWhat, (lpFR->Flags & FR_MATCHCASE), (lpFR->Flags & FR_WHOLEWORD)))
		return (EuFIND_OK);

	return (EuFIND_NOTFOUND);
}
