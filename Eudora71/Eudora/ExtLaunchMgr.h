#ifndef _EXTERNAL_LAUNCH_MANAGER_
#define _EXTERNAL_LAUNCH_MANAGER_

// Uncomment the next 3 lines for ESP

/*
#ifndef _EUSHL_EXT_GENERIC_USER_
#define _EUSHL_EXT_GENERIC_USER_
#endif //_EUSHL_EXT_GENERIC_USER_
*/

#ifndef _EUSHL_EXT_GENERIC_USER_
	#include "..\EuShlExt\RegDefines.h"
#else
	#include "RegDefines.h"
#endif

// Function Pointers fo EuShlExt.dll
const TCHAR szFuncName_RegisterDLL[] = _T("RegisterDLL");
typedef bool (* PFNREGISTERDLL)(LPCTSTR lpszDLLPath, bool bForce);

const TCHAR szFuncName_UnregisterDLL[] = _T("UnregisterDLL");
typedef void (* PFNUNREGISTERDLL)();	

const TCHAR szFuncName_AddPathInRegistry[] = _T("AddPathInRegistry");
typedef short (* PFNADDPATHINREGISTRY)(LPCTSTR lpcszPath);

const TCHAR szFuncName_RemovePathFromRegistry[] = _T("RemovePathFromRegistry");
typedef void (* PFNREMOVEPATHFROMREGISTRY)(LPCTSTR lpcszOldPath);

const TCHAR szFuncName_ReplacePathWithNew[] = _T("ReplacePathWithNew");
typedef bool (* PFNREPLACEPATHWITHNEW)(LPCTSTR lpcszOldPath, LPCTSTR lpcszNewPath);

const TCHAR szFuncName_UpdateAlreadyWarnedSetting[] = _T("UpdateAlreadyWarnedSetting");
typedef void (* PFNUPDATEALREADYWARNEDSETTING)(LPCTSTR lpszFileName,bool bValue);

const TCHAR szFuncName_UpdateWarnExtensionsSetting[] = _T("UpdateWarnExtensionsSetting");
typedef bool (* PFNUPDATEWARNEXTENSIONSSETTING)(LPCTSTR lpcszPathName, LPCTSTR lpcszWarnExtensions);

const TCHAR szFuncName_UpdateDoNotWarnExtensionsSetting[] = _T("UpdateDoNotWarnExtensionsSetting");
typedef bool (* PFNUPDATEDONOTWARNEXTENSIONSSETTING)(LPCTSTR lpcszPathName, LPCTSTR lpcszDoNotWarnExtensions);


// Most of the members are static 'coz the main intent of this class is dealing with reading/updating data in the Registry 
// under HKCU\LM_REG_APP_KEY
class CExternalLaunchManager
{
public:
	static PFNREGISTERDLL						m_pfnRegisterDLL;
	static PFNUNREGISTERDLL						m_pfnUnregisterDLL;
	static PFNADDPATHINREGISTRY					m_pfnAddPathInRegistry;
	static PFNREMOVEPATHFROMREGISTRY			m_pfnRemovePathFromRegistry;
	static PFNREPLACEPATHWITHNEW				m_pfnReplacePathWithNew;
	static PFNUPDATEALREADYWARNEDSETTING		m_pfnUpdateAlreadyWarnedSetting;
	static PFNUPDATEWARNEXTENSIONSSETTING		m_pfnUpdateWarnExtensionsSetting;
	static PFNUPDATEDONOTWARNEXTENSIONSSETTING	m_pfnUpdateDoNotWarnExtensionsSetting;
	
private:
	static	BOOL	m_bRegistered;

	static	HINSTANCE	m_hLib;


#ifndef	_EUSHL_EXT_GENERIC_USER_
	// Registers the EuShlExt.dll 
	static void RegisterDLL();

	// Unregisters the EuShlExt.dll
	static void UnregisterDLL();

	// Load the configuration from the registry
	static void	LoadConfig();
#endif //_EUSHL_EXT_GENERIC_USER_

public:

	~CExternalLaunchManager()
		{ 
#ifndef _EUSHL_EXT_GENERIC_USER_
			UnLoadEuShlExtDll(); 
#endif //_EUSHL_EXT_GENERIC_USER_
		}
	
#ifndef _EUSHL_EXT_GENERIC_USER_
	// Initialize External LaunchManager
	static void	Initialize(void);

	static void RecacheDataFromRegistry()
	{
		try
		{ 
			LoadConfig(); 
		}
		catch (CMemoryException * /* pMemoryException */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing CMemoryException in CExternalLaunchManager::RecacheDataFromRegistry" );
			throw;
		}
		catch (CException * pException)
		{
			// Other MFC exception
			pException->Delete();
			ASSERT( !"Caught CException (not CMemoryException) in CExternalLaunchManager::RecacheDataFromRegistry" );
		}
	};
#endif // _EUSHL_EXT_GENERIC_USER_

	static void LoadEuShlExtDll();
	static void UnLoadEuShlExtDll();

	// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
	static short AddPathInRegistry(CString csPath);	
	
	// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
	static void RemovePathFromRegistry(CString csPath);
	
	// Replaces the registry entry containing lpszOldPath with lpszNewPath
	static BOOL ReplacePathWithNew(LPCTSTR lpszOldPath, LPTSTR lpszNewPath);

	static void UpdateAlreadyWarnedSetting(LPCTSTR lpszFileName,BOOL bValue);
	static BOOL UpdateWarnExtensionsSetting(LPCTSTR lpcszPath, LPCTSTR lpcszWarnExtensions);
	static BOOL UpdateDoNotWarnExtensionsSetting(LPCTSTR lpcszPath, LPCTSTR lpcszDoNotWarnExtensions);
};

extern CExternalLaunchManager	gExternalLM;

#endif	// _EXTERNAL_LAUNCH_MANAGER_


