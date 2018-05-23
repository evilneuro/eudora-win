// ImapMailMgr.cpp: implementation of the IMAP mail management class.
// This class is the main entry point to handling IMAP mail. It will eventuall
// become a COM interface.
//
// Copyright (c) 1997-2003 by QUALCOMM, Incorporated
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
#include "ImapAccountMgr.h"
#include "ImapActionQueue.h"
#include "ImapConnection.h"
#include "ImapMailbox.h"
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


#include "DebugNewHelpers.h"



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

HRESULT CImapMailMgr::ImapGetMail(LPCSTR pszPersonaName,
								  unsigned long ulBitflags,
								  QCPOPNotifier *pNewMailNotifier,
								  BOOL bInBackground /*=FALSE*/,
								  BOOL bFullMailCheck /*=TRUE*/)
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
			hResult = CheckMailInBackground (szPers, ulBitflags, pNewMailNotifier, bFullMailCheck);
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
HRESULT CImapMailMgr::CheckMailInBackground(LPCSTR pszPersona,
											unsigned long ulBitflags,
											QCPOPNotifier *pNewMailNotifier,
											BOOL bFullMailCheck /*TRUE*/)
{
	// This MUST always be called in the main thread.
	if (!IsMainThreadMT())
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Sanity:
	if (!pszPersona)
	{
		return E_FAIL;
	}

	// Get the Inbox's TOC for this account.
	DWORD					 dwAccountID = g_Personalities.GetHash(pszPersona);

	// Make sure the account's mailbox list has been fetched:
	if (!g_ImapAccountMgr.VerifyMboxesFetched(dwAccountID))
	{
		return E_FAIL;
	}

	// This entry point applies only to INBOX.
	CRString				 strInboxName(IDS_IMAP_RAW_INBOX_NAME);

	CTocDoc		*pTocDoc = g_theMailboxDirector.ImapFindTOCByImapName(dwAccountID, strInboxName, '\0');

	if (!pTocDoc)
	{
		return E_FAIL;
	}

	// Make sure the local mailbox directory exists.
	if (!pTocDoc->m_pImapMailbox->VerifyCache(TRUE))
	{
		return E_FAIL;
	}

	// We may have to do the login stuff here:
	if (!SUCCEEDED(pTocDoc->m_pImapMailbox->GetLogin()))
	{
		return HRESULT_MAKE_CANCEL;
	}

	HRESULT hr = CheckMail(pszPersona, pTocDoc, TRUE/*bCheck*/, ulBitflags, pNewMailNotifier);

	// If this is a full mail check (not just the Inbox opening) resync any other mailboxes
	// with their mailbox sync property set.
	if (bFullMailCheck)
	{
		g_theMailboxDirector.ImapSyncMailboxesInAccount(pszPersona, ulBitflags, pNewMailNotifier);
	}

	return hr;
}

//
// CImapMailMgr::CheckMail()
//
// Performs the actual mail checking function.  Takes a personality and a mailbox and
// does a sync on that mailbox.
//
HRESULT CImapMailMgr::CheckMail(LPCSTR szPersona,
								CTocDoc *pTocDoc,
								BOOL bCheck,
								unsigned long ulBitflags,
								QCPOPNotifier *pNewMailNotifier /*= NULL*/,
								BOOL bDownloadedOnly /*= TRUE*/)
{
	if (!pTocDoc || !pTocDoc->m_pImapMailbox || !szPersona || !*szPersona)
	{
		return E_FAIL;
	}

	CImapMailbox		*pImapMailbox = pTocDoc->m_pImapMailbox;
	CImapAccount		*pAccount = g_ImapAccountMgr.FindAccount(pImapMailbox->GetAccountID());
	if (pAccount)
	{
		CImapResyncAction	*pImapResyncAction = DEBUG_NEW CImapResyncAction(pAccount->GetAccountID(),
																			 pImapMailbox->GetImapName(),
																			 pImapMailbox->GetDelimiter(),
																			 bCheck,
																			 ulBitflags,
																			 pNewMailNotifier ? TRUE : FALSE,
																			 bDownloadedOnly);
		if (pImapResyncAction)
		{
			if (!GetIniShort(IDS_INI_CONNECT_OFFLINE))
			{
				// If we are online then make this an immediate action (i.e., put it at
				// the beginning of the queue and process it right away).
				pImapResyncAction->SetPriority(kImapActionPriorityHigh);
			}
			pAccount->QueueAction(pImapResyncAction);
		}
	}

	// We queued it.
	return S_OK;
}


//
// CImapMailMgr::CheckMail()
//
// Performs the actual mail checking function.  Takes a personality and a mailbox and
// does a sync on that mailbox.
//
HRESULT CImapMailMgr::CheckMailOnServer(LPCSTR szPersona,
										CTocDoc *pTocDoc,
										BOOL bCheck,
										unsigned long ulBitflags,
										QCPOPNotifier *pNewMailNotifier /*= NULL*/,
										BOOL bDownloadedOnly /*= TRUE*/)
{
	// Create the object that's going to do the mail checking.
	CImapChecker	*pImapChecker = DEBUG_NEW_NOTHROW CImapChecker(szPersona, pTocDoc->m_pImapMailbox->GetImapName(), ulBitflags);

	if (!pImapChecker)
	{
		return E_FAIL;
	}

	DWORD dwAccountID = g_Personalities.GetHash(szPersona);
	CString csHomie = g_Personalities.GetCurrent();
	g_Personalities.SetCurrent(szPersona);
	g_Personalities.SetCurrent(csHomie);

	// "AddTask" will fail if there's already a similar task.
	// If the task is added, the CImapChecker will have to remove it.
	CString szTaskKey;

	if (!AddTask(dwAccountID, pTocDoc->m_pImapMailbox->GetImapName(), szTaskKey))
	{
		delete pImapChecker->GetTaskInfo();
		delete pImapChecker;
		return E_FAIL;
	}

	// Tell the task what it's task key is.
	pImapChecker->SetTaskKey (szTaskKey);

	// Set name of TCODOC. For now, it's only inbox.
	pImapChecker->SetTocName (pTocDoc->Name());

	// Let the ImapChecker grab needed state data from the CImapMailbox.
	pImapChecker->DoMainThreadInitializations (pTocDoc);

	// Set the current UID list into pImapChecker so it won't have to
	// do it in a worker thread.
	GetCurrentUids (pTocDoc, pImapChecker->GetInternalCurrentUidMap());

	// Indicate what our task is, and set it's parameters.
	if (bCheck)
	{
		pImapChecker->SetCheckMailWorkFunction ();

		// If this is a manual check-mail, force a resync.
		if (ulBitflags == kManualMailCheckBits)
			pImapChecker->ForceResync();
	}
	else
	{
		// Make sure do a resync.
		pImapChecker->ForceResync();

		pImapChecker->SetResyncWorkFunction (bDownloadedOnly, 0xFFFFFFFF );
	}
	 
	// Queue the task now.
	if (!QCGetTaskManager()->QueueWorkerThread(pImapChecker))
	{
		delete pImapChecker->GetTaskInfo();
		delete pImapChecker;
		return E_FAIL;
	}

	if (bCheck)
	{
		// Set the new mail notifier object only after the task has been
		// scheduled successfully.
		pImapChecker->SetNewMailNotifier (pNewMailNotifier);
	}

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
									   BOOL bInBackground /* = FALSE */,
									   BOOL bDoSubMailboxes /* = FALSE */)
{
	// This MUST always be called in the main thread.
	if (!IsMainThreadMT())
	{
		ASSERT(0);
		return E_FAIL;
	}

	// Sanity:
	if (!pTocDoc || !pTocDoc->m_pImapMailbox)
	{
		return E_FAIL;
	}

	// Make sure the local mailbox directory exists.
	if (!pTocDoc->m_pImapMailbox->VerifyCache(TRUE))
	{
		return E_FAIL;
	}

	// Get the persona name.
	CString			 strPersona;
	CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(pTocDoc->m_pImapMailbox->GetAccountID());
	if (pAccount)
	{
		pAccount->GetName(strPersona);
	}

	if (strPersona.IsEmpty())
	{
		return E_FAIL;
	}

	// If resyncing entire tree, sync child mailboxes now.
	if (bDoSubMailboxes)
	{
		QCImapMailboxCommand	*pImapCommand = NULL;
		if (pTocDoc->m_pImapMailbox)
		{
			pImapCommand = g_theMailboxDirector.ImapFindByImapName(pTocDoc->m_pImapMailbox->GetAccountID(), 
																   pTocDoc->m_pImapMailbox->GetImapName(),
																   pTocDoc->m_pImapMailbox->GetDelimiter());
			
		}
		if (pImapCommand)
		{
			CPtrList				*pList = &(pImapCommand->GetChildList());
			if (pList && (pList->GetCount() > 0))
			{
				g_theMailboxDirector.ImapSyncMailboxesInTree(strPersona,
															 kManualMailCheckBits,
															 pList,
															 FALSE/*bOnlyIfPropSet*/,
															 NULL/*pNewMailNotifier*/);
			}
		}
	}

	return CheckMail(strPersona, pTocDoc, FALSE/*bCheck*/, kManualMailCheckBits,
					 NULL/*pNewMailNotifier*/, bDownloadedOnly);
}


//======================= Access to task map ==========================//


// RemoveTask [PUBLIC]
//
// If we can find a task like our's in the map, remove it.
// Note: The CImapChecker calls this to remove it's task.
//
void CImapMailMgr::RemoveTask (LPCSTR pTaskKey)
{
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
			QCMailboxCommand* pImapFolder = g_theMailboxDirector.FindByPathname( (const char *) doc->GetMBFileName() );
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
	// CImapConnection::Initialize() and Cleanup() haven't done any actual work
	// for some time now.  They have been removed and maybe we should remove
	// the equivalent CImapMailMgr calls, but for now they remain.
	// CImapConnection::Initialize();
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
				if( nAccountID == -1 || doc->m_pImapMailbox->GetAccountID() == (unsigned)nAccountID )
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
	// CImapConnection::Initialize() and Cleanup() haven't done any actual work
	// for some time now.  They have been removed and maybe we should remove
	// the equivalent CImapMailMgr calls, but for now they remain.
	// CImapConnection::Cleanup();
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

