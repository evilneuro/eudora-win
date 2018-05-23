#ifndef _EUSHL_REGISTRY_ENTRY_MANAGER_
#define _EUSHL_REGISTRY_ENTRY_MANAGER_

#include "EuShlExt.h"

// Enum for the Settings in registry for every path.
enum EuShlRegSettings { _EUSHL_ALREADY_WARNED = 0,
						_EUSHL_WARN_EXTENSION,
						_EUSHL_DO_NOT_WARN_EXTENSION};	

// Most of the members are static 'coz the main intent of this class is dealing with reading/updating data in the Registry 
// under HKCU\LM_REG_APP_KEY
class CEuShlRegEntryMgr
{
private:
	
	// This function removes any redundant LM_REG_PATH_KEY entries (A Path#y entry of a directory within a directory, which also has a Path#x
	// entry is considered redundant)
	static void	CompactRegistryEntries();
	
	// Load the configuration from the registry
	static void	LoadConfig();

public:
	// Registers the EuShlExt.dll 
	static void RegisterDLL(TCHAR* lpszDLLPath, BOOL bForce = false);

	// Unregisters the EuShlExt.dll
	static void UnregisterDLL();	
	
	// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
	static short AddPathInRegistry(LPCTSTR lpcszPath);
	
	// Add the given path in the registry under HKCU\LM_REG_APP_KEY\LM_REG_PATH_KEY
	static void RemovePathFromRegistry(LPCTSTR lpcszPath);
	
	// Replaces the registry entry containing lpcszOldPath with lpszNewPath
	static bool ReplacePathWithNew(LPCTSTR lpcszOldPath, LPCTSTR lpszNewPath);

	static void UpdateAlreadyWarnedSetting(LPCTSTR lpcszFileName,bool bValue);

	// Update appropriate setting in the registry under HKEY_CUURENT_USER\LM_REG_APP_KEY
	static bool	UpdateSetting(short nPathIndex, EuShlRegSettings Setting, DWORD dwValue);
	static bool	UpdateSetting(short nPathIndex, EuShlRegSettings Setting, LPCTSTR);	

	// Gets the path(LM_REG_PATH_KEY) relative position/index from the reg under HKCU\LM_REG_APP_KEY
	static short GetPathIndex(LPCTSTR lpcszFileName);
};

extern CEuShlRegEntryMgr	gEuShlRegEntryMgr;

#endif //_EUSHL_REGISTRY_ENTRY_MANAGER_
