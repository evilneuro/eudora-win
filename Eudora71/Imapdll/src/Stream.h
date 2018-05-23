// Stream.h - Interface for the CStream helper class.
//
#ifndef __STREAM_H
#define __STREAM_H


// Imported.
//
extern const char *days[];	/* day name strings */
extern const char *months[];	/* month name strings */


// Forwards:
//

// Base class for the IMAP protocol implementation.
//
class CStream
{
public:
	CStream ();
	~CStream();

// Public methods:
//
public:

	void StreamNotify (NOTIFY_TYPE notify_type, void *data);

	void Lock()
		{ m_bLocked = TRUE; }

	void UnLock()
		{ m_bLocked = FALSE; }

	BOOL IsLocked ()
		{ return m_bLocked; }


	void SetMailGets (mailgets_t pGets)
		{ m_pMailgets = pGets; }

	mailgets_t GetMailGets ()
		{ return (mailgets_t) m_pMailgets; }

	void SetAllowAuthenticate (BOOL bValue)
		{ m_bAllowAuthenticate = bValue; }

	void SetKrbLibName (LPCSTR pName)
		{ m_szKrbLibName = pName; }

	void SetDesLibName (LPCSTR pName)
		{ m_szDesLibName = pName; }

	void GetGssLibraryName (CString& szName);
	void GetKrb4LibraryName (CString& szName);
	void GetDesLibraryName (CString& szName);

	void SetGssLibName (LPCSTR pName)
		{ m_szGssLibName = pName; }

	// Upcalls from the protocol itself.
	//
	void mm_searched	(unsigned long number);
	void mm_exists		(unsigned long number);
	void mm_recent		(unsigned long number);
	void mm_expunged	(unsigned long number);
	void mm_flags		(unsigned long number);
	void mm_uid			(unsigned long msgno, unsigned long uid);
	void mm_elt_flags	(MESSAGECACHE *elt);
	void mm_notify		(char *string, long errflg);
	void mm_alert		(char *string);
	void mm_list		(int delimiter,char *mailbox,long attributes);

	void mm_lsub (int delimiter, char *mailbox, long attributes);
	void mm_status (char *mailbox, MAILSTATUS *status);

	void mm_log			(char *string,long errflg);

	void mm_dlog		(char *string);
	void mm_login (char *user, char *pwd,long trial);
	void mm_critical ();
	void mm_nocritical ();
	long mm_diskerror (long errcode, long serious);
	void mm_fatal (char *string);
	void mm_error (unsigned short id);

	void	ReportError	(LPCSTR pStr, UINT Type);
	void	AppendErrorStringAndType	(UINT Type, LPCSTR pString);
	HRESULT GetLastServerMessage (LPSTR szErrorBuf, short nBufSize);
	int		GetLastErrorStringAndType (LPSTR szErrorBuf, short nBufSize);
	int		NumEntriesInErrorDbase ();
	void	ClearAllErrorEntries();
	void	AddLastError(int Type, UINT StringID, ...);
	void	AddLastErrorString (int Type, const char *pString);

	void	SetErrorCallback (ImapErrorFunc pImapErrorFn);
	void	ResetErrorCallback ();

	void	SetAlertCallback (ImapErrorFunc pImapAlertFn);

// Login and password stuff:
//
	void SetLogin (LPCSTR pLogin)
		{ m_szLogin = pLogin; }

	LPCSTR GetLogin ()
		{ return m_szLogin; }

	void SetPassword (LPCSTR pPw)
		{ m_szPassword = pPw; }

	LPCSTR GetPassword ()
		{ return m_szPassword; }

	
	// Connection state:
	//
	void SetAuthenticated (BOOL value)
		{ m_bAuthenticated = value; }

	void SetConnected (BOOL value)
		{ m_bConnected = value; }

	void SetSelected (BOOL value)
		{ m_bSelected = value; }

	BOOL	IsConnected ()
		{ return m_bConnected; }

	BOOL	IsAuthenticated ()
		{ return m_bAuthenticated; }

	BOOL	IsSelected ()
		{ return m_bSelected; }

	// Capabilities:
	//
	BOOL HasStatusCommand ()
		{ return (m_bImap4rev1 || m_bUseStatus); }

	BOOL IsImap4Rev1 ()
		{ return m_bImap4rev1; }

	// Either 4rev1 or 4.
	BOOL IsImap4 ()
		{ return (m_bImap4rev1 == 1) || (m_bImap4 == 1); }

	BOOL IsImap4Only ()
		{ return m_bImap4; }

	BOOL IsLowerThanImap4 ()
		{ return (m_bImap4 == 0 && m_bImap4rev1 == 0); }

	BOOL IsImap2bis ()
		{ return m_bImap2bis; }

	BOOL IsImap1176 ()
		{ return m_bRfc1176; }
	
	BOOL HasOptimize1 ()
		{ return m_bOptimize1; }

	BOOL HasSort ()
		{ return m_bSort; }

	BOOL HasScan ()
		{ return m_bUseScan; }

	BOOL HasAcl ()
		{ return m_bAcl; }

	BOOL HasQuota ()
		{ return m_bQuota; }

	BOOL HasLiteralPlus ()
		{ return m_bLiteralplus; }

	BOOL HasXNonHierarchicalRename ()
		{ return m_bX_NonHierarchicalRename; }

	BOOL HasUIDPLUS()
		{ return m_bSupportsUIDPLUS; }

	BOOL HasNAMESPACE()
		{ return m_bSupportsNAMESPACE; }

	void GetNameSpace(CString &strNameSpace)
		{ strNameSpace = m_strNameSpace; }

	// 
	void SetUserCallback(ImapNotificationCallback_p pUserCallback)
		{ m_pUserCallback = pUserCallback; }

	void SetUserCallbackData(unsigned long ulData)
		{ m_ulUserData = ulData; }

	// Mailbox name:
	//
	LPCSTR GetMailbox ()
		{ return m_szMailbox; }

	void SetMailbox (LPCSTR pMailboxName)
		{ m_szMailbox = pMailboxName; }

	BOOL IsReadOnly()
		{ return m_bReadOnly; }

	BOOL IsAnonymous ()
		{ return m_bAnonymous; }

	unsigned long GetUidValidity ()
		{ return m_UidValidity; }

	unsigned long GetNumberOfMessages()
		{ return m_Nmsgs; }

	TCHAR GetDelimiterChar ()
		{ return m_DelimiterChar; }


// Externals access these - must disallow!
// Data:
public:

	// THis should go away!!
	unsigned int m_ServiceID;


	void			*mgets_data;
	void			*mboxgets_data;
	void 			*m_pEltWriter;
	
	// The last tagged response is copied here before parsing so it can be available to
	// a caller.
	// NOTE: Make sure this is as array because we use "sizeof" to determine it's size.
	TCHAR		szSrvTRespBuf [512];


// Set by descendent:
//
protected:
	void SetReadOnly (BOOL bValue)
		{ m_bReadOnly = bValue; }

	void SetAnonymous (BOOL bValue)
		{ m_bAnonymous = bValue; }

	void ClearMessageMap();

// Private functions:
private:
	void Initialize();


// Descendent can inherit.
// Attributes.
protected:
	// Store message list results in this, as a command separated list of uid's.
	CString			m_szSearchResults;

	// Library names:
	//
	CString		m_szKrbLibName;
	CString		m_szDesLibName;
	CString		m_szGssLibName;

	//
	unsigned long		m_PermUserFlags;			/* mask of permanent user flags */
	unsigned long		m_Gensym;					/* generated tag */
	unsigned long		m_Recent;					/* # of recent msgs */

	unsigned long		m_UidValidity;				/* UID validity sequence */
	unsigned long		m_UidLast;					/* last assigned UID */

	char*				m_UserFlags[NUSERFLAGS];	/* pointers to user flags in bit order */

	unsigned long		m_Nmsgs;					/* # of associated msgs */

	unsigned int		m_UseAuth;

	/* Added by JOK !!!  Used to pass back context data.  */
	void			*login_data;

	// Per-stream drivers:
	void		*m_pMailgets;
	void		*mailreadprogress;

	// Set to TRUE if not to user AUTHENTICATE.
	BOOL m_bAllowAuthenticate;

	// Pass back top level delimiter char in this.
	TCHAR m_DelimiterChar;

	// This is protected by the following mutex:
	ImapErrorFunc	m_pErrorFn;
	HANDLE			m_hErrorLockable;

	ImapErrorFunc	m_pAlertFn;

	// Stuff that used to be in "LOCAL"
	//
	IMAPPARSEDREPLY m_Reply;			/* last parsed reply */

	BOOL	m_bImap4rev1;			/* server is IMAP4rev1 */
	BOOL	m_bImap4;				/* server is IMAP4 */
	BOOL	m_bImap2bis;			/* server is IMAP2bis */
	BOOL	m_bRfc1176;				/* server is RFC-1176 IMAP2 */
	BOOL	m_bUseStatus;			/* server has STATUS */
	BOOL	m_bUseScan;				/* server has SCAN */
	BOOL    m_bSupportsSSL;         // server supports SSL
	BOOL    m_bSupportsUIDPLUS;     // server supports UIDPLUS
	BOOL	m_bSupportsNAMESPACE;	// server supports NAMESPACE
	BOOL	m_bUidsearch;			/* UID searching */
	BOOL	m_bByeseen;				/* saw a BYE response */

	BOOL				m_bPermSeen;				/* permanent Seen flag */
	BOOL				m_bPermDeleted;				/* permanent Deleted flag */
	BOOL				m_bPermFlagged;				/* permanent Flagged flag */
	BOOL				m_bPermAnswered;			/* permanent Answered flag */
	BOOL				m_bPermDraft;				/* permanent Draft flag */
	BOOL				m_bKwdCreate;				/* can create new keywords */

	// Additional capabilities bitfields.
	BOOL	m_bAcl;
	BOOL	m_bQuota;
	BOOL	m_bLiteralplus;
	BOOL	m_bX_NonHierarchicalRename;

	BOOL	m_bSort;
	BOOL	m_bOptimize1;

	BOOL		m_bReadOnly;

	BOOL		m_bAnonymous;

	char *m_Prefix;					/* find prefix */
	char m_tmpbuf[IMAPTMPLEN];		/* temporary buffer */

	CString		m_strNameSpace;

	// This is a linear list mapping msgno to UID.
	// We use a linked list so we can easily renumber expunged messages, i.e.,
	// the numbering is implicit.
	//
	CPtrMsgList	m_msgMap;

	// Used to return list of uid's acrually removed.
	//
	CString		m_szExpungeUidList;

private:
	// Store these status flags explicitly (JOK).
	BOOL		m_bConnected;
	BOOL		m_bAuthenticated;
	BOOL		m_bSelected;

	// Locked flag.
	//
	BOOL		m_bLocked;

	CString		m_szLogin;
	CString		m_szPassword;

	// User registers this when this struct is instantiated:
	//
	ImapNotificationCallback_p	m_pUserCallback;
	//
	// User passes this opaque data that must be passed back in 
	// m_pUserCallback.
	//
	unsigned long m_ulUserData; 

	// Our mailbox name.
	//
	CString		m_szMailbox;

	// Error database. This is an MFC list object.
	CPtrList	m_ErrorList;
};



#endif // __STREAM_H