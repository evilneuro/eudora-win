// DEBUG.CPP
//
// Routines for writing info to the debug file
//

#include "stdafx.h"
#include "resource.h"

#include "debug.h"

#include "QCSharewareManager.h"
#include "QCUtils.h"

//#include "fileutil.h"  //EudoraDir

#include "rs.h"


#include "DebugNewHelpers.h"

bool		QCLogFileMT::s_bOutputEudoraStatusInfoNow = true;
DebugMaskType	QCLogFileMT::DebugMask = 0;
short		QCLogFileMT::s_Mode = SWM_MODE_ADWARE;
long		QCLogFileMT::DebugLogSize;
const time_t QCLogFileMT::kInitialTime = 1;
time_t		QCLogFileMT::MarkerTime = kInitialTime;
JJFileMT*	QCLogFileMT::s_pLogFile = NULL;

CString		QCLogFileMT::g_strLogFileName;
CString		QCLogFileMT::g_strEudoraDir;
CCriticalSection QCLogFileMT::m_Guard;
BOOL		QCLogFileMT::m_bIsNT = FALSE;

CString QCLogFileMT::m_AssertFmtStr = "ASSERT FAILED: [\"%s\"] %s (%d)";


BOOL QCLogFileMT::InitDebug(DWORD dwMask, DWORD dwSize, short nMode, LPCTSTR strEudoraDir, LPCTSTR strLogFileName)
{
	ASSERT(::IsMainThreadMT());
	
	DebugMask = dwMask;
	DebugLogSize = dwSize;
	g_strEudoraDir = strEudoraDir;

	s_Mode = nMode;
	
	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osInfo) && osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		m_bIsNT = TRUE;
	
	// No need to do anything if there are no conditions to log
	if (DebugMask)
	{
		if (!s_pLogFile)
		{
			s_pLogFile = DEBUG_NEW_NOTHROW JJFileMT;
			if(!s_pLogFile) return FALSE;
		}

		char Filename[_MAX_PATH + 1];
		//wsprintf(Filename, "%s%s", (const char*)EudoraDir, GetIniString(IDS_INI_DEBUG_LOG_FILE_NAME));
		wsprintf(Filename, "%s%s", strEudoraDir, strLogFileName);

		g_strLogFileName = Filename;

		//if (FAILED(s_pLogFile->Open(Filename, O_CREAT | O_APPEND | O_WRONLY)))
		if (FAILED(s_pLogFile->Open(g_strLogFileName, O_CREAT | O_APPEND | O_WRONLY)))
		{
			// Format the error string (just like JJFile does)
			CString		strError;

			::AfxFormatString2( strError, IDS_ERR_FILE_OPEN,
								static_cast<const char *>(g_strLogFileName),
								static_cast<const char *>(CRString(IDS_ERR_FILE_OPEN_WRITING)) );

			// Format the error message (just like JJFile does)
			int nError = s_pLogFile->GetLastError_();
			if (nError < 0 || nError > 36)
				nError = 37;		// unknown error

			const int IDS_FILE_BASE_ERROR = 8500;

			CString		strErrorMessage;

			strErrorMessage.Format( IDS_DOS_ERR_FORMAT, static_cast<const char *>(strError),
									static_cast<const char *>(CRString(IDS_FILE_BASE_ERROR + nError)),
									nError );

			// Combine the file error message with message explaining that Eudora
			// can't work without the ability to write to Eudora.log.
			CString		strMessage;

			strMessage.Format( IDS_ERR_OPENING_DEBUG_LOG,
							   static_cast<const char *>(strErrorMessage),
							   strLogFileName );

			// Display the error message before we return FALSE and Eudora quits
			MessageBox(NULL, strMessage, NULL, MB_OK | MB_ICONERROR);

			delete s_pLogFile;
			s_pLogFile = NULL;

			return (FALSE);
		}
	}

	return (TRUE);
}


void QCLogFileMT::NotifyIniChange(UINT nStringNum, long nValue)
{
	switch (nStringNum)
	{
		case IDS_INI_DEBUG_LOG_LEVEL:
			s_bOutputEudoraStatusInfoNow = ( DebugMask != static_cast<DebugMaskType>(nValue) );
			DebugMask = nValue;
			break;

		case IDS_INI_MODE:
			s_bOutputEudoraStatusInfoNow = ( s_Mode != static_cast<short>(nValue) );
			s_Mode = static_cast<short>(nValue);
			break;
	}
}


void QCLogFileMT::CleanupDebug()
{
	WriteDebugLog(DebugMask, "Logging shutdown");
	delete s_pLogFile;
	s_pLogFile = NULL;
}


#ifdef TRUE //_DEBUG

// Place null terminated character data into the log file
void PutDebugLog(DebugMaskType ID, LPCTSTR Buffer, int Length /* = -1*/)
{
	if ((QCLogFileMT::DebugMask & ID) == 0 || !QCLogFileMT::s_pLogFile)
		return;
	QCLogFileMT::WriteDebugLog(ID, Buffer, Length);
}

#endif


BOOL QCLogFileMT::DebugMaskSet(DebugMaskType Mask)
{ 
	return ((DebugMask & Mask) != 0); 
}


void QCLogFileMT::PutLineHeader(DebugMaskType ID)
{
	char DigitString[32];
	time_t Now = time(NULL);

	// Close and reopen the file every minute so that when running under
	// OSes like Win 3.1 the file gets updated
	ASSERT(s_pLogFile);
	if ((Now - MarkerTime) % 60 == 0)
	{
		s_pLogFile->Close();

		CString strFilename("???");
		{
			BSTR bstrFilename = NULL;
			if (SUCCEEDED(s_pLogFile->GetFName(&bstrFilename)))
			{
				strFilename = bstrFilename;
				::SysFreeString(bstrFilename);
			}
		}

		if (FAILED(s_pLogFile->Open(strFilename, O_CREAT | O_APPEND | O_WRONLY)))
		{
			delete s_pLogFile;
			s_pLogFile = NULL;
			return;
		}
	}

	if ( s_bOutputEudoraStatusInfoNow || (Now > MarkerTime + 1200) )
	{
		if (MarkerTime == kInitialTime)
		{
			// Make it more obvious where Eudora is actually launching
			if ( FAILED(s_pLogFile->PutLine()) ) return;
		}

		// Write out the time and version of Eudora
		if (FAILED(s_pLogFile->PutLine(ctime(&Now), 24))) return;
		if (FAILED(s_pLogFile->PutLine(CRString(IDS_VERSION)))) return;

		// Write out the log level
		sprintf(DigitString, "LogLevel %d (0x%X)", QCLogFileMT::DebugMask, QCLogFileMT::DebugMask);
		if (FAILED(s_pLogFile->PutLine(DigitString))) return;

		// Write out the Eudora mode (i.e. sponsored, light, paid) 
		sprintf(DigitString, "Mode %d", QCLogFileMT::s_Mode);
		if (FAILED(s_pLogFile->PutLine(DigitString))) return;

		MarkerTime = Now;
		s_bOutputEudoraStatusInfoNow = false;
	}

	//First write the thread ID
	if (::IsMainThreadMT())
		strcpy(DigitString, "MAIN ");
	else
	{
		if (m_bIsNT)
			wsprintf(DigitString, "%-4.0u ", GetCurrentThreadId());  //reuse the digitstring
		else
			wsprintf(DigitString, "%-4.0X ", GetCurrentThreadId() & 0xFFFF);  //reuse the digitstring
	}
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	

	wsprintf(DigitString, "%5d", ID);
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	if (FAILED(s_pLogFile->Put(":"))) return;
	wsprintf(DigitString, "%2ld", (Now - MarkerTime) / 60);
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	if (FAILED(s_pLogFile->Put("."))) return;
	wsprintf(DigitString, "%02ld", (Now - MarkerTime) % 60);
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	if (FAILED(s_pLogFile->Put(" "))) return;

	switch (ID)
	{
	case DEBUG_MASK_TRANS:	s_pLogFile->Put(CRString(IDS_DEBUG_SENT));		break;
	case DEBUG_MASK_RCV:	s_pLogFile->Put(CRString(IDS_DEBUG_RECEIVED));	break;
	case DEBUG_MASK_DIALOG:	s_pLogFile->Put(CRString(IDS_DEBUG_DIALOG));	break;
	case DEBUG_MASK_TRANS_BASIC:	
	case DEBUG_MASK_TRANS_ADV:	s_pLogFile->Put(CRString(IDS_DEBUG_TRANS));	break;
	}
}



void QCLogFileMT::PutLineTrailer(DebugMaskType ID)
{
	if (ID == DEBUG_MASK_TRANS || ID == DEBUG_MASK_RCV || ID == DEBUG_MASK_DIALOG)
		s_pLogFile->PutLine("\"");
	else
		s_pLogFile->PutLine();
}

void QCLogFileMT::WriteAssertLog(LPCTSTR pExpr, LPCTSTR pFileName, int nLineNum)
{
	// If no options are set, then ignore this call
	if (((DebugMask & DEBUG_MASK_ASSERT) == 0) || (!s_pLogFile))
		return;

	char str[1024];
	const int nLen = sprintf(str, m_AssertFmtStr, pExpr, pFileName, nLineNum);

	WriteDebugLog(DEBUG_MASK_ASSERT, str, nLen);
}

//
// Outputs a single line.  Input is a null terminated string.
//
//void QCLogFileMT::PutDebugLog(DebugMaskType ID, LPCTSTR Buffer, int Length /*= -1*/)
void QCLogFileMT::WriteDebugLog(DebugMaskType ID, LPCTSTR Buffer, int Length /*= -1*/)
{
	if (!Buffer || !*Buffer)
		return;

	//
	// Normally, you should only call this if you have a known
	// good logfile object.  However, in the startup case, it is
	// possible to get an error when trying to open the this logfile.
	// The problem is that the code tries to log a logfile open error
	// (your basic chicken/egg problem).  Therefore, the solution
	// seems to be to bounce attempts to write to an invalid logfile
	// here at a low level.
	//
	if ((s_pLogFile == NULL) || (S_FALSE == s_pLogFile->IsOpen()))
		return;

	if (Length < 0)
		Length = strlen(Buffer);

	// If no options are set, then ignore this call
	if ((DebugMask & ID) == 0 || !s_pLogFile)
		return;

	CSingleLock lock(&m_Guard, TRUE);

	BOOL WasCR = FALSE;
	BOOL StartNewLine = TRUE;
	for (; Length; Buffer++, Length--)
	{
		if (StartNewLine)
		{
			PutLineHeader(ID);
			StartNewLine = FALSE;
		}
		if (*Buffer == '\r')
		{
			if (FAILED(s_pLogFile->Put("\\r"))) return;
			WasCR = TRUE;
		}
		else if (*Buffer == '\n')
		{
			if (FAILED(s_pLogFile->Put("\\n"))) return;
			PutLineTrailer(ID);
			StartNewLine = TRUE;
			WasCR = FALSE;
		}
		else
		{
			if (WasCR)
			{
				PutLineTrailer(ID);
				WasCR = FALSE;
				PutLineHeader(ID);
			}
			if (*Buffer >= ' ' && *Buffer <= 0x7F)	{ s_pLogFile->Put(*Buffer); }
			else if (*Buffer == '\t')				{ s_pLogFile->Put("\\t"); }
			else if (*Buffer == '\b')				{ s_pLogFile->Put("\\b"); }
			else if (*Buffer == '\f')				{ s_pLogFile->Put("\\f"); }
			else if (*Buffer == '\007')				{ s_pLogFile->Put("\\g"); }
			else
			{
				if (FAILED(s_pLogFile->Put('\\'))) return;
				if (FAILED(s_pLogFile->Put((char)((((*Buffer) >> 6) & 0x01) + '0')))) return;
				if (FAILED(s_pLogFile->Put((char)((((*Buffer) >> 3) & 0x03) + '0')))) return;
				if (FAILED(s_pLogFile->Put((char)((((*Buffer) >> 0) & 0x03) + '0')))) return;
			}
		}
	}

	if (!StartNewLine)
		PutLineTrailer(ID);
	s_pLogFile->Flush();

	struct stat statbuf;
	s_pLogFile->Stat(&statbuf);
	if (statbuf.st_size > (DebugLogSize * 1024))
	{
		char Filename[_MAX_PATH + 1];
		//strcpy(Filename, EudoraDir);
		strcpy(Filename, g_strEudoraDir);
		strcat(Filename, CRString(IDS_DEBUG_LOG_FILE_NAME_OLD));
		::FileRemoveMT(Filename);
		s_pLogFile->Rename(Filename);
		s_pLogFile->Close();
		//strcpy(Filename + EudoraDirLen, GetIniString(IDS_INI_DEBUG_LOG_FILE_NAME));
		//s_pLogFile->Open(Filename, O_CREAT | O_WRONLY | O_TRUNC);
		s_pLogFile->Open(g_strLogFileName, O_CREAT | O_WRONLY | O_TRUNC);

		// Make sure new log file starts with the logging info
		s_bOutputEudoraStatusInfoNow = true;
	}
}
