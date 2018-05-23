/******************************************************************************/
/*																										*/
/*	Name		:	ILIST.H			  																*/
/* Date     :  4/30/1997                                                      */
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

/*	Desc.		:	Linked list manager class definitions									*/
/*																										*/
/******************************************************************************/
#ifndef _ILISTP_H_
#define _ILISTP_H_

#include "ilist.h"

typedef class IList * LPILIST;
class IList : public IListMan
{
public:
	/* IUnknown methods																			*/
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
   STDMETHODIMP_(ULONG) AddRef(void);
   STDMETHODIMP_(ULONG) Release(void);

	//BOOL 		virtual PASCAL Initialize(ILIST_TYPE wType,DWORD dwFlags,ILISTFREECB pCB,LPVOID pUser);
	BOOL     virtual PASCAL Initialize(ILISTFREECB pCB,LPVOID pUser);
	void 		virtual PASCAL Attach(LPVOID pItem);
	void		virtual PASCAL Insert(LPVOID pRefItem,LPVOID pItem,WORD wWhere);
	LPVOID 	virtual PASCAL Unlink(LPVOID pItem);
	void		virtual PASCAL DeleteItem(LPVOID pItem);
	void 		virtual PASCAL Erase(void);
	LPVOID	virtual PASCAL Head(void);
	DWORD   	virtual PASCAL Count(void);
	void 		virtual PASCAL Push(LPVOID pItem);
	LPVOID	virtual PASCAL Pop();
	WORD		virtual PASCAL VerifyList(void);

	static IUnknown *CreateInstance();
	IList(void);
	~IList(void);

protected:
	BOOL		virtual PASCAL VerifyForwardLink(LPITEM pItem);
	BOOL  	virtual PASCAL VerifyBackwardLink(LPITEM pItem);
	
	LONG			m_nRefs;
	DWORD			m_dwCount;
	LPITEM		m_pHead;
	ILIST_TYPE	m_wType;
	DWORD			m_dwFlags;
	ILISTFREECB	m_pFreeCB;
	LPVOID		m_pFreeUserData;
} ;

#endif



