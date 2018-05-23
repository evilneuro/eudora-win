#ifndef _QCSMTPSettings_h_
#define _QCSMTPSettings_h_


class QCNetworkSettings;


//Read-only setting for performing mail-send
class QCSMTPSettings 
{
public:

	QCSMTPSettings(const CString& persona);
	~QCSMTPSettings();

	int GrabSettings();
	CString GetPOPAccount() { return m_szPOPAccount; }
	CString GetSMTPServer() { return m_szSMTPServer; }
	QCNetworkSettings *GetNetSettings(){ return m_pNetSettings; }

public:
	//also contains a netsettings object
	QCNetworkSettings *m_pNetSettings;
	int GetSmtpPort(){ return m_nSmtpPort; }
	bool UsePOPSend(){ return m_bUsePOPSend; }
	bool RequirePOPLoginToSend() { return m_bRequirePOPLoginToSend; }
private:
	
	//for Network

	CString m_szPersona;
	CString m_szPOPAccount;
	CString m_szSMTPServer;
	int nPopPort;
	CString szPOPServiceName;
	int m_nSmtpPort;

	bool m_bUsePOPSend; 
	bool m_bRequirePOPLoginToSend;
	
	
	// Method of 1st unread msg (Normal, Use Last, Use Msg status)
	// Method of fetching: Normal, LMOS, AllHeaders, Delete All
	// Skip Msgs > xxK
	// Authentication Type: password, Kerb, APOP
	// Connection Type: file I/O, Winsock
	
	
};


#endif

