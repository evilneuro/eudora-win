/******************************************************************************/
/*																										*/
/*	Name		:	QUERY.H 																			*/
/* Date     :  8/7/1997                                                     	*/
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

/*	Desc.		:	Private Directory Services Query Object definition	 				*/
/*																										*/
/******************************************************************************/
#ifndef __QUERY_H__
#define __QUERY_H__

#include "ilist.h"
#include "ISched.h"
#include "dsapi.h"
#include "dspapi.h"
#include "database.h"

typedef enum {
	QSTATE_STARTING,
	QSTATE_STARTED,
	QSTATE_WAITING,
	QSTATE_IDLE,
	QSTATE_DONE
} QSTATE;
							
typedef struct	tagRECENT
{
	ITEM			item;
	IDSPRecord	*pRec;
} RECENT, *LPRECENT;
										
typedef struct tagQENT *LPQENT;

class CQuery : public IDSQuery
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();
	/* IDSQuery																						*/
	virtual HRESULT      __stdcall Initialize(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField);
	virtual HRESULT		__stdcall AddDatabase(IDSDatabase *pDatabase);
	virtual HRESULT		__stdcall SetSearchBase(LPSTR pszSearchBase);	
	virtual void         __stdcall Start(DBQUERYCB pCB,LPVOID pCtx);
	virtual void			__stdcall WaitForRecord(DBQUERYCB pCB,LPVOID pCtx);
	virtual QUERY_STATUS	__stdcall GetRecord(IDSPRecord **ppRecord);
	virtual void         __stdcall Cancel();
	virtual QUERY_STATUS __stdcall GetLastError();
	virtual HRESULT    	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen);
	
	static HRESULT CreateInstance(IUnknown **pUnk);
	static void QentFreeCB(LPITEM pItem,LPVOID pUser);
	static void RecEntFreeCB(LPITEM pItem,LPVOID pUser);
	static void StartCB(LPVOID pCtx);
	static void WaitCB(LPVOID pCtx);
	
	CQuery();
	~CQuery();
	
protected:
	virtual void 		__stdcall PushErrorRecord(QUERY_STATUS qErr,LPQENT pQent);
	
	LONG					m_cRef;				/* Reference count							*/
	ISchedule			*m_pSched;			/* Async scheduler							*/
	IListMan				*m_pRecList;		/* List of records (RECENT)	  			*/
	IListMan				*m_pDBList;			/* Database list	(QENT) 					*/
	IListMan          *m_pDBFailedList; /* Failed Database list  (QENT)        */
	int					m_nDBCount;			/* Number of databases in array			*/
	LPSTR					m_pszSearch;		/* The query									*/
	DS_FIELD				m_nField;  			/* What field to match 						*/
	DWORD					m_dwFlags;			/* Query Flags									*/
	BOOL					m_bParentStarted;	/* TRUE == parent startCB called 		*/
	BOOL					m_bCallerWaiting;	/* Set when caller is waiting for data	*/
	QUERY_STATUS		m_LastError;		
	
	DBQUERYCB			m_pStartCB;
	LPVOID				m_pStartData;
	
	DBQUERYCB			m_pWaitCB;
	LPVOID				m_pWaitData;
};

typedef struct tagQENT
{
	ITEM			item;
	
	int			nState;
	DWORD			dwState;
	
	CQuery		*pcQuery;
	CDSDatabase	*pDatabase;
	IDSPQuery	*pQuery;
} QENT, *LPQENT;

#endif


