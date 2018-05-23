/*////////////////////////////////////////////////////////////////////////////

NAME:
	CLsBmpButton - Transparent Bitmap Button Class

FILE:		LsBmpButton.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	Derived from CButton.

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/30/97   lss     -Initial
08/15/97   lss     -Will replace RGB(255,255,255) with COLOR_BTNHILIGHT and
					RGB(128, 128, 128) with COLOR_BTNSHADOW

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "qvoice.h"
#include "resource.h"

// LOCAL INCLUDES
#include "LsBmpButton.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CLsBmpButton::CLsBmpButton() : m_borderStyle(BORDER_NONE), 
	m_transparentColor(RGB(192, 192, 192))
{
}

CLsBmpButton::~CLsBmpButton()
{
}

/////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CLsBmpButton::AutoLoad( UINT nID, CWnd* pParent, int borderStyle )
{
	// first attach the CLsBmpButton to the dialog control
	if ( !SubclassDlgItem(nID, pParent) )
		return FALSE;

	m_borderStyle = borderStyle;

	CString buttonName;
	GetWindowText( buttonName );
	ASSERT( !buttonName.IsEmpty() );

	CBitmap bmpBtn;
	if ( !bmpBtn.LoadBitmap( buttonName + _T("U") ) )
	 {
		TRACE0("Failed to load bitmap!\n");
		return FALSE;
	 }
	BITMAP bmInfo;
	VERIFY( bmpBtn.GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo) );
	m_sizeBtn.cx = bmInfo.bmWidth; m_sizeBtn.cy = bmInfo.bmHeight;
	bmpBtn.DeleteObject();

	m_bmpBtnUp.DeleteObject();
	m_bmpBtnDn.DeleteObject();
	m_bmpBtnFc.DeleteObject();
	m_bmpBtnDs.DeleteObject();

	// size to content
	SizeToContent();
	
	makeOtherBitmap( buttonName + _T("U"), m_bmpBtnUp, TRUE, borderStyle );
	makeOtherBitmap( buttonName + _T("D"), m_bmpBtnDn, TRUE, borderStyle );
	makeOtherBitmap( buttonName + _T("F"), m_bmpBtnFc, TRUE, borderStyle );
	makeOtherBitmap( buttonName + _T("X"), m_bmpBtnDs, TRUE, borderStyle );

	return TRUE;
}

// SizeToContent will resize the button to the size of the bitmap + border
void CLsBmpButton::SizeToContent()
{
	int width = m_sizeBtn.cx;
	int height = m_sizeBtn.cy;

	if ( BORDER_NONE != m_borderStyle )
	 {
		width = m_sizeBtn.cx+6+4;
		height = m_sizeBtn.cy+6+3;
	 }

	VERIFY(SetWindowPos(NULL, -1, -1, width, height,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE));
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void CLsBmpButton::drawBorder( CDC& dc, CRect& rc, int borderStyle )
{
	if ( BORDER_NONE == borderStyle ) return;

	// draw outer border
	dc.DrawFrameControl( rc, DFCS_BUTTONPUSH, 
										DFCS_ADJUSTRECT | DFCS_PUSHED );
	rc.InflateRect( 1, 1, 0, 0 );

	// draw inner border
	if ( BORDER_UP == borderStyle )
	 {
		CBrush br1( COLORREF(GetSysColor(COLOR_3DDKSHADOW)) );
		dc.FrameRect( rc, &br1 );			
		CBrush br2( COLORREF(GetSysColor(COLOR_BTNHILIGHT)) );
		rc.InflateRect( -1, -1 );
		dc.FrameRect( rc, &br2 );
		CBrush br3( COLORREF(GetSysColor(COLOR_BTNSHADOW)) );
		rc.InflateRect( 1, 1, 0, 0 );
		dc.FrameRect( rc, &br3 );
		rc.InflateRect( -2, -2 );
	 }
	else
	 {
		CBrush br1( COLORREF(GetSysColor(COLOR_3DDKSHADOW)) );
		dc.FrameRect( rc, &br1 );			
		dc.DrawEdge( rc, BDR_SUNKENINNER, BF_TOPLEFT );
	 }
/*
	// focus
	rc.InflateRect( -1, -1 );
	dc.DrawFocusRect( rc );
*/
}

BOOL CLsBmpButton::makeOtherBitmap(
	LPCTSTR		  bitmapResource,
	CBitmap&      destBitmap,
	BOOL          bDrawTransParent,
	int	          borderStyle
)
{
	CClientDC dc(this);

	CRect rc;

    // Create memory DCs
	CDC srcMemDC;

	CBitmap srcBitmap;

	srcMemDC.CreateCompatibleDC( &dc );

	srcBitmap.DeleteObject();
	if ( !srcBitmap.LoadBitmap(bitmapResource) )
	 {
		TRACE0("Failed to load bitmap!\n");
		return FALSE;
	 }

	GetWindowRect( rc );
	rc -= rc.TopLeft();
	CSize bmpSize = m_sizeBtn;//(rc.Width(), rc.Height());

	CBitmap* pSrcOldBmp = srcMemDC.SelectObject( &srcBitmap );

	CBitmap& memBitmap = destBitmap;
	CPoint srcUpLeft(0,0);
	if ( BORDER_NONE != borderStyle ) srcUpLeft = CPoint(4,4);
	if ( bDrawTransParent )
	 {
		CBitmap imageBitmap0;
		CBitmap imageBitmap;
		CBitmap maskBitmap;

		CDC maskDC;
		CDC imageDC0;
		CDC imageDC;

		// Create compatible dc's
		maskDC.CreateCompatibleDC( &dc );

		imageDC0.CreateCompatibleDC( &dc );
		imageDC.CreateCompatibleDC( &dc );

		CPalette* pImagePalette0 = imageDC0.SelectPalette(
				dc.GetCurrentPalette(), TRUE );
		CPalette* pImagePalette = imageDC.SelectPalette(
				dc.GetCurrentPalette(), TRUE );

		// Create the memory bitmap.
		memBitmap.CreateCompatibleBitmap( &dc, rc.Width(), rc.Height() );

		// Create a compatible dc for the source image.
		imageBitmap0.CreateCompatibleBitmap( &dc, bmpSize.cx, bmpSize.cy );
		imageBitmap.CreateCompatibleBitmap( &dc, bmpSize.cx, bmpSize.cy );

		// Create a mask bitmap.
		maskBitmap.CreateBitmap( bmpSize.cx, bmpSize.cy, 1, 1, NULL );

		// Select them into their dc's
		CBitmap* oldImage0 = imageDC0.SelectObject( &imageBitmap0 );
		CBitmap* oldImage = imageDC.SelectObject( &imageBitmap );
		
		// Copy the source image into the image dc.
		imageDC0.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &srcMemDC, 0, 0, 
				SRCCOPY );
		imageDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &srcMemDC, 0, 0, 
				SRCCOPY );

		// Select the memory bitmap in now.
		srcMemDC.SelectObject( &memBitmap );

		CBitmap* oldMask = maskDC.SelectObject( &maskBitmap );

		// Copy the background into the memory dc.
//		srcMemDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy,
//				&dc, 0, 0, SRCCOPY );

		int newColor[3] = { COLOR_3DFACE, COLOR_BTNHILIGHT, COLOR_BTNSHADOW };
		COLORREF oldColor[3] = { RGB(192, 192, 192), RGB(255, 255, 255), RGB(128, 128, 128) }; 
		
		for ( int i = 0; i < 3; i++ )
		 {
		// fill in our background with the btn face color
		//srcMemDC.FillSolidRect( rc, COLORREF(GetSysColor(COLOR_3DFACE)) );
		srcMemDC.FillSolidRect( rc, COLORREF(GetSysColor(newColor[i])) );

		if ( 0 == i ) drawBorder( srcMemDC, rc, borderStyle );

		// Create the mask.
		// All bits matching the transparent color are set to white (1)
		// in the mask bitmap. All other bits are set to black (0).
		if ( borderStyle != BORDER_NONE )
			m_transparentColor = ::GetPixel( imageDC0, 0, i );
		else
			m_transparentColor = oldColor[i];

		imageDC0.SetBkColor( m_transparentColor );
		imageDC.SetBkColor( m_transparentColor );
		maskDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &imageDC0, 0, 0,
				SRCCOPY );

		// Use the mask to copy the image to the dc.

		// XOR the screen with the bitmap.
		srcMemDC.SetBkColor( RGB(0xFF, 0xFF, 0xFF) );
		srcMemDC.BitBlt( srcUpLeft.x, srcUpLeft.y, bmpSize.cx, bmpSize.cy,
				&imageDC, 0, 0,	SRCINVERT );

		// AND the screen and the mask.
		srcMemDC.BitBlt ( srcUpLeft.x, srcUpLeft.y, bmpSize.cx, bmpSize.cy,
				&maskDC, 0, 0, SRCAND );

		// XOR the screen and the image again.
		srcMemDC.BitBlt ( srcUpLeft.x, srcUpLeft.y, bmpSize.cx, bmpSize.cy,
				&imageDC, 0, 0,	SRCINVERT );

		imageDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &srcMemDC, 0, 0, 
			SRCCOPY );

		 }
		// Do a little clean up.
		maskDC.SelectObject( oldMask );
		imageDC0.SelectObject( oldImage0 );
		imageDC.SelectObject( oldImage );

		imageDC0.SelectPalette( pImagePalette0, TRUE );
		imageDC.SelectPalette( pImagePalette, TRUE );

		maskDC.DeleteDC();
		imageDC0.DeleteDC();
		imageDC.DeleteDC();
	 }
	else
		drawBorder( srcMemDC, rc, borderStyle );

	srcMemDC.SelectObject( pSrcOldBmp );
	srcMemDC.DeleteDC();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// Message Handlers, Appwizard Generated Stuff ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CLsBmpButton message handlers

void CLsBmpButton::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	ASSERT(lpDIS != NULL);
	// must have at least the first bitmap loaded before calling DrawItem
	ASSERT(m_bmpBtnUp.m_hObject != NULL);     // required

	// use the up bitmap, the selected bitmap for down
	CBitmap* pBitmap = &m_bmpBtnUp;
	UINT state = lpDIS->itemState;
	if ((state & ODS_SELECTED) && m_bmpBtnDn.m_hObject != NULL)
		pBitmap = &m_bmpBtnDn;
	else if ((state & ODS_FOCUS) && m_bmpBtnFc.m_hObject != NULL)
		pBitmap = &m_bmpBtnFc;   // third image for focused
	else if ((state & ODS_DISABLED) && m_bmpBtnDs.m_hObject != NULL)
		pBitmap = &m_bmpBtnDs;   // last image for disabled

	// draw the whole button
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap* pOld = memDC.SelectObject(pBitmap);
	if (pOld == NULL)
		return;     // destructors will clean up

	CRect rect;
	rect.CopyRect(&lpDIS->rcItem);
	pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(),
		&memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOld);
}

BEGIN_MESSAGE_MAP(CLsBmpButton, CButton)
	//{{AFX_MSG_MAP(CLsBmpButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()






