// SearchEngine.cpp
//
// 11/2/98 - Scott Manjourides

#include "stdafx.h"
#include "SearchCriteria.h"
#include "searchresult.h"

#include "SearchEngine.h"

#include "resource.h"
#include "hsregex.h"

#include "summary.h"
#include "tocdoc.h"
#include "msgdoc.h"
#include "msgutils.h"
#include "Html2Text.h" //for Html2Text
#include "rs.h"  //for CRString
#include "progress.h"

#include "QCMailBoxCommand.h"
#include "QCMailboxDirector.h"  //for g_theMailboxDirector
extern QCMailboxDirector	g_theMailboxDirector;

#include "guiutils.h"

#include <vector>
#include <set>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

unsigned int ChopList(LPCSTR pStr, char cDelim = ';', list<CString> *pStrList = NULL);

CTocDoc *GetTocWithFlags(const char *szPath, bool *pLoaded);
int GetAttachments(CSummary *pSum, char *szMessage = NULL, list<CString> *pAttachStrList = NULL, bool bFullPath = false);

// --------------------------------------------------------------------------

#define SAFEDELETE(p) { if (p) { delete p; p = NULL; } }
enum eSummaryState { FOUND_HITS, FOUND_NO_HITS };

// --------------------------------------------------------------------------

SearchEngine::SearchEngine() : m_CorrectResults(1), m_pMbxNames(NULL), m_pMSC(NULL)
{
}

SearchEngine::SearchEngine(MultSearchCriteria *pMSC, const list<CString> *pMbxNames) : m_CorrectResults(1)
{
	 if( !Init(pMSC, pMbxNames))
	{
		ASSERT(0);
	}
}


SearchEngine::~SearchEngine()
{ }

SearchEngine::SearchEngine(const SearchEngine &copy)
{
	if (copy.m_pMbxNames)
		m_pMbxNames = new list<CString>(*copy.m_pMbxNames);

	if (copy.m_pMSC)
		m_pMSC = new MultSearchCriteria(*copy.m_pMSC);
}

SearchEngine &SearchEngine::operator=(const SearchEngine &copy)
{
	if (&copy != this)
	{
		SAFEDELETE(m_pMbxNames);
		SAFEDELETE(m_pMSC);

		if (copy.m_pMbxNames)
			m_pMbxNames = new list<CString>(*copy.m_pMbxNames);

		if (copy.m_pMSC)
			m_pMSC = new MultSearchCriteria(*copy.m_pMSC);
	}

	return (*this);
}

bool SearchEngine::Init(MultSearchCriteria *pMSC, const list<CString> *pMbxNames)
{
	ASSERT(pMSC);
	ASSERT(pMbxNames);

	if(!pMSC || !pMbxNames)
		return false;

	m_pMSC = pMSC;
	m_pMbxNames = pMbxNames;

	return true;
}


bool CompareTextSearchCriteria(CString& strText, SearchCriteria& Sct) //, CriteriaVerbTextCompare verb, const char *findme)
{

	CriteriaVerbTextCompare verb = Sct.GetVerbTextCmp();
	const char *findme;
	CRegex *pRegex;
	
	if( (verb == CRITERIA_TEXT_COMPARE_MATCHESREGEXP) || (verb == CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE) )
		pRegex = Sct.GetRegex();
	else
	{
		findme = Sct.GetValueText();
		strText.MakeLower();
	}
	const char *text = strText;
	
	
	//do the comaprisons
	switch(verb)
	{
	case CRITERIA_TEXT_COMPARE_CONTAINS: 
		if(strstr(text, findme))
			return true;
		break;
	
	case CRITERIA_TEXT_COMPARE_CONTAINSWORD: 
		{
			char *ptr;
			if(ptr = strstr(text, findme))
			{
				unsigned char bChar = (ptr == text)? ' ': *(ptr-1);
				unsigned char eChar = *(ptr+strlen(findme));

				if(!isalnum(bChar) && !isalnum(eChar))
					return true;
			}
		}
		break;

	case CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN: 
		if(!strstr(text, findme))
			return true;
		break;

	case CRITERIA_TEXT_COMPARE_IS: 
		if(strcmp(text, findme) == 0)
			return true;
		break;
	
	case CRITERIA_TEXT_COMPARE_ISNOT: 
		if(strcmp(text, findme) != 0)
			return true;
		break;

	case CRITERIA_TEXT_COMPARE_STARTSWITH: 
		if(strncmp(text, findme, strlen(findme)) == 0)
			return true;
		break;

	case CRITERIA_TEXT_COMPARE_ENDSWITH: 
		if(strncmp(text+strlen(text)-strlen(findme), findme, strlen(findme)) == 0)
			return true;
		break;

	case CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE: 
	case CRITERIA_TEXT_COMPARE_MATCHESREGEXP: 
		if(pRegex)
		{
			if(pRegex->Match(text))
				return true;
		}
		break;

	default:
		break;

	} //switch

	return false;
}

template <class T>
bool CompareEqualSearchCriteria(T msgValue, CriteriaVerbEqualCompare verb, T value)
{

	switch(verb)
	{
	case CRITERIA_EQUAL_COMPARE_IS:
		if( msgValue == value)
			return true;
		break;
	
	case CRITERIA_EQUAL_COMPARE_ISNOT:
		if( msgValue != value)
			return true;
		break;
	default: 
		ASSERT(0);
		break;
	}
	return false;
}

template<class T>
bool CompareNumSearchCriteria(T msgValue, CriteriaVerbNumCompare verb, T value)
{
	switch(verb)
	{
	case CRITERIA_NUM_COMPARE_IS:
		if(msgValue == value)
			return true;
		break;
	
	case CRITERIA_NUM_COMPARE_ISNOT:
		if(msgValue != value)
			return true;
		break;

	case CRITERIA_NUM_COMPARE_ISGREATERTHAN:
		if(msgValue > value)
			return true;
		break;

	case CRITERIA_NUM_COMPARE_ISLESSTHAN:
		if(msgValue < value)
			return true;
		break;

	default: 
		ASSERT(0);
		break;
	}
	return false;
}

bool CompareDateSearchCriteria(CriteriaValueDate msgDate, CriteriaVerbDateCompare verb, CriteriaValueDate date)
{
	switch(verb)
	{
	case CRITERIA_DATE_COMPARE_IS:
		if(msgDate == date)
			return true;
		break;
	
	case CRITERIA_DATE_COMPARE_ISNOT:
		if(msgDate != date)
			return true;
		break;
	
	case CRITERIA_DATE_COMPARE_ISAFTER:
		if(msgDate > date)
			return true;
		break;
	
	case CRITERIA_DATE_COMPARE_ISBEFORE:
		if(msgDate < date)
			return true;
		break;
	
	default: 
		ASSERT(0);
		break;
	}
	return false;
}




bool SearchEngine::GetStatusToSearch(CSummary *pSum, CriteriaObject Criteria, CriteriaValueStatus& status)
{
	bool bRes = true;

	char state = pSum->GetState();

#define HandleStateCase(theCase)	case MS_##theCase: status = CRITERIA_VALUE_STATUS_##theCase; break
#define MS_REDIRECTED	MS_REDIRECT	// has to be one oddball...
	switch(state)
	{
		HandleStateCase(UNREAD);
		HandleStateCase(READ);
		HandleStateCase(REPLIED);
		HandleStateCase(FORWARDED);
		HandleStateCase(REDIRECTED);
		HandleStateCase(SENT);
		HandleStateCase(SENDABLE);
		HandleStateCase(UNSENT);
		HandleStateCase(QUEUED);
		HandleStateCase(TIME_QUEUED);
		HandleStateCase(UNSENDABLE);
		HandleStateCase(RECOVERED);

	default:
		bRes = false;
		break;
	}
	return bRes;
}


CriteriaValueText SearchEngine::GetTextToSearch(CSummary *pSum, CriteriaObject Criteria, char *szMessage)
{
	switch(Criteria)
	{
	
	//Get from summary
	case CRITERIA_OBJECT_FROM:
		//return pSum->GetFrom();
		{
			char *from = NULL;
			CString strFrom = from = HeaderContents(IDS_HEADER_FROM, szMessage);
			delete [] from;
			return strFrom;
		}
		break;
	
	case CRITERIA_OBJECT_SUBJECT:
		return pSum->GetSubject();
		break;

	case CRITERIA_OBJECT_SUMMARY:
		{
		CString strText = pSum->GetFrom();
		strText += pSum->GetSubject();
		return strText;
		}
		break;
	//end get from summary

	case CRITERIA_OBJECT_ANYWHERE:
		{
			if(pSum->IsHTML() && m_CorrectResults > 0)
			{
				const char *body = FindBody(szMessage);
				CString strCorrectMsg(szMessage, body - szMessage);

				return strCorrectMsg + Html2Text(body);
			}
			else
				return CString(szMessage);
		}
		break;

	case CRITERIA_OBJECT_BODY:
		{
			const char *body = FindBody(szMessage);
			
			if(pSum->IsHTML() && m_CorrectResults > 0)
				return Html2Text(body);
			else
				return CString(body);
		}
		break;


	case CRITERIA_OBJECT_HEADERS:
		{
			const char *body = FindBody(szMessage);
			return CString(szMessage, body - szMessage);
		}
		break;
	
	case CRITERIA_OBJECT_ATTACHNAMES:
		{
			ASSERT(0); // This is special-cased in SearchMessage()
		}
		break;

	
	case CRITERIA_OBJECT_TO:
		{
			char *to = NULL;
			CString strTo = to = HeaderContents(IDS_HEADER_TO, szMessage);
			delete [] to;
			return strTo;
		}
		break;

	case CRITERIA_OBJECT_CC:
		{
			char *cc = NULL;
			CString strCc = cc  = HeaderContents(IDS_HEADER_CC, szMessage);
			delete [] cc;
			return strCc;
		}
		break;

	case CRITERIA_OBJECT_BCC:
		{
			char *bcc = NULL;
			CString strBcc = bcc = HeaderContents(IDS_HEADER_BCC, szMessage);
			delete [] bcc;
			return strBcc;
		}
		break;

	case CRITERIA_OBJECT_ANYRECIP:
		{
			char *to = NULL, *cc = NULL, *bcc = NULL;
			CString strAnyRecip = to = HeaderContents(IDS_HEADER_TO, szMessage);
			cc = HeaderContents(IDS_HEADER_CC, szMessage);
			strAnyRecip += cc;
			bcc = HeaderContents(IDS_HEADER_BCC, szMessage);
			strAnyRecip += bcc;

			delete [] to;
			delete [] cc;
			delete [] bcc;

			return strAnyRecip;
		}
		break;

	default:
		ASSERT(0);
		break;
	}
	return "";
}



bool SearchEngine::SearchMessage(CSummary *pSum, SearchCriteria Sct, char *szMessage)
{
	
	CriteriaVerbType verbType;
	CriteriaValueType valueType;
	CriteriaObject criteria;
	if(!Sct.GetCurObj(criteria))
		return false;

	SearchCriteria::GetVerbValPair(criteria, verbType, valueType);

	if(verbType == CRITERIA_VERB_TEXT_COMPARE_TYPE)
	{
		ASSERT(valueType == CRITERIA_VALUE_TEXT_TYPE);

		// Special case ATTACHMENT NAMES
		if(CRITERIA_OBJECT_ATTACHNAMES == criteria)
		{
			list<CString> StrLst;
			int nCount = GetAttachments(pSum, szMessage, &StrLst);

			for (list<CString>::iterator itr = StrLst.begin(); itr != StrLst.end(); itr++)
			{
				if (CompareTextSearchCriteria(*itr, Sct))
					return (true);
			}

			return (false);
		}
		else
		{
			CriteriaValueText strText = GetTextToSearch(pSum, criteria, szMessage);
			return CompareTextSearchCriteria(strText, Sct);
		}
	}

	
	if(verbType == CRITERIA_VERB_EQUAL_COMPARE_TYPE)
	{
		if( valueType == CRITERIA_VALUE_STATUS_TYPE)
		{
			CriteriaValueStatus status;
			if(!GetStatusToSearch(pSum, criteria, status))
				return false;

			return CompareEqualSearchCriteria(status, Sct.GetVerbEqualCmp(), Sct.GetValueStatus());
		}

		if( valueType == CRITERIA_VALUE_LABEL_TYPE)
		{
			CriteriaValueLabel nLabel = pSum->GetLabel();
			ASSERT(nLabel <= 7);
			return CompareEqualSearchCriteria(nLabel, Sct.GetVerbEqualCmp(), Sct.GetValueLabel());
		}
		
		if( valueType == CRITERIA_VALUE_PERSONA_TYPE)
		{
			///please send only CStrings as we need an == operator
			return CompareEqualSearchCriteria(pSum->GetPersona(), Sct.GetVerbEqualCmp(), Sct.GetValuePersona());
		}
	}

	if(verbType == CRITERIA_VERB_NUM_COMPARE_TYPE)
	{
		if(valueType == CRITERIA_VALUE_PRIORITY_TYPE)
		{
			CriteriaValuePriority valPriority = Sct.GetValuePriority();
			short sPriority = 0;
			if(valPriority == CRITERIA_VALUE_PRIORITY_HIGHEST)
				sPriority = MSP_HIGHEST;
			else if(valPriority == CRITERIA_VALUE_PRIORITY_HIGH)
				sPriority = MSP_HIGH;
			else if(valPriority == CRITERIA_VALUE_PRIORITY_NORMAL)
				sPriority = MSP_NORMAL;
			else if(valPriority == CRITERIA_VALUE_PRIORITY_LOW)
				sPriority = MSP_LOW;
			else if(valPriority == CRITERIA_VALUE_PRIORITY_LOWEST)
				sPriority = MSP_LOWEST;
			else { 
				ASSERT(0);
				return false;
			}

			return CompareNumSearchCriteria(pSum->m_Priority, Sct.GetVerbNumCmp(), sPriority);
		}

		if(valueType == CRITERIA_VALUE_SIZE_TYPE)
		{			
			CriteriaValueSize nValue = pSum->GetSize();
			return CompareNumSearchCriteria(nValue, Sct.GetVerbNumCmp(), Sct.GetValueSize());
		}
			
		if(valueType == CRITERIA_VALUE_ATTACHCOUNT_TYPE)
		{
			CriteriaValueAttachCount nAttachments = GetAttachments(pSum, szMessage);
			return CompareNumSearchCriteria(nAttachments, Sct.GetVerbNumCmp(), Sct.GetValueAttachCount());
		}
		
		if(valueType == CRITERIA_VALUE_AGE_TYPE)
		{
			CriteriaValueAge nAge = (time(NULL) - (pSum->m_Seconds + pSum->m_TimeZoneMinutes * 60))/(60*60*24); //days
			return CompareNumSearchCriteria(nAge, Sct.GetVerbNumCmp(), Sct.GetValueAge());
		}
	}

	if(verbType == CRITERIA_VERB_DATE_COMPARE_TYPE)
	{
		ASSERT(valueType == CRITERIA_VALUE_DATE_TYPE);
		
		//CriteriaValueDate date(time_t(pSum->m_Seconds + pSum->m_TimeZoneMinutes * 60));
		CriteriaValueDate date(time_t(pSum->m_Seconds));
		date.SetDate(date.GetYear(), date.GetMonth(), date.GetDay());
		return CompareDateSearchCriteria(date, Sct.GetVerbDateCmp(), Sct.GetValueDate());
		
	}

	//never come here; we should match above only
	ASSERT(0);
	return false;
}
	


#define USE_OFFSETS 1
bool SearchEngine::Start(ResultsCBType ReportResults /* = NULL */, unsigned int nFreq /* = 1000 */)
{
	DWORD nLastCallbackTick = GetTickCount();

	int nStart = GetTickCount();
	int nStartHits = 0;

	const SearchCriteriaList* pList = m_pMSC->GetCriteriaList();
	if(pList == NULL)
	{
		ASSERT(0);
		return false;
	}
	
	ASSERT(nFreq > 0);
	if (0 == nFreq)
		return false;


	//bool bStopRequested = false;
	m_bStopRequested = false;

	unsigned int nResults = 0;
	list<SearchResult> ResultsList;
	
	bool bOR = m_pMSC->IsOpOR()?true:false;
	m_pMSC->PreProcessForMatching(); 

	m_nImapSkipCount = 0;
	m_dsImapResults.clear(); //get rid of any previous resulst

	//collect some statistics
	int nTotalTime = 0;
	int nHits = 0;
	int nHTMLMsgs = 0;
	int nOpenedMsgs = 0;

	int nMailBoxes = m_pMbxNames->size();
	CRString strProgress(IDS_SEARCH_SEARCHING);
	int nMbxIndex = 0;

	
	for(MbxIterType MbxIter = m_pMbxNames->begin(); MbxIter != m_pMbxNames->end() && !m_bStopRequested; ++MbxIter, nMbxIndex++)
	{

		CString strFileName = *MbxIter;

		bool bForceAllMbxMatch = false;
		bool bWholeMbxFailed = false;

		int nMailboxCritCount = 0;
		bool bAtLeastOneMbxCritMatched = false;

		// Searching on mailbox name is a very special beast
		// Do all mailbox name matching now!

		// Walk through the criteria and look for MAILBOX NAME criteria
		for(SearchCriteriaIter iter = pList->begin(); (!bWholeMbxFailed) && (!bForceAllMbxMatch) && (iter != pList->end()); ++iter)
		{
			SearchCriteria Sct = *iter;
			CriteriaObject CritObj = Sct.GetCriteria();

			if (CritObj == CRITERIA_OBJECT_MAILBOXNAME)
			{
				nMailboxCritCount++;

				CString sMbxName;
				QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname(strFileName);

				ASSERT(pCommand);
				if (pCommand)
					sMbxName = pCommand->GetName();

				if (sMbxName.IsEmpty())
				{
					ASSERT(0); // Couldn't get the display name of the mailbox, defaulting to path name
					sMbxName = strFileName;
				}
				
				const bool bMatch = (CompareTextSearchCriteria(sMbxName, Sct));

				if (bMatch)
					bAtLeastOneMbxCritMatched = true;

				if ((bMatch) && (bOR))
					bForceAllMbxMatch = true;
				else if ((!bMatch) && (!bOR))
					bWholeMbxFailed = true;
			}
		}

		if (bWholeMbxFailed)
			continue;

		// Obviously bWholeMbxFailed is now always false

		// If we get here:
		//      1. AND with all mailbox crits matching      (bForceAllMbxMatch=false; bAtLeastOneMbxCritMatched=true)
		// -or- 2. OR with no mailbox crits matches         (bForceAllMbxMatch=false; bAtLeastOneMbxCritMatched=false)
		// -or- 3. OR with at least one mailbox crits match (bForceAllMbxMatch=true;  bAtLeastOneMbxCritMatched=true)

		// Now, in case 1, we want to flip bForceAllMbxMatch, because there are no more criterias (only mailbox name crits)

		if ((!bOR) && (((int)pList->size()) == nMailboxCritCount))
		{
			ASSERT(!bForceAllMbxMatch);
			ASSERT(bAtLeastOneMbxCritMatched);

			bForceAllMbxMatch = true;
		}


		int time = GetTickCount();
		nTotalTime += GetTickCount() - time;

	
		bool bComputedOffsets = false;
		bool bFoundMatches = false;
		set<CSummary *> HitSums;
		HitSums.clear();

		
		bool bLoadedToc = false;
		CTocDoc *pToc = pToc = GetTocWithFlags(strFileName, &bLoadedToc);
		if(!pToc){
			ASSERT(0); //whats up with you Dude
			return false;
		}

		bool bIsImapToc = pToc->IsImapToc()?true:false;


//		bool bIsImapToc = false;

//		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByNamedPath( strFileName );
//		if(pCommand && pCommand->IsImapType())
//				bIsImapToc = true;


		bool bDoImapServerSearch = false;
		bool bDoImapLocalSearch = false;
		bool bImapFullLocalSearch = false;
		CImapResults ImapResult;
		unsigned long nMsgsMatchedUnknown = 0;
		
		CSumList SumsFromImapServer;
		CSumSet SumsFromLocalSearch;

		if(bIsImapToc)
		{
			for(SearchCriteriaIter iter = pList->begin(); iter != pList->end(); ++iter)
			{
				SearchCriteria Sct = *iter;
				if(CTocDoc::ImapServerCanSearch(&Sct) )
					bDoImapServerSearch = true;
				else
				{
					bDoImapLocalSearch = true;
					if( !bImapFullLocalSearch && Sct.NeedFullMessage())
						bImapFullLocalSearch = true;
				}
			}

		}

//		bool bLoadedToc = false;
//		CTocDoc *pToc = NULL;
//		
//		pToc = GetTocWithFlags(strFileName, &bLoadedToc);
//		if(!pToc){
//			ASSERT(0); //whats up with you Dude
//			return false;
//		}
//
		if(bIsImapToc)
		{
			// make sure this IMAP mailbox can contain messages.  If not, there's nothing to search here. -jdboyd
			QCMailboxCommand *pCommand = g_theMailboxDirector.FindByPathname(strFileName);
			if (pCommand && !pCommand->CanContainMessages()) continue;

			ImapResult.SetName(strFileName);
			ImapResult.SetUnknownMsgs( pToc->ImapNumberOfUnknownMsgs() );
			ImapResult.SetKnownUnfetchedMsgs( pToc->ImapNumberOfUnfetchedMsgs() );

			if( bDoImapServerSearch)
			{
				if(pToc->ImapOnServerSearchAll(m_pMSC, SumsFromImapServer, &nMsgsMatchedUnknown))
				{
					ImapResult.SetSearched(true);
					ImapResult.SetMatchedUnknownMsgs(nMsgsMatchedUnknown);
				}
			}
		}
//#endif


		//for OR matches, find first criteria that matches; for AND matches, go through all the criteria
		CSumList & SumList = pToc->m_Sums;
		int nSums = SumList.GetCount();
		SearchResult Result;

		POSITION pos = SumList.GetHeadPosition();
		CSummary* pSum;
		
		for (int i = 0; pos && !m_bStopRequested; i++)
		{

//#ifdef IMAP
			if( pToc->IsImapToc() && bDoImapLocalSearch == false)
				break;
//#endif

			pSum = SumList.GetNext(pos);
			
			CMessageDoc *pMsgDoc = NULL;
			bool bNewDoc = false;
			char *szMessage = NULL;
			bool bMatched = bForceAllMbxMatch; // false;
			
			if (!bForceAllMbxMatch)
			{
				for(SearchCriteriaIter iter = pList->begin(); iter != pList->end(); ++iter)
				{
					SearchCriteria srchCriteria = *iter;
				
					if (srchCriteria.GetCriteria() == CRITERIA_OBJECT_MAILBOXNAME)
						continue; // Skip MAILBOX NAME, as it is handled elsewhere

					bool bOpenMessage = false;
					bool bDoneSearching = false;
					
					//if the message need full message search, ie we canot just find using summary
					if( srchCriteria.NeedFullMessage()  && !pMsgDoc )
					{
						
						if(!bComputedOffsets)
						{
							int nTicks = GetTickCount();
							bFoundMatches = FindPossibleHitSummaries(*m_pMSC, strFileName, HitSums);
							nStartHits += (GetTickCount() - nTicks);
							bComputedOffsets = true;
						}
						
						if( HitSums.find(pSum) == HitSums.end()) // no search strings
						{
							
							bOpenMessage = NeedToOpenMessageForRefinement(false, srchCriteria, &bMatched);
							
							//if more than one criteria is doing text searches, we can not determine which text matched
							//Message need to be opened for furthur testing
							if(!m_pMSC->HasSingleTextCmp() && bMatched == true)
								bOpenMessage = true;
							
							if(srchCriteria.GetCriteria() == CRITERIA_OBJECT_ATTACHNAMES && bMatched == true)
							{
								if(!pSum->HasAttachment()){
									bMatched = false;
									bOpenMessage = false;
								}
							}


						}
						else //found atleast one search string
						{
						
							bOpenMessage = NeedToOpenMessageForRefinement(true, srchCriteria, &bMatched);
								
							//if more than one criteria is doing text searches, we can not determine which text matched
							//Message need to be opened for furthur testing
							if(!m_pMSC->HasSingleTextCmp() && bMatched == true)
								bOpenMessage = true;

							if(srchCriteria.GetCriteria() == CRITERIA_OBJECT_ATTACHNAMES && bMatched == true)
							{
								if(!pSum->HasAttachment()){
									bMatched = false;
									bOpenMessage = false;
								}
							}
							
						}

						
						if(!bOpenMessage && (bMatched == true) && m_CorrectResults > 0 && pSum->IsHTML())
						{
							//user wants correct results even if the search is slower;
							//so open the message and do the rigth search by removing HTML
							bOpenMessage = true; // open and make sure of the hit to be correct
						}
						
						if(!bOpenMessage)
							bDoneSearching = true;
					
					}

					if( srchCriteria.GetCriteria() == CRITERIA_OBJECT_ATTACHCOUNT)
					{
						if(pSum->HasAttachment())
							bOpenMessage = true;
					}
						

					if(bOpenMessage)
					{
						//if done searching, why we are opening the msg amigo
						ASSERT(bDoneSearching == false);
						nOpenedMsgs++;
						if(pSum->IsHTML())
							nHTMLMsgs++;

						bNewDoc = (pSum->FindMessageDoc() == NULL);
						pMsgDoc = (CMessageDoc *)pSum->GetMessageDoc();
						if (!pMsgDoc)
						{
							ASSERT(0);
							continue;
						}

						//get the full message
						szMessage = pMsgDoc->GetFullMessage();
					}

					
					if(!bDoneSearching)
					{
						//Search the message
						if(SearchMessage(pSum, srchCriteria, szMessage))
							bMatched = true;
						else
							bMatched = false;
					}				

					if(bOR)
					{
						if(bMatched)   // any one good match, we are out of here
							break;
					}
					else 
					{
						if(!bMatched)  //any one bad match, we failed
							break;
					}
					
				}//for each search criteria
			}
						
			//pack the results and send 'em if needed
			if(bMatched)
			{
				if(bDoImapServerSearch)
				{
					SumsFromLocalSearch.insert(pSum);
				}
				else
				{
					//Build search result
					ResultsList.push_back( SearchResult(pSum));
					nResults++;
				}
			}

			if ((ReportResults) && !bDoImapServerSearch && ((GetTickCount() - nLastCallbackTick) >= nFreq))
			{
				
				//Let's report progress first
				Progress(nMbxIndex*100 + (i*100/nSums), strProgress + pToc->Name(), nMailBoxes*100);
				
				//report the results to the UI
				if(ReportResults(ResultsList) == false)
					m_bStopRequested = true;
				
				//ready for next results
				ResultsList.clear();
				nResults = 0; //reset

				nLastCallbackTick = GetTickCount();
			}
		
			//cleanup
			if(szMessage)
				delete [] szMessage;

			if(bNewDoc)
				pMsgDoc->OnCloseDocument();
		
#ifdef USE_OFFSETS
			if(m_pMSC->NonSummarySearch() && !m_pMSC->HasNegationSearch() && !bFoundMatches)
				break;
#endif

		} //for each summary


//#ifdef IMAP
		if(pToc->IsImapToc())
		{
			//If Imap, report using imap results object
			m_dsImapResults.push_back(ImapResult);
		}

		if(pToc->IsImapToc() && bDoImapServerSearch)
		{
			if( m_pMSC->GetCriteriaList()->size() == 1 || 
				m_pMSC->IsOpOR() ||
				(m_pMSC->IsOpAND() && (bDoImapLocalSearch == false)) )
			{
				int Count = SumsFromImapServer.GetCount();
				POSITION spos = SumsFromImapServer.GetHeadPosition();
				for (int i = 0; i < Count; i++)
					SumsFromLocalSearch.insert(SumsFromImapServer.GetNext(spos));
			}
			else  // AND
			{
//				int Count = SumsFromImapServer.GetCount();
//				POSITION spos = SumsFromImapServer.GetHeadPosition();
//				for (int i = 0; i < Count; i++)
//				{
//					CSummary *pSum = SumsFromImapServer.GetNext(spos);
//					CSumIter SI = SumsFromLocalSearch.find(pSum);
//					if( SI == SumsFromLocalSearch.end())
//						SumsFromLocalSearch.erase( SI );
//				}

				CSumIter SI = SumsFromLocalSearch.begin(); 
				while( SI != SumsFromLocalSearch.end())
				{
					CSummary *pSum = *SI;
					if( SumsFromImapServer.Find(pSum) == NULL)
						SumsFromLocalSearch.erase(SI++);
					else
						++SI;
				}
			}
			
			//remove elements so that destructor won't call CSummary destructor
			SumsFromImapServer.RemoveAll(); 

			for(CSumIter SI = SumsFromLocalSearch.begin(); SI != SumsFromLocalSearch.end(); ++SI)
			{
				ResultsList.push_back( SearchResult(*SI));
				nResults++;
			}
		
		}

		//if any resluts left over due to frequency or Imap server search, report them NOW
		if(!m_bStopRequested && nResults && ReportResults)
			ReportResults(ResultsList);

		//ready for next results
		ResultsList.clear();

		//compute the final Imap skip count
		if(pToc->IsImapToc())
		{
			if(m_pMSC->IsOpOR())
				m_nImapSkipCount +=  max(ImapResult.GetMatchedUnknownMsgs(), ImapResult.GetUnknownMsgs());
			else
				m_nImapSkipCount +=  min(ImapResult.GetMatchedUnknownMsgs(), ImapResult.GetUnknownMsgs());
			
			if(bImapFullLocalSearch)
				m_nImapSkipCount += ImapResult.GetKnownUnfetchedMsgs();
		}



		//delete the doc if we loaded it
		if(bLoadedToc)
			pToc->OnCloseDocument();

	} //for each mailbox

#ifdef _DEBUG
	{
		char buf[512];
		sprintf(buf, "Total %d Exta Time spent %d, Time for HitSums %d # of saves %d\n", 
				GetTickCount() - nStart, nTotalTime, nStartHits, nHits);
		OutputDebugString(buf);
		sprintf(buf, "%d Msgs opened. %d msgs are HTML\n", nOpenedMsgs, nHTMLMsgs);
		OutputDebugString(buf);
	}
#endif

	return (true);
}



bool SearchEngine::NeedToOpenMessageForRefinement(bool bFoundSummary, SearchCriteria& Sct, bool *pbFinalMatch)
{

	CriteriaVerbTextCompare verb = Sct.GetVerbTextCmp();
	CriteriaObject Criteria = Sct.GetCriteria();
	
	*pbFinalMatch = false; //means not matched

	bool bRefinement = false;
	bool bMatched = false;

	if( !bFoundSummary) // no substring
	{
		
		switch(verb)
		{
		case CRITERIA_TEXT_COMPARE_CONTAINS:
		case CRITERIA_TEXT_COMPARE_CONTAINSWORD:
		case CRITERIA_TEXT_COMPARE_IS:
		case CRITERIA_TEXT_COMPARE_STARTSWITH:
		case CRITERIA_TEXT_COMPARE_ENDSWITH:
		case CRITERIA_TEXT_COMPARE_MATCHESREGEXP:
		case CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE:
			bMatched = false;
			break;
		case CRITERIA_TEXT_COMPARE_ISNOT:
		case CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN:
			bMatched = true;
			break;
		default:
			ASSERT(0);
			break;
		}
		
	}
	else //matched with the search string
	{
		switch(verb)
		{
		case CRITERIA_TEXT_COMPARE_CONTAINS:
		case CRITERIA_TEXT_COMPARE_MATCHESREGEXP:
		case CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE:
			bMatched = true;
			if(Criteria != CRITERIA_OBJECT_ANYWHERE) //need to know where exactly is the string
				bRefinement = true;
			break;
		
		case CRITERIA_TEXT_COMPARE_CONTAINSWORD:
		case CRITERIA_TEXT_COMPARE_IS:
		case CRITERIA_TEXT_COMPARE_STARTSWITH:
		case CRITERIA_TEXT_COMPARE_ENDSWITH:
			bMatched = true;
			bRefinement = true;
			break;
		case CRITERIA_TEXT_COMPARE_ISNOT:
		case CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN:
			bMatched = false;
			bRefinement = true;  //how about CC
			break;
		default:
			ASSERT(0);
			break;
		}
		
	}

	//if not refining the search by opeing the msg, set the final result
	if(!bRefinement)
		*pbFinalMatch = bMatched;


	return bRefinement;
}



const int FINDBUFSIZE = 1024*64;

class SumLessThan 
{
public:
	bool operator ()(CSummary *Sum1, CSummary *Sum2)
	{
		return (Sum1->m_Offset < Sum2->m_Offset)?true:false;
	}
};


bool SearchEngine::FindPossibleHitSummaries(MultSearchCriteria& MSC, const char *szFileName, set<CSummary *>& HitSums)
{
	set<long> Offsets;

	if(!FindStringInFile(MSC, szFileName, Offsets))
		return false;

	if(::EscapePressed())
	{
		m_bStopRequested = true;
		return false;
	}

	CTocDoc *pToc = GetToc(szFileName);
	if(!pToc)
		return false;
	
	CSumList& SumList = pToc->m_Sums;
	int Count = SumList.GetCount();
	if( Count == 0)
		return false;
	
	POSITION pos = SumList.GetHeadPosition();
	
	set<CSummary *, SumLessThan> SumSet;
	
	pos = SumList.GetHeadPosition();
	for (int i = 0; i < Count; i++)
		SumSet.insert( SumList.GetNext(pos) );
	
		
	set<CSummary*, SumLessThan>::iterator SumIter = SumSet.begin();
	CSummary *pSum = *SumIter;
	
	long nLowOffset;
	long nHighOffset;

	for(set<long>::iterator si = Offsets.begin(); si != Offsets.end(); ++si)
	{
		long nFileOffset = *si;
		
		for( ; SumIter != SumSet.end(); ++SumIter) //while the offset finds a sum or end of the TOC
		{
			pSum = *SumIter;
			nLowOffset = pSum->m_Offset;
			nHighOffset = nLowOffset + pSum->m_Length;

			if( nFileOffset < nLowOffset)
				break;
		
			if( nFileOffset >= nLowOffset && nFileOffset <= nHighOffset)
			{
				HitSums.insert(pSum);
				break;
			}
			
		}

		if(SumIter == SumSet.end())
			break;
	}

	if(HitSums.size() > 0)
		return true;
	else
		return false;
}
		

		


bool SearchEngine::FindStringInFile(MultSearchCriteria& MSC, const char *szFileName, set<long>& Offsets)
{

	CFile MBXFile;
	CFileException fexcept;
	CRString strProgress(IDS_SEARCH_INDEXING);
	strProgress += szFileName;

	Progress(strProgress);

	try{
	
		if( !MBXFile.Open(szFileName, CFile::modeRead, &fexcept))
		{
			//created
			//these error are expected ENOENT or directory non-existant ESRCH
			if(!(fexcept.m_lOsError == ENOENT || fexcept.m_lOsError == ESRCH))
			{
				ASSERT(0);  //if none of above errors the what is it??
			}
			return false;
		}


		int nFindLen = 256;
		int nRead = 0, nFilePosition = 0;
		char szBuffer[FINDBUFSIZE+1] = {0};
		char *ptr = NULL;
		const SearchCriteriaList *pList = MSC.GetCriteriaList();
		CString strFindMe;

		while (nRead = MBXFile.Read(szBuffer, FINDBUFSIZE))
		{
			if(::EscapePressed())
			{
				m_bStopRequested = true;
				return false;
			}

			szBuffer[nRead] = '\0'; //make it a string
			
			//for each search criteria, compute offsets and save em in a set
			for(SearchCriteriaIter iter = pList->begin(); iter != pList->end(); ++iter)
			{
				ptr = szBuffer;
				SearchCriteria Sct = *iter;
				if(!Sct.NeedFullMessage())
					continue;
				
				bool bUsingRegex = (NULL != Sct.GetRegex());
				
				if(!bUsingRegex)
				{
					strlwr(szBuffer);
					strFindMe = Sct.GetValueText();
				
					while(*ptr && (ptr = strstr(ptr, strFindMe)) )
					{
						Offsets.insert(nFilePosition + ptr - szBuffer);
						ptr = ptr+nFindLen;
					}
				}
				else
				{
					CRegex *pRegex = Sct.GetRegex();
					if(pRegex == NULL)
					{
						ASSERT(0);
						continue;
					}

					char *line = strtok(szBuffer, "\r\n");
					if(pRegex->Match(line))
						Offsets.insert(nFilePosition + line - szBuffer);

					while( line != NULL)
					{
						line = strtok(NULL, "\r\n");
						if( pRegex->Match(line))
							Offsets.insert(nFilePosition + line - szBuffer);
					}
				}

				
			}
			
			if(nRead == FINDBUFSIZE)
				MBXFile.Seek(-nFindLen, CFile::current);

			nFilePosition = MBXFile.GetPosition();
		}
		MBXFile.Close();
		
		if(Offsets.size() > 0)
			return true;

	}
	
	catch(CFileException* fe)
	{	
		TRACE("Search: Failed loading MBX file \"%s\",  Cause: \"%d\"\n", fe->m_strFileName, fe->m_lOsError);
	    ASSERT(FALSE);
		fe->Delete();
	    return false; 
	}

	
	return false;
}




CTocDoc *GetTocWithFlags(const char *szPath, bool *pLoaded)
{
	CTocDoc *pTocDoc = NULL;
	*pLoaded = false;
	
	// First try to get the TOC from memory (this means it's open)
	if (!(pTocDoc = GetToc(szPath, NULL, FALSE, TRUE)))
	{
		// Okay, so it either failed badly or it's not in memory
		// Try again, but allow it to be loaded (this means it's not open)
		if (!(pTocDoc = GetToc(szPath))) // try not in mem
		{
			// Both calls failed, so something is really wrong
			return NULL; // returns mild error
		}
		else
		{
			// We got the TOC, but it wasn't in memory, so be sure to close it
			//  when we are done.
			*pLoaded = true;
		}
	}
	return pTocDoc;
}


int GetAttachments(CSummary *pSum, char *szMessage/* = NULL*/, list<CString> *pAttachStrList /*= NULL*/, bool bFullPath /* = false */)
{
	ASSERT(pSum);

	int nAttachments = 0;
	
	if ((pSum) && (pSum->HasAttachment()))
	{
		if (pSum->IsComp()) // outgoing message
		{
			ASSERT(szMessage);

			if (szMessage)
			{
				// get x-attachments header
				char *pHeaderStr = HeaderContents(IDS_HEADER_ATTACHMENTS, szMessage);

				if (pHeaderStr)
				{
					nAttachments = (int) ChopList(pHeaderStr, ';', pAttachStrList);
					delete [] pHeaderStr;
				}
			}
		}
		else // read msg
		{
			CMessageDoc *pDoc = (CMessageDoc *)pSum->FindMessageDoc();
			ASSERT(pDoc);

			if (pDoc)
			{
				CString strAttach;
				pDoc->m_QCMessage.GetAttachments(strAttach);

				nAttachments = (int) ChopList(strAttach, ';', pAttachStrList);
			}
		}
	}

	// Check if we have to remove the full path from the file names
	if ((pAttachStrList) && (!bFullPath))
	{
		CString tmpStr;
		char *cp = NULL;

		for (list<CString>::iterator itr = pAttachStrList->begin(); itr != pAttachStrList->end(); itr++)
		{
			if (cp = strrchr((*itr), '\\'))
			{
				tmpStr = (cp + 1);
				(*itr) = tmpStr;
			}
		}
	}

	return nAttachments;
}

// --------------------------------------------------------------------------

// Chop up the a list into separate strings
unsigned int ChopList(LPCSTR pStr, char cDelim /* = ';' */, list<CString> *pStrList /* = NULL */)
{
	// S.Manjourides, 2/18/99

	// This is probabaly already somewhere else, but it's so much fun
	// to reinvent the wheel.

	ASSERT(pStr);

	if (NULL == pStr)
		return (0);

	unsigned int nStrCount = 0;
	char *pBeg = NULL, *pEnd = NULL, *pLastNonWS = NULL, *pNextBeg = (LPSTR) pStr;

	while (pNextBeg)
	{
		pBeg = pNextBeg;

		// Skip white space
		while (::isspace(*pBeg))
			pBeg++;

		pLastNonWS = pEnd = pBeg;

		// Move end to next semicolon or end of string
		while ((*pEnd) && (*pEnd != cDelim))
		{
			// Keeping track of last non-whitespace
			if (!(::isspace(*pEnd)))
				pLastNonWS = pEnd;

			pEnd++;
		}
		
		if (*pEnd)
			pNextBeg = pEnd + 1;
		else
			pNextBeg = NULL; // We're done

		// Skip back over the spearator -- it's not in the string
		pEnd--;

		// Ignore trailing whitespace -- only move back
		if (pEnd > pLastNonWS)
			pEnd = pLastNonWS;

		// Check for quotes
		if ((pEnd > pBeg) && ('\"' == *pEnd) && ('\"' == *pBeg))
		{
			// Remove quotes on both sides
			pBeg++;
			pEnd--;
		}

		// In the case where pEnd < pBeg, there is no valid string
		// Otherwise, pBeg is the first char, pEnd the last

		if (pEnd >= pBeg) // We have a valid attachment name
		{
			nStrCount++; // Inc count

			if(pStrList)
			{
				const int nLen = (pEnd - pBeg) + 1;
				CString str(static_cast<LPCTSTR>(pBeg), nLen);

				// Add string to list
				pStrList->push_back(str);
			}
		}
	}

#ifdef _DEBUG
	if (pStrList)
		ASSERT(pStrList->size() == nStrCount);
#endif // _DEBUG

	return (nStrCount);
}

// --------------------------------------------------------------------------


//#define QCSTR_SUBSTR_SEARCH    (0)
//#define QCSTR_WORD_SEARCH      (1)
//#define QCSTR_BEGWORD_SEARCH   (2)
//#define QCSTR_ENDWORD_SEARCH   (3)

char *QCStrstr(const char * str1,  const char * str2, 
				   bool bMatchCase /*= QCSTR_CASE_SENSITIVE*/, int nWholeWord /* = QCSTR_SUBSTR_SEARCH */)
{
       
	if (bMatchCase && (QCSTR_SUBSTR_SEARCH == nWholeWord))
		return strstr(str1, str2);
	else
	{
        if ( !*str2 )
            return((char *)str1);

		char *cp = (char *) str1;
        char *s1, *s2;
		
		while (*cp)
        {
            s1 = cp;
            s2 = (char *) str2;

            while ( *s1 && *s2 && !((bMatchCase?*s1:tolower(*s1)) - (bMatchCase?*s2:tolower(*s2))) )
				s1++, s2++;
			
            if (!*s2)
			{
				switch (nWholeWord)
				{
					case QCSTR_WORD_SEARCH:
					{
						const unsigned char bChar = (cp == str1)? ' ': *(cp-1);
						const unsigned char eChar = *(cp+strlen(str2));

						if(!isalnum(bChar) && !isalnum(eChar))
							return cp;
					}
					break;

					case QCSTR_BEGWORD_SEARCH:
					{
						const unsigned char bChar = (cp == str1)? ' ': *(cp-1);

						if (!isalnum(bChar))
							return cp;
					}
					break;

					case QCSTR_ENDWORD_SEARCH:
					{
						const unsigned char eChar = *(cp+strlen(str2));

						if (!isalnum(eChar))
							return cp;
					}
					break;
					case QCSTR_SUBSTR_SEARCH:
					{
						return cp;
					}
					break;

					default:
						ASSERT(0);
				}
			}

            cp++;
        }

        return(NULL);
	}
}

