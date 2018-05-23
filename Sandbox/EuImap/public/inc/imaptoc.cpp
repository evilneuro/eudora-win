// imaptoc.cpp - IMAP-specific methods added to TocDoc:

// NOTE: All this is IMAP specific.

// Local MACROS.

#ifdef IMAP4 // Only for IMAP.

#include "ImapTypes.h"

void CommaUidListToSumList (LPCSTR pUids, CTocDoc* pTocDoc, CSumList& SumList, unsigned long* pnSumsNotFound);
BOOL RewriteMbxStub (CSummary *pSum, CTocDoc *pSourceToc, CTocDoc *pTargetToc, unsigned long NewUid,
					long& MStart, long& NewLength);
static long WriteMessageSeparator (JJFile *pMbxFile);
void  ClearPreviewCache (CTocDoc *pTocDoc);
void DisplayNextMessageOrClose (CTocDoc *pTocDoc, CSummary* SingleSum, POSITION LastSelectedPos);
BOOL ImapSetRemoteStateOfUidlist (CImapMailbox *pImapMailbox, CString& szUidList, char State, BOOL bUnset);
BOOL RemoveAttachments (CTocDoc *pTocDoc, CSummary *pSum);
BOOL MoveContents ( CTocDoc* pSrcToc, CTocDoc* pTargetToc, CSummary *pSum);

//=============================================================////
//// Do this to implement enabling/disabling of the TOC window.//
//
class CDisableToc
{
public:
	CDisableToc (CTocDoc *pTocDoc);
	~CDisableToc ();

	CTocDoc *m_pTocDoc;
};

CDisableToc::CDisableToc (CTocDoc *pTocDoc)
{
	m_pTocDoc = pTocDoc;

	if (pTocDoc)
	{
		CTocView *pView = pTocDoc->GetView();
		if (pView)
			pView->EnableWindow (FALSE);
	}
}


CDisableToc::~CDisableToc()
{
	// Re-enable the view.
	if (m_pTocDoc)
	{
		CTocView *pView = m_pTocDoc->GetView();
		if (pView)
			pView->EnableWindow (TRUE);
	}
}


// ================ AdditionalIMAP methods =======================//


// ImapInitialize [PRIVATE]
// FUNCTION
// Instantiate the contained CImapMailbox object if it doesn't exist.
// END FUNCTION.
BOOL CTocDoc::ImapInitialize (unsigned long AccountID)
{
	BOOL	bResult = FALSE;

	// If this is a temporary mailbox, ignore.
	if ( ImapIsTmpMbx (MBFilename()) )
		return TRUE;

	// Make sure this is an IMAP toc.
	if (AccountID == 0)
		return FALSE;

	// Set type 
	m_Type = MBT_IMAP_MAILBOX;
	m_bAutoDelete = FALSE;

	// Create the contained IMAP mailbox object.
	if (m_pImapMailbox) // Shouldn't be.
		delete m_pImapMailbox;

	m_pImapMailbox = new CImapMailbox ((ACCOUNT_ID)AccountID);
	if (m_pImapMailbox)
	{
		CString dir;

		MbxFilePathToMailboxDir (MBFilename(), dir);
		m_pImapMailbox->SetDirname (dir);

		// Make sure the MBX file exists. Note: CreateLocalFile(Path, FALSE) returns
		// TRUE if path exists. It doesn't delete it's contents.
		bResult	= CreateLocalFile (MBFilename(), FALSE);

		// Read info from the IMAP mailbox's info file.
		if (bResult)
			bResult = m_pImapMailbox->ReadImapInfo ();

		// If we couldn't read, fail.
		if (!bResult)
		{
			delete m_pImapMailbox;
			m_pImapMailbox = NULL;
		}
	}

	if (m_pImapMailbox)
		m_pImapMailbox->SetParentToc(this);

	return bResult;
}




// ImapXfer [PUBLIC]
// FUNCTION
// Handle transfers where one of the mailboxes is an IMAP mailbox.
// END FUNCTION

// NOTES:
// TargetToc can be NULL!! In this case the source toc MUST be an IMAP toc and
// we interpret the action to be to flag the remote messages as deleted, possibly
// followed by an EXPUNGE (if Copy is false).
// END NOTES

CSummary* CTocDoc::ImapXfer(CTocDoc* TargetToc, CSummary* SingleSum /*= NULL*/,
	BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/, BOOL RedisplayTOCs /*= TRUE*/)
{
	static BOOL bXferActive = FALSE;			// reentrancy semaphore
	CSummary	*CopySum = NULL;
	int			HighlightIndex = -1;
	CTocView	*View = NULL, *TargetView = NULL;

	if (bXferActive)
	{
		ASSERT(0);								// whoops, somehow got here reentrantly
		return NULL;
	}
	else
		bXferActive = TRUE;						// don't leave unless you reset this flag!

	// Don't make any preview get done until we are completed.
//	ClearPreviewCache (this);

//	if (TargetToc)
//		ClearPreviewCache (TargetToc);

	View = GetView();
	if (TargetToc)
		TargetView = TargetToc->GetView();
	else
		TargetView = NULL;

	// Also, if we're in the main thread, set a wait cursor.
	CCursor waitCursor;

	// We'd better not be trying to transfer selected messages when there's no view
	ASSERT(SingleSum || View);

	// Cleanup undo.		
	if (!Copy)
	{
		ClearUndoFrom();
		if (TargetToc)
		{
			TargetToc->ClearUndoTo();

#if 0 // Undo doesn't work - don't allow
			m_UndoFromToc = TargetToc;
			TargetToc->m_UndoToToc = this;
#endif

		}
	}

	// Must also handle transfer to the out mailbox.
	BOOL FromOut = (m_Type == MBT_OUT);


	// If SingleSum, find the summary after it for display below.
	// Note: We use "NextPos" below.
	POSITION	pos, NextPos;
	pos = m_Sums.GetHeadPosition();
	NextPos = NULL;

	for (NextPos = pos; pos; pos = NextPos)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
		else
			break;
	}


	// Call ImapXferMessages to do the real work. 
	// Note: imapXferMessgages modifies HighlightIndex.
	HighlightIndex = -1;
	CopySum = ImapXferMessages (TargetToc, SingleSum, HighlightIndex, CloseWindow, Copy);

	// If user wants to expunge from the source toc, go do so now.
	//
	if (!Copy && IsImapToc())
	{
		if ( ExpungeAfterDelete (this) )
		{
			ImapExpunge();

			// CopySum may no longer be valid.
			// 
			CopySum = NULL;
		}
	}

	// Redisplay. Note: Our original View may have been deleted. Get the 
	// new one here!!!
	//

	View = GetView();

	if (View)
	{

#if 0 // Don't do this here anymore!! (JOK)

		if (HighlightIndex >= 0)
		{
			if (HighlightIndex >= NumSums())
				HighlightIndex = NumSums() - 1;
			View->m_SumListBox.SetSel(HighlightIndex);
		}
#endif // JOK

		// Set redraw back to TRUE
//		if (RedisplayTOCs && View && !Copy)
		{
			View->m_SumListBox.SetRedraw(TRUE);
//			View->Invalidate();
		}
	}

	// Make sure redraw is reset.
	if (RedisplayTOCs && TargetView)
	{
		TargetView->m_SumListBox.SetRedraw(TRUE);
		TargetView->m_SumListBox.UpdateWindow();
	}

	if (!Copy)
	{
		Write();
	}

	if (RedisplayTOCs && View)
	{
		View->m_SumListBox.SetRedraw(TRUE);
//		View->m_SumListBox.UpdateWindow();

		// Tends to lose focus!
		View->SetFocus ();
	}

	ASSERT(bXferActive);
	bXferActive = FALSE;

	return CopySum;
}





// 
// ImapXferMessages [PRIVATE]
// FUNCTION
// Internal method to move/copy messages from or to an IMAP mailbox.
// Return CopySum. Also, set "HighlightIndex".
// END FUNCTION

// NOTES
// This function does the work of copy/move, including removal of TOCS.
// Return  
// END NOTES

// HISTORY
// Created June 11, 1997 by J. King.
// END HISTORY

// NOTE (8/5/97)
// pTargetToc can be NULL. Also SingleSum can be NULL.
// END NOTE

CSummary* CTocDoc::ImapXferMessages (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/)
{
	BOOL			FromImap, ToImap;
	CSummary		*CopySum = NULL;

	// Initialize in case we fail.
	HighlightIndex = -1;

	// Make sure one of us is IMAP.
	FromImap = IsImapToc();

	// If pTargetToc is NULL, flags ToImap as TRUE.
	ToImap   = (pTargetToc == NULL) || (pTargetToc && pTargetToc->IsImapToc());

	if (!(FromImap || ToImap))
	{
		ASSERT (0);
		return NULL;
	}

	// Make sure the CImapMailbox object is instantiated in any IMAP tocs.
	if (FromImap)
	{
		if (!m_pImapMailbox)
			return NULL;
	}

	if (ToImap && pTargetToc)
	{
		if (! pTargetToc->m_pImapMailbox)
			return NULL;
	}

	// Disable events while we're doing this.
	CDisableToc disAble (this);

	// If From Imap, all server messages go through our m_pImapMailbox. 
	// Open connection to IMAP if it is not open.
	BOOL bConnected					= FALSE;
	BOOL bMustCloseConnection		= FALSE;
	BOOL bMustCloseTargetConnection = FALSE;

	// If we're offline, ask what to do:
	//
	BOOL bIsOffline = GetIniShort(IDS_INI_CONNECT_OFFLINE);

	if ( bIsOffline &&
		 ( (m_pImapMailbox && !m_pImapMailbox->IsSelected() ) || 
		   ( pTargetToc && !pTargetToc->m_pImapMailbox->IsSelected() ) ) )
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
		{
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);

			// And fall through.
		}
		else if (res == IDCANCEL)
		{
			return NULL;

			// And fall through.
		}
	}

	if (FromImap)
	{
		if (!m_pImapMailbox->IsSelected())
		{
			bConnected = SUCCEEDED ( m_pImapMailbox->OpenMailbox() );

			if (!bConnected)
				return NULL;
		}
	}
	else if (ToImap && pTargetToc)
	{
		if (!pTargetToc->m_pImapMailbox->IsSelected())
		{
			bConnected = SUCCEEDED ( pTargetToc->m_pImapMailbox->OpenMailbox() );

			if (!bConnected)
				return NULL;

			// If the toc has a view, keep the IMAP connection open.
			CTocView*	TargetView = pTargetToc->GetView ();
			if (!TargetView)
				bMustCloseTargetConnection = TRUE;
		}
	}


	// If both mailboxes are IMAP, and they are in the same account, we need to
	// accumulate lists and do batch commands. Handle each case differently.

	if (FromImap && ToImap)
	{
		// If no target toc, add to list. Will be handled when the queue is handled.
		if ( (pTargetToc == NULL) ||
			 ( pTargetToc && (m_pImapMailbox->GetAccountID() == pTargetToc->m_pImapMailbox->GetAccountID()) ) )
		{
			CopySum = ImapXferOnSameAccount (pTargetToc, SingleSum, HighlightIndex, CloseWindow, Copy);
		}
		else
		{
			CopySum = ImapXferBetweenAccounts (pTargetToc, SingleSum, HighlightIndex, CloseWindow, Copy);
		}
	}
	else
	{
		if (FromImap && pTargetToc)
		{
			CopySum = ImapXferFromImapToLocal (pTargetToc, SingleSum, HighlightIndex, CloseWindow, Copy);
		}
		else if (ToImap && pTargetToc)
		{
			CopySum = ImapXferFromLocalToImap (pTargetToc, SingleSum, HighlightIndex, CloseWindow, Copy);
		}
		else
		{
			// Should no get here..
			ASSERT (0);
		}
	}

	// If we're offline, close all connections.
	//
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE ) )
	{
		bMustCloseConnection = bMustCloseTargetConnection = TRUE;
	}

	if (bMustCloseConnection)
	{
		if (FromImap && m_pImapMailbox)
			m_pImapMailbox->Close();
	}

	if (bMustCloseTargetConnection)
	{
		if (ToImap && pTargetToc && pTargetToc->m_pImapMailbox)
			pTargetToc->m_pImapMailbox->Close();
	}

	return CopySum;
}



// FUNCTION
// Transfer messages between mailboxes on the same account.
// Return HighlightIndex or -1 if error.
// END FUNCTION

// NOTES
// NOTE: pTargetToc can be NULL, in which case, the messages are flagged as (\Deleted).
// NOTE: Now changed (10/21/97) so MUST have a target toc. This now just does copy/move,
// no deletes!!
// If "Copy" is FALSE, the source messages are flagged for deletion. 
// END NOTES

CSummary* CTocDoc::ImapXferOnSameAccount (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/)
{
	POSITION	pos = NULL, NextPos = NULL;
	CString		sUidlist, sUid;
	CSumList	ImapMoveSumList;
	BOOL		bResult = TRUE;
	BOOL		FoundSelected	 = FALSE;
	int			i = 0;
	CTocView*	View = NULL;
	int			nCount = 0;
	BOOL		bWasInProgress = FALSE;
	POSITION	LastSelectedPos = NULL;
	BOOL		bMessagesCopied = FALSE;
	BOOL		bUseFancyTrash = FALSE;


	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return NULL;
	}

	// Must also have a target toc!! (JOK, 10/21/97).
	if (! (pTargetToc && pTargetToc->m_pImapMailbox) )
	{
		ASSERT (0);
		return NULL;
	}

	// We can't copy/move to the same mailbox!
	if ( this == pTargetToc )
	{
		ASSERT (0);
		return FALSE;
	}

	// If target mailbox is read-only, issue warning..
	if (pTargetToc->m_pImapMailbox->IsReadOnly())
	{
		CString buf;

		ErrorDialog( IDS_WARN_IMAP_RDONLY_MOVETO, pTargetToc->Name());

		return FALSE;
	}

	// We determine if to continue based on the value of this.
	bResult = TRUE;

	// If this is a MOVE, check if source mailbox is read-only and issue warning.
	if (!Copy)
	{
		if (m_pImapMailbox->IsReadOnly())
		{
			CString buf;

			buf.Format (CRString (IDS_WARN_IMAP_RDONLY_MOVEFROM), Name());
			MessageBeep(MB_ICONEXCLAMATION);
			bResult = ( IDYES == ::AfxMessageBox(buf, MB_ICONQUESTION | MB_YESNO) );
			
			if (bResult)
				Copy = TRUE;
		}
	}

	if (!bResult)
	{
		goto end;
	}

	// We can put up a progress window here!
	if ( ::IsMainThreadMT() )
	{
		CString buf;

		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		buf.Format ( Copy ? CRString(IDS_IMAP_COPYING_MSGS) : CRString(IDS_IMAP_TRANSFERING_MSGS));
		MainProgress(buf);
	}

	// The destination (IMAP) mailbox may not be open so we may just be copying these
	// on the server. In that case, we need to manually "bold" the destination
	// mailbox in the mailbox tree, in which case, we have to determine here if we are copying message
	// that are considered "unread".
	//
	long FreshnessDate;
	BOOL bCopiedUnread;

	FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
	bCopiedUnread = FALSE;

	// Get the view so we can loop.
	View = GetView();

	// Whether this is a copy or move, we need to accumulate a list of summaries we'll
	// be operating on..

	FoundSelected	 = FALSE;

	pos = m_Sums.GetHeadPosition();
	HighlightIndex = -1;

	i = 0;
	nCount = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			// See if we found the first unselected message after a selected message
			// and note its index for later selection
			if (!Copy && FoundSelected && HighlightIndex < 0)
				HighlightIndex = i;

			continue;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!Copy)
		{
			CFrameWnd* Frame = Sum->m_FrameWnd;
			if (Frame)
			{
				if (!(Frame->GetActiveDocument()->CanCloseFrame(Frame))) continue;
			}
		}

		FoundSelected = TRUE;

		// Ok. This is selected. Add to list.
		if (sUidlist.IsEmpty())
			sUidlist.Format("%lu", Sum->m_Hash);
		else
		{
			sUid.Format (",%lu", Sum->m_Hash);
			sUidlist += sUid;
		}

		// Also, if this is a move, add this summary to a list for later deletion.
		// NOTE: Make sure this is in the same order as m_Sums!!!
		//
		if (!Copy)
			ImapMoveSumList.AddTail (Sum);

		// Check if this is an "unread" message:
		//
		if (Sum->m_State == MS_UNREAD && Sum->m_Seconds > FreshnessDate)
			bCopiedUnread = TRUE;

		// Found another one.
		nCount++;

		// 
		// If SingleSum, then only that summary we're processing.
		//
		if (SingleSum)
			break;
	}

	//
	// Mark "NextPos" for later use. Note: Could be NULL.
	//

 	LastSelectedPos = NextPos;

	// Must have found some.
	if (nCount <= 0)
	{
		ASSERT (0);
		goto end;
	}

	// Now transfer the messages on the server.
	bResult = SUCCEEDED ( m_pImapMailbox->XferMessagesOnServer ((LPCSTR)sUidlist,
								pTargetToc->m_pImapMailbox->GetImapName(), TRUE, TRUE ) );
	if (!bResult)
	{
		m_pImapMailbox->ShowLastImapError ();

		goto end;
	}

	//
	// Indicate that we copied messages.
	//
	bMessagesCopied = TRUE;

	//
	// Flag as deleted or use fancy trash.
	//
	if (!Copy)
	{
		bUseFancyTrash = m_pImapMailbox->UseFancyTrash();

		if ( bUseFancyTrash )
		{
			bResult = DoFancyExpunge ((LPCSTR) sUidlist);
		}
		else
		{
			// Flag on server now finally.
			//
			bResult = SUCCEEDED (m_pImapMailbox->DeleteMessagesFromServer ( (LPCSTR) sUidlist, FALSE, TRUE));

			if (bResult)
			{
				//
				// If this was a move from a TOC window and CloseWindow is TRUE, tell
				// ImapRemoveOrderedSubSums to close message windows as well.
				//
				// Note: If SingleSum, don't close the window. Allow "DisplayNextMessageOrClose"
				// to close it if necessary.
				//

				BOOL bAllowCloseWindow = CloseWindow && !SingleSum;

				bResult = ImapRemoveOrderedSubSums (&ImapMoveSumList, HighlightIndex,
					    bAllowCloseWindow, TRUE, TRUE);

				//
				// If this was a move from a readmsgdoc window,
				// we may need to open the next message.
				//
				if (!bAllowCloseWindow && bResult)
				{
					DisplayNextMessageOrClose (this, SingleSum, LastSelectedPos);
				}
			}
			else
			{
				// Tell the user that the copy succeeded but couldn't flag the messages as deleted.
				TCHAR buf [512];
				int iType = IMAPERR_BAD_CODE;
				buf[0] = 0;

				m_pImapMailbox->GetLastImapErrorAndType (buf, 510, &iType);
	
				ErrorDialog (IDS_ERR_IMAP_CANNOT_MODIFYFLAG, Name(), buf);

				// The focus tends to get lost here.
				// Set the focus back to the view for this mailbox!!!
				//

				if (View && !SingleSum)
				{
					View->SetFocus ();
				}
			}
		}
	}

	// Update target toc if it's opened in a view. and we copied messages.
	CTocView *TargetView;

	TargetView = pTargetToc->GetView();

	if (TargetView && bMessagesCopied)
	{
		// Do the fetch in the background.
		//
		pTargetToc->m_pImapMailbox->DoManualResync (pTargetToc, TRUE, FALSE, FALSE);
	}

	// If we copied unread messages to the destination mailbox, update the status.
	if (bCopiedUnread)
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( pTargetToc->MBFilename() );
		if (pCommand)
			pCommand->Execute( CA_UPDATE_STATUS, ( void* ) US_YES );
	}
		

end:

	// Remove progress bar.
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress();
	}

	// Redraw again.
	if (View && !bUseFancyTrash)
	{
		View->m_SumListBox.SetRedraw(TRUE);

		// Set the focus back. This will also trigger a new preview.
		View->SetFocus();
	}

	// Must clean this up.
	ImapMoveSumList.RemoveAll();

	return SingleSum;
}




// FUNCTION
// Transfer messages between mailboxes on different servers.
// Return HighlightIndex or -1 if error.
// END FUNCTION

CSummary* CTocDoc::ImapXferBetweenAccounts (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/)
{
	POSITION	pos = NULL, NextPos = NULL;
	CString		sUidlist, sUid;
	CSumList	ImapQueue;
	CSumList	ImapMoveSumList;
	BOOL		bResult = TRUE;
	int			i = 0;
	CSummary*	pCopySum = NULL;
	CSummary*	pLastCopySum = NULL;
	unsigned long nMsgs = 0;
	BOOL		ProgressIsUp = FALSE;
	BOOL		CountdownProgressIsUp = FALSE;
	int			count = 0;
	POSITION	LastSelectedPos = NULL;
	BOOL		bUseFancyTrash = FALSE;

	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return NULL;
	}

	// Must also have a target IMAP mailbox.
	if (! (pTargetToc && pTargetToc->m_pImapMailbox) )
	{
		ASSERT (0);
		return NULL;
	}

	// If target mailbox is read-only, issue warning..
	if (pTargetToc->m_pImapMailbox->IsReadOnly())
	{
		CString buf;

		ErrorDialog( IDS_WARN_IMAP_RDONLY_MOVETO, pTargetToc->Name());

		goto end;
	}

	// We determine if to continue based on the value of this.
	bResult = TRUE;

	// If this is a MOVE, check if source mailbox is read-only and issue warning.
	if (!Copy)
	{
		if (m_pImapMailbox->IsReadOnly())
		{
			CString buf;

			buf.Format (CRString (IDS_WARN_IMAP_RDONLY_MOVEFROM), Name());
			MessageBeep(MB_ICONEXCLAMATION);
			bResult = ( IDYES == ::AfxMessageBox(buf, MB_ICONQUESTION | MB_YESNO) );

			// Set Copy to TRUE if user wants to continue.
			if (bResult)
				Copy = TRUE;
		}
	}

	if (!bResult)
	{
		goto end;;
	}

	// If we are in the foreground, we can put up a progress window here!
	if ( ::IsMainThreadMT() && (SingleSum == NULL))
	{
		CString buf;
		buf.Format (CRString(IDS_IMAP_TRANSFERING_MSGS_TO), pTargetToc->Name());
		MainProgress(buf);

		ProgressIsUp = TRUE;
	}

	// The destination (IMAP) mailbox may not be open so we may just be copying these
	// on the server. In that case, we need to manually "bold" the destination
	// mailbox in the mailbox tree, in which case, we have to determine here if we are copying message
	// that are considered "unread".
	//
	long FreshnessDate;
	BOOL bCopiedUnread;

	FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
	bCopiedUnread = FALSE;

	CTocView* View;  View = GetView();

	//
	// Calculate the number of messages we'd be xfering.
	//
	pos = m_Sums.GetHeadPosition();

	i = 0;
	nMsgs = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// Check if this is an "unread" message:
		//
		if (Sum->m_State == MS_UNREAD && Sum->m_Seconds > FreshnessDate)
			bCopiedUnread = TRUE;

		nMsgs++;

		if (SingleSum)
			break;
	}

	// 
	// Must have some.
	//
	if (nMsgs <= 0)
	{
		goto end;
	}

	// Now, loop through the messages and COPY one at a time.
	// NOTE:  Don't remove the source tocs yet!!
	// Note; As a message is copied, its UID is removed from ImapQueue so if there's
	// a failure, we can go undelete what left in ImapQueue.
	// At the same time, accumulate a new list in ImapMoveSumList of messages actually 
	// copied.

	// Set redraw of the target view to FALSE so we don't get flicker
	CTocView *TargetView; 	TargetView = NULL;

	if (pTargetToc)
		TargetView = pTargetToc->GetView();

	if (TargetView)
		TargetView->m_SumListBox.SetRedraw(FALSE);

	BOOL bPartialSuccess; bPartialSuccess = FALSE;
	BOOL bPartialFailure; bPartialFailure = FALSE;

//	POSITION UidPos, UidNext; UidPos = UidNext = NULL;

	// This may take some time!!.
	if ( ProgressIsUp && (nMsgs > 0) )
	{
		CountdownProgress (CRString(IDS_IMAP_XFERMSGS_LEFT), nMsgs);
		Progress(0, NULL, nMsgs);

		CountdownProgressIsUp = TRUE;
	}

	pos = m_Sums.GetHeadPosition();
	::AsyncEscapePressed(TRUE);

	i = 0;
	count = 0;

	// Clear this before we loop.
	sUidlist.Empty();

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Give some time back, and let user cancel out by hitting Esc
		if (AsyncEscapePressed())
		{
			bPartialFailure = TRUE;
			break;
		}

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!Copy)
		{
			CFrameWnd* Frame = Sum->m_FrameWnd;
			if (Frame)
			{
				if (!(Frame->GetActiveDocument()->CanCloseFrame(Frame))) continue;
			}
		}

		// Copy the message but DON'T remove the source TOC or the message!.
		// Note: Always reset pCopySum.
		pCopySum = NULL;
		if ( !ImapCopyMsgBetweenAccounts (Sum, pTargetToc, &pCopySum) )
		{
			bPartialFailure = TRUE;
			break;
		}

		// We succeeded at least once.
		bPartialSuccess = TRUE;

		// Set what will be returned:
		if (pCopySum)
			pLastCopySum = pCopySum;

		// If a Move, accumulate UID's into a list as well as a comma-separated 
		// string.
		if (!Copy)
		{

			// Add to ImapMoveSumList for later deletion.
			// NOTE: This MUST be added in the same order as m_Sums!!!
			//
			ImapMoveSumList.AddTail (Sum);

			// Also create a comma-separated array.
			if (sUidlist.IsEmpty())
				sUidlist.Format("%lu", Sum->m_Hash);
			else
			{
				sUid.Format (",%lu", Sum->m_Hash);
				sUidlist += sUid;
			}
		}

		// If this is not a copy, add to the target undo list here.
		// Must add to the source undo list when the source summary is actually removed.
		if (!Copy)
		{
#if 0 // JOK - Undo doesn't work.

			// Add to the UndoTo list of the target mailbox
			if (pCopySum)
				pTargetToc->m_UndoXferTo.AddTail(pCopySum);
#endif // JOK

		}

		if ( CountdownProgressIsUp )
		{
			DecrementCountdownProgress();

			Progress(++count);
		}

		if (SingleSum)
			break;
	}

	//
	// Mark "NextPos" for later use. Note: Could be NULL.
	//
	LastSelectedPos = NextPos;

	// Resume redraw of target.
	if (TargetView)
		TargetView->m_SumListBox.SetRedraw(TRUE);

	// OK. See if we succeeded in copying any messages.
	if (!bPartialSuccess)
	{
		ErrorDialog(IDS_ERR_IMAP_NO_MSGS_XFERRED);
		bResult = FALSE;
	}

	if (!bResult)
	{
		goto end;
	}


	// Ok. Some were copied. If this is a MOVE, go flag the source messages as \deleted.
	// Note: This may set HighlightIndex for us. "ImapQueue" and "HighlightIndex" are both passed
	// as references.

	if (!Copy && !sUidlist.IsEmpty())
	{
		bUseFancyTrash = m_pImapMailbox->UseFancyTrash();

		if (bUseFancyTrash)
		{
			bResult = DoFancyExpunge ((LPCSTR) sUidlist);
		}
		else
		{
			bResult = SUCCEEDED ( m_pImapMailbox->DeleteMessagesFromServer ( (LPCSTR) sUidlist, TRUE) );
		}

		if (!bResult)
		{
			// Error message.
			// The delete failed. Display error message.
			TCHAR  buf [512];
			int iType = IMAPERR_BAD_CODE;

			buf[0] = '\0';
			m_pImapMailbox->GetLastImapErrorAndType (buf, 510, &iType);

			ErrorDialog (IDS_ERR_IMAP_CANNOT_MODIFYFLAG, Name(), buf);

			goto end;
		}

		if (!bUseFancyTrash)
		{
			// Stop redraw of the source.
			CTocView *View = GetView();
			if (View)
				View->m_SumListBox.SetRedraw(FALSE);

			// Note: If SingleSum, don't close the window. Allow "DisplayNextMessageOrClose"
			// to close it if necessary.
			//

			BOOL bAllowCloseWindow = CloseWindow && !SingleSum;

			// Just flag them for deletion.	
			bResult = ImapRemoveOrderedSubSums (&ImapMoveSumList, HighlightIndex, bAllowCloseWindow, TRUE, TRUE);

			//
			// If tow-truck transfer, display next summary.
			//
			if (!bAllowCloseWindow && bResult)
			{
				DisplayNextMessageOrClose (this, SingleSum, LastSelectedPos);
			}	

			// Redraw again.
			if (View)
			{
				View->m_SumListBox.SetRedraw(TRUE);
			}
		}
	}

	// Put up message if some messages did not get transferred.
	if (bPartialFailure)
		ErrorDialog (IDS_WARN_IMAP_SOME_XFER_FAILED);

	// Cleanup before we exit.
end:

	// Update the target mailbox, if it's open.
	if (TargetView && bPartialSuccess)
	{
		pTargetToc->m_pImapMailbox->DoManualResync (pTargetToc, TRUE, FALSE, FALSE);
	}

	// If we copied unread messages to the destination mailbox, uodate the status.
	if (bPartialSuccess && bCopiedUnread)
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( pTargetToc->MBFilename() );
		if (pCommand)
			pCommand->Execute( CA_UPDATE_STATUS, ( void* ) US_YES );
	}


	// Make sure Free memory in Imapqueue and IMapCopyQueue.
	ImapQueue.RemoveAll();
	ImapMoveSumList.RemoveAll();

	if ( ProgressIsUp )
	{
		CloseProgress();
	}

	return pLastCopySum;
}



// FUNCTION
// Handle transfers from am IMAP mailbox to a local one.
// Return the index to be highlighted, or -1 if error.
// END FUNCTION

// NOTE: Assume that a connection to our IMAP mailbox is already open.

CSummary* CTocDoc::ImapXferFromImapToLocal (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/)
{
	POSITION	pos = NULL, NextPos = NULL;
	CString		sUidlist, sUid;
	CSumList	ImapQueue;
	CSumList	ImapMoveSumList;
	BOOL		bResult = TRUE;
	int			i = 0;
	CSummary*	pCopySum = NULL;
	CSummary*	pLastCopySum = NULL;
	BOOL		ProgressIsUp = FALSE;
	BOOL		CountdownProgressIsUp = FALSE;
	int			Count = 0;
	POSITION	LastSelectedPos = NULL;

	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return NULL;
	}

	// We determine if to continue based on the value of this.
	bResult = TRUE;

	// If this is a MOVE, check if source mailbox is read-only and issue warning.
	if (!Copy)
	{
		if (m_pImapMailbox->IsReadOnly())
		{
			CString buf;

			buf.Format (CRString (IDS_WARN_IMAP_RDONLY_MOVEFROM), Name());
			MessageBeep(MB_ICONEXCLAMATION);
			bResult = ( IDYES == ::AfxMessageBox(buf, MB_ICONQUESTION | MB_YESNO) );

			// Set Copy to TRUE if user wants to continue.
			if (bResult)
				Copy = TRUE;
		}
	}

	if (!bResult)
	{
		return NULL;
	}

	// If we are in the foreground, we can put up a progress window here!
	if ( ::IsMainThreadMT() && (SingleSum == NULL) && pTargetToc)
	{
		CString buf;

		buf.Format (CRString(IDS_IMAP_XFERING_TOLOCAL),
				Copy ? CRString (IDS_IMAP_COPYING) : CRString (IDS_IMAP_TRANSFERING),
				Name(), pTargetToc->Name());

		MainProgress(buf);

		ProgressIsUp = TRUE;
	}


	CTocView* View = GetView();


	// Whether we would be deleting the messages later or not, accumulate
	// the summaries into the ImapQueue CSumList.
	//

	Count = 0;

	pos = m_Sums.GetHeadPosition();

	i = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!Copy)
		{
			CFrameWnd* Frame = Sum->m_FrameWnd;
			if (Frame)
			{
				if (!(Frame->GetActiveDocument()->CanCloseFrame(Frame))) continue;
			}
		}

		// Ok. This is selected. Add to list.

		// Add to ImapQueue for later deletion.
		// Note!! These MUST be added in the same order as they currently exist
		// in m_Sums.
		//
		ImapQueue.AddTail (Sum);

		// Also create a comma-separated array.
		if (sUidlist.IsEmpty())
			sUidlist.Format("%lu", Sum->m_Hash);
		else
		{
			sUid.Format (",%lu", Sum->m_Hash);
			sUidlist += sUid;
		}

		// 
		Count++;

		if (SingleSum)
			break;
	}

	// If this is a MOVE, attempt to flag the messages a \\Deleted.
	//
	if (!Copy)
	{
		// Attempt to flag messages as deleted.
		bResult = SUCCEEDED ( m_pImapMailbox->DeleteMessagesFromServer ( (LPCSTR) sUidlist, FALSE, TRUE) );

		if (!bResult)
		{
			TCHAR  buf [512];
			CString msg;

			buf[0] = '\0';
			m_pImapMailbox->GetLastServerMessage (buf, 512);

			msg.Format( CRString(IDS_ERR_IMAP_DELMSGS_CONTINUE), Name(), buf);
			MessageBeep(0xFFFFFFFF);
			bResult = ( IDYES == ::AfxMessageBox(msg, MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) );

			// Set Copy to TRUE if user wants to continue.
			if (bResult)
			{
				Copy = TRUE;

				// Change progress message.
				if (ProgressIsUp && pTargetToc)
				{
					CString buf;
	
					buf.Format (CRString(IDS_IMAP_XFERING_TOLOCAL),
								CRString (IDS_IMAP_COPYING),
								Name(), pTargetToc->Name());

					MainProgress(buf);
				}
			}
		}
	}

	// Should we continue?
	if (!bResult)
	{
		// Free memory in Imapqueue and IMapCopyQueue.
		ImapQueue.RemoveAll();
		ImapMoveSumList.RemoveAll();

		if (ProgressIsUp)
			CloseProgress ();

		return NULL;
	}

	//
	// Count number of messages for progress bar. The summaries would now be in  
	// "ImapQueue" so just use it's count.
	//
	Count = ImapQueue.GetCount ();

	// This may take some time!!.
	if ( ProgressIsUp && (Count > 0) )
	{
		CountdownProgress (CRString(IDS_IMAP_XFERMSGS_LEFT), Count);
		Progress(0, NULL, Count);

		CountdownProgressIsUp = TRUE;
	}

	// Now, loop through ImapQueue and COPY one at a time.
	// NOTE:  Don't remove the source tocs yet!!
	// Note; As a message is copied, its summary is removed from ImapQueue so if there's
	// a failure, we can go undelete what left in ImapQueue.
	// At the same time, accumulate a new list in ImapMoveSumList of messages actually 
	// copied.

	// Set redraw of the target view to FALSE so we don't get flicker
	CTocView *TargetView = NULL;
	if (pTargetToc)
		TargetView = pTargetToc->GetView();

	if (TargetView && !SingleSum)
		TargetView->m_SumListBox.SetRedraw(FALSE);

	BOOL bPartialSuccess = FALSE;
	BOOL bPartialFailure = FALSE;

	::AsyncEscapePressed(TRUE);

	pos = ImapQueue.GetHeadPosition();
	i = 0;

	// Note: Reuse "Count":
	Count = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = ImapQueue.GetNext(NextPos);

		// Copy the message but DON'T remove the source TOC or the message!.
		// Note: Always reset pCopySum.
		pCopySum = NULL;
		if ( !ImapCopyMsgFromImapToLocal (Sum, pTargetToc, &pCopySum) )
		{
			bPartialFailure = TRUE;
			break;
		}

		// We succeeded at least once.
		bPartialSuccess = TRUE;

		// Set what will be returned:
		if (pCopySum)
			pLastCopySum = pCopySum;

		// Remove this from ImapQueue.
		if (!Copy)
		{
			// Add to ImapCopyQueue for later deletion.
			ImapMoveSumList.AddTail (Sum);

			// If we succeed in copying the message, delete this summary from ImapQueue.
			//
			ImapQueue.RemoveAt (pos);
		}

		// If this is not a copy, add to the target undo list here.
		// Must add to the source undo list when the source summary is actually removed.
		if (!Copy)
		{
#if 0 // JOK - Undo doesn't work
			// Add to the UndoTo list of the target mailbox
			if (pCopySum)
				pTargetToc->m_UndoXferTo.AddTail(pCopySum);
#endif
		}

		if ( CountdownProgressIsUp )
		{
			DecrementCountdownProgress();

			Progress(++Count);
		}

		if (SingleSum)
			break;
	}

	//
	// Mark "NextPos" for later use. Note: Could be NULL.
	//
	LastSelectedPos = NextPos;

	// Resume redraw of target.
	if (TargetView && !SingleSum)
		TargetView->m_SumListBox.SetRedraw(TRUE);

	// OK. See if we succeeded in copying any messages.
	if (!bPartialSuccess)
	{
		ErrorDialog(IDS_ERR_IMAP_NO_MSGS_XFERRED);
		bResult = FALSE;
	}

	if (!bResult)
	{
		// Free memory in Imapqueue and IMapCopyQueue.
		ImapQueue.RemoveAll();
		ImapMoveSumList.RemoveAll();

		if (ProgressIsUp)
			CloseProgress ();

		return NULL;
	}

	// Ok. Some were copied. This is now equivalent to removing the messages from the remote
	// mailbox and from the local cache.
	// Note: This may set HighlightIndex for us. "ImapQueue" and "HighlightIndex" are both passed
	// as references.

	BOOL		bUseFancyTrash = FALSE;

	if (!Copy)
	{
		// If we have un-copied messgaes, undelete them.
		sUidlist.Empty();

		pos = ImapQueue.GetHeadPosition();

		for (NextPos = pos; pos; pos = NextPos)
		{
			CSummary *Sum = ImapQueue.GetNext (NextPos);
	
			if (Sum)
			{
				if (sUidlist.IsEmpty())
					sUidlist.Format("%lu", Sum->m_Hash);
				else
				{
					sUid.Format (",%lu", Sum->m_Hash);
					sUidlist += sUid;
				}
			}

			ImapQueue.RemoveAt (pos);
		}

		if (!sUidlist.IsEmpty())
			m_pImapMailbox->UnDeleteMessagesFromServer ( (LPCSTR) sUidlist, TRUE);

		// Fancy trash??
		//
		bUseFancyTrash = m_pImapMailbox->UseFancyTrash();

		if (bUseFancyTrash)
		{
			// Bug!! - This will expunge previously \\deleted messages. Need to 
			// expunge only copied messages.
			//
			bResult = DoFancyExpunge (NULL);
		}
		else
		{
			// Stop redraw of the source.
			CTocView *View = GetView();
			if (View && !SingleSum)
				View->m_SumListBox.SetRedraw(FALSE);
	
			// Note: Just flag them for deletion.
			// 
			// Note: If SingleSum, don't close the window. Allow "DisplayNextMessageOrClose"
			// to close it if necessary.
			//

			BOOL bAllowCloseWindow = CloseWindow && !SingleSum;

			bResult = ImapRemoveOrderedSubSums (&ImapMoveSumList, HighlightIndex, bAllowCloseWindow, TRUE, TRUE);

			//
			// If tow-truck transfer, display next summary.
			//
			if (!bAllowCloseWindow && bResult)
			{
				DisplayNextMessageOrClose (this, SingleSum, LastSelectedPos);
			}

			// Redraw again.
			if (View && !SingleSum)
			{
				View->m_SumListBox.SetRedraw(TRUE);
			}
		}
	}

	// Put up message if some messages did not get transferred.
	if (bPartialFailure)
		ErrorDialog (IDS_WARN_IMAP_SOME_XFER_FAILED);

	// Make sure Free memory in Imapqueue and IMapCopyQueue.
	ImapQueue.RemoveAll();
	ImapMoveSumList.RemoveAll();

	// Make sure.
	if (ProgressIsUp)
		CloseProgress ();

	return pLastCopySum;
}





// FUNCTION
// Handle transfers from a local mailbox to an IMAP one.
// Return HighlightIndex or -1 if error.
// END FUNCTION

CSummary* CTocDoc::ImapXferFromLocalToImap (CTocDoc *pTargetToc, CSummary* SingleSum, int& HighlightIndex,
					BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/)
{
	POSITION	pos = NULL, NextPos = NULL;
	int			i = 0;
	CSummary*	pCopySum = NULL;
	CSummary*	pLastCopySum = NULL;
	BOOL		ProgressIsUp = FALSE;
	BOOL		CountdownProgressIsUp = FALSE;
	int			Count = 0;

	// Sanity: Must have a target toc.
	if (!pTargetToc)
		return NULL;
	
	// Target Toc must have a valid IMAP object.
	if (! pTargetToc->m_pImapMailbox )
	{
		ASSERT (0);
		return NULL;
	}

	// Check if source mailbox is read-only and issue warning.
	// We cannot copy/move message to a read-only mailbox.

	if (pTargetToc->m_pImapMailbox->IsReadOnly())
	{
		ErrorDialog( IDS_WARN_IMAP_RDONLY_MOVETO, pTargetToc->Name());

		return NULL;
	}

	//
	// Get the CImapAccount of the target mailbox so we canget the server name.
	//
	CString szMachineName;

	CImapAccount *pAccount = g_ImapAccountMgr.FindAccount ( pTargetToc->m_pImapMailbox->GetAccountID() );
	if (pAccount)
		pAccount->GetAddress (szMachineName);

	// If we are in the foreground, we can put up a progress window here!
	if ( ::IsMainThreadMT() && (SingleSum == NULL))
	{
		CString buf;

		buf.Format (CRString(IDS_IMAP_XFERING_TOREMOTE),
				Copy ? CRString (IDS_IMAP_COPYING) : CRString (IDS_IMAP_TRANSFERING),
				pTargetToc->Name(), szMachineName);

		MainProgress(buf);

		ProgressIsUp = TRUE;
	}


	// Initialize
	HighlightIndex = -1;

	// Stop updating both the source and destination mailboxes.
	CTocView* View = GetView();
	if (View && !SingleSum)
		View->m_SumListBox.SetRedraw(FALSE);
		
	CTocView *TargetView = pTargetToc->GetView();
	if (TargetView && !SingleSum)
		TargetView->m_SumListBox.SetRedraw(FALSE);

	//
	// Count number of messages for progress bar.
	//
	Count = 0;

	pos = m_Sums.GetHeadPosition();

	i = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// This is one.
		Count++;
	}

	// This may take some time!!.
	if ( ProgressIsUp && (Count > 0) )
	{
		CountdownProgress (CRString(IDS_IMAP_XFERMSGS_LEFT), Count);
		Progress(0, NULL, Count);

		CountdownProgressIsUp = TRUE;
	}

	// The destination (IMAP) mailbox may not be open so we may just be copying these
	// on the server. In that case, we need to manually "bold" the destination
	// mailbox in the mailbox tree, in which case, we have to determine here if we are copying message
	// that are considered "unread".
	//
	long FreshnessDate;
	BOOL bCopiedUnread;

	FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
	bCopiedUnread = FALSE;

	// Loop through the messages and COPY one at a time.
	// NOTE:  Don't remove the source tocs yet!!
	// Note; As a message is copied, its UID is removed from ImapQueue so if there's
	// a failure, we can go undelete what left in ImapQueue.
	// At the same time, accumulate a new list in ImapCopyQueue of messgae actually 
	// copied.

	BOOL bPartialSuccess = FALSE;
	BOOL bPartialFailure = FALSE;
	BOOL FoundSelected	 = FALSE;

	pos = m_Sums.GetHeadPosition();
	::AsyncEscapePressed(TRUE);

	i = 0;

	// 
	// Reuse Count
	//
	Count = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Give some time back, and let user cancel out by hitting Esc
		if (AsyncEscapePressed())
			break;

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			// See if we found the first unselected message after a selected message
			// and note its index for later selection
			if (!Copy && FoundSelected && HighlightIndex < 0)
				HighlightIndex = i;

			continue;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!Copy)
		{
			CFrameWnd* Frame = Sum->m_FrameWnd;
			if (Frame)
			{
				if (!(Frame->GetActiveDocument()->CanCloseFrame(Frame))) continue;
			}
		}

		if (View && !Copy && Sum == GetPreviewableSummary())
		{
			//
			// Unfortunately, a TocFrame has to keep a cached Summary pointer
			// for the message being displayed in the preview pane.  However,
			// this Xfer() method can nuke such Summary objects, leaving
			// the TocFrame with an invalid pointer.  So, before we get around
			// to nuking the Summary object below, we need to inform the frame
			// that the Summary is about to go away.
			//
			InvalidatePreviewableSummary(Sum);

			CView* pPreviewView = GetPreviewView();
			if (pPreviewView)
			{
				//
				// Check to see if preview pane has focus.  If so, then
				// just tweak the preview without opening a new
				// message window.
				//
				CWnd* pFocusWnd = CWnd::GetFocus();
				if (pPreviewView == pFocusWnd || pPreviewView->IsChild(pFocusWnd))
				{
					//
					// FORNOW, the big assumption here is that the change
					// focus event will be enough to kick off the normal
					// preview-the-summary-with-focus logic in the TocView.
					// This may change if we get away from a focus-based
					// preview scheme.
					//
					ASSERT(View);
					View->SetFocus();
				}
			}
		}

		FoundSelected = TRUE;

		// Copy the message to IMAP. pCopySum, if non-NULL, is the new destination summary object.
		// Note: Always reset pCopySum.
		pCopySum = NULL;
		if ( !ImapCopyMsgFromLocalToImap (Sum, pTargetToc, &pCopySum) )
		{
			bPartialFailure = TRUE;
			break;
		}

		// We succeeded at least once.
		bPartialSuccess = TRUE;

		// Check if this is an "unread" message:
		//
		if (Sum->m_State == MS_UNREAD && Sum->m_Seconds > FreshnessDate)
			bCopiedUnread = TRUE;


		// Set what will be returned:
		if (pCopySum)
		{
			pLastCopySum = pCopySum;

			if (!Copy && pTargetToc)
			{
#if 0 // JOK - Undo doesn't work - don't allow
				// Add to the target's UndoFrom list of this mailbox
				pTargetToc->m_UndoXferTo.AddTail(pCopySum);
#endif
			}
		}

		// If we succeed and this is a move,delete source summary if it's different from pCopySum.
		if (!Copy && Sum != pCopySum)
		{
			// If we're moving the summary from the out box, don't delete the summary 
			// because it can cause a crash in the post-filtering code (sendmail.cpp)
			// that re-uses the summary after post-filtering!! Instead, move it to the
			// local Trash folder.
			//

			if (Sum->IsComp() || m_Type == MBT_OUT)
			{
				CTocDoc *pTrashToc = GetTrashToc();
				if ( pTrashToc && MoveContents ( this, pTrashToc, Sum) )
				{
					// Move the summary.
					//
					SubtractSum(pos, i);

					pTrashToc->AddSum(Sum);
				}
			}
			else
			{
				// Remove any attachments before we delete the summary.
				//
				RemoveAttachments (this, Sum);

				// close the message window
				// (this used to be done in RemoveAttachments() -jdboyd 8/16/99
				CFrameWnd* Frame = Sum->m_FrameWnd;
				if (Frame)
				{
					if (Frame->GetActiveDocument()->CanCloseFrame(Frame))
					{
						Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
					}
				}

				// Includes free-ing memory.
				//
				RemoveSum(Sum);

				Sum = NULL;
			}
		}

		// We have deleted a source summary!
		if (!Copy)
			i--;

		if (SingleSum)
			break;

		// Update progress.
		//
		if ( CountdownProgressIsUp )
		{
			DecrementCountdownProgress();

			Progress(Count++);
		}
	}


	// Resume updating both the source and destination mailboxes.
	if (TargetView)
	{
			// If some were copied:
		if (bPartialSuccess)
		{
			// Update target.
			if ( pTargetToc && (pTargetToc->m_pImapMailbox != NULL) )
			{	
				// Update target in b/gnd.
				pTargetToc->m_pImapMailbox->DoManualResync (pTargetToc, TRUE, FALSE, FALSE);

			}
		}

		if (!SingleSum)
			TargetView->m_SumListBox.SetRedraw(TRUE);
	}

	if (View)
	{
		if (!Copy && HighlightIndex < 0)
		{
			if (SingleSum)
			{
				// If there isn't a selected summary, select the next one
				if (View->GetSelCount() == 0)
					HighlightIndex = i;
			}
			// Select the last summary if there weren't any other summaries selected
			else if (HighlightIndex < 0)
				HighlightIndex = NumSums() - 1;
		}
			
		if (HighlightIndex >= 0)
		{
			if (HighlightIndex >= NumSums())
				HighlightIndex = NumSums() - 1;
		}

		// Set redraw back to TRUE
		if (!SingleSum)
			View->m_SumListBox.SetRedraw(TRUE);
	}


	// OK. See if we succeeded in copying any messages.
	if (!bPartialSuccess)
	{
		ErrorDialog(IDS_ERR_IMAP_NO_MSGS_XFERRED);
	}
	else if (bPartialFailure)
	{
		ErrorDialog (IDS_WARN_IMAP_SOME_XFER_FAILED);
	}

	// If we copied unread messages to the destination mailbox, uodate the status.
	if (bPartialSuccess && bCopiedUnread && pTargetToc)
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( pTargetToc->MBFilename() );
		if (pCommand)
			pCommand->Execute( CA_UPDATE_STATUS, ( void* ) US_YES );
	}



	// Make sure.
	if (ProgressIsUp)
		CloseProgress ();

	return pLastCopySum;
}



// ImapXferMsgFromImapToLocal [PRIVATE]

// NOTES
// Fetches undownloaded body parts then copies them to the local Attach directory.
// NOTE: Don't remove source toc!!.

// NOTE: Returns a pointer to the new summary in the destination mailbox in "ppCopySum".

// END NOTES

BOOL CTocDoc::ImapCopyMsgFromImapToLocal (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum)
{
	BOOL		bResult = FALSE;
	long		MStart, NewLength;

	// Initialize ppCopySum.
	if (!ppCopySum)
		return FALSE;
	else
		*ppCopySum = NULL;

	// Sanity
	if (!(pSum && pTargetToc))
		return FALSE;

	// We must be an IMAP mailbox!
	if (!m_pImapMailbox)
		return FALSE;

	// Initialize
	MStart = NewLength = 0;
	bResult = FALSE;

	// Filter MBX stub as we copy. Also, if has attachments, download them..

	// Note: If successful, this will copy stuff to the end of the target MBX file and 
	// return new values for MStart and NewLength that are user below in creating
	// the new target TOC.
	bResult = RewriteMbxStub (pSum, this, pTargetToc, 0, MStart, NewLength);

	// Did we succeed?? If so, Copy the summary BUT DON'T DELETE THE SOURCE SUMMARY!!
	// Note: This applies to both attachment and non-attachment cases.
	if (bResult)
	{
		// turn this into a comp message yif we're transferring to the OUT box -jdboyd
		if (pTargetToc->m_Type == MBT_OUT)
		{
			CCompMessageDoc* comp = pSum->SendAgain(FALSE);
			if (comp)
			{
				if (comp->Write(NULL) != FALSE)
				{
					if (pSum->m_State == MS_SENT)
					{
						comp->m_Sum->SetState(MS_SENT);
						comp->m_Sum->SetDate(pSum->m_Seconds);
						comp->m_Sum->m_Seconds = pSum->m_Seconds;
					}

					// This is returned in ppCopySum
					*ppCopySum = comp->m_Sum;
	//				comp->OnCloseDocument();
				}
			}
		}
		else
		{
			CSummary *pCopySum = new CSummary;
			if (pCopySum)
			{
				pCopySum->Copy(pSum);

				pTargetToc->AddSum(pCopySum);
				pCopySum->SetOffset(MStart);
				pCopySum->m_Length = NewLength;

				// This is returned in ppCopySum.
				*ppCopySum = pCopySum;
			}
		}
	}
	
	return bResult;
}
			


// ImapCopyMsgBetweenAccounts [PRIVATE]
// NOTES 
// Does an APPEND to the remote mailbox.
// Note: Does NOT delete source TOC.
// NOTE: We are the source TOC.

// Note: If the destination server suports Optimize-1 and we can create the destination
// CSummary, that new summary is created and returned in ppCopySum, othersize NULL 
// is returned in ppCopySum..

// END NOTES

BOOL CTocDoc::ImapCopyMsgBetweenAccounts (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum)
{
	BOOL			bResult = FALSE;
	unsigned long	NewUid = 0;

	// Initialize ppCopySum.
	if (!ppCopySum)
		return FALSE;
	else
		*ppCopySum = NULL;

	// We must be an IMAP mailbox.
	if (!m_pImapMailbox)
	{
		ASSERT (0);
		return NULL;
	}

	// Sanity
	if (!(pSum && pTargetToc))
		return FALSE;

	// Target TOC must be an IMAP mailbox!
	if (!pTargetToc->m_pImapMailbox)
	{
		ASSERT (0);
		return FALSE;
	}

	// If the message does NOT have attachments, do an append from local to remote.
	if ( pSum->HasAttachment() )
	{
		// Download message text as-is from the server append to destination server.
		bResult = SUCCEEDED ( pTargetToc->m_pImapMailbox->AppendMessageAcrossRemotes (pSum, &NewUid, TRUE) );
	}
	else
	{
		// Accumulate source message into an RFC822 message file and call
		// the mailbox's append API.
		bResult = SUCCEEDED (pTargetToc->m_pImapMailbox->AppendMessageFromLocal (pSum, &NewUid));
	}

	// If the append failed, put up error message:
	if (!bResult)
	{
		TCHAR  buf [512];
		int iType = IMAPERR_BAD_CODE;

		buf[0] = '\0';

		pTargetToc->m_pImapMailbox->GetLastImapErrorAndType (buf, 510, &iType);

		if (iType != IMAPERR_USER_CANCELLED && iType != IMAPERR_LOCAL_ERROR)
		{
			ErrorDialog( IDS_ERR_IMAP_APPEND_FAILED, pTargetToc->Name(), buf);
		}
	}


	// If we got a new Uid, we can do the copy locally.
	if (bResult && NewUid)
	{
		long	MStart = 0, NewLength = 0;
		
		bResult = RewriteMbxStub (pSum, this, pTargetToc, NewUid, MStart, NewLength);
		if (bResult);
		{ 
			// Create target summary.
			CSummary *pCopySum = new CSummary;
			if (pCopySum)
			{
				pCopySum->Copy(pSum);

				pTargetToc->AddSum(pCopySum);
				pCopySum->SetOffset(MStart);
				pCopySum->m_Length = NewLength;

				// This is returned in ppCopySum.
				*ppCopySum = pCopySum;
			}
		}
	}

	return bResult;
}




// ImapXferMsgFromLocalToImap [PRIVATE]
// NOTES 
// Does an APPEND to the remote mailbox.
// Note: Does NOT delete source TOC.
// NOTE: We are the source TOC.

// Note: If the server suports Iptimize-1 and we can create the destination CSummary, that
// new summary is created and returned in ppCopySum, othersize NULL is returned in ppCopySum..

// END NOTES

BOOL CTocDoc::ImapCopyMsgFromLocalToImap (CSummary *pSum, CTocDoc *pTargetToc, CSummary** ppCopySum)
{
	BOOL			bResult = FALSE;
	unsigned long	NewUid = 0;

	// Initialize ppCopySum.
	if (!ppCopySum)
		return FALSE;
	else
		*ppCopySum = NULL;

	// Sanity
	if (!(pSum && pTargetToc))
		return FALSE;

	// Target TOC must be an IMAP mailbox!
	if (!pTargetToc->m_pImapMailbox)
		return FALSE;

	// Accumulate source message into an RFC822 message file and call
	// the mailbox's append API.

	bResult = SUCCEEDED (pTargetToc->m_pImapMailbox->AppendMessageFromLocal (pSum, &NewUid));

	// If the append failed, put up error message:
	if (!bResult)
	{
		TCHAR  buf [512];
		int iType = IMAPERR_BAD_CODE;

		buf[0] = '\0';

		pTargetToc->m_pImapMailbox->GetLastImapErrorAndType (buf, 510, &iType);

		if (iType != IMAPERR_USER_CANCELLED && iType != IMAPERR_LOCAL_ERROR)
		{
			ErrorDialog( IDS_ERR_IMAP_APPEND_FAILED, pTargetToc->Name(), buf);
		}
	}

	// If we got a new Uid, we can do the copy locally.
	if (bResult && NewUid)
	{
		long	MStart = 0, NewLength = 0;
		
		bResult = RewriteMbxStub (pSum, this, pTargetToc, NewUid, MStart, NewLength);
		if (bResult);
		{ 
			// Create target summary.
			CSummary *pCopySum = new CSummary;
			if (pCopySum)
			{
				pCopySum->Copy(pSum);

				pTargetToc->AddSum(pCopySum);
				pCopySum->SetOffset(MStart);
				pCopySum->m_Length = NewLength;

				// This is returned in ppCopySum.
				*ppCopySum = pCopySum;
			}
		}
	}

	return bResult;
}



// FUNCTION [PUBLIC] 
// Loop through the summary list and delete messages with UID's given in pUidList.
// Also set HighlightIndex.
// END FUNCTION

// NOTE:
// This can be called directly from outside.
// END NOTE

// NOTE:
// NOTE: pUidList should be cast to a CPtrUidList*.
// Don't allow user to stop this process.
// NOTE:
// If called from outside, the caller can set "SetViewSelection" to TRUE and we will
// set the selected index for him.
// NOTE: (JOK - 10/21/97) If bJustSetFlag == TRUE,the summaries aren't 
// removed. They are just flagged for deletion!!!
// END NOTES!
 
BOOL CTocDoc::ImapRemoveListedTocs (CUidMap *pUidMap,
			 int& HighlightIndex, BOOL CloseWindow /* = TRUE */,
			 BOOL SetViewSelection /* = FALSE */, 
			 BOOL bJustSetFlag /* = FALSE */)
{
	BOOL FoundSelected = FALSE;
	POSITION pos, NextPos;
	CTocView  *View;
	int i = 0;

	// Initialize.
	HighlightIndex = -1;

	// If no UID list, then nothing to do.
	if (!pUidMap)
		return TRUE;

	// If nothing to remove, don't continue.
	if (pUidMap->GetCount () == 0)
		return TRUE;

	View = GetView();

	// Stop updating the view until we're done.
	if (View)
		View->m_SumListBox.SetRedraw(FALSE);

	pos = m_Sums.GetHeadPosition();

	BOOL bUidFound;

	
	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// If this is in pUidList, we've found one. 
		// Note: This is a in-efficient search because it's O(n**2)!!
		//
		bUidFound = FALSE;

		// If this is in pUidMap, we've found one. 
		//
		UidIteratorType ci = pUidMap->find ( Sum->GetHash() );

		if ( ci != pUidMap->end() )
		{
			// Found it. Note: Don't remove it from pUidMap!! This way we can
			// catch any duplicated local summaries.
			//

			bUidFound = TRUE;
		}

		if (!bUidFound)
		{
			// If this is selected, set HighLight to it.
			if (View && View->m_SumListBox.GetSel(i) > 0)
			{
				// We found one that's not to be deleted. Set this as the one
				// to highlight finally.
				//
				HighlightIndex = i;
			}

			continue;
		}

		// If this is selected and we haven't yet found a selection, set this as the
		// value to highlight. It will highlight the next in line.
		if (View && View->m_SumListBox.GetSel(i) > 0)
		{
			if (HighlightIndex < 0)
				HighlightIndex = i;
		}

		//
		// Ok. We've got one.
		//
		// Close window if need be
		//
		// Note: Don't close the read-message window if told to.
		CFrameWnd* Frame = Sum->m_FrameWnd;
		if (CloseWindow && Frame)
		{
			if (!Frame->GetActiveDocument()->CanCloseFrame(Frame))
			{
				if (HighlightIndex < 0)
					HighlightIndex = i;
				break;
			}

			Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);		// this is the new way
		}

		// Note: No need to do this if we're just flagging.
		if ( View && (Sum == GetPreviewableSummary()) )
		{
			//
			// Unfortunately, a TocFrame has to keep a cached Summary pointer
			// for the message being displayed in the preview pane.  However,
			// this Xfer() method can nuke such Summary objects, leaving
			// the TocFrame with an invalid pointer.  So, before we get around
			// to nuking the Summary object below, we need to inform the frame
			// that the Summary is about to go away.
			//
			InvalidatePreviewableSummary(Sum);

			CView* pPreviewView = GetPreviewView();
			if (pPreviewView)
			{
				//
				// Check to see if preview pane has focus.  If so, then
				// just tweak the preview without opening a new
				// message window.
				//
				CWnd* pFocusWnd = CWnd::GetFocus();
				if (pPreviewView == pFocusWnd || pPreviewView->IsChild(pFocusWnd))
				{
					//
					// FORNOW, the big assumption here is that the change
					// focus event will be enough to kick off the normal
					// preview-the-summary-with-focus logic in the TocView.
					// This may change if we get away from a focus-based
					// preview scheme.
					//
					ASSERT(View);
					View->SetFocus();
				}
			}
		}

		FoundSelected = TRUE;

		// If we're just flagging, do that.
		if (bJustSetFlag)
		{
			Sum->m_Imflags |= IMFLAGS_DELETED;
		}
		else
		{
			// Remove any attachments before we delete the summary.
			//
			RemoveAttachments (this, Sum);

			// Remove the summary, including freeing memory.
	
			RemoveSum(Sum);

			// We removed a summary, so decrement counter.
			i--;
		}
	}

	// Set HighlightIndex if the previously highlighted index was deleted.
	if (View)
	{
		if (HighlightIndex < 0)
		{
			// Select the last summary if there weren't any other summaries selected
			if (HighlightIndex < 0)
				HighlightIndex = NumSums() - 1;
		}
			
		if (HighlightIndex >= 0)
		{
			if (HighlightIndex >= NumSums())
				HighlightIndex = NumSums() - 1;
		}

		// Redraw.
		View->m_SumListBox.SetRedraw(TRUE);

		// Should we set the selection for the user??
		if (SetViewSelection)
		{
			if (HighlightIndex >= 0)
			{
				View->m_SumListBox.SetSel(HighlightIndex);
				MailboxChangedByHuman();
			}

			// If called from outside, update the window also.
			// If no more summaries, update the whole frame, otherwise just the 
			// list.
			//
			View->m_SumListBox.Invalidate();
			View->m_SumListBox.UpdateWindow();

			if (NumSums () <= 0)
			{
				CTocFrame* pTocFrame = (CTocFrame *) View->GetParentFrame();
				if (pTocFrame)
				{
					ASSERT_KINDOF(CTocFrame, pTocFrame);

					pTocFrame->Invalidate();

					pTocFrame->UpdateWindow();
				}
			}
		}
	}

	return TRUE;
}





// ImapRemoveOrderedSubSums [PUBLIC]
//
// FUNCTION [PUBLIC] 
//
// Loop through the TOC's summary list and delete summaries that are also
// in the given "pOrderedSubSumList".
//
// NOTE (VERY IMPORTANT!!!): pOrderedSubSumList MUST be in the same order as 
// m_Sums!!!
//
// END FUNCTION

// NOTE:
// This can be called directly from outside.
// END NOTE

// NOTE:
// Don't allow user to stop this process.
// NOTE:
// If called from outside, the caller can set "SetViewSelection" to TRUE and we will
// set the selected index for him based on an internal algorithm.
// NOTE: (JOK - 10/21/97) If bJustSetFlag == TRUE,the summaries aren't 
// removed. They are just flagged for deletion!!!
//
// NOTE: If "bUndelete", then this is a request to remove the IMFLAGS_DELETED
// flag from the messages.
//
// END NOTES.
 
BOOL CTocDoc::ImapRemoveOrderedSubSums (CSumList *pOrderedSubSumList,
			 int& HighlightIndex, BOOL CloseWindow /* = TRUE */,
			 BOOL SetViewSelection /* = FALSE */, 
			 BOOL bJustSetFlag /* = FALSE */, BOOL bUndelete /* = FALSE */)
{
	POSITION pos, NextPos;
	POSITION UidPos;
	CTocView  *View;
	int i = 0;
	int LastSelectedIndex = -1;

	// Initialize.
	HighlightIndex = -1;

	// If no summaries in the TOC. get out.
	if (NumSums () == 0)
		return TRUE;

	// If no summary list, then nothing to do.
	if (!pOrderedSubSumList)
		return TRUE;

	// If nothing to remove, don't continue.
	if (pOrderedSubSumList->GetCount () == 0)
		return TRUE;

	View = GetView();

	// Stop updating the view until we're done.
	if (View)
		View->m_SumListBox.SetRedraw(FALSE);

	// Disable preview while we're doing this!!
	SetPreviewableSummary (NULL);

	pos = m_Sums.GetHeadPosition();

	i = 0;
	LastSelectedIndex = -1;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Get the last selected summary. 
		//
		if (View)
		{
			if (View->m_SumListBox.GetSel(i) > 0)
			{
				LastSelectedIndex = i;

				// If caller wants us to set selection for him, de-select 
				// this summary if it's selected.
				//
				if ( SetViewSelection )
				{
					// Remove selection 
					View->m_SumListBox.SetSel(i, FALSE);
				}
			}
		}

		//
		// NOTE: WE ASSUME that pOrderedSubSumList is in the same order as 
		// m_Sums, so we're always picking from the top of pOrderedSubSumList.
		//

		UidPos = pOrderedSubSumList->GetHeadPosition();

		//
		// Have we exhausted pOrderedSubSumList??
		// Note: Loop through all summaries whether there in the ordered list or
		// not, because we might need to de-select them.
		//
		if (UidPos)
		{
			CSummary *pSubSum = (CSummary *) pOrderedSubSumList->GetAt (UidPos);
	
			if ( ! (pSubSum && (Sum == pSubSum) ) )
			{
				// Keep searching.
				continue;
			}

			// Delete the summary entry from the sub sumlist.
			pOrderedSubSumList->RemoveAt (UidPos);

			//
			// Ok. We've got one.
			//
			// Close window if need be
			//
			// Note: Close the read-message window if told to.
			CFrameWnd* Frame = Sum->m_FrameWnd;
			if (CloseWindow && Frame)
			{
				Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);	

//				Frame->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);	
			}

			// If we're just flagging, do that.
			if (bJustSetFlag)
			{
				// Is this a DELETE or an UNDELETE??
				if (bUndelete)
				{
					// Remove the flag.
					Sum->m_Imflags &= ~IMFLAGS_DELETED;
				}
				else
				{
					// Set the flag.
					Sum->m_Imflags |= IMFLAGS_DELETED;
				}
			}
			else
			{
				// Remove any attachments before we delete the summary.
				//
				RemoveAttachments (this, Sum);

				// Remove the summary, including freeing memory.
		
				RemoveSum(Sum);

				// We removed a summary, so decrement counter.
				i--;
			}
		} // if UidPos.
	}

	// sET HighlightIndex.
	// If there is a summary after "LastSelectedIndex", select that one.
	//
	HighlightIndex = LastSelectedIndex + 1;

	if ( NumSums() <= LastSelectedIndex )
		LastSelectedIndex = NumSums() - 1;

	//
	// Set the selection??
	//
	if (View)
	{
		// Redraw.
		View->m_SumListBox.SetRedraw(TRUE);

		// Should we set the selection for the user??
		if (SetViewSelection)
		{
			View->m_SumListBox.SetSel(HighlightIndex);

			// NOTE:
			MailboxChangedByHuman();

			// If called from outside, update the window also.
			// If no more summaries, update the whole frame, otherwise just the 
			// list.
			//
			View->m_SumListBox.Invalidate();
			View->m_SumListBox.UpdateWindow();

			if (NumSums () <= 0)
			{
				CTocFrame* pTocFrame = (CTocFrame *) View->GetParentFrame();
				if (pTocFrame)
				{
					ASSERT_KINDOF(CTocFrame, pTocFrame);

					pTocFrame->Invalidate();

					pTocFrame->UpdateWindow();
				}
			}
		}
	}

	return TRUE;
}





// FUNCTION [PUBLIC] 
// Loop through the summary list and remove all summaries.
// END FUNCTION

// NOTE:
// This can be called directly from outside.
// END NOTE


void CTocDoc::ImapRemoveAllSummaries ()
{
	POSITION pos, NextPos;
	CTocView  *View;
	int i = 0;


	View = GetView();

	// Stop updating the view until we're done.
	if (View)
		View->m_SumListBox.SetRedraw(FALSE);

	pos = m_Sums.GetHeadPosition();

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		//
		// Close window if open.
		//
		CFrameWnd* Frame = Sum->m_FrameWnd;

		if (Frame)
		{
			Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);

//			Frame->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
		}


		// Make sure we don't start a preview..
		if ( View && (Sum == GetPreviewableSummary()) )
		{
			//
			// Unfortunately, a TocFrame has to keep a cached Summary pointer
			// for the message being displayed in the preview pane.  However,
			// this Xfer() method can nuke such Summary objects, leaving
			// the TocFrame with an invalid pointer.  So, before we get around
			// to nuking the Summary object below, we need to inform the frame
			// that the Summary is about to go away.
			//
			InvalidatePreviewableSummary(Sum);

			CView* pPreviewView = GetPreviewView();
			if (pPreviewView)
			{
				//
				// Check to see if preview pane has focus.  If so, then
				// just tweak the preview without opening a new
				// message window.
				//
				CWnd* pFocusWnd = CWnd::GetFocus();
				if (pPreviewView == pFocusWnd || pPreviewView->IsChild(pFocusWnd))
				{
					//
					// FORNOW, the big assumption here is that the change
					// focus event will be enough to kick off the normal
					// preview-the-summary-with-focus logic in the TocView.
					// This may change if we get away from a focus-based
					// preview scheme.
					//
					ASSERT(View);
					View->SetFocus();
				}
			}
		}

		// Remove any attachments before we delete the summary.
		//
		RemoveAttachments (this, Sum);

		// Remove the summary, including freeing memory.
	
		RemoveSum(Sum);

			// We removed a summary, so decrement counter.
		i--;
	}

	// Set HighlightIndex if the previously highlighted index was deleted.
	if (View)
	{
		// Redraw.
		View->m_SumListBox.SetRedraw(TRUE);

		// We need to do this!
		CTocFrame* pTocFrame = (CTocFrame *) View->GetParentFrame();
		if (pTocFrame)
		{
			ASSERT_KINDOF(CTocFrame, pTocFrame);

			pTocFrame->Invalidate();
			pTocFrame->UpdateWindow();
		}
	}
}






// ImapDeleteMessages [PUBLIC]
// FUNCTION
// Implement mesage deletion as setflag to (\Deleted).
// If "Expunge" is TRUE, expunge the mailbox also.
// Implement this by calling ImapFlagMessages() below.
// If "bExpunge", go expunge the mailbox also.
// END FUNCTION

HRESULT CTocDoc::ImapDeleteMessages (CSummary *SingleSum /* = NULL */, BOOL bExpunge /* = FALSE */, BOOL bSilent /* = TRUE */)
{
	HRESULT hResult = E_FAIL;
	BOOL	  bMustCloseProgress = FALSE;

	// Sanity:
	// 
	if (!m_pImapMailbox)
		return E_FAIL;

	// If we're offline, ask what to do:
	//
	BOOL bMustCloseConnection = FALSE;

	if ( !m_pImapMailbox->IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
		{
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);

			// And fall through.
		}
		else if (res == IDCANCEL)
		{
			return S_OK;

			// And fall through.
		}
		else
		{
			// Single connection allowed..
			//
			bMustCloseConnection = TRUE;
		}
	}

	// Put up progress bar if it's not already up.
	if (!InProgress && !bSilent && ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format (CRString(IDS_IMAP_DELETING_MSGS));

		MainProgress(buf);

		bMustCloseProgress = TRUE;
	}

	// Either copy to trash or flag messages.
	//
	BOOL bUseFancyTrash = m_pImapMailbox->UseFancyTrash();

	if (bUseFancyTrash)
		hResult = ImapMoveMsgsToTrash (SingleSum);
	else
		hResult = ImapFlagDeleteMsgs (SingleSum, bExpunge, bSilent);

	if (bMustCloseProgress)
		CloseProgress ();

	// Force a re-prewiew.
	//
	CTocView *pView = GetView();

	if (pView)
		pView->SetFocus();

	if (bMustCloseConnection)
		m_pImapMailbox->Close();

	return hResult;
}



// ImapUnDeleteMessages [PUBLIC]
// FUNCTION
// Remove the (\Deleted) flag from selected messages (or SingleSum if given).
// END FUNCTION

HRESULT CTocDoc::ImapUnDeleteMessages (CSummary *SingleSum /* = NULL */, BOOL bSilent /* = TRUE */)
{
	HRESULT	  hResult = E_FAIL;
	BOOL	  bMustCloseProgress = FALSE;

	// If we're offline, ask what to do:
	//
	BOOL bMustCloseConnection = FALSE;

	if ( !m_pImapMailbox->IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
		{
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);

			// And fall through.
		}
		else if (res == IDCANCEL)
		{
			return S_OK;

			// And fall through.
		}
		else
		{
			// Single connection allowed..
			//
			bMustCloseConnection = TRUE;
		}
	}

	// Put up progress bar if it's not already up.
	if (!InProgress && !bSilent && ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format (CRString(IDS_IMAP_UNDELETING_MSGS));

		MainProgress(buf);

		bMustCloseProgress = TRUE;
	}
	
	// Route the call to ImapFlagMessages()
	hResult = ImapFlagMessages (SingleSum, TRUE);

	if (bMustCloseProgress)
		CloseProgress ();

	// Force a re-prewiew.
	//
	CTocView *pView = GetView();

	if (pView)
		pView->SetFocus();

	if (bMustCloseConnection)
		m_pImapMailbox->Close();

	return hResult;
}



// ImapExpunge [PUBLIC]
// FUNCTION
// Expunge mailbox. Later, must implement selective expunge, 
// i.e., expunge only messages we have marked as deleted.
// END FUNCTION

HRESULT CTocDoc::ImapExpunge (BOOL bSilent /* = TRUE */)
{
	HRESULT hResult	= E_FAIL;
	BOOL    ProgressIsUp	= FALSE;

	if ( !IsImapToc () )
		return E_FAIL;

	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If we're offline, ask what to do:
	//
	BOOL bMustCloseConnection = FALSE;

	if ( !m_pImapMailbox->IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
		{
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);

			// And fall through.
		}
		else if (res == IDCANCEL)
		{
			return S_OK;

			// And fall through.
		}
		else
		{
			// Single connection allowed..
			//
			bMustCloseConnection = TRUE;
		}
	}

	// Put up progress bar.
	// If we are in the foreground, we can put up a progress window here!
	if ( !bSilent && ::IsMainThreadMT() )
	{
		CString buf;
		buf.Format (CRString(IDS_IMAP_PURGING_REMOTE), Name());

		MainProgress(buf);

		ProgressIsUp = TRUE;
	}

	// In case the progress is not shown immediately:
	//
	CCursor cur;

	// Expunge from the server mailbox.
	hResult = m_pImapMailbox->Expunge();

	// If we succeeded, update the mailbox. Do a full resync.
	if ( SUCCEEDED (hResult) )
	{
		m_pImapMailbox->DoManualResync (this, FALSE, TRUE, TRUE);

		// Must re-serialize the TOC:
		//
		Write();
	}

	// Remove progress.
	if ( ProgressIsUp )
	{
		CloseProgress();
	}

	// Resume updates.
	CTocView *pView = GetView();
	if (pView)
	{
		pView->m_SumListBox.SetRedraw(TRUE);

		// If no more messages, there is a redraw problem.
		if (m_Sums.GetCount() == 0)
		{
			CTocFrame* pTocFrame = (CTocFrame *) pView->GetParentFrame();
			if (pTocFrame)
			{
				ASSERT_KINDOF(CTocFrame, pTocFrame);
				pTocFrame->Invalidate ();
				pTocFrame->UpdateWindow ();
			}

			pView->m_SumListBox.SetRedraw(TRUE);
			pView->m_SumListBox.Invalidate ();
			pView->m_SumListBox.UpdateWindow();

			// Also the view itself.
			pView->Invalidate();
			pView->UpdateWindow();
		}

		//
		// Set the focus back to the sumlist!
		//
		pView->SetFocus();
	}

	if ( bMustCloseConnection )
	{
		m_pImapMailbox->Close();
	}

	return hResult;
}




// DoFancyExpunge [PRIVATE]
//
BOOL CTocDoc::DoFancyExpunge (LPCSTR pUidlist)
{
	// Expunge from the server mailbox.
	CUidMap mapUidsActuallyRemoved;

	BOOL bResult = SUCCEEDED(m_pImapMailbox->UIDExpunge(pUidlist, mapUidsActuallyRemoved));

	// Remove the summaries corresponding to the UID's that were actually expunged.
	//
	if ( bResult )
	{
		int HighlightIndex;
	
		ImapRemoveListedTocs (&mapUidsActuallyRemoved,
						HighlightIndex, TRUE,			// CloseWindow
						TRUE,							// SetViewSelection
						FALSE);							// bJustSetFlag,

		// Force a serialization of the TOC:
		//
		// Write();
	}

	return bResult;
}





// ImapMoveMsgsToTrash [PRIVATE]
//
// Internal method to do the real work of the fancy trash.
//
HRESULT CTocDoc::ImapMoveMsgsToTrash (CSummary* SingleSum)
{
	HRESULT		hResult = S_OK;
	BOOL		bRet = FALSE;
	POSITION	LastSelectedPos = NULL;

	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the mailbox is read-only, we can't continue.
	if (m_pImapMailbox->IsReadOnly())
	{
		CString buf;

		ErrorDialog( IDS_WARN_IMAP_RDONLY_MODIFYFLAG, Name());

		return E_FAIL;
	}

	// Get the view so we can find selected messages.
	CTocView *View = GetView ();

	// Accumulate a uid list so we can do a block IMAP command.
	//
	POSITION	pos = m_Sums.GetHeadPosition();
	POSITION	NextPos;
	CString		sUidlist, sUid;
	int			i = 0;
	int			nCount = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// Ok. This is selected. Add to list.
		if (sUidlist.IsEmpty())
			sUidlist.Format("%lu", Sum->m_Hash);
		else
		{
			sUid.Format (",%lu", Sum->m_Hash);
			sUidlist += sUid;
		}

		nCount++;

		if (SingleSum)
			break;
	}

	// Must have found some.
	if (nCount <= 0)
	{
		return S_OK;
	}

	//
	// Mark "NextPos" for later use. Note: Could be NULL.
	//
	LastSelectedPos = NextPos;

	// Ok, ask the CImapMailbox to do the xfer for us. If it succeeds, it will return
	// a UidMap of the UID's that were actually removed from the mailbox.
	//
	CUidMap mapUidsActuallyRemoved;

	hResult = m_pImapMailbox->MoveMsgsToTrash ( (LPCSTR) sUidlist, mapUidsActuallyRemoved);

	// Remove the summaries corresponding to the UID's that were actually expunged.
	//
	if ( SUCCEEDED (hResult) )
	{
		int HighlightIndex;

		ImapRemoveListedTocs (&mapUidsActuallyRemoved,
						HighlightIndex, TRUE,			// CloseWindow
						TRUE,							// SetViewSelection
						FALSE);							// bJustSetFlag,

		// Force a serialization of the TOC:
		//
		Write();

		//
		// If this was a delete from a messageframe, we may need to open the next message.
		//
		if (bRet)
		{
			// Only open next window if:
			//  1. This is only happening on one message
			//  2. There is another message in the mailbox to open
			//  3. The switch enabling this feature is turned on
			//	4. The message to open is unread
			//
			DisplayNextMessageOrClose (this, SingleSum, LastSelectedPos);
		}
	}
	else
	{
		TCHAR  buf [512];
		int iType = IMAPERR_BAD_CODE;
		CString msg;

		buf[0] = '\0';

		m_pImapMailbox->GetLastImapErrorAndType (buf, 510, &iType);
	
		if (buf[0])
		{
			if (iType != IMAPERR_USER_CANCELLED && iType != IMAPERR_LOCAL_ERROR)
			{
				ErrorDialog (IDS_ERR_IMAP_CANNOT_MODIFYFLAG, Name(), buf);
			}
			else if (iType != IMAPERR_USER_CANCELLED)
			{
				ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
			}
		}

		// Set the focus back to the sumlist.
		if (View && !SingleSum)
		{
			View->m_SumListBox.SetFocus ();
		}
	}

	return hResult;
}




// ImapFlagDeleteMsgs [PRIVATE]
//
// Implements the flag-deleted method of deleting messages.
//
HRESULT CTocDoc::ImapFlagDeleteMsgs (CSummary* SingleSum, BOOL bExpunge, BOOL bSilent)
{
	// Route the call to ImapFlagMessages.
	HRESULT hResult = ImapFlagMessages (SingleSum);

	// Should we expunge also?? If caller set bExpunge to TRUE, obey
	// his wishes, otherwise use the IDS_INI_IMAP_REMOVE_ON_DELETE
	// personality setting.
	//
	if (SUCCEEDED (hResult) )
	{
		BOOL bNeedToExpunge = bExpunge;

		if (!bNeedToExpunge)
		{
			bNeedToExpunge = ExpungeAfterDelete (this);
		}

		if (bNeedToExpunge)
		{
			hResult = ImapExpunge (bSilent);
		}
	}

	return hResult;
}




//////////////////////////////////////////////////////////////////////////////
// ImapFlagMessages [PRIVATE]
//
// Flags messages as Deleted, or (if bUndelete), remove the \Deleted flag.
//////////////////////////////////////////////////////////////////////////////
HRESULT CTocDoc::ImapFlagMessages (CSummary *SingleSum /* = NULL */, BOOL bUndelete /* = FALSE */, BOOL bExpunge /* = FALSE */)
{
	POSITION	pos = NULL, NextPos = NULL;
	CString		sUidlist, sUid;
	CSumList	ImapMoveSumList;
	HRESULT		hResult = S_OK;
	BOOL		bRet = FALSE;
	int			i = 0;
	int nCount = 0;
	POSITION	LastSelectedPos = NULL;

	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return E_FAIL;
	}

	// If the mailbox is read-only, we can't continue.
	if (m_pImapMailbox->IsReadOnly())
	{
		CString buf;

		ErrorDialog( IDS_WARN_IMAP_RDONLY_MODIFYFLAG, Name());

		return E_FAIL;
	}

	// Get the view so we can find selected messages.
	CTocView *View = GetView ();

	// Disable preview.
	SetPreviewableSummary (NULL);

	// Accumulate a uid list so we can do a block IMAP command.
	//
	pos = m_Sums.GetHeadPosition();

	i = 0;
	nCount = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// If this is an undelete and it's not flagged as deleted, ignore.
		if (bUndelete && ((Sum->m_Imflags & IMFLAGS_DELETED) == 0) )
			continue;

		// Ok. This is selected. Add to list.
		if (sUidlist.IsEmpty())
			sUidlist.Format("%lu", Sum->m_Hash);
		else
		{
			sUid.Format (",%lu", Sum->m_Hash);
			sUidlist += sUid;
		}

		// Also, add to ImapMoveSumList so we can change summary flags below.
		// NOTE: Make SURE that "ImapMoveSumList" is an ordered subset of m_Sums!!!
		//
		ImapMoveSumList.AddTail (Sum);

		nCount++;

		if (SingleSum)
			break;
	}

	// Must have found some.
	if (nCount <= 0)
	{
		goto end;
	}

	//
	// Mark "NextPos" for later use. Note: Could be NULL.
	//
	LastSelectedPos = NextPos;

	// Ok, attempt to flag messages as deleted/undeleted.
	//
	if (bUndelete)
	{
		hResult = m_pImapMailbox->UnDeleteMessagesFromServer ( (LPCSTR) sUidlist, TRUE);
	}
	else
	{
		hResult = m_pImapMailbox->DeleteMessagesFromServer ( (LPCSTR) sUidlist, FALSE, TRUE);
	}

	if ( !SUCCEEDED (hResult) )
	{
		TCHAR  buf [512];
		int iType = IMAPERR_BAD_CODE;
		CString msg;

		buf[0] = '\0';

		m_pImapMailbox->GetLastImapErrorAndType (buf, 510, &iType);
	
//#if 0 // JOK (Do we need this?? 12/17/97)
		// Allow redraw of view while dialog is up.
		if (View)
		{
			View->m_SumListBox.SetRedraw(TRUE);
		}
// #endif // JOK

		if (buf[0])
		{
			if (iType != IMAPERR_USER_CANCELLED && iType != IMAPERR_LOCAL_ERROR)
			{
				ErrorDialog (IDS_ERR_IMAP_CANNOT_MODIFYFLAG, Name(), buf);
			}
			else if (iType != IMAPERR_USER_CANCELLED)
			{
				ErrorDialog (IDS_ERR_IMAP_COMMAND_FAILED);
			}
		}

		// Set the focus back to the sumlist.
		if (View && !SingleSum)
		{
			View->m_SumListBox.SetFocus ();
		}

		goto end;
	}

	// Ok. The server command succeeded. Go now and flag local summaries.
	//
	int HighlightIndex; HighlightIndex = 0;

	// If SingleSum and SingleSum has a message window, don't close the window
	// here. That will be handled below.
	//
	BOOL bAllowCloseWindow;
	bAllowCloseWindow = !(SingleSum && SingleSum->m_FrameWnd);

	bRet = ImapRemoveOrderedSubSums (&ImapMoveSumList, HighlightIndex,
					    bAllowCloseWindow,
						SingleSum == NULL,  // If SingleSum, don't set view selection
						TRUE,				// Just flag
						bUndelete);			// Delete or Undelete.

	// Must re-serialize the TOC:
	//
	Write();

end:
	// Must clean this up.
	ImapMoveSumList.RemoveAll();

#if 0 // Do we need this??
	// Redraw the complete toc.
	if (View)
	{
		View->m_SumListBox.SetRedraw(TRUE);
		View->Invalidate();
	}
#endif 

	//
	// If this was a delete from a messageframe, we may need to open the next message.
	//
	if (bRet && !bAllowCloseWindow && !bUndelete)
	{
		// Only open next window if:
		//  1. This is only happening on one message
		//  2. There is another message in the mailbox to open
		//  3. The switch enabling this feature is turned on
		//	4. The message to open is unread
		//
		DisplayNextMessageOrClose (this, SingleSum, LastSelectedPos);
	}

	return hResult;
}




// ImapSetMessageState [PUBLIC]
// Set remote message status corresponding to MS_READ, MS_UNREAD and MS_REPLIED. 
// This is called by CSummary::SetState() for an IMAP summary.
// If "bUnset", then remove the flag.
//
BOOL CTocDoc::ImapSetMessageState (CSummary *pSum, char State, BOOL bUnset /* = FALSE */)
{
	CString szUid;

	if (!pSum)
		return FALSE;

	// Turn on a wait cursor during this procedure:
	CCursor waitCursor;

	// We handle only IMAP tocs.
	if (! (pSum->m_TheToc && (pSum->m_TheToc == this)) )
		return TRUE;

	if (!m_pImapMailbox)
		return FALSE;

	// Single UID:
	szUid.Format ("%lu", pSum->GetHash());

	return ImapSetRemoteStateOfUidlist (m_pImapMailbox, szUid, State, bUnset);
}





// ImapSetStatusOfSelectedMessages [PUBLIC]
// Set remote message status corresponding to MS_READ, MS_UNREAD and MS_REPLIED. 
// Operates on selected messages only.
// Also sets the local CSummary's state bit.
//
BOOL CTocDoc::ImapSetStatusOfSelectedMessages (char State)
{
	CString szUidList;
	BOOL bResult = FALSE;

	// Must have a view.
	//
	CTocView *pView = GetView ();
	if (!pView)
		return FALSE;

	// Turn on a wait cursor during this procedure:
	CWaitCursor waitCursor;

	// Loop through the sum list.
	//
	POSITION pos = m_Sums.GetHeadPosition();
	POSITION NextPos;
	int i = 0;
	szUidList.Empty();

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* pSum = m_Sums.GetNext(NextPos);

		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( pSum && (pView->m_SumListBox.GetSel(i) > 0) )
		{
			// Set the local state bit. This assumes that we will succeed below!!
			//
			pSum->SetState(State);

			// Add UID's
			if ( szUidList.IsEmpty() )
				szUidList.Format ("%lu", pSum->GetHash());
			else
			{
				CString szUid;

				szUid.Format (",%lu", pSum->GetHash());
				szUidList += szUid;
			}
		}
	}

	if (!szUidList.IsEmpty())
		bResult = ImapSetRemoteStateOfUidlist (m_pImapMailbox, szUidList, State, FALSE);
	else
		bResult = FALSE;

	// Re-serialize the TOC:
	//
	Write();

	return bResult;
}



// ImapToggleStatusOfSelectedMessages [PUBLIC]
// Set remote message status corresponding to MS_READ, MS_UNREAD and MS_REPLIED. 
// Similar to ImapSetStatusOfSelectedMessages() above but toggles SEEN/UNSEEN.
//
BOOL CTocDoc::ImapToggleStatusOfSelectedMessages ()
{
	CString szUidToReadList;
	CString szUidToUnReadList;
	BOOL	bResult = TRUE;

	// Must have a view.
	//
	CTocView *pView = GetView ();
	if (!pView)
		return FALSE;

	// Turn on a wait cursor during this procedure:
	CWaitCursor waitCursor;

	// Loop through the sum list.
	//
	POSITION pos = m_Sums.GetHeadPosition();
	POSITION NextPos;
	int i = 0;
	szUidToReadList.Empty();
	szUidToUnReadList.Empty();

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* pSum = m_Sums.GetNext(NextPos);

		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( pSum && (pView->m_SumListBox.GetSel(i) > 0) )
		{
			// 
			if (pSum->IsComp())
			{
				// Ignore.
			}

			else if (MS_UNREAD == pSum->m_State)
			{
				// Add to the ToRead list.
				//
				pSum->SetState(MS_READ);

				if ( szUidToReadList.IsEmpty() )
					szUidToReadList.Format ("%lu", pSum->GetHash());
				else
				{
					CString szUid;

					szUid.Format (",%lu", pSum->GetHash());
					szUidToReadList += szUid;
				}
			}
			else if (MS_READ == pSum->m_State)
			{
				pSum->SetState(MS_UNREAD);

				// Add to ToUnread list.
				//
				if ( szUidToUnReadList.IsEmpty() )
					szUidToUnReadList.Format ("%lu", pSum->GetHash());
				else
				{
					CString szUid;

					szUid.Format (",%lu", pSum->GetHash());
					szUidToUnReadList += szUid;
				}
			}
		}
	}

	// Set state in 2 separate commands.
	//
	if (!szUidToReadList.IsEmpty())
		bResult = ImapSetRemoteStateOfUidlist (m_pImapMailbox, szUidToReadList, MS_READ, FALSE);

	if (bResult && !szUidToUnReadList.IsEmpty())
		bResult = ImapSetRemoteStateOfUidlist (m_pImapMailbox, szUidToUnReadList, MS_UNREAD, FALSE);

	// Re-serialize the TOC:
	//
	Write();

	return bResult;
}



// ImapMergeTmpMbx [PUBLIC]
// FUNCTION
// Mark for compaction any summaries still in the mailbox but no longer on the server. These are
// listed in "pExpungedUidList".
// Then add new messages from the temporary MBX file. 
// END FUNCION
BOOL CTocDoc::ImapMergeTmpMbx (CPtrList *pExpungedUidList) 
{
	CString			sUidlist, sUid;
	BOOL			bResult = TRUE;
	JJFile			MboxFile, TmpMboxFile;
	CTocDoc			*pTempToc = NULL;
	CTocView		*View = NULL;
	POSITION		pos, NextPos;
	POSITION		pLastMessagePos = NULL;
	CSummary		*Sum, *CopySum;
	CRString		INBOX (IDS_IMAP_RAW_INBOX_NAME);

	// Parameter sanity: pExpungedUidList can be NULL.

	// This MUST be an IMAP toc.
	if (!m_pImapMailbox)
		return FALSE;

	// Go build a TOC for the temporary MBX file.
	CString TmpMbxFilePath;
	GetTmpMbxFilePath (m_pImapMailbox->GetDirname(), TmpMbxFilePath);

	pTempToc = GetToc (TmpMbxFilePath);

	if (!pTempToc)
	{
		goto end;
	}

	if (TmpMboxFile.Open (TmpMbxFilePath, O_RDONLY) < 0)
	{	
		goto end;
	}

	if (MboxFile.Open (MBFilename(), O_RDWR | O_CREAT | O_APPEND) < 0)
	{	
		goto end;
	}

	// Make sure we seek to the end of the file.
	MboxFile.Seek(0L, SEEK_END);

	// Set redraw to FALSE so we don't get flicker
	View = GetView();
	if (View)
		View->m_SumListBox.SetRedraw(FALSE);

	// Wade through this toc and subtract any expunged messges.
	if (pExpungedUidList)
	{
		POSITION	uidPos, uidNext;
		CImapFlags  *pF;
		pos = m_Sums.GetHeadPosition();

		for (NextPos = pos; pos; pos = NextPos)
		{
			Sum = m_Sums.GetNext(NextPos);
			if (Sum)
			{
				// Is it in the expunged list.
				uidPos = pExpungedUidList->GetHeadPosition();

				// Optimization. No more uid's in pExpunged list.
				if (!uidPos)
					break;

				for( uidNext = uidPos; uidPos; uidPos = uidNext )
				{
					pF = ( CImapFlags * ) pExpungedUidList->GetNext( uidNext );
					if (pF)
					{
						if (Sum->m_Hash == pF->m_Uid)
						{
							// Seems like the only way to prevent bogus preview asserts.

							if (View && Sum == GetPreviewableSummary())
							{
								//
								// Unfortunately, a TocFrame has to keep a cached Summary pointer
								// for the message being displayed in the preview pane.  However,
								// this Xfer() method can nuke such Summary objects, leaving
								// the TocFrame with an invalid pointer.  So, before we get around
								// to nuking the Summary object below, we need to inform the frame
								// that the Summary is about to go away.
								//
								InvalidatePreviewableSummary(Sum);

								CView* pPreviewView = GetPreviewView();
								if (pPreviewView)
								{
									//
									// Check to see if preview pane has focus.  If so, then
									// just tweak the preview without opening a new
									// message window.
									//
									CWnd* pFocusWnd = CWnd::GetFocus();
									if (pPreviewView == pFocusWnd || pPreviewView->IsChild(pFocusWnd))
									{
										//
										// FORNOW, the big assumption here is that the change
										// focus event will be enough to kick off the normal
										// preview-the-summary-with-focus logic in the TocView.
										// This may change if we get away from a focus-based
										// preview scheme.
										//
										ASSERT(View);
										View->SetFocus();
									}
								}
							}

							// Remove any attachments before we delete the summary.
							//
							RemoveAttachments (this, Sum);

							// close the message window
							// (this used to be done in RemoveAttachments() -jdboyd 8/16/99
							CFrameWnd* Frame = Sum->m_FrameWnd;
							if (Frame)
							{
								if (Frame->GetActiveDocument()->CanCloseFrame(Frame))
								{
									Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
								}
							}

							// Note: this frees the memory occupied by the summary.
							RemoveSum (Sum);

							pExpungedUidList->RemoveAt (uidPos);

							// Free memory.
							delete pF;

							break;
						}
					}
				}						
			}
		}
	}

	// Find the position of the last message in the real toc before we add the new ones.
	// This is saved for use below if we need to filter.
	pos = m_Sums.GetHeadPosition();
	pLastMessagePos = pos;
	while (pos)
	{
		pLastMessagePos = pos;
		m_Sums.GetNext(pos);
	}

	// Now, append the message stubs to the real mailbox, keeping the position of the first 
	// appended mesage in case we need to filter.
	// Add messages to the end of the real MBX file.

	pos = pTempToc->m_Sums.GetHeadPosition();

#ifdef _DEBUG
	int count; count = 0;
#endif 

	// Make sure we append to the file.
	MboxFile.Seek(0L, SEEK_END);
	
	for (NextPos = pos; pos; pos = NextPos)
	{
		Sum = pTempToc->m_Sums.GetNext(NextPos);
		if (Sum)
		{
			long MStart = 0;

			MboxFile.Tell(&MStart);

			if (TmpMboxFile.JJBlockMove(Sum->m_Offset, Sum->m_Length, &MboxFile) < 0)
			{
				bResult = FALSE;
				break;
			}

			// Add a new TOC to the target mailbox.
			CopySum = new CSummary;
			CopySum->Copy(Sum);
			AddSum(CopySum);
			CopySum->SetOffset(MStart);

#ifdef _DEBUG
			count++;
#endif 

		}
	}


	// Redisplay toc.
	if (View)
		View->m_SumListBox.SetRedraw(TRUE);

end:
	// Cleanup.

	if (MboxFile.IsOpen())
		MboxFile.Close();
	if (TmpMboxFile.IsOpen())
		TmpMboxFile.Close();

	// Get the filename to the TOC file so we can delete it and the MBX file below.
	char TocName[_MAX_PATH + 1];
	strcpy (TocName, TmpMbxFilePath);
	SetFileExtensionMT (TocName, CRString (IDS_TOC_EXTENSION));

	// Delete the memory structure before deleting the file.
	pTempToc->m_bAutoDelete = TRUE;
	pTempToc->OnCloseDocument ();

	// Now, delete the TOC file.
	DeleteLocalFile (TocName);

	// Delete the tbx file.
	DeleteLocalFile (TmpMbxFilePath);
 
	return bResult;
}






// ImapFilterMessages [MODULE]
// FUNCTION
// We are filtering from an INBOX. Messages have already been added.
// END FUNCTION

// NOTES:
// THis is called from ImapMergeTmpMbx().
// END NOTES

void CTocDoc::ImapFilterMessages (CSummary *SingleSum /* = NULL */)
{
	// Just call the folder's FilterMessages method.

	// This MUST be an IMAP toc.
	if (!m_pImapMailbox)
		return;;

	m_pImapMailbox->FilterMessages (this, SingleSum);
}





//////////////////////////////////////////////////////////////////////////////
// ImapFetchMessages [PUBLIC]
//
// Loop through the selected messages a do a full download of each.
// Redownload it if it was already downloaded.
// If SingleSum,just do that.
// NOTE: If "bClearCacheOnly is TRUE, then this just removes downloaded contents 
// without fetching anything.
//
//////////////////////////////////////////////////////////////////////////////
CSummary *CTocDoc::ImapFetchMessages (CSummary *SingleSum /* = NULL */,
										 BOOL bDownloadAttachments /* = TRUE */,
										 BOOL bOnlyIfNotDownloaded /* = TRUE */,
										 BOOL bClearCacheOnly /* = FALSE */)
{
	POSITION	pos = NULL, NextPos = NULL;
	int			i = 0;
	int nCount = 0;
	BOOL		bWasInProgress = FALSE;
	BOOL		bResult = FALSE;

	// Sanity: Must have a valid IMAP object.
	if (! m_pImapMailbox )
	{
		ASSERT (0);
		return NULL;
	}

	// Get the view so we can find selected messages.
	CTocView *View = GetView ();

	// Put up a wait cursor
	CWaitCursor wc;

	// Do a count so we can put up progress..
	//
	pos = m_Sums.GetHeadPosition();

	i = 0;
	nCount = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// Already downloaded?
		//
		if (!bClearCacheOnly)
		{
			if (bOnlyIfNotDownloaded && !(Sum->m_Imflags & (IMFLAGS_NOT_DOWNLOADED|IMFLAGS_UNDOWNLOADED_ATTACHMENTS)))
			{
				continue;
			}
		}

		nCount++;

		if (SingleSum)
			break;
	}

	// Must have found some.
	if (nCount <= 0)
	{
		return NULL;
	}

	// We can put up a progress window here!
	//

	CString szMsg;

	if ( ::IsMainThreadMT() )
	{
		if (InProgress)
		{
			bWasInProgress = TRUE;
			PushProgress();
		}

		if (bClearCacheOnly)
		{
			szMsg.Format (CRString(IDS_IMAP_MSGS_LEFTTOPROCESS), nCount);
		}
		else
		{
			szMsg.Format (CRString(IDS_POP_MESSAGES_LEFT), nCount);
		}

		MainProgress(szMsg);
	}

	// Get the "current" summary so re-preview it.
	//
	CSummary *pCurSum = NULL;

	// Now loop throught each summary individually.
	//
	BOOL bModified = FALSE;
	i = 0;

	pos = m_Sums.GetHeadPosition();
	
	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i) <= 0) )
		{
			continue;
		}

		// Already downloaded?
		//
		if (!bClearCacheOnly)
		{
			if (bOnlyIfNotDownloaded && !(Sum->m_Imflags & (IMFLAGS_NOT_DOWNLOADED|IMFLAGS_UNDOWNLOADED_ATTACHMENTS)))
			{
				continue;
			}
		}

		if ( View && (Sum == GetPreviewableSummary()) )
		{
			//
			pCurSum = Sum;
		}

		// Purge from cache if already downloaded.
		//
		if (!bOnlyIfNotDownloaded || ! (Sum->m_Imflags & (IMFLAGS_NOT_DOWNLOADED|IMFLAGS_UNDOWNLOADED_ATTACHMENTS)) )
		{
			// Delete any attachments.
			//
			RemoveAttachments (this, Sum);

			// Mark the summary as not downloaded:
			//
			Sum->m_Imflags |= IMFLAGS_NOT_DOWNLOADED;

			// No downloaded attachments.
			//
			Sum->m_nUndownloadedAttachments = 0;

			// Signal that we may need compacting.
			// Must set a non-zero value for pTocDoc->m_delSpace.
			//
			m_DelSpace += Sum->m_Length;

			// Can now reset these:
			//
			Sum->SetOffset ( 0 );
			Sum->m_Length = 0;

			// These will enable the "Compact" button on the view window.
			m_NeedsCompact = TRUE;

			bModified = TRUE;
		}

		//
		// Ok. THis is one to fetch. Download all attachments as well.
		//
		if (bClearCacheOnly)
		{
			bResult = TRUE;
		}
		else
		{
			BOOL b = SUCCEEDED (m_pImapMailbox->DownloadSingleMessage (this, Sum, bDownloadAttachments));
			bResult = bResult || b;
		}

		nCount--;

		// Update progress bar.
		if ( ::IsMainThreadMT() )
		{
			if (bClearCacheOnly)
			{
				szMsg.Format (CRString(IDS_IMAP_MSGS_LEFTTOPROCESS), nCount);
			}
			else
			{
				szMsg.Format (CRString(IDS_POP_MESSAGES_LEFT), nCount);
			}

			MainProgress(szMsg);
		}

		if (SingleSum || !bResult)
			break;
	}

	// Remove progress bar.
	if ( ::IsMainThreadMT() )
	{
		if (bWasInProgress)
			PopProgress ();
		else
			CloseProgress();
	}

	// Update the "Compact" button in case we purged stuff from the MBX file.
	//
	if (View && m_NeedsCompact)
		View->SetCompact();

	// Update these.
	if (bResult || bModified)
		SetModifiedFlag();

	// Refresh preview:
	if (pCurSum)
	{
		InvalidatePreviewableSummary(pCurSum);		
	}

	// Set the focus back to the view:
	//
	if (View)
		View->SetFocus();

	return SingleSum;
}





// ImapOnServerSearch [PUBLIC]
//
// Entry point to an IMAP on-server search.
// NOTE: pUidRange is either NULL or one of the foll:
// (1). An range of UIDs of the form 234:444. First UID MUST be lower than second UID.
// (2). Comma-separated UIDs', e.g., 5, 7, 45, 1, 23. UID's not necessarily in any order.
// (3). Combination of (1) and (2)
// If NULL, then a range of "1:*" is assumed.
//
BOOL CTocDoc::ImapOnServerSearchAll (MultSearchCriteria* pMultiSearchCriteria, CSumList& slResults, unsigned long* pNmsgsMatchedUndownloaded)
{
	BOOL bResult = FALSE;

	// Initialize!
	//
	if (pNmsgsMatchedUndownloaded)
		*pNmsgsMatchedUndownloaded = 0;

	// Pass the buck to the contained IMAP interface.
	//
	if (m_pImapMailbox)
	{
		CString szResults;
		bResult = SUCCEEDED (m_pImapMailbox->DoOnServerSearch (pMultiSearchCriteria, szResults) );

		if (bResult)
		{
			// Find the summaries represented by the comma-separated uid's and put them into the
			// given CSumList. Note that the value returned in "pNmsgsMatchedUndownloaded" 
			// is the number of hits found on the server but not yet in our local cache.
			//
			CommaUidListToSumList (szResults, this, slResults, pNmsgsMatchedUndownloaded);
		}
	}

	return bResult;
}




#if 0 // JOK - Probably not very useful.

// Search only the summaries we know about.
//
BOOL CTocDoc::ImapOnServerSearchKnown (MultSearchCriteria* pMultiSearchCriteria, CSumList& slResults, unsigned long *pnMsgsUnknown)
{
	BOOL bResult = FALSE;

	// Initialize!
	//
	if (pnMsgsUnknown)
		*pnMsgsUnknown = 0;

	// Pass the buck to the contained IMAP interface.
	//
	if (m_pImapMailbox)
	{
		unsigned long ulMaxUid = ImapGetMaxSummaryUid ();

		if (ulMaxUid > 0)
		{
			CString szResults;

			bResult = SUCCEEDED (m_pImapMailbox->DoOnServerSearchKnown (pMultiSearchCriteria, szResults) );

			if (bResult)
			{
				// Return number of messages on the server that were not searched.
				//
				if ( pnMsgsUnknown && ((long)m_pImapMailbox->GetNumberOfMessages() > m_Sums.GetCount()) )
				{
					*pnMsgsUnknown = m_pImapMailbox->GetNumberOfMessages() - m_Sums.GetCount();
				}

				// Find the summaries represented by the comma-separated uid's and put them into the
				// given CSumList
				//.
				CommaUidListToSumList (szResults, this, slResults, NULL);
			}
		}
	}

	return bResult;
}

#endif // 



// ImapGetMinSummaryUid [PUBLIC]
//
// Loop through summaries to find the minimum UID there:
// NOTE: The returned value MUST be greater than zero. A value of zero
// means that there are no summaries in the TOC.
//
unsigned long CTocDoc::ImapGetMinSummaryUid ()
{
	unsigned long ulMinUid = (unsigned long)-1;

	POSITION pos = m_Sums.GetHeadPosition();
	
	while (pos)
	{
		CSummary *pSum = (CSummary *)m_Sums.GetNext (pos);

		if (pSum && pSum->GetHash() < ulMinUid)
			ulMinUid = pSum->GetHash();
	}

	// If no summaries, return 0!!
	if ( ulMinUid == (unsigned long)-1 )
		ulMinUid = 0;

	return ulMinUid;
}



// ImapGetMaxSummaryUid [PUBLIC]
//
// Loop through summaries to find the maximum UID there:
// NOTE: The returned value MUST be greater than zero. A value of zero
// means that there are no summaries in the TOC.
//
unsigned long CTocDoc::ImapGetMaxSummaryUid ()
{
	unsigned long ulMaxUid = 0;

	POSITION pos = m_Sums.GetHeadPosition();
	
	while (pos)
	{
		CSummary *pSum = (CSummary *)m_Sums.GetNext (pos);

		if (pSum && pSum->GetHash() > ulMaxUid)
			ulMaxUid = pSum->GetHash();
	}

	return ulMaxUid;
}



// ImapGetNumUnfetchedMsgs [PUBLIC]
//
// Wade through the summary list looking for messages that we've only got summaries for.
//
unsigned long CTocDoc::ImapNumberOfUnfetchedMsgs ()
{
	if ( !IsImapToc() )
		return 0;

	unsigned long ulNmsgs = 0;

	POSITION pos = m_Sums.GetHeadPosition();
	CSummary* pSum;

	while(pos)
	{
		pSum = m_Sums.GetNext (pos);

		if( pSum && IsNotDownloaded(pSum) )
		{
			ulNmsgs++;
		}
	}

	return ulNmsgs;

}




// ImapNumberOfUnknownMsgs [PUBLIC]
//
// Return the difference between te number of messages on the server and the number
// that we have summaries for.
//
unsigned long CTocDoc::ImapNumberOfUnknownMsgs ()
{
	if ( !IsImapToc() )
		return 0;

	unsigned long nMsgsUnknown = 0;

	if (m_pImapMailbox)
	{
		// jdboyd June 14, 99 - we're leaving the mailbox open after the ping.
		// this causes the connection to the mailbox to be left open during a search.

		BOOL bMustCloseConnection = !m_pImapMailbox->IsSelected();	

		m_pImapMailbox->Ping();

		if (bMustCloseConnection)
			m_pImapMailbox->Close();

		unsigned long ulMsgs = m_pImapMailbox->GetNumberOfMessages();

		if ( (long)ulMsgs > m_Sums.GetCount() )
		{
			nMsgsUnknown = ulMsgs - m_Sums.GetCount();
		}
	}

	return nMsgsUnknown;
}



// ImapServerCanSearch [PUBLIC]
//
// STATIC
BOOL CTocDoc::ImapServerCanSearch (SearchCriteria *pCurSearchCriteria)
{
	return CImapMailbox::ImapServerCanSearch (pCurSearchCriteria);
}




// ========================== Internal non-class function ====================/
// Find summary 
void CommaUidListToSumList (LPCSTR pUids, CTocDoc* pTocDoc, CSumList& SumList, unsigned long* pnSumsNotFound)
{
	if (! (pUids && pTocDoc) )
		return;

	if (pnSumsNotFound)
		*pnSumsNotFound = 0;

	// Add uid's to the sumlist.
	//
	LPCSTR p = pUids;
	LPCSTR q;
	unsigned long uid;
	CString str;
	CSummary* pSum;

	while (p)
	{
		q = strchr (p, ',');

		if (q && q > p)
		{
			str = CString (p, q-p);
			p = ++q;
		}
		else
		{
			str = CString (p);
			p = NULL;
		}

		uid = atol (str);

		if ( uid > 0)
		{
			if ( (pSum = pTocDoc->m_Sums.GetByUid(uid)) != NULL )
			{
				SumList.AddTail (pSum);
			}
			else if (pnSumsNotFound)
			{
				// Count uid's not in list.
				(*pnSumsNotFound)++;
			}
		}
	}
}




// FUNCTION
// Rewrite an MBX stub from the source mailbox to a destination mailbox, performing the following
// in the process:
// 1. If the passed-in NewUid is non-zero, replace X-UID: field with it, else eliminate it.
// 2. If pSourceToc is an IMAP mailbox, download attachments.
// END FUNCTION

// NOTES
// MStart: Output parameter to return the start of new message in the target MBX file.
// NewLength: Output parm to return the length of the new message stub in target MBX file.
// Note: Don't delete source file!!
// END NOTES
BOOL RewriteMbxStub (CSummary *pSum, CTocDoc *pSourceToc, CTocDoc *pTargetToc, unsigned long NewUid,
					long& MStart, long& NewLength)
{
	char	ACStr[64];
	int		ACLen;
	char	XUIDStr[64];
	int		XUIDLen;
	CString SourceAttachDir;
	CString TargetMailboxDir;
	BOOL	bResult = FALSE;
	long bytesWritten = 0;
	long len;

	// Sanity
	if (! (pSum && pSourceToc && pTargetToc))
		return FALSE;

	// If the source message is from an IMAP toc and not yet downloaded, go do that now.
	if ( pSum->m_TheToc->IsImapToc () )
	{
		if ( !SUCCEEDED (DownloadImapMessage (pSum)) )
			return FALSE;
	}

	// "Attachment converted:" string.
	GetIniString(IDS_ATTACH_CONVERTED, ACStr, sizeof(ACStr));
	ACLen = strlen(ACStr);
	
	/// Also X-UID header.
	strcpy (XUIDStr, CRString (IDS_IMAP_UID_XHEADER));
	XUIDLen = strlen (XUIDStr);

	// Locate the parent of the attachment directories.
	if (pSourceToc->IsImapToc())
	{
		// MUST be:
		if (pSourceToc->m_pImapMailbox)
		{
			GetAttachDirPath (pSourceToc->m_pImapMailbox->GetDirname(), SourceAttachDir);
		}
		else
			return FALSE; // !!!
	}
	else
	{
		SourceAttachDir = EudoraDir + CRString(IDS_ATTACH_FOLDER);
	}

	// Locate attachment directories.
	if (pTargetToc->IsImapToc())
	{
		// MUST be.
		if (pTargetToc->m_pImapMailbox)
		{
			TargetMailboxDir = pTargetToc->m_pImapMailbox->GetDirname();
		}
		else
			return FALSE; 
	}
	else
	{
		TargetMailboxDir = EudoraDir;
	}

	// OK. Open the target file.
	JJFile TargetMBoxFile;
	if ( FAILED (TargetMBoxFile.Open (pTargetToc->MBFilename(), O_RDWR | O_CREAT | O_APPEND) ) )
	{
		return FALSE;
	}


	// Make sure we seek to the end of the file.
	TargetMBoxFile.Seek(0L, SEEK_END);

	// This is the original mailbox size. We may have to delete our additions.
	// NOTE This is an output parameter.
	TargetMBoxFile.Tell(&MStart);

	// Length of new stub in target mbox. Output parameter!!
	NewLength = 0;

	// Go get the message stub. NOTE: FindMessageDoc() will get the existing doc
	// if it exists but GetMessageDoc() will also create one if it doesn't.
	// We need a doc in any case but if we opened one. we must also close it.
	CMessageDoc* MsgDoc = NULL;
	BOOL		 MustCloseMsgDoc = FALSE;	

	// Try FindMesageDoc() first;
	MsgDoc = pSum->FindMessageDoc();
	if (!MsgDoc)
	{
		MsgDoc = pSum->GetMessageDoc();
		MustCloseMsgDoc = TRUE;
	}

	// This is NOT a loop. Do this so we can break out easily.
	for (int iLoop = 0; (iLoop < 1) && MsgDoc; iLoop++)
	{
		// p is the beginning of text we should write to target mailbox.
		char *p = MsgDoc->GetText();

		// Must have text.
		if (NULL == p)
		{
			bResult = FALSE;
			break;
		}

		// If the message text begins with a message separator, i.e., 
		// "From ???@...", don't write it.
		CRString szFrom (IDS_FROM_FORMAT);
		if ( strnicmp (p, szFrom, 12) == 0)
		{
			// Go to next line.
			p = strchr(p, '\n');
			if (p)
				p++;
		}

		// Still OK??
		if (NULL == p)
		{
			bResult = FALSE;
			break;
		}

		// Add a message separator separator line to the MBX file.
		bytesWritten = WriteMessageSeparator (&TargetMBoxFile);

		// Did we write anything??
		if ( bytesWritten <= 0)
		{
			bResult = FALSE;
			break;
		}

		// Ok. We wrote..
		NewLength += bytesWritten;
		
		bResult = TRUE;		// Changed to FALSE if serious error.

		// Don't look for X-UID header if we're in the message's body or we've already found it.
		BOOL bLookForXuidHeader = TRUE;

		// Note: We start a new iteration on a new line.
		for (char* t = p; t && *t; )
		{
			// If we'we still looking for the X-UID header field, look for header/body
			// separator blank line.
			if (bLookForXuidHeader)
			{
				// Look for the blank line separating header from body.
				if (*t == '\r')
				{
					t++;
					// Could we have "\r\r"? It should be "\r\n".
					if (*t == '\r' || *t == '\n')
					{
						bLookForXuidHeader = FALSE;
						t++;
					}
				}
				else if (*t == '\n')
				{
					// Probably meant we had "\n\n"
					bLookForXuidHeader = FALSE;
					t++;
				}
			}

			// Look for X-UID header
			if (bLookForXuidHeader && !strncmp(XUIDStr, t, XUIDLen) )
			{
				// Found it.
				bLookForXuidHeader = FALSE;
				
				// Copy previous chunk.
				len = (long) (t - p);

				if (len > 0)
				{
					if ( FAILED (TargetMBoxFile.Put (p, len) ) )
					{
						bResult = FALSE;
						break;  // Out of loop.
					}

					NewLength += len;
				}

				// Advance where we'll start printing next time. Skip this line.
				p = strchr (t, '\n');
				if (p)
					p++;

				// Write new only if NewUid is non-zero.
				if (NewUid)
				{
					CString szLine;
					szLine.Format ("%s:%lu\r\n", XUIDStr, NewUid);

					if ( FAILED (TargetMBoxFile.Put (szLine, szLine.GetLength()) ) )
					{
						bResult = FALSE;
						break;
					}
				}
			}
			else if ( !strncmp(ACStr, t, ACLen) )
			{
				CString	szAttachmentPathname;
				short	tNext;

				// Copy previous chunk. But, delete last cariage/linefeed to avoid successive
				// insertion of new lines.
				len = (long) (t - p);

				if (len > 0)
				{
					if ( FAILED (TargetMBoxFile.Put (p, len) ) )
					{
						bResult = FALSE;
						break;  // Out of loop.
					}

					NewLength += len;
				}

				// we may stil fail:
				p = t;

				// look in the user specified attach directory as well. -jdboyd
				char AttachDir[_MAX_PATH + 1];
				
				GetIniString(IDS_INI_AUTO_RECEIVE_DIR, AttachDir, sizeof(AttachDir));

				// get the full path name to the attachment
				tNext = StripAttachmentName( t + ACLen, szAttachmentPathname, *AttachDir?AttachDir:SourceAttachDir );

				if( tNext )
				{
					CString NewPathname;

					// If the body part was not downloaded, do that now.
					if (pSourceToc->m_pImapMailbox)
					{
						pSourceToc->m_pImapMailbox->DownloadAttachment (szAttachmentPathname);
					}

					// Copy. Note: "ImapCopyAttachmentFile()" copies the attachment to a file in the 
					// "Attach" subdirectory of "TargetMailboxDir".
					//if (ImapCopyAttachmentFile (SourceAttachDir, TargetMailboxDir, szAttachmentPathname, NewPathname))

					// The downloaded IMAP attahcment is in the user specified attachment directory -jdboyd
					if (ImapCopyAttachmentFile (*AttachDir?AttachDir:SourceAttachDir, TargetMailboxDir, szAttachmentPathname, NewPathname))
					{
						// Write a new attachment converted line to target mailbox.
						CString buf = CRString(IDS_ATTACH_CONVERTED) + NewPathname;

						if ( FAILED (TargetMBoxFile.Put (buf, buf.GetLength()) ) )
						{
							bResult = FALSE;
							break;
						}

						NewLength += buf.GetLength();
						
						// set the pointer to the first character after this attachment 
						t += tNext + ACLen;

						// Reset where we have to write from next time.
						p = t;
					}
				}
			}

			// Go to next line.
			t = strchr(t, '\n');
			if (t)
				t++;
		}

		// If we have left over chars, write them.
		if (p && bResult)
		{
			// Delete last cariage/linefeed to avoid successive
			// insertion of new lines.
			len = (long) strlen ( p );

			if (len > 0)
			{
				if ( FAILED ( TargetMBoxFile.Put ( p, len ) ) )
					bResult = FALSE;
				else
				{
					NewLength += len;
				}
			}
		}

		// If we opened the MsgDoc, we must close it.
		if (MustCloseMsgDoc)
		{
			pSum->NukeMessageDocIfUnused();
			MsgDoc = NULL;

		}
	}

	//  If we succeeded, write a terminating blank line.
	if (bResult)
	{
		bResult = SUCCEEDED (TargetMBoxFile.PutLine ());
		if (bResult)
			NewLength += 2;
	}

	//  And get the new file offset:
	if (bResult)
	{
		long MEnd = MStart;

		TargetMBoxFile.Tell(&MEnd);

		if (MEnd <= MStart)
			bResult = FALSE;
		else
		{
			// Set length of new message.
			NewLength = MEnd - MStart;
		}
	}

	// If we failed, reset the values of MStart and NewLength.
	if (!bResult)
	{
		// Remove the stuff we added.	
		TargetMBoxFile.ChangeSize (MStart);

		MStart = 0;
		NewLength = 0;
	}

	// Cleanup.
	TargetMBoxFile.Flush();
	TargetMBoxFile.Close();

	return bResult;
}




// Loop through the message "stub" and delete all attachments and embedded objects.
// Note: This works for BOTH POP and IMAP mailboxes!!!
//
//

BOOL RemoveAttachments (CTocDoc *pTocDoc, CSummary *pSum)
{
	CMessageDoc* pMsgDoc = NULL;

	// Sanity:
	if ( !(pTocDoc && pSum) )
		return FALSE;

	// If this is an IMAP mailbox, and the message is not downloaded, then nothing to do.
	//
	if (pTocDoc->IsImapToc() && pSum->m_Imflags & IMFLAGS_NOT_DOWNLOADED)
	{
		return TRUE;
	}

	// If there's are attachment or embedded objects, we must remove them.
	//
	char	ACStr[64];
	int		ACLen;
	CString AttachDir;

	// "Attachment converted:" string.
	GetIniString(IDS_ATTACH_CONVERTED, ACStr, sizeof(ACStr));
	ACLen = strlen(ACStr);

	// Locate the parent of the attachment directories.
	if (pTocDoc->IsImapToc())
	{
		// MUST be:
		if (pTocDoc->m_pImapMailbox)
		{
			GetAttachDirPath (pTocDoc->m_pImapMailbox->GetDirname(), AttachDir);
		}
		else
			return FALSE; // !!!
	}
	else
	{
		AttachDir = EudoraDir + CRString(IDS_ATTACH_FOLDER);
	}

	// If we didn't get a msgdoc, go read the message:
	//
	pMsgDoc = pSum->FindMessageDoc();
	BOOL bWeLoadedMsgDoc = FALSE;

	if (!pMsgDoc)
	{
		pMsgDoc = pSum->GetMessageDoc();
		bWeLoadedMsgDoc = TRUE;
	}

	if (pMsgDoc)
	{
		// If has attachment.
		//
		if (pSum->HasAttachment())
		{
			for (char* t = pMsgDoc->GetText(); t; t = strchr(t, '\n'))
			{
				t++;

				if ( !strncmp(ACStr, t, ACLen) )
				{
					CString	szAttachmentPathname;
					short	tNext;
							
					// get the full path name to the attachment
					tNext = StripAttachmentName( t + ACLen, szAttachmentPathname, AttachDir );

					if( tNext )
					{
						// delete the current attachment
						::FileRemoveMT( szAttachmentPathname );
					}
							
					// set the pointer to the first character after this attachment 
					t += tNext + ACLen;
				}
			}
		}

		// get rid of embedded objects
		char* buf = pMsgDoc->GetFullMessage();		// gots ta get da URIMap
		CString Paths;
		pMsgDoc->m_QCMessage.GetEmbeddedObjectPaths( Paths );
		DeleteEmbeddedObjects( Paths );
		delete [] buf;

		// Delete the msgdoc if we loaded it.
		//
		if (bWeLoadedMsgDoc)
		{
			pSum->NukeMessageDocIfUnused();
			pMsgDoc = NULL;

		}
	}

	return TRUE;
}





// WriteMessageSeparator
// FUNCTION
// Write a From ??? etc. line at the current file pointer.
// Return the number of bytes written
// END FUNCTION
long WriteMessageSeparator (JJFile *pMbxFile)
{
	time_t counter;
	struct tm* Time;
	char buf[2048];

	// Sanity
	if (!pMbxFile)
		return 0;

	// If file is not open, some error ocurred.
	if (S_OK != pMbxFile->IsOpen())
		return 0;

    // Put envelope to file
	time(&counter);
	if (counter < 0)
		counter = 1;

	Time = localtime(&counter);

	CRString Weekdays(IDS_WEEKDAYS);
	CRString Months(IDS_MONTHS);
	wsprintf(buf, CRString(IDS_FROM_FORMAT) + "\r\n", ((const char*)Weekdays) + Time->tm_wday * 3,
			((const char*)Months) + Time->tm_mon * 3, Time->tm_mday, Time->tm_hour,
			Time->tm_min, Time->tm_sec, Time->tm_year + 1900);

	if ( SUCCEEDED( pMbxFile->Put(buf, strlen(buf) ) ) )
		return strlen (buf);
	else 
		return 0;
}



//
// If some summaries have been removed, we'd want to do this.
//
void  ClearPreviewCache (CTocDoc *pTocDoc)
{
	if (!pTocDoc)
		return;
	
	CTocView *View = pTocDoc->GetView();

	if (View)
	{
		//
		// Unfortunately, a TocFrame has to keep a cached Summary pointer
		// for the message being displayed in the preview pane.  However,
		// this Xfer() method can nuke such Summary objects, leaving
		// the TocFrame with an invalid pointer.  So, before we get around
		// to nuking any Summary objects below, we need to inform the frame
		// that Summaries are about to go away.
		//
		CFrameWnd* pParentFrame = View->GetParentFrame();
		if (pParentFrame)
		{
			extern UINT umsgPurgeSummaryCache;
			pParentFrame->SendMessage(umsgPurgeSummaryCache);
		}
	}
}



///////////////////////////////////////////////////////////////////////////
// DisplayNextMessageOrClose
// 
// Implement logic to display next message or close the message frame 
// in cases where a delete or a tow truck transfer was performed.
///////////////////////////////////////////////////////////////////////////
void DisplayNextMessageOrClose (CTocDoc *pTocDoc, CSummary* SingleSum, POSITION LastSelectedPos)
{
	if (!pTocDoc)
		return;

	CView *pView = pTocDoc->GetView();

	//close the connection to the server incase we dont need it anymore
	if(!pView && pTocDoc->IsImapToc() && pTocDoc->m_pImapMailbox)
			pTocDoc->m_pImapMailbox->Close();
		
	// Do the test.
	if (SingleSum && SingleSum->m_FrameWnd)
	{
		// If the flag's not set, just close it.
		if (!pView || !GetIniShort(IDS_INI_EASY_OPEN))
		{
			// Just close the frame.
			SingleSum->m_FrameWnd->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);

//			SingleSum->m_FrameWnd->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
		}
		else
			{
			//
			// Is there a next message that's unread?
			//
			CSummary* pNextSum = NULL;

			if (LastSelectedPos)
			{
				pNextSum = pTocDoc->m_Sums.GetAt(LastSelectedPos);
			}

			if (pNextSum)
			{
				pTocDoc->DoSwitchMessage(ID_NEXT_MESSAGE, LastSelectedPos);
			}
			else
			{
				// Just close the frame.
				SingleSum->m_FrameWnd->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);	

//				SingleSum->m_FrameWnd->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);	

			}
		}
	}
}





// Copy the contents of summary from one local mailboxc to another and add the summary to 
// the target toc.
//
BOOL MoveContents ( CTocDoc* pSrcToc, CTocDoc* pTargetToc, CSummary *pSum)
{
	JJFile TargetMBox;
	JJFile SrcMBox;

	if (! (pSrcToc && pTargetToc && pSum) )
		return FALSE;

	//
	if (pSrcToc == pTargetToc)
		return TRUE;

	if (FAILED(TargetMBox.Open(pTargetToc->MBFilename(), O_RDWR | O_CREAT | O_APPEND)) ||
		FAILED(SrcMBox.Open(pSrcToc->MBFilename(), O_RDONLY)))
	{
		return FALSE;
	}

	long lMStart = 0;
	TargetMBox.Tell(&lMStart);

	ASSERT(lMStart >= 0);

	if (FAILED(SrcMBox.JJBlockMove(pSum->m_Offset, pSum->m_Length, &TargetMBox)))
	{
		TargetMBox.ChangeSize (lMStart);

		return FALSE;
	}

	pSum->SetOffset(lMStart);

	return TRUE;
}






//========================================================================//


// IMAP utility functions:
//
// ImapSumIsDeleted 
//
// Return TRUE if the given summary is from an IMAP TOC and 
// it is flagged as deleted.
////////////////////////////////////////
BOOL ImapSumIsDeleted (CSummary *pSum)
{
	if (!pSum)
		return FALSE;

	if ( pSum->m_TheToc && pSum->m_TheToc->IsImapToc() )
	{
		if (pSum->m_Imflags & IMFLAGS_DELETED)
			return TRUE;
	}

	return FALSE;
}



///////////////////////////////////////////////////////////////////
// ImapTocHasDeletedMsgs
//
// Return TRUE if at least 1 summary has the \DELETED flag set.
///////////////////////////////////////////////////////////////////
BOOL ImapTocHasDeletedMsgs (CTocDoc *pTocDoc)
{
	// Sanity:
	if (! (pTocDoc && pTocDoc->IsImapToc() ) )
		return FALSE;

	POSITION pos, NextPos;

	pos = pTocDoc->m_Sums.GetHeadPosition();

	for (NextPos = pos; pos; pos = NextPos)
	{
		CSummary* Sum = pTocDoc->m_Sums.GetNext(NextPos);

		if (Sum && (Sum->m_Imflags & IMFLAGS_DELETED))
		{
			// If at least 1 message has this flag.
			return TRUE;
		}
	}

	// If we get here.
	return FALSE;
}



// Internal utility function that signals a CImapMailbox object to
// set the state of remote messages.
//
BOOL ImapSetRemoteStateOfUidlist (CImapMailbox *pImapMailbox, CString& szUidList, char State, BOOL bUnset)
{
	// Sanity:
	if (! pImapMailbox )
		return FALSE;

	// If we're offline, don't initiate a connection.
	//
	if ( !pImapMailbox->IsSelected() && GetIniShort(IDS_INI_CONNECT_OFFLINE) )
	{
		return FALSE;
	}

	// Set the state. Note: Do it silently.
	if (State == MS_READ)
	{
		if (bUnset)
			return SUCCEEDED (pImapMailbox->ImapUnsetSeenFlag (szUidList, TRUE));
		else
			return SUCCEEDED (pImapMailbox->ImapSetSeenFlag (szUidList, TRUE));
	}
	else if (State == MS_UNREAD)
	{
		if (bUnset)
			return SUCCEEDED (pImapMailbox->ImapSetSeenFlag (szUidList, TRUE));
		else
			return SUCCEEDED (pImapMailbox->ImapUnsetSeenFlag (szUidList, TRUE));
	}
	else if (State == MS_REPLIED)
	{
		if (bUnset)
			return SUCCEEDED (pImapMailbox->ImapUnsetAnsweredFlag (szUidList, TRUE));
		else
			return SUCCEEDED (pImapMailbox->ImapSetAnsweredFlag (szUidList, TRUE));
	}

	// If we get here, it's not a state IMAP can set. That's OK.
	return TRUE;
}
	

#endif // IMAP4

