/******************************************************************************/
/*																										*/
/*	Name		:	QUERY.CPP  		  																*/
/* Date     :  8/7/1997		                                                   */
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

/*	Desc.		:	Directory Services Query Object implementation						*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "ISched.h"
#include "factory.h"
#include "record.h"
#include "query.h"
#include "resource.h"
#include "QCUtils.h"


extern long g_cComponents;

//IUnknown * CQuery::CreateInstance()
//{
//	CDirServ *pDS = new CDirServ;
//	
//	if(pDS) {
//		return(static_cast<IUnknown *>(pDS));
//	}
//	return(NULL);
//}

CQuery::CQuery() : m_cRef(1)
{
    m_pSched				= NULL;
    m_pRecList        = NULL;
    m_pDBList 	      = NULL;
    m_pDBFailedList 	= NULL;
    m_nDBCount			= 0;
    m_pszSearch			= NULL;
    m_nField				= DS_UNKNOWN;
    m_dwFlags			= 0;
    m_pStartCB			= NULL;
    m_pStartData		= NULL;
    m_pWaitCB			= NULL;
    m_pWaitData			= NULL;
    m_bParentStarted	= FALSE;
    m_bCallerWaiting	= FALSE;
    m_LastError			= QUERY_OK;
    InterlockedIncrement(&g_cComponents);
}

/* Static */
HRESULT CQuery::CreateInstance(IUnknown **pUnk)
{
    CQuery *pQuery = DEBUG_NEW_NOTHROW CQuery;
	
    if(!pQuery) {
		*pUnk = NULL;
		return E_OUTOFMEMORY;
    }
	
    *pUnk = static_cast<IUnknown *>(pQuery);
    return S_OK;
}

CQuery::~CQuery()
{
    if(m_pSched) {
		m_pSched->Cancel();
		m_pSched->Release();
    }
    if(m_pDBList) 			m_pDBList->Release();
    if(m_pDBFailedList) 	m_pDBFailedList->Release();
    if(m_pRecList)    	m_pRecList->Release();

	delete [] m_pszSearch;
	
    m_pRecList	= NULL;
    m_pDBList 	= NULL;
    m_pszSearch	= NULL;
    m_pSched		= NULL;
    InterlockedDecrement(&g_cComponents);
}

HRESULT	__stdcall CQuery::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == IID_IUnknown)
	*ppv = static_cast<IUnknown*>(this);
    else
	if (iid == IID_IDirServ)
	    *ppv = static_cast<IDSQuery*>(this);
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

/* Static */
void CQuery::QentFreeCB(LPITEM pItem, LPVOID /* pUser */)
{
    LPQENT pQent = (LPQENT) pItem;
	
    if(pQent) {
		if(pQent->pQuery) {
			pQent->pQuery->Cancel();
			pQent->pQuery->Release();
			pQent->pQuery = NULL;
		}
			
		if(pQent->pDatabase) {
			pQent->pDatabase->Release();
			pQent->pDatabase= NULL;
		}
		
		delete pQent;
    }
}

void CQuery::RecEntFreeCB(LPITEM pItem, LPVOID /* pUser */)
{
    LPRECENT	pRec = (LPRECENT) pItem;
	
    if(pRec) {
		if(pRec->pRec) pRec->pRec->Release();
		pRec->pRec = NULL;

		delete pRec;
    }
}

HRESULT __stdcall CQuery::AddDatabase(IDSDatabase *pDatabase)
{
    HRESULT		hErr;
    IDSPConfig	*pConfig = NULL;
	
    /* Make sure we have been initiliazed!!!												*/
    if(!m_pSched || !m_pDBList)
		return E_FAIL;
	
    /* Create the wrapper for this query object											*/
	LPQENT		pQent 	= DEBUG_NEW_NOTHROW QENT;
	if(!pQent)
		return E_OUTOFMEMORY;
	
    /* Stuff the required fields																*/
    memset(pQent,0,sizeof(QENT));
    pQent->pcQuery		= this;
    pQent->pDatabase 	= static_cast<CDSDatabase *>(pDatabase);
    pConfig 				= pQent->pDatabase->GetWrappedInterface();
	
    /* Don't forget to keep this database entry around!								*/
    pQent->pDatabase->AddRef();
	
    /* Create a protocol level IDSPQuery object, initialize it and add it to 	*/
    /* out list of query objects for this query											*/
    if(SUCCEEDED(hErr = pConfig->CreateObject(IID_IDSPQuery,(void **) &(pQent->pQuery)))) {
	if(SUCCEEDED(hErr = pQent->pQuery->InitQuery(m_pszSearch,m_dwFlags,m_nField))) {
	    m_pDBList->Attach(pQent);
	}
    }
    return hErr;
}

HRESULT __stdcall CQuery::Initialize(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField)
{
    HRESULT	hErr;

    if(NULL == (m_pszSearch = SafeStrdupMT(pszSearch)))
		return E_OUTOFMEMORY;
	
    /* Create our query list object and our scheduling object						*/
    if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pDBList))) {
	if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pDBFailedList))) {
	    if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pRecList))) {
		m_pDBList->Initialize(CQuery::QentFreeCB,NULL);
		m_pDBFailedList->Initialize(CQuery::QentFreeCB,NULL);
		m_pRecList->Initialize(CQuery::RecEntFreeCB,NULL);
		if(SUCCEEDED(hErr = CoCreateInstance(CLSID_ISchedule,NULL,CLSCTX_INPROC_SERVER,IID_ISchedule,(void **) &m_pSched))) {
		    m_nField		= nField;
		    m_dwFlags	= dwFlags;
		    return S_OK;
		}
	    }
	}
    }
    return hErr;
}

HRESULT __stdcall CQuery::SetSearchBase(LPSTR /* pszSearchBase */)
{
    return E_NOTIMPL;
}

/* Protected */
void __stdcall CQuery::PushErrorRecord(QUERY_STATUS qErr,LPQENT pQent)
{
	LPRECENT	pRec = DEBUG_NEW_NOTHROW RECENT;
    char		szBuf[512];
	
	szBuf[0] = '\0';
    pQent->pQuery->GetErrorString(qErr,szBuf,sizeof(szBuf));
	
	if(pRec) {
		memset(pRec,0,sizeof(RECENT));
		if(NULL != (pRec->pRec = DEBUG_NEW_NOTHROW CErrorRecord(qErr,szBuf))) {
			m_pRecList->Attach(pRec);
		}
    }
}

/* static */
void CQuery::StartCB(LPVOID pCtx)
{
    LPQENT				pQent		= (LPQENT) pCtx;
    register CQuery	*pcQuery	= pQent->pcQuery;
	
    /* If the start failed for this query object, push an error record			*/
    if(QUERY_OK != (pcQuery->m_LastError = pQent->pQuery->GetLastError())) {
	LPQENT	pTmp = NULL;

	pcQuery->PushErrorRecord(pcQuery->m_LastError,pQent);
	/* NOTE: We need to keep the query object around until we die, so move	*/
	/* 		it to another list.  The list and its contents are toasted in	*/
	/*			our destructor.																*/
	if(NULL != (pTmp = (LPQENT)pcQuery->m_pDBList->Unlink(pQent))) {
	    pcQuery->m_pDBFailedList->Attach(pTmp);
	}
	pQent = NULL;
    }
    else {
	pQent->nState = QSTATE_STARTED;
    }

    /* Call the parent on the first run through											*/
    if(FALSE == pcQuery->m_bParentStarted)
	pcQuery->m_pSched->Schedule(pcQuery->m_pStartCB,pcQuery->m_pStartData);
    else {
	if(TRUE == pcQuery->m_bCallerWaiting && pQent) {
	    pQent->pQuery->WaitForRecord(CQuery::WaitCB,(void *) pQent);
	    pQent->nState = QSTATE_WAITING;
	}
    }
}

void __stdcall CQuery::Start(DBQUERYCB pCB,LPVOID pCtx)
{
    int 		i;
    LPQENT	pQent 	= NULL;
    DWORD		dwCount	= 0;
	
    m_pStartCB 		= pCB;
    m_pStartData	= pCtx;
	
    /* Get count of query objects to start													*/
    dwCount 	= m_pDBList->Count();
    pQent		= (LPQENT) m_pDBList->Head();
    for(i=0;i<(int) dwCount;i++) {
		pQent->nState = QSTATE_STARTING;
		pQent->pQuery->Start(CQuery::StartCB,(void *)pQent);
		pQent = (LPQENT) ILIST_ITEMNEXT(pQent);
    }
    return;
}

/* static */
void CQuery::WaitCB(LPVOID pCtx)
{
    LPQENT			pQent	= (LPQENT) pCtx;
    IDSPRecord		*pRec = NULL;
    QUERY_STATUS	qErr;
	
    /* Did we get something???																*/
    pQent->nState = QSTATE_IDLE;
    if(QUERY_OK == (qErr = pQent->pQuery->GetRecord(&pRec))) {
		LPRECENT	pRecEnt = DEBUG_NEW_NOTHROW RECENT;
		
		if(pRecEnt) {
			memset(pRecEnt,0,sizeof(RECENT));
			pRecEnt->pRec = pRec;
			pQent->pcQuery->m_pRecList->Attach(pRecEnt);
			if(TRUE == pQent->pcQuery->m_bCallerWaiting) {
				pQent->pcQuery->m_bCallerWaiting = FALSE;
				pQent->pcQuery->m_pSched->Schedule(pQent->pcQuery->m_pWaitCB,pQent->pcQuery->m_pWaitData);
			}
		}
    }
    /* Either the query is done, or there was an error								*/
    else {
	CQuery	*pQuery = pQent->pcQuery;
		
	/* Remove the query object from our list so we won't reschedule it	*/
	pQuery->m_pDBList->DeleteItem(pQent);
		
	/* If the caller is waiting and all the queries are done, call 'em	*/
	if(TRUE == pQuery->m_bCallerWaiting && 0 == pQuery->m_pDBList->Count()) {
	    pQuery->m_bCallerWaiting = FALSE;
	    pQuery->m_pSched->Schedule(pQuery->m_pWaitCB,pQuery->m_pWaitData);
	}
    }
}

void __stdcall CQuery::WaitForRecord(DBQUERYCB pCB,LPVOID pCtx)
{
    LPQENT	pQent 	= NULL;
    DWORD		dwCount	= 0;
    DWORD		i;
	
    m_pWaitCB 	= pCB;
    m_pWaitData = pCtx;
	
    /* If we have records in our queue, or all the Queries are done dispatch 	*/
    /* the caller CB and return.  As Queries finish, the objects are Released	*/
    /* and removed from our list.  So if the list is empty, we're done bud!		*/
    if(0 != m_pRecList->Count() || 0 == m_pDBList->Count()) {
	m_pSched->Schedule(m_pWaitCB,m_pWaitData);
	return;
    }
	
    /* Otherwise, we need more data!															*/
    pQent 	= (LPQENT) m_pDBList->Head();
    dwCount	= m_pDBList->Count();
    for(i=0;i<dwCount;i++) {
	/* If the query has been started and is not currently waiting				*/
	if(QSTATE_STARTING != pQent->nState && QSTATE_WAITING != pQent->nState) {
	    pQent->pQuery->WaitForRecord(CQuery::WaitCB,(void *) pQent);
	    pQent->nState = QSTATE_WAITING;
	}
	pQent = (LPQENT) ILIST_ITEMNEXT(pQent);
    }
	
    m_bCallerWaiting = TRUE;
	
    return;
}

QUERY_STATUS __stdcall CQuery::GetRecord(IDSPRecord **ppRecord)
{
    LPRECENT	pRec = NULL;
	
    /* If there is a record to return, give it to them									*/
    if(NULL != (pRec = (LPRECENT) m_pRecList->Pop())) {
		pRec->pRec->AddRef();
		*ppRecord = pRec->pRec;
		RecEntFreeCB((LPITEM)pRec,NULL);
		return QUERY_OK;
    }
	
    /* If there are no more query objects, we are done...								*/
    if(!m_pDBList->Count())
	return QUERY_DONE;
	
    /* Otherwise we are waiting for more data...											*/
    return QUERY_WOULDBLOCK;
}

void __stdcall CQuery::Cancel()
{
    DWORD		dwCount 	= 0;
    LPQENT	pQent 	= NULL;
    DWORD		i;		  	
	
    /* Send Cacnel() to all open query objects											*/
    dwCount 	= m_pDBList->Count();
    pQent 	= (LPQENT) m_pDBList->Head();
    for(i=0;i<dwCount;i++) {
	pQent->pQuery->Cancel();
	pQent = (LPQENT) ILIST_ITEMNEXT(pQent);
    }
    return;
}

QUERY_STATUS __stdcall CQuery::GetLastError()
{
    return m_LastError;
}

HRESULT __stdcall CQuery::GetErrorString(QUERY_STATUS /* nCode */, LPSTR /* pszBuffer */, int /* nlen */)
{
    return E_NOTIMPL;
}

