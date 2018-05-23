/////////////////////////////////////////////////////////////////////////////
// SecCalc.h : header file for the SECCalculator and SECPopupCalculator
//             custom controls
//
// Stingray Software Extension Classes
// Copyright (C) 1996-1997 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
//		Author:		Don			20/01/97
//

#ifndef __SECCALC_H__
#define __SECCALC_H__

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


// SECCalculator styles
#define SEC_CS_RAISEDEDGE			0x0001L		// Same look as WS_EX_WINDOWEDGE
												// except compatible with Win 3.
#define SEC_CS_DIVIDER				0x0002L		// Divider at top of calculator

// SECPopupCalculator style
#define SEC_PCS_DESTROY_ON_EQUALS	0x0010L		// Popup calculator destroyed
												// everytime equals is pressed.

// Forward references ...
class SECCalcBtn;


/////////////////////////////////////////////////////////////////////////////
// SECCalculator window
//
class SECCalculator : public CWnd
{
	DECLARE_DYNAMIC(SECCalculator)

// Construction
public:
	SECCalculator();

	BOOL Create(DWORD dwStyle, int x, int y, CWnd* pParentWnd, UINT nID, 
				CCreateContext* pContext = NULL);

	BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, int x, int y, 
				  CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

// Attributes
public:
	// List of calculator functions/buttons
	enum SECCalcOp
	{
		OpNull     = 0,			// No operation
		OpMultiply = 1,			// Multiply operator
		OpPlus     = 2,			// Plus operator
		OpMinus    = 3,			// Minus operator
		OpDivide   = 4,			// Divide operator
		OpPercent  = 5,			// Percentage operator
		OpEquals   = 6,			// Equals operator
		OpC        = 7,			// Clear function
		OpCE	   = 8,			// Clear error function
		Op0		   = 9,			// Decimal digit 0
		Op1        = 10,		// Decimal digit 1
		Op2        = 11,		// Decimal digit 2
		Op3        = 12,		// Decimal digit 3
		Op4        = 13,		// Decimal digit 4
		Op5        = 14,		// Decimal digit 5
		Op6        = 15,		// Decimal digit 6
		Op7        = 16,		// Decimal digit 7
		Op8        = 17,		// Decimal digit 8
		Op9        = 18,		// Decimal digit 9
		OpDP       = 19,		// Decimal place
		OpBack     = 20,		// Delete last character

		// Operations not yet implemented
		OpMPlus	   = 21,		// Memory add
		OpMMinus   = 22,		// Memory subtract
		OpM        = 23,		// Places current number into memory
		OpMRecall  = 24,		// Memory recall
		OpSqrt     = 25,		// Square root

		// Operations for user use only
		OpUser1    = 100,
		OpUser2    = 101,
		OpUser3    = 102,
		OpUser4    = 103,
		OpUser5    = 104,
		OpUser6    = 105,
		OpUser7    = 106,
		OpUser8    = 107
	};

// Operations
public:
	// Invalidates the button at the given index.
	void InvalidateBtn(int nIndex, BOOL bErase = FALSE);

	// Invalidates the LCD panel
	void InvalidatePanel(BOOL bErase = FALSE);

	// Resets the calculator back to a "zero" state
	void ResetCalc();

	// Returns the index of the button under the given point
	int HitTest(CPoint point) const;

	// Set/Get the current calculator value
	void   SetValue(double val);
	double GetValue() const;

	// Set the maximum number of decimal places to be displayed
	void SetDecimalPlaces(int nDP);
	int  GetDecimalPlaces() const;

// Overrides
protected:
	virtual BOOL CreateBtns(CWnd* pParentWnd);
	virtual BOOL CreatePanel(CWnd* pParentWnd);
	virtual BOOL HandleEvent(SECCalcOp op);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECCalculator)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECCalculator();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Layes out the calculator buttons
	CSize CalcLayout(DWORD dwStyle);

	// Formats the value into a displayable string
	void SetDisplayString();

	// Presses the given button
	void PressBtn(int nIndex);

	// Sizes the panel to the width of the calculator.
	void SizePanel();

	// Formats the current number to the supplied no. of decimal places into
	// the supplied buffer
	void FormatNo(LPTSTR ptr, int nDP);

	// Returns the class style that this window "will" be registered with
	virtual UINT GetClassStyle() const;

	// Draws the calculator
	virtual void Draw(CDC& dc);

	// Draws the panel
	void DrawPanel(CDC& dc);

	// Functions used for handling inputs
	BOOL HandleDigit(SECCalcOp op);
	BOOL HandleDecimalPoint(SECCalcOp op);
	BOOL HandleBackspace(SECCalcOp op);
	BOOL HandleOperator(SECCalcOp op);
	BOOL HandleClear(SECCalcOp op);
	BOOL HandleClearError(SECCalcOp op);

	// Implementation, data members.
protected:
	enum // State flags
	{
		Capture         = 0x0001,	// We currently have capture
		HaveLead        = 0x0002,	// Have received the first byte of a 
									// multi-byte pair
		HasInitialValue = 0x0004,	// Reset won't clear current value
		CalcError       = 0x0008	// Calculator in error (divide by zero etc)
	};

	// General data members
	TCHAR     m_cLeadByte;			// Used by WM_CHAR for saving first byte
									// of multi-byte pair
	int       m_nState;				// Holds the current state(see state flags)

	// Calculation engine data members
	double    m_val;				// The current value 
	SECCalcOp m_lastOp;				// The last operator entered. This will
									// be actioned when the next operator is
									// entered.
	BOOL      m_bReplace;			// The next digit entered will replace the
									// current displayed number
	int       m_nDP;				// Max. no. of DP to display to
	int       m_nTotalDigits;		// Total no. of displayable digits
	
	// Panel (LCD display) data members
	CRect     m_panelRect;			// The outer rectange of the panel
	CString   m_strDisplay;			// The current display string for the panel

	// Drawing/layout data members
	CFont     m_btnFont;			// Font used for drawing buttons
	CFont     m_panelFont;			// Font used for drawing the panel
	HBITMAP   m_hEqualsBmp;			// Handle of bitmap used on equals button
	int       m_nBtnWidth;			// Width of a button (a column)
	int       m_nBtnHeight;			// Height of a button
	int       m_nVIndent;			// Vertical Indent of button matrix
	int       m_nHIndent;			// Horizontal Indent of button matrix
	int       m_nVSpace;			// Space between button rows
	int       m_nHSpace;			// Space between button columns
	int       m_nCapture;			// Index of button with capture

	// We maintain an array of the calculator buttons, m_btns. This would
	// normally be defined with CTypedPtrArray, but templates are not supported
	// by the Microsoft 16-Bit compiler and some Unix compilers. The following
	// is what CTypedPtrArray would have defined.
	class SECCalcBtnArray : public CObArray
	{
	public:
		// Accessing elements
		SECCalcBtn* GetAt(int nIndex) const
			{ return (SECCalcBtn*)CObArray::GetAt(nIndex); }
		SECCalcBtn*& ElementAt(int nIndex)
			{ return (SECCalcBtn*&)CObArray::ElementAt(nIndex); }
		void SetAt(int nIndex, SECCalcBtn* ptr)
			{ CObArray::SetAt(nIndex, (CObject*) ptr); }

		// Potentially growing the array
		void SetAtGrow(int nIndex, SECCalcBtn* newElement)
		   { CObArray::SetAtGrow(nIndex, (CObject*) newElement); }
		int Add(SECCalcBtn* newElement)
		   { return CObArray::Add((CObject*) newElement); }
#ifdef WIN32
		int Append(const SECCalcBtnArray& src)
		   { return CObArray::Append(src); }
		void Copy(const SECCalcBtnArray& src)
			{ CObArray::Copy(src); }
#endif

		// Operations that move elements around
		void InsertAt(int nIndex, SECCalcBtn* newElement, int nCount = 1)
			{ CObArray::InsertAt(nIndex, (CObject*) newElement, nCount); }
		void InsertAt(int nStartIndex, SECCalcBtnArray* pNewArray)
		   { CObArray::InsertAt(nStartIndex, pNewArray); }

		// overloaded operator helpers
		SECCalcBtn* operator[](int nIndex) const
			{ return (SECCalcBtn*)CObArray::operator[](nIndex); }
		SECCalcBtn*& operator[](int nIndex)
			{ return (SECCalcBtn*&)CObArray::operator[](nIndex); }
	} m_btns;

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg void OnSysColorChange();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
};


/////////////////////////////////////////////////////////////////////////////
// SECPopupCalculator window
//
class SECPopupCalculator : public SECCalculator
{
	DECLARE_DYNAMIC(SECPopupCalculator)

// Construction
public:
	SECPopupCalculator();

	BOOL Create(DWORD dwStyle, const RECT& ctrlRect, CWnd* pParentWnd, 
				CCreateContext* pContext = NULL);

	BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& ctrlRect, 
				  CWnd* pParentWnd, CCreateContext* pContext = NULL);

// Attributes
public:
	// Set to TRUE if you want the calculator object to be deleted with
	// the window
	BOOL m_bAutoDelete;

// Overrides
public:
	// Called when a button is pressed on the calculator
	virtual BOOL HandleEvent(SECCalcOp op);

	// Handles dispatching the result off to interested windows
	virtual void Notify();

// Implementation
public:
	virtual ~SECPopupCalculator();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Returns the class style that this window "will" be registered with
	virtual UINT GetClassStyle() const;

// Implementation - message map and entries
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void PostNcDestroy();
};


/////////////////////////////////////////////////////////////////////////////

// Function to convert Dialog Base Units to pixels.
CSize SECConvertDBU(int x, int y, HFONT hFont = NULL);


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif //WIN32

#endif //__SECCALC_H__
