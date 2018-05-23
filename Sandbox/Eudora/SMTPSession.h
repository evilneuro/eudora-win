#ifndef _QCSMTPThreadMT_h_
#define _QCSMTPThreadMT_h_

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
	enum { IGNORE_ERRORS = true };
public:
	
	QCSMTPThreadMT(const char *szPersona, QCSMTPSettings* pMailSettings);
	virtual ~QCSMTPThreadMT();

	HRESULT DoAllMessages();
	HRESULT DoOneMessage(COutSpoolInfo *pSpoolInfo);
	HRESULT StartSMTP(const char* SMTPServer);
	HRESULT DoSMTPIntro();
	HRESULT EndSMTP(BOOL abort);

	//POP Send  -- must get execetued in the main thread
	BOOL PreProcessPOPSend();

	virtual void RequestThreadStop();

	void ErrorDialog(const char *error_buf);

	// SASL Callbacks.
	int	  SaslChallenge (char *outbuf, int outsize);
	BOOL  SaslResponse  (char *response, unsigned long size);

	LPCSTR GetPassword () { return m_szPassword; }

private:

	HRESULT SMTPCmdGetReply(const char* cmd, const char* arg, char* buf, int size, int& status,
			bool ShowProgress = false, bool IgnoreError = false);
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
