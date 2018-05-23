/*
This class is used to check the header fields for any intersecting nicknames on the Boss
Protector list and warn the user accordingly
*/


#include "stdafx.h"

#include "resource.h"
#include "headervw.h"
#include "BossProtector.h"
#include "doc.h"
#include "nickdoc.h"
#include "address.h"
#include <QCUtils.h>
#include "DebugNewHelpers.h"
#include "rs.h"

CStringList CBossProtector::m_BPInsideDomainList;
CStringList CBossProtector::m_BPOutsideDomainList;

CBossProtector::CBossProtector()
{
	m_hEdit = NULL;
}

CBossProtector::~CBossProtector()
{
	
}

void CBossProtector::Build()
{
	int	WarnOutsideDomains = GetIniShort(IDS_INI_BP_SEND_MAIL_OUTSIDE_DOMAIN);
	if(WarnOutsideDomains == 1) {
		char OutsideDomainsList[1024];
		GetIniString(IDS_INI_BP_OUTSIDE_DOMAINS, OutsideDomainsList, sizeof(OutsideDomainsList));
		BuildDomainList(OutsideDomainsList, m_BPOutsideDomainList);
	} else {
		m_BPOutsideDomainList.RemoveAll();
	}

	int	WarnInsideDomains = GetIniShort(IDS_INI_BP_SEND_MAIL_INSIDE_DOMAIN);
	if(WarnInsideDomains == 1) {
		char InsideDomainsList[1024];
		GetIniString(IDS_INI_BP_INSIDE_DOMAINS, InsideDomainsList, sizeof(InsideDomainsList));
		BuildDomainList(InsideDomainsList, m_BPInsideDomainList);
	} else {
		m_BPInsideDomainList.RemoveAll();
	}
}

void CBossProtector::BuildDomainList(char* domains, CStringList &domainList)
{
		if(!domainList.IsEmpty())
			domainList.RemoveAll();		
		char* Start = domains;
		while (*Start)
		{
			char* Next = strchr(Start, ',');
			if (Next)
				*Next++ = 0;
			else
				Next = Start + strlen(Start);
				
			::TrimWhitespaceMT(Start);
			if(Start && (strlen(Start) != 0))
				domainList.AddTail(Start);
				
			Start = Next;
		}
}

bool CBossProtector::Check(CEdit* hEdit, int LinesFromTop, CStringList * pAddressList /* = NULL */)
{
	m_hEdit = hEdit;
	ASSERT(m_hEdit);
	m_bHilightWord = false;
	bool result = false;

	if(m_hEdit) {
			m_BegChar = 0;
			m_EndChar = m_hEdit->GetWindowTextLength();

		m_BegRow = m_hEdit->LineFromChar(m_BegChar);
		m_EndRow = m_hEdit->LineFromChar(m_EndChar);
		m_BegCol = m_BegChar - m_hEdit->LineIndex(m_BegRow);
		m_EndCol = m_EndChar - m_hEdit->LineIndex(m_EndRow);

		m_CurRow = m_BegRow;
		m_CurBufIndex = m_CurIndex = m_CurCol = m_BegCol;
    
		m_DidTopScroll = FALSE;
		m_TopVisibleLine = LinesFromTop;//3 - LinesFromTop;

        char   szProbWord[1024];

        CWaitCursor wait;
       
		for ( ;; ) {
            result = CheckBlock( szProbWord, pAddressList );

           if( result )
               break;

		}

	}

	return m_bHilightWord;

}  

void CBossProtector::HilightWord()
{
	int BeginChar	= m_CurCol;
	int EndChar		= m_CurIndex;
	if(m_bComma)
		EndChar -= 2;
	
	BeginChar += m_hEdit->LineIndex(m_CurRow);
	EndChar += m_hEdit->LineIndex(m_CurRow);

	CHeaderField *pField = DYNAMIC_DOWNCAST(CHeaderField, m_hEdit);
	if ( (pField) && (GetIniShort( IDS_INI_BP_COLOR_CODE_ADDRESSES )) )
		pField->DrawBPAddressesLine(BeginChar, EndChar);

	if (!m_DidTopScroll)
	{
		m_hEdit->SendMessage(WM_VSCROLL, SB_TOP);
		m_DidTopScroll = TRUE;
	}

	int ScrollAmount = m_CurRow - m_TopVisibleLine;
	if (ScrollAmount > 0)
	{
			//The current row is off the screen so scroll
			m_hEdit->LineScroll(ScrollAmount); 
			m_TopVisibleLine += ScrollAmount;
	}
}   

bool CBossProtector::CheckBlock(char* errWord, CStringList * pAddressList) 
{
	int WordLen;  
	char szLine[1024] = {'\0'};

//	int ExcerptLevel=0;
	
	while (m_CurRow <= m_EndRow)
	{ 
		int nCount = 0;
		m_CurBufIndex = m_CurIndex;
		nCount = m_hEdit->GetLine(m_CurRow, szLine, sizeof(szLine) - 1);
		szLine[nCount] = 0; 
		if (nCount != 0) 
		{

			//each word can either be a 
			//(i)nickname (automatically expand nicknames is turned off) - Sangita_Mohan
			//(ii)nickname (automatically expand nicknames is turned on) - Sangita_Mohan <sangitam@qualcomm.com>
			//(iii)individual email address - sangitam@qualcomm.com
			//(iv)distribution list(automatically expand nicknames is turned off) - Sangita_Mohan_DistList
			//(v)distribution list(automatically expand nicknames is turned on) - 
			//"Sangita_Mohan_DistList":sangitam@flagg2.qualcomm.com, sangitam@qualcomm.com, sangitamohan@qualcomm.com, sangita@qualcomm.com;

			//when do we decide to warn users?
			//we always expand all nicknames in the header field into the respective addresses. 
			//we also maintain a list of all the addresses on BP list
			//we get this list by expanding all nicknames and nicknames from address books on the BP list into a list of addresses
			//then check if any of the addresses on the recipient list match the addresses on the BP list
				
			//we do address matching instead of nickname matching
			//assume i have Steve_Dorner on my BP list that expands to sdorner@qualcomm.com
			//if i were to type the sdorner@qualcomm.com instead of using the nickname 
			//or use a distribution list(not on the BP list)
			//that has Steve's address in it, then i need to be warned.
			//for this reason, we need to do address matching and not nickname matching
			//only drawback is that assume user can the following case
			//nick1 expanding to test1, test2, test3
			//nick2 expanding to test2, test3, test4
			//the option to expand nicknames is turned off
			//now nick1 is on BP list, user sends email to nick2
			//nick2 is colored and it could be confusing to user since nick2 is not on BP list
			//nick2 is colored only because it has addresses that intersect with nick1
			//better to default to warning the user more often than not warning the user
			//so we color nick2 anway
	
			while ((WordLen = ParseLine(szLine, (char*)errWord, nCount)) > 0)
			{
				if (m_CurRow == m_BegRow && m_CurIndex < m_BegCol) continue;
				if (m_CurRow == m_EndRow && m_CurIndex > m_EndCol) continue;
	            
				CheckBPAddress( errWord, pAddressList );				
			}

		}

		m_CurRow++;
		m_CurIndex=0;		// number of characters checked in the current line
	}
	
	return true;
}

bool CBossProtector::CheckBPAddress(const char* address, CStringList * pAddressList) 
{
	bool result = false;
	char* ExpandedLine;
	if (!(ExpandedLine = ExpandAliases(address, TRUE, TRUE, TRUE)))
		return false;
		
	CString copy(ExpandedLine);
	char* line = copy.GetBuffer(copy.GetLength() + 1);
	char* end = line + ::SafeStrlenMT(line);
	char addr[258];
	char* start;

	if (!ExpandedLine || !*ExpandedLine || !line)
		return false;

	for (; line < end; line++)
	{
		start = line;
		line = FindAddressEnd(start);
		if (!line)
			return false;
		*line = 0;

		StripAddress(start);

		// Get rid of comments in group syntax
		char LastChar = 0;
		BOOL InQuote = FALSE;
		for (char* s = start; *s; LastChar = *s++)
		{
			if (*s == '"')
				InQuote = !InQuote;
			else if (!InQuote && LastChar == ':')
			{
				if (*s != ':')
					start = s;
				else if (*++s == 0)
					break;
			}
		}
		strcpy(addr, start);

		int len = strlen(addr);
		if (addr[len - 1] == ';')
			addr[--len] = 0;
				
		TrimWhitespaceMT(addr);

		result = ( g_Nicknames->IsAddressOnBPList( addr ) || IsOnBPDomainList( addr ) );
		
		if(result && pAddressList)
			pAddressList->AddTail(addr);
		
		if (result) {
			HilightWord();
			m_bHilightWord = true;
		}
					
	}
				
	delete [] ExpandedLine;
	return result;
}

int CBossProtector::ParseLine(char* CurLin, char* CurWord, int LinLen)
{
	int WordLen;
	m_bComma = false;
	WordLen = 0;		// initialize the variables
	bool bStartParseLine = true;
    
	while (m_CurBufIndex < LinLen)
	{
		char CurChar = CurLin[m_CurBufIndex];

		while(bStartParseLine) {
			if(isspace(CurChar)) {
				m_CurCol++;
				m_CurBufIndex++;
				m_CurIndex++;
				CurChar = CurLin[m_CurBufIndex];
			} else {
				bStartParseLine = false;
			}
		}

		//we get the individual nicknames when we hit a comma or end of line
		if((CurChar == ',') ||(CurChar == '\n') || (CurChar == '\r'))
		{
			if(CurChar == ',')
				m_bComma = true;
         	// a word extracted
			CurWord[WordLen] = 0;	// delimit the current word
			m_CurIndex++;					// skip the ,
			m_CurBufIndex++;
			return (WordLen);		// return the length of the word
		}
		// add a char to the output token
		CurWord[WordLen] = CurChar;
		if (WordLen == 0)
			m_CurCol = m_CurIndex;
		WordLen++;
		m_CurIndex++;					// next character
		m_CurBufIndex++;
	}
    
	if (WordLen > 0)					// check the last word of the line
	{
		CurWord[WordLen] = 0;			// delimit the current word
		m_CurIndex--;
		return (WordLen);				// return the length of the word
	}
    

	return (0);							// end of line encountered
}  


int CBossProtector::Popup(CEdit* pEdit, char* pWord, POINT& point, BOOL doubledWord/*false*/) 
{
/*	if (!pWord || !pEdit )
		return -1;
	
	m_hEdit = pEdit;
	//
	// This should of already been created 
	//
	if ( m_pSpellDialog == NULL )
		m_pSpellDialog = DEBUG_NEW CSpellDialog( this );
	//
	// Convert client Paige coords to screen
	//
	POINT pt;
	pt.x = point.x;
	pt.y = point.y;
	pEdit->ClientToScreen(&pt);
	
	char PrevWord[256];
	strcpy(PrevWord,pWord);
	//
	// Hilight the misspelled word and copy it into a buffer
	//
	//
	// Let the spell checker put up a popup of suggestions
	//

	//First check if it is a doubled word.

	int ret;
	if ( ret=m_pSpellDialog->Popup(pWord, pt, doubledWord,1) )
	{
		
		if ( ret == COMMAND_ID_ADD )
		{
			//
			// Add this word to the proper spelling dictionary
			//
			if ( ret == COMMAND_ID_IGNOREALL )
				m_pSpellDialog->IgnoreAll((unsigned char*)pWord);
			else
				m_pSpellDialog->AddToLex((unsigned char*)pWord);

			//	Broadcast the add to all CCompMessageFrames, including our own
			g_theSpellingDirector.NotifyClients(NULL, CA_SPELLING_ADD_WORD, pWord);
		}
		else if (ret == COMMAND_ID_DELETE_WORD)
		{

		}
		else 
		{
			pEdit->ReplaceSel(pWord);
		}
	}
*/
	return 0;
}

COLORREF CBossProtector::GetBPColorCode()
{
	CString sValue;
	long red = 0, green = 0, blue = 0;
	bool bColor = true;

	if (GetIniString(IDS_INI_BP_COLOR, sValue)) {

		LPSTR lpStr = NULL;
		red = strtol((LPCSTR)sValue, &lpStr, 10);
		if ((*lpStr++) != ',') {
			bColor = false;
		} else {
			green = strtol(lpStr, &lpStr, 10);
			if ((*lpStr++) != ',') {
				bColor = false;
			} else {
				blue = strtol(lpStr, &lpStr, 10);
			}
		}

		if(bColor)
			return RGB(red,green,blue);				
		else
			return RGB(255,0,255);				

	} 
		
	return RGB(255,0,255);				
}

bool CBossProtector::IsOnBPDomainList(const char* address)
{
	char* domain = strchr(address, '@');
	bool bMatch = false;
	if (domain)
	{
		domain++;
		
		if(!m_BPInsideDomainList.IsEmpty()) {

			POSITION	 pos = m_BPInsideDomainList.GetHeadPosition();
			POSITION	 posNext = NULL;
			for (posNext = pos; pos; pos = posNext)	{
				CString	in_domain = m_BPInsideDomainList.GetNext(posNext);
				//remove any wild card characters that the in_domain may start with
				if(in_domain.Left(2) == "*.")
					in_domain.Delete(0, 2);
				int domainLength = strlen(domain);
				int in_domainLength = in_domain.GetLength();
				if(in_domainLength >= domainLength) {
					if(in_domain.Find(domain) == (in_domainLength - domainLength)) {
						bMatch = true;
						break;
					}
				} else {
					CString szDomain(domain);
					if(szDomain.Find(in_domain) == (domainLength - in_domainLength)) {
						bMatch = true;
						break;
					}
				}
			}
		} else if(!m_BPOutsideDomainList.IsEmpty()) {

			POSITION	 pos = m_BPOutsideDomainList.GetHeadPosition();
			POSITION	 posNext = NULL;
			for (posNext = pos; pos; pos = posNext)	{
				CString	out_domain = m_BPOutsideDomainList.GetNext(posNext);	
				//remove any wild card characters that the out_domain may start with
				if(out_domain.Left(2) == "*.")
					out_domain.Delete(0, 2);
				int domainLength = strlen(domain);
				int out_domainLength = out_domain.GetLength();
				if(out_domainLength >= domainLength) {
					if(out_domain.Find(domain) == (out_domainLength - domainLength)) {
						bMatch = false;
						break;
					} else {
						bMatch = true;
					}
				} else {
					CString szDomain(domain);
					if(szDomain.Find(out_domain) == (domainLength - out_domainLength)) {
						bMatch = false;
						break;
					} else {
						bMatch = true;
					}
				}
			}
		}

	}
	return bMatch;
}
