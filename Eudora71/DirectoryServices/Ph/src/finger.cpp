/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					finger.cpp
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
#pragma warning(disable : 4514 4710)
#include <afx.h>
#include <objbase.h>
#include <prsht.h>
#include <commctrl.h>
#include <assert.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"
#include "component.h"
#include "ustring.h"
#include "dlghelper.h"
#include "finger.h"
#include "fquery.h"
#include "configinit.h"
#include "resource.h"
#include "QCUtils.h"


#define MAXPORTNUM	999

const char szServerRoot[] = "FINGER";


///////////////////////////////////////////////////////////////////////////////
// CFingerProtocol


CFingerProtocol::CFingerProtocol()
    : _h_The_Bitmap(NULL)
{
	CUString str(IDS_FINGER);
	m_pszName = SafeStrdupMT(str);

	m_bCreateSection	= 1;
}
	
CFingerProtocol::~CFingerProtocol()
{
    if (_h_The_Bitmap != NULL)
		::DeleteObject(_h_The_Bitmap);

	delete [] m_pszName;
}


HRESULT	__stdcall CFingerProtocol::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
	UINT len = strlen(m_pszName);
	if (len > nBufferSize-1)
		return E_FAIL;
	strcpy(lpszBuffer, m_pszName);
	return S_OK;	
}

HBITMAP	__stdcall CFingerProtocol::GetProtocolImage(int /*iImage*/)
{
    if (_h_The_Bitmap == NULL) {
	_h_The_Bitmap = ::LoadBitmap(CFactory::s_hModule,
				     MAKEINTRESOURCE(IDB_FINGERIMAGESMALL));
    }
    return(_h_The_Bitmap);
}

DWORD		__stdcall CFingerProtocol::GetProtocolFlags()
{	
  return PROTO_RAWONLY; 
}


HRESULT __stdcall CFingerProtocol::CreateServer(IUnknown **ppunkNew)
{
	*ppunkNew = reinterpret_cast<IUnknown*>(DEBUG_NEW_NOTHROW CFingerServer(this, m_bCreateSection));
	if (!*ppunkNew)
		return E_OUTOFMEMORY;

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// CFingerProtocol Internal (non-interface) methods

HRESULT __stdcall CFingerProtocol::LoadServers()
{
	BOOL bAnyLoaded = 0;
	char buffer[80];
	DWORD dwSize;

	// first check if there are any servers installed
	for (;;) {
		dwSize = sizeof(buffer)-1;
		if (FAILED(m_pIDirConfig->EnumDirSections(buffer, &dwSize)))
			break;
		if (0 == strncmp(buffer, szServerRoot, sizeof(szServerRoot)-1))
			bAnyLoaded = 1;
	}

	// if there are not any servers loaded, install default server
	// information into the registry from the INI file
	if (!bAnyLoaded) {
		CFingerInit	* pInit = DEBUG_NEW_NOTHROW CFingerInit(m_pIDirConfig);
		pInit->LoadPreConfigData();
    delete pInit;
	}

	for (;;) {
		dwSize = sizeof(buffer)-1;
		if (FAILED(m_pIDirConfig->EnumDirSections(buffer, &dwSize)))
			break;
		if (0 == strncmp(buffer, szServerRoot, sizeof(szServerRoot)-1)) {
			CFingerServer * pIConfig;
			m_bCreateSection = 0;
			HRESULT hr = CreateObject(IID_IDSPConfig, (LPVOID*)&pIConfig);
			m_bCreateSection = 1;
			if (SUCCEEDED(hr)) {
				pIConfig->SetSection(buffer);
				BOOL bError = pIConfig->Read();
#ifdef _DEBUG
				if (bError)
					OutputDebugString("\nError reading server data!");
#endif
			}
			else
				return E_FAIL;
		}
	}

	return S_OK;
}

BOOL CFingerProtocol::Write()
{
	if (!m_pIDirConfig)
		return 0;

	m_pIDirConfig->SetDirStr(NULL, "ProtocolName", m_pszName);
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
// CFingerServer

// constructor
CFingerServer::CFingerServer(CFingerProtocol *pProtocol, BOOL bInitialize/*=TRUE*/)
{
	m_pProtocol				= pProtocol;
	m_pszSection			= NULL;
	m_pszName					= NULL;
	m_pszHostName			= NULL;
	m_nPort						= 79;
	m_nPropPageCount	= 1;
	m_pIDirConfig			= NULL;
	m_pszGUID					= NULL;
	m_bDirtyData			= 1;

	// hang onto the IDirConfig pointer
	m_pIDirConfig = m_pProtocol->GetIDirConfig();
	if (m_pIDirConfig)
		m_pIDirConfig->AddRef();

	// create the server's section in the registry
	if (bInitialize) {
		GUID guid;
		char buffer[50];
		if (SUCCEEDED(CoCreateGuid(&guid))) {
			CLSIDtochar(guid, buffer, sizeof(buffer));
			m_pszGUID = SafeStrdupMT(buffer);
		}
		CreateUniqueSection();
	}
}


CFingerServer::~CFingerServer()
{
	if (m_pIDirConfig)
		m_pIDirConfig->Release();

	delete [] m_pszName;
	delete [] m_pszHostName;
	delete [] m_pszSection;
	delete [] m_pszGUID;
}


///////////////////////////////////////////////////////////////////////////////
// IUnknown methods

HRESULT	__stdcall CFingerServer::QueryInterface(const IID& iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IDSPConfig*>(this);
	else
	if (iid == IID_IDSPConfig)
		*ppv = static_cast<IDSPConfig*>(this);
	else
	if (iid == IID_IPrivateConfig)
		*ppv = static_cast<IPrivateConfig*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}


ULONG		__stdcall CFingerServer::AddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}


ULONG __stdcall CFingerServer::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}



///////////////////////////////////////////////////////////////////////////////
// Internal (non-interface) methods


BOOL CFingerServer::CreateUniqueSection()
{
	int i = 0;
	BOOL bExists = 0;
	char buffer[40];

	if (!m_pIDirConfig)
		return 0;

	delete [] m_pszSection;
	m_pszSection = NULL;
	
	for (;;) {
		sprintf(buffer, "%s.%d", szServerRoot, i++);
		HRESULT hr = m_pIDirConfig->CreateDirSection(buffer, &bExists);
		if (FAILED(hr))
			return 0;
		if (!bExists)
			break;		
	}
	
	m_pszSection = SafeStrdupMT(buffer);

	return 1;
}


BOOL CFingerServer::Write()
{
    if (!m_pIDirConfig)
        return 0;

    if (m_bDirtyData) {
        CUString str;

        if (m_pszName != NULL) {
            str.Set(IDS_REGNAME);
            m_pIDirConfig->SetDirStr(m_pszSection, str, m_pszName);
        }

        if (m_pszHostName != NULL) {
            str.Set(IDS_REGHOSTNAME);
            m_pIDirConfig->SetDirStr(m_pszSection, str, m_pszHostName);
        }

        str.Set(IDS_REGPORT);
        m_pIDirConfig->SetDirInteger(m_pszSection, str, m_nPort);

        // save our GUID as the default (unnamed) value for the section
        m_pIDirConfig->SetDirStr(m_pszSection, NULL, m_pszGUID);
        m_bDirtyData = 0;
    }

    return 1;
}


BOOL CFingerServer::Read()
{
	HRESULT hr;
	BOOL bError = 0;
	const char szDef[] = "Unnamed";
	char buffer[80];
	DWORD dwSize;
	CUString str;

	if (!m_pIDirConfig || !m_pszSection)
		return 0;

	// read in the name
	buffer[0] = 0;
	dwSize = sizeof(buffer)-1;
	str.Set(IDS_REGNAME);
	hr = m_pIDirConfig->GetDirStr(m_pszSection, str, (LPSTR)szDef, sizeof(szDef), buffer, &dwSize);
	if (SUCCEEDED(hr) && dwSize) {
		m_pszName = SafeStrdupMT(buffer);
	}
	else
		bError |= 1;

	// read in the host name
	buffer[0] = 0;
	dwSize = sizeof(buffer)-1;
	str.Set(IDS_REGHOSTNAME);
	hr = m_pIDirConfig->GetDirStr(m_pszSection, str, (LPSTR)szDef, sizeof(szDef), buffer, &dwSize);
	if (SUCCEEDED(hr) && dwSize) {
		m_pszHostName = SafeStrdupMT(buffer);
	}
	else
		bError |= 1;

	// read in the port number
	str.Set(IDS_REGPORT);
	hr = m_pIDirConfig->GetDirInteger(m_pszSection, str, 105, (int*)&m_nPort);
	if (FAILED(hr))
		bError |= 1;

	// read in our GUID
	buffer[0] = 0;
	dwSize = sizeof(buffer)-1;
	hr = m_pIDirConfig->GetDirStr(m_pszSection, NULL, "", 0, buffer, &dwSize);
	if (SUCCEEDED(hr) && dwSize) {
		m_pszGUID = SafeStrdupMT(buffer);
	}
	else
		bError |= 1;
	
	return bError;
}

///////////////////////////////////////////////////////////////////////////////
// IPrivateConfig interface

LPSTR __stdcall CFingerServer::GetHostName()
{
	return m_pszHostName;
}

int __stdcall CFingerServer::GetPortNumber()
{
	return m_nPort;
}

LPSTR __stdcall CFingerServer::ParseLine(LPSTR /*lpsz*/, int * /*pds*/)
{
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// IDSPConfig methods


HRESULT	__stdcall CFingerServer::GetName(LPSTR lpszBuffer, UINT nBufferSize)
{
	if (!lpszBuffer || nBufferSize < 1)
		return E_INVALIDARG;
	
	if (m_pszName) {
		UINT len = strlen(m_pszName);
		if (len > nBufferSize-1)
			return E_FAIL;
		strcpy(lpszBuffer, m_pszName);
	}
	else
		lpszBuffer[0] = 0;		// return an empty string
		
	return S_OK;	
}


HBITMAP	__stdcall CFingerServer::GetImage(int /*iImage*/)
{
	return NULL; 
}


LPSTR		__stdcall CFingerServer::GetID()
{
	return m_pszGUID;
}


HRESULT	__stdcall CFingerServer::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
	return m_pProtocol ? m_pProtocol->GetProtocolName(lpszBuffer, nBufferSize) : E_FAIL;
}


HBITMAP	__stdcall CFingerServer::GetProtocolImage(int iImage)
{
	return m_pProtocol ? m_pProtocol->GetProtocolImage(iImage) : NULL;
}


DWORD	__stdcall CFingerServer::GetProtocolFlags()
{
	return m_pProtocol ? m_pProtocol->GetProtocolFlags() : 0;
}


HRESULT __stdcall CFingerServer::DeleteData()
{
	if (!m_pIDirConfig || !m_pszSection)
		return E_FAIL;

	m_pIDirConfig->DeleteDirSection(m_pszSection);
	return S_OK;
}


HRESULT __stdcall CFingerServer::CreateObject(REFIID riid, LPVOID *ppv)
{
	if (!ppv)
		return E_INVALIDARG;

	if (riid == IID_IDSPQuery) {
		CFingerQuery * pQuery = DEBUG_NEW_NOTHROW CFingerQuery(reinterpret_cast<IUnknown*>(this));
		if (!pQuery)
			return E_OUTOFMEMORY;
		pQuery->SetDatabaseID(m_pszGUID);
		*ppv = pQuery;
		return S_OK;
	}
	else	
		return E_NOINTERFACE;
}


int	__stdcall CFingerServer::GetPropPageCount()
{
	return m_nPropPageCount;
}


HRESULT	__stdcall CFingerServer::GetPropPages(HPROPSHEETPAGE *paHPropPages)
{
	PROPSHEETPAGE psp;
	psp.dwSize 			= sizeof(psp);
	psp.dwFlags 		= PSP_DEFAULT;
	psp.hInstance 	= CFactory::s_hModule;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_FINGERPROPPAGE1);
	psp.pfnDlgProc 	= EXPORTEDDLGPROC(PageProc1);
	psp.lParam 			= (LONG)this;

	*paHPropPages = CreatePropertySheetPage(&psp);
	if (!*paHPropPages)
		return E_FAIL;

	return S_OK;
}


////////////////////////////////////////////////////////////
// Make these non-static class methods callable from Windows

MAKEEXPORTEDPAGEPROC(CFingerServer, PageProc1)


///////////////////////////////////////////////////////////////////////////////
// Dialog helper functions

///////////////////////////////////////////////////////////////////////////////
// Property page dialog procs

// Page 1 (Network)
BOOL CFingerServer::PageProc1(HWND hwndDlg, UINT uMsg, WPARAM /*wParam*/, LPARAM lParam)
{
	switch (uMsg) {
		
		case WM_INITDIALOG: {
			SetDlgItemText(hwndDlg, IDC_NAME, m_pszName);
			SetDlgItemText(hwndDlg, IDC_HOSTNAME, m_pszHostName);
			SetDlgItemInt(hwndDlg, IDC_PORT, m_nPort, 0);
			return TRUE;
		}

		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				
				case PSN_KILLACTIVE: {
          BOOL bErrorOnPage = CheckEditFieldEmpty(hwndDlg, IDC_HOSTNAME, IDS_ERRHOSTNAME);
					if (!bErrorOnPage)
						bErrorOnPage = CheckEditFieldInt(hwndDlg, IDC_PORT, 0, MAXPORTNUM, IGNOREMAX);
					SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)bErrorOnPage);
					return TRUE;
				}

				case PSN_APPLY: {
					CUString str(hwndDlg, IDC_NAME);
//					SetName(str);
					FreeDup(m_pszName, str);

					str.Set(hwndDlg, IDC_HOSTNAME);
//					SetHostName(str);
					FreeDup(m_pszHostName, str);

					BOOL bTrans;
					m_nPort = (int)GetDlgItemInt(hwndDlg, IDC_PORT, &bTrans, FALSE);
					m_bDirtyData = 1;
					Write();
					return TRUE;
				}
			} // switch
			break;

	}	// switch(uMsg)

	return FALSE;
}



