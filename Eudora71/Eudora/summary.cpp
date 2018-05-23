// SUMMARY.CPP
//
// Routines for message summaries
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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
#include <afxcmn.h>
#include <ctype.h>

#include <QCUtils.h>

#include "cursor.h"
#include "resource.h"
#include "rs.h"
#include "fileutil.h"
#include "utils.h"
#include "guiutils.h"
#include "address.h"
#include "summary.h"
#include "doc.h"
#include "tocview.h"
#include "eudora.h"
#include "msgutils.h"
#include "font.h"
#include "mainfrm.h"
#include "compmsgd.h"
#include "ReadMessageDoc.h"
#include "msgframe.h"
#include "sendmail.h"
#include "jjnet.h"
#include "progress.h"
#include "header.h"
#include "mime.h"
#include "MoodWatch.h"

#include "pop.h"
#include "MsgRecord.h"
#include "persona.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCProtocol.h"
#include "TocFrame.h"
#include "AutoCompleteSearcher.h"

#include "trnslate.h"
#include "QCSharewareManager.h"
#include "SearchManager.h"

// IMAP4
#include "ImapMailbox.h"
// END IMAP4

//AKN:UsgStats
// Usage Statistics
#include "StatMng.h"
//AKN:UsgStats

extern QCMailboxDirector	g_theMailboxDirector;

#include "DebugNewHelpers.h"


// in_bOwnsSums - false if destructor shouldn't delete summaries. Used for lists
//				  that contain summaries that are owned by and will be deleted
//				  by someone else. Specifically used for CTempSumLists.
// in_bUseMap -	  false if maps shouldn't be used to speed up operations. Cannot
//				  be turned on midstream - either always true for lifetime of
//				  the object or always false. Useful for CSumLists that can
//				  contain summaries that don't need to be found quickly via Find
//				  and GetByMessageId. Also useful for CSumLists that can contain
//				  summaries with a 0 message ID.
CSumList::CSumList(
	bool						in_bOwnsSums,
	bool						in_bUseMap)
	:	m_bOwnsSums(in_bOwnsSums), m_bUseMaps(in_bUseMap), m_ObList()
{

}


CSumList::~CSumList()
{
	if (m_bOwnsSums)
	{
		// Turn off map use so that we don't try to clear the maps one by one
		m_bUseMaps = false;
		
		// Delete each summary
		while ( !IsEmpty() )
			delete RemoveHead();
	}
}


void
CSumList::MoveSumsToOtherList(
	CSumList *					out_pDestination)
{
	// Verify that we have a good destination
	ASSERT(out_pDestination);
	if (!out_pDestination)
		return;

	// Move all sums to the destination list
	POSITION pos = GetHeadPosition();
	
	while (pos)
	{
		CSummary *	pSum = GetNext(pos);
		out_pDestination->AddTail(pSum);
	}

	// Remove our copies of the sums
	RemoveAll();
}


CSummary *
CSumList::RemoveHead()
{
	CSummary *	pSummary = reinterpret_cast<CSummary *>( m_ObList.RemoveHead() );

	RemoveSummaryFromMaps(pSummary);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	return pSummary;
}


CSummary *
CSumList::RemoveTail()
{
	CSummary *	pSummary = reinterpret_cast<CSummary *>( m_ObList.RemoveTail() );

	RemoveSummaryFromMaps(pSummary);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	return pSummary;
}


POSITION
CSumList::AddHead(
	CSummary *					in_pNewSummary)
{
	POSITION	pos = m_ObList.AddHead(in_pNewSummary);

	AddSummaryToMaps(pos, in_pNewSummary);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	return pos;
}


POSITION
CSumList::AddTail(
	CSummary *					in_pNewSummary)
{
	POSITION	pos = m_ObList.AddTail(in_pNewSummary);

	AddSummaryToMaps(pos, in_pNewSummary);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	return pos;
}


void
CSumList::RemoveAll()
{
	m_ObList.RemoveAll();
	m_mapMessageIDToSummary.clear();
	m_mapSummaryToPosition.clear();
}


void
CSumList::SetAt(
	POSITION					position,
	CSummary *					in_pSummary)
{
	// Remove the old data at the specified position, if any.
	// We only remove the old data if our maps still list it as being in the old
	// position. That handles the operation of sorting, which runs through the
	// list replacing the elements where they stand. For example consider resorting
	// a b c => c b a. When SetAt is called with the third position the list looks
	// like this: c b c. The maps now record "c" as being in the first position, but
	// m_ObList also has it in the third position. In this case RemoveSummaryFromMaps
	// will skip removing it (leaving the first position alone) and AddSummaryToMaps
	// will correctly add the "a" to the third position.
	CSummary *	pSummary = GetAt(position);
	if (pSummary)
		RemoveSummaryFromMaps(pSummary, position);

	// If the caller specified something other than NULL, then
	// add it to the maps.
	if (in_pSummary)
		AddSummaryToMaps(position, in_pSummary);

	m_ObList.SetAt(position, in_pSummary);
}


void
CSumList::RemoveAt(
	POSITION					position)
{
	CSummary *	pSummary = GetAt(position);

	RemoveSummaryFromMaps(pSummary, position);

	m_ObList.RemoveAt(position);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );
}


POSITION
CSumList::InsertBefore(
	POSITION					in_position,
	CSummary *					in_pSummary)
{
	POSITION	posInserted = m_ObList.InsertBefore(in_position, in_pSummary);

	AddSummaryToMaps(posInserted, in_pSummary);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	return posInserted;
}


POSITION
CSumList::InsertAfter(
	POSITION					in_position,
	CSummary *					in_pSummary)
{
	POSITION	posInserted = m_ObList.InsertAfter(in_position, in_pSummary);

	AddSummaryToMaps(posInserted, in_pSummary);

	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	return posInserted;
}


//
//	CSumList::GetIndex()
//
//	Determine the index of Sum in this list.  If bIgnoreDeletedIMAP is TRUE then we need to
//	adjust the index to account for any deleted messages that precede Sum in the list.
//
int CSumList::GetIndex(const CSummary* Sum, BOOL bIgnoreDeletedIMAP) const
{
	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	if (bIgnoreDeletedIMAP)
	{
		// If we are hiding deleted IMAP messages start looking for a match at the
		// beginning so we can count the number of deleted IMAP messages that
		// occur before the match so we can subtract that number from the actual index.
		POSITION	 pos = GetHeadPosition();
		int			 iCount = GetCount();
		int			 iDeleted = 0;
		for (int i = 0; i < iCount; ++i)
		{
			const CSummary *	pSum = GetNext(pos);
			if (pSum && pSum->IsIMAPDeleted())
			{
				++iDeleted;
			}
			else if (Sum == pSum)
			{
				return (i - iDeleted);
			}
		}
	}
	else
	{
		POSITION pos = GetTailPosition();
		
		for (int i = GetCount() - 1; i >= 0 ; i--)
		{
			if (Sum == GetPrev(pos))
				return (i);
		}
	}
	
	return (-1);
}

//
//	CSumList::GetCount()
//
//	Returns the count of the summaries.  If bIgnoreDeletedIMAP is true, then we manually count the
//	summaries that are not deleted, otherwise, we go with the default CObList::GetCount() which relies
//	on the m_nCount member field.  Obviously manually counting is considerably slower.
//
//	Note: This function assumes that if bIgnoreDeletedIMAP is true then this list of summaries belongs
//	to a CTocDoc that is an IMAP toc.  This assumption requires the calling code to make this determination
//	(which is generally not an issue since the caller is almost always the CTocView) and frees this function
//	of the responsibility to extract this info.
//
int CSumList::GetCount(BOOL bIgnoreDeletedIMAP) const
{
	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );
	
	// If we are ignoring deleted IMAP messages manually count the summaries that are not marked as deleted.
	if (bIgnoreDeletedIMAP)
	{
		int			 iCount = 0;
		POSITION	 pos = GetHeadPosition();
		while (pos)
		{
			const CSummary *	pSum = GetNext(pos);
			if (pSum && !pSum->IsIMAPDeleted())
			{
				++iCount;
			}
		}
		return iCount;
	}

	// If we are not ignoring deleted IMAP messages, return the count.
	return m_ObList.GetCount();
}

// Find
//
// Optimized via a std::map.
//
POSITION
CSumList::Find(
	CSummary *					in_pFindSummary) const
{
	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	POSITION					pos = NULL;

	if (m_bUseMaps)
	{
		// We've been maintaining the maps, so find it the fast way
		SummaryMapConstIteratorT	iterator = m_mapSummaryToPosition.find(in_pFindSummary);

		// ASSERT when we couldn't find it using the new way AND we could find it
		// using the old way. This avoids ASSERTing in cases where it's ok that
		// we're returning NULL. 
		ASSERT( (iterator != m_mapSummaryToPosition.end()) || !m_ObList.Find(in_pFindSummary) );

		if ( iterator != m_mapSummaryToPosition.end() )
		{
			pos = iterator->second;

			// ASSERT that the map found the correct position
			ASSERT( GetAt(pos) == in_pFindSummary );
		}
	}
	else
	{
		// No maps, so tell object list to find it
		pos = m_ObList.Find(in_pFindSummary);
	}

	return pos;
}


void
CSumList::AddSummaryToMaps(
	POSITION					in_pos,
	CSummary *					in_pSummary)
{
	if (!m_bUseMaps)
		return;
	
	ASSERT(in_pos);
	ASSERT(in_pSummary);
	ASSERT( in_pSummary->GetUniqueMessageId() );
	
	if (in_pSummary)
	{
		if ( in_pSummary->GetUniqueMessageId() )
			m_mapMessageIDToSummary[ in_pSummary->GetUniqueMessageId() ] = in_pSummary;

		if (in_pos)
			m_mapSummaryToPosition[ in_pSummary ] = in_pos;
	}
}


void
CSumList::RemoveSummaryFromMaps(
	CSummary *					in_pSummary,
	POSITION					in_pos)
{
	if (!m_bUseMaps)
		return;

	ASSERT(in_pSummary);
	
	if (in_pSummary)
	{
		SummaryMapIteratorT		iterSummaryMap = m_mapSummaryToPosition.find(in_pSummary);

		// Verify that we found a position for the summary
		ASSERT( iterSummaryMap != m_mapSummaryToPosition.end() );

		// If the summary being removed was found and either:
		// * We don't care what position it was in (i.e. in_pos is NULL)
		// * It matches the position we expect
		// then proceed with removing it from the lists.
		if ( (iterSummaryMap != m_mapSummaryToPosition.end()) && 
			 (!in_pos || (iterSummaryMap->second == in_pos)) )
		{
			m_mapSummaryToPosition.erase(iterSummaryMap);
		
			unsigned long			uUniqueMessageID = in_pSummary->GetUniqueMessageId();
			MessageIDMapIteratorT	iterMessageIDMap = m_mapMessageIDToSummary.find(uUniqueMessageID);

			ASSERT( iterMessageIDMap != m_mapMessageIDToSummary.end() );
			if ( iterMessageIDMap != m_mapMessageIDToSummary.end() )
				m_mapMessageIDToSummary.erase(iterMessageIDMap);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSummary

IMPLEMENT_DYNAMIC(CSummary, CObject)

/////////////////////////////////////////////////////////////////////////////
// CSummary diagnostics

#ifdef _DEBUG
void CSummary::AssertValid() const
{
	CObject::AssertValid();
}

void CSummary::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	AFX_DUMP1(dc, "\n\tFrom = ", m_From);
	AFX_DUMP1(dc, "\n\tSubject = ", m_Subject);
	AFX_DUMP1(dc, "\n\tDate = ", m_Date);
}

#endif //_DEBUG


CSummary::CSummary() : CObject(),
	m_SavedPos(-1, -1, -1, -1)
{
	m_Offset = 0L;
	m_Length = 0L;
	m_State = MS_UNREAD;
	m_ucJunkScore = 0;
	m_ulJunkPluginID = 0;
	m_bManuallyJunked = false;
	m_ucOldJunkScore = 0;
	m_ulOldJunkPluginID = 0;
	m_bOldManuallyJunked = false;
	m_bOldJunk = false;
	m_bLastXferWasJunkAction = false;
	m_Label = 0;
	memset(m_From, 0, sizeof(m_From));
	memset(m_Subject, 0, sizeof(m_Subject));
	memset(m_Date, 0, sizeof(m_Date));
	m_Seconds = 0L;
	m_lArrivalSeconds = 0L;
	m_TimeZoneMinutes = 0;
	m_TheToc = NULL;
	m_FrameWnd = NULL;
	//m_Flags = ( unsigned short ) (GetIniShort(IDS_INI_SHOW_ALL_HEADERS)? MSF_SHOW_ALL_HEADERS : 0);
	m_Flags = 0;
	m_Priority = MSP_NORMAL;
	m_nMood = MSM_MOOD_UNKNOWN;
	m_Hash = 0L;
	m_UniqueMessageId = 0L;
	m_ResponseSum = m_RespondingToSum = NULL;
	m_TransHdr.Empty();
	m_SigHdr.Empty();
	m_SigSelected.Empty();
	m_Precedence.Empty();
	m_FlagsEx = 0;

	// The number of attachments that would be associated with this summary when the mail is being sent
	m_nAttachmentCount = 0;
	m_DeleteEmbedded = TRUE;
	SetPersona( g_Personalities.GetCurrent() );

#ifdef IMAP4
	m_Imflags = 0;
	m_MsgSize = 0;
	m_nUndownloadedAttachments = 0;
#endif

	m_rrDialogOverride = false;	// RR dialog override -jdboyd 8/23/99

	m_bReplyAll = false;

	m_bNotifySearchManager = true;
	m_bShouldBeAddedToIndex = false;
}

CSummary::~CSummary()
{
	// Clean up pointers used to undo state changes when a comp is cancelled
	if (m_ResponseSum)
		m_ResponseSum->m_RespondingToSum = NULL;
	if (m_RespondingToSum)
		m_RespondingToSum->m_ResponseSum = NULL;

	// Normally, a window shouldn't still exist for a summary when then
	// summary is being deleted.  But there's a weird case during shutdown
	// where this can happen.  So if the window still exists, we need to
	// reset to NULL the m_Sum member variable of the CMessageDoc object.
	CMessageDoc* Doc = NULL;
	if (Doc = FindMessageDoc())
	{
		// JB and SD say "go away, this is normal"  ASSERT(0);
		if (Doc->IsKindOf(RUNTIME_CLASS(CMessageDoc)))
			Doc->m_Sum = NULL;
	}

	// Don't bother telling m_TheToc to invalidate us out of its cached
	// preview summaries, because CTocListBox::DeleteItem will already
	// handle any such deletions.
}

void CSummary::Copy(CSummary* NewInfo)
{
	m_Length = NewInfo->m_Length;
	m_State = NewInfo->m_State;
	m_ucJunkScore = NewInfo->m_ucJunkScore;
	m_bManuallyJunked = NewInfo->m_bManuallyJunked;
	m_ulJunkPluginID = NewInfo->m_ulJunkPluginID;
	m_ucOldJunkScore = NewInfo->m_ucOldJunkScore;
	m_ulOldJunkPluginID = NewInfo->m_ulOldJunkPluginID;
	m_bOldManuallyJunked = NewInfo->m_bOldManuallyJunked;
	m_bLastXferWasJunkAction = NewInfo->m_bLastXferWasJunkAction;
	m_Priority = NewInfo->m_Priority;
	m_nMood = NewInfo->m_nMood;
	m_Label = NewInfo->m_Label;
	m_Seconds = NewInfo->m_Seconds;
	m_lArrivalSeconds = NewInfo->m_lArrivalSeconds;
	m_TimeZoneMinutes = NewInfo->m_TimeZoneMinutes;
	m_SavedPos = NewInfo->m_SavedPos;
	memcpy(m_Date, NewInfo->m_Date, sizeof(m_Date));
	memcpy(m_From, NewInfo->m_From, sizeof(m_From));
	memcpy(m_Subject, NewInfo->m_Subject, sizeof(m_Subject));
	m_Flags = NewInfo->m_Flags;
	//Make sure Show-all-headers is turned off
	m_Flags &= ~MSF_SHOW_ALL_HEADERS;
	m_FlagsEx = NewInfo->m_FlagsEx;
	m_Hash = NewInfo->m_Hash;
	m_PersonaHash = NewInfo->m_PersonaHash;

	m_nAttachmentCount = NewInfo->m_nAttachmentCount;

#ifdef IMAP4
	m_Imflags = NewInfo->m_Imflags;
	m_MsgSize = NewInfo->m_MsgSize;
#endif
	//
	// Intentionally not copying the message id.  The assumption is
	// that we're populating a newly created summary object that has
	// not yet been added to any TocDoc.  The TocDoc should determine
	// how "virgin" summary objects get their message id.
	//
	ASSERT(0 == m_UniqueMessageId);
}

BOOL CSummary::Selected() const
{
	CTocView* TocView;
	
	if (m_TheToc && (TocView = m_TheToc->GetView()))
		return (TocView->Selected(this));
		
	return (FALSE);
}

int CSummary::Select(BOOL bSelect /*= TRUE*/, bool in_bResultDirectUserAction /*= false*/)
{
	CTocView* TocView;
	
	if (m_TheToc && (TocView = m_TheToc->GetView()))
		return TocView->Select(this, bSelect, in_bResultDirectUserAction);
		
	return (LB_ERR);
}


int CSummary::GetSize()
{
	//if POP, get the length of the message in mbx excluding the size of attachments
	
	if( m_TheToc && m_TheToc->IsImapToc())
	{
		return m_MsgSize;
	}
	else
	{
		//POP stores size in bytes
		return (m_Length / 1024); //in KB
	}
}

const char* CSummary::GetFrom()
{
	if (*m_From)
		return (m_From);

	return (strcpy(m_From + 1,
		(m_State < MS_UNSENDABLE? CRString(IDS_NO_SENDER) : CRString(IDS_NO_RECIPIENT))));
}

////////////////////////////////////////////////////////////////////////
// GetPriority [public]
//
// Returns an ASCII string which corresponds to the current priority
// value.
////////////////////////////////////////////////////////////////////////
CString CSummary::GetPriority() const
{
	CString priority;

	switch (m_Priority)
	{
	case MSP_HIGHEST:
		priority.LoadString(IDS_PRIORITY_HIGHEST);
		break;
	case MSP_HIGH:
		priority.LoadString(IDS_PRIORITY_HIGH);
		break;
	case MSP_NORMAL:
		priority.LoadString(IDS_PRIORITY_NORMAL);
		break;
	case MSP_LOW:
		priority.LoadString(IDS_PRIORITY_LOW);
		break;
	case MSP_LOWEST:
		priority.LoadString(IDS_PRIORITY_LOWEST);
		break;
	default:
		ASSERT(0);
		break;
	}

	return priority;
}

const char* CSummary::GetSubject()
{
	if (*m_Subject)
		return (m_Subject);

	return (strcpy(m_Subject + 1, CRString(IDS_NO_SUBJECT)));
}

const char* CSummary::MakeTitle()
{
	m_Title.Format(IDS_SUM_TITLE_FORMAT, GetFrom(),
					(m_Date[0]? ", " : ""), m_Date, GetSubject());

	return (m_Title);
}

void CSummary::SetJunkScore(unsigned char ucJunkScore)
{
	if ( (ucJunkScore != m_ucJunkScore) && m_TheToc )
	{
		m_TheToc->SetModifiedFlag();
		
		SearchManager::Instance()->NotifySumModification(m_TheToc, this);
	}
	
	m_ucJunkScore = ucJunkScore;
}

// FindMessageDoc
// Returns the exsiting message document associated with this summary, if one exists, otherwise NULL.
//
CMessageDoc* CSummary::FindMessageDoc()
{
	CMultiDocTemplate* MessageTemplate = (IsComp()? CompMessageTemplate : ReadMessageTemplate);
	CMessageDoc* doc = NULL;

	POSITION pos = MessageTemplate->GetFirstDocPosition();
	for (POSITION NextPos = pos; pos != NULL; pos = NextPos)
	{
		doc = (CMessageDoc*)MessageTemplate->GetNextDoc(NextPos);
		if (doc->m_Sum == this)
			break;
    }

	return (pos? doc : NULL);
}

// GetMessageDoc
// Gets the message document associated with this summary.  Will create one if necessary.
//
CMessageDoc* CSummary::GetMessageDoc(bool * pbWasCreated)
{
	if (pbWasCreated)
		*pbWasCreated = false;

	CMessageDoc* doc = FindMessageDoc();
	
	if (doc)
		return (doc);
		
	if (doc = (CMessageDoc*)NewChildDocument(IsComp()? CompMessageTemplate : ReadMessageTemplate))
	{
		doc->m_Sum = this;

		if (pbWasCreated)
			*pbWasCreated = true;
	}

	return (doc);
}

// NukeMessageDocIfUnused
// Deletes CMessageDoc if it exists and isn't being viewed
//
BOOL CSummary::NukeMessageDocIfUnused(CSummary * pSum, CMessageDoc * pMessageDoc)
{
	if (pMessageDoc)
	{
		// Is it being used by Read/Comp views?
		POSITION pos = pMessageDoc->GetFirstViewPosition();
		if (!pos)
		{
			pMessageDoc->OnCloseDocument();
			return TRUE;
		}
	}

	return FALSE;
}

// Display
// Displays a read or composition message, or brings it to the front if already open
//
BOOL CSummary::Display()
{
	if (m_FrameWnd)
	{
		//
		// Bring the existing frame window to the front.
		//
		ASSERT_VALID(m_FrameWnd);
		ASSERT_KINDOF(QCWorksheet, m_FrameWnd);
		if (m_FrameWnd->IsIconic())
			m_FrameWnd->MDIRestore();
		else
			m_FrameWnd->ActivateFrame();
		return TRUE;
	}

#ifdef IMAP4
	// If we're going to display this, if this is part of an IMAP toc,
	// attempt to download the message here before we dig ourselves any deeper.
	//
	if ( m_TheToc && m_TheToc->IsImapToc () )
	{
		if ( !SUCCEEDED (DownloadImapMessage (this)) )
			return FALSE;
	}
#endif // IMAP4

	//
	// If we get this far, we need to load up the doc and
	// create a new frame window for it.
	//
	CMultiDocTemplate* MessageTemplate = (IsComp()? CompMessageTemplate : ReadMessageTemplate);
	CMessageDoc* pMessageDoc = GetMessageDoc();
	if (! pMessageDoc)
		return FALSE;

	// Mark the message as read if it was unread
	if (m_State == MS_UNREAD)
		SetState(MS_READ);

#ifdef IMAP4 // IMAP4
			// Must also set the remote message status.
			// We should do this before we call CSummary::SetState() but this
			// is less disruptive of the code.
			// 
			if (m_TheToc && m_TheToc->IsImapToc())
			{
				m_TheToc->ImapSetMessageState (this, MS_READ);
			}
#endif // IMAP4

	// Sometimes funky stuff happens when Eudora is not the foreground
	// app and we try to display a composition window
	if (IsComp() && CMDIChild::m_nCmdShow != SW_HIDE && !IsForegroundApp())
		AfxGetMainWnd()->SetForegroundWindow();
			
	return (NewChildFrame(MessageTemplate, pMessageDoc) != NULL);
}


BOOL CSummary::DisplayBelowTopMostMDIChild()
{
	//Find the top level window and save it
	CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	BOOL bIsMaximized = FALSE;
	CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);

	if(!pActiveMDIChild || (pActiveMDIChild && !pActiveMDIChild->IsWindowVisible()) )
	{
		Display();
		return TRUE;
	}


	CMDIChild::m_nCmdShow = SW_HIDE;
	Display();
	if(m_FrameWnd)
		m_FrameWnd->SetWindowPos(pActiveMDIChild, 0,0,0,0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );
	CMDIChild::m_nCmdShow = -1;

	return TRUE;
}


// ComposeMessage
// Creates the new composition message based on this summary and
// the type of response (reply, forward, redirect).
//
BOOL CSummary::ComposeMessage
(
	UINT MenuItemID, 
	const char* DefaultTo /*= NULL*/,
	BOOL AutoSend /*=FALSE*/, 
	const char* pszStationery /*= NULL*/,
	const char* pszPersona /*= NULL*/,
	const char* selectedText /*= NULL*/,
	BOOL bTurboRedirectOK /*TRUE*/ 
)
{
    char* OldSubject = NULL;
    char* NewTo = NULL;
    char* NewSubject = NULL;
    char* NewCc = NULL;
    const char* theMessage = NULL;
    char* message = NULL;
    const char* mes2 = NULL;
    char* QuotedBody = NULL;
    CString TempMes;
    BOOL bSuccess = FALSE;
    char ResponseType = 0;
    CCursor cursor;
    char**      pSrcMessage = NULL;
	bool		bDocWasCreated = false;
        
    UINT IsRich = 0;
    long lBufLen;

	// Figure out the message state from the ID of the menu item
	switch (MenuItemID)
	{
	case ID_MESSAGE_REPLY: {	
		ResponseType = MS_REPLIED;		
		break; }
	case ID_MESSAGE_REPLY_ALL: {
		ResponseType = MS_REPLIED;		
		break; }
	case ID_MESSAGE_FORWARD: {
		ResponseType = MS_FORWARDED;	
		break; }
	case ID_MESSAGE_REDIRECT: {
		ResponseType = MS_REDIRECT;		
		break; }
	default: 
		ASSERT(FALSE);
	}

	bool	bCreatedDoc = false;

	CMessageDoc* doc = GetMessageDoc(&bCreatedDoc);
	if (!doc)
		return FALSE;

	if ( IsComp() )
	{
		CCompMessageDoc* comp = (CCompMessageDoc*)doc;
		
		// Make sure it's read in
		comp->GetText();

		// Collect all the headers with stuff in it
		for (int i = 0; i < MaxHeaders; i++)
		{
			const char* contents = comp->GetHeaderLine(i);
			while (isspace((int)(unsigned char)*contents))
				contents++;
			if (*contents)
				TempMes += CRString(IDS_HEADER_TO + i) + ' ' + contents + "\r\n";
		}

		// Add on the body
		TempMes += "\r\n";
		TempMes += comp->GetText();
		message = DEBUG_NEW char[TempMes.GetLength() + 1];
		strcpy(message, TempMes);
	}
	else
	{
		if (!doc->IsModified() || doc->SaveModified())
		{
			// Don't use only the selection for redirect
			if (ResponseType == MS_REDIRECT)
				selectedText = NULL;
			else
			{
				// Get the INI setting for what characters indicate a valid reply selection
				CString		szReplySelectionChars;
				GetIniString(IDS_INI_REPLY_SELECTION_CHARS, szReplySelectionChars);

				bool		bLookForHTMLLineBreakTags = false;
				
				// If the selected text that we were given is HTML, we may need to look
				// for HTML line break tags and ignore line break characters.
				if ( selectedText && (strlen(selectedText) >= 6) &&
					 (strnicmp(selectedText, "<html>", 6) == 0) )
				{
					// Are we interested in HTML line breaks?
					bLookForHTMLLineBreakTags = (szReplySelectionChars.Find('\n') != -1);

					// If we're interested in HTML line breaks, don't look for
					// character line breaks.
					if (bLookForHTMLLineBreakTags)
						szReplySelectionChars.Remove('\n');
				}

				// If we were told not to reply to selection or
				// if it doesn't have tasty chars, pretend no selection
				if ( selectedText && (!GetIniShort(IDS_INI_REPLY_SELECTION) ||
					 !strpbrk(selectedText, szReplySelectionChars)) )
				{
					if (!bLookForHTMLLineBreakTags)
					{
						// We're done - we know that there were no characters that indicate
						// that we should keep the selection, so pretend no selection.
						selectedText = NULL;
					}
					else	// bLookForHTMLLineBreakTags
					{
						// So far it doesn't look like we want to keep the selection, but there
						// may be HTML line breaks that make us want to keep the selection.
						CString		szBRTag;
						CString		szDivTag;
						CString		szPTag;

						szBRTag.Format( "<%s>", CRString(IDS_HTML_BREAK) );
						szDivTag.Format( "<%s>", CRString(IDS_HTML_DIV) );
						szPTag.Format( "<%s>", CRString(IDS_HTML_P) );

						// Scan for HTML line break tags
						bool			bFoundHTMLLineBreak = false;
						const char *	pScanText = strchr(selectedText, '<');

						while (pScanText)
						{
							if ( (strnicmp(pScanText, szBRTag, szBRTag.GetLength()) == 0) ||
								(strnicmp(pScanText, szDivTag, szDivTag.GetLength()) == 0) ||
								(strnicmp(pScanText, szPTag, szPTag.GetLength()) == 0) )
							{
								// Found HTML line break
								bFoundHTMLLineBreak = true;
								break;
							}
							else
							{
								// Look for next HTML tag
								pScanText = strchr(pScanText + 1, '<');
							}
						}

						// If we didn't find an HTML line break, ignore the selected
						// text that we were passed.
						if (!bFoundHTMLLineBreak)
							selectedText = NULL;
					}
				}
			}
		    
			// use selection if there is (a suitable) one
			theMessage = selectedText ? (char *)selectedText : doc->GetText();

			// Possible for theMessage to be NULL - e.g. IMAP msg not downloaded and user 
			// cancels login. Must escape.
			//
			if (!theMessage)
			{
				if (bCreatedDoc)
					NukeMessageDocIfUnused(doc);
				return FALSE;
			}

			message = DEBUG_NEW char[strlen(theMessage) + 1];
			strcpy(message, theMessage);
		}
		else 
		{
			if (bCreatedDoc)
				NukeMessageDocIfUnused(doc);
			return FALSE;
		}
    }

	// we really want a copy of the full message so headers can be dealt with
	char *fullMes = doc->GetFullMessage();
	if (!fullMes)
	{
		delete [] message;
		if (bCreatedDoc)
			NukeMessageDocIfUnused(doc);
		return FALSE;
	}

	// Retrieve the old subject early enough to use it in warning dialogs if necessary
	OldSubject = IsComp() ? ::SafeStrdupMT(((CCompMessageDoc*)doc)->GetHeaderLine(HEADER_SUBJECT)) : HeaderContents(IDS_HEADER_SUBJECT, fullMes);

	// Show All Headers flag off may hide some important headers, so let's get the whole thing
	mes2 = message;

#ifdef TODO
	if (!IsComp() && !ShowAllHeaders())
	{
		if (((CReadMessageDoc*)doc)->Read(SAH_YES))
			mes2 = doc->GetText();
		else
			mes2 = message;
	}
#endif

	IsRich = IsFancy(fullMes);
	if (!IsFlowed() && !IsXRich() && IsRich != IS_FLOWED)
		IsRich = IS_ASCII;

	CString sAttach;
	if (ResponseType == MS_FORWARDED || ResponseType == MS_REDIRECT)
	{
		if ( IsComp() )
			sAttach = ((CCompMessageDoc*)doc)->GetHeaderLine(HEADER_ATTACHMENTS);
		else
		{
			doc->m_QCMessage.GetAttachments(sAttach);
			if (sAttach.IsEmpty() == FALSE && ';' != sAttach[sAttach.GetLength() - 1])
				sAttach += ';';
		}
		
#ifdef IMAP4
		// If we got attachments and this is an IMAP mbox summary, we may have to 
		// download attachments. Ask user and do the download, if necessary.
		//
		if ( m_TheToc && m_TheToc->IsImapToc() )
		{
			if (!m_TheToc->m_pImapMailbox)
			{
				ASSERT (0);
			}
			else
			{
				// If the user doesn't want to download the attachments, "sAttach"
				// is cleared!!
				m_TheToc->m_pImapMailbox->DownloadAttachmentsBeforeCompose (sAttach, ResponseType);
			}
		}
#endif  // IMAP4

		// Code to handle & make sure that attachments are indeed present in the directory specified by user 
		// (if default directory is not used) 
		// This is also required in case of messages containing attachments that are being forwarded or redirected 
		// after the attachment directory is changed by the user - 02/10/2000

		char* a = sAttach.GetBuffer(sAttach.GetLength());
    	char* semi;
   		BOOL  foundSemi = FALSE;
 		CString csAllAttachNames;
		bool	bSendAttachment;
		int		nForwardingFilesNotInAttachDirAction;
		
		while(strlen(a) > 0)  //while (1)
   		{
   			if ( semi = strchr(a, ';') )
   			{
   				*semi = 0;
   				foundSemi = TRUE;
   			}
 			
			CString csAttachName = a;
			csAttachName.TrimLeft();
			FindAttachment(csAttachName);

			//	Assume that we want to send the attachment
			bSendAttachment = true;

			//	Get the setting every time through the loop so that we avoid
			//	additional dialogs if the user checks "Don't warn me anymore".
			nForwardingFilesNotInAttachDirAction = GetIniShort(IDS_INI_FWD_FILES_NOT_IN_ATTACH_DIR_ACTION);

			if ( !IsInAttachmentDir(csAttachName) && (nForwardingFilesNotInAttachDirAction != 0) )
			{
				if (nForwardingFilesNotInAttachDirAction == 1)
				{
					//	1 means ask the user
					CRString	szAction( (ResponseType == MS_FORWARDED) ? IDS_FORWARDING : IDS_REDIRECTING );

					int			nResponse = WarnOneOptionCancelDialogWithDefButton(
												IDS_INI_FWD_FILES_NOT_IN_ATTACH_DIR_ACTION,
												IDS_ERR_ATTACHMENT_NOT_IN_ATTACH_DIR,
												IDC_ATTACH, IDCANCEL,
												csAttachName, szAction, OldSubject );

					bSendAttachment = (nResponse != IDCANCEL);
				}
				else if (nForwardingFilesNotInAttachDirAction == 2)
				{
					//	2 means just don't do it
					bSendAttachment = false;
				}
			}

			if (bSendAttachment)
			{
				csAttachName += ';';
				if (FALSE != csAllAttachNames.IsEmpty())
					csAllAttachNames = csAttachName;
				else
					csAllAttachNames += csAttachName;
			}
			
			if (foundSemi)
   			{
   				*semi = ';';
   				a = semi + 1;
   			}
   			else
   				break;           
   		}
		sAttach.ReleaseBuffer();
		sAttach = csAllAttachNames;

	}

	StripAttachFromBody((char*)message, IDS_ATTACH_CONVERTED);

	if (ResponseType == MS_FORWARDED)
	{
		if (! ShowAllHeaders() )
			CReadMessageDoc::StripTabooHeaders( (char *)message );
	}

	CString csPersona;
	if (pszPersona)
	{
		//
		// Use caller-provided Persona.
		//
		csPersona = pszPersona;
	}
	else
	{
		//
		// No Persona provided, so we can either use the explicitly
		// set X-Persona header value, if any, or we can grab the
		// personality from the summary record.  The personality in
		// the summary record takes precedence if it is different than
		// the X-Persona value (i.e., the user explicitly requested a
		// persona change via filters or manually via
		// Edit:Change:Personality).
		//
		CString strSummaryPersona = GetPersona();

		// 
		// Grab the personality from the Persona X-Header, if any.
		//
		CString strXPersona;
		{
			char *pszPersonaHeader = HeaderContents(IDS_PERSONA_XHEADER, fullMes);
			if (pszPersonaHeader)
			{
				char *cp = pszPersonaHeader;

				if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
				{
					cp++;
					*(cp + strlen(cp) - 1) = '\0';
					strXPersona = cp;
				}
				delete [] pszPersonaHeader;
			}
		}
		if (strSummaryPersona != strXPersona)
			csPersona = strSummaryPersona;	// can be empty
		else
			csPersona = strXPersona;		// can be empty
	}

	// we now know the Personality for this message...
	// switch to it so the stuff that follows is Personality specific
	CString	homie =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent(csPersona);

	if (ResponseType == MS_REPLIED || ResponseType == MS_FORWARDED)
	{
		if (ResponseType == MS_REPLIED &&(!OldSubject || RemovePrefixMT("Re:", OldSubject) == OldSubject))
		{
			CRString ReplyPrefix(IDS_REPLY_PREFIX);
			NewSubject = DEBUG_NEW char[ReplyPrefix.GetLength() + ::SafeStrlenMT(OldSubject) + 1];
			strcpy(NewSubject, ReplyPrefix);
			if (OldSubject)
				strcat(NewSubject, OldSubject);
		}
		if (ResponseType == MS_FORWARDED &&
			(!OldSubject || (RemovePrefixMT("Fwd:", OldSubject) == OldSubject) && RemovePrefixMT("Fw:", OldSubject) == OldSubject))
		{
			CRString ForwardPrefix(IDS_FORWARD_PREFIX);
			NewSubject = DEBUG_NEW char[ForwardPrefix.GetLength() + ::SafeStrlenMT(OldSubject) + 1];
			strcpy(NewSubject, ForwardPrefix);
			if (OldSubject)
				strcat(NewSubject, OldSubject);
		}
		NewTo = ::SafeStrdupMT(DefaultTo);
		if (ResponseType == MS_FORWARDED)
		{
			const char* MessageText = message;
			if (IsComp() && m_State == MS_SENT && m_Seconds)
			{
				char DateBuf[80];
				ComposeDate(DateBuf, m_Seconds, m_TimeZoneMinutes);
				if (*DateBuf)
				{
					lBufLen = ::SafeStrlenMT(DateBuf) + ::SafeStrlenMT(message)+ 3;
					char* NewBuf = DEBUG_NEW_NOTHROW char[lBufLen+1];
					if (NewBuf)
					{

						*NewBuf = 0;
						strcat(NewBuf,DateBuf);
						strcat(NewBuf,"\r\n");
						if (message)
							strcat(NewBuf, message);
						MessageText = NewBuf;
					}
				}
			}
			
			QuotedBody = QuoteText(MessageText, TRUE, IsRich, "\r\n");
			if (MessageText != message)
				delete [] (char*)MessageText;
		}
		else
		{
			if (!NewTo)
			{
				NewTo = HeaderContents(IDS_HEADER_REPLY_TO, fullMes);
				if (!NewTo)
				{
					NewTo = HeaderContents(IDS_HEADER_FROM, fullMes);
					// And add it to the history list if the INI entry is on.
					if (NewTo && GetIniShort(IDS_INI_ADD_FROM_LINES_TO_HISTORY))
					{
						AutoCompList* pACL = GetAutoCompList();
						if (pACL)
							pACL->Add(NewTo, false, true);
					}
				}
			}
				
			// Add in reply intro string
			CString Attribution;
			char szDateBuf[sizeof(m_Date)+32];  // Extra long (+32) for day of week that wasn't counted on when m_date was created.
			SetDateString(szDateBuf, sizeof(m_Date)+32, GetIniShort(IDS_INI_LOCAL_TIME_DISPLAY), TRUE);

			LPCSTR rglpsz[2];
			rglpsz[0] = szDateBuf;
			rglpsz[1] = m_From;

			CString		FromSelf;

			if ( IsComp() )
			{
				// It's a comp so the attribution should actually be the current personality's
				// realname rather than the "From" field (which actually contains the To: field contents).
				GetPersRealName(FromSelf);
				rglpsz[1] = FromSelf;
			}

			if (ID_MESSAGE_REPLY_ALL == MenuItemID)
				AfxFormatStrings(Attribution, GetIniString(IDS_INI_REPLY_ALL_ATTRIBUTION), rglpsz, 2);
			else
				AfxFormatStrings(Attribution, GetIniString(IDS_INI_REPLY_ATTRIBUTION), rglpsz, 2);
			Attribution += "\r\n";

			// If we're replying to selected text, by god reply to selected text,
			// and don't try to suck away headers from the selected text  SD 2/19/99
			QuotedBody = QuoteText(selectedText ? mes2:FindBody(mes2), FALSE, IsRich, Attribution);

			if (ID_MESSAGE_REPLY_ALL == MenuItemID)
			{
				char* OldTo = HeaderContents(IDS_HEADER_TO, fullMes);
				NewCc = HeaderContents(IDS_HEADER_CC, fullMes);
				if (OldTo && *OldTo)
				{
					::TrimWhitespaceMT(OldTo);
					if (GetIniShort(IDS_INI_REPLY_TO_TO_CC))
					{
						// Original To: address(es) get moved to Cc:
						char* TmpCc = DEBUG_NEW_NOTHROW char[::SafeStrlenMT(NewCc) + 2 + ::SafeStrlenMT(OldTo) + 1];
						if (TmpCc)
						{
							if (!NewCc || !*NewCc)
								strcpy(TmpCc, OldTo);
							else
								sprintf(TmpCc, "%s, %s", OldTo, NewCc);
							delete [] NewCc;
							NewCc = TmpCc;
						}
					}
					else
					{
						// Original To: address(es) stay in To:
						char* TmpTo = DEBUG_NEW_NOTHROW char[::SafeStrlenMT(NewTo) + 2 + ::SafeStrlenMT(OldTo) + 1];
						if (TmpTo)
						{
							if (!NewTo && !*NewTo)
								strcpy(TmpTo, OldTo);
							else
								sprintf(TmpTo, "%s, %s", NewTo, OldTo);
							delete [] NewTo;
							NewTo = TmpTo;
						}
					}
				}
				delete [] OldTo;
			}
	//		ConvertAttachmentLine(QuotedBody);
		}
	}
	else if (ResponseType == MS_REDIRECT)
		NewTo = ::SafeStrdupMT(DefaultTo);

	const char* Sub = NewSubject? NewSubject : OldSubject;
	const char* Body = QuotedBody? QuotedBody : FindBody(message);
	::TrimWhitespaceMT(NewTo);
	::TrimWhitespaceMT(NewCc);

	int		nPriority = MSP_NORMAL;
	if ( GetIniShort(IDS_INI_COPY_PRIORITY) || (ResponseType == MS_REDIRECT) )
	{
		char* OldPrior = HeaderContents(IDS_HEADER_PRIORITY_MATCH, fullMes);
		if (OldPrior)
		{
			nPriority = ParsePriority(OldPrior);
			delete [] OldPrior;
		}
		else if ((OldPrior = HeaderContents(IDS_HEADER_IMPORTANCE_MATCH, fullMes)))
		{
			nPriority = ParseImportance(OldPrior);
			delete [] OldPrior;
		}
	}

	bool	bTurbo = false;
	if (ResponseType == MS_REDIRECT)
	{
		// Do turbo redirect if there's something in the To: header and
		// either exclusively the turbo redirect by default setting is on
		// or the shift key is down
		BOOL TR = GetIniShort(IDS_INI_TURBO_REDIRECT) != 0;
		bTurbo = NewTo && *NewTo && (TR != ShiftDown());
	}

	CCompMessageDoc *	comp = ComposeMessage( m_TheToc, false, NewTo, Sub, NewCc, sAttach,
											   Body, pszStationery, csPersona, ResponseType,
											   fullMes, m_Precedence, IsRich, nPriority,
											   kDontDisplayOrQueue );
	if (comp)
	{
		bSuccess = TRUE;

		// Change status of non-comp messages, saving a pointer to the orginal message
		// and original state so that it can be undone if the response is cancelled
		if ( !IsComp() )
		{
			// If we already have a response to this message, make sure the original
			// response doesn't get left with a link back to this message because
			// otherwise Bad Things (TM) will happen if you delete this message first
			// and then try to delete the original response
			if (m_ResponseSum)
				m_ResponseSum->m_RespondingToSum = NULL;
			m_ResponseSum = comp->m_Sum;
			comp->m_Sum->m_RespondingToSum = this;
			comp->m_Sum->m_RespondingToOldState = m_State;
			SetState(ResponseType);

			// Set the remote message's state also.
			if ( m_TheToc && m_TheToc->IsImapToc() )
			{
				m_TheToc->ImapSetMessageState (this, ResponseType);
			}
		}

		if (m_TheToc)
		{
			// Delete message if we're doing turbo redirect, but only if it's not already in the Trash.
		
			// bTurboRedirectOK is set to FALSE in filters to fix the exception which occurs when we hit
			// two turbo redirects for a message in filter.
			//
			if (bTurbo && m_TheToc->m_Type != MBT_TRASH && bTurboRedirectOK)
				m_TheToc->Xfer(GetTrashToc(), this);
		
			m_TheToc->SetModifiedFlag();
		}
		
		if (ID_MESSAGE_REPLY_ALL == MenuItemID)
			comp->m_Sum->m_bReplyAll = true;

		// fix up the To: and Cc: lines
		if ( (ResponseType != MS_FORWARDED) && (ID_MESSAGE_REPLY_ALL == MenuItemID) )
		{
			const BOOL bStripAll = !GetIniShort(IDS_INI_INCLUDE_SELF);

			if (bStripAll || GetIniShort(IDS_INI_STRIP_DUPLICATE_ADDRESSES))
			{
				BOOL bFoundOne;

				CString szNewTo = comp->m_Headers[ HEADER_TO ];
				bFoundOne = StripMe( szNewTo.GetBuffer(0), bStripAll );

				//	SetHeaderLine is not currently absolutely necessary (because we haven't initiated
				//	display for the user yet), but it's safer in case we change this in the future.
				comp->SetHeaderLine(HEADER_TO, szNewTo);

				CString szNewCc = comp->m_Headers[ HEADER_CC ];
				StripMe( szNewCc.GetBuffer(0), bStripAll || bFoundOne );

				//	SetHeaderLine is not currently absolutely necessary (because we haven't initiated
				//	display for the user yet), but it's safer in case we change this in the future.
				comp->SetHeaderLine(HEADER_CC, szNewCc);
			}
		}

		// fix up the From: line
		if (ResponseType == MS_REDIRECT)
		{
			char* oldFrom = HeaderContents(IDS_HEADER_FROM, fullMes);
			comp->SetupRedirect(oldFrom);
			delete [] oldFrom;
		}

		// Reply threading stuff
		if (ResponseType == MS_REPLIED)
		{
			//	Get the old References and Message-Id
			char *		szOldReferences = HeaderContents(IDS_HEADER_REFERENCES, fullMes);
			char *		szOldMessageId = HeaderContents(IDS_HEADER_MESSAGE_ID, fullMes);

			CString		szNewReferences;

			if (szOldReferences)
			{
				//	Copy the old References field to the new References field
				szNewReferences = szOldReferences;
			}
			else
			{
				//	No "References:" field in the original message
				//
				//	RFC 2822 says:
				//	If the parent message does not contain a "References:" field
				//	but does have an "In-Reply-To:" field containing a single message
				//	identifier, then the "References:" field will contain the contents
				//	of the parent's "In-Reply-To:" field followed by the contents of
				//	the parent's "Message-ID:" field (if any).
				szOldReferences = HeaderContents(IDS_HEADER_IN_REPLY_TO, fullMes);

				if (szOldReferences)
				{
					char *		pStartMessageId = strchr(szOldReferences, '<');
					
					//	Use the "In-Reply-To:" field from the original message if it
					//	contains exactly one Message Id (as determined by only one
					//	occurence of '<').
					if ( pStartMessageId && !strchr(pStartMessageId+1, '<') )
						szNewReferences = szOldReferences;
				}
			}

			//	We got some previous reference information - separate it from
			//	the message Id with a space
			if ( !szNewReferences.IsEmpty() && szOldMessageId && *szOldMessageId )
				szNewReferences += " ";

			if (szOldMessageId)
				szNewReferences += szOldMessageId;
			
			//	Calling SetHeaderLine instead of just setting the header directly doesn't
			//	really matter here because it only makes a difference when the header is
			//	visible, but just in case either of these is visible in the future go
			//	ahead and use SetHeaderLine just in case.
			comp->SetHeaderLine(HEADER_IN_REPLY_TO, szOldMessageId ? szOldMessageId : "");
			comp->SetHeaderLine(HEADER_REFERENCES, szNewReferences);

			delete [] szOldReferences;
			delete [] szOldMessageId;
		}

		//	In our call to the other form of ComposeMessage above we told it to not to
		//	handle displaying or queuing of the message so that we can do so here.
		bool	bDisplay = (!AutoSend && !bTurbo);

		if (bDisplay)
		{
			comp->m_Sum->Display();
		}
		else
		{
			//	It's important to not queue the message until here (after headers are set
			//	above) so that the header changes are properly saved.
			if (IsRich == IS_HTML)
			{
				// show the view as hidden -- this forces html to
				// get cleaned up by going through the save code
				CMDIChild::m_nCmdShow = SW_HIDE;
				comp->m_Sum->Display();
			}
			
			// save & queue the message
			comp->Queue(TRUE);
			CMDIChild::m_nCmdShow = -1;
		}
	}

	if (bDocWasCreated)
		NukeMessageDocIfUnused(doc);

	// leave the Personality as we found it
	g_Personalities.SetCurrent( homie );

	delete [] OldSubject;
	delete [] NewTo;
	delete [] NewSubject;
	delete [] NewCc;
	delete [] fullMes;
	delete [] QuotedBody;
	delete [] message;
	
	if( pSrcMessage )
	{
		delete [] *pSrcMessage;
	}

	return bSuccess;
}


CCompMessageDoc *
CSummary::ComposeMessage(
	CTocDoc *				in_pOriginatingTOC,
	bool					in_bDisplay,
	const char *			in_szTo,
	const char *			in_szSubject,
	const char *			in_szCc,
	const char *			in_szAttach,
	const char *			in_szBody,
	const char *			in_szStationery,
	const char *			in_szPersona,
	char					in_cResponseType,
	const char *			in_szECHeaders,
	const char *			in_szPrecedence,
	int						in_nIsFancy,
	int						in_nPriority,
	bool					in_bDisplayOrQueueImmediately)
{
	// we now know all the defaults - create the message
	CCompMessageDoc *	pCompMsgDoc = NewCompDocument( in_szTo, NULL, in_szSubject, in_szCc,
													   NULL, in_szAttach, in_szBody,
													   in_szStationery, in_szPersona,
													   in_cResponseType, in_szECHeaders );
	if (pCompMsgDoc)
	{
		if ( !in_szAttach || !*in_szAttach )
			pCompMsgDoc->m_Sum->UnsetFlag(MSF_HAS_ATTACHMENT);
		else
			pCompMsgDoc->m_Sum->SetFlag(MSF_HAS_ATTACHMENT);

		if (in_szPrecedence && *in_szPrecedence)
			pCompMsgDoc->m_Sum->SetPrecedence(in_szPrecedence);

		switch (in_nIsFancy)
		{
			case IS_FLOWED:
				pCompMsgDoc->m_Sum->SetFlagEx(MSFEX_FLOWED);
				break;
			case IS_HTML:
				pCompMsgDoc->m_Sum->SetFlagEx(MSFEX_HTML);
				// fall through
			case IS_RICH:
				pCompMsgDoc->m_Sum->SetFlag(MSF_XRICH);
				break;
		}
		
		if ( GetIniShort(IDS_INI_COPY_PRIORITY) || (in_cResponseType == MS_REDIRECT) )
		{
			pCompMsgDoc->m_Sum->SetPriority(in_nPriority);
		}

		// Shareware: Only allow FCC in FULL FEATURE version.
		if ( in_pOriginatingTOC && UsingFullFeatureSet() )
		{
			bool bNonSystemToc = false;
			if (in_pOriginatingTOC->IsImapToc())
			{
				if( IsInbox( in_pOriginatingTOC->m_pImapMailbox->GetImapName()) == false)
					bNonSystemToc = true;
			}
			else
			{
				//pop
				if(in_pOriginatingTOC->m_Type == MBT_REGULAR)
					bNonSystemToc = true;
			}

			if (in_cResponseType == MS_REPLIED && GetIniShort(IDS_INI_REPLY_AUTOFCC) && bNonSystemToc)
							//in_pOriginatingTOC->m_Type == MBT_REGULAR)
			{	//AutoFCC implementation.
				QCMailboxCommand*	pCommand = g_theMailboxDirector.FindByPathname( in_pOriginatingTOC->GetMBFileName() );
				if( pCommand != NULL )
				{
					pCompMsgDoc->InsertFCCInBCC( pCommand );
				}
			}
		}

		// Check to see if we need to get rid of the signature
		if ((in_cResponseType == MS_REPLIED && !GetIniShort(IDS_INI_INCLUDE_SIG_REPLY)) ||
			(in_cResponseType == MS_FORWARDED && !GetIniShort(IDS_INI_INCLUDE_SIG_FORWARD)) ||
			(in_cResponseType == MS_REDIRECT && !GetIniShort(IDS_INI_INCLUDE_SIG_REDIRECT)))
		{
			pCompMsgDoc->m_Sum->UnsetFlag(MSF_USE_SIGNATURE);
			pCompMsgDoc->m_Sum->UnsetFlag(MSF_ALT_SIGNATURE);
		}

		if (in_bDisplayOrQueueImmediately)
		{
			if (in_bDisplay)
			{
				pCompMsgDoc->m_Sum->Display();
			}
			else
			{
				if (in_nIsFancy == IS_HTML)
				{
					// show the view as hidden -- this forces html to
					// get cleaned up by going through the save code
					CMDIChild::m_nCmdShow = SW_HIDE;
					pCompMsgDoc->m_Sum->Display();
				}
				
				// save & queue the message
				pCompMsgDoc->Queue(TRUE);
				CMDIChild::m_nCmdShow = -1;
			}
		}
	}

	return pCompMsgDoc;
}


CCompMessageDoc* CSummary::SendAgain
(
	BOOL DisplayIt /*= TRUE*/,
	const char* pszPersona /*= NULL*/
)
{
	bool			bCreatedDoc;
	CMessageDoc*	doc = GetMessageDoc(&bCreatedDoc);
	CCompMessageDoc* NewDoc = NULL;
	char* Start;
	char* End;
	
	if (!doc)
		return (NULL);

	CString csPersona;
	if (pszPersona)
	{
		//
		// Use the caller-provided personality.
		//
		csPersona = pszPersona;
	}
	else
	{
		//
		// Caller didn't provide a persona, so grab it from the 
		// X-Persona header of the original message's personality.
		// We really want a copy of the full message so headers can 
		// be dealt with.
		//
		char *fullMes = doc->GetFullMessage();
		//::MessageBox(NULL,fullMes, "message", MB_OK);
		if ( fullMes )
		{
			// get the Persona X-Header
			char *PersonaHeader = HeaderContents(IDS_PERSONA_XHEADER, fullMes);
			if (PersonaHeader)
			{
				char *cp = PersonaHeader;

				if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
				{
					cp++;
					*(cp + strlen(cp) - 1) = '\0';
					csPersona = cp;
				}
				delete [] PersonaHeader;
			}

			delete [] fullMes;
		}
	}

	if (IsComp())
	{
		CCompMessageDoc* CompDoc = (CCompMessageDoc*)doc;

		// Need to make sure the message is read in if it doesn't have a window
		if (!m_FrameWnd)
			doc->GetText();

		CString theStuff;
		CompDoc->m_QCMessage.GetEmbeddedObjectHeaders( theStuff );
		
		NewDoc = NewCompDocument(CompDoc->GetHeaderLine(HEADER_TO), NULL,
			CompDoc->GetHeaderLine(HEADER_SUBJECT), CompDoc->GetHeaderLine(HEADER_CC),
			CompDoc->GetHeaderLine(HEADER_BCC), CompDoc->GetHeaderLine(HEADER_ATTACHMENTS),
			CompDoc->GetText(), NULL, csPersona, 0, theStuff);
		if (NewDoc)
		{
			NewDoc->m_Sum->SetPriority(m_Priority);
			NewDoc->m_Sum->SetTranslators(GetTranslators(), TRUE);  
			NewDoc->m_Sum->m_SigSelected = m_SigSelected;
			CString csExtra = CompDoc->m_ExtraHeaders;
			int nFind2, nFind1 = csExtra.Find(CRString(IDS_PLUGIN_INFO_HEADER));
			if (nFind1 < 0)
				nFind1 = 0;
			nFind2 = csExtra.Find("\r\n", nFind1);
			csExtra = csExtra.Mid(nFind1, nFind2- nFind1);
			if(!csExtra.IsEmpty())
				NewDoc->m_ExtraHeaders += csExtra + "\r\n";

		}
	}
	else
	{
		BOOL gotReject = FALSE;
		const char* message;
		const char* mes2;

		message = doc->GetText();

		if (!(mes2 = FindBody(message)))
		{
			mes2 = message;
		}
		else
		{
			// search for the first Recieved: line
			Start = strstr( mes2, CRString( IDS_NEWLINE_RECEIVED ) );
			
			if( Start != NULL ) 
			{
				// found it
				mes2 = Start + 2;
				gotReject = TRUE;			
			}
			else
			{
				// If this is a mailer-daemon reject, then grab the orginal message
				char RejectStrings[128];
				GetIniString(IDS_REJECT_MESSAGE_STR, RejectStrings, sizeof(RejectStrings));
				Start = RejectStrings;
				
				for (; *Start; Start = End)
				{
					End = strchr(Start, '\n');
					if (End)
						*End++ = 0;
					else
						End = Start + strlen(Start);
					if (Start = strstr(mes2, Start))
					{
						if (Start = strchr(Start, '\n'))
							mes2 = Start + 1;
						gotReject = TRUE;
						break;
					}
				}
			}
		}

		// Reset message back to begining so it'll get all info if
		// its not a reject
		if (!gotReject)
			mes2 = message;

		char* To		= HeaderContents(IDS_HEADER_TO, mes2);
		char* Subject	= HeaderContents(IDS_HEADER_SUBJECT, mes2);
		char* Cc		= HeaderContents(IDS_HEADER_CC, mes2);
		char* Bcc		= HeaderContents(IDS_HEADER_BCC, mes2);
		char* OldPrior	= HeaderContents(IDS_HEADER_PRIORITY_MATCH, mes2);
		char* OldImportance	= HeaderContents(IDS_HEADER_IMPORTANCE_MATCH, mes2);
		char* Attach = NULL;
		if ( IsComp() )
			Attach	= HeaderContents(IDS_HEADER_ATTACHMENTS, mes2);
		else
		{
			CString szAttach;
			doc->m_QCMessage.GetAttachments(szAttach);
			Attach = DEBUG_NEW char[szAttach.GetLength() + 1];
			strcpy(Attach, szAttach);
		}
	
		//Strip the attach converted lines if its a read message
		char* TempMsg = NULL;
		if (Attach && !IsComp() )
		{
			TempMsg = DEBUG_NEW char[strlen(mes2) + 1];
			strcpy(TempMsg, mes2);
			StripAttachFromBody(TempMsg, IDS_ATTACH_CONVERTED);
			mes2 = TempMsg;
		}

		NewDoc = NewCompDocument(To, NULL, Subject, Cc, Bcc, Attach, FindBody(mes2), NULL, csPersona);

		if (NewDoc)
		{
			// Grab old priority any way we can
			if (OldPrior)
				NewDoc->m_Sum->SetPriorityFromHeader(OldPrior);
			else if (OldImportance)
				NewDoc->m_Sum->SetPriorityFromImportanceHeader(OldImportance);
			else if (IsComp())
				NewDoc->m_Sum->SetPriority(m_Priority);
		}

		delete [] To;
		delete [] Subject;
		delete [] Cc;
		delete [] Bcc;
		delete [] Attach;
		delete [] OldPrior;
		delete [] TempMsg;
	}

    if (NewDoc)
	{
		// If the original message was a composition, then copy all the flags because they'll
		// have info like the encoding type, signature, and all the switches
		if (IsComp()) {
			NewDoc->m_Sum->CopyFlags(this);
			NewDoc->m_Sum->CopyFlagsEx(this);
		}
			
		if (DisplayIt)
			NewDoc->m_Sum->Display();
	}

	if (bCreatedDoc)
		NukeMessageDocIfUnused(doc);

	return (NewDoc);
}

//------------------------------------------------------------


BOOL IsHeader(const char* Line)
{
	for (const char* s = Line; *s && *s != ' ' && *s != '\t'; s++)
	{
		if (*s == ':')
			return (TRUE);
	}

	return (FALSE);
}

// GetTime
//
// If FromLine is TRUE and Line is not a valid Unix-style From line, then
// return 0L.  Otherwise, return the time given in the line, if parsable. 
// Here's a sample From line:
//   From beckley@qualcomm.com Thu Oct 15 16:15:08 1992 
//
long GetTime(const char* Line, BOOL FromLine)
{

	return GetTimeMT(Line, FromLine);

}

void CSummary::GetStatus(const char *Line)
{
	for (const char* l = Line; *l; l++)
	{
		if (toupper(*l) == 'R')
    	{
			m_State = MS_READ;
			break;
		}
		else if (toupper(*l) == 'U')
    	{
			m_State = MS_UNREAD;
			break;
		}
	}
}

// VERY IMPORTANT : THIS SHOULD BE CHANGED TO CString post 5.0
// Why : If the To: line exceeds the buffer size & you close Eudora, it WILL crash 'coz of memory overrun.
// See VI# 6242 for more detailed information
static char sBuffer[8192];	//8192 should do it for now ...

long CSummary::m_lLastTime = 0L;
long CSummary::m_lBegin = 0L;

int CSummary::Build(JJFile* in, BOOL Rebuild)
{
	int RetVal = 0;
	long lNumBytesRead = 0;		// -1 means error, 0 means EOF
	int IsOut = (m_TheToc->m_Type == MBT_OUT);
	int IsIn  = (m_TheToc->m_Type == MBT_IN);
	BOOL believeStatus = GetIniShort(IDS_INI_BELIEVE_STATUS)!=0;

	// Some strings that will be used in this routine	
	CRString AttachConvert(IDS_ATTACH_CONVERTED);
	int ACLen = AttachConvert.GetLength();
	CRString HeaderTo(IDS_HEADER_TO);
	CRString HeaderFrom(IDS_HEADER_FROM);
	CRString HeaderSubject(IDS_HEADER_SUBJECT);
	CRString HeaderCc(IDS_HEADER_CC);
	CRString HeaderBcc(IDS_HEADER_BCC);
	CRString HeaderAttachments(IDS_HEADER_ATTACHMENTS);
	CRString HeaderDate(IDS_HEADER_DATE);
	CRString HeaderStatus(IDS_HEADER_STATUS);
	CRString HeaderPriority(IDS_HEADER_PRIORITY);
	CRString HeaderImportance(IDS_HEADER_IMPORTANCE);
	CRString HeaderReplyTo(IDS_HEADER_REPLY_TO);
	CRString HeaderContentType(IDS_MIME_HEADER_CONTENT_TYPE);
	CRString HeaderTrans(IDS_TRANS_XHEADER);
	CRString HeaderSig(IDS_SIGNATURE_XHEADER);
	CString SignatureContents;

#ifdef IMAP4
	// IMAP UID is written to an "X-UID" header field when the MBX file is generated.
	CRString HeaderImapUid(IDS_IMAP_UID_XHEADER);
	CRString HeaderImapFlags(IDS_IMAP_IMFLAGS_XHEADER);
#endif

	if (!CSummary::m_lLastTime)
	{
		if (FAILED(in->GetLine(sBuffer, sizeof(sBuffer), &lNumBytesRead)) || (0 == lNumBytesRead))
			return int(lNumBytesRead);
		m_Seconds = GetTime(sBuffer, TRUE);
	}
	else
		m_Seconds = CSummary::m_lLastTime;

	if (Rebuild)
	{
		// When rebuilding use the message timestamp as the arrival time
		// (hopefully the timestamp is close to the original arrival time).
		m_lArrivalSeconds = m_Seconds;
	}
	else
	{
		// When building a new summary use the current time as the arrival time.
		CTime			 now = CTime::GetCurrentTime();
		m_lArrivalSeconds = now.GetTime();
	}

	// Turn off notifying search manager
	bool	bSaveNotifySearchManager = GetNotifySearchManager();
	SetNotifySearchManager(false);

	if (IsOut)
		SetState(MS_UNSENDABLE,Rebuild);
	else if (IsIn && Rebuild)
		SetState(MS_UNREAD,Rebuild);
	else if (Rebuild)
		SetState(MS_READ,Rebuild);

	m_Offset = CSummary::m_lBegin;

	BOOL InHeader = TRUE;
	BOOL bIncludeHeader=FALSE;
	BOOL bBodyEmpty = TRUE;
	BOOL bFoundSig = FALSE;

	BOOL bDoIncomgMoodCheck = 0;
	BOOL bDoMoodCheck = 0;
	CMoodWatch moodwatch;	
	unsigned long lBodyMoodOptions=0;
	BOOL bInSubject = FALSE;

	if (UsingFullFeatureSet())
	{
		//IDS_INI_MOOD_MAIL_CHECK is for outgoing 
/*		if ( (bDoIncomgMoodCheck = GetIniShort(IDS_INI_MOOD_MAIL_INCOMING)&& !IsComp())
			  || (GetIniShort(IDS_INI_MOOD_MAIL_CHECK) && IsComp() && Rebuild)) */
		if (Rebuild)
		{
			bDoMoodCheck = 0;
		}
		else if (bDoIncomgMoodCheck = GetIniShort(IDS_INI_MOOD_MAIL_INCOMING)&& 
			GetIniShort( IDS_INI_MOOD_MAIL_CHECK ) && !IsComp())
		{
			if(moodwatch.Init())
				bDoMoodCheck = 1;
		}
	}

	while (SUCCEEDED(in->GetLine(sBuffer, sizeof(sBuffer), &lNumBytesRead)) && (lNumBytesRead > 0))
	{
		long time = IsFromLine(sBuffer);
		if (time)
		{
			CSummary::m_lLastTime = time;
			RetVal = 1;
			break;
		}

		if (InHeader && *sBuffer)
		{
			if (*sBuffer != ' ' && *sBuffer !='\t')
			{
				bInSubject = FALSE;
				if (!strnicmp(sBuffer, HeaderTo, 3) ||
				!strnicmp(sBuffer, HeaderCc, 3) ||
				!strnicmp(sBuffer, HeaderFrom, 4)||
				!strnicmp(sBuffer, HeaderSubject, 8))
					bIncludeHeader = TRUE;
				else 
					bIncludeHeader = FALSE;
			}
			if (bIncludeHeader && bDoMoodCheck)
				moodwatch.AddText(sBuffer,strlen(sBuffer),NULL,TAE_IGNORESAFETEXT);

			if (!*m_Date && !strnicmp(sBuffer, HeaderDate, 5))
			{
				long sec = GetTime(sBuffer + 5, FALSE);
				if (sec > 0)
				{
					m_Seconds = sec;
					FormatDate(sBuffer);
				}
			}
			else if (!IsOut && !strnicmp(sBuffer, HeaderFrom, 5))
				SetFrom(GetRealName(sBuffer + 5));
			else if (IsOut && !*m_From &&
				(!strnicmp(sBuffer, HeaderTo, 3) ||
				!strnicmp(sBuffer, HeaderCc, 3) ||
				!strnicmp(sBuffer, HeaderBcc, 4)))
			{
				SetFrom(GetRealName(strchr(sBuffer, ':') + 1));
				if (*m_From && !Rebuild)
					SetState(MS_SENDABLE,Rebuild);
			}
			else if (!*m_Subject && bInSubject || !strnicmp(sBuffer, HeaderSubject, 8))
			{
				::TrimWhitespaceMT(sBuffer + 8);
				SetSubject(sBuffer + 8);
				bInSubject = TRUE;
			}
			else if (!strnicmp(sBuffer, HeaderStatus, 7))
				{ if (believeStatus) GetStatus(sBuffer + 7); }
			else if (!IsOut && !*m_From && !strnicmp(sBuffer, HeaderReplyTo, 9))
				SetFrom(GetRealName(sBuffer + 9));
			else if (!strnicmp(sBuffer, HeaderPriority, 7))
				SetPriorityFromHeader(sBuffer + 11);
			else if (!strnicmp(sBuffer, HeaderImportance, 11))
				SetPriorityFromImportanceHeader(sBuffer + 12);
//			else if (!strnicmp(sBuffer, HeaderAttachments, 14) ||
//				!strnicmp(sBuffer, ((const char*)HeaderAttachments) + 2, 12))
//			{
//				if ((strlen(sBuffer)) > 16)
//					SetFlag(MSF_HAS_ATTACHMENT);
//			}
			else if (!strnicmp(sBuffer, HeaderContentType, 13))
			{
				if (strstr(sBuffer, CRString(IDS_MIME_ENRICHED)))
					SetFlag(MSF_XRICH);
				if (strstr(sBuffer, CRString(IDS_MIME_HTML)))
				{
					SetFlag(MSF_XRICH);
					SetFlagEx(MSFEX_HTML);
				}
			}
    		else if ( !strnicmp(sBuffer, HeaderTrans, 15) )
				SetTranslators(sBuffer + 15);
    		else if ( !strnicmp(sBuffer, HeaderSig, 20) )
			{
				char *cp = (sBuffer + 20);

				if (*cp == '<' && *(cp + strlen(cp) - 1) == '>')
				{
					cp++;
					*(cp + strlen(cp) - 1) = '\0';
					SignatureContents = cp;
				}
			}
#ifdef IMAP4	// "X-UID:" header. Note: string includes the colon.
			else if ( !strnicmp(sBuffer, HeaderImapUid, 6) )
			{
				CString suid = sBuffer + 6;
				suid.TrimLeft();
				suid.TrimRight();
				m_Hash = (unsigned long)atol (suid);
			}
			// X-IMFLAGS: RAFDF for \Read, \Answered, \Flagged, \Deleted and \Draft resp.
    		else if ( !strnicmp(sBuffer, HeaderImapFlags, 9) )
			{
				unsigned long ImapParseImflags (LPCSTR pImflags);
				CString Imflags = sBuffer + 9;
				Imflags.TrimLeft();
				Imflags.TrimRight();
				m_Imflags = ImapParseImflags (Imflags);
				//The above value set can be tested by manually modifying an 
				//IMAP mbx file and adding X-IMFLADS:RAFDF as one of lines
				//in the header for the message. Modifying the mbx file will 
				//result in eudora rebuilding the toc ( since it thinks its corrupted) and
				//ImapParseFlags will be called because of the header. 
				//This flag was never being used in the past due to hiding of m_Imflags by local
				//instance. - sagar 7/19/2000.
				
			}
#endif // IMAP4
		}
		else
		{
			if (Rebuild)
			{
				if (InHeader)
				{
					if (IsOut)
					{
						SetSignature(SignatureContents);

						if (GetIniShort(IDS_INI_SEND_MIME))		SetFlag(MSF_MIME);
						if (GetIniShort(IDS_INI_SEND_UUENCODE))	SetFlag(MSF_UUENCODE);
						if (GetIniShort(IDS_INI_USE_QP))		SetFlag(MSF_QUOTED_PRINTABLE);
						if (GetIniShort(IDS_INI_WORD_WRAP))		SetFlag(MSF_WORD_WRAP);
						if (GetIniShort(IDS_INI_TABS_IN_BODY))	SetFlag(MSF_TABS_IN_BODY);
						if (GetIniShort(IDS_INI_KEEP_COPIES))	SetFlag(MSF_KEEP_COPIES);
						if (GetIniShort(IDS_INI_TEXT_AS_DOC))	SetFlag(MSF_TEXT_AS_DOC);
					}
				}
				else if (!strnicmp(sBuffer, AttachConvert, ACLen))
					SetFlag(MSF_HAS_ATTACHMENT);
			}

			if (InHeader)
				bBodyEmpty = IsHTML()? FALSE : TRUE;
			else
			{
				if (!IsHTML())
				{
					if (bBodyEmpty && !bFoundSig)
					{
						if (strcmp(sBuffer, "-- ") == 0)
							bFoundSig = TRUE;
						else if (strcmp(sBuffer, "<x-flowed>") && strcmp(sBuffer, "</x-flowed>"))
						{
							int i = strspn(sBuffer, " \t");
							if (sBuffer[i] != 0)
								bBodyEmpty = FALSE;
						}
					}
					if (!bBodyEmpty)
					{
						switch (::IsFancy(sBuffer))
						{
						case IS_HTML:
							SetFlagEx(MSFEX_HTML);
							// fall through
						case IS_RICH:
							SetFlag(MSF_XRICH);
							lBodyMoodOptions = TAE_CONTAINSHTML;
							break;
						}
					}
				}
				else
					lBodyMoodOptions = TAE_CONTAINSHTML;
			}

			if (bDoMoodCheck)
				moodwatch.AddText(sBuffer,strlen(sBuffer),NULL,lBodyMoodOptions);

			InHeader = FALSE;
		}

		in->Tell(&CSummary::m_lBegin);
		ASSERT(CSummary::m_lBegin >= 0);
	}

	if (!RetVal)
		CSummary::m_lLastTime = 0L;
		
	if (lNumBytesRead < 0)
	{
		// Restore notifying search manager
		SetNotifySearchManager(bSaveNotifySearchManager);
		
		return (-1);
	}

	if (bBodyEmpty)
		SetFlagEx(MSFEX_EMPTY_BODY);
	else
		UnsetFlagEx(MSFEX_EMPTY_BODY);

	if (bDoMoodCheck)
	{
		int nRetVal = moodwatch.GetScore(); 
		if (nRetVal != -1)
			SetMood((char)(nRetVal+ 1));
	}

	m_Length = CSummary::m_lBegin - m_Offset;
	if (!*m_Date)
	{
		m_TimeZoneMinutes = -GetGMTOffset();
		m_Seconds -= m_TimeZoneMinutes * 60;
		FormatDate();
	}

	// For IMAP:
	if (m_MsgSize <= 1)
		m_MsgSize = (unsigned short) max( (m_Length/1024 + 1), 1 );

	// Restore notifying search manager
	SetNotifySearchManager(bSaveNotifySearchManager);
		
	return (RetVal);
}

static const int SumUnusedDiskSpace = 2;
 
int CSummary::Read(JJFile* in)
{
	// Turn off notifying search manager
	bool	bSaveNotifySearchManager = GetNotifySearchManager();
	SetNotifySearchManager(false);
	
	// Default these to some arbitrary values.
	short x1 = 0, y1 = 0, x2 = 400, y2 = 300, tzm = 0;
	char nIgnoreChar;
	unsigned char c = 0;
	HRESULT hrRead = S_OK;		// way up here 'cos of the goto's

    if (FAILED(in->Get(&m_Offset))) goto fail;
	if (FAILED(in->Get(&m_Length))) goto fail;
	if (FAILED(in->Get(&m_Seconds))) goto fail;
	if (FAILED(in->Get(&m_State))) goto fail;
	if (FAILED(in->Get(&m_Flags))) goto fail;
	if (FAILED(in->Get(&m_Priority))) goto fail;
	if (FAILED(in->Get(&nIgnoreChar))) goto fail;
	// We need some more room for data.  The date string field is regenerated
	// based on m_Seconds so in reality we could just remove it altogether.
	// However, most times the date string does not take up the full 32 bytes
	// so we steal the last 4 for the arrival timestamp.  In most cases these
	// bytes will be NULL and the behavior will not change at all.
	if (FAILED(in->Read(m_Date, sizeof(m_Date) - 4))) goto fail;
	if (FAILED(in->Get(&m_lArrivalSeconds))) goto fail;
	if (FAILED(in->Read(m_From, sizeof(m_From)))) goto fail;
	if (FAILED(in->Read(m_Subject, sizeof(m_Subject)))) goto fail;

	if (FAILED(in->Get(&x1))) goto fail;
	if (FAILED(in->Get(&y1))) goto fail;
	if (FAILED(in->Get(&x2))) goto fail;
	if (FAILED(in->Get(&y2))) goto fail;
	m_SavedPos.SetRect(x1, y1, x2, y2);

	if (FAILED(in->Get(&m_Label))) goto fail;
	if (FAILED(in->Get(&m_Hash))) goto fail;
	if (FAILED(in->Get(&m_UniqueMessageId))) goto fail;
	if (FAILED(in->Get(&m_FlagsEx))) goto fail;
	if (FAILED(in->Get(&m_PersonaHash))) goto fail;
	if (FAILED(in->Get(&tzm))) goto fail;
	m_TimeZoneMinutes = tzm;

	SetDate();

// IMAP4
	if (FAILED(in->Get(&m_Imflags))) goto fail;
	if (FAILED(in->Get(&m_MsgSize))) goto fail;

	// Read the number of un-downloaded attachments from the appropriate bits of m_Imflags.
	m_nUndownloadedAttachments = (m_Imflags & IMFLAGS_UNDOWNLOADED_ATTACHMENTS) >> UNDOWNLOADED_ATTACHMENTS_BIT_SHIFT;

	if (m_MsgSize <= 1)
		m_MsgSize = (unsigned short) max( (m_Length/1024 + 1), 1 );
// END IMAP
	if (FAILED(in->Get(&m_nMood))) goto fail;
	// Get the junk score byte.
	if (FAILED(in->Get((char*)&c))) goto fail;
	if (c & 0x80)
	{
		// If the high bit is set note that this message was manually junked
		// and unset the high bit.
		m_bManuallyJunked = true;
		c &= 0x7F;
	}
	m_ucJunkScore = c;
	if (FAILED(in->Get(&m_ulJunkPluginID))) goto fail;

	// Restore notifying search manager
	SetNotifySearchManager(bSaveNotifySearchManager);

	hrRead = in->Read(sBuffer, SumUnusedDiskSpace);
	if (FAILED(hrRead))
		return -1;
	else if (S_OK == hrRead)
		return SumUnusedDiskSpace;
	else if (S_FALSE == hrRead)
		return 0;		// hit EOF
	else
	{
		ASSERT(0);		// unexpected status
		return -1;
	}

fail:
	// Restore notifying search manager
	SetNotifySearchManager(bSaveNotifySearchManager);

	return (-1);
}


int CSummary::Write(JJFile *out)
{
	unsigned char c = 0;

    if (FAILED(out->Put(m_Offset))) goto fail;
	if (FAILED(out->Put(m_Length))) goto fail;
	if (FAILED(out->Put(m_Seconds))) goto fail;
	if (FAILED(out->Put(m_State))) goto fail;
	if (FAILED(out->Put(m_Flags))) goto fail;
	if (FAILED(out->Put((char)m_Priority))) goto fail;
	if (FAILED(out->Put((char)0))) goto fail;
	// See note on Read() about stealing bytes from date field.
	if (FAILED(out->Put(m_Date, sizeof(m_Date) - 5))) goto fail;
	// NULL terminate the date field.
	if (FAILED(out->Put((char)0))) goto fail;
	if (FAILED(out->Put(m_lArrivalSeconds))) goto fail;
	if (FAILED(out->Put(m_From, sizeof(m_From)))) goto fail;
	if (FAILED(out->Put(m_Subject, sizeof(m_Subject)))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.left))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.top))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.right))) goto fail;
	if (FAILED(out->Put((short)m_SavedPos.bottom))) goto fail;
	if (FAILED(out->Put(m_Label))) goto fail;
	if (FAILED(out->Put(m_Hash))) goto fail;
	if (FAILED(out->Put(m_UniqueMessageId))) goto fail;
	if (FAILED(out->Put(m_FlagsEx))) goto fail;
	if (FAILED(out->Put(m_PersonaHash))) goto fail;
	if (FAILED(out->Put((short)m_TimeZoneMinutes))) goto fail;

#ifdef IMAP4
	// Clear undownloaded attachment bits
	m_Imflags &= ~IMFLAGS_UNDOWNLOADED_ATTACHMENTS;

	// Format the number of un-downloaded attachments into the appropriate bits of m_Imflags
	m_Imflags |= m_nUndownloadedAttachments << UNDOWNLOADED_ATTACHMENTS_BIT_SHIFT;

	if (FAILED(out->Put(m_Imflags))) goto fail;
	if (FAILED(out->Put(m_MsgSize))) goto fail;
#endif

	if (FAILED(out->Put((char)m_nMood))) goto fail;
	c = m_ucJunkScore;
	// If message was manually junked note this by setting the high bit.
	if (m_bManuallyJunked)
	{
		c |= 0x80;
	}
	if (FAILED(out->Put((char)c))) goto fail;
	if (FAILED(out->Put(m_ulJunkPluginID))) goto fail;
	memset(sBuffer, 0, SumUnusedDiskSpace);
	if (FAILED(out->Put(sBuffer, SumUnusedDiskSpace))) goto fail;

	return (1);

fail:
	return (-1);
}

void CSummary::RedisplayField(int field, BOOL SetDirty /*= TRUE*/)
{
	if (!m_TheToc)
	{
		// If there is no TOC there really isn't anything to do.
		// This can legitimately happen in certain junk cases.
		return;
	}

	//
	// The server status bits are actual stored in the LMOS file, so
	// don't mark the TOC as being dirty for server status changes.
	//
    if (SetDirty && ((field != FW_SERVER) 
#ifdef IMAP4
	// Ignore field test if this is an IMAP mailbox:
		|| m_TheToc->IsImapToc()
#endif	
	) )
	{
        m_TheToc->SetModifiedFlag();

		// Notify search manager if indicated, unless the document was spooled.
		// We don't allow users to search for spooled documents, and they just
		// get set to sent in another moment anyway.
		if ( m_bNotifySearchManager &&
			 ((field != FW_STATUS) || (GetState() != MS_SPOOLED)) )
		{
			SearchManager::Instance()->NotifySumModification(m_TheToc, this);
		}
	}

	m_TheToc->UpdateAllViews(NULL, field, this);

	if (FW_SERVER == field)
	{
		if (m_FrameWnd)
		{
			// give Frame a chance to display new server status
			extern UINT umsgButtonSetCheck;
			m_FrameWnd->SendMessage(umsgButtonSetCheck, ID_DELETE_FROM_SERVER, IsDeleteFromServerSet());
			m_FrameWnd->SendMessage(umsgButtonSetCheck, ID_DOWNLOAD_FROM_SERVER, IsDownloadFromServerSet());
		}
	}

	// We've changed some summary info in a field, so we need
	// to resort if the mailbox is being sorted by that field
	for (int i = 0; i < m_TheToc->NumSortColumns; i++)
	{
		const SortType st = m_TheToc->GetSortMethod(i);
		if (st == NOT_SORTED)
			break;
		if ((st - (st < BY_REVERSE_STATUS? 1 : BY_REVERSE_STATUS)) == field)
		{
			m_TheToc->SetNeedsSorting();
			break;
		}
	}
}

void CSummary::SetState(char State, BOOL bRebuild)
{
	if (m_State != State)
	{
		m_State = State;
		RedisplayField(FW_STATUS);


		// If we are rebuilding the TOC & hence the call to SetState of individual summary, then we don't need to account these mails for the 
		// maintaining/calculating statistics 'coz these have already been accounted for before. As mentioned above, one reason of this happening 
		// is when the TOC gets corrupted & is rebuilt.
		if (!bRebuild)
		{
			try
			{

				// Depending upon the State of the Message, appropriate Usage Statistic counter is updated
				switch(m_State)
				{
					case MS_UNREAD:
						break;
					case MS_READ:
						UpdateNumStat(US_STATREADMSG,1,m_Seconds + ( m_TimeZoneMinutes *60));
						break;
					case MS_SENT :
						// The logic for updating Usage Statistics counter for SENT messages is in sendmail.cpp (PostProcessOutgoingMessages())
						break;					
					default:
						break;
				}
			}
			catch (CMemoryException * /* pMemoryException */)
			{
				// Catastrophic memory exception - rethrow
				ASSERT( !"Rethrowing CMemoryException in CSummary::SetState" );
				throw;
			}
			catch (CException * pException)
			{
				// Other MFC exception
				pException->Delete();
				ASSERT( !"Caught CException (not CMemoryException) in CSummary::SetState" );
			}
			catch (std::bad_alloc & /* exception */)
			{
				// Catastrophic memory exception - rethrow
				ASSERT( !"Rethrowing std::bad_alloc in CSummary::SetState" );
				throw;
			}
			catch (std::exception & /* exception */)
			{
				ASSERT( !"Caught std::exception (not std::bad_alloc) in CSummary::SetState" );
			}
		}		
	}
}

void CSummary::SetTranslators(const char *trans, BOOL bValidate /* = FALSE */)
{ 
    if (*trans == '<') 
        trans++;

    int end = strlen(trans) - 1;

// BOG: so much for "const char" w/this following line. Very ugly coding, imo.
    if (*(trans + end) == '>') 
        *((char *)trans + end) = 0;

    // validating the translators will cause an error message to be displayed
    // for trans ids without assoc translators---the actual plugins can't be
    // identified tho. Bad ids will be stripped out of the list.

    if ( bValidate ) {
        CTranslatorManager* tmgr = ((CEudoraApp*)AfxGetApp())->GetTranslators();
        long lFlags = EMSF_Q4_TRANSMISSION | EMSF_Q4_COMPLETION;
        tmgr->ValidateTransIDs( trans, lFlags, m_TransHdr );
    }
    else
        m_TransHdr = trans;
}



void CSummary::SetPriority(int Priority, BOOL bUpdateFrame /* = TRUE */)
{
	if (m_Priority != Priority)
	{
		m_Priority = (char)Priority;
		RedisplayField(FW_PRIORITY);

		if ((bUpdateFrame) && (m_FrameWnd))
		{
			//
			// Notify the frame window of the priority change so that it
			// can update its UI.
			//
			extern UINT umsgSetPriority;
			m_FrameWnd->SendMessage(umsgSetPriority, 0, LPARAM(this));
		}
	}
}

void CSummary::SetLabel(int Label)
{
	if (m_Label != Label)
	{
		m_Label = (short)Label;
		RedisplayField(FW_LABEL);
	}
}

void CSummary::SetServerStatus(UINT nID)
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();

	if (pMsgRecord)
	{
		const LMOS_DELETE_FLAG OldDeleteFlag = pMsgRecord->GetDeleteFlag();
		const LMOS_RETRIEVE_FLAG OldRetrieveFlag = pMsgRecord->GetRetrieveFlag();

		if (nID == ID_MESSAGE_SERVER_FETCH || nID == ID_MESSAGE_SERVER_FETCH_DELETE)
			pMsgRecord->SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE /*0*/);
		else
			pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE /*1*/);

		if (pMsgRecord->GetDeleteFlag() != LMOS_DELETE_SENT /*-1*/)
		{
			if (nID == ID_MESSAGE_SERVER_DELETE || nID == ID_MESSAGE_SERVER_FETCH_DELETE)
				pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE /*0*/);
			else
				pMsgRecord->SetDeleteFlag(LMOS_DONOT_DELETE /*1*/);
		}

		if (OldDeleteFlag != pMsgRecord->GetDeleteFlag() || OldRetrieveFlag != pMsgRecord->GetRetrieveFlag())
		{
			LMOSRecord.WriteLMOS();
			RedisplayField(FW_SERVER);
		}
	}
}

int CSummary::ParsePriority(const char * szHeaderContents)
{
	int		nPriority = MSP_NORMAL;
	if (szHeaderContents)
	{
		nPriority = atoi(szHeaderContents);
		if ( (nPriority < MSP_HIGHEST) || (nPriority > MSP_LOWEST) )
			nPriority = MSP_NORMAL;
	}

	return nPriority;
}

#define IsWhite(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')

int CSummary::ParseImportance(const char * szHeaderContents)
{
	if (szHeaderContents)
	{
		while (IsWhite(*szHeaderContents)) szHeaderContents++;
		CRString highest(IDS_PRIORITY_HIGHEST);
		if (!strncmp(highest,szHeaderContents,highest.GetLength())) return MSP_HIGHEST;
		CRString high(IDS_PRIORITY_HIGH);
		if (!strncmp(high,szHeaderContents,high.GetLength())) return MSP_HIGH;
		CRString low(IDS_PRIORITY_LOW);
		if (!strncmp(low,szHeaderContents,low.GetLength())) return MSP_LOW;
		CRString lowest(IDS_PRIORITY_LOWEST);
		if (!strncmp(lowest,szHeaderContents,lowest.GetLength())) return MSP_LOWEST;
	}

	return MSP_NORMAL;
}

void CSummary::SetFrom(const char* NewFrom)
{
    if (NewFrom && !strcmp(m_From, NewFrom))
        return; 
    memset(m_From, 0, sizeof(m_From));
    if (NewFrom)
    	strncpy(m_From, NewFrom, sizeof(m_From) - 1);
    RedisplayField(FW_SENDER);
}

void CSummary::SetSubject(const char* NewSubject)
{
    if (NewSubject && !strcmp(m_Subject, NewSubject))
        return; 
    memset(m_Subject, 0, sizeof(m_Subject));
    if (NewSubject)
    	strncpy(m_Subject, NewSubject, sizeof(m_Subject) - 1);
    RedisplayField(FW_UNUSED);
}


void CSummary::SetPersona(const char *persona)
{
	unsigned long	nPersonaHash = g_Personalities.GetHash( persona );

	if ( (nPersonaHash != m_PersonaHash) && m_TheToc )
	{
		m_TheToc->SetModifiedFlag();
		
		SearchManager::Instance()->NotifySumModification(m_TheToc, this);
	}

	m_PersonaHash = nPersonaHash;
}

CString CSummary::GetPersona()
{
	return g_Personalities.GetByHash( m_PersonaHash );
}

void CSummary::SetSignature( const char *sig)
{
	m_SigHdr = sig;
	m_SigSelected = sig;
	if (sig && *sig)
	{
		m_Flags |= MSF_ALT_SIGNATURE; 
		m_Flags &= ~MSF_USE_SIGNATURE;
	}

	if (m_FrameWnd)
	{
		//
		// Notify the frame window of the signature change so that it
		// can update its UI.
		//
		extern UINT umsgSetSignature;
		m_FrameWnd->SendMessage(umsgSetSignature, 0, LPARAM(this));
	}
}


// SetDate
//
// Sets the internal date/time/timezone of a summary if a value is specified as an argument,
// which should only happen if this is a composition message OR if this is a junk message
// and the user wants to ignore the message's date on junk.  Also, the m_Date string field
// is updated to contain the correct display for mailboxes.
//
void CSummary::SetDate(time_t Seconds /*= 0L*/, BOOL bJunk)
{
	if (Seconds)
	{
		ASSERT(IsComp() || bJunk);
		m_Seconds = Seconds;
		m_TimeZoneMinutes = -GetGMTOffset();
	}

	char DateBuf[sizeof(m_Date)+32];
	m_Date[sizeof(m_Date) - 1] = 0;

	SetDateString(DateBuf, sizeof(m_Date)+32,
				  GetIniShort(IDS_INI_LOCAL_TIME_DISPLAY), 
				  GetIniShort(IDS_INI_FIXED_DATE_DISPLAY));

	if (strcmp(m_Date, DateBuf))
	{
		strncpy(m_Date, DateBuf, sizeof(m_Date)-1);
		RedisplayField(FW_DATE, !bJunk);
	}
}


////////////////////////////////////////////////////////////////////////
// SetDateString [public]
//
// Helper function for formatting the date string displayed in the TOC.
////////////////////////////////////////////////////////////////////////
void CSummary::SetDateString(char* pszDateBuf, int nSize, time_t nSeconds, int nTimeZoneMinutes, BOOL bDisplayLocalTime, BOOL bUseFixedFormat)
{
	ASSERT(pszDateBuf != NULL);
	ASSERT(nSize > 0);

	::memset(pszDateBuf, 0, nSize);
	char localDateBuf[128];
	*localDateBuf = 0;

	if (nSeconds)
	{
		time_t DisplayDate = nSeconds + nTimeZoneMinutes * 60;
		if (bDisplayLocalTime)
		{
			DisplayDate = nSeconds - (GetGMTOffset() * 60);
			nTimeZoneMinutes = INT_MAX;
		}

		if (bUseFixedFormat)
			::TimeDateStringFormatMT(localDateBuf, DisplayDate, nTimeZoneMinutes, GetIniString(IDS_INI_FIXED_DATE_FORMAT));
		else
		{
			time_t Age = time(NULL) - DisplayDate;
			if (Age > 60 * 60 * GetIniLong(IDS_INI_ANCIENT_DATE_HOURS))
				::TimeDateStringFormatMT(localDateBuf, DisplayDate, nTimeZoneMinutes, GetIniString(IDS_INI_ANCIENT_DATE_FORMAT));
			else if (Age > 60 * 60 * GetIniLong(IDS_INI_OLD_DATE_HOURS))
				::TimeDateStringFormatMT(localDateBuf, DisplayDate, nTimeZoneMinutes, GetIniString(IDS_INI_OLD_DATE_FORMAT));
			else
				::TimeDateStringFormatMT(localDateBuf, DisplayDate, nTimeZoneMinutes, GetIniString(IDS_INI_RECENT_DATE_FORMAT));
		}
	}

	strncpy(pszDateBuf,localDateBuf,MIN(nSize-1,(int)strlen(localDateBuf)));
	ASSERT((int)strlen(pszDateBuf) < nSize); // better late than never
}


void CSummary::FormatDate(const char* GMTOffset)
{
	if (!m_Seconds)
	{
		SetDate();
		return;
	}

	FormatDateMT(GMTOffset,m_Seconds,m_TimeZoneMinutes);

	SetDate();
}

void CSummary::SetMood(char NewMood)
{
    if (m_nMood != NewMood)
	{
		m_nMood = NewMood;
		RedisplayField(FW_MOOD);
	}
}

// Abort procedure for printing
//extern BOOL CALLBACK AFX_EXPORT _AfxAbortProc(HDC, int);

void CSummary::PrintSum()
{
	BOOL		bWasOpen;
	QCProtocol*	pProtocol;

	bWasOpen = (m_FrameWnd != NULL);

	CMDIChild::m_nCmdShow = SW_HIDE;
	
	if( !m_FrameWnd )
	{
		Display();
	}
	
	pProtocol = QCProtocol::QueryProtocol( QCP_QUIET_PRINT, m_FrameWnd );
	
	if( pProtocol )
	{
		pProtocol->QuietPrint();
	}

	CMDIChild::m_nCmdShow = -1;

	if( !bWasOpen )
	{
		GetMessageDoc()->OnCloseDocument();
	}

}

void CSummary::ConvertAttachmentLine( char* buf )
{
	CString szPath;
	CRString szAttach(IDS_ATTACH_CONVERTED);
	int szAttachLen = szAttach.GetLength();

	char* pos=buf;

	while ( pos = strstr(buf, szAttach ) )
	{
		pos += szAttachLen +1;
		// Replace first quote with a space
		*pos = ' ';
		// Replace second quote with a space
		for (; *pos != '\"' && pos; pos++ )
			*pos = ' ';		
	}
}

BOOL CSumList::Print()
{
	CEudoraApp *theApp = (CEudoraApp *)AfxGetApp();
	POSITION pos = GetHeadPosition();

	if (pos)
	{
		// disable main window while printing & init printing status dialog
		AfxGetMainWnd()->EnableWindow(FALSE);
	
		CountdownProgress(CRString(IDS_PRINTING_MESSAGES_LEFT), GetCount());
		
		//BUG FIX 1793.  If no printer is attatched then let's not go any further.  If we allow 
		//this to continue strange stuff happens.
		CPrintInfo printInfo;
		if (!theApp->GetPrinterDeviceDefaults(&printInfo.m_pPD->m_pd))
		{
			CloseProgress();
			AfxGetMainWnd()->EnableWindow(TRUE);
			// bring up dialog to alert the user they need to install a printer.
			if (theApp->DoPrintDialog(printInfo.m_pPD) != IDOK)
				return FALSE;
		}
		
		while (pos)
		{
			CSummary* sum = GetNext(pos);
			if (sum)
			{
				Progress(-1, sum->GetTitle(), -1);
				sum->PrintSum();
			}
			if (EscapePressed(1))
				break;
			
			DecrementCountdownProgress();
		}
		CloseProgress();
		AfxGetMainWnd()->EnableWindow();    // enable main window
	}
	return (TRUE);
}


////////////////////////////////////////////////////////////////////////
// FindNextByMessageId [public]
//
// This is tricky.  Returns the POSITION of the *next* item, if any,
// that matches the search criteria in the list *after* the item with
// the given 'messageId'.  A messageId of 0xFFFFFFFF is a special case
// indicating that we want the first element in the list that meets
// the search criteria.
//
// This function supports Simple MAPI.
//
////////////////////////////////////////////////////////////////////////
POSITION CSumList::FindNextByMessageId(unsigned long messageId, BOOL unreadOnly)
{
	//
	// Get next item in the list matching the search criteria.
	//
	POSITION pos = GetHeadPosition();		// can be NULL
	BOOL return_next_match = (0xFFFFFFFF == messageId) ? TRUE : FALSE;
	while (pos != NULL)
	{
		CSummary* p_sum = GetAt(pos);
		ASSERT(p_sum != NULL);

		if (unreadOnly && (MS_UNREAD != p_sum->m_State))
		{
			GetNext(pos);		// skip it since we want only unread messages
			continue;
		}

		if (return_next_match)
			break;
		else if (p_sum->GetUniqueMessageId() == (long)messageId)
			return_next_match = TRUE;

		//
		// Move ahead to the *next* message, if any.
		//
		GetNext(pos);
	}

	return pos;		// can be NULL
}


////////////////////////////////////////////////////////////////////////
// GetByMessageId [public]
//
// Search the list for a message with a matching messageId and return
// a pointer to the summary object.  Silently return NULL if there is
// no match.
//
// This function supports Simple MAPI.
//
////////////////////////////////////////////////////////////////////////
CSummary *
CSumList::GetByMessageId(unsigned long messageId)
{
	// Verify that the counts match up if we're using maps
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapMessageIDToSummary.size())) );
	ASSERT( !m_bUseMaps || (m_ObList.GetCount() == static_cast<INT_PTR>(m_mapSummaryToPosition.size())) );

	CSummary *	pSummary = NULL;
	
	ASSERT(messageId != 0);
	if (messageId != 0)
	{
	#ifdef _DEBUG
		// When running a debug build, we verify cases where we couldn't
		// find the message ID by searching using the old linear method.
		// That way we only ASSERT if the new faster method fails when
		// the old linear method would have worked. This avoids ASSERTing
		// in cases where it's ok that we're returning NULL.
		bool	bVerifyUsingOldWay = false;
	#endif
		
		if (m_bUseMaps)
		{
			// We've been maintaining the maps, so find it the fast way
			MessageIDMapIteratorT	iterator = m_mapMessageIDToSummary.find(messageId);

			if ( iterator != m_mapMessageIDToSummary.end() )
			{
				pSummary = iterator->second;

				// ASSERT that the map found the correct summary
				ASSERT( pSummary->GetUniqueMessageId() == static_cast<long>(messageId) );
			}
		#ifdef _DEBUG
			else
			{
				bVerifyUsingOldWay = true;
			}
		#endif
		}

	#ifdef _DEBUG
		if (!m_bUseMaps || bVerifyUsingOldWay)
	#else
		if (!m_bUseMaps)
	#endif
		{
			// Can't use maps so find it the slow way
			POSITION pos = GetHeadPosition();		// can be NULL
			while (pos != NULL)
			{
				CSummary *  pScanSum = GetNext(pos);
				ASSERT(pScanSum);

				if ( pScanSum && (pScanSum->GetUniqueMessageId() == static_cast<long>(messageId)) )
				{
					// We found the matching summary - stop the search
					pSummary = pScanSum;
					break;
				}
			}

			// We want to catch cases where we're doing verification and we found
			// a summary (which is bad because above we didn't find a summary).
			// ASSERT that we're not verifying or that we didn't find the summary.
			ASSERT( !bVerifyUsingOldWay || !pSummary );
		}
	}

	return pSummary;
}



////////////////////////////////////////////////////////////////////////
// GetByMessageUid [public]
//
// Search the list for a message with a matching hash and return
// a pointer to the summary object.  Silently return NULL if there is
// no match.
//
// This function supports IMAP.
//
////////////////////////////////////////////////////////////////////////
CSummary* CSumList::GetByUid(unsigned long Uid)
{
	if (0 == Uid)
	{
		ASSERT(0);
		return NULL;
	}

	POSITION pos = GetHeadPosition();		// can be NULL
	while (pos != NULL)
	{
		CSummary* p_sum = GetNext(pos);
		ASSERT(p_sum != NULL);

		if (p_sum->GetHash() == Uid)
			return p_sum;
	}

	return NULL;		// can be NULL
}



void CSummary::SetSavedPos( const CRect& thePosition )
{
	m_SavedPos = thePosition;
	m_TheToc->SetModifiedFlag();
}

void CSummary::ToggleDeleteFromServer()
{
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	if ( pMsgRecord )
	{
		if( pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE)
			pMsgRecord->SetDeleteFlag(LMOS_DONOT_DELETE);
		else
			pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE);

		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::ToggleDownloadFromServer()
{
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		if(pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE)
			pMsgRecord->SetRetrieveFlag(LMOS_DONOT_RETRIEVE);
		else
			pMsgRecord->SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE);
		
		// If a message gets marked for retrieve and LMOS is off, then set for removal
		// if not already set as such
		if (pMsgRecord->GetRetrieveFlag() == LMOS_DONOT_RETRIEVE && !GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) &&
			pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE /*0*/)
		{
			pMsgRecord->SetDeleteFlag( LMOS_DONOT_DELETE /*1*/ );
		}

		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


BOOL CSummary::IsMessageIsOnServer()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	return (pMsgRecord)?TRUE:FALSE;
	
}


BOOL CSummary::IsDeleteFromServerSet()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		//return !( pMsgRecord->GetDeleteFlag() );
		return ( pMsgRecord->GetDeleteFlag() == LMOS_DELETE_MESSAGE);
	}

	return FALSE;
}


BOOL CSummary::IsDownloadFromServerSet()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord )
	{
		//return !( pMsgRecord->GetRetrieveFlag() );
		return ( pMsgRecord->GetRetrieveFlag() == LMOS_RETRIEVE_MESSAGE);
	}

	return FALSE;
}


void CSummary::SetMessageServerLeave()
{
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord && (pMsgRecord->GetDeleteFlag() != LMOS_DONOT_DELETE || pMsgRecord->GetRetrieveFlag() != LMOS_DONOT_RETRIEVE))
	{
		pMsgRecord->SetDeleteFlag( LMOS_DONOT_DELETE /*1*/ );
		pMsgRecord->SetRetrieveFlag( LMOS_DONOT_RETRIEVE /*1*/ );

		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::SetMessageServerDelete()
{
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();

	if ( pMsgRecord && (pMsgRecord->GetDeleteFlag() != LMOS_DELETE_MESSAGE || pMsgRecord->GetRetrieveFlag() != LMOS_DONOT_RETRIEVE))
	{
		pMsgRecord->SetDeleteFlag( LMOS_DELETE_MESSAGE /*0*/ );
		pMsgRecord->SetRetrieveFlag( LMOS_DONOT_RETRIEVE /*1*/ );

		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}


void CSummary::SetMessageServerDownload()
{
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord && pMsgRecord->GetRetrieveFlag() != LMOS_RETRIEVE_MESSAGE)
	{
		pMsgRecord->SetRetrieveFlag( LMOS_RETRIEVE_MESSAGE /*0*/ );
					
		// If a message gets marked for retrieve and LMOS is off, then set for removal
		// if not already set as such
		if ( !GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) )
		{
			pMsgRecord->SetDeleteFlag( LMOS_DELETE_MESSAGE /*0*/ );
		}
		else
		{
			pMsgRecord->SetDeleteFlag( LMOS_DONOT_DELETE /*1*/ );
		}				

		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}

int CSummary::CalculateMood(JJFile* pFile)
{
	if (m_nMood <= 0)
	{
		//Do Mood Watch only if it is POP or IMAP with full message download
		if ( ( !m_TheToc->IsImapToc() ) || 
			( m_TheToc->IsImapToc() && IsIMAPMessageBodyDownloaded() ) )
		{
			char* pMessage= DEBUG_NEW_NOTHROW char[m_Length+1];
			if (pMessage)
			{
				pFile->Seek(m_Offset,SEEK_SET);
				pFile->Read(pMessage,m_Length);
				pMessage[m_Length] = 0;
				int nRetVal = -1;
				CMoodWatch moodwatch;
				if(moodwatch.Init())
				{
					char* pTo = HeaderContents(IDS_HEADER_TO, pMessage);
					if(pTo)
					{
						moodwatch.AddText(pTo,strlen(pTo),NULL,
							TAE_IGNORESAFETEXT);
						delete [] pTo;
					}	
					char* pFrom = HeaderContents(IDS_HEADER_FROM, pMessage);
					if(pFrom)
					{
						moodwatch.AddText(pFrom,strlen(pFrom),NULL,
							TAE_IGNORESAFETEXT);
						delete [] pFrom;
					}	
					char* pSubject = HeaderContents(IDS_HEADER_SUBJECT, pMessage);
					if(pSubject)
					{
						moodwatch.AddText(pSubject,strlen(pSubject),NULL,
							TAE_IGNORESAFETEXT);
						delete [] pSubject;
					}	
					char* pCC= HeaderContents(IDS_HEADER_CC, pMessage);
					if(pCC)
					{
						moodwatch.AddText(pCC,strlen(pCC),NULL,
							TAE_IGNORESAFETEXT);
						delete [] pCC;
					}
					if(IsComp())
					{
						char* pBCC = HeaderContents(IDS_HEADER_BCC, pMessage);
						if(pBCC)
						{
							moodwatch.AddText(pBCC,strlen(pBCC),NULL,
								TAE_IGNORESAFETEXT);
							delete [] pBCC;
						}	
						char* pAttachment= HeaderContents(IDS_HEADER_ATTACHMENTS, pMessage);
						if(pAttachment)
						{
							moodwatch.AddText(pAttachment,strlen(pAttachment),NULL,
								TAE_IGNORESAFETEXT);
							delete [] pAttachment;
						}
					}
					const char *pBody = FindBody(pMessage);
					unsigned long lBodyMoodOptions=0;
					if (IsHTML())
						lBodyMoodOptions = TAE_CONTAINSHTML;

					if(pBody)
						moodwatch.AddText((char *)pBody,strlen(pBody),NULL,
								lBodyMoodOptions);
					nRetVal = moodwatch.GetScore();

				}
				delete [] pMessage;
				if(nRetVal != -1)
				{
					SetMood((char)(nRetVal +1));
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}
	return 0;
}

void CSummary::SetMessageServerDownloadDelete()
{
	//CMsgRecord* pMsgRecord = GetMsgByHash( this );
	CLMOSRecord LMOSRecord(this);
	CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
	if ( pMsgRecord && (pMsgRecord->GetDeleteFlag() != LMOS_DELETE_MESSAGE || pMsgRecord->GetRetrieveFlag() != LMOS_RETRIEVE_MESSAGE))
	{
		pMsgRecord->SetDeleteFlag( LMOS_DELETE_MESSAGE /*0*/ );
		pMsgRecord->SetRetrieveFlag( LMOS_RETRIEVE_MESSAGE /*0*/ );

		LMOSRecord.WriteLMOS();
		RedisplayField(FW_SERVER);
	}
}

#ifdef IMAP4
//
//	CSummary::IMAPGetFromHeader()
//
//	Download and return the From: header for this summary.
//
bool CSummary::IMAPGetFromHeader(CString &strFrom)
{
	HRESULT		hresult = E_FAIL;

	hresult = DownloadFromHeader(this, strFrom);

	return hresult != E_FAIL;
}
#endif


// JOK, June 16, 1997.

// ImapParseImflags ()
// FUNCTION
// Extract character flags into an unsigned long.
// END FUNCTION

#ifdef IMAP4

unsigned long ImapParseImflags (LPCSTR pImflags)
{
	unsigned long Flags = 0;
	int c;

	if (!pImflags)
		return 0;

	while (c = *pImflags)
	{
		switch (c)
		{
			case 'R':
			case 'r':
				Flags |= IMFLAGS_SEEN;
				break;
			case 'A':
			case 'a':
				Flags |= IMFLAGS_ANSWERED;
				break;
			case 'F':
			case 'f':
				Flags |= IMFLAGS_FLAGGED;
				break;
			case 'D':
			case 'd':
				Flags |= IMFLAGS_DELETED;
				break;
			case ' T':
			case 't':
				Flags |= IMFLAGS_DRAFT;
				break;
			default:
				break;
		}
		pImflags++;
	}

	return Flags;
}

#endif


///////////////////////////////////////////////////////////////////////
// DownloadImapMessage [EXPORTED]
// FUNCTION
// If this message's contents are not yet downloaded, ask the Imap mailbox to download
// them now. 
// HISTORY:
// 12/18/97 (JOK) Added bQueueIfBusy argument: Passed to the IMAP reoutine.
// END HISTORY
///////////////////////////////////////////////////////////////////////
#ifdef IMAP4

HRESULT DownloadImapMessage (CSummary *pSum, BOOL bDownloadAttachments /* = FALSE */)
{
	HRESULT hResult = E_FAIL;

	if (!pSum)
		return E_INVALIDARG;

	CTocDoc *pTocDoc = pSum->m_TheToc;
	if ( pTocDoc && pTocDoc->IsImapToc() )
	{
		if (pTocDoc->m_pImapMailbox)
		{
			// Are the contents already down?
			if ( pSum->IsNotIMAPDownloadedAtAll() )
			{
				hResult = pTocDoc->m_pImapMailbox->OnPreviewSingleMessage(pSum, bDownloadAttachments);

				if ( SUCCEEDED (hResult) )
				{
			        pTocDoc->SetModifiedFlag();

					CTocView *pTocView = pTocDoc->GetView();
					if (pTocView)
					{
						pSum->RedisplayField (FW_SERVER);
						pSum->RedisplayField (FW_SIZE);
					}
				}
			}
			else if ( pSum->m_Offset < 0 || pSum->m_Length <= 0)
			{
				// Make a check on m_Length.
				hResult = E_FAIL;
			}
			else
			{
				// It's been downloaded
				hResult = S_OK;
			} 
		}
	}

	return hResult;
}

#endif


#ifdef IMAP4
//
//	DownloadFromHeader()
//
//	Ask the CImapMailbox to download the headers for the specified message
//	and give us back the From: header.
//
//	Note: This is necessary because junk whitelisting and adding senders to
//	the address book needs the From: field which is not stored in the summary.
//
HRESULT DownloadFromHeader(CSummary *pSum, CString &strFrom)
{
	HRESULT		 hresult = E_FAIL;

	if (!pSum)
	{
		return E_INVALIDARG;
	}

	CTocDoc		*pTocDoc = pSum->m_TheToc;
	if (pTocDoc && pTocDoc->IsImapToc() && pTocDoc->m_pImapMailbox)
	{
		hresult = pTocDoc->m_pImapMailbox->FetchFromHeaderString(pTocDoc, pSum, strFrom);
	}

	return hresult;
}
#endif
