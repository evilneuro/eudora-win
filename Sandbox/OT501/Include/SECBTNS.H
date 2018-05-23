/////////////////////////////////////////////////////////////////////////////
// SecBtns.h : Declaration of the SECOwnerDrawButton, SECWellButton,
//             SECBitmapButton and SECMenuButton custom controls.
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


#ifndef __SECBTNS_H__
#define __SECBTNS_H__

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


class SECPopupColorWell;

/////////////////////////////////////////////////////////////////////////////
// SECOwnerDrawButton

class SECOwnerDrawButton : public CButton
{
	DECLARE_DYNAMIC(SECOwnerDrawButton);

// Construction
public:
	SECOwnerDrawButton();

	// Attach to an existing button.
	virtual	BOOL AttachButton(int nCtlID, CWnd* pParentWnd);

	// Dynamic button creation.
	virtual BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect,
						CWnd* pParentWnd, int nID);

// Attributes
public:

// Operations
public:

// Overrideables
protected:
	// Method invoked when button is clicked
	virtual void OnBnClicked() {}

// Implementation
public:
	virtual ~SECOwnerDrawButton();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Drawing functions.
	virtual void  DrawItem(LPDRAWITEMSTRUCT lpDS);
	virtual void  PreDrawButton(CDC* pScreenDC, CDC* pMemDC);
	virtual void  PostDrawButton(CDC* pScreenDC, CDC* pMemDC);
	virtual void  DrawDisabled(CDC* pDC, int nWidth, int nHeight);
	virtual void  DrawFocus(CDC* pDC, LPDRAWITEMSTRUCT lpDS);
	virtual void  DrawFocus(CDC* pDC, CRect& rect) = 0;
	virtual CRect DrawButton(CDC* pDC, LPDRAWITEMSTRUCT lpDS );
	virtual void  DrawSpecific(CDC* pDC, LPDRAWITEMSTRUCT lpDS, CRect& rect) = 0;

protected:
	// Implementation, data members.
	BOOL m_bWin4;					// TRUE if windows version 4.

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSysColorChange();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// SECWellButton window

class SECWellButton : public SECOwnerDrawButton
{
	DECLARE_DYNAMIC(SECWellButton);

// Construction
public:
	SECWellButton();

// Attributes
public:
	// Set/Get the currently selected color.
	void SetColor(COLORREF cl);
	COLORREF GetColor() const;

	// Set/Get whether the popup has an other button or not.
	void SetOtherButton( BOOL bOther );
	BOOL GetOtherButton() const;

	// Set/Get the current palette relizaion mode (TRUE=palette is realized).
	void SetPaletteRealization(BOOL bRealize);
	BOOL GetPaletteRealization() const;

// Operations
public:
	// Function to set the Popup to something other than the default.
	void SetPopup(SECPopupColorWell* pPopup);

// Overrideables
protected:
	// Function for creating the popup color well.
	virtual void CreatePopup();

	// Function for creating and initialising the palette.
	virtual void CreatePalette();

	// Function for specifying options for the color common dialog
	virtual DWORD GetColorDialogFlags() const;

// Implementation
public:
	virtual ~SECWellButton();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Method invoked when button is clicked (create popup)
	virtual void OnBnClicked();
	
	// Drawing functions.
	virtual void PreDrawButton(CDC* pScreenDC, CDC* pMemDC);
	virtual void PostDrawButton(CDC* pScreenDC, CDC* pMemDC);
	virtual void DrawFocus(CDC* pDC, CRect& rect);
	virtual void DrawSpecific(CDC* pDC, LPDRAWITEMSTRUCT lpDS, CRect& rect);

protected:
	// Implementation, data members.
	SECPopupColorWell* m_pPopup;// The popup color well.
	CPalette m_palette;			// Palette used for holding current color	
	CPalette* m_pOldPal;			// Used during drawing
	CPalette* m_pMemOldPal;		// Used during drawing
	BOOL m_bRealizePalette;		// TRUE if we are to realize our palette before drawing
	BOOL m_bHasOther;			// TRUE if popup has other button.

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg LONG OnColorChange(UINT wParam, LONG lParam);
	afx_msg LONG OnCustomColor(UINT wParam, LONG lParam);
};

/////////////////////////////////////////////////////////////////////////////
// DDX functions for use with the SECWellButton
void DDX_Color(CDataExchange* pDX, SECWellButton& cButton, COLORREF& color);

/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// SECBitmapButton window

class SECBitmapButton : public SECOwnerDrawButton
{
	DECLARE_DYNAMIC(SECBitmapButton);

// Construction
public:
	SECBitmapButton();

	// The alignment modes.
	enum Alignment
	{
		Al_Left,		// Bitmap on left, text on right
		Al_Right,		// Bitmap on right, text on left
		Al_Top,			// Bitmap on top, text on bottom
		Al_Bottom,		// Bitmap on bottom, text on top
		Al_Center		// Bitmap centered in button. No caption.
	};

	// Attach to an existing button.
	virtual BOOL AttachButton(int nCtlID, Alignment alignment, int nIDBmp, 
							  CWnd* pParentWnd);
	virtual BOOL AttachButton(int nCtlID, Alignment alignment, LPCTSTR lpszBmpName,
							  CWnd* pParentWnd);

	// Dynamic button creation
	virtual BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle, Alignment alignment, 
						LPCTSTR lpszBmpName, const CRect& rect, CWnd* pParentWnd, 
						int nID);
	virtual BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle, Alignment alignment, 
						int nIDBmp,	const CRect& rect, CWnd* pParentWnd, int nID);

// Attributes
public:
	// Set/Get the indentation of the bitmap and text from the edge of the
	// button face
	void SetIndents(CSize sz);
	CSize GetIndents() const;

// Operations
public:
	// Size the button to fit it's contents
	void SizeToContent();

// Overrides
protected:
	// Function to calculate the position of the bitmap and text.
	virtual void CalcLayout();

// Implementation
public:
	virtual ~SECBitmapButton();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Drawing functions.
	virtual void DrawSpecific(CDC* pDC, LPDRAWITEMSTRUCT lpDS, CRect& rect);
	virtual void DrawFocus(CDC* pDC, CRect& rect);

protected:
	// Implementation, data members.
	LPCTSTR m_lpszBmpName;		// Resource name for bitmap.
	CBitmap m_bmp;				// The bitmap (need I say more)
	CSize m_indents;			// Indentations of bitmap from edge of button face.
	CRect m_bmpRect;			// Window rectangles for the bitmap and caption,
	CRect m_textRect;			// note that they are aligned with 0,0.
	Alignment m_alignment;		// The alignment mode.

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSysColorChange();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
// SECMenuButton window

class SECMenuButton : public SECBitmapButton
{
	DECLARE_DYNAMIC(SECMenuButton);

// Construction
public:
	SECMenuButton();

	// Attach to an existing button.
	enum Direction
	{
		DT_Down,
		DT_Right
	};
	virtual	BOOL AttachButton(int nCtlID, CWnd* pParentWnd, HMENU hMenu,
							  Direction direction=DT_Down);

	// Dynamic button creation
	virtual BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle, const CRect& rect, 
						CWnd* pParentWnd, int nID, HMENU hMenu, 
						Direction direction=DT_Down);

	// Attributes
public:
	// Set/Get the arrow direction
	void SetDirection(Direction direction);
	Direction GetDirection() const;
	
// Operations
public:
	// Set the popup menu.
	void SetMenu(HMENU hMenu);

// Overrideables
protected:

// Implementation
public:
	virtual ~SECMenuButton();

#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Method invoked when button is clicked (create popup menu)
	virtual void OnBnClicked();

	// Function to perform object initialisation.
	virtual SECBitmapButton::Alignment Initialise(LPCTSTR lpszCaption, HMENU hMenu, 
  										  Direction direction, UINT& nIDBmp);

protected:
	// Implementation, data members.
	CMenu	  m_menu;					// The popup menu.
	Direction m_direction;				// Direction of arrow down or right.
	BOOL	  m_bAllowDraw;				// When FALSE, the button is not drawn.

	// Implementation - message map and entries
	DECLARE_MESSAGE_MAP()

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDS);
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Function to load a bitmap changing it's colors to the current system color
// scheme.
//
extern BOOL SECLoadSysColorBitmap(CBitmap& bmp, LPCTSTR lpszName, bool bToolbar = false);


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif //__SECBTNS_H__




