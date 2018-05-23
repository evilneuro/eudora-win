// QCImapMailboxCommand.cpp: implementation of the QCMailboxCommand class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "rs.h"
#include "eudora.h"
#include "fileutil.h"
#include "guiutils.h"
#include "progress.h"
#include "QCCommandActions.h"
#include "QCMailboxDirector.h"
#include "QCMailboxCommand.h"
#include "QCImapMailboxCommand.h"
#include "tocdoc.h"
#include "tocview.h"
#include "tocframe.h"


// IMAP:
#include "imapmlst.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imapfol.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CString			EudoraDir;
extern QCMailboxDirector	g_theMailboxDirector;


// Internal 



//========================================================//


IMPLEMENT_DYNAMIC(QCImapMailboxCommand, QCMailboxCommand)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCImapMailboxCommand::QCImapMailboxCommand(
QCMailboxDirector*	pDirector,
LPCSTR				szName,
LPCSTR				szImapName,
LPCSTR				szPathname,	// For IMAP, this is the pathname to the directory containing the mailbox files.
MailboxType			theType,
UnreadStatusType	theStatus,
ACCOUNT_ID			AccountID,
BOOL				bNoselect /* = TRUE */,
BOOL				bMarked /* = FALSE */,
BOOL				bUnmarked /* = FALSE */,
BOOL				bNoInferiors) : QCMailboxCommand( pDirector, szName, szPathname, theType, theStatus  )
{
	m_AccountID		= AccountID;

	// szImapName can be NULL.
	if (szImapName)		
		m_szImapName	= szImapName;	
	else
		m_szImapName.Empty();

	m_bNoselect		= bNoselect;
	m_bMarked		= bMarked;
	m_bUnmarked		= bUnmarked;
	m_Delimiter		= 0;
	m_bNoInferiors  = bNoInferiors;
	m_bReadOnly		= FALSE;		// Until we know for sure.

	m_Valid		= TRUE;

	m_bMboxShownAsSubMenu = FALSE;
	m_bXferShownAsSubMenu = FALSE;
}


QCImapMailboxCommand::~QCImapMailboxCommand()
{
}



BOOL QCImapMailboxCommand::	ShouldShowAsSubMenu ()
{
	 return ( (MBT_IMAP_ACCOUNT == GetType() ) ||
		( CanHaveChildren() &&
				( GetIniShort(IDS_INI_IMAP_LEAFMENU) || ((GetChildList().GetCount() > 0)) ) ) );
}




// Overrides parent's.
BOOL QCImapMailboxCommand::IsEmpty()
{
	BOOL		bIsEmpty = TRUE;
	CTocDoc*	pTocDoc;

	// FIrst, check if the child list is empty.;
	bIsEmpty = GetChildList().IsEmpty();

	// If there's no child list, check for mailboxes if this is a
	// selectable mailbox.
	if (bIsEmpty && ( GetType() == MBT_IMAP_MAILBOX ) && !IsNoSelect())
	{
		// Check if mailbox has any messages
		if( FileExistsMT ( GetPathname() ) )
		{
			pTocDoc = GetToc( GetPathname() );
		
			if( ( pTocDoc != NULL ) && ( pTocDoc->NumSums() != 0 ) )
			{
				bIsEmpty = FALSE;
			}
		}
	}

	return bIsEmpty;;
}


// NOTES
// For CA_IMAP_REFRESH and CA_IMAP_RESYNC, pData is a pointer to a 
// BOOL to receive the value of "ViewNeedsRefreshed".
// END NOTES
void	QCImapMailboxCommand::Execute(
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	if( theAction == CA_IMAP_REFRESH )
	{
		CString Pathname;
		BOOL	bRecurse;

		// Do we have to recurse through mailboxes? On input, pData poinmts to a 
		// BOOL indicating this. On output, set the value of *pData to indicate if
		// we need to update the mbox tree control.
		bRecurse = FALSE;
		if (pData)
		{
			bRecurse = *(BOOL *)pData;

			// On return, we have to tell caller if we succeeded and if
			// it needs to refresh the mailbox tree - Assume NOT.
			//
			*((BOOL *)pData) = FALSE;
		}

		// Create a CImapMlistMgr object to fetch mailbox lists from IMAP and build the temporary
		// mailboxes.lst files.

		GetObjectDirectory (Pathname);

		CImapMlistMgr imapMlistMgr (GetAccountID(),
									Pathname, 
									GetImapName(),
									GetDelimiter(),
									MBTTypeToImapType (GetType()));

		// Get the new mailboxes and, in the process, mark the mailboxes that we know
		// exist on the server as valid.
		if (imapMlistMgr.UpdateMailboxLists(bRecurse))
		{
			if (imapMlistMgr.NeedsResync ())
			{
				// Resynchronize mailboxes.
			}

			// pData is a pointer to a BOOL for receiving whether we  need to refresh the mailbox list.
			if (pData)
			{
				BOOL *b = (BOOL *)pData;
				if (b)
				*b = TRUE;
			}

			// This will update the mailbox tree.
			NotifyDirector(theAction, NULL);
		}
	}
	else if ( theAction == CA_IMAP_RESYNC )
	{
		// Can only resync mailboxes at this time. Ignore accounts and namespaces. 
		switch (GetType())
		{
			case MBT_IMAP_MAILBOX:
				break;

			default:
				return;
		} 

		// Always do a re-sync via a TOC, whether open or not.
		CTocDoc*			pTocDoc;

		pTocDoc = GetToc( GetPathname(), GetName() );
		
		if( pTocDoc != NULL )
		{
			if (pTocDoc->m_pImapMailbox)
			{
				// Get the number of summaries before update.
				long nCount = pTocDoc->NumSums();

				// This is equivalent to a check mail with a forced re-sync.
				//
				BOOL bUpdateSucceeded = 
					SUCCEEDED (pTocDoc->m_pImapMailbox->DoManualResync ( pTocDoc, TRUE, FALSE ) );

				// Was there a change?
				if ( bUpdateSucceeded)
				{
					if ( nCount != pTocDoc->NumSums() )
					{
						// Only if it's not opened.
						if (!pTocDoc->GetView())
						{
							pTocDoc->Display();
						}

						// AlertDialog(IDD_NEW_MAIL);
					}
					else
					{
						// AlertDialog(IDD_NO_NEW_MAIL);
					}
				}
			}
		}

		return;
	}
	else if ( theAction == CA_IMAP_PROPERTIES )
	{
		// Modification of account from here has been removed.
		
		return;
	}
	else if ( theAction == CA_OPEN )
	{
		// Do an Open.
		CTocDoc*			pTocDoc;

		pTocDoc = GetToc( GetPathname(), GetName() );
		
		if( pTocDoc != NULL )
		{
			pTocDoc->Display();
		}

		return;
	}
	else if ( theAction == CA_DELETE_MESSAGE || theAction == CA_UNDELETE_MESSAGE )
	{
		// Must have a TOC.
		CTocDoc*			pTocDoc;

		pTocDoc = GetToc( GetPathname(), GetName() );
		
		if( NULL == pTocDoc )
		{
			ASSERT (0);
			return;
		}

		CSummary *pSummary = NULL;

		// 
		// If no data, delete selected summaries. If data, it MUST be a summary.
		// 
		if ( pData )
		{
			//
			if( ! ( ( CObject* ) pData ) ->IsKindOf( RUNTIME_CLASS( CSummary ) ) )
			{
				ASSERT (0);
				return;
			}
				
			pSummary = ( CSummary* ) pData;
		}

		//
		// If the server was busy, we may have to repost this operation:
		//
		HRESULT hResult = E_FAIL;
		CTocView *pTocView = NULL;
		CTocFrame* pTocFrame = NULL;

		pTocView = pTocDoc->GetView ();
		if (pTocView)
			pTocFrame = (CTocFrame *) pTocView->GetParentFrame();

		//
		// Delete of Undelete??
		//
		if ( theAction == CA_DELETE_MESSAGE)
		{
			// If pSummary was NULL and the operation failed because 
			// the IMAP server was busy, repost the operation.
			hResult = pTocDoc->ImapDeleteMessages (pSummary);
			
		}
		else
		{
			hResult = pTocDoc->ImapUnDeleteMessages (pSummary);
		}

		// Repost action if we failed?? Only if no SingleSum and we got a frame.
		//

#if 0  // Doesn't work!!! (JOK)

		if ( pTocFrame && !pSummary && IMAP_SERVER_WAS_BUSY (hResult) )
		{
			if (theAction == CA_DELETE_MESSAGE)
				pTocFrame->PostMessage (WM_COMMAND, ID_MESSAGE_DELETE);
			else
				pTocFrame->PostMessage (WM_COMMAND, ID_MESSAGE_UNDELETE);
		}
#endif // JOK

		return;
	}
	else if ( theAction == CA_UPDATE_STATUS )
	{
		// If we're trying to turn off bolding of the mbox, don't allow it if any immediate child
		// mailbox has unread messages:
		//
		UnreadStatusType theStatus = ( UnreadStatusType ) ( ULONG ) pData;
		BOOL bFoundChildWithUnread = FALSE;

		if (theStatus != US_YES)
		{
			CPtrList& rList = GetChildList();

			POSITION pos = rList.GetHeadPosition();

			while( pos && !bFoundChildWithUnread)
			{
				QCMailboxCommand *pCurrent = ( QCMailboxCommand* ) rList.GetNext( pos );

				ASSERT( pCurrent );
			
				if( pCurrent && (pCurrent->GetStatus() == US_YES) )
				{
					bFoundChildWithUnread = TRUE;
					break;
				}
			}

			// If there's a child w/unread msgs, change the unread status but don't
			// un-bold:
			//
			if (bFoundChildWithUnread)
			{
				return;
			}
		}

		//
		// Do it:.
		//
		QCMailboxCommand::Execute( theAction, pData );
	}
	else
	{
		//
		// pass it up the ladder.
		//
		QCMailboxCommand::Execute( theAction, pData );
	}

}





// GetImapCommandDirectory
// FUNCTION
// Return, in "Dirpath", the directory housing the account, namespace or mailbox that
// pImapCommand object represents.
// END FUNCTION
BOOL QCImapMailboxCommand::GetObjectDirectory (CString& Dirpath)
{
	// Get the directory containing the mailboxes for this account/namespace/mailbox.
	switch (GetType())
	{
		case MBT_IMAP_ACCOUNT:
		case MBT_IMAP_NAMESPACE:
			Dirpath = GetPathname();
			break;
		case MBT_IMAP_MAILBOX:
			MbxFilePathToMailboxDir (GetPathname(), Dirpath);
			break;
		default:
			Dirpath.Empty();
			ASSERT (0);
			return FALSE;
		break;
	}

	return TRUE;
}


// InvalidateChildMailboxes [private]
// FUNCTION
// Set all child mailboxes as invalid.
// END FUNCTION

void QCImapMailboxCommand::InvalidateChildMailboxes (BOOL bRecurse)
{
	CPtrList&	theList = GetChildList();
	POSITION	pos;
	QCImapMailboxCommand *pCurrent;
	
	pos = theList.GetHeadPosition();

	while (pos)
	{
		pCurrent = (QCImapMailboxCommand *)theList.GetNext (pos);
		if (pCurrent)
		{
			pCurrent->MarkInvalid();

			// Recurse.
			if (bRecurse)
				pCurrent->InvalidateChildMailboxes (bRecurse);
		}
	}
}
	
		
// RemoveInvalidChildMailboxes [private]
// FUNCTION
// Invalid mailboxes no longer exist on the server. Remove them from our
// local menus and lists.
// END FUNCTION

void QCImapMailboxCommand::RemoveInvalidChildMailboxes (BOOL bRecurse)
{
	CPtrList&	theList = GetChildList();
	POSITION	pos;
	QCImapMailboxCommand *pCurrent;
	
	pos = theList.GetHeadPosition();

	while (pos)
	{
		pCurrent = (QCImapMailboxCommand *)theList.GetNext (pos);
		if (pCurrent)
		{
			// Remove leaf nodes first.
			if (bRecurse)
				pCurrent->RemoveInvalidChildMailboxes (bRecurse);

			// Me now.
			if (!pCurrent->IsValid())
			{
				// This will eventually cause local menus to get deleted,
				// local directories to be deleted, etc.

				// NOTE: This will cause pCurrent to get deleted. Make sure we don't use
				// it after this.

				// NOTE: Don't let ImapNotifyClients recurse because we are doing that here.
				g_theMailboxDirector.ImapNotifyClients (pCurrent, CA_DELETE, NULL);
			}
		}
	}
}


// Convert from IMapMailboxType to MailboxType; We do this to
// avoid #including a bunch of header files from the main Eudora code.
MailboxType QCImapMailboxCommand::ImapTypeToMBTType (ImapMailboxType Type)
{
	MailboxType mType = MBT_IMAP_MAILBOX;

	switch (Type)
	{
		case IMAP_MAILBOX:
			mType = MBT_IMAP_MAILBOX;
			break;
		case IMAP_ACCOUNT:
			mType = MBT_IMAP_ACCOUNT;
			break;
		case IMAP_NAMESPACE:
			mType = MBT_IMAP_NAMESPACE;
			break;
		default:
			// See above.
			break;
	}

	return mType;
}




// Convert from Eudora's MailboxType to IMapMailboxType; We do this to
// avoid #including a bunch of header files from the main Eudora code.
ImapMailboxType QCImapMailboxCommand::MBTTypeToImapType (MailboxType Type)
{
	ImapMailboxType mType = IMAP_MAILBOX;

	switch (Type)
	{
		case MBT_IMAP_MAILBOX:
			mType = IMAP_MAILBOX;
			break;
		case MBT_IMAP_ACCOUNT:
			mType = IMAP_ACCOUNT;
			break;
		case MBT_IMAP_NAMESPACE:
			mType = IMAP_NAMESPACE;
			break;
		default:
			break;
	}

	return mType;
}



// 
// FUNCTION
// This is TRUE if it's an account or if it doesn't have the NoInferiors flag.
// Overrides parent.
// END FUNCTION

BOOL QCImapMailboxCommand::CanHaveChildren ()
{
	return (GetType() == MBT_IMAP_ACCOUNT) || (!IsNoInferiors());	
}


// IsImapInbox [PUBLIC]
// 
// Is this the IMAP INBOX??
//
BOOL QCImapMailboxCommand::IsImapInbox()
{
	return m_szImapName.CompareNoCase ("INBOX") == 0;
}



#endif // IMAp4


