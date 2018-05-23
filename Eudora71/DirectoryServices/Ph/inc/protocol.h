/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					protocol.h
Description:		COM components for protocol server
Date:						7/2/97
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
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <dspapi.h>


class CServer;

///////////////////////////////////////////////////////////////////////////////
// CLSID_DSPROTOCOLxxx component


class CProtocol : public IDSPProtocol
{
public:
	// IUnknown
	virtual HRESULT	__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	// IDSPProtocol
	virtual HRESULT	__stdcall Initialize(IUnknown *pUnkCaller);
	virtual HRESULT __stdcall CreateObject(REFIID riid, LPVOID *ppv);
	virtual HRESULT __stdcall DeleteObject(IUnknown *punkObject);
	virtual HRESULT __stdcall EnumDatabases(IDSPEnumConfig **ppenum);
	
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HBITMAP	__stdcall GetProtocolImage(int iImage) = 0;
	virtual DWORD		__stdcall GetProtocolFlags() = 0;

	// non-interface methods
protected:
	virtual HRESULT __stdcall CreateServer(IUnknown **ppunkNew) = 0;
	virtual HRESULT __stdcall LoadServers() = 0;
public:
	IDirConfig *		__stdcall	GetIDirConfig() { return m_pIDirConfig; }

	// Constructor
	CProtocol();

	// Destructor
	virtual ~CProtocol();

protected:
	long						m_cRef;
	LPVOID					m_pServerList;				// list of servers that we created
	CServer *				m_pCurrentServer;			// used by IEnum functions to keep track of next
	IDirConfig *		m_pIDirConfig;				// IDirConfig interface that we got from QI on our creator 
	int							m_cNumServers;
};


///////////////////////////////////////////////////////////////////////////////
// CServer component

class CServer : public IDSPConfig
{
public:
#if 0
	// IUnknown
	virtual HRESULT	__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
#endif
	// IConfig
	virtual HRESULT	__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HBITMAP	__stdcall GetImage(int iImage) = 0;
	virtual LPSTR		__stdcall GetID() = 0;
	virtual int			__stdcall GetPropPageCount() = 0;
	virtual HRESULT	__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages) = 0;

	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HBITMAP	__stdcall GetProtocolImage(int iImage) = 0;
	virtual DWORD		__stdcall GetProtocolFlags() = 0;
	virtual HRESULT __stdcall CreateObject(REFIID riid,LPVOID *ppv) = 0;

	// other (non-interface) methods
	virtual HRESULT __stdcall DeleteData() = 0;

	// Constructor
	CServer();
	
	// Destructor
	virtual	~CServer();

	// list functions
public:
  // pRefNode insertion constants
	#define INSERT_APPEND		(CServer*)(0x00000000)
	#define INSERT_PREPEND	(CServer*)(0x00000001)

public:
	BOOL 					Attach(LPVOID * ppList, CServer* pRefNode=INSERT_APPEND);
	BOOL					Detach(LPVOID *ppList);
	CServer * 		FindPrev(LPVOID *ppList, CServer* pNode);
	CServer *			GetNext() {return pNext;}
	BOOL 					IsHead(LPVOID * ppList) 	{ return (GetHead(ppList) == this); }
	static
	CServer * 		GetHead(LPVOID * ppList) 	{ return (ppList) ? *((CServer**)ppList) : NULL; }
	static
	void					FreeList(LPVOID * ppList);

protected:
	CServer *			pNext;
	long					m_cRef;
};


///////////////////////////////////////////////////////////////////////////////
// CEnumConfig class (not a COM instantiable component - no CLSID)

class CEnumConfig : public IDSPEnumConfig
{
public:
	// IUnknown
	virtual HRESULT	__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	// IEnumConfig
	virtual HRESULT __stdcall Next(ULONG celt, IDSPConfig ** rgelt, ULONG * pceltFetched);
	virtual HRESULT __stdcall Skip(ULONG celt);
	virtual HRESULT __stdcall Reset(void);
	virtual HRESULT __stdcall Clone(IDSPEnumConfig ** ppenum);

	// Constructor
	CEnumConfig(IUnknown *pUnkOuter, LPVOID pServerList, LPVOID pCurrentServer);
	CEnumConfig(CEnumConfig *pSource);

	// Destructor
	~CEnumConfig();

private:
	CServer * 			GetNextServer();

private:
	long						m_cRef;
	IUnknown *			m_pUnkOuter;
	LPVOID					m_pServerList;
	CServer *				m_pCurrentServer;
};


#endif	//__PROTOCOL_H__

