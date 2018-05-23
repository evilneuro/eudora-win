// QCWinSockLib.cpp
//
// Manages dynamic loading of WinSock Library DLL.
//
#include "stdafx.h"
#include "afxmt.h"

#include <direct.h>				// for _getdrive()
#include "QCWinSockLib.h"
#include "QCUtils.h"

#include "DebugNewHelpers.h"


//
// Declaration and initialization of statics.
//
WS_CLOSESOCKET* QCWinSockLibMT::m_pfnclosesocket = NULL;
WS_CONNECT* QCWinSockLibMT::m_pfnconnect = NULL;
WS_GETHOSTBYADDR* QCWinSockLibMT::m_pfngethostbyaddr = NULL;
WS_GETHOSTBYNAME* QCWinSockLibMT::m_pfngethostbyname = NULL;
WS_GETHOSTNAME* QCWinSockLibMT::m_pfngethostname = NULL;
WS_GETSERVBYNAME* QCWinSockLibMT::m_pfngetservbyname = NULL;
WS_HTONS* QCWinSockLibMT::m_pfnhtons = NULL;
WS_INET_ADDR* QCWinSockLibMT::m_pfninet_addr = NULL;
WS_INET_NTOA* QCWinSockLibMT::m_pfninet_ntoa = NULL;
WS_IOCTLSOCKET* QCWinSockLibMT::m_pfnioctlsocket = NULL;
WS_RECV* QCWinSockLibMT::m_pfnrecv = NULL;
WS_SEND* QCWinSockLibMT::m_pfnsend = NULL;
WS_SETSOCKOPT* QCWinSockLibMT::m_pfnsetsockopt = NULL;
WS_GETSOCKOPT* QCWinSockLibMT::m_pfngetsockopt = NULL;
WS_SHUTDOWN* QCWinSockLibMT::m_pfnshutdown = NULL;
WS_SOCKET* QCWinSockLibMT::m_pfnsocket = NULL;
WS_SELECT* QCWinSockLibMT::m_pfnselect = NULL;
WS_WSAASYNCGETHOSTBYADDR* QCWinSockLibMT::m_pfnWSAAsyncGetHostByAddr = NULL;
WS_WSAASYNCGETHOSTBYNAME* QCWinSockLibMT::m_pfnWSAAsyncGetHostByName = NULL;
WS_WSAASYNCGETSERVBYNAME* QCWinSockLibMT::m_pfnWSAAsyncGetServByName = NULL;
WS_WSAASYNCSELECT* QCWinSockLibMT::m_pfnWSAAsyncSelect = NULL;
WS_WSACANCELASYNCREQUEST* QCWinSockLibMT::m_pfnWSACancelAsyncRequest = NULL;
WS_WSACANCELBLOCKINGCALL* QCWinSockLibMT::m_pfnWSACancelBlockingCall = NULL;
WS_WSACLEANUP* QCWinSockLibMT::m_pfnWSACleanup = NULL;
WS_WSAGETLASTERROR* QCWinSockLibMT::m_pfnWSAGetLastError = NULL;
WS_WSASETBLOCKINGHOOK* QCWinSockLibMT::m_pfnWSASetBlockingHook = NULL;
WS_WSASETLASTERROR* QCWinSockLibMT::m_pfnWSASetLastError = NULL;
WS_WSASTARTUP* QCWinSockLibMT::m_pfnWSAStartup = NULL;



HINSTANCE QCWinSockLibMT::m_hWinSockLib = (HINSTANCE)NULL;
bool QCWinSockLibMT::m_bLoaded = false;
int QCWinSockLibMT::m_nRefCount = 0;
bool QCWinSockLibMT::m_bInitialized = false;
CCriticalSection QCWinSockLibMT::m_Guard;


bool QCWinSockLibMT::IsLibraryLoaded()
{ 
	return m_bLoaded; 
}

QCWinSockLoader::QCWinSockLoader(bool bCleanup /*=false*/) : m_bCleanup(bCleanup)
{
	QCWinSockLibMT::Init();
}

QCWinSockLoader::~QCWinSockLoader()
{
	QCWinSockLibMT::Cleanup(m_bCleanup);
}

											   

bool QCWinSockLibMT::Init()
{
	CSingleLock lock(&m_Guard, TRUE);
	
	if( QCWinSockLibMT::m_nRefCount == 0 && !m_bInitialized)
	{
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(1, 1);

		//Load the DLL if its not already loaded
		if( !m_bLoaded)
			QCWinSockLibMT::LoadWSLibrary();

		if(!m_bLoaded)
			return false;

		int status = QCWinSockLibMT::WSAStartup(wVersionRequested, &wsaData);
		if (status != 0)
		{
			/*
			if (status == WSASYSNOTREADY)
				ErrorDialog(IDS_ERR_NET_NOT_READY);
			else
				ErrorDialog(IDS_ERR_NO_WINSOCK_DLL);
			*/

			ASSERT(0);
			return false;
		}
		m_bInitialized = true;
	}

	InterlockedIncrement((long *)&QCWinSockLibMT::m_nRefCount); //thread-safe now
	return true;
}



bool QCWinSockLibMT::Cleanup(bool bCleanupAfterClose)
{
	CSingleLock lock(&m_Guard, TRUE);

	ASSERT( m_bInitialized == true);
	
	if( InterlockedDecrement((long *)&QCWinSockLibMT::m_nRefCount) == 0 && bCleanupAfterClose)
	{
		if(QCWinSockLibMT::WSACleanup() == SOCKET_ERROR)
		{

			ASSERT(0);
			switch (QCWinSockLibMT::WSAGetLastError())
			{
			case WSANOTINITIALISED:
			case WSAENETDOWN:
				break;
			case WSAEINPROGRESS:
				QCWinSockLibMT::WSACancelBlockingCall();
			default:
				ASSERT(0);
				break;
			}
		}
		m_bInitialized = false;


		//Free the DLL as we have no need for it
		//QCWinSockLibMT::FreeWSLibrary();
		
		return true;
	}
	return false;
}



////////////////////////////////////////////////////////////////////////
// LoadWSLibrary [public, static]
//
////////////////////////////////////////////////////////////////////////
int QCWinSockLibMT::LoadWSLibrary()
{
	CSingleLock lock(&m_Guard, TRUE);
	
	if(m_bLoaded)
		return true;

	if(!m_bLoaded)
	{
		//
		// Change the current drive so that if it is set to some
		// removable media drive (like a floppy), then the
		// LoadLibrary() call won't cause that drive to be searched
		// (which causes a "Cannot read from Drive X")
		//
		{
			char szPathname[MAX_PATH];
			if (! ::GetModuleFileName(NULL, szPathname, sizeof(szPathname)))
			{
				ASSERT(0);
				return false;
			}

			int nExeDriveNum = ::toupper(szPathname[0]) - 'A' + 1;
			if (::_getdrive() != nExeDriveNum)
				::_chdrive(nExeDriveNum);
		}

		//
		// Load the library.
		//
		ASSERT(NULL == QCWinSockLibMT::m_hWinSockLib);
		if ((QCWinSockLibMT::m_hWinSockLib = ::AfxLoadLibrary("WSOCK32.DLL")) == NULL)
			return false;

		ASSERT(NULL == QCWinSockLibMT::m_pfnclosesocket);
		ASSERT(NULL == QCWinSockLibMT::m_pfnconnect);
		ASSERT(NULL == QCWinSockLibMT::m_pfngethostbyaddr);
		ASSERT(NULL == QCWinSockLibMT::m_pfngethostbyname);
		ASSERT(NULL == QCWinSockLibMT::m_pfngethostname);
		ASSERT(NULL == QCWinSockLibMT::m_pfngetservbyname);
		ASSERT(NULL == QCWinSockLibMT::m_pfnhtons);
		ASSERT(NULL == QCWinSockLibMT::m_pfninet_addr);
		ASSERT(NULL == QCWinSockLibMT::m_pfninet_ntoa);
		ASSERT(NULL == QCWinSockLibMT::m_pfnioctlsocket);
		ASSERT(NULL == QCWinSockLibMT::m_pfnrecv);
		ASSERT(NULL == QCWinSockLibMT::m_pfnsend);
		ASSERT(NULL == QCWinSockLibMT::m_pfnsetsockopt);
		ASSERT(NULL == QCWinSockLibMT::m_pfnshutdown);
		ASSERT(NULL == QCWinSockLibMT::m_pfnsocket);
		ASSERT(NULL == QCWinSockLibMT::m_pfnselect);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSAAsyncGetHostByAddr);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSAAsyncGetHostByName);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSAAsyncGetServByName);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSAAsyncSelect);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSACancelAsyncRequest);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSACancelBlockingCall);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSACleanup);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSAGetLastError);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSASetBlockingHook);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSASetLastError);
		ASSERT(NULL == QCWinSockLibMT::m_pfnWSAStartup);

		QCWinSockLibMT::m_pfnclosesocket			= (WS_CLOSESOCKET *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "closesocket");
		QCWinSockLibMT::m_pfnconnect				= (WS_CONNECT *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "connect");
		QCWinSockLibMT::m_pfngethostbyaddr			= (WS_GETHOSTBYADDR *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "gethostbyaddr");
		QCWinSockLibMT::m_pfngethostbyname			= (WS_GETHOSTBYNAME *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "gethostbyname");
		QCWinSockLibMT::m_pfngethostname			= (WS_GETHOSTNAME *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "gethostname");
		QCWinSockLibMT::m_pfngetservbyname			= (WS_GETSERVBYNAME *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "getservbyname");
		QCWinSockLibMT::m_pfnhtons					= (WS_HTONS *)					::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "htons");
		QCWinSockLibMT::m_pfninet_addr				= (WS_INET_ADDR *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "inet_addr");
		QCWinSockLibMT::m_pfninet_ntoa				= (WS_INET_NTOA *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "inet_ntoa");
		QCWinSockLibMT::m_pfnioctlsocket			= (WS_IOCTLSOCKET *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "ioctlsocket");
		QCWinSockLibMT::m_pfnrecv					= (WS_RECV *)					::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "recv");
		QCWinSockLibMT::m_pfnsend					= (WS_SEND *)					::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "send");
		QCWinSockLibMT::m_pfnsetsockopt				= (WS_SETSOCKOPT *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "setsockopt");
		QCWinSockLibMT::m_pfnshutdown				= (WS_SHUTDOWN *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "shutdown");
		QCWinSockLibMT::m_pfnsocket					= (WS_SOCKET *)					::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "socket");
		QCWinSockLibMT::m_pfnselect					= (WS_SELECT *)					::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "select");

		QCWinSockLibMT::m_pfnWSAAsyncGetHostByAddr	= (WS_WSAASYNCGETHOSTBYADDR *)	::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSAAsyncGetHostByAddr");
		QCWinSockLibMT::m_pfnWSAAsyncGetHostByName	= (WS_WSAASYNCGETHOSTBYNAME *)	::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSAAsyncGetHostByName");
		QCWinSockLibMT::m_pfnWSAAsyncGetServByName	= (WS_WSAASYNCGETSERVBYNAME *)	::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSAAsyncGetServByName");
		QCWinSockLibMT::m_pfnWSAAsyncSelect			= (WS_WSAASYNCSELECT *)			::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSAAsyncSelect");
		QCWinSockLibMT::m_pfnWSACancelAsyncRequest	= (WS_WSACANCELASYNCREQUEST *)	::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSACancelAsyncRequest");
		QCWinSockLibMT::m_pfnWSACancelBlockingCall	= (WS_WSACANCELBLOCKINGCALL *)	::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSACancelBlockingCall");
		QCWinSockLibMT::m_pfnWSACleanup				= (WS_WSACLEANUP *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSACleanup");
		QCWinSockLibMT::m_pfnWSAGetLastError		= (WS_WSAGETLASTERROR *)		::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSAGetLastError");
		QCWinSockLibMT::m_pfnWSASetBlockingHook		= (WS_WSASETBLOCKINGHOOK *)		::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSASetBlockingHook");
		QCWinSockLibMT::m_pfnWSASetLastError		= (WS_WSASETLASTERROR *)		::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSASetLastError");
		QCWinSockLibMT::m_pfnWSAStartup				= (WS_WSASTARTUP *)				::GetProcAddress(QCWinSockLibMT::m_hWinSockLib, "WSAStartup");
	}

	//
	// FORNOW, not verifying that we loaded all the entry
	// points successfully.
	//
	ASSERT(QCWinSockLibMT::m_pfnclosesocket != NULL);
	ASSERT(QCWinSockLibMT::m_pfnconnect != NULL);
	ASSERT(QCWinSockLibMT::m_pfngethostbyaddr != NULL);
	ASSERT(QCWinSockLibMT::m_pfngethostbyname != NULL);
	ASSERT(QCWinSockLibMT::m_pfngethostname != NULL);
	ASSERT(QCWinSockLibMT::m_pfngetservbyname != NULL);
	ASSERT(QCWinSockLibMT::m_pfnhtons != NULL);
	ASSERT(QCWinSockLibMT::m_pfninet_addr != NULL);
	ASSERT(QCWinSockLibMT::m_pfninet_ntoa != NULL);
	ASSERT(QCWinSockLibMT::m_pfnioctlsocket != NULL);
	ASSERT(QCWinSockLibMT::m_pfnrecv != NULL);
	ASSERT(QCWinSockLibMT::m_pfnsend != NULL);
	ASSERT(QCWinSockLibMT::m_pfnsetsockopt != NULL);
	ASSERT(QCWinSockLibMT::m_pfnshutdown != NULL);
	ASSERT(QCWinSockLibMT::m_pfnsocket != NULL);
	ASSERT(QCWinSockLibMT::m_pfnselect != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSAAsyncGetHostByAddr != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSAAsyncGetHostByName != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSAAsyncGetServByName != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSAAsyncSelect != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSACancelAsyncRequest != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSACancelBlockingCall != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSACleanup != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSAGetLastError != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSASetBlockingHook != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSASetLastError != NULL);
	ASSERT(QCWinSockLibMT::m_pfnWSAStartup != NULL);

	m_bLoaded = true;
	return true;
}


////////////////////////////////////////////////////////////////////////
// FreeWSLibrary [public, static]
//
////////////////////////////////////////////////////////////////////////
int QCWinSockLibMT::FreeWSLibrary()
{
	CSingleLock lock(&m_Guard, TRUE);
	
	if(m_bLoaded)
	{

		ASSERT(QCWinSockLibMT::m_nRefCount == 0);
		
		//call WSACleanup ifd needed
		if( m_bInitialized)
		{
			if(QCWinSockLibMT::WSACleanup() == SOCKET_ERROR)
			{
				switch (QCWinSockLibMT::WSAGetLastError())
				{
				case WSANOTINITIALISED:
				case WSAENETDOWN:
					break;
				case WSAEINPROGRESS:
					QCWinSockLibMT::WSACancelBlockingCall();
				default:
					ASSERT(0);
					break;
				}
			}
			m_bInitialized = false;
		}
		
		ASSERT(QCWinSockLibMT::m_hWinSockLib);
		::AfxFreeLibrary(QCWinSockLibMT::m_hWinSockLib);

		m_bLoaded = false;

		QCWinSockLibMT::m_hWinSockLib = NULL;

		QCWinSockLibMT::m_pfnclosesocket = NULL;
		QCWinSockLibMT::m_pfnconnect = NULL;
		QCWinSockLibMT::m_pfngethostbyaddr = NULL;
		QCWinSockLibMT::m_pfngethostbyname = NULL;
		QCWinSockLibMT::m_pfngethostname = NULL;
		QCWinSockLibMT::m_pfngetservbyname = NULL;
		QCWinSockLibMT::m_pfnhtons = NULL;
		QCWinSockLibMT::m_pfninet_addr = NULL;
		QCWinSockLibMT::m_pfninet_ntoa = NULL;
		QCWinSockLibMT::m_pfnioctlsocket = NULL;
		QCWinSockLibMT::m_pfnrecv = NULL;
		QCWinSockLibMT::m_pfnsend = NULL;
		QCWinSockLibMT::m_pfnsetsockopt = NULL;
		QCWinSockLibMT::m_pfnshutdown = NULL;
		QCWinSockLibMT::m_pfnsocket = NULL;
		QCWinSockLibMT::m_pfnselect = NULL;
		QCWinSockLibMT::m_pfnWSAAsyncGetHostByAddr = NULL;
		QCWinSockLibMT::m_pfnWSAAsyncGetHostByName = NULL;
		QCWinSockLibMT::m_pfnWSAAsyncGetServByName = NULL;
		QCWinSockLibMT::m_pfnWSAAsyncSelect = NULL;
		QCWinSockLibMT::m_pfnWSACancelAsyncRequest = NULL;
		QCWinSockLibMT::m_pfnWSACancelBlockingCall = NULL;
		QCWinSockLibMT::m_pfnWSACleanup = NULL;
		QCWinSockLibMT::m_pfnWSAGetLastError = NULL;
		QCWinSockLibMT::m_pfnWSASetBlockingHook = NULL;
		QCWinSockLibMT::m_pfnWSASetLastError = NULL;
		QCWinSockLibMT::m_pfnWSAStartup = NULL;
	}
	
	return QCSOCK_SUCCESS;
}



int QCWinSockLibMT::closesocket(SOCKET s){
	return (m_pfnclosesocket)?m_pfnclosesocket(s):SOCKET_ERROR;
}

int QCWinSockLibMT::connect(SOCKET s, const struct sockaddr FAR *name, int namelen){
	return (m_pfnconnect)?m_pfnconnect(s, name, namelen):SOCKET_ERROR;
}

struct hostent FAR * QCWinSockLibMT::gethostbyaddr(const char FAR * addr, int len, int type){
	
	return (m_pfngethostbyaddr)?m_pfngethostbyaddr(addr, len, type):NULL;
}

struct hostent FAR * QCWinSockLibMT::gethostbyname(const char FAR * name){
	return (m_pfngethostbyname)?m_pfngethostbyname(name):NULL;
}

int QCWinSockLibMT::gethostname(char FAR * name, int namelen){
	return (m_pfngethostname)?m_pfngethostname(name, namelen):SOCKET_ERROR;
}

struct servent FAR * QCWinSockLibMT::getservbyname(const char FAR * name, const char FAR * proto){
	return (m_pfngetservbyname)?m_pfngetservbyname(name, proto):NULL;
}

u_short QCWinSockLibMT::htons(u_short hostshort){
	return (m_pfnhtons)?m_pfnhtons(hostshort):hostshort;
}

unsigned long QCWinSockLibMT::inet_addr(const char FAR * cp){
	return (m_pfninet_addr)?m_pfninet_addr(cp):INADDR_NONE;
}

char FAR * QCWinSockLibMT::inet_ntoa(struct in_addr in){
	return (m_pfninet_ntoa)?m_pfninet_ntoa(in):NULL;
}

int QCWinSockLibMT::ioctlsocket(SOCKET s, long cmd, u_long FAR *argp){
	return (m_pfnioctlsocket)?m_pfnioctlsocket(s, cmd, argp):SOCKET_ERROR;
}

int QCWinSockLibMT::recv(SOCKET s, char FAR * buf, int len, int flags){
	return (m_pfnrecv)?m_pfnrecv(s, buf, len, flags):SOCKET_ERROR;
}

int QCWinSockLibMT::send(SOCKET s, const char FAR * buf, int len, int flags){
	return (m_pfnsend)?m_pfnsend(s, buf, len, flags):SOCKET_ERROR;
}

int QCWinSockLibMT::setsockopt(SOCKET s, int level, int optname, const char FAR * optval, int optlen){
	return (m_pfnsetsockopt)?m_pfnsetsockopt(s, level, optname, optval, optlen):SOCKET_ERROR;
}

int QCWinSockLibMT::getsockopt(SOCKET s, int level, int optname, const char FAR * optval, int FAR *optlen){
	return (m_pfngetsockopt)?m_pfngetsockopt(s, level, optname, optval, optlen):SOCKET_ERROR;
}

int QCWinSockLibMT::shutdown(SOCKET s, int how){
	return (m_pfnshutdown)?m_pfnshutdown(s, how):SOCKET_ERROR;
}

SOCKET QCWinSockLibMT::socket(int af, int type, int protocol){
	return (m_pfnsocket)?m_pfnsocket(af, type, protocol):INVALID_SOCKET;
}

int QCWinSockLibMT::select (int nfds,fd_set FAR * readfds,fd_set FAR * writefds,fd_set FAR * exceptfds,const struct timeval FAR * timeout){
	return (m_pfnselect)?m_pfnselect(nfds, readfds, writefds, exceptfds, timeout):INVALID_SOCKET;
}

HANDLE QCWinSockLibMT::WSAAsyncGetHostByAddr(HWND hWnd, UINT wMsg, const char FAR * addr, int len, int type, char FAR *buf, int buflen){
	return (m_pfnWSAAsyncGetHostByAddr)?m_pfnWSAAsyncGetHostByAddr(hWnd, wMsg, addr, len, type, buf, buflen):0;
}

HANDLE QCWinSockLibMT::WSAAsyncGetHostByName(HWND hWnd, UINT wMsg, const char FAR * name, char FAR *buf, int buflen){
		return (m_pfnWSAAsyncGetHostByName)?
			m_pfnWSAAsyncGetHostByName(hWnd, wMsg, name, buf, buflen):0;
}
	
HANDLE QCWinSockLibMT::WSAAsyncGetServByName(HWND hWnd, UINT wMsg, const char FAR * name, const char FAR * proto, char FAR * buf, int buflen){
	return (m_pfnWSAAsyncGetServByName)?
		m_pfnWSAAsyncGetServByName(hWnd, wMsg, name, proto, buf, buflen):0;
}

int QCWinSockLibMT::WSAAsyncSelect(SOCKET s, HWND hWnd, UINT wMsg, long LEvent){
	return (m_pfnWSAAsyncSelect)?m_pfnWSAAsyncSelect(s, hWnd, wMsg, LEvent):SOCKET_ERROR;
}

int QCWinSockLibMT::WSACancelAsyncRequest(HANDLE handle){
	return (m_pfnWSACancelAsyncRequest)?m_pfnWSACancelAsyncRequest(handle):SOCKET_ERROR;
}

int QCWinSockLibMT::WSACancelBlockingCall(void){
	return (m_pfnWSACancelBlockingCall)?m_pfnWSACancelBlockingCall():SOCKET_ERROR;
}

int QCWinSockLibMT::WSAGetLastError(void){
	return (m_pfnWSAGetLastError)?m_pfnWSAGetLastError():0;
}

FARPROC QCWinSockLibMT::WSASetBlockingHook(FARPROC lpBlockFunc){
	return (m_pfnWSASetBlockingHook)?m_pfnWSASetBlockingHook(lpBlockFunc):NULL;
}

void QCWinSockLibMT::WSASetLastError(int iError){
	m_pfnWSASetLastError(iError);
}

int QCWinSockLibMT::WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData){
	TRACE("STARTING Winsock\n");
	return (m_pfnWSAStartup)?m_pfnWSAStartup(wVersionRequired, lpWSAData):WSAEINVAL;
}

int QCWinSockLibMT::WSACleanup(void){
	TRACE("CLEANING Winsock\n");
	return (m_pfnWSACleanup)?m_pfnWSACleanup():SOCKET_ERROR;
}
