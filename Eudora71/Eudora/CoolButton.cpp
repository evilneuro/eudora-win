//	CoolButton.cpp
//
//	Button that supports either split button functionality (i.e. part button
//	part drop down menu) or straight drop down menu functionality. Supports
//	optional icon-sized bitmap. Supports XP themes.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */



#include "stdafx.h"
#include "CoolButton.h"

#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCoolButton, CButton)

//	Message map
BEGIN_MESSAGE_MAP(CCoolButton, CButton)
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SYSKEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(BM_SETSTYLE, OnSetStyle)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover )
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave )
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
//		* CCoolButton												 [Public]
// ---------------------------------------------------------------------------
//	Constructor

CCoolButton::CCoolButton()
	:	m_menu(), m_btnImage(), m_hiconButton(NULL), m_hiconArrow(NULL),
		m_ilButton(), m_sizeIcon(0, 0), m_sizeArrow(0, 0), m_nMenuButtonWidth(0),
		m_bIsMenuPushed(false), m_bMenuInitialized(false),
		m_bIsDefaultButton(false), m_bIsSplit(false), m_bLeftAlignMenu(true),
		m_bIsMouseOverButton(false), m_bTracking(FALSE)
{
	m_menu.CreatePopupMenu();

	//	Load the bitmap for the combo-box arrow.
	//	Must have #define OEMRESOURCE in order to use OBM_COMBO.
	CBitmap			bmpArrow;
	VERIFY( bmpArrow.LoadOEMBitmap(OBM_COMBO) );

	//	Get the bitmap info
	BITMAP			bmArrow;
	bmpArrow.GetBitmap(&bmArrow);

	//	Create an icon based on the bitmap via an image list
	CImageList		imageArrow;
	imageArrow.Create(bmArrow.bmWidth, bmArrow.bmHeight, ILC_COLOR8 | ILC_MASK, 1, 1);
	imageArrow.Add( &bmpArrow, GetSysColor(COLOR_BTNFACE) );

	m_hiconArrow = imageArrow.ExtractIcon(0);
	m_sizeArrow.cx = bmArrow.bmWidth;
	m_sizeArrow.cy = bmArrow.bmHeight;

	//	Set the drop button width based on the size of the arrow plus some padding
	m_nMenuButtonWidth = m_sizeArrow.cx + kMenuButtonPadding;
}


// ---------------------------------------------------------------------------
//		* ~CCoolButton												 [Public]
// ---------------------------------------------------------------------------
//	Destructor

CCoolButton::~CCoolButton()
{
	if (m_hiconButton)
		DestroyIcon(m_hiconButton);

	if (m_hiconArrow)
		DestroyIcon(m_hiconArrow);
}


// ---------------------------------------------------------------------------
//		* PreSubclassWindow											 [Public]
// ---------------------------------------------------------------------------
//	Enforces owner draw style and opens theme

void
CCoolButton::PreSubclassWindow() 
{
	//	Enforce owner draw style, since it's crucial for us to draw correctly
	ModifyStyle(0, BS_OWNERDRAW);
	CButton::PreSubclassWindow();

	//	Open theme now that we're sure we have an hwnd
	m_xpThemedApppearance.OpenTheme(GetSafeHwnd(), L"Button");
}


// ---------------------------------------------------------------------------
//		* OnGetDlgCode											  [Protected]
// ---------------------------------------------------------------------------
//	Overridden to return whether or not we're a default button

UINT
CCoolButton::OnGetDlgCode()
{
	UINT	nCode = CButton::OnGetDlgCode();

	nCode |= m_bIsDefaultButton ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON;

	return nCode;
}


// ---------------------------------------------------------------------------
//		* OnSetStyle											  [Protected]
// ---------------------------------------------------------------------------
//	Overridden to track whether or not we're a default button and avoid
//	changing control type.

LRESULT
CCoolButton::OnSetStyle(
	WPARAM						wParam,
	LPARAM						lParam)
{
	//	Track whether or not we're a default button
	m_bIsDefaultButton = wParam & BS_DEFPUSHBUTTON;
	
	//	Enforce continued owner-draw style and avoid changing control type.
	//	Allow system to process changes to other style bits.
	return DefWindowProc(BM_SETSTYLE, (wParam & ~BS_TYPEMASK) | BS_OWNERDRAW, lParam);
}


// ---------------------------------------------------------------------------
//		* OnSetFocus											  [Protected]
// ---------------------------------------------------------------------------
//	Overridden to redraw.

void
CCoolButton::OnSetFocus(
	CWnd *						pOldWnd)
{
	CButton::OnSetFocus(pOldWnd);
	Invalidate();
}


// ---------------------------------------------------------------------------
//		* OnKillFocus											  [Protected]
// ---------------------------------------------------------------------------
//	Overridden for balance (and in case we need to add an Invalidate).

void
CCoolButton::OnKillFocus(
	CWnd *						pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
}


// ---------------------------------------------------------------------------
//		* OnSysColorChange										  [Protected]
// ---------------------------------------------------------------------------
//	Overridden to redraw.

void
CCoolButton::OnSysColorChange()
{
	CButton::OnSysColorChange();
	Invalidate();	
}


// ---------------------------------------------------------------------------
//		* OnSysKeyDown											  [Protected]
// ---------------------------------------------------------------------------
//	Overridden to show menu.

void
CCoolButton::OnSysKeyDown(
	UINT						nChar,
	UINT						nRepCnt,
	UINT						nFlags)
{
	switch ( nChar )
	{
		case VK_DOWN:
			OnMenuButton();
			break;
	}
	
	CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


// ---------------------------------------------------------------------------
//		* OnMouseMove											  [Protected]
// ---------------------------------------------------------------------------
//	Handles tracking mouse for determining when mouse is over button.

void
CCoolButton::OnMouseMove(
	UINT						nFlags,
	CPoint						point)
{
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT		tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;

		m_bTracking = _TrackMouseEvent(&tme) != FALSE;		
	}

	CButton::OnMouseMove(nFlags, point);
}


// ---------------------------------------------------------------------------
//		* OnMouseHover											  [Protected]
// ---------------------------------------------------------------------------
//	Helps track when mouse is over button.

LRESULT
CCoolButton::OnMouseHover(
	WPARAM						wparam,
	LPARAM						lparam)
{
	m_bIsMouseOverButton = true;
	Invalidate();
	return 1;
}


// ---------------------------------------------------------------------------
//		* OnMouseLeave											  [Protected]
// ---------------------------------------------------------------------------
//	Helps track when mouse is over button.

LRESULT
CCoolButton::OnMouseLeave(
	WPARAM						wparam,
	LPARAM						lparam)
{
	m_bTracking = FALSE;
	m_bIsMouseOverButton = false;
	Invalidate(FALSE);
	return 0;
}


// ---------------------------------------------------------------------------
//		* OnLButtonDown											  [Protected]
// ---------------------------------------------------------------------------
//	Handles mouse down. Overridden to handle menu portion of button.

void
CCoolButton::OnLButtonDown(
	UINT						nFlags,
	CPoint						point)
{
	SetFocus();
	if (m_bIsMenuPushed)
	{
		m_bIsMenuPushed = false;
		Invalidate();
		return;
	}

	if ( m_bMenuInitialized && (!m_bIsSplit || PtInMenuRect(point)) )
	{
		OnMenuButton();
		return;
	}

	Invalidate();
	CButton::OnLButtonDown(nFlags, point);
}


// ---------------------------------------------------------------------------
//		* OnLButtonDown											  [Protected]
// ---------------------------------------------------------------------------
//	Handles mouse up. Overridden to handle menu portion of button.

void
CCoolButton::OnLButtonUp(
	UINT						nFlags,
	CPoint						point)
{
	if (m_bIsMenuPushed)
	{
		m_bIsMenuPushed = false;
		Invalidate();
	}

	CButton::OnLButtonUp(nFlags, point);
}


// ---------------------------------------------------------------------------
//		* AddMenuItem												 [Public]
// ---------------------------------------------------------------------------
//	Adds menu item to button.

BOOL
CCoolButton::AddMenuItem(
	UINT						in_nMenuID,
	const char *				szMenuItem,
	UINT						nMenuFlags)
{
	BOOL bRet = m_menu.AppendMenu(nMenuFlags | MF_STRING, in_nMenuID, szMenuItem);

	if (bRet)
		m_bMenuInitialized = true;

	return bRet;
}


// ---------------------------------------------------------------------------
//		* RemoveMenuItem											 [Public]
// ---------------------------------------------------------------------------
//	Removes menu item from button.

BOOL
CCoolButton::RemoveMenuItem(
	UINT						in_nMenuID,
	UINT						nMenuFlags)
{
	BOOL bRet = m_menu.RemoveMenu( in_nMenuID, nMenuFlags | MF_STRING);

	if (bRet)
		m_bMenuInitialized = true;

	return bRet;
}


// ---------------------------------------------------------------------------
//		* ResetMenu													 [Public]
// ---------------------------------------------------------------------------
//	Clears all menu items and resets whether or not we're using it.

void
CCoolButton::ResetMenu()
{
	UINT	nMenuItemCount = m_menu.GetMenuItemCount();

	while (nMenuItemCount > 0)
	{
		m_menu.RemoveMenu( 0, MF_BYPOSITION);
		nMenuItemCount--;
	}
	
	m_bMenuInitialized = false;
}


// ---------------------------------------------------------------------------
//		* EnableMenuItem											 [Public]
// ---------------------------------------------------------------------------
//	Enables or disables button menu item.

BOOL
CCoolButton::EnableMenuItem(
	UINT						in_nMenuID,
	BOOL						bEnable)
{
	BOOL bRet = m_menu.EnableMenuItem(in_nMenuID , bEnable);

	if (bRet)
		m_bMenuInitialized = true;

	return bRet;
}


// ---------------------------------------------------------------------------
//		* CheckMenuItem												 [Public]
// ---------------------------------------------------------------------------
//	Checks or unchecks button menu item.

UINT
CCoolButton::CheckMenuItem(
	UINT						in_nMenuID,
	UINT						nMenuFlags)
{ 
	return m_menu.CheckMenuItem(in_nMenuID, nMenuFlags);
}


// ---------------------------------------------------------------------------
//		* SetButtonBitmap											 [Public]
// ---------------------------------------------------------------------------
//	Sets button image from a bitmap.

void
CCoolButton::SetButtonBitmap(
	UINT						in_nResourceID,
	COLORREF					in_crMask)
{
	//	Destroy any previous button image
	if (m_hiconButton)
		DestroyIcon(m_hiconButton);
	if ( m_ilButton.GetSafeHandle() )
		m_ilButton.DeleteImageList();
	
	//	Load the indicated bitmap
	CBitmap			bmp;
	
	if ( bmp.LoadBitmap(in_nResourceID) )
	{
		//	Get the bitmap info
		BITMAP			bitmap;
		bmp.GetBitmap(&bitmap);

		//	Create the image list and add the bitmap to it
		if ( m_ilButton.Create(bitmap.bmWidth, bitmap.bmHeight, ILC_COLOR8 | ILC_MASK, 1, 1) &&
			 (m_ilButton.Add(&bmp, in_crMask) != -1) )
		{
			//	Extract the icon for the button
			m_hiconButton = m_ilButton.ExtractIcon(0);

			if (m_hiconButton)
			{
				m_ilButton.Remove(0);
				m_ilButton.Add(m_hiconButton);
				
				//	Grab the icon size
				m_sizeIcon.cx = bitmap.bmWidth;
				m_sizeIcon.cy = bitmap.bmHeight;
			}
		}
	}
}


// ---------------------------------------------------------------------------
//		* SetButtonIcon												 [Public]
// ---------------------------------------------------------------------------
//	Sets button image from an icon.

void
CCoolButton::SetButtonIcon(
	UINT						in_nResourceID)
{
	//	Destroy any previous button image
	if (m_hiconButton)
		DestroyIcon(m_hiconButton);
	if ( m_ilButton.GetSafeHandle() )
		m_ilButton.DeleteImageList();

	//	Assume that an icon operation failed
	bool			bIconOperationFailed = true;

	//	Load the icon
	ICONINFO		ii;
	HINSTANCE		hResInst = QCFindResourceHandle( MAKEINTRESOURCE(in_nResourceID), RT_ICON );
	m_hiconButton = reinterpret_cast<HICON>( ::LoadImage(hResInst, MAKEINTRESOURCE(in_nResourceID), IMAGE_ICON, 16, 16, LR_SHARED) );
	
	//	Get the icon info if loading the icon worked
	if ( m_hiconButton && GetIconInfo(m_hiconButton, &ii) )
	{
		//	Delete the bitmap for the icon mask - we don't need it
		::DeleteObject(ii.hbmMask);
		
		//	Attach the bitmap for the icon itself to a CBitmap, which will
		//	be responsible for deleting it after we leave the local scope.
		CBitmap		bmp;
		bmp.Attach(ii.hbmColor);

		//	Get the bitmap info
		BITMAP		bitmap;
		bmp.GetBitmap(&bitmap);

		//	Create the image list and add the icon to it
		if ( m_ilButton.Create(bitmap.bmWidth, bitmap.bmHeight, ILC_COLOR8 | ILC_MASK, 1, 1) &&
			 (m_ilButton.Add(m_hiconButton) != -1) )
		{
			//	Grab the icon size
			m_sizeIcon.cx = bitmap.bmWidth;
			m_sizeIcon.cy = bitmap.bmHeight;

			//	All icon operations succeeded
			bIconOperationFailed = false;
		}
	}
	
	if (bIconOperationFailed && m_hiconButton)
	{
		DestroyIcon(m_hiconButton);
		m_hiconButton = NULL;
	}
}


// ---------------------------------------------------------------------------
//		* DrawItem													 [Public]
// ---------------------------------------------------------------------------
//	Draws button.

void
CCoolButton::DrawItem(
	LPDRAWITEMSTRUCT			lpDIS)
{
	//	We expect to be an owner drawn button
	if (lpDIS->CtlType != ODT_BUTTON)
		return;

	//	Get the device context
	CDC *		pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT(pDC);
	if (!pDC)
		return;

	CRect		rectEntireButton(lpDIS->rcItem);
	CRect		rectButtonContent(rectEntireButton);
	CRect		rectMenuContent(rectEntireButton);
	CRect		rectButtonBackground(rectEntireButton);
	CRect		rectMenuBackground(rectEntireButton);
	CRect		rectSplit(rectEntireButton);
	long		nTextWidth = 0;
	long		nTextAndIconOuterMargin = 0;
	long		nTextAndIconInnerMargin = kIconTextPadding;

	if (m_bMenuInitialized)
	{
		//	We have a menu, so the button content (text and/or icon) needs
		//	to leave space for the menu.
		rectButtonContent.right -= m_nMenuButtonWidth;

		//	The menu content (i.e. the arrow) starts immediately to the
		//	right of the button content.
		rectMenuContent.left = rectButtonContent.right;

		if (m_bIsSplit)
		{
			//	We're a split button, so the button background needs
			//	to leave space for the menu.
			rectButtonBackground.right -= m_nMenuButtonWidth;

			//	If the menu is pushed, then we adjust the background
			//	slightly so that the menu pressing is drawn better.
			if (m_bIsMenuPushed)
				rectButtonBackground.right--;

			//	The menu portion of the button background starts immediately
			//	to the right of the button background.
			rectMenuBackground.left = rectButtonBackground.right;
		}
	}

	//	Draw splitter if we think we should be split and we have menu items.
	bool		bDrawSplitter = m_bIsSplit && m_bMenuInitialized;
	if (bDrawSplitter)
	{
		//	Determine splitter rect
		rectSplit.DeflateRect(0, 3);
		rectSplit.left = rectMenuContent.left - 1;
		rectSplit.right = rectMenuContent.left;
	}

	//	Select the default font
	CFont *		pFont = CFont::FromHandle( static_cast<HFONT>( GetStockObject(DEFAULT_GUI_FONT) ) );

	//	Determine if we're disabled
	bool		bIsDisabled = (ODS_DISABLED & lpDIS->itemState) != 0;

	//	Is the button pushed?
	bool		bIsButtonPushed = false;
	if (m_bIsMenuPushed)
	{
		if (!m_bIsSplit)
			bIsButtonPushed = true;
	}
	else
	{
		bIsButtonPushed = (ODS_SELECTED & lpDIS->itemState) != 0;
	}

	//	Get the button caption
	CString		strCaption;
	GetWindowText(strCaption);

	//	Select the font
	CFont*		pOldFont = pDC->SelectObject(pFont);

	//	Determine the text and icon metrics and spacing
	CRect		rectText(rectButtonContent);
	long		nTextAndIconWidth = m_sizeIcon.cx;
	if ( !strCaption.IsEmpty() )
	{
		//	Calculate the text width via a call to DrawText (doesn't actually draw)
		pDC->DrawText(strCaption, rectText, DT_SINGLELINE | DT_CALCRECT);
		nTextWidth = rectText.Width();
		nTextAndIconWidth += nTextWidth;

		//	If there's an icon too, then space out the text and icon
		if (m_sizeIcon.cx)
			nTextAndIconWidth += nTextAndIconInnerMargin;
	}

	//	Calculate the available space for the button content
	long	nButtonContentAvailableWidth = 0;
	if (!m_bIsSplit && m_bMenuInitialized)
		nButtonContentAvailableWidth = rectEntireButton.Width() - m_sizeArrow.cx;
	else
		nButtonContentAvailableWidth = rectButtonContent.Width();

	//	Center the icon and text
	nTextAndIconOuterMargin = (nButtonContentAvailableWidth - nTextAndIconWidth) / 2;

	if (nTextAndIconOuterMargin < 0)
	{
		//	Not enough space - reduce the space between the icon and text
		nTextAndIconInnerMargin += nTextAndIconOuterMargin;
		if (nTextAndIconInnerMargin < 0)
			nTextAndIconInnerMargin = 0;

		nTextAndIconOuterMargin = 0;
	}

	//	Determine rect for text - with left edge immediately after icon
	rectText = rectButtonContent;
	rectText.left += nTextAndIconOuterMargin + m_sizeIcon.cx + nTextAndIconInnerMargin;

	//	Are we using XP style themed drawing?
	bool		bShouldUseXPThemedDrawing = m_xpThemedApppearance.ShouldUseXPThemedDrawing();

	//	Our strategy for drawing split buttons involves expanding the rect to the right
	//	or left and clipping the drawing so that we get what we want.
	//	For example we draw overlapped like		==>	But we clip the drawing and draw a
	//	this ("xxxx" designates overlap):			splitter so that it looks like this:
	//	+----------+-----+----+						+-------------+-------+
	//  |          |xxxxx|    |						|             :       |
	//  +----------+-----+----+						+-------------+-------+
	CRect		rectClipButtonHalf(rectButtonBackground);
	CRect		rectConnectButtonBackgrounds(rectButtonBackground);

	if (bDrawSplitter)
	{
		//	We're drawing split, so overlap the button portion to the right
		rectConnectButtonBackgrounds.right += 5;

		//	Special case to make non-XP drawing look better when button
		//	is pushed.
		if (!bShouldUseXPThemedDrawing && !bIsDisabled && m_bIsMenuPushed)
			rectClipButtonHalf.right++;
	}

	//	Draw button portion background
	HRESULT		hr;
	int			nButtonStateID = PBS_NORMAL;
	UINT		uButtonStateID = DFCS_BUTTONPUSH;

	if (bShouldUseXPThemedDrawing)
	{
		//	Determine theme state ID
		if (bIsDisabled)
			nButtonStateID = PBS_DISABLED;
		else if (bIsButtonPushed)
			nButtonStateID = PBS_PRESSED;
		else if (!m_bIsMenuPushed)
		{
			if (m_bIsMouseOverButton)
				nButtonStateID = PBS_HOT;
			else if (m_bIsDefaultButton)
				nButtonStateID = PBS_DEFAULTED;
		}

		//	Draw theme background, clipping it appropriately
		hr = m_xpThemedApppearance.DrawThemeBackground( *pDC, BP_PUSHBUTTON, nButtonStateID,
														&rectConnectButtonBackgrounds,
														&rectClipButtonHalf );
		ASSERT( SUCCEEDED(hr) );
	}
	else
	{
		//	Determine state ID flag field
		if (bIsDisabled)
			uButtonStateID |= DFCS_INACTIVE;
		else if (bIsButtonPushed)
			uButtonStateID |= DFCS_PUSHED;

		//	Clip the drawing ourselves
		CRgn		rgnClip;

		rgnClip.CreateRectRgnIndirect(&rectClipButtonHalf);	
		pDC->SelectClipRgn(&rgnClip);
		rgnClip.DeleteObject();	

		//	Draw old style background
		pDC->DrawFrameControl(&rectConnectButtonBackgrounds, DFC_BUTTON, uButtonStateID);

		//	Clear clipping
		pDC->SelectClipRgn(NULL);
	}

	if (bDrawSplitter)
	{
		//	Determine rects for drawing menu portion
		rectConnectButtonBackgrounds = rectMenuBackground;
		rectClipButtonHalf = rectMenuBackground;

		//	Most of the time we want to overlap to the left, but when
		//	pushed and using non-themed drawing we want to see the
		//	indented button, so we adjust by less in that case.
		if (bShouldUseXPThemedDrawing || bIsDisabled || !m_bIsMenuPushed)
			rectConnectButtonBackgrounds.left -= 5;
		else
			rectClipButtonHalf.left++;
		
		//	Draw menu portion of button
		int		nMenuStateID = PBS_NORMAL;
		UINT	uMenuStateID = DFCS_BUTTONPUSH;

		if (bShouldUseXPThemedDrawing)
		{
			//	Determine theme state ID
			if (bIsDisabled)
				nMenuStateID = PBS_DISABLED;
			else if (m_bIsMenuPushed)
				nMenuStateID = PBS_PRESSED;
			else if (!bIsButtonPushed)
				nMenuStateID = nButtonStateID;

			//	Draw theme background, clipping it appropriately
			hr = m_xpThemedApppearance.DrawThemeBackground( *pDC, BP_PUSHBUTTON, nMenuStateID,
															&rectConnectButtonBackgrounds,
															&rectClipButtonHalf );
			ASSERT( SUCCEEDED(hr) );
		}
		else
		{
			//	Determine state ID flag field
			if (bIsDisabled)
				uMenuStateID |= DFCS_INACTIVE;
			else if (m_bIsMenuPushed)
				uMenuStateID |= DFCS_PUSHED;

			//	Clip the drawing ourselves
			CRgn		rgnClip;

			rgnClip.CreateRectRgnIndirect(&rectClipButtonHalf);	
			pDC->SelectClipRgn(&rgnClip);
			rgnClip.DeleteObject();	

			//	Draw old style background
			pDC->DrawFrameControl(&rectConnectButtonBackgrounds, DFC_BUTTON, uMenuStateID);

			//	Clear clipping
			pDC->SelectClipRgn(NULL);
		}
	}

	if ( !bShouldUseXPThemedDrawing && !bIsDisabled &&
		 (m_bIsDefaultButton || bIsButtonPushed || m_bIsMenuPushed) )
	{
		//	Old style default or pushed buttons need to a black frame rectangle.
		pDC->FrameRect( &rectEntireButton, CBrush::FromHandle( (HBRUSH) GetStockObject(BLACK_BRUSH) ) );
	}

	//	Draw the button text, if any
	if ( !strCaption.IsEmpty() )
	{
		if (bShouldUseXPThemedDrawing)
		{
			//	Draw the button text - it's easy with themes
			m_xpThemedApppearance.DrawThemeText( *pDC, BP_PUSHBUTTON, nButtonStateID,
												 strCaption, strCaption.GetLength(),
												 DT_SINGLELINE | DT_VCENTER, 0,
												 &rectText );
		}
		else
		{
			//	Set the drawing mode and color
			int			nOldBkMode = pDC->SetBkMode(TRANSPARENT);
			COLORREF	crOldColor = pDC->SetBkColor( GetSysColor(COLOR_BTNFACE) );

			if (bIsDisabled)
			{
				//	Draw the disabled text
				rectText.OffsetRect(1, 1);
				pDC->SetTextColor( GetSysColor(COLOR_WINDOW) );
				pDC->DrawText(strCaption, rectText, DT_SINGLELINE | DT_VCENTER);

				rectText.OffsetRect(-1, -1);
				pDC->SetTextColor( GetSysColor(COLOR_GRAYTEXT) );
				pDC->DrawText(strCaption, rectText, DT_SINGLELINE | DT_VCENTER);
			}
			else
			{
				//	Move text down and to the right if pushed
				if (bIsButtonPushed)
					rectText.OffsetRect(1, 1);
				
				//	Draw the text
				pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				pDC->DrawText(strCaption, rectText, DT_SINGLELINE | DT_VCENTER);
			}

			//	Restore the old drawing mode and color
			pDC->SetBkMode(nOldBkMode);
			pDC->SetBkColor(crOldColor);
		}
	}

	//	Done drawing text - restore the old font
	pDC->SelectObject(pOldFont);

	//	Determine if we need a focus rect
	if ( (!m_bIsMenuPushed || !m_bIsSplit) && (bIsButtonPushed || m_bIsDefaultButton) )
	{
		//	Is the focus rect the entire button, or are we split?
		CRect		rectFocus = (m_bMenuInitialized && !m_bIsSplit) ? rectEntireButton : rectButtonContent;

		//	With themed drawing, the focus rect is closer to the border than
		//	it is without themed drawing.
		if (bShouldUseXPThemedDrawing)
			rectFocus.DeflateRect(2, 2);
		else
			rectFocus.DeflateRect(3, 3);
		
		//	Draw the focus rect
		::DrawFocusRect(*pDC, &rectFocus);
	}

	//	Draw button icon if we have one
	if (m_hiconButton)
	{
		//	Draw shifted down and to the right when we're not using XP theme drawing
		short		nShiftIconWhenPushed = bIsButtonPushed && !bShouldUseXPThemedDrawing ? 1 : 0;
		
		CPoint		ptIcon;

		ptIcon.x = nTextAndIconOuterMargin + nShiftIconWhenPushed;
		ptIcon.y = (rectEntireButton.bottom - rectEntireButton.top - m_sizeIcon.cy) / 2 + nShiftIconWhenPushed;
		
		pDC->DrawState( ptIcon, CSize(0,0), m_hiconButton,
						IsWindowEnabled() ? DSS_NORMAL : DSS_DISABLED,
						static_cast<HBRUSH>(NULL) );
	}

	if (bDrawSplitter)
	{
		//	Draw the splitter by hand. There may be a better way to do this with themes.
		CPen		brFace(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
		CPen *		pOldPen = pDC->SelectObject(&brFace);
		pDC->MoveTo(rectSplit.left, rectSplit.top);
		pDC->LineTo(rectSplit.left, rectSplit.bottom);

		if (!m_bIsMenuPushed)
		{
			CPen brLite(PS_SOLID,1,GetSysColor(COLOR_3DHILIGHT));
			pDC->SelectObject(&brLite);
			pDC->MoveTo(rectSplit.right , rectSplit.top);
			pDC->LineTo(rectSplit.right, rectSplit.bottom);
		}

		//	Restore the old pen
		pDC->SelectObject(pOldPen);
	}

	//	Draw the menu drop down arrow
	if (m_bMenuInitialized)
	{
		//	Draw shifted down and to the right when we're not using XP theme drawing
		short	nShiftArrowWhenPushed = m_bIsMenuPushed && !bShouldUseXPThemedDrawing ? 1 : 0;
		
		CPoint pt(rectMenuContent.CenterPoint());
		pt += CPoint(nShiftArrowWhenPushed, nShiftArrowWhenPushed);

		DrawArrow(pDC,pt);
	}
}


// ---------------------------------------------------------------------------
//		* DrawArrow												  [Protected]
// ---------------------------------------------------------------------------
//	Draws menu arrow for button.

void
CCoolButton::DrawArrow(
	CDC *						in_pDC,
	CPoint						in_ptArrowCenter)
{
	CPoint		ptArrowUpperLeft;

	ptArrowUpperLeft.x = in_ptArrowCenter.x - m_sizeArrow.cx  / 2;
	ptArrowUpperLeft.y = in_ptArrowCenter.y - m_sizeArrow.cy / 2;

	in_pDC->DrawState( ptArrowUpperLeft, CSize(0,0), m_hiconArrow,
					   IsWindowEnabled() ? DSS_NORMAL : DSS_DISABLED,
					   static_cast<HBRUSH>(NULL) );
}


// ---------------------------------------------------------------------------
//		* PtInMenuRect											  [Protected]
// ---------------------------------------------------------------------------
//	Detects whether or not a given point is within the menu portion of
//	the button.

BOOL
CCoolButton::PtInMenuRect(
	CPoint						in_point)
{
	if (!m_bMenuInitialized)
		return FALSE; // Don't allow menu button drop down if no menu items are loaded

	ClientToScreen(&in_point);
	CRect rect;
	GetWindowRect(rect);
	rect.left = rect.right - m_nMenuButtonWidth;

	return rect.PtInRect(in_point);
}


// ---------------------------------------------------------------------------
//		* OnMenuButton											  [Protected]
// ---------------------------------------------------------------------------
//	Handles menu click (or equivalent user event) and displays menu.

void
CCoolButton::OnMenuButton()
{
	if (m_bMenuInitialized)
	{
		//	Set that the menu is pushed
		m_bIsMenuPushed = true;
		
		//	Invalidate
		Invalidate();

		//	Track the menu choice
		CRect		rectWindow;
		GetWindowRect(rectWindow);

		CommandRouteMenu( GetParent(), &m_menu );

		int		nAlignWith = m_bLeftAlignMenu ? rectWindow.left : rectWindow.right;
		UINT	nAlignFlag = m_bLeftAlignMenu ? TPM_LEFTALIGN : TPM_RIGHTALIGN;

		m_menu.TrackPopupMenu( nAlignFlag | TPM_LEFTBUTTON,
							   nAlignWith, rectWindow.bottom,
							   GetParent() );

		//	Check for left button down event
		MSG		msg;
		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		if (msg.message == WM_LBUTTONDOWN)
		{
			//	Pump the left button down event so that we process it while
			//	m_bIsMenuPushed is still true (so that we dismiss the menu
			//	with the left-click rather than re-displaying it).
			AfxPumpMessage();
		}

		//	Set that the menu is not pushed any more
		m_bIsMenuPushed = false;

		//	Invalidate
		Invalidate();
	}
}


// ---------------------------------------------------------------------------
//		* CommandRouteMenu										  [Protected]
// ---------------------------------------------------------------------------
//	Command routes the button's menu so that ON_UPDATE_COMMAND_UI, etc.
//	can be used.

void
CCoolButton::CommandRouteMenu(
	CWnd *						in_pWnd,
	CMenu *						in_pPopupMenu)
{
	CCmdUI		state;
	state.m_pMenu = in_pPopupMenu;
	state.m_pParentMenu = in_pPopupMenu;
	state.m_nIndexMax = in_pPopupMenu->GetMenuItemCount();

	for ( state.m_nIndex = 0; 
		  state.m_nIndex < state.m_nIndexMax;
		  state.m_nIndex++ ) 
	{
		state.m_nID = in_pPopupMenu->GetMenuItemID(state.m_nIndex);

		//	If it doesn't have an ID it's a menu separator or an invalid
		//	command so ignore it.
		if (state.m_nID == 0) continue; 

		if (state.m_nID == (UINT)-1)
		{
			//	Check for hierarchical sub menu and route to sub menu if found
			CMenu * pSub = in_pPopupMenu->GetSubMenu(state.m_nIndex);
			if (pSub)
				CommandRouteMenu(in_pWnd,pSub);
		}
		else 
		{
			//	Normal menu item. Disables if command is not a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(in_pWnd, FALSE);
		}
	}
}
