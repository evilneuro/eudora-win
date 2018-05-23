/////////////////////////////////////////////////////////////////////////////
// SecWell.h : header file for the SECColorWell and SECPopupColorWell custom
//             controls
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
//		Author:		Don			04/02/96
//

 
#ifndef __SECWELL_H__
#define __SECWELL_H__

// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#define CWN_COLOR_CHANGE		(WM_USER+101)
#define CWN_CUSTOM_COLOR		(WM_USER+102)
#define CWS_RAISED_BORDER		0x0001L			// Draw well with raised border

class SECOtherButton;

/////////////////////////////////////////////////////////////////////////////
// SECColorWell window

class SECColorWell : public CWnd
{
	DECLARE_DYNAMIC(SECColorWell);

// Construction
public:
	SECColorWell();
	virtual BOOL Create(DWORD dwStyle, 
						int x,							// Top left corner (auto sizes)
						int y, 
						CWnd* pParentWnd, 
						UINT nID, 
						BOOL bDialogBaseUnits = FALSE,	// x,y are dialog base units 
														// rather than pixels
						BOOL bOtherButton = TRUE,		// Has an other button
						CCreateContext* pContext = NULL);

// Attributes
public:
	// Set/Get the currently selected color.
	void SetColor(COLORREF color);
	COLORREF GetColor() const;

	// Set/Get the current mouse tracking mode (TRUE=mouse tracking enabled).
	void SetMouseTracking(BOOL bTracking);
	BOOL GetMouseTracking() const;

	// Set/Get the current palette relizaion mode (TRUE=palette is realized).
	void SetPaletteRealization(BOOL bRealize);
	BOOL GetPaletteRealization() const;

// Operations
public:

// Overrideables
protected:
	// Functions for drawing the color cells.
	virtual void DrawCell(CDC& memDC, CDC& paintDC, const CRect& internalRect, 
						  int nCol, int nRow);
	virtual void DrawSelectedCell(CDC& memDC, CDC& paintDC, const CRect& internalRect,
								  int nCol, int nRow);

	// Function for notifying parent of selection change
	virtual void NotifyChange();

	// Function for handling a selection change.
	enum SelTypes
	{
		ST_Keyboard,
		ST_MouseMove,
		ST_MouseButton
	};
	virtual void HandleSelection(int nCol, int nRow, SelTypes selType);

	// Function for creating and initialising the color palette.
	virtual void CreatePalette();

	// Function to set the size of the grid
	virtual void SetGridSize();

	// Function to do any extra color initialization
	virtual void InitAdditionalColors(LPPALETTEENTRY pPalEntry, int nCols, int nSystemCols) {pPalEntry; nCols; nSystemCols;}

	// Function for specifying options for the color common dialog
	virtual DWORD GetColorDialogFlags() const;

	// Function for providing non-default class styles.
	virtual UINT GetClassStyle() const;

	// Reset the state of the well. Do not call if created!
	virtual void Reset();

	// Function to indicate if I have a focus rectangle or not.
	virtual BOOL HasFocusRectangle() const; 

// Implementation
public:
	virtual ~SECColorWell();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Functions for obtaining the client rectangle if an individual cell
	void GetCellRect(const CRect& internalRect, CRect& rect, int nCol, int nRow);
	void GetCellRect(CRect& rect, int nCol, int nRow);

	// Function for drawing the well
	void Paint(CDC& memDC, CDC& paintDC);
	void DrawFocusRect();

	// Function for invalidating an individual cell
	void InvalidateCell(int nCol, int nRow);

	// Function to hit test which cell a specified point is in
	BOOL HitTest(CPoint point, int& nCol, int& nRow);

	// Functions for obtaining the color associated with an individual cell
	int  GetPaletteIndex( int nCol, int nRow ) const;		// As palette index
	COLORREF GetPaletteColor( int nCol, int nRow ) const;	// As RGB color

	// For changing the selection to a given cell.
	void ChangeSelection(int nCol, int nRow);

	// Funtion to calculate the internal client rectangle (less 3d borders).
	void GetInternalRect(CRect& rect);

	// Function to calulate the required size of the client rectangle
	CRect CalculateSize(CWnd* pParentWnd, int nCol, int nRow, BOOL bDialogBaseUnits);

protected:
	// Implementation, data members.
	CPalette m_palette;				// Our palette. Holds the colors for cells
	int m_nRows;					// No. of cell rows
	int m_nCols;					// No. of cell columns
	int m_nIndexOtherCol;			// Column no. of other cell
	int m_nIndexOtherRow;			// Row no. of other cell
	int m_nCellHeight;				// Height of cell
	int m_nCellWidth;				// Width of cell
	int m_nIndentWidth;				// Width of border
	int m_nIndentHeight;			// Height of border
	int m_nButtonHeight;			// Height of other button
	int m_nCurRow;					// The row of the currently selected cell
	int m_nCurCol;					// The col of the currently selected cell
	int m_nPaletteIndexOther;		// Index of other cell in color palette
	BOOL m_bHasFocus;				// True if I currently have focus.
	BOOL m_bMouseTracking;			// Whether mouse tracking is enabled.
	BOOL m_bHasOther;				// Whether we have an "other" button.
	BOOL m_bOtherSet;				// Has the color for "other" been set.
	BOOL m_bRealizePalette;			// Whether we realize our palette.
#ifdef WIN32
	CBrush m_brushBtnFace;			// Brush for painting button face
#endif
	SECOtherButton* m_pOtherButton;	// Object for other button
	
	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnClickedOther();
	afx_msg void OnSysColorChange();
#ifdef WIN32
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#endif
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// SECPopupColorWell window

class SECPopupColorWell : public SECColorWell
{
	DECLARE_DYNAMIC(SECPopupColorWell);

// Construction
public:
	SECPopupColorWell();

	virtual BOOL Create(DWORD dwStyle, 
						CRect& buttonRect, 
						CWnd* pParentWnd, 
						BOOL bHasOther = TRUE,		// TRUE if has other button
						CCreateContext* pContext = NULL);

// Attributes
public:
	// Is the popup in a created state ?
	BOOL IsCreated() const;

// Operations
public:
	// Function to set window to notify of color changes.
	void SetNotifyWindow(CWnd* pWnd);

	// Before creating the popup must be reset
	virtual void Reset();

// Overridables
protected:

// Implementation
public:
	virtual ~SECPopupColorWell() {};

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Functions for handling selection changes (overriden from SECColorWell).
	virtual void HandleSelection( int x, int y, SelTypes selType );
	virtual void NotifyChange();

	// Override PostNcDestroy to automatically destruct on destroying window
	virtual void PostNcDestroy();

	// Provide non-default class style (CS_SAVEBITS).
	virtual UINT GetClassStyle() const;

	// Function to indicate if I have a focus rectangle or not.
	virtual BOOL HasFocusRectangle() const; 

protected:
	// Implementation, data members.
	HWND m_notifyHwnd;				// Window to notify of color changes
	CWnd* m_pNotifyWnd;				// Window to return focus to on destroying popup.
	BOOL m_bCreated;				// True when the popup has been created

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClickedOther();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// DDX functions for use with the SECColorWell
void DDX_Color(CDataExchange* pDX, SECColorWell& cWell, COLORREF& color);


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif
            
