// ImapFiltersd.cpp
//
// Contains CImapFilterActions, a subclass of CFilterActions that provides extra functions for
// performing server queries for filter matches and functions for performing IMAP specific
// filter action operations.
//
// When using this subclass call DoFiltering() or DoManualFiltering() rather than FilterOne().
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

#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#include <mmsystem.h>
#endif

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "mdichild.h"
#include "fileutil.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "tocview.h"
#include "controls.h"
#include "progress.h"
#include "filtersd.h"
#include "filtersv.h"
#include "eudora.h"

#include "mainfrm.h"
#include "FilterReportView.h"

// Imap
#include "imaptypes.h"
#include "ImapMailbox.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapFiltersd.h"

//
#include "guiutils.h"
#include "3dformv.h"
#include "tocview.h"

#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

#include "QCImapMailboxCommand.h"

extern QCMailboxDirector	g_theMailboxDirector;


#include "DebugNewHelpers.h"


// Local helper functions
BOOL IsOnSameServer (CTocDoc *pSrcToc, QCImapMailboxCommand *pImapCommand);
void SumListToUidList (CSumList& SumList, CPtrUidList& UidList);
void GetFirstAndLastUids (CPtrUidList& UidList, unsigned long& UidFirst, unsigned long& UidLast);
void CopyUidList (CPtrUidList& SrcUidList, CPtrUidList& DstUidList);
void CommaListToUidList (LPCSTR pStr, CPtrUidList& DstUidList);
void ApplyConjunction (int Conjunction, CPtrUidList& List1, CPtrUidList& List2, CPtrUidList& DstUidList);
POSITION FindInSumlist (CSumList& SumList, unsigned long Uid);
void RemoveUidFromUidList (CPtrUidList& UidList, unsigned long Uid);
static BOOL HasSkipRest (CFilter *pFilt);
void UidListToCommaString (CPtrUidList& UidList, CString& szStr);
BOOL IsXferOnSameServer (CTocDoc *pTocDoc, CFilter *pFilt);
void SetLocalSummariesFlag (CSumList *pSumList, LPCSTR szUids, unsigned long ulBitFlags, BOOL bSet);
int CountUidsInCommaString (LPCSTR szUids);
static unsigned long UIDSTRFindLargest (LPCSTR szUids);
BOOL MaybeStartProgress();
void MaybeCloseProgress(BOOL bWasInProgress);


CImapFilterActions::CImapFilterActions()
{
	m_bPreFilterXferred = FALSE;
	m_bPostFilterXferred = FALSE;

	m_strlstFilterMboxList.RemoveAll();

	m_HighestUidXferred = 0;

	// INI options.
	m_bOpenInMailbox	= FALSE;
	m_bDoFilterReport	= FALSE;

	m_bOpenInMailbox	= GetIniShort(IDS_INI_OPEN_IN_MAILBOX);
	m_bDoFilterReport	= GetIniShort(IDS_INI_FILTER_REPORT);

	m_ulNewMsgsLeftInInbox = 0;
}

CImapFilterActions::~CImapFilterActions()
{
}

//
//	CImapFilterActions::StartFiltering()
//
//	Starts the filtering process.
//
//	dwiggins - Actually, only calls the method it overrides.  Should this be eliminated?
//
BOOL CImapFilterActions::StartFiltering()
{
	return CFilterActions::StartFiltering();
}

//
//	CImapFilterActions::EndFiltering()
//
//	Finishes up the filtering after all messages have been processed.
//
//	dwiggins - How does this differ from CFilterActions::EndFiltering() and do we really need all this code?
//
BOOL CImapFilterActions::EndFiltering()
{
	BOOL Normal = FALSE;
	POSITION pos;	


	// Open up messages that have been filtered	with "Open Message"
	pos = m_FilterOpenSumList.GetHeadPosition();
	while (pos)
	{
		CSummary* sum = m_FilterOpenSumList.GetNext(pos);
		if (sum)
			sum->Display();
	}

	// Turn redraw back on for all mailboxes, which will get the ones that
	// temporarily had redraw turned off as a result of a transfer
	pos = TocTemplate->GetFirstDocPosition();
	while (pos)
	{
		CTocDoc* toc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (toc->GetView())
			toc->GetView()->m_SumListBox.SetRedraw(TRUE);
	}

	// Open up mailboxes that have been filtered with "Open Message"
	//
	pos = m_FilterOpenMBoxTocList.GetHeadPosition();
	while (pos)
	{
		// Open up the mailbox if it isn't open already (except Trash),
		CTocDoc* toc = m_FilterOpenMBoxTocList.GetNext(pos);
		if ( toc && (toc->m_Type != MBT_TRASH) )
		{
			UpdateTocAfterFiltering (toc);

			toc->SetNeedsFumlub();
		}
	}

	// Open up remote mailboxes that had message filtered into them
	// during a pre-filter.
	// 
	pos = m_strlstFilterMboxList.GetHeadPosition();
	while (pos)
	{
		// Open up the mailbox if it isn't open already (except Trash),
		LPCSTR pPathname = m_strlstFilterMboxList.GetNext(pos);

		if (pPathname)
		{

			// Bold all mailboxes that had msgs filtered into them, regerdless of whether we will be 
			// opening them or not.
			//

			QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (pPathname);
			if (pCommand)
				pCommand->Execute( CA_UPDATE_STATUS, ( void* ) US_YES );

			// 
			// Based on the settings, make sure that mailboxes with
			// newly-filtered mail are opened. If the mailboxes are already opened,
			// get the new messages.
			//

			// Gets TOC only if open.
			CTocDoc *toc = GetToc (pPathname, NULL, TRUE, TRUE);

			if ( ( toc && toc->GetView() ) || m_bOpenInMailbox)
			{
				CTocDoc *toc = GetToc (pPathname);
				if ( toc  && (toc->m_Type != MBT_TRASH) )
				{
					UpdateTocAfterFiltering (toc);

					//
					// If the mailbox was just opened or was already opened, then
					// be sure to scroll to the "First Unread Message of the Last
					// Unread Block" ... you know, Fumlub.
					// 
					toc->SetNeedsFumlub();
				}
			}
		}
	}


	// Open up local mailboxes that had message filtered into them
	// during a PostFilter.
	pos = m_FilterTocList.GetHeadPosition();
	while (pos)
	{
		// Open up the mailbox if it isn't open already (except Trash),
		CTocDoc* toc = m_FilterTocList.GetNext(pos);

		// 
		// Based on the settings, make sure that mailboxes with
		// newly-filtered mail are opened.
		//
		if (toc && m_bOpenInMailbox && (toc->m_Type != MBT_TRASH))
		{
			// While it should perhaps be safe to open (and resync) an IMAP mailbox multiple times
			// simultaneously evidence suggests this is problematic.  If more than one filter touches
			// an IMAP message it is possible that the mailbox will be opened above in the loop for
			// m_FilterOpenMBoxTocList in a worker thread.  If this loop is reached quickly enough
			// and the mailbox appears in this list as well then UpdateTocAfterFiltering() will cause
			// a resync of the mailbox in the main thread.  This seems to cause a deadlock situation.
			// The real solution is probably to deal with the deadlock itself, but making sure we don't
			// sync a single mailbox in two different threads is an easy quick fix. -dwiggins
			POSITION	 posTmp = m_FilterOpenMBoxTocList.GetHeadPosition();
			CTocDoc		*pTocTmp = NULL;
			while (posTmp)
			{
				pTocTmp = m_FilterOpenMBoxTocList.GetNext(posTmp);
				if (pTocTmp == toc)
				{
					break;
				}
			}
			// Only update this toc if we didn't do it in the m_FilterOpenMBoxTocList loop above.
			if (pTocTmp != toc)
			{
				UpdateTocAfterFiltering (toc);
			}
		}

		//
		// If the mailbox was just opened or was already opened, then
		// be sure to scroll to the "First Unread Message of the Last
		// Unread Block" ... you know, Fumlub.
		// 
		toc->SetNeedsFumlub();
	}
	
	// Dump Report
	pos = m_NotifyReportList.GetHeadPosition();
	if (pos)
	{
		// Find active Filter Report window, if any.
		CFilterReportView* pView = CFilterReportView::GetFilterReportView();
		if (pView)
		{
			// Do the time stamp thang
			time_t now = time(NULL);
			while (pos)
			{
				CStringCount *mboxName = m_NotifyReportList.GetNext(pos);
				pView->AddEntry( mboxName->m_Count, (const char*)mboxName->m_Name, now );
			}
			// Previously the code checked if m_bDoFilterReport was true before showing the filter report.
			// The variable m_bDoFilterReport simply holds the value of IDS_INI_FILTER_REPORT.  The problem
			// is this won't open the filter report window in the case where IDS_INI_FILTER_REPORT is 0 but
			// a filter is set to notify the user via a report.  To the best that I can determine, if
			// anything actually makes it into m_NotifyReportList then there is reason to show the filter
			// report window.  I leave this comment here in case I am proven wrong on this. -dwiggins
			CMainFrame* pMainFrame = (CMainFrame *) AfxGetMainWnd();
			ASSERT_KINDOF(CMainFrame, pMainFrame);
			pMainFrame->PostMessage(WM_COMMAND, IDM_VIEW_FILTER_REPORT);	// activate the report window
		}
	}


	// PlaySounds();
	pos = m_SoundList.GetHeadPosition();
	while (pos)
	{
		CString snd = m_SoundList.GetNext(pos);
		if (SyncPlayMedia(snd) == FALSE)
			break;
	}

	pos = m_NotifyApp.GetHeadPosition();
	while (pos)
	{
		CString cmdLine = m_NotifyApp.GetNext(pos);
		if (!cmdLine.IsEmpty())
			WinExec(cmdLine,SW_SHOWNA);
		if (EscapePressed(1))
			break;
	}

	// Print Summaries
	m_FilterPrintSum.Print();

	// Clear out lists
	ClearAllLists();

	// Clear new (IMAP) lists.
	m_strlstFilterMboxList.RemoveAll();

	CloseProgress();

	return (Normal);
}

//
//	CImapFilterActions::DoFiltering()
//
//	Does filtering on messages in the given pSumList. 
// This is typically called either just after a check mail or as a result of
// a manual check.
// 
// SumList is a list of summaries within the TOC on which to apply the 
// filters.
//
// The flags determine which filters to apply:
// 1. If bDoSameServerXfers is FALSE, then COPY/XFER's to mailbox on the same server
//	  are omitted.
// 2. If bDoIncoming is FLASE, omit incoming filters.
// 3. If bDoManual is FALSE, omit manual filters.
//
// NOTES:
//
// END NOTES
//
int CImapFilterActions::DoFiltering(CTocDoc *pTocDoc,
									CSumList &sumlist,
									CString *pstrTransUIDs,
									BOOL bDoSameServerXfers,
									BOOL bDoIncoming,
									BOOL bDoManual)
{
	POSITION	 pos = NULL;
	POSITION	 posNext = NULL;
	BOOL		 bUserAborted = FALSE;
	BOOL		 bSearchOnServer = FALSE;

	// If sumlist is empty, nothing to do.
	if (sumlist.GetCount() == 0)
	{
		return 0;
	}

	// Must have a toc.
	if (!pTocDoc)
	{
		ASSERT(0);
		return 0;
	}

	// Must be an IMAP mailbox.
	CImapMailbox		*pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT(0);
		return 0;
	}

	// Make sure we have filters.
	CFilterList			*pFiltList = &(GetFiltersDoc()->m_Filters);
	if (!pFiltList)
	{
		return 0;
	}

	// Re-initialize these:
	m_szAnsweredUids.Empty();
	m_szReadUids.Empty();
	m_szUnreadUids.Empty();
	m_szDeletedUids.Empty();
	m_szFetchUids.Empty();

	CObArray	oaABHashes;

	// Generate the address book hashes once for the entire filtering loop
	CFilter::GenerateHashes(&oaABHashes);

	// Transfering IMAP messages deletes old summaries and creates new, so keep track of the
	// new summaries in this list.
	CTempSumList	sumlistnew;

	if (!bSearchOnServer)
	{
		pos = sumlist.GetHeadPosition();

		for (posNext = pos; pos; pos = posNext)
		{
			CSummary	*pSum = sumlist.GetNext(posNext);
			if (pSum && !pSum->IsIMAPDeleted())
			{
				CSummary	*pSumNew = NULL;
				FilterOne(pSum,
						  bDoIncoming ? WTA_INCOMING : WTA_MANUAL,
						  &oaABHashes,
						  false/*bNoJunkAction*/,
						  &pSumNew);
				// If the summary changed, add the new one to the list, otherwise just use the old.
				if (pSumNew != pSum)
				{
					sumlistnew.AddTail(pSumNew);
				}
				else
				{
					sumlistnew.AddTail(pSum);
				}
			}
		}
	}
	else
	{
		// Get a UID list for the summaries we have in sumlist.
		CPtrUidList		 ulstTotalUidList;
		SumListToUidList(sumlist, ulstTotalUidList);

		// Initialize our global ulstSkipRestUidList:
		CPtrUidList		 ulstSkipRestUidList;

		// Keep track new msgs that are left in  INbox after xferring:
		m_ulNewMsgsLeftInInbox = sumlist.GetCount();

		// Loop through filters.
		CFilter		*pFilt = NULL;
		BOOL		 bApplyFilt = FALSE;
		pos = pFiltList->GetHeadPosition();

		for (posNext = pos; pos; pos = posNext)
		{
			// Check for cancel:
			if (EscapePressed())
			{
				bUserAborted = TRUE;
				break;
			}

			pFilt = pFiltList->GetNext(posNext);
			if (!pFilt)
			{
				continue;
			}

			bApplyFilt = FALSE;

			// Ignore incoming?
			if ( bDoIncoming && pFilt->IsIncoming() )
			{
				bApplyFilt = TRUE;
			}

			// Ignore manual filters?
			if ( bDoManual && pFilt->IsManual() )
			{
				bApplyFilt = TRUE;
			}

			// bApplyFilt MUIST be TRUE at this point!!
			if (!bApplyFilt)
			{
				continue;
			}

			// We have to handle SKIP REST here as well: We may not be applying the
			// filter if it's a Xfer on same server, but if it contains a SKIP REST,
			// we still have to add it to our skip rest UID list!!
			//

			BOOL bHandlingFilter = TRUE;

			//  Must we handle same-server xfers?
			//
			if (!bDoSameServerXfers && IsXferOnSameServer(pTocDoc, pFilt))
			{
				bHandlingFilter = FALSE;
			}

			// Does this filter contain a skip-rest?
			BOOL bHasSkipRest = HasSkipRest(pFilt);

			// We need to do an ImapMatch if either is TRUE:
			if ( bHandlingFilter || bHasSkipRest)
			{
				// Get a UID subset of the summaries that match this filter criteria.
				CPtrUidList		 ulstSubUidList;

				ImapMatch(pTocDoc, pFilt, ulstTotalUidList, &sumlist, ulstSubUidList);

				// Perform the actions. Note: This may result in some uid's 
				// from ulstTotalUidList being removed so further filters don't act
				// on them. This may also result in summaries being deleted from 
				// sumlist.
				if (ulstSubUidList.GetCount () > 0)
				{
					// Note: If we are going to be add these to the skip rest list,
					// copy ulstSubUidList to a temporary list:
					CPtrUidList		 ulstTmpSkipRestUidList;

					if ( bHasSkipRest )
					{
						CPtrUidList		 ulstDummyUidList;
						ApplyConjunction(IDS_FIO_OR, ulstSubUidList, ulstDummyUidList, ulstTmpSkipRestUidList);
						ulstDummyUidList.DeleteAll();
					}

					if ( bHandlingFilter )
					{
						// Before we perform the action, we must remove any UID's in 
						// ulstSkipRestUidList.
						//
						CPtrUidList		 ulstDestUidList;
						ApplyConjunction(IDS_FIO_UNLESS, ulstSubUidList, ulstSkipRestUidList, ulstDestUidList);
						
						PerformAction(pTocDoc, pFilt, sumlist, ulstTotalUidList, ulstDestUidList, pstrTransUIDs, &oaABHashes);
						
						ulstDestUidList.DeleteAll();
					}

					// If this filter had a SKIP_REST, we must add ALL these UID's to the ulstSkipRestUidList
					// so no future actions is performed on them. NOTE: We do this AFTER the operation is 
					// performed!!
					//
					if ( bHasSkipRest )
					{
						// We have to handle SKIP REST at this point. Add the UID's to our global 
						// "SkipRest uid list.
						// Note: Because we're using references, this becomes messy:
						//
						CPtrUidList		 ulstDestUidList;
						ApplyConjunction(IDS_FIO_OR, ulstSkipRestUidList, ulstTmpSkipRestUidList, ulstDestUidList);

						// We have to clean ulstSkipRestUidList and copy from DestUidList back to
						// ulstSkipRestUidList.
						ulstSkipRestUidList.DeleteAll();

						// The following will do it:
						ulstTmpSkipRestUidList.DeleteAll();

						ApplyConjunction(IDS_FIO_OR, ulstDestUidList, ulstTmpSkipRestUidList, ulstSkipRestUidList);

						ulstDestUidList.DeleteAll();
					}

					ulstTmpSkipRestUidList.DeleteAll();
				}

				// We're paranoid.
				ulstSubUidList.DeleteAll ();
			}
		}

		// Cleanup:
		ulstTotalUidList.DeleteAll();
		ulstSkipRestUidList.DeleteAll();
	}

	if (!bSearchOnServer)
	{
		// As of this point, sumlist may point to deleted items so clean it out and copy in the contents
		// of sumlistnew.
		POSITION		 pos = NULL;
		POSITION		 posNext = NULL;

		sumlist.RemoveAll();

		pos = sumlistnew.GetHeadPosition();

		for (posNext = pos; pos; pos = posNext)
		{
			CSummary	*pSum = sumlistnew.GetNext(posNext);
			if (pSum)
			{
				sumlist.AddTail(pSum);
			}
		}
	}

	if (!bUserAborted)
	{
		// If we accumulated any UID's which need a change of READ, UNREAD or 
		// ANSWERED, or DELETED server flags, go perform those block actions now.
		DoServerStatusChanges(pTocDoc, sumlist);

		// If there are messages to be completely fetched (as a result of 
		// a "Server Options" filter action, go fetch those messages now.
		// The UID's would have been accumulated in m_szFetchUids.
		DoFetchRequestedMessages(pTocDoc, sumlist);
	}

	// Did we filter out all new msgs from TOC??
	if (m_ulNewMsgsLeftInInbox > 0)
	{
		// Make sure the toc is in the "OpenMailbox" list.
		if (m_FilterTocList.Find(pTocDoc) == NULL)
		{
			m_FilterTocList.AddTail(pTocDoc);
		}
	}

	// Clean out the local lists before they are freed because freeing will delete the contents.
	sumlist.RemoveAll();
	sumlistnew.RemoveAll();

	// Make sure we reset these:
	m_szAnsweredUids.Empty();
	m_szReadUids.Empty();
	m_szUnreadUids.Empty();
	m_szDeletedUids.Empty();
	m_szFetchUids.Empty();

	return 1;
}

//
//	CImapFilterActions::DoManualFiltering()
//
//	Perform manual filtering.  Generates a list of UIDs of selected messages and calls
//	DoFiltering() on that list.
//
void CImapFilterActions::DoManualFiltering(CTocDoc *pTocDoc, CSummary *pSum /*= NULL*/)
{
	CTempSumList	sumlist;
	CTocView	   *pView = NULL;
	BOOL			bWasInProgress = FALSE;
	BOOL			bUserAborted = FALSE;

	// Sanity:
	if (!pTocDoc)
	{
		return;
	}

	// Must put up progress bar!!
	// We can put up a progress window here!
	bWasInProgress = MaybeStartProgress();
	if (bWasInProgress)
	{
		MainProgress(CRString(IDS_IMAP_FILTERING_MSGS));
	}

	pView = pTocDoc->GetView();

	CSumList &	listSums = pTocDoc->GetSumList();
	POSITION	pos = listSums.GetHeadPosition ();
	POSITION	posNext;
	int			i = 0;
	int			iDeleted = 0;

	// Add selected summaries.
	for (posNext = pos; pos; pos = posNext, i++)
	{
		// Check for cancel:
		if (EscapePressed())
		{
			bUserAborted = TRUE;
			break;
		}

		CSummary* pSumTmp = listSums.GetNext(posNext);

		// Keep searching for pSum if we haven't hit it yet
		if (pSum && (pSum != pSumTmp))
		{
			continue;
		}

		if (pTocDoc->m_bHideDeletedIMAPMsgs && pSumTmp && pSumTmp->IsIMAPDeleted())
		{
			++iDeleted;
		}

		// If this summary isn't selected (non-pSum case) then continue on
		if ( !pSum && pView && (pView->m_SumListBox.GetSel(i - iDeleted) <= 0) )
		{
			continue;
		}

		// Ok. We've got this. Add to list.
		sumlist.AddTail(pSumTmp);

		if (pSum)
			break;
	}

	// Continue only if we got summaries.
	if (!bUserAborted && sumlist.GetCount() > 0)
	{
		// Include same server xfers but omit incoming filters.
		CString		 strUIDList;
		DoFiltering(pTocDoc,
					sumlist,
					&strUIDList,
					TRUE/*bDoSameServerXfers*/,
					FALSE/*bDoIncoming*/,
					TRUE/*bDoManual*/);

		// No longer expunge anything.  With hiding of deleted messages we don't want to auto-expunge.
#if 0
		// Expunge anything that was transferred.
		// Since UIDExpunge() currently just does an EXPUNGE and acts on all deleted messages
		// (not just those deleted by filtering).  The long term solution is better deletion
		// handling, but for now we just don't do the EXPUNGE if fancy trash mode is off. -dwiggins
		if (pTocDoc->m_pImapMailbox->UseFancyTrash())
		{
			CUidMap	 mapUidsActuallyRemoved;
			BOOL	 bResult = SUCCEEDED(pTocDoc->m_pImapMailbox->UIDExpunge(strUIDList, mapUidsActuallyRemoved));

			// Remove the summaries corresponding to the UID's that were actually expunged.
			if (bResult)
			{
				int	 iHighlightIndex = 0;
			
				pTocDoc->ImapRemoveListedTocs(&mapUidsActuallyRemoved,
											  iHighlightIndex,
											  TRUE/*CloseWindow*/,
											  TRUE/*SetViewSelection*/,
											  FALSE/*bJustSetFlag*/);
			}
		}
#endif
	}

	// If we put up progress, take it down.
	MaybeCloseProgress(bWasInProgress);
}

//
//	CImapFilterActions::SetImapMsgStatus()
//
//	Called by the filter's ImapLocalAction routine if the filter has
//	a Make status action. This adds the summary's UID to the
//	appropriate comma-separated list so we can later do a block
//	STORE FLAGS on the server.
//
void CImapFilterActions::SetImapMsgStatus(CSummary *pSum, unsigned char ucStatus)
{
	switch (ucStatus)
	{
		case MS_READ:
			AddUidToStatusString (pSum, m_szReadUids);
			break;

		case MS_UNREAD:
			AddUidToStatusString (pSum, m_szUnreadUids);
			break;

		case MS_REPLIED:
			AddUidToStatusString (pSum, m_szAnsweredUids);
			break;
	}
}

//
//	CImapFilterActions::ImapSetServerOpt()
//
//	Called by the filter's ImapLocalAction routine if the filter has
//	a Server Option action. This adds the summary's UID to the
//	appropriate comma-separated list so we can later do a block
//	STORE FLAGS on the server.
//
void CImapFilterActions::ImapSetServerOpt(CSummary *pSum, unsigned int uiServerOpt)
{
	if (uiServerOpt & SO_DELETE)
		AddUidToStatusString (pSum, m_szDeletedUids);

	if (uiServerOpt & SO_FETCH)
		AddUidToStatusString (pSum, m_szFetchUids);
}

//
//	CImapFilterActions::ImapMatch()
//
//	Called from DoFiltering() to find which summaries match the criteria.
//
//	Returns in ulstMatches a list of the UIDs of messages that match the criteria in pFilt.
//	For junk score matches, psumlistToSearch is used as the source list, for all other criteria
//	ulstToSearch is used as the source list.  Personality and junk score matches are handled
//	locally, all others use a server SEARCH query to find matches.
//
void CImapFilterActions::ImapMatch(CTocDoc *pTocDoc,
								   CFilter *pFilt,
								   CPtrUidList &ulstToSearch,
								   CSumList *psumlistToSearch,
								   CPtrUidList &ulstMatches)
{
	int			 iHeaderNum;
	BOOL		 bPersonalityMatched = FALSE;
	CPtrUidList	 ulist1, ulist2;
	BOOL		 bWasInProgress = FALSE;

	// Must have these:
	if (!pTocDoc || !pFilt)
	{
		return;
	}

	// Must be an IMAP mailbox.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);
		return;
	}

	// Must have uid's.
	if (ulstToSearch.GetCount() == 0)
	{
		return;
	}

	// Formulate ulstToSearch into a comma-string for UIDFind.
	CString szUidStr;

	UidListToCommaString(ulstToSearch, szUidStr);

	// Must have some uid's
	if (szUidStr.IsEmpty())
	{
		return;
	}

	// Tell user what we're doing.
	bWasInProgress = MaybeStartProgress();
	if (bWasInProgress)
	{
		CString		 strBuf;
		strBuf.Format(CRString(IDS_IMAP_SEARCHING_SERVER), pFilt->m_Name);
		MainProgress(strBuf);
	}


	// Initialize this:
	ulstMatches.DeleteAll();

	for (iHeaderNum = 0; iHeaderNum < 2; ++iHeaderNum)
	{
		// Act appropriately for the header we are matching.
		if (!CompareRStringI(IDS_FHEADER_PERSONALITY, pFilt->m_Header[iHeaderNum]))
		{
			// If this is a personality match, then all the UID's in ulstToSearch
			// match! Create our first uid list (List1).
			//
			if (IsOurPersonality(pTocDoc, pFilt, iHeaderNum))
			{
				CopyUidList(ulstToSearch, ulist1);
				bPersonalityMatched = TRUE;
			}
		}
		else if (!CompareRStringI(IDS_FHEADER_JUNK_SCORE, pFilt->m_Header[iHeaderNum]) && psumlistToSearch)
		{
			// This is a junk score match.
			CSummary		*pSum = NULL;
			POSITION		 pos = NULL;
			POSITION		 posNext = NULL;
			int				 iVerb = VERB_FIRST + pFilt->m_Verb[iHeaderNum];
			int				 iMatchScore = atoi(pFilt->m_Value[iHeaderNum]);
			int				 iJunkScore = 0;

			pos = psumlistToSearch->GetHeadPosition();

			for (posNext = pos; pos; pos = posNext)
			{
				pSum = psumlistToSearch->GetNext(posNext);
				iJunkScore = (char)pSum->GetJunkScore();
				if (((iVerb == IDS_LESS_THAN) && (iJunkScore < iMatchScore)) ||
					((iVerb == IDS_MORE_THAN) && (iJunkScore > iMatchScore)))
				{
					ulist1.OrderedInsert(pSum->GetHash(), pSum->m_Imflags, FALSE);
				}
			}
		}
		else
		{
			// Not personality or junk score. Search server for match.
			//
			// Formulate the search criteria for searching the IMAP server.
			// Returns a comma-seoarated list of header names in "HeaderList", along
			// with setting the values of bBody to TRUE if a body search was specified,
			// and setting bNot to TRUE if the verb is a negative.
			//
			CString	 strHeaderList;
			BOOL	 bBody = FALSE;
			BOOL	 bNot = FALSE;
			int		 iVerb = VERB_FIRST + pFilt->m_Verb[iHeaderNum];

			FormulateImapSearchCriteria(pFilt->m_Header[iHeaderNum],
										iVerb,
										pFilt->m_Value[iHeaderNum],
										strHeaderList,
										&bBody,
										&bNot);

			if (bBody || !strHeaderList.IsEmpty())
			{
				// Ask the IMAP server to get messages within our range that match our criterion.
				// NOTE here: Because of IMAP's limitations, we can only do the following 
				// verbs: "Contains", "Doesn't contain".
				CString		 strResults;
				CString		 strText = pFilt->m_Value[iHeaderNum];

				pImapMailbox->UIDFind(strHeaderList, bBody, bNot, strText, szUidStr, strResults);

				// YUCK - Searching sets the \seen flag of some servers (UW). 
				// Unset them here before we do the transfers!!
// (4/1/98)		pImapMailbox->ResetUnseenFlags (ulstToSearch);

				// Did we find matches? If we did, add the uid's to List1.
				if (!strResults.IsEmpty())
				{
					// Convert froma comma-separated list to an ordered UID list. 
					CommaListToUidList(strResults, ulist1);
				}
			}
		}

		// Can we ignore the second criterion??
		if ((iHeaderNum == 0) && (CONJUNCTION_FIRST + pFilt->m_Conjunction == IDS_FIO_IGNORE))
		{
			// The first header was sufficient.  Close up shop and return.
			CopyUidList(ulist1, ulstMatches);

			// If we put up progress, take it down.
			MaybeCloseProgress(bWasInProgress);

			return;
		}
	}

	// If we get to here, both List1 and List2 potentially have contents.
	// Now go merge List1 and List2 using whatever conjunction we have.
	// Return the result in ulstMatches.
	//

	// Get the conjunction.
	int		 iConjunction = CONJUNCTION_FIRST + pFilt->m_Conjunction;

	ApplyConjunction(iConjunction, ulist1, ulist2, ulstMatches);

	// If we put up progress, take it down.
	MaybeCloseProgress(bWasInProgress);
}

//
//	CImapFilterActions::FormulateImapSearchCriteria
// 
//	Formulate the given text strings into a comma-separated list of headers and return
//	it in "HeaderList". If "szHeader" is <<Body>>, set the bBody flag. If the
//	verb is "NOT CONTAINS", set the bNot flag.
//
void CImapFilterActions::FormulateImapSearchCriteria(LPCSTR szHeader,
													 int iVerb,
													 LPCSTR szValue,
													 CString &strHeaderList,
													 BOOL *pbBody,
													 BOOL *pbNot)
{
	// Make sure of this.
	strHeaderList.Empty();

	// Sanity: Must have a header and a value.
	if (!szHeader || !szValue)
	{
		ASSERT(0);
		return;
	}

	// Initialize these.
	if (pbBody)		*pbBody = FALSE;
	if (pbNot)		*pbNot = FALSE;

	// The verb MUST be either CONTAINS, IS, IS NOT or NOT CONTAINS:
	if ((iVerb == IDS_CONTAINS) || (iVerb == IDS_IS))
	{
		if (pbNot)	*pbNot = FALSE;
	}
	else if ((iVerb == IDS_DOESNT_CONTAIN) || (iVerb == IDS_IS_NOT))
	{
		if (pbNot)	*pbNot = TRUE;
	}
	else
	{
		// Shouldn't get here.
		// fewerasserts ASSERT (0);
		return;
	}

	// If <<Body>> was specified, just set the bBody flag and return.
	if (CompareRStringI(IDS_FHEADER_BODY, szHeader) == 0)
	{
		if (pbBody)		*pbBody = TRUE;
		return;
	}

	//
	// Ok. It's a more complicated header spec.
	//

	//
	// Get the header strings in the form we need them. Strip the trailing ":".
	//
	int			 nColonPos = -1;

	// To:
	CString		 strTo = CRString(IDS_FHEADER_TO); 
	nColonPos = strTo.Find(':');
	if (nColonPos > 0)
	{
		strTo = strTo.Left(nColonPos);
	}

	// From:
	CString		 strFrom = CRString(IDS_FHEADER_FROM); 
	nColonPos = strFrom.Find(':');
	if (nColonPos > 0)
	{
		strFrom = strFrom.Left(nColonPos);
	}

	// Subject:
	CString		 strSubject = CRString(IDS_FHEADER_SUBJECT); 
	nColonPos = strSubject.Find(':');
	if (nColonPos > 0)
	{
		strSubject = strSubject.Left (nColonPos);
	}

	// CC:
	CString		 strCC = CRString(IDS_FHEADER_CC); 
	nColonPos = strCC.Find(':');
	if (nColonPos > 0)
	{
		strCC = strCC.Left(nColonPos);
	}

	// Reply-To:
	CString		 strReplyTo = CRString(IDS_FHEADER_REPLY_TO); 
	nColonPos = strReplyTo.Find(':');
	if (nColonPos > 0)
	{
		strReplyTo = strReplyTo.Left(nColonPos);
	}

	// What's our spec?
	//	
	// Any header?	
	if (!CompareRStringI(IDS_FHEADER_ANY, szHeader))
	{
		strHeaderList = strTo + "," + strFrom + "," + strSubject + "," + strCC + "," + strReplyTo;
	}
	// Any Recipient?
	else if (!CompareRStringI(IDS_FHEADER_ADDRESSEE, szHeader))
	{
		strHeaderList = strTo + "," + strCC;
	}
	// To:
	else if (!CompareRStringI(IDS_FHEADER_TO, szHeader))
	{
		strHeaderList = strTo;
	}
	// From:
	else if (!CompareRStringI(IDS_FHEADER_FROM, szHeader))
	{
		strHeaderList = strFrom;
	}
	// Subject:
	else if (!CompareRStringI(IDS_FHEADER_SUBJECT, szHeader))
	{
		strHeaderList = strSubject;
	}
	// Cc:
	else if (!CompareRStringI(IDS_FHEADER_CC, szHeader))
	{
		strHeaderList = strCC;
	}
	// Reply-to:
	else if (!CompareRStringI(IDS_FHEADER_REPLY_TO, szHeader))
	{
		strHeaderList = strReplyTo;
	}
	else
	{
		// Custom header: Use header as-is:
		strHeaderList = szHeader;

		nColonPos = strHeaderList.Find (':');
		if (nColonPos > 0)
		{
			strHeaderList = strHeaderList.Left(nColonPos);
		}

		strHeaderList.TrimRight();
		strHeaderList.TrimLeft();
	}
}

////////////////////////////////////////////////////////////////////////////////////
// DoRemoteTransfers [PRIVATE]
//
// FUNCTION
// Do the actual message transfer to the remote destination mailbox
// based on the given filter (pFilt).
// "pResults" is a comma-separated list of UID's that match an IMAP SEARCH request.
// If the copy/transfer succeeds, remove the uid's from ulstNewUidList.
// END FUNCTION

// NOTE:
// pSumList may be NULL, but if it isn't, whenever we do an XFER, look for the 
// matching summaries and flag the messages as \deleted.
//
////////////////////////////////////////////////////////////////////////////////////
BOOL CImapFilterActions::DoRemoteTransfers(CTocDoc *pTocDoc,
										   CFilter *pFilt,
										   CPtrUidList& ulstNewUidList,
										   LPCSTR szUids,
										   CSumList *pSumList /*= NULL*/)
{
	BOOL		 bCopy = TRUE;
	BOOL		 bResult = FALSE;
	BOOL		 bUserAborted = FALSE;

	// Sanity:
	if (!pTocDoc || !pFilt  || !szUids)
	{
		ASSERT(0);
		return FALSE;
	}

	// Must have one of these.
	CImapMailbox		*pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT(0);
		return FALSE;
	}

	// Get the server's name we're xfering on.
	CImapAccount		*pAccount = g_ImapAccountMgr.FindAccount(pImapMailbox->GetAccountID());
	if (!pAccount)
	{
		ASSERT(0);
		return FALSE;
	}

	CString		 szServerName;
	pAccount->GetAddress(szServerName);

	// If this is being done in the foreground, raise the progress bar.
	BOOL		 bWasInProgress = MaybeStartProgress();

	//		
	// Loop through the actions and perform only copy/transfer to a remote mailbox.
	//
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Check for cancel:
		//
		if (EscapePressed())
		{
			bUserAborted = TRUE;
			break;
		}

		switch(pFilt->m_Actions[i])
		{
			// Found one. Get the destination mailbox.
			case ID_FLT_COPY:
			case ID_FLT_TRANSFER:
			case ID_FLT_JUNK:
				if (!pFilt->m_CopyTo[i].IsEmpty() || pFilt->IsTransfer() || (pFilt->m_Actions[i] == ID_FLT_JUNK))
				{
					CString mboxPath;

					bCopy = TRUE;

					if (pFilt->m_Actions[i] == ID_FLT_TRANSFER)
					{
						mboxPath = pFilt->m_Mailbox;
						bCopy = FALSE;
					}
					else if (pFilt->m_Actions[i] == ID_FLT_COPY)
					{
						mboxPath = pFilt->m_CopyTo[i];
						bCopy = TRUE;
					}
					else
					{
						// Junk.  This is a little contorted.  In order to find the path
						// to the junk mailbox, we need to get the account associated with
						// the given TOC and ask it for its junk mailbox.
						CImapMailbox	*pImapMailbox = pTocDoc->m_pImapMailbox;
						if (pImapMailbox)
						{
							CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(pImapMailbox->GetAccountID());
							if (pAccount)
							{
								pAccount->GetJunkLocalMailboxPath(mboxPath, TRUE/*MakeSureExists*/, TRUE/*bSilent*/);
							}
						}
						bCopy = FALSE;
					}

					// Find the QCImapMailboxCommand object and get the imap name from it.
					// Note: pCommand is the DESTINATION mailbox.
					//
					QCMailboxCommand	*pCommand = g_theMailboxDirector.FindByPathname(mboxPath);
					if (!pCommand)
					{
						ASSERT(0);
						break;	// Out of switch.
					}

					if (!pCommand->IsKindOf(RUNTIME_CLASS(QCImapMailboxCommand)))
					{
						ASSERT(0);
						break;	// Out of switch.
					}

					// If this is not on the same server, ignore it.
					if (!IsOnSameServer(pTocDoc, (QCImapMailboxCommand*)pCommand))
					{
						break;
					}

					CString		 szImapName = ((QCImapMailboxCommand *)pCommand)->GetImapName();

					// Show some kind of progress.  
					if (::IsMainThreadMT())
					{
						CString szBuf;

						szBuf.Format(CRString (IDS_IMAP_XFERING_TOREMOTE), 
									 bCopy ? CRString (IDS_IMAP_COPYING) : CRString (IDS_IMAP_TRANSFERING),
									 szImapName, szServerName);

						MainProgress(szBuf);
					}

					// Do the transfer on the server.
					CDWordArray		 dwaNewUIDs;
					bResult = SUCCEEDED(pImapMailbox->XferMessagesOnServer(szUids, &dwaNewUIDs, szImapName, bCopy));

					// If we transferred messages into a destination mailbox, update the
					// command object's UnreadStatus to US_YES.
					// BUG ALERT!! This is NOT really correct, because the messages may be
					// new to us but may have been read by others!!! We sould 
					// really check the IMAP \\SEEN flags here but thath would involve
					// a FETCH FLAGS, which we DON'T want to do here!!!
					//
					if (bResult)
					{
						// Use the UIDPLUS data to move summary data between the TOC's.
						CTocDoc			*pTargetToc = GetToc(((QCImapMailboxCommand *)pCommand)->GetPathname(),
															 NULL,
															 TRUE,
															 FALSE);
						if (pTargetToc)
						{
							CString		 strUIDs = szUids;
							pTocDoc->ImapTransferSummaryData(strUIDs, pTargetToc, &dwaNewUIDs);
						}

						pCommand->Execute(CA_UPDATE_STATUS, (void*)US_YES);

						// Keep track of the highest UID we've xferred.
						//
						unsigned long ulMax = UIDSTRFindLargest(szUids);

						if (ulMax > m_HighestUidXferred)
							m_HighestUidXferred = ulMax;
					}


					// Flag the summaries as \deleted if we transferred.
					// Note: If this was part of a pre-filter, "pSumList" would be NULL
					// so this wouldn't get executed (which is what we want).
					//
					if (bResult && pSumList && !bCopy)
					{
						SetLocalSummariesFlag(pSumList, szUids, IMFLAGS_DELETED, TRUE);
					}


					// If "Copy" was FALSE, messages were flagged for deletion.
					// Signal that we should do an EXPUNGE..
					if (!bCopy)
					{
						m_bPreFilterXferred = TRUE;
					}

					// If we succeeded, remove uid's from ulstNewUidList.
					if (bResult)
					{
						// Count how many messages we copied/xferred. This is used below 
						// in adding the destination mbox name to the filter report.
						//
						int		 nMsgsXferred = CountUidsInCommaString(szUids);

						RemoveUidsFromList(ulstNewUidList, szUids);

						//
						// Add the command object's pathname to m_strlstFilterMboxList so we can open
						// the toc after filtering.
						//

						LPCSTR	 szPathname = pCommand->GetPathname();

						// Note: For IMAP, since these are remote mailboxes, add to this list
						// so that later, we can check to see if the user has them open. If he
						// does, we'd need to update those mailboxes.
						//
						if (szPathname && (m_strlstFilterMboxList.Find(szPathname) == NULL))
						{
							m_strlstFilterMboxList.AddTail(szPathname);
						}

						// Add to filter report if we need to:
						if (((pFilt->m_NotfyUserNR < 0) && m_bDoFilterReport) ||
							((pFilt->m_NotfyUserNR > 0) && (pFilt->m_NotfyUserNR & NO_REPORT)))
						{
							CString	  szFullMailboxName;

							szFullMailboxName = g_theMailboxDirector.BuildNamedPath(pCommand);

							// We need to add this name to the report list "nMsgsXferred" times
							// becuase that's how the report window is told how many messages were
							// transfered.
							//
							for (int i = 0; i < nMsgsXferred; i++)
							{
								m_NotifyReportList.Add(szFullMailboxName);
							}
						}
					}
				}

				break;
			default:
				break;

		}	// switch
	} // for.

	// Restore progress.
	MaybeCloseProgress(bWasInProgress);

	if (bUserAborted)
	{
		bResult = FALSE;
	}

	return bResult;
}

/////////////////////////////////////////////////////////////////////////
// RemoveUidsFromList [PRIVATE]
//
// Remove the Uid's in the comma-separated list "szUids" from ulstNewUidList.
//
/////////////////////////////////////////////////////////////////////////
BOOL CImapFilterActions::RemoveUidsFromList (CPtrUidList& ulstNewUidList, LPCSTR szUids)
{
	TCHAR	Comma = ',';
	TCHAR	buf [64];
	LPCSTR	p = NULL;
	LPCSTR	q = NULL;

	// Sanity:
	if (! szUids )
	{
		ASSERT (0);
		return FALSE;
	}

	// Loop through Uids.
	//
	p = szUids;

	while (p && *p)
	{
		unsigned long Uid = 0;
		size_t	len;

		q = strchr (p, Comma);
		if (q)
		{
			len = (size_t) (q - p);
			if ( len < sizeof (buf) )	
			{
				strncpy (buf, p, len);
				buf[len] = 0;

				::TrimWhitespaceMT (buf);

				Uid = atol (buf);
			}

			// Advance p;
			p = q + 1;
		}
		else
		{
			// Last or only uid.
			len = strlen (p);
			if ( len < sizeof (buf) )
			{
				strcpy (buf, p);

				::TrimWhitespaceMT (buf);

				Uid = atol (buf);
			}

			// Terminate loop.
			p = NULL;
		}

		if (Uid != 0)
		{
			// Look in list.
			POSITION pos = ulstNewUidList.GetHeadPosition ();
			while (pos)
			{
				CImapFlags *pF = (CImapFlags *) ulstNewUidList.GetAt (pos);
				if  (pF && (pF->m_Uid == Uid) )
				{
					// Found it. Remove it.
					ulstNewUidList.RemoveAt (pos);
					delete pF;
					pF = NULL;
					
					// Get out of loop.
					break;
				}

				ulstNewUidList.GetNext (pos);
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
// PerformAction [PRIVATE}
//
// Perform the action specified by the filter "filt" to the summaries in SumList.
// Do this by wading through ulstSubUidList, find the corresponding Summary in
// SumList and do the action on it, then delete the uid from ulstTotalUidList
// so we don't do the action twice.
//
// NOTE: SumList is just a reference list that contains summaries on which
// to perform the filter actions. The summaries HAVE ALREADY BEEN ADDED to the
// TOC's list, so DON"T destroy any of the summar objects themselves. We may 
// remove the object from the list but we can't destroy the object itself!!!
//
// HISTORY:
//		 (JOK - 4/15/98) - now returns FiltActs.
//
////////////////////////////////////////////////////////////////////////////////
int CImapFilterActions::PerformAction(CTocDoc *pTocDoc,
									  CFilter *pFilt,
									  CSumList &sumlist,
									  CPtrUidList &ulstTotalUidList,
									  CPtrUidList &ulstSubUidList,
									  CString *pstrTransUIDs,
									  CObArray *poaABHashes)
{
	POSITION	 pos = NULL;
	POSITION	 posNext = NULL;
	int			 iNumSavedXferActions = 0;
	int			 iXferActions[NUM_FILT_ACTS];
	int			 iSavedActions[NUM_FILT_ACTS];
	BOOL		 bUserAborted = FALSE;
	int			 iFiltActs = FA_NORMAL;
	BOOL		 bJunking = FALSE;

	// Sanity:
	if (!pTocDoc || !pFilt)
	{
		return iFiltActs;
	}

	// If no uid's to act on, don't continue.
	if (ulstSubUidList.GetCount () <= 0)
		return iFiltActs;

	if (EscapePressed())
	{
		return iFiltActs;
	}

	//
	// If the action we're going to be performing is a transfer to a mailbox on the same server,
	// do a batch xfer.
	// Implement this by saving any matching COPY or TRANSFER in iXferActions and iSavedActions,
	// setting the corresponding actions within the filter to NONE, run the normal
	// Action code on the remaining actions, then coming back and do the
	// COPY or TRANSFER's on the same server.
	//

	// This keeps the number of saved actions.
	iNumSavedXferActions = 0;

	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if (EscapePressed())
		{
			bUserAborted = TRUE;
			break;
		}

		if ((pFilt->m_Actions[i] == ID_FLT_COPY) ||
			(pFilt->m_Actions [i] == ID_FLT_TRANSFER) ||
			(pFilt->m_Actions[i] == ID_FLT_JUNK))
		{
			CString mboxPath;

			// Is the destination mailbox on the same server??
			if (pFilt->m_Actions[i] == ID_FLT_TRANSFER)
			{
				mboxPath = pFilt->m_Mailbox;
			}
			else if (pFilt->m_Actions[i] == ID_FLT_COPY)
			{
				mboxPath = pFilt->m_CopyTo[i];
			}
			else
			{
				// Junk.  This is a little contorted.  In order to find the path
				// to the junk mailbox, we need to get the account associated with
				// the given TOC and ask it for its junk mailbox.
				CImapMailbox	*pImapMailbox = pTocDoc->m_pImapMailbox;
				if (pImapMailbox)
				{
					CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(pImapMailbox->GetAccountID());
					if (pAccount)
					{
						pAccount->GetJunkLocalMailboxPath(mboxPath, TRUE/*MakeSureExists*/, TRUE/*bSilent*/);
					}
				}
			}

			// Find the QCImapMailboxCommand object and get the imap name from it.
			QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (mboxPath);

			if (pCommand)
			{
				if( pCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
				{
					if (IsOnSameServer (pTocDoc, (QCImapMailboxCommand *) pCommand) && (pTocDoc->GetMBFileName() != pCommand->GetPathname()))
					{
						// We've found one.
						iXferActions[iNumSavedXferActions] = i;
						iSavedActions[iNumSavedXferActions] = pFilt->m_Actions[i];

						// Set the action temporarily to none!!!
						pFilt->m_Actions [i] = ID_FLT_NONE;

						iNumSavedXferActions++;
					}
				}
			}
		}
	}

	// If user pressed stop, just go put actions back in.
	if (bUserAborted)
	{
		if (iNumSavedXferActions > 0)
		{
			// Put back the actions into the filter.
			for (i = 0; i < iNumSavedXferActions; i++)
			{
				int			 iActionIndex = iXferActions[i];

				pFilt->m_Actions[iActionIndex] = iSavedActions[i];
			}
		}

		return iFiltActs;
	}

	// This filters one summary at a time. Put up normal progress bar.
	BOOL	 bWasInProgress = FALSE;
	int		 nCount = ulstSubUidList.GetCount();


	// Tell user what we're doing.
	bWasInProgress = MaybeStartProgress();
	if (bWasInProgress && (nCount > 0))
	{
		::CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), nCount);
		::Progress(-1, NULL, -1);
	}

	// Do the rest of the filtering::

	// Stop redraw of the TOC while this is going on:
	CTocView		*pView = pTocDoc->GetView();

	if (pView)
	{
		pView->m_SumListBox.SetRedraw(FALSE);
	}

	// Wade through ulstSubUidList.
	pos = ulstSubUidList.GetHeadPosition();

	for (posNext = pos; pos; pos = posNext)
	{
		if (EscapePressed())
		{
			bUserAborted = TRUE;
			break;
		}

		CImapFlags		*pF = (CImapFlags *)ulstSubUidList.GetNext(posNext);

		if (!pF)
		{
			continue;
		}

		unsigned long	 lUid = pF->m_Uid;
		if (!lUid)
		{
			continue;
		}

		// Find the summary.
		POSITION		 pSumPos = FindInSumlist(sumlist, lUid);
		if (pSumPos)
		{
			CSummary	*pSum = sumlist.GetAt(pSumPos);

			if (!pSum)
			{
				continue;
			}

			// Well, perform the action on the summary.

			// While this gives the impression that ImapLocalAction() will handle copy and
			// transfer actions, code above sets any copy or transfer actions to ID_FLT_NONE
			// and restores them below.  As a result, ImapLocalAction() doesn't do copy and
			// transfer which are handled by DoRemoteTransfers() below.  -dwiggins

			for (int i = 0; i < NUM_FILT_ACTS; i++)
			{
				iFiltActs |= pFilt->ImapAction(pSum, this, i, poaABHashes);
			}

			// If pSum is no longer in the TOC, remove it.
			CSumList &	listTocsSums = pTocDoc->GetSumList();
			if ( !listTocsSums.Find(pSum) )
			{
				sumlist.RemoveAt(pSumPos);

				// Remove it from ulstSubUidList as well because we can't operate on
				// it anymore (say for saved XFET actions).
				ulstSubUidList.RemoveAt(pos);

				delete pF;

				// Remove this uid from ulstTotalUidList as well.
				RemoveUidFromUidList(ulstTotalUidList, lUid);
			}

			::DecrementCountdownProgress();
		}
	}

	// If we put up progress, take it down.
	MaybeCloseProgress(bWasInProgress);

	// If we have any saved xfer/copies, go do those now.
	// Note: We MUST put the actions back in!!.
	//
	if (iNumSavedXferActions > 0)
	{
		// Put back the actions into the filter.
		for (i = 0; i < iNumSavedXferActions; i++)
		{
			int		 iActionIndex = iXferActions[i];

			pFilt->m_Actions[iActionIndex] = iSavedActions[i];
			if (pFilt->m_Actions[iActionIndex] == ID_FLT_TRANSFER)
			{
				iFiltActs |= FA_TRANSFER;
			}
			if (pFilt->m_Actions[iActionIndex] == ID_FLT_JUNK)
			{
				iFiltActs |= FA_TRANSFER;
				bJunking = TRUE;
			}
		}

		// Signal that we may need to do an expunge.
		if ((iFiltActs & FA_TRANSFER) && !pFilt->IsCopyTo())
		{
			m_bPostFilterXferred = TRUE;

			// Keep tab of number of msgs left in Inbox:
			//
			m_ulNewMsgsLeftInInbox--;
		}

		if (!bUserAborted)
		{
			// If we don't have any more uid's to act on, don't continue.
			if (ulstSubUidList.GetCount () > 0)
			{
				// If junking make the necessary summary changes.  The timing of this
				// is a little odd.  Above when we are iterating through the summaries
				// the Junk filter actions have been temporarily removed so we don't
				// actually know we are junking at that time.
				if (bJunking)
				{
					POSITION	 pos = ulstSubUidList.GetHeadPosition();
					POSITION	 posNext = NULL;
					short		 sJunkScore = GetIniShort(IDS_INI_MANUAL_JUNK_SCORE);

					for (posNext = pos; pos; pos = posNext)
					{
						CImapFlags		*pImapFlags = (CImapFlags*)ulstSubUidList.GetNext(posNext);
						if (pImapFlags && pImapFlags->m_Uid)
						{
							POSITION	 pSumPos = FindInSumlist(sumlist, pImapFlags->m_Uid);
							if (pSumPos)
							{
								CSummary	*pSum = sumlist.GetAt(pSumPos);
								if (pSum)
								{
									if (sJunkScore > 0)
									{
										pSum->SetJunkScore((unsigned char)sJunkScore);
									}
									pSum->SetJunkPluginID(0);
								}
							}
						}
					}
				}

				// Format ulstSubUidList into a comma-separated list.
				CString			 strUIDs;
				UidListToCommaString(ulstSubUidList, strUIDs);

				if (!strUIDs.IsEmpty())
				{
					// Now pass that to DoRemoteTransfers.
					DoRemoteTransfers(pTocDoc, pFilt, ulstTotalUidList, strUIDs, &sumlist);
				}

				// If the caller wants to know which messages were transfered update that list.
				if (pstrTransUIDs)
				{
					*pstrTransUIDs = strUIDs;
				}
			}
		}
	}

	// This should update \deleted flags, etc!!.
	if (pView)
	{
		pView->m_SumListBox.SetRedraw(TRUE);
	}

	return iFiltActs;
}

// DoServerStatusChanges [PRIVATE]
//
// If there are any UID's in the szAnsweredUids, etc., strings, go set their server status.
//
// NOTE: The 
void CImapFilterActions::DoServerStatusChanges(CTocDoc *pTocDoc, CSumList& sumlist)
{
	// Sanity:
	if (!pTocDoc)
	{
		return;
	}

	// Must have a valid CImapMailbox object.
	CImapMailbox		*pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT(0);
		return;
	}
		
	// Do READ changes.
	if ( !m_szReadUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->ImapSetSeenFlag (m_szReadUids)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&sumlist, m_szReadUids, IMFLAGS_SEEN, TRUE);
		}
	}

	// Do UNREAD changes.
	if ( !m_szUnreadUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->ImapUnsetSeenFlag (m_szUnreadUids)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&sumlist, m_szUnreadUids, IMFLAGS_SEEN, FALSE);
		}
	}

	// Do REPLIED changes.
	if ( !m_szAnsweredUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->ImapSetAnsweredFlag (m_szAnsweredUids)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&sumlist, m_szAnsweredUids, IMFLAGS_ANSWERED, TRUE);
		}
	}

	// Do DELETED changes (This is from a "Server Options" filter action!!!.
	if ( !m_szDeletedUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->DeleteMessagesFromServer (m_szDeletedUids, FALSE)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&sumlist, m_szDeletedUids, IMFLAGS_DELETED, TRUE);
		}
	}
}

// DoFetchRequestedMessages [PRIVATE]
// If a filter with a "Server Options" action found UID matches, wade through
// m_szFetchUid's and download the message if it was only minimally downloaded.
// Make sure the UID is in sumlist before doing the download.
//
void CImapFilterActions::DoFetchRequestedMessages (CTocDoc *pTocDoc, CSumList& sumlist)
{
	char buf [24];
	char Comma = ',';

	// Sanity:
	if (!pTocDoc)
		return;

	if ( m_szFetchUids.IsEmpty () )
		return;

	// Wade through the comma-separated UID list:
	const char *p = m_szFetchUids;
	const char *q;
	size_t len;
	CSummary *pSum;
	POSITION pos;
	unsigned long Uid;

	while (p && *p)
	{
		*buf = 0;

		q = strchr (p, Comma);

		if (q)
		{
			len = (q - p);

			if (len < sizeof (buf) )
			{
				strncpy (buf, p, len);
				buf[len] = 0;
			}

			// Advance p:
			p = q + 1;
		}
		else
		{
			len = strlen (p);
			if ( len < sizeof (buf) )
				strcpy (buf, p);

			// No more uid's

			p = NULL;
		}

		if (*buf)
		{
			Uid = atol (buf);

			if (Uid != 0)
			{
				pos = FindInSumlist (sumlist, Uid);
				if (pos)
				{
					pSum = sumlist.GetAt (pos);
					if (pSum)
					{
						// If it wasn't downloaded, go do it now. If we failed to download
						// any one, stop the procedure.
						//
						if ( !SUCCEEDED (DownloadImapMessage (pSum, FALSE)) )
							break;  // Out of while
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
// ImapXferMatched [PRIVATE]
// Examine "pFilt" to see if it's a filter that has a transfer or copy action:
//
//
BOOL CImapFilterActions::IsXferFilter (CFilter* pFilt)
{
	// Ok. Does this have a Transfer or Copy operation?
	//

	BOOL bGoodFilter = FALSE;

	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if ((ID_FLT_COPY == pFilt->m_Actions[i]) ||
			(ID_FLT_TRANSFER == pFilt->m_Actions[i]) ||
			(ID_FLT_JUNK == pFilt->m_Actions[i]))
		{
			// Found one.
			bGoodFilter = TRUE;
				break;
		}
	}

	return bGoodFilter;
}

/////////////////////////////////////////////////////////////////////
// IsOurPersonality [PRIVATE]
// Return TRUE if the value in m_Value[HeaderNum] matches (exactly, contains, etc),
// the name of our personality.
//
BOOL CImapFilterActions::IsOurPersonality (CTocDoc *pTocDoc, CFilter *pFilt, int HeaderNum)
{
	// Sanity;
	if (! (pTocDoc && pFilt) )
		return FALSE;

	// HeaderNum MUST be 0 or 1.
	if (HeaderNum < 0 || HeaderNum > 1)
		return FALSE;

	// Get the CImapMailbox pointer from the TOC.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;

	if ( !pImapMailbox )
	{
		ASSERT (0);
		return FALSE;
	}

	//
	// Get our persoanlity name;
	//
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( pImapMailbox->GetAccountID () );
	if (!pAccount)
	{
		ASSERT (0);
		return FALSE;
	}

	CString szOurPersona; pAccount->GetName ( szOurPersona );

	// Matches are case insensitive. Note that personality match supports 
	// all the possible verbs.
	//
	szOurPersona.MakeLower();

	return pFilt->MatchValue (HeaderNum, szOurPersona);
}

// AddUidToStatusString [PRIVATE]
//
// Add the UID of the given summary to m_szReadUids.
// Make sure the list is in proper comma-separated form.
//
void CImapFilterActions::AddUidToStatusString (CSummary *pSum, CString& szStr)
{
	if (!pSum)
	{
		ASSERT (0);
		return;
	}

	// Add to szStr.
	if (szStr.IsEmpty())
		szStr.Format ( "%lu", pSum->GetHash() );
	else
	{
		CString szUid; 
		szUid.Format ( ",%lu", pSum->GetHash() );
		szStr += szUid;
	}
}

// UpdateTocAfterFiltering [PRIVATE]
//
// Either display the TOC or cause it to go fetch new messages.
//
void CImapFilterActions::UpdateTocAfterFiltering (CTocDoc *toc)
{
	if (!toc)
		return;

	// JOK - if it's an IMAP mailbox and was already open,
	// go check mail so it will retrieve the new mesages.
	//
	if ( toc->IsImapToc () )
	{
		if (! toc->GetView() )
		{
			toc->Display ();
		}
		else if (toc->m_pImapMailbox)
		{
			// If the toc is the same as the toc from which messages were
			// just filtered, we just want to bring it to the front without
			// doing anothe check mail!!
			// 
			if (!m_szMBFilename.IsEmpty () && 
					(m_szMBFilename.CompareNoCase (toc->GetMBFileName()) == 0) )
			{
				toc->Display ();
			}
			else
			{
				// This must be a destination mailbox. We WNAT to chack
				// mail.
				//
				toc->m_pImapMailbox->CheckNewMail (toc);
			}
		}
	}
	else // Just display it.
	{
		toc->Display();
	}
}


//=================== Utility functions =======================//

// IsOnSameServer [EXPORTED]
//
// Return TRUE if the command object is on the same IMAP server as pSrcToc.
//
// static
BOOL IsOnSameServer (CTocDoc *pSrcToc, QCImapMailboxCommand *pImapCommand)
{
	// This MUST be performed in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// Sanity:
	if (! (pSrcToc && pImapCommand) )
		return FALSE;

	// 
	if( ! pImapCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
		return FALSE;

	if (!pSrcToc->m_pImapMailbox)
		return FALSE;

	// If have same account ID, then a match.
	if ( pSrcToc->m_pImapMailbox->GetAccountID () == pImapCommand->GetAccountID () )
	{
		return TRUE;
	}

	return FALSE;
}

//
// Get the uid's from a summary list into a CPtrUidList.
//
void SumListToUidList (CSumList& sumlist, CPtrUidList& UidList)
{
	POSITION pos, next;

	//
	// Clear this first, including the objects themselves.
	//
	UidList.DeleteAll ();

	pos = sumlist.GetHeadPosition();

	for (next = pos; pos; pos = next)
	{
		CSummary* pSum = sumlist.GetNext(next);

		if (pSum)
		{
			unsigned long Uid = pSum->GetHash();
			if (Uid)
			{
				UidList.OrderedInsert (Uid, pSum->m_Imflags, FALSE);
			}
		}
	}
}



//
// Get the first and last UID's from the ordered UID list "UidList"
//
void GetFirstAndLastUids (CPtrUidList& UidList, unsigned long& UidFirst, unsigned long& UidLast)
{
	// Initialize:
	UidFirst = UidLast = 0;

	// Get the first and last uid's to search.
	//
	// Determine UidFirst and UidLast from ulstNewUidList. These must be valid.
	// Note: ulstNewUidList is ordered in ascending UID.
	//
	POSITION pos = UidList.GetHeadPosition ();
	if (pos)
	{
		CImapFlags *pF = (CImapFlags *) UidList.GetNext (pos);
		if (pF)
			UidFirst = pF->m_Uid;
	}

	// Get last entry.
	pos = UidList.GetTailPosition ();
	if (pos)
	{
		CImapFlags *pF = (CImapFlags *) UidList.GetPrev (pos);
		if (pF)
			UidLast = pF->m_Uid;
	}
}

//
// Copy source uid list to destination, creating new objects.
//
void CopyUidList (CPtrUidList& SrcUidList, CPtrUidList& DstUidList)
{
	unsigned long Uid;

	// Free this.
	DstUidList.DeleteAll();

	POSITION pos = SrcUidList.GetHeadPosition ();

	while (pos)
	{
		CImapFlags *pF = (CImapFlags *) SrcUidList.GetNext (pos);
		if (pF)
		{
			Uid = pF->m_Uid;
			if (Uid)
			{
				DstUidList.OrderedInsert (Uid, pF->m_Imflags, FALSE);
			}
		}
	}
}

//
// Extract the Uid's from a comma-separated list and add to DstUidList, 
// We don't have any flags info so set that to 0.
//
void CommaListToUidList (LPCSTR pStr, CPtrUidList& DstUidList)
{
	const char *p, *q;
	char Comma = ',';
	char buf [12];
	unsigned long len;
	unsigned long Uid;

	// Free this.
	DstUidList.DeleteAll ();

	p = pStr;

	while (p && *p)
	{
		*buf = 0;

		q = strchr (p, Comma);

		if (q)
		{
			len = (unsigned long)q - (unsigned long)p;

			if ( len < sizeof(buf) )
			{
				strncpy (buf, p, len);
				buf[len] = 0;
			}

			// Advance over comma.
			p = q + 1;
		}
		else
		{
			if (strlen (p) < sizeof (buf) )
			{
				strcpy (buf, p);
			}

			// Terminate.
			p = NULL;
		}

		Uid = atol (buf);

		if (Uid > 0)
		{
			DstUidList.OrderedInsert (Uid, 0, FALSE);
		}
	}
}

//
// Format a uid list into a comma-separated string of uid's.
//
void UidListToCommaString (CPtrUidList& UidList, CString& szStr)
{
	// Clear this before we loop.
	szStr.Empty();

	POSITION pos = UidList.GetHeadPosition ();
	POSITION posNext;
	CString sUid;

	for (posNext = pos; pos; pos = posNext)
	{
		CImapFlags *pF = (CImapFlags *) UidList.GetNext (posNext);

		if (pF && pF->m_Uid)
		{
			if (szStr.IsEmpty())
				szStr.Format("%lu", pF->m_Uid);
			else
			{
				sUid.Format (",%lu", pF->m_Uid);
				szStr += sUid;
			}
		}
	}
}

// 
// Count the number of UIDs in a comma-separated string of UIDS.
// 
int CountUidsInCommaString (LPCSTR szUids)
{
	const char *p, *q;
	const char Comma = ',';
	int nMsgs = 0;

	if (!szUids)
		return 0;

	p = szUids;

	while (p && *p)
	{
		// If we get here, we assume we have at least 1 more uid.

		nMsgs++;

		// Do we have more??
		//
		q = strchr (p, Comma);

		if (q)
		{
			// Advance over comma.
			// Note: This ASSUMES that we don't have a situation sucs as ",<one or more space>,"
			// That's OK.
			//
			p = q + 1;
		}
		else
		{
			// Last (or only) UID.
			// Terminate.
			p = NULL;
		}
	}

	return nMsgs;
}

//
// Given two UID lists and a "Conjunction", merge them based on whether the
// "conjunction" is AND, OR or UNLESS.
// The resulting list is "DstUidList".
// Note: CPtrUidList's are ordered by UID.
//
void ApplyConjunction (int Conjunction, CPtrUidList& List1, CPtrUidList& List2, CPtrUidList& DstUidList)
{
	POSITION L1Pos, L1Next;
	POSITION L2Pos, L2Next;
	CImapFlags *pL1F, *pL2F;
	unsigned long curUid;

	// Free this first.
	DstUidList.DeleteAll ();

	// Must be valid.
	if (Conjunction == IDS_FIO_IGNORE)
		return;

	// Merge.
	L1Pos = List1.GetHeadPosition();
	L2Pos = List2.GetHeadPosition();

	// If second list is NULL and conjunction is OR or unless, just add what's is first list.
	if (!L2Pos)
	{
		if ( (Conjunction == IDS_FIO_OR) || (Conjunction == IDS_FIO_UNLESS) )
		{
			for (L1Next = L1Pos; L1Pos; L1Pos = L1Next)
			{
				pL1F = ( CImapFlags * ) List1.GetNext ( L1Next );

				if (pL1F && (pL1F->m_Uid != 0))
				{
					DstUidList.OrderedInsert (pL1F->m_Uid, 0, FALSE);
				}
			}
		}
	}
	else
	{
		// Else if first list is empty, add second list if conjunction is OR.
		if (!L1Pos)
		{
			if (Conjunction == IDS_FIO_OR)
			{
				for (L2Next = L2Pos; L2Pos; L2Pos = L2Next)
				{
					pL2F = ( CImapFlags * ) List2.GetNext ( L2Next );

					if (pL2F && (pL2F->m_Uid != 0))
					{
						DstUidList.OrderedInsert (pL2F->m_Uid, 0, FALSE);
					}
				}
			}
		}
		// Else we've got in both lists. Merge them.
		else
		{
			// Loop through List1.
			//
			for (L1Next = L1Pos; L1Pos; L1Pos = L1Next)
			{
				pL1F = ( CImapFlags * ) List1.GetNext( L1Next );

				if (!pL1F)
				{
					continue;
				}

				curUid = pL1F->m_Uid;

				// Ignore zero uid.
				if (curUid == 0)
				{
					continue;
				}

				// If no more in list 2, break out of list 1. 
				// Note: Keep this ahead of the "while" to make sure we ALWAYS loop through
				// list 1. This avoids the situation where we kave an UNLESS and only 1 
				// matching entry in list 2. If we break after the while, we'll end up 
				// with the entry from list 1 still making it into the final list.
				// 

				if (!L2Pos)
					break;
		
				// Loop through List2 until we get to the same or next higher UID.
				//
				while (L2Pos)
				{
					L2Next = L2Pos;

					pL2F = ( CImapFlags * ) List2.GetNext ( L2Next );

					if (!pL2F)
					{
						L2Pos = L2Next;
						continue;
					}

					// ignore any zero-uid entries.
					if (pL2F->m_Uid == 0)
					{
						// Loop to the next entry.
						L2Pos = L2Next;

						continue;
					}

					// Same UID's in list??
					if (curUid == pL2F->m_Uid)
					{
						// AND or OR means add one of them.
						if (Conjunction == IDS_FIO_AND || Conjunction == IDS_FIO_OR)
						{
							DstUidList.OrderedInsert (curUid, 0, FALSE);
						}

						// Note: Unless means none of them, so ignore both.
						// Advance on both lists.
						//
						L2Pos = L2Next;

						break;
					}
					else if (curUid > pL2F->m_Uid)
					{
						// Add L2F and increment it if it's OR, otherwise just increment it
						// and test again.
						if (Conjunction == IDS_FIO_OR)
						{
							DstUidList.OrderedInsert (pL2F->m_Uid, 0, FALSE);
						}
						// If UNLESS, and this is the last member of List 2, add
						// CurUid because otherwise, we'd be skipping it.
						//
						else if ( (Conjunction == IDS_FIO_UNLESS) && (L2Next == NULL) )
						{
							DstUidList.OrderedInsert (curUid, 0, FALSE);
						}

						L2Pos = L2Next;

						// Note: Don't break out. Must loop through list 2.
					}
					else // pL2F->m_Uid > curUid.
					{
						// We've gone past the uid in list 1. This means there's no
						// match in list 2. If this is an OR or an unless, add the 
						// List1 entry.
						// 
						if (Conjunction == IDS_FIO_OR || Conjunction == IDS_FIO_UNLESS)
						{
							// Add list1 and increment it but don't increment list2.
							//
							DstUidList.OrderedInsert (curUid, 0, FALSE);
						}

						// Advance through List 1, not list 2.

						// Next time we start with this pL2Pos again.
						// Don't update newPos.
						//
						break; // Out of list 2.
					}
				}  // End while List 2.

			} // For list 1.


			// If Conjunction is an OR or UNLESS, add any left-overs from list 1.

			if ( (Conjunction == IDS_FIO_OR) || (Conjunction == IDS_FIO_UNLESS) )
			{
				for (L1Next = L1Pos; L1Pos; L1Pos = L1Next)
				{
					pL1F = ( CImapFlags * ) List1.GetNext ( L1Next );

					if (pL1F && (pL1F->m_Uid != 0))
					{
						DstUidList.OrderedInsert (pL1F->m_Uid, 0, FALSE);
					}
				}
			}

			// If this is an OR, we must add any remaining uid in List2.
			if ( Conjunction == IDS_FIO_OR )
			{
				for (L2Next = L2Pos; L2Pos; L2Pos = L2Next)
				{
					pL2F = ( CImapFlags * ) List2.GetNext ( L2Next );

					if (pL2F && (pL2F->m_Uid != 0))
					{
						DstUidList.OrderedInsert (pL2F->m_Uid, 0, FALSE);
					}
				}
			}
		}  // Has uids in List 1.
	} // else uids in list 2.
}

// Loop through sumlist looking for Uid. Return the POSITION.
//
POSITION FindInSumlist (CSumList& sumlist, unsigned long Uid)
{
	POSITION pos, next;

	pos = sumlist.GetHeadPosition ();

	for (next = pos; pos; pos = next)
	{
		CSummary *pSum = sumlist.GetNext (next);

		unsigned long ThisUid = pSum->GetHash ();

		if (ThisUid == Uid)
			return pos;
	}

	return NULL;
}

// Remove this uid from UidList;
void RemoveUidFromUidList (CPtrUidList& UidList, unsigned long Uid)
{
	POSITION pos, next;

	pos = UidList.GetHeadPosition ();

	for (next = pos; pos; pos = next)
	{
		CImapFlags *pF = (CImapFlags *)UidList.GetNext (next);

		if (pF)
		{
			if (pF->m_Uid == Uid)
			{
				UidList.RemoveAt (pos);

				delete pF;

				break;
			}
		}
	}
}

//
// Return TRUE if this filter has a COPY or TRANSFER to a mailbox on the same server as pTocDoc.
//
BOOL IsXferOnSameServer (CTocDoc *pTocDoc, CFilter *pFilt)
{
	BOOL bResult = FALSE;

	// Sanity
	if (! (pTocDoc && pFilt) )
		return FALSE;

	// Does it have a copy/xfer?
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		switch(pFilt->m_Actions[i])
		{
			// Found one. Get the destination mailbox.
			case ID_FLT_COPY:
			case ID_FLT_TRANSFER:
			case ID_FLT_JUNK:
				if (!pFilt->m_CopyTo[i].IsEmpty() || pFilt->IsTransfer())
				{
					CString mboxPath;

					if (pFilt->m_Actions[i] == ID_FLT_TRANSFER)
					{
						mboxPath = pFilt->m_Mailbox;
					}
					else if (pFilt->m_Actions[i] == ID_FLT_COPY)
					{
						mboxPath = pFilt->m_CopyTo[i];
					}
					else
					{
						// Junk.  This is a little contorted.  In order to find the path
						// to the junk mailbox, we need to get the account associated with
						// the given TOC and ask it for its junk mailbox.
						CImapMailbox	*pImapMailbox = pTocDoc->m_pImapMailbox;
						if (pImapMailbox)
						{
							CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(pImapMailbox->GetAccountID());
							if (pAccount)
							{
								pAccount->GetJunkLocalMailboxPath(mboxPath, TRUE/*MakeSureExists*/, TRUE/*bSilent*/);
							}
						}
					}

					// Find the QCImapMailboxCommand object and get the imap name from it.
					QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (mboxPath);
					if (!pCommand)
					{
						break;	// Out of switch.
					}

					if( ! pCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
					{
						break;	// Out of switch.
					}

					// If this is not on the same server, ignore it.
					if (! IsOnSameServer (pTocDoc, (QCImapMailboxCommand *) pCommand) )
					{
						break;
					}

					// Seems like we found one.
					bResult = TRUE;
					break;
				}

				break;

			default:
				break;
		} // switch

		// Break after the first TRUE response;
		if (bResult)
			break;

	} // for

	return bResult;
}

static BOOL HasSkipRest(CFilter *pFilt)
{
	// Sanity
	if (! pFilt )
		return FALSE;

	// Look for ANY skip rest:
	//
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if (pFilt->m_Actions[i] == ID_FLT_SKIP_REST)
			return TRUE;
	}

	return FALSE;
}

// SetLocalSummariesFlag [INTERNAL]
//
// Loop through pSumList looking for each pUid. If found, set the
// local summary flag depending on what's contained in "ulBitFlag".
// NOTE: ulBitFlag is an OR'd combination of the IMFLAGS_* bitflags.
// NOTE: If "bSet" is FALSE, remove the flag instead of setting it.
//
void SetLocalSummariesFlag (CSumList *pSumList, LPCSTR szUids, unsigned long ulBitFlags, BOOL bSet)
{
	char buf [24];
	char Comma = ',';

	// Sanity:
	if (! (pSumList && szUids) )
		return;

	const char *p = szUids;
	const char *q;
	size_t len;
	CSummary *pSum;
	POSITION pos;
	unsigned long Uid;

	while (p && *p)
	{
		*buf = 0;

		q = strchr (p, Comma);

		if (q)
		{
			len = (q - p);

			if (len < sizeof (buf) )
			{
				strncpy (buf, p, len);
				buf[len] = 0;
			}

			// Advance p:
			p = q + 1;
		}
		else
		{
			len = strlen (p);
			if ( len < sizeof (buf) )
				strcpy (buf, p);

			// No more uid's

			p = NULL;
		}

		if (*buf)
		{
			Uid = atol (buf);

			if (Uid != 0)
			{
				pos = FindInSumlist (*pSumList, Uid);
				if (pos)
				{
					pSum = pSumList->GetAt (pos);
					if (pSum)
					{
						// Set the CSummary IMFLAGS_DELETED flag:
						if (ulBitFlags & IMFLAGS_DELETED)
						{
							if (bSet)
								pSum->m_Imflags |= IMFLAGS_DELETED;
							else
								pSum->m_Imflags &= !IMFLAGS_DELETED;

						}

						// READ/UNREAD: Must also set the POP flag.
						if (ulBitFlags & IMFLAGS_SEEN)
						{
							if (bSet)
							{
								pSum->m_Imflags |= IMFLAGS_SEEN;
								pSum->SetState (MS_READ);
							}
							else
							{
								pSum->m_Imflags &= !IMFLAGS_SEEN;
								pSum->SetState (MS_UNREAD);
							}
						}

						// ANSWERED: (i.e. replied) Must also set the POP flag.
						if (ulBitFlags & IMFLAGS_ANSWERED)
						{
							if (bSet)
							{
								pSum->m_Imflags |= IMFLAGS_ANSWERED;
								pSum->SetState (MS_REPLIED);
							}
						}
					}
				}
			}
		}
	}
}

// 
// FUNCTION
// Wade through the given comma-separated string and return the largest UID in it.
//
static unsigned long UIDSTRFindLargest (LPCSTR szUids)
{
	unsigned long ulMax = 0;
	unsigned long Uid;
	char buf [24];
	char Comma = ',';
	const char *p;
	const char *q;
	size_t len;

	if (!szUids)
		return 0;

	// Loop through list:
	//

	p = szUids;

	while (p && *p)
	{
		*buf = 0;

		q = strchr (p, Comma);

		if (q)
		{
			len = (q - p);

			if (len < sizeof (buf) )
			{
				strncpy (buf, p, len);
				buf[len] = 0;
			}

			// Advance p:
			p = q + 1;
		}
		else
		{
			len = strlen (p);
			if ( len < sizeof (buf) )
				strcpy (buf, p);

			// No more uid's

			p = NULL;
		}

		if (*buf)
		{
			Uid = atol (buf);

			if (Uid > ulMax)
				ulMax = Uid;
		}
	}

	return ulMax;
}

BOOL MaybeStartProgress()
{
	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			PushProgress();
			return TRUE;
		}
	}
	return FALSE;
}

void MaybeCloseProgress(BOOL bWasInProgress)
{
	if (::IsMainThreadMT())
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

#endif // IMAP4
