// QCWinSockLib.h
// 
// Manages dynamic loading of WinSock Library DLL.
//

#ifndef __QCWINSOCKLIB_H__
#define __QCWINSOCKLIB_H__

#include <winsock.h>
   

#define QCSOCK_SUCCESS 0
#define QCSOCK_FAIL -1


//
// Typedefs for WinSock API functions.
//
typedef int (CALLBACK WS_CLOSESOCKET)(SOCKET s);
typedef int (CALLBACK WS_CONNECT)(SOCKET s, const struct sockaddr FAR *name, int namelen);
typedef struct hostent FAR * (CALLBACK WS_GETHOSTBYADDR)(const char FAR * addr, int len, int type);
typedef struct hostent FAR * (CALLBACK WS_GETHOSTBYNAME)(const char FAR * name);
typedef int (CALLBACK WS_GETHOSTNAME)(char FAR * name, int namelen);
typedef struct servent FAR * (CALLBACK WS_GETSERVBYNAME)(const char FAR * name, const char FAR * proto);
typedef u_short (CALLBACK WS_HTONS)(u_short hostshort);
typedef unsigned long (CALLBACK WS_INET_ADDR)(const char FAR * cp);
typedef char FAR * (CALLBACK WS_INET_NTOA)(struct in_addr in);
typedef int (CALLBACK WS_IOCTLSOCKET)(SOCKET s, long cmd, u_long FAR *argp);
typedef int (CALLBACK WS_RECV)(SOCKET s, char FAR * buf, int len, int flags);
typedef int (CALLBACK WS_SEND)(SOCKET s, const char FAR * buf, int len, int flags);
typedef int (CALLBACK WS_SETSOCKOPT)(SOCKET s, int level, int optname, const char FAR * optval, int optlen);
typedef int (CALLBACK WS_GETSOCKOPT)(SOCKET s, int level, int optname, const char FAR * optval, int FAR *optlen);
typedef int (CALLBACK WS_SHUTDOWN)(SOCKET s, int how);
typedef SOCKET (CALLBACK WS_SOCKET)(int af, int type, int protocol);
typedef int (CALLBACK WS_SELECT)(int nfds,fd_set FAR * readfds,fd_set FAR * writefds,fd_set FAR * exceptfds,const struct timeval FAR * timeout);
typedef HANDLE (CALLBACK WS_WSAASYNCGETHOSTBYADDR)(HWND hWnd, UINT wMsg, const char FAR * addr, int len, int type, char FAR *buf, int buflen);
typedef HANDLE (CALLBACK WS_WSAASYNCGETHOSTBYNAME)(HWND hWnd, UINT wMsg, const char FAR * name, char FAR *buf, int buflen);
typedef HANDLE (CALLBACK WS_WSAASYNCGETSERVBYNAME)(HWND hWnd, UINT wMsg, const char FAR * name, const char FAR * proto, char FAR * buf, int buflen);
typedef int (CALLBACK WS_WSAASYNCSELECT)(SOCKET s, HWND hWnd, UINT wMsg, long LEvent);
typedef int (CALLBACK WS_WSACANCELASYNCREQUEST)(HANDLE handle);
typedef int (CALLBACK WS_WSACANCELBLOCKINGCALL)(void);
typedef int (CALLBACK WS_WSACLEANUP)(void);
typedef int (CALLBACK WS_WSAGETLASTERROR)(void);
typedef FARPROC (CALLBACK WS_WSASETBLOCKINGHOOK)(FARPROC lpBlockFunc);
typedef void (CALLBACK WS_WSASETLASTERROR)(int iError);
typedef int (CALLBACK WS_WSASTARTUP)(WORD wVersionRequired, LPWSADATA lpWSAData);


class AFX_EXT_CLASS QCWinSockLibMT
{
public:
	static int LoadWSLibrary();
	static int FreeWSLibrary();
	static bool IsLibraryLoaded();

	static bool Cleanup(bool bCleanupAfterClose);
	static bool Init();
	
	static int closesocket(SOCKET s);
	static int connect(SOCKET s, const struct sockaddr FAR *name, int namelen);	
	static struct hostent FAR * gethostbyaddr(const char FAR * addr, int len, int type);
	static struct hostent FAR * gethostbyname(const char FAR * name);	
	static int gethostname(char FAR * name, int namelen);	
	static struct servent FAR * getservbyname(const char FAR * name, const char FAR * proto);	
	static u_short htons(u_short hostshort);	
	static unsigned long inet_addr(const char FAR * cp);	
	static char FAR * inet_ntoa(struct in_addr in);
	static int ioctlsocket(SOCKET s, long cmd, u_long FAR *argp);	
	static int recv(SOCKET s, char FAR * buf, int len, int flags);
	static int send(SOCKET s, const char FAR * buf, int len, int flags);	
	static int setsockopt(SOCKET s, int level, int optname, const char FAR * optval, int optlen);
	static int getsockopt(SOCKET s, int level, int optname, const char FAR * optval, int FAR *optlen);	
	static int shutdown(SOCKET s, int how);	
	static SOCKET socket(int af, int type, int protocol);
	static int select (int nfds,fd_set FAR * readfds,fd_set FAR * writefds,fd_set FAR * exceptfds,const struct timeval FAR * timeout);
	static HANDLE WSAAsyncGetHostByAddr(HWND hWnd, UINT wMsg, const char FAR * addr, int len, int type, char FAR *buf, int buflen);
	static HANDLE WSAAsyncGetHostByName(HWND hWnd, UINT wMsg, const char FAR * name, char FAR *buf, int buflen);		
	static HANDLE WSAAsyncGetServByName(HWND hWnd, UINT wMsg, const char FAR * name, const char FAR * proto, char FAR * buf, int buflen);
	static int WSAAsyncSelect(SOCKET s, HWND hWnd, UINT wMsg, long LEvent);
	static int WSACancelAsyncRequest(HANDLE handle);
	static int WSACancelBlockingCall(void);
	static int WSAGetLastError(void);
	static FARPROC WSASetBlockingHook(FARPROC lpBlockFunc);	
	static void WSASetLastError(int iError);
	
	static int WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);
	static int WSACleanup(void);

	
private:
	QCWinSockLibMT();
	~QCWinSockLibMT();

	QCWinSockLibMT(const QCWinSockLibMT&);
	void operator=(const QCWinSockLibMT&);

	//
	// Function pointer declarations.
	//
	static WS_CLOSESOCKET* m_pfnclosesocket;
	static WS_CONNECT* m_pfnconnect;
	static WS_GETHOSTBYADDR* m_pfngethostbyaddr;
	static WS_GETHOSTBYNAME* m_pfngethostbyname;
	static WS_GETHOSTNAME* m_pfngethostname;
	static WS_GETSERVBYNAME* m_pfngetservbyname;
	static WS_HTONS* m_pfnhtons;
	static WS_INET_ADDR* m_pfninet_addr;
	static WS_INET_NTOA* m_pfninet_ntoa;
	static WS_IOCTLSOCKET* m_pfnioctlsocket;
	static WS_RECV* m_pfnrecv;
	static WS_SEND* m_pfnsend;
	static WS_SETSOCKOPT* m_pfnsetsockopt;
	static WS_GETSOCKOPT* m_pfngetsockopt;
	static WS_SHUTDOWN* m_pfnshutdown;
	static WS_SOCKET* m_pfnsocket;
	static WS_SELECT* m_pfnselect;
	static WS_WSAASYNCGETHOSTBYADDR* m_pfnWSAAsyncGetHostByAddr;
	static WS_WSAASYNCGETHOSTBYNAME* m_pfnWSAAsyncGetHostByName;
	static WS_WSAASYNCGETSERVBYNAME* m_pfnWSAAsyncGetServByName;
	static WS_WSAASYNCSELECT* m_pfnWSAAsyncSelect;
	static WS_WSACANCELASYNCREQUEST* m_pfnWSACancelAsyncRequest;
	static WS_WSACANCELBLOCKINGCALL* m_pfnWSACancelBlockingCall;
	static WS_WSAGETLASTERROR* m_pfnWSAGetLastError;
	static WS_WSASETBLOCKINGHOOK* m_pfnWSASetBlockingHook;
	static WS_WSASETLASTERROR* m_pfnWSASetLastError;
	static WS_WSASTARTUP* m_pfnWSAStartup;
	static WS_WSACLEANUP* m_pfnWSACleanup;
	

	//
	// Reference count for outstanding LoadWSLibrary() calls.
	//
	static int m_nRefCount;
	static bool m_bInitialized;
	
	static CCriticalSection m_Guard;

	static bool m_bLoaded;

	//
	// HINSTANCE for dynamically-loaded DLL.
	//
	static HINSTANCE m_hWinSockLib;

};


class AFX_EXT_CLASS QCWinSockLoader
{
public:
	QCWinSockLoader(bool bCleanup = false);
	~QCWinSockLoader();
private:
	bool m_bCleanup;
};

#endif // __QCWINSOCKLIB_H__
