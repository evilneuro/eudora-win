/******************************************************************************/
/*																										*/
/*	Name		:	QUERY.H   	  																   */
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

/*	Desc.		:	IDSPQuery interface defintiions      									*/
/*																										*/
/******************************************************************************/
#ifndef __QUERY_H__
#define __QUERY_H__

#include <stdio.h>
#include "IList.h"
#include "ISched.h"
#include "dspapi.h"
#include "database.h"

class CQuery : public IDSPQuery
{
public:	
   /* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();
   /* IDSPQuery																					*/
	virtual HRESULT		__stdcall InitQuery(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField);
	virtual HRESULT		__stdcall SetSearchBase(LPSTR pszSearchBase);
	virtual void        	__stdcall Start(DBQUERYCB pCB,LPVOID pCtx);
	virtual void			__stdcall WaitForRecord(DBQUERYCB pCB,LPVOID pCtx);
	virtual QUERY_STATUS	__stdcall GetRecord(IDSPRecord **ppRecord);
	virtual void         __stdcall Cancel();
	virtual QUERY_STATUS __stdcall GetLastError();
	virtual HRESULT     	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen);

   /* CQuery                                                                  */
	CQuery(CDatabase *pServer);
	~CQuery();

   static HRESULT       __stdcall CreateInstance(IUnknown **pUnk,CDatabase *pServer);

protected:
   virtual BOOL         __stdcall FuzzyCompare(LPSTR lpszCheck);
   virtual BOOL         __stdcall Match(IDSPRecord *pRec);

   LONG					m_cRef;				/* Reference count							*/
   CDatabase         *m_pDatabase;     /* The "database" we are querying      */
   ISchedule         *m_pSched;        /* Our scheduler object                */
   LPSTR             m_pszSearch;      /* String to search for                */
   LPLTBL            m_pCur;           /* Current matching record             */
   QUERY_STATUS      m_nLastError;     /* Status of last operation            */
   DWORD             m_dwFlags;        /* Query flags                         */
   DS_FIELD          m_nField;         /* Database field to search            */
   IListMan          *m_pRecordList;   /* List of records                     */
   LPRECENT          m_pRE;            /* Next record                         */
   BOOL              m_bCancled;       /* Operation canceled                  */
};

#endif




