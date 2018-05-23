/******************************************************************************/
/*																										*/
/*	Name		:	DATABASE.CPP	  																*/
/* Date     :  7/30/1997                                                      */
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

/*	Desc.		:	IDSPConfig object wrapper class definition							*/
/*																										*/
/******************************************************************************/
#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "ilist.h"
#include "dsapi.h"
#include "dspapi.h"
#include "proto.h"

class CDSDatabase : public IDSDatabase
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();
	
	/* IDSDatabase																					*/
	virtual HRESULT		__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP		__stdcall GetImage(int iImage);
	virtual LPSTR			__stdcall GetDatabaseID();
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage);
	virtual DWORD			__stdcall GetProtocolFlags();
	virtual int				__stdcall GetPropPageCount();
	virtual HRESULT		__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages);
	
	virtual IDSPConfig * __stdcall GetWrappedInterface();
	
	CDSDatabase(IDSPConfig *pConfig);
	~CDSDatabase();
	
protected:
	LONG						m_cRef;
	IDSPConfig 				*m_pConfig;		/* Our underlying proto driver object	*/
};

typedef struct tagDBENT DBENT;
class CDSEnumDatabases : public IDSEnumDatabases
{
public:
	/* IUnknown																						*/
	virtual HRESULT	__stdcall QueryInterface(REFIID iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	
	/* IDSEnumProtocols																			*/
	virtual HRESULT 	__stdcall Next(ULONG celt, IDSDatabase ** rgelt, ULONG * pceltFetched);
	virtual HRESULT 	__stdcall Skip(ULONG celt);
	virtual HRESULT 	__stdcall Reset();
	virtual HRESULT 	__stdcall Clone(IDSEnumDatabases ** ppenum);
	
	static void DentFreeCB(LPITEM pItem,LPVOID pUser);
	CDSEnumDatabases(IListMan *pProtocolList,CDSProtocol *pcProto,BOOL bClone);
	~CDSEnumDatabases();
	
protected:		
	virtual HRESULT	__stdcall AddProtocolDatabases(IDSPProtocol *pProto);
	
	LONG					m_cRef;				/* Object reference count					*/
	BOOL					m_bToast;			/* Creation error flag						*/
	IListMan				*m_pList;			/* Enumeration list							*/
	DBENT					*m_pCur;				/* Current item to enumerate				*/
	DWORD					m_dwIdx;				/* List index for enumeration				*/
};

#endif




