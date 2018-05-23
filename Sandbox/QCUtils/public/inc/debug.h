// DEBUG.H
//
// Routines for writing info to the debug file
//

#ifndef _EUDORA_DEBUG_H_
#define _EUDORA_DEBUG_H_


#include "afxmt.h"
typedef DWORD DebugMaskType;

// Logging bitmask values
const DebugMaskType DEBUG_MASK_SEND		= 0x01;	// Log sending of a message
const DebugMaskType DEBUG_MASK_RCVD		= 0x02;	// Log receipt of a message
const DebugMaskType DEBUG_MASK_NAV		= 0x04;	// Serial login navagation
const DebugMaskType DEBUG_MASK_DIALOG	= 0x08;	// Dialog message logging
const DebugMaskType DEBUG_MASK_PROG		= 0x10;	// Progress box logging
const DebugMaskType DEBUG_MASK_TRANS	= 0x20;	// All bytes Tx
const DebugMaskType DEBUG_MASK_RCV		= 0x40;	// All bytes Rx

const DebugMaskType DEBUG_MASK_TOC_CORRUPT = 0x80;	// Check a TOC bug

const DebugMaskType DEBUG_MASK_TRANS_BASIC	= 0x100;	// Basic Translator Stuff
const DebugMaskType DEBUG_MASK_TRANS_ADV	= 0x200;	// Advanced Translator Stuff

const DebugMaskType DEBUG_MASK_ASSERT		= 0x800;	// Log ASSERTs & VERIFYs

class JJFileMT;

class AFX_EXT_CLASS QCLogFileMT
{

public:
	// Load debug values from EUDORA.INI
	//static BOOL InitDebug();
	static BOOL InitDebug(DWORD dwMask, DWORD dwSize, LPCTSTR strEudoraDir, LPCTSTR strLogFileName);

	// Cleanup left over structures.
	static void CleanupDebug();

	static void PutLineHeader(DebugMaskType ID);
	static void PutLineTrailer(DebugMaskType ID);
	static void WriteDebugLog(DebugMaskType ID, LPCTSTR Buffer, int Length = -1);
	static void WriteAssertLog(LPCTSTR pExpr, LPCTSTR pFileName, int nLineNum);

	// TRUE indicates that at least one of the mask bits are set
	static BOOL DebugMaskSet(DebugMaskType Mask);
	
public:

	static DebugMaskType	DebugMask;
	static JJFileMT*	s_pLogFile;

private:
	
	static long			DebugLogSize;
	static time_t		MarkerTime;
	
	static CString g_strLogFileName;
	static CString g_strEudoraDir;
	static CString m_AssertFmtStr;

	static CCriticalSection m_Guard;

	static BOOL m_bIsNT;

};


#ifdef TRUE //_DEBUG

__declspec( dllexport )  void PutDebugLog(DebugMaskType ID, LPCTSTR Buffer, int Length = -1);

#else
// Place null terminated character data into the log file
inline __declspec( dllexport )  void PutDebugLog(DebugMaskType ID, LPCTSTR Buffer, int Length = -1)
{
	if ((QCLogFileMT::DebugMask & ID) == 0 || !QCLogFileMT::s_pLogFile)
		return;
	QCLogFileMT::WriteDebugLog(ID, Buffer, Length);
}

#endif


#endif // _EUDORA_DEBUG_H_
