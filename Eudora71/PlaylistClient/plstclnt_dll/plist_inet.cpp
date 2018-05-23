// plist_inet.cpp -- encapsulates http, etc.

// TODO: this is mostly blue-sky right now. I don't have the time to spend
// working out a reasonable API for doing http. I suppose I could just
// borrow ideas from wininet, but that will have to wait until we ditch
// the MFC layer, which won't happen for a while. for now we just keep our
// global session object here.
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

//

#include "stdafx.h"

// basic mfc, and mfc/wininet
#include "afx.h"
#include "afxwin.h"
#include "afxinet.h"

// the usual suspects
#include "assert.h"

// our own silly ol' ass
#include "plist_inet.h"

#include "../../Version.h"

#include "DebugNewHelpers.h"

////////////////////////////////////////////////////////////////////////////////


//static CInternetSession* g_pTheSession = NULL;
HINTERNET g_pTheSession = 0;

class NetSession : public CInternetSession
{
	NetSession(){}
public:
	NetSession( char* userAgent )
		: CInternetSession( userAgent, 1, PRE_CONFIG_INTERNET_ACCESS,
				NULL, NULL, INTERNET_FLAG_ASYNC ){}
	~NetSession(){}
#if 0
	void OnStatusCallback( DWORD dwContext, DWORD dwInternetStatus,
		LPVOID lpvStatusInfo, DWORD dwStatusInfoLength )
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() );
		IInetContext* pctx = (IInetContext*) dwContext;

		if ( dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE ) {
			LPINTERNET_ASYNC_RESULT prslt =
					(LPINTERNET_ASYNC_RESULT) lpvStatusInfo;

			InetResult ir;
			ir.result = prslt->dwResult;
			ir.error = prslt->dwError;

			pctx->InetComplete( &ir );
		}
		else
			pctx->InetStatus( dwInternetStatus, lpvStatusInfo, dwStatusInfoLength );
	}
#endif
};

void CALLBACK dorko_e_porko
(
	HINTERNET /*hInternet*/,
	DWORD dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInfo,
	DWORD dwStatusInfoLength
)
{
	IInetContext* pctx = (IInetContext*) dwContext;

	if ( dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE ) {
		LPINTERNET_ASYNC_RESULT prslt =
				(LPINTERNET_ASYNC_RESULT) lpvStatusInfo;

		InetResult ir;
		ir.result = prslt->dwResult;
		ir.error = prslt->dwError;

		pctx->InetComplete( &ir );
	}
	else
		pctx->InetStatus( dwInternetStatus, lpvStatusInfo, dwStatusInfoLength );
}

int NetStartup()
{
#ifdef PLAYLIST_PARSER_TEST
    if ( !AfxWinInit( ::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0 ) ) {
        printf( "MFC Failed to initialize.\n" );
        return 1;
    }
#endif

    if ( !g_pTheSession ) {
//        g_pTheSession = new NetSession( "Secret Agent Man" );
//		g_pTheSession->EnableStatusCallback( TRUE );
		g_pTheSession = InternetOpen( "Eudora/" EUDORA_BUILD_VERSION " (Win)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
//		InternetSetStatusCallback( g_pTheSession, dorko_e_porko );
	}
    else
        assert( 0 );

        return (g_pTheSession == NULL);
}

int NetShutdown()
{
#if 0
    g_pTheSession->Close();
    delete g_pTheSession;
    g_pTheSession = NULL;
        return 0;
#endif
	InternetCloseHandle( g_pTheSession );
	g_pTheSession = 0;
	return 0;
}

#if 0
CInternetSession* NetGetSession()
{
    if ( !g_pTheSession )
        NetStartup();

    return g_pTheSession;
}
#endif

inet_handle NetGetSessionHandle()
{
	if ( !g_pTheSession )
		NetStartup();

	return g_pTheSession;
}


////////////////////////////////////////////////////////////////////////////////
// InetContext -- base class; impliments IInetContext interface

void InetContext::InetStatus( unsigned nStatus, void* pvStatusInfo,
		unsigned /*infoLen*/ )
{
	if ( nStatus == INTERNET_STATUS_HANDLE_CREATED ) {
		LPINTERNET_ASYNC_RESULT prslt =
				(LPINTERNET_ASYNC_RESULT) pvStatusInfo;

		if ( !hConnection )
			hConnection = (inet_handle) prslt->dwResult;
	}
}

char* NetErrorDesc( int ecode )
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ecode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	return (char*) lpMsgBuf;
}

void NetFreeError( char* msg )
{
	LocalFree( msg );
}
