// imapfr.h - definitions of the IMAP "friend" classes that actually call 
// the c-client-derived IMAP protocol functions

// Declarations for the CImap* friend classes.
//
// Notes:
// This file contains classes that actually do the implementation work internally
// that are exported in the imap.h file.


#ifndef _IMAPFR_H_
#define _IMAPFR_H_


//	========================================================
// CMImap class. Encapsulates IMAP stuff.
// 

class QCNetworkSettings;
class CSyncObject;
class CProtocol;

class CMImap
{
   
// Implementation
public:
	// Use this to create a "control" stream.
	CMImap(const char *ServerName, UINT ServiceID, const char *PortNum = NULL);
	~CMImap();

	BOOL		StreamIsLocked();

// Open a "control" stream, i.e., with no mailbox selected.
	HRESULT		OpenControlStream();

// Methods for initiating the connection to the IMAP server.
	HRESULT		OpenMailbox  (void);

// This sets the current mailbox.
	HRESULT		OpenMailbox  (const char *MailboxName);

	//
	void		RequestStop();


// Free memory allocated by this DLL.	
	void		FreeMemory(void **pMem);

	void		DestroyEnvelope(ENVELOPE **ppEnvelope);


	// Convert UID to a MSGNO.
	HRESULT		UidToMsgmno (IMAPUID uid, DWORD* dwMsgno);

// Mailbox-level commands that do not require a SELECTED mailbox.
	HRESULT		IMAPListUnSubscribed (LPCSTR pReference, CLister *Lister, BOOL INCLUDE_INBOX);

// Fetches.

// Top level message only.
	HRESULT		UIDFetchEnvelope (unsigned long uid, ENVELOPE **ppEnvelope);

	HRESULT		UIDFetchEnvelopeAndExtraHeaders (
						IMAPUID		Uid, 
						ENVELOPE**	pEnvelope,
						char*		pFields,
						CWriter*	pWriter);

	HRESULT		UIDFetchStructure	(unsigned long uid, BODY **ppBody);
	HRESULT		UIDFetchFlags (const char *pSequence, CEltWriter *pEltWriter);
	void		FreeBodyStructure	(BODY *pBody);

	HRESULT		UIDFetchFast (unsigned long uid, IMAPFULL **ppFlags);
	HRESULT		UIDFetchAll (unsigned long uid, IMAPFULL **ppFlags);
	HRESULT		UIDFetchFull	(unsigned long uid, IMAPFULL **ppFlags);

	HRESULT		UIDFetchInternalDate(unsigned long uid, char **ppDate);

	HRESULT		UIDFetchHeader	(unsigned long uid, CWriter *Writer);

	HRESULT		UIDFetchMessage  (unsigned long uid, CWriter *Writer, BOOL Peek);

	HRESULT		UIDFetchMessageBody (unsigned long uid, CWriter *Writer, BOOL Peek);


	HRESULT		UIDFetchPartialMessage  (unsigned long uid, unsigned long first, unsigned long nBytes,
						CWriter *Writer, BOOL Peek);

	HRESULT		UIDFetchPartialMessageBody (unsigned long uid, unsigned long first, unsigned long nBytes,
						CWriter *Writer, BOOL Peek);

	HRESULT		UIDFetchPartialBodyText (unsigned long uid, char *sequence, unsigned long first,
					unsigned long nBytes, CWriter *Writer, BOOL Peek);

	HRESULT		FetchUID (unsigned long msgNum, IMAPUID *pUid);

// These apply to a "message/rfc822"part. (Can also be used for top-level message.)
	HRESULT		UIDFetchRFC822Header (unsigned long uid, char *pSequence, CWriter *Writer);
	HRESULT		UIDFetchRFC822Text (unsigned long uid, char *sequence, CWriter *Writer);
	HRESULT		UIDFetchRFC822HeaderFields (unsigned long uid, char *sequence, char *Fields, CWriter *Writer);
	HRESULT		UIDFetchRFC822HeaderFieldsNot (unsigned long uid, char *sequence, char *fields, CWriter *Writer);
	
	// BODY part level fetches.
	HRESULT		UIDFetchMimeHeader (unsigned long uid, char *sequence, CWriter *Writer);
	HRESULT		UIDFetchBodyText (unsigned long uid, char *sequence, CWriter *Writer, BOOL Peek);

// Preamble and trailer text fetch (use complicated method).
// If "sequence" is NULL, apply to top level message. Must refer to a multipart message,
// an embedded message/rfc822 part that has a multipart body, or a multipart sub-part.
// or part. 
	HRESULT		UIDFetchPreamble(unsigned long uid, char *sequence, CWriter *Writer);
	HRESULT		UIDFetchTrailer(unsigned long uid, char *sequence, CWriter *Writer);

// Methods for STORE'ing flags. For IMAP commands that return new values of flags,
// the new values are returned in the IMAPFLAGS parameter.
// BUG: These methods should be extended to handle uid sets.
	HRESULT		UIDSaveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent);
	HRESULT		UIDAddFlags  (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent);
	HRESULT		UIDRemoveFlags (unsigned long uid, IMAPFLAGS *Flags, BOOL Silent);
	HRESULT		UIDRemoveFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */);
	HRESULT		UIDAddFlaglist (LPCSTR pUidList, LPCSTR pFlagList, BOOL Silent /* = TRUE */);

// COPY/MOVE methods. Destination mailbox MUST be on the same server.
// BUG: Should be extended to handle message sets.
	HRESULT		UIDCopy (const char *pUidlist, const char *pDestMailbox);

// Append:
	HRESULT		AppendMessage (const char* pFlags, unsigned long Seconds, STRING *pMsg);

// Utilities.
	HRESULT		UIDMessageIsMultipart (unsigned long uid, BOOL *pVal);

	HRESULT		UIDGetInternalDate (IMAPUID uid, unsigned long *pSeconds);

	HRESULT		GetRfc822Size (IMAPUID uid,unsigned long *pSize);

	char*		CreatePatSome (char *pReference);
	char*		CreatePatAll (char *Reference);
	char*		CreatePat (char *pReference, char WildCard);

// Return the mailbox's current UIDVALIDITY number.
// The mailbox must have already been OpenMailbox()'d before this will return a valid value, 
// otherwise it returns 0. (Note: UIDVALIDITY cannot be zero (see Imap4rev1 formal spec.)).
	UIDVALIDITY		UIDValidity();

// Search messages in a mailbox..

	void		Search (char *charset, SEARCHPGM *pgm, CString& szResults);
	// For searches.
	//
	BOOL		FormatSearchCriteria (SEARCHPGM *pPgm, LPCSTR pHeaderList, BOOL bBody, 
						BOOL bNot, LPCSTR pSearchString);

	HRESULT		UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
						LPCSTR pUidStr,  CString& szResults);

// Search entire message text.
	HRESULT		UIDFindText (LPCSTR pSearchString, unsigned long UidFirst, unsigned long UidLast, CString& szResults);

// Return the rdonly value in the protocol stream.
	BOOL			IsReadOnly ();

// Status-related methods.
	HRESULT		UIDFetchStatus (const char *pMailbox, long flags, MAILSTATUS *pStatus);

	BOOL		HasStatusCommand ();
	BOOL		IsImap4Rev1 ();
	BOOL		IsImap4Only ();
	BOOL		IsLowerThanImap4 ();
	BOOL		HasOptimize1 ();
	BOOL		HasSort ();
	BOOL		HasScan ();
	BOOL		HasAcl ();
	BOOL		HasQuota ();
	BOOL		HasLiteralPlus ();
	BOOL		HasXNonHierarchicalRename ();


// ========== non_UID functions ==================/
	HRESULT		FetchHeader (unsigned long msgNum, CWriter *Writer);

// Mailbox level methods:
// Check and NOOP
	HRESULT		Check();
	HRESULT		Noop ();

	// Create mailbox
	HRESULT		CreateMailbox (LPCSTR pImapName);
	HRESULT		DeleteMailbox (LPCSTR pImapName);
	HRESULT		RenameMailbox (LPCSTR pImapName, LPCSTR pNewImapName);

	// Subscribe a mailbox..
	HRESULT		SubscribeMailbox (LPCSTR pImapName);

	// UnSubscribeMailbox a mailbox..
	HRESULT		UnSubscribeMailbox (LPCSTR pImapName);

	// DO a LIST on the mailbox to get it's attributes.
	HRESULT		FetchMailboxAttributes (LPCSTR pImapName, CLister *Lister);

// Delete list of messages.
	HRESULT		UIDDeleteMessages (LPCSTR pUidList, BOOL Expunge);

// Undelete
	HRESULT		UIDUnDeleteMessages (LPCSTR pUidList);

// UID Expunge.
	HRESULT		UIDExpunge (LPCSTR pUidList, CString& szUidsActuallyRemoved);

// Simple Expunge.
	HRESULT		Expunge ();

	HRESULT		RecreateMessageMap ();

// Logout.
	HRESULT		Logout ();

	unsigned long	GetMessageCount();

	HRESULT		GetTopMboxDelimiter (TCHAR *pDelimiter);

// Methods for querying the state of the IMAP stream.
	BOOL		IsConnected();
	BOOL		IsAuthenticated();
	BOOL		IsSelected (void);

	// 
	void		RegisterUserCallback (ImapNotificationCallback_p pUserCallback, unsigned long ulData);

	// Set the QCNetworkSettings to point to the given parameter.
	// Note that this is contained by reference.
	//
	void		SetNetSettingsObject (QCNetworkSettings* pNetSettings)
					{ m_pNetSettings = pNetSettings; }

	void		SetLogin (LPCSTR pLogin)
					{ m_szLogin = pLogin; }
	void		SetPassword (LPCSTR pPassword)
					{ m_szPassword = pPassword; }

	// KERB:
	void		SetKrbDllName (LPCSTR pKrbLibName)
					{ m_szKrbLibName = pKrbLibName; }

	void		SetDesDllName (LPCSTR pDesLibName)
					{ m_szDesLibName = pDesLibName; }

	void		SetGssDllName (LPCSTR pGssLibName)
					{ m_szGssLibName = pGssLibName; }
		
	// For fetching error strings.
	HRESULT		GetLastErrorStringAndType (LPSTR szErrorBuf, short nBufSize, int *pType);

	HRESULT		GetLastServerMessage (LPSTR szErrorBuf, short nBufSize);

	void		SetErrorCallback (ImapErrorFunc pImapErrorFn);
	void		ResetErrorCallback ();

	void		AllowAuthenticate (BOOL Value);

	void		ClearErrorDatabase ();

	// These are class methods:  (STATIC)
	//
	static SEARCHPGM* CMImap::NewSearchPgm();

	static void	FreeSearchpgm (SEARCHPGM** pPgm);

	static void	FreePgmUidList (SEARCHPGM *pPgm);

	static BOOL	SetORSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList, BOOL bBody,
						LPCSTR pSearchString);

	static BOOL	SetORHeaderSearchCriteria (SEARCHPGM *pPgm, LPSTR pHeaderList,
						LPCSTR pSearchString);

	static BOOL	SetNewPgmUidList (SEARCHPGM *pPgm, LPCSTR pStr);


protected:  

	HRESULT OpenStream(const char *Mailbox);


// Internal state.
private:
	unsigned long	m_currentMsgNum;
	unsigned long	m_currentUID;
	unsigned long	m_MessageSizeLimit;

	// Name of mailbox
	char*			m_pName;			// Does NOT contain stuff like braces.

	// Use this for communication with c-client.
	//
	CProtocol		*m_pProtocol;

	// IMAP Server identification:
	char*			m_pServer;		// Name of server.
	char*			m_pPortNum;		// TCP/IP port number.

	// Login and password.
	//
	CString			m_szLogin;
	CString			m_szPassword;

	BOOL			m_bAllowAuthenticate;

	// Do this so caller can set this before the mailbox is opened.
	//
	ImapNotificationCallback_p m_pUserCallback;
	unsigned long	m_ulUserData;

	// ServiceID - This should go away.
	//
	UINT			m_ServiceID;

	// Contains by reference.
	//
	QCNetworkSettings *m_pNetSettings;

	//
	CString		m_szKrbLibName;
	CString		m_szDesLibName;
	CString		m_szGssLibName;

private:
	// Syncgronization object for locking the list while we search it or create a new one.
	//
	CSyncObject* m_pLockable;



};

#endif // _IMAPFR_H_


