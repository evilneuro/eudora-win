/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					query.cpp
Description:		Implementation of PH directory services object
Date:						8/08/97
Version:  			1.0 
Module:					PH.DLL (PH protocol directory service object)
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
#include "query.h"
#include "ustring.h"
#include "resource.h"
#include "QCUtils.h"


extern long g_cComponents;				// Count of active components

#define WRITABLE(fn)		m_pISock->Writable(ISOCKCALLBACK(fn), (LPVOID)this)
#define READABLE(fn)		m_pISock->Readable(ISOCKCALLBACK(fn), (LPVOID)this)

// smohanty: 04/21/98
#define TEMPBUFSIZE 2048

///////////////////////////////////////////////////////////////////////////////
// Utility functions


// trim trailing chars of string 'psz' if they 
// are contained in the set specified by 'pszCharSet'
void Trim(LPSTR psz, LPSTR pszCharSet)
{
  char szComp[2];
  szComp[1] = 0;
  int len = strlen(psz);
  for(LPSTR p=psz+len-1; len; len--, p--) {
    szComp[0] = *p;
    if (NULL == strpbrk(szComp, pszCharSet))
      break;
    *p = 0;
  }
}

BOOL GetNextToken(LPSTR psz, LPSTR pszBuffer, UINT nBufferLen)
{
  if (!psz || !pszBuffer || nBufferLen < 1)
    return FALSE;
  // strip leading spaces
  for (; *psz == ' '; psz++)
    ;
  // scan for first space
  for (UINT i=0; psz[i] && psz[i] != ' '; i++)
    ;
  if (!i || i > nBufferLen-1)
    return FALSE;
  // copy token to buffer (up to max buffer size)
  strncpy(pszBuffer, psz, i);
  pszBuffer[i] = 0;
  return TRUE;
}

BOOL IsQueryCommand(LPSTR psz)
{
  char buffer[80];
  GetNextToken(psz, buffer, sizeof(buffer));
  return (0 == stricmp(buffer, "query")) || (0 == stricmp(buffer, "ph"));
}


///////////////////////////////////////////////////////////////////////////////
// CPHQuery class


CPHQuery::CPHQuery(IUnknown *punkParent) : m_cRef(1)
{
	m_bConnected			= 0;
	m_pPrivPH 				= NULL;
	m_pISock					= NULL;
	m_pISchedule			= NULL;
	m_pszSearch 			= NULL;
	m_dwFlags 				= 0;
	m_nField 					= 0;
	m_pfnQueryCB 			= NULL;
	m_bLastRecord			= 0;
	m_nQueryStatus		= QUERY_OK;
	m_bQueryIssued 		= 0;
	m_pRecord 				= NULL;
	m_pReadPos				= NULL;
	m_nBytesToRead		= 0;
	m_pParsePos				= NULL;
	m_pWritePos				= NULL;
	m_nBytesToWrite		= 0;
	m_pszDatabaseID		= NULL;
  m_bFoundMatch     = 0;
  m_bDoRetry        = 0;
  m_pszSaveRawQuery = NULL;
  m_bRetrying       = 0;
  // smohanty: Added 04/21/98
  m_nLastIndex = 0;
  m_nLastRCode = 0;
  siteInfoNotImpl = false;

  m_bDoingSiteInfo = 0;
  m_pszTempBuffer  = NULL;

	// get the private PH interface
	HRESULT hr = punkParent->QueryInterface(IID_IPrivateConfig, (LPVOID*)&m_pPrivPH);
	assert(SUCCEEDED(hr));

	InterlockedIncrement(&g_cComponents);
}

CPHQuery::~CPHQuery()
{
	delete [] m_pszSearch;
	delete [] m_pszSaveRawQuery;

	if (m_pPrivPH)
		m_pPrivPH->Release();

	if (m_pISock)
		m_pISock->Release();

	if (m_pISchedule)
		m_pISchedule->Release();

	delete [] m_pszDatabaseID;
	delete [] m_pszTempBuffer;

	// if there is a leftover record, release it...
	if (m_pRecord)
		m_pRecord->Release();

	InterlockedDecrement(&g_cComponents);
}


// IUnknown
HRESULT __stdcall CPHQuery::QueryInterface(const IID& iid, void** ppv)
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


ULONG __stdcall CPHQuery::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}


ULONG __stdcall CPHQuery::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}

///////////////////////////////////////////////////////////////////////////////
// Non-interface methods

// virtualize the socket creation so that we can substitute a CBufferNetIO (or
// other object) for an ISock object...

HRESULT CPHQuery::CreateSocket()
{
	HRESULT hr;
	 
	if (SUCCEEDED(hr = CoCreateInstance(CLSID_ISock, NULL, CLSCTX_INPROC_SERVER, IID_ISock, (LPVOID *)&m_pISock)))
		if (SUCCEEDED(hr = CoCreateInstance(CLSID_ISchedule, NULL, CLSCTX_INPROC_SERVER, IID_ISchedule, (LPVOID *)&m_pISchedule)))
			return hr;

	m_nQueryStatus = QUERY_EPROTOCOL;
	return hr;
}


BOOL CPHQuery::Connect()
{
	m_bConnected = 0;

	assert(m_pISock);

	if (SUCCEEDED(m_pISock->Initialize(m_pPrivPH->GetHostName(), m_pPrivPH->GetPortNumber(), ISOCK_FLAG_NOFLAGS)))
		if (SUCCEEDED(m_pISock->Connect(ISOCKCALLBACK(ConnectCB), (LPVOID)this)))
			return 1;
	
	m_nQueryStatus = QUERY_ECONNECT;
	return 0;
}


void CPHQuery::ConnectCB()
{
	int nSocketError;

	if (m_nQueryStatus == QUERY_CANCELLED)
		return;
	
	m_pISock->GetSocketError(&nSocketError);
	if (!nSocketError)
		m_bConnected = 1;
	else
		m_nQueryStatus = QUERY_ECONNECT;

  if (m_bRetrying) {
    WaitForRecord(m_pfnQueryCB, m_pContext);
    m_bRetrying = 0;
  }
  else
      if (!DoSiteInfo()) // smohanty: 04/21/98
	  ScheduleCallback();
}

// smohanty: 04/21/98
BOOL CPHQuery::DoSiteInfo()
{
    static const char szSiteInfo[] = "siteinfo\n";
    if (NULL == m_pPrivPH->GetMailDomain()) {
	m_bDoingSiteInfo = 1;
	m_pWritePos = (LPSTR)szSiteInfo;
	m_nBytesToWrite = strlen(szSiteInfo);
	WRITABLE(WriteCB);
	return TRUE;
    }
    return FALSE;
}

void CPHQuery::StartWrite()
{
	m_pWritePos = m_pszSearch;
	m_nBytesToWrite = strlen(m_pszSearch);
	WRITABLE(WriteCB);
}

void CPHQuery::StartRead()
{
	m_pReadPos = m_pParsePos = m_achReadBuf;
	m_nBytesToRead = sizeof(m_achReadBuf)-2;
	READABLE(ReadCB);
}


void CPHQuery::WriteCB()
{
	if (m_nQueryStatus == QUERY_CANCELLED || m_nQueryStatus == QUERY_DONE)
		return;
	
	int nBytesWritten = 0;
	HRESULT hr = m_pISock->Write(m_pWritePos, m_nBytesToWrite, &nBytesWritten);
	if (FAILED(hr)) {
		m_nQueryStatus = QUERY_EWRITE;
		ScheduleCallback();
	}
	else {
		m_nBytesToWrite -= nBytesWritten;
		m_pWritePos += nBytesWritten;
		if (m_nBytesToWrite > 0)
			WRITABLE(WriteCB);
		else {
		    if (!m_bDoingSiteInfo)
			m_bQueryIssued = 1;
			StartRead();
		}
	}
}


void CPHQuery::ReadCB()
{
	HRESULT hr = S_OK;
	int nBytesRead = 0;
  BOOL bDoReadable = 0;

	if (m_nQueryStatus == QUERY_CANCELLED || m_nQueryStatus == QUERY_DONE)
		return;

	hr = m_pISock->Read(m_pReadPos, m_nBytesToRead, &nBytesRead);

	if (FAILED(hr)) {
		int nSocketError;
		m_pISock->GetSocketError(&nSocketError);
		if (nSocketError == WSAEWOULDBLOCK)
      bDoReadable = 1;
		else {
			m_nQueryStatus = QUERY_EREAD;
			ScheduleCallback();
			return;
		}
	}

	m_nBytesToRead -= nBytesRead;
	m_pReadPos += nBytesRead;
	
	// smohanty: 04/21/98
	if (m_bDoingSiteInfo) {
	    if (IsCompleteSiteInfo()) {
		m_bDoingSiteInfo = 0;
		ScheduleCallback();
	    }
	    else 
		READABLE(ReadCB);
		return;
	    
	}

  int nComplete = IsCompleteRecord();		
  if (nComplete == 2)
    return;
	if (nComplete) {
    ScheduleCallback();
    return;
  }
	else
    bDoReadable = 1;

  if (bDoReadable) {
    // move up the contents of the read buffer when the
    // free space at the top is greater than our threshold
    if ((m_pParsePos - m_achReadBuf) > ADJUST_THRESHOLD)
      AdjustReadBuf();
		READABLE(ReadCB);
  }
}



BOOL CPHQuery::CreateRecord()
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



void __stdcall CPHQuery::WaitForRecord(DBQUERYCB pCB, LPVOID pCtx)
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
		
	if (!m_bQueryIssued) {
		StartWrite();							// write the query string (first time through only)
	}
	else {
    int nComplete = IsCompleteRecord();		
    if (!nComplete) {
      // move up the contents of the read buffer when the
      // free space at the top is greater than our threshold
      if ((m_pParsePos - m_achReadBuf) > ADJUST_THRESHOLD)
        AdjustReadBuf();
			READABLE(ReadCB);				// otherwise, schedule another winsock read
    }
    else
    if (nComplete==1)
			ScheduleCallback();			// schedule a notification callback
	}

}


QUERY_STATUS __stdcall CPHQuery::GetRecord(IDSPRecord **ppRecord)
{
	if (m_nQueryStatus != QUERY_OK)
    return m_nQueryStatus;

  // return the IDSPRecord interface
  if (ppRecord && m_pRecord) {
    m_pRecord->QueryInterface(IID_IDSPRecord, (void**)ppRecord);
    m_pRecord->Release();		// release our pointer to object
    m_pRecord = NULL;
  }

  // move up the contents of the read buffer when the
  // free space at the top is greater than our threshold
  if ((m_pParsePos - m_achReadBuf) > ADJUST_THRESHOLD)
    AdjustReadBuf();

  if (m_bLastRecord)
    m_nQueryStatus = QUERY_DONE;

  return (*ppRecord) ? QUERY_OK : QUERY_FAILED;
}

//smohanty: 04/21/98
// returns: 0 = not complete, do a readable
//          1 = complete
int CPHQuery::IsCompleteSiteInfo()
{
	LPSTR pEol;
  int nIndex = 0;
  int nRCode = 0;

	*m_pReadPos = 0;	// NULL terminate the buffer so we can use strxxx functions

	// find the end-of-line
	while (pEol = strchr(m_pParsePos, '\n')) {
		LPSTR pLine     = m_pParsePos;
    LPSTR pToken    = pLine;
    LPSTR pEndToken = NULL;
		m_pParsePos     = pEol+1;

    nRCode = 0;
    nIndex = 0;

    *pEol = 0;    // terminate the line for string functions

		int nFields = sscanf(pLine, "%d:%d:", &nRCode, &nIndex);
	
 		// is the result code a 'multiline' response ?
		if (nRCode == 200)
      return 1;

		// smohanty: 05/20/98
		// Fix for sites that don't support siteinfo.
		if (nRCode == 598) {
		    siteInfoNotImpl = true;
		    return 1;
		}

		if (nRCode == -200) {
			// strip off the result & index codes from the beginning of the line
			for (int i=0; i < nFields; i++)
				pToken = strchr(pToken, ':') + 1;

			if (NULL != (pEndToken = strchr(pToken, ':')))
				*pEndToken = 0;
			else
				continue;

			if (pToken) {
				Trim(pToken, " ");
				if (0 == stricmp(pToken, "maildomain")) {
					pToken = pEndToken + 1;
					Trim(pToken, " \r\n");
					if (strlen(pToken)) {
//            m_pszMailDomain = strdup(pToken);
						m_pPrivPH->SetMailDomain(pToken);
					}
				}
			}
		}
  }

	return 0;
}
    
// returns: 0 = not complete, do a readable
//          1 = complete
//          2 = doing retry, don't do a readable
int CPHQuery::IsCompleteRecord()
{
	assert(m_pPrivPH);

	LPSTR pEol;
  int nIndex = 0;
  int nRCode = 0;
  BOOL bRaw = (m_dwFlags & QUERY_FLG_RAW);

 	// create a new record
	if (!CreateRecord())
		return 1;

	if (m_nQueryStatus == QUERY_DONE)
		return 1;

	*m_pReadPos = 0;	// NULL terminate the buffer so we can use strxxx functions

	// find the end-of-line
	while (pEol = strchr(m_pParsePos, '\n')) {
    DS_FIELD ds   = DS_UNKNOWN;
    LPSTR pUser   = NULL;
		LPSTR pLine   = m_pParsePos;
    LPSTR pToken  = pLine;
		m_pParsePos   = pEol+1;

    nRCode = 0;
    nIndex = 0;

    *pEol = 0;    // terminate the line for string functions

		int nFields = sscanf(pLine, "%d:%d:", &nRCode, &nIndex);
		if (nFields < 1) {
			m_nQueryStatus = QUERY_EBADFORMAT;
			return 1;
		}
	
    // init the last result code
    if (m_nLastRCode == 0)
      m_nLastRCode = nRCode;
    
    // init the last index
    if (m_nLastIndex == 0)
      m_nLastIndex = nIndex;

    // strip off the result & index codes from the beginning of the line
    for (int i=0; i < nFields; i++)
      pToken = strchr(pToken, ':') + 1;

    // for RAW mode only- if the result code changed, the record is complete
    if (bRaw && (nRCode != m_nLastRCode)) {
	m_nLastRCode = 0;
      m_pParsePos = pLine;
      *pEol = '\n';           // restore the end of line for next time through
			m_nQueryStatus = QUERY_OK;
      m_bFoundMatch = TRUE;
			return 1;
		}
	
		// is the result code a 'multiline' response ?
		if (nRCode == -200) {

			if (nFields != 2) {
				m_nQueryStatus = QUERY_EBADFORMAT;
				return 1;
			}

			// when index changes, we have a new record
			if (nIndex != m_nLastIndex) {
			        m_nLastIndex = 0;
				m_pParsePos = pLine;		// reset the parse pos pointer to the BOL
        *pEol = '\n';           // restore the end of line for next time through
				m_nQueryStatus = QUERY_OK;
        m_bFoundMatch = TRUE;
				return 1;								// we have a complete record
			}

			// parse this line, see if we match an attribute in the map
      if (!bRaw) {
  			if (NULL != (pToken = m_pPrivPH->ParseLine(pToken, (int*)&ds))) {
          // trim trailing cr/lf/whsp
          Trim(pToken, "\r\n\t\b");
          // if Unknown attribute type, add the token as a "user string"
          if (ds == DS_UNKNOWN) {
            LPSTR p;
            // find end of token
            for(p=pToken; *p && *p != ' '; p++)
              ;
            *p = 0;
            pUser = pToken;
            pToken = p+1;
          }
  			}
      }
		}

    // suppress adding this token to record if not RAW 
    // and not a normal multiline entry
    if (!bRaw && (nRCode != -200))
      pToken = NULL;
    
    // if we have a token, add it to the record
    if (pToken) {
	if (ds == DS_EMAIL) {
	    if ((siteInfoNotImpl == false) && !strchr(pToken, '@') && (m_pPrivPH->GetMailDomain() != NULL)) {
		strcpy(m_pszTempBuffer, pToken);
		strcat(m_pszTempBuffer, "@");
		strcat(m_pszTempBuffer, m_pPrivPH->GetMailDomain()/*m_pszMailDomain*/);
		pToken = m_pszTempBuffer;
	    }
	}
      if (NULL == m_pRecord->AddStringRecEnt(pToken, ds, pUser)) {
        m_nQueryStatus = QUERY_EOUTOFOMEMORY;
        return 1;
      }
      m_bFoundMatch = TRUE;
    }

    // handle the query done result code
    if (nRCode >= 200) {

      // check if we need to do a retry
      if ((nRCode == 598) && m_bDoRetry && m_pszSaveRawQuery) {
//        assert(m_pszSaveRawQuery);
        BuildQueryCommand(m_pszSaveRawQuery, 0, 0);
        m_bDoRetry = 0;
        m_bRetrying = 1;

		delete [] m_pszSaveRawQuery;
        m_pszSaveRawQuery = NULL;

        if (m_pRecord) {
          m_pRecord->Release();
          m_pRecord = NULL;
        }
        m_bQueryIssued 		= 0;
        m_nQueryStatus 		= QUERY_OK;
        m_bLastRecord			= 0;
        m_bFoundMatch     = 0;

        m_pReadPos = m_pParsePos = m_achReadBuf;
        m_nBytesToRead = sizeof(m_achReadBuf)-2;

	    	m_pISock->Cancel();
        m_pISock->Release();
        m_pISock = NULL;

	if (m_pISchedule) {
	                m_pISchedule->Cancel();
			m_pISchedule->Release();
			m_pISchedule = NULL;
	}

       	// recreate the ISock object
	      HRESULT hr = CreateSocket();
	      if (FAILED(hr)) {
          m_nQueryStatus = QUERY_EPROTOCOL;
		      return 1;
        }
        // try to reconnect...
        if (!Connect())
          ScheduleCallback();
        return 2;
      }

      m_bLastRecord = 1;
			m_nQueryStatus = m_bFoundMatch ? QUERY_OK : QUERY_ENOMATCH;
      return 1;
    }

	}

	return 0;
}


void CPHQuery::AdjustReadBuf()
{
	// move memory from m_pParsePos to m_achReadBuf, count of m_pReadPos - m_pParsePos
	int nDataSize = m_pReadPos - m_pParsePos;
	int nAdjust = m_pParsePos - m_achReadBuf;
	memmove(m_achReadBuf, m_pParsePos, nDataSize);
	m_pParsePos -= nAdjust;
	m_pReadPos -= nAdjust;
	m_nBytesToRead += nAdjust;
}



void CPHQuery::ScheduleCallback()
{
	m_pISchedule->Schedule(ISOCKCALLBACK(Callback), this);
}


void CPHQuery::Callback()
{
	assert(m_pfnQueryCB);
	m_pfnQueryCB(m_pContext);
}


void CPHQuery::SetDatabaseID(LPSTR pszID)
{
	delete [] m_pszDatabaseID;
	m_pszDatabaseID = NULL;

	if (pszID)
		m_pszDatabaseID = SafeStrdupMT(pszID);
}


///////////////////////////////////////////////////////////////////////////////
// IDSPQuery

BOOL CPHQuery::BuildQueryCommand(LPSTR pszSearch, BOOL bRaw, BOOL bAppendReturnAll)
{
  // free any pre-allocated query string
  delete [] m_pszSearch;

  m_pszSearch = NULL;

  // remove trailing newline(s) if present
  Trim(pszSearch, "\r\n");

  // allocate space for the new query command
  m_pszSearch = DEBUG_NEW_NOTHROW char [strlen(pszSearch) + 80];

  if (!m_pszSearch) {
      return FALSE;
  }

  if (bRaw)
  	strcpy(m_pszSearch, pszSearch);
  else {
  	strcpy(m_pszSearch, "query ");
    strcat(m_pszSearch, pszSearch);
    if (bAppendReturnAll)
      strcat(m_pszSearch, " return all");
  }
  strcat(m_pszSearch, "\n");
  return TRUE;
}


HRESULT __stdcall CPHQuery::InitQuery(LPSTR pszSearch, DWORD dwFlags, DS_FIELD nField)
{
	if (!pszSearch)
		return E_FAIL;
	
	// create the ISock object
	HRESULT hr = CreateSocket();
	if (FAILED(hr))
		return E_FAIL;

  BOOL bRaw = (dwFlags & QUERY_FLG_RAW) || (dwFlags & QUERY_FLG_RAWQUERY);
  BuildQueryCommand(pszSearch, bRaw, !bRaw);
  
  // if this is a raw command and the query command does
  // not contain a known query command, then set up for
  // a possible retry on failure...
  m_bDoRetry = (bRaw && !IsQueryCommand(pszSearch));
  if (m_bDoRetry)
    m_pszSaveRawQuery = SafeStrdupMT(pszSearch);

	if (m_pRecord) {
		m_pRecord->Release();
		m_pRecord = NULL;
	}
	m_dwFlags 				= dwFlags;
	m_nField 					= nField;
	m_bQueryIssued 		= 0;
	m_nQueryStatus 		= QUERY_OK;
	m_bLastRecord			= 0;
  m_bFoundMatch     = 0;
  m_bRetrying       = 0;
  m_nLastIndex = 0;
  m_nLastRCode = 0;

  // smohanty: 04/21/98
  if (!m_pszTempBuffer) {
	  m_pszTempBuffer = DEBUG_NEW_NOTHROW char[TEMPBUFSIZE];
      if (!m_pszTempBuffer)
	      return E_OUTOFMEMORY;
  }

	return S_OK;
}


void __stdcall CPHQuery::Start(DBQUERYCB pCB, LPVOID pCtx)
{
	m_pfnQueryCB 	= pCB;
	m_pContext 		= pCtx;
	if (!Connect())
		ScheduleCallback();
}

void __stdcall CPHQuery::Cancel()
{
	m_nQueryStatus = QUERY_CANCELLED;
	if (m_pISchedule)
		m_pISchedule->Cancel();
	if (m_pISock)
		m_pISock->Cancel();
}

QUERY_STATUS __stdcall CPHQuery::GetLastError()
{
	return m_nQueryStatus;
}

HRESULT __stdcall CPHQuery::GetErrorString(QUERY_STATUS nCode, LPSTR pszBuffer, int nlen)
{
  typedef struct {
    QUERY_STATUS  qs;
    int           nResIDString;
  } ERRORDESC, FAR *LPERRORDESC;

  ERRORDESC arErrors[] = {
    QUERY_OK,						  IDS_QUERYOK,
    QUERY_FAILED,         IDS_QUERYFAILED,
    QUERY_WOULDBLOCK,			IDS_WOULDBLOCK,
    QUERY_DONE,						IDS_DONE,
    QUERY_CANCELLED,			IDS_CANCELLED,
    QUERY_EBADFORMAT,			IDS_EBADFORMAT,
    QUERY_ECONNECT,				IDS_ECONNECT,
    QUERY_EADDRESS,				IDS_EADDRESS,
    QUERY_EREAD,					IDS_EREAD,
    QUERY_EWRITE,					IDS_EWRITE,
    QUERY_EOUTOFOMEMORY,	IDS_EOUTOFMEMORY,
    QUERY_EPROTOCOL,      IDS_EPROTOCOL
  };
  const NUMERRORDESC = sizeof(arErrors) / sizeof(ERRORDESC);

  LPERRORDESC ped;
  int i;

  for (i=0,ped=arErrors; ped && (i < NUMERRORDESC); i++, ped++) {
    if (ped->qs == nCode) {
      // load string from resource file
      CUString str(ped->nResIDString);
      if (0 == strlen((LPSTR)str))
        break;
      strncpy(pszBuffer, (LPSTR)str, nlen);
      return S_OK;
    }
  }
  return E_FAIL;
}


HRESULT __stdcall CPHQuery::SetSearchBase(LPSTR /* pszSearchBase */)
	{ return S_OK; }		// not needed for PH




