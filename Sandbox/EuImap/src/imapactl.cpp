// imapactl.cpp: implementation
//
// Functions to add IMAP mailboxes to Mailboxes and Transfer menus.
// For WIN32, mailboxes are also added to the tree control.

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include <fcntl.h>

#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "cursor.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "usermenu.h"
// #include "mboxmap.h"
#include "font.h"
// #include "recip.h"
#include "station.h"
#include "utils.h"
#include "guiutils.h"

#ifdef WIN32
// #include "tooltip.h"	// for MAINFRM.H
#include "mainfrm.h"
#endif

#include "eudora.h"
#include "ems-wglu.h"
#include "trnslate.h"
#include "toolmenu.h"
#include "sigmgr.h"
#include "persona.h"
#include "pop.h"

#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"

#include "imapopt.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imaputil.h"

#include "ImapMailMgr.h" //for CImapMailMgr

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// We need to extern this - yuck!!
extern QCMailboxDirector	g_theMailboxDirector;


// Globals
CImapAccountMgr g_ImapAccountMgr;


// ============= INTERNALS ==========
static const DWORD dwLockTimeout = 100;			// Milliseconds.

// ============ Local Functions ===================//


// ==========================================================================//


// ================== class CImapAccountList =================================

CImapAccountList::CImapAccountList(LPCSTR rootDirectory)
{
	m_needsReading = TRUE;

	if (rootDirectory)
		m_rootDirectory = rootDirectory;
	else
		m_rootDirectory.Empty();
}


// Make sure we free our allocated objects in the array elements.
CImapAccountList::~CImapAccountList()
{
	for (int i = 0; i < GetSize(); i++)
	{
		CImapAccount* pAccount = GetAt(i);
		if (pAccount != NULL)
		{
			delete pAccount;
			ElementAt(i) = NULL;
		}
		else
			ASSERT(0);
	}

	RemoveAll ();
}



// SetRootDirectory [PUBLIC]
// NOTES
// The value of m_rootDirectory MUST end with a trailing directory delimiter.
// END NOTES
void CImapAccountList::SetRootDirectory (LPCSTR pRootDirectory)
{
	m_rootDirectory = pRootDirectory;

	// Trim and add trailing directory delimiter.
	if (!m_rootDirectory.IsEmpty())
	{
		m_rootDirectory.TrimRight();
		m_rootDirectory.TrimLeft();
		if (!m_rootDirectory.IsEmpty())
		{
			if (m_rootDirectory.Right ( 1 ) != DirectoryDelimiter)
				m_rootDirectory + DirectoryDelimiter;
		}
	}
}



// Add an already created account.
BOOL  CImapAccountList::Add(CImapAccount *pAccount)
{
	// Sanity checks.
	if (!pAccount)
		return FALSE;

	if (CObArray::Add((CObject *)pAccount) >= 0)
	{
		// Set parent list.
		pAccount->SetParentList (this );
		return TRUE;
	}
	else
		return FALSE;
}



//
// Remove the account from the account list AND delete the account object.
//
BOOL CImapAccountList::Delete(CImapAccount* pAccount)
{
	BOOL bResult = FALSE;

	if (!pAccount)
		return TRUE;

	// Loop through the array looking for "pAccount" and remove it if found.
	for (int i = 0; i < GetSize(); i++)
	{
		if ((CImapAccount *)GetAt(i) == pAccount)
		{
			// Remove account from list.
			RemoveAt (i, 1);

			// Delete the account objetc also.
			delete pAccount;

			bResult = TRUE;
			break;
		}
	}

	return bResult;
}


// Delete all account from this account list. That means deleting the folder tree
// from each account as well.
void CImapAccountList::DeleteAllAccounts(void)
{
	CImapAccount*	pAccount = NULL;
	int			count = GetSize();

	for (short i = 0; i < count; i++)
	{
		pAccount = (CImapAccount *)GetAt (i);
		if (pAccount)
			delete pAccount;
	}

	// Free array memory. Note that we had to free the objects ourselves.
	RemoveAll();
}


////////////////////////////////////////////////////////////////////////
// GetAt [public]
//
// Tsk, tsk.  Redefinition which hides the non-virtual implementation in
// the base class.  Oh, well ... at least it gives an opportunity for
// type casting with the appropriate sanity checks.
////////////////////////////////////////////////////////////////////////
CImapAccount* CImapAccountList::GetAt(int nIndex) const
{
	if (nIndex < 0 || nIndex > GetUpperBound())
	{
		ASSERT(0);
		return NULL;
	}

	CImapAccount* pAccount = (CImapAccount *) CObArray::GetAt(nIndex);
	if (pAccount)
	{
		ASSERT(pAccount->IsKindOf(RUNTIME_CLASS(CImapAccount)));
	}
	else
	{
		ASSERT(0);
	}
	return pAccount;
}

	
//////////////////////////////////////////////////////////////////////
// FindByName [public]
// Given an account name, loop through all accounts looking for it.
//////////////////////////////////////////////////////////////////////
CImapAccount* CImapAccountList::FindByName(LPCSTR pName, BOOL CaseSensitive /* = FALSE */)
{
	CImapAccount	*pAccount = NULL;
	int				i, iResult;
	CString			myName, yrName;

	// Sanity
	if (!pName)
		return NULL;

	yrName = pName;
	// Strip any trailing directory delimiter.
	StripTrailingDelimiter (myName);

	// Loop through.
	for (i = 0; i < GetSize(); i++)
	{
		pAccount = GetAt (i);
		if (pAccount)
		{
			pAccount->GetName(myName);
			if ( !myName.IsEmpty() )
			{
				StripTrailingDelimiter (myName);
				if (CaseSensitive)
				{
					iResult = yrName.Compare (myName);
				}
				else
				{
					iResult = yrName.CompareNoCase (myName);
				}
				if (iResult == 0)
					return pAccount;
			}
		}
	}

	return NULL;
}



//////////////////////////////////////////////////////////////////////
// FindByID [public]
// Given an account ID, loop through all accounts looking for it.
//////////////////////////////////////////////////////////////////////
CImapAccount* CImapAccountList::FindByID(ACCOUNT_ID AccountId)
{
	CImapAccount	*pAccount = NULL;
	int				i;

	// Sanity
	if (AccountId == 0)
		return NULL;

	// Loop through.
	for (i = 0; i < GetSize(); i++)
	{
		pAccount = GetAt (i);
		if (pAccount)
		{
			if (pAccount->GetAccountID() == AccountId)
				return pAccount;
		}
	}

	return NULL;
}




// ====================== CImapAccountMgr methods  ==================//


CImapAccountMgr::CImapAccountMgr(void)
{
	m_bGotNewMail = FALSE;

	// Create the sync. object. Use a critical section.
	m_pLockable = new CCriticalSection;
}


// 
CImapAccountMgr::~CImapAccountMgr()
{
	for (int i = 0; i < GetSize(); i++)
	{
		CImapAccountList* pAccountList = GetAt(i);
		if (pAccountList != NULL)
		{
			delete pAccountList;
			ElementAt(i) = NULL;
		}
		else
			ASSERT(0);
	}

	RemoveAll ();

	// Delete the sync. object:
	if (m_pLockable)
		delete m_pLockable;
}



//======== Private list management methods ===================/

// AddAccountListByDir [private]
// Create a new account object and add it to the array.
// An account list is identified by its rootDirectory.
CImapAccountList* CImapAccountMgr::AddAccountListByDir(LPCTSTR pRootDirectory)
{
	CImapAccountList	*pAccountList = NULL;

	if (!pRootDirectory)
		return FALSE;

	// Create the Account list object.
	pAccountList = new CImapAccountList(pRootDirectory);
	if (!pAccountList)
		return FALSE;

	// Set root directory.
	pAccountList->SetRootDirectory (pRootDirectory);

	// Whoever is going to read the list must explicitly set this.
	pAccountList->SetNeedsReading (FALSE);

	// Now, add to CObArray.
	if (CObArray::Add((CObject *)pAccountList) >= 0)
		return pAccountList;
	else
	{
		delete pAccountList;
		return NULL;
	}						
}


// DeleteAccountListByDir [private]
// Looks for the account list with given name and removes it from the array.
// Returns TRUE only if found and removed.
BOOL CImapAccountMgr::DeleteAccountListByDir(LPCSTR pRootDirectory)
{
	BOOL bResult = FALSE;

	if (!pRootDirectory)
		return FALSE;
	
	int i = FindAccountListByDir (pRootDirectory);
	if (1 >= 0)
	{
		RemoveAt (i, 1);
		bResult = TRUE;
	}

	return bResult;
}



// DeleteAccountList [private]
BOOL CImapAccountMgr::DeleteAccountList (CImapAccountList* pAccountList)
{
	BOOL bResult = FALSE;

	if (!pAccountList)
		return TRUE;

	// Loop through the array looking for "pAccount" and remove it if found.
	for (int i = 0; i < GetSize(); i++)
	{
		if ((CImapAccountList *)GetAt(i) == pAccountList)
		{
			// Remove account from list.
			RemoveAt (i, 1);
			bResult = TRUE;
			break;
		}
	}

	return bResult;
}




// FindAccountListByDir [private]
// Searches through the array looking for an entry with given "rootDirectory".
// Returns the index to the object or -1 if not found.
//
int CImapAccountMgr::FindAccountListByDir (LPCTSTR pDir)
{
	int				index = -1;
	int				count = GetSize();
	CImapAccountList*	pAccountList = NULL;
	CString			Dir, AccountDir;

	if (!pDir)
		return -1;
	if (!*pDir)
		return -1;

	// Make sure ends in a directory delimiter. This will take care of the case
	// where the directory is the root directory (i.e. pDir == '\\' alone.).
	Dir = pDir;
	Dir.TrimLeft();
	Dir.TrimRight();
	if (Dir.Right ( 1 ) != DirectoryDelimiter)
	{
		Dir += DirectoryDelimiter;
	}
	AccountDir.Empty();
	for (int i = 0; i < count; i++)
	{
		pAccountList = (CImapAccountList *)GetAt (i);
		if (pAccountList)
		{
			AccountDir = pAccountList->GetRootDirectory();
			AccountDir.TrimLeft();
			AccountDir.TrimRight();
			if (AccountDir.Right ( 1 ) != DirectoryDelimiter)
			{
				AccountDir += DirectoryDelimiter;
			}
			// Compare now.
			if (AccountDir.Compare(Dir) == 0)
			{
				index = i;
				break;
			}
		}
	}

	return index;
}




////////////////////////////////////////////////////////////////////////
// GetAt [private]
//
// Tsk, tsk.  Redefinition which hides the non-virtual implementation in
// the base class.  Oh, well ... at least it gives an opportunity for
// type casting with the appropriate sanity checks.
////////////////////////////////////////////////////////////////////////
CImapAccountList* CImapAccountMgr::GetAt(int nIndex) const
{
	if (nIndex < 0 || nIndex > GetUpperBound())
	{
		ASSERT(0);
		return NULL;
	}

	return (CImapAccountList *) CObArray::GetAt(nIndex);
}




//========= CImapAccountMgr public interfaces.
// AddAccountFromPersonality [PUBLIC]

// FUNCTION
// Create an account object in the account list for the local directory "pszParentDir".
// "pszAccountSubDir" is the accout's directory, relative to pszParentDir.
// END FUNCTION

// NOTES
// Assume that the personality is an IMAP personality.
// END NOTES

// HISTORY
// Created 9/15/97 by JOK.
// END HISTORY

CImapAccount* CImapAccountMgr::AddAccountFromPersonality (LPCSTR pszPersonaName, LPCSTR pszParentDir, LPCSTR pszAccountSubdir)
{
	CImapAccountList*	pAccountList = NULL;
	CImapAccount*		pAccount = NULL;

	// Sanity.
	if (! (pszPersonaName && pszParentDir && pszAccountSubdir) )
		return NULL;

	// Do we already have entries for this parent dir??
	int listIndex = FindAccountListByDir (pszParentDir);
	if (listIndex >= 0)
		pAccountList = GetAt (listIndex);
	else
	{
		pAccountList = AddAccountListByDir (pszParentDir);
	}

	if (!pAccountList)
		return NULL;

	// Set the hash of the personality name as the "AccountID".
	DWORD AccountID = g_Personalities.GetHash( pszPersonaName );
	if (AccountID == 0)
		return NULL;

	//
	// Formulate the full path to the account's directory.
	// Note: the account list's root directory ends in a directory delimiter.
	//
	CString sAccountDir = pAccountList->GetRootDirectory ();
	sAccountDir += pszAccountSubdir;

	//
	// Create account object here.
	//
	pAccount = new CImapAccount (AccountID, sAccountDir);
	if (!pAccount)
		return NULL;

	// Make sure the directory exists.
	if (!pAccount->InitializeDir ())
	{
		delete pAccount;
		pAccount = NULL;

		return NULL;
	}

	// Add it to the account list if we succeeded.
	if (!pAccountList->Add (pAccount))
	{
		delete pAccount;
		pAccount = NULL;
	}

	return pAccount;
}



//////////////////////////////////////////////////////////////////////
// FindAccount [public]
// Given an account ID, loop through all account lists looking for it.
// If pPath is non_NULL, search only the account list for that directory level.
//////////////////////////////////////////////////////////////////////
CImapAccount* CImapAccountMgr::FindAccount(ACCOUNT_ID AccountId, LPCSTR pPath /* = NULL */)
{
	CImapAccountList	*pAccountList = NULL;
	CImapAccount		*pAccount = NULL;
	int				listIndex = 0;
	int				numLists = 0;

	// Sanity
	if (AccountId == 0)
		return NULL;

	numLists = GetSize();
	if (numLists <= 0)
		return NULL;

	// We may have to loop through all lists.
	listIndex = 0;

	// If pPath, look for a list at this directory level.
	if (pPath)
	{
		listIndex = g_ImapAccountMgr.FindAccountListByDir (pPath);
		if (listIndex < 0)
			listIndex = 0;
	}

	for ( ; listIndex < numLists; listIndex++)
	{
		pAccountList = GetAt (listIndex);
		if (pAccountList)
		{
			pAccount = pAccountList->FindByID (AccountId);
			if (pAccount)
				break;
		}
	}

	return pAccount;
}



//////////////////////////////////////////////////////////////////////
// FindByName [public]
// Given an account name, loop through all accounts looking for it.
//////////////////////////////////////////////////////////////////////
CImapAccount* CImapAccountMgr::FindByName(LPCSTR pName, BOOL CaseSensitive /* = FALSE */)
{
	CImapAccountList *pAccountList = NULL;
	CImapAccount     *pAccount = NULL;

	// Loop through all lists.
	for (int i = 0; i < GetSize(); i++)
	{
		pAccountList = GetAt (i);
		if (pAccountList)
		{
			pAccount = pAccountList->FindByName (pName, CaseSensitive);
			if (pAccount)
				return pAccount;
		}
	}

	return NULL;
}



//////////////////////////////////////////////////////////////////////
// FindOwningAccount [public]
// Given the name of an account, namespace or mailbox, find the account that contains
// it. (In the case of an account, find the account.)
// Loop throuh all account lists.
//////////////////////////////////////////////////////////////////////
ACCOUNT_ID CImapAccountMgr::FindOwningAccount(LPCSTR pPathname)
{
	CImapAccountList	*pAccountList = NULL;
	CImapAccount		*pAccount = NULL;
	int					length;
	CString				myPath, yrPath, buf;

	// Sanity
	if (!pPathname)
		return BAD_ACCOUNT_ID;

	yrPath = pPathname;
	StripTrailingDelimiter ( myPath );

	// We may have to loop through all lists.
	for (int i = 0; i < GetSize(); i++)
	{
		pAccountList = GetAt (i);
		if (pAccountList)
		{
			for (int j = 0; j < pAccountList->GetSize(); j++)
			{
				pAccount = pAccountList->GetAt(j);
				if (pAccount)
				{
					pAccount->GetDirectory(myPath);
					StripTrailingDelimiter (myPath);

					if (!myPath.IsEmpty())
					{
						length = myPath.GetLength();
						buf = yrPath.Left (length);
						if ( buf.CompareNoCase (myPath) == 0 )
							return pAccount->GetAccountID();
						
					}
				}
			}
					
		}
	}

	return BAD_ACCOUNT_ID;
}




// FUNCTION
// Find the very first account if the first account list.
// END FUNCTION
ACCOUNT_ID CImapAccountMgr::FindFirstAccountID ()
{
	CImapAccountList	*pAccountList = NULL;
	CImapAccount		*pAccount = NULL;
	ACCOUNT_ID			ID;

	// Do we even have any accounts??
	if (GetSize() <= 0)
		return 0;

	// Initialize
	ID = 0;

	// Find the first account list.
	pAccountList = GetAt (0);
	if (pAccountList)
	{
		if (pAccountList->GetSize() > 0)
		{
			pAccount = pAccountList->GetAt(0);
			if (pAccount)
				ID = pAccount->GetAccountID();
		}
	}
	
	return ID;
}



// FUNCTION
// Get the next account ID, either in the same list or the next.
// A returned value of 0 indicates no more accounts.
// END FUNCTION
ACCOUNT_ID CImapAccountMgr::FindNextAccountID (ACCOUNT_ID CurAccountID)
{
	CImapAccountList	*pAccountList = NULL;
	CImapAccount		*pAccount = NULL;
	int				listIndex = 0;
	int				numLists = 0;
	ACCOUNT_ID		ID;

	// If current account ID is 0, get the first account ID.
	if (CurAccountID == 0)
		return FindFirstAccountID();

	numLists = GetSize();
	if (numLists <= 0)
		return 0;

	// We have to loop through all lists.
	ID = 0;
    BOOL FOUND = FALSE;

	for (listIndex = 0; listIndex < numLists; listIndex++)
	{
		pAccountList = GetAt (listIndex);
		if (pAccountList)
		{
			for (int i = 0; i < pAccountList->GetSize(); i++)
			{
				pAccount = pAccountList->GetAt (i);
				if (pAccount)
				{
					// Have we yet found the given ID?
					if (FOUND)
					{
						ID = pAccount->GetAccountID();
						break;
					}
					else if (pAccount->GetAccountID() == CurAccountID)
						FOUND = TRUE;
				}
			}
		}
	}

	return ID;
}




// FUNCTION
// Find the very first account if the first account list.
// END FUNCTION
CImapAccount *CImapAccountMgr::FindFirstAccount ()
{
	CImapAccountList	*pAccountList = NULL;
	CImapAccount		*pAccount = NULL;

	// Do we even have any accounts??
	if (GetSize() <= 0)
		return 0;

	// Find the first account list.
	pAccountList = GetAt (0);
	if (pAccountList)
	{
		if (pAccountList->GetSize() > 0)
		{
			pAccount = pAccountList->GetAt(0);
			if (pAccount)
				return pAccount;
		}
	}
	
	return NULL;
}



// FUNCTION
// Get the next account, either in the same list or the next.
// A returned value of NULL indicates no more accounts.
// END FUNCTION
CImapAccount* CImapAccountMgr::FindNextAccount (CImapAccount *pCurAccount)
{
	CImapAccountList	*pAccountList = NULL;
	CImapAccount		*pAccount = NULL;
	int				listIndex = 0;
	int				numLists = 0;

	// If current account ID is NULL, get the first account.
	if (pCurAccount == NULL)
		return FindFirstAccount();

	numLists = GetSize();
	if (numLists <= 0)
		return 0;

	// We have to loop through all lists.
    BOOL FOUND = FALSE;

	for (listIndex = 0; listIndex < numLists; listIndex++)
	{
		pAccountList = GetAt (listIndex);
		if (pAccountList)
		{
			for (int i = 0; i < pAccountList->GetSize(); i++)
			{
				pAccount = pAccountList->GetAt (i);
				if (pAccount)
				{
					// Have we yet found the given ID?
					if (FOUND)
					{
						return pAccount;
						break;
					}
					else if (pAccount->GetAccountID() == pCurAccount->GetAccountID())
						FOUND = TRUE;
				}
			}
		}
	}

	return NULL;
}




	





// DeleteAccount [public]
// FUNCITON
// Delete an account given it's ID
// END FUNCTION
//
BOOL CImapAccountMgr::DeleteAccount (ACCOUNT_ID AccountId)
{
	int				i;
	int				count = GetSize();
	CImapAccountList*	pAccountList = NULL;
	CImapAccount*		pAccount = NULL;

	if (!AccountId)
		return NULL;

	// MUST have some account lists.
	if (count <= 0)
		return FALSE;

	// Look for the containing list.
	for (i = 0; i < count; i++)
	{
		pAccountList = (CImapAccountList *)GetAt (i);
		if (pAccountList)
		{
			pAccount = pAccountList->FindByID(AccountId);
			if (pAccount)
				break;
		}

		// We use these to check for success below.
		pAccountList = NULL;
		pAccount = NULL;
	}

	// Did we find a valid account?
	if (!(pAccountList && pAccount))
		return FALSE;

	// Delete the account from the list. Note: This also deletes the account object.
	pAccountList->Delete (pAccount);
	pAccount = NULL;

	return TRUE;
}



// DeleteMailbox
// FUNCTION
// This is a convenience function for deleting a mailbox.
// END FUNCTION

// NOTES
// Note: The account ID MUST be given.
// END NOTES

BOOL CImapAccountMgr::DeleteMailbox (ACCOUNT_ID AccountId, LPCSTR pImapName, TCHAR Delimiter)
{
	BOOL bResult = FALSE;

	if (!(AccountId && pImapName))
		return FALSE;

	// Get the account object.
	CImapAccount *pAccount = FindAccount (AccountId);
	if (pAccount)
		bResult = pAccount->DeleteMailbox (pImapName, Delimiter);

	return bResult;
}




// MailboxExistsOnServer
// FUNCTION
// This is a convenience function for seeing if a mailbox exists on the server.
// END FUNCTION

// NOTES
// Note: The account ID MUST be given.
// END NOTES

BOOL CImapAccountMgr::MailboxExistsOnServer (ACCOUNT_ID AccountId, LPCSTR pImapName)
{
	BOOL bResult = FALSE;

	if (!(AccountId && pImapName))
		return FALSE;

	// Get the account object.
	CImapAccount *pAccount = FindAccount (AccountId);

	if (pAccount)
	{
		// Fetch the mailbox's attributes.
		ImapMailboxNode *pImapNode = pAccount->FetchMailboxAttributes (pImapName);

		if (pImapNode)
		{
			DeleteMailboxNode (pImapNode);

			bResult = TRUE;
		}
	}

	return bResult;
}



//////////////////////////////////////////////////////////
// GetTopImapDirectory [PUBLIC]
// Return the name of the top IMAP directory. 
// Top-level accounts are created under this directory.
// It is something like: <EudoraDir>/Imap.
// Return the string in Dir.
/////////////////////////////////////////////////////////
void CImapAccountMgr::GetTopImapDirectory (CString &Dir)
{
	Dir = EudoraDir + CRString (IDS_IMAP_TOP_DIRECTORY_NAME);
}


// 
// Make sure the IMAP top directory exists, creating it if necessary.
// Return TRUE if it already exists, or if we can create it.
//
BOOL CImapAccountMgr::CreateTopImapDirectory ()
{
	CString Dir = EudoraDir + CRString (IDS_IMAP_TOP_DIRECTORY_NAME);

	// If the dirname exists as a file, delete it.
	DWORD Attributes = GetFileAttributes (Dir);
	if ( (Attributes != 0xFFFFFFFF)	&&					// Exists
		 ! (Attributes & FILE_ATTRIBUTE_DIRECTORY) )	// Is a file.
	{
		DeleteLocalFile (Dir);
	}

	// If the directory doesn't exist, try now to create it.
	if ( !DirectoryExists (Dir) )
	{
		// Try to create.
		if (!CreateDirectory (Dir, NULL))
		{
			return FALSE;
		}
	}

	// If it doesn't exist now, we really can't create it.
	if (!DirectoryExists (Dir))
	{
		return FALSE;
	}
	else
		return TRUE;
}





// ClearAllPasswords [PUBLIC]
// FUNCTION
// Loop through all accounts a clear passwords.
// END FUNCTION

void CImapAccountMgr::ClearAllPasswords ()
{
	// Must be in main thread.
	ASSERT ( IsMainThreadMT() );

	int numLists = 0;

	// Get number of lists.
	numLists = GetSize();

	if (numLists > 0)
	{
		for (int listIndex = 0; listIndex < numLists; listIndex++)
		{
			CImapAccountList* pAccountList = GetAt (listIndex);
			if (pAccountList)
			{
				// Loop through accounts.
				int numAccounts = pAccountList->GetSize();
				for (int acctIndex = 0; acctIndex < numAccounts; acctIndex++)
				{
					CImapAccount* pAccount = pAccountList->GetAt (acctIndex);
					if (pAccount)
					{
						pAccount->InvalidatePasswordMT (TRUE);
					}
				}
			}
		}
	}
}




// ClearAllPasswords [PUBLIC]
// FUNCTION
// Loop through all accounts a clear passwords.
// END FUNCTION

void CImapAccountMgr::ForgetPassword (LPCTSTR pPersona)
{
	// Must be in main thread.
	ASSERT ( IsMainThreadMT() );

	CImapAccount* pAccount = FindByName (pPersona);

	if (pAccount)
	{
		pAccount->InvalidatePasswordMT (TRUE);
		//pAccount->SavePasswordToDisk ();
		CImapMailMgr::CloseImapConnections (pAccount->GetAccountID());
	}
}






// AccountsHavePasswords [PUBLIC]
// FUNCTION
// Return TRUe if there's at least 1 account witha a cached password.
// END FUNCTION

BOOL CImapAccountMgr::AccountsHavePasswords ()
{
	int numLists = 0;

	// Get number of lists.
	numLists = GetSize();

	if (numLists > 0)
	{
		for (int listIndex = 0; listIndex < numLists; listIndex++)
		{
			CImapAccountList* pAccountList = GetAt (listIndex);
			if (pAccountList)
			{
				// Loop through accounts.
				int numAccounts = pAccountList->GetSize();
				for (int acctIndex = 0; acctIndex < numAccounts; acctIndex++)
				{
					CImapAccount* pAccount = pAccountList->GetAt (acctIndex);
					if (pAccount)
					{
						CString szPw;
						pAccount->GetPasswordMT (szPw);

						if (!szPw.IsEmpty())
							return TRUE;
					}
				}
			}
		}
	}

	// If we get here:
	return FALSE;
}


//
// Get the password cached in the specified account object.
// The account is specified by the persona's name.
// Return it in szImapPassword.
//
BOOL CImapAccountMgr::GetPassword (LPCSTR pPersona, CString& szImapPassword)
{
	unsigned long AccountID = 0;

	// Set to blank in case of error.
	szImapPassword.Empty();

	// Sanity.
	if (!pPersona)
		return FALSE;

	AccountID = g_Personalities.GetHash( pPersona );

	if (AccountID == 0)
		return FALSE;

	CImapAccount *pAccount = FindAccount(AccountID);

	if (!pAccount)
		return FALSE;

	pAccount->GetPasswordMT (szImapPassword);

	return !szImapPassword.IsEmpty();
}



HRESULT CImapAccountMgr::FindAccount(ACCOUNT_ID AccountID, CImapAccount **ppAccount, AccountLockType sAccountLock)
{
	HRESULT			hResult = E_FAIL;

	// Sanity:
	//
	if (! (AccountID && ppAccount) )
		return NULL;

	// Try to lock the global list.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock with a timeout value.
	//
	if (!localLock.Lock(dwLockTimeout))
	{
		return E_FAIL;
	}

	// Ok. Find the account.
	//
	CImapAccount* pAccount = FindAccount (AccountID);

	if (pAccount)
	{
		// If pAccount, try to set our advisory lock on the object.
		//
		if ( !pAccount->SetAdvisoryLock (sAccountLock) )
		{
			pAccount = NULL;
		}
	}

	// Return data (could be NULL).
	*ppAccount = pAccount;

	// $$$$$$$$$$ Should return an HRESULT here indicating the type of error!!!

	return pAccount ? S_OK : E_FAIL;
}



// This updates an INI setting:
//
BOOL CImapAccountMgr::SetTopMboxesFetched ( unsigned long AccountID, BOOL bVal )
{
	CString key = g_Personalities.GetIniKeyName( IDS_INI_IMAP_MBOXES_FETCHED );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	if (bVal)
		g_Personalities.WriteProfileString( strPersona, key, "1" );
	else
		g_Personalities.WriteProfileString( strPersona, key, "0" );

	return TRUE;
}



// This updates an INI setting:
//
BOOL CImapAccountMgr::TopMboxesFetched ( unsigned long AccountID )
{
	CString key = g_Personalities.GetIniKeyName( IDS_INI_IMAP_MBOXES_FETCHED );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	return (g_Personalities.GetProfileInt (strPersona, key, 1) > 0);
}



// VerifyMboxesFetched [PRIVATE]
//
// Note: Set "bOnNewAccount" if this is called on the initial query when a new account has just been
// created. This causes a slightly different message to be displayed.
//
BOOL CImapAccountMgr::VerifyMboxesFetched ( unsigned long AccountID )
{
	ASSERT ( IsMainThreadMT() );

	// We have to check if the top-level account's mailbox list has been fetched. If not, inform user:
	//
	if ( ! TopMboxesFetched (AccountID ) )
	{
		CString szPersona;

		CImapAccount* pAccount = FindAccount ( AccountID );
		if (!pAccount)
		{
			return FALSE;
		}

		pAccount->GetName (szPersona);

		QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *) g_theMailboxDirector.ImapFindByName (
												NULL, 
												szPersona);
		if (!pImapCommand)
		{
			return FALSE;
		}

		// Change current persona.
		//
		CString	szOldPersonality =	g_Personalities.GetCurrent();
		g_Personalities.SetCurrent( szPersona );


		// OK, We may have to ask:
		//
		BOOL bWarn = GetIniShort(IDS_INI_WARN_FETCH_BEFORE_CHECK);

		BOOL bFetch = TRUE;

		if (bWarn)
		{
			// Allow user to MOM.

			bFetch = (YesNoDialog(IDS_INI_WARN_FETCH_BEFORE_CHECK,
								 IDS_INI_ALWAYS_FETCH_BEFORE_CHECK,
								 IDS_WARN_FETCH_BEFORE_CHECK,
								 szPersona ) == IDOK);

#if 0 

			bFetch = (YesNoDialogPlain(IDS_WARN_FETCH_BEFORE_CHECK,
								 szPersona ) == IDOK);
#endif

		}
		else
		{
			bFetch = GetIniShort (IDS_INI_ALWAYS_FETCH_BEFORE_CHECK);
		}

		// Go out and fetch?
		if (bFetch)
		{
			// Refresh complete child list. 
				//
			BOOL ViewNeedsRefresh = TRUE;

			pImapCommand->Execute (CA_IMAP_REFRESH, &ViewNeedsRefresh);

			g_theMailboxDirector.UpdateImapMailboxLists (pImapCommand, TRUE, TRUE);
		}

		// Reset:
		g_Personalities.SetCurrent( szOldPersonality );

	}

	return TRUE;
}





//=================================================================//



// Exported function to return the global account manager.
//

CImapAccountMgr* GetImapAccountMgr()
{
	return &g_ImapAccountMgr;
}




#endif // IMAP4

