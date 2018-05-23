// SearchResult.cpp
//
// 11/2/98 - Scott Manjourides

#include "stdafx.h"
#include "SearchCriteria.h"

#include "SearchResult.h"
#include "summary.h"
#include "tocdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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
		m_strPath = pSum->m_TheToc->MBFilename();
		m_strTitle = pSum->m_TheToc->Name();

		m_UniqueID = pSum->GetUniqueMessageId();
		m_From = pSum->GetFrom();
		m_Subject = pSum->GetSubject();
		m_Date = pSum->GetDate();
		m_Seconds = pSum->m_Seconds;
	}
}