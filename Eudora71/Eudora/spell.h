// SPELL.H
//
#ifndef _SPELL_H_
#define _SPELL_H_

#define NO_MISSPELLINGS -2
class CSpellDialog;

#include "ssce.h"

class CSpell
{
	CSpellDialog*	m_pSpellDialog;

public:
	CSpell(BOOL hasQuotedText = FALSE);
	~CSpell();

private:
	int ErrMsg(int ret);

public:
	virtual Check(CRichEditCtrl* hEdit, int LinesFromTop = 0, BOOL autoCheck = FALSE);
	virtual int Check(CEdit* hEdit, int LinesFromTop = 0, BOOL autoCheck = FALSE,BOOL bInteractive =FALSE);
	

	virtual void myHilightWord(BOOL bMark=FALSE);
	virtual void mySetRedraw(BOOL bRedraw=TRUE) {};
	virtual int my_CheckBlock(long options, UCHAR* errWord, UCHAR* repWord);
	virtual void my_ReplaceBlockWord(UCHAR* CurWord, const UCHAR* repWord);
	virtual int Popup(CEdit* pEdit, char* pWord, POINT& point, BOOL doubledWord= FALSE); 

private:
	virtual int BlockIsDoubledWord(const char* curWord); 

protected:
	virtual ReallyCheck(int LinesFromTop, BOOL autoCheck);
	BOOL IsPunctuation(char CurChar);

	int ParseLine(char* CurLin, char* CurWord, int LinLen, BOOL quoteLine = FALSE);

	BOOL m_bWasHighlighted;
	BOOL m_bInteractive; 
	long m_BegChar, m_EndChar;
	int m_BegRow, m_EndRow, m_BegCol, m_EndCol;
	int m_CurRow, m_CurCol;
	int	m_CurIndex;		// Number of characters checked in current line
	int	m_CurBufIndex;		
	int m_TopVisibleLine;
	BOOL m_DidTopScroll;
	CRichEditCtrl* m_hRichEdit;
	CEdit* m_hEdit;
/*04-27-01 changed the buffer size 	to fix crash
	UCHAR m_prevWord[30];*/
	UCHAR m_prevWord[SpellCheck_MAX_WORD_SZ];
	UCHAR m_termChar;

	BOOL	m_HasQuotedText;
public :
	BOOL m_bJustQueue;
};

#endif
