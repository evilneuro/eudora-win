/******************************************************************************/
/*																										*/
/*	Name		:	POST.CPP			  																*/
/* Date     :  9/18/1997                                                      */
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

/*	Desc.		:	IDSPost interface implementation          							*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "ds.h"
#include "resource.h"


extern long g_cComponents;

HRESULT	__stdcall CPost::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IDirServ)
		*ppv = static_cast<IDSPost*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CPost::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CPost::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

CPost::CPost() : m_cRef(1)
{
   m_pSched    = NULL;
   m_pPost     = NULL;
	InterlockedIncrement(&g_cComponents);
}

CPost::~CPost()
{
   if(m_pSched) {
      m_pSched->Cancel();
      m_pSched->Release();
   }

   if(m_pPost) {
      m_pPost->Cancel();
      m_pPost->Release();
   }

   m_pPost     = NULL;
   m_pSched    = NULL;
	InterlockedDecrement(&g_cComponents);
}

/* Static */
HRESULT CPost::CreateInstance(IUnknown **pUnk)
{
	CPost *pPost = DEBUG_NEW_NOTHROW CPost;
	
	if(!pPost) {
		*pUnk = NULL;
		return E_OUTOFMEMORY;
	}
	
	*pUnk = static_cast<IUnknown *>(pPost);
	return S_OK;
}

/* Static */
void CPost::StartCB(LPVOID pCtx)
{
   CPost *p = (CPost *) pCtx;

   /* We've started, cll the user supplied callback and lets get on with it!  */
   p->m_pSched->Schedule(p->m_pStartCB,p->m_pStartData);
}

HRESULT __stdcall CPost::Initialize(IDSDatabase *pDatabase,DWORD dwFlags)
{
   HRESULT  hErr = E_FAIL;

   /* Does this object support posting????                                    */
   if((pDatabase->GetProtocolFlags() & PROTO_WRITEABLE)) {
      /* Create a scheduler object                                            */
      if(SUCCEEDED(hErr = CoCreateInstance(CLSID_ISchedule,NULL,CLSCTX_INPROC_SERVER,IID_ISchedule,(void **) &m_pSched))) {
         /* Now recover the Protocols interface pointer and create the        */
         /* IDSPPost object we will be using.                                 */
         IDSPConfig  *pConfig = reinterpret_cast<CDSDatabase *>(pDatabase)->GetWrappedInterface();
         if(SUCCEEDED(hErr = pConfig->CreateObject(IID_IDSPPost,(void **)&m_pPost))) {
            m_dwFlags   = dwFlags;
            return m_pPost->Initialize(POST_INIT_NOFLAGS);
         }
      }
   }
   return hErr;
}

void __stdcall CPost::Start(DBQUERYCB pCB,LPVOID pCtx)
{
   m_pStartCB  = pCB;
   m_pStartData= pCtx;
   m_pPost->Start(CPost::StartCB,(LPVOID)this);
}

void __stdcall CPost::Postable(DBQUERYCB pCB,LPVOID pCtx)
{
   m_pStartCB  = pCB;
   m_pStartData= pCtx;
   m_pPost->Postable(CPost::StartCB,(LPVOID)this);
}

QUERY_STATUS __stdcall CPost::PostRecord(IDSPRecord *pRecord,DWORD dwFlags)
{
   return m_pPost->PostRecord(pRecord,dwFlags);
}

void __stdcall CPost::Cancel()
{
   m_pPost->Cancel();
   m_pSched->Cancel();
}

QUERY_STATUS __stdcall CPost::GetLastError()
{
   return m_pPost->GetLastError();
}

HRESULT __stdcall CPost::GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen)
{
   return m_pPost->GetErrorString(nCode,pszBuffer,nlen);
}




