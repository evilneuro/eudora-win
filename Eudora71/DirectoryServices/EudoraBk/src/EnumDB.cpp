/******************************************************************************/
/*																										*/
/*	Name		:	ENUMDB.CPP  																   */
/* Date     :  10/10/1997                                                     */
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

/*	Desc.		:	IDSPEnumConfig Implementation  											*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <objbase.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"
#include "database.h"
#include "EnumDB.h"


extern long g_cComponents;

HRESULT	__stdcall CEnumDatabase::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else if (iid == IID_IDSPEnumConfig)
		*ppv = static_cast<IDSPEnumConfig*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CEnumDatabase::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CEnumDatabase::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

CEnumDatabase::CEnumDatabase(CProtocol *pProtocol,IListMan *pDatabaseList,BOOL bClone) : m_cRef(1)
{
   m_pCur         = NULL;
   m_pProtocol    = pProtocol;
   m_pList        = NULL;
   m_dwIdx        = 0;
   m_bToast       = FALSE;
   m_pProtocol->AddRef();

   if(TRUE == bClone) {
      InterlockedIncrement(&g_cComponents);
      return;
   }

   m_pList        = pDatabaseList;
   m_pList->AddRef();

   InterlockedIncrement(&g_cComponents);
}

CEnumDatabase::~CEnumDatabase()
{
   if(m_pProtocol)      m_pProtocol->Release();
   if(m_pList)          m_pList->Release();

   m_pList              = NULL;
   m_pProtocol          = NULL;
   m_pCur               = NULL;
   InterlockedDecrement(&g_cComponents);
}


HRESULT __stdcall CEnumDatabase::Next(ULONG celt,IDSPConfig ** rgelt,ULONG * pceltFetched)
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
		m_pCur->pUnkDatabase->AddRef();
		rgelt[Fetched] = static_cast<IDSPConfig*>(m_pCur->pUnkDatabase);
		
		/* Increment count and move to next item											*/
		Fetched++;
		m_pCur 			= (LPDBENT) ILIST_ITEMNEXT(m_pCur);
	}
	if(pceltFetched) 		*pceltFetched = Fetched;
	if(Fetched == celt)  return S_OK;
	return S_FALSE;
}

HRESULT __stdcall CEnumDatabase::Skip(ULONG celt)
{
	ULONG			Fetched 		= 0;
	HRESULT		hErr			= E_OUTOFMEMORY;
	IDSPConfig	*pcDatabase	= NULL;
	
	if(TRUE == m_bToast)	
		return S_FALSE;
	
	pcDatabase = (IDSPConfig *) DEBUG_NEW_NOTHROW char[sizeof(IDSPConfig *) * celt];
	
	if(pcDatabase) {
		memset(pcDatabase, 0, sizeof(IDSPConfig *) * celt);
		hErr = Next(celt,&pcDatabase,&Fetched);

		delete [] reinterpret_cast<char *>(pcDatabase);

		return(S_OK);
	}
	return hErr;
}

HRESULT __stdcall CEnumDatabase::Reset(void)
{
	if(TRUE == m_bToast)	
		return S_FALSE;
	
	m_pCur = NULL;
	return(S_OK);
}

HRESULT __stdcall CEnumDatabase::Clone(IDSPEnumConfig ** ppIEnum)
{
	if(TRUE == m_bToast)	
		return S_FALSE;
	
	CEnumDatabase	*p = DEBUG_NEW_NOTHROW CEnumDatabase(m_pProtocol,NULL,TRUE);
	
	*ppIEnum = NULL;
	if(p) {
		p->m_pCur = m_pCur;
		p->m_dwIdx= m_dwIdx;
		p->m_pList= m_pList;
		reinterpret_cast<IUnknown*>(p->m_pList)->AddRef();
		(*ppIEnum) = static_cast<IDSPEnumConfig*>(p);
		return(S_OK);
	}
	return(E_OUTOFMEMORY);
}




