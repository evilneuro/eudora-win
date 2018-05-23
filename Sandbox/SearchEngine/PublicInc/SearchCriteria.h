// SearchCriteria.h
//
// 10/26/98 - Scott Manjourides

#ifndef _SEARCHCRITERIA_H_
#define _SEARCHCRITERIA_H_

// --------------------------------------------------------------------------

#include "vector"
#include "set"

// --------------------------------------------------------------------------

#define CRITERIA_OBJECT_COUNT    (20)

typedef enum {
	CRITERIA_OBJECT_ANYWHERE = 0,
	CRITERIA_OBJECT_HEADERS,
	CRITERIA_OBJECT_BODY,
	CRITERIA_OBJECT_ATTACHNAMES,
	CRITERIA_OBJECT_SUMMARY,
	CRITERIA_OBJECT_STATUS,
	CRITERIA_OBJECT_PRIORITY,
	CRITERIA_OBJECT_ATTACHCOUNT,
	CRITERIA_OBJECT_LABEL,
	CRITERIA_OBJECT_DATE,
	CRITERIA_OBJECT_SIZE,
	CRITERIA_OBJECT_AGE,
	CRITERIA_OBJECT_PERSONA,
	CRITERIA_OBJECT_MAILBOXNAME,
	CRITERIA_OBJECT_TO,
	CRITERIA_OBJECT_FROM,
	CRITERIA_OBJECT_SUBJECT,
	CRITERIA_OBJECT_CC,
	CRITERIA_OBJECT_BCC,
	CRITERIA_OBJECT_ANYRECIP
} CriteriaObject;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ** VERBS **

typedef enum {
	CRITERIA_VERB_TEXT_COMPARE_TYPE,
	CRITERIA_VERB_EQUAL_COMPARE_TYPE,
	CRITERIA_VERB_NUM_COMPARE_TYPE,
	CRITERIA_VERB_DATE_COMPARE_TYPE
} CriteriaVerbType;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// CRITERIA_VERB_TEXT_COMPARE_TYPE
typedef enum {
	CRITERIA_TEXT_COMPARE_CONTAINS = 0,
	CRITERIA_TEXT_COMPARE_CONTAINSWORD,
	CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN,
	CRITERIA_TEXT_COMPARE_IS,
	CRITERIA_TEXT_COMPARE_ISNOT,
	CRITERIA_TEXT_COMPARE_STARTSWITH,
	CRITERIA_TEXT_COMPARE_ENDSWITH,
	CRITERIA_TEXT_COMPARE_MATCHESREGEXP,
	CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE
} CriteriaVerbTextCompare;

// CRITERIA_VERB_EQUAL_COMPARE_TYPE
typedef enum {
	CRITERIA_EQUAL_COMPARE_IS = 0,
	CRITERIA_EQUAL_COMPARE_ISNOT
} CriteriaVerbEqualCompare;

// CRITERIA_VERB_NUM_COMPARE_TYPE
typedef enum {
	CRITERIA_NUM_COMPARE_IS = 0,
	CRITERIA_NUM_COMPARE_ISNOT,
	CRITERIA_NUM_COMPARE_ISGREATERTHAN,
	CRITERIA_NUM_COMPARE_ISLESSTHAN
} CriteriaVerbNumCompare;

// CRITERIA_VERB_DATE_COMPARE_TYPE
typedef enum {
	CRITERIA_DATE_COMPARE_IS = 0,
	CRITERIA_DATE_COMPARE_ISNOT,
	CRITERIA_DATE_COMPARE_ISAFTER,
	CRITERIA_DATE_COMPARE_ISBEFORE
} CriteriaVerbDateCompare;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ** VALUES **

typedef enum {
	CRITERIA_VALUE_TEXT_TYPE,
	CRITERIA_VALUE_ATTACHCOUNT_TYPE,
	CRITERIA_VALUE_SIZE_TYPE,
	CRITERIA_VALUE_STATUS_TYPE,
	CRITERIA_VALUE_LABEL_TYPE,
	CRITERIA_VALUE_PERSONA_TYPE,
	CRITERIA_VALUE_PRIORITY_TYPE,
	CRITERIA_VALUE_DATE_TYPE,
	CRITERIA_VALUE_AGE_TYPE
} CriteriaValueType;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// CRITERIA_VALUE_TEXT_TYPE
typedef CString CriteriaValueText;

// CRITERIA_VALUE_ATTACHCOUNT_TYPE
typedef int CriteriaValueAttachCount;

// CRITERIA_VALUE_SIZE_TYPE
typedef int CriteriaValueSize;

// CRITERIA_VALUE_STATUS_TYPE
typedef enum {
	CRITERIA_VALUE_STATUS_UNREAD = 0,
	CRITERIA_VALUE_STATUS_READ,
	CRITERIA_VALUE_STATUS_REPLIED,
	CRITERIA_VALUE_STATUS_FORWARDED,
	CRITERIA_VALUE_STATUS_REDIRECTED,
	CRITERIA_VALUE_STATUS_SENT,
	CRITERIA_VALUE_STATUS_SENDABLE,
	CRITERIA_VALUE_STATUS_UNSENT,
	CRITERIA_VALUE_STATUS_QUEUED,
	CRITERIA_VALUE_STATUS_TIME_QUEUED,
	CRITERIA_VALUE_STATUS_UNSENDABLE,
	CRITERIA_VALUE_STATUS_RECOVERED
} CriteriaValueStatus;

// CRITERIA_VALUE_LABEL_TYPE
typedef int CriteriaValueLabel;

// CRITERIA_VALUE_PERSONA_TYPE
typedef CString CriteriaValuePersona;

// CRITERIA_VALUE_PRIORITY_TYPE
typedef enum {
	CRITERIA_VALUE_PRIORITY_HIGHEST = 0,
	CRITERIA_VALUE_PRIORITY_HIGH,
	CRITERIA_VALUE_PRIORITY_NORMAL,
	CRITERIA_VALUE_PRIORITY_LOW,
	CRITERIA_VALUE_PRIORITY_LOWEST
} CriteriaValuePriority;

// CRITERIA_VALUE_DATE_TYPE
typedef COleDateTime CriteriaValueDate;

// CRITERIA_VALUE_AGE_TYPE
typedef unsigned int CriteriaValueAge; // Num days

// --------------------------------------------------------------------------

class CRegex;

class SearchCriteria
{
public:
	SearchCriteria(); // Standard constructor
	~SearchCriteria(); // Destructor
	SearchCriteria(const SearchCriteria &); // Copy constructor

	SearchCriteria &operator=(const SearchCriteria &); // Assignment

	// Get

	static bool GetVerbValPair(const CriteriaObject &obj, CriteriaVerbType &verb, CriteriaValueType &val);
	static CriteriaVerbType GetVerbType(const CriteriaObject &obj);
	static CriteriaValueType GetValueType(const CriteriaObject &obj);

	bool GetCurObj(CriteriaObject &) const;
	
	CriteriaObject GetCriteria() const;
	
	bool GetCurVerbTextCmp(CriteriaVerbTextCompare &) const;
	bool GetCurVerbEqualCmp(CriteriaVerbEqualCompare &) const;
	bool GetCurVerbNumCmp(CriteriaVerbNumCompare &) const;
	bool GetCurVerbDateCmp(CriteriaVerbDateCompare &) const;

	CriteriaVerbTextCompare		GetVerbTextCmp()	const;
	CriteriaVerbEqualCompare	GetVerbEqualCmp()	const;
	CriteriaVerbNumCompare		GetVerbNumCmp()		const;
	CriteriaVerbDateCompare		GetVerbDateCmp()	const;

	bool GetCurValueText(CriteriaValueText &) const;
	bool GetCurValueAttachCount(CriteriaValueAttachCount &) const;
	bool GetCurValueSize(CriteriaValueSize &) const;
	bool GetCurValueStatus(CriteriaValueStatus &) const;
	bool GetCurValueLabel(CriteriaValueLabel &) const;
	bool GetCurValuePersona(CriteriaValuePersona &) const;
	bool GetCurValuePriority(CriteriaValuePriority &) const;
	bool GetCurValueDate(CriteriaValueDate &) const;
	bool GetCurValueAge(CriteriaValueAge &) const;

	CriteriaValueText			GetValueText()			const;
	CriteriaValueSize			GetValueSize()			const;
	CriteriaValueAttachCount	GetValueAttachCount()	const;
	CriteriaValueStatus			GetValueStatus()		const;
	CriteriaValueLabel			GetValueLabel()			const;
	CriteriaValuePersona		GetValuePersona()		const;
	CriteriaValuePriority		GetValuePriority()		const;
	CriteriaValueDate			GetValueDate()			const;
	CriteriaValueAge			GetValueAge()			const;


	// Set

	bool SetCurObj(const CriteriaObject &); // Must set this before verb or value

	bool SetCurVerbTextCmp(const CriteriaVerbTextCompare &);
	bool SetCurVerbEqualCmp(const CriteriaVerbEqualCompare &);
	bool SetCurVerbNumCmp(const CriteriaVerbNumCompare &);
	bool SetCurVerbDateCmp(const CriteriaVerbDateCompare &);

	bool SetCurValueText(const CriteriaValueText &);
	bool SetCurValueAttachCount(const CriteriaValueAttachCount &);
	bool SetCurValueSize(const CriteriaValueSize &);
	bool SetCurValueStatus(const CriteriaValueStatus &);
	bool SetCurValueLabel(const CriteriaValueLabel &);
	bool SetCurValuePersona(const CriteriaValuePersona &);
	bool SetCurValuePriority(const CriteriaValuePriority &);
	bool SetCurValueDate(const CriteriaValueDate &);
	bool SetCurValueAge(const CriteriaValueAge &);

	//properties
	bool NeedFullMessage()const { return m_bNeedFullMessage; }
	friend bool operator <(const SearchCriteria Sct1, const SearchCriteria Sct2);

	void PreProcessForMatching();
	CRegex *GetRegex(){ return m_pRegex; }

protected:
	void DeleteAll();
	void DeleteVerbs();
	void DeleteValues();

	CriteriaObject *m_pObj;

	// Only one verb is ever non-NULL at any given time
	CriteriaVerbTextCompare  *m_pVerbTxtCmp;
	CriteriaVerbEqualCompare *m_pVerbEqualCmp;
	CriteriaVerbNumCompare   *m_pVerbNumCmp;
	CriteriaVerbDateCompare  *m_pVerbDateCmp;

	// Only one value is ever non-NULL at any given time
	CriteriaValueText         *m_pValueText;
	CriteriaValueAttachCount  *m_pValueAttachCount;
	CriteriaValueSize         *m_pValueSize;
	CriteriaValueStatus       *m_pValueStatus;
	CriteriaValueLabel        *m_pValueLabel;
	CriteriaValuePersona      *m_pValuePersona;
	CriteriaValuePriority     *m_pValuePriority;
	CriteriaValueDate         *m_pValueDate;
	CriteriaValueAge          *m_pValueAge;

	bool m_bNeedFullMessage;
	CRegex *m_pRegex;
};

// --------------------------------------------------------------------------

typedef vector<SearchCriteria> SearchCriteriaList;
typedef vector<SearchCriteria>::const_iterator SearchCriteriaIter;
typedef vector<SearchCriteria>::iterator SCIter;

class CSummary;
typedef set<CSummary *> CSumSet;
typedef set<CSummary *>::iterator CSumIter;

// --------------------------------------------------------------------------

class MultSearchCriteria
{
public:
	MultSearchCriteria();
	~MultSearchCriteria();
	MultSearchCriteria(const MultSearchCriteria &);

	bool IsOpAND();
	bool IsOpOR();
	SearchCriteriaList *GetCriteriaList(); 
	bool NonSummarySearch();

	void SetOpAND();
	void SetOpOR();
	void Add(const SearchCriteria& Sct);
	void PreProcessForMatching();
	bool GetSearchString(CString& strSearch, bool *bWholeWord);
	enum eStart { FIRST, NEXT } ;

	bool GetSearchString(CString& strSearch, bool *bWholeWord, eStart start);
	bool HasSingleTextCmp() const { return m_nTextCmpCriteria == 1; }
	bool HasNegationSearch() const { return m_bHasNegationSearch; }

private:
	SearchCriteriaList m_CriteriaList;
	unsigned int m_Index;
	int m_nTextCmpCriteria;
	bool m_bHasNegationSearch;

protected:
	enum { OP_AND, OP_OR, OP_UNKNOWN } m_operator;
	
};

#endif // _SEARCHCRITERIA_H_
