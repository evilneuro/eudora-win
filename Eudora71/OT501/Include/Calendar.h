/////////////////////////////////////////////////////////////////////////////
// Calendar.h : header file for the SECCalendar custom control
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

// Don's changes:
//  (1) Added SEC_PCS_ALIGNBOTTOMLEFT
//  (2) Added GetSelectedDateValue
//  (3) Removed & from date/time passed into SelectDate

#ifndef __SEC_CALENDAR_H__
#define __SEC_CALENDAR_H__

#ifndef __SEC_CALTITLE_H__
#include "CalTitle.h"
#endif //__SEC_CALTITLE_H__

#ifndef __SEC_DAYBAR_H__
#include "DayBar.h"
#endif //__SEC_DAYBAR_H__

#ifndef __SEC_DAYBOX_H__
#include "DayBox.h"
#endif //__SEC_DAYBOX_H__

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

#define SEC_CAL_SCHEMA					0x0100

class SECCalendar;


// Behavior options. High 16 bits describe overall calendar modes,
// low 16 bits describe component behavior modes.
#define SECBM_AUTOSIZE_FONT				0x00010000L
#define SECBM_AUTOSIZE_RECT				0x00020000L 
#define SECBM_SINGLE_DATE				0x00040000L
#define SECBM_MONTH_BUTTONS				0x00080000L
#define SECBM_YEAR_BUTTONS				0x00100000L
#define SECBM_KEYBOARD_CONTROL			0x00200000L
#define SECBM_MONTH_FROZEN				0x00400000L
#define SECBM_AUTO_HIDE					SECPBM_AUTOCLOSEABLE
#define SECBM_LBUTTONTRACK				0x01000000L

// Not yet implemented.
#define SECBM_RESIZEABLE				0x00800000L

// Draw options. High 16 bits describe overall calendar modes,
// low 16 bits describe component behavior modes.
#define SECDM_FULL_MONTH_NAMES			0x10000000L
#define SECDM_FULL_DAY_NAMES			0x20000000L
  
// Styles for SECPopupCalendar
#define SEC_PCS_ALIGNBOTTOMLEFT			0x0001		// Align calendar at bottom
													// left of supplied rect.

// Calendar-control-to-parent messages.
#define SEC_DATESELECTED				(WM_USER+1)
#define SEC_HIGHLIGHTLISTCHANGED		(WM_USER+2)
#define SEC_OK							(WM_USER+3)
#define SEC_CANCEL						(WM_USER+4)

// Defaults
#define DEFAULT_DAY_BKGD_COLOR			COLOR_WINDOW	
#define DEFAULT_DAY_PRE_POST_BKGD_COLOR	COLOR_BTNSHADOW			
#define DEFAULT_DAYBAR_BKGD_COLOR		COLOR_BTNFACE
#define DEFAULT_DAYBAR_FGD_COLOR		COLOR_BTNTEXT
#define DEFAULT_TITLE_BKGD_COLOR		COLOR_ACTIVECAPTION
#define DEFAULT_TITLE_FGD_COLOR			COLOR_CAPTIONTEXT
#define DEFAULT_LINES_COLOR				COLOR_BTNTEXT
#define DEFAULT_SELECTED_COLOR			COLOR_BKGD
#define DEFAULT_HIGHLIGHTED_COLOR		COLOR_HIGHLIGHT
#define DEFAULT_TEXT_COLOR				COLOR_WINDOWTEXT
#define DEFAULT_GRAYTEXT_COLOR			COLOR_GRAYTEXT

#define SECBM_DEFAULT_VIEW_BEHAVIOR \
			SECPBM_DEFAULT_VIEW_BEHAVIOR | SECBM_AUTOSIZE_FONT | \
			SECBM_YEAR_BUTTONS |SECBM_MONTH_BUTTONS | SECBM_KEYBOARD_CONTROL 

#define SECBM_DEFAULT_DIALOG_BEHAVIOR SECPBM_DEFAULT_DIALOG_BEHAVIOR | \
		SECBM_AUTOSIZE_FONT | SECBM_YEAR_BUTTONS | SECBM_MONTH_BUTTONS | \
		SECBM_KEYBOARD_CONTROL 
									  
#define SECBM_DEFAULT_POPUP_BEHAVIOR SECPBM_DEFAULT_DIALOG_BEHAVIOR | \
			SECBM_AUTOSIZE_FONT | SECBM_YEAR_BUTTONS | SECBM_MONTH_BUTTONS | \
			SECBM_KEYBOARD_CONTROL | SECBM_AUTO_HIDE

#define SECDM_DEFAULT_VIEW_DRAW SECPDM_DEFAULT_VIEW_DRAW | \
			SECDM_FULL_MONTH_NAMES | SECDM_FULL_DAY_NAMES
#define SECDM_DEFAULT_DIALOG_DRAW SECPDM_DEFAULT_DIALOG_DRAW | \
			SECDM_FULL_MONTH_NAMES
#define SECDM_DEFAULT_POPUP_DRAW SECPDM_DEFAULT_POPUP_DRAW

#define SECDM_DEFAULT_VIEW_PPDRAW SECPDM_DEFAULT_VIEW_PPDRAW
#define SECDM_DEFAULT_DIALOG_PPDRAW SECPDM_DEFAULT_DIALOG_PPDRAW
#define SECDM_DEFAULT_POPUP_PPDRAW SECPDM_DEFAULT_POPUP_PPDRAW



#define SECC_NUMDAYS					42				/* 7 days x 6 wks. */
#define SECC_DEFAULT_FONT				_T("Arial")

// Geometry of the Calendar
#define VD_TOP							0.000
#define VD_TITLE						0.175
#define	VD_WK_0							0.250
#define VD_WK_ARRAY						{ 0.250,0.375,0.500,0.625,\
												0.750,0.875,1.000}

#define HD_LEFT							0.000
#define HD_TITLE						1.000
#define HD_DAYBAR						1.000
#define HD_DAY_ARRAY					{0.000, 0.143, 0.286, 0.429,\
										 0.572, 0.715, 0.858, 1.000}

/////////////////////////////////////////////////////////////////////////////
// SECCalendar:
// See Calendar.cpp for the implementation of this class
//

class SECCalendar : public CWnd
{
	DECLARE_SERIAL( SECCalendar ) 
// Implementation

	//{{AFX_MSG(SECCalendar)
	afx_msg void OnPaint();
	afx_msg LONG OnOK(UINT u, long l);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg LONG OnCancel(UINT u, long l);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNextYear(); 
	afx_msg void OnPrevYear(); 
	afx_msg void OnNextMonth();
	afx_msg void OnPrevMonth();
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Construction
public:
	SECCalendar( );
	virtual BOOL Create(	DWORD dwStyle, 
							CRect &Rect, 
							CWnd *pWndParent, 
							UINT nID,
							CCreateContext* pContext = NULL );


// Attributes
public:
	long GetPrePostDrawMode( void );
	BOOL SetPrePostDrawMode( long l );
	long GetBehaviorMode( void );
	BOOL SetBehaviorMode( long m );
	BOOL FAR SetDrawMode( long m );
	long GetDrawMode( void );
#ifdef WIN32
	BOOL SetPage( const COleDateTime &calendarPage, BOOL bRedraw = FALSE );
	const COleDateTime& GetSelectedDateValue( void );
	UINT GetAllHighlightedDates( COleDateTime *pDates, UINT nDates );
#else
	const CTime& GetSelectedDateValue( void );
#endif
	UINT GetAllHighlightedDates( CTime *pDates, UINT nDates );
	BOOL SetPage( const CTime &calendarPage, BOOL bRedraw = FALSE );
	BOOL MarkDate( const CTime &date, BOOL bSelectIt );
	SECDayBox *GetSelectedDate( void );
	UINT GetNumHighlightedDates( void );
	BOOL GetAutosizeFont( void );
	BOOL GetAutosizeRect( void );
	void SetAutosizeRect( BOOL bSet );
	void SetAutosizeFont( BOOL bSet );
	const CRect	&GetControlRect( void );
	CRect SetControlRect( CRect &newRect );
	BOOL GetSingleDate( void );
	BOOL SetSingleDate( BOOL bTurnOn );
	BOOL GetMonthButtons( void );
	BOOL SetMonthButtons( BOOL bTurnOn );
	BOOL GetYearButtons( void );
	BOOL SetYearButtons( BOOL bTurnOn );
	BOOL GetKeyboardControl( void );
	BOOL SetKeyboardControl( BOOL bTurnOn );
	BOOL GetMonthFrozen( void );
	BOOL SetMonthFrozen( BOOL bMakeFrozen );
	BOOL GetAutoHide( void );
	BOOL SetAutoHide( BOOL bHideAutomatically );
	UINT GetAlignmentFlag( void );
	UINT SetAlignmentFlag( UINT lNewFlag );
	CWnd *GetNotifyWnd( void );
	void SetNotifyWnd( CWnd *pWnd );
	COLORREF SetTextColor( COLORREF newColor );
	COLORREF GetTextColor( void );
	void SetFirstDayOfWeek(int nNewFirstDay, BOOL bRedraw = FALSE);
	int GetFirstDayOfWeek() const;


// Operations
public:
	virtual BOOL AdvanceDay( BOOL bSelectionFollows = TRUE );
	virtual BOOL RetreatDay(  BOOL bSelectionFollows = TRUE );
	virtual BOOL AdvanceWeek( BOOL bSelectionFollows = TRUE );
	virtual BOOL RetreatWeek(  BOOL bSelectionFollows = TRUE );
	virtual BOOL AdvanceMonth( BOOL bSelectionFollows = FALSE );
	virtual BOOL RetreatMonth(  BOOL bSelectionFollows = FALSE );
	virtual BOOL AdvanceYear(  BOOL bSelectionFollows = FALSE );
	virtual BOOL RetreatYear(  BOOL bSelectionFollows = FALSE );
#ifdef WIN32
	virtual BOOL DeltaSelection( COleDateTimeSpan &ts );
	virtual BOOL DeltaPage( COleDateTimeSpan &ts );
	virtual	BOOL HighlightDate( COleDateTime &date, BOOL bSet );
	virtual	BOOL SelectDate( COleDateTime date, BOOL bSet );
	virtual	BOOL ToggleSelectDate( COleDateTime &date );
	virtual	BOOL ToggleHighlightDate( COleDateTime &date );
#endif
	virtual BOOL DeltaSelection( CTimeSpan &ts );
	virtual BOOL DeltaPage( CTimeSpan &ts );
	virtual	BOOL HighlightDate( CTime &date, BOOL bSet );
	virtual	BOOL SelectDate( CTime date, BOOL bSet );
	virtual	BOOL ToggleSelectDate( CTime &date );
	virtual	BOOL ToggleHighlightDate( CTime &date );
	void Serialize( CArchive &ar );

protected:
	void PurgeDateList( void );
	virtual BOOL IsDisplayableDate( int year, int month, int day );


// Overrideables
public:
	virtual void InitColors();

// Implementation
public:
	virtual	~SECCalendar( );
	void TextCenteredInRect( CDC *pDC, 
							 CRect theRect, 
							 COLORREF clrFgnd, 
							 COLORREF clrBkgd,
							 CString& strText);

	SECDayBar m_pnlDayBar;
	SECCalTitle m_pnlTitle;

protected:
#ifdef WIN32
	void NormalizeDate( COleDateTime &date );
	BOOL DateFoundInList( COleDateTime date );
	BOOL RemoveDateFromList( COleDateTime date );
	BOOL AddDateToList( COleDateTime date );
#else
	void NormalizeDate( CTime &date );
	BOOL DateFoundInList( CTime date );
	BOOL RemoveDateFromList( CTime date );
	BOOL AddDateToList( CTime date );
#endif
	virtual void OnDatePegged( void );
	UINT m_nBevelLines;
	virtual UINT GetClassStyle( void );
	CWnd *m_pNotifyWnd;
	void AdjustFontSizes( CDC *pDC = NULL );
	void AdjustRectSizes( CRect &r, CDC *pDC = NULL );
	CWnd *m_pParent;
	void SizeTitle( void );
	void SizeDayBar( void );
	void SizeDates( void );
	void InitFonts( void );
	void SetUpDateBoxes( void );
	void PaintDates( CDC *pDC );
	enum deltaDate { negYear, negMonth, negDay, Day, Month, Year };

protected:
	BOOL m_bCreated;
#ifdef _DEBUG
	void DumpDateList( void );
#endif

#ifdef WIN32
	COleDateTime m_page;
	COleDateTime m_selection;
	COleDateTime *m_pSelection;
	COleDateTime m_phantomSelection;
#else
	CTime        m_page;
	CTime        m_selection;
	CTime        *m_pSelection;
	CTime        m_phantomSelection;
#endif

	SECDayBox m_day[SECC_NUMDAYS];
	SECDayBox *m_pSelectedDay;

	UINT	m_nOpMode;
	UINT	m_nAlignment;

	BOOL	m_bFullMonthNames;
	BOOL	m_bFullDayNames;

	BOOL	m_bCapture;
	BOOL    m_bDaySaved;
#ifdef WIN32
	COleDateTime m_SaveSelectedDay;
#else
	CTime        m_SaveSelectedDay;
#endif

	CObList m_HltDateList;

	LOGFONT	m_lfTitleFont;
	LOGFONT	m_lfDayFont;
	LOGFONT	m_lfDateFont;
	CFont   m_TitleFont;
	CFont	m_DayFont;
	CFont   m_DateFont;

public:
	CFont* GetTitleFont() { return &m_TitleFont; }
	CFont* GetDayBarFont() { return &m_DayFont; }
	CFont* GetDateFont() { return &m_DateFont; }

protected:
	COLORREF m_clrText;
	COLORREF m_clrLines;
	COLORREF m_clrDayBarText;
	COLORREF m_clrDayBarFace;
	COLORREF m_clrTitleText;
	COLORREF m_clrTitleFace;
	COLORREF m_clrGrayText;

	long m_iPrePostDrawMode;
	long m_iDrawMode;
	long m_iPrePostBehaMode;
	long m_iBehaMode;
};


/////////////////////////////////////////////////////////////////////////////
// SECPopupCalendar window

class SECPopupCalendar : public SECCalendar
{

	DECLARE_SERIAL(SECPopupCalendar);
	
	//{{AFX_MSG(SECPopupCalendar)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Construction
public:
	SECPopupCalendar( );
	virtual BOOL Create(DWORD dwStyle, 
						CRect &buttonRect, 
						CWnd *pParentWnd, 
						CCreateContext *pContext = NULL);

// Attributes
public:
	long SetHideMode( long okOrCancel ); // IDOK or IDCANCEL
	virtual UINT GetClassStyle( void );

// Operations
public:
	void Serialize( CArchive &ar );


// Overridables
public:

// Implementation
public:
	~SECPopupCalendar( );

protected:
	BOOL m_bHideDoesOK;
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
#endif // __SEC_CALENDAR_H__   
