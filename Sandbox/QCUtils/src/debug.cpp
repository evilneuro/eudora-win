// DEBUG.CPP
//
// Routines for writing info to the debug file
//

#include "stdafx.h"
#include "resource.h"

#include "debug.h"

#include "QCUtils.h"

//#include "fileutil.h"  //EudoraDir

#include "rs.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

DebugMaskType	QCLogFileMT::DebugMask = 0;
long		QCLogFileMT::DebugLogSize;
time_t		QCLogFileMT::MarkerTime = 1;
JJFileMT*	QCLogFileMT::s_pLogFile = NULL;

CString		QCLogFileMT::g_strLogFileName;
CString		QCLogFileMT::g_strEudoraDir;
CCriticalSection QCLogFileMT::m_Guard;
BOOL		QCLogFileMT::m_bIsNT = FALSE;

CString QCLogFileMT::m_AssertFmtStr = "ASSERT FAILED: [\"%s\"] %s (%d)";


BOOL QCLogFileMT::InitDebug(DWORD dwMask, DWORD dwSize, LPCTSTR strEudoraDir, LPCTSTR strLogFileName)
{
	ASSERT(::IsMainThreadMT());
	
	//DebugMask = (DWORD)GetIniLong(IDS_INI_DEBUG_LOG_LEVEL);
	//DebugLogSize = GetIniLong(IDS_INI_DEBUG_LOG_FILE_SIZE);

	DebugMask = dwMask;
	DebugLogSize = dwSize;
	g_strEudoraDir = strEudoraDir;
	
	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osInfo) && osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		m_bIsNT = TRUE;
	
	// No need to do anything if there are no conditions to log
	if (DebugMask)
	{
		if (!s_pLogFile)
		{
			s_pLogFile = new JJFileMT;
			if(!s_pLogFile) return FALSE;
		}

		char Filename[_MAX_PATH + 1];
		//wsprintf(Filename, "%s%s", (const char*)EudoraDir, GetIniString(IDS_INI_DEBUG_LOG_FILE_NAME));
		wsprintf(Filename, "%s%s", strEudoraDir, strLogFileName);

		g_strLogFileName = Filename;

		//if (FAILED(s_pLogFile->Open(Filename, O_CREAT | O_APPEND | O_WRONLY)))
		if (FAILED(s_pLogFile->Open(g_strLogFileName, O_CREAT | O_APPEND | O_WRONLY)))
		{
			delete s_pLogFile;
			s_pLogFile = NULL;
			return (FALSE);
		}
	}

	return (TRUE);
}


void QCLogFileMT::CleanupDebug()
{
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
	char DigitString[10];
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

	if (Now > MarkerTime + 1200)
	{
		if (FAILED(s_pLogFile->PutLine(ctime(&Now), 24))) return;
		if (FAILED(s_pLogFile->PutLine(CRString(IDS_VERSION)))) return;
		MarkerTime = Now;
	}

	//First write the thread ID
	if (m_bIsNT)
		wsprintf(DigitString, "%-4.0u ", GetCurrentThreadId());  //reuse the digitstring
	else
		wsprintf(DigitString, "%-4.0X ", GetCurrentThreadId() & 0xFFFF);  //reuse the digitstring
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	

	wsprintf(DigitString, "%d", ID);
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	if (FAILED(s_pLogFile->Put(":"))) return;
	wsprintf(DigitString, "%ld", (Now - MarkerTime) / 60);
	if (FAILED(s_pLogFile->Put(DigitString))) return;
	if (FAILED(s_pLogFile->Put("."))) return;
	wsprintf(DigitString, "%ld", (Now - MarkerTime) % 60);
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
	if (ID & (DEBUG_MASK_TRANS | DEBUG_MASK_RCV | DEBUG_MASK_DIALOG))
		s_pLogFile->PutLine("\"");
	else
		s_pLogFile->PutLine();
}

void QCLogFileMT::WriteAssertLog(LPCSTR pExpr, LPCSTR pFileName, int nLineNum)
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
	}
}
