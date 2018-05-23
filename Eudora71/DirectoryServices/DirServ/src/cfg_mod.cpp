// Directory Services
//
// File:     cfg_mod.cpp
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 07/17/1997 Qualcomm Inc.  All Rights Reserved.
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
// Configuration data modification Methods of the IConfigDir COM object.
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include "DebugNewHelpers.h"
#include "proto.h"


// We are storing an Integer as a DWORD, since the registry does not have an integer native type.
//
HRESULT __stdcall CDSProtocol::SetDirInteger( LPSTR  szSection,      // in:
										LPSTR  szKey,          // in:
										int    new_value       // in:
									  )
{
	//DBREAK;
	
    return( SetDirDword( szSection, szKey, (DWORD)new_value ));
}


// We are storing a Boolean as a DWORD, since the registry does not have a Boolean native type.
//
HRESULT __stdcall CDSProtocol::SetDirBool( LPSTR  szSection,      // in:
								     LPSTR  szKey,          // in:
									 BOOL   new_value       // in:
								   )
{
	//DBREAK;
	
    return( SetDirDword( szSection, szKey, (DWORD)new_value ));
}



//
//
HRESULT __stdcall CDSProtocol::SetDirDword( LPSTR   szSection,      // in:
									  LPSTR   szKey,          // in:
									  DWORD   new_value	      // in:
									)
{
    HKEY  hSection;
    DWORD buf_size = sizeof( DWORD );
    SCODE sResult  = S_OK;
    LONG  lResult;

	 //DBREAK;
	
    // -> first the "szSection" must already exist as a registry key
    lResult = RegOpenKeyEx( m_hBaseKey, szSection, 0, KEY_ALL_ACCESS, &hSection );
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        return( ResultFromScode( E_FAIL ));
    }

    // ->
    lResult = RegSetValueEx( hSection, szKey, NULL, REG_DWORD, (CONST BYTE*)&new_value, buf_size );
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        sResult = E_FAIL;
    }
    RegCloseKey( hSection );
    return( ResultFromScode( sResult ));
}


// The registry has a native string type.
//
HRESULT __stdcall CDSProtocol::SetDirStr( LPSTR  szSection,       // in:
									LPSTR  szKey,           // in:
									LPSTR  new_value        // in:
								  )
{
    HKEY  hSection;
    SCODE sResult  = S_OK;
    LONG  lResult;

	 //DBREAK;
	
    // -> first the "szSection" must already exist as a registry key
    lResult = RegOpenKeyEx( m_hBaseKey, szSection, 0, KEY_ALL_ACCESS, &hSection );
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        return( ResultFromScode( E_FAIL ));
    }

    // -> is the data type the same as its supposed to be ?
    lResult = RegSetValueEx( hSection, szKey, NULL, REG_SZ, (LPBYTE)new_value, (DWORD)lstrlen(new_value)+1);
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        sResult = E_FAIL;
    }
    RegCloseKey( hSection );
    return( ResultFromScode( sResult ));
}


// The registry has a native Binary type.
//
HRESULT __stdcall CDSProtocol::SetDirBinary( LPSTR   szSection,       // in:
									   LPSTR   szKey,           // in:
									   LPBYTE  new_value,       // in
									   DWORD   cbValue          // in:
									 )
{
    HKEY  hSection;
    SCODE sResult  = S_OK;
    LONG  lResult;

	 //DBREAK;
	
    // -> first the "szSection" must already exist as a registry key
    lResult = RegOpenKeyEx( m_hBaseKey, szSection, 0, KEY_ALL_ACCESS, &hSection );
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        return( ResultFromScode( E_FAIL ));
    }

    // -> is the data type the same as its supposed to be ?
    lResult = RegSetValueEx( hSection, szKey, NULL, REG_BINARY, new_value, cbValue );
    if (lResult != ERROR_SUCCESS) {
        m_ErrorValue = lResult;
        sResult = E_FAIL;
    }
    RegCloseKey( hSection );
    return( ResultFromScode( sResult ));
}





