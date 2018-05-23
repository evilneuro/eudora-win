
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
SECImage* LoadImage(LPCSTR pURL, int TransparencySysColor = COLOR_3DFACE);
bool MetafileFromImage(LPCSTR pURL, QCMetaFileInfo* pMFI, int TransparencySysColor = COLOR_3DFACE);
bool IsEudoraPaletteCurrent(void);
bool BlitToDevice(SECImage* pImage, CDC* pCDC);

typedef enum
{
	IF_CANT_HANDLE = 0,
	IF_BMP,
	IF_JPEG,
	IF_PNG,
	IF_GIF
} ImageFormatType;

bool IsValidImageFileQuickCheck(const char * in_szFullPath);
ImageFormatType CanHandleImageInternally(LPCTSTR FileName, BOOL bDisplayFileErrors = TRUE);


class QCJpeg : public SECJpeg
{
public:
	BOOL DoSaveImage(CFile* pFile);
};

class QCPng : public SECDib
{
public:
	QCPng(int in_nSystemTransparencySysColor = COLOR_3DFACE);
	QCPng(COLORREF in_crTransparent);
	virtual BOOL LoadImage(CFile* pFile);
	virtual BOOL LoadImage(LPCTSTR lpszFileName);
};

class QCImage : public SECDib
{
public:
	QCImage(int in_nSystemTransparencySysColor = COLOR_3DFACE);
	QCImage(COLORREF in_crTransparent);
	virtual BOOL LoadImage(CFile* pFile);
	virtual BOOL LoadImage(LPCTSTR lpszFileName);
};

#endif // _QCGraphics_h_

