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

#define SINGLELL		1

class CServer;

///////////////////////////////////////////////////////////////////////////////
// CLSID_DSPROTOCOLxxx component


class CProtocol : public IProtocol
{
public:
	// IUnknown
	virtual HRESULT	__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	// IProtocol
	virtual HRESULT	__stdcall Initialize(IUnknown *pUnkCaller);
	virtual HRESULT __stdcall CreateObject(REFIID riid, LPVOID *ppv);
	virtual HRESULT	__stdcall ReleaseObject(IUnknown * punkObject);
	virtual HRESULT __stdcall EnumDatabases(IPEnumConfig **ppenum);
	
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HICON		__stdcall GetProtocolImage() = 0;
	virtual DWORD		__stdcall GetProtocolFlags() = 0;

protected:
	virtual HRESULT __stdcall CreateServer(IUnknown **ppunkNew) = 0;

	// Constructor
	CProtocol();

	// Destructor
	virtual ~CProtocol();

private:
	long						m_cRef;
	LPVOID					m_pServerList;				// list of servers that we created
	CServer *				m_pCurrentServer;			// used by IEnum functions to keep track of next
	IUnknown *			m_pUnkCaller;					// IUnknown of our Creator so we may kneel and worship
	int							m_cNumServers;
};


#endif

