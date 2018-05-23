/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					ph.cpp
Description:		Implementation of PH directory services object
Date:						7/22/97
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
#pragma warning(disable : 4514 4706 4710)
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
#include "configinit.h"
#include "trace.h"
#include "ph.h"
#include "resource.h"
#include "QCUtils.h"


const char szServerRoot[] = "PH";

#define MAXPORTNUM	999

#define DBRETOLPSZ(p)		((LPSTR)p->data)


///////////////////////////////////////////////////////////////////////////////
// CPHProtocol methods

CPHProtocol::CPHProtocol()
{
	m_pszName = LoadDup(CFactory::s_hModule, IDS_PH);
	m_bCreateSection	= 1;
  m_hbmProtocolImage = NULL;
}


CPHProtocol::~CPHProtocol()
{
	delete [] m_pszName;

  if (m_hbmProtocolImage)                 // we're calling the Win32 DeleteObject() here
    ::DeleteObject(m_hbmProtocolImage);   // - not IDSPProtocol::DeleteObject() !!      
}


HRESULT	__stdcall CPHProtocol::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
	UINT len = strlen(m_pszName);
	if (len > nBufferSize-1)
		return E_FAIL;
	strcpy(lpszBuffer, m_pszName);
	return S_OK;	
}


HBITMAP	__stdcall CPHProtocol::GetProtocolImage(int /* iImage */)
{
  if (!m_hbmProtocolImage)
	  m_hbmProtocolImage = LoadBitmap(CFactory::s_hModule, MAKEINTRESOURCE(IDB_PHIMAGESMALL));
  return m_hbmProtocolImage;
}
	

DWORD	__stdcall CPHProtocol::GetProtocolFlags()
{	
  return PROTO_RAW;     // we support raw queries
}


HRESULT __stdcall CPHProtocol::CreateServer(IUnknown **ppunkNew)
{
	*ppunkNew = reinterpret_cast<IUnknown*>(DEBUG_NEW_NOTHROW CPHServer(this, m_bCreateSection));
	if (!*ppunkNew)
		return E_OUTOFMEMORY;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// CPHProtocol Internal (non-interface) methods


HRESULT __stdcall CPHProtocol::LoadServers()
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
		CPHInit	* pInit = DEBUG_NEW CPHInit(m_pIDirConfig);

		pInit->LoadPreConfigData();
    delete pInit;
	}

	// now create all servers out of registry data
	for (;;) {
		dwSize = sizeof(buffer)-1;
		if (FAILED(m_pIDirConfig->EnumDirSections(buffer, &dwSize)))
			break;
		if (0 == strncmp(buffer, szServerRoot, sizeof(szServerRoot)-1)) {
			CPHServer * pIConfig;
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


BOOL CPHProtocol::Write()
{
	if (!m_pIDirConfig)
		return 0;

	m_pIDirConfig->SetDirStr(NULL, "ProtocolName", m_pszName);
	return 1;
}



///////////////////////////////////////////////////////////////////////////////
// CPHServer methods

CPHServer::CPHServer(CPHProtocol *pProtocol, BOOL bInitialize/*=TRUE*/)
{
	m_pProtocol				= pProtocol;
	m_pszName					= NULL;
	m_pszHostName 		= NULL;
	m_nPort 					= 105;
	m_nPropPageCount 	= 2;
	m_pAttributes			= DEBUG_NEW CAttribMap();

	m_pszSection 			= NULL;
	m_bDirtyMap 			= 1;
	m_bDirtyData 			= 1;
	m_pszGUID 				= NULL;
	// smohanty:04/21/98
	// Added m_pszMailDomain field.
	m_pszMailDomain = NULL;

	// by default we should have these two entries in the attribute table
	if (bInitialize) {
		m_pAttributes->Add("name: ", DS_NAME);
		m_pAttributes->Add("email: ", DS_EMAIL);
	}

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


CPHServer::~CPHServer()
{
	if (m_pIDirConfig)
		m_pIDirConfig->Release();

	delete [] m_pszName;
	delete [] m_pszHostName;
	delete [] m_pszSection;
	delete [] m_pszGUID;
	delete [] m_pszMailDomain;
	delete m_pAttributes;
}


//////////////////////
// IUnknown methods

HRESULT __stdcall CPHServer::QueryInterface(REFIID iid, void** ppv)
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

ULONG __stdcall CPHServer::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CPHServer::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}

///////////////////////////////////////////////////////////////////////////////
// IPrivateConfig interface

LPSTR __stdcall CPHServer::ParseLine(LPSTR lpsz, int * pds)
{
	LPSTR pszName, pszAttrib;
	LPDBRECENT pRecEnt;

  for (lpsz; *lpsz == ' '; lpsz++) 	// strip white space
    ;

	for (pRecEnt=NULL; pRecEnt=m_pAttributes->GetNext(pRecEnt); ) {
		pszName = DBRETOLPSZ(pRecEnt);
		pszAttrib = strstr(lpsz, pszName);
		if (pszAttrib) {
			if (pds)
				*pds = pRecEnt->nName;
      // skip past field name 
			pszAttrib += strlen(pszName);
      // skip white space
      for (; *pszAttrib == ' '; pszAttrib++)
        ;
      // return the next token
			return pszAttrib;
		}
	}

	// if we didn't find an attribute...
	if (pds)
		*pds = DS_UNKNOWN;
  
  return lpsz;
}

// smohanty: 04/21/98
BOOL __stdcall CPHServer::SetMailDomain(LPCSTR pszMailDomain)
{
	delete [] m_pszMailDomain;
    m_pszMailDomain = NULL;
 
	m_pszMailDomain = SafeStrdupMT(pszMailDomain);
    return (m_pszMailDomain != NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Internal (non-interface) methods


BOOL CPHServer::CreateUniqueSection()
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

		if (!bExists) {
			// save our GUID as the default (unnamed) value for the section
			m_pIDirConfig->SetDirStr(buffer, NULL, m_pszGUID);
			break;		
		}
	}
	
	m_pszSection = SafeStrdupMT(buffer);

	return 1;
}


BOOL CPHServer::Write()
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
		m_bDirtyData = 0;
	}

	if (m_bDirtyMap) {
		m_pAttributes->Write(m_pIDirConfig, m_pszSection);
		m_bDirtyMap = 0;
	}

	return 1;
}


BOOL CPHServer::Read()
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
	if (SUCCEEDED(hr) && dwSize)
		m_pszName = SafeStrdupMT(buffer);
	else
		bError |= 1;

	// read in the host name
	buffer[0] = 0;
	dwSize = sizeof(buffer)-1;
	str.Set(IDS_REGHOSTNAME);
	hr = m_pIDirConfig->GetDirStr(m_pszSection, str, (LPSTR)szDef, sizeof(szDef), buffer, &dwSize);
	if (SUCCEEDED(hr) && dwSize)
		m_pszHostName = SafeStrdupMT(buffer);
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
	if (SUCCEEDED(hr) && dwSize)
		m_pszGUID = SafeStrdupMT(buffer);
	else
		bError |= 1;
	
	// read in the attribute map
	if (m_pAttributes)
    m_pAttributes->Read(m_pIDirConfig, m_pszSection);

	return bError;
}



///////////////////////////////////////////////////////////////////////////////
// IDSPConfig methods


HRESULT __stdcall CPHServer::DeleteData()
{
	if (!m_pIDirConfig || !m_pszSection)
		return E_FAIL;

	m_pIDirConfig->DeleteDirSection(m_pszSection);
	return S_OK;
}



HRESULT	__stdcall CPHServer::GetName(LPSTR lpszBuffer, UINT nBufferSize)
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


HBITMAP __stdcall CPHServer::GetImage(int /* iImage */)
{ 
	return NULL; 
}


LPSTR	__stdcall CPHServer::GetID()
{
	return m_pszGUID;
}


HRESULT	__stdcall CPHServer::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
	return m_pProtocol ? m_pProtocol->GetProtocolName(lpszBuffer, nBufferSize) : E_FAIL;
}


HBITMAP __stdcall CPHServer::GetProtocolImage(int iImage)
{
	return m_pProtocol ? m_pProtocol->GetProtocolImage(iImage) : NULL;
}


DWORD	__stdcall CPHServer::GetProtocolFlags()
{
	return m_pProtocol ? m_pProtocol->GetProtocolFlags() : 0;
}


// ----------------------------------------------------------------------- 
// CreateObject()																		
// Create object is used to create new instances of:
// IID_Query		Creates an object to begin a new query transaction
// IID_IPost		Creates an object to begin a new post transaction
// -----------------------------------------------------------------------
HRESULT __stdcall CPHServer::CreateObject(REFIID riid, LPVOID *ppv)
{
	if (!ppv)
		return E_INVALIDARG;

	if (riid == IID_IDSPQuery) {
		CPHQuery * pQuery = DEBUG_NEW_NOTHROW CPHQuery(reinterpret_cast<IUnknown*>(this));
		if (!pQuery)
			return E_OUTOFMEMORY;

		pQuery->SetDatabaseID(m_pszGUID);
		*ppv = pQuery;
		return S_OK;
	}
	else	
		return E_NOINTERFACE;
}


int __stdcall CPHServer::GetPropPageCount()
{ 
	return m_nPropPageCount; 
}


HRESULT __stdcall CPHServer::GetPropPages(HPROPSHEETPAGE *paHPropPages)
{ 
	PROPSHEETPAGE psp = {0};

	psp.dwSize 			= sizeof(psp);
	psp.dwFlags 		= PSP_DEFAULT;
	psp.hInstance 	= CFactory::s_hModule;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PHPROPPAGE1);
	psp.pfnDlgProc 	= EXPORTEDDLGPROC(PageProc1);
	psp.lParam 			= (LONG)this;

	*paHPropPages = CreatePropertySheetPage(&psp);
	if (!*paHPropPages)
		return E_FAIL;

	paHPropPages++;
  psp.dwSize 			= sizeof(psp);
  psp.dwFlags 		= PSP_DEFAULT;
  psp.hInstance 	= CFactory::s_hModule;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PHPROPPAGE2);
	psp.pfnDlgProc 	= EXPORTEDDLGPROC(PageProc2);
  psp.lParam 			= (LONG)this;

	*paHPropPages = CreatePropertySheetPage(&psp);
	if (!*paHPropPages)
		return E_FAIL;

	return S_OK;
}


////////////////////////////////////////////////////////////
// Make these non-static class methods callable from Windows

MAKEEXPORTEDPAGEPROC(CPHServer, PageProc1)
MAKEEXPORTEDPAGEPROC(CPHServer, PageProc2)

///////////////////////////////////////////////////////////////////////////////
// Property page dialog procs

// Page 1 (Network)
BOOL CPHServer::PageProc1(HWND hwndDlg, UINT uMsg, WPARAM /* wParam */, LPARAM lParam)
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
					FreeDup(m_pszName, str);

					str.Set(hwndDlg, IDC_HOSTNAME);
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


// Page 2 (Atributes)
BOOL CPHServer::PageProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		
		case WM_INITDIALOG: {
			HWND hwndList = GetDlgItem(hwndDlg, IDC_LVATTRIBUTE);

      HIMAGELIST himlSmall = ImageList_LoadBitmap(CFactory::s_hModule, MAKEINTRESOURCE(IDB_INVISIBLE), 1, 1, RGB(255,255,255));
      ListView_SetImageList(hwndList, himlSmall, LVSIL_SMALL);

      DWORD dwExStyle = ListView_GetExtendedListViewStyle(hwndList);
      ListView_SetExtendedListViewStyle(hwndList, dwExStyle|LVS_EX_FULLROWSELECT);

			SetListViewColumn(hwndList, 0, IDS_ATTRIBNAME,  100);
			SetListViewColumn(hwndList, 1, IDS_PRESENTNAME, 300);   // last column will be auto-sized below

			for (LPDBRECENT pRecEnt=NULL; pRecEnt=m_pAttributes->GetNext(pRecEnt); ) {
				char buffer[100];
				if (SUCCEEDED(m_pIDirConfig->GetFieldName(pRecEnt->nName, buffer, sizeof(buffer)-1)))
					SetListViewEntry(hwndList, DBRETOLPSZ(pRecEnt), buffer, (int)pRecEnt->nName);
			}
      ListView_AutoResizeLastColumn(hwndList);
			return TRUE;
		}

		case WM_COMMAND:
			switch (wParam) {
				case IDC_ADD:
          AddAttribute(hwndDlg);
					break;

				case IDC_EDIT: {
					HWND hwndList = GetDlgItem(hwndDlg, IDC_LVATTRIBUTE);
          ListView_EditFocused(hwndList);
					break;
				}

				case IDC_REMOVE: {
					HWND hwndList = GetDlgItem(hwndDlg, IDC_LVATTRIBUTE);
					int iIndex = ListView_GetFocusedItem(hwndList);
					ListView_DeleteItem(hwndList, iIndex);
				}
			}
			break;

		case WM_NOTIFY: {
			NMHDR * phdr = (NMHDR*)lParam;
			switch (phdr->code) {
				
				case PSN_APPLY: {
					CAttribMap * pNewAttrib = DEBUG_NEW CAttribMap();
					HWND hwndList = GetDlgItem(hwndDlg, IDC_LVATTRIBUTE);
					int nCount = ListView_GetItemCount(hwndList);
					for (int i=0; i < nCount; i++) {
						m_szBuffer[0] = 0;
						ListView_GetItemText(hwndList, i, 0, m_szBuffer, sizeof(m_szBuffer)-1);
						LPSTR psz = strlen(m_szBuffer) ? m_szBuffer : NULL;
						if (psz) {
							DS_FIELD nPresent = (DS_FIELD)ListView_GetItemData(hwndList, i);
							pNewAttrib->Add(psz, nPresent);
						}
					}
					
					m_pAttributes->Erase(m_pIDirConfig, m_pszSection);		// erase the old map entries
					delete m_pAttributes;																	// delete the old CAttribMap object
					m_pAttributes = pNewAttrib;														// hang on the the new one
					m_bDirtyMap = 1;																			// make sure it gets written out
					Write();
					return TRUE;
				}
					
				case LVN_BEGINLABELEDIT:
					break;

				case LVN_ENDLABELEDIT: {
					SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}

				case LVN_KEYDOWN: {
 					LV_KEYDOWN * pkd = (LV_KEYDOWN*)lParam;
					if (pkd->wVKey == VK_F2) {
						HWND hwndList = GetDlgItem(hwndDlg, IDC_LVATTRIBUTE);
						ListView_EditFocused(hwndList);
					}
					break;
				}

				default:
					return FALSE;
			}
			break;
		}

	}

	return FALSE;
}



BOOL CPHServer::AddAttribute(HWND hwndParent)
{
	CEditAttribDlg dlg(CFactory::s_hModule, hwndParent, m_pIDirConfig);
	dlg.m_szAttribute[0] = 0;
	dlg.m_nType = DS_NAME;
	if (IDOK == dlg.DoModal()) {
		HWND hwndList = GetDlgItem(hwndParent, IDC_LVATTRIBUTE);
		char buffer[100];
		if (SUCCEEDED(m_pIDirConfig->GetFieldName(dlg.m_nType, buffer, sizeof(buffer)-1)))
			SetListViewEntry(hwndList, dlg.m_szAttribute, buffer, (int)dlg.m_nType);
	}

	return 1;
}

		

///////////////////////////////////////////////////////////////////////////////
CEditAttribDlg::CEditAttribDlg(HINSTANCE hInstance, HWND hwndParent, IDirConfig *pIDirConfig)
	: CPhDialog(hInstance, IDD_EDITATTRIB1, hwndParent)
{
	assert(pIDirConfig);
	m_pIDirConfig = pIDirConfig;	
	m_pIDirConfig->AddRef();
}


CEditAttribDlg::~CEditAttribDlg()
{
	m_pIDirConfig->Release();
}


BOOL CEditAttribDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM /* lParam */)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			for (int i=(int)DS_NAME; i < (int)DS_MAXFIELD-2; i++) {
				char buffer[100];
				if (SUCCEEDED(m_pIDirConfig->GetFieldName(DS_FIELD(i), buffer, sizeof(buffer)-1)))
					SendDlgItemMessage(m_hDlg, IDC_PRESENTEDAS, LB_ADDSTRING, 0, (LPARAM)buffer);
			}

			SetDlgItemText(m_hDlg, IDC_ATTRIBNAME, m_szAttribute);
			SendDlgItemMessage(m_hDlg, IDC_PRESENTEDAS, LB_SETCURSEL, m_nType, 0);
			break;
		}

		case WM_COMMAND:
			switch (wParam) {
				case IDOK:
					if (wParam == IDOK) {
            if (CheckEditFieldEmpty(m_hDlg, IDC_ATTRIBNAME, IDS_ERRATTRIBNAME))
              return 1;
            m_szAttribute[0] = 0;
						GetDlgItemText(m_hDlg, IDC_ATTRIBNAME, m_szAttribute, sizeof(m_szAttribute)-1);
						m_nType = (DS_FIELD)SendDlgItemMessage(m_hDlg, IDC_PRESENTEDAS, LB_GETCURSEL, 0, 0);
					}
				// fall through...
				case IDCANCEL:
					EndDialog(wParam);
					return 1;
			}
			break;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// CAttribMap 

// This class, which is derived from CPHRecord, defines an object that 
// manages a list of DBRECENT structures. Note that the contents of the
// list can only be changed by appending to the  list or by clearing 
// the entire list. Single element deletions or insertions  somewhere 
// other than at the end of the list are not allowed.


CAttribMap::CAttribMap()
{
	m_nCount = 0;
}

LPDBRECENT CAttribMap::Add(LPSTR psz, DS_FIELD nPresent, LPSTR pszUser/*=NULL*/)
{
	LPDBRECENT pRecEnt = AddStringRecEnt(psz, nPresent, pszUser);
	m_nCount++;
	return pRecEnt;
}


// Remove all attributes in this map from the registry
BOOL CAttribMap::Erase(IDirConfig * pIDirConfig, LPSTR pszSection)
{
	if (pIDirConfig && pszSection && strlen(pszSection)) {
		for (LPDBRECENT pRecEnt=NULL; pRecEnt=GetNext(pRecEnt); ) {
			LPSTR pszName = DBRETOLPSZ(pRecEnt);
			if (pszName && strlen(pszName))
				pIDirConfig->DeleteDirKey(pszSection, pszName);
		}
	}

	return 1;
}


// Write all attributes to the registry (actually to IDirConfig)
BOOL CAttribMap::Write(IDirConfig * pIDirConfig, LPSTR pszSection)
{
	if (pIDirConfig && pszSection && strlen(pszSection)) {
		for (LPDBRECENT pRecEnt=NULL; pRecEnt=GetNext(pRecEnt); ) {

			LPSTR pszName = DBRETOLPSZ(pRecEnt);
			char buffer[100];
			if (SUCCEEDED(pIDirConfig->GetFieldKey(pRecEnt->nName, buffer, sizeof(buffer)-1))) {
				if (!strlen(buffer))
					continue;
			if (pszName && strlen(pszName))
				pIDirConfig->SetDirStr(pszSection, pszName, buffer);
			}
		}
	}

	return 1;
}

BOOL CAttribMap::Read(IDirConfig * pIDirConfig, LPSTR pszSection)
{
	char 	szName[256];
	char	szValue[256];

	// To read in the attributes, we must EnumDirKeys for all items that do not start with a '.'
	for (;;) {
		DataType	dt;
		DWORD dwNameSize = sizeof(szName)-1;
		if (FAILED(pIDirConfig->EnumDirKeys(pszSection, szName, &dwNameSize, &dt)))
			break;
		DWORD dwValueSize = sizeof(szValue)-1;
		if (strlen(szName) && *szName != '.' && *(szName+1) != '.') {
			if (SUCCEEDED(pIDirConfig->GetDirStr(pszSection, szName, "", 0, szValue, &dwValueSize))) {
				DS_FIELD ds;
				if (SUCCEEDED(pIDirConfig->MapFieldKey(&ds, szValue)))
					Add(szName, ds);
			}
		}
	}
	pIDirConfig->EndEnumDirKeys(pszSection);
	return 1;
}


BOOL CAttribMap::Reset()
{
	FreeRecordList();
	m_nCount = 0;
	return TRUE;
}



