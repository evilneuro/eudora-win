#ifndef STRIPRICH_H
#define STRIPRICH_H

#define MIN_COLUMN	20

class CStripRichString : public CString
{
public:
	CStripRichString(UINT uExcerptLevel = 0, BOOL bNoFill = FALSE);

	const CString& Strip(LPCTSTR pBuf, UINT uExcerptLevel = 0, BOOL bNoFill = FALSE);

protected:
	UINT	m_uWWColumn;
	UINT	m_uWWMax;
	UINT	m_uExcerptLevel;
	BOOL	m_bNoFill;
	UINT	m_uLastLineIndex;
	UINT	m_uLastLineLength;
	INT		m_iLastSpacePos;
	CString	m_szExcerpt;

	static CString	m_szExcerptPrefix;
	static CString	m_szExcerptStart;
	static CString	m_szExcerptEnd;
	static CString	m_szPARAM_ON;
	static CString	m_szEXCERPT_ON;
	static CString	m_szPARAGRAPH_ON;
	static CString	m_szNOFILL_ON;
	static CString	m_szPARAM_OFF;
	static CString	m_szEXCERPT_OFF;
	static CString	m_szPARAGRAPH_OFF;
	static CString	m_szNOFILL_OFF;


	void Append(LPCSTR pBuf, ULONG ulLength);
	void SetExcerptLevel(UINT uExcerptLevel);
};

#endif
