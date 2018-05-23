
#ifndef _EUSHLEXT_H_
#define _EUSHLEXT_H_

/*********************************************************************************************
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
**********************************************************************************************/

#ifdef EUSHLEXT_EXPORTS
#  ifdef __cplusplus
#    define EUSHLEXT_API extern "C" __declspec(dllexport)
#  else
#    define EUSHLEXT_API __declspec(dllexport)
#  endif
#else
#define EUSHLEXT_API __declspec(dllimport)
#endif

	// Registers the EuShlExt.dll 
	EUSHLEXT_API bool RegisterDLL(LPCTSTR lpszDLLPath, bool bForce);
	
	// Unregisters the EuShlExt.dll
	EUSHLEXT_API void UnregisterDLL();
	
	// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
	EUSHLEXT_API short AddPathInRegistry(LPCTSTR lpcszPath);
	
	// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
	EUSHLEXT_API void RemovePathFromRegistry(LPCTSTR lpcszPath);
	
	// Replaces the registry entry containing lpcszOldPath with lpszNewPath
	EUSHLEXT_API bool ReplacePathWithNew(LPCTSTR lpcszOldPath, LPCTSTR lpcszNewPath);
	
	// Update appropriate setting in the registry under HKEY_CUURENT_USER\LM_REG_APP_KEY
	EUSHLEXT_API bool UpdateDoNotWarnExtensionsSetting(LPCTSTR lpcszPathName, LPCTSTR lpcszDoNotWarnExtensions);
	
	EUSHLEXT_API bool UpdateWarnExtensionsSetting(LPCTSTR lpcszPathName, LPCTSTR lpcszWarnExtensions);
	
	EUSHLEXT_API void UpdateAlreadyWarnedSetting(LPCTSTR lpcszFileName,bool bValue);

#endif // _EUSHLEXT_H_
