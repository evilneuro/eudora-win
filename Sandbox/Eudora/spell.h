// SPELL.H
//
#ifndef _SPELL_H_
#define _SPELL_H_

class CSpellDialog;

class CSpell
{
	CSpellDialog*	m_pSpellDialog;

public:
	CSpell(BOOL hasQuotedText = FALSE);
	~CSpell();

private:
	int ErrMsg(int ret);

public:
#ifdef WIN32
	virtual Check(CRichEditCtrl* hEdit, int LinesFromTop = 0, BOOL autoCheck = FALSE);
#endif
	virtual Check(CEdit* hEdit, int LinesFromTop = 0, BOOL autoCheck = FALSE);
	

	virtual void myHilightWord(BOOL bMark=FALSE);
	virtual void mySetRedraw(BOOL bRedraw=TRUE) {};
	virtual int my_CheckBlock(long options, UCHAR* errWord, UCHAR* repWord);
	virtual void my_ReplaceBlockWord(UCHAR* CurWord, const UCHAR* repWord);

private:
	virtual int BlockIsDoubledWord(const char* curWord); 

protected:
	virtual ReallyCheck(int LinesFromTop, BOOL autoCheck);
	BOOL IsPunctuation(char CurChar);
	int ParseLine(char* CurLin, char* CurWord, int LinLen, BOOL quoteLine = FALSE);

	BOOL m_bWasHighlighted;
#ifdef WIN32
	long m_BegChar, m_EndChar;
#else
	int m_BegChar, m_EndChar;
#endif
	int m_BegRow, m_EndRow, m_BegCol, m_EndCol;
	int m_CurRow, m_CurCol;
	int	m_CurIndex;		// Number of characters checked in current line
	int	m_CurBufIndex;		
	int m_TopVisibleLine;
	BOOL m_DidTopScroll;
#ifdef WIN32
	CRichEditCtrl* m_hRichEdit;
#else
	CEdit* m_hRichEdit;
#endif
	CEdit* m_hEdit;
	UCHAR m_prevWord[30];
	UCHAR m_termChar;

	BOOL	m_HasQuotedText;
};

#endif
