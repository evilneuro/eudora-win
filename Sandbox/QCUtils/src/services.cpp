////////////////////////////////////////////////////////////////////////
// File: session.cpp
//
//
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <io.h>
#include <ctype.h>

#include "qcerror.h"
#include "services.h"

const char chrSLASH = '\\';

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


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
			sprintf(TimeZone, "%+2.2d%2.2d", TimeZoneMinutes / 60, TimeZoneMinutes % 60);
	}

	{
		CString Result;
		::AfxFormatStrings(Result, Format, Args, 4);
		::strcpy(buf, Result);

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
	static int nGMTOffset = -1;

	if (NULL == pszTimeZone || NULL == pOffset)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	if (nGMTOffset != -1)
	{
		//
		// We've already done the calculation once, so short circuit
		// the calculation and return the previously computed result.
		//
		*pOffset = nGMTOffset;
		return S_OK;
	}

//FORNOW	GetIniString(IDS_INI_TIME_ZONE, GMTstr, sizeof(GMTstr));	"TimeZone"

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

	char* pszNewString = new char[strlen(pszString) + 1];
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
			if (isalpha(*s))
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
	const char *pPre1 = "re";
	const char *pPre2 = "fwd";

	const size_t nLen1 = 2;
	const size_t nLen2 = 3;

	const char cDelim = ':';

	const char *pSave;

	if (pSubjectStr)
		while (*pSubjectStr)
		{
			pSave = pSubjectStr;

			// Look for first prefix
			if (_strnicmp(pPre1, pSubjectStr, nLen1) == 0)
			{
				pSubjectStr += nLen1;
			}
			// Look for second preix
			else if (_strnicmp(pPre2, pSubjectStr, nLen2) == 0)
			{
				pSubjectStr += nLen2;
			}
			else
				break; // Niether prefix, so we are done

			// Now skip non-alpha chars til we hit the deliminator char
			while ((*pSubjectStr) && (*pSubjectStr != cDelim) && (!isalpha(*pSubjectStr)))
				pSubjectStr++;

			// If we hit an alpha or end before the delim, then
			// we back up to before the prefix and we are done
			if (*pSubjectStr != cDelim)
				return (pSave);

			// Skip the delim
			pSubjectStr++;

			// Skip whitespace after the delim
			while (isspace(*pSubjectStr))
				pSubjectStr++;
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
// FileRenameMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
HRESULT FileRenameMT(const char* pszOldName, const char* pszNewName)
{
	if (!pszOldName || !pszNewName)
	{
		ASSERT(0);
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_RENAME);
	}

	//if (::rename(pszOldName, pszNewName)) use 32 bit functions
	if (!::MoveFile(pszOldName, pszNewName))
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_RENAME);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// FileRemoveMT [extern, exported]
//
////////////////////////////////////////////////////////////////////////
HRESULT FileRemoveMT(const char* pszFilename)
{
	if (!pszFilename || !*pszFilename)
	{
		ASSERT(0);
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_DELETE);
	}

	if (::FileExistsMT(pszFilename) && ::remove(pszFilename) == -1)
		return MAKE_HRESULT(1, FACILITY_ITF, QCUTIL_E_FILE_DELETE);

	return S_OK;
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

	char szDrive[4];			// stores the "C:\" part of the pathname
	if (::strlen(pszPathname) > 3)
	{
		szDrive[0] = *pszPathname;
		szDrive[1] = *(pszPathname+1);
		szDrive[2] = *(pszPathname+2);
		szDrive[3] = 0;
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
