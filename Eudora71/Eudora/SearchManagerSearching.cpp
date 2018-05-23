//	SearchManagerSearching.cpp
//
//	Search Manager searching methods.
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
#include "SearchManagerSearching.h"
#include "SearchManagerInfo.h"
#include "SearchManagerUtils.h"

#include "persona.h"
#include "resource.h"
#include "SearchCriteria.h"
#include "summary.h"

//	X1 Include
#include "X1Wrapper.h"

#include <deque>

#include "DebugNewHelpers.h"


// ---------------------------------------------------------------------------
//		* GetX1FindType												 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearchTerm to get the X1 find type to use.

void
SearchManager::Searching::GetX1FindType(
	const SearchCriteria &		in_searchCriteria,
	X1::X1FindType &			out_x1FindType,
	SearchTypeEnum &			out_eSearchType,
	bool &						out_bDoInversion,
	bool &						out_bDoSearchTerm)
{
	CriteriaObject			eCriteriaWhere = in_searchCriteria.GetCriteria();
	CriteriaVerbType		criteriaVerbType = SearchCriteria::GetVerbType(eCriteriaWhere);

	switch (criteriaVerbType)
	{
		case CRITERIA_VERB_TEXT_COMPARE_TYPE:
			{
				//	Default to text search
				out_eSearchType = st_Text;
				
				CriteriaVerbTextCompare		criteriaVerb = in_searchCriteria.GetVerbTextCmp();

				switch(criteriaVerb)
				{
					case CRITERIA_TEXT_COMPARE_CONTAINS:
						out_x1FindType = X1::ftContainsPrefix;
						break;
				
					case CRITERIA_TEXT_COMPARE_CONTAINSWORD:
						out_x1FindType = X1::ftContainsExact;
						break;

					case CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN:
						out_x1FindType = X1::ftContainsPrefix;
						out_bDoInversion = true;
						break;

					case CRITERIA_TEXT_COMPARE_MATCHESX1QUERY:
						out_eSearchType = st_X1Query;
						break;
					
					case CRITERIA_TEXT_COMPARE_IS:
					case CRITERIA_TEXT_COMPARE_ISNOT:
					case CRITERIA_TEXT_COMPARE_STARTSWITH:
					case CRITERIA_TEXT_COMPARE_ENDSWITH:
					case CRITERIA_TEXT_COMPARE_MATCHESREGEXP_ICASE:
					case CRITERIA_TEXT_COMPARE_MATCHESREGEXP:
						ASSERT( !"Search verb needs to be removed/disabled with indexed search");
						out_bDoSearchTerm = false;
						break;

					default:
						ASSERT( !"Unknown search verb");
						out_bDoSearchTerm = false;
						break;
				}
			}
			break;

		case CRITERIA_VERB_EQUAL_COMPARE_TYPE:
			{
				//	Text search
				out_eSearchType = st_Text;
				
				CriteriaVerbEqualCompare		criteriaVerb = in_searchCriteria.GetVerbEqualCmp();

				switch (criteriaVerb)
				{
					case CRITERIA_EQUAL_COMPARE_IS:
						out_x1FindType = X1::ftContainsExact;
						break;

					case CRITERIA_EQUAL_COMPARE_ISNOT:
						out_x1FindType = X1::ftContainsExact;
						out_bDoInversion = true;
						break;
				}
			}
			break;

		case CRITERIA_VERB_NUM_COMPARE_TYPE:
			{
				//	Numeric search
				out_eSearchType = st_Numeric;
				
				CriteriaVerbNumCompare		criteriaVerb = in_searchCriteria.GetVerbNumCmp();

				switch (criteriaVerb)
				{
					case CRITERIA_NUM_COMPARE_IS:
						out_x1FindType = X1::ftContainsExact;
						break;

					case CRITERIA_NUM_COMPARE_ISNOT:
						out_x1FindType = X1::ftContainsExact;
						out_bDoInversion = true;
						break;

					case CRITERIA_NUM_COMPARE_ISGREATERTHAN:
						out_x1FindType = X1::ftContainsPrefix;

						//	Our default searching for numbers yields "is greater than",
						//	therefore most search fields do not invert the search results
						//	here. Priority and age are the exceptions need inverted.
						//
						//	Priority is opposite because the number scheme is reversed
						//	of what users expect. "Highest" has the lowest numeric value
						//	and "Lowest" has the highest numeric value.
						//
						//	Age is opposite because searching for messages whose age
						//	"is greater than" a given number means messages whose date
						//	is less than the date that corresponds to the specified age.
						out_bDoInversion = (eCriteriaWhere == CRITERIA_OBJECT_PRIORITY) ||
										   (eCriteriaWhere == CRITERIA_OBJECT_AGE);
						break;

					case CRITERIA_NUM_COMPARE_ISLESSTHAN:
						out_x1FindType = X1::ftContainsPrefix;

						//	Our default searching for numbers yields "is greater than",
						//	therefore most search fields need to negate the search
						//	results here to get "is less than". Priority and age are
						//	the exceptions and do not need the inversion
						//
						//	Priority is opposite because the number scheme is reversed
						//	of what users expect. "Highest" has the lowest numeric value
						//	and "Lowest" has the highest numeric value.
						//
						//	Age is opposite because searching for messages whose age
						//	"is less than" a given number means messages whose date is
						//	greater than the date that corresponds to the specified age.
						out_bDoInversion = (eCriteriaWhere != CRITERIA_OBJECT_PRIORITY) &&
										   (eCriteriaWhere != CRITERIA_OBJECT_AGE);
						break;
				}
			}
			break;


		case CRITERIA_VERB_DATE_COMPARE_TYPE:
			{
				//	Numeric search
				out_eSearchType = st_Numeric;
				
				CriteriaVerbDateCompare		criteriaVerb = in_searchCriteria.GetVerbDateCmp();

				switch (criteriaVerb)
				{
					case CRITERIA_DATE_COMPARE_IS:
						out_x1FindType = X1::ftContainsExact;
						break;

					case CRITERIA_DATE_COMPARE_ISNOT:
						out_x1FindType = X1::ftContainsExact;
						out_bDoInversion = true;
						break;

					case CRITERIA_DATE_COMPARE_ISAFTER:
						out_x1FindType = X1::ftContainsPrefix;
						break;

					case CRITERIA_DATE_COMPARE_ISBEFORE:
						out_x1FindType = X1::ftContainsPrefix;
						out_bDoInversion = true;
						break;
				}
			}
			break;
	}
}


// ---------------------------------------------------------------------------
//		* GetX1SearchFieldAndNumericValue							 [Public]
// ---------------------------------------------------------------------------
//	Called by DoTextSearchTerm to get the X1 fields to search and the text
//	for which to search.

void
SearchManager::Searching::GetX1SearchFieldAndNumericValue(
	const SearchCriteria &		in_searchCriteria,
	long &						out_nX1Field,
	long &						out_nValue,
	long &						out_nMaxValue,
	RomanNumeralsEnum &			out_eLastDigit)
{
	CriteriaObject		eCriteriaWhere = in_searchCriteria.GetCriteria();

	switch (eCriteriaWhere)
	{
		case CRITERIA_OBJECT_PRIORITY:
			{
				out_nX1Field = efid_PriorityRomanNumeral;
				
				CriteriaValuePriority		priority = in_searchCriteria.GetValuePriority();

				switch (priority)
				{
					case CRITERIA_VALUE_PRIORITY_HIGHEST:
						out_nValue = MSP_HIGHEST;
						break;

					case CRITERIA_VALUE_PRIORITY_HIGH:
						out_nValue = MSP_HIGH;
						break;

					case CRITERIA_VALUE_PRIORITY_NORMAL:
						out_nValue = MSP_NORMAL;
						break;

					case CRITERIA_VALUE_PRIORITY_LOW:
						out_nValue = MSP_LOW;
						break;

					case CRITERIA_VALUE_PRIORITY_LOWEST:
						out_nValue = MSP_LOWEST;
						break;

				}

				out_nMaxValue = MSP_LOWEST;
				out_eLastDigit = rn_LastDigitIsOne;
			}
			break;

		case CRITERIA_OBJECT_JUNKSCORE:
			{
				out_nX1Field = efid_JunkScoreRomanNumeral;
				
				CriteriaValueJunkScore		nJunkScore = in_searchCriteria.GetValueJunkScore();

				out_nValue = nJunkScore;
				out_nMaxValue = kMaxPossibleJunkScore;
				out_eLastDigit = rn_LastDigitIsTen;
			}
			break;

		case CRITERIA_OBJECT_DATE:
		case CRITERIA_OBJECT_AGE:
			{
				out_nX1Field = efid_DateDaysRomanNumeral;
				
				COleDateTime		date;
				
				if (eCriteriaWhere == CRITERIA_OBJECT_DATE)
				{
					//	Get the date that the user specified
					date = in_searchCriteria.GetValueDate();
				}
				else	//	eCriteriaWhere == CRITERIA_OBJECT_AGE
				{
					//	Get the current date. Below after we convert it to a
					//	number of days, we'll adjust for the age the user wanted.
					date = COleDateTime::GetCurrentTime();

					date.SetDate( date.GetYear(), date.GetMonth(), date.GetDay() );
				}

				//	Convert CriteriaValueDate (aka COleDateTime) to CTime
				SYSTEMTIME			systemTime;
				
				if ( date.GetAsSystemTime(systemTime) )
				{
					//	Convert system time to timespan since 1970
					CTime				time(systemTime);
					CTimeSpan			timeSpan( time.GetTime() );

					//	Get number of days since 1970
					out_nValue = static_cast<long>( timeSpan.GetDays() );

					if (eCriteriaWhere == CRITERIA_OBJECT_AGE)
					{
						//	If the user specified an age search, adjust the
						//	number of days by the age in days.
						out_nValue -= in_searchCriteria.GetValueAge();
					}
				}
				else
				{
					ASSERT(!"Error getting date as system time");
					out_nValue = 0;
				}
				out_nMaxValue = kRomanNumeralMaxPossibleValue;
				out_eLastDigit = rn_LastDigitIsThousand;
			}
			break;

		case CRITERIA_OBJECT_ATTACHCOUNT:
			{
				out_nX1Field = efid_AttachmentCountRomanNumeral;
				
				CriteriaValueAttachCount	nAttachCount = in_searchCriteria.GetValueAttachCount();

				out_nValue = nAttachCount;
				out_nMaxValue = kMaxNumAttachments;
				out_eLastDigit = rn_LastDigitIsTen;
			}
			break;

		case CRITERIA_OBJECT_SIZE:
			{
				out_nX1Field = efid_SizeRomanNumeral;

				CriteriaValueSize		nSize = in_searchCriteria.GetValueSize();

				out_nValue = nSize;
				out_nMaxValue = kRomanNumeralMaxPossibleValue;
				out_eLastDigit = rn_LastDigitIsThousand;
			}
			break;
	}
}


// ---------------------------------------------------------------------------
//		* GetX1SearchFieldsAndText									 [Public]
// ---------------------------------------------------------------------------
//	Called by DoTextSearchTerm to get the X1 fields to search and the text
//	for which to search.

void
SearchManager::Searching::GetX1SearchFieldsAndText(
	const SearchCriteria &		in_searchCriteria,
	std::deque<long> &			out_queueX1Fields,
	CString &					out_szSearchText,
	bool &						out_bDoSearchTerm)
{
	CriteriaObject		eCriteriaWhere = in_searchCriteria.GetCriteria();
	CriteriaVerbType	criteriaVerbType = SearchCriteria::GetVerbType(eCriteriaWhere);

	//	Default to doing the search term
	out_bDoSearchTerm = true;

	if (criteriaVerbType == CRITERIA_VERB_TEXT_COMPARE_TYPE)
	{
		switch (eCriteriaWhere)
		{
			case CRITERIA_OBJECT_ANYWHERE:
				//	Anywhere = All Headers + Body, but we've set it up so that these
				//	two fields are the only ones that are searched by a default search.
				//	Therefore we just specify -1 (which means search default fields).
				out_queueX1Fields.push_back(-1);
				break;

			case CRITERIA_OBJECT_HEADERS:
				out_queueX1Fields.push_back(efid_AllHeaders);
				break;

			case CRITERIA_OBJECT_BODY:
				out_queueX1Fields.push_back(efid_Body);
				break;

			case CRITERIA_OBJECT_ATTACHNAMES:
				out_queueX1Fields.push_back(efid_Attachments);
				break;

			case CRITERIA_OBJECT_SUMMARY:
				//	Summary = From + Subject
				out_queueX1Fields.push_back(efid_Who);
				out_queueX1Fields.push_back(efid_Subject);
				break;

			case CRITERIA_OBJECT_TO:
				out_queueX1Fields.push_back(efid_ToHeader);
				break;

			case CRITERIA_OBJECT_FROM:
				out_queueX1Fields.push_back(efid_FromHeader);
				break;

			case CRITERIA_OBJECT_SUBJECT:
				out_queueX1Fields.push_back(efid_Subject);
				break;

			case CRITERIA_OBJECT_CC:
				out_queueX1Fields.push_back(efid_CcHeader);
				break;

			case CRITERIA_OBJECT_BCC:
				out_queueX1Fields.push_back(efid_BccHeader);
				break;

			case CRITERIA_OBJECT_ANYRECIP:
				//	Any Recipient = To + Cc + Bcc
				out_queueX1Fields.push_back(efid_ToHeader);
				out_queueX1Fields.push_back(efid_CcHeader);
				out_queueX1Fields.push_back(efid_BccHeader);
				break;

			case CRITERIA_OBJECT_MAILBOXNAME:
				out_queueX1Fields.push_back(efid_MailboxName);
				break;

			default:
				ASSERT( !"Unknown search location." );
				out_bDoSearchTerm = false;
				break;
		}

		//	Get the text value to search for
		out_szSearchText = in_searchCriteria.GetValueText();
	}
	else
	{
		switch (eCriteriaWhere)
		{
			case CRITERIA_OBJECT_STATUS:
				{
					out_queueX1Fields.push_back(efid_State);
					
					CriteriaValueStatus		status = in_searchCriteria.GetValueStatus();

					switch (status)
					{
						case CRITERIA_VALUE_STATUS_UNREAD:
							out_szSearchText.LoadString(IDS_STATUS_UNREAD);
							break;

						case CRITERIA_VALUE_STATUS_READ:
							out_szSearchText.LoadString(IDS_STATUS_READ);
							break;

						case CRITERIA_VALUE_STATUS_REPLIED:
							out_szSearchText.LoadString(IDS_STATUS_REPLIED);
							break;

						case CRITERIA_VALUE_STATUS_FORWARDED:
							out_szSearchText.LoadString(IDS_STATUS_FORWARDED);
							break;

						case CRITERIA_VALUE_STATUS_REDIRECTED:
							out_szSearchText.LoadString(IDS_STATUS_REDIRECTED);
							break;

						case CRITERIA_VALUE_STATUS_SENT:
							out_szSearchText.LoadString(IDS_STATUS_SENT);
							break;

						case CRITERIA_VALUE_STATUS_SENDABLE:
							out_szSearchText.LoadString(IDS_STATUS_SENDABLE);
							break;

						case CRITERIA_VALUE_STATUS_UNSENT:
							out_szSearchText.LoadString(IDS_STATUS_UNSENT);
							break;

						case CRITERIA_VALUE_STATUS_QUEUED:
							out_szSearchText.LoadString(IDS_STATUS_QUEUED);
							break;

						case CRITERIA_VALUE_STATUS_TIME_QUEUED:
							out_szSearchText.LoadString(IDS_STATUS_TIME_QUEUED);
							break;

						case CRITERIA_VALUE_STATUS_UNSENDABLE:
							out_szSearchText.LoadString(IDS_STATUS_UNSENDABLE);
							break;

						case CRITERIA_VALUE_STATUS_RECOVERED:
							out_szSearchText.LoadString(IDS_STATUS_RECOVERED);
							break;
					}
				}
				break;

			case CRITERIA_OBJECT_LABEL:
				{
					out_queueX1Fields.push_back(efid_Label);

					CriteriaValueLabel		nLabel = in_searchCriteria.GetValueLabel();
					out_szSearchText.Format("%d", nLabel);
				}
				break;

			case CRITERIA_OBJECT_PERSONA:
				{
					out_queueX1Fields.push_back(efid_PersonaHash);

					CriteriaValuePersona	szPersona = in_searchCriteria.GetValuePersona();
					DWORD					nPersonaHash = g_Personalities.GetHash(szPersona);
					out_szSearchText.Format("%d", nPersonaHash);
				}
				break;

		}
	}
}


// ---------------------------------------------------------------------------
//		* DoNumericSearchTerm										 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearchTerm to search for a specific numeric term.
//
//	Roman numeral searching scheme:
//	Get number of M's, C's, X's, and I's for number.
//	To search for all numbers greater than 4329 we search for:
//	MMMM && CCC && XX && IIIIIIIIII OR
//	MMMM && CCC && XXX OR
//	MMMM && CCCC OR
//	MMMMM
//
//	> 50 => >= 51
//	< 50 => ! >=50

void
SearchManager::Searching::DoNumericSearchTerm(
	const SearchCriteria &		in_searchCriteria,
	X1::X1FindType				in_x1FindType,
	bool						in_bDoInversion,
	X1::IX1ItemResult **		out_ppX1ItemResult)
{
	try
	{
		long					nX1Field;
		long					nValue;
		long					nMaxValue;
		RomanNumeralsEnum		eLastDigit;
		
		GetX1SearchFieldAndNumericValue(in_searchCriteria, nX1Field, nValue, nMaxValue, eLastDigit);

		if ( (in_x1FindType == X1::ftContainsPrefix) && !in_bDoInversion )
		{
			//	Contains prefix, without doing inversion means that we're searching
			//	for >. Doing our prefix style searches we will actually find >=.
			//	Therefore we need to bump the number up by one.
			nValue++;

			//	When searching at the limit of our range, we actually want to be able to
			//	exceed our max value by one (for example searching for a junk score > 100
			//	means searching for junk scores >= 101). We don't expect such searches to
			//	yield any results, but X1 doesn't seem to provide any way to just get an
			//	empty result set and just allowing such searches simplifies things.
			//
			//	We need to increase the max value, because otherwise our other sanity
			//	checking code would "fix" the value by setting it back to the max.
			nMaxValue++;
		}

		//	Calculate how many of each roman numeral we need
		long		nThousands = 0;
		long		nHundreds = 0;
		long		nTens = 0;
		long		nOnes = 0;

		SearchManager::Utils::CalculateNumRomanNumeralDigits( nValue, nMaxValue, eLastDigit,
															  nThousands, nHundreds, nTens, nOnes );

		CString					szRomanNumeral;
		_bstr_t					bstrSearchText;
		X1::IX1DatabasePtr		pEmailDB = SearchManager::Instance()->m_pInfo->m_pX1EmailDB;

		//	Do the initial search for the actual roman numeral value.
		//	Encode the roman numeral value with appended ='s to indicate
		//	that we want each of the exact strings.
		SearchManager::Utils::EncodeRomanNumeralDigits(nThousands, nHundreds, nTens, nOnes, szRomanNumeral);

		bstrSearchText = szRomanNumeral;

		if (in_x1FindType == X1::ftContainsExact)
		{
			//	Search for the exact roman numeral string
			pEmailDB->FindItems(in_x1FindType, bstrSearchText, nX1Field, out_ppX1ItemResult);
		}
		else
		{
			//	Search for the roman numeral string. We use the search object to allow
			//	X1 to better cache our search results. X1 will translate our query into
			//	the AND of several searches. When we repeatedly search for some of the
			//	larger roman numerals, X1 will find the previous results in its cache.
			X1::IX1SearchPtr		pSearch = SearchManager::Instance()->GetX1Search();

			pSearch->Reset(pEmailDB);
			pSearch->AddTerms(bstrSearchText, nX1Field);
			pSearch->Perform(out_ppX1ItemResult);
			
			if (eLastDigit != rn_LastDigitIsOne)
			{
				//	We're doing a prefix search with more than one digit.
				long *						pCurrentDigit = NULL;
				X1::IX1ItemResultPtr		pCombinedResult;
				X1::IX1ItemResultPtr		pCurrentResult;
				long						nCurrentDigit = rn_LastDigitIsTen;
				long						nLastDigit = eLastDigit;

				do {
					//	What's the current digit
					switch (nCurrentDigit)
					{
						case rn_LastDigitIsTen:
							pCurrentDigit = &nTens;
							break;

						case rn_LastDigitIsHundred:
							pCurrentDigit = &nHundreds;
							break;

						case rn_LastDigitIsThousand:
							pCurrentDigit = &nThousands;
							break;
					}

					ASSERT(pCurrentDigit);

					//	Increment the current digit
					(*pCurrentDigit)++;

					//	Do we need to search for the number with this digit?
					//
					//	If it's the last digit the answer is yes no matter what.
					//
					//	If it's not the last digit, then we only need to search if
					//	it's < 10, because we would have never encoded 10 or more
					//	characters in a row.
					if ( (nCurrentDigit == nLastDigit) || (*pCurrentDigit < 10) )
					{
						//	Attach to the results up to this point without adding to the ref count
						//	by passing false in the second parameter. (FindItems already returns it
						//	with a ref count of 1, if we allowed the ref count to be incremented we
						//	would leak).
						pCombinedResult.Attach(*out_ppX1ItemResult, false);				
						
						//	Encode the current roman numeral
						SearchManager::Utils::EncodeRomanNumeralDigits(nThousands, nHundreds, nTens, 0, szRomanNumeral);

						//	Convert the string for searching
						bstrSearchText = szRomanNumeral;

						//	Search for the roman numeral string
						pSearch->Reset(pEmailDB);
						pSearch->AddTerms(bstrSearchText, nX1Field);
						pSearch->Perform(&pCurrentResult);

						//	Must NULL it in order for X1 to give us results
						*out_ppX1ItemResult = NULL;

						//	Combine the new result
						pCurrentResult->OpOr(pCombinedResult, out_ppX1ItemResult);
					}

					//	Next time through the loop we won't care about this digit anymore
					*pCurrentDigit = 0;

					//	Move to the next digit
					nCurrentDigit++;
				} while(nCurrentDigit <= nLastDigit);
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "DoNumericSearchTerm", e.ErrorMessage() );

		//	Re-throw the exception for DoSearchTerm to catch
		throw;
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "DoNumericSearchTerm");

		//	Re-throw the exception for DoSearchTerm to catch - don't delete
		//	the pException object, because DoSearchTerm will need it.
		throw;
	}
}


// ---------------------------------------------------------------------------
//		* DoTextSearchTerm											 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearchTerm to search for a specific text term.

void
SearchManager::Searching::DoTextSearchTerm(
	const SearchCriteria &		in_searchCriteria,
	SearchTypeEnum				in_eSearchType,
	X1::X1FindType				in_x1FindType,
	X1::IX1ItemResult **		out_ppX1ItemResult)
{
	try
	{
		bool				bDoSearchTerm = true;
		CString				szSearchText;
		std::deque<long>	queueX1Fields;

		GetX1SearchFieldsAndText(in_searchCriteria, queueX1Fields, szSearchText, bDoSearchTerm);

		if (bDoSearchTerm)
		{
			_bstr_t							bstrSearchText(szSearchText);
			X1::IX1DatabasePtr				pEmailDB = SearchManager::Instance()->m_pInfo->m_pX1EmailDB;
			X1::IX1SearchPtr				pSearch = SearchManager::Instance()->GetX1Search();
			X1::IX1ItemResultPtr			pCombinedResult;
			X1::IX1ItemResultPtr			pCurrentResult;
			std::deque<long>::iterator		x1FieldsIterator = queueX1Fields.begin();
			long							nX1Field;

			if ( !queueX1Fields.empty() )
			{
				//	Get the first field
				nX1Field = queueX1Fields.front();
				queueX1Fields.pop_front();

				//	Do the first field
				if (in_eSearchType == st_X1Query)
				{
					//	When doing an X1 query, we call the X1 "higher" level API that will
					//	interpret X1's query language.
					pSearch->Reset(pEmailDB);
					pSearch->AddTerms(bstrSearchText, nX1Field);
					pSearch->Perform(out_ppX1ItemResult);
				}
				else
				{
					//	When doing anything other than an X1 query, we call the X1 "lower"
					//	level API that will just search exactly as we tell it to without
					//	interpreting the search text.
					pEmailDB->FindItems(in_x1FindType, bstrSearchText, nX1Field, out_ppX1ItemResult);
				}
			}

			while ( !queueX1Fields.empty() )
			{
				//	Attach to the results up to this point without adding to the ref count
				//	by passing false in the second parameter. (FindItems already returns it
				//	with a ref count of 1, if we allowed the ref count to be incremented we
				//	would leak).
				pCombinedResult.Attach(*out_ppX1ItemResult, false);

				//	Get the next field
				nX1Field = queueX1Fields.front();
				queueX1Fields.pop_front();

				//	Do the next field
				if (in_eSearchType == st_X1Query)
				{
					//	When doing an X1 query, we call the X1 "higher" level API that will
					//	interpret X1's query language.
					pSearch->Reset(pEmailDB);
					pSearch->AddTerms(bstrSearchText, nX1Field);
					pSearch->Perform(&pCurrentResult);
				}
				else
				{
					//	When doing anything other than an X1 query, we call the X1 "lower"
					//	level API that will just search exactly as we tell it to without
					//	interpreting the search text.
					pEmailDB->FindItems(in_x1FindType, bstrSearchText, nX1Field, &pCurrentResult);
				}

				//	Must NULL it in order for X1 to give us results
				*out_ppX1ItemResult = NULL;

				//	Combine the new result
				pCurrentResult->OpOr(pCombinedResult, out_ppX1ItemResult);
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "DoTextSearchTerm", e.ErrorMessage() );

		//	Re-throw the exception for DoSearchTerm to catch
		throw;
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "DoTextSearchTerm");

		//	Re-throw the exception for DoSearchTerm to catch - don't delete
		//	the pException object, because DoSearchTerm will need it.
		throw;
	}
}


// ---------------------------------------------------------------------------
//		* DoSearchTerm												 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearch to search for a specific term.

void
SearchManager::Searching::DoSearchTerm(
	const SearchCriteria &		in_searchCriteria,
	X1::IX1ItemResult **		out_ppX1ItemResult)
{
	try
	{
		bool				bDoSearchTerm = true;
		bool				bDoInversion = false;
		SearchTypeEnum		eSearchType;
		X1::X1FindType		x1FindType;

		GetX1FindType(in_searchCriteria, x1FindType, eSearchType, bDoInversion, bDoSearchTerm);

		if (bDoSearchTerm)
		{
			if (eSearchType == st_Numeric)
				DoNumericSearchTerm(in_searchCriteria, x1FindType, bDoInversion, out_ppX1ItemResult);
			else
				DoTextSearchTerm(in_searchCriteria, eSearchType, x1FindType, out_ppX1ItemResult);

			if (bDoInversion)
			{
				//	Get the inverse of the previously found results
				X1::IX1ItemResultPtr		pResult;

				//	Attach to the results up to this point without adding to the ref count
				//	by passing false in the second parameter. (FindItems already returns it
				//	with a ref count of 1, if we allowed the ref count to be incremented we
				//	would leak).
				pResult.Attach(*out_ppX1ItemResult, false);

				//	Must NULL it in order for X1 to give us results
				*out_ppX1ItemResult = NULL;

				//	Get the inverse of the previously found results
				pResult->OpNot(out_ppX1ItemResult);
			}
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "DoSearchTerm", e.ErrorMessage() );

		//	Re-throw the exception for DoSearch to catch
		throw;
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "DoSearchTerm");

		//	Re-throw the exception for DoSearchTerm to catch - don't delete
		//	the pException object, because DoSearch will need it.
		throw;
	}
}


// ---------------------------------------------------------------------------
//		* FindMessagesInMailbox										 [Public]
// ---------------------------------------------------------------------------
//	Called by FindMessagesInMailboxes to find messages that match a
//	specific mailbox.

void
SearchManager::Searching::FindMessagesInMailbox(
	CString &					in_strMailboxPath,
	X1::IX1ItemResult **		out_ppX1ItemResult)
{
	//	Determine the relative path of the mailbox.
	//	We prefix it with "UniqueRootPrefixForEudoraMailboxPaths " to make it
	//	possible to uniquely find mailboxes without worrying about sub-paths
	//	incorrectly matching our searches (see SearchManager.cpp for more info).
	CString			szMailboxRelativePath = SearchManager::kMailboxRelativePathRootTag;
	szMailboxRelativePath += static_cast<const char *>(in_strMailboxPath) + EudoraDirLen;

	//	Look for the mailbox or folder relative path
	X1::IX1DatabasePtr		pEmailDB = SearchManager::Instance()->m_pInfo->m_pX1EmailDB;
	_bstr_t					bstrSearchText = szMailboxRelativePath;
	pEmailDB->FindItems( X1::ftContainsExact, bstrSearchText, efid_MailboxRelativePath, out_ppX1ItemResult );
}


// ---------------------------------------------------------------------------
//		* FindMessagesInMailboxes									 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearch to search for a list of mailboxes.

void
SearchManager::Searching::FindMessagesInMailboxes(
	std::deque<CString> &		in_listCheckedMailboxes,
	std::deque<CString> &		in_listUncheckedMailboxes,
	X1::IX1ItemResult **		out_ppX1ItemResult)
{
	try
	{
		std::deque<CString> *	pListMailboxes;
		bool					bDoInversion = false;

		if ( in_listCheckedMailboxes.size() <= in_listUncheckedMailboxes.size() )
		{
			//	List of check mailboxes is smaller
			pListMailboxes = &in_listCheckedMailboxes;

			//	Only do inversion in special case when list of checked mailboxes is empty
			bDoInversion = in_listCheckedMailboxes.empty();
		}
		else
		{
			//	List of unchecked mailboxes is smaller
			pListMailboxes = &in_listUncheckedMailboxes;

			//	Do inversion, unless list of unchecked mailboxes is empty
			bDoInversion = !in_listUncheckedMailboxes.empty();
		}

		if ( pListMailboxes->empty() )
		{
			//	No list of mailboxes, so just find every darn message by searching
			//	for contains prefix of an empty string.
			X1::IX1DatabasePtr		pEmailDB = SearchManager::Instance()->m_pInfo->m_pX1EmailDB;
			_bstr_t					bstrSearchText = "";
			pEmailDB->FindItems( X1::ftContainsPrefix, bstrSearchText, efid_MailboxRelativePath, out_ppX1ItemResult );
		}
		else	//	!pListMailboxes->empty()
		{
			X1::IX1ItemResultPtr			pCombinedResult;
			X1::IX1ItemResultPtr			pCurrentResult;
			std::deque<CString>::iterator	iteratorMailboxPaths = pListMailboxes->begin();

			//	Look for the mailbox or folder relative path
			FindMessagesInMailbox( (*iteratorMailboxPaths), out_ppX1ItemResult );

			for ( iteratorMailboxPaths++;
				iteratorMailboxPaths != pListMailboxes->end();
				iteratorMailboxPaths++ )
			{
				//	Attach to the results up to this point without adding to the ref count
				//	by passing false in the second parameter. (FindItems already returns it
				//	with a ref count of 1, if we allowed the ref count to be incremented we
				//	would leak).
				pCombinedResult.Attach(*out_ppX1ItemResult, false);

				//	Look for the mailbox or folder relative path
				FindMessagesInMailbox( (*iteratorMailboxPaths), &pCurrentResult );

				//	Must NULL it in order for X1 to give us results
				*out_ppX1ItemResult = NULL;

				//	Combine the new result
				pCurrentResult->OpOr(pCombinedResult, out_ppX1ItemResult);
			}
		}

		if (bDoInversion)
		{
			X1::IX1ItemResultPtr	pPrevResult;
			
			//	Attach to the results up to this point without adding to the ref count
			//	by passing false in the second parameter. (FindItems already returns it
			//	with a ref count of 1, if we allowed the ref count to be incremented we
			//	would leak).
			pPrevResult.Attach(*out_ppX1ItemResult, false);

			//	Must NULL it in order for X1 to give us results
			*out_ppX1ItemResult = NULL;

			//	Get the inverse of the previously found results
			pPrevResult->OpNot(out_ppX1ItemResult);
		}
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "FindMessagesInMailboxes", e.ErrorMessage() );

		//	Re-throw the exception for DoSearch to catch
		throw;
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "FindMessagesInMailboxes");

		//	Re-throw the exception for DoSearch to catch - don't delete
		//	the pException object, because DoSearch will need it.
		throw;
	}
}


// ---------------------------------------------------------------------------
//		* FindMessagesNotIMAPDeleted								 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearch to find all messages that are *not* marked as
//	IMAP deleted (i.e. all POP messages and all IMAP messages without
//	Deleted flag).

void
SearchManager::Searching::FindMessagesNotIMAPDeleted(
	X1::IX1ItemResult **		out_ppX1ItemResult)
{
	try
	{
		//	Load the "\\Deleted" string
		CRString		szIMAPDeletedFlag(IDS_IMAP_DELETED_STRING_FLAG);

		//	Trim off the leading "\\" from the "\\Deleted" string
		szIMAPDeletedFlag.Trim('\\');

		//	No list of mailboxes, so just find every darn message by searching
		//	for contains prefix of an empty string.

		//	Find all deleted IMAP messages
		X1::IX1ItemResultPtr	pCurrentResult;
		X1::IX1DatabasePtr		pEmailDB = SearchManager::Instance()->m_pInfo->m_pX1EmailDB;
		_bstr_t					bstrSearchText = szIMAPDeletedFlag;
		pEmailDB->FindItems( X1::ftContainsExact, bstrSearchText, efid_ImapFlags, &pCurrentResult );

		//	Negate the search result to find all messages that are *not*
		//	marked as IMAP deleted.
		pCurrentResult->OpNot(out_ppX1ItemResult);
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		SearchManager::Utils::LogError( "COM", "FindMessagesNotIMAPDeleted", e.ErrorMessage() );

		//	Re-throw the exception for DoSearch to catch
		throw;
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
		
		//	Log error
		SearchManager::Utils::LogCException(pException, "FindMessagesNotIMAPDeleted");

		//	Re-throw the exception for DoSearch to catch - don't delete
		//	the pException object, because DoSearch will need it.
		throw;
	}
}


// ---------------------------------------------------------------------------
//		* PrepareSortColumns										 [Public]
// ---------------------------------------------------------------------------
//	Called by DoSearch to prepare search column info.

void
SearchManager::Searching::PrepareSortColumns(
	COleSafeArray &				in_safeArraySortBy,
	COleSafeArray &				in_safeArraySortOrder)
{
	int		nColumns = 0;

	//	Count how many legitimate columns we have
	for ( Info::SortColumnsIteratorT i = SearchManager::Instance()->m_pInfo->m_sortColumns.begin();
		  i != SearchManager::Instance()->m_pInfo->m_sortColumns.end();
		  i++ )
	{
		if (*i != NOT_SORTED)
			nColumns++;
	}

	//	Set up what we want the search results to be sorted by
	int *			pSortByData;
	int *			pSortOrderData;

	//	Setup the type of arrays that we need
	V_VT(&in_safeArraySortBy) = VT_ARRAY | VT_INT;
	V_VT(&in_safeArraySortOrder) = VT_ARRAY | VT_INT;

	if (nColumns == 0)
	{
		//	Create the sort by array with one element: -1
		V_ARRAY(&in_safeArraySortBy) = SafeArrayCreateVector(VT_INT, 0, 1);
		in_safeArraySortBy.AccessData( reinterpret_cast<void **>(&pSortByData) );
		pSortByData[0] = -1;
		in_safeArraySortBy.UnaccessData();

		//	Create the sort order array with one element: 0
		V_ARRAY(&in_safeArraySortOrder) = SafeArrayCreateVector(VT_INT, 0, 1);
		in_safeArraySortOrder.AccessData( reinterpret_cast<void **>(&pSortOrderData) );
		pSortOrderData[0] = 0;
		in_safeArraySortOrder.UnaccessData();
	}
	else
	{
		Info::SortColumnsIteratorT		i;
		int								nColumns = 0;
		SearchManager::Info *			pInfo = SearchManager::Instance()->m_pInfo;

		//	Count how many legitimate columns we have
		for ( Info::SortColumnsIteratorT i = pInfo->m_sortColumns.begin();
			  i != pInfo->m_sortColumns.end();
			  i++ )
		{
			if (*i != NOT_SORTED)
				nColumns++;
		}
		
		//	Create sort by and sort order with nColumns
		V_ARRAY(&in_safeArraySortBy) = SafeArrayCreateVector(VT_INT, 0, nColumns);
		V_ARRAY(&in_safeArraySortOrder) = SafeArrayCreateVector(VT_INT, 0, nColumns);

		in_safeArraySortBy.AccessData( reinterpret_cast<void **>(&pSortByData) );
		in_safeArraySortOrder.AccessData( reinterpret_cast<void **>(&pSortOrderData) );

		//	Fill in sort by and sort order arrays
		int		j = 0;

		for ( Info::SortColumnsIteratorT i = pInfo->m_sortColumns.begin();
			  i != pInfo->m_sortColumns.end();
			  i++ )
		{
			switch (*i)
			{
				case NOT_SORTED:
					break;

				case BY_STATUS:
				case BY_REVERSE_STATUS:
					pSortByData[j] = efid_State;
					pSortOrderData[j] = (*i == BY_STATUS) ? 0 : 1;
					break;

				case BY_JUNK:
				case BY_REVERSE_JUNK:
					pSortByData[j] = efid_JunkScoreNumber;
					pSortOrderData[j] = (*i == BY_JUNK) ? 0 : 1;
					break;

				case BY_PRIORITY:
				case BY_REVERSE_PRIORITY:
					pSortByData[j] = efid_PriorityNumber;
					pSortOrderData[j] = (*i == BY_PRIORITY) ? 0 : 1;
					break;

				case BY_ATTACHMENT:
				case BY_REVERSE_ATTACHMENT:
					pSortByData[j] = efid_AttachmentCountNumber;
					pSortOrderData[j] = (*i == BY_ATTACHMENT) ? 0 : 1;
					break;

				case BY_LABEL:
				case BY_REVERSE_LABEL:
					pSortByData[j] = efid_Label;
					pSortOrderData[j] = (*i == BY_LABEL) ? 0 : 1;
					break;

				case BY_SENDER:
				case BY_REVERSE_SENDER:
					pSortByData[j] = efid_Who;
					pSortOrderData[j] = (*i == BY_SENDER) ? 0 : 1;
					break;

				case BY_DATE:
				case BY_REVERSE_DATE:
					pSortByData[j] = efid_DateSeconds;
					pSortOrderData[j] = (*i == BY_DATE) ? 0 : 1;
					break;

				case BY_SIZE:
				case BY_REVERSE_SIZE:
					pSortByData[j] = efid_SizeNumber;
					pSortOrderData[j] = (*i == BY_SIZE) ? 0 : 1;
					break;

				case BY_SERVERSTATUS:
				case BY_REVERSE_SERVERSTATUS:
					ASSERT(!"Server status sorting not handled yet");
					break;

				case BY_MOOD:
				case BY_REVERSE_MOOD:
					pSortByData[j] = efid_Mood;
					pSortOrderData[j] = (*i == BY_MOOD) ? 0 : 1;
					break;

				case BY_SUBJECT:
				case BY_REVERSE_SUBJECT:
					pSortByData[j] = efid_Subject;
					pSortOrderData[j] = (*i == BY_SUBJECT) ? 0 : 1;
					break;
			}
		}

		//	Unaccess the data
		in_safeArraySortOrder.UnaccessData();
		in_safeArraySortBy.UnaccessData();
	}
}
