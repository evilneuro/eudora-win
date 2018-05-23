/////////////////////////////////////////////////////////////////////////////
// 	File: SEARCHUTIL.H 
//
//  Purpose:  Helper class for conversion of MultiSearchCriteria lists to search
//			  programs.
//
//	Author: 
//	Created:  11/16/98
//  Modified:
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _SEARCHUTIL_H_
#define _SEARCHUTIL_H_


// Specifies which field of a SEARCHPGM to add a given text search criterion.
//
typedef enum
{
	PGMTEXT_BCC = 1,
	PGMTEXT_BODY,
	PGMTEXT_CC,
	PGMTEXT_FROM,
	PGMTEXT_KEYWORD,
	PGMTEXT_UNKEYWORD,
	PGMTEXT_SUBJECT,
	PGMTEXT_TEXT,
	PGMTEXT_TO,
}PGMTEXT_WHERE;


class	MultSearchCriteria;
class	SearchCriteria;


class CSearchUtil
{
public:

	CSearchUtil();
	~CSearchUtil();

// Main entry point.
	BOOL FormatSearchCriteria (SEARCHPGM** ppTopPgm, MultSearchCriteria* pMultiSearchCriteria, LPCSTR pUidRange);


	// Is this criterion one that an IMAP server can handle??
	//
	static BOOL	ImapServerCanSearch (SearchCriteria *pCurSearchCriteria);
	
	

// Search criteria manipulation.
private:

	BOOL	Initialize (MultSearchCriteria* pMultSearchCriteria);

	SearchCriteria* GetNextCriterion ();

	// 
	BOOL IsOpOR(SearchCriteria* pCriterion);
	BOOL IsOpAND(SearchCriteria* pCriterion);

	BOOL HasNext();

	BOOL	AddORSearchCriteria (SEARCHPGM*  pCurPgm, SearchCriteria *pCurSearchCriteria);
	BOOL	AddANDSearchCriteria (SEARCHPGM* pCurPgm, SearchCriteria *pCurSearchCriteria);

	// Add single criterion without recursing.
	// Can have multiple criteria which are ANDED.
	//
	BOOL	AddSingleCriterion (SEARCHPGM* pCurPgm, SearchCriteria *pCurSearchCriteria);

	BOOL	AddTextObject (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria, PGMTEXT_WHERE where);
	BOOL	AddObjectFullHeader (SEARCHPGM* pCurPgm, SearchCriteria *pCurSearchCriteria);
	BOOL	AddObjectDate (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria);
	BOOL	AddObjectAge (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria);
	BOOL	AddObjectHeader (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria, CriteriaObject curObjectType);
	BOOL	AddAnyRecipient (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria);
	BOOL	AddNamedHeader (SEARCHPGM* pCurPgm, SearchCriteria* pCurSearchCriteria, LPCSTR pHeaderName);

	BOOL	AddORNamedHeaderList (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria, LPCSTR pszNamedHeaders);

	BOOL	AppendTextCriterion (SEARCHPGM* pCurPgm, CriteriaValueText szText, PGMTEXT_WHERE where);

	void	FreePgmUidList (SEARCHPGM *pPgm);
	BOOL	SetNewPgmUidList (SEARCHPGM *pPgm, LPCSTR pStr);

private:
	// Cache the inputs.
	MultSearchCriteria*		m_pMultSearchCriteria;
	SearchCriteriaList*		m_pCriteriaList;

	// The iterator.
	//
	SearchCriteriaList::iterator m_Iterator;
};







#endif // _SEARCHUTIL_H_
