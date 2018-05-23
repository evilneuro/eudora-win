// RS.CPP
//
// Routines for handling resource strings
//

#include "stdafx.h"

#include <QCUtils.h>
#include "resource.h"

#include "rs.h"

#include "utils.h"  //for QCLoadString

#include "fileutil.h" //for JJFile
#include "persona.h"

#include <map>
using namespace std;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Number and size of string buffers for default buffers GetRString() keeps track of
const int INI_BUF_SIZE = 512;
const int INI_NUM_BUFS = 16;

#define MAX_SETTINGS_SECTION_ID 10600

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


// --------------------------------------------------------------------------

bool GetEudoraProfileString(
	LPCTSTR lpSectionName, // points to section name
	LPCTSTR lpKeyName,     // points to key name
	CString *sValueStr)    // destination buffer
{
	LPCTSTR lpDefault = "\0x7F\0x7F";
	const DWORD nSize = 128;
	LPTSTR lpRetStr = sValueStr->GetBuffer(nSize);
	
	GetPrivateProfileString(lpSectionName, lpKeyName, lpDefault, lpRetStr, nSize, INIPath);

	if (strcmp(lpRetStr, lpDefault) == 0)
		GetPrivateProfileString(lpSectionName, lpKeyName, lpDefault, lpRetStr, nSize, DefaultINIPath);

	sValueStr->ReleaseBuffer();

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
	
	int SectionNum = StringNum / 100 * 100;
	
	if (SectionNum < MAX_SETTINGS_SECTION_ID)
		SectionNum = IDS_INISN_SETTINGS;

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
	const int BufSize = INI_BUF_SIZE;
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
		IniEntry = new CIniEntry; ASSERT(IniEntry);
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
	int SectionNum = StringNum / 100 * 100;

	if (SectionNum < MAX_SETTINGS_SECTION_ID)
		SectionNum = IDS_INISN_SETTINGS;
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
    char Entry[256] = {0};
	char IniEntry[256];
	
	str.Empty();

	if (QCLoadString(StringNum, Entry, sizeof(Entry)) && (*Entry))
	{
		// The names for the entries have the format: <name>[\n<default value>]
		char* Newline = strchr(Entry, '\n');
		char* ResourceValue;

		// We want ResourceValue to point to the value.  If the string
		// has a name and a value, this is the character after the newline.
		// If the string has just a name, we'll point the value at the NULL
		// terminator, to indicate an empty value.
		if (Newline)
		{
			*Newline = NULL;	// null-terminate the name
			ResourceValue = Newline+1;
		}
		else
			ResourceValue = Entry + strlen(Entry);

		// Check the DEudora.ini file just in case an admin has given defaults.
		if (!DefaultINIPath.IsEmpty())
		{
			// use the settings section
			CRString Section(IDS_INISN_SETTINGS); 
			// Grab value from INI file, using resource value as default
			GetPrivateProfileString(Section, Entry, ResourceValue, IniEntry, sizeof(IniEntry), DefaultINIPath);
			// GPPS either found the value or copied ResourceValue into IniEntry, so use IniEntry
			str = IniEntry;
		}
		else
			str = ResourceValue;	// No default INI file, so use what we have

		return true;
	}

	return false;
}

// --------------------------------------------------------------------------

bool GetIniString(const UINT StringNum, CString& str)
{
	CIniEntry* IniEntry = NULL;
	CacheIteratorType ci = g_IniEntries.find(StringNum);
	if(ci == g_IniEntries.end()) //if not already in the cache
	{
		IniEntry = new CIniEntry();
		ASSERT(IniEntry);
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
	char Section[64];
    char Entry[256];
	int SectionNum = StringNum / 100 * 100;

	if (SectionNum < MAX_SETTINGS_SECTION_ID)
		SectionNum = IDS_INISN_SETTINGS;
	if (!QCLoadString(SectionNum, Section, sizeof(Section)) ||
		!QCLoadString(StringNum, Entry, sizeof(Entry)))
	{
		ASSERT(FALSE);
		return (false);
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
	const int BufSize = INI_BUF_SIZE;
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
		if (!isxdigit(pHexStr[idx]))
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
	while (isspace(*rs))
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
//				if (!isxdigit(rs[idx]))
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

int CompareRString(UINT StringNum, const char* String, int len /*= -1*/)
{
	char buf[JJFileMT::BUF_SIZE];

	if (String)
	{
		const char* s = GetIniString(StringNum, buf, sizeof(buf));
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

	if (String)
	{
		const char* s = GetIniString(StringNum, buf, sizeof(buf));
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

	if (String)
	{
		if (len < 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				if (!strcmp(GetIniString(StartNum, buf, sizeof(buf), TRUE), String))
					return (i);
			}
		}
		else if (len > 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				if (!strncmp(GetIniString(StartNum, buf, sizeof(buf), TRUE), String, len))
					return (i);
			}
		}
		else
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				const char* s = GetIniString(StartNum, buf, sizeof(buf), TRUE);
				if (!strncmp(s, String, strlen(s)))
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

	if (String)
	{
		if (len < 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				if (!stricmp(GetIniString(StartNum, buf, sizeof(buf), TRUE), String))
					return (i);
			}
		}
		else if (len > 0)
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				if (!strnicmp(GetIniString(StartNum, buf, sizeof(buf), TRUE), String, len))
					return (i);
			}
		}
		else
		{
			for (i = 0; StartNum <= EndNum; i++, StartNum++)
			{
				const char* s = GetIniString(StartNum, buf, sizeof(buf), TRUE);
				if (!strnicmp(s, String, strlen(s)))
					return (i);
			}
		}
	}

	return (-1);
}


// Use this to load up an array of resource strings ... be sure to allocate the array first
void LoadCStringArray(CString *StrArray, UINT StartNum, UINT EndNum)
{
	if (StartNum > EndNum)
	{
		ASSERT(0);
		return;
	}

	int numItems = EndNum - StartNum + 1;
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
    char String[256]; 
    char Section[64];
	char Entry[64];

	
	for( CacheIteratorType ci = g_IniEntries.begin(); ci != g_IniEntries.end(); ci++)
	{
		CIniEntry* IniEntry = (*ci).second;
		int StringNum = (*ci).first;
		
		if (!(IniEntry->m_Status & IES_DIRTY))
			continue;
		
		int SectionNum = StringNum / 100 * 100;
		if (SectionNum < MAX_SETTINGS_SECTION_ID)
			SectionNum = IDS_INISN_SETTINGS;
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
			*String = 0;
		else
		{
        	// Put quotes around in leading or trailing space
			strncpy(String, (const char *)IniEntry->m_StringData, sizeof(String) - 2);
			String[sizeof(String) - 2] = 0;
			if (*String == ' ' || String[strlen(String) - 1] == ' ')
			{
				strcat(String, "\"");
				memmove(String + 1, String, strlen(String) + 1);
				*String = '"';
			}
    	}

		// password stuff goes to current personality
		if ( StringNum == IDS_INI_SAVE_PASSWORD_TEXT )
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
	int nSectionNum = nID / 100 * 100;
	if (nSectionNum < MAX_SETTINGS_SECTION_ID)
		nSectionNum = IDS_INISN_SETTINGS;

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

	{
		char* p_newline = strchr(key, '\n');
		if (NULL == p_newline)
		{
			ASSERT(0);
			return FALSE;
		}

		*p_newline = '\0';
	}

	//
	// Flush a Eudora INI setting string from the INI file.
	//
	::WritePrivateProfileString(section, key, NULL, INIPath);
	return TRUE;
}


