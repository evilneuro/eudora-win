//	X1EmailScanner.cpp
//
//	Scans email to build X1 indices.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#include "stdafx.h"
#include "X1EmailScanner.h"
#include "X1EmailIndexItem.h"
#include "SearchManagerInfo.h"
#include "SearchManagerTaskInfo.h"
#include "SearchManagerUtils.h"
#include "debug.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCTaskManager.h"


extern QCMailboxDirector		g_theMailboxDirector;


// ---------------------------------------------------------------------------
//		* X1EmailScanner											 [Public]
// ---------------------------------------------------------------------------
//	X1EmailScanner constructor

X1EmailScanner::X1EmailScanner(
	X1::IX1Database *			in_pX1DB)
	:	m_nRefCount(0), m_pEmailDB(in_pX1DB), m_nConnectionHandle(0),
		m_nScanCount(0), m_eScanState(ss_Stopped),
		m_eLastScanType(st_None), m_eScanType(st_None), m_eNextScanType(st_None),
		m_bProcessingIndexUpdateActions(false), m_szMessageBuffer(),
		m_nMessageIDLastAddAttempt(0), m_pUpdateActionsTaskInfo(NULL),
		m_nTotalUpdateActions(0), m_nCurrentUpdateAction(0),
		m_szMailboxName(), m_nTotalMessagesToRemove(0),
		m_pMainScansTaskInfo(NULL),
		m_pMailboxList(NULL), m_posCurrentMailbox(NULL), m_pCurrentMailbox(NULL),
		m_nTotalMailboxes(0), m_nCurrentMailbox(0),
		m_pX1IndexedItems(), m_nNumX1IndexedItems(0),
		m_nX1IndexedItem(0)
{
	//	Initialize arrays
	m_pCurrentTocDoc[sd_MainScans] = NULL;
	m_bMailboxFileExists[sd_MainScans] = TRUE;
	m_posCurrentSummary[sd_MainScans] = NULL;
	m_posLastSummaryAddAttempt[sd_MainScans] = NULL;
	m_nCurrentMessage[sd_MainScans] = 0;
	m_nSoFarBase[sd_MainScans] = 0;
	m_bCheckForIndexedMessages[sd_MainScans] = true;
	m_bCheckingMessages[sd_MainScans] = false;
	m_bNeedToAddMessages[sd_MainScans] = false;

	m_pCurrentTocDoc[sd_IndexUpdateActions] = NULL;
	m_bMailboxFileExists[sd_IndexUpdateActions] = TRUE;
	m_posCurrentSummary[sd_IndexUpdateActions] = NULL;
	m_posLastSummaryAddAttempt[sd_IndexUpdateActions] = NULL;
	m_nCurrentMessage[sd_IndexUpdateActions] = 0;
	m_nSoFarBase[sd_IndexUpdateActions] = 0;
	m_bCheckForIndexedMessages[sd_IndexUpdateActions] = true;
	m_bCheckingMessages[sd_IndexUpdateActions] = false;
	m_bNeedToAddMessages[sd_IndexUpdateActions] = false;
}


// ---------------------------------------------------------------------------
//		* ~X1EmailScanner											 [Public]
// ---------------------------------------------------------------------------
//	X1EmailScanner destructor

X1EmailScanner::~X1EmailScanner() 
{
}


// ---------------------------------------------------------------------------
//		* QueryInterface											 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::QueryInterface() implementation

HRESULT __stdcall
X1EmailScanner::QueryInterface(
	const IID &				in_iid,
	void **					out_ppvObject)
{
	if ( (in_iid == IID_IUnknown) || (in_iid == X1::IID_IX1Scanner) )
	{
		*out_ppvObject = static_cast<X1::IX1Scanner*>(this);
	}
	else
	{
		*out_ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* AddRef													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::AddRef() implementation

ULONG __stdcall
X1EmailScanner::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}


// ---------------------------------------------------------------------------
//		* Release													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::Release() implementation

ULONG __stdcall
X1EmailScanner::Release()
{
	if (InterlockedDecrement(&m_nRefCount) == 0)
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


// ---------------------------------------------------------------------------
//		* raw_ScanItem												 [Public]
// ---------------------------------------------------------------------------
//	Called by the scanner manager to index the next item. This should
//	generally  only scan a single item at a time. pbDone should be set
//	to FALSE if there are more messages to scan, otherwise it should be
//	left untouched.

HRESULT __stdcall
X1EmailScanner::raw_ScanItem(
	unsigned char *				out_pbDone)
{	
	//	Don't do anything if the scanner isn't running...
	if (m_eScanState != ss_Running)
	{
		return S_OK;
	}

	//	Bump scan count so that we can detect when we've been scanning
	m_nScanCount++;

	//	Assume that there's more to do
	*out_pbDone = FALSE;

	//	Provided we're not currently doing a culling scan, we want to process
	//	index update actions whenever we have any. That way recently received
	//	email gets added immediately even when we're waiting for a slower
	//	add/update scan. The st_ProcessingIndexUpdateActions scan type is
	//	only really used as a placeholder when we're not using any other
	//	scan type.
	//
	//	We don't interrupt a culling scan because as it is currently implemented
	//	it requires that the index is not modified while it is running.
	//	(In fact in order to avoid modifying the index while it scans, the
	//	culling scan queues "remove" index update actions when it finds an index
	//	item to cull).
	if ( (m_eScanType != st_CullingScan) &&
		 SearchManager::Instance()->HasIndexUpdateActions() )
	{
		//	Do we have progress displaying yet?
		if ( !m_pUpdateActionsTaskInfo &&
			 (GetIniShort(IDS_INI_SHOW_INDEX_UPDATE_PROGRESS) != 0) )
		{
			long	nNumberUpdateActions = SearchManager::Instance()->GetNumberIndexUpdateActions();
			
			if ( (nNumberUpdateActions > GetIniLong(IDS_INI_INDEX_UPDATE_MIN_TO_SHOW_PROGRESS)) ||
				 SearchManager::Instance()->HasMailboxUpdateActions() )
			{
				QCTaskManager *		pTaskManager = QCGetTaskManager();

				if (pTaskManager)
				{
					//	Add task to task manager to display progress
					m_pUpdateActionsTaskInfo = DEBUG_NEW SearchManagerTaskInfo(false);
					pTaskManager->Register(m_pUpdateActionsTaskInfo);
				}
			}
		}

		if ( m_pUpdateActionsTaskInfo && (m_nCurrentUpdateAction == m_nTotalUpdateActions) )
		{
			//	We starting processing the next "batch" of actions. Set the current action
			//	to 0, and the total number of actions to the current remaining count.
			m_nCurrentUpdateAction = 0;
			m_nTotalUpdateActions = SearchManager::Instance()->GetNumberIndexUpdateActions();
			m_pUpdateActionsTaskInfo->SetTotal(m_nTotalUpdateActions * 100);
			m_nSoFarBase[sd_IndexUpdateActions] = 0;

			//	Set the title
			m_pUpdateActionsTaskInfo->SetTitle( CRString(IDS_UPDATING_INDEX_TITLE) );

			//	Set the initial main progress text
			CString		szMainText;
			szMainText.Format(IDS_UPDATING_INDEX_REMAINING, m_nTotalUpdateActions - m_nCurrentUpdateAction);
			m_pUpdateActionsTaskInfo->SetMainText(szMainText);
		}
		
		ProcessNextIndexUpdateItem();

		if (m_pUpdateActionsTaskInfo)
		{
			//	Update the progress text with the latest remaining count
			CString		szMainText;
			szMainText.Format(IDS_UPDATING_INDEX_REMAINING, m_nTotalUpdateActions - m_nCurrentUpdateAction);
			m_pUpdateActionsTaskInfo->SetMainText(szMainText);
		}

		if ( !SearchManager::Instance()->HasIndexUpdateActions() )
		{
			//	We're done (for now) so close the last mailbox that we had open
			CloseCurrentMailbox(sd_IndexUpdateActions);

			//	Tell SearchManager to return FALSE in SearchManager::Idle so that
			//	extended idle processing stops until the next idle timer.
			//	This will give our progress bar a chance to display full.
			SearchManager::Instance()->SetPauseExtendedIdleProcessing(true);
		}
	}
	else
	{
		if (m_pUpdateActionsTaskInfo)
		{
			//	We have task info up from processing update actions. We're done
			//	now, so remove it. (We wait until now, i.e. until another idle
			//	after we finished, so that the task info has a chance to show
			//	a full progress bar).

			//	Reset action counts
			m_nCurrentUpdateAction = 0;
			m_nTotalUpdateActions = 0;

			//	Kill progress display
			m_pUpdateActionsTaskInfo->Kill();

			//	Kill triggers deletion of object
			m_pUpdateActionsTaskInfo = NULL;
		}
		
		switch (m_eScanType)
		{
			case st_AddOrUpdateItemsScan:
				//	Do the add/update items scan
				DoAddOrUpdateItemsScan();
				break;

			case st_CullingScan:
				//	Do the culling scan
				DoCullingScan();
				break;

			case st_ProcessingIndexUpdateActions:
				//	We would only get here if we don't have any index update
				//	actions. Therefore we're done with the current scan.
				StopCurrentScan();
				break;

			case st_ReadyForNextScanType:
				//	Move to the next scan
				m_eScanType = m_eNextScanType;

				//	Next scan is now nothing
				m_eNextScanType = st_None;

				switch (m_eScanType)
				{
					case st_AddOrUpdateItemsScan:
						//	Init add/update items scan
						InitAddOrUpdateItemsScan();
						break;

					case st_CullingScan:
						//	Init culling scan
						InitCullingScan();
						break;

					case st_None:
						//	Stop all scanning
						StopAllScanning();

						//	There's no more to do
						*out_pbDone = TRUE;
						break;
				}
				break;
		}
	}

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* IsScanning												 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not we're currently scanning.

bool
X1EmailScanner::IsScanning() const
{
	return (m_eScanState != ss_Stopped);
}


// ---------------------------------------------------------------------------
//		* IsDoingFullScan											 [Public]
// ---------------------------------------------------------------------------
//	Returns whether or not we're currently scanning.

bool
X1EmailScanner::IsDoingFullScan() const
{
	bool	bIsDoingFullScan = false;

	//	Check to see if we're scanning at all
	if ( IsScanning() )
	{
		//	Are we currently doing an add/update scan or culling scan?
		if ( (m_eScanType == st_AddOrUpdateItemsScan) || (m_eScanType == st_CullingScan) )
			bIsDoingFullScan = true;

		//	Are we about to do an add/update scan or culling scan?
		if ( (m_eScanType == st_ReadyForNextScanType) && 
			 ((m_eScanType == st_AddOrUpdateItemsScan) || (m_eScanType == st_CullingScan)) )
		{
			bIsDoingFullScan = true;
		}
	}
	
	return bIsDoingFullScan;
}


// ---------------------------------------------------------------------------
//		* StartScan													 [Public]
// ---------------------------------------------------------------------------
//	Starts scanning

void
X1EmailScanner::StartScan(
	ScanTypeEnum				in_eScanType,
	ScanTypeEnum				in_eNextScanType,
	const char *				in_szIndexingMailboxRelativePath)
{
	//	Start scan if:
	if ( (m_eScanState != ss_Running) || (m_eScanType != in_eScanType) )
	{
		TRACE("X1 scan started\n");

		//	Set scan state to running
		m_eScanState = ss_Running;

		//	Setup current and next scan type as indicated
		m_eLastScanType = st_None;
		m_eScanType = in_eScanType;
		m_eNextScanType = in_eNextScanType;

		//	Init scan types appropriately
		switch (m_eScanType)
		{
			case st_AddOrUpdateItemsScan:
				PutDebugLog( DEBUG_MASK_SEARCH, "Starting search indexing (adding and updating items)" );

				InitAddOrUpdateItemsScan(in_szIndexingMailboxRelativePath);
				break;

			case st_CullingScan:
				PutDebugLog( DEBUG_MASK_SEARCH, "Starting search indexing (culling items)" );
				
				InitCullingScan(in_szIndexingMailboxRelativePath);
				break;

			case st_ProcessingIndexUpdateActions:
				PutDebugLog( DEBUG_MASK_SEARCH, "Starting search indexing (processing index update items)" );
				break;
		}
	}
}


// ---------------------------------------------------------------------------
//		* PauseScan													 [Public]
// ---------------------------------------------------------------------------
//	Pauses scanning

void
X1EmailScanner::PauseScan(
	bool						in_bWaitingForIdle)
{
	if (m_eScanState == ss_Running)
	{
		TRACE("X1 scan paused\n");
		m_eScanState = ss_Paused;
	}

	if (in_bWaitingForIdle)
	{
		if (m_pUpdateActionsTaskInfo)
			m_pUpdateActionsTaskInfo->Progress( CRString(IDS_INDEXING_PAUSING) );

		if (m_pMainScansTaskInfo)
			m_pMainScansTaskInfo->Progress( CRString(IDS_INDEXING_PAUSING) );
	}
}


// ---------------------------------------------------------------------------
//		* ResumeScan												 [Public]
// ---------------------------------------------------------------------------
//	Resumes scanning

void
X1EmailScanner::ResumeScan()
{
	if (m_eScanState == ss_Paused)
	{
		TRACE("X1 scan resumed\n");
		m_eScanState = ss_Running;
	}
}


// ---------------------------------------------------------------------------
//		* StopCurrentScan											 [Public]
// ---------------------------------------------------------------------------
//	Stops current scan

void
X1EmailScanner::StopCurrentScan()
{
	if (m_eScanType == st_AddOrUpdateItemsScan)
		PutDebugLog( DEBUG_MASK_SEARCH, "Finished with search indexing (adding and updating items)" );
	else if (m_eScanType == st_CullingScan)
		PutDebugLog( DEBUG_MASK_SEARCH, "Finished with search index (culling items)" );

	if (m_pMainScansTaskInfo)
	{
		//	We have task info up from doing main scan processing.
		//	We're done now, so remove it.
		//	Kill progress display
		m_pMainScansTaskInfo->Kill();

		//	Kill triggers deletion of object
		m_pMainScansTaskInfo = NULL;
	}
	
	m_eLastScanType = m_eScanType;
	m_eScanType = st_ReadyForNextScanType;
}


// ---------------------------------------------------------------------------
//		* StopAllScanning											 [Public]
// ---------------------------------------------------------------------------
//	Stops all scanning

void
X1EmailScanner::StopAllScanning()
{
	if (m_eScanState != ss_Stopped)
	{
		if (m_pMainScansTaskInfo)
		{
			//	We have task info up from doing main scan processing.
			//	We're done now, so remove it.
			//	Kill progress display
			m_pMainScansTaskInfo->Kill();

			//	Kill triggers deletion of object
			m_pMainScansTaskInfo = NULL;
		}
		
		//	Mark ourselves stopped
		m_eScanState = ss_Stopped;

		//	No current or next scan types anymore
		m_eScanType = st_None;
		m_eNextScanType = st_None;
	
		//	Output information	
		TRACE("X1 scan stopped\n");
		PutDebugLog( DEBUG_MASK_SEARCH, "Stopping search indexing" );

		if ( (m_eLastScanType == st_AddOrUpdateItemsScan) ||
			 (m_eLastScanType == st_CullingScan) )
		{
			//	Notify Search Manager that we're done
			SearchManager::Instance()->NotifyIndexingStatus(m_eScanType, m_eNextScanType, "");
		}
	}
}


// ---------------------------------------------------------------------------
//		* NotifySumRemoval											 [Public]
// ---------------------------------------------------------------------------
//	Called by SearchManager when a summary is being removed from a given TOC.

void
X1EmailScanner::NotifySumRemoval(
	const CTocDoc *				in_pTocDoc,
	POSITION					in_posSum)
{
	for (ScanDataEnum i = sd_MainScans; i <= sd_IndexUpdateActions; )
	{
		if ( m_pCurrentTocDoc[i] && (in_pTocDoc == m_pCurrentTocDoc[i]) &&
			 m_posCurrentSummary[i] && (in_posSum == m_posCurrentSummary[i]) )
		{
			//	User is deleting the summary that we would have looked at next.
			//	Move on to the next summary now.
			CSumList &		listSums = m_pCurrentTocDoc[i]->GetSumList();

			listSums.GetNext( m_posCurrentSummary[i] );
		}

		//	Incrementing this was a pain, but this should do it
		i = static_cast<ScanDataEnum>( static_cast<int>(i) + 1 );
	}
}


// ---------------------------------------------------------------------------
//		* NotifyTemporarilyCloseMailbox								 [Public]
// ---------------------------------------------------------------------------
//	Called by SearchManager when we need to close a specified mailbox file
//	during mailbox compaction to make sure that we don't have the mailbox
//	open so that we don't prevent renaming the temporary mailbox to the
//	final name.

void
X1EmailScanner::NotifyTemporarilyCloseMailbox(
	const char *				in_szMailboxRelativePath)
{
	//	Check to see if the current mailbox is affected by comparing the
	//	path of the mailbox being used by the main scan (if any).
	if ( m_pCurrentTocDoc[sd_MainScans] &&
		 (m_szMailboxRelativePath[sd_MainScans].CompareNoCase(in_szMailboxRelativePath) == 0) )
	{
		//	Close only the current mailbox file. We'll reopen it the next
		//	we idle (if we can), by which time Eudora will be done
		//	with compaction.
		m_CurrentMailboxFile[sd_MainScans].Close();
	}

	//	Check to see if the current mailbox is affected by comparing the
	//	path of the mailbox being used by the index update scan (if any).
	if ( m_pCurrentTocDoc[sd_IndexUpdateActions] &&
		 (m_szMailboxRelativePath[sd_IndexUpdateActions].CompareNoCase(in_szMailboxRelativePath) == 0) )
	{
		//	Close only the current mailbox file. We'll reopen it the next
		//	we idle (if we can), by which time Eudora will be done
		//	with compaction.
		m_CurrentMailboxFile[sd_IndexUpdateActions].Close();
	}	
}


// ---------------------------------------------------------------------------
//		* NotifyRestartProcessingMailbox							 [Public]
// ---------------------------------------------------------------------------
//	Called when we need to restart processing a specific mailbox because it
//	has been sorted (so the summaries no longer have the same positions).

void
X1EmailScanner::NotifyRestartProcessingMailbox(
	const char *				in_szMailboxRelativePath)
{
	//	Check to see if the current mailbox for the main scan is affected.
	if ( m_pCurrentTocDoc[sd_MainScans] &&
		 (m_szMailboxRelativePath[sd_MainScans].CompareNoCase(in_szMailboxRelativePath) == 0) )
	{		
		//	Restart summary iteration
		CSumList &		listSums = m_pCurrentTocDoc[sd_MainScans]->GetSumList();
		m_posCurrentSummary[sd_MainScans] = listSums.GetHeadPosition();
		m_posLastSummaryAddAttempt[sd_MainScans] = NULL;
		m_nCurrentMessage[sd_MainScans] = 0;
	}
}


// ---------------------------------------------------------------------------
//		* NotifyStopProcessingMailbox								 [Public]
// ---------------------------------------------------------------------------
//	Called when we need to stop processing a specific mailbox because it
//	has been deleted, moved, or renamed.

void
X1EmailScanner::NotifyStopProcessingMailbox(
	const char *				in_szMailboxRelativePath)
{
	//	Check to see if the current mailbox for the main scan is affected.
	if ( m_pCurrentTocDoc[sd_MainScans] &&
		 (m_szMailboxRelativePath[sd_MainScans].CompareNoCase(in_szMailboxRelativePath) == 0) )
	{
		//	Close the current mailbox
		CloseCurrentMailbox(sd_MainScans, false);

		if (m_eScanType == st_AddOrUpdateItemsScan)
		{
			//	Move to the next mailbox
			NextMailbox();

			//	Open the mailbox
			if (m_pCurrentMailbox)
				OpenCurrentMailbox(sd_MainScans);
		}
	}

	//	Check to see if the mailbox for the index update scan is affected.
	if ( m_pCurrentTocDoc[sd_IndexUpdateActions] &&
		 (m_szMailboxRelativePath[sd_IndexUpdateActions].CompareNoCase(in_szMailboxRelativePath) == 0) )
	{
		//	Close the current mailbox. The addition of the appropriate index update
		//	action will handle removing the current index update action if appropriate.
		CloseCurrentMailbox(sd_IndexUpdateActions, false);
	}	
}


// ---------------------------------------------------------------------------
//		* InitAddOrUpdateItemsScan									 [Public]
// ---------------------------------------------------------------------------
//	Initializes mailbox iteration for add new items scan.

void
X1EmailScanner::InitAddOrUpdateItemsScan(
	const char *				in_szContinueIndexingMailbox)
{
	//	Init mailbox scan members
	m_szMailboxRelativePath[sd_MainScans].Empty();
	m_pMailboxList = g_theMailboxDirector.GetMailboxList();
	m_posCurrentMailbox = NULL;
	m_pCurrentMailbox = NULL;
	m_pCurrentTocDoc[sd_MainScans] = NULL;
	m_posCurrentSummary[sd_MainScans] = NULL;
	m_posLastSummaryAddAttempt[sd_MainScans] = NULL;
	
	//	Keep track of whether we found a mailbox with which to continue
	CString					szResumeMailboxRelativePath;
	POSITION				posResumeMailbox = NULL;
	CPtrList *				pResumeMailboxList = g_theMailboxDirector.GetMailboxList();

	ASSERT(pResumeMailboxList);
	if (pResumeMailboxList)
	{
		posResumeMailbox = pResumeMailboxList->GetHeadPosition();
		ASSERT(posResumeMailbox);
	}

	if (pResumeMailboxList && posResumeMailbox)
	{
		POSITION				posResumeMailbox = pResumeMailboxList->GetHeadPosition();
		QCMailboxCommand *		pResumeMailbox = reinterpret_cast<QCMailboxCommand *>( pResumeMailboxList->GetNext(posResumeMailbox) );

		//	Init mailbox scan to look for mailbox to continue scan
		//	and count the total number of mailboxes
		m_posCurrentMailbox = m_pMailboxList->GetHeadPosition();
		m_pCurrentMailbox = reinterpret_cast<QCMailboxCommand *>( m_pMailboxList->GetNext(m_posCurrentMailbox) );
		m_nCurrentMailbox = 0;
		m_nTotalMailboxes = 0;

		//	Get path of first mailbox, so that we can find it below
		if (m_pCurrentMailbox)
			m_szMailboxRelativePath[sd_MainScans] = static_cast<const char *>(m_pCurrentMailbox->GetPathname()) + EudoraDirLen;
		
		//	Attempt to find the mailbox that we were last scanning so that we can
		//	resume our scan with it
		while (m_pCurrentMailbox)
		{
			if ( in_szContinueIndexingMailbox && *in_szContinueIndexingMailbox &&
				 (m_szMailboxRelativePath[sd_MainScans].CompareNoCase(in_szContinueIndexingMailbox) == 0) )
			{
				//	Remember all the mailbox information for the mailbox with which
				//	we'll be resuming
				szResumeMailboxRelativePath = m_szMailboxRelativePath[sd_MainScans];
				pResumeMailboxList = m_pMailboxList;
				posResumeMailbox = m_posCurrentMailbox;
				pResumeMailbox = m_pCurrentMailbox;
				m_nCurrentMailbox = m_nTotalMailboxes;
			}
			
			NextMailbox();

			m_nTotalMailboxes++;
		}

		//	Resume with the mailbox we found or the beginning of the list
		m_szMailboxRelativePath[sd_MainScans] = szResumeMailboxRelativePath;
		m_pMailboxList = pResumeMailboxList;
		m_posCurrentMailbox = posResumeMailbox;
		m_pCurrentMailbox = pResumeMailbox;
		m_nCurrentMessage[sd_MainScans] = 0;

		ASSERT(m_pCurrentMailbox);

		//	Open the current mailbox so that we'll scan it instead of moving
		//	on to the next mailbox.
		if (m_pCurrentMailbox)
		{
			m_szMailboxRelativePath[sd_MainScans] = static_cast<const char *>(m_pCurrentMailbox->GetPathname()) + EudoraDirLen;
			OpenCurrentMailbox(sd_MainScans);
		}

		//	Do we have progress displaying yet?
		if (!m_pMainScansTaskInfo)
		{
			QCTaskManager *		pTaskManager = QCGetTaskManager();

			if (pTaskManager)
			{
				//	Add task to task manager to display progress
				m_pMainScansTaskInfo = DEBUG_NEW SearchManagerTaskInfo(true);
				pTaskManager->Register(m_pMainScansTaskInfo);
			}
		}

		if (m_pMainScansTaskInfo)
		{
			//	Set the total progress to the number of mailboxes.
			m_pMainScansTaskInfo->SetTotal(m_nTotalMailboxes * 100);

			//	Set where we're at with the progress
			m_nSoFarBase[sd_MainScans] = m_nCurrentMailbox * 100;
			m_pMainScansTaskInfo->SetSoFar( m_nSoFarBase[sd_MainScans] );

			//	Set the title
			m_pMainScansTaskInfo->SetTitle( CRString(IDS_ADD_UPDATE_SCAN_TITLE) );

			//	Set the initial main progress text
			CString		szMainText;
			szMainText.Format(IDS_ADD_UPDATE_SCAN_MAILBOXES_REMAINING, m_nTotalMailboxes - m_nCurrentMailbox);
			m_pMainScansTaskInfo->SetMainText(szMainText);
		}
	}
}


// ---------------------------------------------------------------------------
//		* InitCullingScan											 [Public]
// ---------------------------------------------------------------------------
//	Initializes mailbox iteration for culling scan.

void
X1EmailScanner::InitCullingScan(
	const char *				in_szContinueIndexingMailbox)
{
	m_pCurrentTocDoc[sd_MainScans] = NULL;
	m_szMailboxRelativePath[sd_MainScans].Empty();

	//	If we have an old list of indexed items, release it
	if (m_pX1IndexedItems.GetInterfacePtr() != NULL)
		m_pX1IndexedItems.Release();

	try
	{
		//	Search the X1 email DB for all items
		X1::IX1ItemResultPtr	pItemResult = NULL;
		m_pEmailDB->FindItems(X1::ftContainsPrefix, _bstr_t(), SearchManager::efid_MessageID, &pItemResult);

		ASSERT( pItemResult.GetInterfacePtr() );

		if (pItemResult)
		{
			//	Set up what we want the search results to be sorted by
			COleSafeArray	safeArraySortBy;
			int *			pData;

			V_VT(&safeArraySortBy) = VT_ARRAY | VT_INT;
			V_ARRAY(&safeArraySortBy) = SafeArrayCreateVector(VT_INT, 0, 2);

			//	Sort first by mailbox path, then by message ID
			safeArraySortBy.AccessData( reinterpret_cast<void **>(&pData) );
			pData[0] = SearchManager::efid_MailboxRelativePath;
			pData[1] = SearchManager::efid_MessageID;
			safeArraySortBy.UnaccessData();

			//	Set up that we want the search results to be sorted forwards
			COleSafeArray	safeArraySortOrder;

			V_VT(&safeArraySortOrder) = VT_ARRAY | VT_INT;
			V_ARRAY(&safeArraySortOrder) = SafeArrayCreateVector(VT_INT, 0, 2);

			safeArraySortOrder.AccessData( reinterpret_cast<void **>(&pData) );
			pData[0] = 0;
			pData[1] = 0;
			safeArraySortOrder.UnaccessData();

			//	Actually get the search results
			pItemResult->GetItemList(safeArraySortBy, safeArraySortOrder, &m_pX1IndexedItems);

			ASSERT( m_pX1IndexedItems.GetInterfacePtr() );

			if (m_pX1IndexedItems)
			{
				//	Get the number of items in the X1 email DB
				m_pX1IndexedItems->get_count(&m_nNumX1IndexedItems);

				//	By default we'll start the culling scan with the first indexed item
				m_nX1IndexedItem = 0;

				if (in_szContinueIndexingMailbox && *in_szContinueIndexingMailbox)
				{
					//	Attempt to find the mailbox that we were last scanning so that we can
					//	resume our scan with it
					_bstr_t		strLastMailbox;

					for (long i = 0; i < m_nNumX1IndexedItems; i++)
					{
						long		nX1ItemNum = m_pX1IndexedItems->Getitem(i);

						BOOL		bIsValidIndex = m_pEmailDB->IsValid(nX1ItemNum);

						ASSERT(bIsValidIndex);

						if (bIsValidIndex)
						{
							//	Compare native strings first to save time
							if ( strLastMailbox != m_pEmailDB->GetFieldString(SearchManager::efid_MailboxRelativePath, nX1ItemNum) )
							{
								//	Remember the native string for this mailbox
								strLastMailbox = m_pEmailDB->GetFieldString(SearchManager::efid_MailboxRelativePath, nX1ItemNum);
								
								//	Get the CString version of the mailbox path string
								CString		szMailboxRelativePath = static_cast<char *>(strLastMailbox);

								//	Did we find the correct mailbox yet?
								if (szMailboxRelativePath.CompareNoCase(in_szContinueIndexingMailbox) == 0)
								{
									//	Start with the first item for this mailbox
									m_nX1IndexedItem = i;

									//	No need to look any further
									break;
								}
							}
						}
					}
				}

				//	Do we have progress displaying yet?
				if (!m_pMainScansTaskInfo)
				{
					QCTaskManager *		pTaskManager = QCGetTaskManager();

					if (pTaskManager)
					{
						//	Add task to task manager to display progress
						m_pMainScansTaskInfo = DEBUG_NEW SearchManagerTaskInfo(true);
						pTaskManager->Register(m_pMainScansTaskInfo);
					}
				}

				if (m_pMainScansTaskInfo)
				{
					//	Set the total progress to the number of mailboxes.
					m_pMainScansTaskInfo->SetTotal(m_nNumX1IndexedItems);

					//	Set where we're at with the progress
					m_nSoFarBase[sd_MainScans] = m_nX1IndexedItem;
					m_pMainScansTaskInfo->SetSoFar( m_nSoFarBase[sd_MainScans] );

					//	Set the title
					m_pMainScansTaskInfo->SetTitle( CRString(IDS_CULLING_SCAN_TITLE) );

					//	Set the initial main progress text
					CString		szMainText;
					szMainText.Format(IDS_CULLING_SCAN_ITEMS_REMAINING, m_nNumX1IndexedItems - m_nX1IndexedItem);
					m_pMainScansTaskInfo->SetMainText(szMainText);
				}
			}
			else
			{
				PutDebugLog( DEBUG_MASK_SEARCH, "Stopping search indexing (culling) because there are no items in the index" );
				
				//	Stop the culling scan
				StopCurrentScan();
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "InitCullingScan", e.ErrorMessage() );

		//	Stop the culling scan
		StopCurrentScan();
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "InitCullingScan");

		//	Delete exception object
		pException->Delete();

		//	Stop the culling scan
		StopCurrentScan();
	}
}


// ---------------------------------------------------------------------------
//		* NextMailbox												 [Public]
// ---------------------------------------------------------------------------
//	Moves m_pCurrentMailbox to the next mailbox that could contain messages
//	in a depth first traversal of the mailbox tree.

void
X1EmailScanner::NextMailbox()
{	
	if (!m_pMailboxList || !m_pCurrentMailbox)
	{
		//	We've scanned through all mailboxes
		//	Stop the current scan
		StopCurrentScan();

		//	Return - we're done
		return;
	}

	//	Depth first traversal of mailboxes
	QCMailboxCommand *		pStartMailbox = m_pCurrentMailbox;

	while (m_pCurrentMailbox)
	{
		//	Dive deeper. Stop when:
		//	* We find a mailbox that can contain messages
		//	* We find a mailbox that doesn't have any children (leaf)
		while ( m_pCurrentMailbox && m_pCurrentMailbox->CanHaveChildren() &&
				!m_pCurrentMailbox->GetChildList().IsEmpty() )
		{
			if ( m_pCurrentMailbox && (m_pCurrentMailbox != pStartMailbox) &&
				 m_pCurrentMailbox->CanContainMessages() )
			{
				break;
			}
			
			//	Get children
			m_pMailboxList = &m_pCurrentMailbox->GetChildList();

			//	Get first child
			m_posCurrentMailbox = m_pMailboxList->GetHeadPosition();
			m_pCurrentMailbox = reinterpret_cast<QCMailboxCommand *>( m_pMailboxList->GetNext(m_posCurrentMailbox) );
		}

		//	Sanity check
		ASSERT(m_pMailboxList);
		ASSERT(m_pCurrentMailbox);

		//	Did we find a mailbox that can contain messages by diving deeper?
		//	If so, then we're done.
		if ( (m_pCurrentMailbox != pStartMailbox) && m_pCurrentMailbox->CanContainMessages() )
			break;

		//	Move to next mailbox when we can't dive deeper
		if (m_posCurrentMailbox)
		{
			//	More siblings - get the next sibling
			m_pCurrentMailbox = reinterpret_cast<QCMailboxCommand *>( m_pMailboxList->GetNext(m_posCurrentMailbox) );
		}
		else
		{
			//	No more siblings - move up the tree and find the sibling of an ancestor

			//	Move up a level - find parent of current mailbox
			m_pCurrentMailbox = g_theMailboxDirector.FindParent(m_pCurrentMailbox);
			m_posCurrentMailbox = NULL;

			while (m_pCurrentMailbox)
			{
				//	Get parent of current mailbox so that we can find a sibling
				QCMailboxCommand *	pParent = g_theMailboxDirector.FindParent(m_pCurrentMailbox);

				if (pParent)
				{
					//	Get parent's children (our siblings)
					m_pMailboxList = &pParent->GetChildList();
				}
				else
				{
					//	Assume that we're dealing with a root entry, therefore our siblings
					//	are found in the root mailbox list.
					m_pMailboxList = g_theMailboxDirector.GetMailboxList();
				}

				//	Get parent's children (our siblings)
				QCMailboxCommand *	pMailbox = NULL;
				
				m_posCurrentMailbox = m_pMailboxList->GetHeadPosition();

				//	Look for position of current mailbox in siblings
				while (m_posCurrentMailbox)
				{
					pMailbox = reinterpret_cast<QCMailboxCommand *>( m_pMailboxList->GetNext(m_posCurrentMailbox) );
					
					//	We found the current mailbox
					if (pMailbox == m_pCurrentMailbox)
						break;
				}

				//	Sanity check that we found the current mailbox
				ASSERT(pMailbox == m_pCurrentMailbox);

				if ( (pMailbox == m_pCurrentMailbox) && m_posCurrentMailbox )
				{
					//	We have another sibling - move on to it
					m_pCurrentMailbox = reinterpret_cast<QCMailboxCommand *>( m_pMailboxList->GetNext(m_posCurrentMailbox) );

					//	Break so that we can explore the sibling
					break;
				}
				else
				{
					//	No siblings - move up another level. If we were already at
					//	the root level this will terminate our traversal.
					m_pCurrentMailbox = pParent;
				}
			}
		}
	}

	//	Set up the new mailbox path
	if (m_pCurrentMailbox)
		m_szMailboxRelativePath[sd_MainScans] = static_cast<const char *>(m_pCurrentMailbox->GetPathname()) + EudoraDirLen;
	else
		m_szMailboxRelativePath[sd_MainScans].Empty();

	if (m_pMainScansTaskInfo)
	{
		//	Update the progress display
		m_nCurrentMessage[sd_MainScans] = 0;
		m_nCurrentMailbox++;
		m_nSoFarBase[sd_MainScans] = m_nCurrentMailbox * 100;
		m_pMainScansTaskInfo->SetSoFar( m_nSoFarBase[sd_MainScans] );

		//	Update main progress text
		CString		szMainText;
		szMainText.Format(IDS_ADD_UPDATE_SCAN_MAILBOXES_REMAINING, m_nTotalMailboxes - m_nCurrentMailbox);
		m_pMainScansTaskInfo->SetMainText(szMainText);
	}
}


// ---------------------------------------------------------------------------
//		* AddOrUpdateMailboxMessages								 [Public]
// ---------------------------------------------------------------------------
//	Scan to add the next item to the index.

bool
X1EmailScanner::AddOrUpdateMailboxMessages(
	ScanDataEnum				in_eScanData)
{
	//	Verify that the mailbox is open. It could be closed if we were told to
	//	close it for compaction (via NotifyTemporarilyCloseMailbox) or if we
	//	failed when we tried to open it.
	if (m_CurrentMailboxFile[in_eScanData].IsOpen() != S_OK)
	{
		CTocDoc *	pTocDoc = m_pCurrentTocDoc[in_eScanData];
		
		//	Make sure that mailbox still exists
		if ( FileExistsMT(pTocDoc->GetMBFileName()) )
		{
			//	Attempt to open or re-open mailbox - currently we never give up
			m_CurrentMailboxFile[in_eScanData].Open(pTocDoc->GetMBFileName(), O_RDONLY);
		}
		else
		{
			//	The mailbox doesn't exist anymore. Close our TocDoc, etc.
			CloseCurrentMailbox(in_eScanData);

			//	Reset m_posCurrentSummary so that we move on
			m_posCurrentSummary[in_eScanData] = NULL;
		}

		//	Return now so that we don't use up too much time
		return (m_posCurrentSummary[in_eScanData] == NULL);
	}

	try
	{
		CSumList &			listSums = m_pCurrentTocDoc[in_eScanData]->GetSumList();
		CSummary *			pSum = listSums.GetAt( m_posCurrentSummary[in_eScanData] );
		X1::IX1OneUsePtr	pOneUse;

		if (!pSum)
		{
			//	Unexpected failure. Log failure and move on so that we don't get stuck.
			ASSERT(!"Getting summary failed");
			PutDebugLog( DEBUG_MASK_SEARCH, "Failed to get summary while doing search indexing" );
			listSums.GetNext( m_posCurrentSummary[in_eScanData] );
		}
		else if ( m_pEmailDB && m_pEmailDB->CanModifyIndex(&pOneUse) )
		{
			long	nX1ItemNum = -1;
			UINT	nProgressResID;

			if ( m_bCheckingMessages[in_eScanData] )
			{
				//	Do we need to check to see if the current summary is in the index, and if so
				//	is the current summary already in the index?
				if ( IsIndexed(m_szMailboxRelativePath[in_eScanData], pSum->GetUniqueMessageId(), nX1ItemNum) )
				{
					//	Does the current summary match what's in the index?
					if ( DoesIndexedItemMatchSummary(pSum, nX1ItemNum) )
					{
						//	It matches, so we don't need to add it to the index
						pSum->SetShouldBeAddedToIndex(false);
					}
					else
					{
						//	It doesn't match - remove it from the index
						m_pEmailDB->RemoveFromIndex(pOneUse, nX1ItemNum);

						//	Set that we need to re-add it to the index
						pSum->SetShouldBeAddedToIndex(true);

						//	Remember that we need to add one or more messages
						m_bNeedToAddMessages[in_eScanData] = true;
					}
				}
				else
				{
					//	Not indexed, so remember that we need to add it to the index
					pSum->SetShouldBeAddedToIndex(true);

					//	Remember that we need to add one or more messages
					m_bNeedToAddMessages[in_eScanData] = true;
				}

				//	Move on to the next message
				m_nCurrentMessage[in_eScanData]++;
				listSums.GetNext(m_posCurrentSummary[in_eScanData]);

				//	Use the appropriate progress string
				nProgressResID = IDS_INDEXING_VERIFYING_MESSAGES_REMAINING;
			}
			else
			{
				//	If there were already some messages indexed for this mailbox,
				//	then we checked this message to see if it should be added to
				//	the index. If it shouldn't be added, then we're already done.
				bool	bDone = m_bCheckForIndexedMessages[in_eScanData] && !pSum->ShouldBeAddedToIndex();

				//	Remember whether or not this is our second add attempt
				bool	bSecondAddAttemptForSummary = ( m_posCurrentSummary[in_eScanData] == m_posLastSummaryAddAttempt[in_eScanData] );

				//	If it's not in the index, then add it to the index
				if ( !bDone && m_pEmailDB->CanModifyIndex(&pOneUse) )
				{
					bool	bWaitingForX1 = false;

					bDone = AddToIndex(in_eScanData, pOneUse, pSum, bWaitingForX1);

					if (!bDone && !bWaitingForX1)
					{
						//	Adding to the index failed and it wasn't because we were
						//	waiting for X1. We'll try once more and then move on.
						m_posLastSummaryAddAttempt[in_eScanData] = m_posCurrentSummary[in_eScanData];
					}
				}

				//	If we're done or it's our second attempt, then move on to the next summary
				if (bDone || bSecondAddAttemptForSummary)
				{
					m_nCurrentMessage[in_eScanData]++;
					listSums.GetNext(m_posCurrentSummary[in_eScanData]);

					if (!bDone)
					{
						CString		szLogEntry;

						szLogEntry.Format( "Failed to add message to index twice while doing search indexing (Subject: %s)",
											pSum->m_Subject );
						PutDebugLog(DEBUG_MASK_SEARCH, szLogEntry);
					}
				}

				//	Use the appropriate progress string
				nProgressResID = IDS_INDEXING_ADDING_MESSAGES_REMAINING;
			}

			SearchManagerTaskInfo *		pTaskInfo = NULL;
			CString						szMailboxName;

			if ( (in_eScanData == sd_MainScans) && m_pMainScansTaskInfo )
			{
				pTaskInfo = m_pMainScansTaskInfo;
				szMailboxName = m_pCurrentMailbox->GetName();
			}
			else if ( (in_eScanData == sd_IndexUpdateActions) && m_pUpdateActionsTaskInfo )
			{
				pTaskInfo = m_pUpdateActionsTaskInfo;
				szMailboxName = m_szMailboxName;
			}

			if (pTaskInfo)
			{
				//	Update progress display info
				CString		szProgress;
				long		nTotalMessages = listSums.GetCount();
				long		nMessagesRemaining = listSums.GetCount() - m_nCurrentMessage[in_eScanData];

				//	Saw a negative number once - check to see if I see it again and why
				ASSERT(nMessagesRemaining >= 0);

				//	If it went negative, just display 0
				if (nMessagesRemaining < 0)
					nMessagesRemaining = 0;

				szProgress.Format(nProgressResID, szMailboxName, nMessagesRemaining);
				pTaskInfo->Progress(szProgress);
				
				//	Update the progress bar
				long	nSoFar = m_nSoFarBase[in_eScanData] + m_nCurrentMessage[in_eScanData] * 100 / nTotalMessages;
				pTaskInfo->SetSoFar(nSoFar);

				//	Update persona
				pTaskInfo->SetPersona( pSum->GetPersona() );
			}

			if ( m_bCheckingMessages[in_eScanData] && (m_posCurrentSummary[in_eScanData] == NULL) )
			{
				//	We've finished checking the messages
				m_bCheckingMessages[in_eScanData] = false;

				if ( m_bNeedToAddMessages[in_eScanData] )
				{
					//	Scan through mailbox again, this time actually adding the messages
					m_posCurrentSummary[in_eScanData] = listSums.GetHeadPosition();
					m_posLastSummaryAddAttempt[in_eScanData] = NULL;
					m_nCurrentMessage[in_eScanData] = 0;
				}
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "AddOrUpdateMailboxMessages", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "AddOrUpdateMailboxMessages");

		//	Delete exception object
		pException->Delete();
	}

	//	Return whether or not we're done.
	return (m_posCurrentSummary[in_eScanData] == NULL);
}


// ---------------------------------------------------------------------------
//		* DoAddOrUpdateItemsScan									 [Public]
// ---------------------------------------------------------------------------
//	Scan to add the next item to the index.

void
X1EmailScanner::DoAddOrUpdateItemsScan()
{	
	if ( !m_posCurrentSummary[sd_MainScans] )
	{
		//	Close the current mailbox
		CloseCurrentMailbox(sd_MainScans);
		
		//	Move to the next mailbox
		NextMailbox();

		//	Open the mailbox
		if (m_pCurrentMailbox)
			OpenCurrentMailbox(sd_MainScans);

		//	Return now so that we don't use up too much time
		return;
	}
	
	AddOrUpdateMailboxMessages(sd_MainScans);
}


// ---------------------------------------------------------------------------
//		* DoCullingScan												 [Public]
// ---------------------------------------------------------------------------
//	Culling scan to check the next item to see if it should be removed
//	from the index.

void
X1EmailScanner::DoCullingScan()
{
	try
	{
		//	Update progress display info
		if (m_pMainScansTaskInfo)
			m_pMainScansTaskInfo->Progress( CRString(IDS_CULLING_SCAN_ACTIVE) );
		
		if (m_nX1IndexedItem >= m_nNumX1IndexedItems)
		{
			//	Hit the end of X1 indexed items - stop the culling scan
			StopCurrentScan();
		}

		if (m_nX1IndexedItem < m_nNumX1IndexedItems)
		{
			long		nX1ItemNum = m_pX1IndexedItems->Getitem(m_nX1IndexedItem);

			BOOL		bIsValidIndex = m_pEmailDB->IsValid(nX1ItemNum);

			ASSERT(bIsValidIndex);

			if (bIsValidIndex)
			{
				CString		szMailboxRelativePath = static_cast<char *>(
								m_pEmailDB->GetFieldString(SearchManager::efid_MailboxRelativePath, nX1ItemNum) );
				
				if (szMailboxRelativePath.CompareNoCase(m_szMailboxRelativePath[sd_MainScans]) != 0)
				{
					//	Close the current mailbox
					CloseCurrentMailbox(sd_MainScans);

					//	Switch to the new mailbox
					m_szMailboxRelativePath[sd_MainScans] = szMailboxRelativePath;

					//	Open the new mailbox
					OpenCurrentMailbox(sd_MainScans);

					//	Don't move on to the next X1 indexed item so that we can process
					//	it the next Idle.
				}
				else
				{
					CString		szMessageID = static_cast<char *>(
									m_pEmailDB->GetFieldString(SearchManager::efid_MessageID, nX1ItemNum) );
					long		nMessageID = atol(szMessageID);
					
					//	In case of errors with the mailbox file:
					//	* If the mailbox file doesn't exist - remove the item
					//	* If the mailbox file exists, but we couldn't open the TOC doc - leave
					//	  the item alone and move on to the next item
					bool		bRemoveItem = false;

					if (!m_bMailboxFileExists)
					{
						//	The mailbox file for the current item doesn't even exist - remove the item
						bRemoveItem = true;
					}
					else if (m_pCurrentTocDoc[sd_MainScans])
					{
						//	Check to see if we should remove the current item
						CSumList &	listSums = m_pCurrentTocDoc[sd_MainScans]->GetSumList();
						CSummary *	pSummary = listSums.GetByMessageId(nMessageID);

						//	If we couldn't find the summary in the specified mailbox,
						//	then remove the item
						if (!pSummary)
							bRemoveItem = true;
					}

					if (bRemoveItem)
					{
						//	Queue action to remove the item
						SearchManager::Instance()->AddIndexUpdateAction(
								SearchManager::ua_RemoveMessage,
								szMailboxRelativePath,
								nMessageID );

						//	We don't call NotifyIndexingStatus or attempt to trigger saving
						//	the SearchManager actions here, because they'll be recreated
						//	if we crash since we pick up at the start of the most recent mailbox
						//	being scanned. NotifyIndexingStatus is called when we switch to the
						//	next mailbox, which suits us just fine.
					}
					
					//	Move on to the next X1 indexed item
					m_nX1IndexedItem++;
				}
			}
			else
			{
				//	Move on to the next X1 indexed item
				m_nX1IndexedItem++;
			}
		}

		if (m_pMainScansTaskInfo)
		{
			//	Set where we're at with the progress
			m_nSoFarBase[sd_MainScans] = m_nX1IndexedItem;
			m_pMainScansTaskInfo->SetSoFar( m_nSoFarBase[sd_MainScans] );

			//	Set the initial main progress text
			CString		szMainText;
			szMainText.Format(IDS_CULLING_SCAN_ITEMS_REMAINING, m_nNumX1IndexedItems - m_nX1IndexedItem);
			m_pMainScansTaskInfo->SetMainText(szMainText);
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "DoCullingScan", e.ErrorMessage() );

		//	Stop the culling scan
		StopCurrentScan();
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "DoCullingScan");

		//	Delete exception object
		pException->Delete();

		//	Stop the culling scan
		StopCurrentScan();
	}
}


// ---------------------------------------------------------------------------
//		* ProcessNextIndexUpdateItem								 [Public]
// ---------------------------------------------------------------------------
//	Processes next index update item.

void
X1EmailScanner::ProcessNextIndexUpdateItem()
{
	try
	{
		X1::IX1OneUsePtr	pOneUse;

		if ( m_pEmailDB && m_pEmailDB->CanModifyIndex(&pOneUse) )
		{
			bool					bDoneProcessingCurrentAction = true;
			
			//	Get the next index update action
			IndexUpdateAction *		pUpdateAction = SearchManager::Instance()->GetFirstIndexUpdateAction();

			ASSERT(pUpdateAction);

			if (pUpdateAction)
			{
				switch ( pUpdateAction->GetUpdateAction() )
				{
					case SearchManager::ua_RemoveMessage:
					case SearchManager::ua_UpdateMessage:
						bDoneProcessingCurrentAction = ProcessRemoveOrUpdateMessage(pUpdateAction, pOneUse);
						break;

					case SearchManager::ua_AddMessage:
						bDoneProcessingCurrentAction = ProcessAddMessage(pUpdateAction, pOneUse);
						break;

					case SearchManager::ua_RemoveMailbox:
					case SearchManager::ua_ReindexMailbox:
						bDoneProcessingCurrentAction = ProcessRemoveOrReindexMailbox(pUpdateAction, pOneUse);
						break;

					case SearchManager::ua_AddMailbox:
						bDoneProcessingCurrentAction = ProcessAddMailbox(pUpdateAction, pOneUse);
						break;
				}
			}

			if (bDoneProcessingCurrentAction)
			{
				//	Done processing index update action so pop it
				SearchManager::Instance()->PopIndexUpdateAction();

				if (m_pUpdateActionsTaskInfo)
				{
					m_nCurrentUpdateAction++;
					m_nSoFarBase[sd_IndexUpdateActions] = m_nCurrentUpdateAction * 100;
					m_pUpdateActionsTaskInfo->SetSoFar( m_nSoFarBase[sd_IndexUpdateActions] );
					m_nTotalMessagesToRemove = 0;
				}
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "ProcessNextIndexUpdateItem", e.ErrorMessage() );

		//	We had an error while processing the current index update item.
		//	Move on to the next item so that we don't get stuck trying to
		//	process the current item. I don't like this at all, but I'm not
		//	sure how else to avoid getting stuck.
		SearchManager::Instance()->PopIndexUpdateAction();
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "ProcessNextIndexUpdateItem");

		//	Delete exception object
		pException->Delete();

		//	We had an error while processing the current index update item.
		//	Move on to the next item so that we don't get stuck trying to
		//	process the current item. I don't like this at all, but I'm not
		//	sure how else to avoid getting stuck.
		SearchManager::Instance()->PopIndexUpdateAction();
	}
}


// ---------------------------------------------------------------------------
//		* ProcessRemoveOrUpdateMessage								 [Public]
// ---------------------------------------------------------------------------
//	Processes removal or update of a message.

bool
X1EmailScanner::ProcessRemoveOrUpdateMessage(
	X1EmailScanner::IndexUpdateAction *			in_pUpdateAction,
	X1::IX1OneUsePtr &							in_pOneUse)
{
	UpdateActionEnum	eUpdateAction = in_pUpdateAction->GetUpdateAction();
	bool				bDoneProcessingCurrentAction = true;
	
	//	Update starts functioning just like a remove - it removes
	//	the message from the index. After it's done removing, however,
	//	it transforms itself into an add (see below).
	//
	//	Remove searches for the specified message and removes it from
	//	the index if found.
	long	nX1ItemNum = -1;

	//	Is the specified message still in the index?
	if ( IsIndexed(in_pUpdateAction->GetMailboxRelativePath(), in_pUpdateAction->GetMessageID(), nX1ItemNum) )
	{
		//	Remove it from the index
		m_pEmailDB->RemoveFromIndex(in_pOneUse, nX1ItemNum);
	}

	if (eUpdateAction == SearchManager::ua_UpdateMessage)
	{
		//	We removed the outdated indexed information for the message.
		//	In the next idle we'll want to add the updated information
		//	for the message.
		in_pUpdateAction->SetUpdateAction(SearchManager::ua_AddMessage);
		bDoneProcessingCurrentAction = false;
		
		//	Remember that this was an update so that we can display a
		//	better progress message.
		in_pUpdateAction->SetIsUpdate(true);
	}
	else if (m_pUpdateActionsTaskInfo)
	{
		//	Display progress for removing
		m_pUpdateActionsTaskInfo->Progress( CRString(IDS_UPDATING_INDEX_PROGRESS_REMOVING_MESSAGE) );
	}

	return bDoneProcessingCurrentAction;
}


// ---------------------------------------------------------------------------
//		* ProcessAddMessage											 [Public]
// ---------------------------------------------------------------------------
//	Processes message addition.

bool
X1EmailScanner::ProcessAddMessage(
	X1EmailScanner::IndexUpdateAction *			in_pUpdateAction,
	X1::IX1OneUsePtr &							in_pOneUse)
{
	const CString &		szMailboxRelativePath = in_pUpdateAction->GetMailboxRelativePath();
	bool				bDoneProcessingCurrentAction = true;

	if (szMailboxRelativePath.CompareNoCase(m_szMailboxRelativePath[sd_IndexUpdateActions]) != 0)
	{
		//	Mailbox for the current action is different than mailbox
		//	for the previous action. Close the current mailbox,
		//	move on to the next mailbox and open it.
		CloseCurrentMailbox(sd_IndexUpdateActions);
		m_szMailboxRelativePath[sd_IndexUpdateActions] = szMailboxRelativePath;
		OpenCurrentMailbox(sd_IndexUpdateActions);

		//	We'll actually process this action the next time we're
		//	idled. For now we're not done.
		bDoneProcessingCurrentAction = false;

		//	Reset the last message ID that we attempted to add to 0
		m_nMessageIDLastAddAttempt = 0;
	}
	else if (m_CurrentMailboxFile[sd_IndexUpdateActions].IsOpen() != S_OK)
	{
		//	Mailbox file isn't open. It could be closed if we were told to close
		//	it in order for compaction to work (via NotifyTemporarilyCloseMailbox)
		//	or if opening it failed previously.
		CTocDoc *	pTocDoc = m_pCurrentTocDoc[sd_IndexUpdateActions];
		
		//	Make sure that mailbox file still exists
		if ( pTocDoc && FileExistsMT(pTocDoc->GetMBFileName()) )
		{
			//	Attempt to open or re-open mailbox - currently we never give up
			m_CurrentMailboxFile[sd_IndexUpdateActions].Open(pTocDoc->GetMBFileName(), O_RDONLY);
		}
		else
		{
			//	The mailbox doesn't exist anymore. Close our TocDoc, etc.
			CloseCurrentMailbox(sd_IndexUpdateActions);
		}
	}
	else
	{
		//	We already opened (or attempted to open) the mailbox for
		//	the current action. Now attempt to process the addition.
		CString			szProgress;

		//	Check to see if we were able to open the current mailbox
		if ( m_bMailboxFileExists[sd_IndexUpdateActions] &&
			 m_pCurrentTocDoc[sd_IndexUpdateActions] )
		{
			//	OK we have an open mailbox, we'll want to get the summary
			CSummary *		pSum = NULL;

			//	Check to see that we have a good message ID first
			long			nCurrentMessageID = in_pUpdateAction->GetMessageID();
			ASSERT(nCurrentMessageID != 0);
			if (nCurrentMessageID != 0)
			{
				//	Get the summary by message ID
				CSumList &	listSums = m_pCurrentTocDoc[sd_IndexUpdateActions]->GetSumList();
				pSum = listSums.GetByMessageId(nCurrentMessageID);

				//	Verify that we found the summary
				ASSERT(pSum);
			}

			//	Check to see if we had a good ID and found the summary
			if (pSum)
			{
				if (m_pUpdateActionsTaskInfo)
				{
					//	Display add or update progress
					UINT		nStringFormatID = in_pUpdateAction->IsUpdate() ?
												  IDS_UPDATING_INDEX_PROGRESS_UPDATING_MESSAGE :
												  IDS_UPDATING_INDEX_PROGRESS_ADDING_MESSAGE;

					szProgress.Format( nStringFormatID, pSum->MakeTitle() );

					m_pUpdateActionsTaskInfo->Progress(szProgress);

					//	Update persona
					m_pUpdateActionsTaskInfo->SetPersona( pSum->GetPersona() );
				}

				bool		bRepeatingMessage = (m_nMessageIDLastAddAttempt == nCurrentMessageID);
				bool		bWaitingForX1 = false;
				
				//	Attempt to add the message to the index
				bDoneProcessingCurrentAction = AddToIndex(sd_IndexUpdateActions, in_pOneUse, pSum, bWaitingForX1);

				//	If we're not waiting for X1 and we're repeating a message
				//	something is going wrong when we try to add it. Continue
				//	on to the next message.
				if (!bWaitingForX1 && bRepeatingMessage)
					bDoneProcessingCurrentAction = true;

				//	Remember the last message that we attempted to add
				m_nMessageIDLastAddAttempt = nCurrentMessageID;
			}
			else if (m_pUpdateActionsTaskInfo)
			{
				//	Display progress about missing message
				szProgress.LoadString(IDS_SKIPPING_UPDATE_MISSING_MESSAGE);
				m_pUpdateActionsTaskInfo->Progress(szProgress);
			}
		}
		else if (m_pUpdateActionsTaskInfo)
		{
			//	Display progress about missing mailbox
			szProgress.Format( IDS_SKIPPING_UPDATE_MISSING_MAILBOX, m_szMailboxRelativePath[sd_IndexUpdateActions] );
			m_pUpdateActionsTaskInfo->Progress(szProgress);
		}						
	}

	return bDoneProcessingCurrentAction;
}


// ---------------------------------------------------------------------------
//		* ProcessRemoveOrReindexMailbox								 [Public]
// ---------------------------------------------------------------------------
//	Processes mailbox removal.

bool
X1EmailScanner::ProcessRemoveOrReindexMailbox(
	X1EmailScanner::IndexUpdateAction *			in_pUpdateAction,
	X1::IX1OneUsePtr &							in_pOneUse)
{
	bool				bDoneProcessingCurrentAction = true;
	
	//	Close any mailbox left open after processing a previous action
	if ( m_pCurrentTocDoc[sd_IndexUpdateActions] )
		CloseCurrentMailbox(sd_IndexUpdateActions);

	//	Are there indexed messages for this mailbox?
	X1::IX1ItemListPtr		pMailboxItemList;
	bDoneProcessingCurrentAction = !HasIndexedMessages( in_pUpdateAction->GetMailboxRelativePath(), pMailboxItemList );

	if (!bDoneProcessingCurrentAction)
	{
		long	nCount = 0;

		//	Determine how many indexed messages we found for the given mailbox
		pMailboxItemList->get_count(&nCount);

		//	If we just started removing items from this mailbox, remember the
		//	total number of messages to remove.
		if ( (m_nTotalMessagesToRemove == 0) && (nCount != 0) )
			m_nTotalMessagesToRemove = nCount;

		//	Should have found messages, but HasIndexedMessages assumes true
		//	on failure, so make sure.
		ASSERT(nCount);
		if (nCount > 0)
		{
			//	Remove the first message that we found that isn't removed or invalid.
			//
			//	WARNING: By looping here it is possible that we could process longer
			//			 than we should during idle. So far items that are removed
			//			 or invalid appearing in the search results seem to be a rare
			//			 aberration that occasionally blocked us from completing
			//			 correctly (since we would call RemoveFromIndex every time
			//			 we're called with the same item). If this turns out to be a
			//			 more common situation, then this loop will need to be factored
			//			 out to work correctly in a more time sliced manner.
			long	nItemIndex = 0;
			BOOL	bIsRemovedOrInvalid = FALSE;

			do
			{
				long	nX1ItemNum = pMailboxItemList->Getitem(nItemIndex);

				bIsRemovedOrInvalid = m_pEmailDB->IsRemovedOrInvalid(nX1ItemNum);

				if (!bIsRemovedOrInvalid)
					m_pEmailDB->RemoveFromIndex(in_pOneUse, nX1ItemNum);

				nItemIndex++;
			} while( bIsRemovedOrInvalid && (nItemIndex < nCount) );

			//	We're done processing the current action if we hit the end of the
			//	messages in the mailbox and didn't find any that we're not removed
			//	or invalid.
			if (nItemIndex == nCount)
				bDoneProcessingCurrentAction;
		}
		else
		{
			//	We're done processing the current action if we didn't find any messages
			bDoneProcessingCurrentAction = true;
		}

		if (m_pUpdateActionsTaskInfo)
		{
			//	Display progress about number of messages remaining to be removed
			CString			szProgress;
			szProgress.Format( IDS_UPDATING_INDEX_PROGRESS_REMOVING_MAILBOX,
							   nCount, in_pUpdateAction->GetMailboxRelativePath() );
			m_pUpdateActionsTaskInfo->Progress(szProgress);

			int		nSoFar = m_nSoFarBase[sd_IndexUpdateActions] + (m_nTotalMessagesToRemove - nCount) * 100 / m_nTotalMessagesToRemove;
			m_pUpdateActionsTaskInfo->SetSoFar(nSoFar);
		}
	}

	if ( bDoneProcessingCurrentAction &&
		 (in_pUpdateAction->GetUpdateAction() == SearchManager::ua_ReindexMailbox) )
	{
		//	We're done with the removal phase of a reindex mailbox action.
		//	Transform ourselves into an add mailbox action.
		in_pUpdateAction->SetUpdateAction(SearchManager::ua_AddMailbox);
		bDoneProcessingCurrentAction = false;
	}

	return bDoneProcessingCurrentAction;
}


// ---------------------------------------------------------------------------
//		* ProcessAddMailbox											 [Public]
// ---------------------------------------------------------------------------
//	Processes mailbox addition.

bool
X1EmailScanner::ProcessAddMailbox(
	X1EmailScanner::IndexUpdateAction *			in_pUpdateAction,
	X1::IX1OneUsePtr &							in_pOneUse)
{
	bool				bDoneProcessingCurrentAction = true;
	const CString &		szMailboxRelativePath = in_pUpdateAction->GetMailboxRelativePath();
	
	if ( !m_pCurrentTocDoc[sd_IndexUpdateActions]  ||
		 (szMailboxRelativePath.CompareNoCase(m_szMailboxRelativePath[sd_IndexUpdateActions]) != 0) )
	{
		//	Current mailbox is not open yet (either no open mailbox or previous mailbox
		//	is still open). Close the previous mailbox (if any), move to the current
		//	mailbox and open it.
		CloseCurrentMailbox(sd_IndexUpdateActions);
		m_szMailboxRelativePath[sd_IndexUpdateActions] = szMailboxRelativePath;
		OpenCurrentMailbox(sd_IndexUpdateActions);

		CString		szMailboxFullPath = EudoraDir + szMailboxRelativePath;

		//	Get the mailbox name for progress display
		QCMailboxCommand *	pMailbox = g_theMailboxDirector.FindByPathname(szMailboxFullPath);
		if (pMailbox)
			m_szMailboxName = pMailbox->GetName();
		else
			m_szMailboxName.Empty();

		//	We're done for good if the mailbox file does not exist or if it's empty.
		bDoneProcessingCurrentAction = !m_bMailboxFileExists[sd_IndexUpdateActions] ||
									   (m_posCurrentSummary[sd_IndexUpdateActions] == NULL);
	}
	else
	{
		//	Process adding mailbox
		bDoneProcessingCurrentAction = AddOrUpdateMailboxMessages(sd_IndexUpdateActions);
	}

	return bDoneProcessingCurrentAction;
}



// ---------------------------------------------------------------------------
//		* OpenCurrentMailbox										 [Public]
// ---------------------------------------------------------------------------
//	Opens the current mailbox TOC and MBX.

void
X1EmailScanner::OpenCurrentMailbox(
	ScanDataEnum				in_eScanData)
{
	//	If we're currently doing a main scan (i.e. add/update or culling), then
	//	notify Search Manager that we're moving on to the next mailbox.
	if (in_eScanData == sd_MainScans)
		SearchManager::Instance()->NotifyIndexingStatus(m_eScanType, m_eNextScanType, m_szMailboxRelativePath[in_eScanData]);
	
	CString			szMailboxFullPath = EudoraDir + m_szMailboxRelativePath[in_eScanData];
	
	//	Check to see if the mailbox exists
	m_bMailboxFileExists[in_eScanData] = FileExistsMT(szMailboxFullPath);
	
	if ( !m_bMailboxFileExists[in_eScanData] )
	{
		if (m_eScanType == st_AddOrUpdateItemsScan)
		{
			ASSERT(!"Error mailbox file does not exists!");

			CString		szLogEntry;
			szLogEntry.Format( "Mailbox file \"%s\" was missing during search indexing",
							   m_pCurrentMailbox->GetPathname() );
			PutDebugLog(DEBUG_MASK_SEARCH, szLogEntry);
		}

		return;
	}

	const char *	szMailboxName = NULL;

	if ( (in_eScanData == sd_MainScans) && (m_eScanType == st_AddOrUpdateItemsScan) )
		szMailboxName = m_pCurrentMailbox->GetName();
	
	//	Open TocDoc
	m_pCurrentTocDoc[in_eScanData] = GetToc(szMailboxFullPath, szMailboxName);

	ASSERT( m_pCurrentTocDoc[in_eScanData] );

	if ( m_pCurrentTocDoc[in_eScanData] )
	{
		CTocDoc *	pTocDoc = m_pCurrentTocDoc[in_eScanData];

		//	Tell TocDoc not to auto-delete
		pTocDoc->IncrementPreventAutoDelete();

		//	Only need to open the mailbox file if we could call AddToIndex.
		//	We only call AddToIndex when we're doing an add/update scan or an index
		//	update actions scan (i.e. specifically not when doing a culling scan).
		if ( (in_eScanData == sd_IndexUpdateActions) || (m_eScanType == st_AddOrUpdateItemsScan) )
		{
			//	Open the mailbox file
			m_CurrentMailboxFile[in_eScanData].Open(pTocDoc->GetMBFileName(), O_RDONLY);
		}

		//	Prepare for possible summary iteration
		CSumList &	listSums = pTocDoc->GetSumList();
		m_posCurrentSummary[in_eScanData] = listSums.GetHeadPosition();
		m_posLastSummaryAddAttempt[in_eScanData] = NULL;
		m_nCurrentMessage[in_eScanData] = 0;

		X1::IX1ItemListPtr		pMailboxItemList;
		
		//	Have any messages been indexed for this mailbox yet?
		//	If yes, we'll need to check each message to see if it's already
		//	in the index. If not, we can optimize our initial indexing and
		//	skip checking for each message.
		m_bCheckForIndexedMessages[in_eScanData] = HasIndexedMessages( m_szMailboxRelativePath[in_eScanData], pMailboxItemList );

		m_bCheckingMessages[in_eScanData] = m_bCheckForIndexedMessages[in_eScanData];
		m_bNeedToAddMessages[in_eScanData] = false;
	}
}


// ---------------------------------------------------------------------------
//		* CloseCurrentMailbox										 [Public]
// ---------------------------------------------------------------------------
//	Closes the current mailbox TOC and MBX.

void
X1EmailScanner::CloseCurrentMailbox(
	ScanDataEnum				in_eScanData,
	bool						in_bOkToCloseTOC)
{	
	if ( m_pCurrentTocDoc[in_eScanData] )
	{
		CTocDoc *	pTocDoc = m_pCurrentTocDoc[in_eScanData];

		//	Give our go ahead for TocDoc to auto-delete again
		pTocDoc->DecrementPreventAutoDelete();

		//	If caller says it's ok to close the TOC (i.e. caller is not aware
		//	of any outstanding CTocDoc *'s), then close the TocDoc if it is
		//	not is use by any views.
		if (in_bOkToCloseTOC)
			CloseTOCIfNotInUse(pTocDoc);

		//	Close the mailbox (we only open it when we have a Toc Doc, so
		//	checking for a Toc Doc before closing is reasonable)
		m_CurrentMailboxFile[in_eScanData].Close();

		//	Done with this Toc Doc
		m_pCurrentTocDoc[in_eScanData] = NULL;
	}

	//	Empty mailbox path
	m_szMailboxRelativePath[in_eScanData].Empty();
}


// ---------------------------------------------------------------------------
//		* HasIndexedMessages										 [Public]
// ---------------------------------------------------------------------------
//	Determines whether or not the specified mailbox has been indexed at all
//	yet. Mailbox is specified by path.

bool
X1EmailScanner::HasIndexedMessages(
	const char *				in_szMailboxRelativePath,
	X1::IX1ItemListPtr &		out_pMailboxItemList)
{
	//	Assume that mailbox has indexed messages, so that if anything goes wrong
	//	we check each message we add before adding it (to avoid duplicates).
	bool				bHasIndexedMessages = true;

	//	Get the X1 email database and search object
	X1::IX1Search *		pSearch = SearchManager::Instance()->GetX1Search();

	ASSERT(pSearch);

	if (m_pEmailDB && pSearch)
	{
		try
		{
			//	Find matching mailbox
			X1::IX1ItemResultPtr		pMailboxResult;
			CString						szMailboxPathWithRootTag = SearchManager::kMailboxRelativePathRootTag;
			szMailboxPathWithRootTag += in_szMailboxRelativePath;
			_bstr_t						bstrSearchText(szMailboxPathWithRootTag);
	
			m_pEmailDB->FindItems( X1::ftContainsExact, bstrSearchText,
								   SearchManager::efid_MailboxRelativePath, &pMailboxResult );

			ASSERT( pMailboxResult.GetInterfacePtr() );

			if (pMailboxResult)
			{
				//	Set up what we want the search results to be sorted by
				COleSafeArray	safeArraySortBy;
				int *			pData;

				V_VT(&safeArraySortBy) = VT_ARRAY | VT_INT;
				V_ARRAY(&safeArraySortBy) = SafeArrayCreateVector(VT_INT, 0, 1);

				safeArraySortBy.AccessData( reinterpret_cast<void **>(&pData) );
				pData[0] = -1;
				safeArraySortBy.UnaccessData();

				//	Set up that we want the search results to be sorted forwards
				COleSafeArray	safeArraySortOrder;

				V_VT(&safeArraySortOrder) = VT_ARRAY | VT_INT;
				V_ARRAY(&safeArraySortOrder) = SafeArrayCreateVector(VT_INT, 0, 1);

				safeArraySortOrder.AccessData( reinterpret_cast<void **>(&pData) );
				pData[0] = 0;
				safeArraySortOrder.UnaccessData();

				//	Actually get the search results
				pMailboxResult->GetItemList(safeArraySortBy, safeArraySortOrder, &out_pMailboxItemList);

				ASSERT( out_pMailboxItemList.GetInterfacePtr() );

				if (out_pMailboxItemList)
				{
					//	Determine how many search results we found
					long	nCount = 0;
					out_pMailboxItemList->get_count(&nCount);

					bHasIndexedMessages = (nCount > 0);
				}
			}
		}
		catch (_com_error & e)
		{
			//	Get developer's attention
			ASSERT(!"X1 COM Error");

			SearchManager::Utils::LogError( "COM", "HasIndexedMessages", e.ErrorMessage() );
		}
		catch (CException * pException)
		{
			//	Get developer's attention
			ASSERT(!"X1 CException");
			
			//	Log error
			SearchManager::Utils::LogCException(pException, "HasIndexedMessages");

			//	Delete exception object
			pException->Delete();
		}
	}
	
	return bHasIndexedMessages;
}


// ---------------------------------------------------------------------------
//		* IsIndexed													 [Public]
// ---------------------------------------------------------------------------
//	Determines whether or not the specified message is already in the X1
//	index. Message is specified by mailbox path and message ID.

bool
X1EmailScanner::IsIndexed(
	const char *				in_szMailboxRelativePath,
	long						in_nUniqueMessageID,
	long &						out_nX1ItemNum)
{
	//	Assume that we found something, so that if anything goes wrong
	//	we don't re-add any items.
	long				nCount = 1;

	//	Get the X1 email database and search object
	X1::IX1Search *		pSearch = SearchManager::Instance()->GetX1Search();

	ASSERT(pSearch);

	if (m_pEmailDB && pSearch)
	{
		try
		{
			//	Find matching mailbox
			X1::IX1ItemResultPtr		pMailboxResult;
			CString						szMailboxPathWithRootTag = SearchManager::kMailboxRelativePathRootTag;
			szMailboxPathWithRootTag += in_szMailboxRelativePath;
			_bstr_t						bstrSearchText(szMailboxPathWithRootTag);
	
			m_pEmailDB->FindItems( X1::ftContainsExact, bstrSearchText,
								   SearchManager::efid_MailboxRelativePath, &pMailboxResult );

			//	Find matching message IDs (in any mailbox)
			X1::IX1ItemResultPtr		pMessageIDResult;
			CString						szUniqueMessageID;

			szUniqueMessageID.Format("%d", in_nUniqueMessageID);
			bstrSearchText = szUniqueMessageID;
			m_pEmailDB->FindItems( X1::ftContainsExact, bstrSearchText,
								   SearchManager::efid_MessageID, &pMessageIDResult );

			//	Combine results to find unique result 
			X1::IX1ItemResultPtr		pItemResult = NULL;
			pMailboxResult->OpAnd(pMessageIDResult, &pItemResult);

			ASSERT( pItemResult.GetInterfacePtr() );

			if (pItemResult)
			{
				//	Set up what we want the search results to be sorted by
				COleSafeArray	safeArraySortBy;
				int *			pData;

				V_VT(&safeArraySortBy) = VT_ARRAY | VT_INT;
				V_ARRAY(&safeArraySortBy) = SafeArrayCreateVector(VT_INT, 0, 1);

				safeArraySortBy.AccessData( reinterpret_cast<void **>(&pData) );
				pData[0] = -1;
				safeArraySortBy.UnaccessData();

				//	Set up that we want the search results to be sorted forwards
				COleSafeArray	safeArraySortOrder;

				V_VT(&safeArraySortOrder) = VT_ARRAY | VT_INT;
				V_ARRAY(&safeArraySortOrder) = SafeArrayCreateVector(VT_INT, 0, 1);

				safeArraySortOrder.AccessData( reinterpret_cast<void **>(&pData) );
				pData[0] = 0;
				safeArraySortOrder.UnaccessData();

				//	Actually get the search results
				X1::IX1ItemListPtr		pItemList = NULL;
				pItemResult->GetItemList(safeArraySortBy, safeArraySortOrder, &pItemList);

				ASSERT( pItemList.GetInterfacePtr() );

				if (pItemList)
				{
					//	Determine how many search results we found
					pItemList->get_count(&nCount);
					
					//	If we have at least one result return the item num for the first one
					if (nCount > 0)
						out_nX1ItemNum = pItemList->Getitem(0);
				}
			}
		}
		catch (_com_error & e)
		{
			//	Get developer's attention
			ASSERT(!"X1 COM Error");

			SearchManager::Utils::LogError( "COM", "IsIndexed", e.ErrorMessage() );
		}
		catch (CException * pException)
		{
			//	Get developer's attention
			ASSERT(!"X1 CException");
			
			//	Log error
			SearchManager::Utils::LogCException(pException, "IsIndexed");

			//	Delete exception object
			pException->Delete();
		}
	}

	//	We intended to uniquely identify an item. We don't expect to find more
	//	than one result.
	ASSERT(nCount < 2);
	
	return (nCount >= 1);
}


// ---------------------------------------------------------------------------
//		* DoesIndexedItemMatchSummary								 [Public]
// ---------------------------------------------------------------------------
//	Determines whether or not the specified summary is already in the X1
//	index.

bool
X1EmailScanner::DoesIndexedItemMatchSummary(
	CSummary *					in_pSummary,
	long						in_nX1ItemNum)
{
	bool	bMatches = true;
	
	try
	{
		//	We compare all the information that we can easily retrieve from the
		//	summary object. To save time, we do not compare everything. In
		//	particular we do not load the body of the message.
		//
		//	We compare those values that are most likely to change in every day
		//	use (marking item read, recalculating junk score, editing the subject,
		//	etc.). Hopefully pencil edit changes of the body will be noticed because
		//	the size will no longer match.
		CString		szIndexedValue;
		int			nIndexedValue;

		//	Does subject match? Compare case insensitively.
		szIndexedValue = static_cast<char *>( m_pEmailDB->GetFieldString(SearchManager::efid_Subject, in_nX1ItemNum) );
		bMatches = (szIndexedValue.CompareNoCase(in_pSummary->m_Subject) == 0);

		if (bMatches)
		{
			//	Does label match?
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_Label, in_nX1ItemNum) );
			bMatches = (in_pSummary->m_Label == nIndexedValue);
		}

		if (bMatches)
		{
			//	Does the state match?
			//	We support indexing every state except spooled
			short	nState = (in_pSummary->m_State != MS_SPOOLED) ? in_pSummary->m_State : 0;
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_State, in_nX1ItemNum) );
			bMatches = (nState == nIndexedValue);
		}

		if (bMatches)
		{
			//	Does the mood score match?
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_Mood, in_nX1ItemNum) );
			bMatches = (in_pSummary->m_nMood == nIndexedValue);
		}
		
		if (bMatches)
		{
			//	Does the priority number match?
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_PriorityNumber, in_nX1ItemNum) );
			bMatches = (in_pSummary->m_Priority == nIndexedValue);
		}

		if (bMatches)
		{
			//	Does the junk score match?
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_JunkScoreNumber, in_nX1ItemNum) );
			bMatches = (in_pSummary->m_ucJunkScore == nIndexedValue);
		}

		if (bMatches)
		{
			//	Does the size match?
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_SizeNumber, in_nX1ItemNum) );
			bMatches = (in_pSummary->GetSize() == nIndexedValue);
		}

		if ( bMatches && in_pSummary->m_TheToc && in_pSummary->m_TheToc->IsImapToc() )
		{
			//	Do the IMAP flags match?
			nIndexedValue = atoi( m_pEmailDB->GetFieldString(SearchManager::efid_ImapFlags, in_nX1ItemNum) );
			bMatches = ( static_cast<int>(in_pSummary->m_Imflags & IMFLAGS_DELETED) == nIndexedValue );
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "DoesIndexedItemMatchSummary", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "DoesIndexedItemMatchSummary");

		//	Delete exception object
		pException->Delete();
	}

	return bMatches;
}


// ---------------------------------------------------------------------------
//		* AddToIndex												 [Public]
// ---------------------------------------------------------------------------
//	Attempts to add specified summary to X1 index.

bool
X1EmailScanner::AddToIndex(
	ScanDataEnum				in_eScanData,
	X1::IX1OneUse *				in_pOneUse,
	CSummary *					in_pSummary,
	bool &						out_bWaitingForX1)
{
	bool	bAddedToIndex = false;

	//	Assume that we're not waiting for X1
	out_bWaitingForX1 = false;

	//	Enforce pre-conditions - we must have a valid toc doc and summary right now
	ASSERT( m_pCurrentTocDoc[in_eScanData] && in_pSummary );
	if ( !m_pCurrentTocDoc[in_eScanData] || !in_pSummary )
		return false;

	try
	{
		char *		pBuffer = NULL;
		bool		bAddToIndex = true;
		
		if ( !m_pCurrentTocDoc[in_eScanData]->IsImapToc() || in_pSummary->IsIMAPMessageBodyDownloaded() )
		{
			//	Assume failure
			bAddToIndex = false;
			HRESULT		hr = m_CurrentMailboxFile[in_eScanData].Seek(in_pSummary->m_Offset, SEEK_SET);

			if ( SUCCEEDED(hr) )
			{
				pBuffer = m_szMessageBuffer.GetBuffer(in_pSummary->m_Length);
				
				hr = m_CurrentMailboxFile[in_eScanData].Read(pBuffer, in_pSummary->m_Length);

				m_szMessageBuffer.ReleaseBuffer(in_pSummary->m_Length);

				//	Allow adding to index to continue
				bAddToIndex = SUCCEEDED(hr);
			}
		}

		if (bAddToIndex)
		{
			CComPtr<X1EmailIndexItem>	pIndexItem = DEBUG_NEW X1EmailIndexItem(in_pSummary, pBuffer);

			bAddedToIndex = (m_pEmailDB->AddToIndex(in_pOneUse, pIndexItem, FALSE) == X1::iarKeptAll);

			//	Make sure that we remember that this message does not need to be added
			//	to the index. Particularly important if we interrupted other indexing
			//	of the mailbox that contains this message.
			if (bAddedToIndex)
				in_pSummary->SetShouldBeAddedToIndex(false);

			//	If we didn't add it to the index it was because we're waiting for X1
			out_bWaitingForX1 = !bAddedToIndex;
		}

		//	Done with message buffer
		m_szMessageBuffer.Empty();
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "AddToIndex", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "AddToIndex");

		//	Delete exception object
		pException->Delete();
	}
	
	return bAddedToIndex;	
}
