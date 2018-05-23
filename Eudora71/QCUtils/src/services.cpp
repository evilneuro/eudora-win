////////////////////////////////////////////////////////////////////////
// File: session.cpp
//
//
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <afxpriv.h>		// for T2W()

#include <io.h>
#include <ctype.h>

#include "qcerror.h"
#include "services.h"

#include "tlhelp32.h"

#include "rs.h"
#include "resource.h"

const char chrSLASH = '\\';

#include "DebugNewHelpers.h"


// API definitions
#ifndef REPLACEFILE_WRITE_THROUGH
	#define REPLACEFILE_WRITE_THROUGH 0x00000001
#endif
#ifndef REPLACEFILE_IGNORE_MERGE_ERRORS
	#define REPLACEFILE_IGNORE_MERGE_ERRORS 0x00000002
#endif

#ifndef ERROR_UNABLE_TO_MOVE_REPLACEMENT
	#define ERROR_UNABLE_TO_MOVE_REPLACEMENT 1176L
#endif
#ifndef ERROR_UNABLE_TO_MOVE_REPLACEMENT_2
	#define ERROR_UNABLE_TO_MOVE_REPLACEMENT_2 1177L
#endif

typedef BOOL				(WINAPI * IndirectReplaceFile)(
											LPCWSTR		lpReplacedFileName,		// file name
											LPCWSTR		lpReplacementFileName,	// replacement file
											LPCWSTR		lpBackupFileName,		// optional backup file
											DWORD		dwReplaceFlags,			// replace options
											LPVOID		lpExclude,				// reserved
											LPVOID		lpReserved );			// reserved

// Constants
DWORD						kMinExtraFreeSpace = 1024 * 1024;
								// Prefer to not have less than 1 Meg free


//	Local, non-exported function declarations
void
GetFileNameAndParentPath(
	const char *			in_szFullPath,
	char *					out_szParentPathBuffer,
	int						in_nParentPathBufferSize,
	CString *				out_szFileName = NULL);
HRESULT
MoveFileMaintainingCorrectPermissions(
	const char *			in_szOldName,
	const char *			in_szNewName,
	DWORD					in_dwFileSize,
	DWORD					in_dwFreeSpaceDestinationVolume);
void
SetLastErrorResult(
	HRESULT* phr);


////////////////////////////////////////////////////////////////////////
// HashMT [extern, exported]
//
// The following hashing algorithm, KRHash, is derived from Karp & Rabin,
// Harvard Center for Research in Computing Technology Tech. Report TR-31-81
// The constant prime number in use is kKRHashPrime.  It happens to be the largest
// prime number that will fit in 31 bits, except for 2^31-1 itself....
////////////////////////////////////////////////////////////////////////
ULONG HashMT(const char* pszStr)
{
	const ULONG kKRHashPrime = 2147483629;

	ULONG ulSum = 0;		// returned

	for (; *pszStr; pszStr++)
	{
		for (int nBit = 0x80; nBit != 0; nBit >>= 1)
		{
			ulSum += ulSum;
			if (ulSum >= kKRHashPrime)
				ulSum -= kKRHashPrime;

			if ((*pszStr) & nBit)
				++ulSum;

			if (ulSum>= kKRHashPrime)
				ulSum -= kKRHashPrime;
		}
	}

	return (ulSum + 1);
}


////////////////////////////////////////////////////////////////////////
// HashMT [extern, exported]
//
// The following hashing algorithm, KRHash, is derived from Karp & Rabin,
// Harvard Center for Research in Computing Technology Tech. Report TR-31-81
// The constant prime number in use is kKRHashPrime.  It happens to be the largest
// prime number that will fit in 31 bits, except for 2^31-1 itself....
////////////////////////////////////////////////////////////////////////
ULONG HashMTIgnoreCase(const char* pszStr)
{
	const ULONG kKRHashPrime = 2147483629;

	ULONG ulSum = 0;		// returned

	for (; *pszStr; pszStr++)
	{
		for (int nBit = 0x80; nBit != 0; nBit >>= 1)
		{
			ulSum += ulSum;
			if (ulSum >= kKRHashPrime)
				ulSum -= kKRHashPrime;

			//here we create the hash always by ignoring the hash values
			char ch = *pszStr;
			if (tolower(ch) & nBit)
				++ulSum;

			if (ulSum>= kKRHashPrime)
				ulSum -= kKRHashPrime;
		}
	}

	return (ulSum + 1);
}


////////////////////////////////////////////////////////////////////////
// IsMainThreadMT [extern, exported]
//
// Returns TRUE if the calling thread is the main MFC thread represented
// by CWinApp.
////////////////////////////////////////////////////////////////////////
BOOL IsMainThreadMT()
{
	ASSERT(::AfxGetApp()->m_nThreadID);
	return (::GetCurrentThreadId() == ::AfxGetApp()->m_nThreadID);
}


////////////////////////////////////////////////////////////////////////
// TimeDateStringMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
char* TimeDateStringMT(char* pszBuffer, long lSeconds, BOOL bDoDate)
{
	return ::TimeDateStringFormatMT(pszBuffer, lSeconds, 0, bDoDate ? "%1 %2" : "%1");
}


////////////////////////////////////////////////////////////////////////
// TimeDateStringFormatMT [extern, exported]
//
// Produces a time/date string in a specified format.
// Format can contain the following parameters:
//		%1	Time
//		%2	Date
//		%3	Day of week
//		%4	Timezone
// Caller should pass in at least a 128-byte buffer
////////////////////////////////////////////////////////////////////////
char* TimeDateStringFormatMT(char* buf, long Seconds, int TimeZoneMinutes, const char* Format)
{
	char Time[32];
	char Date[32];
	char DayOfWeek[32];
	char TimeZone[32];
	const char* Args[4] = {Time, Date, DayOfWeek, TimeZone};

	static BOOL bWasInited = FALSE;
	static char iTime[3];
	static char sTime[3];
	static char s1159[8];
	static char s2359[8];
	static char sShortDate[32];
	if (!bWasInited)
	{
		if (!::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME, iTime, sizeof(iTime)))
			::strcpy(iTime, "0");
		if (!::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, sTime, sizeof(sTime)))
			::strcpy(sTime, ":");
		if (!::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_S1159, s1159, sizeof(s1159)))
			::strcpy(s1159, "AM");
		if (!::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_S2359, s2359, sizeof(s2359)))
			::strcpy(s2359, "PM");
		if (!::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, sShortDate, sizeof(sShortDate)))
			::strcpy(sShortDate, "m/d/yy");
		bWasInited = TRUE;
	}

	if (Seconds < 0)
		Seconds = 1;
	struct tm *TheTime = ::localtime(&Seconds);

	// Find out types are in the format string
	BOOL bHasTime = FALSE;
	BOOL bHasDate = FALSE;
	BOOL bHasDayOfWeek = FALSE;
	BOOL bHasTimeZone = FALSE;
	for (const char* f = Format; *f; )
	{
		if (*f++ == '%')
		{
			switch (*f)
			{
			case '1': bHasTime = TRUE;		break;
			case '2': bHasDate = TRUE;		break;
			case '3': bHasDayOfWeek = TRUE;	break;
			case '4': bHasTimeZone = TRUE;	break;
			}
			f++;
		}
	}


	if (bHasDate)
	{
		char *d = Date;
		int count;
		for (const char *s = sShortDate; *s; s++)
		{
			switch (*s)
			{
			case 'M':
			case 'm':
				count = 1;
				while (s[1] == 'M' || s[1] == 'm')
				{
					count++;
					s++;
				}
				d += ::sprintf(d, (count < 2? "%d" : "%02d"), TheTime->tm_mon + 1);
				break;

			case 'D':
			case 'd':
				count = 1;
				while (s[1] == 'D' || s[1] == 'd')
				{
					count++;
					s++;
				}
				d += ::sprintf(d, (count < 2? "%d" : "%02d"), TheTime->tm_mday);
				break;

			case 'Y':
			case 'y':
				count = 1;
				while (s[1] == 'Y' || s[1] == 'y')
				{
					count++;
					s++;
				}
				d += ::sprintf(d, (count < 4? "%02d" : "%04d"),
					(count < 4? TheTime->tm_year % 100 : TheTime->tm_year + 1900));
				break;

			default:
				*d++ = *s;
			}
		}

		// Optimization, as most dates will just be ANCIENT format
		if (strcmp(Format, "%2") == 0)
		{
			::strcpy(buf, Date);
			return (buf);
		}
	}

	if (bHasTime)
	{
		int hour = TheTime->tm_hour;
		if ('0' == *iTime)
		{
			hour %= 12;
			if (hour == 0)
				hour = 12;
		}
		char *AmPm = "";
		if ('0' == *iTime)
			AmPm = TheTime->tm_hour < 12? s1159 : s2359;
		::sprintf(Time, "%02d%s%02d%s%s", hour, sTime, TheTime->tm_min, ('0' == *iTime? " " : ""), AmPm);
	}

	if (bHasDayOfWeek)
		::strftime(DayOfWeek, sizeof(DayOfWeek), "%A", TheTime);

	if (bHasTimeZone)
	{
		if (TimeZoneMinutes == INT_MAX)
			TimeZone[0] = 0;
		else
			sprintf(TimeZone, "%+2.2d%2.2d", TimeZoneMinutes / 60, abs(TimeZoneMinutes) % 60);
	}

	{
		CString Result;
		::AfxFormatStrings(Result, Format, Args, 4);
		Result.TrimRight();			// remove trailing whitespace
		int len = Result.GetLength();
		if (len>63) len = 63;		// 63 makes it not crash	
		::strncpy(buf, Result, len);
		buf[len] = 0;
		return (buf);
	}
}


////////////////////////////////////////////////////////////////////////
// FormatTimeMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
CString FormatTimeMT( time_t theTime, const char * pFormat )
{
	char    szBuffer[ 128 ];

	struct tm* ptmTemp = ::localtime( &theTime );
	ASSERT(ptmTemp != NULL); // make sure the time has been initialized!

	if (!::strftime(szBuffer, sizeof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';

	return szBuffer;
}


////////////////////////////////////////////////////////////////////////
// IsDaylightSavingsTimeMT [static]
//
// Zortech C++ compiler doesn't figure Daylight Savings Time, so we
// have to do it ourselves.
//
// Daylight Savings Time changes the first Sunday in April and
// the last Sunday in October.
////////////////////////////////////////////////////////////////////////
static BOOL IsDaylightSavingsTimeMT()
{
	time_t curTime = ::time(NULL);
	struct tm* pTimeInfo = localtime(&curTime);
	if (NULL == pTimeInfo)
		return FALSE;

	int nMonth = pTimeInfo->tm_mon;
	
	if (nMonth < 3 || nMonth > 9)
		return (FALSE);		// Months 0 - 2, and 10 - 11 are always not DST
	if (nMonth > 3 && nMonth < 9)
		return TRUE;		// Months 4 - 8 are always DST

	// Check out an April date
	if (nMonth == 3)
	{
		// See if there's a Sunday in the month before (and including) today
		if (pTimeInfo->tm_wday - pTimeInfo->tm_mday < 0)
			return TRUE;	// found one, so it is DST
		return FALSE;		// before first Sunday, so not DST
	}

	// Check out an October date
	// See if there's a Sunday in the month after today
	int nDaysLeft = 31 - pTimeInfo->tm_mday - 1;
	if (pTimeInfo->tm_wday + nDaysLeft > 6)
		return TRUE;	// found one, so still DST
	return FALSE;		// past last Sunday, so now we're not DST
}


////////////////////////////////////////////////////////////////////////
// GetGMTOffsetMT [extern]
//
// Find the time zone offset from GMT of this PC.  FORNOW, this doesn't 
// really belong here anymore...
////////////////////////////////////////////////////////////////////////
HRESULT GetGMTOffsetMT(const char* pszTimeZone, int* pOffset)
{
	if (NULL == pszTimeZone || NULL == pOffset)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	static int nGMTOffset = -1;
	static int LastHourChecked = -1;
	static int LastDayChecked = -1;
	time_t Now = time(NULL);
	const struct tm* TimeStruct = localtime(&Now);

	if (nGMTOffset != -1)
	{
		if (TimeStruct && TimeStruct->tm_hour == LastHourChecked && TimeStruct->tm_mday == LastDayChecked)
		{
			// We've already done the calculation in this hour, so short circuit
			// the calculation and return the previously computed result.
			*pOffset = nGMTOffset;
			return S_OK;
		}
	}

	if (TimeStruct)
	{
		LastHourChecked = TimeStruct->tm_hour;
		LastDayChecked = TimeStruct->tm_mday;
	}

	char szTimeZone[64] = { '\0' };
	strncpy(szTimeZone, pszTimeZone, sizeof(szTimeZone));
	szTimeZone[sizeof(szTimeZone) - 1] = '\0';		// just in case

	if (!*szTimeZone)
	{
		char szBuffer[64];
		if (::GetEnvironmentVariable("TZ", szBuffer, sizeof(szBuffer)))
		{
			//
			// TimeZone INI setting not set, so use TZ environment
			// variable.
			//
			::strcpy(szTimeZone, szBuffer);
		}
		else
		{
			//
			// TimeZone INI setting and TZ environment variable are both
			// not set, so see if the system can figure it out.
			//
			TIME_ZONE_INFORMATION tzi;
			switch (::GetTimeZoneInformation(&tzi))
			{
			case TIME_ZONE_ID_STANDARD: 
				*pOffset = tzi.Bias + tzi.StandardBias;
				break;
			case TIME_ZONE_ID_DAYLIGHT: 
				*pOffset = tzi.Bias + tzi.DaylightBias;
				break;
			case TIME_ZONE_ID_UNKNOWN:
				// Weird case.  Under NT, if the current timezone doesn't change time
				// for Daylight Savings, then GetTimeZoneInforamtion() returns
				// TIME_ZONE_ID_UNKNOWN.  Win 95 returns TIME_ZONE_ID_STANDARD, so it
				// will get correctly handled above and StandardBias will be zero.
				OSVERSIONINFO OSInfo;
				ZeroMemory(&OSInfo, sizeof(OSInfo));
				OSInfo.dwOSVersionInfoSize = sizeof(OSInfo);
				if (GetVersionEx(&OSInfo) && OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
				{
					*pOffset = tzi.Bias;
					break;
				}
				// fall through to default case
			default:
				// Don't know, so just use a zero offset
				ASSERT(0);
				*pOffset = 0;
			}

			nGMTOffset = *pOffset;
			return S_OK;
		}
	}

	//
	// Parse the time zone from either the TimeZone INI setting or the
	// TZ system environment variable.
	//
	::TrimWhitespaceMT(szTimeZone);
	if (::strlen(szTimeZone) < 4)
	{
		*pOffset = 0;
		return E_FAIL;
	}

	char* pszZone = szTimeZone;
	while (*pszZone && !::isdigit((int)(unsigned char)*pszZone) && *pszZone != '-' && *pszZone != '+')
		pszZone++;
	if (!*pszZone)
	{
		*pOffset = 0;
		return E_FAIL;
	}
	nGMTOffset = ::atoi(pszZone);
	if (*pszZone == '-' || *pszZone == '+')
		pszZone++;
	if (!::isdigit((int)(unsigned char)*pszZone))
	{
		*pOffset = 0;
		return E_FAIL;
	}
	while (::isdigit((int)(unsigned char)*pszZone))
		pszZone++;

	nGMTOffset *= 60;
	if (*pszZone && IsDaylightSavingsTimeMT())
		nGMTOffset -= 60;
	if (nGMTOffset == -24*60)
		nGMTOffset = 0;
	else if (nGMTOffset < -23*60 || nGMTOffset > 23*60)
	{
		*pOffset = 0;
		return E_FAIL;
	}

	*pOffset = nGMTOffset;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// CSortedStringList::Add [public]
//
////////////////////////////////////////////////////////////////////////
void CSortedStringListMT::Add( LPCSTR lpString )	// does a sorted insert
{
	CString str(lpString);
	Add(str);
}


////////////////////////////////////////////////////////////////////////
// CSortedStringList::Add [public]
//
////////////////////////////////////////////////////////////////////////
void CSortedStringListMT::Add( CString & aString )	// does a sorted insert
{
	if ( IsEmpty() )
	{
		AddHead( aString );
	}
	else
	{
		POSITION pos = GetHeadPosition();
		POSITION prevpos = NULL;

		while ( pos )
		{
			CString & node = GetNext( pos );

			if ( node > aString )
			{
				InsertBefore( prevpos, aString );
				return;
			}

			prevpos = pos;
		}

		// still not added
		AddTail( aString );
	}
}


//FORNOWvoid CSortedStringList::Dump( void )
//FORNOW{
//FORNOW	POSITION pos;
//FORNOW
//FORNOW	pos = GetHeadPosition();
//FORNOW
//FORNOW	while ( pos )
//FORNOW	{
//FORNOW		CString & node = GetNext( pos );
//FORNOW
//FORNOW		AfxMessageBox( node );
//FORNOW	}
//FORNOW}


////////////////////////////////////////////////////////////////////////
// SafeStrdupMT [extern, exported]
//
// A safe form of strdup that checks for NULL pointers,
// and uses new instead of malloc.
////////////////////////////////////////////////////////////////////////
char* SafeStrdupMT(const char* pszString)
{
	if (NULL == pszString)
		return NULL;

	char* pszNewString = DEBUG_NEW_NOTHROW char[strlen(pszString) + 1];
	if (pszNewString)
		return ::strcpy(pszNewString, pszString);

	return NULL;
}


////////////////////////////////////////////////////////////////////////
// SafeStrlenMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
size_t SafeStrlenMT(const char* pszString)
{
	return (pszString ? ::strlen(pszString) : 0);
}


////////////////////////////////////////////////////////////////////////
// TrimWhitespaceMT [extern, exported]
//
// Gets rid of white space at the beginning and end of a string.
////////////////////////////////////////////////////////////////////////
char* TrimWhitespaceMT(char* pszBuffer)
{
	if (!pszBuffer || !*pszBuffer)
		return pszBuffer;

	//
	// Seek to first non-space character.
	//
	char *pszBufPtr = pszBuffer;
	while (pszBufPtr && ::isspace((int)(unsigned char)*pszBufPtr))
		pszBufPtr++;

	//
	// Save start character.
	//
	char *pszBegin = pszBufPtr++;

	for (char* pszEnd = pszBufPtr; *pszBufPtr; pszBufPtr++)
		if (!::isspace((int)(unsigned char)*pszBufPtr))
			pszEnd = pszBufPtr;

	// if buf is 1 char long and 'pszEnd' is a WS, terminate it
	if (*pszEnd && ::isspace((int)(unsigned char)*pszEnd))
		*pszEnd = '\0';
	else if (*pszEnd)
		pszEnd[1] = '\0';

	//
	// Copy trimmed string in-place, if necessary.
	//
	if (pszBegin != pszBuffer)
		::strcpy(pszBuffer, pszBegin);

	return pszBuffer;
}


////////////////////////////////////////////////////////////////////////
// TrimWhitespaceMT [extern, exported]
//
// Gets rid of white space at the beginning and end of a string
////////////////////////////////////////////////////////////////////////
const char* TrimWhitespaceMT(CString& cs)
{
	::TrimWhitespaceMT(cs.GetBuffer(cs.GetLength()));
	cs.ReleaseBuffer();
	return cs;
}


////////////////////////////////////////////////////////////////////////
// RemovePrefixMT [extern, exported]
//
// Routine to get rid of prefixes.  Handy for ditching Re: part of subjects.
////////////////////////////////////////////////////////////////////////
const char* RemovePrefixMT(const char* Prefix, const char* String)
{
	ASSERT(Prefix && *Prefix);	// Bozo!

	if (!String)
		return (String);

	const char* s = String;
	for (; *Prefix && *s; s++)
	{
		if (tolower(*s) == tolower(*Prefix))
			Prefix++;
		else
		{
			if (isalpha((unsigned char)*s))
				return (String);
		}
	}

	// If we didn't match the whole prefix, then it's not a match
	if (*Prefix)
		return (String);

	while (isspace((int)(unsigned char)*s))
		s++;

	return (s);
}

//
// RemoveSubjectPrefixMT [extern, exported]
//
// Removes appropriate subject prefixes -- quickly.
//
const char *RemoveSubjectPrefixMT(const char *pSubjectStr)
{
	const char cDelim = ':';
	const char *pSave;

	if (pSubjectStr)
	{
		while (*pSubjectStr)
		{
			pSave = pSubjectStr;

			// Look for "Re"
			if ((pSubjectStr[0] == 'R' || pSubjectStr[0] == 'r') &&
				(pSubjectStr[1] == 'E' || pSubjectStr[1] == 'e'))
			{
				pSubjectStr += 2;
			}
			// Look for "Fw" or "Fwd"
			else if ((pSubjectStr[0] == 'F' || pSubjectStr[0] == 'f') &&
				(pSubjectStr[1] == 'W' || pSubjectStr[1] == 'w'))
			{
				pSubjectStr += 2;
				if (pSubjectStr[0] == 'D' || pSubjectStr[0] == 'd')
					pSubjectStr += 1;
			}
			else
				break; // Niether prefix, so we are done

			// Now skip non-alpha chars til we hit the deliminator char
			while ((*pSubjectStr) && (*pSubjectStr != cDelim) && (!isalpha((unsigned char)*pSubjectStr)))
				pSubjectStr++;

			// If we hit an alpha or end before the delim, then
			// we back up to before the prefix and we are done
			if (*pSubjectStr != cDelim)
				return (pSave);

			// Skip the delim
			pSubjectStr++;

			// Skip whitespace after the delim
			while (isspace((unsigned char)*pSubjectStr))
				pSubjectStr++;
		}
	}

	return (pSubjectStr);
}

////////////////////////////////////////////////////////////////////////
// SwapShortMT [extern, exported]
//
// Byte swapping routine for 16-bit quantities.
////////////////////////////////////////////////////////////////////////
unsigned short SwapShortMT(unsigned short sword)
{
	ASSERT(sizeof(unsigned short) == 2);

#pragma warning(disable : 4244)
	// 32 bit compiler will use an unsigned integer for the math
	unsigned int t = ((unsigned int)sword) & 0x00FF;
	sword = (unsigned short)(((unsigned int)sword >> 8) & 0x00FF) | ((t << 8) & 0xFF00);
	return (sword);
#pragma warning(default : 4244)
}


////////////////////////////////////////////////////////////////////////
// SwapLongMT [extern, exported]
//
// Byte swapping routine for 32-bit quantities.
////////////////////////////////////////////////////////////////////////
unsigned long SwapLongMT(unsigned long lword)
{
	ASSERT(sizeof(unsigned long) == 4);

	unsigned long t = lword & 0x0000FFFF;
	lword = (lword >> 16) & 0x0000FFFF;
	t = ::SwapShortMT((unsigned short) t);
	lword = ::SwapShortMT((unsigned short) lword);

	lword = lword | ((t << 16) & 0xFFFF0000);

	return (lword);
}


bool IsVersionGreaterThanOSR2()
{
	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(!GetVersionEx(&osInfo))
		return false;
	
	//if os is greater than win95 OSR2 
	if( (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
		(osInfo.dwMinorVersion > 0 || osInfo.dwBuildNumber > 1000) )
		return true;

	if( (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		(osInfo.dwMajorVersion >= 4) )
		return true;

	return false;
}



////////////////////////////////////////////////////////////////////////
// GetAvailableSpaceMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
DWORD GetAvailableSpaceMT(const char* pszDrive)
{
	DWORD	dwTotBytes = 0xFFFFFFFFL;	// very large by default
	ASSERT( pszDrive && *pszDrive );
	
	// pszDrive should be the drive letter, case-insensitive OR UNC drive path
	// It doesn't matter what follows (Root can point to a path like EudoraDir)
	char szDrive[ _MAX_PATH ];
	strncpy( szDrive, pszDrive, _MAX_PATH - 1 );
	szDrive[ _MAX_PATH - 1 ] = '\0';

	if ( strlen( szDrive ) < 3 )
		return dwTotBytes;

	if ( szDrive[ 1 ] == ':' )
	{
		szDrive[ 3 ] = '\0';
		strupr( szDrive );
	}
	else if ( strstr( szDrive, "\\\\" ) == szDrive )
	{
		// UNCs are \\machine\sharename\ - find end of machine name
		char * end = strchr( &szDrive[ 2 ], '\\' );
		if ( end )
		{
			// now find the end of the sharename
			end = strchr( end + 1, '\\' );
			if ( end )
				*(end + 1) = '\0';
		}
	}
	else
		return dwTotBytes;	// bad path

	// see if we have enough disk space
	{
		DWORD	SectorsPerCluster;
		DWORD	BytesPerSector;
		DWORD	FreeClusters;
		DWORD	TotalClusters;

		ULARGE_INTEGER FreeBytesAvailableToCaller;
		ULARGE_INTEGER TotalNumberOfBytes;
		ULARGE_INTEGER TotalNumberOfFreeBytes;
        
		
		// smohanty: 04/24/98 
		//
		// BEGIN
		//
		// GetDiskFreeSpaceEx is a function in Kernel32.dll in NT and >=Win95 OSR2.
		// The code here used to call GetDiskFreeSpaceEx() which would obviously cause the
		// run-time loader to fail on a <Win95 OSR2 since this function does not exist in
		// those versions versions of Kernel32.dll. The fix here is to call GetDiskFreeSpaceEx()
		// through the address obtained by GetProcAddress(). This means that GetDisFreeSpaceEx()
		// never gets called directly and the run-time loader will merrily load away.

		HINSTANCE hKernel32Inst = ::GetModuleHandle("KERNEL32.DLL");

		if (hKernel32Inst) {
			// But GetDiskFreeSpaceEx is a macro. So, we have to call the right one.
			const char *pGetDiskFreeSpaceEx =
#ifdef UNICODE
				"GetDiskFreeSpaceExW";
#else
				"GetDiskFreeSpaceExA";
#endif
			FARPROC fProcGetDiskFreeSpaceEx = ::GetProcAddress(hKernel32Inst, pGetDiskFreeSpaceEx);
			if (fProcGetDiskFreeSpaceEx != NULL) {
				typedef BOOL (__stdcall *IndirectGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER,
										   PULARGE_INTEGER);
				IndirectGetDiskFreeSpaceEx pFn = (IndirectGetDiskFreeSpaceEx)(fProcGetDiskFreeSpaceEx);
				if ((*pFn)(szDrive, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes)) {
					dwTotBytes = (TotalNumberOfFreeBytes.HighPart > 0) ? 0xFFFFFFFFL : TotalNumberOfFreeBytes.LowPart;
				}
			}
			else { // Could not get the address of GetDiskFreeSpaceEx --- revert back to GetDiskFreeSpace
				if ( ::GetDiskFreeSpace( szDrive, &SectorsPerCluster, &BytesPerSector, &FreeClusters, &TotalClusters ) ) {
					dwTotBytes = FreeClusters * SectorsPerCluster * BytesPerSector;
				}
			}
		}
		else { // Could not load Kernel32.dll. Well, how the hell did the person boot his machine?
			ASSERT(0);
		}
		// smohanty: 04/24/98 
		//
		// END
	}

	return dwTotBytes;
}


////////////////////////////////////////////////////////////////////////
// SetFileExtensionMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
char* SetFileExtensionMT(char* pszFilename, const char* pszExtension)
{
	if (!pszFilename || !pszExtension)
		return (pszFilename);

	char* p = strrchr(pszFilename, chrSLASH);
	if (!p)
		p = pszFilename;
	if ((p = strrchr(p, '.')) != NULL)
		p[1] = 0;
	else
		strcat(pszFilename, ".");

	if (*pszExtension == '.')
		pszExtension++;

	return (strcat(pszFilename, pszExtension));
}


////////////////////////////////////////////////////////////////////////
// StripIllegalShortMT [static]
//
////////////////////////////////////////////////////////////////////////
static char* StripIllegalShortMT(char* pszFilename)
{
	//
	// Read-only, therefore this is thread safe.
	//
	static const char s_szIllegalCharShort[] = ". \t\r\n\"*+,/:;<=>?[\\]|";

    if (!pszFilename)
        return (NULL);

    char* s;
    char* t;
    
    s = t = pszFilename;
    while (1)
    {
        int len = strcspn(s, s_szIllegalCharShort);
        if (len)
        {
            strncpy(t, s, len);
            t += len;
        }
        s += len;
        if (!*s)
            break;
        s++;
    }
    *t = 0;
    
    return (pszFilename);
}


////////////////////////////////////////////////////////////////////////
// StripIllegalMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
char* StripIllegalMT(char* pszFilename, const CString& strTargetDirectory)
{
    char*	szSrc;
    char*	szDest;

    if( !pszFilename )
	{
        return NULL;
	}
    
	if( !::LongFileSupportMT( strTargetDirectory ) )
	{
		return ::StripIllegalShortMT( pszFilename );
	}

    szSrc = szDest = pszFilename;
    
	while( *szSrc )
    {
		if( ( *szSrc == 32 ) || 
			( *szSrc > 127 ) ||
			( ( *szSrc >= 'a' ) && ( *szSrc <= 'z' ) ) ||
			( ( *szSrc >= 'A' ) && ( *szSrc <= 'Z' ) ) ||
			( ( *szSrc >= '0' ) && ( *szSrc <= '9' ) ) ||
			( strchr( "$%'-_@~`!(){}^#&+,;=[]", *szSrc ) != NULL ) )
		{
			*szDest = *szSrc;
			szDest++;
		}				
		szSrc++;
	}
    
	*szDest = 0;
    
    return (pszFilename);
}


////////////////////////////////////////////////////////////////////////
// FileExistsMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
BOOL FileExistsMT(const char* pszFilename, BOOL IsWritable /*= FALSE*/)
{
	if (!pszFilename || !*pszFilename)
	{
		ASSERT(0);
		return FALSE;
	}

	if (::access(pszFilename, IsWritable? 2 : 0) == 0)
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// CreateDirectoryMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
BOOL CreateDirectoryMT(LPCTSTR DirectoryName)
{
	return FileExistsMT(DirectoryName, TRUE) || CreateDirectory(DirectoryName, NULL);
}

////////////////////////////////////////////////////////////////////////
// CreateShortcutMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
HRESULT CreateShortcutMT(LPCTSTR Target, LPCTSTR Link, LPCTSTR Arguments /*= NULL*/, LPCTSTR WorkingDir /*= NULL*/)
{
	HRESULT hr;
	IShellLink* piSL;

	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
							(LPVOID*)&piSL);
	if (SUCCEEDED(hr))
	{
		piSL->SetPath(Target);
		if (Arguments)
			piSL->SetArguments(Arguments);
		if (WorkingDir)
			piSL->SetWorkingDirectory(WorkingDir);

		IPersistFile* piPF;
		hr = piSL->QueryInterface(IID_IPersistFile, (LPVOID*)&piPF);

		if (SUCCEEDED(hr))
		{
			CString FullLink(Link);

			// If Link ends in backslash, that means it is a directory that
			// should hold the link and use the name of the target file
			if (FullLink[FullLink.GetLength() - 1] == '\\')
				FullLink += _tcsrchr(Target, '\\') + 1;

			CString Temp = FullLink;
			Temp.MakeLower();
			if (Temp.Find(".lnk") == -1)
				FullLink += ".lnk";
			WORD wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, FullLink, -1, wsz, MAX_PATH);

			hr = piPF->Save(wsz, TRUE);

			piPF->Release();
		}
		piSL->Release();
	}

	return hr;
}


void
GetFileNameAndParentPath(
	const char *			in_szFullPath,
	char *					out_szParentPathBuffer,
	int						in_nParentPathBufferSize,
	CString *				out_szFileName)
{
	// Get the full path to the parent directory for the old file name
	LPTSTR		lpszFileName;
	GetFullPathName(in_szFullPath, in_nParentPathBufferSize, out_szParentPathBuffer, &lpszFileName);

	// If the caller wanted the file name copy it before NULL terminating
	if (out_szFileName)
		*out_szFileName = lpszFileName;
	
	// NULL terminate so that in_nParentPathBufferSize is just the path
	*lpszFileName = '\0';
}


HRESULT
MoveFileMaintainingCorrectPermissions(
	const char *			in_szOldName,
	const char *			in_szNewName,
	DWORD					in_dwFileSize,
	DWORD					in_dwFreeSpaceDestinationVolume)
{
	// Start by assuming failure
	HRESULT		hr = MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_RENAME);

	// Get the full path to the parent directory for the old file name
	TCHAR		szOldFilePath[_MAX_PATH];
	GetFileNameAndParentPath(in_szOldName, szOldFilePath, _MAX_PATH);

	// Get the full path to the parent directory for the new file name
	TCHAR		szNewFilePath[_MAX_PATH];
	GetFileNameAndParentPath(in_szNewName, szNewFilePath, _MAX_PATH);

	// Determine if they're inside the same directory
	bool		bInsideSameDirectory = (stricmp(szOldFilePath, szNewFilePath) == 0);
	if (!bInsideSameDirectory)
	{
		// Pad the file size by the minimum free bytes that we'd like to avoid
		// falling below.
		in_dwFileSize += kMinExtraFreeSpace;
		
		if (in_dwFileSize < in_dwFreeSpaceDestinationVolume)
		{
			// In Windows 2000 and XP renaming a file that we created in the temp folder
			// to move it into another folder is not ideal. The file created in the temp
			// folder is considered private to the current user - no other users have
			// rights to files created in that folder. Renaming the file to move it
			// into the proper Eudora directory will not change the default permissions
			// to reflect the new parent directory.
			//
			// We'll attempt to copy the file to the new name. This is slower, but it's
			// the only way to make sure that the default permissions are set up correctly.
			if ( ::CopyFile(in_szOldName, in_szNewName, TRUE) )
			{
				// CopyFile worked.
				hr = S_OK;
				
				// Remove the original.
				FileRemoveMT(in_szOldName);
			}
			else
			{
				SetLastErrorResult(&hr);
			}
		}
	}

	if ( FAILED(hr) )
	{
		// Either inside the same directory (so permissions should already
		// correct) or failed to copy the file (perhaps because there was
		// too little space).
		if ( ::MoveFile(in_szOldName, in_szNewName) )
		{
			// MoveFile worked.
			hr = S_OK;
		}
		else
		{
			SetLastErrorResult(&hr);
		}
	}

	return hr;
}

void
SetLastErrorResult(
	HRESULT* phr)
{
	HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
	if (FAILED(hr))
		*phr = hr;
}


////////////////////////////////////////////////////////////////////////
// FileRenameMT [extern, exported]
//
// Deprecated. Eventually remove in favor of FileRenameReplaceMT.
////////////////////////////////////////////////////////////////////////
HRESULT FileRenameMT(const char* pszOldName, const char* pszNewName)
{
	HRESULT hr = MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_RENAME);

	if (!pszOldName || !pszNewName)
		ASSERT(0);
	else if (!::MoveFile(pszOldName, pszNewName))
		SetLastErrorResult(&hr);
	else
		hr = S_OK;

	return hr;
}


////////////////////////////////////////////////////////////////////////
// FileRenameReplaceMT [extern, exported]
//
// Renames file from old file name to new file name, optionally
// replacing existing file.
//
// Key features:
// * When not replacing (i.e. when being used like old FileRename),
//	 FileRenameReplaceMT maintains default file attributes and
//	 permissions by copying the file into the destination directory
//	 (when necessary - not done when already in same directory).
// * When replacing correctly maintains all original file information
//	 when possible (i.e. creation date, attributes, permissions)
// * Maintains original file if rename/replace fails
////////////////////////////////////////////////////////////////////////
HRESULT FileRenameReplaceMT(const char * in_szOldName, const char * in_szNewName, BOOL in_bReplace /*= TRUE*/)
{
	if (!in_szOldName || !in_szNewName)
	{
		ASSERT(0);
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_RENAME);
	}	
	
	HRESULT			hr = S_OK;

	// Get the file size and the volume size so that we can make smart
	// decisions about whether or not we should even try to copy the file.
	CFileStatus		fileStatus;

	CFile::GetStatus(in_szOldName, fileStatus);

	// Statics
	static bool						s_bInitializedReplaceFile = false;
	static IndirectReplaceFile		s_pReplaceFile = NULL;
	
	DWORD			dwFileSize = static_cast<DWORD>( fileStatus.m_size );
	DWORD			dwFreeSpaceDestinationVolume = GetAvailableSpaceMT(in_szNewName);

	// Check to see if replacing is really needed
	if ( in_bReplace && !FileExistsMT(in_szNewName) )
		in_bReplace = FALSE;

	if (in_bReplace && !s_bInitializedReplaceFile)
	{
		// First time replacing a file. Attempt to get the newfangled ReplaceFile API,
		// which is only available in Windows 2000 and later.
		s_bInitializedReplaceFile = true;

		// Check to see if Kernel32.dll is available
		HMODULE		hKernel32 = GetModuleHandleA("KERNEL32");
		if (hKernel32)
		{
			// Get the ReplaceFile function from Kernel32.dll (available in Windows 2000 and later).
			s_pReplaceFile = reinterpret_cast<IndirectReplaceFile>( GetProcAddress(hKernel32, "ReplaceFile") );
		}
	}

	if (!in_bReplace)
	{
		// Not replacing file - just attempt to rename the file.
		hr = MoveFileMaintainingCorrectPermissions(in_szOldName, in_szNewName, dwFileSize, dwFreeSpaceDestinationVolume);
	}
	else
	{
		BOOL		bWorked = FALSE;

		// Break the new file name full path into path and name
		TCHAR		szNewFilePath[_MAX_PATH];
		CString		strNewFileName;
		GetFileNameAndParentPath(in_szNewName, szNewFilePath, _MAX_PATH, &strNewFileName);

		// Get a temp file name that looks like New<something>.tmp
		char		szTempPath[_MAX_PATH];
		hr = GetTempFileName(szNewFilePath, strNewFileName, 0, szTempPath);
		if ( FAILED(hr) )
			return hr;

		// Keep track of whether or not we copied the file to the destination
		// volume in order to be able to use ReplaceFile
		bool			bCopiedFileToDestVolume = false;
		char			szDestVolumeTempFile[_MAX_PATH];
		const char *	szOldName = in_szOldName;
		
		// Do we have the newfangled ReplaceFile API?
		if (s_pReplaceFile)
		{
			bool		bSameVolume = (strnicmp(in_szNewName, in_szOldName, 2) == 0);

			// Pad the file size by the minimum free bytes that we'd like to avoid
			// falling below.
			dwFileSize += kMinExtraFreeSpace;

			if ( !bSameVolume && (dwFileSize < dwFreeSpaceDestinationVolume) )
			{
				// We're on a different volume. ReplaceFile won't work across volumes.
				// Try copying to the new volume first, because we'd really prefer for
				// ReplaceFile to work. It preserves more of the file information.

				// Get a temp file name that looks like New<something>.tmp
				hr = GetTempFileName(szNewFilePath, strNewFileName, 0, szDestVolumeTempFile);
				if ( SUCCEEDED(hr) )
				{
					// Attempt to copy the file to the correct volume
					if ( ::CopyFile(in_szOldName, szDestVolumeTempFile, FALSE) )
					{
						// We succeeded in copying the file to the new volume.
						// The temporary copy of the file is now the source for all
						// future operations.
						szOldName = szDestVolumeTempFile;

						// Remember that we made a temporary copy of the file
						bCopiedFileToDestVolume = true;

						// The source and destination are now on the same volume
						bSameVolume = true;
					}
				}
			}

			// Remove the file that GetTempFileName created (we kept it around
			// until now so that our second call to GetTempFileName would get
			// a different file name).
			// Remove now before we call ReplaceFile so that ReplaceFile can
			// create a backup copy of the original file.
			FileRemoveMT(szTempPath);
			
			if (bSameVolume)
			{
				USES_CONVERSION;
				
				// Try the newfangled ReplaceFile API.
				bWorked = s_pReplaceFile( T2W(in_szNewName), T2W(szOldName), T2W(szTempPath),
										  REPLACEFILE_WRITE_THROUGH | REPLACEFILE_IGNORE_MERGE_ERRORS,
										  NULL, NULL );

				// If it worked, then we no longer need the backup copy of the file
				// that ReplaceFile created.
				if (bWorked)
					FileRemoveMT(szTempPath);
			}
		}
		else
		{
			// Remove the file that GetTempFileName created (we kept it around
			// until now so that our second call to GetTempFileName would get
			// a different file name).
			FileRemoveMT(szTempPath);
		}

		// Start over with our error code (either ReplaceFile worked or we'll get
		// an error code when we do the old method of replacing a file below).
		hr = S_OK;

		if (!bWorked)
		{
			// We're either running on an older version of Windows, or the newfangled
			// ReplaceFile API failed us. Revert to the old method of replacing a file.

			// Here's the plan for a safe rename:
			//
			// 1. Rename New to New<something>.tmp
			// 2. Rename Old to New
			// 3. Delete New<something>.tmp
			//
			// That way if something goes wrong, we have a copy of the file
			// that's going to get blown away as a result of the rename, and
			// we avoid the case where we wind up with neither the old nor
			// new version of the file.

			// 1. Rename New to New<something>.tmp (if it still exists)
			if ( !FileExistsMT(in_szNewName) )
			{
				// We already checked to see if the file we're replacing existed
				// at all before doing this code. The only reason why the file
				// should be missing at this point is if ReplaceFile already
				// renamed the file we're replacing. Verify that this is the case
				// and that we're still replacing.
				if ( !FileExistsMT(szTempPath) )
				{
					ASSERT(0);
					in_bReplace = FALSE;
				}
			}
			else
			{
				// Rename New to New<something>.tmp
				hr = FileRenameMT(in_szNewName, szTempPath);
				if ( FAILED(hr) )
					return hr;
			}

			// 2. Rename Old to New
			if ( SUCCEEDED(hr) )
			{
				// Try to move the file twice
				for (int i = 0; i < 2; i++)
				{
					// Attempt to move the file to the new name, while maintaining
					// correct permissions.
					hr = MoveFileMaintainingCorrectPermissions(szOldName, in_szNewName, dwFileSize, dwFreeSpaceDestinationVolume);

					// If we succeeded, we don't need to try again
					if ( SUCCEEDED(hr) )
						break;

					if (i == 0)
					{
						// Failed to rename Old to New, maybe New still existed
						// (i.e. rename of New to New<something> hasn't completed)?
						// If so, then deleting New and trying rename again may work.
						FileRemoveMT(in_szNewName);
					}
				}
			}

			if (in_bReplace)
			{
				if ( SUCCEEDED(hr) )
				{
					// 3. Delete New<something>.tmp
					// Rename went ok, so get rid of the copy of the file being replaced.
					FileRemoveMT(szTempPath);
				}
				else	//	FAILED(hr)
				{
					// Uh oh, failed to rename after two attempts, restore the destination file now
					FileRenameMT(szTempPath, in_szNewName);
				}
			}
		}

		if (bCopiedFileToDestVolume)
		{
			// in_szOldName and in_szNewName specified different volumes.
			// We copied the file to the destination volume before calling ReplaceFile
			// (because ReplaceFile only works on the same volume). Even if ReplaceFile
			// failed, we kept the temporary copy around so that the file would not need
			// to be copied from one volume to the other again.			
			if ( SUCCEEDED(hr) )
			{
				// We succeeded, so we need to delete the original file now.
				FileRemoveMT(in_szOldName);
			}
			else	//	FAILED(hr)
			{
				// We failed, so we need to delete the temporary copy we made to
				// the destination volume.
				FileRemoveMT(szDestVolumeTempFile);
			}
		}
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
// FileRemoveMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
HRESULT FileRemoveMT(const char* pszFilename)
{
	HRESULT hr = MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_DELETE);

	if (!pszFilename || !*pszFilename)
		ASSERT(0);
	else if (::FileExistsMT(pszFilename) && ::remove(pszFilename) == -1)
		SetLastErrorResult(&hr);
	else
		hr = S_OK;

	return hr;
}


////////////////////////////////////////////////////////////////////////
// StripQuotesMT [extern, exported]
//
// Strip off quotes surrounding long filenames
////////////////////////////////////////////////////////////////////////
void StripQuotesMT(char *pszString)
{
	if (!(*pszString))
		return;

	char *pszStart = pszString;
	if (*pszStart == '\"')
		pszStart++;
	char *c = ((char *)pszStart + strlen(pszStart) - 1);
	if (*c == '\"')
		*c = 0;

	::strcpy(pszString, pszStart);
}


////////////////////////////////////////////////////////////////////////
// LongFileSupportMT [extern, exported]
//
// This does all the dirty work to see if we can use long filenames
////////////////////////////////////////////////////////////////////////
BOOL LongFileSupportMT(const char *pszPathname)
{
	char szDrive[_MAX_PATH];

	if (::strlen(pszPathname) < 3)
		return FALSE;

	if (pszPathname[1] == ':')
	{
		szDrive[0] = *pszPathname;
		szDrive[1] = ':';
		szDrive[2] = '\\';
		szDrive[3] = 0;
	}
	else if (pszPathname[0] == '\\' && pszPathname[1] == '\\')
	{
		const char* backslash;

		if ((backslash = strchr(pszPathname + 2, '\\')) != NULL &&
			(backslash = strchr(backslash + 1, '\\')) != NULL)
		{
			const int len = backslash - pszPathname + 1;
			strncpy(szDrive, pszPathname, len);
			szDrive[len] = 0;
		}
	}
	else
		return FALSE;

	char szVolumeName[_MAX_PATH];
	char szFileSys[255];
	DWORD dwMaxFileName = 0;
	DWORD dwFlags = 0;
	BOOL bSuccess = ::GetVolumeInformation(szDrive, szVolumeName, _MAX_PATH, NULL,
									&dwMaxFileName, &dwFlags, szFileSys, 255);

	if (bSuccess && dwMaxFileName > 12)
		return TRUE;
	else
		return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetTmpFileNameMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
CString GetTmpFileNameMT(const char *pszPrefix /*="eud"*/)
{
	char szPath[_MAX_PATH];				// pathname to TMP directory
	::GetTempPath(_MAX_PATH, szPath);

	char szTempFile[_MAX_PATH];			// create new temp file in TMP directory
	::GetTempFileName(szPath, pszPrefix, 0, szTempFile);

	return CString(szTempFile);
}

LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata,data);
        RegCloseKey(hkey);
    }

    return retval;
}

HINSTANCE GotoURL(LPCTSTR url, int showcmd)
{
    TCHAR key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) {

        if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
            lstrcat(key, _T("\\shell\\open\\command"));

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
                TCHAR *pos;
                pos = _tcsstr(key, _T("\"%1\""));
                if (pos == NULL) {                     // No quotes found
                    pos = strstr(key, _T("%1"));       // Check for %1, without quotes 
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcat(pos, _T(" "));
                lstrcat(pos, url);
                result = (HINSTANCE) WinExec(key,showcmd);
            }
        }
    }

    return result;
}

// Routines to find if a specific process is executing currently
// The reason why functions to find modules are implemented in two ways 
// is because ToolHelp32 way would work 'only' under Win95/98
// and PSAPI.DLL is by default present in NT 3.* and 4.* 

// Created: 01/05/2000, Apul Nahata

// Local Error Codes
#define _FP_SUCCESS						TRUE
#define _FP_ERROR_BASE					8000
#define _FP_SYSTEM_DLL_NOT_FOUND		_FP_ERROR_BASE + 1
#define _FP_QUERIED_MODULE_NOT_FOUND	_FP_ERROR_BASE + 2
#define _FP_GENERIC_ERROR				_FP_ERROR_BASE + 3

// NETSCAPE's Most Significant 32 bit value for version 4.05
#define _NETSCAPE_405_PRODUCT_VERSION_MS_	262149 // Hex of 262149 = 40005 i.e 4.05

// Function pointers for ToolHelp32 functions
typedef HANDLE (WINAPI * PFNCREATETOOLHELP32SNAPSHOT)(DWORD dwFlags, DWORD dwth32ProcessID);  
typedef BOOL (WINAPI * PFNPROCESS32FIRST)(HANDLE hSnapshot, LPPROCESSENTRY32 lpProcessEntry32);    
typedef BOOL (WINAPI * PFNPROCESS32NEXT)(HANDLE hSnapshot, LPPROCESSENTRY32 lpProcessEntry32);    
typedef BOOL (WINAPI * PFNMODULE32FIRST)(HANDLE hSnapshot, LPMODULEENTRY32 lpModuleEntry32);    
typedef BOOL (WINAPI * PFNMODULE32NEXT)(HANDLE hSnapshot, LPMODULEENTRY32 lpModuleEntry32);


// Function pointers for PSAPI.DLL functions 
typedef BOOL (WINAPI * PFNENUMPROCESSES)(DWORD *lpidProcess,
										 DWORD dwCb,
										 DWORD *dwCbNeeded);

typedef BOOL (WINAPI * PFNENUMPROCESSMODULES)(HANDLE hProcess,
											  HMODULE *lphModule,
											  DWORD dwCb,
											  LPDWORD lpcbNeeded);

typedef DWORD (WINAPI * PFNGETMODULEFILENAMEEXA)(HANDLE hProcess,
												 HMODULE hModule,
												 LPSTR lpFilename,
												 DWORD dwSize);

//Find if the module is loaded using ToolHelp32
//DWORD FindifExecutingUsingToolHelp32(LPCTSTR lpszModuleName)
DWORD FindifExecutingUsingToolHelp32(LPCTSTR lpszModuleName,DWORD *dwProductVersionMS = NULL, DWORD *dwProductVersionLS = NULL)
{
    static HMODULE hModKERNEL32 = 0;
    static PFNCREATETOOLHELP32SNAPSHOT pfnCreateToolhelp32Snapshot = 0;
    static PFNPROCESS32FIRST pfnProcess32First = 0;
    static PFNPROCESS32NEXT pfnProcess32Next = 0;
    static PFNMODULE32FIRST pfnModule32First = 0;
    static PFNMODULE32NEXT pfnModule32Next = 0;

    // Hook up to the ToolHelp32 functions dynamically.
    
    if ( NULL == hModKERNEL32 )
        hModKERNEL32 = GetModuleHandle( "KERNEL32.DLL" );

    pfnCreateToolhelp32Snapshot = (PFNCREATETOOLHELP32SNAPSHOT)
        GetProcAddress( hModKERNEL32, "CreateToolhelp32Snapshot" );
        
    pfnProcess32First = (PFNPROCESS32FIRST)
        GetProcAddress( hModKERNEL32, "Process32First" );
        
    pfnProcess32Next = (PFNPROCESS32NEXT)
        GetProcAddress( hModKERNEL32, "Process32Next" );
        
    pfnModule32First = (PFNMODULE32FIRST)
        GetProcAddress( hModKERNEL32, "Module32First" );
        
    pfnModule32Next = (PFNMODULE32NEXT)
        GetProcAddress( hModKERNEL32, "Module32Next" );
        
    if ( (NULL == pfnCreateToolhelp32Snapshot) ||  
		 (NULL == pfnProcess32First) || 
		 (NULL == pfnProcess32Next) ||  
		 (NULL == pfnModule32First) || 
		 (NULL == pfnModule32Next) )
        return _FP_SYSTEM_DLL_NOT_FOUND;


    // Create a ToolHelp32 snapshot containing the process list    
    HANDLE hSnapshotProcess;
    hSnapshotProcess = pfnCreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if ( NULL == hSnapshotProcess )
        return _FP_GENERIC_ERROR;

    // Iterate through each of the processes in the snapshot
    PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
    BOOL bProcessWalkContinue;
        
    for (bProcessWalkContinue = pfnProcess32First(hSnapshotProcess,&processEntry);
         bProcessWalkContinue;
         bProcessWalkContinue = pfnProcess32Next(hSnapshotProcess,&processEntry) )
    {
        
        // Enumerate the module list for this process.
        
        HANDLE hSnapshotModule;
        hSnapshotModule = pfnCreateToolhelp32Snapshot( TH32CS_SNAPMODULE,
                                                    processEntry.th32ProcessID );
        if ( NULL == hSnapshotModule )
            continue;

        // Iterate through each module in the snapshot                                                  
        MODULEENTRY32 moduleEntry = { sizeof(MODULEENTRY32) };
        BOOL bModuleWalkContinue;

        for (bModuleWalkContinue = pfnModule32First(hSnapshotModule,&moduleEntry);
             bModuleWalkContinue;
             bModuleWalkContinue = pfnModule32Next(hSnapshotModule,&moduleEntry) )
        {
            // Hack alert!  not so intelligent way to figure out if this is EXE module itself
            if ( 0 == _tcsicmp( moduleEntry.szExePath, processEntry.szExeFile ) )
			{
				// Compare to find the actual module.exe here
				if (NULL != _tcsstr(_tcslwr(moduleEntry.szExePath),_tcslwr((TCHAR *)lpszModuleName)))
				{
					DWORD dwHandle = 0;
					DWORD dwSize = ::GetFileVersionInfoSize((TCHAR *)(const TCHAR *) moduleEntry.szExePath, &dwHandle);
					if (NULL == dwSize)
						return _FP_GENERIC_ERROR;		// EXE doesn't have VERSIONINFO data?

					
					// Allocate data buffers of the proper sizes.						
					LPSTR pVerData = DEBUG_NEW_NOTHROW TCHAR[dwSize];
					if (NULL == pVerData)
						return _FP_GENERIC_ERROR;
											
					// Fetching the actual VERSIONINFO data.						
					if (! ::GetFileVersionInfo((TCHAR *)(const TCHAR *) moduleEntry.szExePath, dwHandle, dwSize, pVerData))
						
					{
						ASSERT(0);		// missing VERSIONINFO data?							
						delete [] pVerData;
						return _FP_GENERIC_ERROR;
					}

					LPSTR pVsFixedFileInfoData = NULL;
					if (NULL == (pVsFixedFileInfoData = DEBUG_NEW_NOTHROW TCHAR [sizeof(VS_FIXEDFILEINFO)] ) )
					{
						delete [] pVerData;
						return _FP_GENERIC_ERROR;
					}

					UINT bufsize = 0;

					TCHAR sztmpStr[5];
					_tcscpy(sztmpStr, "\\");
					if (::VerQueryValue(pVerData,
										sztmpStr, 
										(void **) &pVsFixedFileInfoData, 
										&bufsize))
					{
						if (pVsFixedFileInfoData && bufsize)
						{
							VS_FIXEDFILEINFO* p_info = (VS_FIXEDFILEINFO *) pVsFixedFileInfoData;
							*dwProductVersionMS = p_info->dwProductVersionMS;
							*dwProductVersionLS = p_info->dwProductVersionLS;
						}
					}
					
					delete [] pVerData;
					return _FP_SUCCESS;				
				}
                
			}            
        }
        
        CloseHandle( hSnapshotModule ); // Done with module list snapshot
    }
    
    CloseHandle( hSnapshotProcess );    // Done with process list snapshot
    
    return _FP_QUERIED_MODULE_NOT_FOUND;
}

//Find if the module is loaded using PSAPI 
//DWORD FindifExecutingUsingPSAPI(LPCTSTR lpszModuleName)
DWORD FindifExecutingUsingPSAPI(LPCTSTR lpszModuleName,DWORD *dwProductVersionMS = NULL, DWORD *dwProductVersionLS = NULL)
{
    static HMODULE hModPSAPI = 0;
    static PFNENUMPROCESSES pfnEnumProcesses = 0;
    static PFNENUMPROCESSMODULES pfnEnumProcessModules = 0;
    static PFNGETMODULEFILENAMEEXA pfnGetModuleFileNameExA = 0;

    // Hook up to the functions in PSAPI.DLL dynamically.      
    if ( NULL == hModPSAPI )
        hModPSAPI = LoadLibrary( "PSAPI.DLL" );

    if ( NULL == hModPSAPI )
        return _FP_SYSTEM_DLL_NOT_FOUND;
        
    pfnEnumProcesses = (PFNENUMPROCESSES)
            GetProcAddress( hModPSAPI,"EnumProcesses" );

    pfnEnumProcessModules = (PFNENUMPROCESSMODULES)
            GetProcAddress( hModPSAPI, "EnumProcessModules" );

    pfnGetModuleFileNameExA = (PFNGETMODULEFILENAMEEXA)
            GetProcAddress( hModPSAPI, "GetModuleFileNameExA" );

    if ( (NULL == pfnEnumProcesses) ||
		 (NULL == pfnEnumProcessModules) ||
		 (NULL == pfnGetModuleFileNameExA) )
        return _FP_SYSTEM_DLL_NOT_FOUND;
    
    // Successfully hooked upto PSAPI.DLL functions
    DWORD pidArray[1024];
    DWORD cbNeeded;
    DWORD nProcesses;
            
    // EnumProcesses returns an array of process IDs
    if ( NULL == pfnEnumProcesses(pidArray, sizeof(pidArray), &cbNeeded) )
        return _FP_GENERIC_ERROR;

    nProcesses = cbNeeded / sizeof(DWORD);  // Determine number of processes
    
    // Iterate through each process in the array
    for ( unsigned i = 0; i < nProcesses; i++ )
    {
        HMODULE hModuleArray[1024];
        HANDLE hProcess;
        DWORD pid = pidArray[i];
        DWORD nModules;
        
        // Use the process ID to open up a handle to the process
        hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                PROCESS_VM_READ,
                                FALSE, pid );
        if ( NULL == hProcess )
            continue;
            
        // EnumProcessModules returns an array of HMODULEs for the process
        if ( NULL == pfnEnumProcessModules(hProcess, hModuleArray,
                                    sizeof(hModuleArray), &cbNeeded ) )
        {
            CloseHandle( hProcess );
            continue;
        }

        // Calculate number of modules in the process                                   
        nModules = cbNeeded / sizeof(hModuleArray[0]);

        // Iterate through each of the process's modules
        for ( unsigned j=0; j < nModules; j++ )
        {
            HMODULE hModule = hModuleArray[j];
            TCHAR szModuleName[MAX_PATH];

            pfnGetModuleFileNameExA(hProcess, hModule,
                                    szModuleName, sizeof(szModuleName) );

            if ( 0 == j )   // The first module is always the EXE
            {
                
				if (_tcsstr(_tcslwr(szModuleName),_tcslwr((TCHAR *)lpszModuleName)))
				{
					DWORD dwHandle = 0;
					DWORD dwSize = ::GetFileVersionInfoSize((TCHAR *)(const TCHAR *) szModuleName, &dwHandle);
					if (NULL == dwSize)
						return _FP_GENERIC_ERROR;		// EXE doesn't have VERSIONINFO data?

					
					// Allocate data buffers of the proper sizes.						
					LPSTR pVerData = DEBUG_NEW_NOTHROW TCHAR[dwSize];
					if (NULL == pVerData)
						return _FP_GENERIC_ERROR;
											
					// Fetching the actual VERSIONINFO data.						
					if (! ::GetFileVersionInfo((TCHAR *)(const TCHAR *) szModuleName, dwHandle, dwSize, pVerData))
						
					{
						ASSERT(0);		// missing VERSIONINFO data?							
						delete [] pVerData;
						return _FP_GENERIC_ERROR;
					}

					LPSTR pVsFixedFileInfoData = NULL;
					UINT bufsize = 0;
					TCHAR sztmpStr[5];
					_tcscpy(sztmpStr, "\\");
					if (::VerQueryValue(pVerData,
										sztmpStr, 
										(void **) &pVsFixedFileInfoData, 
										&bufsize))
					{
						if (pVsFixedFileInfoData && bufsize)
						{
							VS_FIXEDFILEINFO* p_info = (VS_FIXEDFILEINFO *) pVsFixedFileInfoData;
							*dwProductVersionMS = p_info->dwProductVersionMS;
							*dwProductVersionLS = p_info->dwProductVersionLS;
						}
					}
					
					delete [] pVerData;
					return _FP_SUCCESS;	
				}
		
            }            
        }
    
        CloseHandle( hProcess );    // Closing the process handle
    }
    
    return _FP_QUERIED_MODULE_NOT_FOUND;
}

//BOOL IsCurrentlyExecuting(LPCTSTR lpszModuleName)
BOOL IsCurrentlyExecuting(LPCTSTR lpszModuleName, DWORD *dwProductVersionMS, DWORD *dwProductVersionLS)
{
	DWORD dwReturn;
	
	dwReturn = FindifExecutingUsingToolHelp32(lpszModuleName,dwProductVersionMS, dwProductVersionLS);

	if ( _FP_SYSTEM_DLL_NOT_FOUND == dwReturn)
    {
        // ToolHelp32 was perhaps not present.Let's try PSAPI.DLL 
        dwReturn = FindifExecutingUsingPSAPI(lpszModuleName,dwProductVersionMS, dwProductVersionLS);        
    }
	
	return ((dwReturn == _FP_SUCCESS) ? TRUE : FALSE);	
}


// Function to check if netscape is executing & if so is the version greater than or equal to 4.05
// Actually we are not really interested in knowing if it is executing or not. What matters FOR NOW is the version (if at all executing)
BOOL IsNetscapeAbove405Executing()
{
	DWORD dwProductVersionMS = 0, dwProductVersionLS = 0;

	// See if Netscape is Executing
	if (_FP_SUCCESS == IsCurrentlyExecuting("netscape.exe", &dwProductVersionMS, &dwProductVersionLS))
	{
		// Check if the version that is executing is 4.05 or above
		if (dwProductVersionMS >= _NETSCAPE_405_PRODUCT_VERSION_MS_)
		{
			return TRUE;
		}
	}
	return FALSE;
}

char* StripAddressMT(char* line)
{
	if (!line)
		return (NULL);

	// Remove parenthesized comments
	char *p = line;
	char *t = line;
	int InParen = 0;
	int InQuote = 0;
	char Last = 0;
	for (; *p; p++)
	{
		if (Last == '\\')
		{
			if (*p == '\\')
				Last = 0;
			else
				Last = *p;
		}
		else
		{
			Last = *p;
			if (!InQuote && *p == '(')
			{
				InParen++;
				continue;
			}
			else if (InParen && *p == ')')
			{
				InParen--;
				continue;
			}
			else if (*p == '"')
				InQuote = !InQuote;
		}
		if (!InParen)
			*t++ = *p;
	}
	*t = 0;

	const char* BegAngle = strchr(line, '<');
	char* EndAngle = BegAngle? strchr(BegAngle + 1, '>') : NULL;
	if (BegAngle && EndAngle && BegAngle < EndAngle)
	{
		*EndAngle = 0;
		strcpy(line, BegAngle + 1);
	}

	return (::TrimWhitespaceMT(line));
}

// This function parses the RFC822 date string into Time Zone (in Minutes) & 
// the actual time (in seconds)
// Note : This has been moved here from Summary.cpp
void FormatDateMT(const char* GMTOffset, long &lSeconds, int &nTimeZoneMinutes)
{
	char GMTbuf[128];

	if (GMTOffset)
	{
		strncpy(GMTbuf, GMTOffset, sizeof(GMTbuf) - 1);
		GMTbuf[sizeof(GMTbuf) - 1] = 0;

		// StripAddress() will strip out () comments
		StripAddressMT(GMTbuf);

		GMTOffset = strrchr(GMTbuf, ' ');
		if (GMTOffset && GMTOffset[1] != '-' && GMTOffset[1] != '+' &&
			!isalpha((int)(unsigned char)(GMTOffset[1])))
		{
			GMTOffset = NULL;
		}
	}

	// Store seconds as GMT time so sorts by date occur correctly
	if (GMTOffset)
	{
		GMTOffset++;
		if (!isdigit((int)(unsigned char)*GMTOffset) && *GMTOffset != '-' && *GMTOffset != '+')
		{
			// Do we have a timezone abbreviation for it
			int len = strlen(GMTOffset);
			CString ZoneInfo;
			for (UINT i = IDS_TZ_FIRST; TRUE; i++)
			{
				if (!ZoneInfo.LoadString(i) || ZoneInfo.IsEmpty())
					break;
				if (!strnicmp(ZoneInfo, GMTOffset, len))
				{
					strcpy((char *)GMTOffset,(const char*)ZoneInfo);
					GMTOffset += len;
					while(!isdigit((int)(unsigned char)*GMTOffset) && *GMTOffset != '-' && *GMTOffset != '+')
						GMTOffset++;
					break;
				}
			}
		}
		int num = atoi(GMTOffset);

		// Check for bogus timezones.  Ignore if bad.
		if (num >= -2400 && num <= 2400)
		{
			nTimeZoneMinutes = num / 100 * 60;

			// Deal with minutes if there are some
			num %= 100;
			if (num)
				nTimeZoneMinutes += num;

			lSeconds -= nTimeZoneMinutes * 60;
		}
	}	
}

// ComposeDate
// Get the info together to compose an RFC-822 happy date stamp
//
int ComposeDateMT(char* DateBuf, unsigned long GMTTime, int TimeZoneMinutes, BOOL bEudorasSMTPFormat /* = TRUE */ )
{
	if (DateBuf)
	{
		*DateBuf = 0;

		// Date must be in the format of
		// "Date: <weekday> , <date> <month> <year> HH:MM:SS <gmt offset>"
		// where <gmt offset> = [+|-]HHMM for current time zone

		GMTTime += TimeZoneMinutes * 60;
		
		CTime Time = GMTTime;
		if (Time.GetTime() < 0)
			Time = 0;

		// We need to use the string resources that have English month and day of week abbreviations
		// because Date: headers should always be in English.  All other routines wind up using
		// strftime(), which localizes month and day of week names in to the current language.
		CRString Weekdays(IDS_WEEKDAYS);
		CRString Months(IDS_MONTHS);
		const char* ThisWeekday = ((LPCTSTR)Weekdays) + (Time.GetDayOfWeek() - 1) * 3;
		const char* ThisMonth = ((LPCTSTR)Months) + (Time.GetMonth() - 1) * 3;

		CString csDateFormat;
		csDateFormat.LoadString(IDS_SMTP_DATE_FORMAT);
						
		if (!bEudorasSMTPFormat)
		{
			int nMatchFound = -1;
			nMatchFound = csDateFormat.Find(':');
			if (nMatchFound != -1)
				csDateFormat = csDateFormat.Right(csDateFormat.GetLength() - nMatchFound - 1);
		}

		sprintf(DateBuf, csDateFormat,
							ThisWeekday,
							Time.GetDay(),
							ThisMonth,
							Time.GetYear(),
							Time.GetHour(),
							Time.GetMinute(),
							Time.GetSecond());
		::TimeDateStringFormatMT(DateBuf + strlen(DateBuf), 0, TimeZoneMinutes, " %4");

		return (TRUE);
	}
	else
		return FALSE;
}

//Date: %0.3s, %02d %0.3s %4d %02d:%02d:%02d

BOOL Weekday(const char* Line)
{
	ASSERT(Line);
	
	CRString Weekdays(IDS_WEEKDAYS);

	for (LPCTSTR w = Weekdays; *w; w += 3)
	{
		if (strnicmp(Line, w, 3) == 0)
			return TRUE;
	}

	return FALSE;
}


// GetTime
//
// If FromLine is TRUE and Line is not a valid Unix-style From line, then
// return 0L.  Otherwise, return the time given in the line, if parsable. 
// Here's a sample From line:
//   From beckley@qualcomm.com Thu Oct 15 16:15:08 1992 
//
long GetTimeMT(const char* Line, BOOL FromLine)
{
	// alternate from line
	//
	// Note that we're careful to specify the maximum width that we want for
	// the timezone string portion - using sscanf without this is begging for
	// a buffer overflow crash, which is exactly what we were seeing for certain
	// corrupt mailboxes before this was added.
	const char* AltFromScan1 = "%3s %d %d:%d 7%s %d";
	const char* AltFromScan2 = "%3s %d %d:%d:%d 7%s %d";
	char timeZone[8];

	struct tm time;
	char month[5];
	int mon;
	int fraction;

	if (!Line) return (0L);

	time.tm_year = time.tm_hour = time.tm_min = time.tm_sec = 0;
	if (FromLine)
	{
		// Check for "From "
		if (Line[0] != 'F' || Line[1] != 'r' || Line[2] != 'o' ||
			Line[3] != 'm' || Line[4] != ' ')
		{
			return (0L);
		}
		Line += 5;

		// Skip address
		while (*Line && (*Line == ' ' || *Line == '\t'))
			Line++;
		while (*Line && *Line != ' ' && *Line != '\t')
			Line++;
		while (*Line && (*Line == ' ' || *Line == '\t'))
			Line++;

		// Verify weekday
		if (!Weekday(Line))
			return (0L);

		// Skip the weekday
		while (*Line && *Line != ' ' && *Line != '\t')
			Line++;
		while (*Line && (*Line == ' ' || *Line == '\t'))
			Line++;

		if (!*Line)
			return (0L);

		// Read in rest of date
		if (sscanf(Line, CRString(IDS_SCAN_DATE_1), month, &time.tm_mday,
			&time.tm_hour, &time.tm_min, &time.tm_sec, &time.tm_year) != 6)
		{
			if (sscanf(Line, AltFromScan1, month, &time.tm_mday, &time.tm_hour,
				&time.tm_min, timeZone, &time.tm_year) != 6)
			{
				sscanf(Line, AltFromScan2, month, &time.tm_mday, &time.tm_hour,
					&time.tm_min, &time.tm_sec, timeZone, &time.tm_year);
			}
		}
	}
	else
	{
		while (*Line && !isdigit((int)(unsigned char)*Line))
			Line++;
		if (!*Line)
			return (0L);
		if ((sscanf(Line, CRString(IDS_SCAN_DATE_2), &time.tm_mday, month,
			&time.tm_year, &time.tm_hour, &time.tm_min, &time.tm_sec) < 5) &&
			(sscanf(Line, CRString(IDS_SCAN_DATE_3), &time.tm_mday, month,
			&time.tm_year, &time.tm_hour, &time.tm_min, &time.tm_sec, &fraction) < 7))
		{
			return (0L);
		}
	}

	// Find corresponding month number
	CRString Months(IDS_MONTHS);
	LPCTSTR MonthIndex = Months;
	mon = -1;
	for (int i = 0; *MonthIndex; MonthIndex += 3, i++)
	{
		if (strnicmp(month, MonthIndex, 3) == 0)
		{
			mon = i;
			break;
		}
	}
	if (mon < 0)
		return (0L);
	time.tm_mon = mon;

	// Get year as number of years past 1900
	if (time.tm_year < 70)
		time.tm_year += 100;
	else if (time.tm_year > 1900)
	{
		time.tm_year -= 1900;
		if (time.tm_year < 70)
			time.tm_year = 70;
	}

	time.tm_isdst = -1;
	time_t seconds = mktime(&time);

	// Ok, we've got this far.  This is probably a "From " line of some
	// variety, but the rest of it may not match up.  So if we get a bogus
	// time, still consider it a "From " line, but we don't know exactly what
	// date/time it represents, so just return 1.

	return (seconds < 0L? (FromLine? 1L : 0L) : seconds);
}


// Wrapper for functions needed to determine Disk Spun state
// In Win2000,Win98 & above GetDeviceState is implemented ( & works)
// In Win95 & above, GetSystemPowerStatus works (altho' the kernel has this in WinNT 4.0 too)

/*Excerpt from : http://msdn.microsoft.com/library/backgrnd/html/onnowapp.htm
Power Status Functions
Applications can get system power status to tune behavior. The best example of this is the possible work that the applications can do
to preserve battery life on mobile PCs. When a PC is running on batteries, the operating system will attempt to put the CPU into a low
power state and to spin down the hard disk whenever idle, because the processor and hard disk are two of the highest power devices
in mobile PCs. Applications that do not pay attention to the power state of the system can cause the hard disk to spin up frequently,
thus losing all savings gained from turning off the drive.

For a mobile PC, an application can track system status information and stop nonessential background tasks such as background
pagination. To prevent the hard disk from spinning up frequently, the application can postpone low-priority tasks while the PC is
running on battery power.

Applications can also get the power state of a device to optimize power consumption.*/

// This is the filename of a temporary file which is required, so that we have a valid handle to the file when calling GetDevicePowerState.
const TCHAR gszTempFileName[] = "DoNotDel.tmp";

class CShouldFileIOBeDone
{
private:	
	HANDLE	m_hFile;	// Handle of File
	
	// _MAX_PATH is defined as 255 & hence using 2048 instead

	TCHAR szFileName[_MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];	// Name of the File
	TCHAR szDirectory [2048 + sizeof(TCHAR)];	

	typedef BOOL (WINAPI * PFNGETDEVICEPOWERSTATE)(HANDLE hFile, BOOL *pState);

	// Function pointer for GetDevicePowerState(HANDLE, BOOL *)
	PFNGETDEVICEPOWERSTATE m_pfnGetDevicePowerState;

public :	
	CShouldFileIOBeDone();
	~CShouldFileIOBeDone()
	{
		if (m_hFile)
		{
			CloseHandle(m_hFile);
			remove(szFileName);	// delete the file
			m_hFile = NULL;
		}
	}

	void SetDirectory(LPCTSTR szDir);

	BOOL IsDiskSpun(BOOL &bSpunState);
	BOOL IsSystemOnBattery(BOOL &bOnBattery);
	BOOL ShouldFileIOBeDone(BOOL &bShouldFileIOBeDone);
};

// Golbal 
static CShouldFileIOBeDone gShouldFileIOBeDone;


CShouldFileIOBeDone::CShouldFileIOBeDone()
{ 
	m_hFile = NULL;
	*szDirectory = '\0';		
}

void CShouldFileIOBeDone::SetDirectory(LPCTSTR szDir)
{
	TCHAR szBuffer[4096];
	// Set the directory
	_tcscpy(szDirectory, szDir);

	static HMODULE hModKERNEL32 = NULL;
	m_pfnGetDevicePowerState = NULL;
	
	//try - compiler warned that try wasn't necessary
	//{	
		if ( NULL == hModKERNEL32 )
		{
			hModKERNEL32 = GetModuleHandle( "KERNEL32.DLL" );

			if (!m_pfnGetDevicePowerState)
				m_pfnGetDevicePowerState = (PFNGETDEVICEPOWERSTATE) GetProcAddress( hModKERNEL32, "GetDevicePowerState" );
		
			if(m_pfnGetDevicePowerState)
			{
				_tcscpy(szFileName,gszTempFileName);
				// Create/Open the file, which we will need to pass to GetDevicePowerState whenever requested
				if (0 == strlen(szDirectory))
				{
					// If there isn't a data directory specified by Eudora, then take the current directory
					GetCurrentDirectory(sizeof(szDirectory),szDirectory);
					strcat(szDirectory,"\\");
				}
				
				strcpy(szBuffer,szDirectory);
				// Eudora's data directory already consists of a trailing "\\"
				strcat(szBuffer,szFileName);
				m_hFile = CreateFile(szBuffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL); 
			}
		}
		
	//}
	//catch(...)
	//{
	//	m_pfnGetDevicePowerState = NULL;
	//}
}

BOOL CShouldFileIOBeDone::IsDiskSpun(BOOL &bSpunState)
{
	static HMODULE hModKERNEL32 = 0;
	static PFNGETDEVICEPOWERSTATE pfnGetDevicePowerState = NULL;

	try
	{	
		if (!m_pfnGetDevicePowerState)			// Not supported by kernel
			return FALSE;
		
		if (m_pfnGetDevicePowerState&& m_hFile)
			m_pfnGetDevicePowerState(m_hFile,&bSpunState);	// Call the GetDevicePowerState
		else
			return FALSE;	// Ideally this should not happen

		return TRUE;
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in CShouldFileIOBeDone::IsDiskSpun" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in CShouldFileIOBeDone::IsDiskSpun" );
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in CShouldFileIOBeDone::IsDiskSpun" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in CShouldFileIOBeDone::IsDiskSpun" );
	}

	// Something went wrong - return FALSE
	return FALSE;
}

BOOL CShouldFileIOBeDone::IsSystemOnBattery(BOOL &bOnBattery)
{
	SYSTEM_POWER_STATUS SystemPower;	
	if (GetSystemPowerStatus(&SystemPower))
	{		
		if (SystemPower.ACLineStatus == 0)	// offline
			bOnBattery = TRUE;
		else if (SystemPower.ACLineStatus == 1)	// online
			bOnBattery = FALSE;
		else 
			return FALSE;	// Unknown status
	}
	else
		return FALSE;	// Don't know ..probably because it's not supported by the kernel

	return TRUE;
}

BOOL CShouldFileIOBeDone::ShouldFileIOBeDone(BOOL &bShouldFileIOBeDone)
{
	BOOL bSpunState = FALSE, bOnBattery = FALSE;

	if (IsDiskSpun(bSpunState))
		bShouldFileIOBeDone = bSpunState;
	else if (IsSystemOnBattery(bOnBattery))	
		bShouldFileIOBeDone  = !bOnBattery;
	else	
		return FALSE;

	return TRUE;
}

// Exported
// This function first determines the disk spun state if it can ('coz this feature is supported only in Win2000, Win98 & above
// If the disk is not spun the function sets bSpunState to FALSE.
//
// The function returns FALSE if it cannot determine the state either thru' GetDevicePowerState.
// Check the function's return value. If it's TRUE, then only bSpunState will be set correctly.
// Whenever the function returns FALSE, ignore the bSpunState value.

BOOL IsDiskSpun(BOOL &bSpunState)
{
	return gShouldFileIOBeDone.IsDiskSpun(bSpunState);
}

// Exported
// The function determines if the system is using a battery(DC) or AC (this is avalaible in Win95 & above)
// If the system is using battery, the function sets bOnBattery to FALSE.
//
// The function returns FALSE if it cannot determine the state either thru' GetSystemPowerStatus.
// Check the function's return value. If it's TRUE, then only bOnBattery will be set correctly.
// Whenever the function returns FALSE, ignore the bOnBattery value.

BOOL IsSystemOnBattery(BOOL &bOnBattery)
{
	return gShouldFileIOBeDone.IsSystemOnBattery(bOnBattery);
}

// Exported
// This function first determines the disk spun state if it can ('coz this feature is supported only in Win2000, Win98 & above
// If not, then it tries to see if the system is using a battery(DC) or AC (this is avalaible in Win95 & above)
// If the disk is not spun or if the system is using battery, the function sets bShouldFileIOBeDone to FALSE.
//
// The function returns FALSE if it cannot determine the state either thru' GetDevicePowerState or GetSystemPowerStatus.
// Check the function's return value. If it's TRUE, then only bShouldFileIOBeDone will be set correctly.
// Whenever the function returns FALSE, ignore the bShouldFileIOBeDone value.

BOOL ShouldFileIOBeDone(BOOL &bShouldFileIOBeDone)
{
	return gShouldFileIOBeDone.ShouldFileIOBeDone(bShouldFileIOBeDone);	
}

// Exported
// Set a temp directory, where the file will be created, which will then be opened & used for the above IsDiskSpun API.
void SetTempFileDirectory(LPCTSTR lpszDirectory)
{
	// Update this function if lpszDirectory could be used anywhere else too
	gShouldFileIOBeDone.SetDirectory(lpszDirectory);
}
