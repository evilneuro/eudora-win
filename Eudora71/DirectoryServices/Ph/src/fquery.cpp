/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					fquery.cpp
Description:		Implementation of finger query object
Date:						8/08/97
Version:  			1.0 
Module:					PH.DLL (PH/Finger protocol directory service object)
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
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <objbase.h>
#include <assert.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "ph.h"
#include "trace.h"
#include "fquery.h"
#include "QCUtils.h"

extern long g_cComponents;				// Count of active components

#define WRITABLE(fn)		m_pISock->Writable(ISOCKCALLBACK(fn), (LPVOID)this)
#define READABLE(fn)		m_pISock->Readable(ISOCKCALLBACK(fn), (LPVOID)this)

///////////////////////////////////////////////////////////////////////////////
// CFingerQuery class


CFingerQuery::CFingerQuery(IUnknown *punkParent) : m_cRef(1)
{
	m_bConnected			= 0;
	m_pPrivate 				= NULL;
	m_pISock					= NULL;
	m_pISchedule			= NULL;
	m_pszSearch 			= NULL;
	m_dwFlags 				= 0;
	m_nField 					= 0;
	m_nIndex					= -1;
  m_pfnQueryCB 			= NULL;
	m_bQueryIssued 		= 0;
	m_pRecord 				= NULL;
	m_nQueryStatus		= QUERY_OK;
	m_pReadPos				= NULL;
	m_nBytesToRead		= 0;
	m_pParsePos				= NULL;
	m_pWritePos				= NULL;
	m_nBytesToWrite		= 0;
	m_pszDatabaseID		= NULL;
#ifndef OLD
  m_bDataAvailable  = 0;
#endif

	// get the private PH interface
	HRESULT hr = punkParent->QueryInterface(IID_IPrivateConfig, (LPVOID*)&m_pPrivate);
	assert(SUCCEEDED(hr));

	InterlockedIncrement(&g_cComponents);
}

CFingerQuery::~CFingerQuery()
{
  if (m_pszSearch)
		delete [] m_pszSearch;

	if (m_pPrivate)
		m_pPrivate->Release();

	if (m_pISock) {
		m_pISock->Cancel();
		m_pISock->Release();
	}

	if (m_pISchedule) {
		m_pISchedule->Cancel();
		m_pISchedule->Release();
	}

	if (m_pszDatabaseID)
		delete [] m_pszDatabaseID;

	// if there is a leftover record, release it...
	if (m_pRecord)
		m_pRecord->Release();

	InterlockedDecrement(&g_cComponents);
}


// IUnknown
HRESULT __stdcall CFingerQuery::QueryInterface(const IID& iid, void** ppv)
{
	if (iid == IID_IUnknown)
    *ppv = (IUnknown*)this;
	else
	if (iid == IID_IDSPQuery)
		*ppv = (IDSPQuery*)this;
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	((IUnknown*)*ppv)->AddRef();

	return S_OK;
}


ULONG __stdcall CFingerQuery::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}


ULONG __stdcall CFingerQuery::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}

///////////////////////////////////////////////////////////////////////////////
// Non-interface methods

HRESULT CFingerQuery::CreateSocket()
{
	HRESULT hr;
	 
	if (SUCCEEDED(hr = CoCreateInstance(CLSID_ISock, NULL, CLSCTX_INPROC_SERVER, IID_ISock, (LPVOID *)&m_pISock)))
		if (SUCCEEDED(hr = CoCreateInstance(CLSID_ISchedule, NULL, CLSCTX_INPROC_SERVER, IID_ISchedule, (LPVOID *)&m_pISchedule)))
			return hr;

	m_nQueryStatus = QUERY_EPROTOCOL;
	return hr;
}


BOOL CFingerQuery::Connect()
{
	m_bConnected = 0;

	assert(m_pISock);
	assert(m_pPrivate);

	if (SUCCEEDED(m_pISock->Initialize(m_pPrivate->GetHostName(), m_pPrivate->GetPortNumber(), ISOCK_FLAG_NOFLAGS)))
		if (SUCCEEDED(m_pISock->Connect(ISOCKCALLBACK(ConnectCB), (LPVOID)this)))
			return 1;
	
	m_nQueryStatus = QUERY_ECONNECT;
	ScheduleCallback();
	return 0;
}


void CFingerQuery::ConnectCB()
{
	if (m_nQueryStatus == QUERY_CANCELLED)
		return;
	
	int nSocketError;
	m_pISock->GetSocketError(&nSocketError);
	if (nSocketError)
		m_nQueryStatus = QUERY_ECONNECT;
	else
		m_bConnected = 1;

	ScheduleCallback();
}


void CFingerQuery::StartWrite()
{
	m_pWritePos = m_pszSearch;
	m_nBytesToWrite = strlen(m_pszSearch);
	WRITABLE(WriteCB);
}

void CFingerQuery::StartRead()
{
	m_pReadPos = m_pParsePos = m_achReadBuf;
	m_nBytesToRead = sizeof(m_achReadBuf)-2;
	READABLE(ReadCB);
}

void CFingerQuery::WriteCB()
{
	if (m_nQueryStatus == QUERY_CANCELLED)
		return;
	
	int nBytesWritten = 0;
	if (FAILED(m_pISock->Write(m_pWritePos, m_nBytesToWrite, &nBytesWritten))) {
		SetStatus(QUERY_EWRITE);
	}
	else {
		m_nBytesToWrite -= nBytesWritten;
		m_pWritePos += nBytesWritten;
		if (m_nBytesToWrite > 0)
			WRITABLE(WriteCB);
		else {
			m_bQueryIssued = 1;
			StartRead();
		}
	}
}


void CFingerQuery::ReadCB()
{
	int nBytesRead = 0;
	int nSocketError = 0;

	if (m_nQueryStatus == QUERY_CANCELLED)
		return;

	if (FAILED(m_pISock->Read(m_pReadPos, m_nBytesToRead, &nBytesRead))) {
		m_pISock->GetSocketError(&nSocketError);
		if (nSocketError == WSAEWOULDBLOCK) {
			READABLE(ReadCB);
    }
		else
			SetStatus(QUERY_EREAD);
	}
	else {
#ifdef OLD
		if (nBytesRead == 0) {
			SetStatus(QUERY_DONE);
#else
    if (nBytesRead == 0) {
      SetStatus(QUERY_OK);
      ScheduleCallback();
#endif
			return;
		}

		m_nBytesToRead -= nBytesRead;
		m_pReadPos += nBytesRead;
#ifdef OLD
		if (IsCompleteRecord())
			ScheduleCallback();
    else {
			READABLE(ReadCB);
    }
#else
    if (!IsCompleteRecord()) {
     	// move up the contents of the read buffer when the
    	// free space at the top is greater than our threshold
    	if ((m_pParsePos - m_achReadBuf) > ADJUST_THRESHOLD)
    		AdjustReadBuf();
      READABLE(ReadCB);
    }
#endif
	}
}


void __stdcall CFingerQuery::WaitForRecord(DBQUERYCB pCB, LPVOID pCtx)
{
	m_pfnQueryCB 	= pCB;
	m_pContext 		= pCtx;
	
  // put this in to prevent WaitForRecord() to be executed 
  // without an intervening GetRecord() call
  if (m_pRecord)
    return;

	if (m_nQueryStatus == QUERY_CANCELLED)
		return;

	if (m_nQueryStatus == QUERY_DONE) {
		ScheduleCallback();
		return;
	}

	// create a new record
	CreateRecord();

	if (!m_bQueryIssued)
		StartWrite();							// write the query string (first time through only)
	else
#ifdef OLD
	if (IsCompleteRecord())			// if a complete record is already available in our buffer
		ScheduleCallback();				// schedule a notification callback
  else {
		READABLE(ReadCB);					// otherwise, schedule another winsock read
  }
#else
  if (!IsCompleteRecord()) {
    // move up the contents of the read buffer when the
  	// free space at the top is greater than our threshold
	  if ((m_pParsePos - m_achReadBuf) > ADJUST_THRESHOLD)
		  AdjustReadBuf();
    READABLE(ReadCB);
  }
#endif

}


QUERY_STATUS __stdcall CFingerQuery::GetRecord(IDSPRecord **ppRecord)
{
	if ((m_nQueryStatus == QUERY_CANCELLED) 	|| 
			(m_nQueryStatus == QUERY_WOULDBLOCK) 	|| 
			(m_nQueryStatus == QUERY_DONE))
		return m_nQueryStatus;

	// return the IDSPRecord interface
	if (ppRecord && m_pRecord) {
		m_pRecord->QueryInterface(IID_IDSPRecord, (void**)ppRecord);
		m_pRecord->Release();		// release our pointer to object
		m_pRecord = NULL;
#ifndef OLD
    m_nQueryStatus = QUERY_DONE;
#endif
	}

	// move up the contents of the read buffer when the
	// free space at the top is greater than our threshold
	if ((m_pParsePos - m_achReadBuf) > ADJUST_THRESHOLD)
		AdjustReadBuf();

	return (*ppRecord) ? QUERY_OK : QUERY_FAILED;
}


BOOL CFingerQuery::CreateRecord()
{
	if (!m_pRecord) {
		m_pRecord = DEBUG_NEW_NOTHROW CPHRecord();
		if (!m_pRecord) {
			m_nQueryStatus = QUERY_EOUTOFOMEMORY;
			return 0;
		}

		m_pRecord->SetDatabaseID(m_pszDatabaseID);
		m_nQueryStatus = QUERY_WOULDBLOCK;
	}
	return 1;
}


BOOL CFingerQuery::IsCompleteRecord()
{
	LPSTR pEol;

 	// create a new record
	if (!CreateRecord())
		return 1;

	if (m_nQueryStatus == QUERY_DONE ||
			m_nQueryStatus == QUERY_CANCELLED)
		return 1;

	*m_pReadPos = 0;	// NULL terminate the buffer so we can use strxxx functions

#ifdef OLD
	// find the end-of-line
	if (pEol = strchr(m_pParsePos, '\n')) {
		LPSTR pLine = m_pParsePos;
		m_pParsePos = pEol+1;
 		*pEol = 0;
		if (NULL != m_pRecord->AddStringRecEnt(pLine, DS_UNKNOWN))
			m_nQueryStatus = QUERY_OK;
		else
			m_nQueryStatus = QUERY_EOUTOFOMEMORY;
		return 1;
	}
#else
	// find the end-of-line
	while (pEol = strchr(m_pParsePos, '\n')) {
		LPSTR pLine = m_pParsePos;
		m_pParsePos = pEol+1;
		*pEol = 0;
		if (!m_pRecord->AddStringRecEnt(pLine, DS_UNKNOWN))
			m_nQueryStatus = QUERY_EOUTOFOMEMORY;
    else
      m_bDataAvailable = 1;
	}
#endif

	return 0;
}


void CFingerQuery::AdjustReadBuf()
{
	// move memory from m_pParsePos to m_achReadBuf, count of m_pReadPos - m_pParsePos
	int nDataSize = m_pReadPos - m_pParsePos;
	int nAdjust = m_pParsePos - m_achReadBuf;
	memmove(m_achReadBuf, m_pParsePos, nDataSize);
	m_pParsePos -= nAdjust;
	m_pReadPos -= nAdjust;
	m_nBytesToRead += nAdjust;
}



void CFingerQuery::ScheduleCallback()
{
	m_pISchedule->Schedule(ISOCKCALLBACK(Callback), this);
}


void CFingerQuery::Callback()
{
	assert(m_pfnQueryCB);
  m_pfnQueryCB(m_pContext);
}


void CFingerQuery::SetDatabaseID(LPSTR pszID)
{
	if (m_pszDatabaseID)
		delete [] m_pszDatabaseID;

	m_pszDatabaseID = NULL;

	if (pszID)
		m_pszDatabaseID = SafeStrdupMT(pszID);
}


void CFingerQuery::SetStatus(QUERY_STATUS nStatus)
{
	m_nQueryStatus = nStatus;
	ScheduleCallback();
}


///////////////////////////////////////////////////////////////////////////////
// IDSPQuery

HRESULT __stdcall CFingerQuery::InitQuery(LPSTR pszSearch, DWORD dwFlags, DS_FIELD nField)
{
#if 0
	if (!pszSearch || ((dwFlags & QUERY_FLG_RAW) == 0))
		return E_FAIL;
#else
	if (!pszSearch)
		return E_FAIL;
#endif

	// create the ISock object
	if (FAILED(CreateSocket()))
		return E_FAIL;

	int nLen = strlen(pszSearch);

	m_pszSearch = DEBUG_NEW_NOTHROW char[nLen + 20];
	if (!m_pszSearch) {
		m_nQueryStatus = QUERY_EOUTOFOMEMORY;
		return E_FAIL;
	}

  // 10/28/97 - request long format
  //strcpy(m_pszSearch, "-l ");
	//strcat(m_pszSearch, pszSearch);
	strcpy(m_pszSearch, pszSearch);

  nLen = strlen(m_pszSearch);
	// make sure string has an ending newline
	if (m_pszSearch[nLen-1] != '\n')
		strcat(m_pszSearch, "\r\n");

	if (m_pRecord) {
		m_pRecord->Release();
		m_pRecord = NULL;
	}
#ifndef OLD
  m_bDataAvailable  = 0;
#endif

	m_dwFlags 				= dwFlags;
	m_nField 					= nField;
	m_bQueryIssued 		= 0;
	m_nIndex					= -1;
	m_nQueryStatus 		=	QUERY_OK;

	return S_OK;
}


void __stdcall CFingerQuery::Start(DBQUERYCB pCB, LPVOID pCtx)
{
	m_pfnQueryCB 	= pCB;
	m_pContext 		= pCtx;
	Connect();
}

void __stdcall CFingerQuery::Cancel()
{
	m_nQueryStatus = QUERY_CANCELLED;
	if (m_pISchedule)
		m_pISchedule->Cancel();
	if (m_pISock)
		m_pISock->Cancel();
}

QUERY_STATUS __stdcall CFingerQuery::GetLastError()
{
	return m_nQueryStatus;
}

HRESULT __stdcall CFingerQuery::GetErrorString(QUERY_STATUS /* nCode */, LPSTR /* pszBuffer */, int /* nlen */)
{
	return E_FAIL;
}

HRESULT __stdcall CFingerQuery::SetSearchBase(LPSTR /* pszSearchBase */)
{ 
	return S_OK; 			// not needed for finger
}




