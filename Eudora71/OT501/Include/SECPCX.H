// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC PCX file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	7/22/95	Started
//    MSW       9/30/95 Code Review
//
//

#ifndef __SECPCX_H__
#define __SECPCX_H__

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

#define PCX_COLORMAP	0x0c
#define SEC_PCX_CACHE_SIZE 60000

typedef struct _PcxHeader
{
	BYTE cIdentifier;		// PCX Id Number (Always 0x0a)
	BYTE cVersion;			// Version number
	BYTE cEncoding;			// Encoding format
	BYTE cBitsPerPixel;		// Bits per pixel
	WORD wXStart;			// Left of image
	WORD wYStart;			// Top of Image
	WORD wXEnd;				// right of image
	WORD wYEnd;				// bottom of image
	WORD wHorzRes;			// horizontal resolution
	WORD wVertRes;			// vertical resolution
	BYTE Palette[48];		// 16 color EGA palette
	BYTE Reserved1;			// (always 0)
	BYTE cNumBitPlanes;		// number of bit planes
	WORD wBytesPerLine;		// bytes per scan line
	WORD wPaletteType;		// palette type
	WORD wHorzScreenSize;	// horizontal screen size
	WORD wVertScreenSize;	// vertical screen size
	BYTE Reserved2[54];		// always 0
} PCXHDR;


class SECPcx: public SECImage
{
    DECLARE_SERIAL(SECPcx);

// Construction
public:
    SECPcx();

// Overrides
protected:
    virtual BOOL DoLoadImage(CFile* pFile);
    virtual BOOL DoSaveImage(CFile* pFile);

// Implementation
protected:
    PCXHDR	m_PcxHdr;
    
    void LoadImageBits();
    void WriteImageBits();
    void LoadColorMap();
    void WriteColorMap();
    
public:
    virtual ~SECPcx();
    virtual void Serialize(CArchive& ar);
    
#ifdef _DEBUG
	void AssertValid() const;
#endif

};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECPCX_H__

