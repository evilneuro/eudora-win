// ImapConnection.cpp
//
// CImapConnection class and its helper classes and functions.
// This class serves as the connection point between the EuImap
// and ImapDll projects.
//
// Note that the CImapConnection used to be called (unhelpfully)
// CIMAP.  This class was renamed and given its own module.
//
// Copyright (c) 1997-2003 by QUALCOMM, Incorporated
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

#ifdef IMAP4 // Only for IMAP.

#include "ImapConnection.h"		// Includes imapdll's imap.h

#include "QCNetSettings.h"

#include "rs.h"
#include "mainfrm.h"
#include "guiutils.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "progress.h"
#include "QCTaskManager.h"
#include "TaskInfo.h"
#include "persona.h"
#include "SearchCriteria.h"

#include "imap.h"
#include "ImapStream.h"
#include "ImapExports.h"
#include "ImapTypes.h"

#include "imapgets.h"
#include "ImapMailbox.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapLoginDlg.h"
#include "mime.h"
#include "searchutil.h"


//	Defines and other Constants	=============================
#include "DebugNewHelpers.h"


//	Internals	=================================================
// Passed to imap.dll
BOOL  __TimeoutQuery ();


// If we want the command to wait:
static const DWORD dwWaitTimeout	= 20000;		// Milliseconds.

// If we want the command not to wait.
static const DWORD dwNoWaitTimeout	= 5000;			// Milliseconds.


// ======================= static functions ========================/
void rfc822_write_address (char *dest,MAILADDRESS *adr);
void rfc822_address (char *dest,MAILADDRESS *adr);
void rfc822_address (char *dest,MAILADDRESS *adr);
void rfc822_cat (char *dest,char *src,const char *specials);

static void NetworkNotificationHandler (unsigned long ulData, unsigned short NotifierType, unsigned long ulValue);
static NetworkNotifierType	ConvertToNotifierType (unsigned short uNotifierType);
static void DummyCallback (const char *buf);
void DummyErrorFunc(const char *, TaskErrorType);


// ========================================================================//



// Specialized synchronization object for a CImapConnection object.
//
class CCIMapLock
{

public:
	CCIMapLock (CImapConnection* pImap);
	CCIMapLock::~CCIMapLock();

	BOOL	LockNoWait (ImapErrorFunc pErrFn = NULL);

private:
	BOOL	m_bGotALock;

	// By reference only.
	CImapConnection*	m_pImap;

};



CCIMapLock::CCIMapLock (CImapConnection* pImap)
{
	m_pImap = pImap;

	m_bGotALock = FALSE;
};


CCIMapLock::~CCIMapLock()
{
	if (m_bGotALock)
	{
		if (m_pImap)
		{
			m_pImap->ResetErrorCallback ();

			m_pImap->Unlock();
		}

		m_bGotALock = FALSE;
	}
}


// LockNoWait [PUBLIC]
//
// Call the m_pImap object to determine if we can get a ock on it.
// The CImapConnection object will determine if we 'er executing in the same thread 
// as the client that's got the lock, and it may allow us to proceed.
//
BOOL CCIMapLock::LockNoWait (ImapErrorFunc pErrFn /* = NULL */)
{
	if (!m_pImap)
		return FALSE;

	// We must call the CImapConnection do lock itselt for us.
	//
	m_bGotALock = m_pImap->LockNoWait();

	if (m_bGotALock)
	{
		if (pErrFn)
			m_pImap->SetErrorCallback(pErrFn);
		else
			m_pImap->ResetErrorCallback();
	}

	return m_bGotALock;
}




//================================================================================//

class CNetworkNotifier
{
public:
	CNetworkNotifier ();
	~CNetworkNotifier ();

	void SetNotification (NetworkNotifierType Type,	unsigned long ulValue);

	// Reset to default vallues.
	//
	void ResetNotifications	();

	// Get the value of a particular notification type.
	//
	long GetNotification (NetworkNotifierType Type);
		
	
private:
	// THese are set to TRUE if a change in status occurred.
	// Default to FALSE;
	//
	BOOL	m_bStopRequested;
	BOOL	m_bSetReadOnlyStatus;
	BOOL	m_bSetReadWriteStatus;
	BOOL	m_bPasswordInvalid;
};


CNetworkNotifier::CNetworkNotifier ()
{
	ResetNotifications();
}


CNetworkNotifier::~CNetworkNotifier ()
{
}


// Reset to default values.
// 
void CNetworkNotifier::ResetNotifications ()
{
	m_bStopRequested		= FALSE;
	m_bSetReadOnlyStatus	= FALSE;
	m_bSetReadWriteStatus	= FALSE;
	m_bPasswordInvalid		= FALSE;
}


// Fetch the current value.
//
long CNetworkNotifier::GetNotification (NetworkNotifierType Type)
{
	switch (Type)
	{
		case NETNOTIFY_STOP_REQUESTED:

			return m_bStopRequested;

			break;

		case NETNOTIFY_SET_READONLY_STATUS:
			return m_bSetReadOnlyStatus;
			break;

		case NETNOTIFY_SET_READWRITE_STATUS:
			return m_bSetReadWriteStatus;

			break;

		case NETNOTIFY_INVALID_PASSWORD:
			return m_bPasswordInvalid;
			break;

	}

	// ?????
	return 0;
}


// Cache a particular notification value.
//
void CNetworkNotifier::SetNotification (NetworkNotifierType Type,	unsigned long ulValue)
{
	switch (Type)
	{
		case NETNOTIFY_STOP_REQUESTED:

			m_bStopRequested = (BOOL) ulValue;

			break;

		case NETNOTIFY_SET_READONLY_STATUS:
			m_bSetReadOnlyStatus = (BOOL) ulValue;

			break;

		case NETNOTIFY_SET_READWRITE_STATUS:
			m_bSetReadWriteStatus = (BOOL) ulValue;

			break;

		case NETNOTIFY_INVALID_PASSWORD:
			m_bPasswordInvalid = TRUE;

	}
}




//	================================================================================
//	================================================================================
//	================================================================================
//		IMAP Mail Service Object
//	================================================================================
//	================================================================================
//	================================================================================

//	================================================================================
//////////////////////////////////////////////////////////////////////////////
// CImapConnection


// Note: We should always create a CImapConnection in the main thread.
// We can use it in a worker thread, but instantiate it in the main
// thread first.
//

CImapConnection::CImapConnection()
{
	m_Server.Empty();

	Init();
}


// Constructor that passes in the server name.
CImapConnection::CImapConnection(const char *server) : m_Server (server)
{
	Init();
}

// PRIVATE:
// MAIN THREAD
//
void CImapConnection::Init ()
{
	// JOK
	m_pImapStream = NULL;
	m_Server.Empty();
	m_PortNum = "143";

	// These must correspond, otherwise a grievous error has occured.
	m_AccountID = 0;
	m_pAccount	= NULL;

	// These are contained interfaces:
	//
	m_pNetSettings		= NULL;
	m_pNetworkNotifier	= DEBUG_NEW CNetworkNotifier;


	// Synchronization state stuff:
	//
	m_pNegotiateLockable = DEBUG_NEW CCriticalSection;

	m_curThreadID		= 0;
	m_lReEnterRefCount	= 0;

	//
	m_bTooBusy			= FALSE;

	// Only one thread can start a command at a time.
	//
	m_hThreadLockable = CreateMutex(NULL,	// Security
							  FALSE,		// Initial owner
							  NULL);	// Name of object

	m_szKrbLibName = GetIniString (IDS_INI_IMAP_KRB4_DLLNAME);
	m_szDesLibName = GetIniString (IDS_INI_IMAP_DES_DLLNAME);
	m_szGssLibName = GetIniString (IDS_INI_GSS_DLLNAME);


	m_bImapStatisticsForInboxOnly = GetIniShort(IDS_INI_IMAP_STATISTICS_FOR_INBOX_ONLY);

	// Construct our alert callback:
	//
	m_pAlertCallback = makeCallback( (QCNetworkSettings::ErrorCallback *)0, *this, &CImapConnection::AlertCallback);

	m_pTaskInfo = NULL;
}



CImapConnection::~CImapConnection()
{

	// Close the stream
	Close();

	if (m_pNetSettings)
		delete m_pNetSettings;

	m_pNetSettings = NULL;

	if (m_pNetworkNotifier)
		delete m_pNetworkNotifier;

	if(m_hThreadLockable)
		CloseHandle(m_hThreadLockable);


	delete m_pNegotiateLockable;
}


// Close [PUBLIC]
//
// NOTE: This destroys the IMAP Stream!!
//
HRESULT CImapConnection::Close()
{
	// Closes the IMAP STREAM regardless.
	if (m_pImapStream)
	{
		// Disable the main application's frame while closing the connection.
		CMainFrame* pMainFrame = (CMainFrame *) ::AfxGetMainWnd();

		if (pMainFrame)
			pMainFrame->EnableWindow (FALSE);

		delete m_pImapStream;

		m_pImapStream = NULL;

		// Re-enable it.
		if (pMainFrame)
			pMainFrame->EnableWindow (TRUE);
	}

	// No mailbox name any longer associated with this.
	m_szImapName.Empty();

	return S_OK;
}


void CImapConnection::SetErrorCallback (ImapErrorFunc func)
{
	if (m_pImapStream && func)
	{
		m_pImapStream->SetErrorCallback	(func);
	}
}


void CImapConnection::SetAlertCallback (ImapErrorFunc func)
{
	if (m_pImapStream && func)
	{
		m_pImapStream->SetAlertCallback	(func);
	}
}


void CImapConnection::ResetErrorCallback ()
{
	if (m_pImapStream)
		m_pImapStream->ResetErrorCallback	();
}



void CImapConnection::ResetProgressCallback ()
{
	if (m_pNetSettings)
		m_pNetSettings->SetProgressCallback	(
			(makeCallback( (Callback1<LPCSTR> *)0, DummyCallback) ) );
}


// RequestStop [PUBLIC]
//
void CImapConnection::RequestStop()
{
	if (m_pImapStream)
		m_pImapStream->RequestStop();
}


// =========== Synchronization methods =============//

// LockNoWait [PUBLIC]
//
// Attempt to obtain permission to use this object.
// Algorithm:
// If we're already locked and we're in the same thread as whoever's
// got the lock, determine if we can proceed.
//
// If there's no lock on the object, just try to get one.
//
BOOL CImapConnection::LockNoWait(BOOL bAllowReEnter /* = FALSE */)
{
	if (!m_hThreadLockable)
		return FALSE;

	// If the current thread already has the lock on the connection object,
	// we can act on it because no other thread can get the lock until we're
	// done with it.
	// 

	DWORD threadID = ::GetCurrentThreadId();

	if (threadID == m_curThreadID)
	{
		// OK, We can do what we like.
		//
		m_lReEnterRefCount++;

		return TRUE;
	}

	// Ok, It's either no thread or another thread. We can safely attempt to 
	// lock the object.
	//

	BOOL bResult =  __LockNoWait();

	return bResult;
}


// Unlock [PUBLIC]
//
// Anyone who calls CImapConnection::LockNoWait() MUST call this when they're done!!!
//
// Note: An assumption we can make here is that the current thread MUST have
// had the lock, so we don't need to synchronize access to m_curThreadID.
//
void CImapConnection::Unlock()
{
	if (!m_hThreadLockable)
		return;

	// The current thread MUST have the lock!!
	//
	if ( !IsLocked() )
	{
		ASSERT (0);
		return;
	}

	DWORD threadID = ::GetCurrentThreadId();

	// we must be in the same thread if we're trying to unlock.
	//
	if (threadID != m_curThreadID)
	{
		ASSERT (0);
		return;
	}

	// Because we're in the same thread as the guy who's got
	// the lock, we can do this stuff here without worrying that he's
	// going to preempt us.
	//
	m_lReEnterRefCount--;

	ASSERT (m_lReEnterRefCount >= 0);

	// we had the lock if m_lReEnterRefCount is zero!!
	//
	if (m_lReEnterRefCount == 0)
	{
		// release the main lock.
		//

		m_curThreadID = 0;

		ReleaseMutex (m_hThreadLockable);
	}
}


// IsLocked [PRIVATE]
//

BOOL CImapConnection::IsLocked()
{
	return	(m_curThreadID != 0);
}






// __LockNoWait [PRIVATE]
//
// Attempt to get a a lock on the main synchronization object.
// This is guaranteed to be called in a thread other than the one that's
// currently got the lock (in any);
//
BOOL CImapConnection::__LockNoWait()
{
	if (!m_hThreadLockable)
		return FALSE;

	// Try to lock it.
	//
	DWORD dwRes;

	// Wait on the mutex:
	//
	dwRes = WaitForSingleObject (m_hThreadLockable, dwNoWaitTimeout);

	BOOL bTest = TRUE;

	if (WAIT_TIMEOUT == dwRes)
	{
		bTest = FALSE;
		return FALSE;
	}
	else if (WAIT_ABANDONED == dwRes)
	{
		bTest = FALSE;
		return FALSE;
	}
	else if (WAIT_FAILED == dwRes)
	{
		bTest = FALSE;
		return FALSE;
	}

	// Otherwise, we're got the lock.

	// If we got the lock, update local status:
	//
	m_curThreadID = GetCurrentThreadId();

	m_lReEnterRefCount++;

	// Should be exactky 1 here.
	//
	ASSERT (m_lReEnterRefCount == 1);

	return TRUE;
}
	


//========= Maintenence methods. ========================//

void DummyErrorFunc(const char *, TaskErrorType)
{
	//Dummy
}


// Create or re-fetch network settings:
// This should be called in the main thread!
//
// MAIN THREAD
//
void CImapConnection::UpdateNetworkSettings ()
{
	ASSERT ( ::IsMainThreadMT() );

	// Must have either an accountID or an account.
	//
	if (!m_pAccount)
		m_pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);

	if (!m_pAccount)
	{
		ASSERT (0);
		return;
	}

	// Get the persona.
	//
	CString szPersona; m_pAccount->GetName (szPersona);

	// The only way we can grab new network settings is bt re-creating m_pNetSettings.
	// BUT, we may have already set this pointer value into an opened IMAP stream, so
	// we can't delete it. We need a way to update the values without deleting 
	// m_pNetSettings
	//
	if (!m_pNetSettings)
	{
		m_pNetSettings = DEBUG_NEW_NOTHROW QCNetworkSettings (szPersona);

	}

	// set dummy default callbacks. THese can be overridden later.
	//
	if (m_pNetSettings)
	{
		ResetErrorCallback ();

		// Set the net settings callback to a dummy function. We don't really use it because 
		// it's just the QCSocket that calls it.
		//
		m_pNetSettings->SetReportErrorCallback( 
			(makeCallback( (Callback2<LPCSTR, TaskErrorType> *)0, DummyErrorFunc) ) );
       
		ResetProgressCallback ();
	}
}



// ================================================================================
// OpenMailbox [PUBLIC] - open IMAP mailbox.
//
// This calls an internal method to do the real work. Here, we do
// the negotiation for the lock on this object.
//
// NOTES
// NOTE: TO open a mailbox we MUST already have an owning account established.
// END NOTES

HRESULT CImapConnection::OpenMailbox (const char *Fullname, BOOL bSilent /* = FALSE */, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Flag that we're too busy to handle any more open requests.
	//
	SetTooBusy(TRUE);

	// If we're not selected but we have a CImapStream, we need to 
	// get rid of that and create a new one.
	//
	if ( !IsSelected() && (NULL != m_pImapStream) ) 
	{
		delete m_pImapStream;
		m_pImapStream = NULL;
	}

	// Otherwise, we got the lock.
	// Do the real work.
	//
	HRESULT hResult = __OpenMailbox (Fullname, bSilent);

	// On success tell the account about the server's namespace.
	if (SUCCEEDED(hResult))
	{
		if (HasNAMESPACE())
		{
			CString		 strNameSpace;
			GetNameSpace(strNameSpace);
			m_pAccount->SetNameSpace(strNameSpace);
		}
	}

	// If the login failed, we need to remove any saved password from the INI file.
	//
	if ( !SUCCEEDED (hResult) )
	{
		TCHAR buf[512];
		buf[0] = 0;

		//
		// If the user cancelled, don't show error message.
		//
		int iType = IMAPERR_BAD_CODE;

		GetLastImapErrorAndType (buf, 510, &iType);

		if (iType != IMAPERR_USER_CANCELLED)
		{
			if (IsMainThreadMT())
				ShowLastImapError (this);
			else if (pErrFn && *buf)
			{
				(pErrFn) (buf, TERR_AUTH);
			}
		}
		else
		{
			// Signal caller that user cancelled:
			//
			hResult = HRESULT_MAKE_CANCEL;
		}

		// Did we fail beause the login failed?
		//
		if (m_pAccount && HRESULT_CONTAINS_LOGON_FAILURE(hResult) )
		{
			m_pAccount->InvalidatePasswordMT (TRUE);
		}
	}
//	else	// Must SetTooBusy(FALSE) no matter if we succeeded or not -jdboyd
		//No need for this anymore as we changed the way passwords are saved
//	if ( !m_pAccount->IsAuthKerberos() )
//	{
//		// We succeeded. We've got a valid pasword.
//		//
//		m_pAccount->ValidatePasswordMT();
//	}

	// Ok. We can handle more requests.
	//
	SetTooBusy(FALSE);

	return hResult;
}


// Create a half-open stream to the server in m_Server (which must be non-empty).
//
// Note: pAccount CAN be NULL.!!
//
HRESULT CImapConnection::OpenControlStream(CImapAccount *pAccount /* = NULL */, BOOL bSilent /* = FALSE */, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Flag that we're too busy to handle any more open requests.
	//
	SetTooBusy(TRUE);

	// If we're not connected but we have a CImapStream, we need to 
	// get rid of that and create a new one.
	//
	if ( !IsConnected() && (NULL != m_pImapStream) ) 
	{
		delete m_pImapStream;
		m_pImapStream = NULL;
	}

	// Otherwise, we got the lock.
	// Do the real work.
	//
	HRESULT hResult = __OpenControlStream (pAccount, bSilent);

	// On success tell the account about the server's namespace.
	if (SUCCEEDED(hResult))
	{
		if (HasNAMESPACE())
		{
			CString		 strNameSpace;
			GetNameSpace(strNameSpace);
			pAccount->SetNameSpace(strNameSpace);
		}
	}

	// Put up error messages if we failed.
	if (!SUCCEEDED (hResult) )
	{
		TCHAR buf[512];
		buf[0] = 0;

		//
		// If the user cancelled, don't show error message.
		//
		int iType = IMAPERR_USER_CANCELLED;

		GetLastImapErrorAndType (buf, 510, &iType);

		if (iType != IMAPERR_USER_CANCELLED)
		{
			// The failure was something other than a user cancellation.  First check if it was an
			// SSL cert failure in the main thread.  If SSL failed and there was no CTaskInfoMT object
			// associated with the network connection then we know the connection was in the main thread
			// and the following error values have been set.  If the connection was in a worker thread
			// then these values will be unset and we proceed with the normal error reporting.  See
			// QCWorkerSocket::InitializeQCSSL() for further explanation on this. -dwiggins
			int			 iSSLErr = m_pImapStream->GetSSLError();
			void		*pCertData = m_pImapStream->GetSSLCert();

			if ((iSSLErr > 0) && pCertData)
			{
				// The failure was due to the server cert being rejected.  Ask the user if they want to
				// trust the cert.
				CString				strRejection;
				CString				strCertText;
				m_pImapStream->GetSSLCertRejection(strRejection);
				m_pImapStream->GetSSLCertText(strCertText);
				CTrustCertDlg		trustCertDlg(NULL, strCertText, strRejection);
				if (trustCertDlg.DoModal() == IDOK)
				{
					m_pImapStream->AddTrustedCert(pCertData);
				}
				// Clear all of the ImapStream's SSL cert data fields since we no longer need them.
				m_pImapStream->ClearSSLCertData();
			}
			else
			{
				// The failure was something other than a cert failure.  Do the usual error reporting.
				if ( IsMainThreadMT() )
				{
					ShowLastImapError (this);
				}
				else if (pErrFn && *buf)
				{
					(pErrFn) (buf, TERR_AUTH);
				}
			}
		}
		else
		{
			// Signal caller that user cancelled:
			//
			hResult = HRESULT_MAKE_CANCEL;
		}

		// Did we fail beause the login failed?
		//
		if (m_pAccount && HRESULT_CONTAINS_LOGON_FAILURE(hResult) )
		{
			m_pAccount->InvalidatePasswordMT (TRUE);
		}
	}
//	else	// must SetTooBusy(FALSE) no matter if we succeeded or not -jdboyd
	//No need for this anymore as we changed the way passwords are saved		
//	if (m_pAccount && !m_pAccount->IsAuthKerberos() )
//	{
//		// We succeeded. We've got a valid pasword.
//		//
//		m_pAccount->ValidatePasswordMT();
//	}

	// Ok. We can handle more requests.
	//
	SetTooBusy(FALSE);

	return hResult;
}




// FUNCTION
// Do a NOOP on the mailbox stream
// END FUNCTION
HRESULT CImapConnection::Noop (ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call the imapdll function
	if (m_pImapStream)
		return m_pImapStream->Noop();
	else
		return E_FAIL;
}



// FUNCTION
// Do a CHECK on the mailbox stream
// END FUNCTION
HRESULT CImapConnection::Check (ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call the imapdll function
	if (m_pImapStream)
		return m_pImapStream->Check ();

	return E_FAIL;
}



// FUNCTION
// Get the message count from the stream
// END FUNCTION
HRESULT CImapConnection::GetMessageCount(unsigned long& ulCount, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = S_OK;

	ulCount = 0;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call the imapdll function
	if (m_pImapStream)
	{
		ulCount = m_pImapStream->GetMessageCount (); 

		hResult = S_OK;
	}
	else
	{
		hResult = E_FAIL;
	}

	return hResult;
}





// ================================================================================


// CopyMessages [PUBLIC]
// FUNCTION
// Copy messages to another mailbox on the same server.
// END FUNCTION
// NOTES
// Uidlist is an IMAP uid message set, eg: "23:45,56,58".
// END NOTES.

HRESULT CImapConnection::CopyMessages (const char *pUidlist, const char *pDestMailbox, CDWordArray *dwaNewUIDs, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = E_FAIL;

	// Sanity
	if (!(pUidlist && pDestMailbox))
		return E_INVALIDARG;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call the imapdll function
	if (m_pImapStream)
	{
		hResult = m_pImapStream->UIDCopy (pUidlist, pDestMailbox, dwaNewUIDs);
	}

	return hResult;
}



//
// FUNCTION
// Append the message represented by the STRING msg to the remote mailbox.
// The remote mailbox must have already been SELECT'ed.
// END FUNCTION
 
HRESULT CImapConnection::AppendMessage ( IMAPFLAGS* pImapFlags, unsigned long Seconds, STRING *pMsg, CDWordArray *dwaNewUIDs, ImapErrorFunc pErrFn /* = NULL */ )
{
	// Sanity.
	if (!(pImapFlags && pMsg))
		return E_INVALIDARG;

	// Must have an open stream.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	
	// Format flags character flags.
	CString Flags;

	// Avoid lots of memory allocs:
	//
	CRString seen (IDS_IMAP_SEEN_STRING_FLAG);
	CRString deleted (IDS_IMAP_DELETED_STRING_FLAG);
	CRString flagged (IDS_IMAP_FLAGGED_STRING_FLAG);
	CRString answered (IDS_IMAP_ANSWERED_STRING_FLAG);
	CRString draft (IDS_IMAP_DRAFT_STRING_FLAG);

	int size = seen.GetLength() + 
			   deleted.GetLength() +
			   flagged.GetLength() +
			   answered.GetLength() +
			   draft.GetLength();

	LPTSTR p = Flags.GetBuffer (size + 16);
	if (p)
	{
		*p = 0;
		Flags.ReleaseBuffer(-1);
	}

	Flags = "(";

	BOOL bIsFirst = TRUE;

	if (pImapFlags->SEEN)
	{
		if (!bIsFirst)
			Flags += " ";

		Flags += seen;

		bIsFirst = FALSE;
	}

	if (pImapFlags->DELETED)
	{
		if (!bIsFirst)
			Flags += " ";

		Flags += deleted;

		bIsFirst = FALSE;
	}

	if (pImapFlags->FLAGGED)
	{
		if (!bIsFirst)
			Flags += " ";

		Flags += flagged;

		bIsFirst = FALSE;
	}


	if (pImapFlags->ANSWERED)
	{
		if (!bIsFirst)
			Flags += " ";

		Flags += answered;

		bIsFirst = FALSE;
	}


	if (pImapFlags->DRAFT)
	{
		if (!bIsFirst)
			Flags += " ";

		Flags += draft;

		bIsFirst = FALSE;
	}

	Flags += ")";

	// Send the command.

	return m_pImapStream->AppendMessage ((LPCSTR)Flags, Seconds, pMsg, dwaNewUIDs);
}



// NOTES
// Make sure that the server name and port number info in the CImapConnection class is the
// same as the corresponding info in the pAccount class. Anytime the account is "set",
// that info gets copied from the account into m_Server and m_PortNum members of
// CImapConnection.
// Note that we can set a NULL account into CImapConnection. 
// END NOTES
void CImapConnection::SetAccount (CImapAccount *pAccount)
{
	m_pAccount = pAccount;	// Can be NULL.

	// Copy server machine info.
	if (pAccount)
	{
		m_pAccount->GetAddress(m_Server);
		m_pAccount->GetPortNum(m_PortNum);
	}
}




// 
// FetchHeaderFull
// Fetch the header string for this message from c-client.
//
HRESULT	CImapConnection::UIDFetchHeaderFull (IMAPUID uid, CWriter *pWriter, ImapErrorFunc pErrFn /* = NULL */)
{
	if (!(uid && pWriter))
		return E_INVALIDARG;

	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}
	// Call UIDFetchHeader with the Writer callback class..
	return m_pImapStream->UIDFetchHeader (uid, pWriter);
}


// FUNCTION
// Fetch the given header fields as text.
// END FUNCTION

HRESULT CImapConnection::UIDFetchRFC822HeaderFields(IMAPUID uid,
													char *szSequence,
													char *szSection,
													char *Fields,
													CWriter *pWriter,
													ImapErrorFunc pErrFn /*=NULL*/)
{
	if ((!uid && !szSequence) || !pWriter)
		return E_INVALIDARG;

	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call UIDFetchHeader with the Writer callback class.
	return m_pImapStream->UIDFetchRFC822HeaderFields (uid, szSequence, szSection, Fields, pWriter);
}




// UIDFetchRFC822Header [PUBLIC]
//
// FUNCTION
// Fetch the header of the given message/rfc822 enbedded mesage.
// NOTE: Can also be used to fetch the header of the top message.
// END FUNCTION

HRESULT CImapConnection::UIDFetchRFC822Header (IMAPUID uid, char *szSection, CWriter *pWriter, ImapErrorFunc pErrFn /* = NULL */)
{
	if (!(uid && pWriter))
		return E_INVALIDARG;

	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call UIDFetchHeader with the Writer callback class.
	return m_pImapStream->UIDFetchRFC822Header (uid, szSection, pWriter);
}




// 
// Return TRUE if this message is a MIME multipart message.
//
HRESULT	CImapConnection::MessageIsMultipart (IMAPUID uid, BOOL *pVal, ImapErrorFunc pErrFn /* = NULL */)
{
	if (!pVal)
		return E_INVALIDARG;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (m_pImapStream)
		return m_pImapStream->UIDMessageIsMultipart (uid, pVal);
	else
		return E_FAIL;
}



// FetchAttachmentContents.
// Read "Filename" and extract the body part szSection from it.
// The file should contain a magic number on the first line and the szSection string
// on the second line.
// If the file does not exist, return FALSE.
// If the file exists but the magic number is not found, assume that the 
// contents have already been fetched.
// Otherwise do the foll:
// - Fetch the body part contents into the specified file, decoding it as we go.
// - Note: The original contents of the file are replaced by the body contents.
// - Return TRUE.
//
HRESULT CImapConnection::FetchAttachmentContents(IMAPUID uid, const char *Filename, ImapErrorFunc pErrFn /* = NULL */)
{
	JJFileMT*			ATFile = NULL;
	char			szSection[1024];
	unsigned long	magicNumber;
	HRESULT			hResult = E_FAIL;
	short			encoding = ENC7BIT;		// Default.

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Let's get the file
	ATFile = DEBUG_NEW_NOTHROW JJFileMT;
	if (!ATFile || !SUCCEEDED (ATFile->Open(Filename, O_CREAT | O_RDWR) ) )
	{
		if (ATFile)
			delete ATFile;
        ATFile = NULL;

		return E_FAIL;
	}

	*szSection = 0;
	ATFile->Get(&magicNumber);
	if (magicNumber == _MAGICNUMBER)
	{
		// Read szSection string now. If no szSection, this is an error.
		if (SUCCEEDED (ATFile->Read (szSection, sizeof(szSection) - 1) ) )
		{
			// truncate and close the file so the streamer can put bytes into it.
			ATFile->Close();

			// Strip \r\n
			char *p = strchr(szSection, '\r');
			if (p)
				*p = 0;
			else if (p = strchr(szSection, '\n'))
				*p = 0;

			// Set decoder for this body type and go out and fetch contents.
			hResult = FetchAttachmentContentsToFile (uid, szSection, Filename, encoding);
		}
	} 
	else 
	{
		hResult = S_OK;		// Assume we already read it.
	}

	// Cleanup.
	if (ATFile)
	{
		if (S_OK == ATFile->IsOpen())
			ATFile->Close();
		delete ATFile;
	}
		
	return hResult;
}


// FUNCTION
// Fetch bytes between the offsets "first" and "last" of the body part.
// Return the length of text obtained in pBuffer in the output parameter: pLen.
// END FUNCTION

// NOTES
// Note: The size of the given buffer should be at lease as big as (last - first + 1);
// Note: No decoding is done. The raw text is returned.
// NOTE: The returned string is NOT nul-terminated.
// END NOTES

HRESULT CImapConnection::UIDFetchPartialContentsToBuffer(unsigned long uid, char *szSection, int first, 
			 unsigned long nBytes, LPTSTR pBuffer, unsigned long nBufferSize, unsigned long *pLen,
			 ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT			hResult = E_FAIL;
	CStringWriter	stringWriter;
	unsigned long   len = 0;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Sanity.
	if (! (pBuffer && pLen) )
		return E_INVALIDARG;

	if (nBytes <= 0)
		return E_INVALIDARG;
	
	// Init:
	*pLen = 0;

	if (m_pImapStream)
	{
		// Fetch the body.
		hResult = m_pImapStream->UIDFetchPartialBodyText(uid, szSection, first, nBytes, &stringWriter, TRUE);

		// Copy text into given buffer.
		if ( SUCCEEDED (hResult) )
		{
			len = stringWriter.GetCurrentSize ();

			if (len <= nBufferSize)
			{
				strcpy (pBuffer, stringWriter.GetString ());
			}
			else
			{
				// Overflow. Shouldn't happen.
				ASSERT (0);
				len = 0;
			}
		}
	}

	// Output: 
	*pLen = len;

	return hResult;
}



// Fetch contents of body, decode and stream bytes into "Filename".
HRESULT CImapConnection::DownloadAttachmentContents (IMAPUID uid, char *szSection, CWriter *pWriter, ImapErrorFunc pErrFn /* = NULL */)
{

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Sanity
	if (!(pWriter && m_pImapStream))
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// Fetch the body.
	return m_pImapStream->UIDFetchBodyText(uid, szSection, pWriter, TRUE);
}



// Fetch contents of body, decode and stream bytes into "Filename".
HRESULT CImapConnection::FetchAttachmentContentsToFile (IMAPUID uid, char *szSection, const char *Filename, short encoding,
				 LPCSTR pSubtype /* = NULL */, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT			hResult = E_FAIL;
	CFileWriter		fileWriter (Filename);

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (m_pImapStream)
	{
		// If we're given a subtype and it's "Applefile",
		// do special things!!
		//
		if (pSubtype && 
				stricmp (pSubtype, CRString (IDS_MIME_APPLEFILE)) == 0)
		{
			fileWriter.SetApplefileDecoder (encoding);
		}
		// Otherwise, set decoder based on "encoding".
		else
		{
			// Setup decoder, etc.
			fileWriter.SetDecoder (encoding);
		}

		// Show progress as well:
		fileWriter.ShowProgress (TRUE);

		// Fetch the body.
		hResult = m_pImapStream->UIDFetchBodyText(uid, szSection, &fileWriter, TRUE);

		// Reset decoder:
		fileWriter.ResetDecoder();
	}

	return hResult;
}



// Fetch the MIME header for this section
// jdboyd 8/24/99
HRESULT CImapConnection::FetchMIMEHeader (IMAPUID uid, char *szSection, CWriter *pWriter, ImapErrorFunc pErrFn)
{
	HRESULT			hResult = E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (m_pImapStream)
	{
		// Fetch the MIME header.
		hResult = m_pImapStream->UIDFetchMimeHeader(uid, szSection, pWriter);
	}

	return hResult;
}



HRESULT	CImapConnection::FetchHeaderValue (const char *name, char *pValue, ImapErrorFunc pErrFn /* = NULL */)
{

	return E_FAIL;
}


// Fetch our list of subscribed IMAP mailboxes from "server". Return the
// list in "mboxlist".
// Note: "server" can be NULL, in which case user the m_server attribute.
void CImapConnection:: MailFetchSubscribed (CImapAccount *pAccount, ImapErrorFunc pErrFn /* = NULL */)
{
	if (!pAccount)
		return;

}




// Fetch our list of un-subscribed IMAP mailboxes from "server". Return the
// list in "mboxlist".
// Note: "server" can be NULL, in which case user the m_server attribute.
// If "INCLUDE"INBOX" is TRUE, Fetch inbox as a separate command.
void CImapConnection:: MailFetchThisLevel (LPCSTR pReference, CImapAccount *pAccount, BOOL INCLUDE_INBOX, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return;
	}

	if (!(pReference && pAccount))
		return;
	
	// If we have a stream, use it, else open temporary one.
	if (!m_pImapStream)
		OpenControlStream(pAccount);

	// Make sure our account parameters are properly set into m_ImapControl.
	SetAccount (pAccount);

	if (m_pImapStream)
	{
		// Create a CMboxLister class to pass to c-client.
		CMboxLister *pLister = DEBUG_NEW_NOTHROW CMboxLister (pAccount);

		if (pLister)
		{
			m_pImapStream->IMAPListUnSubscribed (pReference, pLister, INCLUDE_INBOX);

			// No longer need lister.
			delete pLister;
		}
	}	
}




// Send the IMAP CREATE command to create a mailbox.
HRESULT CImapConnection::CreateMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn /* = NULL */ )
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (!pImapName)
		return E_INVALIDARG;

	// Must have an open stream.
	if ( !m_pImapStream )
		return E_FAIL;

	// Send the create command.
	return m_pImapStream->CreateMailbox ( pImapName );
}



// Send the IMAP DELETE command to remove a mailbox.
HRESULT CImapConnection::DeleteMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn /* = NULL */ )
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (!pImapName)
		return E_INVALIDARG;

	// Must have an open stream.
	if ( !m_pImapStream )
		return E_FAIL;

	// Send the create command.
	return m_pImapStream->DeleteMailbox ( pImapName );
}



// Send the IMAP RENAME command to rename a mailbox.
HRESULT CImapConnection::RenameMailbox ( LPCSTR pImapName, LPCSTR pNewImapName, ImapErrorFunc pErrFn /* = NULL */ )
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (! (pImapName && pNewImapName) )
		return E_INVALIDARG;

	// Must have an open stream.
	if ( !m_pImapStream )
		return E_FAIL;

	// Send the create command.
	return m_pImapStream->RenameMailbox ( pImapName, pNewImapName );
}




// Send the IMAP SUBSCRIBE command to remove a mailbox.
HRESULT CImapConnection::SubscribeMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn /* = NULL */ )
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (!pImapName)
		return E_INVALIDARG;

	// Must have an open stream.
	if ( !m_pImapStream )
		return E_FAIL;

	// Send the create command.
	return m_pImapStream->SubscribeMailbox ( pImapName );
}




// Send the IMAP UNSUBSCRIBE command to remove a mailbox.
HRESULT CImapConnection::UnSubscribeMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn /* = NULL */ )
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (!pImapName)
		return E_INVALIDARG;

	// Must have an open stream.
	if ( !m_pImapStream )
		return E_FAIL;

	// Send the create command.
	return m_pImapStream->UnSubscribeMailbox ( pImapName );
}





// Do a LIST on a single mailbox in order to get it's attributes.
// Return a QCImapMailboxCOmmand object containing the attributes.
ImapMailboxNode* CImapConnection:: FetchMailboxAttributes (LPCSTR pImapName, ImapErrorFunc pErrFn /* = NULL */)
{
	ImapMailboxNode  *pImapNode = NULL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return NULL;
	}

	if (!pImapName)
		return NULL;
	
	// Must have a stream.
	if (!(m_pImapStream && m_pAccount))
		return NULL;

	// Create a CMboxLister class to pass to c-client.
	CImapNodeLister *pLister = DEBUG_NEW_NOTHROW CImapNodeLister (m_pAccount->GetAccountID());

	if (pLister)
	{
		m_pImapStream->FetchMailboxAttributes (pImapName, pLister);

		// Get the first object from pLister.
		if (pLister->m_pTopMailbox)
		{
			// Detach the top node. There should obky be one node anyway.
			pImapNode = pLister->m_pTopMailbox;
			pLister->m_pTopMailbox = pImapNode->SiblingList;  // Should be NULL.
		}

		// No longer need lister.
		delete pLister;
	}	

	// Can be NULL.
	return pImapNode;
}




// ================ Wrappers around the stream functions ====================//

// UIDFetchStructure [PUBLIC]
// NOTES
// This is a wrapper around CImapStream::UIDFetchStructure().
// END NOTES
HRESULT CImapConnection::UIDFetchStructure (IMAPUID uid, BODY **ppBody, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Sanity:
	if (!ppBody)
		return E_INVALIDARG;

	if (!m_pImapStream)
		return E_FAIL;

	return m_pImapStream->UIDFetchStructure (uid, ppBody);
}




// UIDFetchStructure [PUBLIC]
// NOTES
// This is a wrapper around CImapStream::UIDFetchStructure().
// END NOTES

void CImapConnection::FreeBodyStructure	(BODY *pBody, ImapErrorFunc pErrFn /* = NULL */)
{
	if (m_pImapStream)
		m_pImapStream->FreeBodyStructure (pBody);
}




// IsSelected [PUBLIC]
// NOTES
// This is a wrapper around CImapStream::IsSelected().
// END NOTES

BOOL CImapConnection::IsSelected (ImapErrorFunc pErrFn /* = NULL */)
{
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->IsSelected ();
}



// IsConnected [PUBLIC]
// NOTES
// This is a wrapper around CImapStream::IsConnected().
// END NOTES

BOOL CImapConnection::IsConnected (ImapErrorFunc pErrFn /* = NULL */)
{
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->IsConnected ();
}




// IsReadOnly [PUBLIC]
// NOTES
// Ask CImapMailbox if the stream is readonly..
// END NOTES

BOOL CImapConnection::IsReadOnly (ImapErrorFunc pErrFn /* = NULL */)
{
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->IsReadOnly ();
}




// RecreateMessageMap [PUBLIC]
//
void CImapConnection::RecreateMessageMap ()
{
	// Only if selected.
	//
	if ( m_pImapStream && m_pImapStream->IsSelected() )
		m_pImapStream->RecreateMessageMap();
}




// GetUidValidity [PUBLIC]
// NOTES
// This is a wrapper around CImapStream::UIDValidity().
// END NOTES

HRESULT CImapConnection::GetUidValidity (unsigned long& UidValidity, ImapErrorFunc pErrFn /* = NULL */)
{
	// In case we fail.
	//
	UidValidity = 0;
	
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return (unsigned long) (MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY));
	}


	UidValidity = m_pImapStream->UIDValidity ();

	return S_OK;
}





// FindBody
// Given a szSection string, locate corresponding c-client body part.
// Return a pointer to a BODY structure.
// Based on imap_fetchbody() in c-client.
//
// NOTE: Returns an ALLOCATED BODY structure in "ppBody" that must be freed
// by the caller!!
//
HRESULT CImapConnection::FindBody (IMAPUID uid, char *szSection, BODY **ppBody, ImapErrorFunc pErrFn /* = NULL */)
{
	BODY	*body = NULL;
	PART	*part = NULL;
	char	*s;
	unsigned long i;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if (!(ppBody && szSection && *szSection && isdigit (*szSection)))
		return E_INVALIDARG;

	// Init:
	*ppBody = NULL;

	if (!m_pImapStream)
		return E_FAIL;

	// Fetch to body.
	m_pImapStream->UIDFetchStructure (uid, &body);
	if (!body)
		return E_FAIL;

	// Initialize
	s = szSection;

	/* get section number, header if 0 BUT BAD BODY */
	i = strtoul (s,&s,10);

	// Header sections not allowed.
	if (i == 0)
		return E_FAIL;


	// Ok. First section specifier >= 1. Recurse. 
	// 
	do		/* until find desired body part */
	{				
		/* multipart content? */
		if (body->type == TYPEMULTIPART) {
			part = body->nested.part;	/* yes, find desired part */
			while (--i && (part = part->next));
			if (!part)
				return E_FAIL;	/* bad specifier */

			/* note new body, check valid nesting */
			if (((body = &part->body)->type == TYPEMULTIPART) && !*s)
				return E_FAIL;
		}
		else if (i != 1)
			return E_FAIL;		/* otherwise must be section 1 */

				/* need to go down further? */
		i = *s;
		if(i > 0)
		{
			switch (body->type)
			{
				case TYPEMESSAGE:		/* embedded message, calculate new base */
					if (!((*s++ == '.') && isdigit (*s)))
						return E_FAIL;

					/* body section? */
					if (i = strtoul (s,&s,10))
						body = (body->nested.msg)->body;
					else
					{			/* header section - not valid BODY. */
						return E_FAIL;
				    }
					break;

				case TYPEMULTIPART:		/* multipart, get next section */
					if ((*s++ == '.') && isdigit (*s) && (i = strtoul (s,&s,10)) > 0)
						break;

				default:			/* bogus subpart specification */
					return E_FAIL;
			}
		} // switch.
	} while (i);

	// Output:
	*ppBody = body;

	return body ? S_OK : E_FAIL;
}





HRESULT CImapConnection::GetTime (IMAPUID uid, unsigned long *pSeconds)
{
	// Sanity:
	if (!pSeconds)
		return E_INVALIDARG;

	// Initialize:
	*pSeconds = 0;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	return m_pImapStream->UIDGetInternalDate(uid, pSeconds);
}


HRESULT CImapConnection::GetRfc822Size (IMAPUID uid, unsigned long *pSize, ImapErrorFunc pErrFn /* = NULL */)
{
	// Sanity:
	if (!pSize)
		return E_INVALIDARG;

	// Init:
	*pSize = 0;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}


	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	return m_pImapStream->GetRfc822Size(uid, pSize);

}


HRESULT CImapConnection::GetRfc822Size(char *szSequence,
									   unsigned long *pulSizes,
									   int iNumMsgs,
									   ImapErrorFunc pErrFn /* = NULL */)
{
	// Sanity:
	if (!pulSizes)
	{
		return E_INVALIDARG;
	}

	// Init:
	for (int i = 0; i < iNumMsgs; ++i)
	{
		pulSizes[i] = 0;
	}

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}


	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	return m_pImapStream->GetRfc822Size(szSequence, pulSizes, iNumMsgs);
}


HRESULT	CImapConnection::UIDFetchEnvelope (unsigned long uid, ENVELOPE **ppEnvelope, ImapErrorFunc pErrFn /* = NULL */)
{
	// Sanity:
	if (!ppEnvelope)
		return E_INVALIDARG;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}


	// Init:
	*ppEnvelope = NULL;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	return m_pImapStream->UIDFetchEnvelope(uid, ppEnvelope);
}



///////////////////////////////////////////////////////////
// UIDFetchEnvelopeAndExtraHeaders [PUBLIC]
//
// NOTE: pFields and pWriter can be NULL. 
//
///////////////////////////////////////////////////////////
HRESULT	CImapConnection::UIDFetchEnvelopeAndExtraHeaders (
						IMAPUID		Uid, 
						ENVELOPE**	ppEnvelope,
						char*		pFields,
						CWriter*	pWriter,
						ImapErrorFunc pErrFn /* = NULL */)
{
	// Sanity:
	if (!ppEnvelope)
		return E_INVALIDARG;

	// Init:
	*ppEnvelope = NULL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	return m_pImapStream->UIDFetchEnvelopeAndExtraHeaders(
					Uid, ppEnvelope, pFields, pWriter);
}






// Do the UID FETCH 1:* and accumulate the flags into the given CPtrUidList.
HRESULT CImapConnection::FetchAllFlags (CPtrUidList *pUidList, ImapErrorFunc pErrFn /* = NULL */)
{
	return FetchFlags ("1:*", pUidList);
}


// FETCH UID and FLAGS of the given UID sequence.
// Sequence can be of the form: uidfirst:uidlast.
HRESULT CImapConnection::FetchFlags (LPCSTR pSequence, CPtrUidList *pUidList, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT  hResult = E_FAIL;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Must have a UID list object. BUT, pSequence can be NULL!!
	if (!pUidList)
		return E_INVALIDARG;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}


	// Instantiate our ELT writer.
	CImapEltWriter *pEltWriter = DEBUG_NEW_NOTHROW CImapEltWriter (pUidList);
	if (pEltWriter)
	{
		hResult = m_pImapStream->UIDFetchFlags (pSequence, pEltWriter);
		delete pEltWriter;
	}

	return hResult;
}



// Fetch flags into a MAP.
// FETCH UID and FLAGS of the given UID sequence.
// Sequence can be of the form: uidfirst:uidlast.
HRESULT CImapConnection::FetchFlags (LPCSTR pSequence, CUidMap *pUidMap, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT  hResult = E_FAIL;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Must have a UID list object. BUT, pSequence can be NULL!!
	if (!pUidMap)
		return E_INVALIDARG;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Instantiate our ELT writer.
	CImapEltWriterMap *pEltWriter = DEBUG_NEW_NOTHROW CImapEltWriterMap (pUidMap);
	if (pEltWriter)
	{
		hResult = m_pImapStream->UIDFetchFlags (pSequence, pEltWriter);
		delete pEltWriter;
	}

	return hResult;
}




// Wrapper around stream function.
void CImapConnection::FreeMemory (void **pMem)
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return;

	m_pImapStream->FreeMemory (pMem);
}




// Wrapper around stream function.
void CImapConnection::DestroyEnvelope (ENVELOPE **ppEnvelope)
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return;

	m_pImapStream->DestroyEnvelope (ppEnvelope);
}


HRESULT	 CImapConnection::UIDFetchMessage (unsigned long uid, CWriter* pWriter, ImapErrorFunc pErrFn)
{
	if (!pWriter)
		return E_INVALIDARG;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	return m_pImapStream->UIDFetchMessage(uid, pWriter, TRUE);
}



//
// NOTES
// Fetch the UID  given a message number.
HRESULT	CImapConnection::FetchUID (unsigned long msgNum, IMAPUID *pUid, ImapErrorFunc pErrFn /* = NULL */)
{
	// Sanity:
	if (!pUid)
		return E_INVALIDARG;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	return m_pImapStream->FetchUID (msgNum, pUid);
}


// UIDDeleteMessages [PUNLIC]
// FUNCTION
// Flag the gived message UID's as deleted. If Expunge. issue an expuunge.
// END FUNCTION

HRESULT CImapConnection::UIDDeleteMessages (LPCSTR pList, CUidMap& mapUidsActuallyRemoved, BOOL Expunge, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = E_FAIL;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Flag as deleted first before we expunge!!
	// Dunno why this code thinks we can't simply let m_pImapStream->UIDDeleteMessages() do the
	// EXPUNGE but use a temporary UID result list since it won't be used.
	CString		 strTmpList;
	hResult = m_pImapStream->UIDDeleteMessages (pList, strTmpList, Expunge);

	if (SUCCEEDED (hResult) && Expunge)
	{
		CString szUidsActuallyRemoved;

		hResult = m_pImapStream->Expunge (szUidsActuallyRemoved);

		if ( SUCCEEDED (hResult) )
		{
			// Add uid's to the STL MAP output argument.
			//
			CommaListToUidMap (szUidsActuallyRemoved, mapUidsActuallyRemoved);
		}
	}

	return hResult;
}



// UIDUnDeleteMessages [PUNLIC]
// FUNCTION
// Remove the deleted flag from the message UID's.
// END FUNCTION

HRESULT CImapConnection::UIDUnDeleteMessages (LPCSTR pList, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = E_FAIL;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Flag as deleted first before we expunge!!
	hResult = m_pImapStream->UIDUnDeleteMessages (pList);

	return hResult;
}


// UIDExpunge [PUBLIC]
// FUNCTION
// Expunge the remote mailbox.
// END FUNCTION

HRESULT CImapConnection::UIDExpunge (LPCSTR pUidList, CUidMap& mapUidsActuallyRemoved, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// If we don't have a mailbox open, fail.
	if (m_pImapStream)
	{
		CString szUidsActuallyRemoved;

		hResult = m_pImapStream->UIDExpunge (pUidList, szUidsActuallyRemoved);

		if ( SUCCEEDED (hResult) )
		{
			// Add uid's to the STL MAP output argument.
			//
			CommaListToUidMap (szUidsActuallyRemoved, mapUidsActuallyRemoved);
		}
	}

	return hResult;
}



// Expunge [PUBLIC]
// FUNCTION
// Copy messages to another mailbox on the same server.
// END FUNCTION
// NOTES
// Uidlist is an IMAP uid message set, eg: "23:45,56,58".
// END NOTES.

HRESULT CImapConnection::Expunge (CUidMap& mapUidsActuallyRemoved, ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Call the imapdll function.
	if (m_pImapStream)
	{
		CString szUidsActuallyRemoved;

		hResult = m_pImapStream->Expunge (szUidsActuallyRemoved);

		if ( SUCCEEDED (hResult) )
		{
			// Add uid's to the STL MAP output argument.
			//
			CommaListToUidMap (szUidsActuallyRemoved, mapUidsActuallyRemoved);
		}
	}

	return hResult;
}








// GetLastImapError [PUBLIC]
// FUNCTION
// Get the last error string from the IMAP object.
// END FUNCTION

HRESULT CImapConnection::GetLastImapError (LPSTR pBuffer, short nBufferSize)
{
	HRESULT  hResult = E_FAIL;
	int  iType = IMAPERR_BAD_CODE;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	if (NULL == pBuffer || nBufferSize <= 0)
		return E_INVALIDARG; 

	hResult = m_pImapStream->GetLastErrorStringAndType (pBuffer, nBufferSize, &iType);

	return hResult;
}




// GetLastImapError [PUBLIC]
// FUNCTION
// Get the last error string from the IMAP object.
// END FUNCTION

HRESULT CImapConnection::GetLastImapErrorAndType (LPSTR pBuffer, short nBufferSize, int *pType)
{
	HRESULT hResult = E_FAIL;
	int  iType = IMAPERR_BAD_CODE;

	// Sanity:
	if (!pType)
		return E_INVALIDARG;

	// Init:
	*pType = IMAPERR_BAD_CODE;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	if (NULL == pBuffer || nBufferSize <= 0)
		return E_INVALIDARG; 

	hResult = m_pImapStream->GetLastErrorStringAndType (pBuffer, nBufferSize, &iType);

	// Output:
	*pType = iType;

	return hResult;
}



// GetLastServerMessage [PUBLIC]
// FUNCTION
// Get the last error string from the IMAP object.
// END FUNCTION

HRESULT CImapConnection::GetLastServerMessage (LPSTR pBuffer, short nBufferSize)
{
	HRESULT hResult = E_FAIL;
	int iType = IMAPERR_BAD_CODE;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	if (NULL == pBuffer || nBufferSize <= 0)
		return E_INVALIDARG; 

	// Get the last error type and see if it was a server message.
	hResult = m_pImapStream->GetLastErrorStringAndType (pBuffer, nBufferSize, &iType);

	if (iType == IMAPERR_BAD_CODE)
	{
		return E_FAIL;
	}
	// If it's a local error, return the string.
	else if ( iType == IMAPERR_LOCAL_ERROR			|| 
			  iType == IMAPERR_CONNECTION_CLOSED	||
			  iType == IMAPERR_USER_CANCELLED )
	{
		return hResult;
	}
	else
	{
		return m_pImapStream->GetLastServerMessage (pBuffer, nBufferSize);
	}
}




// DoOnServerSearch [PUBLIC]
//
HRESULT CImapConnection::DoOnServerSearch (MultSearchCriteria* pMultiSearchCriteria, CString& szResults, LPCSTR pUidRange /* = NULL */)
{
	BOOL bResult = TRUE;

	if (!m_pImapStream)
		return FALSE;

	// Helper class for MultiSearchCriteria->SEARCHPGM conversion.
	//
	CSearchUtil searchUtil;

	// Will be allocated by CSearchUtil methods.
	SEARCHPGM*	pTopPgm		= NULL;

	// Get the CSearchUtil to convert the MultiSearchCriteria to a SEARCHPGM for us.
	//
	bResult = searchUtil.FormatSearchCriteria (&pTopPgm, pMultiSearchCriteria, pUidRange);

	// Send to IMAP.
	if (bResult && pTopPgm)
		m_pImapStream->Search (NULL, pTopPgm, szResults);

	// Cleanup.
	if (pTopPgm)
	{
		CImapStream::FreeSearchpgm (&pTopPgm);
	}

	return bResult;

}





// UIDFind [PUBLIC]
// 
// FUNCTION
// Search the headers in the comma-separated list for an occurrence of
// pSearchString.
// If bBody os TRUE, search the body also.
// 
// Return the list of UID hits in szResults.
//
// END FUNCTION

// NOTES
// If the mailbox is not selected, fail.
// END NOTES
//

HRESULT CImapConnection::UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
			LPCSTR pUidStr, CString& szResults, ImapErrorFunc pErrFn /* = NULL */)
{
	// In case we fail.
	szResults.Empty();

	// Sanity: If the header list is NULL or empty, bBody must be TRUE.
	if ( ! ( (pHeaderList && *pHeaderList) || bBody) )
		return E_INVALIDARG;

	// pSearchString MUST be valid.
	if (! (pSearchString && *pSearchString) )
		return E_INVALIDARG;

	// If we don't have the mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}


	// Pass it to the CImapStream..
	return m_pImapStream->UIDFind (pHeaderList, bBody, bNot, pSearchString, pUidStr,  szResults);
}





// UIDFindText [PUBLIC]
// 
// FUNCTION
// Search the entire message text for an occurrence of  "pSearchString".
// 
// Return the list of UID hits in szResults.
//
// END FUNCTION

// NOTES
// If the mailbox is not selected, fail.
// END NOTES
//
HRESULT CImapConnection::UIDFindText (LPCSTR pSearchString, unsigned long UidFirst, unsigned long UidLast, CString& szResults, ImapErrorFunc pErrFn /* = NULL */)
{
	// In case we fail.
	szResults.Empty();

	// pSearchString MUST be valid.
	if (! (pSearchString && *pSearchString) )
		return E_INVALIDARG;

	// If we don't have the mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Pass it to the CImapStream.
	return m_pImapStream->UIDFindText (pSearchString, UidFirst, UidLast,  szResults);
}






// HasStatusCommand [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasStatusCommand ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasStatusCommand();
}




// IsImap4Rev1 [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::IsImap4Rev1 ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->IsImap4Rev1();
}



// IsImap4Only [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::IsImap4Only ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->IsImap4Only();
}



// IsLowerThanImap4 [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::IsLowerThanImap4 ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->IsLowerThanImap4();
}



// HasOptimize1 [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasOptimize1 ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasOptimize1();
}



// HasSort [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasSort ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasSort();
}



// HasScan [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasScan ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasScan();
}



// HasAcl [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasAcl ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasAcl();
}



// HasQuota [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasQuota ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasQuota();
}



// HasLiteralPlus [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasLiteralPlus ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasLiteralPlus();
}



// HasXNonHierarchicalRename [PUBLIC]
//
// Ask the IMAP dll if the stream supports the capability.
//
BOOL CImapConnection::HasXNonHierarchicalRename ()
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return FALSE;

	return m_pImapStream->HasXNonHierarchicalRename();
}

//
// CImapConnection::HasUIDPLUS()
//
// Ask the IMAP DLL if the stream supports the UIDPLUS capability.
//
BOOL CImapConnection::HasUIDPLUS()
{
	if (!m_pImapStream)
	{
		return FALSE;
	}

	return m_pImapStream->HasUIDPLUS();
}

//
// CImapConnection::HasNAMESPACE()
//
// Ask the IMAP DLL if the stream supports the NAMESPACE capability.
//
BOOL CImapConnection::HasNAMESPACE()
{
	if (!m_pImapStream)
	{
		return FALSE;
	}

	return m_pImapStream->HasNAMESPACE();
}

//
// CImapConnection::GetNameSpace()
//
// Ask the IMAP DLL for the NAMESPACE value (if any) for the stream.
//
void CImapConnection::GetNameSpace(CString &strNameSpace)
{
	if (!m_pImapStream)
	{
		return;
	}

	m_pImapStream->GetNameSpace(strNameSpace);
}

// UIDFetchStatus [PUBLIC]
//
// Wrapper around the STATUS command..
//
HRESULT CImapConnection::UIDFetchStatus (const char *pMailbox, long flags, MAILSTATUS *pStatus, ImapErrorFunc pErrFn /* = NULL */)
{
	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	return m_pImapStream->UIDFetchStatus (pMailbox, flags, pStatus);
}



// UIDFetchLastUid [PUBLIC]
//
// Fetch the highest UID in the mailbox.
// Do this using a FETCH * FLAGS.
// Return 0 if we failed.
//
HRESULT CImapConnection::UIDFetchLastUid (unsigned long& LastUid, ImapErrorFunc pErrFn /* = NULL */)
{
	unsigned long uid = 0;
	HRESULT		  hResult = E_FAIL;

	// In case we fail:
	//
	LastUid = 0;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return (unsigned long) (MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY));
	}

	// If no messages in the mailbox, we can't do this.
	unsigned long MessageCount = m_pImapStream->GetMessageCount ();

	// (JOK) Do a non-UID fetch of the UID for the last message
	// int the mailbox instead of using "*".
	// 
	if (MessageCount == 0)
	{
		LastUid = 0;

		hResult = S_OK;
	}
	else
	{
		hResult = FetchUID (MessageCount, &uid);

		if ( SUCCEEDED (hResult) )
			LastUid = uid;
	}

	return hResult;
}




// UIDFetchFirstUid [PUBLIC]
//
// Fetch the lowest UID in the mailbox.
// Return 0 if we failed.
//
HRESULT CImapConnection::UIDFetchFirstUid (unsigned long& FirstUid, ImapErrorFunc pErrFn /* = NULL */)
{
	unsigned long uid = 0;
	HRESULT		  hResult = E_FAIL;

	// In case we fail:
	//
	FirstUid = 0;

	// If we don't have a mailbox open, fail.
	if (!m_pImapStream)
		return E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return (unsigned long) (MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY));
	}

	// If no messages in the mailbox, we can't do this.
	unsigned long MessageCount = m_pImapStream->GetMessageCount ();

	// (JOK) Do a non-UID fetch of the UID for the last message
	// int the mailbox instead of using "*".
	// 
	if (MessageCount == 0)
	{
		FirstUid = 0;

		hResult = S_OK;
	}
	else
	{
		hResult = FetchUID (1, &uid);

		if ( SUCCEEDED (hResult) )
			FirstUid = uid;
	}

	return hResult;
}





//////////////////////////////////////////////////////////////////////
// UpdateMailboxStatus [PUBLIC]

// Get the new values of:
// - Number of messages
// - uidvalidity
// for the remote mailbox.
// If the server is an IMAP4rev1 server, do a STATUS command, else,
// close the stream and reopen it.
//////////////////////////////////////////////////////////////////////
HRESULT CImapConnection::UpdateMailboxStatus (ImapErrorFunc pErrFn /* = NULL */)
{
	HRESULT hResult = E_FAIL;

	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	// Open the stream if it's closed. This is all we need to get the new 
	// status values.
	//
	if ( ! m_pImapStream )
		return OpenMailbox (m_szImapName);

	// If' it's IMAP4 only, or is not selected, close the stream and re-open it.
	if ( (! m_pImapStream->IsSelected () )	|| 
		 (! m_pImapStream->HasStatusCommand() ) )
	{
		//
		// Save mailbox full IMAP name before we close.
		//
		CString szImapName = m_szImapName;
		//
		// BUG: Should be able to do a re-select instead of closing and re-openning.
		//
		Close();

		return OpenMailbox (szImapName);
	}

	//
	// The STATUS now.
	//
	MAILSTATUS mStatus;
	memset ( (void *)&mStatus, 0, sizeof (MAILSTATUS) );

	hResult =  m_pImapStream->UIDFetchStatus (m_szImapName, SA_MESSAGES | SA_UIDVALIDITY, &mStatus);

	return hResult;
}		



//
// Get top mailbox delimiter char.
//
HRESULT CImapConnection::GetTopMboxDelimiter (TCHAR *pDelimiter, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if ( m_pImapStream )
		return m_pImapStream->GetTopMboxDelimiter (pDelimiter);
	else
		return E_FAIL;
}


// UIDRemoveFlaglist [PUBLIC]
// 
// Unset the specified flags.
// Note: pFlagList is a comma-sparated list of "\\Deleted", "\\Seen", etc.
//
HRESULT CImapConnection::UIDRemoveFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}


	if ( m_pImapStream )
		return m_pImapStream->UIDRemoveFlaglist (pUidList, pFlagList, Silent);
	else
		return E_FAIL;
}


// UIDAddFlaglist [PUBLIC]
//
// Set the specified flags.
// Note: pFlagList is a comma-sparated list of "\\Deleted", "\\Seen", etc.
//
HRESULT CImapConnection::UIDAddFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */, ImapErrorFunc pErrFn /* = NULL */)
{
	// Negotiate for the lock on the object.
	//
	CCIMapLock localLock (this);

	// Attempt to lock with a timeout value.
	//
	if ( !localLock.LockNoWait(pErrFn) )
	{
		// Must return this!!!
		//
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);
	}

	if ( m_pImapStream )
		return m_pImapStream->UIDAddFlaglist (pUidList, pFlagList, Silent);
	else
		return E_FAIL;
}


// NetworkNotificationDispatcher [PUBLIC]
//
// This gets called during a notification, Pass the notification on:
//
void CImapConnection::NetworkNotificationDispatcher (NetworkNotifierType NotifierType, unsigned long ulValue)
{
	// For now, we just set internal state.
	//
	if (m_pNetworkNotifier)
		m_pNetworkNotifier->SetNotification (NotifierType, ulValue);
}


// Stub for use as a default calback function:
//
// static
void CImapConnection::ErrorCallback (LPCSTR pBuf)
{


}




// ================================================================================
// OpenMailbox [PRIVATE] - open IMAP mailbox.
//
// Does the real work of opening the mailbox.
//
// NOTES
// NOTE!! Assume that we already have the lock on this object.
// Only one IMAP operation can be performed at a time (FOR NOW).
//
// END NOTES

HRESULT CImapConnection::__OpenMailbox (const char *Fullname, BOOL bSilent, ImapErrorFunc pErrFn /* = NULL */)
{
	// Set the DLL's debug logging callback routine.
	SetDebugLog (__ImapDebugLogger);
	SetTimeoutQuery (__TimeoutQuery);

	// Sanity. Must have these:
	if ( ! Fullname )
		return (E_INVALIDARG);

	// Copy the Fullname to the internal Imap name string.
	m_szImapName = Fullname;

	// Must have had this set:
	//
	if (!m_pNetSettings)
	{
		return E_FAIL;
	}

	if( !m_pNetSettings->GetRasPassword())
	{
		return E_FAIL;
	}

	// DO we have a valid account ID?
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
	{
		// Invalid account ID
		return E_FAIL;
	}
	
	// Check for validity of a previously set account pointer.
	if ( m_pAccount )
	{
		if (m_pAccount != pAccount) 
		{
			// Account ID doesn't match previously set account. Can't 
			// use one CImapConnection to connect to different accounts. 
			return E_FAIL;
		}
	}
	else
	{
		SetAccount (pAccount);
	}

	// Clear noticifations.
	//
	if (m_pNetworkNotifier)
		m_pNetworkNotifier->ResetNotifications();

	// Make sure.
	CString szPortNum;  pAccount->GetPortNum(szPortNum);
	SetPortNum (szPortNum);

	// Clear noticifations.
	//
	if (m_pNetworkNotifier)
		m_pNetworkNotifier->ResetNotifications();

	// If this is NULL, create it now.
	if (!m_pImapStream)
	{
		m_pImapStream = DEBUG_NEW_NOTHROW CImapStream (
											(LPCSTR)m_Server, 
											IDS_IMAP_SERVICE,
											(LPCSTR)m_PortNum);

		// 
		if (m_pImapStream)
		{
			if ( !m_szKrbLibName.IsEmpty() )
				m_pImapStream->SetKrbDllName (m_szKrbLibName);

			if ( !m_szDesLibName.IsEmpty() )
				m_pImapStream->SetDesDllName (m_szDesLibName);

			if ( !m_szGssLibName.IsEmpty() )
				m_pImapStream->SetGssDllName (m_szGssLibName);
		}
	}

	if (!m_pImapStream)
		return E_FAIL;

	// If the stream's not connected, set new login and password.
	//
	if (!m_pImapStream->IsConnected())
	{
		m_pImapStream->SetLogin (m_szLogin);
		m_pImapStream->SetPassword (m_szPassword);
	}

	m_pImapStream->SetNetSettingsObject (m_pNetSettings);
	CString szPersona;
	m_pAccount->GetName (szPersona);
	m_pImapStream->SetSSLSettings((SSLSettings*)&pAccount->m_SSLSettings,szPersona);


	// $$$$$$$$$$$$$$$
	// Note: Must make access to gtting this value from teh account thread safe.
	//
	m_pImapStream->AllowAuthenticate ( pAccount->IsAllowAuthenticate () );

	// Register our user callback handler:
	m_pImapStream->RegisterUserCallback ( NetworkNotificationHandler, (unsigned long) this);

	// Set the alert callback.  We call this here to set the alert callback function
	// pointer for the CMImap object.  This is step 1 in a 2 step process because the
	// ultimate goal is to tell the CProtocol object about the callback but it does
	// not exist yet.  It will be created somewhere inside m_pImapStream->OpenMailbox()
	// so we need to tell the CMImap object about the alert callback so it can tell
	// the CProtocol about it at the appropriate moment. - dwiggins.
	SetAlertCallback(m_pAlertCallback);

	// Open the mailbox now.
	return m_pImapStream->OpenMailbox(Fullname);
}



// __OpenControlStream [PRIVATE]
//
// Does the real work.
// Create a half-open stream to the server in m_Server (which must be non-empty).
//
// Note: pAccount CAN be NULL.!!
//
HRESULT CImapConnection::__OpenControlStream(CImapAccount *pAccount, BOOL bSilent, ImapErrorFunc pErrFn /* = NULL */)
{
	// Set the DLL's debug logging callback routine.
	SetDebugLog(__ImapDebugLogger);
	SetTimeoutQuery (__TimeoutQuery);

	// Must have had this set:
	//
	if (!m_pNetSettings)
	{
		return E_FAIL;
	}

	if( !m_pNetSettings->GetRasPassword())
	{
		return E_FAIL;
	}

	// Set the passed-in pAccount into the object.
	if (pAccount)
		m_pAccount = pAccount;

	// If the account hasn't been set, we can't continue.
	if (m_pAccount == NULL)
	{
		ASSERT (0);
		return E_FAIL;
	}


	// Clear noticifations.
	//
	if (m_pNetworkNotifier)
		m_pNetworkNotifier->ResetNotifications();

	// If this is NULL, create it now.
	if (!m_pImapStream)
	{
		m_pImapStream = DEBUG_NEW_NOTHROW CImapStream (
											(LPCSTR)m_Server,
											IDS_IMAP_SERVICE,
											(LPCSTR)m_PortNum);

		if (m_pImapStream)
		{
			if ( !m_szKrbLibName.IsEmpty() )
				m_pImapStream->SetKrbDllName (m_szKrbLibName);

			if ( !m_szDesLibName.IsEmpty() )
				m_pImapStream->SetDesDllName (m_szDesLibName);

			if ( !m_szGssLibName.IsEmpty() )
				m_pImapStream->SetGssDllName (m_szGssLibName);
		}
	}

	if (!m_pImapStream)
		return E_FAIL;

	// If the stream's not connected, set new login and password.
	//
	if (!m_pImapStream->IsConnected())
	{
		m_pImapStream->SetLogin (m_szLogin);
		m_pImapStream->SetPassword (m_szPassword);
	}

	// Pass the QCNetworksettings pointer to the CImapStream.
	//
	m_pImapStream->SetNetSettingsObject (m_pNetSettings);
	CString szPersona;
	m_pAccount->GetName (szPersona);

	m_pImapStream->SetSSLSettings((SSLSettings*)&pAccount->m_SSLSettings, szPersona);	

	// Set these:
	// $$$$$$$$$$$$$$$
	// Note: Must make access to getting this value from the account thread safe.
	//
	m_pImapStream->AllowAuthenticate ( pAccount->IsAllowAuthenticate () );

	// Register our user callback handler:
	m_pImapStream->RegisterUserCallback ( NetworkNotificationHandler, (unsigned long) this );

	// Open the mailbox now.
	return m_pImapStream->OpenControlStream();
}




/////////////////////////////////////////////////////////////////
// __UIDFind [PRIVATE]
//
// Internal worker routine. Do the real work.
// NOTES: 
//	Assume that parameters have already been validated!!
// END NOTES
/////////////////////////////////////////////////////////////////

HRESULT CImapConnection::__UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr,  CString& szResults)
{
	BOOL		  bResult = FALSE;

	// Must have a stream.
	if (!m_pImapStream)
		return E_FAIL;

	// Fill a search program with the criteria.
	SEARCHPGM* pPgm = CImapStream::NewSearchPgm();
	if (!pPgm)
		return E_FAIL;

	// Make sure:
	//
	memset ( (void *)pPgm, 0, sizeof (SEARCHPGM) );

	//
	// Formulate the tree of search programs representing the search criteria.
	//
	bResult = m_pImapStream->FormatSearchCriteria (pPgm, pHeaderList, bBody, bNot, pSearchString);

	if (bResult)
	{
		// Formulate the UID range(s) over which the search will be performed/
		//
		bResult = CImapStream::SetNewPgmUidList (pPgm, pUidStr);
	}

	if (bResult)
	{
		// Send.
		//
		m_pImapStream->Search (NULL, pPgm, szResults);
	}

	// Cleanup.
	CImapStream::FreeSearchpgm (&pPgm);

	return bResult ? S_OK : E_FAIL;
}


void CImapConnection::AlertCallback (const char *str, TaskErrorType terr)
{
	if (!str)
		return;

	if (m_pTaskInfo)
	{
		m_pTaskInfo->SetPersona(m_pAccount->GetPersona());
		m_pTaskInfo->SetMainText(CRString(IDS_IMAP_ALERT));

		m_pTaskInfo->CreateError (str, terr);
	}
}





//=============================================================================//


//
// Our own proxy callback handler:
// The data is a CImapConnection object.

static void NetworkNotificationHandler (unsigned long ulData, unsigned short uNotifierType, unsigned long ulValue )
{
	// Sanity:
	if ( !ulData )
		return;

	// Get the CImapConnection object;
	CImapConnection *pImap = (CImapConnection *)ulData;

	NetworkNotifierType	ntVal = ConvertToNotifierType (uNotifierType);

	// Call the class's dispatcher.
	pImap->NetworkNotificationDispatcher (ntVal, ulValue);
}



//========================== Internal utility functions ============================/


/* full RFC-822 specials */
const char *rspecials =  "()<>@,;:\\\"[].";
				/* body token specials */
const char *tspecials = " ()<>@,;:\\\"[]./?=";
				/* parse-word specials */
const char *wspecials = " ()<>@,;:\\\"[]";
				/* parse-token specials for parsing */
const char *ptspecials = " ()<>@,;:\\\"[]/?=";


/* Write RFC822 address
 * Accepts: pointer to destination string
 *	    address to interpret
 */

void rfc822_write_address (char *dest,MAILADDRESS *adr)
{
  while (adr) {
				/* start of group? */
    if (adr->mailbox && !adr->host) {
				/* yes, write group name */
      rfc822_cat (dest,adr->mailbox,rspecials);
      strcat (dest,": ");	/* write group identifier */
      adr = adr->next;		/* move to next address block */
    }
    else {			/* end of group? */
      if (!adr->host) strcat (dest,";");
				/* simple case? */
      else if (!(adr->personal || adr->adl)) rfc822_address (dest,adr);
      else {			/* no, must use phrase <route-addr> form */
	if (adr->personal) {	/* in case have adl but no personal name */
	  rfc822_cat (dest,adr->personal,rspecials);
	  strcat (dest," ");
	}
	strcat (dest,"<");	/* write address delimiter */
	rfc822_address (dest,adr);/* write address */
	strcat (dest,">");	/* closing delimiter */
      }
				/* delimit if there is one */
      if ((adr = adr->next) && adr->mailbox) strcat (dest,", ");
    }
  }
}


/* Write RFC822 route-address to string
 * Accepts: pointer to destination string
 *	    address to interpret
 */

void rfc822_address (char *dest,MAILADDRESS *adr)
{
  if (adr && adr->host) {	/* no-op if no address */
    if (adr->adl) {		/* have an A-D-L? */
      strcat (dest,adr->adl);
      strcat (dest,":");
    }
				/* write mailbox name */
    rfc822_cat (dest,adr->mailbox,wspecials);
    if (*adr->host != '@') {	/* unless null host (HIGHLY discouraged!) */
      strcat (dest,"@");	/* host delimiter */
      strcat (dest,adr->host);	/* write host name */
    }
  }
}


/* Concatenate RFC822 string
 * Accepts: pointer to destination string
 *	    pointer to string to concatenate
 *	    list of special characters
 */

void rfc822_cat (char *dest,char *src,const char *specials)
{
  char *s;
  if (strpbrk (src,specials)) {	/* any specials present? */
    strcat (dest,"\"");		/* opening quote */
				/* truly bizarre characters in there? */
    while (s = strpbrk (src,"\\\"")) {
      strncat (dest,src,s-src);	/* yes, output leader */
      strcat (dest,"\\");	/* quoting */
      strncat (dest,s,1);	/* output the bizarre character */
      src = ++s;		/* continue after the bizarre character */
    }
    if (*src) strcat (dest,src);/* output non-bizarre string */
    strcat (dest,"\"");		/* closing quote */
  }
  else strcat (dest,src);	/* otherwise it's the easy case */
}




// Debug logger passed to the IMAP dll.
BOOL  __TimeoutQuery ()
{
	return AlertDialog (IDD_NETWORK_TIMEOUT, NULL) == IDOK;
}



// Convert from the IMAP dll's notifier values to our own:
//
NetworkNotifierType	ConvertToNotifierType (unsigned short uNotifierType)
{
	NetworkNotifierType ntVal = NETNOTIFY_NONE;

	switch (uNotifierType)
	{
		case IMAP_NOTIFY_SET_READONLY_STATUS:
			ntVal = NETNOTIFY_SET_READONLY_STATUS;

			break;

		case IMAP_NOTIFY_SET_READWRITE_STATUS:
			ntVal = NETNOTIFY_SET_READWRITE_STATUS;

			break;

		case IMAP_NOTIFY_ESCAPE_PRESSED:
			ntVal = NETNOTIFY_STOP_REQUESTED;
			break;

		case IMAP_NOTIFY_INVALID_PASSWORD:
			ntVal = NETNOTIFY_INVALID_PASSWORD;
			break;
	}

	return ntVal;
}



// Stub for use as a default calback function:
//
// static
void DummyCallback (const char *buf)
{


}



#endif // IMAP4

