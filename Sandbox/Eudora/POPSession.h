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


class POPVALUES {
public:
	BOOL bGotNewMail;
	QCPOPSettings *settings;
};




void ReportError(void *pvCaller, UINT StringId, ...);

class CPOPRecoverer : public QCWorkerThreadMT
{
	char *m_szPersona;
public:
	CPOPRecoverer(CString strPersona);
	HRESULT DoWork();
	void RequestThreadStop(){}
	virtual ~CPOPRecoverer(){}
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
	bool NeedsCleanup(){ return m_bCleanup; }
	void Cleanup();

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


	//Methods
	int ReadPOPLine(char* pszBuffer, int nBufferSize);

	void GetCapabilities();

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

	BOOL OpenPOPConnection_();
	BOOL DoLogon_();
	int POPAuthenticate_();
	KerberosErrType KerberosAuthenticate_();
	int RPAAuthenticate_();

	BOOL DoReconcileUIDLInfo_();
	CPopHost* GetHostFromUIDLServer_();
	BOOL ReconcileLists_(CPopHost* pOldList, CPopHost* pNewList);

	int POPMsgHasBeenRead_(int bMsgNum);

	
	FetchType GetFetchType_() const { return m_Settings->GetFetchType(); }
	BOOL DoGetPOPMail_();  				// Normal
	int FindFirstUnread_();
	
	//TO SPOOL
	MsgFetchType GetMsgFetchType(CMsgRecord *pMsgRecord);

	BOOL DoGetPOPMailToSpool();
	unsigned long ComputeMessageIdAndHash(CString& strFileName, char *szMesgIdDate);
	int POPGetMessageToSpool(CMsgRecord *pMsgRecord);
	int FetchMessageToSpool(int nMessageNum, long estSize);
		
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

	// Capabilities
	bool m_bCanMangle;
	
	//Added for Thread-safety. SK
	QCPOPSettings *m_Settings;
	QCPOPNotifier *m_Notifier;
	CString m_strPersona;
	unsigned int m_Notifications;
	CPopHost *m_pPopHost;

	//Authentication safety
	static CCriticalSection m_RPAAuthGuard;
	static CCriticalSection m_KerberosAuthGuard;

	//Generic object safety
	CCriticalSection m_Guard;
	
	//POSITION m_LastMessagePos;
	//BOOL m_bNotifyNewMail;				// TRUE if new messages have been downloaded
	//int m_nDownloadCount;
	//int	m_nTrashedMessageCount;			// number of messages trashed during filtering
	
};





//	===============================================================================================                                                         
class CPOP
{

public:
	CPOP(POPVALUES *pv, QCPOPNotifier *notes);
	CPOP(CString strPersona);  //called when recovering from a crash as we dont have other pertinant info
	virtual ~CPOP(){}

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






