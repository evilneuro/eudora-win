/******************************************************************************/
/*																										*/
/*	Name		:	DATABASE.CPP	  																*/
/* Date     :  7/30/1997                                                      */
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

/*	Desc.		:	IDSPConfig object wrapper class implementation						*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "database.h"


extern LONG g_cComponents;

typedef struct tagDBENT {
    ITEM			item;
    CDSDatabase	*pcDatabase;
} DBENT, *LPDBENT;

/* Static */
void CDSEnumDatabases::DentFreeCB(LPITEM pItem,LPVOID)
{
    LPDBENT pDent = (LPDBENT) pItem;
	
    pDent->pcDatabase->Release();
    pDent->pcDatabase = NULL;

	delete pDent;
}

/* Protected */
HRESULT __stdcall CDSEnumDatabases::AddProtocolDatabases(IDSPProtocol *pProto)
{
    IDSPEnumConfig	*pEnum	= NULL;
    IDSPConfig		*pConfig	= NULL;
    ULONG				nConfig	= 0;
	
    if(SUCCEEDED(pProto->EnumDatabases(&pEnum))) {
	while(S_FALSE != pEnum->Next(1,&pConfig,&nConfig)) {
		LPDBENT 	pDent = DEBUG_NEW_NOTHROW DBENT;
	    
	    if (pDent) {
			pDent->pcDatabase = DEBUG_NEW_NOTHROW CDSDatabase(pConfig);
			m_pList->Attach(pDent);
	    }
	    pConfig->Release();
	}
	pEnum->Release();
	return S_OK;
    }
    return E_FAIL;
}

/******************************************************************************/
/* CDSEnumDatabases																				*/
/******************************************************************************/
CDSEnumDatabases::CDSEnumDatabases(IListMan *pProtocolList,CDSProtocol *pcProto,BOOL bClone) : m_cRef(1)
{
    m_pList 	= NULL;
    m_pCur	= NULL;
    m_dwIdx	= 0;
    m_bToast	= FALSE;
	
    if(TRUE == bClone)
	return;
	
    if(SUCCEEDED(CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pList))) {
	m_pList->Initialize(CDSEnumDatabases::DentFreeCB,NULL);
	/* Enumerate only the specified protocol											*/
	if(pcProto) {
	    if(FAILED(AddProtocolDatabases(pcProto->GetWrappedInterface()))) {
		m_bToast = TRUE;
	    }
	}
	/* Enumerate ALL the protocols														*/
	else {
	    LPPENT	pPent		= (LPPENT) pProtocolList->Head();
	    DWORD		dwCount 	= pProtocolList->Count();
	    DWORD		i;
			
	    for(i=0;i<dwCount;i++) {
			AddProtocolDatabases(pPent->pDSP->GetWrappedInterface());
			pPent = (LPPENT) ILIST_ITEMNEXT(pPent);
	    }
	}
    }
    else {
	m_bToast			= TRUE;
    }
    InterlockedIncrement(&g_cComponents);
}

CDSEnumDatabases::~CDSEnumDatabases()
{
    if(m_pList)		m_pList->Release();

    m_pList			= NULL;
    InterlockedDecrement(&g_cComponents);
}


HRESULT	__stdcall CDSEnumDatabases::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == IID_IUnknown)
	*ppv = static_cast<IUnknown*>(this);
    else
	if (iid == IID_IDSEnumProtocols)
	    *ppv = static_cast<IDSEnumDatabases*>(this);
	else {
	    *ppv = NULL;
	    return E_NOINTERFACE;
	}
	
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;
}

ULONG __stdcall CDSEnumDatabases::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDSEnumDatabases::Release()
{
    if (0 == InterlockedDecrement(&m_cRef)) {
	delete this;
	return 0;
    }
    return m_cRef;
}


HRESULT __stdcall CDSEnumDatabases::Next(ULONG celt,IDSDatabase ** rgelt,ULONG * pceltFetched)
{
    DWORD	dwTotal 	= m_dwIdx + celt;
    ULONG	Fetched 	= 0;
	
    if(TRUE == m_bToast)	
	return S_FALSE;
	
    if(NULL == m_pCur)
	m_pCur = (LPDBENT) m_pList->Head();
	
    for(m_dwIdx;m_dwIdx<dwTotal;m_dwIdx++) {
	if(m_dwIdx >= m_pList->Count())
	    break;
	/* Stuff Pointer																			*/
	reinterpret_cast<IUnknown*>(m_pCur->pcDatabase)->AddRef();
	rgelt[Fetched] = static_cast<IDSDatabase*>(m_pCur->pcDatabase);
		
	/* Increment count and move to next item											*/
	Fetched++;
	m_pCur 			= (LPDBENT) ILIST_ITEMNEXT(m_pCur);
    }
    if(pceltFetched) 		*pceltFetched = Fetched;
    if(Fetched == celt)  return S_OK;
    return S_FALSE;
}

HRESULT __stdcall CDSEnumDatabases::Skip(ULONG celt)
{
    ULONG			Fetched = 0;
    HRESULT			hErr = E_OUTOFMEMORY;
    IDSDatabase	*	pcDatabase = reinterpret_cast<IDSDatabase *>( DEBUG_NEW_NOTHROW char[sizeof(IDSDatabase *) * celt] );
	
    if (TRUE == m_bToast)	
		return S_FALSE;
	
    if (pcDatabase) {
		hErr = Next(celt,&pcDatabase,&Fetched);

		delete [] reinterpret_cast<char *>(pcDatabase);

		return(S_OK);
    }
    return hErr;
}

HRESULT __stdcall CDSEnumDatabases::Reset(void)
{
    if(TRUE == m_bToast)	
	return S_FALSE;
	
    m_pCur = NULL;
    return(S_OK);
}

HRESULT __stdcall CDSEnumDatabases::Clone(IDSEnumDatabases ** ppIEnum)
{
    if(TRUE == m_bToast)	
	return S_FALSE;
	
    CDSEnumDatabases	*p = DEBUG_NEW_NOTHROW CDSEnumDatabases(NULL,NULL,TRUE);
	
    *ppIEnum = NULL;
    if(p) {
		p->m_pCur = m_pCur;
		p->m_dwIdx= m_dwIdx;
		p->m_pList= m_pList;
		reinterpret_cast<IUnknown*>(p->m_pList)->AddRef();
		(*ppIEnum) = static_cast<IDSEnumDatabases *>(p);
		return(S_OK);
    }
    return(E_OUTOFMEMORY);
}

/******************************************************************************/
/* CDSDatabase																						*/
/******************************************************************************/

CDSDatabase::CDSDatabase(IDSPConfig *pConfig) : m_cRef(1)
{
    m_pConfig = pConfig;
    m_pConfig->AddRef();
    InterlockedIncrement(&g_cComponents);
}

CDSDatabase::~CDSDatabase()
{
    if(m_pConfig) m_pConfig->Release();
    m_pConfig = NULL;
    InterlockedDecrement(&g_cComponents);
}

HRESULT	__stdcall CDSDatabase::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == IID_IUnknown)
	*ppv = static_cast<IUnknown*>(this);
    else
	if (iid == IID_IDSDatabase)
	    *ppv = static_cast<IDSDatabase*>(this);
	else {
	    *ppv = NULL;
	    return E_NOINTERFACE;
	}
	
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;
}

ULONG __stdcall CDSDatabase::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDSDatabase::Release()
{
    if (0 == InterlockedDecrement(&m_cRef)) {
	delete this;
	return 0;
    }
    return m_cRef;
}

HRESULT __stdcall CDSDatabase::GetName(LPSTR lpszBuffer, UINT nBufferSize)
{
    if(m_pConfig)
	return m_pConfig->GetName(lpszBuffer,nBufferSize);
	
    return E_FAIL;
}

HBITMAP __stdcall CDSDatabase::GetImage(int iImage)
{
    if(m_pConfig)
	return m_pConfig->GetImage(iImage);
	
    return NULL;
}

HRESULT __stdcall CDSDatabase::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
    if(m_pConfig)
	return m_pConfig->GetProtocolName(lpszBuffer,nBufferSize);
	
    return E_FAIL;
}

HBITMAP __stdcall CDSDatabase::GetProtocolImage(int iImage)
{
    if(m_pConfig)
	return m_pConfig->GetProtocolImage(iImage);
	
    return NULL;
}

DWORD	__stdcall CDSDatabase::GetProtocolFlags()
{
    if(m_pConfig)
	return m_pConfig->GetProtocolFlags();
	
    return 0;
}

int __stdcall CDSDatabase::GetPropPageCount()
{
    if(m_pConfig)
	return m_pConfig->GetPropPageCount();
	
    return 0;
	
}

HRESULT 	__stdcall CDSDatabase::GetPropPages(HPROPSHEETPAGE *paHPropPages)
{
    if(m_pConfig)
	return m_pConfig->GetPropPages(paHPropPages);
	
    return E_FAIL;
}


IDSPConfig * __stdcall CDSDatabase::GetWrappedInterface()
{
    return(m_pConfig);
}

LPSTR __stdcall CDSDatabase::GetDatabaseID()
{
    return m_pConfig->GetID();
}



