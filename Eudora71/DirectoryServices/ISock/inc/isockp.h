/******************************************************************************/
/*																										*/
/*	Name		:	ISOCKP.H																			*/
/* Date     :  7/15/1997                                                      */
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

/*	Desc.		:	ISock Private Class definition	 										*/
/*																										*/
/******************************************************************************/
#ifndef _ISOCKP_H_
#define _ISOCKP_H_

#include "isock.h"							/* Public interface definition			*/

typedef class ISock1 *LPISOCK1;
class ISock1 : public IUnknown
{
public:
	/* Gets the HANDLE of a pending WSAAsyncXXXXX operation							*/
	virtual HRESULT STDMETHODCALLTYPE GetAsyncHandle(HANDLE *pH) PURE;
	
	/* Sets the value returned from GetSocketError()									*/
	virtual HRESULT STDMETHODCALLTYPE SetSocketError(int err) PURE;
	
	/* Get and set the WSA_AsyncSelect() FD_XXXXX event flags for					*/
	virtual HRESULT STDMETHODCALLTYPE GetSocketEvents(int *pnEvents) PURE;
	virtual HRESULT STDMETHODCALLTYPE SetSocketEvents(int nEvents) PURE;
	
	/* Gets the socket assosiated with this object										*/
	virtual SOCKET GetSocket(void) PURE;
	
	/* Pushes an execution frame onto the execution stack								*/
	virtual HRESULT STDMETHODCALLTYPE PushExecFrame(ISOCKCB pCB,LPVOID pCtx) PURE;
	
	/* Dispatche the continuation routine from the next execution frame			*/
	virtual HRESULT STDMETHODCALLTYPE DispatchNext(void) PURE;
};

typedef struct TAGEXECFRAME *LPEXECFRAME;
typedef class ISockp * LPISOCKP;
class ISockp : public ISock , public ISock1
{
public:
	/* IUnknown methods																			*/
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	
	/* Begin IID_ISock																			*/
	
	/* These methods can be called before Initialize()									*/
	virtual HRESULT STDMETHODCALLTYPE GetWinsockVersion(WORD *pwVersion);
	virtual HRESULT STDMETHODCALLTYPE GetWinsockMaxVersion(WORD *pwVersion);
	virtual HRESULT STDMETHODCALLTYPE GetWinsockDesc(LPSTR pszBuffer,int nBufLen);
	virtual HRESULT STDMETHODCALLTYPE GetWinsockStatus(LPSTR pszBuffer,int nBufLen);
	virtual HRESULT STDMETHODCALLTYPE GetMaxSockets(unsigned short *pusMaxSock);
	virtual HRESULT STDMETHODCALLTYPE GetMaxUdpSize(int *pusMaxUdp);
	
	/* This method must be called before any of the following methods.			*/
	virtual HRESULT STDMETHODCALLTYPE Initialize(LPSTR pszHost,int nPort,DWORD dwFlags);
	
	/* Create a connection to the host that was specified in Initialize()		*/
	virtual HRESULT STDMETHODCALLTYPE Connect(ISOCKCB pCB,LPVOID pCtx);
	
	/* Schedule a read operation when data is available								*/
	virtual HRESULT STDMETHODCALLTYPE Readable(ISOCKCB pCB,LPVOID pCtx);
	
	/* Read data from socket																	*/
	virtual HRESULT STDMETHODCALLTYPE Read(LPSTR pBuf,int nLen,int *nBytesRead);
	
	/* Schedule a write operation																*/
	virtual HRESULT STDMETHODCALLTYPE Writable(ISOCKCB pCB,LPVOID pCtx);
	
	/* Write data to the socket																*/
	virtual HRESULT STDMETHODCALLTYPE Write(LPSTR pBuf,int nLen,int *bBytesWritten);
	
	/* Cancels any pending Async socket operations										*/
	virtual HRESULT STDMETHODCALLTYPE Cancel(void);
	
	/* Gets the winsock socket error assoiated with the last socket operation.	*/
	/* This value will remain valid until the next socket operation.				*/
	virtual HRESULT STDMETHODCALLTYPE GetSocketError(int *pErr);
	
	/* End IID_ISock																				*/

	
	
	/* Begin IID_ISock1																			*/
	
	/* Gets the HANDLE of a pending WSAAsyncXXXXX operation							*/
	virtual HRESULT STDMETHODCALLTYPE GetAsyncHandle(HANDLE *pH);
	
	/* Sets the value returned from GetSocketError()									*/
	virtual HRESULT STDMETHODCALLTYPE SetSocketError(int err);
	
	/* Get and set the WSA_AsyncSelect() FD_XXXXX event flags for					*/
	virtual HRESULT STDMETHODCALLTYPE GetSocketEvents(int *pnEvents);
	virtual HRESULT STDMETHODCALLTYPE SetSocketEvents(int nEvents);
	
	/* Gets the socket assosiated with this object										*/
	virtual SOCKET GetSocket(void);
	
	/* Pushes an execution frame onto the execution stack								*/
	virtual HRESULT STDMETHODCALLTYPE PushExecFrame(ISOCKCB pCB,LPVOID pCtx);
	
	/* Dispatche the continuation routine from the next execution frame			*/
	virtual HRESULT STDMETHODCALLTYPE DispatchNext(void);
		
	/* End IID_ISock1		 		  																*/
	
	
	static IUnknown *CreateInstance();
	
	/* Static callback functions																*/
	static void GetHostByNameCB(LPVOID pCtx);
	static void ConnectCB(LPVOID pCtx);
	static void ExecListFreeCB(LPITEM pItem,LPVOID pUser);
	
	ISockp(void);
   ~ISockp(void);
	
protected:
	friend class CApp;
	
	virtual HRESULT STDMETHODCALLTYPE GetHostEnt(struct hostent * pHe);
	virtual ISOCK_STATE GetSocketState(void);
	virtual void SetSocketState(ISOCK_STATE nState);
	virtual void DoConnect(void);
	
	/* Object state																				*/
	LONG 						m_nRefs;	  	  	/* Object referenece count					*/
	BOOL						m_bInitialized;/* TRUE = ::Initialize() already called*/
	ISOCK_STATE				m_nState;	  	/* Current state of socket					*/
	BOOL						m_bAsyncPending;/* Waiting for async op to complete?	*/
	int						m_nErr;			/* Last socket error							*/
	WSADATA					*m_pWSAData;	/* Pointer to WSADATA from WSAStartup()*/
	DWORD						m_dwFlags;		/* Unused (Must be ISOCK_FLAG_NOFLAGS)	*/
	LPILISTMAN				m_pExecStack;	/* Continuation execution stack			*/
	LPEXECFRAME				m_pCurFrame;	/* Current execution frame					*/
	LPISCHEDULE				m_pSched;		/* Our scheduling object					*/
	
	/* Connection information																	*/
	LPSTR						m_pszHostName;	/* Hostname/Dotted-Quad address			*/
	int						m_nPort;		  	/* Connection port							*/
	char						m_pHostEnt[MAXGETHOSTSTRUCT];
	struct sockaddr_in	m_sin;
	SOCKET					m_s;	  	  	  	/* Our socket									*/
	HANDLE					m_hOp;			/* Handle of current async operation	*/
	int						m_nEvent;		/* Events waiting on (FD_READ etc.)		*/
	
	/* Continuation information																*/
	ISOCKCB					m_pNextCB;
	LPVOID					m_pNextCtx;

	/* User callback data																		*/
	ISOCKCB					m_pCB;
	LPVOID					m_pCtx;
};

/* {B72A06A1-FD4F-11d0-8871-00805F4A192C}													*/
DEFINE_GUID(IID_ISock1, 0xb72a06a1, 0xfd4f, 0x11d0, 0x88, 0x71, 0x0, 0x80, 0x5f, 0x4a, 0x19, 0x2c);

#endif



