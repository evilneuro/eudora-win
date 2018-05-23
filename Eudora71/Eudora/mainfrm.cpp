// MAINFRM.CPP
//
// Copyright (c) 1994-2001 by QUALCOMM, Incorporated
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

#include "afxpriv.h"	// Needed for WM_SETMESSAGESTRING

#include <ctype.h>
#include <afxcmn.h>
#include <afxrich.h>

#include <vfw.h>

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
#include "textfile.h"
#include "TextFileView.h"

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
#include "LinkHistoryWazooWnd.h"

#include "TaskErrorView.h"
#include "TaskErrorWazooWnd.h"

#include "DSWazooWndNewIface.h"
#include "NewSignatureDialog.h"
#include "PaigeEdtView.h"

#include "QCToolBarManager.h"
#include "QCCustomToolBar.h"
#include "SearchBar.h"

#include "QCLabelDirector.h"
#include "QCSharewareManager.h"
#include "QCRASConnection.h"
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
#include "QCSelectedTextURLCommand.h"
#include "Automation.h"
#include "TraceFile.h"
#include "WizardPropSheet.h"
#include "SafetyPal.h"
#include "QCGraphics.h"
#include "QCTaskManager.h"
#include "AutoCompleteSearcher.h"
#include "ImportMailDlog.h"
#include "ImportAdvDlog.h"
#include "QCFindMgr.h"
#include "timestmp.h"
#include "EudoraMsgs.h"
#include "ExceptionHandler.h"

#include "FaceTime.h"

#include "StatMng.h"

#include "JunkMail.h"
#include "SearchManager.h"

// playlist subsystem interface
#include "plist_mgr.h"
#include "AdView.h"
#include "nsprefs.h"

#include "EmoticonMenu.h"
#include "EmoticonManager.h"

HMENU g_EudoraProHMENU;

#include "spell.h"
#include "PaymentAndRegistrationDlg.h"
#include "DockBar.h"
#include "AdWazooBar.h"
#include "ProfileNag.h"

#include "PopupText.h"

#ifdef IMAP4
#include "ImapTypes.h"
#include "QCImapMailboxCommand.h"
#include "ImapMailMgr.h"
#include "ImapAccountMgr.h"
#endif

#include "DebugNewHelpers.h"

//	Definitions from newer version of "WinUser.h" and "BaseTsd.h" in Microsoft platform SDK
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR           30

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

//	Definition not normally allowed with our current WINVER value.
//	We check for recent enough version of Windows before using.
#ifndef SM_CMONITORS
#define SM_CMONITORS            80
#endif

#if defined(_WIN64)
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
    typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
#endif

typedef struct tagMENUINFO
{
    DWORD   cbSize;
    DWORD   fMask;
    DWORD   dwStyle;
    UINT    cyMax;
    HBRUSH  hbrBack;
    DWORD   dwContextHelpID;
    ULONG_PTR dwMenuData;
}   MENUINFO, FAR *LPMENUINFO;
typedef MENUINFO CONST FAR *LPCMENUINFO;
#endif

//	More definitions from newer version of "WinUser.h" in Microsoft platform SDK
#ifndef MIM_BACKGROUND
#define MIM_BACKGROUND              0x00000002
#endif

#ifndef MIM_APPLYTOSUBMENUS
#define MIM_APPLYTOSUBMENUS         0x80000000
#endif


extern QCCommandStack		g_theCommandStack;
extern QCAutomationDirector g_theAutomationDirector;
extern QCMailboxDirector	g_theMailboxDirector;
extern QCPluginDirector		g_thePluginDirector;
extern QCRecipientDirector	g_theRecipientDirector;
extern QCSignatureDirector	g_theSignatureDirector;
extern QCStationeryDirector	g_theStationeryDirector;
extern QCPersonalityDirector g_thePersonalityDirector;

// Thread for DDE
static UINT DDEClientThreadFunc(LPVOID lpParameter);

// WINDOWSX.H has a SubclassWindow macro which collides
#ifdef SubclassWindow
#undef SubclassWindow
#endif // SubclassWindow

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

// Submenu positions -- these are needed because a submenu does not have an ID

// Edit Menu, positions of submenus
#define POS_EDIT_INSERT_SUBMENU        (11)
#define POS_EDIT_INSERTRECIPIENT_MENU  (17)
#define POS_EDIT_FIND_SUBMENU          (19)

// Find submenu, position of "Find Using" submenu
#define POS_FIND_FINDUSING_SUBMENU      (4)

// Message Menu, positions of submenus
#define POS_NEW_MESSAGE_TO    7
#define POS_FORWARD_TO        8
#define POS_REDIRECT_TO       9
#define POS_NEW_MESSAGE_WITH  11
#define POS_REPLY_WITH		  12
#define POS_REPLY_TO_ALL_WITH 13
#define POS_ATTACH			  16	
#define POS_CHANGE			  18

// Special Menu, positions of submenus
#define POS_REMOVE_RECIPIENT  5

#define TOOBAR_AD_CMD_BASE	31000

BEGIN_BUTTON_MAP(btnMap)
END_BUTTON_MAP()


// overwrite mode for editing is global state
extern bool g_bOverwriteMode;

// STATIC (IMAP)
UINT IMAPFindStartingPos (QCImapMailboxCommand* pImapCommand);
void IMAPAppendSeparator (QCImapMailboxCommand* pImapCommand, CMenu* pMenu);


///////////////////////////////////////////////////////////////////////
// CAdInfo
//
// Info for an ad toolbar entry.
///////////////////////////////////////////////////////////////////////

CAdInfo::CAdInfo(UINT iCmd, CBitmap* pBmp, CBitmap* pBmpAlt, CString& strID, CString& strAdURL, CString& strAdTitle)
{
	m_iCmd = iCmd;
	m_pBmp = pBmp;
	m_pBmpAlt = pBmpAlt;
	m_strURL = strID;
	m_strID = strAdURL;
	m_strTitle = strAdTitle;
}

CAdInfo::~CAdInfo()
{
	if (m_pBmp)
	{
		delete m_pBmp;
		m_pBmp = NULL;
	}
	if (m_pBmpAlt)
	{
		delete m_pBmpAlt;
		m_pBmpAlt = NULL;
	}
}


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
	ON_WM_TIMECHANGE()
	
	ON_COMMAND(ID_CTRL_F, OnCtrlF)
	ON_COMMAND(ID_EDIT_FIND_FINDMSG, OnEditFindFindMsg)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDMSG, OnUpdateEditFindFindMsg)
	
	ON_COMMAND(ID_SHIFT_CTRL_F, OnShiftCtrlF)
	ON_COMMAND(ID_EDIT_FIND_FINDTEXT, OnEditFindFindText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXT, OnUpdateEditFindFindText)
	
	ON_COMMAND(ID_EDIT_FIND_FINDTEXTAGAIN, OnEditFindFindTextAgain)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_FINDTEXTAGAIN, OnUpdateEditFindFindTextAgain)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_COMMAND(ID_CTRL_J, OnCtrlJ)
	ON_COMMAND(ID_SHIFT_CTRL_J, OnShiftCtrlJ)
	ON_COMMAND(ID_SHIFT_CTRL_L, OnShiftCtrlL)
	ON_COMMAND(ID_FILE_CHECKMAIL, OnFileCheckMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_TOGGLE_OFFLINE, OnUpdateToggleOffline)
	ON_COMMAND(ID_FILE_TOGGLE_OFFLINE, OnFileToggleOffline)
	ON_COMMAND(ID_MESSAGE_NEWMESSAGE, OnMessageNewMessage)
	ON_COMMAND(ID_MESSAGE_REPLY_CTRL_R, OnMessageReplyCtrlR)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_EMPTYTRASH, OnUpdateSpecialEmptyTrash)
	ON_COMMAND(ID_SPECIAL_EMPTYTRASH, OnSpecialEmptyTrash)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_TRIMJUNK, OnUpdateTrimJunk)
	ON_COMMAND(ID_SPECIAL_TRIMJUNK, OnSpecialTrimJunk)
	ON_COMMAND(ID_SPECIAL_COMPACTMAILBOXES, OnSpecialCompactMailboxes)
	ON_COMMAND(ID_SPECIAL_SETTINGS, OnSpecialSettings)
	ON_COMMAND(ID_SPECIAL_NEWACCOUNT, OnSpecialNewAccount)
	ON_COMMAND(ID_SPECIAL_CHANGEPASSWORD, OnSpecialChangePassword)
	ON_COMMAND(ID_SPECIAL_FORGETPASSWORD, OnSpecialForgetPassword)
	ON_COMMAND(ID_WINDOW_FILTERS, OnWindowFilters)
	ON_COMMAND(ID_WINDOW_MAILBOXES, OnWindowMailboxes)
	ON_COMMAND(ID_WINDOW_NICKNAMES, OnWindowNicknames)
	ON_COMMAND(ID_HELP_SEARCHFORHELPON, OnHelpSearchForHelpOn)
	ON_COMMAND_RANGE(TOOBAR_AD_CMD_BASE, TOOBAR_AD_CMD_BASE + MAX_TOOLBAR_ADS, OnToolBarAd)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_FORGETPASSWORD, OnUpdateSpecialForgetPassword)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
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
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
	ON_WM_NCHITTEST()
	ON_WM_ENABLE()
	ON_WM_NCACTIVATE()
	ON_WM_ENTERIDLE()
	ON_COMMAND(ID_WINDOW_PH, OnViewLookup)
	ON_COMMAND(ID_WINDOW_LINK_HISTORY, OnViewLinkHistory)
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
	ON_COMMAND(ID_HELP_PAYMENT_AND_REGISTRATION, OnPaymentAndRegistration)
	ON_UPDATE_COMMAND_UI(ID_HELP_PAYMENT_AND_REGISTRATION, OnUpdatePaymentAndRegistration)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_USAGE_STATS, OnUpdateStatistics)
	ON_COMMAND(ID_SYSTEM_TRAY_OPEN_EUDORA, OnOpenEudora)
	ON_COMMAND(ID_SYSTEM_TRAY_MINIMIZE, OnMinimizeToSystemTray)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_TRAY_MINIMIZE, OnUpdateMinimizeToSystemTray)
	ON_COMMAND(ID_SYSTEM_TRAY_ALWAYS_SHOW, OnAlwaysShowTrayIcon)
	ON_UPDATE_COMMAND_UI(ID_SYSTEM_TRAY_ALWAYS_SHOW, OnUpdateAlwaysShowTrayIcon)
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
	ON_COMMAND(ID_WINDOW_USAGE_STATS, OnViewUsageStatistics)

	ON_COMMAND(ID_TOOLS_RESET_TOOLS, OnResetTools)

//FORNOW	ON_COMMAND(ID_CTRLBAR_SHOW_MAILBOXES, OnShowMboxBar)					// WIN32 only
	ON_COMMAND(ID_CTRLBAR_SHOW_TOOLBAR, OnShowToolBar)						// WIN32 only
	ON_COMMAND(ID_CTRLBAR_SHOW_SEARCHBAR, OnShowSearchBar)					// WIN32 only
	ON_COMMAND(ID_CTRLBAR_SHOW_STATUSBAR, OnShowStatusBar)					// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CTRLBAR_SHOW_MAILBOXES, OnUpdateControlBarMenu)	// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CTRLBAR_SHOW_TOOLBAR, OnUpdateControlBarMenu)	// WIN32 only
	ON_UPDATE_COMMAND_UI(ID_CTRLBAR_SHOW_SEARCHBAR, OnUpdateControlBarMenu)	// WIN32 only
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

	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	
	ON_COMMAND(ID_HELP_MOREHELP, OnMoreHelp)

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
	ON_COMMAND( ID_MAILBOX_JUNK, OnMailboxJunk )

	ON_COMMAND( IDM_IMPORT_MAIL, OnImportMail)

	ON_COMMAND( ID_NEW_MAILBOX_IN_ROOT, OnNewMailboxInRoot )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )
	ON_COMMAND_EX_RANGE( ID_EDIT_CTRL_2, ID_EDIT_CTRL_9, OnSelectedTextURLCommand )

// IMAP stuff: - After startup, resync opened mailboxes.
	ON_COMMAND(ID_MBOX_IMAP_RESYNC_OPENED, OnImapResyncOpened)

	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	
	ON_MESSAGE_VOID(msgConnectionWasMade, OnConnectionWasMade)

	ON_MESSAGE(MCIWNDM_NOTIFYMODE, OnMCINotifyMode)
	ON_MESSAGE(WM_USER_MCI_NOTIFY_MODE, OnUserMCINotifyMode)

#ifdef _DEBUG
	// Debug-only menu items
	ON_COMMAND(ID_SHAREWAREDEBUG_CHANGESTATE_LIGHT, OnSharewareChangeStateLight) // LIGHT
	ON_UPDATE_COMMAND_UI(ID_SHAREWAREDEBUG_CHANGESTATE_LIGHT, OnUpdateSharewareChangeStateLight)

	ON_COMMAND(ID_SHAREWAREDEBUG_CHANGESTATE_PRO, OnSharewareChangeStatePro) // PRO
	ON_UPDATE_COMMAND_UI(ID_SHAREWAREDEBUG_CHANGESTATE_PRO, OnUpdateSharewareChangeStatePro)

	ON_COMMAND(ID_SHAREWAREDEBUG_CHANGESTATE_ADWARE, OnSharewareChangeStateAdware) // ADWARE
	ON_UPDATE_COMMAND_UI(ID_SHAREWAREDEBUG_CHANGESTATE_ADWARE, OnUpdateSharewareChangeStateAdware)

	// Debug-only menu items
	ON_COMMAND(ID_DEBUG_NEW_PLAYLIST, OnRequestNewPlaylist)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_NEW_PLAYLIST, OnUpdateRequestNewPlaylist)

	ON_COMMAND(ID_DEBUG_BRAND_NEW_PLAYLIST, OnRequestBrandNewPlaylist)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_BRAND_NEW_PLAYLIST, OnUpdateRequestBrandNewPlaylist)

	ON_COMMAND(ID_DEBUG_DUMP_ADSTATE, OnDumpAdState)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_DUMP_ADSTATE, OnUpdateDumpAdState)

	ON_COMMAND_RANGE( ID_DEBUG_PLAYLIST_SERVER0, ID_DEBUG_PLAYLIST_SERVER9, OnChangePlaylistServer )
	ON_UPDATE_COMMAND_UI_RANGE( ID_DEBUG_PLAYLIST_SERVER0, ID_DEBUG_PLAYLIST_SERVER9, OnUpdateChangePlaylistServer)
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
	ID_MESSAGE_FORWARD,
		ID_SEPARATOR,
	ID_PREVIOUS_MESSAGE,
	ID_NEXT_MESSAGE,
		ID_SEPARATOR,
	ID_MESSAGE_ATTACHFILE,
		ID_SEPARATOR,
	ID_EDIT_FIND_FINDMSG,
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
	m_timerID = 0;
	m_iNewMsgs =0;
	m_bStartedIconic = FALSE;
	m_pToolBar = NULL;
	m_pSearchBar = NULL;
	m_bFlushBars = FALSE;
	m_pToolBarAd = NULL;
	m_pBitmapToolBar = NULL;
	m_iAdsShown = 0;
	m_padSponsor = NULL;
	m_nSponsorBitmapWidth = 0;
	m_nSponsorBitmapHeight = 0;
	m_pActiveMboxTreeCtrl		= NULL;	// FORNOW, backwards compatibility hack
	m_pActiveFilterReportView	= NULL;	// FORNOW, backwards compatibility hack
	m_pActiveLookupView			= NULL;	// FORNOW, backwards compatibility hack
	m_pActivePersonalityView	= NULL;	// ditto
	m_pActiveTaskStatusView		= NULL;	// ditto
	m_pDirectoryServicesWazooWndNew	=	NULL ;  // ditto

	// Various cached menu pointers
	m_hFileMenu = NULL;
	m_hEditMenu = NULL;
	m_pEditInsertRecipientMenu = NULL;
	m_pEditMessagePluginsMenu = NULL;
	m_pMailboxMenu = NULL;
	m_hMessageMenu = NULL;
	m_pMessageNewMessageToMenu = NULL;
	m_pMessageForwardToMenu = NULL;
	m_pMessageRedirectToMenu = NULL;
	m_pMessageNewMessageWithMenu = NULL;
	m_pMessageReplyWithMenu = NULL;
	m_pMessageReplyToAllWithMenu = NULL;
	m_pMessageAttachMenu = NULL;
	m_pMessageChangePersonalityMenu = NULL;
	m_hMessageChangeLabelMenu = NULL;
	m_pTransferMenu = NULL;
	m_hSpecialMenu = NULL;
	m_pSpecialRemoveRecipientMenu = NULL;
	m_pSavedSearchMenu = NULL;
	m_hToolsMenu = NULL;
	m_hWindowMenu = NULL;
	m_hHelpMenu = NULL;

	m_bTransferMenuBeingUsed = FALSE;

	// DRW - Moved here from CAdView now that ads are served here.
	m_ldb = 0;

	m_pControlBarManager = DEBUG_NEW_MFCOBJ_NOTHROW QCToolBarManager( this );
}

CMainFrame::~CMainFrame()
{
	// DRW - Moved here from CAdView now that ads are served here.
	MGR_Shutdown(m_ldb);

	delete m_pControlBarManager;
	if (m_pBitmapToolBar)
	{
		delete m_pBitmapToolBar;
		m_pBitmapToolBar = NULL;
	}
	if (m_padSponsor)
	{
		delete m_padSponsor;
		m_padSponsor = NULL;
	}

	CAdInfo			*ptbiTemp = NULL;
	for (int i = 0; i < m_arrayToolBarAds.GetSize(); ++i)
	{
		ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(i);
		delete ptbiTemp;
	}
}


bool CMainFrame::FinishInitAndShowWindow(
	int			nWindowState,
	CWnd *		pAboutDlg)
{
	QCToolBarManager *		pMgr = reinterpret_cast<QCToolBarManager *>(m_pControlBarManager);

	if (!pMgr)
		return false;

	//	Do the first half of what CMainFrame::LoadBarState (or more accurately
	//	SECMDIFrameWnd::LoadBarState) used to do. This will allow us to check
	//	for the previous existence of the Search Bar (or any other future
	//	dynamically created toolbars).
	SECDockState state(m_pControlBarManager);
	if (AfxGetThread()->m_pMainWnd == NULL)
		AfxGetThread()->m_pMainWnd = this;
	state.LoadState( _T("ToolBar") );

	//	Look for the previous existence of the Search Bar or failing that look
	//	for the info for the main toolbar.
	bool	bSearchBarConfigInINI = false;
	DWORD	dwAlignStyle = 0;
	CRect	rectSearchBar(0, 0, 0, 0);

	for (int i = 0; (i < state.m_arrBarInfo.GetSize()) && !bSearchBarConfigInINI; i++)
	{
		SECControlBarInfo *		pInfo = reinterpret_cast<SECControlBarInfo *>( state.m_arrBarInfo[i] );
		ASSERT(pInfo != NULL);

		if (!pInfo)
			break;

		switch(pInfo->m_nBarID)
		{
			case AFX_IDW_TOOLBAR:
				//	Grab the alignment of the main toolbar in case we don't find the Search Bar.
				dwAlignStyle = pInfo->m_dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
				break;

			case IDC_SEARCH_BAR:
				//	We found the Search Bar. Remember that we found it and grab some key
				//	information while we're at it.
				dwAlignStyle = pInfo->m_dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
				rectSearchBar = pInfo->m_rectMRUDockPos;
				bSearchBarConfigInINI = true;
				break;
		}
	}

	//	Create and initialize the Search Bar
	m_pSearchBar = DEBUG_NEW_MFCOBJ_NOTHROW CSearchBar;

	if (!m_pSearchBar)
	{
		ASSERT(0);
		return false;
	}

	m_pSearchBar->SetManager(pMgr);
	m_pSearchBar->m_bAutoDelete = TRUE;

	if( !m_pSearchBar->Create(this, dwAlignStyle) )
	{
		ASSERT(0);
		return false;
	}

	m_pSearchBar->EnableDocking(CBRS_ALIGN_ANY);

	if (bSearchBarConfigInINI)
	{		
		//	The Search Bar was already created in a previous session, therefore
		//	we should dock and show it now so that when we SetDockState below
		//	it's previous state (docking location, etc.) can be restored.
		DockControlBar(m_pSearchBar, static_cast<CDockBar *>(NULL), &rectSearchBar);
		m_pSearchBar->ShowWindow( GetIniShort(IDS_INI_SHOW_SEARCH_BAR) ? SW_SHOWNA : SW_HIDE );
	}

	// In the special maximized window handling, I may have introduced a bug that
	// causes Eudora not to open on the correct monitor for multiple monitor
	// systems. For now just bail if the number of monitors is more than one - I
	// can improve if I later get a multiple monitor setup with which to test.
	// Note that we can only detect multiple monitors if using Windows 2000
	// or better.
	int			nNumMonitors = 1;
	CRect		rectNormal;

	if ( IsWinNT2000() )
		nNumMonitors = GetSystemMetrics(SM_CMONITORS);

	if ( (nWindowState == SW_SHOWMAXIMIZED) && (nNumMonitors == 1) )
	{
		//	Remember the normal size of the window so that we can restore it after
		//	we show the window below.
		GetWindowRect(rectNormal);

		//	Get the rect of the entire desktop
		CRect		rectDesktop;
		CWnd::GetDesktopWindow()->GetWindowRect(rectDesktop);

		//	The previous window state was maximized. By setting the window to take up
		//	the entire desktop we avoid the toolbar state setting code below from
		//	shifting to the left any toolbars that would have fallen outside of the
		//	non-maximized / entire desktop rect.
		//	We do this via SetWindowPos telling it not to activate, draw, or affect
		//	the layering order so that this change is not visible to the user at all
		//	(unlike actually maximizing the window via ShowWindow or SetWindowPlacement).
		SetWindowPos( NULL,
					  rectDesktop.left, rectDesktop.top, rectDesktop.Width(), rectDesktop.Height(),
					  SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER );
	}

	//	Set the dock state now that the Search Bar has been initialized (if appropriate).
	//	This completes the work previously done by CMainFrame::LoadBarState (or more
	//	accurately SECMDIFrameWnd::LoadBarState).
	SetDockState(state);
	pMgr->LoadState(_T("ToolBar"));

	//	Detemine if we're currently using large toolbar buttons.
	BOOL		bEnableLargeButtons = (GetIniShort(IDS_INI_SHOW_LARGEBUTTONS) != 0);

	if (!bSearchBarConfigInINI)
	{
		//	The Search Bar was never created in any previous session (at least none
		//	that got saved), so create it now. We just want to end up to the right
		//	or below the main toolbar. The easiest way to do this seems to be
		//	to just dock ourselves one "column" farther than the main toolbar
		//	(this seems to do the correct thing regardless of whether the alignment
		//	is horizontal or vertical because the definition of "column" seems to
		//	change appropriately).
		int				nCol = 0;
		int				nRow = 0;
		QCDockBar *		pDockBar = reinterpret_cast<QCDockBar *>(m_pToolBar->m_pDockBar);
		
		if (pDockBar)
		{
			if ( !pDockBar->FindControlBarLocation(m_pToolBar, nCol, nRow) )
			{
				nCol = 0;
				nRow = 0;
			}
		}	
		
		int		nDefaultHeight = bEnableLargeButtons ?
								 CSearchBar::kLargeToolbarButtonHeight :
								 CSearchBar::kSmallToolbarButtonHeight;
		DockControlBarEx(m_pSearchBar, m_pToolBar->m_pDockBar->GetDlgCtrlID(), nCol+1, nRow, 1.0, nDefaultHeight);
	}

	// After loading the toolbars but before the ad toolbar is created
	// remove any ad toolbars that were inadvertently saved.
	RemoveBogusAdToolBars();

	if( pMgr->ConvertOldStuff() )
		SaveBarState(_T("ToolBar"));

	// DRW - Both Eudora and the SEC toolbar maintain their own INI entries
	// for cool look and large buttons and when LoadState() is called above
	// it uses the SEC INI entries and ignores Eudora's entries.  Since we really
	// want to use the Eudora entries (the entries that the "Tools|Options"
	// dialog uses) we should check the Eudora INI entries after the
	// SEC toolbar code has had its shot.
	pMgr->EnableCoolLook(GetIniShort(IDS_INI_SHOW_COOLBAR) != 0);
	pMgr->QCEnableLargeBtns(bEnableLargeButtons);

	//
	// Restore the Wazoo windows to their Wazoo containers...  IMPORTANT!
	// This must be done AFTER you call CMainFrame::LoadBarState(),
	// otherwise very bad things will happen.
	// (Or equivalently, CMainFrame::SetDockState now that we're doing
	// the work of CMainFrame::LoadBarState in two steps).
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
	// Immediately after the main window is initialized, we need to
	// inform any visible wazoo containers to choose an active
	// wazoo and perform the Just In Time display initializations for
	// those wazoos.
	//
	SetActiveWazooWindows();

	// Reset DS resize logic.
	ResetDSResize();

	// Adjust the splash screen so that it stays on top and doesn't move
	// or get drawn with the wrong colors.
	CRect		rectAbout;
	if (pAboutDlg)
	{
		/*
		Removed to compile with VS .Net

		CEudoraApp *	pEudoraApp = reinterpret_cast<CEudoraApp *>( AfxGetApp() );
		ASSERT(pEudoraApp);
		
		if (pEudoraApp)
		{
			// The SEC stuff messes with the system color, which would make the splash
			// screen get redrawn as gray. Force the dialog oolors to the system colors.
			pEudoraApp->ResetDialogColors();
		}
		*/

		// Show ourselves without activating so that we show behind the splash screen.
		ShowWindow(SW_SHOWNA);

		// Get the splash screen rect before changing its parent
		pAboutDlg->GetWindowRect(&rectAbout);

		// Make ourselves the splash screen parent
		pAboutDlg->SetParent(this);

		// Translate the splash screen position so that we can move it back to
		// where it was before the user sees anything drawn.
		ScreenToClient(&rectAbout);

		// Move the splash screen back to where it was. Tell it not to redraw
		// to avoid needless flickering.
		pAboutDlg->SetWindowPos( &CWnd::wndTop,
								 rectAbout.left, rectAbout.top,
								 rectAbout.Width(), rectAbout.Height(),
								 SWP_NOREDRAW );

		// Get the screen coordinates of the splash screen so that we can move it
		// again to compensate for any movement we do if we're maximized.
		pAboutDlg->GetWindowRect(&rectAbout);
	}

	// Now show ourselves so that we can maximize, etc. if necessary.
	ShowWindow(nWindowState);

	if (pAboutDlg)
	{
		// Translate the splash screen position so that we can move it back to
		// where it was before the user sees anything drawn.
		ScreenToClient(&rectAbout);

		// Move the splash screen back to where it was. Tell it not to redraw
		// to avoid needless flickering.
		pAboutDlg->SetWindowPos( &CWnd::wndTop,
								 rectAbout.left, rectAbout.top,
								 rectAbout.Width(), rectAbout.Height(),
								 SWP_NOREDRAW );
	}

	if ( (nWindowState == SW_SHOWMAXIMIZED) && (nNumMonitors == 1) )
	{
		//	The window is maximized. We messed with the normal rect above to make
		//	sure that the toolbar code didn't incorrectly shift any toolbars to
		//	the left.
		//	Restore the normal rect now. Need to do this after showing the window
		//	above or the toolbar code will shift the toolbars to the left.
		WINDOWPLACEMENT		wp;

		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&wp);
		wp.rcNormalPosition = rectNormal;
		wp.showCmd = SW_SHOWMAXIMIZED;
		SetWindowPlacement(&wp);
	}

	// Lets create and start our timer function
	SetGeneralTimer();

	// Create the popup text window
	CPopupText::Initialize(CPoint(0,0), this);

	// Initialize the playlist manager
	m_ldb = MGR_Init( "Eudora", EudoraDir, "EudPriv\\Ads\\", (long)this, PlaylistCallback,
		GetSharewareMode(), GetAdFailure() );

	return true;
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
			EmoticonMenu menuEmoticon;
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

			// get the menu string
			menuMain.GetMenuString( ID_EDIT_EMOTICON, szMenuLabel, MF_BYCOMMAND );

			// modify the Edit/Text/Color menu item
			menuMain.ModifyMenu(	ID_EDIT_EMOTICON,
									MF_BYCOMMAND | MF_POPUP , 
									(UINT) menuEmoticon.GetSafeHmenu(), 
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

				DebugPopup.AppendMenu(MF_SEPARATOR);

				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_NEW_PLAYLIST, "Request Pla&ylist");
				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_BRAND_NEW_PLAYLIST, "Request &Brand New Playlist");
				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_DUMP_ADSTATE, "Dump Ad &State");

				DebugPopup.AppendMenu(MF_SEPARATOR);

				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_PLAYLIST_SERVER0, "Production Playlist Server");
				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_PLAYLIST_SERVER1, "Trunk on Echo");
				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_PLAYLIST_SERVER2, "Eitanzone on Adserver");
				DebugPopup.AppendMenu(MF_STRING, ID_DEBUG_PLAYLIST_SERVER3, "PLRestore on Echo");

				
				menuMain.AppendMenu(MF_POPUP, (UINT)(DebugPopup.Detach()), "&Debug");
			}
#endif // _DEBUG

			
			// detach both menus from the objects
			menuColor.Detach();
			menuEmoticon.Detach();
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
		AfxRegisterClass(&wndcls);
	}
	cs.lpszClass = EudoraMainWindowClass;
	return bRes;
}

// Since Debug builds have a non-standard "Debug" menu after Help, the
// standard MFC method must be overridden to setup the Window menu
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pCreateContext)
{
#ifdef _DEBUG
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
#else
	return QCWorkbook::OnCreateClient(lpcs, pCreateContext);
#endif
}



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
	m_pWBClient = DEBUG_NEW QCWorkbookClient();
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
	if (!m_bFlushBars)
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

	FlushINIFile();
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
	if (contains_compatible_bar_state && !m_bFlushBars)
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
	    if (num_sections <= 0 || num_sections > 100) {
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
	// Flush the [WazooBars] section.
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
					char* pszAtSign = strrchr(szPOPUserName, '@');
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

	if (nState == WA_INACTIVE && bMinimized && GetIniShort(IDS_INI_SHOW_ONLY_IN_TRAY))
	{
		// Force tray item to show up
		SetIcon(-1, -1, TRUE);
	}
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	if ( SearchManager::Instance() )
		SearchManager::Instance()->OnActivateApp(bActive);
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

	// Change the toolbar color scheme (we do this by forcing a reload
	// of whatever toolbar resource we are currently using).
	QCToolBarManager* pMgr = (QCToolBarManager*)m_pControlBarManager;
	if (pMgr)
	{
		pMgr->QCEnableLargeBtns(GetIniShort(IDS_INI_SHOW_LARGEBUTTONS), TRUE);
		RefreshAdToolBar();
	}

//	SendMessageToDescendants(WM_SYSCOLORCHANGE, 0, 0L, TRUE, TRUE);
	SendMessageToDescendants(WM_SYSCOLORCHANGE);

	// Try to make sure that our menu color changes appropriately
	FixMenuDrawingIfXP();
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

	// DRW - Moved cool look and large button toolbar attribute handling
	// to below pMgr->LoadState(_T("ToolBar")).  See comment there.

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
	DragAcceptFiles();
	        
	EnableDocking(CBRS_ALIGN_ANY);

	m_pToolBar = DEBUG_NEW_MFCOBJ_NOTHROW QCCustomToolBar;  
	
	if( m_pToolBar == NULL )
	{
		ASSERT( 0 );
		return -1;
	}

	m_pToolBar->SetManager( pMgr );
	m_pToolBar->m_bAutoDelete = TRUE;
	dwStyle = CBRS_TOP | CBRS_SIZE_DYNAMIC;
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

	if (GetIniShort(IDS_INI_SHOW_TOOLBAR))
		m_pToolBar->ShowWindow( SW_SHOWNA );
	else
		m_pToolBar->ShowWindow( SW_HIDE );

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

	//
	// And what the hell, let's register our OLE drag and drop object here...
	//
	VERIFY(m_dropTarget.Register(this));

	// Also read Eudora.ini for the POP_ACCOUNT field and split it into POP_SERVER and LOGIN_NAME
	// just for backward compatibility 
	SplitPOPACCOUNT() ;

	// Register this application as a running automation server
	// if automation is enabled
	if (gbAutomationEnabled)
	{
		AutomationRegisterFactory();
	}
	
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

	switch (uMouseMsg)
	{
	case WM_LBUTTONDBLCLK:
		OnOpenEudora();
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			CMenu menu;
			CMenu* pPopupMenu;

			if (menu.Attach(QCLoadMenu(IDR_SYSTEM_TRAY_MENU)) && (pPopupMenu = menu .GetSubMenu(0)))
			{
				CPoint point;
				UINT nID;

				SetMenuDefaultItem(*pPopupMenu, ID_SYSTEM_TRAY_OPEN_EUDORA, FALSE);
				GetCursorPos(&point);

				// Need to use TPM_RETURNCMD so that we can set and reset g_bPopupOn
				UINT uFlags = TPM_RETURNCMD | (uMouseMsg == WM_LBUTTONUP? TPM_LEFTBUTTON : TPM_RIGHTBUTTON);
				g_bPopupOn = TRUE;
				nID = pPopupMenu->TrackPopupMenu(uFlags, point.x, point.y, this);
				if (nID)
				{
					// Show main window if we're going to do New Message or Check Mail
					if ((nID == ID_MESSAGE_NEWMESSAGE || nID == ID_FILE_CHECKMAIL) && !IsWindowVisible())
						OnOpenEudora();
					PostMessage(WM_COMMAND, nID, 0);
				}
				g_bPopupOn = FALSE;
			}
		}
		break;
	}

  return 0;
}


BOOL CMainFrame::TrayItem(int iIcon, int iOperation)
{
	NOTIFYICONDATA nid;

	nid.cbSize = sizeof(nid);					//  Required by API

	nid.hWnd = m_hWnd;							//  Notify this window
	nid.uID = iIcon + 999;						//  Give tray item some ID starting at 1000
	nid.uFlags = NIF_MESSAGE |					//  We got a message
				 NIF_ICON |						//  an icon
				 NIF_TIP;						//  and tooltip text
	nid.uCallbackMessage = WM_TRAY_CALLBACK;	//  Use WM_TRAY_CALLBACK for messages

	HINSTANCE hResInst = QCFindResourceHandle( MAKEINTRESOURCE(m_IconID), RT_GROUP_ICON );
	nid.hIcon = (HICON) ::LoadImage(hResInst, MAKEINTRESOURCE(m_IconID), IMAGE_ICON, 16, 16, LR_SHARED);

	char szTip[64];
	if (iOperation == NIM_DELETE)
		szTip[0] = 0;
	else
	{
		strcpy(szTip, CRString(IDS_PRODUCT_NAME));
		if (m_iNewMsgs > 0)
		{
			strcat(szTip, ": ");
			sprintf(szTip + strlen(szTip), (const char*)CRString(m_iNewMsgs == 1? IDS_ONE_NEW_MESSAGE : IDS_MULTIPLE_NEW_MESSAGES), m_iNewMsgs);
		}
	}

	lstrcpyn(nid.szTip, szTip, sizeof(nid.szTip));

	Shell_NotifyIcon(iOperation, &nid);

	return (TRUE);
}

void CMainFrame::SetGeneralTimer(UINT nElapse /* = kGeneralTimerPeriod */)
{
	if (m_timerID)
		KillTimer(m_timerID);

	VERIFY(m_timerID = SetTimer(GENERAL_BACKGROUND_TIMER_ID, nElapse, NULL));
}

// SetIcon
// Set the icon that gets displayed when minimized.
// HaveMailStatus can be set to TRUE or FALSE, or -1 means keep current HaveMail status.
// TrayItem ignores any calls made by a 16bit app.
void CMainFrame::SetIcon(int HaveMailStatus, int iNewMsgCount, BOOL bForceChange /*= FALSE*/)
{
	UINT IconID = IDI_NO_MAIL;
	const BOOL bShowOnlyInTray = GetIniShort(IDS_INI_SHOW_ONLY_IN_TRAY);
	const BOOL bKeepTrayIcon = GetIniShort(IDS_INI_ALWAYS_SHOW_TRAY_ICON) ||
								(IsIconic() && bShowOnlyInTray);

	if (!bShowOnlyInTray && !IsWindowVisible())
		ShowWindow(SW_SHOWNA);
	else if (bShowOnlyInTray && IsWindowVisible() && IsIconic())
		ShowWindow(SW_HIDE);

	if (HaveMailStatus == TRUE || (HaveMailStatus == -1 && m_IconID >= IDI_HAVE_MAIL))
		IconID += 2;
	if (QueueStatus & QS_READY_TO_BE_SENT)
		IconID++;
	if (bForceChange || IconID != m_IconID)
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
		TrayItem(IDR_MAINFRAME, NIM_DELETE); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
		if ( bKeepTrayIcon || IconID >= IDI_NO_MAIL + 2 )
			TrayItem(IDR_MAINFRAME, NIM_ADD); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
	}
	else if ( HaveMailStatus == TRUE )
	{
		m_iNewMsgs += iNewMsgCount;
	 	TrayItem(IDR_MAINFRAME, NIM_MODIFY); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
	}
	else
	{
		m_iNewMsgs = 0;
		if (bKeepTrayIcon)
		 	TrayItem(IDR_MAINFRAME, NIM_MODIFY); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT
	}
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

////////////////////////////////////////////////////////////////////////
// EnableDocking [public]
//
// This is really an override, but the base class doesn't declare this
// function as virtual, so always be sure to call this with a pointer
// to this class and not a base class pointer.
//
// The reason we're overriding it is to create our own derived class
// for dock bars so that we can do some customizations.
////////////////////////////////////////////////////////////////////////
void CMainFrame::EnableDocking(DWORD dwDockStyle)
{
	// Code pilfered from OT, and slightly modified to create our own dock bar
	for (int i = 0; i < 4; i++)
	{
		if (dwSECDockBarMap[i][1] & dwDockStyle & CBRS_ALIGN_ANY)		   // protected
		{
			CDockBar* pDock = (CDockBar*)GetControlBar(dwSECDockBarMap[i][0]);
			if (pDock == NULL)
			{
				pDock = DEBUG_NEW QCDockBar;
				pDock->Create(this, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE |
									dwSECDockBarMap[i][1], dwSECDockBarMap[i][0]);
			}
		}
	}

	QCWorkbook::EnableDocking(dwDockStyle);
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

void CMainFrame::DragAcceptFiles(BOOL bAccept /*= TRUE*/)
{
	QCWorkbook::DragAcceptFiles(bAccept);
	m_wndStatusBar.DragAcceptFiles(bAccept);
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

	// Turn off redrawing so that Eudora opens the previously open windows
	// faster (no time wasted redrawing each only to be immediately obscurred)
	// and so that startup looks smoother.
	SetRedraw(FALSE);

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
					if (!(s = strtok(NULL, ",")))
					{
						ASSERT(FALSE);
	            		continue;
					}
					int Index = atoi(s);
					if ( !Toc || (Index < 0) )
					{
						ASSERT( !"Bad TOC or index" );

						continue;
					}

					CSumList &		listSums = Toc->GetSumList();
					POSITION		pos = listSums.FindIndex(Index);

					if (!pos)
					{
						// A really common thing is to create a new composition message and
						// then quit before making any modifications to it.  When that happens
						// the comp message gets saved in to the open window list, but then
						// gets removed when the window is closed.  So don't ASSERT when the
						// message index is just one more than the number of messages in the
						// Out mailbox.
						ASSERT( (WindowType == IDR_COMPMESS) || (Index == listSums.GetCount()) );

						continue;
					}

					CSummary *		Sum = listSums.GetAt(pos);

					if (!Sum)
					{
						// This should not happen. but if does lets ASSERT
						ASSERT( !"Unable to find summary from POSITION" );
					}
					else
					{
						Sum->Display();
					}
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

		case IDR_USAGE_STATISTICS:
			OnViewUsageStatistics();
		}
	}

	// Restore the value to display child windows normally
	CMDIChild::m_nCmdShow = -1;

	PurgeIniSetting(IDS_INI_OPEN_WINDOWS_FAILED);

	// Turn redrawing back on
	SetRedraw(TRUE);

	// Redraw the main frame and all child windows
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

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
			strcpy(Path, Toc->GetMBFileName());
			// Only save part of path past the main Eudora directory
			wsprintf(String + strlen(String), ",%s,%s", Path + EudoraDirLen, (const char*)Toc->Name());
			if (Sum)
			{
				int Index = Toc->GetSumList().GetIndex(Sum);
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

			static TCHAR szBars[] = "Bars";

			// Due to some reason, once in a while, the Eudora.ini file is filled with a lot of Bar#n=0 lines.
			// Because of that, Eudora's response time gradually decreases. The more such entries, the worse the response.			
			
			// The following if loop is an attempt to clean such IRRELEVANT entries.

			// If we have multiple entries with Bar#n = 0, then condense them into one entry of Bar#x = 0, 
			// (where x = start of the series of the multiple entries). i.e if we have 
			// Bar#0 = 0, Bar #1 = 1, Bar#2 = 0, Bar#3 = 0, Bar#4 = 0, Bar#5 = 1, Bar#6 = 0 as the entries,
			// then the final output after the following if loop will be :
			// Bar#0 = 0, Bar #1 = 1, Bar#2 = 0, Bar#3 = 1, Bar#4 = 0.			
			
			// Appropriately update the "Bars" value to reflect the new number of bars.i.e 
			// In first case original Bars = 7 has been changed to Bars = 5

			// Query if there is a "Bars" entry
			if (RegQueryValueEx(hSecKey, szBars, NULL, &DataType, (LPBYTE)ValueData, &(ValueDataSize = sizeof(ValueData))) == ERROR_SUCCESS)
			{
				// we found a "Bars" entry

				static DWORD i = 0;
				DWORD dwCurValue = 0, dwPrevValue = 0;
				int nReplaceSeriesNumber = 0;

				TCHAR ActualValue[MAX_PATH + 1];
				BOOL bFoundFirst = FALSE;

				// It should have a value type REG_DWORD..ASSERT if not
				if (DataType == REG_DWORD)			
					i = *((DWORD*)ValueData);
				else
					ASSERT(0);

				// Number of Bars cannot be negative
				ASSERT(i >= 0);				

				for (DWORD k = 0; k < i; k++)
				{
					// Look for a "Bar#n" entry
					sprintf(ValueName,"Bar#%d",k);
					if (RegQueryValueEx(hSecKey, ValueName, NULL, &DataType, (LPBYTE)ValueData, &(ValueDataSize = sizeof(ValueData))) == ERROR_SUCCESS)
					{
						dwCurValue = *((DWORD*)ValueData);

						/*
						// The condensation of entries start only after we have atleast found a positive value. i.e if we have 
						// Bar#0 = 0, Bar #1 = 0, Bar#2 = 0, Bar#3 = 0, Bar#4 = 1, Bar#5 = 0, Bar#6 = 0, then after the 
						// following if loop is executed, output will be :
						// Bar#0 = 0, Bar #1 = 0, Bar#2 = 0, Bar#3 = 0, Bar#4 = 1, Bar#5 = 0.	
						// We should not replace any values that are zeros till we find atleast one non-zero (positive) value

						// The above mentioned commented logic is now being ignored. Now we condense any successive zeros into just one zero
						// Uncomment the next if loop & comment the one following it, if required to go back to the above mentioned logic.
						*/
						/*if ( (dwCurValue > 0) && !bFoundFirst)
						{
							bFoundFirst = TRUE;
							nReplaceSeriesNumber = k;
						}*/

						// Now we are replacing successive zero's whenever we encounter them irrespective of whether we have found 
						// atleast one positive value or not
						if ( !bFoundFirst)
						{
							bFoundFirst = TRUE;
							nReplaceSeriesNumber = k-1;
						}
						else if (bFoundFirst)
						{
							if (dwCurValue == 0 )
							{
								if(dwPrevValue == 0)
								{
									// Delete the value if both the dwPrevValue & dwCurValue are zero
									RegDeleteValue(hSecKey,ValueName);
									continue;								
								}								
							}
						}
						
						nReplaceSeriesNumber++;							

						// We want to modify the registry keys (update the Bar# label), but the only way i know is to delete & create again. 
						// So here it goes..

						// Delete the value
						RegDeleteValue(hSecKey,ValueName);							
						
						// Compose a new value
						sprintf(ValueName,"Bar#%d",nReplaceSeriesNumber);
						sprintf(ActualValue,"%d",dwCurValue);
						
						// Set the new value.
						RegSetValueEx(hSecKey,ValueName,0, REG_DWORD,(const PBYTE) &dwCurValue, sizeof( DWORD ));
					}
					// Lets update the prev value to the current value so that in the next iteration, we can have both
					dwPrevValue = dwCurValue;
				}

				if(bFoundFirst)
				{
					// Do this only if we have found a case of multiple Bar#n = 0, after atleast one  Bar#?=positive value.
					nReplaceSeriesNumber++;
					RegSetValueEx(hSecKey,szBars,0, REG_DWORD,(const PBYTE) &nReplaceSeriesNumber, sizeof( DWORD ));
					
				}
			}


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
	//	Relay event to tooltip style popup text so that it can cancel itself if necessary
	if ( CPopupText::WasCreated() )
		CPopupText::Instance()->RelayEvent(pMsg);
	
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
	WORD	wCommandID = HIWORD(wParam);
	if (wCommandID <= 1)
		wCommandID = LOWORD(wParam);

#ifdef _DEBUG
	TRACE("CMainFrame::OnCommand -- got command %d (0x%X)\n", wCommandID, wCommandID);
#endif

	CWazooWnd* pWazooWnd = GetWazooWindowWithFocus();
	if (pWazooWnd)
	{
#ifdef _DEBUG
		TRACE("CMainFrame::OnCommand -- routing command %d (0x%X) to wazoo\n", wCommandID, wCommandID);
#endif
		ASSERT_KINDOF(CWazooWnd, pWazooWnd);
		if (AfxCallWndProc(pWazooWnd, pWazooWnd->m_hWnd, WM_COMMAND, wParam, lParam) != 0)
			return TRUE; // handled by wazoo
	}

	// If we're processing an edit command and we have a focused edit field
	// that is not a child of an MDI child window (e.g. the Search Bar),
	// then bypass our traditional processing which would route the command
	// to the active MDI child (ignoring the focused edit field).
	switch (wCommandID)
	{
		case ID_EDIT_CLEAR:
		case ID_EDIT_COPY:
		case ID_EDIT_CUT:
		case ID_EDIT_PASTE:
		case ID_EDIT_SELECT_ALL:
		case ID_EDIT_UNDO:
			// Ok, we're processing an edit command
			CEdit *		pEdit = GetCurrentEdit();

			if (pEdit)
			{
				// We have an edit field, is it a child of an MDI child window?
				// If not, then bypass our traditional processing.
				CFrameWnd *		pParentFrame = pEdit->GetParentFrame();

				if ( !pParentFrame || !pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)) )
					return CFrameWnd::OnCommand(wParam, lParam);
			}
			break;
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

	// If we're processing an edit command and we have a focused edit field
	// that is not a child of an MDI child window (e.g. the Search Bar),
	// then bypass our traditional processing which would route the command
	// to the active MDI child (ignoring the focused edit field).
	CEdit *		pEdit = NULL;

	switch (nID)
	{
		case ID_EDIT_CLEAR:
		case ID_EDIT_COPY:
		case ID_EDIT_CUT:
		case ID_EDIT_PASTE:
		case ID_EDIT_SELECT_ALL:
		case ID_EDIT_UNDO:
			// Ok, we're processing an edit command
			pEdit = GetCurrentEdit();

			if (pEdit)
			{
				// We have an edit field, is it a child of an MDI child window?
				// If not, then we'll bypass our traditional processing.
				CFrameWnd *		pParentFrame = pEdit->GetParentFrame();

				if ( pParentFrame && pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)) )
					pEdit = NULL;
			}
			break;
	}

	// Kept previous code that bypasses the active MDI child window if it's not visible.
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if ( pEdit || (pActiveChild && !pActiveChild->IsWindowVisible()) )
		return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

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
	// Destroy the popup text window
	CPopupText::Destroy();
	
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
		lpMIS->itemHeight = __max(GetSystemMetrics(SM_CYMENUCHECK ), size.cy + 2);
	}
	else if ( ( MenuID >= ID_COLOR0 ) && ( MenuID <= ID_COLOR16 ) )
	{
		CColorMenu::DoMeasureItem( lpMIS );
	}
	else if ( g_theCommandStack.Lookup( ( WORD ) MenuID, &pObject, &theAction ) )
	{
		//ASSERT_KINDOF( QCPluginCommand, pObject );
		//pObject->Execute( CA_MEASURE, lpMIS );

		//ASSERT(pObject->IsKindOf(RUNTIME_CLASS(QCPluginCommand) || pObject->IsKindOf(RUNTIME_CLASS(Emoticon));

		if(pObject->IsKindOf(RUNTIME_CLASS(QCPluginCommand)))
			pObject->Execute( CA_MEASURE, lpMIS );
		else if(pObject->IsKindOf(RUNTIME_CLASS(Emoticon)))
			EmoticonMenu::DoMeasureItem( lpMIS );
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
		//ASSERT_KINDOF( QCPluginCommand, pCommand );

		//ASSERT(pCommand->IsKindOf(RUNTIME_CLASS(QCPluginCommand) || pCommand->IsKindOf(RUNTIME_CLASS(Emoticon));

		if(pCommand->IsKindOf(RUNTIME_CLASS(Emoticon))) {
			EmoticonMenu::DoDrawItem( lpDIS );
		} else { 
			ASSERT_KINDOF( QCPluginCommand, pCommand );
			pCommand->Execute( CA_DRAW, lpDIS );
		}

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


void CMainFrame::OnUpdateToggleOffline(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( GetIniShort(IDS_INI_CONNECT_OFFLINE) ? 1 : 0 );
}



void CMainFrame::OnFileToggleOffline()
{
	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);
	}
	else
	{
		SetIniShort(IDS_INI_CONNECT_OFFLINE, 1);
	}
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

	CRichEditCtrl* RichEdit = GetCurrentRichEdit();
	if (RichEdit)
		return ((RichEdit->GetStyle() & ES_READONLY)? FALSE : TRUE);

	CWnd* pWnd = GetFocus();
	if (pWnd)
	{
		QCProtocol* QP = QCProtocol::QueryProtocol(QCP_TRANSLATE, pWnd);
		if (QP)
			return !QP->IsReadOnly();
	}

	return (FALSE);
}

BOOL CMainFrame::GetSelectedText(CString& SelText)
{
	// Good hygiene
	SelText.Empty();

	CEdit* Edit = GetCurrentEdit();
	if (Edit)
	{
		int Start, End;
		Edit->GetSel(Start, End);
		if (Start != End)
		{
			if (Start > End)
				swap(Start, End);
			Edit->GetWindowText(SelText);
			SelText = SelText.Mid(Start, End - Start);
			return TRUE;
		}

		return FALSE;
	}

	CRichEditCtrl* RichEdit = GetCurrentRichEdit();
	if (RichEdit)
	{
		if (RichEdit->GetSelectionType() & SEL_TEXT)
		{
			SelText = RichEdit->GetSelText();
			return TRUE;
		}

		return FALSE;
	}

	CWnd* pWnd = GetFocus();
	while (pWnd)
	{
		QCProtocol* QP = QCProtocol::QueryProtocol(QCP_TRANSLATE, pWnd);
		if (!QP)
			QP = QCProtocol::QueryProtocol(QCP_GET_MESSAGE, pWnd);
		if (QP)
			return QP->GetSelectedText(SelText);
		pWnd = pWnd->GetParent();
	}

	return FALSE;
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
	pCmdUI->Enable(CanModifyEdit() && IsClipboardFormatAvailable(CF_TEXT));
}

void CMainFrame::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetCurrentEdit() != NULL) ||
					 (GetCurrentRichEdit() != NULL));
}

void CMainFrame::OnUpdateAttach(CCmdUI* pCmdUI)
{
	char	attachFileString[255];
	CMenu*	messageMenu = CMenu::FromHandle(m_hMessageMenu);

	// The Attach File Item can be Attach File or Attach to New Message depending what's on top
	if (messageMenu->GetMenuString(ID_MESSAGE_ATTACHFILE, attachFileString,
		sizeof(attachFileString), MF_BYCOMMAND))
	{
		CRString fileText(IDS_ATTACH_FILE_NEW_TEXT);

		if (stricmp(attachFileString, fileText))
		{
			messageMenu->ModifyMenu(ID_MESSAGE_ATTACHFILE, 
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

void CMainFrame::OnCtrlJ()
{
	// Need to use SendMessage() so that command routing can be done
	if (GetIniShort(IDS_INI_WHO_GETS_CTRL_J) == CTRL_J_JUNK)
		SendMessage(WM_COMMAND, ID_JUNK);
	else
		SendMessage(WM_COMMAND, ID_SPECIAL_FILTERMESSAGES); // Default Ctrl-F
}

void CMainFrame::OnShiftCtrlJ()
{
	// Need to use SendMessage() so that command routing can be done
	if (GetIniShort(IDS_INI_WHO_GETS_CTRL_J) == CTRL_J_JUNK)
		SendMessage(WM_COMMAND, ID_NOT_JUNK);
}

void CMainFrame::OnShiftCtrlL()
{
	// Need to use SendMessage() so that command routing can be done
	if (GetIniShort(IDS_INI_WHO_GETS_CTRL_J) == CTRL_J_JUNK)
		SendMessage(WM_COMMAND, ID_SPECIAL_FILTERMESSAGES);
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
		// Default is "Reply to All", but the shift key reverses the command to "Reply"
		PostMessage( WM_COMMAND, ShiftDown() ? ID_MESSAGE_REPLY : ID_MESSAGE_REPLY_ALL, 0 );
	}
	else
	{
		// Default is "Reply", but the shift key reverses the command to "Reply to All"
		PostMessage( WM_COMMAND, ShiftDown() ? ID_MESSAGE_REPLY_ALL : ID_MESSAGE_REPLY, 0 );
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

void CMainFrame::OnUpdateSpecialEmptyTrash(CCmdUI* pCmdUI)
{
	CTocDoc *		pTrashToc = GetTrashToc();

	if (!pTrashToc)
	{
		ASSERT(FALSE);
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(pTrashToc->NumSums() != 0);
	}
}

void CMainFrame::OnSpecialEmptyTrash()
{
	EmptyTrash();
}

void CMainFrame::OnUpdateTrimJunk(CCmdUI* pCmdUI)
{
	OnUpdateFullFeatureSet(pCmdUI);
}

void CMainFrame::OnSpecialTrimJunk()
{
	TrimJunk(true/*bUserRequested*/);
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
	INT				iOldCool;
	INT				iNewCool;
	INT				iOldLarge;
	INT				iNewLarge;

	//
	// If the user is holding down Shift and Ctrl, that means what they may want to do is to
	// "reset" Eudora.ini (strip it down to the bare essential settings). If so, we leave a
	// signature behind that CEudoraApp::ExitInstance() will look at it. We don't just set a
	// boolean because we want to be very sure it's what the user wanted and not the result
	// of a stray pointer or something.
	//
	// Removing the settings now doesn't work because a bunch of them will get written back out
	// again during the app's shutdown.
	//
	// ResetIniFileOnExit and its special value should really be in a header, but I didn't want
	// to change one of the commonly used ones and force everyone to recompile a zillion modules
	// (this is right before the 4.3 release), nor did I want to create a new one just for this
	// one little thing. - JR
	//
	if (ShiftDown() && CtrlDown())
	{
		if (AfxMessageBox(IDS_RI_OK_TO_PROCEED,
			              MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
		{
			extern long ResetIniFileOnExit;

			ResetIniFileOnExit = 0xCAFEB0EF;
			AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_APP_EXIT);
		}
		return;
	}
	
	QCToolBarManager* pMgr = (QCToolBarManager*)( m_pControlBarManager );
	if (pMgr)
	{
		iOldCool = GetIniShort( IDS_INI_SHOW_COOLBAR );
		iOldLarge = GetIniShort( IDS_INI_SHOW_LARGEBUTTONS );

		dlg.DoModal();

		iNewCool = GetIniShort( IDS_INI_SHOW_COOLBAR );
		iNewLarge = GetIniShort( IDS_INI_SHOW_LARGEBUTTONS );
			
		if(iNewCool != iOldCool )
		{
			pMgr->EnableCoolLook( iNewCool != 0 );
		}
		if (iNewLarge != iOldLarge)
		{
			pMgr->QCEnableLargeBtns(iNewLarge != 0);
			SwitchAdToolBarImages();
		}
		if ((iNewCool != iOldCool) || (iNewLarge != iOldLarge))
		{
			pMgr->SaveState( _T( "ToolBar" ) );
			RecalcLayout();
		}
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
	// If Shift was held down, grab the selected text of the
	// topmost window and do a query with it
	CString SelText;
	const BOOL bAutoQuery = ShiftDown();
	
	if (bAutoQuery)
		GetSelectedText(SelText);

	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(DirectoryServicesWazooWndNew));

	if (m_pDirectoryServicesWazooWndNew)
	{
		ASSERT_VALID(m_pDirectoryServicesWazooWndNew);
		ASSERT(::IsWindow(m_pDirectoryServicesWazooWndNew->m_hWnd));

		if (bAutoQuery && !SelText.IsEmpty())
			m_pDirectoryServicesWazooWndNew->DoQuery(SelText);
		else
			m_pDirectoryServicesWazooWndNew->GiveQueryInputFocus();
	}
}


void CMainFrame::OnViewLinkHistory()
{
	m_WazooBarMgr.ActivateWazooWindow(this, RUNTIME_CLASS(CLinkHistoryWazooWnd));
}


void CMainFrame::OnViewUsageStatistics()
{
	NewChildDocFrame(UsageStatsTemplate);
}

void CMainFrame::OnResetTools()
{
	if (WarnYesNoDialog(0, IDS_WARN_TOOLBAR_RESET) == IDOK)
	{
		// Reset toolbars/wazoobars
		m_bFlushBars = TRUE;
		FlushIncompatibleToolBarState();

		// Various other bar-related settings to go back to default
		PurgeIniSetting(IDS_INI_SHOW_TOOLBAR);
		PurgeIniSetting(IDS_INI_SEEN_PLUGINS);
		PurgeIniSetting(IDS_INI_SHOW_TOOLTIPS);
		PurgeIniSetting(IDS_INI_SHOW_COOLBAR);
		PurgeIniSetting(IDS_INI_SHOW_LARGEBUTTONS);
		PurgeIniSetting(IDS_INI_SHOW_STATUS_BAR);
		PurgeIniSetting(IDS_INI_MDI_TASKBAR);
		PurgeIniSetting(IDS_INI_SHOW_SEARCH_BAR);
		PurgeIniSetting(IDS_INI_SEARCH_BAR_WIDTH);
		PurgeIniSetting(IDS_INI_AD_TOOLBAR_WIN_POS);
		PurgeIniSetting(IDS_INI_AD_TOOLBAR_FLOATING);
		PurgeIniSetting(IDS_INI_AD_TOOLBAR_DOCKING);
		PurgeIniSetting(IDS_INI_SEARCH_BAR_WIDTH);

		PostMessage(WM_COMMAND, ID_APP_EXIT);
	}
}

void CMainFrame::NewMessage(LPCTSTR To, LPCTSTR From, LPCTSTR Subject, LPCTSTR Cc, 
							LPCTSTR Bcc, LPCTSTR Attach, LPCTSTR Body)
{
	CCompMessageDoc* pComp = NewCompDocument(To, From, Subject, Cc, Bcc, Attach, Body);
	
	if (pComp)
	{
		// Make sure Eudora gets brought to the foreground, even if it is minimized.
		ActivateFrame();
		SetForegroundWindow();

		// Display the new composition window.
		pComp->m_Sum->Display();
		pComp->SetModifiedFlag();
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

void CMainFrame::CreateMessage(LPCTSTR cmdLine)				
{
	CString csAttachLine = cmdLine;		
	
	try
	{
		// Since we are not sure how much would be the buffer size after all the command line file names are converted to 
		// the long filenames (which is done in GetAttachmentLine()), our best bet would be to pass a CString by reference
		// and let CString manage the task of buffer management	(reallocation, if required) - 02/03/00

		GetAttachmentLine(cmdLine, csAttachLine);
		if (FALSE == csAttachLine.IsEmpty())
			NewMessage(NULL, NULL, NULL, NULL, NULL, (LPCTSTR)csAttachLine, NULL);
		
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in CMainFrame::CreateMessage" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in CMainFrame::CreateMessage" );
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in CMainFrame::CreateMessage" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in CMainFrame::CreateMessage" );
	}
	
	return; 	
}

// Changed the second parameter from LPTSTR to CString, so that the memory re-allocation (if needed) would be handled automatically.- 02/03/00
void CMainFrame::GetAttachmentLine(LPCTSTR cmdLine, CString& csAttachLine)
{	
	char *c;
	struct stat FileAttr;
	BOOL DONE = FALSE;
	char path[_MAX_PATH +1];
	csAttachLine.Empty();

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

				//if (!*csAttachLine)
				if (TRUE == csAttachLine.IsEmpty())
				{
					csAttachLine = path;
				}
				else
				{
					csAttachLine += " ";
					csAttachLine += path;
				}

				// always end with a ';'
				csAttachLine += ";";
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
 	
	if (Msg == 1)
	{
		if (pcds->lpData)
		{
			// Make a copy of the cmdline before it goes away
			char* cmdLine = DEBUG_NEW char[strlen((char *)pcds->lpData) + 1];
			strcpy(cmdLine, (char *)pcds->lpData);

			((CEudoraApp*)AfxGetApp())->DoCommandLineActions(cmdLine);

			delete [] cmdLine;
		}

		return TRUE;
	}
 	

	//extern SetKerb16Ticket(PCOPYDATASTRUCT);
	if (Msg == SEND_TICKET || Msg == FAILED_SEND_TICKET)
	{
		//SetKerb16Ticket(pcds);
		QCKerberos::SetKerb16Ticket(pcds);
		return TRUE;
	}

	if (Msg >= MAPI_MSG_FIRST && Msg <= MAPI_MSG_LAST)
		return (HandleMAPIMessage(pcds));

	return FALSE;
}

void CMainFrame::OnHelpSearchForHelpOn()
{
	AfxGetApp()->WinHelp((DWORD)(LPCSTR)"", HELP_PARTIALKEY);
}

void CMainFrame::OnToolBarAd(UINT uCmdID)
{
	// Figure out which toolbar ad button was clicked and launch the appropriate URL.
	int				 iFoundAt = -1;
	CAdInfo			*ptbiTemp = NULL;
	for (int i = 0; (i < m_iAdsShown) && (iFoundAt == -1); ++i)
	{
		ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(i);
		if (ptbiTemp && (ptbiTemp->m_iCmd == uCmdID))	iFoundAt = i;
	}
	if ((iFoundAt != -1) && ptbiTemp)
	{
		LaunchURL(ptbiTemp->m_strURL);
	}
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

void CMainFrame::OnMoreHelp()
{
	::WinHelp(m_hWnd, CRString(IDS_MOREHELP_FILE), HELP_CONTENTS, 0L);
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
	// This code is required for handling the close of the print preview window.
	// MFC changes the print preview window as the MainFrame window of the app in release build.
	// So when one closes the print preview by clicking on the 'X' this is where it will land
	// The next 2 lines of code make sure that the close is indeed for close of app & not the 
	// close of print preview. If it is for close of print preview, then just bail out
	if (m_lpfnCloseProc != NULL && !(*m_lpfnCloseProc)(this))
		return;

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
	{
		KillTimer(m_timerID);
		m_timerID = 0;
	}

#ifdef OLDSTUFF
	// Save recipient list (member function checks whether or not a save is necessary)
	g_RecipientList.Write();
#endif

	Network::CleanSSLLibrary();
	// Winsock and Dialup must shutdown before the app can.
	if (NetConnection)
	{
		delete(NetConnection);
		NetConnection = NULL;
	}

	TrayItem(IDR_MAINFRAME,NIM_DELETE); // SHAREWARE. Pro: IDR_MAINFRAME, Light: IDR_MAINFRAME_LIGHT

	// Get rid of any dynamic menu C++ objects.  Don't need to delete the menu
	// items because that will be taken care of when the window is destoyed.
	CDynamicMenu::DeleteMenuObjects(GetMenu(), FALSE);

	QCWorkbook::OnClose();

	// Get rid of TOCs still in memory
	TocCleanup();
	
	// Lets destroy the popHost list at this point
	CHostList::Cleanup();

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

	// Perform any IMAP actions that have been done locally but not online.
	g_ImapAccountMgr.FinishUpActionQueues();

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

	// Trim the junk mailbox.  For now trim on every quit, eventually be more clever.
	TrimJunk();

	if (!m_bFlushBars)
	{
		// DRW - Since the contents of the ad toolbar are created dynamically by the ad server,
		// we don't want to have the toolbar manager store the information about the ad toolbar
		// in the conventional way.  Store the location, floating and docking states of the
		// toolbar, then remove it from the toolbar manager.
		CRect		rectAdToolBar(0,0,0,0);
		DWORD		dwStyle = 0;
		short		iFloating = 0;
		short		iDocking = 0;
		if (m_pToolBarAd)
		{
			if (m_pToolBarAd->IsVisible())
			{
				m_pToolBarAd->GetWindowRect(&rectAdToolBar);
				iFloating = (m_pToolBarAd->IsFloating() != 0);
				// The docking values (CBRS_ALIGN_LEFT, etc.) range from 0x1000 to 0x8000.
				// For simplicity I reduce these to 0x01 to 0x08 and store just that value.
				dwStyle = m_pToolBarAd->GetBarStyle();
				iDocking = (short)(dwStyle & (CBRS_ALIGN_LEFT | CBRS_ALIGN_TOP | CBRS_ALIGN_RIGHT | CBRS_ALIGN_BOTTOM));
				iDocking = (short)((iDocking>>12) & 0x0F);
			}
			RemoveControlBar(m_pToolBarAd);

			RemoveAdToolBarFromItsDockBar(m_pToolBarAd);
		}
		SetIniWindowPos(IDS_INI_AD_TOOLBAR_WIN_POS, rectAdToolBar);
		SetIniShort(IDS_INI_AD_TOOLBAR_FLOATING, iFloating);
		SetIniShort(IDS_INI_AD_TOOLBAR_DOCKING, iDocking);

		RemoveBogusAdToolBars();

		SaveBarState(_T("ToolBar"));

		m_WazooBarMgr.SaveWazooBarConfigToIni(); // saves docked and floating window *sizes*
	}

	// If we got this far without crashing, remember that.
	// (Used for future crash detection). Do before
	// WriteToolBarMarkerToIni, because it flushes the INI file.
	g_QCExceptionHandler.SaveCrashStateToINI();

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

// This function isn't hooked up to a Help menu item any more, so it
// shouldn't get called. Just in case it makes a comeback, I updated it
// to test the mode rather than the obsolete user code, ...JR
void CMainFrame::OnTechSupport()
{
#ifdef COMMENTED_OUT
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
#endif

	SharewareModeType mode = QCSharewareManager::GetModeFromIni();
	if (mode == SWM_MODE_LIGHT || mode == SWM_MODE_DEADBEAT)
		// No support in Light mode, so act like the old NC user code
		// is prsent
		WinHelp(0x10000 + IDC_MHELP_NC);
	else
		// Full support in Adware and Pro modes, so act like the old IC
		// user code is present
		WinHelp(0x10000 + IDC_MHELP_IC);
	return;
}

void CMainFrame::OnPaymentAndRegistration()
{
#ifndef DEATH_BUILD
	CPaymentAndRegistrationDlg dlg;

	dlg.DoModal();
#endif
}

void CMainFrame::OnUpdatePaymentAndRegistration(CCmdUI* pCmdUI)
{
#ifdef DEATH_BUILD
	pCmdUI->Enable(FALSE);
#else
	pCmdUI->Enable(TRUE);
#endif
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
	
	CMenu*  pMainMenu;
		
	// This check is required to make sure that the MainFrame does have a Menu associated with it.
	// In Release build, when the app is in print preview mode, the App's actual Mainframe is replaced
	// by the SDI Frame Window of the print preview window. In that specific scenario, there would be
	// no menu associated with the MainFrame
	// Helpful Microsoft Knowledge Base reference : http://support.microsoft.com/support/kb/articles/Q166/1/35.ASP

	pMainMenu = GetMenu();
	if (NULL != pMainMenu)
	{
		int nCount = GetMenuItemCount(m_hWindowMenu);

		// Start at the beginning of the listing of open windows.
		for (int i = MP_MDICHILDWND_LIST; i < nCount; i++)
		{
			// ignore separators (nID == 0)
			if (nID = GetMenuItemID(m_hWindowMenu, i))
			{
				GetMenuString(m_hWindowMenu, i, szText, sizeof(szText), MF_BYPOSITION);
				menuPopup->AppendMenu(MF_STRING, nID, szText);
			}
		}
	
		// It's better not to show the context menu in cases where the GetMenu for the Mainframe 
		// fails to avoid any other problems that would arise when user clicks on the context menu so popped
		// e.g If we show the pop the MAIN_MENU_WINDOW on right click in print preview mode and user clicks on 
		// the "New Message" menu item, then we would have to make sure that the code for composing new message 
		// does not assume that the AfxGetMainWnd() would return the Mainframe of the app (which would not be the case here).

		CContextMenu::MatchCoordinatesToWindow(HWND(wParam), ptScreen);
		CContextMenu(menuPopup, ptScreen.x, ptScreen.y);
	}

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
	if (pCmdUI->m_nID == ID_CUSTOMIZE_LINK)
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
	else
	{
		BOOL	bIsVisible = FALSE;

		switch (pCmdUI->m_nID)
		{
			case ID_CTRLBAR_SHOW_TOOLBAR:
				bIsVisible = m_pToolBar->IsVisible();
				break;

			case ID_CTRLBAR_SHOW_SEARCHBAR:
				bIsVisible = m_pSearchBar->IsVisible();
				break;

			case ID_CTRLBAR_SHOW_STATUSBAR:
				bIsVisible = m_wndStatusBar.IsVisible();
				break;

			default:
				ASSERT(0);
				break;
		}

		pCmdUI->SetCheck(bIsVisible);
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
// IsSearchbarVisible [PUBLIC]
//
// Returns TRUE is the search bar is visible, FALSE otherwise
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::IsSearchbarVisible() const
{
	ASSERT(m_pSearchBar);
	return (m_pSearchBar->IsVisible());
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


//#define DEBUG_TOGGLE_SPONSOR


////////////////////////////////////////////////////////////////////////
// ChangeSponsor [public]
//
// Change the sponsor to the given ad.
////////////////////////////////////////////////////////////////////////
void CMainFrame::ChangeSponsor(CString& strAdID, CString& strButtonAdFile, CString& strAdURL, CString& strAdTitle)
{
	#ifdef DEBUG_TOGGLE_SPONSOR
	static CString str;
	if (strButtonAdFile == "")
	{
		strButtonAdFile = str;	
	}
	else
	{
		str = strButtonAdFile;
	}
	#endif

	// Don't do anything if we are already showing the specified sponsor.
	if (m_padSponsor && m_padSponsor->m_pBmp && (m_padSponsor->m_strID == strAdID))
	{
		TRACE1("CMainFrame::ChangeSponsor called more than once with same ad %s\n", strAdID);
		return;
	}

	SECImage* pImage = LoadImage(strButtonAdFile);
	if (pImage)
	{
		//	Create the bitmap that we're going to blit into
		CPaletteDC		cdc;
		CWindowDC		screenDC(NULL);
		
		if (m_padSponsor == NULL)
		{
			m_padSponsor = DEBUG_NEW_MFCOBJ_NOTHROW CAdInfo(0, DEBUG_NEW CBitmap(), NULL, strAdURL, strAdID, strAdTitle);
		}
		else
		{
			if (m_padSponsor->m_pBmp != NULL)
			{
				delete m_padSponsor->m_pBmp;
			}
			m_padSponsor->m_iCmd = 0;
			m_padSponsor->m_pBmp = DEBUG_NEW_MFCOBJ_NOTHROW CBitmap();
			m_padSponsor->m_pBmpAlt = NULL;
			m_padSponsor->m_strURL = strAdURL;
			m_padSponsor->m_strID = strAdID;
		}

		m_nSponsorBitmapWidth = pImage->dwGetWidth();
		m_nSponsorBitmapHeight = pImage->dwGetHeight();
		m_nLogoBitmapWidth = m_nSponsorBitmapWidth;
		m_nLogoBitmapHeight = m_nSponsorBitmapHeight;

		if ((m_padSponsor != NULL) && (m_padSponsor->m_pBmp != NULL) &&
			 m_padSponsor->m_pBmp->CreateCompatibleBitmap(&screenDC, m_nLogoBitmapWidth, m_nLogoBitmapHeight))
		{
			m_dcLogo.SelectObject(m_padSponsor->m_pBmp);

			// If the sponsor ad is taller than the standard height (29) plus
			// some buffer increase the height of the sponsor area to compensate.
			if (m_nSponsorBitmapHeight > 29)
			{
				m_pWBClient->SetMargins(0, 0, 0, m_nSponsorBitmapHeight + 4);
				RecalcLayout(TRUE);
				// Invalidate and update.
				Invalidate(TRUE);
				UpdateWindow();
			}

			//	Erase the background to white
			CRect		rect(0, 0, m_nSponsorBitmapWidth, m_nSponsorBitmapHeight);
			CBrush		textBrush(RGB(255,255,255));
			m_dcLogo.FillRect(&rect, &textBrush);

			//	We might prefer HALFTONE, but some msdn docs say that it only works on NT.
			//	COLORONCOLOR seems to work well enough.
			m_dcLogo.SetStretchBltMode(COLORONCOLOR);

			// Stretch to the same size (essentially BitBlt()).
			::StretchDIBits(
				m_dcLogo,
				0, 0,
				m_nSponsorBitmapWidth, m_nSponsorBitmapHeight,
				0, 0,
				m_nSponsorBitmapWidth, m_nSponsorBitmapHeight,
				pImage->m_lpSrcBits,
				pImage->m_lpBMI, DIB_RGB_COLORS,
				SRCCOPY);

			Invalidate(TRUE);
			UpdateWindow();
		}
		delete pImage;
	}
}

////////////////////////////////////////////////////////////////////////
// ResetSponsor [public]
//
// Change the sponsor back to the original (Qualcomm).
////////////////////////////////////////////////////////////////////////
void CMainFrame::ResetSponsor()
{
	// Free up the previous bitmap.
	if (m_padSponsor && m_padSponsor->m_pBmp)
	{
		delete m_padSponsor->m_pBmp;
		m_padSponsor->m_pBmp = NULL;
	}

	// Select the bitmap for the original Qualcomm logo.
	m_dcLogo.SelectObject(&m_LogoBitmap);

	BITMAP bm;
	m_LogoBitmap.GetObject(sizeof(bm), &bm);
	m_nLogoBitmapWidth = bm.bmWidth;
	m_nLogoBitmapHeight = bm.bmHeight;

	m_pWBClient->SetMargins(0, 0, 0, 30);
	RecalcLayout(TRUE);

	// Invalidate and update.
	Invalidate(TRUE);
	UpdateWindow();
}

////////////////////////////////////////////////////////////////////////
// OnDrawBorder [protected, virtual]
//
// Override default base class implementation to draw the QC logo
// on the Auto-Wazoo bar.  Note that we completely override the SEC
// base class implementation here.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnDrawBorder(CDC* pDC)
{
	if (m_padSponsor && m_padSponsor->m_pBmp)
	{
		// If we have a sponsor ad showing, use the ad bitmap.
		// This is mostly the code from QCWorkbook::OnDrawBorder(), but the
		// crucial difference is that the original QC logo is a bitmap and
		// mask, whereas a sponsor ad is just a bitmap, so we do a BitBlt()
		// with SRCCOPY.
		CPoint TopLeft;
		
		// This is as good a place as any to figure out if somebody has
		// done something behind our backs
		if (m_lastVisCount != CountVisibleTabs() && !m_bInRecalcLayout)
			ResetTaskBar();

		// Ok, now do our thing
		if (CalcLogoTopLeft(&TopLeft))
		{
			// Erase the background first.
			CDC			dcBlank;
			CDC			dcMask;

			if (dcBlank.CreateCompatibleDC(pDC) && dcMask.CreateCompatibleDC(pDC))
			{
				CRect		rect(0, 0, m_nLogoBitmapWidth, m_nLogoBitmapHeight);
				CBrush		textBrush(RGB(255,255,255));
				CBrush		textBrushMask(RGB(0,0,0));
				dcBlank.FillRect(&rect, &textBrush);
				dcMask.FillRect(&rect, &textBrushMask);				

				pDC->BitBlt(TopLeft.x, TopLeft.y, m_nSponsorBitmapWidth, m_nSponsorBitmapHeight, &dcMask, 0, 0, SRCAND);
				pDC->BitBlt(TopLeft.x, TopLeft.y, m_nSponsorBitmapWidth, m_nSponsorBitmapHeight, &dcBlank, 0, 0, SRCPAINT);
			}

			// Now display the ad using SRCCOPY (no mask).
			pDC->BitBlt(TopLeft.x, TopLeft.y, m_nLogoBitmapWidth, m_nLogoBitmapHeight, &m_dcLogo, 0, 0, SRCCOPY);
		}
	}
	else
	{
		QCWorkbook::OnDrawBorder(pDC);
	}
}

////////////////////////////////////////////////////////////////////////
// LogoClicked [protected]
//
// User clicked on the logo, launch the logo URL.
////////////////////////////////////////////////////////////////////////
void CMainFrame::LogoClicked()
{
	if (m_padSponsor && m_padSponsor->m_pBmp)
	{
		// If there is an ad bitmap, then we aren't looking at the default logo.
		LaunchURL(m_padSponsor->m_strURL);

		// If we are doing toggle testing switch back to the QC logo.
		#ifdef DEBUG_TOGGLE_SPONSOR
		ResetSponsor();
		#endif
	}
	else
	{
		// User clicked on the Qualcomm logo, do the default behavior.
		QCWorkbook::LogoClicked();

		// If we are doing toggle testing switch back to having a sponsor.
		#ifdef DEBUG_TOGGLE_SPONSOR
		CString		s1(_T("FakeID"));
		CString		s2(_T("c:\\temp\\121.png"));
		CString		s3(_T("FakeURL"));
		CString		s4(_T("FakeTitle"));
		ChangeSponsor(s1, s2, s3, s4);
		#endif
	}
}

////////////////////////////////////////////////////////////////////////
// GetLogoMessageText [protected]
//
// Return the message text associated with the logo.
////////////////////////////////////////////////////////////////////////
CString CMainFrame::GetLogoMessageText()
{
	CString			 strMessage(_T(""));

	if (m_padSponsor && m_padSponsor->m_pBmp)
	{
		strMessage = m_padSponsor->m_strURL;
	}
	else
	{
		strMessage = CRString(IDS_MAIN_WEB_URL);
	}

	//	Remove everything up to and including "url="
	int		pos = strMessage.Find("url=");

	if (pos > 0)
	{
		strMessage = strMessage.Right(strMessage.GetLength() - pos - 4);
	}
	
	//	Remove everything after "&distributorid"
	pos = strMessage.Find("&distributorid");
	if (pos > 0)
	{
		strMessage = strMessage.Left(pos);
	}
	//	Unescape for nicer display
	strMessage = UnEscapeURL(strMessage);

	return strMessage;
}

////////////////////////////////////////////////////////////////////////
// RemoveBogusAdToolBars [public]
//
// Sometimes (through means yet to be determined) the ad toolbar is
// saved.  Remove any toolbars that consist only of ad buttons.
////////////////////////////////////////////////////////////////////////
void CMainFrame::RemoveBogusAdToolBars()
{
	CPtrList	listToDelete;

	// Iterate through the list of control bars, looking for bogus ones.
	CPtrList&	list = m_listControlBars;
	POSITION	pos = list.GetHeadPosition();
	while (pos != NULL)
	{
		CControlBar* pBar = (CControlBar*)(list.GetNext(pos));

		// When checking custom toolbars, be sure to skip the Search Bar -
		// it never contains any buttons, but it shouldn't be removed.
		if ( pBar->IsKindOf(RUNTIME_CLASS(SECCustomToolBar)) &&
			 !pBar->IsKindOf(RUNTIME_CLASS(CSearchBar)) )
		{
			// Default to removing the toolbar.
			int		bDeleteIt = true;
			for (int i = 0; bDeleteIt && (i < ((SECCustomToolBar*)pBar)->GetBtnCount()); i++ )
			{
				UINT	iBtnID = ((SECCustomToolBar*)pBar)->GetItemID(i);
				if ((iBtnID < TOOBAR_AD_CMD_BASE) || (iBtnID > TOOBAR_AD_CMD_BASE + MAX_TOOLBAR_ADS))
				{
					// If we find any buttons that are not ads, skip this toolbar.
					bDeleteIt = false;
				}
			}
			if (bDeleteIt)
			{
				// Don't want to be deleting items in an iterator, so just save a reference.
				listToDelete.AddTail(pBar);
			}
		}
	}

	// Delete any toolbars in the list.
	pos = listToDelete.GetHeadPosition();
	while (pos != NULL)
	{
		SECCustomToolBar* pBar = (SECCustomToolBar*)(listToDelete.GetNext(pos));
		if (pBar)
		{
			// Hide and remove the toolbar.
			ShowControlBar(pBar, FALSE, FALSE);
			RemoveControlBar(pBar);

			RemoveAdToolBarFromItsDockBar(pBar);
		}
	}
}

////////////////////////////////////////////////////////////////////////
// AddAdToolBar [public]
//
// Add (or simply show) the ad toolbar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::AddAdToolBar()
{
	if (m_pToolBarAd)
	{
		// Ad toolbar already exists, just show it.
		m_pToolBarAd->ShowWindow(SW_SHOW);
	}
	else
	{
		// Ad toolbar doesn't exist, need to create it.
		QCToolBarManager* pMgr = (QCToolBarManager*)m_pControlBarManager;
		if (pMgr && m_pToolBar)
		{
			// Start with the main toolbar style.
			DWORD		dwStyle = m_pToolBar->GetBarStyle();
			DWORD		dwExStyle = m_pToolBar->GetExBarStyle();
			// Use the ad toolbar's docking INI entry if one exists.
			short		bDocking = GetIniShort(IDS_INI_AD_TOOLBAR_DOCKING);
			if (bDocking > 0)
			{
				// Clear the default docking info.
				dwStyle &= ~(CBRS_ALIGN_LEFT | CBRS_ALIGN_TOP | CBRS_ALIGN_RIGHT | CBRS_ALIGN_BOTTOM | CBRS_EX_DISALLOW_FLOAT | CBRS_BORDER_ANY);
				// Set the saved docking info.
				dwStyle |= (bDocking<<12);

				// Set the correct border styles based on our alignment
				if (dwStyle & CBRS_ALIGN_LEFT)
					dwStyle |= CBRS_BORDER_RIGHT;
				if (dwStyle & CBRS_ALIGN_TOP)
					dwStyle |= CBRS_BORDER_BOTTOM;
				if (dwStyle & CBRS_ALIGN_RIGHT)
					dwStyle |= CBRS_BORDER_LEFT;
				if (dwStyle & CBRS_ALIGN_BOTTOM)
					dwStyle |= CBRS_BORDER_TOP;
			}
			
			m_pToolBarAd = ((QCToolBarManager*)pMgr)->CreateUserToolBar(_T("Sponsor Bar"), dwStyle, dwExStyle);

			if (m_pToolBarAd)
			{
				// Allow the toolbar to be docked.
				m_pToolBarAd->EnableDocking(CBRS_ALIGN_ANY);

				// For dual size buttons:
				int			iButtonSize = pMgr->LargeBtnsEnabled() ? 32 : 16;
				int			iToolBarWidth = iButtonSize * m_iAdsShown;
				BOOL		bToolBarAdFloating = GetIniShort(IDS_INI_AD_TOOLBAR_FLOATING);
				CRect		rectWindow(0,0,0,0);
				CRect		rectMainTB(0,0,0,0);
				CRect		rectSavedAdTB(0,0,0,0);
				RECT		rectAdTB;

				GetWindowRect(&rectWindow);
				m_pToolBar->GetWindowRect(&rectMainTB);
				rectAdTB.right = 0;		// This is our key as to whether a position has been determined.

				// If we previously saved an ad toolbar position, restore it.
				if (GetIniWindowPos(IDS_INI_AD_TOOLBAR_WIN_POS, rectSavedAdTB))
				{
					if ((rectSavedAdTB.left > 0) && (rectSavedAdTB.right > 0))
					{
						// We have a valid location to restore to.
						rectAdTB.left = rectSavedAdTB.left;
						rectAdTB.top = rectSavedAdTB.top;
						rectAdTB.right = rectSavedAdTB.right;
						rectAdTB.bottom = rectSavedAdTB.bottom;
					}
				}
				// If there is no saved position, use the default location
				// appropriate for the style of the main toolbar (horizontal
				// or vertical).
				if (rectAdTB.right == 0)
				{
					// Make default location appropriate for alignment of main toolbar.
					if ((dwStyle & CBRS_ALIGN_TOP) || (dwStyle & CBRS_ALIGN_BOTTOM))
					{
						// Main toolbar is horizontal, place ad toolbar at the
						// same vertical position at the far right.
						rectAdTB.left = rectWindow.right - iToolBarWidth;
						rectAdTB.top = rectMainTB.top;
						rectAdTB.right = rectAdTB.left + iToolBarWidth;
						rectAdTB.bottom = rectMainTB.bottom;
					}
					else
					{
						// Main toolbar is vertical, place ad toolbar at the
						// same horizontal position at the bottom.
						rectAdTB.left = rectMainTB.left;
						rectAdTB.top = rectMainTB.bottom + 1;
						rectAdTB.right = rectAdTB.left + iToolBarWidth;
						rectAdTB.bottom = rectAdTB.top + iButtonSize + 1;
					}
				}

				// Float or dock the ad toolbar as appropriate.
				if (bToolBarAdFloating)
				{
					// Since we don't currently allow the ad toolbar to float, if we
					// find a floating ad toolbar dock it to the nearest location.
					#if 0
					POINT		pt;
					pt.x = rectAdTB.left;
					pt.y = rectAdTB.top;
					FloatControlBar(m_pToolBarAd, pt);
					#endif
					DockControlBar(m_pToolBarAd, (UINT)0, &rectAdTB);
				}
				else
				{
					DockControlBar(m_pToolBarAd, (UINT)0, &rectAdTB);
				}

				// to show, or not to show...
				m_pToolBarAd->ShowWindow(GetIniShort(IDS_INI_SHOW_TOOLBAR)?SW_SHOWNORMAL:SW_HIDE);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
// RemoveAdToolBar [public]
//
// Remove the ad toolbar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::RemoveAdToolBar()
{
	if (m_pToolBarAd)
	{
		m_pToolBarAd->DestroyWindow();
	}
}

////////////////////////////////////////////////////////////////////////
// RemoveAdToolBarFromItsDockBar [public]
//
// Remove the ad toolbar from its dock bar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::RemoveAdToolBarFromItsDockBar(CControlBar * pAdToolBar)
{
	if (pAdToolBar->m_pDockBar != NULL)
	{
		//	Removing the ad toolbar from its dock bar makes sure that it doesn't
		//	get saved in the list of BarId's, which in turn avoids forcing
		//	any items after it from loading in the next row (during loading
		//	it's place would be NULL, which tells the layout code to move to the
		//	next row).

		//	First detach the dock bar from the ad toolbar. This avoids a
		//	crash when an ad toolbar is floating (not normal) and is removed.
		//	In that case, the toolbar was removed from the dock bar, which
		//	caused the floating window to be destroyed, which in turn attempted
		//	to remove the toolbar again. A bad index value caused MFC to have
		//	fits when a release build was running with a bad toolbar like this.
		CDockBar *	pDockBar = pAdToolBar->m_pDockBar;
		pAdToolBar->m_pDockBar = NULL;

		//	Now remove the ad toolbar from its dock bar - calling the appropriate
		//	version of RemoveControlBar (since its not a virtual method).
		if (pDockBar->IsKindOf(RUNTIME_CLASS(SECDockBar)))
			reinterpret_cast<SECDockBar *>(pDockBar)->RemoveControlBar(pAdToolBar);
		else
			pDockBar->RemoveControlBar(pAdToolBar);
	}
}

////////////////////////////////////////////////////////////////////////
// ToolBarAdAlreadyShowing [public]
//
// Is the specified ad already in the ad toolbar?
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::ToolBarAdAlreadyShowing(CString& strAdID)
{
	BOOL			 bAdShowing = FALSE;
	CAdInfo			*ptbiTemp = NULL;
	for (int i = 0; (i < m_iAdsShown) && !bAdShowing; ++i)
	{
		ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(i);
		if (ptbiTemp && (ptbiTemp->m_strID == strAdID))	bAdShowing = TRUE;
	}

	return bAdShowing;
}

////////////////////////////////////////////////////////////////////////
// ShowToolBarAd [public]
//
// We have a new toolbar ad to show
////////////////////////////////////////////////////////////////////////
void CMainFrame::ShowToolBarAd(CString& strAdID, CString& strButtonAdFile, CString& strAdURL, CString& strAdTitle)
{
	// If we aren't showing toolbars, don't show the ad.
	if (!GetIniShort(IDS_INI_SHOW_TOOLBAR))
	{
		return;
	}

	// Create or show the ad toolbar
	AddAdToolBar();

	ASSERT(m_iAdsShown < MAX_TOOLBAR_ADS);

	// Display the specified ad in the toolbar
	if (m_pToolBarAd && (m_iAdsShown < MAX_TOOLBAR_ADS))
	{
		// Proceed only if the specified ad isn't already showing.
		if (ToolBarAdAlreadyShowing(strAdID))
		{
			TRACE1("CMainFrame::ShowToolBarAd called more than once with same ad %s\n", strAdID);
		}
		else
		{
			// Read the PNG file and convert it to a BMP (this code shamelessly
			// borrowed from Geoff's link history code).
			SECImage		*pImage = LoadImage(strButtonAdFile);
			CBitmap			*pBitmapButtonAd = NULL;
			CBitmap			*pBitmapAltImage = NULL;
			if (pImage)
			{
				//	Create the bitmap that we're going to blit into
				CPaletteDC		cdc;
				CWindowDC		screenDC(NULL);
				
				pBitmapButtonAd = DEBUG_NEW_MFCOBJ_NOTHROW CBitmap();

				// For dual size buttons:
				QCToolBarManager* pMgr = (QCToolBarManager*)(m_pControlBarManager);
				int		iButtonSize = 32;
				int		iAltButtonSize = 16;

				if ( (pBitmapButtonAd != NULL) && cdc.CreateCDC() &&
					 pBitmapButtonAd->CreateCompatibleBitmap(&screenDC, iButtonSize, iButtonSize) )
				{
					CBitmap	*	pSavePrevBitmap = cdc.SelectObject(pBitmapButtonAd);

					//	Erase the background to white
					CRect		rect(0, 0, iButtonSize, iButtonSize);
					CBrush		textBrush( RGB(255,255,255) );
					cdc.FillRect(&rect, &textBrush);

					//	We might prefer HALFTONE, but some msdn docs say that it only works on NT.
					//	COLORONCOLOR seems to work well enough.
					cdc.SetStretchBltMode(COLORONCOLOR);

					::StretchDIBits(
						cdc.GetSafeHdc(),
						0, 0,
						iButtonSize, iButtonSize,
						0, 0,
						iButtonSize, iButtonSize,
						pImage->m_lpSrcBits,
						pImage->m_lpBMI, DIB_RGB_COLORS,
						SRCCOPY );

					// If there is an alternate bitmap store it as well.
					if (iAltButtonSize > 0)
					{
						pBitmapAltImage = DEBUG_NEW_MFCOBJ_NOTHROW CBitmap();
						if (pBitmapAltImage && pBitmapAltImage->CreateCompatibleBitmap(&screenDC, iAltButtonSize, iAltButtonSize))
						{
							cdc.SelectObject(pBitmapAltImage);

							//	Erase the background to white
							CRect		rectAlt(0, 0, iAltButtonSize, iAltButtonSize);
							CBrush		textBrushAlt( RGB(255,255,255) );
							cdc.FillRect(&rectAlt, &textBrushAlt);

							//	We might prefer HALFTONE, but some msdn docs say that it only works on NT.
							//	COLORONCOLOR seems to work well enough.
							cdc.SetStretchBltMode(COLORONCOLOR);

							::StretchDIBits(
								cdc.GetSafeHdc(),
								0, 0,
								iAltButtonSize, iAltButtonSize,
								iButtonSize, 0,
								iAltButtonSize, iAltButtonSize,
								pImage->m_lpSrcBits,
								pImage->m_lpBMI, DIB_RGB_COLORS,
								SRCCOPY );
						}
						// If we are using small toolbar icons, make the small one the primary.
						if (!pMgr->LargeBtnsEnabled())
						{
							CBitmap		*pBmp = pBitmapButtonAd;
							pBitmapButtonAd = pBitmapAltImage;
							pBitmapAltImage = pBmp;
						}
					}

					cdc.SelectObject(pSavePrevBitmap);

					CAdInfo			*ptbi = DEBUG_NEW CAdInfo(TOOBAR_AD_CMD_BASE + m_iAdsShown,
																pBitmapButtonAd,
																pBitmapAltImage,
																strAdURL,
																strAdID,
																strAdTitle);

					m_arrayToolBarAds.Add(ptbi);
					++m_iAdsShown;

					RefreshAdToolBar();
				}
				delete pImage;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
// HideToolBarAd [public]
//
// Remove the specified toolbar ad
////////////////////////////////////////////////////////////////////////
void CMainFrame::HideToolBarAd(CString& strAdID)
{
	if (m_pToolBarAd)
	{
		int				 iFoundAt = -1;
		CAdInfo			*ptbiTemp = NULL;
		for (int i = 0; (i < m_iAdsShown) && (iFoundAt == -1); ++i)
		{
			ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(i);
			if (ptbiTemp && (ptbiTemp->m_strID == strAdID))	iFoundAt = i;
		}
		if ((iFoundAt != -1) && ptbiTemp)
		{
			m_arrayToolBarAds.RemoveAt(iFoundAt);
			--m_iAdsShown;
			delete ptbiTemp;
			ptbiTemp = NULL;
		}
		RefreshAdToolBar();
	}
}

////////////////////////////////////////////////////////////////////////
// RefreshAdToolBar [public]
//
// Refresh the ad toolbar with the current array of ads.
////////////////////////////////////////////////////////////////////////
void CMainFrame::RefreshAdToolBar()
{
	if (m_pToolBarAd)
	{
		if (m_iAdsShown > 0)
		{
			CPaletteDC		 cdc;
			CWindowDC		 screenDC(NULL);
			CBitmap			*pBitmapOldToolBar = m_pBitmapToolBar;

			m_pBitmapToolBar = DEBUG_NEW_MFCOBJ_NOTHROW CBitmap();

			// For dual size buttons:
			QCToolBarManager* pMgr = (QCToolBarManager*)(m_pControlBarManager);
			int		iButtonSize = pMgr->LargeBtnsEnabled() ? 32 : 16;

			if ((m_pBitmapToolBar != NULL) && cdc.CreateCDC() && 
				 m_pBitmapToolBar->CreateCompatibleBitmap(&screenDC, m_iAdsShown * iButtonSize, iButtonSize))
			{
				// Create the toolbar bitmap from the individual ad bitmaps.
				CBitmap				*pSavePrevBitmap = (CBitmap*)cdc.SelectObject(m_pBitmapToolBar);
				CBitmap				*pBmpOldSrc = NULL;
				CAdInfo				*ptbiTemp = NULL;
				CPaletteDC			 dcSrc;
				dcSrc.CreateCDC();
				for (int i = 0; i < m_iAdsShown; ++i)
				{
					ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(i);
					if (ptbiTemp)
					{
						pBmpOldSrc = (CBitmap*)dcSrc.SelectObject(ptbiTemp->m_pBmp->GetSafeHandle());
						cdc.BitBlt(i * iButtonSize, 0, iButtonSize, iButtonSize, &dcSrc, 0, 0, SRCCOPY);

						// Diddling with putting ad buttons in the main toolbar...
						#if 0
						QCToolBarManager* pMgr = (QCToolBarManager*)m_pControlBarManager;
						pMgr->AddButtonImage(*(ptbiTemp->m_pBmp), ptbiTemp->m_iCmd);
						m_pToolBar->AddButton(m_pToolBar->GetBtnCount(), ptbiTemp->m_iCmd);
						#endif
					}
				}
				cdc.SelectObject(pSavePrevBitmap);

				// Create the array of command ids.
				UINT	piAdButtons[MAX_TOOLBAR_ADS];
				for (int j = 0; j < m_iAdsShown; ++j)
				{
					ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(j);
					if (ptbiTemp)
					{
						piAdButtons[j] = ptbiTemp->m_iCmd;
					}
				}

				// Update the toolbar info.
				m_pToolBarAd->SetToolBarInfo((HBITMAP)m_pBitmapToolBar->GetSafeHandle(), piAdButtons, m_iAdsShown, iButtonSize, iButtonSize);
				m_pToolBarAd->SetButtons(piAdButtons, m_iAdsShown);
				
				// Refresh and resize the toolbar.
				m_pToolBarAd->GetParentFrame()->RecalcLayout(TRUE);
				m_pToolBarAd->Invalidate(TRUE);
				m_pToolBarAd->UpdateWindow();

			}

			// Delete the bitmap from last time.
			if (pBitmapOldToolBar)
			{
				delete pBitmapOldToolBar;
			}
		}
		else
		{
			// If there are no buttons in the ad toolbar, destroy it.
			m_pToolBarAd->DestroyWindow();
			m_pToolBarAd = NULL;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// SwitchAdToolBarImages [public]
//
// Reread the ad toolbar images from their files.
////////////////////////////////////////////////////////////////////////
void CMainFrame::SwitchAdToolBarImages()
{
	CAdInfo		*pAdInfo = NULL;
	for (int i = 0; i < m_iAdsShown; ++i)
	{
		pAdInfo = (CAdInfo*)m_arrayToolBarAds.GetAt(i);
		if (pAdInfo && pAdInfo->m_pBmp && pAdInfo->m_pBmpAlt)
		{
			CBitmap		*pBitmap = pAdInfo->m_pBmp;
			pAdInfo->m_pBmp = pAdInfo->m_pBmpAlt;
			pAdInfo->m_pBmpAlt = pBitmap;
		}
	}

	RefreshAdToolBar();
}

////////////////////////////////////////////////////////////////////////
// IsAdToolBar [public]
//
// Return TRUE if the given toolbar represents the ad toolbar.
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::IsAdToolBar(QCCustomToolBar *pToolBar)
{
	if (pToolBar && (pToolBar == m_pToolBarAd))
	{
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// IsToolBarAd [public]
//
// Return TRUE if the given command represents a toolbar ad.
////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::IsToolBarAd(int nCmd)
{
	if ((nCmd >= TOOBAR_AD_CMD_BASE) && (nCmd <= TOOBAR_AD_CMD_BASE + m_iAdsShown))
	{
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// GetToolBarAdToolTip [public]
//
// Return the tool tip for the specified button.
////////////////////////////////////////////////////////////////////////
void CMainFrame::GetToolBarAdToolTip(int nCmd, CString& strToolTip)
{
	if (nCmd - TOOBAR_AD_CMD_BASE <= m_arrayToolBarAds.GetSize())
	{
		CAdInfo			*ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(nCmd - TOOBAR_AD_CMD_BASE);
		if (ptbiTemp)
		{
			strToolTip = ptbiTemp->m_strTitle;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// ToolBarButtonDeleted [public]
//
// Called when button is dragged off toolbar so ad toolbar can
// handle ad deletion.
////////////////////////////////////////////////////////////////////////
void CMainFrame::ToolBarButtonDeleted(QCCustomToolBar *pToolBar, int nIndex)
{
	if (pToolBar && (pToolBar == m_pToolBarAd))
	{
		CAdInfo			*ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(nIndex);
		if (ptbiTemp)
		{
			m_arrayToolBarAds.RemoveAt(nIndex);
			--m_iAdsShown;
			ASSERT(m_ldb);
			MGR_UserTweaked(m_ldb, ptbiTemp->m_strID, kTweakUserDelete);
			delete ptbiTemp;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// ToolBarClosed [public]
//
// Handle the case where a toolbar has been closed (currently relevant
// only for the ad toolbar).
////////////////////////////////////////////////////////////////////////
void CMainFrame::ToolBarClosed(QCCustomToolBar *pToolBar)
{
	if (pToolBar && (pToolBar == m_pToolBarAd))
	{
		// Notify button ads that they are no longer showing.
		while (m_arrayToolBarAds.GetSize() > 0)
			ToolBarButtonDeleted(pToolBar, 0);
	}
}

////////////////////////////////////////////////////////////////////////
// ToolBarDeleted [public]
//
// Called when a toolbar is deleted so we know if the ad toolbar has
// been deleted (which happens if all ads are removed).
////////////////////////////////////////////////////////////////////////
void CMainFrame::ToolBarDeleted(QCCustomToolBar *pToolBar)
{
	if (pToolBar && (pToolBar == m_pToolBarAd))
	{
		m_pToolBarAd = NULL;
	}
}

////////////////////////////////////////////////////////////////////////
// FloatControlBar [public]
//
// Don't allow the ad toolbar to float.
////////////////////////////////////////////////////////////////////////
void CMainFrame::FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle)
{
	if (pBar != m_pToolBarAd)
	{
		QCWorkbook::FloatControlBar(pBar, point, dwStyle);

		// Whatta hack!  This is the only place I could find to figure out
		// when the ad wazoo bar gets floated.  All other notifications
		// occur before the bar has been reparented to the mini frame window.
		CAdWazooBar* pAdWazooBar = DYNAMIC_DOWNCAST(CAdWazooBar, pBar);
		if (pAdWazooBar)
			pAdWazooBar->RemoveSystemMenu();
	}
	else
	{
		// We should probably do more than just beep (Geoff insists we should
		// change the mouse pointer to the cancel pointer) but for now a beep
		// will suffice.  Maybe the OT upgrade will actually implement
		// CBRS_EX_DISALLOW_FLOAT and this won't be our problem anymore.
		MessageBeep(MB_OK);
	}
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
// ShowControlBar [protected]
//
// Intercept this to handle the ad toolbar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay)
{
	QCWorkbook::ShowControlBar(pBar, bShow, bDelay);
	if ((bShow == FALSE) && m_pToolBarAd && (pBar == m_pToolBarAd))
	{
		RemoveAdToolBar();
	}
}

////////////////////////////////////////////////////////////////////////
// OnCustomizeLink [protected]
//
// Command handler for displaying toolbar Property Sheets.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnCustomizeLink()
{
	// Secret back door to restore the toolbars to the default state
	if (ShiftDown() && CtrlDown())
	{
		OnResetTools();
	}
	// Shareware: Only allow customization of toolbar in FULL FEATURE version.
	else if (UsingFullFeatureSet())
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
// OnShowSearchBar [protected]
//
// Command handler for hiding/showing the global search bar.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnShowSearchBar()
{
	// toggle current visibility
	BOOL is_visible = m_pSearchBar->IsVisible();
	ShowControlBar(m_pSearchBar, !is_visible, FALSE);
	SetIniShort(IDS_INI_SHOW_SEARCH_BAR, !is_visible);
	RecalcLayout();
}


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
//
// wParam values can be interepreted as below
//	HIWORD : 0 - Default value, this value is ignored
//			 1 - Eudora is set as default Emailer
//			 2 - Eudora is NOT set as default Emailer
//
// lParam values can be interepreted as below
//	LOWORD : 0 - Default value, this value is ignored
//	LOWORD : 1 - Add entries in Netscape's Prefs.js files for 
//						making Eudora as the mailto: handler
//			 2 - Delete entries in Netscape's Prefs.js files to
//						remove Eudora association as the mailto: handler
//			 3 - Check Netscape's Prefs.js if it already has entries for
//						making Eudora as the mailto: handler 
//
// NOTE : For some reasons calls to IsNetscapeAbove405Executing() could get 
// blocked on some systems (VI Bug # 5932). This happens in the Module32Next()
// in kernel32.dll. Till i move this part in a secondary thread, i am commenting
// all such calls. When convinced that Module32Next is indeed the reason for
// freeze, i will move it to a secondary thread.
////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnUserRegisterMailto(WPARAM wParam, LPARAM lParam)
{            
	BOOL bUseDDE = FALSE;
	BOOL bCheckPrefsFile = FALSE;
	BOOL bUpdatePrefsFile = FALSE;
	// Check if Eudora is default email handler is not required here ..will remove later if my assumption is always true
	// const BOOL bOverWrite = GetIniShort(IDS_INI_DEFAULT_MAILTO_OVERWRITE);
	if ( /*(TRUE == bOverWrite) || */
		 ( 1 == HIWORD(wParam)) /*User just made Eudora as the default emailer & the cached INI file is yet to be updated*/ )
	{
		// Make sure that DDE is not being forced to handle mailto
		if (FALSE == GetIniShort(IDS_INI_FORCE_DDE_FOR_HANDLING_MAILTO))
		{
			// Commenting tht next line because on some systems, prefs.js file is not getting updated for reasons yet to be known
			// & hence for 4.3 release these line is commented
			/*if (TRUE == IsNetscapeAbove405Executing())
			{
				// Warn the User to close all instances of netscape
				::AfxMessageBox(IDS_WARN_TO_CLOSE_EXECUTING_NETSCAPE, MB_ICONEXCLAMATION | MB_ICONWARNING);
			}*/

			// Continue updating the prefs.js file.
			bUpdatePrefsFile = UpdateNetscapePrefsFile(/*TRUE*/);
			if (TRUE == bUpdatePrefsFile)
			{
				// We are done & are using Postal API for handling netscape mailto
				bUseDDE = FALSE;					
			}
			else // Something went wrong ..fallback to DDE
			{
				// Use DDE
				bUseDDE = TRUE;

			}
		}
		else
		{
			// We are forced to use DDE
			bUseDDE = TRUE;
		}
	}
	else if( 2 == HIWORD(wParam) )// Eudora is not the default mailer
	{
		// Check if prefs file had the Eudora specific settings. If so delete them
		
		bCheckPrefsFile = CheckIfNetscapePrefsFileUpdated();
		if (bCheckPrefsFile == TRUE)
		{
			// Commenting tht next line because on some systems, prefs.js file is not getting updated for reasons yet to be known
			// & hence for 4.3 release this line is commented
			/*if (TRUE == IsNetscapeAbove405Executing())
			{
				// Warn the User to close all instances of netscape for setting to take effect
				::AfxMessageBox(IDS_WARN_TO_CLOSE_EXECUTING_NETSCAPE, MB_ICONEXCLAMATION | MB_ICONWARNING);
			}*/
			
			// Make sure prefs.js file does not have entry for EudoraNS.dll
			// Continue updating the prefs.js file.
			BOOL bUpdatePrefsFile = UpdateNetscapePrefsFile(FALSE);
			if (TRUE == bUpdatePrefsFile)
			{
				// We are done 			
				return 0;
			}
		}
	}

	// Handling LOWORD(wParam)
	
	switch(LOWORD(wParam))
	{
		case 0:
			break;
		case 1:	// Adding lines in prefs.js file to make Eudora as the default mailto handler
			bCheckPrefsFile = CheckIfNetscapePrefsFileUpdated();
			if (bCheckPrefsFile == FALSE)
			{	
				// Commenting tht next line because on some systems, prefs.js file is not getting updated for reasons yet to be known
				// & hence for 4.3 release this line is commented
				/*if (TRUE == IsNetscapeAbove405Executing())
				{
					// Warn the User to close all instances of netscape
					//::AfxMessageBox(IDS_WARN_MAILTO_SETTINGS_CHANGE, MB_ICONEXCLAMATION | MB_ICONWARNING);
				}*/

				// Continue updating the prefs.js file.
				bUpdatePrefsFile = UpdateNetscapePrefsFile(/*TRUE*/);
				if (TRUE == bUpdatePrefsFile)
				{
					// We are done & are using Postal API for handling netscape mailto
					bUseDDE = FALSE;					
				}
				else // Something went wrong ..fallback to DDE
				{
					// Use DDE
					bUseDDE = TRUE;

				}
			}
			break;
		case 2:	// Remove lines in prefs.js file so that Eudora is not the default mailto handler
			bCheckPrefsFile = CheckIfNetscapePrefsFileUpdated();
			if (bCheckPrefsFile == TRUE)
			{
				// Commenting tht next line because on some systems, prefs.js file is not getting updated for reasons yet to be known
				// & hence for 4.3 release this line is commented					
				/*if (TRUE == IsNetscapeAbove405Executing())
				{
					// Warn the User to close all instances of netscape for setting to take effect
					//::AfxMessageBox(IDS_WARN_MAILTO_SETTINGS_CHANGE, MB_ICONEXCLAMATION | MB_ICONWARNING);
				}*/
				
				// Make sure prefs.js file does not have entry for EudoraNS.dll
				// Continue updating the prefs.js file.
				bUpdatePrefsFile = UpdateNetscapePrefsFile(FALSE);
				if (TRUE == bUpdatePrefsFile)
				{
					// We are done 			
					bUseDDE = TRUE;
				}
			}

			break;

		case 3:	
			// Make sure that DDE is not being forced to handle mailto
			if (FALSE == GetIniShort(IDS_INI_FORCE_DDE_FOR_HANDLING_MAILTO))
			{
				bCheckPrefsFile = CheckIfNetscapePrefsFileUpdated();
				if (bCheckPrefsFile == TRUE)
				{
					// Use Postal API				
					bUseDDE = FALSE;
				}
				else
				{
					// Use DDE this time
					bUseDDE = TRUE;			
				}

				// Just making sure that if this specific version is lower than 4.05

				/*if (FALSE == IsNetscapeAbove405Executing())
				{
					bUseDDE = TRUE;					
				}*/	
			}
			else
			{
				bUseDDE = TRUE;
			}

			break;
		default :
			break;
	}

	
	//if (GetIniShort(IDS_INI_NETSCAPE_MAILTO))
	if (TRUE == bUseDDE)
	{
		// Register Eudora as Netscape's mailto handler

		//Create a worker thread for registering Eudora thru' DDE
		AfxBeginThread((AFX_THREADPROC) DDEClientThreadFunc, 
			           this, 
					   THREAD_PRIORITY_NORMAL, 
					   0,
					   0);
	}

 	return 0;
}


UINT DDEClientThreadFunc(LPVOID lpParameter)
{
	CDDEClient dde_client;

	// Since these are blocking functions, we don't need to have a wait after calling these functions
	// but before exiting the thread.
	dde_client.RegisterNetscapeProtocol("mailto");
	dde_client.RegisterNetscapeProtocol("x-eudora-option");
	
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
	if (WarnYesNoDialog(0, IDS_TIME_BOMB_MESSAGE_BETA) == IDOK)
		LaunchURLWithQuery(NULL, ACTION_UPDATE);

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
		else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CTextFileView ) ) )
		{
			enable = TRUE;
		}
		else if ( focusWnd->IsKindOf( RUNTIME_CLASS( CSignatureView ) ) )
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

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	if (wParam <= QC_LAST_COMMAND_ID && wParam >= QC_FIRST_COMMAND_ID)
	{
		QCCommandObject*	pCommand;
		COMMAND_ACTION_TYPE	theAction;

		if (g_theCommandStack.Lookup((WORD)wParam, &pCommand, &theAction) && pCommand)
		{
			UINT uID = pCommand->GetFlyByID(theAction);

			if (uID)
				wParam = uID;
		}
	}
	else if (( wParam >= TOOBAR_AD_CMD_BASE ) && 
			 ( wParam <= TOOBAR_AD_CMD_BASE + MAX_TOOLBAR_ADS ))
	{
		int					 iArrayPos = wParam - TOOBAR_AD_CMD_BASE;
		if (iArrayPos < m_arrayToolBarAds.GetSize())
		{
			CString			 strMessage(_T(""));
			CAdInfo			*ptbiTemp = (CAdInfo*)m_arrayToolBarAds.GetAt(iArrayPos);
			if (ptbiTemp)
			{
				strMessage = ptbiTemp->m_strURL;

				//	Remove everything up to and including "url="
				int		pos = strMessage.Find("url=");

				if (pos > 0)
				{
					strMessage = strMessage.Right(strMessage.GetLength() - pos - 4);
				}
				
				//	Remove everything after "&distributorid"
				pos = strMessage.Find("&distributorid");
				if (pos > 0)
				{
					strMessage = strMessage.Left(pos);
				}
				//	Unescape for nicer display
				strMessage = UnEscapeURL(strMessage);

				SetMessageText(strMessage);
				return 0;
			}
		}
	}

	return (QCWorkbook::OnSetMessageString(wParam, lParam));
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

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	// Give dynamic menus a chance to populate themselves
	CDynamicMenu* pDynMenu = DYNAMIC_DOWNCAST(CDynamicMenu, pPopupMenu);
	if (pDynMenu)
		pDynMenu->OnInitMenuPopup(nIndex, bSysMenu);

	QCWorkbook::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
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

		pCommand->Execute( theAction, (void*)LPCTSTR(Name) );
		return TRUE;
	}

	if( theAction == CA_SELECTED_TEXT_URL )
	{
		ASSERT_KINDOF( QCSelectedTextURLCommand, pCommand );

		CString SelText, EscapedURLText;
		GetSelectedText(SelText);
		EscapedURLText = EscapeURL(SelText);
		pCommand->Execute( theAction, (void*)(LPCSTR)EscapedURLText );
		return TRUE;
	}

	if( theAction == CA_SAVED_SEARCH )
	{
		ASSERT_KINDOF( SearchManager::SavedSearchCommand, pCommand );
		
		//	Open the SearchView
		OnEditFindFindMsg();

		//	Now the global doc should be avail
		ASSERT(gSearchDoc);
		if (gSearchDoc)
		{
			//	Get the SearchView
			POSITION		pos = gSearchDoc->GetFirstViewPosition();
			CSearchView *	pSearchView = (CSearchView *) gSearchDoc->GetNextView(pos);
			ASSERT(pSearchView);

			if (pSearchView)
			{
				//	Post the command for the SearchView to handle
				SearchManager::SavedSearchCommand *		pSavedSearchCommand =
						reinterpret_cast<SearchManager::SavedSearchCommand *>(pCommand);
				
				pSearchView->PostMessage( WM_COMMAND, pSavedSearchCommand->GetCommandID() );
			}
		}

		return TRUE;
	}

	pCommand->Execute( theAction );
	return TRUE;
}

void CMainFrame::OnUpdateDynamicCommand(CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	
	 			
	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( theAction == CA_TRANSFER_TO || theAction == CA_TRANSFER_NEW )
			{
				if (m_bTransferMenuBeingUsed)
					return;
			}
			if( theAction == CA_INSERT_RECIPIENT )
			{
				if ( CanModifyEdit() )
				{
					pCmdUI->Enable( TRUE );
					return;
				}
			}
			else if( theAction == CA_TRANSLATE_PLUGIN )
			{
				// Translators can work on any edit view
				CView*	pView = GetActiveFrame()->GetActiveView();
				
				if( pView )
				{
					CWnd*	pFocusWnd = pView->GetFocus();
					
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
			else if( theAction == CA_SPECIAL_PLUGIN)
			{
				BOOL					bShouldEnable = FALSE;
				QCPluginCommand *		pPluginCommand = reinterpret_cast<QCPluginCommand *>(pCommand);
				CSpecial *				pSpecial = pPluginCommand->GetSpecialObject();
				
				if ( pSpecial )
				{
					CTransAPI *		pTransAPI = pSpecial->GetTransAPI();
					if ( pTransAPI )
						bShouldEnable = !pTransAPI->IsFullFeatureSetOnly() || UsingFullFeatureSet();
				}

				pCmdUI->Enable(bShouldEnable);
				return;
			}
			else if( theAction == CA_SELECTED_TEXT_URL )
			{
				CString SelText;

				GetSelectedText(SelText);
				pCmdUI->Enable(!SelText.IsEmpty());
				return;
			}
			else if( theAction == CA_SAVED_SEARCH)
			{
				pCmdUI->Enable( UsingFullFeatureSet() ? TRUE : FALSE );
				return;
			}
			else if(( theAction == CA_NEW_MESSAGE_WITH ) ||
					( theAction == CA_NEW_MAILBOX ) ||
					( theAction == CA_NEW_MESSAGE_TO ) ||
					( theAction == CA_ATTACH_PLUGIN ) ||
					( ( ( theAction == CA_OPEN ) || ( theAction == CA_DELETE ) ) && pCommand->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) ) ||
					( ( theAction == CA_OPEN ) && pCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) ) ||
					( ( theAction == CA_OPEN ) && pCommand->IsKindOf( RUNTIME_CLASS( QCStationeryCommand ) ) ) ||
					( ( theAction == CA_DELETE ) && pCommand->IsKindOf( RUNTIME_CLASS( QCRecipientCommand ) ) ) )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->Enable( FALSE );
}

static UINT g_SelectedTextURLCommandIDs[9] = {0,0,0,0,0,0,0,0,0};

BOOL CMainFrame::OnSelectedTextURLCommand(UINT nID)
{
	int Index = nID - ID_EDIT_CTRL_2 + 1;
	UINT MappedID = g_SelectedTextURLCommandIDs[Index];

	if (MappedID)
		return OnDynamicCommand(MappedID);

	return FALSE;
}

void CMainFrame::FixMenuDrawingIfXP()
{
	// COLOR_MENUBAR is only defined starting with XP, so older
	// versions of Windows will just return NULL.
	HBRUSH		hMenuBarColorBrush = GetSysColorBrush(COLOR_MENUBAR);
	CMenu *		pMainMenu = GetMenu();

	// We should always be able to get our main menu
	ASSERT(pMainMenu);

	if (pMainMenu && hMenuBarColorBrush)
	{
		// We're running in XP or later
		HMODULE		hUser32 = GetModuleHandle("User32.dll");

		// We should always be able to get the User32.dll
		ASSERT(hUser32);

		if (hUser32)
		{
			// Get the SetMenuInfo function from User32.dll (available in Windows 98/2000 and later).
			typedef BOOL			(__stdcall *IndirectSetMenuInfo)(HMENU, LPCMENUINFO);
			IndirectSetMenuInfo		pSetMenuInfo = reinterpret_cast<IndirectSetMenuInfo>( GetProcAddress(hUser32, "SetMenuInfo") );

			// We should always be able to get pSetMenuInfo, because we already know that
			// we're running in XP or later.
			ASSERT(pSetMenuInfo);
			
			if (pSetMenuInfo)
			{
				// When using the built in menus on XP, there's a strange bug - hovering over
				// the menu bar draws a menu title with white behind the text. Clicking on it draws
				// the entire rectangle with a white background - even after the user has moved
				// on to another menu.
				//
				// Recent Microsoft apps don't seem to have this issue, because none of them
				// use the built in menus - they all do toolbar menus. We may want to something
				// similar eventually, but for now we just want to draw better in XP.
				//
				// Our fix is this:
				// Set the entire menu background (including all submenus) to be the current menu
				// color (on XP this is typically white by default). Then set just the menu (i.e. the menu bar)
				// to be the menu bar color (on XP this is typically gray by default).
				// So basically we're just setting the menu to draw with the colors that it should
				// already be using. For whatever reason this seems to fix menu drawing. I have
				// no idea why.
				MENUINFO	menuInfo;
				menuInfo.cbSize = sizeof(MENUINFO);
				menuInfo.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
				menuInfo.hbrBack = GetSysColorBrush(COLOR_MENU);
				
				pSetMenuInfo(pMainMenu->GetSafeHmenu(), &menuInfo);
				
				menuInfo.fMask = MIM_BACKGROUND;
				menuInfo.hbrBack = hMenuBarColorBrush;
				
				pSetMenuInfo(pMainMenu->GetSafeHmenu(), &menuInfo);
			}
		}
	}
}

void CMainFrame::InitMenus()
{
	FixMenuDrawingIfXP();
	
	// Edit menu
	VERIFY(m_hEditMenu = GetTopMenuByPosition(MAIN_MENU_EDIT));

	// Edit->Insert Recipients submenu
	m_pEditInsertRecipientMenu = DEBUG_NEW CDynamicRecipientMenu(CA_INSERT_RECIPIENT);
	m_pEditInsertRecipientMenu->Attach(GetSubMenu(m_hEditMenu, POS_EDIT_INSERTRECIPIENT_MENU));

	// Edit->Message Plug-ins submenu
	m_pEditMessagePluginsMenu = DEBUG_NEW CDynamicPluginMenu(CA_TRANSLATE_PLUGIN);
	m_pEditMessagePluginsMenu->Attach(GetSubMenu(m_hEditMenu, GetMenuItemCount(m_hEditMenu) - 1));

	// Edit->Selected Text URL menu items
	for (int i = 1; 1; i++)
	{
		char ININame[32];
		CString Value;
		int sep;

		sprintf(ININame, "SelectedTextURL%d", i);
		GetEudoraProfileString(CRString(IDS_INISN_SETTINGS), ININame, &Value);
		if (Value.IsEmpty())
			break;

		sep = Value.Find('\t');

		if (sep > 0)
		{
			char* buf = Value.GetBuffer(0);
			buf[sep] = 0;
			CString Name(buf);

			// Add accelerators:
			// 1-3 map to Ctrl+7 - Ctrl+9
			// 4-7 map to Ctrl+2 - Ctrl+5
			if (i <= 7)
			{
				Name += "\tCtrl+";
				if (i <= 3)
					Name += (char)('6' + i);
				else
					Name += (char)('1' + (i - 3));
			}

			QCSelectedTextURLCommand* pCommand = DEBUG_NEW QCSelectedTextURLCommand(buf + sep + 1);
			UINT uID = g_theCommandStack.AddCommand(pCommand, CA_SELECTED_TEXT_URL);
			::InsertMenu(m_hEditMenu, ::GetMenuItemCount(m_hEditMenu) - 2, MF_BYPOSITION,
							uID, Name);

			// Save off IDs for acceleartor items so they can be used when accelerators are chosen
			if (i <= 3)
				g_SelectedTextURLCommandIDs[5 + i] = uID;
			else if (i <= 7)
				g_SelectedTextURLCommandIDs[i - 3] = uID;
		}
	}

	// We allow the user to swap the Ctrl-F and Shif-Ctrl-F functionality. We handle the action
	// separately from the command, so here we should init the menu to reflect what command
	// is what accelerator.
	//
	// Default is Ctrl-F maps to Find Msgs, Shift-Ctrl-F maps to Find Text.
	//
	if (m_bSwitchFindAccel)
	{
		CMenu* pEditMenu = CMenu::FromHandle(m_hEditMenu);
		CString sFindMsg, sFindText;
		VERIFY(pEditMenu->GetMenuString(ID_EDIT_FIND_FINDMSG, sFindMsg, MF_BYCOMMAND) > 0);
		VERIFY(pEditMenu->GetMenuString(ID_EDIT_FIND_FINDTEXT, sFindText, MF_BYCOMMAND) > 0);

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

		VERIFY(pEditMenu->ModifyMenu(ID_EDIT_FIND_FINDMSG, MF_BYCOMMAND | MF_STRING, ID_EDIT_FIND_FINDMSG, (LPCSTR) sFindMsg));
		VERIFY(pEditMenu->ModifyMenu(ID_EDIT_FIND_FINDTEXT, MF_BYCOMMAND | MF_STRING, ID_EDIT_FIND_FINDTEXT, (LPCSTR) sFindText));
	}

	// START MESSAGE MENU
	// Message menu
	VERIFY(m_hMessageMenu = GetTopMenuByPosition(MAIN_MENU_MESSAGE));

	//Menu popup  --> New Message to
	m_pMessageNewMessageToMenu = DEBUG_NEW CDynamicRecipientMenu(CA_NEW_MESSAGE_TO);
	m_pMessageNewMessageToMenu->Attach(GetSubMenu(m_hMessageMenu, POS_NEW_MESSAGE_TO));
	
	//Menu popup  --> Forward to
	m_pMessageForwardToMenu = DEBUG_NEW CDynamicRecipientMenu(CA_FORWARD_TO);
	m_pMessageForwardToMenu->Attach(GetSubMenu(m_hMessageMenu, POS_FORWARD_TO));

	//Menu popup  --> Redirect To
	m_pMessageRedirectToMenu = DEBUG_NEW CDynamicRecipientMenu(CA_REDIRECT_TO);
	m_pMessageRedirectToMenu->Attach(GetSubMenu(m_hMessageMenu, POS_REDIRECT_TO));
 
	//Menu popup  --> New Message With
	m_pMessageNewMessageWithMenu = DEBUG_NEW CDynamicStationeryMenu(CA_NEW_MESSAGE_WITH);
	m_pMessageNewMessageWithMenu->Attach(GetSubMenu(m_hMessageMenu, POS_NEW_MESSAGE_WITH));

	//Menu popup  --> Reply With
	m_pMessageReplyWithMenu = DEBUG_NEW CDynamicStationeryMenu(CA_REPLY_WITH);
	m_pMessageReplyWithMenu->Attach(GetSubMenu(m_hMessageMenu, POS_REPLY_WITH));

	//Menu popup  --> Reply to All With
	m_pMessageReplyToAllWithMenu = DEBUG_NEW CDynamicStationeryMenu(CA_REPLY_TO_ALL_WITH);
	m_pMessageReplyToAllWithMenu->Attach(GetSubMenu(m_hMessageMenu, POS_REPLY_TO_ALL_WITH));
	
	//Menu popup  --> Attach
	m_pMessageAttachMenu = DEBUG_NEW CDynamicPluginMenu(CA_ATTACH_PLUGIN);
	m_pMessageAttachMenu->Attach(GetSubMenu(m_hMessageMenu, POS_ATTACH));

	//Menu popup  --> Change
	CMenu* pSubMenu;
	VERIFY(pSubMenu = CMenu::FromHandle(GetSubMenu(m_hMessageMenu, POS_CHANGE)));
	if (pSubMenu)
	{
		//
		// Update items on the Message:Change submenu.  First, add bitmaps
		// to the Message:Change:Status menu items.  Second, add bitmaps
		// to the Message:Change:Priority menu items.  Third, populate the
		// Message:Change:Persona menu.
		//
		CMenu* pSubSubMenu = pSubMenu->GetSubMenu(1);		// Message:Change:Status
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

		pSubSubMenu = pSubMenu->GetSubMenu(2);				// Message:Change:Priority
		ASSERT(g_PriorityHighestMenuBitmap.GetSafeHandle());		// bitmaps should be loaded before we get here
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_HIGHEST, MF_BYCOMMAND, &g_PriorityHighestMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_HIGH,    MF_BYCOMMAND, &g_PriorityHighMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_NORMAL,  MF_BYCOMMAND, &g_PriorityNormalMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_LOW,     MF_BYCOMMAND, &g_PriorityLowMenuBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_PRIOR_LOWEST,  MF_BYCOMMAND, &g_PriorityLowestMenuBitmap, NULL);

		VERIFY(m_hMessageChangeLabelMenu = GetSubMenu(*pSubMenu, 3));	// Message:Change:Label

		pSubSubMenu = pSubMenu->GetSubMenu(4);				// Message:Change:Server Status
		ASSERT(g_ServerLeaveBitmap.GetSafeHandle());				// bitmaps should be loaded before we get here
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_LEAVE,        MF_BYCOMMAND, &g_ServerLeaveBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_FETCH,        MF_BYCOMMAND, &g_ServerFetchBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_DELETE,       MF_BYCOMMAND, &g_ServerDeleteBitmap, NULL);
		pSubSubMenu->SetMenuItemBitmaps(ID_MESSAGE_SERVER_FETCH_DELETE, MF_BYCOMMAND, &g_ServerFetchDeleteBitmap, NULL);

		m_pMessageChangePersonalityMenu = DEBUG_NEW CDynamicPersonalityMenu();
		m_pMessageChangePersonalityMenu->Attach(GetSubMenu(*pSubMenu, 5));	// Message:Change:Persona
	}
	// END MESSAGE MENU


	// Mailbox menu
	m_pMailboxMenu = DEBUG_NEW CDynamicMailboxMenu(NULL, FALSE);
	m_pMailboxMenu->Attach(GetTopMenuByPosition(MAIN_MENU_MAILBOX));

	// Transfer menu
	m_pTransferMenu = DEBUG_NEW CDynamicMailboxMenu(NULL, TRUE);
	m_pTransferMenu->Attach(GetTopMenuByPosition(MAIN_MENU_TRANSFER));

	// Special menu
	VERIFY(m_hSpecialMenu = GetTopMenuByPosition(MAIN_MENU_SPECIAL));
	if (GetIniShort(IDS_INI_REMOVE_CHANGE_PASSWORD))
		VERIFY(RemoveMenu(m_hSpecialMenu, ID_SPECIAL_CHANGEPASSWORD, MF_BYCOMMAND));

	// Special->Remove Recipient submenu
	m_pSpecialRemoveRecipientMenu = DEBUG_NEW CDynamicRecipientMenu(CA_DELETE);
	m_pSpecialRemoveRecipientMenu->Attach(GetSubMenu(m_hSpecialMenu, POS_REMOVE_RECIPIENT));

	// Edit->Find->Find Using submenu
	m_pSavedSearchMenu = DEBUG_NEW CDynamicSavedSearchMenu(CA_SAVED_SEARCH);
	HMENU		hFindSubMenu = GetSubMenu(m_hEditMenu, POS_EDIT_FIND_SUBMENU);
	if (hFindSubMenu)
	{
		HMENU	hFindUsingSubMenu = GetSubMenu(hFindSubMenu, POS_FIND_FINDUSING_SUBMENU);

		if (hFindUsingSubMenu)
			m_pSavedSearchMenu->Attach(hFindUsingSubMenu);
	}
	
	// Tools menu
	VERIFY(m_hToolsMenu = GetTopMenuByPosition(MAIN_MENU_TOOLS));

	// add the special plugins to the tools menu
	UINT uCount = GetMenuItemCount(m_hToolsMenu);

	// add the items, assuming that the last item is the Tools:Options
	// item and that the next to last item is a separator
	g_thePluginDirector.NewMessageCommands(CA_SPECIAL_PLUGIN, CMenu::FromHandle(m_hToolsMenu), uCount - 1);

	// check the count again
	UINT uNewCount = GetMenuItemCount(m_hToolsMenu);
	if (uNewCount - uCount)
	{
		// if things were added, add a separator
		InsertMenu(m_hToolsMenu, uNewCount - 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	}
	::WrapMenu(m_hToolsMenu);

	// Window menu
	VERIFY(m_hWindowMenu = GetTopMenuByPosition(MAIN_MENU_WINDOW));

	// register as a command client
	g_theMailboxDirector.Register(this);
	g_theRecipientDirector.Register(this);
	g_theStationeryDirector.Register(this);
	g_theSignatureDirector.Register(this);
	g_thePersonalityDirector.Register(this);
}

//
//	CMainFrame::InitJunkMenus()
//
//	Initialize the Junk menus.  On the first launch of this version or later
//	see if the user currently has any manual filters.  If they do, ask them
//	if they want to switch Ctrl-J to mean Junk not Filter Messages.  If they
//	do then redo some control key mappings.
//
//	Note: It would make sense to do this in InitMenus() but InitMenus() is
//	called at a bad time for the AlertDialog() call so do it here.
//
void CMainFrame::InitJunkMenus()
{
	// START MESSAGE MENU
	// Message menu
	VERIFY(m_hMessageMenu = GetTopMenuByPosition(MAIN_MENU_MESSAGE));

	// Special menu
	VERIFY(m_hSpecialMenu = GetTopMenuByPosition(MAIN_MENU_SPECIAL));

	// See if a mapping scheme has been selected.
	short		 sCtrlJ = GetIniShort(IDS_INI_WHO_GETS_CTRL_J);
	if (sCtrlJ == CTRL_J_UNKNOWN)
	{
		// No mapping scheme selected.
		if (CFilterActions::AnyManualFilters())
		{
			// User has manual filters: ask them what to do.
			int iResult = AlertDialog(IDD_CTRL_J_FOR_JUNK);
			if (iResult == IDOK)
			{
				sCtrlJ = CTRL_J_JUNK;
			}
			else
			{
				sCtrlJ = CTRL_J_FILTER;
			}
		}
		else
		{
			// User has no manual filters: map Ctrl-J to Junk.
			sCtrlJ = CTRL_J_JUNK;
		}
	}
	SetIniShort(IDS_INI_WHO_GETS_CTRL_J, sCtrlJ);

	CMenu			*pMessageMenu = CMenu::FromHandle(m_hMessageMenu);
	CMenu			*pSpecialMenu = CMenu::FromHandle(m_hSpecialMenu);

	if (pMessageMenu && pSpecialMenu)
	{
		CString			 strFilterMessages;
		CString			 strJunk;
		CString			 strNotJunk;

		VERIFY(pSpecialMenu->GetMenuString(ID_SPECIAL_FILTERMESSAGES, strFilterMessages, MF_BYCOMMAND) > 0);
		VERIFY(pMessageMenu->GetMenuString(ID_JUNK, strJunk, MF_BYCOMMAND) > 0);
		VERIFY(pMessageMenu->GetMenuString(ID_NOT_JUNK, strNotJunk, MF_BYCOMMAND) > 0);

		// Trim the accelerator text from the menu strings.
		int				 pos;

		if ((pos = strJunk.Find('\t')) >= 0) strJunk = strJunk.Left(pos);

		if ((pos = strNotJunk.Find('\t')) >= 0) strNotJunk = strNotJunk.Left(pos);

		if ((pos = strFilterMessages.Find('\t')) >= 0) strFilterMessages = strFilterMessages.Left(pos);

		if ( sCtrlJ == CTRL_J_JUNK )
		{
			// Change the menus to show their new mappings.
			strFilterMessages += "\tCtrl+Shift+L";
			strJunk += "\tCtrl+J";
			strNotJunk += "\tCtrl+Shift+J";
		}
		else
		{
			// Change the menus to show their new mappings.
			strFilterMessages += "\tCtrl+J";
		}

		VERIFY(pSpecialMenu->ModifyMenu(ID_SPECIAL_FILTERMESSAGES, MF_BYCOMMAND | MF_STRING, ID_SPECIAL_FILTERMESSAGES, (LPCSTR)strFilterMessages));
		VERIFY(pMessageMenu->ModifyMenu(ID_JUNK, MF_BYCOMMAND | MF_STRING, ID_JUNK, (LPCSTR)strJunk));
		VERIFY(pMessageMenu->ModifyMenu(ID_NOT_JUNK, MF_BYCOMMAND | MF_STRING, ID_NOT_JUNK, (LPCSTR)strNotJunk));
	}
}



void CMainFrame::Notify(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction, void* pData)
{
	CMenu*				pMenu;
	CMenu				theSubMenu;
	UINT				uID;
	CString				szName;
	CString				szNamedPath;
	CString				szCurrentPath;
	INT					i;
	QCMailboxCommand*	pCommand;
	UnreadStatusType	theStatus;

	if (theAction == CA_SWM_CHANGE_FEATURE) 
		SWMNotify(pObject, theAction, pData);

	if (theAction != CA_DELETE &&
		theAction != CA_NEW &&
		theAction != CA_GRAFT &&
		theAction != CA_UPDATE_STATUS && 
		theAction != CA_RENAME)
	{
		return;
	}

	if (pObject->IsKindOf(RUNTIME_CLASS(QCRecipientCommand)))
	{
		szName = ((QCRecipientCommand*)pObject)->GetName();
		
		if (theAction == CA_DELETE || theAction == CA_RENAME)
		{
			// delete the old name from the Insert Recipient Menu
			if (m_pEditInsertRecipientMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pEditInsertRecipientMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pEditInsertRecipientMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID), CA_INSERT_RECIPIENT);
			}

			// delete the old name from the New Message To menu
			if (m_pMessageNewMessageToMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pMessageNewMessageToMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pMessageNewMessageToMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID), CA_NEW_MESSAGE_TO);
			}

			// delete the old name from the Forward To menu
			if (m_pMessageForwardToMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pMessageForwardToMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pMessageForwardToMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID), CA_FORWARD_TO);
			}

			// delete the old name from the Redirect To menu
			if (m_pMessageRedirectToMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pMessageRedirectToMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR) pData, *m_pMessageRedirectToMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID), CA_REDIRECT_TO);
			}

			// delete the old name from Remove Recipient menu
			if (m_pSpecialRemoveRecipientMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pSpecialRemoveRecipientMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pSpecialRemoveRecipientMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID), CA_DELETE);
			}
		}
		else if (theAction == CA_NEW)
		{
			// add the name to the Insert Recipient menu
			if (m_pEditInsertRecipientMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_INSERT_RECIPIENT);
				::AddMenuItem(uID, szName, *m_pEditInsertRecipientMenu);
			}

			// add the name to the New Message To menu
			if (m_pMessageNewMessageToMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_NEW_MESSAGE_TO);
				::AddMenuItem(uID, szName, *m_pMessageNewMessageToMenu);
			}
			
			// add the name to the Forward To menu
			if (m_pMessageForwardToMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_FORWARD_TO);
				::AddMenuItem(uID, szName, *m_pMessageForwardToMenu);
			}

			// add the name to the Redirect To menu
			if (m_pMessageRedirectToMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_REDIRECT_TO);
				::AddMenuItem(uID, szName, *m_pMessageRedirectToMenu);
			}

			// add the name to the Remove Recipient menu
			if (m_pSpecialRemoveRecipientMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_DELETE);
				::AddMenuItem(uID, szName, *m_pSpecialRemoveRecipientMenu);
			}
		}
	}
	else if (pObject->IsKindOf(RUNTIME_CLASS(QCStationeryCommand)))
	{
		szName = ((QCStationeryCommand*)pObject)->GetName();
		
		if (theAction == CA_DELETE || theAction == CA_RENAME)
		{
			// delete the old name from the New Message With menu
			if (m_pMessageNewMessageWithMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pMessageNewMessageWithMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pMessageNewMessageWithMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID), CA_NEW_MESSAGE_WITH);
			}

			// delete the old name from the Reply With menu
			if (m_pMessageReplyWithMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pMessageReplyWithMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pMessageReplyWithMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID),  CA_REPLY_WITH);
			}

			// delete the old name from the Reply to All With menu
			if (m_pMessageReplyToAllWithMenu->HasBeenBuilt())
			{
				uID = ::RemoveMenuItem(szName, *m_pMessageReplyToAllWithMenu);
				if (theAction == CA_RENAME)
					::AddMenuItem(uID, (LPCSTR)pData, *m_pMessageReplyToAllWithMenu);
				else
					g_theCommandStack.DeleteCommand(WORD(uID),  CA_REPLY_TO_ALL_WITH);
			}
		}
		else if (theAction == CA_NEW)
		{
			// Bug 2760.  If '&' is used in a name it is underlined in the menu..this call fixes this
			// Will insert another '&' so it will appear correctly
			char szNewName[255];
			GenerateNewName(szName, szNewName);

			// add the name to the New Message With menu
			if (m_pMessageNewMessageWithMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_NEW_MESSAGE_WITH);
				::AddMenuItem(uID, szNewName, *m_pMessageNewMessageWithMenu);
			}

			// add the name to the Reply With menu
			if (m_pMessageReplyWithMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_REPLY_WITH);
				::AddMenuItem(uID, szNewName, *m_pMessageReplyWithMenu);
			}

			// add the name to the Reply To All With menu
			if (m_pMessageReplyToAllWithMenu->HasBeenBuilt())
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_REPLY_TO_ALL_WITH);
				::AddMenuItem(uID, szNewName, *m_pMessageReplyToAllWithMenu);
			}
		}
	}
	else if (pObject->IsKindOf(RUNTIME_CLASS(QCMailboxCommand)))
	{
		if (theAction == CA_UPDATE_STATUS)
		{	
			// I HATE having to do this here, but OnUpdate... doesn't
			// get called for folder items :-(
					
			pCommand = (QCMailboxCommand*)pObject;
			szNamedPath = g_theMailboxDirector.BuildNamedPath( pCommand );

			if (pCommand->IsImapType())
			{
				pMenu = ::ImapFindNamedMenuItem(szNamedPath, m_pMailboxMenu, &i, 
										((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(FALSE));
			}
			else
				pMenu = ::FindNamedMenuItem(szNamedPath, m_pMailboxMenu, &i, pCommand->GetType() == MBT_FOLDER);

			// on startup menu may not be ready yet
			if (!pMenu)
				return;

			theStatus = (UnreadStatusType)(ULONG)pData;
			pMenu->CheckMenuItem(i, MF_BYPOSITION | (theStatus == US_YES? MF_CHECKED : MF_UNCHECKED));
		}

		// handle the mailboxes menu
		HandleMailboxNotification(m_pMailboxMenu, CA_OPEN, (QCMailboxCommand*)pObject,
			theAction, pData);

		// handle the transfer menu
		HandleMailboxNotification(m_pTransferMenu, CA_TRANSFER_TO, (QCMailboxCommand*)pObject,
			theAction, pData);
	}
	else if (pObject->IsKindOf(RUNTIME_CLASS(QCPersonalityCommand)))
	{
		// Only need to update the menu if it's been built already
		if (m_pMessageChangePersonalityMenu->HasBeenBuilt())
		{
			szName = ((QCPersonalityCommand*)pObject)->GetName();
			
			if (theAction == CA_DELETE)
			{
				// delete the old name from the Change Personality Menu
				uID = ::RemoveMenuItem(szName, *m_pMessageChangePersonalityMenu);
				g_theCommandStack.DeleteCommand(WORD(uID), CA_CHANGE_PERSONA);
			}
			else if (theAction == CA_NEW)
			{
				uID = g_theCommandStack.AddCommand(pObject, CA_CHANGE_PERSONA);
				::AddMenuItem(uID, szName, *m_pMessageChangePersonalityMenu);
			}
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

	uPos = ::AddMenuItem( uID, szMenuItem, *pMenu, *pSubMenu, iStartPos, iEndPos );

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

void CMainFrame::HandleMailboxNotification(CMenu* pMenu, COMMAND_ACTION_TYPE theNewAction,
	QCMailboxCommand* pCommand, COMMAND_ACTION_TYPE theAction, void* pData)
{
	CMenu*	pSubMenu;
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

	CDynamicMailboxMenu* pDynMenu = DYNAMIC_DOWNCAST(CDynamicMailboxMenu, pMenu);
	if (!pMenu)
	{
		ASSERT(0);
		return;
	}

	// No top-level menu yet?  Nothing to do since it will get bulit later.
	if (!pDynMenu->HasBeenBuilt())
		return;

	bMaximized = FALSE;
	MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );

	if (theNewAction == CA_TRANSFER_TO)
	{
		uMenuIndex = MAIN_MENU_TRANSFER + uOffset;
		bIsXferMenu = TRUE;
	}
	else
	{
		uMenuIndex = MAIN_MENU_MAILBOX + uOffset;
		bIsXferMenu = FALSE;
	}

	if (theAction == CA_DELETE || theAction == CA_RENAME || theAction == CA_GRAFT)
	{
		// find the current menu entry
		szNamedPath = g_theMailboxDirector.BuildNamedPath( pCommand );

		if (pCommand->IsImapType())
		{
			pSubMenu = ::ImapFindNamedMenuItem( szNamedPath, pMenu, &i, 
									((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu));
		}
		else
			pSubMenu = ::FindNamedMenuItem( szNamedPath, pMenu, &i, ( pCommand->GetType() == MBT_FOLDER ) );

		// Might not exist because the menu hasn't been created yet.
		if (!pSubMenu)
			return;

		uID = pSubMenu->GetMenuItemID(i);
		
// IMAP4  // Account for IMAP types.
		if( ( pCommand->GetType() == MBT_FOLDER ) ||
			( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu) ) )
		{
			if ((INT)uID != -1)
			{
				// this should never happen
				ASSERT(0);
				return;
			}
			
			pMenu = pSubMenu->GetSubMenu( i );
		}
		else
			ASSERT((INT)uID != -1);

		// finally, remove the item
		pSubMenu->RemoveMenu(i, MF_BYPOSITION);

		// If we're deleting an item corresponding to an IMAP ACCOUNT object, we may need to delete
		// a following separator item.
		//
		if (pCommand->GetType() == MBT_IMAP_ACCOUNT)
		{
			// Note: "i" should now point to the separator item (if there's one).
			// Separator items have ID's of zero.
			//
			if (((UINT)i < pSubMenu->GetMenuItemCount()) && (pSubMenu->GetMenuItemID(i) == 0))
				pSubMenu->RemoveMenu(i, MF_BYPOSITION);
		}

		if (theAction == CA_DELETE)
		{
			// unwrap if necessary
			WrapMenu(pSubMenu->GetSafeHmenu());

// IMAP4
			if( ( pCommand->GetType() == MBT_FOLDER ) ||
				( pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu) ) )
			{
				// walk the menu and delete all the commands 
				// actually, this is just a precaution -- the menu should 
				// be empty by the time we get here because the recursion
				// processes all mailboxes before folders.

				g_theCommandStack.DeleteAllMenuCommands(pMenu, theNewAction);
				g_theCommandStack.DeleteAllMenuCommands(pMenu, bIsXferMenu? CA_TRANSFER_NEW : CA_NEW_MAILBOX);

				// destroy the menu
				pMenu->DestroyMenu();
			}
			else
				g_theCommandStack.DeleteCommand(WORD(uID),  theNewAction);

			return;	
		}

		if (theAction == CA_RENAME)
		{
			// Adding an IMAP mailbox is different. Add IMAP accounts in alphabetical order after
			// POP folders. 
			//
			if (pCommand->IsImapType())
				iFolderPos = IMAPFindStartingPos ((QCImapMailboxCommand *)pCommand);
			else
			{
				// find the first folder 
				iFolderPos = pSubMenu->GetMenuItemCount();

				while (--iFolderPos >= 0)
				{
					UINT mID = pSubMenu->GetMenuItemID(iFolderPos);

					// Skip separators as well.
					if (mID && (mID != (UINT)-1))
						break;
				}
				iFolderPos ++;
			}

//  IMAP4
			if (pCommand->GetType() == MBT_FOLDER ||
				(pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShouldShowAsSubMenu()))
			{
				// add it to the menu in alphabetical order.
				AddMailboxMenuItem(0, (LPCSTR)pData, pSubMenu, pMenu, iFolderPos, INT_MAX, bIsXferMenu);
			}
			else if (pCommand->IsImapType())
				AddMailboxMenuItem(uID, (LPCSTR)pData, pSubMenu, NULL, iFolderPos, INT_MAX, bIsXferMenu);
			else
			{
				// if it's in the root, insert it in alphabetical order after the
				// 5th item, else add it after the first
				if (pSubMenu == GetMenu()->GetSubMenu(uMenuIndex))
					iMailboxPos = 4;
				else
					iMailboxPos = 1;
				
				AddMailboxMenuItem(uID, (LPCSTR)pData, pSubMenu, NULL, iMailboxPos, iFolderPos, bIsXferMenu);
			}
			return;
		} 

		// ok were grafting 
		// find the new parent
		pSubMenu = GetMenu()->GetSubMenu(uMenuIndex);

		if (pData)
		{
			ASSERT_KINDOF(QCMailboxCommand, (QCMailboxCommand*)pData);
			szNamedPath = g_theMailboxDirector.BuildNamedPath((QCMailboxCommand*)pData);

// IMAP4
			if (((QCMailboxCommand*)pData)->IsImapType())
				pSubMenu = ::ImapFindNamedMenuItem( szNamedPath, pSubMenu, &i, TRUE);
			else
				pSubMenu = ::FindNamedMenuItem( szNamedPath, pSubMenu, &i, TRUE );
	
			if (pSubMenu == NULL || (pSubMenu = pSubMenu->GetSubMenu( i )) == NULL)
			{
				// Might happen if the menu isn't built yet.
				return;
			}
		}

		// find the first folder 
		iFolderPos = pSubMenu->GetMenuItemCount();

		while (--iFolderPos >= 0)
		{
			UINT mID = pSubMenu->GetMenuItemID(iFolderPos);

			// Skip separators as well.
			if (mID && (mID != (UINT)-1))
				break;
		}
		iFolderPos++;

		// insert the new item
//  IMAP4
		if (pCommand->GetType() == MBT_FOLDER ||
			(pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShownAsSubMenu(bIsXferMenu)))
		{
			AddMailboxMenuItem(0, pCommand->GetName(), pSubMenu, pMenu, iFolderPos, INT_MAX, bIsXferMenu);
		}
		else
		{
			// if it's in the root, insert it in alphabetical order after the
			// 5th item, else add it after the first
			if (pSubMenu == GetMenu()->GetSubMenu(uMenuIndex))
				iMailboxPos = 4;
			else
				iMailboxPos = 1;
				
			AddMailboxMenuItem(uID, pCommand->GetName(), pSubMenu, NULL, iMailboxPos, iFolderPos, bIsXferMenu);
		}
		return;
	}

	if (theAction == CA_NEW)
	{
		// find the parent
		szNamedPath = g_theMailboxDirector.BuildNamedPath(pCommand);
		
		if ((i = szNamedPath.ReverseFind( '\\')) >= 0)
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
					ASSERT(0);
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
				pSubMenu = ::FindNamedMenuItem(szNamedPath, pMenu, &i, TRUE);

			// Might not exist because the menu hasn't been created yet.
			if (!pSubMenu)
				return;

			pMenu = pSubMenu->GetSubMenu(i);
			if (!pMenu)
			{
				// Now this is a problem!  It found it above, so why can't it now?
				ASSERT(0);
				return;
			}

			CDynamicMailboxMenu* pDynSubMenu = DYNAMIC_DOWNCAST(CDynamicMailboxMenu, pMenu);
			if (!pDynSubMenu || !pDynSubMenu->HasBeenBuilt())
			{
				ASSERT(pDynSubMenu);
				return;
			}
		}

		// add it to the transfer menu
		szName = pCommand->GetName();

		// Adding an IMAP mailbox is different. Add IMAP accounts in alphabetical order after
		// POP folders. 
		//
		if (pCommand->IsImapType())
			iFolderPos = IMAPFindStartingPos ((QCImapMailboxCommand *)pCommand);
		else
		{
			// find the first folder 
			iFolderPos = pMenu->GetMenuItemCount();

			while (--iFolderPos >= 0)
			{
				UINT mID = pMenu->GetMenuItemID( iFolderPos );

				// Skip separators as well.
				if (mID && (mID != (UINT)-1))
					break;
			}
			iFolderPos++;
		}

// IMAP4 
		if (pCommand->GetType() == MBT_FOLDER ||
			(pCommand->IsImapType() && ((QCImapMailboxCommand *)pCommand)->ShouldShowAsSubMenu()))
		{
			CDynamicMailboxMenu* theSubMenu = DEBUG_NEW CDynamicMailboxMenu(pCommand, bIsXferMenu);
			theSubMenu->CreatePopupMenu();

			AddMailboxMenuItem((UINT)-1, szName, pMenu, theSubMenu, iFolderPos, INT_MAX, bIsXferMenu);

			// IMAP ACCOUNT: Append a separator.
			if (pCommand->GetType() == MBT_IMAP_ACCOUNT)
				IMAPAppendSeparator((QCImapMailboxCommand *)pCommand, pMenu);

			// Tell command object that it's been added as a sub-menu:
			if (pCommand->IsImapType())
				((QCImapMailboxCommand *)pCommand)->SetShownAsSubMenu (TRUE, bIsXferMenu);
		}
		else if (pCommand->IsImapType())
		{
			uID = g_theCommandStack.AddCommand(pCommand, theNewAction);

			AddMailboxMenuItem(uID, szName, pMenu, NULL, iFolderPos, INT_MAX, bIsXferMenu);
		}
		else
		{
			// if it's in the root, insert it in alphabetical order after the
			// 5th item, else add it after the first
			if (pMenu == GetMenu()->GetSubMenu(uMenuIndex))
				iMailboxPos = 4;
			else
				iMailboxPos = 1;
				
			uID = g_theCommandStack.AddCommand(pCommand, theNewAction);

			AddMailboxMenuItem(uID, szName, pMenu, NULL, iMailboxPos, iFolderPos, theNewAction == CA_TRANSFER_TO);
		}
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
	if (!(pImapCommand && pNamedPath && pMenu && pOutIndex))
	{
		ASSERT (0);
		return NULL;
	}

	const BOOL bIsXferMenu = (theNewAction == CA_TRANSFER_TO || theNewAction == CA_TRANSFER_NEW);

	// Is this currently displayed as a submenu?
	if (pImapCommand->ShownAsSubMenu(bIsXferMenu))
		return ::ImapFindNamedMenuItem(pNamedPath, pMenu, pOutIndex, TRUE);

	// Otherwise, delete the menu item and replace it with a sub-menu.
	int index;
	CMenu* pSubMenu = ::ImapFindNamedMenuItem( pNamedPath, pMenu, &index, FALSE);

	if (!pSubMenu)
	{
		// We either found some problem, or the submenu hasn't been built yet.
		// If either case, there's nothing to change so just bail out.
		return NULL;
	}

	UINT uID = pSubMenu->GetMenuItemID(index);

	if ((INT)uID == -1)
	{
		// Huh?  It just found it above?
		ASSERT(0);
		return NULL;
	}

	// Remove the item
	pSubMenu->RemoveMenu(index, MF_BYPOSITION);

	// unwrap if necessary
	WrapMenu(pSubMenu->GetSafeHmenu());

	// The command ID may be different when we re-create it, so we have to delete the
	// item from the command stack.
	g_theCommandStack.DeleteCommand(WORD(uID), theNewAction);

	// Create now as a sub-menu:
//	this->Notify(pImapCommand, CA_NEW, NULL);

	HandleMailboxNotification(pMenu, theNewAction, pImapCommand, CA_NEW, NULL);

	return ::ImapFindNamedMenuItem(pNamedPath, pMenu, pOutIndex, TRUE);
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

void CMainFrame::OnMailboxJunk()
{
	QCMailboxCommand*	pCommand;

	pCommand = g_theMailboxDirector.FindByName( NULL, CRString( IDS_JUNK_MBOX_NAME ) );

	if( pCommand )
	{
		pCommand->Execute( CA_OPEN );
	}
	else
	{
		ASSERT( 0 );
	}
}

void CMainFrame::OnImportMail()
{
	CImportMailDlog importBawx;

	// TODO: Add wizard to offer advanced importing instead of that
	// dumb lookin empty tree control that users say.. now what?

	if(importBawx.m_pImporter->InitPlugins())
	{
		// TODO: Bring em to advanced if we don't find a provider.
		if(importBawx.m_pImporter->InitProviders(true))
			importBawx.DoModal();
		else
		{
			if(YesNoDialogPlain(IDS_ERR_NO_IMPORT_PROVIDERS) == IDOK)
			{
				CImportAdvDlog AdvDlog;
				AdvDlog.DoModal();
			}

		}
	}
	else
	{
		CString szUrlSite;
		GetJumpURL(&szUrlSite);
		ConstructURLWithQuery(szUrlSite, ACTION_SUPPORT, _T("no-importer"));

		WarnUrlDialog(CRString(IDR_MAINFRAME), CRString(IDS_ERR_IMPORT_DLL_NOTINSTALLED), CRString(IDS_TECHSPPT_TITLE), szUrlSite);
	}
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

	// This code is pretty important.  What is does is when the main window is
	// being disabled, it finds out if the window with the focus is not in an
	// MDI child window.  If that's true, then it looks if the focus window is
	// inside a CView-derived window, and saves that CView as the active view.
	// This allows CView-based objects in docked windows (e.g. wazoo windows)
	// to get the focus back after being reenabled.  If we didn't do this, MFC
	// would wind of giving focus to the first MDI child window.
	if (bEnable == FALSE)
	{
		CWnd* pWnd = GetFocus();
		CView* pActiveView = NULL;

		if (pWnd && pWnd->GetParentFrame() == this)
		{
			do
			{
				if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
				{
					pActiveView = (CView*)pWnd;
					break;
				}
			}
			while ((pWnd = pWnd->GetParent()) && pWnd != this);
		}

		SetActiveView(pActiveView);
	}

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
void CMainFrame::GenerateNewName(const char* strOldName, char* szNewName)
{
	const char* Amp = strchr(strOldName, '&');

	if (Amp)
	{
		int nFirstPos = Amp - strOldName;
		
		_mbsnbcpy((unsigned char*)szNewName, (unsigned char*) strOldName, nFirstPos + 1);
		
		INT nLen = strlen(strOldName);
		int i, j;
		for(i=j= (nFirstPos + 1); i < nLen; i++,j++)
		{
			if(strOldName[i] == '&')
			{
				szNewName[j] = '&';
				j++;
				szNewName[j] = '&';
			}
			else
				szNewName[j] = strOldName[i];
		}
		szNewName[j] = '\0';
		
	}
	else
		strcpy(szNewName, strOldName);
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
			((CMainFrame*)GetParent())->MDIGetActive(&bIsMax);
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
			pSWM->SetMode(SWM_MODE_LIGHT, true, NULL, true);
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
			pSWM->SetMode(SWM_MODE_PRO, true, NULL, true);
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
			pSWM->SetMode(SWM_MODE_ADWARE, true, NULL, true);
	}
}

void CMainFrame::OnUpdateRequestNewPlaylist(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ldb!=NULL);
}

void CMainFrame::OnRequestNewPlaylist()
{
	ASSERT(m_ldb);
	MGR_UpdatePlaylists(m_ldb,FALSE);
}

void CMainFrame::OnUpdateRequestBrandNewPlaylist(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ldb!=NULL);
}

void CMainFrame::OnRequestBrandNewPlaylist()
{
	MGR_UpdatePlaylists(m_ldb,TRUE);
}

void CMainFrame::OnUpdateDumpAdState(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ldb!=NULL);
}

void CMainFrame::OnDumpAdState()
{
	MGR_DumpState(m_ldb);
}

void CMainFrame::OnUpdateChangePlaylistServer(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnChangePlaylistServer(UINT uCmdID)
{
	switch (uCmdID)
	{
		case ID_DEBUG_PLAYLIST_SERVER0: LaunchURLNoHistory("x-eudora-option:PlaylistServer="); break;
		case ID_DEBUG_PLAYLIST_SERVER1: LaunchURLNoHistory("x-eudora-option:PlaylistServer=http://echo.qualcomm.com/adjoindev/playlists"); break;
		case ID_DEBUG_PLAYLIST_SERVER2: LaunchURLNoHistory("x-eudora-option:PlaylistServer=http://adserver.eudora.com/eitanzone/playlists"); break;
		case ID_DEBUG_PLAYLIST_SERVER3: LaunchURLNoHistory("x-eudora-option:PlaylistServer=http://echo.qualcomm.com/chandhokzone/plrestore"); break;
	}
}

#endif // _DEBUG


void CMainFrame::OnIdleUpdateCmdUI()
{
	QCWorkbook::OnIdleUpdateCmdUI();

	// DRW - Previously this method returned immediately if Eudora wasn't
	// in adware mode, but we need to serve ads when in light mode to get
	// the light sponsor, so only return out if we are in paid mode.
	if (GetSharewareMode() == SWM_MODE_PRO)
		return;

	static int LastScheduleCheckTime = 0;

	// Do ad checks and updates here because we're guaranteed not to be in some
	// modal dialog at the time since this gets called from the main idle loop.
	if (time(NULL) > LastScheduleCheckTime)
	{
		if (LastScheduleCheckTime)
		{
			// We need to get to the point where we at least
			// show one ad before we go in to suspend mode
			unsigned long ulIdlePeriod = g_pApp->GetIdlePeriodFacetime();

			// playlist client knows/cares about deadbeat mode
			SharewareModeType clientMode = (SharewareModeType) DEFAULT_SWM_MODE;
			QCSharewareManager* pSWM = QCSharewareManager::GetSharewareManager();

			if ( pSWM ) {
				if ( pSWM->IsDeadbeat() )
					clientMode = SWM_MODE_DEADBEAT;
				else
					clientMode = pSWM->GetMode();
			}

			MGR_Idle( m_ldb, !CAdView::OneAdShown() || ulIdlePeriod > 2,
					IsOffline() == FALSE, clientMode );

			LastScheduleCheckTime = time(NULL);
		}
		else
		{
			// Start scheduling ads here.  This will wait until startup is
			// finished because UpdateCmdUI handlers aren't called until you
			// get in to the main idle loop.
			MGR_BeginScheduling( m_ldb );
			LastScheduleCheckTime = 1;
		}
	}
}

// fetch a playlist -- pass "true" to override request-interval throttle
void CMainFrame::UpdatePlaylists( bool bUpdateNow )
{
	MGR_UpdatePlaylists( m_ldb, bUpdateNow );
}

/* static */ int CMainFrame::PlaylistCallback( int cmd, long userRef, long lParam )
{
	// DRW - Let the CAdView callback handle PLIST_BLANK and anything having to do with
	// kAd or kRunout.  Handle everything else here.
    switch ( cmd ) {
    case PLIST_SETADFAILURE:
    	SetAdFailure( lParam );
    	break;
    case PLIST_BLANK:
		// Let the CAdView callback handle it.
		CAdView::PlaylistCallback(cmd, userRef, lParam);
        break;
    case PLIST_SHOWTHIS:
	{
		ENTRY_INFO* pei = (ENTRY_INFO*) lParam;
		switch ( pei->entry_type )
		{
			case kAd:
			case kRunout:
			{
				// Let the CAdView callback handle it.
				CAdView::PlaylistCallback(cmd, userRef, lParam);
				break;
			}
			
			case kSponsor:
			{
				CString			 strAdID = pei->id;
				CString			 strButtonAdFile = pei->src;
				CString			 strAdURL = pei->href;
				CString			 strAdTitle = pei->title;
				if (strButtonAdFile.Left(7) == "file://")
				{
					strButtonAdFile = strButtonAdFile.Right(strButtonAdFile.GetLength() - 7);
				}
				// DRW - I do this several times in this method.  Since this is a static method,
				// we can't refer to "this" so even though we are the CMainFrame we still need
				// to derive ourselves.
				CMainFrame		*pmainframe = (CMainFrame*)AfxGetMainWnd();
				if (pmainframe)		pmainframe->ChangeSponsor(strAdID, strButtonAdFile, strAdURL, strAdTitle);
				break;
			}
			
			case kButton:
			{
				CString			 strAdID = pei->id;
				CString			 strButtonAdFile = pei->src;
				CString			 strAdURL = pei->href;
				CString			 strAdTitle = pei->title;
				if (strButtonAdFile.Left(7) == "file://")
				{
					strButtonAdFile = strButtonAdFile.Right(strButtonAdFile.GetLength() - 7);
				}
				// DRW - See above.
				CMainFrame		*pmainframe = (CMainFrame*)AfxGetMainWnd();
				if (pmainframe)		pmainframe->ShowToolBarAd(strAdID, strButtonAdFile, strAdURL, strAdTitle);
				break;
			}
		}
		MGR_DisposeEntryInfo( pei );
        break;
	}
    case PLIST_DELETE:
	{
		ENTRY_INFO* pei = (ENTRY_INFO*) lParam;
		switch ( pei->entry_type )
		{
			case kSponsor:
			{
				if (GetSharewareMode() != SWM_MODE_PRO)
				{
					// DRW - See above.
					CMainFrame		*pmainframe = (CMainFrame*)AfxGetMainWnd();
					if (pmainframe)		pmainframe->ResetSponsor();
				}
				break;
			}
			
			case kButton:
			{
				CString			 strAdID = pei->id;
				// DRW - See above.
				CMainFrame		*pmainframe = (CMainFrame*)AfxGetMainWnd();
				if (pmainframe)		pmainframe->HideToolBarAd(strAdID);
				break;
			}
		}
		MGR_DisposeEntryInfo( pei );
        break;
	}
	case PLIST_DELETE_PROFILEID:
	{
		SetIniString( IDS_INI_PROFILE, NULL );
		break;
	}
	case PLIST_SET_PROFILEID:
	{
		SetIniString( IDS_INI_PROFILE, (LPCSTR)lParam );
		break;
	}
	case PLIST_SET_CLIENTMODE:
	{
		// if the server says this loser is of the deadbeat variety, we give
		// them one more chance to sell out.
		if ( (SharewareModeType)lParam == SWM_MODE_DEADBEAT ) {

			time_t		ignoreTime;
			bool		bLowOnAds = MGR_GetClientUpdateStatus( CMainFrame::QCGetMainFrame()->GetLDB(), &ignoreTime );
			CString		szProfile;
			GetIniString(IDS_INI_PROFILE, szProfile);

			// We only do this if we believe the playlist servlet.
			// Apparently the playlist servlet can spit back a deadbeat mode we gave
			// it because we were low on ads (according to previous comment by SD).
			// Ignore if:
			// * We're not in sponsored mode (profile deadbeat only makes sense
			//   if we're in sponsored mode)
			// * We're low on ads and we already have a profile (in which case 
			//   CAdFailureNagTask will handle forcing the user to light)
			if ( (GetSharewareMode() != SWM_MODE_ADWARE) ||
				 (bLowOnAds && !szProfile.IsEmpty()) ) {
				 break;
			}

			// returns "true" if we should recheck the user's profile, and
			// defer setting the client mode until next time.
			if ( DoDeadbeatNag() ) {
				((CMainFrame*)AfxGetMainWnd())->UpdatePlaylists( true );
				break;
			}
		}

		// now set the mode. "false" means no downgrade ui.
		QCSharewareManager* psm = GetSharewareManager();
		psm->SetMode( (SharewareModeType)lParam, false );
		break;
	}
	case PLIST_NAG_USER:
	{
		NAG_INFO* pni = (NAG_INFO*)lParam;
		if ( pni )
			DoProfileNag( NULL, pni->level, (LPCSTR)pni->text );

		break;
	}
    
    // Noooooooo, we wouldn't want to pollute things by making
    // it easy to call these routines from the playlist manager,
    // would we?  BAH!  HUMBUG!
    case PLIST_FMBNEW:
    	*(FMBHANDLE*)userRef = NewFaceMeasure();
    	break;
    case PLIST_FMBDISP:
    	DisposeFaceMeasure((FMBHANDLE)userRef);
    	break;
    case PLIST_FMBBEGIN:
    	FaceMeasureBegin((FMBHANDLE)userRef);
    	break;
    case PLIST_FMBRESET:
    	FaceMeasureReset((FMBHANDLE)userRef);
    	break;
    case PLIST_FMBREPORT:
    	FaceMeasureReport((FMBHANDLE)userRef,(time_t *)lParam,NULL,NULL,NULL);
    	break;
    case PLIST_LOG:
		PutDebugLog(DEBUG_MASK_PLIST, (const char *)lParam); 
		break;
    case PLIST_MONITOR:
		{
			POSITION docpos = TextFileTemplate->GetFirstDocPosition();
			while (docpos)
			{
				CDocument* doc = TextFileTemplate->GetNextDoc(docpos);
				if ( doc && !strcmp("adsysmon.txt",doc->GetTitle( )) )
				{
					// lParam==NULL means the playlist manager is asking if
					// we have the window before it generates all the output
					if ( lParam )
					{
						CView* pView = NULL;
						POSITION pos = doc->GetFirstViewPosition();   
						if (pos != NULL)  
							pView = doc->GetNextView(pos); 
				
						if( pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CPaigeEdtView)) ) 
						{
							QCProtocol*	pProtocol = QCProtocol::QueryProtocol( QCP_TRANSLATE, pView );
							pProtocol->SetAllHTML((const char *)lParam, false);
							CPaigeEdtView *pPvw = DYNAMIC_DOWNCAST(CPaigeEdtView,pView);
							pPvw->SetReadOnly();
							doc->SetModifiedFlag( FALSE );
						}
					}
					
					return 0;	// we found it!
				}
			}
			return -1;	// we didn't find the window
		}
		break;
	case PLIST_GET_INI_STRING:
		// only allow the playlist server to be switched in debug or expiring builds
#if !defined(DEBUG) && !defined(EXPIRING)
		if ( (UINT)lParam==IDS_INI_DEBUG_PLAYLIST_SERVER ) *(char **)userRef = NULL; else
#endif
		{
			const char *theString = GetIniString ( (UINT)lParam );
			char *returnMe = DEBUG_NEW_NOTHROW char[strlen(theString)+1];
			if (returnMe) strcpy(returnMe,theString);
			*(char **)userRef = returnMe;
		}
		break;
    default:
        assert(0);
    }
    
    return 0;
}

// --------------------------------------------------------------------------

// SetAdFailure [PROTECTED]
//
// This stashes the ad failure value in a "safe" place; the Out toc
//
void CMainFrame::SetAdFailure( int days )
{
	CTocDoc* toc = GetOutToc();
	if ( toc )
	{
		toc->SetAdFailure( (BYTE) days );
	}
}

// --------------------------------------------------------------------------

// SetAdFailure [PROTECTED]
//
// This stashes the ad failure value in a "safe" place; the Out toc
//
int CMainFrame::GetAdFailure( )
{
	int days = 0;
	
	CTocDoc* toc = GetOutToc();
	if ( toc )
	{
		days = toc->GetAdFailure();
	}
	return days;
}

// --------------------------------------------------------------------------

// OnAConnectionWasMade [PUBLIC]
//
// Let the playlist manager know that a connection has been made
//
void CMainFrame::OnConnectionWasMade( )
{
	MGR_CheckingMail( m_ldb );
}

// --------------------------------------------------------------------------

// OnMCINotifyMode [PUBLIC]
//
// Notification when playing media files change mode
//
LRESULT CMainFrame::OnMCINotifyMode(WPARAM wParam, LPARAM lParam)
{
	// Have to do things this way because MCI notifications are sent
	// synchronously via SendMessage() and we sometimes need to be
	// notified asynchronously (e.g. message grabbed from msg queue).
	PostMessage(WM_USER_MCI_NOTIFY_MODE, wParam, lParam);

	return 0;
}

// --------------------------------------------------------------------------

// OnUserMCINotifyMode [PUBLIC]
//
// Asynchronous handler of mode changes so that we can delete media data
//
LRESULT CMainFrame::OnUserMCINotifyMode(WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = (HWND)wParam;

	if (lParam == MCI_MODE_STOP || lParam == MCI_MODE_NOT_READY)
	{
		// Save off the position of the media player window if it's visible
		if (lParam == MCI_MODE_STOP && (GetWindowStyle(hWnd) & WS_VISIBLE))
		{
			RECT rect;

			if (::GetWindowRect(hWnd, &rect))
				SetIniWindowPos(IDS_INI_PLAY_MEDIA_WINDOW_POS, rect);
		}

		MCIWndDestroy(hWnd);
	}

	return 0;
}

// --------------------------------------------------------------------------

// GetTopMenuByPosition [PUBLIC]
//
// This returns the top-level menu (file, edit, mailbox, etc.)
// Defined positions are in mainfrm.h as MAIN_MENU_*
//
HMENU CMainFrame::GetTopMenuByPosition(int nPos)
{
    BOOL bMaximized = FALSE;
    MDIGetActive(&bMaximized);
    const int uOffset = ( bMaximized ? 1 : 0 );

    ASSERT(GetMenu());

    return (GetSubMenu(*GetMenu(), nPos + uOffset ));
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
            //   "Picture Link..." (ID_EDIT_INSERT_PICTURE_LINK)
            //   "Horizontal Line" (ID_EDIT_INSERT_HR)
            ASSERT(m_hEditMenu);
            if (m_hEditMenu)
                EnableMenuItem(m_hEditMenu, POS_EDIT_INSERT_SUBMENU, MF_ENABLED | MF_BYPOSITION);

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
        //   "Picture Link..." (ID_EDIT_INSERT_PICTURE_LINK)
        //   "Horizontal Line" (ID_EDIT_INSERT_HR)
        //
        ASSERT(m_hEditMenu);
        if (m_hEditMenu)
            EnableMenuItem(m_hEditMenu, POS_EDIT_INSERT_SUBMENU, MF_GRAYED | MF_BYPOSITION);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        //
        // Nickname stuff -- only one nick file allowed in REDUCED FEATURE mode
        //
        ASSERT(g_Nicknames);
        if (g_Nicknames)
            g_Nicknames->UpdateSharewareFeatureMode();
    }

	// If not in light, remove the sponsor ad.  (If we are in light, it
	// will created when the first ad is received.)
	BOOL	bShowSponsorAd = (GetSharewareMode() != SWM_MODE_PRO);
	if (!bShowSponsorAd)
	{
		ResetSponsor();
	}

	// If not in adware, remove the ad toolbar.  (If we are in adware, it
	// will created when the first ad is received.)
	BOOL	bShowToolBarAd = (GetSharewareMode() != SWM_MODE_PRO);
	if (!bShowToolBarAd)
	{
		RemoveAdToolBar();
	}
}

void CMainFrame::OnTimeChange() 
{
	//CFrameWnd::OnTimeChange();
	QCWorkbook::OnTimeChange();

	// Usage Statistics need to be refreshed
	USRefreshStats();
	
}

void CMainFrame::OnUpdateStatistics(CCmdUI* pCmdUI) 
{
	// Pass it down to the SharewareManager & let it decide what to do
	OnUpdateFullFeatureSet(pCmdUI);

	if (UsingFullFeatureSet())
	{
		if(GetIniShort(IDS_INI_STATISTICS_DISABLE))
			pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnOpenEudora()
{
	if (IsIconic())
		ShowWindow(SW_RESTORE);
	SetForegroundWindow();
}

void CMainFrame::OnMinimizeToSystemTray()
{
	SetIniShort(IDS_INI_SHOW_ONLY_IN_TRAY, !GetIniShort(IDS_INI_SHOW_ONLY_IN_TRAY));
	SetIcon(-1, -1, TRUE);
}

void CMainFrame::OnUpdateMinimizeToSystemTray(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetIniShort(IDS_INI_SHOW_ONLY_IN_TRAY));
}

void CMainFrame::OnAlwaysShowTrayIcon()
{
	SetIniShort(IDS_INI_ALWAYS_SHOW_TRAY_ICON, !GetIniShort(IDS_INI_ALWAYS_SHOW_TRAY_ICON));
	SetIcon(-1, -1, TRUE);
}

void CMainFrame::OnUpdateAlwaysShowTrayIcon(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetIniShort(IDS_INI_ALWAYS_SHOW_TRAY_ICON));
}
