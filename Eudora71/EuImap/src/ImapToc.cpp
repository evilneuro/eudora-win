// imaptoc.cpp - IMAP-specific methods added to TocDoc:
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

// NOTE: All this is IMAP specific.

#include "stdafx.h"

// Local MACROS.

#ifdef IMAP4 // Only for IMAP.

#include "ImapTypes.h"
#include "ImapSum.h"
#include "ImapMailbox.h"
#include "ImapAccountMgr.h"
#include "ImapAccount.h"
#include "ImapActionQueue.h"

#include "eudora.h"
#include "resource.h"	// for filtersd.h
#include "doc.h"		// for filtersd.h
#include "tocdoc.h"		// for filtersd.h
#include "filtersd.h"
#include "tocview.h"
#include "JunkMail.h"
#include "trnslate.h"  
#include "msgdoc.h"
#include "msgframe.h"
#include "compmsgd.h"
#include "TocFrame.h"
#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "cursor.h"
#include "progress.h"
#include "guiutils.h"
#include "fileutil.h"
#include "SearchManager.h"

#include "DebugNewHelpers.h"


// ========= GLobal externals =============/
extern QCMailboxDirector		g_theMailboxDirector;

BOOL CanCloseSummaryFrame(CSummary *pSum);
void CommaUidListToSumList (LPCSTR pUids, CTocDoc* pTocDoc, CSumList& SumList, unsigned long* pnSumsNotFound);
BOOL RewriteMbxStub (CSummary *pSum, CTocDoc *pSourceToc, CTocDoc *pTargetToc, unsigned long NewUid,
					long& MStart, long& NewLength);
static long WriteMessageSeparator (JJFile *pMbxFile);
void DisplayNextMessageOrClose (CTocDoc *pTocDoc, CSummary* SingleSum, POSITION LastSelectedPos);
BOOL ImapSetRemoteStateOfUidlist (CImapMailbox *pImapMailbox, CString& szUidList, char State, BOOL bUnset);


// ========= Misc helper function =============/

//
//	MarkAllAsRead()
//
//	Marks all messages in sumlist as read, assuming they were marked as unread before.
//	This is a miscellaneous helper function to include code that is needed a number of
//	places in this module.
//
void MarkAllAsRead(CSumList &sumlist)
{
	POSITION	 pos = sumlist.GetHeadPosition();
	POSITION	 posNext = NULL;
	CSummary	*pSum = NULL;
	for (posNext = pos; pos; pos = posNext)
	{
		pSum = sumlist.GetNext(posNext);
		if (pSum && (pSum->GetState() == MS_UNREAD))
		{
			pSum->SetState(MS_READ);
		}
	}
}


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
	if ( ImapIsTmpMbx (GetMBFileName()) )
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

	m_pImapMailbox = DEBUG_NEW_MFCOBJ_NOTHROW CImapMailbox ((ACCOUNT_ID)AccountID);
	if (m_pImapMailbox)
	{
		CString dir;

		MbxFilePathToMailboxDir (GetMBFileName(), dir);
		m_pImapMailbox->SetDirname (dir);

		// Make sure the MBX file exists. Note: CreateLocalFile(Path, FALSE) returns
		// TRUE if path exists. It doesn't delete it's contents.
		bResult	= CreateLocalFile (GetMBFileName(), FALSE);

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
		m_pImapMailbox->SetTocDoc(this);

	return bResult;
}

//
//	CTocDoc::ImapXfer()
//
//	Handle transfers where at least one of the mailboxes is an IMAP mailbox.  This particular function
//	does some preliminary checking then calls ImapXferMessages() to continue the process.  After that
//	this function updates the views.
//
//	Note that with the new offline/cache/online model by the time we get here the transfer has already
//	been done locally in CTocDoc::Xfer().
//
//	Parameters:
//		pTargetToc [in] - Destination doc.
//		pSumList [in] - Array of summaries to transfer.
//		bCloseWindow [in] - TRUE if we should close the document's window when we are done.
//		bCopy [in] - TRUE if we are copying not transferring.
//		bRedisplayTOCs [in] - TRUE if we should redisplay the TOC views.
//
//	Return: Returns the CopySum returned by the function that does the actual transfer.
//
CSummary* CTocDoc::ImapXfer(CTocDoc* pTargetToc,
							CSumList *pSumList,
							BOOL bCloseWindow /*=TRUE*/,
							BOOL bCopy /*=FALSE*/,
							BOOL bRedisplayTOCs /*=TRUE*/)
{
	static BOOL	 bXferActive = FALSE;			// reentrancy semaphore
	CSummary	*pCopySum = NULL;
	int			 iHighlightIndex = -1;	// Passed to (and set by) other functions, then ignored.  Use it or lose it. -dwiggins
	CTocView	*pView = NULL;
	CTocView	*pTargetView = NULL;

	// Bail if a transfer is already happening.
	if (bXferActive)
	{
		ASSERT(0);
		return NULL;
	}

	// Bail if we don't have a destination or a list of summaries to transfer.
	if (!pTargetToc || !pSumList)
	{
		ASSERT(0);
		return NULL;
	}

	// Set the reentrancy flag.
	bXferActive = TRUE;

	// Set a wait cursor.
	CWaitCursor waitCursor;

	// Call ImapXferMessages to do the real work. 
	pCopySum = ImapXferMessages(pTargetToc, pSumList, iHighlightIndex, bCloseWindow, bCopy);

	// If user wants to expunge from the source toc, go do so now.
	if (!bCopy && IsImapToc() && ExpungeAfterDelete(this))
	{
		ImapExpunge();

		// CopySum may no longer be valid.
		pCopySum = NULL;
	}

	// If something changed (we did a transfer, not a copy) write the TOC.
	if (!bCopy)
	{
		Write();
	}

	// Redisplay.
	pView = GetView();
	if (pView)
	{
		// Set redraw back to TRUE
		pView->m_SumListBox.SetRedraw(TRUE);

		// Tends to lose focus!
		pView->SetFocus();
	}

	// Make sure redraw is reset for the destination mailbox.
	if (bRedisplayTOCs)
	{
		pTargetView = pTargetToc->GetView();
		if (pTargetView)
		{
			pTargetView->m_SumListBox.SetRedraw(TRUE);
			pTargetView->m_SumListBox.UpdateWindow();
		}
	}

	// We just did a COPY so see if our UIDPLUS status has changed.
	if (m_pImapMailbox)
	{
		CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
		if (pAccount)
		{
			pAccount->WarnIfUIDPLUSStatusChanged();
		}
	}

	// Reset reentrancy flag.
	ASSERT(bXferActive);
	bXferActive = FALSE;

	return pCopySum;
}

//
//	CTocDoc::ImapXferMessages()
//
//	Does some preliminary work common to all transfers involving IMAP then calls the appropriate function
//	to perform the actual type of transfer (IMAP->IMAP (same server), IMAP->IMAP (different servers),
//	IMAP->local or local->IMAP).  The preliminary stuff consists of making sure the source and destination
//	mailboxes are properly set up and checking the online status (which is tied to mailbox status).  After
//	the transfer is done this function cleans up the network connections in the case where Eudora is offline
//	but this one action was allowed online.
//
//	Parameters:
//		pTargetToc [in] - Destination doc.
//		pSumList [in] - Array of summaries to transfer.
//		HighlightIndex [out] - Index of item to be selected (set by the actual transfer function).
//		bCloseWindow [in] - TRUE if we should close the document's window when we are done.
//		Copy [in] - TRUE if we are copying not transferring.
//
//	Return: Returns the CopySum returned by the function that does the actual transfer.
//
CSummary* CTocDoc::ImapXferMessages(CTocDoc *pTargetToc,
									CSumList *pSumList,
									int &iHighlightIndex,
                                    BOOL bCloseWindow /*= TRUE*/,
									BOOL bCopy /*= FALSE*/)
{
	BOOL		 bFromImap = FALSE;
	BOOL		 bToImap = FALSE;
	CSummary	*pCopySum = NULL;

	// Initialize in case we fail.
	iHighlightIndex = -1;

	// Make sure destination TOC is specified and has a CImapMailbox.
	if (!pTargetToc)
	{
		ASSERT(0);
		return NULL;
	}

	// We can't copy/move to the same mailbox!
	if (pTargetToc == this)
	{
		ASSERT(0);
		return NULL;
	}

	// Make sure one of us is IMAP.
	bFromImap = IsImapToc();
	bToImap   = pTargetToc->IsImapToc();

	if (!(bFromImap || bToImap))
	{
		ASSERT (0);
		return NULL;
	}

	// Make sure the CImapMailbox object is instantiated in this TOC (if IMAP).
	if (bFromImap && !m_pImapMailbox)
	{
		return NULL;
	}

	// If we are moving from an IMAP mailbox that is read-only warn the user and proceed only if
	// user agrees to make it a copy rather than a move.
	if (bFromImap && !bCopy && m_pImapMailbox->IsReadOnly())
	{
		CString buf;
		buf.Format(CRString (IDS_WARN_IMAP_RDONLY_MOVEFROM), Name());
		MessageBeep(MB_ICONEXCLAMATION);
		if (::AfxMessageBox(buf, MB_ICONQUESTION|MB_YESNO) == IDYES)
		{
			bCopy = TRUE;
		}
	}

	// If the target mailbox is read-only, warn the user and bail.
	if (bToImap && pTargetToc->m_pImapMailbox && pTargetToc->m_pImapMailbox->IsReadOnly())
	{
		CString buf;
		ErrorDialog( IDS_WARN_IMAP_RDONLY_MOVETO, pTargetToc->Name());
		return NULL;
	}

	// Disable events while we're doing this.
	// Note: the constructor for CDisableToc disables the TOC view window and the destructor enables it.
	// Creating this local variable guarantees the window will be enabled when this function terminates,
	// no matter what path leads to termination. -dwiggins
	CDisableToc disAble (this);

	// If From Imap, all server messages go through our m_pImapMailbox. 
	// Open connection to IMAP if it is not open.
	BOOL bConnected					= FALSE;
	BOOL bMustCloseConnection		= FALSE;

	// If we're offline, ask what to do:
	if ( GetIniShort(IDS_INI_CONNECT_OFFLINE) &&
		 ( (m_pImapMailbox && !m_pImapMailbox->IsSelected() ) || 
		   ( pTargetToc && pTargetToc->m_pImapMailbox && !pTargetToc->m_pImapMailbox->IsSelected() ) ) )
	{
		int		 iConnectionState = GetConnectionState(bDoAllowOffline);
		if (iConnectionState == iStateGoOnlineForThis)
		{
			// The connection is being allowed for this action only: note that we must close the connection
			// when we are done.
			bMustCloseConnection = TRUE;
		}
		else if (iConnectionState == iStateStayOfflineDisallow)
		{
			// No connection is being created and this action cannot be done offline: bail out.
			return NULL;
		}
	}

	// Open the source and destination mailboxes (if they are IMAP).
	if (bFromImap)
	{
		if (!m_pImapMailbox->IsSelected())
		{
			bConnected = SUCCEEDED ( m_pImapMailbox->OpenMailbox() );

			if (!bConnected)
				return NULL;
		}
	}
	else if (bToImap && pTargetToc)
	{
		if (!pTargetToc->m_pImapMailbox->IsSelected())
		{
			bConnected = SUCCEEDED ( pTargetToc->m_pImapMailbox->OpenMailbox() );

			if (!bConnected)
				return NULL;
		}
	}

	// Call the appropriate function to perform the kind of transfer needed (IMAP->IMAP, IMAP->local, etc.).
	if (bFromImap && bToImap)
	{
		if (m_pImapMailbox->GetAccountID() == pTargetToc->m_pImapMailbox->GetAccountID())
		{
			pCopySum = ImapXferOnSameAccount(pTargetToc, pSumList, iHighlightIndex, bCloseWindow, bCopy);
		}
		else
		{
			pCopySum = ImapXferBetweenAccounts(pTargetToc, pSumList, iHighlightIndex, bCloseWindow, bCopy);
		}
	}
	else
	{
		if (bFromImap && pTargetToc)
		{
			pCopySum = ImapXferFromImapToLocal(pTargetToc, pSumList, iHighlightIndex, bCloseWindow, bCopy);
		}
		else if (bToImap && pTargetToc)
		{
			pCopySum = ImapXferFromLocalToImap(pTargetToc, pSumList, iHighlightIndex, bCloseWindow, bCopy);
		}
		else
		{
			// Should not get here..
			ASSERT (0);
		}
	}

	// If we're offline but the user allowed this one action, close all connections.
	if (bMustCloseConnection)
	{
		if (bFromImap && m_pImapMailbox)
			m_pImapMailbox->Close();

		if (bToImap && pTargetToc && pTargetToc->m_pImapMailbox)
			pTargetToc->m_pImapMailbox->Close();
	}

	return pCopySum;
}

//
//	CTocDoc::ImapXferOnSameAccount()
//
//	Transfers messages between mailboxes on the same account.
//
CSummary* CTocDoc::ImapXferOnSameAccount(CTocDoc *pTargetToc,
										 CSumList *pSumList,
										 int &iHighlightIndex,
                                         BOOL bCloseWindow /*=TRUE*/,
										 BOOL bCopy /*=FALSE*/)
{
	POSITION							 pos = NULL;
	POSITION							 posNext = NULL;
	POSITION							 posLastSelected = NULL;
	CString								 strUidlist;
	CString								 strUid;
	CSumList							 sumlistToMove;
	BOOL								 bResult = TRUE;
	int									 nCount = 0;
	CSummary							*pSingleSum = NULL;
	CSummary							*pSumNew = NULL;
	CImapAccount						*pAccount = NULL;
	CImapSameServerTransferMsgAction	*pTransferAction = NULL;

	if (!m_pImapMailbox || !pTargetToc || !pTargetToc->m_pImapMailbox)
	{
		ASSERT(0);
		return NULL;
	}

	pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
	if (!pAccount)
	{
		ASSERT(0);
		return NULL;
	}

	iHighlightIndex = -1;

	pos = pSumList->GetHeadPosition();
	nCount = 0;
	for (posNext = pos; pos; pos = posNext)
	{
		CSummary	*pSum = pSumList->GetNext(posNext);

		if (!pSum)	continue;

		if ((pSingleSum == NULL) && (pSumList->GetCount() == 1))
		{
			pSingleSum = pSum;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!bCopy && !CanCloseSummaryFrame(pSum)) continue;

		// This summary is movable: add its UID to the string.
		if (strUidlist.IsEmpty())
		{
			strUidlist.Format("%lu", pSum->m_Hash);
		}
		else
		{
			strUid.Format (",%lu", pSum->m_Hash);
			strUidlist += strUid;
		}

		// Also, if this is a move, add this summary to a list for later deletion.
		// NOTE: Make sure this is in the same order as m_Sums!!!
		if (!bCopy)
		{
			sumlistToMove.AddTail(pSum);
			pSum->SetDeleteEmbedded(FALSE);
		}

		// Found another one.
		nCount++;
	}

	// Mark "posNext" for later use. Note: Could be NULL.
 	posLastSelected = posNext;

	// Bail now if we didn't find at least one message that we can act on.
	if (nCount <= 0)
	{
		return NULL;
	}

	// Create an action object for this operation and add it to the queue.
	pTransferAction = DEBUG_NEW CImapSameServerTransferMsgAction(m_pImapMailbox->GetAccountID(),
																 (char*)m_pImapMailbox->GetImapName(),
																 m_pImapMailbox->GetDelimiter(),
																 strUidlist.GetBuffer(0),
																 m_pImapMailbox->GetAccountID(),
																 pTargetToc->m_pImapMailbox->GetImapName(),
																 pTargetToc->m_pImapMailbox->GetDelimiter(),
																 bCopy);
	if (!pTransferAction)
	{
		return NULL;
	}

	pAccount->QueueAction(pTransferAction);

	if (!bCopy)
	{
		// If this was a move from a TOC window and bCloseWindow is TRUE, tell
		// ImapRemoveOrderedSubSums to close message windows as well.
		//
		// Note: Allow "DisplayNextMessageOrClose" to close it if necessary.
		bResult = ImapRemoveOrderedSubSums(&sumlistToMove, iHighlightIndex, bCloseWindow, TRUE, TRUE);

		// If this was a move from a readmsgdoc window, we may need to open the next message.
		if (pSingleSum && bCloseWindow && bResult)
		{
			DisplayNextMessageOrClose(this, pSingleSum, posLastSelected);
		}
	}

	// Must clean this up.
	sumlistToMove.RemoveAll();

	return pSumNew;
}

//
//	CTocDoc::ImapXferBetweenAccounts()
//
//	Transfer messages between mailboxes on different servers.
//
CSummary* CTocDoc::ImapXferBetweenAccounts(CTocDoc *pTargetToc,
										   CSumList *pSumList,
										   int &iHighlightIndex,
                                           BOOL bCloseWindow/*=TRUE*/,
										   BOOL bCopy/*=FALSE*/)
{
	POSITION							 pos = NULL;
	POSITION							 posNext = NULL;
	CString								 strUidlist;
	CString								 strUid;
	CTempSumList						 sumlistToMove;
	CSummary							*pSingleSum = NULL;
	BOOL								 bResult = TRUE;
	unsigned long						 nMsgs = 0;
	POSITION							 posLastSelected = NULL;
	BOOL								 bUseFancyTrash = FALSE;
	CTocView							*pTargetView = NULL;
	BOOL								 bPartialSuccess = FALSE;
	CImapAccount						*pAccount = NULL;
	CImapCrossServerTransferMsgAction	*pTransferAction = NULL;

	if (!m_pImapMailbox || !pTargetToc || !pTargetToc->m_pImapMailbox)
	{
		ASSERT(0);
		return NULL;
	}

	pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
	if (!pAccount)
	{
		return NULL;
	}

	CTocView	*pView = GetView();

	nMsgs = pSumList->GetCount();

	// Now, loop through the messages and COPY one at a time.
	// NOTE:  Don't remove the source tocs yet!!
	// At the same time, accumulate a new list in sumlistToMove of messages actually 
	// copied.

	// Set redraw of the target view to FALSE so we don't get flicker
	pTargetView = pTargetToc->GetView();
	if (pTargetView)
	{
		pTargetView->m_SumListBox.SetRedraw(FALSE);
	}

	BOOL bPartialFailure = FALSE;

	::AsyncEscapePressed(TRUE);

	pos = pSumList->GetHeadPosition();
	iHighlightIndex = -1;

	for (posNext = pos; pos; pos = posNext)
	{
		CSummary	*pSum = pSumList->GetNext(posNext);

		if ((pSingleSum == NULL) && (pSumList->GetCount() == 1))
		{
			pSingleSum = pSum;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!bCopy && !CanCloseSummaryFrame(pSum)) continue;

		// We succeeded at least once.
		bPartialSuccess = TRUE;

		if (!bCopy)
		{
			// Add to sumlistToMove for later deletion.
			// NOTE: This MUST be added in the same order as m_Sums!!!
			//
			sumlistToMove.AddTail(pSum);
		}

		// Also create a comma-separated array.
		if (strUidlist.IsEmpty())
		{
			strUidlist.Format("%lu", pSum->m_Hash);
		}
		else
		{
			strUid.Format(",%lu", pSum->m_Hash);
			strUidlist += strUid;
		}
	}

	// Create an action object for this operation and add it to the queue.
	pTransferAction = DEBUG_NEW CImapCrossServerTransferMsgAction(m_pImapMailbox->GetAccountID(),
																  (char*)m_pImapMailbox->GetImapName(),
																  m_pImapMailbox->GetDelimiter(),
																  strUidlist.GetBuffer(0),
																  pTargetToc->m_pImapMailbox->GetAccountID(),
																  pTargetToc->m_pImapMailbox->GetImapName(),
																  pTargetToc->m_pImapMailbox->GetDelimiter(),
																  bCopy);
	if (!pTransferAction)
	{
		return NULL;
	}

	pAccount->QueueAction(pTransferAction);

	// Mark "posNext" for later use. Note: Could be NULL.
	posLastSelected = posNext;

	// Resume redraw of target.
	if (pTargetView)
	{
		pTargetView->m_SumListBox.SetRedraw(TRUE);
	}

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

	// Ok. Some were copied.

	if (!bCopy && !strUidlist.IsEmpty())
	{
		bUseFancyTrash = m_pImapMailbox->UseFancyTrash();

		if (!bUseFancyTrash)
		{
			// Stop redraw of the source.
			if (pView)
			{
				pView->m_SumListBox.SetRedraw(FALSE);
			}

			// Note: Allow "DisplayNextMessageOrClose" to close it if necessary.
			// Just flag them for deletion.	
			bResult = ImapRemoveOrderedSubSums(&sumlistToMove, iHighlightIndex, bCloseWindow, TRUE, TRUE);

			// If tow-truck transfer, display next summary.
			if (pSingleSum && !bCloseWindow && bResult)
			{
				DisplayNextMessageOrClose(this, pSingleSum, posLastSelected);
			}	

			// Redraw again.
			if (pView)
			{
				pView->m_SumListBox.SetRedraw(TRUE);
			}
		}
	}

	// Put up message if some messages did not get transferred.
	if (bPartialFailure)
		ErrorDialog (IDS_WARN_IMAP_SOME_XFER_FAILED);

	// Cleanup before we exit.
end:

	// Make sure Free memory in IMapCopyQueue.
	sumlistToMove.RemoveAll();

	return NULL;
}

//
//	CTocDoc::ImapXferFromImapToLocal()
//
//	Transfer messages from an IMAP mailbox to a local one.
//
//	NOTE: Assume that a connection to our IMAP mailbox is already open.
//
CSummary* CTocDoc::ImapXferFromImapToLocal(CTocDoc *pTargetToc,
										   CSumList *pSumList,
										   int &iHighlightIndex,
                                           BOOL bCloseWindow /*=TRUE*/,
										   BOOL bCopy /*=FALSE*/)
{
	POSITION						 pos = NULL;
	POSITION						 posNext = NULL;
	CString							 strUidlist;
	CString							 strUid;
	CTempSumList					 sumlistToMove;
	CTempSumList					 sumlistToDelete;
	CSummary						*pSingleSum = NULL;
	BOOL							 bResult = TRUE;
	POSITION						 posLastSelected = NULL;
	CImapAccount					*pAccount = NULL;
	CImapToLocalTransferMsgAction	*pTransferAction = NULL;

	if (!m_pImapMailbox || !pTargetToc)
	{
		ASSERT(0);
		return NULL;
	}

	pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
	if (!pAccount)
	{
		return NULL;
	}

	// Whether we would be deleting the messages later or not, accumulate
	// the summaries into the sumlistToMove CSumList.

	pos = pSumList->GetHeadPosition();
	iHighlightIndex = -1;

	for (posNext = pos; pos; pos = posNext)
	{
		CSummary	*pSum = pSumList->GetNext(posNext);

		if ((pSingleSum == NULL) && (pSumList->GetCount() == 1))
		{
			pSingleSum = pSum;
		}

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!bCopy && !CanCloseSummaryFrame(pSum)) continue;

		// Add to sumlistToMove for later deletion.
		// Note!! These MUST be added in the same order as they currently exist in m_Sums.
		sumlistToMove.AddTail(pSum);

		// Also create a comma-separated array.
		if (strUidlist.IsEmpty())
		{
			strUidlist.Format("%lu", pSum->m_Hash);
		}
		else
		{
			strUid.Format(",%lu", pSum->m_Hash);
			strUidlist += strUid;
		}
	}

	// Bail now if there is nothing we can do.
	if (sumlistToMove.GetCount() == 0)
	{
		ErrorDialog(IDS_ERR_IMAP_NO_MSGS_XFERRED);
		return NULL;
	}

	// Create an action object for this operation and add it to the queue.
	pTransferAction = DEBUG_NEW CImapToLocalTransferMsgAction(m_pImapMailbox->GetAccountID(),
															  (char*)m_pImapMailbox->GetImapName(),
															  m_pImapMailbox->GetDelimiter(),
															  strUidlist.GetBuffer(0),
															  pTargetToc->GetMBFileName(),
															  bCopy);
	if (!pTransferAction)
	{
		return NULL;
	}

	pAccount->QueueAction(pTransferAction);

	// Mark "posNext" for later use. Note: Could be NULL.
	posLastSelected = posNext;

	// Ok. Some were copied. This is now equivalent to removing the messages from the remote
	// mailbox and from the local cache.
	// Note: This may set iHighlightIndex for us. "sumlistToMove" and "iHighlightIndex" are both passed
	// as references.

	if (!bCopy)
	{
		// Stop redraw of the source.
		CTocView	*pView = GetView();
		if (pView)
		{
			pView->m_SumListBox.SetRedraw(FALSE);
		}

		// Mark deleted messages as read.
		MarkAllAsRead(sumlistToMove);

		// Note: Just flag them for deletion.
		// 
		// Note: Allow "DisplayNextMessageOrClose" to close it if necessary.
		bResult = ImapRemoveOrderedSubSums(&sumlistToMove,
										   iHighlightIndex,
										   bCloseWindow,
										   TRUE/*SetViewSelection*/,
										   TRUE/*bJustSetFlag*/);

		// If tow-truck transfer, display next summary.
		if (pSingleSum && !bCloseWindow && bResult)
		{
			DisplayNextMessageOrClose(this, pSingleSum, posLastSelected);
		}

		// Redraw again.
		if (pView)
		{
			pView->m_SumListBox.SetRedraw(TRUE);
		}
	}

	sumlistToDelete.RemoveAll();

	return NULL;
}

//
//	CTocDoc::ImapXferFromImapToLocal()
//
//	Transfer messages from a local mailbox to an IMAP one.
//
CSummary* CTocDoc::ImapXferFromLocalToImap(CTocDoc *pTargetToc,
										   CSumList *pSumList,
										   int& iHighlightIndex,
                                           BOOL bCloseWindow /*=TRUE*/,
										   BOOL bCopy /*=FALSE*/)
{
	POSITION						 pos = NULL;
	POSITION						 posNext = NULL;
	CString							 strUid;
	CString							 strUidlist;
	CImapAccount					*pAccount = NULL;
	CImapFromLocalTransferMsgAction	*pTransferAction = NULL;

	if (!pTargetToc || !pTargetToc->m_pImapMailbox)
	{
		ASSERT (0);
		return NULL;
	}

	// Initialize
	iHighlightIndex = -1;

	pAccount = g_ImapAccountMgr.FindAccount(pTargetToc->m_pImapMailbox->GetAccountID());
	if (!pAccount)
	{
		return NULL;
	}

	// Stop updating both the source and destination mailboxes.
	CTocView	*pView = GetView();
	if (pView)
	{
		pView->m_SumListBox.SetRedraw(FALSE);
	}

	CTocView	*pTargetView = pTargetToc->GetView();
	if (pTargetView)
	{
		pTargetView->m_SumListBox.SetRedraw(FALSE);
	}

	iHighlightIndex = -1;

	// Loop through the messages and COPY one at a time.
	// NOTE:  Don't remove the source tocs yet!!
	// Note; As a message is copied, its UID is removed from ImapQueue so if there's
	// a failure, we can go undelete what left in ImapQueue.
	// At the same time, accumulate a new list in ImapCopyQueue of messgae actually 
	// copied.

	BOOL bPartialSuccess = FALSE;
	BOOL bPartialFailure = FALSE;

	pos = pSumList->GetHeadPosition();
	::AsyncEscapePressed(TRUE);

	for (posNext = pos; pos; pos = posNext)
	{
		CSummary* pSum = pSumList->GetNext(posNext);

		// Give some time back, and let user cancel out by hitting Esc
		if (AsyncEscapePressed())
			break;

		// If we're not copying, don't move this summary unless we're allowed to close it.
		// jdboyd 8/23/99
		if (!bCopy && !CanCloseSummaryFrame(pSum)) continue;

		InvalidateCachedPreviewSummary(pSum, true);

		// Also create a comma-separated array.
		if (strUidlist.IsEmpty())
		{
			strUidlist.Format("%lu", pSum->m_Hash);
		}
		else
		{
			strUid.Format(",%lu", pSum->m_Hash);
			strUidlist += strUid;
		}

		// We succeeded at least once.
		bPartialSuccess = TRUE;

		// If we succeed and this is a move, delete source summary.
		if (!bCopy)
		{
			// If we're moving the summary from the out box, don't delete the summary 
			// because it can cause a crash in the post-filtering code (sendmail.cpp)
			// that re-uses the summary after post-filtering!! Instead, move it to the
			// local Trash folder.
			if (pSum->IsComp() || m_Type == MBT_OUT)
			{
				CTocDoc			*pTrashToc = GetTrashToc();
				POSITION		 posInSums = m_Sums.Find(pSum);
				if (pTrashToc && posInSums && ImapMoveContents(pTrashToc, pSum))
				{
					// Move the summary.
					SubtractSum(posInSums, -1);
					pTrashToc->AddSum(pSum);
				}
			}
			else
			{
				// Remove any attachments before we delete the summary.
				// With the local/cache/online IMAP model we don't want to delete this attachment
				// just yet because the transfer won't be complete until the action is completed
				// online.
				// ImapRemoveAttachments(pSum);

				// close the message window
				// (this used to be done in ImapRemoveAttachments() -jdboyd 8/16/99
				CFrameWnd* Frame = pSum->m_FrameWnd;
				if (Frame)
				{
					if (Frame->GetActiveDocument()->CanCloseFrame(Frame))
					{
						Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
					}
				}

				// Includes free-ing memory.
				RemoveSum(pSum);

				pSum = NULL;
			}
		}
	}

	// Create an action object for this operation and add it to the queue.
	pTransferAction = DEBUG_NEW CImapFromLocalTransferMsgAction(m_MBFileName,
																pTargetToc->m_pImapMailbox->GetAccountID(),
																(char*)pTargetToc->m_pImapMailbox->GetImapName(),
																pTargetToc->m_pImapMailbox->GetDelimiter(),
																strUidlist.GetBuffer(0),
																bCopy);
	if (!pTransferAction)
	{
		return NULL;
	}

	pAccount->QueueAction(pTransferAction);

	// Resume updating both the source and destination mailboxes.
	if (pTargetView)
	{
		pTargetView->m_SumListBox.SetRedraw(TRUE);
	}

	if (pView)
	{
		if (!bCopy && iHighlightIndex < 0)
		{
			// Select the last summary if there weren't any other summaries selected
			if (iHighlightIndex < 0)
				iHighlightIndex = NumSums() - 1;
		}
			
		if (iHighlightIndex >= 0)
		{
			if (iHighlightIndex >= NumSums())
				iHighlightIndex = NumSums() - 1;
		}

		// Set redraw back to TRUE
		pView->m_SumListBox.SetRedraw(TRUE);
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

	return NULL;
}



// ImapCopyMsgFromImapToLocal [PRIVATE]

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
	
	pSum->SetDeleteEmbedded(FALSE);

//	if(pTargetToc->IsT
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
				if ( SUCCEEDED(comp->Write(NULL)) )
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
			// The only time we should get here is if the user transferred an undownloaded IMAP
			// message to a local mailbox.  (If an IMAP message was fully downloaded then the local
			// transfer that was done earlier did all the work already.)  We expect to find a
			// summary for this message in the target toc (since we did a local copy) but in some
			// failure cases there is no copy.  We handle both of those cases below.
			CSummary		*pCopySum = pTargetToc->GetSummaryFromUID(pSum->GetHash());
			if (pCopySum)
			{
				// We found the summary in the target toc so subtract the summary from the target toc,
				// reset its ID (which is no longer valid) and update its length (which changed when
				// the message was downloaded).
				pTargetToc->SubtractSum(pCopySum);
				pCopySum->m_UniqueMessageId = 0;
				pCopySum->m_Length = pSum->m_Length;
			}
			else
			{
				// We did not find the summary in the target toc so create a new summary now and fill
				// it with the data from the source summary.
				pCopySum = DEBUG_NEW_MFCOBJ_NOTHROW CSummary;
				if (pCopySum)
				{
					pCopySum->m_UniqueMessageId = 0;
					pCopySum->Copy(pSum);
				}
			}
			if (pCopySum)
			{
				// We now have a summary (whether old or new) so add it to the target toc.
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

	// We must be an IMAP mailbox.
	if (!pSum || !ppCopySum || !m_pImapMailbox || !pTargetToc || !pTargetToc->m_pImapMailbox)
	{
		ASSERT(0);
		return FALSE;
	}

	// Initialize ppCopySum.
	*ppCopySum = NULL;

	pSum->SetDeleteEmbedded(FALSE);

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
		if (bResult)
		{ 
			CSummary		*pCopySum = pTargetToc->GetSummaryFromUID(pSum->GetHash());
			if (pCopySum)
			{
				pTargetToc->SubtractSum(pCopySum);
			}
			else
			{
				pCopySum = DEBUG_NEW_MFCOBJ_NOTHROW CSummary;
			}
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
    pSum->SetDeleteEmbedded(FALSE);
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
		if (bResult)
		{ 
			// Create target summary.
			CSummary *pCopySum = DEBUG_NEW_MFCOBJ_NOTHROW CSummary;
			if (pCopySum)
			{
				pCopySum->Copy(pSum);

				pTargetToc->AddSum(pCopySum);
				pCopySum->SetOffset(MStart);
				pCopySum->m_Length = NewLength;

				// Messages in the local Out mailbox have a hash of 0.  If the user copies more
				// than one message from Out to an IMAP mailbox all copies in the IMAP mailbox
				// will have hashes of 0 which causes problems later.  If we are transferring
				// a message with an empty hash, assign it the new hash now.
				if (pSum->GetHash() == 0)
				{
					pSum->SetHash(NewUid);
				}

				// This is returned in ppCopySum.
				*ppCopySum = pCopySum;
			}
		}
	}

	return bResult;
}


//
//	CTocDoc::ImapTransferSummaryData()
//
//	Copies the data between two summary objects in two different TOC's
//	using UIDPLUS data to associate the two objects.  This allows us to
//	preserve junk score data when messages are moved between IMAP mailboxes.
//
//	Parameters:
//		strUIDList[in] - List of UIDs for summary objects in this TOC.
//		pTargetToc[in] - TOC where messages are being moved.
//		pdwaNewUIDs[in] - Array of longs showing UIDs for summary objects
//			in pTargetToc.
//		pImapSum[in] - Optional pointer to a CImapSum object.  If this is
//			non-NULL strUIDList will be ignored and this object's UID will
//			be used.
//		bFindNew[in] - If true look for matching new summary before creating
//			one.  This function can now be called in a couple situations --
//			the previous case where we did not expect to find a summary and a
//			new case where we know the summary has been created (via the
//			local/cache/online model).  Perhaps someday this function will
//			always be used in the latter case but for now we use this parameter
//			to allow both.
//
CSummary *CTocDoc::ImapTransferSummaryData(CString &strUIDList,
										   CTocDoc *pTargetToc,
										   CDWordArray *pdwaNewUIDs,
										   CImapSum *pImapSum,
										   bool bFindNew)
{
	if (strUIDList.IsEmpty() || !pTargetToc || !pdwaNewUIDs)
	{
		return NULL;
	}

	int				 i = 0;
	int				 iSize = pdwaNewUIDs->GetSize();
	unsigned long	 lOldUID = 0;
	unsigned long	 lNewUID = 0;
	CString			 strLocalUIDList = strUIDList;
	CString			 strOldUID;
	int				 iPos = -1;
	CSummary		*pSumOld = NULL;
	CSummary		*pSumNew = NULL;
	bool			 bFoundNew = true;

	// This is potentially way too inefficient for a long term solution.
	// Better would be to loop through the summaries once, adding to a CSumList any summarys that match
	// the list and go until the size of the list matches the number of entries.
	while (!strUIDList.IsEmpty() && (i < iSize))
	{
		iPos = strLocalUIDList.Find(",");
		if (iPos > -1)
		{
			strOldUID = strLocalUIDList.Left(iPos);
			strLocalUIDList = strLocalUIDList.Right(strLocalUIDList.GetLength() - iPos - 1);
		}
		else
		{
			strOldUID = strLocalUIDList;
		}
		lOldUID = atoi(strOldUID);
		lNewUID = pdwaNewUIDs->ElementAt(i);

		if (bFindNew)
		{
			pSumNew = pTargetToc->GetSummaryFromUID(lOldUID);
		}
		if (!pSumNew)
		{
			bFoundNew = false;
			pSumNew = DEBUG_NEW_MFCOBJ_NOTHROW CSummary;
		}
		if (pSumNew)
		{
			bool	bSaveNotifySearchManager = pSumNew->GetNotifySearchManager();
			pSumNew->SetNotifySearchManager(false);
			
			if (pImapSum)
			{
				pImapSum->CopyToCSummary(pSumNew);
				if (!pImapSum->m_RawDateString.IsEmpty())
				{
					CString		 strTZ = pImapSum->m_RawDateString.Right(5);
					pSumNew->m_TimeZoneMinutes = (((atoi((const char *)strTZ)) / 100) * 60);
				}
				pSumNew->m_Seconds -= (pSumNew->m_TimeZoneMinutes * 60);
				pSumNew->SetDate();
			}
			else
			{
				pSumOld = GetSummaryFromUID(lOldUID);
				if (pSumOld)
				{
					// If the new summary already exists, keep its message ID.
					long		 lUniqueMessageId = pSumNew->m_UniqueMessageId;
					if (bFindNew)
					{
						// Set the message ID to 0 while we do the Copy().
						pSumNew->m_UniqueMessageId = 0;
					}
					// Because transfers are done locally first, by the time we get here the source copy
					// of the transferred message is marked as deleted so if we copy that flag the new
					// copy of the message will be marked as deleted.  Because deleted messages are marked
					// as read the state will probably also be wrong.  Cache the flags and state and reset
					// them after the call to Copy().
					unsigned long		lFlags = pSumNew->m_Imflags;
					short				sState = pSumNew->m_State;
					pSumNew->Copy(pSumOld);
					if (bFindNew)
					{
						// Restore the valid message ID.
						pSumNew->m_UniqueMessageId = lUniqueMessageId;
					}
					pSumNew->m_Imflags = lFlags;
					pSumNew->m_State = sState;
				}
			}
			pSumNew->m_Hash = lNewUID;

			//	Restore notification of the search manager
			pSumNew->SetNotifySearchManager(bSaveNotifySearchManager);

			// Only do the following if the new summary didn't previously exist.	
			if (!bFoundNew)
			{
				// Don't want to copy these values since they do not apply
				// to the new summary.
				pSumNew->m_Imflags = IMFLAGS_NOT_DOWNLOADED;
				pSumNew->m_Offset = 0;
				pSumNew->m_Length = 0;

				// Add the new summary.  During the subsequent mailbox check
				// CImapMailbox::ImapAppendSumList() will find this summary
				// and use it instead of creating a new one.
				pTargetToc->AddSum(pSumNew);
			}
			else
			{
				// Summary already existed, notify search manager if indicated
				if (bSaveNotifySearchManager)
					SearchManager::Instance()->NotifySumModification(pSumNew->m_TheToc, pSumNew);
			}
		}

		++i;
	}
	return pSumNew;
}

//
//	CTocDoc::GetSummaryFromUID()
//
//	Returns a pointer to the CSummary object in this TOC whose
//	hash matches lUID.
//
//	Parameters:
//		lUID[in] - UID to match.
//
//	Return:
//	Pointer to CSummary whose hash matches lUID.  NULL if no match found.
//
CSummary *CTocDoc::GetSummaryFromUID(unsigned long lUID)
{
	CSummary		*pSummary = NULL;
	POSITION		 pos = NULL;
	POSITION		 posNext = NULL;
	CSummary		*pSum = NULL;

	pos = m_Sums.GetHeadPosition();

	for (posNext = pos; pos && !pSummary; pos = posNext)
	{
		pSum = m_Sums.GetNext(posNext);
		if (pSum->GetHash() == lUID)
		{
			pSummary = pSum;
		}
	}

	return pSummary;
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
		InvalidateCachedPreviewSummary(Sum, true);

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
			ImapRemoveAttachments(Sum);

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
				// Previous code said this is the result of direct user action so maintain
				// that (passing true for the last parameter is equivalent to calling
				// MailboxChangedByHuman, but slightly more efficient).
				View->m_SumListBox.SetSel(HighlightIndex, TRUE, true);
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
	InvalidateCachedPreviewSummary();

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
					Sum->SetState(MS_READ);
				}

				SearchManager::Instance()->NotifySumModification(Sum->m_TheToc, Sum);
			}
			else
			{
				// Remove any attachments before we delete the summary.
				//
				ImapRemoveAttachments(Sum);

				// Remove the summary, including freeing memory.
		
				RemoveSum(Sum);

				// We removed a summary, so decrement counter.
				i--;
			}
		} // if UidPos.
	}

	if (bJustSetFlag && View)
	{
		View->MaybeResetContents();
	}

	// sET HighlightIndex.
	// If there is a summary after "LastSelectedIndex", select that one.
	//
	if (m_bHideDeletedIMAPMsgs)
	{
		// We are hiding deleted messages, no need to increment selection because the previously
		// selected message was deleted and its summary was hidden.
		HighlightIndex = LastSelectedIndex;
	}
	else if (bUndelete)
	{
		HighlightIndex = LastSelectedIndex;
	}
	else
	{
		// We are showing deleted messages, increment the highlight so it is the message following
		// the just deleted message.
		HighlightIndex = LastSelectedIndex + 1;
	}

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

	// Make sure preview updates
	InvalidateCachedPreviewSummary(NULL, true);

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
		InvalidateCachedPreviewSummary(Sum, true);

		// Remove any attachments before we delete the summary.
		//
		ImapRemoveAttachments(Sum);

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

	if (!m_pImapMailbox->IsSelected())
	{
		int		 iConnectionState = GetConnectionState(bDoAllowOffline);
		if (iConnectionState == iStateGoOnlineForThis)
		{
			// The connection is being allowed for this action only: note that we must close the connection
			// when we are done.
			bMustCloseConnection = TRUE;
		}
		else if (iConnectionState == iStateStayOfflineDisallow)
		{
			// No connection is being created and this action cannot be done offline: bail out.
			return S_OK;
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

	if (!m_pImapMailbox->IsSelected())
	{
		int		 iConnectionState = GetConnectionState(bDoAllowOffline);
		if (iConnectionState == iStateGoOnlineForThis)
		{
			// The connection is being allowed for this action only: note that we must close the connection
			// when we are done.
			bMustCloseConnection = TRUE;
		}
		else if (iConnectionState == iStateStayOfflineDisallow)
		{
			// No connection is being created and this action cannot be done offline: bail out.
			return S_OK;
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


//
//	CTocDoc::ImapJunkMessages()
//
//	Handles the bookkeeping tasks associated with junking messages (are we connected?
//	showing progress) and calls ImapChangeMsgsJunkStatus() to do the actual work.
//
//	Parameters
//		bJunk[in] - true if message is becoming junk, false otherwise.
//		pSingleSum[in] - Pointer to summary to process.  If this pointer
//			is NULL process all selected summaries.
//		bSilent[in] - If TRUE don't show progress.
//
//	Return
//		S_OK on success, an E_ failure code on failure.
//
HRESULT CTocDoc::ImapJunkMessages(bool bJunk, CSummary *pSingleSum /*=NULL*/, BOOL bSilent /*=TRUE*/)
{
	HRESULT		 hResult = E_FAIL;
	BOOL		 bMustCloseProgress = FALSE;

	if (!m_pImapMailbox)
	{
		return E_FAIL;
	}

	// If we're offline, ask what to do:
	BOOL bMustCloseConnection = FALSE;
	if (!m_pImapMailbox->IsSelected())
	{
		int		 iConnectionState = GetConnectionState(bDoAllowOffline);
		if (iConnectionState == iStateGoOnlineForThis)
		{
			// The connection is being allowed for this action only: note that we must close the connection
			// when we are done.
			bMustCloseConnection = TRUE;
		}
		else if (iConnectionState == iStateStayOfflineDisallow)
		{
			// No connection is being created and this action cannot be done offline: bail out.
			return S_OK;
		}
	}

	// Put up progress bar if it's not already up.
	if (!InProgress && !bSilent && ::IsMainThreadMT())
	{
		CString buf;
		buf.Format(CRString(IDS_IMAP_DELETING_MSGS));

		MainProgress(buf);

		bMustCloseProgress = TRUE;
	}

	// Move the messages to the Junk mailbox.
	hResult = ImapChangeMsgsJunkStatus(pSingleSum, bJunk, true/*bManual*/);

	// Close progress if necessary.
	if (bMustCloseProgress)
	{
		CloseProgress();
	}

	CTocView	*pView = GetView();
	if (pView)
	{
		// Force a re-prewiew.
		pView->SetFocus();
	}

	// Close the connection if necessary.
	if (bMustCloseConnection)
	{
		m_pImapMailbox->Close();
	}

	return hResult;
}

//
//	CTocDoc::ImapExpunge()
//
//	Creates an IMAP action object to perform the EXPUNGE.
//
HRESULT CTocDoc::ImapExpunge (BOOL bSilent /* = TRUE */)
{
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

	if (!m_pImapMailbox->IsSelected())
	{
		int		 iConnectionState = GetConnectionState(bDoAllowOffline);
		if (iConnectionState == iStateGoOnlineForThis)
		{
			// The connection is being allowed for this action only: note that we must close the connection
			// when we are done.
			bMustCloseConnection = TRUE;
		}
		else if (iConnectionState == iStateStayOfflineDisallow)
		{
			// No connection is being created and this action cannot be done offline: bail out.
			return S_OK;
		}
	}

	CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
	if (pAccount)
	{
		CImapExpungeAction		*pExpungeAction = DEBUG_NEW CImapExpungeAction(m_pImapMailbox->GetAccountID(),
																			(char*)m_pImapMailbox->GetImapName(),
																			m_pImapMailbox->GetDelimiter());
		if (!pExpungeAction)
		{
			return NULL;
		}

		pAccount->QueueAction(pExpungeAction);
	}

	return S_OK;
}

//
//	CTocDoc::ExpungeOnServer()
//
//	Performs the online EXPUNGE of this mailbox.
//
void CTocDoc::ImapExpungeOnServer()
{
	if (!m_pImapMailbox)
	{
		ASSERT(0);
		return;
	}

	// Expunge from the server mailbox.
	m_pImapMailbox->Expunge();
}

//
//	CTocDoc::ImapDoPostExpunge()
//
//	Performs the actions needed after the online EXPUNGE of this mailbox.
//
void CTocDoc::ImapDoPostExpunge()
{
	// In case the progress is not shown immediately:
	CCursor cur;

	// Resume updates.
	CTocView	*pView = GetView();
	if (pView)
	{
		// Reset the view's contents.
		pView->MaybeResetContents();

		pView->m_SumListBox.SetRedraw(TRUE);

		// If no more messages, there is a redraw problem.
		if (m_Sums.GetCount() == 0)
		{
			CTocFrame	*pTocFrame = (CTocFrame*)(pView->GetParentFrame());
			if (pTocFrame)
			{
				ASSERT_KINDOF(CTocFrame, pTocFrame);
				pTocFrame->Invalidate ();
				pTocFrame->UpdateWindow ();
			}

			pView->m_SumListBox.SetRedraw(TRUE);
			pView->m_SumListBox.Invalidate();
			pView->m_SumListBox.UpdateWindow();

			// Also the view itself.
			pView->Invalidate();
			pView->UpdateWindow();
		}

		// Set the focus back to the sumlist!
		pView->SetFocus();
	}
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
	int			iDeleted = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		if (m_bHideDeletedIMAPMsgs && Sum && Sum->IsIMAPDeleted())
		{
			++iDeleted;
			continue;
		}

		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i - iDeleted) <= 0) )
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

	CDWordArray		 dwaNewUIDs;
	CTocDoc			*pTargetToc = NULL;
	hResult = m_pImapMailbox->MoveMsgsToTrash((LPCSTR)sUidlist,
											  &dwaNewUIDs,
											  mapUidsActuallyRemoved,
											  &pTargetToc);

	// Remove the summaries corresponding to the UID's that were actually expunged.
	//
	if ( SUCCEEDED (hResult) )
	{
		// Use the UIDPLUS data to move summary data between the TOC's.
		ImapTransferSummaryData(sUidlist, pTargetToc, &dwaNewUIDs);

		int HighlightIndex;
		if (mapUidsActuallyRemoved.GetCount() > 0)
		{
			// This case is probably defunct.  With the ability to hide deleted IMAP messages
			// we never automatically expunge messages.  mapUidsActuallyRemoved is only filled
			// in when messages are actually expunged.
			ImapRemoveListedTocs (&mapUidsActuallyRemoved,
							HighlightIndex,
							TRUE,							// CloseWindow
							TRUE,							// SetViewSelection
							FALSE);							// bJustSetFlag,
		}
		else
		{
			// This is now the expected case.  No messages are actually expunged so we fill
			// mapUidsActuallyRemoved with the contents of sUidlist and use that list.
			CommaListToUidMap(sUidlist, mapUidsActuallyRemoved);
			ImapRemoveListedTocs (&mapUidsActuallyRemoved,
							HighlightIndex,
							TRUE,							// CloseWindow
							TRUE,							// SetViewSelection
							TRUE);							// bJustSetFlag,
		}

		// Force a serialization of the TOC:
		//
		Write();

		if (View)
		{
			View->MaybeResetContents();
		}

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


//
//	CTocDoc::ImapChangeMsgsJunkStatus() [PRIVATE]
//
//	Translates the summary (if specified) or selected summaries (if none specified)
//	into a CString of UID's and passes this string to the ImapChangeMsgsJunkStatus()
//	which takes a string and does the actual work.
//
//	Parameters
//		pSingleSum[in] - Pointer to summary to process.  If this pointer
//			is NULL process all selected summaries.
//		bJunk[in] - true if message is becoming junk, false otherwise.
//
//	Return
//		S_OK on success, an E_ failure code on failure.
//
HRESULT CTocDoc::ImapChangeMsgsJunkStatus(CSummary *pSingleSum, bool bJunk, bool bManual)
{
	// Get the view so we can find selected messages.
	CTocView	*pView = GetView();

	// Accumulate a uid list so we can do a block IMAP command.
	POSITION	 pos = m_Sums.GetHeadPosition();
	POSITION	 posNext = NULL;
	CString		 strUIDList, strUID;
	int			 i = 0;
	int			 nCount = 0;
	CSummary	*pSum = NULL;

	CObArray	oaABHashes;

	// Generate the address book hashes once for the entire loop
	CFilter::GenerateHashes(&oaABHashes);

	int			 iDeleted = 0;

	for (i = 0, posNext = pos; pos; pos = posNext, i++)
	{
		pSum = m_Sums.GetNext(posNext);

		// If summary was specified, see if we have found it.
		if (pSingleSum && (pSingleSum != pSum))
		{
			continue;
		}
			
		if (m_bHideDeletedIMAPMsgs && pSum && pSum->IsIMAPDeleted())
		{
			++iDeleted;
			continue;
		}

		// If this summary isn't selected (non-pSingleSum case) then continue on
		if (!pSingleSum && pView && (pView->m_SumListBox.GetSel(i - iDeleted) <= 0))
		{
			continue;
		}

		// Ok. This is selected.

		// If this is a manual action, fill in the appropriate fields.
		if (bManual)
		{
			// Inform the junk translators of the change in status.
			if (bManual)
			{
				long			lContext = bJunk ? EMSFJUNK_MARK_IS_JUNK : EMSFJUNK_MARK_NOT_JUNK;
				lContext |= EMSFJUNK_USER_INITIATED;
				GetTransMan()->CallJunkTranslators(pSum, lContext, &oaABHashes);
			}

			// Note that this summary was manually junked/not junked and set its score.
			if (bJunk)
			{
				pSum->SetJunkScore((unsigned char)GetIniShort(IDS_INI_MANUAL_JUNK_SCORE));
			}
			else
			{
				pSum->SetJunkScore((unsigned char)GetIniShort(IDS_INI_MANUAL_NOT_JUNK_SCORE));
			}
			pSum->SetJunkPluginID(0);
			pSum->SetManuallyJunked(true);

			// Add the sender to the address book if desired.
			if (!bJunk &&
				GetIniShort(IDS_INI_ADDBOOK_IS_WHITELIST) &&
				GetIniShort(IDS_INI_ADD_NONJUNK_TO_ADDBOOK))
			{
				CObArray		*poaABHashes = NULL;
				poaABHashes = DEBUG_NEW CObArray;
				CFilter::GenerateHashes(poaABHashes);
				if (!CJunkMail::IsWhitelisted(pSum, poaABHashes))
				{
					CJunkMail::AddMsgSenderToAddressBook(pSum);
				}
			}
		}

		// Add UID to list.
		if (strUIDList.IsEmpty())
		{
			strUIDList.Format("%lu", pSum->m_Hash);
		}
		else
		{
			strUID.Format (",%lu", pSum->m_Hash);
			strUIDList += strUID;
		}

		nCount++;

		if (pSingleSum)
		{
			break;
		}
	}

	// If no summaries found just exit.
	if (nCount <= 0)
	{
		return S_OK;
	}

	return ImapChangeMsgsJunkStatus(strUIDList, bJunk);
}


//
//	CTocDoc::ImapChangeMsgsJunkStatus() [PRIVATE]
//
//	Translates the summary into a CString of UID with its UID and passes this
//	string to the ImapChangeMsgsJunkStatus() which takes a string and does the
//	actual work.
//
//	Parameters
//		pSum[in] - Pointer to summary to process.
//		bJunk[in] - true if message is becoming junk, false otherwise.
//
//	Return
//		S_OK on success, an E_ failure code on failure.
//
HRESULT CTocDoc::ImapChangeMsgsJunkStatus(CImapSum *pImapSum, bool bJunk)
{
	if (!pImapSum)
	{
		return E_FAIL;
	}

	CString		 strUIDList;
	strUIDList.Format("%lu", pImapSum->GetHash());

	HRESULT		 hResult = ImapChangeMsgsJunkStatus(strUIDList, bJunk, pImapSum);

	if (bJunk)
	{
		if (hResult == S_OK)
		{
			// We junked the message: find the summary for the original (non-Junk mailbox) copy
			// of this message and mark it as read.
			CSummary		*pSum = GetSummaryFromUID(pImapSum->GetHash());
			pSum->SetState(MS_READ);
		}
	}

	CTocView	*pView = GetView();
	if (pView)
	{
		pView->MaybeResetContents();
	}

	return hResult;
}


//
//	CTocDoc::ImapChangeMsgsJunkStatus() [PRIVATE]
//
//	Internal method to do the real work of transferring messages to junk.
//
//	Parameters
//		strUIDList[in] - String containing UIDs of messages to junk.
//		bJunk[in] - true if message is becoming junk, false otherwise.
//
//	Return
//		S_OK on success, an E_ failure code on failure.  In most cases
//		this simply returns the code returned by MoveMsgsToOrFromJunk().
//
HRESULT CTocDoc::ImapChangeMsgsJunkStatus(CString &strUIDList, bool bJunk, CImapSum *pImapSum)
{
	HRESULT		hResult = S_OK;

	// This shouldn't happen, but...
	if (strUIDList.IsEmpty())
	{
		return S_OK;
	}

	// Sanity: Must have a valid IMAP object.
	if (!m_pImapMailbox)
	{
		ASSERT(0);
		return E_FAIL;
	}

	// If the mailbox is read-only, we can't continue.
	if (m_pImapMailbox->IsReadOnly())
	{
		ErrorDialog(IDS_WARN_IMAP_RDONLY_MODIFYFLAG, Name());
		return E_FAIL;
	}

	// Ok, ask the CImapMailbox to do the xfer for us. If it succeeds, it will return
	// a UidMap of the UID's that were actually removed from the mailbox.
	CUidMap			 mapUidsActuallyMoved;
	CDWordArray		 dwaNewUIDs;
	CTocDoc			*pTargetToc = NULL;
	hResult = m_pImapMailbox->MoveMsgsToOrFromJunk(bJunk,
												   (LPCSTR)strUIDList,
												   &dwaNewUIDs,
												   mapUidsActuallyMoved,
												   &pTargetToc);

	if (SUCCEEDED(hResult))
	{
		// Use the UIDPLUS data to move summary data between the TOC's.
		ImapTransferSummaryData(strUIDList, pTargetToc, &dwaNewUIDs, pImapSum);

		int HighlightIndex;
		if (mapUidsActuallyMoved.GetCount() > 0)
		{
			// Remove the summaries corresponding to the UID's that were actually expunged.
			ImapRemoveListedTocs(&mapUidsActuallyMoved,
								 HighlightIndex,
								 TRUE /*CloseWindow*/,
								 TRUE /*SetViewSelection*/,
								 FALSE /*bJustSetFlag*/);
		}
		else
		{
			CommaListToUidMap(strUIDList, mapUidsActuallyMoved);
			ImapRemoveListedTocs(&mapUidsActuallyMoved,
								 HighlightIndex,
								 TRUE /*CloseWindow*/,
								 TRUE /*SetViewSelection*/,
								 TRUE /*bJustSetFlag*/);
		}

		// Reset the view's contents to account for any messages now marked as deleted.
		// We should someday be smarter and make sure the specified messages actually moved.  If the user
		// is allowing messages to be marked as junk inside the Junk mailbox then this function will be
		// called but the messages will not actually be marked as deleted. -dwiggins
		CTocView	*pView = GetView();
		if (pView)
		{
			pView->MaybeResetContents();
		}

		// Write the TOC.
		Write();
	}
	else
	{
		TCHAR		 buf[512];
		int			 iType = IMAPERR_BAD_CODE;
		CString		 msg;

		buf[0] = '\0';

		m_pImapMailbox->GetLastImapErrorAndType(buf, 510, &iType);
	
		if (buf[0])
		{
			if (iType != IMAPERR_USER_CANCELLED && iType != IMAPERR_LOCAL_ERROR)
			{
				ErrorDialog(IDS_ERR_IMAP_CANNOT_MODIFYFLAG, Name(), buf);
			}
			else if (iType != IMAPERR_USER_CANCELLED)
			{
				ErrorDialog(IDS_ERR_IMAP_COMMAND_FAILED);
			}
		}
	}

	return hResult;
}


//
//	CTocDoc::ImapRecheckMessageForJunk()
//
//	Recalculate the junk score for the message specified by pSum and
//	do the appropriate transfer if the new score changes the message's
//	junk status.
//
//	Parameters
//		pSum [in] - Summary of the message to be rescored.
//		poaABHashes[in] - Address Book hashes
//
void CTocDoc::ImapRecheckMessageForJunk(CSummary *pSum, CObArray *poaABHashes)
{
	if (!pSum)
	{
		return;
	}

	CTranslatorManager	*pManager = ((CEudoraApp*)AfxGetApp())->GetTranslators();
	if (pManager)
	{
		pManager->CallJunkTranslators(pSum, EMSFJUNK_SCORE_ON_ARRIVAL, poaABHashes);

		if (UsingFullFeatureSet() && GetIniShort(IDS_INI_USE_JUNK_MAILBOX))
		{
			if (m_pImapMailbox && IsInbox(m_pImapMailbox->GetImapName()) && (pSum->GetJunkScore() >= (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK)))
			{
				// The message is not in the junk mailbox but now has a sufficiently high junk score:
				// move it to Junk.
				ImapChangeMsgsJunkStatus(pSum, true);
			}
			else if (IsJunk() && (pSum->GetJunkScore() < (unsigned char)GetIniShort(IDS_INI_MIN_SCORE_TO_JUNK)))
			{
				// The message is in the junk mailbox but now has a sufficiently low junk score:
				// move it out of Junk.
				ImapChangeMsgsJunkStatus(pSum, false);
			}
		}
	}
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
	CTempSumList	ImapMoveSumList;
	HRESULT		hResult = S_OK;
	BOOL		bRet = FALSE;
	int			i = 0;
	int nCount = 0;
	POSITION	LastSelectedPos = NULL;
	BOOL		bAllowCloseWindow = FALSE;

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
	InvalidateCachedPreviewSummary();

	// Accumulate a uid list so we can do a block IMAP command.
	//
	pos = m_Sums.GetHeadPosition();

	i = 0;
	nCount = 0;
	int		 iDeleted = 0;

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* Sum = m_Sums.GetNext(NextPos);

		// Keep searching for SingleSum if we haven't hit it yet
		if (SingleSum && SingleSum != Sum)
			continue;
			
		if (m_bHideDeletedIMAPMsgs && Sum && Sum->IsIMAPDeleted())
		{
			++iDeleted;
			continue;
		}

		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( !SingleSum && View && (View->m_SumListBox.GetSel(i - iDeleted) <= 0) )
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
		if (bQueueAction)
		{
			// We are using the local/cache/online technique: create an action object for
			// this operation and add it to the queue.
			CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
			if (pAccount)
			{
				CImapUnDeleteMsgAction	*pUnDeleteAction = DEBUG_NEW CImapUnDeleteMsgAction(m_pImapMailbox->GetAccountID(),
																							(const char*)m_pImapMailbox->GetImapName(),
																							m_pImapMailbox->GetDelimiter(),
																							sUidlist.GetBuffer(0));
				if (pUnDeleteAction)
				{
					pAccount->QueueAction(pUnDeleteAction);
				}
			}
		}
		else
		{
			hResult = m_pImapMailbox->UnDeleteMessagesFromServer((LPCSTR)sUidlist, TRUE);
		}
	}
	else
	{
		if (bQueueAction)
		{
			// We are using the local/cache/online technique: create an action object for
			// this operation and add it to the queue.
			CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
			if (pAccount)
			{
				CImapDeleteMsgAction	*pDeleteAction = DEBUG_NEW CImapDeleteMsgAction(m_pImapMailbox->GetAccountID(),
																						(const char*)m_pImapMailbox->GetImapName(),
																						m_pImapMailbox->GetDelimiter(),
																						sUidlist.GetBuffer(0));
				if (pDeleteAction)
				{
					pAccount->QueueAction(pDeleteAction);
				}
			}
		}
		else
		{
			// We are using the old peform immediately technique: perform the action online now.
			hResult = m_pImapMailbox->DeleteMessagesFromServer ( (LPCSTR) sUidlist, FALSE, TRUE);
		}
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

	// Make sure preview updates
	InvalidateCachedPreviewSummary(NULL, true);

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
	int	iDeleted = 0;

	szUidList.Empty();

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* pSum = m_Sums.GetNext(NextPos);

		if (m_bHideDeletedIMAPMsgs && pSum && pSum->IsIMAPDeleted())
		{
			++iDeleted;
			continue;
		}

		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( pSum && (pView->m_SumListBox.GetSel(i - iDeleted) > 0) )
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
	int		 iDeleted = 0;
	szUidToReadList.Empty();
	szUidToUnReadList.Empty();

	for (NextPos = pos; pos; pos = NextPos, i++)
	{
		CSummary* pSum = m_Sums.GetNext(NextPos);

		if (m_bHideDeletedIMAPMsgs && pSum && pSum->IsIMAPDeleted())
		{
			++iDeleted;
			continue;
		}

		// If this summary isn't selected (non-SingleSum case) then continue on
		if ( pSum && (pView->m_SumListBox.GetSel(i - iDeleted) > 0) )
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

	if (MboxFile.Open (GetMBFileName(), O_RDWR | O_CREAT | O_APPEND) < 0)
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
							InvalidateCachedPreviewSummary(Sum, true);

							// Remove any attachments before we delete the summary.
							//
							ImapRemoveAttachments(Sum);

							// close the message window
							// (this used to be done in ImapRemoveAttachments() -jdboyd 8/16/99
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
			CopySum = DEBUG_NEW CSummary;
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

//
//	CTocDoc::ImapFetchMessages()
//
//	Iterates through the selected messages finding any that need to be fetched (or unfetched)
//	based on the specified flags.  Calls CImapMailbox::FetchMessages() to do the actual work.
//
int	CTocDoc::ImapFetchMessages(BOOL bDownloadAttachments /*=TRUE*/,
							   BOOL bOnlyIfNotDownloaded /*=TRUE*/,
							   BOOL bClearCacheOnly /*=FALSE*/,
							   BOOL bInvalidateCachedPreviewSums /*=TRUE*/,
							   BOOL bDoItNow /*=FALSE*/)
{
	if (!m_pImapMailbox)
	{
		ASSERT(0);
		return 0;
	}

	// Get the view so we can find selected messages.
	CTocView	*pView = GetView();
	if (!pView)
	{
		ASSERT(0);
		return 0;
	}

	CTocFrame	*pTocFrame = (CTocFrame*)pView->GetParentFrame();
	if (!pTocFrame)
	{
		ASSERT(0);
		return 0;
	}

	// Just in case the caller doesn't indicate this, clearing the cache is always done immediately
	// since there is no online component to it.
	if (bClearCacheOnly)	bDoItNow = TRUE;

	CTocFrame::SummaryArrayT	selSumsArray;
	pTocFrame->GetSelectedSummaries(selSumsArray);

	int		nNumSelectedSums = selSumsArray.GetSize();

	// Put up a wait cursor
	CWaitCursor	 wc;

	CTempSumList	 sumList;
	CString			 strUid;
	CString			 strUidList;
	int				 i = 0;

	for (i = 0; i < nNumSelectedSums; i++)
	{
		CSummary	*pSum = selSumsArray[i];
		
		// Already downloaded?
		if (!bClearCacheOnly && bOnlyIfNotDownloaded)
		{
			// We're already downloaded if:
			// * We want attachments and have everything (including attachments)
			// * We don't want attachments and we have the message body
			if ((bDownloadAttachments && pSum->IsIMAPFullyDownloaded()) ||
				(!bDownloadAttachments && pSum->IsIMAPMessageBodyDownloaded()))
			{
				continue;
			}
		}

		sumList.AddTail(pSum);

		if (strUidList.IsEmpty())
		{
			strUidList.Format("%lu", pSum->m_Hash);
		}
		else
		{
			strUid.Format (",%lu", pSum->m_Hash);
			strUidList += strUid;
		}
	}

	// Must have found some.
	if (sumList.GetCount() == 0)
	{
		// Return the number of selected summaries, because we checked
		// them all and none needed downloading
		return nNumSelectedSums;
	}

	int		 iConnectionState = GetConnectionState(bDoAllowOffline);
	if (iConnectionState == iStateStayOfflineDisallow)
	{
		return 0;
	}

	bool	 bIsOffline = (iConnectionState == iStateStayOfflineAllow);
	int		 nNumIMAPSummariesDownloaded = 0;

	if (bIsOffline || !bDoItNow)
	{
		// Queue the action if 1) we are offline so we can't possibly fetch the data now or
		// 2) we don't care if it happens immediately (queuing and waiting for the replay is currently
		// too painful for something like previewing a message or opening a message to view it).
		QueueImapFetch(strUidList,
					   bDownloadAttachments,
					   bOnlyIfNotDownloaded,
					   bClearCacheOnly,
					   bInvalidateCachedPreviewSums);
	}
	else
	{
		// OK, we want this to happen immediately.
		nNumIMAPSummariesDownloaded = ImapFetchMessagesFromServer(&sumList,
																  bDownloadAttachments,
																  bOnlyIfNotDownloaded,
																  bClearCacheOnly,
																  bInvalidateCachedPreviewSums);
	}

	return nNumIMAPSummariesDownloaded;
}

bool CTocDoc::QueueImapFetch(CString strUidList,
							BOOL bDownloadAttachments,
							BOOL bOnlyIfNotDownloaded,
							BOOL bClearCacheOnly,
							BOOL bInvalidateCachedPreviewSums)
{
	CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
	if (pAccount)
	{
		CImapDownloadAction		*pImapDownloadAction = DEBUG_NEW CImapDownloadAction(m_pImapMailbox->GetAccountID(),
																					(char*)m_pImapMailbox->GetImapName(),
																					m_pImapMailbox->GetDelimiter(),
																					strUidList,
																					bDownloadAttachments,
																					bOnlyIfNotDownloaded,
																					bClearCacheOnly,
																					bInvalidateCachedPreviewSums);
		if (pImapDownloadAction)
		{
			pAccount->QueueAction(pImapDownloadAction);
			return true;
		}
	}
	return false;
}

int	CTocDoc::ImapFetchMessagesFromServer(CSumList *pSumList,
										 BOOL bDownloadAttachments,
										 BOOL bOnlyIfNotDownloaded,
										 BOOL bClearCacheOnly,
										 BOOL bInvalidateCachedPreviewSums)
{
	if (!pSumList || pSumList->GetCount() == 0)
	{
		return 0;
	}

	BOOL		bResult = FALSE;
	BOOL		bWasInProgress = FALSE;
	int			nCount = pSumList->GetCount();

	// We can put up a progress window here!
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

	// Now loop throught each summary individually.
	//
	BOOL bModified = FALSE;

	int nNumSummariesProcessed = 0;

	POSITION		 pos = NULL;
	POSITION		 posNext = NULL;

	pos = pSumList->GetHeadPosition();
	for (posNext = pos; pos; pos = posNext)
	{
		CSummary	*pSum = pSumList->GetNext(posNext);

		if (!pSum)	continue;

		nNumSummariesProcessed++;

		// Purge from cache if:
		// * We are allowing mucking with the TOC (i.e., this is not a message view or preview) AND
		// * We don't care whether or not it's been downloaded (hence !bOnlyIfNotDownloaded) OR
		// * The message body has been downloaded (don't check to see if it's fully downloaded
		//   because we want to delete any attachment stubs)
		if (!bOnlyIfNotDownloaded || pSum->IsIMAPMessageBodyDownloaded())
		{
			// Delete any attachments.
			//
			ImapRemoveAttachments(pSum);

			// Mark the summary as not downloaded:
			//
			pSum->m_Imflags |= IMFLAGS_NOT_DOWNLOADED;

			// No downloaded attachments.
			//
			pSum->m_nUndownloadedAttachments = 0;

			// Signal that we may need compacting.
			// Must set a non-zero value for pTocDoc->m_delSpace.
			//
			m_DelSpace += pSum->m_Length;

			// Can now reset these:
			//
			pSum->SetOffset ( 0 );
			pSum->m_Length = 0;

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
			BOOL b = SUCCEEDED (m_pImapMailbox->DownloadSingleMessage (this, pSum, bDownloadAttachments));
			bResult = bResult || b;
		}

		nCount--;

		// Update progress bar.
		if ( ::IsMainThreadMT() )
		{
			//	Check for escape key press
			if ( EscapePressed() )
				break;
			
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

		if (!bResult)
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

	CTocView	*pView = GetView();

	// Update the "Compact" button in case we purged stuff from the MBX file.
	//
	if (pView && m_NeedsCompact)
		pView->SetCompact();

	// Update these.
	if (bResult || bModified)
		SetModifiedFlag();

	if (bInvalidateCachedPreviewSums)
	{
		// Refresh preview:
		//
		// Previously the code kept track of whether or not "the single" previewable
		// summary was one of the ones that we fetched. Since there no longer is
		// just one previewable summary we'll just invalidate the entire cache
		// and redisplay.
		//
		// If this should prove flickery or otherwise annoying, then we'll have
		// to somehow add a check to see if one of the messages downloaded was being
		// previewed before invalidating everything.
		InvalidateCachedPreviewSummary(NULL, true);
	}

	pSumList->RemoveAll();

	return nNumSummariesProcessed;
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

		if( pSum && pSum->IsNotIMAPDownloadedAtAll() )
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

// Loop through the message "stub" and delete all attachments and embedded objects.
// Note: This works for BOTH POP and IMAP mailboxes!!!
//
//

BOOL CTocDoc::ImapRemoveAttachments(CSummary *pSum)
{
	CMessageDoc* pMsgDoc = NULL;

	if (!pSum)
	{
		return FALSE;
	}

	// If this is an IMAP mailbox, and the message is not downloaded, then nothing to do.
	//
	if (IsImapToc() && pSum->IsNotIMAPDownloadedAtAll())
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
	if (IsImapToc())
	{
		// MUST be:
		if (m_pImapMailbox)
		{
			GetAttachDirPath(m_pImapMailbox->GetDirname(), AttachDir);
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



		//this should be done only if we are not transferring to a local mailbox. 
		// get rid of embedded objects
		if(pSum && pSum->CanDeleteEmbedded())
		{
			char* buf = pMsgDoc->GetFullMessage();		// gots ta get da URIMap
			CString Paths;
			pMsgDoc->m_QCMessage.GetEmbeddedObjectPaths( Paths );
			DeleteEmbeddedObjects( Paths );
			delete [] buf;
			pSum->SetDeleteEmbedded();
		}
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

// Copy the contents of summary from one mailbox to another and add the summary to 
// the target toc.
//
BOOL CTocDoc::ImapMoveContents(CTocDoc* pTargetToc, CSummary *pSum)
{
	JJFile TargetMBox;
	JJFile SrcMBox;

	if (!pTargetToc || !pSum)
		return FALSE;

	//
	if (this == pTargetToc)
		return TRUE;

	if (FAILED(TargetMBox.Open(pTargetToc->GetMBFileName(), O_RDWR | O_CREAT | O_APPEND)) ||
		FAILED(SrcMBox.Open(GetMBFileName(), O_RDONLY)))
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




// ========================== Internal non-class function ====================/

//
//	CanCloseSummaryFrame()
//
//	Returns TRUE if the summary's frame can be closed.
//
//	This should be a CSummary method, but honestly this module contains the vast
//	majority of the places that use this particular code so, eh, I'll just make it
//	a local for now.
//
BOOL CanCloseSummaryFrame(CSummary *pSum)
{
	if (pSum)
	{
		CMessageFrame	*pMsgFrame = pSum->m_FrameWnd;
		if (pMsgFrame)
		{
			return (pMsgFrame->GetActiveDocument()->CanCloseFrame(pMsgFrame));
		}
	}
	return TRUE;
}


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
			CSumList &	listTocSums = pTocDoc->GetSumList();
			if ( (pSum = listTocSums.GetByUid(uid)) != NULL )
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
	if ( FAILED (TargetMBoxFile.Open (pTargetToc->GetMBFileName(), O_RDWR | O_CREAT | O_APPEND) ) )
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
				CSumList &	listSums = pTocDoc->GetSumList();
				pNextSum = listSums.GetAt(LastSelectedPos);
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

	CSumList &	listSums = pTocDoc->GetSumList();
	pos = listSums.GetHeadPosition();

	for (NextPos = pos; pos; pos = NextPos)
	{
		CSummary* Sum = listSums.GetNext(NextPos);

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
	if (!pImapMailbox)
	{
		return FALSE;
	}

	if (bQueueAction)
	{
		// We are using the local/cache/online technique: create an action object for
		// this operation and add it to the queue.
		CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(pImapMailbox->GetAccountID());
		if (pAccount)
		{
			CImapChangeMsgStatusAction	*pChangeMsgStatusAction = new CImapChangeMsgStatusAction(pImapMailbox->GetAccountID(),
																									(const char*)pImapMailbox->GetImapName(),
																									pImapMailbox->GetDelimiter(),
																									szUidList.GetBuffer(0),
																									State,
																									!bUnset);
			if (pChangeMsgStatusAction)
			{
				pAccount->QueueAction(pChangeMsgStatusAction);
			}
		}
		return TRUE;
	}
	else
	{
		return pImapMailbox->SetRemoteState(szUidList, State, bUnset);
	}
}
	

#endif // IMAP4

