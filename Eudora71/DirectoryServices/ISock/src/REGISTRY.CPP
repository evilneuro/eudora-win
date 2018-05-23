//
// Registry.cpp
//

#pragma warning(disable : 4201 4514)
#include <objbase.h>
#pragma warning(default : 4201)
#include <assert.h>
#include "DebugNewHelpers.h"
#include "Registry.h"


////////////////////////////////////////////////////////
//
// Internal helper functions prototypes
//

// Set the given key and its value.
BOOL setKeyAndValue(const char* pszPath,
                    const char* szSubkey,
                    const char* szValue) ;

// Convert a CLSID into a char string.
void CLSIDtochar(const CLSID& clsid,
                 char* szCLSID,
                 int length) ;

// Delete szKeyChild and all of its descendents.
LONG recursiveDeleteKey(HKEY hKeyParent, const char* szKeyChild) ;

////////////////////////////////////////////////////////
//
// Constants
//

// Size of a CLSID as a string
const int CLSID_STRING_SIZE = 39 ;

/////////////////////////////////////////////////////////
//
// Public function implementation
//

// Ignore "local variable is initialized but not referenced" warning because it
// only applies to release builds. Debug builds use the variable(s) to assert.
#pragma warning( disable : 4189 )

//
// Register the component in the registry.
//
HRESULT RegisterServer(HMODULE hModule,            // DLL module handle
                       const CLSID& clsid,         // Class ID
                       const char* szFriendlyName, // Friendly Name
                       const char* szVerIndProgID, // Programmatic
                       const char* szProgID,       //   IDs
											 const char* szExtraInfo)    // additional information
{
	// Get server location.
	char szModule[512] ;
	DWORD dwResult =
		::GetModuleFileName(hModule,
		                    szModule,
		                    sizeof(szModule)/sizeof(char)) ;
	assert(dwResult != 0) ;

	// Convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	// Build the key CLSID\\{...}
	char szKey[64] ;
	strcpy(szKey, "CLSID\\") ;
	strcat(szKey, szCLSID) ;

	// Add the CLSID to the registry.
	setKeyAndValue(szKey, NULL, szFriendlyName) ;

	// Add the server filename subkey under the CLSID key.
	setKeyAndValue(szKey, "InprocServer32", szModule) ;

	// Add the ProgID subkey under the CLSID key.
	if (szProgID != NULL)
		setKeyAndValue(szKey, "ProgID", szProgID);

	if (szVerIndProgID) {
		// Add the version-independent ProgID subkey under CLSID key.
		setKeyAndValue(szKey, "VersionIndependentProgID", szVerIndProgID);

		// Add the version-independent ProgID subkey under HKEY_CLASSES_ROOT.
		setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ;
		setKeyAndValue(szVerIndProgID, "CLSID", szCLSID) ;
		setKeyAndValue(szVerIndProgID, "CurVer", szProgID) ;
	}

	if (szProgID != NULL) {
		// Add the versioned ProgID subkey under HKEY_CLASSES_ROOT.
		setKeyAndValue(szProgID, NULL, szFriendlyName) ;
		setKeyAndValue(szProgID, "CLSID", szCLSID) ;
	}

	// Add CLSID under extra information key if available
	if (szExtraInfo != NULL)
		setKeyAndValue(szExtraInfo, szCLSID, szFriendlyName);

	return S_OK ;
}

// Ignore "local variable is initialized but not referenced" warning because it
// only applies to release builds. Debug builds use the variable(s) to assert.
#pragma warning( default : 4189 )


//
// Remove the component from the registry.
//
LONG UnregisterServer(const CLSID& clsid,         // Class ID
                      const char* szVerIndProgID, // Programmatic
                      const char* szProgID,       //   IDs
											const char* szExtraInfo)    // additional information
{
	// Convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	// Build the key CLSID\\{...}
	char szKey[64] ;
	strcpy(szKey, "CLSID\\") ;
	strcat(szKey, szCLSID) ;

	// Delete the CLSID Key - CLSID\{...}
	LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ; // Subkey may not exist.

	// Delete the version-independent ProgID Key.
	if (szVerIndProgID) {
		lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID) ;
		assert((lResult == ERROR_SUCCESS) ||
					 (lResult == ERROR_FILE_NOT_FOUND)) ; // Subkey may not exist.
	}

	// Delete the ProgID key.
	if (szProgID != NULL) {
		lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID) ;
		assert((lResult == ERROR_SUCCESS) ||
					 (lResult == ERROR_FILE_NOT_FOUND)) ; // Subkey may not exist.
	}

  // Delete the extra information key
	if (szExtraInfo != NULL) {
		HKEY hKeyChild;
		LONG lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT, szExtraInfo, 0,
												 KEY_ALL_ACCESS, &hKeyChild) ;
		if (lRes != ERROR_SUCCESS)
			return lRes;

		lResult = recursiveDeleteKey(hKeyChild, szCLSID);
		assert((lResult == ERROR_SUCCESS) ||
					 (lResult == ERROR_FILE_NOT_FOUND)) ; // Subkey may not exist.
	}

	return S_OK ;
}

// Ignore "local variable is initialized but not referenced" warning because it
// only applies to release builds. Debug builds use the variable(s) to assert.
#pragma warning( disable : 4189 )

///////////////////////////////////////////////////////////
//
// Internal helper functions
//

// Convert a CLSID to a char string.
void CLSIDtochar(const CLSID& clsid,
                 char* szCLSID,
                 int length)
{
	assert(length >= CLSID_STRING_SIZE) ;
	// Get CLSID
	LPOLESTR wszCLSID = NULL ;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;
	assert(SUCCEEDED(hr)) ;

	// Covert from wide characters to non-wide.
	wcstombs(szCLSID, wszCLSID, length) ;

	// Free memory.
	CoTaskMemFree(wszCLSID) ;
}

// Ignore "local variable is initialized but not referenced" warning because it
// only applies to release builds. Debug builds use the variable(s) to assert.
#pragma warning( default : 4189 )


HRESULT charToCLSID(const CLSID& clsid, char* szCLSID)
{
	wchar_t wchBuffer[CLSID_STRING_SIZE+2];
	mbstowcs(wchBuffer, szCLSID, CLSID_STRING_SIZE);
	return CLSIDFromString(wchBuffer, (CLSID*)&clsid);
}



//
// Delete a key and all of its descendents.
//
LONG recursiveDeleteKey(HKEY hKeyParent,           // Parent of key to delete
                        const char* lpszKeyChild)  // Key to delete
{
	// Open the child.
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
	                         KEY_ALL_ACCESS, &hKeyChild) ;
	if (lRes != ERROR_SUCCESS)
	{
		return lRes ;
	}

	// Enumerate all of the decendents of this child.
	FILETIME time ;
	char szBuffer[256] ;
	DWORD dwSize = 256 ;
	while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
	                    NULL, NULL, &time) == S_OK)
	{
		// Delete the decendents of this child.
		lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey(hKeyChild) ;
			return lRes;
		}
		dwSize = 256 ;
	}

	// Close the child.
	RegCloseKey(hKeyChild) ;

	// Delete this child.
	return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

//
// Create a key and set its value.
//   - This helper function was borrowed and modifed from
//     Kraig Brockschmidt's book Inside OLE.
//
BOOL setKeyAndValue(const char* szKey,
                    const char* szSubkey,
                    const char* szValue)
{
	HKEY hKey;
	char szKeyBuf[1024] ;

	// Copy keyname into buffer.
	strcpy(szKeyBuf, szKey) ;

	// Add subkey name to buffer.
	if (szSubkey != NULL)
	{
		strcat(szKeyBuf, "\\") ;
		strcat(szKeyBuf, szSubkey ) ;
	}

	// Create and open key and subkey.
	long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
	                              szKeyBuf,
	                              0, NULL, REG_OPTION_NON_VOLATILE,
	                              KEY_ALL_ACCESS, NULL,
	                              &hKey, NULL) ;
	if (lResult != ERROR_SUCCESS)
	{
		return FALSE ;
	}

	// Set the Value.
	if (szValue != NULL)
	{
		RegSetValueEx(hKey, NULL, 0, REG_SZ,
		              (BYTE *)szValue,
		              strlen(szValue)+1) ;
	}

	RegCloseKey(hKey) ;
	return TRUE ;
}


HRESULT SetKeyAsCLSID(HKEY hKey, LPSTR lpSubkey, const CLSID& clsid, const char* szFriendlyName)
{
	HKEY 	hNewKey1, hNewKey2;

	// Convert the CLSID into an char str
	char szCLSID[CLSID_STRING_SIZE+2] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	if (ERROR_SUCCESS == RegCreateKeyEx(hKey, lpSubkey, 0, NULL, REG_OPTION_NON_VOLATILE,
																				KEY_ALL_ACCESS, NULL, &hNewKey1, NULL)) {
		if (ERROR_SUCCESS == RegCreateKeyEx(hNewKey1, szCLSID, 0, NULL, REG_OPTION_NON_VOLATILE,
																				KEY_ALL_ACCESS, NULL, &hNewKey2, NULL)) {
			if (szFriendlyName && strlen(szFriendlyName))
				RegSetValueEx(hNewKey2, NULL, 0, REG_SZ, (BYTE*)szFriendlyName, strlen(szFriendlyName)+1);
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT DeleteKeyAsCLSID(HKEY hKey, LPSTR lpSubkey, const CLSID& clsid)
{
	HKEY 	hNewKey;

	// Convert the CLSID into an char str
	char szCLSID[CLSID_STRING_SIZE+2] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	LONG lResult = RegOpenKeyEx(hKey, lpSubkey, 0, KEY_ALL_ACCESS, &hNewKey);
	if (ERROR_SUCCESS != lResult)
		return E_FAIL;

	lResult = recursiveDeleteKey(hNewKey, szCLSID);
	if ((lResult == ERROR_SUCCESS) || (lResult == ERROR_FILE_NOT_FOUND))
		return S_OK;
	
	return E_FAIL;
}



