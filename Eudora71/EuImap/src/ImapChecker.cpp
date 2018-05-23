// ImapChecker.cpp
//
// Does the actual work of seeing what messages in a given mailbox need to be downloaded
// and downloading those messages, assigning junk scores if appropriate and filtering them
// if the mailbox is the Inbox.
//
// Copyright (c) 1997-2004 by QUALCOMM, Incorporated
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

#include "QCNetSettings.h"

#include "eudora.h"		// FOr IsVersion4()".
#include "mainfrm.h"
#include "doc.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "tocdoc.h"
#include "tocview.h"
#include "tocframe.h"
#include "progress.h"
#include "filtersd.h"
#include "persona.h"
#include "POPSession.h"
#include "Trnslate.h"
#include "JunkMail.h"

#include "ImapSum.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapConnection.h"
#include "ImapMailbox.h"
#include "ImapTypes.h"
#include "ImapMailMgr.h"
#include "ImapDownload.h"
#include "ImapFiltersd.h"
#include "ImapChecker.h"
#include "ImapConnectionMgr.h"
#include "ImapGuiUtils.h"
#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"


#include "DebugNewHelpers.h"


extern QCMailboxDirector		g_theMailboxDirector;


// Post processing statics
static void DoMergeNewTocs(void *pv, bool bLastTime);
static void DummyPostProc(void *pv, bool bLastTime);

//
//	DoMergeNewTocs()
//
//	Handles the post processing for the case where we have new mail to process.
//
void DoMergeNewTocs(void *pv, bool bLastTime)
{
	// Sanity:
	if (!pv)
	{
		return;
	}

	((CImapChecker*)pv)->DoMergeNewTocs();
}

//
//	DummyPostProc()
//
//	Handles the post processing for the case where we did not get new mail to process.
//
//	Task manager MUST have a post processing proc.  If we fail, we don't want to process
//	the messages we got, so use this as the post processing proc so we can, at least, delete
//	the CImapChecker object.
//
void DummyPostProc(void *, bool)
{
}


CImapChecker::CImapChecker(LPCSTR szPersona, LPCSTR szImapName, unsigned long ulCheckBitFlags) :
	m_strPersona(szPersona),
	m_strImapName(szImapName),
	m_ulBitFlags(ulCheckBitFlags),
	m_pImapConnection(NULL),
	m_pErrorCallback(NULL),
	m_pTocDoc(NULL),
	m_bIsInbox(FALSE),
	m_bIsJunk(FALSE),
	m_bStopRequested(FALSE),
	m_bIsReadOnly(FALSE),
	m_ulOldNumberOfMessages(0),
	m_ulNewNumberOfMessages(0),
	m_ulOldUIDHighest(0),
	m_ulNewUIDHighest(0),
	m_ulOldUidValidity(0),
	m_ulNewUidValidity(0),
	m_ulServerUidHighest(0),
	m_bGotNewMail(FALSE),
	m_bSilent(FALSE),
	m_bInMainTreadProgress(FALSE),
	m_bDoResync(FALSE),
	m_ulNewMsgs(0),
	m_ulMaxUidToResync(0),
	m_bDownloadedOnlyDuringResync(TRUE),
	m_bRemoveAllLocalSumsBeforeMerging(FALSE),
	m_enumWorkerFunctionType(WORKERFUNCTION_TYPE_BADTYPE)
{
	m_bIsInbox = ::IsInbox(m_strImapName);

	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	m_pTaskInfo->SetPersona(m_strPersona);
	m_pTaskInfo->SetTitle(CRString(IDS_POP_LOOKING_FOR_MAIL));

	m_pNewMailNotifier = NULL;

	// Construct our error callback.
	m_pErrorCallback = makeCallback((QCNetworkSettings::ErrorCallback*)0, *this, &CImapChecker::ErrorCallback);
}

CImapChecker::~CImapChecker()
{
	TRACE("In CImapChecker's destructor for %s\n", m_strImapName);

	m_mapCurrentUIDs.DeleteAll();
	m_mapNewUIDs.DeleteAll();
	m_mapModifiedUIDs.DeleteAll();
	m_mapNewOldUIDs.DeleteAll();
	m_islNewImapSummaries.DeleteAll();
	m_islNewOldImapSummaries.DeleteAll();

	if (m_pImapConnection)
	{
		m_pImapConnection->SetTaskInfo(NULL);
		GetImapConnectionMgr()->Release(m_pImapConnection);
	}

	GetImapMailMgr()->RemoveTask(m_strTaskKey);

	// We MUST do this before we exit!!!
	if (m_pNewMailNotifier)
	{
		m_pNewMailNotifier->Decrement();
	}
}


//
// Thread initialization.
//

//
// CImapChecker::DoMainThreadInitializations()
//
//	Perform the initializations that must be done in the main thread.
//	This includes getting the current state of the local cache
// Get current state of the local cache from the CImapMailbox.
// When we're done, we must let the CImapMailbox update its state from
// what new things we've learned.
//
// NOTE: This MUST be called in the main thread.
//
HRESULT CImapChecker::DoMainThreadInitializations(CTocDoc *pTocDoc)
{
	ASSERT(IsMainThreadMT());

	if (!pTocDoc)
	{
		return E_INVALIDARG;
	}

	// While it seems to make sense to set m_pTocDoc here, see CImapChecker::DoMergeNewTocs()
	// for why we don't yet do this. -dwiggins

	HRESULT			 hResult = S_OK;
	CImapMailbox	*pImapMailbox = NULL;

	pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		return E_FAIL;
	}

	m_bIsJunk = (pTocDoc->IsJunk() == TRUE);

	// Get the current state of the local cache.
	m_ulOldNumberOfMessages	= pImapMailbox->GetNumberOfMessages();
	m_ulOldUIDHighest		= pImapMailbox->GetUIDHighest();
	m_ulOldUidValidity		= pImapMailbox->GetUidvalidity();
	m_ulAccountID			= pImapMailbox->GetAccountID();

	// Default to the last known good values for data that might be updated.
	m_ulNewNumberOfMessages	= m_ulOldNumberOfMessages;
	m_ulNewUIDHighest		= m_ulOldUIDHighest;
	m_ulNewUidValidity		= m_ulOldUidValidity;

	// Grab the full path to the real MBX file.
	GetMbxFilePath(pImapMailbox->GetDirname(), m_strRealMbxFilePath);

	// Create the temporary MBX file.
	GetTmpMbxFilePath(pImapMailbox->GetDirname(), m_strTmpMbxFilePath);
	CreateLocalFile(m_strTmpMbxFilePath, TRUE);

	// Determine how much we will be downloading.
	m_bDoingMinimalDownload = pImapMailbox->GetSettingsShort(IDS_INI_IMAP_MINDNLOAD);

	// Obtain a connection to the IMAP server from the connections manager.
	// This MUST be done in the main thread. We don't open the
	// connection here but the CImapConnection must be setup in the main thread.
	hResult = AcquireNetworkConnection();

	if (!m_pImapConnection)
	{
		// $$$$$ SHOULD put up error message here!!
		//
		return E_FAIL;
	}

	// If the CImapConnection is not connected, make sure it has a password and login.
	if (!m_pImapConnection->IsConnected(m_pErrorCallback))
	{
		hResult = pImapMailbox->GetLogin();
	}

	// Set the task info pointers.
	if (m_pImapConnection->m_pNetSettings)
	{
		m_pImapConnection->m_pNetSettings->SetTaskInfo(m_pTaskInfo);
	}

	return hResult;
}

// AcquireNetworkConnection [PRIVATE]
//
// Note: This sets the m_pImapConnection object.
//
HRESULT CImapChecker::AcquireNetworkConnection()
{
	CImapConnection		*pImapConnection = NULL;

	// This might get an existing one.
	HRESULT hResult = GetImapConnectionMgr()->CreateConnection(m_ulAccountID,
															   m_strImapName,
															   &pImapConnection,
															   TRUE);

	if (pImapConnection && SUCCEEDED(hResult))
	{
		m_pImapConnection = pImapConnection;
		m_pImapConnection->SetTaskInfo(m_pTaskInfo);
	}

	return hResult;
}

//
//	CImapChecker::SetCheckMailWorkFunction()
//
//	This function is called to set up the CImapChecker object to perform a mail check.
//
//	NOTE: The calling code must call this or SetResyncWorkFunction() before performing
//	the thread portion.
//
void CImapChecker::SetCheckMailWorkFunction()
{
	m_enumWorkerFunctionType = WORKERFUNCTION_TYPE_CHECKMAIL;
}

//
//	CImapChecker::SetResyncWorkFunction()
//
//	This function is called to set up the CImapChecker object to perform a resync.
//
//	NOTE: The calling code must call this or SetCheckMailWorkFunction() before performing
//	the thread portion.
//
void CImapChecker::SetResyncWorkFunction(BOOL bDownloadedOnly, unsigned long ulMaxUid)
{
	m_enumWorkerFunctionType		= WORKERFUNCTION_TYPE_RESYNC;
	m_bDownloadedOnlyDuringResync	= bDownloadedOnly;
	m_ulMaxUidToResync				= ulMaxUid;
}


//
// Thread work.
//

//
//	CImapChecker::DoWork()
//
//	Called by the thread manager to do the first stage of the fetching.
//
// Note: This is always executed in a worker thread.
//
// Note:
// A CImapChecker can do mail checks. background resync, etc. The work function
// must be setup beforehand by calling one of:
// - CImapChecker::SetCheckMailWorkFunction(),
// - CImapChecker::SetResyncWorkFunction ().
// These set the internal function pointer to tha appropriate method.
//
HRESULT CImapChecker::DoWork()
{
	InitTaskInfo();

	// Default to a dummy post process function.  Depending on what happens in this mail check
	// we might end up using a different post process function.
	m_pTaskInfo->SetPostProcFn("Cleanup", ::DummyPostProc, this);

	// Perform the mail check.
	HRESULT		 hResult = DoCheckMailMT();
	if (!SUCCEEDED(hResult))
	{
		// The check failed, show the error.
		ShowCheckMailError();

		// Do we really need this???  Is it possible someone will set this to something else
		// even if the check failed?  If so, we should stop that or not set this same default
		// above. -dwiggins
		// m_pTaskInfo->SetPostProcFn("Cleanup", ::DummyPostProc, this);
	}

	// We are ready for post processing.
	m_pTaskInfo->SetPostProcessing(TRUE);

	// There is a pointer to the settings object
	// buried deep down here.  Make sure it
	// gets null'ed out so that it doesn't get used
	// once the task info has been destroyed
	m_pImapConnection->m_pNetSettings->SetTaskInfo(NULL);

	return hResult;
}

//
//	CImapChecker::InitTaskInfo()
//
//	Initialize the task info object.
//
void CImapChecker::InitTaskInfo()
{
	CString		 strTask;

	if (IsInbox())
	{
		if (m_ulBitFlags == kValidateLoginOnly)
		{
			strTask.LoadString(IDS_IMAP_VALIDATING);
			m_pTaskInfo->IgnoreIdle();
		}
		else
		{
			strTask.Format(CRString(IDS_IMAP_UPDATING_MAILBOX), GetTocName());

			// If a manual re-sync, such as after an expunge, don't wait for idle.
			if (WORKERFUNCTION_TYPE_RESYNC == m_enumWorkerFunctionType)
			{
				m_pTaskInfo->IgnoreIdle();
			}
		}
	}
	else
	{
		strTask.Format(CRString(IDS_IMAP_RESYNCING_MAILBOX), GetTocName());

		// In this case, don't wait for idle.
		m_pTaskInfo->IgnoreIdle();
	}

	m_pTaskInfo->SetMainText(strTask);
}

//
//	CImapChecker::ShowCheckMailError()
//
//	Displays the appropriate error for the mail check failure.
//
void CImapChecker::ShowCheckMailError()
{
	TCHAR		 szErrorBuf[1024];
	CString		 strErr = "Operation Failed: ";

	*szErrorBuf = '\0';

	if (m_pImapConnection)
	{
		m_pImapConnection->GetLastImapError(szErrorBuf, 1020);
	}

	if (*szErrorBuf)
	{
		strErr += szErrorBuf;
	}

	m_pTaskInfo->CreateError(strErr, TERR_MSG);
}

//
//	CImapChecker::StartResyncProgress()
//
//	Set up the progress display via the progress window if this is the main thread,
//	otherwise via the task info window.
//
//	Returns whether or not the progress window was already started.
//
BOOL CImapChecker::StartResyncProgress()
{
	BOOL			 bWasInProgress = FALSE;
	CString			 strProgress;
	strProgress.Format(CRString(IDS_IMAP_RESYNCING_MAILBOX), GetTocName());
	if (::IsMainThreadMT())
	{
		if (!m_bSilent)
		{
			if (InProgress)
			{
				bWasInProgress = TRUE;
				PushProgress();
			}
			MainProgress(strProgress);
		}
	}
	else if (m_pTaskInfo)
	{
		m_pTaskInfo->SetMainText(strProgress);
	}
	return bWasInProgress;
}

//
//	CImapChecker::StopResyncProgress()
//
//	Stop the resync progress display.
//
void CImapChecker::StopResyncProgress(BOOL bWasInProgress)
{
	if (::IsMainThreadMT() && !m_bSilent)
	{
		if (bWasInProgress)
		{
			PopProgress();
		}
		else
		{
			CloseProgress();
		}
	}
}

//
//	CImapChecker::StartResyncProgress()
//
//	Set up the progress display via the progress window if this is the main thread,
//	otherwise via the task info window.
//
void CImapChecker::StartDownloadProgress(int iProgressInc, CTaskInfoMT *pTaskInfo)
{
	CString			 strMainText;
	strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), iProgressInc);

	// Now go download remaining messages.
	if (InProgress && ::IsMainThreadMT() && (iProgressInc > 0))
	{
		MainProgress(strMainText);
	}
	else if (pTaskInfo && !IsMainThreadMT() && (iProgressInc > 0))
	{
		// Initialize for background progress.
		pTaskInfo->SetMainText(strMainText);
		pTaskInfo->SetTotal(iProgressInc);
	}
}

//
//	CImapChecker::UpdateDownloadProgress()
//
//	Update the progress display via the progress window if this is the main thread,
//	otherwise via the task info window.
//
void CImapChecker::UpdateDownloadProgress(int iProgressInc, CTaskInfoMT *pTaskInfo)
{
	CString			 strMainText;
	strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), iProgressInc);

	if (InProgress && ::IsMainThreadMT())
	{
		MainProgress(strMainText);
	}
	else if (!::IsMainThreadMT() && pTaskInfo)
	{
		pTaskInfo->SetMainText(strMainText);
		pTaskInfo->ProgressAdd(iProgressInc);
	}
}

//
//	CImapChecker::DoCheckMailMT()
//
//	Perform the mail check.
//
HRESULT CImapChecker::DoCheckMailMT()
{
	HRESULT		 hResult = S_OK;

	// Must have a valid persona and task info.
	if (m_strPersona.IsEmpty() || !m_pTaskInfo)
	{
		return E_FAIL;
	}

	// Setup the connection.
	hResult = SetupConnectionMT();
	if (!SUCCEEDED(hResult) || m_ulBitFlags == kValidateLoginOnly)
		return hResult;

	// Get things like new number of messages, etc, from the server.
	hResult = GetNewMboxState();
	if (!SUCCEEDED(hResult))	return hResult;

	// Get the list of new UID's in the remote mailbox.
	hResult = FetchNewUIDsMT();
	if (!SUCCEEDED(hResult))	return hResult;

	// Get the number of new messages in the remote mailbox.
	m_ulNewMsgs = m_mapNewUIDs.GetCount();

	// Determine what messages we need to sync.
	if (m_bDoResync)
	{
		// We have to do resynchronizations to determine which messages we
		// download or re-download.
		hResult = DoResyncMailboxMT();
		if (!SUCCEEDED(hResult))	return hResult;
		m_bDoResync = FALSE;
	}
	else
	{
		// If we're not re-syncing, we must remove UID's from current list so we don't
		// think it contains messages no longer on the server.
		m_mapCurrentUIDs.DeleteAll();
	}

	// At this point m_mapNewUIDs will contain the UID's of the messages that must be retrieved.

	// Download new messages to the temporary TOC and MBX.
	// Note that we don't check the return value of this function because even if it failed
	// it might have downloaded some messages so we want to proceed and merge anything we got.
	// We'll get the rest next time.
	DownloadNewMessagesToTmpTocMT();

	// Set up post-processing to merge the temporary TOC and MBX in the foreground.
	if (m_pTaskInfo)
	{
		m_pTaskInfo->SetMainText(CRString (IDS_IMAP_WAITING_FOR_POSTPROCESS));
		m_pTaskInfo->SetPostProcFn("Update Mailbox", ::DoMergeNewTocs, this);
	}

	return S_OK;
}

//
//	CImapChecker::SetupConnectionMT()
//
// Establish initial conditions and setup network for checking mail.
//
// NOTE: This can be done in the main thread!!!
// Question: Why do we care?  This is always called in a worker thread. -dwiggins
//
HRESULT CImapChecker::SetupConnectionMT()
{
	HRESULT			hResult = S_OK;

	// Reset this.
	m_bStopRequested = FALSE;

	if (!m_pImapConnection)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Why must we constantly be passing in m_pErrorCallback? -dwiggins

	// Open the connection if the connection is not already open.
	// If we dropped the connection for whatever reason, try to re-connect.
	//
	if (!m_pImapConnection->IsSelected(m_pErrorCallback))
	{
		if (m_ulBitFlags == kValidateLoginOnly)
			hResult = m_pImapConnection->OpenControlStream(m_pImapConnection->m_pAccount, FALSE, m_pErrorCallback);
		else
			hResult = m_pImapConnection->OpenMailbox(m_strImapName, TRUE, m_pErrorCallback);
	}

	return hResult;
}

//
//	CImapChecker::GetNewMboxState()
//
// Get m_ulNewNumberOfMessages and m_ulServerUidHighest, etc, in preparation for the 
// real jobs.
//
HRESULT CImapChecker::GetNewMboxState()
{
	HRESULT hResult = S_OK;

	// Reset this.
	m_bStopRequested = FALSE;

	// We must have a connection object which is selected.  Previously this function
	// would call OpenMailbox() if the connection object was not selected, but we do
	// the exact same thing in SetupConnectionMT() which is called immediately before
	// this so why do it twice?
	if (!m_pImapConnection || !m_pImapConnection->IsSelected(m_pErrorCallback))
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Send a NOOP so we get any pending EXISTS messages.
	m_pImapConnection->Noop(m_pErrorCallback);

	// The UIDVALIDITY may have changed.
	unsigned long ulNewUV = 0;
	hResult = m_pImapConnection->GetUidValidity(ulNewUV, m_pErrorCallback);
	if (!SUCCEEDED(hResult))	return hResult;
	m_ulNewUidValidity = ulNewUV;

	// See how many messages the server has.
	hResult = m_pImapConnection->GetMessageCount(m_ulNewNumberOfMessages, m_pErrorCallback);
	if (!SUCCEEDED(hResult))	return hResult;

	// See what the server's highest message UID is.
	hResult = m_pImapConnection->UIDFetchLastUid(m_ulServerUidHighest, m_pErrorCallback);
	if (!SUCCEEDED(hResult))	return hResult;

	// If we think our cache became out of sync set the flag to force a resync.
	if (m_ulOldNumberOfMessages != (unsigned long)m_mapCurrentUIDs.GetCount())
	{
#if 0
		// Temporary logging until we get to the root of the unnecessary resync problem. -dwiggins
		CString			 str;
		str = "Forcing resync of %s (old=%ld, new=%ld)";
		str.Format(str, m_strImapName, m_ulOldNumberOfMessages, (unsigned long)m_mapCurrentUIDs.GetCount());
		::PutDebugLog(DEBUG_MASK_DIALOG, str);
#endif

		ForceResync();
	}

	return hResult;
}

//
//	CImapChecker::FetchNewUIDsMT()
//
//	Generates a list of UIDs of new messages to download.
//
//	At the end of this method m_mapNewUIDs will contain a list of UIDs of messages on the server
//	which we have never seen.
//
HRESULT CImapChecker::FetchNewUIDsMT()
{
	HRESULT			 hResult = S_OK;
	unsigned long	 ulNextUid = 0; 

	if (!m_pImapConnection)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Make sure this is empty:
	m_mapNewUIDs.DeleteAll();

	// If there are messages in the mailbox, m_ulServerUidHighest MUST be non-zero.
	if ((m_ulNewNumberOfMessages > 0) && (m_ulServerUidHighest == 0))
	{
		m_pImapConnection->Close();

		// Open (SELECT) the mailbox on the server.
		hResult = m_pImapConnection->OpenMailbox(m_strImapName, TRUE, m_pErrorCallback);
		if (!SUCCEEDED(hResult))	return hResult;

		// Get the number of messages in the mailbox.
		hResult = m_pImapConnection->GetMessageCount(m_ulNewNumberOfMessages, m_pErrorCallback);
		if (!SUCCEEDED(hResult))	return hResult;

		// Get the mailbox's UIDVALIDITY.
		unsigned long		 ulNewUV = 0;
		hResult = m_pImapConnection->GetUidValidity(ulNewUV, m_pErrorCallback);
		if (!SUCCEEDED(hResult))	return hResult;
		m_ulNewUidValidity = ulNewUV;

		// Get the highest message UID in the mailbox.
		hResult = m_pImapConnection->UIDFetchLastUid(m_ulServerUidHighest, m_pErrorCallback);
		if (!SUCCEEDED(hResult))	return hResult;
	}

	// If there are no messages in remote mailbox, just delete our local cache.
	if (m_ulNewNumberOfMessages == 0)
	{
		// m_mapCurrentUIDs is no longer valid.  Flush it and redownload all.
		// Ummm... clearing out the list now before we are ready to delete the related summaries
		// actually prevents flushing and causes these "stale" messages to remain around when they
		// should be removed. - dwiggins
		// m_mapCurrentUIDs.DeleteAll();

		// We must clear our local cache and redownload everything.
		m_bRemoveAllLocalSumsBeforeMerging = TRUE;

		// Don't change our concept of highest UID, unless the UIDVALIDITY changed.
		if (m_ulNewUidValidity != m_ulOldUidValidity)
		{
			// We must refetch everything next time.
			m_ulNewUIDHighest = m_ulOldUIDHighest = 0;
		}
		else
		{
			// UIDVALIDITY didn't change so our highest UID value is OK.
			m_ulNewUIDHighest = m_ulOldUIDHighest;
		}
	
		// Set m_ulMaxUidToResync to the highest server UID so we redownload everything
		// during the re-sync phase.
		m_ulMaxUidToResync = m_ulServerUidHighest;

		return hResult;
	}

	// If the UIDVALIDITY has changed, we need to just re-download every message.
	if (m_ulNewUidValidity != m_ulOldUidValidity)
	{
		// m_mapCurrentUIDs is no longer valid. Flush it and redownload all.
		// Ummm... clearing out the list now before we are ready to delete the related summaries
		// actually prevents flushing and causes these "stale" messages to remain around when they
		// should be removed. - dwiggins
		// m_mapCurrentUIDs.DeleteAll();

		// In this case, filtering doesn't make sense because UID's are no longer valid.
		// Set a flag indicating that, during the post processing, we need to remove all
		// local summaries before merging the new ones.
		m_bRemoveAllLocalSumsBeforeMerging = TRUE;

		// However, set new UIDHighest to the current server's highest UID so we treat
		// all messages as if they were previously seen,
		m_ulNewUIDHighest = m_ulServerUidHighest;

		// Also, old cached UID highest values are no longer valid.
		m_ulOldUIDHighest = m_ulServerUidHighest;
	
		// Set m_ulMaxUidToResync to the highest server uid so we redownload everything
		// during the re-sync phase.
		m_ulMaxUidToResync = m_ulServerUidHighest;

		return hResult;
	}

	// Handle the manual resync case.
	if (m_enumWorkerFunctionType == WORKERFUNCTION_TYPE_RESYNC)
	{
		if (m_bDownloadedOnlyDuringResync)
		{
			// If this is a manual resync and we're re-syncing only previously downloaded messages,
			// just set flags and limits and return.
			m_ulMaxUidToResync = m_ulOldUIDHighest;

			// Don't update UIDHighest in the CImapMailbox because we're not fetching new mail.
			m_ulNewUIDHighest = m_ulOldUIDHighest;

			// Don't continue.
			return hResult;
		}
		else 
		{
			// Fetch new mail AND update the CImapMailbox's UIDHighest.
			m_ulMaxUidToResync = m_ulServerUidHighest;
			m_ulNewUIDHighest = m_ulServerUidHighest;
		}
	}

	// If we get here, we're downloading new messages.

	// The current uid list (from the TOC) would have been calculated and set
	// during the main thread, before starting this procedure.
	//
	// Get the highest one we have locally and download the next one.
	ulNextUid = max(m_mapCurrentUIDs.GetHighestUid(), m_ulOldUIDHighest);

	// We may have removed messages at the tail end of the mailbox.
	ulNextUid = min(m_ulServerUidHighest, ulNextUid);

	// Did user abort??
	if (m_bStopRequested)
	{
		return HRESULT_MAKE_CANCEL;
	}

	// Now, fetch only if server has higher UID.
	if ((m_ulServerUidHighest > 0) && (m_ulServerUidHighest > ulNextUid))
	{
		CString		 strSeq;
		strSeq.Format ("%lu:%lu", ulNextUid + 1, m_ulServerUidHighest);
		hResult = m_pImapConnection->FetchFlags(strSeq, &m_mapNewUIDs, m_pErrorCallback);
	}

	// If we're re-syncing, resync up to ulNextUid.
	m_ulMaxUidToResync = ulNextUid;

	return hResult;
}

//
//	CImapChecker::DoResyncMailboxMT()
//
//	Perform all necessary resynchronizations of summaries for messages currently in our cache.
//	If an individual message's flags have change it will be necessary to redownload them.
//	If the UIDVALIDITY changed or if the TOC was rebuilt we will need to redownload all of the
//	messages.
//
//	This function assumes m_mapNewUIDs already contains a list of UIDs for new messages.
//
//	At the end of this method the following member variables will hold the described data:
//		m_mapNewUIDs contains only UIDs of truly new messages.
//		m_mapNewOldUIDs contains only UIDs of messages that are on the server but are not
//			in our local cache, for whatever reason. 
//		m_mapCurrentUIDs contains only UIDs of message in our local cache that are no longer
//			on the server.  (Yes, I find it confusing that "current" now means "orphaned" but
//			this is the way it has been done and maintaining a new list just for this seems
//			extreme.  Live with it.)
//		m_mapModifiedUIDs contains UIDs of messages in our local cache that are still on the
//			server, but whose m_Imflags may have new values.
//
HRESULT CImapChecker::DoResyncMailboxMT()
{
	HRESULT		hResult = S_OK;
	BOOL		bWasInProgress = FALSE;

	if (!m_pImapConnection)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Clear internal lists.
	m_mapModifiedUIDs.DeleteAll();
	m_mapNewOldUIDs.DeleteAll();

	// Make sure there is something to resync.
	if ((m_ulMaxUidToResync == 0) || (m_ulNewNumberOfMessages == 0))
	{
		return S_OK;
	}

	// Setup the progress display.
	bWasInProgress = StartResyncProgress();

	// Fetch uid's into m_mapNewOldUIDs.
	CString			 strSeq;
	strSeq.Format("1:%lu", m_ulMaxUidToResync);
	hResult = m_pImapConnection->FetchFlags(strSeq, &m_mapNewOldUIDs, m_pErrorCallback);

	// If the FETCH failed or the user cancelled, clean up and bail.
	if (m_bStopRequested || !SUCCEEDED(hResult))
	{
		m_mapNewOldUIDs.DeleteAll();

		StopResyncProgress(bWasInProgress);

		if (m_bStopRequested)
		{
			return HRESULT_MAKE_CANCEL;
		}
		return E_FAIL;
	}

	// At this point, m_mapNewOldUIDs may contain UIDs that are also in m_mapNewUIDs.
	// Before doing the merge remove from m_mapNewOldUIDs any UIDs that are also in m_mapNewUIDs.
	RemoveSubUidMapMT(m_mapNewOldUIDs, m_mapNewUIDs);

	// At this point, m_mapCurrentUIDs may contain UIDs that are also in m_mapNewUIDs.
	// Duplicates are removed from both lists and if a duplicate's flags have changed the
	// duplicate is added to m_mapModifiedUIDs.
	MergeUidMapsMT(m_mapCurrentUIDs, m_mapNewOldUIDs, m_mapModifiedUIDs);

	// Cleanup.
	StopResyncProgress(bWasInProgress);

	return S_OK;
}

//
//	CImapChecker::DownloadNewMessagesToTmpTocMT()
//
//	Download truly new messages (m_mapNewUIDs) and messages that are missing from
//	our cache (m_mapNewOldUIDs) into a temporary MBX file.  The real work is done
//	by calling DownloadRemainingMessagesMT() on each of these two UID lists.
//
HRESULT CImapChecker::DownloadNewMessagesToTmpTocMT()
{
	HRESULT		 hResult = S_OK;

	// First download messages we've never seen.
	m_pTaskInfo->SetMainText(CRString(IDS_IMAP_FETCHING_NEWMSGS));
	hResult = DownloadRemainingMessagesMT(m_mapNewUIDs, &m_islNewImapSummaries, TRUE, m_pTaskInfo);

	// The download may have been interrupted, so the actual number of messages downloaded is the
	// count of m_islNewImapSummaries.
	m_ulNewMsgs += m_islNewImapSummaries.GetCount();

	if (!SUCCEEDED(hResult))
	{
		if (m_pTaskInfo)
		{
			m_pTaskInfo->CreateError("Some message headers may not have been downloaded.", TERR_MSG);
		}
		return hResult;
	}

	// Next fetch messages that need to be redownload.
	m_pTaskInfo->SetMainText(CRString(IDS_IMAP_REDOWNLOADING_MSGS));
	hResult = DownloadRemainingMessagesMT(m_mapNewOldUIDs, &m_islNewOldImapSummaries, TRUE, m_pTaskInfo);

	if (!SUCCEEDED(hResult))
	{
		if (m_pTaskInfo)
		{
			m_pTaskInfo->CreateError("Some message headers may not have been downloaded.", TERR_MSG);
		}
	}

	return hResult;
}

//
//	CImapChecker::DownloadRemainingMessagesMT()
//
//	Download the messages from uidmap into pSumList.
//
HRESULT CImapChecker::DownloadRemainingMessagesMT(CUidMap &uidmap,
												  CImapSumList *pSumList,
												  BOOL bToTmpMbx,
												  CTaskInfoMT *pTaskInfo)
{
	HRESULT		 hResult = S_OK;
	IMAPUID		 uid = 0;
	CImapSum	*pImapSum = NULL;

	// Must have a summary list object.
	if (!pSumList)
	{
		return E_INVALIDARG;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Get new value of count. Note that uidmap contains new messages only.
	int			 nCountAfterFiltering = uidmap.size();

	// Use actual count of messages.  The old code used a percent which is wrong in so many ways
	// that I won't take the time to go into them.
	int			 iProgressInc = nCountAfterFiltering;
	StartDownloadProgress(iProgressInc, pTaskInfo);

	// Download the message.
	// FORNOW - enforce minimal download if in background:
	if (m_bDoingMinimalDownload || !IsMainThreadMT())
	{
		if (uidmap.size() > 0)
		{
			// Return a partially filled-in new CImapSum object.
			hResult = DoMinimalDownload(uidmap, pSumList, &m_ulNewUIDHighest);
		}
	}
	else
	{
		// Download the complete message.
		pImapSum = DEBUG_NEW_MFCOBJ_NOTHROW CImapSum;
		if (pImapSum)
		{
			pImapSum->SetHash (uid);

			// Fetch message, probably including all attachments.
			hResult = DownloadSingleMessage(pImapSum, FALSE, bToTmpMbx);

			if (SUCCEEDED(hResult))
			{			
				// Indicate that this was more than a minimal download.
				pImapSum->m_Imflags &= ~IMFLAGS_NOT_DOWNLOADED;

				// If this our highest UID so far?
				if (uid > m_ulNewUIDHighest)
				{
					m_ulNewUIDHighest = uid;
				}
			}
			else
			{
				// We couldn't download the message so clean up the summary.
				delete pImapSum;
				pImapSum = NULL;
			}
		}
	}

	return hResult;
}

//
//	CImapChecker::DoMinimalDownload
//
//	Do a minimal download just to get the attributes for the specified message(s).
//	This method operates on a uidmap and fills in a CImapSumList with one CImapSum
//	for each message specified in the uidmap.
//
HRESULT CImapChecker::DoMinimalDownload(CUidMap &uidmap,
										CImapSumList *pSumList,
										unsigned long *puiHighestUID)
{
	HRESULT			 hResult = S_OK;

	if (pSumList == NULL)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	if (!m_pImapConnection)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Instantiate a CImapDownloader object to do the actual downloading.
	CImapDownloader		 imapDownloader(m_ulAccountID, m_pImapConnection, NULL);
	hResult = imapDownloader.DoMinimalDownload(uidmap, pSumList, puiHighestUID);

	return hResult;
}

//
//	CImapChecker::DownloadSingleMessage()
//
//	Create a CImapDownloader object to download the specified message and fill
//	its data into pSum.
// 
HRESULT CImapChecker::DownloadSingleMessage(CImapSum *pSum,	BOOL bDownloadAttachments, BOOL bToTmpMbx)
{
	HRESULT		 hResult = S_OK;

	if (!pSum)
	{
		return E_INVALIDARG;
	}

	if (!m_pImapConnection)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Open the MBX file for appending. This can be either the real MBX file or the temporary one.
	CString		 strMbxFilePath;
	if (bToTmpMbx)
	{
		strMbxFilePath = m_strTmpMbxFilePath;
	}
	else
	{
		strMbxFilePath = m_strRealMbxFilePath;
	}

	// Instantiate an ImapDownloader object to do the downloading.
	CImapDownloader		 imapDownloader(m_ulAccountID, m_pImapConnection, (LPCSTR)strMbxFilePath);

	// 	Make sure the file exists. Open it for appending. Don't truncate!!
	if (!imapDownloader.OpenMbxFile(FALSE))
	{
		// ErrorDialog(IDS_ERR_FILE_OPEN, szMbxFilePath, CRString(IDS_ERR_FILE_OPEN_WRITING));
		return E_FAIL;
	}

	// Go download it.
	hResult = imapDownloader.DownloadSingleMessage(pSum, bDownloadAttachments);

	// Note: Only the CImapDownloader can set IMFLAGS_NOT_DOWNLOADED into the summary.
	// Don't do it here because bResult can be TRUE if we wrote a dummy message to the MBX file.

	// Close the file.
	imapDownloader.CloseMbxFile();

	return hResult;
}

//
//	CImapChecker::DownloadSingleMessageFully()
//
//	Create a CImapDownloader object to fully download the specified message and fill
//	its data into pSum.
// 
HRESULT CImapChecker::DownloadSingleMessageFully(CImapSum *pSum)
{
	HRESULT		 hResult = S_OK;

	if (!pSum)
	{
		return E_INVALIDARG;
	}

	if (!m_pImapConnection)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Instantiate an ImapDownloader object to do the downloading.
	CImapDownloader		 imapDownloader(m_ulAccountID, m_pImapConnection, (LPCSTR)m_strRealMbxFilePath);

	// 	Make sure the file exists. Open it for appending. Don't truncate!!
	if (!imapDownloader.OpenMbxFile(FALSE))
	{
		// ErrorDialog(IDS_ERR_FILE_OPEN, szMbxFilePath, CRString(IDS_ERR_FILE_OPEN_WRITING));
		return E_FAIL;
	}

	// Go download it.
	hResult = imapDownloader.DownloadSingleMessageFully(pSum);

	// Note: Only the CImapDownloader can set IMFLAGS_NOT_DOWNLOADED into the summary.
	// Don't do it here because bResult can be TRUE if we wrote a dummy message to the MBX file.

	// Close the file back.
	imapDownloader.CloseMbxFile();

	return hResult;
}


//
// Thread post processing.
//

//
//	CImapChecker::DoMergeNewTocs()
//
// Do the postprocessing in the main thread.
//
// Here, we must check to see if the TOC we're dealing with is still open!!
//
HRESULT CImapChecker::DoMergeNewTocs()
{
	BOOL		 bResult = TRUE;
	BOOL		 bWeCreatedToc = FALSE;

	ASSERT(IsMainThreadMT());

	TRACE("Starting DoMergeNewTocs for %s\n", m_strImapName);

	// Try to obtain a pointer to the TocDoc so we know if we loaded it.
	// It isn't clear to me that the following steps are strictly necessary (t seems to me that
	// we are always called with a loaded TOC) but I'm not ready to make that assumption just yet.
	// For now, once we are sure the TOC is loaded here, set our m_pTocDoc member. -dwiggins
	CTocDoc		*pTocDoc = GetToc(m_strRealMbxFilePath, NULL, FALSE, TRUE);

	if (!pTocDoc)
	{
		// We're loading it.
		pTocDoc = GetToc(m_strRealMbxFilePath);
		bWeCreatedToc = TRUE;
	}

	if (!pTocDoc)
	{
		TRACE("Exiting DoMergeNewTocs for %s\n", m_strImapName);
		return E_FAIL;
	}

	m_pTocDoc = pTocDoc;

	// Must have an IMAP mailbox object.
	CImapMailbox	*pImapMailbox = m_pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT(0);

		// Close TOC if we opened it.
		if (bWeCreatedToc)
		{
			m_pTocDoc->m_bAutoDelete = TRUE;
			m_pTocDoc->OnCloseDocument();
		}

		TRACE("Exiting DoMergeNewTocs for %s\n", m_strImapName);

		return E_FAIL;
	}

	// Should we put up progress??
	ULONG			 ulTotLoad = m_islNewOldImapSummaries.GetCount() + m_islNewImapSummaries.GetCount();
	BOOL			 bInProgress = FALSE;

	// Put up progress immediately if load is enough (the load value is arbitrary).
	if (ulTotLoad > 10)
	{
		// Put up progress immediately:
		ShowPostProcessProgress(m_pTocDoc->Name());
		bInProgress = TRUE;
	}

	// Change the current persona here but we MUST reset it before we leave.
	CString			 strCurrPersona = g_Personalities.GetCurrent();
	g_Personalities.SetCurrent(m_strPersona);

	// If the mailbox is open, stop redrawing for now and turn it back on later.
	CTocView		*pView = m_pTocDoc->GetView();
	if (pView)
	{
		pView->m_SumListBox.SetRedraw(FALSE);
	}

	// If there are new or redownloaded messages to post process do it now.
	if (ulTotLoad > 0)
	{
		// Post process the summary lists containing the redownloaded messages and the new messages.
		bResult = PostProcessSummaries();
	}
	else
	{
		// If there were no messages to process, treat this as success.
		// Note that with the current code flow setting bResult to TRUE isn't necessary since 
		// will be TRUE but we don't want future changes to cause problems. -dwiggins
		bResult = TRUE;
	}

	if (bResult)
	{
		// Update existing summary attributes.
		pImapMailbox->UpdateOldSummaries(m_pTocDoc, m_mapModifiedUIDs);

		// Now, remove stale summaries first.
		if (m_mapCurrentUIDs.size() > 0)
		{
			int		 iHighlightIndex = 0;
			m_pTocDoc->ImapRemoveListedTocs(&m_mapCurrentUIDs, iHighlightIndex, TRUE, TRUE);
		}
	}

	// Make sure to do this!!!
	m_slPostFilterSummaries.RemoveAll();

	// Save the TOC before we update cache, in case of failure later.
	m_pTocDoc->SaveModified();

	// Update the local cache data.
	if (bResult)
	{
		// Update the count of messages in the local cache.
		pImapMailbox->SetNumberOfMessages(m_ulNewNumberOfMessages);

		// Update UIDVALIDITY if necessary.
		if (m_ulNewUidValidity != m_ulOldUidValidity)
		{
			pImapMailbox->SetUidvalidity(m_ulNewUidValidity);
		}

		// Update the highest UID seen.
		pImapMailbox->SetUIDHighest(m_ulNewUIDHighest);
	}

	// Update saved info, whether we succeeded or not.
	pImapMailbox->WriteImapInfo(TRUE);

	// If this is the Inbox tell the TOC it needs FUMLUB so that the selection will be correct after
	// filtering.  This is particularly a problem with hidden deleted messages.
	if (IsInbox() && (ulTotLoad > 0))
	{
		m_pTocDoc->SetNeedsFumlub();
	}

	// Refresh the TOC.
	if (pView)
	{
		// Maybe reset the contents if some messages arrived.  Truthfully we shouldn't be resetting the
		// contents unless at least one message was marked as deleted in post processing (via junking or
		// filtering).
		if (ulTotLoad > 0)
		{
			pView->MaybeResetContents();
		}
		RedrawTOC(pView);
	}

	// Finally, notify manager if we got new mail.
	SetNotificationFlags();

	// Reset persona.
	g_Personalities.SetCurrent(strCurrPersona);

	// If we're offline, close the connection after us.
	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		pImapMailbox->Close();
	}

	// Close the TOC if we opened it.
	if (bWeCreatedToc)
	{
		m_pTocDoc->m_bAutoDelete = TRUE;
		m_pTocDoc->OnCloseDocument();

		// This is no longer valid.
		pImapMailbox = NULL;
	}

	TRACE("Exiting DoMergeNewTocs for %s\n", m_strImapName);

	// Close progress.
	if (bInProgress)
	{
		CloseProgress();
	}

	return bResult ? S_OK : E_FAIL;
}

//
//	CImapChecker::ShowPostProcessProgress()
//
//	Display the post process progress.
//
void CImapChecker::ShowPostProcessProgress(CString &strTocName)
{
	// CImmediateProgress Imp;
	CString		 strProgress;
	strProgress.Format(CRString(IDS_IMAP_DOING_POSTPROCESS), strTocName);
	MainProgress(strProgress);
}

//
//	CImapChecker::PostProcessSummaries()
//
//	Post process the summary lists containing the redownloaded messages and the new messages.
//
BOOL CImapChecker::PostProcessSummaries()
{
	BOOL			 bResult = FALSE;

	if (!m_pTocDoc || !m_pTocDoc->m_pImapMailbox)
	{
		return FALSE;
	}

	CImapMailbox	*pImapMailbox = m_pTocDoc->m_pImapMailbox;

	// Cache the number of summaries in the TOC before adding anything.
	int				 iNumSumsAtStart = m_pTocDoc->NumSums();

	// If we need to remove all local summaries before merging, go do that now.
	if (m_bRemoveAllLocalSumsBeforeMerging)
	{
		m_pTocDoc->ImapRemoveAllSummaries();
	}

	// Process redownloaded messages now.

	// If we need to download message content for redownloaded messages, do that here.
	HandlePostFetch(m_islNewOldImapSummaries);

	// Add the summaries to the TocDoc's list.
	bResult = pImapMailbox->ImapAppendSumList(m_pTocDoc, &m_islNewOldImapSummaries, NULL, TRUE);

	// Process new messages now.

	// "m_slPostFilterSummaries" gets filled with references to the summaries added. These
	// are the ones we need to apply the "post filtering" on.
	m_slPostFilterSummaries.RemoveAll();

	if (bResult)
	{
		// Don't set the internal "Got new mail" flag until we're sure we've added new summaries to INBOX.
		BOOL bGotNewMessages = FALSE;

		// If we need to download message content for new messages, do that here.
		// Note that this is fine for downloading content but not for doing anything with summaries
		// since we don't add the summaries to the TOC until ImapAppendSumList().
		HandlePostFetch(m_islNewImapSummaries);

		// Add the summaries to the TocDoc's list.
		bResult = pImapMailbox->ImapAppendSumList(m_pTocDoc,
												  &m_islNewImapSummaries,
												  &m_slPostFilterSummaries,
												  TRUE);

		// Do the non-downloading portions of the post processing after the summaries have been
		// added to the TOC above.

		// Do the junk processing.
		DoJunkProcessing(m_islNewImapSummaries);

		// Set this after HandlePostFetch() because it now does junk handling
		// which might junk some or all of the incoming mail.  Also, this previously
		// only cared about the number of summaries in m_islNewImapSummaries which doesn't
		// capture every case.  Now also check to see if the number of summaries has grown.
		bGotNewMessages = ((m_islNewImapSummaries.GetCount() > 0) || (m_pTocDoc->NumSums() > iNumSumsAtStart));

		if (bResult)
		{
			if (bGotNewMessages)
			{
				m_bGotNewMail = TRUE;
			}

			// If this in the Inbox, perform post filtering here.
			if (IsInbox() &&
				GetIniShort(IDS_INI_FILTER_INCOMING_MAIL) &&
				GetIniShort(IDS_INI_IMAP_FILTER_INCOMING))
			{
				// Create a new sumlist containing only summaries for messages that are not
				// marked as deleted.  We don't want to be filtering deleted messages, but there
				// is a timing issue involved where transferring a message causes an expunge which
				// causes deleted message summaries to go away, causing a potential crash.  This
				// change is a stop-gap measure because handling of deleted summaries is due for a
				// revamp very shortly. -dwiggins
				CSumList	 sumlistNotDeletedSummaries;
				POSITION	 pos = NULL;
				POSITION	 posNext = NULL;

				pos = m_slPostFilterSummaries.GetHeadPosition();

				for (posNext = pos; pos; pos = posNext)
				{
					CSummary	*pSum = m_slPostFilterSummaries.GetNext(posNext);
					if (pSum && !(pSum->m_Imflags & IMFLAGS_DELETED))
					{
						sumlistNotDeletedSummaries.AddTail(pSum);
					}
				}

				DoPostFiltering(sumlistNotDeletedSummaries);
			}
		}
	}
	return bResult;
}

//
//	CImapChecker::HandlePostFetch()
// 
// Since the background stuff just did minimal fetches, we need to do any
// requested on-check fetch in the main thread (until this becomes thread safe).
// Go do that now.
//
HRESULT CImapChecker::HandlePostFetch(CImapSumList& imapSumList)
{
	ASSERT(IsMainThreadMT());

	HRESULT			 hResult = S_OK;
	POSITION		 pos = imapSumList.GetHeadPosition();
	POSITION		 posNext = NULL;
	CImapSum		*pImapSum = NULL;
	CObArray		oaABHashes;

	// Iterate over the messages, downloading as appropriate.
	for (posNext = pos; pos; pos = posNext)
	{
		pImapSum = (CImapSum*)imapSumList.GetNext(posNext);

		if (pImapSum)
		{
			if (pImapSum->IsFullDownloadMarked())
			{
				hResult = DownloadSingleMessageFully(pImapSum);
			}
			else if (!m_bDoingMinimalDownload)
			{
				hResult = DownloadSingleMessage(pImapSum, FALSE, FALSE);
			}
			if (!SUCCEEDED(hResult))
			{
				break;
			}
		}
	}

	return hResult;
}

//
//	CImapChecker::DoJunkProcessing()
//
//	Perform junk processing on the messages included in imapSumList.
//
HRESULT CImapChecker::DoJunkProcessing(CImapSumList& imapSumList)
{
	ASSERT(IsMainThreadMT());

	HRESULT			 hResult = S_OK;
	bool			 bUseJunkMailbox = false;
	bool			 bUsingWhiteList = false;
	bool			 bDidJunk = false;
	POSITION		 pos = imapSumList.GetHeadPosition();
	POSITION		 posNext = NULL;
	CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_ulAccountID);
	CImapSum		*pImapSum = NULL;
	CObArray		oaABHashes;

	if (!pAccount || !pAccount->DoJunkScoring())
	{
		return E_FAIL;
	}

	if (pAccount->UseJunkMailbox())
	{
		bUseJunkMailbox = true;
	}

	CFilter::GenerateHashes(&oaABHashes);
	bUsingWhiteList = (GetIniShort(IDS_INI_ADDBOOK_IS_WHITELIST) != 0);

	// Iterate over the messages, junk scoring as appropriate.
	for (posNext = pos; pos; pos = posNext)
	{
		pImapSum = (CImapSum*)imapSumList.GetNext(posNext);

		if (pImapSum)
		{
			// If we didn't download the message above we may want to process
			// it for junk now.  (If we did download it above, it will have
			// been scored based on the entire message and scoring on headers
			// alone would be meaningless.)
			ProcessMessageForJunk(pImapSum, &oaABHashes, bUsingWhiteList);

			if (bUseJunkMailbox)
			{
				if (MaybeJunkTheMessage(pImapSum))
				{
					imapSumList.RemoveAt(pos);
					bDidJunk = true;
				}
			}
		}
	}

	return hResult;
}

//
//	CImapChecker::ProcessMessageForJunk()
//
//	If conditions are right, assign a junk score to the message specified by pImapSum.
//
void CImapChecker::ProcessMessageForJunk(CImapSum *pImapSum, CObArray *poaABHashes, bool bUsingWhiteList)
{
	if (!pImapSum)
	{
		return;
	}

	if (!pImapSum->m_bManuallyJunked)
	{
		// If this wasn't manually junked score it now.  Even with UIDPLUS
		// We need to score regardless of what mailbox this is because this
		// check might be happening on a different machine than the original
		// junk handling happened on.  (For example, mail was checked on another
		// machine and declared junk so when we check the junk mailbox on this
		// machine we want to see some kind of score (even if it isn't the same
		// score as on the original machine).)
		if (bUsingWhiteList && CJunkMail::IsWhitelisted(pImapSum, poaABHashes))
		{
			// Message sender matches address book entry: not junk.
			pImapSum->SetJunkScore(0);
			pImapSum->SetJunkPluginID(0);
		}
		else
		{
			CTranslatorManager	*pManager = ((CEudoraApp*)AfxGetApp())->GetTranslators();
			if (pManager)
			{
				// Pass the summary through the junk translators.
				pManager->CallJunkTranslators(pImapSum, EMSFJUNK_SCORE_ON_ARRIVAL, poaABHashes);
			}
		}
	}
}

//
//	CImapChecker::MaybeJunkTheMessage()
//
//	If conditions are correct, junk the message specified by pImapSum.
//	Returns true if the message was junked, false otherwise.
//
bool CImapChecker::MaybeJunkTheMessage(CImapSum *pImapSum)
{
	if (!pImapSum || !m_pTocDoc)
	{
		return false;
	}

	bool		 bDidJunk = false;

	// Junk the message if appropriate.  Although we always score, we only junk if this is the Inbox.
	if (m_bIsInbox && !pImapSum->m_bManuallyJunked && !(pImapSum->m_Imflags & IMFLAGS_DELETED) &&
		UsingFullFeatureSet() &&
		GetIniShort(IDS_INI_USE_JUNK_MAILBOX) &&
		(pImapSum->GetJunkScore() >= (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK)))
	{
		// Junk the message.
		m_pTocDoc->ImapChangeMsgsJunkStatus(pImapSum, true);
		bDidJunk = true;
	}
	else if (m_bIsJunk && (pImapSum->GetJunkScore() == 0))
	{
		// Don't want junk showing up in junk mailbox with a score of 0.
		pImapSum->SetJunkScore((unsigned char)GetIniShort(IDS_INI_MANUAL_JUNK_SCORE));
		pImapSum->SetJunkPluginID(0);
	}

	return bDidJunk;
}

//
//	CImapChecker::DoPostFiltering()
//
//	This is only called on the Inbox so we need not doing any such checking here.
//
void CImapChecker::DoPostFiltering(CSumList &sumlistNewSums)
{
	ASSERT(IsMainThreadMT());

	CImapFilterActions	 ifaFilterActions;
	CString				 strTransUIDs;

	if (ifaFilterActions.StartFiltering())
	{
		// Do the actual filtering.
		ifaFilterActions.DoFiltering(m_pTocDoc,
									 sumlistNewSums,
									 &strTransUIDs,
									 TRUE/*bDoSameServerXfers*/,
									 TRUE/*bDoIncoming*/,
									 FALSE/*bDoManual*/);

		// If we transferred messages, we need to do an expunge.
		if (ifaFilterActions.m_bPostFilterXferred)
		{
			if (m_pImapConnection && m_pTocDoc)
			{
				// Note: This is currently passing in the entire summary list,
				// not just the list of summaries to be expunged.  We really need
				// to get back from DoPostFiltering() a list of which summaries
				// we actually transferred and thus need to be expunged.
				DoPostFilterExpunge(strTransUIDs);
			}
		}

		// Do terminating things.
		ifaFilterActions.EndFiltering();
	}
}

//
//	CImapChecker::DoPostFilterExpunge()
//
//	Performs an expunge on the summaries in pSumList and removes those summaries
//	from the TOC.
//
//	Parameters:
//		pPListTransUIDs [in] - List of UIDs of transferred messages.
//
void CImapChecker::DoPostFilterExpunge(CString &strUIDList)
{
	if (!m_pTocDoc || m_pTocDoc->m_pImapMailbox || strUIDList.IsEmpty())
	{
		return;
	}

	CUidMap	 mapUIDsActuallyRemoved;
	BOOL	 bResult = SUCCEEDED(m_pTocDoc->m_pImapMailbox->UIDExpunge(strUIDList, mapUIDsActuallyRemoved));

	// Remove the summaries corresponding to the UID's that were actually expunged.
	if (bResult)
	{
		int	 iHighlightIndex = 0;
	
		m_pTocDoc->ImapRemoveListedTocs(&mapUIDsActuallyRemoved,
										iHighlightIndex,
										TRUE/*CloseWindow*/,
										TRUE/*SetViewSelection*/,
										FALSE/*bJustSetFlag*/);
	}
}

//
//	CImapChecker::RedrawTOC()
// 
//	Turn drawing back on for the specified TOC and make sure everything is redrawn.
//
void CImapChecker::RedrawTOC(CTocView *pView)
{
	if (!pView)
	{
		return;
	}

	CTocFrame	*pTocFrame = (CTocFrame*)pView->GetParentFrame();

	if (pTocFrame)
	{
		// If this is the top window, do focus stuff - ouch.
		CWnd		*pFocusWnd = pView->GetFocus();
		if (pFocusWnd)
		{
			// If the message was not downloaded, do this:
			//
			// Notify the frame's Preview logic that the
			// message it was previewing (or was about to
			// preview) has been invalidated.
			//
			
			// The code here previously checked to see if "the" previewable summary
			// IsNotIMAPDownloadedAtAll. However now there could be multiple
			// previewable summaries. It seems like it should be ok to just always
			// tell the preview code to forget its cached summaries and refresh.
			//
			// If this proves flickery or otherwise undesirable we may need to add
			// code here to loop through each previewable summary, check
			// IsNotIMAPDownloadedAtAll and only call InvalidateCachedPreviewSummary
			// if necessary.
			pTocFrame->InvalidateCachedPreviewSummary(NULL, true);
		}
	}

	// Redraw the complete TOC:
	pView->Invalidate();
	pView->m_SumListBox.SetRedraw(TRUE);
}

//
//	CImapChecker::SetNotificationFlags()
// 
// Given that we got new mail during a mail check, update the
// passed-in boolean references based on INI settings.
// This is currently called from pop.cpp AFTER all POP accounts have been checked 
// for new mail.
//
void CImapChecker::SetNotificationFlags()
{
	unsigned int		 uiFlags = 0;

	// Set new mail icon regardless of options:
	//
	if (m_bGotNewMail)
	{
		if (AfxGetMainWnd()->IsIconic() || IsVersion4())
		{
			((CMainFrame*)AfxGetMainWnd())->SetIcon(TRUE, m_ulNewMsgs);
		}
	}

	if (m_pNewMailNotifier)
	{
		if (!m_bGotNewMail)
		{
			// No new mail.
			if (!(m_ulBitFlags & kSilentCheckBit))
			{
				if (GetIniShort(IDS_INI_ALERT))
				{
					uiFlags |= CMF_NOTIFY_NOMAIL;
				}
			}
		}
		else
		{
			// We got new mail.
			m_pNewMailNotifier->SetNotifyNewMail(true);

			// Do we alert?
			if (GetIniShort(IDS_INI_ALERT))
			{
				uiFlags |= CMF_ALERT_NEWMAIL;
			}

			// Play sound?
			if (GetIniShort(IDS_INI_SOUND))
			{
				uiFlags |= CMF_PLAY_SOUND;
			}

			uiFlags |= CMF_RECEIVED_MAIL;
		}

		m_pNewMailNotifier->AddNotifications(uiFlags);
	}
}


//
// Data access
//

//
//	CImapChecker::SetNewMailNotifier()
//
//	Handle the ref-counting for the notifier object.
//
void CImapChecker::SetNewMailNotifier(QCPOPNotifier *pNotifier)
{
	if (m_pNewMailNotifier != pNotifier)
	{
		if (m_pNewMailNotifier)
		{
			m_pNewMailNotifier->Decrement();
		}
		m_pNewMailNotifier = pNotifier;
		m_pNewMailNotifier->Increment();
	}
}


//
// Error callback
//

//
//	CImapChecker::ErrorCallback()
//
// If the "error type" is not one of CTaskInfo's registered types,
// ignore it and send a registered type:
//
void CImapChecker::ErrorCallback(LPCSTR szStr, UINT iErrorType)
{
	if (!szStr)
	{
		return;
	}

	TaskErrorType	 terr = (TaskErrorType)iErrorType;

	if (terr != TERR_WINSOCK)
	{
		terr = TERR_MSG;
	}

	if (m_pTaskInfo)
	{
		m_pTaskInfo->CreateError(szStr, terr);
	}
}


//
//	Unused - Should it be?
//

//
//	This is currently completely unused and without this function m_bStopRequested
//	will never be set to TRUE and thus is also useless.  Figure out if we should be
//	using this, otherwise remove it. -dwiggins
//
void CImapChecker::RequestThreadStop()
{
	m_bStopRequested = TRUE;

	if (m_pImapConnection)
	{
		m_pImapConnection->RequestStop();
	}
}

#endif // IMAP4
