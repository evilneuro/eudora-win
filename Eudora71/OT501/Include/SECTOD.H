// 
// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//
//  Author:		Scot Wingo
//  Description:	sectod.h - Definition of SECTipOfDay
//  Created:		July 25 1995
//
//  CHANGELOG:
//
//    MSW    9/30/95    Code Review
//
//
//

#ifndef __SECTOD_H__
#define __SECTOD_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#define _MAX_TIP 512

class SECTipOfDay : public CDialog
{
    DECLARE_DYNAMIC(SECTipOfDay)
    
// Construction
public:
    SECTipOfDay(const TCHAR * lpszTipFileName,
		const TCHAR * didYouKnow,
		int nTip = 1,
		BOOL bShowAtStart = 1,
		CWnd* pParent = NULL);
    virtual BOOL Create();
    
//Operations
    int     GetCurrentTip() {return m_nCurrentTip;}
    LPCTSTR  GetCurrentFileName() {return m_lpTipFileName;}
    
    BOOL    GetShowAtStartup() {return m_bShowState;}    
    void    SetCurrentTip(int nNewTip) {m_nCurrentTip = nNewTip;}
    void    SetTipIcon(UINT nNewIconID) {m_nIconID = nNewIconID;}    
    void    SetLeadInText(const TCHAR * lpNewLeadInText);
    void    SetLeadInFont(CFont * pNewLeadInFont) {m_pLeadInFont = pNewLeadInFont;}
    void    SetTipFont(CFont * pNewTipFont) {m_pTipFont = pNewTipFont;}
    
// Overrides
    virtual void OnHelp() {return;} //Place holder
    virtual int  OnInitDialog();
    
// Implementation
protected:
    BOOL    m_bShowState;
    int     m_nCurrentTip;
    int     m_nTipsInFile;
    const TCHAR *  m_lpTipFileName;
    TCHAR   m_szCurrentTip[_MAX_TIP];
    CRect   m_rectTip;
    
    //Customizeable values
    TCHAR  * m_lpLeadInText;
    CFont * m_pTipFont;
    CFont * m_pLeadInFont;
    UINT    m_nIconID;

    //Internal routines
    void LoadTip(int nTipToLoad);
    void RedrawTip();
    void CountTips();
public:
    virtual ~SECTipOfDay();
#ifdef _DEBUG
    void Dump(CDumpContext & dc) const;
    void AssertValid() const;
#endif
    // Message map functions
    afx_msg void OnNextTip();
    afx_msg void OnPreviousTips();
    afx_msg void OnPaint();
    afx_msg void OnShowButtonChanged();
    
    DECLARE_MESSAGE_MAP()
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif //__SECTOD_H__
