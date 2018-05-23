/////////////////////////////////////////////////////////////////////////////
// File: services.h
//
// Houses several miscellaneous, but globally-useful functions.
/////////////////////////////////////////////////////////////////////////////

#ifndef __SERVICES_H__
#define __SERVICES_H__

__declspec(dllexport) ULONG HashMT(const char* pszHashStr);
__declspec(dllexport) ULONG HashMTIgnoreCase(const char* pszHashStr);
__declspec(dllexport) BOOL IsMainThreadMT();

//
// Generic time formatting routines.
//
__declspec(dllexport) char* TimeDateStringMT(char* buf, long Seconds, BOOL DoDate);
__declspec(dllexport) char* TimeDateStringFormatMT(char* buf, long Seconds, int TimeZoneMinutes, const char* Format);
__declspec(dllexport) CString FormatTimeMT( time_t theTime, const char * pFormat );
__declspec(dllexport) HRESULT GetGMTOffsetMT(const char* pszTimeZone, int* pOffset);

//
// Real thin layer on CStringList.
//
class AFX_EXT_CLASS CSortedStringListMT : public CStringList
{
public:
	CSortedStringListMT() : CStringList() {}

	void Add( LPCSTR lpString );	// does a sorted insert
	void Add( CString & aString );	// does a sorted insert
//FORNOW	void Dump( void );

protected:
	// Don't allow copy constructor or assignment operator
	CSortedStringListMT(CSortedStringListMT & in_copy);
	operator=(CSortedStringListMT & in_rhs);
};

//
// Safe string operations that are lenient about NULL pointers.
//
__declspec(dllexport) char* SafeStrdupMT(const char* string);
__declspec(dllexport) size_t SafeStrlenMT(const char* string);

//
// Trim leading and trailing whitespace.
//
__declspec(dllexport) char* TrimWhitespaceMT(char* buf);
__declspec(dllexport) const char* TrimWhitespaceMT(CString& cs);

//
// Routine to get rid of prefixes.  Handy for ditching Re: part of subjects.
//
__declspec(dllexport) const char* RemovePrefixMT(const char* Prefix, const char* String);

//
// Routine for removing subject prefixes. Very fast.
//
__declspec(dllexport) const char *RemoveSubjectPrefixMT(const char *pSubjectStr);

//
// Byte swapping routines.
//
__declspec(dllexport) unsigned short SwapShortMT(unsigned short sword);
__declspec(dllexport) unsigned long SwapLongMT(unsigned long lword);

//
// File system routines.
//
__declspec(dllexport) DWORD GetAvailableSpaceMT( const char* Drive );	// like 'A', 'a', 'C', EudoraDir, etc.
__declspec(dllexport) char* SetFileExtensionMT(char* Filename, const char* Extension);
__declspec(dllexport) char* StripIllegalMT(char* Filename, const CString& strTargetDirectory);
__declspec(dllexport) BOOL FileExistsMT(const char* Filename, BOOL IsWritable = FALSE);
__declspec(dllexport) BOOL CreateDirectoryMT(LPCTSTR DirectoryName);
__declspec(dllexport) HRESULT CreateShortcutMT(LPCTSTR Target, LPCTSTR Link, LPCTSTR Arguments = NULL, LPCTSTR WorkingDir = NULL);
__declspec(dllexport) HRESULT FileRenameMT(const char* OldName, const char* NewName);
__declspec(dllexport) HRESULT FileRenameReplaceMT(const char * in_szOldName, const char * in_szNewName, BOOL in_bReplace = TRUE);
__declspec(dllexport) HRESULT FileRemoveMT(const char* Filename);
__declspec(dllexport) void StripQuotesMT(char* line);
__declspec(dllexport) BOOL LongFileSupportMT(const char *path);
__declspec(dllexport) CString GetTmpFileNameMT(const char *Prefix = "eud");
__declspec(dllexport) HINSTANCE GotoURL(LPCTSTR url, int showcmd);

//
// Routine for detecting if a specific executable is currently running
//
__declspec(dllexport) BOOL IsCurrentlyExecuting(LPCTSTR lpszModuleName,DWORD *dwProductVersionMS = NULL, DWORD *dwProductVersionLS = NULL);

//
// Routine for detecting if a netscape is currently executing & if so is the version greater than or equal to 4.05
//
__declspec(dllexport) BOOL IsNetscapeAbove405Executing();

//
// StripAddress, Gets only the pertinent parts from an address
//
__declspec(dllexport) char* StripAddressMT(char* line);

//
// Routines for converting RFC822 time from String to long & vice-versa
//
_declspec(dllexport) void FormatDateMT(const char* GMTOffset, long &lSeconds, int &nTimeZoneMinutes);
_declspec(dllexport) int ComposeDateMT(char* DateBuf, unsigned long GMTTime, int TimeZoneMinutes, BOOL bEudorasSMTPFormat = TRUE);

_declspec(dllexport) long GetTimeMT(const char* Line, BOOL FromLine);

//
// Limited routines for Power Management & getting Disk Spun state
//
_declspec(dllexport) BOOL IsDiskSpun(BOOL &bSpunState);
_declspec(dllexport) BOOL IsSystemOnBattery(BOOL &bOnBattery);
_declspec(dllexport) BOOL ShouldFileIOBeDone(BOOL &bShouldFileIOBeDone);
// Sets a temp directory, where the file will be created, which will then be opened & used for the above IsDiskSpun API.
_declspec(dllexport) void SetTempFileDirectory(LPCTSTR lpszDataDirectory);

#endif // __SERVICES_H__
