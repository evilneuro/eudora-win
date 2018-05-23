// Network.cpp - implementation of the CNetStream class.


#include "stdafx.h"

#include "MyTypes.h"

// From Eudora's QCSocket library:
#include "QCWorkerSocket.h"
#include "QCSockCallbacks.h"

// This is from Eudora - NASTY!!!! Should be in QCSocket!!!
//
#include "QCNetSettings.h"

// Whenever imapmail.h and imap.h are included together, define _IMAPDLL_INTERNAL
//#define _IMAPDLL_INTERNAL_
//#include "imapmail.h"
//#include "imap.h"

#include "exports.h"
#include "ImapDefs.h"
#include "mm.h"
#include "Proto.h"		// Our class defs.

#include "osdep.h"
#include <time.h>
#include "resource.h"
#include "Network.h"
#include "SSLSettings.h"

#include "DebugNewHelpers.h"


// Maximum length of a line returned by the server. 
#define MAX_LINE_LENGTH	8192

// Set by main program. Initially zero.
extern UINT g_umsgPreviewMessage;

// static callback function pointers.
extern BOOL (__cdecl *g_pTimeoutQuery)	();

// USe a message box to display notice to the user.
void USER_Warn (UINT StringID, ...);


// ====================== CNetStream class ======================================//

CNetStream::CNetStream()
{
	m_szHost.Empty();
	m_Port				= 0;

	m_pNetSettings		= NULL;

	// Contains bet value - destroy in destructor.
	m_pNetConnection	= NULL;
};


CNetStream::~CNetStream()
{
	// Make sure the connection is properly closed.
	if (m_pNetConnection)
	{
		NetClose();

		delete m_pNetConnection;
	}
}



void CNetStream::SetNetSettings (QCNetworkSettings* pNetSettings)
{
	m_pNetSettings = pNetSettings;

	if (m_pNetSettings)
	{
		// Don't change these during the life of this object!!
		//
		m_pNetSettings->SetCloseCallback	(
				(makeCallback( (Callback1<int> *)0, *this, &CNetStream::CloseCallback) ) );

		m_pNetSettings->SetReportErrorCallback	(
				makeCallback( (QCNetworkSettings::ErrorCallback *)0, *this, &CNetStream::SocketErrorCallback) );
	}
}



// CloseCallback [PUBLIC]
//
//
void CNetStream::CloseCallback (int ErrorValue)
{
	ErrorValue = ErrorValue;

	// Just need to notify the stream which will set appropriate flags.
	//
	NetNotify (NETSTREAM_CLOSED, NULL);
}


// [PRIVATE]
//
// This is the callback that's passed to the QCSocket object. It must be set
// into the CNetSettings object on creation of our (CNetStream) object AND MUST NOT
// BE RESET.
// We handle access to the real error callback that's protected by a mutex.
//

void CNetStream::SocketErrorCallback (LPCSTR str, UINT ErrorType)
{
	// Just call the pure virtual function (must be overridden), NetReportError(), on the stream.
	//
	NetReportError (str, ErrorType);
}



// NetOpen [PUBLIC]
//
// Given the machine name, and the port, it opens
// the port and sets up the socket.
//

BOOL CNetStream::NetOpen(const char* Machine, const char* Service, UINT ServiceID, int Port, const char * loginname)
{
	BOOL bResult = FALSE;

	// This is no used. Note: We should get rid of "ServiceID", see below.
	UNREFERENCED_PARAMETER( Service );


	// Must have a host.
	if (!Machine)
	{
		ASSERT (0);
		return FALSE;
	}

	// Must have a QCNetworkSettings!
	//
	if (!m_pNetSettings)
	{
		ASSERT (0);
		return FALSE;
	}
	
	
	// Copy parms:
	// 
	m_szHost	= Machine;
	m_Port	= Port;

	// Port as a string:
	//
	m_szPort.Format ("%d", m_Port);

	// Create the contained QCWorkerSocket object.
	// Note: the MAILSTREAM contains the network settings.
	//

	m_pNetConnection = DEBUG_NEW_NOTHROW QCWorkerSocket ( m_pNetSettings );

	if (!m_pNetConnection)
	{
		return FALSE;
	}
	// Start Hesiod //

	if(m_szHost.CompareNoCase("hesiod") ==0 && SUCCEEDED(QCHesiodLibMT::LoadHesiodLibrary()))
	{
		CString LoginName(loginname);
		int At = LoginName.ReverseFind('@');
		if (At > 0)
			LoginName.ReleaseBuffer(At);
		int nHesiodError = -1;
		char szHostFromHesiod[128]={0};

		if (FAILED(QCHesiodLibMT::GetHesiodServer(CRString(IDS_IMAP_SERVICE), LoginName, (char*)szHostFromHesiod, sizeof(szHostFromHesiod), &nHesiodError)))
		{
		QCHesiodLibMT::FreeHesiodLibrary();
		return FALSE;
		}

		QCHesiodLibMT::FreeHesiodLibrary();

		if (szHostFromHesiod[0]) m_szHost = szHostFromHesiod;
	}
	// End Hesiod /
	
	// Open the connection to the server.
	//
	if (m_pNetConnection->Open(m_szHost, ServiceID, Port, Port, TRUE) < 0)
	{
		// We failed. Cleanup.
		delete m_pNetConnection;
	
		m_pNetConnection = NULL;

		return FALSE;
	}
		
	// OK. We got it.
	//
	bResult = TRUE;

	return (bResult);
}



// Close
// Closes the connection
//
int CNetStream::NetClose()
{
	int status = -1;

	if (m_pNetConnection)
	{
		status = m_pNetConnection->Close();
	}

	return (status);
}



void CNetStream::RequestStop()
{
	if (m_pNetConnection)
		m_pNetConnection->RequestStop();
}


void CNetStream::GetCanonicalHostname (CString& szHostName)
{ 
	if (!m_pNetConnection)
	{
		szHostName = GetMachineName();
	}
	else
	{
		// Must have net settings.
		//
		if (m_pNetSettings)
		{
			// QCSocket only get the real hostname if kerberos is set.
			//
			if (m_pNetSettings->DoAuthenticateKerberos())
				szHostName = m_pNetConnection->GetMachineName ();
			else
				szHostName = GetMachineName();
		}
	}
}



// GetLine
// FUNCTION
// Read a line of text into an allocated buffer and return the allocated buffer.
// END FUNCTION

char* CNetStream::Getline ()
{
	char *ret = NULL;

	// Must have a connection.
	//
	if (!m_pNetConnection)
		return NULL;

	if ( m_pNetConnection->Aborted() )
	{
		NetNotify (NETSTREAM_CLOSED, NULL);

		// Get out right away.
		//
		return NULL;
	}

	// Allocate the returned buffer. Note: This is returned and must
	// be freed by caller.
	//
	long		IncBlocksize	= MAX_LINE_LENGTH;
	long	CurBlocksize	= IncBlocksize;

	ret = (char *)fs_get (CurBlocksize); 
	if (!ret)
		return NULL;

	// Initialize.
	*ret = 0;

	// Read until we find an end-of-line (\r\n)!!
	//
	int		ErrorStatus = -1;

	// Length of returned line.
	//
	long	Length = 0;
	BOOL	bSucceeded = FALSE;
	BOOL	CRFound = FALSE;
	char	c;

	while (1)
	{
		// Get the line from QCSocket. Note: QCWorkerSocket::GetLine returns the number of 
		// bytes read, or -1 if error.
		//
		ErrorStatus = m_pNetConnection->GetCh (&c);

		if (ErrorStatus < 0)
			break;

		if (ErrorStatus == 0)
			continue;

		// Do we need to allocate a larger buffer??
		//
		if (Length >= CurBlocksize)
		{
			fs_resize ((void **)&ret, CurBlocksize + IncBlocksize);
			CurBlocksize += IncBlocksize;
		}

		// Note: Size we may resize, don't use a moving pointer into "ret"!
		//
		ret[Length++] = c;

		// Is this \r?
		if (c == '\r')
		{
			CRFound = TRUE;
		}
		else if (c == '\n')
		{
			// A line MUST end in \r\n, otherwise reject the response.
			// Cannot have a \n by itself.
			//
			if (CRFound)
			{
				bSucceeded = TRUE;
			}

			// reject a \n by itself!
			//
			break;
		}
		// A \r MUST be followed by a \n.
		else if (CRFound)
		{
			break;
		}
	}

	// Must have at least \r\n in the buffer:
	//
	if (bSucceeded && Length >= 2)
	{
		// Remove the \r\n.
		ret[Length - 2] = '\0';
	}
	else
	{
		fs_give ( (void **)&ret);
		ret = NULL;
	}

	return ret;
}



// Getbuffer [PUBLIC]
//
// Fills "buffer" with "size" bytes from the network.
// returns TRUE if successful.
//
BOOL CNetStream::Getbuffer (unsigned long size, char *buffer)
{


	char *bufptr = buffer;

	// Sanity:
	if (! (buffer && (size > 0)) )
		return FALSE;

	// Initialize:
	*buffer = 0;

	// Must have a connection:
	//
	if (!m_pNetConnection)
		return FALSE;

	if ( m_pNetConnection->Aborted() )
	{
		NetNotify (NETSTREAM_CLOSED, NULL);

		// Get out right away.
		//
		return FALSE;
	}

	//
	char c;
	int ErrorStatus;

	/* until request satisfied */
	while (size > 0)
	{
		ErrorStatus = m_pNetConnection->GetCh (&c);

		if(ErrorStatus == 0)
			continue;

		if (ErrorStatus < 0)
		{
			*buffer = 0;

			// Did the stream close??
			//
			if ( m_pNetConnection->Aborted() )
			{
				NetNotify (NETSTREAM_CLOSED, NULL);
			}

			return FALSE;
		}

		// Add to buffer:
		*bufptr++ = c;

		size--;
	}

	bufptr[0] = '\0';		/* tie off string */

	return TRUE;
}



// 
BOOL CNetStream::NetSoutr (char *string)
{
	return NetSout (string, (unsigned long) strlen (string));
}


// Similar to the original SendData.
BOOL CNetStream::NetSout(const char* buf, unsigned long length)
{
	BOOL	bResult = TRUE;

	// Sanity.
	if (NULL == buf || length <= 0)
		return FALSE;

	// Must have a connection:
	//
	if (!m_pNetConnection)
		return FALSE;

	if ( m_pNetConnection->Aborted() )
	{
		NetNotify (NETSTREAM_CLOSED, NULL);

		// Get out right away.
		//
		return FALSE;
	}

	bResult = (m_pNetConnection->PutDirect(buf, length) >= 0);

	if (!bResult)
	{
		// Did the stream close??
		//
		if ( m_pNetConnection->Aborted() )
		{
			NetNotify (NETSTREAM_CLOSED, NULL);
		}
	}

// Log that we sent this stuff:
#ifdef _DEBUG
	char *pPP = DEBUG_NEW_NOTHROW char [length + 4];
	if (pPP)
	{
		strncpy (pPP, buf, length);
		pPP [length] = 0;
		NetLog (pPP, 0);
		delete [] pPP;
	}
#endif // _DEBUG 

	return (bResult);
}




/* TCP/IP abort stream
 * Accepts: WinSock SOCKET
 */

BOOL CNetStream::Abort ()
{
	NetClose();

	return TRUE;
}



// User requested stop, or stream killed by server.
//
BOOL CNetStream::UserAborted ()
{
	// Must write!!
	//
	if (!m_pNetConnection)
		return TRUE;

	return m_pNetConnection->Aborted();
}

bool CNetStream::SetSSLMode(bool bVal, const CString& persona, SSLSettings* pSSLSettings, const CString& protocol )
{
	if(m_pNetConnection)
		m_pNetConnection->SetSSLMode(bVal, persona, pSSLSettings, protocol);
	return m_pNetConnection!=NULL;
}



//
// Popup a dialog box warning the user of some terrible error, such as 
// that the network was disconnected.
// Uses a ::MessageBox call.
//.
void USER_Warn (UINT StringID, ...)
{
	TCHAR buf [1024];
	
	buf[0] = '\0';

	// Get the arguments.
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString (StringID), args);
	va_end(args);

	// Put up a warning message box.
	::MessageBox (NULL, buf, CRString (IDS_IMAP_ERROR), 
				MB_TASKMODAL | MB_ICONWARNING | MB_OK);
}


