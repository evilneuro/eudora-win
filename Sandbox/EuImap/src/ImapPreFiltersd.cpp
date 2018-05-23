// ImapPreFiltersd.cpp
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>
#include <mmsystem.h>

#include "resource.h"
#include "rs.h"
#include "QCUtils.h"
// Need these for non-threadsafe stuff.
//
#include "doc.h"
#include "tocview.h"
#include "summary.h"
#include "filtersd.h"


#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

#include "QCImapMailboxCommand.h"

extern QCMailboxDirector	g_theMailboxDirector;

#include "imapfol.h"
#include "ImapPreFiltersd.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef EXPIRING
#include "timestmp.h"
extern CTimeStamp	g_TimeStamp;
#endif

#define NUM_FILT_ACTS 5

/////////////////////////////////////////////////////////////////////////////
CPreFilter::CPreFilter()
{
	m_WhenToApply = m_Verb[0] = m_Verb[1] = m_Conjunction = m_Transfer = 0;
	m_SkipRest= 0;
	m_ServerOpt = -1;

	m_Mailbox.Empty();
	
	for (int i = 0; i < 5; i++)
	{
		m_Actions[i]	= ID_FLT_NONE;
		m_DoCopyTo[i]	= FALSE;

		m_CopyTo[i].Empty();

		m_szDstImapName [i].Empty();
	}
}



CPreFilter::~CPreFilter()
{


}



/////////////////////////////////////////////////////////////////////////////
BOOL CPreFilter::MatchValueMT(int NumCondition, const char* Contents) const
{
	int verb = VERB_FIRST + m_Verb[NumCondition];
	CString value(m_Value[NumCondition]);
	int len1, len2;

	// Filter comparisons are case insensitive, so convert to lowercase
	value.MakeLower();

	switch (verb)
	{
	case IDS_CONTAINS:
	case IDS_DOESNT_CONTAIN:
		if (strstr(Contents, value))
			return (TRUE);
		break;

	case IDS_IS:
	case IDS_IS_NOT:
		if (!strcmp(Contents, value))
			return (TRUE);
		break;
	case IDS_STARTS_WITH:
		if (!strncmp(Contents, value, value.GetLength()))
			return (TRUE);
		break;

	case IDS_ENDS_WITH:
		len1 = value.GetLength();
		len2 = strlen(Contents);
		if (len1 < len2 && !strcmp(Contents + len2 - len1, value))
			return (TRUE);
		break;

	case IDS_APPEARS:
	case IDS_DOESNT_APPEAR:
		return (TRUE);

	case IDS_INTERSECTS:
		// We don't handle this - we want to be thread safe.
		break;
	}

	return (FALSE);
}


// CopyFromCFilter [PUBLIC]
//
// NOTE: NOT Thread Safe!!!
//
void CPreFilter::CopyFromCFilter (CFilter* pFilt)
{
	if (!pFilt)
		return;

	// Must be in the main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// We copy only relevant stuff.
	// NOTE:
	// This is very unsafe. Most of CFilter's attributes are public so we just copy
	// them. If CFilter should change......!
	//

	// CStrings.
	m_Name			= pFilt->m_Name;
	m_Mailbox		= pFilt->m_Mailbox;
	
	m_WhenToApply	= pFilt->m_WhenToApply;

	// Header strin array (2) CStrings.
	m_Header[0]		= pFilt->m_Header[0];
	m_Header[1]		= pFilt->m_Header[1];

	// Verb array (2) CStrings.
	m_Verb[0]		= pFilt->m_Verb[0];
	m_Verb[1]		= pFilt->m_Verb[1];

	// Values of the verd objects (2).
	m_Value[0]		= pFilt->m_Value[0];
	m_Value[1]		= pFilt->m_Value[1];

	m_Conjunction	= pFilt->m_Conjunction;

	m_Transfer		= pFilt->m_Transfer;
	m_SkipRest		= pFilt->m_SkipRest;
	m_ServerOpt		= pFilt->m_ServerOpt;

	// 5 actions, Aand corresponding data. Will the number change???
	//
	for (int i = 0; i < 5; i++)
	{
		m_Actions[i]	= pFilt->m_Actions[i];

		m_DoCopyTo[i]	= pFilt->m_DoCopyTo[i];

		// These are CStrings.
		m_CopyTo[i]		= pFilt->m_CopyTo[i];
	}

	// Filter reporting??
	//
	m_NotfyUserNR = pFilt->m_NotfyUserNR;
}



// ContainsCopyOrXfer [PUBLIC]
//
// Return TRUE if this object contains a copy or transfer action,
// regardless of destination.
//
BOOL CPreFilter::ContainsCopyOrXfer ()
{
	BOOL bResult = FALSE;

	// Does it have a copy/xfer?
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		switch(m_Actions[i])
		{
			// Found one?
			case ID_FLT_COPY:
			case ID_FLT_TRANSFER:
				if (!m_CopyTo[i].IsEmpty() || IsTransfer())
				{
					return TRUE;
				}

				break;

			default:
				break;
		}
	}

	return FALSE;
}


// HasSkipRestMT [PUBLIC]
//
// Return TRUE if contains a skip resg action.
//
BOOL CPreFilter::HasSkipRest ()
{
	BOOL bResult = FALSE;

	// Look for ANY skip rest:
	//
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if (m_Actions[i] == ID_FLT_SKIP_REST)
			return TRUE;
	}

	return FALSE;
}




/////////////////////////////////////////////////////////////////////////////

CPreFilterList::~CPreFilterList()
{
	NukeAllData();
}


/////////////////////////////////////////////////////////////////////////////
void CPreFilterList::NukeAllData()
{
	while (!IsEmpty())
		delete RemoveHead();
}


/////////////////////////////////////////////////////////////////////////////
CPreFilter* CPreFilterList::IndexData(int nIndex) const
{
	POSITION pos = FindIndex(nIndex);
	
	return (pos? GetAt(pos) : NULL);
}



// AddFromCFilter [PUBLIC] (NOT THREAD SAFE)
//
// Determine if this 
// Create a new CPreFilter object, copy relevant data from the given CFilter object,
// and add to te end of the list.
//

void CPreFilterList::AddFromCFilter (CFilter* pFilt, CTocDoc *pSrcToc)
{
	// Must be in main thread.
	//
	ASSERT ( IsMainThreadMT() );

	// Sanity:
	//
	if (! (pSrcToc && pFilt) )
		return;

	// If we found one, we'd use this pointer.
	//
	CPreFilter*	pNewPF = NULL;


	// Loop through the filter actions and determine if this is a filter we want to copy.
	// We copy:
	//	- Those that have an IMAP same-server copy/transfer action, 
	//	- Those that contain a skip rest action.
	//
	// Note: We loop through all actions because we may have several COPY's
	// For each COPY or XFER actions, we MUST get the destination IMAP mailbox
	// and save it to the CPreFilter object for that action.
	//
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		BOOL		bOneToCopy = FALSE;

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
						break; // Out of switch.
					}

					// Seems like we found one.
					// Have we created a copy yet??
					//
					if (!pNewPF)
					{
						pNewPF = CreateFromFilter (pFilt);
					}

					if (!pNewPF)
					{
						// Don't even try any further.
						return;
					}
		
					// Must set the destination IMAP mailbox name.
					//
					// Copy destination name to the out paramater.
					//
					CString szDstImapName = ((QCImapMailboxCommand *)pCommand)->GetImapName ();

					pNewPF->SetDstImapName (i, szDstImapName);
				}

				break;

			case ID_FLT_SKIP_REST:
				// Yep. Copy this.
				//
				if (!pNewPF)
				{
					pNewPF = CreateFromFilter (pFilt);
				}

				if (!pNewPF)
				{
					return;
				}

				break;

			default:
				break;
		} // switch

		// Note: Loop through ALL actions.

	} // for
}




// Create a new CPreFilter object and copy basic stuff.
//
CPreFilter* CPreFilterList::CreateFromFilter (CFilter* pFilt)
{
	if (!pFilt)
		return NULL;

	// Must be in main thread.
	//
	ASSERT ( IsMainThreadMT() );

	CPreFilter* pNewPF = new CPreFilter;

	if (!pNewPF)
	{
		return NULL;
	}

	// Copy general things.
	pNewPF->CopyFromCFilter (pFilt);

	// Add to end of list:
	//
	AddTail (pNewPF);

	return pNewPF;
}





// ===========================  EXPORTED FUNCTION ==================//

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


