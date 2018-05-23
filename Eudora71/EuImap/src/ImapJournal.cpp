// ImapJournal.cpp: implementation of the CImapJournaler class.
//
// Functions to add IMAP mailboxes to Mailboxes and Transfer menus.
// For WIN32, mailboxes are also added to the tree control.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <fcntl.h>

#include "cursor.h"
#include "progress.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "utils.h"
#include "header.h"
#include "mime.h"
#include "msgutils.h"
#include "guiutils.h"

#include "imap.h"
#include "ImapResyncer.h"
#include "imapjournal.h"
#include "imapgets.h"
#include "ImapMailbox.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapConnection.h"
#include "ImapMlistMgr.h"

#include "DebugNewHelpers.h"


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

	