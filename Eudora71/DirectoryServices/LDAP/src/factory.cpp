//
// File: 			factory.cpp
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
// Description:		Generic class factory implementation
//
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include "DebugNewHelpers.h"
#include "registry.h"
#include "factory.h"


// Static variables
//
LONG 	CFactory::s_cServerLocks = 0;       // Count of locks on this server
HMODULE CFactory::s_hModule      = NULL;	// DLL module handle


// CFactory Methods
//
CFactory::CFactory( const CFactoryData* pFactoryData ) : m_cRef( 1 )
{
	m_pFactoryData = pFactoryData;
}


//
//
HRESULT	__stdcall CFactory::QueryInterface( REFIID iid, LPVOID* ppv)
{
	     if (iid == IID_IUnknown) {
		     *ppv = static_cast<IUnknown*>(this);
    }
	else if (iid == IID_IClassFactory) {
		     *ppv = static_cast<IClassFactory*>(this);
    }
	else {
		     *ppv = NULL;
		     return( E_NOINTERFACE );
	}	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return( S_OK );
}


//
ULONG __stdcall CFactory::AddRef()
{
	return( InterlockedIncrement( &m_cRef ));
}


//
ULONG __stdcall CFactory::Release()
{
	if (InterlockedDecrement( &m_cRef ) == 0) {
		delete this;
		return( 0 );
	}
	return( m_cRef );
}


// IClassFactory Methods
//
HRESULT __stdcall CFactory::CreateInstance( IUnknown* pUnknownOuter, REFIID iid, LPVOID* ppv )
{
	HRESULT hr;

	if (pUnknownOuter != NULL) return( CLASS_E_NOAGGREGATION );

	IUnknown* pObj = m_pFactoryData->CreateInstance();
	if (!pObj) return( E_OUTOFMEMORY );

	hr = pObj->QueryInterface( iid, ppv );
	pObj->Release();
	return( hr );
}


//
//
HRESULT __stdcall CFactory::LockServer( BOOL bLock )
{
	if ( bLock )
		 InterlockedIncrement( &s_cServerLocks );
	else InterlockedDecrement( &s_cServerLocks );
	return( S_OK );
}


// GetClassObject - Create a class factory based on a CLSID.
//
HRESULT CFactory::GetClassObject( const CLSID& clsid, const IID& iid, LPVOID* ppv )
{
	if ((iid != IID_IUnknown) && (iid != IID_IClassFactory)) return( E_NOINTERFACE );
	
	// -> traverse the array of data looking for this class ID.
	for( UINT i=0; i < g_cFactoryDataEntries; i++ ) 
    {
		const CFactoryData* pData = &g_FactoryDataArray[i];
		if (pData->IsClassID( clsid )) 
        {
			// -> found the ClassID in the array of components we can create.  So create a class factory for 
            //    this component. Pass the CFactoryData structure to the class factory so that it knows what
            //    kind of components to create.
			*ppv = (IUnknown*) new CFactory( pData );
			if ( *ppv == 0 ) {
				 return( E_OUTOFMEMORY );
			}
			else {
			    return( NOERROR );
			}

		}
	}
	return( CLASS_E_CLASSNOTAVAILABLE );
}


// COM objects should self register and unregister when called upon to do so by install programs, etc.
//
HRESULT CFactory::RegisterAll()
{
	for( UINT i=0; i < g_cFactoryDataEntries; i++ )	RegisterServer( s_hModule, *(g_FactoryDataArray[i].m_pCLSID));
	return( S_OK );
}


//
//
HRESULT CFactory::UnregisterAll()
{
	for( UINT i=0; i < g_cFactoryDataEntries; i++ )	UnregisterServer( *(g_FactoryDataArray[i].m_pCLSID));
	return( S_OK );
}


// Special case: for us only the first entry in the Factory Data Array defines an Installable CLSID
// The second entry is an internal COM object. 
//
HRESULT CFactory::InstallDirCLSIDs( HKEY registryKey, LPSTR subKey )
{
	return( InstallServer( registryKey, subKey, *(g_FactoryDataArray[0].m_pCLSID)) );
}


//
//
HRESULT CFactory::UninstallServer( HKEY registryKey, LPSTR subKey )
{
	for( UINT i=0; i < g_cFactoryDataEntries; i++ ) {
		 DeleteKeyAsCLSID( registryKey, subKey, *(g_FactoryDataArray[i].m_pCLSID));
	}
	return( S_OK );
}
