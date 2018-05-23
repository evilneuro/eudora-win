// IMAPSUM.CPP
//
// Routines for IMAP's version of a summary object.
// Note: It is used JUST TO store stuff during a minimal fetch
// that may be done in the background.
//

#include "stdafx.h"
#include <afxcmn.h>
#include <ctype.h>

#include <QCUtils.h>

#include "cursor.h"
#include "resource.h"
#include "rs.h"

// Only for constants.
#include "summary.h"

#include "ImapSum.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



//================ CImapSumList implementation  =======================//
CImapSumList::~CImapSumList()
{
	// DO NOT Remove elements. Whoever uses this class MUST manage 
	// its member objects.
	//
}

int CImapSumList::GetIndex(const CImapSum* pSum) const
{
	POSITION pos = GetHeadPosition();
	
	for (int i = 0; pos; i++)
	{
		if ((CObject*)pSum == GetNext(pos))
			return (i);
	}
	
	return (-1);
}


// DeleteAll [PUBLIC]
// FUNCTION
// Delete all entries from a CIMapSumList and free the individual CImapSum's as well..
// END FUNCITON
void CImapSumList::DeleteAll ()
{
	POSITION pos, next;
	CImapSum *pSum;

	pos = GetHeadPosition();

	for( next = pos; pos; pos = next )
	{
		pSum = ( CImapSum * ) GetNext( next );
		if (pSum)
		{
			SetAt (pos, NULL);
			delete pSum;
		}

		// Remove the list entry.
		RemoveAt (pos);
	}

	RemoveAll();
}




/////////////////////////////////////////////////////////////////////////////
// CImapSum diagnostics

#ifdef _DEBUG
void CImapSum::AssertValid() const
{
	CObject::AssertValid();
}

void CImapSum::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	AFX_DUMP1(dc, "\n\tFrom = ", m_From);
	AFX_DUMP1(dc, "\n\tSubject = ", m_Subject);
}

#endif //_DEBUG


CImapSum::CImapSum() : CObject()
{
	m_Offset = 0L;
	m_Length = 0L;
	m_State = MS_UNREAD;

	memset(m_From, 0, sizeof(m_From));
	memset(m_Subject, 0, sizeof(m_Subject));

	m_Seconds = 0L;

	//m_Flags = ( unsigned short ) (GetIniShort(IDS_INI_SHOW_ALL_HEADERS)? MSF_SHOW_ALL_HEADERS : 0);
	m_Flags = 0;
	m_Priority = MSP_NORMAL;
	m_Hash = 0L;
	m_FlagsEx = 0;


// IMAP4
	m_Imflags = 0;
	m_MsgSize = 1;		// Default to 1k.

	m_nUndownloadedAttachments = 0;
}

CImapSum::~CImapSum()
{

}


const char* CImapSum::GetFrom()
{
	if (*m_From)
		return (m_From);

	return ( strcpy(m_From + 1, (CRString(IDS_NO_RECIPIENT))) );
}


////////////////////////////////////////////////////////////////////////
// GetPriority [public]
//
// Returns an ASCII string which corresponds to the current priority
// value.
////////////////////////////////////////////////////////////////////////

CString CImapSum::GetPriority() const
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

const char* CImapSum::GetSubject()
{
	if (*m_Subject)
		return (m_Subject);

	return (strcpy(m_Subject + 1, CRString(IDS_NO_SUBJECT)));
}



void CImapSum::GleanPriority(const char* HeaderContents)
{
	int prior = MSP_NORMAL;
	if (HeaderContents)
	{
		prior = atoi(HeaderContents);
		if( (prior < MSP_HIGHEST) || (prior > MSP_LOWEST) )
		{
			prior = MSP_NORMAL;
		}
	}
	SetPriority(prior);
}


void CImapSum::SetFrom(const char* NewFrom)
{
    if (NewFrom && !strcmp(m_From, NewFrom))
        return; 
    memset(m_From, 0, sizeof(m_From));
    if (NewFrom)
    	strncpy(m_From, NewFrom, sizeof(m_From) - 1);
}

void CImapSum::SetSubject(const char* NewSubject)
{
    if (NewSubject && !strcmp(m_Subject, NewSubject))
        return; 
    memset(m_Subject, 0, sizeof(m_Subject));
    if (NewSubject)
    	strncpy(m_Subject, NewSubject, sizeof(m_Subject) - 1);
}



// PUBLIC - copy from a CSummary:
//
void CImapSum::CopyFromCSummary (CSummary* pSum)
{
	if (!pSum)
		return;

	// public members:
	//
	m_Offset	= pSum->m_Offset;
	m_Length	= pSum->m_Length;
	m_State		= pSum->m_State;
	m_Priority	= pSum->m_Priority;

	m_Imflags	= pSum->m_Imflags;
	m_MsgSize   = pSum->m_MsgSize;

	m_Seconds	= pSum->m_Seconds;

	// Be careful with these: do strcpy to avoid length problems.
	//
	strcpy (m_From, pSum->m_From);
	strcpy (m_Subject, pSum->m_Subject);

	// private members
	m_Flags		= pSum->GetFlags();
	m_FlagsEx	= pSum->GetFlagsEx();
	m_Hash		= pSum->GetHash();

	m_nUndownloadedAttachments = pSum->m_nUndownloadedAttachments;

}



void CImapSum::CopyToCSummary (CSummary* pSum)
{
	if (!pSum)
		return;

	// public members:
	//
	pSum->m_Offset	= m_Offset;
	pSum->m_Length	= m_Length;
	pSum->m_State	= m_State;
	pSum->m_Priority = m_Priority;

	pSum->m_Imflags	= m_Imflags;
	pSum->m_MsgSize = m_MsgSize;
	pSum->m_Seconds	= m_Seconds;

	// Be careful with these: do strcpy to avoid length problems.
	//
	strcpy (pSum->m_From, m_From);
	strcpy (pSum->m_Subject, m_Subject);

	// private members - first clear them..
	// 

	pSum->UnsetFlag	((WORD)-1);
	pSum->SetFlag	(m_Flags);

	pSum->UnsetFlagEx	((WORD)-1);
	pSum->SetFlagEx		(m_FlagsEx);

	pSum->SetHash(m_Hash);

	pSum->m_nUndownloadedAttachments = m_nUndownloadedAttachments;
}



