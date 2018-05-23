/*////////////////////////////////////////////////////////////////////////////

NAME:
	CMainFrame - 

FILE:		MainFrm.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	CMainStatusBar
	CMainToolBar
	CNameToolBar

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/??/96   lss     -Initial
04/28/97   lss     -Added File Name edit box (really a combox box) on its own
					tool bar just beneath the main tool bar. Also has ability
					to toggle this name toolbar
11/03/97   lss     -Post 'release mic' msg to IBM VR program

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "QVoice.h"
#include "QVoiceView.h"

// LOCAL INCLUDES
#include "MainFrm.h"

// LOCAL DEFINES
#define ID_FILEMAP_VOICENAME "FB177150BDDE11D0BA9100805FB4B97E"

#define WRITE_PROFILE_STAYONTOP(bSet)	\
					AfxGetApp()->WriteProfileInt( "Settings", "StayOnTop", bSet )
#define GET_PROFILE_STAYONTOP()			\
					AfxGetApp()->GetProfileInt( "Settings", "StayOnTop", 0 )

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_MESSAGE(WM_USER_FILENAME, OnUserFileName)
	ON_COMMAND(IDT_STAY_ONTOP, OnStayOntop)
	ON_UPDATE_COMMAND_UI(IDT_STAY_ONTOP, OnUpdateStayOntop)
	ON_COMMAND(IDT_SHOWNAME, OnShowname)
	ON_UPDATE_COMMAND_UI(IDT_SHOWNAME, OnUpdateShowname)
	ON_WM_CLOSE()
	ON_COMMAND(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
};

// for progress on the status bar
static UINT progress[] =
{
	ID_SEPARATOR,			// for setting up progress bar area
	ID_SEPARATOR,
};

//////////////////////////////////////////////////////////////////////////////
// CMainStatusBar implementation
//////////////////////////////////////////////////////////////////////////////
BOOL CMainStatusBar::Init(CWnd* pParentWnd)  // creates normal status bar
{
    if (!Create(pParentWnd) ||
       !SetIndicators(indicators,sizeof(indicators)/sizeof(UINT)))
     {
        TRACE0("Failed to create status bar\n");
        return FALSE;      // fail to create
     }
//    SetPaneStyle( 0, SBPS_STRETCH );
    return TRUE;
}

CProgressCtrl* CMainStatusBar::SetUpProgress() 
{                                              
// initializes status bar with space for progress bar
// returns pointer to a progress bar ctrl
                
    SetIndicators(progress,sizeof(progress)/sizeof(UINT));
    SetPaneStyle(1,SBPS_NOBORDERS);  // pane 0 used for message or tooltips
                                     // pane 1 used for progress bar
    RECT rt;
    CStatusBarCtrl* sb = &(GetStatusBarCtrl());

    sb->GetRect(1,&rt);     // get bounding rect for pane 1
    rt.top += 2;
    rt.bottom -= 2;
    m_wndProgress.Create(WS_CHILD|WS_VISIBLE, rt, sb, 0);

    Invalidate();
    UpdateWindow();
    return &m_wndProgress;
}

BOOL CMainStatusBar::RemoveProgress()   // displays normal status bar again
{
    m_wndProgress.DestroyWindow();
    SetIndicators(indicators,sizeof(indicators)/sizeof(UINT));
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CMainToolBar implementation
///////////////////////////////////////////////////////////////////////////////
BOOL CMainToolBar::Init(CWnd* pParentWnd) 
{
	if (!Create(pParentWnd) ||
		!LoadToolBar(IDR_MAINFRAME))
	 {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	 }

#ifdef LS_TOOLBAR_COMBOBOX
	SetButtonStyle( 2, TBBS_CHECKBOX );

	// Toolbar resource has a "dummy" button, followed by a separator, and then
	// the 1st "real" toolbar button.
	// We will take over the dummy button and replace it with acombo-box.
	// This technique will work for any control, placed in any position in the
	// toolbar.  Just visually place a "dummy" in the visual resource editor
	// where you want the control to go!  Will also work for multiple controls.
	// ID_TOOLBAR_COMBO has been defined as a string resource, so that VC++ handles the
	// unique value for us, and offers tooltip support for the control automatically!
	
	// Combo dimension constants
	const int nComboWidth = 160;
	const int nDropHeight = 100;
        
	// Turn the dummy button into a separator, of the width that we wish to make
	// our combo box.
	SetButtonInfo(4, IDC_CMB_CODEC, TBBS_SEPARATOR, nComboWidth);

	// Get the dimensions of our new toolbar real estate
	CRect rectComboPlacement;
	GetItemRect(4, &rectComboPlacement);
	// ".bottom" value actually represents bottom of drop-down list 
	// (not bottom of text box)
	rectComboPlacement.bottom = rectComboPlacement.top + nDropHeight;

	// Create the combo on the toolbar
	if (!m_cmbCodec.Create(CBS_DROPDOWNLIST |
		WS_VSCROLL | WS_VISIBLE | WS_TABSTOP,
		rectComboPlacement, this, IDC_CMB_CODEC))
	{
		TRACE0("Failed to create Toolbar Combo\n");
		return FALSE;
	}
	// Set the font for the Combo to the same as the dialog default
	// This is usually "MS Sans Serif" 8pt
	m_cmbCodec.SetFont(GetFont());
#endif
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CMainFrame Implementation
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CMainFrame::CMainFrame() : mb_StayOnTop(FALSE), m_hWndEudoraFileNameReq(NULL)
{
}

CMainFrame::~CMainFrame()
{
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

void CMainFrame::SendFileName2Eudora( CString& fileName, BOOL bAttachDesc )
{
	if ( NULL == m_hWndEudoraFileNameReq ) return;
#ifdef _DEBUG	
	AfxMessageBox(fileName);
#endif
	HANDLE hmmf = CreateFileMapping( HANDLE(0xFFFFFFFF), NULL, PAGE_READWRITE,
		0, _MAX_PATH, ID_FILEMAP_VOICENAME );

	LPTSTR sharedbuf = (LPTSTR)MapViewOfFile( hmmf, FILE_MAP_WRITE, 0, 0, 0 );
	if ( sharedbuf == NULL ) 
	 {
		CloseHandle( hmmf );
		return;
	 }
	ZeroMemory(sharedbuf, _MAX_PATH);
	lstrcpy( sharedbuf, fileName );
	if ( bAttachDesc )
		sharedbuf[ fileName.GetLength() + 1 ] = 1;
	UnmapViewOfFile( sharedbuf );

	TRACE1("WM_USER_EUDORA_FN sent. File Name = %s\n", fileName );
	// need to be SendMessage so we can stick around until
	// the Eudora EMSAPI plugin can do OpenFileMapping
	::SendMessage( m_hWndEudoraFileNameReq, WM_USER_EUDORA_FN, 0, 0 );
}

//////////////////////////////////////////////////////////////////////////////
// MESSAGE HANDLERS
///////////////////

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

#ifndef LS_PLAYER_ONLY
	if (m_wndToolBar.Init(this) == FALSE) return -1;
	if (m_wndNameBar.Init(this) == FALSE) return -1;
	if (m_wndStatusBar.Init(this) == FALSE) return -1;
#endif

#ifndef LS_PLAYER_ONLY
	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS );// | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndNameBar.SetBarStyle(m_wndNameBar.GetBarStyle() |
		CBRS_TOOLTIPS );// | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
#endif

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
	//DockControlBar(&m_wndToolBar);

	// get previous session's settings
	if ( GET_PROFILE_STAYONTOP() )
	 {
		// Stay on Top
		OnStayOntop();
	 }

	SetWindowText( AfxGetAppName() );
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if ( IsWindow(((CQVoiceApp*)AfxGetApp())->m_cmdLineInfo.m_hWndParent) )
	 {
		if ( ((CQVoiceApp*)AfxGetApp())->m_cmdLineInfo.mb_EudoraFileNameReq )
		 {
			m_hWndEudoraFileNameReq = 
					((CQVoiceApp*)AfxGetApp())->m_cmdLineInfo.m_hWndParent;
		 }
		else
		 {
			m_hWndEudoraFileNameReq = NULL;
		 }
	 }

	cs.hwndParent = m_hWndEudoraFileNameReq;
	//(FromHandle(m_hWndEudoraFileNameReq)->GetParent())->GetSafeHwnd();
	cs.style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUP;// | WS_OVERLAPPED;
	cs.dwExStyle |= WS_EX_CONTROLPARENT;
	cs.x  = 0;
	cs.y  = 0;
	cs.cx = 500;
	cs.cy = 500;

	BOOL ret = CFrameWnd::PreCreateWindow(cs);	

	return ret;
}

void CMainFrame::OnClose() 
{
	CWnd *pWnd = GetOwner();
	if ( m_hWndEudoraFileNameReq && 
		(m_hWndEudoraFileNameReq == pWnd->GetSafeHwnd()) )
		SetOwner(NULL);
/*	char tmp[256];
	sprintf( tmp, "Owner = %d, %d", cwnd->GetSafeHwnd(), m_hWndEudoraFileNameReq );
	if ( cwnd ) {AfxMessageBox(tmp); SetOwner(NULL);}
*/
	CFrameWnd::OnClose();
}

BOOL CMainFrame::DestroyWindow() 
{
	if ( !((CQVoiceApp*)AfxGetApp())->m_cmdLineInfo.mb_EudoraFileNameReq )
		((CQVoiceApp*)AfxGetApp())->m_singleInstance.RemoveProperty( 1 );
	// save our stay on top state
	WRITE_PROFILE_STAYONTOP( mb_StayOnTop ? 1 : 0 );
	{
		// 11.3.97 : Release mic to IBM VR program
		HWND hwndMain = ::FindWindow("VTEudora_InterfaceWindow", NULL);
		if (hwndMain!=NULL)
		 {
			// Bring main window to focus.
			//::BringWindowToTop(hwndMain);
			::PostMessage(hwndMain, WM_USER + 301, 0, 0);
		 }
	}
	return CFrameWnd::DestroyWindow();
}

LRESULT CMainFrame::OnUserFileName(WPARAM wParam, LPARAM lParam)
{
	return ((CQVoiceView*)GetActiveView())->OnUserFileName( wParam, lParam );
}

void CMainFrame::OnStayOntop() 
{
	mb_StayOnTop = !mb_StayOnTop;
	if ( !mb_StayOnTop )
	 {
		SetWindowPos( &wndNoTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
	 }
	else
	 {
		SetWindowPos( &wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
	 }
}

void CMainFrame::OnUpdateStayOntop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( mb_StayOnTop ? 1 : 0 );
}

void CMainFrame::OnShowname() 
{
	m_wndNameBar.Show( !m_wndNameBar.IsWindowVisible() );	
}

void CMainFrame::OnUpdateShowname(CCmdUI* pCmdUI) 
{
//	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck( m_wndNameBar.IsWindowVisible() ? 1 : 0 );	
}

void CMainFrame::OnHelp() 
{
	WinHelp( 0, HELP_FINDER );	// no context help yet
}

/////////////////////////////////////////////////////////////////////////////
// DEBUG
////////

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG
