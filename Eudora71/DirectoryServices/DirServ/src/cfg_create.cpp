// Directory Services
//
// File:     cfg_create.cpp
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 07/18/1997 Qualcomm Inc.  All Rights Reserved.
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

//
// Description:
// Creation Methods of the IConfigDir COM object.
// We map the concepts of Sections and Keys presented at the IConfigDir interface onto the Win32 Registry as follows:
// (a) a Section is a key under a standard place in the registry for all Qualcomm directory services info.
// (b) a Key is the registry name field of a value under the registry key for a Section.
//
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include "DebugNewHelpers.h"
#include "proto.h"


HRESULT __stdcall CDSProtocol::GetDirLastError( LONG* ResultCode )   // out:
{
	//DBREAK;
	
    *ResultCode  = m_ErrorValue;
    m_ErrorValue = 0;
    return( ResultFromScode( S_OK ));
}
					
// For the registry implementation of this interface a "szSection" is just a registry key.
//
STDMETHODIMP CDSProtocol::CreateDirSection( LPSTR szSection, BOOL *pbExists )  // in:
{
    DWORD  disposition;
    HKEY   hSection;
    LONG   lResult;

    lResult = RegCreateKeyEx( m_hBaseKey, szSection, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSection, &disposition );
    if ( lResult == ERROR_SUCCESS ) {
         RegCloseKey( hSection );
				 if (pbExists)
					 *pbExists = (disposition == REG_OPENED_EXISTING_KEY);
         return( ResultFromScode( S_OK ));
    }
    else {
         m_ErrorValue = lResult;
         return( ResultFromScode( E_FAIL ));
    }
}

#if 0
// For the registry implementation of this interface a "szSection" is just a registry key.
//
HRESULT __stdcall CDSProtocol::CreateDirSection( LPSTR szSection )  // in:
{
    DWORD  disposition;
    HKEY   hSection;
    LONG   lResult;

	 //DBREAK;
	
    lResult = RegCreateKeyEx( m_hBaseKey, szSection, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSection, &disposition );
    if ( lResult == ERROR_SUCCESS ) {
         RegCloseKey( hSection );
         return( ResultFromScode( S_OK ));
    }
    else {
         m_ErrorValue = lResult;
         return( ResultFromScode( E_FAIL ));
    }
}
#endif

// For the registry implementation of this interface a "szKey" is a value's name field (NOT A REGISTRY KEY).
// So all we do here is to make sure the "szSection" already exists, because when we write we will specify its registry name.
//
HRESULT __stdcall CDSProtocol::CreateDirKey( LPSTR szSection,  // in:
									   LPSTR       // in:
									 )
{
    HKEY hSection;
    LONG lResult;

	 //DBREAK;
	
    // -> first the "szSection" must already exist as a registry key
    lResult = RegOpenKeyEx( m_hBaseKey, szSection, 0, KEY_ALL_ACCESS, &hSection );
    if ( lResult == ERROR_SUCCESS ) {
         RegCloseKey( hSection );
         return( ResultFromScode( S_OK ));
    }
    else {
         m_ErrorValue = lResult;
         return( ResultFromScode( E_FAIL ));
    }
}


// On Windows 95: The RegDeleteKey function deletes a key and all its descendents
// On Windows NT: The RegDeleteKey function deletes the specified key. This function cannot delete a key that has subkeys
//
// The best way to use this function is to make sure that all "szKey"s under an "szSection" are deleted first via the
// DeleteDirKey function.
//
HRESULT __stdcall CDSProtocol::DeleteDirSection( LPSTR szSection )  // in:
{
	//DBREAK;
	
    LONG lResult  = RegDeleteKey( m_hBaseKey, (LPCTSTR)szSection );
    if ( lResult != ERROR_SUCCESS ) {
         m_ErrorValue = lResult;
         return( ResultFromScode( E_FAIL ));
    }
    else return( ResultFromScode( S_OK ));
}


// For our implementation this translates into deleting a registry value under the registry key for "szSection".
//
HRESULT __stdcall CDSProtocol::DeleteDirKey( LPSTR szSection,   // in:
									   LPSTR szKey        // in:
									 )
{
    HKEY hSection;
    LONG lResult;

	 //DBREAK;
	
    // -> first the "szSection" must already exist as a registry key
    lResult = RegOpenKeyEx( m_hBaseKey, szSection, 0, KEY_ALL_ACCESS, &hSection );
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        return( ResultFromScode( E_FAIL ));
    }

    // ->
    lResult = RegDeleteValue( hSection, (LPCTSTR)szKey );
    if ( lResult != ERROR_SUCCESS ) {
         m_ErrorValue = lResult;
         return( ResultFromScode( E_FAIL ));
    }
    else return( ResultFromScode( S_OK ));
}


