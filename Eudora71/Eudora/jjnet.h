// JJNET.HPP
// 
// Class definition for Network IO for Eudora
//



//this file now refers to QCWorkerSocket.h

#include "QCWorkerSocket.h"
#include "fileutil.h"       //for JJfile
#include "QCFileIo.h"       //For CFileIO











#if 0



extern "C" {
#include "winsock.h"
}
   
#include "QCRasConnection.h"		//FORNOW
#include "fileutil.h"			// for JJFile


// Forward Declarations
class CCursor;

// Virtual Class Definition

class Network : public CWnd
{
	DECLARE_DYNAMIC(Network)

public:
	virtual ~Network();

	void NukeCursor( void );
	
	BOOL IsOK() const;

	virtual int Open(const char* Machine, UINT ServiceID, UINT PortIniID, UINT DefaultPort, int Quiet = FALSE,
		int FailOver = FALSE) = 0;
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

protected:
	int			m_nBufferSize;		// Size of our buffer
	char*		m_pBuffer; 			// Pointer to storage
	char*		m_pBufPtr;			// Pointer into current read position
	int			m_nValidBytes;		// Valid bytes in this page
	BOOL		m_bEscapePressed;
	CCursor*	m_pCursor;
	BOOL		m_bIsFileBased;
	
	virtual int Read() = 0;
	virtual int Write() = 0;
	
//FORNOW	virtual int GetMessage(BOOL Block = FALSE, WORD Expect = 0);
//FORNOW	int MessageWait(WORD Expect = 0)
//FORNOW		{ return (GetMessage(TRUE, Expect)); }
//FORNOW
//FORNOW	void ResetCommand()
//FORNOW		{ m_Timeout = GetTickCount() + (1000L * m_TimeoutLength); }

	friend Network* CreateNetConnection(BOOL, BOOL);
	Network(LONG Size = -1);
};


// WinSock Class Definition

#define USER_WINSOCK_MESSAGE		WM_USER + 100
#define USER_GETSERV_MESSAGE		WM_USER + 101
#define USER_GETHOSTNAME_MESSAGE	WM_USER + 102
#define USER_CONNECT_MESSAGE		WM_USER + 103
#define USER_GETHOSTADDR_MESSAGE	WM_USER + 104
//#define USER_READ_MESSAGE			WM_USER + 105
//#define USER_WRITE_MESSAGE			WM_USER + 106
//#define USER_CLOSE_MESSAGE			WM_USER + 107

class WinSock : public Network
{
	DECLARE_DYNAMIC(WinSock)

public:
	virtual ~WinSock();

	BOOL BlockingHook();
	virtual int Open(const char* Machine, UINT ServiceID, UINT PortIniID, UINT DefaultPort, int Quiet = FALSE, int FailOver = FALSE);	// required override
	virtual int Close();												// required override

	virtual void Reset()												// required override
		{ m_pBufPtr = m_pBuffer; m_nValidBytes = 0; }
	virtual void Flush()												// required override
		{ Write(); }

	virtual void SetLinger(int time = 10);

	virtual int PutDirect(const char* buf = NULL, int length = -1);		// required override

	virtual const char *GetMachineName()
		{ return m_szMachine; }
	virtual const SOCKET GetSocket() 
		{ return m_Socket; }

protected:
	SOCKET m_Socket;
	int m_ErrorValue;
	BOOL m_AsyncNonDatabase;	// Use async calls for non-databse functions
	BOOL m_AsyncDatabase;		// Use async calls for databse functions
	UINT m_AsyncEvent;
	BOOL m_bAbortConnection;
	DWORD m_dwTimeout;
	DWORD m_dwTimeoutLength;
	BOOL m_bTimerExpired;
	char m_szMachine[128];
	BOOL m_bWinSockLoaded;
	BOOL m_bGotRasConnection;

	//
	// FORNOW, since we assume only have one WinSock object at a time,
	// we can embed the QCRasConnection object here.
	//
	QCRasConnection m_rasConnection;

	void ShutdownNetwork();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
//FORNOW	virtual int GetMessage(BOOL Block = FALSE, WORD Exepct = 0);
	int GetMessage(BOOL bBlock = FALSE, WORD wExpect = 0);
	void CancelCommand()
		{ m_dwTimeout = 0; m_AsyncEvent = 0; }

	int MessageWait(WORD Expect = 0)
		{ return GetMessage(TRUE, Expect); }

	void ResetCommand()
		{ m_dwTimeout = GetTickCount() + (1000L * m_dwTimeoutLength); }

	virtual int Read();													// required override
	virtual int Write();												// required override

	int StartWinSock();
	void GetRealHostName(const char *machine, char *realname, int retsize);

	int ReportError(const char* message, int ErrorNum);
	int ReadError(int ErrorNum);
	int WriteError(int ErrorNum);
	int SendData(const char* buf, int length);
	u_short GetPort(UINT ServiceID, UINT Port);
	u_long GetAddress(const char* Machine, int Quiet);

private:
	friend Network* CreateNetConnection(BOOL, BOOL);
	WinSock(LONG Size = -1);
};



// -- File base connection --
class CFileIO : public Network
{
	DECLARE_DYNAMIC(CFileIO)

public:
	virtual ~CFileIO();

	virtual int Open(const char* Machine, UINT ServiceID, UINT PortIniID, UINT DefaultPort, int Quiet = FALSE, int FailOver = FALSE);	// required override

	//
	// FORNOW, it seems to be bad style to override a pure virtual
	// like this with the same name as the pure virtual.  Probably
	// better to use a different name here.
	//
	int OpenFile(const char* FileName, int mode);	
	
	virtual int Close();												// required override

	virtual void Reset();												// required override
	virtual void Flush();												// required override

	virtual int PutDirect(const char* buf = NULL, int length = -1);		// required override
	virtual int Put(const char* bufptr = NULL, int length = -1);


protected:

	virtual int Read();													// required override
	virtual int Write();												// required override
	JJFile m_jjFile;

private:
	friend Network* CreateNetConnection(BOOL, BOOL);
	CFileIO(int Size = -1);
};


// Function Prototypes

Network* CreateNetConnection(BOOL ShowProgress = TRUE, BOOL FileIO = FALSE );

extern Network* NetConnection;
#endif