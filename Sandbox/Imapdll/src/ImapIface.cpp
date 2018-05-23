// imap.cpp - Interface routines to imap.dll


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


// Whenever imapmail.h and imap.h are included together, define _IMAPDLL_INTERNAL
// #define _IMAPDLL_INTERNAL_
// #include "Network.h"
// #include "imapmail.h"
#include "imap.h"
#include "ImapDefs.h"
#include "imapfr.h"			// Friend classes.
#include "mm.h"


// Global data structures
BOOL (__cdecl *g_pTimeoutQuery)	() = NULL;

// Parent's preview message. Sometimes we flush it so a preview won't occur.
UINT g_umsgPreviewMessage = 0;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// ==================================================================//
// Routines for setting callback functions.
void SetDebugLog (void (*fn) (char *, long))
{
	if (fn)
		mm_set_debug_log(fn);
}



void SetTimeoutQuery (BOOL (*fn) () )
{
	if (fn)
		g_pTimeoutQuery = fn;
}


// =============== Routine for setting the value of the preview event. ==========/
void SetPreviewMessage (UINT umsgPreviewMessage)
{

	g_umsgPreviewMessage = umsgPreviewMessage;
}

	

// ================================================================= //

//=== Implementation of the CImapStream class ===/
//=== The methods just call their appropriate friend class methods.==/

// FUNCTION:
// * Establishes the initial connection to the IMAP server.
// * Issues the LOGIN command.
// END FUNCTION

// NOTES:
// * Returns FALSE if attempt fails.
// END NOTES

CImapStream::CImapStream(const char *ServerName, UINT ServiceID, const char *PortNum)
{
	// Instantiate internal CCImap class.
	m_cmImap = new CMImap (ServerName, ServiceID, PortNum);
}


// FUNCTION:
// * Cleans up the connection to the IMAP server.
// * Issues the LOGOUT command.
// END FUNCTION

// NOTES:
// Added logout command Aug 2, 1999 -jdboyd
// END NOTES
CImapStream::~CImapStream()
{
	m_cmImap->Logout();	// send the logout command if appropriate

	if (m_cmImap)
	{
		delete m_cmImap;
		m_cmImap = NULL;
	}
}




// Methods for initiating the connection to the IMAP server.

// FUNCTION:
// * Establishes the initial connection to the IMAP server.
// * Issues the LOGIN command.
// END FUNCTION

// NOTES:
// * Returns FALSE if attempt fails.
// END NOTES

HRESULT  CImapStream::OpenControlStream()
{
	if (m_cmImap)
		return m_cmImap->OpenControlStream();
	else
		return E_FAIL;
}



// FUNCTION
// Free memory allocated by this DLL.	
// END FUNCTION
void CImapStream::FreeMemory(void **pMem)
{
	if (m_cmImap)
		m_cmImap->FreeMemory (pMem);
}




// FUNCTION
// Destroy an envelope, include free-ing all internal data..	
// END FUNCTION
void CImapStream::DestroyEnvelope(ENVELOPE **ppEnvelope)
{
	if (m_cmImap)
		m_cmImap->DestroyEnvelope (ppEnvelope);
}



// FUNCTION
// * Performs an IMAP "SELECT" on the mailbox. If the connection to the mailbox has not
// * already been made, do that as well.
// END FUNCTION

// NOTES
// * Returns TRUE if success.
// END NOTES

HRESULT CImapStream::OpenMailbox  ()
{
	if (m_cmImap)
		return m_cmImap->OpenMailbox();
	else
		return E_FAIL;
}



// FUNCTION
// * Performs an IMAP "SELECT" on the mailbox. If the connection to the mailbox has not
// * already been made, do that as well.
// END FUNCTION

// NOTES
// * Returns TRUE if success.
// END NOTES

HRESULT CImapStream::OpenMailbox  (const char *MailboxName)
{
	if (m_cmImap)
		return m_cmImap->OpenMailbox(MailboxName);
	else
		return E_FAIL;
}


// RequestStop [PUBLIC]
//
void CImapStream::RequestStop()
{
	if (m_cmImap)
		m_cmImap->RequestStop();
}



// Fetches.

// Top level message only.

// FUNCTION: UIDFetchEnvelope.
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::UIDFetchEnvelope (unsigned long uid, ENVELOPE **ppEnvelope)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchEnvelope (uid, ppEnvelope);
	else
		return E_FAIL;
}




HRESULT	CImapStream::UIDFetchEnvelopeAndExtraHeaders (
						IMAPUID		Uid, 
						ENVELOPE**	ppEnvelope,
						char*		pFields,
						CWriter*	pWriter)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchEnvelopeAndExtraHeaders (
			Uid, ppEnvelope, pFields, pWriter);
	else
		return E_FAIL;
}



// FUNCTION:  UIDFetchStructure
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::UIDFetchStructure	(unsigned long uid, BODY **ppBody)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchStructure (uid, ppBody);
	else
		return E_FAIL;
}




// FUNCTION:  FreeBodyStructure
// END FUNCTION

// NOTES
// END NOTES
void CImapStream::FreeBodyStructure	(BODY *pBody)
{
	if (m_cmImap)
		m_cmImap->FreeBodyStructure (pBody);
}



// FUNCTION:  UIDFetchFlags
// END FUNCTION

// NOTES
// Send UID FETCH 1:* FLAGS
// END NOTES
HRESULT CImapStream::UIDFetchFlags	(const char *pSequence, CEltWriter *pEltWriter)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchFlags (pSequence, pEltWriter);
	else
		return E_FAIL;
}




// FUNCTION: UIDFetchFast
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::UIDFetchFast (unsigned long uid, IMAPFULL **ppFlags)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchFast (uid, ppFlags);
	else
		return E_FAIL;
}


// FUNCTION: UIDFetchAll
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::UIDFetchAll (unsigned long uid, IMAPFULL **ppFlags)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchAll (uid, ppFlags);
	else
		return E_FAIL;
}



// FUNCTION: UIDFetchFull
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::UIDFetchFull	(unsigned long uid, IMAPFULL **ppFlags)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchFull (uid, ppFlags);
	else
		return E_FAIL;
}


// FUNCTION: UIDFetchInternalDate
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::UIDFetchInternalDate(unsigned long uid, char **ppDate)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchInternalDate (uid, ppDate);
	else
		return E_FAIL;
}


// FUNCTION: UIDFetchHeader
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchHeader (unsigned long uid, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchHeader (uid, Writer);
	else
		return E_FAIL;
}



// FUNCTION: UIDFetchMessage
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchMessage (unsigned long uid, CWriter *Writer, BOOL Peek)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchMessage (uid, Writer, Peek);
	else
		return E_FAIL;
}



// FUNCTION: UIDFetchMessageBody
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchMessageBody (unsigned long uid, CWriter *Writer, BOOL Peek)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchMessageBody (uid, Writer, Peek);
	else
		return E_FAIL;
}




// FUNCTION: UIDFetchPartialMessage
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchPartialMessage  (unsigned long uid, unsigned long first, unsigned long last,
			 CWriter *Writer, BOOL Peek)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchPartialMessage (uid, first, last, Writer, Peek);
	else
		return E_FAIL;
}




// FUNCTION: UIDFetchPartialMessageBody
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchPartialMessageBody  (unsigned long uid, unsigned long first, unsigned long last,
			 CWriter *Writer, BOOL Peek)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchPartialMessageBody (uid, first, last, Writer, Peek);
	else
		return E_FAIL;
}




// FUNCTION: UIDFetchPartialMessageBody
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchPartialBodyText  (unsigned long uid, char *sequence, unsigned long first,
						unsigned long last,  CWriter *Writer, BOOL Peek)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchPartialBodyText (uid, sequence, first, last, Writer, Peek);
	else
		return E_FAIL;
}





// FUNCTION: FetchUID
// END FUNCTION

// NOTES
// END NOTES
HRESULT	CImapStream::FetchUID (unsigned long msgNum, IMAPUID *pUid)
{
	if (m_cmImap)
		return m_cmImap->FetchUID (msgNum, pUid);
	else
		return E_FAIL;
}



// These apply to a "message/rfc822"part. (Can also be used for top-level message.)

// FUNCTION: UIDFetchRFC822Header
// END FUNCTION

// NOTES
// END NOTES
HRESULT  CImapStream::UIDFetchRFC822Header (unsigned long uid, char *sequence, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchRFC822Header (uid, sequence, Writer);
	else
		return E_FAIL;
}


// FUNCTION: UIDFetchRFC822Text
// END FUNCTION

// NOTES
// END NOTES
HRESULT CImapStream::UIDFetchRFC822Text (unsigned long uid, char *sequence, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchRFC822Text(uid, sequence, Writer);
	else
		return E_FAIL;
}


// FUNCTION: UIDFetchRFC822HeaderFields
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDFetchRFC822HeaderFields (unsigned long uid, char *sequence, char *Fields, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchRFC822HeaderFields(uid, sequence, Fields, Writer);
	else
		return E_FAIL;
}


// FUNCTION: UIDFetchRFC822HeaderFieldsNot
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDFetchRFC822HeaderFieldsNot (unsigned long uid, char *sequence, char *fields, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchRFC822HeaderFieldsNot(uid, sequence, fields, Writer);
	else
		return E_FAIL;
}




// BODY part level fetches.


// FUNCTION: UIDFetchMimeHeader
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDFetchMimeHeader (unsigned long uid, char *sequence, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchMimeHeader(uid, sequence, Writer);
	else
		return E_FAIL;
}



// FUNCTION: UIDFetchBodyText
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDFetchBodyText (unsigned long uid, char *sequence, CWriter *Writer, BOOL Peek)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchBodyText(uid, sequence, Writer, Peek);
	else
		return E_FAIL;
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
HRESULT CImapStream::UIDFetchPreamble(unsigned long uid, char *sequence, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchPreamble(uid, sequence, Writer);
	else
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
HRESULT CImapStream::UIDFetchTrailer(unsigned long uid, char *sequence, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->UIDFetchTrailer(uid, sequence, Writer);
	else
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
HRESULT CImapStream::UIDSaveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent)
{
	if (m_cmImap)
		return m_cmImap->UIDSaveFlags (uid, Flags, Silent);
	else
		return E_FAIL;
}


// FUNCTION: UIDAddFlags
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDAddFlags  (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent)
{
	if (m_cmImap)
		return m_cmImap->UIDAddFlags (uid, Flags, Silent);
	else
		return E_FAIL;
}


// FUNCTION: UIDRemoveFlags
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDRemoveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent)
{
	if (m_cmImap)
		return m_cmImap->UIDRemoveFlags (uid, Flags, Silent);
	else
		return E_FAIL;
}



// COPY/MOVE methods. Destination mailbox MUST be on the same server.
// BUG: Should be extended to handle message sets.


// FUNCTION: UIDCopy
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDCopy (const char *pUidlist, const char *pDestMailbox)
{
	if (m_cmImap)
		return m_cmImap->UIDCopy (pUidlist, pDestMailbox);
	else
		return E_FAIL;
}



// FUNCTION: Append
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT	CImapStream::AppendMessage (const char* pFlags, unsigned long Seconds, STRING *pMsg)
{
	if (m_cmImap)
		return m_cmImap->AppendMessage (pFlags, Seconds, pMsg);
	else
		return E_FAIL;
}



// Utilities.

// FUNCTION: UIDMessageIsMultipart
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT	CImapStream::UIDMessageIsMultipart (unsigned long uid, BOOL *pVal)

{
	if (m_cmImap)
		return m_cmImap->UIDMessageIsMultipart (uid, pVal);
	else
		return E_FAIL;
}



// FUNCTION: UIDGetTime
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT	CImapStream::UIDGetTime (IMAPUID uid, unsigned long *pSeconds)
{

	if (m_cmImap)
		return m_cmImap->UIDGetInternalDate (uid, pSeconds);
	else
		return 0L;
}



// FUNCTION: GetRfc822Size
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT	CImapStream::GetRfc822Size (IMAPUID uid,unsigned long *pSize)
{
	if (m_cmImap)
		return m_cmImap->GetRfc822Size (uid, pSize);
	else
		return E_FAIL;
}




//========== non-UID methods =================/
// FUNCTION: FetchHeader
// END FUNCTION
// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::FetchHeader (unsigned long msgNum, CWriter *Writer)
{
	if (m_cmImap)
		return m_cmImap->FetchHeader (msgNum, Writer);
	else
		return E_FAIL;
}

	

//===== Mailbox level methods =========/

// FUNCTION: Check
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::Check()
{
	if (m_cmImap)
		return m_cmImap->Check();

	return E_FAIL;
}


// FUNCTION: Noop
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::Noop ()
{
	if (m_cmImap)
		return m_cmImap->Noop();
	else
		return E_FAIL;
}


// FUNCTION: UIDDeleteMessages
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDDeleteMessages (LPCSTR pList, BOOL Expunge)
{

	if (m_cmImap)
		return m_cmImap->UIDDeleteMessages(pList, Expunge);
	else
		return E_FAIL;
}


// FUNCTION: UIDUnDeleteMessages
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::UIDUnDeleteMessages (LPCSTR pList)
{

	if (m_cmImap)
		return m_cmImap->UIDUnDeleteMessages(pList);
	else
		return E_FAIL;
}



// FUNCTION: Expunge
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT	CImapStream::UIDExpunge (LPCSTR pUidList, CString& szUidsActuallyRemoved)
{
	if (m_cmImap)
		return m_cmImap->UIDExpunge(pUidList, szUidsActuallyRemoved);
	else
		return E_FAIL;
}


// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::Expunge ()
{

	if (m_cmImap)
		return m_cmImap->Expunge();
	else
		return E_FAIL;
}



// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::RecreateMessageMap ()
{
	if (m_cmImap)
		return m_cmImap->RecreateMessageMap();
	else
		return E_FAIL;
}





// FUNCTION: Logout
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
HRESULT CImapStream::Logout ()
{

	if (m_cmImap)
		return m_cmImap->Logout();
	else
		return E_FAIL;
}


// FUNCTION: IsSelected
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
BOOL CImapStream::IsSelected ()
{

	if (m_cmImap)
		return m_cmImap->IsSelected();
	else
		return FALSE;
}



// FUNCTION: UIDValidity
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
UIDVALIDITY CImapStream::UIDValidity ()
{

	if (m_cmImap)
		return m_cmImap->UIDValidity();
	else
		return FALSE;
}



// FUNCTION: GetMessageCount
// END FUNCTION

// NOTES
// END NOTES

// HISTORY
// * Written by J.King, 3/20/97
// END HISTORY
unsigned long CImapStream::GetMessageCount ()
{

	if (m_cmImap)
		return m_cmImap->GetMessageCount();
	else
		return 0L;
}



//
// Get the top-level mailbox delimiter.
//
HRESULT	CImapStream::GetTopMboxDelimiter (TCHAR *pDelimiter)
{
	if (m_cmImap)
		return m_cmImap->GetTopMboxDelimiter(pDelimiter);
	else
		return E_FAIL;
}




// Fetch our list of un-subscribed IMAP mailboxes from "server". Return the
// list in "mboxlist".
// Note: "server" can be NULL, in which case user the m_server attribute.
// If "INCLUDE"INBOX" is TRUE, Fetch inbox as a separate command.
HRESULT CImapStream::IMAPListUnSubscribed (LPCSTR pReference, CLister *Lister, BOOL INCLUDE_INBOX)
{

	if (m_cmImap)
		return m_cmImap->IMAPListUnSubscribed (pReference, Lister, INCLUDE_INBOX);
	else
		return E_FAIL;
}



// Create mailbox
HRESULT CImapStream::CreateMailbox (LPCSTR pImapName)
{

	if (m_cmImap)
		return m_cmImap->CreateMailbox (pImapName);
	else
		return E_FAIL;
}


// Delete mailbox
HRESULT CImapStream::DeleteMailbox (LPCSTR pImapName)
{

	if (m_cmImap)
		return m_cmImap->DeleteMailbox (pImapName);
	else
		return E_FAIL;
}



// Rename mailbox
HRESULT CImapStream::RenameMailbox (LPCSTR pImapName, LPCSTR pNewImapName)
{
	if (m_cmImap)
		return m_cmImap->RenameMailbox (pImapName, pNewImapName);
	else
		return E_FAIL;
}



// Subscribe a mailbox..
HRESULT CImapStream::SubscribeMailbox (LPCSTR pImapName)
{
	if (m_cmImap)
		return m_cmImap->SubscribeMailbox (pImapName);
	else
		return E_FAIL;
}


// UnSubscribeMailbox a mailbox..
HRESULT CImapStream::UnSubscribeMailbox (LPCSTR pImapName)
{
	if (m_cmImap)
		return m_cmImap->UnSubscribeMailbox (pImapName);
	else
		return E_FAIL;
}




// DO a LIST on the mailbox to get it's attributes.
HRESULT CImapStream::FetchMailboxAttributes (LPCSTR pImapName, CLister *Lister)
{

	if (m_cmImap)
		return m_cmImap->FetchMailboxAttributes (pImapName, Lister);
	else
		return E_FAIL;
}




void CImapStream::SetNetSettingsObject (QCNetworkSettings* pNetSettings)
{
	if (m_cmImap)
		m_cmImap->SetNetSettingsObject (pNetSettings);
}



BOOL CImapStream::IsConnected ()
{

	if (m_cmImap)
		return m_cmImap->IsConnected ();
	else
		return FALSE;
}	


BOOL CImapStream::IsAuthenticated ()
{

	if (m_cmImap)
		return m_cmImap->IsAuthenticated ();
	else
		return FALSE;
}	





// Register a user callback for IMAP notifications, user cancel, etc.
//
void CImapStream::RegisterUserCallback (ImapNotificationCallback_p pHandler, unsigned long ulData)
{

	if (m_cmImap)
		m_cmImap->RegisterUserCallback (pHandler, ulData);
}	



void CImapStream::SetLogin (LPCSTR pLogin)
{
	if (m_cmImap)
		m_cmImap->SetLogin (pLogin);
}



void CImapStream::SetPassword (LPCSTR pPassword)
{
	if (m_cmImap)
		m_cmImap->SetPassword (pPassword);
}


void CImapStream::SetKrbDllName (LPCSTR pKrbLibName)
{
	if (m_cmImap)
		m_cmImap->SetKrbDllName (pKrbLibName);
}


void CImapStream::SetDesDllName (LPCSTR pDesLibName)
{
	if (m_cmImap)
		m_cmImap->SetDesDllName (pDesLibName);
}


void CImapStream::SetGssDllName (LPCSTR pGssLibName)
{
	if (m_cmImap)
		m_cmImap->SetGssDllName (pGssLibName);
}


// FUNCTION
// Return the last error message. Not necessarily from the server.
// END FUNCTION. 
HRESULT	CImapStream::GetLastErrorStringAndType (LPSTR szErrorBuf, short nBufSize, int *pType)
{
	if (m_cmImap)
		return m_cmImap->GetLastErrorStringAndType (szErrorBuf, nBufSize, pType);

	return E_FAIL;
}	

// FUNCTION
// Return the last tagged message from the server.
// END FUNTION
HRESULT CImapStream::GetLastServerMessage (LPSTR szErrorBuf, short nBufSize)
{

	if (m_cmImap)
		return m_cmImap->GetLastServerMessage (szErrorBuf, nBufSize);

	return E_FAIL;
}	


void CImapStream::SetErrorCallback (ImapErrorFunc pImapErrorFn)
{
	if (m_cmImap)
		m_cmImap->SetErrorCallback (pImapErrorFn);
}


void CImapStream::ResetErrorCallback ()
{
	if (m_cmImap)
		m_cmImap->ResetErrorCallback ();
}




// FUNCTION
// Return the rdonly value in the stream..
// END FUNTION
BOOL CImapStream::IsReadOnly ()
{

	if (m_cmImap)
		return m_cmImap->IsReadOnly ();

	return FALSE;
}	


// 
// If the stream supports the STATUS command
//
BOOL CImapStream::HasStatusCommand ()
{

	if (m_cmImap)
		return m_cmImap->HasStatusCommand ();

	return FALSE;
}	



// 
// If the stream supports IMAP4rev1.
//
BOOL CImapStream::IsImap4Rev1 ()
{

	if (m_cmImap)
		return m_cmImap->IsImap4Rev1 ();

	return FALSE;
}	


// 
// If the stream supports the rfc1730 only.
//
BOOL CImapStream::IsImap4Only ()
{

	if (m_cmImap)
		return m_cmImap->IsImap4Only ();

	return FALSE;
}	


// 
// If the stream only supports imap2bis and lower.
//
BOOL CImapStream::IsLowerThanImap4 ()
{

	if (m_cmImap)
		return m_cmImap->IsLowerThanImap4 ();

	return FALSE;
}	



// 
// If the stream supports the Optimize-1.
//
BOOL CImapStream::HasOptimize1 ()
{

	if (m_cmImap)
		return m_cmImap->HasOptimize1 ();

	return FALSE;
}	




// 
// If the stream supports the SORT extension.
//
BOOL CImapStream::HasSort ()
{

	if (m_cmImap)
		return m_cmImap->HasSort ();

	return FALSE;
}	


// 
// If the stream supports the SCAN extension
//
BOOL CImapStream::HasScan ()
{

	if (m_cmImap)
		return m_cmImap->HasScan ();

	return FALSE;
}	


// 
// If the stream supports the ACL extension.
//
BOOL CImapStream::HasAcl ()
{

	if (m_cmImap)
		return m_cmImap->HasAcl ();

	return FALSE;
}	



// 
// If the stream supports the QUOTA
//
BOOL CImapStream::HasQuota ()
{

	if (m_cmImap)
		return m_cmImap->HasQuota ();

	return FALSE;
}	



// 
// If the stream supports the LITERAL+
//
BOOL CImapStream::HasLiteralPlus ()
{

	if (m_cmImap)
		return m_cmImap->HasLiteralPlus ();

	return FALSE;
}	



// 
// If the stream supports the X-NON-HIERARCHICAL-RENAME extension.
//
BOOL CImapStream::HasXNonHierarchicalRename ()
{

	if (m_cmImap)
		return m_cmImap->HasOptimize1 ();

	return FALSE;
}	




// 
// Do a STATUS command.
//
HRESULT	CImapStream::UIDFetchStatus (const char *pMailbox, long flags, MAILSTATUS *pStatus)

{
	if (m_cmImap)
		return m_cmImap->UIDFetchStatus (pMailbox, flags, pStatus);

	return E_FAIL;
}	




// Search [PUBLIC]
//
// Generic search command.
//
void CImapStream::Search (char *charset, SEARCHPGM *pgm, CString& szResults)
{
	if (m_cmImap)
		m_cmImap->Search (charset, pgm, szResults);
}




// FUNCTION
// Search for the given string in the message's body or a head.
// "pHeaderList" is a comma-separated list of header field names, e.g. "To,From".
// END FUNTION
HRESULT CImapStream::UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr,  CString& szResults)
{
	if (m_cmImap)
		return m_cmImap->UIDFind (pHeaderList, bBody, bNot, pSearchString, pUidStr, szResults);

	return E_FAIL;
}	


// FUNCTION
// Search for the given string in the entire message's text.
// "pHeaderList" is a comma-separated list of header field names, e.g. "To,From".
// The results are returned in szResults as a comma-separated array of UID's.
// END FUNTION
HRESULT CImapStream::UIDFindText (LPCSTR pSearchString, unsigned long UidFirst,
						 unsigned long UidLast,  CString& szResults)
{
	if (m_cmImap)
		return m_cmImap->UIDFindText (pSearchString, UidFirst, UidLast, szResults);

	return E_FAIL;
}	



void CImapStream::AllowAuthenticate (BOOL Value)
{
	if (m_cmImap)
		m_cmImap->AllowAuthenticate (Value);
}



HRESULT CImapStream::UIDRemoveFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */)
{
	if (m_cmImap)
		return m_cmImap->UIDRemoveFlaglist (pUidList, pFlagList, Silent);

	return E_FAIL;
}



HRESULT CImapStream::UIDAddFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */)
{
	if (m_cmImap)
		return m_cmImap->UIDAddFlaglist (pUidList, pFlagList, Silent);

	return E_FAIL;
}




// Static class functions!!
//

// Initialize() - MUST call this function once before using 
// this DLL.
//
HRESULT CImapStream::Initialize ()
{

#if 0 // Don't need to do this anymore!!

	// Initialize winsock:
	return CNetSock::StartWinSock();

#endif

	return S_OK;

}


// Must call this function once when the client application
// is shutting down.
//
HRESULT CImapStream::Cleanup ()
{

#if 0 // Don't need to do this anymore!!

	// Shut down our instance of winsock.
	return CNetSock::ShutdownNetwork ();

#endif

	return S_OK;
}




// SEARCHPROGRAM handling.

// FormatSearchCriteria [PUBLIC]
//
BOOL CImapStream::FormatSearchCriteria (SEARCHPGM *pPgm, LPCSTR pHeaderList, BOOL bBody, 
						BOOL bNot, LPCSTR pSearchString)
{
	if (m_cmImap)
		return m_cmImap->FormatSearchCriteria (pPgm, pHeaderList, bBody, bNot, pSearchString);

	return FALSE;
}


// STATIC
BOOL CImapStream::SetORSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList, BOOL bBody,
						LPCSTR pSearchString)
{
	return CMImap::SetORSearchCriteria (pPgm, pHeaderList, bBody, pSearchString);
}


// STATIC
BOOL CImapStream::SetORHeaderSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList,
						LPCSTR pSearchString)
{
	return CMImap::SetORHeaderSearchCriteria (pPgm, pHeaderList, pSearchString);
}

// STATIC
void CImapStream::FreePgmUidList (SEARCHPGM *pPgm)
{
	CMImap::FreePgmUidList (pPgm);
}


// STATIC
BOOL CImapStream::SetNewPgmUidList (SEARCHPGM *pPgm, LPCSTR pStr)
{
	return CMImap::SetNewPgmUidList (pPgm, pStr);
}


SEARCHPGM* CImapStream::NewSearchPgm()
{
	return CMImap::NewSearchPgm();
}


// STATIC
void CImapStream::FreeSearchpgm (SEARCHPGM** pPgm)
{
	CMImap::FreeSearchpgm (pPgm);
}





#endif
