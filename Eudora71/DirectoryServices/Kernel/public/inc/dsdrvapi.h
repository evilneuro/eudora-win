/******************************************************************************/
/*																			  */
/*	Name		:	DSDRVAPI.H													  */
/* Date     :  6/9/1997              	                                      */
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

/*	Desc.		:	Directory Services Protocol Driver API
/*																			  */
/******************************************************************************/
#ifndef __DSDRVAPI_H__
#define __DSDRVAPI_H__

// Class created by driver DLL via CreateObject.  It basically, is an IUknown which at the
// protocol level contains session and/or object information.  It is used for it's QI interface
// to create the enum, query, etc. interfaces.

typedef class IDSProtocolDriver	FAR * LPIDSPROTOCOLDRIVER;
typedef class IDSServDBQuery		FAR * LPIDSSERVDBQUERY;
typedef class IDSServDBWrite		FAR * LPIDSSERVDBWRITE;

class IDSProtocolDriver:public IUnknown
{
public:
	// Allows more granular control over object creation.  If the object fails to initialize
	// properly, it can do so here, returning an error string.  This string is owned by the 
	// object and will become invalid after the object is released.

	LPSTR				virtual Initialize() = 0;			// returns error string on failure, NULL on success...

	// Returns next protocol managed by driver...
	// NOTE - Protocol is returned after AddRef - Must call Release()!!!!

	LPIDSPROTOCOL	virtual NextProtocol(LPIDSPROTOCOL pCur, BOOL bRelease=FALSE) = 0;		// returns next protocol...
};

// Query object for server-based DB queries.  Note that this is similar to IDQuery, but 
// because it is derived from a server, it does not require the server stuff...

class IDSServDBQuery : public IUnknown
{
public:
	virtual void			InitQuery(LPSTR pszKey) PURE;
	virtual void			InitQuery(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField) PURE;
	virtual void			Start(DBQUERYCB pCB,LPVOID pCtx) PURE;
	virtual HRESULT		SetSearchBase(LPSTR pszSearchBase) PURE;
	virtual void			Stop() PURE ;
	virtual QUERY_STATUS	GetLastError() PURE ;
	virtual LPSTR			GetErrorString(QUERY_STATUS nCode) PURE;
};


class IDSServDBWrite : public IUnknown
{
public:
	virtual void			Save(LPDBRECENT *ppRecord, DBQUERYCB pfn, LPVOID pUser, DWORD dwFlags) PURE;
	virtual WRITE_STATUS GetLastError() PURE;
	virtual LPSTR			GetErrorString(WRITE_STATUS nCode) PURE;
};

// GUIDs placed in this file are protected from the API.  Although the user
// interface may see the resulting objects (IDSServer, IDSProtocol), it should 
// NOT have access to the underlying GUIDs for instantiating the drivers directly.

DEFINE_GUID(IID_IDSServDBQuery	,0xefb1c120,0xef15,0x11d0,0x9b,0xdb,0xd8,0x6e,0x03,0xc1,0x00,0x00);
DEFINE_GUID(IID_IDSProtocol		,0xcaa00b80,0xeefc,0x11d0,0x9b,0xdb,0xd8,0x6e,0x03,0xc1,0x00,0x00);
DEFINE_GUID(IID_IDSServer			,0x13fe16a0,0xeefd,0x11d0,0x9b,0xdb,0xd8,0x6e,0x03,0xc1,0x00,0x00);
DEFINE_GUID(IID_IDSServDBWrite	,0x643b2f40,0xf3a8,0x11d0,0x9b,0xdb,0xd8,0x6e,0x03,0xc1,0x00,0x00);

// This GUID is used in the case when the protocol DLL is loaded via LoadLibrary.  Each
// Directory Services Protocol Driver should support this GUID in addition to it's own...

DEFINE_GUID(IID_IDSProtocolDriver,0x2d6c7500,0xef02,0x11d0,0x9b,0xdb,0xd8,0x6e,0x03,0xc1,0x00,0x00);
DEFINE_GUID(CLSID_IDSProtocolDriver,0x2d6c7500,0xef02,0x11d0,0x9b,0xdb,0xd8,0x6e,0x03,0xc1,0x00,0x00);

#endif

