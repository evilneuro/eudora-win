// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	SECSplashWnd (Splash Window) declaration
//  Created:		Mon Jun 29 21:18:15 1995
//
//  CHANGELOG: 
//    MSW     9/30/95   Code Review, repaired minor bugs.
//

#ifndef __SECSPLSH_H__
#define __SECSPLSH_H__

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

#define IDT_SPLASHTIMER 1245

class  SECSplashWnd : public CWnd
{
// Construction
public:
    SECSplashWnd(UINT nNewBitmapID, UINT nNewDuration = 2500);
    
// Attributes
public:
    virtual BOOL Create(CWnd* pParentWnd = NULL, 
		                DWORD dwStyle = WS_POPUP|WS_VISIBLE, 
						DWORD dwExStyle = WS_EX_TOPMOST);
	
// Operations
public:

// Implementation
public:
    virtual ~SECSplashWnd();
    virtual BOOL PreTranslateMessage(MSG * pMsg);

protected:
	BOOL CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors, CPalette *pPal);


protected:
    CBitmap m_bmSplash;
    int     m_nWndWidth;
    int     m_nWndHeight;
    UINT    m_nBitmapID;
    UINT    m_nSplashTimer;
    UINT    m_nSplashDuration;
	CPalette m_Pal; 
    
// Message map functions
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT nTimerID);
    
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

#endif //__SECSPLSH_H__
