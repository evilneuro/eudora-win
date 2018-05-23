/******************************************************************************/
/*																										*/
/*	Name		:	PROTOCOL.H	  																   */
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

/*	Desc.		:	IDSPProtocol Definitions   												*/
/*																										*/
/******************************************************************************/
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <ilist.h>
#include <dspapi.h>

typedef struct tagDBENT {
   ITEM           item;                /* Prev/Next pointers                  */
   IUnknown       *pUnkDatabase;       /* IUnknown of CDatabase               */
} DBENT, *LPDBENT;

class CProtocol : public IDSPProtocol
{
public:
	/* IUnknown																						*/
	virtual HRESULT		__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG			__stdcall AddRef();
	virtual ULONG			__stdcall Release();

	/* IDSPProtocol																				*/
	virtual HRESULT		__stdcall Initialize(IUnknown *pUnkCaller);
	virtual HRESULT 		__stdcall CreateObject(REFIID riid,LPVOID *ppv);
	virtual HRESULT 		__stdcall DeleteObject(IUnknown * punkObject);
	virtual HRESULT		__stdcall EnumDatabases(IDSPEnumConfig **ppenum);
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage);
	virtual DWORD			__stdcall GetProtocolFlags();

	/* CProtocol																				   */
   virtual HRESULT      __stdcall GetDirConfig(IDirConfig **ppIDirConfig);
   static IUnknown *CreateInstance();
   static void DBENTFreeCB(LPITEM pItem,LPVOID pUser);
   CProtocol();
   ~CProtocol();

protected:
   virtual HRESULT      __stdcall CreateDatabase(LPSTR pszPath,LPDBENT *ppEnt);
   virtual HRESULT      __stdcall LoadAddressBooks();

	LONG					m_cRef;				/* Reference count							*/
   HINSTANCE         m_hInst;          /* Module instance                     */
   LPSTR             m_pszName;        /* Protocol Name                       */
   HBITMAP           m_hbitSmall;      /* Small Bitmap                        */
   IListMan          *m_pDatabaseList; /* List of addressbook database objects*/
   IDirConfig        *m_pIDirConfig;   /* Callers IDirConfig interface        */
} ;



// {CDAEADF1-40F4-11d1-889F-00805F4A192C}
#define CLSID_DsProtocolEUBK_STR    L"{CDAEADF1-40F4-11d1-889F-00805F4A192C}"
DEFINE_GUID(CLSID_DsProtocolEUBK,   0xcdaeadf1,0x40f4,0x11d1,0x88,0x9f,0x0,0x80,0x5f,0x4a,0x19,0x2c);

#endif



