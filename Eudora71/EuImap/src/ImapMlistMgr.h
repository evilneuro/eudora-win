#ifndef __IMAPMLSTMGR_H__
#define __IMAPMLSTMGR_H__

// ImapMlstMgr.h : Interface file for the CImapMlistMgr class.
//

// Forward declarations.


// Some generic defs.
#include "ImapMailbox.h"


// A CImapMlistMgr object handles tasks such as updating mailboxes.lst files from 
// the IMAP server and creating stub mailboxes for newly subscribed mailboxes.
// It will call the mailbox browser to allow a user to select interesting mailboxes.
// Also handles creating a new mailbox as a user command. 

class CImapMlistMgr
{
public:
	// A CImapMlistMgr object manages mailboxes.lst files in the given "pPathname" and
	// its subdirectories.
	// It fetches the names of imap mailboxes that a children of the given "pParentImapname"
	// mailbox within the account specified by "AccountID".
	// "Delimiter" is the mailbox delimiter. It can be '0', in which case, we don't
	// know what it really is.

	CImapMlistMgr(ACCOUNT_ID AccountID, LPCSTR pPathname, LPCSTR pParentImapname,
					TCHAR Delimiter, ImapMailboxType Type);
   ~CImapMlistMgr();

// Interface
public:
	// Fetch mailbox lists. If "ALL" is TRUE, recurse through child lists.
	BOOL	UpdateMailboxLists (BOOL Recurse);

// Public Inlines
	BOOL	NeedsResync ()			{ return m_bNeedsResync; }


// Internal methods.
private:
	// Do the real work of updating the lists.
	BOOL __UpdateMailboxListFiles(ImapMailboxNode *pTopNode, LPCSTR pPathname);

	

// Internal Attributes.	
private:
	ACCOUNT_ID		m_AccountID;
	CString			m_Pathname;			// Local path.
	CString			m_ParentImapname;	// ~/Mail/, user., etc.
	TCHAR 			m_Delimiter;		// Character Delimiter.
	ImapMailboxType m_Type;
	BOOL			m_bNeedsResync;		// Set to TRUE if, after an update, the mailboxes
										// themselves should go through a re-sync procedure.
};


//==================== Exported functions. ====================/
TCHAR ImapMailboxTypeToTypeChar (ImapMailboxType Type);
BOOL MakeSuitableMailboxName (LPCSTR pParentDir, ImapMailboxNode *pTopnode, ImapMailboxNode *pCurNode, CString &NewName, short MaxNameLength);
/////////////////////////////////////////////////////////////////////////////
#endif // __IMAPMLSTMGR_H__
