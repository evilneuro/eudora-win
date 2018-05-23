// Directory Services
//
// File:     cfg_enum.cpp
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
// Configuration data enum Methods of the IConfigDir COM object.
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include "DebugNewHelpers.h"
#include "proto.h"


// Enumerate all registry subkeys (i.e., to use Sections) below m_hBaseKey.
//
HRESULT __stdcall CDSProtocol::EnumDirSections( LPSTR   szSection,   // out:
										  DWORD*  cbBuffer     // in/out:
									    )
{
    FILETIME LastWriteTime;
    LONG    lResult;

	 //DBREAK;
	
    lResult = RegEnumKeyEx( m_hBaseKey, m_SubKeyIndex++, szSection, cbBuffer, NULL, NULL, NULL, &LastWriteTime );
    if ( lResult == ERROR_SUCCESS ) {
         return( ResultFromScode( S_OK ));
    }
    else {
         m_ErrorValue  = lResult;
         m_SubKeyIndex = 0;
         return( ResultFromScode( E_FAIL ));
    }
}


// Enumerate all the registry value names (i.e., to use Keys) in the Section.
//
HRESULT __stdcall CDSProtocol::EnumDirKeys( LPSTR      szSection,             // in:
									  LPSTR      szKey,                 // out:
									  DWORD*     cbBuffer,              // in/out:
									  DataType*  type_of_stored_value   // out:
									)
{
    DWORD  data_type;
    LONG   lResult;
    SCODE  sResult = S_OK;

	 //DBREAK;
	
    // -> first the "szSection" must already exist as a registry key
    if (!m_EnumKeyValid)
    {
        lResult = RegOpenKeyEx( m_hBaseKey, szSection, 0, KEY_ALL_ACCESS, &m_hEnumKey );
        if (lResult != ERROR_SUCCESS) {
            m_ErrorValue = lResult;
            return( ResultFromScode( E_FAIL ));
        }
        m_EnumKeyValid = TRUE;
    }

    // ->
    lResult = RegEnumValue( m_hEnumKey, m_ValueIndex++, szKey, cbBuffer, NULL, &data_type, NULL, NULL );
    if ( lResult == ERROR_SUCCESS )
    {
         // -> the registry does not distinguish between { DWORDs, Integers, Booleans }
         switch( data_type ) {
         case REG_BINARY: *type_of_stored_value = DIR_BINARY;  break;
         case REG_DWORD:  *type_of_stored_value = DIR_DWORD;   break;
         case REG_SZ:     *type_of_stored_value = DIR_STR;     break;
         default:         *type_of_stored_value = DIR_UNKNOWN; break;
         }
    }
    else {
         m_ErrorValue  = lResult;
         m_ValueIndex = 0;
         sResult      = E_FAIL;
         RegCloseKey( m_hEnumKey );
         m_EnumKeyValid = FALSE;
    }
    return( ResultFromScode( sResult ));
}




// Stop Key Enumeration
//
HRESULT __stdcall CDSProtocol::EndEnumDirKeys( LPSTR ) // in:
{
	//DBREAK;
	
    if (m_EnumKeyValid) {
        RegCloseKey( m_hEnumKey );
        m_ValueIndex   = 0;
        m_EnumKeyValid = FALSE;
    }
    return( S_OK );
}

