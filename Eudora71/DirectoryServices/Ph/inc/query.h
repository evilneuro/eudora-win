/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					query.h
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
#ifndef __QUERY_H__
#define __QUERY_H__

#include <dspapi.h>
#include <isock.h>
#include "record.h"


// this macro converts FuncName to xFuncName (the static callback wrapper function)
#define CPPCALLBACK(name)			(x##name)
#define ISOCKCALLBACK(name) 	(ISOCKCB)CPPCALLBACK(name)
#define DEFCPPCALLBACK(classname, name)	\
	static void CPPCALLBACK(name)(classname * pThis) {pThis->name();}	\
	void name();



class CPHRecord;

#define READBUF_SIZE				8192
#define READ_THRESHOLD			2000
#define ADJUST_THRESHOLD		2000


///////////////////////////////////////////////////////////////////////////////
// CPHQuery class

class CPHQuery : public IDSPQuery
{
public: 	
	// IUnknown
	virtual HRESULT				__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG					__stdcall AddRef();
	virtual ULONG					__stdcall Release();
	// IDSPQuery
	virtual HRESULT				__stdcall InitQuery(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField);
	virtual HRESULT				__stdcall SetSearchBase(LPSTR pszSearchBase);
	virtual void    			__stdcall Start(DBQUERYCB pCB,LPVOID pCtx);
	virtual void					__stdcall WaitForRecord(DBQUERYCB pCB,LPVOID pCtx);
	virtual QUERY_STATUS	__stdcall GetRecord(IDSPRecord **ppRecord);
	virtual void         	__stdcall Cancel();
	virtual QUERY_STATUS 	__stdcall GetLastError();
	virtual HRESULT     	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen);

	CPHQuery(IUnknown *punkParent);
	~CPHQuery();

	HRESULT 							CreateSocket();
	BOOL 									CreateRecord();
	BOOL		 							Connect();
  void 									StartWrite();
	void 									StartRead();
	int 									IsCompleteRecord();
	void 									AdjustReadBuf();
  void 									ScheduleCallback();
	void									SetDatabaseID(LPSTR pszID);
  BOOL                  BuildQueryCommand(LPSTR pszSearch, BOOL bRaw, BOOL bAppendReturnAll);

    // smohanty: 04/21/98
    BOOL DoSiteInfo();
    int  IsCompleteSiteInfo();

  DEFCPPCALLBACK(CPHQuery, ConnectCB)
	DEFCPPCALLBACK(CPHQuery, WriteCB)
	DEFCPPCALLBACK(CPHQuery, ReadCB)
	DEFCPPCALLBACK(CPHQuery, Callback)

protected:
	long									m_cRef;
	BOOL									m_bConnected;
  IPrivateConfig *			m_pPrivPH;
	LPISOCK								m_pISock;
	LPISCHEDULE						m_pISchedule;	// our scheduling/dispatch object
	LPSTR									m_pszSearch;
	DWORD									m_dwFlags;
	int										m_nField;
  DBQUERYCB							m_pfnQueryCB;
	LPVOID								m_pContext;
	BOOL									m_bLastRecord;
  BOOL                  m_bFoundMatch;  // we received at least one 'hit' for this query
	QUERY_STATUS					m_nQueryStatus;
	BOOL									m_bQueryIssued;
	CPHRecord *						m_pRecord;		// current record we are building
	LPSTR									m_pszDatabaseID;
  BOOL                  m_bDoRetry;   // flag to do a retry if raw query fails
  BOOL                  m_bRetrying;  // flag that tells us we are in the middle of a retry
  LPSTR                 m_pszSaveRawQuery; // place to save the raw query string for retries

    // smohanty: 04/21/98
    int m_nLastIndex;
    int m_nLastRCode;
    bool siteInfoNotImpl;

    BOOL m_bDoingSiteInfo;
    LPSTR m_pszTempBuffer;
	// read buffer members
	LPSTR									m_pReadPos;
	int										m_nBytesToRead;
	LPSTR									m_pParsePos;

	// write buffer members
	LPSTR									m_pWritePos;
	int										m_nBytesToWrite;

	// Read buffer
	char									m_achReadBuf[READBUF_SIZE];
};



#endif __QUERY_H__

