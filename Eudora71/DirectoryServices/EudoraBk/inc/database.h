/******************************************************************************/
/*																										*/
/*	Name		:	DATABASE.H	  																   */
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

/*	Desc.		:	IDSPDatabase Definitions   												*/
/*																										*/
/******************************************************************************/
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <dspapi.h>
#include "protocol.h"

typedef struct tagLTBL
{
   ITEM        item;
   LPSTR       pszLine;
} LTBL, *LPLTBL;

typedef struct tagRECENT
{
   ITEM           item;
   IDSPRecord     *pRecord;
} RECENT, *LPRECENT;

class CDatabase : public IDSPConfig
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();

	/* IDSPConfig  																				*/
	virtual HRESULT		__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP		__stdcall GetImage(int iImage);
	virtual LPSTR			__stdcall GetID();
	virtual int				__stdcall GetPropPageCount();
	virtual HRESULT		__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages);
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage);
	virtual DWORD			__stdcall GetProtocolFlags();
	virtual HRESULT 		__stdcall CreateObject(REFIID riid,LPVOID *ppv);

   /* CDatabase																				   */
   CDatabase();
   ~CDatabase();

   static IUnknown *CreateInstance();
   static void LTblFreeCB(LPITEM pItem,LPVOID pUser);
   static void RecentFreeCB(LPITEM pItem,LPVOID pUser);
   static BOOL CALLBACK PageProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

   virtual HRESULT      __stdcall Initialize(CProtocol *pProtocol,LPSTR pszPathname);
   virtual LPSTR        __stdcall GetPath();
   virtual HRESULT      __stdcall GetRecordList(IListMan **ppRecordList);
   virtual HRESULT      __stdcall GetFieldName(DS_FIELD nField,LPSTR pszBuffer,int nLen);
   virtual BOOL         __stdcall ExtractField(DS_FIELD nName,IDSPRecord *pRec,LPSTR pszBuf,int nLen);

   /* Methods for managing the line table                                     */
   virtual LPLTBL       __stdcall Find(IListMan *pLineTable,LPSTR pszKey,LPSTR pszName);
   virtual QUERY_STATUS __stdcall Add(IListMan *pLineTable,IDSPRecord *pRec);
   virtual QUERY_STATUS __stdcall Delete(IListMan *pLineTable,LPSTR pszAlias,LPLTBL pTbl);
   virtual QUERY_STATUS __stdcall Update(IListMan *pLineTable,IDSPRecord *pRec,LPSTR pszAlias,LPLTBL pTbl);

   /* Child CQuery object count methods (Called by CQuery)                    */
   virtual ULONG        __stdcall IncQuery();
   virtual ULONG        __stdcall DecQuery();

protected:
   /* Methods for formatting addressbool lines from IDSPRecords               */
   virtual LPLTBL       __stdcall BuildAlias(IDSPRecord *pRec);
   virtual LPLTBL       __stdcall BuildNote(IDSPRecord *pRec);

   /* Methods for building the line table                                     */
   virtual HRESULT      __stdcall BuildLineTable();
   virtual HRESULT      __stdcall AddLine(LPSTR pszBuf);
   virtual HRESULT      __stdcall FoldInTheAddresBookKludge(LPSTR pszFoldedLine);

   /* Methods for building the list of records                                */
   virtual HRESULT      __stdcall RefreshDatabase();
   virtual BOOL         __stdcall DatabaseChanged();
   virtual HRESULT      __stdcall BuildRecordList();
   virtual BOOL         __stdcall LTBLIsAlias(LPLTBL pTbl);
   virtual HRESULT      __stdcall LTBLExtractAlias(LPLTBL pTbl,LPSTR pszBuffer,int nLen);
   virtual HRESULT      __stdcall LTBLCreateRecord(LPLTBL pTbl,LPLTBL pTbl1,IDSPRecord **pRecord);
   virtual HRESULT      __stdcall LTBLParseNote(LPSTR pLine,IDSPRecord *pRec);
   virtual HRESULT      __stdcall LTBLParseAlias(LPSTR pLine,IDSPRecord *pRec);
   virtual HRESULT      __stdcall AddField(LPSTR pszField,IDSPRecord *pRec);
   virtual LPSTR        __stdcall SpecialNextChar(LPSTR pszCur);
   virtual LPSTR        __stdcall FindEmailEnd(LPSTR pStr);

#ifdef _DEBUG
   virtual void         __stdcall DebugDumpRecordList();
   virtual void         __stdcall DebugDumpRecord(IDSPRecord *pRecord,HANDLE hFile,LPSTR pszBuffer,int nLen);
   virtual void         __stdcall DebugDumpField(DBRECENT *pEnt,HANDLE hFile,LPSTR pszBuffer,int nLen);
#endif

	LONG					m_cRef;				/* Reference count							*/
   CProtocol         *m_pProtocol;     /* Our parent object                   */
   IListMan          *m_pLineTable;    /* List of addressbook lines           */
   IListMan          *m_pRecordList;   /* List of CRecords                    */
   LPSTR             m_pszPath;        /* Path of addressbook file            */
   LPSTR             m_pszName;        /* Display name of address book        */
   LONG              m_cQueries;       /* Count of running CQuery Objects     */
   FILETIME          m_ftLastModified; /* Time database file was last modified*/
} ;

#define ALIAS_STR "alias"
#define NOTE_STR  "note"

#endif




