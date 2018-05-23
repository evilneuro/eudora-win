// POPSession.h
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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


#ifndef _POPSESSION_H_
#define _POPSESSION_H_

#include "pop.h" 
#include "MsgRecord.h"
#include "pophost.h"
#include "hostlist.h"
#include "QCNetSettings.h"
#include "QCPOPSettings.h"
#include "QCWorkerThreadMT.h"
#include <stdarg.h>

//Forward declarations
class CTocDoc;
class HeaderDesc;
class CSpoolInfo;
class LineReader;
class Network;
class QCPOPSettings;
class CSasl;

class POPVALUES {
public:
	BOOL bGotNewMail;
	QCPOPSettings *settings;
};




void ReportError(void *pvCaller, UINT StringId, ...);

class CPOPRecoverer : public QCWorkerThreadMT
{
	CString m_Persona;
public:
	CPOPRecoverer(const char* strPersona);
	HRESULT DoWork();
	void RequestThreadStop(){}
};


//	===============================================================================================                                                         
class CPOPSession : public QCWorkerThreadMT
{
	friend void ReportError(void *pvCaller, UINT StringId, ...);
public:
	CPOPSession(QCPOPSettings *set, QCPOPNotifier *notifier=NULL);
	virtual ~CPOPSession();

	void RequestThreadStop();
	//
	// This is the starting point for the check mail sequence...
	//
	void GetMail();
	HRESULT DoWork();
	BOOL DoGetPassword_();

	// ESMTP AUTH uses POP's login and password, so we need to export these:
	LPCSTR ExportLogin ()	{return m_Settings ? m_Settings->GetPOPAccount() : NULL; }
	LPCSTR ExportPassword()	{ return m_strPassword; }

	int POPCmdGetReply(UINT uCmdID, const char* pszArgs, char* pszBuffer, int nBufferSize, BOOL bReportError = TRUE);
	
	unsigned int GetNotificationFlags(){ return m_Notifications; }
	POPVALUES *GetSettings();
	CString GetGSSLibraryName();
	bool NeedsCleanup(){ return m_bCleanup; }

	enum KerberosErrType
	{
		KERBERR_OK					= 0,		// status okay (no error)
		KERBERR_INTERNAL			= -9999,	// Internal error
		KERBERR_NO_NAME				= -9998,	// No pop user name found with @ sign
		KERBERR_REG_FAILED			= -900,		// Kerb Name registration failed
		KERBERR_PUT_FAILED			= -901,		// Sending kerb ticket to server failed
		KERBERR_GET_TICKET_FAILED	= -902,		// Getting a kerb ticket failed 
		KERBERR_LOAD_LIB_FAILED		= -903,		// Getting a kerb ticket failed 
		KERBERR_CANCELED			= -904		// Getting a kerb ticket failed 
	};

	enum ReDownloadStatusT
	{
		kNoReDownload							= 0,
		kReDownloadForPluginFlag				= 0x0001,
		kReDownloadKeepingSpoolFileOpenFlag		= 0x0002
	};

	enum SpoolErrorActionT
	{
		kReDownloadKeepingSpoolFileOpen			= 0,
		kFail									= 1,
		kIgnoreFailureAndDeleteMessage			= 2
	};


	//Methods
	int ReadPOPLine(char* pszBuffer, int nBufferSize);

	void GetCapabilities();
	void ForgetCapabilities();

	unsigned long MakeFakeMessageIdAndHash_(int nMsgNum, char* pszMesgIdDate);
	void ExtractDateTime_(const char* pszBuffer, char* pszDate);
	void ExtractMessageID_(const char* pszBuffer, char* pszMsgId);

	CPopHost* BuildNonUIDLMsgs_(int nStart, int nFinish);
	BOOL MatchLastMessage_(int nLast);

	int GetPOPPassword_();
	AuthenticationType GetAuthenticationType_() const 
			{ return m_Settings->GetAuthenticationType(); }

	BOOL SetupNetConnection_();
	BOOL TeardownNetConnection_();

	int	  SaslChallenge(char *outbuf, int outsize);
	BOOL  SaslResponse(char *response, unsigned long size);
	
	BOOL OpenPOPConnection_();
	BOOL DoLogon_();
	int POPAuthenticate_();
	KerberosErrType KerberosAuthenticate_();
	int RPAAuthenticate_();

	BOOL DoReconcileUIDLInfo_();
	BOOL GetHostFromUIDLServer_(CPopHost*& pServerHost);
	BOOL ReconcileLists_(CPopHost* pOldList, CPopHost* pNewList);

	int POPMsgHasBeenRead_(int bMsgNum);

	
	FetchType GetFetchType_() const { return m_Settings->GetFetchType(); }
	BOOL DoGetPOPMail_();  				// Normal
	int FindFirstUnread_();
	
	//TO SPOOL
	MsgFetchType GetMsgFetchType(CMsgRecord *pMsgRecord);

	BOOL DoGetPOPMailToSpool();
	unsigned long ComputeMessageIdAndHash(const char* strFileName, char *szMesgIdDate);
	int POPGetMessageToSpool(CMsgRecord *pMsgRecord);
	int FetchMessageToSpool(CMsgRecord *pMsgRecord, long estSize, bool bEntireMessage, bool bKeepSpoolFileOpen, int& reDownloadStatus);
		
	BOOL PurgeMail_();
	BOOL ClosePOPConnection_();
	void RemoveDeletedMsgRecords();

	void ErrorDialog(UINT StringID, ...);
	void ErrorDialog(UINT StringID, va_list argList);
	void AuthError(const char *);

	Network *GetNetConnection(){ return m_pNetConnection; }
	QCPOPNotifier *GetNotifier(){ return m_Notifier; }
	void Notify();
	bool IsQuitSuccessful(){ return m_bQuitIssuedSuccessfully; }
	bool SSLStart();
	time_t LastAuthTime() { return m_lastAuthTime; }
	
private:
	CString m_strPassword;
	unsigned long m_ulBitFlags;
	BOOL m_bCreatedNetConnection;		// TRUE if we created connection in SetupNetConnection()
	BOOL m_bPOPConnected;				// TRUE after we establish a POP connection in StartPOP()
	Network *m_pNetConnection;
	
	int m_nMessageCount;				// number of messages available on server
	BOOL m_bGotNewMail;
	bool m_bCleanup;
	bool m_bQuitIssuedSuccessfully;
	bool m_bTaskStarted;
	time_t m_lastAuthTime;				// last time we authed successfully
	time_t m_GMTOffset;				// can't do this in a thread, have to stash...
	long m_nLMOSBackupCount;

	// Capabilities
	bool m_bCanMangle;
	bool m_bSupportsAuthRespCode;
	
	//Added for Thread-safety. SK
	QCPOPSettings *m_Settings;
	QCPOPNotifier *m_Notifier;
	CString m_strPersona;
	unsigned int m_Notifications;
	CPopHost *m_pPopHost;
	CString m_strGSSLibName;

	//Authentication safety
	static CCriticalSection m_RPAAuthGuard;
	static CCriticalSection m_KerberosAuthGuard;

	//Generic object safety
	CCriticalSection m_Guard;
	
	//POSITION m_LastMessagePos;
	//BOOL m_bNotifyNewMail;				// TRUE if new messages have been downloaded
	//int m_nDownloadCount;
	//int	m_nTrashedMessageCount;			// number of messages trashed during filtering
	bool m_bSupportsSSL;
	
	CSasl*	m_pSasl;
};





//	===============================================================================================                                                         
class CPOP
{
public:
	CPOP(POPVALUES *pv, QCPOPNotifier *notes);
	CPOP(const char* strPersona);  //called when recovering from a crash as we dont have other pertinant info

	//
	// This is the starting point for the check mail sequence...
	//
	void GetMail(unsigned long ulBitFlags);

	//int POPCmdGetReply(UINT uCmdID, const char* pszArgs, char* pszBuffer, int nBufferSize, BOOL bReportError = TRUE) const;
	//static int ReadPOPLine(char* pszBuffer, int nBufferSize);
	//int GetNotificationFlags(){ return m_Notifier->m_Notifications; }
	
	bool GetMailFromSpool(bool bCrashRecovery=false);
	BOOL DoNotifyNewMail();


	
private:

	
//NEW class from here

	//FROM SPOOL
	bool DoGetPOPMailFromSpool(bool bCrashRecovery=false);
	int FetchMessageTextFromSpool(CTocDoc* pInTOC, CSpoolInfo *pSpoolInfo);
	int WriteMessageToMBX_(HeaderDesc& hd, LineReader *pLineReader);
	
	
	void DoProcessJunkMail_();
	BOOL DoFilterMail_();
	//BOOL DoNotifyNewMail_(BOOL bOpenInbox);
	

	//Attributes

private:

	unsigned long m_ulBitFlags;
	//BOOL m_bCreatedNetConnection;		// TRUE if we created connection in SetupNetConnection()
	//BOOL m_bPOPConnected;				// TRUE after we establish a POP connection in StartPOP()
	//int m_nMessageCount;				// number of messages available on server
	
	POSITION m_LastMessagePos;
	
	BOOL m_bGotNewMail;
	
	int	m_nTrashedMessageCount;			// number of messages trashed during filtering
	int	m_nJunkedMessageCount;			// number of messages junked during junk processing
	BOOL m_bNotifyNewMail;				// TRUE if new messages have been downloaded
	int m_nDownloadCount;
	
	//Added for Thread-safety. SK
	bool m_bEscapePressed;
	bool m_bOpenInMailbox;
	CString m_strPersona;
	//unsigned int m_Notifications;

	QCPOPSettings *m_Settings;
	QCPOPNotifier *m_Notifier;
};

//
// Helper macros.
//
#define RETURN_IF_NULL(__expr_to_check__) if (!__expr_to_check__) { ASSERT(0); return; }
#define RETURN_NULL_IF_NULL(__expr_to_check__) if (!__expr_to_check__) { ASSERT(0); return NULL; }

//Support functions
void NotifyMailCheckResults(int NotifyFlags);

#endif






