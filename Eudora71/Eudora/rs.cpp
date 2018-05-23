// RS.CPP
//
// Routines for handling resource strings
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include <QCUtils.h>
#include "resource.h"

#include "rs.h"
#include "QCSharewareManager.h"

#include "utils.h"  //for QCLoadString

#include "fileutil.h" //for JJFile
#include "persona.h"
#include "SearchManager.h"

#include <map>
using namespace std;


#include "DebugNewHelpers.h"


// Number and size of string buffers for default buffers GetRString() keeps track of
const int INI_NORMAL_BUF_SIZE = 512;
const int INI_LARGE_BUF_SIZE = 2048;
const int INI_NUM_BUFS = 16;

CString INIPath;
CString DefaultINIPath;


enum IES_FALGS { IES_UNINITIALIZED=0, IES_LOOKED_UP=1, IES_DIRTY=2};

class CIniEntry
{
public:
	CIniEntry(): m_Status(IES_UNINITIALIZED) {}
	CString	m_StringData;
	int	m_Status;
};


typedef map<UINT, CIniEntry *, less<UINT> >::iterator CacheIteratorType;

class CIniEntryCache : public map<UINT, CIniEntry *, less<UINT> >
{
public:
	~CIniEntryCache();
		
};

CIniEntryCache::~CIniEntryCache()
{
	for (CacheIteratorType ci = begin(); ci != end(); ++ci)
	{
		CIniEntry *entry = (*ci).second;
		delete entry;
	}
}


static CIniEntryCache g_IniEntries;


static UINT GetSectionID(UINT IniID)
{
	if (IniID > IDS_INISN_SETTINGS2)
		return IDS_INISN_SETTINGS;
	if (IniID > IDS_INISN_WINDOW_POSITION)
		return IDS_INISN_WINDOW_POSITION;
	if (IniID > IDS_INISN_DEBUG)
		return IDS_INISN_DEBUG;
	return IDS_INISN_SETTINGS;
}

// --------------------------------------------------------------------------

bool GetEudoraProfileString(
	LPCTSTR lpSectionName, // points to section name
	LPCTSTR lpKeyName,     // points to key name
	CString *sValueStr)    // destination buffer
{
	LPCTSTR lpDefault = "\0x7F\0x7F";
	TCHAR buf[128];
	
	GetPrivateProfileString(lpSectionName, lpKeyName, lpDefault, buf, sizeof(buf), INIPath);

	if (strcmp(buf, lpDefault) == 0)
		GetPrivateProfileString(lpSectionName, lpKeyName, lpDefault, buf, sizeof(buf), DefaultINIPath);

	*sValueStr = buf;

	return (sValueStr->Compare(lpDefault) != 0);
}

// --------------------------------------------------------------------------

bool GetEudoraProfileInt(
	LPCTSTR lpSectionName, // points to section name
	LPCTSTR lpKeyName,     // points to key name
	int *sValueInt)    // destination value
{
	CString str;

	if (GetEudoraProfileString(lpSectionName, lpKeyName, &str))
	{
		*sValueInt = atoi((LPCSTR)str);
		return (true);
	}

	return (false);
}

// --------------------------------------------------------------------------

bool GetEudoraProfileLong(
	LPCTSTR lpSectionName, // points to section name
	LPCTSTR lpKeyName,     // points to key name
	long *sValueLong)    // destination value
{
	CString str;

	if (GetEudoraProfileString(lpSectionName, lpKeyName, &str))
	{
		char *cp = NULL;
		*sValueLong = strtol((LPCSTR)str, &cp, 10);
		return (true);
	}

	return (false);
}

// --------------------------------------------------------------------------

bool GetEudoraProfileULong(
	LPCTSTR lpSectionName, // points to section name
	LPCTSTR lpKeyName,     // points to key name
	unsigned long *sValueULong)    // destination value
{
	CString str;

	if (GetEudoraProfileString(lpSectionName, lpKeyName, &str))
	{
		char *cp = NULL;
		*sValueULong = strtoul((LPCSTR)str, &cp, 10);
		return (true);
	}

	return (false);
}

// --------------------------------------------------------------------------

bool WriteEudoraProfileString(
	LPCTSTR lpSectionName,  // pointer to section name
	LPCTSTR lpKeyName,      // pointer to key name
	LPCTSTR lpString)       // pointer to string to add
{
	return (WritePrivateProfileString(lpSectionName, lpKeyName, lpString, INIPath) == TRUE);
}
 
bool WriteEudoraProfileInt(
	LPCTSTR lpSectionName,  // pointer to section name
	LPCTSTR lpKeyName,      // pointer to key name
	const int nValue)       // value
{
	CString str;
	str.Format("%d", nValue);
	return (WriteEudoraProfileString(lpSectionName, lpKeyName, str));
}
 
bool WriteEudoraProfileLong(
	LPCTSTR lpSectionName,  // pointer to section name
	LPCTSTR lpKeyName,      // pointer to key name
	const long nValue)       // value
{
	CString str;
	str.Format("%ld", nValue);
	return (WriteEudoraProfileString(lpSectionName, lpKeyName, str));
}
 
bool WriteEudoraProfileULong(
	LPCTSTR lpSectionName,  // pointer to section name
	LPCTSTR lpKeyName,      // pointer to key name
	const unsigned long nValue)       // value
{
	CString str;
	str.Format("%lu", nValue);
	return (WriteEudoraProfileString(lpSectionName, lpKeyName, str));
}
 
// --------------------------------------------------------------------------

void RemoveIniKey(UINT StringNum)
{
	RemoveIniFromCache(StringNum);

	char Section[64]={0};
	char Entry[64]={0};
	
	UINT SectionNum = GetSectionID(StringNum);
	
	if (!QCLoadString(SectionNum, Section, sizeof(Section)) || !QCLoadString(StringNum, Entry, sizeof(Entry)))
	{
		return;
	}

	// The names for the entries have the format: <name>[\n<default value>]
	char* Newline = strchr(Entry, '\n');
	if (Newline)
		*Newline++ = 0;

	WritePrivateProfileString(Section, Entry, NULL, INIPath);
}

// GetIniString
//
// If StringNum is just an entry in the string table, then a pointer to a
// static buffer is returned with the contents of the string in the string
// table.
//
// If StringNum is an entry in the INI file, then a copy of the value found
// in the INI entry is returned.
//
// If a Buffer is specified, that is used instead of the static buffer, where
// size is the length of that buffer.
// 
const char* GetIniString(UINT StringNum, char* Buffer /*= NULL*/, int size /*= -1*/,
	BOOL GetName /*= FALSE*/)
{
	ASSERT(::IsMainThreadMT());		// FORNOW, until the statics are removed or data access is synchronized

	// There are several buffers so that multiple calls can be used without
	// squashing previous calls
	const int NumBuffs = INI_NUM_BUFS;
	const int BufSize = INI_NORMAL_BUF_SIZE;
	static char StringBuf[NumBuffs][BufSize];
	static int BufNum = 0;
	char* buf;

	
	if (size == -1)
	{              
		ASSERT(Buffer==NULL);
		size = BufSize;
	}

	if (Buffer)
		buf = Buffer;
	else
	{
		BufNum = ++BufNum % NumBuffs;
		buf = StringBuf[BufNum];
		size = BufSize;
	}
	*buf = 0;
	if (StringNum < 10000 || GetName)
	{
		// This is just a string in the string table
		QCLoadString(StringNum, buf, size);
		return (buf);
	}


	CIniEntry* IniEntry = NULL;
	CacheIteratorType ci = g_IniEntries.find(StringNum);
	if(ci == g_IniEntries.end()) //if not already in the cache
	{
		IniEntry = DEBUG_NEW CIniEntry; ASSERT(IniEntry);
		g_IniEntries[StringNum] = IniEntry;
	}
	else 
	{
		IniEntry = (*ci).second;
		if(IniEntry->m_Status != IES_UNINITIALIZED)
		{
			// If we've loaded a value already, return it
			strncpy(buf, (const char *)IniEntry->m_StringData, size);
			buf[size-1] = 0;
			return buf;
		}
	}


    // Haven't loaded a value, so look it up in the INI file
	char Section[64];
    char Entry[BufSize];
	UINT SectionNum = GetSectionID(StringNum);

	if (!QCLoadString(SectionNum, Section, sizeof(Section)) ||
		!QCLoadString(StringNum, Entry, sizeof(Entry)))
	{
		ASSERT(FALSE);
		return buf;
	}
	
	// The names for the entries have the format: <name>[\n<default value>]
	char* Newline = strchr(Entry, '\n');
	if (Newline)
	{
		*Newline++ = 0;
		IniEntry->m_StringData = Newline;
	}

	// check the Personality first
	CString	Personality = g_Personalities.GetCurrent();

	const char BogusEntry[] = "\177\177";

	g_Personalities.GetProfileString(Personality, Entry, BogusEntry, buf, size );
	if (!strcmp(buf, BogusEntry))
	{
		GetPrivateProfileString(Section, Entry, BogusEntry, buf, size, INIPath);
		if (!strcmp(buf, BogusEntry))
			GetPrivateProfileString(Section, Entry, BogusEntry, buf, size, DefaultINIPath);
	}

	if (!strcmp(buf, BogusEntry))
	{
		strncpy(buf, (const char *)IniEntry->m_StringData, size);
		buf[size-1] = 0;
	}
	else
		IniEntry->m_StringData = buf;

	IniEntry->m_Status |= IES_LOOKED_UP;

	return buf;
}

// --------------------------------------------------------------------------

bool GetDefaultIniSetting(const UINT StringNum, CString& str)
{	
	str.Empty();

	CString		Entry;

	bool	bSettingGood = Entry.LoadString(StringNum) && !Entry.IsEmpty();

	if (bSettingGood)
	{
		// The names for the entries have the format: <name>[\n<default value>]
		CString		ResourceValue;

		int		nNewLine = Entry.Find('\n');
		if (nNewLine >= 0)
		{
			ResourceValue = Entry.Right( Entry.GetLength() - nNewLine - 1 );
			Entry = Entry.Left(nNewLine);
		}

		// Check the DEudora.ini file just in case an admin has given defaults.
		if (!DefaultINIPath.IsEmpty())
		{
			char		IniEntry[INI_LARGE_BUF_SIZE];
			
			// use the settings section
			CRString Section(IDS_INISN_SETTINGS); 
			// Grab value from INI file, using resource value as default
			GetPrivateProfileString(Section, Entry, ResourceValue, IniEntry, sizeof(IniEntry), DefaultINIPath);
			// GPPS either found the value or copied ResourceValue into IniEntry, so use IniEntry
			str = IniEntry;
		}
		else
		{
			str = ResourceValue;	// No default INI file, so use what we have
		}
	}

	return bSettingGood;
}

// --------------------------------------------------------------------------

bool GetIniString(const UINT StringNum, CString& str)
{
	CIniEntry* IniEntry = NULL;
	CacheIteratorType ci = g_IniEntries.find(StringNum);
	if(ci == g_IniEntries.end()) //if not already in the cache
	{
		IniEntry = DEBUG_NEW CIniEntry();
		g_IniEntries[StringNum] = IniEntry;
	}
	else 
	{
		IniEntry = (*ci).second;

		if(IniEntry->m_Status != IES_UNINITIALIZED)
		{
			// If we've loaded a value already, return it
			str = IniEntry->m_StringData;
			return (true);
		}
	}

    // Haven't loaded a value, so look it up in the INI file
	char		Section[64];
	CString		Entry;
	UINT SectionNum = GetSectionID(StringNum);

	if ( !QCLoadString(SectionNum, Section, sizeof(Section)) ||
		 !Entry.LoadString(StringNum) )
	{
		ASSERT(FALSE);
		return (false);
	}

	// The names for the entries have the format: <name>[\n<default value>]
	int		nNewLine = Entry.Find('\n');
	if (nNewLine >= 0)
	{
		IniEntry->m_StringData = Entry.Right( Entry.GetLength() - nNewLine - 1 );
		Entry = Entry.Left(nNewLine);
	}

	// check the Personality first
	CString	Personality = g_Personalities.GetCurrent();

	const char BogusEntry[] = "\177\177";
	const int BufSize = INI_LARGE_BUF_SIZE;
	static char StringBuf[BufSize];

	char *buf = StringBuf;
	int size = BufSize;

	g_Personalities.GetProfileString(Personality, Entry, BogusEntry, buf, size );
	if (!strcmp(buf, BogusEntry))
	{
		GetPrivateProfileString(Section, Entry, BogusEntry, buf, size, INIPath);
		if (!strcmp(buf, BogusEntry))
			GetPrivateProfileString(Section, Entry, BogusEntry, buf, size, DefaultINIPath);
	}

	IniEntry->m_Status |= IES_LOOKED_UP;

	if (!strcmp(buf, BogusEntry)) // Not found, use default entry
	{
		str = IniEntry->m_StringData;
		return (false);
	}
	else
	{
		IniEntry->m_StringData = buf;
	}

	str = buf;

	return (true);
}

void GetIniStringList(UINT in_nStringNum, char in_cDelimeter, CStringList & out_listResourceStrings)
{
	CString		szEntireString;
	CString		szString;
	int			nDelimeterIndex;

	GetIniString(in_nStringNum, szEntireString);

	do
	{
		nDelimeterIndex = szEntireString.Find(in_cDelimeter);

		if (nDelimeterIndex != -1)
		{
			szString = szEntireString.Left(nDelimeterIndex);
			szEntireString = szEntireString.Right(szEntireString.GetLength() - nDelimeterIndex - 1);
		}
		else
		{
			szString = szEntireString;
		}

		if ( !szString.IsEmpty() )
			out_listResourceStrings.AddTail(szString);
	} while (nDelimeterIndex != -1);
}

short GetIniShort(UINT StringNum)
{
	char buf[32]={0};

	const char* rs = GetIniString(StringNum, buf, sizeof(buf));
	if (rs)
		return ((short)atoi(rs));
		
    ASSERT(FALSE);	// GetIniString should always return a buffer
    return 0;
}

long GetIniLong(UINT StringNum)
{
	char buf[32]={0};

	const char* rs = GetIniString(StringNum, buf, sizeof(buf));
	if (rs)
		return (atol(rs));
		
    ASSERT(FALSE);	// GetIniString should always return a buffer
	return (0L);
}

BOOL GetIniWindowPos(UINT StringNum, long& left, long& top, long& right, long& bottom)
{
	char buf[JJFileMT::BUF_SIZE]={0};
	const char* Entry = GetIniString(StringNum, buf, sizeof(buf));

	if (!Entry || !*Entry)
	{
	    ASSERT(FALSE);	// GetIniString should always return a buffer
		return FALSE;
	}

	sscanf(Entry, "%ld,%ld,%ld,%ld", &left, &top, &right, &bottom);

	return TRUE;
}

BOOL ConvertHexToColor(LPCSTR pHexStr, COLORREF& clr)
{
	if (strlen(pHexStr) != 6) // Six hex chars only
		return (FALSE);

	const char hex[] = "0123456789ABCDEF";

	// Ensure hex-ness
	for (int idx = 0; idx < 6; idx++)
	{
		if (!isxdigit((unsigned char)pHexStr[idx]))
			return (FALSE);
	}

	long red, green, blue;

	red = (((BYTE) (strchr(hex, toupper(pHexStr[0])) - hex)) << 4) | ((BYTE) (strchr(hex, toupper(pHexStr[1])) - hex));
	green = (((BYTE) (strchr(hex, toupper(pHexStr[2])) - hex)) << 4) | ((BYTE) (strchr(hex, toupper(pHexStr[3])) - hex));
	blue = (((BYTE) (strchr(hex, toupper(pHexStr[4])) - hex)) << 4) | ((BYTE) (strchr(hex, toupper(pHexStr[5])) - hex));

	clr = RGB(red,green,blue);

	return (TRUE);
}

BOOL GetIniColorRef(UINT StringNum, COLORREF& clr)
{
	char buf[32] = {0};
	const char *pIniStr = GetIniString(StringNum, buf, sizeof(buf));
	ASSERT(pIniStr);

	if (!pIniStr)
		return (FALSE);

	char *rs = (char *) pIniStr;
	while (isspace((unsigned char)*rs))
		rs++;

	if ((rs) && (strlen(rs) > 4)) // Min length = "0,0,0" (5)
	{
		// XRRGGBB or xRRGGBB or #RRGGBB
		if (('X' == (toupper(rs[0]))) || ('#' == (rs[0])))
		{
			return (ConvertHexToColor((rs + 1), clr));

//			if (strlen(rs) != 7) // Six hex chars and the start symbol
//				return (FALSE);
//
//			const char hex[] = "0123456789ABCDEF";
//
//			// Ensure hex-ness
//			for (int idx = 1; idx < 7; idx++)
//			{
//				if (!isxdigit((unsigned char)rs[idx]))
//					return (FALSE);
//			}
//
//			red = (((BYTE) (strchr(hex, toupper(rs[1])) - hex)) << 4) | ((BYTE) (strchr(hex, toupper(rs[2])) - hex));
//			green = (((BYTE) (strchr(hex, toupper(rs[3])) - hex)) << 4) | ((BYTE) (strchr(hex, toupper(rs[4])) - hex));
//			blue = (((BYTE) (strchr(hex, toupper(rs[5])) - hex)) << 4) | ((BYTE) (strchr(hex, toupper(rs[6])) - hex));
//
//			clr = RGB(red,green,blue);
//			return (TRUE);
		}

		long red, green, blue;
		if (sscanf(rs, "%ld,%ld,%ld", &red, &green, &blue) == 3)
		{
			clr = RGB(red,green,blue);
			return (TRUE);
		}
	}

	return (FALSE);

}

BOOL SetIniString(UINT StringNum, const char *String /*= NULL*/)
{

	CacheIteratorType ci = g_IniEntries.find(StringNum);
	if(ci == g_IniEntries.end())
	{
		// We haven't previously loaded a value, so use GetIniString to do the dirty work
		char TinyBuf[1];
		GetIniString(StringNum, TinyBuf, sizeof(TinyBuf));
		if ((ci=g_IniEntries.find(StringNum)) == g_IniEntries.end())
		{
			// If we still didn't get an entry, then something definitely went wrong
			ASSERT(FALSE);
			return (FALSE);
		}
	}
	CIniEntry *IniEntry = (*ci).second;


    // If the info is the same, don't write anything 
	if (String && IniEntry->m_StringData.Compare(String) == 0)
    	return (TRUE);

	IniEntry->m_StringData = String;
	IniEntry->m_Status |= IES_DIRTY;

    return (TRUE);
}

BOOL SetIniShort(UINT StringNum, short Num)
{
	char Entry[8];
    
	itoa(Num, Entry, 10);

	return (SetIniString(StringNum, Entry));
}

BOOL SetIniLong(UINT StringNum, long Num)
{
	char Entry[16];

	ltoa(Num, Entry, 10);
	
	return (SetIniString(StringNum, Entry));
}

BOOL SetIniWindowPos(UINT StringNum, long left, long top, long right, long bottom)
{
	char Entry[32];
	wsprintf(Entry, "%ld,%ld,%ld,%ld", left, top, right, bottom);

	return (SetIniString(StringNum, Entry));
}


BOOL SetIniColorRef(UINT StringNum, COLORREF clr)
{
	char Entry[32];
	wsprintf(Entry, "%ld,%ld,%ld", (clr & 0xFF), ((clr >> 8) & 0xFF), ((clr >> 16) & 0xFF));

	return (SetIniString(StringNum, Entry));
}

// Call NotifyIniUpdates *after* user specifically changes a setting.
// We don't want to reduce performance by always calling in any of the
// SetIni... routines.
//
// Currently support mode changes and changes made via x-Eudora-option
// dialog. Changes made via settings dialog are currently only partially
// supported. Add further support for settings dialog changes as needed.
void NotifyIniUpdates(UINT nStringNum)
{
	// Notify anyone who may care that an INI setting has been changed by the user.
	
	// QCLogFileMT needs us to pass the corresponding INI value (for any INI values
	// that it's interested in) because QCLogFileMT is in QCUtils.dll and therefore
	// cannot access INI values itself.
	if (nStringNum == IDS_INI_DEBUG_LOG_LEVEL)
		QCLogFileMT::NotifyIniChange( nStringNum, GetIniLong(nStringNum) );

	if (nStringNum == IDS_INI_MODE)
	{
		QCLogFileMT::NotifyIniChange( nStringNum, GetSharewareMode() );
		SearchManager::Instance()->NotifyIniChange(nStringNum);
	}

	if (nStringNum == IDS_INI_USE_INDEXED_SEARCH)
		SearchManager::Instance()->NotifyIniChange(nStringNum);
}

int CompareRString(UINT StringNum, const char* String, int len /*= -1*/)
{
	char buf[JJFileMT::BUF_SIZE];
    assert(StringNum <= 10000);
	if (String)
	{
		const char* s = NULL;
		QCLoadString(StringNum, buf,JJFileMT::BUF_SIZE) ;
		s = buf;
		if (s)
		{
			if (len < 0)
				return (strcmp(s, String));
			else if (len > 0)
				return (strncmp(s, String, len));
			else
				return (strncmp(s, String, strlen(s)));
		}
	}
	return (1);
}

int CompareRStringI(UINT StringNum, const char* String, int len /*= -1*/)
{
	char buf[JJFileMT::BUF_SIZE];
    assert(StringNum <= 10000);
	if (String)
	{
		const char* s = NULL;
		QCLoadString(StringNum, buf,JJFileMT::BUF_SIZE) ;
		s = buf;
		if (s)
		{
			if (len < 0)
				return (stricmp(s, String));
			else if (len > 0)
				return (strnicmp(s, String, len));
			else
				return (strnicmp(s, String, strlen(s)));
		}
	}
	return (1);
}


int FindRStringIndex(UINT StartNum, UINT EndNum, const char* String, int len /*= -1*/)
{
	int i;
	char buf[JJFileMT::BUF_SIZE];
    assert(EndNum <= 10000);

	if (String)
	{
		if (len < 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				QCLoadString(StartNum,buf, JJFileMT::BUF_SIZE);
				if (!strcmp(buf, String))
					return (i);
			}
		}
		else if (len > 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{	
				QCLoadString(StartNum,buf, JJFileMT::BUF_SIZE);
				if (!strncmp(buf, String, len))
					return (i);
			}
		}
		else
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				QCLoadString(StartNum,buf, JJFileMT::BUF_SIZE);
				if (!strncmp(buf, String, strlen(buf)))
					return (i);
			}
		}
	}

	return (-1);
}

int FindRStringIndexI(UINT StartNum, UINT EndNum, const char* String, int len /*= -1*/)
{
	int i;
	char buf[JJFileMT::BUF_SIZE];
    assert(EndNum <= 10000);

	if (String)
	{
		if (len < 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				QCLoadString(StartNum,buf, JJFileMT::BUF_SIZE);
				if (!stricmp(buf, String))
					return (i);
			}
		}
		else if (len > 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{	
				QCLoadString(StartNum,buf, JJFileMT::BUF_SIZE);
				if (!strnicmp(buf, String, len))
					return (i);
			}
		}
		else
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				QCLoadString(StartNum,buf, JJFileMT::BUF_SIZE);
				if (!strnicmp(buf, String, strlen(buf)))
					return (i);
			}
		}
	}

	return (-1);
}


// Strips a string (or strings) from the beginning of the string provided.
//
// * The strip string can contain multiple strings to match and
//   potentially strip, each separated by a specified delimeter character.
// * At most one strip string will be stripped from the beginning of the string.
// * All matches are case insensitive.
void StripLeadingStrings(CString & io_strToModify, CString & io_strStripStrings, char in_cDelimeter)
{
	CString		strSearch = io_strToModify;

	// Make search case insensitive
	strSearch.MakeLower();
	io_strStripStrings.MakeLower();

	if (in_cDelimeter)
	{
		int			nCharAfterPrevDelimeter = 0;
		int			nNextDelimeterIndex;
		
		do
		{
			//	Find the next delimeter (if any)
			nNextDelimeterIndex = io_strStripStrings.Find(in_cDelimeter, nCharAfterPrevDelimeter);

			//	If we didn't find the delimeter take the rest of the string
			if (nNextDelimeterIndex == -1)
				nNextDelimeterIndex = io_strStripStrings.GetLength();
			
			//	Get the portion of the strip string from the previous delimeter
			//	to the current delimeter.
			const CString &		strResMatch = io_strStripStrings.Mid( nCharAfterPrevDelimeter,
																	  nNextDelimeterIndex-nCharAfterPrevDelimeter );
			
			//	If the string to be searched is long enough, then compare the start
			//	of it with the current portion of the strip string.
			if ( (strResMatch.GetLength() <= strSearch.GetLength()) &&
				 (strSearch.Left(strResMatch.GetLength()).CompareNoCase(strResMatch) == 0) )
			{
				//	It matched - so strip the portion of the string that matched
				io_strToModify = io_strToModify.Right( io_strToModify.GetLength() - strResMatch.GetLength() );
				break;
			}

			nCharAfterPrevDelimeter = nNextDelimeterIndex + 1;
		} while( nNextDelimeterIndex < io_strStripStrings.GetLength() );
	}
	else if ( (io_strStripStrings.GetLength() <= strSearch.GetLength()) &&
			  (strSearch.Left(io_strStripStrings.GetLength()).CompareNoCase(io_strStripStrings) == 0) )
	{
		//	It matched - so strip the portion of the string that matched
		io_strToModify = io_strToModify.Right( io_strToModify.GetLength() - strSearch.GetLength() );
	}
}


// Strips a string (or strings) from the end of the string provided.
//
// * The strip string can contain multiple strings to match and
//   potentially strip, each separated by a specified delimeter character.
// * At most one strip string will be stripped from the beginning of the string.
// * All matches are case insensitive.
void StripTrailingStrings(CString & io_strToModify, CString & io_strStripStrings, char in_cDelimeter)
{
	CString		strSearch = io_strToModify;

	// Make search case insensitive
	strSearch.MakeLower();
	io_strStripStrings.MakeLower();

	if (in_cDelimeter)
	{
		int			nCharAfterPrevDelimeter = 0;
		int			nNextDelimeterIndex;
		
		do
		{
			//	Find the next delimeter (if any)
			nNextDelimeterIndex = io_strStripStrings.Find(in_cDelimeter, nCharAfterPrevDelimeter);

			//	If we didn't find the delimeter take the rest of the string
			if (nNextDelimeterIndex == -1)
				nNextDelimeterIndex = io_strStripStrings.GetLength();
			
			//	Get the portion of the strip string from the previous delimeter
			//	to the current delimeter.
			const CString &		strResMatch = io_strStripStrings.Mid( nCharAfterPrevDelimeter,
																	  nNextDelimeterIndex-nCharAfterPrevDelimeter );
			
			//	If the string to be searched is long enough, then compare the start
			//	of it with the current portion of the strip string.
			if ( (strResMatch.GetLength() <= strSearch.GetLength()) &&
				 (strSearch.Right(strResMatch.GetLength()).CompareNoCase(strResMatch) == 0) )
			{
				//	It matched - so strip the portion of the string that matched
				io_strToModify = io_strToModify.Left( io_strToModify.GetLength() - strResMatch.GetLength() );
				break;
			}

			nCharAfterPrevDelimeter = nNextDelimeterIndex + 1;
		} while( nNextDelimeterIndex < io_strStripStrings.GetLength() );
	}
	else if ( (io_strStripStrings.GetLength() <= strSearch.GetLength()) &&
			  (strSearch.Right(io_strStripStrings.GetLength()).CompareNoCase(io_strStripStrings) == 0) )
	{
		//	It matched - so strip the portion of the string that matched
		io_strToModify = io_strToModify.Left( io_strToModify.GetLength() - strSearch.GetLength() );
	}
}


// Returns whether or not a resource string appears in the string provided.
// * The resource string can contain multiple strings to match and
//   potentially strip, each separated by a specified delimeter character.
// * All matches are case insensitive.
bool ResourceStringAppearsInString(const char * in_szSearch, UINT in_nResourceID, char in_cDelimeter)
{
	bool		bResourceStringAppearsInString = false;
	CString		strSearch = in_szSearch;
	CRString	strEntireRes(in_nResourceID);

	// Make search case insensitive
	strSearch.MakeLower();
	strEntireRes.MakeLower();

	if (in_cDelimeter)
	{
		int			nCharAfterPrevDelimeter = 0;
		int			nNextDelimeterIndex;
		
		do
		{
			//	Find the next delimeter (if any)
			nNextDelimeterIndex = strEntireRes.Find(in_cDelimeter, nCharAfterPrevDelimeter);

			//	If we didn't find the delimeter take the rest of the string
			if (nNextDelimeterIndex == -1)
				nNextDelimeterIndex = strEntireRes.GetLength();
			
			//	Search for the portion of the resource string from the previous
			//	delimeter to the current delimeter.
			const CString &		strResMatch = strEntireRes.Mid( nCharAfterPrevDelimeter,
																nNextDelimeterIndex-nCharAfterPrevDelimeter );
			if (strSearch.Find(strResMatch) != -1)
			{
				bResourceStringAppearsInString = true;
				break;
			}

			nCharAfterPrevDelimeter = nNextDelimeterIndex + 1;
		} while( nNextDelimeterIndex < strEntireRes.GetLength() );
	}
	else
	{
		//	No delimeter provided, just search for the entire string
		bResourceStringAppearsInString = (strSearch.Find(strEntireRes) != -1);
	}

	return bResourceStringAppearsInString;
}


// Returns whether or not the string provided starts with a start string.
// * The start string can contain multiple strings to match and
//   potentially strip, each separated by a specified delimeter character.
// * All matches are case insensitive.
bool StringStartsWithString(const char * in_szSearch, CString & io_strStartStrings, char in_cDelimeter)
{
	bool		bStringEndsWithResourceString = false;
	CString		strSearch = in_szSearch;

	// Make search case insensitive
	strSearch.MakeLower();
	io_strStartStrings.MakeLower();

	if (in_cDelimeter)
	{
		int			nCharAfterPrevDelimeter = 0;
		int			nNextDelimeterIndex;
		
		do
		{
			//	Find the next delimeter (if any)
			nNextDelimeterIndex = io_strStartStrings.Find(in_cDelimeter, nCharAfterPrevDelimeter);

			//	If we didn't find the delimeter take the rest of the string
			if (nNextDelimeterIndex == -1)
				nNextDelimeterIndex = io_strStartStrings.GetLength();
			
			//	Get the portion of the resource string from the previous delimeter
			//	to the current delimeter.
			const CString &		strResMatch = io_strStartStrings.Mid( nCharAfterPrevDelimeter,
																	  nNextDelimeterIndex-nCharAfterPrevDelimeter );
			
			//	If the string to be searched is long enough, then compare the start
			//	of it with the current portion of the resource string.
			if ( (strResMatch.GetLength() <= strSearch.GetLength()) &&
				 (strSearch.Left(strResMatch.GetLength()).CompareNoCase(strResMatch) == 0) )
			{
				bStringEndsWithResourceString = true;
				break;
			}

			nCharAfterPrevDelimeter = nNextDelimeterIndex + 1;
		} while( nNextDelimeterIndex < io_strStartStrings.GetLength() );
	}
	else if ( (io_strStartStrings.GetLength() <= strSearch.GetLength()) &&
			  (strSearch.Left(io_strStartStrings.GetLength()).CompareNoCase(io_strStartStrings) == 0) )
	{
		bStringEndsWithResourceString = true;
	}

	return bStringEndsWithResourceString;
}


bool StringStartsWithStringInList(const char * in_szSearch, CStringList & in_listStrings, POSITION & out_posFoundTag)
{
	bool		bStartsWithStringInList = false;
	POSITION	posNext = in_listStrings.GetHeadPosition();
	POSITION	posCurrent = posNext;

	while (posCurrent)
	{
		CString &	szString = in_listStrings.GetNext(posNext);

		bStartsWithStringInList = (strnicmp(szString, in_szSearch, szString.GetLength()) == 0);
		if (bStartsWithStringInList)
		{
			out_posFoundTag = posCurrent;
			break;
		}

		posCurrent = posNext;
	}

	return bStartsWithStringInList;
}


// Returns whether or not the string provided ends with an end string.
// * The end string can contain multiple strings to match and
//   potentially strip, each separated by a specified delimeter character.
// * All matches are case insensitive.
bool StringEndsWithString(const char * in_szSearch, CString & io_strEndStrings, char in_cDelimeter)
{
	bool		bStringEndsWithResourceString = false;
	CString		strSearch = in_szSearch;

	// Make search case insensitive
	strSearch.MakeLower();
	io_strEndStrings.MakeLower();

	if (in_cDelimeter)
	{
		int			nCharAfterPrevDelimeter = 0;
		int			nNextDelimeterIndex;
		
		do
		{
			//	Find the next delimeter (if any)
			nNextDelimeterIndex = io_strEndStrings.Find(in_cDelimeter, nCharAfterPrevDelimeter);

			//	If we didn't find the delimeter take the rest of the string
			if (nNextDelimeterIndex == -1)
				nNextDelimeterIndex = io_strEndStrings.GetLength();
			
			//	Get the portion of the resource string from the previous delimeter
			//	to the current delimeter.
			const CString &		strResMatch = io_strEndStrings.Mid( nCharAfterPrevDelimeter,
																	nNextDelimeterIndex-nCharAfterPrevDelimeter );
			
			//	If the string to be searched is long enough, then compare the end
			//	of it with the current portion of the resource string.
			if ( (strResMatch.GetLength() <= strSearch.GetLength()) &&
				 (strSearch.Right(strResMatch.GetLength()).CompareNoCase(strResMatch) == 0) )
			{
				bStringEndsWithResourceString = true;
				break;
			}

			nCharAfterPrevDelimeter = nNextDelimeterIndex + 1;
		} while( nNextDelimeterIndex < io_strEndStrings.GetLength() );
	}
	else if ( (io_strEndStrings.GetLength() <= strSearch.GetLength()) &&
			  (strSearch.Right(io_strEndStrings.GetLength()).CompareNoCase(io_strEndStrings) == 0) )
	{
		bStringEndsWithResourceString = true;
	}

	return bStringEndsWithResourceString;
}


// Use this to load up an array of resource strings ... be sure to allocate the array first
void LoadCStringArray(CString *StrArray, UINT StartNum, UINT EndNum)
{
	if (StartNum > EndNum)
	{
		ASSERT(0);
		return;
	}

	for(int i = 0; StartNum <= EndNum; StartNum++, i++)
		StrArray[i].LoadString(StartNum);
}


// Use this to compare the items of a CString Array to a String
// returns the Array Index
int FindCStringArrayIndex(int numItems, CString *StrArray, const char *String, int len /*= -1*/)
{
	int i;
	
	if (len == 0 || strlen(String) == 0)
		return -1;

	for (i = 0; i < numItems; i++)
	{
		if (len < 0)
		{
			if (!strcmp(StrArray[i], String))
				return (i);
		}
		else if (!strncmp(StrArray[i], String, len))
			return (i);
	}
	return (-1);
}


// Use this to compare no case the items of a CString Array to a String
// returns the Array Index
int FindCStringArrayIndexI(int numItems, CString *StrArray, const char *String, int len /*= -1*/)
{
	int i;
	if (len == 0 || strlen(String) == 0)
		return -1;

	for (i = 0; i < numItems; i++)
	{
		if (len < 0)
		{
			if (!stricmp(StrArray[i], String))
				return (i);
		}
		else if (!strnicmp(StrArray[i], String, len))
			return (i);
	}
	return (-1);
}


int FlushINIFile()
{
    CString		String; 
    char		Section[64];
	char		Entry[64];
	
	for( CacheIteratorType ci = g_IniEntries.begin(); ci != g_IniEntries.end(); ci++)
	{
		CIniEntry* IniEntry = (*ci).second;
		int StringNum = (*ci).first;
		
		if (!(IniEntry->m_Status & IES_DIRTY))
			continue;
		
		UINT SectionNum = GetSectionID(StringNum);
		if (!QCLoadString(SectionNum, Section, sizeof(Section)) ||
			!QCLoadString(StringNum, Entry, sizeof(Entry)))
		{
			continue;
		}

		// The names for the entries have the format: <name>[\n<default value>]
		char* Newline = strchr(Entry, '\n');
		if (Newline)
			*Newline++ = 0;

		if (IniEntry->m_StringData.IsEmpty())
			String.Empty();
		else
		{
        	// Put quotes around in leading or trailing space

			// Assigning a CString to a CString does shared memory ref counting
			// until you modify the original. That's cool because it avoids
			// copying the string in most cases. The downside is that in those
			// cases where we need to add quotes, we'll need to preallocate
			// every time rather than once before this entire for loop.
			String = IniEntry->m_StringData;

			if ( (String[0] == ' ') || (String[String.GetLength()-1] == ' ') )
			{
				// Preallocate the string so that we don't expand it twice.
				String.GetBuffer(String.GetLength() + 2);
				
				String.Insert(0, '"');
				String += '"';
			}
    	}

		// password stuff goes to current personality
		if ( StringNum == IDS_INI_SAVE_PASSWORD || StringNum == IDS_INI_SAVE_PASSWORD_TEXT )
		{
			CString	yo = g_Personalities.GetCurrent();
			if ( ! yo.IsEmpty() )
			{
				if (g_Personalities.WriteProfileString(yo, Entry, String))
					IniEntry->m_Status &= ~IES_DIRTY;
			}
			else
			{
				// handle the default 
				if (WritePrivateProfileString(Section, Entry, String, INIPath))
					IniEntry->m_Status &= ~IES_DIRTY;
			}

		}
		else
		{
			if (WritePrivateProfileString(Section, Entry, String, INIPath))
				IniEntry->m_Status &= ~IES_DIRTY;
		}
	}

	// Flush changes to disk
	WritePrivateProfileString(NULL, NULL, NULL, INIPath);

	return (TRUE);
}

// ConvertIni
// Moves the entries from the given section in to the [Settings] section
//
BOOL ConvertIni(const char* IniFilename, UINT SectionID)
{
	char Filename[_MAX_PATH + 1];
	BOOL InSection = FALSE;
	CString SettingsName;
	CString SectionName;
	char buf[2048];	
	JJFile In;
	JJFile Out;
	int	Fail = 0;

	SettingsName.LoadString(IDS_INISN_SETTINGS);
	SectionName.LoadString(SectionID);
	
	// If the section isn't in the file, then there's no need to do anything
	if (!GetPrivateProfileString(SectionName, NULL, "", buf, sizeof(buf), IniFilename))
		return (TRUE);

	strcpy(Filename, IniFilename);
	::SetFileExtensionMT(Filename, "bak");
	SectionName = '[' + SectionName + ']';	
	SettingsName = '[' + SettingsName + ']';	
	
	if (FAILED(In.Open(IniFilename, O_RDONLY)) ||
		FAILED(Out.Open(Filename, O_WRONLY | O_CREAT | O_TRUNC)))
	{
		return FALSE;
	}

	// Write out [Settings] section name
	if (FAILED(Out.PutLine(SettingsName)))
		Fail = 1;
		
	// First pass is to write all entries from [Settings] and the given section to the temp file
	HRESULT hrGetLine = S_OK;
	long lNumBytesRead = 0;
	while ( !Fail && SUCCEEDED(hrGetLine = In.GetLine(buf, sizeof(buf), &lNumBytesRead)) && (lNumBytesRead > 0))
	{
		if (InSection && *buf == '[')
			InSection = FALSE;
		if (!InSection && (!SectionName.CompareNoCase(buf) || !SettingsName.CompareNoCase(buf)))
			InSection = TRUE;
		else if (InSection)
		{
			if (FAILED(Out.PutLine(buf)))
			{
				Fail = 1;
			}
		}
	}
	if (FAILED(hrGetLine))
		Fail = 1;
	
	if (!Fail)
	{
		// Second pass is to write all other sections to the temp file, including the given section
		InSection = FALSE;
		In.Seek(0L);
		while (!Fail && SUCCEEDED(hrGetLine = In.GetLine(buf, sizeof(buf), &lNumBytesRead)) && (lNumBytesRead > 0))
		{
			if (InSection && *buf == '[')
				InSection = FALSE;
			if (!InSection && (/*!SectionName.CompareNoCase(buf) || */!SettingsName.CompareNoCase(buf)))
				InSection = TRUE;
			else if (!InSection)
			{
				if (FAILED(Out.PutLine(buf)))
					Fail = 1;
			}
		}

		if (FAILED(hrGetLine))
			Fail = 1;
	}

	if (!Fail)
	{
		// Everything looks good so far, so lets move the temp file to the real file
		In.Delete();
		Out.Rename(IniFilename);
		return (TRUE);
	}
	

	Out.Delete();
	return (FALSE);
}

int SetupINIFilename(const char* Filename /*= NULL*/)
{
	CRString ININame(IDS_EUDORA_INI_NAME);
	CRString DefaultININame(IDS_DEUDORA_INI_NAME);

	// Default INI filename
	DefaultINIPath = ExecutableDir;
	DefaultINIPath += DefaultININame;
	if (!::FileExistsMT(DefaultINIPath))
		DefaultINIPath.Empty();

	// User INI filename
	if (Filename && *Filename)
	{
		// Make the INI file relative to the EudoraDir if the passed in INI
		// filename has no path
		if (strchr(Filename, SLASH))
			INIPath = Filename;
		else
		{
			INIPath = EudoraDir;
			INIPath += Filename;
		}
	}
	else
	{
		INIPath = EudoraDir;
		INIPath += ININame;
	}

	// fully qualify the INIPath if not already
	if ( ( INIPath[ 1 ] != ':' ) && ( strstr( INIPath, "\\\\" ) != INIPath ) )
	{
		char tmp[ 256 ];
		int drive = _getdrive();
		tmp[ 0 ] = (char)(drive + 'A' - 1);	// drive 1 = A, 2 = B, etc.
		tmp[ 1 ] = ':';
		tmp[ 2 ] = '\0';

		strcat( tmp, INIPath );
		INIPath = tmp;
	}

	// Convert from old .INI format with [Configurations], [Switches], and [Miscellaneous]
	// to new format with [Settings]
	// If there's already a [Settings] section, then a conversion has already been done
	char buf[4];
	CRString SettingsName(IDS_INISN_SETTINGS);
	if (::FileExistsMT(INIPath) && !GetPrivateProfileString(SettingsName, NULL, "", buf, sizeof(buf), INIPath))
	{
		if (ConvertIni(INIPath, IDS_INISN_CONFIGURATIONS) && ConvertIni(INIPath, IDS_INISN_SWITCHES))
			ConvertIni(INIPath, IDS_INISN_MISCELLANEOUS);
	}
	if (!DefaultINIPath.IsEmpty() &&
		!GetPrivateProfileString(SettingsName, NULL, "", buf, sizeof(buf), DefaultINIPath))
    {
		if (ConvertIni(DefaultINIPath, IDS_INISN_CONFIGURATIONS) && ConvertIni(DefaultINIPath, IDS_INISN_SWITCHES))
			ConvertIni(DefaultINIPath, IDS_INISN_MISCELLANEOUS);
	}
    
	return (TRUE);
}

void IniStringCleanUp()
{
	FlushINIFile();
}


void RemoveIniFromCache(UINT StringNum)
{

	CIniEntry* IniEntry = NULL;
	CacheIteratorType ci = g_IniEntries.find(StringNum);
	if(ci != g_IniEntries.end())
	{
		IniEntry = (*ci).second;
		g_IniEntries.erase(ci);
		delete IniEntry;
	}
}

void RemoveAllFromCache(void)
{
	CIniEntry* IniEntry = NULL;
	CacheIteratorType ci = g_IniEntries.begin();
	while(ci != g_IniEntries.end())
	{
		IniEntry = (*ci).second;
		g_IniEntries.erase(ci++);
		delete IniEntry;
	}
}


////////////////////////////////////////////////////////////////////////
// PurgeIniSetting [extern]
//
// Purge a Eudora INI setting string from the appropriate section of the
// INI file.  This ensures that we will get the default value for the
// INI setting the next time we start.
////////////////////////////////////////////////////////////////////////
BOOL PurgeIniSetting(int nID)
{
	//
	// Determine the section header keyword id from the INI id.
	//
	UINT nSectionNum = GetSectionID(nID);

	//
	// Load section name and key name strings from resources.
	//
	char section[64];
	char key[100];
	if (! QCLoadString(nSectionNum, section, sizeof(section)) ||
		! QCLoadString(nID, key, sizeof(key)))
	{
		return FALSE;
	}

	char* p_newline = strchr(key, '\n');
	if (p_newline)
		*p_newline = 0;

	//
	// Flush a Eudora INI setting string from the INI file.
	//
	::WritePrivateProfileString(section, key, NULL, INIPath);
	return TRUE;
}
