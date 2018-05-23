/******************************************************************************/
/*																										*/
/*	Name		:	RECORD.CPP 		  																*/
/* Date     :  8/8/1997		                                                   */
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

/*	Desc.		:	Directory Services Error Record Object implementation				*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "dspapi.h"
#include "factory.h"
#include "record.h"
#include "resource.h"


extern long g_cComponents;

CErrorRecord::CErrorRecord(QUERY_STATUS qErr,LPSTR pszErrorText) : m_cRef(1)
{
	m_pRecs	= NULL;
	memset(m_szUserText,0,sizeof(m_szUserText));
	
	if(pszErrorText)
	{
		int		nLastChar = sizeof(m_szUserText) - 1;
		strncpy(m_szUserText,pszErrorText, nLastChar);
		m_szUserText[nLastChar] = '\0';
	}
	
	m_nQueryError	= qErr;
	
	InterlockedIncrement(&g_cComponents);
}

CErrorRecord::~CErrorRecord()
{
	delete m_pRecs;
	m_pRecs = NULL;
	
	InterlockedDecrement(&g_cComponents);
}

HRESULT	__stdcall CErrorRecord::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IDSPRecord)
		*ppv = static_cast<IDSPRecord*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CErrorRecord::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CErrorRecord::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

DBRECENT *	__stdcall CErrorRecord::GetRecordList()
{
	return NULL;
}

LPSTR __stdcall CErrorRecord::GetRecordID()
{
	return NULL;
}

LPSTR __stdcall CErrorRecord::GetDatabaseID()
{
	return NULL;
}

QUERY_STATUS __stdcall CErrorRecord::GetError(LPSTR pszBuffer,int nLen)
{
	if(pszBuffer && nLen)
		lstrcpyn(pszBuffer,m_szUserText,nLen);
	
	return m_nQueryError;
}

