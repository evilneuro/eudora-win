// ExceptionHandler.h
//

#ifndef __EXCEPTION_HANDLER_H__
#define __EXCEPTION_HANDLER_H__

#include <imagehlp.h>

class QCExceptionHandler
{
  public:
								QCExceptionHandler(bool bEnableCrashHandler = true);
								~QCExceptionHandler( );

	void						EnableCrashHandler();
	void						DisableCrashHandler();
	void						EnableBufferOverflowHandler();
	
	void						SetExceptionLogFileName(
										const char *			szLogFileName);

	void						SetMiniDumpFileName(
										const char *			szMiniDumpFileName);

	void						SaveCrashStateToINI() const;

  protected:
	// entry point where control comes on an unhandled exception
	static LONG __stdcall		QCCrashHandler(
										PEXCEPTION_POINTERS		pExceptionInfo);

	// where report info is extracted and generated	
	static void					GenerateExceptionReport(
										PEXCEPTION_POINTERS		pExceptionInfo);
	static void					SecurityErrorHandler();
	static void					GenerateBufferOverflowReport();

	// Helper functions
	static int __cdecl			_tprintf(const TCHAR * format, ...);
		
	// Variables used by the class
	static TCHAR				m_szExceptionLogFileName[MAX_PATH];
	static TCHAR				m_szMiniDumpLogFileName[MAX_PATH];
	static HANDLE				m_hReportFile;
	static MINIDUMP_TYPE		m_eMiniDumpType;
	static bool					m_bEudoraCrashedDuringThisRun;
	static bool					m_bEudoraCrashedDuringLastRun;
	static bool					m_bBufferOverflowCheck;
	static bool					m_bBufferOverflowAskUserBeforeQuitting;
};

extern QCExceptionHandler g_QCExceptionHandler;	//  global instance of class



#endif
