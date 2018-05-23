/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					protocol.cpp
Description:		Generic directory services protocol COM component
Date:						7/22/97
Version:  			1.0 
Module:					SERVER.DLL
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <afx.h>
#pragma warning(disable : 4201 4514)
#include <objbase.h>
#pragma warning(default : 4201)
#include "DebugNewHelpers.h"
#include <dsapi.h>
#include "protocol.h"


extern long g_cComponents;				// Count of active components



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// CProtocol methods

// Constructor
CProtocol::CProtocol() : m_cRef(1)
{
	m_pIDirConfig = NULL;
  m_pServerList = NULL;
	m_pCurrentServer = NULL;
	m_cNumServers = 0;
	InterlockedIncrement(&g_cComponents);
}

// Destructor
CProtocol::~CProtocol()
{
	if (m_pIDirConfig)
		m_pIDirConfig->Release();
	
	CServer::FreeList(&m_pServerList);
	InterlockedDecrement(&g_cComponents);
}

//////////////////////
// IUnknown methods

HRESULT	__stdcall CProtocol::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
    *ppv = (IUnknown*)this;
	else
	if (iid == IID_IDSPProtocol)
		*ppv = (IDSPProtocol*)this;
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	((IUnknown*)*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CProtocol::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CProtocol::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}
	
///////////////////////
// IDSPProtocol methods

HRESULT	__stdcall CProtocol::Initialize(IUnknown *pUnkCaller)
{
	if (!pUnkCaller)
		return E_INVALIDARG;

	HRESULT hr = pUnkCaller->QueryInterface(IID_IDirConfig, (LPVOID*)&m_pIDirConfig);

	if (SUCCEEDED(hr))
		hr = LoadServers();
	
	return hr;
}


HRESULT __stdcall CProtocol::CreateObject(REFIID iid, LPVOID *ppv)
{
	if (iid == IID_IDSPConfig) {
		IUnknown * punkServer;
		HRESULT hResult = CreateServer(&punkServer);
		if (FAILED(hResult))
			return hResult;

		HRESULT hr = punkServer->QueryInterface(iid, ppv);		// if QI succeeds, ref count will be 2
		if (FAILED(hr))
			punkServer->Release();															// if QI failed, object will be destroyed
		else {
			((CServer*)punkServer)->Attach(&m_pServerList);			// else attach this object to our server list
			m_cNumServers++;
		}
		return hr;
	}
	else
		return E_NOINTERFACE;
}


HRESULT	__stdcall CProtocol::DeleteObject(IUnknown * punkObject)
{
	HRESULT hr;
	IDSPConfig * pIConfig = NULL;
	
	// find out what type of object it is...
	hr = punkObject->QueryInterface(IID_IDSPConfig, (LPVOID*)&pIConfig);
	if (SUCCEEDED(hr)) {
		((CServer*)pIConfig)->DeleteData();								// tell the config object to erase it's config data
		pIConfig->Release();															// release the interface retrieved from QI
		((CServer*)punkObject)->Detach(&m_pServerList);		// detach it from the server list
		m_cNumServers--;
		punkObject->Release();														// release the object
		return S_OK;
	}

// query for another interface here...
// hr = punkObject->QueryInterface(...);

	return E_FAIL;
}


HRESULT __stdcall CProtocol::EnumDatabases(IDSPEnumConfig **ppenum)
{
	if (!ppenum)
		return E_INVALIDARG;

	*ppenum = DEBUG_NEW_NOTHROW CEnumConfig((IUnknown*)this, m_pServerList, m_pCurrentServer);
	if (!*ppenum)
		return E_OUTOFMEMORY;

	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// CServer methods

CServer::CServer() : m_cRef(1), pNext(0)
{
	InterlockedIncrement(&g_cComponents);
}
	
CServer::~CServer()
{
	InterlockedDecrement(&g_cComponents);
}

#if 0
//////////////////////
// IUnknown methods

HRESULT __stdcall CServer::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown || iid == IID_IDSPConfig)
		*ppv = static_cast<IDSPConfig*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CServer::AddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall CServer::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}
#endif

//////////////////////
// list methods


CServer * CServer::FindPrev(LPVOID *ppList, CServer* pNode)
{
	CServer *p;
	
	for (p=(CServer*)*ppList; p; p=p->pNext)
		if (pNode == p->pNext)
			return p;

	return NULL;
}


BOOL CServer::Attach(LPVOID * ppList, CServer* pRefNode/*=INSERT_APPEND*/)
{
	if (!ppList)
		return FALSE;

	pNext = NULL;

	// special case if empty list
	if (!*ppList) {
		*ppList = this;
	}
	// inserting at head of list
	else
	if (pRefNode == INSERT_PREPEND) {
		pNext = (CServer*)*ppList;
		*ppList = this;
	}
	// inserting at tail of list
	else
	if (pRefNode == INSERT_APPEND) {
		CServer * p = FindPrev(ppList, NULL);
		p->pNext = this;
	}
	// inserting after a reference node
	else {
    pNext = pRefNode->pNext;
		pRefNode->pNext = this;
	}
		
	return TRUE;
}


BOOL CServer::Detach(LPVOID * ppList)
{
	if (!ppList)
		return FALSE;

	// adjust the list pointer if we are the first node
	if (IsHead(ppList))
		*ppList = pNext;
	
	CServer* p = FindPrev(ppList, this);
	if (!p)
		return FALSE;

	p->pNext = pNext;
	return TRUE;
}


// (static)
void CServer::FreeList(LPVOID * ppList)
{
	while (*ppList) {
		CServer *p = GetHead(ppList);
		p->Detach(ppList);
		delete p;
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// CEnumConfig methods


// Constructor
CEnumConfig::CEnumConfig(IUnknown *pUnkOuter, LPVOID pServerList, LPVOID pCurrentServer)
 : m_cRef(1)
{
	m_pUnkOuter 			= pUnkOuter;
	m_pServerList 		= pServerList;
	m_pCurrentServer 	= (CServer*)pCurrentServer;
	InterlockedIncrement(&g_cComponents);
	m_pUnkOuter->AddRef();			// reference count the outer object
}

// copy constructor
CEnumConfig::CEnumConfig(CEnumConfig *pSource)
{
	m_pUnkOuter 			= pSource->m_pUnkOuter;
	m_pServerList 		= pSource->m_pServerList;
	m_pCurrentServer 	= pSource->m_pCurrentServer;
	InterlockedIncrement(&g_cComponents);
	m_pUnkOuter->AddRef();			// reference count the outer object
}

CEnumConfig::~CEnumConfig()
{
	m_pUnkOuter->Release();			// release the outer object
	InterlockedDecrement(&g_cComponents);
}

// IUnknown methods
HRESULT	__stdcall CEnumConfig::QueryInterface(const IID& iid, void** ppv)
{
	if (iid == IID_IUnknown || iid == IID_IDSPEnumConfig)
		*ppv = (IDSPEnumConfig*)this;
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	((IUnknown*)*ppv)->AddRef();
	
	return S_OK;
}

ULONG __stdcall CEnumConfig::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CEnumConfig::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}

// IEnumConfig methods
HRESULT __stdcall CEnumConfig::Next(ULONG celt, IDSPConfig ** rgelt, ULONG * pceltFetched)
{
	for (ULONG i=0; i < celt; i++) {
		CServer * p = GetNextServer();
		if (!p)
			return S_FALSE;
		m_pCurrentServer = p;
		if (pceltFetched)
			(*pceltFetched)++;
		((IUnknown*)p)->AddRef();					// must reference count interface before returning
		*rgelt++ = (IDSPConfig*)p;					// put IConfig * into returned array
	}
	return S_OK;
}

HRESULT __stdcall CEnumConfig::Skip(ULONG celt)
{
	for (ULONG i=0; i < celt; i++) {
		CServer * p = GetNextServer();
		if (!p)
			return S_FALSE;
		m_pCurrentServer = p;
	}
	return S_OK;
}

HRESULT __stdcall CEnumConfig::Reset()
{
	m_pCurrentServer = NULL;
	return S_OK;
}

HRESULT __stdcall CEnumConfig::Clone(IDSPEnumConfig ** ppenum)
{
	if (!ppenum)
		return E_INVALIDARG;

	*ppenum = DEBUG_NEW_NOTHROW CEnumConfig(this);
	if (!*ppenum)
		return E_OUTOFMEMORY;

	return S_OK;
}


// helper function
inline CServer * CEnumConfig::GetNextServer()
{ 
	return !m_pCurrentServer ? CServer::GetHead(&m_pServerList) 
													 : m_pCurrentServer->GetNext(); 
}




