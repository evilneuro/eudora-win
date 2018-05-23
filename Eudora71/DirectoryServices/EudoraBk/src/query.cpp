/******************************************************************************/
/*																										*/
/*	Name		:	QUERY.CPP  																      */
/* Date     :  10/13/1997                                                     */
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

/*	Desc.		:	IDSPQuery Implementation  												   */
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "dspapi.h"
#include "query.h"
#include "resource.h"
#include "QCUtils.h"


extern long g_cComponents;

/* Static */
HRESULT __stdcall CQuery::CreateInstance(IUnknown **pUnk,CDatabase *pServer)
{
	CQuery *pPost = DEBUG_NEW_NOTHROW CQuery(pServer);
	
	if(!pPost) {
		*pUnk = NULL;
		return E_OUTOFMEMORY;
	}
	
	*pUnk = static_cast<IUnknown *>(pPost);
	return S_OK;
}

HRESULT	__stdcall CQuery::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)         *ppv = static_cast<IUnknown*>(this);
	else if (iid == IID_IDSPQuery)    *ppv = static_cast<IDSPQuery*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CQuery::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CQuery::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

CQuery::CQuery(CDatabase *pServer) : m_cRef(1)
{
   m_pSched       = NULL;
   m_nLastError   = QUERY_OK;
   m_dwFlags      = 0;
   m_nField       = DS_UNKNOWN;
   m_bCancled     = FALSE;
   m_pszSearch    = NULL;
   m_pRE          = NULL;
   m_pRecordList  = NULL;
   m_pDatabase    = pServer;
   m_pDatabase->AddRef();
   m_pDatabase->IncQuery();
	InterlockedIncrement(&g_cComponents);
}

CQuery::~CQuery()
{
   if(m_pSched) {
      m_pSched->Cancel();
      m_pSched->Release();
   }

   m_pDatabase->DecQuery();
   if(m_pDatabase)   m_pDatabase->Release();

   delete [] m_pszSearch;

   if(m_pRecordList) m_pRecordList->Release();

   m_pRecordList     = NULL;
   m_pszSearch       = NULL;
   m_pDatabase       = NULL;
   m_pSched          = NULL;
	InterlockedDecrement(&g_cComponents);
}

HRESULT __stdcall CQuery::InitQuery(LPSTR pszSearch,DWORD dwFlags,DS_FIELD nField)
{
   HRESULT  hErr;

   if(NULL == (m_pszSearch = SafeStrdupMT(pszSearch)))
      return E_OUTOFMEMORY;

   m_dwFlags   = dwFlags;
   m_nField    = nField;
   if(SUCCEEDED(hErr = CoCreateInstance(CLSID_ISchedule,NULL,CLSCTX_INPROC_SERVER,IID_ISchedule,(void **) &m_pSched))) {
      hErr = m_pDatabase->GetRecordList(&m_pRecordList);
   }
   return hErr;
}

HRESULT __stdcall CQuery::SetSearchBase(LPSTR /* pszSearchBase */)
{
   return E_FAIL;
}

void __stdcall CQuery::Start(DBQUERYCB pCB,LPVOID pCtx)
{
   m_pSched->Schedule(pCB,pCtx);
}

void __stdcall CQuery::WaitForRecord(DBQUERYCB pCB,LPVOID pCtx)
{
   m_pSched->Schedule(pCB,pCtx);
}

/* Protected */
BOOL __stdcall CQuery::FuzzyCompare(LPSTR lpszCheck)
{
   int            iLen;
   char           ch;
   int            nRet;
   register LPSTR pszCheck = lpszCheck;

   iLen = lstrlen(m_pszSearch);
   while(lstrlen(pszCheck) >= iLen){
      ch = pszCheck[iLen];
      pszCheck[iLen] = 0;
      nRet = lstrcmpi(pszCheck,m_pszSearch);
      pszCheck[iLen] = ch;
      if(!nRet)
         return TRUE;
      pszCheck++;
   }
   return FALSE;
}

BOOL __stdcall CQuery::Match(IDSPRecord *pRec)
{
   BOOL     bKludge  = (m_nField == DS_NAME);
   DBRECENT *pEnt    = pRec->GetRecordList();

   while(pEnt) {
      /* Search Name and Alias                                                */
      if(TRUE == bKludge) {
         if(pEnt->nName == m_nField || pEnt->nName == DS_EMAILALIAS) {
            if(TRUE == FuzzyCompare((LPSTR)pEnt->data))
               return TRUE;
         }
      }
      /* Search what the caller really wants                                  */
      else {
         if(pEnt->nName == m_nField) {
            if(TRUE == FuzzyCompare((LPSTR)pEnt->data))
               return TRUE;
         }
      }

      pEnt = pEnt->pNext;
   }
   return FALSE;
}

QUERY_STATUS __stdcall CQuery::GetRecord(IDSPRecord **ppRecord)
{
   if(m_nLastError == QUERY_OK) {
      DWORD    dwCount  = m_pRecordList->Count();
      DWORD    i;

      if(QUERY_DONE == m_nLastError)
         return m_nLastError;

      if(m_pRE == (LPRECENT) m_pRecordList->Head()) {
         m_nLastError= QUERY_DONE;
         return m_nLastError;
      }

      if(!m_pRE)
         m_pRE = (LPRECENT) m_pRecordList->Head();

      /* Walk the list looking for matches                                    */
      for(i=0;i<dwCount;i++) {
         if(TRUE == Match(m_pRE->pRecord)) {
            m_pRE->pRecord->AddRef();
            *ppRecord   = static_cast<IDSPRecord *>(m_pRE->pRecord);
            m_nLastError= QUERY_OK;
            m_pRE       = (LPRECENT) ILIST_ITEMNEXT(m_pRE);
            goto CQ_GR_DONE;
         }
         m_pRE = (LPRECENT) ILIST_ITEMNEXT(m_pRE);

         if(m_pRE == (LPRECENT) m_pRecordList->Head()) {
            m_nLastError= QUERY_DONE;
            break;
         }
      }
      m_nLastError = QUERY_DONE;
   }

CQ_GR_DONE:
   return m_nLastError;
}

void __stdcall CQuery::Cancel()
{
   m_bCancled = TRUE;
   m_pSched->Cancel();
}

QUERY_STATUS __stdcall CQuery::GetLastError()
{
   return m_nLastError;
}

HRESULT __stdcall CQuery::GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen)
{
   int nID = IDS_QUERY_OK;

   switch(nCode) {
      case QUERY_OK:             nID = IDS_QUERY_OK;           break;
      case QUERY_FAILED:         nID = IDS_QUERY_FAILED;       break;
      case QUERY_WOULDBLOCK:     nID = IDS_QUERY_WOULDBLOCK;   break;
      case QUERY_DONE:           nID = IDS_QUERY_DONE;         break;
      case QUERY_CANCELLED:      nID = IDS_QUERY_CANCELLED;    break;
      case QUERY_EBADFORMAT:     nID = IDS_QUERY_EBADFORMAT;   break;
      case QUERY_ECONNECT:       nID = IDS_QUERY_ECONNECT;     break;
      case QUERY_EADDRESS:       nID = IDS_QUERY_EADDRESS;     break;
      case QUERY_EREAD:          nID = IDS_QUERY_EREAD;        break;
      case QUERY_EWRITE:         nID = IDS_QUERY_EWRITE;       break;
      case QUERY_EOUTOFOMEMORY:  nID = IDS_QUERY_EOUTOFMEMORY; break;
      case QUERY_EPROTOCOL:      nID = IDS_QUERY_EPROTOCOL;    break;
      case QUERY_EEXISTS:        nID = IDS_QUERY_EEXISTS;      break;
      case QUERY_ENOMATCH:       nID = IDS_QUERY_ENOMATCH;     break;
   }
   ::LoadString(CFactory::s_hModule,nID,pszBuffer,nlen);
   return S_OK;
}





