// ImapFiltersd.cpp
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
#include "imapfol.h"
#include "imapacct.h"
#include "imapactl.h"
#include "ImapFiltersd.h"

#include "ImapPreFiltersd.h"

//
#include "guiutils.h"
#include "3dformv.h"
#include "tocview.h"

#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

#include "QCImapMailboxCommand.h"

extern QCMailboxDirector	g_theMailboxDirector;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



// STATIC FUNCIONS
void SumListToUidList (CSumList& SumList, CPtrUidList& UidList);
void GetFirstAndLastUids (CPtrUidList& UidList, unsigned long& UidFirst, unsigned long& UidLast);
void CopyUidList (CPtrUidList& SrcUidList, CPtrUidList& DstUidList);
void CommaListToUidList (LPCSTR pStr, CPtrUidList& DstUidList);
void ApplyConjunction (int Conjunction, CPtrUidList& List1, CPtrUidList& List2, CPtrUidList& DstUidList);
POSITION FindInSumlist (CSumList& SumList, unsigned long Uid);
void RemoveUidFromUidList (CPtrUidList& UidList, unsigned long Uid);
static 
BOOL HasSkipRest (CFilter *pFilt);
void UidListToCommaString (CPtrUidList& UidList, CString& szStr);
BOOL IsXferOnSameServer (CTocDoc *pTocDoc, CFilter *pFilt);
void SetLocalSummariesFlag (CSumList *pSumList, LPCSTR pUids, unsigned long ulBitFlags, BOOL bSet);
int CountUidsInCommaString (LPCSTR pUids);
static unsigned long UIDSTRFindLargest (LPCSTR pUids);

//-----------------------------------------------------------------------------//
// Note:: AccountId here CANNOT be NULL.
CImapFilterActions::CImapFilterActions()
{
	m_EscapePressed = FALSE;

	m_bPreFilterXferred = FALSE;

	m_bPostFilterXferred = FALSE;

	// Clear all lists that we added.
	ClearAllImapLists ();

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




// ImapStartFiltering()
//
// FUNCTION
// Starts the filtering process
// END FUNCION

BOOL CImapFilterActions::ImapStartFiltering()
{
	return StartFiltering();
}




// InitializeFromBackgroundFilter [PUBLIC]
//
// If a background pre filter occurred, call this function to
// add results of that process to our internal info.
// 
// NOTE: strDstList contains allocated CString objects that contain the
// full paths to the destination MBX files. we REMOVE THEM HERE!!!
//
// NOTE: strReportList contains allocated CString objects that contain the
// full paths to the destination MBX files that should. we REMOVE THEM HERE!!!
//
void CImapFilterActions::InitializeFromBackgroundFilter (CObList& strDstList, CObList& strReportList)
{
	// Must be in the main thread.
	//
	ASSERT (IsMainThreadMT());

	// Find command objects for the MBX files and add the pathname to m_FilterMboxList.
	// Also, determine if they should be added to the filter report list, and
	// if so, add it.
	//

	POSITION pos = strDstList.GetHeadPosition();
	POSITION next;

	for (next = pos; pos; pos = next)
	{
		CString * pStr = (CString *) strDstList.GetNext (next);

		if (pStr)
		{
			// Find the QCImapMailboxCommand object.
			//
			QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (*pStr);

			if (pCommand)
			{
				LPCSTR pPathname = pCommand->GetPathname ();

				if ( pPathname && (m_FilterMboxList.Find(pPathname) == NULL) )
				{
					m_FilterMboxList.AddTail (pPathname);
				}
			}

			// Remove the entry.
			//
			strDstList.RemoveAt (pos);

			delete pStr;
		}
	}

	// Go handle filter report stuff now:
	// Here, we call the CStringCountList's "Add" method on the data in strReportList.
	// NOTE: We're duplicating a lot of stuff here!!!
	//
	pos = strReportList.GetHeadPosition();
	next;

	for (next = pos; pos; pos = next)
	{
		CString * pStr = (CString *) strReportList.GetNext (next);

		if (pStr)
		{
			// Find the QCImapMailboxCommand object.
			//
			QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (*pStr);

			if (pCommand)
			{
				CString szFullMailboxName = g_theMailboxDirector.BuildNamedPath( pCommand );

				m_NotifyReportList.Add( szFullMailboxName );
			}

			// Remove the entry.
			//
			strReportList.RemoveAt (pos);

			delete pStr;
		}
	} // for
}



// DoXferPreFiltering [PUBLIC]
//
// Does pre-filtering on ALL messages in NewUidList. Filter actions
// that are handled here are: 
//	- Copy/Xfer to another mbox on the same server.
//
// On input, NewUidList contains new message UID's that need to be filtered.
// On output, NewUidList contains message UID's that still exist in 
// the source mailbox. 
//
// NOTES: 
// 1. We only handle INCOMING filters here!!!!
// 2. Return the highest UID actually xferred in "ulHighestUidXferred".
//
// END NOTES
//
int CImapFilterActions::DoXferPreFiltering (CTocDoc *pTocDoc, CPtrUidList& NewUidList)
{
	CFilterList *pFiltList	= NULL;

	// If no UID's, nothing to do.
	if ( NewUidList.GetCount () <= 0 )
		return 0;

	// Must have a toc.
	if (!pTocDoc)
	{
		ASSERT (0);
		return 0;
	}

	// Must be an IMAP mailbox.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);
		return 0;
	}

	// Get the Uid list into a comma-separated string.
	CString szUidStr;

	UidListToCommaString (NewUidList, szUidStr);

	// Must have som uid's
	if ( szUidStr.IsEmpty () )
	{
		return 0;
	}

	// Create our "skip rest" uid list to accumulate uid's that need to be skipped
	// in succeeding filters.
	//
	CPtrUidList SkipRestUidList;

	// Note: Only handle main filters here.
	pFiltList = &(GetFiltersDoc()->m_Filters);

	POSITION pos = pFiltList->GetHeadPosition();

	while (pos)
	{
		CFilter* filt = pFiltList->GetNext(pos);

		// Check for cancel:
		//
		if ( pImapMailbox->EscapePressed() )
		{
			break;
		}

		 if (!filt)
			continue;

		// NOTE:
		// We only handle INCOMING filters here.
		//
		if ( !filt->IsIncoming() )
		{
			continue;
		}

		//
		// Is this a filter that applies to us? 
		// If it has a copy or xfer action to a mailbox on the same server,
		// then we want to  perform the action here.
		// Note: "IsXferFilter()" will return FALSE if it meets a "Skip Rest"
		// ahead of a transfer-to, but TRUE otherwise. 
		//
		BOOL bIsXferFilter = IsXferOnSameServer (pTocDoc, filt);

		// So the test for whether this contains a skip-rest here as well, because we
		// may need to to the ImapMatch in either case.
		//
		BOOL bHasSkipRest = HasSkipRest (filt);

		if (bIsXferFilter || bHasSkipRest)
		{
			// Do the server search and get a UID subset of NewUidList that match
			//  this filter criteria.
			// NOTE: "ImapMatch" does the SEARCH on the server and gets the list of UID's
			// that matcg the filter criteria.
			//
			CPtrUidList SubUidList;

			ImapMatch (pTocDoc, filt, NewUidList, SubUidList);

			// If we got any, do remote transfers.
			if ( SubUidList.GetCount() > 0 )
			{
				// Note: If we are going to be add these to the skip rest list,
				// copy SubUidList to a temporary list:
				//
				CPtrUidList TmpSkipRestUidList;

				if ( bHasSkipRest )
				{
					CPtrUidList DummyUidList;

					ApplyConjunction (IDS_FIO_OR, SubUidList, DummyUidList, TmpSkipRestUidList);

					DummyUidList.DeleteAll ();
				}

				// Handle transfers first:
				//
				if ( bIsXferFilter )
				{
					// If any of the matches are in our SkipRestUidList, remove them:
					//
					CPtrUidList DestUidList;

					ApplyConjunction (IDS_FIO_UNLESS, SubUidList, SkipRestUidList, DestUidList);

					// Do the remote xfers on what we have left:
					//
					CString szStrMatches;

					UidListToCommaString (DestUidList, szStrMatches);

					if (! szStrMatches.IsEmpty() )
					{
						//
						// Did we find matches? If we did, make sure the UID is in NewUidList,
						// perform the filtering, and delete successful transfers from NewUidList.

						DoRemoteTransfers (pTocDoc, filt, NewUidList, szStrMatches);
					}

					DestUidList.DeleteAll ();


				}

				// Handle skip rest. Note: Add our matches even if we transferred them.
				//
				if ( bHasSkipRest )
				{
					// We have to handle SKIP REST at this point. Add the UID's to our global 
					// "SkipRest uid list.
					// Note: Because we're using references, this becomes messy:
					//
					CPtrUidList DestUidList;

					ApplyConjunction (IDS_FIO_OR, SkipRestUidList, TmpSkipRestUidList, DestUidList);

					// We have to clean SkipRestUidList and copy from DestUidList back to
					// SkipRestUidList.
					//
					SkipRestUidList.DeleteAll();

					// The following will do it:
					//
					TmpSkipRestUidList.DeleteAll ();

					ApplyConjunction (IDS_FIO_OR, DestUidList, TmpSkipRestUidList, SkipRestUidList);

					DestUidList.DeleteAll();
				}

				TmpSkipRestUidList.DeleteAll ();
			}

			//
			SubUidList.DeleteAll ();
		}

		// Go handle the next filter.
	}

	// Cleanup:
	SkipRestUidList.DeleteAll ();

	return 1;	
}




/////////////////////////////////////////////////////////////////////
// ImapPersAndXferMatched [PRIVATE]
// Examine "pFilt" to see if it's a filter we'er interested in.
// It must:
// 1. Match our personality,
// 2. Have a "Transfer or Copy" action, 
// 3. Have a Contains or Not Contains verb qualifying the matching text.
//
// Return 0 or 1 indicating the index into pFilt->m_Header[] to find the text to match,
// or -1 if error.
//
int CImapFilterActions::ImapPersAndXferMatched (CTocDoc *pTocDoc, CFilter* pFilt)
{
	int HeaderNumToMatch = -1;

	// Sanity:
	if ( ! (pTocDoc && pFilt) )
	{
		ASSERT (0);
		return -1;
	}

	// Get the CImapMailbox pointer from the TOC.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;

	if ( !pImapMailbox )
	{
		ASSERT (0);
		return -1;
	}

	HeaderNumToMatch = -1;

	// Is one of the header fields "<<Personality>>"?
	if ( !MyCompareRStringI(IDS_FHEADER_PERSONALITY, pFilt->m_Header[0]) )
	{
		HeaderNumToMatch = 1;
	}
	else if ( !MyCompareRStringI(IDS_FHEADER_PERSONALITY, pFilt->m_Header[1]) )
	{
		HeaderNumToMatch = 0;
	}

	// Did we succeed?
	if (HeaderNumToMatch < 0 || HeaderNumToMatch > 1)
		return -1;

	//
	// Get our persoanlity name;
	//
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( pImapMailbox->GetAccountID () );
	if (!pAccount)
	{
		ASSERT (0);
		return -1;
	}

	CString szOurPersona; pAccount->GetName ( szOurPersona );

	//
	// Is the personality same as ours?
	//
	int PersonalityHeader = 1 - HeaderNumToMatch;

	// Matches are case insensitive. Note that personality match supports 
	// all the possible verbs.
	//
	szOurPersona.MakeLower();

	if ( !pFilt->MatchValue (PersonalityHeader, szOurPersona) )
	{
		return -1;
	}

	// Ok. Does this have a Transfer or Copy operation?
	//

	BOOL bGoodFilter = FALSE;

	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if ( (ID_FLT_COPY == pFilt->m_Actions[i]) || (ID_FLT_TRANSFER == pFilt->m_Actions[i]) )
		{
			// Found one.
			bGoodFilter = TRUE;
				break;
		}
	}

	// Found our action?
	if (!bGoodFilter)
	{
		return -1;
	}

	//
	// Is the verb appropriate?? We handle only CONTAINS and NOT CONTAINS.
	// Note: Treat Is Na IsNot the same as CONTAINS and NOT CONTAINS!!!
	//
	int Verb = VERB_FIRST + pFilt->m_Verb[HeaderNumToMatch];

	bGoodFilter = (Verb == IDS_CONTAINS) || (Verb == IDS_DOESNT_CONTAIN) || 
				  (Verb == IDS_IS) || (Verb == IDS_IS_NOT) ;
	

	if (!bGoodFilter)
		return -1;

	// Looks like we succeeded.
	return HeaderNumToMatch;
}



/////////////////////////////////////////////////////////////////////////////
// FormulateImapSearchCriteria [PRIVATE]
// 
// Formulate the given text strings into a comma-separated list of headers and return
// it in "HeaderList". If "pHeader" is <<Body>>, set the bBody flag. If the
// verb is "NOT CONTAINS", set the bNot flag.
//
void CImapFilterActions::FormulateImapSearchCriteria (LPCSTR pHeader, int Verb, 
							LPCSTR pValue, CString& HeaderList, BOOL *pbBody, BOOL *pbNot)
{
	// Make sure of this.
	HeaderList.Empty ();

	// Sanity: Must have a header and a value.
	if ( ! (pHeader && pValue) )
	{
		ASSERT (0);
		return;
	}

	// Initialize these.
	if (pbBody)
		*pbBody = FALSE;

	if (pbNot)
		*pbNot = FALSE;

	// The verb MUST be either CONTAINS, IS, IS NOT or NOT CONTAINS:
	if ((Verb == IDS_CONTAINS) || (Verb == IDS_IS))
	{
		if (pbNot)
			*pbNot = FALSE;
	}
	else if ((Verb == IDS_DOESNT_CONTAIN) || (Verb == IDS_IS_NOT))
	{
		if (pbNot)
			*pbNot = TRUE;
	}
	else
	{
		// Shouldn't get here.
		ASSERT (0);
		return;
	}

	// If <<Body>> was specified, just set the bBody flag and return.
	if (MyCompareRStringI(IDS_FHEADER_BODY, pHeader) == 0)
	{
		if (pbBody)
			*pbBody = TRUE;

		return;
	}

	//
	// Ok. It's a more complicated header spec.
	//

	//
	// Get the header strings in the form we need them. Strip the trailing ":".
	//
	int nc;

	// To:
	CString To = CRString (IDS_FHEADER_TO); 
	nc = To.Find (':');
	if (nc > 0)
		To = To.Left (nc);
	
	// From:
	CString From = CRString (IDS_FHEADER_FROM); 
	nc = From.Find (':');
	if (nc > 0)
		From = From.Left (nc);

	// Subject:
	CString Subject = CRString (IDS_FHEADER_SUBJECT); 
	nc = Subject.Find (':');
	if (nc > 0)
		Subject = Subject.Left (nc);

	// Cc:
	CString Cc = CRString (IDS_FHEADER_CC); 
	nc = Cc.Find (':');
	if (nc > 0)
		Cc = Cc.Left (nc);

	// Reply-To:
	CString ReplyTo = CRString (IDS_FHEADER_REPLY_TO); 
	nc = ReplyTo.Find (':');
	if (nc > 0)
		ReplyTo = ReplyTo.Left (nc);

	// What's our spec?
	//	
	// Any header?	
	if (!MyCompareRStringI(IDS_FHEADER_ANY, pHeader))
	{
		HeaderList = To + "," + From + "," + Subject + "," + Cc + "," + ReplyTo;
	}
	// Any Recipient?
	else if (!MyCompareRStringI(IDS_FHEADER_ADDRESSEE, pHeader))
	{
		HeaderList = To + "," + Cc;
	}
	// To:
	else if (!MyCompareRStringI(IDS_FHEADER_TO, pHeader))
	{
		HeaderList = To;
	}
	// From:
	else if (!MyCompareRStringI(IDS_FHEADER_FROM, pHeader))
	{
		HeaderList = From;
	}
	// Subject:
	else if (!MyCompareRStringI(IDS_FHEADER_SUBJECT, pHeader))
	{
		HeaderList = Subject;
	}
	// Cc:
	else if (!MyCompareRStringI(IDS_FHEADER_CC, pHeader))
	{
		HeaderList = Cc;
	}
	// Reply-to:
	else if (!MyCompareRStringI(IDS_FHEADER_REPLY_TO, pHeader))
	{
		HeaderList = ReplyTo;
	}
	else
	{
		// Custom header: Use header as-is:
		HeaderList = pHeader;

		nc = HeaderList.Find (':');
		if (nc > 0)
			HeaderList = HeaderList.Left (nc);


		HeaderList.TrimRight();
		HeaderList.TrimLeft();
	}
}




// EndFiltering
//
// Finishes up the filtering after all messages have been processed
//
BOOL CImapFilterActions::ImapEndFiltering()
{
	BOOL Success = TRUE;
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

			if (toc->GetView())
				toc->GetView()->Fumlub();
		}
	}

	// Open up remote mailboxes that had message filtered into them
	// during a pre-filter.
	// 
	pos = m_FilterMboxList.GetHeadPosition();
	while (pos)
	{
		// Open up the mailbox if it isn't open already (except Trash),
		LPCSTR pPathname = m_FilterMboxList.GetNext(pos);

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
					if (toc->GetView())
						toc->GetView()->Fumlub();
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
			UpdateTocAfterFiltering (toc);
		}

		//
		// If the mailbox was just opened or was already opened, then
		// be sure to scroll to the "First Unread Message of the Last
		// Unread Block" ... you know, Fumlub.
		// 
		if (toc->GetView())
			toc->GetView()->Fumlub();
	}
	
	// Dump Report
	pos = m_NotifyReportList.GetHeadPosition();
	if (pos)
	{
		// Find active Filter Report window, if any.
		CMainFrame* pMainFrame = (CMainFrame *) AfxGetMainWnd();
		ASSERT_KINDOF(CMainFrame, pMainFrame);
		CFilterReportView* pView = pMainFrame->GetActiveFilterReportView();
		if (pView)
		{
			// Do the time stamp thang
			time_t now;
			time( &now );
			while (pos)
			{
				CStringCount *mboxName = m_NotifyReportList.GetNext(pos);
				pView->AddEntry( mboxName->m_Count, (const char*)mboxName->m_Name, now );
			}
			if (m_bDoFilterReport)
				pMainFrame->PostMessage(WM_COMMAND, IDM_VIEW_FILTER_REPORT);	// activate the report window
		}
	}


	// PlaySounds();
	pos = m_SoundList.GetHeadPosition();
	while (pos)
	{
		CString snd = m_SoundList.GetNext(pos);
		if (!snd.IsEmpty()) 
			PlaySound(snd, NULL, SND_SYNC | SND_FILENAME | SND_NODEFAULT);
		if (EscapePressed(1))
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

	// Clea new (IMAP) lists.
	ClearAllImapLists();

	return (Normal);
}



//
// Clear all lists that we added.
//
void CImapFilterActions::ClearAllImapLists ()
{
	// List of remote mailboxes we transfered messages to.
	m_FilterMboxList.RemoveAll ();
}
	




////////////////////////////////////////////////////////////////////////////////////
// DoRemoteTransfers [PRIVATE]
//
// FUNCTION
// Do the actual message transfer to the remote destination mailbox
// based on the given filter (pFilt).
// "pResults" is a comma-separated list of UID's that match an IMAP SEARCH request.
// If the copy/transfer succeeds, remove the uid's from NewUidList.
// END FUNCTION

// NOTE:
// pSumList may be NULL, but if it isn't, whenever we do an XFER, look for the 
// matching summaries and flag the messages as \deleted.
//
////////////////////////////////////////////////////////////////////////////////////
BOOL CImapFilterActions::DoRemoteTransfers (CTocDoc *pTocDoc, CFilter *pFilt, CPtrUidList& NewUidList, LPCSTR pUids, CSumList *pSumList /* = NULL */)
{
	BOOL	Copy	= TRUE;
	BOOL	bResult = FALSE;
	BOOL	bUserAborted = FALSE;

	// Sanity:
	if ( ! (pTocDoc && pFilt && pUids) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Must have one of these.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);
		return FALSE;
	}

	// Get the server's name we're xfering on.
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( pImapMailbox->GetAccountID() );
	if (!pAccount)
	{
		ASSERT (0);
		return FALSE;
	}

	CString szServerName; pAccount->GetAddress (szServerName);

	// Save the previous progress state for retrieval below.
	BOOL bWasInProgress = FALSE;

	// If this is being done in the foreground, raise the progress bar.
	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			PushProgress();
			bWasInProgress = TRUE;
		}
	}

	//		
	// Loop through the actions and perform only copy/transfer to a remote mailbox.
	//
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Check for cancel:
		//
		if ( pImapMailbox->EscapePressed() )
		{
			bUserAborted = TRUE;
			break;
		}

		switch(pFilt->m_Actions[i])
		{
			// Found one. Get the destination mailbox.
			case ID_FLT_COPY:
			case ID_FLT_TRANSFER:
				if (!pFilt->m_CopyTo[i].IsEmpty() || pFilt->IsTransfer())
				{
					CString mboxPath;

					Copy = TRUE;

					if (pFilt->m_Actions[i] == ID_FLT_TRANSFER)
					{
						mboxPath = pFilt->m_Mailbox;
						Copy = FALSE;
					}
					else
					{
						mboxPath = pFilt->m_CopyTo[i];
						Copy = TRUE;
					}

					// Find the QCImapMailboxCommand object and get the imap name from it.
					// Note: pCommand is the DESTINATION mailbox.
					//
					QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (mboxPath);
					if (!pCommand)
					{
						ASSERT (0);
						break;	// Out of switch.
					}

					if( ! pCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
					{
						ASSERT (0);
						break;	// Out of switch.
					}

					// If this is not on the same server, ignore it.
					if (! IsOnSameServer (pTocDoc, (QCImapMailboxCommand *) pCommand) )
					{
						break;
					}

					CString szImapName = ((QCImapMailboxCommand *)pCommand)->GetImapName ();

					// Show some kind of progress.  
					if ( ::IsMainThreadMT() )
					{
						CString szBuf;

						szBuf.Format ( CRString (IDS_IMAP_XFERING_TOREMOTE), 
									   Copy ? CRString (IDS_IMAP_COPYING) : CRString (IDS_IMAP_TRANSFERING),
									   szImapName, szServerName);

						MainProgress( szBuf );
					}

					// Do the transfer on the server.
					bResult = SUCCEEDED (pImapMailbox->XferMessagesOnServer (pUids, szImapName, Copy));

					// If we transferred messages into a destination mailbox, update the
					// command object's UnreadStatus to US_YES.
					// BUG ALERT!! This is NOT really correct, because the messages may be
					// new to us but may have been read by others!!! We sould 
					// really check the IMAP \\SEEN flags here but thath would involve
					// a FETCH FLAGS, which we DON'T want to do here!!!
					//
					if (bResult)
					{
						pCommand->Execute( CA_UPDATE_STATUS, ( void* ) US_YES );

						// Keep track of the highest UID we've xferred.
						//
						unsigned long ulMax = UIDSTRFindLargest (pUids);

						if (ulMax > m_HighestUidXferred)
							m_HighestUidXferred = ulMax;
					}


					// Flag the summaries as \deleted if we transferred.
					// Note: If this was part of a pre-filter, "pSumList" would be NULL
					// so this wouldn't get executed (which is what we want).
					//
					if (bResult && pSumList && !Copy)
					{
						SetLocalSummariesFlag (pSumList, pUids, IMFLAGS_DELETED, TRUE);
					}


					// If "Copy" was FALSE, messages were flagged for deletion.
					// Signal that we should do an EXPUNGE..
					if (!Copy)
						m_bPreFilterXferred = TRUE;

					// If we succeeded, remove uid's from NewUidList.
					if (bResult)
					{
						// Count how many messages we copied/xferred. This is used below 
						// in adding the destination mbox name to the filter report.
						//
						int nMsgsXferred = CountUidsInCommaString (pUids);

						RemoveUidsFromList ( NewUidList, pUids);

						//
						// Add the command object's pathname to m_FilterMboxList so we can open
						// the toc after filtering.
						//

						LPCSTR pPathname = pCommand->GetPathname ();

						// Note: For IMAP, since these are remote mailboxes, add to this list
						// so that later, we can check to see if the user has them open. If he
						// does, we'd need to update those mailboxes.
						//
						if ( pPathname && (m_FilterMboxList.Find(pPathname) == NULL) )
						{
							m_FilterMboxList.AddTail (pPathname);
						}

						// Add to filter report if we need to:
						if ( (pFilt->m_NotfyUserNR < 0 && m_bDoFilterReport)  || 
							 (pFilt->m_NotfyUserNR > 0 && (pFilt->m_NotfyUserNR & NO_REPORT )) )
						{
							CString	  szFullMailboxName;

							szFullMailboxName = g_theMailboxDirector.BuildNamedPath( pCommand );

							// We need to add this name to the report list "nMsgsXferred" times
							// becuase that's how the report window is told how many messages were
							// transfered.
							//
							for (int i = 0; i < nMsgsXferred; i++)
							{
								m_NotifyReportList.Add( szFullMailboxName );
							}
						}
					}
				}

				break;
			default:
				break;

		}	// switch
	} // for.

	//
	// Restore progress.
	//
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress();
		else
			CloseProgress();
	}

	if (bUserAborted)
		bResult = FALSE;

	return bResult;
}






/////////////////////////////////////////////////////////////////////////
// RemoveUidsFromList [PRIVATE]
//
// Remove the Uid's in the comma-separated list "pUids" from NewUidList.
//
/////////////////////////////////////////////////////////////////////////
BOOL CImapFilterActions::RemoveUidsFromList (CPtrUidList& NewUidList, LPCSTR pUids)
{
	TCHAR	Comma = ',';
	TCHAR	buf [64];
	LPCSTR	p = NULL;
	LPCSTR	q = NULL;

	// Sanity:
	if (! pUids )
	{
		ASSERT (0);
		return FALSE;
	}

	// Loop through Uids.
	//
	p = pUids;

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
			POSITION pos = NewUidList.GetHeadPosition ();
			while (pos)
			{
				CImapFlags *pF = (CImapFlags *) NewUidList.GetAt (pos);
				if  (pF && (pF->m_Uid == Uid) )
				{
					// Found it. Remove it.
					NewUidList.RemoveAt (pos);
					delete pF;
					pF = NULL;
					
					// Get out of loop.
					break;
				}

				NewUidList.GetNext (pos);
			}
		}
	}

	return TRUE;
}




// DoManualFiltering [PUBLIC]
//
// Public hook to do manual filtering.
//
void CImapFilterActions::DoManualFiltering (CTocDoc *pTocDoc, CSummary *SingleSum /* = NULL */)
{
	CSumList SumList;
	CTocView *View = NULL;
	BOOL	bWasInProgress = FALSE;
	BOOL	bUserAborted = FALSE;

	// Sanity:
	if (!pTocDoc)
		return;

	// Must put up progress bar!!
	// We can put up a progress window here!
	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		MainProgress(CRString(IDS_IMAP_FILTERING_MSGS));
	}


	View = pTocDoc->GetView ();

	POSITION pos = pTocDoc->m_Sums.GetHeadPosition ();
	POSITION NextPos;
	int i = 0;

	// Add selected summaries.
	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		// Check for cancel:
		//
		if ( pTocDoc->m_pImapMailbox && pTocDoc->m_pImapMailbox->EscapePressed() )
		{
			bUserAborted = TRUE;
			break;
		}

		CSummary* Sum = pTocDoc->m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// Ok. We've got this. Add to list.
		SumList.Add (Sum);

		if (SingleSum)
			break;
	}

	// Continue only if we got summaries.
	if (!bUserAborted && SumList.GetCount() > 0)
	{
		// Include same server xfers but omit incoming filters.
		//
		DoPostFiltering (pTocDoc, SumList, TRUE, FALSE, TRUE);
	}

	// Clear these:
	SumList.RemoveAll ();

	// If we put up progress, take it down.
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress();
	}
}






// DoPostFiltering [PUBLIC]
//
// Does filtering on messages in the given pSumList. 
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
int CImapFilterActions::DoPostFiltering (CTocDoc *pTocDoc, CSumList& SumList, BOOL bDoSameServerXfers,
							BOOL bDoIncoming, BOOL bDoManual)
{
	POSITION pos, NextPos;
	BOOL	 bUserAborted = FALSE;

	// If SumList is empty, nothing to do.
	if ( SumList.GetCount() == 0 )
		return 0;

	// Must have a toc.
	if (!pTocDoc)
	{
		ASSERT (0);
		return 0;
	}

	// Must be an IMAP mailbox.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);
		return 0;
	}

	//
	// Make sure we have filters.
	//
	CFilterList *pFiltList = &(GetFiltersDoc()->m_Filters);
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

	// Get a UID list for the summaries we have in SumList.
	CPtrUidList TotalUidList;

	SumListToUidList (SumList, TotalUidList);

	// Initialize or global SkipResrUidList:
	//
	CPtrUidList SkipRestUidList;

	// Keep track new msgs that are left in  INbox after xferring:
	//
	m_ulNewMsgsLeftInInbox = SumList.GetCount();

	// Loop through filters.
	pos = pFiltList->GetHeadPosition();

	for (NextPos = pos; pos; pos = NextPos)
	{
		// Check for cancel:
		//
		if ( pImapMailbox->EscapePressed() )
		{
			bUserAborted = TRUE;
			break;
		}

		CFilter* filt = pFiltList->GetNext(NextPos);
		BOOL bApplyFilt = FALSE;

		 if (!filt)
			continue;

		// Ignore incoming?
		if ( bDoIncoming && filt->IsIncoming() )
		{
			bApplyFilt = TRUE;
		}

		// Ignore manual filters?
		if ( bDoManual && filt->IsManual() )
		{
			bApplyFilt = TRUE;
		}

		// bApplyFilt MUIST be TRUE at this point!!
		if (!bApplyFilt)
			continue;

		// We have to handle SKIP REST here as well: We may not be applying the
		// filter if it's a Xfer on same server, but if it contains a SKIP REST,
		// we still have to add it to our skip rest UID list!!
		//

		BOOL bHandlingFilter = TRUE;

		//  Must we handle same-server xfers?
		//
		if ( !bDoSameServerXfers && IsXferOnSameServer (pTocDoc, filt) )
			bHandlingFilter = FALSE;

		// Does this filter contain a skip-rest?
		//
		BOOL bHasSkipRest = HasSkipRest (filt);

		// We need to do an ImapMatch if either is TRUE:
		//
		if ( bHandlingFilter || bHasSkipRest)
		{
			// Get a UID subset of the summaries that match this filter criteria.
			CPtrUidList SubUidList;

			ImapMatch (pTocDoc, filt, TotalUidList, SubUidList);

			// Perform the actions. Note: This may result in some uid's 
			// from TotalUidList being removed so further filters don't act
			// on them. This may also result in summaries being deleted from 
			// SumList.
			//
			if (SubUidList.GetCount () > 0)
			{
				// Note: If we are going to be add these to the skip rest list,
				// copy SubUidList to a temporary list:
				CPtrUidList TmpSkipRestUidList;

				if ( bHasSkipRest )
				{
					CPtrUidList DummyUidList;

					ApplyConjunction (IDS_FIO_OR, SubUidList, DummyUidList, TmpSkipRestUidList);

					DummyUidList.DeleteAll ();
				}

				if ( bHandlingFilter )
				{
					// Before we perform the action, we must remove any UID's in 
					// SkipRestUidList.
					//
					CPtrUidList DestUidList;

					ApplyConjunction (IDS_FIO_UNLESS, SubUidList, SkipRestUidList, DestUidList);

					PerformAction (pTocDoc, filt, SumList, TotalUidList, DestUidList);

					DestUidList.DeleteAll();
				}

				// If this filter had a SKIP_REST, we must add ALL these UID's to the SkipRestUidList
				// so no future actions is performed on them. NOTE: We do this AFTER the operation is 
				// performed!!
				//
				if ( bHasSkipRest )
				{
					// We have to handle SKIP REST at this point. Add the UID's to our global 
					// "SkipRest uid list.
					// Note: Because we're using references, this becomes messy:
					//
					CPtrUidList DestUidList;

					ApplyConjunction (IDS_FIO_OR, SkipRestUidList, TmpSkipRestUidList, DestUidList);

					// We have to clean SkipRestUidList and copy from DestUidList back to
					// SkipRestUidList.
					//
					SkipRestUidList.DeleteAll();

					// The following will do it:
					//
					TmpSkipRestUidList.DeleteAll ();

					ApplyConjunction (IDS_FIO_OR, DestUidList, TmpSkipRestUidList, SkipRestUidList);

					DestUidList.DeleteAll();
				}

				TmpSkipRestUidList.DeleteAll ();
			}

			// We're paranoid..
			SubUidList.DeleteAll ();
		}
	} // While.

	if (!bUserAborted)
	{
		// If we accumulated any UID's which need a change of READ, UNREAD or 
		// ANSWERED, or DELETED server flags, go perform those block actions now.
		//
		DoServerStatusChanges (pTocDoc, SumList);

		// If there are messages to be completely fetched (as a result of 
		// a "Server Options" filter action, go fetch those messages now.
		// The UID's would have been accumulated in m_szFetchUids.
		//
		DoFetchRequestedMessages (pTocDoc, SumList);
	}

	// Did we filter out all new msgs from TOC??
	//
	if (m_ulNewMsgsLeftInInbox > 0)
	{
		// Make sure the toc is in the "OpenMailbox" list.
		//
		if (m_FilterTocList.Find(pTocDoc) == NULL)
			m_FilterTocList.AddTail(pTocDoc);
	}


	// Make sure we reset these:
	m_szAnsweredUids.Empty();
	m_szReadUids.Empty();
	m_szUnreadUids.Empty();
	m_szDeletedUids.Empty();
	m_szFetchUids.Empty();

	// Cleanup:
	TotalUidList.DeleteAll ();
	SkipRestUidList.DeleteAll ();

	return 1;
}



// SetImapMsgStatus [PUBLIC]
//
// Called by the filter's ImapLocalAction routine if the filter has
// a Make status action. This adds the summary's UID to the
// appropriate comma-separated list so we can later do a block
// STORE FLAGS on the server.
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




// ImapSetServerOpt [PUBLIC]
//
// Called by the filter's ImapLocalAction routine if the filter has
// a Make status action. This adds the summary's UID to the
// appropriate comma-separated list so we can later do a block
// STORE FLAGS on the server.
//
void CImapFilterActions::ImapSetServerOpt (CSummary *pSum, unsigned int uiServerOpt)
{
	if (uiServerOpt & SO_DELETE)
		AddUidToStatusString (pSum, m_szDeletedUids);

	if (uiServerOpt & SO_FETCH)
		AddUidToStatusString (pSum, m_szFetchUids);
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



// ImapMatch [PRIVATE]
//
// Ask the IMAP server for a subset of "TotalUidList" that matches the criteria in "filt"
// Return the list in "SubUidList".
// NOTE:
// This works regardless of the filter action, i.e., it doesn't only work for Xfer/Copy!
//
void CImapFilterActions::ImapMatch (CTocDoc *pTocDoc, CFilter *filt, CPtrUidList& TotalUidList, CPtrUidList& SubUidList)
{
	int		HeaderNum;
	BOOL	bPersonalityMatched = FALSE;
	CPtrUidList List1, List2;
	BOOL	bWasInProgress = FALSE;

	// Must have these:
	if (!(pTocDoc && filt) )
		return;

	// Must be an IMAP mailbox.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
	{
		ASSERT (0);
		return;
	}

	// Must have uid's.
	if (TotalUidList.GetCount() == 0)
		return;

	// Formulate TotalUidList into a comma-string for UIDFind.
	CString szUidStr;

	UidListToCommaString (TotalUidList, szUidStr);

	// Must have some uid's
	if ( szUidStr.IsEmpty () )
	{
		return;
	}

	// Tell user what we're doing.
	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		CString buf;
		buf.Format (CRString(IDS_IMAP_SEARCHING_SERVER), filt->m_Name);

		MainProgress(buf);
	}


	// Initialize this:
	SubUidList.DeleteAll ();

	// Do first criterion.
	HeaderNum = 0;

	//
	// Is the first of the header fields "<<Personality>>"?
	//
	if ( !MyCompareRStringI(IDS_FHEADER_PERSONALITY, filt->m_Header[HeaderNum]) )
	{
		// If this is a personality match, then all the UID's in TotalUidList
		// match! Create our first uid list (List1).
		//
		if (IsOurPersonality (pTocDoc, filt, HeaderNum))
		{
			CopyUidList (TotalUidList, List1);

			bPersonalityMatched = TRUE;
		}
	}
	else
	{
		// Not personality. Search server for match.
		//
		// Formulate the search criteria for searching the IMAP server.
		// Returns a comma-seoarated list of header names in "HeaderList", along
		// with setting the values of bBody to TRUE if a body search was specified,
		// and setting bNot to TRUE if the verb is a negative.
		//
		CString HeaderList;
		BOOL bBody = FALSE;
		BOOL bNot = FALSE;
		int  Verb = VERB_FIRST + filt->m_Verb[HeaderNum];

		FormulateImapSearchCriteria (filt->m_Header[HeaderNum], Verb,
						 filt->m_Value[HeaderNum], HeaderList, &bBody, &bNot);

		if ( bBody || (!HeaderList.IsEmpty()) )
		{
			//
			// Ask the IMAP server to get messages within our range that match our criterion.
			// NOTE here: Because of IMAP's limitations, we can only do the following 
			// verbs: "Contains", "Doesn't contain".
			//
			CString szResults;
			CString szText = filt->m_Value [HeaderNum];

			pImapMailbox->UIDFind ( HeaderList, bBody, bNot, szText, szUidStr, szResults);

			// 
			// YUCK - Searching sets the \seen flag of some servers (UW). 
			// Unset them here before we do the transfers!!
			//
// (4/1/98)			pImapMailbox->ResetUnseenFlags (TotalUidList);

			//
			// Did we find matches? If we did, add the uid's to List1.
			//
			if ( !szResults.IsEmpty () )
			{
				// Convert froma comma-separated list to an ordered UID list. 
				CommaListToUidList (szResults, List1);
			}
		}
	}	// Else.

	// Can we ignore the second criterion??
	//
	if (CONJUNCTION_FIRST + filt->m_Conjunction == IDS_FIO_IGNORE)
	{
		CopyUidList (List1, SubUidList);

		// If we put up progress, take it down.
		if ( ::IsMainThreadMT() )
		{
			if (bWasInProgress)
				PopProgress ();
			else
				CloseProgress();
		}

		return;
	}


	// Go handle the next criterion.
	HeaderNum = 1;

	//
	// Is this "<<Personality>>"?
	//
	if ( !MyCompareRStringI(IDS_FHEADER_PERSONALITY, filt->m_Header[HeaderNum]) )
	{
		// If this is a personality match, then all the UID's in TotalUidList
		// match!
		//
		if (IsOurPersonality (pTocDoc, filt, HeaderNum))
		{
			// If first one is personality, ignore this.
			if (!bPersonalityMatched)
			{
				CopyUidList (TotalUidList, List2);

				bPersonalityMatched = TRUE;
			}
		}
	}
	else
	{
		// Not personality. Search server for for match.
		//
		// Formulate the search criteria for searching the IMAP server.
		// Returns a comma-seoarated list of header names in "HeaderList", along
		// with setting the values of bBody to TRUE if a body search was specified,
		// and setting bNot to TRUE if the verb is a negative.
		//
		CString HeaderList;
		BOOL bBody = FALSE;
		BOOL bNot = FALSE;
		int  Verb = VERB_FIRST + filt->m_Verb[HeaderNum];

		FormulateImapSearchCriteria (filt->m_Header[HeaderNum], Verb,
						 filt->m_Value[HeaderNum], HeaderList, &bBody, &bNot);

		//
		// Ask the IMAP server to get messages within our range that match our criterion.
		// NOTE here: Because of IMAP's limitations, we can only do the following 
		// verbs: "Contains", "Doesn't contain".
		//
		CString szResults;
		CString szText = filt->m_Value [HeaderNum];

		pImapMailbox->UIDFind ( HeaderList, bBody, bNot, szText, szUidStr, szResults);

		// 
		// YUCK - Searching sets the \seen flag of some servers (UW). 
		// Unset them here before we do the transfers!!
		//
// (4/1/98)		pImapMailbox->ResetUnseenFlags (TotalUidList);

		//
		// Did we find matches? If we did, Merge the uid's into 
		//
		if ( !szResults.IsEmpty () )
		{
			// Convert froma comma-separated list to an ordered UID list. 
			CommaListToUidList (szResults, List2);
		}
	}

	// Now go merge List1 and List2 using whatever conjunction we have.
	// Return the result in SubUidList.
	//

	// Get the conjunction.
	int Conjunction = CONJUNCTION_FIRST + filt->m_Conjunction;

	ApplyConjunction (Conjunction, List1, List2, SubUidList);

	// If we put up progress, take it down.
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress();
	}

}




//////////////////////////////////////////////////////////////////////////////////
// PerformAction [PRIVATE}
//
// Perform the action specified by the filter "filt" to the summaries in SumList.
// Do this by wading through SubUidList, find the corresponding Summary in
// SumList and do the action on it, then delete the uid from TotalUidList
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
int CImapFilterActions::PerformAction (CTocDoc *pTocDoc, CFilter *filt, CSumList& SumList,
							CPtrUidList& TotalUidList, CPtrUidList& SubUidList)
{
	POSITION pos, next;
	BOOL	NumSavedXferActions = 0;
	int		iXferActions [NUM_FILT_ACTS];
	int		SavedActions [NUM_FILT_ACTS];
	BOOL	bUserAborted = FALSE;
	int		FiltActs = FA_NORMAL;

	// Sanity:
	if (! (pTocDoc && filt) )
		return FiltActs;

	// If no uid's to act on, don't continue.
	if (SubUidList.GetCount () <= 0)
		return FiltActs;

	if ( pTocDoc->m_pImapMailbox && pTocDoc->m_pImapMailbox->EscapePressed() )
	{
		return FiltActs;
	}

	//
	// If the action we're going to be performing is a transfer to a mailbox on the same server,
	// do a batch xfer.
	// Implement this by saving any matching COPY or TRANSFER in iXferActions and SavedActions,
	// setting the corresponding actions within the filter to NONE, run the normal
	// Action code on the remaining actions, then coming back and do the
	// COPY or TRANSFER's on the same server.
	//

	// This keeps the number of saved actions.
	NumSavedXferActions = 0;

	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if ( pTocDoc->m_pImapMailbox && pTocDoc->m_pImapMailbox->EscapePressed() )
		{
			bUserAborted = TRUE;
			break;
		}

		if ( (filt->m_Actions[i] == ID_FLT_COPY) || (filt->m_Actions [i] == ID_FLT_TRANSFER) )
		{
			CString mboxPath;

			// Is the destination mailbox on the same server??
			if (filt->m_Actions[i] == ID_FLT_TRANSFER)
			{
				mboxPath = filt->m_Mailbox;
			}
			else
			{
				mboxPath = filt->m_CopyTo[i];
			}

			// Find the QCImapMailboxCommand object and get the imap name from it.
			QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname (mboxPath);

			if (pCommand)
			{
				if( pCommand->IsKindOf( RUNTIME_CLASS( QCImapMailboxCommand ) )  )
				{
					if (IsOnSameServer (pTocDoc, (QCImapMailboxCommand *) pCommand) )
					{
						// We've found one.
						iXferActions [NumSavedXferActions] = i;
						SavedActions [NumSavedXferActions] = filt->m_Actions [i];

						// Set the action temporarily to none!!!
						filt->m_Actions [i] = ID_FLT_NONE;

						NumSavedXferActions++;
					}
				}
			}
		}
	}

	// If user pressed stop, just go put actions back in.
	if (bUserAborted)
	{
		if (NumSavedXferActions > 0)
		{
			// Put back the actions into the filter.
			for (i = 0; i < NumSavedXferActions; i++)
			{
				int ActionIndex = iXferActions [i];

				filt->m_Actions [ActionIndex] = SavedActions [i];
			}
		}

		return FiltActs;
	}

	// This filters one summary at a time. Put up normal progress bar.
	BOOL bWasInProgress = FALSE;
	int nCount = SubUidList.GetCount ();


	// Tell user what we're doing.
	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		if (nCount > 0)
		{
			::CountdownProgress(CRString(IDS_FILTER_MESSAGES_LEFT), nCount);
			::Progress(-1, NULL, -1);
		}
	}


	// Do the rest of the filtering::

	// Stop redraw of the TOC while this is going on:
	CTocView *pView = pTocDoc->GetView ();

	if (pView )
		pView->m_SumListBox.SetRedraw(FALSE);

	// Wade through SubUidList.
	pos = SubUidList.GetHeadPosition ();

	for (next = pos; pos; pos = next)
	{
		if ( pTocDoc->m_pImapMailbox && pTocDoc->m_pImapMailbox->EscapePressed() )
		{
			bUserAborted = TRUE;
			break;
		}

		CImapFlags *pF = (CImapFlags *)SubUidList.GetNext (next);

		if (!pF)
			continue;

		unsigned long Uid = pF->m_Uid;
		if (!Uid)
			continue;

		// Find the summary.
		POSITION pSumPos = FindInSumlist (SumList, Uid);
		if (pSumPos)
		{
			CSummary *pSum = SumList.GetAt (pSumPos);

			if (!pSum)
				continue;

			// Well, perform the action on the summary.

			FiltActs |= filt->ImapLocalAction (pSum, this);

			// Signal that we may need to do an expunge.
			//
			if ( (FiltActs & FA_TRANSFER) && !filt->IsCopyTo() )
			{
				m_bPostFilterXferred = TRUE;

				// Keep tab of number of msgs left in Inbox:
				//
				m_ulNewMsgsLeftInInbox--;
			}

			// If pSum is no longer in the TOC, remove it.
			if ( !pTocDoc->m_Sums.Find (pSum) )
			{
				SumList.RemoveAt (pSumPos);

				// Remove it from SubUidList as well because we can't operate on
				// it anymore (say for saved XFET actions).
				SubUidList.RemoveAt (pos);

				delete pF;

				// Remove this uid from TotalUidList as well.
				RemoveUidFromUidList (TotalUidList, Uid);
			}

			::DecrementCountdownProgress();
		}
	}

	// If we put up progress, take it down.
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress();
	}

	// If we have any saved xfer/copies, go do those now.
	// Note: We MUST put the actions back in!!.
	//
	if (NumSavedXferActions > 0)
	{
		// Put back the actions into the filter.
		for (i = 0; i < NumSavedXferActions; i++)
		{
			int ActionIndex = iXferActions [i];

			filt->m_Actions [ActionIndex] = SavedActions [i];
		}

		if (!bUserAborted)
		{
			// If we don't have any more uid's to act on, don't continue.
			if (SubUidList.GetCount () > 0)
			{
				// Format SubUidList into a comma-separated list.
				CString szUids;
		
				UidListToCommaString (SubUidList, szUids);

				if ( ! szUids.IsEmpty () )
				{
					// Now pass that to DoRemoteTransfers.

					DoRemoteTransfers (pTocDoc, filt, TotalUidList, szUids, &SumList);
				}
			}
		}
	}

	// This should update \deleted flags, etc!!.
	if (pView )
		pView->m_SumListBox.SetRedraw(TRUE);

	return FiltActs;
}





// DoServerStatusChanges [PRIVATE]
//
// If there are any UID's in the szAnsweredUids, etc., strings, go set their server status.
//
// NOTE: The 
void CImapFilterActions::DoServerStatusChanges (CTocDoc *pTocDoc, CSumList& SumList)
{
	// Sanity:
	if (!pTocDoc)
		return;

	// Must have a valid CImapMailbox object.
	CImapMailbox *pImapMailbox = pTocDoc->m_pImapMailbox;
	if (!pImapMailbox)
		return;
		
	// Do READ changes.
	if ( !m_szReadUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->ImapSetSeenFlag (m_szReadUids)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&SumList, m_szReadUids, IMFLAGS_SEEN, TRUE);
		}
	}

	// Do UNREAD changes.
	if ( !m_szUnreadUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->ImapUnsetSeenFlag (m_szUnreadUids)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&SumList, m_szUnreadUids, IMFLAGS_SEEN, FALSE);
		}
	}

	// Do REPLIED changes.
	if ( !m_szAnsweredUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->ImapSetAnsweredFlag (m_szAnsweredUids)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&SumList, m_szAnsweredUids, IMFLAGS_ANSWERED, TRUE);
		}
	}

	// Do DELETED changes (This is from a "Server Options" filter action!!!.
	if ( !m_szDeletedUids.IsEmpty () )
	{
		// Set status:
		if ( SUCCEEDED (pImapMailbox->DeleteMessagesFromServer (m_szDeletedUids, FALSE)) )
		{
			// Set the local summary flags.
			SetLocalSummariesFlag (&SumList, m_szDeletedUids, IMFLAGS_DELETED, TRUE);
		}
	}
}




// DoFetchRequestedMessages [PRIVATE]
// If a filter with a "Server Options" action found UID matches, wade through
// m_szFetchUid's and download the message if it was only minimally downloaded.
// Make sure the UID is in SumList before doing the download.
//
void CImapFilterActions::DoFetchRequestedMessages (CTocDoc *pTocDoc, CSumList& SumList)
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
				pos = FindInSumlist (SumList, Uid);
				if (pos)
				{
					pSum = SumList.GetAt (pos);
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
		if ( (ID_FLT_COPY == pFilt->m_Actions[i]) || (ID_FLT_TRANSFER == pFilt->m_Actions[i]) )
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
					(m_szMBFilename.CompareNoCase (toc->MBFilename()) == 0) )
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

//
// Get the uid's from a summary list into a CPtrUidList.
//
void SumListToUidList (CSumList& SumList, CPtrUidList& UidList)
{
	POSITION pos, next;

	//
	// Clear this first, including the objects themselves.
	//
	UidList.DeleteAll ();

	pos = SumList.GetHeadPosition();

	for (next = pos; pos; pos = next)
	{
		CSummary* pSum = SumList.GetNext(next);

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
	// Determine UidFirst and UidLast from NewUidList. These must be valid.
	// Note: NewUidList is ordered in ascending UID.
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
	POSITION NextPos;
	CString sUid;

	for (NextPos = pos; pos; pos = NextPos)
	{
		CImapFlags *pF = (CImapFlags *) UidList.GetNext (NextPos);

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
int CountUidsInCommaString (LPCSTR pUids)
{
	const char *p, *q;
	const char Comma = ',';
	int nMsgs = 0;

	if (!pUids)
		return 0;

	p = pUids;

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



// Loop through SumList looking for Uid. Return the POSITION.
//
POSITION FindInSumlist (CSumList& SumList, unsigned long Uid)
{
	POSITION pos, next;

	pos = SumList.GetHeadPosition ();

	for (next = pos; pos; pos = next)
	{
		CSummary *pSum = SumList.GetNext (next);

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
				if (!pFilt->m_CopyTo[i].IsEmpty() || pFilt->IsTransfer())
				{
					CString mboxPath;

					if (pFilt->m_Actions[i] == ID_FLT_TRANSFER)
					{
						mboxPath = pFilt->m_Mailbox;
					}
					else
					{
						mboxPath = pFilt->m_CopyTo[i];
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




static
BOOL HasSkipRest (CFilter *pFilt)
{
	BOOL bResult = FALSE;

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
void SetLocalSummariesFlag (CSumList *pSumList, LPCSTR pUids, unsigned long ulBitFlags, BOOL bSet)
{
	char buf [24];
	char Comma = ',';

	// Sanity:
	if (! (pSumList && pUids) )
		return;

	const char *p = pUids;
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
static unsigned long UIDSTRFindLargest (LPCSTR pUids)
{
	unsigned long ulMax = 0;
	unsigned long Uid;
	char buf [24];
	char Comma = ',';
	const char *p;
	const char *q;
	size_t len;

	if (!pUids)
		return 0;

	// Loop through list:
	//

	p = pUids;

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




#endif // IMAP4


