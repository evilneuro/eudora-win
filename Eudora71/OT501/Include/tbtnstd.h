// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Authors:		Don
//  Description:	Declarations for SECToolBar
//  Created:		August 1996
//

#ifdef WIN32

#ifndef __TBTNSTD_H__
#define __TBTNSTD_H__

#include "SafetyPal.h"

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

// Forward Declarations
class SECCustomToolBar;
struct SECButtonClass;

// Define the height and width of an individual item in the toolbar bitmap
#define SEC_TOOLBAR_IMAGE_WIDTH		16
#define SEC_TOOLBAR_IMAGE_HEIGHT	15

///////////////////////////////////////////////////////////////////////////////
// SECBtnDrawData struct - contains cached info for drawing toolbars
//
struct SECBtnDrawData
{
	CPaletteDC m_monoDC;			// mono DC used during drawing
	CPaletteDC m_bmpDC;				// DC holding the toolbar bmp
	CPaletteDC m_drawDC;			// mem. DC we use to draw to
	CBrush m_ditherBrush;			// Dither brush used for drawing
									// checked/indeterminate buttons

	SECBtnDrawData();
	~SECBtnDrawData();

	// Called when system color scheme changes.
	void SysColorChange();

	// Pre/Post drawing initialization
	BOOL PreDrawButton(CDC& dc, HBITMAP hBmp, int nMaxBtnWidth,
					   SECCustomToolBar* pToolBar);
	void PostDrawButton();

protected:
	HBRUSH CreateDitherBrush();

	// Members used by pre/post drawing functions
	CBitmap  m_bmpDraw;
	CBitmap* m_pOldBmpDraw;
	CBitmap  m_bmpMono;
	CBitmap* m_pOldBmpMono;
	HGDIOBJ  m_hOldBmp;
};

///////////////////////////////////////////////////////////////////////////////
// SECStdBtn class
//
class SECStdBtn
{
// Construction
public:
	SECStdBtn();

// Attributes
public:
	enum InformCode			// Codes informing us to take some kind of action
	{
		IBase   = 0x0000,
		Styles  = IBase + 1	// Add/Remove button styles.
	};

	struct StyleChange		// Used by InformCode::Styles
	{
		DWORD dwAdd;		// Style bits to add
		DWORD dwRemove;		// Style bits to remove
	};

	UINT m_nID;				// Button ID
	UINT m_nImage;			// Index of image in toolbar bitmap
	UINT m_nStyle;			// Button styles
	int  m_x;				// x position in parent toolbars client rect
	int  m_y;				// y position in parent toolbars client rect
	int  m_cx;				// Width of button
	int  m_cy;				// Height of button

	ULONG	m_ulData;		// additional data

// Operations
public:
	// Sets the parent toolbar
	void SetToolBar(SECCustomToolBar* pToolBar);

	// Returns the window rect. for this button (relative to parent's client)
	void GetBtnRect(CRect& rect) const;

	// SetMode informs the button when the button either enters/leaves a
	// vertically docked state
	virtual void SetMode(BOOL bVertical);

	// Modifies our window position
	virtual void SetPos(int x, int y);

	// Invalidates the button
	virtual void Invalidate(BOOL bErase = FALSE) const;

// Overrideables
public:
	// Initialises button after creation.
	virtual void Init(SECCustomToolBar* pToolBar, const UINT* pData);

	// Draw the button on the given DC.
	virtual void DrawButton(CDC& dc, SECBtnDrawData& data);

	// Handle mouse events in the button's window rect.
	virtual BOOL BtnPressDown(CPoint point);
	virtual void BtnPressMouseMove(CPoint point);
	virtual void BtnPressCancel();
	virtual UINT BtnPressUp(CPoint point);

	// Our parent toolbars style has been modified.
	virtual void BarStyleChanged(DWORD dwStyle);

	// Tool-tip/flyby help hit-testing
	virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	// Informs button of some event
	virtual void InformBtn(UINT nCode, void* pData);

	// Called when toolbar bitmap has changed ... buttons should now adjust
	// their size.
	virtual void AdjustSize();

	// Functions called when saving/loading a toolbar state, allowing the
	// button to save state info.
	virtual void GetBtnInfo(BYTE* nSize, LPBYTE* ppInfo) const;
	virtual void SetBtnInfo(BYTE nSize, const LPBYTE pInfo);

protected:
	// Drawing functions ...
	virtual void DrawFace(SECBtnDrawData& data, BOOL bForce, int& x, int& y, 
						  int& nWidth, int& nHeight, int nImgWidth = -1);
	virtual void DrawDisabled(SECBtnDrawData& data, int x, int y, int nWidth, 
							  int nHeight);
	virtual void DrawChecked(SECBtnDrawData& data, int x, int y, int nWidth, 
							 int nHeight);
	virtual void DrawIndeterminate(SECBtnDrawData& data, int x, int y, 
								   int nWidth, int nHeight);
	virtual void DrawConfigFocus(SECBtnDrawData& data);

// Implementation
public:
	virtual ~SECStdBtn();

protected:	
	// Drawing helper functions
	void CreateMask(SECBtnDrawData& data, int x, int y, int nWidth, int nHeight);

protected:
	// Implementation, data members.
	SECCustomToolBar* m_pToolBar;			// Our parent toolbar

public:
	// Dynamic creation helpers
	virtual SECButtonClass* GetButtonClass() const;
	static const AFX_DATA SECButtonClass classSECStdBtn;
	static SECStdBtn* PASCAL CreateButton();
};

///////////////////////////////////////////////////////////////////////////////
// Dynamic button creation helpers
//
#define BUTTON_CLASS(class_name) ((SECButtonClass*)(&class_name::class##class_name))

#define DECLARE_BUTTON(class_name) \
public: \
	static const AFX_DATA SECButtonClass class##class_name; \
	virtual SECButtonClass* GetButtonClass() const; \
	static SECStdBtn* PASCAL CreateButton();

#define IMPLEMENT_BUTTON(class_name) \
	SECStdBtn* PASCAL class_name::CreateButton() \
		{ return new class_name; } \
	const AFX_DATADEF SECButtonClass class_name::class##class_name = { \
		class_name::CreateButton }; \
	SECButtonClass* class_name::GetButtonClass() const \
		{ return BUTTON_CLASS(class_name); } \

struct SECButtonClass 
{
// Attributes
	SECStdBtn* (PASCAL* m_pfnCreateButton)();

// Operations
	SECStdBtn* CreateButton();
};

///////////////////////////////////////////////////////////////////////////////
// Button map definitions and helper macros	 **** DEAN - Should this be moved?
//
#define BTNMAP_DATA_ELEMENTS	5
struct SECBtnMapEntry
{
	SECButtonClass* m_pBtnClass;					// Class for this button
	UINT            m_nID;							// ID for this button
	UINT	 	    m_nStyle;						// Style for this button
	UINT            m_nData[BTNMAP_DATA_ELEMENTS];	// Button specific info.
};

// Define the beginning of a toolbar map.
#define BEGIN_BUTTON_MAP(name) \
	static const SECBtnMapEntry name[] = {

// Define a standard toolbar button
#define STD_BUTTON(id, style) \
	{BUTTON_CLASS(SECStdBtn), id, style|TBBS_BUTTON, 0, 0, 0, 0, 0},

// Define the end of the toolbar map
#define END_BUTTON_MAP() \
	{NULL, 0, 0, 0, 0, 0, 0, 0} \
	};


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TBTNSTD_H__

#endif // WIN32