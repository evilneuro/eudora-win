#if !defined(AFX_MOODWATCH_H__3A69ECBE_3BFB_11D4_AD74_0008C7D3A427__INCLUDED_)
#define AFX_MOODWATCH_H__3A69ECBE_3BFB_11D4_AD74_0008C7D3A427__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MoodWatch.h : header file
//
//Different type of Mood Words
#define MOOD_H_WORD				1
#define MOOD_NON_H_WORD			2

#include "TAE.h"
/////////////////////////////////////////////////////////////////////////////
// CMoodMailStatic window

class CMoodWatch 
{
// Construction
public:
	CMoodWatch();
    virtual	~CMoodWatch();

// Attributes
public:
	//CImageList m_listMoodImages;

// Operations
public:
	BOOL Init();
	//int AddText();
	static struct TAEDictState	m_taeds;
	static BOOL m_bDictInitialized;
	static BOOL m_bMoodWatchEnabled;
	static char **m_ppszWhitelist;

	static void CloseMoodDictionary();

	int AddText(char *pChar, int nLength,TAEAllMatches *sTaeAllMatches=NULL, 
		unsigned long lOptions=0);
	int GetScore();
	TAESessionState* GetSessionState(){ return &m_taess;}

protected:
	int m_nScore;
	int m_nTextBlocksAdded;
	BOOL m_bAddTextFailed;
	BOOL m_bSessionStarted;
	struct TAEScoreData		 m_taesd;
	struct TAESessionState	 m_taess;

	
};




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOODWATCH_H__3A69ECBE_3BFB_11D4_AD74_0008C7D3A427__INCLUDED_)
