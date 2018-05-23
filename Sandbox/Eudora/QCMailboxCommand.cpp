// QCMailboxCommand.cpp: implementation of the QCMailboxCommand class.
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CString			EudoraDir;

IMPLEMENT_DYNAMIC(QCMailboxCommand, QCCommandObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCMailboxCommand::QCMailboxCommand(
QCMailboxDirector*	pDirector,
LPCSTR				szName,
LPCSTR				szPathname,
MailboxType			theType,
UnreadStatusType	theStatus) : QCCommandObject( pDirector )
{
	m_szName = szName;
	m_szPathname = szPathname;
	m_theType = theType;
	m_theStatus = theStatus;
}


QCMailboxCommand::~QCMailboxCommand()
{
	while( m_theChildList.IsEmpty() == FALSE )
	{
		delete ( QCMailboxCommand* ) m_theChildList.RemoveTail();
	}
}


void	QCMailboxCommand::Execute(
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	CTocDoc*			pTocDoc;
	CTocDoc*			pSrcToc;
	CSummary*			pSrcSummary;
	UnreadStatusType	theStatus;

	if( theAction == CA_COMPACT )
	{
		if( pData != NULL )
		{
			ASSERT_KINDOF( CTocDoc, ( CTocDoc* ) pData );
		}

		if( CompactMailbox( ( CTocDoc* ) pData ) == CRT_DRASTIC_ERROR )
		{
			return;		
		}
	}

	if( theAction == CA_UPDATE_STATUS )
	{
		theStatus = ( UnreadStatusType ) ( ULONG ) pData;

		if( m_theStatus == theStatus )
		{
			// no change -- no notification
			return;
		}
		
		m_theStatus = theStatus;
	}

	NotifyDirector( theAction, pData );	

	if( theAction == CA_OPEN )
	{
		pTocDoc = GetToc( m_szPathname, m_szName );
		
		if( pTocDoc != NULL )
		{
			pTocDoc->Display();
		}
		
		return;
	}	

	if( ( theAction == CA_TRANSFER_TO ) || ( theAction == CA_COPY ) )
	{
		pTocDoc = GetToc( m_szPathname );
		
		if( ( ( CObject* ) pData ) ->IsKindOf( RUNTIME_CLASS( CTocDoc ) ) )
		{
			pSrcToc = ( CTocDoc* ) pData;
			pSrcSummary = NULL;
		}
		else if( ( ( CObject* ) pData ) ->IsKindOf( RUNTIME_CLASS( CSummary ) ) )
		{
			pSrcSummary = ( CSummary* ) pData;
			pSrcToc = pSrcSummary->m_TheToc;		
		}
		else
		{
			ASSERT( 0 );
			return;
		}

		if( ( pSrcToc == NULL ) || ( pTocDoc == NULL ) )
		{
			ASSERT( 0 );
			return;
		}
		
		ASSERT_KINDOF( CTocDoc, pSrcToc );

		if ( ( theAction == CA_TRANSFER_TO ) && ( m_theType == MBT_TRASH ) )
		{
			pSrcToc->Xfer( pTocDoc, pSrcSummary );
		}
		else
		{
			pSrcToc->Xfer( pTocDoc, pSrcSummary, TRUE, ( theAction == CA_COPY ) );
		}

		if ( theAction == CA_TRANSFER_TO )
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
		}
	}
}


BOOL	QCMailboxCommand::IsEmpty()
{
	CTocDoc* pTocDoc;

	if( m_theType == MBT_FOLDER )
	{
		return m_theChildList.IsEmpty();
	}

	// Check if mailbox has any messages
	if( ::FileExistsMT( m_szPathname ) )
	{
		pTocDoc = GetToc( m_szPathname );
		
		if( ( pTocDoc != NULL ) && ( pTocDoc->NumSums() != 0 ) )
		{
			return FALSE;
		}
	}

	return TRUE;
}			



// CompactOneMailbox
//
// Compacts only one mailbox 
//
COMPACT_RESULT_TYPE QCMailboxCommand::CompactMailbox( 
CTocDoc*			pTocDoc )
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
	
	if( FAILED(theMBoxFile.Open( pTocDoc->MBFilename(), O_RDWR )) )
	{
		// returns mild error
		goto done;
	}
	
	// Let's see if compaction can be reduced to truncating the file
	
	pos = pTocDoc->m_Sums.GetHeadPosition();
	
	for( lLastOffset = 0L, i = 0; pos; i++ )
	{
		pSum = pTocDoc->m_Sums.GetNext( pos );
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
		theStatus = CRT_DRASTIC_ERROR;
		goto done;
    }

	pos = pTocDoc->m_Sums.GetHeadPosition();
	
	for( i = 0; pos; i++ )
	{
		pSum = pTocDoc->m_Sums.GetNext(pos);
		
		theTempFile.Tell(&lOffset);
		ASSERT(lOffset >= 0);
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
			theStatus = CRT_DRASTIC_ERROR;
			break;
		}
	}

	// If we didn't get all the way through, something went wrong so clean up
	if( i < pTocDoc->NumSums() )
	{
		pos = pTocDoc->m_Sums.GetHeadPosition();
		
		for (; pos && i; i--)
		{
			pSum = pTocDoc->m_Sums.GetNext( pos );
			pSum->m_Offset = pSum->m_Temp;
		}
		
		theTempFile.Delete();
		theMBoxFile.Close();
		goto done;
	}
	
	theTempFile.Close();
	theMBoxFile.Close();

	theTempFile.Rename( pTocDoc->MBFilename() );
	pTocDoc->m_NeedsCompact = FALSE;
	pTocDoc->ClearDelSpace();
	pTocDoc->Write();
	pTocDoc->ClearUndo();
	theStatus = CRT_COMPACTED;

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


void QCMailboxCommand::SetType(
MailboxType theType )
{
	m_theType = theType;
}


UINT QCMailboxCommand::GetFlyByID(
COMMAND_ACTION_TYPE theAction )
{
	switch( theAction )
	{
		case CA_TRANSFER_TO:	
			switch( m_theType )
			{
				case MBT_IN:
					return ID_TRANSFER_IN;
				break;
				case MBT_OUT:
					return ID_TRANSFER_OUT;
				break;
				case MBT_TRASH:
					return ID_TRANSFER_TRASH;
				break;
				default:
					return ID_TRANSFER_USER;
				break;
			}
		break;

		case CA_OPEN:
			switch( m_theType )
			{
				case MBT_IN:
					return ID_MAILBOX_IN;
				break;
				case MBT_OUT:
					return ID_MAILBOX_OUT;
				break;
				case MBT_TRASH:
					return ID_MAILBOX_TRASH;
				break;
				default:
					return ID_MAILBOX_USER;
				break;
			}
		break;

		case CA_TRANSFER_NEW:
			return ID_TRANSFER_NEW_MBOX_IN_ROOT;
		break;

		case CA_NEW_MAILBOX:
			return ID_NEW_MAILBOX_IN_ROOT;
		break;	

		case CA_INSERT_FCC:
			return IDS_FCC_FLYBY;
		break;

		case CA_FCC_NEW:
			return ID_FCC_NEW_MBOX_IN_ROOT;
		break;
	}

	return 0;
}


CString	QCMailboxCommand::GetToolTip(
COMMAND_ACTION_TYPE theAction)
{
	CString szToolTip( "" );

	switch( theAction )
	{
		case CA_TRANSFER_TO:	
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_TRANSFER );
			szToolTip += " ";
			szToolTip += m_szName;
		break;

		case CA_OPEN:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_MAILBOX );
			szToolTip += " ";
			szToolTip += m_szName;
		break;

		default:
		break;
	}

	return szToolTip;
}
