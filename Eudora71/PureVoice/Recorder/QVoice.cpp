// QVoice.cpp : Defines the class behaviors for the application.
//

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "QVoice.h"
#include "MainFrm.h"
#include "QVoiceDoc.h"
#include "QVoiceView.h"
#include "TimeExpire.cpp"
#ifdef LS_DEBUG0
	#include "LsDebug.h"
#endif
//#ifndef _DEBUG
#include <winreg.h>
//#endif

// LOCAL DEFINES
#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CQVoiceApp

BEGIN_MESSAGE_MAP(CQVoiceApp, CWinApp)
	//{{AFX_MSG_MAP(CQVoiceApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQVoiceApp construction

CQVoiceApp::CQVoiceApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CQVoiceApp object

CQVoiceApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CQVoiceApp initialization

BOOL CQVoiceApp::InitInstance()
{
	TRACE0("Entering QVoiceApp::InitInstance()\n");
#ifndef LS_RELEASE
//	TimeExpire expire;
//	if ( !expire.IsValidDate() )
//		return FALSE;
#endif
	OSVERSIONINFO vInfo; vInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &vInfo ); 
	m_OsPlatform = vInfo.dwPlatformId;

//	AfxMessageBox( GetCommandLine() );

	// setup our section under Qualcomm in the registery
	SetRegistryKey( "Qualcomm" );
	ParseCommandLine( m_cmdLineInfo );

	if ( m_singleInstance.CheckInstance( APP_ID_STR, m_cmdLineInfo ) )
	 {	
		TRACE0("Instance of this program already exist so I'm quitting\n");
        return FALSE;
	 }

	// 11.3.97 : Get mic from IBM VR program
	HWND hwndMain = ::FindWindow("VTEudora_InterfaceWindow", NULL);
	if (hwndMain!=NULL)
	 {
		// Bring main window to focus.
		::BringWindowToTop(hwndMain);
		::PostMessage(hwndMain, WM_USER + 300, 0, 0);
	 }

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	//_CrtSetBreakAlloc(143);
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

//	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	mp_DocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CQVoiceDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CQVoiceView));
	AddDocTemplate(mp_DocTemplate);

//#ifdef LS_PLAYER_ONLY
//#ifndef _DEBUG
	EnableShellOpen();
	RegisterShellFileTypes();
	// setup DefaultIcon in registery since MFC won't do it
    // without setting up print and print to
	CString strFileTypeId;
	if (mp_DocTemplate->GetDocString(strFileTypeId,
		   CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
	 {
		CString strPathName, strTemp;
		TCHAR szBuff[_MAX_PATH];
		VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), szBuff, _MAX_PATH));
		GetShortPathName( szBuff, szBuff, _MAX_PATH );
		strTemp = strFileTypeId + CString(_T("\\DefaultIcon"));
		strPathName = szBuff;
#ifdef LS_PLAYER_ONLY
		strPathName += _T(",0");
#else
		strPathName += _T(",1");
#endif
		::RegSetValue(HKEY_CLASSES_ROOT, LPCTSTR(strTemp), REG_SZ,
					  LPCTSTR(strPathName), strPathName.GetLength() );
	 }
//#endif
//#endif

	if ( m_cmdLineInfo.mb_RegisterOnly )
	 return FALSE;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(m_cmdLineInfo))
		return FALSE;

	// enable file manager drag/drop and DDE Execute open
	m_pMainWnd->DragAcceptFiles();

	// if not called from eudora emsapi then do this for limiting # of this
	// instance
	if ( !m_cmdLineInfo.mb_EudoraFileNameReq )
		m_singleInstance.SetProperty( 1 );	

	TRACE0("Exiting QVoiceApp::InitInstance()\n");	

	return TRUE;
}

void CQVoiceApp::YieldEvents()
{
	MSG msg;

	if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) // check for messages
	 { 
		if ( !PumpMessage() ) // if FALSE app has terminated
		 { 
			//m_bProcessActive = FALSE;
			::PostQuitMessage(0); 
		 } 
	 } 
	// let MFC do its idle processing
	LONG lIdle = 0;
	while ( OnIdle(lIdle++ ));
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();


// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
//	CButton	m_btnHomeUrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEudoraUrl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// App command to run the dialog
void CQVoiceApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
//	DDX_Control(pDX, IDC_HOME_URL, m_btnHomeUrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_RBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_EUDORA_URL, OnEudoraUrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAboutDlg::OnEudoraUrl() 
{
	TimeExpire expire;
	expire.GotoOurWebPage();
}

void CAboutDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	if ( (nFlags & MK_CONTROL) && (nFlags & MK_LBUTTON) && 
		 (nFlags & MK_SHIFT) )
	 {
		extern void LsAbout();
		EnableWindow( FALSE );
		LsAbout();
		EnableWindow();
	 }
	CDialog::OnRButtonDblClk(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////////

CSingleInstance::CSingleInstance() : m_nID(0)
{
}

CSingleInstance::~CSingleInstance()
{
}

BOOL CSingleInstance::CheckInstance(
	LPCTSTR lpszId,
	CCommandLineInfo& cmdLineInfo )
{
	// if called from eudora emsapi, then no instance limit
	if ( ((CLsCommandLineInfo*)&cmdLineInfo)->mb_EudoraFileNameReq )
		return FALSE;

	// closes handle automatically when process terminates
	::CreateMutex(NULL, TRUE, lpszId);

	if( ::GetLastError() == ERROR_ALREADY_EXISTS )
	 {
		TRACE("Instance already exists\n");

		CWnd* pPrevWnd = CWnd::GetDesktopWindow()->GetWindow(GW_CHILD);
        
		//loop through all child windows on the desktop
		while( pPrevWnd )
		 {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), lpszId ) )
			 {
				// pass along the filename from commandline
				if ( cmdLineInfo.m_nShellCommand 
					== CCommandLineInfo::FileOpen )
				 {
					SendFileName( pPrevWnd, cmdLineInfo.m_strFileName );
				 }

				if( pPrevWnd->IsIconic() )
				 {
					pPrevWnd->ShowWindow(SW_RESTORE);
				 }
				pPrevWnd->SetForegroundWindow();
				// GetLastActivePopup will return a pointer equal to pPrevWnd
				// so it is safe to call SetForegroundWindow with the pointer
				// that is returned.  
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return TRUE;
			 }
			//move to the next window
			pPrevWnd = pPrevWnd->GetWindow(GW_HWNDNEXT);
		 }
		TRACE("Couldn't Find Previous Instance Window!!\n");
		return TRUE;
	 }
	return FALSE;
}

BOOL CSingleInstance::SetProperty( UINT nID )
{
	m_nID = nID;
	return ::SetProp( AfxGetMainWnd()->GetSafeHwnd(), 
			APP_ID_STR, (HANDLE)m_nID );
}

BOOL CSingleInstance::RemoveProperty( UINT nID ) 
{
	int nRetCode = ::EnumPropsEx( AfxGetMainWnd()->GetSafeHwnd(),
									  PropEnumFuncEx, (LPARAM)nID );
	if( nRetCode == -1 )
	 {
		TRACE0("No Prop Found to remove\n");
		return FALSE;
	 }
	return TRUE;
}

void CSingleInstance::SendFileName( CWnd *wnd, CString& fileName )
{
/*
	// use anonymous pipe to send our filename to already living instance
	// of this program
	HANDLE hReadPipe, hWritePipe;
	DWORD bytesWritten;

	// create anonymous pipe
	::CreatePipe( &hReadPipe, &hWritePipe, NULL, _MAX_PATH );
	::WriteFile( hWritePipe, LPCTSTR(fileName), fileName.GetLength() + 1,
		&bytesWritten, NULL );
	::CloseHandle( hWritePipe );

	// send the handle of read end of anonymous pipe
	wnd->SendMessage( WM_USER_FILENAME, 0, LPARAM(hReadPipe) );
*/
	HANDLE hmmf = CreateFileMapping( HANDLE(0xFFFFFFFF), NULL, PAGE_READWRITE,
		0, _MAX_PATH, 
		LPCTSTR(CString(APP_ID_STR) + CString("filename")));
	if ( hmmf == NULL )
	 {
#ifdef LS_DEBUG0
		LsDebug err;
		err.SystemErrorMsgBox("TX: CreateFileMapping Failed");
#endif
		return;
	 }

	LPTSTR sharedbuf = (LPTSTR)MapViewOfFile( hmmf, FILE_MAP_WRITE, 0, 0, 0 );
	if ( sharedbuf == NULL ) 
	 {
#ifdef LS_DEBUG0
		LsDebug err;
		err.SystemErrorMsgBox("TX: MapViewOfFile Failed");
#endif
		CloseHandle( hmmf );
		return;
	 }
	lstrcpy( sharedbuf, LPCTSTR(fileName) );
	UnmapViewOfFile( sharedbuf );

	TRACE1("WM_USER_FILENAME sent. File Name = %s\n", fileName );
	// need to be SendMessage so we can stick around until
	// the first process can do OpenFileMapping
	wnd->SendMessage( WM_USER_FILENAME );
}

BOOL CALLBACK CSingleInstance::PropEnumFuncEx(
	HWND hWnd, 
	LPTSTR lpszName,
	HANDLE hData,
	DWORD dwData )
{
	if( CString(lpszName) == CString(APP_ID_STR) )
	 {
		if( UINT(hData) == UINT(dwData) )
		 {
			::RemoveProp( hWnd, lpszName );
			TRACE0("Prop removed\n");
			return FALSE;
		 }
	 }
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

void CLsCommandLineInfo::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
	if ( bFlag )
	 {
		if ( !strcmp(lpszParam, "o") )
		 {
			mb_OutFileName = TRUE;
			return;
		 }
		if ( !strcmp(lpszParam, "dll") )
		 {
			mb_DllPathName = TRUE;
			return;	
		 }
		if ( !strcmp(lpszParam, "nomapi") )
		 {
			mb_UseMapi = FALSE;
			return;	
		 }
		if ( !strcmp(lpszParam, "hwnd") )
		 {
			mb_hWndParent = TRUE;
			return;	
		 }
		if ( !strcmp(lpszParam, "record") )
		 {
			mb_Record = TRUE;
			return;	
		 }
		if ( !strcmp(lpszParam, "reg") )
		 {
			mb_RegisterOnly = TRUE;
			return;	
		 }
		if ( !strcmp(lpszParam, "eudora") )
		 {
			mb_EudoraFileNameReq = TRUE;
			return;	
		 }
	 }

	if ( mb_OutFileName )
	 {
		m_strOutFileName = LPCTSTR(lpszParam);
		int ch = lpszParam[m_strOutFileName.GetLength()-1];
		if ( (ch == '\\') || (ch == '/') )
		 {
			m_strOutPathName = lpszParam;
			m_strOutFileName.Empty();
		 }

		mb_OutFileName = FALSE;
		return;
	 }

	if ( mb_DllPathName )
	 {
		m_strDllPathName = lpszParam;
		mb_DllPathName = FALSE;
		return;
	 }

	if ( mb_hWndParent )
	 {
		char *junk;

		m_hWndParent  = (HWND)strtol(lpszParam, &junk, 10);
		mb_hWndParent = FALSE;
		return;
	 }

	CCommandLineInfo::ParseParam( lpszParam, bFlag, bLast );
}
