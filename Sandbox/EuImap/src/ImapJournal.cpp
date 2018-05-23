// ImapJournal.cpp: implementation of the CImapJournaler class.
//
// Functions to add IMAP mailboxes to Mailboxes and Transfer menus.
// For WIN32, mailboxes are also added to the tree control.


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <fcntl.h>

#include "cursor.h"
#include "progress.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "usermenu.h"
#include "utils.h"
#include "header.h"
#include "mime.h"
#include "msgutils.h"
#include "guiutils.h"

#include "imap.h"
#include "imapresync.h"
#include "imapjournal.h"
#include "imapgets.h"
#include "imapfol.h"
#include "imapopt.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imaputil.h"
#include "imapmlst.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// =============== Internal function declarations ===============//


// ==============================================================//


CImapJournaler::CImapJournaler ()
{
	m_pImapMailbox = NULL;
	m_DBPathname.Empty();
}




// Initialize [PUBLIC]
// FUNCTION
// Create a file called <dirname>.opt
// for storing mailbox resync options for the owning mailbox.
// If "Reset" is TRUE, truncate it to zero bytes if it already existed,
// otherwise, return TRUE if the file already exists.
// END FUNCTION
// NOTES
// Return FALSE if we couldn't create the file if it didn't exist.
// END NOTES
BOOL CImapJournaler::InitializeDatabase (BOOL Reset)
{
	BOOL	bResult = FALSE;

	// Must be associated with a mailbox. 
	if (!m_pImapMailbox)
		return FALSE;

	if (!SetDatabaseFilePath ())
		return FALSE;

	// If it exists, OK
	if (FileExistsMT (m_DBPathname))
	{
		if (Reset)
		{
			bResult = ChangeFileSize (m_DBPathname, 0);
		}
		else
			bResult = TRUE;
	}
	else
	{
		bResult = CreateLocalFile (m_DBPathname, TRUE);
	}

	return bResult;
}



// SetResyncOptFilePath [PRIVATE]
// FUNCTION
// Given the mailbox directory (m_pMailboxDir), create a full pathname to
// the file containing the resync options for this mailbox. Set the pathname into m_Pathname.
// END FUNCTION

BOOL CImapJournaler::SetDatabaseFilePath ()
{
	int		length, i;
	CString MailboxDir;

	if (!m_pImapMailbox)
		return FALSE;

	MailboxDir = m_pImapMailbox->GetDirname();
	if (MailboxDir.IsEmpty())
		return FALSE;

	// MailboxDir MUST have at least a directory delimiter AND another char in the name.
	// i.e., at least "\a".
	length = MailboxDir.GetLength();
	if (length < 2)
		return FALSE;

	// Remove any trailing delimiter.
	if (MailboxDir[length - 1] == DirectoryDelimiter)
	{
		MailboxDir = MailboxDir.Left (length - 1);
		length--;
	}

	// Add the last name.
	i = MailboxDir.ReverseFind (DirectoryDelimiter);
	if ((i >= 0) && (i < (length - 1)) )
	{
		m_DBPathname = MailboxDir + DirectoryDelimiter + MailboxDir.Mid (i + 1) + CRString(IDS_IMAP_JRNL_FILE_EXTENSION);

		return TRUE;
	}

	return FALSE;
}


// Add an XFER record.
BOOL CImapJournaler::QueueRemoteXfer (IMAPUID sourceUID, ACCOUNT_ID DestAccountID, LPCSTR pDestImapMboxName, BOOL Copy)
{

	return TRUE;
}


#endif // IMAP4

	