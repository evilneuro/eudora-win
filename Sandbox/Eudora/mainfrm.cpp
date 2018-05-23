// MAINFRM.CPP
//

#include "stdafx.h"
#include "afxpriv.h"	// Needed for WM_SETMESSAGESTRING

#include <ctype.h>
#include <afxcmn.h>
#include <afxrich.h>

#include "resource.h"
	   
#include "mainfrm.h"
#include "eudora.h"
#include "utils.h"
#include "cursor.h"
#include "fileutil.h"
#include "rs.h"
#include "guiutils.h"
#include "summary.h"
#include "doc.h"
#include "3dformv.h"
#include "tocdoc.h"
#include "tocview.h"
#include "pop.h"
#include "kclient.h"
#include "password.h"
#include "nickdoc.h"
#include "helpxdlg.h"
#include "settings.h"
#include "acapdlg.h"
#include "signatur.h"
#include "controls.h"
#include "filtersd.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "font.h"
#include "mdichild.h"

#include "SearchDoc.h"
#include "SearchView.h"

#include "filtersv.h"
#include "jjnet.h"
#include "newmbox.h"
#include "debug.h"
#include "msgutils.h"
#include "progress.h"
#include "address.h"
#include "helpcntx.h"
#include "ddeclien.h"
#include "hostlist.h"


#include "CompMessageFrame.h"


#include "eumapi.h"

#include "saveas.h"
#include "persona.h"
#include "AboutEMS.h"

#include "urledit.h"

#include "ems-wglu.h"
#include "trnslate.h"
#include "colorlis.h"
#include "mtransop.h"

#include "ClipboardMgr.h"
#include "WazooBar.h"
#include "WazooWnd.h"
#include "FilterReportView.h"		// FORNOW A TOTAL HACK
#include "MboxWazooWnd.h"
#include "SignatureWazooWnd.h"
#include "StationeryWazooWnd.h"
#include "FilterReportWazooWnd.h"
#include "FiltersWazooWnd.h"
#include "FileBrowseWazooWnd.h"
#include "NicknamesWazooWnd.h"
#include "PersonalityWazooWnd.h"
#include "TaskStatusView.h"
#include "TaskStatusWazooWnd.h"

#include "TaskErrorView.h"
#include "TaskErrorWazooWnd.h"

#include "WebBrowserView.h"
#include "WebBrowserWazooWnd.h"

#include "DirectoryServicesWazooWndNewIface.h"
#include "NewSignatureDialog.h"
#include "PaigeEdtView.h"

#include "QCToolBarManager.h"
#include "QCCustomToolBar.h"

#include "QCLabelDirector.h"
#include "QCSharewareManager.h"

#include "QCCommandActions.h"
#include "QCCommandObject.h"
#include "QCCommandStack.h"
#include "QCAutomationDirector.h"
#include "QCMailboxDirector.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"
#include "QCPersonalityCommand.h"
#include "QCPersonalityDirector.h"
#include "Automation.h"
#include "TraceFile.h"
#include "WizardPropSheet.h"
#include "SafetyPal.h"
#include "QCGraphics.h"
#include "QCTaskManager.h"
#include "AutoCompleteSearcher.h"
#include "ImportMailDlog.h"
#include "QCFindMgr.h"

HMENU g_EudoraProHMENU;

#include "DelSigDg.h"
#include "spell.h"

#ifdef IMAP4
#include "QCImapMailboxCommand.h"
#include "ImapMailMgr.h"
#include "imapactl.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
#ifndef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#endif
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack		g_theCommandStack;
extern QCAutomationDirector g_theAutomationDirector;
extern QCMailboxDirector	g_theMailboxDirector;
extern QCPluginDirector		g_thePluginDirector;
extern QCRecipientDirector	g_theRecipientDirector;
extern QCSignatureDirector	g_theSignatureDirector;
extern QCStationeryDirector	g_theStationeryDirector;
extern QCPersonalityDirector g_thePersonalityDirector;

// WINDOWSX.H has a SubclassWindow macro which collides
#ifdef SubclassWindow
#undef SubclassWindow
#endif // SubclassWindow

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

#ifdef COMMERCIAL	

// Submenu positions -- these are needed because a submenu does not have an ID

// Edit Menu, positions of submenus

//Edit->Insert Recipients SubMenu
#define POS_EDIT_INSERT_SUBMENU        (11)
#define POS_EDIT_INSERTRECIPIENT_MENU  (16)

//Under Message Menu, positions of submenus
#define POS_NEW_MESSAGE_TO    7
#define POS_FORWARD_TO        8
#define POS_REDIRECT_TO       9
#define POS_NEW_MESSAGE_WITH  11
#define POS_REPLY_WITH		  12
#define POS_REPLY_TO_ALL_WITH 13
#define POS_ATTACH			  16	
#define POS_CHANGE			  18

#else
// For light version, the insert -> line menu is removed and the offset for the next menu is off by two (14)
#define POS_EDIT_INSERTRECIPIENT_MENU 14

//Under Message Menu, positions of submenus
#define POS_NEW_MESSAGE_TO    7
#define POS_FORWARD_TO        8
#define POS_REDIRECT_TO       9

#define POS_ATTACH			  12
#define POS_CHANGE			  14

#endif // COMMERCIAL


BEGIN_BUTTON_MAP(btnMap)
END_BUTTON_MAP()


// overwrite mode for editing is global state
extern bool g_bOverwriteMode;

// STATIC (IMAP)
UINT IMAPFindStartingPos (QCImapMailboxCommand* pImapCommand);
void IMAPAppendSeparator (QCImapMailboxCommand* pImapCommand, CMenu* pMenu);

///////////////////////////////////////////////////////////////////////
// QCWorkbookClient
//
// Derivation of SECWorkbookClient (MDI Client Window) to hook messages 
// destined for the MDI client window.
///////////////////////////////////////////////////////////////////////


#include "qtwrapper.h"

class QCWorkbookClient : public SECWorkbookClient
{ 
    DECLARE_DYNCREATE(QCWorkbookClient);

    // awesome-groovy mdi client background
    HBITMAP     m_hBmpBackground;
    CRect       m_rcBmpBackground;
    COLORREF    m_clrBackground;

public:
    void Init();

public:
    QCWorkbookClient(){m_hBmpBackground = NULL;}
    ~QCWorkbookClient(){};

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Generated message map functions
protected:

    BOOL OnEraseBkgnd( CDC* pDC );

    //{{AFX_MSG(QCWorkbookClient)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP();
};


IMPLEMENT_DYNCREATE(QCWorkbookClient, SECWorkbookClient)

BEGIN_MESSAGE_MAP(QCWorkbookClient, SECWorkbookClient)
        //{{AFX_MSG_MAP(QCWorkbookClient)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// QCWorkbookClient background hacks
BOOL QCWorkbookClient::OnEraseBkgnd( CDC* pDC ) 
{
    CBrush theBrush;
    theBrush.CreateSolidBrush( m_clrBackground );
    CBrush* pOldBrush = (CBrush*) pDC->SelectObject( &theBrush );

    CRect rect;
    pDC->GetClipBox( &rect );
    pDC->PatBlt( rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY );
    pDC->SelectObject( pOldBrush );

    if ( m_hBmpBackground ) {
	CRect rc;
	GetClientRect( &rc );
	int x = (m_rcBmpBackground.Width() - rc.Width()) / 2;
	int y = (m_rcBmpBackground.Height() - rc.Height()) / 2;

	HDC hdcOS = ::CreateCompatibleDC( pDC->GetSafeHdc() );
	HBITMAP hbmOld = (HBITMAP) ::SelectObject( hdcOS, m_hBmpBackground );
	::SetViewportOrgEx( hdcOS, x, y, NULL );
	::BitBlt( pDC->GetSafeHdc(),
		  rect.left, rect.top,
		  rect.Width(), rect.Height(),
		  hdcOS,
		  rect.left, rect.top,
		  SRCCOPY );

	::SelectObject( hdcOS, hbmOld );
	::DeleteDC( hdcOS );
    }

    return TRUE;
}


void QCWorkbookClient::Init()
{
    char bgImage[MAX_PATH] = "";
    ::GetPrivateProfileString( "Decorations", "BackgroundImage", "", bgImage,
			       sizeof(bgImage), INIPath );

    char bgColor[8] = "";
    ::GetPrivateProfileString( "Decorations", "BackgroundColor", "", bgColor,
			       sizeof(bgColor), INIPath );

    // bgColor is an rgb color "hex string" [e.g. "FFFFFF" is white]. some
    // day, i'll put in support for english names [e.g. "white"].

    if (*bgColor)
	{
		VERIFY(ConvertHexToColor(bgColor, m_clrBackground));
    }
    else
		m_clrBackground = ::GetSysColor( COLOR_APPWORKSPACE );

    if ( *bgImage )
	{
		if ( QTW_Startup() )
			QTW_DibFromQtImage( bgImage, &m_hBmpBackground, &m_rcBmpBackground );
    }
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

//UINT umsgRefreshTaskStatusPane = ::RegisterWindowMessage("umsgRefreshTaskStatusPane");
UINT umsgShowProgress = RegisterWindowMessage( "umsgShowProgress" );


IMPLEMENT_DYNAMIC(CMainFrame, QCWorkbook)

BEGIN_MESSAGE_MAP(CMainFrame, QCWorkbook)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_QUERYDRAGICON()
	ON_WM_QUERYOPEN()
	
	ON_COMMAND(ID_CTRL_F, OnCtrlF)
	ON_COMMAND(ID_EDIT_FIND_FINDMSG, OnEditFindFindMsg)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDMSG, OnUpdateEditFindFindMsg)
	
	ON_COMMAND(ID_SHIFT_CTRL_F, OnShiftCtrlF)
	ON_COMMAND(ID_EDIT_FIND_FINDTEXT, OnEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	
	ON_COMMAND(ID_EDIT_FIND_FINDTEXTAGAIN, OnEditFindFindTextAgain)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_COMMAND(ID_FILE_CHECKMAIL, OnFileCheckMail)
	ON_COMMAND(ID_MESSAGE_NEWMESSAGE, OnMessageNewMessage)
	ON_COMMAND(ID_MESSAGE_REPLY_CTRL_R, OnMessageReplyCtrlR)
	ON_COMMAND(IDM_PURCHASE_INFO, OnPurchaseInfo)
	ON_COMMAND(ID_SPECIAL_EMPTYTRASH, OnSpecialEmptyTrash)
	ON_COMMAND(ID_SPECIAL_COMPACTMAILBOXES, OnSpecialCompactMailboxes)
	ON_COMMAND(ID_SPECIAL_SETTINGS, OnSpecialSettings)
	ON_COMMAND(ID_SPECIAL_NEWACCOUNT, OnSpecialNewAccount)
	ON_COMMAND(ID_SPECIAL_CHANGEPASSWORD, OnSpecialChangePassword)
	ON_COMMAND(ID_SPECIAL_FORGETPASSWORD, OnSpecialForgetPassword)
	ON_COMMAND(ID_WINDOW_FILTERS, OnWindowFilters)
	ON_COMMAND(ID_WINDOW_MAILBOXES, OnWindowMailboxes)
	ON_COMMAND(ID_WINDOW_NICKNAMES, OnWindowNicknames)
	ON_COMMAND(ID_HELP_SEARCHFORHELPON, OnHelpSearchForHelpOn)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_FORGETPASSWORD, OnUpdateSpecialForgetPassword)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATE()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_COMMAND(ID_FILE_SENDQUEUEDMESSAGES, OnSendQueuedMessages)
	ON_UPDATE_COMMAND_UI(ID_FILE_SENDQUEUEDMESSAGES, OnUpdateSendQueuedMessages)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY, OnUpdateMessageReply)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_REPLY_ALL, OnUpdateMessageReplyAll)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SENDIMMEDIATELY, OnUpdateMessageSendImmediately)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNREAD, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_1, OnUpdateMessageLabel)
	ON_WM_QUERYENDSESSION()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_WINDOW_SENDTOBACK, OnSendToBack)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_SENDTOBACK, OnUpdateSendToBack)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateNeedSelEdit)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateNeedClip)
	ON_COMMAND(ID_EDIT_PASTEASQUOTATION, OnEditPasteAsQuotation)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_WRAPSELECTION, OnEditWrapSelection)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FINISHNICKNAME, OnUpdateEditFinishNickname)
	ON_COMMAND(ID_EDIT_FINISHNICKNAME, OnEditFinishNickname)
	ON_COMMAND(ID_SPECIAL_ADDASRECIPIENT, OnSpecialAddAsRecipient)
	ON_COMMAND(ID_PROCESS_ALL, OnProcessAllofThem)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_ABOUT_EMS_PLUGINS, OnAboutEmsPlugins)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_ATTACHFILE, OnUpdateAttach)
	ON_COMMAND(ID_MESSAGE_ATTACHFILE, OnAttachFile)
	ON_WM_MENUSELECT()
	ON_WM_NCHITTEST()
	ON_WM_ENABLE()
	ON_WM_NCACTIVATE()
	ON_WM_ENTERIDLE()
	ON_COMMAND(ID_WINDOW_PH, OnViewLookup)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_READ, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REPLIED, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_FORWARDED, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REDIRECTED, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNSENDABLE, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENDABLE, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_QUEUED, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TIME_QUEUED, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_SENT, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNSENT, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_RECOVERED, OnDisableMessageStatus)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_2, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_3, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_4, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_5, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_6, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_LABEL_7, OnUpdateMessageLabel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEASQUOTATION, OnUpdateNeedClip)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNeedSelEdit)
	ON_UPDATE_COMMAND_UI(ID_EDIT_WRAPSELECTION, OnUpdateNeedSelEdit)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_ADDASRECIPIENT, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	//}}AFX_MSG_MAP
 	
	// Global help commands
	ON_COMMAND(ID_HELP_TOPICS, OnHelpTopics) 
	ON_COMMAND(ID_HELP_TECHNICAL_SUPPORT, OnTechSupport)
	ON_COMMAND(ID_HELP, QCWorkbook::OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_COMMAND(IDM_VIEW_FILTER_REPORT, OnViewFilterReport )
	ON_COMMAND(IDM_VIEW_STATIONERY, OnViewStationery )
	ON_COMMAND(IDM_VIEW_FILE_BROWSER, OnViewFileBrowser )
	ON_COMMAND( ID_TOOLS_VIEW_SIGNATURES, OnViewSignatures )
	ON_COMMAND(IDM_VIEW_PERSONALITIES, OnViewPersonalities )
	ON_COMMAND(IDM_VIEW_TASK_STATUS, OnViewTaskStatus )
	ON_COMMAND(IDM_VIEW_TASK_ERROR, OnViewTaskError )
	ON_COMMAND(IDM_VIEW_WEB_BROWSER, OnViewWebBrowser )
//FORNOW	ON_COMMAND(ID_CTRLBAR_SHOW_MAILBOXES, OnShowMboxBar)					// WIN32 only
	ON_COMMAND(ID_CTRLBAR_SHOW_TOOLBAR, OnShowToolBar)						// WIN32 only
	ON_COMMAND(ID_CTRLBAR_SHOW_STATUSBAR, OnShowStatusBar)					// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CTRLBAR_SHOW_MAILBOXES, OnUpdateControlBarMenu)	// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CTRLBAR_SHOW_TOOLBAR, OnUpdateControlBarMenu)	// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CTRLBAR_SHOW_STATUSBAR, OnUpdateControlBarMenu)	// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_OVR, OnUpdateOvrIndicator)			// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CUSTOMIZE_LINK, OnUpdateControlBarMenu)			// WIN32 only
	ON_COMMAND(ID_CUSTOMIZE_LINK, OnCustomizeLink)							// WIN32 only
	ON_COMMAND(ID_DEFAULT_HELP, OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, QCWorkbook::OnContextHelp) 

	// Owner draw stuff for label menus
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	
	// Commands not handled by ClassWizard
	ON_COMMAND(ID_CLOSE_ALL, OnCloseAll)
	ON_COMMAND(ID_SAVE_ALL, OnSaveAll)
	ON_COMMAND(ID_FAKE_APP_EXIT, OnFakeAppExit)
	ON_UPDATE_COMMAND_UI(ID_FAKE_APP_EXIT, OnUpdateFakeAppExit)
	ON_WM_SYSCOMMAND()

#ifdef COMMERCIAL
	ON_COMMAND(ID_TOOL_SIGNATURE_NEW, OnNewSignature)
	ON_COMMAND(ID_TOOL_SIGNATURE_DELETE, OnDeleteSignature)
#endif // COMMERCIAL

	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	
	ON_COMMAND(ID_HELP_MOREHELP, OnMoreHelp)
	ON_COMMAND_EX(ID_HELP_QUESTNEWS, OnMailingList)
	ON_COMMAND_EX(ID_HELP_EUDORAFORUM, OnMailingList)

	ON_MESSAGE(WM_TRAY_CALLBACK,OnTrayCallback)
	ON_MESSAGE(WM_USER_CHECKMAIL, OnUserCheckMail)
	ON_MESSAGE(WM_USER_REGISTER_MAILTO, OnUserRegisterMailto)
#ifdef EXPIRING
	ON_MESSAGE(WM_USER_EVAL_EXPIRED, OnUserEvalExpired)
#endif // EXPIRING
	ON_MESSAGE(WM_USER_AUTOMATION_MSG, OnUserAutomationMsg)
	ON_REGISTERED_MESSAGE(umsgShowProgress, OnShowProgress)
#ifdef COMMERCIAL
	ON_COMMAND(ID_EDIT_CHECKSPELLING, CheckSpelling)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CHECKSPELLING, OnEditable)
#endif // COMMERCIAL
	ON_MESSAGE( WM_SETMESSAGESTRING, OnSetMessageString )
//	ON_REGISTERED_MESSAGE(umsgRefreshTaskStatusPane, OnRefreshTaskStatusPane)

	ON_COMMAND( ID_MAILBOX_IN, OnMailboxIn )
	ON_COMMAND( ID_MAILBOX_OUT, OnMailboxOut )
	ON_COMMAND( ID_MAILBOX_TRASH, OnMailboxTrash )

	ON_COMMAND( IDM_NEW_STATIONERY, OnNewStationery )
	ON_COMMAND( IDM_DELETE_STATIONERY, OnDeleteStationery )
	ON_COMMAND( IDM_IMPORT_MAIL, OnImportMail)

	ON_COMMAND( ID_NEW_MAILBOX_IN_ROOT, OnNewMailboxInRoot )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )

// IMAP stuff: - After startup, resync opened mailboxes.
	ON_COMMAND(ID_MBOX_IMAP_RESYNC_OPENED, OnImapResyncOpened)

#ifdef _DEBUG
	// Debug-only menu items
	ON_COMMAND(ID_SHAREWAREDEBUG_CHANGESTATE_LIGHT, OnSharewareChangeStateLight) // LIGHT
	ON_UPDATE_COMMAND_UI(ID_SHAREWAREDEBUG_CHANGESTATE_LIGHT, OnUpdateSharewareChangeStateLight)

	ON_COMMAND(ID_SHAREWAREDEBUG_CHANGESTATE_PRO, OnSharewareChangeStatePro) // PRO
	ON_UPDATE_COMMAND_UI(ID_SHAREWAREDEBUG_CHANGESTATE_PRO, OnUpdateSharewareChangeStatePro)

	ON_COMMAND(ID_SHAREWAREDEBUG_CHANGESTATE_ADWARE, OnSharewareChangeStateAdware) // ADWARE
	ON_UPDATE_COMMAND_UI(ID_SHAREWAREDEBUG_CHANGESTATE_ADWARE, OnUpdateSharewareChangeStateAdware)
#endif // _DEBUG
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

// toolbar buttons - IDs are command buttons
static const UINT BASED_CODE buttons[] =
{
	// same order as in the bitmap 'toolbar.bmp'
	ID_MESSAGE_DELETE,
		ID_SEPARATOR,
	ID_MAILBOX_IN,
	ID_MAILBOX_OUT,
		ID_SEPARATOR,
	ID_FILE_CHECKMAIL,
		ID_SEPARATOR,
	ID_MESSAGE_NEWMESSAGE,
	ID_MESSAGE_REPLY,
	ID_MESSAGE_REPLY_ALL,
		ID_SEPARATOR,
	ID_MESSAGE_FORWARD,
	ID_MESSAGE_REDIRECT,
		ID_SEPARATOR,
	ID_PREVIOUS_MESSAGE,
	ID_NEXT_MESSAGE,
		ID_SEPARATOR,
	ID_MESSAGE_ATTACHFILE,
#ifdef COMMERCIAL	
	ID_EDIT_CHECKSPELLING,
#endif // COMMERCIAL
		ID_SEPARATOR,
	ID_WINDOW_NICKNAMES,
		ID_SEPARATOR,
	ID_FILE_PRINT,
		ID_SEPARATOR,
	 ID_CONTEXT_HELP
};


extern BOOL g_bPopupOn; 


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

/* static */ CMainFrame *CMainFrame::m_pTheMainFrame = NULL;

/* static */ CMainFrame *CMainFrame::QCGetMainFrame()
{
	ASSERT(m_pTheMainFrame);
	return (m_pTheMainFrame);
}


CMainFrame::CMainFrame() :
	m_idAutoActivateTimer(0)
{
	ASSERT(!m_pTheMainFrame);
	m_pTheMainFrame = this;

	// Setting this to FALSE means that menu items without handlers do not get disabled.
	// This is used to control enabling/disabling of menu items in Mailboxes, Transfer,
	// and recpient menus.
//	m_bAutoMenuEnable = FALSE;
	m_hIcon = NULL;
	m_IconID = 0;
	m_iNewMsgs =0;
	m_bStartedIconic = FALSE;
	m_pToolBar = NULL;
	m_pActiveMboxTreeCtrl		= NULL;	// FORNOW, backwards compatibility hack
	m_pActiveFilterReportView	= NULL;	// FORNOW, backwards compatibility hack
	m_pActiveLookupView			= NULL;	// FORNOW, backwards compatibility hack
	m_pActivePersonalityView	= NULL;	// ditto
	m_pActiveTaskStatusView		= NULL;	// ditto
	m_pActiveWebBrowserView		= NULL;	// ditto
	m_pDirectoryServicesWazooWndNew	=	NULL ;  // ditto

	m_pControlBarManager = new QCToolBarManager( this );
}

CMainFrame::~CMainFrame()
{
	delete m_pControlBarManager;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)				   
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	WNDCLASS wndcls;

	if (::GetClassInfo(hInst, EudoraMainWindowClass, &wndcls))
	{
		if ( cs.hMenu != NULL )
		{
			// add the color text menu
			CMenu		menuMain;
			CColorMenu	menuColor;
			CString		szMenuLabel;

			// attach to the main menu
			menuMain.Attach( cs.hMenu );

			// get the menu string
			menuMain.GetMenuString( ID_EDIT_TEXT_COLOR, szMenuLabel, MF_BYCOMMAND );

			// modify the Edit/Text/Color menu item
			menuMain.ModifyMenu(	ID_EDIT_TEXT_COLOR,
									MF_BYCOMMAND | MF_POPUP , 
									(UINT) menuColor.GetSafeHmenu(), 
									szMenuLabel );

#ifdef _DEBUG	
{
	// Add a Debug menu
	CMenu DebugPopup;
	DebugPopup.CreatePopupMenu();
	DebugPopup.AppendMenu(MF_STRING, ID_DMM_DUMPLOG, "&DumpLog");
	DebugPopup.AppendMenu(MF_STRING, ID_DMM_HIGHWATERMARK, "&HighWatermark");
	DebugPopup.AppendMenu(MF_STRING, ID_DMM_CURRENT_ALLOC, "&CurrentAlloc");

	DebugPopup.AppendMenu(MF_SEPARATOR);

	DebugPopup.AppendMenu(MF_STRING, ID_SHAREWAREDEBUG_CHANGESTATE_LIGHT, "Become &Light");
	DebugPopup.AppendMenu(MF_STRING, ID_SHAREWAREDEBUG_CHANGESTATE_PRO, "Become &Pro");
	DebugPopup.AppendMenu(MF_STRING, ID_SHAREWAREDEBUG_CHANGESTATE_ADWARE, "Become &Adware");

	menuMain.AppendMenu(MF_POPUP, (UINT)(DebugPopup.Detach()), "&Debug");
}
#endif // _DEBUG

			
			// detach both menus from the objects
			menuColor.Detach();
			menuMain.Detach();
		}									
	
	}

	//
	// Set the initial main frame window size and position.  This must
	// be done before the toolbar/wazoo creation stuff happens,
	// otherwise the initial wazoo window sizes won't be initialized
	// correctly.
	//
	{
		CRect rectMainFrame = GetMainWindowPosFromIni();
		cs.x = rectMainFrame.left;
		cs.y = rectMainFrame.top;
		cs.cx = rectMainFrame.Width();
		cs.cy = rectMainFrame.Height();
	}
	
	BOOL bRes = QCWorkbook::PreCreateWindow(cs);
	 
	// see if the class already exists
	if (!::GetClassInfo(hInst, EudoraMainWindowClass, &wndcls))
	{
		// get default stuff
		::GetClassInfo(hInst, cs.lpszClass, &wndcls);
		//FORNOWwndcls.style &= ~(CS_HREDRAW|CS_VREDRAW);
		// register a new class
		wndcls.lpszClassName = EudoraMainWindowClass;
		wndcls.hIcon = QCLoadIcon(MAKEINTRESOURCE(IDR_MAINFRAME)); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
		ASSERT(wndcls.hIcon != NULL);
		if (!::RegisterClass(&wndcls))
			AfxThrowResourceException();
	}
	cs.lpszClass = EudoraMainWindowClass;
	return bRes;
}

#ifndef COMMERCIAL
// Since Light has a non-standard "About Pro" menu after Help, the standard MFC method must
// be overridden to setup the Window menu
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext*)
{
	CMenu* pMenu = NULL;

	if (m_hMenuDefault == NULL)
	{
		// default implementation for MFC V1 backward compatibility
		pMenu = GetMenu();
		ASSERT(pMenu != NULL);
		// This is attempting to guess which sub-menu is the Window menu.
		// The Windows user interface guidelines say that the right-most
		// menu on the menu bar should be Help and Window should be one
		// to the left of that.
		int iMenu = pMenu->GetMenuItemCount() - 3;	// was 2 in standard MFC - KCM

		// If this assertion fails, your menu bar does not follow the guidelines
		// so you will have to override this function and call CreateClient
		// appropriately or use the MFC V2 MDI functionality.
		ASSERT(iMenu >= 0);
		pMenu = pMenu->GetSubMenu(iMenu);
		ASSERT(pMenu != NULL);
	}

	return CreateClient(lpcs, pMenu);
}

#endif //COMMERCIAL


////////////////////////////////////////////////////////////////////////
// CreateClient [public, virtual]
//
// Override of MFC and SEC base class implementations which installs
// our MDI client window in favor of the SEC MDI client window.
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu)
{
    BOOL bResult = QCWorkbook::CreateClient(lpCreateStruct, pWindowMenu);

    ASSERT_VALID(m_pWBClient);
    ASSERT(m_pWBClient->GetSafeHwnd() == m_hWndMDIClient);
    m_pWBClient->Detach();
    delete m_pWBClient;
    m_pWBClient = new QCWorkbookClient();
    ASSERT_VALID(m_pWBClient);
    m_pWBClient->SubclassWindow(m_hWndMDIClient);

    ((QCWorkbookClient*)m_pWBClient)->Init();

    return bResult;
}


////////////////////////////////////////////////////////////////////////
// WriteToolBarMarkerToIni [protected]
//
// For commercial builds, forces the IDS_INI_COMMERCIAL_32_VERSION to
// get written with the latest version number, but flushes the
// IDS_INI_LIGHT_32_VERSION from [Settings] section of the INI file.
//
// For light builds, forces the IDS_INI_LIGHT_32_VERSION to get
// written with the latest version number, but flushes the
// IDS_INI_COMMERCIAL_32_VERSION from [Settings] section of the INI
// file.
////////////////////////////////////////////////////////////////////////
void CMainFrame::WriteToolBarMarkerToIni() const
{
	int ID_TO_FLUSH = 0;

	// Shareware: We should eventually remove this, but for now it stays
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE
		ID_TO_FLUSH = IDS_INI_LIGHT_32_VERSION;
		SetIniLong(IDS_INI_COMMERCIAL_32_VERSION, COMMERCIAL_32_VERSION);
	}
	else
	{
		// REDUCED-FEATURE
		ID_TO_FLUSH = IDS_INI_COMMERCIAL_32_VERSION;
		SetIniLong(IDS_INI_LIGHT_32_VERSION, LIGHT_32_VERSION);
	}

	ASSERT(ID_TO_FLUSH != 0);

	//
	// Flush the conflicting INI setting, if any, from the INI file.
	//
	PurgeIniSetting(ID_TO_FLUSH);
}

////////////////////////////////////////////////////////////////////////
// FlushIncompatibleToolBarState [protected]
//
// Check to see if the current EUDORA.INI that we're about to read has
// compatible ToolBar state info.  If not, then flush the old
// [ToolBar-xxx] sections from the INI file to avoid crashes in MFC's
// LoadBarState() method.  There is a known incompatibility between
// "old" 32-bit versions which don't support the dockable mailbox 
// "toolbar" and even betweeen "contemporary" Pro 32 and Light 32 
// versions which somehow have incompatible toolbar state info.
////////////////////////////////////////////////////////////////////////
void CMainFrame::FlushIncompatibleToolBarState()
{
	//
	// Load up the Commercial32Version and Light32Version settings
	// from the [Settings] section of the INI file.  If the version
	// strings are not present in the INI file, the default values are
	// supposed to be set to 0 (zero) in the resource files to
	// indicate that the setting is not valid for that version of
	// Eudora.
	//
	long commercial32_version = GetIniLong(IDS_INI_COMMERCIAL_32_VERSION);
	long light32_version = GetIniLong(IDS_INI_LIGHT_32_VERSION);

	BOOL contains_compatible_bar_state = FALSE;

	// Shareware: We should eventually remove this, but for now it stays
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE
		if (COMMERCIAL_32_VERSION == commercial32_version &&
			0 == light32_version)
		{
			contains_compatible_bar_state = TRUE;
		}
	}
	else
	{
		// REDUCED-FEATURE
		if (0 == commercial32_version &&
			LIGHT_32_VERSION == light32_version)
		{
			contains_compatible_bar_state = TRUE;
		}
	}

	//
	// Okay, we have reached the decision point....  To Flush or Not
	// To Flush.  That is The Question(tm).
	//
	if (contains_compatible_bar_state)
	{
		//
		// We're running a "contemporary" INI file, so it's normal and
		// expected to have toolbar state in the INI file, so don't
		// flush it.
		//
		return;
	}
	else
	{
		//
		// Otherwise, if the toolbar state is missing, then let's
		// assume that it's an "old" INI file, so flush it, just in case.
		//
	}

	//
	// Read up the number of [ToolBar-BarXX] sections from the
	// [ToolBar-Summary] key.
	//
	const CString TOOLBAR_SUMMARY("ToolBar-Summary");
	const CString BARS("Bars");
	const CString TOOLBAR_SECTION_FMT("ToolBar-Bar%d");
	char buf[32];

	if (::GetPrivateProfileString(TOOLBAR_SUMMARY, BARS, "",
				      buf, sizeof(buf), INIPath)) {
	    int num_sections = atoi(buf);
	    if (num_sections <= 0 || num_sections > 20) {
		ASSERT(0);			// let's be reasonable
	    }
	    else {
		//
		// Flush the [ToolBar-Summary] section.
		//
		::WritePrivateProfileString(TOOLBAR_SUMMARY, NULL, NULL,
					    INIPath);
		//
		// Flush the [ToolBar-BarXX] sections.
		//
		for (int i = 0; i < num_sections; i++) {
		    wsprintf(buf, TOOLBAR_SECTION_FMT, i);
		    ::WritePrivateProfileString(buf, NULL, NULL, INIPath);
		}
	    }
	}

	//
	// Flush certain mailbox window settings from the INI file.
	//
	PurgeIniSetting(IDS_INI_SHOW_MBOX_BAR);
	PurgeIniSetting(IDS_INI_DOCK_MBOX_BAR);

	//
	// Flush the [WazooBar] section.
	//
	::WritePrivateProfileString(CRString(IDS_INI_WAZOOBAR_SECTION_NAME), NULL, NULL, INIPath);

	//
	// Flush nickname window settings from the INI file.  Really, this
	// has nothing to do with toolbar settings, but its a convenient place
	// to handle INI upgrades from older versions of Eudora as related
	// to nickname window settings.
	//
	PurgeIniSetting(IDS_INI_NICKNAMES_WINDOW_SPLITTER);

	// HACK ALERT! -- sorry, it's not the place for this, I know.  
	// But it does work and I'm in a hurry.

	UpdateFontSettings();
}


//////////////////////////////////////////////////////////////////////////////
//	SplitPOPACCOUNT() : In Eudora.ini, if POP_SERVER or LOGIN_NAME fields
//  are not present, then attempt to extract these from the POP_ACCOUNT field
//////////////////////////////////////////////////////////////////////////////
void CMainFrame::SplitPOPACCOUNT()
{
	// do this for all personalities
	
	char szPOPUserName[ MAX_PATH];
	char szServer [MAX_PATH ];
	char szUser   [ MAX_PATH];

	for (LPSTR pszList = g_Personalities.List();
		 pszList && *pszList;
		 pszList += strlen(pszList) + 1)
	{

			 g_Personalities.GetProfileString( pszList, CPersonality::GetIniKeyName(IDS_INI_PERSONA_POP_ACCOUNT), 
				 CPersonality::GetIniDefaultValue(IDS_INI_PERSONA_POP_ACCOUNT), 
								szPOPUserName, sizeof(szPOPUserName));
			 
			 g_Personalities.GetProfileString( pszList, CPersonality::GetIniKeyName(IDS_INI_POP_SERVER), 
				 CPersonality::GetIniDefaultValue(IDS_INI_POP_SERVER), 
								szServer, sizeof(szServer));

			 g_Personalities.GetProfileString( pszList, CPersonality::GetIniKeyName(IDS_INI_LOGIN_NAME), 
				 CPersonality::GetIniDefaultValue(IDS_INI_LOGIN_NAME), 
								szUser, sizeof(szUser));

			 if( szUser[0] == '\0' ||  szServer[0] == '\0') {

				 // need to split the POP account
					char* pszAtSign = strchr(szPOPUserName, '@');
					if( pszAtSign) {
						char szTemp[ 128];
						strcpy( szTemp, pszAtSign + 1) ;
						*pszAtSign = 0 ;
						CRString key( IDS_INI_LOGIN_NAME );
						g_Personalities.WriteProfileString( pszList, key, szPOPUserName) ;
						key	=	IDS_INI_POP_SERVER ;
						g_Personalities.WriteProfileString( pszList, key, szTemp) ;
					}
			 }
	}

}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	m_wndStatusBar.PostMessage(WM_ACTIVATE, MAKEWPARAM(nState, bMinimized), (LPARAM) pWndOther);
	QCWorkbook::OnActivate(nState, pWndOther, bMinimized);
}

BOOL CMainFrame::OnQueryNewPalette()
{
	//QCWorkbook::OnQueryNewPalette();
	SendMessageToDescendants(WM_USER_QUERY_PALETTE);
	return TRUE;
}

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
	// do not call:	QCWorkbook::OnPaletteChanged(pFocusWnd);

	// Check to see if the palette change message is 
	// applicable to Eudora's child windows
	bool bDoPaletteChange = false;
	if (pFocusWnd == AfxGetMainWnd() || AfxGetMainWnd()->IsChild(pFocusWnd))
	{
		// The window that changed the palette belongs to Eudora
		if (!IsEudoraPaletteCurrent())
		{
			// The palette currently used in Eudora does not have
			// exactly the same palette entries as the system
			gbUseMirrorPalette = true;
			SetMirrorPalette();
			bDoPaletteChange = true;
		}
	}
	else
	{
		// The window that changed the palette is external to Eudora
		bDoPaletteChange = true;
	}
	if (bDoPaletteChange)
	{
		SendMessageToDescendants(WM_PALETTECHANGED, (WPARAM)pFocusWnd->GetSafeHwnd());
	}
	// Windows have had a chance to update
	// clear this flag
	gbMirrorPaletteChanged = false;
}

void CMainFrame::OnSysColorChange()
{
	// This is the first SysColorChange in Eudora to get called.
	// Then all the toolbars etc will update afterwards
	// The safety palette is dependant upon four system colors so
	// it will have to change as well
	SetSafetyPalette(IDB_SAFETY_PALETTE);

	QCWorkbook::OnSysColorChange();

	//
	// Force cached bitmaps for TOC column buttons to be
	// re-initialized with a new button face color.
	//
	CTocHeaderButton::PurgeButtonImageCache();
	CTocHeaderButton::LoadButtonImageCache();

//	SendMessageToDescendants(WM_SYSCOLORCHANGE, 0, 0L, TRUE, TRUE);
	SendMessageToDescendants(WM_SYSCOLORCHANGE);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{  
	DWORD				dwStyle;
	DWORD				dwExStyle;
	
	if (QCWorkbook::OnCreate(lpCreateStruct) == -1)
		return (-1);

	// Shareware: Register that we want to know of feature changes
	QCSharewareManager *pSWM = GetSharewareManager();
	if (pSWM)
	{
		pSWM->QCCommandDirector::Register((QICommandClient*)this);
	}

	// Check if the user wants the Ctrl-F and Shift-Ctrl-F accels swapped (Find Text / Find Msgs)
	m_bSwitchFindAccel = (GetIniShort(IDS_INI_SEARCH_ACCEL_SWITCH) != 0);

	// Add Check Mail to the System menu
	CMenu* hMenu = GetSystemMenu(FALSE);
	
	if (hMenu)
	{
		CRString CheckMail(IDS_SYSTEM_MENU_CHECKMAIL);
		hMenu->AppendMenu(MF_SEPARATOR);
		hMenu->AppendMenu(MF_STRING, ID_SYSTEM_MENU_CHECKMAIL, CheckMail);
	}

	InitMenus();

	if( m_pControlBarManager == NULL )
	{
		return -1;
	}
	
	QCToolBarManager* pMgr = (QCToolBarManager*)m_pControlBarManager;

	// Define the toolbar bitmap and mappings of bitmaps index to cmd

	if( !pMgr->LoadToolBarResource() )
	{
		ASSERT( 0 );
		return -1;
	}

	// Set the button map
	pMgr->SetButtonMap(btnMap);

	if ( GetIniShort( IDS_INI_SHOW_COOLBAR ) )
	{
		pMgr->EnableCoolLook( TRUE );
	}
	else
	{
		pMgr->EnableCoolLook( FALSE );	
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE("Failed to create status bar\n");
		return (-1);      // fail to create
	}

	if (!GetIniShort(IDS_INI_SHOW_STATUS_BAR))
		m_wndStatusBar.ShowWindow(SW_HIDE);

	//
	// Check to see if the current EUDORA.INI that we're about to read
	// has compatible ToolBar state info.  If not, then flush the old
	// [ToolBar-xxx] sections from the INI file to avoid crashes in
	// MFC's LoadBarState() method (called below).  There is a known
	// incompatibility between the 32-bit Pro version and any Light
	// version or 16-bit version which doesn't support the dockable
	// mailbox "toolbar".
	//
	FlushIncompatibleToolBarState();

	//
	// Hack to override Runtime Class of mini docking frame windows
	// owned by the main frame to use the QC-defined mini docking
	// frame class.
	//
	ASSERT(NULL == m_pFloatingFrameClass);
	m_pFloatingFrameClass = RUNTIME_CLASS(QCMiniDockFrameWnd);

	//
	// Create the dockable Wazoo control bars.  IMPORTANT!  This MUST
	// be done before you call CMainFrame::LoadBarState(), otherwise
	// very bad things will happen.
	//
	m_WazooBarMgr.CreateInitialWazooBars(this);

	SetClassLong(m_hWnd, GCL_HICON, NULL);
	SetIcon();

	// Handle drag-and-drop messages
	// Toolbar and statusbar have to handle drag-and-drop because they will be
	// the ones notified when the main window is minimized
	DragAcceptFiles();
	m_wndStatusBar.DragAcceptFiles();
	        
	EnableDocking(CBRS_ALIGN_ANY);

	m_pToolBar = new QCCustomToolBar;  
	
	if( m_pToolBar == NULL )
	{
		ASSERT( 0 );
		return -1;
	}

	m_pToolBar->SetManager( pMgr );
	m_pToolBar->m_bAutoDelete = TRUE;
	dwStyle = WS_VISIBLE | WS_CHILD | CBRS_TOP | CBRS_SIZE_DYNAMIC;
	dwExStyle = 0L;
	
	if( pMgr->FlyByEnabled() )
	{
		dwStyle |= CBRS_FLYBY;
	}

	if( pMgr->ToolTipsEnabled() )
	{
		dwStyle |= CBRS_TOOLTIPS;
	}
	
	if( pMgr->CoolLookEnabled() )
	{
		dwExStyle = (CBRS_EX_COOLBORDERS | CBRS_EX_GRIPPER);
	}

	
	if( ! m_pToolBar->CreateEx(dwExStyle, this, dwStyle, AFX_IDW_TOOLBAR, _T(CRString(IDS_TOOLBAR_TITLE))))
	{
		ASSERT( 0 );
		return -1;
	}

	pMgr->SetToolBarInfo( m_pToolBar );
	m_pToolBar->EnableDocking(CBRS_ALIGN_ANY);
	m_pToolBar->SetButtons( buttons, DIM( buttons ) );
	DockControlBar( m_pToolBar );

	if (!GetIniShort(IDS_INI_SHOW_TOOLBAR))
	{
		m_pToolBar->ShowWindow( SW_HIDE );
	}

  	const CString TOOLBAR_SUMMARY("ToolBar-Summary");
  	const CString BARS("Bars");
  	char tbBuf[32] = { '\0' };
  	if (! ::GetPrivateProfileString(TOOLBAR_SUMMARY, BARS, "",
  					tbBuf, sizeof(tbBuf), INIPath)) {
    	    CString				szSection;
    	    POSITION			pos;
    	    pos = m_listControlBars.GetHeadPosition();
    	    while(pos != NULL) {
    		SECCustomToolBar*	pToolBar;
    		pToolBar = (SECCustomToolBar*) m_listControlBars.GetNext(pos);

    		if( pToolBar->IsKindOf( RUNTIME_CLASS( QCCustomToolBar ) ) ) {
    		    szSection.Format( "%s-BarID%lu", "ToolBar",
    				      pToolBar->GetDlgCtrlID() );
		    // Get the number of custom buttons.
		    int iCBC =  AfxGetApp()->GetProfileInt(szSection,
							   "CustomButtonCount",
							   0);
		    // Delete them from the INI file.
		    for (int iCBCIndex = 0; iCBCIndex < iCBC; iCBCIndex++) {
			char szValue[1024] = { '\0' };
			char *p = NULL, *pNext = NULL;
			int   iPosition = 0;
			CString szTBIDItem;
			szTBIDItem.Format("cmd%d", iCBCIndex);
			::GetPrivateProfileString(szSection, szTBIDItem, "",
						  szValue, 1024, INIPath);

			if( szValue[0] != '\0' ) {
			    // get the bitmap command id
			    p = strchr( szValue, ',' );
			    // Remove the button
			    if( p != NULL ) {
				// Ignore it and get the position.
				pNext = p + 1;
				p = strchr( pNext, ',' );

				if( p != NULL ) {
				    *p = '\0';
				    iPosition = atoi(pNext);
				    if (iPosition >= 0) {
					CString szTBBtn;
					szTBBtn.Format("btn%d", iPosition);
					::WritePrivateProfileString(szSection,
								    szTBBtn,
								    NULL,
								    INIPath);
				    }
				}
			    }
			    // Remove the command
			    ::WritePrivateProfileString(szSection, szTBIDItem,
							NULL, INIPath);
			}

		    }
		    // Set the CustomButtonCount to 0.
		    if (iCBC != 0) {
			::WritePrivateProfileString(szSection,
						    "CustomButtonCount",
						    NULL, INIPath);
		    }
    		}

    	    }
  	}

	LoadBarState(_T("ToolBar"));
	pMgr->LoadState(_T("ToolBar"));

	if( pMgr->ConvertOldStuff() )
		SaveBarState(_T("ToolBar"));

	//
	// Restore the Wazoo windows to their Wazoo containers...  IMPORTANT!
	// This must be done AFTER you call CMainFrame::LoadBarState(),
	// otherwise very bad things will happen.
	//
	m_WazooBarMgr.LoadWazooBarConfigFromIni();

//FORNOW	if (NULL == m_wndWazooBar.m_pDockBar)
//FORNOW	{
//FORNOW		//
//FORNOW		// Normally, LoadBarState() will take care of redocking
//FORNOW		// the CMboxBar back to its previous visibility state and
//FORNOW		// docking location.  However, in the one-time startup
//FORNOW		// case where the INI file does not contain the previous
//FORNOW		// toolbar state, then we must be sure to force the
//FORNOW		// CMboxBar to be docked, otherwise it won't be visible.
//FORNOW		// 
//FORNOW		DockControlBarEx(&m_wndWazooBar, AFX_IDW_DOCKBAR_LEFT, 0, 0, (float)1.00, 180);
//FORNOW	}
//FORNOW
//FORNOW	if (NULL == m_wndWazooBar2.m_pDockBar)
//FORNOW	{
//FORNOW		//
//FORNOW		// Normally, LoadBarState() will take care of redocking
//FORNOW		// the CMboxBar back to its previous visibility state and
//FORNOW		// docking location.  However, in the one-time startup
//FORNOW		// case where the INI file does not contain the previous
//FORNOW		// toolbar state, then we must be sure to force the
//FORNOW		// CMboxBar to be docked, otherwise it won't be visible.
//FORNOW		// 
//FORNOW		DockControlBarEx(&m_wndWazooBar2, AFX_IDW_DOCKBAR_BOTTOM, 0, 0, (float)1.00, 180);
//FORNOW	}

	SECControlBar::m_bOptimizedRedrawEnabled = FALSE;

	//
	// Set visibility of Auto-Wazoo task bar and intialize the tooltips
	// for the Auto-Wazoo task bar.
	//
	ShowMDITaskBar(GetIniShort(IDS_INI_MDI_TASKBAR));
	InitMDITaskBarTooltips();

	g_theMailboxDirector.InitializeMailboxTreeControl( GetActiveMboxTreeCtrl(), 0, NULL );

	//
	// And what the hell, let's register our OLE drag and drop object here...
	//
	VERIFY(m_dropTarget.Register(this));

	// Also read Eudora.ini for the POP_ACCOUNT field and split it into POP_SERVER and LOGIN_NAME
	// just for backward compatibility 
	SplitPOPACCOUNT() ;

#ifdef ADWARE_PROTOTYPE
	OnViewWebBrowser();
#endif

	// Register this application as a running automation server
	// if automation is enabled
	if (gbAutomationEnabled)
	{
		AutomationRegisterFactory();
	}

	// Lets create and start our timer function
	VERIFY(m_timerID = SetTimer(GENERAL_BACKGROUND_TIMER_ID, 1000, NULL));
	
	return (0);
}


////////////////////////////////////////////////////////////////////////
// OnTimer [protected]
//
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
	case AUTOACTIVATE_TIMER_EVENT_ID:
		{
			if (0 == m_idAutoActivateTimer)
				return;				// ignore queued leftover timer messages

			POINT pt;
			::GetCursorPos(&pt);
			ScreenToClient(&pt);

			//
			// Require at least two samples at the same cursor location
			// before doing an auto-activate.
			//
			static POINT last_pt = { -1, -1 };
			if ((pt.x == last_pt.x) && (pt.y == last_pt.y))
			{
				//
				// Check to see if we're over an "inactive" tab button.  If so,
				// then handle the auto-activate behavior here.
				//
				SECWorksheet* pHitInactiveSheet = TabHitTest(pt, FALSE);
				if (pHitInactiveSheet)
				{
					//
					// Note:  The following is copied from the 
					// QCWorkbook::OnLButtonDown() implementation.
					//
					MDIActivate(pHitInactiveSheet);
					UpdateWindow();
				}
			}
			last_pt.x = pt.x;
			last_pt.y = pt.y;
		}
		break;

	case GENERAL_BACKGROUND_TIMER_ID:
		// If Automation has a command queued, execute it now
		if (gbAutomationCommandQueued)
		{
			g_theAutomationDirector.ExecuteCommand();
			return;
		}
		break;

	default:
		//
		// Let the base class deal with timers that we didn't set.
		//
		QCWorkbook::OnTimer( nIDEvent );
	}
}


BOOL CMainFrame::OnQueryOpen()
{
	// Main window is being restored, so take away new mail icon
	SetIcon(FALSE);
	
	return (TRUE);
}

HCURSOR CMainFrame::OnQueryDragIcon()
{
	return (m_hIcon);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	// Since we don't set the icon in the class, WM_ICONERASEBKGND will never get called,
	// so we'll just do it when we get WM_ERASEBKGND and the app is minimized
	if (IsIconic() && m_hIcon)
	{
		DefFrameProc(m_hWnd, m_hWndMDIClient, WM_ICONERASEBKGND, (WPARAM)pDC->m_hDC, 0);
		return (TRUE);
	}
	
	return (QCWorkbook::OnEraseBkgnd(pDC));
}

void CMainFrame::OnPaint()
{
	// Since we don't have a class icon, we need to draw the icon
	if (IsIconic() && m_hIcon)
	{
		CPaintDC pDC(this);
		CRect rect;
		
		GetClientRect(&rect);
		pDC.DrawIcon(2, 2, m_hIcon);
		return;
	}

	QCWorkbook::OnPaint();

	// Don't you like this ClassWizard comment...
	// Do not call QCWorkbook::OnPaint() for painting messages
}		 

LRESULT CMainFrame::OnTrayCallback( WPARAM wParam, LPARAM lParam)
{
	UINT uID;
	UINT uMouseMsg;

	uID = (UINT)wParam;
	uMouseMsg = (UINT) lParam;

	if (uMouseMsg == WM_LBUTTONDBLCLK)
	{
		if (IsIconic())
			ShowWindow(SW_RESTORE);

		SetForegroundWindow();
	}

  return 0;
}


BOOL CMainFrame::TrayItem(int iIcon, int iOperation)
{
#ifdef WIN32
	NOTIFYICONDATA nid;			 

	nid.cbSize = sizeof(nid);					//  Required by API

	nid.hWnd = m_hWnd;							//  Notify this window
	nid.uID = iIcon + 999;						//  Give tray item some ID starting at 1000
	nid.uFlags = NIF_MESSAGE |					//  We got a message
				 NIF_ICON |						//  an icon
				 NIF_TIP;						//  and tooltip text
	nid.uCallbackMessage = WM_TRAY_CALLBACK;	//  Use WM_TRAY_CALLBACK for messages

	HINSTANCE hResInst = QCFindResourceHandle( MAKEINTRESOURCE(iIcon), RT_ICON );
	nid.hIcon = (HICON__*)::LoadImage(hResInst, MAKEINTRESOURCE(iIcon), IMAGE_ICON,16,16,0);
	
	char szTip[64];
	if (iOperation == NIM_DELETE)
		szTip[0] = 0;
	else
	{
		if (m_iNewMsgs == 1)
			sprintf(szTip, (const char*)CRString(IDS_ONE_NEW_MESSAGE), m_iNewMsgs);
		else
			sprintf(szTip, (const char*)CRString(IDS_MULTIPLE_NEW_MESSAGES), m_iNewMsgs);
	}

	lstrcpyn(nid.szTip, szTip, sizeof(szTip));


	Shell_NotifyIcon(iOperation, &nid);
 #endif

	return (TRUE);
}

// SetIcon
// Set the icon that gets displayed when minimized.
// HaveMailStatus can be set to TRUE or FALSE, or -1 means keep current HaveMail status.
// TrayItem ignores any calls made by a 16bit app.
void CMainFrame::SetIcon(int HaveMailStatus, int iNewMsgCount)
{
	UINT IconID = IDI_NO_MAIL;
	
	if (HaveMailStatus == TRUE || (HaveMailStatus == -1 && m_IconID >= IDI_HAVE_MAIL))
		IconID += 2;
	if (QueueStatus & QS_READY_TO_BE_SENT)
		IconID++;  
	if (IconID != m_IconID)
	{
	
		if (m_hIcon)
			DestroyIcon(m_hIcon);

		m_hIcon = QCLoadIcon( MAKEINTRESOURCE( IconID ) );
		m_IconID = IconID;

		// Redraw if minimized
		if (IsIconic())	
			Invalidate();

		m_iNewMsgs = iNewMsgCount;
	if (IsVersion4())
	{
		HICON	hIcon = QCLoadIcon(MAKEINTRESOURCE(IconID));
		SetClassLong(m_hWnd, GCL_HICON, (long)hIcon);
	}
		TrayItem(IDR_MAINFRAME,NIM_DELETE); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
		if ( IconID >= IDI_NO_MAIL + 2 )
			BOOL res= TrayItem(IDR_MAINFRAME,NIM_ADD); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
			
	
	}
	else if ( HaveMailStatus == TRUE )
	{
		m_iNewMsgs += iNewMsgCount;
	 	TrayItem(IDR_MAINFRAME,NIM_MODIFY); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
	}
	else
		m_iNewMsgs = 0;
}

UINT CMainFrame::GetTopWindowID()
{
	CFrameWnd* Top = GetActiveFrame();
	
	if (Top != this)
	{
		//
		// FORNOW, it is no longer true that all MDI child windows
		// are derived from the CMDIChild class defined by Eudora.
		//
		CMDIChild* pMDIChild = DYNAMIC_DOWNCAST(CMDIChild, Top);
		if (pMDIChild)
			return pMDIChild->GetTemplateID();
	}
		
	return (0);
}


////////////////////////////////////////////////////////////////////////
// GetWazooWindowWithFocus [public]
//
// Returns a pointer the Wazoo window, if any, that currently has the
// keyboard focus.
////////////////////////////////////////////////////////////////////////
CWazooWnd* CMainFrame::GetWazooWindowWithFocus()
{
	//
	// Root around to see if any Wazoo control bar has focus.
	//
	CWazooBar* pWazooBar = m_WazooBarMgr.GetWazooBarWithFocus();
	if (pWazooBar)
	{
		CWazooWnd* pActiveWazooWnd = pWazooBar->GetActiveWazooWindow();
		if (pActiveWazooWnd)
		{
			ASSERT_KINDOF(CWazooWnd, pActiveWazooWnd);
			return pActiveWazooWnd;
		}
	}
	
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// SetActiveWazooWindows [public]
//
// At start up time, we need to designate an "active" wazoo for each 
// Wazoo container.  Really, the idea here is to give the active wazoo
// in each visible wazoo container a chance to do its Just In Time
// display initialization.
////////////////////////////////////////////////////////////////////////
void CMainFrame::SetActiveWazooWindows()
{
	m_WazooBarMgr.SetActiveWazooWindows();
}


// Get the "real" size of client window, i.e. MDI client minus height of toolbar and status bar
void CMainFrame::GetRealClientRect(LPRECT lpRect)
{
	WINDOWPLACEMENT wp;
	
	::GetClientRect(m_hWndMDIClient, lpRect);

	// For a minimized window, get the restored size
	if (IsIconic())
	{
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp))
			*lpRect = wp.rcNormalPosition;

		//Adjust Rect back to zero so windows show up 
		// in the right place after iconization
		lpRect->bottom -= lpRect->top;
		lpRect->top = 0;
		lpRect->right -= lpRect->left;
		lpRect->left = 0;
	}
}

// Open window settings have the following format in the INI file:
//
// OpenWindow#=<Window ID> <State> [<Path> <Name> [<Index>] ]
//
// where:
//
// <Window ID>	Normalized type of the window (e.g. IDR_TOC is 2)
// <State>		State of the window (SW_SHOWNORMAL, SW_SHOWMINIMIZED, SW_SHOWMAXIMIZED)
// <Path>		Folder path of the mailbox (e.g. mailboxes in the first level
// 					will have an empty <Path>)
// <Name>		Full text name of the mailbox as seen in the menus (e.g. In)
// <Index>		Zero-based index of the message in the mailbox


// LoadOpenWindows
//
// Opens up the windows specified in the INI file
//
BOOL CMainFrame::LoadOpenWindows()
{
	CRString OpenWinSection(IDS_OPEN_WIN_SECTION);
	CRString OpenWinEntry(IDS_OPEN_WIN_ENTRY);
	char String[_MAX_PATH + 32];
	char MailboxPath[_MAX_PATH + 1];
	int NumWindows = 1;
	char Entry[32];
	CTocDoc* Toc;

	// If we didn't complete opening the windows on startup last time,
	// then let's not open them up this time, but allow it next time
	if (GetIniShort(IDS_INI_OPEN_WINDOWS_FAILED))
	{
		PurgeIniSetting(IDS_INI_OPEN_WINDOWS_FAILED);
		return TRUE;
	}

	SetIniShort(IDS_INI_OPEN_WINDOWS_FAILED, TRUE);
	FlushINIFile();

	strcpy(MailboxPath, EudoraDir);
	while (1)
	{
		wsprintf(Entry, OpenWinEntry, NumWindows++);
		if (!GetPrivateProfileString(OpenWinSection, Entry, "", String, sizeof(String), INIPath))
        	break;

		UINT WindowType;
		int WindowState = SW_SHOWNORMAL;
		char* s = strtok(String, ",");

		if (!s)
		{
			ASSERT(FALSE);
        	continue;
        }
		WindowType = atoi(s);
		if (!WindowType)
		{
			ASSERT(FALSE);
			continue;
		}
		WindowType += IDR_TOC - 2;
		if (!(s = strtok(NULL, ",")))
		{
			ASSERT(FALSE);
			continue;
		}
		WindowState = atoi(s);

		// Compatibility with old versions of Eudora
		if (WindowState == 128)
			WindowState = SW_SHOWMINIMIZED;
		else if (WindowState == 64)
			WindowState = SW_SHOWMAXIMIZED;
		else if (WindowState > 16)
			WindowState = SW_SHOWNORMAL;
			
		CMDIChild::m_nCmdShow = WindowState;

		switch (WindowType)
		{
		case IDR_TOC:
		case IDR_READMESS:
		case IDR_COMPMESS:
			{
				// Can't use strtok here because path could be empty
				char* path = s + strlen(s) + 1;
				if (!*path)
				{
					ASSERT(FALSE);
					continue;
				}
				s = strchr(path, ',');
				if (!s)
				{
					ASSERT(FALSE);
            		continue;
				}
           		*s++ = 0;
				strcpy(MailboxPath + EudoraDirLen, path);
				if (!(s = strtok(s, ",")))
				{
					ASSERT(FALSE);
            		continue;
				}
				Toc = GetToc(MailboxPath);
				if (WindowType == IDR_TOC)
				{
					if (Toc)
						Toc->Display();
				}			            	
				else
				{
					CSummary* Sum;
					POSITION pos;
					
					if (!(s = strtok(NULL, ",")))
					{
						ASSERT(FALSE);
	            		continue;
					}
					int Index = atoi(s);
					if (!Toc || Index < 0 || !(pos = Toc->m_Sums.FindIndex(Index)) ||
						!(Sum = Toc->m_Sums.GetAt(pos)))
					{
						// A really common thing is to create a new composition message and
						// then quit before making any modifications to it.  When that happens
						// the comp message gets saved in to the open window list, but then
						// gets removed when the window is closed.  So don't ASSERT when the
						// message index is just one more than the number of messages in the
						// Out mailbox.
						if (WindowType != IDR_COMPMESS || Index != Toc->m_Sums.GetCount())
							ASSERT(FALSE);
						continue;
					}
					Sum->Display();
				}
			}
			break;

		case IDR_SEARCH_WND:
		{
			OnEditFindFindMsg(); // Open the search window
		}
		break;

		case IDR_SIGNATURES:
			{
				// Can't use strtok here because signature name could be empty
				char* signature = s + strlen(s) + 1;
				
				if (!*signature)
				{
					ASSERT(FALSE);
					continue;
				}
				
				QCCommandObject* pCommand = g_theSignatureDirector.Find( signature );
				
				if( pCommand )
				{
					pCommand->Execute( CA_OPEN );
				}
			}
			break;
	
		case IDR_TEXTFILE:
			AfxGetApp()->OpenDocumentFile(s + strlen(s) + 1);
			break;
		}
	}

	// Restore the value to display child windows normally
	CMDIChild::m_nCmdShow = -1;

	PurgeIniSetting(IDS_INI_OPEN_WINDOWS_FAILED);

	return (TRUE);
}

// SaveOpenWindows
//
// Saves open windows to the INI file and closes the windows
//
BOOL CMainFrame::SaveOpenWindows(BOOL Close)
{
	CRString OpenWinSection(IDS_OPEN_WIN_SECTION);
	CRString OpenWinEntry(IDS_OPEN_WIN_ENTRY);
	int NumWindows = 1;
	char Entry[32];
	CCursor cursor;

    // Clear out [Open Windows] section
	WritePrivateProfileString(OpenWinSection, NULL, NULL, INIPath);

	CFrameWnd* Win = GetActiveFrame();
	
	// If no child windows, GetActiveFrame() returns a pointer to the main frame
	if (Win == this)
		return (TRUE);
		
	// Start out with the last child window, and work back to the first so that
	// the windows are openned up in the correct order
	while (Win->GetNextWindow())
		Win = (CFrameWnd*)Win->GetNextWindow();
		
	CWnd* PrevWin;
	for (; Win; Win = (CFrameWnd*)PrevWin)
	{
		PrevWin = Win->GetNextWindow(GW_HWNDPREV);
		
		// When a window gets minimized, there are some "extra" windows as top-level child windows.
		// We can skip by these "windows" by testing to see if they have permanent C++ classes
		// that wrap them.  If they don't, then they aren't windows that we're concerned with.
		if (!FromHandlePermanent(Win->m_hWnd))
			continue;
		
		UINT WindowType = ((CMDIChild*)Win)->GetTemplateID();
		CDocument* doc= Win->GetActiveDocument();
		char String[_MAX_PATH + 32];
		char Path[_MAX_PATH + 1];
		CSummary* Sum = NULL;
		CTocDoc* Toc = NULL;

		if (!doc)
		{
			//
			// The only known MDI Child windows without documents are the
			// MDI Wazoo container windows.  The SECControl stuff takes
			// care of handling the save/restore of these windows so
			// we don't have to worry about these here.
			//
			ASSERT_KINDOF(SECWorksheet, Win);
			continue;
		}
		if (Close && !doc->CanCloseFrame(Win))
			break;
		if (WindowType == IDR_TOC)
			Toc = (CTocDoc*)doc;
		else if (WindowType == IDR_READMESS || WindowType == IDR_COMPMESS)
		{
			if (!(Sum = ((CMessageDoc*)doc)->m_Sum) || !(Toc = Sum->m_TheToc))
			{
				ASSERT(FALSE);
				continue;
			}
		}

		int WindowState = SW_SHOWNORMAL;
		WINDOWPLACEMENT wp;
				
		wp.length = sizeof(WINDOWPLACEMENT);
		if (Win->GetWindowPlacement(&wp))
			WindowState = wp.showCmd;
		// Window type is compatible with older versions of Eudora, but window state isn't
		wsprintf(String, "%d,%d", WindowType - (IDR_TOC - 2), WindowState); 

		if (Toc)
		{
			strcpy(Path, Toc->MBFilename());
			// Only save part of path past the main Eudora directory
			wsprintf(String + strlen(String), ",%s,%s", Path + EudoraDirLen, (const char*)Toc->Name());
			if (Sum)
			{
				int Index = Toc->m_Sums.GetIndex(Sum);
				if (Index < 0)
					continue;
				wsprintf(String + strlen(String), ",%d", Index);
			}
		}
		else if (WindowType == IDR_SIGNATURES)
		{
			wsprintf(String + strlen(String), ",%s", (const char *) ((CSignatureDoc*)doc)->GetTitle());
		}
		else if (WindowType == IDR_TEXTFILE)
		{
			sprintf(String + strlen(String), ",%s", (const char*)doc->GetPathName());
		}
		wsprintf(Entry, OpenWinEntry, NumWindows++);
		WritePrivateProfileString(OpenWinSection, Entry, String, INIPath);
	}

	return (Win? FALSE : TRUE);
}

// SaveBarState
//
// This function override is a performance hack.  The standard routines save out the
// individual entries one at a time with CWinApp::WriteProfile[String|Int]().  This leads
// to very bad performance because the .INI file is opened in exclusive mode, the entry
// written, and then the file is closed: for each and every line!  So the hack is to
// temporarily put Eudora in to a state of thinking it should write out to the registry,
// calling the base class to do the dirty work.  Then copying the entries for an entire
// section and putting them in one buffer so that they can be efficiently written to the
// .INI file.  Lastly returning Eudora back to the state of writing out to the .INI file.
//
// If anything goes wrong during this, we assume that Something Bad Happened(TM), and so
// we'll just let it work the old way, under the assumption that writing out the info in
// an inefficient manor is better than not writing any info at all or the wrong info.
//
void CMainFrame::SaveBarState(LPCTSTR lpszProfileName) const
{
	CWinApp* pApp = AfxGetApp();
	const TCHAR RegRoot[] = "Qualcomm\\Eudora";
	LPCTSTR OldRegistryKey = pApp->m_pszRegistryKey;
	BOOL bMissionControlWeHaveAProblem = FALSE;

	// Turn on writing to the registry
	pApp->m_pszRegistryKey = RegRoot;

	while (1)
	{
		// Grunt work.  Note these two lines are duplicated below if there was a failure.
		QCWorkbook::SaveBarState(lpszProfileName);
		((QCToolBarManager*)m_pControlBarManager)->SaveState(lpszProfileName);

		HKEY hRootKey = pApp->GetSectionKey("");
		if (hRootKey == NULL)
		{
			bMissionControlWeHaveAProblem = TRUE;
			break;
		}

		TCHAR SectionName[MAX_PATH + 1];
		DWORD dwSectionIndex = 0;
		while (RegEnumKey(hRootKey, dwSectionIndex++, SectionName, sizeof(SectionName)) == ERROR_SUCCESS)
		{
			HKEY hSecKey;
			if (RegOpenKey(hRootKey, SectionName, &hSecKey) != ERROR_SUCCESS)
			{
				// Huh!  What happened?  I was told it was there!
				ASSERT(0);
				bMissionControlWeHaveAProblem = TRUE;
				break;
			}

			DWORD dwValueIndex = 0;
			TCHAR ValueName[MAX_PATH + 1];
			DWORD ValueNameSize;
			DWORD DataType;
			BYTE ValueData[MAX_PATH + 1];
			DWORD ValueDataSize;
			CString TotalSectionEntries;

			// Pre-allocate buffer size to prevent lots of allocations
			TotalSectionEntries.GetBuffer(1024);

			while (RegEnumValue(hSecKey, dwValueIndex++, ValueName, &(ValueNameSize = sizeof(ValueName)),
					NULL, &DataType, ValueData, &(ValueDataSize = sizeof(ValueData))) == ERROR_SUCCESS)
			{
				CString FormattedEntry;

				switch (DataType)
				{
				case REG_DWORD:
					FormattedEntry.Format("%s=%d\n", ValueName, *((DWORD*)ValueData));
					break;
				case REG_SZ:
					FormattedEntry.Format("%s=%s\n", ValueName, (LPCTSTR)ValueData);
					break;
				case REG_BINARY:
					{
						FormattedEntry.GetBuffer(ValueNameSize + 1 + ValueDataSize * 2 + 1);
						FormattedEntry.Format("%s=", ValueName);
						for (DWORD i = 0; i < ValueDataSize; i++)
						{
							FormattedEntry += (TCHAR)((ValueData[i] & 0x0F) + 'A');
							FormattedEntry += (TCHAR)(((ValueData[i] >> 4) & 0x0F) + 'A');
						}
						FormattedEntry += "\n";
					}
					break;
				default:
					// Shouldn't be any data in any other form
					ASSERT(0);
				}

				if (FormattedEntry.IsEmpty() == FALSE)
					TotalSectionEntries += FormattedEntry;
			}

			RegCloseKey(hSecKey);

			if (TotalSectionEntries.IsEmpty())
			{
				// We should have got something
				ASSERT(0);
			}
			else
			{
				// WritePrivateProfileSection() needs each line NULL-terminated
				LPTSTR Buffer = TotalSectionEntries.GetBuffer(TotalSectionEntries.GetLength());
				for (LPTSTR b = Buffer; *b; b++)
				{
					if (*b == '\n')
						*b = 0;
				}
				WritePrivateProfileSection(SectionName, Buffer, INIPath);
			}

			// There should have been at least one entry in there
			ASSERT(dwValueIndex > 1);
		}

		// Didn't your mother always tell you to clean up after you make a mess!
		// This will automatically do a RegCloseKey() on hRootKey.
		pApp->DelRegTree(hRootKey, "");

		// There should have been at least one section in there
		ASSERT(dwSectionIndex > 1);

		break;
	}

	// Turn back on writing to the INI file
	pApp->m_pszRegistryKey = OldRegistryKey;

	if (bMissionControlWeHaveAProblem)
	{
		// There was trouble, so let's just do it all
		// again writing directly to the INI file.
		// Note these two lines are duplicated above.
		QCWorkbook::SaveBarState(lpszProfileName);
		((QCToolBarManager*)m_pControlBarManager)->SaveState(lpszProfileName);
	}
}

CRect CMainFrame::GetMainWindowPosFromIni()
{
	CRect rectMainFrame(0, 0, 640, 480);

	if (GetIniWindowPos(IDS_INI_MAIN_WIN_POS, rectMainFrame))
	{
		CRect rectDesktop, rectIntersect;
		CWnd::GetDesktopWindow()->GetWindowRect(rectDesktop);

		if (rectIntersect.IntersectRect(&rectDesktop, &rectMainFrame) == FALSE)
		{
			//
			// The rect is set to (0,0,0,0), so this must be the first time 
			// we've launched Eudora.  In this case, auto-size Eudora to fit
			// nicely on the center of the desktop.
			//
			ASSERT(0 == rectDesktop.top && 0 == rectDesktop.left);

			if (rectDesktop.right <= 640 || rectDesktop.bottom <= 480)
			{
				//
				// Dang.  VGA resolution (or worse).  Take over the entire desktop.
				//
				rectMainFrame.SetRect(0, 0, 640, 480);
			}
			else
			{
				//
				// Cool.  Super VGA resolution.  Take over 90% of the desktop.
				//
				rectMainFrame = rectDesktop;
				rectMainFrame.DeflateRect(rectDesktop.right / 20, rectDesktop.bottom / 20);
			}
		}
	}

	return rectMainFrame;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	QCWorkbook::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	QCWorkbook::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
// Specialized handler for WM_SIZE messages on the main application
// window.  Basically, we need to override this to make an extra call
// to RecalcLayout() when the main window is restored or maximized.
// This extra call fixes up size of the docked mailboxes window for
// those cases.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	QCWorkbook::OnSize(nType, cx, cy);

	// The following code fixes the bug where if Eudora is iconified when closed,
	// restarted, then restored, the CompMessageViews and ReadMessageViews display
	// the text in a little box in the upper left corner of the frame.
	// Also, iconified views are usually displayed off screen.

	static BOOL bWasIconic = FALSE;
	static BOOL bArrangedIcons = FALSE;

	// make sure the RichEditView re-flows the text
	if ( nType == SIZE_MINIMIZED )
		bWasIconic = TRUE;
	else if ( ( nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED ) && bWasIconic )
	{
		bWasIconic = FALSE;

		// make sure all CompMessageViews display their text properly
#ifdef OLDSTUFF
		POSITION docpos = CompMessageTemplate->GetFirstDocPosition();
		while (docpos)
		{
			CDocument* doc = CompMessageTemplate->GetNextDoc(docpos);
			if ( doc )
			{
				POSITION viewpos = doc->GetFirstViewPosition();
				while (viewpos)
				{
				
					CCompMessageView* View = (CCompMessageView*)doc->GetNextView(viewpos);
					if ( View && View->IsKindOf(RUNTIME_CLASS(CCompMessageView)) ) 
					{
						// force the CompMessageView to re-flow the text
						CRect theRect;
						View->GetClientRect( theRect );
						View->PostMessage( WM_SIZE, SIZE_RESTORED, MAKELPARAM( theRect.Width(), theRect.Height() ) );
					}
				}
			}
		}
		// make sure all ReadMessageViews display their text properly
		docpos = ReadMessageTemplate->GetFirstDocPosition();
		while (docpos)
		{
			CDocument* doc = ReadMessageTemplate->GetNextDoc(docpos);
			if ( doc )
			{
				POSITION viewpos = doc->GetFirstViewPosition();
				while (viewpos)
				{
				
					CReadMessageView* View = (CReadMessageView*)doc->GetNextView(viewpos);
					if ( View && View->IsKindOf(RUNTIME_CLASS(CReadMessageView)) ) 
					{
						// force the ReadMessageView to re-flow the text
						if ( m_bStartedIconic && View->m_bFirstRestore && ! View->GetParentFrame()->IsIconic() )
						{
							View->m_bFirstRestore = FALSE;

							View->SetText();
							View->IconizeAttachments( FALSE );
							View->RedrawWindow();
						}
						else
						{
							CRect theRect;
							View->GetClientRect( theRect );
							View->PostMessage( WM_SIZE, SIZE_RESTORED, MAKELPARAM( theRect.Width(), theRect.Height() ) );
						}
					}
				}
			}
		}
#endif
		// arrange the icons
		if ( m_bStartedIconic && ! bArrangedIcons )
		{
			bArrangedIcons = TRUE;
			AfxGetMainWnd()->PostMessage( WM_COMMAND, ID_WINDOW_ARRANGE );
		}
	}
	// end of crunched text buf fix

	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		//
		// Dockable mailboxes window needs another kick in
		// the head to resize itself properly.
		//
		RecalcLayout();
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// IsOnStatusBar [public]
//
// Simple hit test for point on status bar.
//
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::IsOnStatusBar(const CPoint& ptClient) const
{
	if (m_wndStatusBar.IsVisible())
	{
		CRect rectClient;
		m_wndStatusBar.GetWindowRect(&rectClient);
		ScreenToClient(&rectClient);

		return rectClient.PtInRect(ptClient);
	}

	return FALSE;
}


BOOL CMainFrame::SetStatusBarText(const char* szText)
{
	if (m_wndStatusBar.IsVisible())
	{
		return m_wndStatusBar.SetPaneText(0,szText,true);
	}

	return FALSE;

	
}
////////////////////////////////////////////////////////////////////////
// PreTranslateMessage [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	ASSERT(m_pControlBarManager != NULL);
	ASSERT_KINDOF(SECToolBarManager, m_pControlBarManager);

	if(((SECToolBarManager*)m_pControlBarManager)->IsMainFrameEnabled())
	{
		// Stop the application closing with a customize dialog still active
		if(pMsg->message == WM_SYSCOMMAND && pMsg->wParam == SC_CLOSE)
			return TRUE;
	}

	if (QCWorkbook::PreTranslateMessage(pMsg))
		return TRUE;

	//
	// Give the main frame one more chance to handle MDI accelerator
	// messages like Ctrl+Tab and Ctrl+F4.  FORNOW, I wish I could be
	// more explicit on WHY we need to do this, but this hack seems to
	// solve the problems of various views and/or wazoos "eating"
	// these accelerators via IsDialogMessage() when the default
	// main frame handling ends up ignoring the keystrokes.
	//
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
	{
		// the MDICLIENT window may translate it
		if (::TranslateMDISysAccel(m_hWndMDIClient, pMsg))
		{
			TRACE2("CMainFrame::PreTranslateMessage: processed MDI keycode: %d (0x%X)\n", pMsg->wParam, pMsg->wParam);
			return TRUE;
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnCommand [protected, virtual]
//
// Override of virtual function from base class.  The idea is to change
// the standard command routing to include the Wazoo window, if any,
// that has the keyboard focus.
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	//
	// Root around to see if any Wazoo window has focus.
	//
#ifdef _DEBUG
	WORD CommandID = HIWORD(wParam);
	if (CommandID <= 1)
		CommandID = LOWORD(wParam);
	TRACE("CMainFrame::OnCommand -- got command %d (0x%X)\n", CommandID, CommandID);
#endif

	CWazooWnd* pWazooWnd = GetWazooWindowWithFocus();
	if (pWazooWnd)
	{
#ifdef _DEBUG
		TRACE("CMainFrame::OnCommand -- routing command %d (0x%X) to wazoo\n", CommandID, CommandID);
#endif
		ASSERT_KINDOF(CWazooWnd, pWazooWnd);
		if (AfxCallWndProc(pWazooWnd, pWazooWnd->m_hWnd, WM_COMMAND, wParam, lParam) != 0)
			return TRUE; // handled by wazoo
	}

	return QCWorkbook::OnCommand(wParam, lParam);
}


////////////////////////////////////////////////////////////////////////
// OnCmdMsg [public, virtual]
//
// Override of virtual function from base class.  The idea is to change
// the standard command routing to include the Wazoo window, if any,
// that has the keyboard focus.
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	//
	// Root around to see if any Wazoo window has focus.
	//
	CWazooWnd* pWazooWnd = GetWazooWindowWithFocus();
	if (pWazooWnd)
	{
		//TRACE2("CMainFrame::OnCmdMsg -- routing command %d (0x%X) to wazoo\n", nID, nID);
		if (pWazooWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return QCWorkbook::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


#ifdef OLDSTUFF
// This function changes the attach menu from Attach to Attach to New Message
void CMainFrame::AttachContextText()
{   
	char attachFileString[255];
	CMenu* messgeMenu = AfxGetMainWnd()->GetMenu()->GetSubMenu(3 + MenuOffset() );
	
	// The Attach File Item can be Attach File or Attach to New Message depending what's on top
	if (messgeMenu->GetMenuString( ID_MESSAGE_ATTACHFILE, attachFileString, 255, MF_BYCOMMAND ))
	{
		if (GetTopWindowID() == IDR_COMPMESS)
		{
			CRString fileText(IDS_ATTACH_FILE_TEXT);
			if (stricmp(attachFileString, fileText))
				messgeMenu->ModifyMenu(ID_MESSAGE_ATTACHFILE, 
										MF_BYCOMMAND | MF_STRING, 
										ID_MESSAGE_ATTACHFILE, 
										fileText);
		}
		else
		{
			CRString fileText(IDS_ATTACH_FILE_NEW_TEXT);
			if (stricmp(attachFileString, fileText))
				messgeMenu->ModifyMenu(ID_MESSAGE_ATTACHFILE, 
										MF_BYCOMMAND | MF_STRING, 
										ID_MESSAGE_ATTACHFILE, 
										fileText);
		}
	}

#ifdef COMMERCIAL
	int AttachOffset = 15;
#else
	int AttachOffset = 12;
#endif // COMMERCIAL
	char  attachString[255];

	// Attach Submenu can be Attach or Attach to New Message.  This menu may or may not be there...
	UINT isAttachMenu = messgeMenu->GetMenuItemID( AttachOffset );
	if (isAttachMenu == -1)
	{
		if (messgeMenu->GetMenuString( AttachOffset, attachString, 255, MF_BYPOSITION ))
		{
			extern CAttacherMenu g_AttacherMenu;

			if (GetTopWindowID() == IDR_COMPMESS)
			{
				CRString attachMenuText(IDS_ATTACH_MENU_TEXT);
				if (stricmp(attachString, attachMenuText))
				{

					// Detach the Submenu before ModifyMenu because modify will destroy the 
					// current submenu, then replace ... if it's the same ???
					HMENU safeMenu = messgeMenu->GetSubMenu(AttachOffset)->Detach();
					messgeMenu->ModifyMenu(AttachOffset, 
											MF_BYPOSITION | MF_STRING | MF_POPUP, 
											(UINT)safeMenu, 
											attachMenuText);
					g_AttacherMenu.Attach(safeMenu);
				}
			}
			else
			{
				CRString attachMenuText(IDS_ATTACH_MENU_NEW_TEXT);
				if (stricmp(attachString, attachMenuText))
				{
					// Detach the Submenu before ModifyMenu because modify will destroy the 
					// current submenu, then replace ... if it's the same ???
					HMENU safeMenu = messgeMenu->GetSubMenu(AttachOffset)->Detach();
					messgeMenu->ModifyMenu(AttachOffset, 
											MF_BYPOSITION | MF_STRING | MF_POPUP, 
											(UINT)safeMenu, 
											attachMenuText);
					g_AttacherMenu.Attach(safeMenu);
				}
			}
		}
	}
}
#endif


void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    QCWorkbook::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (CanShowProgress() || CCursor::m_Count)
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
		return (TRUE);
	}
	
	return (QCWorkbook::OnSetCursor(pWnd, nHitTest, message));
}

void CMainFrame::OnDestroy()
{
	// Save position, size, and state of main window
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);

	CRect rect;
	GetWindowRect(&rect);

	if (GetWindowPlacement(&wp))
	{
		if (wp.showCmd != SW_RESTORE && wp.showCmd != SW_SHOWNORMAL)
			rect = wp.rcNormalPosition;
		SetIniShort(IDS_INI_MAIN_WINDOW_STATE, short(wp.showCmd));
	}

	if (rect.IsRectEmpty() == FALSE)
		SetIniWindowPos(IDS_INI_MAIN_WIN_POS, rect);
	
	// Turn off drag-and-drop
	// Don't know if this actually does anything, but better safe than sorry
	DragAcceptFiles(FALSE);

	m_wndStatusBar.DragAcceptFiles(FALSE);

	// Free up resources for icon
	if (m_hIcon)
	{
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}

	ShutdownMDITaskBarTooltips();
	m_WazooBarMgr.DestroyAllWazooBars();

	//
	// Undo the subclassing of the MDI client, but keep the CWnd
	// wrapper attached to the MDI client's HWND.  If we don't do
	// this, then MFC gets upset when it tries to dispatch the
	// WM_DESTROY message to the bogus CWnd wrapper's WindowProc().
	// The CWnd wrapper is destroyed by the SEC base class.
	//
	ASSERT_VALID(m_pWBClient);
	ASSERT(m_pWBClient->GetSafeHwnd() == m_hWndMDIClient);
	m_pWBClient->UnsubclassWindow();		// detaches HWND, too
	m_pWBClient->Attach(m_hWndMDIClient);

	QCWorkbook::OnDestroy();
}

void CMainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction;

	UINT MenuID = lpMIS->itemID;

	// Is This A Label
	if ((MenuID >= ID_MESSAGE_LABEL_1) && (MenuID <= ID_MESSAGE_LABEL_7))
	{
		UINT nLabelIdx = (lpMIS->itemID - ID_MESSAGE_LABEL_1) + 1;
		LPCSTR Text = (QCLabelDirector::GetLabelDirector()->GetLabelText(nLabelIdx));
		
		CDC theDC;
		
		theDC.CreateCompatibleDC( NULL );
		
		if (!strlen(Text))	Text = "dummy";
		
		CSize size(theDC.GetTextExtent(Text, ::SafeStrlenMT(Text)));

		lpMIS->itemWidth = size.cx + LOWORD(GetMenuCheckMarkDimensions());
		lpMIS->itemHeight = size.cy + 2;
	}
	else if ( ( MenuID >= ID_COLOR0 ) && ( MenuID <= ID_COLOR16 ) )
	{
		CColorMenu::DoMeasureItem( lpMIS );
	}
	else if ( g_theCommandStack.Lookup( ( WORD ) MenuID, &pObject, &theAction ) )
	{
		ASSERT_KINDOF( QCPluginCommand, pObject );
		pObject->Execute( CA_MEASURE, lpMIS );
	}
	else
	{
		QCWorkbook::OnMeasureItem(nIDCtl, lpMIS);
	}
}

void CMainFrame::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	UINT				MenuID;
	COMMAND_ACTION_TYPE	theAction;
	QCCommandObject*	pCommand;
		
	MenuID = lpDIS->itemID & 0xFFFF;
	
	// Is This A Label
	if ((MenuID >= ID_MESSAGE_LABEL_1) && (MenuID <= ID_MESSAGE_LABEL_7))
	{
		CDC* pDC = CDC::FromHandle(lpDIS->hDC);
		UINT nLabelIdx = (MenuID - ID_MESSAGE_LABEL_1) + 1;
		BOOL Grayed = (lpDIS->itemState & ODS_GRAYED) == ODS_GRAYED;
		CRect rect(lpDIS->rcItem);
		COLORREF BackColor, ForeColor;

		if (lpDIS->itemState & ODS_SELECTED)
		{
			BackColor = Grayed? GetSysColor(COLOR_HIGHLIGHT) : (QCLabelDirector::GetLabelDirector()->GetLabelColor(nLabelIdx)); // GetIniLong(IniColorID);
			ForeColor = Grayed? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_MENU);
		}
		else
		{
			BackColor = GetSysColor(COLOR_MENU);
			ForeColor = Grayed? GetSysColor(COLOR_GRAYTEXT) : (QCLabelDirector::GetLabelDirector()->GetLabelColor(nLabelIdx)); // GetIniLong(IniColorID);
		}
		
		// Draw background
		CBrush BackBrush(BackColor);
		pDC->FillRect(&rect, &BackBrush);
		
		// Set text colors
		pDC->SetTextColor(ForeColor);
		pDC->SetBkMode(TRANSPARENT);
		
		LPCSTR pText = (QCLabelDirector::GetLabelDirector()->GetLabelText(nLabelIdx));
		rect.left += LOWORD(GetMenuCheckMarkDimensions()) * 3 / 2;
		if (IsVersion4())
			pDC->DrawState(rect.TopLeft(), rect.Size(), pText,
				DST_PREFIXTEXT | (Grayed? DSS_DISABLED : DSS_NORMAL), TRUE, 0, (HBRUSH)NULL);
		else
			pDC->DrawText(pText, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}
	else if ( ( MenuID >= ID_COLOR0 ) && ( MenuID <= ID_COLOR16 ) )
	{
		CColorMenu::DoDrawItem( lpDIS );
	}
	else if ( g_theCommandStack.Lookup( ( WORD ) MenuID, &pCommand, &theAction ) )
	{
		ASSERT_KINDOF( QCPluginCommand, pCommand );
		pCommand->Execute( CA_DRAW, lpDIS );
	}
	else
	{
		QCWorkbook::OnDrawItem(nIDCtl, lpDIS);
	}
}

void CMainFrame::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	CDocument* pDoc = GetActiveFrame()->GetActiveDocument();
	
	pCmdUI->Enable(pDoc && pDoc->IsModified());
}

void CMainFrame::OnSendQueuedMessages()
{
	if (ShiftDown())
	{
		CMailTransferOptionsDialog dlg(TRUE);
		dlg.DoModal();
	}
	else
	{
		SendQueuedMessages();
	}
}

void CMainFrame::OnUpdateSendQueuedMessages(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((QueueStatus & QS_READY_TO_BE_SENT) == QS_READY_TO_BE_SENT);
}


void CMainFrame::OnFileCheckMail()
{
	g_pApp->OnCheckMail();
}


CEdit* CMainFrame::GetCurrentEdit()
{
	CWnd* wnd = GetFocus();
	char ClassName[5];
	
	if (wnd && GetClassName(wnd->m_hWnd, ClassName, sizeof(ClassName)) > 0)
	{
		if (strnicmp(ClassName, "edit", 5) == 0)
			return ((CEdit*)wnd);
	}

	return (NULL);
}

CRichEditCtrl* CMainFrame::GetCurrentRichEdit()
{
	CWnd* wnd = GetFocus();
	char ClassName[9];
	
	if (wnd && GetClassName(wnd->m_hWnd, ClassName, sizeof(ClassName)) > 0)
	{
		if (strnicmp(ClassName, "richedit", 9) == 0)
			return ((CRichEditCtrl*)wnd);
	}

	return (NULL);
}


BOOL CMainFrame::CanModifyEdit()
{
    CEdit* Edit = GetCurrentEdit();

    if (Edit)
	return ((Edit->GetStyle() & ES_READONLY)? FALSE : TRUE);
    else {
	CRichEditCtrl* RichEdit = GetCurrentRichEdit();
	if (RichEdit)
	    return ((RichEdit->GetStyle() & ES_READONLY)? FALSE : TRUE);
	else {
	    CWnd *pWnd = GetFocus();
	    if (pWnd) {
		QCProtocol* QP = QCProtocol::QueryProtocol( QCP_TRANSLATE,
							    pWnd );
		if ( QP ) {
		    return ! QP->IsReadOnly();
		}
	    }
	}
    }

    return (FALSE);
}


void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	CEdit* Edit = GetCurrentEdit();

	pCmdUI->SetText(CRString(IDS_EDIT_UNDO));

	if (Edit)
	{
		pCmdUI->Enable(Edit->CanUndo());
		if (CanModifyEdit())
			return;
	}
	else
	{
		CRichEditCtrl* RichEdit = GetCurrentRichEdit();
		if (RichEdit)
		{
			pCmdUI->Enable(RichEdit->CanUndo());
			if (CanModifyEdit())
				return;
		}
	}

	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateEditFinishNickname(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanModifyEdit());
}

void CMainFrame::OnUpdateNeedSelEdit(CCmdUI* pCmdUI)
{
	CEdit* Ctrl = GetCurrentEdit();
	
	if (!CanModifyEdit())
		pCmdUI->Enable(FALSE);
	else
		OnUpdateNeedSel(pCmdUI);
}

void CMainFrame::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
	CEdit* Edit = GetCurrentEdit();
	
	if (Edit)
	{
		int nStartChar, nEndChar;
		Edit->GetSel(nStartChar, nEndChar);
		pCmdUI->Enable(nStartChar != nEndChar);
		return;
	}
	else
	{
		CRichEditCtrl* RichEdit = GetCurrentRichEdit();
		if (RichEdit)
		{
			long nStartChar, nEndChar;
			RichEdit->GetSel(nStartChar, nEndChar);
			pCmdUI->Enable(nStartChar != nEndChar);
			return;
		}
	}

	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateNeedClip(CCmdUI* pCmdUI)
{
	CEdit* Ctrl = GetCurrentEdit();
	
	pCmdUI->Enable(CanModifyEdit() && IsClipboardFormatAvailable(CF_TEXT));
}

void CMainFrame::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetCurrentEdit() != NULL) ||
					 (GetCurrentRichEdit() != NULL));
}

void CMainFrame::OnUpdateAttach(CCmdUI* pCmdUI)
{
	char		attachFileString[255];
	CMenu*		messgeMenu;
	CRString	fileText(IDS_ATTACH_FILE_NEW_TEXT);
	UINT		uOffset;
	BOOL		bMaximized;

	bMaximized = FALSE;
	MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );

	messgeMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset );
	
	// The Attach File Item can be Attach File or Attach to New Message depending what's on top
	if (messgeMenu->GetMenuString( ID_MESSAGE_ATTACHFILE, attachFileString, 255, MF_BYCOMMAND ))
	{
		if( stricmp( attachFileString, fileText ) )
		{
			messgeMenu->ModifyMenu(ID_MESSAGE_ATTACHFILE, 
									MF_BYCOMMAND | MF_STRING, 
									ID_MESSAGE_ATTACHFILE, 
									fileText);
		}
	}

	pCmdUI->Enable( TRUE );
}


void CMainFrame::OnEditUndo()
{
	CEdit* Edit = GetCurrentEdit();
	
	if (Edit)
		Edit->Undo();
	else
	{
		CRichEditCtrl* RichEdit = GetCurrentRichEdit();
		if (RichEdit)
			RichEdit->Undo();
	}
}

void CMainFrame::OnEditCut()
{
	CWnd* Ctrl = GetFocus();
	
	if (Ctrl)
		Ctrl->SendMessage(WM_CUT);
}


void CMainFrame::OnEditCopy()
{
	CWnd* Ctrl = GetFocus();
	
	if (Ctrl)
		Ctrl->SendMessage(WM_COPY);
}


void CMainFrame::OnEditPaste()
{
	CWnd* Ctrl = GetFocus();
	
	if (Ctrl)
		Ctrl->SendMessage(WM_PASTE);
}

void CMainFrame::OnEditPasteAsQuotation()
{
	CWnd* Ctrl = GetFocus();

	if (Ctrl && CanModifyEdit())
	{
		HANDLE hText;
		char* QuotedText = NULL;
		if (OpenClipboard())
		{
			hText = GetClipboardData(CF_TEXT);
			if (hText)
			{
				char* text = (char*)GlobalLock(hText);
				if (text)
					QuotedText = QuoteText(text, FALSE);
				GlobalUnlock(hText);
				if (QuotedText)
				{
					Ctrl->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)QuotedText);
					delete QuotedText;
				}
			}
			CloseClipboard();
		}
	}
}

void CMainFrame::OnEditClear()
{
	CWnd* Ctrl = GetFocus();
	
	if (Ctrl)
		Ctrl->SendMessage(WM_CLEAR);
}

void CMainFrame::OnEditSelectAll()
{
	CEdit* Edit = GetCurrentEdit();
	
	if (Edit)
		Edit->SetSel(0, -1, TRUE);
	else
	{
		CRichEditCtrl* RichEdit = GetCurrentRichEdit();
		if (RichEdit)
			RichEdit->SetSel(0, -1);
	}
}

void CMainFrame::OnEditWrapSelection()
{
	CWnd* Ctrl = GetFocus();
	BOOL DoUnwrap = ShiftDown();
	
	if (Ctrl && CanModifyEdit())
	{
		Ctrl->SendMessage(WM_COPY);
		
		if (OpenClipboard())
		{
			HANDLE hText = GetClipboardData(CF_TEXT);
			if (hText)
			{
				char* text = (char*)GlobalLock(hText);
				if (DoUnwrap)
					UnwrapText(text);
				else
					text = WrapText(text);
				if (text)
				{
					Ctrl->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)text);
					if (!DoUnwrap)
						delete text;
				}
				GlobalUnlock(hText);
			}
		}
		CloseClipboard();
	}
}

void CMainFrame::OnEditFinishNickname()
{
	CEdit* Ctrl = GetCurrentEdit();
	
	if (Ctrl)
		FinishNickname(Ctrl);
	else
	{
		CRichEditCtrl* RichCtrl = GetCurrentRichEdit();
		if (RichCtrl)
			FinishNickname(RichCtrl);
	}
}

// --------------------------------------------------------------------------

void CMainFrame::OnCtrlF()
{
	// Need to use SendMessage() so that command routing can be done
	if (m_bSwitchFindAccel) // IDS_INI_SEARCH_ACCEL_SWITCH
		SendMessage(WM_COMMAND, ID_EDIT_FIND_FINDTEXT);
	else
		SendMessage(WM_COMMAND, ID_EDIT_FIND_FINDMSG); // Default Ctrl-F
}

void CMainFrame::OnShiftCtrlF()
{
	// Need to use SendMessage() so that command routing can be done
	if (m_bSwitchFindAccel) // IDS_INI_SEARCH_ACCEL_SWITCH
		SendMessage(WM_COMMAND, ID_EDIT_FIND_FINDMSG);
	else
		SendMessage(WM_COMMAND, ID_EDIT_FIND_FINDTEXT); // Default Shift-Ctrl-F
}

void CMainFrame::OnEditFindFindMsg() // Find Msgs
{
	// Opening the Find Messages window can take a while
	CCursor WaitCursor;

	NewChildDocFrame(SearchTemplate);
}

void CMainFrame::OnEditFindFindText() // Find Text
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (pFindMgr)
		pFindMgr->Create();
}

void CMainFrame::OnEditFindFindTextAgain() // Find Again (F3)
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	if (pFindMgr)
		VERIFY(pFindMgr->DoFindAgain());
}

void CMainFrame::OnUpdateEditFindFindMsg(CCmdUI* pCmdUI) // Find Msg
{
	// Always available.
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateEditFindFindText(CCmdUI* pCmdUI) // Find Text
{
	// If we handle this, then no child has, so disable menu item.
	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI) // Find Again (F3)
{
	// If we handle this, then no child has, so disable menu item.
	pCmdUI->Enable(FALSE);
}

//
// OnFindReplace
//
// This is the message handler for WM_FINDREPLACE. The find dlg is a child
// of the CMainFrame, thus all messages are sent directly here. We keep
// an internal pointer to the actual window where the user wants to
// find. We then pass the WM_FINDREPLACE message to that window from here.
//
// The window pointer is set in OnActivate of the find dlg. The last window
// which had focus and all parents of that window are asked if they support
// find (WM_FINDREPLACE). The first window which supports find will get
// the message.
//
// If no window is found which supports find, then the pointer is cleared.
//
// This allows either the child window (control) to support the find
// directly, or a parent frame to support find for a group of controls.
//
// Look in CFindDlg and QCFindMgr for more information.
//
// Scott Manjourides, 12-7-98
//
LONG CMainFrame::OnFindReplace(WPARAM wParam, LPARAM lParam) // WM_FINDREPLACE
{
	QCFindMgr *pFindMgr = QCFindMgr::GetFindMgr();
	ASSERT(pFindMgr);

	// This is our internal check for asking if a given CWnd supports
	// find. Although we handle the WM_FINDREPLACE message, we do
	// not do any finding in the MainFrame, so return zero (false).
	if (pFindMgr->IsAck(wParam, lParam))
		return (EuFIND_ACK_NO);

	if (pFindMgr)
		VERIFY(pFindMgr->DoFind(wParam, lParam));

	return (EuFIND_OK);
}

// --------------------------------------------------------------------------

void CMainFrame::OnCloseAll()
{
	CWnd*		NextWin;
	CFrameWnd*	Win = GetActiveFrame();
	
	// The ID_FILE_SAVE commands sent below will cause this command handler to get called
	// recursively, so make sure we're only in here once
	static BOOL AlreadyIn = FALSE;
	if (AlreadyIn)
		return;
	AlreadyIn = TRUE;

	// If no child windows, GetActiveFrame() returns a pointer to the main frame
	if (Win == this)
		return;

	for (; Win; Win = (CFrameWnd*)NextWin)
	{
		NextWin = Win->GetNextWindow();
		Win->SendMessage(WM_CLOSE);
	}	

	AlreadyIn = FALSE;
}

void CMainFrame::OnSaveAll()
{
	CWnd*		NextWin;
	CFrameWnd*	Win = GetActiveFrame();
	
	// The ID_FILE_SAVE commands sent below will cause this command handler to get called
	// recursively, so make sure we're only in here once
	static BOOL AlreadyIn = FALSE;
	if (AlreadyIn)
		return;
	AlreadyIn = TRUE;

	// If no child windows, GetActiveFrame() returns a pointer to the main frame
	if (Win == this)
		return;
		
	for (; Win; Win = (CFrameWnd*)NextWin)
	{
		NextWin = Win->GetNextWindow();
		Win->SendMessage(WM_COMMAND, ID_FILE_SAVE);
	}	

	AlreadyIn = FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnFakeAppExit [protected]
//
// Workaround for a nasty toolbar bug.  That is, if you customize your
// toolbar and add a "File:Exit" button, clicking the File:Exit button
// will cause Eudora to crash since the SEC toolbar stuff handles the
// command via SendMessage().  Therefore, the workaround is to rig
// the toolbar File:Exit to generate a "ID_FAKE_APP_EXIT" command
// which is then converted to a real ID_APP_EXIT here via PostMessage().
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFakeAppExit()
{
	PostMessage(WM_COMMAND, ID_APP_EXIT);
}


////////////////////////////////////////////////////////////////////////
// OnUpdateFakeAppExit [protected]
//
// Always enable the fake exit command.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateFakeAppExit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


////////////////////////////////////////////////////////////////////////
// HuntForTopmostCompMessage [protected]
//
// Given a Wazoo window, hunt for the "next" MDI frame window which
// contains a usable comp message target.
//
// If we're checking this from an MDI mode Wazoo container, we need to
// check the "next" MDI child in the Z-order with a known non-Wazoo
// type.  Otherwise, check the topmost MDI window.
////////////////////////////////////////////////////////////////////////
CCompMessageDoc* CMainFrame::HuntForTopmostCompMessage(CWazooWnd* pWazooWnd)
{
	ASSERT(::IsMainThreadMT());

	CCompMessageDoc* pCompMessageDoc = NULL;		// returned

	ASSERT(pWazooWnd);
	ASSERT_KINDOF(CWazooWnd, pWazooWnd);
	SECControlBar* pParentBar = (SECControlBar *) pWazooWnd->GetParentControlBar();
	ASSERT_KINDOF(SECControlBar, pParentBar);

	if (pParentBar->IsMDIChild())
	{
		//
		// Given wazoo is housed in an MDI mode wazoo, so hunt for the
		// "next" MDI window, if any, in the Z-order.
		//
		CFrameWnd* pNextFrame = (CFrameWnd *) pWazooWnd->GetParentFrame()->GetNextWindow();
		if (pNextFrame)
		{
			ASSERT_KINDOF(CMDIChildWnd, pNextFrame);		// MFC MDI base class

			// 1st chance, check for pCompMessageDoc msg window
			CMDIChild* pMDIChild = DYNAMIC_DOWNCAST(CMDIChild, pNextFrame);
			if (pMDIChild && (IDR_COMPMESS == pMDIChild->GetTemplateID()))
			{
				pCompMessageDoc = (CCompMessageDoc*) pMDIChild->GetActiveDocument();
				if (pCompMessageDoc)
				{
					if (!pCompMessageDoc->m_Sum || pCompMessageDoc->m_Sum->CantEdit())
						pCompMessageDoc = NULL;
				}
			}
		}
	}
	else
	{
		//
		// Given wazoo is housed in a docked/floating wazoo, so
		//  hunt for "topmost" MDI window, if any.
		//

		// 1st chance, check for pCompMessageDoc msg window
		if (GetTopWindowID() == IDR_COMPMESS)
		{
			CMDIChildWnd* pMDIChild = (CMDIChild *) GetActiveFrame();
			ASSERT_KINDOF(CMDIChild, pMDIChild);
			pCompMessageDoc = (CCompMessageDoc *) pMDIChild->GetActiveDocument();
			ASSERT_KINDOF(CCompMessageDoc, pCompMessageDoc);
		}
	}

	return pCompMessageDoc;		// can be NULL
}


void CMainFrame::OnMessageNewMessage()
{
	// Create A New Message
	CCompMessageDoc* CompDoc = NULL;
	CompDoc = NewCompDocument();
	if ( CompDoc )
	{
		ASSERT_VALID(CompDoc);
		NewChildFrame(CompMessageTemplate, CompDoc);
	}
}

void CMainFrame::OnMessageReplyCtrlR()
{
	if ( GetIniShort(IDS_INI_REPLY_TO_ALL) )
	{
		PostMessage( WM_COMMAND, ID_MESSAGE_REPLY_ALL, 0 );
	}
	else
	{
		PostMessage( WM_COMMAND, ID_MESSAGE_REPLY, 0 );
	}
}

void CMainFrame::OnProcessAllofThem()
{

	QCGetTaskManager()->RequestPostProcessing();
}


void CMainFrame::OnUpdateMessageReply(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu)
	{
		// Set the text of the Reply/Reply To All menu item
		CRString Text( IDS_MESSAGE_REPLY );

		if ( GetIniShort(IDS_INI_REPLY_TO_ALL ) == 0 )
			Text += CRString( IDS_CTRL_R );

		UINT State = pCmdUI->m_pMenu->GetMenuState(pCmdUI->m_nIndex, MF_BYPOSITION);
		
		int nLen = Text.GetLength();
		int nPos;
		if ( g_bPopupOn )	
			if ( ( nPos = Text.Find('\t')) != -1)
				nLen= nPos;
			
		VERIFY(pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | MF_STRING | State,
			pCmdUI->m_nID, Text.Left(nLen)));
	}
}

void CMainFrame::OnUpdateMessageReplyAll(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu)
	{
		// Set the text of the Reply/Reply To All menu item
		CRString Text( IDS_MESSAGE_REPLY_ALL );

		if ( GetIniShort(IDS_INI_REPLY_TO_ALL ) != 0 )
			Text += CRString( IDS_CTRL_R );

		UINT State = pCmdUI->m_pMenu->GetMenuState(pCmdUI->m_nIndex, MF_BYPOSITION);
		
		int nLen = Text.GetLength();
		int nPos;
		if ( g_bPopupOn )	
			if ( ( nPos = Text.Find('\t')) != -1)
				nLen= nPos;
			
		VERIFY(pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | MF_STRING | State,
			pCmdUI->m_nID, Text.Left(nLen)));
	}
}


void CMainFrame::OnUpdateMessageSendImmediately(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu)
	{
		// Set the text of the Send Immediate/Queue For Delivery menu item
		CRString Text(GetIniShort(IDS_INI_IMMEDIATE_SEND)?
			IDS_MESSAGE_SEND_IMMEDIATE : IDS_MESSAGE_Q4_DELIVERY);
		UINT State = pCmdUI->m_pMenu->GetMenuState(pCmdUI->m_nIndex, MF_BYPOSITION);
		
		int nLen = Text.GetLength();
		int nPos;
		if ( g_bPopupOn )	
			if ( ( nPos = Text.Find('\t')) != -1)
				nLen= nPos;

		VERIFY(pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | MF_STRING | State, 
		pCmdUI->m_nID, Text.Left(nLen)));
	}
}

void CMainFrame::OnDisableMessageStatus(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateMessageLabel(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu)
	{
		UINT State = pCmdUI->m_pMenu->GetMenuState(pCmdUI->m_nIndex, MF_BYPOSITION) | MF_OWNERDRAW;
		VERIFY(pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | State, pCmdUI->m_nID));
	}
}



void CMainFrame::OnSpecialAddAsRecipient()
{
	
	CEdit* Edit = GetCurrentEdit();
	
	if (Edit)
	{
		int SelStart, SelEnd;
		Edit->GetSel(SelStart, SelEnd);
		
		// Let's not be ridiculous
		if (SelEnd > SelStart + 64)
		{
			::ErrorDialog(IDS_ERR_BAD_RECIPIENT_SELECTION);
			return;
		}

		Edit->Copy();
	}
	else
	{
		CRichEditCtrl* RichEdit = GetCurrentRichEdit();
		
		if( RichEdit == NULL )
		{
			return;
		}

		long SelStart, SelEnd;
		RichEdit->GetSel(SelStart, SelEnd);

		// Let's not be ridiculous
		if (SelEnd > SelStart + 64)
		{
			::ErrorDialog(IDS_ERR_BAD_RECIPIENT_SELECTION);
			return;
		}


		RichEdit->Copy();
	}

	if (!OpenClipboard())
		return;

	HANDLE hText = GetClipboardData(CF_TEXT);
	if (hText)
	{
		char* text = (char*)GlobalLock(hText);
		// No multiline allowed
		if (text && !strpbrk(text, "\r\n"))
		{
			g_theRecipientDirector.AddCommand( text );
		}
		else
		{
			::ErrorDialog(IDS_ERR_BAD_RECIPIENT_SELECTION);
		}
		GlobalUnlock(hText);
	}
	CloseClipboard();
}



void CMainFrame::OnSpecialEmptyTrash()
{
	EmptyTrash();
}

void CMainFrame::OnSpecialCompactMailboxes()
{
	g_theMailboxDirector.CompactMailboxes();
}


//called from eudora.cpp initinstance if no dominant account is specified
void CMainFrame::OnSpecialNewAccount()
{
	if (!gbAutomationRunning)
	{
		CWizardPropSheet dlg("", NULL, 0, true) ;
		dlg.StartWizard() ;
	}
}

void CMainFrame::OnSpecialSettings()
{
	CSettingsDialog dlg;
	INT				iOld;
	INT				iNew;

	iOld = GetIniShort( IDS_INI_SHOW_COOLBAR );
	dlg.DoModal();
	iNew = GetIniShort( IDS_INI_SHOW_COOLBAR );
		
	if( iNew != iOld )
	{
		QCToolBarManager* pMgr = (QCToolBarManager*)( m_pControlBarManager );
		pMgr->EnableCoolLook( iNew != 0 );
		pMgr->SaveState( _T( "ToolBar" ) );
		RecalcLayout();
	}
}

void CMainFrame::OnUpdateSpecialForgetPassword(CCmdUI* pCmdUI)
{
	//pCmdUI->Enable(POPPassword.IsEmpty() == FALSE);

	//
	// If there IMAP accounts with cached passwords, enable it.
	//
	if( g_Personalities.AccountsHavePasswords() || GetImapAccountMgr()->AccountsHavePasswords() )
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);

	
}

void CMainFrame::OnSpecialForgetPassword()
{

	ForgetPasswordByDialog();
	
	/*g_Personalities.EraseAllPasswords();
	ClearPassword();
	
	// We also need to clear the Kerberos ticket info
	//ClearKerberosTicket();
	QCKerberos::ClearKerberosTicket();

#ifdef IMAP4

	// IMAP:: Clear any cached passwords.
	GetImapAccountMgr()->ClearAllPasswords ();
#endif // IMAP4
	*/

}

void CMainFrame::OnSpecialChangePassword()
{
	ChangePassword();
}


void CMainFrame::OnUpdateSendToBack(CCmdUI* pCmdUI)
{
	SECWorksheet* pMDIChild = (SECWorksheet *) MDIGetActive();
	
	if (pMDIChild)
	{
		ASSERT_KINDOF(SECWorksheet, pMDIChild);
		pCmdUI->Enable(pMDIChild->GetNextWindow() != NULL);
	}
	else
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnSendToBack()
{
	MDINext();
}

void CMainFrame::OnWindowFilters()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CFiltersWazooWnd));
}

void CMainFrame::OnWindowMailboxes()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CMboxWazooWnd));

//FORNOW	if (m_wndWazooBar.IsMDIChild())
//FORNOW	{
//FORNOW		//
//FORNOW		// Activate the MDI child window containing the tree control.
//FORNOW		//
//FORNOW		SECWorksheet* p_childframe = (SECWorksheet *) m_wndWazooBar.GetParentFrame();
//FORNOW		ASSERT(p_childframe);
//FORNOW		ASSERT_KINDOF(SECWorksheet, p_childframe);
//FORNOW		p_childframe->MDIActivate();
//FORNOW	}
//FORNOW
//FORNOW	if (m_wndWazooBar.IsVisible())
//FORNOW	{
//FORNOW		// Set focus to the tree control window
//FORNOW		m_wndWazooBar.ActivateWazooWindow(RUNTIME_CLASS(CMboxWazooWnd));
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		// Not visible, so make it visible.
//FORNOW		OnShowMboxBar();
//FORNOW	}
}

void CMainFrame::OnViewStationery()
{
#ifdef COMMERCIAL
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CStationeryWazooWnd));
#endif // COMMERCIAL


//FORNOW	if (m_wndWazooBar.IsMDIChild())
//FORNOW	{
//FORNOW		//
//FORNOW		// Activate the MDI child window containing the tree control.
//FORNOW		//
//FORNOW		SECWorksheet* p_childframe = (SECWorksheet *) m_wndWazooBar.GetParentFrame();
//FORNOW		ASSERT(p_childframe);
//FORNOW		ASSERT_KINDOF(SECWorksheet, p_childframe);
//FORNOW		p_childframe->MDIActivate();
//FORNOW	}
//FORNOW
//FORNOW	if (m_wndWazooBar.IsVisible())
//FORNOW	{
//FORNOW		// Set focus to the tree control window
//FORNOW		m_wndWazooBar.ActivateWazooWindow(RUNTIME_CLASS(CStationeryWazooWnd));
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		// Not visible, so make it visible.
//FORNOW		OnShowStationery();
//FORNOW	}
}


void CMainFrame::OnViewFilterReport()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CFilterReportWazooWnd));

//FORNOW	if (m_wndWazooBar2.IsMDIChild())
//FORNOW	{
//FORNOW		//
//FORNOW		// Activate the MDI child window containing the tree control.
//FORNOW		//
//FORNOW		SECWorksheet* p_childframe = (SECWorksheet *) m_wndWazooBar2.GetParentFrame();
//FORNOW		ASSERT(p_childframe);
//FORNOW		ASSERT_KINDOF(SECWorksheet, p_childframe);
//FORNOW		p_childframe->MDIActivate();
//FORNOW	}
//FORNOW
//FORNOW	if (m_wndWazooBar2.IsVisible() )
//FORNOW	{
//FORNOW		// Set focus to the filter report window
//FORNOW		m_wndWazooBar2.ActivateWazooWindow(RUNTIME_CLASS(CFilterReportWazooWnd));
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		// Not visible, so make it visible.
//FORNOW		OnShowFilterReport();
//FORNOW	}
}


void CMainFrame::OnViewFileBrowser()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CFileBrowseWazooWnd));

//FORNOW	if (m_wndWazooBar.IsMDIChild())
//FORNOW	{
//FORNOW		//
//FORNOW		// Activate the MDI child window containing the tree control.
//FORNOW		//
//FORNOW		SECWorksheet* p_childframe = (SECWorksheet *) m_wndWazooBar.GetParentFrame();
//FORNOW		ASSERT(p_childframe);
//FORNOW		ASSERT_KINDOF(SECWorksheet, p_childframe);
//FORNOW		p_childframe->MDIActivate();
//FORNOW	}
//FORNOW
//FORNOW	if (m_wndWazooBar.IsVisible())
//FORNOW	{
//FORNOW		// Set focus to the file browser window
//FORNOW		m_wndWazooBar.ActivateWazooWindow(RUNTIME_CLASS(CFileBrowseWazooWnd));
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		// Not visible, so make it visible.
//FORNOW		OnShowFileBrowser();
//FORNOW	}
}


void CMainFrame::OnViewSignatures()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CSignatureWazooWnd));
}

void CMainFrame::OnViewPersonalities()
{
#ifdef COMMERCIAL
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CPersonalityWazooWnd));
#endif // COMMERCIAL
}

void CMainFrame::OnViewTaskStatus()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CTaskStatusWazooWnd));
}

void CMainFrame::OnViewTaskError()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CTaskErrorWazooWnd));
}

void CMainFrame::OnViewWebBrowser()
{
#ifdef WEB_BROWSER_WAZOO
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CWebBrowserWazooWnd));
#endif
}

void CMainFrame::OnWindowNicknames()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CNicknamesWazooWnd));

//FORNOW	if (g_Nicknames)
//FORNOW	{
//FORNOW		// Activate an existing window
//FORNOW		NewChildDocFrame(NicknamesTemplate);
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		//
//FORNOW		// Launch a new window.
//FORNOW		//
//FORNOW		if (LoadNicknames())
//FORNOW		{
//FORNOW			BOOL execute_zoom_hack = FALSE;
//FORNOW			{
//FORNOW				CFrameWnd* p_activewnd = GetActiveFrame();
//FORNOW				if ((p_activewnd != this) && p_activewnd->IsZoomed())
//FORNOW				{
//FORNOW					//
//FORNOW					// Holy cow Batman!  Whatta hack.  Willie can't figure out
//FORNOW					// how to get the darn 32-bit Nicknames window to initialize
//FORNOW					// properly when MDI is in zoomed mode.  So, let's take
//FORNOW					// MDI out of zoomed mode temporarily while we launch
//FORNOW					// the Nicknames window.
//FORNOW					//
//FORNOW//					p_activewnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
//FORNOW					MDIRestore(p_activewnd);
//FORNOW					execute_zoom_hack = TRUE;
//FORNOW				}
//FORNOW			}
//FORNOW			ASSERT(g_Nicknames != NULL);
//FORNOW			CFrameWnd* p_framewnd = NewChildFrame(NicknamesTemplate, g_Nicknames);
//FORNOW			if (execute_zoom_hack)
//FORNOW			{
//FORNOW				//
//FORNOW				// Hack, part 2.  MDI was previously in zoomed mode, so
//FORNOW				// zoom the freshly launched Nicknames window.  This
//FORNOW				// completes the hack.  We now return you to your
//FORNOW				// regularly scheduled program.
//FORNOW				//
//FORNOW				p_framewnd->PostMessage(WM_SYSCOMMAND, SC_ZOOM, 0);
//FORNOW			}
//FORNOW
//FORNOW//FORNOW			BOOL bIsMaximized = FALSE;
//FORNOW//FORNOW			CMDIChildWnd* pActiveMDIChild = MDIGetActive(&bIsMaximized);
//FORNOW//FORNOW			CMDIChildWnd* p_framewnd = (CMDIChildWnd *) NewChildFrame(NicknamesTemplate, g_Nicknames);
//FORNOW//FORNOW			ASSERT_KINDOF(CMDIChildWnd, p_framewnd);
//FORNOW//FORNOW//			m_pWBClient->SendMessage(WM_MDIACTIVATE, WPARAM(p_framewnd->GetSafeHwnd()), 0);
//FORNOW//FORNOW			if (bIsMaximized)
//FORNOW//FORNOW			{
//FORNOW//FORNOW//				p_framewnd->MDIMaximize();
//FORNOW//FORNOW				m_pWBClient->SetRedraw(FALSE);
//FORNOW//FORNOW				m_pWBClient->SendMessage(WM_MDIRESTORE, WPARAM(p_framewnd->GetSafeHwnd()), 0);
//FORNOW//FORNOW				m_pWBClient->SendMessage(WM_MDIMAXIMIZE, WPARAM(p_framewnd->GetSafeHwnd()), 0);
//FORNOW//FORNOW				m_pWBClient->SetRedraw(TRUE);
//FORNOW//FORNOW				p_framewnd->Invalidate();
//FORNOW//FORNOW				p_framewnd->UpdateWindow();
//FORNOW//FORNOW			}
//FORNOW		}
//FORNOW	}
}

void CMainFrame::OnViewLookup()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(DirectoryServicesWazooWndNew));

//FORNOW	if (m_wndWazooBar2.IsMDIChild())
//FORNOW	{
//FORNOW		//
//FORNOW		// Activate the MDI child window containing the lookup window.
//FORNOW		//
//FORNOW		SECWorksheet* p_childframe = (SECWorksheet *) m_wndWazooBar2.GetParentFrame();
//FORNOW		ASSERT(p_childframe);
//FORNOW		ASSERT_KINDOF(SECWorksheet, p_childframe);
//FORNOW		p_childframe->MDIActivate();
//FORNOW	}
//FORNOW
//FORNOW	if (m_wndWazooBar2.IsVisible())
//FORNOW	{
//FORNOW		// Set focus to the tree control window
//FORNOW		m_wndWazooBar2.ActivateWazooWindow(RUNTIME_CLASS(CLookupWazooWnd));
//FORNOW	}
//FORNOW	else
//FORNOW	{
//FORNOW		// Not visible, so make it visible.
//FORNOW		OnShowLookup();
//FORNOW	}
}


void CMainFrame::OnDeleteSignature()
{
#ifdef COMMERCIAL
	CDeleteDialog theDlg( FALSE );
	theDlg.DoModal();
#endif // COMMERCIAL
}


void CMainFrame::OnNewSignature()
{
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


void CMainFrame::NewMessage(char* to, char *from, char *sub,char *cc, 
							char *bcc, char *attach, char *body)				
{
	CCompMessageDoc* p_comp = NewCompDocument(to,from, sub, cc, bcc,attach, body); 
	
	if (p_comp)
	{
		// Make sure Eudora gets brought to the foreground,
		// even if it is minimized.
		ActivateFrame();
		SetForegroundWindow();

		// Display the new composition window.
		//p_comp->m_Sum->SetFlag(MSF_MAPI_MESSAGE);
		p_comp->m_Sum->Display();
		p_comp->SetModifiedFlag();
	}
}


////////////////////////////////////////////////////////////////////////
// GetActiveMboxTreeCtrl [public]
//
// Return a pointer to the current, active CMboxTreeCtrl object.
////////////////////////////////////////////////////////////////////////
CMboxTreeCtrl* CMainFrame::GetActiveMboxTreeCtrl()
{
	if (m_pActiveMboxTreeCtrl)
	{
		ASSERT_VALID(m_pActiveMboxTreeCtrl);
		ASSERT(::IsWindow(m_pActiveMboxTreeCtrl->m_hWnd));
	}
	else
	{
		ASSERT(0);
	}

	return m_pActiveMboxTreeCtrl;
}


////////////////////////////////////////////////////////////////////////
// GetActiveFilterReportView [public]
//
// Return a pointer to the current, active CFilterReportView object.
////////////////////////////////////////////////////////////////////////
CFilterReportView* CMainFrame::GetActiveFilterReportView()
{
	if (m_pActiveFilterReportView)
	{
		ASSERT_VALID(m_pActiveFilterReportView);
		ASSERT(::IsWindow(m_pActiveFilterReportView->m_hWnd));
	}
	else
	{
		ASSERT(0);
	}

	return m_pActiveFilterReportView;
}

// smohanty: commented out for the time being.
////////////////////////////////////////////////////////////////////////
// GetActiveLookupView [public]
//
// Return a pointer to the current, active CLookupView object.
////////////////////////////////////////////////////////////////////////
// CLookupView* CMainFrame::GetActiveLookupView()
// {
// 	if (m_pActiveLookupView)
// 	{
// 		ASSERT_VALID(m_pActiveLookupView);
// 		ASSERT(::IsWindow(m_pActiveLookupView->m_hWnd));
// 	}
// 	else
// 	{
// 		ASSERT(0);
// 	}

// 	return m_pActiveLookupView;
// }

////////////////////////////////////////////////////////////////////////
// GetActivePersonalityView [public]
//
// Return a pointer to the current, active CPersonalityView object.
////////////////////////////////////////////////////////////////////////
CPersonalityView* CMainFrame::GetActivePersonalityView()
{
	if (m_pActivePersonalityView)
	{
		ASSERT_VALID(m_pActivePersonalityView);
		ASSERT(::IsWindow(m_pActivePersonalityView->m_hWnd));
	}
	else
	{
		ASSERT(0);
	}

	return m_pActivePersonalityView;
}

////////////////////////////////////////////////////////////////////////
// GetActiveTaskStatusView [public]
//
// Return a pointer to the current, active CTaskStatusView object.
////////////////////////////////////////////////////////////////////////
CTaskStatusView* CMainFrame::GetActiveTaskStatusView()
{
	if (m_pActiveTaskStatusView)
	{
		ASSERT_VALID(m_pActiveTaskStatusView);
		ASSERT(::IsWindow(m_pActiveTaskStatusView->m_hWnd));
	}
	else
	{
		ASSERT(0);
	}

	return m_pActiveTaskStatusView;
}

////////////////////////////////////////////////////////////////////////
// GetActiveWebBrowserView [public]
//
// Return a pointer to the current, active CWebBrowserView object.
////////////////////////////////////////////////////////////////////////
CWebBrowserView* CMainFrame::GetActiveWebBrowserView()
{
#ifdef WEB_BROWSER_WAZOO
	if (m_pActiveWebBrowserView)
	{
		ASSERT_VALID(m_pActiveWebBrowserView);
		ASSERT(::IsWindow(m_pActiveWebBrowserView->m_hWnd));
	}
	else
	{
		ASSERT(0);
	}
#endif

	return m_pActiveWebBrowserView;
}

void CMainFrame::CreateMessage(char* cmdLine)				
{
	//
	// Handle mailto URL on command line, if any.
	//
	if (!strnicmp(cmdLine, "/m ", 3) || !strnicmp(cmdLine, "mailto:", 7))
	{
		CString mailto(cmdLine + (*cmdLine == '/'? 3 : 0));	// working copy as CString

		//
		// Parse the mailto string, looking for Netscape-specific
		// parameters.
		//
		CString to;
		CString cc;
		CString bcc;
		CString subject;
		CString body;
		if (CURLEdit::ParseMailtoArgs(mailto, to, cc, bcc, subject, body))
		{
			NewMessage((char *) ((const char *) to),		// To
						NULL,								// From
						(char *) ((const char *) subject),	// Subject
						(char *) ((const char *) cc), 		// Cc
						(char *) ((const char *) bcc),		// Bcc
						NULL,								// Attachments
						(char *) ((const char *) body));	// Body
			return; 
		}
	}

	char attachLine[_MAX_PATH +1];
	GetAttachmentLine(cmdLine, attachLine);
	{
		if (*attachLine)
			NewMessage(NULL,NULL,NULL,NULL,NULL,attachLine,NULL);
		return; 
	}
}

void CMainFrame::GetAttachmentLine(char* cmdLine, char *attachLine)
{	
	char *c;
	struct stat FileAttr;
	BOOL DONE = FALSE;
	char path[_MAX_PATH +1];
	attachLine[0] = 0;

	// build up attachments line (disregard folders)
	while (!DONE)
	{
		if (c = strchr(cmdLine, ' '))
			*c++ = 0;
		else
			DONE = TRUE;

		stat(cmdLine, &FileAttr);
		if (!(FileAttr.st_mode & S_IFDIR))
		{
			*path = 0;
			char *c = NULL;
			c = strrchr(cmdLine, '\\');
			
			// Make sure we got a full path name
			if (c)
			{
				// Get the long file name
				WIN32_FIND_DATA ffd;
				HANDLE FF = FindFirstFile(cmdLine,&ffd);
				FindClose(FF);

				// create entire path
				*c = 0;
				lstrcpy(path, cmdLine);
				lstrcat(path, "\\");
				lstrcat(path, ffd.cFileName);

				if (!*attachLine)
				{
					lstrcpy(attachLine, path);
				}
				else
				{
					lstrcat(attachLine, " ");
					lstrcat(attachLine, path);
				}

				// always end with a ';'
				lstrcat(attachLine, ";");
			}
		}
		cmdLine = c;
	}
}


LRESULT CMainFrame::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;
	if (!pcds) return -1;
	DWORD Msg = pcds->dwData;
 	
 	if (Msg == 1/*ATTACH_FILE*/ || Msg == 2/*STATIONERY_FILE*/ || Msg == 3/*MESSAGE_FILE*/)
	{
		if ((char *)pcds->lpData && strlen((char *)pcds->lpData))
		{
			// Make a copy of the cmdline before it goes away
			char *cmdLine = new char[strlen((char *)pcds->lpData)+1];
			strcpy(cmdLine,(char *)pcds->lpData);

			if (Msg == 1/*ATTACH_FILE*/ )
			{
				CreateMessage(cmdLine);
			}

			if (Msg == 2/*STATIONERY_FILE*/ )
			{
				CCompMessageDoc* Comp = NewCompDocumentWith(cmdLine);
				if (Comp)
					Comp->m_Sum->Display();
			}

			if (Msg == 3/*MESSAGE_FILE*/) 
			{
				NewMessageFromFile( cmdLine );
			}

			delete []cmdLine;
		}				
	}
 	

	//extern SetKerb16Ticket(PCOPYDATASTRUCT);
	if (Msg == SEND_TICKET || Msg == FAILED_SEND_TICKET)
	{
		//SetKerb16Ticket(pcds);
		QCKerberos::SetKerb16Ticket(pcds);
	}

	if (Msg >= MAPI_MSG_FIRST && Msg <= MAPI_MSG_LAST)
		return (HandleMAPIMessage(pcds));

	return (0);
}

void CMainFrame::OnHelpSearchForHelpOn()
{
	AfxGetApp()->WinHelp((DWORD)(LPCSTR)"", HELP_PARTIALKEY);
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	int Count = DragQueryFile(hDropInfo, (UINT)(-1), NULL, 0);
	BOOL Minimized = IsIconic();
	CCompMessageDoc* CompDoc = NULL;
	
	// If the main window is minimized and the top window is a composition window that
	// hasn't been sent, then attach to that message, otherwise create a new meesage
	// and attach the file(s) to it
	if (Minimized && GetTopWindowID() == IDR_COMPMESS)
	{
		CompDoc = (CCompMessageDoc*)GetActiveFrame()->GetActiveDocument();
		
		if (CompDoc && CompDoc->m_Sum->CantEdit())
			CompDoc = NULL;
	}
	
	if (!CompDoc)
	{
		CompDoc = NewCompDocument();
		if (!CompDoc || CompDoc->m_Sum->Display() == FALSE)
			return;
	}

	for (int i = 0; i < Count; i++)
	{
		char Filename[_MAX_PATH + 1];
		
		if (DragQueryFile(hDropInfo, i, Filename, sizeof(Filename)) > 0)
			CompDoc->AddAttachment(Filename);
	}
}

void CreateAutoMessage(const char* BodyText, UINT AddressID = IDS_MAILING_LIST_ADDRESS)
{
	CCompMessageDoc* comp = NewCompDocument(CRString(AddressID),
		NULL,CRString(IDS_MAILING_LIST_SUBJECT), NULL, NULL, NULL, BodyText);

	comp->m_Sum->UnsetFlag(MSF_USE_SIGNATURE | MSF_ALT_SIGNATURE);
	comp->Queue(TRUE);

	// Don't want the message sent right away if Send Immediate is on
	FlushQueue = FALSE;

	ErrorDialog(IDS_MAILING_LIST_OUT);
}

void CMainFrame::OnMoreHelp()
{
	::WinHelp(m_hWnd, CRString(IDS_MOREHELP_FILE), HELP_CONTENTS, 0L);
}

BOOL CMainFrame::OnMailingList(UINT nID)
{
	BOOL IsForum = (nID == ID_HELP_EUDORAFORUM);
	CRString Blurb(IsForum? IDS_EUDORAFORUM_TEXT : IDS_QUESTNEWS_TEXT);

	int ReturnValue = AlertDialog(IDD_MAILING_LIST, (const char*)Blurb);
	
	if (ReturnValue != IDCANCEL)
	{
		CString Command;
		
		AfxFormatString1(Command, (ReturnValue == IDC_UNSUBSCRIBE?
			IDS_MAILING_LIST_UNSUB : IDS_MAILING_LIST_SUB),
			(const char*)CRString(IsForum? IDS_EUDORAFORUM_LIST : IDS_QUESTNEWS_LIST));
		CreateAutoMessage(Command);
	}
	
	return (TRUE);
}


BOOL CMainFrame::QuerySendQueuedMessages()
{
	if (!GetIniShort(IDS_INI_WARN_QUIT_WITH_QUEUED))
		return (TRUE);

	SetQueueStatus();

	if (QueueStatus & QS_DELAYED_WITHIN_12)
	{
		switch (AlertDialog(IDD_SEND_QUEUED_12))
		{
		case IDCANCEL:
			return (FALSE);
			
		case IDC_SEND_ALL_DELAYED:
			//if (SendQueuedMessages(QS_DELAYED_MORE_THAN_12) != SQR_ALL_OK)
			SendQueuedMessagesAndQuit(QS_DELAYED_MORE_THAN_12);
			return (FALSE);
			break;

		case IDOK:
			//if (SendQueuedMessages(QS_DELAYED_WITHIN_12) != SQR_ALL_OK)
			SendQueuedMessagesAndQuit(QS_DELAYED_MORE_THAN_12);
			return (FALSE);
			break;
		}
	}
	else if (QueueStatus & QS_READY_TO_BE_SENT)
	{
		switch (AlertDialog(IDD_SEND_QUEUED))
		{
		case IDCANCEL:
			return (FALSE);

		case IDOK:
			//if (SendQueuedMessages() != SQR_ALL_OK)
			SendQueuedMessagesAndQuit(QS_READY_TO_BE_SENT, TRUE);
			return (FALSE);
		}
	} 

	return (TRUE);
}

BOOL CMainFrame::OnQueryEndSession()
{
	if (!CloseDown())
		return (FALSE);
		
	return (QCWorkbook::OnQueryEndSession());
}

void CMainFrame::OnClose()
{
	if (!CloseDown()) return;

#ifdef IMAP4
	// Do any IMAP cleanup required:
	CImapMailMgr::CloseImapConnections ();
#endif

	if (GetIniShort(IDS_INI_EMPTY_TRASH_ON_QUIT))
	{
		// we're going down regardless of whether the user confirms
		// the Empty Trash warning or not...
		EmptyTrash();
	}

	SetIcon(FALSE);

	// Timer needs to be killed before the window closes
	if (m_timerID)
		KillTimer(m_timerID);

#ifdef OLDSTUFF
	// Save recipient list (member function checks whether or not a save is necessary)
	g_RecipientList.Write();
#endif

	// Winsock and Dialup must shutdown before the app can.
	if (NetConnection)
	{
		delete(NetConnection);
		NetConnection = NULL;
	}

	TrayItem(IDR_MAINFRAME,NIM_DELETE); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT

	QCWorkbook::OnClose();

	// Get rid of TOCs still in memory
	TocCleanup();
	
	// Lets destroy the popHost list at this point
	delete gPopHostList;
	gPopHostList = NULL;		

	// Get rid of our one and only Search doc
	delete gSearchDoc;

	// Shut down automation if running
	AutomationStop();

	// Unregister from shareware notifications
	QCSharewareManager *pSWM = GetSharewareManager();
	if (pSWM)
	{
		pSWM->UnRegister(this);
	}
}

BOOL CMainFrame::CloseDown()
{ 
	int nTaskCount = QCGetTaskManager()->GetTaskCount();
	if (nTaskCount > 0)
	{
		if (GetIniShort(IDS_INI_TASK_WARN_CLOSE))
		{
			if (WarnYesNoDialog(IDS_INI_TASK_WARN_CLOSE, IDS_TASK_WARN_CLOSE, nTaskCount) == IDCANCEL)
				return FALSE;
		}
	}

	if(QCRasLibrary::IsActive())
	{
		if( GetIniShort(IDS_INI_RAS_CLOSE_ON_EXIT))
			QCRasLibrary::CloseConnection();
	}


    // Close DS print preview if it is open.
    if (m_pDirectoryServicesWazooWndNew) {
	ASSERT_VALID(m_pDirectoryServicesWazooWndNew);
	ASSERT(::IsWindow(m_pDirectoryServicesWazooWndNew->m_hWnd));
	m_pDirectoryServicesWazooWndNew->CheckClosePrintPreview();
    }

	// Clean up MAPI DLLs
	if (GetIniShort(IDS_INI_MAPI_USE_NEVER) || GetIniShort(IDS_INI_MAPI_USE_EUDORA))
	{
		if (MAPIUninstall(TRUE) > CMapiInstaller::STATUS_NOT_INSTALLED)
			return FALSE;
	}

	//
	// Give user an opportunity to save Filters changes...
	// In reality, the CFiltersDoc::CanCloseDocument()
	// never returns FALSE, so the user won't get a
	// chance to cancel the main app shutdown.
	//
	if (!CanCloseFiltersFrame())
		return FALSE;

	//
	// Give user an opportunity to save Nicknames changes...
	// In reality, the CNicknamesDoc::CanCloseDocument()
	// never returns FALSE, so the user won't get a
	// chance to cancel the main app shutdown.
	//
	ASSERT(g_Nicknames);
	if (! g_Nicknames->CanCloseFrame(NULL))
		return FALSE;

	if (! QuerySendQueuedMessages()) 
		return FALSE; 
	if (! SaveOpenWindows(TRUE)) 
		return FALSE;
  	
	SaveBarState(_T("ToolBar"));

	m_WazooBarMgr.SaveWazooBarConfigToIni(); // saves docked and floating window *sizes*

	WriteToolBarMarkerToIni();

	return TRUE;
}

VOID CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == ID_SYSTEM_MENU_CHECKMAIL)
	{
		g_pApp->OnCheckMail();
	} else 
		CFrameWnd::OnSysCommand(nID,lParam);

	return;
}

void CMainFrame::OnHelp()
{
	// Does nothing on purpose.  CMDIFrame::OnContextHelp
	// calls this function to display default help because
	// WM_HELPHITTEST returned FALSE.  The reason it returned 
	// FALSE, however, is because CContextHelp  already 
	// handled the display of the help topic.  
 	return ;
}		

void CMainFrame::OnHelpTopics()
{
	WinHelp(0L,HELP_FINDER);
 	return ;
}

void CMainFrame::OnTechSupport()
{
	CString strUserCode = GetIniString(IDS_INI_USER_CODE);

	if(strUserCode.IsEmpty() || (strUserCode.CompareNoCase("NC") == 0))
		WinHelp(0x10000 + IDC_MHELP_NC);
	else if(strUserCode.CompareNoCase("IC") == 0)
		WinHelp(0x10000 + IDC_MHELP_IC);
	else if(strUserCode.CompareNoCase("EC") == 0)
		WinHelp(0x10000 + IDC_MHELP_EC);
	else if(strUserCode.CompareNoCase("EVAL") == 0)
		WinHelp(0x10000 + IDC_MHELP_EVAL);
	else if(strUserCode.CompareNoCase("OC") == 0)
		WinHelp(0x10000 + IDC_MHELP_OC);
	else
		WinHelp(0L,HELP_FINDER);
	return;
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Note that we use a raw ON_MESSAGE handler here rather than the
// ON_WM_CONTEXTMENU handler defined by CWnd.  The reason is that we
// actually route the message to the base class implementation, which
// may or may not handle the message.  If the base class doesn't handle
// the message, then we take care of it ourselves.
////////////////////////////////////////////////////////////////////////
long CMainFrame::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    // smohanty
    // If the DirectoryServices PrintPreview mode is on
    // just return FALSE, since we don't have a context menu.
    if (m_pDirectoryServicesWazooWndNew) {
	ASSERT_VALID(m_pDirectoryServicesWazooWndNew);
	ASSERT(::IsWindow(m_pDirectoryServicesWazooWndNew->m_hWnd));
	if (m_pDirectoryServicesWazooWndNew->IsPrintPreviewMode() == true) {
	    return(FALSE);
	}
    }    

	//
	// Check to see whether the user hit a toolbar, a dockable
	// window, or the status bar.
	//
	ASSERT(::IsWindow(HWND(wParam)));
	CWnd* pWnd = CWnd::FromHandlePermanent(HWND(wParam));
	CPoint ptScreen(LOWORD(lParam), HIWORD(lParam));
 
	SECControlBar* pControlBar = DYNAMIC_DOWNCAST(SECControlBar, pWnd);
	SECDockBar* pDockBar = DYNAMIC_DOWNCAST(SECDockBar, pWnd);
	if (pControlBar != NULL || pDockBar != NULL)
	{
		//
		// Yep.  User hit an SECControlBar object, so display
		// the context menu for toolbars, etc.
		//

		// use the QCCustomToolBar version -- it adds the user defined toolbars
		m_pToolBar->OnContextMenu( pWnd, ptScreen );
		return TRUE;
	}


	//
	// If we get this far, then we didn't hit a control bar thingie,
	// so give second crack to the Auto-Wazoo MDI task bar.
	//
	if (QCWorkbook::OnContextMenu(wParam, lParam))
		return TRUE;

	//
	// If we get this far, the user did not click over a SECDockBar
	// object or an Auto-Wazoo MDI task bar, so display the normal
	// "main" context menu at the given cursor coordinates.
	//

	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = QCLoadMenu(IDR_CONTEXT_POPUPS);
	if ( ! hMenu || ! menu.Attach( hMenu ) )
	   return FALSE;
	
	// SubMenu(0) is the popup for the nonclient area.
	CMenu* menuPopup = menu.GetSubMenu(0);

	// Copy list of open MDI child windows from "Window" menu to popup.
	char szText[256];
	UINT nID;
	BOOL bMaximized;
	
	bMaximized = FALSE;
	MDIGetActive( &bMaximized );
	
	CMenu*	pWindowMenu;	
	VERIFY ( pWindowMenu = GetMenu()->GetSubMenu( MAIN_MENU_WINDOW + ( bMaximized ? 1 : 0 ) ) );
	
	int nCount =  pWindowMenu->GetMenuItemCount();

	// Start at the beginning of the listing of open windows.
	for ( int i=MP_MDICHILDWND_LIST; i<nCount  ; i++)
	{
		// ignore separators (nID==0)
		if ( nID = pWindowMenu->GetMenuItemID(i) )
		{
			pWindowMenu->GetMenuString(i,szText,256,MF_BYPOSITION);
			menuPopup->AppendMenu(MF_STRING,nID,szText);
		}
	}

	CContextMenu::MatchCoordinatesToWindow(HWND(wParam), ptScreen);
	CContextMenu(menuPopup, ptScreen.x, ptScreen.y);

	menu.DestroyMenu();
	return TRUE;
} 


////////////////////////////////////////////////////////////////////////
// OnUpdateOvrIndicator [protected]
//
// UI handler for displaying toggle status of Insert key
//
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateOvrIndicator(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( g_bOverwriteMode );
}


////////////////////////////////////////////////////////////////////////
// OnUpdateControlBarMenu [protected]
//
// UI handler for checking/unchecking menu items on the SECControlBar 
// popup menu.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateControlBarMenu(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
//FORNOW	case ID_CTRLBAR_SHOW_MAILBOXES:
//FORNOW		{
//FORNOW			if (m_wndWazooBar.IsVisible())
//FORNOW				pCmdUI->SetCheck(1);
//FORNOW			else
//FORNOW				pCmdUI->SetCheck(0);
//FORNOW		}
//FORNOW		break;
	case ID_CTRLBAR_SHOW_TOOLBAR:
		{
			if (m_pToolBar->IsVisible())
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}
		break;
	case ID_CTRLBAR_SHOW_STATUSBAR:
		{
			if (m_wndStatusBar.IsVisible())
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}
		break;
	case ID_CUSTOMIZE_LINK:
		{
			// Shareware: Reduced feature set does not include customizable toolbar
			//
			// We shouldn't get here in that case because the menu item should
			// have been disabled.
			if (UsingFullFeatureSet())
			{
				pCmdUI->Enable(TRUE);
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}
}


//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// OnShowMboxBar [protected]
//FORNOW//
//FORNOW// Command handler for hiding/showing the global Wazoo control bar.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMainFrame::OnShowMboxBar()
//FORNOW{
//FORNOW	BOOL is_visible = m_wndWazooBar.IsVisible();
//FORNOW	if (!is_visible)
//FORNOW	{
//FORNOW		// Force a refresh of the mailbox tree contents,
//FORNOW		// before we redisplay it.
//FORNOW		BuildUserMenus();
//FORNOW	}
//FORNOW	ShowControlBar(&m_wndWazooBar, !is_visible, FALSE);
//FORNOW	m_wndWazooBar.ActivateWazooWindow(RUNTIME_CLASS(CMboxWazooWnd));
//FORNOW	RecalcLayout();
//FORNOW}
//FORNOW
//FORNOW
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// OnShowStationery [protected]
//FORNOW//
//FORNOW// Command handler for hiding/showing the global WazooBar containing
//FORNOW// the stationery manager window.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMainFrame::OnShowStationery()
//FORNOW{
//FORNOW	BOOL is_visible = m_wndWazooBar.IsVisible();
//FORNOW	if (!is_visible)
//FORNOW	{
//FORNOW		// Force a refresh of the mailbox tree contents,
//FORNOW		// before we redisplay it.
//FORNOW		BuildUserMenus();
//FORNOW	}
//FORNOW	ShowControlBar(&m_wndWazooBar, !is_visible, FALSE);
//FORNOW	m_wndWazooBar.ActivateWazooWindow(RUNTIME_CLASS(CStationeryWazooWnd));
//FORNOW	RecalcLayout();
//FORNOW}
//FORNOW
//FORNOW
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// OnShowFilterReport [protected]
//FORNOW//
//FORNOW// Command handler for hiding/showing the global WazooBar containing
//FORNOW// the filter report window.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMainFrame::OnShowFilterReport()
//FORNOW{
//FORNOW	BOOL is_visible = m_wndWazooBar2.IsVisible();
//FORNOW	ShowControlBar(&m_wndWazooBar2, !is_visible, FALSE);
//FORNOW	m_wndWazooBar2.ActivateWazooWindow(RUNTIME_CLASS(CFilterReportWazooWnd));
//FORNOW	RecalcLayout();
//FORNOW}
//FORNOW
//FORNOW
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// OnShowFileBrowser [protected]
//FORNOW//
//FORNOW// Command handler for hiding/showing the global WazooBar containing
//FORNOW// the file browser window.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMainFrame::OnShowFileBrowser()
//FORNOW{
//FORNOW	BOOL is_visible = m_wndWazooBar.IsVisible();
//FORNOW	ShowControlBar(&m_wndWazooBar, !is_visible, FALSE);
//FORNOW	m_wndWazooBar.ActivateWazooWindow(RUNTIME_CLASS(CFileBrowseWazooWnd));
//FORNOW	RecalcLayout();
//FORNOW}
//FORNOW
//FORNOW
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOW// OnShowLookup [protected]
//FORNOW//
//FORNOW// Command handler for hiding/showing the global WazooBar containing
//FORNOW// the lookup window.
//FORNOW////////////////////////////////////////////////////////////////////////
//FORNOWvoid CMainFrame::OnShowLookup()
//FORNOW{
//FORNOW	BOOL is_visible = m_wndWazooBar2.IsVisible();
//FORNOW	ShowControlBar(&m_wndWazooBar2, !is_visible, FALSE);
//FORNOW	m_wndWazooBar2.ActivateWazooWindow(RUNTIME_CLASS(CLookupWazooWnd));
//FORNOW	RecalcLayout();
//FORNOW}

////////////////////////////////////////////////////////////////////////
// IsToolbarVisible [PUBLIC]
//
// Returns TRUE is the main toolbar is visible, FALSE otherwise
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::IsToolbarVisible() const
{
	ASSERT(m_pToolBar);
	return (m_pToolBar->IsVisible());
}

////////////////////////////////////////////////////////////////////////
// IsStatbarVisible [PUBLIC]
//
// Returns TRUE is the status bar is visible, FALSE otherwise
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::IsStatbarVisible() const
{
	return (m_wndStatusBar.IsVisible());
}

////////////////////////////////////////////////////////////////////////
// OnShowToolBar [protected]
//
// Command handler for hiding/showing the global tool bar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnShowToolBar()
{
	// toggle current visibility
	BOOL is_visible = m_pToolBar->IsVisible();
	ShowControlBar(m_pToolBar, !is_visible, FALSE);
	SetIniShort(IDS_INI_SHOW_TOOLBAR, !is_visible);
	RecalcLayout();
}

////////////////////////////////////////////////////////////////////////
// OnCustomizeLink [protected]
//
// Command handler for displaying toolbar Property Sheets.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnCustomizeLink()
{
	// Shareware: Only allow customization of toolbar in FULL FEATURE version.
	if (UsingFullFeatureSet())
	{
		( ( QCToolBarManager * ) m_pControlBarManager ) ->OnCustomize();
	}
}

//Dont have a clue about this, so we'll comment it out for now
//void CMainFrame::OnProperties()
//{
//	( ( QCToolBarManager * ) m_pControlBarManager ) ->OnCustomize();
//}


////////////////////////////////////////////////////////////////////////
// OnShowStatusBar [protected]
//
// Command handler for hiding/showing the global status bar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnShowStatusBar()
{
	// toggle current visibility
	BOOL is_visible = m_wndStatusBar.IsVisible();
	ShowControlBar(&m_wndStatusBar, !is_visible, FALSE);
	SetIniShort(IDS_INI_SHOW_STATUS_BAR, !is_visible);
	RecalcLayout();
}




LRESULT CMainFrame::OnUserCheckMail(WPARAM wParam, LPARAM lParam)
{            
	// Allows others apps to tell Eudora to check mail
    
    BOOL bWaitForIdle = LOWORD(wParam);
    
 	((CEudoraApp *) AfxGetApp())->SetExternalCheckMail(bWaitForIdle);
 	
 	return 0;

}


////////////////////////////////////////////////////////////////////////
// OnUserRegisterMailto [protected]
//
// Command handler for user-defined message, WM_USER_REGISTER_MAILTO.
// The idea is to register Eudora as Netscape's <mailto:> URL handler.
////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnUserRegisterMailto(WPARAM wParam, LPARAM lParam)
{            
	if (GetIniShort(IDS_INI_NETSCAPE_MAILTO))
	{
		// Register Eudora as Netscape's mailto handler
		CDDEClient dde_client;
		dde_client.RegisterNetscapeProtocol("mailto");
	}

 	return 0;
}

#ifdef EXPIRING
////////////////////////////////////////////////////////////////////////
// OnUserEvalExpired [protected]
//
// Command handler for user-defined message, WM_USER_EVAL_EXPIRED.
////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnUserEvalExpired(WPARAM wParam, LPARAM lParam)
{            
	WarnDialog(0, IDS_TIME_BOMB_MESSAGE_BETA);

 	return 0;
}

#endif // EXPIRING

////////////////////////////////////////////////////////////////////////
// OnUserAutomationMsg [protected]
//
// Command handler for user-defined message, WM_USER_AUTOMATION_MSG.
////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnUserAutomationMsg(WPARAM wParam, LPARAM lParam)
{            
	g_theAutomationDirector.OnMessage(wParam, lParam);
	return 0;
}

///////////////////////////////////////////////////////////////////////
// OnEditable [protected]
//
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnEditable(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	CWnd *focusWnd = CWnd::GetFocus();

	if ( focusWnd )
	{
		if( focusWnd->IsKindOf( RUNTIME_CLASS( CRichEditView ) ) )
		{
			enable = TRUE;
		}
		else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CEditView ) ) )
		{
			enable = TRUE;
		}
		else if( focusWnd->IsKindOf( RUNTIME_CLASS( CRichEditCtrl ) ) )
		{
			enable = TRUE;
		}
		else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CEdit ) ) )
		{
			enable = TRUE;
		}
	}

	pCmdUI->Enable(enable);
}

////////////////////////////////////////////////////////////////////////
// OnAboutEMSPlugins 
//
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnAboutEmsPlugins()
{
	CAboutPlugInDlg aboutEMSDlg;
	aboutEMSDlg.DoModal();
}

////////////////////////////////////////////////////////////////////////
// CheckSpelling (on any edit text)
void CMainFrame::CheckSpelling()
{
#ifdef COMMERCIAL

	CWnd *focusWnd = CWnd::GetFocus();

	if (!focusWnd)
		return;


	CSpell Spell(TRUE);

	if( focusWnd->IsKindOf( RUNTIME_CLASS( CRichEditCtrl ) ) )
	{
		Spell.Check((CRichEditCtrl *)focusWnd);
	}
	else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CEdit ) ) )
	{
		Spell.Check((CEdit *)focusWnd);
	}	
	else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CRichEditView ) ) )
	{
		CRichEditCtrl& theCtrl = ( ( CRichEditView* ) focusWnd )->GetRichEditCtrl();
		Spell.Check( &theCtrl );
	}
	else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CEdit ) ) )
	{
		CEdit& theCtrl = ( ( CEditView* ) focusWnd )->GetEditCtrl();
		Spell.Check( &theCtrl );
	}

#endif // COMMERCIAL

	return;
}


////////////////////////////////////////////////////////////////////////
// AttachFile : make's sure there's a message to attach to 
//
// Note that this only gets called if there is currently no message
// to which we can attach a file; therefore, we create a new one.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnAttachFile()
{   
	// make a new comp window
	NewMessage(NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	// grab it, validate the hell out of it, and tell it what we want
	CCompMessageFrame* pMF = (CCompMessageFrame*) GetActiveFrame();
	VERIFY( pMF );
	BOOL isCompFrame = pMF->IsKindOf( RUNTIME_CLASS(CCompMessageFrame) );
	ASSERT( isCompFrame );
	if ( isCompFrame )
		pMF->OnAttachFile();

	return;
}

////////////////////////////////////////////////////////////////////////
// OnRefreshTaskStatusPane [protected]
//
// Command handler for user-defined message when Task Status pane in
// status bar needs to be redrawn (usually when bitmap changes).
////////////////////////////////////////////////////////////////////////
long CMainFrame::OnRefreshTaskStatusPane(WPARAM wParam, LPARAM lParam)
{
	CRect rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_wndStatusBar.InvalidateRect(&rect, TRUE);

	return 0;
}

void CMainFrame::OnPurchaseInfo()
{
	LaunchURL(CRString(IDS_EUDORA_BUY_OFFER_URL));
}


/////////////////////////////////////////////////////////////////////
//
// OnNewMailboxInRoot 
// 
// Creates a new mailbox in the root directory.  This is actually a work
// around for the standard new mailbox procedure.  Unlike the sub folder 
// versions of New..., there is no folder command objected to associate this 
// with. 
void CMainFrame::OnNewMailboxInRoot()
{
	g_theMailboxDirector.CreateTargetMailbox( NULL, FALSE );
}


void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
	CString	szTrace;
	CString szFlags;

	if( nFlags == 0xFFFF )
	{
		szFlags = "0xFFFF";
	}
	else
	{
		szFlags = "";
	
		if ( nFlags & MF_POPUP )
		{
			szFlags += "MF_POPUP";
		}
		
		if ( nFlags & MF_HILITE )
		{
			if( szFlags != "" )
			{
				szFlags += " | ";
			}

			szFlags += "MF_HILITE";
		}

		if ( nFlags & MF_DISABLED )
		{
			if( szFlags != "" )
			{
				szFlags += " | ";
			}

			szFlags += "MF_DISABLED";
		}

		if ( nFlags & MF_SYSMENU )
		{
			if( szFlags != "" )
			{
				szFlags += " | ";
			}

			szFlags += "MF_SYSMENU";
		}
	}

	szTrace.Format( "nItemID = 0x%x, nFlags = %s, hSysMenu = 0x%x\n", nItemID, szFlags, hSysMenu );
//	TRACE( szTrace );

	QCWorkbook::OnMenuSelect(nItemID, nFlags, hSysMenu);
	
	
	if ( ( ( nFlags & MF_HILITE ) != 0 ) && 
		 ( ( nFlags & MF_DISABLED ) == 0 ) &&
		 ( ( nFlags & MF_POPUP ) == 0 ) &&
		 ( nItemID <= QC_LAST_COMMAND_ID ) && 
		 ( nItemID >= QC_FIRST_COMMAND_ID ) )
	{
		g_theCommandStack.SaveCommand( ( WORD ) nItemID );
	}			
}



LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;
	UINT				uID;

	if( ( wParam <= QC_LAST_COMMAND_ID ) && 
		( wParam >= QC_FIRST_COMMAND_ID ) )
	{
		if( g_theCommandStack.Lookup( ( WORD ) wParam, &pCommand, &theAction ) )
		{
			uID = pCommand->GetFlyByID( theAction );

			if( uID )
			{
				wParam = uID;
			}
		}
	}

	return (QCWorkbook::OnSetMessageString(wParam, lParam));
}



BOOL CMainFrame::OnDynamicCommand(
UINT uID )
{
	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	
	CCompMessageDoc*		pDocument;
	struct TRANSLATE_DATA	theData;

	if( ! g_theCommandStack.GetCommand( ( WORD ) uID, &pCommand, &theAction ) )
	{		
		return FALSE;
	}

	if( ( pCommand == NULL ) || !theAction )
	{
		return FALSE;
	}

	if( theAction == CA_NEW_MAILBOX )
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );

		g_theMailboxDirector.CreateTargetMailbox( (QCMailboxCommand *) pCommand, FALSE );
		return TRUE;
	}

	if( theAction == CA_ATTACH_PLUGIN )
	{
		NewMessage(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		pDocument = (CCompMessageDoc*)GetActiveFrame()->GetActiveDocument();

		ASSERT( pDocument );
		
		if( pDocument != NULL )
		{
			pCommand->Execute( theAction, pDocument );
		}
		
		return TRUE;
	}

	if( theAction == CA_TRANSLATE_PLUGIN )
	{
		theData.m_pView = GetActiveFrame()->GetActiveView();
		theData.m_bBuildAddresses = FALSE;
		
		pCommand->Execute( theAction, &theData );

		return TRUE;

	}

	if( theAction == CA_INSERT_RECIPIENT )
	{
		ASSERT_KINDOF( QCRecipientCommand, pCommand );
		CString Name = ( ( QCRecipientCommand* ) pCommand )->GetName();

		pCommand->Execute( theAction, &Name );
		return TRUE;
	}

	pCommand->Execute( theAction );
	return TRUE;
}


void CMainFrame::OnUpdateDynamicCommand(
CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	
	CView*				pView;
	CWnd*				pFocusWnd;
	 			
	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( theAction == CA_INSERT_RECIPIENT )
			{
				if ( CanModifyEdit() )
					pCmdUI->Enable( TRUE );
				else
					pCmdUI->Enable( FALSE );
				return;
			}

			if( theAction == CA_TRANSLATE_PLUGIN )
			{
				// Translators can work on any edit view
				pView = GetActiveFrame()->GetActiveView();
				
				if( pView )
				{
					pFocusWnd = pView->GetFocus();
					
					if( pFocusWnd )
					{
						if(	pView->IsKindOf( RUNTIME_CLASS( CRichEditView ) ) || 
							pFocusWnd->IsKindOf( RUNTIME_CLASS( CRichEditCtrl) ) ||
							pView->IsKindOf( RUNTIME_CLASS( CEditView ) ) ||
							pFocusWnd->IsKindOf( RUNTIME_CLASS( CEdit ) ) )
						{
							pCmdUI->Enable( TRUE );
							return;
						}
					}
				}
			}
			else if(( theAction == CA_NEW_MESSAGE_WITH ) ||
					( theAction == CA_NEW_MAILBOX ) ||
					( theAction == CA_NEW_MESSAGE_TO ) ||
					( theAction == CA_ATTACH_PLUGIN ) ||
					( theAction == CA_SPECIAL_PLUGIN ) ||
					( ( ( theAction == CA_OPEN ) || ( theAction == CA_DELETE ) ) && pCommand->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) ) ||
					( ( theAction == CA_OPEN ) && pCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) ) ||
					( ( theAction == CA_OPEN ) && pCommand->IsKindOf( RUNTIME_CLASS( QCStationeryCommand ) ) ) ||
					( ( theAction == CA_DELETE ) && pCommand->IsKindOf( RUNTIME_CLASS( QCRecipientCommand ) ) ) ||
					( ( theAction == CA_FILTER_TRANSFER ) || (theAction == CA_FILTER_TRANSFER_NEW ) ) )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->Enable( FALSE );
}


void CMainFrame::InitMenus()
{
	CMenu*	pMenu;
	CMenu*	pSubMenu;
	UINT	uCount;
	BOOL	bMaximized;
	UINT	uOffset;

	bMaximized = FALSE;
	MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );

	// Edit menu
	VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_EDIT + uOffset ) );

	//Edit->Insert Recipients SubMenu
	VERIFY( pSubMenu = pMenu->GetSubMenu(POS_EDIT_INSERTRECIPIENT_MENU));
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theRecipientDirector.NewMessageCommands( CA_INSERT_RECIPIENT, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );

	VERIFY( pSubMenu = pMenu->GetSubMenu( pMenu->GetMenuItemCount() - 1 ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_thePluginDirector.NewMessageCommands( CA_TRANSLATE_PLUGIN, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );


	// We allow the user to swap the Ctrl-F and Shif-Ctrl-F functionality. We handle the action
	// separately from the command, so here we should init the menu to reflect what command
	// is what accelerator.
	//
	// Default is Ctrl-F maps to Find Msgs, Shift-Ctrl-F maps to Find Text.
	//
	if (m_bSwitchFindAccel)
	{
		CString sFindMsg, sFindText;
		VERIFY(pMenu->GetMenuString(ID_EDIT_FIND_FINDMSG, sFindMsg, MF_BYCOMMAND) > 0);
		VERIFY(pMenu->GetMenuString(ID_EDIT_FIND_FINDTEXT, sFindText, MF_BYCOMMAND) > 0);

		CString sFindMsgPrefix, sFindMsgSuffix, sFindTextPrefix, sFindTextSuffix;

		int pos = (-1);
		
		// We want to swap the accel info, which is separated from the menu text by a tab character.
		// So, find the tab in each menu string and swap everything after (and including) the tab.

		// Parse the Find Msg menu string
		pos = sFindMsg.Find('\t');
		if (pos < 0)
		{
			sFindMsgPrefix = sFindMsg;
			sFindMsgSuffix.Empty();
		}
		else
		{
			sFindMsgPrefix = sFindMsg.Left(pos);
			sFindMsgSuffix = sFindMsg.Right(sFindMsg.GetLength() - pos);
		}

		// Parse the Find Text menu string
		pos = sFindText.Find('\t');
		if (pos < 0)
		{
			sFindTextPrefix = sFindText;
			sFindTextSuffix.Empty();
		}
		else
		{
			sFindTextPrefix = sFindText.Left(pos);
			sFindTextSuffix = sFindText.Right(sFindText.GetLength() - pos);
		}

		sFindMsg = sFindMsgPrefix + sFindTextSuffix;
		sFindText = sFindTextPrefix + sFindMsgSuffix;

		VERIFY(pMenu->ModifyMenu(ID_EDIT_FIND_FINDMSG, MF_BYCOMMAND | MF_STRING, ID_EDIT_FIND_FINDMSG, (LPCSTR) sFindMsg));
		VERIFY(pMenu->ModifyMenu(ID_EDIT_FIND_FINDTEXT, MF_BYCOMMAND | MF_STRING, ID_EDIT_FIND_FINDTEXT, (LPCSTR) sFindText));
	}

	// START MESSAGE MENU
	// Message menu
	VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset ) );

	//Menu popup  --> New Message to
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_NEW_MESSAGE_TO ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theRecipientDirector.NewMessageCommands( CA_NEW_MESSAGE_TO, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );
	
	//Menu popup  --> Forward to
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_FORWARD_TO ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theRecipientDirector.NewMessageCommands( CA_FORWARD_TO, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );

	//Menu popup  --> Redirect To
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_REDIRECT_TO ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theRecipientDirector.NewMessageCommands( CA_REDIRECT_TO, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );
 

#ifdef COMMERCIAL //no stationery allowed in 4.1 Light

	//Menu popup  --> New Message With
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_NEW_MESSAGE_WITH ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theStationeryDirector.NewMessageCommands( CA_NEW_MESSAGE_WITH, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );

	//Menu popup  --> Reply With
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_REPLY_WITH ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theStationeryDirector.NewMessageCommands( CA_REPLY_WITH, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );

	//Menu popup  --> Reply to All With
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_REPLY_TO_ALL_WITH ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	g_theStationeryDirector.NewMessageCommands( CA_REPLY_TO_ALL_WITH, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );

#endif // COMMERCIAL
	
	//Menu popup  --> Attach
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_ATTACH ) );

	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	
	g_thePluginDirector.NewMessageCommands( CA_ATTACH_PLUGIN, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );

	//Menu popup  --> Change
	VERIFY( pSubMenu = pMenu->GetSubMenu( POS_CHANGE ) );
	{
		//
		// Update items on the Edit:Change submenu.  First, add bitmaps
		// to the Edit:Change:Status menu items.  Second, add bitmaps
		// to the Edit:Change:Priority menu items.  Third, populate the
		// Edit:Change:Persona menu.
		//
		CMenu* pSubSubMenu = pSubMenu->GetSubMenu( 1 );		// Edit:Change:Status
		ASSERT(g_StatusUnreadBitmap.GetSafeHandle());				// bitmaps should be loaded before we get here
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_UNREAD,      MF_BYCOMMAND, &g_StatusUnreadBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_READ,        MF_BYCOMMAND, &g_StatusReadBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_REPLIED,     MF_BYCOMMAND, &g_StatusRepliedBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_FORWARDED,   MF_BYCOMMAND, &g_StatusForwardedBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_REDIRECTED,  MF_BYCOMMAND, &g_StatusRedirectBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_UNSENDABLE,  MF_BYCOMMAND, &g_StatusUnsendableBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_SENDABLE,    MF_BYCOMMAND, &g_StatusSendableBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_QUEUED,      MF_BYCOMMAND, &g_StatusQueuedBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_SENT,        MF_BYCOMMAND, &g_StatusSentBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_UNSENT,      MF_BYCOMMAND, &g_StatusUnsentBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_TIME_QUEUED, MF_BYCOMMAND, &g_StatusTimeQueuedBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_STATUS_RECOVERED, MF_BYCOMMAND, &g_StatusRecoveredBitmap, NULL);

		pSubSubMenu = pSubMenu->GetSubMenu( 2 );			// Edit:Change:Priority
		ASSERT(g_PriorityHighestMenuBitmap.GetSafeHandle());		// bitmaps should be loaded before we get here
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_HIGHEST, MF_BYCOMMAND, &g_PriorityHighestMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_HIGH,    MF_BYCOMMAND, &g_PriorityHighMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_NORMAL,  MF_BYCOMMAND, &g_PriorityNormalMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_LOW,     MF_BYCOMMAND, &g_PriorityLowMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_LOWEST,  MF_BYCOMMAND, &g_PriorityLowestMenuBitmap, NULL);

		pSubSubMenu = pSubMenu->GetSubMenu( 4 );			// Edit:Change:Server Status
		ASSERT(g_ServerLeaveBitmap.GetSafeHandle());				// bitmaps should be loaded before we get here
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_LEAVE,        MF_BYCOMMAND, &g_ServerLeaveBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_FETCH,        MF_BYCOMMAND, &g_ServerFetchBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_DELETE,       MF_BYCOMMAND, &g_ServerDeleteBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_FETCH_DELETE, MF_BYCOMMAND, &g_ServerFetchDeleteBitmap, NULL);

//Allow only one personality in 4.1 Light
#ifdef COMMERCIAL		
		pSubSubMenu = pSubMenu->GetSubMenu( 5 );			// Edit:Change:Persona
		VERIFY( pSubSubMenu );

		if( ( pSubSubMenu->GetMenuItemCount() > 0 ) && 
			( pSubSubMenu->GetMenuItemID(0) == 0 ) )
		{
			pSubSubMenu->RemoveMenu( 0, MF_BYPOSITION );
		}
		
		g_thePersonalityDirector.NewMessageCommands( CA_CHANGE_PERSONA, pSubSubMenu );
		::WrapMenu( pSubSubMenu->GetSafeHmenu() );
#endif // COMMERCIAL
	}

	// END MESSAGE EMNU

	
	
	// Mailbox menu
	VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MAILBOX + uOffset ) );
	if( ( pMenu->GetMenuItemCount() > 0 ) && 
		( pMenu->GetMenuItemID(0) == 0 ) )
	{
		pMenu->RemoveMenu( 0, MF_BYPOSITION );
	}

	g_theMailboxDirector.NewMessageCommands( CA_OPEN, pMenu, CA_NEW_MAILBOX );
	::WrapMenu( pMenu->GetSafeHmenu() );
	
	// add the new mailbox in root entry
	pMenu->InsertMenu( 3, MF_BYPOSITION, ID_NEW_MAILBOX_IN_ROOT, CRString( IDS_MAILBOX_NEW ) );
	// add the separator
	pMenu->InsertMenu( 3, MF_BYPOSITION | MF_SEPARATOR );

	// Transfer menu
	VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_TRANSFER + uOffset ) );
	if( ( pMenu->GetMenuItemCount() > 0 ) && 
		( pMenu->GetMenuItemID(0) == 0 ) )
	{
		pMenu->RemoveMenu( 0, MF_BYPOSITION );
	}

	g_theMailboxDirector.NewMessageCommands( CA_TRANSFER_TO, pMenu, CA_TRANSFER_NEW );	
	::WrapMenu( pMenu->GetSafeHmenu() );
	
	// add the new mailbox in root entry
	pMenu->InsertMenu( 3, MF_BYPOSITION, ID_TRANSFER_NEW_MBOX_IN_ROOT, CRString( IDS_MAILBOX_NEW ) );
	// add the separator
	pMenu->InsertMenu( 3, MF_BYPOSITION | MF_SEPARATOR );

	// Special menu
	VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_SPECIAL + uOffset ) );

	VERIFY( pSubMenu = pMenu->GetSubMenu( 5 ) );
	if( ( pSubMenu->GetMenuItemCount() > 0 ) && 
		( pSubMenu->GetMenuItemID(0) == 0 ) )
	{
		pSubMenu->RemoveMenu( 0, MF_BYPOSITION );
	}
	
	g_theRecipientDirector.NewMessageCommands( CA_DELETE, pSubMenu );
	::WrapMenu( pSubMenu->GetSafeHmenu() );
	
	// Tools menu
	VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_TOOLS + uOffset ) );

	// add the special plugins to the tools menu
	
	// first, save the menu count
	uCount = pMenu->GetMenuItemCount();
	
	// add the items, assuming that the last item is the Tools:Options
	// item and that the next to last item is a separator
	g_thePluginDirector.NewMessageCommands( CA_SPECIAL_PLUGIN, pMenu, uCount - 1 );
	
	// check the count again
	if (pMenu->GetMenuItemCount() - uCount)
	{
		// if things were added, add a separator
		pMenu->InsertMenu( pMenu->GetMenuItemCount() - 1, MF_BYPOSITION | MF_SEPARATOR );
	}

	::WrapMenu( pMenu->GetSafeHmenu() );

	// register as a command client
	g_theMailboxDirector.Register( this );
	g_theRecipientDirector.Register( this );
	g_theStationeryDirector.Register( this );
	g_theSignatureDirector.Register( this );
	g_thePersonalityDirector.Register( this );
}



void CMainFrame::Notify(
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction,
void*				pData)
{
	CMenu*				pMenu;
	CMenu*				pSubMenu;
	CMenu				theSubMenu;
	UINT				uID;
	CString				szName;
	CString				szNamedPath;
	CString				szCurrentPath;
	INT					i;
	QCMailboxCommand*	pCommand;
	UnreadStatusType	theStatus;
	UINT				uOffset;
	BOOL				bMaximized;

	bMaximized = FALSE;
	MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );

	if (theAction == CA_SWM_CHANGE_FEATURE) 
	{
		SWMNotify(pObject, theAction, pData);
	}

	if( ( theAction != CA_DELETE ) &&
		( theAction != CA_NEW ) &&
		( theAction != CA_GRAFT ) &&
		( theAction != CA_UPDATE_STATUS ) && 
		( theAction != CA_RENAME ) )
	{
		return;
	}

	if( pObject->IsKindOf( RUNTIME_CLASS( QCRecipientCommand ) ) )
	{
		szName = ( ( QCRecipientCommand* ) pObject )->GetName();
		
		if( ( theAction == CA_DELETE ) || ( theAction == CA_RENAME ) )
		{
			// delete the old name from the Insert Recipient Menu

			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_EDIT + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu(POS_EDIT_INSERTRECIPIENT_MENU));

			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID), CA_INSERT_RECIPIENT );
			}

			// delete the old name from the New Message To menu

			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( POS_NEW_MESSAGE_TO ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID), CA_NEW_MESSAGE_TO );
			}

			// delete the old name from the Forward To menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( POS_FORWARD_TO ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID), CA_FORWARD_TO );
			}

			// delete the old name from the Redirect To menu
			VERIFY( pSubMenu = pMenu->GetSubMenu(POS_REDIRECT_TO) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID), CA_REDIRECT_TO );
			}

			// delete the old name from Remove Recipient menu

			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_SPECIAL + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( 5 ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID), CA_DELETE );
			}

			return;
		}

		if( theAction == CA_NEW )
		{
			// add the name to the Edit- Insert Recipient menu
			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_EDIT + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu(POS_EDIT_INSERTRECIPIENT_MENU));
			uID = g_theCommandStack.AddCommand( pObject, CA_INSERT_RECIPIENT );
			::AddMenuItem( uID, szName, pSubMenu );

			// add the name to the New Message To menu
			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( 7 ) );			
			uID = g_theCommandStack.AddCommand( pObject, CA_NEW_MESSAGE_TO );
			::AddMenuItem( uID, szName, pSubMenu );

			
			// add the name to the Forward To menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 8 ) );
			uID = g_theCommandStack.AddCommand( pObject, CA_FORWARD_TO );
			::AddMenuItem( uID, szName, pSubMenu );

			// add the name to the Redirect To menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 9 ) );
			uID = g_theCommandStack.AddCommand( pObject, CA_REDIRECT_TO );
			::AddMenuItem( uID, szName, pSubMenu );

			// add the name to the Remove Recipient menu
			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_SPECIAL + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( 5 ) );
			uID = g_theCommandStack.AddCommand( pObject, CA_DELETE );
			::AddMenuItem( uID, szName, pSubMenu );
			return;
		}
	}
	else
	if( pObject->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) )
	{
		VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_TOOLS + uOffset ) );

		szName = ( ( QCSignatureCommand* ) pObject )->GetName();
		
/*		if( ( theAction == CA_DELETE ) || ( theAction == CA_RENAME ) )
		{
			// delete the old name from the Signatures menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( pMenu->GetMenuItemCount() - 4 ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID),  CA_OPEN );
			}

			return;
		}

		if( theAction == CA_NEW )
		{
			// add the name to the signatures menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( pMenu->GetMenuItemCount() - 4 ) );			
			uID = g_theCommandStack.AddCommand( pObject, CA_OPEN );
			::AddMenuItem( uID, szName, pSubMenu );

			return;
		}
*/	}
	else
	if( pObject->IsKindOf( RUNTIME_CLASS( QCStationeryCommand ) ) )
	{

#ifdef COMMERCIAL
		VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset ) );

		szName = ( ( QCStationeryCommand* ) pObject )->GetName();
		
		if( ( theAction == CA_DELETE ) || ( theAction == CA_RENAME ) )
		{
			// delete the old name from the New Message With menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 11 ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID),  CA_NEW_MESSAGE_WITH );
			}

			// delete the old name from the Reply With menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 12 ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID),  CA_REPLY_WITH );
			}

			// delete the old name from the Reply to All With menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 13 ) );
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID),  CA_REPLY_TO_ALL_WITH );
			}

			// delete the old name from the Stationery menu
/*			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_TOOLS + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( pMenu->GetMenuItemCount() - 3 ) );
			
			uID = ::RemoveMenuItem( szName, pSubMenu );

			if( theAction == CA_RENAME )
			{
				::AddMenuItem( uID, (LPCSTR) pData, pSubMenu );
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID),  CA_OPEN );
			}
*/
			return;
		}

		if( theAction == CA_NEW )
		{

			char szNewName[255];
			//Bug 2760.  If '&' is used in a name it is underlined in the menu..this call fixes this
			//Will insert another '&' so it will appear correctly
			GenerateNewName(szName, szNewName);

			// add the name to the New Message With menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 11 ) );			
			uID = g_theCommandStack.AddCommand( pObject, CA_NEW_MESSAGE_WITH );
			::AddMenuItem( uID, szNewName, pSubMenu );

			// add the name to the Reply With menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 12 ) );			
			uID = g_theCommandStack.AddCommand( pObject, CA_REPLY_WITH );
			::AddMenuItem( uID, szNewName, pSubMenu );

			// add the name to the Reply To All With menu
			VERIFY( pSubMenu = pMenu->GetSubMenu( 13 ) );			
			uID = g_theCommandStack.AddCommand( pObject, CA_REPLY_TO_ALL_WITH );
			::AddMenuItem( uID, szNewName, pSubMenu );

			// add the name to the stationery menu
/*			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_TOOLS + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( pMenu->GetMenuItemCount() - 3 ) );			
			uID = g_theCommandStack.AddCommand( pObject, CA_OPEN );
			::AddMenuItem( uID, szName, pSubMenu );*/

			return;
		}
#endif // COMMERCIAL
	}
	else
	if( pObject->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
	{

		if( theAction == CA_UPDATE_STATUS )
		{	
			// I HATE having to do this here, but OnUpdate... doesn't
			// get called for folder items :-(
					
			pCommand = ( QCMailboxCommand* ) pObject;
			// find the current menu entry
			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MAILBOX + uOffset ) );
			
			szNamedPath = g_theMailboxDirector.BuildNamedPath( pCommand );

//  IMAP4 // Need to handle IMAP types.
			if (pCommand->IsImapType())
			{
				pMenu = ::ImapFindNamedMenuItem( szNamedPath, pMenu, &i, 
										((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(FALSE));
			}
			else
			{
				pMenu = ::FindNamedMenuItem( szNamedPath, pMenu, &i, ( pCommand->GetType() == MBT_FOLDER ) );
			}

			// on startup menu may not be ready yet
			if (!pMenu) return;

			theStatus = ( UnreadStatusType ) ( ULONG ) pData;
			pMenu->CheckMenuItem( i, MF_BYPOSITION | ( ( theStatus == US_YES ) ? MF_CHECKED : MF_UNCHECKED ) );

			return;
		}

		// handle the mailboxes menu
		VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MAILBOX + uOffset ) );
		HandleMailboxNotification( pMenu, CA_OPEN, ( QCMailboxCommand* ) pObject, theAction, pData );

		// handle the transfer menu
		VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_TRANSFER + uOffset ) );
		HandleMailboxNotification( pMenu, CA_TRANSFER_TO, ( QCMailboxCommand* ) pObject, theAction, pData );
	}
	else
	if( pObject->IsKindOf( RUNTIME_CLASS( QCPersonalityCommand ) ) )
	{
		szName = ( ( QCPersonalityCommand* ) pObject )->GetName();
		
		if( theAction == CA_DELETE )
		{
			// delete the old name from the Change Personality Menu

			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( 18 ) );		// should be the Message:Change submenu
			{
				CMenu* pSubSubMenu = pSubMenu->GetSubMenu( 5 );
				VERIFY( pSubSubMenu );
				uID = ::RemoveMenuItem( szName, pSubSubMenu );
				g_theCommandStack.DeleteCommand( WORD(uID), CA_CHANGE_PERSONA );
			}

			return;
		}

		if( theAction == CA_NEW )
		{
			VERIFY( pMenu = GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset ) );
			VERIFY( pSubMenu = pMenu->GetSubMenu( 18 ) );
			{
				CMenu* pSubSubMenu = pSubMenu->GetSubMenu( 5 );
				VERIFY( pSubSubMenu );
				uID = g_theCommandStack.AddCommand( pObject, CA_CHANGE_PERSONA );
				::AddMenuItem( uID, szName, pSubSubMenu );
			}
			return;
		}
	}
}


void CMainFrame::AddMailboxMenuItem(
UINT	uID,
LPCSTR	szMenuItem,
CMenu*	pMenu,
CMenu*	pSubMenu,
INT		iStartPos,
INT		iEndPos,
BOOL	bTransferMenu )
{
	UINT	uPos;

	uPos = ::AddMenuItem( uID, szMenuItem, pMenu, pSubMenu, iStartPos, iEndPos );

	if( bTransferMenu )
	{
		pMenu->SetMenuItemBitmaps( uPos, 
										MF_BYPOSITION, 
										&( g_theMailboxDirector.GetTransferBitmap() ), 
										NULL );
	}
	else
	{
		pMenu->SetMenuItemBitmaps( uPos, 
										MF_BYPOSITION, 
										NULL,
										&( g_theMailboxDirector.GetUnreadStatusBitmap() ) );
	}
}

void CMainFrame::HandleMailboxNotification(
CMenu*				pMenu,
COMMAND_ACTION_TYPE theNewAction,
QCMailboxCommand*	pCommand,
COMMAND_ACTION_TYPE	theAction,
void*				pData)
{
	CMenu*	pSubMenu;
	CMenu	theSubMenu;
	UINT	uID;
	CString	szName;
	CString	szNamedPath;
	CString	szCurrentPath;
	INT		iMailboxPos;
	UINT	uMenuIndex;
	BOOL	bIsXferMenu;
	INT		iFolderPos;
	INT		i;
	UINT	uOffset;
	BOOL	bMaximized;

	bMaximized = FALSE;
	MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );


	if( theNewAction == CA_TRANSFER_TO ) 
	{
		uMenuIndex = MAIN_MENU_TRANSFER + uOffset;
		bIsXferMenu = TRUE;
	}
	else
	{
		uMenuIndex = MAIN_MENU_MAILBOX + uOffset;
		bIsXferMenu = FALSE;
	}

	if( ( theAction == CA_DELETE ) || 
		( theAction == CA_RENAME ) || 
		( theAction == CA_GRAFT ) )
	{
		// find the current menu entry
		szNamedPath = g_theMailboxDirector.BuildNamedPath( pCommand );

		if (pCommand->IsImapType())
		{
			pSubMenu = ::ImapFindNamedMenuItem( szNamedPath, pMenu, &i, 
									((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu));
		}
		else
		{
			pSubMenu = ::FindNamedMenuItem( szNamedPath, pMenu, &i, ( pCommand->GetType() == MBT_FOLDER ) );
		}

		if( pSubMenu == NULL )
		{
			// this should happen ONLY if you exceed MaxMenus!!!
			ASSERT( 0 );
			return;
		}

		uID = pSubMenu->GetMenuItemID( i );
		
// IMAP4  // Account for IMAP types.
		if( ( pCommand->GetType() == MBT_FOLDER ) ||
			( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu) ) )
		{
			if( ( INT ) uID != -1 )
			{
				// this should never happen
				ASSERT( 0 );
				return;
			}
			
			pMenu = pSubMenu->GetSubMenu( i );
		}
		else
		{
			ASSERT( ( INT ) uID != -1 );
		}

		// finally, remove the item
		pSubMenu->RemoveMenu( i, MF_BYPOSITION );

		// If we're deleting an item corresponding to an IMAP ACCOUNT object, we may need to delete
		// a following separator item.
		//
		if (pCommand->GetType() == MBT_IMAP_ACCOUNT)
		{
			// Note: "i" should now point to the separator item (if there's one).
			// Separator items have ID's of zero.
			//
			if ( ((UINT)i < pSubMenu->GetMenuItemCount()) && (pSubMenu->GetMenuItemID( i ) == 0) )
			{
				pSubMenu->RemoveMenu( i, MF_BYPOSITION );
			}
		}

		if( theAction == CA_DELETE )
		{
			// unwrap if necessary
			WrapMenu( pSubMenu->GetSafeHmenu() );

// IMAP4
			if( ( pCommand->GetType() == MBT_FOLDER ) ||
				( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu) ) )
			{
				// walk the menu and delete all the commands 
				// actually, this is just a precaution -- the menu should 
				// be empty by the time we get here because the recursion
				// processes all mailboxes before folders.

				g_theCommandStack.DeleteAllMenuCommands( pMenu, theNewAction );
				g_theCommandStack.DeleteAllMenuCommands( pMenu, ( theNewAction == CA_OPEN ) ? CA_NEW_MAILBOX : CA_TRANSFER_NEW );

				// destroy the menu
				pMenu->DestroyMenu();
			}
			else
			{
				g_theCommandStack.DeleteCommand( WORD(uID),  theNewAction );
			}

			return;	
		}

		if( theAction == CA_RENAME )
		{
			// Adding an IMAP mailbox is different. Add IMAP accounts in alphabetical order after
			// POP folders. 
			//
			if ( pCommand->IsImapType() )
			{
				iFolderPos = IMAPFindStartingPos ((QCImapMailboxCommand *)pCommand);
			}
			else
			{
				// find the first folder 
				iFolderPos = pSubMenu->GetMenuItemCount();

				while( --iFolderPos >= 0 )
				{
					UINT mID = pSubMenu->GetMenuItemID( iFolderPos );

					// Skip separators as well.

					if( mID && (mID != (UINT) -1) )
					{
						break;
					}
				}

				iFolderPos ++;
			}

//  IMAP4
			if( ( pCommand->GetType() == MBT_FOLDER ) ||
				( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShouldShowAsSubMenu() ) )
			{
				// add it to the menu in alphabetical order.
				AddMailboxMenuItem( 0, (LPCSTR) pData, pSubMenu, pMenu, iFolderPos, INT_MAX, ( theNewAction == CA_TRANSFER_TO ) );
			}
			else if ( pCommand->IsImapType() )
			{
				AddMailboxMenuItem( uID, (LPCSTR) pData, pSubMenu, NULL, iFolderPos, INT_MAX, ( theNewAction == CA_TRANSFER_TO ) );
			}
			else
			{
				// if it's in the root, insert it in alphabetical order after the
				// 5th item, else add it after the first
				if( pSubMenu == GetMenu()->GetSubMenu( uMenuIndex ) )
				{
					iMailboxPos = 4;
				}
				else
				{
					iMailboxPos = 1;
				}
				
				AddMailboxMenuItem( uID, (LPCSTR) pData, pSubMenu, NULL, iMailboxPos, iFolderPos, ( theNewAction == CA_TRANSFER_TO ) );
			}
			return;
		} 

		// ok were grafting 
		// find the new parent
		pSubMenu = GetMenu()->GetSubMenu( uMenuIndex );
		
		if( pData != NULL )
		{
			ASSERT_KINDOF( QCMailboxCommand, ( QCMailboxCommand* ) pData );
			szNamedPath = g_theMailboxDirector.BuildNamedPath( ( QCMailboxCommand* ) pData );

// IMAP4
			if ( (( QCMailboxCommand* ) pData)->IsImapType())
			{
				pSubMenu = ::ImapFindNamedMenuItem( szNamedPath, pSubMenu, &i, TRUE);
			}
			else
			{
				pSubMenu = ::FindNamedMenuItem( szNamedPath, pSubMenu, &i, TRUE );
			}
	
			if( pSubMenu == NULL || (pSubMenu = pSubMenu->GetSubMenu( i )) == NULL )
			{
				// should never happen
				ASSERT( 0 );
				return;
			}
		}

		// find the first folder 
		iFolderPos = pSubMenu->GetMenuItemCount();

		while( --iFolderPos >= 0 )
		{
			UINT mID = pSubMenu->GetMenuItemID( iFolderPos );

			// Skip separators as well.

			if( mID && (mID != (UINT) -1) )
			{
				break;
			}
		}

		iFolderPos ++;

		// insert the new item
//  IMAP4
		if( ( pCommand->GetType() == MBT_FOLDER ) ||
			( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu) ) )
		{
			AddMailboxMenuItem( 0, pCommand->GetName(), pSubMenu, pMenu, iFolderPos, INT_MAX, ( theNewAction == CA_TRANSFER_TO ) );
		}
		else
		{
			// if it's in the root, insert it in alphabetical order after the
			// 5th item, else add it after the first
			if( pSubMenu == GetMenu()->GetSubMenu( uMenuIndex ) )
			{
				iMailboxPos = 4;
			}
			else
			{
				iMailboxPos = 1;
			}
				
			AddMailboxMenuItem( uID, pCommand->GetName(), pSubMenu, NULL, iMailboxPos, iFolderPos, ( theNewAction == CA_TRANSFER_TO ) );
		}
		return;
		
	}

	if( theAction == CA_NEW )
	{
		// find the parent

		szNamedPath = g_theMailboxDirector.BuildNamedPath( pCommand );
		
		if( ( i = szNamedPath.ReverseFind( '\\' ) ) >= 0 )
		{
			szNamedPath = szNamedPath.Left( i );
//  IMAP4
			if (pCommand->IsImapType() && pCommand->GetType() != MBT_IMAP_ACCOUNT)
			{
				// Parent may not have been shown as a sub-menu. We now have to possibly
				// delete a menu item and create a sub-menu in its place.
				//
				QCImapMailboxCommand *pParent = g_theMailboxDirector.ImapFindParent( (QCImapMailboxCommand *)pCommand );
				if (!pParent)
				{	
					ASSERT (0);
					pSubMenu = NULL;
				}
				else
				{	
					// If parent is already displayed as a sub-menu, just return the submenu.
					// Otherwise, create one.
					//
					pSubMenu = IMAPRedisplayAsSubMenu (pParent, szNamedPath, pMenu, &i, theNewAction);
				}
			}
			else
				pSubMenu = ::FindNamedMenuItem( szNamedPath, pMenu, &i, TRUE );			

			if( pSubMenu == NULL )
			{
				ASSERT( 0 );
				return;
			}

			pMenu = pSubMenu->GetSubMenu( i );
			
			if( pMenu == NULL )
			{
				ASSERT( 0 );
				return;
			}
		}
		
		// add it to the transfer menu
		szName = pCommand->GetName();

		// Adding an IMAP mailbox is different. Add IMAP accounts in alphabetical order after
		// POP folders. 
		//
		if ( pCommand->IsImapType() )
		{
			iFolderPos = IMAPFindStartingPos ((QCImapMailboxCommand *)pCommand);
		}
		else
		{
			// find the first folder 
			iFolderPos = pMenu->GetMenuItemCount();

			while( --iFolderPos >= 0 )
			{
				UINT mID = pMenu->GetMenuItemID( iFolderPos );

				// Skip separators as well.

				if( mID && (mID != (UINT) -1) )
				{
					break;
				}
			}

			iFolderPos ++;
		}

// IMAP4 
		if( ( pCommand->GetType() == MBT_FOLDER ) ||
			( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShouldShowAsSubMenu() ) )
		{
			theSubMenu.CreatePopupMenu();

			AddMailboxMenuItem( ( UINT ) -1, szName, pMenu, &theSubMenu, iFolderPos, INT_MAX, ( theNewAction == CA_TRANSFER_TO ) );			

			// IMAP ACCOUNT: Append a separator.
			//
			if (pCommand->GetType() == MBT_IMAP_ACCOUNT)
			{
				IMAPAppendSeparator ((QCImapMailboxCommand *)pCommand, pMenu);
			}

			// insert the New... command

			uID = g_theCommandStack.AddCommand( pCommand, ( theNewAction == CA_TRANSFER_TO ) ? CA_TRANSFER_NEW : CA_NEW_MAILBOX );

			if( uID )
			{
				theSubMenu.AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, uID, CRString( IDS_MAILBOX_NEW ) ); 
			}

//  IMAP4
			// If this is a selectable IMAP mailbox that can have children, put in the "This Mailbox" item.
			if (uID && pCommand->IsImapType() )
			{
				// Tell command object that it's been added as a sub-menu:
				//
				((QCImapMailboxCommand *)pCommand)->SetShownAsSubMenu (TRUE, bIsXferMenu);

				QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *) pCommand;
				if ( !pImapCommand->IsNoSelect() )
				{
					uID = g_theCommandStack.AddCommand( pCommand, theNewAction );

					if( uID )
					{
						// Insert separator.
						theSubMenu.AppendMenu( MF_SEPARATOR);

						// AddMailboxMenuItem( uID, pCommand->GetName(), pSubMenu, NULL, 0, iFolderPos, ( theNewAction == CA_TRANSFER_TO ) );

						theSubMenu.AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, uID, CRString ( IDS_IMAP_THIS_MAILBOX) ); 

						// Insert separator again.
						theSubMenu.AppendMenu( MF_SEPARATOR);
					}
				}
			}
// IMAP4

			theSubMenu.Detach();
		}
		else if ( pCommand->IsImapType() )
		{
			uID = g_theCommandStack.AddCommand( pCommand, theNewAction );

			AddMailboxMenuItem( uID, szName, pMenu, NULL, iFolderPos, INT_MAX, ( theNewAction == CA_TRANSFER_TO ) );
		}

		else
		{
			// if it's in the root, insert it in alphabetical order after the
			// 5th item, else add it after the first
			if( pMenu == GetMenu()->GetSubMenu( uMenuIndex ) )
			{
				iMailboxPos = 4;
			}
			else
			{
				iMailboxPos = 1;
			}
				
			uID = g_theCommandStack.AddCommand( pCommand, theNewAction );

			AddMailboxMenuItem( uID, szName, pMenu, NULL, iMailboxPos, iFolderPos, ( theNewAction == CA_TRANSFER_TO ) );
		}

		return;
	}
}



// IMAPRedisplayAsSubMenu [PRIVATE]
//
// Because we now possibly show an IMAP mailbox that CAN have children but doesn't
// as a plain menu item, if we subsequently create a child mailbox, we need to replace the
// menu item with a submenu now.
//
CMenu* CMainFrame::IMAPRedisplayAsSubMenu (QCImapMailboxCommand *pImapCommand, LPCSTR pNamedPath,
								 CMenu* pMenu, int *pOutIndex, COMMAND_ACTION_TYPE theNewAction)
{
	// Sanity:
	//
	if ( ! (pImapCommand && pNamedPath && pMenu && pOutIndex) )
	{
		ASSERT (0);
		return NULL;
	}

	BOOL  bIsXferMenu;

	if( theNewAction == CA_TRANSFER_TO ) 
	{
		bIsXferMenu = TRUE;
	}
	else
	{
		bIsXferMenu = FALSE;
	}

	// Is this currently displayed as a submenu?
	//
	BOOL bAsSubMenu = pImapCommand->ShownAsSubMenu(bIsXferMenu);

	if (bAsSubMenu)
	{
		return ::ImapFindNamedMenuItem( pNamedPath, pMenu, pOutIndex, TRUE );
	}

	// Otherwise, delete the menu item and replace it with a sub-menu.
	//
	int index;
	CMenu* pSubMenu = ::ImapFindNamedMenuItem( pNamedPath, pMenu, &index, FALSE);

	if( pSubMenu == NULL )
	{
		// this should never happen
		ASSERT( 0 );
		return NULL;
	}

	UINT uID = pSubMenu->GetMenuItemID( index );
		
	if ( ( INT ) uID == -1 )
	{
		ASSERT(0);
		return NULL;
	}

	// Remove the item
	pSubMenu->RemoveMenu( index, MF_BYPOSITION );

	// unwrap if necessary
	WrapMenu( pSubMenu->GetSafeHmenu() );

	// The command ID may be different when we re-create it, so we have to delete the
	// item from the command stack.
	//
	g_theCommandStack.DeleteCommand( WORD(uID),  theNewAction );

	// Create now as a sub-menu:
	//
//	this->Notify(pImapCommand, CA_NEW, NULL);

	HandleMailboxNotification(pMenu, theNewAction, pImapCommand, CA_NEW, NULL);

	return ::ImapFindNamedMenuItem( pNamedPath, pMenu, pOutIndex, TRUE );
}




void CMainFrame::OnMailboxIn()
{
	QCMailboxCommand*	pCommand;
	
	pCommand = g_theMailboxDirector.FindByName( NULL, CRString( IDS_IN_MBOX_NAME ) );

	if( pCommand )
	{
		pCommand->Execute( CA_OPEN );
	}
	else
	{
		ASSERT( 0 );
	}
}

void CMainFrame::OnMailboxOut()
{
	QCMailboxCommand*	pCommand;

	pCommand = g_theMailboxDirector.FindByName( NULL, CRString( IDS_OUT_MBOX_NAME ) );

	if( pCommand )
	{
		pCommand->Execute( CA_OPEN );
	}
	else
	{
		ASSERT( 0 );
	}
}


void CMainFrame::OnMailboxTrash()
{
	QCMailboxCommand*	pCommand;

	pCommand = g_theMailboxDirector.FindByName( NULL, CRString( IDS_TRASH_MBOX_NAME ) );

	if( pCommand )
	{
		pCommand->Execute( CA_OPEN );
	}
	else
	{
		ASSERT( 0 );
	}
}


void CMainFrame::OnNewStationery()
{

}


void CMainFrame::OnDeleteStationery()
{
#ifdef COMMERCIAL
	CDeleteDialog theDlg( TRUE );
	
	theDlg.DoModal();	
#endif // COMMERCIAL
}

void CMainFrame::OnImportMail()
{
	CImportMailDlog blah;
	blah.DoModal();
}


/*****************************************************************************/
/*                          CMainFrameOleDropTarget                          */
/*****************************************************************************/


////////////////////////////////////////////////////////////////////////
// OnDragEnter [public, virtual]
//
// Handler for registered COleDropTarget::OnDragEnter() callbacks.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMainFrameOleDropTarget::OnDragEnter(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	// delegates to CMainFrame
	CMainFrame* pMainFrame = (CMainFrame *) pWnd;		// type cast
	ASSERT_VALID(pMainFrame);

	if (pMainFrame->m_bWorkbookMode && 
		g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_QCWILDCARD))
	{
		//
		// Start auto-activate timer
		//
		TRACE("CMainFrameOleDropTarget::OnDragEnter: Start timer\n");
		ASSERT(0 == pMainFrame->m_idAutoActivateTimer);
		pMainFrame->m_idAutoActivateTimer = pMainFrame->SetTimer(CMainFrame::AUTOACTIVATE_TIMER_EVENT_ID, 400, NULL);
	}

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDragOver [public, virtual]
//
// Handler for registered COleDropTarget::OnDragOver() callbacks.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMainFrameOleDropTarget::OnDragOver(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DWORD dwKeyState, 
	CPoint point)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	if (! g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_QCWILDCARD))
	{
		return DROPEFFECT_NONE;
	}

	//
	// Okay, clipboard has usable drop data from our instance, so
	// let's do some hit tests to see if we're over a valid drop
	// target.
	//
	CMainFrame* pMainFrame = (CMainFrame *) pWnd;		// type cast
	ASSERT_VALID(pMainFrame);
	if (pMainFrame->m_bWorkbookMode && pMainFrame->TabHitTest(point, FALSE))
	{
		//
		// We're over an "inactive" tab button.  Use DROPEFFECT_LINK as
		// a hack indication to the OleDropSource to display a normal cursor.
		//
		return DROPEFFECT_LINK;
	}
	else if (pMainFrame->IsOnMDIClientAreaBlankSpot(point) ||
			 pMainFrame->IsOnStatusBar(point))
	{
		//
		// We're over a "blank" area of the MDI client region, or over
		// the status bar, so see if there is usable Wazoo data in the
		// clipboard.
		//
		// Do an extra check here to make sure we're not
		// "moving" a Wazoo from an MDI-mode container with only
		// a single Wazoo.  We prohibit this since it is an
		// expensive way to move an MDI Wazoo window!
		//
		QCFWAZOO qcfWazoo;
		if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfWazoo))
			return DROPEFFECT_NONE;

		ASSERT_KINDOF(CWazooBar, qcfWazoo.m_pSourceWazooBar);

		if (qcfWazoo.m_pSourceWazooBar->IsMDIChild() && (qcfWazoo.m_pSourceWazooBar->GetWazooCount() == 1))
			return DROPEFFECT_NONE;

		return DROPEFFECT_MOVE;
	}

	return DROPEFFECT_NONE;
}


////////////////////////////////////////////////////////////////////////
// OnDrop [public, virtual]
//
// Handler for registered COleDropTarget::OnDrop() callbacks.
////////////////////////////////////////////////////////////////////////
BOOL CMainFrameOleDropTarget::OnDrop(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect, 
	CPoint point)
{
	ASSERT_VALID(this);
	TRACE("CMainFrameOleDropTarget::OnDrop\n");

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
	{
		ASSERT(0);
		return FALSE;
	}

	CMainFrame* pMainFrame = (CMainFrame *) pWnd;
	ASSERT_VALID(pMainFrame);

	//
	// First things first ... always kill the auto activate timer.
	//
	if (pMainFrame->m_idAutoActivateTimer)
	{
		ASSERT(pMainFrame->m_bWorkbookMode);
		pMainFrame->KillTimer(pMainFrame->m_idAutoActivateTimer);
		pMainFrame->m_idAutoActivateTimer = 0;
	}

	//
	// Now, check to see if the user dropped a Wazoo window onto a
	// blank spot of the MDI Client area.
	//
	if (g_theClipboardMgr.IsInstanceDataAvailable(pDataObject, QCClipboardMgr::CF_WAZOO))
	{
		QCFWAZOO qcfWazoo;

		if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfWazoo))
		{
			ASSERT(0);
			return DROPEFFECT_NONE;
		}

		//
		// Check to see if we're over a "blank" area of the MDI
		// client region.
		//
		if (pMainFrame->IsOnMDIClientAreaBlankSpot(point) ||
			pMainFrame->IsOnStatusBar(point))
		{
			//
			// This is the big moment ... create a new Wazoo
			// container and move the dragged wazoo into its new home.
			//
			TRACE("CMainFrameOleDropTarget::OnDrop: dropped WazooWnd\n");
			CWazooBar* pTargetWazooBar = pMainFrame->m_WazooBarMgr.CreateNewWazooBar(pMainFrame);
			if (pTargetWazooBar)
			{
				ASSERT_KINDOF(CWazooBar, qcfWazoo.m_pSourceWazooBar);
				ASSERT_KINDOF(CWazooWnd, qcfWazoo.m_pSourceWazooWnd);
				qcfWazoo.m_pSourceWazooBar->RemoveWazooWindow(qcfWazoo.m_pSourceWazooWnd);
				pTargetWazooBar->AddWazooWindow(qcfWazoo.m_pSourceWazooWnd, CPoint(-1, -1));
				pMainFrame->RecalcLayout();
				pTargetWazooBar->ActivateWazooWindow(qcfWazoo.m_pSourceWazooWnd->GetRuntimeClass());

				return TRUE;
			}
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnDropEx [public, virtual]
//
// Handler for registered COleDropTarget::OnDropEx() callbacks.
////////////////////////////////////////////////////////////////////////
DROPEFFECT CMainFrameOleDropTarget::OnDropEx(
	CWnd* pWnd, 
	COleDataObject* pDataObject,
	DROPEFFECT dropDefault, 
	DROPEFFECT dropList, 
	CPoint point)
{
	ASSERT_VALID(this);
	TRACE("CMainFrameOleDropTarget::OnDropEx\n");

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
	{
		ASSERT(0);
		return DROPEFFECT_NONE;
	}

	//
	// Normally, this would delegate to the CMainFrame::OnDropEx()
	// method.  However, since we're currently not doing anything like
	// right button drags, let's just return a "not implemented" status
	// code to force MFC to call the older OnDrop() method.
	//
	return DROPEFFECT(-1);
}


////////////////////////////////////////////////////////////////////////
// OnDragLeave [public, virtual]
//
// Handler for registered COleDropTarget::OnDragLeave() callbacks.
////////////////////////////////////////////////////////////////////////
void CMainFrameOleDropTarget::OnDragLeave(CWnd* pWnd)
{
	ASSERT_VALID(this);

	if (! pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
	{
		ASSERT(0);
		return;
	}

	// delegates to workbook
	CMainFrame* pMainFrame = (CMainFrame *) pWnd;
	ASSERT_VALID(pMainFrame);

	//
	// Stop auto-activate timer
	//
	if (pMainFrame->m_idAutoActivateTimer)
	{
		ASSERT(pMainFrame->m_bWorkbookMode);
		TRACE("CMainFrameOleDropTarget::OnDragLeave: Kill timer\n");
		pMainFrame->KillTimer(pMainFrame->m_idAutoActivateTimer);
		pMainFrame->m_idAutoActivateTimer = 0;
	}
}


UINT CMainFrame::OnNcHitTest(CPoint point) 
{
	ASSERT(m_pControlBarManager != NULL);
	ASSERT_KINDOF(SECToolBarManager, m_pControlBarManager);

	if(!((SECToolBarManager*)m_pControlBarManager)->IsMainFrameEnabled())
		return QCWorkbook::OnNcHitTest(point);
	else
		return (UINT)HTERROR;
}

void CMainFrame::OnEnable(BOOL bEnable) 
{
	ASSERT(m_pControlBarManager != NULL);
	ASSERT_KINDOF(SECToolBarManager, m_pControlBarManager);

	if(((SECToolBarManager*)m_pControlBarManager)->IsMainFrameEnabled())
		m_nFlags &= ~WF_STAYACTIVE;

	QCWorkbook::OnEnable(bEnable);
}

LRESULT CMainFrame::OnShowProgress(WPARAM wParam, LPARAM lParam)
{
	extern BOOL OpenProgress();
	if (InProgress)
		OpenProgress();

	return 1;
}

BOOL CMainFrame::OnNcActivate(BOOL bActive) 
{
	ASSERT(m_pControlBarManager != NULL);
	ASSERT_KINDOF(SECToolBarManager, m_pControlBarManager);

	if(((SECToolBarManager*)m_pControlBarManager)->IsMainFrameEnabled())
		m_nFlags &= ~WF_STAYACTIVE;
	
	// This makes sure the Progress window gets shown when the main
	// window comes to the foreground
	if (bActive)
		PostMessage(umsgShowProgress);
	
	return QCWorkbook::OnNcActivate(bActive);
}

void CMainFrame::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
	ASSERT(m_pControlBarManager != NULL);
	ASSERT_KINDOF(SECToolBarManager, m_pControlBarManager);

	if(nWhy == MSGF_DIALOGBOX && ((SECToolBarManager*)m_pControlBarManager)->IsMainFrameEnabled())
	{
		// We have a customize dialog up. Check to see if we require a delayed
		// "RecalcLayout". Normally these would be handled on WM_IDLE ... but,
		// sadly, we don't get these while a dialog is displayed.
		if(m_nIdleFlags & idleLayout)
		{
			RecalcLayout(m_nIdleFlags & idleNotify);
			UpdateWindow();
		}
	}

	QCWorkbook::OnEnterIdle(nWhy, pWho);	
}

void CMainFrame::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	//Since Paige is the only one with support for Redo, disable it!
	pCmdUI->SetText(CRString(IDS_EDIT_REDO));
	pCmdUI->Enable(FALSE);	
}

//Bug 2760.  If '&' is used in a name it is underlined in the menu..this call fixes this
//Will insert another '&' so it will appear correctly eg. if strOldName is "te&st" szNewName will be "te&&st"
void CMainFrame::GenerateNewName(CString strOldName, char *szNewName)
{
	INT nLen = strOldName.GetLength();
	LPTSTR pszName = strOldName.GetBuffer(nLen);
	int nFirstPos;

	if(nFirstPos = strOldName.Find("&") >= 0)
	{
		int i, j;
		
		_mbsnbcpy((unsigned char*)szNewName, (unsigned char*) pszName, nFirstPos + 1);
		
		for(i=j= (nFirstPos + 1); i < nLen; i++,j++)
		{
			if(pszName[i] == '&')
			{
				szNewName[j] = '&';
				j++;
				szNewName[j] = '&';
			}
			else
				szNewName[j] = pszName[i];
		}
		szNewName[j] = '\0';
		
	}
	else
		strcpy(szNewName, pszName);

	strOldName.ReleaseBuffer();
}

// Reset DS resize logic.
void
CMainFrame::ResetDSResize()
{
    if (m_pDirectoryServicesWazooWndNew) {
	ASSERT_VALID(m_pDirectoryServicesWazooWndNew);
	ASSERT(::IsWindow(m_pDirectoryServicesWazooWndNew->m_hWnd));
	m_pDirectoryServicesWazooWndNew->ResetResize();
    }
}


// OnImapResyncOpened [PUBLIC]
//
// Ask IMAP mail manager to resync opened tocs. This is performed just after
// startup.
// Resync's are launched in series (each involves a background part).
//
void CMainFrame::OnImapResyncOpened ()
{
	GetImapMailMgr()->ResyncOpenedTocs();
}


/*****************************************************************************/
/*                              QCWorkbookClient                             */
/*****************************************************************************/


////////////////////////////////////////////////////////////////////////
// WindowProc [protected, virtual]
//
// Hook messages for the MDI client window.
////////////////////////////////////////////////////////////////////////
LRESULT QCWorkbookClient::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MDIREFRESHMENU:
		{
			CWnd* Win;
			BOOL bFoundOne = FALSE;
			for (Win = GetWindow(GW_CHILD); Win ; Win = Win->GetNextWindow())
			{
				if (Win->IsWindowVisible())
				{
					if (bFoundOne)
					{
						bFoundOne = FALSE;
						break;
					}
					bFoundOne = TRUE;
				}
			}
			BOOL bIsMax;
			CMDIChildWnd* ChildWin = ((CMainFrame*)GetParent())->MDIGetActive(&bIsMax);
			if (bFoundOne)
				SetIniLong(IDS_INI_LAST_WINDOW_MAX, bIsMax);
		}
		break;

	case WM_MDIDESTROY:
		{
			//
			// This is a workaround suggested by MS Knowledge Base article
			// Q125435 which acknowledges that there is a bug in Windows
			// that causes the Window menu's list to MDI child windows
			// to get corrupted when there are one or more disabled
			// or hidden MDI child windows.  The article explains that
			// the corruption only occurs after you destroy the first
			// non-hidden/non-disabled MDI child window.  Therefore,
			// the workaround is to always disable and hide each MDI child
			// window just before it is destroyed.
			//
			// The 'wParam' contains the hwnd of the child being destroyed
			//
			HWND hMDIChild = (HWND)wParam;
			::EnableWindow(hMDIChild, FALSE);
			::ShowWindow(hMDIChild, SW_HIDE);
		}
		break;

	case WM_ERASEBKGND:
		{
			HDC hDC = (HDC) wParam;
			OnEraseBkgnd( CDC::FromHandle( hDC ) );
			return 1;	 // nonzero if we do the background
		}

	case WM_WINDOWPOSCHANGED:
		Invalidate();
		break;

	default:
		break;
	}

	return DefWindowProc(message, wParam, lParam);
}



//======================================= INTERNAL ==================================//
//

// Find the starting menu item for alphabetical insertion in the menu.
//
UINT IMAPFindStartingPos (QCImapMailboxCommand* pImapCommand)
{
	UINT iStartPos = 0;

	// If the mailbox object to be inserted is an IMAP ACCOUNT, insert it in
	// alphabetical order after the last MBT_FOLDER.
	//
	if ( pImapCommand->GetType() == MBT_IMAP_ACCOUNT )
	{
		CPtrList* pTopList = g_theMailboxDirector.GetMailboxList();

		UINT	 iPos	= 0;
		POSITION pos	= pTopList->GetHeadPosition();

		while (pos)
		{
			iPos++;

			QCMailboxCommand *pCurrent = (QCMailboxCommand *) pTopList->GetNext(pos);

			if ( pCurrent->IsImapType() )
			{
				// Add menu items for separators, etc.
				//
				iStartPos = iPos + 1;
				break;
			}
		}
	}
	else
	{
		// Is parent an ACCOUNT?
		//
		QCMailboxCommand* pParent = g_theMailboxDirector.FindParent( pImapCommand );

		if ( pParent && pParent->GetType() == MBT_IMAP_ACCOUNT )
		{
			CPtrList* pList = &pParent->GetChildList();

			POSITION pos = pList->GetHeadPosition();

			if (pos)
			{
				QCImapMailboxCommand* pCurrent = (QCImapMailboxCommand *) pList->GetNext (pos);

				if ( pCurrent && pCurrent->IsImapInbox() )
				{
					iStartPos = 2;
				}
			}
		}
		else
		{
			// Start from the top:
			//
			iStartPos = 1;
		}
	}


	// Failure?
	//
	if (iStartPos == 0)
		iStartPos = 1;

	return iStartPos;
}




void IMAPAppendSeparator (QCImapMailboxCommand* pImapCommand, CMenu* pMenu)
{
	if (! (pImapCommand && pMenu) )
		return;

	CString szNamedPath = g_theMailboxDirector.BuildNamedPath( pImapCommand );
	INT nPos = -1;

	if ( ImapFindNamedMenuItem( szNamedPath, pMenu, &nPos, TRUE) && (nPos >= 0) )
	{
		// If szNamedPath is in the last position, make sure a separator is above it:
		//
		UINT nLastPos = pMenu->GetMenuItemCount() - 1;
		if ( ((UINT)nPos == nLastPos) && (nLastPos > 0) )
		{
			UINT uID = pMenu->GetMenuItemID( nPos - 1 );

			if (uID != 0)
				pMenu->InsertMenu( nPos, MF_BYPOSITION | MF_SEPARATOR);
		}
		else
		{
			// Append separator.
			pMenu->InsertMenu( nPos + 1, MF_BYPOSITION | MF_SEPARATOR);
		}
	}
}

// --------------------------------------------------------------------------

#ifdef _DEBUG

// Debug-only menu items

void CMainFrame::OnUpdateSharewareChangeStateLight(CCmdUI* pCmdUI)
{
	const bool bIsLight = (GetSharewareMode() == SWM_MODE_LIGHT);

	pCmdUI->SetRadio(bIsLight);
	pCmdUI->Enable(!bIsLight);
}

void CMainFrame::OnSharewareChangeStateLight() // ON_COMMAND( ID_SHAREWAREDEBUG_CHANGESTATE_LIGHT )
{
	// Shareware: Register that we want to know of feature changes
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		if (GetSharewareMode() == SWM_MODE_LIGHT) // Already in LIGHT
			AfxMessageBox("Already running in LIGHT mode.");
		else
			pSWM->SetMode(SWM_MODE_LIGHT);
	}
}

void CMainFrame::OnUpdateSharewareChangeStatePro(CCmdUI* pCmdUI)
{
	const bool bIsPro = (GetSharewareMode() == SWM_MODE_PRO);

	pCmdUI->SetRadio(bIsPro);
	pCmdUI->Enable(!bIsPro);
}

void CMainFrame::OnSharewareChangeStatePro() // ON_COMMAND( ID_SHAREWAREDEBUG_CHANGESTATE_PRO )
{
	// Shareware: Register that we want to know of feature changes
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		if (GetSharewareMode() == SWM_MODE_PRO) // Already in PRO
			AfxMessageBox("Already running in PRO mode.");
		else
			pSWM->SetMode(SWM_MODE_PRO);
	}
}

void CMainFrame::OnUpdateSharewareChangeStateAdware(CCmdUI* pCmdUI)
{
	const bool bIsAdware = (GetSharewareMode() == SWM_MODE_ADWARE);

	pCmdUI->SetRadio(bIsAdware);
	pCmdUI->Enable(!bIsAdware);
}

void CMainFrame::OnSharewareChangeStateAdware() // ON_COMMAND( ID_SHAREWAREDEBUG_CHANGESTATE_ADWARE )
{
	// Shareware: Register that we want to know of feature changes
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		if (GetSharewareMode() == SWM_MODE_ADWARE) // Already in ADWARE
			AfxMessageBox("Already running in ADWARE mode.");
		else
			pSWM->SetMode(SWM_MODE_ADWARE);
	}
}

#endif // _DEBUG

// --------------------------------------------------------------------------

// GetTopMenuByPosition [PUBLIC]
//
// This returns the top-level menu (file, edit, mailbox, etc.)
// Defined positions are in mainfrm.h as MAIN_MENU_*
//
CMenu *CMainFrame::GetTopMenuByPosition(int nPos)
{
    BOOL bMaximized = FALSE;
    MDIGetActive(&bMaximized);
    const int uOffset = ( bMaximized ? 1 : 0 );

    ASSERT(GetMenu());

    return (GetMenu()->GetSubMenu( nPos + uOffset ));
}


// SWMNotify [PUBLIC]
//
// Message handler for Shareware change notification. Getting here means the app has changed
// from FULL to REDUCED (or visa versa) feature set.
//
void
CMainFrame::SWMNotify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData)
{
    if (theAction != CA_SWM_CHANGE_FEATURE)
    {
        ASSERT(0);
    }
        
    // Shareware: In reduced feature mode, you only get a standard toolbar
    if (UsingFullFeatureSet())
    {
        // Moving from Pro to Adware or Adware to Pro is a noop as far as we're concerned here
		// pData points to the PREVIOUS mode, not the current one.
        if (*((SharewareModeType *)pData) == SWM_MODE_LIGHT)
        {
            // ++++ Moving from REDUCED to FULL ++++

            //
            // Toolbar crap-o-la
            //
            FlushIncompatibleToolBarState(); // Update the INI
            WriteToolBarMarkerToIni(); // Update the marker in INI

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            // Enable the Edit->Insert-> submenu (POS_EDIT_INSERT_SUBMENU), which contains...
            //   "Picture..." (ID_EDIT_INSERT_PICTURE)
            //   "Horizontal Line" (ID_EDIT_INSERT_HR)
            CMenu *pMenu = GetTopMenuByPosition(MAIN_MENU_EDIT);
            ASSERT(pMenu);
            if (pMenu)
                pMenu->EnableMenuItem(POS_EDIT_INSERT_SUBMENU, MF_ENABLED | MF_BYPOSITION);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            //
            // Nickname stuff -- only one nick file allowed in REDUCED FEATURE mode
            //
            ASSERT(g_Nicknames);
            if (g_Nicknames)
                g_Nicknames->UpdateSharewareFeatureMode();
        }
    }
    else
    {
        // ++++ Moving from FULL to REDUCED ++++

        //
        // Toolbar should be reset to default
        //
        m_pToolBar->SetButtons( buttons, DIM( buttons ) ); // Reset the buttons

        FlushIncompatibleToolBarState(); // Update the INI
        WriteToolBarMarkerToIni(); // Update the marker in INI

        LoadBarState(_T("ToolBar")); // Update the toolbar

        QCToolBarManager* pMgr = (QCToolBarManager*)m_pControlBarManager;
        pMgr->LoadState(_T("ToolBar")); // Update toolbar mgr

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        //
        // Disable the Edit->Insert-> submenu (POS_EDIT_INSERT_SUBMENU), which contains...
        //   "Picture..." (ID_EDIT_INSERT_PICTURE)
        //   "Horizontal Line" (ID_EDIT_INSERT_HR)
        //
        CMenu *pMenu = GetTopMenuByPosition(MAIN_MENU_EDIT);
        ASSERT(pMenu);
        if (pMenu)
            pMenu->EnableMenuItem(POS_EDIT_INSERT_SUBMENU, MF_GRAYED | MF_BYPOSITION);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        //
        // Nickname stuff -- only one nick file allowed in REDUCED FEATURE mode
        //
        ASSERT(g_Nicknames);
        if (g_Nicknames)
            g_Nicknames->UpdateSharewareFeatureMode();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        //
        // Filters - when going from FULL to REDUCED FEATURE mode, we need
        //  to warn the user if they have any restricted filters.
        //
        ASSERT(g_pApp);
        if (g_pApp)
            g_pApp->DoCheckProFilters();
    }

}
