// imapfol.cpp: implementation of the CImapMailbox class.
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

#include "rs.h"
#include "address.h"
#include "summary.h"
#include "doc.h"
#include "fileutil.h"
#include "resource.h"
#include "tocdoc.h"
#include "tocview.h"
#include "tocframe.h"
#include "msgdoc.h"
#include "cursor.h"
#include "progress.h"
#include "font.h"
#include "station.h"
#include "utils.h"
#include "header.h"
#include "mime.h"
#include "msgutils.h"
#include "guiutils.h"
#include "filtersd.h"
#include "persona.h"

#include "QCWorkerThreadMT.h"

#ifdef WIN32
#include "mainfrm.h"
#endif

#include "eudora.h"

#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"

#include "imap.h"
#include "ImapTypes.h"
#include "ImapSum.h"
#include "ImapSettings.h"
#include "imapgets.h"
#include "imapjournal.h"
#include "ImapResyncer.h"
#include "ImapMailbox.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapMlistMgr.h"
#include "ImapAppend.h"
#include "imapmime.h"
#include "pop.h"
#include "ImapDownload.h"
#include "ImapFiltersd.h"
#include "ImapMailMgr.h"
#include "ImapChecker.h"
#include "ImapConnectionMgr.h"
#include "SearchCriteria.h"
#include "searchutil.h"

#include "StatMng.h"

#include "DebugNewHelpers.h"

// We need to extern this - yuck!!
extern QCMailboxDirector	g_theMailboxDirector;

#ifdef EXPIRING
	// The Evaluation Time Stamp object
	#include "timestmp.h"
	extern CTimeStamp	g_TimeStamp;
#endif

// =============== Internal function declarations ===============//
BOOL DoingMinimalDownload (CImapMailbox *pImapMailbox);
BOOL CopyTextFromTmpMbx (CTocDoc* pTocDoc, CSummary* pSum);
void TerminateProgress (BOOL bWePutProgressUp, BOOL bWasInProgress);
LPCSTR GetNextUidChunk (LPCSTR pStart, CString& szChunk);
TCHAR FigureOutDelimiter(CImapAccount *pAccount);

// ===============================================================//

// Internal data

// constants:
const unsigned long ImapMboxVersion		= 0x0100;	// Version 1.00.
const long	InfMboxPathSize				= 256;
const long	InfColumnSize				= 48;
const long	InfImapNameSize				= 256;
const long	InfDelimiterFieldSize		= 4;
const char	comma						= ',';
const int   MAX_UIDS_PER_COMMAND		= 60;		// backed down from 100 to cooperate with worldmail -jdboyd


// char *pImapShortHeaderFields		= "Date,Subject,From,Sender,Reply-to,To,Priority, Content-Type, Mime-Version";

char *pImapShortHeaderFields		= "Date,Subject,Reply-to,From,X-Priority,Importance,Content-Type";
//char *pImapShortHeaderFields		= "Date,Subject,Reply-to,From,X-Priority,Importance,Content-Type,Mime-Version,Content-Disposition";
//We will probably never need the Mime-Version and/or Content-Disposition . This was added fot the plugin sake.

// Macros
#define		PARTIAL			0x8000
#define		HAS_UNREAD		0x4000
#define		NO_INFERIORS	0x2000
#define		NO_SELECT		0x1000
#define		MARKED			0x0800
#define		UNMARKED		0x0400

// Local buffer size.
#ifdef MAXBUF
#undef MAXBUF
#endif
#define MAXBUF	2048

// Re-entrancy class to prevent multiple attempts at logging into the same server.
// Note: Al this is done in the main thread so we don't need to a synchronization
// object. However, we may have re-entrancy problems so we need to guard 
// against this.
//
class CLoginLock
{
public:
	CLoginLock(CImapMailbox* pImapMailbox);
	~CLoginLock();

	BOOL WeGotTheLock()
		{ return m_bWeGotTheLock; }

private:
	CImapMailbox*	m_pImapMailbox;
	BOOL			m_bWeGotTheLock;
};

CLoginLock::CLoginLock(CImapMailbox* pImapMailbox) : m_pImapMailbox (pImapMailbox)
{
	m_bWeGotTheLock		= FALSE;
//	m_PrevPreviewState	= TRUE;

	if (pImapMailbox)
	{

#if 0 // Not necessary.
		// Disable (another?) preview while we're doing this:
		//
		m_PrevPreviewState = pImapMailbox->PreviewAllowed();

		pImapMailbox->SetPreviewAllowed (FALSE);
#endif // 

		if ( !pImapMailbox->IsLoginBusy() )
		{
			pImapMailbox->SetLoginBusy(TRUE);

			m_bWeGotTheLock = TRUE;
		}
	}
}

CLoginLock::~CLoginLock()
{
	if (m_pImapMailbox)
	{
#if 0 // Not necessary
		m_pImapMailbox->SetPreviewAllowed (m_PrevPreviewState);
#endif

		if (m_bWeGotTheLock)
		{
			m_pImapMailbox->SetLoginBusy(FALSE);
		}

	}
}




//=================================================================================//
//============================== CImapMailbox =====================================//

CImapMailbox::CImapMailbox() :
	m_AccountID(0),
	m_pImap(NULL),
	m_pTocDoc(NULL)
{
	InitAttributes();
}




CImapMailbox::CImapMailbox(ACCOUNT_ID AccountID) :
	m_AccountID(AccountID),
	m_pImap(NULL),
	m_pTocDoc(NULL)
{
	InitAttributes();

	// Grab settings for this account:
	GrabSettings();
}



// InitAttributes [PRIVATE]
//
// Single method called from both constructors to initialize attributes. This way,
// we don't have to duplicate.
//
void CImapMailbox::InitAttributes()
{
	m_Delimiter	= 0;					// For this folder hierarchy.
	m_bHasUnread	= FALSE;			// If has unread mesages.
	m_bPartial		= FALSE;
	m_bNoInferiors	= TRUE;				// If can never contain other folders.
	m_bNoSelect		= FALSE;			// If cannot be selected.
	m_bMarked		= FALSE;
	m_bUnMarked		= FALSE;

	// Don't know mailbox type just yet.
	//
	m_Type			= IMAP_MAILBOX;	// Default.

	m_Uidvalidity		= 0;
	m_NumberOfMessages	= 0;
	m_UIDHighest	= 0;

	// Set the context of the contained journal and resyncer classes.
	m_Journaler.SetMailbox(this);
	m_Resyncer.SetMailbox(this);

	m_bModified		= FALSE;

	m_bNeedsUpdateFromTemp = FALSE;

	m_bReadOnly = FALSE;	

	m_bExpunged		= FALSE;

	m_bJustCheckedMail = FALSE;

	// Allow preview unless we say no.
	//
	m_bPreviewAllowed	= TRUE;

	m_bAlreadyCheckingMail = FALSE;

	m_bForceResync = FALSE;

	m_bTocWasJustRebuilt = FALSE;

	m_bUidValidityChanged = FALSE;

	m_pImapSettings = NULL;

	//
	m_bLoginBusy			= FALSE;

	m_pTocDoc = NULL;
}





// The CImapConnection will close the stream.
// Important: Don't call "Close" at this point!!
//
CImapMailbox::~CImapMailbox ()
{
	// Close the CImapConnection if there's one.
	//
	if (m_pImap)
	{
		Close();

		m_pImap = NULL;
	}

	if (m_pImapSettings)
		delete m_pImapSettings;
}


// ResetAttributes [PUBLIC]
// FUNCTION
// Reset all attributes to what they would be at instantiation time.
// NOTE: Leave the account context intact.
// END FUNCTION
void CImapMailbox::ResetAttributes()
{
	m_Type = IMAP_MAILBOX;				// Default.
	m_Uidvalidity = 0;					// Invalid.
	m_NumberOfMessages = 0;
	m_UIDHighest = 0;

	m_ImapName.Empty();
	m_Dirname.Empty();

	// m_AccountID	= 0;		// Leave account ID as is.

	m_Delimiter	= 0;					// For this folder hierarchy.
	m_bHasUnread	= FALSE;			// If has unread mesages.
	m_bPartial		= FALSE;
	m_bNoInferiors	= TRUE;				// If can never contain other folders.
	m_bNoSelect		= FALSE;			// If cannot be selected.
	m_bMarked		= FALSE;
	m_bUnMarked		= FALSE;

	// Set the context of the contained journal and resyncer classes.
	m_Journaler.SetMailbox(this);
	m_Resyncer.SetMailbox(this);

	m_bModified		= FALSE;

	m_bNeedsUpdateFromTemp = FALSE;

	m_bReadOnly = FALSE;	

	m_bUidValidityChanged = FALSE;

	// Leave the contained CImapConnection's attributes as-is.
}



// Copy [public]
// FUNCTION
// Copy stuff from the given CImapMailbox.
// END FUNCTION
void CImapMailbox::Copy (CImapMailbox *pImapMailbox)
{
	if (!pImapMailbox)
		return;

	m_Uidvalidity		= pImapMailbox->GetUidvalidity ();

	m_NumberOfMessages	= pImapMailbox->GetNumberOfMessages ();

	m_UIDHighest		= pImapMailbox->GetUIDHighest ();

	SetImapType ( pImapMailbox->GetImapType() );

	SetImapName ( pImapMailbox->GetImapName() );

	SetDirname ( pImapMailbox->GetDirname() );

	SetAccountID ( pImapMailbox->GetAccountID() );

	SetDelimiter ( pImapMailbox->GetDelimiter() );

	SetHasUnread ( pImapMailbox->HasUnread() );

	SetNoInferiors ( pImapMailbox->IsNoInferiors() );

	SetNoSelect ( pImapMailbox->IsNoSelect() );

	SetMarked ( pImapMailbox->IsMarked() );

	SetUnMarked ( pImapMailbox->IsUnMarked() );

	SetAutoSync ( pImapMailbox->IsAutoSync() );

	SetModifiedFlag ( pImapMailbox->IsModified() );

	m_bUidValidityChanged = pImapMailbox->m_bUidValidityChanged;
}




// InitializeLocalStorage
// FUNCTION
// Make sure the mailbox directory exists and contains at least stubs of the necessary files.
// If "Clean", delete messages from the MBX file, etc.
// END FUNCTION
BOOL CImapMailbox::InitializeLocalStorage(BOOL Clean)
{
	// Create the directory housing the mailbox.
	if ( CreateMailboxDirectory (GetDirname()) )
	{
		// Create the MBX file if it doesn't exist.
		CreateMbxFile (GetDirname());

		// Create the attachment directory.
		CreateAttachDir (GetDirname());

		// Write imap info.
		WriteImapInfo (Clean == TRUE);

		// Initialize the journal database.
		m_Journaler.SetMailbox(this);
		m_Journaler.InitializeDatabase (TRUE);

		// And the resyncer too.
		m_Resyncer.SetMailbox(this);
		m_Resyncer.InitializeDatabase (TRUE);

		return TRUE;
	}

	return FALSE;
}



// 
// FUNCTION
// Reads the IMAP mailbox's info file and fill in ImapName, UIDVALIDITY, etc.
// END FUNCTION
// NOTES
// m_AccountID and m_Dirname MUST have already been set.
// END NOTES
BOOL CImapMailbox::ReadImapInfo ()
{
	JJFileMT		in;
	CString		InfFilePath;
	unsigned long ulValue;
	char		buf[1024];
	BOOL		bResult = FALSE;

	if (m_AccountID == 0 || m_Dirname.IsEmpty())
		return FALSE;

	// Format the info filename:
	GetInfFilePath (m_Dirname, InfFilePath);

	// Make sure the file exists.
	if ( !SUCCEEDED ( in.Open(InfFilePath, O_RDONLY) ) )
		return FALSE;
	
	// Get the version.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;
	// 	Make sure it's correct.
	if (ulValue != ImapMboxVersion) goto end;

	// infModTime (4 bytes) - leave blank.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// Full path to local mailbox directory (256 bytes)
	// BUG: Don't use this info here because m_Dirname is alread set.
	if ( !SUCCEEDED (in.Read(buf, InfMboxPathSize) ) ) goto end;

	// m_Dirname = buf;

	// infFlags ( 4 bytes)
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// Set object attributes.
	m_bHasUnread	= (ulValue & HAS_UNREAD) > 0;
	m_bPartial		= (ulValue & PARTIAL) > 0;
	m_bNoInferiors	= (ulValue & NO_INFERIORS) > 0;
	m_bNoSelect		= (ulValue & NO_SELECT) > 0;
	m_bMarked		= (ulValue & MARKED) > 0;
	m_bUnMarked		= (ulValue & UNMARKED) > 0;

	// infColumns (48 bytes) - ignore;
	if ( !SUCCEEDED (in.Read(buf, InfColumnSize) ) ) goto end;

	// infType2SelColumn ( 4 bytes) - ignore.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// infLastOpened 4 bytes) - ignore.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// UIDVALIDITY (4 bytes);
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;
	m_Uidvalidity = ulValue;

	// infNumberOfMessages 4 bytes.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;
	m_NumberOfMessages = ulValue;

	// infUIDHighest 4 bytes.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;
	m_UIDHighest = ulValue;

	// infI4Server 4 bytes - ignore.
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// Mailbox ID 4 bytes - ignore
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// infI4MailboxName, 256 bytes.
	if (! SUCCEEDED (in.Read(buf, InfImapNameSize) ) ) goto end;
	m_ImapName = buf;

	// infLastResynced 4 bytes - ignore
	if (!SUCCEEDED ( in.Get(&ulValue) ) ) goto end;

	// Delimiter char 4 bytes.
	if (! SUCCEEDED (in.Read(buf, InfDelimiterFieldSize) ) ) goto end;
 	m_Delimiter = buf[0];

	SetModifiedFlag(FALSE);

	bResult = TRUE;

end:
	in.Close();
	return bResult;
}



// 
// FUNCTION
// Writes the IMAP mailbox's info to the info file
// If Overwrite is FALSE and the file exists, leave it as is.
// END FUNCTION
// NOTES
// Must at least have a valid account ID and mailbox dir, AND the mailbox must exist.
// END NOTES
BOOL CImapMailbox::WriteImapInfo (BOOL Overwrite)
{
	JJFileMT		out;
	CString		InfFilePath;
	unsigned long ulValue;
	char		buf[1024];
	const char	*p;

	if (m_AccountID == 0 || m_Dirname.IsEmpty())
		return FALSE;

	// Format the info filename:
	GetInfFilePath (m_Dirname, InfFilePath);

	if (FileExistsMT(InfFilePath) && !Overwrite)
		return TRUE;

	// Try to open:
	if ( !SUCCEEDED (out.Open(InfFilePath, O_CREAT | O_TRUNC | O_WRONLY) ) )
		return (FALSE);

	// Write version (4 bytes)
	ulValue = ImapMboxVersion;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// infModTime (4 bytes) - leave blank.
	ulValue = 0;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// Full path to local mailbox directory (256 bytes)
	memset(buf, 0, InfMboxPathSize);
	p = GetDirname();
	if (p)
		strcpy(buf, p);
	if (!SUCCEEDED (out.Put(buf, InfMboxPathSize) ) ) return (FALSE);

	// infFlags ( 4 bytes)
	ulValue = 0;
	if (m_bHasUnread)
		ulValue |= HAS_UNREAD;
	if (m_bPartial)
		ulValue |= PARTIAL;
	if (m_bNoInferiors)
		ulValue |= NO_INFERIORS;
	if (m_bNoSelect)
		ulValue |= NO_SELECT;
	if (m_bMarked)
		ulValue |= MARKED;
	if (m_bUnMarked)
		ulValue |= UNMARKED;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);


	// infColumns (48 bytes) - ignore;
	memset(buf, 0, InfColumnSize);
	if (!SUCCEEDED (out.Put(buf, InfColumnSize) ) ) return (FALSE);

	// infType2SelColumn ( 4 bytes) - ignore.
	ulValue = 0;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// infLastOpened 4 bytes) - ignore.
	ulValue = 0;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// UIDVALIDITY (4 bytes);
	ulValue = m_Uidvalidity;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// infNumberOfMessages 4 bytes.
	ulValue = m_NumberOfMessages;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// infUIDHighest 4 bytes.
	ulValue = m_UIDHighest;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// infI4Server 4 bytes
	ulValue = 0;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// Mailbox ID 4 bytes
	ulValue = 0;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// infI4MailboxName, 256 bytes.
	memset(buf, 0, InfImapNameSize);
	p = GetImapName();
	if (p)
		strcpy(buf, GetImapName());
	if (!SUCCEEDED (out.Put(buf, InfImapNameSize) ) ) return (FALSE);

	// infLastResynced 4 bytes.
	ulValue = 0;
	if ( !SUCCEEDED (out.Put(ulValue) ) ) return (FALSE);

	// Delimiter char 4 bytes.
	memset (buf, 0, InfDelimiterFieldSize);
	buf[0] = m_Delimiter;
	if (SUCCEEDED (out.Put(buf, InfDelimiterFieldSize) ) ) return (FALSE);

	SetModifiedFlag(FALSE);

	return (TRUE);
}


// GetSettingsShort [PUBLIC]
//
// Interface to internal settings object.
//
short CImapMailbox::GetSettingsShort (UINT StringNum)
{
	if (!m_pImapSettings)
		return 0;

	return m_pImapSettings->GetSettingsShort (StringNum);
}



// GetSettingsLong [PUBLIC]
//
// Interface to internal settings object.
//
long CImapMailbox::GetSettingsLong (UINT StringNum)
{
	if (!m_pImapSettings)
		return 0;

	return m_pImapSettings->GetSettingsLong (StringNum);
}



// GetSettingsString [PUBLIC]
//
// Interface to internal settings object.
// Note: Returns a const pointer.
//
LPCSTR	CImapMailbox::GetSettingsString (UINT StringNum, char *Buffer /* = NULL */, int len /* = -1 */)
{
	if (!m_pImapSettings)
		return NULL;

	return m_pImapSettings->GetSettingsString (StringNum, Buffer, len);
}





// GetLogin [PUBLIC]
//
// Does the initial setup before we attempt to open the mailbox.
// This MUST be performed in the main thread.
//
HRESULT CImapMailbox::GetLogin ()
{
	HRESULT	hResult = S_OK;

	// This MUST be called in the main thread!!
	//
	ASSERT ( ::IsMainThreadMT() );

	// Don't allow this if eudora isn't fully initialized:
	//
	if ( !CImapMailMgr::AppIsInitialized () )
	{
		return E_FAIL;
	}

	// Check re-entrancy flag:
	//
	CLoginLock lock (this); 
	if ( !lock.WeGotTheLock() )
	{
		return HRESULT_MAKE_BUSY;
	}

#ifdef EXPIRING
	// this is the first line of defense
	if ( g_TimeStamp.IsExpired0() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);

		return HRESULT_MAKE_CANCEL;
	}
#endif

	// Get info from the ACCOUNT
	if (m_AccountID == 0)
		return E_FAIL;

	// Set the friendly name:.
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );
	if (!pImapCommand)
	{
		ASSERT (0);
		return E_FAIL;
	}

	m_FriendlyName = pImapCommand->GetName();

	// If m_pImap is NULL, we must create or acquire one.
	//
	if (!m_pImap)
	{
		// Note: this lock the connection so we're the only one that can use it.
		//
		hResult = AcquireNetworkConnection ();
	}

	if (!m_pImap)
	{
		hResult = E_FAIL;
		ASSERT (0);

		return E_FAIL;
	}

	// If already selected, no need to continue.
	//
	if ( m_pImap->IsSelected() )
		return S_OK;

	// If the connection object is too busy to deal with our request, we must fail.
	//
	if (m_pImap->IsTooBusy())
	{
		return E_FAIL;
	}

	// If the connection is not conencted, we need to go get new password/login.
	//
	// If the CImapConnection is not connected, we may need to get (possibly)
	// new login and password. 
	//
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
		return E_FAIL;

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !pAccount->Login (szLogin, szPassword) )
	{
		return HRESULT_MAKE_CANCEL;
	}

	m_pImap->SetLogin	 (szLogin);
	m_pImap->SetPassword (szPassword);

	// Success. Note: we haven't logged into the IMAP server! We've only done
	// some main-thread initializations.
	//
	return S_OK;
}






// OpenMailbox [PUBLIC]
// NOTES
// Sometimes we want to open the connection to the server mailbox without
// updating it.
// It "bSilent" is TRUE, don't put up any progress stuff.
// END NOTES
HRESULT CImapMailbox::OpenMailbox (BOOL bSilent /* = FALSE */)
{
	HRESULT	hResult = S_OK;

	if ( ::IsMainThreadMT() )
	{
		// Did user cancel login??
		//
		if ( !SUCCEEDED (GetLogin()) )
			return HRESULT_MAKE_CANCEL;
	}

	// Check re-entrancy flag:
	//
	CLoginLock lock (this); 
	if ( !lock.WeGotTheLock() )
	{
		return HRESULT_MAKE_BUSY;
	}

	// Must have one of these:
	//
	if (!m_pImap)
	{
		hResult = E_FAIL;
		ASSERT (0);

		return E_FAIL;
	}

	// If already selected, no need to continue.
	//
	if ( m_pImap->IsSelected() )
		return S_OK;

	// If the connection object is too busy to deal with our request, we must fail.
	//
	if (m_pImap->IsTooBusy())
	{
		return E_FAIL;
	}

	// We need our account below.
	//
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
		return E_FAIL;

	// The following can be performed in a background thread.
	//

	// We can put up a progress window here!
	//
	BOOL	bWasInProgress = FALSE;

	if ( ::IsMainThreadMT() && !bSilent )
	{
		CString buf;

		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		buf.Format (CRString(IDS_IMAP_OPENING_MAILBOX), m_FriendlyName);
		MainProgress(buf);
	}


	// Open the connection in the main thread. If the connection is already open,
	// CImapConnection::OpenMailbox is smart enough not to re-open it.
	//
	hResult = m_pImap->OpenMailbox (m_ImapName);


	// If we're doing this in the main thread, update saved info here.
	//
	if ( ::IsMainThreadMT() )
	{
		if ( SUCCEEDED (hResult) )
			WriteImapInfo (TRUE);

		// Close progress window if we had one. Close it before we put up 
		// any error message.
		//
		if ( !bSilent)
		{	
			if (bWasInProgress)
				PopProgress ();
			else
				CloseProgress();
		}
	}

	return hResult;
}






// FUNCTION
// CLose the connection to the IMAP server.
// This calls Release on the connection object.
//
// NOTE: After this, the m_pImap object is no longer valid. We
// MUST call AcquireNetworkConnection() again if we want another connection.
//.
// END FUNCTION

HRESULT CImapMailbox::Close()
{
	HRESULT hResult = S_OK;

	// NOTE: Don't update the imap info file at this time!!
	//
	if (m_pImap)
	{
		// We no longer own the connection.
		//
		hResult = GetImapConnectionMgr()->Release (m_pImap);

		m_pImap = NULL;
	}

	return hResult;
}





/////////////////////////////////////////////////////////////////////////
// OpenOnDisplay [PUBLIC]
// 
// This method is called only when the mailbox is first opened!!!
// See CTocDoc::Display()
/////////////////////////////////////////////////////////////////////////

HRESULT CImapMailbox::OpenOnDisplay (CTocDoc *pTocDoc)
{
	HRESULT hResult = S_OK;

	// Sanity:
	if (!pTocDoc)
		return E_INVALIDARG;

	//
	// Set the frame's name to <name (<Account>)>
	//
	SetFrameName (pTocDoc);

	// Put up cursor while we're doing this.
	//
	CCursor cur;

	// If the application hasn't yet initialized, don't continue.
	//
	if ( !CImapMailMgr::AppIsInitialized() )
	{
		return S_OK;
	}		

	// If we're offline, don't initiate a connection.
	//
	if ( !IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		return HRESULT_MAKE_OFFLINE;
	}

	// Don't allow preview at this time.
	//
	SetPreviewAllowed (FALSE);


#if 0 // 
	// Open the connection before we go any further.
	//
	hResult = OpenMailbox (FALSE);
#endif

	// Don't open the mailbox here. Just do the initializations.
	//
	hResult = GetLogin();

	if ( !SUCCEEDED (hResult) )
	{
		SetPreviewAllowed (TRUE);
		
		// Attempt to do a preview in case the summary is already downloaded.
		pTocDoc->InvalidateCachedPreviewSummary(NULL, true);

		return hResult;
	}

	if (::EscapePressed())
	{
		SetPreviewAllowed (TRUE);
		return HRESULT_MAKE_CANCEL;
	}

	bool		 bCheckMail = true;
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount(m_AccountID);
	if (pAccount)
	{
		bCheckMail = !pAccount->ActionsQueuedForMailbox(this);
	}

	if (bCheckMail)
	{
		// If we didn't just do a checkmail, do one as we open.
		if ( !JustCheckedMail () )
		{
			// Don't allow re-entrant check mail on the same mailbox.
			//
			SetJustCheckedMail (TRUE);

			// Set a time to do the check mail.
			//

			// If this is Inbox, set the ForceResync instance flag and
			// call the global GetMail routine out in pop.cpp.
			//
			CString szName = GetImapName ();
			szName.TrimLeft (); szName.TrimRight ();

			if ( IsInbox (szName) )
			{
				m_bForceResync = TRUE;

				// Get this personality's name.
				//
				CString szPersona; pAccount->GetName (szPersona);

				LPSTR pszPersonaName = DEBUG_NEW_NOTHROW char[ szPersona.GetLength() + 4 ];

				if (pszPersonaName)
				{
					// Note: Must terminate this string with double nuls.
					strcpy (pszPersonaName, (LPCSTR)szPersona);
					pszPersonaName[szPersona.GetLength() + 1] = '\0';

					GetMail (kManualMailCheckBits, pszPersonaName, FALSE/*bFullMailCheck*/);

					// Unset this:
					m_bForceResync = FALSE;

					hResult = S_OK;

					// Free it:
					delete[] pszPersonaName;
				}
			}
			else
			{
				//
				// Just force a resync:
				//
//				hResult = CheckMail ( pTocDoc, FALSE, TRUE);

//				hResult = ResyncMailbox (pTocDoc, FALSE, 0xFFFFFFFF, FALSE);

				hResult = GetImapMailMgr()->DoManualResync (pTocDoc,
														FALSE,		// BOOL bCheckMail /* = FALSE */,
														FALSE,		// BOOL bDownloadedOnly /* = TRUE */,
														TRUE);		// BOOL bInBackground /* = FALSE */)

			}
		}
	}
			
	// Reset so we catch it next time.
	SetJustCheckedMail (FALSE);

	// Make sure!!
	//
	SetPreviewAllowed (TRUE);

	pTocDoc->InvalidateCachedPreviewSummary(NULL, true);

	return hResult;
}




// DoManualResync [PUBLIC]
//
// External interface to a manual resync. operation.
//
// Resync what we currently have then do a check mail.
//
// HISTORY:
// JOK - 1/21/98. Added "bCheckMail" parameter.
//
HRESULT CImapMailbox::DoManualResync (CTocDoc *pTocDoc,
								       BOOL bCheckMail /* = FALSE */,
									   BOOL bDownloadedOnly /* = TRUE */,
									   BOOL bInForeGround /* = FALSE */,
									   BOOL bDoSubMailboxes /* = FALSE */)
{

	HRESULT hResult;


	if (bInForeGround)
	{
		// OFFLINE$$$
		// if "Offline mode is set", must ASK if to attempt connection.
		//


		//
		hResult = DoForegroundResync (pTocDoc, bDownloadedOnly);
	}
	else
	{
		hResult = DoBackgroundResync (pTocDoc, bCheckMail, bDownloadedOnly, bDoSubMailboxes);
	}

	// This may have been disabled during startup:
	//
	SetPreviewAllowed (TRUE);

	pTocDoc->InvalidateCachedPreviewSummary(NULL, true);

	// If still in offline mode, clse connection behind us.
	//
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE) )
		Close();
	
	return hResult;
}




///////////////////////////////////////////////////////////////
//  CheckNewMail [PUBLIC]
//
// External interface to checking for new mail.
///////////////////////////////////////////////////////////////
HRESULT CImapMailbox::CheckNewMail (CTocDoc *pTocDoc, BOOL bSilent /* = FALSE */)
{
	HRESULT hResult = S_OK;
	BOOL bWeOpenedConnection = FALSE; 

	// Sanity:
	if (!pTocDoc)
		return E_INVALIDARG;

	// Open the connection if the connection is not already open,
	// If we dropped the connection for whatever reason, try to re-connect.
	// Note: Keep tab of if we opened the connection because we may have to 
	// close it if there's no view.
	//
	if (!IsSelected())
	{
		hResult = OpenMailbox (FALSE);

		if (! SUCCEEDED (hResult) )
		{
			return hResult;
		}

		// Flag the we opened the connection.
		//
		bWeOpenedConnection = TRUE;
	}

	// Call the internal routine. Note" Don't do a re-sync!!
	hResult = CheckMail ( pTocDoc, bSilent, FALSE);

	// If no window and we opened the connection, close it.
	//
	CTocView *pView = pTocDoc->GetView ();
	
	// If we're offline, close the connection again.
	//
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		Close();
	}
	else if (bWeOpenedConnection && !pView)
	{
		Close();
	}

	// Redraw the complete TOC:
	//
	if (pView)
		pView->Invalidate();
			
	return hResult;
}






// DownloadSingleMessage [PUBLIC]
//
// External interface to downloading a new message.
// Accepts a CSummary object. Since we deal only with CImapSum objects internally,
// we need to instantiate a new CImapSum object, copy info from the CSummary into
// it, do the download, then copy back stuff that may have changed.
//
HRESULT CImapMailbox::DownloadSingleMessage (CTocDoc *pTocDoc, CSummary *pSum,
											 BOOL bDownloadAttachments, BOOL bToTmpMbx /* = FALSE */)
{
	HRESULT hResult = S_OK;

	// Sanity:
	if (! (pTocDoc && pSum) )
		return E_INVALIDARG;

	// If we're offline and not connected, don't continue.
	//

//	if ( !IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
//	{
//		return HRESULT_MAKE_OFFLINE;
//	}

	// INstantiate a CImapSum and copy.
	//
	CImapSum *pImapSum = DEBUG_NEW_MFCOBJ_NOTHROW CImapSum;

	if (!pImapSum)
		return E_FAIL;

	// Copy stuff:
	//
	pImapSum->CopyFromCSummary (pSum);

	// DO this manually:
	//
	pImapSum->m_SummaryDate = pSum->m_Date;

	// Do the download:
	//
	hResult = DownloadSingleMessage (pTocDoc, pImapSum, bDownloadAttachments, bToTmpMbx);

	if ( SUCCEEDED (hResult) )
	{
		// If we had a date in the original summary, preserve that.
		//
		long Seconds = pSum->m_Seconds;

		pImapSum->CopyToCSummary (pSum);

		if (Seconds)
			pSum->m_Seconds = Seconds;
	}

	// No longer need pImapSum.
	delete pImapSum;

	return hResult;
}




// DoForegroundResync [PRIVATE]
// 
// Force a full resync of the mailbox, and do it in the foreground.
// If it's INBOX, don't do any filtering.
//
HRESULT CImapMailbox::DoForegroundResync (CTocDoc *pTocDoc,
									   BOOL bDownloadedOnly /* = TRUE */)
{

	HRESULT hResult = S_OK;
	BOOL bWeOpenedConnection = FALSE; 

	// Sanity:
	if (!pTocDoc)
		return E_INVALIDARG;

	// Open the connection if the connection is not already open,
	// If we dropped the connection for whatever reason, try to re-connect.
	// Note: Keep tab of if we opened the connection because we may have to 
	// close it if there's no view.
	//
	if (!IsSelected())
	{
		hResult = OpenMailbox (FALSE);

		if (!SUCCEEDED (hResult) )
		{
			return hResult;
		}

		// Flag the we opened the connection.
		//
		bWeOpenedConnection = TRUE;
	}

	// If this is set to TRUE, only resync what we've already downloaded.
	if (bDownloadedOnly)
	{
		// Do a resync of what we have already seen.
		//
		hResult = ResyncMailbox (pTocDoc, FALSE, 0, TRUE);
	}
	else
	{
		// Do a resync of everything.
		//

		hResult = ResyncMailbox (pTocDoc, FALSE, 0xFFFFFFFF, FALSE);
	}

	// If no window and we opened the connection, close it.
	//
	CTocView *pView = pTocDoc->GetView ();
	
	if (bWeOpenedConnection && !pView)
	{
		Close();
	}
	else
	{
		// Make sure tell the IMAP agent to update it's 
		//
//		RecreateMessageMap();
	}

	// Redraw the complete TOC:
	//
	if (pView)
		pView->Invalidate();
			
	return hResult;
}




// DoBackgroundResync [PRIVATE]
//
// Initiate a background resync.
//
HRESULT CImapMailbox::DoBackgroundResync (CTocDoc *pTocDoc,
								       BOOL bCheckMail /* = FALSE */,
									   BOOL bDownloadedOnly /* = TRUE */,
									   BOOL bDoSubMailboxes /* = FALSE */)
{
	HRESULT hResult = S_OK;
//	BOOL bWeOpenedConnection = FALSE; 

	// Sanity:
	if (!pTocDoc)
		return E_INVALIDARG;

	// Open the connection if the connection is not already open,
	// If we dropped the connection for whatever reason, try to re-connect.
	// Note: Keep tab of if we opened the connection because we may have to 
	// close it if there's no view.
	//
	if (!IsSelected())
	{
		// Just get login info.
		hResult = GetLogin();

//		hResult = OpenMailbox (FALSE);

		if (!SUCCEEDED (hResult) )
		{
			return hResult;
		}

		// Flag the we opened the connection.
		//
//		bWeOpenedConnection = TRUE;

	}

	// If this is Inbox and the "bCheckMail" flag is set,
	// set the ForceResync instance flag and
	// call the global GetMail routine out in pop.cpp.
	//
	CString szName = GetImapName ();
	szName.TrimLeft (); szName.TrimRight ();

	BOOL bIsInbox = IsInbox (szName);

	if ( bIsInbox && bCheckMail)
	{
		m_bForceResync = TRUE;

		CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
		if (pAccount)
		{
			// Get this personality's name.
			//
			CString szPersona; pAccount->GetName (szPersona);

			LPSTR pszPersonaName = DEBUG_NEW_NOTHROW char[ szPersona.GetLength() + 4 ];

			if (pszPersonaName)
			{
				// Note: Must terminate this string with double nuls.
				strcpy (pszPersonaName, (LPCSTR)szPersona);
				pszPersonaName[szPersona.GetLength() + 1] = '\0';

				GetMail(kManualMailCheckBits, pszPersonaName, FALSE/*bFullMailCheck*/);

				// Unset this:
				m_bForceResync = FALSE;

				hResult = S_OK;

				// Free it:
				delete[] pszPersonaName;
			}
		}
	}
	// If this is set to TRUE, only resync what we've already downloaded.
	else if (bDownloadedOnly)
	{
		// Do a resync of what we have already seen.
		//

		hResult = GetImapMailMgr()->DoManualResync (pTocDoc,
													FALSE,				// BOOL bCheckMail /* = FALSE */,
													bDownloadedOnly,	// BOOL bDownloadedOnly /* = TRUE */,
													TRUE,				// BOOL bInBackground /* = FALSE */,
													bDoSubMailboxes);	// BOOL bDoSubMailboxes /* = FALSE */)

	}
	// 
	else if (!bIsInbox)
	{
		// Resync of a non-INBOX mailbox - we want to retrieve everything!
		//
		hResult = GetImapMailMgr()->DoManualResync (pTocDoc,
													FALSE,				// BOOL bCheckMail /* = FALSE */,
													FALSE,				// BOOL bDownloadedOnly /* = TRUE */,
													TRUE,				// BOOL bInBackground /* = FALSE */,
													bDoSubMailboxes);	// BOOL bDoSubMailboxes /* = FALSE */)
	}
	else
	{
		// Do a resync of everything.
		//
		hResult = GetImapMailMgr()->DoManualResync (pTocDoc,
													FALSE,		// BOOL bCheckMail /* = FALSE */,
													FALSE,		// BOOL bDownloadedOnly /* = TRUE */,
													TRUE,				// BOOL bInBackground /* = FALSE */,
													bDoSubMailboxes);	// BOOL bDoSubMailboxes /* = FALSE */)
	}

	// If no window and we opened the connection, close it.
	//
	CTocView *pView = pTocDoc->GetView ();
	

#if 0 // The background task may still be in progress.
//
	if (bWeOpenedConnection && !pView)
		Close();
#endif // JOK 

	// Redraw the complete TOC:
	//
	if (pView)
		pView->Invalidate();
			
	return hResult;
}




//////////////////////////////////////////////////////////////////////////////
// CheckMail [PRIVATE]
//
// NOTES
// Checks first for a changed UIDVALIDITY. If it has changed, re-download
// all messages up to m_UIDHighest without filtering.
//
//
// If UIDVALIDITY hasn't changed, then use m_UIDHighest to determine if there are
// new messages. Download and filter any new messages. If an EXPUNGE occured
// as a result of filtering, we have to do a resync, otherwise, do a re-sync 
// only if "bForceResync" is TRUE.
//
// NOTE: Must have a TOC!!
//
//
// END NOTES

//////////////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::CheckMail (CTocDoc *pTocDoc, BOOL bSilent /* = FALSE */, BOOL bForceResync /* = FALSE */)
{
	HRESULT			hResult = S_OK;
	CString			buf;			// Scratch buffer.
	BOOL			bWasInProgress = FALSE;
	BOOL			bWeOpenedConnection = FALSE;
	CTocView		*pView = NULL;
	unsigned long	NewUidvalidity; 
	BOOL			bWePutProgressUp = FALSE;

	// Must have a TOC.
	if (!pTocDoc)
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// Try to thwart a preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	// Get info from the ACCOUNT
	if (m_AccountID == 0)
		return E_FAIL;

	// Get the account's name.
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
	{
		ASSERT (0);
		return E_FAIL;
	}
	
	// Prevent re-entrance problems!!
	if ( AlreadyCheckingMail () )
	{
		return S_OK;
	}

	// !! Otherewise, we're the one and only. Make sure un-set it before we leave!!!
	//
	SetAlreadyCheckingMail (TRUE);


	CString szPersona; pAccount->GetName (szPersona);

	// 
	// See if the mailbox is open:
	//
	pView = pTocDoc->GetView ();

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	// We can put up a progress window here!
	if ( ::IsMainThreadMT() && !bSilent )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		buf.Format (CRString(IDS_IMAP_UPDATING_MAILBOX), pTocDoc->Name());

		MainProgress(buf);

		bWePutProgressUp = TRUE;
	}

	// Open the connection if the connection is already open,
	// If we dropped the connection for whatever reason, try to re-connect.
	//
	if (!IsSelected())
	{
		hResult = OpenMailbox (bSilent);

		if (! SUCCEEDED (hResult) )
		{
			TerminateProgress (bWePutProgressUp, bWasInProgress);

			// MAKE SURE TO DO THIS!!!
			SetAlreadyCheckingMail (FALSE);

			return hResult;
		}

		// Flag that we opened the connection. We might want to close it when we leave.
		bWeOpenedConnection = TRUE;
	}

	// Must have a m_pImap now.
	//
	if (!m_pImap)
	{
		TerminateProgress (bWePutProgressUp, bWasInProgress);

		return E_FAIL;
	}

	// Change the progress message back.
	if ( bWePutProgressUp )
	{
		buf.Format (CRString(IDS_IMAP_UPDATING_MAILBOX), pTocDoc->Name());

		MainProgress(buf);
	}


	//
	// Reset hResult back to E_FAIL so we can detect success below.
	//
	hResult = E_FAIL;

	// Update the READ/WRITE state.
	BOOL bIsReadOnly = m_pImap->IsReadOnly();

	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);

		TerminateProgress (bWePutProgressUp, bWasInProgress);

		// Close connection?
		if (bWeOpenedConnection && !pView)
		{
			Close();
		}

		// MAKE SURE TO DO THIS!!!
		SetAlreadyCheckingMail (FALSE);

		return E_FAIL;
	}

	// If the R/W status has changed, reflect this.
	if (bIsReadOnly != IsReadOnly())
	{
		SetReadOnly (bIsReadOnly);
		pImapCommand->SetReadOnly (bIsReadOnly);

		// Update mailbox display.
		g_theMailboxDirector.ImapNotifyClients (pImapCommand, CA_IMAP_PROPERTIES, NULL);
	}

	// Make sure the local mailbox directory exists. "CreateMailboxDirectory()" will return TRUE
	// if the directory already exists as a direcotry.

	if ( ! VerifyCache (bSilent) )
	{
		TerminateProgress (bWePutProgressUp, bWasInProgress);

		// Close connection?
		if (bWeOpenedConnection && !pView)
		{
			Close();
		}

		// MAKE SURE TO DO THIS!!!
		SetAlreadyCheckingMail (FALSE);

		return E_FAIL;
	}

	// Set this here to prevent a recursive check mail if the TOC hasn't yet been opened
	// in a view.
	SetJustCheckedMail (TRUE);

	// Reset this:
	m_bUidValidityChanged = FALSE;

	// Send a Noop so we get any pending EXISTS messages.
	m_pImap->Noop ();

	// How many messages do we now think we have? 
	// Update m_NumberOfMessages only if we succeed.
	//
	unsigned long nMsgs = 0;

	hResult = m_pImap->GetMessageCount(nMsgs);

	if ( !SUCCEEDED (hResult) )
	{
		TerminateProgress (bWePutProgressUp, bWasInProgress);

		return hResult;
	}

	// Ok so far.
	m_NumberOfMessages = nMsgs;

	// Get the server's highest UID to see if it's zero. If zero, we need to 
	// close the conection and restart it.
	//
	unsigned long ServerUidHighest = 0;

	hResult = m_pImap->UIDFetchLastUid(ServerUidHighest);

	if (::EscapePressed())
	{
		TerminateProgress (bWePutProgressUp, bWasInProgress);

		return HRESULT_MAKE_CANCEL;
	}

	if ( !SUCCEEDED (hResult) )
	{
		TerminateProgress (bWePutProgressUp, bWasInProgress);

		return hResult;
	}

	if ( (m_NumberOfMessages > 0) && (ServerUidHighest == 0) )
	{
		Close();

		hResult = OpenMailbox (bSilent);

		if (! SUCCEEDED (hResult) )
		{
			TerminateProgress (bWePutProgressUp, bWasInProgress);

			return hResult;
		}

		// How many messages do we now think we have? 
		//
		unsigned long nMsgs = 0;

		hResult = m_pImap->GetMessageCount(nMsgs);

		if ( !SUCCEEDED (hResult) )
		{
			TerminateProgress (bWePutProgressUp, bWasInProgress);

			return hResult;
		}

		// Ok so far.
		m_NumberOfMessages = nMsgs;
	}

	// Now that we have opened the mailbox successfully, get the UIDValidity,
	//
	hResult = m_pImap->GetUidValidity(NewUidvalidity);

	if ( !SUCCEEDED (hResult) )
	{
		TerminateProgress (bWePutProgressUp, bWasInProgress);

		return hResult;
	}


	// Allow zero UIDVALIDITY??

	// If no messages in remote mailbox, just delete our local cached.
	if (m_NumberOfMessages == 0)
	{
		pTocDoc->ImapRemoveAllSummaries ();
	}

	// If the UIDVALIDITY changed, we have to download everything.
	if (m_Uidvalidity != NewUidvalidity)
	{
		pTocDoc->ImapRemoveAllSummaries ();

		m_bUidValidityChanged = TRUE;

		// We have to redownload stuff, and we can't do filtering during
		// this redownload because our local "m_UIDHighest" is no longer valid.
		// We don't know what we previously filtered.
		//

		m_UIDHighest = 0;

		// Tell this to the user if it's inbox.
		//
#if 0	// What about during a timed check mail??
		//
		if ( IsInbox (GetImapName()) )
			ErrorDialog (IDS_NOTE_IMAP_UIDVALIDITY_CHANGED, GetImapName());
#endif
	}

	// Ok. Get and filter new messages. Set the m_bExpunged flag if filtering
	// caused an expunge.
	//

	m_bExpunged = FALSE;

	BOOL bWasResynced = FALSE;

	hResult = GetAndFilterNewMessages (pTocDoc, bWasResynced, TRUE, bSilent);

	// If we were asked to force a re-sync, or if messages
	// were removed as part of a filter, do a re-sync of
	// what we got. Don't fetch any new messages at this time.
	// Note: "GetAndFilterNewMessages" would have updated "m_UIDHighest"
	// to include any new messages we already received.
	//
	// Note that a client may also force a resync by setting the member
	// flag "m_bForceResync".
	//
	bForceResync = bForceResync | m_bForceResync;

	// Remember, attempt a re-sync only if user didn't cancel.
	//

	if ( SUCCEEDED (hResult) && ( bForceResync || (m_bExpunged && !bWasResynced) ) )
	{
		// This forces a resync only of messages we already received.
		//
		hResult = ResyncMailbox (pTocDoc, bSilent, m_UIDHighest, TRUE);
	}

	// Update stuff if we succeeded.
	//
	if ( SUCCEEDED (hResult) )
	{
		//
		// Update mailbox info.
		//
		m_Uidvalidity = NewUidvalidity;

		// Reset this:
		m_bUidValidityChanged = FALSE;
	}

	// Update saved info, whether we succeeded or not.
	//
	WriteImapInfo (TRUE);

	// Close connection?
	if (bWeOpenedConnection && !pView)
	{
		Close();
	}

	TerminateProgress (bWePutProgressUp, bWasInProgress);

	// MAKE SURE TO DO THIS!!!
	SetAlreadyCheckingMail (FALSE);

	return hResult;
}




///////////////////////////////////////////////////////////////////////////////////////////
// GetAndFilterNewMessages [PRIVATE]
// 
// Use m_UIDHighest to get messages with a UID higher than it and filter those messages.
//
// On OUTPUT: Set the m_Expunged flag if filtering caused an EXPUNGE of the mailbox.
//
// Assume the mailbox is connected. 
// 
// NOTE: This may still cause a full resync. If it did, convey this info back to
// caller by seeting "bWasResynced" to TRUE.
//
////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::GetAndFilterNewMessages (CTocDoc *pTocDoc, BOOL& bWasResynced, BOOL bDoFiltering /* = TRUE */, BOOL bSilent /* = FALSE */)
{
	HRESULT hResult = S_OK;
	unsigned long NextUid = 0; 

	// Make sure.
	bWasResynced = FALSE;

	// Sanity:
	if (!pTocDoc)
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Get our current list of uid's from the summary.
	CUidMap CurrentUidMap;

	GetCurrentUids (pTocDoc, CurrentUidMap);

	// Get the highest one we have locally.
	unsigned long LocalUidHighest	= CurrentUidMap.GetHighestUid();

	unsigned long ServerUidHighest = 0;

	hResult = m_pImap->UIDFetchLastUid(ServerUidHighest);

	if ( !SUCCEEDED (hResult) )
	{
		return hResult;
	}


	// If there are messages in the mailbox, ServerUidHighest MUST be
	// non-zero.
	//
	if ( (m_NumberOfMessages > 0) && (ServerUidHighest == 0) )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If UIDVALIDITY was hanged, we have to redownload everything, regardless.
	//
	if (m_bUidValidityChanged)
	{
		if (ServerUidHighest > 0)
		{
			// Do a complete re-sync only of what we already have.
			hResult = ResyncMailbox (pTocDoc, bSilent, ServerUidHighest, FALSE);

			// Tell caller that we did.
			bWasResynced = TRUE;

			m_UIDHighest = ServerUidHighest;
		}
	}
	// Resync if user deleted local cache.
	else
	{
		// If the toc was rebuilt or the user deleted the local cache, resync first.
		//
		if ( m_bTocWasJustRebuilt || (m_UIDHighest != 0 && LocalUidHighest == 0) )
		{
			// Do a complete re-sync only of what we already have.
			hResult = ResyncMailbox (pTocDoc, bSilent, m_UIDHighest, FALSE);

			if (!SUCCEEDED (hResult))
				return E_FAIL;

			// Tell caller that we did.
			bWasResynced = TRUE;

			// Reset our flag.
			m_bTocWasJustRebuilt = FALSE;
		}

		// If all goes well, download the next after this.
		//
		NextUid = max (LocalUidHighest, m_UIDHighest);

		// We may have removed messages at the tail end of the mailbox:
		//
		NextUid = min(ServerUidHighest, NextUid);

		// Did user abort??
		//
		if (EscapePressed())
		{
			return HRESULT_MAKE_CANCEL;
		}

		// Now, Fetch only if server has higher UID.
		//
		if ( (ServerUidHighest > 0) && (ServerUidHighest > NextUid) )
		{
			CString szSeq;
			CUidMap NewUidMap;

			szSeq.Format ("%lu:%lu", NextUid + 1, ServerUidHighest);

			m_pImap->FetchFlags (szSeq, &NewUidMap);

			// Did user cancel??
			if (EscapePressed())
			{
				hResult = HRESULT_MAKE_CANCEL;
			}
			else if (NewUidMap.GetCount() > 0)
			{
				// We need to know if filtering caused an expunge.
				m_bExpunged = FALSE;

				// Note: THis does the filtering (if it's INBOX)!!
				// Also: This MUST set the new m_UIDHighest.
				hResult = DoFetchNewMessages (pTocDoc, NewUidMap, bDoFiltering);

				// Note!! m_UIDHighest would have been updated in "DoFetchMessages".
				// Don't update it here!!
			}

			// Make sure:
			NewUidMap.DeleteAll ();
		}

		// Did user abort??
		//
		if (EscapePressed())
		{
			hResult = HRESULT_MAKE_CANCEL;
		}
	}

	return hResult;
}





// ResyncMailbox [PRIVATE]
// FUNCTION
// Do the dirty work of fetching the flags from the server and updating 
// the local mailbox cache.
// 
// NOTE: This can NOT be called directly.
//
// END FUNCTION

// NOTES
// Assume that the connection is open.
//
// NOTE: This now re-sync's only up to the given "ulMaxUid". 
// If "ulMaxUid" is 0xFFFFFFFF, we re-sync ALL messages, unless the "bDownloadedOnly"
// flag is set, in which case we only resync what was already downloaded.
// Note: If "bDownloadedOnly" is set, we ignore "ulMaxUid".
//
// END NOTES

HRESULT CImapMailbox::ResyncMailbox (CTocDoc *pTocDoc, BOOL bSilent /* = FALSE */,
									IMAPUID ulMaxUid /* = 0xFFFFFFFF */,
									BOOL bDownloadedOnly /* = TRUE */)
{
	HRESULT		hResult = S_OK;
	BOOL		bWasInProgress = FALSE;
	unsigned long NewUidvalidity = m_Uidvalidity;

	// Must be part of a valid account.
	if (m_AccountID == 0)
		return E_FAIL;

	// Must have a toc.
	if (!pTocDoc)
		return E_INVALIDARG;

	// Get the account's name.
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Must have one of these:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	CString szPersona; pAccount->GetName (szPersona);

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	// Send a Noop so we get any pending EXISTS messages.
	m_pImap->Noop ();

	// Make these checks again because things may have changed.
	//
	// How many messages do we now think we have? 
	//
	unsigned long nMsgs = 0;

	hResult = m_pImap->GetMessageCount(nMsgs);

	if ( !SUCCEEDED (hResult) )
	{
		return hResult;
	}

	// Ok so far.
	m_NumberOfMessages = nMsgs;


	// Get the server's highest UID to see if it's zero. If zero, we need to 
	// close the conection and restart it.
	//
	unsigned long ServerUidHighest = 0;

	hResult = m_pImap->UIDFetchLastUid(ServerUidHighest);

	if ( !SUCCEEDED (hResult) )
	{
		return hResult;
	}

	// We can put up a progress window here!
	if ( ::IsMainThreadMT() && !bSilent )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		CString buf;

		buf.Format (CRString(IDS_IMAP_RESYNCING_MAILBOX), pTocDoc->Name());

		MainProgress(buf);
	}

	if ( (m_NumberOfMessages > 0) && (ServerUidHighest == 0) )
	{
		Close();

		hResult = OpenMailbox (bSilent);

		if (! SUCCEEDED (hResult) )
		{
			if ( ::IsMainThreadMT() && !bSilent )
			{
				if (bWasInProgress)
					PopProgress();
				else
					CloseProgress();
			}

			return hResult;
		}

		// How many messages do we now think we have? 
		unsigned long nMsgs = 0;

		hResult = m_pImap->GetMessageCount(nMsgs);

		if ( SUCCEEDED (hResult) )
		{
			m_NumberOfMessages = nMsgs;

			// Now that we have opened the mailbox successfully, get the UIDValidity,
			//
			hResult = m_pImap->GetUidValidity(NewUidvalidity);

			if ( SUCCEEDED(hResult) )
			{
				hResult = m_pImap->UIDFetchLastUid(ServerUidHighest);
			}
		}

		if (! SUCCEEDED (hResult) )
		{
			if ( ::IsMainThreadMT() && !bSilent )
			{
				if (bWasInProgress)
					PopProgress();
				else
					CloseProgress();
			}

			return hResult;
		}
	}


	/// Note: Ignore UIDVALIDITY here,

	// If no messages in remote mailbox, just delete our local cached.
	if (m_NumberOfMessages == 0)
	{
		pTocDoc->ImapRemoveAllSummaries ();

		hResult = S_OK;
	}

	// If the UIDVALIDITY changed, we have to download everything.
	if (m_Uidvalidity != NewUidvalidity)
	{
		pTocDoc->ImapRemoveAllSummaries ();

		m_bUidValidityChanged = TRUE;

		// We have to redownload stuff, and we can't do filtering during
		// this redownload because our local "m_UIDHighest" is no longer valid.
		// We don't know what we previously filtered.
		//
		m_UIDHighest = 0;
	}

	if (m_NumberOfMessages > 0)
	{
		// If we get here, do the resync.

		CUidMap NewUidMap;
		CUidMap StaleUidMap;

		// Get the curent list into "StaleUidMap"
		GetCurrentUids (pTocDoc, StaleUidMap);

		// If bDownloadedOnly is TRUE, resync only to what we already know
		// about.
		//
		IMAPUID MaxUidToFetch;

		if (bDownloadedOnly)
		{
			// Get the highest one we have locally.
			unsigned long LocalUidHighest	=  StaleUidMap.GetHighestUid();

			MaxUidToFetch = max (LocalUidHighest, m_UIDHighest);
		}
		else
		{
			// Fetch UID's up to ulMaxUid from the server into the other list.
			//
			MaxUidToFetch = min (ulMaxUid, ServerUidHighest);
		}

		if (MaxUidToFetch > 0)
		{
			CString szSeq;

			szSeq.Format ("1:%lu", MaxUidToFetch);

			hResult = m_pImap->FetchFlags (szSeq, &NewUidMap);
		}

		if (EscapePressed() || !SUCCEEDED (hResult) )
		{
			NewUidMap.DeleteAll();

			if ( ::IsMainThreadMT() && !bSilent )
			{
				if (bWasInProgress)
					PopProgress();
				else
					CloseProgress();
			}

			if (EscapePressed())
			{
				return HRESULT_MAKE_CANCEL;
			}
			else
				return E_FAIL;
		}

		// Update "MaxUidToFetch" to reflect the highest UID we ACTUALLY fetched.
		//
		MaxUidToFetch = NewUidMap.GetHighestUid();

		// Merge the two lists.
		// When done, the two lists will be in the following states:
		// 1. StaleUidMap contains only uid's no longer on the server.
		// 2. NewUidMap contains only new uid's
		// 3. NewOldUidMap contains uid's still on the server, but
		//	  m_Imflags have new values.

		CUidMap NewOldUidMap;

		MergeUidMapsMT (StaleUidMap, NewUidMap, NewOldUidMap);

		//
		// Go update the flags of old summaries.
		// Note: UpdateOldSummaries removes entries from NewOldUidMap!!!
		//
		UpdateOldSummaries (pTocDoc, NewOldUidMap);

		// Now, remove any stale summaries.

		// Now, remove summaries in StaleUidMap.
		if (StaleUidMap.GetCount() > 0 )
		{
			int HighlightIndex;
			pTocDoc->ImapRemoveListedTocs (&StaleUidMap, HighlightIndex, TRUE, TRUE);
		}

		// If there are messages, download them but don't do any filtering.
		// Filtering at this point can again leave the mailboxes out of sync.
		//
		if (NewUidMap.GetCount() > 0)
		{
			hResult = DoFetchNewMessages (pTocDoc, NewUidMap, FALSE);

			/// If we fetched a higher UID than we already knew about, update 
			// our stored m_UIDHighest.
			//
			if ( SUCCEEDED (hResult) && (MaxUidToFetch > m_UIDHighest) )
			{
				m_UIDHighest = MaxUidToFetch;
			}
		}

		// Make sure.
		NewUidMap.DeleteAll();
		NewOldUidMap.DeleteAll ();
		StaleUidMap.DeleteAll ();
	}

	// Cleanup.

	if ( ::IsMainThreadMT() && !bSilent )
	{
		if (bWasInProgress)
			PopProgress();
		else
			CloseProgress();
	}

	return hResult;
}





/////////////////////////////////////////////////////////////////////////////////////
// DoFetchNewMessages [private]
// FUNCTION
// Do the details of Fetching new messages.
// New summaries are created and added to "pSumList".
// END FUNCTION
// NOTES
// This also does a crude "resync" in that it checks to see if any messages
// have been deleted from the server or if message flags have changed. 
// Note: NewUidMap now contains ONLY new messages. Downloaded messages are removed
// from the list.
// Note:Return FALSE only if a grievous error ocurred.
//
// NOTE: If bDoFiltering is FALSE, don't filter inbox.
// END NOTES.
/////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::DoFetchNewMessages (CTocDoc *pTocDoc, CUidMap& NewUidMap, BOOL bDoFiltering)
{
	BOOL			bMustCloseProgress = FALSE;
	HRESULT			hResult = S_OK;
	CImapSumList	SumList;

	// Must have a summary list object.
	if (! pTocDoc )
		return E_INVALIDARG;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	//
	// First find out how many we have to download. Note: NewUidMap contains
	// only new messgages!!
	//

	int nCountBeforeFiltering = NewUidMap.GetCount ();

	// If we don't have any, don't continue.
	if (nCountBeforeFiltering <= 0)
	{
		return S_OK;
	}

	// If progress window is NOT already up, put one up.
	if ( ::IsMainThreadMT() )
	{
		if (!InProgress)
		{
			CString buf;

			buf.Format (CRString(IDS_IMAP_UPDATING_MAILBOX), pTocDoc->Name());
			MainProgress(buf);

			bMustCloseProgress = TRUE;
		}
	}

	// If this is "Inbox", do "transfer" filtering at this point.
	// This will transfer matching messages to mailboxes
	// on the same server, or download messages into local mailboxes.
	// When this returns, NewUidMap will contain only messages that should be downloaded 
	// into this mailbox.
	//
	// NOTE: This is the first stage in the filtering process.!!
	//

	CString szName = GetImapName ();
	szName.TrimLeft (); szName.TrimRight ();

	BOOL bFiltering = FALSE;

	// At this point, we know we've got new messages. If this is the
	// INBOX, we need to do a few things.
	//
	if (IsInbox (szName))
	{
		// Tell our parent account that we've got mail.
		CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
		if (pAccount)
			pAccount->SetGotNewMail (TRUE);

		// Set to TRUE if this is a mailbox that we'er filtering.
		bFiltering = bDoFiltering;
	}

	// In case we're filtering.
	CImapFilterActions FilterActions;

	// NOTE: Always set filename of mailbox.
	FilterActions.SetMBFilename (pTocDoc->GetMBFileName ());

	// Make sure reset these.
	FilterActions.m_bPreFilterXferred = FALSE;
	FilterActions.m_bPostFilterXferred = FALSE;

	// Clear our own as well.
	m_bExpunged = FALSE;

	if (bFiltering)
	{
		// Flag if filtering cause an expunge.
		//

		// Change the progress message string.
		if ( InProgress )
		{
			MainProgress( CRString(IDS_IMAP_DOING_SAMESERVER_FILTERING) );
		}

		//
		// NOTE: If StartFiltering succeeds, we MUST call EndFiltering
		// before we leave this method!!!
		//
		if (FilterActions.StartFiltering())
		{
			// Update highest UID so far.
			//
			IMAPUID ulHighestUidSeen = FilterActions.GetHighestUidSeen ();

			if (ulHighestUidSeen > m_UIDHighest)
				m_UIDHighest = ulHighestUidSeen;
		}
		else
			bFiltering = FALSE;
	}

	if (EscapePressed())
	{
		//
		// Downloaded messages that remain after pre-filtering.
		// NOTE: Accumulate new summaries in SumList.
		// NOTE: This also updates "m_UIDHighest".
		//
		DownloadRemainingMessages (pTocDoc, NewUidMap, &SumList);
	}

	// Add the summaries to the TOC now.
	// NOTE: ImapAppendSumList() adds then to the TOC BUT also adds each new summary
	// to the CSumList that's passed in. This is so we can do the
	// post filtering below. Note that "RefSumList" contains the same CSummaries that
	// are in the TOC, so they MUST NET BE freed when RefSumList is deleted!!
	//
	CTempSumList RefSumList;

	ImapAppendSumList (pTocDoc, &SumList, &RefSumList);

	// NOTE: If we started the filtering process, we must end it.
	if (bFiltering)
	{

		// Do post filtering on pSumList. Ignore "same server xfers".
		if (!EscapePressed())
		{
			// Note: Bool Flags are: bDoSameServerXfers, bDoIncoming, bDoManual.
			//
			FilterActions.DoFiltering(pTocDoc,
									  RefSumList,
									  NULL/*pstrTransUIDs*/,
									  FALSE/*bDoSameServerXfers*/,
									  TRUE/*bDoIncoming*/,
									  FALSE/*bDoManual*/);
		}

		// If we need to do an expunge, do it here.
		if ( FilterActions.m_bPreFilterXferred ||
			 (FilterActions.m_bPostFilterXferred && ExpungeAfterDelete (pTocDoc) ) )
		{
			Expunge ();

			m_bExpunged = TRUE;
		}

		// Do terminating things.
		FilterActions.EndFiltering();
	}


	// Must free RefSumList's internal memory. Note: Not the data. Just the list.
	//
	RefSumList.RemoveAll();

	// For the CImapSumList, we must free the object memory as well.
	//
	SumList.DeleteAll();

	// Close progres if we started it..

	if ( ::IsMainThreadMT() )
	{
		if (bMustCloseProgress)
			CloseProgress();
	}

	if (EscapePressed())
	{
		hResult = HRESULT_MAKE_CANCEL;
	}

	return hResult;
}




/////////////////////////////////////////////////////////////////////
// DownloadRemainingMessages [PRIVATE]
//
// Called after prefiltering to download the remaining messages.
//
/////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::DownloadRemainingMessages (CTocDoc *pTocDoc, CUidMap& NewUidMap,
							 CImapSumList *pSumList, BOOL bToTmpMbx /* = FALSE */, CTaskInfoMT* pTaskInfo /* = NULL */ )
{
	HRESULT		hResult = S_OK;
	IMAPUID		uid		= 0;
	CString		strMainText;

	// Must have a summary list object.
	if (! (pTocDoc && pSumList) )
		return E_INVALIDARG;

	//
	// Get new value of count. Note that NewUidMap contains new messages only.
	//
	int nCountAfterFiltering = NewUidMap.GetCount ();

	strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nCountAfterFiltering);

	// Now go download remaining messages.
	if (InProgress && ::IsMainThreadMT() && (nCountAfterFiltering > 0) )
	{
		MainProgress(strMainText);
	}
	// Initialize for background progress.
	//
	else if (pTaskInfo && !IsMainThreadMT() && (nCountAfterFiltering > 0) )
	{
		pTaskInfo->SetMainText(strMainText);

		pTaskInfo->SetTotal(nCountAfterFiltering);
	}

	// Figure out what kind of download we'd be doing.
	BOOL bMinimalDownload = DoingMinimalDownload (this);

	//
	// Keep tab of how many we download..
	//
	int count = 0;

	// If error mid-way, stop the transfer.
	hResult = S_OK;

	// Loop through all messages.

	CImapFlags	*pF;

	//
	// Download what's left after filtering into the mailbox.
	//

	for( UidIteratorType ci = NewUidMap.begin(); ci != NewUidMap.end(); ci++  )
	{
		// Does user want to quit??
		if (::EscapePressed())
		{
			hResult = HRESULT_MAKE_CANCEL;

			break;
		}

		pF = ( CImapFlags * ) (*ci).second;
		if (!pF)
			continue;

		uid = pF->m_Uid;

		if (uid == 0 || !pF->m_IsNew)
			continue;

		// 
		count++;

		// Count only new messages.
		if ( ::IsMainThreadMT() )
			Progress(count);

		// Set the flags for this message:
		m_bHasUnread = m_bHasUnread || !(pF->m_Imflags & IMFLAGS_SEEN);

		// If success, this will be non-NULL:
		CImapSum *pImapSum; pImapSum = NULL;

		// If this message was not filtered, go download to the mailbox.
		// Check to see if to download all or just minimal download.
		if ( bMinimalDownload )
		{
			// Return a partially filled-in new CSummary object.
			hResult = DoMinimalDownload (pTocDoc, uid, &pImapSum);

			// If this our highest UID so far?
			//
			if ( pImapSum )
			{
				if ( uid > m_UIDHighest )
					m_UIDHighest = uid;

				// Indicate that this is a minimal download.
				pImapSum->m_Imflags |= IMFLAGS_NOT_DOWNLOADED;
			}
		}
		else
		{
			pImapSum = DEBUG_NEW_MFCOBJ_NOTHROW CImapSum;
			if (pImapSum)
			{
				// At lease these must be filled in:
				//
				pImapSum->SetHash (uid);

				// Fetch message, probably including all attachments.
				hResult = DownloadSingleMessage (pTocDoc, pImapSum, FALSE, bToTmpMbx);

				if ( SUCCEEDED (hResult) )
				{			
					// Indicate that this was more than a minimal download..
					pImapSum->m_Imflags &= ~IMFLAGS_NOT_DOWNLOADED;

					// If this our highest UID so far?
					//
					if ( uid > m_UIDHighest )
						m_UIDHighest = uid;
				}
				else
				{
					// We created pImapSum. Delete it.
					delete pImapSum;

					pImapSum = NULL;
				}
			}
		}

		// Did we get a summary??
		if (pImapSum)
		{
			// Set Sum->m_Imflags flags based on pF->m_Imflags.
			// Note: Set selected flags. Don't do a blanket copy.
			//
			//
			// First, clear the bits we're interested in.
			pImapSum->m_Imflags &= ~( IMFLAGS_SEEN | IMFLAGS_ANSWERED | IMFLAGS_FLAGGED | 
										  IMFLAGS_DELETED | IMFLAGS_DRAFT | IMFLAGS_RECENT );
			//
			pImapSum->m_Imflags |= (pF->m_Imflags & IMFLAGS_SEEN);
			pImapSum->m_Imflags |= (pF->m_Imflags & IMFLAGS_ANSWERED);
			pImapSum->m_Imflags |= (pF->m_Imflags & IMFLAGS_FLAGGED);
			pImapSum->m_Imflags |= (pF->m_Imflags & IMFLAGS_DELETED);
			pImapSum->m_Imflags |= (pF->m_Imflags & IMFLAGS_DRAFT);
			pImapSum->m_Imflags |= (pF->m_Imflags & IMFLAGS_RECENT);
	
			// We must set some of Eudora's flags too.
			if (pF->m_Imflags & IMFLAGS_SEEN)
				pImapSum->SetState (MS_READ);
			else
				pImapSum->SetState (MS_UNREAD);

			if (pF->m_Imflags & IMFLAGS_ANSWERED)
				pImapSum->SetState (MS_REPLIED);

			// Add the CImapSum now.
			pSumList->AddTail (pImapSum);
		}

		strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nCountAfterFiltering - count);

		// Update progress bar.
		if (InProgress && ::IsMainThreadMT() && SUCCEEDED (hResult) )
		{
			MainProgress(strMainText);
		}
		// If in background, use the task-status' progress.
		else if (!::IsMainThreadMT() && pTaskInfo)
		{
			pTaskInfo->SetMainText(strMainText);

			pTaskInfo->ProgressAdd(1);
		}

		// Does user want to quit??
		if (EscapePressed())
		{
			hResult = HRESULT_MAKE_CANCEL;

			break;
		}

	}  // For.

	return hResult;
}






/////////////////////////////////////////////////////////////////////////////////////////////////
// DoMinimalDownload [PRIVATE]
//
// FUNCTION
// Do a minimal download just to get the message's attributes, fill in a new CSummary
// and return it.
// Mark the new CSummary with the flag to indicate that the message is only partially downloaded.
// The new CSummary is returned, or NULL.
// END FUNCTION
////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::DoMinimalDownload (CTocDoc *pTocDoc, IMAPUID Uid, CImapSum **ppSum)
{
	BOOL			bWeOpenedOurConnection = FALSE;
	CImapSum*		pSum = NULL;
	HRESULT			hResult = S_OK;

	// Sanity. Must have a valid TocDoc and Uid. 
	if ( NULL == pTocDoc || Uid == 0 || NULL == ppSum)
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// Intialize output parms:
	*ppSum = NULL;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	// Make sure we can open the remote connection to our mailbox.
	if ( !IsSelected() )
	{
		hResult =  OpenMailbox (FALSE);

		if (! SUCCEEDED (hResult) )
			return hResult;

		bWeOpenedOurConnection = TRUE;
	}

	// Must have one of these now!!
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Instantiate a CImapDownloader object to do the actual downloading.
	CImapDownloader ImapDownloader (GetAccountID(), m_pImap, NULL);

	pSum = NULL;
	hResult = ImapDownloader.DoMinimalDownload (Uid, &pSum);

	// Should we close the stream?
	CTocView *View = pTocDoc->GetView ();

	if (bWeOpenedOurConnection && !View)
		Close();

	// Set output parm:
	*ppSum = pSum;

	return hResult;
}





//////////////////////////////////////////////////////////////////////////////////////
// DownloadSingleMessage [PUBLIC] - CImapSum version!!!
// FUNCTION
// This is a public wrapper around the corresponding CImapDownloader's
// method of the same name.
// "pSum" is a stub pointing to an as yet undownloaded message. Go out and 
// append the message to the TOC's MBX file.
// Use the INI options to determine if to download attachments.
// Create stub files as place-holders for un-downloaded attachments.
// Additional possible return codes:
//	ERROR_CANCELLED : user cancelled.
// 
// END FUNCTION.

// HISTORY
// Created 9/15/97 by JOK.
// END HISTORY.
//////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::DownloadSingleMessage (CTocDoc *pTocDoc, CImapSum *pSum,
											 BOOL bDownloadAttachments, BOOL bToTmpMbx /* = FALSE */)
{
	HRESULT hResult = S_OK;

	// Sanity:
	if (! (pTocDoc && pSum) )
		return E_INVALIDARG;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	//
	// Also, don't accept any mouse or keyboard input during this.
	// Note: Re-enable before we leave.
	//

	CTocFrame* pTocFrame = NULL;

	CTocView *pView = pTocDoc->GetView ();

	if (pView)
	{
		// Disable the whole darn frame!!
		pTocFrame = (CTocFrame *) pView->GetParentFrame();
	}

	// If we're offline, write a stub to the MBX file and return OK.
	//
	if ( !IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		return WriteOfflineMessage (pTocDoc, pSum, bToTmpMbx);
	}



	//
	// If this is the TOC m_pImapMailbox object, make sure it's still connected
	// to the server, If not, re-open the connection.
	//

	// Try to re-open if, for some reason, the connection was dropped.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox (FALSE);

		if ( !SUCCEEDED (hResult) )
			return hResult;
	}

	// Must have one of these now.
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Open the MBX file for appending. This can be either the real
	// MBX file or the temporary one.
	//
	CString szMbxFilePath;

	if (bToTmpMbx)
	{
		GetTmpMbxFilePath (GetDirname(), szMbxFilePath);
	}
	else
	{
		szMbxFilePath = pTocDoc->GetMBFileName();
	}

	// Instantiate an ImapDownloader object to do the downloading.
	CImapDownloader MbxDownloader (GetAccountID(), m_pImap, (LPCSTR) szMbxFilePath);

	// 	Make sure the file exists. Open it for appending. Don't truncate!!
	if (!MbxDownloader.OpenMbxFile (FALSE))
	{
		ErrorDialog ( IDS_ERR_FILE_OPEN, szMbxFilePath, CRString (IDS_ERR_FILE_OPEN_WRITING) );

		return E_FAIL;
	}

	// Go download it.

	hResult = MbxDownloader.DownloadSingleMessage (pSum, bDownloadAttachments);

	//
	// Note: Only the CImapDownloader can set IMFLAGS_NOT_DOWNLOADED into the summary.
	// Don't do it here because bResult can be TRUE if we wrote a dummy message to the MBX file.
	//

	// CLose the file back.
	MbxDownloader.CloseMbxFile ();

	return hResult;
}


//
//	CImapMailbox::DownloadFullHeaders()
//
//	Download the entire set of headers for the message specified in pSum and place the results in strHeaders.
//
HRESULT CImapMailbox::DownloadFullHeaders(CTocDoc *pTocDoc, CSummary *pSum, CString &strHeaders)
{
	HRESULT hResult = S_OK;

	// Sanity:
	if (!pTocDoc || !pSum)
	{
		return E_INVALIDARG;
	}

	// If this is the TOC m_pImapMailbox object, make sure it's still connected
	// to the server, If not, re-open the connection.
	if (!IsSelected())
	{
		hResult = OpenMailbox(FALSE);

		if (!SUCCEEDED(hResult))
		{
			return hResult;
		}
	}

	// Bail if we don't have a connection.
	if (!m_pImap)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Instantiate an ImapDownloader object to do the downloading.
	CImapDownloader		imapdownloader(GetAccountID(), m_pImap, NULL);
	hResult = imapdownloader.DownloadFullHeaderToString(pSum->GetHash());
	strHeaders = imapdownloader.m_strFullHeader;

	return hResult;
}

//////////////////////////////////////////////////////////////////////////////////////
// OnPreviewSingleMessage [PUBLIC]
// FUNCTION
// This is the method that should be called from a preview.
// It tests to see if the user just cancelled an attempt to open the remote mailbox
// and don't re-attempt until after a certain time has elapsed.
//
// Additional possible return codes:
//	ERROR_CANCELLED : user cancelled.
// END FUNCTION.

// HISTORY
// Created 9/15/97 by JOK.
// END HISTORY.
//////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapMailbox::OnPreviewSingleMessage(CSummary *pSum, BOOL bDownloadAttachments)
{
	HRESULT hResult = S_OK;

	// Must be in main thread.
	//
	ASSERT (IsMainThreadMT());

	// Is preview not allowed at this time?
	if (!PreviewAllowed())
	{
		return HRESULT_MAKE_CANCEL;
	}

	int		 iConnectionState = GetConnectionState(bDoAllowOffline);
	if (iConnectionState == iStateStayOfflineDisallow)
	{
		hResult = HRESULT_MAKE_CANCEL;
	}
	else if (iConnectionState == iStateStayOfflineAllow)
	{
		CString		 strUidList;
		strUidList.Format("%lu", pSum->GetHash());
		if (!m_pTocDoc->QueueImapFetch(strUidList,
									   bDownloadAttachments,
									   TRUE/*bOnlyIfNotDownloaded*/,
									   FALSE/*bClearCacheOnly*/,
									   FALSE/*bInvalidateCachedPreviewSums*/))
		{
			hResult = E_FAIL;
		}
	}
	else
	{
		hResult = DownloadSingleMessage(m_pTocDoc, pSum, bDownloadAttachments);
	}

	return hResult;
}

//
//	CImapMailbox::FetchHeaderString()
//
//	Download the headers for the message associated with pSum.  Downloading
//	will fill in the summary's m_strRawFrom field (which is not preserved) and
//	we fill strFrom with that value.
//
HRESULT CImapMailbox::FetchFromHeaderString(CTocDoc *pTocDoc, CSummary *pSum, CString &strFrom)
{
	HRESULT hResult = S_OK;

	// Must be in main thread.
	ASSERT (IsMainThreadMT());

	// Must have one of these now.
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Instantiate a CImapDownloader object to do the actual downloading.
	CImapDownloader	 ImapDownloader(m_AccountID, m_pImap, NULL);
	CImapSum		*pImapSum = DEBUG_NEW CImapSum();
	hResult = ImapDownloader.DoMinimalDownload(pSum->GetHash(), &pImapSum);
	strFrom = pImapSum->m_strRawFrom;
	delete pImapSum;

	return hResult;
}


// DidUserCancel [PUBLUC]
// 
// Since we're the one who constructed the HRESULT, we're the one to
// determine if it contains a USER CANCELLED.
// A caller will pass us an HRESULT that we previously returned to him.
// We return TRUE is it contains a flag that we set, indicating that the
// user did cancel.
//
BOOL CImapMailbox::DidUserCancel (HRESULT hResult)
{
	return HRESULT_CONTAINS_CANCEL (hResult);
}



// XferMessagesOnServer [PUBLIC]
// FUNCTION
// Given a comma-separated array of message UID's, send a single UID COPY command
// to the IMAP server. pDestination is the IMAP name of the destination mailbox.
// If !Copy, flag them for deletion.
// Return a BOOL for now but later, we should return an ALLOCATED CString
// containing the uid's of successfully moved messages,
// END FUNCTION
// NOTES
// In order to optimize the COPY, somewhere along the line we should format 
// uid's corresponding to consecutive messages using the ":". 
// END NOTES.
HRESULT CImapMailbox::XferMessagesOnServer (LPCSTR pUidlist,
											CDWordArray *pdwaNewUIDs,
											LPCSTR pDestination,
											BOOL Copy,
											BOOL bSilent/*=FALSE*/)
{
	HRESULT			hResult = S_OK;

	// Sanity.
	if (!(pUidlist && pDestination))
		return E_INVALIDARG;

	// There was code commented out here to check our online status before proceeding.  This code is reached
	// by four different operations: normal transfers, trashing and junking (which all perform the online
	// status check before this point) and old style filtering.  Old style filtering does its matching by
	// searching the messages on the server so if we get here by that route we know we are online.  The
	// bottom line is that if we get this far we know we are online so no checking is needed here. -dwiggins

	// Make sure the stream is connected and alive.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox ( bSilent );
	
		if (! SUCCEEDED ( hResult ) )
			return hResult;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Update the READ/WRITE state now that we have opened the mailbox.
	BOOL bIsReadOnly = m_pImap->IsReadOnly();

	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);
		hResult = E_FAIL;
	}
	else
	{
		// If the R/W status has changed, reflect this.
		if (bIsReadOnly != IsReadOnly())
		{
			SetReadOnly (bIsReadOnly);
			pImapCommand->SetReadOnly (bIsReadOnly);

			// Update mailbox display.
			g_theMailboxDirector.ImapNotifyClients (pImapCommand, CA_IMAP_PROPERTIES, NULL);
		}

		// If this is a MOVE, flag an error if the source mailbox is read-only.
		// If it's read-only, put up message.
		if ( !bSilent && IsReadOnly ())
		{
			CString err;
			err.Format ( CRString (IDS_ERR_IMAP_MBOX_RDONLY), pImapCommand->GetName () );
			ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, err);

			hResult = E_FAIL;
		}
	}


	if ( SUCCEEDED (hResult) )
	{
		// Ok, Send the command.
		hResult = m_pImap->CopyMessages (pUidlist, pDestination, pdwaNewUIDs);
	}

	// If no UIDPLUS data was returned tell our account object about it.
	if (SUCCEEDED(hResult) && pdwaNewUIDs)
	{
		CImapAccount		*pAccount = g_ImapAccountMgr.FindAccount(GetAccountID());
		if (pAccount)
		{
			pAccount->SetSupportsUIDPLUS(pdwaNewUIDs->GetSize() > 0);
		}
	}

	// If we failed here, display the server's message.
	if ( ( !SUCCEEDED (hResult) ) && !bSilent)
	{
		ShowLastImapError ();
	}
	else
	{
		if ( (SUCCEEDED (hResult)) && !Copy)
		{
			// NOTE: DOn't expunge them. Just flag them for deletion!!
			CUidMap			 mapUidsRemoved;
			hResult = m_pImap->UIDDeleteMessages (pUidlist, mapUidsRemoved, FALSE);

			// If we failed here, display the server's message.
			if ( (!SUCCEEDED(hResult)) && !bSilent)
			{
				ShowLastImapError ();
			}
		}
	}

	return hResult;
}



// UseFancyTrash [PUBLIC]
//
// Fancy trash is set on a per-account basis - ask the account for this info.
//
BOOL CImapMailbox::UseFancyTrash()
{
	BOOL bUseFancyTrash = FALSE;

	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( GetAccountID() );

	if (pAccount)
		bUseFancyTrash = pAccount->UseFancyTrash();

	return bUseFancyTrash;
}


// GetTrashMailboxMbxName
// FUNCTION
// Call the account of which this mailbox is a member to get the imapname
// of the one and only account's Trash mailbox.
// Return the MBX filename housing thye mailbox.
// END FUNCTION.
// NOTES
// If "CreateRemote" is TRUE, make sure the remote trash mailbox exists and that there
// is a local cache representation.
// END NOTES

BOOL CImapMailbox::GetTrashMailboxMbxName (CString& MbxFilePath, BOOL CreateRemote, BOOL bSilent /* = FALSE */)
{
	CImapAccount *pAccount;
	BOOL		  bResult = FALSE;
	CString		  path;

	// If errorn return an empty string.
	MbxFilePath.Empty();

	// Must have an accountID
	if (!m_AccountID)
		return FALSE;

	pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
		return FALSE;

	// Get the MBX filepath:
	bResult = pAccount->GetTrashLocalMailboxPath(path, CreateRemote, bSilent);

	// Copy path to outgoing if we succeeded.
	if (bResult)
		MbxFilePath = path;

	return bResult;
}


// FUNCTION
// Determine if this is the account's trash mailbox.
// END FUNCTION
BOOL CImapMailbox::IsImapTrashMailbox (LPCSTR pPathname)
{
	CImapAccount *pAccount;
	BOOL		  bResult = FALSE;
	CString		  path;

	// Sanity
	if (!pPathname)
		return FALSE;

	// Must have an accountID
	if (!m_AccountID)
		return FALSE;

	pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
		return FALSE;

	// Get the MBX filepath:
	bResult = pAccount->GetTrashLocalMailboxPath(path, FALSE);

	// If we found a name, compare..
	if (bResult)
	{
		bResult = (path.CompareNoCase (pPathname) == 0);
	}

	return bResult;
}


// MoveMsgsToTrash [PUBLIC]
//
HRESULT CImapMailbox::MoveMsgsToTrash(LPCSTR pUidlist,
									  CDWordArray *pdwaNewUIDs,
									  CUidMap& mapUidsActuallyRemoved,
									  CTocDoc **pTrashToc)
{
	// Sanity:
	//
	if (! (pUidlist && *pUidlist) )
		return S_OK;

	if (!m_pImap)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// If we're offline, don't initiate a connection.
	//
	if ( !IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		return HRESULT_MAKE_OFFLINE;
	}

	// 
	HRESULT hResult = S_OK;

	if (!IsSelected())
	{
		hResult = OpenMailbox (FALSE);

		if (! SUCCEEDED (hResult) )
		{
			return hResult;
		}
	}

	// Get trash mailbox for this account.
	//
	CImapAccount* pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);
	if (!pAccount)
		return E_FAIL;

	TCHAR pImapDelimiter = GetDelimiter ();
	CString		 strName;
	CString		 strFullName;
	pAccount->GetTrashMailboxName(strName, strFullName, &pImapDelimiter);

	// If no mailbox name, must ask user.
	//
	QCImapMailboxCommand *pTrashCommand = NULL;

	if (pImapDelimiter=='\0' || pImapDelimiter==' ')	// Make sure we have a delimiter of some kind -jdboyd
	{
		// the current mailbox doesn't have a delimiter.  Use the top level delimiter.
		
		// Unfortunately, the top level delimiter isn't kept up to date.  It's only set when
		// a refresh or some such thing happens.  I'm too scared to change this, so instead,
		// scan the existing mailboxes in this account, and return the first instance of a delimiter.
		pImapDelimiter = FigureOutDelimiter(pAccount);
	}

	if ( !strFullName.IsEmpty() )
	{
		// Find the mailbox in our local mailbox tree.
		//
		pTrashCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											strFullName,
											pImapDelimiter);
	}

#if 0 //
	if (!pTrashCommand)
	{
		// Try to create it on the server.
		//
		if ( !GetImapAccountMgr()->MailboxExistsOnServer (GetAccountID(), szImapTrashName) )
		{
		ACCOUNT_ID AccountId, LPCSTR pImapName)

		// Ask.
		//

		
		
	}
#endif

	// Still no trash command, give up.
	//
	if (!pTrashCommand)
	{
		CRString fmt (IDS_ERR_IMAP_INVALID_MBOXNAME);

		CString szText; szText.Format (fmt, strFullName);

		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, szText);

		return E_FAIL;
	}

	// Are source and destination the same?
	//
	BOOL bSameAsDest = strFullName.Compare (GetImapName()) == 0;

	// Do the transfer.
	//
	BOOL bXferSucceeded = FALSE;

	// If source and destination are the same, don't do the xfer part.
	//

	if ( !bSameAsDest )
	{
		hResult = XferMessagesOnServer (pUidlist,
										pdwaNewUIDs,
										strFullName,
										TRUE/*Copy*/,
										TRUE/*bSilent*/);
	}
	else
	{
		bXferSucceeded = TRUE;
		hResult = S_OK;
	}
		
	//
	if ( SUCCEEDED (hResult) )
	{	
		bXferSucceeded = TRUE;

		// First mark the messages to be deleted as seen.
		hResult = m_pImap->UIDAddFlaglist(pUidlist, "\\Seen", TRUE/*Silent*/);

		// Do a UID expunge. This will expunge only the messages in the uid list,
		// either using the UIDPLUS command (if server supports it), or by
		// unflagging currently \\DELETED ones first.
		//
		// Since UIDExpunge() currently just does an EXPUNGE and acts on all deleted messages
		// (not just those deleted by trashing).  The long term solution is better deletion
		// handling, but for now we just don't do the EXPUNGE if fancy trash mode is off. -dwiggins
		hResult = m_pImap->UIDDeleteMessages(pUidlist, mapUidsActuallyRemoved, FALSE/*Expunge*/);

		// If we didn't succeed, we must copy back xferred messages!!! $$$$$$
		//
	}

	// If we did the transfer, go update the destination mailbox if it's open.
	//
	if (!bSameAsDest && bXferSucceeded)
	{
		// See if the TOC open on the desktop.
		//
		*pTrashToc = GetToc(pTrashCommand->GetPathname(), NULL, TRUE, TRUE);

		if (*pTrashToc && (*pTrashToc)->GetView())
		{
			BOOL ViewNeedsUpdate = FALSE;		
			pTrashCommand->Execute (CA_IMAP_RESYNC, &ViewNeedsUpdate);
		}
	}

	if (m_pTocDoc)
	{
		DoManualResync(m_pTocDoc);
	}

	// We just did a COPY so see if our UIDPLUS status has changed.
	pAccount->WarnIfUIDPLUSStatusChanged();

	return hResult;
}


//
//	CImapMailbox::MoveMsgsToOrFromJunk() [PUBLIC]
//
//	Moves the specified messages to the IMAP account's Junk mailbox.
//
HRESULT CImapMailbox::MoveMsgsToOrFromJunk(bool bToJunk,
										   LPCSTR pUidlist,
										   CDWordArray *pdwaNewUIDs,
										   CUidMap &mapUidsActuallyRemoved,
										   CTocDoc **pTargetToc)
{
	if (!pUidlist || !*pUidlist)
	{
		return S_OK;
	}

	if (!m_pImap)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// If we're offline, don't initiate a connection.
	if (!IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		return HRESULT_MAKE_OFFLINE;
	}

	HRESULT hResult = S_OK;

	// Open the source mailbox.
	if (!IsSelected())
	{
		hResult = OpenMailbox(FALSE);

		if (!SUCCEEDED(hResult))
		{
			return hResult;
		}
	}

	CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_AccountID);
	if (!pAccount)
	{
		return E_FAIL;
	}

	CString					 strImapName;
	QCImapMailboxCommand	*pCommand = NULL;

	if (bToJunk)
	{
		// Destination is Junk mailbox for this account.
		pCommand = pAccount->GetJunkMailbox(strImapName);
	}
	else
	{
		// Destination is Inbox for this account.
		pCommand = pAccount->GetInMailbox(strImapName);
	}

	if (!pCommand)
	{
		return E_FAIL;
	}

	// Should we move the message?  Don't move if we are junking and this is the junk mailbox or
	// if we are not junking and this isn't the junk mailbox.
	BOOL bShouldMove = TRUE;
	BOOL bIsJunk = IsJunk(GetImapName());
	if ((bToJunk && bIsJunk) || (!bToJunk && !bIsJunk))
	{
		bShouldMove = FALSE;
	}

	// Do the transfer.
	BOOL bXferSucceeded = FALSE;

	// If source and destination are the same, don't do the xfer part.
	if (bShouldMove)
	{
		hResult = XferMessagesOnServer(pUidlist,
										pdwaNewUIDs,
										strImapName,
										TRUE,				// Do a COPY only.
										TRUE);				// bSilent.
	}
	else
	{
		bXferSucceeded = TRUE;
		hResult = S_OK;
	}
		
	//
	if (bShouldMove && SUCCEEDED(hResult))
	{	
		bXferSucceeded = TRUE;

		// Do a UID expunge. This will expunge only the messages in the uid list,
		// either using the UIDPLUS command (if server supports it), or by
		// unflagging currently \\DELETED ones first.

		// Since UIDExpunge() currently just does an EXPUNGE and acts on all deleted messages
		// (not just those deleted by junking).  The long term solution is better deletion
		// handling, but for now we just don't do the EXPUNGE if fancy trash mode is off. -dwiggins
		hResult = m_pImap->UIDDeleteMessages(pUidlist, mapUidsActuallyRemoved, FALSE/*Expunge*/);

		// If the delete succeeded mark the messages as seen.  This is an option on the Mac but for
		// now we always mark deleted messages as seen.
		if (hResult == S_OK)
		{
			m_pImap->UIDAddFlaglist(pUidlist, "(\\Seen)");
		}

		// Get the TOC.  Note that we need to open the TOC to complete the transfer
		// process with the UIDPLUS data so open it if it isn't already open.
		*pTargetToc = GetToc(pCommand->GetPathname(), NULL/*Name*/, TRUE/*HeaderOnly*/, FALSE/*OnlyIfLoaded*/);
		if (*pTargetToc && (*pTargetToc)->GetView())
		{
			// If the TOC is open update it.
			BOOL	 bViewNeedsUpdate = FALSE;		
			pCommand->Execute(CA_IMAP_RESYNC, &bViewNeedsUpdate);
		}
	}

	if (m_pTocDoc)
	{
		DoManualResync(m_pTocDoc);
	}

	// We just did a COPY so see if our UIDPLUS status has changed.
	pAccount->WarnIfUIDPLUSStatusChanged();

	return hResult;
}

//
//	CImapMailbox::ReadyToAutoExpunge()
//
//	Returns TRUE if conditions are right to auto expunge, FALSE otherwise.
//
BOOL CImapMailbox::ReadyToAutoExpunge()
{
	int			 iPercent = -1;
	int			 iSetting = -1;

	CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_AccountID);
	if (!pAccount)
	{
		return FALSE;
	}

	iSetting = pAccount->GetAutoExpungeSetting(&iPercent);

	if (iSetting == IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS)
	{
		return TRUE;
	}
	else if (iSetting == IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT)
	{
		if (iPercent < 0)
		{
			// This really shouldn't happen but if we can't determine a valid percent don't go on.
			return FALSE;
		}

		if (m_pTocDoc)
		{
			unsigned int	 iTotalSummaries = m_pTocDoc->NumSums();
			unsigned int	 iDeletedSummaries = 0;
			CSumList &		 listSums = m_pTocDoc->GetSumList();
			POSITION		 pos = listSums.GetHeadPosition();
			POSITION		 posNext = NULL;
			CSummary		*pSum = NULL;

			for (posNext = pos; pos; pos = posNext)
			{
				pSum = listSums.GetNext(posNext);
				if (pSum && pSum->IsIMAPDeleted())
				{
					++iDeletedSummaries;
					if (((iDeletedSummaries * 100) / iTotalSummaries) >= (unsigned int)iPercent)
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

// DeleteMessagesFromServer [PUBLIC]
// FUNCTION
// Set the delete flags and (optionally) expunge the mailbox.
// END FUNCTION
HRESULT CImapMailbox::DeleteMessagesFromServer (LPCSTR pUidList, BOOL Expunge, BOOL bSilent /* = FALSE */)
{
	HRESULT			hResult = S_OK;

	// Sanity.
	if (!pUidList)
		return E_INVALIDARG;

	// Make sure the stream is connected and alive.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox ( bSilent );

		if (! SUCCEEDED (hResult) )
			return hResult;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Update the READ/WRITE state now that we have opened the mailbox.
	BOOL bIsReadOnly = m_pImap->IsReadOnly();


	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the R/W status has changed, reflect this.
	if (bIsReadOnly != IsReadOnly())
	{
		SetReadOnly (bIsReadOnly);
		pImapCommand->SetReadOnly (bIsReadOnly);

		// Update mailbox display.
		g_theMailboxDirector.ImapNotifyClients (pImapCommand, CA_IMAP_PROPERTIES, NULL);
	}

	// If it's read-only, put up message.
	if (!bSilent && IsReadOnly ())
	{
		CString err;
		err.Format ( CRString (IDS_ERR_IMAP_MBOX_RDONLY), pImapCommand->GetName () );
		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, err);

		hResult = E_FAIL;
	}
	else
	{
		// Ok, Send the command.

		// First mark the messages to be deleted as seen.
		hResult = m_pImap->UIDAddFlaglist(pUidList, "\\Seen", TRUE/*Silent*/);

		// Now do the delete.
		CUidMap			 mapUidsRemoved;
		hResult = m_pImap->UIDDeleteMessages (pUidList, mapUidsRemoved, Expunge);
	}

	// If we failed here, display the server's message.
	if (!SUCCEEDED(hResult))
	{
		if (!bSilent)
		{
			ShowLastImapError ();
		}
	}

	return hResult;
}

	

// UnDeleteMessagesFromServer [PUBLIC]
// FUNCTION
// Remove the delete flags from the messages.
// END FUNCTION
HRESULT CImapMailbox::UnDeleteMessagesFromServer (LPCSTR pUidList, BOOL bSilent /* = FALSE */)
{
	HRESULT			hResult = E_FAIL;

	// Sanity.
	if (!pUidList)
		return E_INVALIDARG;

	// Make sure the stream is connected and alive.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox ( bSilent );

		if ( !SUCCEEDED (hResult) )
			return hResult;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Update the READ/WRITE state now that we have opened the mailbox.
	BOOL bIsReadOnly = m_pImap->IsReadOnly();


	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the R/W status has changed, reflect this.
	if (bIsReadOnly != IsReadOnly())
	{
		SetReadOnly (bIsReadOnly);
		pImapCommand->SetReadOnly (bIsReadOnly);

		// Update mailbox display.
		g_theMailboxDirector.ImapNotifyClients (pImapCommand, CA_IMAP_PROPERTIES, NULL);
	}

	// If it's read-only, put up message.
	if (!bSilent && IsReadOnly ())
	{
		CString err;
		err.Format ( CRString (IDS_ERR_IMAP_MBOX_RDONLY), pImapCommand->GetName () );
		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, err);

		return E_FAIL;
	}

	// Ok, Send the command.
	hResult = m_pImap->UIDUnDeleteMessages (pUidList);

	// If we failed here, display the server's message.
	if ( !SUCCEEDED(hResult) )
	{
		if (!bSilent)
		{
			ShowLastImapError ();
		}
	}

	return hResult;
}




// Expunge [PUBLIC]
// FUNCTION
// Wrapper around CImapConnection's expunge.
// END FUNCTION
HRESULT CImapMailbox::Expunge ()
{
	HRESULT hResult = E_FAIL;

	// Make sure the stream is connected and alive.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox ( FALSE );

		if (! SUCCEEDED (hResult) )
			return hResult;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	CUidMap			 mapUidsRemoved;
	return m_pImap->Expunge(mapUidsRemoved);
}




// UIDExpunge [PUBLIC]
// FUNCTION
// Wrapper around CImapConnection's expunge.
// END FUNCTION
HRESULT CImapMailbox::UIDExpunge (LPCSTR pUidList, CUidMap& mapUidsActuallyRemoved)
{
	HRESULT hResult = E_FAIL;

	// Make sure the stream is connected and alive.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox ( FALSE );

		if (! SUCCEEDED (hResult) )
			return hResult;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	return m_pImap->UIDExpunge (pUidList, mapUidsActuallyRemoved);
}


//
//	CImapMailbox::SetRemoteState()
//
//	Calls the appropriate function to set/unset the specified flag for the specified messages
//	on the server.
//
BOOL CImapMailbox::SetRemoteState(CString& szUidList, char State, BOOL bUnset)
{
	if (!IsSelected())
	{
		return FALSE;
	}

	// Set the state. Note: Do it silently.
	if (State == MS_READ)
	{
		if (bUnset)
			return SUCCEEDED(ImapUnsetSeenFlag(szUidList, TRUE));
		else
			return SUCCEEDED(ImapSetSeenFlag(szUidList, TRUE));
	}
	else if (State == MS_UNREAD)
	{
		if (bUnset)
			return SUCCEEDED(ImapSetSeenFlag(szUidList, TRUE));
		else
			return SUCCEEDED(ImapUnsetSeenFlag(szUidList, TRUE));
	}
	else if (State == MS_REPLIED)
	{
		if (bUnset)
			return SUCCEEDED(ImapUnsetAnsweredFlag(szUidList, TRUE));
		else
			return SUCCEEDED(ImapSetAnsweredFlag(szUidList, TRUE));
	}

	// If we get here, it's not a state IMAP can set. That's OK.
	return TRUE;
}


// ImapSetSeenFlag [PUBLIC]
//
// Set seen flag
//
HRESULT CImapMailbox::ImapSetSeenFlag   (LPCSTR pUidList,  BOOL bSilent /* = FALSE */)
{
	return ModifyRemoteFlags (pUidList, IMFLAGS_SEEN, TRUE, bSilent);
}




// ImapUnsetSeenFlag [PUBLIC]
//
// Remove seen flag
//
HRESULT CImapMailbox::ImapUnsetSeenFlag (LPCSTR pUidList,  BOOL bSilent /* = FALSE */)
{
	return ModifyRemoteFlags (pUidList, IMFLAGS_SEEN, FALSE, bSilent);
}



// ImapSetAnsweredFlag [PUBLIC]
//
// Set Answered flag
//
HRESULT CImapMailbox::ImapSetAnsweredFlag   (LPCSTR pUidList,  BOOL bSilent /* = FALSE */)
{
	return ModifyRemoteFlags (pUidList, IMFLAGS_ANSWERED, TRUE, bSilent);
}



// ImapUnsetAnsweredFlag [PUBLIC]
//
// Remove answered flag.
//
HRESULT CImapMailbox::ImapUnsetAnsweredFlag   (LPCSTR pUidList,  BOOL bSilent /* = FALSE */)
{
	return ModifyRemoteFlags (pUidList, IMFLAGS_ANSWERED, FALSE, bSilent);
}



// ModifyRemoteFlags [PUBLIC]
// FUNCTION
// Does the real work of setting message (IMAP system) flags.
// The flags are bitflag-combinations of the IMFLAGS_* bitflags defined in summary.h.
// Can set multiple flags in a single call.
//
//If "bSet" is FALSE, the flags are REMOVED instead of being set.
//
// END FUNCTION
HRESULT CImapMailbox::ModifyRemoteFlags (LPCSTR pUidList, unsigned ulFlags, BOOL bSet,  BOOL bSilent /* = FALSE */)
{
	HRESULT			hResult = E_FAIL;
	BOOL	bWeOpenedOurConnection = FALSE;

	// Sanity.
	if (!(pUidList && ulFlags))
		return E_INVALIDARG;

#if 0 // JOK 
	// If we're offline, don't initiate a connection.
	//
	if ( !IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		return HRESULT_MAKE_OFFLINE;
	}
#endif // JOK 

	// Make sure the stream is connected and alive.
	//
	if ( !IsSelected() )
	{
		hResult = OpenMailbox (bSilent);

		if (! SUCCEEDED (hResult) )
			return E_FAIL;

		bWeOpenedOurConnection = TRUE;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the mailbox is read-only, put up message.
	if (!bSilent && IsReadOnly ())
	{
		CString err;
		err.Format ( CRString (IDS_ERR_IMAP_MBOX_RDONLY), pImapCommand->GetName () );
		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, err);

		return E_FAIL;
	}

	// Format the IMAP char-format flaglist.
	CString szFlaglist;

	ImapBitflagsToFlaglist (ulFlags, szFlaglist);

	if (szFlaglist.IsEmpty())
	{
		ASSERT (0);
		return E_FAIL;
	}	

	// Send the IMAP command now:
	if (bSet)
		hResult = m_pImap->UIDAddFlaglist (pUidList, szFlaglist, bSilent);
	else
		hResult = m_pImap->UIDRemoveFlaglist (pUidList, szFlaglist, bSilent);

	// If we failed here, display the server's message.
	if ( !SUCCEEDED(hResult) )
	{
		if (!bSilent)
		{
			ShowLastImapError ();
		}
	}

	return hResult;
}





//
// FUNCTION
// Send a message from a local mailbox up to a remote IMAP mailbox.
// We are the target mailbox.
// Format attachments into a valid RFC822/MIME message.
// END FUNCTION

// NOTES
// Assume that all members of "this" are valid. Typically we are being called from
// within a CTocDoc and this object is the CTocDoc's contained imap object.
// NOTE: Do NOT delete target message or components.

// NOTE: If the server supports Optimize-1 and we can get a destination UID, 
//       return the new UID in "*pNewUid".
// END NOTES

HRESULT CImapMailbox::AppendMessageFromLocal (CSummary *pSum, unsigned long *pNewUid, BOOL bSilent)
{
	HRESULT			hResult = E_FAIL;
	CMessageDoc* 	pMsgDoc = NULL;
	BOOL			MustCloseMsgDoc = FALSE;	
	CTocDoc			*pSrcToc;

	// Sanity
	if ( !(pSum && pNewUid) )
		return E_INVALIDARG;

	// Initialize
	*pNewUid = 0;

	// Get the source TOC.
	pSrcToc = pSum->m_TheToc;
	if (pSrcToc == NULL)
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// Disable preview.
//	pSrcToc->SetPreviewableSummary (NULL);

	// If the source toc is an IMAP mailbox. verify it's contained IMPA maibox object.
	if (pSrcToc->IsImapToc())
	{
		if (!pSrcToc->m_pImapMailbox)
		{
			ASSERT ( 0 );
			return E_FAIL;
		}
	}

	// Make sure we can open the remote connection first.
	//
	if ( !IsSelected() )
	{
		hResult = OpenMailbox ( bSilent );

		if (! SUCCEEDED (hResult) )
			return hResult;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Update the READ/WRITE state now that we have opened the mailbox.
	BOOL bIsReadOnly = m_pImap->IsReadOnly();


	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the R/W status has changed, reflect this.
	if (bIsReadOnly != IsReadOnly())
	{
		SetReadOnly (bIsReadOnly);
		pImapCommand->SetReadOnly (bIsReadOnly);

		// Update mailbox display.
		g_theMailboxDirector.ImapNotifyClients (pImapCommand, CA_IMAP_PROPERTIES, NULL);
	}

	// If it's read-only, put up message.
	if (!bSilent && IsReadOnly ())
	{
		CString err;
		err.Format ( CRString (IDS_ERR_IMAP_MBOX_RDONLY), pImapCommand->GetName () );
		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, err);

		return E_FAIL;
	}

	// Instantiate an append object to do the grunge work.
	CDWordArray		 dwaNewUIDs;
	CImapAppend		 Append(m_pImap, NULL, pSum, 0, &dwaNewUIDs);

	// Map local summary flags to IMAP flags.
	// If the summary is from the local outbox, set SEEN flag.
	//
	int				State	= pSum->m_State;
	unsigned long	Imflags = 0;

	if (pSum->m_TheToc && pSum->m_TheToc->m_Type == MBT_OUT)
	{
		Imflags |= IMFLAGS_SEEN;
	}
	else if (State == MS_READ)
	{
		Imflags |= IMFLAGS_SEEN;
	}
	else if (State == MS_REPLIED || State == MS_FORWARDED || State == MS_REDIRECT)
	{
		Imflags |= IMFLAGS_ANSWERED;
	}

	Append.SetFlags (Imflags);

	// With the new IMAP local/cache/online model the summary is actually in the destination
	// (IMAP) TOC at this point, but the attachment is in the POP attachment directory.  The
	// previous code actually checked to see if the summary was in a POP TOC and used the
	// attachment directory for the IMAP mailbox if it wasn't.  The cold, hard truth is that we
	// would probably be in trouble if we got here and the source message was originally in an
	// IMAP mailbox so I have taken that check out. -dwiggins
	char DirName[_MAX_PATH+1];
	GetIniString(IDS_INI_AUTO_RECEIVE_DIR, DirName, sizeof(DirName));
	if( strcmp(DirName, "") != 0)
		Append.SetSrcAttachDir ( DirName );
	else
		Append.SetSrcAttachDir ( (LPCSTR) ( EudoraDir + CRString(IDS_ATTACH_FOLDER) ) );

	// Tell the append object to use the target's mailbox directory if it need to spool stuff.
	Append.SetSpoolDir (m_Dirname);
 
	// Get the MBX file stub.
	MustCloseMsgDoc = FALSE;
	pMsgDoc = pSum->FindMessageDoc();
	if (!pMsgDoc)
	{
		pMsgDoc = pSum->GetMessageDoc();
		MustCloseMsgDoc = TRUE;
	}

	hResult = S_OK;

	if (pMsgDoc)
	{
		// Copy this to the Append object.
		// Note: GetFullMessage returns allocated text that must be freed!!
		LPTSTR pText = pMsgDoc->GetFullMessage ();
		if (!pText)
		{
			ASSERT (0);
			hResult = E_FAIL;
		}
		else
		{
			// This copies the text, so we can safeky free pText.
			Append.SetMbxStub (pText);

			delete[] pText;
			pText = NULL;
		}

		// Did we create the message doc?
		if (MustCloseMsgDoc)
		{
			pSum->NukeMessageDocIfUnused();
			pMsgDoc = NULL;

		}
	}

	// Make sure.
	pMsgDoc = NULL;


	// Call over to append.cpp to do the dirty work of appending. This may 
	// involve spooling the mesage to a temporary file first, for example if we are doing this
	// in the background.
	// For now, spool and do in foreground.

	if ( SUCCEEDED (hResult) )
	{
		//
		// Copy the summary's m_Seconds in case the message doesn't have a date field.
		//
		Append.m_Seconds = pSum->m_Seconds ;/*+ 60*pSum->m_TimeZoneMinutes;*/
	
		hResult = Append.AppendMessage (TRUE, FALSE);

		if (dwaNewUIDs.GetCount() == 1)
		{
			*pNewUid = dwaNewUIDs.GetAt(0);
		}
	}

	return hResult;
}





//
// FUNCTION
// Download message text directly from a remote mailbox and append it up to a destination
// remote IMAP mailbox on a different server.
// END FUNCTION

// NOTES
// Assume that all members of "this" are valid. Typically we are being called from
// within a CTocDoc and this object is the CTocDoc's contained imap object.

// Note: We are the TARGET mailbox!!!

// NOTE: If the server supports Optimize-1 and we can get a destination UID, 
//       return the new UID in "*pNewUid".
// END NOTES

HRESULT CImapMailbox::AppendMessageAcrossRemotes (CSummary *pSum, unsigned long *pNewUid, BOOL bSilent)
{
	HRESULT			hResult = E_FAIL;
	CTocDoc			*pSrcToc;
	BOOL			bWeOpenedSrcConnection = FALSE, bWeOpenedOurConnection = FALSE;

	// Sanity
	if ( !(pSum && pNewUid) )
		return E_INVALIDARG;

	// Initialize
	*pNewUid = 0;

	// Get the source TOC.
	pSrcToc = pSum->m_TheToc;
	if (pSrcToc == NULL)
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// Disable preview.
//	pSrcToc->SetPreviewableSummary (NULL);

	// The source TOC is also an IMAP mailbox. Verify it's contained IMAP maibox object.
	if (!pSrcToc->m_pImapMailbox)
	{
		ASSERT ( 0 );
		return E_FAIL;
	}

	//
	hResult = S_OK;

	// Make sure we can open the remote connection to our mailbox.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox (TRUE);

		if (! SUCCEEDED (hResult) )
			return E_FAIL;

		bWeOpenedOurConnection = TRUE;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Open to source mailbox as well.
	if ( !pSrcToc->m_pImapMailbox->IsSelected() )
	{
		hResult = pSrcToc->m_pImapMailbox->OpenMailbox (TRUE);

		if (!SUCCEEDED (hResult) )
			return hResult;

		bWeOpenedSrcConnection = TRUE;
	}

	// Source toc MUST have an IMAP agent as well.
	//
	if ( !pSrcToc->m_pImapMailbox->GetImapAgent() )
	{
		ASSERT (0);
		return E_FAIL;
	}


	// Update out READ/WRITE state now that we have opened it.
	BOOL bIsReadOnly = IsReadOnly();

	// We need this a couple of times below:
	QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

	if (!pImapCommand)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the R/W status has changed, reflect this.
	if (bIsReadOnly != IsReadOnly())
	{
		SetReadOnly (bIsReadOnly);
		pImapCommand->SetReadOnly (bIsReadOnly);

		// Update mailbox display.
		g_theMailboxDirector.ImapNotifyClients (pImapCommand, CA_IMAP_PROPERTIES, NULL);
	}

	// If it's read-only, put up message.
	if (!bSilent && IsReadOnly ())
	{
		CString err;
		err.Format ( CRString (IDS_ERR_IMAP_MBOX_RDONLY), pImapCommand->GetName () );
		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, err);

		return E_FAIL;
	}

	// Call over to append.cpp to do the dirty work of appending. This may 
	// involve spooling the mesage to a temporary file first, for example if we are doing this
	// in the background.
	// For now, spool and do in foreground.

	CDWordArray		 dwaNewUIDs;
	CImapAppend	Append(m_pImap, pSrcToc->m_pImapMailbox->m_pImap, pSum, pSum->GetHash(), &dwaNewUIDs);

	Append.m_Seconds = pSum->m_Seconds;

	hResult = Append.AppendMessageAcrossRemotes ();

	if (dwaNewUIDs.GetCount() == 1)
	{
		*pNewUid = dwaNewUIDs.GetAt(0);
	}

	return hResult;
}



// FindAll [PUBLIC]
// FUNCTION
// Do a server search of all messages for a match of the given string.
// Search the entire text.
// END FUNCTION
//
HRESULT CImapMailbox::FindAll (LPCSTR pSearchString, CString& szImapUidMatches)
{
	HRESULT	hResult = E_FAIL;
	BOOL	bWeOpenedOurConnection = FALSE;

	// Do this first.
	szImapUidMatches.Empty();
	
	// Must have a search string.
	if (! (pSearchString && *pSearchString) )
		return E_INVALIDARG;

	// Make sure we can open the remote connection to our mailbox.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox (TRUE);

		if (!SUCCEEDED (hResult) )
			return hResult;

		bWeOpenedOurConnection = TRUE;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Call CImap to do the work.
	hResult = m_pImap->UIDFindText (pSearchString, 1, 0xFFFFFFFF, szImapUidMatches);

	// If user cancelled, return that to caller in the HRESULT:
	//
	if (EscapePressed())
	{
		hResult = HRESULT_MAKE_CANCEL;
	}

	// If we opened the connection, close it back.
	if (bWeOpenedOurConnection)
		Close();

	return hResult;
}



//
// Wrapper around CImapConnection:UIDFind.
//
HRESULT CImapMailbox::UIDFind (LPCSTR pHeaderList, BOOL bBody, BOOL bNot, LPCSTR pSearchString,
			LPCSTR pUidStr, CString& szResults)
{
	HRESULT	hResult = E_FAIL;
	BOOL	bWeOpenedOurConnection = FALSE;

	// Do this first.
	szResults.Empty();
	
	// Must have a search string.
	if (! (pSearchString && *pSearchString) )
		return E_INVALIDARG;

	// Make sure we can open the remote connection to our mailbox.
	if ( !IsSelected() )
	{
		hResult = OpenMailbox (TRUE);

		if (!SUCCEEDED (hResult) )
			return hResult;

		bWeOpenedOurConnection = TRUE;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Call CImap to do the work.
	hResult = m_pImap->UIDFind (pHeaderList, bBody, bNot, pSearchString, pUidStr, szResults);

	// If we opened the connection, close it back.
	if (bWeOpenedOurConnection)
		Close();

	return hResult;
}





// DoOnServerSearch [PUBLIC]
//
// Pass to the CImapConnection.
// Note: If "pUidRange" is NULL, all messages on the server are searched.
//
// NOTE: 12/23/98 - Modified to do search in chunks.
//
HRESULT CImapMailbox::DoOnServerSearch (MultSearchCriteria* pMultiSearchCriteria, CString& szResults, LPCSTR pUidRange /* = NULL */)
{
	HRESULT	hResult = E_FAIL;
	BOOL	bMustCloseConnection = FALSE;

	// Do this first.
	szResults.Empty();
	
	// Must have a search string.
	if (! pMultiSearchCriteria )
		return E_INVALIDARG;

	// Make sure we can open the remote connection to our mailbox.
	if (!IsSelected())
	{
		int		 iConnectionState = GetConnectionState(bDontAllowOffline);
		if (iConnectionState == iStateGoOnlineForThis)
		{
			// The connection is being allowed for this action only: note that we must close the connection
			// when we are done.
			bMustCloseConnection = TRUE;
		}
		else if (iConnectionState == iStateStayOfflineDisallow)
		{
			// No connection is being created and this action cannot be done offline: bail out.
			return HRESULT_MAKE_OFFLINE;
		}

		hResult = OpenMailbox (TRUE);

		if (!SUCCEEDED(hResult))
		{
			// Close the connection if it was opened just for us.
			if (bMustCloseConnection)
			{
				Close();
			}
			return hResult;
		}
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Do it in chunks.
	//
	hResult = DoOnServerSearchInChunks_ (pMultiSearchCriteria, szResults, pUidRange);

	// Close the connection if it was opened just for us.
	if (bMustCloseConnection)
	{
		Close();
	}

	return hResult;
}



// DoOnServerSearchKnown [PUBLIC]
//
BOOL CImapMailbox::DoOnServerSearchKnown (MultSearchCriteria* pMultiSearchCriteria, CString& szResults)
{
	// If this is an as yet un-opened mailbox, then no matches found.
	//
	if (m_UIDHighest == 0)
	{
		return TRUE;
	}

	// Search up to what summary we've got.
	//
	CString szRange; szRange.Format ("1:%lu", m_UIDHighest);

	return DoOnServerSearch (pMultiSearchCriteria, szResults, szRange);
}




// Is this a criterion that IMAP can handle??
//
// STATIC 
BOOL CImapMailbox::ImapServerCanSearch (SearchCriteria *pCurSearchCriteria)
{
	// Ask CSeasrchUtil:
	//
	return CSearchUtil::ImapServerCanSearch (pCurSearchCriteria);
}




// DoOnServerSearchInChunks_ [PRIVATE]
//
// NOTE: 12/23/98 - Modified to do search in chunks.
// Assume all sanity checks have already been made.
//
HRESULT CImapMailbox::DoOnServerSearchInChunks_ (MultSearchCriteria* pMultiSearchCriteria, CString& szResults, LPCSTR pUidRange /* = NULL */)
{
	if (!m_pImap)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Do this first.
	szResults.Empty();

	// Make sure we have messages on the server.
	//
	Ping();
	if (m_NumberOfMessages == 0)
		return S_OK;

	// If in main thread, put up progress bar.
	//
	BOOL bWasInProgress		= FALSE;
	BOOL bWePutProgressUp	= FALSE;
	if ( IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		CString buf;
		buf.Format ("Searching mailbox %s", m_FriendlyName);

		MainProgress(buf);

		Progress(0, buf, m_NumberOfMessages);

		bWePutProgressUp = TRUE;
	}

	// Estimate how many search iterations we'd have.
	//
	int dm = m_NumberOfMessages / MAX_UIDS_PER_COMMAND + 1;

	// Expand the given uid range into a comma-separated list.
	// If pUidRange is NULL, then we expand it to ALL uid's.
	// Note: ExpandUidRange return an allocated array - MUST FREE IT.
	//
	LPSTR pszRange = ExpandUidRange (pUidRange);

	BOOL bResult = FALSE;

	// Succeed if we did at least one a search.
	LPCSTR pNext = pszRange;
	CString szChunk;
	CString szCurrentResults;

	while (pNext)
	{
		szChunk.Empty();
		szCurrentResults.Empty();

		pNext = GetNextUidChunk (pNext, szChunk);

		if ( !szChunk.IsEmpty() && 
			( SUCCEEDED(m_pImap->DoOnServerSearch (pMultiSearchCriteria, szCurrentResults, szChunk)) ) )
		{
			// Found any this trip?
			if ( !szCurrentResults.IsEmpty() )
			{
				if ( !szResults.IsEmpty() )
					szResults += comma;

				szResults += szCurrentResults;
			}

			bResult = TRUE;
		}
		else
		{
			break;
		}

		ProgressAdd(dm);
	}

	if (pszRange)
		delete[] pszRange;

	if ( IsMainThreadMT() )
		TerminateProgress (bWePutProgressUp, bWasInProgress);

	return bResult ? S_OK : E_FAIL;
}





// 
// Search a comma-separated list of UID's for one matching "ulUid".
//
#define BSIZE  1024

BOOL CImapMailbox::Matched (CString& theList, unsigned long ulUid)
{
	BOOL		bResult = FALSE;
	CString		str;
	CString		buf;
	int			nc;
	unsigned long Uid;

	// Must have a non-zero UID.
	if (ulUid == 0)
		return FALSE;

	// Copy:
	str = theList;

	while (!str.IsEmpty())
	{
		nc = str.Find (comma);
		if (nc < 0)
		{
			// May still have a match
			Uid = atol (str);
			if (Uid == ulUid)
			{
				bResult = TRUE;
			}

			// In any case, get out..
			break;
		}
		else if (nc == 0)
		{
			// Skip;
			str = str.Mid (1);
		}
		else
		{
			// Test.
			buf = str.Left (nc);
			str = str.Mid (nc + 1);

			// Do we have a match??
			buf.TrimRight ();
			buf.TrimLeft  ();
			
			Uid = atol (buf);
			if (Uid == ulUid)
			{
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}




// FUNCTION
// Wrapper around CImapConnection's FetchAttachmentContentsToFile.
// This will check to see if we are connected and selected and if not, 
// open the mailbox first.
// END FUNCION

HRESULT CImapMailbox::FetchAttachmentContentsToFile (IMAPUID uid, char *sequence, const char *Filename, short encoding, LPCSTR pSubtype /* = NULL */)
{
	HRESULT hResult = S_OK;
	BOOL bSilent;

	// If we are in main thread, put up a progress bar.
	bSilent = ! ::IsMainThreadMT();

	// Make sure we can open the remote connection to our mailbox.
	//
	BOOL bWeOpenedConnection = FALSE;

	if ( !IsSelected() )
	{
		hResult = OpenMailbox (bSilent);

		if ( !SUCCEEDED (hResult) )
			return hResult;

		bWeOpenedConnection = TRUE;
	}

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	hResult = m_pImap->FetchAttachmentContentsToFile (uid, sequence, Filename, encoding, pSubtype);

	// WE're getting closer to downloading all attachments.
	//
	if (SUCCEEDED (hResult) && m_pTocDoc)
	{
		CSumList &	listSums = m_pTocDoc->GetSumList();
		CSummary *	pSum = listSums.GetByUid(uid);
		if (pSum)
		{
			ASSERT (pSum->m_nUndownloadedAttachments > 0);

			// If the TOC has to be rebuilt, it's possible to get an incorrect value for
			// the number of undownloaded attachments. There's nothing we can do about that. 
			//
			if (pSum->m_nUndownloadedAttachments > 0)
			{
				pSum->m_nUndownloadedAttachments--;

				// Usage Statistics Support STARTS here
				BOOL bUpdateStats = FALSE;
				// Check if Stats need to accounted only for INBOX or not
				if (m_pImap && m_pImap->IsImapStatisticsForInboxOnly()) 
				{
					// Tracking IMAP stats only for INBOX
					if(m_pImap->GetImapName() && !CRString(IDS_IMAP_RAW_INBOX_NAME).CompareNoCase(m_pImap->GetImapName()))
						bUpdateStats = TRUE;					
				}
				else
				{
					// Tracking stats for all the IMAP mailboxes ..
					bUpdateStats = TRUE;
				}
				
				if (bUpdateStats)
				{
					// Update the Usage Statistics for Attachment received
					UpdateNumStat(US_STATRECEIVEDATTACH,1,pSum->m_Seconds + (pSum->m_TimeZoneMinutes * 60));
				}
				// Usage Statistics Support ENDS here

				if (pSum->m_nUndownloadedAttachments == 0)
					pSum->RedisplayField (FW_SERVER);
			}
		}
	}

	if (bWeOpenedConnection)
		Close();

	return hResult;
}




// FUNCTION
// So it can be called from imaptoc
// END FUNCION

HRESULT CImapMailbox::DownloadAttachment ( LPCSTR pFilePath)
{
	return ImapDownloaderFetchAttachment (pFilePath);
}




// ============== Public Interfacs to JOURNAL object methods =================/ 

// Public interfaces to journal and resyncer object methods.	
BOOL CImapMailbox::QueueRemoteXfer (IMAPUID sourceUID, ACCOUNT_ID DestAccountID, LPCSTR pDestImapMboxName, BOOL Copy)
{
	return m_Journaler.QueueRemoteXfer (sourceUID, DestAccountID, pDestImapMboxName, Copy);
}




// ImapAppendSumList [PRIVATE]
//
// FUNCTION
// For each CImapSum object in "pImapSumList", create a CSummary and add it to the summaries currently in the TOC,
// updating any appropriate TOC attributes.
//
// NOTE: Also add it to pRefSumList BUT, only as a reference. Caller MUST NOT delete the object memory
// when deleting the list itself!!!
//
// END SUMMARY
//
// HISTORY
// (5/14/98 - JOK: Added bFromTmpMbx. If TRUE, then any summaries that
//		have pointers to an offset in an MBX file point to the temporary
//		MBX file. The data must be copied to the main MBX file.
//	(5/14/98 (JOK)) - Added pRefSumList.
//
// END HISTORY
//
// NOTE: This is NOT thread safe!!! This is done in the main thread.
//
// NOTE: This DOES NOT remove entries from the (in-parameter) pImapSumList.
//

BOOL CImapMailbox::ImapAppendSumList (CTocDoc* pTocDoc, CImapSumList* pImapSumList, CSumList* pRefSumList, BOOL bFromTmpMbx)
{
	POSITION pos, next;
	BOOL	 bResult = TRUE;

	// Sanity. Must have a TOC
	if (!pTocDoc)
		return FALSE;

	// If no CImapSum summary list, then nothing to do.
	// Note: pRefSumList can be NULL.
	//
	if ( !(pImapSumList) )
		return TRUE;

	// If none to add:
	//
	if (pImapSumList->GetCount() == 0)
		return TRUE;

	// Must create a new summary for each.
	//
	pos = pImapSumList->GetHeadPosition ();

	BOOL bRes;

	for (next = pos; pos; pos = next)
	{
		CImapSum *pImapSum = (CImapSum *) pImapSumList->GetNext (next);
		if (pImapSum)
		{
			CSummary *pSum = NULL;

			// First see if a summary with a matching UID already exists
			// this will happen when a previous operation using UIDPLUS
			// tries to preserve data associated with a summary after a
			// message has been transferred.

			pSum = pTocDoc->GetSummaryFromUID(pImapSum->GetHash());

			if (!pSum)
			{
				pSum = DEBUG_NEW_MFCOBJ_NOTHROW CSummary;
				if (pSum)
				{
					// Turn off notifying search manager (don't need to save
					// previous value because the summary was just created).
					pSum->SetNotifySearchManager(false);
					
					pSum->m_TheToc = pTocDoc;

					pImapSum->CopyToCSummary (pSum);

					// Copy any text from teemporary to main MBX file (if bfromTmpMbx).
					//

					bRes = TRUE;

					if (bFromTmpMbx)
					{
						bRes = CopyTextFromTmpMbx (pTocDoc, pSum);
					}

					// Now go add the summary.
					//
					if (bRes)
					{
						// Note: Must format date here.
						//
						if ( pImapSum->m_RawDateString.IsEmpty() )
							pSum->FormatDate();
						else
							pSum->FormatDate(pImapSum->m_RawDateString);

						// Restore notifying search manager. Don't need to notify
						// search manager of the above changes, because we haven't
						// added it yet. Adding the summary below will notify the
						// search manager.
						pSum->SetNotifySearchManager(true);

						pTocDoc->AddSum(pSum);

						// Add to pRefSumList as well.
						//
						if (pRefSumList)
							pRefSumList->AddTail (pSum);
					}
					else
					{
						delete pSum;
					}
				}
			}
		}

		// Give back time to op-sys.
		//
		if (::EscapePressed())
			break;
	}

	// Whether we succeeded or not, force a save of the TOC when we're exiting.
	// We don't want the MBX file to have an earlier timespamp than the TOC file.

	pTocDoc->SetModifiedFlag();
	pTocDoc->SaveModified();
	
	return bResult;
}	 




/////////////////////////////////////////////////////////////
// UpdateOldSummaries 
// FUNCTION
// Loop through old summaries and update status flags.
// NOTE: This function removes entries from NewUidList!!!
// END FUNCTION
/////////////////////////////////////////////////////////////
void CImapMailbox::UpdateOldSummaries (CTocDoc *pTocDoc, CPtrUidList& CurrentUidList)
{
	// Sanity.
	if (!pTocDoc)
		return;

	if (CurrentUidList.GetCount() == 0)
		return;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	CSumList &	listSums = pTocDoc->GetSumList();
	POSITION	pos = listSums.GetHeadPosition();
	POSITION	NextPos;
	
	for (NextPos = pos; pos; pos = NextPos)
	{
		CSummary* Sum = listSums.GetNext(NextPos);

		// If this is in CurrentUidList, we've found one. 
		// Note: This is an inefficient search procedure because we may have to 
		// loop through all of "CurrentUidList" to find the match!!!
		//
		POSITION UidPos = CurrentUidList.GetHeadPosition();
		POSITION UidNext;

		for (UidNext = UidPos; UidPos; UidPos = UidNext)
		{
			CImapFlags *pF = (CImapFlags *) CurrentUidList.GetNext (UidNext);
			if ( pF && (pF->m_Uid == Sum->GetHash()) && !pF->m_IsNew)
			{
				// Copy selected Imap Flags from old messages.
				// NOTE: Important. Don't erase any other flag from Sum->m_Imflags,
				// especially the flag indicating the download status!!
				//
				//
				// First, clear the bits we'er interested in.
				Sum->m_Imflags &= ~(IMFLAGS_SEEN | IMFLAGS_ANSWERED | IMFLAGS_FLAGGED | IMFLAGS_DELETED | IMFLAGS_DRAFT );
				//
				// Now set them if they're set.
				Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_ANSWERED);
				Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_FLAGGED);
				Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_DELETED);
				Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_DRAFT);
				Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_SEEN);

				//
				// Set the summary's READ, UNREAD and Answered flags.
				// If current state is one of the other flags, leave as-is.
				//
				int State = Sum->m_State;

				if (State == MS_READ || State == MS_UNREAD || State == MS_REPLIED)
				{
					if (Sum->m_Imflags & IMFLAGS_SEEN)
						Sum->SetState (MS_READ);
					else
						Sum->SetState (MS_UNREAD);

					if (Sum->m_Imflags & IMFLAGS_ANSWERED)
						Sum->SetState (MS_REPLIED);
				}

				// Now get rid of it.
				delete pF;
				CurrentUidList.RemoveAt (UidPos);

				break;
			}
		}

		// Give back time to op-sys.
		//
		if (::EscapePressed())
			break;
	}
}




/////////////////////////////////////////////////////////////
// UpdateOldSummaries - MAP version 
// FUNCTION
// Loop through old summaries and update status flags.
// NOTE: This function removes entries from ModifiedUidMap!!!
// NOTE: ModifiedUidMap will typically contain only UID's whose flags have actually
// changed.
// END FUNCTION
/////////////////////////////////////////////////////////////
void CImapMailbox::UpdateOldSummaries (CTocDoc *pTocDoc, CUidMap& ModifiedUidMap)
{
	// Sanity.
	if (!pTocDoc)
		return;

	if ( ModifiedUidMap.GetCount() == 0 )
		return;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	CSumList &	listSums = pTocDoc->GetSumList();
	POSITION	pos = listSums.GetHeadPosition();
	POSITION	NextPos;

	for (NextPos = pos; pos; pos = NextPos)
	{
		// Since we remove stuff from ModifiedUidMap, it may becose empty:
		if ( ModifiedUidMap.GetCount() == 0 )
			break;

		CSummary* Sum = listSums.GetNext(NextPos);

		// If this is the Inbox and this message is junk then junk that message now.
		if (IsInbox(GetImapName()) &&
			((Sum->m_Imflags & IMFLAGS_DELETED) == 0) &&
			(Sum->GetJunkScore() >= (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK)) &&
			UsingFullFeatureSet() &&
			GetIniShort(IDS_INI_USE_JUNK_MAILBOX))
		{
			// Junk the message.
			pTocDoc->ImapChangeMsgsJunkStatus(Sum, true);
		}
		else
		{
			// If this is in ModifiedUidMap, we've found one. 
			//
			UidIteratorType ci = ModifiedUidMap.find ( Sum->GetHash() );

			if ( ci != ModifiedUidMap.end() )
			{
				CImapFlags *pF = (CImapFlags *) (*ci).second;

				if ( pF && (pF->m_Uid == Sum->GetHash()) && !pF->m_IsNew)
				{
					// Copy selected Imap Flags from old messages.
					// NOTE: Important. Don't erase any other flag from Sum->m_Imflags,
					// especially the flag indicating the download status!!
					//
					//
					// First, clear the bits we'er interested in.
					Sum->m_Imflags &= ~(IMFLAGS_SEEN | IMFLAGS_ANSWERED | IMFLAGS_FLAGGED | IMFLAGS_DELETED | IMFLAGS_DRAFT );
					//
					// Now set them if they're set.
					Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_ANSWERED);
					Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_FLAGGED);
					Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_DELETED);
					Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_DRAFT);
					Sum->m_Imflags |= (pF->m_Imflags & IMFLAGS_SEEN);

					//
					// Set the summary's READ, UNREAD and Answered flags.
					// If current state is one of the other flags, leave as-is.
					//
					int State = Sum->m_State;

					if (State == MS_READ || State == MS_UNREAD || State == MS_REPLIED)
					{
						if (Sum->m_Imflags & IMFLAGS_SEEN)
							Sum->SetState (MS_READ);
						else
							Sum->SetState (MS_UNREAD);

						if (Sum->m_Imflags & IMFLAGS_ANSWERED)
							Sum->SetState (MS_REPLIED);
					}

					// Now get rid of it.
					ModifiedUidMap.erase (ci);

					delete pF;
				}
			}
		}

		// Give back time to op-sys.
		//
		if (::EscapePressed())
			break;
	}
}





//
// Set the TocDoc's frame name to <Account::Name>
//
void CImapMailbox::SetFrameName (CTocDoc *pTocDoc)
{

	// Sanity:
	if (!pTocDoc)
		return;

	// Get acount.
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount (m_AccountID);

	if (pAccount)
	{
		CString szPersona; pAccount->GetName (szPersona);

		// Get the command object and the friendly name.
		QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(
											GetAccountID(), 
											GetImapName (),
											GetDelimiter () );

		if (pImapCommand)
		{
			CString szFullName;

			szFullName.Format ("%s (%s)", pImapCommand->GetName(), szPersona);

			pTocDoc->ReallySetTitle (szFullName);
		}
	}
}


//
// Internal error display method. Just call the external function.
//
void CImapMailbox::ShowLastImapError ()
{
	::ShowLastImapError (m_pImap);
}




// ResetUnseenFlags [PUBLIC]
// All messages in NewUidList that do not have the \seen flags set, unset it
// on tyhe server.
//
void CImapMailbox::ResetUnseenFlags (CPtrUidList& NewUidList)
{
	POSITION pos, nextPos;
	CImapFlags *pF;
	CString sUidlist, sUid;

	// Must have one of these now:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return;
	}

	pos = NewUidList.GetHeadPosition ();

	for (nextPos = pos; pos; pos = nextPos)
	{
		pF = ( CImapFlags * ) NewUidList.GetNext( nextPos );
		if (! (pF && pF->m_Uid))
			continue;

		if (pF->m_Imflags & IMFLAGS_SEEN)
			continue;

		// Ok. This is not seen. Add to list.
		if (sUidlist.IsEmpty())
			sUidlist.Format("%lu", pF->m_Uid);
		else
		{
			sUid.Format (",%lu", pF->m_Uid);
			sUidlist += sUid;
		}
	}

	if (sUidlist.IsEmpty())
		return;

	// Go remove the seen flag.

	m_pImap->UIDRemoveFlaglist (sUidlist, "(\\SEEN)");
}





//
// Perform Manual filtering action. This is called when the "Filter Messages"
// menu is selected.
//
void CImapMailbox::FilterMessages (CTocDoc *pTocDoc, CSummary *SingleSum /* = NULL */)
{
	// Sanity:
	if (!pTocDoc)
		return;

	HRESULT		 hResult = E_FAIL;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	CImapFilterActions FilterActions;

	// NOTE: Always set filename of mailbox.
	FilterActions.SetMBFilename (pTocDoc->GetMBFileName ());

	// In the case of manual filtering forcing us to always be online might sometimes be overly harsh.
	// For example, if all filters do things like labelling that need no connection then we shouldn't
	// force the user online.  This is a first pass and allowing offline manual filtering is a feature for
	// much further down the road.  We'll deal with this properly then.  In the meantime, don't allow
	// manual filtering when offline. -dwiggins
	BOOL	 bMustCloseConnection = FALSE;
	int		 iConnectionState = GetConnectionState(bDontAllowOffline);
	if (iConnectionState == iStateGoOnlineForThis)
	{
		// The connection is being allowed for this action only: note that we must close the connection
		// when we are done.
		bMustCloseConnection = TRUE;
	}
	else if (iConnectionState == iStateStayOfflineDisallow)
	{
		// No connection is being created and this action cannot be done offline: bail out.
		return;
	}

	hResult = OpenMailbox (TRUE);

	if (!SUCCEEDED(hResult))
	{
		// Close the connection if it was opened just for us.
		if (bMustCloseConnection)
		{
			Close();
		}
		return;
	}
	
	if (FilterActions.StartFiltering())
	{
		FilterActions.DoManualFiltering (pTocDoc, SingleSum);

		FilterActions.EndFiltering();
	}

	// This should update |deleted flage, etc!!.
	CTocView *pView = pTocDoc->GetView ();

	if (pView )
		pView->m_SumListBox.SetRedraw(TRUE);

	// Close the connection if it was opened just for us.
	if (bMustCloseConnection)
	{
		Close();
	}
}






// VerifyCache [PUBLIC]
// Make sure the local mailbox directory exists. "CreateMailboxDirectory()" will return TRUE
// if the directory already exists as a direcotry.
//
BOOL CImapMailbox::VerifyCache (BOOL bSilent /* = FALSE */)
{
	CString			buf;			// Scratch buffer.

	if (!CreateMailboxDirectory (m_Dirname))
	{
		if ( ::IsMainThreadMT() && !bSilent )
		{
			ErrorDialog( IDS_ERR_DIR_CREATE,  m_Dirname);
			
		}

		return FALSE;
	}

	// Create the local "Attach" directory for this mailbox.
	CString AttachDir;
	if (!CreateAttachDir (m_Dirname))
	{
		if ( ::IsMainThreadMT() && !bSilent)
		{
			GetAttachDirPath (m_Dirname, buf);
			ErrorDialog( IDS_ERR_DIR_CREATE,  buf);
		}

		return FALSE;
	}

	// Make sure the MBX file exists.
	CString MbxFilePath;
	GetMbxFilePath (m_Dirname, MbxFilePath);
	if (!CreateLocalFile (MbxFilePath, FALSE))
	{
		if ( ::IsMainThreadMT() && !bSilent)
		{
			ErrorDialog( IDS_ERR_IMAP_FILE_CREATE,  MbxFilePath);
		}

		return FALSE;
	}

	return TRUE;
}



// Create the contained CImapSettings object and fill it with user preferences.
//
// Note: Leave this as a separate method so we can always re-get the
// settings.
//
void CImapMailbox::GrabSettings()
{
	// Fill it:
	//
	if (!m_pImapSettings)
	{
		m_pImapSettings = DEBUG_NEW_NOTHROW CImapSettings ();
	}

	if (m_pImapSettings)
		m_pImapSettings->GrabSettings(m_AccountID);

	// Force the CImapConnection to update it's network settings:
	//
	if (m_pImap)
		m_pImap->UpdateNetworkSettings();
}




HRESULT CImapMailbox::GetLastImapError (LPSTR pBuffer, short nBufferSize)
{
	if (!pBuffer)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Initialize.
	*pBuffer = 0;

	if (m_pImap)
	{
		return m_pImap->GetLastImapError (pBuffer,  nBufferSize);
	}

	return E_FAIL;
}



HRESULT	CImapMailbox::GetLastServerMessage (LPSTR pBuffer, short nBufferSize)
{
	if (!pBuffer)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Initialize.
	*pBuffer = 0;

	if (m_pImap)
	{
		return m_pImap->GetLastServerMessage (pBuffer,  nBufferSize);
	}

	return E_FAIL;
}


HRESULT	CImapMailbox::GetLastImapErrorAndType (LPSTR pBuffer, short nBufferSize, int *pType)
{
	if (!pBuffer)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Initialize.
	*pBuffer = 0;

	if (m_pImap)
	{
		return m_pImap->GetLastImapErrorAndType (pBuffer,  nBufferSize, pType);
	}

	return E_FAIL;
}



BOOL CImapMailbox::IsSelected ()
{
	return m_pImap ? m_pImap->IsSelected() : FALSE;
}


// RecreateMessageMap [PUBLIC]
//
void CImapMailbox::RecreateMessageMap ()
{
	if (m_pImap)
		m_pImap->RecreateMessageMap();
}



// AcquireNetworkConnection [PUBLIC]
//
// Negotiate with the connection manager for a CImapConnection object.
//
HRESULT CImapMailbox::AcquireNetworkConnection ()
{
	CImapConnection *pImap = NULL;

	HRESULT hResult = GetImapConnectionMgr()->CreateConnection (m_AccountID, m_ImapName, &pImap, TRUE);

	if ( pImap && SUCCEEDED (hResult) )
	{
		// Set internal pointer.
		//
		m_pImap = pImap;
	}

	return hResult;
}



// DownloadAttachmentsBeforeCompose [PUBLIC]
// 
// If we had to ask, and the user said NOT to download, we then
// clear "szAttach" and return HRESULT_MAKE_CANCEL;
// If we need to download attachments, we do thaht here.
//
HRESULT CImapMailbox::DownloadAttachmentsBeforeCompose (CString& szAttach, UINT Optype)
{
	HRESULT hResult = S_OK;

	if ( szAttach.IsEmpty() )
		return S_OK;

	BOOL bDownloadAttachments = GetIniShort(IDS_INI_ALWAYS_DNLOAD_BEFORE_FWD);

	BOOL bWarn = GetIniShort(IDS_INI_WARN_DNLOAD_BEFORE_FWD);

	if (bWarn)
	{
		// Use out internal copy of string:
		//
		CString szOurAttach = szAttach;

		// Need to determine if any attachment is un-downloaded:
		//
		BOOL bFoundUndownloaded = FALSE;

		CString szOneAttach; szOneAttach.Empty();
		int ns = 0;
		BOOL bDone = FALSE;

		while (!bDone && !bFoundUndownloaded)
		{
			ns = szOurAttach.Find (';');
			if (ns > 0)
			{
				szOneAttach = szOurAttach.Mid (0, ns);	// don't include the ';' 
				szOurAttach = szOurAttach.Mid (ns + 2);	// don't include the ';' or the space in the next filename -jdboyd
			}
			else
			{
				szOneAttach = szOurAttach;
				bDone = TRUE;
			}

			if (szOneAttach.GetLength()) bFoundUndownloaded = !IsImapDownloaded (szOneAttach);
		}

		if (!bFoundUndownloaded)
		{
			return S_OK;
		}

		// OK, We have to ask:
		//
		bDownloadAttachments = (YesNoDialog(IDS_INI_WARN_DNLOAD_BEFORE_FWD,
								 IDS_INI_ALWAYS_DNLOAD_BEFORE_FWD,
								 IDS_WARN_DNLOAD_BEFORE_FWD,
								 Optype == MS_FORWARDED ? CRString (IDS_FIO_FORWARD) :
								 CRString (IDS_FIO_REDIRECT) ) == IDOK);
		
		if (!bDownloadAttachments)
			hResult = HRESULT_MAKE_CANCEL;
	}

	if (!bDownloadAttachments)
	{
		szAttach.Empty();
		return hResult;
	}

	// Otherwise, download all attachments.
	//

	// Use out internal copy of string:
	//
	CString szOurAttach = szAttach;

	CString szOneAttach; szOneAttach.Empty();
	int ns = 0;
	BOOL bDone = FALSE;

	while (!bDone)
	{
		ns = szOurAttach.Find (';');
		if (ns > 0)
		{
			szOneAttach = szOurAttach.Mid (0, ns);	// don't include the ';'
			szOurAttach = szOurAttach.Mid (ns + 2);	// don't include the ';' or the space in the next filename -jdboyd
		}
		else
		{
			szOneAttach = szOurAttach;
			bDone = TRUE;
		}

		if (szOneAttach.GetLength()) ImapDownloaderFetchAttachment (szOneAttach);
	}

	return S_OK;
}



// WriteOfflineMessage [PRIVATE]
//
// Call the downloaded to write a dummy "offline" message if we're attempting to download a message
// and we;re offline.
//
HRESULT CImapMailbox::WriteOfflineMessage (CTocDoc* pTocDoc, CImapSum* pSum, BOOL bToTmpMbx /* = FALSE */)
{
	HRESULT hResult = S_OK;

	// Sanity:
	//
	if (! (pTocDoc && pSum) )
		return E_FAIL;

	// Open the MBX file for appending. This can be either the real
	// MBX file or the temporary one.
	//
	CString szMbxFilePath;

	if (bToTmpMbx)
	{
		GetTmpMbxFilePath (GetDirname(), szMbxFilePath);
	}
	else
	{
		szMbxFilePath = pTocDoc->GetMBFileName();
	}

	// Instantiate an ImapDownloader object to do the downloading.
	CImapDownloader MbxDownloader (GetAccountID(), m_pImap, (LPCSTR) szMbxFilePath);

	// 	Make sure the file exists. Open it for appending. Don't truncate!!
	if (!MbxDownloader.OpenMbxFile (FALSE))
	{
		ErrorDialog ( IDS_ERR_FILE_OPEN, szMbxFilePath, CRString (IDS_ERR_FILE_OPEN_WRITING) );

		return E_FAIL;
	}

	// Go write dummy message to file.
	//
	hResult = MbxDownloader.WriteOfflineMessage (pSum);

	//
	// Note: Only the CImapDownloader can set IMFLAGS_NOT_DOWNLOADED into the summary.
	// Don't do it here because bResult can be TRUE if we wrote a dummy message to the MBX file.
	//

	// CLose the file back.
	MbxDownloader.CloseMbxFile ();

	return hResult;
}



// Ping [PUBLIC]
//
// Send a NOOP to the IMAP mailbox so we update our idea of the number of 
// messages on the server.
//
// NOTE: This can be called from outside. In that case, the connection is made
// if it's not already.
//
void CImapMailbox::Ping()
{
	// If we're offline, ignore it.
	//
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE) )
		return;

	if (!IsSelected())
	{
		HRESULT hResult = OpenMailbox (TRUE);

		if (! SUCCEEDED (hResult) )
		{
			return;
		}
	}

	if ( IsSelected() && m_pImap)
	{
		m_pImap->Noop();

		unsigned long nMsgs = 0;
		if ( SUCCEEDED(m_pImap->GetMessageCount(nMsgs)) )
			m_NumberOfMessages = nMsgs;
	}

}



// ExpandUidRange [PRIVATE]
//
// NOTE: This returns an allocate array - MUST FREE IT.
//
// UNFINISHED: We just return a comma0list of ALL UID's. We need to
// handle mixes of commas and colons, etc.
//
LPSTR CImapMailbox::ExpandUidRange (LPCSTR pUidRange)
{
	// If range is a pure comma-separated list, just resturn that.
	//
	if ( pUidRange && (strchr (pUidRange, ':') == NULL) )
		return SafeStrdupMT(pUidRange);

	if (!m_pImap)
	{
		ASSERT(0);
		return NULL;
	}

	// Get these in any case.
	//	
	unsigned long uidHigh	= 0;
	unsigned long uidLow	= 0;

	if ( !SUCCEEDED(m_pImap->UIDFetchLastUid (uidHigh)) )
		return NULL;

	if ( !SUCCEEDED(m_pImap->UIDFetchFirstUid (uidLow)) )
		return NULL;

	// Note: uidHigh can be same as uidLow.
	//
	if (uidHigh == 0 || uidLow == 0 || uidHigh < uidLow)
		return NULL;

	// Estimate how much of a buffer we need.
	// #digits per UID * #uid's
	//
	int nDigits = 2;	// Includes one for the comma.
	unsigned long max = uidHigh;
	while (max = max/10)
		nDigits++;

	// Include the first and last!
	//
	size_t totlen = (nDigits * (uidHigh - uidLow + 1)) + 1;
	LPSTR pStr  = DEBUG_NEW_NOTHROW char[totlen];
	LPSTR szUid = DEBUG_NEW_NOTHROW char [nDigits + 2];
	int len;

	if (pStr && szUid)
	{
		*pStr = 0;

		LPSTR p = pStr;
		for (unsigned long uid = uidLow; uid <= uidHigh; uid++)
		{
			if (p == pStr)
				sprintf (szUid, "%lu", uid);
			else
				sprintf (szUid, ",%lu", uid);

			len = strlen(szUid);

			if ((pStr + totlen) > (p + len))
			{
				sprintf (p, "%s", szUid);
				p += len;
			}
			else
			{
				// If we run out of room we obviously botched the size calculation above.
				// In the past we just overran the buffer, now we stop adding items if we
				// didn't allocate enough space.  If this ASSERT() ever fires re-evaluate
				// the above size calculation.
				ASSERT(0);
				break;
			}
		}
	}

	delete[] szUid;
			
	return pStr;
}



		
//================ ImapMailboxMode functions ========================/




// NewImapMailboxNode ()
// Create a new ImapMailboxNode structure and initialize fields.
// NOTES
// END NOTES
ImapMailboxNode *NewImapMailboxNode()
{
	ImapMailboxNode *pNode = DEBUG_NEW_NOTHROW ImapMailboxNode;
	if (pNode)
	{
		memset ((void *)pNode, 0, sizeof (ImapMailboxNode));
	}

	return pNode;
}



// SetImapAttributesFromImapNode
// FUNCTION
// Copy attributes from pMboxNode to the CImapMailbox object;
// END FUNCTION

void SetImapAttributesFromImapNode (CImapMailbox* pImapMbox, ImapMailboxNode *pNode)
{
	if (!(pImapMbox && pNode))
		return;

	// Set current mailbox state.
	pImapMbox->SetImapName (pNode->pImapName);
	pImapMbox->SetDirname (pNode->pDirname);
	pImapMbox->SetDelimiter (pNode->Delimiter);
	pImapMbox->SetNoInferiors (pNode->NoInferiors);
	pImapMbox->SetMarked (pNode->Marked);
	pImapMbox->SetUnMarked (pNode->UnMarked);
	pImapMbox->SetAutoSync (pNode->AutoSync);
	pImapMbox->SetImapType (pNode->Type);
}



// Free memory for a folders. Assumes that child list already deleted.
void DeleteMailboxNode (ImapMailboxNode* pFolder)
{
	if (pFolder)
	{
		if (pFolder->pDirname)
			delete[] pFolder->pDirname;

		if (pFolder->pImapName)
			delete[] pFolder->pImapName;

		delete pFolder;
	}
}

// Delete a child folder list from a folder.
// Note: Don't delete the parent folder itself.
void DeleteChildList (ImapMailboxNode *pParentFolder)
{
	ImapMailboxNode		*pFolder, *pNextFolder;
	if (!pParentFolder)
		return;

	pFolder = pParentFolder->ChildList;
	while (pFolder)
	{
		// Make sure.
		pNextFolder = pFolder->SiblingList;

		// Delete it's child list before deleting it.
		DeleteChildList (pFolder);

		// Delete it now.
		DeleteMailboxNode (pFolder);

		// Loop through siblings.
		pFolder = pNextFolder;
	}

	// Make sure to do this.
	pParentFolder->ChildList = NULL;
}


// Delete a sibling folder list from a folder.
// Note: Don't delete the given folder itself.
void DeleteSiblingList (ImapMailboxNode *pGivenFolder)
{
	ImapMailboxNode		*pFolder, *pNextFolder;
	if (!pGivenFolder)
		return;

	pFolder = pGivenFolder->SiblingList;
	while (pFolder)
	{
		// Make sure.
		pNextFolder = pFolder->SiblingList;

		// Delete it's child list before deleting it.
		DeleteChildList (pFolder);

		// Delete it now.
		DeleteMailboxNode (pFolder);

		// Loop through siblings.
		pFolder = pNextFolder;
	}

	// Make sure to do this.
	pGivenFolder->SiblingList = NULL;
}


// Delete a complete folder subtree, INCLUDING the starting folder.
// Note: It is up to the calling function to take note of the fact that
// this memory is no longer available and that the passed-in pointer is no longer valid!
void DeleteFolderSubtree (ImapMailboxNode *pStartingFolder)
{
	// Sanity.
	if (!pStartingFolder)
		return;

	// Delete child list.
	DeleteChildList (pStartingFolder);
	pStartingFolder->ChildList = NULL;

	// Delete sibling list.
	DeleteSiblingList (pStartingFolder);
	pStartingFolder->SiblingList = NULL;

	// Now, delete me.
	DeleteMailboxNode (pStartingFolder);
}




// =================== Global Routines ==============================//




// ================= exported Utility Functions ===========================//

// ExpungeAfterDelete [EXPORTED]
//
// Return TRUE if the IDS_INI_IMAP_REMOVE_ON_DELETE flag is set for the personality that
// pTocDoc is a member of.
//
BOOL ExpungeAfterDelete (CTocDoc *pTocDoc)
{
	if (! (pTocDoc && pTocDoc->m_pImapMailbox) )
		return FALSE;

	// Just return the value in m_Settings.
	//
	return pTocDoc->m_pImapMailbox->GetSettingsShort (IDS_INI_IMAP_REMOVE_ON_DELETE);
}





// FUNCTION
// Wade through NewUidList and any entries that are also in CurrentUidList, move to NewOldUidList
// with the new flags.
// If there is any entry in NewUidList that has Uid == 0, remove it.
// AT the end, we should have the New Uid list containing truly new messages and 
// "CurrentUidList" containing only messages that are no longer on the server.
// "NewOldUidList" will contain messages that should remain in the local cache,
// but probably with new flags (e.g, someone may have set the \Deleted flags.
// END FUNCTION
// 
// NOTE:
// The assumption is that these lists are ordered in ascending order of UID!!!
// END NOTE
//
// 
void MergeUidListsMT (CPtrUidList& CurrentUidList, CPtrUidList& NewUidList, CPtrUidList& NewOldUidList)
{
	POSITION oldPos, oldNext;
	POSITION newPos, newNext;
	CImapFlags *pOldF, *pNewF;
	unsigned long curUid;

	//
	// Clear NewOldUidList
	//
	NewOldUidList.DeleteAll ();

	oldPos = CurrentUidList.GetHeadPosition();
	newPos = NewUidList.GetHeadPosition();

	// Loop through oldPos.
	for (oldNext = oldPos; oldPos; oldPos = oldNext)
	{
		pOldF = ( CImapFlags * ) CurrentUidList.GetNext( oldNext );
		if (pOldF)
		{
			curUid = pOldF->m_Uid;
			
			// Loop through pNewF until we get to the same or next higher UID.
			//
			while (newPos)
			{
				newNext = newPos;

				pNewF = ( CImapFlags * ) NewUidList.GetNext ( newNext );
				if (pNewF)
				{
					// Delete any zero-uid entries.
					if (pNewF->m_Uid == 0)
					{
						// Delete
						NewUidList.RemoveAt (newPos);
						delete pNewF;

						// Loop to the next entry.
						newPos = newNext;
					}
					else if (pNewF->m_Uid == curUid)
					{

						// This gets deleted both from NewUidList and from CurrentUidList,
						// and added to NewOldUidList;
						// Note: The resulting "NewOldUidList" will be in ascending UID-order.
						//
						CurrentUidList.RemoveAt (oldPos);

						NewOldUidList.AddTail ( pOldF);

						//
						// Merge flags. Set the Seen flag from our value but copy the rest from remote.
						//
						// First, clear the bits we'er interested in.
						pOldF->m_Imflags &= ~( IMFLAGS_SEEN | IMFLAGS_ANSWERED | IMFLAGS_FLAGGED | IMFLAGS_DELETED | IMFLAGS_DRAFT | IMFLAGS_RECENT );
						//
						pOldF->m_Imflags |= (pNewF->m_Imflags & IMFLAGS_DELETED);
						pOldF->m_Imflags |= (pNewF->m_Imflags & IMFLAGS_RECENT);
						pOldF->m_Imflags |= (pNewF->m_Imflags & IMFLAGS_SEEN);
						pOldF->m_Imflags |= (pNewF->m_Imflags & IMFLAGS_ANSWERED);
						pOldF->m_Imflags |= (pNewF->m_Imflags & IMFLAGS_FLAGGED);
						pOldF->m_Imflags |= (pNewF->m_Imflags & IMFLAGS_DRAFT);

						// Delete pOldF from list.
						NewUidList.RemoveAt (newPos);
						delete pNewF;
						pNewF = NULL;

						// Do next on current list.
						newPos = newNext;
			
						break;
					}
					else if (pNewF->m_Uid > curUid)
					{
						//
						// Next time we start with this pNewF again.
						// Don't update newPos.
						//
						break;
					}
					else // pNewF->m_Uid < curUid.
					{
						// Haven't got to a UID that is equal or greater. Keep looping.
						newPos = newNext;
					}
				}
				else
				{
					// Invalid entry. Remove.
					//
					NewUidList.RemoveAt (newPos);

					// Keep looping.
					newPos = newNext;
				}
			}  // while new list
		}
		else
		{
			// Remove any empty node.
			CurrentUidList.RemoveAt (oldPos);
		}
	} // for current list.
}											



// Allocate memory and copy buf to it.
LPSTR CopyString (LPCSTR buf)
{
	LPSTR	p = NULL;

	if (!buf)
		return NULL;

	p = DEBUG_NEW_NOTHROW char [strlen (buf) + 2];
	if (p)
	{
		strcpy (p, buf);
	}

	return p;
}
	


// CreateLocalFile
// FUNCTION
// Given the full path to a file, if it doesn't exist, create it read/write.
// If "Truncate", delete its contents.
// END FUNCTION

BOOL CreateLocalFile (LPCSTR pFilepath, BOOL Truncate)
{
	DWORD	Attributes;
	BOOL	bResult = FALSE;

	if (pFilepath)
	{
		// If file exists, truncate it.
		Attributes = GetFileAttributes (pFilepath);
		if (Attributes != 0xFFFFFFFF)
		{
			// Make sure it's a file.
			if (! (Attributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				// Its a file. Truncate?
				if (Truncate)
				{
					bResult = ChangeFileSize (pFilepath, 0);
				}
				else 
					bResult = TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			// Doesnt exist. Create it.
			JJFileMT	jFile;
			if (SUCCEEDED (jFile.Open (pFilepath, O_RDWR | O_CREAT) ) )
			{
				bResult = TRUE;
				jFile.Close();
			}
		}
	}

	return bResult;
}

				

// If the file exists, remove it.
void DeleteLocalFile (LPCSTR pPathname)
{
	if (!pPathname)
		return;

	if (FileExistsMT (pPathname))
	{
		_chmod (pPathname, _S_IREAD | _S_IWRITE);
		_unlink (pPathname);
	}
}



// Given a parent directory name and a child name, create a full pathname.
void MakePath (LPCSTR pParent, LPCSTR pName, CString &Path)
{
	if (!(pParent && pName))
		return;

	Path = pParent;
	if (Path.Right ( 1 ) != DirectoryDelimiter)
		Path += DirectoryDelimiter;

	Path += pName;
}



// DirectoryExists 
// Return TRUE if a directory with the given pathname exists.
// The name must be a full path.
BOOL  DirectoryExists (LPCSTR pPath)
{
	DWORD Attributes;

	// If the directory exists, OK.
	Attributes = GetFileAttributes ((LPCSTR)pPath);
	if (Attributes == 0xFFFFFFFF)
		return FALSE;
	else
	{
		// Make sure it's a directory.
		if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
			return TRUE;
		else
			return FALSE;
	}
}


// FormatBasePath
// Given a directory name, make sure it has a trailing directory
// delimiter character.
// NOTES
// Write the result into BasePath.
// END NOTES
void FormatBasePath (LPCSTR pPath, CString &BasePath)
{
	int i;

	if (!pPath)
		return;
	
	BasePath = pPath;
			
	i = BasePath.GetLength();
	if (i <= 0)
		return;

	if (BasePath.GetAt (i - 1) != DirectoryDelimiter)
		BasePath += CString (DirectoryDelimiter);
}




// 
void StripTrailingDelimiter (CString &myName)
{
	int length;

	length = myName.GetLength();

	while ((length > 0) && (myName.Right ( 1 ) == DirectoryDelimiter))
	{
		length--;
		myName = myName.Left ( length );
	}
}


// ChangeFileSize.

BOOL ChangeFileSize (LPCSTR pFilepath, long length)
{
	JJFileMT	jFile;
	BOOL		bResult = FALSE;

	if (pFilepath)
	{
		if (SUCCEEDED(jFile.Open (pFilepath, O_RDWR) ) )
		{
			bResult = SUCCEEDED (jFile.ChangeSize ( length ) );
			jFile.Close();
		}
	}

	return bResult;
}



// BOOL CreateMailboxDirectory (LPCSTR pDirname)
// FUNCTION
// Create the directory housing a mailbox.
// END FUNCTION.

BOOL CreateMailboxDirectory (LPCSTR pDirname)
{
	if (!pDirname)
		return FALSE;

	// If the dirname exists as a file, delete it.
	DWORD Attributes = GetFileAttributes (pDirname);
	if ( (Attributes != 0xFFFFFFFF)	&&					// Exists
		 ! (Attributes & FILE_ATTRIBUTE_DIRECTORY) )	// Is a file.
	{
		DeleteLocalFile (pDirname);
	}

	// If the directory doesn't exist, try now to create it.
	if ( !DirectoryExists (pDirname) )
	{
		// Try to create.
		if (!CreateDirectory (pDirname, NULL))
		{
			return FALSE;
		}
	}

	// If it doesn't exist now, we really can't create it.
	if (!DirectoryExists (pDirname))
	{
		return FALSE;
	}
	else
		return TRUE;
}



// CreateMbxFile
// NOTES
// If the file already exists, don't truncate.
// END NOTES

BOOL CreateMbxFile (LPCSTR pMailboxDir)
{
	CString MbxFilePath;

	if (pMailboxDir)
	{
		if (GetMbxFilePath (pMailboxDir, MbxFilePath))
		{
			return CreateLocalFile (MbxFilePath, FALSE);
		}
	}

	return FALSE;
}




// CreateAttachDir
// FUNCTION
// Given the name of a directory housing a mailbox, create a subdirectory named "Attach"
// for storing attachments.
// END FUNCTION
// NOTES
// Return FALSE if we couldn't create the directory.
// END NOTES
BOOL CreateAttachDir (LPCSTR pPathname)
{
	CString AttachDir;

	GetAttachDirPath (pPathname, AttachDir);

	// If it exists, OK
	DWORD Attributes = GetFileAttributes (AttachDir);

	// Exists?
	if (Attributes != 0xFFFFFFFF)	// Yes. It exists.
	{
		if (Attributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
			return TRUE;
		}
		else
		{
			// It's a file, remove it.
			DeleteLocalFile (AttachDir);
		}
	}

	// Ok Try to create it.
	return CreateDirectory (AttachDir, NULL);
}



// GetAttachDirPath
// FUNCTION
// Given the name of a directory housing a mailbox, format a full path to 
// its "Attach" subdirectory.
// END FUNCTION
// NOTES
// END NOTES
void GetAttachDirPath (LPCSTR pPathname, CString& FullPath)
{
	if (!pPathname)
		return;

	FullPath = pPathname;
	if (FullPath.Right ( 1 ) != DirectoryDelimiter)
	{
		FullPath += DirectoryDelimiter;
	}

	FullPath += CRString (IDS_IMAP_ATTACH_DIRNAME);
}
				



// GetMbxFilePath
// FUNCTION
// Given the mailbox directory (pMailboxDir), create a full pathname to
// the contained MBX file.
// Return the MBX filepath in MbxFilePath.
// END FUNCTION

BOOL GetMbxFilePath (LPCSTR pMailboxDir, CString& MbxFilePath)
{
	int		length, i;
	CString path;

	if (!pMailboxDir)
		return FALSE;

	// pMailboxDir MUST have at least a directory delimiter AND another char in the name.
	// i.e., at least "\a".
	path = pMailboxDir;
	length = path.GetLength();
	if (length < 2)
		return FALSE;

	// Remove any trailing delimiter.
	if (path[length - 1] == DirectoryDelimiter)
	{
		path = path.Left (length - 1);
		length--;
	}

	// Add the last name.
	i = path.ReverseFind (DirectoryDelimiter);
	if ((i >= 0) && (i < (length - 1)) )
	{
		MbxFilePath = path + DirectoryDelimiter + path.Mid (i + 1) + CRString (IDS_MAILBOX_EXTENSION);

		return TRUE;
	}

	return FALSE;
}




// GetTmpMbxFilePath
// FUNCTION
// Given the mailbox directory (pMailboxDir), create a full pathname to
// the contained temporary MBX file.
// Return the MBX filepath in MbxFilePath.
// END FUNCTION

BOOL GetTmpMbxFilePath (LPCSTR pMailboxDir, CString& TmpMbxFilePath)
{
	int		length, i;
	CString path;

	if (!pMailboxDir)
		return FALSE;

	// pMailboxDir MUST have at least a directory delimiter AND another char in the name.
	// i.e., at least "\a".
	path = pMailboxDir;
	length = path.GetLength();
	if (length < 2)
		return FALSE;

	// Remove any trailing delimiter.
	if (path[length - 1] == DirectoryDelimiter)
	{
		path = path.Left (length - 1);
		length--;
	}

	// Change name to tmp_Name.tbx..
	i = path.ReverseFind (DirectoryDelimiter);
	if ((i >= 0) && (i < (length - 1)) )
	{
		CString Name = "tmp_" + path.Mid (i + 1);
		TmpMbxFilePath = path + DirectoryDelimiter + Name + CRString (IDS_IMAP_TMPMBX_EXTENSION);

		return TRUE;
	}

	return FALSE;
}



// GetInfFilePath
// FUNCTION
// Given the mailbox directory (pMailboxDir), create a full pathname to
// the contained INF file.
// Return the INF filepath in InfFilePath.
// END FUNCTION

BOOL GetInfFilePath (LPCSTR pMailboxDir, CString& InfFilePath)
{
	int		length, i;
	CString path;

	if (!pMailboxDir)
		return FALSE;

	// pMailboxDir MUST have at least a directory delimiter AND another char in the name.
	// i.e., at least "\a".
	path = pMailboxDir;
	length = path.GetLength();
	if (length < 2)
		return FALSE;

	// Remove any trailing delimiter.
	if (path[length - 1] == DirectoryDelimiter)
	{
		path = path.Left (length - 1);
		length--;
	}

	// Add the last name.
	i = path.ReverseFind (DirectoryDelimiter);
	if ((i >= 0) && (i < (length - 1)) )
	{
		InfFilePath = path + DirectoryDelimiter + path.Mid (i + 1) + CRString (IDS_IMAP_INFO_FILE_EXTENSION);

		return TRUE;
	}

	return FALSE;
}





// MbxFilePathToMailboxDir
// FUNCTION
// Given the full path to a mailbox directory, extract the name of the containing directory into
// "MailboxDir".
// END FUNCTION

BOOL MbxFilePathToMailboxDir (LPCSTR pMbxFilepath, CString& MailboxDir)
{
	int		i;
	CString path;
	BOOL	bResult = FALSE;

	if (!pMbxFilepath)
		return FALSE;

	// Make sure this is an mbx file path.
	path = pMbxFilepath;
	path.TrimRight();

	i = path.ReverseFind ('.');
	if (i > 0)
	{
		path = path.Mid (i);		// .mbx (includes dot)
		if (path.CompareNoCase (CRString (IDS_MAILBOX_EXTENSION)) == 0)
		{
			bResult = TRUE;
		}
	}

	if (bResult)
	{
		bResult = FALSE;
		path = pMbxFilepath;

		i = path.ReverseFind (DirectoryDelimiter);
		if (i == 0) // \name.mbx????
		{
			path = path.Left (1);
			bResult = TRUE;
		}
		else
		{
			path = path.Left (i);
			bResult = TRUE;
		}

		if (bResult)
			MailboxDir = path;
	}

	return bResult;
}



// ImapIsTmpMbx
// FUNCTION
// Return TRUE if the given filename ends in ".tbx".
// END FUNCTION

BOOL ImapIsTmpMbx (LPCSTR pFilename)
{
	if (!pFilename)
		return FALSE;

	CString ext =  pFilename;
	int i = ext.ReverseFind ('.');
	if (i > 0)
	{
		ext = ext.Mid (i);
		if (ext.CompareNoCase (CRString (IDS_IMAP_TMPMBX_EXTENSION)) == 0)
			return TRUE;
	}

	return FALSE;
}

	


// CopyAttachmentFile
// FUNCTION
// Copy an attachment file between attachment directories.
// END FUNCTION
// NOTES
// "pFilename" may be a full pathname, in which case, strip tha path off. Get a unique
// filename in the destination directory. Return the full pathname of the destination
// file in "NewPathName".
// END NOTES

BOOL ImapCopyAttachmentFile (LPCSTR pSourceAttachmentDir, LPCSTR pTargetMailboxDir, LPCSTR pFilename, CString& NewPathname)
{
	char		buf [MAXBUF + 4];
	char		Name [MAXBUF + 4];
	CString		str;
	JJFileMT*   AttachFile = NULL;
	BOOL		bResult = FALSE;

	if (!(pSourceAttachmentDir && pTargetMailboxDir && pFilename))
		return FALSE;

	// Strip path, if any.
	str = pFilename;
	int i = str.ReverseFind (DirectoryDelimiter);
	if (i >= 0)
		str = str.Mid (i + 1);
	
	// Copy to Name.
	if (str.GetLength() > MAXBUF)
		return FALSE;
	strcpy (Name, (LPCSTR) str);

	// Get unique filename in the "Attach" subdirectory of pTargetMailboxDir.
	str = pTargetMailboxDir;

	// Need to do this for "OpenLocalAttachFile()!!!"
	if (str.Right ( 1 ) != DirectoryDelimiter)
	{
		str += DirectoryDelimiter;;
	}

	// We need to put stuff into a char array for OpenLocalAttachFile().
	if (str.GetLength() > MAXBUF)
		return FALSE;

	strcpy (buf, (LPCSTR)str);

	AttachFile = OpenLocalAttachFile(buf, Name, false);
	if (AttachFile)
	{
		BSTR pAllocStr = NULL;

		// Put the filename into "NewPathname".
		if (SUCCEEDED ( AttachFile->GetFName(&pAllocStr) ) && pAllocStr != NULL)
		{
			NewPathname = pAllocStr;
			SysFreeString (pAllocStr);
			pAllocStr = NULL;
		}

		AttachFile->Close();

		if (!NewPathname.IsEmpty())
		{
			// Full path to source file.
			str  = pSourceAttachmentDir;
			if (str.Right ( 1 ) != DirectoryDelimiter)
			{
				str += DirectoryDelimiter;
			}
			str += Name;

			// Do the copy. Overwrite existing file.
			bResult = CopyFile (str, NewPathname, FALSE);
		}

		delete AttachFile;
	}

	if (!bResult)
		NewPathname.Empty();

	return bResult;
}


// 
// NOTES
// This returns a complete path in "Newpath"
// END NOTES

BOOL GetUniqueTempFilepath (LPCSTR pParentDir, LPCSTR pBaseName, CString &Newpath)
{
	CString			Name, BaseName;
	int				i, trial;
	const int		MaxTrial = 1000;
	BOOL			bResult = FALSE;
	CString			DirectoryPrefix;

	// pTopNode can be NULL.
	if (!(pParentDir && pBaseName))
		return FALSE;

	// Use pCurNode->pImapName as the suggested name.
	// If name begins with a dot, keep it a part of the name,
	// otherwise use only the chars before the dot.
	// Make sure it's at most MaxNameLength characters long.

	Name = pBaseName;

	// Delete any ".extension" in filename.
	i = Name.Find('.');
	while (i >= 0)
	{
		if (i == 0)
		{
			// If name is something like ".xxx", use "xxx".
			Name = Name.Mid ( 1 );

			// Make sure we don't have any more.
			i = Name.Find ('.');
		}
		else if (i > 0)
		{
			// Use basename;
			Name = Name.Left (i);
			i = -1;  // Stop here.
		}
		else
		{
			// else use as is.
			i = -1;
		}
	}

	// Do we have anything left?
	if (Name.IsEmpty())
		Name = '0';


	// Extract basename if contains directory delimiters.

	i = Name.ReverseFind (DirectoryDelimiter);
	if (i >= 0)
	{
		Name = Name.Mid (i + 1);
	}

	// Do we have anything left?
	if (Name.IsEmpty())
		Name = '0';

	// See if we already have that name (Case insensitive compare, since the name become
	// files in a possibly FAT file system). Add a uniquifier digit.

	// Format the containing directory name with a trailing directory delimiter.
	DirectoryPrefix = pParentDir;
	if (DirectoryPrefix.Right ( 1 ) != DirectoryDelimiter)
		DirectoryPrefix += DirectoryDelimiter;

	// Now go make sure there's no duplicate file or directory name.
	BaseName = Name;
	bResult = FALSE;

	for (trial = 0; trial <= MaxTrial; trial++)
	{
		if (FileExistsMT (DirectoryPrefix + Name))
		{
			// Found an existing file or directory. Add a uniquifier.
			Name.Format ("%s%d", (LPCSTR)BaseName, trial);
		} // if
		else
		{
			// Found a unique name.
			bResult = TRUE;
			break;		
		}
	}

	// Did we get a unique name??
	if (trial > MaxTrial || Name.IsEmpty())
		return FALSE;

	// Ok. Found one at last. 
	bResult = TRUE;		// Found a unique name.

	// Make sure copy Name to NewName.
	if (bResult)
	{
		Newpath = DirectoryPrefix + Name;
	}

	return bResult;
}		



// FUNCTION
// Delete a filesystem directory. If "DeleteChildren" is TRUE, delete all
// contained files and subdirectories.
// END FUNCTION

BOOL RemoveLocalDirectory (LPCSTR pPathname, BOOL RemoveChildren)
{
	BOOL		bResult = TRUE;
	CString		Path;
	CString		Name;

	if (!pPathname)
		return FALSE;

	// If this is a simple file, just delete it.
	DWORD Attributes = GetFileAttributes (pPathname);

	// Does it even exist? If not, still return TRUE.
	if (Attributes == 0xFFFFFFFF)
		return TRUE;

	// Is it a directory.
	if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (RemoveChildren)
		{
		    CFileFind finder;

			Path = pPathname;
			Path += "\\*.*";

		    BOOL bWorking = finder.FindFile(Path);

			while (bWorking)
			{
				bWorking = finder.FindNextFile();
				
				// Ignore "." and ".."
				Path = finder.GetFilePath();
				Name = finder.GetFileName();
				if ( !( (Name.Compare (".") == 0) || (Name.Compare ("..") == 0) ) )
				{
					if (finder.IsDirectory())
					{
						bResult = RemoveLocalDirectory( Path, TRUE) && bResult;
					}
					else
						DeleteLocalFile (Path);
				}
			}
		}

		// Remove the directory itselt.
		// BUG: Should do the foll:
		// 1. If the directory is the current working directory, set the cwd up one level.

		bResult = RemoveDirectory (pPathname) && bResult;

#ifdef _MYDEBUG
		if (!bResult)
		{
			// For debugging:
			int error = GetLastError();
		}
#endif

	}
	else
	{
		DeleteLocalFile (pPathname);
	}

	return bResult;
}



// FUNCTION
// Formulate a friendly name for a mailbox.
// Write the result into FriendlyName.
// END FUNCTION

void FormulateFriendlyImapName (LPCSTR pImapName, TCHAR DelimiterChar, CString& FriendlyName)
{

	if (!pImapName)
		return;

	// At the very least, return pImapName.
	FriendlyName = pImapName;

	// Is this INBOX??
	CRString INBOX (IDS_IMAP_RAW_INBOX_NAME);

	if (INBOX.CompareNoCase (pImapName) == 0)
	{
		FriendlyName = CRString (IDS_IMAP_FRIENDLY_INBOX_NAME);
	}
	else
	{
		// User the basename.
		if (DelimiterChar)
		{
			int i = FriendlyName.ReverseFind (DelimiterChar);
			if (i >= 0)
				FriendlyName = FriendlyName.Mid (i + 1);
		}
	}
}





// DoingMinimalDownload [INTERNAL]
// FUNCTION
// DETERMINE if the user wants us to do minimal download for the given mailbox.
// END FUNCTION

BOOL DoingMinimalDownload (CImapMailbox *pImapMailbox)
{
	// Must have a mailbox.
	if (!pImapMailbox)
		return TRUE;

	// Just do a GetIniShort for now.
	return pImapMailbox->GetSettingsShort (IDS_INI_IMAP_MINDNLOAD);
}
		



///////////////////////////////////////////////////////////////////////////////////
// RenameMboxDirAndContents
//
// Given the name of a local directory housing a mailbox, and an ImapMailboxNode
// representing a renamed mailbox, go find a new unique name for the mailbox
// within it's parent directory, rename it and it's contents.
// Return the new name (just the relative name) in szNewDirname.
///////////////////////////////////////////////////////////////////////////////////
BOOL RenameMboxDirAndContents (LPCSTR pOldDirpath, ImapMailboxNode *pNode, CString& szNewDirname, LPCSTR pNewParentDir /* = NULL */)
{
	CString				szPathname;
	BOOL				bResult = FALSE;

	// Sanity:
	if ( ! (pOldDirpath && pNode) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Clear this in case...
	szNewDirname.Empty();

	// If a parent directory is given, use that, otherwise use
	// the same parent directory as pOldDirpath.
	//
	if (pNewParentDir)
	{
		szPathname = pNewParentDir;
	}
	else
	{
		char				szDrive[ _MAX_DRIVE ];
		char				szDir[ 1024 ];
		char				szFname[ 256 ];
		char				szExt[ _MAX_EXT ];

		//
		// Find the parent directory. MUST have one!!
		//
		_splitpath( pOldDirpath, szDrive, szDir, szFname, szExt );
	
		// This is the parent directory.
		szPathname = szDrive;
		szPathname += szDir;
	}

	// remove trailing directory delimiter if any.
	if( szPathname.Right(1) == '\\' )
	{
		szPathname = szPathname.Left( szPathname.GetLength() - 1 );
	}
	
	//
	// Get a new unique directory name.
	//
	CString szBasename;

	if ( MakeSuitableMailboxName (szPathname, NULL, pNode, szBasename, 64) )
	{
		// Format a full path.
		szPathname += DirectoryDelimiter + szBasename;

		// Now rename the directory. If we couldn't rename directory, 
		// create new one.
		//
		BOOL bSuccess = SUCCEEDED ( ::FileRenameMT( pOldDirpath, szPathname ) );

		if (!bSuccess)
		{
			bSuccess = CreateMailboxDirectory (szPathname);
		}

		// Rename all the internal files 
		RenameInternalMailboxFiles (pOldDirpath, szPathname);

		// Copy basename to szNewDirname to be returned.
		szNewDirname = szBasename;

		// Succeeded.
		bResult = bSuccess;
	}

	return bResult;
}





//
// RenameInternalMailboxFiles
// Alter a mailbox directory has been renamed, rename the internal
// MBX, TOC, inf, act, opt files.
// 
//
// NOTES
// This may be a non-selectable mailbox, in which case the MBX and TOC files may not exist.
// END NOTES
//
BOOL RenameInternalMailboxFiles (LPCSTR pOldDirPathname, LPCSTR pNewDirPathname)
{
	CString szOldDirname;
	CString szNewDirname;
	CString szOldFilepath, szNewFilepath;

	// Sanity
	if (! (pOldDirPathname && pNewDirPathname) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Extract the old directory name.
	szOldDirname = pOldDirPathname;
	int nc = szOldDirname.ReverseFind (DirectoryDelimiter);
	if (nc >= 0)
		szOldDirname = szOldDirname.Mid (nc + 1);

	// Extract the new directory name.
	szNewDirname = pNewDirPathname;
	nc = szNewDirname.ReverseFind (DirectoryDelimiter);
	if (nc >= 0)
		szNewDirname = szNewDirname.Mid (nc + 1);

	// Rename the MBX file
	szOldFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szOldDirname, CRString (IDS_MAILBOX_EXTENSION) );
	szNewFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szNewDirname, CRString (IDS_MAILBOX_EXTENSION) );

	//
	// Make sure no file exists with the new name. If a sub-directory exists with this name, 
	// "RemoveLocalDirectory" will remove it. It will also remove it if it's a file.
	//
	RemoveLocalDirectory (szNewFilepath, TRUE);

	// BUG: We should check for success here!!
	FileRenameMT (szOldFilepath, szNewFilepath);

	// Rename the TOC file
	szOldFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szOldDirname, CRString (IDS_TOC_EXTENSION) );
	szNewFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szNewDirname, CRString (IDS_TOC_EXTENSION) );

	//
	// Make sure no file exists with the new name. If a sub-directory exists with this name, 
	// "RemoveLocalDirectory" will remove it. It will also remove it if it's a file.
	//
	RemoveLocalDirectory (szNewFilepath, TRUE);

	FileRenameMT (szOldFilepath, szNewFilepath);

	// Rename the INF file
	szOldFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szOldDirname, CRString (IDS_IMAP_INFO_FILE_EXTENSION) );
	szNewFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szNewDirname, CRString (IDS_IMAP_INFO_FILE_EXTENSION) );

	//
	// Make sure no file exists with the new name. If a sub-directory exists with this name, 
	// "RemoveLocalDirectory" will remove it. It will also remove it if it's a file.
	//
	RemoveLocalDirectory (szNewFilepath, TRUE);

	FileRenameMT (szOldFilepath, szNewFilepath);

	// Rename the ACT file
	szOldFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szOldDirname, CRString (IDS_IMAP_JRNL_FILE_EXTENSION) );
	szNewFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szNewDirname, CRString (IDS_IMAP_JRNL_FILE_EXTENSION) );

	//
	// Make sure no file exists with the new name. If a sub-directory exists with this name, 
	// "RemoveLocalDirectory" will remove it. It will also remove it if it's a file.
	//
	RemoveLocalDirectory (szNewFilepath, TRUE);

	FileRenameMT (szOldFilepath, szNewFilepath);

	// Rename the OPT file
	szOldFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szOldDirname, CRString (IDS_IMAP_RESYNCOPT_FILE_EXTENSION) );
	szNewFilepath.Format ("%s%c%s%s", pNewDirPathname, DirectoryDelimiter, szNewDirname, CRString (IDS_IMAP_RESYNCOPT_FILE_EXTENSION) );

	//
	// Make sure no file exists with the new name. If a sub-directory exists with this name, 
	// "RemoveLocalDirectory" will remove it. It will also remove it if it's a file.
	//
	RemoveLocalDirectory (szNewFilepath, TRUE);

	FileRenameMT (szOldFilepath, szNewFilepath);

	return TRUE;
}





//////////////////////////////////////////////////////////////////////////////////////////////
// CreateDirectoryForMailbox
//
// Get a unique directory within "pParentDir" and create it. Set the full pathname into 
// pImapNode->pDirname;
// "pImapNode" contains all the info needed.
////////////////////////////////////////////////////////////////////////////////////////////// 
BOOL CreateDirectoryForMailbox (LPCSTR pParentDir, ImapMailboxNode *pImapNode, CString& szNewDirname)
{
	BOOL bResult = FALSE;

	// Sanity:
	if (! (pParentDir && pImapNode) )
		return FALSE;

	// Get a unique directory name and create it NOW!.We will create the
	// files within the directory later.

	CString szBasename;

	bResult = MakeSuitableMailboxName (pParentDir, NULL, pImapNode, szBasename, 64);

	if (bResult)
	{
		// Format a path to DirName and copy it to pCurNode->pPathname.
		CString Path = pParentDir;

		if (Path.Right (1) != DirectoryDelimiter)
			Path += DirectoryDelimiter;

		Path +=  szBasename;

		// Set it into the node.
		if (pImapNode->pDirname)
			delete[] (pImapNode->pDirname);

		pImapNode->pDirname	= CopyString (Path);

		// Make sure the directory exists or that we can create it.
		bResult = CreateMailboxDirectory (Path);

		// Set output parameters.
		if (bResult)
			szNewDirname = szBasename;
	}

	return bResult;
}



/////////////////////////////////////////////////////////////////////////////////
// ServerHasHigherUid
//
// Look at the last element in the list and return it's UID.
// that any UID in CurrentUidList.
// 
/////////////////////////////////////////////////////////////////////////////////
unsigned long GetLocalHighestUid (CPtrUidList& CurrentUidList)
{
	POSITION pos;
	unsigned long Uid = 0;

	//
	// Note: We can simply compare UidServerHighest with the tail position in the
	// list because CurrentUidList is ordered!!
	//
	pos = CurrentUidList.GetTailPosition ();

	if (pos)
	{
		CImapFlags *pF = (CImapFlags *) CurrentUidList.GetPrev( pos );
		
		if( pF )
		{
			Uid = pF->m_Uid;
		}
	}

	return Uid;		
}




////////////////////////////////////////////////////////////////////////
// GetCurrentUidList 
//
// Accumulate current uid's and flags in "CurrentUidList"
//
////////////////////////////////////////////////////////////////////////

void GetCurrentUidList (CTocDoc *pTocDoc, CPtrUidList& CurrentUidList)
{
	unsigned long uid;

	// Clear list first.
	CurrentUidList.DeleteAll ();

	// Sanity:
	if (! pTocDoc )
		return;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	CSumList &	listSums = pTocDoc->GetSumList();
	POSITION	pos = listSums.GetHeadPosition();
	CSummary *	pSum;

	while (pos)
	{
		pSum = listSums.GetNext(pos);
		if (pSum)
		{
			uid = pSum->GetHash();
			if (uid != 0)
			{
				CurrentUidList.OrderedInsert (uid, pSum->m_Imflags, FALSE);
			}
		}
	}
}





////////////////////////////////////////////////////////////////////////
// GetCurrentUids - STL "MAP" version.
//
// Accumulate current uid's and flags in "CurrentUidMap"
//
////////////////////////////////////////////////////////////////////////

void GetCurrentUids (CTocDoc *pTocDoc, CUidMap& UidMap)
{
	unsigned long uid;

	// Clear list first.
	UidMap.DeleteAll ();

	// Sanity:
	if (! pTocDoc )
		return;

	// Disable preview.
//	pTocDoc->SetPreviewableSummary (NULL);

	CSumList &	listSums = pTocDoc->GetSumList();
	POSITION	pos = listSums.GetHeadPosition();
	CSummary *	pSum;

	while (pos)
	{
		pSum = listSums.GetNext(pos);
		if (pSum)
		{
			uid = pSum->GetHash();
#if 0  // JOK - 7/8/98 - Include sums with uid of 0 also so they could get deleted during a resync.
			if (uid != 0)
#endif 
			{
				UidMap.OrderedInsert (uid, pSum->m_Imflags, FALSE);
			}
		}
	}
}



//
// SHow that last IMAP error logged in the CImapConnection object specified by
// "pImap".
//
void  ShowLastImapError (CImapConnection *pImap)
{
	TCHAR  buf [512];
	int iType = IMAPERR_BAD_CODE;

	if (!pImap)
		return;

	buf[0] = '\0';

	pImap->GetLastImapErrorAndType (buf, 510, &iType);

	::TrimWhitespaceMT (buf);

	// If we didn't get a message, just put up a plain error message.
	if (!buf[0])
	{
		ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
		return;
	}

	// See what type of message it was.
	switch (iType)
	{
		case IMAPERR_USER_CANCELLED:
		case IMAPERR_LOCAL_ERROR:
			ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, buf);
			break;

		case IMAPERR_COMMAND_FAILED:
		case IMAPERR_CONNECTION_CLOSED:
			ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_SRVMSG, buf);
			break;

		// All other types:
		default:
			// If "buf" is non-empty, display that:
			//
			if (*buf)
			{ 
				ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, buf);
			}
			else
			{
				ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
			}
			break;
	}
}


// ImapBitflagsToFlaglist [EXPORTED]
// 
// Create a parenthesized, comma-separated IMAP flaglist given IMFLAGS_*
// bitflags. Write the result in szFlaglist.
// If ulBitflags don't contain proper bitflags, return an empty
// string, otherwise format a proper comma-separated string of flags.
//
void ImapBitflagsToFlaglist (unsigned ulBitflags, CString& szFlaglist)
{
	// In case of error:
	szFlaglist.Empty();

	// \\SEEN:
	if (ulBitflags & IMFLAGS_SEEN)
	{
		if ( szFlaglist.IsEmpty() )
			szFlaglist = "(\\SEEN";
		else
			szFlaglist += ", \\SEEN";
	}

	// \\ANSWERED:
	if (ulBitflags & IMFLAGS_ANSWERED)
	{
		if ( szFlaglist.IsEmpty() )
			szFlaglist = "(\\ANSWERED";
		else
			szFlaglist += ", \\ANSWERED";
	}

	// \\DELETED:
	if (ulBitflags & IMFLAGS_DELETED)
	{
		if ( szFlaglist.IsEmpty() )
			szFlaglist = "(\\DELETED";
		else
			szFlaglist += ", \\DELETED";
	}

	// \\FLAGGED:
	if (ulBitflags & IMFLAGS_FLAGGED)
	{
		if ( szFlaglist.IsEmpty() )
			szFlaglist = "(\\FLAGGED";
		else
			szFlaglist += ", \\FLAGGED";
	}

	// \\DRAFT:
	if (ulBitflags & IMFLAGS_DRAFT)
	{
		if ( szFlaglist.IsEmpty() )
			szFlaglist = "(\\DRAFT";
		else
			szFlaglist += ", \\DRAFT";
	}

	// Terminate parenthesis?
	if (!szFlaglist.IsEmpty())
		szFlaglist += ")";
}


// Return TRUE if the given name is INBOX:
//
BOOL IsInbox (LPCSTR pName)
{
	BOOL bRet = FALSE;

	if (pName)
	{
		bRet = ( stricmp (pName, "INBOX") == 0 );
	}

	return bRet;
}

// Return TRUE if the given name is Junk:
//
BOOL IsJunk (LPCSTR pName)
{
	BOOL bRet = FALSE;

	if (pName)
	{
		bRet = ( stricmp (pName, "Junk") == 0 );
		if (!bRet)
		{
			bRet = ( stricmp (pName, "INBOX.Junk") == 0 );
		}
	}

	return bRet;
}

// Return TRUE if the given name is Trash:
//
BOOL IsTrash (LPCSTR pName)
{
	BOOL bRet = FALSE;

	if (pName)
	{
		bRet = ( stricmp (pName, "Trash") == 0 );
	}

	return bRet;
}

// CopyTextFromTmpMbx [INTERNAL]
//
// If the summary has an offset into an MBX file, then it must be the 
// temporary MBX file. Go copy the text into the main MBX file.
//
BOOL CopyTextFromTmpMbx (CTocDoc* pTocDoc, CSummary* pSum)
{
	// Sanity:
	if (! (pTocDoc && pSum) )
		return FALSE;

	// Does the summary have am offset??
	//
	if ( pSum->IsNotIMAPDownloadedAtAll() )
	{
		return TRUE;
	}

	if (!pSum->m_Length)
	{
		return TRUE;
	}

	CImapMailbox* pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);
		return FALSE;
	}

	// Look for temporary MBX file.
	//
	CString szTmpMbxFilePath;

	GetTmpMbxFilePath (pImapMailbox->GetDirname(), szTmpMbxFilePath);

	if ( !FileExistsMT(szTmpMbxFilePath) )
	{
		// File sould have been here!!
		//
		ASSERT (0);
		return FALSE;
	}
	
	// Ok. Open both files.
	//
	CString szMainMbxFilePath = pTocDoc->GetMBFileName();

	JJFile TargetMBox;
	JJFile SrcMBox;

	if (FAILED(TargetMBox.Open(szMainMbxFilePath, O_RDWR | O_CREAT | O_APPEND)) ||
			FAILED(SrcMBox.Open(szTmpMbxFilePath, O_RDONLY)))
	{
		return FALSE;
	}

	// Get current offset into target file because we may need to truncate it if
	// failed.
	//
	long lOffset = -1;

	TargetMBox.Tell(&lOffset);
	
	if (FAILED(SrcMBox.JJBlockMove(pSum->m_Offset, pSum->m_Length, &TargetMBox)))
	{
		TargetMBox.ChangeSize(lOffset);
		return FALSE;
	}

	return TRUE;
}

// Utility function for closing or popping the progress.
//
void TerminateProgress (BOOL bWePutProgressUp, BOOL bWasInProgress)
{
	if ( bWePutProgressUp )
	{
		if (bWasInProgress)
			PopProgress();
		else
			CloseProgress();
	}
}




// GetNextUidChunk
// 
// Iterate through a comma-separated list of UID's.
// Parameters:
// "pStart	- pointer to the current start of the list.
// "szChunk - next comma-separated UID's
//
// Return: Pointer to where next to start, or NULL if list exhausted.
//
 
LPCSTR GetNextUidChunk (LPCSTR pStart, CString& szChunk)
{
	if (!pStart)
		return NULL;

	LPCSTR pNext = NULL;
	int nUids;

	for (pNext = pStart, nUids = 0; pNext && nUids < MAX_UIDS_PER_COMMAND; nUids++)
	{
		pNext = strchr (pNext + 1, comma);
	}

	// Copy to szChunk.
	if (pNext && pNext > pStart)
	{
		szChunk = CString (pStart, pNext - pStart);
	}
	else
	{
		// Last chunk.
		szChunk = pStart;
		pNext = NULL;
	}

	return pNext;
}

// 
//	FigureOutDelimiter
//	jdboyd 5/24/99
//
//	Return the first delimiter character found in the mailboxes belonging
//	to this account.  First, check the top level delimiter.  If no valid
//	delimiter character is found, scan the top level mailboxes for a delimiter.
//	Return it.
//

TCHAR FigureOutDelimiter(CImapAccount *pAccount)
{
	TCHAR d = '\0';

	d = pAccount->GetTopDelimiter();
	if ( d=='\0' || d==' ' )
	{
		// iterate through the top level of mailboxes looking for a delimiter
		QCMailboxCommand *pC;
		QCImapMailboxCommand *pIC;
		CString strDirectory; 
		
		// find the mailboxes that belong to this account
		pAccount->GetDirectory (strDirectory);
		pC = g_theMailboxDirector.FindByPathname( strDirectory );

		// only makes sense to do this if they're IMAP mailboxes
		if (pC->GetType() == MBT_IMAP_ACCOUNT)
		{
			// find the first IMAP mailbox
			CPtrList& ChildList = pC->GetChildList ();
			POSITION pos = ChildList.GetHeadPosition();
					
			// iterate through the mailboxes, stop when we find a valid delimiter
			while( pos && !d )
			{
				pIC = ( QCImapMailboxCommand* ) ChildList.GetNext( pos );
				if (pIC)					
				{
					d = pIC->GetDelimiter();
					if (d == ' ') d = '\0';	// space is an invalid delimiter character
				}
			}
		}
	}

	return (d);
}
#endif