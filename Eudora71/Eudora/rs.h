// RS.H
// Constant definitions for resource strings


#ifndef _RS_H_
#define _RS_H_

#ifndef RC_INVOKED

// Data Types

class CRString : public CString
{
public:
	CRString(UINT StringID) { LoadString(StringID); }
};


// Data Items

extern CString DefaultINIPath;
extern CString INIPath;


bool GetEudoraProfileString( LPCTSTR lpSectionName, LPCTSTR lpKeyName, CString *sValueStr);
bool GetEudoraProfileInt(LPCTSTR lpSectionName, LPCTSTR lpKeyName, int *sValueInt);
bool GetEudoraProfileLong(LPCTSTR lpSectionName, LPCTSTR lpKeyName, long *sValueLong);
bool GetEudoraProfileULong(LPCTSTR lpSectionName, LPCTSTR lpKeyName, unsigned long *sValueInt);    // destination value

bool WriteEudoraProfileString(LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpString);
bool WriteEudoraProfileInt(LPCTSTR lpSectionName, LPCTSTR lpKeyName, const int nValue);
bool WriteEudoraProfileLong(LPCTSTR lpSectionName, LPCTSTR lpKeyName, const long nValue);
bool WriteEudoraProfileULong(LPCTSTR lpSectionName, LPCTSTR lpKeyName, const unsigned long nValue);

void RemoveIniKey(UINT StringNum);

// Function Prototypes
bool GetIniString(const UINT StringNum, CString& str);
bool GetDefaultIniSetting(const UINT StringNum, CString& str);

const char *GetIniString(UINT StringNum, char* Buffer = NULL, int size = -1, BOOL getname = FALSE);
void GetIniStringList(UINT in_nStringNum, char in_cDelimeter, CStringList & out_listResourceStrings);
short GetIniShort(UINT StringNum);
long GetIniLong(UINT StringNum);
inline int GetIniInt(UINT StringNum)
	{ return (GetIniLong(StringNum)); }

BOOL GetIniWindowPos(UINT StringNum, long& left, long& top, long& right, long& bottom);
inline BOOL GetIniWindowPos(UINT StringNum, RECT& rect)
	{ return (GetIniWindowPos(StringNum, rect.left, rect.top, rect.right, rect.bottom)); }

BOOL ConvertHexToColor(LPCSTR pHexStr, COLORREF& clr);
BOOL GetIniColorRef(UINT StringNum, COLORREF& clr);

BOOL SetIniString(UINT StringNum, const char *String = NULL);
BOOL SetIniShort(UINT StringNum, short Num);
BOOL SetIniLong(UINT StringNum, long Num);
inline int SetIniInt(UINT StringNum, int Num)
	{ return (SetIniLong(StringNum, Num)); }

BOOL SetIniWindowPos(UINT StringNum, long left, long top, long right, long bottom);
inline BOOL SetIniWindowPos(UINT StringNum, const RECT& rect)
	{ return (SetIniWindowPos(StringNum, rect.left, rect.top, rect.right, rect.bottom)); }

BOOL SetIniColorRef(UINT StringNum, COLORREF clr);

void NotifyIniUpdates(UINT nStringNum);

int CompareRString(UINT StringNum, const char* String, int len = -1);
int CompareRStringI(UINT StringNum, const char* String, int len = -1);

int FindRStringIndex(UINT StartNum, UINT EndNum, const char* String, int len = -1);
int FindRStringIndexI(UINT StartNum, UINT EndNum, const char* String, int len = -1);

void StripLeadingStrings(CString & io_strToModify, CString & io_strStripStrings, char in_cDelimeter);
void StripTrailingStrings(CString & io_strToModify, CString & io_strStripStrings, char in_cDelimeter);
inline void StripLeadingResourceStrings(CString & io_strToModify, UINT in_nResourceID, char in_cDelimeter)
		{
			CRString	strResourceString(in_nResourceID);
	
			StripLeadingStrings(io_strToModify, strResourceString, in_cDelimeter);
		}
inline void StripTrailingResourceStrings(CString & io_strToModify, UINT in_nResourceID, char in_cDelimeter)
		{
			CRString	strResourceString(in_nResourceID);
			
			StripTrailingStrings(io_strToModify, strResourceString, in_cDelimeter);
		}
inline void StripLeadingSettingStrings(CString & io_strToModify, UINT in_nSettingID, char in_cDelimeter)
		{
			CString		strSettingString;

			GetIniString(in_nSettingID, strSettingString);

			StripLeadingStrings(io_strToModify, strSettingString, in_cDelimeter);
		}
inline void StripTrailingSettingStrings(CString & io_strToModify, UINT in_nSettingID, char in_cDelimeter)
		{
			CString		strSettingString;

			GetIniString(in_nSettingID, strSettingString);
			
			StripTrailingStrings(io_strToModify, strSettingString, in_cDelimeter);
		}


bool ResourceStringAppearsInString(const char * in_szSearch, UINT in_nResourceID, char in_cDelimeter);

bool StringStartsWithString(const char * in_szSearch, CString & io_strStartStrings, char in_cDelimeter);
bool StringStartsWithStringInList(const char * in_szSearch, CStringList & in_listStrings, POSITION & out_posFoundTag);
bool StringEndsWithString(const char * in_szSearch, CString & io_strEndStrings, char in_cDelimeter);
inline bool StringStartsWithResourceString(const char * in_szSearch, UINT in_nStartStringsResID, char in_cDelimeter)
		{
			CRString	strStartStrings(in_nStartStringsResID);
	
			return StringStartsWithString(in_szSearch, strStartStrings, in_cDelimeter);
		}
inline bool StringEndsWithResourceString(const char * in_szSearch, UINT in_nEndStringsResID, char in_cDelimeter)
		{
			CRString	strEndStrings(in_nEndStringsResID);
			
			return StringEndsWithString(in_szSearch, strEndStrings, in_cDelimeter);
		}
inline bool StringStartsWithSettingString(const char * in_szSearch, UINT in_nStartStringsSettingID, char in_cDelimeter)
		{
			CString		strStartStrings;

			GetIniString(in_nStartStringsSettingID, strStartStrings);

			return StringStartsWithString(in_szSearch, strStartStrings, in_cDelimeter);
		}
inline bool StringEndsWithSettingString(const char * in_szSearch, UINT in_nEndStringsSettingID, char in_cDelimeter)
		{
			CString		strEndStrings;

			GetIniString(in_nEndStringsSettingID, strEndStrings);
			
			return StringEndsWithString(in_szSearch, strEndStrings, in_cDelimeter);
		}

void LoadCStringArray(CString *StrArray, UINT StartNum, UINT EndNum);
int FindCStringArrayIndex(int numItems, CString *StrArray, const char *String, int len = -1);
int FindCStringArrayIndexI(int numItems, CString *StrArray, const char *String, int len = -1);

int LoadOpenWindows();
int SaveOpenWindows(int Close = TRUE);

int FlushINIFile();
int SetupINIFilename(const char* Filename = NULL);
void IniStringCleanUp();

void RemoveIniFromCache(UINT StringNum);
void RemoveAllFromCache(void);

BOOL PurgeIniSetting(int nID);

#endif


#endif
