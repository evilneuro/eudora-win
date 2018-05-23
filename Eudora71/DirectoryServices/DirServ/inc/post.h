/******************************************************************************/
/*																										*/
/*	Name		:	POST.H 																			*/
/* Date     :  9/18/1997                                                     	*/
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

/*	Desc.		:	Private Directory Services Post Object definition	 				*/
/*																										*/
/******************************************************************************/
#ifndef __POST_H__
#define __POST_H__

#include "ISched.h"
#include "dsapi.h"
#include "dspapi.h"
#include "database.h"

class CPost : public IDSPost
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();
	/* IDSPost																						*/
	virtual HRESULT      __stdcall Initialize(IDSDatabase *pDatabase,DWORD dwFlags);
	virtual void         __stdcall Start(DBQUERYCB pCB,LPVOID pCtx);
	virtual void			__stdcall Postable(DBQUERYCB pCB,LPVOID pCtx);
   virtual QUERY_STATUS	__stdcall PostRecord(IDSPRecord *pRec,DWORD dwflags);
	virtual void         __stdcall Cancel();
	virtual QUERY_STATUS __stdcall GetLastError();
	virtual HRESULT    	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen);
	
	static HRESULT CreateInstance(IUnknown **pUnk);
	static void StartCB(LPVOID pCtx);
	
	CPost();
	~CPost();
	
protected:
	LONG					m_cRef;				/* Reference count							*/
   DWORD             m_dwFlags;        /* Currently unused                    */
   ISchedule         *m_pSched;        /* Our scheduler object                */
   IDSPPost          *m_pPost;         /* Database protocols post object      */
	DBQUERYCB			m_pStartCB;
	LPVOID				m_pStartData;
};

#endif



