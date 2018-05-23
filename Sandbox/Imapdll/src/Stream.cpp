// Stream.cpp - Base class for the IMAP protocol implementation.
//

#include "stdafx.h"
#include "MyTypes.h"

#include <afxmt.h>
#include <stdio.h>
#include <time.h>
#include "exports.h"
#include "ImapDefs.h"
#include "Stream.h"		// Our class defs.
#include "imap.h"
#include "mm.h"

#include "resource.h"
#include "osdep.h"		// For fs_get and fs_give.
#include "rfc822.h"
#include "misc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Statics:


// Used in the error database.
//
#define MAX_ERROR_LIST_SIZE		6

const DWORD dwErrorWaitTimeout = 2;		// seconds


const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


//==========================================================================//

// Class used to store error data for each error.
class CErrorData
{
public:
	CErrorData ();
	~CErrorData ();
	
// Public data.
	CString m_Msg;		// error message
	int		m_Type;		// Error type. 	
};


CErrorData::CErrorData ()
{
	m_Type = -1;
}


CErrorData::~CErrorData ()
{
}

//=========================================================================//


// ===================== CProtocol implementation ===============================

CStream::CStream()
{
	m_bLocked = FALSE;


	// Initialize:
	m_bReadOnly = m_bAnonymous = FALSE;

	m_Nmsgs = 0;

	// Put this here.
	m_hErrorLockable = CreateMutex(NULL,	// Security
							  FALSE,		// Initial owner
							  NULL);	// Name of object

	// Initialize rest:
	//
	Initialize();
}

CStream::~CStream()
{
	if (m_hErrorLockable)
		CloseHandle(m_hErrorLockable);

	// Free memory in m_Reply.
	//
	fs_give ((void **)&m_Reply.line);
}


// Initialize [PRIVATE]
// Must do this.
//
void CStream::Initialize ()
{
	// Library names:
	//
	m_szKrbLibName.Empty();
	m_szDesLibName.Empty();
	m_szGssLibName.Empty();

	// THis should go away!!
	m_ServiceID = 0;

	mgets_data = mboxgets_data = m_pEltWriter = NULL;
	
	// Store search results results in this, as a command separated list of uid's.
	m_szSearchResults.Empty();

	// The last tagged response is copied here before parsing so it can be available to
	// a caller.
	//
	*szSrvTRespBuf	= 0;

	//
	m_PermUserFlags = m_Gensym = m_Recent = 0;

	m_UidValidity	= m_UidLast = 0;

	for (int i = 0; i < NUSERFLAGS; i++)
		m_UserFlags[i] = NULL;

	m_Nmsgs		= 0;
	m_UseAuth	= 0;

	//
	login_data	= m_pMailgets = mailreadprogress = NULL;

	// Pass back top level delimiter char in this.
	m_DelimiterChar = 0;

	// This is protected by the following mutex:
	m_pErrorFn	= NULL;

	// By value.
	memset ( &m_Reply, 0, sizeof(IMAPPARSEDREPLY) );


	m_Prefix	= NULL;
	*m_tmpbuf	= 0;

	// BOOLS.
	m_bAllowAuthenticate	= 
	m_bImap4rev1			= 
	m_bImap4				=
	m_bImap2bis				=
	m_bRfc1176				=
	m_bUseStatus			=
	m_bUseScan				= 
	m_bAcl					=
	m_bQuota				=
	m_bLiteralplus			=
	m_bX_NonHierarchicalRename = 
	m_bSort					=
	m_bOptimize1			= FALSE;

	m_bUidsearch			=
	m_bByeseen				=
	m_bPermSeen				=
	m_bPermDeleted			=
	m_bPermFlagged			=
	m_bPermAnswered			=
	m_bPermDraft			=
	m_bKwdCreate			=
	m_bReadOnly				=
	m_bAnonymous			= FALSE;


	// Store these status flags explicitly (JOK).
	m_bConnected = m_bAuthenticated = m_bSelected = FALSE;

	// Locked flag.
	//
	m_bLocked = FALSE;

	m_szLogin.Empty();
	m_szPassword.Empty();

	// User registers this when this struct is instantiated:
	//
	m_pUserCallback = NULL;
	//
	// User passes this opaque data that must be passed back in 
	// m_pUserCallback.
	//
	m_ulUserData = 0;

	// Our mailbox name.
	//
	m_szMailbox.Empty();

	// Error database.
	//
	ClearAllErrorEntries();
}








// StreamNotify [PUBLIC]
//
// Receives notifications of events that may affect it.
//
void CStream::StreamNotify (NOTIFY_TYPE notify_type, void *)
{
	// "data not used yet- coddle compiler.

	switch (notify_type)
	{
		// In this case, the netstream has been closed.
		case NETSTREAM_CLOSED:
			// Reset flags.
			m_bConnected		= FALSE;
			m_bAuthenticated	= FALSE;
			m_bSelected			= FALSE;

			break;

		default:
			break;
	}
}





void CStream::mm_searched (unsigned long number)
{
	if ( 0 == number )
		return;

	// Accumulate the message numbers into m_szSearchResults, as a comma-separated list 
	//
	if ( m_szSearchResults.IsEmpty() )
		m_szSearchResults.Format ("%ld", number);
	else
	{
		CString sUid;
		sUid.Format (",%ld", number);

		m_szSearchResults += sUid;
	}
}


void CStream::mm_exists (unsigned long number)
{
	m_Nmsgs = number;
}



void CStream::mm_recent (unsigned long number)
{
	m_Recent = number;
}


void CStream::mm_expunged (unsigned long MessageNumber)
{
	m_Nmsgs--;

	if (m_Nmsgs < 0)
	{
		ASSERT (0);
		m_Nmsgs = 0;
	}
	else
	{
		// Remove MessageNumber from the msgnum-to-UID list.
		// "Remove" returns the uid removed.
		//
		unsigned long uid = m_msgMap.Remove (MessageNumber);

		if (uid > 0)
		{
			CString sUid;

			if ( m_szExpungeUidList.IsEmpty() )
			{
				m_szExpungeUidList.Format ("%lu", uid);
			}
			else
			{		
				sUid.Format (",%lu", uid);

				m_szExpungeUidList += sUid;
			}
		}
	}
}



void CStream::mm_flags (unsigned long)
{


}


void CStream::mm_elt_flags (MESSAGECACHE *elt)
{
	if (elt)
	{
		if (m_pEltWriter)
		{
			((CEltWriter *)m_pEltWriter)->SetFlag ( elt->privat.uid, 
								  elt->seen,
								  elt->deleted,
								  elt->flagged,
								  elt->answered,
								  elt->draft,
								  elt->recent);
		}
	}
}



// Called during FetchUids.
//
void CStream::mm_uid (unsigned long msgno, unsigned long uid)
{
	// Append to message/UID map.
	//
	m_msgMap.Append (msgno, uid);
}


void CStream::mm_notify (char *string, long errflg)
{
	mm_log (string,errflg);
}


void CStream::mm_alert (char *string)
{
	if (string)
	{
		ReportError (string, IMAPERR_COMMAND_FAILED);
	
		mm_log (string, 0);
	}
}



void CStream::mm_list (int delimiter,char *mailbox,long attributes)
{
	// If mailbox is NULL or empty, and there is a delimiter, assume that this
	// is the "account's" top level delimiter. Set this within the
	// stream
	// 
	if ( !(mailbox && *mailbox) )
	{
		if (delimiter)
		{
			m_DelimiterChar = (char)delimiter;
		}
		return;
	}

	//
	// A real mailbox. Go add it to list.
	//
	CMboxGets *Mboxgets = (CMboxGets *) (mboxgets_data);
	if (Mboxgets)
	{
		Mboxgets->DoMboxgets (mailbox, delimiter, attributes);
	}
}


void CStream::mm_log (char *string,long errflg)
{
	// Call the global one for now:
	//
	::mm_log (string, errflg);
}



void CStream::mm_dlog (char *string)
{
#ifdef _DEBUG

	mm_log (string, NIL);
#else

	UNREFERENCED_PARAMETER(string);

#endif // _DEBUG

}




void CStream::mm_lsub (int delimiter, char *mailbox, long attributes)
{
	mm_list (delimiter, mailbox, attributes);
}



void CStream::mm_status (char *pMailbox, MAILSTATUS *pStatus)
{
	//  If we STATUS'd ouw own mailbox, update our state.
	//
	if ( pMailbox && pStatus && (m_szMailbox.Compare (pMailbox) == 0) )
	{
		// Update the stream's m_Nmsgs counter.
		//
		if (pStatus->flags & SA_MESSAGES)
		m_Nmsgs = pStatus->messages;

		// uid_validity
		if (pStatus->flags & SA_UIDVALIDITY)
			m_UidValidity = pStatus->uidvalidity;

		// recent
		if (pStatus->flags & SA_RECENT)
			m_Recent = pStatus->recent;
	}
}



void CStream::mm_login (char *user, char *pwd,long trial)
{
	UNREFERENCED_PARAMETER(trial);

	// Just return m_szLogin and m_szPassword from the stream.
	//
	strcpy (user, m_szLogin);
	strcpy (pwd,  m_szPassword);
}
		


void CStream::mm_critical ()
{
	mm_dlog ("mm_critical entered");
}


void CStream::mm_nocritical ()
{
}


long CStream::mm_diskerror (long, long)
{

  // abort ();
  mm_log ("mm_diskerror: Disk error occurred", IMAPERROR);

  return NIL;
}



void CStream::mm_fatal (char *string)
{
	if (string)
		mm_log (string, IMAPERROR);
}




void CStream::mm_error (unsigned short)
{


}




// GetGssLibraryName [PUBIC]
//
// Return kerberos library names.
//
void CStream::GetGssLibraryName (CString& szName)
{ 
	// Overridden??
	//
	if ( !m_szGssLibName.IsEmpty() )
		szName = m_szGssLibName;
	else
		szName = CRString (IDS_GSSAPI_LIBNAME);
}


// GetKrb4LibraryName [PUBLIC]
//
void CStream::GetKrb4LibraryName (CString& szName)
{ 
	// Overridden??
	//
	if ( !m_szKrbLibName.IsEmpty() )
		szName = m_szKrbLibName;
	else
		szName = CRString (IDS_KRB4_LIBNAME);
}


// GetDesLibraryName [PUBLIC]
//
void CStream::GetDesLibraryName (CString& szName)
{ 
	// Overridden??
	//
	if ( !m_szDesLibName.IsEmpty() )
		szName = m_szDesLibName;
	else
		szName = CRString (IDS_DES_LIBNAME);
}




// Error management stuff.
//


// If there's an error callback in the stream, go grab it and use it.
//
void CStream::ReportError (LPCSTR pStr, UINT Type)
{

	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	if (dwRes != WAIT_OBJECT_0)
		return;

	// 
	if (m_pErrorFn)
		(m_pErrorFn) (pStr, (TaskErrorType)Type);
	else
	{
		// Can call this because we've got the mutex:
		//
		AppendErrorStringAndType (Type, pStr);
	}

	// Make sure:
	ReleaseMutex (m_hErrorLockable);
}


// FUNCTION
// Add the given string as-is to the error database.
// END FUNCTION

void CStream::AddLastErrorString (int Type, const char *pString)
{
	// Must have a stream.
	if (! pString )
		return;

	// Just call Report error on the stream.
	//
	ReportError(pString, Type);		
}


// Error database routines per stream (JOK)

void CStream::AddLastError(int Type, UINT StringID, ...)
{
	TCHAR buf [1024];
	
	buf[0] = '\0';

	// Get the arguments.
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString (StringID), args);
	va_end(args);

	// Just call Report error on the stream.
	//
	ReportError(buf, Type);
}




// AppendErrorStringAndType [INTERNAL]
// NOTE: IMPORTANT!!
// NOTE: IMPORTANT!!
// This MUST ONLY BE CALLED by ReportError, AFTER
// it has already obtained a mutex on the error database!!
//
// static
void CStream::AppendErrorStringAndType (UINT Type, LPCSTR pString)
{

	//
	// Create new error data object.
	//
	CErrorData *pErrData = new CErrorData;

	if (pErrData)
	{
		// Set message.
		pErrData->m_Msg = pString;

		// And type:
		pErrData->m_Type = Type;

		// Add to list.
		if (m_ErrorList.GetCount() == 0)
			m_ErrorList.AddTail ( (void *) pErrData);
		else
		{
			// Must we remove the first list member?
			if (m_ErrorList.GetCount() >= MAX_ERROR_LIST_SIZE)
			{
				POSITION pos = m_ErrorList.GetHeadPosition ();
				if (pos)
				{
					CErrorData *p = (CErrorData *) m_ErrorList.GetAt (pos);
					m_ErrorList.RemoveAt (pos);
	
					if (p)
						delete p;
				}
			}

			// Now add to the end.
			m_ErrorList.AddTail( (void *) pErrData);
		}
	}
}




// GetLastServerMessage [PUBLIC]
//
// FUNCTION 
// Copy the tagged response buffer from the stream into the given buffer.
//
// NOTE: Use the same mutex as used by the error database functions.
//
// END FUNCTION
//
HRESULT CStream::GetLastServerMessage (LPSTR szErrorBuf, short nBufSize)
{	
	BOOL bResult = FALSE;

	if ( (NULL == szErrorBuf) || (nBufSize < 1) )
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	// In case we fail.
	szErrorBuf[0] = '\0';

	short len;
	len = (short) min ( nBufSize - 1, (short) strlen (szSrvTRespBuf) );

	if (len > 0)
	{
		strncpy (szErrorBuf, szSrvTRespBuf, len);
		szErrorBuf[len] = '\0';

		bResult = TRUE;
	}

	// Make sure:
	ReleaseMutex (m_hErrorLockable);

	return bResult ? S_OK : E_FAIL;
}	



// FUNCTION
// Copy the last error string into the supplied buffer.
// Return the corresponding error code if we got a string.
// IMAPERR_BAD_CODE if we failed.
// NOTE: Don't remove the string from the error string list!!
// END FUNCTION

int CStream::GetLastErrorStringAndType (LPSTR szErrorBuf, short nBufSize)
{
	POSITION pos;
	int iType = IMAPERR_BAD_CODE;

	if ( (NULL == szErrorBuf) || (nBufSize < 1) )
	{
		ASSERT (0);
		return iType;
	}

	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	if (dwRes != WAIT_OBJECT_0)
		return iType;

	// In case we fail.
	*szErrorBuf = 0;

	// Get the object at the end of the list.
	pos = m_ErrorList.GetTailPosition();

	if (pos)
	{
		CErrorData	 *p = (CErrorData *) m_ErrorList.GetAt (pos);
		if (p)
		{
			size_t len = (size_t) min (nBufSize - 1, p->m_Msg.GetLength());
			if (len > 0)
			{
				strncpy (szErrorBuf, p->m_Msg.Mid(0), len);
				szErrorBuf [len] = '\0';

				// Error Type.
				iType = p->m_Type;
			}
		}
	}

	// Make sure:
	ReleaseMutex (m_hErrorLockable);

	return iType;
}


// FUNCTION
// Delete all objects in the error string list and free internal memory.
// However, don't free the list object itself.
// END FUNCTION

void CStream::ClearAllErrorEntries()
{
	POSITION pos, next;
	CErrorData	 *p;

	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	pos = m_ErrorList.GetHeadPosition();

	for( next = pos; pos; pos = next )
	{
		p = ( CErrorData * ) m_ErrorList.GetNext( next );
		if (p)
		{
			m_ErrorList.RemoveAt (pos);
			delete p;
		}
	}

	m_ErrorList.RemoveAll();

	// Note: Don't free the list object itself.

	// Make sure:
	ReleaseMutex (m_hErrorLockable);
}



int CStream::NumEntriesInErrorDbase ()
{
	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	POSITION pos = m_ErrorList.GetHeadPosition();
	int iNumEntries = 0;

	while (pos)
	{
		iNumEntries++;
		m_ErrorList.GetNext(pos);

	}

	// Make sure:
	ReleaseMutex (m_hErrorLockable);

	return iNumEntries;
}
	


void CStream::SetErrorCallback (ImapErrorFunc pImapErrorFn)
{
	if (! pImapErrorFn )
		return;

	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	if (dwRes != WAIT_OBJECT_0)
		return;

	// 
	m_pErrorFn = pImapErrorFn;

	// Make sure:
	ReleaseMutex (m_hErrorLockable);
}


void CStream::ResetErrorCallback ()
{
	// First, get a lock on the mutex:
	//
	DWORD dwRes = WaitForSingleObject (m_hErrorLockable, dwErrorWaitTimeout);

	if (dwRes != WAIT_OBJECT_0)
		return;

	// 
	m_pErrorFn = NULL;

	// Make sure:
	ReleaseMutex (m_hErrorLockable);
}



//====================== PROTECTED METHODS ==========================//

void CStream::ClearMessageMap()
{
	// Clear the map.
	//
	m_msgMap.DeleteAll();
}

