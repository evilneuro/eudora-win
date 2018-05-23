// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC Targa file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    AAB	6/22/95	Started
//    MSW       9/30/95 Code Review
//
//


#ifndef __SECTARGA_H__
#define __SECTARGA_H__

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

//  TGA File Header
typedef struct _TgaHeader
{                           
    BYTE   IdLength;            /* Image ID Field Length      */
    BYTE   CmapType;            /* Color Map Type             */
    BYTE   ImageType;           /* Image Type                 */
	//
    // Color Map Specification
	//
    WORD   CmapIndex;           /* First Entry Index          */
    WORD   CmapLength;          /* Color Map Length           */
    BYTE   CmapEntrySize;       /* Color Map Entry Size       */
    /*
    ** Image Specification
    */
    WORD   X_Origin;            /* X-origin of Image          */
    WORD   Y_Origin;            /* Y-origin of Image          */
    WORD   ImageWidth;          /* Image Width                */
    WORD   ImageHeight;         /* Image Height               */
    BYTE   PixelDepth;          /* Pixel Depth                */
    BYTE   ImagDesc;            /* Image Descriptor           */
} TGAHEADER;


/*
**  Image/Color Map Data
*/
typedef struct _TgaColorMap
{
    char  *IdField;             /* Image ID Field             */
    BYTE  *CmapData;            /* Color Map Data             */
} TGACMAP;
              

/*
**  Developer Area Tag Structure
*/
typedef struct _TgaTag
{
    WORD    TagNumber;           /* Number of the Tag          */
    DWORD   TagOffset;           /* Offset of the Tag Data     */
    DWORD   TagSize;             /* Size of the Tag Data       */
    BYTE   *TagData;             /* Pointer to the Tag Data    */
    struct _TgaTag *Next;        /* Link to next Tag           */
} TGATAG;


/*
**  Developer Area
*/
typedef struct _TgaDeveloper
{
    WORD    NumberOfTags;     /* Number of Tags in Directory  */
    TGATAG *TagList;          /* Link to list of Tags         */
} TGADEVELOPER;


/*
**  Extension Area
*/
typedef struct _TgaExtension        
{
    WORD   Size;                /* Extension Size             */
    char   AuthorName[41];      /* Author Name                */
    char   AuthorComment[324];  /* Author Comment             */
    WORD   StampMonth;          /* Date/Time Stamp: Month     */
    WORD   StampDay;            /* Date/Time Stamp: Day       */
    WORD   StampYear;           /* Date/Time Stamp: Year      */
    WORD   StampHour;           /* Date/Time Stamp: Hour      */
    WORD   StampMinute;         /* Date/Time Stamp: Minute    */
    WORD   StampSecond;         /* Date/Time Stamp: Second    */
    char   JobName[41];         /* Job Name/ID                */
    WORD   JobHour;             /* Job Time: Hours            */
    WORD   JobMinute;           /* Job Time: Minutes          */
    WORD   JobSecond;           /* Job Time: Seconds          */
    char   SoftwareId[41];      /* Software ID                */
    WORD   VersionNumber;       /* Software Version Number    */
    BYTE   VersionLetter;       /* Software Version Letter    */     
#ifndef WIN32
    BYTE   pad;  //FOR 16-bit warnings.
#endif //WIN32    
    DWORD  KeyColor;            /* Key Color                  */
    WORD   PixelNumerator;      /* Pixel Aspect Ratio Numerator   */
    WORD   PixelDenominator;    /* Pixel Aspect Ratio Denominator */
    WORD   GammaNumerator;      /* Gamma Value                */
    WORD   GammaDenominator;    /* Gamma Value                */
    DWORD  ColorOffset;         /* Color Correction Offset    */
    DWORD  StampOffset;         /* Postage Stamp Offset       */
    DWORD  ScanOffset;          /* Scan Line Table Offset     */
    BYTE   AttributesType;      /* Attributes Types           */   
#ifndef WIN32
    BYTE   padtwo;                  //For 16-bit byte alignment
#endif //WIN32
    DWORD *ScanLineTable;       /* Scan Line Table            */
    BYTE   StampWidth;          /* Width of postage stamp image */
    BYTE   StampHeight;         /* Height of postage stamp image */
    BYTE  *StampImage;          /* Postage Stamp Image        */
    WORD   ColorTable[1024];    /* Color Correction Table     */
} TGAEXTENSION;                                  
                                    
                                    
/*                                  
**  TGA File Footer
*/
typedef struct _TgaFooter
{
    DWORD ExtensionOffset;      /* Extension Area Offset      */
    DWORD DeveloperOffset;      /* Developer Directory Offset */
    char  Signature[18];        /* Signature, dot, and NULL   */
} TGAFOOTER;
                            

/*
**  Complete TGA File Format
*/
typedef struct _TgaFormat
{
    TGAHEADER      Head;            /* Header Area                          */
    TGACMAP        Cmap;            /* Image/Color Map Area                 */
    TGADEVELOPER   Developer;       /* Developer Area                       */
    TGAEXTENSION   Extension;       /* Extension Area                       */
    TGAFOOTER      Foot;            /* File Footer Area                     */
    BOOL           NewTgaFormat;    /* TRUE if file is v2.0 TGA format      */
    DWORD          TgaDataOffset;   /* Offset of the image data in the file */ 
} TGA;

#define TGASIGNATURE   "TRUEVISION-XFILE.\0"
#define TGA_BPP ((m_TgaHead.Head.PixelDepth + 7) >> 3)


class SECTarga: public SECImage
{
    DECLARE_SERIAL(SECTarga);

// Construction
public:
    SECTarga();

// Overrides
protected:
    virtual BOOL DoLoadImage(CFile* pFile);
    virtual BOOL DoSaveImage(CFile* pFile);

// Implementation
protected:
    TGA m_TgaHead;
    short ReadTgaHeader();
    VOID  WriteTgaFile();
#ifdef WIN32
    short TgaDecodeScanLine(BYTE *, WORD, WORD, LPBYTE);
    short TgaEncodeScanLine(BYTE *, WORD, WORD, WORD, LPBYTE);
#else
    short TgaDecodeScanLine(BYTE *, WORD, WORD, BYTE huge *);
    short TgaEncodeScanLine(BYTE *, WORD, WORD, WORD, BYTE huge *);
#endif
    void MakeBMI();
    
public:
    virtual ~SECTarga();
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


#endif // __SECTARGA_H__
