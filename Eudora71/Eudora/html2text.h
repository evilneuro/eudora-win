// Html2Text.h : header file
//
#ifndef _HTML2TEXT_H_
#define _HTML2TEXT_H_

/////////////////////////////////////////////////////////////////////////////
// CHtml2Text window


#include <afxtempl.h>

class CHtmlBlockInfo : public CObject
{
public:
	LONG				m_lIndent;
	
	CHtmlBlockInfo::CHtmlBlockInfo() 
	{
		m_lIndent = 0;
	}

	CHtmlBlockInfo::CHtmlBlockInfo( const CHtmlBlockInfo&	src ) 
	{
		*this = src;
	}

	CHtmlBlockInfo::~CHtmlBlockInfo()
	{
	}

	CHtmlBlockInfo&	operator = ( const CHtmlBlockInfo& src );
};

typedef CList<CHtmlBlockInfo, CHtmlBlockInfo&> CHtmlBlockInfoList;


////////////////////////////////////////////////////////////////////////////
// CHtml2Text

class CHtml2Text 
{
	static CString		s_szEudoraTag;
	static CString		s_szAutoURL;
	static CString		s_szSignature;

	static CString		s_szA, s_szOffA;
	static CString		s_szALIGN;
	static CString		s_szALT;
	static CString		s_szAMP;
	static CString		s_szB, s_szOffB;
	static CString		s_szBASE;
	static CString		s_szBIG, s_szOffBIG;
	static CString		s_szBLOCKQUOTE, s_szOffBLOCKQUOTE;
	static CString		s_szBQ, s_szOffBQ;
	static CString		s_szBR;
	static CString		s_szCENTER, s_szOffCENTER;
	static CString		s_szCOLOR;
	static CString		s_szDD;	
	static CString		s_szDIR, s_szOffDIR;
	static CString		s_szDIV, s_szOffDIV;
	static CString		s_szDL, s_szOffDL;
	static CString		s_szDT;
	static CString		s_szEM, s_szOffEM;
	static CString		s_szFACE;
	static CString		s_szFONT, s_szOffFONT;
	static CString		s_szGT;
	static CString		s_szH1, s_szOffH1;
	static CString		s_szH2, s_szOffH2;
	static CString		s_szH3, s_szOffH3;
	static CString		s_szH4, s_szOffH4;
	static CString		s_szH5, s_szOffH5;
	static CString		s_szH6, s_szOffH6;
	static CString		s_szHEAD, s_szOffHEAD;
	static CString		s_szHR;
	static CString		s_szHREF;
	static CString		s_szHTML, s_szOffHTML;
	static CString		s_szI, s_szOffI;
	static CString		s_szIMG;
	static CString		s_szLEFT;
	static CString		s_szLI;
	static CString		s_szLink;
	static CString		s_szLT;
	static CString		s_szNAME;
	static CString		s_szNBSP;
	static CString		s_szOL, s_szOffOL;
	static CString		s_szP, s_szOffP;
	static CString		s_szPARA, s_szOffPARA;
	static CString		s_szPRE, s_szOffPRE;
	static CString		s_szQUOT;
	static CString		s_szRIGHT;
	static CString		s_szSIZE;
	static CString		s_szSMALL, s_szOffSMALL;
	static CString		s_szSTRONG, s_szOffSTRONG;
	static CString		s_szTABLE, s_szOffTABLE;
	static CString		s_szTD;
	static CString		s_szTH;
	static CString		s_szTITLE, s_szOffTITLE;
	static CString		s_szTR;
	static CString		s_szU, s_szOffU;
	static CString		s_szUL, s_szOffUL;
	static CString		s_szXHTML, s_szOffXHTML;

	static CString		s_szBLACK;
	static CString		s_szSILVER;
	static CString		s_szGRAY;
	static CString		s_szWHITE;
	static CString		s_szMAROON;
	static CString		s_szRED;
	static CString		s_szPURPLE;
	static CString		s_szFUCHSIA;
	static CString		s_szGREEN;
	static CString		s_szLIME;
	static CString		s_szOLIVE;
	static CString		s_szYELLOW;
	static CString		s_szNAVY;
	static CString		s_szBLUE;
	static CString		s_szTEAL;
	static CString		s_szAQUA;
	
	static CString		s_szAacuteCAP;
	static CString		s_szAgraveCAP;
	static CString		s_szAcircCAP;
	static CString		s_szAtildeCAP;
	static CString		s_szAringCAP;
	static CString		s_szAumlCAP;
	static CString		s_szAEligCAP;
	static CString		s_szCcedilCAP;
	static CString		s_szEacuteCAP;
	static CString		s_szEgraveCAP;
	static CString		s_szEcircCAP;
	static CString		s_szEumlCAP;
	static CString		s_szIacuteCAP;
	static CString		s_szIgraveCAP;
	static CString		s_szIcircCAP;
	static CString		s_szIumlCAP;
	static CString		s_szETHCAP;
	static CString		s_szNtildeCAP;
	static CString		s_szOacuteCAP;
	static CString		s_szOgraveCAP;
	static CString		s_szOcircCAP;
	static CString		s_szOtildeCAP;
	static CString		s_szOumlCAP;
	static CString		s_szOslashCAP;
	static CString		s_szUacuteCAP;
	static CString		s_szUgraveCAP;
	static CString		s_szUcircCAP;
	static CString		s_szUumlCAP;
	static CString		s_szYacuteCAP;
	static CString		s_szThornCAP;
	
	static CString		s_szligLOWER;

	static CString		s_szAacuteLOWER;
	static CString		s_szAgraveLOWER;
	static CString		s_szAcircLOWER;
	static CString		s_szAtildeLOWER;
	static CString		s_szAringLOWER;
	static CString		s_szAumlLOWER;
	static CString		s_szAEligLOWER;
	static CString		s_szCcedilLOWER;
	static CString		s_szEacuteLOWER;
	static CString		s_szEgraveLOWER;
	static CString		s_szEcircLOWER;
	static CString		s_szEumlLOWER;
	static CString		s_szIacuteLOWER;
	static CString		s_szIgraveLOWER;
	static CString		s_szIcircLOWER;
	static CString		s_szIumlLOWER;
	static CString		s_szETHLOWER;
	static CString		s_szNtildeLOWER;
	static CString		s_szOacuteLOWER;
	static CString		s_szOgraveLOWER;
	static CString		s_szOcircLOWER;
	static CString		s_szOtildeLOWER;
	static CString		s_szOumlLOWER;
	static CString		s_szOslashLOWER;
	static CString		s_szUacuteLOWER;
	static CString		s_szUgraveLOWER;
	static CString		s_szUcircLOWER;
	static CString		s_szUumlLOWER;
	static CString		s_szYacuteLOWER;
	static CString		s_szThornLOWER;
	static CString		s_szYumlLOWER;
	static CString		s_szCopy;
	static CString		s_szReg;
	static CString		s_szTrade;
	static CString		s_szShy;
	static CString		s_szCBSP;


	void AddNewLine(UINT nCount, BOOL bEnsureFirst);
	void AddText(LPCTSTR szText, ULONG uLen);

	void GetAttributeList(LPCTSTR* pszText, CStringArray& theArray);

	void SaveBlockSettings();
	void SetBlockSettings(CStringArray& szAttributeList, LONG lRelativeIndex);
	void RestoreBlockSettings(BOOL bRemoveSettings = TRUE);

	void SetExcerptLevel(INT iInc);

	void NewAnchor(CStringArray& szAttributelist);

// Construction
public:
	CHtml2Text(BOOL bDoFormatFlowed);

public:
	CString	Html2Text(LPCTSTR text);
	
// Implementation
public:
	BOOL	LoadMyStrings();

protected:
	static bool			s_bStringsLoaded;
	CString				m_szDest;
	BOOL				m_bDoFormatFlowed;
	UINT				m_uExcerptLevel;
	BOOL				m_bTextOnLine;
	UINT				m_uInPre;
	LONG				m_lIndent;
	CHtmlBlockInfoList	m_BlockList;
};



CString	Html2Text(LPCTSTR szSource, BOOL bDoFormatFlowed = FALSE);

#endif 
