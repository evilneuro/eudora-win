// QCMailboxCommand.cpp: implementation of the QCMailboxCommand class.
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "QCUtils.h"

#include "eudora.h"
#include "fileutil.h"
#include "guiutils.h"
#include "progress.h"
#include "QCCommandActions.h"
#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "tocdoc.h"
#include "tocview.h"
#include "ImapMailbox.h"
#include "SearchManager.h"


#include "DebugNewHelpers.h"


IMPLEMENT_DYNAMIC(QCMailboxCommand, QCCommandObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCMailboxCommand::QCMailboxCommand(QCMailboxDirector* pDirector, LPCSTR szName,
									LPCSTR szPathname, MailboxType theType /*= MBT_REGULAR*/,
									UnreadStatusType theStatus /*= US_UNKNOWN*/) :
	QCCommandObject( pDirector )
{
	m_szName = szName;
	m_szPathname = szPathname;
	m_theType = theType;
	m_theStatus = theStatus;
}

QCMailboxCommand::~QCMailboxCommand()
{
	while (m_theChildList.IsEmpty() == FALSE)
		delete (QCMailboxCommand*)m_theChildList.RemoveTail();
}

void QCMailboxCommand::Execute(COMMAND_ACTION_TYPE theAction, void* pData /*= NULL*/)
{
	CTocDoc*			pTocDoc;
	CTocDoc*			pSrcToc;
	CSummary*			pSrcSummary;
	UnreadStatusType	theStatus;

	if (theAction == CA_COMPACT)
	{
		if (pData != NULL)
			ASSERT_KINDOF(CTocDoc, (CTocDoc*)pData);

		if (CompactMailbox((CTocDoc*)pData) == CRT_DRASTIC_ERROR)
			return;		
	}

	if (theAction == CA_UPDATE_STATUS)
	{
		theStatus = (UnreadStatusType)(ULONG)pData;
		if (m_theStatus == theStatus)
			return;
		m_theStatus = theStatus;
	}

	NotifyDirector(theAction, pData);	

	if (theAction == CA_OPEN)
	{
		// Make sure we have a wait cursor in case this takes a while
		CCursor		waitCursor;
		
		pTocDoc = GetToc(m_szPathname, m_szName);
		if (pTocDoc)
			pTocDoc->Display();
	}	
	else if (theAction == CA_TRANSFER_TO || theAction == CA_COPY)
	{
		if (pData == NULL)
		{
			ASSERT(0);
			return;
		}

		pTocDoc = GetToc(m_szPathname);
		
		if (((CObject*)pData)->IsKindOf(RUNTIME_CLASS(CTocDoc)))
		{
			pSrcToc = ( CTocDoc* ) pData;
			pSrcSummary = NULL;
		}
		else if (((CObject*)pData)->IsKindOf(RUNTIME_CLASS(CSummary)))
		{
			pSrcSummary = (CSummary*)pData;
			pSrcToc = pSrcSummary->m_TheToc;		
		}
		else
		{
			ASSERT(0);
			return;
		}

		if (!pSrcToc || !pTocDoc)
		{
			ASSERT(0);
			return;
		}
		
		ASSERT_KINDOF(CTocDoc, pSrcToc);

		if (theAction == CA_TRANSFER_TO && m_theType == MBT_TRASH)
			pSrcToc->Xfer(pTocDoc, pSrcSummary);
		else
			pSrcToc->Xfer(pTocDoc, pSrcSummary, TRUE, theAction == CA_COPY);

		if (theAction == CA_TRANSFER_TO)
		{
			//
			// Okay, big assumption time.  It is assumed here that all
			// Transfer commands that get routed through here happen
			// as a result of an explicit user action.  This is an
			// important distinction because it affects whether or not
			// the next message to be previewed in the TOC window is
			// to automatically marked as read or not.
			//
			// All we do here is notify the source TocDoc that the
			// contents of the TOC changed due to user action.  The
			// idea is for the TOC to then pretend that it got an
			// LBN_SELCHANGE notification.
			//
			pSrcToc->MailboxChangedByHuman();

			// OK, one more thing.  If the user initiated the transfer
			// maybe reset the contents of the view to account for any
			// IMAP messages that are now marked as deleted.
			CTocView	*pTocView = pSrcToc->GetView();
			if (pTocView)
			{
				pTocView->MaybeResetContents();
			}
		}
	}
}

//
//	QCMailboxCommand::IsJunk()
//
//	Returns whether or not this command represents the system Junk mailbox
//	or the Junk mailbox for an IMAP account.
//
BOOL QCMailboxCommand::IsJunk()
{
	if (GetType() == MBT_JUNK)
	{
		return TRUE;
	}
	if (GetType() == MBT_IMAP_MAILBOX)
	{
		CTocDoc		*pTocDoc = GetToc(m_szPathname);
		if (pTocDoc)
		{
			return (pTocDoc->IsJunk() == true);
		}
	}
	return FALSE;
}

//
//	QCMailboxCommand::IsTrash()
//
//	Returns whether or not this command represents the system Trash mailbox
//	or the Trash mailbox for an IMAP account.
//
BOOL QCMailboxCommand::IsTrash()
{
	if (GetType() == MBT_TRASH)
	{
		return TRUE;
	}
	if (GetType() == MBT_IMAP_MAILBOX)
	{
		return ::IsTrash(GetName());
	}
	return FALSE;
}

BOOL QCMailboxCommand::IsEmpty()
{
	if (m_theType == MBT_FOLDER)
		return m_theChildList.IsEmpty();

	// Check if mailbox has any messages
	if (::FileExistsMT(m_szPathname))
	{
		CTocDoc* pTocDoc = GetToc(m_szPathname);
		
		if (pTocDoc && pTocDoc->NumSums())
			return FALSE;
	}

	return TRUE;
}			

void QCMailboxCommand::SetPathname(LPCSTR szPathname)
{
	if ( (m_theType == MBT_REGULAR) || (m_theType == MBT_IMAP_MAILBOX) )
	{
		// Special notification for Search Manager. May want to
		// switch to a more general scheme at some later point.
		SearchManager::Instance()->NotifyMailboxPathNameChange(m_szPathname, szPathname);
	}

	m_szPathname = szPathname;
}

// CompactOneMailbox
//
// Compacts only one mailbox 
//
COMPACT_RESULT_TYPE QCMailboxCommand::CompactMailbox(CTocDoc* pTocDoc, bool bUserRequested)
{
	COMPACT_RESULT_TYPE	theStatus;
    char				szFullPath[_MAX_PATH + 1];
	char				buf[_MAX_PATH + 1];
	CSummary*			pSum;
	POSITION			pos;
	char*				ptr;
	long				lLastOffset;
	int					i;
	long				lOffset;
	JJFile				theMBoxFile;
	JJFile				theTempFile;
	BOOL				bDoClose;
	char				szLogBuf[512];
	int					assertNumSums;		// Belt and suspenders checking

	bDoClose = FALSE; // Default to not closing -- only close if we load the TOC

	theStatus = CRT_MILD_ERROR;

	if( !pTocDoc )
    {
		// Scot Manjourides, 4/14/98
		//
		// Big fix #3327, 4.0.1 (b327): Special-Compact Mailboxes closes any open mailboxes besides In, Out, and Trash.
		//
		// Problem: Some mailboxes are open when we compact 'em, we need to leave
		//          them open. In, Out, Trash are special because they are never
		//          closed (always in mem).
		//
		// Solution: We have to know if we load the TOC or the TOC was already
		//           in memory. The hack answer is to make two calls to GetToc. One
		//           asking for in-memory TOC only. This will let us determine
		//           if the TOC was loaded by our hands. If we do load it, we must close
		//           it when done.
		//
		// A better solution would be a call that told us if the TOC is in memory or not.

		// First try to get the TOC from memory (this means it's open)
		if (!(pTocDoc = GetToc(m_szPathname, NULL, TRUE, TRUE)))
		{
			// Okay, so it either failed badly or it's not in memory
			// Try again, but allow it to be loaded (this means it's not open)
			if (!(pTocDoc = GetToc(m_szPathname, NULL, TRUE, FALSE))) // try not in mem
			{
				// Both calls failed, so something is really wrong
				goto done; // returns mild error
			}
			else
			{
				// We got the TOC, but it wasn't in memory, so be sure to close it
				//  when we are done.
				bDoClose = TRUE;
			}
		}
    }

//	Convince myself that the world is good
	ASSERT ( pTocDoc != NULL );
//	Make sure that the toc and the mailbox refer to the same thing
	ASSERT ( pTocDoc->GetMBFileName().CompareNoCase (this->GetPathname ()) == 0);
	assertNumSums = pTocDoc->NumSums ();

	// if it's in or out, do backups
	// we do backups even for 0 so that setting backups to 0 deletes the files, but -1 will leave them
	{
		int nBackups = GetIniLong(IDS_INI_INOUT_BACKUPS);
		if ((pTocDoc->m_Type==MBT_IN || pTocDoc->m_Type==MBT_OUT) && nBackups >= 0)
		{
			if (FAILED(CascadeBackupFile(pTocDoc->GetMBFileName(), nBackups))) goto done;	// mild error
			if (FAILED(CascadeBackupFile(pTocDoc->GetFileName(), nBackups))) goto done;	// mild error
		}
	}

	// If this is the Junk mailbox, trim old junk before compacting.
	if (bUserRequested && (pTocDoc->m_Type == MBT_JUNK))
	{
		TrimJunk(true/*bUserRequested*/);
	}

	if( !pTocDoc->m_NeedsCompact )
	{
		theStatus = CRT_COMPACTED;
		goto done;
	}

	if( !pTocDoc->Read() )
	{
		// returns mild error
		goto done;
	}

	strcpy( buf, m_szPathname );
	ptr = strrchr(buf, '.');
	if (!ptr)
		goto done;
	*ptr = 0;
    ptr = strrchr(buf, SLASH);
	
	if (!ptr)
	{
		// returns mild error ?
		goto done;
	}
	
	sprintf( szFullPath, "%s%s.tmp", (const char*)TempDir, ptr + 1 );
	
	if( FAILED(theMBoxFile.Open( pTocDoc->GetMBFileName(), O_RDWR )) )
	{
		// returns mild error
		goto done;
	}

	// Log the fact that we are compacting a mailbox.  Yeah, the call to Progress()
	// below will log just the mailbox name (no path) but since there is some
	// suspicion of compaction causing problems I'll go a little overboard.
	sprintf(szLogBuf, "Compacting Mailbox %s", pTocDoc->GetMBFileName());
	PutDebugLog(DEBUG_MASK_PROG, szLogBuf);

	CSumList &		listSums = pTocDoc->GetSumList();
	
	// Do some sanity checking.

	// Count the actual number of summaries.
	pos = listSums.GetHeadPosition();
	for(i = 0; pos; ++i)
	{
		listSums.GetNext( pos );
	}

	// Make sure the number of summaries found matches the official count.
	if (i != pTocDoc->NumSums())
	{
		// This clearly should not happen but if it does, do not proceed with
		// the compaction for fear that the user could lose messages.
		ASSERT(0);
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			sprintf(szLogBuf, "Compacting error: NumSums=%d, sums found=%d", pTocDoc->NumSums(), i);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		}
		theStatus = CRT_DRASTIC_ERROR;
		goto done;
	}
	
	// Let's see if compaction can be reduced to truncating the file
	
	pos = listSums.GetHeadPosition();
	
	for( lLastOffset = 0L, i = 0; pos; i++ )
	{
		pSum = listSums.GetNext( pos );
		if( pSum->m_Offset != lLastOffset )
		{
			break;
		}
		lLastOffset += pSum->m_Length;
	}
	
	if( i == pTocDoc->NumSums() )
	{
		if( SUCCEEDED(theMBoxFile.ChangeSize( lLastOffset )) )
		{
			// Need to close the .mbx file before writing the .toc because the .toc header includes the
			// size of the .mbx file, and Win 95/98 doesn't updates file sizes until the file is closed
			theMBoxFile.Close();

			pTocDoc->m_NeedsCompact = FALSE;
			pTocDoc->ClearDelSpace();
			pTocDoc->WriteHeader();
			pTocDoc->ClearUndo();
			theStatus = CRT_COMPACTED;
		}
		
		goto done;
	}

	Progress( 0, pTocDoc->Name(), pTocDoc->NumSums() );

	if( FAILED(theTempFile.Open( szFullPath, O_WRONLY | O_CREAT | O_TRUNC)) )
	{
		// Log the compaction failure.
		sprintf(szLogBuf, "Compaction error: couldn't open temp file");
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		theStatus = CRT_DRASTIC_ERROR;
		goto done;
    }

	pos = listSums.GetHeadPosition();
	
	for( i = 0; pos; i++ )
	{
		pSum = listSums.GetNext(pos);
		
		theTempFile.Tell(&lOffset);
		ASSERT(lOffset >= 0);
		if (lOffset < 0)
			break;

		pSum->m_Temp = pSum->m_Offset;
		
		if( FAILED(theMBoxFile.JJBlockMove(	pSum->m_Offset, 
											pSum->m_Length, 
											&theTempFile ) ) )
		{
			break;
		}

		pSum->m_Offset = lOffset;
		
		ProgressAdd( 1 );

		// If the user presses Esc, then cancel this compaction
		
		if( EscapePressed() )
		{
			// Log the compaction failure.
			sprintf(szLogBuf, "Compaction error: user cancelled compaction");
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

			theStatus = CRT_DRASTIC_ERROR;
			break;
		}
	}

	theMBoxFile.Close();

	// Tell the SearchManager to close this mailbox if it has it open
	// so that renaming the mailbox will work.
	SearchManager::Instance()->NotifyTemporarilyCloseMailbox( pTocDoc->GetMBFileName() );

	// If we didn't get all the way through, something went wrong so clean up
	if (i < pTocDoc->NumSums() ||
		FAILED(theTempFile.Rename(pTocDoc->GetMBFileName())))
	{
		// Log the fact that we didn't get all the way through.
		ASSERT(0);
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			sprintf(szLogBuf, "Compaction error: only moved %d of %d sums", i, pTocDoc->NumSums());
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);
		}

		pos = listSums.GetHeadPosition();

		for (; pos && i; i--)
		{
			pSum = listSums.GetNext( pos );
			pSum->m_Offset = pSum->m_Temp;
		}

		theTempFile.Delete();
		goto done;
	}

	pTocDoc->m_NeedsCompact = FALSE;
	pTocDoc->ClearDelSpace();
	pTocDoc->Write();
	pTocDoc->ClearUndo();
	theStatus = CRT_COMPACTED;

//	Do we still have the same number of messages that we started with?
	ASSERT ( assertNumSums == pTocDoc->NumSums ());

done:
	theTempFile.Close();
	theMBoxFile.Close();

	if( bDoClose && ( pTocDoc != NULL ) )
	{
//		if( pTocDoc->GetView() )
//		{
//			pTocDoc->GetView()->SetCompact();
//		}
		
		if (pTocDoc->m_bAutoDelete)
		{
			pTocDoc->OnCloseDocument();
		}
    }

    return theStatus;
}

UINT QCMailboxCommand::GetFlyByID(COMMAND_ACTION_TYPE theAction)
{
	switch (theAction)
	{
	case CA_TRANSFER_TO:	
		switch (m_theType)
		{
		case MBT_IN:	return ID_TRANSFER_IN;
		case MBT_OUT:	return ID_TRANSFER_OUT;
		case MBT_TRASH:	return ID_TRANSFER_TRASH;
		case MBT_JUNK:	return ID_TRANSFER_JUNK;
		default:		return ID_TRANSFER_USER;
		}

	case CA_OPEN:
		switch (m_theType)
		{
		case MBT_IN:	return ID_MAILBOX_IN;
		case MBT_OUT:	return ID_MAILBOX_OUT;
		case MBT_TRASH:	return ID_MAILBOX_TRASH;
		case MBT_JUNK:	return ID_MAILBOX_JUNK;
		default:		return ID_MAILBOX_USER;
		}

	case CA_TRANSFER_NEW:	return ID_TRANSFER_NEW_MBOX_IN_ROOT;
	case CA_NEW_MAILBOX:	return ID_NEW_MAILBOX_IN_ROOT;
	}

	return 0;
}


CString	QCMailboxCommand::GetToolTip(COMMAND_ACTION_TYPE theAction)
{
	UINT StringID = 0;

	switch (theAction)
	{
	case CA_TRANSFER_TO:	StringID = IDS_USERDEF_TOOLTIP_TRANSFER;	break;
	case CA_OPEN:			StringID = IDS_USERDEF_TOOLTIP_MAILBOX;	break;
	}

	CString szToolTip;

	if (StringID)
	{
		szToolTip.LoadString(StringID);
		szToolTip += " ";
		szToolTip += m_szName;
	}

	return szToolTip;
}
