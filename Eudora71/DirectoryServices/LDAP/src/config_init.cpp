// LDAP Directory Services 
//
// File:     config_init.cpp
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 08/25/1997 Qualcomm Inc.  All Rights Reserved.
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
// Look for and load pre-defined configuration data for the LDAP client.  This pre-defined
// data will appear in a file named "LDAPinit.ini" in the same directory as the LDAP.DLL 
// resides.  This pre-defined configuration is ONLY loaded if no other configuration data already
// exists in the registry (or other storage device like ACAP).
// 
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <stdlib.h>
#include <string.h>
#include "DebugNewHelpers.h"
#include <dspapi.h>
#include "standard.h"
#include "config.h"
#include "log.h"
#include "util.h"

// Copy all fixed (i.e., key strings that we know) pre-defined configuration data from the ".ini" file 
// to our permanent configuration store (e.g., Win32 Registry, ACAP server, etc.).
//
void Config::StoreFixedConfig( char*  szFileName,        // in: path to ".ini" file
                               char*  szReadSection,     // in: Section in the ".ini" file input to read from
                               char*  szWriteSection     // in: Section in the registry to write to
				 			 )
{
     DWORD BytesCopied;
     UINT  iValue;

     BytesCopied = GetPrivateProfileString( szReadSection, "AccountName", "", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".AccountName" );
         m_ConfigIO->SetDirStr(    szWriteSection, ".AccountName", (LPSTR)m_UtilBuf );
     }
/*     BytesCopied = GetPrivateProfileString( szReadSection, "AccountPassword", "", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".AccountPassword" );
         m_ConfigIO->SetDirStr(    szWriteSection, ".AccountPassword", (LPSTR)m_UtilBuf );
     }*/

     BytesCopied = GetPrivateProfileString( szReadSection, "LogFile", "", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".LogFile" );
         m_ConfigIO->SetDirStr(    szWriteSection, ".LogFile", (LPSTR)m_UtilBuf );
     }
     BytesCopied = GetPrivateProfileString( szReadSection, "SearchBase", "c=US", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     m_ConfigIO->CreateDirKey( szWriteSection, ".SearchBase" );
     m_ConfigIO->SetDirStr(    szWriteSection, ".SearchBase", (LPSTR)m_UtilBuf );

     BytesCopied = GetPrivateProfileString( szReadSection, "UserSrvName", "", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".UserSrvName" );
         m_ConfigIO->SetDirStr(    szWriteSection, ".UserSrvName", (LPSTR)m_UtilBuf );
     }
     BytesCopied = GetPrivateProfileString( szReadSection, "LogOverWrite", "FALSE", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".LogOverWrite" );
         m_ConfigIO->SetDirBool(   szWriteSection, ".LogOverWrite", (BOOL)(lstrcmpi( m_UtilBuf, "TRUE" ) == 0));
     }
     BytesCopied = GetPrivateProfileString( szReadSection, "LogonRequired", "FALSE", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".LogonRequired" );
         m_ConfigIO->SetDirBool(   szWriteSection, ".LogonRequired", (BOOL)(lstrcmpi( m_UtilBuf, "TRUE" ) == 0));
     }
     BytesCopied = GetPrivateProfileString( szReadSection, "SSLRequired", "FALSE", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".SSLRequired" );
         m_ConfigIO->SetDirBool(   szWriteSection, ".SSLRequired", (BOOL)(lstrcmpi( m_UtilBuf, "TRUE" ) == 0));
     }
     BytesCopied = GetPrivateProfileString( szReadSection, "SearchFilter", "", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".SearchFilter" );
         m_ConfigIO->SetDirStr(    szWriteSection, ".SearchFilter", (LPSTR)m_UtilBuf );
     }
     BytesCopied = GetPrivateProfileString( szReadSection, "CNSearchFilter", "", m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied > 0) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".CNSearchFilter" );
         m_ConfigIO->SetDirStr(    szWriteSection, ".CNSearchFilter", (LPSTR)m_UtilBuf );
     }

     iValue = GetPrivateProfileInt( szReadSection, "Port", 389, szFileName );
     m_ConfigIO->CreateDirKey( szWriteSection, ".Port" );
     m_ConfigIO->SetDirDword(  szWriteSection, ".Port", (DWORD)iValue );

     iValue = GetPrivateProfileInt( szReadSection, "LogFlags", 0, szFileName );
     m_ConfigIO->CreateDirKey( szWriteSection, ".LogFlags" );
     m_ConfigIO->SetDirDword(  szWriteSection, ".LogFlags", (DWORD)iValue );

     iValue = GetPrivateProfileInt( szReadSection, "SearchHitLimit", 0, szFileName );
     m_ConfigIO->CreateDirKey( szWriteSection, ".SearchHitLimit" );
     m_ConfigIO->SetDirDword(  szWriteSection, ".SearchHitLimit", (DWORD)iValue );

     iValue = GetPrivateProfileInt( szReadSection, "SearchTimeOut", 0, szFileName );
     m_ConfigIO->CreateDirKey( szWriteSection, ".SearchTimeOut" );
     m_ConfigIO->SetDirDword(  szWriteSection, ".SearchTimeOut",  (DWORD)iValue  );

     BytesCopied = GetPrivateProfileString( szReadSection, "ReturnAll", "TRUE", m_UtilBuf,
					    UTIL_BUF_SIZE, szFileName );
     if ( BytesCopied > 0 ) {
         m_ConfigIO->CreateDirKey( szWriteSection, ".ReturnAll" );
         m_ConfigIO->SetDirBool( szWriteSection, ".ReturnAll", (BOOL)(lstrcmpi( m_UtilBuf, "TRUE" ) == 0) );
     }
}


//
//
void Config::StoreAttribTable( char*  szFileName,        // in: path to ".ini" file
                               char*  szReadSection,     // in: Section in the ".ini" file input to read from
                               char*  szWriteSection     // in: Section in the registry to write to
			  			     )
{
     char   szKey[500];
     char*  pStartToken;
     char*  pEndToken;
     DWORD  attrib_count = 0;
     DWORD  BytesCopied;
     UINT   length;

     // -> since we don't know the names of the keys that make up the section we must read in the section all at once
     m_UtilBuf[0] = '\0';
     BytesCopied = GetPrivateProfileSection( szReadSection, m_UtilBuf, UTIL_BUF_SIZE, szFileName );
     if (BytesCopied == 0) return;
     pStartToken = m_UtilBuf;

     // -> break out each key string pair and write both as one unit into our configuration store
     while( pStartToken )
     {
            // -> data format: key=string0key=string00
            szKey[0] = '\0';
            pEndToken = strchr((const char*) pStartToken, '=' );
            if (pEndToken == NULL) break;
            *pEndToken = '\0';
            lstrcpy( szKey, pStartToken );
            m_ConfigIO->CreateDirKey( szWriteSection, szKey );
            *pEndToken = '=';
            pStartToken = ++pEndToken;
            m_ConfigIO->SetDirStr( szWriteSection, szKey, (LPSTR)pStartToken );
            attrib_count++;

            // -> move to next key=string pair
            length = (UINT) lstrlen( pStartToken ) + 1;
            pStartToken += length;
     }
     m_ConfigIO->CreateDirKey( szWriteSection, ".AttributeCount" );
     m_ConfigIO->SetDirDword(  szWriteSection, ".AttributeCount", attrib_count );
}


//
//
BOOL Config::StorePreDefinedConfig( char szFileName[MAX_PATH] )
{
    char     szGUID[CLSID_STRING_SIZE] = {0};
    char     szReadFrom[100];
    char     szWriteTo[100];
    char     szKey[20];
    UINT     write_count = 0;
    UINT     nServers;
    GUID     nUid;
    HRESULT  hErr;

    // [B] Create the main list of all configured LDAP servers
    //  -> the pre-defined configuration data is stored in standard ".ini" file format
    nServers = GetPrivateProfileInt( "LDAP", "ServerCount", 0, szFileName ); 
    if (nServers == 0) return( FALSE );
    m_ConfigIO->CreateDirSection( "LDAP--Servers", NULL );
    m_ConfigIO->SetDirDword( "LDAP--Servers", ".Version", (DWORD)MAKELONG(0,1));   // -> MAKELONG( MINOR, MAJOR )
    m_ConfigIO->SetDirDword( "LDAP--Servers", ".NextSection", nServers+2 );


    // [C] Create each section and copy over its parameters and attribute table
    for( UINT i=0; i < nServers; i++ )
    {
        wsprintf( szKey, "%d", i+1 );
        DWORD BytesCopied = GetPrivateProfileString( "LDAP", szKey, "",  m_UtilBuf, UTIL_BUF_SIZE, szFileName );
        if (BytesCopied > 0) 
        {
            wsprintf( szWriteTo, LDAP_SECTION, i );
            m_ConfigIO->CreateDirKey( "LDAP--Servers", szWriteTo );
            m_ConfigIO->SetDirStr(    "LDAP--Servers", szWriteTo, (LPSTR)m_UtilBuf );
            m_ConfigIO->CreateDirSection( szWriteTo, NULL );

            // -> we are using GUIDs are unique IDs for each directory server config info
            hErr = CoCreateGuid( &nUid );
            if (SUCCEEDED( hErr )) {
                CLSIDtochar((const CLSID) nUid, szGUID, sizeof( szGUID ));
                m_ConfigIO->SetDirStr( szWriteTo, ".UID", szGUID );
            }
            wsprintf( szReadFrom, "%d-parameters", i+1 );
            StoreFixedConfig( szFileName, szReadFrom, szWriteTo );
            wsprintf( szReadFrom, "%d-attributes", i+1 );
            StoreAttribTable( szFileName, szReadFrom, szWriteTo );
            write_count++;
        }
    }

    m_ConfigIO->SetDirDword( "LDAP--Servers", ".ServerCount", write_count );
    return( TRUE );
}



void Config::ReadPreDefinedConfig()
{
    HMODULE hMod = NULL;

    // get the complete directory path where this DLL resides
    if ( (hMod = GetModuleHandle( "LDAP.DLL" )) != NULL ) {
        char szFileName[MAX_PATH] = "";

        if ( GetModuleFileName( hMod, szFileName, MAX_PATH ) ) {
            char* pEndToken = NULL;

            if ( (pEndToken = strrchr((const char*) szFileName, '\\' )) != NULL ) {
                *pEndToken = '\0';

                // whack the ini file name on da back
                if ( (lstrlen( szFileName ) + 13) <= MAX_PATH ) {
                    lstrcat( szFileName, "\\LDAPinit.ini" );

                    // check the "update registry" flag stored in ini file
                    char buf[8] = "";
                    GetPrivateProfileString( "LDAP", "UpdateRegistry", "FALSE", buf, sizeof(buf), szFileName );
                    bool bUpdateRegistry = (lstrcmpi( buf, "TRUE" ) == 0);

                    // -> the version number of "0" is invalid
                    DWORD version = 0;
                    HRESULT hErr = m_ConfigIO->GetDirDword( "LDAP--Servers", ".Version", 0, &version );

                    if ( GetScode( hErr ) == E_FAIL || version == 0 || bUpdateRegistry )
                        StorePreDefinedConfig( szFileName );
                }
            }
        }
    }
}
