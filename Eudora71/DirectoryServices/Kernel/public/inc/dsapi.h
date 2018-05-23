/******************************************************************************/
/*																										*/
/*	Name		:	DSAPI.H			                                                */
/* Date     :  6/9/1997                                                       */
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

/*	Desc.		:	UI <-> Directory Services API Definitions	 			            */
/*																			                     */
/******************************************************************************/
#ifndef __DSAPI_H__
#define __DSAPI_H__

#include "dsdefs.h"

typedef void (*PETBCB)(char *, void *);

interface IDSEnumProtocols;
interface IDSEnumConfig;

interface IDSProtocol : IUnknown
{
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) PURE;
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage) PURE;
	virtual DWORD			__stdcall GetProtocolFlags() PURE;
};
// {499E65B5-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSProtocol,     0x499e65b5,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSEnumProtocols : IUnknown
{
	virtual HRESULT 		__stdcall Next(ULONG celt, IDSProtocol ** rgelt, ULONG * pceltFetched) PURE;
	virtual HRESULT 		__stdcall Skip(ULONG celt) PURE;
	virtual HRESULT 		__stdcall Reset(void) PURE;
	virtual HRESULT 		__stdcall Clone(IDSEnumProtocols ** ppenum) PURE;
};
// {499E65B7-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSEnumProtocols,0x499e65b7,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSDatabase : IUnknown
{
	virtual HRESULT		__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize) PURE;
	virtual HBITMAP		__stdcall GetImage(int iImage) PURE;
	virtual LPSTR			__stdcall GetDatabaseID() PURE;
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) PURE;
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage) PURE;
	virtual DWORD			__stdcall GetProtocolFlags() PURE;
	
	virtual int				__stdcall GetPropPageCount() PURE;
	virtual HRESULT		__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages) PURE;

};
// {499E65B9-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSDatabase,    0x499e65b9,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSEnumDatabases : IUnknown
{
	virtual HRESULT 		__stdcall Next(ULONG celt, IDSDatabase ** rgelt, ULONG * pceltFetched) PURE;
	virtual HRESULT 		__stdcall Skip(ULONG celt) PURE;
	virtual HRESULT 		__stdcall Reset(void) PURE;
	virtual HRESULT 		__stdcall Clone(IDSEnumDatabases ** ppenum) PURE;
};
// {499E65B8-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSEnumDatabases, 0x499e65b8,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSQuery : IUnknown
{
	virtual HRESULT      __stdcall Initialize(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField) PURE;
	virtual HRESULT		__stdcall AddDatabase(IDSDatabase *pDatabase) PURE;
	virtual HRESULT		__stdcall SetSearchBase(LPSTR pszSearchBase) PURE;	
	virtual void         __stdcall Start(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual void			__stdcall WaitForRecord(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual QUERY_STATUS	__stdcall GetRecord(IDSPRecord **ppRecord) PURE;
	virtual void         __stdcall Cancel() PURE;
	virtual QUERY_STATUS __stdcall GetLastError() PURE;
	virtual HRESULT    	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen) PURE;
};
// {499E65BA-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSQuery,        0x499e65ba,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSPost : IUnknown
{
	virtual HRESULT      __stdcall Initialize(IDSDatabase *pIDatabase,DWORD dwFlags) PURE;
	virtual void         __stdcall Start(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual void 			__stdcall Postable(DBQUERYCB pCB,LPVOID pCtx) PURE;
   virtual QUERY_STATUS	__stdcall PostRecord(IDSPRecord *pRec,DWORD dwflags) PURE;
	virtual void         __stdcall Cancel() PURE;
	virtual QUERY_STATUS __stdcall GetLastError() PURE;
	virtual HRESULT    	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen) PURE;
};
// {4BE19272-307A-11d1-889B-00805F4A192C}
DEFINE_GUID(IID_IDSPost,         0x4be19272,0x307a,0x11d1,0x88,0x9b,0x0,0x80,0x5f,0x4a,0x19,0x2c);

// -> supported flag values for export routines
#define EXPORT_AS_SIMPLE_TABLE  0x01

interface IDirServ : IUnknown
{
	virtual HRESULT		__stdcall Initialize() PURE;
	
	virtual HRESULT 		__stdcall EnumProtocols(IDSEnumProtocols ** ppIEnum) PURE;
	virtual HRESULT 		__stdcall EnumDatabases(IDSProtocol *pProto,IDSEnumDatabases ** ppIEnum) PURE;
	virtual IDSDatabase *__stdcall FindDatabase(LPSTR pszDatabaseID) PURE;
	
	virtual HRESULT      __stdcall CreateDatabase(IDSProtocol *pProto,IDSDatabase **pDatabase) PURE;
	virtual HRESULT      __stdcall DeleteDatabase(IDSDatabase *pDatabase) PURE;
	
	/* Used to create an IID_IDSQuery or IID_IDSPost									*/
	virtual HRESULT		__stdcall CreateObject(REFIID riid,void **ppv) PURE;
	
	virtual HRESULT		__stdcall ExportToHTML(IDSPRecord *pRec,LPSTR pszFilename,BOOL bOverWrite,DWORD dwflags) PURE;

	virtual HRESULT 		__stdcall ExportToText(IDSPRecord* pSearchHit,LPSTR pszFileName,BOOL OverWrite) PURE;
	virtual HRESULT 		__stdcall ExportToVCard(IDSPRecord* pSearchHit,LPSTR pszFileName,BOOL OverWrite) PURE;
	virtual HRESULT 		__stdcall ExportToBuffer(IDSPRecord* pSearchHit, PETBCB petbcb, void *pv) PURE;
	virtual HRESULT 		__stdcall ExportToBufferFormatted(void *data, PETBCB petbcb, void *pv) PURE;
};
// {499E65B6-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDirServ,        0x499e65b6,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

// {499E65BB-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(CLSID_DirServ,      0x499e65bb,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

#endif
