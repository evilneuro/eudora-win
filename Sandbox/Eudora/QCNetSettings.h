#ifndef _QCNetSettings_h_
#define _QCNetSettings_h_

#define QCMS_BUF_SIZE 256
#define QC_NETWORK_BUF_SIZE 3072

#include "Callbacks.h"
#include <ras.h>
#include "TaskErrorTypes.h"

typedef void (* ReportErrorFnType)(void *, UINT StringID, ...);

class QCNetworkSettings {
public:
	QCNetworkSettings(const CString& persona) : m_strPersona(persona), ClosedByPeer(0), m_pRasDialParams(NULL)
					{ GrabSettings(); }
	
	~QCNetworkSettings();
	bool DoCaching(){ return m_bDoCaching; }
	bool DoConnectOffline() { return m_bConnectOffline;}
	bool DoAuthenticateKerberos(){ return m_bAuthenticateKerberos; }
	bool IsDialupConnection() { return m_bDialupConnection;}

	const CString GetPersona(){ return m_strPersona; }
	const CString GetRASEntryName(){ return m_strRASEntryName; }
	RASDIALPARAMS *GetRasDialParams(){ return m_pRasDialParams; }
	bool GetSavePasswordFlag(){ return m_bRASSavePassword; }
	bool GetRasPassword();
	
	int GetNetworkBufferSize()  { return m_nNetworkBufferSize; }
	int GetNetworkOpenTimeout() { return m_nNetworkOpenTimeout; }
	int GetNetworkTimeout() { return m_nNetworkTimeout; }
	bool GetNetImmediateClose(){ return m_bNetImmediateClose; }
	
	
	typedef Callback2<const char *, TaskErrorType> ErrorCallback;
	typedef Callback1<const char *> ProgressCallback;

	void SetReportErrorCallback(ErrorCallback func){ ReportError = func; }
	void SetProgressCallback(ProgressCallback func){ Progress = func; }
	void SetCloseCallback(Callback1<int> func){ ClosedByPeer = func; }
	

private:
	int GrabSettings();

	CString m_strPersona;
	bool m_bDoCaching;

	//RAS
	bool m_bDialupConnection;
	RASDIALPARAMS *m_pRasDialParams;
	CString m_strRASEntryName;
	
	bool m_bRASSavePassword;

	//Network
	bool m_bConnectOffline;
	int m_nNetworkBufferSize;
	int m_nNetworkOpenTimeout;
	int m_nNetworkTimeout;

	bool m_bNetImmediateClose;		//IDS_INI_NET_IMMEDIATE_CLOSE
	
	//Authentication
	bool m_bAuthenticateKerberos;
	

public:
	//To report errors
	//ReportErrorFnType ReportError;
	Callback2<const char *, TaskErrorType> ReportError;
	
	//To report progress 
	Callback1<const char *> Progress;

	//To Report Closes by the server
	Callback1<int> ClosedByPeer;
		
};


#endif