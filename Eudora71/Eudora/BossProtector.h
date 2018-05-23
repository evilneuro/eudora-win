// BOSSPROTECTOR.H
//
#ifndef _BOSSPROTECTOR_H_
#define _BOSSPROTECTOR_H_

class CBossProtector
{

public:
	CBossProtector();
	~CBossProtector();
	static void Build();

public:
	bool Check(CEdit* hEdit, int LinesFromTop, CStringList *pAddressList = NULL);
	static COLORREF GetBPColorCode();
		
private:
	int ParseLine(char* CurLin, char* CurWord, int LinLen);
	
	bool CheckBPAddress(const char* address, CStringList * pAddressList);
	void HilightWord();
	void SetRedraw(BOOL bRedraw=TRUE) {};
	bool CheckBlock(char* errWord, CStringList * pAddressList);
	int Popup(CEdit* pEdit, char* pWord, POINT& point, BOOL doubledWord= FALSE); 
	static void BuildDomainList(char* domains, CStringList &domainList);
	bool IsOnBPDomainList(const char* address);

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
	BOOL m_bComma;
	CEdit* m_hEdit;
	BOOL m_bWasHighlighted;
	//UCHAR m_termChar;
	static CStringList m_BPInsideDomainList;
	static CStringList m_BPOutsideDomainList;
	bool m_bHilightWord;
};

#endif
