#ifndef __IMAPACTL_H__
#define __IMAPACTL_H__

// For synchronization objects.
#include <afxmt.h>

// Include the file that defines the Account class.
#include "ImapAccount.h"

// Forward defs.
class CImapAccountList;


// ========== CImapAccountMgr class ============/
// 
// This is the global array of "account lists". Its elements are
// CImapAccountList's
// 
class CImapAccountMgr : public CObArray
{
public:
	CImapAccountMgr();
	~CImapAccountMgr();


// Interface
public:
// Public interface to managing accounts.	
//
	CImapAccount* AddAccountFromPersonality (LPCSTR pszPersonaName, LPCSTR pszParentDir, LPCSTR pszAccountSubdir);

	CImapAccount* FindAccount(ACCOUNT_ID AccountId,LPCSTR pPath = NULL);
	ACCOUNT_ID	  FindOwningAccount(LPCSTR pPathname);

	// Loop through all accounts, spanning account lists.
	CImapAccount* FindByName(LPCSTR pName, BOOL CaseSensitive = FALSE);
	ACCOUNT_ID	  FindFirstAccountID ();
	ACCOUNT_ID	  FindNextAccountID (ACCOUNT_ID CurAccountID);
	CImapAccount* FindFirstAccount ();
	CImapAccount* FindNextAccount (CImapAccount *pCurAccount);

	// retrieving an account with a lock on the account.
	//
	HRESULT FindAccount(ACCOUNT_ID AccountID, CImapAccount **ppAccount, AccountLockType sAccountLock);

	BOOL	SetTopMboxesFetched ( unsigned long AccountID, BOOL bVal );
	// Have  they been fetched?
	BOOL	TopMboxesFetched ( unsigned long AccountID );

	BOOL	VerifyMboxesFetched ( unsigned long AccountID );


	// Return the name of the top IMAP directory. 
	// Top-level accounts are created under this directory.
	// It is something like: <EudoraDir>/Imap.
	void	GetTopImapDirectory (CString &Dir);	

	// Create top directory.
	BOOL	CreateTopImapDirectory ();

				
	// Convenience functions.
	BOOL	DeleteAccount (ACCOUNT_ID AccountId);
	BOOL	DeleteMailbox (ACCOUNT_ID AccountId, LPCSTR pImapName, TCHAR Delimiter);

	// Return TRUE if the mailbox exists on the server.
	BOOL	MailboxExistsOnServer (ACCOUNT_ID AccountId, LPCSTR pImapName);

	// Clear account passwords.
	void	ClearAllPasswords ();

	void	ForgetPassword (LPCTSTR pPersona);

	// Are there accounts with cached passwords??
	BOOL	AccountsHavePasswords ();

	BOOL	GetPassword (LPCSTR pPersona, CString& szImapPassword);

	BOOL	ProcessActionQueues(bool bHighPriorityOnly);
	void	FinishUpActionQueues();

// Private list of IMAP lists. User shouldn't know about this.
private:
	// Note: The "name" is the rootDirectory attribute of a CImapAccountList.
	CImapAccountList*		AddAccountListByDir(LPCTSTR rootDirectory);
	BOOL					DeleteAccountListByDir(LPCSTR rootDirectory);
	BOOL					DeleteAccountList (CImapAccountList* pAccountList);
	int						FindAccountListByDir (const char* rootDirectory);

	// private Access.
	CImapAccountList* GetAt(int nIndex) const;
	CImapAccountList*& ElementAt(int nIndex)
		{ return ((CImapAccountList*&)CObArray::ElementAt(nIndex)); }
	CImapAccountList* operator [](int nIndex) const
		{ return (GetAt(nIndex)); }
	CImapAccountList*& operator [](int nIndex)
		{ return (ElementAt(nIndex)); }

	// private state:

	// These are set by the last ImapGetMail.
	BOOL m_bGotNewMail;

	// Syncgronization object.
	CSyncObject* m_pLockable;
};



//========================== CImapAccountList class =========================/


// Mbox account list class. This is an array of CImapAccount objects.
// Accounts can be created at any level in the folder hierarchy. An account 
// is differentiated by a unique ID. 
// An account must have:
//	- a NAME:	User defined name. 
//  - a Directory: Location on the local disk for creating mailboxes.
//
// There is one global in-memory list of CImapAccount's.
//
class CImapAccountList : public CObArray
{

public:
	CImapAccountList(LPCTSTR pRootDirectory);
	~CImapAccountList();
	
// Interface
public:
	// Fetch list of account from the pce file (or from ACAP) and create the list of
	// CImapAccount objects.
	//
	BOOL			Delete(CImapAccount* Account);
	CImapAccount*	FindByName(LPCSTR pName, BOOL CaseSensitive = FALSE );

	CImapAccount*	FindByID(ACCOUNT_ID AccountId);
	void			DeleteAllAccounts (void);

	// Add an already created account.
	BOOL			Add (CImapAccount *pAccount);

	// Access.
	CImapAccount* GetAt(int nIndex) const;
	CImapAccount*& ElementAt(int nIndex)
		{ return ((CImapAccount*&)CObArray::ElementAt(nIndex)); }
	CImapAccount* operator [](int nIndex) const
		{ return (GetAt(nIndex)); }
	CImapAccount*& operator [](int nIndex)
		{ return (ElementAt(nIndex)); }
	
	void SetRootDirectory (LPCSTR pRootDirectory);	// See implementation file.
	
	LPCTSTR GetRootDirectory (void)
		{ return (LPCSTR) m_rootDirectory; }
	BOOL NeedsReading ()
		{ return m_needsReading; }
	void SetNeedsReading (BOOL value)
		{ m_needsReading = value; }


// Attributes:
private:
	CString	m_rootDirectory;	// Fully qualified name of Eudora's directory at which
								// this account list is rooted. NOTE: MUST end with a trailing
								// directory delimiter.
	BOOL	m_needsReading;

};



// Globals
extern CImapAccountMgr g_ImapAccountMgr;


// Exported functions.
BOOL NewDirName (LPCSTR pParentDir, LPCSTR pSuggestedName, CString &NewName, short MaxNameLength);

CImapAccountMgr* GetImapAccountMgr();


#endif // __IMAPSRVL_H__
