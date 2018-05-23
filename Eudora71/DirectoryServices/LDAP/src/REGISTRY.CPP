//
// File: 			registry.cpp
// Date:			7/19/97
// Copyright		07/19/1997 Qualcomm Inc.  All Rights Reserved.
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

// Version:  		1.0 
// Description:		Helper functions for self registering and unregistering a component
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include <assert.h>
#include "DebugNewHelpers.h"
#include "Registry.h"
#include "util.h"


// Set the given key and its value.
//
BOOL setKeyAndValue( LPCSTR pszPath, LPCSTR szSubkey, LPCSTR szValue );

// Delete szKeyChild and all of its descendents.
// Windows NT automatically does recursive delete but Windows 95 does not.
//
LONG recursiveDeleteKey( HKEY hKeyParent, LPCSTR szKeyChild );


// Ignore "local variable is initialized but not referenced" warning because it
// only applies to release builds. Debug builds use the variable(s) to assert.
#pragma warning( disable : 4189 )

// Public function implementation
//
// All Registry changes are made to HKEY_CLASSES_ROOT
//
HRESULT RegisterServer( HMODULE      hModule,           // DLL module handle
                        const CLSID& clsid              // Class ID
                      )
{	
	// -> get server location.
	char szModule[512];
	DWORD dwResult = ::GetModuleFileName( hModule, szModule, sizeof( szModule )/sizeof( char ));
	assert( dwResult != 0 );

	// -> convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE];
	CLSIDtochar( clsid, szCLSID, sizeof( szCLSID ));

	// -> build the key CLSID\\{...}
	char szKey[64];
	lstrcpy( szKey, "CLSID\\" );
	lstrcat( szKey, szCLSID );
  
    // -> we must have the server filename subkey under the CLSID key.
	setKeyAndValue( szKey, "InprocServer32", szModule ); 
	return( S_OK );
}


// Remove the component from the registry.
//
LONG UnregisterServer( const CLSID& clsid )        // Class ID
{
	// Convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE];
	CLSIDtochar( clsid, szCLSID, sizeof( szCLSID ));

	// -> build the key CLSID\\{...}
	char szKey[64];
	lstrcpy( szKey, "CLSID\\" );
	lstrcat( szKey, szCLSID );

	// -> delete the CLSID Key - CLSID\{...}
	LONG lResult = recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)); // Subkey may not exist.

	return( S_OK );
}
// Ignore "local variable is initialized but not referenced" warning because it
// only applies to release builds. Debug builds use the variable(s) to assert.
#pragma warning( default : 4189 )


// Used to tell other Directory Services COM objects what class IDs we support in this DLL.
//
HRESULT InstallServer( HKEY         mainKey,        // already open main key
                       LPSTR        subKey,			// subKey to place all our clsid under
                       const CLSID& clsid           // Class ID
                     )
{
    HKEY  hKey1, hKey2;
    LONG  lResult;
    SCODE sResult = S_OK;

	// -> convert the CLSID into a char.
	char szCLSID[CLSID_STRING_SIZE];
	CLSIDtochar( clsid, szCLSID, sizeof( szCLSID ));

	// -> build the key CLSID\\{...}
	char szKey[64] = {0};
	lstrcpy( szKey, "CLSID\\" );
	lstrcat( szKey, szCLSID );
  
    // -> make a mainkey\subkey\clsid key in the registry
	lResult = RegOpenKeyEx( mainKey, subKey, 0, KEY_ALL_ACCESS, &hKey1 );
    if (lResult != ERROR_SUCCESS) return( E_FAIL );

	lResult = RegCreateKeyEx( hKey1, szCLSID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey2, NULL );
    if ( lResult != ERROR_SUCCESS ) 
         sResult = E_FAIL;
    else RegCloseKey( hKey2 );

    RegCloseKey( hKey1 );
	return( sResult );
}



// Internal helper functions
//
// Delete a key and all of its descendents.
//
LONG recursiveDeleteKey( HKEY   hKeyParent,        // Parent of key to delete
                         LPCSTR lpszKeyChild       // Key to delete
                       )
{
	// -> open the child.
	HKEY hKeyChild ;
	LONG lResult = RegOpenKeyEx( hKeyParent, lpszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild );
	if (lResult != ERROR_SUCCESS) return( lResult );

	// -> enumerate all of the decendents of this child.
	FILETIME time;
	char     szBuffer[256];
	DWORD    dwSize = 256;

	while( RegEnumKeyEx( hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time ) == S_OK )
	{
		// -> delete the decendents of this child.
		lResult = recursiveDeleteKey( hKeyChild, szBuffer );
		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey( hKeyChild );
			return( lResult );
		}
		dwSize = 256;
	}

	// -> delete this child.
	RegCloseKey( hKeyChild );
	return( RegDeleteKey( hKeyParent, lpszKeyChild ));
}


// Create a key and set its value.
//   - This helper function was borrowed and modifed from
//     Kraig Brockschmidt's book Inside OLE.
//
BOOL setKeyAndValue( LPCSTR szKey,
                     LPCSTR szSubkey,
                     LPCSTR szValue
                   )
{
	char szKeyBuf[1024];
	HKEY hKey;

	lstrcpy( szKeyBuf, szKey );
	if (szSubkey != NULL)
	{
		lstrcat( szKeyBuf, "\\" );
		lstrcat( szKeyBuf, szSubkey );
	}

	// -> create and open key and subkey.
	LONG lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, szKeyBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL );
	if (lResult != ERROR_SUCCESS) return( FALSE );

	// -> set the Value.
	if (szValue != NULL) RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)szValue, lstrlen(szValue)+1 );
	RegCloseKey( hKey );
	return( TRUE );
}


//
//
HRESULT DeleteKeyAsCLSID( HKEY hKey, LPSTR lpSubkey, const CLSID& clsid )
{
	HKEY 	hNewKey;

	// Convert the CLSID into an char str
	char szCLSID[CLSID_STRING_SIZE+2] ;
	CLSIDtochar( clsid, szCLSID, sizeof( szCLSID )) ;

	LONG lResult = RegOpenKeyEx( hKey, lpSubkey, 0, KEY_ALL_ACCESS, &hNewKey );
	if (ERROR_SUCCESS != lResult) return( E_FAIL );

	lResult = recursiveDeleteKey( hNewKey, szCLSID );
	if ((lResult == ERROR_SUCCESS) || (lResult == ERROR_FILE_NOT_FOUND)) return( S_OK );
	return( E_FAIL );
}


