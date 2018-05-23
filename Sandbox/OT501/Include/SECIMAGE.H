//
// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// SECImage Abstract Base Class (ABC)
//
// Description
//  
//  Right now SECImage uses a DIB structure to 
//  store image data in.  This will most likely change
//  in the future, so depending on any of the member
//  data staying the same is very dangerous at this point.
//  We recommend sticking to the member functions instead,
//  this interface will not be changing.
//
// CHANGELOG: 
//
//    AAB	6/22/95	Started
//    MSW	8/01/95 tweaks
//    MSW       9/30/95 Code review
//
//

#ifndef __SECIMAGE_H__
#define __SECIMAGE_H__

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

class  SECImage: public CObject
{

// Construction
public:
    SECImage();
    SECImage(LPCTSTR lpszFileName);
    
// Attributes
public:
    DWORD dwGetWidth();
    DWORD dwGetHeight();
    
    BOOL CreatePalette();
    int m_nSrcBitsPerPixel;
	int m_nSysBitsPerPixel;
    int m_nBitPlanes;
    BOOL m_bIsPadded;
    DWORD m_dwPadWidth; // for calculating DWORD boundary of scan line
    DWORD m_dwWidth;
    DWORD m_dwHeight;
    WORD m_wColors;
    WORD m_wSysColors;
    LPCTSTR m_lpszFileName; // filename used for source of bitmap
    CPalette *m_pPalette;
    CFile *m_pFile;
#ifdef WIN32
    LPBYTE m_lpSrcBits;
#else
    BYTE huge * m_lpSrcBits;
#endif
    LPBITMAPINFO m_lpBMI;
    LPRGBQUAD m_lpRGB;
	BOOL m_bUseHalftone;
	int m_nBitHolder;	// used for conversion of 4 or 1 bpp images to 8 bpp.
	DWORD m_dwBitCount; //used for conversion of 4 or 1 bpp images to 8 bpp and greater
	DWORD m_dwError;	// Error message
	BOOL m_bSwapOnWrite;	// Flag to swap monochrom bytes when writing out
				// Default is FALSE;
    
// Operations
public:    
    BOOL CopyImage(SECImage *pSrc);
    BOOL ConvertImage(SECImage *pSrc);
    
    CBitmap *MakeBitmap(CDC *pDC, const TCHAR *lpszFileName);
    CBitmap *MakeBitmap(CDC *pDC); // MSW
    
    virtual BOOL LoadImage(CFile* pFile);
    virtual BOOL LoadImage(LPCTSTR lpszFileName);
    virtual BOOL SaveImage(CFile* pFile);
    virtual BOOL SaveImage(LPCTSTR lpszFileName);
    
// Operations - Image Manipulation
public:    
    BOOL FlipHorz();
    BOOL FlipVert();
    BOOL Rotate90(BOOL bClockwise=TRUE);
    void ContrastImage(short nSharpen);
    BOOL Crop(long nLeft, long nTop, long nWidth, long nHeight);
    DWORD NumBytes();
    WORD NumColors();
	int StretchDIBits(CDC *pDC, int XDest, int YDest, int cxDest, int cyDest,
	int XSrc, int YSrc, int cxSrc, int cySrc, const void FAR* lpvBits,
	LPBITMAPINFO lpbmi, UINT fuColorUse, DWORD fdwRop);
	BOOL CreateFromBitmap(CDC *pDC, CBitmap *pSrcBitmap);  

// Overridables
protected:
    virtual BOOL PreLoadImage();
    virtual BOOL PostLoadImage();
    virtual BOOL PreSaveImage();
    virtual BOOL PostSaveImage();

    // SECImage derivatives *must* define these.
    virtual BOOL DoSaveImage(CFile* pFile)=0;
    virtual BOOL DoLoadImage(CFile* pFile)=0;
    
// Implementation
protected:
// member variables and functions for the internal image file cache
    DWORD m_dwCacheMax;		// Size of maximum bytes in file cache
    DWORD m_dwCacheSize;	// Size of current file cache
    DWORD m_dwCachePos;		// location of file cache position
#ifdef WIN32
    LPBYTE m_lpCache;	// the actual file cache array
#else
    BYTE huge *m_lpCache;	// the actual file cache array
#endif
public:
	DWORD CalcPadding(DWORD dwBitsPerPixel, DWORD dwPixels);	// calculate padding width
	DWORD LastByte(DWORD dwBitsPerPixel, DWORD dwPixels);
	DWORD CalcBytesPerLine(DWORD dwBitsPerPixel, DWORD dwWidth);
    BOOL PadBits();  // Pad to DWORD boundary consistent with DIB format
    BOOL UnPadBits();	// Unpad bits back to sequential data
protected:
    // Cache convenience functions
    BOOL InitCache(DWORD dwBytes, WORD wMode);  // Fills in a cache from a file
    DWORD LoadCache();  // Fills in a cache from a file
    DWORD GetCache(void *pBuf, DWORD dwBytes);  // Get cache bytes
    DWORD PutCache(void *pBuf, DWORD dwBytes);  // put bytes in cache
    void FlushCache(); //writes current cache out to file
    void FreeCache();   
    BOOL SECLoadDib(CFile *pFile); //used to serialize/load intermediate file format.
    
public:
    void Serialize(CArchive &ar);
    virtual ~SECImage();
    
#ifdef _DEBUG
	void AssertValid() const;
#endif //_DEBUG
};

// Macros - These help with padding and also define
// the BMP file marker.

#define PADWIDTH(x)	(((x)*8 + 31)  & (~31))/8
#define DIB_HEADER_MARKER	((WORD) ('M' << 8) | 'B')
#define SEC_READ_CACHE 0x01
#define SEC_WRITE_CACHE 0x02

// iteration used for converting 1bpp and 4bpp images over to 8bpp.
#define NEXT_IMAGE_PIXEL(bDest, lpSrc) \
switch(m_nSrcBitsPerPixel) { \
case 1: \
		if (m_nBitHolder > 7) { \
			m_nBitHolder = 0; \
			lpSrc++; \
		} \
		if (m_bSwapOnWrite ==TRUE)\
			bDest = (BYTE)((*lpSrc >> (7-m_nBitHolder)) & (BYTE)0x01);\
		else\
			bDest = (BYTE)((*lpSrc >> m_nBitHolder) & (BYTE)0x01);\
		m_nBitHolder ++; \
		if (++m_dwBitCount >= m_dwWidth) { \
			m_dwBitCount = 0; \
			m_nBitHolder = 0; \
			lpSrc++; \
			} \
		break; \
case 4: \
		if (m_nBitHolder ==1) { \
			m_nBitHolder = 0; \
			bDest = (BYTE)(*lpSrc & (BYTE)0xf); \
			lpSrc++; \
			m_dwBitCount++; \
			if (m_dwBitCount >= m_dwWidth) \
				m_dwBitCount = 0; \
} \
		else {\
			bDest = (BYTE)(*lpSrc >> 4); \
			if (++m_dwBitCount >= m_dwWidth) { \
				lpSrc++; \
				m_dwBitCount = 0; \
				} \
			else \
				m_nBitHolder=1; \
} \
		bDest = (BYTE)(bDest & (BYTE)0x0f); \
		break; \
default : \
		bDest = *lpSrc++; \
}


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECIMAGE_H__


