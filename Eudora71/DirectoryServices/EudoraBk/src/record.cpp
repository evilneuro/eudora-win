/******************************************************************************/
/*																										*/
/*	Name		:	RECORD.CPP  																   */
/* Date     :  10/14/1997                                                     */
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

/*	Desc.		:	IDSPRecord Implementation  												*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"
#include "record.h"
#include "resource.h"
#include "QCUtils.h"


extern long g_cComponents;

HRESULT	__stdcall CRecord::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)         *ppv = static_cast<IUnknown*>(this);
	else if (iid == IID_IDSPRecord)  *ppv = static_cast<IDSPRecord*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CRecord::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CRecord::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

CRecord::CRecord(CDatabase *pDatabase) : m_cRef(1)
{
   LPSTR          pszDBID = pDatabase->GetID();
   m_pRecList     = NULL;
   m_pTail        = NULL;
   m_pszDatabaseID= NULL;
   m_nError       = QUERY_OK;

   if(pszDBID) {
      m_pszDatabaseID = SafeStrdupMT(pszDBID);
   }

	InterlockedIncrement(&g_cComponents);
}

CRecord::~CRecord()
{
   delete m_pszDatabaseID;

   while(m_pRecList) {
      DBRECENT *pEnt = m_pRecList;

      m_pRecList     = pEnt->pNext;
	  delete pEnt;
   }
   m_pszDatabaseID         = NULL;
	InterlockedDecrement(&g_cComponents);
}

DBRECENT * __stdcall CRecord::GetRecordList()
{
   return m_pRecList;
}

LPSTR __stdcall CRecord::GetRecordID()
{
   return m_pszRecID;
}

LPSTR __stdcall CRecord::GetDatabaseID()
{
   return m_pszDatabaseID;
}

QUERY_STATUS __stdcall CRecord::GetError(LPSTR pszBuffer,int nLen)
{
   if(pszBuffer)
      ::LoadString(CFactory::s_hModule,IDS_COOL_STR,pszBuffer,nLen);
   return m_nError;
}

HRESULT __stdcall CRecord::Add(DS_FIELD nField,LPSTR pszData)
{

   if(pszData && *pszData) {
      int      len = lstrlen(pszData);
      DBRECENT *pE = reinterpret_cast<DBRECENT *>( DEBUG_NEW_NOTHROW char[sizeof(DBRECENT) + len + 1] );

      if(pE) {
         memset(pE, 0, sizeof(DBRECENT) + len + 1);
         lstrcpy((LPSTR) pE->data,pszData);
         pE->nName   = nField;
         pE->nType   = DST_ASCIIZ;
         pE->dwSize  = (DWORD) len;

         if(!m_pRecList) {
            m_pRecList = m_pTail = pE;
         }
         else {
            m_pTail->pNext = pE;
            m_pTail = pE;
         }
         return S_OK;
      }
      return E_OUTOFMEMORY;
   }
   return E_INVALIDARG;
}





