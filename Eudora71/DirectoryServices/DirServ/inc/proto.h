/******************************************************************************/
/*																										*/
/*	Name		:	PROTO.H  		  																*/
/* Date     :  7/28/1997                                                      */
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

/*	Desc.		:	IPProtocol object wrapper class implementation						*/
/*																										*/
/******************************************************************************/
#ifndef __PROTO_H__
#define __PROTO_H__
#include "ilist.h"
#include "dsapi.h"
#include "dspapi.h"

#define UTIL_BUF_SIZE 500

class CDSProtocol : public IDSProtocol, public IDirConfig
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();
	
	/* IDSProtocol																					*/
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage);
	virtual DWORD			__stdcall GetProtocolFlags();
	
	/* IDirConfig																					*/
	virtual HRESULT 		__stdcall CreateDirSection(LPSTR szSection,BOOL *bExists);
	virtual HRESULT 		__stdcall CreateDirKey(LPSTR szSection,LPSTR szKey);
	virtual HRESULT 		__stdcall DeleteDirSection(LPSTR szSection);
	virtual HRESULT 		__stdcall DeleteDirKey(LPSTR szSection,LPSTR szKey);
	virtual HRESULT 		__stdcall GetDirInteger(LPSTR szSection,LPSTR szKey,int nDefault,int* returned_value);
	virtual HRESULT 		__stdcall GetDirBool(LPSTR szSection,LPSTR szKey,BOOL bDefault,BOOL* returned_value);
	virtual HRESULT 		__stdcall GetDirDword(LPSTR szSection,LPSTR szKey,DWORD dwDefault,DWORD* returned_value);
	virtual HRESULT		__stdcall GetDirStr(LPSTR szSection,LPSTR szKey,LPSTR default_value,DWORD cbDefault,LPSTR returned_value,DWORD* cbReturned);
	virtual HRESULT		__stdcall GetDirBinary(LPSTR szSection,LPSTR szKey,LPBYTE default_value,DWORD cbDefault,LPBYTE returned_value,DWORD* cbReturned);
	virtual HRESULT 		__stdcall EnumDirSections(LPSTR szSection,DWORD* cbBuffer);
	virtual HRESULT 		__stdcall EnumDirKeys(LPSTR szSection,LPSTR szKey,DWORD* cbBuffer,DataType* type_of_stored_value);
	virtual HRESULT		__stdcall EndEnumDirKeys(LPSTR szSection);
	virtual HRESULT 		__stdcall SetDirInteger(LPSTR szSection,LPSTR szKey,int new_value);
	virtual HRESULT 		__stdcall SetDirBool(LPSTR szSection,LPSTR szKey,BOOL new_value);
	virtual HRESULT 		__stdcall SetDirDword(LPSTR szSection,LPSTR szKey,DWORD new_value);
	virtual HRESULT 		__stdcall SetDirStr(LPSTR szSection,LPSTR szKey,LPSTR new_value);
	virtual HRESULT 		__stdcall SetDirBinary(LPSTR szSection,LPSTR szKey,LPBYTE new_value,DWORD cbValue);
	virtual HRESULT 		__stdcall GetDirLastError(LONG* ResultCode);
	virtual HRESULT 		__stdcall GetFieldName(DS_FIELD nField,LPSTR pszBuffer,int nLen);
	virtual HRESULT 		__stdcall GetFieldKey(DS_FIELD nField,LPSTR pszBuffer,int nLen);
	virtual HRESULT 		__stdcall MapFieldKey(DS_FIELD *pnField,LPSTR pszKey);
	
	/* Implementation specific methods														*/
	virtual HRESULT		__stdcall Initialize(LPSTR pRegStr,LPSTR pCLSID);
	virtual HRESULT		__stdcall CreateObject(REFIID riid,void **ppv);
	virtual HRESULT      __stdcall DeleteObject(IUnknown *pUnk);
	virtual IDSPProtocol *__stdcall GetWrappedInterface();
	
	virtual HRESULT		__stdcall HarryCarry();
	CDSProtocol();
	~CDSProtocol();
protected:


	LONG						m_cRef;
	
	/* IDSProtocol data																			*/
	IDSPProtocol			*m_pProtoObj;
	
	/* IDirConfig Data																			*/
	HKEY    m_hBaseKey;                 // Open registry key to the start of all our directory configuration data
	BOOL    m_BaseKeyValid;             // What is an illegal value for a registry key ?
	char    m_UtilBuf[UTIL_BUF_SIZE];   //
	LONG    m_ErrorValue;               // Error code from previous call
	DWORD   m_SubKeyIndex;              // Registry subkey index in the registry
	DWORD   m_ValueIndex;               // Registry value index of values under a key in the registry
	HKEY    m_hEnumKey;                 // Keep key open when doing Enum of values under a key
	BOOL    m_EnumKeyValid;             //
};

typedef struct tagPENT {
	ITEM				item;
	CDSProtocol		*pDSP;
} PENT, *LPPENT;

class CDSEnumProtocols : public IDSEnumProtocols
{
public:
	/* IUnknown																						*/
	virtual HRESULT	__stdcall QueryInterface(REFIID iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	
	/* IDSEnumProtocols																			*/
	virtual HRESULT 	__stdcall Next(ULONG celt, IDSProtocol ** rgelt, ULONG * pceltFetched);
	virtual HRESULT 	__stdcall Skip(ULONG celt);
	virtual HRESULT 	__stdcall Reset();
	virtual HRESULT 	__stdcall Clone(IDSEnumProtocols ** ppenum);
	
	CDSEnumProtocols(IListMan *pList);
	~CDSEnumProtocols();
	
protected:		
	LONG					m_cRef;
	IListMan				*m_pList;
	PENT					*m_pCur;
	DWORD					m_dwIdx;
};

//#define _DEBUG_DIRCONFIG
#ifdef _DEBUG_DIRCONFIG
#define DBREAK	DebugBreak()
#else
#define DBREAK
#endif

#endif


