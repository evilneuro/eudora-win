#ifndef __Registry_H__
#define __Registry_H__
//
// Registry.h
//   - Helper functions registering and unregistering a component.
//

// This function will register a component in the Registry.
// The component calls this function from its DllRegisterServer function.
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const char* szFriendlyName,
                       const char* szVerIndProgID,
                       const char* szProgID,
											 const char* szExtraInfo);

// This function will unregister a component.  Components
// call this function from their DllUnregisterServer function.
HRESULT UnregisterServer(const CLSID& clsid,
                         const char* szVerIndProgID,
                         const char* szProgID,
												 const char* szExtraInfo);

HRESULT charToCLSID(const CLSID& clsid, char* szCLSID);

HRESULT SetKeyAsCLSID(HKEY hKey, LPSTR lpSubkey, const CLSID& clsid, const char* szFriendlyName);

HRESULT DeleteKeyAsCLSID(HKEY hKey, LPSTR lpSubkey, const CLSID& clsid);

#endif