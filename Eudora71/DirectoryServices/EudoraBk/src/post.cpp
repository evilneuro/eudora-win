/******************************************************************************/
/*																										*/
/*	Name		:	POST.CPP		  																   */
/* Date     :  9/19/1997                                                      */
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

/*	Desc.		:	IDSPPost interface implementation										*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4201 4514 4706)
#include <windows.h>
#pragma warning(default : 4201)
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"
#include "dspapi.h"
#include "post.h"


extern long g_cComponents;

/* Static */
HRESULT __stdcall CDSPPost::CreateInstance(IUnknown **pUnk,CDatabase *pServer)
{
	CDSPPost *pPost = DEBUG_NEW_NOTHROW CDSPPost(pServer);
	
	if(!pPost) {
		*pUnk = NULL;
		return E_OUTOFMEMORY;
	}
        fBaptizeBlockMT(pPost, "EudoraBk-CDSPPost::CreateInstance);
	
	*pUnk = static_cast<IUnknown *>(pPost);
	return S_OK;
}

HRESULT	__stdcall CDSPPost::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)         *ppv = static_cast<IUnknown*>(this);
	else if (iid == IID_IDSPPost)    *ppv = static_cast<IDSPPost*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CDSPPost::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDSPPost::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

CDSPPost::CDSPPost(CDatabase *pServer) : m_cRef(1)
{
   m_pSched    = NULL;
   m_hWrite    = NULL;
   m_bCanceled = FALSE;
   m_nLastError= QUERY_OK;
   m_pStartCB  = NULL;
   m_pStartData= NULL;
   m_pDatabase   = pServer;
   m_pDatabase->AddRef();
	InterlockedIncrement(&g_cComponents);
}

CDSPPost::~CDSPPost()
{
   if(m_hWrite) {
      fclose(m_hWrite);
      if(m_bCanceled == TRUE) Revert();
      else                    Cleanup();
   }

   if(m_pSched) {
      m_pSched->Cancel();
      m_pSched->Release();
   }

   if(m_pDatabase)     m_pDatabase->Release();

   m_pDatabase   = NULL;
   m_pSched    = NULL;
   m_hWrite    = NULL;
	InterlockedDecrement(&g_cComponents);
}

HRESULT __stdcall CDSPPost::Initialize(DWORD dwFlags)
{
   HRESULT  hErr;

   m_dwFlags = dwFlags;

   if(SUCCEEDED(hErr = CoCreateInstance(CLSID_ISchedule,NULL,CLSCTX_INPROC_SERVER,IID_ISchedule,(void **) &m_pSched))) {
      if(SUCCEEDED(hErr = m_pDatabase->BuildLineTable(&m_pLineTable))) {
         /* Backup the file and open it in write mode                        */
         CalcBackupName();
         hErr = PrepareForWrite();
      }
   }
   return hErr;
}

/* Protected */
void __stdcall CDSPPost::CalcBackupName()
{
   LPSTR pEnd;
   LPSTR pStart;

   lstrcpy(m_szBkName,m_pDatabase->GetPath());
   /* First find the filename                                                 */
   if(NULL != (pEnd = strrchr(m_szBkName,'\\'))) {
      /* Now trim the extension IF there is one                               */
      pStart = (pEnd+1);
      if(NULL != (pEnd = strrchr(pStart,'.'))) {
         *pEnd = 0;
      }
      /* And append the backup extension                                      */
      lstrcat(m_szBkName,".bak");
   }
}

/* Protected */
HRESULT __stdcall CDSPPost::PrepareForWrite()
{
   m_nLastError = QUERY_FAILED;
   /* Delete Any previous backups                                             */
   DeleteFile(m_szBkName);

   /* Backup the original file                                                */
   if(FALSE == MoveFile(m_pDatabase->GetPath(),m_szBkName))
      return E_FAIL;

   if(NULL != (m_hWrite = fopen(m_pDatabase->GetPath(),"w"))) {
      m_nLastError = QUERY_OK;
      return S_OK;
   }
   m_bCanceled = TRUE;/* The backupfile will revert back to the original file */
                     /* in our destructor                                     */
   return E_FAIL;
}

/* Protected */
HRESULT __stdcall CDSPPost::Revert()
{
   DeleteFile(m_pDatabase->GetPath());
   if(TRUE == MoveFile(m_szBkName,m_pDatabase->GetPath()))
      return S_OK;

   return E_FAIL;
}

void __stdcall CDSPPost::Cleanup()
{
   DeleteFile(m_szBkName);
}

/* Static */
void CDSPPost::StartCB(LPVOID pCtx)
{
   CDSPPost *pPost = (CDSPPost *) pCtx;

   pPost->m_pStartCB(pPost->m_pStartData);
}

void __stdcall CDSPPost::Start(DBQUERYCB pCB,LPVOID pCtx)
{
   m_pStartCB     = pCB;
   m_pStartData   = pCtx;
   m_nLastError   = QUERY_OK;
   m_pSched->Schedule(CDSPPost::StartCB,(LPVOID)this);
}

void __stdcall CDSPPost::Postable(DBQUERYCB pCB,LPVOID pCtx)
{
   /* we're always postable!                                                  */
   m_pStartCB     = pCB;
   m_pStartData   = pCtx;
   m_nLastError   = QUERY_OK;
   m_pSched->Schedule(CDSPPost::StartCB,(LPVOID)this);
}

QUERY_STATUS __stdcall CDSPPost::PostRecord(IDSPRecord *pRec,DWORD dwFlags)
{
   char  szName[512] = {0};

   if(FALSE == m_pDatabase->ExtractField(DS_NAME,pRec,szName,sizeof(szName))) {
      m_nLastError = QUERY_EBADFORMAT;
      return QUERY_EBADFORMAT;
   }

   if(dwFlags & POST_ADD) {
      if(NULL == m_pDatabase->Find(m_pLineTable,ALIAS_STR,szName)) {
         m_nLastError = m_pDatabase->Add(m_pLineTable,pRec);
         return m_nLastError;
      }
      m_nLastError = QUERY_EEXISTS;
      return QUERY_EEXISTS;
   }
   else if(dwFlags & POST_UPDATE) {
      LPLTBL pTbl;

      if(NULL != (pTbl = m_pDatabase->Find(m_pLineTable,ALIAS_STR,szName))) {
         m_nLastError = m_pDatabase->Update(m_pLineTable,pRec,szName,pTbl);
         return m_nLastError;
      }
      m_nLastError = QUERY_ENOMATCH;
      return QUERY_ENOMATCH;
   }
   else if(dwFlags & POST_DELETE) {
      LPLTBL pTbl;

      if(NULL != (pTbl = m_pDatabase->Find(m_pLineTable,ALIAS_STR,szName))) {
         m_nLastError = m_pDatabase->Delete(m_pLineTable,szName,pTbl);
         return m_nLastError;
      }
      m_nLastError = QUERY_ENOMATCH;
      return QUERY_ENOMATCH;
   }
   m_nLastError = QUERY_EBADFORMAT;
   return QUERY_EBADFORMAT;
}

void __stdcall CDSPPost::Cancel()
{
   m_pSched->Cancel();
   m_bCanceled = TRUE;
}

QUERY_STATUS __stdcall CDSPPost::GetLastError()
{
   return m_nLastError;
}

HRESULT __stdcall CDSPPost::GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen)
{
   return E_FAIL;
}


