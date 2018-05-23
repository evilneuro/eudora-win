/////////////////////////////////////////////////////////////////////////////
// DayBar.h : header file for the SECCalendar's Day Bar class.
//
// Stingray Software Extension Classes
// Copyright (C) 1996 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG:
//
//    PCB	01/19/96	Started
//
//

#ifndef __DAYBAR_H__
#define __DAYBAR_H__

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

#include "Panel.h"

/////////////////////////////////////////////////////////////////////////////
// SECDayBar window

class SECDayBar : public SECPanel
{
// Construction
public:
	SECDayBar();

// Attributes
public:

// Operations
public:
	CSize RectSizeFromFont( CDC *pDC, SECCalendar* pWnd, LOGFONT *pLf = NULL );
	LOGFONT	FontSizeFromRect( CDC *pDC, 
							  LOGFONT *pLf = NULL, 
							  CRect *pR = NULL );

	virtual BOOL OnDrawLabel( CDC *pDC );
	void SetFirstDayOfWeek(int nFirstDay);
	int GetFirstDayOfWeek() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECDayBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECDayBar();

	int m_nFirstDayOfWeek;
};


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


/////////////////////////////////////////////////////////////////////////////
#endif // __DAYBAR_H__
