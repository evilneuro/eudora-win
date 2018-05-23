/******************************************************************************/
/*																										*/
/*	Name		:	EnumDB.h      																   */
/* Date     :  10/10/1997                                                     */
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

/*	Desc.		:	IDSPEnumConfig Definitions 												*/
/*																										*/
/******************************************************************************/
#ifndef __ENUMDB_H__
#define __ENUMDB_H__

#include "protocol.h"

class CEnumDatabase : public IDSPEnumConfig
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();

	/* IDSPEnumConfig																				*/
	virtual HRESULT 		__stdcall Next(ULONG celt, IDSPConfig ** rgelt, ULONG * pceltFetched);
	virtual HRESULT 		__stdcall Skip(ULONG celt);
	virtual HRESULT 		__stdcall Reset();
	virtual HRESULT 		__stdcall Clone(IDSPEnumConfig ** ppenum);

   CEnumDatabase(CProtocol *pProtocol,IListMan *pDatabaseList,BOOL bClone);
   ~CEnumDatabase();

protected:
   long                 m_cRef;
   CProtocol            *m_pProtocol;
   IListMan             *m_pList;
   LPDBENT              m_pCur;
   DWORD                m_dwIdx;
   BOOL                 m_bToast;
};

#endif

