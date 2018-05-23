// SearchResult.cpp
//
// 11/2/98 - Scott Manjourides

#include "stdafx.h"
#include "SearchCriteria.h"

#include "SearchResult.h"
#include "summary.h"
#include "tocdoc.h"

#include "fileutil.h"
#include "SearchManager.h"
#include "X1Wrapper.h"

#include "DebugNewHelpers.h"

// --------------------------------------------------------------------------

#define SAFEDELETE(p) { if (p) { delete p; p = NULL; } }


SearchResult::SearchResult() // Default constructor
{ }

SearchResult::~SearchResult() // Destructor
{ }

SearchResult::SearchResult(const SearchResult &copy) // Copy constructor
	: m_strPath(copy.m_strPath), m_strTitle(copy.m_strTitle), m_UniqueID(copy.m_UniqueID),
	  m_Subject(copy.m_Subject), m_From(copy.m_From), m_Date(copy.m_Date), m_Seconds(copy.m_Seconds)
{ }

SearchResult::SearchResult(CSummary *pSum) // Init constructor
{
	ASSERT(pSum->m_TheToc);

	if(pSum->m_TheToc)
	{
		m_strPath = pSum->m_TheToc->GetMBFileName();
		m_strTitle = pSum->m_TheToc->Name();

		m_UniqueID = pSum->GetUniqueMessageId();
		m_From = pSum->GetFrom();
		m_Subject = pSum->GetSubject();
		m_Date = pSum->GetDate();
		m_Seconds = pSum->m_Seconds;
	}
}


SearchResult::SearchResult(
	X1::IX1Database *		in_pX1EmailDB,
	long					in_nItemNum)
{
	//	Initialize search result based on X1 item
	CString		szField;
	
	//	Get the mailbox name
	m_strTitle = static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_MailboxName, in_nItemNum) );

	//	Format the full path to the mailbox
	m_strPath = EudoraDir;
	m_strPath += static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_MailboxRelativePath, in_nItemNum) );

	//	Get the message ID
	szField = static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_MessageID, in_nItemNum) );
	m_UniqueID = atol(szField);

	//	Get the Who value
	m_From = static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_Who, in_nItemNum) );

	//	Get the Subject field
	m_Subject = static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_Subject, in_nItemNum) );

	//	Convert the Date field
	szField = static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_DateSeconds, in_nItemNum) );
	m_Seconds = atol(szField);

	//	Get the time zone
	szField = static_cast<char *>( in_pX1EmailDB->GetFieldString(SearchManager::efid_TimeZoneMinutes, in_nItemNum) );
	int			nTimeZoneMinutes = atoi(szField);

	//	Format the date string
	CSummary::SetDateString( m_Date.GetBuffer(128), 128, m_Seconds, nTimeZoneMinutes,
							 GetIniShort(IDS_INI_LOCAL_TIME_DISPLAY),
							 GetIniShort(IDS_INI_FIXED_DATE_DISPLAY) );
	m_Date.ReleaseBuffer();
}