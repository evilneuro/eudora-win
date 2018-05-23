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

int CompareRString(UINT StringNum, const char* String, int len = -1);
int CompareRStringI(UINT StringNum, const char* String, int len = -1);

int FindRStringIndex(UINT StartNum, UINT EndNum, const char* String, int len = -1);
int FindRStringIndexI(UINT StartNum, UINT EndNum, const char* String, int len = -1);

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
