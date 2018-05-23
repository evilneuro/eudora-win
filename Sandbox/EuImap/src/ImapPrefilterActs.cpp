// ImapPreFilterActs.cpp
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
#include "QCUtils.h"

// We olly use stuff in these header files during a copy of filters,
// which must be in the main thread.
//
#include "progress.h"
#include "summary.h"
#include "tocdoc.h"
#include "tocview.h"
#include "filtersd.h"
#include "TaskInfo.h"

// Imap
#include "imapfol.h"
#include "ImapTypes.h"
#include "imapacct.h"
#include "imapactl.h"
#include "ImapSum.h"
#include "ImapPreFiltersd.h"
#include "ImapPreFilterActs.h"
#include "ImapConnectionMgr.h"

#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

#include "QCImapMailboxCommand.h"
#include "mainfrm.h"

extern QCMailboxDirector	g_theMailboxDirector;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


#ifdef EXPIRING
	// The Evaluation Time Stamp object
	#include "timestmp.h"
	extern CTimeStamp	g_TimeStamp;
#endif


#define NUM_FILT_ACTS 5

// STATIC FUNCIONS


void SumListToUidListMT (CImapSumList& SumList, CPtrUidList& UidList);
void GetFirstAndLastUidsMT (CPtrUidList& UidList, unsigned long& UidFirst, unsigned long& UidLast);
void RemoveUidFromUidListMT (CPtrUidList& UidList, unsigned long Uid);


static void CopyUidListMT (CPtrUidList& SrcUidList, CPtrUidList& DstUidList);
static void CommaListToUidListMT (LPCSTR pStr, CPtrUidList& DstUidList);
static void UidListToCommaStringMT (CPtrUidList& UidList, CString& szStr);
static int CountUidsInCommaStringMT (LPCSTR pUids);
static void ApplyConjunctionMT (int Conjunction, CPtrUidList& List1, CPtrUidList& List2, CPtrUidList& DstUidList);
static POSITION FindInSumlistMT (CImapSumList& SumList, unsigned long Uid);
static void SetLocalSummariesFlagMT (CImapSumList *pSumList, LPCSTR pUids, unsigned long ulBitFlags, BOOL bSet);
static unsigned long UIDSTRFindLargestMT (LPCSTR pUids);

// non-THREADSAFE!!
//
BOOL IsXferOnSameServer (CTocDoc *pSrcToc, CFilter *pFilt, CString& szDstImapName);
BOOL HasSkipRest (CFilter *pFilt);


//-------------------CPreFilterActions --------------------------------------------------//

//======= METHODS THAT MUST BE EXECUTED IN THE MAIN THREAD ===============//
// 

// Note: This must be created in the main thread!!!
//
CPreFilterActions::CPreFilterActions()
{
	m_bStopRequested = FALSE;

	// Set to TRUE if we performed at least 1 "MOVE" instead of just a "COPY".
	m_bPreFilterMoved = FALSE;

	// Clear all lists that we added.
	ClearAllImapLists ();

	m_HighestUidXferred = 0;

	// INI options.
	m_bOpenInMailbox	= FALSE;
	m_bDoFilterReport	= FALSE;

	m_bOpenInMailbox	= GetIniShort(IDS_INI_OPEN_IN_MAILBOX);
	m_bDoFilterReport	= GetIniShort(IDS_INI_FILTER_REPORT);

	// New ones: (5/29/98)
	//
	m_AccountID			= 0;

	// Make sure. If this becomes non-NULL, we have to release it before we
	// destroy the instance.
	//
	m_pImap				= NULL;

	m_pTaskInfo			= NULL;

}



CPreFilterActions::~CPreFilterActions()
{
	// Note:
	// This MUST be run in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// Release our hold on the IMAP agent.
	//
	if (m_pImap)
	{
		GetImapConnectionMgr()->Release (m_pImap);
	}
}




// ImapStartFiltering()
//
// FUNCTION
// Starts the filtering process
// END FUNCION

BOOL CPreFilterActions::StartFiltering(CTocDoc *pSrcToc)
{
	BOOL bResult = TRUE;

	// Note:
	// This MUST be run in the main thread.
	//
	ASSERT ( IsMainThreadMT() );


#ifdef EXPIRING
	if ( g_TimeStamp.IsExpired1() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);

		return FALSE;
	}
#endif 

	// Make sure filters are loaded up.
	//
	if (!GetFiltersDoc())
	{
		ASSERT (0);

		return (FALSE);
	}

	// Sanity:
	//
	if (!pSrcToc)
	{
		return FALSE;
	}

	// Get the CImapMailbox pointer from the TOC.
	//
	CImapMailbox *pImapMailbox = pSrcToc->m_pImapMailbox;

	if ( !pImapMailbox )
	{
		ASSERT (0);
		return FALSE;
	}

	// Our account ID.
	//
	m_AccountID = pImapMailbox->GetAccountID ();

	//
	// Get our personality name. No need to be thread safe, since this is in the main thread.
	//
	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( m_AccountID );
	if (!pAccount)
	{
		ASSERT (0);
		return FALSE;
	}

	// Set stuff that would be needed during background operations.
	//
	pAccount->GetName ( m_szPersona );
	pAccount->GetAddress (m_szServerName);

	m_szSrcImapName	= pImapMailbox->GetImapName();

	// This is the visible mailbox's name for use in background progress bars.
	//
	SetMBFilename (pSrcToc->MBFilename ());

	// We MUST get a CIMAP agent before we can continue!!
	//
	// Obtain a connection to the IMAP server from the connections manager.
	// This MUST be done in the main thread. We don't open the
	// connection here but the CIMAP must be setup in the main thread.
	//
	if (!m_pImap)
		bResult = SUCCEEDED ( AcquireNetworkConnection () );

	// Did we get it??
	//
	if (m_pImap)
	{
		// Make our copy of filters that will be used during a pre-filter,
		// i.e., those that have a transfer or copy on the same server.
		//
		CreatePreFilterList(pSrcToc);
	}
	
	return (TRUE);
}



// CreatePreFilterList [PUBLIC]
//
// Loop through the global filter actions and copy the ones that contain 
// XFER or COPY actions to mailboxes on the same server as the source mailbox.
// This result in populating "m_PreFilterList", which is a list of our own
// "CPreFilter objects.
//
// NOTE: This MUST be done n the main thread.
//
void CPreFilterActions::CreatePreFilterList(CTocDoc* pSrcToc)
{
	ASSERT ( IsMainThreadMT() );

	CFilterList *pFiltList	= NULL;

	// Must have a pSrcToc.
	//
	if (!pSrcToc)
	{
		ASSERT (0);
		return;
	}

	// Clear our list of PreFilters.
	//
	m_PreFilterList.NukeAllData();

	// Note: Only handle main filters here.
	pFiltList = &(GetFiltersDoc()->m_Filters);

	POSITION pos = pFiltList->GetHeadPosition();
	POSITION next;

	for (next = pos; pos; pos = next)
	{
		CFilter* filt = pFiltList->GetNext(next);

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
		// Is this a filter that applies to us? Perhaps.
		// Pass the filter to the CPreFilterList class to determine that.
		// If it agrees, then it would make an equivalent "CPreFilter" object 
		// and add it to it's list.

		m_PreFilterList.AddFromCFilter (filt, pSrcToc);

	}	// for.

	return;
}


// DoXferPreFilteringMT [PUBLIC] - MAP version.
//
// This is just a wrapper around the UidList version. Someday I'd
// convert to natively use MAP.
//
int CPreFilterActions::DoXferPreFilteringMT (CUidMap& NewUidMap)
{
	int iResult = 0;

	if (NewUidMap.GetCount() == 0)
		return 0;

	// Convert from the map to a CPtrUidList;
	//
	CPtrUidList NewUidList;

	UidIteratorType ci, Nextci;
	
	for ( ci = NewUidMap.begin(); ci != NewUidMap.end(); ci = Nextci)
	{
		Nextci = ci; Nextci++;

		CImapFlags* pF = ( CImapFlags * ) (*ci).second;
		if (pF)
		{
			NewUidList.OrderedInsert (pF->m_Uid, pF->m_Imflags, pF->m_IsNew);
		}

		NewUidMap.erase (ci);

		if (pF)
			delete pF;
	}

	iResult = DoXferPreFilteringMT (NewUidList);

	// Copy back the results!!
	//
	POSITION pos = NewUidList.GetHeadPosition();
	POSITION next;

	for (next = pos; pos; pos = next)
	{
		CImapFlags* pF = ( CImapFlags * ) NewUidList.GetNext (next);
		if (pF)
		{
			NewUidMap.OrderedInsert (pF->m_Uid, pF->m_Imflags, pF->m_IsNew);
		}

		NewUidList.RemoveAt (pos);

		if (pF)
			delete pF;
	}
		
	return iResult;
}





// DoXferPreFilteringMT [PUBLIC]
//
// THREADSAFE prefiltering.
//
// Does pre-filtering on ALL messages in NewUidList. Filter actions
// Are the ones in our own "m_PreFilterList". Tjis list contains
// filters that were copied from the global "g_Filter" object, and which
// contains only XFER's or COPY's or SKIP REST's.
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
int CPreFilterActions::DoXferPreFilteringMT (CPtrUidList& NewUidList)
{
	// If no UID's, nothing to do.
	if ( NewUidList.GetCount () <= 0 )
		return 0;

	// If we don't have any filters...
	//
	if (m_PreFilterList.IsEmpty())
	{
		return 0;
	}

	// Get the Uid list into a comma-separated string.
	CString szUidStr;

	UidListToCommaStringMT (NewUidList, szUidStr);

	// Must have som uid's
	if ( szUidStr.IsEmpty () )
	{
		return 0;
	}

	// Inform user??
	//
	if (m_pTaskInfo)
	{
		m_pTaskInfo->SetTitle (CRString(IDS_IMAP_FILTERING_MSGS));
	}

	// Create our "skip rest" uid list to accumulate uid's that need to be skipped
	// in succeeding filters.
	//
	CPtrUidList SkipRestUidList;

	POSITION pos = m_PreFilterList.GetHeadPosition();
	POSITION next;

	for (next = pos; pos; pos = next)
	{
		CPreFilter* filt = m_PreFilterList.GetNext(next);

		// Check for cancel:
		//
		if ( m_bStopRequested )
		{
			break;
		}

		 if (!filt)
			continue;

		//
		// Is this a filter that applies to us? 
		// If it has a copy or xfer action to a mailbox on the same server,
		// then we want to  perform the action here.
		// Note: "IsXferFilter()" will return FALSE if it meets a "Skip Rest"
		// ahead of a transfer-to, but TRUE otherwise. 
		//
		BOOL bIsXferFilter = filt->ContainsCopyOrXfer ();

		// Do the test for whether this contains a skip-rest here as well, because we
		// may need to to the ImapMatch in either case.
		//
		BOOL bHasSkipRest = filt->HasSkipRest ();

		if (bIsXferFilter || bHasSkipRest)
		{
			// Do the server search and get a UID subset of NewUidList that match
			//  this filter criteria.
			// NOTE: "ImapMatch" does the SEARCH on the server and gets the list of UID's
			// that matcg the filter criteria.
			//

			CPtrUidList SubUidList;

			ImapMatchMT (filt, NewUidList, SubUidList);

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

					ApplyConjunctionMT (IDS_FIO_OR, SubUidList, DummyUidList, TmpSkipRestUidList);

					DummyUidList.DeleteAll ();
				}

				// Handle transfers first:
				//
				if ( bIsXferFilter )
				{
					// If any of the matches are in our SkipRestUidList, remove them:
					//
					CPtrUidList DestUidList;

					ApplyConjunctionMT (IDS_FIO_UNLESS, SubUidList, SkipRestUidList, DestUidList);

					// Do the remote xfers on what we have left:
					//
					CString szStrMatches;

					UidListToCommaStringMT (DestUidList, szStrMatches);

					if (! szStrMatches.IsEmpty() )
					{
						//
						// Did we find matches? If we did, make sure the UID is in NewUidList,
						// perform the filtering, and delete successful transfers from NewUidList.

						DoRemoteTransfersMT (filt, NewUidList, szStrMatches);
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

					ApplyConjunctionMT (IDS_FIO_OR, SkipRestUidList, TmpSkipRestUidList, DestUidList);

					// We have to clean SkipRestUidList and copy from DestUidList back to
					// SkipRestUidList.
					//
					SkipRestUidList.DeleteAll();

					// The following will do it:
					//
					TmpSkipRestUidList.DeleteAll ();

					ApplyConjunctionMT (IDS_FIO_OR, DestUidList, TmpSkipRestUidList, SkipRestUidList);

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



// ImapMatchMT [PRIVATE]
//
// Ask the IMAP server for a subset of "TotalUidList" that matches the criteria in "filt"
// Return the list in "SubUidList".
// NOTE:
// This works regardless of the filter action, i.e., it doesn't only work for Xfer/Copy!
//
void CPreFilterActions::ImapMatchMT (CPreFilter *filt, CPtrUidList& TotalUidList, CPtrUidList& SubUidList)
{
	int		HeaderNum;
	BOOL	bPersonalityMatched = FALSE;
	CPtrUidList List1, List2;
	BOOL	bWasInProgress = FALSE;

	// Must have these:
	if ( !(filt) )
		return;

	// Must have uid's.
	if (TotalUidList.GetCount() == 0)
		return;

	// Must have a valid IMAP Agent!!!
	//
	if (!m_pImap)
	{
		return;
	}

	// Formulate TotalUidList into a comma-string for UIDFind.
	CString szUidStr;

	UidListToCommaStringMT (TotalUidList, szUidStr);

	// Must have some uid's
	if ( szUidStr.IsEmpty () )
	{
		return;
	}

	// Tell user what we're doing.
	//
	CString buf;
	buf.Format (CRString(IDS_IMAP_SEARCHING_SERVER), filt->m_Name);

	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		MainProgress(buf);
	}
	else if (m_pTaskInfo)
	{
		m_pTaskInfo->SetMainText(buf);
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
		if (IsOurPersonalityMT (filt, HeaderNum))
		{
			CopyUidListMT (TotalUidList, List1);

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

		FormulateImapSearchCriteriaMT (filt->m_Header[HeaderNum], Verb,
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

			m_pImap->UIDFind ( HeaderList, bBody, bNot, szText, szUidStr, szResults);

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
				CommaListToUidListMT (szResults, List1);
			}
		}
	}	// Else.

	// Can we ignore the second criterion??
	//
	if (CONJUNCTION_FIRST + filt->m_Conjunction == IDS_FIO_IGNORE)
	{
		CopyUidListMT (List1, SubUidList);

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
		if (IsOurPersonalityMT (filt, HeaderNum))
		{
			// If first one is personality, ignore this.
			if (!bPersonalityMatched)
			{
				CopyUidListMT (TotalUidList, List2);

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

		FormulateImapSearchCriteriaMT (filt->m_Header[HeaderNum], Verb,
						 filt->m_Value[HeaderNum], HeaderList, &bBody, &bNot);

		//
		// Ask the IMAP server to get messages within our range that match our criterion.
		// NOTE here: Because of IMAP's limitations, we can only do the following 
		// verbs: "Contains", "Doesn't contain".
		//
		CString szResults;
		CString szText = filt->m_Value [HeaderNum];

		m_pImap->UIDFind ( HeaderList, bBody, bNot, szText, szUidStr, szResults);

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
			CommaListToUidListMT (szResults, List2);
		}
	}

	// Now go merge List1 and List2 using whatever conjunction we have.
	// Return the result in SubUidList.
	//

	// Get the conjunction.
	int Conjunction = CONJUNCTION_FIRST + filt->m_Conjunction;

	ApplyConjunctionMT (Conjunction, List1, List2, SubUidList);

	// If we put up progress, take it down.
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress();
	}

}





/////////////////////////////////////////////////////////////////////
// IsOurPersonality [PRIVATE]
// Return TRUE if the value in m_Value[HeaderNum] matches (exactly, contains, etc),
// the name of our personality.
//
BOOL CPreFilterActions::IsOurPersonalityMT (CPreFilter *pFilt, int HeaderNum)
{
	// Sanity;
	if (! (pFilt) )
		return FALSE;

	// HeaderNum MUST be 0 or 1.
	if (HeaderNum < 0 || HeaderNum > 1)
		return FALSE;

	// Matches are case insensitive. Note that personality match supports 
	// all the possible verbs.
	//
	CString szOurPersona = m_szPersona;

	szOurPersona.MakeLower();

	return pFilt->MatchValueMT (HeaderNum, szOurPersona);
}





/////////////////////////////////////////////////////////////////////////////
// FormulateImapSearchCriteria [PRIVATE]
// 
// Formulate the given text strings into a comma-separated list of headers and return
// it in "HeaderList". If "pHeader" is <<Body>>, set the bBody flag. If the
// verb is "NOT CONTAINS", set the bNot flag.
//
void CPreFilterActions::FormulateImapSearchCriteriaMT (LPCSTR pHeader, int Verb, 
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




////////////////////////////////////////////////////////////////////////////////////
// DoRemoteTransfersMT [PRIVATE]
//
// FUNCTION
// Do the actual message transfer to the remote destination mailbox
// based on the given filter (pFilt).
// "pUids" is a comma-separated list of UID's that match an IMAP SEARCH request.
// If the copy/transfer succeeds, remove the uid's from NewUidList.
// END FUNCTION

// NOTE:
// pSumList may be NULL, but if it isn't, whenever we do an XFER, look for the 
// matching summaries and flag the messages as \deleted.
//
////////////////////////////////////////////////////////////////////////////////////
BOOL CPreFilterActions::DoRemoteTransfersMT (CPreFilter *pFilt, CPtrUidList& NewUidList, LPCSTR pUids, CImapSumList *pSumList /* = NULL */)
{
	BOOL	bResult = FALSE;
	BOOL	bUserAborted = FALSE;

	// Sanity:
	if ( ! (pFilt && pUids) )
	{
		ASSERT (0);
		return FALSE;
	}

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
		if ( m_bStopRequested )
		{
			bUserAborted = TRUE;
			break;
		}

		// for this action:
		//
		BOOL	Copy	= TRUE;

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
						Copy = FALSE;
					}
					else
					{
						mboxPath = pFilt->m_CopyTo[i];
						Copy = TRUE;
					}

					CString szDstImapName = pFilt->GetDstImapName (i);

					if ( szDstImapName.IsEmpty() )
					{
						break;
					}

					// Show some kind of progress.  
					CString szBuf;
					szBuf.Format ( CRString (IDS_IMAP_XFERING_TOREMOTE), 
									   Copy ? CRString (IDS_IMAP_COPYING) : CRString (IDS_IMAP_TRANSFERING),
									   szDstImapName, m_szServerName);

					if ( ::IsMainThreadMT() )
					{
						MainProgress( szBuf );
					}
					else if (m_pTaskInfo)
					{
						m_pTaskInfo->SetMainText(szBuf);
					}

					// Co the copy first.
					bResult = SUCCEEDED (m_pImap->CopyMessages (pUids, szDstImapName));

					if (bResult)
					{
						// We've copied a message to a destination mailbox.
						// Update notification lists.
						//

						int nMsgsCopied = CountUidsInCommaStringMT (pUids);

						UpdateAfterCopy (pFilt, mboxPath, nMsgsCopied);

						// Keep track of the highest UID we've xferred.
						//
						unsigned long ulMax = UIDSTRFindLargestMT (pUids);

						if (ulMax > m_HighestUidXferred)
							m_HighestUidXferred = ulMax;

						// XFER instead of COPY?
						//
						if ( !Copy)
						{
							// Just flag them for deletion.
							//
							bResult = SUCCEEDED ( m_pImap->UIDDeleteMessages (pUids, FALSE) );

							// Flag the summaries as \deleted if we transferred.
							// Note: If this was part of a pre-filter, "pSumList" would be NULL
							// so this wouldn't get executed (which is what we want).
							//
							if (bResult)
							{
								if (pSumList)
								{
									SetLocalSummariesFlagMT (pSumList, pUids, IMFLAGS_DELETED, TRUE);
								}

								// If "Copy" was FALSE, messages were flagged for deletion.
								// Signal that we should do an EXPUNGE..
								//
								m_bPreFilterMoved = TRUE;

								RemoveUidsFromListMT ( NewUidList, pUids);
							}
						}
					}
				}

				break;	// XFER of COPY.
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



// UpdateAfterCopy [PRIVATE]
//
// We've just copied a message to a destination mailbox as part of pre-filtering.
// Update our lists so we can perform necessary notifications later.
//
void CPreFilterActions::UpdateAfterCopy (CPreFilter* pFilt, LPCSTR pDstMboxPath, UINT nMsgsCopied)
{
	// Sanity:
	//
	if (! (pFilt && pDstMboxPath) )
		return;

	// This keeps mboxPath names of destination mailboxes.
	// 

	m_FilterMboxMap[pDstMboxPath] = 1;

	// Determine if this should be in a filter report and add it.
	//
	if ( (pFilt->m_NotfyUserNR < 0 && m_bDoFilterReport)  || 
		 (pFilt->m_NotfyUserNR > 0 && (pFilt->m_NotfyUserNR & NO_REPORT )) )
	{
		StringMapIteratorType ci = m_FilterReportMap.find(pDstMboxPath);

		if ( ci != m_FilterReportMap.end() )
		{
			(*ci).second += nMsgsCopied;
		}
		else
		{
			// Add a new one.
			//
			m_FilterReportMap[pDstMboxPath] = nMsgsCopied;
		}
	}
}



/////////////////////////////////////////////////////////////////////////
// RemoveUidsFromList [PRIVATE]
//
// Remove the Uid's in the comma-separated list "pUids" from NewUidList.
//
/////////////////////////////////////////////////////////////////////////
BOOL CPreFilterActions::RemoveUidsFromListMT (CPtrUidList& NewUidList, LPCSTR pUids)
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




// ClearAllImapLists [PRIVATE]
//
// Clear all lists that we added.
//
void CPreFilterActions::ClearAllImapLists ()
{
	// List of remote mailboxes we transfered messages to.
	m_FilterMboxMap.DeleteAll ();
}





// CopyDestinationMboxList [PUBLIC]
//
// Copy information from m_FilterMboxMap and m_FilterReportMap into given parameters.
//
// For m_FilterMboxMap, allocate one CString object for each entry and add to strDstList.
// For m_FilterReportMap, allocate a CString object for each message.
//

void CPreFilterActions::GetResultsLists (CObList& strDstList, CObList& strReportList)
{
	ASSERT ( IsMainThreadMT() );

	StringMapIteratorType ci;

	// m_FilterMboxMap:
	//
	for( ci = m_FilterMboxMap.begin(); ci != m_FilterMboxMap.end(); ci++  )
	{
		LPCSTR pDstMboxPath = (LPCSTR) (*ci).first;

		if (pDstMboxPath)
		{
			CString* newStr = new CString (pDstMboxPath);

			if (newStr)
				strDstList.AddTail ((CObject *)newStr);
		}
	}

	// m_FilterReportMap:
	//
	for( ci = m_FilterReportMap.begin(); ci != m_FilterReportMap.end(); ci++  )
	{
		LPCSTR pDstMboxPath = (LPCSTR) (*ci).first;
		unsigned long nMsgs = (unsigned long) (*ci).second;

		if (pDstMboxPath && (nMsgs > 0) )
		{
			for (unsigned long i = 0; i < nMsgs; i++)
			{
				CString* newStr = new CString (pDstMboxPath);

				if (newStr)
					strReportList.AddTail ((CObject *)newStr);
			}
		}
	}

}

		




// EndFiltering
//
// Finishes up the pre filtering after all messages have been processed.
// NOTE: This MUST be done in the main thread.
//
BOOL CPreFilterActions::ImapEndFiltering()
{

	ASSERT ( IsMainThreadMT() );

	// Nothing really to do.

	return (TRUE);
}




#if 0 // NOT YET



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


#endif // JOK - Slowly modify.



// AcquireNetworkConnection [PRIVATE]
//
// Note: This sets the m_pImap object.
//
HRESULT CPreFilterActions::AcquireNetworkConnection ()
{
	CIMAP *pImap = NULL;

	// THis might get an existing one.
	//
	HRESULT hResult = GetImapConnectionMgr()->CreateConnection (m_AccountID, m_szSrcImapName, &pImap, TRUE);

	if ( pImap && SUCCEEDED (hResult) )
	{
		// Set he internal pointer.
		//
		m_pImap = pImap;
	}

	return hResult;
}





//=================== Utility functions =======================//


// =========  THREAD    SAFE    FUNCTION ==================/

// These are thread safe as long as the objects they operate on exist only in the
// current thread.
// 

// Get the uid's from a CImapSUmList list into a CPtrUidList.
//
// static 
void SumListToUidListMT (CImapSumList& SumList, CPtrUidList& UidList)
{
	POSITION pos, next;

	//
	// Clear this first, including the objects themselves.
	//
	UidList.DeleteAll ();

	pos = SumList.GetHeadPosition();

	for (next = pos; pos; pos = next)
	{
		CImapSum* pImapSum = (CImapSum *)SumList.GetNext(next);

		if (pImapSum)
		{
			unsigned long Uid = pImapSum->GetHash();
			if (Uid)
			{
				UidList.OrderedInsert (Uid, pImapSum->m_Imflags, FALSE);
			}
		}
	}
}



//
// Get the first and last UID's from the ordered UID list "UidList"
//
// static 
void GetFirstAndLastUidsMT (CPtrUidList& UidList, unsigned long& UidFirst, unsigned long& UidLast)
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
// static 
void CopyUidListMT (CPtrUidList& SrcUidList, CPtrUidList& DstUidList)
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
// static 
void CommaListToUidListMT (LPCSTR pStr, CPtrUidList& DstUidList)
{
	const char *p, *q;
	char Comma = ',';
	char buf [12];
	int len;
	long Uid;

	// Free this.
	DstUidList.DeleteAll ();

	p = pStr;

	while (p && *p)
	{
		*buf = 0;

		q = strchr (p, Comma);

		if (q)
		{
			len = q - p;
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
// static 
void UidListToCommaStringMT (CPtrUidList& UidList, CString& szStr)
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
// static 
int CountUidsInCommaStringMT (LPCSTR pUids)
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
// static 
void ApplyConjunctionMT (int Conjunction, CPtrUidList& List1, CPtrUidList& List2, CPtrUidList& DstUidList)
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
POSITION FindInSumlistMT (CImapSumList& SumList, unsigned long Uid)
{
	POSITION pos, next;

	pos = SumList.GetHeadPosition ();

	for (next = pos; pos; pos = next)
	{
		CImapSum *pImapSum = (CImapSum *)SumList.GetNext (next);

		unsigned long ThisUid = pImapSum->GetHash ();

		if (ThisUid == Uid)
			return pos;
	}

	return NULL;
}




// Remove this uid from UidList;
// static
void RemoveUidFromUidListMT (CPtrUidList& UidList, unsigned long Uid)
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

		



// SetLocalSummariesFlag [INTERNAL]
//
// Loop through pSumList looking for each pUid. If found, set the
// local summary flag depending on what's contained in "ulBitFlag".
// NOTE: ulBitFlag is an OR'd combination of the IMFLAGS_* bitflags.
// NOTE: If "bSet" is FALSE, remove the flag instead of setting it.
//
// static
void SetLocalSummariesFlagMT (CImapSumList *pSumList, LPCSTR pUids, unsigned long ulBitFlags, BOOL bSet)
{
	char buf [24];
	char Comma = ',';

	// Sanity:
	if (! (pSumList && pUids) )
		return;

	const char *p = pUids;
	const char *q;
	size_t len;
	CImapSum *pImapSum;
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
				pos = FindInSumlistMT (*pSumList, Uid);
				if (pos)
				{
					pImapSum = (CImapSum *)pSumList->GetAt (pos);
					if (pImapSum)
					{
						// Set the CSummary IMFLAGS_DELETED flag:
						if (ulBitFlags & IMFLAGS_DELETED)
						{
							if (bSet)
								pImapSum->m_Imflags |= IMFLAGS_DELETED;
							else
								pImapSum->m_Imflags &= !IMFLAGS_DELETED;

						}

						// READ/UNREAD: Must also set the POP flag.
						if (ulBitFlags & IMFLAGS_SEEN)
						{
							if (bSet)
							{
								pImapSum->m_Imflags |= IMFLAGS_SEEN;
								pImapSum->SetState (MS_READ);
							}
							else
							{
								pImapSum->m_Imflags &= !IMFLAGS_SEEN;
								pImapSum->SetState (MS_UNREAD);
							}
						}

						// ANSWERED: (i.e. replied) Must also set the POP flag.
						if (ulBitFlags & IMFLAGS_ANSWERED)
						{
							if (bSet)
							{
								pImapSum->m_Imflags |= IMFLAGS_ANSWERED;
								pImapSum->SetState (MS_REPLIED);
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
static unsigned long UIDSTRFindLargestMT (LPCSTR pUids)
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




//================!! NON   THREAD   SAFE   FUNCTIONS !!====================//



// IsXferOnSameServer [status]
//
// Return TRUE if this filter has a COPY or TRANSFER to a mailbox on the same server as pTocDoc.
//
// NOTE: This MUST be executed in the main thread.
//
// HISTORY:
// If destination mailbox is on the same server, return its full IMAP name
// in "szDstImapName".
//
BOOL IsXferOnSameServer (CTocDoc *pSrcToc, CFilter *pFilt, CString& szDstImapName)
{
	BOOL bResult = FALSE;

	// This MUST be performed in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// Sanity
	if (! (pSrcToc && pFilt) )
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
					if (! IsOnSameServer (pSrcToc, (QCImapMailboxCommand *) pCommand) )
					{
						break;
					}

					// Seems like we found one.
					// Copy destination name to the out paramater.
					//
					szDstImapName = ((QCImapMailboxCommand *)pCommand)->GetImapName ();

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




//




// HasSkipRest 
// Because it operates on a global CFilter object, it is NOT thread safe!!
//
// static
BOOL HasSkipRest (CFilter *pFilt)
{
	ASSERT ( IsMainThreadMT() );

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




#endif // IMAP4


