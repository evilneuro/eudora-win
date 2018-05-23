// imapacct.cpp: implementation
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

#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "cursor.h"
#include "progress.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "password.h"
#include "font.h"
#include "station.h"
#include "utils.h"
#include "guiutils.h"
#include "ImapGuiUtils.h"

#ifdef WIN32
// #include "tooltip.h"	// for MAINFRM.H
#include "mainfrm.h"
#endif

#include "eudora.h"
#include "ems-wglu.h"
#include "trnslate.h"
#include "persona.h"

#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"

#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapActionQueue.h"
#include "ImapConnection.h"
#include "ImapLoginDlg.h"
#include "ImapMlistMgr.h"
#include "imapthread.h"
#include "QCTaskManager.h"
#include "QCWorkerThreadMT.h"

#include "DebugNewHelpers.h"


static CBitmap g_UnreadStatusBitmap;


// ========= GLobal externals =============/
extern QCMailboxDirector		g_theMailboxDirector;

//==================== Internals ==========================/
static const time_t tLoginCancelInterval = 2;		// seconds;

static const int		 iQueueFileNone = 0;
static const int		 iQueueFileExists = 1;
static const int		 iQueueFileAlphaExists = 1001;

// =================== Internal functions ====================

// ============ STATIC FUNCTIONS ======================/

void UpdateDelimiterChar (CImapConnection *pImap, CImapAccount *pAccount);

// Folder management.

// ============================================================================//

static void DoPostProcessing(void *pv, bool bLastTime);

class CQueueWriter : public QCWorkerThreadMT
{
public:
	CQueueWriter(CImapAccount *pImapAccount);
	~CQueueWriter();

	virtual HRESULT			 DoWork();				// Perform the queue writing.
	virtual void			 DoPostProcessing();	// Perform post processing in the main thread.
	virtual void			 RequestThreadStop();	// A thread stop was requested.

	CImapAccount			*m_pAccount;			// Account whose log to write.
};


CQueueWriter::CQueueWriter(CImapAccount *pImapAccount) :
	QCWorkerThreadMT(),
	m_pAccount(pImapAccount)
{
	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	CRString		strTitle(IDS_IMAP_SAVING);
	m_pTaskInfo->SetTitle(strTitle);
	if (m_pAccount)
	{
		m_pTaskInfo->SetPersona(m_pAccount->GetPersona());
	}
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);
}

CQueueWriter::~CQueueWriter()
{
}

HRESULT CQueueWriter::DoWork()
{
	m_pTaskInfo = NULL;
	if (m_pAccount)
	{
		m_pAccount->WriteQueue();
	}
	return S_OK;
}

void CQueueWriter::DoPostProcessing()
{
	m_pAccount->SetWritingQueue(false);
}

void CQueueWriter::RequestThreadStop()
{
}


void DoPostProcessing(void *pv, bool bLastTime)
{
	if (pv)
	{
		((CQueueWriter*)pv)->DoPostProcessing();
	}
}


// Class CImapAccount

IMPLEMENT_DYNAMIC(CImapAccount, CObject)

// Constructor
CImapAccount::CImapAccount()
{
	Init();
}



CImapAccount::CImapAccount(ACCOUNT_ID AccountID, LPCSTR pDirectory)
{
	// Call this first!!
	//
	Init();
	// Override these:
	//
	m_AccountID = AccountID;

	// Set this
	m_Directory = pDirectory;

	//reset this
	m_strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_strPersona );
	m_SSLSettings.GrabSettings();


	g_Personalities.SetCurrent( CachePersonality );

	ASSERT(!m_strPersona.IsEmpty());


	if (!m_strPersona.IsEmpty())
	{
		CString		 key;
		TCHAR		 szValue[32];

		// Get the UIDPLUS support setting from the INI entry for this personality.
		key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_UIDPLUS_FOUND);
		g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
		m_bSupportsUIDPLUS = ((szValue[0] == '1') || (szValue[0] == '\0'));

		// Get the ignore UIDPLUS support setting from the INI entry for this personality.
		key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_IGNORE_UIDPLUS);
		g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
		m_bIgnoreUIDPLUS = (szValue[0] == '1');

		// Get the ignore namespace setting from the INI entry for this personality.
		key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_IGNORE_NAMESPACE);
		g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
		m_bIgnoreNameSpace = (szValue[0] == '1');

		// Get the score junk setting from the INI entry for this personality.
		key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_SCORE_JUNK);
		g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
		m_bScoreJunk = ((szValue[0] == '1') || (szValue[0] == '\0'));
	}
}



// Init [PRIVATE]
//
void CImapAccount::Init ()
{
	m_AccountID = 0;
	m_Password.Empty();
	m_pParentList = NULL;

	m_pImap = NULL;

	// state
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;

	m_bIgnoreInbox		= TRUE;

	m_TopDelimiter		= 0;

	m_bGotNewMail	= FALSE;

	// Create the synchronization object.
	//
	m_pLockable = DEBUG_NEW CCriticalSection;

	// Initially unlocked.
	m_sAdvisoryLock = 0;

	// Exclusive lock.
	m_bExclusiveLock = FALSE;

	// Valid password??
	m_PasswordValidity = PWVALIDITY_UNKNOWN;

	// Set this so the first attempt to login will succeed!!
	//
	m_tLoginCancelled = 0x7FFFFFFF;

	m_pLoginLockable = DEBUG_NEW CCriticalSection;

	m_bPWNeedsSavingToDisk = FALSE;

	m_pActionQueue = NULL;
	m_bWritingQueue = false;

	// Set up the junk values.
	m_bJunkMailboxOK	= TRUE;		// Always set to true so we can check each time around.
	m_bWarnUserUIDPLUS	= FALSE;
	m_bSupportsUIDPLUS	= TRUE;
	m_bIgnoreUIDPLUS	= FALSE;
	m_bScoreJunk		= TRUE;

	m_bIgnoreNameSpace	= FALSE;
}





//
// ResetInternalState [PRIVATE]
//
// Reset state use in fetching mailboxes.
//
void CImapAccount::ResetInternalState ()
{
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;
	m_bIgnoreInbox		= TRUE;
}



CImapAccount::~CImapAccount()
{
	delete m_pLockable;
	delete m_pLoginLockable;
	delete m_pActionQueue;
}




// LOCK management.

// 
// AdviseLock [PUBLIC]
//
// Attempt to set an advisory lock on 
BOOL CImapAccount::SetAdvisoryLock (AccountLockType sAccountLock)
{
	BOOL bResult = FALSE;

	// First, attempt to acquire the synchronization object.
	//
	// Try to lock the global list.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return FALSE;
	}

	// We can now set the advisory lock.
	//
	switch (sAccountLock)
	{
		case ACCOUNT_LOCK_EXCLUSIVE:
			// Can't get the lock if soneone else has it.
			//
			if (IsExclusiveLocked())
			{
				bResult = FALSE;
			}
			else
			{
				LockExclusively();

				bResult = TRUE;
			}

			break;

		case ACCOUNT_LOCK_DONT_DELETE:
			IncrementAdvisoryLock ();

			bResult = TRUE;
			break;

		default:
			ASSERT(0);

			break;
	}

	return bResult;
}




// 
// RemoveAdviseLock [PUBLIC]
//
// Attempt to set an advisory lock on 
BOOL CImapAccount::RemoveAdvisoryLock (AccountLockType sAccountLock)
{
	BOOL bResult = FALSE;

	// First, attempt to acquire the synchronization object.
	//
	// Try to lock the global list.
	//
	CSingleLock localLock (m_pLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return FALSE;
	}

	// We can now set the advisory lock.
	//
	switch (sAccountLock)
	{
		case ACCOUNT_LOCK_EXCLUSIVE:
			// We must have had it. Note: We can possibly be stealing someone
			// else's lock!!!
			//
			if (!IsExclusiveLocked())
			{
				bResult = FALSE;
			}
			else
			{
				UnLockExclusively();

				bResult = TRUE;
			}

			break;

		case ACCOUNT_LOCK_DONT_DELETE:
			DecrementAdvisoryLock ();

			bResult = TRUE;
			break;

		default:
			ASSERT(0);

			break;
	}

	return bResult;
}


// IsExclusiveLocked [PUBLIC]
//
BOOL CImapAccount::IsExclusiveLocked()
{
	return m_bExclusiveLock;
}



// LockExclusively [PRIVATE]
//
// Make some validity checks!
//
void CImapAccount::LockExclusively()
{
	if (m_bExclusiveLock)
	{
		ASSERT (0);
	}

	m_bExclusiveLock = TRUE;
}



// UnLockExclusively [PRIVATE]
//
// Make some validity checks!
//
void CImapAccount::UnLockExclusively()
{
	if (!m_bExclusiveLock)
	{
		ASSERT (0);
	}

	m_bExclusiveLock = FALSE;
}


// IncrementAdvisoryLock [PRIVATE]
//
void CImapAccount::IncrementAdvisoryLock()
{
	m_sAdvisoryLock++;

	if (m_sAdvisoryLock <= 0)
	{
		ASSERT (0);
		m_sAdvisoryLock = 1;
	}
}


// DecrementAdvisoryLock [PRIVATE]
//
void CImapAccount::DecrementAdvisoryLock()
{	
	m_sAdvisoryLock--;

	if (m_sAdvisoryLock < 0)
	{
		ASSERT (0);

		m_sAdvisoryLock = 0;
	}
}

	

//===================================================================//


//
//	CImapAccount::SetupControlStream()
//
//	If we don't have a control stream object yet create one.  Once we have a valid object update
//	its network settings.
//
//	Note: We need to investigate the possibility of keeping the control stream around for the
//	lifetime of this object and alternately calling OpenControlStream() and Close().
//
bool CImapAccount::SetupControlStream(CString &strLogin, CString &strPassword)
{
	// Create a new control stream to the server if we don't already have one.
	if (!m_pImap)
	{
		CString				 strAddress;
		GetAddress(strAddress);

		m_pImap = DEBUG_NEW CImapConnection(strAddress);

		if (!m_pImap)
		{
			return false;
		}

		CString				 szPortNum;
		GetPortNum(szPortNum);
		m_pImap->SetPortNum(szPortNum);

		m_pImap->SetAccount(this);

		if (strLogin.IsEmpty() || strPassword.IsEmpty())
		{
			Login(strLogin, strPassword);
		}

//		if (strPassword.IsEmpty())
//		{
//			GetPasswordMT(strPassword);
//		}

		m_pImap->SetLogin(strLogin);
		m_pImap->SetPassword(strPassword);
	}

	// Set/Refresh the network settings for this account.  You should always do this before opening
	// the control stream to make sure we have the latest settings.
	m_pImap->UpdateNetworkSettings();

	return true;
}


//
//	CImapAccount::InitQueue()
//
//	Initialize the action queue for this account and read it from disk if needed.
//
void CImapAccount::InitQueue()
{
	CRString		 strFileName(IDS_IMAP_QUEUE_FILENAME);
	CString			 strFullName = m_Directory;
	strFullName += "\\";
	strFullName += strFileName;

	m_pActionQueue = DEBUG_NEW CActionQueue(m_AccountID, strFullName, m_Directory);

	if (m_pActionQueue)
	{
		ReadQueue();
	}
}


//
//	CImapAccount::QueueAction()
//
//	Adds the specified action to the action queue for this account.
//
void CImapAccount::QueueAction(CImapAction *pImapAction)
{
	if (m_pActionQueue)
	{
		m_pActionQueue->Add(pImapAction);
	}
}

//
//	CImapAccount::IsProcessing()
//
//	Returns true if an action is currently being processed for this account, false otherwise.
//
bool CImapAccount::IsProcessing()
{
	if (m_pActionQueue)
	{
		return m_pActionQueue->IsProcessing();
	}
	return false;
}

//
//	CImapAccount::ProcessQueue()
//
//	Performs idle processing on the action queue for this account.
//
//	Returns true if an action was performed, false if there is nothing to do.
//
bool CImapAccount::ProcessQueue(bool bHighPriorityOnly, bool bOnlineStateChanged)
{
	if (m_pActionQueue)
	{
		return m_pActionQueue->OnIdle(bHighPriorityOnly, bOnlineStateChanged);
	}
	return false;
}

//
//	CImapAccount::MaybeSaveQueue()
//
//	If conditions are right, save the action queue to disk.
//
//	Returns true if the queue was written, false if a write isn't needed.
//
bool CImapAccount::MaybeSaveQueue(bool bDoItNow)
{
	if (m_pActionQueue && m_pActionQueue->NeedsWrite() && !m_bWritingQueue)
	{
		// If we got here then the queue has changed since the last time we wrote it and we are not currently
		// writing the file in a thread.
		if (m_pActionQueue->GetCount() == 0)
		{
			// The queue is dirty and empty (presumably we just performed the last item in it) so go ahead
			// and write it now.  In this case we don't bother with a worker thread because writing an empty
			// queue probably takes about the same amount of time as starting a new thread.
			m_bWritingQueue = true;
			WriteQueue();
			m_bWritingQueue = false;
			return true;
		}
		else if (bDoItNow)
		{
			// Someone wants us to write the queue right now (probably Eudora is quitting).  No time to
			// bother with a thread.
			m_bWritingQueue = true;
			WriteQueue();
			m_bWritingQueue = false;
			return true;
		}
		else if ((m_pActionQueue->GetCount() > 1) || GetIniShort(IDS_INI_CONNECT_OFFLINE))
		{
			// Either we are offline (in which case we always write) or there is more than 1 item in the
			// queue.  The minimum number of items to trigger a write is debatable.  Certainly we don't want
			// to bother writing a queue of 1 item because it will take just as long to start the thread to
			// write the queue as it will take to start the thread to do the action.
			m_bWritingQueue = true;
			SetIniInt(IDS_INI_IMAP_OFFLINE_QUEUE_EXISTS, iQueueFileAlphaExists);
			CQueueWriter		*pQueueWriter = DEBUG_NEW_NOTHROW CQueueWriter(this);
			QCGetTaskManager()->QueueWorkerThread(pQueueWriter);
			return true;
		}
	}
	return false;
}

//
//	CImapAccount::WriteQueue()
//
//	Writes the action queue to disk.
//
//	Returns true the write succeeded, false otherwise.
//
bool CImapAccount::WriteQueue()
{
	if (m_pActionQueue)
	{
		return m_pActionQueue->WriteQueue();
	}
	return false;
}

//
//	CImapAccount::ReadQueue()
//
//	Reads the action queue from disk.
//
//	Returns true the read succeeded, false otherwise.
//
bool CImapAccount::ReadQueue()
{
	if (GetIniShort(IDS_INI_IMAP_OFFLINE_QUEUE_EXISTS) ==  iQueueFileNone)
	{
		// Nothing to read, no failure.
		return true;
	}
	if (m_pActionQueue)
	{
		return m_pActionQueue->ReadQueue();
	}
	return false;
}

//
//	CImapAccount::FinishUpQueue()
//
//	Performs clean up of the action queue for this account.
//
void CImapAccount::FinishUpQueue()
{
	if (m_pActionQueue)
	{
		m_pActionQueue->FinishUp();
	}
}

//
//	CImapAccount::HasHighPriorityItemsInQueue()
//
//	Returns true if this account has high priority items in its queue, false otherwise.
//
bool CImapAccount::HasHighPriorityItemsInQueue()
{
	if (m_pActionQueue)
	{
		return m_pActionQueue->HasHighPriorityItems();
	}
	return false;

}

//
//	CImapAccount::ActionsQueuedForMailbox()
//
//	Returns true if an action is pending on the specified mailbox, false otherwise.
//
bool CImapAccount::ActionsQueuedForMailbox(CImapMailbox *pImapMailbox)
{
	if (!pImapMailbox || !m_pActionQueue)
	{
		return false;
	}

	return (m_pActionQueue->GetNextMailboxMatch(HashMT(pImapMailbox->GetImapName())) != NULL);
}

//
//	CImapAccount::DoJunkScoring()
//
//	Indicates whether conditions are right to do junk scoring.
//	This is the default and will only be false if UIDPLUS is not
//	supported and the user told us to turn off junk scoring.
//
BOOL CImapAccount::DoJunkScoring()
{
	return m_bScoreJunk;
}

//
//	CImapAccount::UseJunkMailbox()
//
//	Indicates whether conditions are right to do use the junk mailbox.
//	This means there was no failure finding or creating the Junk mailbox
//	(note that this may simply mean we haven't yet tried to find or
//	create the Junk mailbox), UIDPLUS is supported and the user hasn't
//	told Eudora to ignore UIDPLUS data.
//
BOOL CImapAccount::UseJunkMailbox()
{
	return (m_bSupportsUIDPLUS && !m_bIgnoreUIDPLUS && m_bJunkMailboxOK);
}

//
//	CImapAccount::SetSupportsUIDPLUS()
//
//	Does the appropriate bookkeeping if we discover that UIDPLUS data
//	is not being sent.
//
void CImapAccount::SetSupportsUIDPLUS(BOOL bSupports)
{
	if (m_bSupportsUIDPLUS != bSupports)
	{
		m_bSupportsUIDPLUS = bSupports;
		m_bWarnUserUIDPLUS = TRUE;

		// Set the UIDPLUS support INI entry for this personality.
		CString key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_UIDPLUS_FOUND);

		// Get our personality name.
		CString strPersona = g_Personalities.ImapGetByHash(m_AccountID);

		if (!strPersona.IsEmpty())
		{
			// Write to disk.
			TCHAR	 szValue[2];
			if (m_bSupportsUIDPLUS)
			{
				strcpy(szValue, "1");
			}
			else
			{
				strcpy(szValue, "0");
			}
			g_Personalities.WriteProfileString(strPersona, key, szValue);
		}
	}
}

//
//	CImapAccount::WarnIfUIDPLUSStatusChanged()
//
//	If the UIDPLUS data state has changed warn the user.
//
void CImapAccount::WarnIfUIDPLUSStatusChanged()
{
	if (m_bWarnUserUIDPLUS)
	{
		if (GetCurrentPaidMode() == EMS_ModePaid)
		{
			// Warn the user only if in Paid mode, otherwise they don't get SpamWatch.
			CString			 strName;
			GetName(strName);
			if (SupportsUIDPLUS())
			{
				// Tell the user that UIDPLUS is now supported and SpamWatch is re-enabled.
				ErrorDialog(IDS_IMAP_UIDPLUS_SUPPORT, strName);
				m_bScoreJunk = TRUE;
			}
			else
			{
				// Tell the user that UIDPLUS is not supported and ask them if they want to
				// continue scoring junk (perhaps so they can filter on junk score themselves).
				if (YesNoDialogPlain(IDS_IMAP_NO_UIDPLUS_SUPPORT, strName) == IDOK)
				{
					m_bScoreJunk = FALSE;
				}
				else
				{
					m_bScoreJunk = TRUE;
				}
			}

			// Set the score junk INI entry for this personality.
			CString key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_SCORE_JUNK);
			CString strPersona = g_Personalities.ImapGetByHash(m_AccountID);
			if (!strPersona.IsEmpty())
			{
				// Write to disk.
				TCHAR	 szValue[2];
				if (m_bScoreJunk)
				{
					strcpy(szValue, "1");
				}
				else
				{
					strcpy(szValue, "0");
				}
				g_Personalities.WriteProfileString(strPersona, key, szValue);
			}
		}
		m_bWarnUserUIDPLUS = FALSE;
	}
}

//
// CImapAccount::SetNameSpace()
//
// If the user has not set a prefix and the user has not told us to
// ignore the server's namespace, record the server's namespace for
// future reference (see GetPrefix()).
//
// We should probably considering doing something if the user does
// specify a prefix but it differs from the namespace.  Something
// for a future version. -dwiggins
//
void CImapAccount::SetNameSpace(CString &strNameSpace)
{
	CString		 strPrefix;
	GetPrefix(strPrefix);
	{
		if (strPrefix.IsEmpty() && !m_bIgnoreNameSpace)
		{
			m_strNameSpace = strNameSpace;
		}
	}
}

/////////////////////////////////
// InitializeDir [PUBLIC]	
// Create the directory if it doesn't already exist.
// 
BOOL CImapAccount::InitializeDir ()
{
	CString strDir;

	GetDirectory(strDir);

	if (strDir.IsEmpty())
		return FALSE;

	// If the directory exists, OK.
	if (DirectoryExists (strDir))
		return TRUE;
	else
	{
		// Doesn't exist. Try to create.
		if (!CreateDirectory((LPCSTR) strDir, NULL))
			return FALSE;
		else
			return TRUE;
	}
}



// FetchChildFolders [PUBLiC]
// Fetch the list of child mailboxes of the given mailbox name. The "mailbox" may be an account,
// a "namespace" or a mailbox.
// Return a "ImapMailboxNode" tree structure.
HRESULT CImapAccount::FetchChildMailboxList (LPCSTR pParentImapname, TCHAR Delimiter, ImapMailboxType Type, ImapMailboxNode **ppTopmostNode, BOOL Recurse)
{
	BOOL				IncludeInbox = FALSE;
	ImapMailboxNode		*pTopNode = NULL;
	HRESULT				hResult = E_FAIL;

	// Sanity
	if ( ! (pParentImapname && ppTopmostNode) )
		return E_INVALIDARG;

	// Make sure:
	//
	*ppTopmostNode = NULL;

	// Must have a server address.
	CString strAddress; GetAddress (strAddress);
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_strPersona );
	m_SSLSettings.GrabSettings();


	g_Personalities.SetCurrent( CachePersonality );


	CImapConnection Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return HRESULT_MAKE_CANCEL;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// Clear current state:
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;
	m_bIgnoreInbox		= TRUE;

	// If we are starting at the account level, include inbox.
	if (IMAP_ACCOUNT == Type)
	{
		IncludeInbox = TRUE;
		m_bIgnoreInbox = FALSE;
	}

	// Put up progress immediately:
	//
	CImmediateProgress Imp;

	// Put up progres dialog if in main thread.
	if ( ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format ( CRString(IDS_IMAP_FETCHING_MAILBOXES), strAddress );
		MainProgress(buf);
	}


	hResult = Imap.OpenControlStream(this);

	if ( SUCCEEDED(hResult) )
	{
		// If parent is an Account and there's no delimiter, go get it.
		if ((IMAP_ACCOUNT == Type) && (Delimiter == 0))
			UpdateDelimiterChar (&Imap, this);

		__FetchChildMailboxList (pParentImapname, Delimiter, &pTopNode, &Imap, 0, IncludeInbox,  Recurse);

		// Return what we got:
		//
		*ppTopmostNode = pTopNode;
	}

	// Reset these:
	m_curLevel			= 0;
	m_curReference.Empty();
	m_curParentMailbox	= NULL;
	m_curMailboxList	= NULL;
	m_bIgnoreInbox		= TRUE;

	if ( ::IsMainThreadMT() )
	{
		CloseProgress();

		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}

	return hResult;
}



//
// ====== Methods that query name, login, etc, are routed to the personality.
// ====== Note: An account's m_AccountID is a hash of the personality name.
//

//
// Get the prefix from the INI file.
// Copy the result into the given string.
//
// NOTE:
// Don't use GetIniString here because it reads from [Settings] if
// there is no persona prefix.
// END NOTE

BOOL CImapAccount::GetPrefix(CString& strBuf)
{
	// ALways do this first.
	strBuf.Empty();

	// If we have a non-empty m_strNameSpace field then use that instead
	// of the actual prefix value (see SetNameSpace() for details).
	if (!m_strNameSpace.IsEmpty())
	{
		strBuf = m_strNameSpace;
		return TRUE;
	}

	CString key = g_Personalities.GetIniKeyName( IDS_INI_PERSONA_IMAP_PREFIX );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// Note: The prefix can be empty.
	return TRUE;
}


//
// NOTES
// Copy the result into the given string.
// END NOTES
//
BOOL CImapAccount::GetName(CString& strBuf)
{
	// Find the personality corresponding to the account ID and copy the name.
	strBuf = g_Personalities.ImapGetByHash( m_AccountID );

	// An empty name means it's the dominant personality.
	if ( strBuf.IsEmpty() )
	{
		strBuf = CRString (IDS_DOMINANT);
	}

	return !strBuf.IsEmpty();
}



//
// Copy the result into the given string.
//

BOOL CImapAccount::GetLogin(CString& strBuf)
{
	// Must be called in the made thread.
	ASSERT ( IsMainThreadMT() );

	// ALways do this first.
	strBuf.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_LOGIN_NAME );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// Note: It's an error if no login name.
	return !strBuf.IsEmpty();
}



// MustSavePasswordToDisk [PUBLIC]
//
// Determine if this account's password must be saved to disk.
//
BOOL CImapAccount::MustSavePasswordToDisk ()
{
	// 
	//CString key = g_Personalities.GetIniKeyName( IDS_INI_SAVE_PASSWORD );

	// Get our personality name.
	//CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	//if ( strPersona.IsEmpty() )
	//	return FALSE;

	//Set to the persona to current
	CString	CachePersonality =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( m_strPersona );

	BOOL bSavePass = GetIniShort(IDS_INI_SAVE_PASSWORD) > 0?TRUE:FALSE;

	g_Personalities.SetCurrent( CachePersonality );

	return bSavePass;

	//
	// Read from disk.
	//
	//return (g_Personalities.GetProfileInt (strPersona, key, 0) > 0);
}



//
// Copy the result into the given string.
//

BOOL CImapAccount::GetPasswordMT(CString& strBuf)
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return FALSE;
	}

	// Just copy internal data member.
	strBuf = m_Password;

	return !strBuf.IsEmpty();
}



// 
// Return TRUE if the user has anything other than Password" clicked.
//
BOOL CImapAccount::IsAllowAuthenticate ()
{
	CString key1 = g_Personalities.GetIniKeyName( IDS_INI_AUTH_APOP );
	CString key2 = g_Personalities.GetIniKeyName( IDS_INI_AUTH_KERB );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Read from disk.
	// Return TRUE if CRAM-MD5 or Kerberos is selected.
	//
	return (g_Personalities.GetProfileInt (strPersona, key1, 0) > 0) ||
		   (g_Personalities.GetProfileInt (strPersona, key2, 0) > 0);
}


// 
// Return TRUE if the user has anything other than Password" clicked.
//
BOOL CImapAccount::IsAuthKerberos ()
{
//	ASSERT ( IsMainThreadMT() );

	CString key2 = g_Personalities.GetIniKeyName( IDS_INI_AUTH_KERB );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Read from disk.
	// Return TRUE if CRAM-MD5 or Kerberos is selected.
	//
	return (g_Personalities.GetProfileInt (strPersona, key2, 0) > 0);
}


//
//	CImapAccount::GetInMailbox()
//
//	Return a QCImapMailboxCommand pointer to this account's In mailbox.
//
//	Parameters:
//		strImapName[out] - CString for holding name of the In mailbox.
//
//	Return:
//		Pointer to a QCImapMailboxCommand pointer to this account's in mailbox.
//		NULL if mailbox was not found.
//
QCImapMailboxCommand *CImapAccount::GetInMailbox(CString &strImapName)
{
	QCImapMailboxCommand	*pCommand = NULL;
	CRString				 szImapInboxName(IDS_IMAP_RAW_INBOX_NAME);
	strImapName = szImapInboxName;
	if (!strImapName.IsEmpty())
	{
		// Find the mailbox in our local mailbox tree.
		pCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
														   szImapInboxName,
														   0/*pImapDelimiter*/);
	}

	return pCommand;
}

// UseFancyTrash [PUBLIC]
//
// 	Return TRUE if to move to TRASH on delete.
//
BOOL CImapAccount::UseFancyTrash()
{
	CString key2 = g_Personalities.GetIniKeyName( IDS_INI_IMAP_XFERTOTRASH );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Read from disk.
	//
	return (g_Personalities.GetProfileInt (strPersona, key2, 0) > 0);
}




// GetTrashMailboxName [PUBLIC]
//
// Go read the persona's specified trash mailbox name.
// Note: This is the full IMAP name of the mailbox.
//
BOOL CImapAccount::GetTrashMailboxName(CString &strBuf, CString &strFullName, TCHAR *pcDelimiter)
{
	ASSERT ( IsMainThreadMT() );

	// ALways do this first.
	strBuf.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_IMAP_TRASH_MBOXNAME );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// If empty, see what's in the default.
	//
	if ( strBuf.IsEmpty() )
		strBuf = GetIniString (IDS_INI_IMAP_TRASH_MBOXNAME);

	// The name (which should not be empty at this point) should contain the mailbox name.
	// We now need to prepend the prefix so we have the full name of the mailbox.
	GetPrefix(strFullName);
	if (!strFullName.IsEmpty())
	{
		char		*szBuf = strFullName.GetBuffer(0);
		*pcDelimiter = szBuf[strFullName.GetLength() - 1];
	}
	// If the prefix has not already been prepended then prepend it now.  Checking for the prefix
	// is necessary because earlier versions of Eudora prepended the prefix to the mailbox name when
	// you specified the Trash mailbox.
	if (strBuf.Find(strFullName) != 0)
	{
		strFullName += strBuf;
	}
	else
	{
		strFullName = strBuf;
	}

	return TRUE;
}


//
//	CImapAccount::GetJunkMailbox() [PUBLIC]
//
//	Return a QCImapMailboxCommand pointer to this account's junk mailbox.
//	If this mailbox doesn't exist try to create it.
//
//	Parameters:
//		strImapName[out] - CString for holding name of the Junk mailbox.
//		bCreate [in] - If true attempt to create the mailbox if it was not found.
//
//	Return:
//		Pointer to a QCImapMailboxCommand pointer to this account's junk mailbox.
//		NULL if mailbox was not found and could not be created.
//
QCImapMailboxCommand *CImapAccount::GetJunkMailbox(CString &strImapName, bool bCreate)
{
	QCImapMailboxCommand	*pCommand = NULL;
	CString					 strName;
	TCHAR					 cDelimiter = 0;

	// Get desintation mailbox.
	GetJunkMailboxName(strName, strImapName, &cDelimiter);

	if (!strImapName.IsEmpty())
	{
		// Look for the mailbox at the root of our local mailbox tree.
		pCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
														   strImapName,
														   cDelimiter);
	}

	// If Junk mailbox doesn't exist, try to create it now.
	if (!pCommand && bCreate)
	{
		CString				 strParentName;
		QCMailboxCommand	*pCommandParent = NULL;

		GetDirectory(strParentName);
		pCommandParent = g_theMailboxDirector.FindByPathname(strParentName);

		if (pCommandParent)
		{
			if (!GetImapAccountMgr()->MailboxExistsOnServer(GetAccountID(), strImapName))
			{
				pCommand = (QCImapMailboxCommand*)g_theMailboxDirector.AddCommand(strName,
																				  MBT_IMAP_MAILBOX,
																				  pCommandParent);
			}
		}
	}

	// Still no command, give up.
	if (!pCommand && bCreate)
	{
		m_bJunkMailboxOK = FALSE;

		CRString	 fmt(IDS_ERR_IMAP_INVALID_MBOXNAME);
		CString		 szText;

		szText.Format (fmt, strImapName);
		ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, szText);
	}

	return pCommand;
}

//
//	CImapAccount::GetJunkMailboxName() [PUBLIC]
//
//	Go read the persona's specified junk mailbox name.
//	Note: This is the full IMAP name of the mailbox.
//
BOOL CImapAccount::GetJunkMailboxName(CString &strName, CString& strFullName, TCHAR *pcDelimiter)
{
	ASSERT(IsMainThreadMT());

	// Always do this first.
	strName.Empty();
	strFullName.Empty();

	// Get the junk mailbox name from the INI entry for this personality.
	CString key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_JUNK_MBOXNAME);

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash(m_AccountID);

	if (strPersona.IsEmpty())
	{
		return FALSE;
	}

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strName = szValue;

	// Trim!!
	strName.TrimRight();
	strName.TrimLeft();

	// If empty, see what's in the default.
	//
	if (strName.IsEmpty())
	{
		strName = GetIniString(IDS_INI_IMAP_JUNK_MBOXNAME);
	}

	// The name (which should not be empty at this point) should contain the mailbox name.
	// We now need to prepend the prefix so we have the full name of the mailbox.
	GetPrefix(strFullName);
	if (!strFullName.IsEmpty())
	{
		char		*szBuf = strFullName.GetBuffer(0);
		*pcDelimiter = szBuf[strFullName.GetLength() - 1];
	}
	strFullName += strName;

	return TRUE;
}


//
// Cache the account's password.
//
void CImapAccount::SetPasswordMT (LPCSTR pPassword)
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return;
	}

	// Cache the password locally.
	m_Password = pPassword;

	//
	// Also, if this is the dominant personality, save the password in
	// POPPassword.

	// Find the personality corresponding to the account ID.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return;

	if (g_Personalities.IsDominant (strPersona) )
	{
		// Copy to the CString.
		POPPassword = pPassword;
	}
}




//
// Go read the persona's saved password.
//
BOOL CImapAccount::GetSavedPassword (CString& Password)
{
	// Must be called in the main thread!
	//
	ASSERT( IsMainThreadMT() );

	// Clear parm.
	Password.Empty();

	CString key = g_Personalities.GetIniKeyName( IDS_INI_SAVE_PASSWORD_TEXT );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	Password = ::DecodePassword(szValue);

	return TRUE;
}



// 
// Save the cached password to disk, even if it's blank.
// NOTE: Can use this to clear the password as well.
//
BOOL CImapAccount::SavePasswordToDisk ()
{
	// Must be called in the main thread!
	//
	//ASSERT( IsMainThreadMT() );

	//CString key = g_Personalities.GetIniKeyName( IDS_INI_SAVE_PASSWORD_TEXT );
	CRString key(IDS_INI_SAVE_PASSWORD_TEXT);

	// Get our personality name.
	//CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	CString szPw = m_Password;

	// Note: we may want to save a blank password!
	if (!szPw.IsEmpty())
		szPw = EncodePassword(szPw);

	// Encode and write.
	g_Personalities.WriteProfileString( m_strPersona, key, szPw );

	return TRUE;
}	





//
// Return the server name or IP address.
// Copy the result into the given string.
//

BOOL CImapAccount::GetAddress(CString& strBuf)
{
	// ALways do this first.
	strBuf.Empty();


#if 0 // JOK

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	// Find the personality corresponding to the account ID.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Temporarily switch to this personality so that we can get the value.
	//
	if (g_Personalities.SetCurrent(strPersona))
	{
		 strBuf = GetIniString ( IDS_INI_PERSONA_POP_SERVER );

		// Strip any text after a colon (possible "address:port")
		int nc = strBuf.Find (':');
		if (nc >= 0)
			strBuf = strBuf.Left (nc);

		// Trim!!
		strBuf.TrimRight();
		strBuf.TrimLeft();
	}

	//
	// Reset current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);

	// Note: It's an error if no address.
	return !strBuf.IsEmpty();

#endif 


	CString key = g_Personalities.GetIniKeyName( IDS_INI_PERSONA_POP_SERVER );

	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	// Copy the value to the output string.
	strBuf = szValue;

	// Strip any text after a colon (possible "address:port")
	int nc = strBuf.Find (':');
	if (nc >= 0)
		strBuf = strBuf.Left (nc);

	// Trim!!
	strBuf.TrimRight();
	strBuf.TrimLeft();

	// Note: It's an error if no address.
	return !strBuf.IsEmpty();
}



// FUNCTION
// Get the server address from the persona to see if it's of the
// form "address:port_num". Otherwiase, return the IMAP default.
// END FUNCTION

//
// Return the server name or IP address.
// Copy the result into the given string.
//

BOOL CImapAccount::GetPortNum(CString& strBuf)
{
	// This is the default.
	strBuf = CRString (IDS_IMAP_DEFAULT_PORT);


#if 0 // JOK

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	// Find the personality corresponding to the account ID.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );
	if ( strPersona.IsEmpty() )
		return FALSE;

	//
	// Temporarily switch to this personality so that we can get the value.
	//
	if (g_Personalities.SetCurrent(strPersona))
	{
		 CString szServer = GetIniString ( IDS_INI_PERSONA_POP_SERVER );

		// Look for ":"
		int nc = szServer.Find (':');
		if (nc >= 0)
		{
			strBuf = szServer.Mid (nc + 1);

			// Trim!!
			strBuf.TrimRight();
			strBuf.TrimLeft();
		}
	}

	//
	// Reset current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);

	// Note: It's an error if no address.
	return !strBuf.IsEmpty();

#endif // JOK


	// Get our personality name.
	CString strPersona = g_Personalities.ImapGetByHash( m_AccountID );

	if ( strPersona.IsEmpty() )
		return FALSE;

	// Read from disk.
	TCHAR szValue [512];

	*szValue = 0;

	// Look for the IMAPPort INI setting first:
	//
	CString key = g_Personalities.GetIniKeyName( IDS_INI_IMAP_PORT );

	g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

	CString szTmp = szValue;
	szTmp.TrimRight();
	szTmp.TrimLeft();

	// Did we get anything?
	if ( szTmp.IsEmpty() )
	{
		key = g_Personalities.GetIniKeyName( IDS_INI_PERSONA_POP_SERVER );

		*szValue = 0;

		g_Personalities.GetProfileString(strPersona, key, "", szValue, sizeof(szValue));

		// Copy the value to the output string.
		CString szServer = szValue;

		// Look for ":"
		int nc = szServer.Find (':');
		if (nc >= 0)
		{
			szTmp = szServer.Mid (nc + 1);

			szTmp.TrimRight();
			szTmp.TrimLeft();
		}
	}

	// Did we get?
	if ( !szTmp.IsEmpty() )
	{
		strBuf = szTmp;
	}

	// Note: It's an error if no portnum.
	return !strBuf.IsEmpty();
}




//
// Return the server name or IP address.
// Copy the result into the given string.
//

BOOL CImapAccount::GetDirectory(CString& strBuf)
{
	// Just copy internal data member.
	strBuf = m_Directory;

	return !strBuf.IsEmpty();
}




// ModifyFromPersonality [PUBLIC]
////////////////////////////////////////////////////////////////////////////////
// Called when a new personality is created and when an existing personality
// is modified.
// If login name or server address has changed, we may have to refresh the 
// mailbox list and the local cache. Ask user if to do that, then set output 
// parameter: "MboxListNeedsUpdating" to TRUE and return.

/////////////////////////////////////////////////////////////////////////////////

BOOL CImapAccount::ModifyFromPersonality (BOOL& MboxlistNeedsUpdating, BOOL IsNewAccount /* = FALSE */)
{
	// Initialize this for return.
	MboxlistNeedsUpdating = FALSE;
	
	// If this is a new account, just set internal values.
	if (IsNewAccount)
	{
		GetPrefix	(m_Prefix);
		GetLogin	(m_Login);
		GetAddress	(m_Address);
	}
	else
	{
		// we need to determine what has changed.
		CString szPrefix;	GetPrefix (szPrefix);
		CString szAddress;  GetAddress (szAddress);
		CString szLogin;	GetLogin (szLogin);

		// Has login changed?
		if (m_Login.Compare (szLogin) != 0)
		{
			MboxlistNeedsUpdating = TRUE;

			// Clear password.
			SetPasswordMT ("");
		}

		// Has prefix changed?
		if (m_Prefix.Compare (szPrefix) != 0)
		{
			MboxlistNeedsUpdating = TRUE;

			// We need to modify the ImapName in the account's command object,
			// because the prefix is the ImapName of an account object.
			//
			// Find the top level command object for this account.
			CString strDirectory; GetDirectory (strDirectory);

			QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( strDirectory );

			ASSERT (pCommand);

			if (pCommand)
			{
				ASSERT ( pCommand->IsKindOf ( RUNTIME_CLASS( QCImapMailboxCommand ) ) );

				ASSERT ( pCommand->GetType() == MBT_IMAP_ACCOUNT );

				(( QCImapMailboxCommand * ) pCommand)->SetImapName (szPrefix);
			}
		}

		// Has address changed?
		if (m_Address.Compare (szAddress) != 0)
		{
			MboxlistNeedsUpdating = TRUE;

			// Clear delimiter and password.
			SetPasswordMT ("");

			SetTopDelimiter ('\0');
		}

		// We must update in any case.
		m_Prefix	= szPrefix;
		m_Login		= szLogin;
		m_Address	= szAddress;

		// Ask user? Do this whether we have a connection to the current server
		// or not.
		if ( MboxlistNeedsUpdating )
		{
			// Clear the top level delimiter.

			SetTopDelimiter ('\0');
		}
	}

	return TRUE;
}




// __FetchChildFolders [PRIVATE]
// NOTES
// Do the hard work of the fetch.
// END NOTES

void CImapAccount::__FetchChildMailboxList (LPCSTR pParentImapname, TCHAR Delimiter, ImapMailboxNode **ppNode, CImapConnection *pImap, short Level, BOOL IncludeInbox,  BOOL Recurse)
{
	ImapMailboxNode		*pTopNode = NULL;
	CString				Reference;

	// Sanity
	if (!(pParentImapname && ppNode && pImap))
		return;

	// Set ppNode into the current mailbox address so stuff gets aded to it.
	m_curMailboxList = ppNode;

	// Construct parent reference.
	Reference = pParentImapname;

	// If we have a delimiter, make sure the name ends in it.
	if (!Reference.IsEmpty() && Delimiter)
	{
		if (Reference.Right ( 1 ) != Delimiter)
		{
			Reference += Delimiter;
		}
	}

	// Set the m_Level to zero since this is the top of the new ImapMailboxNode tree.
	m_curLevel = Level;

	// Set current reference string.
	m_curReference = Reference;
		
	// Go fetch the names of the mailboxes. 
	pImap->MailFetchThisLevel (Reference, this, IncludeInbox);
	
	// Did we get any folders??
	if (*ppNode)
	{
		// Do we recurse?
		if (Recurse)
		{
			pTopNode = *ppNode;
			while (pTopNode)
			{
				// Recurse if not "NoInferiors". Also, MUST have a delimiter.
				if (pTopNode->Delimiter && (!pTopNode->NoInferiors))
				{
					// Set current parent mailbox.
					m_curParentMailbox = *ppNode;
					__FetchChildMailboxList (pTopNode->pImapName, pTopNode->Delimiter, &(pTopNode->ChildList), pImap, (short)(Level + 1), FALSE,  TRUE);

					// Reset our level
					m_curLevel = Level;
				}

				pTopNode = pTopNode->SiblingList;
			}
		}
	}
}
				



// Insert
// Insert a new mailbox into the mailbox tree of the account, at the
// correct level and in the correct order.
// Return the position (in the sibling list at that level) at which it is inserted, or -1.
// 
int CImapAccount::Insert(const char *mailbox, TCHAR delimiter, long attributes, ImapMailboxType Type)
{
	short i;
	ImapMailboxNode *pFolder = NULL, *pPrevFolder = NULL;
	CString		str;
	QCImapMailboxCommand *pExistingCommand = NULL;

	// Sanity
	if (!mailbox)
	{
		ASSERT (0);
		return -1;
	}

	// m_curMailboxList should NOT be NULL!!
	if (!m_curMailboxList)
	{
		ASSERT (0);
		return -1;
	}

	//
	// If the mailbox begins with the delimiter, remove it.
	//
	CString szMbox = mailbox;

#if 0 // Keep blanks in the mailbox name!! (JOK - 8/12/98).

	szMbox.TrimLeft ();
	szMbox.TrimRight ();

#endif // 

#if 0 //

	int nc = szMbox.Find (delimiter);
	if (nc == 0)
	{
		szMbox = szMbox.Mid (1);
	}


	// Remove any delimiter left.
	nc = szMbox.Find (delimiter);
	if (nc >= 0)
	{
		szMbox = szMbox.Left (nc);
	}

#endif //

	// Must have something left.
	if ( szMbox.IsEmpty () )
	{
		return FALSE;
	}

	// If the name is the same as the current reference, ignore it.
	if ( m_curReference.Compare (szMbox) == 0)
		return (-1);

	CRString Inbox (IDS_IMAP_RAW_INBOX_NAME);

	// Trap duplicate instances of INBOX.
	if ( Inbox.CompareNoCase (szMbox) == 0 )
	{
		// Should we ignore this or not?
		if (m_bIgnoreInbox)
			return -1;
		else
		{
			// Ignore all other inboxes
			m_bIgnoreInbox = TRUE;
		}
	}

	// Create a new ImapFolder structure and insert it into the list at this level.
	pFolder = NewImapMailboxNode();

	if (pFolder)
	{
		// Level and type.
		pFolder->level			= m_curLevel;
		pFolder->Type			= Type;
		pFolder->Delimiter		= delimiter;
		pFolder->AccountID		= m_AccountID;
		pFolder->pParentMailbox = m_curParentMailbox;

		// pImapName is a straight copy of mailbox. .
		pFolder->pImapName = DEBUG_NEW_NOTHROW char[szMbox.GetLength () + 1];
		if (pFolder->pImapName)
		{
			strcpy (pFolder->pImapName, szMbox);
		}

		pFolder->NoInferiors	= (attributes & LATT_NOINFERIORS) > 0;
		pFolder->NoSelect		= (attributes & LATT_NOSELECT) > 0;
		pFolder->Marked			= (attributes & LATT_MARKED) > 0;
		pFolder->UnMarked		= (attributes & LATT_UNMARKED) > 0;
		pFolder->AutoSync		= FALSE;

		// Check to see if we already have an IMAP mailbox by this name and
		// mark it as FOUND so that the lister will not attempt to create a new local directory
		// for it. 
		// Also, if it's found, set the directory name.
		pExistingCommand = g_theMailboxDirector.ImapFindByImapName ( GetAccountID(), szMbox, delimiter);

		// If this already exists locally, set the local directory.
		if (pExistingCommand)
		{
			CString Dirname;
			pExistingCommand->GetObjectDirectory (Dirname);

			pFolder->pDirname = CopyString ( (LPCSTR) Dirname);
			pFolder->bExists = TRUE;
		}

		// Make sure.						
		pFolder->SiblingList	= pFolder->ChildList = NULL;

		// pFolder->HasUnread   = ?? Won't know this until the folder is selected.
	}

	// Validity check.
	if (!pFolder)
	{
		return (-1);
	} 
	// MUST have ImapName!!
	else if (!(pFolder->pImapName))
	{
		delete pFolder;
		return (-1);
	}
	else
	{
		// Insert this in correct order.
		if (!*m_curMailboxList)
		{
			*m_curMailboxList = pFolder;
			i = 0;		// First position, zero based.
		}
		else if (strcmp ((*m_curMailboxList)->pImapName, pFolder->pImapName) > 0)
		{
			pFolder->SiblingList = *m_curMailboxList;
			*m_curMailboxList = pFolder;
			i = 0;		// First position, zero based.
		}
		else
		{
			pPrevFolder = *m_curMailboxList;
			i = 1;
			while (pPrevFolder->SiblingList)
			{
				if (strcmp (pPrevFolder->SiblingList->pImapName, pFolder->pImapName) > 0)
				{
					pFolder->SiblingList = pPrevFolder->SiblingList;
					pPrevFolder->SiblingList = pFolder;
					break;
				}
				else
				{
					pPrevFolder = pPrevFolder->SiblingList;
					i++;
				}
			}

			// Append to end of list?
			if (!pPrevFolder->SiblingList)
				pPrevFolder->SiblingList = pFolder;
		}

		return i;
	}
}



// GetTrashLocalMailboxPath [PUBLIC]
// FUNCTION
// Wade through the list of top-level mailboxes and look for a mailbox with
// terminal IMAP name of "Trash".
// If we have namespaces at the top level, look for the trash
// in the "#Personal" namespace.
// If "MakeSureExists" is TRUE, we have to attempt to create the mailbox if it
// doesn't exist, and make a local cached copy of it.
// END FUNCTION

// NOTES
// Return the path to the MBX file within the directory representing the mailbox.
// END NOTES.
BOOL CImapAccount::GetTrashLocalMailboxPath (CString& TrashMbxPath, BOOL MakeSureExists, BOOL bSilent /* = FALSE */)
{
	QCMailboxCommand		*pC;
	BOOL					bResult = FALSE;

	// Find the top level command object for this account.
	CString strDirectory; GetDirectory (strDirectory);

	pC = g_theMailboxDirector.FindByPathname( strDirectory );
	if (!pC)
	{
		ASSERT (0);
		return FALSE;
	}

	// This command must be of type MBT_IMAP_ACCOUNT!
	if (pC->GetType() != MBT_IMAP_ACCOUNT)
	{
		ASSERT(0);
		return FALSE;
	}

	// Verify the runtime class type as well!
	if (! ( ( CObject* ) pC ) ->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) ) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Wade through through the child list.
	CPtrList& ChildList = pC->GetChildList ();

	bResult = __GetTrashLocalMailboxPath (ChildList, TrashMbxPath);

	// If we haven't found one, attempt to create one at the top level.
	if (!bResult)
	{
		QCImapMailboxCommand *pNewCommand = CreateMailbox (CRString (IDS_IMAP_TRASH_BASENAME), (QCImapMailboxCommand *)pC, bSilent);
		if (pNewCommand)
		{
			// Make sure the mbx file actually exists.
			bResult = FileExistsMT (pNewCommand->GetPathname());
			if (bResult)
				TrashMbxPath = pNewCommand->GetPathname();
		}
	}

	return bResult;
}


// __GetTrashLocalMailboxPath [PRIVATE]
// FUNCTION
// Search pList for the mailbox. Recurse down through.
// Return the path to thye MBX file in "TrashMbxPath".
// END FUNCTION
BOOL CImapAccount::__GetTrashLocalMailboxPath (CPtrList& List, CString& TrashMbxPath)
{
	POSITION				pos;
	CString					Basename;
	QCImapMailboxCommand*	pImapCommand;
	TCHAR					Delimiter;
	int						i;
	CString					TrashBaseName;
	BOOL					bResult = FALSE;
	
	pos = List.GetHeadPosition();
	TrashBaseName = CRString (IDS_IMAP_TRASH_BASENAME);
	while( pos )
	{
		pImapCommand = ( QCImapMailboxCommand* ) List.GetNext( pos );
		if (pImapCommand)
		{
			if( pImapCommand->GetType() == MBT_IMAP_NAMESPACE )
			{
				// Not yet handled.
				return FALSE;
			}
			else if( pImapCommand->GetType() == MBT_IMAP_MAILBOX )
			{
				const CString& Pathname = pImapCommand->GetImapName();
				Delimiter = pImapCommand->GetDelimiter ();
				i = Pathname.ReverseFind (Delimiter);
				if (i > 0)
				{
					Basename = Pathname.Mid (i + 1);
					if (Basename.Compare (TrashBaseName) == 0)
					{
						TrashMbxPath = pImapCommand->GetPathname();
						return TRUE;
					}
				}
			}
		}
	}

	return bResult;
}


// GetJunkLocalMailboxPath [PUBLIC]
// FUNCTION
// Wade through the list of top-level mailboxes and look for a mailbox with
// terminal IMAP name of "Junk".
// If we have namespaces at the top level, look for the junk
// in the "#Personal" namespace.
// If "MakeSureExists" is TRUE, we have to attempt to create the mailbox if it
// doesn't exist, and make a local cached copy of it.
// END FUNCTION

// NOTES
// Return the path to the MBX file within the directory representing the mailbox.
// END NOTES.
BOOL CImapAccount::GetJunkLocalMailboxPath (CString& JunkMbxPath, BOOL MakeSureExists, BOOL bSilent /* = FALSE */)
{
	BOOL					bResult = FALSE;

	// Find the parent command object for mailboxes for this account.
	CString				 strParentName;
	QCMailboxCommand	*pCommandParent = NULL;
	GetPrefix(strParentName);
	if (!strParentName.IsEmpty())
	{
		// A prefix was specified: try to find a command object for the specified location.
		char			*szBuf = strParentName.GetBuffer(0);
		TCHAR			 cDelimiter = szBuf[strParentName.GetLength()];

		strParentName = strParentName.Left(strParentName.GetLength() - 1);
		pCommandParent = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
																 strParentName,
																 cDelimiter);
	}

	// At this point, pCommandParent could be NULL for two reasons: either strParentName is empty
	// or it is not empty but we couldn't find a parent command.  Both of these cases indicate this
	// is a root mailbox.  (If the prefix indicates a namespace other than the default then strParentName
	// will indicate the namespace, not an actual parent mail folder so no parent will be found.)
	if (pCommandParent == NULL)
	{
		// This is a root level mailbox.
		GetDirectory(strParentName);
		pCommandParent = g_theMailboxDirector.FindByPathname(strParentName);
	}

	if (pCommandParent)
	{
		// Verify the runtime class type as well!
		if (!((CObject*)pCommandParent)->IsKindOf(RUNTIME_CLASS(QCImapMailboxCommand)))
		{
			ASSERT (0);
			return FALSE;
		}

		// Wade through through the child list.
		CPtrList& ChildList = pCommandParent->GetChildList ();

		bResult = __GetJunkLocalMailboxPath (ChildList, JunkMbxPath);

		// If we haven't found one, attempt to create one.
		if (!bResult && MakeSureExists)
		{
			// Try creating the junk mailbox at the top level.
			QCImapMailboxCommand *pNewCommand = CreateMailbox(CRString(IDS_IMAP_JUNK_BASENAME),
															  (QCImapMailboxCommand*)pCommandParent,
															  TRUE/*bSilent*/);
			if (pNewCommand)
			{
				// Make sure the mbx file actually exists.
				bResult = FileExistsMT (pNewCommand->GetPathname());
				if (bResult)
					JunkMbxPath = pNewCommand->GetPathname();

				// Refresh the mailbox list to show the new mailbox.
				BOOL	 bViewNeedsRefresh = TRUE;
				pCommandParent->Execute(CA_IMAP_REFRESH, &bViewNeedsRefresh);
				if (bViewNeedsRefresh)
				{
					// Delete all child items and force a re-fresh.
					// Note: Recurse.
					g_theMailboxDirector.UpdateImapMailboxLists((QCImapMailboxCommand*)pCommandParent, TRUE, TRUE);
				}
			}
		}
	}

	return bResult;
}


// __GetJunkLocalMailboxPath [PRIVATE]
// FUNCTION
// Search pList for the mailbox. Recurse down through.
// Return the path to the MBX file in "strJunkMbxPath".
// END FUNCTION
BOOL CImapAccount::__GetJunkLocalMailboxPath (CPtrList& List, CString& JunkMbxPath)
{
	POSITION				pos;
	CString					Basename;
	QCImapMailboxCommand*	pImapCommand;
	TCHAR					Delimiter;
	int						i;
	CString					JunkBaseName;
	BOOL					bResult = FALSE;
	
	pos = List.GetHeadPosition();
	JunkBaseName = CRString (IDS_IMAP_JUNK_BASENAME);
	while( pos )
	{
		pImapCommand = ( QCImapMailboxCommand* ) List.GetNext( pos );
		if (pImapCommand)
		{
			if( pImapCommand->GetType() == MBT_IMAP_NAMESPACE )
			{
				// Not yet handled.
				return FALSE;
			}
			else if( pImapCommand->GetType() == MBT_IMAP_MAILBOX )
			{
				const CString& Pathname = pImapCommand->GetImapName();
				Delimiter = pImapCommand->GetDelimiter ();
				i = Pathname.ReverseFind (Delimiter);
				if (i > 0)
				{
					Basename = Pathname.Mid (i + 1);
				}
				else
				{
					Basename = Pathname;
				}
				if (Basename.Compare (JunkBaseName) == 0)
				{
					JunkMbxPath = pImapCommand->GetPathname();
					return TRUE;
				}
			}
		}
	}

	return bResult;
}

//
//	CImapAccount::GetAutoExpungeSetting()
//
//	Returns the auto expunge setting for this personality and, if the setting is for a percent,
//	returns the percent in iPercent.
//
int CImapAccount::GetAutoExpungeSetting(int *piPercent)
{
	CString		 key;
	TCHAR		 szValue[32];

	key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_NEVER);
	g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
	if (szValue[0] == '1')
	{
		return IDS_INI_IMAP_AUTO_EXPUNGE_NEVER;
	}

	key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS);
	g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
	if (szValue[0] == '1')
	{
		return IDS_INI_IMAP_AUTO_EXPUNGE_ALWAYS;
	}

	key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT);
	g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
	if (szValue[0] == '1')
	{
		key = g_Personalities.GetIniKeyName(IDS_INI_IMAP_AUTO_EXPUNGE_PCT);
		g_Personalities.GetProfileString(m_strPersona, key, "", szValue, sizeof(szValue));
		*piPercent = atoi(szValue);
		return IDS_INI_IMAP_AUTO_EXPUNGE_ON_PCT;
	}

	return -1;
}


//
// CImapAccount::FindMailboxAtRoot()
//
//	Find a root level mailbox within this account that matches the specified name.
//	This is a convenience routine to deal with IMAP servers that require everything
//	to be under the Inbox.  If the mailbox is not found at the actual root it will
//	try to find it under the Inbox.
//
QCImapMailboxCommand *CImapAccount::FindMailboxAtRoot(CString &strName)
{
	QCImapMailboxCommand	*pCommand = NULL;
	TCHAR					 cDelimiter = 0;

	// Get desintation mailbox.
	CString					 strImapName;
	if (!strImapName.IsEmpty())
	{
		char				*szBuf = strImapName.GetBuffer(0);
		cDelimiter = szBuf[strImapName.GetLength() - 1];
	}
	strImapName += strName;

	if (!strImapName.IsEmpty())
	{
		// Look for the mailbox at the root of our local mailbox tree.
		pCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
														   strImapName,
														   cDelimiter);
	}

	return pCommand;
}


//
// CImapAccount::CreateMailboxAtRoot()
//
//	Create a new mailbox within this account at the root level.  This is a convenience
//	routine to deal with IMAP servers that require everything to be under the Inbox.
//	If creating the mailbox at the actual root fails, try again using Inbox as the root.
//
QCImapMailboxCommand *CImapAccount::CreateMailboxAtRoot(CString &strName,
														BOOL bSilent/*=FALSE*/,
														BOOL bMakeItAFolder/*=FALSE*/)
{
	QCImapMailboxCommand	*pCommand = NULL;
	TCHAR					 cDelimiter = 0;
	BOOL					 bCancelled = FALSE;

	if (!strName.IsEmpty())
	{
		// Look for the mailbox at the root of our local mailbox tree.
		pCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
														   strName,
														   cDelimiter);
	}

	// If mailbox doesn't exist, try to create it now.
	if (!pCommand)
	{
		CString				 strParentName;
		QCMailboxCommand	*pCommandParent = NULL;

		GetDirectory(strParentName);
		pCommandParent = g_theMailboxDirector.FindByPathname(strParentName);
		if (pCommandParent)
		{
			if (!GetImapAccountMgr()->MailboxExistsOnServer(GetAccountID(), strName))
			{
				CString			 strPassword;
				GetPasswordMT(strPassword);
				// MailboxExistsOnServer() above should have checked for a password if
				// necessary.  If we don't have a password at this point then the
				// user clearly cancelled when asked above.  Don't go any further.
				if (strPassword.IsEmpty())
				{
					bCancelled = TRUE;
				}
				else
				{
					pCommand = (QCImapMailboxCommand*)g_theMailboxDirector.AddCommand(strName,
																					  MBT_IMAP_MAILBOX,
																					  pCommandParent);
					if (pCommand)
					{
						strName = pCommand->GetImapName();
					}
				}
			}
		}
	}

	// Still no command, give up.
	if (!pCommand)
	{
		m_bJunkMailboxOK = FALSE;

		if (!bSilent && !bCancelled)
		{
			CRString	 fmt(IDS_ERR_IMAP_INVALID_MBOXNAME);
			CString		 szText;

			szText.Format (fmt, strName);
			ErrorDialog (IDS_ERR_IMAP_CMD_FAILED_REASON, szText);
		}
	}

	return pCommand;
}

//
//	CImapAccount::CreateMailbox()
//
//	Locally create the specified mailbox from the account and queue an action to perform the online
//	creation at a later time.
//
//	Note: For the time being for purposes of switching back to the old behavior the code conditionally
//	will do the action online as well.
// 
QCImapMailboxCommand* CImapAccount::CreateMailbox(LPCSTR szName,
												  QCImapMailboxCommand *pParentCommand,
												  BOOL bSilent /*=FALSE*/,
												  BOOL bMakeItAFolder /*=FALSE*/)
{
	CString					 strNewImapName;
	CString					 strNewMailboxName;
	TCHAR					 cDelimiter;
	QCImapMailboxCommand	*pNewImapCommand = NULL;
	BOOL					 bCreateSuccess = TRUE;

	if (!szName || !*szName || !pParentCommand)
	{
		return NULL;
	}

	// Mildly convoluted logic here.  We don't allow creating a mailbox with a name consisting
	// entirely of spaces, but leading and trailing spaces are fine.  Trim the spaces and see if
	// something is left if not, bail, if so go back to the original name.
	strNewMailboxName = szName;
	strNewMailboxName.TrimRight();
	strNewMailboxName.TrimLeft();
	if (strNewMailboxName.IsEmpty())
	{
		ErrorDialog(IDS_IMAP_INVALID_MBOXNAME, szName);
		return NULL;
	}

	// If we got this far the name contains something besides spaces (which is all the name checking
	// we currently perform) so restore the name and proceed.
	strNewMailboxName = szName;

	// Get the login and password.
	CString		 strLogin;
	CString		 strPassword;
	if (!Login(strLogin, strPassword))
	{
		return NULL;
	}

	// Format the full IMAP name of the mailbox. The parent's name becomes a prefix.
	strNewImapName = pParentCommand->GetImapName();
	cDelimiter = pParentCommand->GetDelimiter();

	// A couple things aren't entirely clear to me: 1) How do we get a delimiter from the server
	// when we are offline and 2) Do we need to?  The delimiter is 0 for top level mailboxes where
	// (it seems) we don't need a delimiter and is not 0 for all others.  I could be wrong on both
	// of these.  For example, on a server where all mailboxes are under Inbox what about the
	// delimiter?  Can we use the prefix for this?
	// If cDelimiter is 0 and parent is an account, go update delimiter.
	// if ( (cDelimiter == 0) && (pParentCommand->GetType () == MBT_IMAP_ACCOUNT) )
	// {
	//	UpdateDelimiterChar (m_pImap, this);
	//
	//	// Did we get one?
	//	cDelimiter	= pParentCommand->GetDelimiter();
	// }

	// Don't allow a mailbox name to be nothing more than the delimiter character.
	if (cDelimiter && (strNewMailboxName.GetLength() == 1) && (strNewImapName.Right(1) == cDelimiter))
	{
		ErrorDialog (IDS_IMAP_INVALID_MBOXNAME, szName);
		return NULL;
	}

	// The IMAP name is the parent's IMAP name, plus any delimiter, plus the mailbox name.
	if (!strNewImapName.IsEmpty() && cDelimiter)
	{
		if (strNewImapName.Right(1) != cDelimiter)
		{
			strNewImapName += cDelimiter;
		}
	}
	strNewImapName += strNewMailboxName;

	// The newly formed IMAP name must not be empty.
	if (strNewImapName.IsEmpty())
	{
		ASSERT (0);
		return NULL;
	}

	// If "bMakeItAFolder is TRUE, append delimiter to this new name.
	if (cDelimiter && bMakeItAFolder)
	{
		if (strNewImapName.Right(1) != cDelimiter)
		{
			strNewImapName += cDelimiter;
		}
	}

	// Reset the state data for mailbox manipulations.
	ResetInternalState();

	// If "strNewImapName" had a trailing delimiter (i.e. we did a "Make it a folder"),
	// that's not part of the mailbox name!! Take it back off.
	if (strNewImapName.Right (1) == cDelimiter)
		strNewImapName = strNewImapName.Left (strNewImapName.GetLength() - 1);

	// Create a new mailbox node to hold the attributes.
	ImapMailboxNode *pImapNode = DEBUG_NEW ImapMailboxNode;
	if (!pImapNode)
	{
		return NULL;
	}

	// Since we can't go online to get the mailbox attributes we have to take our best guess
	// until we go online later.
	memset((void *)pImapNode, 0, sizeof (ImapMailboxNode));
	pImapNode->Type	= IMAP_MAILBOX;
	// level appears to be unused.
	// pImapNode->level = 0;
	pImapNode->pImapName = DEBUG_NEW char[strNewImapName.GetLength() + 1];
	strcpy(pImapNode->pImapName, strNewImapName);
	pImapNode->AccountID = m_AccountID;
	// What if cDelimiter is 0?
	pImapNode->Delimiter = cDelimiter;

	// Make a last ditch effort to make sure that we really don't already have the
	// mailbox in the mboxtree list.
	pNewImapCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
															  strNewImapName,
															  pImapNode->Delimiter);
	if (pNewImapCommand)
	{
		// The mailbox already exists: bail.
		ASSERT(0);
		return NULL;
	}

	CString		 strPath;
	CString		 strDirName;

	pParentCommand->GetObjectDirectory(strPath);

	// Get a unique directory name and create it NOW!.We will create the
	// files within the directory later.
	if ( MakeSuitableMailboxName (strPath, NULL, pImapNode, strDirName, 64) )
	{
		// Format a path to strDirName and copy it to pCurNode->pPathname.
		strPath += DirectoryDelimiter + strDirName;

		// Set it into the node.
		if (pImapNode->pDirname)
			delete[] (pImapNode->pDirname);
		pImapNode->pDirname	= CopyString (strPath);

		// Make sure the directory exists or that we can create it.
		if (!DirectoryExists (pImapNode->pDirname))
			CreateDirectory (pImapNode->pDirname, NULL);

		// Get an MBX file path.
		GetMbxFilePath (pImapNode->pDirname, strPath);

		// Create a new command object and return it.

		pNewImapCommand = DEBUG_NEW QCImapMailboxCommand(&g_theMailboxDirector,
														 strNewMailboxName,
														 pImapNode->pImapName,
														 (LPCSTR)strPath,
														 pParentCommand->ImapTypeToMBTType(pImapNode->Type),
														 US_UNKNOWN,
														 GetAccountID(),
														 pImapNode->NoSelect,
														 pImapNode->Marked,
														 pImapNode->UnMarked,
														 pImapNode->NoInferiors,
														 pImapNode->AutoSync); 

		if (pNewImapCommand)
		{
			pNewImapCommand->SetDelimiter (pImapNode->Delimiter);
		}
	}
	else
	{
		bCreateSuccess = FALSE;
	}

	// No longer need pImapNode.
	DeleteMailboxNode (pImapNode);
	pImapNode = NULL;

	if (bCreateSuccess && bQueueAction)
	{
		// We are using the local/cache/online technique: create an action object for
		// this operation and add it to the queue.
		CImapCreateMailboxAction	*pCreateMailboxAction = DEBUG_NEW CImapCreateMailboxAction(m_AccountID,
																								strNewImapName,
																								cDelimiter,
																								strLogin,
																								strPassword);
		if (pCreateMailboxAction)
		{
			QueueAction(pCreateMailboxAction);
		}
	}

	return pNewImapCommand;
}

//
// CImapAccount::CreateMailboxOnServer()
//
//	Create the specified mailbox on the server.
//
BOOL CImapAccount::CreateMailboxOnServer(LPCSTR szImapName,
										 TCHAR cDelimiter)
{
	BOOL		 bResult = FALSE;

	if (!szImapName || !*szImapName)
	{
		return FALSE;
	}

	// Open the control stream and do the online action.
	if (SUCCEEDED(m_pImap->OpenControlStream(this)))
	{
		// Create the mailbox online.
		bResult = SUCCEEDED(m_pImap->CreateMailbox(szImapName));

		if (!bResult)
		{
			ShowError(IDS_ERR_IMAP_CMD_FAILED_SRVMSG, m_pImap, szImapName, cDelimiter);
		}

		// Since we created the command object without going online we need to fetch the mailbox
		// attributes from the server and update the command object.
		QCImapMailboxCommand	*pImapCommand = NULL;
		pImapCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
															   szImapName,
															   cDelimiter);
		if (pImapCommand)
		{
			ImapMailboxNode *pImapNode = m_pImap->FetchMailboxAttributes(szImapName);
			if (pImapNode)
			{
				pImapCommand->SetNoSelect(pImapNode->NoSelect);
				pImapCommand->SetMarked(pImapNode->Marked);
				pImapCommand->SetUnmarked(pImapNode->UnMarked);
				pImapCommand->SetNoInferiors(pImapNode->NoInferiors);
				pImapCommand->SetAutoSync(pImapNode->AutoSync); 
			}
		}

		// Subscribe the mailbox.
		m_pImap->SubscribeMailbox(szImapName);
	}

	if (!bResult)
	{
		ShowError(IDS_ERR_IMAP_CMD_FAILED_SRVMSG, m_pImap, szImapName, cDelimiter);
	}

	// This is purely a temporary measure.  We really should keep this object around so we don't have
	// to create a new one for every offline account level action we need to replay.
//	delete m_pImap;
//	m_pImap = NULL;

	return bResult;
}

//
//	CImapAccount::DeleteMailbox()
//
//	Locally delete the specified mailbox from the account and queue an action to perform the online
//	delete at a later time.
//
//	Note: For the time being for purposes of switching back to the old behavior the code conditionally
//	will do the action online as well.
// 
BOOL CImapAccount::DeleteMailbox(LPCSTR szImapName, TCHAR cDelimiter)
{
	BOOL	 bResult = FALSE;

	if (!szImapName || !*szImapName)
	{
		return FALSE;
	}

	// Get the login and password for this account.
	CString		 strLogin;
	CString		 strPassword;
	if (!Login(strLogin, strPassword))
	{
		return FALSE;
	}

	if (bQueueAction)
	{
		// We are using the local/cache/online technique: create an action object for
		// this operation and add it to the queue.
		CImapDeleteMailboxAction	*pDeleteMailboxAction = DEBUG_NEW CImapDeleteMailboxAction(m_AccountID,
																								szImapName,
																								cDelimiter,
																								strLogin,
																								strPassword);
		if (pDeleteMailboxAction)
		{
			QueueAction(pDeleteMailboxAction);
			bResult = TRUE;
		}
	}
	else
	{
		DeleteMailboxOnServer(strLogin, strPassword, szImapName, cDelimiter);
	}

	return bResult;
}

//
//	CImapAccount::DeleteMailboxOnServer()
//
//	Delete the specified mailbox on the server.
//
BOOL CImapAccount::DeleteMailboxOnServer(CString &strLogin,
										 CString &strPassword,
										 LPCSTR szImapName,
										 TCHAR cDelimiter)
{
	BOOL		 bResult = FALSE;

	if (strLogin.IsEmpty() || strPassword.IsEmpty() || !szImapName || !*szImapName)
	{
		return FALSE;
	}

	// Open the control stream and do the online action.
	if (SUCCEEDED(m_pImap->OpenControlStream(this)))
	{
		bResult = SUCCEEDED(m_pImap->DeleteMailbox(szImapName));

		if (!bResult)
		{
			ShowError(IDS_ERR_IMAP_SERVER_MBOX_DELETE, m_pImap, szImapName, cDelimiter);
		}
	}

	// This is purely a temporary measure.  We really should keep this object around so we don't have
	// to create a new one for every offline account level action we need to replay.
//	delete m_pImap;
//	m_pImap = NULL;

	return bResult;
}

// RenameMailbox [PUBLIC]
// FUNCTION
// Rename an IMAP mailbox.
// END 

// NOTES
// NOTE: "szNewImapName" is just the base name. Get the superior
// parts of the name from pImapCommand.
//
// NOTE: Only a "simple" rename is permitted here. That is, the name cannot contain
// the mailbox hierachy character.
//
// Important note!: It is possible for the rename to result in the creation of a new
// mailbox and the retention of the old one, albeit with modification of it's status.
// In that case, the output parameter "bOldMboxStillExists" is TRUE and "ppNewImapMailbox"
// will, on output, contain a pointer to the new (allocated) command object that must
// be linked into the UI structures.
//
// NOTE: (2/9/98, JOK) We now handle progress mesages only if the progress bar was already up.
//
// END NOTES
//
BOOL CImapAccount::RenameMailbox(QCImapMailboxCommand *pImapCommand,
								 LPCSTR szNewImapName,
								 CString &strNewDirName,
								 BOOL &bOldMboxStillExists,
								 QCImapMailboxCommand **ppNewImapMailbox)
{
	BOOL	bResult = FALSE;
	TCHAR	cDelimiter = 0;

	// Do these. They are output parameters.
	strNewDirName.Empty();
	bOldMboxStillExists = FALSE;

	if (!pImapCommand || !szNewImapName || !*szNewImapName || !ppNewImapMailbox)
	{
		ASSERT (0);
		return FALSE;
	}

	// Get a login and password for this account.
	CString		 strLogin;
	CString		 strPassword;
	if (!Login(strLogin, strPassword) )
	{
		return FALSE;
	}

	// Get hierarchical delimiter of the account itself if we haven't yet.
	TCHAR cTopDelimiter = GetTopDelimiter();

//	if (!cTopDelimiter)
//		UpdateDelimiterChar(m_pImap, this);

	// Default this to cTopDelimiter.
	//
	cDelimiter = cTopDelimiter;

	// Get old IMAP name.
	CString	 strOldImapName = pImapCommand->GetImapName();
	CString	 strNewFullImapName;

	//
	// If we are trying to rename INBOX, prepend the account's location
	// prefix.
	//
	if (strOldImapName.CompareNoCase ( CRString (IDS_IMAP_RAW_INBOX_NAME) ) == 0)
	{
		// The default.
		strNewFullImapName = szNewImapName;

		// For INBOX, the delimiter is the account's delimiter.
		//
		cDelimiter = cTopDelimiter;

		// Prepend account's location prefix.
		CString prefix; GetPrefix (prefix);

		if (!prefix.IsEmpty())
		{
			// Make sure ends in delimiter.
			if (cDelimiter && prefix.Right ( 1 ) != cDelimiter)
				prefix += cDelimiter;

			strNewFullImapName = prefix + szNewImapName;
		}
	}
	else
	{
		//
		// Format new full IMAP hierarchical name.
		//
		cDelimiter = pImapCommand->GetDelimiter();

		if (cDelimiter == '\0')
		{
			// No hierarchy. Use basename as is.
			strNewFullImapName = szNewImapName;
		}
		else
		{
			strNewFullImapName = strOldImapName;

			int nc = strNewFullImapName.ReverseFind(cDelimiter);

			if (nc < 0)
			{
				// Top level. Use as is.
				strNewFullImapName = szNewImapName;
			}
			else
			{
				// Replace basename.
				strNewFullImapName = strNewFullImapName.Left (nc + 1);
				strNewFullImapName += szNewImapName;
			}
		}
	}

	// 	Go rename the local directory and it's contents.
	// Return the new mailbox name in strNewDirName.

	CString szOldMboxDir;
	pImapCommand->GetObjectDirectory (szOldMboxDir);


	// Instead of getting the mailbox properties from the server, get them from the local cache.
	ImapMailboxNode		*pImapNode = DEBUG_NEW ImapMailboxNode;
	if (pImapNode)
	{
		pImapNode->pImapName = (char*)malloc(strNewFullImapName.GetLength() + 1);
		if (pImapNode->pImapName)
		{
			strcpy(pImapNode->pImapName, strNewFullImapName);
			pImapNode->Delimiter = cDelimiter;
			bResult = RenameMboxDirAndContents(szOldMboxDir, pImapNode, strNewDirName);
			free(pImapNode->pImapName);
		}
		delete pImapNode;
	}

	CImapRenameMailboxAction	*pRenameMailboxAction = DEBUG_NEW CImapRenameMailboxAction(m_AccountID,
																							strOldImapName.GetBuffer(0),
																							cDelimiter,
																							strNewFullImapName,
																							strLogin,
																							strPassword);
	if (pRenameMailboxAction)
	{
		QueueAction(pRenameMailboxAction);
	}

	// This is purely a temporary measure.  We really should keep this object around so we don't have
	// to create a new one for every offline account level action we need to replay.
//	delete m_pImap;
//	m_pImap = NULL;

	if (!bResult)
	{
		ShowError(IDS_ERR_IMAP_SERVER_MBOX_RENAME, m_pImap, strOldImapName, '\0');
	}

	return bResult;
}

//
//	CImapAccount::RenameMailboxOnServer()
//
//	Rename the specified mailbox on the server.
//
BOOL CImapAccount::RenameMailboxOnServer(CString &strLogin,
										 CString &strPassword,
										 LPCSTR szOldImapName,
										 LPCSTR szNewImapName,
										 TCHAR cDelimiter)
{
	BOOL		 bResult = FALSE;

	if (strLogin.IsEmpty() || strPassword.IsEmpty() ||
		!szOldImapName || !*szOldImapName ||
		!szNewImapName || !*szNewImapName)
	{
		return FALSE;
	}

	// Open the control stream and do the online action.
	if (SUCCEEDED(m_pImap->OpenControlStream(this)))
	{
		bResult = SUCCEEDED(m_pImap->RenameMailbox(szOldImapName, szNewImapName));

		// If we didn't succeed, see what the server had to say.
		if (!bResult)
		{
			ShowError(IDS_ERR_IMAP_SERVER_MBOX_RENAME, m_pImap, szOldImapName, '\0');
			return bResult;
		}

		ImapMailboxNode			*pImapNode = NULL;
		QCImapMailboxCommand	*pImapCommand = NULL;

#if 0
		// Set this to FALSE unless we see that the old mailbox is still there.
		BOOL		 bOldMboxStillExists = FALSE;
		ResetInternalState();

		pImapNode = m_pImap->FetchMailboxAttributes(szOldImapName);
		if (pImapNode)
		{
			// Still there! No fear. Some servers (Cyrus) may actually create another
			// mailbox and retain the old one, but now with the (\noselect} flag.
			// That will be handled below, but here, we retain the old mailbox
			// and just update it's attributes.

			pImapCommand->SetNoInferiors(pImapNode->NoInferiors);
			pImapCommand->SetNoSelect(pImapNode->NoSelect);
			pImapCommand->SetMarked(pImapNode->Marked);
			pImapCommand->SetUnmarked(pImapNode->UnMarked);
			pImapCommand->SetAutoSync(pImapNode->AutoSync);

			DeleteMailboxNode(pImapNode);
			pImapNode = NULL;

			// Don't quit. Flag that the old mailbox still exists and fall through.
			bOldMboxStillExists = TRUE;

			// Be sure to free it.
			DeleteMailboxNode(pImapNode);
			pImapNode = NULL;
		}
#endif

		ResetInternalState();

		pImapCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
															   szNewImapName,
															   cDelimiter);

		if (pImapCommand)
		{
			pImapNode = m_pImap->FetchMailboxAttributes(szNewImapName);
			if (pImapNode)
			{
				pImapCommand->SetNoInferiors(pImapNode->NoInferiors);
				pImapCommand->SetNoSelect(pImapNode->NoSelect);
				pImapCommand->SetMarked(pImapNode->Marked);
				pImapCommand->SetUnmarked(pImapNode->UnMarked);
				pImapCommand->SetAutoSync(pImapNode->AutoSync);

				DeleteMailboxNode(pImapNode);
				pImapNode = NULL;
			}
		}

#if 0
		if (pImapNode)
		{
			// Ok. We actually succeeded. If the old mailbox still exists, this is 
			// a new mailbox. Go create our local representation of it.
			if (bOldMboxStillExists)
			{
				// This is a new mailbox.  
				// We return the pathname housing the newly created mailbox in strNewDirName.
				QCImapMailboxCommand *pNewCommand = CreateChildCommandFromNode (
									(QCImapMailboxCommand *) g_theMailboxDirector.ImapFindParent (pImapCommand),
									pImapNode,
									szNewImapName,
									strNewDirName);

				if (pNewCommand)
				{
					// Copy to output parameter.
					*ppNewImapMailbox = pNewCommand;
					bResult = TRUE;
				}
				else
				{
					// Couldn't create new mailbox.
					bResult = FALSE;
				}
			}
			else
			{
				// We actually changed the name of the mailbox on the server.
				// 	Go rename the local directory and it's contents.
				// Return the new mailbox name in strNewDirName.

				CString szOldMboxDir;
				pImapCommand->GetObjectDirectory (szOldMboxDir);

				bResult = RenameMboxDirAndContents (szOldMboxDir, pImapNode, strNewDirName);
			}

			// Be sure to free it.
			DeleteMailboxNode(pImapNode);
			pImapNode = NULL;
		}
#endif
	}

	// This is purely a temporary measure.  We really should keep this object around so we don't have
	// to create a new one for every offline account level action we need to replay.
//	delete m_pImap;
//	m_pImap = NULL;

	if (!bResult)
	{
		ShowError(IDS_ERR_IMAP_SERVER_MBOX_RENAME, m_pImap, szOldImapName, '\0');
	}

	return bResult;
}

// MoveMailbox [PUBLIC]
// FUNCTION
// Rename an IMAP mailbox, but now, we know the full path of the destination
// mailbox.
// END 

// NOTES
// NOTE: (2/9/98, JOK) We now handle progress mesages only if the progress bar was already up.

// END NOTES
//
BOOL CImapAccount::MoveMailbox ( QCImapMailboxCommand *pImapCommand, QCImapMailboxCommand *pNewParent,
								  BOOL& bOldMboxStillExists,
								  QCImapMailboxCommand **ppNewLastParent,
								  QCImapMailboxCommand **ppNewImapMailbox)
{
	BOOL	bResult = FALSE;
	TCHAR	ImapDelimiter = 0;
	BOOL	bHasChildMboxes = FALSE;

	// Sanity.
	if (! (pImapCommand && pNewParent && ppNewLastParent && ppNewImapMailbox) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Initialize these.
	//
	*ppNewLastParent = NULL;
	*ppNewImapMailbox = NULL;

	// Flag this for use below:
	bHasChildMboxes = !pImapCommand->GetChildList().IsEmpty();


	// Open a new control stream to the server.
	// BUG: Each account should have an open control stream that can be re-used!!
	CString strAddress;  GetAddress (strAddress);
	CImapConnection Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return FALSE;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	//
	// Open it..
	//
	if ( !SUCCEEDED(Imap.OpenControlStream(this)) )
	{

		return FALSE;
	}

	// Get hierarchical delimiter of the account itself if we haven't yet.
	//
	TCHAR TopDelimiter = GetTopDelimiter ();

	if (!TopDelimiter)
		UpdateDelimiterChar (&Imap, this);

	//
	// Get the hierarchy delimiter.
	//
	ImapDelimiter = pImapCommand->GetDelimiter();

	if (ImapDelimiter == '\0')
	{
		ImapDelimiter = TopDelimiter;
	}

	// Get basename of the mailbox we're moving.
	//
	CString szOldImapName = pImapCommand->GetImapName();
	CString szBaseName = szOldImapName;

	if (ImapDelimiter != '\0')
	{
		int nc = szBaseName.ReverseFind (ImapDelimiter);

		if (nc >= 0)
		{
			szBaseName = szBaseName.Mid (nc + 1);
		}
	}

	// Format new full IMAP name.
	//
	CString szNewFullImapName = pNewParent->GetImapName();

	szNewFullImapName.TrimRight();

	if ( ImapDelimiter && !szNewFullImapName.IsEmpty() &&
			(szNewFullImapName.Right(1) != ImapDelimiter) )
	{
		szNewFullImapName += ImapDelimiter;
	}

	// Add Basename:
	//
	szNewFullImapName += szBaseName;

	//
	// Put up progress bar.
	//
	if ( InProgress &&  ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format ( CRString(IDS_IMAP_RENAMING_MBOX), szOldImapName, szNewFullImapName);
		MainProgress(buf);
	}

	//
	// Note: we're NOT looping - just a way to easily get out.
	//
	while (1)
	{
		ImapMailboxNode *pImapNode = NULL;

		// Ask the server to rename.

		bResult = SUCCEEDED(Imap.RenameMailbox (szOldImapName, szNewFullImapName));

		// If we didn't succeed, see what the server had to say.
		if (!bResult)
		{
			break;
		}

		//
		// Seems like we succeeded. Make sure we did!!
		// Don't trust the result from IMAP. Go make sure that the old mailbox no longer exists
		// AND that the new one exists.
		//
		bResult = FALSE;

		// Set this to FALSE unless we see that the old mailbox is still there.
		bOldMboxStillExists = FALSE;

		ResetInternalState ();

		pImapNode = Imap.FetchMailboxAttributes (szOldImapName);

		if (pImapNode)
		{
			// Still there! No fear. Some servers (Cyrus) may actually create another
			// mailbox and retain the old one, but now with the (\noselect} flag.
			// That will be handled below, but here, we retain the old mailbox
			// and just update it's attributes.

			pImapCommand->SetNoInferiors (pImapNode->NoInferiors);
			pImapCommand->SetNoSelect	 (pImapNode->NoSelect);
			pImapCommand->SetMarked		 (pImapNode->Marked);
			pImapCommand->SetUnmarked	 (pImapNode->UnMarked);
			pImapCommand->SetAutoSync	 (pImapNode->AutoSync);

			DeleteMailboxNode (pImapNode);
			pImapNode = NULL;

			// Don't quit. Flag that the old mailbox still exists and fall through.
			bOldMboxStillExists = TRUE;
		}
		else
		{
			// If old mailbox no longer exists, caller needs to refresh subtree.
			// Find the highest parent mailbox that still exists and return it as an output 
			// parameter.
			//

			ImapMailboxNode *pParentImapNode = NULL;
	
			CString szStr = szOldImapName;
			int nc = szStr.ReverseFind (ImapDelimiter);

			while (nc > 0)
			{
				szStr = szStr.Left (nc);

				ResetInternalState ();

				pParentImapNode = Imap.FetchMailboxAttributes (szStr);

				if (pParentImapNode)
					break;
				else
					nc = szStr.ReverseFind (ImapDelimiter);
			}

			// If we got a node, find it's local command object and return that in 
			// ppNewLastParent.

			if (pParentImapNode)
			{
				if (pParentImapNode->pImapName)
				{
					*ppNewLastParent = g_theMailboxDirector.ImapFindByImapName(
										GetAccountID(), 
										pParentImapNode->pImapName,
										ImapDelimiter);
				}
				
				DeleteMailboxNode (pParentImapNode);
				pParentImapNode = NULL;
			}
		} // Old mbox no longer exists.


		//
		// Ok Old mailbox may or may not still exist. In any case, look for new one.
		//

		ResetInternalState ();

		// If "szNewFullImapName" had a trailing delimiter.
		// that's not part of the mailbox name!! Take it back off.
		//
		if (szNewFullImapName.Right (1) == ImapDelimiter)
			szNewFullImapName = szNewFullImapName.Left (szNewFullImapName.GetLength() - 1);

		// 
		pImapNode = Imap.FetchMailboxAttributes (szNewFullImapName);

		if (!pImapNode)
		{
			// We failed. Grievous error. However, return TRUE so caller will clean things
			// up.

			bResult = TRUE;
			break;
		}
		else
		{
			// Well, we created the new one. Create a command object and associated local
			// storage for the new mailbox and return it. The caller should refresh
			// it's subtree if it had child mailboxes.
			//
			bResult = TRUE;

			CString szNewDirName;

			QCImapMailboxCommand *pNewCommand = CreateChildCommandFromNode (
										pNewParent, 
										pImapNode,
										szBaseName,
										szNewDirName);


			if (pNewCommand)
			{
				// Copy to output parameter.
				*ppNewImapMailbox = pNewCommand;
			}

			// Be sure to free it.
			DeleteMailboxNode (pImapNode);
			pImapNode = NULL;
		}

		// Don't loop!!
		break;
	}

	//
	// Don't close progress bar.
	//
	if ( ::IsMainThreadMT() )
	{

		// Cursor seems to remain. Fake a cursor movement.
		AfxGetMainWnd ()->SendMessage (WM_SETCURSOR);
	}


	// If we didn't succeed, see what the server had to say.
	if (!bResult)
	{
		TCHAR  buf [512];
		int iType;
		buf[0] = '\0';

		Imap.GetLastImapErrorAndType (buf, 510, &iType);

		if ( buf[0] )
		{
			ErrorDialog (IDS_ERR_IMAP_SERVER_MBOX_RENAME, szOldImapName, szNewFullImapName, buf);
		}
		else
		{
			ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
		}
	}

	return bResult;
}





//
// Checking mail for "inbox"
// NOTE: Do this only if inbox is OPEN.
//
void CImapAccount::CheckMail ()
{
	// Always reset the first.
	// NOTE: The mailbox itself (must be INBOX) will alert us if
	// it got new mail!!.
	//
	SetGotNewMail (FALSE);

	// Make sure we have a command object for inbox.
	QCImapMailboxCommand*	pImapCommand = g_theMailboxDirector.ImapFindByImapName(
												GetAccountID (),
												"Inbox",
												'\0');
	if (pImapCommand)
	{
		// Get the toc even if it's not already loaded.
		CTocDoc* pTocDoc = GetToc( pImapCommand->GetPathname(), pImapCommand->GetName());

		if (pTocDoc && pTocDoc->m_pImapMailbox)
			pTocDoc->m_pImapMailbox->CheckNewMail(pTocDoc);
	}
}



// ===  Methods to handle multithreaded IMAP operations =====//

BOOL CImapAccount::FetchNewMessages (CImapMailbox *pImapMailbox, CPtrList& UidList)
{
	if (!pImapMailbox)
		return FALSE;

	// Create a new message thread object.
	CImapMessageThread  *pMessageThread = DEBUG_NEW_MFCOBJ_NOTHROW CImapMessageThread ();

	if (!pMessageThread)
		return FALSE;

	// Tell it our account ID so it can talk back to us.
	pMessageThread->SetAccountID (GetAccountID());

	// Get the thread to copy our uid list.
	pMessageThread->CopyUidList (UidList);


	// Start the thread and return.
	// NOTE: The thread object copies stuff from pImapMailbox.
	return pMessageThread->FetchNewMessages(pImapMailbox);
}





/////////////////////////////////////////////////////////////////////////////////////
// CreateChildCommandFromNode [private]
//
// FUNCTION
// Given an Imap command object representing a mailbox, and an ImapNode structure 
// representing a mailbox that resides on the server,
// END FUNCTION
/////////////////////////////////////////////////////////////////////////////////////
QCImapMailboxCommand* CImapAccount::CreateChildCommandFromNode (QCImapMailboxCommand *pParentCommand, ImapMailboxNode *pImapNode, LPCSTR pNewName, CString szNewDirname)
{
	QCImapMailboxCommand *pNewImapCommand = NULL;

	// Sanity.
	if (! (pParentCommand && pNewName && pImapNode) )
		return NULL;

	//
	// Get the parent directory so we can attempt to create local storage for the mailbox.
	//
	CString szPath;

	pParentCommand->GetObjectDirectory (szPath);

	//
	// Create local storage.
	//
	BOOL bResult = CreateDirectoryForMailbox (szPath, pImapNode, szNewDirname);

	// If we succeeded, create the command object.
	if (bResult)
	{
		// Get the MBX file path.
		szPath +=  DirectoryDelimiter + szNewDirname;

		CString szMbxPath;
		GetMbxFilePath (szPath, szMbxPath);

		// Create a new command object and return it.

		pNewImapCommand = DEBUG_NEW_MFCOBJ_NOTHROW QCImapMailboxCommand (
								&g_theMailboxDirector,
							    pNewName, 
								pImapNode->pImapName, (LPCSTR)szMbxPath, 
								MBT_IMAP_MAILBOX,
								US_UNKNOWN,							
								GetAccountID(),
								pImapNode->NoSelect,
								pImapNode->Marked, 
								pImapNode->UnMarked,
								pImapNode->NoInferiors,
								pImapNode->AutoSync); 

		if (pNewImapCommand)
 			pNewImapCommand->SetDelimiter (pImapNode->Delimiter);


	}

	return pNewImapCommand;
}




/////////////////////////////////////////////////////////////////////////
// FetchMailboxAttributes
//
// FUNCTION
// This is a wrapper around CImapConnection::FetchMailboxAttributes ().
// Useful for seeing if a remote mailbox exists.
// END FUNCTION

// NOTES
// If returns an ImapMailboxNode that must be freed by the caller!!
// END NOTES
/////////////////////////////////////////////////////////////////////////
ImapMailboxNode* CImapAccount::FetchMailboxAttributes (LPCSTR pImapName, BOOL bSilent /* = TRUE */)
{
	ImapMailboxNode *pImapNode = NULL;

	// Sanity:
	if (!pImapName)
	{
		ASSERT (0);
		return NULL;
	}

	// Open a new control stream to the server.
	CString strAddress;
	GetAddress (strAddress);

	CImapConnection Imap (strAddress);

	// Initialize:
	CString szPortNum; GetPortNum (szPortNum);
	Imap.SetPortNum ( szPortNum );
	Imap.SetAccount (this);

	// Get a login and password for this account.
	//
	CString szLogin, szPassword;

	if ( !Login (szLogin, szPassword) )
	{
		return NULL;
	}

	Imap.SetLogin	 (szLogin);
	Imap.SetPassword (szPassword);

	// MUST do this!! Do it AFTER setting the acoount!
	//
	Imap.UpdateNetworkSettings ();

	BOOL bWePutUpProgress = FALSE;

	// Put up progres dialog if in main thread.
	if (!bSilent && ::IsMainThreadMT() )
	{
		bWePutUpProgress = TRUE;

		CString buf;
		buf.Format ( CRString(IDS_IMAP_CONNECTING_TO), strAddress );
		MainProgress(buf);
	}

	// Open a new control stream!!
	if (SUCCEEDED(Imap.OpenControlStream(this)))
	{
		// We can't stop the operation after this.
		if (bWePutUpProgress)
		{
			CloseProgress();

			// But, put up the wait cursor.
			CWaitCursor waitCursor;
		}

		// Reset CImapAccount internal state before a fetch.
		ResetInternalState ();


		// Get attributes now.
		pImapNode = Imap.FetchMailboxAttributes (pImapName);
	}
	else
	{
		if (bWePutUpProgress)
			CloseProgress();
	}

	return pImapNode;
}



// Login [PUBLIC]
//
// This doesn't really log into the IMAP server. It just gets a password,
// either a store one or from the user.
// Return FALSE if the user cancelled, TRUE if we got any password whatsoever.
// 
BOOL CImapAccount::Login(CString& szLogin, CString& szPassword)
{
	CString Login, Password;

	// MUST be called in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// If the user just cancelled a previous attempt, ignore this:
	//
	if ( UserJustCancelledLogin() )
	{
		return FALSE;
	}

	szPassword.Empty();

	// Initialize
	Login.Empty();
	Password.Empty();

	// Get the account's login name.
	//
	GetLogin(Login);

	// If "Kerberos" is selected for this account, don't prompt for a password.
	//
	if ( IsAuthKerberos () )
	{
		szLogin = Login;

		return TRUE;
	}

	// If we've got a non-empty (cached) password and it's flagged as valid, return that.
	//
	GetPasswordMT (Password);

	if ( !Password.IsEmpty() )
	{
//		if (m_PasswordValidity == PWVALIDITY_IS_VALID || m_PasswordValidity == PWVALIDITY_MAYBE_VALID)
		{
			// Copy to out parms. These may be empty.
			szLogin		= Login;
			szPassword	= Password;

			return TRUE;
		}
	}
	else //if ( MustSavePasswordToDisk() )
	{
		// Get saved password.
		GetSavedPassword (Password);
	}


	BOOL bGotNewLogin = FALSE;

	// Do we have to put up dialog box??
	if (Password.IsEmpty())
	{
		BOOL GOTLOGIN = ::GetPassword(Password, 0, FALSE, m_strPersona);

		// User cancelled?.
		if (!GOTLOGIN)
		{
			// User cancelled. Make sure these are empty.
			Login.Empty();
			Password.Empty();

			// Clear out parms.
			szLogin.Empty();
			szPassword.Empty();

			// Flag this!
			SetUserJustCancelledLogin();
		}
		else
		{
			bGotNewLogin = TRUE;
		}
	}

	// Copy to out parms. These may be empty.
	szLogin		= Login;
	szPassword	= Password;

	// Cache the password, but we won't know if it's valid until user attempt to log in.
	//
	SetPasswordMT ((LPCSTR)szPassword);
	
	if( bGotNewLogin && MustSavePasswordToDisk())
		SavePasswordToDisk();
	
	m_PasswordValidity = PWVALIDITY_MAYBE_VALID;
	//actually the above statement will not be of any use since the PWVALIDITY_MAYBE_VALID 
	//value is never used at all.- sagar.

	return !szPassword.IsEmpty();
}





// UserJustCancelled [PRIVATE]
// Note:
// Always use UserJustCancelled() in other methods. Don't try
// to reference m_tConnectionCancelled any place else.
//
BOOL CImapAccount::UserJustCancelledLogin ()
{
	time_t now = time (0);

	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return FALSE;
	}


	// Allow login on startup:
	//
	if (m_tLoginCancelled > now)
		return FALSE;
	else
		return (now - m_tLoginCancelled) <= tLoginCancelInterval;
}



// SetUserJustCancelled [PRIVATE]
// Note:
// Always use SetUserJustCancelled() in other methods. Don't try
// to reference m_tConnectionCancelled any place else.
//
void CImapAccount::SetUserJustCancelledLogin ()
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return;
	}

	// Just set the curent timestamp.
	//
	m_tLoginCancelled = time(0);
}




// InvalidatePasswordMT [PUBLIC]
//
// Signal that the cached password is invalid.
// If "bEraseSavedPassword" is TRUE, also remove any saved password for
// this account.
//
void CImapAccount::InvalidatePasswordMT (BOOL bEraseSavedPassword)
{
	// Make his thread safe..
	//
	CSingleLock localLock (m_pLoginLockable, FALSE);

	// Attempt to lock.
	//
	if (!localLock.Lock())
	{
		return;
	}

	// Login failed? Update in-memory flag.
	//
	m_PasswordValidity = PWVALIDITY_UNKNOWN;

	// Should we erase saved password??
	// Note: This just set the flag. Caller must arrange for the paswd to be cleared from disk 
	// in the main thread.
	//
	if ( bEraseSavedPassword )
	{	
		SetPasswordMT ("");
		//m_bPWNeedsSavingToDisk = TRUE;
		SavePasswordToDisk();
	}
}




// ValidatePasswordMT [PUBLIC]
//
//void CImapAccount::ValidatePasswordMT ()
//{
//	// Make his thread safe..
//	//
//	CSingleLock localLock (m_pLoginLockable, FALSE);
//
//	// Attempt to lock.
//	//
//	if (!localLock.Lock())
//	{
//		return;
//	}
//
//	// password must be non-empty to validate it.
//	if ( !m_Password.IsEmpty() )
//	{
//		// This may be called even though the password is already valid.
//		//
//		if ( m_PasswordValidity != PWVALIDITY_IS_VALID )
//		{
//			m_PasswordValidity = PWVALIDITY_IS_VALID;
//
//			// Force a save to disk in the main thread.
//			// Note: This method may be called in a worker thread, in which case
//			// we can't do the saving to disk here. Whoever calls this method
//			// will need to save the password in the main thread. Of course that
//			// depends on the INI setting for this account (which must be queried
//			// only in the main thread.
//			//
//			//m_bPWNeedsSavingToDisk = TRUE;
//			SavePasswordToDisk();
//		}
//	}
//}

//
//	CImapAccount::ShowError()
//
//	Show a message for the specified error in way that is appropriate for how the action was performed.
//	If we are online and the item is cached we will want to show the error differently than if we are
//	replaying a queue of actions.
//
//	This is a work in progress.
//
void CImapAccount::ShowError(UINT iError, CImapConnection *pImap, LPCSTR szImapName, TCHAR cDelimiter)
{
	if (!pImap)
	{
		return;
	}

	// Yeah, this is a pretty clunky way to do this.  It will get better over time, I promise.
	if ((iError == IDS_ERR_IMAP_SERVER_MBOX_DELETE) ||
		(iError == IDS_ERR_IMAP_CMD_FAILED_SRVMSG))
	{
		if (!szImapName || !*szImapName)
		{
			return;
		}
		QCImapMailboxCommand *pImapCommand = g_theMailboxDirector.ImapFindByImapName(GetAccountID(), 
																					 szImapName,
																					 cDelimiter);
		if (pImapCommand)
		{
			TCHAR  buf[512];
			int iType;
			buf[0] = '\0';

			pImap->GetLastImapErrorAndType(buf, 510, &iType);

// Don't want to go and throw up modal error dialog while doing this in the background.
#if 0
			if (buf[0])
			{
				ErrorDialog(iError, pImapCommand->GetName(), buf);
			}
			else
			{
				ErrorDialog(IDS_ERR_IMAP_COMMAND_FAILED);
			}
#endif
		}
		else
		{
#if 0
			ErrorDialog(IDS_ERR_IMAP_COMMAND_FAILED);
#endif
		}
	}
	else if (iError == IDS_ERR_IMAP_SERVER_MBOX_RENAME)
	{
#if 0
		TCHAR  buf[512];
		int iType;
		buf[0] = '\0';

		Imap.GetLastImapErrorAndType(buf, 510, &iType);

		if (buf[0])
		{
			ErrorDialog(iError, szOldImapName, szNewFullImapName, buf);
		}
		else
		{
			ErrorDialog(IDS_ERR_IMAP_COMMAND_FAILED);
		}
#endif
	}
}



//============================ non-class function ======================/

//
// Go get the account's delimiter char and update the command object's value.
//
void UpdateDelimiterChar (CImapConnection *pImap, CImapAccount *pAccount)
{
	TCHAR Delimiter = 0;

	// Sanity:
	if (! (pImap && pAccount) ) 
		return;

	pImap->GetTopMboxDelimiter (&Delimiter);

	if (Delimiter)
	{
		// Set it into the account.
		pAccount->SetTopDelimiter (Delimiter);

		// Find the top level command object for this account.
		CString strDirectory; pAccount->GetDirectory (strDirectory);

		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( strDirectory );

		QCImapMailboxCommand *pImapCommand = (QCImapMailboxCommand *)pCommand;

		if (pImapCommand)
			pImapCommand->SetDelimiter (Delimiter);
	}
}

	
#endif // IMAP4
