// IMAPSUM.CPP
//
// Routines for IMAP's version of a summary object.
// Note: It is used JUST TO store stuff during a minimal fetch
// that may be done in the background.
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


#include "DebugNewHelpers.h"



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
	m_nMood = 0;
	m_ucJunkScore = 0;
	m_ulJunkPluginID = 0;
	m_bManuallyJunked = false;

// IMAP4
	m_Imflags = 0;
	m_MsgSize = 1;		// Default to 1k.

	m_nUndownloadedAttachments = 0;
	m_markForDelete = FALSE;
	m_markFullDownload = FALSE;

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

#define IsWhite(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')

void CImapSum::GleanImportance(const char* szHeaderContents)
{
	int prior = MSP_NORMAL;

	if (szHeaderContents)
	{
		while (IsWhite(*szHeaderContents)) szHeaderContents++;
		CRString highest(IDS_PRIORITY_HIGHEST);
		if (!strncmp(highest,szHeaderContents,highest.GetLength())) prior = MSP_HIGHEST;
		else
		{
			CRString high(IDS_PRIORITY_HIGH);
			if (!strncmp(high,szHeaderContents,high.GetLength())) prior = MSP_HIGH;
			else
			{
				CRString low(IDS_PRIORITY_LOW);
				if (!strncmp(low,szHeaderContents,low.GetLength())) prior = MSP_LOW;
				else
				{
					CRString lowest(IDS_PRIORITY_LOWEST);
					if (!strncmp(lowest,szHeaderContents,lowest.GetLength())) prior = MSP_LOWEST;
				}
			}
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
	m_nMood		= pSum->m_nMood;
	m_ucJunkScore = pSum->m_ucJunkScore;
	m_ulJunkPluginID = pSum->m_ulJunkPluginID;
	m_bManuallyJunked = pSum->m_bManuallyJunked;

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
	pSum->m_Priority = (char)m_Priority;

	pSum->m_Imflags	= m_Imflags;
	pSum->m_MsgSize = m_MsgSize;
	pSum->m_Seconds	= m_Seconds;
	pSum->m_nMood	= m_nMood;
	pSum->m_ucJunkScore = m_ucJunkScore;
	pSum->m_ulJunkPluginID = m_ulJunkPluginID;
	pSum->m_bManuallyJunked = m_bManuallyJunked;

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



