/******************************************************************************/
/*																										*/
/*	Name		:	DS.CPP			  																*/
/* Date     :  7/25/1997                                                      */
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

/*	Desc.		:	Directory Services Object implementation								*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "ds.h"
#include "resource.h"


extern long g_cComponents;

void __stdcall OleError(HRESULT hErr,LPSTR pszTitle)
{
    char		szTitle[128]= {0};
    LPVOID 	lpMsgBuf 	= NULL;

    FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			GetScode(hErr),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
	);	
	
    /* If no title, use default																*/
    if(!pszTitle)
	LoadString((HINSTANCE)CFactory::s_hModule,IDS_ERR_OLE,szTitle,sizeof(szTitle));
	
    /* Did they pass a resource ID instead of a real string???						*/
    else if(!HIWORD(pszTitle))
	LoadString((HINSTANCE)CFactory::s_hModule,LOWORD(pszTitle),szTitle,sizeof(szTitle));
	
    else
	strcpy(szTitle,pszTitle);
	
    ::MessageBox(NULL,(LPSTR)lpMsgBuf,szTitle,MB_OK|MB_ICONINFORMATION);
	
    // Free the buffer.
    if(lpMsgBuf)
	LocalFree(lpMsgBuf);
}


IUnknown * CDirServ::CreateServInstance()
{
    CDirServ *pDS = DEBUG_NEW_NOTHROW CDirServ;
	
    if(pDS) {
		return(static_cast<IUnknown *>(pDS));
    }
    return(NULL);
}

CDirServ::CDirServ() : m_cRef(1)
{
//#define _SHOWOBJECTSIZE 1
#ifdef _SHOWOBJECTSIZE
    char	szDbg[256];

    wsprintf(szDbg,"CDirServ: %d\nCDSDatabase: %d\nCDSProtocol: %d\nCQuery: %ld\nCErrorRecord: %d",
	     sizeof(CDirServ),
	     sizeof(CDSDatabase),
	     sizeof(CDSProtocol),
	     sizeof(CQuery),
	     sizeof(CErrorRecord)
	);
    ::MessageBox(NULL,szDbg,"Info",MB_OK);
#endif
    m_pProtocolList	= NULL;
    m_Initialized		= FALSE;
    InterlockedIncrement(&g_cComponents);
}

CDirServ::~CDirServ()
{
    if(m_pProtocolList) m_pProtocolList->Release();
    m_pProtocolList = NULL;
    InterlockedDecrement(&g_cComponents);
}

HRESULT	__stdcall CDirServ::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == IID_IUnknown)
	*ppv = static_cast<IUnknown*>(this);
    else
	if (iid == IID_IDirServ)
	    *ppv = static_cast<IDirServ*>(this);
	else {
	    *ppv = NULL;
	    return E_NOINTERFACE;
	}
	
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;
}

ULONG __stdcall CDirServ::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDirServ::Release()
{
    if (0 == InterlockedDecrement(&m_cRef)) {
	delete this;
	return 0;
    }
    return m_cRef;
}

/******************************************************************************/
/* List of protocol classes to load can be found at:									*/
/* HKEY_CURRENT_USER/Qualcom/Ds/Protocols													*/
/*											{CLSID1}													*/
/*                               ...		                                    */
/*                               {CLSIDn}                                     */
/******************************************************************************/
#define PROTOCOL_CLASSES "Software\\Qualcomm\\Shared\\DirServices\\Drivers"

/* Private */
HRESULT __stdcall CDirServ::Initialize()
{
    HKEY		hKey;
    HRESULT	hErr;
    BOOL		bSucceeded	= FALSE;
	
    if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pProtocolList))) {
		
	m_pProtocolList->Initialize(CDirServ::ProtoFreeCB,NULL);
	if(ERROR_SUCCESS == (hErr = RegOpenKey(HKEY_CURRENT_USER,PROTOCOL_CLASSES,&hKey))) {
	    char	szCLSID[128];
	    DWORD	nSubKeys = 0;
	    DWORD	i;
			
	    RegQueryInfoKey(hKey,NULL,NULL,NULL,&nSubKeys,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	    for(i=0;i<nSubKeys;i++) {
		DWORD	nKeyLen	= sizeof(szCLSID);
				
		RegEnumKeyEx(hKey,i,szCLSID,&nKeyLen,NULL,NULL,NULL,NULL);
		if(FAILED(hErr =  LoadProtocol(szCLSID))) {
//					OleError(hErr,MAKEINTRESOURCE(IDS_ERR_LOADPROTO));
		}
		else bSucceeded = TRUE;
	    }
	    RegCloseKey(hKey);
			
	    if(TRUE == bSucceeded) {
		hErr = S_OK;
	    }
	}
    }
    return(hErr);
}

/* Private */
void CDirServ::ProtoFreeCB(LPITEM pItem, LPVOID /* pUser */)
{
    LPPENT	pEnt = (LPPENT) pItem;
	
    pEnt->pDSP->HarryCarry();
    pEnt->pDSP->Release();

    delete pEnt;
}

/* Private */
/* Note: the CDSProtocol object has already been AddRef'd							*/
HRESULT __stdcall CDirServ::AddProtocol(CDSProtocol *pProto)
{
    LPPENT pEnt = DEBUG_NEW_NOTHROW PENT;
	
    if(pEnt) {
		pEnt->pDSP = pProto;
		m_pProtocolList->Attach(pEnt);
		return S_OK;
    }
    return E_OUTOFMEMORY;
}

/* Private */
HRESULT __stdcall CDirServ::LoadProtocol(LPSTR pszCLSID)
{
    HRESULT		hErr = E_OUTOFMEMORY;
    CDSProtocol	*pProto = DEBUG_NEW_NOTHROW CDSProtocol;
    char			szTmp[1024] = {0};
	
    if(pProto) {
		lstrcpy(szTmp,PROTOCOL_CLASSES);
		lstrcat(szTmp,"\\");
		lstrcat(szTmp,pszCLSID);
		if(SUCCEEDED(pProto->Initialize(szTmp,pszCLSID))) {
			if(FAILED(hErr = AddProtocol(pProto))) {
			pProto->Release();
			}
		}
    }
    return(hErr);
}

HRESULT __stdcall CDirServ::EnumProtocols(IDSEnumProtocols ** ppIEnum)
{
    /* CDSEnumProtocols's ref count is initialized to 1, no AddRef() needed		*/
    *ppIEnum = static_cast<IDSEnumProtocols*>(DEBUG_NEW_NOTHROW CDSEnumProtocols(m_pProtocolList));
	
    if(*ppIEnum) {
		return(S_OK);
    }
	
    return(E_OUTOFMEMORY);
}


HRESULT __stdcall CDirServ::EnumDatabases(IDSProtocol *pProto,IDSEnumDatabases ** ppIEnum)
{
    /* CDSEnumDatabases's ref count is initialized to 1, no AddRef() needed		*/
    *ppIEnum = static_cast<IDSEnumDatabases*>(DEBUG_NEW_NOTHROW CDSEnumDatabases(m_pProtocolList,static_cast<CDSProtocol*>(pProto),FALSE));
	
    if(*ppIEnum) {
		return(S_OK);
    }
	
    return(E_OUTOFMEMORY);
}

HRESULT __stdcall CDirServ::CreateDatabase(IDSProtocol *pProto,IDSDatabase **ppDatabase)
{
    IDSPConfig	*pConfig		= NULL;
    CDSProtocol	*pcProto		= (CDSProtocol *) pProto;
    HRESULT		hErr;
	
    if(!pProto || !ppDatabase)
	return E_FAIL;
	
    /* Ask the Database driver to create an empty database definition object	*/
    if(SUCCEEDED(hErr = pcProto->CreateObject(IID_IDSPConfig,(void **)&pConfig))) {
		*ppDatabase = static_cast<IDSDatabase*>(DEBUG_NEW_NOTHROW CDSDatabase(pConfig));
			
		if(!(*ppDatabase)) {
			hErr = E_OUTOFMEMORY;
			pConfig->Release();
		}
    }
    return hErr;
}


HRESULT __stdcall CDirServ::DeleteDatabase(IDSDatabase *pDatabase)
{
    char			szBuf[128] 	= {0};
    char        szFoo[128]  = {0};
    DWORD			dwCount		= m_pProtocolList->Count();
    LPPENT		pEnt			= NULL;
    CDSDatabase	*pCDatabase	= (CDSDatabase *) pDatabase;
    DWORD	i;
	
    /* Find out which protocol this belongs to											*/
    pCDatabase->GetProtocolName(szBuf,sizeof(szBuf));
    pEnt = (LPPENT) m_pProtocolList->Head();
    for(i=0;i<dwCount;i++) {
		pEnt->pDSP->GetProtocolName(szFoo,sizeof(szFoo));
		if(!strcmp(szBuf,szFoo)) {
			return pEnt->pDSP->DeleteObject(pCDatabase->GetWrappedInterface());
		}
		pEnt = (LPPENT) ILIST_ITEMNEXT(pEnt);
    }
    return E_FAIL;
}

HRESULT __stdcall CDirServ::CreateObject(REFIID riid,void **ppv)
{
    if(IID_IDSQuery == riid) {
	return CQuery::CreateInstance((IUnknown **)ppv);
    }
    return E_NOINTERFACE;
}

IDSDatabase *__stdcall CDirServ::FindDatabase(LPSTR pszDatabaseID)
{
    IDSDatabase 		*pDatabase 	= NULL;
    IDSEnumDatabases	*pEnum		= NULL;
    ULONG					nFetched		= 0;
    LPSTR					pszID			= NULL;
	
    if(SUCCEEDED(EnumDatabases(NULL,&pEnum))) {
	while(S_OK == pEnum->Next(1,&pDatabase,&nFetched)) {
	    if(pszID = pDatabase->GetDatabaseID()) {
		if(!lstrcmp(pszID,pszDatabaseID)) {
		    break;
		}
	    }
	    pDatabase->Release();
	    pDatabase = NULL;
	}
	pEnum->Release();
    }
    return pDatabase;
}


HRESULT __stdcall MapFieldName(HINSTANCE hInst,DS_FIELD nField,LPSTR pszBuffer,int nLen)
{
    switch(nField) {
    case DS_NAME:        ::LoadString(hInst,IDS_DS_NAME,pszBuffer,nLen);       break;
    case DS_COMPANY:     ::LoadString(hInst,IDS_DS_COMPANY,pszBuffer,nLen);    break;
    case DS_DIVISION:    ::LoadString(hInst,IDS_DS_DIVISION,pszBuffer,nLen);   break;
    case DS_POSITION:    ::LoadString(hInst,IDS_DS_POSITION,pszBuffer,nLen);   break;
    case DS_LOCATION:    ::LoadString(hInst,IDS_DS_LOCATION,pszBuffer,nLen); 	break;
    case DS_ADDRESS:     ::LoadString(hInst,IDS_DS_ADDRESS,pszBuffer,nLen);  	break;
    case DS_CITY:        ::LoadString(hInst,IDS_DS_CITY,pszBuffer,nLen);     	break;
    case DS_STATE:       ::LoadString(hInst,IDS_DS_STATE,pszBuffer,nLen);    	break;
    case DS_ZIPCODE:     ::LoadString(hInst,IDS_DS_ZIPCODE,pszBuffer,nLen);  	break;
    case DS_PHONE:       ::LoadString(hInst,IDS_DS_PHONE,pszBuffer,nLen);    	break;
    case DS_CELL:        ::LoadString(hInst,IDS_DS_CELL,pszBuffer,nLen);     	break;
    case DS_PAGER:       ::LoadString(hInst,IDS_DS_PAGER,pszBuffer,nLen);    	break;
    case DS_PEMAIL:      ::LoadString(hInst,IDS_DS_PEMAIL,pszBuffer,nLen);   	break;
    case DS_FAX:         ::LoadString(hInst,IDS_DS_FAX,pszBuffer,nLen);      	break;
    case DS_EMAIL:       ::LoadString(hInst,IDS_DS_EMAIL,pszBuffer,nLen);    	break;
    case DS_PGPKEY:      ::LoadString(hInst,IDS_DS_PGPKEY,pszBuffer,nLen);   	break;
    case DS_PAGERTYPE:   ::LoadString(hInst,IDS_DS_PAGERTYPE,pszBuffer,nLen);	break;
    case DS_PAGERPIN:    ::LoadString(hInst,IDS_DS_PAGERPIN,pszBuffer,nLen); 	break;
    case DS_HADDRESS:    ::LoadString(hInst,IDS_DS_HADDRESS,pszBuffer,nLen); 	break;
    case DS_HCITY:       ::LoadString(hInst,IDS_DS_HCITY,pszBuffer,nLen);    	break;
    case DS_HSTATE:      ::LoadString(hInst,IDS_DS_HSTATE,pszBuffer,nLen);   	break;
    case DS_HZIPCODE:    ::LoadString(hInst,IDS_DS_HZIPCODE,pszBuffer,nLen); 	break;
    case DS_HPHONE:      ::LoadString(hInst,IDS_DS_HPHONE,pszBuffer,nLen);   	break;
    case DS_HCELL:       ::LoadString(hInst,IDS_DS_HCELL,pszBuffer,nLen);    	break;
    case DS_HPAGER:      ::LoadString(hInst,IDS_DS_HPAGER,pszBuffer,nLen);   	break;
    case DS_HPEMAIL:     ::LoadString(hInst,IDS_DS_HPEMAIL,pszBuffer,nLen);  	break;
    case DS_HEMAIL:      ::LoadString(hInst,IDS_DS_HEMAIL,pszBuffer,nLen);   	break;
    case DS_HFAX:        ::LoadString(hInst,IDS_DS_HFAX,pszBuffer,nLen);     	break;
    case DS_HPGPKEY:     ::LoadString(hInst,IDS_DS_HPGPKEY,pszBuffer,nLen);  	break;
    case DS_HPAGERTYPE:  ::LoadString(hInst,IDS_DS_HPAGERTYPE,pszBuffer,nLen); break;
    case DS_HPAGERPIN:   ::LoadString(hInst,IDS_DS_HPAGERPIN,pszBuffer,nLen);  break;
    case DS_USERDEFINED: ::LoadString(hInst,IDS_DS_USERDEFINED,pszBuffer,nLen);break;
    case DS_UNKNOWN:     ::LoadString(hInst,IDS_DS_UNKNOWN,pszBuffer,nLen);    break;
    case DS_COMMENTS:    ::LoadString(hInst,IDS_DS_COMMENTS,pszBuffer,nLen);   break;
    case DS_EMAILALIAS:  ::LoadString(hInst,IDS_DS_EMAILALIAS,pszBuffer,nLen); break;
    case DS_NULL:        ::LoadString(hInst,IDS_DS_NULL, pszBuffer, nLen);     break;
    default: {
	return E_FAIL;
    }
    }
    return S_OK;
}

char	*FieldData[] = {
    "Name",
    "Company",
    "Division",
    "Position",
    "Location",
    "Address",
    "City",
    "State",
    "Zip",
    "Phone",
    "Cell",
    "Pager",
    "PEmail",
    "Fax",
    "Email",
    "PGPKey",
    "PagerType",
    "PagerPIN",
    "HomeAddress",
    "HomeCity",
    "HomeState",
    "HomeZipCode",
    "HomePhone",
    "HomeCell",
    "HomePager",
    "HomePagerEmail",
    "HomeFax",
    "HomeEmail",
    "HomePGPKey",
    "HomePagerType",
    "HomePagerPIN",
    "UserDefined",
    "Unknown",
    "Comments",
    "Alias"
} ;


HRESULT __stdcall FindFieldKey(DS_FIELD nField,LPSTR pszBuffer,int nLen)
{
    if(nField > DS_MAXFIELD)
	return E_FAIL;
	
    if(nLen < lstrlen(FieldData[nField]))
	return E_FAIL;
	
    lstrcpy(pszBuffer,FieldData[nField]);
    return S_OK;
}

HRESULT __stdcall LookupFieldKey(DS_FIELD *pnField,LPSTR pszKey)
{
    int i;
	
    for(i=0;i<DS_MAXFIELD;i++) {
	if(!strcmpi(FieldData[i],pszKey)) {
	    *pnField = (DS_FIELD) i;
	    return S_OK;
	}
    }
    return E_FAIL;
}
