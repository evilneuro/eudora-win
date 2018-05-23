
#include "windows.h"
#include <stdio.h>
#include "ShellHookImpl.h"
#include "tchar.h"
#include "time.h"
#include "resource.h"

extern unsigned int  g_cRefThisDll;

extern DWORD TlsIndex;
extern HANDLE	g_hModule;

// The Shell Factory Class 
CShellExtClassFactory::CShellExtClassFactory()
{
    //DumpLogInformation("CShellExtClassFactory::CShellExtClassFactory()\n");
    m_cRef = 0L;	
    g_cRefThisDll++;
}


CShellExtClassFactory::~CShellExtClassFactory()				
{	
    g_cRefThisDll--;
}


STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{	
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);


    DumpLogInformation("In ShellExtClassFactory::QueryInterface()");

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}	

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{	
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);


    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

    if (--m_cRef)
        return m_cRef;

	delete this;

    return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;

	pti = (THREADINFO *)TlsGetValue(TlsIndex);

	DumpLogInformation("In ShellExtClassFactory::CreateInstance()");

	*ppvObj = NULL;

	// Shell extensions typically don't support aggregation (inheritance)
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;

	LPCSHELLEXT pShellExt = NULL;

	if (pShellExt == NULL)
	{
		try 
		{
			//Create the CShellExt object
			pShellExt = new CShellExt();  
		} catch (...) 
		{
		  // Something's gone wrong. Write it to the log.
			DumpLogInformation("In ShellExtClassFactory::CreateInstance: Unable to create ShellExt Object");
			pShellExt=NULL;
		}
	}

	if (NULL == pShellExt)
		return E_OUTOFMEMORY;

	return pShellExt->QueryInterface(riid, ppvObj);
}

STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{	
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

    //return NOERROR;
	return S_OK;
}


CShellExt::CShellExt()
{
   //DumpLogInformation("CShellExt::CShellExt()\n");

   m_cRef = 0L;
   m_pDataObj = NULL;
   m_pLMRegData = NULL;
   m_nPathCount = 0;
   g_cRefThisDll++;   

}

CShellExt::~CShellExt()
{
	//DumpLogInformation("CShellExt::~CShellExt()\n");

	if (m_pDataObj)
		m_pDataObj->Release();
	if (m_pLMRegData)
		delete [] m_pLMRegData;

	g_cRefThisDll--;
}


STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);


	*ppv = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPersistFile))
	{
		DumpLogInformation("In ShellExt::QueryInterface() --> IID_IPersistFile");
		*ppv = (LPPERSISTFILE)this;
	}
	else if (IsEqualIID(riid, IID_IShellExecuteHook))
	{
		DumpLogInformation("In ShellExt::QueryInterface() --> IID_IShellExecuteHook");
		*ppv = (IShellExecuteHook *)this;
		LoadConfiguration();
	}

	if (*ppv)
	{
		AddRef();
		return NOERROR;
	}

	DumpLogInformation("In ShellExt::QueryInterface() --> Unsupported Interface!");

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	DumpLogInformation("In ShellExt::AddRef()");
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
	DumpLogInformation("In ShellExt::Release()");

	if (--m_cRef)
	  return m_cRef;

	delete this;

   return 0L;
}

INT_PTR CALLBACK WarnDlgProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	if (uMsg == WM_INITDIALOG)
	{
		// Buffer used for loading the string from the string table
		TCHAR szBuff[1024];	

		// This buffer will contain the final text so formed.
		TCHAR szFinalTextBuff[1024 /* size of szBuff*/ + _MAX_PATH + _MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];
		
		HWND hWndControl = GetDlgItem(hwndDlg, IDC_ALERT_TEXT);		

		// Load the string fro the string table.
		LoadString((HINSTANCE) g_hModule, IDS_WARN_STRING, szBuff, sizeof(szBuff));

		sprintf(szFinalTextBuff,szBuff, lParam);	
		if (hWndControl)
			SetWindowText(hWndControl, szFinalTextBuff);
	}
	else if (uMsg == WM_COMMAND) 		
	{
		if (HIWORD(wParam) == BN_CLICKED)
		{
			// Destroy the Dialog.
			EndDialog(hwndDlg, LOWORD(wParam) );
		}		
	}
	return FALSE;
}

STDMETHODIMP CShellExt::Execute( LPSHELLEXECUTEINFO pei)
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
   
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	TCHAR		szData[1024] = {0};
	TCHAR		szValueName[1024] = {0};
	DWORD		dwDataBufSize=1024;	
	BOOL bAlreadyWarned = FALSE;
	
	DWORD		dwKeyDataType = 0;	
	HKEY		hKey = NULL;
	DWORD		dwValue = 0;

	TCHAR		szShortPathName1[_MAX_PATH + _MAX_FNAME + _MAX_EXT + SIZEOF_NULL_TERMINATOR] = {0};
	TCHAR		szShortPathName2[_MAX_PATH + _MAX_FNAME + _MAX_EXT + SIZEOF_NULL_TERMINATOR] = {0};
	static	int	sBufLength = _MAX_PATH + _MAX_FNAME + _MAX_EXT;
	
	try
	{
		for (int i = 0; i < m_nPathCount; i++)
		{			
			bAlreadyWarned = FALSE;

			// Convert to Short Path Name. Also if for some reason the entry in registry has an empty path, then make sure we skip it.
			// Any path that is registered as a path to be "protected" by Shell Extension CANNOT BE EMPTY
			if ( GetShortPathName((LPTSTR)pei->lpFile, szShortPathName1, sBufLength) && 
				 GetShortPathName(m_pLMRegData[i].szPath, szShortPathName2, sBufLength) &&
				 (_tcslen(szShortPathName2) > 0) )
			{
				if( (m_pLMRegData) && _tcsstr( _tcslwr(szShortPathName1),_tcslwr(szShortPathName2) ) )
				{
					if (IsExecutable(pei->lpFile,i+1))
					{
						// Check if already warned by Application that is launching (e.g Eudora). If so, then do not warn again	
						// It's the application's (the one that is launching/executing this file) duty to set the value of AlreadyWarned'N' to the
						// appropriate value.
						if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
						{
							sprintf(szValueName,LM_REG_ALREADYWARNED_KEY,i+1);
							if (RegQueryValueEx(hKey, szValueName, NULL, &dwKeyDataType, (LPBYTE)szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
							{
								bAlreadyWarned = *((DWORD*)szData);
								
								//Reset the "AlreadyWarned#n"
								sprintf(szValueName,LM_REG_ALREADYWARNED_KEY,i+1);
								dwValue = 0;
								RegSetValueEx(hKey, szValueName, 0, REG_DWORD, (const PBYTE) &dwValue, sizeof(DWORD));
							}
							RegCloseKey(hKey);
						}
						
						if (!bAlreadyWarned)
						{
							if ( (pei) && ( (long) pei->hInstApp < 32) )
								pei->hInstApp = (HINSTANCE) 32;	// This makes sure that Shell does not generate any error message box.

							if (IDCANCEL == ::DialogBoxParam( (HINSTANCE) g_hModule, MAKEINTRESOURCE(IDD_YES_NO), NULL, (DLGPROC ) &WarnDlgProc, LPARAM(pei->lpFile)))
								return S_OK;	// Let Shell know that we handled it.
							else
								return S_FALSE;	// Let Shell Handle it. We are done.
						}
					}
				}
			} else if (_tcslen(szShortPathName2) <= 0)
				DumpLogInformation("The Path, Path#%d is empty",i + 1);
		}	
	}
	catch(...)
	{
		// Something's gone wrong here ..write to the log.
		DumpLogInformation("Exception when trying to determine if Executable, Path : %s",(LPTSTR)pei->lpFile);
	}

	return S_FALSE;
}

STDMETHODIMP_(VOID) CShellExt::LoadConfiguration()
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
    
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	// This is the function that loads the configuration from the registry
	TCHAR		szData[1024] = {0};
	TCHAR		szValueName[1024] = {0};
	DWORD		dwDataBufSize=1024;	
	
	DWORD		dwKeyDataType = 0;	
	HKEY		hKey = NULL;
	
	// All the entries are under HKCU\Qualcomm\Eudora\LauchManager
	if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, LM_REG_PATHCOUNT_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
		{
			if (dwKeyDataType == REG_DWORD)
			{
				m_nPathCount = *((DWORD*)szData);
				DumpLogInformation("Loading Configuration Info : PathCount %d",m_nPathCount);
			}
			else	// Write to the log that somehow the value is corrupted
			{
				DumpLogInformation("Error Loading Configuration Info : PathCount %d ...resetting to zero",*((DWORD*)szData));
				m_nPathCount = 0;
			}
				
			// Also write to the log if the Path Count is negative			
			if (m_nPathCount < 0)
			{
				DumpLogInformation("Error PathCount value is negative : %d ...resetting to zero",*((DWORD*)szData));
				m_nPathCount = 0;
			}


			// Delete the memory allocated (if any)
			if(m_pLMRegData)
				delete [] m_pLMRegData;

			// Allocate memory here so that we could fill the data.
			m_pLMRegData = new LM_REG_DATA[m_nPathCount];

			for (int i = 0; i < m_nPathCount; i++)
			{
				// Allocate memory
				sprintf(szValueName,LM_REG_PATH_KEY,i+1);
				if (RegQueryValueEx(hKey, szValueName, NULL, &dwKeyDataType, (LPBYTE)szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
				{
					_tcsncpy(m_pLMRegData[i].szPath,(LPCTSTR)szData,_MAX_PATH > 1024 ? _MAX_PATH : 1024);
				}
			}
		}

		RegCloseKey(hKey);
	}	
	return;
}

// This function determines if the FileName so passed is one amongst the malicious file-types that could possibly contain
// viruses, worms etc when opened/executed. The list which specifies the malicious extensions is provided in the registry.
STDMETHODIMP_(BOOL) CShellExt::IsExecutable(LPCTSTR Filename, int nPathOffset)
{
	THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	TCHAR		szData[1024] = {0};
	TCHAR		szValueName[1024] = {0};
	DWORD		dwDataBufSize=1024;	
	
	DWORD		dwKeyDataType = 0;	
	HKEY		hKey = NULL;
	
	TCHAR		szWarnExtensions[1024] = {0};
	TCHAR		szDoNotWarnExtensions[1024] = {0};

	BOOL		bFound = FALSE;

	if (!Filename)
		return FALSE;

	LPSTR lpLastPeriod = _tcsrchr(Filename, '.');
	
	if (lpLastPeriod)
	{
		// We found the file's extension
		if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			// Get the WarnExtensions value for the specified Path Index
			sprintf(szValueName,LM_REG_WARN_KEY,nPathOffset);
			if (RegQueryValueEx(hKey, szValueName, NULL, &dwKeyDataType, (LPBYTE)szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
			{
				_tcscpy(szWarnExtensions,(LPCTSTR)szData);
			}

			// Get the DoNotWarnExtensions value for the specified Path Index
			sprintf(szValueName,LM_REG_DONOTWARN_KEY,nPathOffset);
			if (RegQueryValueEx(hKey, szValueName, NULL, &dwKeyDataType, (LPBYTE)szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
			{
				_tcscpy(szDoNotWarnExtensions,(LPCTSTR)szData);
			}
			RegCloseKey(hKey);
		}

		if ( szWarnExtensions[_tcslen(szWarnExtensions) - 1] != '|')
			_tcscat(szWarnExtensions,"|"); // This helps us while parsing

		TCHAR seps[]   = "|";	// token's that will be used for parsing.
		TCHAR *token;

		token = _tcstok( szWarnExtensions, seps );
		while( token != NULL )
		{
		  // While there are tokens
		  if (!_tcsnicmp(token,lpLastPeriod + 1,_tcslen(token)) )
		  {
			  // Found an extension in the WarnExtensions List
			  bFound = TRUE;
			  break;
		  }		
		  // Get next token
		  token = _tcstok( NULL, seps );
		}

		if (bFound)
		{
			token = _tcstok( szDoNotWarnExtensions, seps );
			while( token != NULL )
			{
			  // While there are tokens
			  if (!_tcsnicmp(token,lpLastPeriod + 1,_tcslen(token)) )
			  {
				  // Found an extension in the DoNotWarnExtensions List ..hence set the bFound to FALSE.
				  bFound = FALSE;
				  break;
			  }		
			  // Get next token
			  token = _tcstok( NULL, seps );
			}
		}
	}

	return bFound;
}

//  This function will dump the logging information in a file when required.
void DumpLogInformation(const TCHAR *szFormat ... )
{
	TCHAR szTempBuffer[4096] = {0};	// Sufficient enough to store the string to display
	TCHAR szBuffer[3968] = {0};
	TCHAR szDate[128], szTime[128];
	
	va_list argptr;

	// try/catch block so that if somehow the code misbehaves, we do not crash the Shell.
	try 
	{
		// Today's date
		_strdate(szDate);

		// Current time
		_strtime(szTime);

		// Format the passed arguments
		va_start( argptr, szFormat );
		wvsprintf( szBuffer, szFormat, argptr );
		va_end( argptr );

#ifdef _DEBUG
		// Wrapped this in #ifdef because OutputDebugString dumps the information in release mode also
		sprintf(szTempBuffer,"EuShlExt : %s, %s - %s\n", szDate, szTime, szBuffer);
		OutputDebugString(szTempBuffer);
#endif 
		
		TCHAR	szData[1024];	
		DWORD	dwDataBufSize=1024;	
		
		DWORD	dwKeyDataType;	
		HKEY	hKey = NULL;	

		DWORD	dwEnableLogging = FALSE;

		TCHAR	szLogFilePath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
		
		if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
		{
			dwKeyDataType = REG_DWORD;
			if (RegQueryValueEx(hKey, LM_REG_ENABLELOGGING_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
			{
				if (dwKeyDataType == REG_DWORD)			
					dwEnableLogging = *((DWORD*)szData);
				else
					dwEnableLogging = FALSE;
			
				// If logging enabled & a valid DLLPath is found
				if ( (dwEnableLogging) && 
					 (RegQueryValueEx(hKey, LM_REG_DLLPATH_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS) )
				{
					if (szData)
					{
						TCHAR *pChar = _tcsrchr((LPTSTR)szData,'\\');
						if (pChar)
						{
							*pChar = '\0';
							_tcscpy(szLogFilePath, szData);
							_tcscat(szLogFilePath, "\\EuShlExt.log");

							FILE* fLogFile = NULL;
							fLogFile = fopen(szLogFilePath,"a");
							if (fLogFile)
							{
								sprintf(szTempBuffer,"%s, %s - %s\n", szDate, szTime, szBuffer);
								fprintf(fLogFile, szTempBuffer);
								fclose(fLogFile);
							}						
						}
					}
				}
			}
		}	
	}
	catch(...)
	{
		// Do nothing, makes sure any misbehaviour does not crash the shell
	}
}
