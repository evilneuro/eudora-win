/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					record.cpp
Description:		Implementation of CPHRecord class
Date:						8/21/97
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
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <objbase.h>
#include <assert.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "trace.h"
#include "record.h"
#include "QCUtils.h"


extern long g_cComponents;				// Count of active components


///////////////////////////////////////////////////////////////////////////////
// CPHRecord class

CPHRecord::CPHRecord() : m_cRef(1)
{
	m_pRecordList 	= NULL;
	m_ppAttach 			= &m_pRecordList;		// first attach will be to m_pRecordList
	m_pszRecordID 	= NULL;
	m_pszDatabaseID = NULL;
	InterlockedIncrement(&g_cComponents);
}


CPHRecord::~CPHRecord()
{
	FreeRecordList();

	delete [] m_pszRecordID;
	delete [] m_pszDatabaseID;

	InterlockedDecrement(&g_cComponents);
}


HRESULT __stdcall CPHRecord::QueryInterface(const IID& iid, void** ppv)
{
	if (iid == IID_IUnknown)
    *ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IDSPRecord)
		*ppv = static_cast<IDSPRecord*>(this);
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}


ULONG	__stdcall CPHRecord::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}


ULONG	__stdcall CPHRecord::Release()
{
	if (InterlockedDecrement(&m_cRef) == 0) {
		delete this;
		return 0;
	}
	return m_cRef;
}

// IDSPRecord
DBRECENT * __stdcall CPHRecord::GetRecordList()
{
	return m_pRecordList;
}

LPSTR	__stdcall CPHRecord::GetRecordID()
{
	return m_pszRecordID;
}

LPSTR __stdcall CPHRecord::GetDatabaseID()
{
	return m_pszDatabaseID;
}

QUERY_STATUS __stdcall CPHRecord::GetError(LPSTR, int)
{
	return QUERY_OK;
}

////////////////////////////////
// non-interface methods


void CPHRecord::SetDatabaseID(LPSTR pszID)
{
	delete [] m_pszDatabaseID;

	m_pszDatabaseID = NULL;
	if (pszID)
		m_pszDatabaseID = SafeStrdupMT(pszID);
}


LPDBRECENT CPHRecord::CreateRecEnt(DWORD dwSize)
{
	LPDBRECENT pRecEnt = reinterpret_cast<LPDBRECENT>( DEBUG_NEW_NOTHROW char[sizeof(DBRECENT) + dwSize] );

	if (pRecEnt) {
		pRecEnt->pNext 				= NULL;
		pRecEnt->nName 				= DS_UNKNOWN;
		pRecEnt->lpszUserName = NULL;
		pRecEnt->nType 				= DST_ASCIIZ;			// gotta default to something!
		pRecEnt->dwSize 			= dwSize;
	}
	return pRecEnt;
}

LPDBRECENT CPHRecord::AddStringRecEnt(LPSTR lpsz, DS_FIELD dsName, LPSTR pszUser/*=NULL*/)
{
	LPDBRECENT pRecEnt = CreateRecEnt((DWORD)(strlen(lpsz)+1));
	if (pRecEnt) {
		pRecEnt->nName = dsName;
		if (pszUser)
			pRecEnt->lpszUserName = SafeStrdupMT(pszUser);

		strcpy((LPSTR)pRecEnt->data, lpsz);
		AttachRecEnt(pRecEnt);
	}
	return pRecEnt;
}

void CPHRecord::FreeRecEnt(DBRECENT *pRecEnt)
{
	if (pRecEnt) {
		delete [] pRecEnt->lpszUserName;

		delete [] reinterpret_cast<char *>(pRecEnt);
	}
}

void CPHRecord::AttachRecEnt(DBRECENT *pRecEnt)
{
	assert(pRecEnt->pNext==NULL);
	assert(m_ppAttach!=NULL);

	*m_ppAttach = pRecEnt;					// store pRecEnt in m_pRecordList(first time only) or (Previous)pRecEnt->pNext
	m_ppAttach = &pRecEnt->pNext;		// next attach position is new record's next pointer
}


void CPHRecord::FreeRecordList()
{
	LPDBRECENT pFree;
	 
	while (pFree = m_pRecordList) {
		m_pRecordList = pFree->pNext;
		FreeRecEnt(pFree);
	}
	m_ppAttach = &m_pRecordList;
}





