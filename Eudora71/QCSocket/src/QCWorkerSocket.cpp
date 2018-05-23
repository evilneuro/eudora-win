// QCWorkerSocket.cpp
//
// Contains the code for Network, Winsock, and Dialup class
//
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include "afxmt.h"
#include "resource.h"

#include "QCUtils.h"
#include "debug.h" //For Logging

#include "QCNetSettings.h"
#include "QCSockCallbacks.h"
#include "QCRasConnection.h"



#include "QCWorkerSocket.h"
#include "TaskInfo.h"
#include "sslsettings.h"

#pragma warning(disable: 4786 4663 4244 4018 4146 4100)

#include <xstddef>
#include <map>

//#pragma warning (default: 4663 4244 4018 4146)


#include "EudoraMsgs.h"

using namespace std;

#ifdef EXPIRING  
#include "mainfrm.h"
#include "timestmp.h"
extern CTimeStamp	g_TimeStamp;
#endif

static HINSTANCE g_QCSSLDll = NULL;
static FPNQCSSLBeginSession g_fnQCSSLBeginSession = NULL;
static FPNQCSSLEndSession   g_fnQCSSLEndSession = NULL;
static FPNQCSSLClean g_fnQCSSLClean = NULL;
static FPNQCSSLWrite g_fnQCSSLWrite = NULL;
static FPNQCSSLRead g_fnQCSSLRead = NULL;
static FPNQCSSLGetConnectionInfo q_fnQCSSLGetConnectionInfo = NULL;
static FPNQCSSLAddTrustedCertFromFile g_fnQCSSLAddTrustedCertFromFile = NULL;										 
static FPNQCSSLGetRootCertList g_fnQCSSLGetRootCertList = NULL;
static FPNQCSSLGetUserCertList g_fnQCSSLGetUserCertList = NULL;
static FPNQCSSLAddTrustedUserCert g_fnQCSSLAddTrustedUserCert = NULL;
static FPNQCSSLDeleteTrustedUserCert g_fnQCSSLDeleteTrustedUserCert = NULL;

static bool		g_bSSLHandshakeDone = false;


class CRString : public CString
{
public:
	CRString(UINT StringID) { LoadString(StringID); }
};




#define ALL_OK			 0
#define TIMEREXPIRED	-1
#define ESCAPED			-2
#define WINSOCK_ERROR	-5 
#define UNEXPECTED_CLOSE -6


struct CString_NoCaseLess{
	bool operator()(CString first, CString second) const 
	{ return first.CompareNoCase(second)<0;}
};

template <class T>
class QCSocketCacheMT {
	CCriticalSection m_Guard;
	map<CString, T, CString_NoCaseLess> cmap; 
	
public:

	void Add(CString str, T port);
	void Remove(CString str);
	T Find(CString str);
};



#include "DebugNewHelpers.h"





template <class T>
void QCSocketCacheMT<T>::Add(CString str, T port)
{
	CSingleLock lock(&m_Guard, TRUE);
	cmap[str] = port;
}

template <class T>
void QCSocketCacheMT<T>::Remove(CString str)
{
	CSingleLock lock(&m_Guard, TRUE);
	if(cmap.find(str) != cmap.end())
		cmap.erase(cmap.find(str));
}

template <class T>
T QCSocketCacheMT<T>::Find(CString str)
{
	CSingleLock lock(&m_Guard, TRUE);
	map<CString, T, CString_NoCaseLess>::const_iterator pCache;
	if( (pCache=cmap.find(str)) != cmap.end())
		return (*pCache).second;
	else return T(0);
}


//thread safe Cache global statics
static QCSocketCacheMT <u_long>  s_AddressList;
static QCSocketCacheMT <u_int>   s_PortList;


Network::Network(QCNetworkSettings *pSettings /*=NULL*/, LONG Size /*= -1*/):m_Settings(pSettings),m_bSSLMode(false)

{
	
	//m_nBufferSize = int(Size < 0 ? GetIniLong(IDS_INI_NETWORK_BUFFER_SIZE) : Size);
	
	if(Size < 0){
		ASSERT(m_Settings);
		m_nBufferSize = m_Settings->GetNetworkBufferSize();
	}
	else
		m_nBufferSize = Size;

	m_pBuffer = DEBUG_NEW_NOTHROW char[m_nBufferSize];
	
	m_pBufPtr = m_pBuffer;
	m_nValidBytes = 0;
	m_bEscapePressed = FALSE;

	// A filebase subclass should reset this
	m_bIsFileBased = FALSE;
	m_Port = 0;// the "actual" port number used

	m_GetChValidBytes = 0;
}







Network::~Network()
{
	delete [] m_pBuffer;
	m_pBuffer = NULL;
	
	//NetConnection = NULL;
}


BOOL Network::IsOK() const
{
	return ( m_pBuffer != NULL );
}



// GetLine
// Read a line of text into our buffer
// note: len cannot be larger than size bytes

int Network::GetLine(char* line, int len)
{
	BOOL NewlineFound = FALSE;
	char* OrigLine = line;
	int ErrorStatus;
	int Trans = 0;

	*line = 0;
	
	//The buffer size should be atleast 3K, else we may not be able to 
	//read the entire line into the buffer & the rest of the line is skipped.
	//QC_NETWORK_BUF_SIZE defined in QCNetSettings.h
	ASSERT(len >= QC_NETWORK_BUF_SIZE);
	
	FlushLogGetChBuffer();

	if (!m_nValidBytes)
	{
		if ((ErrorStatus = Read()) <= 0)
			return (ErrorStatus);
	}
	
	//TODO:
	if(m_bEscapePressed)
		return -1;

	while (len > 2)
	{
		Trans++;
		m_nValidBytes--;
		len--;
		if ((*line++ = *m_pBufPtr++) == '\n')
		{
			NewlineFound = TRUE;
			
			if (*(line-2) != '\r')
			{
				*(line - 1) = '\r';
				*line = '\n';
				line++;
				len--;
				Trans++;
			}
			*line = 0;
			break;
		}
		if (!m_nValidBytes)
		{
			if ((ErrorStatus = Read()) <= 0)
				return (ErrorStatus);
		}
	}

	if (NewlineFound == FALSE)
	{
		if (len == 2)
		{
			m_nValidBytes--;
			if (*m_pBufPtr == '\n')
				NewlineFound = TRUE;
			*line++ = *m_pBufPtr++;
			*line = 0;
			len--;
			Trans++;
		}

		while (NewlineFound == FALSE)
		{
			if (m_nValidBytes)
			{
				m_nValidBytes--;
				if (*m_pBufPtr++ == '\n')
					NewlineFound = TRUE;
			}
			else
			{
				if ((ErrorStatus = Read()) <= 0)
					return (ErrorStatus);
			}
		}
	}

	PutDebugLog(DEBUG_MASK_RCV, OrigLine, Trans);
    
	return (Trans);
}

//
// GetCh
// 
// Returns one character or 0 if error
//

#pragma warning(disable: 4706)
int Network::GetCh(char* c)
{
	int ErrorStatus;

	if (!m_nValidBytes)
	{
		if ((ErrorStatus = Read()) <= 0)
			return -1;
		//We should return -1 and not zero(or ErrorStatus) because
		//we need a way to differentiate between 
		//a null character and error.
		//But we should not modify Read() itself to do this because
		// returning zero by read does never means that it is a null 
		//character - returning zero by read() means that
		//zero bytes were read.(not byte '\0' was read).
		//Also not modifying read saves us from modifying the other
		//code dependent on it - Sagar
	}

	m_nValidBytes--;
	*c = (char)(*m_pBufPtr++ & 0xFF);

	if (m_GetChValidBytes >= sizeof(m_GetChBuffer))
		FlushLogGetChBuffer();
	m_GetChBuffer[m_GetChValidBytes++] = *c;
	if (*c == '\n')
		FlushLogGetChBuffer();

	return ((unsigned char)*c);
}

// Put
// This routine takes the given buffer and writes it first to the
// internal buffer, then onto the network as needed
//
int Network::Put(const char* bufptr, int length)
{
	if (!bufptr)
		return (1);

	FlushLogGetChBuffer();

	PutDebugLog(DEBUG_MASK_TRANS, bufptr, length);

	if (length < 0)
		length = strlen(bufptr);

	if(m_bEscapePressed)
		return -1;
	

	for (; length; length--)
	{
		if (m_nValidBytes >= (int) m_nBufferSize)
		{
			if (Write() < 0)
				return (-1);
		}
		*m_pBufPtr++ = *bufptr++;
		m_nValidBytes++;
	}

	return (1);
}

void Network::SetSSLMode(bool bVal,CString person, SSLSettings *pSettings, CString protocol)
{ 
	if(m_bSSLMode) //this needs to be done only once.
		return;

	g_QCSSLDll = ::LoadLibrary("qcssl");
	//remove this when testing is done
	assert(g_QCSSLDll!=NULL);
	if(!g_QCSSLDll)
		return;

	g_fnQCSSLBeginSession = (FPNQCSSLBeginSession)GetProcAddress(g_QCSSLDll, "QCSSLBeginSession");
	g_fnQCSSLEndSession = (FPNQCSSLEndSession)GetProcAddress(g_QCSSLDll, "QCSSLEndSession");
	g_fnQCSSLClean = (FPNQCSSLClean)GetProcAddress(g_QCSSLDll, "QCSSLClean");
	g_fnQCSSLWrite = (FPNQCSSLWrite)GetProcAddress(g_QCSSLDll, "QCSSLWrite");
	g_fnQCSSLRead =  (FPNQCSSLRead)GetProcAddress(g_QCSSLDll, "QCSSLRead");
	q_fnQCSSLGetConnectionInfo = (FPNQCSSLGetConnectionInfo)GetProcAddress(g_QCSSLDll, "QCSSLGetConnectionInfo");
	g_fnQCSSLAddTrustedCertFromFile = 	(FPNQCSSLAddTrustedCertFromFile)GetProcAddress(g_QCSSLDll, "QCSSLAddTrustedCertFromFile");							 
    g_fnQCSSLGetRootCertList = (FPNQCSSLGetRootCertList)GetProcAddress(g_QCSSLDll, "QCSSLGetRootCertList");;
    g_fnQCSSLGetUserCertList = (FPNQCSSLGetUserCertList)GetProcAddress(g_QCSSLDll, "QCSSLGetUserCertList");
    g_fnQCSSLAddTrustedUserCert = (FPNQCSSLAddTrustedUserCert) GetProcAddress(g_QCSSLDll, "QCSSLAddTrustedUserCert");
    g_fnQCSSLDeleteTrustedUserCert = (FPNQCSSLDeleteTrustedUserCert)GetProcAddress(g_QCSSLDll, "QCSSLDeleteTrustedUserCert");

	m_bSSLMode = bVal;
}

HINSTANCE Network::GetQCSSLDll()
{
	return g_QCSSLDll;
}

FPNQCSSLBeginSession Network::GetQCSSLBeginSession()
{
	return g_fnQCSSLBeginSession;
}

FPNQCSSLEndSession   Network::GetQCSSLEndSession()
{
	return g_fnQCSSLEndSession;
}

FPNQCSSLClean Network::GetQCSSLClean()
{
	return g_fnQCSSLClean;
}

FPNQCSSLWrite Network::GetQCSSLWrite()
{
	return g_fnQCSSLWrite;
}

FPNQCSSLRead Network::GetQCSSLRead()
{
	return g_fnQCSSLRead;
}

FPNQCSSLGetConnectionInfo Network::GetQCSSLGetConnectionInfo()
{
	return q_fnQCSSLGetConnectionInfo;
}

FPNQCSSLAddTrustedCertFromFile Network::GetQCSSLAddTrustedCertFromFile()
{
	return g_fnQCSSLAddTrustedCertFromFile;
}

FPNQCSSLGetRootCertList Network::GetQCSSLGetRootCertList()
{
	return g_fnQCSSLGetRootCertList;
}

FPNQCSSLGetUserCertList Network::GetQCSSLGetUserCertList()
{
	return g_fnQCSSLGetUserCertList;
}

FPNQCSSLAddTrustedUserCert Network::GetQCSSLAddTrustedUserCert()
{
	return g_fnQCSSLAddTrustedUserCert;
}

FPNQCSSLDeleteTrustedUserCert Network::GetQCSSLDeleteTrustedUserCert()
{
	return g_fnQCSSLDeleteTrustedUserCert;
}


bool Network::CleanSSLLibrary()
{
	if(g_QCSSLDll)
	{
		if (g_fnQCSSLClean)
			g_fnQCSSLClean();	

		::FreeLibrary(g_QCSSLDll);
		g_QCSSLDll = NULL;
		return true;
	}
    return false;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//                                                                    //
//                        W i n S o c k                               //
//                                                                    //
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#define CancelCommand()

int QCWorkerSocket::s_OpenConnectionCount = 0;
HANDLE	QCWorkerSocket::m_hAsyncThread = NULL;
DWORD	QCWorkerSocket::m_dwAsyncThreadID = 0;
CWinThread *QCWorkerSocket::m_pWinThread = NULL;
CCriticalSection QCWorkerSocket::m_Guard;	


enum { iOTHER = 0, iCONNECT = 1, iREAD = 2, iWRITE = 3, iCLOSE = 4 };
#define DB_EXPECT_VALUE 200

QCWorkerSocket::QCWorkerSocket(QCNetworkSettings *pSettings) :
	Network(pSettings),
	m_bWinSockLoaded(false),
	m_bDialingInProgress(false),
	m_bAbortConnection(false),
	m_bTimerExpired(false),
	m_hWnd(NULL),
	m_hEvent(NULL),
	m_Socket(INVALID_SOCKET),
	m_bNeedsConnectionCountDecrement(false),
	m_bQCSSLInit(NULL),
	m_pSSLReference(NULL),
	m_iSSLError(0),
	m_pCertData(NULL)
{
	for(int i=0; i < sizeof(m_AsyncEvent)/sizeof(m_AsyncEvent[0]); i++)
		m_AsyncEvent[i] = 0;
}


QCWorkerSocket::~QCWorkerSocket()
{
	if (m_bNeedsConnectionCountDecrement)
	{
		if (--s_OpenConnectionCount < 0)
		{
			ASSERT(0);
			s_OpenConnectionCount = 0;
		}
	}




	Close();
	ShutdownNetwork();
	
	// Get rid of hidden window used for receiving messages, if it exists
	if(m_hWnd)
	{
		ASSERT(IsWindow(m_hWnd));

		SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)NULL);
		
		ResetEvent(m_hEvent);
		if (m_dwAsyncThreadID)
		{
			if( PostThreadMessage (m_dwAsyncThreadID, USER_DESTROY_ASYNC_WINDOW, (WPARAM)m_hWnd, (LPARAM)m_hEvent) != 0)
			{
				WaitForSingleObject(m_hEvent, INFINITE);
				ResetEvent(m_hEvent);
				m_hWnd = NULL;
			}
		}
	}		

	// This is bad if it ASSERTs.  We're probably going to
	// strand a window because no one else will clean it up.
	ASSERT(NULL == m_hWnd);

	if(m_hEvent)
		CloseHandle(m_hEvent);
}

/*static*/ bool QCWorkerSocket::HasOpenConnection()
{
	return s_OpenConnectionCount > 0;
}

// Close
// Closes the connection
//
int QCWorkerSocket::Close()
{
	m_nValidBytes = 0;

	if (m_pSSLReference)
	{
		if (m_pSSLReference->m_pSSL)
		{
			if (g_fnQCSSLEndSession)
			{
				g_fnQCSSLEndSession(m_pSSLReference->m_pSSL);
			}
			m_pSSLReference->m_pSSL = NULL;
		}
		delete m_pSSLReference;
		m_pSSLReference = NULL;
	}

	if (m_Socket == INVALID_SOCKET)
		return (-1);


	m_Settings->Progress(CRString(IDS_CLOSING_CONNECTION));


	//notify peer that we are done sending data
	QCWinSockLibMT::WSAAsyncSelect(m_Socket, m_hWnd, 0, 0);
	QCWinSockLibMT::shutdown(m_Socket, 1);
	char buf[512];
	while (QCWinSockLibMT::recv(m_Socket, buf, sizeof(buf) - 1, 0) > 0)
			;
	
	int status = QCWinSockLibMT::closesocket(m_Socket);

	m_Socket = INVALID_SOCKET;
	
	// if anything has gone wrong unload the winsock lib ( via ShutdownNetwork()
	if (m_bAbortConnection || m_bTimerExpired || m_bEscapePressed)
		ShutdownNetwork();
	
	return (status);
}

void QCWorkerSocket::SwitchToBlockingMode()
{
	int nRet = QCWinSockLibMT::WSAAsyncSelect(m_Socket, m_hWnd, 0, 0);
	if(nRet ==  SOCKET_ERROR)
	{
		ASSERT(0);
		char buf[] = "Failed to set blocking mode";
		PutDebugLog(DEBUG_MASK_TRANS, buf, strlen(buf));
	}

	unsigned long flag = 0; //operate in blocking mode
	QCWinSockLibMT::ioctlsocket(m_Socket, FIONBIO, &flag);
}

void QCWorkerSocket::SwitchToAsyncMode()
{
	QCWinSockLibMT::WSAAsyncSelect(m_Socket, m_hWnd, USER_WINSOCK_MESSAGE, FD_READ | FD_WRITE | FD_CLOSE);
}

//
//	QCWorkerSocket::ClearSSLCertData()
//
//	Clear the data fields used to cache SSL cert error data.  See InitializeQCSSL() for details.
//
void QCWorkerSocket::ClearSSLCertData()
{
	m_iSSLError = 0;
	m_strSSLCertText = "";
	m_strSSLCertRejection = "";
	m_pCertData = NULL;
}

//
//	QCWorkerSocket::AddTrustedCert()
//
//	Public method to allow outside classes to specify a certificate to add to the trusted list.
//
void QCWorkerSocket::AddTrustedCert(void *pCertData)
{
	FPNQCSSLAddTrustedUserCert	 fnAddTrustedUserCert = NULL;
	if (Network::GetQCSSLDll() != NULL)
	{
		fnAddTrustedUserCert = Network::GetQCSSLAddTrustedUserCert();
		if(fnAddTrustedUserCert)
		{
			fnAddTrustedUserCert((CertData*)pCertData, NULL/*ignored*/, NULL/*ignored*/);
		}
	}
}

void QCWorkerSocket::RequestStop()
{ 
	//If ras connection is not yet established, ask it to stop dialing
	if(m_Settings->IsDialupConnection() && m_bDialingInProgress)
	{
		QCRasLibrary::RequestStop();
		m_bEscapePressed = TRUE;
	}
	else
	{
		m_bEscapePressed = TRUE;
		SetEvent(m_hEvent);
	}
}



void QCWorkerSocket::SetLinger(int time)
{
	struct linger lin;

	lin.l_onoff = (u_short)(time);
	lin.l_linger = (u_short)(time);

	ASSERT(m_Socket != INVALID_SOCKET);

	QCWinSockLibMT::setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *) &lin, sizeof(lin));
}

#define IDS_WINSOCK_BASE_ERROR 8000

int QCWorkerSocket::SetError(const char* message, int ErrorNum)
{
	int ret = -1;

	// If cancelled by user, don't report error
	if (m_bEscapePressed)
		ret = ESCAPED;
	else
	{
		if (m_bTimerExpired)
			ErrorNum = IDS_ERR_EUDORA_NET_TO + WSABASEERR - IDS_WINSOCK_BASE_ERROR;	// Eudora timeout value
		else if (!ErrorNum || (ErrorNum == WINSOCK_ERROR))
			ErrorNum = QCWinSockLibMT::WSAGetLastError();
		
		if(ErrorNum == UNEXPECTED_CLOSE)
		{
			if(m_ErrorValue)
				ReportError(IDS_NET_ERR_FORMAT, message,
				(const char*)CRString(IDS_WINSOCK_BASE_ERROR + (m_ErrorValue - WSABASEERR)), m_ErrorValue);
			else
				ReportError(IDS_NET_ERR_FORMAT, message, "Connection closed by foreign host.", 0);
		}
		else
		{
			ReportError(IDS_NET_ERR_FORMAT, message,
				(const char*)CRString(IDS_WINSOCK_BASE_ERROR + (ErrorNum - WSABASEERR)), ErrorNum);
		}
		ret = -1;
	}
	
	return (ret);
}

void QCWorkerSocket::ReportError(UINT StringID, ...)
{
	char buf[1024] = {0};
		
	va_list argList;
	va_start(argList, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), argList);
	va_end(argList);

	m_Settings->ReportError(buf, TERR_WINSOCK);
}


int QCWorkerSocket::ReadError(int ErrorNum)
{
	return (SetError(CRString(IDS_ERR_NET_READING), ErrorNum));
}

int QCWorkerSocket::WriteError(int ErrorNum)
{
	return (SetError(CRString(IDS_ERR_NET_WRITING), ErrorNum));
}



// Get the port for a given service name.  Keeps a cache of hits.
u_short QCWorkerSocket::GetPort(UINT ServiceID, UINT Port)
{
	char buf[MAXGETHOSTSTRUCT];
	struct servent* ServiceInfo = (struct servent*)buf;
	//BOOL DoCaching = (GetIniShort(IDS_INI_NETWORK_CACHING) != 0);
	u_short ReturnPort = 0;
	int Result;
	CRString Service(ServiceID);

	if (m_Settings->DoCaching())
	{
		// If already cached then return it
		int port = s_PortList.Find(Service);
		if(port > 0) 
			return (u_short)port;
	}

	ResetCommand();
	m_Socket = 0;
	// Assign the socket variable to the returned async handle for the comparison 
	// when the Winsock stack returns the async message.  Zero indicates an error
	m_Socket = (SOCKET) QCWinSockLibMT::WSAAsyncGetServByName(m_hWnd, USER_GETSERV_MESSAGE, Service, "tcp", buf, sizeof(buf));
	if(!m_Socket){
		//int err = QCWinSockLibMT::WSAGetLastError();
		ASSERT(m_Socket);
		m_Socket = INVALID_SOCKET;
		return 0;
	}

	Result = MessageWait(USER_GETSERV_MESSAGE);
	CancelCommand();

	if (Result == ALL_OK)
		ReturnPort = ServiceInfo->s_port;
	else
	{
		if (m_Socket != 0)
			QCWinSockLibMT::WSACancelAsyncRequest((HANDLE)m_Socket);
		
		ReturnPort = QCWinSockLibMT::htons((u_short)Port);
	}
	
	m_Socket = INVALID_SOCKET;
	
	//if (DoCaching)
	if (m_Settings->DoCaching())
		s_PortList.Add(Service, ReturnPort);
	
	return (ReturnPort);
}

void QCWorkerSocket::GetRealHostName(const char *machine, char *realname, int retsize)
{
	char tmpBuf[1024] = {0};
	char buf[MAXGETHOSTSTRUCT+512] = {0};
	struct hostent* MachineInfo = (struct hostent*)buf;
	
	char ServerName[512+1];  //128
	u_long Address;
	int Result;

	strcpy(ServerName, machine);
	::TrimWhitespaceMT(ServerName);

	*realname = '\0';

	ResetCommand();
	m_Socket = 0;

	// Assign the socket variable to the returned async handle
	// for the comparison when the Winsock stack returns the
	// async message
	m_Socket = (SOCKET) QCWinSockLibMT::WSAAsyncGetHostByName(m_hWnd, USER_GETHOSTNAME_MESSAGE, ServerName, buf, sizeof(buf));

	if(!m_Socket){
		//int err = QCWinSockLibMT::WSAGetLastError();
		ASSERT(m_Socket);
	}	
	Result = MessageWait(USER_GETHOSTNAME_MESSAGE);
	CancelCommand();

	if (Result != ALL_OK)
	{
		if (m_Socket != 0)
			QCWinSockLibMT::WSACancelAsyncRequest((HANDLE)m_Socket);
		wsprintf(tmpBuf, CRString(IDS_ERR_GETHOSTBYNAME), ServerName);
		SetError(tmpBuf, m_ErrorValue);
		m_Socket = INVALID_SOCKET;
		return;
	}
	m_Socket = INVALID_SOCKET;

	memcpy(&Address, MachineInfo->h_addr_list[0], sizeof(Address));
	
	// Now do the reverse lookup, the A record may be rotating through a
	// list of possible servers.  What we really want is the PTR record value.
	ResetCommand();
	m_Socket = 0;
	// Assign the socket variable to the returned async handle
	// for the comparison when the Winsock stack returns the
	// async message
	m_Socket = (SOCKET) QCWinSockLibMT::WSAAsyncGetHostByAddr(m_hWnd, USER_GETHOSTADDR_MESSAGE, (char *)&Address, 4, PF_INET, buf, sizeof(buf));
	if(!m_Socket){
		//int err = QCWinSockLibMT::WSAGetLastError();
		ASSERT(m_Socket);
	}
	Result = MessageWait(USER_GETHOSTADDR_MESSAGE);
	CancelCommand();

	if (Result != ALL_OK)
	{
		struct in_addr foo;
		memcpy(&foo.s_addr, &Address, sizeof(Address));

		if (m_Socket != 0)
			QCWinSockLibMT::WSACancelAsyncRequest((HANDLE)m_Socket);
		wsprintf(tmpBuf, CRString(IDS_ERR_GETHOSTBYADDR), QCWinSockLibMT::inet_ntoa(foo));
		SetError(tmpBuf, m_ErrorValue);
		m_Socket = INVALID_SOCKET;
		return;
	}
	m_Socket = INVALID_SOCKET;

	strncpy(realname, MachineInfo->h_name, retsize);
	return;
}


// Get the address for a given machine name.  Keeps a cache of hits.
u_long QCWorkerSocket::GetAddress(const char* Machine, int Quiet)
{
	char buf[1024] = {0};
	char szHostEntry[MAXGETHOSTSTRUCT+512] = {0}; //just to be safe
	struct hostent* MachineInfo = (struct hostent*)szHostEntry;
	char ServerName[512+1]; //128
	u_long Address;
	int Result;
	
	strcpy(ServerName, Machine);
	::TrimWhitespaceMT(ServerName);

	if (m_Settings->DoCaching())
	{
		// If already cached then return it
		int addr = s_AddressList.Find(Machine);
		if(addr > 0) return addr;
	}

	// Backwards compatibility with old verisons of Eudora that use
	// square brackets to specify IP addresses
	if (ServerName[0] == '[' && ServerName[strlen(ServerName) - 1] == ']')
	{
		ServerName[strlen(ServerName) - 1] = 0;
		strcpy(ServerName, ServerName + 1);
	}

	// Is it an IP address?
	Address = QCWinSockLibMT::inet_addr(ServerName);
	if (Address != -1)
	{
		if (!Quiet)
		{
			wsprintf(buf, CRString(IDS_CONTACTING_MACHINE), "", ServerName);
			m_Settings->Progress(buf);
		}   
	}
	else
	{
		// It's a machine name, so let's resolve it
		if (!Quiet)
		{
			wsprintf(buf, CRString(IDS_RESOLVING_ADDRESS), ServerName);
			m_Settings->Progress(buf);
		}
	
		ResetCommand();
		m_Socket = 0;

		m_Socket = (SOCKET) QCWinSockLibMT::WSAAsyncGetHostByName(m_hWnd, USER_GETHOSTNAME_MESSAGE, 
														ServerName, szHostEntry, sizeof(szHostEntry));

		if(!m_Socket){
			//int err = QCWinSockLibMT::WSAGetLastError();
			ASSERT(m_Socket);
			m_Socket = INVALID_SOCKET;
			return (u_long)-1;
		}
		Result = MessageWait(USER_GETHOSTNAME_MESSAGE);
		CancelCommand();
	
		if (Result != ALL_OK)
		{
			if (m_Socket != 0)
				QCWinSockLibMT::WSACancelAsyncRequest((HANDLE)m_Socket);
			wsprintf(buf, CRString(IDS_ERR_GETHOSTBYNAME), ServerName);
			SetError(buf, m_ErrorValue);
			m_Socket = INVALID_SOCKET;
			return ((u_long)-1);
		}
		m_Socket = INVALID_SOCKET;
	
		memcpy(&Address, MachineInfo->h_addr, sizeof(Address));
		strncpy(ServerName, MachineInfo->h_name, sizeof(ServerName)-1);
	
		if (!Quiet)
		{
			struct in_addr in;
			in.s_addr = Address;
			wsprintf(buf, CRString(IDS_CONTACTING_MACHINE), ServerName, QCWinSockLibMT::inet_ntoa(in));
			m_Settings->Progress(buf);
		}
	}
	
	if (m_Settings->DoCaching())
		s_AddressList.Add(Machine, Address);

	return (Address);
}



bool QCWorkerSocket::StartWinSock()
{
	//make sure winsock is loaded
	if(!QCWinSockLibMT::LoadWSLibrary())
	{
		ReportError(IDS_WINSOCK_BASE_ERROR + 200);		// display error message
		return false;
	}


	if( QCWinSockLibMT::Init() == true)
		m_bWinSockLoaded = true;


	if(m_Settings->IsDialupConnection())
	{
		ASSERT( m_Settings->Progress);
		ASSERT( m_Settings->ReportError);

		m_bDialingInProgress = true;

		bool bResult = QCRasLibrary::OpenConnection(m_Settings->GetRasDialParams(), m_Settings->GetSavePasswordFlag(),
				m_Settings->GetNetworkOpenTimeout(), m_Settings->Progress, m_Settings->ReportError);
		if(!bResult)
		{
			//ReportError(IDS_STRING8204);
			m_bDialingInProgress = false;
			return false;
		}
		m_bDialingInProgress = false;
	}


	
	// Create hidden window for asynchronous callbacks
	if (!m_hWnd) // && AutoConnection))
	{

		// Create the hidden window for async messages.
		// NOTE: If we fail, just return. Cleaning up the socket will be hndled by
		// the class destructor.
		if ( !CreateAsyncWindow () )
			return false;
	}



	return(true);
}


void QCWorkerSocket::ShutdownNetwork()
{
	/*if (m_bGotRasConnection)
	{
		HRESULT hrClose = m_rasConnection.CloseRasConnection();
		ASSERT(SUCCEEDED(hrClose));
		m_bGotRasConnection = FALSE;
	}*/

	if (m_bWinSockLoaded)
	{
		QCWinSockLibMT::Cleanup(m_Settings->GetNetImmediateClose());
		m_bWinSockLoaded = false;
	}
}

// Open
// Given the machine name, and the port, it opens
// the port and sets up the socket.
//
int QCWorkerSocket::Open(const char* Machine, UINT ServiceID, UINT PortNumber, UINT DefaultPort, int Quiet, int FailOver)
{
	struct sockaddr_in SrvAddr;
	int ConnectStatus;
	char Buffer[256];

	UNREFERENCED_PARAMETER(FailOver);

#ifdef EXPIRING
	if ( g_TimeStamp.IsExpired1() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
		return -2;
	}
#endif
	
	m_bAbortConnection = false;
	//m_dwTimeoutLength = GetIniLong(IDS_INI_NETWORK_OPEN_TIMEOUT);
	m_dwTimeoutLength = m_Settings->GetNetworkOpenTimeout();

	
	// Uh oh, did we forget to clean up in the last connection?
	ASSERT(m_nValidBytes == 0);
	m_nValidBytes = 0;

	m_bEscapePressed = FALSE;

	if (!m_bWinSockLoaded)
	{
	    if (!StartWinSock())
		    return (-1);
	}

	if(m_bEscapePressed)
		return -1;

	if(PortNumber)
	{
		SrvAddr.sin_port = QCWinSockLibMT::htons((u_short)PortNumber);
	}
	else
	{
		//SrvAddr.sin_port = GetPort(ServiceID, PortID);
		//if (!SrvAddr.sin_port)
		SrvAddr.sin_port = GetPort(ServiceID, DefaultPort);
	}

	//if (GetIniShort(IDS_INI_AUTH_KERB))
	if (m_Settings->DoAuthenticateKerberos())
	{
		// Generate real name for Kerberos
		GetRealHostName(Machine, m_szMachine, sizeof(m_szMachine));
		// No longer change the original machine name to the real name as this
		// messes up SSL.
		// Machine = m_szMachine;
		if (!m_szMachine)
			return (-1);
	}
	
	u_long Address = GetAddress(Machine, Quiet);
	if (Address == -1)
	{
		return (-1);
	}

	memcpy(&SrvAddr.sin_addr, &Address, sizeof(Address));
	SrvAddr.sin_family= AF_INET;

	m_ServerName = Machine;
	m_ServerIP = QCWinSockLibMT::inet_ntoa(SrvAddr.sin_addr);
	m_Port = SwapShortMT((u_short)SrvAddr.sin_port);



	ResetCommand();
	m_Socket = QCWinSockLibMT::socket(AF_INET, SOCK_STREAM, 0);
	CancelCommand();

	if (m_Socket == INVALID_SOCKET)
	{
		return (SetError(CRString(IDS_ERR_GETTING_SOCKET), 0));
	}

	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_PROG))
	{
		sprintf(Buffer, "Open %s:%d", QCWinSockLibMT::inet_ntoa(SrvAddr.sin_addr),
			SwapShortMT((u_short)SrvAddr.sin_port));
		PutDebugLog(DEBUG_MASK_PROG, Buffer);
	}

	ResetCommand(FD_CONNECT);
	QCWinSockLibMT::WSAAsyncSelect(m_Socket, m_hWnd, USER_WINSOCK_MESSAGE, FD_CONNECT);
	ConnectStatus = QCWinSockLibMT::connect(m_Socket, (sockaddr *)&SrvAddr, sizeof(SrvAddr));

	if (ConnectStatus == SOCKET_ERROR)
	{
		ConnectStatus = QCWinSockLibMT::WSAGetLastError();

		if (ConnectStatus == WSAEWOULDBLOCK)
			ConnectStatus = MessageWait(FD_CONNECT);
		if (ConnectStatus != ALL_OK)
		{
			// Failed to conenct, so remove the port and address caches
			s_PortList.Remove(Machine);
			s_AddressList.Remove(Machine);

			wsprintf(Buffer, CRString(IDS_ERR_SERVER_CONNECT), Machine);
			return (SetError(Buffer, ConnectStatus));
		}
	}
	CancelCommand();

	s_OpenConnectionCount++;
	m_bNeedsConnectionCountDecrement = true;
	
	AfxGetMainWnd()->PostMessage(msgConnectionWasMade);

	QCWinSockLibMT::WSAAsyncSelect(m_Socket, m_hWnd, USER_WINSOCK_MESSAGE, FD_READ | FD_WRITE | FD_CLOSE);

	//m_dwTimeoutLength = GetIniLong(IDS_INI_NETWORK_TIMEOUT);
	m_dwTimeoutLength = m_Settings->GetNetworkTimeout();

	return (ALL_OK);
}


int QCWorkerSocket::SendData(const char* buf, int length)
{
    int res = ALL_OK;
    if (IsSSLMode() && m_pSSLReference && m_pSSLReference->m_pSSL)
	{
		int nValid = length;
		int err ;
		if ((err = g_fnQCSSLWrite((void*)buf, &nValid, m_pSSLReference->m_pSSL)) == 0)
			res = ALL_OK;
		else
			res = -1;//this logic needs to be modified return exact value of "res" error.
		CTaskInfoMT *pTaskInfo = (CTaskInfoMT*) m_Settings->GetTaskInfo();
	    if(pTaskInfo)
		   pTaskInfo->SetSSL(SSL_NORMAL);
	}
	else
		res = SendDataRaw(buf, length);

	PutDebugLog(DEBUG_MASK_TRANS, buf, length);

	return res;
}




int QCWorkerSocket::SendDataRaw(const char* buf, int length)
{
	int res = ALL_OK;
	int ByteCnt;

	if (m_bEscapePressed)
	{
		m_bAbortConnection = true;
		return (ESCAPED);
	}

	FlushLogGetChBuffer();

	while (length)
	{
		ResetCommand(FD_WRITE);
		ASSERT(m_Socket != INVALID_SOCKET);
		ByteCnt = QCWinSockLibMT::send(m_Socket, buf, length, 0);
		CancelCommand();

		if (ByteCnt == SOCKET_ERROR)
		{
			if ((res = QCWinSockLibMT::WSAGetLastError()) == WSAEWOULDBLOCK)
			{
				res = MessageWait(FD_WRITE);
				CancelCommand();

				if (res >= 0)
					continue;
			}

			m_bAbortConnection = true;
			return (WriteError(res));
		}

		length -= ByteCnt;
		buf += ByteCnt;
	}

	return (res);
}


// PutDirect
// This routine takes a buffer and a length and writes it directly onto
// the network.  It bypasses the normal buffering process.
//
int QCWorkerSocket::PutDirect(const char* buf /*= NULL*/, int length /*= -1*/)
{
	if (m_bAbortConnection)
		return (-1);
	    
	if (!buf)
		return (1);	   

	if (length < 0)
		length = strlen(buf);

	if(m_bSSLMode && !m_bQCSSLInit)
		InitializeQCSSL();
	
	if (m_bAbortConnection)
		return (-1);

	return (SendData(buf, length));
}

// Write
// Writes the data in the internal buffer to the network
//
int QCWorkerSocket::Write()
{
	int res;
	if(m_bSSLMode && !m_bQCSSLInit)
		InitializeQCSSL();
	if (m_bAbortConnection)
		return (-1);
	res = SendData(m_pBuffer, m_nValidBytes);
	
	m_nValidBytes = 0;
	m_pBufPtr = m_pBuffer;

	return (res);
}



// Read
// Read data into our internal buffer
int QCWorkerSocket::Read()
{
	int res = ALL_OK;
	if(m_bSSLMode && !m_bQCSSLInit)
		InitializeQCSSL();
	if (m_bAbortConnection)
		return (-1);

	if (IsSSLMode() && m_pSSLReference && m_pSSLReference->m_pSSL)
	{
		int nValid = 0;
		nValid = m_nBufferSize;
		int err ;
		if ((err = g_fnQCSSLRead(m_pSSLReference->m_pSSL, &nValid, (void*)m_pBuffer)) == 0)
		{
			res = ALL_OK;
		}
		else
			res = -1;//this logic needs to be modified return exact value of "res" error.
	   CTaskInfoMT *pTaskInfo = (CTaskInfoMT*) m_Settings->GetTaskInfo();
	   if(pTaskInfo)
		   pTaskInfo->SetSSL(SSL_NORMAL);

		if (nValid <= 0)
		{
			m_nValidBytes = 0;
			return -1;
		}
		else
			m_nValidBytes = nValid;
	}
	else
	{
		res = ReadRaw(m_pBuffer, m_nBufferSize);
		if (res <= 0)
		{
			m_nValidBytes = 0;
			return -1;
		}
		m_nValidBytes = res;
	}

	m_pBufPtr = m_pBuffer;
	return m_nValidBytes;

}   


//
int QCWorkerSocket::ReadRaw(char* pBuffer, int nBufferSize)
{
	int ByteCnt =0;
	int res = 0;
	int nValidBytes =0;

	*pBuffer = 0;
	    
	if (m_bAbortConnection || m_bEscapePressed)
		return (-1);

	while (1)
	{

		ResetCommand(FD_READ);
		ASSERT(m_Socket != INVALID_SOCKET);
		ByteCnt = QCWinSockLibMT::recv(m_Socket, pBuffer, nBufferSize, 0);
		CancelCommand();
		// Getting zero from recv() means the socket closed, but we
		// are expecting to get some data so this is an error.
		if (ByteCnt == 0)
			return (-1);
		if (ByteCnt == SOCKET_ERROR)
		{
			if ((res = QCWinSockLibMT::WSAGetLastError()) == WSAEWOULDBLOCK)
			{
				res = MessageWait(FD_READ);
				CancelCommand();
		
				if (res >= 0)
					continue;
			}

			m_bAbortConnection = true;
			return (ReadError(res));
		}
		
		break;
	}

	nValidBytes = ByteCnt;
	
	return (nValidBytes);
}   



////////////////////////////////////////////////
//dddddddddddddddddddddddddddddddddddddddddddddd
////////////////////////////////////////////////


const char *pszAsyncClassName = "EudoraAsyncClass";
const long TimeToWait = 10;	// 5 seconds.

//Static Public
//Called when DLL is detaching to kill the asyncthread and release the resources
void QCWorkerSocket::RemoveAsyncThread()
{
	if(m_dwAsyncThreadID)
		::PostThreadMessage(m_dwAsyncThreadID, USER_TERMINATE_THREAD, 0, 0);

	//Just to be sure
	m_dwAsyncThreadID = 0;

	//delete the thread which closes the handle
	delete m_pWinThread;
	m_hAsyncThread = NULL;
}

bool QCWorkerSocket::CreateAsyncWindow()
{
	
	// Create the thread that will create the window.
	//
	if (m_hWnd)
	{
		ASSERT (0);
		return false;
	}


	if(!m_hAsyncThread)
	{
		CSingleLock lock(&m_Guard, TRUE);
		if(!m_hAsyncThread)
		{
			/*m_hAsyncThread = CreateThread (NULL,
										   0, 
										   AsyncThreadFunc,
										   NULL,
										   CREATE_SUSPENDED,
										   &m_dwAsyncThreadID);*/
			m_pWinThread = AfxBeginThread(AsyncThreadFunc, NULL, 
								THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
			if(!m_pWinThread)
				return false;

			m_pWinThread->m_bAutoDelete = FALSE;
			
			m_hAsyncThread = m_pWinThread->m_hThread;
			m_dwAsyncThreadID = m_pWinThread->m_nThreadID;

			
			if (!m_hAsyncThread)
			{
				m_dwAsyncThreadID = 0;
				return false;
			}

			// Start the thread.
			// NOTE: should check return code here.
			//
			//ResumeThread (m_hAsyncThread);
			m_pWinThread->ResumeThread();
			
		}
	}


	m_hEvent = CreateEvent(NULL,			// Security
							  TRUE,			// Manual
							  0,			// Non-signaled on creation
							  NULL);	// Name of object

	if(!m_hEvent) 
		return false;

	//Lets give the other thread a chance to start and ready to go..
	Sleep(0);
	//First time, if thread is not ready PostThreadMessage will fail...go until we are able to post the message
	while( ::PostThreadMessage(m_dwAsyncThreadID, USER_CREATE_ASYNC_WINDOW, (WPARAM)&m_hWnd, (LPARAM)m_hEvent) == 0)
	{
		Sleep(0);
		if(GetLastError() != ERROR_INVALID_THREAD_ID)  //ERROR_INVALID_THREAD_ID expected if thread hasn't yet created the msg Q
		{
			return false;
		}
	}
	
	::WaitForSingleObject(m_hEvent, INFINITE);
	ResetEvent(m_hEvent);

	if(!m_hWnd)
		return false;

	// Set this object as the window data.
	//
	SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this);

	return true;
}


LRESULT CALLBACK GlobalWindowProc(  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!hwnd)
		return -1;

	QCWorkerSocket *pNetSock = (QCWorkerSocket *) GetWindowLong (hwnd, GWL_USERDATA);

	if (!pNetSock)
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	// Pass the buck to the object.

	return pNetSock->WindowProc (uMsg, wParam, lParam);
}



HWND CreateAsyncWindow();
// Thread function called when the Asynchronous Thread is created.
//
UINT AsyncThreadFunc (LPVOID)
{
	
	MSG msg;

	//The following peekmessage is to make sure this thread creates a message queue
	::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while (1)
	{
		
		::GetMessage(&msg, NULL, 0, 0);

		switch (msg.message)
		{
		case USER_TERMINATE_THREAD:
			//Thread is being killed..
			return 0;
            break;

		case USER_CREATE_ASYNC_WINDOW:
			{
				HWND *pWnd = NULL;
				pWnd = (HWND *)(msg.wParam);
				*pWnd = (HWND)::CreateAsyncWindow();
			
				::SetEvent( (HANDLE)msg.lParam );
			}
			break;
		
		case USER_DESTROY_ASYNC_WINDOW:
			{
				::DestroyWindow((HWND)msg.wParam);
				HANDLE hEvent = (HANDLE)msg.lParam;
				if(hEvent)
					::SetEvent(hEvent);
			}
			
        default:
			break;
		}
        
		::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
	}

	return 0;
}


HWND CreateAsyncWindow()
{
	WNDCLASS wndClass;
	
	// Register our class if we didn't already.
	//
	if ( !GetClassInfo( AfxGetInstanceHandle(),
					   pszAsyncClassName,
					   &wndClass) )
	{
		// We have to register it.
		//
		memset ( (void *)&wndClass, 0, sizeof (WNDCLASS) );

		wndClass.style			= 0;
		wndClass.lpfnWndProc	= (WNDPROC) GlobalWindowProc;
		wndClass.cbClsExtra		= 0;
		// Space for an EVENT handle + (WORD) Expect.
		//
		wndClass.cbWndExtra		= sizeof (HANDLE) + sizeof (LONG);
		wndClass.hInstance		= AfxGetInstanceHandle();
		wndClass.lpszClassName	= pszAsyncClassName;

		if ( !RegisterClass( &wndClass ) )
			return NULL;
	}

	HWND hWnd;
	// allow modification of several common create parameters
	CREATESTRUCT cs;
	cs.dwExStyle = 0;
	cs.lpszClass = pszAsyncClassName;
	cs.lpszName = _T(" ");
	cs.style = WS_OVERLAPPED;
	cs.x = 0;
	cs.y = 0;
	cs.cx = 0;
	cs.cy = 0;
	cs.hwndParent = AfxGetMainWnd()->m_hWnd;
	cs.hMenu = NULL;
	cs.hInstance = AfxGetInstanceHandle();
	cs.lpCreateParams = NULL;

	hWnd = ::CreateWindowEx(cs.dwExStyle, cs.lpszClass,
			cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
			cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

	if (hWnd == NULL)
	{
		TRACE1("Warning: Window creation failed: GetLastError returns 0x%8.8X\n",
				GetLastError());
		ASSERT(0);
	}

	return hWnd;
}


int GetIndex(int event)
{
	int index = 0;
	if(event == FD_CONNECT)
		index = iCONNECT;
	else if(event == FD_READ)
		index = iREAD;
	else if(event == FD_WRITE)
		index = iWRITE;
	else if(event == FD_CLOSE)
		index = iCLOSE;
	else index = iOTHER;
	
	return index;
}


void QCWorkerSocket::ResetCommand(int EventType /*= 0*/)
{ 
	m_ErrorValue = 0;
	
	m_AsyncEvent[GetIndex(EventType)] = 0;
	
	m_Expect = EventType;
	if(EventType == 0)
		m_Expect = 200;

	//TRACE("%d ResetCommand issued Expect %d\n", m_hWnd, m_Expect);
	::ResetEvent(m_hEvent);
}


	




LRESULT QCWorkerSocket::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case USER_WINSOCK_MESSAGE:
		if (wParam == (WPARAM)m_Socket)
		{
			int AsyncEvent = WSAGETSELECTEVENT(lParam);
			m_ErrorValue = WSAGETSELECTERROR(lParam);
			//TRACE("%d AsyncEvent = %u, ErrorValue = %u\n", m_hWnd, AsyncEvent, m_ErrorValue);
			
			if (AsyncEvent == 0)
				return 0;

			if(m_Expect == AsyncEvent)
			{
				int index = GetIndex(AsyncEvent);
				m_AsyncEvent[index] = AsyncEvent;

				// If the window signaling data corresponds to this message, signal the
				// event.
				SetEvent(m_hEvent);
			}

			if(AsyncEvent == FD_CLOSE)
			{
				SetEvent(m_hEvent);
				m_AsyncEvent[iCLOSE] = AsyncEvent;
				TRACE("FD_CLOSE received with err %d\n", m_ErrorValue);

				//Unexpected close. 
				if( m_Settings && m_Settings->ClosedByPeer )
					m_Settings->ClosedByPeer(m_ErrorValue);
			}
			
			return 0;
		}
		
		break;
	case USER_GETSERV_MESSAGE:
	case USER_GETHOSTNAME_MESSAGE:
	case USER_CONNECT_MESSAGE:
	case USER_GETHOSTADDR_MESSAGE:	
		//if (wParam == (WPARAM)m_Socket)
		{
			m_AsyncEvent[iOTHER] = message;
			m_ErrorValue = WSAGETSELECTERROR(lParam);
			//TRACE("%d AsyncEvent = %u, ErrorValue = %u\n", m_hWnd, m_AsyncEvent[iOTHER], m_ErrorValue);
		
			// If the window signaling data corresponds to this message, signal the
			// event.
			ASSERT(message != 0);
			if(m_Expect == 200)
				SetEvent(m_hEvent);				
			return 0;
		}
		break;
	default:
		break;	// This should never happen.  This routine should be
			// entered only when we are expecting a Winsock message
	}

	return ::DefWindowProc(m_hWnd, message, wParam, lParam);

}



// Event handled version:
//
int QCWorkerSocket::MessageWait(WORD Expect)
{
	int res = -1;
	DWORD	nWaitCount;
	//HANDLE	hWaitArray [1];
	MSG		msg;

	//TRACE("%d Waiting for event %d\n", m_hWnd, Expect);
	
	// Initialize:
	//
	nWaitCount = 1;
	//hWaitArray[0] = m_hEvent;

	BOOL bDone = FALSE;
	
	// Note: We have to handle timeout within the peek message loop as well!
	//
	LONG tEnd = time(0) + m_dwTimeoutLength;
	int nTimeOut;
	
	int EventIndex = GetIndex(Expect);
	
	while (!bDone)
	{
		// Simply wait on the event.
		// NOTE: Use MsgWaitForMultipleObjects because this may be run in the main
		// thread.
		//
		nTimeOut = tEnd - time(0);
		if(nTimeOut <= 0)  //Our time is over, quit with a time_out failure
		{
			res = -1;
			m_bTimerExpired = true;
			break;
		}

		DWORD dwRes = MsgWaitForMultipleObjects ( nWaitCount,
												  &m_hEvent,  //hWaitArray,		// 
												  FALSE,			// Wake when any one object
																	//  is signaled.
												  nTimeOut * 1000,	// milliseconds,
												  QS_ALLINPUT);		// All user messages.

		// 
		if (dwRes == WAIT_OBJECT_0 + nWaitCount)
		{
			// Pump message.
			while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
				
				::TranslateMessage(&msg);

				if (msg.message == WM_QUIT || (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE))
				{
					bDone = TRUE;
					m_bEscapePressed = TRUE;
					res = -1;

					// Don't swallow the escape
					if (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE)
						::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);

					break;
				}

                ::DispatchMessage(&msg);
            }
			
		}
		else if ( (dwRes >= WAIT_OBJECT_0) && (dwRes < WAIT_OBJECT_0 + nWaitCount) )
		{
			// We succeeded:
			//res =  ALL_OK;

			if (m_AsyncEvent[EventIndex] == Expect){
				res = m_ErrorValue;  	//return (m_ErrorValue);
				//ResetEvent (m_hEvent);
				break;
			}
			
			//If an unexpected close is received or User pressed Escape, we are outofhere
			if (m_AsyncEvent[iCLOSE] == FD_CLOSE)
			{
				/*if( m_Settings && m_Settings->ClosedByPeer )
					m_Settings->ClosedByPeer(m_ErrorValue);*/

				//Unexpected CLOSE received.
				res = UNEXPECTED_CLOSE;
				break;
			}
			
			if(m_bEscapePressed)
			{
				res = -1;
				break;
			}

			TRACE( "%d Assert \n", m_hWnd);
			ASSERT(0);
			
			//bDone = TRUE;
		}
		else if (dwRes == WAIT_TIMEOUT)
		{
			// Ask user:??
			//if (USER_TimeoutQuery () )
			{
				res = -1;
				m_bTimerExpired = true;
				bDone = TRUE;
				break;
			}
			// Otherwise, reset event and continue waiting.

			//ResetEvent (m_hEvent);
		}
		else
		{
			// Some grievous error that we can't handle.
			//
			res = -1;
			bDone = TRUE;
			break;
		}
	}

	m_Expect = 0;
	return res;
}

long QCWorkerSocket::SocketReadCallback(void*, int, const char *pcBuf, int iLen, long lArg, long)
{
	if (!lArg)
	{
		return 0;
	}

	QCSSLReference *pSSLReference = (QCSSLReference *)lArg;
	QCWorkerSocket *pWorkerSocket = (QCWorkerSocket *)pSSLReference->m_pIORef;

	int		 iRet = 0;
	int		 iTotalRead = 0;

	do
	{
		iRet = pWorkerSocket->ReadRaw(((char*)pcBuf) + iTotalRead, iLen - iTotalRead);
		if (iRet < 0)
		{
			return -1;
		}

		iTotalRead += iRet;
		
		if (iRet == 0)
		{
			break;
		}
	} while (iTotalRead != iLen);

    if (iTotalRead <= 0)
	{
		return 0;
	}

	// If logging all bytes read, log the read portions of the handshake in hex format for debugging purposes.
	if (!g_bSSLHandshakeDone && QCLogFileMT::DebugMaskSet(DEBUG_MASK_RCV))
	{
		char			 szStr[256];
		char			 szSubStr[4];
		unsigned char	 c = '\0';
		szStr[0] = 0;
		for (int i = 0; i < iTotalRead; ++i)
		{
			c = *(pcBuf + i);
			sprintf(szSubStr, " %02X", c);
			strcat(szStr, szSubStr);
			if (((i + 1) % 16) == 0)
			{
				PutDebugLog(DEBUG_MASK_RCV, szStr);
				szStr[0] = '\0';
			}
		}
		PutDebugLog(DEBUG_MASK_RCV, szStr);
	}

	return iTotalRead;
}

long QCWorkerSocket::SocketWriteCallback(void*, int, const char *pcBuf, int iLen, long lArg, long)
{
	if (!lArg)
	{
		return 0;
	}

	QCSSLReference *pSSLReference = (QCSSLReference *)lArg;
	QCWorkerSocket *pWorkerSocket = (QCWorkerSocket *)pSSLReference->m_pIORef;

	// If logging all bytes sent, log the write portions of the handshake in hex format for debugging purposes.
	if (!g_bSSLHandshakeDone && QCLogFileMT::DebugMaskSet(DEBUG_MASK_TRANS))
	{
		char			 szStr[256];
		char			 szSubStr[4];
		unsigned char	 c = '\0';
		szStr[0] = 0;
		for (int i = 0; i < iLen; ++i)
		{
			c = *(pcBuf + i);
			sprintf(szSubStr, " %02X", c);
			strcat(szStr, szSubStr);
			if (((i + 1) % 16) == 0)
			{
				PutDebugLog(DEBUG_MASK_TRANS, szStr);
				szStr[0] = '\0';
			}
		}
		PutDebugLog(DEBUG_MASK_TRANS, szStr);
	}

	if (pWorkerSocket->SendDataRaw(pcBuf, iLen) != ALL_OK)
	{
		return 0;
	}

	return iLen;
}

bool QCWorkerSocket::InitializeQCSSL()
{
	g_bSSLHandshakeDone = false;

	if(m_bSSLMode && !m_bQCSSLInit)
	{
		bool bSuccess = g_fnQCSSLBeginSession(m_pSSLReference);
		CTaskInfoMT *pTaskInfo = (CTaskInfoMT*) m_Settings->GetTaskInfo();
		if(pTaskInfo)
		{
			if(bSuccess)
				pTaskInfo->SetSSL(SSL_NORMAL);
			else
			{
				pTaskInfo->SetSSL(SSL_FAIL);
		//		m_bSSLMode = FALSE;
				ConnectionInfo *pConnectionInfo = NULL;
				FPNQCSSLGetConnectionInfo fnConnInfo =NULL;
				fnConnInfo = Network::GetQCSSLGetConnectionInfo();
				pConnectionInfo= fnConnInfo(m_pSSLReference->m_Persona,m_pSSLReference->m_ProtocolInfo.m_ProtocolName);
				CString csError;
				csError = CRString(IDS_ERR_SSL_NEGOTIATION);
				int errorCode = -1;
				if(pConnectionInfo)
				{
					csError += pConnectionInfo->m_Outcome.m_Errors;
					errorCode = pConnectionInfo->m_Outcome.m_ErrorCode;
				}

				// Pass the SSL negotiation data to the Task Info object.
				pTaskInfo->SetSSLError(errorCode);
				if (pConnectionInfo->m_Outcome.m_bCertRejected)
				{
					pTaskInfo->SetSSLCertText(pConnectionInfo->m_Outcome.m_strCertText);
					pTaskInfo->SetSSLCert(pConnectionInfo->m_Outcome.m_pCertData);
					pTaskInfo->SetSSLCertRejection(pConnectionInfo->m_Outcome.m_strCertRejection);
				}

				SetError(csError, errorCode);
				m_bAbortConnection = TRUE;
			}
		}
		else if (!bSuccess)
		{
			// There are operations (for example, IMAP mailbox tree refreshing) that happen in the
			// main thread and thus have no TaskInfo object.  In these cases, we still need a way to
			// communicate information about SSL errors back to the user so we cache the relevant
			// data in this object so we can retrieve it at an appropriate time. -dwiggins
			ConnectionInfo				*pConnectionInfo = NULL;
			FPNQCSSLGetConnectionInfo	 fnConnInfo = NULL;
			fnConnInfo = Network::GetQCSSLGetConnectionInfo();
			pConnectionInfo = fnConnInfo(m_pSSLReference->m_Persona,
										 m_pSSLReference->m_ProtocolInfo.m_ProtocolName);
			m_iSSLError = pConnectionInfo->m_Outcome.m_ErrorCode;
			if (pConnectionInfo->m_Outcome.m_bCertRejected)
			{
				m_strSSLCertText = pConnectionInfo->m_Outcome.m_strCertText;
				m_pCertData = pConnectionInfo->m_Outcome.m_pCertData;
				m_strSSLCertRejection = pConnectionInfo->m_Outcome.m_strCertRejection;
			}
		}
			
		m_bQCSSLInit = TRUE;
		g_bSSLHandshakeDone = true;
	}
	return m_bQCSSLInit;
}


bool QCWorkerSocket::UpdateSSLProgress(int nCode, const char* message, void* ref)
{
  if (!ref)
	{
	  ASSERT(FALSE);
	  return false;
	}
  
  QCWorkerSocket *pSocket = (QCWorkerSocket*)ref;
  
  if(nCode >=0)
	  pSocket->m_Settings->Progress(message);
  else
	  pSocket->m_Settings->ReportError(message, TERR_WINSOCK);
 
  return true;
}

void QCWorkerSocket::SetSSLMode(bool bVal,  CString person, SSLSettings *pSettings, CString protocol)

{
	Network::SetSSLMode(bVal);
	if(!m_pSSLReference)
		m_pSSLReference = DEBUG_NEW QCSSLReference;

	m_pSSLReference->m_Persona = person;
	m_pSSLReference->m_ProtocolInfo.m_ProtocolName = protocol;
	m_pSSLReference->m_ProtocolInfo.m_Port = m_Port;
	m_pSSLReference->m_ProtocolInfo.m_IPAddress = m_ServerIP;
	m_pSSLReference->m_ProtocolInfo.m_ServerName = m_ServerName;
	m_pSSLReference->m_CertificateInfo.m_RootCertStoreDir= pSettings->m_InputCertsDir;
	m_pSSLReference->m_CertificateInfo.m_UserCertStoreDir=pSettings->m_InputCertsDir;

	m_pSSLReference->m_SSLLogSession = 0;

	m_pSSLReference->m_fnQCSSLWriteCallback = SocketWriteCallback;
	m_pSSLReference->m_fnQCSSLReadCallback = SocketReadCallback;
	m_pSSLReference->m_fnUpdateSSLProgressCallback = UpdateSSLProgress;
	
	m_pSSLReference->m_pIORef = this;
	m_pSSLReference->m_pSurrenderRef = NULL;
	m_pSSLReference->m_pUpdateProgressRef = this;
	m_pSSLReference->m_pConnectionManagerInfo = NULL;
	m_pSSLReference->m_pSSL = NULL;

	if(pSettings)
	{
		if(protocol == "SMTP")
		{
			if (pSettings->m_nSSLSendUsage == SSLSettings::SSLUseAlternatePort)
			{
				m_pSSLReference->m_ProtocolInfo.m_ProtocolVersion = pSettings->m_nSSLAltPortSendVersion;
			}
			else
			{
				m_pSSLReference->m_ProtocolInfo.m_ProtocolVersion = pSettings->m_nSSLSendVersion;
			}
			m_pSSLReference->m_CertificateInfo.m_bIgnoreExpired
									= pSettings->m_nSSLSendIgnoreExpiredCerts;
			m_pSSLReference->m_ProtocolInfo.m_AllowSSL_2_0 = pSettings->m_nSSLSendAllowVersion2 ;

			m_pSSLReference->m_SSLLogSession = pSettings->m_SSLSendLogSession;
			m_pSSLReference->m_ProtocolInfo.m_AllowRegExp = pSettings->m_SSLSendAllowRegExp;
			m_pSSLReference->m_ProtocolInfo.m_CipherSuite = pSettings->m_SSLSendCipherSuite;
			m_pSSLReference->m_ProtocolInfo.m_MinEncryption = pSettings->m_SSLSendMinEncryption;
			m_pSSLReference->m_ProtocolInfo.m_MinKeyExchangeLength = pSettings->m_SSLSendMinKeyExchangeLength;
			m_pSSLReference->m_CertificateInfo.m_csPrivateKey = pSettings->m_SSLSendPrivateKey;
			m_pSSLReference->m_CertificateInfo.m_csPassPhrase= pSettings->m_SSLSendPrivateKeyPassword;
		}
		else
		{
			if (pSettings->m_nSSLReceiveUsage == SSLSettings::SSLUseAlternatePort)
			{
				m_pSSLReference->m_ProtocolInfo.m_ProtocolVersion = pSettings->m_nSSLAltPortReceiveVersion;
			}
			else
			{
				m_pSSLReference->m_ProtocolInfo.m_ProtocolVersion = pSettings->m_nSSLSendVersion;
			}
			m_pSSLReference->m_CertificateInfo.m_bIgnoreExpired
									= pSettings->m_nSSLReceiveIgnoreExpiredCerts;
			m_pSSLReference->m_ProtocolInfo.m_AllowSSL_2_0 = pSettings->m_nSSLReceiveAllowVersion2;
			m_pSSLReference->m_SSLLogSession = pSettings->m_SSLReceiveLogSession;
			m_pSSLReference->m_ProtocolInfo.m_AllowRegExp = pSettings->m_SSLReceiveAllowRegExp;
			m_pSSLReference->m_ProtocolInfo.m_CipherSuite = pSettings->m_SSLReceiveCipherSuite;
			m_pSSLReference->m_ProtocolInfo.m_MinEncryption = pSettings->m_SSLReceiveMinEncryption;
			m_pSSLReference->m_ProtocolInfo.m_MinKeyExchangeLength = pSettings->m_SSLReceiveMinKeyExchangeLength;
			m_pSSLReference->m_CertificateInfo.m_csPrivateKey = pSettings->m_SSLReceivePrivateKey;
			m_pSSLReference->m_CertificateInfo.m_csPassPhrase= pSettings->m_SSLReceivePrivateKeyPassword;

		}
	}
}

