/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					ph.h
Description:		COM components for PH protocol server
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
#ifndef __PH_H__
#define __PH_H__

#include <objbase.h>
#include "private.h"
#include "protocol.h"
#include "wbase.h"
#include "dlghelper.h"
#include "utils.h"
#include "query.h"


// forward declarations
class CPHQuery;
class CAttribMap;

///////////////////////////////////////////////////////////////////////////////
// CLSID_DSPROTOCOLPH component


class CPHProtocol : public CProtocol
{
public:
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP	__stdcall GetProtocolImage(int iImage);
	virtual DWORD		__stdcall GetProtocolFlags();

	CPHProtocol();
	virtual ~CPHProtocol();

protected:
	virtual HRESULT __stdcall CreateServer(IUnknown **ppunkNew);
	virtual HRESULT __stdcall LoadServers();
  BOOL 											Write();

private:
	LPSTR						m_pszName;
	BOOL						m_bCreateSection;
  HBITMAP         m_hbmProtocolImage;
};


///////////////////////////////////////////////////////////////////////////////
// CPHServer


const int BUFFERLEN = 512;

class CPHServer : public CServer, IPrivateConfig
{

	friend class CPHProtocol;
	friend class CPHQuery;
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
	virtual LPSTR 	__stdcall GetHostName() 	{ return m_pszHostName; }
	virtual int		__stdcall GetPortNumber() { return m_nPort; }
	virtual LPSTR 	__stdcall ParseLine(LPSTR lpsz, int * pds);
    // smohanty: 04/21/98
    virtual LPCSTR __stdcall GetMailDomain() { return m_pszMailDomain; }
    virtual BOOL   __stdcall SetMailDomain(LPCSTR pszMailDomain);

	// constructor
	CPHServer(CPHProtocol *pProtocol, BOOL bInitialize=TRUE);
	virtual ~CPHServer();

	// dialog proc functions
	DEFEXPORTEDDLGPROC(PageProc1);
	DEFEXPORTEDDLGPROC(PageProc2);

protected:
  BOOL 						SetSection(LPSTR lpsz)  { FreeDup(m_pszSection, lpsz); 	return 1; }
	BOOL 		 	 			Write();
	BOOL 		 	 			Read();
	BOOL 		 	 			CreateUniqueSection();
	 
	BOOL 		 	 			AddAttribute(HWND hwndParent);

private:
	LPSTR						m_pszSection;		// (registry/file) section to store server data
	LPSTR						m_pszName;
	LPSTR						m_pszHostName;
	int							m_nPort;
	int							m_nPropPageCount;
	CAttribMap *		m_pAttributes;
	CPHProtocol *		m_pProtocol;		// back pointer to CPHProtocol object
	BOOL						m_bDirtyData;		// data is dirty-must be written out
	BOOL						m_bDirtyMap;		// map is dirty-must be written out
	IDirConfig *		m_pIDirConfig;
	LPSTR						m_pszGUID;
    LPSTR m_pszMailDomain;
	char						m_szBuffer[BUFFERLEN];	// a general purpose scratch buffer
};



///////////////////////////////////////////////////////////////////////////////
// CEditAttribDlg class

class CEditAttribDlg : public CPhDialog
{
public:
	constructor		CEditAttribDlg(HINSTANCE hInstance, HWND hwndParent, IDirConfig *pIDirConfig);
	destructor		~CEditAttribDlg();
	BOOL					DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	char					m_szAttribute[256];		// attribute value
	DS_FIELD			m_nType;							// the attribute type
	IDirConfig *	m_pIDirConfig;
};


///////////////////////////////////////////////////////////////////////////////
// CAttribMap class

class CAttribMap : public CPHRecord
{
public:
	CAttribMap();

	LPDBRECENT	Add(LPSTR psz, DS_FIELD nPresent, LPSTR pszUser=NULL);
	BOOL 				Write(IDirConfig * pIDirConfig, LPSTR pszSection);
	BOOL 				Read(IDirConfig * pIDirConfig, LPSTR pszSection);
	BOOL 				Reset();
  BOOL 				Erase(IDirConfig * pIDirConfig, LPSTR pszSection);
	LPDBRECENT 	GetNext(LPDBRECENT pRecEnt) { return (!pRecEnt) ? m_pRecordList : pRecEnt->pNext; }

protected:
	int					m_nCount;
};



#endif	//__PH_H__

