// DummyDlg.cpp : implementation file
//

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "qvoice32.h"

// LOCAL INCLUDES
#include "DummyDlg.h"

// LOCAL DEFINES
#define	WM_USER_EUDORA_FN		(WM_USER + 7)
#define ID_FILEMAP_VOICENAME	"FB177150BDDE11D0BA9100805FB4B97E"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CDummyDlg dialog

CDummyDlg::CDummyDlg(
	CString& exeName,
	CString& cmdLine,
	CWnd* pParent /*=NULL*/ 
)
	: CDialog(CDummyDlg::IDD, pParent), m_exeName(exeName),
	m_cmdLine(cmdLine), m_FileName(""), mb_AttachReadMe(FALSE)
{
	//{{AFX_DATA_INIT(CDummyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CDummyDlg, CDialog)
	//{{AFX_MSG_MAP(CDummyDlg)
	ON_MESSAGE(WM_USER_EUDORA_FN, OnGetFileName) 	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDummyDlg message handlers

/////////////////////////////////////////////////////////////////////////////
// lsong
// Using this dummy modal dialog to launch our QVRecord

BOOL CDummyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	STARTUPINFO si;				// pointer to STARTUPINFO 
	PROCESS_INFORMATION pi; 	// pointer to PROCESS_INFORMATION  

	memset( &si, 0, sizeof(si) );
	si.cb = sizeof(si);

	if ( ((CQVoice32DLL*) AfxGetApp())->mb_ImmediateRecord )
	 {
		m_cmdLine.Format( LPCTSTR(m_cmdLine + CString(" /record /hwnd %d")),
	 												GetSafeHwnd() );
	 }
	else
	 {
		m_cmdLine.Format( LPCTSTR(m_cmdLine + CString(" /hwnd %d")), GetSafeHwnd() );
	 }

	TRACE( "m_exeName = %s, m_cmdLine = %s", m_exeName, m_cmdLine );

	CString tmpString;
	tmpString.Format( "\"%s\" %s", (LPCTSTR)m_exeName, (LPCTSTR)m_cmdLine );
	TRACE( tmpString );

	// CreateProcess can modify this param, so let's make sure there's room
	LPTSTR cpCmdLine = tmpString.GetBuffer( _MAX_PATH );

	BOOL ret = ::CreateProcess(
		NULL,					// pointer to name of executable module 
		cpCmdLine,              // pointer to command line string
		NULL,					// pointer to process security attributes 
		NULL,					// pointer to thread security attributes 
		FALSE,					// handle inheritance flag 
		0,						// creation flags 
		NULL,					// pointer to new environment block 
		NULL,					// pointer to current directory name 
		&si,					// pointer to STARTUPINFO 
		&pi 					// pointer to PROCESS_INFORMATION  
	   );

	tmpString.ReleaseBuffer();

	if ( ret )
	 {
		// Ok now this really is a kludge. I'm showing 0 by 0 dialog window
		// so we can stop Windows from showing other windows to foreground
		SetWindowPos(0,0,0,0,0,0);
		ShowWindow(SW_SHOW);

		// wait for our recorder process to finish
		// but do idle wait
		while ( WAIT_OBJECT_0 != ::WaitForSingleObject( pi.hProcess, 0 ) )
		{
			((CQVoice32DLL*) AfxGetApp())->YieldEvents();
			Sleep(5);
		}
	 }
	else
	 {
		LPVOID lpMsgBuf;
		DWORD err = GetLastError();
		if ( err == 123 )
		 {
			char buf[512];
			sprintf( buf, "Cannot find %s.", m_exeName );
			AfxMessageBox( buf );
		 }
		else
		{
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			// Display the string.
			AfxMessageBox( (LPTSTR)lpMsgBuf );
			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
	 }
	EndDialog( IDOK );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CDummyDlg::OnGetFileName(WPARAM wParam, LPARAM lParam)
{
	char fileName[_MAX_PATH];

	HANDLE hmmf = OpenFileMapping(FILE_MAP_READ, FALSE, ID_FILEMAP_VOICENAME);
	if ( hmmf == NULL ) return FALSE;
	LPCTSTR sharedbuf = (LPCTSTR)MapViewOfFile( hmmf, FILE_MAP_READ, 0, 0, 0 );
	if ( sharedbuf == NULL )
	 {
		m_FileName.Empty();
		CloseHandle( hmmf );
		return FALSE;
	 }
	lstrcpy( fileName, sharedbuf );
	// if the next byte after null is 1 then we should do attach instruction
	if ( sharedbuf[lstrlen(fileName)+1] == 1 )
		mb_AttachReadMe = TRUE;	
	UnmapViewOfFile( sharedbuf );
	CloseHandle( hmmf );

	m_FileName = LPCTSTR(fileName);

	return 0L;
}

