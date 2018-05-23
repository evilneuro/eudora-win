// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC GIF file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	7/22/95	Started
//    MSW       9/30/95 Code review
//
//

#ifndef __SECGIF_H__
#define __SECGIF_H__

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

// Typedefs needed by the GIF reading engine.
//
//
typedef struct _GifHeader 
{
	BYTE Signature[3];	// should be "GIF"
	BYTE Version[3];	// should be "87A" or "89A"

	// Logical screen descriptor
	WORD Width;
	WORD Height;
	BYTE ColorInfo;		// Bits 0-2 size of global color table
						// Bit 3 color sorting toggle (for 89A only)
						// Bits 4-6 color resolution; max size of palette
						// Bit 7 toggle for global color table

	BYTE BackColor;		// index into global color table for color to use for background
	BYTE AspectRatio;	// ranges 1 to 255 according to : (AspectRatio + 15)/64
} GifHeader;

typedef struct _GifImageDescriptor 
{
	BYTE Separator;	// should contain value of 0x2c to denote beginning
	WORD Left;
	WORD Top;
	WORD Width;
	WORD Height;
	BYTE ImageInfo;	// Bit 0 local color table flag
					// Bit 1 Interlace flag
					// Bit 2 Sort flag
					// Bits 3-4 Reserved
					// Bits 5-7 size of local color table
} GifImageDescriptor;

typedef struct _GifGraphicsControlExtension
{
	BYTE Introducer;	// always 21h
	BYTE Label;			// always f9h
	BYTE BlockSize;		// always 04h
	BYTE Packed;		// method of graphics disposal to use
	WORD DelayTime;		// hundreths of seconds to wait
	BYTE ColorIndex;	// Transparent Color Index 
	BYTE Terminator;	// Block terminator (always 0)
} GifGraphicControl;

typedef struct _GifPlainTextExtension
{
	BYTE Introducer;	// always 21h
	BYTE Label;			// always 01h
	BYTE BlockSize;		// always 0ch
	WORD TextGridLeft;	// X Position of text grid in pixels
	WORD TextGridTop;	// Y position of text grid in pixels
	WORD TextGridWidth;	// width of text grid in pixels
	WORD TextGridHeight;	// height of text grid in pixels
	BYTE CellWidth;		// Width of a grid cell in pixels
	BYTE CellHeight;	// Height of a grid cell in pixels
	BYTE TextFgColorIndex;	// text fg color index val
	BYTE TextBgColorIndex;	// text bg color index val
	BYTE *PlainTextData;	// the plain text data
	BYTE Terminator;	// Block terminator (always 0)
} GifPlainText;

typedef struct _GifApplicationExtension
{
	BYTE Introducer;	// Extension introducer (0x21)
	BYTE Label;			// always ffh
	BYTE BlockSize;		// always 0bh
	char Identifier[8];	// application identifier
	BYTE AuthentCode[3];	// application authentication
	BYTE *ApplicationData;	// point to application data sub-blocks
	BYTE Terminator;	// block terminator (always 0)
} GifApplication;

typedef struct _GifCommentExtension
{
	BYTE Introducer;	// always 21h
	BYTE Label;			// comment label (always 0xfe)
	BYTE *CommentData;	// Pointer to comment data sub-blocks
	BYTE Terminator;	// always 0
} GifComment;


#define SEC_COLORMAP 0x80
#define READSIZE(y,z)  if ((GetCache((y), (z))) != (z)) return FALSE;
#define EXTFLAG		0x21
#define IMAGESEP	0x2c
#define TRAILER		0x3b
#define INTERLACEMASK 0x40
#define GIF_CACHE_SIZE 60000

class SECGif: public SECImage
{
    
    DECLARE_SERIAL(SECGif);
    
// Construction
    SECGif();
    SECGif(LPCTSTR lpszFileName);
    
// Overridables
protected:
    virtual BOOL DoLoadImage (CFile *pFile); /* Load native format & convert to DIB */
    virtual BOOL DoSaveImage (CFile *pFile); /* Save as native format */
    
// Implementation
protected:

#ifdef WIN32
   BYTE *m_lpPic8, *m_lpRaster, *m_lpRawGIF; 
#else
   BYTE __far *m_lpPic8, *m_lpRaster, *m_lpRawGIF; 
#endif
    HANDLE m_hPic8, m_hRaster, m_hRawGIF;
    WORD m_wXC;
    WORD m_wYC;
    long m_lPass; 
    long m_lBitOffset;
    long m_lColorMapSize;
    long m_lNumCols;
    long m_lBitMask;
    long m_lCodeSize;
    long m_lReadMask;
    int m_nBlockCount;
	int m_nFreeCode;
    // temporary values used in decompress functions
    BYTE m_byte1, m_byte2, m_byte3;     
#ifndef WIN32
    BYTE m_pad; //for 16bit warnings..
#endif //WIN32  
    long m_nByteOffset;
    CPalette m_Palette;
    BOOL	m_b87a;	/*TRUE = old format */
    BOOL m_bInterlaced; // Flag for whether interlaced or not
    GifHeader	m_gifHeader;
    long m_nPixelBits;
    GifImageDescriptor m_gifID;
    GifGraphicControl m_gifGraphicControl;
    GifPlainText m_gifPlainText;
    GifApplication m_gifApplication;
    GifComment m_gifComment;
	BOOL m_bUseTransparent;
	BOOL m_bIsTransparent;
	BYTE m_cTransIndex;
	BYTE m_cTransRed, m_cTransGreen, m_cTransBlue;
    
    DWORD LZWEncodeImage(DWORD data_size);
    BOOL ReadExtension();
    BOOL ReadImageHeader();
    BOOL ReadImage();
    BOOL ReadTrailer();
    BOOL ShiftRasterBytes(long);
    int ReadCode(BOOL);
    BOOL SECCreateColorTable(int nSize);
    int SECReadCode();
    void DoInterlace(int);
    
public:
    void SetInterlace(BOOL bArg) {m_bInterlaced = bArg;}
    void Serialize(CArchive& ar);
    virtual ~SECGif();
	void SetTransparent(BOOL bArg) {m_bUseTransparent = bArg;}
	void SetTransparentColors(BYTE cRed, BYTE cGreen, BYTE cBlue) \
	{ m_cTransRed = cRed; m_cTransGreen = cGreen; m_cTransBlue = cBlue; }

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


#endif // __SECGIF_H__
