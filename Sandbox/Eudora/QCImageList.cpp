
#include "stdafx.h"
#include "windowsx.h"
#include "QCGraphics.h"
#include "QCImageList.h"
#include "SafetyPal.h"
#include "resource.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class QCCommon16ImageList g_theCommon16ImageList;
class QCCommon32ImageList g_theCommon32ImageList;
class QCMailboxImageList g_theMailboxImageList;
class QCTocImageList g_theTocImageList;

void LoadImageLists(void)
{
	g_theCommon16ImageList.Load();
	g_theCommon32ImageList.Load();
	g_theMailboxImageList.Load();
	g_theTocImageList.Load();
}

bool QCCommon16ImageList::Load(void)
{
	return QCImageList::Load(16, 16, 40, MAKEINTRESOURCE(IDB_IL_COMMON16), RGB(192, 192, 192));
}

bool QCCommon32ImageList::Load(void)
{
	return QCImageList::Load(32, 32, 0, MAKEINTRESOURCE(IDB_IL_COMMON32), RGB(192, 192, 192));
}

bool QCMailboxImageList::Load(void)
{
	return QCImageList::Load(16, 16, 50, MAKEINTRESOURCE(IDB_IL_MAILTREE), RGB(192, 192, 192));
}

QCTocImageList::QCTocImageList()
{
}

bool QCTocImageList::Load(void)
{
	bool rc;
	rc = m_ImageListColor.Load(16, 16, 0, MAKEINTRESOURCE(IDB_IL_STATUS), RGB(192, 192, 192));
	if (!rc) 
	{
		ASSERT(0);
		return false;
	}

	rc = m_ImageListMonochrome.Load(16, 16, 0, MAKEINTRESOURCE(IDB_IL_STATUSM), RGB(0, 0, 0));
	if (!rc)
	{
		ASSERT(0);
		return false;
	}

	return true;
}

void QCTocImageList::Draw(int Index, int x, int y, CDC* pdc, COLORREF color)
{
	if (!pdc) 
	{
		ASSERT(0);
		return;
	}

	COLORREF ptcolor = pdc->GetPixel(x+2, y+2);
	short r = GetRValue(ptcolor);
	short g = GetGValue(ptcolor);
	short b = GetBValue(ptcolor);
	if (r < 150 || g < 150 || b < 150)
	{
		//
		// Draw Monochrome bitmap transparently on background, using
		// given text color.
		//

		//
		// Setup memory DCs.
		//
		CPaletteDC memDC1;		// holds Monochrome image
		CPaletteDC memDC2;		// temp DC for rendering and masking

		memDC1.CreateCDC();
		memDC2.CreateCDC();

		CRect rect(0, 0, TocBitmapWidth, TocBitmapHeight);		// for FillRect() calls below

		//
		// Render Monochrome image into DC1, taking care to initialize the
		// target DC1 first.
		//
		CBitmap bmpTemp1;
		bmpTemp1.CreateCompatibleBitmap(pdc, TocBitmapWidth, TocBitmapHeight);
		CBitmap* pBmpOld1 = memDC1.SelectObject(&bmpTemp1);
		CBrush blackBrush;
		blackBrush.CreateStockObject(BLACK_BRUSH);
		memDC1.FillRect(&rect, &blackBrush);
		m_ImageListMonochrome.Draw(Index, 0, 0, &memDC1);

		//
		// Take Monochrome image from DC1 and render it as an
		// inverted image on DC2.
		//
		CBitmap bmpTemp2;
		bmpTemp2.CreateCompatibleBitmap(pdc, TocBitmapWidth, TocBitmapHeight);
		CBitmap* pBmpOld2 = memDC2.SelectObject(&bmpTemp2);
		memDC2.BitBlt(0, 0, TocBitmapWidth, TocBitmapHeight, &memDC1, 0, 0, NOTSRCCOPY);

		//
		// AND the inverted image onto the target to set all pixels
		// to be drawn to black.
		//
		pdc->BitBlt(x, y, TocBitmapWidth, TocBitmapHeight, &memDC2, 0, 0, SRCAND);

		//
		// Render a Monochrome image with the text color into DC2 by
		// "ANDing" with the text color.
		//
		CBrush textBrush(color);
		memDC2.FillRect(&rect, &textBrush);
		memDC2.BitBlt(0, 0, TocBitmapWidth, TocBitmapHeight, &memDC1, 0, 0, SRCAND);

		//
		// Transparently "OR" the results Monochrome image with the text color to the
		// destination DC.
		//
		pdc->BitBlt(x, y, TocBitmapWidth, TocBitmapHeight, &memDC2, 0, 0, SRCPAINT);

		memDC1.SelectObject(pBmpOld1);
		memDC2.SelectObject(pBmpOld2);
	}
	else
	{
		//
		// Draw Color bitmap transparently on background.
		//
		m_ImageListColor.Draw(Index, x, y, pdc);
	}
}

void QCTocImageList::Free(void)
{
	m_ImageListColor.Free();
	m_ImageListMonochrome.Free();
}

QCImageList::~QCImageList()
{
	Free();
}

void QCImageList::Free(void)
{
	DeleteImageList();
}

bool QCImageList::LoadEmpty(int cx, int cy, int nGrow)
{
	// Safety checks
	if (cx < 0 || cy < 0 || cx > 50 || cy > 50) return false;

	// Create new area to hold images in image list
	if (!Create(cx, cy, ILC_COLORDDB | ILC_MASK, 0, nGrow)) return false;

	return true;
}

bool QCImageList::Load(int cx, int cy, int nGrow, LPCTSTR BmpResource, COLORREF xcolor)
{
	// Load image list with no bitmaps
	if (!LoadEmpty(cx, cy, nGrow)) return false;

	// Load bitmap resource
	if (!BmpResource) return false;
	CBitmap bm1;
	if (!QCLoadBitmap(BmpResource, bm1)) return false;

	// Check bitmap size compared to expected size
	BITMAP bm;
	bm1.GetObject(sizeof(bm), &bm);
	if (bm.bmHeight != cy)
	{
		// bitmap for image list is sized wrong
		// fix either the art or the calling code
		ASSERT(0);
		return false;
	}

	Add(&bm1, xcolor);

	// Return success if we made it this far
	return true;
}

void QCImageList::Draw(int Index, int x, int y, CDC* pdc)
{
	if (!pdc) return;
	POINT pt;
	pt.x = x;
	pt.y = y;
	CImageList::Draw(pdc, Index, pt, ILD_NORMAL);
}

