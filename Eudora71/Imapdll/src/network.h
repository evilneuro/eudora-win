// network.h - Underlying network communication reoutines.
// Contains (by value) a "QCWorkerSocket" object (see the QCSocket DLL in eudora/eudora).
//

#ifndef _NETWORK_H_
#define _NETWORK_H_


// Utility class:
// Data Types



// Forward declarations:
//
class QCWorkerSocket;
class QCNetworkSettings;
struct SSLSettings;

// 
class  CNetStream
{
public:
	CNetStream();
	~CNetStream();

// Operations:
public:
	// This must be overridden by child.
	virtual void NetNotify (NOTIFY_TYPE notify_type, void *data) = 0;

	// Must be overridden.
	virtual void NetReportError (LPCSTR pStr, UINT Type) = 0;

	// Should be overridden:
	//
	virtual void NetLog	(char *string,long errflg) = 0;

	// 
	BOOL	NetOpen (const char* Machine, const char* Service, UINT ServiceID, int Port, const char* loginname);
	BOOL	NetClose();

	void	RequestStop();

	// c-client required.
	char*	Getline();
	BOOL	Getbuffer (unsigned long size, char *buffer);

	BOOL	NetSout(const char* buf, unsigned long length);
	BOOL	NetSoutr (char *string);

	BOOL	Abort ();

	void	GetCanonicalHostname (CString& szHostName);

// Error management.

	void	CloseCallback (int ErrorValue);

	void	SocketErrorCallback (LPCSTR str, UINT ErrorType);

	// 
	BOOL	UserAborted ();


	// Internal access.
	//
	const char		*GetMachineName  ()
						{ return (LPCSTR)m_szHost; }

	unsigned long	GetPortNum ()
						{ return m_Port; }

	// Get port number as a string.
	//
	LPCSTR			GetPortStr ()
						{ return m_szPort; }

	void SetNetSettings (QCNetworkSettings* pNetSettings);

	QCNetworkSettings* GetNetSettings ()
		{ return m_pNetSettings; }


	// Public data - should be moved to private.
public:

	// Server particulars.
	//
	CString			m_szHost;			// host name 
	CString			m_szPort;			// As a string.

	// This is the contained QCSocket object that does the real work.
	//
	QCWorkerSocket*			m_pNetConnection;

    bool SetSSLMode(bool bVal, const CString& persona, SSLSettings* pSSLSettings, const CString& protocol );

// Internal attributes.
private:

	unsigned long	m_Port;				// As a number.

	// Contained by reference ONLY!!
	//
	QCNetworkSettings*	m_pNetSettings;
};

#endif	// _NETWORK_H_


