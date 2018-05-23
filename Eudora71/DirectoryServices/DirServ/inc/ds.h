/******************************************************************************/
/*																										*/
/*	Name		:	DS.H																				*/
/* Date     :  7/25/1997                                                     	*/
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

/*	Desc.		:	Private Directory Services Object definition							*/
/*																										*/
/******************************************************************************/
#ifndef __DS_H__
#define __DS_H__

#include "ilist.h"
#include "dsapi.h"
#include "proto.h"
#include "database.h"
#include "query.h"
#include "post.h"
#include "record.h"

typedef void (*PETBCB)(char *, void *);

class CDirServ : public IDirServ
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();
	
	/* IDirServ																						*/
	virtual HRESULT		__stdcall Initialize();
	virtual HRESULT 		__stdcall EnumProtocols(IDSEnumProtocols ** ppIEnum);
	virtual HRESULT 		__stdcall EnumDatabases(IDSProtocol *pProto,IDSEnumDatabases ** ppIEnum);
	virtual IDSDatabase *__stdcall FindDatabase(LPSTR pszDatabaseID);
	virtual HRESULT   	__stdcall CreateDatabase(IDSProtocol *pProto,IDSDatabase **pDatabase);
	virtual HRESULT   	__stdcall DeleteDatabase(IDSDatabase *pDatabase);
	virtual HRESULT		__stdcall CreateObject(REFIID riid,void **ppv);
	virtual HRESULT		__stdcall ExportToHTML(IDSPRecord *pRec,LPSTR pszFilename,BOOL bOverWrite,DWORD dwflags);
	virtual HRESULT 		__stdcall ExportToText(IDSPRecord* pSearchHit,LPSTR pszFileName,BOOL OverWrite );
	virtual HRESULT 		__stdcall ExportToVCard(IDSPRecord* pSearchHit,LPSTR pszFileName,BOOL OverWrite );
	virtual HRESULT 		__stdcall ExportToBuffer(IDSPRecord* pSearchHit, PETBCB petbcb, void *pv);
	virtual HRESULT 		__stdcall ExportToBufferFormatted(void *data, PETBCB petbcb, void *pv);


	static IUnknown *CreateServInstance();
	static void ProtoFreeCB(LPITEM pItem,LPVOID pUser);
	CDirServ();
	~CDirServ();
	
protected:
	/* Load a protocol object																	*/
	HRESULT 				__stdcall LoadProtocol(LPSTR pszCLSID);
	
	/* Add protocol to the list of protocols we control								*/
	HRESULT 				__stdcall AddProtocol(CDSProtocol *pProto);
	
	LONG					m_cRef;				/* Reference count							*/
	IListMan				*m_pProtocolList;	/* List of PENTs								*/
	BOOL					m_Initialized;
};

HRESULT __stdcall MapFieldName(HINSTANCE hInst,DS_FIELD nField,LPSTR pszBuffer,int nLen);
HRESULT __stdcall FindFieldKey(DS_FIELD nField,LPSTR pszBuffer,int nLen);
HRESULT __stdcall LookupFieldKey(DS_FIELD *pnField,LPSTR pszKey);

#endif

