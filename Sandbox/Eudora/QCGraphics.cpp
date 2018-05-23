#include "stdafx.h"
#include "windowsx.h"
#include "QCGraphics.h" 
#include "SecImage.h"
#include "SecDib.h"
#include "SafetyPal.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


bool IsEudoraPaletteCurrent(void)
{
	// If greater than 256 colors in system, then it's ok
	bool bCurrent = true;
	HDC hdcScreen = GetDC(NULL);
	int caps = GetDeviceCaps(hdcScreen, RASTERCAPS);
	if (RC_PALETTE & caps)
	{
		// Compare system palette entries to our palette
		PALETTEENTRY pe[256];
		UINT n = GetSystemPaletteEntries(hdcScreen, 0, 256, &pe[0]);
		int i;
		for (i = 0; i < 256; i++)
		{
			if (memcmp(&MirrorPaletteArray[i], &pe[i], 3))
			{
				bCurrent = false;
				break;
			}
		}
		ReleaseDC(NULL, hdcScreen);
	}
	return bCurrent;
}

bool MetafileFromImage(LPCSTR pURL, QCMetaFileInfo* pMFI)
{
	HMETAFILE hm = 0;

	// See if pointer is valid
	if (!pMFI) return false;

	// Determine extension
	if (!pURL) return false;
	size_t len = strlen(pURL);
	if (len < 5) return false;
	CString ext = (LPCSTR)(pURL + len - 4);
	
	// Create metafile
	CMetaFileDC mdc;
	mdc.Create();
	mdc.SetMapMode(MM_TEXT);

	// Select palette into metafile
	CSafetyPalette Palette;
	Palette.Select(mdc.m_hDC);
	Palette.Realize(mdc.m_hDC);

	bool isJpeg = (ext.CompareNoCase( ".jpg" ) == 0);
	bool isBmp = (ext.CompareNoCase( ".bmp" ) == 0);

	SECImage* pImage = NULL;
	if (isJpeg)
	{
	    // Create jpeg from file
	    pImage = new QCJpeg;
	}
	else if (isBmp)
	{
	    // Create bitmap from file
	    pImage = new QCBitmap; 
	}

	bool bRet = false;
	if (pImage)
	{
		BOOL bLoad = FALSE;

		try
		{
			bLoad = pImage->LoadImage(pURL);
		}
		catch (CFileException* e)
		{
			e->Delete();
			delete pImage;
			return false;
		}

		if (bLoad)
	    {
		// Draw image into metafile
		if (isJpeg)
		{
		    bRet = ((QCJpeg*)pImage)->BlitToDevice(&mdc);
		}
		if (isBmp)
		{
		    bRet = ((QCBitmap*)pImage)->BlitToDevice(&mdc);
		}
		if (bRet)
		{
		    // Unselect palette
		    Palette.Unselect();

		    // Close metafile
		    hm = mdc.Close();

		    // Set metafile return values
		    pMFI->hm = hm;
		    pMFI->width = pImage->m_dwWidth;
		    pMFI->height = pImage->m_dwHeight;
		    bRet = true;
		}
	    }
	    delete pImage;
	}
	return bRet;
}

bool CreateJpegFromIcon(HICON hIcon, LPCSTR filename, COLORREF BackColor)
{
	ICONINFO ii;

	// Get information about icon
	if (!GetIconInfo(hIcon, &ii)) return false;
	
	// Create DC compatible with desktop
	// (do not call DeleteDC; destructor does this)
	CPaletteDC cdc;
	if (!cdc.CreateCDC()) return false;

	// Create MFC bitmap from icon bitmap
	// This will only be valid until end of function
	CBitmap* pBitmap = NULL;
	pBitmap = CBitmap::FromHandle(ii.hbmColor);

	// Select bitmap into device context
	CBitmap* pBitmap0 = cdc.SelectObject(pBitmap);

	// Create DC containing icon transparency mask
	// and change transparent area to the window color
	// If this fails we'll just end up with an image
	// with black in the transparent area
	CPaletteDC cdcm;
	if (cdcm.CreateCDC())
	{
		BITMAP bm;
		pBitmap->GetBitmap(&bm);
		CBitmap* pMask = NULL;
		CBitmap* pMask0 = NULL;
		int dx = bm.bmWidth;
		int dy = bm.bmHeight;
		pMask = CBitmap::FromHandle(ii.hbmMask);
		if (pMask)
		{
			// Setup mask and inverse mask
			pMask0 = cdcm.SelectObject(pMask);

			// Setup memory DCs.
			CPaletteDC memDC1;		// holds Monochrome image
			CPaletteDC memDC2;		// temp DC for rendering and masking
			memDC1.CreateCDC();
			memDC2.CreateCDC();
			// Prepare rect for FillRect call
			CRect rect(0, 0, dx, dy);
			// Render Monochrome image into DC1, taking care to initialize the
			// target DC1 first.
			CBitmap bmpTemp1;
			bmpTemp1.CreateCompatibleBitmap(&cdc, dx, dy);
			CBitmap* pBmpOld1 = memDC1.SelectObject(&bmpTemp1);
			// Copy mask to memDC1
			memDC1.BitBlt(0, 0, dx, dy, &cdcm, 0, 0, SRCCOPY);
			// Take Monochrome image from DC1 and render it as an
			// inverted image on DC2.
			CBitmap bmpTemp2;
			bmpTemp2.CreateCompatibleBitmap(&cdc, dx, dy);
			CBitmap* pBmpOld2 = memDC2.SelectObject(&bmpTemp2);
			memDC2.BitBlt(0, 0, dx, dy, &memDC1, 0, 0, NOTSRCCOPY);
			// AND the inverted image onto the target to set all pixels
			// to be drawn to black.
			cdc.BitBlt(0, 0, dx, dy, &memDC2, 0, 0, SRCAND);
			// Render a Monochrome image with the text color into DC2 by
			// "ANDing" with the text color.
			CBrush textBrush(BackColor);
			memDC2.FillRect(&rect, &textBrush);
			memDC2.BitBlt(0, 0, dx, dy, &memDC1, 0, 0, SRCAND);
			// Transparently "OR" the results Monochrome image with the text color to the
			// destination DC.
			cdc.BitBlt(0, 0, dx, dy, &memDC2, 0, 0, SRCPAINT);
			memDC1.SelectObject(pBmpOld1);
			memDC2.SelectObject(pBmpOld2);

			cdcm.SelectObject(pMask0);
		}
	}
	
	// Create SEC (Stingray) bitmap from MFC bitmap
	SECDib* pDib = new SECDib();
	if (!pDib) return false;
	pDib->CreateFromBitmap(&cdc, pBitmap);
	
	// Unselect palette and bitmaps
	cdc.SelectObject(pBitmap0);

	// Convert SEC bitmap to a jpeg and save to file
	SECJpeg* pJpeg = new QCJpeg();
	if (!pJpeg)	{ delete pDib; return false; }
	pJpeg->ConvertImage(pDib);
	pJpeg->SaveImage(filename);
	
	// Free memory
	delete pJpeg;
	delete pDib;
	return true;
}


// CreateJpegFromBitmap - creates a SEC bitmap from an MFC bitmap, uses
// SEC's conversion to an SEC jpeg, then writes it to "filename"
//
bool CreateJpegFromBitmap( CBitmap* pBitmap, LPCSTR filename )
{
	bool fRet = false;
	CPaletteDC cdc;

	if ( cdc.CreateCDC()) {
		SECDib* pDib = new SECDib();

		if ( pDib ) {
			pDib->CreateFromBitmap( &cdc, pBitmap );
			SECJpeg* pJpeg = new QCJpeg();

			if ( pJpeg ) {
				pJpeg->ConvertImage( pDib );
				pJpeg->SaveImage( filename );
				delete pJpeg;
				fRet = true;
			}
			
			delete pDib;
		}
	}

	return fRet;
}

bool QCLoadBitmap(UINT uBitmap, CBitmap& clsBmp)
{
	return QCLoadBitmap((LPCTSTR)(MAKEINTRESOURCE(uBitmap)), clsBmp);
}

bool QCLoadBitmap(LPCTSTR lpBitmapName, CBitmap& clsBmp)
{
	// Loads the bitmap changing the color table for the current color scheme

	HINSTANCE hInst = AfxFindResourceHandle(lpBitmapName, RT_BITMAP);

	HRSRC hRsrc = ::FindResource(hInst, lpBitmapName, RT_BITMAP);
	if (hRsrc == NULL) return false;

	HGLOBAL hGlb = ::LoadResource(hInst, hRsrc);
	if (hGlb == NULL) return false;

	LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)::LockResource(hGlb);
	if (lpBitmap == NULL)
	{
		::FreeResource(hGlb);
		return false;
	}

	// Take a copy of the BITMAPINFOHEADER structure
	int nColorTableSize = 1 << lpBitmap->biBitCount;
	UINT nSize = (UINT)(lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD));
	LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER) new char[nSize];
	if(lpBitmapInfo == NULL)
	{
		::FreeResource(hGlb);
		return false;
	}

	memcpy(lpBitmapInfo, lpBitmap, nSize);

	int nWidth  = (int)lpBitmapInfo->biWidth;		// Width of bitmap
	int nHeight = (int)lpBitmapInfo->biHeight;		// Height of bitmap

	// Create a new a bitmap and copy the bitmap over with the new color table.
	CWindowDC screenDC(NULL);
	CPaletteDC dc;
	if (!clsBmp.CreateCompatibleBitmap(&screenDC, nWidth, nHeight) ||
	    !dc.CreateCDC())
	{
		::FreeResource(hGlb);
		delete [] lpBitmapInfo;
		return false;
	}
	
	CBitmap* pBmpOld = (CBitmap*)dc.SelectObject(&clsBmp);

	LPBYTE lpBits = LPBYTE(lpBitmap+1);
	lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

	dc.Realize();

	StretchDIBits(dc.m_hDC, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight, 
				  lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	dc.SelectObject(pBmpOld);

	::FreeResource(hGlb);

	delete [] lpBitmapInfo;
	return true;
}

bool QCJpeg::BlitToDevice(CDC* pcdc)
{
	if (!StretchDIBits(pcdc, 
		  0,0,
		  m_dwWidth, m_dwHeight, 
		  0,0, 
		  m_dwWidth, m_dwHeight,
		  m_lpSrcBits,
		  m_lpBMI,
		  DIB_RGB_COLORS,
		  SRCCOPY)) return false;
	return true;
}

int QCJpeg::StretchDIBits(CDC *pDC, int XDest, int YDest, int cxDest, int cyDest,
	int XSrc, int YSrc, int cxSrc, int cySrc, const void FAR* lpvBits,
	LPBITMAPINFO lpbmi, UINT fuColorUse, DWORD fdwRop)
{
	HDC hSafeDC = pDC->GetSafeHdc();

	ASSERT_VALID(pDC);
	ASSERT(lpbmi != 0);

	return ::StretchDIBits(hSafeDC, XDest, YDest, cxDest, cyDest,
		XSrc, YSrc, cxSrc, cySrc, lpvBits, lpbmi, fuColorUse, fdwRop);
}

BOOL QCJpeg::DoSaveImage(CFile* pFile)
{
	m_nQuality = 200;
	return SECJpeg::DoSaveImage(pFile);
}

bool QCBitmap::BlitToDevice(CDC* pcdc)
{
	if (!StretchDIBits(pcdc, 
		  0,0,
		  m_dwWidth, m_dwHeight, 
		  0,0, 
		  m_dwWidth, m_dwHeight,
		  m_lpSrcBits,
		  m_lpBMI,
		  DIB_RGB_COLORS,
		  SRCCOPY)) return false;
	return true;
}

int QCBitmap::StretchDIBits(CDC *pDC, int XDest, int YDest, int cxDest, int cyDest,
	int XSrc, int YSrc, int cxSrc, int cySrc, const void FAR* lpvBits,
	LPBITMAPINFO lpbmi, UINT fuColorUse, DWORD fdwRop)
{
	HDC hSafeDC = pDC->GetSafeHdc();

	ASSERT_VALID(pDC);
	ASSERT(lpbmi != 0);

	return ::StretchDIBits(hSafeDC, XDest, YDest, cxDest, cyDest,
		XSrc, YSrc, cxSrc, cySrc, lpvBits, lpbmi, fuColorUse, fdwRop);
}

