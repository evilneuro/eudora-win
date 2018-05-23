
#ifndef __IMAPACCT_H__
#define __IMAPACCT_H__

// imapsrvc.h : header file
//

// For synchronization objects.
#include <afxmt.h>

// Forward declarations.
class	CImapAccountList;
class   CImapConnection;
class   QCImapMailboxCommand;
class	CImapThread;
class	CImapAction;
class	CActionQueue;


#include "ImapMailbox.h"
#include "sslsettings.h"

#include "ImapTypes.h"

typedef enum AccountLockType
{
	ACCOUNT_LOCK_NOT_LOCKED  = 1,
	ACCOUNT_LOCK_DONT_DELETE = 2,
	ACCOUNT_LOCK_EXCLUSIVE	 = 3,
};


typedef enum PasswordStatus
{
	PWVALIDITY_UNKNOWN	= 1,
	PWVALIDITY_IS_VALID		= 2,
	PWVALIDITY_MAYBE_VALID	= 3,	// We haven't logged in yet.
};


// CImapAccount class declaration
// This maintains a tree of mailboxes.
class CImapAccount : public CObject
{
	DECLARE_DYNAMIC(CImapAccount)
public:
	CImapAccount();
	CImapAccount (ACCOUNT_ID AccountID, LPCSTR pRootDirectory);
	~CImapAccount();

public:
	// Lock management:
	BOOL			 SetAdvisoryLock(AccountLockType sAccountLock);
	BOOL			 RemoveAdvisoryLock(AccountLockType sAccountLock);
	BOOL			 IsExclusiveLocked();

private:
	// Internal locking methods.
	void			 LockExclusively();
	void			 UnLockExclusively();
	void			 IncrementAdvisoryLock();
	void			 DecrementAdvisoryLock();

public:
	bool			 SetupControlStream(CString &strLogin, CString &strPassword);

	void			 InitQueue();
	void			 QueueAction(CImapAction *pImapAction);
	bool			 IsProcessing();
	bool			 ProcessQueue(bool bHighPriorityOnly, bool bOnlineStateChanged);
	bool			 MaybeSaveQueue(bool bDoItNow = false);
	bool			 WriteQueue();
	bool			 ReadQueue();
	void			 SetWritingQueue(bool bWritingQueue) {m_bWritingQueue = bWritingQueue;};
	bool			 IsWritingQueue() {return m_bWritingQueue;};
	void			 FinishUpQueue();
	bool			 HasHighPriorityItemsInQueue();
	bool			 ActionsQueuedForMailbox(CImapMailbox *pImapMailbox);

// Interface
	BOOL DoJunkScoring();
	BOOL UseJunkMailbox();
	void SetSupportsUIDPLUS(BOOL bSupports);
	BOOL SupportsUIDPLUS() {return m_bSupportsUIDPLUS;}
	void WarnIfUIDPLUSStatusChanged();

	void SetNameSpace(CString &strNameSpace);

	// Create the accounts containing directory on the local disk.
	BOOL				InitializeDir ();
	BOOL				ModifyFromPersonality (BOOL& MboxlistNeedsUpdating, BOOL IsNewAccount = FALSE);

	// Creating mailbox.
	QCImapMailboxCommand *FindMailboxAtRoot(CString &strName);
	QCImapMailboxCommand *CreateMailboxAtRoot(CString &strName,
											  BOOL bSilent = FALSE,
											  BOOL bMakeItAFolder = FALSE);
	QCImapMailboxCommand *CreateMailbox(LPCSTR szName,
										QCImapMailboxCommand *pParentCommand,
										BOOL bSilent = FALSE,
										BOOL bMakeItAFolder = FALSE);
	BOOL				 CreateMailboxOnServer(LPCSTR szImapName,
											   TCHAR cDelimiter);

	// Delete a mailbox.
	BOOL				 DeleteMailbox(LPCSTR pImapName, TCHAR Delimiter);
	BOOL				 DeleteMailboxOnServer(CString &strLogin,
											   CString &strPassword,
											   LPCSTR szImapName,
											   TCHAR cDelimiter);

	// Rename.
	BOOL				RenameMailbox(QCImapMailboxCommand *pImapCommand,
									  LPCSTR szNewImapName,
									  CString &strNewDirName,
									  BOOL &bOldMboxStillExists,
									  QCImapMailboxCommand **ppNewImapMailbox);
	BOOL				 RenameMailboxOnServer(CString &strLogin,
											   CString &strPassword,
											   LPCSTR szOldImapName,
											   LPCSTR szNewImapName,
											   TCHAR cDelimiter);

	BOOL				MoveMailbox ( QCImapMailboxCommand *pImapCommand, QCImapMailboxCommand *pNewParent,
								  BOOL& bOldMboxStillExists,
								  QCImapMailboxCommand **ppNewLastParent,
								  QCImapMailboxCommand **ppNewImapMailbox);

	// Fetch attributes of a single mailbox into an ImapMailboxNode structure.
	ImapMailboxNode*	FetchMailboxAttributes (LPCSTR pImapName, BOOL bSilent = TRUE);


	// Utility method.
	QCImapMailboxCommand* CreateChildCommandFromNode (QCImapMailboxCommand *pParentCommand, ImapMailboxNode *pImapNode, LPCSTR pNewName, CString szNewDirname);

	// Fetching mailbox lists.
	HRESULT				FetchChildMailboxList (LPCSTR pParentImapname, TCHAR Delimiter, ImapMailboxType Type, ImapMailboxNode **ppTopmostNode, BOOL Recurse);

	int					Insert(const char *mailbox, TCHAR delimiter, long attributes, ImapMailboxType Type);

	// Trash mailbox management.
	BOOL				GetTrashLocalMailboxPath (CString& TrashMailboxDir, BOOL MakeSureExists, BOOL bSilent = FALSE);
	BOOL				GetJunkLocalMailboxPath (CString& JunkMailboxDir, BOOL MakeSureExists, BOOL bSilent = FALSE);

	int					GetAutoExpungeSetting(int *piPercent);

	// Interface to checking "inbox" mail for this account.
	void				CheckMail ();

	BOOL				GotNewMail ()
						{ return m_bGotNewMail; }

	void				SetGotNewMail (BOOL bVal)
						{ m_bGotNewMail = bVal; }

	// Methods to do multithreaded IMAP operations.

	// Does Multithreaded fetch of new messages.
	BOOL				FetchNewMessages (CImapMailbox *pImapMailbox, CPtrList& UidList);

#if 0 // Not used
	// Management of active threads.
	void				AddThread (CImapThread *pThread);
	void				RemoveThread (CImapThread *pThread);
#endif


	// Get
	ACCOUNT_ID GetAccountID()
		{ return m_AccountID; }

	// Note these!!
	CString				GetPersona(){ return m_strPersona; }
	BOOL			    GetPrefix(CString& strBuf);
	BOOL			    GetName(CString& strBuf);
	BOOL				GetLogin(CString& strBuf);
	BOOL				GetPasswordMT(CString& strBuf);

	BOOL				GetSavedPassword (CString& Password);
	BOOL				SavePasswordToDisk ();

	BOOL				GetAddress(CString& strBuf);
	void				SetPasswordMT (LPCSTR pPassword);

	BOOL				GetDirectory(CString& strBuf);
	BOOL				GetPortNum(CString& strBuf);

	BOOL				IsAllowAuthenticate ();

	BOOL				IsAuthKerberos ();

	QCImapMailboxCommand *GetInMailbox(CString &strImapName);

	BOOL				UseFancyTrash();
	BOOL				GetTrashMailboxName(CString &strBuf, CString& strFullName, TCHAR *pcDelimiter);

	BOOL				GetJunkMailboxName(CString &strName, CString& strFullName, TCHAR *pcDelimiter);
	QCImapMailboxCommand *GetJunkMailbox(CString &strImapName, bool bCreate = true);

	BOOL				Login(CString& szLogin, CString& szPassword);

	CImapAccountList * GetParentList()
		{ return m_pParentList; }

	void SetParentList(CImapAccountList *pParentList)
		{ m_pParentList = pParentList; }

	void SetDirectory (LPCSTR pDirectory)
		{ m_Directory = pDirectory; }

	void SetTopDelimiter (TCHAR DelimiterChar)
		{ m_TopDelimiter = DelimiterChar; }

	TCHAR GetTopDelimiter ()
		{ return m_TopDelimiter; }

	// Password stuff.
	void	ValidatePasswordMT ();
	void	InvalidatePasswordMT (BOOL bEraseSavedPassword);
	PasswordStatus GetPasswordValidityMT ()
		{ return m_PasswordValidity; }

	BOOL MustSavePasswordToDisk ();
	QCSSLSettings m_SSLSettings; //SSL settings for this account.

// Internal utility methods.
private:
	void Init ();

	void __FetchChildMailboxList (LPCSTR pParentImapname, TCHAR Delimiter, ImapMailboxNode **ppNode, CImapConnection *pImap, short Level, BOOL IncludeInbox,  BOOL Recurse);
	BOOL __GetTrashLocalMailboxPath (CPtrList& List, CString& TrashMailboxDir);
	BOOL __GetJunkLocalMailboxPath (CPtrList& List, CString& JunkMailboxDir);
	void	ResetInternalState ();

// IMplementation of a delay before requesting password from user if he just cancelled.
//
	void	SetUserJustCancelledLogin ();
	BOOL	UserJustCancelledLogin ();

	void	ShowError(UINT iError, CImapConnection *pImap, LPCSTR szImapName, TCHAR cDelimiter);

// Internal Attributes.	
private:
	ACCOUNT_ID		m_AccountID;
	CString			m_strPersona;

	CActionQueue	*m_pActionQueue;
	bool			 m_bWritingQueue;

	// This was added so that when replaying actions that were performed offline we don't need to
	// create a new connectin for each account level action.  It may take a while to work this into
	// the general architecture -- we clearly don't need to keep an open connection around all the
	// time -- 	so don't be surprised if some class methods still keep a local copy of this around.
	// -dwiggins
	// Imap control stream for account level actions.
	CImapConnection	*m_pImap;

	// These are cached only for the session.
	CString			m_Password;
	CString			m_Directory;

	// Used only for detecting mods in the personality.
	// Don't expose these anywhere!!
	CString			m_Prefix;
	CString			m_Login;
	CString			m_Address;

	BOOL			m_bIgnoreNameSpace;
	CString			m_strNameSpace;

	// Flag if last check mail got any.
	BOOL m_bGotNewMail;

	// Top level mailbox delimiter.
	TCHAR m_TopDelimiter;

	// Pointer to parent account list of which this is a member.
	CImapAccountList* m_pParentList; 

	// Internal state used in fetching mailbox lists.
	short			m_curLevel;
	CString			m_curReference;			// Used in Insert().
	ImapMailboxNode *m_curParentMailbox;
	ImapMailboxNode		**m_curMailboxList;	// Found folders are added to this list. 
										// Note: This is a reference to memory allocated elsewhere. 
										// DON'T try to free this memory from this!!
	BOOL			m_bIgnoreInbox;     // Used to trap multiple instances of INBOX.

	// Junk mail handling.
	BOOL			m_bSupportsUIDPLUS;
	BOOL			m_bIgnoreUIDPLUS;
	BOOL			m_bWarnUserUIDPLUS;
	BOOL			m_bJunkMailboxOK;
	BOOL			m_bScoreJunk;

#if 0 // Not used
	// Thread list
	CPtrList		m_ThreadList;
#endif

	// Synchronization object. 
	//
	CSyncObject*	m_pLockable;

	// Value of lock on the account. One of:
	// ACCOUNT_LOCK_DONT_DELETE,
	// ACCOUNT_LOCK_EXCLUSIVE.
	//
	short			m_sAdvisoryLock;

	// Exclusive lock:
	BOOL			m_bExclusiveLock;

	// One of:
	// PWVALIDITY_UNKNOWN		- we haven't yet attempted to log in (default).
	// PWVALIDITY_MAYBE_VALID	- A mailbox is in the process of logging in with this
	//							- password.
	// PWVALIDITY_IS_VALID		- we're successfully logged in with this password this session.
	//
	PasswordStatus	m_PasswordValidity;

	// Set this timestamp whenever a user cancelled an attempt to login
	// to a server. Initialize to 0. 
	//
	time_t   m_tLoginCancelled;

	// Critical section for serializing access to password anD login methods.
	//
	CSyncObject*	m_pLoginLockable;

	// Set this to TRUE if we may need to save the password to disk.
	// Note: This may not be acted on if the IDS_INI_SAVE_PASSWORD_TEXT flag
	// is not set, but that must be checked in the main thread.
	//
	BOOL m_bPWNeedsSavingToDisk;
};






/////////////////////////////////////////////////////////////////////////////
#endif // __IMAPACCT_H__
