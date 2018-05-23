// ExceptionHandler.cpp
//
// Any unhandled exceptions in the rest of the application get caught here.
// A log file is written out that contains the values of all the registers
// and the stack crawl.
//


#include <stdafx.h>

#include "ExceptionHandler.h"
#include "resource.h"
#include "rs.h"
#include "guiutils.h"
#include "fileutil.h"
#include "QCUtils.h"
#include "..\Version.h"


#include "DebugNewHelpers.h"

// Constants
#define BUFF_SIZE 2048


//============================== Global Variables =============================

//
// Declare the static variables of the QCExceptionHandler class
//
TCHAR QCExceptionHandler::m_szExceptionLogFileName[MAX_PATH];
TCHAR QCExceptionHandler::m_szMiniDumpLogFileName[MAX_PATH];
HANDLE QCExceptionHandler::m_hReportFile = NULL;
MINIDUMP_TYPE QCExceptionHandler::m_eMiniDumpType = MiniDumpNormal;
bool QCExceptionHandler::m_bEudoraCrashedDuringThisRun = false;
bool QCExceptionHandler::m_bEudoraCrashedDuringLastRun = false;
bool QCExceptionHandler::m_bBufferOverflowCheck = true;
bool QCExceptionHandler::m_bBufferOverflowAskUserBeforeQuitting = true;

QCExceptionHandler g_QCExceptionHandler(false);	// Declare global instance of class


//============================== Class Methods =============================


// ------------------------------------------------------------------------------------------
//		* QCExceptionHandler														[Public]
// ------------------------------------------------------------------------------------------
//	QCExceptionHandler constructor

QCExceptionHandler::QCExceptionHandler(bool bEnableCrashHandler /*= true*/)
{
	if (bEnableCrashHandler)
		EnableCrashHandler();
}


// ------------------------------------------------------------------------------------------
//		* ~QCExceptionHandler														[Public]
// ------------------------------------------------------------------------------------------
//	QCExceptionHandler destructor

QCExceptionHandler::~QCExceptionHandler( )
{
	DisableCrashHandler();
}


// ------------------------------------------------------------------------------------------
//		* EnableCrashHandler														[Public]
// ------------------------------------------------------------------------------------------
//	Call to enable crash handling

void
QCExceptionHandler::EnableCrashHandler()
{
	SetCrashHandlerFilter(QCCrashHandler);
}


// ------------------------------------------------------------------------------------------
//		* DisableCrashHandler														[Public]
// ------------------------------------------------------------------------------------------
//	Call to disable crash handling

void
QCExceptionHandler::DisableCrashHandler()
{
	SetCrashHandlerFilter(NULL);
}


// ------------------------------------------------------------------------------------------
//		* EnableBufferOverflowHandler												[Public]
// ------------------------------------------------------------------------------------------
//	Call to enable buffer overflow handling. Without this the default behavior
//	of buffer overflow handling is to exit with a generic dialog.

void
QCExceptionHandler::EnableBufferOverflowHandler()
{
	_qc_set_security_error_handler(SecurityErrorHandler);

	short	nBufferOverflowCheckSetting = GetIniShort(IDS_INI_BUFFER_OVERFLOW_CHECK);

	m_bBufferOverflowCheck = (nBufferOverflowCheckSetting != 0);
	m_bBufferOverflowAskUserBeforeQuitting = (nBufferOverflowCheckSetting == 2);
}
	

// ------------------------------------------------------------------------------------------
//		* SetExceptionLogFileName													[Public]
// ------------------------------------------------------------------------------------------
//	Sets name and location of the exception log to be generated

void
QCExceptionHandler::SetExceptionLogFileName(
	const char *			szLogFileName)
{
	_tcscpy(m_szExceptionLogFileName, szLogFileName);
}


// ------------------------------------------------------------------------------------------
//		* SetMiniDumpFileName														[Public]
// ------------------------------------------------------------------------------------------
//	Sets name and location of the mini dump crash info to be generated.
//	Also determines whether or not we crashed when we were last running.

void
QCExceptionHandler::SetMiniDumpFileName(
	const char *			szLogFileName)
{
	_tcscpy(m_szMiniDumpLogFileName, szLogFileName);

	m_eMiniDumpType = static_cast<MINIDUMP_TYPE>( GetIniLong(IDS_INI_CRASH_DUMP_TYPE) );

	// Now try to detect whether or not we crashed the last time
	// we were running.

	// Get the information for the last known "EudoraCrashDump.dmp"
	CString		szLastKnownCrashInfo;

	// Get the information for the last know "EudoraCrashDump.dmp"
	GetIniString(IDS_INI_LAST_KNOWN_CRASH_INFO, szLastKnownCrashInfo);

	// Now default to indicating that we have not quit successfully (as indicated
	// by a "0"). Below we'll tack on the current date info for the existing
	// "EudoraCrashDump.dmp", if any.
	SetIniString(IDS_INI_LAST_KNOWN_CRASH_INFO, "0");

	// Do we have an existing "EudoraCrashDump.dmp"?
	if ( FileExistsMT(m_szMiniDumpLogFileName) )
	{
		// Access "EudoraCrashDump.dmp", if we can
		HANDLE		hCrashDumpFile = CreateFile(
										m_szMiniDumpLogFileName, FILE_READ_ATTRIBUTES, 0, 0,
										OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
		if (hCrashDumpFile)
		{
			// Get the last time "EudoraCrashDump.dmp" was written. 
			FILETIME	ftWrite;

			if ( GetFileTime(hCrashDumpFile, NULL, NULL, &ftWrite) )
			{
				CString		szCrashInfo;
				
				// Crash info string format is:
				// %d 0x%08X 0x%08X
				// First number is 0 or 1. 1 means successful quit without crash, 0 means crash may have occurred.
				// Second two numbers encode last write time info for "EudoraCrashDump.dmp"
				szCrashInfo.Format("0x%08X 0x%08X", ftWrite.dwHighDateTime, ftWrite.dwLowDateTime);

				// If the last known crash info is empty or doesn't start with a 0, then
				// we didn't crash.
				if ( !szLastKnownCrashInfo.IsEmpty() && (szLastKnownCrashInfo[0] == '0') )
				{
					if ( (szLastKnownCrashInfo.GetLength() < (szCrashInfo.GetLength() + 2)) ||
						 (szLastKnownCrashInfo[1] != ' ') )
					{
						// Invalid or missing crash dump file info - assume crash during last run
						// (Missing crash dump file info from previous runs isn't surprising - it
						// may be the first crash on this machine - that's a *good* thing).
						m_bEudoraCrashedDuringLastRun = true;
					}
					else
					{
						// Chop off leading "0 "
						szLastKnownCrashInfo = szLastKnownCrashInfo.Right(szLastKnownCrashInfo.GetLength() - 2);
						
						// We crashed during the last run if the modification time for "EudoraCrashDump.dmp"
						// has changed. Do comparison using current crash info length to allow for additional
						// data in future crash info strings.
						m_bEudoraCrashedDuringLastRun =
								( strnicmp( szCrashInfo, szLastKnownCrashInfo, szCrashInfo.GetLength() ) != 0 );
					}
				}

				// Remember the latest crash dump information. We start the string with "0 "
				// for now to indicate that we're running, but haven't quit successfully yet.
				szCrashInfo.Insert(0, "0 ");
				SetIniString(IDS_INI_LAST_KNOWN_CRASH_INFO, szCrashInfo);
			}
			
			// Close the file
			CloseHandle(hCrashDumpFile);
		}

		// If we crashed when we were last running, then backup the "EudoraCrashDump.dmp"
		// file so that if we crash again the information won't be lost.
		if ( m_bEudoraCrashedDuringLastRun && (GetIniLong(IDS_INI_CRASH_DUMP_NUM_COPIES) > 0) )
			CascadeBackupFile( m_szMiniDumpLogFileName, GetIniLong(IDS_INI_CRASH_DUMP_NUM_COPIES) );
	}
}


// ------------------------------------------------------------------------------------------
//		* SaveCrashStateToINI														[Public]
// ------------------------------------------------------------------------------------------
//	Called by CMainFrame::CloseDown while Eudora is quitting.

void
QCExceptionHandler::SaveCrashStateToINI() const
{
	// We already wrote out the crash string such that it indicates
	// that we *did* crash (in SetMiniDumpFileName above). If we
	// got this far without crashing, then remember that.
	if (!m_bEudoraCrashedDuringThisRun)
	{
		CString		szCrashInfo;

		GetIniString(IDS_INI_LAST_KNOWN_CRASH_INFO, szCrashInfo);

		ASSERT( !szCrashInfo.IsEmpty() && (szCrashInfo[0] == '0') );

		// Change leading 0 in string to 1 to indicate that we quit successfully
		if ( !szCrashInfo.IsEmpty() && (szCrashInfo[0] == '0') )
		{
			szCrashInfo.SetAt(0, '1');

			SetIniString(IDS_INI_LAST_KNOWN_CRASH_INFO, szCrashInfo);
		}
	}
}


// ------------------------------------------------------------------------------------------
//		* QCCrashHandler														 [Protected]
// ------------------------------------------------------------------------------------------
//	Called by EuMemMgr when an unhandled exception occurs.

LONG __stdcall
QCExceptionHandler::QCCrashHandler(
	PEXCEPTION_POINTERS			pExceptionInfo)
{
	// Remember that we crashed during this run
	m_bEudoraCrashedDuringThisRun = true;
	
	// Create/open the exception log file
	m_hReportFile = CreateFile( m_szExceptionLogFileName,
								GENERIC_WRITE,
								0,
								0,
								OPEN_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								0 );

	HWND hwndDialog = NULL;
	if ( m_hReportFile )
	{	
		// Display a dialog so that the user knows what's going on
		hwndDialog = CreateDialog( AfxFindResourceHandle(MAKEINTRESOURCE(IDD_EXCEPTION), RT_DIALOG),
								   MAKEINTRESOURCE(IDD_EXCEPTION), NULL, NULL );
		if (hwndDialog)
		{
			CWnd::FromHandle(hwndDialog)->CenterWindow();
			EscapePressed();	// Use this to allow the message queue to be processed
		}

		// Tack new exception info onto the end
		SetFilePointer( m_hReportFile, 0, 0, FILE_END );

		// Generate the exception report
		GenerateExceptionReport( pExceptionInfo );

		// Close the exception report
		CloseHandle( m_hReportFile );
		m_hReportFile = NULL;
	}

	// Create a mini dump of the current crash. Unfortunately this will blast over
	// any previous mini dump. Then again we're now trying to recognize a crash
	// on launch so that we can email the latest crash info immediately.
	CreateCurrentProcessCrashDump( m_eMiniDumpType, m_szMiniDumpLogFileName,
                                   GetCurrentThreadId(), pExceptionInfo );

	// We're done writing out crash info.
	// Destroy the dialog.
	if (hwndDialog)
		DestroyWindow(hwndDialog);

	// Let Eudora finish crashing now.
	return EXCEPTION_CONTINUE_SEARCH;
}


// ------------------------------------------------------------------------------------------
//		* GenerateExceptionReport												 [Protected]
// ------------------------------------------------------------------------------------------
//	Writes information to exception log file.
//	Called by QCCrashHandler.

void
QCExceptionHandler::GenerateExceptionReport(
	PEXCEPTION_POINTERS			pExceptionInfo)
{
	// Start out with a banner
	_tprintf( _T("//=====================================================\r\n") );

	// Time and version info
	time_t Now = time(NULL);
	char* TimeStr = ctime(&Now);
	if (TimeStr)
	{
		// ctime() outputs a string with only a \n at the end, so we get rid of it
		TimeStr[24] = 0;
		_tprintf( _T("%s\r\n"), TimeStr);
	}
	_tprintf( _T("%s\r\n\r\n"), EUDORA_BUILD_VERSION);

	DWORD			dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
	const TCHAR *	szBuff = NULL;

	szBuff = GetFaultReason(pExceptionInfo);
	_tprintf ( _T("%ls\r\n\r\n") , szBuff ) ;

	szBuff = GetRegisterString(pExceptionInfo);
	_tprintf ( _T("Registers:\r\n%ls\r\n\r\n") , szBuff );

	_tprintf( _T("Call stack (CrashFinder):\r\n") );
	_tprintf( _T("Address\r\n") );
	szBuff = GetFirstStackTraceString(0 , pExceptionInfo);
	do
	{
		_tprintf ( _T("%ls\r\n") , szBuff );
		szBuff = GetNextStackTraceString(0 , pExceptionInfo);
	}
	while (NULL != szBuff);

	_tprintf( _T("\r\nCall stack (Extended Info):\r\n") );
	_tprintf( _T("Address       Possible Params                               Location\r\n") );
	szBuff = GetFirstStackTraceString(dwOpts , pExceptionInfo);
	do
	{
		_tprintf ( _T("%ls\r\n") , szBuff );
		szBuff = GetNextStackTraceString(dwOpts , pExceptionInfo);
	}
	while (NULL != szBuff) ;

	_tprintf( _T("\r\n") );
}


// ------------------------------------------------------------------------------------------
//		* SecurityErrorHandler													 [Protected]
// ------------------------------------------------------------------------------------------
//	Eudora's customized handling for when a /GS security error has been detected.

void
QCExceptionHandler::SecurityErrorHandler()
{
	// Re-entrancy protection
	static bool		s_bAlreadyHandlingBufferOverflow = false;
	static bool		s_bFirstBufferOverflow = true;
	
	// Are we doing buffer overflow checking?
	// If yes, then we'll show our dialog, write out our information, and exit Eudora.
	// If no, then we'll just return and hope for the best.
	if (m_bBufferOverflowCheck && !s_bAlreadyHandlingBufferOverflow)
	{
		// Begin preventing re-entrancy
		s_bAlreadyHandlingBufferOverflow = true;
		
		// Display a dialog so that the user knows what's going on
		HWND hwndDialog = CreateDialog( AfxFindResourceHandle(MAKEINTRESOURCE(IDD_BUFFER_OVERFLOW), RT_DIALOG),
										MAKEINTRESOURCE(IDD_BUFFER_OVERFLOW), NULL, NULL );
		if (hwndDialog)
		{
			CWnd::FromHandle(hwndDialog)->CenterWindow();
			EscapePressed();	// Use this to allow the message queue to be processed
		}

		// Create/open the exception log file
		m_hReportFile = CreateFile( m_szExceptionLogFileName,
									GENERIC_WRITE,
									0,
									0,
									OPEN_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									0 );
		if ( m_hReportFile )
		{	
			// Tack new exception info onto the end
			SetFilePointer( m_hReportFile, 0, 0, FILE_END );

			// Generate the exception report
			GenerateBufferOverflowReport();

			// Close the exception report
			CloseHandle( m_hReportFile );
			m_hReportFile = NULL;
		}

		// Create a mini dump of the current crash. Unfortunately this will blast over
		// any previous mini dump. Then again we're now trying to recognize a crash
		// on launch so that we can email the latest crash info immediately.
		SnapCurrentProcessMiniDump(m_eMiniDumpType, m_szMiniDumpLogFileName);

		int		nUserChoice = IDC_WARN_BUTTON2;

		if (m_bBufferOverflowAskUserBeforeQuitting)
		{			
			if (hwndDialog)
			{
				// Close the original dialog before asking the user what to do
				DestroyWindow(hwndDialog);
				hwndDialog = NULL;
			}
			
			nUserChoice = WarnOneOptionCancelDialogWithDefButton(
									0, IDS_BUFFER_OVERFLOW,
									IDS_QUIT_EUDORA_BUTTON, IDCANCEL );
		}

		if (nUserChoice == IDCANCEL)
		{
			// User didn't want to quit. If this is the first buffer overflow, then
			// backup the EudoraCrashDump.dmp file because it's somewhat likely that
			// user is about to crash and we don't want to lose the buffer overflow
			// stack crawl. We don't bother on subsequent buffer overflows because
			// we don't want to overwrite all backed up crash dumps.
			if (s_bFirstBufferOverflow)
				CascadeBackupFile( m_szMiniDumpLogFileName, GetIniLong(IDS_INI_CRASH_DUMP_NUM_COPIES) );
		}
		else
		{
			// User has chosen quit or the setting said to quit without asking.
			// Quit now in the same manner used by the original /GS support.
			_exit(3);
		}

		// Done prevent re-entrancy
		s_bAlreadyHandlingBufferOverflow = false;

		// Remember not to cascade backup the file any more
		s_bFirstBufferOverflow = false;
	}
}


// ------------------------------------------------------------------------------------------
//		* GenerateBufferOverflowReport											 [Protected]
// ------------------------------------------------------------------------------------------
//	Writes information to exception log file.
//	Called by SecurityErrorHandler.

void
QCExceptionHandler::GenerateBufferOverflowReport()
{
	// Start out with a banner
	_tprintf( _T("//=====================================================\r\n") );

	// Time and version info
	time_t Now = time(NULL);
	char* TimeStr = ctime(&Now);
	if (TimeStr)
	{
		// ctime() outputs a string with only a \n at the end, so we get rid of it
		TimeStr[24] = 0;
		_tprintf( _T("%s\r\n"), TimeStr);
	}
	_tprintf( _T("%s\r\n\r\n"), EUDORA_BUILD_VERSION);

	_tprintf( _T("Buffer overflow detected.\r\n") );
	_tprintf( _T("See %s for more details.\r\n\r\n"), m_szMiniDumpLogFileName);
}


// ------------------------------------------------------------------------------------------
//		* _tprintf																 [Protected]
// ------------------------------------------------------------------------------------------
//	Helper function that writes to the exception log file and allows us to use printf
//	style formating.

int __cdecl
QCExceptionHandler::_tprintf(const TCHAR * format, ...)
{
	static TCHAR szBuff[BUFF_SIZE];
	int retValue;
	DWORD cbWritten;
	va_list argptr;
		  
	va_start( argptr, format );
	retValue = wvsprintf( szBuff, format, argptr );
	va_end( argptr );

	WriteFile( m_hReportFile, szBuff, retValue * sizeof(TCHAR), &cbWritten, 0 );

	return retValue;
}
