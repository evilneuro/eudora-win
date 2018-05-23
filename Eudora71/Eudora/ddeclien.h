// ddeclien.h
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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

#ifndef __DDECLIEN_H__
#define __DDECLIEN_H__
////////////////////////////////////////////////////////////////////////
//
// CDDEClient
// Class instantiated and used by DDE client side.
//
////////////////////////////////////////////////////////////////////////

#include <ddeml.h>

class CDDEClient
{
public:
	CDDEClient(void);
	~CDDEClient(void);

	DWORD QueryServers(const char* pServiceName, const char* pTopicName);
	BOOL RegisterNetscapeProtocol(const char* pProtocol, BOOL isRegister = TRUE);
	BOOL OpenNetscapeURL(const char* pCmd);
	BOOL GetEudoraOriginator(CString& originatorAddress);
	BOOL GetEudoraNicknames(CString& nickNames);
	BOOL ExpandEudoraNickname(const char* nickName, CString& expandedNames);
	BOOL GetNextMessageId(CString& messageId, BOOL sortByDate, BOOL unreadOnly);
	BOOL GetMessageById(CString& messageData, const char* messageId,
						BOOL bodyAsFile, BOOL envelopeOnly,
						BOOL markAsRead, BOOL wantAttachments);
	BOOL PutMessageById(const char* messageData, CString& messageId);
	BOOL DeleteMessageById(const char* messageId);
	BOOL GetEudoraMAPIServerVersion(CString& versionInfo);

private:
	CDDEClient(const CDDEClient&);		// not implemented
	void operator=(const CDDEClient&);	// not implemented

#ifdef WIN32
	static HDDEDATA CALLBACK DDECallback
#else
	static HDDEDATA _export CALLBACK DDECallback
#endif // WIN32
	(
		UINT wType, 
		UINT wFmt, 
		HCONV hConv,
		HSZ hsz1, 
		HSZ hsz2, 
		HDDEDATA hDDEData, 
		DWORD dwData1, 
		DWORD dwData2
	);

	BOOL DoLogon_(void);
	void DoLogoff_(void);

	DWORD m_InstId;
};

#endif // __DDECLIEN_H__
