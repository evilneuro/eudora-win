// QCWorkerSocket.h
// 
// Class definition for Network IO for Eudora
//

#ifndef _QCWORKERSOCKET_H_
#define _QCWORKERSOCKET_H_

#include "QCWinSockLib.h"
#include "QCHesiodLib.h"
//#include "QCRasConnection.h"


class QCNetworkSettings;


// Virtual Class Definition
class AFX_EXT_CLASS Network
{
public:
	Network(QCNetworkSettings *pSettings=NULL, LONG Size=-1);	
	virtual ~Network();

	BOOL IsOK() const;

	virtual int Open(const char* Machine, UINT ServiceID, UINT PortIniID, 
			UINT DefaultPort, int Quiet = FALSE, int FailOver = FALSE) = 0;
	
	virtual int Close() = 0;
	virtual int PutDirect(const char* buf = NULL, int length = -1) = 0;

	int GetLine(char* line, int len);
	int GetCh(char* c);
	virtual int Put(const char* bufptr = NULL, int length = -1);
	int PutLine(const char* bufptr = NULL, int length = -1)
		{ return (Put(bufptr, length) < 0? -1 : Put("\r\n", 2)); }
	int Put(char ch)
		{ return (Put(&ch, 1)); }
	int PutLine(char ch)
		{ return (PutLine(&ch, 1)); }

	// Resets the data structure.  To be used for writing only
	virtual void Reset() = 0;
	virtual void Flush() = 0;

	virtual void SetLinger(int time = 10) { (time); }

	virtual const char *GetMachineName() 
		{ ASSERT(FALSE); return NULL; }
	
	virtual const SOCKET GetSocket() 
		{ ASSERT(FALSE); return NULL; }

	BOOL IsFileBased() const
		{ return m_bIsFileBased; }

	virtual void RequestStop(){ m_bEscapePressed = TRUE; }

protected:

	QCNetworkSettings *m_Settings;

	int			m_nBufferSize;		// Size of our buffer
	char*		m_pBuffer; 			// Pointer to storage
	char*		m_pBufPtr;			// Pointer into current read position
	int			m_nValidBytes;		// Valid bytes in this page
	BOOL		m_bEscapePressed;
	BOOL		m_bIsFileBased;

	// The GetCh() function buffers characters and tries to log them a line at a time
	char		m_GetChBuffer[256];
	int			m_GetChValidBytes;
	void FlushLogGetChBuffer()
		{ if (m_GetChValidBytes) { PutDebugLog(DEBUG_MASK_RCV, m_GetChBuffer, m_GetChValidBytes); m_GetChValidBytes = 0; } }


	
	virtual int Read() = 0;
	virtual int Write() = 0;
	
	friend Network* CreateNetConnection(BOOL, BOOL);
	
};


// WinSock Class Definition

#define USER_WINSOCK_MESSAGE		WM_USER + 100	//1124
#define USER_GETSERV_MESSAGE		WM_USER + 101	//1125
#define USER_GETHOSTNAME_MESSAGE	WM_USER + 102	//1126
#define USER_CONNECT_MESSAGE		WM_USER + 103	//1127
#define USER_GETHOSTADDR_MESSAGE	WM_USER + 104	//1128

#define USER_THREAD_MESSAGE			WM_USER + 105	//1129
#define USER_TERMINATE_THREAD		WM_USER + 106	//1130

#define USER_CREATE_ASYNC_WINDOW    WM_USER + 107
#define USER_DESTROY_ASYNC_WINDOW   WM_USER + 108







LRESULT CALLBACK GlobalWindowProc(  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
UINT AsyncThreadFunc (LPVOID pObj);
//DWORD WINAPI AsyncThreadFunc (LPVOID pObj);
HANDLE GetWindowEventObject (HWND hWnd);
WORD GetWindowExpectMessage (HWND hWnd);
void RemoveSignalingData (HWND hWnd);


class AFX_EXT_CLASS QCWorkerSocket : public Network
{
public:
	friend UINT AsyncThreadFunc (LPVOID pObj);

	QCWorkerSocket(QCNetworkSettings *pSettings);	
	virtual ~QCWorkerSocket();

	BOOL BlockingHook();
	bool CreateSocketWindow();
	virtual int Open(const char* Machine, UINT ServiceID, UINT PortIniID, UINT DefaultPort, int Quiet = FALSE, int FailOver = FALSE);	// required override
	virtual int Close();												// required override

	virtual void Reset()	{ m_pBufPtr = m_pBuffer; m_nValidBytes = 0; }
	virtual void Flush()	{ Write(); }

	virtual void SetLinger(int time = 10);

	virtual int PutDirect(const char* buf = NULL, int length = -1);		// required override

	virtual const char *GetMachineName(){ return m_szMachine; }
	virtual const SOCKET GetSocket()	{ return m_Socket; }

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	int MessageWait(WORD Expect);
	void SignalWinsockEvent (UINT message);
	
	static void RemoveAsyncThread();
	virtual void RequestStop();

	bool Aborted(){ return m_bAbortConnection; }
	void SwitchToBlockingMode();
	void SwitchToAsyncMode();

protected:
	HWND m_hWnd;
	SOCKET m_Socket;
	//BOOL m_AsyncNonDatabase;	// Use async calls for non-databse functions
	//BOOL m_AsyncDatabase;		// Use async calls for databse functions
	//DWORD m_dwTimeout;
	DWORD m_dwTimeoutLength;
	bool m_bAbortConnection;
	
	bool m_bTimerExpired;
	char m_szMachine[128];
	bool m_bWinSockLoaded;
	bool m_bDialingInProgress;

	//Asynch imple
	bool CreateAsyncWindow();
	bool ReallyCreateThread();

	static HANDLE	m_hAsyncThread;
	static DWORD	m_dwAsyncThreadID;
	static CWinThread *m_pWinThread;
	static CCriticalSection m_Guard;
	HANDLE m_hEvent;
	int m_ErrorValue;
	int m_AsyncEvent[5];
	int m_Expect;

	

	//
	// FORNOW, since we assume only have one QCWorkerSocket object at a time,
	// we can embed the QCRasConnection object here.
	//
	//QCRasConnection m_rasConnection;

	void ShutdownNetwork();
	//void CancelCommand(){ m_dwTimeout = 0; m_AsyncEvent = 0; }

	
	//int GetMessage(BOOL bBlock = FALSE, WORD wExpect = 0);
	//int MessageWait(WORD Expect = 0){ return GetMessage(TRUE, Expect); }

	void ResetCommand(int EventType = 0);
	virtual int Read();													// required override
	virtual int Write();												// required override

	bool StartWinSock();
	void GetRealHostName(const char *machine, char *realname, int retsize);

	void ReportError(UINT StringID, ...);
	int SetError(const char* message, int ErrorNum);
	int ReadError(int ErrorNum);
	int WriteError(int ErrorNum);
	int SendData(const char* buf, int length);
	u_short GetPort(UINT ServiceID, UINT Port);
	u_long GetAddress(const char* Machine, int Quiet);

private:
	friend Network* CreateNetConnection(BOOL, BOOL);
		
};



// Function Prototypes
extern Network* CreateNetConnection(BOOL ShowProgress = TRUE, BOOL FileIO = FALSE );
Network* CreateNetConnectionMT(QCNetworkSettings *pSettings, 
							   BOOL ShowProgress = FALSE, BOOL FileIO = FALSE);

extern void InitSockCallbacks();

extern Network* NetConnection;


#endif




