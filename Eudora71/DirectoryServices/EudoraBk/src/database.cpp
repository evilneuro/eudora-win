/******************************************************************************/
/*																										*/
/*	Name		:	DATABASE.CPP  																   */
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

/*	Desc.		:	IDSPConfig Implementation  												*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include <stdio.h>
#include <prsht.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"
#include "record.h"
#include "query.h"
#include "database.h"
#include "resource.h"
#include "QCUtils.h"


extern long g_cComponents;

IUnknown * CDatabase::CreateInstance()
{
   CDatabase *pDatabase = DEBUG_NEW_NOTHROW CDatabase;

   return pDatabase;
}

HRESULT	__stdcall CDatabase::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IDSPConfig)
		*ppv = static_cast<IDSPConfig*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CDatabase::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CDatabase::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}


CDatabase::CDatabase() : m_cRef(1)
{
   m_pProtocol    = NULL;
   m_pLineTable   = NULL;
   m_pRecordList  = NULL;
   m_pszPath      = NULL;
   m_pszName      = NULL;
   m_cQueries     = 0;
   InterlockedIncrement(&g_cComponents);
}

CDatabase::~CDatabase()
{
   delete m_pszPath;
   delete m_pszName;

   if(m_pRecordList) m_pRecordList->Release();
   if(m_pLineTable)  m_pLineTable->Release();

   m_pLineTable   = NULL;
   m_pRecordList  = NULL;
   m_pszName      = NULL;
   m_pszPath      = NULL;
   m_pProtocol    = NULL;
   InterlockedDecrement(&g_cComponents);
}

HRESULT __stdcall CDatabase::Initialize(CProtocol *pProtocol,LPSTR pszPathname)
{
   HRESULT  hErr;

   if(!pszPathname || !(*pszPathname) || !pProtocol)
      return E_FAIL;

   m_pProtocol = pProtocol;
   if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pLineTable))) {
      if(SUCCEEDED(hErr = CoCreateInstance(CLSID_IListMan,NULL,CLSCTX_INPROC_SERVER,IID_IListMan,(void **) &m_pRecordList))) {

         m_pLineTable->Initialize(CDatabase::LTblFreeCB,NULL);
         m_pRecordList->Initialize(CDatabase::RecentFreeCB,NULL);

		 if(NULL != (m_pszPath = SafeStrdupMT(pszPathname))) {

            LPSTR lpsz        = NULL;
            LPSTR lpszDot     = NULL;

            /* If this isn't the default addressbook                                */
            if((lpsz = strrchr(m_pszPath,'\\')) && lstrcmpi(lpsz,"\\nndbase.txt") && (lpszDot = strchr(lpsz,'.'))) {
               *lpszDot = 0;
			   m_pszName = SafeStrdupMT((lpsz+1));

			   *lpszDot = '.';
            }
            /* If it is the default address book, use a "nicer" display name        */
            else {
               char szTmp[512] = {0};

               LoadString(CFactory::s_hModule, IDS_DEFAULT_BOOK,szTmp,sizeof(szTmp));
			   m_pszName = SafeStrdupMT(szTmp);
            }

            /* Did we get a name???                                                 */
            if(!m_pszName)
               return E_OUTOFMEMORY;

            hErr = RefreshDatabase();
         }
         else return E_OUTOFMEMORY;
      }
   }
   return hErr;
}

ULONG __stdcall CDatabase::IncQuery()
{
   return InterlockedIncrement(&m_cQueries);
}

ULONG __stdcall CDatabase::DecQuery()
{
   return InterlockedDecrement(&m_cQueries);
}

/* Protected */
BOOL __stdcall CDatabase::DatabaseChanged()
{
   BOOL              bRetval  = FALSE;
   WIN32_FIND_DATA   fd       = {0};
   HANDLE            hFind    = INVALID_HANDLE_VALUE;

   if(INVALID_HANDLE_VALUE != (hFind = FindFirstFile(m_pszPath,&fd))) {
      if(0 != CompareFileTime(&m_ftLastModified,&fd.ftLastWriteTime)) {
         m_ftLastModified.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime;
         m_ftLastModified.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime;
         bRetval = TRUE;
      }
      FindClose(hFind);
   }
   return bRetval;
}

/* Protected */
HRESULT __stdcall CDatabase::RefreshDatabase()
{
   HRESULT  hErr = S_OK;

   if(0 == m_cQueries && TRUE == DatabaseChanged()) {
      m_pLineTable->Erase();
      m_pRecordList->Erase();

      /* Fill Line Table                                                      */
      if(SUCCEEDED(hErr = BuildLineTable())) {
         /* Build CRecord list                                                */
         if(SUCCEEDED(hErr = BuildRecordList())) {
   #ifdef _DEBUG
            DebugDumpRecordList();
   #endif
         }
      }
   }
   return hErr;
}

/* Static */
void CDatabase::RecentFreeCB(LPITEM pItem, LPVOID /* pUser */)
{
   LPRECENT pRE = (LPRECENT) pItem;

   if(pRE->pRecord) pRE->pRecord->Release();

   pRE->pRecord = NULL;

   delete pRE;
}

HRESULT __stdcall CDatabase::GetRecordList(IListMan **ppRecordList)
{
   if(m_pRecordList) {
      m_pRecordList->AddRef();
      *ppRecordList = m_pRecordList;
      return S_OK;
   }
   return E_FAIL;
}

/* Protected */
HRESULT __stdcall CDatabase::BuildRecordList()
{
   HRESULT     hErr        S_OK;
   LPLTBL      pTbl        = NULL;
   LPLTBL      pTbl1       = NULL;
   LPLTBL      pTmp        = NULL;
   char        szBuf[512]  = {0};
   BOOL        bAlias      = FALSE;
   IDSPRecord  *pRecord    = NULL;

   /* Remove a line from the list                                             */
   while(NULL != (pTbl = (LPLTBL) m_pLineTable->Pop())) {

      /* Determine what kind it is (alias or note)                            */
      bAlias = LTBLIsAlias(pTbl);

      /* Extract the alias (both "alias" and "note" have this field)          */
      LTBLExtractAlias(pTbl,szBuf,sizeof(szBuf));

      /* Now find the other matching line                                     */
      if(TRUE == bAlias) {
         pTmp = Find(m_pLineTable,NOTE_STR,szBuf);
      }
      else {
         pTmp = Find(m_pLineTable,ALIAS_STR,szBuf);
      }

      /* Remove it from the list also                                         */
      if(pTmp) pTbl1 = (LPLTBL) m_pLineTable->Unlink(pTmp);

      /* Now create an IDSPRecord using the two lines                         */
      if(SUCCEEDED(hErr = LTBLCreateRecord(pTbl,pTbl1,&pRecord))) {
         LPRECENT pRE = DEBUG_NEW_NOTHROW RECENT;

         /* And Attach it to our record list                                  */
         if(pRE) {
            pRE->pRecord = pRecord;
            m_pRecordList->Attach(pRE);
         }
         else {
            pRecord->Release();
            return E_OUTOFMEMORY;
         }
      }
      if(pTbl)   LTblFreeCB((LPITEM) pTbl,NULL);
      if(pTbl1)  LTblFreeCB((LPITEM) pTbl1,NULL);
      pTbl = pTbl1 = NULL;
   }
   return hErr;
}

/* Static */
void CDatabase::LTblFreeCB(LPITEM pItem, LPVOID /* pUser */)
{
   LPLTBL   pTbl = (LPLTBL) pItem;

//   if(pTbl->pszLine) free(pTbl->pszLine);
   delete [] pTbl->pszLine;

   delete pTbl;
}

/* Protected */
BOOL __stdcall CDatabase::ExtractField(DS_FIELD nName,IDSPRecord *pRec,LPSTR pszBuf,int nLen)
{
   DBRECENT *pEnt;

   if(NULL != (pEnt = pRec->GetRecordList())) {
      while(pEnt) {
         if(pEnt->nName = nName) {
            switch(pEnt->nType) {
               case DST_ASCII:
               case DST_ASCIIZ: {
                  strncpy(pszBuf,(LPSTR)pEnt->data,min(nLen,(int)pEnt->dwSize));
                  return TRUE;
               }
            }
         }
         pEnt = pEnt->pNext;
      }
   }
   return FALSE;
}

/* Protected */
LPLTBL __stdcall CDatabase::BuildAlias(IDSPRecord *pRec)
{
   char  szName[256]    = {0};
   char  szEmail[512]   = {0};

   if(TRUE == ExtractField(DS_NAME,pRec,szName,sizeof(szName))) {
      if(TRUE == ExtractField(DS_EMAIL,pRec,szEmail,sizeof(szEmail))) {
         char     szBuf[1024] = {0};
         LPLTBL   pTbl        = DEBUG_NEW_NOTHROW LTBL;

         if(pTbl) {
            memset( pTbl, 0, sizeof(LTBL) );

            wsprintf(szBuf,"alias \"%s\" %s\r\n",szName,szEmail);
			if(pTbl->pszLine = SafeStrdupMT(szBuf)) {
				return pTbl;
			}
            else
				free(pTbl);
         }
      }
   }
   return NULL;
}

/* Protected */
LPLTBL __stdcall CDatabase::BuildNote(IDSPRecord *pRec)
{
   char  szName[256]    = {0};
   char  szFax[32]      = {0};
   char  szPhone[32]    = {0};
   char  szAddress[256] = {0};
   char  szNotes[1024]  = {0};

   if(TRUE == ExtractField(DS_NAME,pRec,szName,sizeof(szName))) {
      char     szBuf[1024] = {0};
	  LPLTBL   pTbl        = DEBUG_NEW_NOTHROW LTBL;

      if(pTbl) {
         memset( pTbl, 0, sizeof(LTBL) );

         wsprintf(szBuf,"note \"%s\"",szName);
         ExtractField(DS_FAX,pRec,szFax,sizeof(szFax));
         ExtractField(DS_PHONE,pRec,szPhone,sizeof(szPhone));
         ExtractField(DS_ADDRESS,pRec,szAddress,sizeof(szAddress));
         ExtractField(DS_COMMENTS,pRec,szNotes,sizeof(szNotes));

         if(szFax[0])      wsprintf(szBuf+lstrlen(szBuf),"<fax:%s>",szFax);
         if(szPhone[0])    wsprintf(szBuf+lstrlen(szBuf),"<phone:%s>",szPhone);
         if(szAddress[0])  wsprintf(szBuf+lstrlen(szBuf),"<address:%s>",szAddress);
         if(szNotes[0])    wsprintf(szBuf+lstrlen(szBuf),"%s",szNotes);

		 if(pTbl->pszLine = SafeStrdupMT(szBuf)) {
			 return pTbl;
		 }
		 else
			 free(pTbl);
      }
   }
   return NULL;
}

typedef struct tagQCHARS {
   char  cStart;
   char  cEnd;
} QCHARS, *LPQCHARS;

QCHARS   gQuotes[] = {{'\"','\"'},
                      {'(',')'},
                      {'<','>'},
                     };
#define QUOTESZ   (sizeof(gQuotes) / sizeof(QCHARS))

LPSTR __stdcall CDatabase::SpecialNextChar(LPSTR pszCur)
{
   LPSTR          pszNextChar = NULL;
   register int   i;

   for(i=0;i<QUOTESZ;i++) {
      if(gQuotes[i].cStart == *pszCur) {
         if(pszNextChar = strchr(pszCur,gQuotes[i].cEnd))
            pszNextChar++;
         break;
      }
   }

   if(!pszNextChar)
      pszNextChar = (pszCur + 1);

   return pszNextChar;
}

LPSTR __stdcall CDatabase::FindEmailEnd(LPSTR pStr)
{
   LPSTR pEnd     = pStr;

   while(*pEnd) {
      pEnd = SpecialNextChar(pEnd);

      if(',' == *pEnd)
         break;
   }
   return pEnd;
}

HRESULT __stdcall CDatabase::LTBLParseAlias(LPSTR pLine,IDSPRecord *pRec)
{
   HRESULT  hErr;
   LPSTR    pStart   = pLine;
   LPSTR    pEnd     = NULL;
   char     cSave    = 0;
   CRecord  *pcRec   = (CRecord *) pRec;

   /* Move past the "alias "                                                  */
   pStart += lstrlen(ALIAS_STR);

   /* If the string is Quoted                                                 */
   if('\"' ==  *(pStart + 1)) {
      pStart += 2;                  /* Move past Space and quote              */
      /* Find the ending quote                                                */
      if(NULL != (pEnd = strchr(pStart,'\"'))) {
         cSave = *pEnd;
         *pEnd = 0;
      }
   }
   else {
      pStart++;                     /* Move past the space                    */
      if(NULL != (pEnd = strchr(pStart,' '))) {
         cSave = *pEnd;
         *pEnd = 0;
      }
   }

   /* If we didn't get anything, bail...                                      */
   if(!pStart)
      return E_FAIL;

   /* Add the alias to the record                                             */
   if(FAILED(hErr = pcRec->Add(DS_EMAILALIAS,pStart)))
      return hErr;

   /* Is this an alias with no data ?                                         */
   if(!pEnd)
      return S_OK;

   /* Now add the email addresses                                             */
   pStart = pEnd + 1;

   /* Skip whitespace                                                         */
   while(' ' == *pStart)
      pStart++;

   /* No email Addresses ???                                                  */
   if(!(*pStart))
      return S_OK;

   while(*(pEnd = FindEmailEnd(pStart))) {
      *pEnd = 0;
      pcRec->Add(DS_EMAIL,pStart);
      pStart = pEnd+1;
   }
   pcRec->Add(DS_EMAIL,pStart);
   return S_OK;
}

typedef struct tagFIELDS {
   DS_FIELD    nField;
   LPSTR       pszName;
} FIELDS, *LPFIELD;

FIELDS  gFields[4] = {{DS_NAME,   "name:"},
                      {DS_PHONE,  "phone:"},
                      {DS_ADDRESS,"address:"},
                      {DS_FAX,    "fax:"}
                     };
#define FIELDSZ (sizeof(gFields) / sizeof(FIELDS))

HRESULT __stdcall CDatabase::AddField(LPSTR pszField,IDSPRecord *pRec)
{
   HRESULT  hErr  = S_OK;
   CRecord  *pcRec= (CRecord *) pRec;
   int      i;

   for(i=0;i<FIELDSZ;i++) {
      if(!_strnicmp(pszField,gFields[i].pszName,lstrlen(gFields[i].pszName))) {
         hErr = pcRec->Add(gFields[i].nField,pszField+lstrlen(gFields[i].pszName));
         return hErr;
      }
   }

   hErr = pcRec->Add(DS_UNKNOWN,pszField);

   return hErr;
}

HRESULT __stdcall CDatabase::LTBLParseNote(LPSTR pLine,IDSPRecord *pRec)
{
   HRESULT  hErr     = S_OK;
   LPSTR    pTmp     = NULL;
   LPSTR    pStart   = pLine;
   LPSTR    pEnd     = NULL;
   char     cSave    = 0;
   CRecord  *pcRec   = (CRecord *) pRec;

   /* Move past the "note " and alias data                                    */
   pStart += lstrlen(NOTE_STR);

   /* If the aliad adata is Quoted                                            */
   if('\"' ==  *(pStart + 1)) {
      pStart += 2;                  /* Move past Space and quote              */
      /* Find the ending quote                                                */
      if(NULL != (pEnd = strchr(pStart,'\"'))) {
         cSave = *pEnd;
         *pEnd = 0;
      }
   }
   else {
      pStart++;                     /* Move past the space                    */
      if(NULL != (pEnd = strchr(pStart,' '))) {
         cSave = *pEnd;
         *pEnd = NULL;
      }
   }

   /* pEnd can be null because we didn't match a quote, or because there is   */
   /* no data following the alias poertion of the line in which case we have  */
   /* nothing to do.  If pStart == NULL "it's a bad thing", so bail...        */
   if(!pStart || !pEnd)
      return E_FAIL;

   /* Now add the email addresses                                             */
   pStart = pEnd + 1;

   /* Skip whitespace                                                         */
   while(' ' == *pStart)
      pStart++;

   /* No email Data ???                                                       */
   if(!(*pStart))
      return S_OK;

   while(NULL != (pStart = strchr(pStart,'<'))) {
      pStart++;

      if(NULL != (pEnd = strchr(pStart,'>'))) {
         *pEnd = 0;
         hErr = AddField(pStart,pcRec);
         pStart   = pEnd + 1;
         pTmp     = pStart;
      }
   }

   if(!pStart && pTmp && isalpha(*pTmp))
      hErr = pcRec->Add(DS_COMMENTS,pTmp);

   return hErr;
}


HRESULT __stdcall CDatabase::LTBLCreateRecord(LPLTBL pTbl,LPLTBL pTbl1,IDSPRecord **pRecord)
{
   CRecord  *pcRec      = DEBUG_NEW_NOTHROW CRecord(this);

   if(pcRec) {
      if(pTbl) {
         if(TRUE == LTBLIsAlias(pTbl))
            LTBLParseAlias(pTbl->pszLine,pcRec);
         else
            LTBLParseNote(pTbl->pszLine,pcRec);
      }

      if(pTbl1) {
         if(TRUE == LTBLIsAlias(pTbl1))
            LTBLParseAlias(pTbl1->pszLine,pcRec);
         else
            LTBLParseNote(pTbl1->pszLine,pcRec);
      }
      *pRecord = pcRec;
      return S_OK;
   }
   return E_OUTOFMEMORY;
}

BOOL __stdcall CDatabase::LTBLIsAlias(LPLTBL pTbl)
{
   if(!_strnicmp(pTbl->pszLine,ALIAS_STR,lstrlen(ALIAS_STR)))
      return TRUE;
   return FALSE;
}
HRESULT __stdcall CDatabase::LTBLExtractAlias(LPLTBL pTbl,LPSTR pszBuffer,int nLen)
{
   LPSTR pStart= NULL;
   LPSTR pEnd  = NULL;
   char  cSave = 0;

   /* Extract the Alias portion of the line                             */
   /* Find the first space (follows "alias" or "note")                  */
   if(NULL != (pStart = strchr(pTbl->pszLine,' '))) {
      /* If the string is Quoted                                        */
      if('\"' ==  *(pStart + 1)) {
         pStart += 2;                  /* Move past Space and quote     */
         /* Find the ending quote                                       */
         if(NULL != (pEnd = strchr(pStart,'\"'))) {
            cSave = *pEnd;
            *pEnd = 0;
         }
      }
      else {
         pStart++;                     /* Move past the space           */
         if(NULL != (pEnd = strchr(pStart,' '))) {
            cSave = *pEnd;
            *pEnd = NULL;
         }
      }

      if(pStart) {
         strncpy(pszBuffer,pStart,nLen);
         if(pEnd) *pEnd = cSave;
         return S_OK;
      }
   }
   if(cSave && pEnd)
      *pEnd = cSave;
   return E_FAIL;
}

LPLTBL __stdcall CDatabase::Find(IListMan *pLineTable,LPSTR pszKey,LPSTR pszName)
{
   DWORD    i;
   DWORD    dwCount  = pLineTable->Count();
   LPLTBL   pTbl     = (LPLTBL) pLineTable->Head();
   int      nKey     = lstrlen(pszKey);

   for(i=0;i<dwCount;i++) {
      /* If this is a "note" line                                             */
      if(!_strnicmp(pTbl->pszLine,pszKey,nKey)) {
         char  szAlias[256] = {0};

         LTBLExtractAlias(pTbl,szAlias,sizeof(szAlias));
         if(*szAlias && !lstrcmpi(szAlias,pszName)) {
            return pTbl;
         }
      }
      pTbl = (LPLTBL) ILIST_ITEMNEXT(pTbl);
   }
   return NULL;
}

QUERY_STATUS __stdcall CDatabase::Add(IListMan *pLineTable,IDSPRecord *pRec)
{
   LPLTBL   pTbl;

   if(NULL != (pTbl = BuildAlias(pRec))) {
      pLineTable->Attach(pTbl);

      if(NULL != (pTbl = BuildNote(pRec)))
         pLineTable->Attach(pTbl);

      return QUERY_OK;
   }
   return QUERY_FAILED;
}

QUERY_STATUS __stdcall CDatabase::Delete(IListMan *pLineTable,LPSTR pszAlias,LPLTBL pTbl)
{
   if(pTbl) {
      LPLTBL   pTmp;

      pLineTable->DeleteItem(pTbl);
      if(NULL != (pTmp = Find(pLineTable,NOTE_STR,pszAlias)))
         pLineTable->DeleteItem(pTmp);

      return QUERY_OK;
   }
   return QUERY_FAILED;
}

QUERY_STATUS __stdcall CDatabase::Update(IListMan *pLineTable,IDSPRecord *pRec,LPSTR pszAlias,LPLTBL pTbl)
{
   Delete(pLineTable,pszAlias,pTbl);
   return Add(pLineTable,pRec);
}

HRESULT __stdcall CDatabase::GetName(LPSTR lpszBuffer, UINT nBufferSize)
{
   strncpy(lpszBuffer,m_pszName,nBufferSize);
   return S_OK;
}

HBITMAP __stdcall CDatabase::GetImage(int /* iImage */)
{
   return NULL;
}

LPSTR __stdcall CDatabase::GetID()
{
   return m_pszPath;
}

int __stdcall CDatabase::GetPropPageCount()
{
	return(1);
}

BOOL CALLBACK CDatabase::PageProc1(HWND hwndDlg, UINT uMsg, WPARAM /* wParam */, LPARAM lParam)
{
	switch (uMsg) {
		
		case WM_INITDIALOG: {
         CDatabase *p = (CDatabase *) ((PROPSHEETPAGE *)lParam)->lParam;
			SetDlgItemText(hwndDlg, IDC_BOOKPATH, p->m_pszPath);

			//SetDlgItemText(hwndDlg, IDC_HOSTNAME, m_pszHostName);
			//SetDlgItemInt(hwndDlg, IDC_PORT, m_nPort, 0);
			return TRUE;
		}

		case WM_NOTIFY:
			//switch (((NMHDR*)lParam)->code) {
			//	
			//	case PSN_KILLACTIVE: {
         // BOOL bErrorOnPage = CheckEditFieldEmpty(hwndDlg, IDC_HOSTNAME, IDS_ERRHOSTNAME);
			//		if (!bErrorOnPage)
			//			bErrorOnPage = CheckEditFieldInt(hwndDlg, IDC_PORT, 0, MAXPORTNUM);
			//		SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)bErrorOnPage);
			//		return TRUE;
			//	}
         //
			//	case PSN_APPLY: {
			//		CUString str(hwndDlg, IDC_NAME);
			//		FreeDup(m_pszName, str);
         //
			//		str.Set(hwndDlg, IDC_HOSTNAME);
			//		FreeDup(m_pszHostName, str);
         //
			//		BOOL bTrans;
			//		m_nPort = (int)GetDlgItemInt(hwndDlg, IDC_PORT, &bTrans, FALSE);
			//		m_bDirtyData = 1;
			//		Write();
			//		return TRUE;
			//	}
			//} // switch
			break;

	}	// switch(uMsg)

	return FALSE;
}

//*****************************************************************************
//
//
//
//*****************************************************************************
HRESULT __stdcall CDatabase::GetPropPages(HPROPSHEETPAGE *paHPropPages)
{
	PROPSHEETPAGE psp;

	psp.dwSize 			= sizeof(psp);
	psp.dwFlags 		= PSP_DEFAULT;
	psp.hInstance 	   = CFactory::s_hModule;
	psp.pszTemplate   = MAKEINTRESOURCE(IDD_PROPPAGE_GENERAL);
	psp.pfnDlgProc 	= (DLGPROC) PageProc1;
	psp.lParam 			= (LONG)this;

	*paHPropPages = CreatePropertySheetPage(&psp);
	if (!*paHPropPages)
		return E_FAIL;

   return S_OK;
}

HRESULT __stdcall CDatabase::GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize)
{
   return m_pProtocol->GetProtocolName(lpszBuffer,nBufferSize);
}

HBITMAP __stdcall CDatabase::GetProtocolImage(int iImage)
{
   return m_pProtocol->GetProtocolImage(iImage);
}

DWORD __stdcall CDatabase::GetProtocolFlags()
{
   return m_pProtocol->GetProtocolFlags();
}

HRESULT __stdcall CDatabase::CreateObject(REFIID riid,LPVOID *ppv)
{
   HRESULT  hErr;
   IUnknown *pUnk = NULL;

   if(riid == IID_IDSPQuery) {
      RefreshDatabase();
      hErr = CQuery::CreateInstance(&pUnk,this);
      *ppv = pUnk;
   }
   else
      hErr = E_NOINTERFACE;

   return hErr;
}

LPSTR __stdcall CDatabase::GetPath()
{
   return m_pszPath;
}

HRESULT __stdcall CDatabase::GetFieldName(DS_FIELD nField,LPSTR pszBuffer,int nLen)
{
   HRESULT     hErr = E_FAIL;
   if(m_pProtocol) {
      IDirConfig  *pConfig = NULL;

      if(SUCCEEDED(hErr = m_pProtocol->GetDirConfig(&pConfig))) {
         hErr = pConfig->GetFieldName(nField,pszBuffer,nLen);
         pConfig->Release();
      }
   }
   return hErr;
}

#ifdef _DEBUG
void __stdcall CDatabase::DebugDumpField(DBRECENT *pEnt,HANDLE hFile,LPSTR pszBuffer,int nLen)
{
   DWORD    dwWritten;

   memset(pszBuffer,0,nLen);
   GetFieldName(pEnt->nName,pszBuffer,nLen);
   lstrcat(pszBuffer," (");
   switch(pEnt->nType) {
      case DST_ASCII: {
         strncpy(pszBuffer + lstrlen(pszBuffer),(LPSTR)pEnt->data,min(nLen,(int)pEnt->dwSize));
      }
      case DST_URL:
      case DST_ASCIIZ: {
         strncpy(pszBuffer + lstrlen(pszBuffer),(LPSTR)pEnt->data,min(nLen,(int)pEnt->dwSize));
      }
   }
   lstrcat(pszBuffer,")\r\n");
   WriteFile(hFile,pszBuffer,lstrlen(pszBuffer),&dwWritten,NULL);
}

void __stdcall CDatabase::DebugDumpRecord(IDSPRecord *pRecord,HANDLE hFile,LPSTR pszBuffer,int nLen)
{
   if(pRecord) {
      DBRECENT *pEnt = pRecord->GetRecordList();

      while(pEnt) {
         DebugDumpField(pEnt,hFile,pszBuffer,nLen);
         pEnt = pEnt->pNext;
      }
   }
}

#define BEGIN_STR "---- Begin Record ----\r\n"
#define END_STR   "---- End Record ----\r\n\r\n"
#define NOMEM_STR "Error: Out of memory allocating buffer."

void __stdcall CDatabase::DebugDumpRecordList()
{
   if(m_pRecordList) {
      HANDLE   hFile          = INVALID_HANDLE_VALUE;
      char     szFilename[512]= {0};

	  LPSTR    pszBuffer = DEBUG_NEW_NOTHROW char[0x10000]; /* 64k */

      lstrcpy(szFilename,m_pszName);
      lstrcat(szFilename,".dmp");
      if(INVALID_HANDLE_VALUE != (hFile = CreateFile(szFilename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL))) {
         DWORD    dwWritten;

         if(pszBuffer) {
            LPRECENT pRE      = (LPRECENT) m_pRecordList->Head();
            DWORD    dwCount  = m_pRecordList->Count();
            DWORD    i;

            for(i=0;i<dwCount;i++) {
               WriteFile(hFile,BEGIN_STR,lstrlen(BEGIN_STR),&dwWritten,NULL);
               DebugDumpRecord(static_cast<IDSPRecord *>(pRE->pRecord),hFile,pszBuffer,0x10000);
               WriteFile(hFile,END_STR,lstrlen(END_STR),&dwWritten,NULL);
               pRE = (LPRECENT) ILIST_ITEMNEXT(pRE);
            }

			delete [] pszBuffer;
         }
         else WriteFile(hFile,NOMEM_STR,lstrlen(NOMEM_STR),&dwWritten,NULL);
         CloseHandle(hFile);
      }
   }
}

#endif

