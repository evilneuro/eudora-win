// LDAP Directory Services 
//
// File: 			util.cpp
// Copyright		08/12/1997 Qualcomm Inc.  All Rights Reserved.
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
// Description:		
// General Purpose utility functions used by one or more object.
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include <stdlib.h>
#include "DebugNewHelpers.h"
#include "standard.h"
#include "pldap.h"
#include "util.h"
#include "resource.h"

extern HINSTANCE g_hInstance;


// Translate an LDAP result code into a printable string.
//
BOOL TranslateResultCode( int    ResultCode,  // in:
                          LPSTR  pszBuffer,   // out: 
                          int    nlen         // in:
                        )
{
    UINT string_id;

    if (pszBuffer == NULL) return( FALSE );
    pszBuffer[0] = '\0';

    // -> not sure that these mappings from possible LDAP errors to our stored strings make complete sense
    switch( ResultCode ) {
    case LDAP_OPERATIONSERROR:
    case LDAP_PROTOCOLERROR:
    case LDAP_NOSUCHATTRIBUTE:
    case LDAP_UNDEFINEDATTRTYPE:
    case LDAP_INAPPROPIATEMATCING:
    case LDAP_ALIASPROBLEM:
    case LDAP_INVALIDDNSYNTAX:          // -> really general purpose failure
    case LDAP_ALIASDEREFPROBLEM:        string_id = IDS_LDAP_ERROR1;  break;

    case LDAP_AUTHMETHODNOTSUPPORTED:   string_id = IDS_LDAP_ERROR2;  break;

    case LDAP_STRONGAUTHREQUIRED:       string_id = IDS_LDAP_ERROR3;  break;

    case LDAP_BUSY:
    case LDAP_UNAVAILABLE:              string_id = IDS_LDAP_ERROR4;  break;

    case LDAP_INAPPROPRIATEAUTH:
    case LDAP_INVALIDCREDENTIALS:
    case LDAP_INSUFFICENTRIGHTS:        string_id = IDS_LDAP_ERROR6;  break;

    case LDAP_EXP_CONNECTIONCLOSED:     string_id = IDS_LDAP_ERROR7;  break;

    case LDAP_REQUEST_PENDING:          string_id = IDS_LDAP_ERROR8;  break;

    case LDAP_EXP_BADHOSTADDR:          string_id = IDS_LDAP_ERROR9;  break;

    case LDAP_TIMELIMITEXCEEDED:        string_id = IDS_LDAP_ERROR10; break;

    case LDAP_SIZELIMITEXCEEDED:        string_id = IDS_LDAP_ERROR11; break;

    case LDAP_COMPAREFALSE:
    case LDAP_COMPARETRUE:
    case LDAP_SUCCESS:                  string_id = IDS_LDAP_SUCCESS; break;

    case LDAP_CONSTRAINTVIOLATION:
    case LDAP_ATTRORVALUEEXISTS:
    case LDAP_INVALIDATTRSYNTAX:
    case LDAP_NOSUCHOBJECT:
    case LDAP_ISLEAF:
    case LDAP_UNWILLINGTOPERFORM:
    case LDAP_LOOPDETECT:
    case LDAP_NAMINGVIOLATION:
    case LDAP_OBJCLASSVIOLATION:
    case LDAP_NOTALLOWEDONNONLEAF:
    case LDAP_NOTALLOWEDONRDN:
    case LDAP_ENTRYALREADYEXISTS:
    case LDAP_NOOBJCLASSMODS:
    case LDAP_OTHER:
    default:                            string_id = IDS_LDAP_ERROR5;  break;
    } 
    LoadString( g_hInstance, string_id, pszBuffer, nlen );
    return( TRUE );
}


// Convert a CLSID to a char string.
//
void CLSIDtochar( const CLSID& clsid,
                  LPSTR        szCLSID,
                  int          length
                )
{
	// -> get CLSID
	LPOLESTR wszCLSID = NULL;
	//HRESULT  hr       = StringFromCLSID( clsid, &wszCLSID );
	StringFromCLSID( clsid, &wszCLSID );

	// -> covert from wide characters to non-wide.
	wcstombs( szCLSID, wszCLSID, length );
	CoTaskMemFree( wszCLSID );
}


// Convert a character string into a GUID structure
//
HRESULT charToCLSID( const CLSID& clsid, LPSTR szCLSID )
{
	wchar_t wchBuffer[CLSID_STRING_SIZE+2];

	mbstowcs( wchBuffer, szCLSID, CLSID_STRING_SIZE );
	return( CLSIDFromString( wchBuffer, (CLSID*)&clsid )); 
}

