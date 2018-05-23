// LDAP Directory Services 
//
// File:     config.cpp
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 06/18/1997 Qualcomm Inc.  All Rights Reserved.
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
// All configuration for the LDAP object and its child objects are obtained and stored via
// this "Config" object.
//
#include <afx.h>
#pragma warning(disable : 4514 4706)
#include <objbase.h>
#include <windowsx.h>
#include <prsht.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include "DebugNewHelpers.h"
#include <dspapi.h>
#include "standard.h"
#include "config.h"
#include "log.h"
#include "util.h"
#include "resource.h"

extern HINSTANCE g_hInstance;



// We don't write numbers IDs into a configuration file since IDs often change with the addition or
// deletion of entries.
// 
// -> DON'T change these strings they are what is written into the registry
#define CANNED_ATTRIBUTES  33
static char* AttribNames[CANNED_ATTRIBUTES] = 
{  
        // -> attributes common to the work place
    "USERDEFINED", "NAME",     "COMPANY",  "DIVISION",  "POSITION",  "LOCATION",  
    "ADDRESS",     "CITY",     "STATE",    "ZIPCODE",   "PHONE",     "CELL",
    "PAGER",       "PEMAIL",   "FAX",      "EMAIL",     "PGPKEY",    "PAGERTYPE",
    "PAGERPIN",
    
    // -> attributes common to home                                                                                                                                                 
    "HADDRESS",    "HCITY",    "HSTATE",   "HZIPCODE",  "HPHONE",    "HCELL",
    "HPAGER",          "HPEMAIL",  "HFAX",     "HEMAIL",    "HPGPKEY",   "HPAGERTYPE",
    "HPAGERPIN",
    
    // the null attribute
    "NULL",
};

static DS_FIELD InternalNames[CANNED_ATTRIBUTES] =
{
        // -> attributes common to the work place
    DS_USERDEFINED, DS_NAME,    DS_COMPANY, DS_DIVISION, DS_POSITION, DS_LOCATION, 
    DS_ADDRESS,     DS_CITY,    DS_STATE,   DS_ZIPCODE,  DS_PHONE,    DS_CELL,
    DS_PAGER,           DS_PEMAIL,  DS_FAX,     DS_EMAIL,    DS_PGPKEY,   DS_PAGERTYPE, 
    DS_PAGERPIN,
    
    // -> attributes common to home                                                                                                                                            
    DS_HADDRESS,    DS_HCITY,       DS_HSTATE,  DS_HZIPCODE, DS_HPHONE,   DS_HCELL,
    DS_HPAGER,          DS_HPEMAIL, DS_HFAX,    DS_HEMAIL,   DS_HPGPKEY,  DS_HPAGERTYPE,
    DS_HPAGERPIN,

    // the null attribute
    DS_NULL,
};


// -> DON'T change these strings they are what is written into the registry
#define CANNED_TYPES  10
static char* ValueTypeNames[CANNED_TYPES] =
{
        "ASCII",     "ASCIIZ",   "BINARY",   "GIF",   "JPG",   "IMAGE",   
        "WORDDOC",   "EXCEL",    "URL",      "NULL",
};

// -> order in array MUST match order defined in dsapi.h
static DS_TYPE InternalTypes[CANNED_TYPES] =
{
        DST_ASCII,   DST_ASCIIZ, DST_BINARY, DST_GIF, DST_JPG, DST_IMAGE, 
        DST_WORDDOC, DST_EXCEL,  DST_URL, DST_NULL,
};


// -> to send multiple data items to a winproc
typedef struct {
        HWND      hwndList;    // -> handle to a list view control
        int       index;       // -> if == -1 then new value, otherwise existing value
        LPCONFIG  Config;      // -> to get data and built in functions 
} ModifyData_t;


// -> Dlg Boxes for Config into need a pointer to the object to query/write to 
static OPENFILENAME  g_ofn;

// ->
BOOL GetListViewContents( HWND hwndList, int nRow, char* szContents, int content_size, DS_FIELD* InternalName, DS_TYPE* DataType );




//****************************************************************
// Class Management Methods
//
//****************************************************************
//
// This C++ object is used in one of the following ways:
// 1) To iterate all the configured servers with their Section Names,
// 2) To read/write configuration data for a existing server, and 
// 3) To define a new server
// A new instance of this object must be created for each individual task 
// listed above (i.e., no one instance should be used for all three jobs). 
//
Config::Config( IDirConfig* ConfigIO,
                LPSTR       szServerName,
                LPSTR       szSectionName ) : m_LogObj( NULL ),
                                                                                          m_pListViewEx( NULL ),
                                                                                      m_AttribMap( NULL ),
                                                                                      m_MapSize( 0 ),
                                                                                      m_ServerList( NULL ),
                                                                                      m_ServerSize( 0 ),
                                                                                          m_NextServer( 0 ),
                                                                                      m_MapModified( FALSE ),
                                                                                      m_NetModified( FALSE ),
                                                                                          m_OptionsModified( FALSE ),
                                                                                          m_LogModified( FALSE ),
                                                                                      m_TotalBytes( 0 ),
                                                                                      m_CurrentByte( 0 ),
                                                                                      m_ConfigIO( ConfigIO )
{
    // -> does this object represents one, specific server ?
    if (szServerName && szSectionName) {
        NewServerList( szServerName, szSectionName, NULL );
    }
}


//
//
Config::~Config()
{
        // -> has the configuration data been modified ?
        StoreServerList();

        FreeAttributeMap();
    FreeServerList();
    // ** NOTES **
        // -> don't free the log obj because we are sharing it with other objects
}



//****************************************************************
// Handling Attribute Name Mapping Configuration 
//
//****************************************************************
//
// Load in the configuration data for an existing, pre-selected server.
//
void Config::LoadServerInfo()
{
    NameMap_t* AttribName = NULL;
    BOOL AnAttribEntry = TRUE;
    DWORD dwTemp = 0;
    DWORD field_end = 0;
    DWORD length = 0;
    DWORD i = 0;
    DWORD j = 0;
    DWORD in_seconds = 0;
    DataType Type;
    DWORD dwSize; 
    char delimintor;
    HRESULT hErr;

    // [A] Allocate the proper sized Attribute Mapping Table
    if ( m_ServerList == NULL ||
         m_ServerList->DomainName == NULL ||
         m_ServerList->SectionName == NULL ) {

        return;
    }

    hErr = m_ConfigIO->GetDirDword( m_ServerList->SectionName,
                                    ".AttributeCount", 0, &m_MapSize );

    if ( FAILED( hErr ) )
        return;

    if ( m_MapSize > 0 ) {
        if ( (m_AttribMap = DEBUG_NEW_NOTHROW NameMap_t[m_MapSize]) == 0 )
            return;

        for( AttribName=m_AttribMap; j < m_MapSize; j++, AttribName++ ) { 
            AttribName->x500Name = NULL;
            AttribName->UserName = NULL; 
        }
    }
    
    // [B] Load Misc server info  
    CopyConfigStr( &(m_ServerList->UserSrvName), ".UserSrvName", NULL );
    hErr = m_ConfigIO->GetDirDword( m_ServerList->SectionName,
                                    ".Port",
                                    389,
                                    &dwTemp );

    m_ServerList->Port = (UINT) dwTemp;
    CopyConfigStr( &(m_ServerList->szUID), ".UID", "Error" );
    CopyConfigStr( &(m_ServerList->LogFile), ".LogFile", NULL );
    m_ConfigIO->GetDirDword( m_ServerList->SectionName,
                             ".LogFlags",
                             0,
                             &(m_ServerList->LogFlags) );

    m_ConfigIO->GetDirBool( m_ServerList->SectionName,
                            ".LogOverWrite",
                            FALSE,
                            &m_ServerList->OverWrite );

    m_ConfigIO->GetDirBool(  m_ServerList->SectionName,
                             ".LogonRequired",
                             FALSE,
                             &m_ServerList->LogonRequired );

    m_ConfigIO->GetDirBool(  m_ServerList->SectionName,
                             ".SSLRequired",
                             FALSE,
                             &m_ServerList->SSLRequired );

    CopyConfigStr( &(m_ServerList->AccountName), ".AccountName", NULL );
//    CopyConfigStr( &(m_ServerList->AccountPassword), ".AccountPassword",NULL );
    CopyConfigStr( &(m_ServerList->SearchBase), ".SearchBase", "c=US" );
    m_ConfigIO->GetDirDword( m_ServerList->SectionName,
                             ".SearchTimeOut",
                             0,
                             &in_seconds );

    m_ServerList->ServerTimeOut =
        (in_seconds < 30 || in_seconds > 300) ? 0 : in_seconds;

    m_ConfigIO->GetDirDword( m_ServerList->SectionName,
                             ".SearchHitLimit",
                             0,
                             &dwSize );

    m_ServerList->ServerHitLimit = (dwSize > 65535) ? 0 : dwSize;

    // load the search filters
    CopyConfigStr(&(m_ServerList->SearchFilter), ".SearchFilter","(cn=*^0*)");
    CopyConfigStr( &(m_ServerList->CNSearchFilter), ".CNSearchFilter", NULL );

    // load the "ReturnAll" state
    m_ConfigIO->GetDirBool( m_ServerList->SectionName,
			    ".ReturnAll",
			    TRUE,
			    &m_ServerList->ReturnAll );

    // [C] Load Attribute Name Map table
    AttribName   = m_AttribMap;
    m_UtilBuf[0] = '\0';    

    while( i < m_MapSize && m_AttribMap ) {

        // -> X.500 attribute name is the registry entry's Name
        m_UtilBuf[0] = '\0';
        dwSize = UTIL_BUF_SIZE;
        hErr = m_ConfigIO->EnumDirKeys( m_ServerList->SectionName,
                                        m_UtilBuf,
                                        &dwSize,
                                        &Type );

        if ( !FAILED( hErr ) ) {

            // -> Keys names starting with a single dot are not part of
            // the attribute table 
            if ( lstrlen( m_UtilBuf ) == 0 ||
                 (m_UtilBuf[0] == '.' && m_UtilBuf[1] != '.')) {

                AnAttribEntry = FALSE;
                goto NextAttrib; 
            }

            if ( (AttribName->x500Name = DEBUG_NEW_NOTHROW char[dwSize+1] ) == 0 )
                goto AttribEnd;

            lstrcpy( AttribName->x500Name, m_UtilBuf );

            // -> all the rest of the mapping info is encoded in a single
            // string which composes a registry entry's value

            m_UtilBuf[0] = '\0';
            dwSize       = UTIL_BUF_SIZE;
            hErr = m_ConfigIO->GetDirStr( m_ServerList->SectionName,
                                          AttribName->x500Name,
                                          "",
                                          0,
                                          m_UtilBuf,
                                          &dwSize );

            if ( FAILED( hErr ) )
                goto AttribEnd;

            m_CurrentByte = 0;
            m_TotalBytes  = dwSize;

            // -> pull out our internal type
            field_end = TokenEnd();
            m_UtilBuf[field_end] = '\0';
            AttribName->DataType = IntoInternalTypeName();
            m_UtilBuf[field_end] = ' ';
            m_CurrentByte = field_end; 

            // -> pull out our internal name
            field_end  = TokenEnd();
            delimintor = m_UtilBuf[field_end];
            m_UtilBuf[field_end] = '\0';
            AttribName->InternalName = IntoInternalName();
            m_UtilBuf[field_end] = delimintor;
            m_CurrentByte        = field_end;

            // -> pull out the user defined name (stripping off both quotes)
            if ( AttribName->InternalName == DS_USERDEFINED ) {
                field_end  = TokenEnd();
                delimintor = m_UtilBuf[field_end];
                m_UtilBuf[field_end] = '\0';
                length = lstrlen( &m_UtilBuf[m_CurrentByte] );
                if ((AttribName->UserName =
                     DEBUG_NEW_NOTHROW char[length+1] ) == 0) goto AttribEnd;

                // -> remove enclosing quotes and any escape chars
                // for internal quotes
				j = 0;
                for( UINT k=m_CurrentByte+1, l=0;
                     l < length-2; k++, l++ )   {

                    if (m_UtilBuf[k] == '\\' && m_UtilBuf[k+1] == '"')
                        continue;

                    AttribName->UserName[j++] = m_UtilBuf[k];
                }

                AttribName->UserName[j] = '\0';
                m_UtilBuf[field_end]    = delimintor;
            }
            else
                AttribName->UserName = NULL;
        }

    NextAttrib:

        // -> Keys names starting with a single dot are not part
        // of the attribute table 
        if ( AnAttribEntry ) {        
            i++;
            AttribName++;
        }

        AnAttribEntry = TRUE;
    }
    
 AttribEnd:
    m_ConfigIO->EndEnumDirKeys( m_ServerList->SectionName );
    DumpNameMap();
}


// Each of these store functions is called by a different dialog proc.
//
void Config::StoreLogInfo()
{
        if (m_ServerList == NULL || m_ServerList->SectionName == NULL) return;
    m_ConfigIO->CreateDirSection( m_ServerList->SectionName, NULL );

    // Write log control information specific to the selected server
    if (m_LogModified) 
    {
        if ( m_ServerList->LogFile ) 
        {
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".LogFile" );
             m_ConfigIO->SetDirStr(    m_ServerList->SectionName, ".LogFile", (LPSTR)m_ServerList->LogFile );
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".LogFlags" );
             m_ConfigIO->SetDirDword(  m_ServerList->SectionName, ".LogFlags", m_ServerList->LogFlags );
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".LogOverWrite" );
             m_ConfigIO->SetDirBool(   m_ServerList->SectionName, ".LogOverWrite", m_ServerList->OverWrite );
        }
        else {
             // -> remove old values
             m_ConfigIO->DeleteDirKey( m_ServerList->SectionName, ".LogFile"      );
             m_ConfigIO->DeleteDirKey( m_ServerList->SectionName, ".LogFlags"     );
             m_ConfigIO->DeleteDirKey( m_ServerList->SectionName, ".LogOverWrite" );
        }
        m_LogModified = FALSE;
    }
}


// Each of these store functions is called by a different dialog proc.
//
void Config::StoreNetInfo()
{
        if (m_ServerList == NULL || m_ServerList->SectionName == NULL) return;
    m_ConfigIO->CreateDirSection( m_ServerList->SectionName, NULL );

    // Write log control information specific to the selected server
    if (m_NetModified) 
    {
        m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".Port" );
        m_ConfigIO->SetDirDword(  m_ServerList->SectionName, ".Port", (DWORD)m_ServerList->Port );
        m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".LogonRequired" );
        m_ConfigIO->SetDirBool(   m_ServerList->SectionName, ".LogonRequired", m_ServerList->LogonRequired );
        m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".SSLRequired" );
        m_ConfigIO->SetDirBool(   m_ServerList->SectionName, ".SSLRequired", m_ServerList->SSLRequired );

        if ( m_ServerList->DomainName ) 
        {
             m_ConfigIO->CreateDirKey( "LDAP--Servers", m_ServerList->SectionName );
             m_ConfigIO->SetDirStr(    "LDAP--Servers", m_ServerList->SectionName, m_ServerList->DomainName );
        }
        if ( m_ServerList->UserSrvName ) 
        {
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".UserSrvName" );
             m_ConfigIO->SetDirStr(    m_ServerList->SectionName, ".UserSrvName", (LPSTR)m_ServerList->UserSrvName );
        }
        if ( m_ServerList->AccountName ) 
        {
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".AccountName" );
             m_ConfigIO->SetDirStr(    m_ServerList->SectionName, ".AccountName", (LPSTR)m_ServerList->AccountName );
        }
        /*if ( m_ServerList->AccountPassword ) 
        {
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".AccountPassword" );
             m_ConfigIO->SetDirStr(    m_ServerList->SectionName, ".AccountPassword", (LPSTR)m_ServerList->AccountPassword );
        }*/
        m_NetModified = FALSE;
    }
}


// Write the modified Attribute Name Map back into our configuration storage for next time.
//
void Config::StoreOptionInfo()
{
    if ( m_ServerList == NULL || m_ServerList->SectionName == NULL )
        return;

    m_ConfigIO->CreateDirSection( m_ServerList->SectionName, NULL );

    // Write Seach control information specific to the selected server  
    if ( m_OptionsModified ) {
        if ( m_ServerList->SearchBase ) {
            m_ConfigIO->CreateDirKey( m_ServerList->SectionName,
                                      ".SearchBase" );
            m_ConfigIO->SetDirStr( m_ServerList->SectionName,
                                   ".SearchBase",
                                   (LPSTR)m_ServerList->SearchBase );
        }

        m_ConfigIO->CreateDirKey( m_ServerList->SectionName,
                                  ".SearchTimeOut" );
        m_ConfigIO->SetDirDword(  m_ServerList->SectionName,
                                  ".SearchTimeOut",
                                  (DWORD)m_ServerList->ServerTimeOut );

        m_ConfigIO->CreateDirKey( m_ServerList->SectionName,
                                  ".SearchHitLimit" );
        m_ConfigIO->SetDirDword(  m_ServerList->SectionName,
                                  ".SearchHitLimit",
                                  (DWORD)m_ServerList->ServerHitLimit );

        if ( m_ServerList->SearchFilter ) {
            m_ConfigIO->CreateDirKey( m_ServerList->SectionName,
                                      ".SearchFilter" );
            m_ConfigIO->SetDirStr( m_ServerList->SectionName,
                                   ".SearchFilter",
                                   (LPSTR)m_ServerList->SearchFilter );
        }

        if ( m_ServerList->CNSearchFilter ) {
            m_ConfigIO->CreateDirKey( m_ServerList->SectionName,
                                      ".CNSearchFilter" );
            m_ConfigIO->SetDirStr( m_ServerList->SectionName,
                                   ".CNSearchFilter",
                                   (LPSTR)m_ServerList->CNSearchFilter );
        }

        m_OptionsModified = FALSE;
    }
}


// Write the modified Attribute Name Map back into our configuration storage for next time.
//
void Config::StoreAttribMap()
{
        NameMap_t* AttribName = m_AttribMap;
    char*      szTemp     = NULL;
    DWORD      i, j, k, encode_chars;
    DWORD      length,  encoded_length;


        if (m_ServerList == NULL || m_ServerList->SectionName == NULL) return;
    m_ConfigIO->CreateDirSection( m_ServerList->SectionName, NULL );

        // Write out the Attribute Map table for the selected server
        if (!m_MapModified) return;
    m_ConfigIO->CreateDirKey( m_ServerList->SectionName, ".AttributeCount" );
    m_ConfigIO->SetDirDword(  m_ServerList->SectionName, ".AttributeCount", m_MapSize );

        // -> write out each entry
        for( i=0; i < m_MapSize && AttribName; i++, AttribName++ )
        {
         if (AttribName->x500Name) 
         {
             m_UtilBuf[0] = '\0';
             m_ConfigIO->CreateDirKey( m_ServerList->SectionName, AttribName->x500Name );

             // -> <data type><sp><internal name><sp><user name if present>
             wsprintf( m_UtilBuf, "%s %s", IntoTypeString( AttribName->DataType ), IntoAttribString( AttribName->InternalName ));  
                     if (AttribName->UserName != NULL) 
                     {
                 // -> number of extra characters needed to encode string with "" and \" 
                             length = (DWORD) lstrlen( AttribName->UserName );
                             for( j=0, encode_chars=0; j < length; j++ ) if (AttribName->UserName[j] == '"') encode_chars++;
                 encoded_length = length+encode_chars+1+3;
                 szTemp = DEBUG_NEW_NOTHROW char[encoded_length];
                 if (szTemp == 0) continue;
                 szTemp[0] = '\0';
                             
                             // -> writting out with "" and \"
                             szTemp[0] = ' ';
                             szTemp[1] = '"';
                             for( j=0, k=2; j < length && k < encoded_length-2; j++ )
                             {
                                      if (AttribName->UserName[j] == '"') szTemp[k++] = '\\';
                                      szTemp[k++] = AttribName->UserName[j];
                             }
                             szTemp[k++] = '"';
                             szTemp[k]   = '\0';
                     if (encoded_length + lstrlen(m_UtilBuf) < UTIL_BUF_SIZE) lstrcat( m_UtilBuf, szTemp );
                 delete [] szTemp;
                 szTemp = NULL; 
                     }
             m_ConfigIO->SetDirStr( m_ServerList->SectionName, AttribName->x500Name, (LPSTR)m_UtilBuf );
         }
        }
    m_MapModified = FALSE;
}


BOOL Config::GetReturnAll()
{
    if ( m_ServerList )
        return m_ServerList->ReturnAll;
    else
        return TRUE;
}

void Config::SetReturnAll( BOOL bReturnAll )
{
    m_ServerList->ReturnAll = bReturnAll;
    m_ConfigIO->SetDirBool( m_ServerList->SectionName, ".ReturnAll", bReturnAll );
}


// Load the list of all configured servers so we can iterate all of them to our owner.
//
void Config::LoadServerList()
{
        Server_t*  ServerName     = NULL;
    BOOL       AServerEntry   = TRUE;
    DWORD      i = 0, j = 0;
    DataType   Type;
    DWORD      dwSize;
    HRESULT    hErr;

        // [A] Allocate the proper sized table
    hErr = m_ConfigIO->GetDirDword( "LDAP--Servers", ".ServerCount", 1, &m_ServerSize );
    if (FAILED( hErr ) || (m_ServerSize == 0xffffffff)) return;
        if ((m_ServerList = DEBUG_NEW_NOTHROW Server_t[m_ServerSize]) == 0) return;
    for( ServerName=m_ServerList; j < m_ServerSize; j++, ServerName++ ) { 
         ServerName->DomainName = ServerName->SectionName = NULL; 
    }


        // [B] Fill out each entry in the table
        ServerName   = m_ServerList;
    m_UtilBuf[0] = '\0';
        while( i < m_ServerSize )
        {
                 // -> pull out the Server's Domain Name
         m_UtilBuf[0] = '\0';
         dwSize       = UTIL_BUF_SIZE;
         hErr = m_ConfigIO->EnumDirKeys( "LDAP--Servers", m_UtilBuf, &dwSize, &Type );
             if ( !FAILED( hErr )) 
         {
              // -> Keys names starting with a single dot are not a member of the server list (IGNORE IT)
              if (lstrlen( m_UtilBuf ) == 0 || (m_UtilBuf[0] == '.' && m_UtilBuf[1] != '.')) {
                  AServerEntry = FALSE;
                  goto NextServer; 
              }            

              // -> the registry entry's name our generated section name of where all other server specific configuration data is stored
              if ((ServerName->SectionName = DEBUG_NEW_NOTHROW char[dwSize+1]) == 0) goto ServerEnd;
              ServerName->SectionName[0] = '\0';
                      lstrcpy( ServerName->SectionName, m_UtilBuf );

              // -> the entry's value contains the fully qualified LDAP server's domain Name 
              // -> this allows a single domain name to have many different configurations
              m_UtilBuf[0] = '\0';
              dwSize       = UTIL_BUF_SIZE;
              hErr = m_ConfigIO->GetDirStr( "LDAP--Servers", ServerName->SectionName, "", 0, m_UtilBuf, &dwSize );
              if ( FAILED( hErr )) {
                   ServerName->DomainName = NULL;
              }
              else {
                   if ((ServerName->DomainName = DEBUG_NEW_NOTHROW char[dwSize+1]) == 0) goto ServerEnd;
                   ServerName->DomainName[0] = '\0';
                           lstrcpy( ServerName->DomainName, m_UtilBuf );
              }
         }
          // -> these values are stored under ServerName->SectionName
         ServerName->UserSrvName     = NULL;
         ServerName->Port            = 0;
         ServerName->SearchBase      = NULL;
         ServerName->LogFile         = NULL;
         ServerName->OverWrite       = FALSE;
         ServerName->LogFlags        = 0;
         ServerName->szUID           = NULL;
         ServerName->LogonRequired   = FALSE;
		 ServerName->SSLRequired	 = FALSE;
         ServerName->AccountName     = NULL;
         ServerName->AccountPassword = NULL;
         ServerName->ServerTimeOut   = 0;
         ServerName->ServerHitLimit  = 0;
         ServerName->SearchFilter    = NULL;
         ServerName->CNSearchFilter  = NULL;


NextServer:
        // -> Keys names starting with a single dot are not a member of the server list
        if (AServerEntry) {        
            i++;
            ServerName++;
        }
        AServerEntry = TRUE;
        }

ServerEnd:
    m_ConfigIO->EndEnumDirKeys( "LDAP--Servers" );
    DumpServers();
}


// Write out part of the modified ServerList to the "LDAP--Servers" section.
// Server specific information is all placed in a section dedicated to each server.
//
void Config::StoreServerList()
{
        Server_t* ServerName = m_ServerList;

    // -> 
        if (!m_NetModified && m_ServerList) return;
    m_ConfigIO->CreateDirSection( "LDAP--Servers", NULL );
        for( DWORD i=0; i < m_ServerSize && ServerName != NULL; i++, ServerName++ )
        {
         m_ConfigIO->CreateDirKey( "LDAP--Servers", ServerName->SectionName   );
         m_ConfigIO->SetDirStr(    "LDAP--Servers", ServerName->SectionName, ServerName->DomainName );
        }
}


//
//
void Config::SetLog( LPLOG LogTo )   // in:
{
        m_LogObj = LogTo;
}


//****************************************************************
// Member functions to access single server data
//
//****************************************************************
//
// The caller must not free the "properties" parameter returned.
//
void Config::GetServerProperties( Server_t** properties )
{
        *properties = m_ServerList;
}


// A copy of a string is placed into szUserDefined because we do not want to
// force the life time of our Attribute Name Map table to be the same as that
// of the caller's life time.

BOOL Config::MapAttribName
(
    LPSTR     szName,           // in:
    DS_FIELD* internal_name,    // out:
    DS_TYPE*  data_type,        // out:
    LPSTR*    szUserDefined     // out: can be NULL
)
{
    NameMap_t* AttribName = m_AttribMap;
    char*      szTemp     = NULL;

    // [A] Is it in our list of user defined relationships ?
    for( DWORD i=0; i < m_MapSize && AttribName; i++, AttribName++ ) {
        if ( AttribName->x500Name &&
             lstrcmpi( szName, AttribName->x500Name ) == 0 ) {

            if ( AttribName->DataType == DST_NULL )
                return FALSE;

            *internal_name = AttribName->InternalName;
            *data_type     = AttribName->DataType;

            if ( szUserDefined != NULL ) {
                LPTSTR pDisplayName = (AttribName->UserName) ? (LPTSTR) AttribName->UserName
                    : (LPTSTR) KnownFields( *internal_name );

                szTemp = DEBUG_NEW_NOTHROW char[lstrlen( pDisplayName ) + 1];

                if ( szTemp ) {
                    *szTemp = '\0';
                    lstrcpy( (LPTSTR) szTemp, (LPCTSTR) pDisplayName );
                    *szUserDefined = (LPSTR) szTemp;
                }
            }

            return TRUE;
        }
    }

    if ( m_ServerList->ReturnAll ) {

        // [B] Otherwise, is there a standard mapping
        if ( lstrcmpi( szName, "c" ) == 0 ) {
            *internal_name = DS_ADDRESS;
            *data_type = DST_ASCII;
        }
        else if ( lstrcmpi( szName, "cn" ) == 0 ) {
            *internal_name = DS_NAME;
            *data_type = DST_ASCII;
        }
        else if ( lstrcmpi( szName, "sn" ) == 0 ) {
            *internal_name = DS_NAME;
            *data_type = DST_ASCII;
        }
        else if ( lstrcmpi( szName, "mail" ) == 0 ) {
            *internal_name = DS_EMAIL;
            *data_type = DST_ASCII;
        }
        else if ( lstrcmpi( szName, "email" ) == 0 ) {
            *internal_name = DS_EMAIL;
            *data_type = DST_ASCII;
        }
        else {
            // -> we are not sure how the attribute name maps to our internal name
            *internal_name = DS_UNKNOWN;
            *data_type     = DST_ASCII;

            if ( szUserDefined != NULL ) {                
                if ( (szTemp  = DEBUG_NEW_NOTHROW char[lstrlen( szName ) + 1]) != 0 ) {
                    *szTemp = '\0';
                    lstrcpy( (LPTSTR)szTemp, (LPCTSTR)szName );
                    *szUserDefined = (LPSTR) szTemp;
                }
            }
        }
    }
    else {
        // this means that the user has checked the "only show me these" option. if
        // the query module can't map an attribute, it won't be added to the results.
        return FALSE;
    }

    return TRUE;
}


// 
//
char* Config::KnownFields( DS_FIELD KnownField )
{
    m_UtilBuf[0] = '\0';
    HRESULT hErr = m_ConfigIO->GetFieldName( KnownField, (LPSTR)m_UtilBuf, UTIL_BUF_SIZE );
    return((SUCCEEDED( hErr )) ? m_UtilBuf : "" ); 
}


// The caller must not free the "properties" parameter returned.
//
void Config::FirstAttribute( NameMap_t** attribute )
{
        m_EnumAttrib = 0;
        *attribute   = (m_AttribMap == NULL || m_EnumAttrib >= m_MapSize) ? NULL : &m_AttribMap[m_EnumAttrib++];
}


// The caller must not free the "properties" parameter returned.
//
void Config::NextAttribute( NameMap_t** attribute )
{
        *attribute = (m_AttribMap == NULL || m_EnumAttrib >= m_MapSize) ? NULL : &m_AttribMap[m_EnumAttrib++];
}


//
//
BOOL Config::IsLogEnabled( DWORD LogType )  // in: Test for SINGLE log enable flag
{
     if (m_ServerList == NULL) return( FALSE );
     return((LogType & m_ServerList->LogFlags) > 0 );
}


//
//
void Config::ChangePort( UINT Port )   // in:
{
     if (m_ServerList && m_ServerList->Port != Port) {
         m_ServerList->Port = Port;
         m_NetModified  = TRUE;
     }    
}


// 
//
void Config::ChangeDomainName( char* szDomain )   // in:
{
         if (m_ServerList == NULL) return;
     if (m_ServerList->DomainName == NULL || lstrcmp((LPCTSTR) &(m_ServerList->DomainName), (LPCTSTR)szDomain ) != 0) 
     {
             if (m_ServerList->DomainName) { delete [] m_ServerList->DomainName; m_ServerList->DomainName = NULL; }       
         m_ServerList->DomainName = DEBUG_NEW_NOTHROW char[lstrlen(szDomain)+1];
         if (m_ServerList->DomainName) {
                lstrcpy( m_ServerList->DomainName, (LPCTSTR)szDomain );
        }
         m_NetModified = TRUE;
     }
}


// Its what the user wants to call the server.
//
void Config::ChangeUserSrvName( char* szUserSrvName )   // in:
{
         if (m_ServerList == NULL) return;
     if (m_ServerList->UserSrvName == NULL || lstrcmp((LPCTSTR) &(m_ServerList->UserSrvName), (LPCTSTR)szUserSrvName ) != 0) 
     {
             if (m_ServerList->UserSrvName) { delete [] m_ServerList->UserSrvName; m_ServerList->UserSrvName = NULL; }       
         m_ServerList->UserSrvName = DEBUG_NEW_NOTHROW char[lstrlen(szUserSrvName)+1];
         if (m_ServerList->UserSrvName) {
                lstrcpy( m_ServerList->UserSrvName, (LPCTSTR)szUserSrvName );
         }
         m_NetModified = TRUE;
     }
}


// 
//
void Config::ChangeLogon( BOOL  LogonEnabled,  // in:
                          char* szAccount,     // in:
                          char* szPassword,    // in:
						  BOOL  SSLEnabled   // in:
                        )   
{
         if (m_ServerList == NULL) return;

     if (m_ServerList->LogonRequired != LogonEnabled) {
         m_ServerList->LogonRequired  = LogonEnabled;
         m_NetModified = TRUE;
     }
     if (m_ServerList->AccountName == NULL || lstrcmp((LPCTSTR) &(m_ServerList->AccountName), (LPCTSTR)szAccount ) != 0) 
     {
             if (m_ServerList->AccountName) { delete [] m_ServerList->AccountName; m_ServerList->AccountName = NULL; }       
         m_ServerList->AccountName = DEBUG_NEW_NOTHROW char[lstrlen(szAccount)+1];
         if (m_ServerList->AccountName) {
                lstrcpy( m_ServerList->AccountName, (LPCTSTR)szAccount );
        }
         m_NetModified = TRUE;
     }
     if (m_ServerList->AccountPassword == NULL || lstrcmp((LPCTSTR) &(m_ServerList->AccountPassword), (LPCTSTR)szPassword ) != 0) 
     {
             if (m_ServerList->AccountPassword) { delete [] m_ServerList->AccountPassword; m_ServerList->AccountPassword = NULL; }       
         m_ServerList->AccountPassword = DEBUG_NEW_NOTHROW char[lstrlen(szPassword)+1];
         if (m_ServerList->AccountPassword) {
                lstrcpy( m_ServerList->AccountPassword, (LPCTSTR)szPassword );
        }
         m_NetModified = TRUE;
     }
     if (m_ServerList->SSLRequired != SSLEnabled) {
         m_ServerList->SSLRequired  = SSLEnabled;
         m_NetModified = TRUE;
     }
}


// 
//
void Config::ChangeLogInfo( char*  LogPath,    // in:
                            BOOL   OverWrite,  // in:
                                                        DWORD  flags       // in:
                                                  )
{
         if (m_ServerList == NULL || LogPath == NULL) return;
     UINT length = (UINT) lstrlen( LogPath );

     if (m_ServerList->LogFile == NULL || lstrcmpi((LPCTSTR) &(m_ServerList->LogFile), (LPCTSTR)LogPath ) != 0) 
     {
             if (m_ServerList->LogFile) { delete [] m_ServerList->LogFile; m_ServerList->LogFile = NULL; }
         m_ServerList->LogFile = DEBUG_NEW_NOTHROW char[length+1];
         if (m_ServerList->LogFile) {
                lstrcpy( m_ServerList->LogFile, (LPCTSTR)LogPath );
         }
         m_LogModified = TRUE;
     }
     if (m_ServerList->OverWrite != OverWrite) {
             m_ServerList->OverWrite  = OverWrite;
         m_LogModified = TRUE;
     }
     if (m_ServerList->LogFlags != flags) {
             m_ServerList->LogFlags  = flags;
         m_LogModified = TRUE;
     }
}


// 
//
void Config::ChangeOptions
(
    char* SearchBase,             // in:
    const char* SearchFilter,     // in: ldap search filter template
    const char* CNSearchFilter,   // in: template for term-from-query filter
    DWORD ServerTimeOut,          // in: in number of 30 second units
    DWORD ServerHitLimit,         // in:
    BOOL  HitLimit_Valid          // in: ignore last parameter ?
)  
{
    if (m_ServerList == NULL || SearchBase == NULL)
        return;

    UINT length = (UINT) lstrlen( SearchBase );

    if (m_ServerList->ServerTimeOut  != ServerTimeOut) {
        m_ServerList->ServerTimeOut   = ServerTimeOut;
        m_OptionsModified = TRUE;
    }

    if (m_ServerList->ServerHitLimit != ServerHitLimit && HitLimit_Valid) {
        m_ServerList->ServerHitLimit  = ServerHitLimit;
        m_OptionsModified = TRUE;
    }

    if (m_ServerList->SearchBase == NULL ||
        strcmp( (LPCSTR)m_ServerList->SearchBase, (LPCSTR)SearchBase ) != 0 ) {

        if (m_ServerList->SearchBase) {
            delete [] m_ServerList->SearchBase;
            m_ServerList->SearchBase = NULL;
        }

        m_ServerList->SearchBase = DEBUG_NEW_NOTHROW char[length+1];

        if (m_ServerList->SearchBase) {
            lstrcpy( m_ServerList->SearchBase, (LPCTSTR)SearchBase );
        }

        m_OptionsModified = TRUE;
    }


    if ( SearchFilter ) {
        char* pFilter = m_ServerList->SearchFilter;

        if ( !pFilter || strcmp( pFilter, SearchFilter ) ) {
            if ( pFilter ) {
                delete [] pFilter;
                pFilter = NULL;
            }

            pFilter = DEBUG_NEW_NOTHROW char[strlen( SearchFilter ) + 1];

            if ( pFilter ) {
                strcpy( pFilter, SearchFilter );
                m_ServerList->SearchFilter = pFilter;
                m_OptionsModified = TRUE;
            }
        }
    }

    if ( CNSearchFilter ) {
        char* pFilter = m_ServerList->CNSearchFilter;

        if ( !pFilter || strcmp( pFilter, CNSearchFilter ) ) {
            if ( pFilter ) {
                delete [] pFilter;
                pFilter = NULL;
            }

            pFilter = DEBUG_NEW_NOTHROW char[strlen( CNSearchFilter ) + 1];

            if ( pFilter ) {
                strcpy( pFilter, CNSearchFilter );
                m_ServerList->CNSearchFilter = pFilter;
                m_OptionsModified = TRUE;
            }
        }
    }
}


// Simple algorthm at first; free the old table and create a new one.
// Can be a bit inefficient if the user only changes type information. 
// But simplier overall if user makes several changes including deleting and
// adding entries.
//
BOOL Config::ChangeAttribMap( HWND hwndList )
{
    NameMap_t*  AttribName = NULL;
    NameMap_t*  TempMap    = NULL; 
    DWORD       TempSize   = 0;
    DS_FIELD    SelectedName;
    DS_TYPE     Type;
        char        szBuffer[200];
    UINT        length;

    // [A] Create a new blank attribute map
    TempSize = (DWORD)ListView_GetItemCount( hwndList );
        if ((TempMap = DEBUG_NEW_NOTHROW NameMap_t[TempSize]) == 0) return( FALSE );

    // [B] Take into account all possible changes by forcing the entire Name Map to be written after any change
    //  -> we must do this here because the old attribute name map still has all the old key names 
        if (m_ServerList == NULL) { delete [] TempMap; return( FALSE ); }
    DeleteStoredMap( m_ServerList->SectionName );
        FreeAttributeMap();
    m_MapSize   = TempSize;
    m_AttribMap = TempMap;
    if (m_MapSize == 0) return( TRUE );

    
    // [C] Fill in all the attribute map entries from the List View control
    AttribName = m_AttribMap;
    for( DWORD i=0; i < m_MapSize && AttribName; i++, AttribName++ )
    {
         AttribName->UserName = NULL;
         AttribName->x500Name = NULL;

         szBuffer[0] = '\0';
         GetListViewContents( hwndList, (UINT)i, szBuffer, 200, &SelectedName, &Type );
                 length = lstrlen( szBuffer )+1;
         if ((AttribName->x500Name = DEBUG_NEW_NOTHROW char[length] ) != 0) {
             lstrcpy( AttribName->x500Name, szBuffer );
         }

         AttribName->InternalName = (SelectedName <= DS_UNKNOWN) ? SelectedName : DS_UNKNOWN;
         if ( AttribName->InternalName == DS_USERDEFINED ) 
         {
              szBuffer[0] = '\0';
              ListView_GetItemText( hwndList, (int)i, 1, szBuffer, 200 ); 
              length = lstrlen( szBuffer )+1;
              if ((AttribName->UserName = DEBUG_NEW_NOTHROW char[length] ) != 0) {
                  lstrcpy( AttribName->UserName, szBuffer );
              }
         }
         AttribName->DataType = Type;  
    }

    // -> if the entire table was deleted then "DeleteStoreMap" above already made the change
    m_MapModified = TRUE;       
    return( TRUE );
}


// The current server is to be added to our configuration data.
// Handle the following two cases: (1) there is no configuration data at all, 
// (2) add this to an existing list of data
//
BOOL Config::MakeNewServer( char* szBuffer )
{
        char    szGUID[CLSID_STRING_SIZE] = {0};
    BOOL    FoundEmptySection         = FALSE;
    DWORD   server_count              = 0;
    DWORD   next_section              = 1;
    UINT    max_count                 = 0;
    GUID    nUid;
    char    szKey[300];
    DWORD   dwSize;
    HRESULT hErr;

    // -> we are using GUIDs are unique IDs for each directory server config info
    hErr = CoCreateGuid( &nUid );
    if (FAILED( hErr )) return( FALSE );
        CLSIDtochar((const CLSID) nUid, szGUID, sizeof( szGUID ));

    // -> assign the next free section name 
    m_ConfigIO->CreateDirSection( "LDAP--Servers", NULL );
    m_ConfigIO->SetDirDword( "LDAP--Servers", ".Version", (DWORD)MAKELONG(0,1));   // -> MAKELONG( MINOR, MAJOR )
    m_ConfigIO->GetDirDword( "LDAP--Servers", ".NextSection", 0, &next_section );
    do
    {   m_UtilBuf[0] = '\0';
        dwSize       = UTIL_BUF_SIZE;
        wsprintf( szKey, LDAP_SECTION, next_section );
        hErr = m_ConfigIO->GetDirStr( "LDAP--Servers", szKey, "-1", 3,  m_UtilBuf, &dwSize );
        if ( FAILED( hErr ) || lstrcmpi( (LPCTSTR)m_UtilBuf, (LPCTSTR)"-1" ) == 0 ) {
             FoundEmptySection = TRUE;
        }
        else {
             // -> can't find a free section name so give up
             next_section++;
             if (++max_count > 6000) return( FALSE );
        }

    } while( !FoundEmptySection ); 
    m_ConfigIO->SetDirDword(  "LDAP--Servers", ".NextSection", next_section+1 );

    // -> keep the number of entries up to date
    m_ConfigIO->GetDirDword(  "LDAP--Servers", ".ServerCount", 0, &server_count );
    m_ConfigIO->CreateDirKey( "LDAP--Servers", ".ServerCount" );
    m_ConfigIO->SetDirDword(  "LDAP--Servers", ".ServerCount", server_count+1 );

    // -> the section name is the registry value's name and the domain name is its actual value
    m_ConfigIO->CreateDirKey( "LDAP--Servers", szKey );
    hErr = m_ConfigIO->SetDirStr( "LDAP--Servers", szKey, szBuffer );
    if (FAILED( hErr )) return( FALSE );

    // -> we should only need to write the UID into the config info once since it NEVER changes
    m_ConfigIO->CreateDirSection( szKey, NULL );
    m_ConfigIO->SetDirStr( szKey, ".UID", szGUID );

    // -> create and set up a server list of one
    return( NewServerList( szBuffer, szKey, szGUID ));  
}


// 
//
BOOL Config::DeleteAllConfig()
{
    DWORD   server_count;
    HRESULT hErr;

   if (m_ServerList == NULL || m_ServerList->SectionName == NULL) return( FALSE );

    // ->
    hErr = m_ConfigIO->GetDirDword(  "LDAP--Servers", ".ServerCount", 1, &server_count );
    if (FAILED( hErr )) return( FALSE );
    hErr = m_ConfigIO->SetDirDword(  "LDAP--Servers", ".ServerCount", server_count-1 );
    if (FAILED( hErr )) return( FALSE );
    m_ConfigIO->DeleteDirSection( m_ServerList->SectionName );
    m_ConfigIO->DeleteDirKey( "LDAP--Servers", m_ServerList->SectionName );
    m_MapModified     = FALSE;
    m_NetModified     = FALSE;
    m_OptionsModified = FALSE;
    m_LogModified     = FALSE;
    return( TRUE );
}



//****************************************************************
// Member functions to access list of server data
//
//****************************************************************
//
// The caller must not free the "properties" parameter returned.
//
void Config::GetFirstServer( Server_t** properties )
{
    *properties = NULL;
    if (m_ServerList == NULL) return;
        *properties = &m_ServerList[0];
    m_NextServer = 1;
}



// The caller must not free the "properties" parameter returned.
//
void Config::GetNextServer( Server_t** properties )
{
    *properties = NULL;
    if (m_ServerList == NULL || m_NextServer == 0 || m_NextServer >= m_ServerSize) return;
        *properties = &m_ServerList[m_NextServer++];
}



//****************************************************************
// Utility Methods
//
//****************************************************************
//
//   
BOOL Config::NewServerList( LPSTR szServerName,    // in:
                            LPSTR szSectionName,   // in:
                            LPSTR szSectionUID     // in: can be NULL
                          )
{
    UINT length;

    if ((m_ServerList = DEBUG_NEW_NOTHROW Server_t[1]) != 0) {
        m_ServerSize = 1;
        length = (UINT) lstrlen( szServerName );

        if ((m_ServerList->DomainName = DEBUG_NEW_NOTHROW char[length+1] ) != 0) {
            m_ServerList->DomainName[0] = '\0';
            lstrcpy( m_ServerList->DomainName, szServerName );
        }
        else
            m_ServerList->DomainName = NULL;

        length = (UINT) lstrlen( szSectionName );

        if ((m_ServerList->SectionName = DEBUG_NEW_NOTHROW char[length+1] ) != 0) {
            m_ServerList->SectionName[0] = '\0';
            lstrcpy( m_ServerList->SectionName, szSectionName );
        }
        else
            m_ServerList->SectionName = NULL;

        m_ServerList->szUID = NULL;

        if (szSectionUID) {
            length = (UINT) lstrlen( szSectionUID );

            if ((m_ServerList->szUID = DEBUG_NEW_NOTHROW char[length+1] ) != 0) {
                m_ServerList->szUID[0] = '\0';
                lstrcpy( m_ServerList->szUID, szSectionUID );
            }
        }
         
        m_ServerList->UserSrvName     = NULL;
        m_ServerList->Port            = 0;
        m_ServerList->SearchBase      = NULL;
        m_ServerList->LogFile         = NULL;
        m_ServerList->OverWrite       = FALSE;
        m_ServerList->LogFlags        = 0;
        m_ServerList->LogonRequired   = FALSE;
		m_ServerList->SSLRequired	  = FALSE;
        m_ServerList->AccountName     = NULL;
        m_ServerList->AccountPassword = NULL;
        m_ServerList->ServerTimeOut   = 0;
        m_ServerList->ServerHitLimit  = 0;
        m_ServerList->SearchFilter    = NULL;
        m_ServerList->CNSearchFilter  = NULL;

        return( TRUE );
    }

    return( FALSE );
}


// Load a string from configuration data and make a copy of it.  
//
BOOL Config::CopyConfigStr( LPSTR* pString,        // out:
                            LPSTR  szKey,          // in:
                            LPSTR  szUserDefault   // in: can be NULL
                          )
{
    LPSTR   szDefault;
    DWORD   dwSize; 
    HRESULT hErr;

    // -> on entry initialization
    m_UtilBuf[0] = '\0';
    dwSize       = UTIL_BUF_SIZE;
    *pString     = NULL;
    szDefault    = (szUserDefault) ? szUserDefault : "";

    // ->
    hErr = m_ConfigIO->GetDirStr( m_ServerList->SectionName, szKey, szDefault, lstrlen(szDefault)+1, m_UtilBuf, &dwSize );
    if ( !FAILED( hErr ) && dwSize > 0) 
    {
         // -> was a value returned ?
         if (lstrlen( m_UtilBuf ) == 0 && szUserDefault == NULL) { 
             return( TRUE );
         }
         // -> copy the string returned from GetDirStr
         if ((*pString = DEBUG_NEW_NOTHROW char[dwSize+1] ) != 0) { 
              (*pString)[0] = '\0';
                      lstrcpy( (*pString), m_UtilBuf );
              return( TRUE );
         }
    }
    return( FALSE );
}


//
//
void Config::FreeAttributeMap()
{
        NameMap_t* AttribName = m_AttribMap;

        if (m_AttribMap == NULL) return;

        for( DWORD i=0; i < m_MapSize; i++, AttribName++ )
        {
                 if (AttribName->x500Name != NULL) delete [] AttribName->x500Name;
                 if (AttribName->UserName != NULL) delete [] AttribName->UserName;
        }
        delete [] m_AttribMap;
        m_AttribMap = NULL;
        m_MapSize   = 0;
}


// Since we don't keep track of which Attribute Name Map entry has its Key modified 
// what we do is delete the entire Name Map from its store and save it all again at once.
// Thus any and all changes are guaranteed to be done.

void Config::DeleteStoredMap( LPSTR szSection )
{
    NameMap_t* AttribName = m_AttribMap;

    if ( m_AttribMap == NULL || szSection == NULL )
        return;

    for( DWORD i=0; i < m_MapSize; i++, AttribName++ ) {
        if ( AttribName->x500Name )
            m_ConfigIO->DeleteDirKey( szSection, AttribName->x500Name );
    }

    m_ConfigIO->SetDirDword( m_ServerList->SectionName, ".AttributeCount", 0 );
}


//
void Config::FreeServerList()
{
        Server_t* ServerName = m_ServerList;

        if (m_ServerList == NULL) return;

        for( UINT i=0; i < m_ServerSize; i++, ServerName++ )
        {
            if (ServerName->UserSrvName    ) delete [] ServerName->UserSrvName;
            if (ServerName->DomainName     ) delete [] ServerName->DomainName;
            if (ServerName->SearchBase     ) delete [] ServerName->SearchBase;
            if (ServerName->SectionName    ) delete [] ServerName->SectionName;
            if (ServerName->LogFile        ) delete [] ServerName->LogFile;
            if (ServerName->szUID          ) delete [] ServerName->szUID;
            if (ServerName->AccountName    ) delete [] ServerName->AccountName;
            if (ServerName->AccountPassword) delete [] ServerName->AccountPassword;
		    if (ServerName->SearchFilter) delete [] ServerName->SearchFilter;
			if (ServerName->CNSearchFilter) delete [] ServerName->CNSearchFilter;
        }
        delete [] m_ServerList;
        m_ServerList = NULL;
        m_ServerSize = 0;
}


//****************************************************************
// Parse the incoming stream of bytes of the Attribute Name 
// Mapping Configuration data
//
//****************************************************************
//   
// Configuration info uses easy to read strings that indicate the type of the data.
//
DS_TYPE Config::IntoInternalTypeName()
{
    for( UINT i=0; i < CANNED_TYPES; i++ )
        {
         if (lstrcmpi( &m_UtilBuf[m_CurrentByte], ValueTypeNames[i] ) == 0) return( InternalTypes[i] );
    }
        return( DST_BINARY );
}


// F( InternalTypeName ) --> Type Name  OR F( DS_TYPE ) ---> string
//
char* Config::IntoTypeString( DS_TYPE ConfiguredType )  // in:
{
    for( UINT i=0; i < CANNED_TYPES; i++ )
        {
         if (ConfiguredType == InternalTypes[i]) return( ValueTypeNames[i] );
    }
        return( NULL );
}


// Configuration info uses easy to read strings to define user friendly attribute names.
//
DS_FIELD Config::IntoInternalName()
{
    for( UINT i=0; i < CANNED_ATTRIBUTES; i++ )
        {
         if (lstrcmpi( &m_UtilBuf[m_CurrentByte], AttribNames[i] ) == 0) return( InternalNames[i] );
    }
        return( DS_UNKNOWN );
}


// F( InternalName ) --> Attribute Name  OR F( DS_FIELD ) ---> string
//
char* Config::IntoAttribString( DS_FIELD ConfiguredName )  // in:
{
    for( UINT i=0; i < CANNED_ATTRIBUTES; i++ )
        {
         if (ConfiguredName == InternalNames[i]) return( AttribNames[i] );
    }
        return( NULL );
}


// Return an index to the next SP.
// This function assumes a properly built Attribute Name Map String (does not handle bad syntax).
//
int Config::TokenEnd()
{
        BOOL  InQuotes = FALSE;
        BOOL  EscapeOn = FALSE;
        DWORD parse_at;

    // -> ignore leading spaces
        for( ; m_CurrentByte < m_TotalBytes && m_UtilBuf[m_CurrentByte] == ' '; m_CurrentByte++ ) ;
    parse_at = m_CurrentByte;

        // -> 
        for( ; parse_at < m_TotalBytes; parse_at++ )
        {
                     if (m_UtilBuf[parse_at] == '\\'                          ) { EscapeOn = TRUE; continue; }
            else if (m_UtilBuf[parse_at] == '"'  && !InQuotes             )   InQuotes = TRUE;
                else if (m_UtilBuf[parse_at] == '"'  &&  InQuotes && !EscapeOn)   InQuotes = FALSE;
                else if (m_UtilBuf[parse_at] == '\0'                                              )   return( parse_at ); 
                else if (m_UtilBuf[parse_at] == ' '      && !InQuotes             )   return( parse_at );   // -> on the SP
                EscapeOn = FALSE;
        }
        return( -1 );
}


//
//
void Config::DumpNameMap()
{
        NameMap_t* AttribName = m_AttribMap;
        char*      szTemp;

        if (m_LogObj == NULL || m_AttribMap == NULL) return;
        m_LogObj->WriteString( "  " );
        m_LogObj->WriteString( "--- Attribute Name Mapping Table ---" );

        for( DWORD i=0; i < m_MapSize; i++, AttribName++ )
        {
                 wsprintf( m_UtilBuf, "[%d]", i+1 );
                 m_LogObj->WriteString( m_UtilBuf );
                 wsprintf( m_UtilBuf, "X.500 Attribute Name: %s", (AttribName->x500Name) ? AttribName->x500Name : "***" );
                 m_LogObj->WriteString( m_UtilBuf );
                 wsprintf( m_UtilBuf, "Internal Name: %s", (szTemp = IntoAttribString( AttribName->InternalName )) ? szTemp : "***" );
                 m_LogObj->WriteString( m_UtilBuf );
                 if (AttribName->UserName != NULL) {
                     wsprintf( m_UtilBuf, "User Defined Name: %s", AttribName->UserName );
                     m_LogObj->WriteString( m_UtilBuf );
                 }
                 wsprintf( m_UtilBuf, "Attribute Data Type: %s", (szTemp = IntoTypeString( AttribName->DataType )) ? szTemp : "***" );
                 m_LogObj->WriteString( m_UtilBuf );
             m_LogObj->WriteString( "  " );
        }
        m_LogObj->WriteString( "--- Attribute Name Mapping Table ---" );
}


//
//
void Config::DumpServers()
{
        Server_t* ServerName = m_ServerList;

        if (m_LogObj == NULL || m_ServerList == NULL) return;
        m_LogObj->WriteString( "  " );
        m_LogObj->WriteString( "--- Configured LDAP Servers ---" );

        for( UINT i=0; i < m_ServerSize; i++, ServerName++ )
        {
                 wsprintf( m_UtilBuf, "[%d]", i+1 );
                 m_LogObj->WriteString( m_UtilBuf );
                 wsprintf( m_UtilBuf, "Server Name: %s, Port: %d, User's Name: %s", (ServerName->DomainName) ? ServerName->DomainName : "***", ServerName->Port, (ServerName->UserSrvName) ? ServerName->UserSrvName : "***"  );
                 m_LogObj->WriteString( m_UtilBuf );
                 wsprintf( m_UtilBuf, "LDAPDN for Search Base: %s", (ServerName->SearchBase) ? ServerName->SearchBase : "***" );
                 m_LogObj->WriteString( m_UtilBuf );
                 wsprintf( m_UtilBuf, "Configuration Section Name: %s, UID: %s", (ServerName->SectionName) ? ServerName->SectionName : "***", (ServerName->szUID) ? ServerName->szUID : "***" );
                 m_LogObj->WriteString( m_UtilBuf );
                 //wsprintf( m_UtilBuf, "Logon: %s, Account: %s, Password: %s, SSL: %s", (ServerName->LogonRequired) ? "TRUE" : "FALSE", (ServerName->AccountName) ? ServerName->AccountName : "***", (ServerName->AccountPassword) ? ServerName->AccountPassword : "***", (ServerName->SSLRequired) ? "TRUE" : "FALSE" );
                 //m_LogObj->WriteString( m_UtilBuf );
                 wsprintf( m_UtilBuf, "Logon: %s, Account: %s, SSL: %s", (ServerName->LogonRequired) ? "TRUE" : "FALSE", (ServerName->AccountName) ? ServerName->AccountName : "***", (ServerName->SSLRequired) ? "TRUE" : "FALSE" );
                 m_LogObj->WriteString( m_UtilBuf );
                 if (ServerName->LogFile) {
                         wsprintf( m_UtilBuf, "Log File: %s,  OverWrite: %s, LogFlags: %ld", ServerName->LogFile, (ServerName->OverWrite) ? "TRUE" : "FALSE", ServerName->LogFlags );
                     m_LogObj->WriteString( m_UtilBuf );
                 }
             m_LogObj->WriteString( "  " );
        }
        m_LogObj->WriteString( "--- Configured LDAP Servers ---" );
}




//****************************************************************
// Dialog WinProcs
//
//****************************************************************
//
//
BOOL FindFile( HWND   hwnd,        // in:
                           char*  szFileName   // in/out: MUST be at least _MAX_PATH long
                         )
{
        char szFilter[200] = "\0";
        char szTitle[ 200] = "\0";
        UINT length;

        LoadString( g_hInstance, IDS_FINDFILE, szTitle,  sizeof(szTitle )-1 );
        LoadString( g_hInstance, IDS_ALLFILES, szFilter, sizeof(szFilter)-3 );
        length = (UINT) lstrlen( szFilter );
        szFilter[length  ] = '\0';
        szFilter[length+1] = '\0';
        szFileName[0]      = 0;
                
    g_ofn.lStructSize       = sizeof( OPENFILENAME );
    g_ofn.hwndOwner         = hwnd;
    g_ofn.hInstance         = g_hInstance;
    g_ofn.lpstrFilter       = szFilter;
    g_ofn.lpstrCustomFilter = NULL; 
    g_ofn.nMaxCustFilter    = 0;
    g_ofn.nFilterIndex      = 0;
    g_ofn.lpstrFile         = szFileName;
    g_ofn.nMaxFile          = _MAX_PATH;
    g_ofn.lpstrFileTitle    = NULL;
    g_ofn.nMaxFileTitle     = _MAX_PATH;
    g_ofn.lpstrInitialDir   = NULL;
    g_ofn.lpstrTitle        = szTitle;
    g_ofn.Flags             = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_NONETWORKBUTTON;
    g_ofn.nFileOffset       = 0;
    g_ofn.nFileExtension    = 0;
    g_ofn.lpstrDefExt       = "txt";
    g_ofn.lCustData         = 0L;
    g_ofn.lpfnHook          = NULL;
    g_ofn.lpTemplateName    = NULL;

        return( GetOpenFileName( &g_ofn ));
}


//
//
char* KnownTypes( DS_TYPE KnownType,   // in:
                  char*   szBuffer,    // out:
                  UINT    buf_size     // in:
                )
{
    UINT string_id;

        switch( KnownType ) {
        case DST_BINARY:    string_id = IDS_BINARYTYPE; break;
        case DST_GIF:       string_id = IDS_GIFTYPE;    break;
        case DST_JPG:       string_id = IDS_JPEGTYPE;   break;
        case DST_IMAGE:     string_id = IDS_IMAGETYPE;  break;
        case DST_WORDDOC:   string_id = IDS_WORDTYPE;   break;
        case DST_EXCEL:     string_id = IDS_EXCELTYPE;  break;
        case DST_URL:       string_id = IDS_URLTYPE;    break;
//	case DST_NULL:      string_id = IDS_NULLTYPE;   break;
//      case DST_ASCII:     // -> should not be using
        case DST_ASCIIZ:    
    default:            string_id = IDS_TEXTTYPE;   break;
        }
    szBuffer[0] = '\0';
    LoadString( g_hInstance, string_id, (LPTSTR)szBuffer, buf_size );
    return( szBuffer );
}


//
//
void DisplayTime( DWORD nPos, HWND hStatus )
{
     char  szBuffer[100] = {0};
     UINT  time;

     switch( nPos ) {
     case  1:  time = IDS_LDAP30;   break; 
     case  2:  time = IDS_LDAP60;   break;
     case  3:  time = IDS_LDAP90;   break;
     case  4:  time = IDS_LDAP120;  break;
     case  5:  time = IDS_LDAP150;  break;
     case  6:  time = IDS_LDAP180;  break;
     case  7:  time = IDS_LDAP210;  break;
     case  8:  time = IDS_LDAP240;  break;
     case  9:  time = IDS_LDAP270;  break;
     case 10:  time = IDS_LDAP300;  break;
     case 11:  time = IDS_LDAP0;    break;
     default:  return; 
     }
     LoadString( g_hInstance, time, (LPTSTR)szBuffer, 100 );
     SetWindowText( hStatus, (LPCTSTR)szBuffer );
}


//
//
void SetListViewColumn( HWND hwndList, int nColumn, UINT nIDResString, int nWidth )
{
        LV_COLUMN       lvc = {0};
        char buffer[120];

        LoadString( g_hInstance, nIDResString, buffer, sizeof(buffer)-1 );
        lvc.mask         = LVCF_FMT | LVCF_TEXT | LVIF_PARAM | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.fmt          = LVCFMT_LEFT;
    lvc.pszText  = buffer;
        lvc.cx           = nWidth;
        lvc.iSubItem = nColumn;
        ListView_InsertColumn( hwndList, nColumn, &lvc );
}


//
//
void SetListViewContents( HWND     hwndList,     // in:
                                                  int      nRow,                 // in:
                                                  char*    szContents,   // in:
                                                  DS_FIELD InternalName, // in: better be no longer than 16 bit value
                          DS_TYPE  Type,         // in: better be no longer than 16 bit value
                                                  BOOL     new_item              // in: adding a new entry or updating an existing entry
                                                )
{
        LV_ITEM lvi = {0};

        lvi.mask          = LVIF_TEXT  | LVIF_PARAM;
        lvi.iItem         = nRow;
        lvi.iSubItem  = 0;
        lvi.state     = 0;
        lvi.stateMask = 0;
        lvi.pszText   = (LPTSTR)szContents;
    lvi.lParam    = (LPARAM)MAKELONG( (UINT)InternalName, (UINT)Type );

    if ( new_item )
             ListView_InsertItem( hwndList, &lvi );
    else ListView_SetItem( hwndList, &lvi );

}


//
//
BOOL GetListViewContents( HWND       hwndList,       // in:
                          int        nRow,                       // in: 
                          char*      szContents,     // out:
                          int        content_size,   // in:
                          DS_FIELD*  InternalName,   // out:
                          DS_TYPE*   DataType        // out:
                                                )
{
        LV_ITEM lvi = {0};

        lvi.mask           = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem          = nRow;
        lvi.iSubItem   = 0;
        lvi.pszText    = (LPTSTR)szContents;
    lvi.cchTextMax = content_size;

        if ( ListView_GetItem( hwndList, &lvi )) {
         *InternalName = (DS_FIELD) LOWORD((DWORD) lvi.lParam );
         *DataType     = (DS_TYPE ) HIWORD((DWORD) lvi.lParam );
         return( TRUE  );
    }
    else return( FALSE );
}


//
//
BOOL DeleteListViewItem( HWND hwndList )   // in:
{
    int index, nextItem;

    index = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );

    if ( index != -1 ) {
        nextItem = ListView_GetNextItem( hwndList, index, LVNI_ABOVE );

        if ( nextItem == -1 )
            nextItem = ListView_GetNextItem( hwndList, index, LVNI_BELOW );

        if ( nextItem != -1 )
            ListView_SetItemState( hwndList, nextItem,
                                   LVIS_SELECTED  | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

        ListView_DeleteItem( hwndList, index );
        SetFocus( hwndList );
        return( TRUE  );
    }

    else return( FALSE );
}


// Changes to Log and Server List data are NOT flushed out until the config object is deleted.
//
BOOL CALLBACK NetworkDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LPCONFIG       Config     = (LPCONFIG)GetWindowLong( hwndDlg, DWL_USER );
    PROPSHEETPAGE* pPage      = NULL;
    Server_t*      properties = NULL;
    char           szBuffer[500];
    char           szPass[500];
    char           szTemp[10];
    UINT           Port;
    BOOL           LogonEnabled;
	BOOL		   SSLEnabled;

    switch( uMsg ) {
    case WM_INITDIALOG:
	pPage = (PROPSHEETPAGE*)lParam;
	SetWindowLong( hwndDlg, DWL_USER, pPage->lParam );
	Config = (LPCONFIG) pPage->lParam;
	if (Config) 
	{
	    Config->GetServerProperties( &properties );
	    if (properties == NULL) {
			SetDlgItemText( hwndDlg, IDC_HOSTNAME,    (LPCTSTR)"Unknown Name" );
			SetDlgItemText( hwndDlg, IDC_PORTNUMBER,  (LPCTSTR)"389" );
			CheckDlgButton( hwndDlg, IDC_LOGONENABLE, 0 );              
			CheckDlgButton( hwndDlg, IDC_SSLENABLE, 0 );              
			EnableWindow( GetDlgItem(hwndDlg, IDC_ACCOUNT ), FALSE );
			EnableWindow( GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE );
			return( TRUE );
	    }

	    wsprintf( szBuffer, "%d", properties->Port );
	    SetDlgItemText( hwndDlg, IDC_PORTNUMBER,  (LPCTSTR)szBuffer );
	    CheckDlgButton( hwndDlg, IDC_LOGONENABLE, (properties->LogonRequired) ? 1 : 0 );              
		CheckDlgButton( hwndDlg, IDC_SSLENABLE, (properties->SSLRequired) ? 1 : 0 );              
	    EnableWindow( GetDlgItem(hwndDlg, IDC_ACCOUNT ), properties->LogonRequired );
	    EnableWindow( GetDlgItem(hwndDlg, IDC_PASSWORD), properties->LogonRequired );

	    // Eudora's default config supplies some of the more popular public ldap servers.
	    // These are listed by their DomainName, so we init USERSRVNAME to DomainName,
	    // if a user supplied alternative does not exist.
	    if ( properties->UserSrvName )
		SetDlgItemText( hwndDlg, IDC_USERSRVNAME, (LPCTSTR)properties->UserSrvName );
	    else if ( properties->DomainName )
		SetDlgItemText( hwndDlg, IDC_USERSRVNAME, (LPCTSTR)properties->DomainName );

	    if ( properties->DomainName )
		SetDlgItemText( hwndDlg, IDC_HOSTNAME, (LPCTSTR)properties->DomainName );
	    if ( properties->AccountName )
		SetDlgItemText( hwndDlg, IDC_ACCOUNT, (LPCTSTR)properties->AccountName );
	    if ( properties->AccountPassword )
		SetDlgItemText( hwndDlg, IDC_PASSWORD, (LPCTSTR)properties->AccountPassword );
	}

	return( TRUE );

    case WM_COMMAND:
	switch( LOWORD( wParam )) {
	case IDC_LOGONENABLE:
	    LogonEnabled = (IsDlgButtonChecked( hwndDlg, IDC_LOGONENABLE ) == 1);
	    EnableWindow( GetDlgItem(hwndDlg, IDC_ACCOUNT ), LogonEnabled );
	    EnableWindow( GetDlgItem(hwndDlg, IDC_PASSWORD), LogonEnabled );
	    break;
	}
	return( TRUE );


    case WM_NOTIFY:
	switch( ((LPNMHDR) lParam)->code ) {
	case PSN_KILLACTIVE: 
		{
		szBuffer[0] = '\0';
		GetDlgItemText( hwndDlg, IDC_USERSRVNAME, (LPTSTR)szBuffer, 500 );

		int i = 0;
		bool isValid = FALSE;
		while(szBuffer[i] != '\0' && i < 500)
		{
			if(szBuffer[i] != ' ')
			{
				isValid = TRUE;
				break;
			}
          i++;
		};

		if (!isValid)
		{
			LoadString(g_hInstance, IDS_ERR_SERVNAME,(LPTSTR)szBuffer, 100);
			::MessageBox(hwndDlg, szBuffer, "", MB_ICONERROR);
			SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
		}
		return TRUE;
		}
		break;
	case PSN_APPLY:
	    szBuffer[0] = '\0';
	    szTemp[0]   = '\0';
	    GetDlgItemText( hwndDlg, IDC_HOSTNAME,   (LPTSTR)szBuffer, 500 );
	    GetDlgItemText( hwndDlg, IDC_PORTNUMBER, (LPTSTR)szTemp,   10  );
	    Port = (UINT)atoi( szTemp );
	    if (Config) 
	    {
		// -> brand new server to be configured ?
		Config->GetServerProperties( &properties );
		if ( properties == NULL )
			{
			Config->MakeNewServer( szBuffer );
		    Config->LoadServerInfo();
		}
		else Config->ChangeDomainName( szBuffer );
		Config->ChangePort( Port );

		szBuffer[0] = '\0';
		szPass[0]   = '\0';
		GetDlgItemText( hwndDlg, IDC_ACCOUNT,  (LPTSTR)szBuffer, 500 );
		GetDlgItemText( hwndDlg, IDC_PASSWORD, (LPTSTR)szPass,   500 );
		LogonEnabled = (IsDlgButtonChecked( hwndDlg, IDC_LOGONENABLE ) == 1);
		SSLEnabled = (IsDlgButtonChecked( hwndDlg, IDC_LOGONENABLE ) == 1);
		Config->ChangeLogon( LogonEnabled, szBuffer, szPass, SSLEnabled );

		szBuffer[0] = '\0';
		GetDlgItemText( hwndDlg, IDC_USERSRVNAME, (LPTSTR)szBuffer, 500 );
		Config->ChangeUserSrvName( szBuffer );
		Config->StoreNetInfo();
	    }
	    return( TRUE );
	}
	break;
    }
    return( FALSE );
}


// Expects the lParam to be set to either -1 for "Add Attribute" function or >= 0 for "Edit Attribute"
// function.  If >= 0 this value is the index into the List view of the selected item.
//
// ******* NOTE:
// This WinProc allows the user to define his own strings that correspond to any database field name
// (e.g., for "organizationPerson" instead of using the canned DS_NAME he can type in "DOGGIE").
//
BOOL CALLBACK ModifyAttribProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static BOOL    UserDefined;
    HWND           hwndCombo  = GetDlgItem( hwndDlg, IDC_PRESENTEDAS );
    HWND           hwndType   = GetDlgItem( hwndDlg, IDC_DATAFORMAT  );
    ModifyData_t*  change     = (ModifyData_t*)GetWindowLong( hwndDlg, DWL_USER );
    char           szBuffer[200];
    char           szUserDef[200];
    UINT           SelectedName = 1;
    UINT           ComboIndex   = 1;
    DS_TYPE        Type;
    int            index;


    switch( uMsg ) {
    case WM_INITDIALOG: 
    {
        change = (ModifyData_t*)lParam;
        SetWindowLong( hwndDlg, DWL_USER, (LONG)change );
        SendMessage( hwndCombo, CB_INSERTSTRING, 0, (LPARAM)" " );

        for( UINT i=1; i < CANNED_ATTRIBUTES; i++ ) { 
            // -> entry 0 in the combo box is set aside for USERDEFINED items               
            SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)change->Config->KnownFields( InternalNames[i] )); 
        }

        SendMessage( hwndCombo, CB_SETCURSEL, 0, 0 ); 

        // -> we present one, simple string type to the user (the user doesn't care about how the string is terminated)
        for( i=1; i < CANNED_TYPES; i++ ) { 
            SendMessage( hwndType, CB_INSERTSTRING, i-1, (LPARAM)KnownTypes( InternalTypes[i], szBuffer, 200 )); 
        }

        SendMessage( hwndType, CB_SETCURSEL, 0, 0 ); 
        UserDefined = FALSE;
        LoadString( g_hInstance, (change->index == -1) ? IDS_ADDATTRIB : IDS_EDITATTRIB,
                    szBuffer,  sizeof(szBuffer)-1 );

        SetWindowText( hwndDlg, (LPCTSTR)szBuffer ); 

        // -> in edit mode we must set the existing attribute name and its presentation type
        if (change->index >= 0) 
        { 
            szBuffer[0] = '\0';  
            if ( GetListViewContents( change->hwndList, (UINT)change->index, szBuffer,
                                      sizeof(szBuffer), (DS_FIELD*)&SelectedName, &Type )) 
            {
                SetDlgItemText( hwndDlg, IDC_ATTRIBNAME, ( LPCTSTR)szBuffer );

                // -> depends on DS_FIELD order in dsapi.h; 
                if ( SelectedName == (UINT)DS_USERDEFINED ) 
                {
                    UserDefined  = TRUE;
                    szUserDef[0] = '\0';
                    ListView_GetItemText( change->hwndList, change->index, 1, szUserDef, sizeof( szUserDef )); 
                    SendMessage( hwndCombo, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szUserDef );    
                }
                else
                    SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)SelectedName+1, 0 );  
                  
                index = (Type == DST_ASCIIZ || Type == DST_ASCII) ? 0 : ((int)Type)-1;
                SendMessage( hwndType, CB_SETCURSEL, (WPARAM)index, 0 ); 
            }
        }

        SendMessage( hwndCombo, CB_LIMITTEXT, (WPARAM)sizeof( szUserDef ), 0 );    
        return( TRUE );
    }

        
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            // -> user selected from the list box
            if ( IDC_PRESENTEDAS == (int) LOWORD(wParam) ) {
		UserDefined = FALSE;

		// if user has picked "Do not display", we need to gray-out the
		// data-type combo box.
		int curSel = ComboBox_GetCurSel( (HWND)lParam );
		EnableWindow( hwndType, (BOOL)(curSel != DS_NULL) );
	    }

            return( TRUE );
        }

        if (HIWORD(wParam) == CBN_EDITCHANGE) {
            // -> user typed in the edit control
            if ( IDC_PRESENTEDAS == (int) LOWORD(wParam) )
		UserDefined = TRUE;

            return( TRUE );
        }

        switch( LOWORD( wParam )) {
        case IDOK:
            if ( change->index == -1 )    // -> ADD  function, place new entry at end of list view
                index = ListView_GetItemCount( change->hwndList );
            else                          // -> EDIT function, modify selected entry
                index = ListView_GetNextItem( change->hwndList, -1, LVNI_ALL | LVNI_SELECTED );

            // -> read the data from the dialog box
            szBuffer[0] = '\0';
            GetDlgItemText( hwndDlg, IDC_ATTRIBNAME, szBuffer, sizeof( szBuffer ));
            ComboIndex  = (UINT) SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
            ComboIndex  = (ComboIndex > CANNED_ATTRIBUTES || ComboIndex == CB_ERR || UserDefined) ? 0 : ComboIndex;

	    if ( ComboIndex != DS_NULL ) {
		Type = (DS_TYPE) SendMessage( hwndType, CB_GETCURSEL, 0, 0 );
		Type = (Type == CB_ERR) ? DST_ASCIIZ : static_cast<DS_TYPE>( ((int)Type)+1 );
	    }
	    else
		Type = DST_NULL;

            // -> it appears that when we set the main label of an item it deletes the subitem label
            szUserDef[0] = '\0';
            ListView_GetItemText( change->hwndList, index, 1, szUserDef, 200 ); 

            // ->
            SetListViewContents( change->hwndList, index, szBuffer, InternalNames[ComboIndex],
                                 Type, (change->index == -1));

            if ( InternalNames[ComboIndex] == DS_USERDEFINED ) {
                SendMessage( hwndCombo, WM_GETTEXT, (WPARAM)sizeof( szUserDef ), (LPARAM)szUserDef );
                ListView_SetItemText( change->hwndList, index, 1, szUserDef );
            }
            else
                ListView_SetItemText( change->hwndList, index, 1,
                                      change->Config->KnownFields( InternalNames[ComboIndex] ));

            EndDialog( hwndDlg, TRUE  );
            return( TRUE );

        case IDCANCEL:
            EndDialog( hwndDlg, FALSE );
            return( TRUE );
        }

        break;
    }

    return( FALSE );
}


// Changes to the Attribute Name Map table are made immedately due to the way we delete old key names.
//
BOOL CALLBACK AttribDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static BOOL          data_modified;
    static ModifyData_t  change;

    LPCONFIG       Config    = (LPCONFIG)GetWindowLong( hwndDlg, DWL_USER );
    PROPSHEETPAGE* pPage     = NULL;
    HWND           hwndList  = GetDlgItem( hwndDlg, IDC_ATTRIBLIST );
    NameMap_t*     attribute = NULL;
    LRESULT        lResult   = 0;
    LV_KEYDOWN*    keyDown;
    UINT           i;
    int            index;

    // nice coding style, eh? holy cow.
    if ( Config && Config->m_pListViewEx &&
         Config->m_pListViewEx->HandleMessage( hwndDlg, uMsg, wParam, lParam, lResult ) ) {

        return( lResult );
    }

    switch( uMsg ) {
    case WM_INITDIALOG:
        pPage  = (PROPSHEETPAGE*)lParam;
        SetWindowLong( hwndDlg, DWL_USER, pPage->lParam );
        Config = (LPCONFIG) pPage->lParam;
        data_modified = FALSE;

        if (Config) {
            Config->m_pListViewEx = DEBUG_NEW_NOTHROW CListViewEx( hwndList );
        }

        SetListViewColumn( hwndList, 0, IDS_ATTRIBNAME,  150 );
        SetListViewColumn( hwndList, 1, IDS_PRESENTNAME, 150 );

        if (Config) {
            i = 0;
            Config->FirstAttribute( &attribute );

            while( attribute != NULL ) {
                SetListViewContents(  hwndList, i,
                                      (attribute->x500Name) ? attribute->x500Name : "",
                                      attribute->InternalName, attribute->DataType, TRUE );

                if ( attribute->InternalName == DS_USERDEFINED ) {
                    ListView_SetItemText( hwndList, i, 1,
                                          (attribute->UserName) ? attribute->UserName : "" );
                }
                else ListView_SetItemText( hwndList, i, 1,
                                           Config->KnownFields( attribute->InternalName ) );

                Config->NextAttribute( &attribute );
                i++;
            }

            ListView_SetItemState( hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED,
                                   LVIS_SELECTED | LVIS_FOCUSED );

            Config->m_pListViewEx->AutoResizeLastColumn();

            {
            // set the "ReturnAll" checkbox
            WPARAM wCheckState = Config->GetReturnAll() ? BST_UNCHECKED : BST_CHECKED;
            HWND hCkbx = GetDlgItem( hwndDlg, IDC_LISTEDATTRS_ONLY );
            SendMessage( hCkbx, BM_SETCHECK, wCheckState, 0 );
            }
        }

        return( TRUE );

    case WM_DESTROY:
        if (Config && Config->m_pListViewEx) {
            delete Config->m_pListViewEx;
            Config->m_pListViewEx = NULL;
        }

        break;


    case WM_NOTIFY:
        switch( ((LPNMHDR) lParam)->code ) {
        case LVN_KEYDOWN:
            keyDown = (LV_KEYDOWN*) lParam;

            if (keyDown->hdr.hwndFrom == hwndList && keyDown->wVKey == VK_DELETE)
                data_modified = DeleteListViewItem( hwndList );

//            if ( data_modified )
//                Config->m_MapSize--;

            return( TRUE );

        case NM_DBLCLK:
            // -> double click is the same as an edit
            if (((LPNMHDR) lParam)->hwndFrom == hwndList) {
                index = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );

                if (index >= 0) {
                    change.hwndList = hwndList;
                    change.index    = index;
                    change.Config   = Config;
                    data_modified = DialogBoxParam( g_hInstance,
                                                    MAKEINTRESOURCE(IDD_EDITATTRIB),
                                                    hwndDlg,
                                                    (DLGPROC)ModifyAttribProc,
                                                    (LPARAM)&change );
                }
            }

            return( TRUE );

        case PSN_APPLY:
        {
            HWND hCkbx = GetDlgItem( hwndDlg, IDC_LISTEDATTRS_ONLY );
            WPARAM wState = SendMessage( hCkbx, BM_GETCHECK, 0, 0 );
            Config->SetReturnAll( !((BOOL)wState) );

            if ( data_modified ) {
                // -> flush all changes out NOW since "ChangeAttribMap" has already removed all keys
                Config->ChangeAttribMap( hwndList );
                Config->StoreAttribMap();
            }
        }

            return( TRUE );
        }

        break;

    case WM_COMMAND:
        switch( LOWORD( wParam )) {
        case IDC_EDITATTRIB:
            index = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );

            if (index >= 0) {
                change.hwndList = hwndList;
                change.index    = index;
                change.Config   = Config;
                data_modified = DialogBoxParam( g_hInstance,
                                                MAKEINTRESOURCE(IDD_EDITATTRIB),
                                                hwndDlg,
                                                (DLGPROC)ModifyAttribProc,
                                                (LPARAM)&change );
            }

            return( TRUE );

        case IDC_ADDATTRIB:
            change.hwndList = hwndList;
            change.index    = -1;
            change.Config   = Config;
            data_modified = DialogBoxParam( g_hInstance,
                                            MAKEINTRESOURCE(IDD_EDITATTRIB),
                                            hwndDlg,
                                            (DLGPROC)ModifyAttribProc,
                                            (LPARAM)&change );
            return( TRUE );

        case IDC_REMOVEATTRIB:
            data_modified = DeleteListViewItem( hwndList );

//            if ( data_modified )
//                Config->m_MapSize--;

            return( TRUE );
        }

        break;
    }

    return( FALSE );
}


// 
//
BOOL CALLBACK OptionsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam )
{
    LPCONFIG       Config     = (LPCONFIG)GetWindowLong( hwndDlg, DWL_USER );
    HWND           hwndSlider = GetDlgItem( hwndDlg, IDC_TIMEOUT  );
    HWND           hwndSpin   = GetDlgItem( hwndDlg, IDC_MAXMATCH );
    PROPSHEETPAGE* pPage      = NULL;
    Server_t*      properties = NULL;
    BOOL           MaxMatchEnabled;
    char           szLDAPDN[2000];
    int            nScrollCode;
    DWORD          dwPos;
    DWORD          dwSpinner;


    switch( uMsg ) {
    case WM_INITDIALOG:
        pPage  = (PROPSHEETPAGE*)lParam;
        SetWindowLong( hwndDlg, DWL_USER, pPage->lParam );
        Config = (LPCONFIG) pPage->lParam;

        // -> default values
        SetDlgItemText( hwndDlg, IDC_SEARCHBASE, "c=US" );
        SetDlgItemText( hwndDlg, IDC_SEARCHFILTER, "(cn=*^0*)" );
        SendMessage( hwndSlider, TBM_SETRANGE, (WPARAM)TRUE,
                     (LPARAM)MAKELONG(1,11));
        SendMessage( hwndSlider, TBM_SETPOS,   (WPARAM)TRUE, (LPARAM)11 );
        DisplayTime( 11, GetDlgItem( hwndDlg, IDC_TIMEVALUE ));
        SendMessage( hwndSpin, UDM_SETBUDDY,
                     (WPARAM)GetDlgItem(hwndDlg, IDC_MAXMATCHVALUE), 0 );
        SendMessage( hwndSpin, UDM_SETBASE,  10, 0 );
        SendMessage( hwndSpin, UDM_SETRANGE, 0, (LPARAM)MAKELONG(20000,0));
        SendMessage( hwndSpin, UDM_SETPOS,   0, (LPARAM)MAKELONG(1000, 0));
        SendMessage( GetDlgItem(hwndDlg, IDC_MAXMATCHVALUE),
                     EM_SETLIMITTEXT, 5, 0 );
        CheckDlgButton( hwndDlg, IDC_NOMATCHLIMIT, 1 );              
        EnableWindow( GetDlgItem(hwndDlg, IDC_MAXMATCH     ), FALSE );
        EnableWindow( GetDlgItem(hwndDlg, IDC_MAXMATCHVALUE), FALSE );

        if (Config) {
            Config->GetServerProperties( &properties );

            if (properties == NULL)
                return( TRUE );

            if ( properties->SearchBase )
                SetDlgItemText( hwndDlg,
                                IDC_SEARCHBASE, properties->SearchBase );

            if ( properties->SearchFilter ) {
                SetDlgItemText( hwndDlg,
                                IDC_SEARCHFILTER, properties->SearchFilter );
            }

            if ( properties->CNSearchFilter ) {
                SetDlgItemText( hwndDlg,
                                IDC_CN_SEARCHFILTER,
                                properties->CNSearchFilter );
            }

            dwPos = (properties->ServerTimeOut == 0) ? 11
                : properties->ServerTimeOut / 30;

            SendMessage( hwndSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)dwPos );
            DisplayTime( dwPos, GetDlgItem( hwndDlg, IDC_TIMEVALUE ));

            if ( properties->ServerHitLimit != 0 ) {
                CheckDlgButton( hwndDlg, IDC_NOMATCHLIMIT, 0 );              
                EnableWindow( GetDlgItem(hwndDlg, IDC_MAXMATCH     ), TRUE );
                EnableWindow( GetDlgItem(hwndDlg, IDC_MAXMATCHVALUE), TRUE );
                SendMessage( hwndSpin, UDM_SETPOS, 0,
                             (LPARAM)properties->ServerHitLimit );
            }
        }

        return( TRUE );


    case WM_HSCROLL:
        if (hwndSlider == (HWND) lParam) {
            nScrollCode = (int) LOWORD(wParam);
            if (nScrollCode == SB_ENDSCROLL || nScrollCode == TB_ENDTRACK)
                return( TRUE );

            dwPos = SendMessage( hwndSlider, TBM_GETPOS, 0,0 ); 
            DisplayTime( dwPos, GetDlgItem( hwndDlg, IDC_TIMEVALUE ));
        }

        return( TRUE );


    case WM_COMMAND:
        switch( LOWORD( wParam )) {
        case IDC_NOMATCHLIMIT:

            // -> if the "No Limit" check box is empty, then the spin
            // control is enabled

            MaxMatchEnabled =
                (IsDlgButtonChecked( hwndDlg, IDC_NOMATCHLIMIT ) == 0);

            EnableWindow( GetDlgItem(hwndDlg, IDC_MAXMATCH), MaxMatchEnabled );
            EnableWindow( GetDlgItem(hwndDlg,
                                     IDC_MAXMATCHVALUE), MaxMatchEnabled );
            break;
        }

        return( TRUE );


    case WM_NOTIFY:
        switch( ((LPNMHDR) lParam)->code ) {
        case PSN_APPLY:
            {
            char szSF[256] = "";
            char szCNSF[256] = "";

            szLDAPDN[0] = '\0';
            GetDlgItemText( hwndDlg, IDC_SEARCHBASE, szLDAPDN,
                            sizeof( szLDAPDN ));

            GetDlgItemText( hwndDlg, IDC_SEARCHFILTER, szSF, sizeof(szSF) );
            GetDlgItemText( hwndDlg, IDC_CN_SEARCHFILTER, szCNSF,
                            sizeof(szCNSF) );

            dwPos = SendMessage( hwndSlider, TBM_GETPOS, 0,0 );
            dwPos = (dwPos == 11) ? 0 : dwPos * 30; 

            // -> if the "No Limit" check box is checked then we store
            // zero as LDAP expects

            if ( IsDlgButtonChecked( hwndDlg, IDC_NOMATCHLIMIT ) == 0 )
                dwSpinner = SendMessage( hwndSpin, UDM_GETPOS, 0,0 );
            else dwSpinner = 0;

            Config->ChangeOptions( szLDAPDN, szSF, szCNSF,
                                   dwPos, dwSpinner, (HIWORD(dwSpinner) == 0));
            Config->StoreOptionInfo();
            return( TRUE );
            }
        }

        break;
    }

    return( FALSE );
}


// Changes to Log and Server List data are NOT flushed out until the config object is deleted.
// NOTE: this MUST be the last dialog in the TAB sequence!
//
BOOL CALLBACK LogDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LPCONFIG       Config     = (LPCONFIG)GetWindowLong( hwndDlg, DWL_USER );
    PROPSHEETPAGE* pPage      = NULL;
        Server_t*      properties = NULL;
        char           szFileName[_MAX_PATH];
    BOOL           OverWrite;
    DWORD          flags;


        switch( uMsg ) {
    case WM_INITDIALOG:
                 pPage = (PROPSHEETPAGE*)lParam;
                 SetWindowLong( hwndDlg, DWL_USER, pPage->lParam );
         Config = (LPCONFIG) pPage->lParam;
                 if (Config) 
                 {
                         Config->GetServerProperties( &properties );
                         if (properties == NULL) return( TRUE );
                         if (properties->LogFile) SetDlgItemText( hwndDlg, IDC_LOGFILE, (LPCTSTR)properties->LogFile );
                         CheckDlgButton( hwndDlg, IDC_OVERWRITE,   (UINT) properties->OverWrite );
                         CheckDlgButton( hwndDlg, IDC_LOGPROTOCOL, (UINT)(properties->LogFlags & LOG_PROTOCOL));
                         CheckDlgButton( hwndDlg, IDC_LOGCONFIG,   (UINT)(properties->LogFlags & LOG_CONFIG  ));              
                 }
                 return( TRUE );


    case WM_NOTIFY:
                 switch( ((LPNMHDR) lParam)->code ) {
         case PSN_APPLY:
              szFileName[0] = '\0';
              flags = 0;
              GetDlgItemText( hwndDlg, IDC_LOGFILE, szFileName, sizeof( szFileName ));
              OverWrite = (IsDlgButtonChecked( hwndDlg, IDC_OVERWRITE ) == 1);
              flags |= (IsDlgButtonChecked( hwndDlg, IDC_LOGCONFIG   ) == 1) ? LOG_CONFIG   : 0; 
              flags |= (IsDlgButtonChecked( hwndDlg, IDC_LOGPROTOCOL ) == 1) ? LOG_PROTOCOL : 0;
              Config->ChangeLogInfo( szFileName, OverWrite, flags );
              Config->StoreLogInfo();
              return( TRUE );
                 }
                 break;


    case WM_COMMAND:
                 switch( LOWORD( wParam )) {
                 case IDC_FINDLOGFILE:
                          if (FindFile( hwndDlg, szFileName )) SetDlgItemText( hwndDlg, IDC_LOGFILE, (LPCTSTR)szFileName );
                      return( TRUE );
                 }
                 break;
        }
        return( FALSE );
}


//
//
BOOL Config::MakePropPages( HPROPSHEETPAGE* paHPropPages )
{
        PROPSHEETPAGE psp;

        psp.dwSize              = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_LDAP_NETWORKPAGE);
        psp.pfnDlgProc  = (DLGPROC)NetworkDlgProc;
        psp.lParam              = (LONG)this;
        paHPropPages[0] = CreatePropertySheetPage( &psp );
    if (paHPropPages[0] == NULL) return( FALSE );

        psp.dwSize              = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_LDAP_ATTRIBPAGE);
        psp.pfnDlgProc  = (DLGPROC)AttribDlgProc;
        psp.lParam              = (LONG)this;
        paHPropPages[1] = CreatePropertySheetPage( &psp );
    if (paHPropPages[1] == NULL) return( FALSE );

        psp.dwSize              = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_SEARCHOPTIONS);
        psp.pfnDlgProc  = (DLGPROC)OptionsDlgProc;
        psp.lParam              = (LONG)this;
        paHPropPages[2] = CreatePropertySheetPage( &psp );
    if (paHPropPages[2] == NULL) return( FALSE );

        psp.dwSize              = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_LDAP_LOGPAGE);
        psp.pfnDlgProc  = (DLGPROC)LogDlgProc;
        psp.lParam              = (LONG)this;
        paHPropPages[3] = CreatePropertySheetPage( &psp );
    return( paHPropPages[3] != NULL );
}

