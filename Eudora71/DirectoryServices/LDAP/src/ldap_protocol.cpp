// LDAP Directory Services 
//
// File:     ldapobj.cpp
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 06/17/1997 Qualcomm Inc.  All Rights Reserved.
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
// This object provides the standard protocol engine interface for all directory
// services.  It makes use of the ILDAPV2 object to talk to an LDAP server.
//
#define  INITGUID
#pragma warning(disable : 4514)
#include <afx.h>
#include <objbase.h>
#include "DebugNewHelpers.h"
#include "standard.h"
#include "pldap.h"
#include "EnumConfig.h"
#include "ServConfig.h"
#include "config.h"
#include "records.h"
#include "log.h"
#include "resource.h"
#include "readme.h"

extern HINSTANCE g_hInstance;
extern DWORD     g_dwComponentCount;



//****************************************************************
// Implementation of IProtocol Methods
//
//****************************************************************
//
//
LDAP::LDAP() : m_Ref( 1 ),
			   m_pUnkCaller( NULL ),
               m_hProtocolImage( NULL ),
			   m_LogObj( NULL ),
			   m_ConfigObj( NULL ),
			   m_ConfigIO( NULL ),
			   m_Servers( NULL )
{
    HRESULT hErr;

    g_dwComponentCount++;
    m_hProtocolImage = LoadBitmap( g_hInstance, MAKEINTRESOURCE( IDB_LDAPBITMAP )); 

//  hErr = CoCreateInstance( IID_IDirConfig,  NULL, CLSCTX_INPROC_SERVER, IID_IDirConfig, (LPVOID*) &m_ConfigIO );
//  if (FAILED( hErr )) return;

    hErr = CoCreateInstance( CLSID_IListMan, NULL, CLSCTX_INPROC_SERVER, IID_IListMan,  (LPVOID*) &m_Servers );
    if (FAILED(  hErr )) return;
    m_Servers->Initialize( ListFreeCB, NULL );
} 


// Free all allocated resources
//
LDAP::~LDAP()
{
	if (m_Servers  ) m_Servers->Release();

    // -> delete support objects last
	if (m_ConfigObj) delete m_ConfigObj;
	if (m_ConfigIO ) m_ConfigIO->Release();
	if (m_LogObj   ) delete m_LogObj; 

    if (m_hProtocolImage) ::DeleteObject( m_hProtocolImage );
    g_dwComponentCount--;
}


//
HRESULT	__stdcall LDAP::QueryInterface( REFIID refid, LPVOID* lpObj )
{
         if ( refid == IID_IUnknown || refid == IID_IDSPProtocol || refid == IID_LDAP ) {
              *lpObj = static_cast<LPLDAP>(this);
    }
    else {
              *lpObj = NULL;
              return( E_NOINTERFACE );
    }    
    reinterpret_cast<IUnknown*>(*lpObj)->AddRef();
    return( S_OK );
}

//
ULONG __stdcall LDAP::AddRef()
{
    return( ++m_Ref );
}

//
ULONG __stdcall LDAP::Release()
{
    ULONG temp_count; 

    temp_count = --m_Ref;
    if (m_Ref == 0) delete this;
    return( temp_count );
}


// This will change.  The config object will only "go get" config info via the new
// IConfig interface instead of EVERY time one of these objects is created.
//
HRESULT	__stdcall LDAP::Initialize( IUnknown* pUnkCaller )   // in:
{
    Server_t* properties = NULL;
    LPVOID    lpObj;
    HRESULT   hErr;

    // -> get the configuration read/write object from our parent
   m_pUnkCaller = pUnkCaller; 
    if (m_pUnkCaller == NULL) return( E_INVALIDARG );
    hErr = m_pUnkCaller->QueryInterface( IID_IDirConfig, (LPVOID*) &m_ConfigIO );
    if ( FAILED( hErr )) return( E_INVALIDARG );

	// -> load all configured server data into a list of IDSPConfig objects
    m_ConfigObj = DEBUG_NEW_NOTHROW Config( m_ConfigIO, NULL, NULL );
    if (m_ConfigObj == 0) return( E_FAIL );

	m_ConfigObj->ReadPreDefinedConfig();

	m_ConfigObj->LoadServerList();
    m_ConfigObj->GetFirstServer( &properties );
    while( properties != NULL )
    {
           if (properties->DomainName && properties->SectionName) {
               NewConfigObj( &lpObj, properties->DomainName, properties->SectionName );
           }
           m_ConfigObj->GetNextServer( &properties );
    }
    return( S_OK );
}


//
//
HRESULT	__stdcall LDAP::CreateObject( REFIID riid, LPVOID* ppv )
{
    HRESULT hErr;

    *ppv = NULL;

         if (riid == IID_IDSPConfig) {
         return( NewConfigObj( ppv, NULL, NULL ));
    }
    else if (riid == IID_LDAPV2) {
         // -> for someone that wants to write directly to the LDAP V2 COM object
         hErr = CoCreateInstance( CLSID_ILdapv2, NULL, CLSCTX_INPROC_SERVER, IID_LDAPV2, ppv );
         return( (FAILED( hErr )) ? hErr : S_OK ); 
    }
    return( CLASS_E_CLASSNOTAVAILABLE );
}


// For a IDSPConfig object, DeleteObject, means remove all configuration data from
// its persistent store.  Plus make sure it is not marked dirty any more.
//
HRESULT	__stdcall LDAP::DeleteObject( IUnknown* punkObject )  // in:
{
    LPIEXISTCFG lpObj = NULL;
    HRESULT     hErr;

    // -> the "QueryInterface" call does an AddRef on the IID_ExistConfig so we must do a Release after we are done with that COM ptr
    if (punkObject == NULL) return( E_INVALIDARG );
    hErr = punkObject->QueryInterface( IID_ExistConfig, (LPVOID*) &lpObj );
    if ( FAILED( hErr )) return( E_INVALIDARG );

    hErr = lpObj->DeleteConfiguration();
    lpObj->Release();
    if (FAILED( hErr )) return( E_FAIL );
    Free_ExistingConfig( punkObject );
    return( S_OK );
}


//
//
HRESULT	__stdcall LDAP::EnumDatabases( IDSPEnumConfig** ppenum )  // out:
{
    *ppenum = NULL;
    if (m_Servers == NULL) return( E_FAIL );

    *ppenum = DEBUG_NEW_NOTHROW CEnumConfig( m_Servers );    

	return((*ppenum == 0) ? E_FAIL : S_OK );
}


//
HRESULT	__stdcall LDAP::GetProtocolName( LPSTR lpszBuffer,   // out:
									     UINT  nBufferSize   // in:
									   )
{
    lstrcpyn( lpszBuffer, "LDAP", (nBufferSize < 5) ? nBufferSize : 5 );
    return( S_OK );
}


//
//
HBITMAP	__stdcall LDAP::GetProtocolImage( int )  // in:
{
    return( m_hProtocolImage );
}



// Our LDAP Protocol Engine does support Writes, but for now we have not implemented the
// COM interface to make it accessible.
//
DWORD	__stdcall LDAP::GetProtocolFlags()
{
    return( PROTO_MODSRCHBASE /* | PROTO_WRITEABLE */ );
}



//****************************************************************
// Private Internal Helper Methods
//
//****************************************************************
//
//
HRESULT __stdcall LDAP::NewConfigObj( LPVOID* ppv,			// in:
                                      LPSTR   ServerName,   // in: can be NULL
                                      LPSTR   ConfigKey     // in: can be NULL 
									)
{
    LPEXISTING lpObj = NULL;
    DWORD      count;

    // -> Create an IPConfig object inside a structure that can be placed on a list object
    if ((lpObj = DEBUG_NEW_NOTHROW Existing_t) == 0) return( E_OUTOFMEMORY );
    lpObj->pPrev     = NULL;
    lpObj->pNext     = NULL;
    lpObj->ConfigObj = NULL;
    lpObj->ConfigObj = DEBUG_NEW_NOTHROW CServerConfig( m_hProtocolImage );
    if (lpObj->ConfigObj == 0) { delete lpObj; return( E_OUTOFMEMORY ); }

    // -> newly created empty config objects must also be put on our list so the next enum call will display it
    m_Servers->Attach((LPVOID) lpObj );						               
    count = m_Servers->Count();
    lpObj->ConfigObj->Initialize( m_ConfigIO, ServerName, ConfigKey );    
    *ppv = (LPVOID) lpObj->ConfigObj;
    return( S_OK );
}


// Is the object in a proper state for exectution ?
//
BOOL LDAP::IsObjectValid()
{
    return( m_Servers != NULL );
}


//
//
BOOL LDAP::Free_ExistingConfig( IUnknown* punkObject )  // in:
{
    LPEXISTING pitem      = NULL;
    DWORD      item_count = 0;

    // -> get start of search
    if (m_Servers == NULL) return( FALSE );
    pitem      = (LPEXISTING) m_Servers->Head();
    item_count = m_Servers->Count();

    // ->
    for( UINT i=0; i < item_count && pitem; i++ )
    {
         if (reinterpret_cast<IUnknown*>(pitem->ConfigObj) == punkObject) {
             m_Servers->DeleteItem((LPVOID) pitem );
             return( TRUE );
         }
         pitem = (LPEXISTING) ILIST_ITEMNEXT( pitem );
    }
    return( FALSE );
   
}


// List Object Callback to free a single list item.
//
void LDAP::ListFreeCB( LPITEM  pItem,      // in: 
                       LPVOID     // in:
                     )
{
    LPEXISTING item = (LPEXISTING) pItem;

    if (item) 
    {
        if (item->ConfigObj) item->ConfigObj->Release();
        delete item;
    }   
}




