/////////////////////////////////////////////////////////////////////////////
// CalcBtn.h : header file for SECCalcBtn - the base class for SECCalculator
//             buttons.
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

#ifndef __CALCBTN_H__
#define __CALCBTN_H__

#ifdef WIN32

#include "SecCalc.h"


#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


/////////////////////////////////////////////////////////////////////////////
// SECCalcBtn object
//
class SECCalcBtn : public CObject
{
// Construction
public:
	SECCalcBtn();

// Attributes
public:
	// Returns the width of the button
	BOOL GetWidth() const;

	// Returns the operation code for this button
	SECCalculator::SECCalcOp GetOp() const;

	// Set/Get the button state
	int  GetState() const;
	void SetState(int nState);

	// Returns TRUE if the given char is an accelerator for the button
	BOOL IsAccelerator(LPCTSTR lpszChar) const;

	// Returns TRUE if the WM_KEYDOWN char is an accelerator for the button
	BOOL IsKeyDownAccelerator(UINT nChar) const;

	// Sets the WM_KEYDOWN accelerator for this button
	void SetKeyDownAccel(UINT nChar);

	// Set/Get the rectange of the button (relative to calculators client)
	const CRect& GetRect() const;
	void SetRect(const RECT& rect);

	// Defines the attributes used for drawing a calculator button
	class Attrib
	{
	public:
		Attrib();
		Attrib(DWORD dwStyle, COLORREF textColor = 0L, HFONT hFont = NULL, 
			   HBITMAP* lphBitmap = NULL);
		Attrib(const Attrib& attrib);
		~Attrib();

		const Attrib& operator=(const Attrib& attrib);

		HFONT GetFont() const;
		COLORREF GetTextColor() const;
		HBITMAP GetBitmap() const;

	protected:
		HFONT    m_hFont;						// Font handle 
		HBITMAP* m_lphBitmap;					// Bitmap handle
		COLORREF m_textColor;					// Text color
		DWORD    m_dwStyle;						// Style (not currently used)
	};
	
// Operations
public:
	// Initialise button with accelerators are same as caption
	BOOL Initialise(const Attrib& attrib, SECCalculator::SECCalcOp op, 
					LPCTSTR lpszCaption, int nWidth = 1, 
					BOOL bStartsRow = FALSE);

	// Initialise button with single character accelerator
	BOOL Initialise(const Attrib& attrib, SECCalculator::SECCalcOp op, 
					LPCTSTR lpszCaption, TCHAR cAccel, int nWidth = 1, 
					BOOL bStartsRow = FALSE);

	// Initialise button with different accelerators to caption
	BOOL Initialise(const Attrib& attrib, SECCalculator::SECCalcOp op, 
					LPCTSTR lpszCaption, LPCTSTR lpszAccels, int nWidth = 1, 
					BOOL bStartsRow = FALSE);

// Overrides
public:
	// Draws the button!
	virtual void Draw(CDC& dc);

// Implementation
public:
	virtual ~SECCalcBtn();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

// Implementation, data members.
public:
	// enum State values
	enum
	{
		Down      = 0x0001,						// Button is currently down
		StartsRow = 0x0002						// Button starts a new row
	};

protected:
	Attrib		             m_attribs;			// Button attributes
	CString                  m_strCaption;		// Button caption
	CString                  m_strAccels;		// Std keyboard accelerators
	UINT		             m_nKeyDownAccel;	// WM_KEYDOWN accelerators
	int                      m_nWidth;			// Width of button
	CRect		             m_rect;			// Rectangle of button (relative
												// to calculators client)
	int                      m_nState;			// State flags (see state bits)
	SECCalculator::SECCalcOp m_op;				// The operation of this button
};

// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif //WIN32

#endif //__CALCBTN_H__
