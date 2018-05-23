/******************************************************************************/
/*																										*/
/*	Name		:	PROTO.CPP  		  																*/
/* Date     :  7/28/1997                                                      */
/* Author   :  Jim Susoy                                                      */
/* Notice   :  (C) 1997 Qualcomm, Inc. - All Rights Reserved                  */
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

/*	Desc.		:	IPProtocol object wrapper class implementation						*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "dspapi.h"
#include "ds.h"
#include "database.h"
#include "proto.h"
#include "resource.h"


extern long g_cComponents;

/******************************************************************************/
/* CDSEnumProtocols																				*/
/******************************************************************************/
CDSEnumProtocols::CDSEnumProtocols(IListMan *pList) : m_cRef(1)
{
	m_pList	= pList;
	m_pCur	= NULL;
	m_dwIdx	= 0;
	InterlockedIncrement(&g_cComponents);
}

CDSEnumProtocols::~CDSEnumProtocols()
{
	InterlockedDecrement(&g_cComponents);
}


HRESULT	__stdcall CDSEnumProtocols::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IDSEnumProtocols)
		*ppv = static_cast<IDSEnumProtocols*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CDSEnumProtocols::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDSEnumProtocols::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}


HRESULT __stdcall CDSEnumProtocols::Next(ULONG celt,IDSProtocol ** rgelt,ULONG * pceltFetched)
{
	DWORD	dwTotal 	= m_dwIdx + celt;
	ULONG	Fetched 	= 0;
	
	/* If we are at the tail, bail...														*/
	//if((m_dwIdx+1) == m_pList->Count())
	//	return S_FALSE;
		
	if(NULL == m_pCur)
		m_pCur = (LPPENT) m_pList->Head();
	
	for(m_dwIdx;m_dwIdx<dwTotal;m_dwIdx++) {
		if(m_dwIdx >= m_pList->Count())
			break;
		/* Stuff Pointer																			*/
		reinterpret_cast<IUnknown*>(m_pCur->pDSP)->AddRef();
		rgelt[Fetched] = static_cast<IDSProtocol*>(m_pCur->pDSP);
		
		/* Increment count and move to next item											*/
		Fetched++;
		m_pCur 			= (LPPENT) ILIST_ITEMNEXT(m_pCur);
	}
	if(pceltFetched) 		*pceltFetched = Fetched;
	if(Fetched == celt)  return S_OK;
	return S_FALSE;
}

HRESULT __stdcall CDSEnumProtocols::Skip(ULONG celt)
{
	ULONG			Fetched 	= 0;
	HRESULT		hErr		= E_OUTOFMEMORY;
	IDSProtocol	*pDSP		= NULL;
	
	pDSP = reinterpret_cast<IDSProtocol *>( DEBUG_NEW_NOTHROW char[sizeof(IDSProtocol *) * celt] );
	
	if(pDSP) {
		hErr = Next(celt,&pDSP,&Fetched);

		delete [] reinterpret_cast<char *>(pDSP);

		return(S_OK);
	}
	return hErr;
}

HRESULT __stdcall CDSEnumProtocols::Reset(void)
{
	m_pCur = NULL;
	return(S_OK);
}

HRESULT __stdcall CDSEnumProtocols::Clone(IDSEnumProtocols ** ppIEnum)
{
	CDSEnumProtocols	*p = DEBUG_NEW_NOTHROW CDSEnumProtocols(m_pList);
	
	*ppIEnum = NULL;
	if(p) {
		p->m_pCur = m_pCur;
		p->m_dwIdx= m_dwIdx;
		(*ppIEnum) = static_cast<IDSEnumProtocols *>(p);
		return(S_OK);
	}
	return(E_OUTOFMEMORY);
}


/******************************************************************************/
/* CDSPrototocl																					*/
/******************************************************************************/
CDSProtocol::CDSProtocol() : m_cRef(1)
{
	m_pProtoObj 	= NULL;
	m_BaseKeyValid	= FALSE;
	m_ErrorValue	= 0;
	m_SubKeyIndex	= 0;
	m_ValueIndex	= 0;
	m_hEnumKey		= 0;
	m_EnumKeyValid	= FALSE;
   m_UtilBuf[0]	= 0;
	InterlockedIncrement(&g_cComponents);
}

CDSProtocol::~CDSProtocol()
{
	if(m_pProtoObj) m_pProtoObj->Release();
	m_pProtoObj = NULL;
	
	if (TRUE == m_BaseKeyValid) RegCloseKey(m_hBaseKey);
	m_BaseKeyValid = FALSE;
	InterlockedDecrement(&g_cComponents);
}

HRESULT __stdcall CDSProtocol::HarryCarry()
{
	if(m_pProtoObj) {
		m_pProtoObj->Release();
	}
	m_pProtoObj = NULL;
	return S_OK;
}

HRESULT	__stdcall CDSProtocol::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown || iid == IID_IDirServ) {
		//*ppv = static_cast<IUnknown*>(this);
		*ppv = static_cast<IDSProtocol*>(this);
	}
	else if (iid == IID_IDirConfig) {
		*ppv = static_cast<IDirConfig*>(this);
	}
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CDSProtocol::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDSProtocol::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT __stdcall CDSProtocol::Initialize(LPSTR RegStr,LPSTR pCLSID)
{
	HRESULT	hErr;
	CLSID		clsid;
	
	wchar_t wchBuffer[39+2];
	mbstowcs(wchBuffer, pCLSID, 39);
	if(SUCCEEDED(hErr = CLSIDFromString(wchBuffer, (CLSID*)&clsid))) {
		/* Before we do any real work, let setup the data for the IDirConfig		*/
		/* interface.																				*/
		DWORD  disposition;
		
		if(SUCCEEDED(hErr = RegCreateKeyEx(HKEY_CURRENT_USER,RegStr,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&m_hBaseKey,&disposition))) {
			m_BaseKeyValid = TRUE;
			if(SUCCEEDED(hErr = CoCreateInstance(clsid,NULL,CLSCTX_INPROC_SERVER,IID_IDSPProtocol,(void **) &m_pProtoObj))) {
				hErr = m_pProtoObj->Initialize(static_cast<IDirConfig*>(this));
         }
		}
	}
    if(FAILED(hErr) && m_pProtoObj)
		 m_pProtoObj->Release();

	return(hErr);
}

HRESULT __stdcall CDSProtocol::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
	if(m_pProtoObj)
		return m_pProtoObj->GetProtocolName(lpszBuffer,nBufferSize);
	
	return(E_FAIL);
}

HBITMAP __stdcall CDSProtocol::GetProtocolImage(int iImage)
{
	if(m_pProtoObj)
		return m_pProtoObj->GetProtocolImage(iImage);
	
	return((HBITMAP) NULL);
}

DWORD	__stdcall CDSProtocol::GetProtocolFlags()
{
	if(m_pProtoObj)
		return m_pProtoObj->GetProtocolFlags();
	
	return(0);
}

HRESULT __stdcall CDSProtocol::CreateObject(REFIID riid,void **ppv)
{
	if(m_pProtoObj)
		return m_pProtoObj->CreateObject(riid,ppv);
	
	return E_FAIL;
}

HRESULT __stdcall CDSProtocol::DeleteObject(IUnknown *pUnk)
{
	return m_pProtoObj->DeleteObject(pUnk);
}


IDSPProtocol *__stdcall CDSProtocol::GetWrappedInterface()
{
	return(m_pProtoObj);
}

HRESULT __stdcall CDSProtocol::GetFieldName(DS_FIELD nField,LPSTR pszBuffer,int nLen)
{
	return MapFieldName(CFactory::s_hModule,nField,pszBuffer,nLen);
}

HRESULT __stdcall CDSProtocol::GetFieldKey(DS_FIELD nField,LPSTR pszBuffer,int nLen)
{
	return FindFieldKey(nField,pszBuffer,nLen);
}

HRESULT __stdcall CDSProtocol::MapFieldKey(DS_FIELD *pnField,LPSTR pszKey)
{
	return LookupFieldKey(pnField,pszKey);
}


