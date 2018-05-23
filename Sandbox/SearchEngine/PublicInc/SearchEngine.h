// SearchEngine.h
//
// 11/2/98 - Scott Manjourides

#ifndef _SEARCHENGINE_H_
#define _SEARCHENGINE_H_

// --------------------------------------------------------------------------

#include "SearchCriteria.h"
#include "SearchResult.h"
#include <list> // STL list
#include <set>
#include "Callbacks.h"

// --------------------------------------------------------------------------

// Return false to terminate search, true to continue
//typedef bool (ResultCallback_fn)(list<SearchResult *> *);
typedef Callback1wRet< list<SearchResult> &, bool> ResultsCBType;
typedef list<CString>::const_iterator MbxIterType;

class CImapResults
{
	CString m_strName;
	bool m_bSearched;
	int nMsgsMatchedUnknown;   // msgs matched but we dont have summaries for --> above MAXUID
	int nMsgsUnknown;          // msgs on server with UIDs > MAXUID
	int nMsgsKnownUnfetched;   // Have summaries but never downloaded

public:
	CImapResults() : nMsgsMatchedUnknown(0), nMsgsUnknown(0), nMsgsKnownUnfetched(0), m_bSearched(false){}

	void SetName(CString &name){ m_strName = name; }
	void SetMatchedUnknownMsgs(int nMsgs)	{ nMsgsMatchedUnknown = nMsgs; }
	void SetUnknownMsgs(int nMsgs)			{ nMsgsUnknown = nMsgs; }
	void SetKnownUnfetchedMsgs(int nMsgs)	{ nMsgsKnownUnfetched = nMsgs; }
	void SetSearched(bool bSrched){  m_bSearched = bSrched;}

	int GetMatchedUnknownMsgs()	{ return nMsgsMatchedUnknown; }
	int GetUnknownMsgs()		{ return nMsgsUnknown; }
	int GetKnownUnfetchedMsgs()	{ return nMsgsKnownUnfetched; }
	

	bool IsServerSearched(){ return m_bSearched; }

};

typedef vector<CImapResults> typeImapResults;
typedef vector<CImapResults>::const_iterator ImapResultsIter;

// --------------------------------------------------------------------------
class CSummary;

class SearchEngine
{
public:

	//initialize with criterias and mailbox names to search
	bool Init(MultSearchCriteria *pMSC, const list<CString> *pMbxNames);

	//start searching by hooking-up a results callback (freq in milliseconds)
	bool Start(ResultsCBType rcb = NULL, unsigned int nFreq = 1000);

	int HasImapResults(){ return m_dsImapResults.size(); }
	
	const typeImapResults& GetImapResults(){ return m_dsImapResults; }
	int GetImapSkipCount(){ return m_nImapSkipCount; }



public:
	SearchEngine();
	SearchEngine(MultSearchCriteria *pMSC, const list<CString> *pMbxNames);
		
	~SearchEngine();
	SearchEngine(const SearchEngine &);
	SearchEngine &operator=(const SearchEngine &);

	
	//internals
private:	
	CriteriaValueText GetTextToSearch(CSummary *pSum, CriteriaObject Criteria, char *szMessage);
	bool GetStatusToSearch(CSummary *pSum, CriteriaObject Criteria, CriteriaValueStatus& status);

	bool SearchMessage(CSummary *pSum, SearchCriteria Sct, char *szMessage);
	static void InitializeHtmlKeywords();
	
	bool FindStringInFile(MultSearchCriteria& MSC, const char *szFileName, set<long>& Offsets);
	bool FindPossibleHitSummaries(MultSearchCriteria& MSC, const char *szFileName, set<CSummary *>& HitSums);
	bool NeedToOpenMessageForRefinement(bool bFoundSummary, SearchCriteria& Sct, bool *pbFinalMatch);

	
protected:
	const list<CString> *m_pMbxNames;
	MultSearchCriteria *m_pMSC;
	int m_CorrectResults;
	vector<CImapResults> m_dsImapResults;
	int m_nImapSkipCount;
	bool m_bStopRequested;
};


#define QCSTR_SUBSTR_SEARCH    (0)
#define QCSTR_WORD_SEARCH      (1)
#define QCSTR_BEGWORD_SEARCH   (2)
#define QCSTR_ENDWORD_SEARCH   (3)

#define QCSTR_CASE_SENSITIVE true
#define QCSTR_CASE_INSENSITIVE false

char *QCStrstr(const char * str1,  const char * str2, 
				   bool bMatchCase = QCSTR_CASE_SENSITIVE, int nWholeWord = QCSTR_SUBSTR_SEARCH);


#endif // _SEARCHENGINE_H_
