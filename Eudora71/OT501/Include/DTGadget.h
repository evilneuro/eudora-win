/////////////////////////////////////////////////////////////////////////////
// DTGadget.h : Header file for SECDTGadget and associated classes. These
//              are used as display components by SECDateTimeCtrl
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


#ifndef __DTGADGET_H__
#define __DTGADGET_H__

#ifdef WIN32

// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


// Gadget notifications
#define SECGN_INCREMENT		0x0001
#define SECGN_DECREMENT		0x0002
#define SECGN_SPINUP		0x0003
#define SECGN_SPINDOWN		0x0004
#define SECGN_DROPDOWN		0x0005
#define SECGN_VALIDATE		0x0006
#define SECGN_MODIFY		0x0007
#define SECGN_MINIMUM		0x0008
#define SECGN_MAXIMUM		0x0009
#define SECGN_EDIT			0x000a
#define SECGN_NUMERIC_DONE	0x000b	// Input into numeric gadget is complete

// Forward references
class SECDateTimeCtrl;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  class SECDTGadget 
//  
//  Abstract base class for the gadget components used by SECDateTimeCtrl
//
class SECDTGadget : public CObject
{

DECLARE_DYNAMIC(SECDTGadget)
// Construction
public:
	SECDTGadget();

	// Initialises the gadget.
	BOOL Initialise(SECDateTimeCtrl* pCtrl, int nID = -1);

// Attributes
public:
	enum	// Style flags (for m_style and GetStyle/SetStyle)
	{
		WantFocus = 0x0001,		// Gadget accepts focus (i.e. can be enabled)
		WantMouse = 0x0002,		// Gadget wants mouse clicks
		IsFixed   = 0x0004,		// Gadget is not moveable (fixed to right edge
								// of control)
		IsNull    = 0x0008
	};

// Operations
public:
	// Set/Get the style flags for this gadget.
	int  GetStyle() const;
	void SetStyle(int style);

	// Set/Get the current value of the gadget
	void SetValue(int nNewValue);
	int  GetValue() const;

	// Returns the gadget's ID;
	int GetID() const;

	// Set/Get the gadget's rectangle (relative to SECDateTimeCtrl's client)
	const CRect& GetRect() const;
	void SetRect(const RECT& rect);

// Overrideables
public:
	// Function to calculate the required size of the gadget
	virtual void CalcSize(CDC& dc) = 0;

	// Function to draw the gadget
	virtual void Draw(CDC& dc) = 0;

	// Function to enable (give focus to) the gadget (returns FALSE if fails)
	virtual BOOL Enable(BOOL bEnable);

	// Handlers for WM_KEYDOWN/WM_CHAR messages while gadget is enabled
	virtual void OnKeyDown(UINT /* nChar */) {}
	virtual void OnChar(UINT /* nChar */) {}

	// Handler for mouse messages while gadget is enabled (only if
	// style has WantMouse flag).
	virtual void OnLButtonDown(UINT /* nFlags */, CPoint /* point */) {}
	virtual void OnLButtonUp(UINT /* nFlags */, CPoint /* point */) {}
	virtual void OnMouseMove(UINT /* nFlags */, CPoint /* point */) {}
	virtual void OnCancelMode() {}

	// Handles timer events requested by this gadget
	virtual void OnTimer() {}

	// Handler for the WM_SYSCOLORCHANGE message (each gadget receives it)
	virtual void OnSysColorChange() {}

// Implementation
public:
	virtual ~SECDTGadget();

protected:
	// Passes notify events to parent
	BOOL Notify(UINT uNotifyCode, int nID, LONG lParam = 0L);

	// Invalidates this gadget.
	void Invalidate(BOOL bImmediateRedraw = FALSE);

	// Set capture to this gadget
	void CaptureMouse(BOOL bSet);

	// Functions for manipulating timer events
	void StartTimer(UINT uElapse /* ms */);
	void StopTimer();

	// Callback function used for timer events.
	static void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, 
										  DWORD dwTime);

// Implementation - data members
protected:
	CRect               m_rect;			// Gadget window rect (relative to
										// SECDateTimeCtrl's client)
	BOOL                m_bEnabled;		// TRUE if we are enabled (have focus)
	int                 m_style;		// Holds style flags
	int                 m_nID;			// This gadgets ID
	int                 m_nValue;		// This gadgets current value
	SECDateTimeCtrl*    m_pCtrl;		// Parent SECDateTimeCtrl
	static CMapWordToOb m_timers;		// Maps current timers onto gadgets
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  class SECDTStaticGadget
//  
//  Class implementing a non-editable text gadget for SECDateTimeCtrl
//
class SECDTStaticGadget : public SECDTGadget
{
DECLARE_DYNAMIC(SECDTStaticGadget)
// Construction
public:
	SECDTStaticGadget();

	// Initialises the static gadget (and define the caption)
	BOOL Initialise(SECDateTimeCtrl* pCtrl, LPCTSTR lpszCaption, 
					int nID = -1);

// Attributes
public:

// Operations
public:
	// Function to return the caption
	CString GetCaption() const;

// Overrideables
protected:
	// Function to calculate the required size of the gadget
	virtual void CalcSize(CDC& dc);

	// Function to draw the gadget
	virtual void Draw(CDC& dc);

// Implementation
public:
	virtual ~SECDTStaticGadget();

// Implementation - data members
protected:
	CString m_strCaption;				// Text of caption
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  class SECDTNumericGadget
//  
//  Class implementing an editable numeric gadte for SECDateTimeCtrl
//
class SECDTNumericGadget : public SECDTGadget
{
DECLARE_DYNAMIC(SECDTNumericGadget)
// Construction
public:
	SECDTNumericGadget();

	// Initialise the numeric gadget.
	BOOL Initialise(SECDateTimeCtrl* pCtrl, int nMin, int nMax, int nValue, 
					BOOL bLeadZero, int nID = -1);

// Attributes
public:

// Operations
public:

// Overrideables
protected:
	// Function to calculate the required size of the gadget
	virtual void CalcSize(CDC& dc);

	// Function to draw the gadget
	virtual void Draw(CDC& dc);
	
	// Function to enable (give focus to) the gadget (returns FALSE if fails)
	virtual BOOL Enable(BOOL bEnable);

	// Handlers for WM_KEYDOWN and WM_CHAR when gadget is enabled
	virtual void OnKeyDown(UINT nChar);
	virtual void OnChar(UINT nChar);

	// Handler for WM_CANCELMODE
	virtual void OnCancelMode();

// Implementation
public:
	virtual ~SECDTNumericGadget();

protected:
	// Validate's keyboard entry of new date
	BOOL Validate();

// Implementation - data members
protected:
	int  m_nMin;				// Minimum value
	int  m_nMax;				// Maximum value
	int  m_nWidth;				// Maximum no. of chars. allowed
	int  m_nNewValue;			// New value (used during keyboard entry)
	int  m_nCurDigit;			// Current digit being editted
	BOOL m_bLeadZero;			// TRUE if value is to be displayed with
								// leading zero
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  class SECDTListGadget
//  
//  Class implementing a text gadget with a defined list of possible strings
//
class SECDTListGadget : public SECDTGadget
{
DECLARE_DYNAMIC(SECDTListGadget)
// Construction
public:
	SECDTListGadget();

	// Initialise the list gadget
	BOOL Initialise(SECDateTimeCtrl* pCtrl, const CStringArray& list, 
					int nValue, int nID = -1);

// Attributes
public:

// Operations
public:

// Overrideables
protected:
	// Function to calculate the required size of the gadget
	virtual void CalcSize(CDC& dc);

	// Function to draw the gadget
	virtual void Draw(CDC& dc);
	
	// Function to enable (give focus to) the gadget (returns FALSE if fails)
	virtual BOOL Enable(BOOL bEnable);

	// Handlers for WM_KEYDOWN and WM_CHAR when gadget is enabled
	virtual void OnKeyDown(UINT nChar);
	virtual void OnChar(UINT nChar);

	// Handler for timer events
	virtual void OnTimer();

// Implementation
public:
	virtual ~SECDTListGadget();

protected:
	int FindMatch(LPCTSTR lpszPat, int nCurPos);

// Implementation - data members
protected:
	CStringArray m_list;				// List of possible string
	TCHAR        m_cBuf[3];				// Used during entry of Multi-Byte
										// character
	CString      m_strPat;				// Holds string entered so far 
										// (during partial match)
	BOOL         m_bTimerRunning;		// TRUE when we have a timer enabled
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  class SECDTButtonGadget
//  
//  Class implementing a simple push button gadget for SECDateTimeCtrl
//
class SECDTButtonGadget : public SECDTGadget
{
DECLARE_DYNAMIC(SECDTButtonGadget)
// Construction
public:
	SECDTButtonGadget();

	BOOL Initialise(SECDateTimeCtrl* pCtrl, LPCTSTR lpszBmpName, 
					int nID = -1);

// Attributes
public:
	enum				// State flags (see m_nState)
	{
		Capture = 0x0001,			// We currently have capture (tracking
									// button press)
		Down    = 0x0002			// The button is currently pressed
	};

// Operations
public:

// Overrideables
protected:
	// Function to calculate the required size of the gadget
	virtual void CalcSize(CDC& dc);

	// Function to draw the gadget
	virtual void Draw(CDC& dc);

	// Handlers for mouse messages while we are enabled
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnCancelMode();

	// Handler for WM_SYSCOLORCHANGE
	virtual void OnSysColorChange();

// Implementation
public:
	virtual ~SECDTButtonGadget();

// Implementation - data members
protected:
	int     m_nState;				// State information
	CBitmap m_bmp;					// Bitmap displayed on button
	LPCTSTR m_lpszBmpName;			// Resource ID of bitmap
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  class SECDTSpinGadget
//  
//  Class implementing a spin button gadget for use with SECDateTimeCtrl
//
class SECDTSpinGadget : public SECDTGadget
{
DECLARE_DYNAMIC(SECDTSpinGadget)
// Construction
public:
	SECDTSpinGadget();

// Attributes
public:
	enum							// State flags (see m_nState)
	{	
		SpinUp       = 0x0001,		// The spin up button is currently pressed
		SpinDown     = 0x0002,		// The spin down button is currently pressed
		CaptureUp    = 0x0004,		// We are tracking press of spin up btn
		CaptureDown  = 0x0008,		// We are tracking press of spin down btn
		TimerRunning = 0x0010,		// We have a timer running
		InitialTimer = 0x0020		// The timer has been initialised (first
	};								// tick = 500ms, subsequent ticks = 100ms).

// Operations
public:

// Overrideables
protected:
	// Function to calculate the required size of the gadget
	virtual void CalcSize(CDC& dc);

	// Function to draw the gadget
	virtual void Draw(CDC& dc);

	// Handlers for mouse messages while we are enabled
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnCancelMode();

	// Handler for timer events
	virtual void OnTimer();

// Implementation
public:
	virtual ~SECDTSpinGadget();

protected:
	// Draws either up or down spin button
	void DrawPart(CDC& dc, CRect& r, BOOL bInvert, BOOL bDown);

	// Returns 0 if point is on spin up button, 1 if on spin down, else -1
	int  CheckHit(CPoint point) const;

// Implementation - data members
protected:
	int m_nState;					// Holds state information
	int m_nTimer;					// Timer tick count
};

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