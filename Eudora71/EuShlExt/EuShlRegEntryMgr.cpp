#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <tchar.h>

#include "ShellHookImpl.h"
#include "EuShlRegEntryMgr.h"
#include "RegDefines.h"

// The next 3 functions are copied from Eudora's Code base and slightly modified
TCHAR* TrimWhitespaceMT(TCHAR* pszBuffer)
{
	if (!pszBuffer || !*pszBuffer)
		return pszBuffer;

	//
	// Seek to first non-space character.
	//
	TCHAR *pszBufPtr = pszBuffer;
	while (pszBufPtr && _istspace((int)(TCHAR)*pszBufPtr))
		pszBufPtr++;

	//
	// Save start character.
	//
	TCHAR *pszBegin = pszBufPtr++;

	for (TCHAR* pszEnd = pszBufPtr; *pszBufPtr; pszBufPtr++)
		if (!_istspace((int)(TCHAR)*pszBufPtr))
			pszEnd = pszBufPtr;

	// if buf is 1 char long and 'pszEnd' is a WS, terminate it
	if (*pszEnd && _istspace((int)(TCHAR)*pszEnd))
		*pszEnd = '\0';
	else if (*pszEnd)
		pszEnd[1] = '\0';

	//
	// Copy trimmed string in-place, if necessary.
	//
	if (pszBegin != pszBuffer)
		_tcscpy(pszBuffer, pszBegin);

	return pszBuffer;
}

// Check if the file exists
bool FileExistsMT(const TCHAR* pszFilename, bool IsWritable = false)
{
	if (!pszFilename || !*pszFilename)
	{
		return false;
	}

	if (access(pszFilename, IsWritable? 2 : 0) == 0)
		return true;

	return false;
}

// Gets the MS & LS version information of a specified file.
bool GetFileVersion(LPCTSTR szDLLName, DWORD& dwMajorVersion, DWORD& dwMinorVersion)
{
	dwMajorVersion = 0;
	dwMinorVersion = 0;

	TCHAR	szString[1024] = {0};
	bool	bRet = false;

	DWORD	dwHandle = 0;
	DWORD	dwSize = GetFileVersionInfoSize((LPTSTR)szDLLName, &dwHandle);
	
	if (dwSize)
	{
		LPSTR pData = new char[dwSize];
		VS_FIXEDFILEINFO* pFileInfo = NULL;
		UINT uBufSize = 0;

		if (pData && GetFileVersionInfo((LPTSTR)szDLLName, dwHandle, dwSize, pData)) 
		{
			//
			// Okay, you would think that passing a raw string literal to an
			// argument that takes a string pointer would be okay.  Well, not
			// if you're running the 16-bit VerQueryValue() function under
			// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
			// the workaround is to copy the string to a local buffer first,
			// then pass the string in the buffer.  This, for some inexplicable
				// reason, works under all Windows OSes.
			//
			_tcscpy(szString, "\\");
			
			if (VerQueryValue(pData, szString, (void **)&pFileInfo, &uBufSize) && uBufSize && pFileInfo)
			{
				// now check the version
				dwMajorVersion = pFileInfo->dwFileVersionMS;
				dwMinorVersion = pFileInfo->dwFileVersionLS;
				bRet = true;
			}
		}

		delete [] pData;
	}	

	return bRet;
}

// This registers the DLL based on the following criteria
// If lpszDLLPath is NULL, then it registers itself (this DLL).
// bForce forcefully registers the DLL specified in the lpszDLLPath
void CEuShlRegEntryMgr::RegisterDLL(TCHAR* lpszDLLPath, BOOL bForce)
{
	TCHAR	szData[1024] = {0};	
	DWORD	dwDataBufSize=1024;	
	
	DWORD	dwKeyDataType;	
	HKEY	hKey = NULL;	

	TCHAR	szDllPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT] = {0};
	bool	bRegister = false;

	if (lpszDLLPath != NULL)		
		_tcscpy(szDllPath,lpszDLLPath);
	else
	{
		// Get this module's path & copy that
		_tcscpy(szDllPath,lpszDLLPath);
	}

	if (FileExistsMT(szDllPath))
	{	
		if (!bForce)
		{
			DWORD dwMyVersionMS = 0, dwMyVersionLS = 0;
			DWORD dwAlreadyRegVersionMS = 0, dwAlreadyRegVersionLS = 0;

			// Get Version of EuShlExt.dll for the passed lpszDLLPath or for "this" DLL.
			GetFileVersion(szDllPath, dwMyVersionMS , dwMyVersionLS );

			// Register this version of EuShlExt only if it's recent than the one already registered.
			if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
			{
				// It need not be initialized, but just for the heck of not passing it uninitialized, lets initialize it to the 
				// return type we are expecting
				dwKeyDataType = REG_SZ;	
				if (RegQueryValueEx(hKey, LM_REG_DLLPATH_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
				{
					if ( ( !_tcscmp((LPCTSTR)szData,"") ) || 
						 (! FileExistsMT((LPTSTR)szData) ) )
						bRegister = true;
					else if (FALSE != _tcscmp(szData, szDllPath))
					{
						GetFileVersion((LPTSTR)szData, dwAlreadyRegVersionMS, dwAlreadyRegVersionLS);
						
						if ( (dwMyVersionMS > dwAlreadyRegVersionMS) || 
							 ((dwMyVersionLS > dwAlreadyRegVersionLS) && (dwAlreadyRegVersionMS >= dwMyVersionMS)) )
							bRegister = true;
						else
							bRegister = false;
					}
				}
				else
					bRegister = true;
			}
			else
				bRegister = true;
		}
		else
			bRegister = true;

		if (bRegister)
		{	
			
			HINSTANCE	hLib = NULL;
			FARPROC		lpDllEntryPoint = NULL;
			
			hLib = LoadLibrary(szDllPath);

			try
			{
				if (hLib != NULL)
				{		
					// Find the entry point.
					(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, _T("DllRegisterServer"));
					if (lpDllEntryPoint != NULL)
					{
						(*lpDllEntryPoint)();				 
						DumpLogInformation("Registering from - %s",szDllPath);					
					}

				}
				else
					DumpLogInformation("LoadLibrary failed with error - %d",GetLastError());					
			}
			catch(...)
			{
				DumpLogInformation("Exception caught while registering EuShlExt");
			}

			if (hLib) 
				FreeLibrary(hLib);

			if (hKey)
			{
				if (FileExistsMT(szDllPath))
					RegSetValueEx(hKey, LM_REG_DLLPATH_KEY, 0, REG_SZ,(LPBYTE)(LPCTSTR)szDllPath , _tcslen(szDllPath) + SIZEOF_NULL_TERMINATOR);
			}
			
		}
		if (hKey)
			RegCloseKey(hKey);
	}

}

// Unregisters the DLL that is currently registered.
void CEuShlRegEntryMgr::UnregisterDLL()
{
	TCHAR	szData[1024] = {0};	
	DWORD	dwDataBufSize=1024;	
	
	DWORD	dwKeyDataType;	
	HKEY	hKey = NULL;	

	// Unregister the EuShlExt.dll that is currently registered. The LM_REG_DLLPATH_KEY Registry Entry is the one that has the path for the DLL.

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		// It need not be initialized, but just for the heck of not passing it uninitialized, lets initialize it to the 
		// return type we are expecting				
		dwKeyDataType = REG_SZ;
		if (RegQueryValueEx(hKey, LM_REG_DLLPATH_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
		{
			HINSTANCE hLib = NULL;
			FARPROC lpDllEntryPoint = NULL;
			hLib = LoadLibrary((LPTSTR)szData);

			try
			{
				if (hLib != NULL)
				{		
					// Find the entry point.
					(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, _T("DllUnregisterServer"));
					if (lpDllEntryPoint != NULL)
					{
						(*lpDllEntryPoint)();				 
						RegDeleteValue(hKey,"DllPath");
						DumpLogInformation("Unregistering from - %s",(LPTSTR)szData);					
					}
				}
				else
				{
					DumpLogInformation("LoadLibrary failed with error - %d",GetLastError());
					
				}
			}
			catch(...)
			{
				DumpLogInformation("Exception caught while Unregistering EuShlExt");
			}

			if (hLib) 
				FreeLibrary(hLib);
			
		}
		RegCloseKey(hKey);
	}
}

short CEuShlRegEntryMgr::GetPathIndex(LPCTSTR lpcszFileName)
{
	TCHAR	szData[1024] = {0};
	TCHAR	szValueName[1024] = {0};
	DWORD	dwDataBufSize=1024;	
	
	DWORD	dwKeyDataType;	
	HKEY	hKey = NULL;	

	TCHAR	szFileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT + SIZEOF_NULL_TERMINATOR] = {0};
	
	DWORD	dwPathCount = 0;
	DWORD	dwPathIndex = 0;

	_tcscpy(szFileName,lpcszFileName);

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		// It need not be initialized, but just for the heck of not passing it uninitialized, lets initialize it to the 
		// return type we are expecting
		dwKeyDataType = REG_DWORD;				
		if (RegQueryValueEx(hKey, LM_REG_PATHCOUNT_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
		{
			if (dwKeyDataType == REG_DWORD)			
				dwPathCount = *((DWORD*)szData);
			else
				dwPathCount = 0;

			for (DWORD i = 0; i < dwPathCount; i++)
			{
				sprintf(szValueName, LM_REG_PATH_KEY, i+1);
				dwKeyDataType = REG_SZ;
				if (RegQueryValueEx(hKey, szValueName, NULL, &dwKeyDataType, (LPBYTE)szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
				{
					if (!_tcscmp(szData,""))
						continue;
					if(_tcsstr( _tcslwr(szFileName),  _tcslwr(szData)) )
					{
						// Found the path
						dwPathIndex = i + 1;
						break;
					}
				}
			}
		}

		RegCloseKey(hKey);		
	}
	
	return (short) dwPathIndex;
}

bool CEuShlRegEntryMgr::UpdateSetting(short nPathIndex, EuShlRegSettings Setting, DWORD dwValue)
{
	TCHAR	szValueName[1024] = {0};	
	HKEY	hKey = NULL;	

	switch (Setting)
	{
	case _EUSHL_ALREADY_WARNED :
		if ( (RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) )
		{
			sprintf(szValueName, LM_REG_ALREADYWARNED_KEY,nPathIndex);
			RegSetValueEx(hKey, szValueName, 0, REG_DWORD, (const PBYTE) &dwValue, sizeof(DWORD));
			RegCloseKey(hKey);
		}
		break;
		default:
			break;
	}	
	return true;
}


bool CEuShlRegEntryMgr::UpdateSetting(short nPathIndex, EuShlRegSettings Setting, LPCTSTR lpcszStr)
{
	TCHAR	szValueName[1024] = {0};	
	HKEY	hKey = NULL;

	if(!lpcszStr)
		return false;

	TCHAR*	pszTempBuf = new TCHAR[ _tcslen(lpcszStr) + (2 * SIZEOF_NULL_TERMINATOR) ];	// size of passed string + terminal NULL + storage for appending "|" if required
	_tcscpy(pszTempBuf, lpcszStr);
	
	switch (Setting)
	{
		case _EUSHL_WARN_EXTENSION :
			if ( (RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) )
			{
				if (pszTempBuf[_tcslen(pszTempBuf) - 1] != '|')
					_tcscat(pszTempBuf,"|"); 

				sprintf(szValueName,LM_REG_WARN_KEY,nPathIndex);				
				RegSetValueEx(hKey,szValueName,0, REG_SZ,(LPBYTE)pszTempBuf , _tcslen(pszTempBuf) + SIZEOF_NULL_TERMINATOR);

				RegCloseKey(hKey);
			}
		break;
		case _EUSHL_DO_NOT_WARN_EXTENSION :
			if ( (RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) )
			{
				if (pszTempBuf[_tcslen(pszTempBuf) - 1] != '|')
					_tcscat(pszTempBuf,"|"); 
				
				sprintf(szValueName, LM_REG_DONOTWARN_KEY, nPathIndex);
				RegSetValueEx(hKey,szValueName,0, REG_SZ,(LPBYTE)pszTempBuf, _tcslen(pszTempBuf) + SIZEOF_NULL_TERMINATOR);
				
				RegCloseKey(hKey);
			}
		break;
		default:
			break;
	}	

	delete[] pszTempBuf;

	return true;
}

short CEuShlRegEntryMgr::AddPathInRegistry(LPCTSTR lpcszPath)
{
	if (!lpcszPath)
		return false;

	// We have to make a local copy because we need to use _tcslwr later on
	TCHAR*	lpszPath = new TCHAR[ _tcslen(lpcszPath) + SIZEOF_NULL_TERMINATOR];	// size of passed string + terminal NULL
	_tcscpy(lpszPath, lpcszPath);
	
	bool bPathFound = false;

	TCHAR		szData[1024] = {0};
	TCHAR		szValueName[1024] = {0};
	DWORD dwDataBufSize=1024;	
	
	DWORD dwKeyDataType;	
	HKEY		hKey = NULL;	

	DWORD dwPathCount = 0;
	DWORD dwPathIndex = 0;

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		// It need not be initialized, but just for the heck of not passing it uninitialized, lets initialize it to the 
		// return type we are expecting		
		dwKeyDataType = REG_DWORD;
		if (RegQueryValueEx(hKey, LM_REG_PATHCOUNT_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
		{
			if (dwKeyDataType == REG_DWORD)			
				dwPathCount = *((DWORD*)szData);
			else
				dwPathCount = 0;
				
			for (DWORD i = 0; i < dwPathCount; i++)
			{
				sprintf(szValueName, LM_REG_PATH_KEY,i+1);

				// It need not be initialized, but just for the heck of not passing it uninitialized, lets initialize it to the 
				// return type we are expecting
				dwKeyDataType = REG_SZ;
				if (RegQueryValueEx(hKey, szValueName, NULL, &dwKeyDataType, (LPBYTE)szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
				{
					if(_tcsstr( _tcslwr(lpszPath),_tcslwr(szData)) )
					{
						// Found the attachment dir
						bPathFound = true;
						dwPathIndex = i + 1;
						break;
					}
					else if (_tcsstr( _tcslwr(szData), _tcslwr(lpszPath)) )
					{
						// Update the path variable to reflect the new path
						sprintf(szValueName, LM_REG_PATH_KEY,i + 1);					
						RegSetValueEx(hKey,szValueName,0, REG_SZ,(LPBYTE)lpszPath, _tcslen(lpszPath) + SIZEOF_NULL_TERMINATOR);
					}
				}
			}

			if (!bPathFound)
			{
				dwPathCount++;	
				dwPathIndex = dwPathCount;

				// Compose a new value
				sprintf(szValueName, LM_REG_PATH_KEY, dwPathCount);					
				RegSetValueEx(hKey,szValueName,0, REG_SZ,(LPBYTE)lpszPath, _tcslen(lpszPath) + SIZEOF_NULL_TERMINATOR);
				RegSetValueEx(hKey, LM_REG_PATHCOUNT_KEY, 0, REG_DWORD,(const PBYTE) &dwPathCount, sizeof( DWORD ));
			}
		}
		else
		{
			// This should not happen 'coz this is taken care of in Initialize()
			dwPathCount = 0;
			dwPathIndex = 0;
			RegSetValueEx(hKey, LM_REG_PATHCOUNT_KEY, 0, REG_DWORD,(const PBYTE) &dwPathCount, sizeof( DWORD ));	
		}

		RegCloseKey(hKey);		
	}
	else
	{
		// Code duplicated from above ...
		// This should not happen 'coz this is taken care of in Initialize()
		dwPathCount = 0;
		dwPathIndex = 0;
		RegSetValueEx(hKey, LM_REG_PATHCOUNT_KEY, 0, REG_DWORD,(const PBYTE) &dwPathCount, sizeof( DWORD ));	
	}

	if (dwPathIndex)
		CompactRegistryEntries();

	delete[] lpszPath;		
	return (short)dwPathIndex;
}

// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
void CEuShlRegEntryMgr::RemovePathFromRegistry(LPCTSTR lpcszPath)
{
	if (!lpcszPath)
		return;

	ReplacePathWithNew(lpcszPath,"");
	CompactRegistryEntries();
}	

// This function removes any redundant and empty LM_REG_PATH_KEY entries (A Path#y entry of a directory within a directory, 
// which also has a Path#x entry is considered redundant)	
void CEuShlRegEntryMgr::CompactRegistryEntries()
{
	TCHAR	szData[1024] = {0};
	TCHAR	szValueName[1024] = {0};
	DWORD	dwDataBufSize=1024;	
	
	DWORD	dwKeyDataType;	
	HKEY	hKey = NULL;	

	DWORD	dwPathCount = 0;
	LPSTR	pBuffer = NULL;   

	PVALENT	pValent = NULL;
	bool	bRewrite = false;

	LONG	lResult;

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		// It need not be initialized, but just for the heck of not passing it uninitialized, lets initialize it to the 
		// return type we are expecting				
		dwKeyDataType = REG_DWORD;
		if (RegQueryValueEx(hKey, LM_REG_PATHCOUNT_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
		{
			if (dwKeyDataType == REG_DWORD)			
				dwPathCount = *((DWORD*)szData);
			else
				dwPathCount = 0;

			try
			{
				pValent = new VALENT[dwPathCount];
				for (DWORD i = 0; i < dwPathCount; i++)
				{
					pValent[i].ve_valuename = new TCHAR[10];
					sprintf(pValent[i].ve_valuename , LM_REG_PATH_KEY,i+1);								
				}			

				dwDataBufSize = 0;
				if (ERROR_MORE_DATA == (lResult = RegQueryMultipleValues(hKey,pValent, dwPathCount, NULL, &dwDataBufSize)))
				{
					// Allocate a buffer for the version string.						
					pBuffer = new TCHAR [dwDataBufSize];
					if (pBuffer) 
					{
						bool bInc = false;
						RegQueryMultipleValues(hKey,pValent, dwPathCount, pBuffer, &dwDataBufSize);
						i = 0;
						while (i < (int) dwPathCount)
						{
							//DumpLogInformation("Entries before compacting : %s",(LPCSTR)pValent[i].ve_valueptr);
							// I do not want to allocate more memory, hence manipulate the Valent Entries directly
							if (_tcscmp(TrimWhitespaceMT((LPSTR)pValent[i].ve_valueptr),_T("")) )
							{
								for (int j = i + 1; j < (int) dwPathCount; j++)
								{
									if (! _tcscmp(TrimWhitespaceMT((LPSTR)pValent[j].ve_valueptr),_T("")) )
										continue;

									if(_tcsstr ( (LPCTSTR) _tcslwr((LPTSTR)pValent[j].ve_valueptr), _tcslwr((LPTSTR)pValent[i].ve_valueptr) ) )
									{
										_tcscpy((LPTSTR)pValent[j].ve_valueptr,"");
										bRewrite = true;
									}
									else if (_tcsstr( (LPCTSTR) _tcslwr((LPTSTR)pValent[i].ve_valueptr), _tcslwr((LPTSTR)pValent[j].ve_valueptr) ) )
									{
										// Update the path variable to reflect the new path
										_tcscpy((LPTSTR)pValent[i].ve_valueptr,(LPTSTR)pValent[j].ve_valueptr);
										_tcscpy((LPTSTR)pValent[j].ve_valueptr,"");																	
										bRewrite = true;
									}
									
								}
								if (bInc == true)
								{
									i++;
									bInc = false;
								}
								else
									bInc = true;
							}
							else
							{
								bRewrite = true;
								i++;
							}
						}

						if (bRewrite)
						{
							DWORD dwNewPathCount = 0;
							for (i = 0; i < (int) dwPathCount; i++)
							{
								DumpLogInformation("Entries after compacting : %s",(LPCSTR)pValent[i].ve_valueptr);
								if (! _tcscmp((LPCSTR)pValent[i].ve_valueptr,_T("")) )
									continue;
								else
								{
									dwNewPathCount ++;
									sprintf(szValueName, LM_REG_PATH_KEY, dwNewPathCount);
									RegSetValueEx(hKey,szValueName,0, REG_SZ,(LPBYTE)(LPCTSTR)pValent[i].ve_valueptr, 1 + _tcslen((LPCSTR)pValent[i].ve_valueptr));
								}
							}

							DumpLogInformation("Entries after compacting : PathCount %d",dwNewPathCount);

							for (i = dwNewPathCount; i < dwPathCount; i++)
							{
								sprintf(szValueName, LM_REG_PATH_KEY, i + 1);
								RegDeleteValue(hKey, szValueName);
								sprintf(szValueName, LM_REG_WARN_KEY,i + 1);
								RegDeleteValue(hKey, szValueName);
								sprintf(szValueName, LM_REG_DONOTWARN_KEY,i + 1);
								RegDeleteValue(hKey, szValueName);
								sprintf(szValueName, LM_REG_ALREADYWARNED_KEY,i + 1);
								RegDeleteValue(hKey, szValueName);
							}

							RegSetValueEx(hKey, LM_REG_PATHCOUNT_KEY, 0, REG_DWORD,(const PBYTE) &dwNewPathCount, sizeof( DWORD ));
						}

					}

					if(pBuffer)
						delete[] pBuffer;
				}

				if (pValent)
				{
					for (i = 0; i < dwPathCount; i++)
						delete[] pValent[i].ve_valuename;
					delete[] pValent;
				}
			}

			catch(...)
			{
				// Write this to the log
			}
		}
		
		RegCloseKey(hKey);		
	}			
}

bool CEuShlRegEntryMgr::ReplacePathWithNew(LPCTSTR lpcszOldPath, LPCTSTR lpcszNewPath)
{
	DWORD dwPath = 0;
	bool bRet = false;

	if ( dwPath = GetPathIndex(lpcszOldPath) )
	{
		TCHAR		szValueName[1024];
		HKEY		hKey = NULL;	

		if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
		{
			sprintf(szValueName, LM_REG_PATH_KEY, dwPath);					
			RegSetValueEx(hKey,szValueName,0, REG_SZ,(LPBYTE)lpcszNewPath, _tcslen(lpcszNewPath) + SIZEOF_NULL_TERMINATOR);
			RegCloseKey(hKey);		
		}
		
		CompactRegistryEntries();
		bRet = true;
	}

	return bRet;
}

void CEuShlRegEntryMgr::UpdateAlreadyWarnedSetting(LPCTSTR lpcszFileName,bool bValue)
{
	short nPathIndex = gEuShlRegEntryMgr.GetPathIndex(lpcszFileName);
	gEuShlRegEntryMgr.UpdateSetting(nPathIndex, _EUSHL_ALREADY_WARNED ,bValue);
}

