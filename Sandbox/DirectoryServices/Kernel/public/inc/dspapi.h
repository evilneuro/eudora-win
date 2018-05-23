/******************************************************************************/
/*																										*/
/*	Name		:	DSPAPI.H			                                                */
/* Date     :  7/18/1997                                                      */
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

/*	Desc.		:	Directory Services <-> protocol interfaces 			            */
/*																			                     */
/******************************************************************************/
#ifndef __DSPAPI_H__
#define __DSPAPI_H__

#include "dsdefs.h"							/* Shared definitions for Dir. Services*/

interface IDSPEnumConfig;

interface IDSPProtocol : IUnknown
{
	/* ----------------------------------------------------------------------- */
	/* Initialize()																				*/
	/* Initialize will QueryInterface() the passed pUnkCaller for  				*/
	/* IID_IConfigData so that the protocol and its components can read and 	*/
	/* write configuration information in an abstracted manner through this 	*/
	/* interface.																					*/
	/* ----------------------------------------------------------------------- */
	virtual HRESULT		__stdcall Initialize(IUnknown *pUnkCaller) PURE;
	
	/* ----------------------------------------------------------------------- */
	/* CreateObject()																				*/
	/* Create object is used to create new instances of:								*/
	/* IID_IConfig		Creates an empty database information object.				*/
	/* ----------------------------------------------------------------------- */
	virtual HRESULT 		__stdcall CreateObject(REFIID riid,LPVOID *ppv) PURE;
	virtual HRESULT 		__stdcall DeleteObject(IUnknown * punkObject) PURE;
	virtual HRESULT		__stdcall EnumDatabases(IDSPEnumConfig **ppenum) PURE;
	
	/* ----------------------------------------------------------------------- */
	/* GetProtocolName() retrieves a string that can be displayed to the 		*/
	/* 						user which describes this protocol (ie. LDAP,PH, etc.)*/
	/* ----------------------------------------------------------------------- */
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) PURE;
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage) PURE;
	virtual DWORD			__stdcall GetProtocolFlags() PURE;
};
// {499E65B4-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSPProtocol,  	0x499e65b4,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSPConfig : IUnknown
{
	virtual HRESULT		__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize) PURE;
	virtual HBITMAP		__stdcall GetImage(int iImage) PURE;
	virtual LPSTR			__stdcall GetID() PURE;
	virtual int				__stdcall GetPropPageCount() PURE;
	virtual HRESULT		__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages) PURE;
	
	virtual HRESULT		__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) PURE;
	virtual HBITMAP		__stdcall GetProtocolImage(int iImage) PURE;
	virtual DWORD			__stdcall GetProtocolFlags() PURE;
	
	/* ----------------------------------------------------------------------- */
	/* CreateObject()																				*/
	/* Create object is current used to create newu instances of:  					*/
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	/* IID_Query		Creates an object to begin a new query transaction			*/
	/* IID_IPost		Creates an object to begin a new post transaction       	*/
	/* ----------------------------------------------------------------------- */
	virtual HRESULT 		__stdcall CreateObject(REFIID riid,LPVOID *ppv) PURE;
};
// {499E65B3-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSPConfig,		0x499e65b3,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

/* -------------------------------------------------------------------------- */
/* IPEnumConfig()																					*/
/* Enumerates databases that have been configured for access.						*/
/*																										*/
/* See Microsoft documentation for IEnumXXXXX  											*/
/* -------------------------------------------------------------------------- */
interface IDSPEnumConfig : IUnknown
{
	virtual HRESULT 		__stdcall Next(ULONG celt, IDSPConfig ** rgelt, ULONG * pceltFetched) PURE;
	virtual HRESULT 		__stdcall Skip(ULONG celt) PURE;
	virtual HRESULT 		__stdcall Reset() PURE;
	virtual HRESULT 		__stdcall Clone(IDSPEnumConfig ** ppenum) PURE;
};
// {499E65B2-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSPEnumConfig,	0x499e65b2,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

/* -------------------------------------------------------------------------- */
/* Length of the "key" buffer for GetRecord() in IDSPQuery	  						*/
/* -------------------------------------------------------------------------- */
#define KEY_STR_LEN	4096

interface IDSPQuery : IUnknown
{
	virtual HRESULT		__stdcall InitQuery(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField) PURE;
	virtual HRESULT		__stdcall SetSearchBase(LPSTR pszSearchBase) PURE;
	
	/* Consumer driven model																	*/
	virtual void        	__stdcall Start(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual void			__stdcall WaitForRecord(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual QUERY_STATUS	__stdcall GetRecord(IDSPRecord **ppRecord) PURE;
	virtual void         __stdcall Cancel() PURE;
	virtual QUERY_STATUS __stdcall GetLastError() PURE;
	virtual HRESULT     	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen) PURE;
};
// {499E65B1-01FB-11d1-8873-00805F4A192C}
DEFINE_GUID(IID_IDSPQuery,		0x499e65b1,0x1fb,0x11d1,0x88,0x73,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDSPPost : IUnknown
{
	virtual HRESULT      __stdcall Initialize(DWORD dwFlags) PURE;
	virtual void         __stdcall Start(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual void 			__stdcall Postable(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual QUERY_STATUS	__stdcall PostRecord(IDSPRecord *pRec,DWORD dwflags) PURE;
	virtual void         __stdcall Cancel() PURE;
	virtual QUERY_STATUS __stdcall GetLastError() PURE;
	virtual HRESULT    	__stdcall GetErrorString(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen) PURE;
};
// {4BE19271-307A-11d1-889B-00805F4A192C}
DEFINE_GUID(IID_IDSPPost,     0x4be19271,0x307a,0x11d1,0x88,0x9b,0x0,0x80,0x5f,0x4a,0x19,0x2c);

interface IDirConfig : IUnknown
{
     // [B] Creation Methods
     //  -> creates a new section
     virtual HRESULT 	__stdcall CreateDirSection( LPSTR szSection, BOOL *bExists ) PURE;
     //  -> creates a new key under the section
     virtual HRESULT 	__stdcall CreateDirKey( LPSTR szSection, LPSTR szKey ) PURE;
     //  -> deletes a section, all of its keys, and all of their values
     virtual HRESULT 	__stdcall DeleteDirSection( LPSTR szSection ) PURE;
     //  -> deletes a key and all of its value
     virtual HRESULT 	__stdcall DeleteDirKey( LPSTR szSection, LPSTR szKey ) PURE;


     // [C] Accessing Methods
     // A key can have only one value, thus the <key, value> pair is an "attribute=value" relationship.
     //
     // Both the section and keys must be created before these functions can be used (i.e., these functions
     // do not automatically create a section that does not exist).
     //
     //  -> return an integer value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall GetDirInteger( LPSTR szSection, LPSTR szKey, int   nDefault, int*    returned_value ) PURE;
     //  -> return a Boolean value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall GetDirBool(    LPSTR szSection, LPSTR szKey, BOOL  bDefault, BOOL*   returned_value ) PURE;
     //  -> return a DWORD value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall GetDirDword(   LPSTR szSection, LPSTR szKey, DWORD dwDefault, DWORD*  returned_value ) PURE;
     //  -> return a string value stored at <szSection, szKey>, into the callers allocated buffer (truncating if necessary), cbReturned indicates size of return_value buffer
	  virtual HRESULT		__stdcall GetDirStr(LPSTR szSection,LPSTR szKey,LPSTR default_value,DWORD cbDefault,LPSTR returned_value,DWORD* cbReturned) PURE;
     //  -> return a block of binary data stored at <szSection, szKey>, cbReturned modified to indicate size of block (on input it indicates size of return_value buffer)
	  virtual HRESULT		__stdcall GetDirBinary(LPSTR szSection,LPSTR szKey,LPBYTE default_value,DWORD cbDefault,LPBYTE returned_value,DWORD* cbReturned) PURE;

     //  -> return the names of each section one at a time (buffer is caller's allocated memory)
     virtual HRESULT 	__stdcall EnumDirSections(  LPSTR szSection, DWORD* cbBuffer ) PURE;
     //  -> return the names of each key, under the given section, one at a time (buffer is callers allocated memory), also returned is the type of data stored
     virtual HRESULT 	__stdcall EnumDirKeys(    LPSTR szSection, LPSTR szKey, DWORD* cbBuffer, DataType* type_of_stored_value ) PURE;
	  virtual HRESULT		__stdcall EndEnumDirKeys(LPSTR szSection) = 0;

     // [D] Modification Methods
     // A key can have only one value, thus the <key, value> pair is an "attribute=value" relationship.
     //
     // Both the section and keys must be created before these functions can be used (i.e., these functions
     // do not automatically create a section that does not exist).
     //
     //  -> sets an integer value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall SetDirInteger( LPSTR szSection, LPSTR szKey, int  new_value ) PURE;
     //  -> sets a Boolean value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall SetDirBool(    LPSTR szSection, LPSTR szKey, BOOL new_value ) PURE;
     //  -> sets a DWORD value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall SetDirDword(   LPSTR szSection, LPSTR szKey, DWORD new_value ) PURE;
     //  -> sets a string value stored at <szSection, szKey>
     virtual HRESULT 	__stdcall SetDirStr(     LPSTR szSection, LPSTR szKey, LPSTR new_value ) PURE;
     //  -> sets a block of binary data stored at <szSection, szKey>, cbReturned contains the count of byes in new_value
     virtual HRESULT 	__stdcall SetDirBinary(  LPSTR szSection, LPSTR szKey, LPBYTE new_value, DWORD cbValue ) PURE;


     // [E] Misc Methods
     //
     //  -> Called when one of the above functions returns an "E_" HRESULT (e.g., E_FAIL)
     //  -> returns "0" if no error, otherwise negative number indicates problem (e.g., no such section defined)
     virtual HRESULT 	__stdcall GetDirLastError( LONG* ResultCode ) PURE;
	
	  virtual HRESULT 	__stdcall GetFieldName(DS_FIELD nField,LPSTR pszBuffer,int nLen) PURE;
	  virtual HRESULT 	__stdcall GetFieldKey(DS_FIELD nField,LPSTR pszBuffer,int nLen) PURE;
	  virtual HRESULT 	__stdcall MapFieldKey(DS_FIELD *pnField,LPSTR pszKey) PURE;
};
DEFINE_GUID(IID_IDirConfig,   0xece47d70,0xfec3,0x11d0,0x88,0x64,0x0,0x80,0x5f,0x8a,0xd,0x74);

#endif

