//
// File: 				factory.h
// Date:				7/19/97
// Copyright			07/19/1997 Qualcomm Inc.  All Rights Reserved.
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

// Version:  			1.0 
// Description:		    Generic class factory
//
#ifndef __FACTORY_H__
#define __FACTORY_H__


class CFactoryData;

// Global data used by CFactory
extern CFactoryData g_FactoryDataArray[];
extern UINT         g_cFactoryDataEntries;



// Component creation function
typedef IUnknown* (*FPCREATEINSTANCE)();


// CFactoryData
//
class CFactoryData
{
    public:
	const CLSID* 		m_pCLSID;					// The class ID for the component
	FPCREATEINSTANCE 	CreateInstance;				// Pointer to the function that creates it
	const char* 		m_RegistryName;				// Name of the component to register in the registry
	const char* 		m_szProgID;					// ProgID
	const char* 		m_szVerIndProgID;			// Version-independent ProgID
	const char*			m_szExtraInfo;				// Extra key for CLSID storage
													//
	BOOL IsClassID( const CLSID& clsid ) const	    // Helper function for finding the class ID
	{    return( *m_pCLSID == clsid ); }
};


// Generic class factory class
//
class CFactory : public IClassFactory
{
    public:
	// IUnknown
	virtual HRESULT	__stdcall QueryInterface( REFIID iid, LPVOID* ppv );
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IClassFactory
	virtual HRESULT __stdcall CreateInstance( IUnknown* pUnknownOuter, REFIID iid, LPVOID* ppv );
	virtual HRESULT __stdcall LockServer( BOOL bLock );

    CFactory( const CFactoryData* pFactoryData );
	~CFactory() {}

	static BOOL 			  IsLocked() { return( s_cServerLocks > 0 ); }
    static HRESULT 			  GetClassObject( const CLSID& clsid, const IID& iid, LPVOID* ppv );
	static HRESULT 			  RegisterAll();
	static HRESULT 			  UnregisterAll();
	static HRESULT 			  InstallDirCLSIDs( HKEY registryKey, LPSTR subKey );
    static HRESULT            UninstallServer(  HKEY registryKey, LPSTR subKey );

    // Data Members
	long					  m_cRef;
	static LONG				  s_cServerLocks;
	static HMODULE			  s_hModule;
	const  CFactoryData*	  m_pFactoryData;
};


#endif //__FACTORY_H__




