/////////////////////////////////////////////////////////////////////////////
// DTCtrl.h : Header file for SECDateTimeCtrl. A date and/or time editting
//            control.
//
// Stingray Software Extension Classes
// Copyright (C) 1996-7 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
//		Author:		Don			20/01/97
//

#ifndef __SECDATET_H__
#define __SECDATET_H__

#ifdef WIN32

#ifndef __DTGADGET_H__
#include "DTGadget.h"
#endif

#ifndef __AFXDISP_H__
#include "afxdisp.h"
#endif

// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


// SECDateTimeCtrl styles
#define SEC_DTS_CALENDAR	0x0001L
#define SEC_DTS_UPDOWN		0x0002L

// SECDateTimeCtrl notifications
#define SECDTN_CHANGED		0x0100

// SECDateTimeCtrl Gadget IDs (bit settings)
#define SECDTID_12HOUR		0x0001
#define SECDTID_24HOUR		0x0002
#define SECDTID_MINUTE		0x0004
#define SECDTID_SECOND		0x0008
#define SECDTID_AMPM		0x0010
#define SECDTID_DAYNO		0x0020
#define SECDTID_DAYNAME		0x0040
#define SECDTID_MONTHNO		0x0080
#define SECDTID_MONTHNAME	0x0100
#define SECDTID_YEAR2		0x0200
#define SECDTID_YEAR4		0x0400
#define SECDTID_CALDROPDOWN	0x4000
#define SECDTID_SPINNER		0x8000
#define SECDTID_ALL			0xffff


// Forward references
class SECPopupCalendar;

/////////////////////////////////////////////////////////////////////////////
// SECDateTimeCtrl window
//
class SECDateTimeCtrl : public CWnd
{
	DECLARE_DYNAMIC(SECDateTimeCtrl)

// Construction
public:
	SECDateTimeCtrl();

	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, 
				CCreateContext* pContext = NULL);
	BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, 
				  CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	BOOL AttachDateTimeCtrl( UINT uID, CWnd* pParent, DWORD dwDTStyles, 
		                     DWORD dwWinStyles = WS_CHILD|WS_TABSTOP, 
							 DWORD dwExStyle = WS_EX_CLIENTEDGE );
// Attributes
public:
	// Control format type as used by Get/SetFormat.
	enum FormatType
	{
		Time,							// Locale time format
		ShortDate,						// Locale short date format 
		LongDate,						// Locale long date format
		Custom							// A user supplied date/time format 
	};									// string

	// The validation types as used by Get/SetMinMax
	enum ValidationMode
	{
		NoValidation,					// No validation is required
		TimeValidation,					// Validation of time fields only
		DateValidation,					// Validation of date fields only
		DateTimeValidation				// Validation of date & time fields
	};

	// Definitions of non-editable fields as used by Get/SetNoEdit
	enum NoEdit
	{
		FixedYear   = 0x0001,			// The year is non-editable
		FixedMonth  = 0x0002,			// The month is non-editable
		FixedDay    = 0x0004,			// The day is non-editable
		FixedHour   = 0x0008,			// The hour is non-editable
		FixedMinute = 0x0010,			// The minute is non-editable
		FixedSecond = 0x0020,			// The second is non-editable
		FixedDate   = 0x0007,			// The date is non-editable
		FixedTime   = 0x0030			// The time is non-editable
	};

// Operations
public:
	// Forcing a given gadget to be redrawn
	void InvalidateGadget(SECDTGadget* pGadget, BOOL bImmediateRedraw = FALSE);
	void InvalidateGadget(int nIndex, BOOL bImmediateRedraw = FALSE);

	// Return the index of the gadget at the given point
	int  GadgetFromPoint(CPoint point);

	// Bring a given gadget into the displayable portion of the window
	void BringIntoView(SECDTGadget* pGadget);
	void BringIntoView(int nIndex);

	// Set/Get the current date/time format
	void SetFormat(FormatType format);
	void SetFormat(LPCTSTR lpszFormat);
	void GetFormat(FormatType& format) const;
	void GetFormat(CString& format) const;

	// Functions for enabling fast input. With fast input enabled, when a 
	// numeric field has received a valid input (it's maximum number of
	// digits), it advances input focus to the next field.
	void EnableFastEntry(BOOL bEnable = TRUE);
	BOOL IsFastEntryEnabled() const;

	// Force the control to re-size to fit it's contents exactly
	void SizeToContent();

	// Set/Determine the current modified state.
	void SetModified(BOOL bModified);
	void SetModified(BOOL bDate, BOOL bTime);
	BOOL IsModified() const;
	void IsModified(BOOL& bDate, BOOL& bTime) const;

	// Returns TRUE if the user is currently in the middle of editting one
	// of the numeric fields
	BOOL IsEditing() const;

	// Set/Get the current date and time
	BOOL SetDateTime(const COleDateTime& datetime);
	BOOL SetDate(const COleDateTime& date);
	BOOL SetTime(const COleDateTime& time);
	const COleDateTime& GetDateTime() const;

	// Set/Get the current validation mode and the ranges being validated
	// to.
	BOOL SetMinMax(const COleDateTime& minDateTime, 
				   const COleDateTime& maxDateTime, 
				   ValidationMode mode = DateTimeValidation);
	ValidationMode GetMinMax(COleDateTime& minDateTime, 
							 COleDateTime& maxDateTime) const;

	// Validates the given date/time to the current ranges
	BOOL Validate(const COleDateTime& datetime) const;

	// Set/Get the edittable fields.
	int  GetNoEdit() const;
	void SetNoEdit(int nNoEdit);

	// Enter NULL date mode.
	void SetNull(TCHAR cNull = _T('_'));
	TCHAR GetNullChar() const { return m_cNull; }

// Overrides
	// Function called just before a commit of a new date/time is applied.
	// Return FALSE to abandon the commit.
	virtual BOOL OnChanging(const COleDateTime& newDateTime);

	// Function called after the date/time has been modified. The default
	// implementation passes a SECDTN_CHANGED notification to the parent.
	virtual void OnChanged();

// Implementation
public:
	virtual ~SECDateTimeCtrl();

protected:
	// Function responsible for parsing the date/time format string and 
	// building the m_gadgets component list.
	void ParseDateTimeFormat();

	// Functions used by ParseDateTimeFormat for parsing individual components
	// of the date/time string
	void ParseHours(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseMinutes(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseSeconds(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseAmPm(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseDay(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseMonth(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseYear(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseEra(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	void ParseQuotedText(LPCTSTR& lpszStart, LPCTSTR& lpsz, UINT uParam);
	BOOL CheckLeadZero(LPCTSTR& lpsz, TCHAR c) const;
	void FlushStatic(LPCTSTR lpszStart, LPCTSTR lpszEnd);

	// Functions used for changing the value of a speicific field 
	BOOL ChangeHour(UINT uNotifyCode, int nID, LPARAM lParam);
	BOOL ChangeMinute(UINT uNotifyCode, int nID, LPARAM lParam);
	BOOL ChangeSecond(UINT uNotifyCode, int nID, LPARAM lParam);
	BOOL ChangeAmPm(UINT uNotifyCode, int nID, LPARAM lParam);
	BOOL ChangeDay(UINT uNotifyCode, int nID, LPARAM lParam);
	BOOL ChangeMonth(UINT uNotifyCode, int nID, LPARAM lParam);
	BOOL ChangeYear(UINT uNotifyCode, int nID, LPARAM lParam);

	// Functions for moving forward/backwards a gadget
	void AdvanceGadget();
	void RetreatGadget();

	// Function used to layout the gadgets within the controls client
	virtual void LayoutGadgets();

	// Function used for controlling the drawing of the gadgets.
	virtual void Draw(CDC& dc);

	// Handler for spinner events
	BOOL SpinnerEvent(UINT uNotifyCode, int nID, LPARAM lParam);

	// Handler for creating the popup calendar
	BOOL PopupCalendar(UINT uNotifyCode, int nID, LPARAM lParam);

	// Functions for updating the specified gadgets with new values and
	// for forcing them to be redisplayed
	void UpdateGadgets(const COleDateTime& datetime);
	void UpdateGadgets(UINT nFlags = SECDTID_ALL);

	// Performs validation of a new date (and can correct certain
	// fields to make it valid)
	virtual BOOL Validate(COleDateTime& datetime, int nPos) const;

	// Handles notify events from gadgets
	virtual BOOL OnNotify(UINT uNotifyCode, int nID, LPARAM lParam);

	// Handles requests from gadgets to capture the mouse.
	void CaptureMouse(SECDTGadget* pGadget, BOOL bSet);

	// Functions for causing the gadget display to be re-built.
	void Tidy();
	void ResetGadgets();
	void ResetCurrentGadget( int nCurGadget );
	BOOL CheckGadgetFocusAbility( int nGadget );
	int  GetFirstFocusGadget();

	// Returns fields which are present (used for NULL dates)
	WORD GetNullFields() const;

	// Checks whether we can leave a NULL edit state.
	void CheckNull();

protected:
// Implementation - data members
	enum								// Internal state flags
	{
		DateModified = 0x0001,			// The date has been modified
		TimeModified = 0x0002,			// The time has been modified
		Editing      = 0x0004			// A numeric field is currently being
	};									// editted.

	int               m_nState;			// Holds state information (see flags)
	int               m_nNoEdit;		// Holds non-editable fields (see NoEdit)
	BOOL			  m_bFastInput;		// TRUE if fast input enabled
	COleDateTime      m_datetime;		// Current date/time
	COleDateTime      m_datetimeMin;	// Minimum date/time
	COleDateTime      m_datetimeMax;	// Maximum date/time
	ValidationMode    m_validMode;		// Validation mode (see ValidationMode)
	FormatType        m_formatType;		// Date/Time format type (see FormatType)
	CString           m_strCustomFormat;// Holds custom date/time format string
	HFONT             m_hFont;			// Font used by control
	int               m_nCurGadget;		// Current gadget (gadget with focus)
	int               m_nXOrigin;		// Current offset (scrolled percentage)
										// of window.
	int               m_nRightOffset;	// Amount of space on right reserved for
										// fixed gadgets
	int               m_nFixed;			// No. of fixed gadgets.
	SECDTGadget*      m_pCapture;		// Ptr to gadget which has mouse capture
	SECPopupCalendar* m_pPopupCalendar;	// Handle of popup calendar 
										// (normally NULL)
	TCHAR             m_cNull;			// Char. used to display NULL gadgets.
	
	// SECDTGadgetArray would normally be implemented as a template 
	// (CTypedPtrArray). Since templates are not implemented by some Unix
	// compilers, we copy what the template would have done.
	class SECDTGadgetArray : public CObArray
	{
	public:
		// Accessing elements
		SECDTGadget* GetAt(int nIndex) const
			{ return (SECDTGadget*)CObArray::GetAt(nIndex); }
		SECDTGadget*& ElementAt(int nIndex)
			{ return (SECDTGadget*&)CObArray::ElementAt(nIndex); }
		void SetAt(int nIndex, SECDTGadget* ptr)
			{ CObArray::SetAt(nIndex, ptr); }

		// Potentially growing the array
		void SetAtGrow(int nIndex, SECDTGadget* newElement)
		   { CObArray::SetAtGrow(nIndex, newElement); }
		int Add(SECDTGadget* newElement)
		   { return CObArray::Add(newElement); }
		int Append(const SECDTGadgetArray& src)
		   { return CObArray::Append(src); }
		void Copy(const SECDTGadgetArray& src)
			{ CObArray::Copy(src); }

		// Operations that move elements around
		void InsertAt(int nIndex, SECDTGadget* newElement, int nCount = 1)
			{ CObArray::InsertAt(nIndex, newElement, nCount); }
		void InsertAt(int nStartIndex, SECDTGadgetArray* pNewArray)
		   { CObArray::InsertAt(nStartIndex, pNewArray); }

		// overloaded operator helpers
		SECDTGadget* operator[](int nIndex) const
			{ return (SECDTGadget*)CObArray::operator[](nIndex); }
		SECDTGadget*& operator[](int nIndex)
			{ return (SECDTGadget*&)CObArray::operator[](nIndex); }
	};

	// Object maintaining status for null dates.
	class SECNull
	{
	public:
		SECNull();

		// Enables Null edit mode
		void Set();

		// Sets the currently displayed fields
		void SetActualFields(WORD wActualFields);

		// Aborts null edit mode.
		void Clear();
		
		// Returns the bit settings for fields yet to be completed
		WORD GetFields() const { return WORD(m_wNullFields & m_wActualFields); }

		// Returns TRUE if we are currently in NULL date mode
		BOOL IsSet() const     { return m_bSet; }

		// Functions for setting/getting the various date/time fields.
		// Note that the get functions return -1 if the fields has yet to be
		// entered.
		int  GetHour() const;
		void SetHour(int nHour);
		int  GetMinute() const;
		void SetMinute(int nMinute);
		int  GetSecond() const;
		void SetSecond(int nSecond);
		int  GetYear() const;
		void SetYear(int nYear);
		int  GetMonth() const;
		void SetMonth(int nMonth);
		int  GetDay() const;
		void SetDay(int nDay);

		enum							// Bit settings for m_wNullFields
		{
			Hour   = 0x0001,			// Set if hour has yet to be entered
			Minute = 0x0002,			// Set if minute has yet to be entered
			Second = 0x0004,			// Set if second has yet to be entered
			Year   = 0x0008,			// Set if year has yet to be entered
			Month  = 0x0010,			// Set if month has yet to be entered
			Day    = 0x0020				// Set if day has yet to be entered
		};

	protected:
		WORD m_wNullFields;				// Bit settings for incomplete fields
		WORD m_wActualFields;			// Bit settings for fields present
		BOOL m_bSet;					// TRUE if date is currently null
		int  m_nHour;					// Holds hour while in null state
		int  m_nMinute;					// Holds minute while in null state
		int  m_nSecond;					// Holds second while in null state
		int  m_nYear;					// Holds year while in null state
		int  m_nMonth;					// Holds month while in null state
		int  m_nDay;					// Holds day while in null state
	} m_null;

	// m_gadgets is an array of all the gadget components.
	SECDTGadgetArray m_gadgets;

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg void OnCancelMode();
	afx_msg LONG OnSettingChange(UINT wParam, long lParam);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LONG OnSetFont(UINT wParam, long lParam);
	afx_msg void OnDestroy();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg LONG OnCalendarOK(UINT wParam, LONG lParam);
	afx_msg LONG OnCalendarCancel(UINT wParam, LONG lParam);

	// A very dear friend of mine ...
	friend class SECDTGadget;
};


/////////////////////////////////////////////////////////////////////////////
// Function for converting a DATE (base type of COleDateTime) to a tm structure
extern BOOL SECTmFromOleDate(DATE dtSrc, struct tm& tmDest);


/////////////////////////////////////////////////////////////////////////////
// Helper function for setting the date/time of a COleDateTime object
extern BOOL SECSetOleDateTime(COleDateTime& dt, int nYear, int nMonth, 
							  int nDay, int nHour, int nMinute, int nSecond);

/////////////////////////////////////////////////////////////////////////////
// inline functions
inline int SECDateTimeCtrl::SECNull::GetHour() const
{ return ((m_wNullFields & m_wActualFields) & Hour)   ? -1 : m_nHour;   }

inline int  SECDateTimeCtrl::SECNull::GetMinute() const 
{ return ((m_wNullFields & m_wActualFields) & Minute) ? -1 : m_nMinute; }

inline int SECDateTimeCtrl::SECNull::GetSecond() const 
{ return ((m_wNullFields & m_wActualFields) & Second) ? -1 : m_nSecond; }

inline int SECDateTimeCtrl::SECNull::GetYear() const   
{ return ((m_wNullFields & m_wActualFields) & Year)   ? -1 : m_nYear;   }

inline int SECDateTimeCtrl::SECNull::GetMonth() const  
{ return ((m_wNullFields & m_wActualFields) & Month)  ? -1 : m_nMonth;  }

inline int SECDateTimeCtrl::SECNull::GetDay() const    
{ return ((m_wNullFields & m_wActualFields) & Day)    ? -1 : m_nDay;    }


/////////////////////////////////////////////////////////////////////////////

#define ON_SECDTN_CHANGED(id, memberFxn) \
		ON_CONTROL(SECDTN_CHANGED, id, memberFxn)


/////////////////////////////////////////////////////////////////////////////

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif 	// WIN32

#endif

