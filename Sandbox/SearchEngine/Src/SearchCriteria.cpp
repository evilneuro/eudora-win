// SearchCriteria.cpp
//
// 10/26/98 - Scott Manjourides

#include "stdafx.h"
#include "SearchCriteria.h"
#include "hsregex.h"
#include "algorithm"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// --------------------------------------------------------------------------

#define SAFEDELETE(p) { if (p) { delete p; p = NULL; } }

// --------------------------------------------------------------------------

// This order/size must match the CriteriaObject
struct
{
	CriteriaVerbType   verbType;
	CriteriaValueType  valueType;
} gCategoryTable[] = 
	{
		// Verb								// Selection
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// anywhere
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// headers
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// body
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// attachment names
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// summary
		CRITERIA_VERB_EQUAL_COMPARE_TYPE,	CRITERIA_VALUE_STATUS_TYPE,		// status
		CRITERIA_VERB_NUM_COMPARE_TYPE,		CRITERIA_VALUE_PRIORITY_TYPE,	// priority
		CRITERIA_VERB_NUM_COMPARE_TYPE,		CRITERIA_VALUE_ATTACHCOUNT_TYPE,// attachment count
		CRITERIA_VERB_EQUAL_COMPARE_TYPE,	CRITERIA_VALUE_LABEL_TYPE,		// label
		CRITERIA_VERB_DATE_COMPARE_TYPE,	CRITERIA_VALUE_DATE_TYPE,		// date
		CRITERIA_VERB_NUM_COMPARE_TYPE,		CRITERIA_VALUE_SIZE_TYPE,		// size
		CRITERIA_VERB_NUM_COMPARE_TYPE,		CRITERIA_VALUE_AGE_TYPE,		// age
		CRITERIA_VERB_EQUAL_COMPARE_TYPE,	CRITERIA_VALUE_PERSONA_TYPE,	// personality
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// Mailbox Name
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// to
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// from
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// subject
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// Cc
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE,		// Bcc
		CRITERIA_VERB_TEXT_COMPARE_TYPE,	CRITERIA_VALUE_TEXT_TYPE		// any recipient
	};

// --------------------------------------------------------------------------
// SearchCriteria
// --------------------------------------------------------------------------

SearchCriteria::SearchCriteria() // Standard constructor
{
	m_pObj = NULL;

	m_pVerbTxtCmp = NULL;
	m_pVerbEqualCmp = NULL;
	m_pVerbNumCmp = NULL;
	m_pVerbDateCmp = NULL;

	m_pValueText = NULL;
	m_pValueAttachCount = NULL;
	m_pValueSize = NULL;
	m_pValueStatus = NULL;
	m_pValueLabel = NULL;
	m_pValuePersona = NULL;
	m_pValuePriority = NULL;
	m_pValueDate = NULL;
	m_pValueAge = NULL;
	m_pRegex = NULL;
}

SearchCriteria::~SearchCriteria() // Destructor
{
	DeleteAll();
}

void SearchCriteria::DeleteAll()
{
	SAFEDELETE(m_pObj);

	DeleteVerbs();
	DeleteValues();

	SAFEDELETE(m_pRegex);
}


void SearchCriteria::DeleteVerbs()
{
	SAFEDELETE(m_pVerbTxtCmp);
	SAFEDELETE(m_pVerbEqualCmp);
	SAFEDELETE(m_pVerbNumCmp);
	SAFEDELETE(m_pVerbDateCmp);
}

void SearchCriteria::DeleteValues()
{
	SAFEDELETE(m_pValueText);
	SAFEDELETE(m_pValueAttachCount);
	SAFEDELETE(m_pValueSize);
	SAFEDELETE(m_pValueStatus);
	SAFEDELETE(m_pValueLabel);
	SAFEDELETE(m_pValuePersona);
	SAFEDELETE(m_pValuePriority);
	SAFEDELETE(m_pValueDate);
	SAFEDELETE(m_pValueAge);
}

SearchCriteria::SearchCriteria(const SearchCriteria &copy) // Copy constructor
{
	m_pObj = NULL;

	m_pVerbTxtCmp = NULL;
	m_pVerbEqualCmp = NULL;
	m_pVerbNumCmp = NULL;
	m_pVerbDateCmp = NULL;

	m_pValueText = NULL;
	m_pValueAttachCount = NULL;
	m_pValueSize = NULL;
	m_pValueStatus = NULL;
	m_pValueLabel = NULL;
	m_pValuePersona = NULL;
	m_pValuePriority = NULL;
	m_pValueDate = NULL;
	m_pValueAge = NULL;

	m_pRegex = NULL;
	m_bNeedFullMessage = false;

	if (copy.m_pObj)
		m_pObj = new CriteriaObject(*copy.m_pObj);

	if (copy.m_pVerbTxtCmp)
		m_pVerbTxtCmp = new CriteriaVerbTextCompare(*copy.m_pVerbTxtCmp);
	if (copy.m_pVerbEqualCmp)
		m_pVerbEqualCmp = new CriteriaVerbEqualCompare(*copy.m_pVerbEqualCmp);
	if (copy.m_pVerbNumCmp)
		m_pVerbNumCmp = new CriteriaVerbNumCompare(*copy.m_pVerbNumCmp);
	if (copy.m_pVerbDateCmp)
		m_pVerbDateCmp = new CriteriaVerbDateCompare(*copy.m_pVerbDateCmp);

	if (copy.m_pValueText)
		m_pValueText = new CriteriaValueText(*copy.m_pValueText);

	if (copy.m_pValueAttachCount)
		m_pValueAttachCount = new CriteriaValueAttachCount(*copy.m_pValueAttachCount);
	if (copy.m_pValueSize)
		m_pValueSize = new CriteriaValueSize(*copy.m_pValueSize);
	if (copy.m_pValueStatus)
		m_pValueStatus = new CriteriaValueStatus(*copy.m_pValueStatus);
	if (copy.m_pValueLabel)
		m_pValueLabel = new CriteriaValueLabel(*copy.m_pValueLabel);
	if (copy.m_pValuePersona)
		m_pValuePersona = new CriteriaValuePersona(*copy.m_pValuePersona);
	if (copy.m_pValuePriority)
		m_pValuePriority = new CriteriaValuePriority(*copy.m_pValuePriority);
	if (copy.m_pValueDate)
		m_pValueDate = new CriteriaValueDate(*copy.m_pValueDate);
	if (copy.m_pValueAge)
		m_pValueAge = new CriteriaValueAge(*copy.m_pValueAge);
	
	if (copy.m_pRegex)
		m_pRegex = new CRegex(*copy.m_pRegex);
	
	m_bNeedFullMessage = copy.m_bNeedFullMessage;
}

SearchCriteria &SearchCriteria::operator=(const SearchCriteria &copy)
{
	DeleteAll();

	if (copy.m_pObj)
		m_pObj = new CriteriaObject(*copy.m_pObj);

	if (copy.m_pVerbTxtCmp)
		m_pVerbTxtCmp = new CriteriaVerbTextCompare(*copy.m_pVerbTxtCmp);
	if (copy.m_pVerbEqualCmp)
		m_pVerbEqualCmp = new CriteriaVerbEqualCompare(*copy.m_pVerbEqualCmp);
	if (copy.m_pVerbNumCmp)
		m_pVerbNumCmp = new CriteriaVerbNumCompare(*copy.m_pVerbNumCmp);
	if (copy.m_pVerbDateCmp)
		m_pVerbDateCmp = new CriteriaVerbDateCompare(*copy.m_pVerbDateCmp);

	if (copy.m_pValueText)
		m_pValueText = new CriteriaValueText(*copy.m_pValueText);

	if (copy.m_pValueAttachCount)
		m_pValueAttachCount = new CriteriaValueAttachCount(*copy.m_pValueAttachCount);
	if (copy.m_pValueSize)
		m_pValueSize = new CriteriaValueSize(*copy.m_pValueSize);
	if (copy.m_pValueStatus)
		m_pValueStatus = new CriteriaValueStatus(*copy.m_pValueStatus);
	if (copy.m_pValueLabel)
		m_pValueLabel = new CriteriaValueLabel(*copy.m_pValueLabel);
	if (copy.m_pValuePersona)
		m_pValuePersona = new CriteriaValuePersona(*copy.m_pValuePersona);
	if (copy.m_pValuePriority)
		m_pValuePriority = new CriteriaValuePriority(*copy.m_pValuePriority);
	if (copy.m_pValueDate)
		m_pValueDate = new CriteriaValueDate(*copy.m_pValueDate);
	if (copy.m_pValueAge)
		m_pValueAge = new CriteriaValueAge(*copy.m_pValueAge);

	if (copy.m_pRegex)
		m_pRegex = new CRegex(*copy.m_pRegex);
	
	m_bNeedFullMessage = copy.m_bNeedFullMessage;

	return (*this);
}

/* static */ bool SearchCriteria::GetVerbValPair(const CriteriaObject &obj, CriteriaVerbType &verb, CriteriaValueType &val)
{
	verb = SearchCriteria::GetVerbType(obj);
	val = SearchCriteria::GetValueType(obj);

	return (true);
}

/* static */ CriteriaVerbType SearchCriteria::GetVerbType(const CriteriaObject &obj)
{
	return (gCategoryTable[obj].verbType);
}

/* static */ CriteriaValueType SearchCriteria::GetValueType(const CriteriaObject &obj)
{
	return (gCategoryTable[obj].valueType);
}

// Get

bool SearchCriteria::GetCurObj(CriteriaObject &obj) const
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	obj = (*m_pObj);

	return (true);
}

CriteriaObject SearchCriteria::GetCriteria() const
{
	ASSERT(m_pObj);
	return *m_pObj;
}



bool SearchCriteria::GetCurVerbTextCmp(CriteriaVerbTextCompare &verb) const
{
	ASSERT(m_pVerbTxtCmp);

	if (!m_pVerbTxtCmp)
		return (false);

	verb = (*m_pVerbTxtCmp);

	return (true);
}

CriteriaVerbTextCompare	SearchCriteria::GetVerbTextCmp() const
{
	ASSERT(m_pVerbTxtCmp);
	return (*m_pVerbTxtCmp);
}



bool SearchCriteria::GetCurVerbEqualCmp(CriteriaVerbEqualCompare &verb) const
{
	ASSERT(m_pVerbEqualCmp);

	if (!m_pVerbEqualCmp)
		return (false);

	verb = (*m_pVerbEqualCmp);

	return (true);
}

CriteriaVerbEqualCompare SearchCriteria::GetVerbEqualCmp()	const
{
	ASSERT(m_pVerbEqualCmp);
	return (*m_pVerbEqualCmp);
}



bool SearchCriteria::GetCurVerbNumCmp(CriteriaVerbNumCompare &verb) const
{
	ASSERT(m_pVerbNumCmp);

	if (!m_pVerbNumCmp)
		return (false);

	verb = (*m_pVerbNumCmp);

	return (true);
}

CriteriaVerbNumCompare SearchCriteria::GetVerbNumCmp()	const
{
	ASSERT(m_pVerbNumCmp);
	return (*m_pVerbNumCmp);
}


bool SearchCriteria::GetCurVerbDateCmp(CriteriaVerbDateCompare &verb) const
{
	ASSERT(m_pVerbDateCmp);

	if (!m_pVerbDateCmp)
		return (false);

	verb = (*m_pVerbDateCmp);

	return (true);
}

CriteriaVerbDateCompare	SearchCriteria::GetVerbDateCmp() const
{
	ASSERT(m_pVerbDateCmp);
	return (*m_pVerbDateCmp);
}




bool SearchCriteria::GetCurValueText(CriteriaValueText &val) const
{
	ASSERT(m_pValueText);

	if (!m_pValueText)
		return (false);

	val = (*m_pValueText);

	return (true);
}

CriteriaValueText SearchCriteria::GetValueText() const
{
	ASSERT(m_pValueText);
	return  (*m_pValueText);
}



bool SearchCriteria::GetCurValueSize(CriteriaValueSize &val) const
{
	ASSERT(m_pValueSize);

	if (!m_pValueSize)
		return (false);

	val = (*m_pValueSize);

	return (true);
}

CriteriaValueSize SearchCriteria::GetValueSize()	const
{
	ASSERT(m_pValueSize);
	return (*m_pValueSize);
}

bool SearchCriteria::GetCurValueAttachCount(CriteriaValueAttachCount &val) const
{
	ASSERT(m_pValueAttachCount);

	if (!m_pValueAttachCount)
		return (false);

	val = (*m_pValueAttachCount);

	return (true);
}


CriteriaValueAttachCount SearchCriteria::GetValueAttachCount()	const
{
	ASSERT(m_pValueAttachCount);
	return (*m_pValueAttachCount);
}


bool SearchCriteria::GetCurValueStatus(CriteriaValueStatus &val) const
{
	ASSERT(m_pValueStatus);

	if (!m_pValueStatus)
		return (false);

	val = (*m_pValueStatus);

	return (true);
}

CriteriaValueStatus	SearchCriteria::GetValueStatus() const
{
	ASSERT(m_pValueStatus);
	return  (*m_pValueStatus);
}

bool SearchCriteria::GetCurValueLabel(CriteriaValueLabel &val) const
{
	ASSERT(m_pValueLabel);

	if (!m_pValueLabel)
		return (false);

	val = (*m_pValueLabel);

	return (true);
}

CriteriaValueLabel	SearchCriteria::GetValueLabel()	const
{
	ASSERT(m_pValueLabel);
	return (*m_pValueLabel);
}



bool SearchCriteria::GetCurValuePersona(CriteriaValuePersona &val) const
{
	ASSERT(m_pValuePersona);

	if (!m_pValuePersona)
		return (false);

	val = (*m_pValuePersona);

	return (true);
}

CriteriaValuePersona SearchCriteria::GetValuePersona()	const
{
	ASSERT(m_pValuePersona);
	return (*m_pValuePersona);
}


bool SearchCriteria::GetCurValuePriority(CriteriaValuePriority &val) const
{
	ASSERT(m_pValuePriority);

	if (!m_pValuePriority)
		return (false);

	val = (*m_pValuePriority);

	return (true);
}

CriteriaValuePriority SearchCriteria::GetValuePriority() const
{
	ASSERT(m_pValuePriority);
	return (*m_pValuePriority);
}


bool SearchCriteria::GetCurValueDate(CriteriaValueDate &val) const
{
	ASSERT(m_pValueDate);

	if (!m_pValueDate)
		return (false);

	val = (*m_pValueDate);

	return (true);
}

CriteriaValueDate SearchCriteria::GetValueDate() const
{
	ASSERT(m_pValueDate);
	return (*m_pValueDate);
}

bool SearchCriteria::GetCurValueAge(CriteriaValueAge &val) const
{
	ASSERT(m_pValueAge);

	if (!m_pValueAge)
		return (false);

	val = (*m_pValueAge);

	return (true);
}

CriteriaValueAge SearchCriteria::GetValueAge()	const
{
	ASSERT(m_pValueAge);
	return (*m_pValueAge);
}




// Set
bool SearchCriteria::SetCurObj(const CriteriaObject &obj) // Must set this before verb or value
{
	DeleteAll();
	VERIFY(m_pObj = new CriteriaObject(obj));

	switch(*m_pObj)
	{
		//fall through
	case CRITERIA_OBJECT_ANYWHERE:
	case CRITERIA_OBJECT_HEADERS:
	case CRITERIA_OBJECT_BODY:
	case CRITERIA_OBJECT_ATTACHNAMES:
	case CRITERIA_OBJECT_TO:
	case CRITERIA_OBJECT_CC:
	case CRITERIA_OBJECT_BCC:
	case CRITERIA_OBJECT_ANYRECIP:
	case CRITERIA_OBJECT_FROM:
		m_bNeedFullMessage = true;
		break;

		//fall through
	case CRITERIA_OBJECT_SUMMARY:
	case CRITERIA_OBJECT_STATUS:
	case CRITERIA_OBJECT_PRIORITY:
	case CRITERIA_OBJECT_LABEL:
	case CRITERIA_OBJECT_DATE:
	case CRITERIA_OBJECT_SIZE:
	case CRITERIA_OBJECT_AGE:
	case CRITERIA_OBJECT_PERSONA:
	case CRITERIA_OBJECT_MAILBOXNAME:
	case CRITERIA_OBJECT_ATTACHCOUNT:
		
	//case CRITERIA_OBJECT_FROM:
	case CRITERIA_OBJECT_SUBJECT:
		m_bNeedFullMessage = false;
		break;
	
	default:
		ASSERT(0);
		break;
	}

	return (m_pObj != NULL);
}

bool SearchCriteria::SetCurVerbTextCmp(const CriteriaVerbTextCompare &verb)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetVerbType((*m_pObj)) == CRITERIA_VERB_TEXT_COMPARE_TYPE);

	DeleteVerbs();
	VERIFY(m_pVerbTxtCmp = new CriteriaVerbTextCompare(verb));

	return (m_pVerbTxtCmp != NULL);
}

bool SearchCriteria::SetCurVerbEqualCmp(const CriteriaVerbEqualCompare &verb)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetVerbType((*m_pObj)) == CRITERIA_VERB_EQUAL_COMPARE_TYPE);

	DeleteVerbs();
	VERIFY(m_pVerbEqualCmp = new CriteriaVerbEqualCompare(verb));

	return (m_pVerbEqualCmp != NULL);
}

bool SearchCriteria::SetCurVerbNumCmp(const CriteriaVerbNumCompare &verb)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetVerbType((*m_pObj)) == CRITERIA_VERB_NUM_COMPARE_TYPE);

	DeleteVerbs();
	VERIFY(m_pVerbNumCmp = new CriteriaVerbNumCompare(verb));

	return (m_pVerbNumCmp != NULL);
}

bool SearchCriteria::SetCurVerbDateCmp(const CriteriaVerbDateCompare &verb)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetVerbType((*m_pObj)) == CRITERIA_VERB_DATE_COMPARE_TYPE);

	DeleteVerbs();
	VERIFY(m_pVerbDateCmp = new CriteriaVerbDateCompare(verb));

	return (m_pVerbDateCmp != NULL);
}

bool SearchCriteria::SetCurValueText(const CriteriaValueText &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_TEXT_TYPE);

	DeleteValues();
	VERIFY(m_pValueText = new CriteriaValueText(val));

	return (m_pValueText != NULL);
}

bool SearchCriteria::SetCurValueSize(const CriteriaValueSize &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_SIZE_TYPE);

	DeleteValues();
	VERIFY(m_pValueSize = new CriteriaValueSize(val));

	return (m_pValueSize != NULL);
}

bool SearchCriteria::SetCurValueAttachCount(const CriteriaValueAttachCount &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_ATTACHCOUNT_TYPE);

	DeleteValues();
	VERIFY(m_pValueAttachCount = new CriteriaValueAttachCount(val));

	return (m_pValueAttachCount != NULL);
}


bool SearchCriteria::SetCurValueStatus(const CriteriaValueStatus &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_STATUS_TYPE);

	DeleteValues();
	VERIFY(m_pValueStatus = new CriteriaValueStatus(val));

	return (m_pValueStatus != NULL);
}

bool SearchCriteria::SetCurValueLabel(const CriteriaValueLabel &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_LABEL_TYPE);

	DeleteValues();
	VERIFY(m_pValueLabel = new CriteriaValueLabel(val));

	return (m_pValueLabel != NULL);
}

bool SearchCriteria::SetCurValuePersona(const CriteriaValuePersona &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_PERSONA_TYPE);

	DeleteValues();
	VERIFY(m_pValuePersona = new CriteriaValuePersona(val));

	return (m_pValuePersona != NULL);
}

bool SearchCriteria::SetCurValuePriority(const CriteriaValuePriority &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_PRIORITY_TYPE);

	DeleteValues();
	VERIFY(m_pValuePriority = new CriteriaValuePriority(val));

	return (m_pValuePriority != NULL);
}

bool SearchCriteria::SetCurValueDate(const CriteriaValueDate &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_DATE_TYPE);

	DeleteValues();
	VERIFY(m_pValueDate = new CriteriaValueDate(val));

	return (m_pValueDate != NULL);
}

bool SearchCriteria::SetCurValueAge(const CriteriaValueAge &val)
{
	ASSERT(m_pObj);

	if (!m_pObj)
		return (false);

	ASSERT(SearchCriteria::GetValueType((*m_pObj)) == CRITERIA_VALUE_AGE_TYPE);

	DeleteValues();
	VERIFY(m_pValueAge = new CriteriaValueAge(val));

	return (m_pValueAge != NULL);
}

bool operator <(const SearchCriteria Sct1, const SearchCriteria Sct2)
{
	if(Sct1.m_bNeedFullMessage == false && Sct2.m_bNeedFullMessage == true)
		return true;
	else
		return false;
}

void SearchCriteria::PreProcessForMatching()
{
	if( m_pVerbTxtCmp != NULL)
	{
		if( *m_pVerbTxtCmp == CRITERIA_TEXT_COMPARE_MATCHESREGEXP)
		{
			m_pRegex = new CRegex(GetValueText(), true);  //case-insensitive regex
		}
		else if( *m_pVerbTxtCmp == CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE)
		{
			m_pRegex = new CRegex(GetValueText(), false);  //case-sensitive regex
		}
		else if( *m_pVerbTxtCmp == CRITERIA_TEXT_COMPARE_CONTAINSWORD)
		{
			m_pValueText->TrimLeft();
			m_pValueText->TrimRight();
		}
		else{

			(*m_pValueText).MakeLower();
		}
	}
}



// --------------------------------------------------------------------------
// MultSearchCriteria
// --------------------------------------------------------------------------

MultSearchCriteria::MultSearchCriteria()
	: m_operator(OP_UNKNOWN), m_CriteriaList(), m_nTextCmpCriteria(0), m_bHasNegationSearch(false)
{
}

MultSearchCriteria::~MultSearchCriteria()
{
}

MultSearchCriteria::MultSearchCriteria(const MultSearchCriteria &copy)
	: m_operator(copy.m_operator), m_CriteriaList(copy.m_CriteriaList),
	  m_Index(copy.m_Index), m_nTextCmpCriteria(copy.m_nTextCmpCriteria),
	  m_bHasNegationSearch(copy.m_bHasNegationSearch)
{ }

bool MultSearchCriteria::IsOpAND()
{
	ASSERT(OP_UNKNOWN != m_operator);
	return (OP_AND == m_operator);
}

bool MultSearchCriteria::IsOpOR()
{
	ASSERT(OP_UNKNOWN != m_operator);
	return (OP_OR == m_operator);
}

SearchCriteriaList * MultSearchCriteria::GetCriteriaList()
{
	return (&m_CriteriaList);
}


void MultSearchCriteria::Add(const SearchCriteria& Sct)
{
	m_CriteriaList.push_back(Sct);
}

void MultSearchCriteria::SetOpAND()
{
	m_operator = OP_AND;
}

void MultSearchCriteria::SetOpOR()
{
	m_operator = OP_OR;
}

bool MultSearchCriteria::NonSummarySearch()
{
	//for each search criteria, compute offsets and save em in a set
	for(SearchCriteriaIter iter = m_CriteriaList.begin(); iter != m_CriteriaList.end(); ++iter)
	{
		if( !(*iter).NeedFullMessage())
			return false;
	}
	return true;
}


void MultSearchCriteria::PreProcessForMatching()
{
	sort(m_CriteriaList.begin(), m_CriteriaList.end());

	m_nTextCmpCriteria = 0; //rest
	m_bHasNegationSearch = false;
	//for each search criteria, compute offsets and save em in a set
	for(unsigned int i = 0; i < m_CriteriaList.size(); i++)
	{
		m_CriteriaList[i].PreProcessForMatching();
		
		if( m_CriteriaList[i].GetVerbType(m_CriteriaList[i].GetCriteria()) == CRITERIA_VERB_TEXT_COMPARE_TYPE)
		{
			m_nTextCmpCriteria++;
			if( m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN ||
							m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_ISNOT )
					m_bHasNegationSearch = true;
		}
	}
}
			

bool MultSearchCriteria::GetSearchString(CString& strSearch, bool *bWholeWord, eStart start)
{

	*bWholeWord  = false;
	if(start == FIRST)
		m_Index = 0;

	//for each search criteria, compute offsets and save em in a set
	for(unsigned int i = m_Index; i < m_CriteriaList.size(); i++)
	{
		if( m_CriteriaList[i].GetVerbType(m_CriteriaList[i].GetCriteria()) == CRITERIA_VERB_TEXT_COMPARE_TYPE)
		{
			if(!(
				m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN || 
				m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_ISNOT || 
				m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_MATCHESREGEXP || 
				m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE )
				)
			{
				strSearch = m_CriteriaList[i].GetValueText();
				if( m_CriteriaList[i].GetVerbTextCmp() == CRITERIA_TEXT_COMPARE_CONTAINSWORD)
					*bWholeWord = true;
				m_Index = i + 1;  //start from next one
				return true;
			}
		}
	}
	return false;
}

