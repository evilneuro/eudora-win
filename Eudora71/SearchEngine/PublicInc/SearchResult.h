// SearchResult.h
//
// 11/2/98 - Scott Manjourides

#ifndef _SEARCHRESULT_H_
#define _SEARCHRESULT_H_

// --------------------------------------------------------------------------

#include "SearchCriteria.h"

// --------------------------------------------------------------------------

#define SAFEDELETE(p) { if (p) { delete p; p = NULL; } }

//	Forward declarations
class CSummary;
namespace X1
{
	struct IX1Database;
}

// --------------------------------------------------------------------------

class SearchResult
{

	CString m_strPath;
	CString m_strTitle;
	long m_UniqueID;
	CString m_Subject;
	CString m_From;
	CString m_Date;
	long m_Seconds;
	int m_nMatchIndex;
	
public:
	SearchResult();
	SearchResult(CSummary *pSum);
	SearchResult(X1::IX1Database * in_pX1EmailDB, long in_nItemNum);
	~SearchResult();
	SearchResult(const SearchResult &);

	//access methods
	CString GetMbxName() const	{ return m_strTitle; }
	CString GetMbxPath() const	{ return m_strPath; }
	CString GetSubject() const	{ return m_Subject; }
	CString GetWho() const		{ return m_From; }
	CString GetDate() const		{ return m_Date; } 
	long GetSeconds() const		{ return m_Seconds; }
	long	GetMsgID() const	{ return m_UniqueID; }
	int GetMatchedTextIndex()	{ return m_nMatchIndex; }
	

protected:

};

#endif // _SEARCHRESULT_H_
