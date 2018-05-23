#include "stdafx.h"

#include "ExtLaunchMgr.h"

#ifndef _EUSHL_EXT_GENERIC_USER_
	#include "utils.h"
	#include "rs.h"
	#include "qcutils.h"
	#include "resource.h"
#endif //_EUSHL_EXT_GENERIC_USER_

#include "DebugNewHelpers.h"

#ifndef _EUSHL_EXT_GENERIC_USER_
	extern CString EudoraDir;
	extern CString ExecutableDir;
#endif //_EUSHL_EXT_GENERIC_USER_

CExternalLaunchManager	gExternalLM;

// static data members being initialized
BOOL								CExternalLaunchManager::m_bRegistered = FALSE;
HINSTANCE							CExternalLaunchManager::m_hLib = NULL;
PFNREGISTERDLL						CExternalLaunchManager::m_pfnRegisterDLL = NULL;
PFNUNREGISTERDLL					CExternalLaunchManager::m_pfnUnregisterDLL = NULL;
PFNADDPATHINREGISTRY				CExternalLaunchManager::m_pfnAddPathInRegistry = NULL;
PFNREMOVEPATHFROMREGISTRY			CExternalLaunchManager::m_pfnRemovePathFromRegistry = NULL;
PFNREPLACEPATHWITHNEW				CExternalLaunchManager::m_pfnReplacePathWithNew  = NULL;
PFNUPDATEALREADYWARNEDSETTING		CExternalLaunchManager::m_pfnUpdateAlreadyWarnedSetting = NULL;
PFNUPDATEWARNEXTENSIONSSETTING		CExternalLaunchManager::m_pfnUpdateWarnExtensionsSetting = NULL;
PFNUPDATEDONOTWARNEXTENSIONSSETTING	CExternalLaunchManager::m_pfnUpdateDoNotWarnExtensionsSetting = NULL;

#ifndef _EUSHL_EXT_GENERIC_USER_

	void CExternalLaunchManager::LoadConfig()
	{
		static		bAlreadyUnregistered = FALSE;
		TCHAR		szData[1024];
		DWORD		dwDataBufSize=1024;		
		DWORD		dwKeyDataType;	
		HKEY		hKey;	
		short		nPathIndex = 0;

		static		short sCurIniSetting = -1;
		static		short sPrevIniSetting = -1;

		CString csAttachDir = GetIniString(IDS_INI_AUTO_RECEIVE_DIR);		
		if (csAttachDir.IsEmpty())
		{
			CRString csAttachFolder(IDS_ATTACH_FOLDER);
			csAttachDir = EudoraDir + csAttachFolder;
		}
		
		CRString csEmbedFolder(IDS_EMBEDDED_FOLDER);
		CString csEmbedDir = EudoraDir + csEmbedFolder;		
		
		CRString csImapFolder(IDS_IMAP_TOP_DIRECTORY_NAME);
		CString csImapDir = EudoraDir + csImapFolder;		
		
		LoadEuShlExtDll();		

		sCurIniSetting = GetIniShort(IDS_INI_WARN_EXTERNAL_LAUNCH_PROGRAM);

		if (sCurIniSetting)
		{
			DWORD	dwResult = 0;

			if (sCurIniSetting != sPrevIniSetting)
			{		
				m_bRegistered = TRUE;

				RegCreateKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwResult);	
				
				if (RegQueryValueEx(hKey, LM_REG_PATHCOUNT_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) != ERROR_SUCCESS)
				{
					DWORD dwPathCount = 0;
					RegSetValueEx(hKey, LM_REG_PATHCOUNT_KEY, 0, REG_DWORD,(const PBYTE) &dwPathCount, sizeof( DWORD ));
				}
				
				RegCloseKey(hKey);

				CRString csDefaultExtString(IDS_WARN_LAUNCH_DEFAULTS);
				if (csDefaultExtString.IsEmpty() || csDefaultExtString[csDefaultExtString.GetLength() - 1] != '|')
					csDefaultExtString += "|"; 

				CString csLaunchExtString(::GetIniString(IDS_INI_WARN_PROGRAM_EXTENSIONS));
				if (csLaunchExtString.IsEmpty() || csLaunchExtString[csLaunchExtString.GetLength() - 1] != '|')
					csLaunchExtString += "|"; 

				CString csWarnExtensions = csDefaultExtString + csLaunchExtString;

				CString csRemoveString(::GetIniString(IDS_INI_WARN_LAUNCH_REMOVE));
				if (csRemoveString.IsEmpty() || csRemoveString[csRemoveString.GetLength() - 1] != '|')
					csRemoveString += "|"; 				
				
				nPathIndex = AddPathInRegistry(csAttachDir);
				UpdateWarnExtensionsSetting((LPCTSTR)csAttachDir, (LPCTSTR)csWarnExtensions);
				UpdateDoNotWarnExtensionsSetting((LPCTSTR)csAttachDir, (LPCTSTR)csRemoveString);

				nPathIndex = AddPathInRegistry(csEmbedDir);
				UpdateWarnExtensionsSetting((LPCTSTR)csEmbedDir, (LPCTSTR)csWarnExtensions);
				UpdateDoNotWarnExtensionsSetting((LPCTSTR)csEmbedDir, (LPCTSTR)csRemoveString);
				
				nPathIndex = AddPathInRegistry(csImapDir);
				UpdateWarnExtensionsSetting((LPCTSTR)csImapDir, (LPCTSTR)csWarnExtensions);
				UpdateDoNotWarnExtensionsSetting((LPCTSTR)csImapDir, (LPCTSTR)csRemoveString);
				
				// Add any other paths (default paths) here
				// ...	
				

				sPrevIniSetting = sCurIniSetting;
			}
		}
		else
		{
			if (sCurIniSetting != sPrevIniSetting)
			{
				// Assume that we are registered
				m_bRegistered = TRUE;

				ReplacePathWithNew((LPCTSTR)csAttachDir,_T(""));			
				ReplacePathWithNew((LPCTSTR)csEmbedDir,_T(""));
				ReplacePathWithNew((LPCTSTR)csImapDir,_T(""));

				UnregisterDLL();

				// Now we are not registered
				m_bRegistered = FALSE;

				sPrevIniSetting = sCurIniSetting;
			}
		}	
	}

	void CExternalLaunchManager::Initialize()
	{
		// Initialize data here ...
		try
		{
			LoadConfig();
		}
		catch (CMemoryException * /* pMemoryException */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing CMemoryException in CExternalLaunchManager::Initialize" );
			throw;
		}
		catch (CException * pException)
		{
			// Other MFC exception
			pException->Delete();
			ASSERT( !"Caught CException (not CMemoryException) in CExternalLaunchManager::Initialize" );
		}

		return;
	}

	void CExternalLaunchManager::RegisterDLL()
	{
		TCHAR	szDllPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];

		_tcscpy(szDllPath,ExecutableDir);
		_tcscat(szDllPath,gszShellExtensionDLLName);

		if (!FileExistsMT(szDllPath))
		{
			_tcscpy(szDllPath,EudoraDir);
			_tcscat(szDllPath,gszShellExtensionDLLName);
		}

		if (FileExistsMT(szDllPath))
		{	
			// Load the EuShlExt.dll to call RegisterDLL function.
			PFNREGISTERDLL	pfnRegisterDLL;
			HINSTANCE hLib = LoadLibrary(szDllPath);		
			try
			{
				if (hLib != NULL)
				{		
					// Find the entry point.
					pfnRegisterDLL = (PFNREGISTERDLL)GetProcAddress(hLib, szFuncName_RegisterDLL);
					if (pfnRegisterDLL)
						pfnRegisterDLL(szDllPath,false);

					FreeLibrary(hLib);
				}
			}
			catch (CMemoryException * /* pMemoryException */)
			{
				// Catastrophic memory exception - rethrow
				ASSERT( !"Rethrowing CMemoryException in CExternalLaunchManager::RegisterDLL" );
				throw;
			}
			catch (CException * pException)
			{
				// Other MFC exception
				pException->Delete();
				ASSERT( !"Caught CException (not CMemoryException) in CExternalLaunchManager::RegisterDLL" );
			}
		}
	}

	void CExternalLaunchManager::UnregisterDLL()
	{
		TCHAR		szData[1024];	
		DWORD		dwDataBufSize=1024;	
		
		DWORD		dwKeyDataType;	
		HKEY		hKey = NULL;	

		// Unregister the EuShlExt.dll		
		if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
		{
			if (RegQueryValueEx(hKey, LM_REG_DLLPATH_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
			{
				if (m_pfnUnregisterDLL)
					m_pfnUnregisterDLL();
			}
			RegCloseKey(hKey);
		}
	}
#endif //_EUSHL_EXT_GENERIC_USER_

void CExternalLaunchManager::LoadEuShlExtDll()
{
	TCHAR		szData[1024];	
	DWORD		dwDataBufSize=1024;	
	
	DWORD		dwKeyDataType;	
	HKEY		hKey = NULL;	

	#ifndef _EUSHL_EXT_GENERIC_USER_
		RegisterDLL();
	#endif

	// Load the correct EuShlExt.dll		
	if ( RegOpenKeyEx(HKEY_CURRENT_USER, LM_REG_APP_KEY,0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, LM_REG_DLLPATH_KEY, NULL, &dwKeyDataType,(LPBYTE) szData, &(dwDataBufSize = sizeof(szData))) == ERROR_SUCCESS)
		{
			m_hLib = LoadLibrary((LPTSTR)szData);

			//try - compiler said catch code would never be used, so commented out
			//{
				if (m_hLib != NULL)
				{		
					// Find the entry point.
					m_pfnRegisterDLL = (PFNREGISTERDLL)GetProcAddress(m_hLib, szFuncName_RegisterDLL);
					m_pfnUnregisterDLL = (PFNUNREGISTERDLL)GetProcAddress(m_hLib, szFuncName_UnregisterDLL);
					m_pfnAddPathInRegistry = (PFNADDPATHINREGISTRY)GetProcAddress(m_hLib, szFuncName_AddPathInRegistry);
					m_pfnRemovePathFromRegistry = (PFNREMOVEPATHFROMREGISTRY)GetProcAddress(m_hLib, szFuncName_RemovePathFromRegistry);
					m_pfnReplacePathWithNew = (PFNREPLACEPATHWITHNEW)GetProcAddress(m_hLib, szFuncName_ReplacePathWithNew);
					m_pfnUpdateAlreadyWarnedSetting = (PFNUPDATEALREADYWARNEDSETTING)GetProcAddress(m_hLib, szFuncName_UpdateAlreadyWarnedSetting);
					m_pfnUpdateWarnExtensionsSetting = (PFNUPDATEWARNEXTENSIONSSETTING)GetProcAddress(m_hLib, szFuncName_UpdateWarnExtensionsSetting);
					m_pfnUpdateDoNotWarnExtensionsSetting = (PFNUPDATEDONOTWARNEXTENSIONSSETTING)GetProcAddress(m_hLib, szFuncName_UpdateDoNotWarnExtensionsSetting);
				}
				else
				{
					TCHAR szBuf[128];
					sprintf(szBuf,"ExternalLaunchManager : EuShlExt LoadLibrary failed with error - %d\n",GetLastError());
					OutputDebugString(szBuf);
				}
			//}
			//catch(...)
			//{
			//	ASSERT(0);
			//	OutputDebugString("ExternalLaunchManager : Exception caught while Loading EuShlExt.dll\n");
			//}
			
		}
		RegCloseKey(hKey);
	}

}

void CExternalLaunchManager::UnLoadEuShlExtDll()
{
	if (m_hLib != NULL)
	{		
		FreeLibrary(m_hLib);

		m_hLib = NULL;
		m_pfnRegisterDLL = NULL;
		m_pfnUnregisterDLL = NULL;
		m_pfnAddPathInRegistry = NULL;
		m_pfnRemovePathFromRegistry = NULL;
		m_pfnReplacePathWithNew  = NULL;
		m_pfnUpdateAlreadyWarnedSetting = NULL;
		m_pfnUpdateWarnExtensionsSetting = NULL;
		m_pfnUpdateDoNotWarnExtensionsSetting = NULL;
	}	
}


void CExternalLaunchManager::UpdateAlreadyWarnedSetting(LPCTSTR lpcszFileName,BOOL bValue)
{
// Force everything to warning level 3.
#pragma warning ( push , 3 ) 

// 'int' : forcing value to bool 'true' or 'false' (performance warning) - The conversion here is done on purpose
#pragma warning ( disable : 4800 ) 

	if (m_pfnUpdateAlreadyWarnedSetting)
		m_pfnUpdateAlreadyWarnedSetting(lpcszFileName, bValue);

// Get the warning level back to the original one.
#pragma warning ( pop ) 

}

BOOL CExternalLaunchManager::UpdateWarnExtensionsSetting(LPCTSTR lpcszPath, LPCTSTR lpcszWarnExtensions)
{
	if (m_pfnUpdateWarnExtensionsSetting)
		return m_pfnUpdateWarnExtensionsSetting(lpcszPath,lpcszWarnExtensions);

	return FALSE;
}

BOOL CExternalLaunchManager::UpdateDoNotWarnExtensionsSetting(LPCTSTR lpcszPath, LPCTSTR lpcszDoNotWarnExtensions)
{
	if (m_pfnUpdateDoNotWarnExtensionsSetting)
		return m_pfnUpdateDoNotWarnExtensionsSetting(lpcszPath,lpcszDoNotWarnExtensions);

	return FALSE;
}

short CExternalLaunchManager::AddPathInRegistry(CString csPath)
{
	DWORD		dwPathIndex = 0;

	if (m_pfnAddPathInRegistry)
		dwPathIndex = m_pfnAddPathInRegistry((LPCTSTR)csPath);
	
	return (short)dwPathIndex;
}

void CExternalLaunchManager::RemovePathFromRegistry(CString csPath)
{
	if (m_pfnRemovePathFromRegistry)
		m_pfnRemovePathFromRegistry((LPCTSTR) csPath);
}	

BOOL CExternalLaunchManager::ReplacePathWithNew(LPCTSTR lpszOldPath, LPTSTR lpszNewPath)
{
	if (m_pfnReplacePathWithNew)
		return m_pfnReplacePathWithNew(lpszOldPath, lpszNewPath);
	return FALSE;
}

