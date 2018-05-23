
#include "stdafx.h"
#include "windowsx.h"
#include "QCGraphics.h"
#include "QCImageList.h"
#include "SafetyPal.h"
#include "resource.h"


#include "DebugNewHelpers.h"

class QCCommon16ImageList g_theCommon16ImageList;
class QCCommon32ImageList g_theCommon32ImageList;
class QCMailboxImageList g_theMailboxImageList;
class QCTocImageList g_theTocImageList;
class QCMoodImageList g_theMoodImageList;

void LoadImageLists(void)
{
	g_theCommon16ImageList.Load();
	g_theCommon32ImageList.Load();
	g_theMailboxImageList.Load();
	g_theTocImageList.Load();
	g_theMoodImageList.Load();
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
		m_ImageListMonochrome.Draw(Index, x, y, pdc);
	else
		m_ImageListColor.Draw(Index, x, y, pdc);
}

void QCTocImageList::Free(void)
{
	m_ImageListColor.Free();
	m_ImageListMonochrome.Free();
}

QCMoodImageList::QCMoodImageList()
{
}

bool QCMoodImageList::Load(void)
{
	bool rc;
	rc = m_ImageListColor.Load(16, 16, 0, MAKEINTRESOURCE(IDB_IL_MOOD_MAIL), RGB(192, 192, 192));
	if (!rc) 
	{
		ASSERT(0);
		return false;
	}

	rc = m_ImageListMonochrome.Load(16, 16, 0, MAKEINTRESOURCE(IDB_IL_MOOD_MAILM), RGB(0, 0, 0));
	if (!rc)
	{
		ASSERT(0);
		return false;
	}

	return true;
}

void QCMoodImageList::Draw(int Index, int x, int y, CDC* pdc, COLORREF color)
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
		m_ImageListMonochrome.Draw(Index, x, y, pdc);
	else
		m_ImageListColor.Draw(Index, x, y, pdc);
}

void QCMoodImageList::Free(void)
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
	if ( !bm1.LoadBitmap(BmpResource) )
	{
		ASSERT( !"LoadBitmap failed in QCImageList::Load" );
		return false;
	}

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

