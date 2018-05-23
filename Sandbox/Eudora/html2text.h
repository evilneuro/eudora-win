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
	CString				m_szEudoraTag;
	CString				m_szAutoURL;

	CString				m_szA, m_szOffA;
	CString				m_szALIGN;
	CString				m_szALT;
	CString				m_szAMP;
	CString				m_szB, m_szOffB;
	CString				m_szBASE;
	CString				m_szBIG, m_szOffBIG;
	CString				m_szBLOCKQUOTE, m_szOffBLOCKQUOTE;
	CString				m_szBQ, m_szOffBQ;
	CString				m_szBR;
	CString				m_szCENTER, m_szOffCENTER;
	CString				m_szCOLOR;
	CString				m_szDD;	
	CString				m_szDIR, m_szOffDIR;
	CString				m_szDIV, m_szOffDIV;
	CString				m_szDL, m_szOffDL;
	CString				m_szDT;
	CString				m_szEM, m_szOffEM;
	CString				m_szFACE;
	CString				m_szFONT, m_szOffFONT;
	CString				m_szGT;
	CString				m_szH1, m_szOffH1;
	CString				m_szH2, m_szOffH2;
	CString				m_szH3, m_szOffH3;
	CString				m_szH4, m_szOffH4;
	CString				m_szH5, m_szOffH5;
	CString				m_szH6, m_szOffH6;
	CString				m_szHEAD, m_szOffHEAD;
	CString				m_szHR;
	CString				m_szHREF;
	CString				m_szHTML, m_szOffHTML;
	CString				m_szI, m_szOffI;
	CString				m_szIMG;
	CString				m_szLEFT;
	CString				m_szLI;
	CString				m_szLink;
	CString				m_szLT;
	CString				m_szNAME;
	CString				m_szNBSP;
	CString				m_szOL, m_szOffOL;
	CString				m_szP, m_szOffP;
	CString				m_szPARA, m_szOffPARA;
	CString				m_szPRE, m_szOffPRE;
	CString				m_szQUOT;
	CString				m_szRIGHT;
	CString				m_szSIZE;
	CString				m_szSMALL, m_szOffSMALL;
	CString				m_szSTRONG, m_szOffSTRONG;
	CString				m_szTABLE, m_szOffTABLE;
	CString				m_szTD;
	CString				m_szTH;
	CString				m_szTITLE, m_szOffTITLE;
	CString				m_szTR;
	CString				m_szU, m_szOffU;
	CString				m_szUL, m_szOffUL;
	CString				m_szXHTML, m_szOffXHTML;

	CString				m_szBLACK;
	CString				m_szSILVER;
	CString				m_szGRAY;
	CString				m_szWHITE;
	CString				m_szMAROON;
	CString				m_szRED;
	CString				m_szPURPLE;
	CString				m_szFUCHSIA;
	CString				m_szGREEN;
	CString				m_szLIME;
	CString				m_szOLIVE;
	CString				m_szYELLOW;
	CString				m_szNAVY;
	CString				m_szBLUE;
	CString				m_szTEAL;
	CString				m_szAQUA;
	
	CString             m_szAacuteCAP;
	CString             m_szAgraveCAP;
	CString             m_szAcircCAP;
	CString             m_szAtildeCAP;
	CString             m_szAringCAP;
	CString             m_szAumlCAP;
	CString             m_szAEligCAP;
	CString             m_szCcedilCAP;
	CString             m_szEacuteCAP;
	CString             m_szEgraveCAP;
	CString             m_szEcircCAP;
	CString             m_szEumlCAP;
	CString             m_szIacuteCAP;
	CString             m_szIgraveCAP;
	CString             m_szIcircCAP;
	CString             m_szIumlCAP;
	CString             m_szETHCAP;
	CString             m_szNtildeCAP;
	CString             m_szOacuteCAP;
	CString             m_szOgraveCAP;
	CString             m_szOcircCAP;
	CString             m_szOtildeCAP;
	CString             m_szOumlCAP;
	CString             m_szOslashCAP;
	CString             m_szUacuteCAP;
	CString             m_szUgraveCAP;
	CString             m_szUcircCAP;
	CString             m_szUumlCAP;
	CString             m_szYacuteCAP;
	CString             m_szThornCAP;
	
	CString             m_szligLOWER;

	CString             m_szAacuteLOWER;
	CString             m_szAgraveLOWER;
	CString             m_szAcircLOWER;
	CString             m_szAtildeLOWER;
	CString             m_szAringLOWER;
	CString             m_szAumlLOWER;
	CString             m_szAEligLOWER;
	CString             m_szCcedilLOWER;
	CString             m_szEacuteLOWER;
	CString             m_szEgraveLOWER;
	CString             m_szEcircLOWER;
	CString             m_szEumlLOWER;
	CString             m_szIacuteLOWER;
	CString             m_szIgraveLOWER;
	CString             m_szIcircLOWER;
	CString             m_szIumlLOWER;
	CString             m_szETHLOWER;
	CString             m_szNtildeLOWER;
	CString             m_szOacuteLOWER;
	CString             m_szOgraveLOWER;
	CString             m_szOcircLOWER;
	CString             m_szOtildeLOWER;
	CString             m_szOumlLOWER;
	CString             m_szOslashLOWER;
	CString             m_szUacuteLOWER;
	CString             m_szUgraveLOWER;
	CString             m_szUcircLOWER;
	CString             m_szUumlLOWER;
	CString             m_szYacuteLOWER;
	CString             m_szThornLOWER;
	CString             m_szYumlLOWER;
	CString				m_szCopy;
	CString				m_szReg;
	CString				m_szTrade;
	CString				m_szShy;
	CString				m_szCBSP;


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
