// SPELLPAIGE.H
//
#ifndef _SPELL_PAIGE_H_
#define _SPELL_PAIGE_H_

#include "spell.h"

#define MAX_WORD_SIZE 76

class CSpellDialog;

class CSpellPaige : CSpell
{
	CSpellDialog*	m_pSpellDialog;
	int				m_nSpellDialogRefCount;		// Keeps track of when we're in a call that uses the dialog (whether or not it's open)

public:
	CSpellPaige(BOOL hasQuotedText = FALSE);
	~CSpellPaige();

private:
	int ErrMsg(int ret);

public:
    int Check( CPaigeEdtView* pView, int LinesFromTop = 0, BOOL autoCheck = FALSE );
    int Check( select_pair* pTextRange = NULL, int LinesFromTop = 0, BOOL autoCheck = FALSE );

	void myHilightWord(BOOL bMark=FALSE);
	virtual void mySetRedraw(BOOL bRedraw=TRUE);
	int my_CheckBlock(long options, UCHAR* errWord, UCHAR* repWord);
	void my_ReplaceBlockWord(UCHAR* CurWord, const UCHAR* repWord);
	void MarkWordNotMisspelled(CPaigeEdtView * pView, const char * in_szWord);
	int Popup(CPaigeEdtView* pView, POINT& point, select_pair& sel);
    void Init( CPaigeEdtView* pView );
	void Cleanup();

	void			ResetIsRedrawNeeded() { m_bIsRedrawNeeded = false; }
	bool			IsRedrawNeeded() { return m_bIsRedrawNeeded; }

private:
	int BlockIsDoubledWord(const char* curWord); 

protected:
    int ReallyCheck( int LinesFromTop, BOOL autoCheck, select_pair* pTextRange = NULL, bool bInteractive = true );
	
	BOOL m_bWasHighlighted;
	long m_BegChar, m_EndChar;
	long m_CurPos;
	select_pair m_theSelection;
	
	int m_TopVisibleLine;
	BOOL m_DidTopScroll;
	pg_ref m_paigeRef;
	CPaigeEdtView* m_pView;
	CEdit* m_hRichEdit;
	CEdit* m_hEdit;
	UCHAR m_prevWord[MAX_WORD_SIZE];
	UCHAR m_termChar;

	BOOL	m_HasQuotedText;
	bool	m_bRedrawMisspelledWordsImmediately;
	bool	m_bIsRedrawNeeded;
};

#endif
