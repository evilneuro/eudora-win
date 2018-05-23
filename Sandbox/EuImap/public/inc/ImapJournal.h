// ImapJournal.h - declaration of the CImapJournaler class. This manages the logging or local
// mailbox modifications into a database.
// and performs resync operations.
//

#ifndef __IMAPJOURNAL_H
#define __IMAPJOURNAL_H

#include "ImapExports.h"

#include "imap.h"


//======================================================================//

// Forward declarations
class  CIMAP;
class  CImapMailbox;

// CImapMailbox class.
class CImapJournaler
{
public:
	CImapJournaler ();

//Interface
public:
	// Set stuff.
	void	SetMailbox (CImapMailbox *pImapMailbox)
				{ m_pImapMailbox = pImapMailbox; }
	BOOL	InitializeDatabase (BOOL Reset);

// Add journal recorde.
	BOOL QueueRemoteXfer (IMAPUID sourceUID, ACCOUNT_ID DestAccountID, LPCSTR pDestImapMboxName, BOOL Copy);

// Internal functions:
// These may change if we ever use a dbm database to storing options so clients MUST
// not access these directly.
private:
	// Format pathname to the resync options file.
	BOOL	SetDatabaseFilePath ();


// Internal state:
private:
	CImapMailbox*	m_pImapMailbox;		// Referenced. Do not delete.

	CString			m_DBPathname;			// Pathname to the resync options database.
};



#endif // __IMAPJOURNAL_H

