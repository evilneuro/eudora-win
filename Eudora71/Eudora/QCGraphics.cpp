// QCGraphics.cpp
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

//

#include "stdafx.h"

#include "windowsx.h"
#include "atlimage.h"

#include "QCGraphics.h" 
#include "SecImage.h"
#include "SecDib.h"
#include "SafetyPal.h"
#include "resource.h"
#include "rs.h"
#include "fileutil.h"

#include "PgEmbeddedImage.h"
#include "qtwrapper.h"


namespace libpng
{
#include "..\PNG\libpng\png.h"
}

#include "DebugNewHelpers.h"


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
		GetSystemPaletteEntries(hdcScreen, 0, 256, &pe[0]);
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

SECImage* LoadImage(LPCSTR pURL, int TransparencySysColor /*= COLOR_3DFACE*/)
{
	SECImage* pImage = NULL;

	switch (CanHandleImageInternally(pURL))
	{
	case IF_BMP:	pImage = DEBUG_NEW_MFCOBJ_NOTHROW SECDib;	break;
	case IF_JPEG:	pImage = DEBUG_NEW_MFCOBJ_NOTHROW QCJpeg;	break;
	case IF_PNG:	pImage = DEBUG_NEW_MFCOBJ_NOTHROW QCPng(TransparencySysColor); break;
	default:
		if (GetIniShort(IDS_INI_USE_OS_LOAD_IMAGE))
			pImage = DEBUG_NEW_MFCOBJ_NOTHROW QCImage(TransparencySysColor);
		break;
	}

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
		}
		if (!bLoad)
		{
			delete pImage;
			pImage = NULL;
		}
	}

	return pImage;
}

bool MetafileFromImage(LPCSTR pURL, QCMetaFileInfo* pMFI, int TransparencySysColor /*= COLOR_3DFACE*/)
{
	HMETAFILE hm = 0;

	// See if pointer is valid
	if (!pMFI)
		return false;

	// Create metafile
	CMetaFileDC mdc;
	mdc.Create();
	mdc.SetMapMode(MM_TEXT);

	SECImage* pImage = LoadImage(pURL, TransparencySysColor);
	if (pImage)
	{
		::StretchDIBits(mdc.GetSafeHdc(), 0, 0, pImage->m_dwWidth, pImage->m_dwHeight,
										  0, 0, pImage->m_dwWidth, pImage->m_dwHeight,
										  pImage->m_lpSrcBits, pImage->m_lpBMI, DIB_RGB_COLORS, SRCCOPY);

		// Close metafile
		hm = mdc.Close();

		// Set metafile return values
		pMFI->hm = hm;
		pMFI->width = pImage->m_dwWidth;
		pMFI->height = pImage->m_dwHeight;

		delete pImage;

		return true;
	}

	return false;
}

bool CreateJpegFromIcon(HICON hIcon, LPCSTR filename, COLORREF BackColor)
{
	// Create DC compatible with desktop
	// (do not call DeleteDC; destructor does this)
	CPaletteDC cdc;
	if (!cdc.CreateCDC())
		return false;

	// Get information about icon
	ICONINFO ii;
	if (!GetIconInfo(hIcon, &ii))
		return false;
	
	// Create MFC CBitmaps from the icon's bitmap and mask
	CBitmap Bitmap;
	Bitmap.Attach(ii.hbmColor);
	CBitmap Mask;
	Mask.Attach(ii.hbmMask);

	// Select bitmap into device context
	CBitmap* pOldBitmap = cdc.SelectObject(&Bitmap);

	// Create DC containing icon transparency mask
	// and change transparent area to the window color
	// If this fails we'll just end up with an image
	// with black in the transparent area
	CPaletteDC cdcm;
	if (cdcm.CreateCDC())
	{
		BITMAP bm;
		Bitmap.GetBitmap(&bm);
		CBitmap* pOldMask = NULL;
		int dx = bm.bmWidth;
		int dy = bm.bmHeight;
		if (Mask.GetSafeHandle())
		{
			// Setup mask and inverse mask
			pOldMask = cdcm.SelectObject(&Mask);

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

			cdcm.SelectObject(pOldMask);
		}
	}

	// Create SEC (Stingray) bitmap from MFC bitmap
	SECDib Dib;
	Dib.CreateFromBitmap(&cdc, &Bitmap);

	// Unselect bitmap
	cdc.SelectObject(pOldBitmap);

	// Convert SEC bitmap to a jpeg and save to file
	QCJpeg Jpeg;
	Jpeg.ConvertImage(&Dib);
	Jpeg.SaveImage(filename);

	return true;
}


// CreateJpegFromBitmap - creates a SEC bitmap from an MFC bitmap, uses
// SEC's conversion to an SEC jpeg, then writes it to "filename"
//
bool CreateJpegFromBitmap( CBitmap* pBitmap, LPCSTR filename )
{
	bool fRet = false;
	CPaletteDC cdc;

	// I found a case where there wasn't any bitmap - assert
	// to see how often this occurs.
	ASSERT(pBitmap);

	// Check to see pBitmap is non-NULL before proceeding - CreateFromBitmap
	// asserts but doesn't bother checking for NULL before calling a CBitmap method.
	if ( pBitmap && cdc.CreateCDC() ) {
		SECDib* pDib = DEBUG_NEW_MFCOBJ_NOTHROW SECDib();

		if ( pDib ) {
			pDib->CreateFromBitmap( &cdc, pBitmap );
			SECJpeg* pJpeg = DEBUG_NEW_MFCOBJ_NOTHROW QCJpeg();

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


BOOL QCJpeg::DoSaveImage(CFile* pFile)
{
	m_nQuality = 200;
	return SECJpeg::DoSaveImage(pFile);
}


BOOL QCPng::LoadImage(CFile* pFile)
{
	// Don't use this!
	// Use the version of LoadImage() below that takes a file name.
	ASSERT(0);

	return FALSE;
}

// A kindler, gentler, C++ version of the png_composite macro in png.h
inline void alpha_composite(uch& composite, uch fg, uch alpha, uch bg)
{
	int fg_fraction = fg * alpha;
	int bg_fraction = bg * (255 - alpha);

	int temp = fg_fraction + bg_fraction + 128;
	composite = (uch)((temp + (temp >> 8)) >> 8);
}

using namespace libpng;

static void libpng_warning(png_structp png_ptr, png_const_charp message)
{
	TRACE2("libpng warning reading %s: %s\n", png_ptr->error_ptr, message);

	// Warnings are non-fatal, so just continue on
}

static void libpng_error(png_structp png_ptr, png_const_charp message)
{
	TRACE2("libpng error reading %s: %s\n", png_ptr->error_ptr, message);

	// Do this so that the default error handler doesn't get called because it tries to write to stderr
	longjmp(png_ptr->jmpbuf, 1);
}

QCPng::QCPng(int in_nSystemTransparencySysColor /*= COLOR_3DFACE*/) : SECDib()
{
	m_crTransparent = GetSysColor(in_nSystemTransparencySysColor);
}

QCPng::QCPng(COLORREF in_crTransparent) : SECDib()
{
	m_crTransparent = in_crTransparent;
}

BOOL QCPng::LoadImage(LPCTSTR lpszFileName)
{
	PreLoadImage();

	FILE* fp = fopen(lpszFileName, "rb");
	if (!fp)
		return FALSE;

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)lpszFileName, libpng_error, libpng_warning);
	if (!png_ptr)
	{
		fclose(fp);
		return FALSE;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		return FALSE;
	}

	// Yes, I know it's bad to use setjmp() in C++, but libpng requires it.  Stupid C!
#pragma warning(disable:4611)
	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return FALSE;
	}

	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);

	// Get image information
	int color_type, interlace_type;
	png_get_IHDR(png_ptr, info_ptr, &m_dwWidth, &m_dwHeight, &m_nSrcBitsPerPixel, &color_type,
		&interlace_type, NULL, NULL);

	// Expand data so that it comes out RGB
	if (color_type == PNG_COLOR_TYPE_PALETTE ||
		(color_type == PNG_COLOR_TYPE_GRAY && m_nSrcBitsPerPixel < 8) ||
		png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_expand(png_ptr);
	}

	// Expand data so that alpha is always there
	if (m_nSrcBitsPerPixel == 8 && color_type == PNG_COLOR_TYPE_RGB)
		png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);

	// Convert grayscale to RGB
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	// If there's a background color specified in the image then use it, otherwise use the color
	// passed in to the constructor
	png_color_16p pBG;

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_bKGD) && png_get_bKGD(png_ptr, info_ptr, &pBG))
	{
		if (m_nSrcBitsPerPixel == 16)
			m_crTransparent = RGB(pBG->red >> 8, pBG->green >> 8, pBG->blue >> 8);
		else
			m_crTransparent = RGB(pBG->red, pBG->green, pBG->blue);
	}

	// Set up gamma. 2.2 is a good screen gamma for the PC.
	double screen_gamma = atof(GetIniString(IDS_INI_PNG_SCREEN_GAMMA));
	if (screen_gamma <= 0)
		screen_gamma = 2.2;
	double file_gamma;

	if (!png_get_gAMA(png_ptr, info_ptr, &file_gamma))
	{
		// If no gamma specified in the file, then use our own default
		file_gamma = atof(GetIniString(IDS_INI_PNG_DEFAULT_FILE_GAMMA));
		if (file_gamma <= 0)
			file_gamma = 0.45455;
	}
	png_set_gamma(png_ptr, screen_gamma, file_gamma);

// This dithering code doesn't appear to do anything, so I'm commenting it out for now.
// I may be taking a look at it some time in the future.  JB 10/29/99
//	if (color_type & PNG_COLOR_MASK_COLOR)
//	{
////		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))
////		{
////			png_color_16p histogram;
////  
////			png_get_hIST(png_ptr, info_ptr, &histogram);
////			png_set_dither(png_ptr, palette, num_palette, max_screen_colors, histogram, 1);
////		}
////		else
//		if (m_nSysBitsPerPixel == 8)
//		{
//			CSafetyPalette Palette;
//			const int MAX_COLORS = 256;
//			WORD nNumColors;
//			
//			Palette.Select(GetDC(NULL));
//			if (!GetObject(Palette.GetPalette(), sizeof(nNumColors), &nNumColors) || nNumColors != MAX_COLORS)
//			{
//				ASSERT(0);
//			}
//			else
//			{
//				png_color std_color_cube[MAX_COLORS];
//				PALETTEENTRY PaletteEntries[MAX_COLORS];
//
//				VERIFY(GetPaletteEntries(Palette.GetPalette(), 0, MAX_COLORS, PaletteEntries) == MAX_COLORS);
//				for (int i = 0; i < MAX_COLORS; i++)
//				{
//					std_color_cube[i].red = PaletteEntries[i].peRed;
//					std_color_cube[i].green = PaletteEntries[i].peGreen;
//					std_color_cube[i].blue = PaletteEntries[i].peBlue;
//				}
//				png_set_dither(png_ptr, std_color_cube, MAX_COLORS, MAX_COLORS, NULL, 0);
//			}
//
//			Palette.Unselect();
//		}
//	}

	// Need to call this if any transformations are done.
	png_read_update_info(png_ptr, info_ptr);


	int row_size = png_get_rowbytes(png_ptr, info_ptr);
	png_bytep* row_pointers = DEBUG_NEW png_bytep[m_dwHeight];
	png_bytep png_buffer = DEBUG_NEW png_byte[m_dwHeight * row_size];
	for (unsigned row = 0; row < m_dwHeight; row++)
		row_pointers[row] = png_buffer + row * row_size;

	// Read it all in one fell swoop.  That was easy!
	png_read_image(png_ptr, row_pointers);

	// Reads in any other chunks
	png_read_end(png_ptr, info_ptr);

	int channels = png_get_channels(png_ptr, info_ptr);

	// Do some cleanup now while we can
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	delete [] row_pointers;
	row_pointers = NULL;

	// Set up the bitmap structure
	m_wColors = 0;
	m_nBitPlanes = 1;
	m_nSrcBitsPerPixel = 24;	// We forced it to true color with the png_set_expand() call above
	m_dwPadWidth = PADWIDTH((m_dwWidth * m_nSrcBitsPerPixel) / 8);

	m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, sizeof(BITMAPINFOHEADER) + m_wColors * sizeof(RGBQUAD));
	m_lpBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_lpBMI->bmiHeader.biWidth = m_dwWidth;
	m_lpBMI->bmiHeader.biHeight = m_dwHeight;
	m_lpBMI->bmiHeader.biPlanes = (WORD)m_nBitPlanes;
	m_lpBMI->bmiHeader.biBitCount =  (WORD)m_nSrcBitsPerPixel;
	m_lpBMI->bmiHeader.biCompression = BI_RGB;
	m_lpBMI->bmiHeader.biSizeImage = m_dwPadWidth *  m_dwHeight;
	
	m_lpSrcBits = (LPBYTE)GlobalAllocPtr(GHND, m_lpBMI->bmiHeader.biSizeImage);

	for (row = 0; row < m_dwHeight; row++)
	{
		// Win 95 can't handle top-down DIBs, and the data coming back from
		// libpng is top-down, so make it bottom-up
		png_bytep src = png_buffer + (m_dwHeight - row - 1) * row_size;
		LPBYTE dest = m_lpSrcBits + row * m_dwPadWidth;

		ASSERT(channels >= 3);
		if (channels == 3)
		{
			for (unsigned i = m_dwWidth; i > 0; --i)
			{
				unsigned char r = *src++;
				unsigned char g = *src++;
				unsigned char b = *src++;
				*dest++ = b;
				*dest++ = g;
				*dest++ = r;
			}
		}
		else
		{
			for (unsigned i = m_dwWidth; i > 0; --i)
			{
				unsigned char r = *src++;
				unsigned char g = *src++;
				unsigned char b = *src++;
				unsigned char a = *src++;
				if (a == 255)
				{
					*dest++ = b;
					*dest++ = g;
					*dest++ = r;
				}
				else if (a == 0)
				{
					// Transparent pixel, use background
					*dest++ = GetBValue(m_crTransparent);
					*dest++ = GetGValue(m_crTransparent);
					*dest++ = GetRValue(m_crTransparent);
				}
				else
				{
					alpha_composite(*((unsigned char*)dest)++, b, a, 0);
					alpha_composite(*dest++, g, a, 0);
					alpha_composite(*dest++, r, a, 0);
				}
			}
		}
	}

	delete [] png_buffer;

	return TRUE;
}


QCImage::QCImage(int in_nSystemTransparencySysColor /*= COLOR_3DFACE*/) : SECDib()
{
	m_crTransparent = GetSysColor(in_nSystemTransparencySysColor);
}

QCImage::QCImage(COLORREF in_crTransparent) : SECDib()
{
	m_crTransparent = in_crTransparent;
}


BOOL QCImage::LoadImage(CFile* pFile)
{
	// Don't use this!
	// Use the version of LoadImage() below that takes a file name.
	ASSERT(0);

	return FALSE;
}

BOOL QCImage::LoadImage(LPCTSTR lpszFileName)
{
	if (!PreLoadImage())
		return FALSE;

	USES_CONVERSION;

	if ( !::FileExistsMT(lpszFileName) )
		return FALSE;

	CImage			image;
	IPicture *		piPicture = NULL;
	HBITMAP			hBitmap = NULL;
	const char *	pExtension = strrchr(lpszFileName, '.');

	if ( pExtension &&
		 ((stricmp(pExtension, ".wmf") == 0) || (stricmp(pExtension, ".ico") == 0)) )
	{
		// CImage doesn't handle WMF (metafile) or ICO (icon), so try using OleLoadPicturePath
		OleLoadPicturePath((LPOLESTR)T2COLE(lpszFileName), NULL, 0, 0, IID_IPicture, (LPVOID*)&piPicture);

		if (piPicture)
			piPicture->get_Handle( reinterpret_cast<OLE_HANDLE *>(&hBitmap) );
	}
	else
	{
		// CImage handles everything else that OleLoadPicturePath does (JPEG, GIF, BMP)
		// and also supports PNG. More importantly CImage::Load fails with a GIF that
		// a tester sent which hangs in OleLoadPicturePath.
		// Please note, however, that CImage is new in .NET, so this new support will
		// have to wait for Eudora 7.
		if ( SUCCEEDED( image.Load(lpszFileName) ) )
			hBitmap = image;
	}

	BOOL		bResult = FALSE;

	if (hBitmap)
	{
		HDC		memDC = CreateCompatibleDC(NULL);

		if (memDC)
		{
			bResult = CreateFromBitmap( CDC::FromHandle(memDC), CBitmap::FromHandle(hBitmap) );
			DeleteDC(memDC);
		}
	}

	if (piPicture)
		piPicture->Release();

	if (bResult)
		bResult = PostLoadImage();

	return bResult;
}


//	Quickly figure out if an image file is something that we can *likely*
//	display with our without QuickTime. Relies on accuracy of file extensions
//	for better speed. Current intended use is to determine if we want
//	to display an attachment inline or not.
bool IsValidImageFileQuickCheck(const char * in_szFullPath)
{
	if (strlen(in_szFullPath) >= MAX_PATH)
		return false;

	bool			bIsValidImageFile = false;
	bool			bInternalImageFormat = false;
	const char *	pExt = strrchr(in_szFullPath, '.');

	if (pExt)
	{
		bInternalImageFormat = (stricmp(pExt, ".bmp") == 0) ||
							   (stricmp(pExt, ".jpg") == 0) ||
							   (stricmp(pExt, ".jpeg") == 0) ||
							   (stricmp(pExt, ".jpe") == 0) ||
							   (stricmp(pExt, ".png") == 0)||
							   (stricmp(pExt, ".gif") == 0);

		//	If the file extension indicates it's an format that we handle
		//	internally, then check it out more carefully.
		if ( bInternalImageFormat && CanHandleImageInternally(in_szFullPath, FALSE) )
		{
			bIsValidImageFile = true;
		}
		else
		{
			CString		szFilterString;

			//	Check for cached value first, because we might not even need to init QT
			GetIniString(IDS_INI_QT_FILES_FILTER_STRING, szFilterString);

			if ( szFilterString.IsEmpty() && init_quicktime() )
			{
				//	No previous cached value, so get it from QT
				char *		szOfnFilterString;

				if ( QTW_GetOfnFilterString(&szOfnFilterString) )
					szFilterString = szOfnFilterString;
			}

			if ( !szFilterString.IsEmpty() )
			{
				//	Search for our extension in QT's filter string
				CString		szExt = pExt;

				szExt.MakeLower();
				
				bIsValidImageFile = (szFilterString.Find(szExt) != -1);

				//	Passed initial test to see if QT might like it. Now make
				//	sure QT actually likes it.
				if (bIsValidImageFile)
				{
					bIsValidImageFile = init_quicktime();
					
					if (bIsValidImageFile)
						bIsValidImageFile = QTW_IsValidImageFile(in_szFullPath, NULL);
				}
			}
		}
	}

    return bIsValidImageFile;
}


// Figure out if an image file is something we can display with our own code (not QuickTime)
ImageFormatType CanHandleImageInternally(LPCTSTR FileName, BOOL bDisplayFileErrors)
{
	if (!FileName || (strlen(FileName) >= MAX_PATH) )
		return IF_CANT_HANDLE;

// Forego the simple file extension check, as sometimes it's wrong
//
//	const char* ext = strrchr(FileName, '.');
//
//	if (ext)
//	{
//		ext++;
//		if (stricmp(ext, "bmp" ) == 0)
//			return IF_BMP;
//		if ((stricmp(ext, "jpg" ) == 0) || (stricmp(ext, "jpeg" ) == 0) || (stricmp(ext, "jpe" ) == 0))
//			return IF_JPEG;
//		if (stricmp(ext, "png" ) == 0)
//			return IF_PNG;
//	}

	// Let's open the file up and see what it is.  Maybe we can handle it.
	JJFile daFile(JJFile::BUF_SIZE, bDisplayFileErrors);
	DWORD MagicValue;
	if (SUCCEEDED(daFile.Open(FileName, O_RDONLY)) && SUCCEEDED(daFile.Get(&MagicValue)))
	{
		if ((MagicValue & 0xFFFF) == 0x4D42 /* "BM" */)
			return IF_BMP;
		if (MagicValue == 0x474E5089 /* "PNG" */)
			return IF_PNG;
		if ((MagicValue & 0xFFFFFF) == 0xFFD8FF)
			return IF_JPEG;
		if (MagicValue == 0x38464947 /* "GIF8" */)
			return IF_GIF;
	}

	return IF_CANT_HANDLE;
}
