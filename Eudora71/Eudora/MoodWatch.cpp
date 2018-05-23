// MoodWatch.cpp : implementation file
//

#include "stdafx.h"
#include "eudoraexe.h"
#include "MoodWatch.h"
#include "resource.h"

#include "rs.h"
#include "DebugNewHelpers.h"

extern CString ExecutableDir;

struct TAEDictState CMoodWatch::m_taeds = {0};
BOOL CMoodWatch::m_bDictInitialized = 0;
BOOL CMoodWatch::m_bMoodWatchEnabled = 1;
char **CMoodWatch::m_ppszWhitelist = NULL;


/////////////////////////////////////////////////////////////////////////////
// CMoodWatch

CMoodWatch::CMoodWatch()
{
//	m_listMoodImages.Create(IDB_IL_MOOD_MAIL, 16, 0, RGB(192, 192, 192)); 
	m_nScore = 0;
	m_bSessionStarted = 0;
	m_nTextBlocksAdded = 0;
	m_bAddTextFailed = 0;
}

BOOL CMoodWatch::Init()
{
	CString strDictFile;
	if (!m_bMoodWatchEnabled)
	{
		return FALSE;
	}
	if (m_bDictInitialized==0)
	{
		CString strName;
		GetIniString(IDS_INI_MOOD_DICTIONARY,strName);
		strDictFile = ExecutableDir+ strName;
		
		if (TAEInitDictionary(&m_taeds,(char*)(LPCTSTR) strDictFile))
		{
			m_bDictInitialized = 1;
			
			// After initializing the dictionary initialize the array of
			// whitelist words to be used.
			CString		 strWhitelist;
			GetIniString(IDS_INI_MOOD_MAIL_WHITELIST_WORDS, strWhitelist);

			// Our assumption is that the INI entry contains a comma separated list of words with
			// no whitespace.  Whitespace will actually prevent the MoodWatch engine from ever
			// matching anything, so while removing it may cause in unexpected results, leaving
			// it in would probably also cause unexpected results.  We also need to guard against
			// the user doing other unexpected things like having leading or trailing commas or
			// commas with no meaningful content between them.  To deal with those cases, we trim
			// all leading and trailing commas then if we find two commas next to each other we
			// replace the two commas with one, repeating the process until all commas are by
			// themselves.  All told this might be a lengthy process, but 1) this processing
			// is only done once per Eudora session, 2)	most users will never use this feature and
			// 3) those who do are unlikely to have some of these ridiculous cases.
			strWhitelist.Remove(' ');
			strWhitelist.Remove('\t');
			strWhitelist = strWhitelist.TrimLeft(',');
			strWhitelist = strWhitelist.TrimRight(',');
			while (strWhitelist.Replace(",,", ","));

			// At this point we are going to assume that we have a list of zero or more words
			// separated by single commas with no whitespace and no leading or trailing commas.
			if (!strWhitelist.IsEmpty())
			{
				int			iWhitelistWords = 1;
				int			iPos = 0;

				// Count the words in the INI setting.
				while (iPos != -1)
				{
					iPos = strWhitelist.Find(",", iPos);
					if (iPos != -1)
					{
						++iWhitelistWords;
						++iPos;
					}
				}
				// Parse the words into m_ppszWhitelist.
				m_ppszWhitelist = (char**)malloc(sizeof(char*) * (iWhitelistWords + 1));
				for (int i = 0, iPos = 0; i < iWhitelistWords; ++i)
				{
					iPos = strWhitelist.Find(",");
					if (iPos == -1)	iPos = strWhitelist.GetLength();
					m_ppszWhitelist[i] = (char*)malloc(iPos + 1);
					strncpy(m_ppszWhitelist[i], strWhitelist.GetBuffer(), iPos);
					m_ppszWhitelist[i][iPos] = '\0';
					strWhitelist = strWhitelist.Right(strWhitelist.GetLength() - iPos - 1);
				}
				m_ppszWhitelist[i] = NULL;
			}
		}
		else 
		{
			m_bMoodWatchEnabled = 0;
		}
	}
	if(m_bDictInitialized==1)
	{
		if (TAEStartSession(&m_taess, &m_taeds)&&m_bDictInitialized)
		{
			m_bSessionStarted = 1;
			// Optional: Use this only if you want detailed score info.
			return TRUE;
		}
	}
	return FALSE;
}

CMoodWatch::~CMoodWatch()
{
	if (m_bSessionStarted )
	{
		TAEFreeScoreData(&m_taesd);
		TAEEndSession(&m_taess);
	}

}

int CMoodWatch::AddText(char *pChar, int nLength,TAEAllMatches *pTaeAllMatches/*=NULL*/, 
						unsigned long lOptions/*=0*/)
{
	if(m_bSessionStarted && m_bMoodWatchEnabled && (nLength > 0) )
	{
		//TAEInitFreeAllMatches will be calld from PaigeEdtView and PgCompMsgView after
		//Marking the bad words.
		TAEInitAllMatches(pTaeAllMatches);
		if(TAEProcessText(&m_taess, pChar, nLength, pTaeAllMatches, lOptions, m_ppszWhitelist))
		{
			m_nTextBlocksAdded++;
			//TAEFreeAllMatches(&taeallmatches);
			return TRUE;
		}
		else if (m_nTextBlocksAdded == 0)
		{
			//TAEFreeAllMatches(&taeallmatches);
			//Disable MoodWatch and Close Dictionary
			m_bMoodWatchEnabled = 0;
			m_bAddTextFailed = 1;
			CloseMoodDictionary();
		}
	}
	return FALSE;
}

int CMoodWatch::GetScore()
{
	TAEInitScoreData(&m_taesd);
	if( (m_nTextBlocksAdded > 0) && m_bMoodWatchEnabled)
	{
		m_nScore = TAEGetScoreData(&m_taess, &m_taesd, NULL, 0);
		return m_nScore;
	}
	else if ((m_nTextBlocksAdded == 0) && (!m_bAddTextFailed))
	{
		return 0;
	}
	return -1;
}

void CMoodWatch::CloseMoodDictionary()
{
	if(CMoodWatch::m_bDictInitialized)
	{
		TAECloseDictionary(&CMoodWatch::m_taeds);
		CMoodWatch::m_bDictInitialized = 0;
	}
}
