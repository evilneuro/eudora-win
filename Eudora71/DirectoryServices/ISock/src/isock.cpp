//******************************************************************************/
/*																										*/
/*	Name		:	ISOCK.CPP  																		*/
/* Date     :  4/23/1997                                                      */
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

/*	Desc.		:	ISock object methods				  											*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514)
#include <afx.h>
#include <ole2.h>
#include <winsock.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "dllmain.h"
#include "isockp.h"
#include "QCUtils.h"


extern class CApp gApp;
extern long g_cComponents;

/* Static */
IUnknown * ISockp::CreateInstance()
{
	ISockp	*pSock = NULL;
	
	if(FALSE == gApp.IsInitialized())
		if(FALSE == gApp.Initialize(CFactory::s_hModule))
			return NULL;
	
	if(NULL == (pSock = DEBUG_NEW_NOTHROW ISockp))
		return NULL;
	
	return static_cast<ISock *>(pSock);
}

/* Execution stack frame item definition													*/
typedef struct TAGEXECFRAME
{
	struct TAGEXECFRAME	*pPrev;
	struct TAGEXECFRAME  *pNext;
	
	ISOCKCB					pCB;
	LPVOID					pCtx;
} EXECFRAME, *LPEXECFRAME;

/* Global Data																						*/
extern class CApp gApp;

/* IUnknown methods																				*/
STDMETHODIMP ISockp::QueryInterface(REFIID riid, void **ppv)
{
	if(TRUE == IsEqualIID(riid,IID_IUnknown) || TRUE == IsEqualIID(riid,IID_ISock)) {
		*ppv = static_cast<LPISOCK>(this);
	}
	else if(TRUE == IsEqualIID(riid,IID_ISock1)) {
		*ppv = static_cast<LPISOCK1>(this);
	}
	else {
      *ppv = NULL;
		return E_NOINTERFACE;
	}
	
	((IUnknown*)*ppv)->AddRef();
	return ResultFromScode(S_OK);
}

STDMETHODIMP_(ULONG) ISockp::AddRef(void)
{
	return InterlockedIncrement(&m_nRefs);	
}

STDMETHODIMP_(ULONG) ISockp::Release(void)
{
	if (0 == InterlockedDecrement(&m_nRefs)) {
		delete this;
		return 0;
	}
	return m_nRefs;
}


ISockp::ISockp(void) : m_nRefs(1)
{
	m_bInitialized	= FALSE;
   m_nEvent       = 0;
	m_nState			= ISOCK_UNINITIALIZED;
	m_bAsyncPending= FALSE;
	m_nErr 			= 0;
	m_pWSAData		= gApp.GetWSAData();
	m_dwFlags		= 0;
	m_pExecStack	= NULL;
	m_pCurFrame		= NULL;
	m_pSched			= NULL;
	m_pszHostName 	= NULL;
	m_nPort			= 0;
	m_s 				= NULL;
	m_hOp				= NULL;
	m_pNextCB		= NULL;
	m_pNextCtx		= NULL;
	m_pCB				= NULL;
	m_pCtx			= NULL;
	memset(m_pHostEnt,0,MAXGETHOSTSTRUCT);
	memset(&m_sin,0,sizeof(struct sockaddr_in));
	InterlockedIncrement(&g_cComponents);
}

ISockp::~ISockp(void)
{
	if(m_bInitialized) {
		m_bInitialized = FALSE;
		Cancel();						      /* Cancel notifies on this socket		*/
		gApp.SockListRemove(this);			/* Remove ourselfs from the main list	*/
		
		if(m_pSched) 		m_pSched->Release();

		delete m_pszHostName;

		if(m_s)           closesocket(m_s);
		if(m_pExecStack)  m_pExecStack->Release();
		
		m_pSched 		= NULL;
		m_pszHostName	= NULL;
		m_s 				= 0;
		m_pExecStack 	= NULL;
	}
	InterlockedDecrement(&g_cComponents);
}

HRESULT STDMETHODCALLTYPE ISockp::GetWinsockVersion(WORD *pwVersion)
{
	if(pwVersion) {
		*pwVersion = m_pWSAData->wVersion;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::GetWinsockMaxVersion(WORD *pwVersion)
{
	if(pwVersion) {
		*pwVersion = m_pWSAData->wHighVersion;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::GetWinsockDesc(LPSTR pszBuffer,int nBufLen)
{
	if(pszBuffer) {
		lstrcpyn(pszBuffer,m_pWSAData->szDescription,nBufLen);
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::GetWinsockStatus(LPSTR pszBuffer,int nBufLen)
{
	if(pszBuffer) {
		lstrcpyn(pszBuffer,m_pWSAData->szSystemStatus,nBufLen);
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::GetMaxSockets(unsigned short *pusMaxSock)
{
	if(pusMaxSock) {
		*pusMaxSock = m_pWSAData->iMaxSockets;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::GetMaxUdpSize(int *pusMaxUdp)
{
	if(pusMaxUdp) {
		*pusMaxUdp = m_pWSAData->iMaxUdpDg;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
	
}

void ISockp::ExecListFreeCB(LPITEM pItem, LPVOID /* pUser */)
{
	if(pItem)
		delete [] reinterpret_cast<char *>(pItem);
}

STDMETHODIMP ISockp::Initialize(LPSTR pszHost,int nPort,DWORD dwFlags)
{
	if(!gApp.GetWindow() || NULL == pszHost || 0 == *pszHost || TRUE == m_bInitialized)
		return E_FAIL;
	
	m_nErr 			= 0;
	m_nPort			= nPort;
	m_dwFlags		= dwFlags;
	
	if(m_pszHostName) {
		delete [] m_pszHostName;

		m_pszHostName = NULL;
	}
	
	if(pszHost && *pszHost) {
		m_pszHostName 	= SafeStrdupMT(pszHost);
	}
	
	if(NULL == (m_pSched = gApp.CreateSchedObj())) {
		return E_FAIL;
	}
	
	if(NULL == (m_pExecStack = gApp.CreateListObj())) {
		return E_FAIL;
	}
	
	m_pExecStack->Initialize(ExecListFreeCB,(LPVOID) this);
	
	/***************************************************************************/
	/* NOTE: We don't have to set the socket to non-blocking because				*/
	/* 		WSAAsyncSelect() will do that for us.										*/
	/***************************************************************************/
	if(INVALID_SOCKET == (m_s = socket(PF_INET,SOCK_STREAM,0))) {
		m_nErr = WSAGetLastError();
		return E_FAIL;
	}
	
	gApp.SockListAdd(this);
	m_bInitialized = TRUE;
   m_nState       = ISOCK_INITIALIZED;
	return S_OK;
}

STDMETHODIMP ISockp::GetSocketError(int *pErr)
{
	if(pErr) {
		*pErr = m_nErr;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::Readable(ISOCKCB pCB,LPVOID pCtx)
{
   DBGPOKE("Readable");
	if(ISOCK_CONNECTED != GetSocketState())
		return(ResultFromScode(E_FAIL));
	
	m_pCB = pCB;
	m_pCtx= pCtx;
	
	PushExecFrame(m_pCB,m_pCtx);
	
	m_nEvent |= (FD_READ | FD_CLOSE);
	/* Tell winsock to wake us up when data is ready									*/
	if(SOCKET_ERROR == WSAAsyncSelect(m_s,gApp.GetWindow(),WM_SOCKETEVENT,m_nEvent)) {
		/* Save the error and dispatch the users callback	 		  					*/
		SetSocketError(WSAGetLastError());
		DispatchNext();
		return(ResultFromScode(E_FAIL));
	}
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE ISockp::Read(LPSTR pBuf,int nLen,int *nBytesRead)
{
	int	nBytes = 0;
	
   DBGPOKE("Read");
	if(SOCKET_ERROR == (nBytes = recv(m_s,pBuf,nLen,0))) {
		SetSocketError(WSAGetLastError());
		return(ResultFromScode(E_FAIL));
	}
	SetSocketError(WSAGetLastError());
	if(nBytesRead) *nBytesRead = nBytes;
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE ISockp::Writable(ISOCKCB pCB,LPVOID pCtx)
{
   DBGPOKE("Writable");
	if(ISOCK_CONNECTED != GetSocketState())
		return(ResultFromScode(E_FAIL));
	
	m_pCB = pCB;
	m_pCtx= pCtx;
	
	PushExecFrame(m_pCB,m_pCtx);
	
	m_nEvent |= (FD_WRITE | FD_CLOSE);
	/* Tell winsock to wake us up when data is ready									*/
	if(SOCKET_ERROR == WSAAsyncSelect(m_s,gApp.GetWindow(),WM_SOCKETEVENT,m_nEvent)) {
		/* Save the error and dispatch the users callback	 		  					*/
		SetSocketError(WSAGetLastError());
		PushExecFrame(m_pCB,m_pCtx);
		DispatchNext();
		return(ResultFromScode(E_FAIL));
	}
	SetSocketError(WSAGetLastError());
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE ISockp::Write(LPSTR pBuf,int nLen,int *nBytesWritten)
{
	int	nBytes = 0;
	
   DBGPOKE("Write");
	if(SOCKET_ERROR == (nBytes = send(m_s,pBuf,nLen,0))) {
		SetSocketError(WSAGetLastError());
		return(ResultFromScode(E_FAIL));
	}
	SetSocketError(WSAGetLastError());
	if(nBytesWritten) *nBytesWritten = nBytes;
	return(ResultFromScode(S_OK));
}

void ISockp::DoConnect(void)
{
	int				err	= 0;
	
	/* Set our state to connecting, then start the connect							*/
	SetSocketState(ISOCK_CONNECTING);
	
	/* Tell winsock to wake us up on connect												*/
	if(SOCKET_ERROR == WSAAsyncSelect(m_s,gApp.GetWindow(),WM_SOCKETEVENT,FD_CONNECT)) {
		/* Save the error and dispatch the users callback	 		  					*/
		SetSocketError(WSAGetLastError());
		PushExecFrame(m_pCB,m_pCtx);
		DispatchNext();
		return;
	}
	
	/* OK, tell winsock to connect															*/
	if(SOCKET_ERROR == connect(m_s,(struct sockaddr *) &m_sin, sizeof(m_sin)) && WSAEWOULDBLOCK != (err = WSAGetLastError())) {
		/* Save the error and dispatch the users callback	 		  					*/
		SetSocketError(err);
		PushExecFrame(m_pCB,m_pCtx);
	}
	else {
		PushExecFrame(ISockp::ConnectCB,(LPVOID) this);
		return; /* Winsock will call us when we've connected...or not!				*/
	}
	
	DispatchNext();
}

void ISockp::ConnectCB(LPVOID pCtx)
{
	LPISOCKP	pS 	= (LPISOCKP) pCtx;
	int		nErr 	= 0;
		
	/* Handle any socket errors up front.													*/
	pS->GetSocketError(&nErr);
	if(0 != nErr) {
		/* Set the socket state to error and dispatch the users callback	 		*/
		pS->SetSocketState(ISOCK_ERROR);
		pS->PushExecFrame(pS->m_pCB,pS->m_pCtx);
		pS->DispatchNext();
		return;
	}
	
	switch(pS->GetSocketState()) {
		case ISOCK_LOOKUPHOST: {
			struct hostent He 	= {0};
			
			/* Get the hostent and setup the address for the connect					*/
			pS->GetHostEnt(&He);
			memcpy(&pS->m_sin.sin_addr,He.h_addr,He.h_length);
			pS->DoConnect();
			return;
		}
		case ISOCK_CONNECTING: {
			/* Set our state to CONNECTED	and dispatch the users callback			*/
			pS->SetSocketState(ISOCK_CONNECTED);
			pS->PushExecFrame(pS->m_pCB,pS->m_pCtx);
			pS->DispatchNext();
			break;
		}
	}
}

void ISockp::GetHostByNameCB(LPVOID pCtx)
{
	LPISOCKP	pISock = (LPISOCKP) pCtx;
	
	pISock->m_pCB(pISock->m_pCtx);
}

STDMETHODIMP ISockp::Connect(ISOCKCB pCB,LPVOID pCtx)
{
	/* Store the users callback info for later											*/
	m_pCB = pCB;
	m_pCtx= pCtx;
	
	m_sin.sin_family 	= AF_INET;
	m_sin.sin_port 	= htons( static_cast<u_short>(m_nPort) );
	
	/* If our hostname is not a dotted quad string										*/
	if(INADDR_NONE == (m_sin.sin_addr.s_addr = inet_addr(m_pszHostName))) {
		
		/* Set our internal state to indicate we are looking up the host			*/
		SetSocketState(ISOCK_LOOKUPHOST);
		
		/* Setup our internal callback as the next continuation to run				*/
		PushExecFrame(ISockp::ConnectCB,(LPVOID) this);
		
		/* Then schedule the WSAAsyncGetHostByName()										*/
		m_hOp 	= WSAAsyncGetHostByName(gApp.GetWindow(),WM_SOCKETMSG,m_pszHostName,(LPSTR)m_pHostEnt,MAXGETHOSTSTRUCT);
      m_nErr 	= WSAGetLastError();		/* Store any error in our object			*/
		
		/* If we failed to post the gethostbyname, return failure					*/
		if(0 == m_hOp)
			return(ResultFromScode(E_FAIL));
		
		/* Set our internal state to indicate we are waiting for an event			*/
		m_bAsyncPending = TRUE;
	}
	
	/* Otherwise we can schedule our connect directly									*/
	else {
		DoConnect();
	}
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE ISockp::Cancel(void)
{
	if(m_pSched) m_pSched->Cancel();
	
	if(m_hOp && TRUE == m_bAsyncPending) {
		WSACancelAsyncRequest(m_hOp);
	}
	return(ResultFromScode(S_OK));
}

//HRESULT STDMETHODCALLTYPE ISockp::SetNextDispatch(ISOCKCB pCB,LPVOID pCtx)
//{
//	if(pCB) {
//		m_pNextCB = pCB;
//		m_pNextCtx= pCtx;
//		return(ResultFromScode(S_OK));
//	}
//	return(ResultFromScode(S_FALSE));
//}

HRESULT STDMETHODCALLTYPE ISockp::GetHostEnt(struct hostent * pHe)
{
	if(pHe) {
		memcpy(pHe,m_pHostEnt,sizeof(struct hostent));
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

/******************************************************************************/
/* Protected Methods																				*/
/******************************************************************************/

ISOCK_STATE ISockp::GetSocketState(void)
{
	return(m_nState);
}

void ISockp::SetSocketState(ISOCK_STATE nState)
{
	m_nState = nState;
}

								

/******************************************************************************/
/* IID_ISock1 Methods	  																		*/
/******************************************************************************/
HRESULT STDMETHODCALLTYPE ISockp::GetAsyncHandle(HANDLE *pH)
{
	if(m_hOp && pH) {
		*pH = m_hOp;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE ISockp::SetSocketError(int err)
{
	m_nErr = err;
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE ISockp::PushExecFrame(ISOCKCB pCB,LPVOID pCtx)
{
	LPEXECFRAME pFrame = DEBUG_NEW_NOTHROW EXECFRAME;
	
	if(pFrame) {
		pFrame->pCB = pCB;
		pFrame->pCtx= pCtx;
		m_pExecStack->Push(pFrame);
	}
	
	return(ResultFromScode(E_OUTOFMEMORY));
}

HRESULT STDMETHODCALLTYPE ISockp::DispatchNext(void)
{
	m_pCurFrame = (LPEXECFRAME) m_pExecStack->Pop();
	
	if(m_pCurFrame) {
		m_pSched->Schedule((CONTINUATION) m_pCurFrame->pCB,m_pCurFrame->pCtx);
		//m_pCurFrame->pCB(m_pCurFrame->pCtx);

		delete [] reinterpret_cast<char *>(m_pCurFrame);
		m_pCurFrame = NULL;
		return(ResultFromScode(S_OK));
	}
	return(ResultFromScode(E_FAIL));
}

SOCKET ISockp::GetSocket(void)
{
	return(m_s);
}

HRESULT STDMETHODCALLTYPE ISockp::GetSocketEvents(int *pnEvents)
{
	if(!pnEvents)
		return(ResultFromScode(E_FAIL));

	*pnEvents = m_nEvent;
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE ISockp::SetSocketEvents(int nEvents)
{
	m_nEvent = nEvents;
	return(ResultFromScode(S_OK));
}




