// LDAP Directory Services 
//
// File: 			EnumConfig.cpp
// Date:			7/29/97
// Copyright		07/29/1997 Qualcomm Inc.  All Rights Reserved.
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
//
// Description:		
// Implementation of an IDSPEnumConfig COM object.  
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include "DebugNewHelpers.h"
#include <dspapi.h>
#include "standard.h"
#include "EnumConfig.h"


extern DWORD g_dwComponentCount;


// This enum COM object only requires the LPILISTMAN COM object to be around for it.
// No other COM object must be around for it to work properly.
//
CEnumConfig::CEnumConfig( LPILISTMAN lpObj ) : m_Ref( 1 ),
											   m_ServerList( lpObj ),
											   m_CurrentServer( NULL )
{
    g_dwComponentCount++;
    if (m_ServerList) {
        m_ServerList->AddRef();
        m_CurrentServer = (LPEXISTING) m_ServerList->Head();
    }
}


// Copy Constructor used in cloning a CEnumConfig.
//
CEnumConfig::CEnumConfig( const CEnumConfig& eConfig )
{
    m_Ref           = 1;
    m_ServerList    = eConfig.m_ServerList;
    m_CurrentServer = eConfig.m_CurrentServer;
    if (m_ServerList) m_ServerList->AddRef();
}


// Free all allocated resources
//
CEnumConfig::~CEnumConfig()
{
    if (m_ServerList) m_ServerList->Release();
    g_dwComponentCount--;
}


//
//
HRESULT	__stdcall CEnumConfig::QueryInterface( REFIID iid, LPVOID* ppv)
{
	     if (iid == IID_IUnknown) {
		     *ppv = static_cast<IDSPEnumConfig*>(this);
    }
	else {
		     *ppv = NULL;
		     return( E_NOINTERFACE );
	}	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return( S_OK );
}


//
ULONG __stdcall CEnumConfig::AddRef()
{
	return( InterlockedIncrement( &m_Ref ));
}


//
ULONG __stdcall CEnumConfig::Release()
{
	if (InterlockedDecrement( &m_Ref ) == 0) {
		delete this;
		return( 0 );
	}
	return( m_Ref );
}


// Note: m_ServerList is a circular list!
//
HRESULT __stdcall CEnumConfig::Next( ULONG        celt,          // in:
                                     IDSPConfig** rgelt,         // out:
                                     ULONG*       pceltFetched   // out:
								   )
{
    LPEXISTING   pHead   = NULL;
    IDSPConfig** pCopyTo = rgelt;
    ULONG        i       = 0;
    DWORD        count   = 0;

    // -> is there nothing to return ?
    *pceltFetched = 0; 
    if (m_ServerList == NULL) return( S_FALSE );
    pHead = (LPEXISTING) m_ServerList->Head();
    count = m_ServerList->Count();
    if (pHead == NULL) return( S_FALSE );

    // ->
    while( i < celt && m_CurrentServer )
    {
           if (m_CurrentServer->ConfigObj) 
           {   // -> providing a COM object pointer to the caller means that we MUST add ref it
               *pCopyTo++ = static_cast<IDSPConfig*>( m_CurrentServer->ConfigObj ); 
               m_CurrentServer->ConfigObj->AddRef();
               i++;
           }
           // -> are there less entries on our list than required by the caller ?
           m_CurrentServer = (LPEXISTING) ILIST_ITEMNEXT( m_CurrentServer );
           if (m_CurrentServer == pHead) m_CurrentServer = NULL;
    } 
    *pceltFetched = i;
    return((i == celt) ? S_OK : S_FALSE );
}


// Same as "Next" except we don't copy the data into the caller's buffer.
//
HRESULT __stdcall CEnumConfig::Skip( ULONG celt )   // in:
{
    LPEXISTING pHead = NULL;
    ULONG      i     = 0;

    // -> is there nothing to skip ?
    if (m_ServerList == NULL) return( S_FALSE );
    pHead = (LPEXISTING) m_ServerList->Head();
    if (pHead == NULL) return( S_FALSE );

    // ->
    while( i < celt && m_CurrentServer )
    {
           i++;
           // -> are there less entries on our list than required by the caller ?
           m_CurrentServer = (LPEXISTING) ILIST_ITEMNEXT( m_CurrentServer );
           if (m_CurrentServer == pHead) m_CurrentServer = NULL;
    } 
    return((i == celt) ? S_OK : S_FALSE );
}


//
//
HRESULT __stdcall CEnumConfig::Reset()
{
    if (m_ServerList) {
        m_CurrentServer = (LPEXISTING) m_ServerList->Head();
        return( S_OK );
    }
    return( E_FAIL );
}


//
//
HRESULT __stdcall CEnumConfig::Clone( IDSPEnumConfig** ppenum )   // in:
{
    *ppenum = this;
    return((*ppenum == NULL) ? E_FAIL : S_OK );
}


