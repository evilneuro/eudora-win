/////////////////////////////////////////////////////////////////////////////
// CalTitle.h : header file for the SECCalendar's title panel class
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

#ifndef __SECCALTITLE_H__
#define __SECCALTITLE_H__

#ifndef __SECPANEL_H__
#include "panel.h"
#endif //__SECPANL_H__

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


#define SEC_CALTITLE_SCHEMA 0x0100


class SECCalTitle : public SECPanel
{
	DECLARE_SERIAL( SECCalTitle ) 
// Construction
public:
	SECCalTitle();
	BOOL Create( CRect r,
		     CWnd *pParent,
		     DWORD dwStyle,
		     UINT nID,
		     BOOL bPressed, 
		     BOOL bHighlighted, 
		     long iDrawMode,
		     long iBehaviorMode,
		     UINT nBevelLines );
 
// Attributes
public:
	virtual CRect SetContainerRect( CRect &rect );

// Operations
public:
#ifdef WIN32
	virtual BOOL SetPage( const COleDateTime &date );
#else
	virtual BOOL SetPage( const CTime        &date );
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECCalTitle)
	//}}AFX_VIRTUAL
	virtual BOOL OnDraw( CDC *pDC );

// Implementation
public:
	virtual ~SECCalTitle();
	CSize RectSizeFromFont( CDC *pDC, SECCalendar* pWnd, LOGFONT *pLf = NULL );
	LOGFONT FontSizeFromRect( CDC *pDC, 
							  LOGFONT *pLf = NULL,
							  CRect *pR = NULL );

protected:
	void SizeButtons( void );
	SECCalendar *m_pParent;

	CButton	m_btnPrevYear;
	CButton	m_btnPrevMonth;
	CButton	m_btnNextMonth;
	CButton	m_btnNextYear;

	BOOL m_bYearButtons;
	BOOL m_bMonthButtons;

	void OnNextYear();
	void OnPrevYear();
	void OnNextMonth();
	void OnPrevMonth();
};

/////////////////////////////////////////////////////////////////////////////

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECCALTITLE_H__
