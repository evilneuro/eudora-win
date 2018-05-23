// QCSMTPSettings.h
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#ifndef _QCSMTPSettings_h_
#define _QCSMTPSettings_h_


class QCNetworkSettings;
#include "sslsettings.h"


//Read-only setting for performing mail-send
class QCSMTPSettings 
{
public:

	QCSMTPSettings(const char* persona);
	virtual ~QCSMTPSettings();

	int GrabSettings();
	CString& GetPOPAccount() { return m_szPOPAccount; }
	CString& GetSMTPServer() { return m_szSMTPServer; }
	QCNetworkSettings* GetNetSettings(){ return m_pNetSettings; }
	CString& GetBestGuessAtDomain() { return m_szBestGuessAtDomain; }

public:
	//also contains a netsettings object
	QCNetworkSettings* m_pNetSettings;
	int GetSmtpPort(){ return m_nSmtpPort; }
	int GetSmtpSubmissionPort(){ return m_nSmtpSubmissionPort; }
	bool UsePOPSend(){ return m_bUsePOPSend; }
	bool RequirePOPLoginToSend() { return m_bRequirePOPLoginToSend; }
	bool UseSubmissionPort() { return m_bUseSubmissionPort; }
	bool IgnoreQUITErrors() { return m_bIgnoreQUITErrors; }

	bool AuthAllowed(){ return m_bAuthAllowed; }
	bool AuthRequired(){ return m_bAuthRequired; }
	bool AuthK5FollowsINI() {return m_bAuthFollowsK5INI;}
	CString GetAuthBanished() { return m_szAuthBanished; }
	QCSSLSettings m_SSLSettings;

private:
	
	//for Network

	CString m_szPersona;
	CString m_szPOPAccount;
	CString m_szSMTPServer;
	CString m_szBestGuessAtDomain;
	
	//int nPopPort;
	//CString szPOPServiceName;

	int m_nSmtpPort;
	int m_nSmtpSubmissionPort;

	bool m_bUsePOPSend; 
	bool m_bRequirePOPLoginToSend;
	bool m_bUseSubmissionPort;
	bool m_bIgnoreQUITErrors;

	// ESMTP AUTH settings:
	bool m_bAuthAllowed;		// IDS_INI_SMTP_AUTH_ALLOWED
	bool m_bAuthRequired;		// IDS_INI_SMTP_AUTH_REQUIRED
	bool m_bAuthFollowsK5INI;	// IDS_INI_SMTP_AUTH_K5_INI
	CString m_szAuthBanished;	// IDS_INI_SMTP_AUTH_BANISHED

	// Method of 1st unread msg (Normal, Use Last, Use Msg status)
	// Method of fetching: Normal, LMOS, AllHeaders, Delete All
	// Skip Msgs > xxK
	// Authentication Type: password, Kerb, APOP
	// Connection Type: file I/O, Winsock

	
};


#endif

