// LDAP Directory Services 
//
// File: 			ServConfig.cpp
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
// Description:		
// Implementation of an IDSPConfig COM object.  This is done by mainly forwarding
// requests to our build in C++ configuration object that reads/writes all
// configuration data.
//
#pragma warning(disable : 4514)
#include <afx.h>
#include <objbase.h>
#include "DebugNewHelpers.h"
#include <dspapi.h>
#include "standard.h"
#include "ServConfig.h"
#include "query.h"

extern DWORD g_dwComponentCount;


//****************************************************************
// Implementation of IDSPConfig Methods
//
//****************************************************************
//
//
CServerConfig::CServerConfig( HBITMAP hImage ) : m_Ref( 1 ),
												 m_hProtocolImage( hImage ),
												 m_LogObj( NULL ),
												 m_ConfigIO( NULL ),
												 m_ConfigObj( NULL )
{
    g_dwComponentCount++;

} 


// Free all allocated resources
//
CServerConfig::~CServerConfig()
{
    if (m_ConfigObj) delete m_ConfigObj;
    if (m_LogObj   ) delete m_LogObj;
    g_dwComponentCount--;
    // -> don't delete m_ConfigIO since we are sharing this object with others
}


//
//
HRESULT	__stdcall CServerConfig::QueryInterface( REFIID iid, LPVOID* ppv)
{
	     if (iid == IID_IUnknown) {
		     *ppv = static_cast<IDSPConfig*>(this);
    }
	else if (iid == IID_ExistConfig) {
		     *ppv = static_cast<LPIEXISTCFG>(this);
    }
	else {
		     *ppv = NULL;
		     return( E_NOINTERFACE );
	}	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return( S_OK );
}


//
ULONG __stdcall CServerConfig::AddRef()
{
	return( InterlockedIncrement( &m_Ref ));
}


//
ULONG __stdcall CServerConfig::Release()
{
	if (InterlockedDecrement( &m_Ref ) == 0) {
		delete this;
		return( 0 );
	}
	return( m_Ref );
}


//
//
HRESULT __stdcall CServerConfig::CreateObject( REFIID riid,  // in:
                                               LPVOID *ppv   // out:
											 )
{
    BOLdapQuery* pObj = NULL;

    *ppv = NULL;

    if ( riid == IID_IDSPQuery ) 
    {
         pObj = DEBUG_NEW_NOTHROW BOLdapQuery( static_cast<IDSPConfig*>(this) );
         if (pObj == 0) return( E_OUTOFMEMORY );

		 //         if (!pObj->IsObjectValid()) { delete pObj; return( E_FAIL ); }
         *ppv = pObj;
         return( S_OK );
    }
    return( CLASS_E_CLASSNOTAVAILABLE );
}


// For now this "Name" is just the server's domain name.  
// Letter we can add a description screen.
//
HRESULT	__stdcall CServerConfig::GetName( LPSTR lpszBuffer,   // out:
										  UINT  nBufferSize   // in:
										)
{
	Server_t* properties = NULL;
    UINT      length;

    // ->
    if (m_ConfigObj == NULL) return( E_FAIL );    
    m_ConfigObj->GetServerProperties( &properties );
    if (properties == NULL) return( E_FAIL );

    // ->
    if ( properties->UserSrvName != NULL ) 
    {
         length = (UINT) lstrlen( properties->UserSrvName )+1;
         lstrcpyn( lpszBuffer, properties->UserSrvName, (nBufferSize < length) ? nBufferSize : length );
         return( S_OK   );
    }
    else if ( properties->DomainName != NULL ) 
    {
         length = (UINT) lstrlen( properties->DomainName )+1;
         lstrcpyn( lpszBuffer, properties->DomainName, (nBufferSize < length) ? nBufferSize : length );
         return( S_OK   );
    }
    else return( E_FAIL );
}


//
//
HBITMAP __stdcall CServerConfig::GetImage( int )
{
    return( NULL );
}


// The caller must copy the string returned if he wants to keep it around for an unlimited amount of time.
//
LPSTR __stdcall CServerConfig::GetID()
{
	Server_t* properties = NULL;

    if (m_ConfigObj == NULL) return( NULL );
    
    m_ConfigObj->GetServerProperties( &properties );
    return((properties && properties->szUID) ? properties->szUID : NULL );
}


//
//
HBITMAP __stdcall CServerConfig::GetProtocolImage( int )
{
    return( m_hProtocolImage );
}

//
//
HRESULT	__stdcall CServerConfig::GetProtocolName( LPSTR lpszBuffer,    // out:
												  UINT  nBufferSize    // in:
											    )
{
    lstrcpyn( lpszBuffer, "LDAP", (nBufferSize < 5) ? nBufferSize : 5 );
    return( S_OK );
}


//
//
DWORD __stdcall CServerConfig::GetProtocolFlags()
{
    return( PROTO_MODSRCHBASE /* | PROTO_WRITEABLE */ );
}


//
//
int	__stdcall CServerConfig::GetPropPageCount()
{
    return( 4 );
}


//
//
HRESULT	__stdcall CServerConfig::GetPropPages( HPROPSHEETPAGE *pPropPages )  // out:
{
    if (!IsObjectValid()) return( E_FAIL );
    return((m_ConfigObj->MakePropPages( pPropPages )) ? S_OK : E_FAIL );
}



//****************************************************************
// Implementation of IExistingConfig Methods
//
//****************************************************************
//
//
// Here we use our C++ configuration object to hold a pre-existing server's data.
//
HRESULT __stdcall CServerConfig::Initialize( IDirConfig* ConfigIO,    // in:
                                             LPSTR       ServerName,  // in:
                                             LPSTR       ConfigKey    // in:
                                           )
{
    Server_t* properties = NULL;

    if (ConfigIO == NULL) return( E_FAIL );
    m_ConfigIO  = ConfigIO;
    m_ConfigObj = DEBUG_NEW_NOTHROW Config( m_ConfigIO, ServerName, ConfigKey );
    if (m_ConfigObj == 0) return( E_FAIL );

    m_ConfigObj->LoadServerInfo();
	m_ConfigObj->GetServerProperties( &properties );
    if (properties != NULL && properties->LogFile != NULL) {
        m_LogObj = DEBUG_NEW_NOTHROW Log( properties->LogFile, properties->OverWrite );
    }
    if (m_LogObj && m_ConfigObj->IsLogEnabled(LOG_CONFIG)) {
        m_ConfigObj->SetLog((LPLOG) m_LogObj );
        m_ConfigObj->DumpServers();
        m_ConfigObj->DumpNameMap();
    }
    return( S_OK );
}


// For IDSPConfig objects only, we remove all configuration data from its persistent storage
// and make sure the object is marked as not dirty.
// The caller MUST still call Release on the object to free it.
//
HRESULT __stdcall CServerConfig::DeleteConfiguration()
{
    if (!IsObjectValid()) return( E_FAIL );
    // -> if this fails it means that the data is just not on the disk yet
    m_ConfigObj->DeleteAllConfig();
    return( S_OK );
}


// Provide access to configuration data stored in our ConfigObj C++ object.
//
HRESULT __stdcall CServerConfig::GetServerProperties( Server_t** properties )  // out:
{
    *properties = NULL;
    if (!IsObjectValid()) return( E_FAIL );

    m_ConfigObj->GetServerProperties( properties );
    return((*properties == NULL) ? E_FAIL : S_OK );
}


// Simply forward to our handly C++ config object.
//
HRESULT __stdcall CServerConfig::MapAttribName( LPSTR     szName,          // in:
                                                DS_FIELD* internal_name,   // out:
                                                DS_TYPE*  data_type,       // out:
                                                LPSTR*    szUserDefined    // out:
                                              )
{
    BOOL bResult = m_ConfigObj->MapAttribName( szName, internal_name, data_type, szUserDefined );
    return((bResult) ? S_OK : E_FAIL );
}


