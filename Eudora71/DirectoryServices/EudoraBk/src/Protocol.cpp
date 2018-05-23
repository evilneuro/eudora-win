/******************************************************************************/
/*																										*/
/*	Name		:	PROTOCOL.CPP  																   */
/* Date     :  10/9/1997                                                      */
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

/*	Desc.		:	IDSPProtocol Implementation  												*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"
#include "protocol.h"
#include "EnumDB.H"
#include "database.h"
#include "resource.h"
#include "QCUtils.h"


extern long g_cComponents;

/* Static */
IUnknown * CProtocol::CreateInstance()
{
   CProtocol *pProto = DEBUG_NEW_NOTHROW CProtocol;

   return(pProto);
}

/* Static */
void CProtocol::DBENTFreeCB(LPITEM pItem,LPVOID /* pUser */)
{
   LPDBENT  pEnt = (LPDBENT) pItem;

   if(pEnt && pEnt->pUnkDatabase) pEnt->pUnkDatabase->Release();
   pEnt->pUnkDatabase = NULL;

   delete pEnt;
}

HRESULT	__stdcall CProtocol::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IDSPProtocol)
		*ppv = static_cast<IDSPProtocol*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CProtocol::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CProtocol::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

CProtocol::CProtocol() : m_cRef(1)
{
   char  szTmp[256] = {0};

   m_hInst        = CFactory::s_hModule;
   LoadString(m_hInst,IDS_PROTOCOL_NAME,szTmp,sizeof(szTmp));

   m_pszName      = SafeStrdupMT(szTmp);

   m_hbitSmall    = LoadBitmap(m_hInst,MAKEINTRESOURCE(IDB_ABOOK));
   m_pIDirConfig  = NULL;
   InterlockedIncrement(&g_cComponents);
}

CProtocol::~CProtocol()
{
   if(m_pDatabaseList)  m_pDatabaseList->Release();
   if(m_hbitSmall)      ::DeleteObject(m_hbitSmall);

   delete [] m_pszName;

   if(m_pIDirConfig)    m_pIDirConfig->Release();

   m_pIDirConfig        = NULL;
   m_pszName            = NULL;
   m_hbitSmall          = NULL;
   m_pDatabaseList      = NULL;
   InterlockedDecrement(&g_cComponents);
}

HRESULT __stdcall CProtocol::CreateDatabase(LPSTR pszPath,LPDBENT *ppEnt)
{
   HRESULT     hErr        = E_OUTOFMEMORY;
   CDatabase   *pDatabase  = static_cast<CDatabase *>(CDatabase::CreateInstance());
   LPDBENT     pEnt        = DEBUG_NEW_NOTHROW DBENT;

   /* If we created everything                                                */
   if(pDatabase && pEnt) {
      if(SUCCEEDED(hErr = pDatabase->Initialize(this,pszPath))) {
         pEnt->pUnkDatabase = static_cast<IUnknown *>(pDatabase);
         *ppEnt = pEnt;
         return S_OK;
      }
      else goto CPCD_CLEANUP;
   }
   /* Couldn't alloc something, cleanup                                       */
   else {
CPCD_CLEANUP:
      if(pDatabase)  pDatabase->Release();

	  delete pEnt;
   }
   return hErr;
}


HRESULT __stdcall CProtocol::LoadAddressBooks()
{
   HRESULT           hErr = E_FAIL;
	HKEY					hk;
   OFSTRUCT				ofs;
	
   /* Try to find the currently installed Eudora                              */
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Qualcomm\\Eudora\\CommandLine", 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS){
	   char     szPath[1024]   = {0};
	   DWORD		dwLen          = sizeof(szPath);
      DWORD    dwType         = 0;

		if(ERROR_SUCCESS == RegQueryValueEx(hk, "Current", 0, &dwType, (LPBYTE)szPath, &dwLen)) {
         if(*szPath){
            LPSTR	   lpszPath = NULL;
            LPSTR    lpsz     = NULL;
            LPSTR    lpszFile = NULL;

            if((lpszPath = strchr(szPath,' '))) {
               *(lpszPath++) = 0;
               if((lpszPath = strchr(lpszPath,' '))) {
                  *(lpszPath++) = 0;
               }
            }

            /* Isolate the filename portion of the path                       */
            if((lpszFile = strrchr(lpszPath,'\\'))) {
               WIN32_FIND_DATA	fd;
	            HANDLE				hf;

               /* Default server is NNDBASE.TXT...                            */
               lstrcpy(lpszFile,"\\nndbase.txt");
               if(HFILE_ERROR != OpenFile(lpszPath,&ofs,OF_EXIST)) {
                  LPDBENT  pEnt = NULL;

                  if(SUCCEEDED(hErr = CreateDatabase(lpszPath,&pEnt))) {
                     m_pDatabaseList->Attach(pEnt);
                  }
               }

               /* Look for any other address books...                         */
               lstrcpy(lpszFile,"\\Nickname\\*.*");
               lpszFile = strchr(lpszFile,'*');
               if((hf = FindFirstFile(lpszPath,&fd)) != INVALID_HANDLE_VALUE) {
                  do {
                     if((lpsz = strchr(fd.cFileName,'.')) && !lstrcmpi(lpsz,".txt")) {
                        LPDBENT  pEnt = NULL;

                        lstrcpy(lpszFile,fd.cFileName);
                        if(SUCCEEDED(hErr = CreateDatabase(lpszPath,&pEnt))) {
                           m_pDatabaseList->Attach(pEnt);
                        }
                     }
                  }
                  while(FALSE != FindNextFile(hf,&fd));
                  FindClose(hf);
               }
            }
         }
		   RegCloseKey(hk);
      }
	}
   return hErr;
}

HRESULT __stdcall CProtocol::Initialize(IUnknown *pUnkCaller)
{
   HRESULT  hErr = E_FAIL;


   if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pDatabaseList))) {
      m_pDatabaseList->Initialize(CProtocol::DBENTFreeCB,NULL);
      pUnkCaller->QueryInterface(IID_IDirConfig,(void **) &m_pIDirConfig);
      hErr = LoadAddressBooks();
	  if(FAILED(hErr))
		m_pIDirConfig->Release();

   }
   return hErr;
}

HRESULT __stdcall CProtocol::GetDirConfig(IDirConfig **ppIDirConfig)
{
   if(m_pIDirConfig) {
      m_pIDirConfig->AddRef();
      *ppIDirConfig = m_pIDirConfig;
      return S_OK;
   }
   return E_FAIL;
}

HRESULT __stdcall CProtocol::CreateObject(REFIID /* riid */, LPVOID * /* ppv */)
{
   return E_FAIL;
}

HRESULT __stdcall CProtocol::DeleteObject(IUnknown * /* punkObject */)
{
   return E_FAIL;
}

HRESULT __stdcall CProtocol::EnumDatabases(IDSPEnumConfig **ppenum)
{
	/* CDSEnumProtocols's ref count is initialized to 1, no AddRef() needed		*/
	*ppenum = static_cast<IDSPEnumConfig*>(DEBUG_NEW_NOTHROW CEnumDatabase(this,m_pDatabaseList,FALSE));
	
	if(*ppenum) {
		return(S_OK);
        }
	
	return(E_OUTOFMEMORY);
}

HRESULT __stdcall CProtocol::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
   if(!lpszBuffer)
      return E_FAIL;

   strncpy(lpszBuffer,m_pszName,nBufferSize);
   return S_OK;
}

HBITMAP __stdcall CProtocol::GetProtocolImage(int iImage)
{
   if(IMG_SMALL == iImage)
      return m_hbitSmall;
   else
      return NULL;
}

DWORD __stdcall CProtocol::GetProtocolFlags()
{
   return PROTO_LOCAL | PROTO_WRITEABLE | PROTO_INCREMENTAL;
}
