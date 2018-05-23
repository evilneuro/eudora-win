/////////////////////////////////////////////////////////////////////////////
// 	File: IMAPUTIL.H 
//
//  Purpose:  This file provides the declarations for the IMAP
//				routines.   
//
//	Author: 
//	Created:   9/27/96
//  Modified:
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _IMAPUTIL_H_
#define _IMAPUTIL_H_

#include "QCRasConnection.h"		//FORNOW

// Global constants.
const	long			MsgByteLimit 			= 54*1024L;	// 54K for each message -- MAX
const	long			MsgByteLimit32			= 1024*1024L; // Let's limit message to 1 Meg
const	unsigned long	_MAGICNUMBER			= 98754908;	// Arbitrary.

#include "imap.h"

// IMAP types
typedef unsigned long	IMapMsgNum;


enum NetworkNotifierType
{
	NETNOTIFY_NONE,
	NETNOTIFY_SET_READONLY_STATUS,
	NETNOTIFY_SET_READWRITE_STATUS,
	NETNOTIFY_STOP_REQUESTED,
	NETNOTIFY_INVALID_PASSWORD,
};


typedef void (*NetworkCallback_p) (void *data, NetworkNotifierType NotifierType, unsigned long ulValue);


// Forward definitions.
class	CTocDoc;
class	CSummary;	 
class 	HeaderDesc;
class	MultSearchCriteria;
 
// Constants


// Function Prototypes

int 	IMAPByeBye(BOOL abort);
void 	GetImapMail(void);

// Forward declarations.
class CImapTocDoc;
class CImapAccount;
class QCNetworkSettings;
class CNetworkNotifier;
class CUidMap;
class CSyncObject;
class CSingleLock;
class CCriticalSection;


//	===============================================================================================                                                         

// Background tasks should pass a callback to each method in CIMAP.
//
#include "TaskErrorTypes.h"
#include "QCNetSettings.h"
#include "imap.h"

// typedef QCNetworkSettings::ErrorCallback ImapErrorFunc;

// Forward declarations!!
typedef struct imap_mailbox_node ImapMailboxNode;


class CIMAP: public CObject
{
protected:
	// Protected Application methods	
	   

// Implementation
public:
	CIMAP();
	CIMAP(const char *server);

	~CIMAP();


	// Initiating connections. 
	// NOTE: Do NOT provide a method to close the stream. This must only
	// be handles when the object goes out of scope so that the referencing
	// mechanism would work.
	HRESULT		OpenMailbox (const char *Fullname, BOOL bSilent = FALSE, ImapErrorFunc pErrFn = NULL);

	HRESULT		OpenControlStream(CImapAccount *pAccount = NULL, BOOL bSilent = FALSE, ImapErrorFunc pErrFn = NULL);

	// Close connection.
	HRESULT		Close();

	void		RequestStop();

// Ping and check.
	HRESULT			Noop (ImapErrorFunc pErrFn = NULL);
	HRESULT			Check (ImapErrorFunc pErrFn = NULL);


	// Create mailbox
	HRESULT			CreateMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn = NULL );

	// Delete mailbox.
	HRESULT			DeleteMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn = NULL );

	// Subscribe/Unsubscribe:
	HRESULT			SubscribeMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn = NULL);
	HRESULT			UnSubscribeMailbox ( LPCSTR pImapName, ImapErrorFunc pErrFn = NULL);

	// Rename mailbox.
	HRESULT			RenameMailbox ( LPCSTR pImapName, LPCSTR pNewImapName, ImapErrorFunc pErrFn = NULL );

	// Fetch attributes of a mailbox by doing a LIST.
	ImapMailboxNode *FetchMailboxAttributes (LPCSTR pImapName, ImapErrorFunc pErrFn = NULL);

	// Message structure.
	HRESULT			 FetchStructure(CImapTocDoc* toc, unsigned long uid, ImapErrorFunc pErrFn = NULL);

	// Free the BODY returned by UIDFetchStructure.
	void			 FreeBodyStructure	(BODY *pBody, ImapErrorFunc pErrFn = NULL);

	// Expunge mailbox.
	HRESULT			UIDExpunge (LPCSTR pUidList, CUidMap& mapUidsActuallyRemoved, ImapErrorFunc pErrFn = NULL);

	// Simple expunge.
	HRESULT			Expunge (ImapErrorFunc pErrFn = NULL);

	// Append message
	HRESULT			AppendMessage ( IMAPFLAGS* pImapFlags, unsigned long Seconds, STRING *pMsg, ImapErrorFunc pErrFn = NULL );
	// Copy
	HRESULT			CopyMessages (const char *pUidlist, const char *pDestMailbox, ImapErrorFunc pErrFn = NULL);

	HRESULT			MessageIsMultipart (IMAPUID uid, BOOL *pVal, ImapErrorFunc pErrFn = NULL);

// Fetch contents.
	HRESULT			FetchAttachmentContents(unsigned long uid, const char *Filename, ImapErrorFunc pErrFn = NULL);

	HRESULT			UIDFetchPartialContentsToBuffer(unsigned long uid, char *sequence, int first, 
						unsigned long nBytes, LPTSTR pBuffer, unsigned long nBufferSize, unsigned long *pLen, ImapErrorFunc pErrFn = NULL);

	HRESULT			FetchAttachmentContentsToFile (IMAPUID uid, char *sequence, const char *Filename, short encoding, LPCSTR pSubtype = NULL, ImapErrorFunc pErrFn = NULL);

	HRESULT			FetchMIMEHeader (IMAPUID uid, char *sequence, CWriter *pWriter, ImapErrorFunc pErrFn = NULL);

	HRESULT			FetchHeaderValue (const char *name, char *pValue, ImapErrorFunc pErrFn = NULL);

	void	 		MailFetchSubscribed (CImapAccount *pAccount, ImapErrorFunc pErrFn = NULL);

	void			MailFetchThisLevel (LPCSTR pReference, CImapAccount *pAccount, BOOL INCLUDE_INBOX, ImapErrorFunc pErrFn = NULL);

	void			CreatePatSome (char *Reference, CString &pat);
	void			CreatePatAll (char *Reference, CString &pat);

	HRESULT			FindBody (IMAPUID uid, char *sequence, BODY **ppBody, ImapErrorFunc pErrFn = NULL);

	// Utility functions:
	HRESULT			GetTime (IMAPUID uid, unsigned long *pSeconds);
	HRESULT			GetRfc822Size (IMAPUID uid, unsigned long *pSize, ImapErrorFunc pErrFn = NULL);
	HRESULT			UIDFetchEnvelope (unsigned long uid, ENVELOPE **ppEnvelope, ImapErrorFunc pErrFn = NULL);

	HRESULT			UIDFetchEnvelopeAndExtraHeaders (
						IMAPUID		Uid, 
						ENVELOPE**	pEnvelope,
						char*		pFields,
						CWriter*	pWriter, ImapErrorFunc pErrFn = NULL);

	HRESULT			FetchAllFlags (CPtrUidList *pUidList, ImapErrorFunc pErrFn = NULL);
	HRESULT			FetchFlags ( LPCSTR pSequence, CPtrUidList *pUidList, ImapErrorFunc pErrFn = NULL);

	// Fetch flags using as STL "MAP".
	HRESULT			FetchFlags (LPCSTR pSequence, CUidMap *pUidMap, ImapErrorFunc pErrFn = NULL);

	HRESULT			FetchUID (unsigned long msgNum, IMAPUID *pUid, ImapErrorFunc pErrFn = NULL);

	// Wrappers around DLL functions.
	void			FreeMemory (void **pMem);

	void			DestroyEnvelope (ENVELOPE **ppEnvelope);

	HRESULT			UIDFetchStructure	(unsigned long uid, BODY **ppBody, ImapErrorFunc pErrFn = NULL);

	HRESULT			UIDFetchHeaderFull (IMAPUID uid, CWriter *pWriter, ImapErrorFunc pErrFn = NULL);
	HRESULT			UIDFetchRFC822HeaderFields (IMAPUID uid, char *sequence, char *Fields, CWriter *pWriter, ImapErrorFunc pErrFn = NULL);
	HRESULT			UIDFetchRFC822Header (IMAPUID uid, char *sequence, CWriter *pWriter, ImapErrorFunc pErrFn = NULL);

	HRESULT			GetUidValidity (unsigned long& UidValidity, ImapErrorFunc pErrFn = NULL);

	HRESULT			UIDDeleteMessages (LPCSTR pList, BOOL Expunge, ImapErrorFunc pErrFn = NULL);

	// Undelete
	HRESULT			UIDUnDeleteMessages (LPCSTR pList, ImapErrorFunc pErrFn = NULL);

	HRESULT			DownloadAttachmentContents (IMAPUID uid, char *sequence, CWriter *pWriter, ImapErrorFunc pErrFn = NULL);

	BOOL			IsSelected (ImapErrorFunc pErrFn = NULL);
	BOOL			IsConnected (ImapErrorFunc pErrFn = NULL);

	void			RecreateMessageMap ();

	HRESULT			GetLastImapError (LPSTR pBuffer, short nBufferSize);
	HRESULT			GetLastServerMessage (LPSTR pBuffer, short nBufferSize);

	// Ask IMAP if this stream is readonly.
	BOOL			IsReadOnly (ImapErrorFunc pErrFn = NULL);

	// STATUS command
	HRESULT			UIDFetchStatus (const char *pMailbox, long flags, MAILSTATUS *pStatus, ImapErrorFunc pErrFn = NULL);

	// Get the highest UID in the mailbox.
	HRESULT			UIDFetchLastUid  (unsigned long& LastUid,  ImapErrorFunc pErrFn = NULL);
	HRESULT			UIDFetchFirstUid (unsigned long& FirstUid, ImapErrorFunc pErrFn = NULL);

	// Fetch new nmsgs and uidvalidity.
	HRESULT			UpdateMailboxStatus (ImapErrorFunc pErrFn = NULL);

	HRESULT			GetTopMboxDelimiter (TCHAR *pDelimiter, ImapErrorFunc pErrFn = NULL);

	HRESULT			GetLastImapErrorAndType (LPSTR pBuffer, short nBufferSize, int *pType);

	HRESULT			UIDRemoveFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent = TRUE, ImapErrorFunc pErrFn = NULL);

	HRESULT			UIDAddFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent = TRUE, ImapErrorFunc pErrFn = NULL);

	// Error logging:
	//
	void			ErrorCallback (LPCSTR pBuf);


	// CAPABILITIES
	BOOL			HasStatusCommand ();
	BOOL			IsImap4Rev1 ();
	BOOL			IsImap4Only ();
	BOOL			IsLowerThanImap4 ();
	BOOL			HasOptimize1 ();
	BOOL			HasSort ();
	BOOL			HasScan ();
	BOOL			HasAcl ();
	BOOL			HasQuota ();
	BOOL			HasLiteralPlus ();
	BOOL			HasXNonHierarchicalRename ();


	// Find a string in any of the given header list or the message's body.

	HRESULT			UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr, CString& szResults, ImapErrorFunc pErrFn = NULL);

	// Search message text.
	HRESULT			UIDFindText (LPCSTR pSearchString, unsigned long UidFirst,
							 unsigned long UidLast, CString& szResults, ImapErrorFunc pErrFn = NULL);

	// Generic search.
	HRESULT			DoOnServerSearch (MultSearchCriteria* pMultiSearchCriteria, CString& szResults, LPCSTR pUidRange = NULL);

	// Access routines:
	void			 SetAccountID ( ACCOUNT_ID AccountId)
									{ m_AccountID = AccountId; }
	void			 SetAccount (CImapAccount *pAccount);

	// inline
	void			 SetPortNum (const char *PortNum)
							{ m_PortNum = PortNum; }
	void			 SetServer (const char *pServer)
							{ m_Server = pServer; }

	// Access:
	CImapStream*	GetStream		()
							{ return m_pImapStream; }
	const char *	GetPortNum ()
							{ return (const char *)m_PortNum; }
	ACCOUNT_ID		GetAccountID ()
							{ return m_AccountID; }
	LPCSTR			GetImapName ()
							{ return (LPCSTR)m_szImapName; }

	void			SetLogin (LPCSTR pStr)
						{ m_szLogin = pStr; }

	void			SetPassword (LPCSTR pStr)
						{ m_szPassword = pStr; }

	void			SetTooBusy (BOOL bVal)
						{m_bTooBusy = bVal; }

	BOOL			IsTooBusy ()
						{ return m_bTooBusy; }

	void			SetKrbLibName (LPCSTR pStr)
						{ m_szKrbLibName = pStr; }

	void			SetDesLibName (LPCSTR pStr)
						{ m_szDesLibName = pStr; }
	
	
	// Get messages count from the stream!!
	HRESULT			GetMessageCount(unsigned long& ulCount, ImapErrorFunc pErrFn = NULL);

	// Static global functions.
	// Do initialization and cleanup of the IMAP dll.
	//
	static HRESULT	Initialize ();
	static HRESULT	Cleanup();

public:

	// Manage network notifications:
	//
	void NetworkNotificationDispatcher (NetworkNotifierType NotifierType, unsigned long ulValue);

	// General network settings..
	//
	void UpdateNetworkSettings ();

	void SetErrorCallback	 (ImapErrorFunc func);

	void ResetErrorCallback ();
	void ResetProgressCallback ();

// Internal methods:
private:
	HRESULT		__OpenMailbox (const char *Fullname, BOOL bSilent, ImapErrorFunc pErrFn = NULL);
	HRESULT		__OpenControlStream(CImapAccount *pAccount, BOOL bSilent, ImapErrorFunc pErrFn = NULL);

	HRESULT		__UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr,  CString& szResults);

	void GrabSettings();
	void Init ();
	    			
// Internal state.
protected:
// These are included by value!! Must be destroyed when this class
// is destroyed.
	CImapStream*		m_pImapStream;

// Server identification.
	CString				m_Server;	// Network name or IP address of server.
	CString				m_PortNum;	// TCP/IP port number.

	// Login and password of user.
	//
	CString				m_szLogin;
	CString				m_szPassword;

//  If a mailbox is selected, set it's full IMAP name in this:
	CString m_szImapName;

	// When this is set by an external caller, copy the server and port number
	// into into the above m_Server and m_PortNum fields.
	ACCOUNT_ID			m_AccountID;
	CImapAccount		*m_pAccount;

// Internal data.
private:
	// User must set this by calling "UpdateNetworkSettings()"
	//
	QCNetworkSettings*	m_pNetSettings;

	// Contained object to handle notifications from teh network.
	//
	CNetworkNotifier*	m_pNetworkNotifier;

// Object's synchronization public mechanisms.
public:

	BOOL	LockNoWait(BOOL bAllowReEnter = FALSE);
	void	Unlock();

private:
	// Object's synchronization mechanisms.
	//
	BOOL	IsLocked();
	BOOL	__LockNoWait();

	// A thread must have a lock on this in order to proceed with a command.
	// 
//	CCriticalSection*	m_pThreadLockable;

	HANDLE	m_hThreadLockable;

	// Critical section used in negotiating for the real critical section.
	// This is done via LockNoWait() and UnLock().
	//
	CSyncObject*	m_pNegotiateLockable;

	// Thread ID of the one who's got the lock.
	//
	DWORD			m_curThreadID;


	// Allow multiple calls within the same thread.
	//
	long			m_lReEnterRefCount;

	// If this is TRUE, don't allow another command from whatever thread!!
	BOOL			m_bTooBusy;

	//
	CString			m_szKrbLibName;
	CString			m_szDesLibName;
	CString			m_szGssLibName;
};


// Exported functions - NOTE: Use sparingly.
void __ImapDebugLogger (char *string,long errflg);

#endif // _IMAPUTIL_H_

