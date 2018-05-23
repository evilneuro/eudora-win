/*////////////////////////////////////////////////////////////////////////////

NAME:
	CLsButton - My button

FILE:		VolumeBar.cpp
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
04/16/97   lss     -Initial

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "qvoice.h"
#include "resource.h"

// LOCAL INCLUDES
#include "LsButton.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CLsButton::CLsButton() : mb_select(FALSE), mb_enable(TRUE), mb_focus(FALSE)
{
}

CLsButton::~CLsButton()
{
}

/////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CLsButton::AutoLoad( UINT nID, UINT nBmpID, CWnd* pParent )
{
	// first attach the CLsButton to the dialog control
	if (!SubclassDlgItem(nID, pParent))
		return FALSE;

	m_bmpBtnUp.DeleteObject();

	if (!m_bmpBtnUp.LoadBitmap(nBmpID))
	{
		TRACE0("Failed to load bitmap for normal image.\n");
		return FALSE;   // need this one image
	}
//	m_bmpBtnDs.LoadBitmap(IDB_TESTX);

	// size to content
	SizeToContent();

	makeOtherBitmap( IDB_TESTX, m_bmpBtnDs, TRUE, FALSE );

	return TRUE;
}

// SizeToContent will resize the button to the size of the bitmap + border
void CLsButton::SizeToContent()
{
	ASSERT(m_bmpBtnUp.m_hObject != NULL);
	BITMAP bmInfo;
	VERIFY(m_bmpBtnUp.GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo));
	m_sizeBtn.cx = bmInfo.bmWidth;
	m_sizeBtn.cy = bmInfo.bmHeight;
	VERIFY(SetWindowPos(NULL, -1, -1, m_sizeBtn.cx+6+4, m_sizeBtn.cy+6+3,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE));
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void CLsButton::drawBtnUp( CDC& dc, CRect& rc )
{
	drawBtnUpBorder( dc, rc );

	CPoint dp = CPoint(4,4), sp = CPoint(0,0);
	drawBitmap( dc, m_bmpBtnUp, m_sizeBtn, FALSE, &dp, &sp, TRUE );

	if ( mb_focus )
		drawBtnFc( dc, rc );
}

void CLsButton::drawBtnDn( CDC& dc, CRect& rc )
{
	CBrush br1( COLORREF(GetSysColor(COLOR_3DDKSHADOW)) );
	dc.FrameRect( rc, &br1 );			
	dc.DrawEdge( rc, BDR_SUNKENINNER, BF_TOPLEFT );

	CPoint dp = CPoint(4,4), sp = CPoint(0,0);
	drawBitmap( dc, m_bmpBtnUp, m_sizeBtn, TRUE, &dp, &sp, TRUE );
}

void CLsButton::drawBtnFc( CDC& dc, CRect& rc )
{
	drawBtnUpBorder( dc, rc );

	rc.InflateRect( -1, -1 );
	dc.DrawFocusRect( rc );
}

void CLsButton::drawBtnDs( CDC& dc, CRect& rc )
{
	drawBtnUpBorder( dc, rc );

	CPoint dp = CPoint(4,4), sp = CPoint(0,0);
	drawBitmap( dc, m_bmpBtnDs, m_sizeBtn, FALSE, &dp, &sp, TRUE );
}

void CLsButton::drawBtnUpBorder( CDC& dc, CRect& rc )
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

BOOL CLsButton::makeOtherBitmap(
	UINT		  nIDResource,
	CBitmap&      destBitmap,
	BOOL          bDrawTransParent,
	BOOL          bDrawDepressed
)
{
	CClientDC dc(this);

	CRect rc;

    // Create memory DCs
	CDC srcMemDC;
//	CDC destMemDC;

	CBitmap srcBitmap;

	srcMemDC.CreateCompatibleDC( &dc );
//	destMemDC.CreateCompatibleDC( &dc );

	CBitmap* pSrcOldBmp;
//	CBitmap* pDestOldBmp;

	srcBitmap.DeleteObject();
	if ( !srcBitmap.LoadBitmap(nIDResource) )// ||
//		 !destBitmap.LoadBitmap(nIDResource) )
	 {
		TRACE0("Failed to load bitmap!\n");
		return FALSE;
	 }

	BITMAP bmInfo;
	VERIFY(m_bmpBtnUp.GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo));
	CSize bmpSize( bmInfo.bmWidth, bmInfo.bmHeight );

 	// Create the memory bitmap.
	//destBitmap.CreateCompatibleBitmap( &destMemDC, bmpSize.cx, bmpSize.cy );

	try {
		pSrcOldBmp = srcMemDC.SelectObject( &srcBitmap );
//		pDestOldBmp = destMemDC.SelectObject( &destBitmap );
	}
	catch (CException* e) {
		TRACE("ERROR!!!!! ERROR!!!!!\n");
		e->Delete();
	}
 
	CBitmap& memBitmap = destBitmap;

	if (bDrawTransParent)
	 {
		CBitmap imageBitmap;
		CBitmap maskBitmap;

		CDC maskDC;
		CDC imageDC;

		// Create compatible dc's
		maskDC.CreateCompatibleDC( &dc );
		imageDC.CreateCompatibleDC( &dc );

		CPalette* pImagePalette = imageDC.SelectPalette(
				dc.GetCurrentPalette(), TRUE );

		// Create the memory bitmap.
		memBitmap.CreateCompatibleBitmap( &dc, bmpSize.cx, bmpSize.cy );

		// Create a compatible dc for the source image.
		imageBitmap.CreateCompatibleBitmap( &dc, bmpSize.cx, bmpSize.cy );

		// Create a mask bitmap.
		maskBitmap.CreateBitmap( bmpSize.cx, bmpSize.cy, 1, 1, NULL );

		// Select them into their dc's
		CBitmap* oldImage = imageDC.SelectObject( &imageBitmap );
		
		// Copy the source image into the image dc.
		imageDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &srcMemDC, 0, 0, 
				SRCCOPY );

		// Select the memory bitmap in now.
		srcMemDC.SelectObject( &memBitmap );

		CBitmap* oldMask = maskDC.SelectObject( &maskBitmap );

		// Copy the background into the memory dc.
//		srcMemDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy,
//				&dc, 0, 0, SRCCOPY );
		// fill in our background with the btn face color
		GetWindowRect( rc );
		rc -= rc.TopLeft();
		srcMemDC.FillSolidRect( rc, COLORREF(GetSysColor(COLOR_3DFACE)) );

		// Create the mask.
		// All bits matching the transparent color are set to white (1)
		// in the mask bitmap. All other bits are set to black (0).
		imageDC.SetBkColor( ::GetPixel(imageDC,0,0) ); // Pink Usually?
		maskDC.BitBlt( 0, 0, bmpSize.cx, bmpSize.cy, &imageDC, 0, 0,
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

		// Do a little clean up.
		maskDC.SelectObject( oldMask );
		imageDC.SelectObject( oldImage );

		imageDC.SelectPalette( pImagePalette, TRUE );

		maskDC.DeleteDC();
		imageDC.DeleteDC();
	 }

	BOOL bRet = FALSE;

	// test
//	CRect rc( 1, 1, bmpSize.cx+8, bmpSize.cy+7 );
//	drawBtnUpBorder( destMemDC, rc );
//	CPoint dp = CPoint(4,4), sp = CPoint(0,0);

//	drawBitmap( destMemDC, m_bmpBtnDs, bmpSize, FALSE, &dp, &sp, TRUE );

//	bRet = destMemDC.BitBlt((bDrawDepressed ? 1 : 0),
//		(bDrawDepressed ? 1 : 0),
//		bmpSize.cx, bmpSize.cy,
//		&srcMemDC, 0, 0, SRCCOPY );

	HBITMAP newBitmap = (HBITMAP)::SelectObject( srcMemDC.GetSafeHdc(), pSrcOldBmp );
	srcMemDC.DeleteDC();
//	destBitmap.Attach( newBitmap );

//	newBitmap = (HBITMAP)::SelectObject( destMemDC.GetSafeHdc(), pDestOldBmp );
//	destMemDC.SelectObject( pDestOldBmp );
//	destMemDC.DeleteDC();

//	destBitmap.Attach( newBitmap );

	return TRUE;//bRet;
}

BOOL CLsButton::drawBitmap(
	CDC&          dc,
	CBitmap&      bitmap,
	CSize&		  sizeBitmap,	
	BOOL          bDrawDepressed,
	const CPoint* destPoint,
	const CPoint* srcPoint,
	BOOL          bDrawTransParent
)
{
	CSize* destSize = &sizeBitmap;

	// Compute rectangles where NULL specified
	if ( !destPoint )
		destPoint = &CPoint(0,0);
  
	if ( !srcPoint )
		srcPoint = destPoint;
 
        // Create memory DC
	CDC memDC;
	memDC.CreateCompatibleDC( &dc );
 
	CBitmap* pOldBm;
 
	try {
		pOldBm = memDC.SelectObject( &bitmap );
	}
	catch (CException* e) {
		TRACE("ERROR!!!!! ERROR!!!!!\n");
		e->Delete();
	}
 
	CBitmap memBitmap;
/*
	if (bDrawTransParent)
	 {
		CBitmap imageBitmap;
		CBitmap maskBitmap;

		CDC maskDC;
		CDC imageDC;

		// Create compatible dc's
		maskDC.CreateCompatibleDC( &dc );
		imageDC.CreateCompatibleDC( &dc );

		CPalette* pImagePalette = imageDC.SelectPalette(
				dc.GetCurrentPalette(), TRUE );

		// Create the memory bitmap.
		memBitmap.CreateCompatibleBitmap( &dc, destSize->cx, destSize->cy );

		// Create a compatible dc for the source image.
		imageBitmap.CreateCompatibleBitmap( &dc, destSize->cx, destSize->cy );

		// Create a mask bitmap.
		maskBitmap.CreateBitmap( destSize->cx, destSize->cy, 1, 1, NULL );

		// Select them into their dc's
		CBitmap* oldImage = imageDC.SelectObject( &imageBitmap );

		// Copy the source image into the image dc.
		imageDC.BitBlt( 0, 0, destSize->cx, destSize->cy, &memDC, 0, 0, 
				SRCCOPY );

		// Select the memory bitmap in now.
		memDC.SelectObject( &memBitmap );
		CBitmap* oldMask = maskDC.SelectObject( &maskBitmap );

		// Copy the background into the memory dc.
		memDC.BitBlt( 0, 0, destSize->cx, destSize->cy,
				&dc, destPoint->x, destPoint->y, SRCCOPY );

		// Create the mask.
		// All bits matching the transparent color are set to white (1)
		// in the mask bitmap. All other bits are set to black (0).
		imageDC.SetBkColor( ::GetPixel(imageDC,0,0) ); // Pink Usually?
		maskDC.BitBlt( 0, 0, destSize->cx, destSize->cy, &imageDC, 0, 0,
				SRCCOPY );

		// Use the mask to copy the image to the dc.

		// XOR the screen with the bitmap.
		memDC.SetBkColor( RGB(0xFF, 0xFF, 0xFF) );
		memDC.BitBlt( 0, 0, destSize->cx, destSize->cy, &imageDC, 0, 0,
				SRCINVERT );

		// AND the screen and the mask.
		memDC.BitBlt (0,0, destSize->cx, destSize->cy, &maskDC, 0, 0,
				SRCAND );

		// XOR the screen and the image again.
		memDC.BitBlt (0,0, destSize->cx, destSize->cy, &imageDC, 0, 0,
				SRCINVERT );

		// Do a little clean up.
		maskDC.SelectObject( oldMask );
		imageDC.SelectObject( oldImage );

		imageDC.SelectPalette( pImagePalette, TRUE );

		maskDC.DeleteDC();
		imageDC.DeleteDC();
	 }
*/ 
	// Blast bits from memory DC to target DC.
	BOOL bRet = FALSE;

	bRet = dc.BitBlt( destPoint->x+(bDrawDepressed ? 1 : 0),
		destPoint->y+(bDrawDepressed ? 1 : 0),
		destSize->cx, destSize->cy,
		&memDC, srcPoint->x, srcPoint->y, SRCCOPY );

	memDC.SelectObject( pOldBm );
	memDC.DeleteDC();

	return bRet;
}

//////////////////////////////////////////////////////////////////////////////
// Message Handlers, Appwizard Generated Stuff ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CLsButton message handlers

void CLsButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC *pDC;

	CRect rc = lpDrawItemStruct->rcItem;

	// Get the device context from the context
	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	VERIFY(pDC);
	pDC->DrawFrameControl( rc, DFCS_BUTTONPUSH, 
										DFCS_ADJUSTRECT | DFCS_PUSHED );
	rc.InflateRect( 1, 1, 0, 0 );

	switch(lpDrawItemStruct->itemAction)
	{
		case ODA_SELECT:
			if ( ODS_SELECTED & lpDrawItemStruct->itemState )
			 {
				mb_select = TRUE;
				drawBtnDn( *pDC, rc );
				TRACE0("ODA_SELECT - select\n");
			 }
			else
			 {
				mb_select = FALSE;
				drawBtnUp( *pDC, rc );
				TRACE0("ODA_SELECTED - unselect\n");
			 }

		 break;

		case ODA_DRAWENTIRE:
			if ( ODS_DISABLED & lpDrawItemStruct->itemState )
			 {
				mb_enable = FALSE;
				drawBtnDs( *pDC, rc );
				TRACE0("ODA_DRAWENTIRE - disable\n");
			 }
			else
			 {
				mb_enable = TRUE;
				if ( mb_select )
					drawBtnDn( *pDC, rc );
				else
					drawBtnUp( *pDC, rc );
				TRACE0("ODA_DRAWENTIRE - normal\n");
			 }
			// Drop through to draw Frame.
		 break;
		case ODA_FOCUS:
			if ( ODS_FOCUS & lpDrawItemStruct->itemState )
			 {
				mb_focus = TRUE;
				drawBtnFc( *pDC, rc );
				TRACE0("ODA_FOCUS Gained\n");
			 }
			else
			 {
				mb_focus = FALSE;
				drawBtnUp( *pDC, rc );
				TRACE0("ODA_FOCUS Lost\n");
			 }
	}	
}

BEGIN_MESSAGE_MAP(CLsButton, CButton)
	//{{AFX_MSG_MAP(CLsButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()






