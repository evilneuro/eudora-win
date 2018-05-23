/////////////////////////////////////////////////////////////////////////////
// DayBox.h : header file for the SECDayBox component of the SECCalendar 
//            custom control.
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

#ifndef __SEC_DAYBOX_H__
#define __SEC_DAYBOX_H__

#ifndef __SEC_PANEL_H__
#include "Panel.h"
#endif

#ifndef __AFXDISP_H__
#include "afxdisp.h"
#endif

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


#define SEC_DAY_SCHEMA					0x0100

class SECDayBox;
class SECCalendar;



/////////////////////////////////////////////////////////////////////////////
// SECDayBox:
// See SECDayBox.cpp for the implementation of this class
//

class SECDayBox	: public SECPanel
{
	DECLARE_SERIAL( SECDayBox ) 
// Construction
public:
	SECDayBox( );
	BOOL Create( CRect rect,
				 SECCalendar *pParent,
				 UINT nID,
 				 DWORD dwStyle = WS_CHILD|WS_VISIBLE,
				 BOOL bSelected = FALSE,
				 BOOL bHighlighted = FALSE,
				 long iDrawMode = SECPDM_DEFAULT_DIALOG_DRAW,
				 long iBehaviorMode = SECPBM_DEFAULT_DIALOG_BEHAVIOR,
				 UINT nBevelLines = SECP_DEFAULTBEVELLINES );
		
// Attributes
public:
#ifdef WIN32
	COleDateTime SetDate( COleDateTime newDate );
	const COleDateTime &GetDate( void );
#else
	CTime SetDate( CTime newDate );
	const CTime &GetDate( void );
#endif
	
	BOOL SetPrePostState( BOOL bToWhat );
	BOOL GetPrePostState( void );

// Operations
public:

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECDayBox)
	//}}AFX_VIRTUAL
 	virtual BOOL OnDrawLabel( CDC *pDC  );

// Implementation
public:
	virtual ~SECDayBox( );
	CSize GetBorderSize();
	CSize RectSizeFromFont( CDC *pDC, SECCalendar* pWnd, LOGFONT *pLf = NULL );
	LOGFONT	FontSizeFromRect( CDC *pDC, 
							 LOGFONT *pLf = NULL, 
							 CRect *pR = NULL );
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnLButtonDblClk(UINT nFlags, CPoint point);


protected:
	UINT m_ordinal;
#ifdef WIN32
	COleDateTime m_today;
#else
	CTime        m_today;
#endif
	BOOL m_bPrePost;
	BOOL m_bDateInitted;

};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SEC_DAYBOX_H__

