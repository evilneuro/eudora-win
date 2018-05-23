// EuShlExt.cpp : Defines the entry point for the DLL application.
//

#include "windows.h"

#include <objbase.h>
#include <initguid.h>
#include <olectl.h>
#include <tchar.h>
#include "EuShlExt.h"
#include "EuShlRegEntryMgr.h"
#include "ShellHookImpl.h"

// Handle to "this" module.
HANDLE	g_hModule = NULL;

// Stores the TLS (Thread Local Storage) associated value
DWORD TlsIndex = 0;

// This variable is used to keep track of  whther I (this DLL) should unload or not.
bool g_bCanUnloadNow = FALSE;

// Variable to keep count of reference
unsigned int  g_cRefThisDll = 0;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	THREADINFO *pti = NULL;

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hModule = hModule;
			g_bCanUnloadNow = FALSE;
			TlsIndex = TlsAlloc();			
			//fall thru'
		case DLL_THREAD_ATTACH:
			pti = (THREADINFO *)LocalAlloc(LPTR, sizeof(THREADINFO));
			if (!pti) 
			{
				return(FALSE);
			}
			TlsSetValue(TlsIndex, pti);
			break;

		case DLL_THREAD_DETACH: 
			pti = (THREADINFO *)TlsGetValue(TlsIndex);
			if (pti)
			{
				LocalFree((HLOCAL) pti);
				TlsSetValue(TlsIndex, NULL);
			}
			break;

		case DLL_PROCESS_DETACH:			
			g_bCanUnloadNow = TRUE;
			TlsFree(TlsIndex);
			break;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Special entry points required for inproc servers

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;

    if (IsEqualIID(rclsid, CLSID_EudoraShellExtension))
    {
		// The Shell Extension Class Factory Storage Variable
		CShellExtClassFactory *gpcf = NULL;

		try 
		{
			gpcf = new CShellExtClassFactory;
		} catch (...) 
		{				
			// Write this to the log
			gpcf=NULL;
		}
		
		// return the Interface so queried.
        if (gpcf)
			return gpcf->QueryInterface(riid, ppv);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void)
{
	// Unload the DLL if no more referenced
	DumpLogInformation("In DLLCanUnloadNow Ref Count : %d", g_cRefThisDll);
	return (g_cRefThisDll == 0 ? S_OK : S_FALSE);	
}

// Helper Function to do the conversions depending on platform.
int WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
	*pLocal = 0;

#ifdef UNICODE
	lstrcpyn(pLocal, pWide, dwChars);
#else
	WideCharToMultiByte( CP_ACP, 
						 0, 
						 pWide, 
						 -1, 
						 pLocal, 
						 dwChars, 
						NULL, 
						NULL);
#endif

	return lstrlen(pLocal);
}

// By exporting DllRegisterServer, regsvr32.exe/regsvr.exe can be used to register
STDAPI DllRegisterServer(void)
{	
	int      i = 0;
	HKEY     hKey = NULL;
	LRESULT  lResult = 0;
	DWORD    dwDisp = 0;
	TCHAR    szSubKey[MAX_PATH] = {0};
	TCHAR    szCLSID[MAX_PATH] = {0};
	TCHAR    szModule[MAX_PATH] = {0};
	LPWSTR   pwszShellExt = {0};

	//Get the CLSID in string form
	StringFromIID(CLSID_EudoraShellExtension, &pwszShellExt);			    

	if (pwszShellExt) {
		WideCharToLocal(szCLSID, pwszShellExt, sizeof(szCLSID) / sizeof(TCHAR));

		//Free the string
		LPMALLOC pMalloc;
		CoGetMalloc(1, &pMalloc);
		if(pMalloc)
		{
			pMalloc->Free(pwszShellExt);
			pMalloc->Release();
		}
	}															   

	//Get this DLL's path and file name																    
	GetModuleFileName((HINSTANCE) g_hModule, szModule, sizeof(szModule) / sizeof(TCHAR));

	// The following lines of code does the dirty work of registering with the system

	// These entries have their %s's replaced with CLSID strings,
	// their second entries are replaced with the module's path.
	// Register the CLSID entries
	REGSTRUCT ShExClsidEntries[] = 
	{
		// This is the 	HKEY_CLASSES_ROOT/CLSID/<our CLSID> entry. where we place info about our shell ext.
		HKEY_CLASSES_ROOT,	LM_REG_REGISTRATION_CID_KEY,					NULL,                   TEXT(SHELLEXNAME),

		// HKEY_CLASSES_ROOT/CLSID/<our CLSID>/InProcServer . here, we tell the system where our DLL is located
		HKEY_CLASSES_ROOT,	LM_REG_REGISTRATION_CID_I_KEY,					NULL,                   TEXT("%s"),

		// HKEY_CLASSES_ROOT/CLSID/<our CLSID>/InProcServer/ThreadingModel = Apartment . this is a standard thing. 
		HKEY_CLASSES_ROOT,	LM_REG_REGISTRATION_CID_I_KEY,					TEXT("ThreadingModel"), TEXT("Apartment"),

		// All NULL entries signify end the array
		NULL,                NULL,								NULL,                   NULL
	};

	for(i = 0; ShExClsidEntries[i].hRootKey; i++) 
	{
		//Create the sub key string.
		wsprintf(szSubKey, ShExClsidEntries[i].lpszSubKey, szCLSID);

		lResult = RegCreateKeyEx( ShExClsidEntries[i].hRootKey,
								  szSubKey,
								  0,
								  NULL,
								  REG_OPTION_NON_VOLATILE,
								  KEY_WRITE,
								  NULL,
								  &hKey,
								  &dwDisp);

		if(NOERROR == lResult) 
		{
			TCHAR szData[MAX_PATH];
																						 
			//If necessary, create the value string
			wsprintf(szData, ShExClsidEntries[i].lpszData, szModule);

			lResult = RegSetValueEx( hKey,
									 ShExClsidEntries[i].lpszValueName,
									 0,
									 REG_SZ,
									 (LPBYTE)szData,
									 lstrlen(szData) + 1);

			RegCloseKey(hKey);

		} else
			return SELFREG_E_CLASS;
	}                                

	lstrcpy( szSubKey, LM_REG_REGISTRATION_E_SE_KEY);

	lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
							  szSubKey,
							  0,
							  NULL,
							  REG_OPTION_NON_VOLATILE,
							  KEY_WRITE,
							  NULL,
							  &hKey,
							  &dwDisp);

	if(NOERROR == lResult) 
	{
		TCHAR szData[_MAX_PATH];

		//Create the value string.		
		lstrcpy(szData, TEXT(SHELLEXNAME));

		lResult = RegSetValueEx(hKey,
								szCLSID,
								0,
								REG_SZ,
								(LPBYTE)szData,
								lstrlen(szData) + 1);

			RegCloseKey(hKey);
	} 
	else
		return SELFREG_E_CLASS;
	

	//If running on NT, register the extension as approved.
	OSVERSIONINFO  osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	// NT needs to have shell extensions "approved". For Win 2000, it does not matter, but we still add it.
	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)  
	{
		//lstrcpy( szSubKey, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));
		lstrcpy( szSubKey, LM_REG_REGISTRATION_SE_A_KEY);

		lResult = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
							  szSubKey,
							  0,
							  NULL,
							  REG_OPTION_NON_VOLATILE,
							  KEY_WRITE,
							  NULL,
							  &hKey,
							  &dwDisp);

		if(NOERROR == lResult) 
		{
			TCHAR szData[MAX_PATH];

			//Create the value string.
			lstrcpy(szData, TEXT(SHELLEXNAME));

			lResult = RegSetValueEx(   hKey,
									 szCLSID,
									 0,
									 REG_SZ,
									 (LPBYTE)szData,
									 lstrlen(szData) + 1);

			RegCloseKey(hKey);
		} else
			return SELFREG_E_CLASS;
	}

	return S_OK;
}

// By exporting DllUnregisterServer, regsvr32.exe/regsvr.exe can be used to unregister
STDAPI DllUnregisterServer(void)
{
	int      i = 0;
	LRESULT  lResult = 0;
	TCHAR    szSubKey[MAX_PATH] = {0};
	TCHAR    szCLSID[MAX_PATH] = {0};
	LPWSTR   pwszShellExt = {0};

	// Get the CLSID in string form
	StringFromIID(CLSID_EudoraShellExtension, &pwszShellExt);			    

	if (pwszShellExt) 
	{
		// Do the appropriate conversion
		WideCharToLocal(szCLSID, pwszShellExt, sizeof(szCLSID)/sizeof (TCHAR));

		// Free the string
		LPMALLOC pMalloc;						  
		CoGetMalloc(1, &pMalloc);
		if(pMalloc)
		{
			pMalloc->Free(pwszShellExt);
			pMalloc->Release();
		}
	}															   
					    
	// These entries have their %s's replaced with CLSID strings,
	// their second entries are replaced with the module's path.
	// Register the CLSID entries
	REGSTRUCT ShExClsidEntries[] = 
	{  
		// HKEY_CLASSES_ROOT/CLSID/<our CLSID>/InProcServer . here, we tell the system where our DLL is located
		HKEY_CLASSES_ROOT,	LM_REG_REGISTRATION_CID_I_KEY,	NULL,                   NULL,

		// This is the 	HKEY_CLASSES_ROOT/CLSID/<our CLSID> entry. this is where we place info about our shell ext.		
		HKEY_CLASSES_ROOT,	LM_REG_REGISTRATION_CID_KEY,					NULL,                   NULL,		

		// All NULL entries signify end the array
		NULL,                NULL,								NULL,                   NULL
	};

	for(i = 0; ShExClsidEntries[i].hRootKey; i++) 
	{
		//Create the sub key string.
		wsprintf(szSubKey, ShExClsidEntries[i].lpszSubKey, szCLSID);

		lResult = RegDeleteKey(  ShExClsidEntries[i].hRootKey, szSubKey);
																			 
		if(NOERROR != lResult) 
			return SELFREG_E_CLASS;
	}

	HKEY	hSubKey = NULL;
	
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,LM_REG_REGISTRATION_E_SE_KEY,0,KEY_ALL_ACCESS,&hSubKey);	
	lResult = RegDeleteValue( hSubKey,szCLSID);
	
	RegCloseKey(hSubKey);

	//If running on NT, register the extension as approved.
	OSVERSIONINFO  osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	// NT needs to have shell extensions "approved".
	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)  
	{
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,LM_REG_REGISTRATION_SE_A_KEY,0,KEY_ALL_ACCESS,&hSubKey);	
		lResult = RegDeleteValue( hSubKey, szCLSID);
		RegCloseKey(hSubKey);
	}	

	// This is the place which is used by application (Eudora) to figure out from where the DLL is loaded.
	// Lets remove this even if someone uninstalls the DLL from command line (using regsvr32)
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hSubKey);
	lResult = RegDeleteValue(hSubKey,"DllPath");
	
	RegCloseKey(hSubKey);

	return S_OK;
}

// Registers the EuShlExt.dll 
bool RegisterDLL(LPCTSTR lpcszDLLPath, bool bForce)
{
	try
	{
		gEuShlRegEntryMgr.RegisterDLL((TCHAR *)lpcszDLLPath, bForce);
	}
	catch(...)
	{
		DumpLogInformation("Exception caused in RegisterDLL");
		return false;
	}

	return true;
}

// Unregisters the EuShlExt.dll
void UnregisterDLL()
{
	gEuShlRegEntryMgr.UnregisterDLL();
}

// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
short AddPathInRegistry(LPCTSTR lpcszPath)
{
	return gEuShlRegEntryMgr.AddPathInRegistry(lpcszPath);
}

// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
void RemovePathFromRegistry(LPCTSTR lpcszPath)
{
	gEuShlRegEntryMgr.RemovePathFromRegistry(lpcszPath);
}

// Replaces the registry entry containing lpcszOldPath with lpszNewPath
bool ReplacePathWithNew(LPCTSTR lpcszOldPath, LPCTSTR lpcszNewPath)
{
	return gEuShlRegEntryMgr.ReplacePathWithNew(lpcszOldPath, lpcszNewPath);
}

// The next three Functions are to update appropriate setting in the registry under HKEY_CUURENT_USER\LM_REG_APP_KEY
bool UpdateWarnExtensionsSetting(LPCTSTR lpcszPathName, LPCTSTR lpcszWarnExtensions)
{
	short nPathIndex = gEuShlRegEntryMgr.GetPathIndex(lpcszPathName);
	return gEuShlRegEntryMgr.UpdateSetting(nPathIndex, _EUSHL_WARN_EXTENSION, lpcszWarnExtensions);
}

bool UpdateDoNotWarnExtensionsSetting(LPCTSTR lpcszPathName, LPCTSTR lpcszDoNotWarnExtensions)
{
	short nPathIndex = gEuShlRegEntryMgr.GetPathIndex(lpcszPathName);
	return gEuShlRegEntryMgr.UpdateSetting(nPathIndex, _EUSHL_DO_NOT_WARN_EXTENSION, lpcszDoNotWarnExtensions);
}

void UpdateAlreadyWarnedSetting(LPCTSTR lpszFileName,bool bValue)
{
	short nPathIndex = gEuShlRegEntryMgr.GetPathIndex(lpszFileName);
	gEuShlRegEntryMgr.UpdateSetting(nPathIndex, _EUSHL_ALREADY_WARNED ,bValue);
}
