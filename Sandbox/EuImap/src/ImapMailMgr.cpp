// ImapMailMgr.cpp: implementation of the IMAP mail management class.
// This class is the main entry point to handling IMAP mail. It will eventuall
// become a COM interface.
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "doc.h"
#include "tocdoc.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "eudora.h"
#include "persona.h"
#include "pop.h"
#include "QCWorkerThreadMT.h"
#include "QCTaskManager.h"   //for QueueWorkerThread
#include "guiutils.h"

// Yucky - shouldn't have to include this!!!
#include "POPSession.h"

#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"

#include "ImapSum.h"
#include "imapopt.h"
#include "imapactl.h"
#include "imaputil.h"
#include "imapfol.h"
#include "ImapMailMgr.h"
#include "ImapChecker.h"
#include "ImapTypes.h"

#ifdef EXPIRING
	// The Evaluation Time Stamp object
	#include "timestmp.h"
	extern CTimeStamp	g_TimeStamp;
#endif


// Globals: The one and only CImapMailMgr object!!.
CImapMailMgr g_ImapMailMgr;


// We need to extern this - yuck!!
extern QCMailboxDirector	g_theMailboxDirector;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



// ============ Local Functions ===================//


// ==========================================================================//

// This is the global MAP for holding the tasks.
// s_TaskLockable is it's resource lock.
//
static CStringMap		s_TaskMap;
static CCriticalSection s_TaskLockable;

// Global flag to inform IMAP when eudora is fuly started.
//
// static
BOOL CImapMailMgr::m_bAppIsInitialized = FALSE;


//=============================================================================//


// ====================== CImapMailMgr methods  ==================//


CImapMailMgr::CImapMailMgr(void)
{
	m_bGotNewMail = FALSE;

}


// 
CImapMailMgr::~CImapMailMgr()
{


}




// ImapGetMail [PUBLIC]
// FUNCTION
// External interface to checking mail for an IMAP personality.
// END FUNCTION

// NOTES
// Now checks mail for the given personality only.
// Returns an HRESULT.
// NOTE: This is what gets called from the global "GetMail()" function.
// ASSUME current personality is "pszPersonaName".
// END NOTES

HRESULT CImapMailMgr::ImapGetMail (LPCSTR pszPersonaName, unsigned long ulBitflags, QCPOPNotifier *pNewMailNotifier, BOOL bInBackground /* = FALSE */)
{
	HRESULT hResult = E_FAIL;

	// This MUST always be called in the main thread.
	//
	if (! IsMainThreadMT() )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// ALways reset this first thing:
	m_bGotNewMail = FALSE;

	// Must have a personality.
	//.
	if (NULL == pszPersonaName)
	{
		// Needs to handle this error properly.
		// 
		return E_FAIL;
	}

	// Dominant persona must be set to "<Dominant>"
	//
	CString szPers = pszPersonaName;

	if ( szPers.IsEmpty() )
	{
		szPers = CRString (IDS_DOMINANT);
	}
		

	// Check mail only if it's an IMAP personality.
	if ( GetIniShort (IDS_INI_USES_IMAP) )
	{
		// We handle checking mail, either directly or in a background thread.
		//
		if (bInBackground)
			hResult = CheckMailInBackground (szPers, ulBitflags, pNewMailNotifier);
	}

	return hResult;
}




//
// Checking mail for "inbox"
// Note: This method is always executed in the main thread, and it launches 
// a worker thread to do the checkmail in the background.
// 
// HISTORY:
// (JOK - 6/12/98): Added "ulBitflags" - if this is the equivalent of a 
// manual check mail, force a resync.
//
HRESULT CImapMailMgr::CheckMailInBackground (LPCSTR pszPersona, unsigned long ulBitflags, QCPOPNotifier *pNewMailNotifier)
{
	// This MUST always be called in the main thread.
	//
	if (! IsMainThreadMT() )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Sanity:
	// 
	if (! pszPersona  )
		return E_FAIL;

	// Get the Inbox's TOC for this account.
	//
	DWORD AccountID = g_Personalities.GetHash(pszPersona );

	// Make sure the account's mailbox list has been fetched:
	//
	if ( !g_ImapAccountMgr.VerifyMboxesFetched ( AccountID ) )
	{
		return E_FAIL;
	}

	// This entry point applies only to INBOX.
	//
	CRString szInboxName (IDS_IMAP_RAW_INBOX_NAME);


	// Make sure we have a command object for inbox.
	QCImapMailboxCommand*	pImapCommand = g_theMailboxDirector.ImapFindByImapName(
												AccountID,
												szInboxName,
												'\0');
	if (!pImapCommand)
	{
		return E_FAIL;
	}

	// Get the toc even if it's not already loaded.
	CTocDoc* pTocDoc = GetToc( pImapCommand->GetPathname(), pImapCommand->GetName());

	if (!pTocDoc)
	{
		return E_FAIL;
	}

	if (!pTocDoc->m_pImapMailbox)
	{
		return E_FAIL;
	}

	// Make sure the local mailbox directory exists.
	// 
	if ( ! pTocDoc->m_pImapMailbox->VerifyCache (TRUE) )
	{

		return E_FAIL;
	}

	// We may have to do the login stuff here:
	//
	if (!SUCCEEDED (pTocDoc->m_pImapMailbox->GetLogin()) )
	{
		return HRESULT_MAKE_CANCEL;
	}

	// Create the object that's going to do the mail checking.
	//
	CImapChecker* pImapChecker = new CImapChecker (pszPersona, szInboxName, ulBitflags);

	if (!pImapChecker)
	{
		return E_FAIL;
	}

	// "AddTask" will fail if there's already a similar task.
	// If the task is added, the CImapChecker will have to remove it.
	CString szTaskKey;

	if ( !AddTask(AccountID, szInboxName, szTaskKey) )
	{
		delete pImapChecker->GetTaskInfo();
		delete pImapChecker;

		return E_FAIL;
	}

	// Tell the task what it's task key is.
	//
	pImapChecker->SetTaskKey (szTaskKey);

	// Set name of TCODOC. For now, it's only inbox.
	//
	pImapChecker->SetTocName (szInboxName);

	// Let the ImapChecker grab needed state data from the CImapMailbox.
	//
	pImapChecker->DoMainThreadInitializations (pTocDoc);

	// Set the current UID list into pImapChecker so it won't have to
	// do it in a worker thread.
	//
	GetCurrentUids (pTocDoc, pImapChecker->GetInternalCurrentUidMap());

	// Make sure we specify the actual background task:
	//
	pImapChecker->SetCheckMailWorkFunction ();

	// If this is a manual check-mail, force a resync.
	// 
	if (ulBitflags == kManualMailCheckBits)
		pImapChecker->ForceResync();

	// Queue the task now.
	//
	if (!QCGetTaskManager()->QueueWorkerThread(pImapChecker))
	{
		delete pImapChecker->GetTaskInfo();
		delete pImapChecker;

		return E_FAIL;
	}

	// Set the new mail notifier object only after the task has been
	// scheduled successfully.
	//
	pImapChecker->SetNewMailNotifier (pNewMailNotifier);

	// We launched it.
	return S_OK;
}





// Note: This is ALWAYS called in the foreground.
//
// Start a mailbox re-sync. Do stuff in the foreground then, if InBackground is requested, 
// launch the background task.
//
HRESULT CImapMailMgr::DoManualResync (CTocDoc *pTocDoc,
								       BOOL bCheckMail /* = FALSE */,
									   BOOL bDownloadedOnly /* = TRUE */,
									   BOOL bInBackground /* = FALSE */)
{
	if (! IsMainThreadMT() )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// Sanity:
	// 
	if (!pTocDoc)
	{
		return E_FAIL;
	}

	if (!pTocDoc->m_pImapMailbox)
	{
		return E_FAIL;
	}

	// Make sure the local mailbox directory exists.
	// 
	if ( ! pTocDoc->m_pImapMailbox->VerifyCache (TRUE) )
	{

		return E_FAIL;
	}

	// Don't know why we need the persona name...
	//
	CString szPersona;

	CImapAccount* pAccount = g_ImapAccountMgr.FindAccount ( pTocDoc->m_pImapMailbox->GetAccountID() );
	if (pAccount)
	{
		pAccount->GetName (szPersona);
	}

	// Create the object that's going to do the mail checking.
	//
	CImapChecker* pImapChecker = new CImapChecker (szPersona, pTocDoc->m_pImapMailbox->GetImapName(), kManualMailCheckBits);

	if (!pImapChecker)
	{
		return E_FAIL;
	}

	// "AddTask" will fail if there's already a similar task.
	// If the task is added, the CImapChecker will have to remove it.
	CString szTaskKey;

	if ( !AddTask(pAccount->GetAccountID(), pTocDoc->m_pImapMailbox->GetImapName(), szTaskKey) )
	{
		delete pImapChecker->GetTaskInfo();
		delete pImapChecker;

		return E_FAIL;
	}

	// Tell the task what it's task key is.
	//
	pImapChecker->SetTaskKey (szTaskKey);

	// Set name of TCODOC. For now, it's only inbox.
	//
	pImapChecker->SetTocName (pTocDoc->Name());

	// Let the ImapChecker grab needed state data from the CImapMailbox.
	//
	pImapChecker->DoMainThreadInitializations (pTocDoc);

	// Set the current UID list into pImapChecker so it won't have to
	// do it in a worker thread.
	//

	GetCurrentUids (pTocDoc, pImapChecker->GetInternalCurrentUidMap());

	// Make sure do a resync.
	//
	pImapChecker->ForceResync();

	// Indicate what our task is, and set it's parameters.
	//
	pImapChecker->SetResyncWorkFunction (bDownloadedOnly, 0xFFFFFFFF );
	 
	// Queue the task now.
	//
	if (!QCGetTaskManager()->QueueWorkerThread(pImapChecker))
	{
		delete pImapChecker->GetTaskInfo();
		delete pImapChecker;

		return E_FAIL;
	}

	// We launched it.
	return S_OK;
}


//======================= Access to task map ==========================//


// RemoveTask [PUBLIC]
//
// If we can find a task like our's in the map, remove it.
// Note: The CImapChecker calls this to remove it's task.
//
void CImapMailMgr::RemoveTask (LPCSTR pTaskKey)
{
	BOOL	bResult = FALSE;

	if (!pTaskKey)
		return;

	// Attempt to lock task map.
	//
	if ( s_TaskLockable.Lock() )
	{
		// We can't add the task if we've already got one going.
		//
		StringMapIteratorType ci = s_TaskMap.find (pTaskKey);

		if ( ci  != s_TaskMap.end() )
		{
			// remove it.
			//
			s_TaskMap.erase (ci);
		}

		// Make sure to unlock.
		s_TaskLockable.Unlock();
	}
}




// GenerateKey [PRIVATE]
//
//
void CImapMailMgr::GenerateKey (unsigned long AccountID, LPCSTR pImapName, CString& szKey)
{
	szKey.Empty();

	if ( (AccountID == 0) || !(pImapName && *pImapName) )
		return;

	szKey.Format ("%08lu%s", AccountID, pImapName);
}




// AddTask [PRIVATE]
//
// Add a new task if there's no similar task already running.
//
BOOL CImapMailMgr::AddTask(unsigned long AccountID, LPCSTR pImapName, CString& szTaskKey)
{
	BOOL	bResult = FALSE;

	// Make sure.
	szTaskKey.Empty();

	// Sanity:
	if (! (AccountID && pImapName && *pImapName) )
		return FALSE;

	CString szLocalKey;

	GenerateKey (AccountID, pImapName, szLocalKey);

	if ( szLocalKey.IsEmpty() )
		return FALSE;

	// Attempt to lock task map.
	//
	if ( s_TaskLockable.Lock() )
	{
		// We can't add the task if we've already got one going.
		//
		if ( s_TaskMap.find (szLocalKey) == s_TaskMap.end() )
		{
			// we can add.
			//
			s_TaskMap[szLocalKey] = 0;

			// We got it.
			//
			bResult = TRUE;

			// Copy to out:
			//
			szTaskKey = szLocalKey;
		}

		// Make sure to unlock.
		s_TaskLockable.Unlock();
	}

	return bResult;
}


//==========================================================//

// ResyncOpenedTocs [PUBLIC]
//
// Launch manual resync's on all open TOCS:
//

void CImapMailMgr::ResyncOpenedTocs ()
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	// Don't do this if we're offline.
	//
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE) )
		return;

	// Give back some time for a while:
	EscapePressed();

	POSITION pos = TocTemplate->GetFirstDocPosition();
	
	while (pos)
	{
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc && doc->IsImapToc())
		{
			// Pass this to the IMAP command object.
			//
			QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) doc->MBFilename() );
			if( pImapFolder )
				pImapFolder->Execute( CA_IMAP_RESYNC, NULL);
			else
			{
				ASSERT(0);
			}
		}
	}
}




// InitializeImapProvider [STATIC]
// Call the "Initialize()" method in the IMAP DLL to do initialization
// stuff.
// This should be called ONCE, at startup.
//
/* static */
void CImapMailMgr::InitializeImapProvider()
{
	CIMAP::Initialize();
}



// Cleanup [STATIC]
// 
// Called on application shutdown. Close all connections to all IMAP
// servers.
// Loop through all TocDoc's and close IMAP connections.
//

/* static */
void CImapMailMgr::CloseImapConnections (int nAccountID /*=-1*/)
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	POSITION pos = TocTemplate->GetFirstDocPosition();
	
	while (pos)
	{
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc)
		{
			if (doc->IsImapToc() && doc->m_pImapMailbox)
			{
				if( nAccountID == -1 || doc->m_pImapMailbox->GetAccountID() == nAccountID )
					doc->m_pImapMailbox->Close();
			}
		}
	}
}


// Shutdown [STATIC]
//
// SHUTDOWN THE IMAP DLL winsock.
//
/* static */
void CImapMailMgr::Shutdown ()
{
	// Call the IMAP service provider's cleanup routine.
	CIMAP::Cleanup();
}




// Notify IMAP subsystem that the application has been initialized on startup.
//static
void CImapMailMgr::NotifyAppInitialized()
{
	m_bAppIsInitialized = TRUE;
}





// Exported function to return the global account manager.
//
CImapMailMgr* GetImapMailMgr ()
{
	return &g_ImapMailMgr;
}



#endif // IMAP4

