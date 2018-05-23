/////////////////////////////////////////////////////////////////////////////
// DropEdit.h : header file for SECDropEdit - subclassed edit with little
//			    drop down button.
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


#ifndef __DROPEDIT_H__
#define __DROPEDIT_H__

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


/////////////////////////////////////////////////////////////////////////////
// SECDropEdit window
//
// Adds a combo like drop down button to an edit control
//

class SECDropEdit : public CEdit
{
	DECLARE_DYNAMIC(SECDropEdit)

// Construction
public:
	SECDropEdit();

	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	// Attach to an existing edit control
	BOOL AttachEdit(int nCtlID, CWnd* pParentWnd);

// Attributes
public:
	// Methods for manipulating the alignment mode (within the edit control)
	void SetRightAlign(BOOL bRightAlign = TRUE);
	BOOL IsRightAligned() const;

	// Methods for manipulating the border settings
	void SetBorder(WORD wLeftBorder, WORD wRightBorder);
	DWORD GetBorders() const;

// Operations
public:
	// Methods to set/get the bitmap on the button
	BOOL SetBitmap(UINT nBmpID);
	BOOL SetBitmap(LPCTSTR lpszBmpName);
	BOOL SetBitmap();						// Removes button
	HBITMAP GetBitmap() const;

	// Determintes whether the given point (in screen or client
	// coordinates) is a hit on the button.
	BOOL HitTestBtn(CPoint point, BOOL bClient = FALSE) const;

	// Forces a redraw of the button
	void InvalidateBtn();

// Overrideables
protected:
	// Method called when the button has been clicked
	virtual void OnClicked();

	// Method called to draw the button
	virtual void DrawBtn(CDC& dc, CRect r);

// Implementation
public:
	virtual ~SECDropEdit();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Implementation, data members.
	CRect   m_btnRect;				// The window rect of the button (in our
									// parents client coordinates)
	CBitmap m_bmp;					// Bitmap drawn on face of button
	LPCTSTR m_lpszBmpName;			// Resource ID of above bitmap
	WORD    m_wState;				// Current state (see States below)
	WORD    m_wLeftBorder;			// Left hand Gap between bitmap and border
	WORD    m_wRightBorder;			// Right hand Gap between bitmap and border

	// Bit settings in m_wState
	enum States
	{
		Pressed    = 0x0001,		// Button is currently down	
		Capture    = 0x0002,		// We are tracking a left click on the button
		RightAlign = 0x0004			// The button is right aligned within the edit
	};

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnCancelMode();
	afx_msg void OnSysColorChange();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

/////////////////////////////////////////////////////////////////////////////

// Function to take the given rectange in the given dc and draw it disabled.
void SECDrawDisabled(CDC& dc, int x, int y, int nWidth, int nHeight);

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __DROPEDIT_H__

