// ImapChecker.cpp - Does either background or foreground mail checking.
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

#include "imapopt.h"
#include "ImapSum.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imaputil.h"
#include "imapfol.h"
#include "ImapTypes.h"
#include "ImapMailMgr.h"
#include "ImapDownload.h"
#include "ImapPreFiltersd.h"
#include "ImapPreFilterActs.h"
#include "ImapFiltersd.h"
#include "ImapChecker.h"
#include "ImapConnectionMgr.h"
#include "ImapGuiUtils.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// Internals!!
//
static void DoMergeNewTocs (void *pv, bool bLastTime);
static void DummyPostProc (void *pv, bool bLastTime);



//========== Implementation ========================//
//

// NOTE that pNewMailNotifier can be NULL!!!
//
CImapChecker::CImapChecker(LPCSTR pszPersona, LPCSTR pImapName, unsigned long ulCheckBitFlags) :
				 	m_szPersona (pszPersona), 
					m_szImapName (pImapName),
					m_ulBitFlags (ulCheckBitFlags)
					
{
	// No connection yet.
	//
	m_pImap = NULL;

	m_pErrorCallback = NULL;

	// We use this flag a number of times.
	//
	m_bIsInbox				= ::IsInbox (m_szImapName);

	// Initialize these:
	m_bStopRequested		= FALSE;
	m_bIsReadOnly			= FALSE;

	// 
	m_OldNumberOfMessages	= m_NewNumberOfMessages	= 0;

	// Highest UID seen.
	m_OldUIDHighest			= m_NewUIDHighest = 0;

	// UIDVALIDITY management.
	m_OldUidValidity		= m_NewUidValidity = 0;

	// Don't know this yet.
	m_ServerUidHighest		= 0;

	// Did we get new mail???
	//
	m_bGotNewMail			= FALSE;

	// 
	m_pPreFilterActions		= NULL;

	// If to display progress.
	//
	m_bSilent				= FALSE;
	m_bInMainTreadProgress	= FALSE;

	m_bDoExpunge			= FALSE;
	m_bDoResync				= FALSE;

	m_ulNewMsgs				= 0;

	// Internal Resync attributes:
	//
	m_ulMaxUidToResync					= 0;
	m_bDownloadedOnlyDuringResync		= TRUE;
	m_bRemoveAllLocalSumsBeforeMerging	= FALSE;

	// Pointer to the actual DoWork() function.
	// 
	m_WorkerFunctionType			= WORKERFUNCTION_TYPE_BADTYPE;

	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	m_pTaskInfo->SetPersona(m_szPersona);
	m_pTaskInfo->SetTitle (CRString (IDS_POP_LOOKING_FOR_MAIL));

	m_pNewMailNotifier = NULL;

	// Construct our error callback:
	//
	m_pErrorCallback = makeCallback( (QCNetworkSettings::ErrorCallback *)0, *this, &CImapChecker::ErrorCallback);
}


// virtual
CImapChecker::~CImapChecker()
{

	TRACE("In CImapChecker's destructor for %s\n", m_szImapName);

	if (m_pPreFilterActions)
		delete m_pPreFilterActions;

	// If any is left over.
	//
	m_CurrentUidMap.DeleteAll();
	m_NewUidMap.DeleteAll();
	m_ModifiedUidMap.DeleteAll();
	m_NewOldUidMap.DeleteAll();

	// Cleanup internal summary list.
	//
	m_NewImapSumList.DeleteAll();
	m_NewOldImapSumList.DeleteAll();

	// Release the IMAp agent if we have one.
	//
	if (m_pImap)
	{
		GetImapConnectionMgr()->Release (m_pImap);
	}

	// we MUST have a task key. Go remove our task from the global task queue.
	//
	GetImapMailMgr()->RemoveTask (m_szTaskKey);

	// We MUST do this before we exit!!!
	//
	if (m_pNewMailNotifier)
		m_pNewMailNotifier->Decrement();
}




// GrabState [PUBLIC]
//
// Get current state of the local cache from the CImapMailbox.
// When we're done, we must let the CImapMailbox update its state from
// what new things we've learned.
//
// NOTE: This MUST be called in the main thread.
//
HRESULT CImapChecker::DoMainThreadInitializations (CTocDoc* pTocDoc)
{
	ASSERT ( IsMainThreadMT() );

	HRESULT hResult = S_OK;

	if (!pTocDoc)
		return E_INVALIDARG;

	CImapMailbox* pImapMailbox = pTocDoc->m_pImapMailbox;

	if (!pImapMailbox)
		return E_FAIL;

	//Copy some stuff from the CImapMailbox.
	//
	m_OldNumberOfMessages	= pImapMailbox->GetNumberOfMessages();
	m_OldUIDHighest			= pImapMailbox->GetUIDHighest();
	m_OldUidValidity		= pImapMailbox->GetUidvalidity();

	m_AccountID				= pImapMailbox->GetAccountID();

	// These may be updated.
	//
	m_NewNumberOfMessages	= m_OldNumberOfMessages;
	m_NewUIDHighest			= m_OldUIDHighest;
	m_NewUidValidity		= m_OldUidValidity;

	// Grab the full path to the real MBX file.
	//
	GetMbxFilePath (pImapMailbox->GetDirname(), m_szRealMbxFilePath);

	// Truncate the temporary MBX file and save it's name.
	//
	GetTmpMbxFilePath (pImapMailbox->GetDirname(), m_szTmpMbxFilePath);

	CreateLocalFile (m_szTmpMbxFilePath, TRUE);

	m_bDoingMinimalDownload = pImapMailbox->GetSettingsShort (IDS_INI_IMAP_MINDNLOAD);

	// Obtain a connection to the IMAP server from the connections manager.
	// This MUST be done in the main thread. We don't open the
	// connection here but the CIMAP must be setup in the main thread.
	//
	if (!m_pImap)
		hResult = AcquireNetworkConnection ();

	if (!m_pImap)
	{
		// $$$$$ SHOULD put up error message here!!
		//
		return E_FAIL;
	}

	// If the CIMAP is not conencted, make sure it has a password and login.
	//
	// If the connection is not conencted, we need to go get new password/login.
	//
	if ( !m_pImap->IsConnected(m_pErrorCallback) )
	{
		hResult = pImapMailbox->GetLogin();
	}

	// If this is inbox, prepare for filtering.
	//
	if ( SUCCEEDED (hResult) && IsInbox () )
	{
		CreatePreFilterActions (pTocDoc);
	}
		
	return hResult;
}




// 
// This MUST be called in the main thread when the CImapChacher is being created.
//
BOOL CImapChecker::CreatePreFilterActions (CTocDoc* pSrcToc)
{
	ASSERT ( IsMainThreadMT() );

	BOOL bResult = FALSE;

	if (m_pPreFilterActions)
		delete m_pPreFilterActions;

	m_pPreFilterActions = new CPreFilterActions;

	if (m_pPreFilterActions)
	{
		m_pPreFilterActions->StartFiltering(pSrcToc);

		// Share our task info object:
		m_pPreFilterActions->SetTaskInfoObject (m_pTaskInfo);

		bResult = TRUE;
	}

	return bResult;
}




// DoWork [PUBLIC]
//
// Called by the thread manager to do the first stage of the fetching.
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
	// Initialize task info stuff:
	//
	CString buf;

	if ( IsInbox() )
	{
		buf.Format ( CRString(IDS_IMAP_UPDATING_MAILBOX), GetTocName() );

		// If a manual re-sync, such as after an expunge, don't wait for idle.
		//
		if (WORKERFUNCTION_TYPE_RESYNC == m_WorkerFunctionType)
		{
			m_pTaskInfo->IgnoreIdle();
		}
	}
	else
	{
		buf.Format ( CRString(IDS_IMAP_RESYNCING_MAILBOX), GetTocName() );

		// In this case, don't wait for idle.
		//
		m_pTaskInfo->IgnoreIdle();
	}

	m_pTaskInfo->SetMainText(buf);

	// Set this in case it doesn't get set later:
	//
	m_pTaskInfo->SetPostProcFn("Cleanup", ::DummyPostProc, this);


	HRESULT hResult = DoCheckMailMT ();

	// If we failed, set a dummy post proc so we can clean up.
	//
	if ( !SUCCEEDED (hResult) )
	{
		TCHAR ErrorBuf [1024];
		CString szErr = "Operation Failed: ";

		*ErrorBuf = 0;

		if (m_pImap)
			m_pImap->GetLastImapError (ErrorBuf, 1020);

		if (!*ErrorBuf)
			szErr += ErrorBuf;

//		m_pTaskInfo->SetMainText(ErrorBuf);

		m_pTaskInfo->CreateError (szErr, TERR_MSG);

		m_pTaskInfo->SetPostProcFn("Cleanup", ::DummyPostProc, this);
	}

	//Set the thread state to done 
	m_pTaskInfo->SetState(TSS_COMPLETE);

	// Ste this just before exiting.
	//
	m_pTaskInfo->SetPostProcessing(TRUE);

	return hResult;
}


void CImapChecker::SetCheckMailWorkFunction ()
{
	m_WorkerFunctionType = WORKERFUNCTION_TYPE_CHECKMAIL;
}


void CImapChecker::SetResyncWorkFunction (BOOL bDownloadedOnly, unsigned long ulMaxUid)
{
	m_WorkerFunctionType = WORKERFUNCTION_TYPE_RESYNC;

	// Setup parameters.
	//
	m_bDownloadedOnlyDuringResync	= bDownloadedOnly;
	m_ulMaxUidToResync				= ulMaxUid;
}





// DoCheckMail [PRIVATE]
//
// Internal entry point to doing the background check mail.
//
HRESULT CImapChecker::DoCheckMailMT ()
{
	HRESULT hResult = S_OK;

	// Must have a valid persona:
	//
	if ( m_szPersona.IsEmpty() )
		return E_FAIL;

	// Must have these:
	//
	if (!m_pTaskInfo)
		return E_FAIL;


	// Don't make noise because we'er in the background.
	//

	// STAGE 0:
	// Setup. (silently).
	// 
	hResult = SetupConnectionMT ();

	if ( !SUCCEEDED (hResult) )
	{
		return hResult;
	}

	// Get things like new number of messages, etc, from the server.
	//
	hResult = GetNewMboxState ();

	if ( !SUCCEEDED (hResult) )
	{
		return hResult;
	}

	// STAGE 1.
	// Get the list of new UID's in the remote mailbox..
	//
	m_NewUidMap.DeleteAll();

	hResult = FetchNewUidsMT ();

	if ( !SUCCEEDED (hResult) )
	{
		return hResult;
	}

	// THis is the number of new messages in the remote mailbox:
	// At the end of this process, "m_ulNewMsgs" will contain the number of new messages
	// filtered on the server and actually (minimally or fully) downloaded.
	//
	m_ulNewMsgs = m_NewUidMap.GetCount();

	// STAGE 2
	// Do same-server filtering (only for INBOX).
	//
	DoPrefilterMT ();

	// "m_ulNewMsgs" must now contain number of msgs xferred on the server and will no longer
	// be in the remote mailbox.
	// Could be zero.
	//
	m_ulNewMsgs -= m_NewUidMap.GetCount();

	// Do we need to do an expunge before we re-sync??
	//
	if (m_bDoExpunge)
	{
		m_pImap->Expunge(m_pErrorCallback);

		m_bDoExpunge = FALSE;
	}

	// we may have to do resynchronizations to determine which messages we
	// download or re-download.
	// This goes out and fetch a new UID list (up to m_ulMaxUidToResync) and merges it
	// with "m_CurrentUidMap". The results of this process are:
	//
	// - m_ModifiedUidMap: List of previously downloaded UID's, wtill on the server, but flags 
	// have changed.
	//
	// - m_CurrentUidMap: Any messages no longer on the server are left in this list. 
	//	 They must be removed from out local cache.
	//
	// - m_NewOldUidMap: Messages that are not new to us but, for some reason,
	//   are in our local cache. We just re-download these without performing any post
	//	 filtering.
	//
	// - m_NewUid<ap: These are the new messages that may have to be post filtered.
	//
	if (m_bDoResync)
	{
		hResult = DoResyncMailboxMT ();

		if ( !SUCCEEDED (hResult) )
		{
			return hResult;
		}

		m_bDoResync = FALSE;
	}
	else
	{
		// If we're not re-syncing, we must remove UID's from current list so we don't
		// think it contains messages no longer on the server.
		// 
		m_CurrentUidMap.DeleteAll();
	}


	// STAGE :
	// Download new messages to temporary TOC and MBX.
	// m_NewUidMap will now contain UID's of messages that must be retrieved.
	//

	// Note: Don't kill the whole process if downloading failed. If
	// we downloaded any messages at all, merge those. We'd get the rest next time.
	// 
	DownloadNewMessagesToTmpTocMT ();

	// STAGE 4:
	// Last stage: Merge temporary TOC and MBX. Do this in the foreground.
	//	
	if (m_pTaskInfo)
	{
		m_pTaskInfo->SetMainText(CRString (IDS_IMAP_WAITING_FOR_POSTPROCESS));

		m_pTaskInfo->SetPostProcFn("Update Mailbox", ::DoMergeNewTocs, this);
	}

	return S_OK;
}





//////////////////////////////////////////////////////////////////////////////
// SetupConnectionMT [PRIVATE]
//
// Establish initial conditions and setup network for checking mail.
//
// NOTE: This can be done in the main thread!!!
//
//////////////////////////////////////////////////////////////////////////////
HRESULT CImapChecker::SetupConnectionMT ()
{
	HRESULT			hResult = S_OK;
	CString			buf;			// Scratch buffer.

	// Reset this.
	m_bStopRequested = FALSE;

	// Our current vallues of the following must have already been set 
	// before the background task begun:
	//
	//  m_OldNumberOfMessages	- Obtained from the TocDoc->m_pImapMailbox.
	//	m_OldUIDHighest			- Obtained from the TocDoc->m_pImapMailbox.
	//	m_OldUidValidity		- Obtained from the TocDoc->m_pImapMailbox..
	//
	
#if 0 //

	// If this is the main thread and a progress bar's already up, do this.
	if ( m_bInMainTreadProgress )
	{
		PushProgress();

		buf.Format (CRString(IDS_IMAP_UPDATING_MAILBOX), GetTocName());

		MainProgress(buf);
	}
#endif 

	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Open the connection if the connection is not already open,
	// If we dropped the connection for whatever reason, try to re-connect.
	//
	if (!m_pImap->IsSelected(m_pErrorCallback))
	{
		hResult = m_pImap->OpenMailbox (m_szImapName, TRUE, m_pErrorCallback);

		if (! SUCCEEDED (hResult) )
		{
			return hResult;
		}
	}

#if 0 // 
	// Close progress.
	if ( m_bInMainTreadProgress )
	{
		PopProgress();
	}
#endif

	// If we get here:
	return hResult;
}





// GetNewMboxState [PRIVATE]
//
// Get m_NewNumberOfMessages and m_ServerUidHighest, etc, in preparation for the 
// real jobs.
//
HRESULT CImapChecker::GetNewMboxState ()
{
	HRESULT hResult = S_OK;

	// Must have one of these, otherwise we're screwed.
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Make sure we're connected.
	//
	if (!m_pImap->IsSelected(m_pErrorCallback))
	{
		hResult = m_pImap->OpenMailbox (m_szImapName, TRUE, m_pErrorCallback);

		if (! SUCCEEDED (hResult) )
		{
			return hResult;
		}
	}
	else
	{
		// Send a Noop so we get any pending EXISTS messages.
		m_pImap->Noop (m_pErrorCallback);
	}

	// UidValidity may have changed!
	//
	unsigned long NewUV = 0;

	hResult = m_pImap->GetUidValidity(NewUV, m_pErrorCallback);

	if ( !SUCCEEDED (hResult) )
		return hResult;

	m_NewUidValidity = NewUV;
	
	//
	// How many messages do we now think we have? 
	//
	hResult = m_pImap->GetMessageCount(m_NewNumberOfMessages, m_pErrorCallback);

	if ( !SUCCEEDED (hResult) )
		return hResult;

	// And the server's highest.
	hResult = m_pImap->UIDFetchLastUid (m_ServerUidHighest, m_pErrorCallback);

	if ( !SUCCEEDED (hResult) )
		return hResult;

	// Try to determine if our cache became out of sync without refreshing it last time.
	// 
	if (m_OldNumberOfMessages  != (unsigned long) m_CurrentUidMap.GetCount() )
	{
		ForceResync();
	}

	return hResult;
}





///////////////////////////////////////////////////////////////////////////////////////////
// FetchNewUidMapMT [PRIVATE]
// 
// Use m_OldUIDHighest to get messages with a UID higher than it and filter those messages.
//
// On OUTPUT: Set the m_Expunged flag if filtering caused an EXPUNGE of the mailbox.
//
// Assume the mailbox is connected. 
// 
// NOTE: This may still cause a full resync. If it did, convey this info back to
// caller by seeting "bWasResynced" to TRUE.
//
////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CImapChecker::FetchNewUidsMT ()
{
	HRESULT hResult = S_OK;
	unsigned long NextUid = 0; 

	// Make sure this is empty:
	//
	m_NewUidMap.DeleteAll();

	// Must have one of these, otherwise we're screwed.
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// The current uid list (from the TOC) would have been calculated and set
	// during the main thread, before starting this procedure.
	//
	// Get the highest one we have locally.
	//
	unsigned long LocalUidHighest	= m_CurrentUidMap.GetHighestUid();


	// If there are messages in the mailbox, m_ServerUidHighest MUST be
	// non-zero.
	//
	if ( (m_NewNumberOfMessages > 0) && (m_ServerUidHighest == 0) )
	{
		m_pImap->Close();

		hResult = m_pImap->OpenMailbox (m_szImapName, TRUE, m_pErrorCallback);

		if (! SUCCEEDED (hResult) )
		{
			return hResult;
		}

		// Get these again.
		//
		hResult = m_pImap->GetMessageCount(m_NewNumberOfMessages, m_pErrorCallback);

		if ( !SUCCEEDED (hResult) )
			return hResult;

		// UIDVALIDITY
		unsigned long NewUV = 0;

		hResult = m_pImap->GetUidValidity(NewUV, m_pErrorCallback);

		if ( !SUCCEEDED (hResult) )
			return hResult;

		m_NewUidValidity = NewUV;

		// Highest UID.
		hResult = m_pImap->UIDFetchLastUid(m_ServerUidHighest, m_pErrorCallback);

		if ( !SUCCEEDED (hResult) )
			return hResult;
	}

	// If no messages in remote mailbox, just delete our local cached.
	if (m_NewNumberOfMessages == 0)
	{
		// m_CurrentUidMap is no longer valid. Flush it and redownload all.
		//
		m_CurrentUidMap.DeleteAll();

		// we must clear our local cache and redownload everything.
		//
		m_bRemoveAllLocalSumsBeforeMerging = TRUE;

		// Don't change our concept of highest UID, unless the UIDVALIDITY changed.
		//
		if (m_NewUidValidity != m_OldUidValidity)
		{
			// We must refetch everything next time.
			//
			m_NewUIDHighest = m_OldUIDHighest = 0;
		}
		else
		{
			m_NewUIDHighest = m_OldUIDHighest;
		}
	
		// Set m_ulMaxUidToResync to the highest server uid so we redownload everything
		// during the re-sync phase.
		//
		m_ulMaxUidToResync = m_ServerUidHighest;

		return hResult;
	}


	// If the UIDVALIDITY has changed, we need to just re-download every message.
	// In that case, filtering doesn't make sense because UID's are no longer valid.
	// Set a flag indicating tha, during the post processing, we need to remove all
	// local summaries before merging the new ones.
	// 
	if (m_NewUidValidity != m_OldUidValidity)
	{
		// m_CurrentUidMap is no longer valid. Flush it and redownload all.
		//
		m_CurrentUidMap.DeleteAll();

		// we must clear our local cache and redownload everything.
		//
		m_bRemoveAllLocalSumsBeforeMerging = TRUE;

		// However, set new UIDHighest to the current server's highest UID so we treat
		// all messages as if they were previously seen,
		//
		m_NewUIDHighest = m_ServerUidHighest;

		// Also, old cached UID highest values are no longer valid.
		//
		m_OldUIDHighest = m_ServerUidHighest;
	
		// Set m_ulMaxUidToResync to the highest server uid so we redownload everything
		// during the re-sync phase.
		//
		m_ulMaxUidToResync = m_ServerUidHighest;

		return hResult;
	}


	// If this is a manual resync and we're re-syncing only previously downloaded message,
	// just set flags and limits and return.
	if (WORKERFUNCTION_TYPE_RESYNC == m_WorkerFunctionType)
	{
		if (m_bDownloadedOnlyDuringResync)
		{
			m_ulMaxUidToResync	= m_OldUIDHighest;

			// Don't update UIDHighest in the CImapMailbox because we're not
			// fetching new mail.
			//
			m_NewUIDHighest		= m_OldUIDHighest;

			// Don't continue.
			//
			return hResult;
		}
		else 
		{
			// Fetch new mail AND update the CImapMailbox's UIDHighest.
			//
			m_ulMaxUidToResync = m_ServerUidHighest;

			m_NewUIDHighest = m_ServerUidHighest;
		}
	}


	// If we get here, we're downloading new messages..

	// Download the next after "NextUid":
	//
	NextUid = max (LocalUidHighest, m_OldUIDHighest);

	// We may have removed messages at the tail end of the mailbox:
	//
	NextUid = min(m_ServerUidHighest, NextUid);

	// Did user abort??
	//
	if ( m_bStopRequested )
	{
		return HRESULT_MAKE_CANCEL;
	}

	// Now, Fetch only if server has higher UID.
	//
	if ( (m_ServerUidHighest > 0) && (m_ServerUidHighest > NextUid) )
	{
		CString szSeq;

		szSeq.Format ("%lu:%lu", NextUid + 1, m_ServerUidHighest);

		hResult = m_pImap->FetchFlags (szSeq, &m_NewUidMap, m_pErrorCallback);
	}

	// If we're re-syncing, resync up to this:
	//
	m_ulMaxUidToResync = NextUid;

	return hResult;
}




HRESULT CImapChecker::DoPrefilterMT ()
{
	// Are we really filtering? If not INBOX, we're not.
	//
	if (!m_pPreFilterActions)
	{
		return S_OK;
	}

	m_pPreFilterActions->DoXferPreFilteringMT (m_NewUidMap);

	if ( m_pPreFilterActions->MessagesWereMoved() )
	{
		ForceExpunge ();
		ForceResync();
	}

	return S_OK;
}



//
// Call CImapMailbox::DownloadRemainingMessages() to do the real work.
//
// If any content is downloaded, do it to the temporary MBX file.
//

HRESULT CImapChecker::DownloadNewMessagesToTmpTocMT ()
{
	HRESULT hResult = S_OK;

	// First, download messages we've never seen.
	//
	m_pTaskInfo->SetMainText(CRString(IDS_IMAP_FETCHING_NEWMSGS));

	hResult = DownloadRemainingMessagesMT (m_NewUidMap, &m_NewImapSumList, TRUE, m_pTaskInfo);

	// "DownloadRemainingMessagesMT" may have been interrupted.
	// The actual number of msgs downloaded is the count of "m_NewImapSumList"
	//
	m_ulNewMsgs += m_NewImapSumList.GetCount();

	if ( !SUCCEEDED (hResult) )
	{
		if (m_pTaskInfo)
			m_pTaskInfo->CreateError ("Some message headers may not have been downloaded.", TERR_MSG);
		
		return hResult;
	}

	// Note: We should be able to display these before re do the re-download!!!

	// Next, fetch messages that need re-download.
	//
	m_pTaskInfo->SetMainText(CRString(IDS_IMAP_REDOWNLOADING_MSGS));

	hResult = DownloadRemainingMessagesMT (m_NewOldUidMap, &m_NewOldImapSumList, TRUE, m_pTaskInfo);

	if ( !SUCCEEDED (hResult) )
	{
		if (m_pTaskInfo)
			m_pTaskInfo->CreateError ("Some message headers may not have been downloaded.", TERR_MSG);
	}

	return hResult;
}


// DoMergeNewTocs [PUBLIC]
//
// Do the postprocessing in the main thread.
//
// Here, we must check to see if the TOC we're dealing with is still open!!
//
HRESULT CImapChecker::DoMergeNewTocs()
{
	HRESULT hResult = S_OK;
	BOOL	bWeCreatedToc = FALSE;

	ASSERT ( IsMainThreadMT() );

	TRACE("Starting DoMergeNewTocs for %s\n", m_szImapName);

	// Try only if loaded first so we know if we loaded it.
	//
	CTocDoc* pTocDoc = GetToc( m_szRealMbxFilePath, NULL, FALSE, TRUE );

	if (!pTocDoc)
	{
		// We're loading it.
		//
		pTocDoc = GetToc( m_szRealMbxFilePath );

		bWeCreatedToc = TRUE;
	}

	if (!pTocDoc)
	{
		TRACE("Exiting DoMergeNewTocs for %s\n", m_szImapName);

		return E_FAIL;
	}

	// Must have an IMAP mailbox object.
	//
	CImapMailbox* pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);

		// Close TOC if we opened it.
		//
		if (bWeCreatedToc)
		{
			pTocDoc->m_bAutoDelete = TRUE;
			pTocDoc->OnCloseDocument();
		}

		TRACE("Exiting DoMergeNewTocs for %s\n", m_szImapName);

		return E_FAIL;
	}

	// Should we put up progress??
	//
	ULONG ulTotLoad = m_NewOldImapSumList.GetCount() + m_NewImapSumList.GetCount();
	BOOL  bWeInProgress = FALSE;

	// Note: this is arbitrary:
	//
	if (ulTotLoad > 10)
	{
		// Put up progress immediately:
		//
		CImmediateProgress Imp;

		CString buf;
		buf.Format ( CRString(IDS_IMAP_DOING_POSTPROCESS), pTocDoc->Name() );
		MainProgress(buf);

		bWeInProgress = TRUE;
	}

	// NOTE!!! Change current persona here but we MUST reset it before we leave!!!
	//
	CString	homie =	g_Personalities.GetCurrent();

	g_Personalities.SetCurrent( m_szPersona );

	// Before we go any further, we need to update and serialize the CImapMailbox's
	// "UIDHighest" to reflect preflitered messages. This way we avoid a possible second
	// filtering ifwe should fail in anyything below.
	//
	if (m_pPreFilterActions)
	{
		// Also update UIDVALIDITY here.
		//
		if (m_NewUidValidity != m_OldUidValidity)
		{
			pImapMailbox->SetUidvalidity (m_NewUidValidity);

			m_NewUidValidity = m_OldUidValidity;
		}

		UpdateUIDHighest (pImapMailbox, m_pPreFilterActions->GetHighestUidSeen() );
	}

	// 
	// See if the mailbox is open:
	//
	CTocView* pView = pTocDoc->GetView ();

	if (pView)
	{
		pView->m_SumListBox.SetRedraw(FALSE);
	}

	// If we need to remove all local summaries before merging, go do that now.
	//
	if (m_bRemoveAllLocalSumsBeforeMerging)
	{
		pTocDoc->ImapRemoveAllSummaries ();
	}

	BOOL bResult = TRUE;

	// If we need to download message content (FOR RE_DOWNLOADED MESSAGES), do that here
	// (until we can do mime in the background).
	//

	HandlePostFetch (m_NewOldImapSumList);


	// If we had to re-downloaded previously downloaded summaries, go add those
	// to the existing CSummary list. Unfortunately, these would be added to END of
	// the list!!
	//

	bResult = pImapMailbox->ImapAppendSumList (pTocDoc, &m_NewOldImapSumList, NULL, TRUE);

	// Now add new messages at the end.
	//
	// "m_PostFilterSumList" gets filled with references to the summaries added. These
	// are the ones we need to apply the "post filtering" on.
	//
	// NOTE: m_PostFilterSumList is a list of real CSummary's!!
	//
	m_PostFilterSumList.RemoveAll();

	if (bResult)
	{
		// Don't set the internal "Got new mail" flag until we're sure we've added
		// new summaries to INBOX.
		//
		BOOL bGotNewMessages = m_NewImapSumList.GetCount() > 0;

		HandlePostFetch (m_NewImapSumList);

		bResult = pImapMailbox->ImapAppendSumList (pTocDoc, &m_NewImapSumList, &m_PostFilterSumList, TRUE);

		// If we need to download message content (FOR NEW MESSAGES), do that here.
		// (until we can do mime in the background).
		//
		if (bResult)
		{
			if (bGotNewMessages)
				m_bGotNewMail = TRUE;
				
			// Perform post filtering here, if it's inbox.
			// 
			if ( IsInbox() )
			{
				// FOR NEW MESSAGES
				//
				DoPostFiltering (pTocDoc, m_PostFilterSumList);
			}
		}
	}


	// Update existing summary attributes.
	// Note: UpdateOldSummaries may remove entries from NewOldUidMap!!!
	//
	if (bResult)
	{
		pImapMailbox->UpdateOldSummaries (pTocDoc, m_ModifiedUidMap);

		// Now, remove stale summaries first.
		if (m_CurrentUidMap.size() > 0 )
		{
			int HighlightIndex;
			pTocDoc->ImapRemoveListedTocs (&m_CurrentUidMap, HighlightIndex, TRUE, TRUE);
		}
	}

	// Make sure to do this!!!
	//
	m_PostFilterSumList.RemoveAll();

	// Save the TOC before we update cache, in case of failure later.
	//
	pTocDoc->SaveModified ();

	// Do the foll. only if we're OK:
	//
	if (bResult)
	{
		// Update local cache status:
		//
		pImapMailbox->SetNumberOfMessages (m_NewNumberOfMessages);

		// New UIDValidity?
		//
		if (m_NewUidValidity != m_OldUidValidity)
			pImapMailbox->SetUidvalidity (m_NewUidValidity);

		// Highest UID seen. Set it in any case.!!
		//
		pImapMailbox->SetUIDHighest (m_NewUIDHighest);
	}

	// Update saved info, whether we succeeded or not.
	//
	pImapMailbox->WriteImapInfo (TRUE);

	// Redraw the TOC.
	//
	if (pView)
	{
		CTocFrame* pTocFrame = (CTocFrame *) pView->GetParentFrame();

		if (pTocFrame)
		{
			// If this is the top window, do focus stuff - ouch.
			//
			CWnd* pFocusWnd = pView->GetFocus();
			if (pFocusWnd)
			{
				// If the message was not downloaded, do this:
				//
				// Notify the frame's Preview logic that the
				// message it was previewing (or was about to
				// preview) has been invalidated.
				//
				CSummary *pSum = pTocDoc->GetPreviewableSummary();
				if ( pSum && IsNotDownloaded(pSum) )
				{
					extern UINT umsgPurgeSummaryCache;
					pTocFrame->SendMessage(umsgPurgeSummaryCache);
				}

//				pView->SetFocus();
			}
		}

		// Redraw the complete TOC:
		pView->Invalidate();
		pView->m_SumListBox.SetRedraw(TRUE);
	}

	// Finally, notify manager if we got new mail.
	//
	SetNotificationFlags ();

	// If a second re-sync is necessary, do tis by posting a message:
	//
#if 0 // Try to remove individual summaries instead of a full resync!!!

	if (pView && m_bDoResync )
	{
		pView->PostMessage (WM_COMMAND, ID_MBOX_IMAP_RESYNC);
	}
#endif

	// Reset persona.
	//
	g_Personalities.SetCurrent( homie );

	// If we're offline, close the connection after us.
	//
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		pImapMailbox->Close();
	}

	// Close TOC if we opened it.
	//
	if (bWeCreatedToc)
	{
		pTocDoc->m_bAutoDelete = TRUE;
		pTocDoc->OnCloseDocument();

		// This is no longer valid.
		//
		pImapMailbox = NULL;
	}
	else
	{
		// If we're still selected, get imapdll to recreate it's message map.
		//
//		pImapMailbox->RecreateMessageMap();
	}


	TRACE("Exiting DoMergeNewTocs for %s\n", m_szImapName);

	if (bWeInProgress)
	{
		CloseProgress();
	}

	return bResult ? S_OK : E_FAIL;
}



// UpdateUIDHighest [PRIVATE]
//
// If UIDVALIDITY has changed, force an update, otherwise check if "val"
// is larger before updating.
//
void CImapChecker::UpdateUIDHighest ( CImapMailbox *pImapMailbox, unsigned long val)
{
	// Validate:
	// Note: "val" can be zero!!!
	//
	if (!pImapMailbox)
		return;

	BOOL bSet = FALSE;

	// IF UIDVALIDITY changed, force a re-set of UIDHighest.
	//
	if (m_NewUidValidity != m_OldUidValidity)
	{
		pImapMailbox->SetUIDHighest (val);

		bSet = TRUE;
	}
	else
	{
		// Set only if new one is higher.
		//
		if (val > pImapMailbox->GetUIDHighest() )
		{
			pImapMailbox->SetUIDHighest (val);

			bSet = TRUE;
		}
	}

	if (bSet)
		pImapMailbox->WriteImapInfo (TRUE);
}





// HandlePostFetch [PRIVATE]
// 
// Since the background stuff just did minimal fetches, we need to do any
// requested on-check fetch in the main thread (until this becomes thread safe).
// Go do that now.
//
HRESULT CImapChecker::HandlePostFetch (CImapSumList& ImapSumList)
{
	// MAIN THREAD
	//
	ASSERT (IsMainThreadMT());

	HRESULT hResult = S_OK;

	if (!m_bDoingMinimalDownload)
	{
		POSITION pos = ImapSumList.GetHeadPosition();
		POSITION next;

		for (next = pos; pos; pos = next)
		{
			CImapSum* pImapSum = (CImapSum *) ImapSumList.GetNext (next);

			if (pImapSum)
			{
				hResult = DownloadSingleMessage (pImapSum, FALSE, FALSE);

				if (!SUCCEEDED (hResult))
					break;
			}
		}
	}

	return hResult;
}




// DoPostFiltering [PRIVATE]
//
// Must be done in the main thread.
//
void CImapChecker::DoPostFiltering (CTocDoc* pTocDoc, CSumList& NewSumList)
{
	ASSERT ( IsMainThreadMT() );

	CObList strDstList, strReportList;

	// Before we do this, merge the destination mailbox summary from the
	// prefiltering phase into FilterActions.
	//
	if (m_pPreFilterActions)
	{
		m_pPreFilterActions->GetResultsLists (strDstList, strReportList);

		// If we actually copied/moved messages, update the fact that we DID get
		// new messages.
		//
		if (strDstList.GetCount() != 0)
		{
			m_bGotNewMail = TRUE;
		}
	}

	// In case we're filtering.
	CImapFilterActions FilterActions;

	if (FilterActions.ImapStartFiltering ())
	{

		// Add info from prefiltering to FilterActions's internal info.
		// Note: InitializeFromBackgroundFilter() will clean strDstList and strReportList.
		//
		FilterActions.InitializeFromBackgroundFilter (strDstList, strReportList);

		FilterActions.DoPostFiltering (pTocDoc, NewSumList, FALSE, TRUE, FALSE);

#if 0 // Try do remove individual summaries instead os a full resync!!
		// IF we xferred stuff, we need to do an expunge:
		//
		if (FilterActions.m_bPostFilterXferred)
		{
			if (m_pImap)
			{
				m_pImap->Expunge();

				// Flag that a second re-sync is necessary.
				m_bDoResync = TRUE;
			}
		}
#endif // JOK -- 

		// Do terminating things.
		FilterActions.ImapEndFiltering ();
	}
}



// DoResyncMailboxMT [PRIVATE]
//
// Perform all necessary resynchronizations of summaries.
//
// We've got uid's for messages we haven't yet seen. However,
// we may need to re-download old messages in the foll. cases:
// 1. The UIDVALIDITY changed.
// 2. The TOC was rebuilt, in which case we may have lost some summaries.
// 
// The results of this process are:
// - m_ModifiedUidMap: List of previously downloaded UID's but which are still 
//	 on the server and we also have locally, Their flags may have changed.
//
// - m_CurrentUidMap: Any messages no longer on the server are left in this list. 
//	 They must be removed from out local cache.
//
// - m_NewOldUidMap: Messages that are not new to us but, for some reason,
//   are in our local cache. We just re-download these without performing any post
//	 filtering.
//
// - m_NewUidMap: These are the new messages that may have to be post filtered.
//

HRESULT CImapChecker::DoResyncMailboxMT ()
{
	HRESULT		hResult = S_OK;
	BOOL		bWasInProgress = FALSE;

	// Must have acquired a connection.
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Clear internal lists:
	//
	m_ModifiedUidMap.DeleteAll();
	m_NewOldUidMap.DeleteAll();

	// This must have been set.
	//
	if (m_ulMaxUidToResync == 0 || m_NewNumberOfMessages == 0)
	{
		return S_OK;
	}

	// We can put up a progress window here?
	CString buf;
	
	buf.Format (CRString(IDS_IMAP_RESYNCING_MAILBOX), GetTocName());

	if ( ::IsMainThreadMT() )
	{
		if (!m_bSilent )
		{
			if (InProgress)
			{
				bWasInProgress = TRUE;
				PushProgress();
			}

			MainProgress(buf);
		}
	}
	else if( m_pTaskInfo )
	{
		m_pTaskInfo->SetMainText(buf);
	}

	// Fetch uid's into m_NewOldUidMap.
	//

	CString szSeq;

	szSeq.Format ("1:%lu", m_ulMaxUidToResync);

	hResult = m_pImap->FetchFlags (szSeq, &m_NewOldUidMap, m_pErrorCallback);

	if (m_bStopRequested || !SUCCEEDED (hResult) )
	{
		m_NewOldUidMap.DeleteAll();

		if ( ::IsMainThreadMT() && !m_bSilent )
		{
			if (bWasInProgress)
				PopProgress();
			else
				CloseProgress();
		}

		if (m_bStopRequested)
		{
			return HRESULT_MAKE_CANCEL;
		}
		else
			return E_FAIL;
	}

	// Before we do the merge, we must make sure we don't have UID's in "m_NewOldUidMap"
	// that are also in m_NewUidMap, otherwise we may download the same message twice.
	//
	RemoveSubUidMapMT (m_NewOldUidMap, m_NewUidMap);


	// Merge the two lists.
	// When done, the two lists will be in the following states:
	// 1. m_CurrentUidMap contains only uid's no longer on the server.
	// 2. m_NewOldUidMap contains only  uid's of messages that are not in our local cache,
	//	  for whatever reason.
	// 3. m_ModifiedUidMap contains uid's of messages in our cache that are still on the server, but
	//	  m_Imflags may have new values.

	MergeUidMapsMT (m_CurrentUidMap, m_NewOldUidMap, m_ModifiedUidMap);

	// Cleanup.

	if ( ::IsMainThreadMT() && !m_bSilent )
	{
		if (bWasInProgress)
			PopProgress();
		else
			CloseProgress();
	}


	return S_OK;
}




/////////////////////////////////////////////////////////////////////
// DownloadRemainingMessagesMT [PRIVATE]
//
// Called after prefiltering to download the remaining messages.
// 
// Note: pTaskInfo can be NULL.
//
// This downloads all messages whose UID's are in NewUidMap into
// "pSumList".
//
/////////////////////////////////////////////////////////////////////
HRESULT CImapChecker::DownloadRemainingMessagesMT (CUidMap& NewUidMap,
							 CImapSumList *pSumList, BOOL bToTmpMbx, CTaskInfoMT* pTaskInfo)
{
	HRESULT		hResult = S_OK;
	IMAPUID		uid		= 0;
	CString		strMainText;

	// Must have a summary list object.
	if (! (pSumList) )
		return E_INVALIDARG;

	// Reset this:
	//
	m_bStopRequested = FALSE;

	//
	// Get new value of count. Note that NewUidMap contains new messages only.
	//
	int nCountAfterFiltering = NewUidMap.size ();

	// Don't show progress every iteration! Do 1 percent.
	//
	int ProgressInc = (int) max((nCountAfterFiltering * 1) / 100, 1);

	strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nCountAfterFiltering);

	// Now go download remaining messages.
	if (InProgress && ::IsMainThreadMT() && (nCountAfterFiltering > 0) )
	{
		MainProgress (strMainText);
	}
	// Initialize for background progress.
	//
	else if (pTaskInfo && !IsMainThreadMT() && (nCountAfterFiltering > 0) )
	{
		pTaskInfo->SetMainText(strMainText);

		pTaskInfo->SetTotal(nCountAfterFiltering);

	}

	//
	// Keep tab of how many we download..
	//
	int count = 0;

	// If error mid-way, stop the transfer.
	hResult = S_OK;

	// Loop through all messages.

	CImapFlags	*pF;

	for (UidIteratorType ci = NewUidMap.begin(); ci != NewUidMap.end(); ++ci)
	{
		// Does user want to quit??
		if ( m_bStopRequested )
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

		// If success, this will be non-NULL:
		CImapSum *pImapSum; pImapSum = NULL;

		// If this message was not filtered, go download to the mailbox.
		// Check to see if to download all or just minimal download.
		//
		// FORNOW - enforce minimal download if in background:
		//
		if ( m_bDoingMinimalDownload || !IsMainThreadMT() )
		{
			// Return a partially filled-in new CSummary object.
			hResult = DoMinimalDownload (uid, &pImapSum);

			// If this our highest UID so far?
			//
			if ( pImapSum )
			{
				if ( uid > m_NewUIDHighest )
					m_NewUIDHighest = uid;

				// Indicate that this is a minimal download.
				pImapSum->m_Imflags |= IMFLAGS_NOT_DOWNLOADED;
			}
		}
		else
		{
			pImapSum = new CImapSum;
			if (pImapSum)
			{
				// At lease these must be filled in:
				//
				pImapSum->SetHash (uid);

				// Fetch message, probably including all attachments.
				hResult = DownloadSingleMessage (pImapSum, FALSE, bToTmpMbx);

				if ( SUCCEEDED (hResult) )
				{			
					// Indicate that this was more than a minimal download..
					pImapSum->m_Imflags &= ~IMFLAGS_NOT_DOWNLOADED;

					// If this our highest UID so far?
					//
					if ( uid > m_NewUIDHighest )
						m_NewUIDHighest = uid;
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

		
		if ( (ProgressInc == 1) || (count == nCountAfterFiltering) || ((count % ProgressInc) == 0) )
		{
			// Update progress bar.
			//
			strMainText.Format(CRString(IDS_POP_MESSAGES_LEFT), nCountAfterFiltering - count);

			if (InProgress && ::IsMainThreadMT() && SUCCEEDED (hResult) )
			{
				MainProgress (strMainText);
			}
			// If in background, use the task-status' progress.
			else if (!::IsMainThreadMT() && pTaskInfo)
			{
				pTaskInfo->SetMainText(strMainText);

				pTaskInfo->ProgressAdd(ProgressInc);
			}
		}

		// Does user want to quit??
		if ( m_bStopRequested )
		{
			hResult = HRESULT_MAKE_CANCEL;

			break;
		}

	}  // For.

	return hResult;
}






void CImapChecker::RequestThreadStop()
{
	m_bStopRequested = TRUE;

	if (m_pImap)
		m_pImap->RequestStop();
}



// AcquireNetworkConnection [PRIVATE]
//
// Note: This sets the m_pImap object.
//
HRESULT CImapChecker::AcquireNetworkConnection ()
{
	CIMAP *pImap = NULL;

	// THis might get an existing one.
	//
	HRESULT hResult = GetImapConnectionMgr()->CreateConnection (m_AccountID, m_szImapName, &pImap, TRUE);

	if ( pImap && SUCCEEDED (hResult) )
	{
		// Set he internal pointer.
		//
		m_pImap = pImap;
	}

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
HRESULT CImapChecker::DoMinimalDownload (IMAPUID Uid, CImapSum **ppSum)
{
	BOOL			bWeOpenedOurConnection = FALSE;
	CImapSum*		pSum = NULL;
	HRESULT			hResult = S_OK;

	// Sanity. Must have a valid TocDoc and Uid. 
	if ( Uid == 0 || NULL == ppSum)
	{
		ASSERT (0);
		return E_INVALIDARG;
	}

	// Reset this.
	m_bStopRequested = FALSE;

	// Intialize output parms:
	*ppSum = NULL;

	// Must hae this:
	//
	if (!m_pImap)
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Instantiate a CImapDownloader object to do the actual downloading.
	CImapDownloader ImapDownloader (m_AccountID, m_pImap, NULL);

	pSum = NULL;
	hResult = ImapDownloader.DoMinimalDownload (Uid, &pSum);

	// Set output parm:
	*ppSum = pSum;

	return hResult;
}



//////////////////////////////////////////////////////////////////////////////////////
// __DownloadSingleMessage [PRIVATE]
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
HRESULT CImapChecker::DownloadSingleMessage (CImapSum *pSum,
											 BOOL bDownloadAttachments, BOOL bToTmpMbx)
{
	HRESULT hResult = S_OK;

	// Sanity:
	if (! (pSum) )
		return E_INVALIDARG;

	// Reset this:
	//
	m_bStopRequested = FALSE;

	// Must have one of these:
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
		szMbxFilePath = m_szTmpMbxFilePath;
	}
	else
	{
		szMbxFilePath = m_szRealMbxFilePath;
	}

	// Instantiate an ImapDownloader object to do the downloading.
	CImapDownloader MbxDownloader (m_AccountID, m_pImap, (LPCSTR) szMbxFilePath);

	// 	Make sure the file exists. Open it for appending. Don't truncate!!
	if (!MbxDownloader.OpenMbxFile (FALSE))
	{
//		ErrorDialog ( IDS_ERR_FILE_OPEN, szMbxFilePath, CRString (IDS_ERR_FILE_OPEN_WRITING) );

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
// Given that we got new mail during a mail check, update the
// passed-in boolean references based on INI settings.
// This is currently called from pop.cpp AFTER all POP accounts have been checked 
// for new mail.
//
void CImapChecker::SetNotificationFlags ()
{
	unsigned int uiFlags = 0;

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
			if (! (m_ulBitFlags & kSilentCheckBit))
			{
				if ( GetIniShort(IDS_INI_ALERT) )
				{
					uiFlags |= CMF_NOTIFY_NOMAIL;
				}
			}
		}
		else // We got new mail:
		{
			m_pNewMailNotifier->SetNotifyNewMail(true);

			// Do we alert?
			if (GetIniShort(IDS_INI_ALERT) )
			{
				uiFlags |= CMF_ALERT_NEWMAIL;
			}

			// Play sound?
			if (GetIniShort(IDS_INI_SOUND))
				uiFlags |= CMF_PLAY_SOUND;

			uiFlags |= CMF_RECEIVED_MAIL;
		}

		m_pNewMailNotifier->AddNotifications (uiFlags);
	}
}



// [PRIVATE]
//
// If the "error type" is not one of CTaskInfo's registered types,
// ignore it and send a registered type:
//

void CImapChecker::ErrorCallback (LPCSTR str, UINT ErrorType)
{
	if (!str)
		return;

	TaskErrorType terr = (TaskErrorType) ErrorType;

	if (terr != TERR_WINSOCK)
		terr = TERR_MSG;

	if (m_pTaskInfo)
		m_pTaskInfo->CreateError (str, terr);
}




//============================== STATICS ==============================//



// DoMergeNewTocs;
// Second step in the mail grabbing procedure.
//
// Static
void DoMergeNewTocs (void *pv, bool bLastTime)
{
	// Sanity:
	if (!pv)
		return;

	CImapChecker *pImapChecker = (CImapChecker *)pv;

	// Data is a CImapChecker!!
	//
	pImapChecker->DoMergeNewTocs ();
}




// DummyPostProc;
// Task manager MUST have a post processing proc.
// If we fail, we don't want to process the messages we got, so use this
// as the post processing proc so we can, at least, delete the CImapChecker object.
//
// Static
void DummyPostProc (void *pv, bool)
{
	// Sanity:
	if (!pv)
		return;

	CImapChecker *pImapChecker = (CImapChecker *)pv;
}



#endif // IMAP4



