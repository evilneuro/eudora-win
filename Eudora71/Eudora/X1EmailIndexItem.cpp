//	X1EmailIndexItem.h
//
//	Interprets email message information for X1 indexing.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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


#include "stdafx.h"
#include "X1EmailIndexItem.h"
#include <memory>
#include "SearchManager.h"
#include "SearchManagerUtils.h"
#include "DebugNewHelpers.h"
#include "etf2html.h"
#include "fileutil.h"
#include "html2text.h"
#include "msgutils.h"
#include "QCUtils.h"
#include "summary.h"
#include "tocdoc.h"
#include "utils.h"


// ---------------------------------------------------------------------------
//		* X1EmailIndexItem											 [Public]
// ---------------------------------------------------------------------------
//	X1EmailIndexItem constructor

X1EmailIndexItem::X1EmailIndexItem(
	CSummary *					in_pSummary,
	const char *				in_szFullMessage)
	:	m_nRefCount(0)
{
	ASSERT(in_pSummary);
	if (!in_pSummary)
		return;

	//	Get any information we can out of the summary. Some of this may be
	//	replaced if we have the full message available.
	CTocDoc *		pTocDoc = in_pSummary->m_TheToc;

	ASSERT(pTocDoc);
	if (pTocDoc)
	{
		m_szMailboxRelativePath = static_cast<const char *>(pTocDoc->GetMBFileName()) + EudoraDirLen;
		m_szMailboxName = pTocDoc->Name();
	}
	m_nUniqueMessageId = in_pSummary->GetUniqueMessageId();
	m_szWho = in_pSummary->m_From;

	//	If it's not the Out mailbox, default use the "Who" value as the default
	//	From header value (in case we don't have the full message or can't find
	//	the From header in it)
	if (in_pSummary->m_TheToc->m_Type != MBT_OUT)
		m_szFromHeader = m_szWho;

	m_szSubject = in_pSummary->m_Subject;
	m_Seconds = in_pSummary->m_Seconds;
	m_TimeZoneMinutes = in_pSummary->m_TimeZoneMinutes;

	if (m_Seconds <= kSecondsInADay)
	{
		//	Date is bad - there are less seconds than one days worth.
		//	Calling SetDate below would try to set the date before
		//	1/1/1970, which would make COleDateTime throw an exception.
		//	Avoid that by special casing the value and just setting
		//	the number of days to 0.
		m_Days = 0;

		if (m_Seconds < 0)
			m_Seconds = 0;
	}
	else
	{
		//	Convert our time as seconds since 1970 to days since 1970
		COleDateTime	tmDays(m_Seconds);
		tmDays.SetDate( tmDays.GetYear(), tmDays.GetMonth(), tmDays.GetDay() );

		SYSTEMTIME		systemTimeDays;

		if ( tmDays.GetAsSystemTime(systemTimeDays) )
		{
			//	Convert system time to timespan since 1970
			CTime			time(systemTimeDays);
			CTimeSpan		timeSpan( time.GetTime() );

			//	Get number of days since 1970
			m_Days = static_cast<long>( timeSpan.GetDays() );
		}
		else
		{
			ASSERT(!"Date conversion failed");
			m_Days = 0;
		}
	}

	m_State = in_pSummary->m_State;
	ASSERT(m_State >= 0);
	if (m_State < 0)
		m_State = 0;

	m_Label = in_pSummary->m_Label;
	ASSERT(m_Label >= 0);
	if (m_Label < 0)
		m_Label = 0;

	m_nPersonaHash = in_pSummary->GetPersonaHash();
	m_Priority = in_pSummary->m_Priority;
	ASSERT(m_Priority >= 0);
	if (m_Priority < 0)
		m_Priority = 0;

	m_szPriority = in_pSummary->GetPriority();
	m_nMood = in_pSummary->m_nMood;
	ASSERT(m_nMood >= 0);
	if (m_nMood < 0)
		m_nMood = 0;

	m_ucJunkScore = in_pSummary->m_ucJunkScore;
	m_nSize = in_pSummary->GetSize();
	ASSERT(m_nSize >= 0);
	if (m_nSize < 0)
		m_nSize = 0;

	if ( pTocDoc && pTocDoc->IsImapToc() )
	{
		//	We currently only care about the deleted IMAP flag
		m_Imflags = (in_pSummary->m_Imflags & IMFLAGS_DELETED); 
	}
	else
	{
		//	Non-IMAP TOC, make sure IMAP flags are 0
		m_Imflags = 0;
	}

	//	Default to number of un-downloaded attachments
	m_nAttachments = in_pSummary->m_nUndownloadedAttachments;
	ASSERT(m_nAttachments >= 0);
	if (m_nAttachments < 0)
		m_nAttachments = 0;

	if (in_szFullMessage)
	{
		//	Find headers in full message. Use auto_ptr to auto-delete each
		//	result from HeaderContents, which allocates buffers.
		std::auto_ptr<char>		szHeader;

		//	To Header
		szHeader.reset( HeaderContents(IDS_HEADER_TO, in_szFullMessage) );
		if ( szHeader.get() )
			m_szToHeader = szHeader.get();

		//	Cc Header
		szHeader.reset( HeaderContents(IDS_HEADER_CC, in_szFullMessage) );
		if ( szHeader.get() )
			m_szCcHeader = szHeader.get();

		//	Bcc Header
		szHeader.reset( HeaderContents(IDS_HEADER_BCC, in_szFullMessage) );
		if ( szHeader.get() )
			m_szBccHeader = szHeader.get();

		//	From:
		szHeader.reset( HeaderContents(IDS_HEADER_FROM, in_szFullMessage) );
		if ( szHeader.get() )
			m_szFromHeader = szHeader.get();

		//	Check to see if we think we have attachments
		if ( in_pSummary->HasAttachment() )
		{
			if ( in_pSummary->IsComp() )
			{
				//	Outgoing/sent message - get x-attachments header
				szHeader.reset( HeaderContents(IDS_HEADER_ATTACHMENTS, in_szFullMessage) );

				if ( szHeader.get() )
				{
					//	Get the attachments list from the header
					m_szAttachments = szHeader.get();

					//	X-Attachments header has a trailing semi-colon
					m_szAttachments.Trim(';');

					CString		szTemp = m_szAttachments;

					//	X-Attachments header has one semi-colon per attachment.
					//	Count the number of semi-colon's by replacing them with comma's
					//	(hence the use of the temp string).
					m_nAttachments += szTemp.Replace(';', ',');
				}
			}
			else
			{
				//	Received message - get the attachments and count how many we have
				m_nAttachments += GetAttachments(in_szFullMessage, m_szAttachments);
			}
		}

		//	Separate the body from the headers
		const char *	pBody = FindBody(in_szFullMessage);
		long			nAllHeadersLength = pBody ? (pBody - in_szFullMessage) : strlen(in_szFullMessage);

		if (nAllHeadersLength > 0)
		{
			strncpy(m_szAllHeaders.GetBuffer(nAllHeadersLength), in_szFullMessage, nAllHeadersLength);
			m_szAllHeaders.ReleaseBuffer(nAllHeadersLength);
		}

		if (pBody)
		{
			//	Store body as plain text
			switch ( IsFancy(pBody) )
			{
				case IS_ASCII:
					m_szBody = pBody;
					break;

				case IS_FLOWED:
					{
						std::auto_ptr<char>		szFlowedBody( FlowText(pBody) );
						m_szBody = szFlowedBody.get();
					}
					break;

				case IS_RICH:
					{
						int		nSize = Etf2Html_Measure(pBody);

						if (nSize)
						{
							std::auto_ptr<char>	szHTMLBody( DEBUG_NEW_NOTHROW char[nSize+1] );

							if ( szHTMLBody.get() )
								m_szBody = Html2Text( szHTMLBody.get() );
						}
					}
					break;

				case IS_HTML:
					m_szBody = Html2Text(pBody);
					break;
			}
		}
	}

	//	Separate subject into prefix and main subject
	const char *	pStartSubject = m_szSubject;
	const char *	pSubjectMain = RemoveSubjectPrefixMT(pStartSubject);

	m_szSubjectMain = pSubjectMain;

	if (pSubjectMain != pStartSubject)
	{
		m_szSubjectPrefix = m_szSubject.Left(pSubjectMain - pStartSubject);

		const int	nSubjectPrefixLength = m_szSubjectPrefix.GetLength();
		int			nRemainingSubjectPrefixLength = nSubjectPrefixLength;
		char *		pSubjectPrefix = m_szSubjectPrefix.GetBuffer(nSubjectPrefixLength);

		if (pSubjectPrefix)
		{
			//	Normalize the case of all "Re", "Fw", and "Fwd" to be capital letter
			//	followed by lower case letter(s).
			//
			//	We need to do this because X1 will store the first instance of any
			//	string and will point any other instances that match case insensitively
			//	to that first instance. Therefore if we allow "RE" or "FWD" to get
			//	stored - all cases of the subject prefix would appear in all caps.
			//	We prefer the mixed case version, and therefore enforce that.
			
			while (*pSubjectPrefix)
			{
				if ( (nRemainingSubjectPrefixLength >= 2) && (strnicmp(pSubjectPrefix, "Re", 2) == 0) )
				{
					//	Found variant of "Re" - normalize case to be "Re" (e.g. "RE" becomes "Re")
					*pSubjectPrefix++ = 'R';
					*pSubjectPrefix++ = 'e';

					//	Update the remaining length
					nRemainingSubjectPrefixLength -= 2;
				}
				else if ( (nRemainingSubjectPrefixLength >= 2) && (strnicmp(pSubjectPrefix, "Fw", 2) == 0) )
				{
					//	Found variant of "Fw" or "Fwd" - normalize case to be "Fw" or "Fwd" (e.g. "FWD" becomes "Fwd")
					*pSubjectPrefix++ = 'F';
					*pSubjectPrefix++ = 'w';

					//	Update the remaining length
					nRemainingSubjectPrefixLength -= 2;

					if (*pSubjectPrefix == 'D')
					{
						*pSubjectPrefix++ = 'd';

						//	Update the remaining length
						nRemainingSubjectPrefixLength--;
					}
				}
				else
				{
					//	We hit a non-attribution word - skip past it by continuing
					//	as long as the character is alphabetic
					while ( *pSubjectPrefix && isalpha(*pSubjectPrefix) )
					{
						pSubjectPrefix++;

						//	Update the remaining length
						nRemainingSubjectPrefixLength--;
					}

					//	Now move forward until we get to the next candidate word by
					//	continuing as long as the character is *not* alphabetic
					while ( *pSubjectPrefix && !isalpha(*pSubjectPrefix) )
					{
						pSubjectPrefix++;

						//	Update the remaining length
						nRemainingSubjectPrefixLength--;
					}
				}
			}

			m_szSubjectPrefix.ReleaseBuffer(nSubjectPrefixLength);
		}
	}

	//	Format the relative path that we'll use for indexing.
	//	We prefix it with "UniqueRootPrefixForEudoraMailboxPaths " to make it
	//	possible to uniquely find mailboxes without worrying about sub-paths
	//	incorrectly matching our searches (see SearchManager.cpp for more info).
	m_szMailboxRelativePathForIndexing = SearchManager::kMailboxRelativePathRootTag;
	m_szMailboxRelativePathForIndexing += m_szMailboxRelativePath;
}


// ---------------------------------------------------------------------------
//		* ~X1EmailIndexItem											 [Public]
// ---------------------------------------------------------------------------
//	X1EmailIndexItem destructor

X1EmailIndexItem::~X1EmailIndexItem()
{
}


// ---------------------------------------------------------------------------
//		* QueryInterface											 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::QueryInterface() implementation

HRESULT __stdcall
X1EmailIndexItem::QueryInterface(
	const IID &				in_iid,
	void **					out_ppvObject)
{
	if ( (in_iid == IID_IUnknown) || (in_iid == X1::IID_IX1IndexItem) )
	{
		*out_ppvObject = static_cast<X1::IX1IndexItem *>(this);
	}
	else
	{
		*out_ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* AddRef													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::AddRef() implementation

ULONG __stdcall
X1EmailIndexItem::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}


// ---------------------------------------------------------------------------
//		* Release													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::Release() implementation

ULONG __stdcall
X1EmailIndexItem::Release()
{
	if (InterlockedDecrement(&m_nRefCount) == 0)
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


// ---------------------------------------------------------------------------
//		* raw_GetFieldTextForStorage								 [Public]
// ---------------------------------------------------------------------------
//	Returns the field data as text appropriate for storage. For example,
//	when storing a date value, this function should return the text form
//	of a double representation of a date (e.g. "35065.0").

HRESULT __stdcall
X1EmailIndexItem::raw_GetFieldTextForStorage(
	long						in_nFieldID,
	BSTR *						out_pText)
{
	CString		szField;
	
	switch(in_nFieldID)
	{
		case SearchManager::efid_MailboxRelativePath:
			*out_pText = m_szMailboxRelativePath.AllocSysString();
			break;

		case SearchManager::efid_MailboxName:
			*out_pText = m_szMailboxName.AllocSysString();
			break;

		case SearchManager::efid_MessageID:
			szField.Format("%d", m_nUniqueMessageId);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_Who:
			*out_pText = m_szWho.AllocSysString();
			break;

		case SearchManager::efid_DateSeconds:
			szField.Format("%d", m_Seconds);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_State:
			szField.Format("%d", m_State);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_Label:
			szField.Format("%d", m_Label);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_PriorityNumber:
			szField.Format("%d", m_Priority);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_JunkScoreNumber:
			szField.Format("%d", m_ucJunkScore);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_AttachmentCountNumber:
			szField.Format("%d", m_nAttachments);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_SizeNumber:
			szField.Format("%d", m_nSize);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_ImapFlags:
			szField.Format("%d", m_Imflags);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_Mood:
			szField.Format("%d", m_nMood);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_TimeZoneMinutes:
			szField.Format("%d", m_TimeZoneMinutes);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_SubjectPrefix:
			*out_pText = m_szSubjectPrefix.AllocSysString();
			break;

		case SearchManager::efid_SubjectMain:
			*out_pText = m_szSubjectMain.AllocSysString();
			break;

		default:
			ASSERT(!"Unhandled field ID in X1EmailIndexItem::raw_GetFieldTextForStorage");
			break;
	};

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_IsDifferentFieldTextForIndexing						 [Public]
// ---------------------------------------------------------------------------
//	Returns true if the storage form of the field text is the same as the
//	indexing form.

HRESULT __stdcall
X1EmailIndexItem::raw_IsDifferentFieldTextForIndexing(
	long						in_nFieldID,
	unsigned char *				out_pbIsDifferent)
{
	//	Returns TRUE if:
	//	* Field was setup as indexed only (i.e. unnamed and kIsNotColumn)
	//	* Data stored is different than data indexed
	switch(in_nFieldID)
	{
		case SearchManager::efid_Subject:
		case SearchManager::efid_AllHeaders:
		case SearchManager::efid_Body:
		case SearchManager::efid_MailboxRelativePath:
		case SearchManager::efid_FromHeader:
		case SearchManager::efid_ToHeader:
		case SearchManager::efid_CcHeader:
		case SearchManager::efid_BccHeader:
		case SearchManager::efid_Attachments:
		case SearchManager::efid_DateSeconds:
		case SearchManager::efid_DateDaysRomanNumeral:
		case SearchManager::efid_State:
		case SearchManager::efid_PersonaHash:
		case SearchManager::efid_PriorityNumber:
		case SearchManager::efid_PriorityRomanNumeral:
		case SearchManager::efid_JunkScoreRomanNumeral:
		case SearchManager::efid_AttachmentCountRomanNumeral:
		case SearchManager::efid_SizeRomanNumeral:
		case SearchManager::efid_ImapFlags:
			*out_pbIsDifferent = TRUE;
			break;

		default:
			*out_pbIsDifferent = FALSE;
			break;
	}

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_GetFieldTextForIndexing								 [Public]
// ---------------------------------------------------------------------------
//	Returns the field data as text appropriate for indexing. For example,
//	when storing a date value, this function should return the string
//	representation of a date (e.g. "1/1/1996 12:00 am").

HRESULT __stdcall
X1EmailIndexItem::raw_GetFieldTextForIndexing(
	long						in_nFieldID,
	BSTR *						out_pText)
{
	CString		szField;

	switch(in_nFieldID)
	{
		case SearchManager::efid_Subject:
			*out_pText = m_szSubject.AllocSysString();
			break;

		case SearchManager::efid_AllHeaders:
			*out_pText = m_szAllHeaders.AllocSysString();
			break;

		case SearchManager::efid_Body:
			*out_pText = m_szBody.AllocSysString();
			break;

		case SearchManager::efid_MailboxRelativePath:
			*out_pText = m_szMailboxRelativePathForIndexing.AllocSysString();
			break;

		case SearchManager::efid_FromHeader:
			*out_pText = m_szFromHeader.AllocSysString();
			break;

		case SearchManager::efid_ToHeader:
			*out_pText = m_szToHeader.AllocSysString();
			break;

		case SearchManager::efid_CcHeader:
			*out_pText = m_szCcHeader.AllocSysString();
			break;

		case SearchManager::efid_BccHeader:
			*out_pText = m_szBccHeader.AllocSysString();
			break;

		case SearchManager::efid_Attachments:
			*out_pText = m_szAttachments.AllocSysString();
			break;

		case SearchManager::efid_DateSeconds:
			{
				::TimeDateStringFormatMT(szField.GetBuffer(128), m_Seconds, INT_MAX, "%2");
				szField.ReleaseBuffer();

				*out_pText = szField.AllocSysString();
			}
			break;

		case SearchManager::efid_DateDaysRomanNumeral:
			SearchManager::Utils::EncodeNumberAsRomanNumeral(
					m_Days, SearchManager::kRomanNumeralMaxPossibleValue,
					SearchManager::rn_LastDigitIsThousand, szField );
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_State:
			switch (m_State)
			{
				case MS_UNREAD:
					szField.LoadString(IDS_STATUS_UNREAD);
					break;

				case MS_READ:
					szField.LoadString(IDS_STATUS_READ);
					break;

				case MS_REPLIED:
					szField.LoadString(IDS_STATUS_REPLIED);
					break;

				case MS_FORWARDED:
					szField.LoadString(IDS_STATUS_FORWARDED);
					break;

				case MS_REDIRECT:
					szField.LoadString(IDS_STATUS_REDIRECTED);
					break;

				case MS_UNSENDABLE:
					szField.LoadString(IDS_STATUS_UNSENDABLE);
					break;

				case MS_SENDABLE:
					szField.LoadString(IDS_STATUS_SENDABLE);
					break;

				case MS_QUEUED:
					szField.LoadString(IDS_STATUS_QUEUED);
					break;

				case MS_SENT:
					szField.LoadString(IDS_STATUS_SENT);
					break;

				case MS_UNSENT:
					szField.LoadString(IDS_STATUS_UNSENT);
					break;

				case MS_TIME_QUEUED:
					szField.LoadString(IDS_STATUS_QUEUED);
					break;

				case MS_RECOVERED:
					szField.LoadString(IDS_STATUS_RECOVERED);
					break;
			}
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_PriorityNumber:
			*out_pText = m_szPriority.AllocSysString();
			break;

		case SearchManager::efid_PriorityRomanNumeral:
			SearchManager::Utils::EncodeNumberAsRomanNumeral(
					m_Priority, MSP_LOWEST, SearchManager::rn_LastDigitIsOne, szField);
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_JunkScoreRomanNumeral:
			SearchManager::Utils::EncodeNumberAsRomanNumeral(
					m_ucJunkScore, SearchManager::kMaxPossibleJunkScore,
					SearchManager::rn_LastDigitIsTen, szField );
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_AttachmentCountRomanNumeral:
			SearchManager::Utils::EncodeNumberAsRomanNumeral(
					m_nAttachments, SearchManager::kMaxNumAttachments,
					SearchManager::rn_LastDigitIsTen, szField );
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_SizeRomanNumeral:
			SearchManager::Utils::EncodeNumberAsRomanNumeral(
					m_nSize, SearchManager::kRomanNumeralMaxPossibleValue,
					SearchManager::rn_LastDigitIsThousand, szField );
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_ImapFlags:
			if (m_Imflags & IMFLAGS_DELETED)
			{
				//	Load the "\\Deleted" string
				szField.LoadString(IDS_IMAP_DELETED_STRING_FLAG);

				//	Trim off the leading "\\" from the "\\Deleted" string
				szField.Trim('\\');
			}
			*out_pText = szField.AllocSysString();
			break;

		case SearchManager::efid_PersonaHash:
			szField.Format("%d", m_nPersonaHash);
			*out_pText = szField.AllocSysString();
			break;
	}

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_GetMaxWordBytesToIndex								 [Public]
// ---------------------------------------------------------------------------
//	Returns the maximum number of bytes to index. Used to keep the size of
//	the index down. -1 means unlimited.

HRESULT __stdcall
X1EmailIndexItem::raw_GetMaxWordBytesToIndex(
	long *						out_pnMaxWordBytes)
{
	*out_pnMaxWordBytes = -1;

	return S_OK;
}
