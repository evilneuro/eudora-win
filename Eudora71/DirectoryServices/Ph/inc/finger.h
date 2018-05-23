/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					finger.h
Description:		COM components for Finger protocol server
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
#ifndef __FINGER_H__
#define __FINGER_H__

#include <objbase.h>
#include "protocol.h"
#include "dlghelper.h"
#include "private.h"
#include "utils.h"
#include "wbase.h"


///////////////////////////////////////////////////////////////////////////////
// CLSID_DSPROTOCOLFINGER component


class CFingerProtocol : public CProtocol
{
public:
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP	__stdcall GetProtocolImage(int iImage);
	virtual DWORD		__stdcall GetProtocolFlags();

	CFingerProtocol();
	virtual ~CFingerProtocol();

protected:
	virtual HRESULT __stdcall CreateServer(IUnknown **ppunkNew);
	virtual HRESULT __stdcall LoadServers();

	BOOL 											Write();

private:
	LPSTR						m_pszName;
	BOOL						m_bCreateSection;
        HBITMAP                                         _h_The_Bitmap;
};


///////////////////////////////////////////////////////////////////////////////
// CFingerServer


class CFingerServer : public CServer, IPrivateConfig
{
	friend class CFingerProtocol;
public:
  	// IUnknown
	virtual HRESULT	__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	// IDSPConfig
	virtual HRESULT	__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP	__stdcall GetImage(int iImage);
	virtual LPSTR		__stdcall GetID();
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP	__stdcall GetProtocolImage(int iImage);
	virtual DWORD		__stdcall GetProtocolFlags();
	virtual int			__stdcall GetPropPageCount();
	virtual HRESULT	__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages);
	virtual HRESULT __stdcall DeleteData();
	virtual HRESULT __stdcall CreateObject(REFIID riid,LPVOID *ppv);
	// IPrivateConfig interface
	virtual LPSTR __stdcall GetHostName();
	virtual int   __stdcall GetPortNumber();
	virtual LPSTR __stdcall ParseLine(LPSTR lpsz, int * pds);
    // smohanty: 04/21/98
    virtual LPCSTR __stdcall GetMailDomain() { return NULL; }
    virtual BOOL   __stdcall SetMailDomain(LPCSTR) {return FALSE;}

	// constructor
	CFingerServer(CFingerProtocol *pProtocol, BOOL bInitialize=TRUE);
	virtual ~CFingerServer();

	// dialog proc functions
	DEFEXPORTEDDLGPROC(PageProc1);
//	DEFEXPORTEDDLGPROC(PageProc2);

protected:
  BOOL 						CreateUniqueSection();
//	BOOL 						SetHostName(LPSTR lpsz) { FreeDup(m_pszHostName, lpsz); return 1; }
//	BOOL 						SetName(LPSTR lpsz)			{ FreeDup(m_pszName, lpsz); 		return 1; }
	BOOL 						SetSection(LPSTR lpsz)  { FreeDup(m_pszSection, lpsz); 	return 1; }
	BOOL 						Write();
	BOOL 						Read();

private:
	LPSTR						m_pszSection;		// (registry/file) section to store server data
	LPSTR						m_pszName;
	LPSTR						m_pszHostName;
	UINT						m_nPort;
	int							m_nPropPageCount;
	CFingerProtocol *		m_pProtocol;		// back pointer to CFingerProtocol object
	IDirConfig *		m_pIDirConfig;
	LPSTR						m_pszGUID;
	BOOL						m_bDirtyData;

//	char						m_szBuffer[BUFFERLEN];	// a general purpose scratch buffer
};



#endif	// __FINGER_H__



