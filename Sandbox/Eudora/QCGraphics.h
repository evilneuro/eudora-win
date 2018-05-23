
#ifndef _QCGraphics_h_
#define _QCGraphics_h_

#include "SecImage.h"
#include "SecJpeg.h"

typedef struct structQCMetaFileInfo
{
	HMETAFILE hm;
	long width;
	long height;
} QCMetaFileInfo;

class CBitmap;
bool CreateJpegFromIcon(HICON hIcon, LPCSTR filename, COLORREF BackColor);
bool CreateJpegFromBitmap( CBitmap* pBitmap, LPCSTR filename );
bool MetafileFromImage(LPCSTR pURL, QCMetaFileInfo* pMFI);
bool QCLoadBitmap(UINT uBitmap, CBitmap& clsBmp);
bool QCLoadBitmap(LPCTSTR lpBitmapName, CBitmap& clsBmp);
bool IsEudoraPaletteCurrent(void);

class QCJpeg : public SECJpeg
{
public:
	bool BlitToDevice(CDC* pcdc);
	int StretchDIBits(CDC *pDC, int XDest, int YDest, int cxDest, int cyDest, int XSrc, int YSrc, int cxSrc, int cySrc, const void FAR* lpvBits, LPBITMAPINFO lpbmi, UINT fuColorUse, DWORD fdwRop);
	BOOL DoSaveImage(CFile* pFile);
};

class QCBitmap : public SECDib
{
public:
	bool BlitToDevice(CDC* pcdc);
	int StretchDIBits(CDC *pDC, int XDest, int YDest, int cxDest, int cyDest, int XSrc, int YSrc, int cxSrc, int cySrc, const void FAR* lpvBits, LPBITMAPINFO lpbmi, UINT fuColorUse, DWORD fdwRop);
};

#endif // _QCGraphics_h_

