// SMTPSession.h
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

#ifndef _QCSMTPThreadMT_h_
#define _QCSMTPThreadMT_h_

#include "QCWorkerThreadMT.h"

//forward def
class Network;
class CPOPSession;
class QCSMTPSettings;
class COutSpoolInfo;
class QCSpoolMgrMT;
class CEhlo;
class CSasl;

class QCSMTPThreadMT : public QCWorkerThreadMT
{
public:
	
	QCSMTPThreadMT(const char *szPersona, const char *szSpoolPersona, QCSMTPSettings* pMailSettings);
	virtual ~QCSMTPThreadMT();

	HRESULT DoAllMessages();
	HRESULT DoOneMessage(COutSpoolInfo *pSpoolInfo);
	HRESULT StartSMTP(const char* SMTPServer);
	HRESULT DoSMTPIntro();
	HRESULT EndSMTP(BOOL abort);
	const CString & GetSpoolPersona() const { return m_strSpoolPersona; }

	//POP Send  -- must get execetued in the main thread
	BOOL PreProcessPOPSend();

	virtual void RequestThreadStop();

	void ErrorDialog(const char *error_buf);

	// SASL Callbacks.
	int	  SaslChallenge (char *outbuf, int outsize);
	BOOL  SaslResponse  (char *response, unsigned long size);

	LPCSTR GetPassword () { return m_szPassword; }

	QCSMTPSettings *GetSettings() { return m_Settings; }

	CString GetGSSLibraryName();

private:

	HRESULT SMTPCmdGetReply(const char* cmd, const char* arg, char* buf, int size, int& status,
			bool ShowProgress = true, bool IgnoreError = false);
	int IsCmd(const char* buf);
	HRESULT GetSMTPReply(char* buf, int size, int& status);
	HRESULT ReadLine(JJFileMT* file, char* buf, int bSize, 
								  long& lNumBytesRead );
	void SMTPError(const char* Command, const char* Message);
	void ErrorDialog(UINT StringID, ...);

	HRESULT LogError(JJFileMT* file,UINT StringID, ...);

	// Process a 250 response from an EHLO command.
	//
	void EhloLine(LPCTSTR pBuf);
	// Get user's password in f/g. Used in CRAM-MD5 authentication.
	BOOL GetLogin ();
	// Do the authentication in the b/g.
	HRESULT DoSMTPAuth (int& status);
	
private:	
	CString m_strPersona;
	CString m_strSpoolPersona;
	bool m_bUsingPOPSend;

	QCSpoolMgrMT* m_SpoolMgr;
	QCSMTPSettings* m_Settings;
	Network *m_pNetConnection;
	CCriticalSection m_Guard;
	
	int m_nMsgsToProcess;
	long m_nTotalSize;
	bool m_bTaskStarted;
	int m_dwFlags;
	CPOPSession *m_POPSession;

	// Optimization flag to indicate if the current command can be an EHLO command.
	//
	BOOL m_bEhlo;
	void InEhlo (BOOL bVal)  { m_bEhlo = bVal; }
	BOOL IsEhlo ()  { return m_bEhlo; }

	// Class to hold capability info from the EHLO command for this session.
	//
	CEhlo*	m_pEhlo;

	// Sasl object for this session.
	//
	CSasl*	m_pSasl;

	// Authenticate as this user.
	//
	CString m_szUser;
	CString m_szPassword;
		
public:

	virtual HRESULT DoWork();
	const CString& GetPersona()		{ return m_strPersona;}

};

#endif
