// imapfr.cpp -  class implementation that does all the work.


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "MyTypes.h"

#include <afxmt.h>
#include <stdio.h>
#include <time.h>

//#define _IMAPDLL_INTERNAL_
// #include "Network.h"
//#include "imapmail.h"
//#include "imap4r1.h"

#include "exports.h"
#include "ImapDefs.h"
#include "imap.h"		// exported class defs. This must be ahead of imapfr.h.
#include "Proto.h"
#include "imapfr.h"		// Our class def.
#include "mm.h"
#include "ImapUtil.h"

#include "osdep.h"			// For fs_get and fs_give.
#include "misc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static const DWORD dwLockTimeout = 5000;			// Milliseconds.

// Timeout value for the CSingleLoc.Lock() to wait on thye lock (milliseconds).
const unsigned long COMMAND_TIMEOUT = 10000;		// 10 seconds..

// Break a long UID-list into chunks of this number of uid per op.
const int MAX_UIDS_PER_COMMAND = 60;

// Used in several methods.
const char	Comma = ',';


// Internal routines

STRINGLIST *CommaSeparatedTextToStringlist (char *Fields);


//==========================================================================//

//===========================================================================//
//
// Class to handle stream locking. Create a new one of these and it will attempt to
// lock the given stream. On destruction, the stream will be automatically unlocked if 
// that object was the one that locked it in the first place.
//
class CStreamLock
{
public:
	CStreamLock (CProtocol *pProto);
	~CStreamLock();

	BOOL WeGotTheLock ()  { return m_bWeHaveTheLock; }

private:
	BOOL m_bWeHaveTheLock;

	// Referenced stream
	CProtocol *m_pProto;
};



// 
// Constructor: Attempt to lock the stream, and if we succeed, set the internal flag.
// The destructor will automatically unlock it.
//
CStreamLock::CStreamLock (CProtocol *pProto)
{
	// Initialize this.
	m_bWeHaveTheLock = FALSE;
	m_pProto		 = NULL;

	// Attempt to lock the stream.
	if (pProto)
	{
		if ( !pProto->IsLocked() )
		{
			// We got it.
			pProto->Lock();
			m_bWeHaveTheLock = TRUE;

			m_pProto = pProto;
		}
	}
}



//
// It we had the lock, clear it from the stream.
//
CStreamLock::~CStreamLock ()
{
	
	if (m_pProto && m_bWeHaveTheLock)
		m_pProto->UnLock ();

	m_bWeHaveTheLock = FALSE;
}



//============== CMImap class =======================//


// Constructor
CMImap::CMImap(const char *ServerName, UINT ServiceID, const char *PortNum)
{
	m_currentMsgNum = 0;			// Invalid
	m_currentUID	= 0;			// Invalid
	m_MessageSizeLimit = 2000000;	// Arbitrary
	m_pName = NULL;

	// Use this for communication with the protocol stream.
	m_pProtocol = NULL;

	// IMAP Server identification:
	if (ServerName)
		m_pServer = cpystr (ServerName);
	else
		m_pServer = NULL;

	// Main program's notion of resource ID for service. This should disappear!!
	//
	m_ServiceID = ServiceID;

	// Default to 143.
	if (PortNum)
		m_pPortNum = cpystr(PortNum);
	else
		m_pPortNum = NULL;

	// TRUE unless specifically changed.
	m_bAllowAuthenticate = TRUE;

	// Make sure initialize these:
	//
	m_pUserCallback	= NULL;
	m_ulUserData	= 0;

	m_pNetSettings	= NULL;

	// Create the sync. object. Use a critical section.
	m_pLockable = new CCriticalSection;
}


CMImap::~CMImap()
{
	// If still connected, close.
	if (m_pProtocol)
		delete (m_pProtocol);

	if (m_pServer)
	{
		fs_give ( (void **)& m_pServer);
		m_pServer = NULL;
	}

	if (m_pPortNum)
	{
		fs_give ( (void **)&m_pPortNum);
		m_pPortNum = NULL;
	}

	// Free stuff.
	if (m_pName)
	{
		fs_give ( (void **)&m_pName);
		m_pName = NULL;
	}

	// Free.
	delete m_pLockable;
}


BOOL CMImap::StreamIsLocked() 
{
	if (m_pProtocol)
		return m_pProtocol->IsLocked();

	return FALSE;
}





// OpenStream [protected]
// NOTES
// Open either an imap stream. If Mailbox is NULL, open a "control"
// stream, otherwise attempt to "SELECT" the Mailbox as well.
// END NOTES
// HISTORY
// Created 4/10/97
// END HISTORY
HRESULT CMImap::OpenStream(const char *Mailbox)
{
	BOOL		bResult = FALSE;
	long		options = 0L;


	// Must also have a server name or ip address.
	if (!m_pServer)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Must have a QCNetworkSettings object.
	//
	if (!m_pNetSettings)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If we have an open control stream, re-use that!!
	if (m_pProtocol)
	{	
		// If we are just opening a control stream, return TRUE if we are AUTHENTICATED.
		if ( !Mailbox )
		{
			if ( IsAuthenticated() )
			{
				return S_OK;
			}
		}
		else
		{
			// IF we are already connected to the given mailbox, return TRUE!!
			if (IsSelected())
			{
				if ( stricmp (Mailbox, m_pProtocol->GetMailbox() ) == 0)
				{
					// We've got an open stream. Do a NOOP so we re-sync things.
					Noop();

					return S_OK;
				}
			}
		}
	}

	// If we get here, we have to open the stream. However, re-use current one
	// if there is one.
	if (!m_pProtocol)
	{
		// Allocate a new "stream" to pass to imapmail_open. The stream will be
		// filled in by imapmail_open(). It will not change.
		m_pProtocol = new CProtocol (m_pServer, m_pPortNum);

		if (!m_pProtocol)
			return E_FAIL;

		// Initialize fields:
		//
		m_pProtocol->SetNetSettings (m_pNetSettings);

		// Copy login and password:
		//
		m_pProtocol->SetLogin	 ( m_szLogin );
		m_pProtocol->SetPassword (m_szPassword);

		// This is not a part of QCNetworkSettings.
		//
		m_pProtocol->SetAllowAuthenticate (m_bAllowAuthenticate);

		// Set user callback.
		m_pProtocol->SetUserCallback(m_pUserCallback);
		m_pProtocol->SetUserCallbackData(m_ulUserData);

		m_pProtocol->m_ServiceID		= m_ServiceID;

		// Library names:
		//
		if ( !m_szKrbLibName.IsEmpty() )
		{
			m_pProtocol->SetKrbLibName (m_szKrbLibName);
		}

		if ( !m_szDesLibName.IsEmpty() )
		{
			m_pProtocol->SetDesLibName (m_szDesLibName);
		}

		if ( !m_szGssLibName.IsEmpty() )
		{
			m_pProtocol->SetGssLibName (m_szGssLibName);
		}
	}

	// If we compiling the debug version, set the stream's debugging on.
#ifdef _DEBUG
	options |= OP_DEBUG;
#endif

	// If no mailbox, halfopen (i.e. a control stream).
	if (!Mailbox)
		options |= (OP_CONTROLONLY | OP_SILENT);

	// Return this.
	//
	HRESULT hResult = E_FAIL;

	//  Note here: c-client will return our stream if success, otherwise NIL.
	// In any case, it no longer frees our stream.
	//
	RCODE rc = m_pProtocol->Open (Mailbox, options);

	// Did we fail because the login failed??
	//
	if (rc == R_LOGIN_FAILED)
	{
		hResult = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_LOGON_NOT_GRANTED);
	}
	else if ( RCODE_SUCCEEDED (rc) )
	{
		// If we were trying to SELECT a mailbox and the stream just a control
		// stream, then fail.
		// 
		if ( Mailbox && !m_pProtocol->IsSelected() )
			bResult = FALSE;
		else
			// We succeeded.
			bResult = TRUE;

		hResult = bResult ? S_OK : E_FAIL;
	}
	else
	{
		hResult = E_FAIL;
	}

	return hResult;
}
	



// Methods for initiating the connection to the IMAP server.

// FUNCTION:
// * Establishes the initial connection to the IMAP server.
// * Issues the LOGIN command.
// * Does NOT select a mailbox.
// END FUNCTION

// NOTES:
// * Returns FALSE if attempt fails.
// END NOTES

HRESULT CMImap::OpenControlStream()
{
	// Must lock the object.
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// If we have an open and Authenticated stream, return this.
	//
	if ( IsAuthenticated() )
		return S_OK;

	return OpenStream (NULL);
}





// FUNCTION
// * Performs an IMAP "SELECT" on the mailbox. If the connection to the server has not
// * already been made, do that as well.
// END FUNCTION

// NOTES
// * Returns TRUE if success.
// END NOTES

HRESULT CMImap::OpenMailbox  (void)
{
	BOOL bResult = FALSE;

	// Must lock the object.
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// check here!!
	//
	if ( IsSelected() )
		return S_OK;

	// Must have a name.
	if (!m_pName)
		return E_FAIL;

	// Go open or re-open the stream.
	return OpenStream (m_pName);
}



// FUNCTION
// * Performs an IMAP "SELECT" on the mailbox. If the connection to the mailbox has not
// * already been made, do that as well.
// END FUNCTION

// NOTES
// * Returns TRUE if success.
// END NOTES

HRESULT CMImap::OpenMailbox  (const char *pMailboxName)
{
	if (!pMailboxName)
		return E_INVALIDARG;

	// Attempt to lock the opject.
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// check here!!
	//
	if ( IsSelected() )
		return S_OK;

	// Copy pMailboxname.
	if (m_pName)
		fs_give ( (void **)&m_pName);

	m_pName = cpystr (pMailboxName);

	// Now call above method.
	return OpenMailbox ();
}



// RequestStop [PUBLIC]
//
void CMImap::RequestStop()
{
	if (m_pProtocol)
		m_pProtocol->Stop();
}




// FUNCTION: Noop
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY

HRESULT CMImap::Noop ()
{
	// Must have a stream.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);

	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Do the command
	RCODE rc = m_pProtocol->Ping ();

	return RCODE_SUCCEEDED (rc) ? S_OK : E_FAIL;
}




// Convert from a UID to a msgno.
HRESULT CMImap::UidToMsgmno (IMAPUID uid, DWORD* dwMsgno)
{
	DWORD ulResult = 0;

	// In case we fail.
	*dwMsgno = 0;
	
	if (!m_pProtocol)
		return E_FAIL;

	// Must have a non-NULL dwMsgno:
	if (NULL == dwMsgno)
		return E_INVALIDARG;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);

	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Do the command.
	ulResult = m_pProtocol->GetMsgno (uid);

	if (ulResult > 0)
	{
		*dwMsgno = ulResult;
		return S_OK;
	}
	else
		return E_FAIL;
}





// Fetches.

// Top level message only.

// FUNCTION: UIDFetchEnvelope.
// END FUNCTION

// NOTES
// Caller MUST free *ppEnvelope using FreeMemory()!!

// END NOTES
HRESULT CMImap::UIDFetchEnvelope (unsigned long uid, ENVELOPE **ppEnvelope)
{
	// Verify parameter.
	if (!ppEnvelope)
		return E_FAIL;

	// In case we fail.
	*ppEnvelope = NULL;

	if (! m_pProtocol)
		return E_INVALIDARG;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Do the command.
	*ppEnvelope = m_pProtocol->FetchEnvelope (uid, FT_UID);

	if (*ppEnvelope)
		return S_OK;
	else
		return E_FAIL;
}




//
// UIDFetchEnvelopeAndExtraHeaders [PUBLIC]
// Fetch envelope and specified headers in a single command.
//
HRESULT	CMImap::UIDFetchEnvelopeAndExtraHeaders (
						IMAPUID		Uid, 
						ENVELOPE**	ppEnvelope,
						char*		pFields,
						CWriter*	pWriter)
{
	// Verify parameter.
	// Note: pFields and pWriter can be NULL.
	//
	if (!ppEnvelope)
		return E_FAIL;

	// In case we fail.
	*ppEnvelope = NULL;

	if (! m_pProtocol)
		return E_INVALIDARG;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Convert Fields to a STRINGLIST.
	STRINGLIST	*slist = NULL;

	// For writing header text.
	//
	CMailGets *Mailgets = NULL;

	if (pFields)
		slist = CommaSeparatedTextToStringlist (pFields);

	if (slist && pWriter)
	{
		// Allocate a CMailGets object.
		Mailgets = new CMailGets (m_pProtocol, pWriter);

		if (Mailgets)
		{
			// Set a pointer to this class into "stream".
			m_pProtocol->mgets_data = (void *)Mailgets;

			Mailgets->SetMailGets();
		}
	}

	// Go do th fetch now:

	RCODE rc = m_pProtocol->FetchHeader ( Uid, NULL, slist, FT_UID | FT_PEEK, ppEnvelope );

	// Cleanup:
	if (Mailgets)
	{
		Mailgets->ResetMailGets();

		// Reset this because the CWriter class is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}

	if (slist)
	{
		// Cleanup.
		mail_free_stringlist (&slist);
	}			

	if (*ppEnvelope)
		return S_OK;
	else
		return E_FAIL;
}





// FUNCTION:  UIDFetchStructure
// END FUNCTION

// NOTES
// The caller MUST NOT free the retuirned BODY using FreeBodyStructure below!!
// END NOTES
HRESULT CMImap::UIDFetchStructure	(unsigned long uid, BODY **ppBody)
{
	// Verify parameters:
	if (!ppBody)
		return E_FAIL;

	// In case we fail:
	*ppBody = NULL;

	if (!m_pProtocol)
		return E_INVALIDARG;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Get the BODY.
	*ppBody = m_pProtocol->FetchStructure (uid, FT_UID);

	if (*ppBody)
		return S_OK;
	else
		return E_FAIL;
}




// FUNCTION:  FreeBodyStructure
// END FUNCTION

// NOTES
// Free a body structure reutrned by UIDFetchStructure;
// END NOTES
void CMImap::FreeBodyStructure	(BODY *pBody)
{
	if (pBody)
		mail_free_body (&pBody);
}




// FUNCTION:  UIDFetchFlags
// Fetch flags for the given UID sequence.
// END FUNCTION
// NOTES
// If pSequence is NULL, send UID FETCH 1:* FLAGS
// BUG: Need to determine if we retrieved flags for all messages or if an
// error ocurred.
// END NOTES
HRESULT CMImap::UIDFetchFlags (const char *pSequence, CEltWriter *pEltWriter)
{
	char *pS = NULL;
	BOOL bResult = FALSE;

	// Must have an open stream.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Set the EltWriter into the stream. Note: pEltWriter can be NULL, in which case
	// the flags will be fetched but wouldn't be passed to the upper layers.
	m_pProtocol->m_pEltWriter = (void *)pEltWriter;

	// Make our own copy.
	if (!pSequence)
	{
		pS = cpystr ("1:*");
	}
	else	
	{
		pS = cpystr (pSequence);
	}

	if (pS)
	{
		bResult = RCODE_SUCCEEDED ( m_pProtocol->FetchFlags (pS, FT_UID) );
		
		fs_give ((void **)&pS);
	}

	// Make sure.
	m_pProtocol->m_pEltWriter = NULL;

	return bResult ? S_OK : E_FAIL;
}




// FreeMemory [PUBLIC]
// FUNCTION
// FREE memory that was allocated by the DLL. Example, memory allocated in UIDFetchFlags,
// UIDFetchFast or UIDFetchFull.
// END FUNCTION

void CMImap::FreeMemory(void **pMem)
{
	if (pMem && *pMem)
	{
		fs_give (pMem);
	}
}				



void CMImap::DestroyEnvelope(ENVELOPE **ppEnvelope)
{
	if (ppEnvelope)
		mail_free_envelope (ppEnvelope);
}




// FUNCTION: UIDFetchFast
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchFast (unsigned long uid, IMAPFULL **ppFlags)
{
	// Sanity:
	if (!ppFlags)
		return E_FAIL;

	// Coddle compiler:
	uid = uid;

	return E_FAIL;
}


// FUNCTION: UIDFetchAll
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchAll (unsigned long uid, IMAPFULL **ppFlags)
{
	// Sanity:
	if (!ppFlags)
		return E_FAIL;

	// Coddle compiler:
	uid = uid;

	return E_FAIL;
}



// FUNCTION: UIDFetchFull
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchFull	(unsigned long uid, IMAPFULL **ppFlags)
{
	// Sanity:
	if (!ppFlags)
		return E_FAIL;

	// Coddle compiler:
	uid = uid;

	return E_FAIL;
}


// FUNCTION: UIDFetchInternalDate
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchInternalDate(unsigned long uid, char **ppDate)
{
	// Sanity:
	if (!ppDate)
		return E_FAIL;

	// Coddle compiler:
	uid = uid;

	return E_FAIL;
}


// FUNCTION: UIDFetchHeader
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchHeader	(unsigned long uid, CWriter *Writer)
{
	BOOL	bResult = TRUE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;

	// Should be open.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID | FT_PEEK;		// We are using the UID* command.

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();

		bResult = RCODE_SUCCEEDED ( m_pProtocol->FetchHeader (uid, NULL, NULL, flags, NULL) );

		Mailgets->ResetMailGets();

		// Reset this because the CWriter object is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}


	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: UIDFetchMessage
// Feth the complete RFC822 message, INCLUDING the message header.
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchMessage  (unsigned long uid, CWriter *Writer, BOOL Peek)
{
	BOOL	bResult = FALSE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;
		
	if (!m_pProtocol)
		return E_FAIL;

	
	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.
	if (Peek)
		flags |= FT_PEEK;

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();

		bResult = RCODE_SUCCEEDED( m_pProtocol->FetchMessage (uid, flags) );

		Mailgets->ResetMailGets();

		// Reset this because the CWriter object is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}

	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: UIDFetchMessageBody
// Fetch the text of the message body, EXCLUDING the message header.
// END FUNCTION

// NOTES
// Peek is ignored for now.
// END NOTES
HRESULT CMImap::UIDFetchMessageBody (unsigned long uid, CWriter *Writer, BOOL Peek)
{
	BOOL	bResult = FALSE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;
		
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.
	if (Peek)
		flags |= FT_PEEK;

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();
		bResult = RCODE_SUCCEEDED (m_pProtocol->FetchText (uid, "1", flags) );
		Mailgets->ResetMailGets();

		// Reset this because the CWriter object is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}


	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: UIDFetchPartialMessage
// Feth the complete RFC822 message, INCLUDING the message header.
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchPartialMessage  (unsigned long uid, unsigned long first, unsigned long nBytes,
			 CWriter *Writer, BOOL Peek)
{
	BOOL	bResult = FALSE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;
		
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.
	if (Peek)
		flags |= FT_PEEK;

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();

		bResult = RCODE_SUCCEEDED ( m_pProtocol->PartialBody (uid, "", first, nBytes, flags) );

		Mailgets->ResetMailGets();

		// Reset this because the CWriter object is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}


	return bResult ? S_OK : E_FAIL;
}






// FUNCTION: UIDFetchMessageBody
// Fetch the text of the message body, EXCLUDING the message header.
// END FUNCTION

// NOTES
// Peek is ignored for now.
// END NOTES
HRESULT CMImap::UIDFetchPartialMessageBody (unsigned long uid, unsigned long first, unsigned long nBytes,
				CWriter *Writer, BOOL Peek)
{
	BOOL	bResult = FALSE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;
		
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.
	if (Peek)
		flags |= FT_PEEK;

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();
		bResult = RCODE_SUCCEEDED ( m_pProtocol->PartialBody (uid, NULL, first, nBytes, flags) );
		Mailgets->ResetMailGets();

		// Reset this because the CWriter object is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}

	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: UIDFetchPartialBodyText
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDFetchPartialBodyText (unsigned long uid, char *sequence, unsigned long first,
				unsigned long nBytes, CWriter *Writer, BOOL Peek)
{
	BOOL	bResult = FALSE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;
		
	if (!m_pProtocol)
		return E_FAIL;


	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.
	if (Peek)
		flags |= FT_PEEK;

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();

		bResult = RCODE_SUCCEEDED (m_pProtocol->PartialBody (uid, sequence, first, nBytes, flags));

		Mailgets->ResetMailGets();

		// Reset this because the CWriter object may have gone away.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}


	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: FetchUID
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::FetchUID (unsigned long msgNum, IMAPUID *pUid)
{
	unsigned long ulResult = 0L;

	// Sanity:
	if (!pUid)
		return E_INVALIDARG;

	// If case we fail:
	*pUid = 0;

	// Must have a stream.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	*pUid = m_pProtocol->GetUid (msgNum);

	return (*pUid > 0) ? S_OK : E_FAIL;
}





// These apply to a "message/rfc822"part. (Can also be used for top-level message.)

// FUNCTION: UIDFetchRFC822Header
// END FUNCTION

// NOTES
// "pSequence" mcan be NULL, in which case, we're fetching the top level message header.
// END NOTES
HRESULT  CMImap::UIDFetchRFC822Header (unsigned long uid, char *pSequence, CWriter *Writer)
{
	BOOL	bResult = TRUE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;

	// Should be open.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID | FT_PEEK;		// We are using the UID* command.

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();

		bResult = RCODE_SUCCEEDED (m_pProtocol->FetchHeader (uid, pSequence, NULL, flags, NULL));

		Mailgets->ResetMailGets();

		// Reset this because the CWriter object is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}


	return bResult ? S_OK : E_FAIL;
}






// FUNCTION: UIDFetchRFC822Text
// END FUNCTION

// NOTES
// END NOTES
HRESULT CMImap::UIDFetchRFC822Text (unsigned long uid, char *sequence, CWriter *Writer)
{
	// Sanity:
	if (! (uid && sequence && Writer) )
		return E_FAIL;

	return E_FAIL;
}


// FUNCTION: UIDFetchRFC822HeaderFields
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDFetchRFC822HeaderFields (unsigned long uid, char *sequence, char *Fields, CWriter *Writer)
{
	STRINGLIST	*slist;
	BOOL		bResult = FALSE;

	// Sanity. sequence can be NULL.
	if (!(uid && Fields && Writer))
		return E_INVALIDARG;

	// Must have a stream.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Convert Fields to a STRINGLIST.
	slist = CommaSeparatedTextToStringlist (Fields);
	if (slist)
	{

		// Allocate a CMailGets object.
		CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

		if (Mailgets)
		{
			// Set a pointer to this class into "stream".
			m_pProtocol->mgets_data = (void *)Mailgets;

			Mailgets->SetMailGets();

			bResult = RCODE_SUCCEEDED (m_pProtocol->FetchHeader (uid, sequence, slist, FT_UID | FT_PEEK, NULL));

			Mailgets->ResetMailGets();

			// Reset this because the CWriter class is temporary.
			m_pProtocol->mgets_data = NULL;

			delete Mailgets;
	
			// Cleanup.
			mail_free_stringlist (&slist);
		}
	}			

	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: UIDFetchRFC822HeaderFieldsNot
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDFetchRFC822HeaderFieldsNot (unsigned long uid, char *sequence, char *fields, CWriter *Writer)
{
	// Sanity:
	if (! (uid && sequence && fields && Writer) )
		return E_FAIL;

	return E_FAIL;
}




// BODY part level fetches.


// FUNCTION: UIDFetchMimeHeader
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// * Implemented by jdbyd 8/24/99
// END HISTORY
HRESULT CMImap::UIDFetchMimeHeader (unsigned long uid, char *sequence, CWriter *Writer)
{
	BOOL bResult = false;
	unsigned long flags;

	// Sanity:
	if (! (uid && sequence && Writer) )
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();
		bResult = RCODE_SUCCEEDED (m_pProtocol->FetchMIME (uid, sequence, flags));
		Mailgets->ResetMailGets();

		// Reset this because the CWriter class may have gone away.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}

	return bResult ? S_OK : E_FAIL;
}



// FUNCTION: UIDFetchBodyText
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDFetchBodyText (unsigned long uid, char *sequence, CWriter *Writer, BOOL Peek)
{
	BOOL	bResult = FALSE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;
		
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = FT_UID;		// We are using the UID* command.
	if (Peek)
		flags |= FT_PEEK;

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();
		bResult = RCODE_SUCCEEDED (m_pProtocol->FetchBody (uid, sequence, flags));
		Mailgets->ResetMailGets();

		// Reset this because the CWriter class may have gone away.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}

	return bResult ? S_OK : E_FAIL;
}






// Preamble and trailer text fetch (use complicated method).


// FUNCTION: UIDFetchPreamble
// END FUNCTION

// NOTES
// * If "sequence" is NULL, apply to top level message. Must refer to a multipart message,
// * an embedded message/rfc822 part that has a multipart body, or a multipart sub-part.
// * or part. 
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDFetchPreamble(unsigned long uid, char *sequence, CWriter *Writer)
{
	// Sanity:
	if (! (uid && sequence && Writer) )
		return E_FAIL;

	return E_FAIL;
}


// FUNCTION: UIDFetchTrailer
// END FUNCTION

// NOTES
// * If "sequence" is NULL, apply to top level message. Must refer to a multipart message,
// * an embedded message/rfc822 part that has a multipart body, or a multipart sub-part.
// * or part. 
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDFetchTrailer(unsigned long uid, char *sequence, CWriter *Writer)
{
	// Sanity:
	if (! (uid && sequence && Writer) )
		return E_FAIL;

	return E_FAIL;
}



// Methods for STORE'ing flags. For IMAP commands that return new values of flags,
// the new values are returned in the IMAPFLAGS parameter.
// BUG: These methods should be extended to handle uid sets.


// FUNCTION: UIDSaveFlags
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDSaveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent)
{
	// Sanity:
	if (! (uid && Flags) )
		return E_FAIL;

	// Coddle compiler
	Silent = Silent;


	return E_FAIL;
}


// FUNCTION: UIDAddFlags
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDAddFlags  (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent)
{
	// Sanity:
	if (! (uid && Flags) )
		return E_FAIL;

	// Coddle compiler
	Silent = Silent;

	return E_FAIL;
}


// FUNCTION: UIDRemoveFlags
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDRemoveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent)
{
	// Sanity:
	if (! (uid && Flags) )
		return E_FAIL;

	// Coddle compiler
	Silent = Silent;

	return E_FAIL;
}


//
// Issue a STORE -flags command.
// pUidList is a comma-separated string of uid's,
// pFlagList is a space-separated parenthesized list of flags, e.g.
// (\Seen \Deleted)
//
HRESULT CMImap::UIDRemoveFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */)
{
	unsigned long flags;
	TCHAR	sflaglist[1024];

	// Sanity:
	if (!(pUidList && pFlagList))
		return E_INVALIDARG;

	/// Must have a stream.		
	if (! IsSelected () )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Copy in local buffer!!!
	if ( strlen (pFlagList) >= sizeof (sflaglist) )
	{
		return E_FAIL;
	}

	// Copy them
	strcpy (sflaglist, pFlagList);

	// Setup flags. ALways use UID's. 
	// Note that we a re REMOVING flags. It we were SETTING them, 
	// we'd have to add the ST_SET flag.
	//
	flags = ST_UID;

	if (Silent)
		flags |= ST_SILENT;

	RCODE rc = m_pProtocol->SetFlags (pUidList, sflaglist, flags);

	return SUCCEEDED(rc) ? S_OK : E_FAIL;
}






//
// Issue a STORE +flags command.
// pUidList is a comma-separated string of uid's,
// pFlagList is a space-separated parenthesized list of flags, e.g.
// (\Seen \Deleted)
// NOTE: pUidList may be long!!
//
HRESULT CMImap::UIDAddFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */)
{
	unsigned long flags;
	TCHAR	sflaglist[1024];

	// Sanity:
	if (!(pUidList && pFlagList))
		return E_INVALIDARG;

	/// Must have a stream.		
	if (! IsSelected () )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Copy in local buffer!!!
	if ( strlen (pFlagList) >= sizeof (sflaglist) )
	{
		return E_FAIL;
	}

	// Copy flaglist.
	strcpy (sflaglist, pFlagList);

	// Setup flags. ALways use UID's. Note that we are SETTING flags.
	flags = ST_UID | ST_SET;

	if (Silent)
		flags |= ST_SILENT;

	RCODE rc = m_pProtocol->SetFlags (pUidList, sflaglist, flags);

	return SUCCEEDED(rc) ? S_OK : E_FAIL;
}





// COPY/MOVE methods. Destination mailbox MUST be on the same server.
// BUG: Should be extended to handle message sets.


// FUNCTION: UIDCopy
// END FUNCTION

// NOTES
// We do NOT assume OPTIMIZE-1 here, so the server does NOT return a list
// of copied uid's. We would want to have UIDCopy return a list of UID's
// at some point in time but not now.
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDCopy (const char *pUidlist, const char *pDestMailbox)
{
	char	*pList = NULL;
	char	*pMbox = NULL;
	BOOL	bResult = FALSE;

	// Argument sanity.
	if (!(pUidlist && pDestMailbox))
		return E_INVALIDARG;

	// The stream MUST be open, otherwise fail.
	if ( !(m_pProtocol && IsSelected()) )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Copy Uidlist and pDestMaiobox into internal buffer to avoid const char* -> char * cast errors.
	//  Yuck.
	pList = cpystr (pUidlist);
	pMbox = cpystr (pDestMailbox);

	if (pList && pMbox)
	{
		char *p  = pList;
		char *q;
		int nUids;
		BOOL	bThisResult;

		// Loop through pList and send chucks of uid's at a time.
		bThisResult = TRUE;

		while (p && bThisResult)
		{
			//
			// send this in smaller chunks!!
			//
			q = strchr (p, Comma);

			for (nUids = 0; q && nUids < MAX_UIDS_PER_COMMAND; nUids++)
			{
				q = strchr (q + 1, Comma);
			}

			// At this point, it q is non-NULL, it points to a comma.
			// Temporarily tie it off.
			// If q is NULL, then this is the last chunk.
			if (q)
			{
				*q = 0;
			}

			// Send this chunk.
			//
			if (p && *p)
			{
				bThisResult =  RCODE_SUCCEEDED (m_pProtocol->Copy (p, pMbox, CP_UID));
			}

			if (!bThisResult)
				break;

			bResult = bResult || bThisResult;

			// Setup for next chunk.
			if (q)
			{
				// Put back comma.
				*q = Comma;
				p = q + 1;

				// In case have "3,4,5,"
				if (!*p)
					break;
			}
			else
			{
				// Stop loop.
				p = NULL;
			}
		}
	}

	if (pList)
		FreeMemory ((void **)&pList);
	if (pMbox)
		FreeMemory ((void **)&pMbox);

	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: Append
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
HRESULT CMImap::AppendMessage (const char* pFlags, unsigned long Seconds, STRING *pMsg)
{
	char date [MAILTMPLEN + 4];
	char flgs [MAILTMPLEN + 4];
	BOOL bResult = FALSE;

	// Sanity
	if ( !( pFlags && pMsg ) )
		return E_INVALIDARG;

	// Should be open.
	if (!m_pProtocol)
		return E_FAIL;

	// Must also have a name.
	if (!m_pName)
		return FALSE;

	// Must be SELECT'ed
	if ( !IsSelected() )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Copy pFlags to an internal buffer
	*flgs = 0;
	if ( strlen (pFlags) < MAILTMPLEN )
		strcpy (flgs, pFlags);

	if (Seconds)
	{
		// Format current date.
		*date = 0;
		append_date (date, Seconds);

		// Do the append
		bResult = RCODE_SUCCEEDED (m_pProtocol->Append (m_pName, flgs, date, pMsg));
	}
	else
	{
		bResult = RCODE_SUCCEEDED (m_pProtocol->Append (m_pName, flgs, NULL, pMsg));
	}

	return bResult ? S_OK : E_FAIL;
}





// Utilities.

// FUNCTION: UIDMessageIsMultipart
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::UIDMessageIsMultipart (unsigned long uid, BOOL *pVal)
{
	BODY *pBody = NULL;
	BOOL bResult = FALSE;

	// Sanity:
	if (!pVal)
		return E_INVALIDARG;

	// In case we fail.
	*pVal = 0;

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Look at the top body's type.
	pBody = m_pProtocol->FetchStructure (uid, FT_UID);

	bResult = pBody && (pBody->type == TYPEMULTIPART);

	*pVal = bResult;

	return bResult ? S_OK : E_FAIL;
}


// FUNCTION: UIDValidity
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
UIDVALIDITY CMImap::UIDValidity ()
{
	if (m_pProtocol)
		return m_pProtocol->GetUidValidity();
	else
		return 0;
}






// FUNCTION: GetMessageCount
// Always get the message count from the stream.
// END FUNCTION

unsigned long CMImap::GetMessageCount ()
{
	if (m_pProtocol)
		return m_pProtocol->GetNumberOfMessages();
	else
		return 0;
}


// FUNCTION: GetTopMboxDelimiter
// Fetch the top-level mailbox delimiter char by sending
// a LIST "" "".
//
// END FUNCTION

HRESULT CMImap::GetTopMboxDelimiter (TCHAR *pDelimiter)
{
	// Sanity:
	if (!pDelimiter)
		return E_INVALIDARG;

	// Make sure:
	*pDelimiter = 0;

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Do the list.
	m_pProtocol->List ("", "");

	// We must get a non-zero delimiter.
	//
	TCHAR ch = m_pProtocol->GetDelimiterChar();

	if (ch)
	{
		*pDelimiter = ch;
		return S_OK;
	}
	else
		return E_FAIL;
}






// =========== non-UID functions ==================/

// FUNCTION: FetchHeader
// END FUNCTION

// NOTES
// This is same as UIDFetchHeader except that the FT_UID flag is NOT passed
// to mail_fetchheader.
// END NOTES
HRESULT CMImap::FetchHeader (unsigned long msgNum, CWriter *Writer)
{
	BOOL	bResult = TRUE;
	unsigned long flags;

	if (!Writer)
		return E_INVALIDARG;

	// Should be open.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Setup flags.
	flags = 0L;			// msgno fetch - nu UID this time.

	// Allocate a CMailGets object.
	CMailGets *Mailgets = new CMailGets (m_pProtocol, Writer);

	if (Mailgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mgets_data = (void *)Mailgets;

		Mailgets->SetMailGets();

		bResult = RCODE_SUCCEEDED (m_pProtocol->FetchHeader (msgNum, NULL, NULL, flags, NULL));

		Mailgets->ResetMailGets();

		// Reset this because the CWriter class is temporary.
		m_pProtocol->mgets_data = NULL;

		delete Mailgets;
	}
	
	return bResult ? S_OK : E_FAIL;
}





// Collect the "Internal date" into seconds since 1970 and return.
HRESULT CMImap::UIDGetInternalDate (IMAPUID uid, unsigned long *pSeconds)
{
#if 0 // BUG - MUST FIX
	struct tm time;
	MESSAGECACHE	*elt = NULL;
#endif 

	// Sanity:
	if (! (uid && pSeconds) )
		return E_INVALIDARG;

#if 0 // BUG - Must fix.

	// Initialize.
	memset (&time, 0, sizeof(struct tm));

	// Sanity.
	if (!m_pProtocol)
		return E_FAIL;

	// Get the MESSAGECACHE for this message.
	// BUG:: Should make sure the msgno is valid!!
	elt = m_pProtocol->MailElt ();

	if (elt)
	{
		time.tm_mon = elt->month;
		// elt->year is years since 1969.
		if (elt->year < 1)
			time.tm_year = 0;
		else
			time.tm_year = elt->year - 1;
		time.tm_mday = elt->day;
		time.tm_hour = elt->hours;
		time.tm_min  = elt->minutes;
		time.tm_sec  = elt->seconds;
	
		time.tm_isdst = -1;
	}

	time_t seconds = mktime(&time);

	return (seconds < 0L ? 0L : seconds);

#endif // JOK BUG

	return E_FAIL;  // FORNOW
}





// NOTES
// A return value of 0 indicates an error.
// END NOTES

HRESULT CMImap::GetRfc822Size (IMAPUID uid,unsigned long *pSize)
{
	// Sanity:
	if (!pSize)
		return E_INVALIDARG;

	// Safety:
	*pSize = 0;

	if (!(m_pProtocol && IsSelected()))
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	*pSize = m_pProtocol->FetchRfc822size (uid, FT_UID);

	return S_OK;
}





//===== Mailbox level methods =========/

// FUNCTION: Check
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::Check()
{
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Do the command.
	m_pProtocol->Check ();

	return S_OK;
}





// UIDDeleteMessages [PUBLIC]
// FUNCTION
// Set the delete flag on the given messages.
// If Expunge is TRUE, send an expunge afterwards.
// END FUNCTION

HRESULT CMImap::UIDDeleteMessages (LPCSTR pUidList, BOOL Expunge)
{
	if (!pUidList)
		return E_INVALIDARG;

	// We must already be connected. 
	if (!(m_pProtocol && IsSelected()))
	{
		ASSERT (0);
		return FALSE;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	BOOL bResult = SUCCEEDED ( m_pProtocol->SetFlags (pUidList, "\\Deleted", ST_UID | ST_SET | ST_SILENT) );

	if (bResult && Expunge)
		m_pProtocol->Expunge ();

	return bResult ? S_OK : E_FAIL;
}





// UIDUnDeleteMessages [PUBLIC]
// FUNCTION
// Set the delete flag on the given messages.
// If Expunge is TRUE, send an expunge afterwards.
// END FUNCTION

HRESULT CMImap::UIDUnDeleteMessages (LPCSTR pUidList)
{
	if (!pUidList)
		return E_INVALIDARG;

	// We must already be connected. 
	if (!(m_pProtocol && IsSelected()))
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	//
	// Note: If some got undeleted, return TRUE.
	//

	BOOL bResult = SUCCEEDED ( m_pProtocol->SetFlags (pUidList, "\\Deleted", ST_UID | ST_SILENT) );

	return bResult ? S_OK : E_FAIL;
}





// FUNCTION: Expunge
// END FUNCTION

// NOTES
// This will eventually implement the OPTIMZE-1 UID Expunge().
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// BUG: This doeas a simple expunge, NOT a UID expunge!!
// END HISTORY
HRESULT CMImap::UIDExpunge (LPCSTR pUidList, CString& szUidsActuallyRemoved)
{
	if (!(m_pProtocol && IsSelected()))
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Note: pUidList can be NULL!!

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	HRESULT hResult = E_FAIL;

	if ( RCODE_SUCCEEDED (m_pProtocol->UIDExpunge (pUidList, szUidsActuallyRemoved) ) )
		hResult = S_OK;

	return hResult;
}


// FUNCTION: Expunge
// END FUNCTION

// NOTES
// This will eventually implement the OPTIMZE-1 UID Expunge().
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CMImap::Expunge ()
{
	if (!(m_pProtocol && IsSelected()))
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	m_pProtocol->Expunge ();

	// BUG: Must determine if we really expunged.

	return S_OK;
}




// RecreateMessageMap [PUBLIC]
//
// Recreate stream's messagenumber-to-uid map.
HRESULT CMImap::RecreateMessageMap ()
{
	if (!(m_pProtocol && IsSelected()))
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	HRESULT hResult = E_FAIL;

	if ( RCODE_SUCCEEDED (m_pProtocol->RecreateMessageMapMT ()) )
		hResult = S_OK;

	return hResult;
}





// FUNCTION: Logout
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// * Modified by jdboyd 8/02/99 Now sending LOGOUT command if still connected
// END HISTORY
HRESULT CMImap::Logout ()
{
	if (m_pProtocol) m_pProtocol->Close(0);	// issue the LOGOUT command if appropriate

	return E_FAIL;
}


// FUNCTION: IsSelected
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
BOOL CMImap::IsSelected ()
{
	if (!m_pProtocol)
		return FALSE;

	// See if the stream is selected.
	return m_pProtocol->IsSelected();
}


// Methods for querying the state of the IMAP stream.
BOOL CMImap::IsConnected()
{
	if (!m_pProtocol)
		return FALSE;

	return m_pProtocol->IsConnected ();
}



// FUNCTION: IsAuthenticated
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
BOOL CMImap::IsAuthenticated ()
{
	if (!m_pProtocol)
		return FALSE;

	// See if the stream is selected.
	return m_pProtocol->IsAuthenticated();
}




// Create a "pat" string for imapmail_list using the "%".
// Pattern is of the form: {machine:port}user.imap.%
// Note: pReference CAN be NULL.
// Note: "pReference" must end in the appropriate mailbox name delimiter 
// character!!
char* CMImap::CreatePatSome (char *pReference)
{
	return CreatePat (pReference, '%');
}




// Create a "pat" string for imapmail_list using the "*".
// Pattern is of the form: {machine:port}user.imap.*
// Note: "pReference" must end in the appropriate mailbox name delimiter 
// character!!
char* CMImap::CreatePatAll (char *pReference)
{
	return CreatePat (pReference, '*');
}



// Generic routine for creating the pattern. 
// "WildCard" is either "%", or "*".
// Pattern is of the form: {machine:port}user.imap.%
// Note: pReference CAN be NULL.
char* CMImap::CreatePat (char *pReference, char WildCard)
{
	char *pat = NULL;
	long len;

	// Sanity.
	if (!(m_pServer && m_pPortNum))
		return NULL;

	// Calculate max. length we'll need. Literals needed are {:}%
	len = strlen(m_pServer) + strlen (m_pPortNum) + 4;
	if (pReference)
		len += strlen (pReference);

	// Allocate our memory.
	pat = new char[len + 4];		// A little extra space
	if (pat)
	{
		if (pReference)
			sprintf (pat, "{%s:%s}%s%c", m_pServer, m_pPortNum, pReference, WildCard);
		else
			sprintf (pat, "{%s:%s}%c", m_pServer, m_pPortNum, WildCard);
	}

	return pat;
}



// Fetch our list of un-subscribed IMAP mailboxes from "server". Return the
// list in "mboxlist".
// Note: "server" can be NULL, in which case user the m_server attribute.
// If "INCLUDE"INBOX" is TRUE, Fetch inbox as a separate command.
HRESULT CMImap::IMAPListUnSubscribed (LPCSTR pReference, CLister *Lister, BOOL INCLUDE_INBOX)
{
	char 		pattern[2048];
	HRESULT		hResult = S_OK;

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Create our CMboxGets class for passing Clister to mm_list().
	CMboxGets *Mboxgets = new CMboxGets (m_pProtocol, Lister);
	if (Mboxgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mboxgets_data = (void *)Mboxgets;

		// Fetch inbox if asked to.
		if (INCLUDE_INBOX)
		{
			m_pProtocol->List ("", "INBOX");
		}

		// Fetch rest.

		// Create a pattern based on the reference.
		if (pReference && *pReference)
		{
			// Make sure we don't exceed pattern buffer length.
			if (strlen (pReference) < (sizeof (pattern) - 1))
				sprintf (pattern, "%s%%", pReference);
			else
				sprintf (pattern, "%%");
		}
		else
			sprintf (pattern, "%%"); 
	
		// Get the folder list now.
		m_pProtocol->List ("", pattern);

		// No longer need MBoxgets. Make sure reset stream->mboxgets_data.
		m_pProtocol->mboxgets_data = NULL;
		delete Mboxgets;

		// If user cancelled, then the connection would have been broken.
		// Check for this.
		//
		if ( IsConnected () )
		{
			// Assume user cancelled.
			//
			hResult = MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_CANCELLED);
		}
		else
			hResult = S_OK;
	}

	return hResult;
}





// Do a LIST on the given mailbox so we will get it's attributes.
HRESULT CMImap::FetchMailboxAttributes (LPCSTR pImapName, CLister *Lister)
{
	char 		pattern[MAILTMPLEN + 4];
	BOOL		bResult = FALSE;

	if (!(pImapName && Lister))
		return E_INVALIDARG;

	// If the name is too long, reject it.
	if (strlen (pImapName) > MAILTMPLEN)
		return E_FAIL;

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Create our CMboxGets class for passing Clister to mm_list().
	CMboxGets *Mboxgets = new CMboxGets (m_pProtocol, Lister);
	if (Mboxgets)
	{
		// Set a pointer to this class into "stream".
		m_pProtocol->mboxgets_data = (void *)Mboxgets;

		// Do a LIST on the mailbox.

		strcpy (pattern, pImapName);
		m_pProtocol->List ("", pattern);

		// No longer need MBoxgets. Make sure reset stream->mboxgets_data.
		m_pProtocol->mboxgets_data = NULL;
		delete Mboxgets;

		bResult = TRUE;
	}	

	return bResult ? S_OK : E_FAIL;
}





// Do a LIST on the given mailbox so we will get it's attributes.
HRESULT CMImap::CreateMailbox (LPCSTR pImapName)
{
	char 		pattern[MAILTMPLEN + 4];
	BOOL		bResult = FALSE;

	if (!pImapName)
		return E_INVALIDARG;

	// If the name is too long, reject it.
	if (strlen (pImapName) > MAILTMPLEN)
		return E_FAIL;

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return FALSE;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Attempt to create it.
	strcpy (pattern, pImapName);

	bResult = RCODE_SUCCEEDED (m_pProtocol->Create (pattern));

	return bResult ? S_OK : E_FAIL;
}




// Delete a mailbox..
HRESULT CMImap::DeleteMailbox (LPCSTR pImapName)
{
	char 		pattern[MAILTMPLEN + 4];
	BOOL		bResult = FALSE;

	if (!pImapName)
		return E_INVALIDARG;

	// If the name is too long, reject it.
	if (strlen (pImapName) > MAILTMPLEN)
		return E_FAIL;

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Attempt to create it.
	strcpy (pattern, pImapName);
	bResult = RCODE_SUCCEEDED (m_pProtocol->Delete (pattern));

	return bResult ? S_OK : E_FAIL;
}





// Rename a mailbox..
HRESULT CMImap::RenameMailbox (LPCSTR pImapName, LPCSTR pNewImapName)
{
	char 		oldName[MAILTMPLEN + 2];
	char		newName[MAILTMPLEN + 2];
	BOOL		bResult = FALSE;

	if (! (pImapName && pNewImapName) )
		return E_INVALIDARG;

	// We need to copy names into local buffers.
	if (strlen (pImapName) >= sizeof (oldName) )
		return E_FAIL;

	if (strlen (pNewImapName) >= sizeof (newName) )
		return E_FAIL;

	// Copy the names.
	strcpy (oldName, pImapName);
	strcpy (newName, pNewImapName);

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Attempt to rename it.
	bResult = RCODE_SUCCEEDED (m_pProtocol->Rename (oldName, newName));

	return bResult ? S_OK : E_FAIL;
}



// Subscribe a mailbox..
HRESULT CMImap::SubscribeMailbox (LPCSTR pImapName)
{
	char 		pattern[MAILTMPLEN + 4];
	BOOL		bResult = FALSE;

	if (!pImapName)
		return E_INVALIDARG;

	// If the name is too long, reject it.
	if (strlen (pImapName) > MAILTMPLEN)
		return E_FAIL;

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Attempt to send command.
	strcpy (pattern, pImapName);
	bResult = RCODE_SUCCEEDED (m_pProtocol->Subscribe (pattern) );

	return bResult ? S_OK : E_FAIL;
}




// UnSubscribeMailbox a mailbox..
HRESULT CMImap::UnSubscribeMailbox (LPCSTR pImapName)
{
	char 		pattern[MAILTMPLEN + 4];
	BOOL		bResult = FALSE;

	if (!pImapName)
		return E_INVALIDARG;

	// If the name is too long, reject it.
	if (strlen (pImapName) > MAILTMPLEN)
		return E_FAIL;

	// If we have a stream, use it, else open temporary one.
	if (!m_pProtocol)
		OpenControlStream();

	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Do this before each new command.
	ClearErrorDatabase ();

	// Attempt to send command.
	strcpy (pattern, pImapName);
	bResult = RCODE_SUCCEEDED (m_pProtocol->Unsubscribe (pattern) );

	return bResult ? S_OK : E_FAIL;
}



//
// Return the error type as the result. If Error, return 
// IMAPERR_BAD_CODE.
// 
HRESULT CMImap::GetLastErrorStringAndType (LPSTR szErrorBuf, short nBufSize, int *pType)
{	
	int iType = IMAPERR_BAD_CODE;

	// Sanity:
	if (!pType)
		return E_INVALIDARG;

	// init:
	*pType = IMAPERR_BAD_CODE;

	if (m_pProtocol)
		iType = m_pProtocol->GetLastErrorStringAndType (szErrorBuf, nBufSize);

	*pType = iType;

	return S_OK;
}	


// FUNCTION
// Copy the tagged response buffer from the stream into the given buffer.
// END FUNCTION
HRESULT CMImap::GetLastServerMessage (LPSTR szErrorBuf, short nBufSize)
{	
	if (!m_pProtocol)
	{
		ASSERT (0);
		return E_FAIL;
	}

	return m_pProtocol->GetLastServerMessage (szErrorBuf, nBufSize);
}
	



// This may change the current error callback while it's being used.
// We avoid that by protecting the stream's m_pErrorCallback pointer by
// a MUTEX, also within the stream.
//
void CMImap::SetErrorCallback (ImapErrorFunc pImapErrorFn)
{

	if (m_pProtocol)
		m_pProtocol->SetErrorCallback (pImapErrorFn);
}


void  CMImap::ResetErrorCallback ()
{
	if (m_pProtocol)
		m_pProtocol->ResetErrorCallback ();
}



// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::IsReadOnly ()
{
	if (m_pProtocol)
		return m_pProtocol->IsReadOnly();

	return FALSE;
}



// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasStatusCommand ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasStatusCommand ();
	}

	return bResult;
}




// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::IsImap4Rev1 ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->IsImap4Rev1 ();
	}

	return bResult;
}




// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::IsImap4Only ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->IsImap4Only ();
	}

	return bResult;
}



// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::IsLowerThanImap4 ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->IsLowerThanImap4 ();
	}

	return bResult;
}




// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasOptimize1 ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasOptimize1 ();
	}

	return bResult;
}


// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasSort ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasSort ();
	}

	return bResult;
}


// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasScan ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasScan ();
	}

	return bResult;
}


// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasAcl ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasAcl ();
	}

	return bResult;
}


// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasQuota ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasQuota ();
	}

	return bResult;
}


// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasLiteralPlus ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasLiteralPlus ();
	}

	return bResult;
}


// FUNCTION
// ASk the IMAP-specific parts of the stream.
// END FUNCTION
BOOL CMImap::HasXNonHierarchicalRename ()
{
	BOOL bResult = FALSE;

	if (m_pProtocol)
	{
		bResult = m_pProtocol->HasXNonHierarchicalRename ();
	}

	return bResult;
}




// Note: ulData is opaque. Don't attempt to use it!!
//
void CMImap::RegisterUserCallback (ImapNotificationCallback_p pUserCallback, unsigned long ulData)
{
	// Cache this internally: 
	m_pUserCallback = pUserCallback;
	m_ulUserData	= ulData;

	// If we have a stream set it there too:
	if (m_pProtocol)
	{
		m_pProtocol->SetUserCallback(pUserCallback);
		m_pProtocol->SetUserCallbackData(ulData);
	}
}





// FUNCTION
// If the stream supports the STATUS command (IMAP4Rev1), Do it.
// Note: If the stream is selected and the selected mailbox is the
// same as the given mailbox, the stream's values for nmsgs,
// recent and  uidvalidity are updated if they are included in the
// status request.
//
// The status values are returned in the given MAILSTATUS structure.
// END FUNCTION
HRESULT CMImap::UIDFetchStatus (const char *pMailbox, long flags, MAILSTATUS *pStatus)
{
	BOOL bResult = FALSE;
	TCHAR buf [1024];

	// Must have a mailbox and a status.
	if (! (pMailbox && pStatus) )
		return E_INVALIDARG;

	// Must have an authenticated stream. Not necessarily selected.
	if (!m_pProtocol && IsAuthenticated ())
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// Set the stream's pMailStatus pointer to pStatus temporarily.
	m_pProtocol->ClearMailStatus();

	// Fetch the status. Must copy mailbox - yuck.
	*buf = 0;
	strcpy (buf, pMailbox);

	bResult = ( m_pProtocol->Status (buf, flags) != 0 );

	if (bResult && pStatus)
	{
		// Copy results into pStatus;
		//
		memcpy ( pStatus, m_pProtocol->GetMailStatus(), sizeof (MAILSTATUS) );
	}

	// Reset.
	m_pProtocol->ClearMailStatus();

	return bResult ? S_OK : E_FAIL;
}



/////////////////////////////////////////////////////////////////////////////////
// ClearErrorDatabase [PUBLIC]
// Note: This is not exported!!
//
// Clear the stream's error list. This is typically called before a new command.
/////////////////////////////////////////////////////////////////////////////////
void CMImap::ClearErrorDatabase ()
{
	if (m_pProtocol)
	{
		m_pProtocol->ClearAllErrorEntries();

		// Clear the last server m,essages as well.
		m_pProtocol->szSrvTRespBuf [0] = '\0';
	}
}





// Search [PUBLIC]
//
// Forward the command as-is to the protocol.
//
void CMImap::Search (char *charset, SEARCHPGM *pgm, CString& szResults)
{

	if (m_pProtocol)
		m_pProtocol->Search (charset, pgm, SE_UID, szResults);
}




//
// UIDFind [PUBLIC]
//
// FUNCTION 
// DO an IMAP Search of a mailbox. Return the UID list of message matches (if any)
// as a comma-separaped uid list in "szMatches".
// "pHeaderList" is a comma-separated list of header field names, e.g. "To,From".
// pUidStr is a comma-separated list of UID's to match on.
//
// END FUNCTION
//
HRESULT CMImap::UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr,  CString& szResults)
{
	unsigned long flags = 0;
	BOOL		  bResult = FALSE;

	// Clear results string.
	szResults.Empty();

	// Must have a stream.
	if (!m_pProtocol)
		return E_FAIL;

	// Must have a non-empty uid list.
	if (! (pUidStr && *pUidStr) )
	{
		return E_INVALIDARG;
	}

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// If the header list is NULL or empty, bBody must be TRUE.
	if ( ! ( (pHeaderList && *pHeaderList) || bBody) )
		return E_INVALIDARG;

	// pSearchString MUST be valid.
	if (! (pSearchString && *pSearchString) )
		return E_INVALIDARG;

	// Fill a search program with the criteria.
	SEARCHPGM* pPgm = (SEARCHPGM *)fs_get (sizeof (SEARCHPGM) );
	if (!pPgm)
		return E_FAIL;

	// Clear.
	memset ( (void *)pPgm, 0, sizeof (SEARCHPGM) );

	// Set the Uid range in the top level spgm.
	pPgm->uid = (SEARCHSET *) fs_get ( sizeof (SEARCHSET) );
	if (!pPgm->uid)
	{
		mail_free_searchpgm (&pPgm);
		return E_FAIL;
	}

	// Do this before each new command.
	ClearErrorDatabase ();

	//
	// Formulate the tree of search programs representing the search criteria.
	//
	bResult = FormatSearchCriteria (pPgm, pHeaderList, bBody, bNot, pSearchString);

	// Do UID search
	flags |= SE_UID;

	// Allocate a copy of the uid string.
	//

	char *pList = cpystr (pUidStr);
				
	if (bResult && pList)
	{
		// do the search in UID intervals because some servers have a limit.
		BOOL bThisResult;
		int nUids;

		bThisResult = TRUE;

		char *p = pList;
		char *q;

		while (p && bThisResult)
		{
			// send this in smaller chunks!!
			//
			q = strchr (p, Comma);

			for (nUids = 0; q && nUids < MAX_UIDS_PER_COMMAND; nUids++)
			{
				q = strchr (q + 1, Comma);
			}

			// At this point, it q is non-NULL, it points to a comma.
			// Temporarily tie it off.
			// If q is NULL, then this is the last chunk.
			if (q)
			{
				*q = 0;
			}

			// Send this chunk.
			// Set the message flags.

			if (p && *p)
			{
				// Set new uid list into the spgm.
				SetNewPgmUidList (pPgm, p);

				// Do the search.
				if (pPgm->uid)
				{
					CString szThisResult;

					m_pProtocol->Search (NULL, pPgm, flags, szThisResult);

					// Append the current results to what we're returning.
					if ( !szThisResult.IsEmpty() )
					{
						if ( !szResults.IsEmpty () )
						{
							// Ends with comma?
							//
							if ( szResults.Right ( 1 ) != Comma )
								szResults += ",";
						}

						szResults += szThisResult;
					}
				}
			}

			if (!bThisResult)
				break;

			bResult = bResult || bThisResult;

			// Setup for next chunk.
			if (q)
			{
				// Put back comma.
				*q = Comma;
				p = q + 1;

				// In case have "3,4,5,"
				if (!*p)
					break;
			}
			else
			{
				// Stop loop.
				p = NULL;
			}
		}
	}

	// Cleanup.
	mail_free_searchpgm (&pPgm);

	if (pList)
		FreeMemory ( (void **) &pList);

	return bResult ? S_OK : E_FAIL;
}





// FUNCTION
// DO an IMAP Search of a mailbox. Return the UID list of message matches (if any)
// as a comma-separaped uid list in "szMatches".
// "pHeaderList" is a comma-separated list of header field names, e.g. "To,From".
//
// END FUNCTION
//
HRESULT CMImap::UIDFindText (LPCSTR pSearchString, unsigned long UidFirst, unsigned long UidLast, CString& szResults)
{
	unsigned long flags = 0;
	BOOL		  bResult = FALSE;

	// Clear results string.
	szResults.Empty();

	// Must have a stream.
	if (!m_pProtocol)
		return E_FAIL;

	// Attempt to lock the stream:
	CStreamLock sLock (m_pProtocol);
	//
	// Did we get the stream??
	//
	if (! sLock.WeGotTheLock () )
		return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY);

	// pSearchString MUST be valid.
	if (! (pSearchString && *pSearchString) )
		return E_INVALIDARG;

	// UidFirst MUST be non-zero.
	if (UidFirst == 0)
		UidFirst = 1;

	// Fill a search program with the criteria.
	SEARCHPGM* pPgm = (SEARCHPGM *)fs_get (sizeof (SEARCHPGM) );
	if (!pPgm)
		return E_FAIL;

	// Clear.
	memset ( (void *)pPgm, 0, sizeof (SEARCHPGM) );

	// Set the Uid range .
	pPgm->uid = (SEARCHSET *) fs_get ( sizeof (SEARCHSET) );
	if (!pPgm->uid)
	{
		mail_free_searchpgm (&pPgm);
		return E_FAIL;
	}

	// Do this before each new command.
	ClearErrorDatabase ();

	pPgm->uid->first = UidFirst;
	pPgm->uid->last  = UidLast;
	pPgm->uid->next  = NULL;

	// Set the search text.
	
	pPgm->text = (STRINGLIST *) fs_get( sizeof (STRINGLIST) );
	if (pPgm->text)
	{
		pPgm->text->next = NULL;

		pPgm->text->text.data = cpystr (pSearchString);
		pPgm->text->text.size = strlen (pSearchString);

		bResult = TRUE;
	}

	// Do the search.
	if (bResult)
	{
		// Do UID SEARCH
		flags |= SE_UID;

		CString szThisResult;
		m_pProtocol->Search (NULL, pPgm, flags, szThisResult);

		// Copy the stream's results.
		szResults = szThisResult;

		bResult = TRUE;
	}


	// Cleanup.
	mail_free_searchpgm (&pPgm);

	return bResult ? S_OK : E_FAIL;
}



// AllowAuthenticate [PUBLIC]
//
void CMImap::AllowAuthenticate (BOOL Value)
{
	m_bAllowAuthenticate = Value;
}





// FormatSearchCriteria [PUBLIC]
//
BOOL CMImap::FormatSearchCriteria (SEARCHPGM *pPgm, LPCSTR pHeaderList, BOOL bBody, 
						BOOL bNot, LPCSTR pSearchString)
{
	BOOL bResult = FALSE;

	// Must be given a SEARCHPGM.
	if (!pPgm)
	{
		return FALSE;
	}

	// pSearchString MUST be valid.
	if (! (pSearchString && *pSearchString) )
	{
		return FALSE;
	}

	// Make a copy of the header list to pass to SetORSearchCriteria.
	LPSTR pHl = NULL;

	if (pHeaderList)
		pHl = cpystr (pHeaderList);

	// Go accumulate the search criteria.
	if (bNot)
	{
		//
		// Allocate a SEARCHPGMLIST off the ->not member, allocate a 
		// new SPGM in it, and fill that with the OR'd list.
		//
		SEARCHPGMLIST *pNotList;

		pNotList = (SEARCHPGMLIST *) fs_get ( sizeof (SEARCHPGMLIST) );

		if (!pNotList)
		{
			bResult = FALSE;
			goto cleanup;
		}

		// Attach it to tyhe given PGM.
		pPgm->not = pNotList;

		pNotList->next = NULL;

		// Allocate a new SPGM.
		pNotList->pgm = (SEARCHPGM *)fs_get (sizeof (SEARCHPGM) );
		if (! pNotList->pgm)
		{
			bResult = FALSE;
			goto cleanup;
		}

		// Clear.
		memset ( (void *)pNotList->pgm, 0, sizeof (SEARCHPGM) );

		// Set the OR criteria into this now.
		//
		bResult = SetORSearchCriteria (pNotList->pgm, pHl, bBody, pSearchString);
	}
	else
	{
		// Uses a series of OR's
		bResult = SetORSearchCriteria (pPgm, pHl, bBody, pSearchString);
	}


cleanup:

	if (pHl)
		fs_give ( (void **)&pHl);

	pHl = NULL;

	return bResult;
}







//=============================================================================//
//=============================================================================//
//================================= STATIC METHODS ============================//



// SetORSearchCriteria [STATIC]
//
// Accumulate the searech criteria into the given SEARCHPGM.
//
// Created by JOK, 9/16/97
//
BOOL CMImap::SetORSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList, BOOL bBody, LPCSTR pSearchString)
{
	BOOL bResult = FALSE;

	// Sanity; These must be valid.
	if (! (pPgm && pSearchString) )
		return FALSE;

	// Set this to TRUE if we succeed.
	bResult = FALSE;

	// Do we want to search the message body?
	if (bBody)
	{
		// If headers also, use an OR.
		if (pHeaderList && *pHeaderList)
		{
			pPgm->or = (SEARCHOR *) fs_get ( sizeof (SEARCHOR) );
			if (pPgm->or)
			{
				memset ( (void *) pPgm->or, 0, sizeof (SEARCHOR) );

				// Fill body criterion;
				pPgm->or->first = (SEARCHPGM *) fs_get ( sizeof (SEARCHPGM) );
				if (pPgm->or->first)
				{
					memset ( (void *) pPgm->or->first, 0, sizeof (SEARCHPGM) );

					// Fill the body criterion only!
					bResult = SetORSearchCriteria (pPgm->or->first, NULL, bBody, pSearchString);

					if (bResult)
					{
						// Restart.
						bResult = FALSE;

						// Go add the header list.
						pPgm->or->second = (SEARCHPGM *) fs_get ( sizeof (SEARCHPGM) );
						if (pPgm->or->second)
						{
							memset ( (void *) pPgm->or->second, 0, sizeof (SEARCHPGM) );

							// This will recursively add the headers.
							bResult = SetORHeaderSearchCriteria (pPgm->or->second, pHeaderList, pSearchString);
						}
					}
				}
			}
		}
		else
		{
			// No header list. We are searching just the body.

			bResult = FALSE;

			pPgm->body = (STRINGLIST *) fs_get( sizeof (STRINGLIST) );
			if (pPgm->body)
			{
				pPgm->body->next = NULL;

				pPgm->body->text.data = cpystr (pSearchString);
				pPgm->body->text.size = strlen (pSearchString);

				bResult = TRUE;
			}
		}
	}
	else if (pHeaderList && *pHeaderList)
	{
		// Add header list.

		bResult = SetORHeaderSearchCriteria (pPgm, pHeaderList, pSearchString);

	}

	return bResult;
}





// SetORHeaderSearchCriteria [STATIC]
//
// pHeaderList is a comma-separated list of headers that must be put into 
// a OR'd list of searchprograms. 
//
BOOL CMImap::SetORHeaderSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList, LPCSTR pSearchString)
{
	BOOL bResult = FALSE;

	// Sanity; These must be valid.
	if (! (pPgm && pHeaderList && pSearchString) )
		return FALSE;

	// Set this to TRUE if we succeed.
	bResult = FALSE;

	// If multiple headers, use an OR.

	// Is this the last or only header?
	LPSTR p = strchr (pHeaderList, Comma);
	if (p)
	{
		// Tie off temporarily.
		*p = '\0';

		// Use an OR.
		pPgm->or = (SEARCHOR *) fs_get ( sizeof (SEARCHOR) );
		if (pPgm->or)
		{
			memset ( (void *) pPgm->or, 0, sizeof (SEARCHOR) );

			// Add single header.
			pPgm->or->first = (SEARCHPGM *) fs_get ( sizeof (SEARCHPGM) );
			if (pPgm->or->first)
			{
				memset ( (void *) pPgm->or->first, 0, sizeof (SEARCHPGM) );

				bResult = SetORSearchCriteria (pPgm->or->first, pHeaderList, FALSE, pSearchString);
			}

			// Add rest of headers.
			if (bResult)
			{
				bResult = FALSE;

				// Put the comma back.
				*p++ = Comma;

				// Second criteria..
				pPgm->or->second = (SEARCHPGM *) fs_get ( sizeof (SEARCHPGM) );
				if (pPgm->or->second)
				{
					memset ( (void *) pPgm->or->second, 0, sizeof (SEARCHPGM) );

					// This will recursively add the headers.
					bResult = SetORSearchCriteria (pPgm->or->second, p, FALSE, pSearchString);
				}
			}
		}
	} // if p.
	else
	{
		bResult = FALSE;

		// Single header. Add it.
		pPgm->header = (SEARCHHEADER *) fs_get ( sizeof (SEARCHHEADER) );
		if (pPgm->header)
		{
			memset ( (void *) pPgm->header, 0, sizeof (SEARCHHEADER) );				

			pPgm->header->line = cpystr (pHeaderList);
			pPgm->header->text = cpystr (pSearchString);

			pPgm->header->next = NULL;

			bResult = TRUE;
		}
	}

	return bResult;
}




// SetNewPgmUidList [STATIC]
//
// Free the current uid list in the SPGM and create a new one based on the given
// comma-separated string.
BOOL CMImap::SetNewPgmUidList (SEARCHPGM *pPgm, LPCSTR pStr)
{
	SEARCHSET *pSet = NULL;
	char *pList = NULL;

	// Sanity:
	if (! pPgm )
		return FALSE;

	// Free current uid list.
	FreePgmUidList (pPgm);

	// Must have a pStr.
	if (! (pStr && *pStr) )
		return FALSE;

	// Local copy:
	pList = cpystr (pStr);

	char *p = pList;
	char *q;

	while (p)
	{
		// send this in smaller chunks!!
		//
		q = strchr (p, Comma);

		if (q)
		{
			*q = 0;
		}

		// Add this uid.

		if (p && *p)
		{
			unsigned long Uid = atol (p);

			if (Uid != 0)
			{
				if (pSet == NULL)
				{
					pSet = (SEARCHSET *) fs_get ( sizeof (SEARCHSET) );
					if (pSet)
					{
						pSet->first = Uid;
						pSet->last = 0;
						pSet->next = NULL;

						pPgm->uid = pSet;
					}
				}
				else
				{
					pSet->next = (SEARCHSET *) fs_get ( sizeof (SEARCHSET) );
					if (pSet->next)
					{
						pSet = pSet->next;

						pSet->first = Uid;
						pSet->last = 0;
						pSet->next = NULL;
					}
				}
			}
		}

		if (q)
		{
			// Put back;
			*q = Comma;

			p = q + 1;
		}
		else
		{
			// Terminate.
			p = NULL;
		}
	}
			
	// Cleanup.

	if (pList)
		fs_give ( (void **)&pList);

	pList = NULL;

	return TRUE;

}



// FreeSearchpgm [STATIC]
//
void CMImap::FreeSearchpgm (SEARCHPGM** pPgm)
{
	if (pPgm && *pPgm)
		mail_free_searchpgm (pPgm);

}




// NewSearchPgm [STATIC]
//
SEARCHPGM* CMImap::NewSearchPgm()
{
	// Allocate a new one.
	//.
	SEARCHPGM* pPgm = (SEARCHPGM *)fs_get (sizeof (SEARCHPGM) );
	if (pPgm)
	{
		// Clear.
		memset ( (void *)pPgm, 0, sizeof (SEARCHPGM) );
	}

	return pPgm;
}



// FreePgmUidList [STATIC]
// 
void CMImap::FreePgmUidList (SEARCHPGM *pPgm)
{	
	SEARCHSET *pSet = NULL, *pNext = NULL;

	if (!pPgm)
		return;

	// Free current uid list.
	pSet = pPgm->uid;

	while (pSet)
	{
		pNext = pSet->next;

		fs_give ( (void **)&pSet);

		pSet = pNext;
	}

	// Make sure.
	pPgm->uid = NULL;
}





//////////////////////////////////////////////////////////////////////////////
// =======================  Utility functions ==============================//


// FUNCTION
// Convert a comma-separated string of strings into a STRINGLIST.
// END FUNCTION
// NOTES
// Return ALLOCATED memory that must be freed by caller.
// END NOTES
STRINGLIST *CommaSeparatedTextToStringlist (char *Fields)
{
	size_t		len;
	char		buf [MAILTMPLEN];
	char		*p, *q;
	STRINGLIST	*first = NULL, *last, *m;

	// Sanity. sequence can be NULL.
	if (!Fields)
		return NULL;

	// Format the comma-separated list of fields into a STRINGLIST.
	p = Fields;
	q = NULL;
	*buf = 0;

	// Wade through Fields.
	while (p && *p)
	{
		q = strchr (p, ',');

		// Get token.
		if (q)
		{
			len = q - p;
			if (len >= MAILTMPLEN)		// Can't handle long strings.
				*buf = 0;
			else
			{
				strncpy ( buf, p, len );
				buf[len] = 0;
				Trim (buf);
			}					
			p = q + 1;
		}
		else
		{
			len = strlen (buf);
			// Must be last or only one.
			if (strlen(p) >= MAILTMPLEN)
				*buf = 0;
			else
			{
				strcpy (buf, p);
				Trim (buf);
			}
			p = NULL;   // So we stop.
		}

		// Add to stringlist if not blank.
		if (*buf)
		{
			// Get new stringlist.
			m = mail_newstringlist ();
			if (m)
			{
				m->text.data = cpystr (buf);
				m->text.size = strlen (buf);
				m->next = NULL;

				// Link in:
				if (!first)
					first = m;
				else
				{
					last = first;
					while (last->next)
						last = last->next;

					last->next = m;
				}
			}
		}
	}

	return first;
}



#endif // IMAP4
