/////////////////////////////////////////////////////////////////////////////
// 	SEARCHUTIL.cpp - Helper class for conversion from a MultiSearchCriteria list to a
// search program.
//

#include "stdafx.h"

#include "SearchCriteria.h"

#include "eudora.h"
#include "rs.h"
#include "imap.h"
#include "ImapExports.h"
#include "imapfol.h"
#include "imaputil.h"
#include "ImapTypes.h"
#include "searchutil.h"
#include "QCUtils.h"


//	Defines and other Constants	=============================
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// INTERNALS:
//
static const char Comma = ',';
static const char Colon = ':';


static LPCSTR pFullHeaders = "Return-Path,Received,Message-Id,X-Sender,Date,To,From,Subject,In-Reply-To,References,Content-Type,Cc,Bcc";



// ============== CSearchUtil ==========================//

CSearchUtil::CSearchUtil()
{
	m_pMultSearchCriteria	= NULL;
	m_pCriteriaList			= NULL;
	m_Iterator				= NULL;
}



CSearchUtil::~CSearchUtil()
{

}


// Initialize [PRIVATE]
//
// NOTES:
// m_Iterator always points to the current object in the list.
//
// END NOTES
//
BOOL CSearchUtil::Initialize (MultSearchCriteria* pMultSearchCriteria)
{
	if (!pMultSearchCriteria)
		return FALSE;

	// 
	SearchCriteriaList *pCriteriaList = pMultSearchCriteria->GetCriteriaList();
	if (!pCriteriaList)
	{
		return FALSE;
	}

	// Cache the list:
	//
	m_pMultSearchCriteria	= pMultSearchCriteria;
	m_pCriteriaList			= pCriteriaList;

	// Initialize the iterator.
	//
	if (m_pCriteriaList)
		m_Iterator = m_pCriteriaList->begin();
	else
		m_Iterator = NULL;

	return TRUE;
}



// GetNextCriterion [PRIVATE]
//
SearchCriteria* CSearchUtil::GetNextCriterion ()
{
	SearchCriteria* pCurCriterion = NULL;

	if ( m_pCriteriaList && (m_Iterator != NULL) && (m_Iterator != m_pCriteriaList->end()) )
	{
		pCurCriterion = &(*m_Iterator);

		m_Iterator++;
	}

	return pCurCriterion;
}



// IsOpOR [PRIVATE]
BOOL CSearchUtil::IsOpOR(SearchCriteria* pCriterion)
{
	if (m_pMultSearchCriteria)
		return m_pMultSearchCriteria->IsOpOR();
	else
		return FALSE;
}



// IsOpAND [PRIVATE]
BOOL CSearchUtil::IsOpAND(SearchCriteria* pCriterion)
{
	if (m_pMultSearchCriteria)
		return m_pMultSearchCriteria->IsOpAND();
	else
		return FALSE;
}




// HasNext [PRIVATE]
//
BOOL CSearchUtil::HasNext()
{
	BOOL bHas = FALSE;

	return ( m_pCriteriaList && (m_Iterator != NULL) && (m_Iterator != m_pCriteriaList->end()) );
}







// FormatSearchCriteria [PUBLIC]
//
// Main entry point.
//
BOOL CSearchUtil::FormatSearchCriteria (SEARCHPGM** ppTopPgm, MultSearchCriteria* pMultSearchCriteria, LPCSTR pUidRange)
{
	if ( !(ppTopPgm && pMultSearchCriteria) )
		return TRUE;

	// Initialize state info from the given MultSearchCriteria.
	//
	if ( !Initialize(pMultSearchCriteria) )
	{
		return FALSE;
	}

	// Get the first criterion.
	SearchCriteria *pCurSearchCriterion = GetNextCriterion ();

	if (!pCurSearchCriterion)
	{
		ASSERT (0);
		return FALSE;
	}

	// Create our top SPGM.
	//
	SEARCHPGM* pPgm = CImapStream::NewSearchPgm();

	if (!pPgm)
	{
		ASSERT (0);
		return FALSE;
	}

	BOOL bResult = FALSE;

	// Add UID range for the search:
	//
	bResult = SetNewPgmUidList (pPgm, pUidRange);

	// Start the recursion.	
	if ( IsOpOR(pCurSearchCriterion) )
	{
		bResult = AddORSearchCriteria (pPgm, pCurSearchCriterion);
	}
	else
	{
		bResult = AddANDSearchCriteria (pPgm, pCurSearchCriterion);
	}

	// Fix up return.
	if (bResult)
	{
		*ppTopPgm = pPgm;
	}
	else if (pPgm)
	{
		// Back out.
		CImapStream::FreeSearchpgm (&pPgm);

		pPgm = NULL;
	}

	return bResult;
}







//////////////////////////////////////////////////////////////////////////////////
// AddORSearchCriteria [PRIVATE]
//
// Call this if you're beginning a set of OR criteria.
// Can have AND's and other OR's down the road.
// Must have valid pCurPgm AND pCurSearchCriteria.
//////////////////////////////////////////////////////////////////////////////////
BOOL  CSearchUtil::AddORSearchCriteria (SEARCHPGM* pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	// Must have these:
	if ( !(pCurPgm && pCurSearchCriteria) )
	{
		ASSERT (0);
		return FALSE;
	}

	// We MUST have a next criterion for us to do this!!
	// If we don't, we just want to set the current criterion into the given SPGM.
	//
	if ( !HasNext() )
	{
		return AddSingleCriterion (pCurPgm, pCurSearchCriteria);
	}

	// Otherwise, add this criterion and recurse.
	//

	BOOL bResult = TRUE;

	pCurPgm->or = (SEARCHOR *) malloc ( sizeof (SEARCHOR) );
	if (pCurPgm->or)
	{
		memset ( (void *) pCurPgm->or, 0, sizeof (SEARCHOR) );

		// Fill body criterion;
		pCurPgm->or->first = (SEARCHPGM *) malloc ( sizeof (SEARCHPGM) );
		if (pCurPgm->or->first)
		{
			memset ( (void *) pCurPgm->or->first, 0, sizeof (SEARCHPGM) );
		}
		else
			bResult = FALSE;

		if (bResult)
		{
			// Add just the single criterion here.
			//
			bResult = AddSingleCriterion (pCurPgm->or->first, pCurSearchCriteria);
		}

		// Do the next one.
		//
		if (bResult)
		{
			pCurPgm->or->second = (SEARCHPGM *) malloc ( sizeof (SEARCHPGM) );
			if (pCurPgm->or->second)
			{
				memset ( (void *) pCurPgm->or->second, 0, sizeof (SEARCHPGM) );
			}
			else
				bResult = FALSE;
		}

		// Recurse on this criterion now:
		if (bResult)
		{
			
			bResult = AddORSearchCriteria ( pCurPgm->or->second, GetNextCriterion () );
		}
	}

	return bResult;
}
		





//////////////////////////////////////////////////////////////////////////////////
// AddANDSearchCriteria [PRIVATE]
//
// Call this if you're beginning a set of AND criteria.
// Can have AND's and other OR's down the road.
// Must have valid pCurPgm AND pCurSearchCriteria.
//////////////////////////////////////////////////////////////////////////////////
BOOL  CSearchUtil::AddANDSearchCriteria (SEARCHPGM* pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	// Must have these:
	if ( !(pCurPgm && pCurSearchCriteria) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Add the current criterion.
	//
	BOOL bResult = AddSingleCriterion (pCurPgm, pCurSearchCriteria);

	// Get the next one and decide what to do:
	//
	SearchCriteria* pNextCriterion = GetNextCriterion();

	if (pNextCriterion)
	{
		if ( IsOpOR(pNextCriterion) )
		{
			bResult = AddORSearchCriteria (pCurPgm, pNextCriterion);
		}
		else
		{
			// Add the next criterion to the current SPGM.
			//
			bResult = AddANDSearchCriteria (pCurPgm, pNextCriterion);
		}
	}

	return bResult;
}
		







// AddSingleCriterion [PRIVATE]
//
// Add single criterion without recursing.
// Can have multiple criteria which are ANDED.
//
BOOL CSearchUtil::AddSingleCriterion (SEARCHPGM* pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	BOOL bResult = FALSE;

	// MUST have these:
	//
	if ( !(pCurPgm && pCurSearchCriteria) )
		return FALSE;

	CriteriaObject curObjectType;

	if ( !pCurSearchCriteria->GetCurObj(curObjectType) )
		return FALSE;
	
	switch (curObjectType)
	{
		case CRITERIA_OBJECT_ANYWHERE:
			// TEXT anywhere in message.
			bResult = AddTextObject (pCurPgm, pCurSearchCriteria, PGMTEXT_TEXT);
			break;

		case CRITERIA_OBJECT_HEADERS:
			bResult = AddObjectFullHeader (pCurPgm, pCurSearchCriteria);
			break;

		case CRITERIA_OBJECT_BODY:
			// TEXT anywhere in body.
			bResult = AddTextObject (pCurPgm, pCurSearchCriteria, PGMTEXT_BODY);
			break;


#ifdef DATEANDAGE

		case CRITERIA_OBJECT_DATE:
			bResult = AddObjectDate (pCurPgm, pCurSearchCriteria);
			break;

		case CRITERIA_OBJECT_AGE:
			bResult = AddObjectAge (pCurPgm, pCurSearchCriteria);
			break;

#endif // DATEANDAGE

		case CRITERIA_OBJECT_TO:
		case CRITERIA_OBJECT_CC:
		case CRITERIA_OBJECT_BCC:
		case CRITERIA_OBJECT_FROM:			// we can handle FROM now -jdboyd
		case CRITERIA_OBJECT_SUBJECT:		// we can handle SUBJECT now -jdboyd
			bResult = AddObjectHeader (pCurPgm, pCurSearchCriteria, curObjectType);
			break;

		case CRITERIA_OBJECT_ANYRECIP:
			bResult = AddAnyRecipient (pCurPgm, pCurSearchCriteria);
			break;

		// Can't handle these:
		case CRITERIA_OBJECT_SIZE:
		case CRITERIA_OBJECT_ATTACHNAMES:
		case CRITERIA_OBJECT_SUMMARY:
		case CRITERIA_OBJECT_STATUS:
		case CRITERIA_OBJECT_PRIORITY:
		case CRITERIA_OBJECT_ATTACHCOUNT:
		case CRITERIA_OBJECT_LABEL:
		case CRITERIA_OBJECT_PERSONA:
		default:

			bResult = FALSE;
			break;
	}


	return bResult;
}





// ImapServerCanSearch [PUBLIC]
//
// Return TRUE if this search criterion is one that an IMAP4rev1 server can perform.
//
// STATIC
BOOL CSearchUtil::ImapServerCanSearch (SearchCriteria *pCurSearchCriteria)
{
	BOOL bResult = FALSE;

	// MUST have these:
	//
	if ( !pCurSearchCriteria )
		return FALSE;

	CriteriaObject curObjectType;

	if ( !pCurSearchCriteria->GetCurObj(curObjectType) )
		return FALSE;
	
	switch (curObjectType)
	{
		case CRITERIA_OBJECT_ANYWHERE:
		case CRITERIA_OBJECT_HEADERS:
		case CRITERIA_OBJECT_BODY:
		case CRITERIA_OBJECT_TO:
		case CRITERIA_OBJECT_CC:
		case CRITERIA_OBJECT_BCC:
		case CRITERIA_OBJECT_ANYRECIP:
		case CRITERIA_OBJECT_FROM:
		case CRITERIA_OBJECT_SUBJECT:
		{
			CriteriaVerbType  verbType;
			CriteriaValueType valType;

			if (SearchCriteria::GetVerbValPair(curObjectType, verbType, valType) )
			{
				// Verb must be TEXT COMPARE type and value must be CRITERIA_VALUE_TEXT_TYPE.
				if (verbType == CRITERIA_VERB_TEXT_COMPARE_TYPE && valType == CRITERIA_VALUE_TEXT_TYPE)
				{
					// For these, determine if the verb is compatible. Must be "contains" or
					// "not contains"
					//
					CriteriaVerbTextCompare verb;

					if ( pCurSearchCriteria->GetCurVerbTextCmp(verb) )
					{
						if (verb == CRITERIA_TEXT_COMPARE_CONTAINS || verb == CRITERIA_TEXT_COMPARE_DOESNOTCONTAIN)
						{
							bResult = TRUE;
						}
					}
				}
			}

			break;
		}

		case CRITERIA_OBJECT_DATE:
		case CRITERIA_OBJECT_AGE:
		case CRITERIA_OBJECT_SIZE:


		// Can't handle these:
		case CRITERIA_OBJECT_ATTACHNAMES:
		case CRITERIA_OBJECT_SUMMARY:
		case CRITERIA_OBJECT_STATUS:
		case CRITERIA_OBJECT_PRIORITY:
		case CRITERIA_OBJECT_ATTACHCOUNT:
		case CRITERIA_OBJECT_LABEL:
		case CRITERIA_OBJECT_PERSONA:
		default:

			bResult = FALSE;
			break;
	}

	return bResult;
}





//============================ PRIVATE METHODS ====================================//


// AddTextObject [PRIVATE]
//
// This is implemented as a SEARCH TEXT <text>
//
BOOL CSearchUtil::AddTextObject (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria, PGMTEXT_WHERE where)
{
	// Sanity:
	if ( !(pCurPgm && pCurSearchCriteria) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Criterion value Must be of type text.
	//
	CriteriaObject curObjectType;

	if ( !pCurSearchCriteria->GetCurObj(curObjectType) )
		return FALSE;

	if ( SearchCriteria::GetValueType(curObjectType) != CRITERIA_VALUE_TEXT_TYPE )
	{
		ASSERT (0);
		return FALSE;
	}

	BOOL bResult = FALSE;

	// Make sure has valid text. Note: CriteriaValueText is a CString (or has CString semantics).
	//
	CriteriaValueText szText;

	if ( !pCurSearchCriteria->GetCurValueText(szText) )
	{
		ASSERT (0);
		return FALSE;
	}

	return AppendTextCriterion (pCurPgm, szText, where);
}





// AddObjectFullHeader [PRIVATE]
//
// This is implemented by searching for a hit in ANY of a given set of
// header fields. 
// 
BOOL CSearchUtil::AddObjectFullHeader (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	// Note: "pFullHeaders" is static.
	//
	return AddORNamedHeaderList (pCurPgm, pCurSearchCriteria, pFullHeaders);
}



BOOL CSearchUtil::AddObjectDate (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	return TRUE;
}



BOOL CSearchUtil::AddObjectAge (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	return TRUE;
}





BOOL CSearchUtil::AddAnyRecipient (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria)
{
	// Sanity:
	//
	if (! (pCurPgm && pCurSearchCriteria) )
		return FALSE;

	// Formulate list of object criteria:
	//
	CString szNamedHeaders;

	szNamedHeaders.Format ("To,Cc,Bcc");

	return AddORNamedHeaderList (pCurPgm, pCurSearchCriteria, szNamedHeaders);
}






// AddORHeaderList [PRIVATE]
//
// Some single "SearchCriteria" must be represented as multiple OR's to IMAP.
// This is a utility that does this, regardless of whether the main conjunction
// is AND or OR.
//
// The headers are passed in as a comma-separated seat of strings,
// e.g. "To,Subject".
// NOTE: MUST have at least 2 headers!!.
//
BOOL CSearchUtil::AddORNamedHeaderList (SEARCHPGM *pCurPgm, SearchCriteria *pCurSearchCriteria, LPCSTR pszNamedHeaders)
{
	// Sanity:
	//
	if (! (pCurPgm && pCurSearchCriteria && pszNamedHeaders) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Only 1 header??
	// 
	LPCSTR pNextHeader = strchr (pszNamedHeaders, Comma);

	// Is this the last one??
	CString szHeader;
	if (pNextHeader)
	{
		szHeader = CString (pszNamedHeaders, (pNextHeader - pszNamedHeaders));
	}
	else
	{
		szHeader = CString (pszNamedHeaders);
	}

	// Remove blanks.
	szHeader.TrimRight();
	szHeader.TrimLeft();

	// If just one, simply add the criterion to the given SEARCHPGM.
	//
	if (!pNextHeader)
	{
		return AddNamedHeader (pCurPgm, pCurSearchCriteria, szHeader);
	}

	// If we get here, create and new SEARCHOR, add header to it's "first" field, and recurse.
	//

	// Go to the end of the OR list.
	//
	SEARCHOR** ppOr = &pCurPgm->or;

	while (*ppOr)
		ppOr = & (*ppOr)->next;

	BOOL bResult = FALSE;

	*ppOr = (SEARCHOR *) malloc ( sizeof (SEARCHOR) );

	if (*ppOr)
	{
		memset ( (void *) (*ppOr), 0, sizeof (SEARCHOR) );

		// Fill body criterion;
		(*ppOr)->first = (SEARCHPGM *) malloc ( sizeof (SEARCHPGM) );

		if ((*ppOr)->first)
		{
			memset ( (void *) (*ppOr)->first, 0, sizeof (SEARCHPGM) );

			bResult = AddNamedHeader ((*ppOr)->first, pCurSearchCriteria, szHeader);
		}
	}

	// Recurse.
	//
	if (bResult)
	{
		bResult = FALSE;

		// Add the next header to this SEARCHOR's "second".
		//
		(*ppOr)->second = (SEARCHPGM *) malloc ( sizeof (SEARCHPGM) );
		if ((*ppOr)->second)
		{
			memset ( (*ppOr)->second, 0, sizeof (SEARCHPGM) );

			bResult = AddORNamedHeaderList ((*ppOr)->second, pCurSearchCriteria, pNextHeader + 1);
		}
	}
	
	return bResult;
}






// AddObjectHeader [PRIVATE]
//
// Add a single header to the end of the header list in the given "pCurPgm".
// NOTE: This calls "AddNamedHeader" to actually add the header name and text value to
// the PGM.
//
BOOL CSearchUtil::AddObjectHeader (SEARCHPGM* pCurPgm, SearchCriteria* pCurSearchCriteria, CriteriaObject objectType)
{
	// Sanity:
	if ( !(pCurPgm && pCurSearchCriteria) )
	{
		ASSERT (0);
		return FALSE;
	}

	BOOL bResult = FALSE;

	// Criterion value Must be of type text.
	if ( SearchCriteria::GetValueType(objectType) != CRITERIA_VALUE_TEXT_TYPE )
	{
		ASSERT (0);
		return FALSE;
	}
	else bResult = true;	// this is a SeartchCriteria type we can handle -jdboyd

	// Is the SearchCriterion one we can handle??
	//
	if (bResult)
	{
		CString szHeader;

		switch (objectType)
		{
			case CRITERIA_OBJECT_TO:
				szHeader = CRString (IDS_HEADER_TO);
				break;

			case CRITERIA_OBJECT_FROM:
				szHeader = CRString (IDS_HEADER_FROM);
				break;

			case CRITERIA_OBJECT_SUBJECT:
				szHeader = CRString (IDS_HEADER_SUBJECT);
				break;

			case CRITERIA_OBJECT_CC:
				szHeader = CRString (IDS_HEADER_CC);
				break;

			case CRITERIA_OBJECT_BCC:
				szHeader = CRString (IDS_HEADER_BCC);
				break;

			default:
				bResult = FALSE;
				break;
		}

		if (bResult)
		{
			// Remove any ":".
			//
			int i = szHeader.Find (':');
			if (i >= 0)
				szHeader = szHeader.Left (i);

			szHeader.TrimRight();
			szHeader.TrimLeft();

			bResult = AddNamedHeader (pCurPgm, pCurSearchCriteria, szHeader);
		}
	}

	return bResult;
}




// AddNamedHeader [PRIVATE]
//
// Given the name of the header (as a string, not a CriteriaObject), add it to 
// the end of the header list in the given "pCurPgm".
// 
BOOL CSearchUtil::AddNamedHeader (SEARCHPGM* pCurPgm, SearchCriteria* pCurSearchCriteria, LPCSTR pHeaderName)
{
	// Sanity:
	if ( !(pCurPgm && pCurSearchCriteria && pHeaderName) )
	{
		ASSERT (0);
		return FALSE;
	}

	BOOL bResult = FALSE;

	// Make sure has valid text. (Note: CriteriaValueText is a CString (or has CString semantics).
	//
	CriteriaValueText szText;

	if ( !pCurSearchCriteria->GetCurValueText(szText) )
	{
		ASSERT (0);
		return FALSE;
	}

	// Single header. Add it to the end of pCurPgm->header. These are all AND'ed.
	//
	SEARCHHEADER** ppCurHeader = &pCurPgm->header;

	while (*ppCurHeader)
		ppCurHeader = &(*ppCurHeader)->next;

	SEARCHHEADER* pHeader = *ppCurHeader = (SEARCHHEADER *) malloc ( sizeof (SEARCHHEADER) );

	if (pHeader)
	{
		memset ( (void *) pHeader, 0, sizeof (SEARCHHEADER) );

		pHeader->line = NULL;
		pHeader->next = NULL;

		bResult = FALSE;

		// Dup it.
		LPCSTR p = (LPCSTR) szText;
		if ( p && *p)
		{
			pHeader->text = (char *) malloc (strlen (p) + 1);
			if (pHeader->text)
			{
				strcpy (pHeader->text, p);
				bResult = TRUE;
			}
		}
	}
	else
	{
		bResult = FALSE;
	}

	// Add the header line's name.:
	//
	if (bResult && pHeader)
	{
		bResult = FALSE;

		pHeader->line = (char *) malloc (strlen (pHeaderName) + 1);
		if (pHeader->line)
		{
			strcpy (pHeader->line, pHeaderName);
			bResult = TRUE;
		}
	}

	return bResult;
}








// FreePgmUidList [PRIVATE]
// 
void CSearchUtil::FreePgmUidList (SEARCHPGM *pPgm)
{	
	SEARCHSET *pSet = NULL, *pNext = NULL;

	if (!pPgm)
		return;

	// Free current uid list.
	pSet = pPgm->uid;

	while (pSet)
	{
		pNext = pSet->next;

		free ( (void *) pSet);

		pSet = pNext;
	}

	// Make sure.
	pPgm->uid = NULL;
}




// SetNewPgmUidList [PRIVATE]
//
// Free the current uid list in the SPGM and create a new one based on the given
// comma-separated string.
BOOL CSearchUtil::SetNewPgmUidList (SEARCHPGM *pPgm, LPCSTR pStr)
{
	SEARCHSET *pSet = NULL;
	char *pList = NULL;

	// Sanity:
	if (! pPgm )
		return FALSE;

	// Free current uid list.
	FreePgmUidList (pPgm);

	// pStr can be NULL, in which case it's 1:*.
	if (! (pStr && *pStr) )
	{
		pList = SafeStrdupMT("1:*");
	}
	else
	{
		// Local copy:
		pList = SafeStrdupMT (pStr);
	}

	BOOL bResult = TRUE;

	char *p = pList;
	char *q;
	char cOrig = 0;

	while (p)
	{
		// Handle both "a:b" and comma-separated stuff.
		//
		for (q = p; *q && *q != ':' && *q != ','; q++)
		{
			;
		}

		// Are we at end of line?
		if (!*q)
			q = NULL;
		else
		{
			cOrig = 0;
			if (q)
			{
				cOrig = *q;
				*q = 0;
			}
		}

		// Add this uid.

		if (p && *p)
		{
			unsigned long Uid = atol (p);

			if (Uid != 0)
			{
				SEARCHSET* pNewSet = (SEARCHSET *) malloc ( sizeof (SEARCHSET) );

				if (pNewSet)
				{
					pNewSet->first = Uid;
					pNewSet->next = NULL;
					pNewSet->last = 0;

					// Put back.
					if (q)
						*q = cOrig;

					// If this was a range, must set "last":
					//
					if (q && (cOrig == Colon) )
					{
						// Get next one.
						//
						p = ++q;

						for (; *q && *q != ':' && *q != ','; q++)
						{
							;
						}

						if (!*q)
						{
							q = NULL;
						}
						else
						{
							cOrig = *q;
							*q = 0;
						}

						if ( strchr(p, '*') )
							pNewSet->last = 0xFFFFFFFF;
						else
						{
							Uid = atol (p);
							if (Uid != 0)
								pNewSet->last = Uid;
						}

						if (q)
							*q = cOrig;
					}

					// Link in.							
					if (pSet == NULL)
					{
						pSet = pPgm->uid = pNewSet;
					}
					else
					{
						pSet->next = pNewSet;

						pSet = pNewSet;
					}
				}
			}
		}

		if (q)
		{
			p = q + 1;
		}
		else
		{
			// Terminate.
			p = NULL;
		}
	}
			
	// Cleanup.

	if (pList)
		delete[] pList;

	pList = NULL;

	return bResult;
}




// AppendTextCriterion [PRIVATE]
// Set the search text at the end of the STRINGLIST of the given SEARCHPGM.
//
BOOL CSearchUtil::AppendTextCriterion (SEARCHPGM* pCurPgm, CriteriaValueText szText, PGMTEXT_WHERE where)
{
	// Sanity
	//
	if ( !pCurPgm)
		return FALSE;

	STRINGLIST** ppSlist = NULL;

	switch (where)
	{
		case PGMTEXT_BCC:
			ppSlist = &pCurPgm->bcc;
			break;

		case PGMTEXT_BODY:
			ppSlist = &pCurPgm->body;
			break;


		case PGMTEXT_CC:
			ppSlist = &pCurPgm->cc;
			break;


		case PGMTEXT_FROM:
			ppSlist = &pCurPgm->from;
			break;


		case PGMTEXT_KEYWORD:
			ppSlist = &pCurPgm->keyword;
			break;


		case PGMTEXT_UNKEYWORD:
			ppSlist = &pCurPgm->unkeyword;
			break;

		case PGMTEXT_SUBJECT:
			ppSlist = &pCurPgm->subject;
			break;

		case PGMTEXT_TEXT:
			ppSlist = &pCurPgm->text;
			break;

		case PGMTEXT_TO:
			ppSlist = &pCurPgm->to;
			break;

		default:
			return FALSE;
	}

	// Append text to end of list.
	//
	while (*ppSlist)
		ppSlist = & (*ppSlist)->next;

	BOOL bResult = FALSE;

	(*ppSlist) = (STRINGLIST *) malloc ( sizeof (STRINGLIST) );

	if (*ppSlist)
	{
		(*ppSlist)->next = NULL;

		(*ppSlist)->text.size = strlen (szText);

		(*ppSlist)->text.data = (LPSTR) malloc ((*ppSlist)->text.size + 1);
		strcpy ((*ppSlist)->text.data, szText);

		bResult = TRUE;
	}

	return bResult;
}




