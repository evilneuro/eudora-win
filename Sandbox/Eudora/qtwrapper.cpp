// qtwrapper.cpp - this is a wrapper api for anything that makes calls into
// the Quicktime API. Quicktime conflicts with Paige (Bad Paige boys!), so
// we need to isolate any Quicktime stuff here. Absolutely no Paige references
// can be made in this module, and no QT or Paige stuff can exist in the
// interface to this module.

#include "stdafx.h"
#include "resource.h"
#include "assert.h"

#include "Qtml.h"
#include "Movies.h"
#include "MacWindows.h"
#include "Resources.h"
#include "ImageCompression.h"
#include "TextUtils.h"

#include "qtwrapper.h"
#include "rs.h"

// these macros make color masks for high-color+ color tables. kaibo'd these
// from a msdn sample thinking i'd use them for *something*, but i didn't. Now
// they're just here so i don't lose 'em!!
#define MAKE556WORD(r,g,b) ((((WORD)(r) >> 3) << 11) |                   \
                            (((WORD)(g) >> 3) << 6) | ((WORD)(b) >> 2));
#define MAKE565WORD(r,g,b) ((((WORD)(r) >> 3) << 11) |                   \
                            (((WORD)(g) >> 2) << 5) | ((WORD)(b) >> 3));
#define MAKE555WORD(r,g,b) ((((WORD)(r) >> 3) << 10) |                   \
                            (((WORD)(g) >> 3) << 5) | ((WORD)(b) >> 3));
#define MAKE565RGB(w,r,g,b) (r = (w & 0xF800) >> 8, g = (w & 0x07E0) >> 3, \
                             b = (w & 0x001F) << 3);
#define MAKE555RGB(w,r,g,b) (r = (w & 0x7C00) >> 7, g = (w & 0x03E0) >> 2, \
                             b = (w & 0x001F) << 3);


// give this thing a bitmapinfo ptr, and get back the color table
#define GET_COLORTABLE_PTR(p) ((RGBQUAD*)((LPBYTE)p + (WORD)    \
                                       (((PBITMAPINFO)p)->bmiHeader.biSize)))

#ifdef _DEBUG
#define ODS(s) OutputDebugString(s)
#else
#define ODS(s) ((void)0)
#endif

// local genius
static HBITMAP create_compatible_dib( HDC hdc, HPALETTE hpal, ULONG ulWidth,
                               ULONG ulHeight, BITMAPINFO** ppInfo,
                               PVOID *ppvBits );

static BOOL fill_bitmap_info( HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi );
static BOOL fill_color_table( HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi );
static UINT get_system_palette_entries( HDC hdc, UINT iStartIndex,
                                        UINT nEntries, LPPALETTEENTRY lppe );
static void init_filter_string();
static void finish_filter_string();
static void add_to_filter_string( void* p, size_t nBytes );
//static HPALETTE create_dib_palette( PBITMAPINFO hDib );


// Filter String of the format expected by OpenFileName dialog. We only want
// to get this thing once, and then just hand it around.
static char* g_pFS = NULL;

///////////////

struct QTWinVersionInfo : private OSVERSIONINFO
{
// we should at least give the appearance of being... Alive! ALIVE!!
#define WVI_SIZE sizeof(OSVERSIONINFO)
#define _sane_ (assert(this && (dwOSVersionInfoSize == WVI_SIZE)))

// frankenstein
    QTWinVersionInfo(){
        dwOSVersionInfoSize = WVI_SIZE; GetVersionEx(this);}

    ~QTWinVersionInfo(){};

// accessors
    DWORD GetMajorVersion(){_sane_; return dwMajorVersion;}
    DWORD GetMinorVersion(){_sane_; return dwMinorVersion;}
    DWORD GetBuildNumber(){_sane_; return dwBuildNumber;}
    DWORD GetPlatformId(){_sane_; return dwPlatformId;}
    bool GetCSDVersion( char s[128] ){_sane_;
        return (strcpy( s, szCSDVersion ) != 0);}

// da scoop!
    bool IsWinNT(){_sane_; return (dwPlatformId == VER_PLATFORM_WIN32_NT);}
    bool IsWin95(){_sane_;
        return ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
                (dwMinorVersion == 0));}
    bool IsWin98(){_sane_;
        return ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
                (dwMinorVersion == 10));}

} *g_pwvi = NULL;

static bool g_qtInitialized = false;

///////////////////////////////////////////////////////////////////////////////
// exported api

extern "C"
bool QTW_Startup()
{
    if ( !g_qtInitialized && GetIniShort( IDS_INI_USEQUICKTIME ) ) {
        g_qtInitialized = (InitializeQTML( 0 ) == 0);

#if 0
// fornow: no movie support in 4.x
        EnterMovies();
#endif

        g_pwvi = (QTWinVersionInfo*) new QTWinVersionInfo;
    }

    return g_qtInitialized;
}

extern "C"
void QTW_Shutdown()
{
    if ( g_qtInitialized ) {

#if 0
// fornow: no movie support in 4.x
        ExitMovies();
#endif

        TerminateQTML();

        if ( g_pFS )
            free( g_pFS );

        if ( g_pwvi )
            delete g_pwvi;

        g_qtInitialized = false;
    }
}


// QTW_GetOfnFilterString:
// Builds an OPENFILENAME filter string of all the graphics file types
// that Quicktime supports.

enum {
    kFileExt = 'ext ',
    kFileDesc = 'desc',
    kMimeType = 'mime'
};

extern "C"
bool QTW_GetOfnFilterString( char** ppFilterString )
{
    bool bRet = false;

    if ( ppFilterString ) {
        *ppFilterString = NULL;
    }
    else {
        assert( 0 );
        return bRet;
    }

    if ( !g_pFS ) {
        init_filter_string();

        QTAtomContainer ac;
        ComponentDescription cd;
        ComponentInstance ci;

        memset( &ac, '\0', sizeof(QTAtomContainer) );
        memset( &cd, '\0', sizeof(ComponentDescription) );
        cd.componentType = GraphicsImporterComponentType;
        Component c = FindNextComponent( 0, &cd );

        while ( c ) {
            ci = OpenComponent( c );
            GraphicsImportGetMIMETypeList( ci, (void*)&ac );

            QTAtom fileExt;

            for ( short i = 1; true; i++ ) {
                fileExt = QTFindChildByIndex( ac, kParentAtomIsContainer,
                                              kFileExt, i, nil );

                if ( fileExt ) {
                    QTLockContainer( ac );

                    long lSize = 0;
                    char* atomData;
                    QTGetAtomDataPtr( ac, fileExt, &lSize, &atomData );

                    if ( lSize > 0 ) {
                        add_to_filter_string( atomData, lSize );
                    }
                    else {
                        assert( 0 );
                    }

                    QTUnlockContainer( ac );
                }
                else {
                    break;
                }
            }

            CloseComponent( ci );
            QTDisposeAtomContainer( ac );
            c = FindNextComponent( c, &cd );
        }

        finish_filter_string();

        // we make sure that there is a string, it has a length, and that
        // there is something that at least resembles a file extension in it.
        if ( g_pFS && strlen( g_pFS ) && strstr( g_pFS, "*." ) ) {
            *ppFilterString = g_pFS;
            bRet = true;
        }
    }
    else {
        *ppFilterString = g_pFS;
        bRet = true;
    }

    return bRet;
}

static void init_filter_string()
{
    assert( !g_pFS );
    char* pFilterLabel = "Image Files|";
    g_pFS = (char*) malloc( strlen( pFilterLabel ) + 1 );
    assert( g_pFS );

    if ( g_pFS ) {
        strcpy( g_pFS, pFilterLabel );
    }
}

static void finish_filter_string()
{
    char* pLastPart = "All Files|*.*||";

    assert( g_pFS );
    assert( strlen( g_pFS ) );

    if ( g_pFS ) {
        if ( *g_pFS ) {
            char* pLastChar = &g_pFS[strlen( g_pFS ) - 1];

            if ( *pLastChar == '|' ) {
                // dismal failure; we truncate the filter string to zero. this
                // results in a filter string for all files (*.*).
                *g_pFS = '\0';
            }
            else {
                // the last char is a ';'; we replace it with '|' to terminate
                // the "Image Files" filter before adding one for "All Files".
                *pLastChar = '|';
            }
        }

        // now add a filter for "All Files" (i.e. *.*)
        size_t newSize = strlen( pLastPart ) + strlen( g_pFS ) + 1;
        char* pNewFS = (char*) realloc( g_pFS, newSize );
        assert( pNewFS );
        assert( strlen( pNewFS ) );

        if ( pNewFS ) {
            g_pFS = pNewFS;
            strcat( g_pFS, pLastPart );
        }
    }
    else {
        // make one last try at providing something useable
        g_pFS = (char*) malloc( strlen( pLastPart ) + 1 );

        if ( g_pFS )
            strcpy( g_pFS, pLastPart );
    }
}

static void add_to_filter_string( void* p, size_t nBytes )
{
    assert( g_pFS );
    assert( strlen( g_pFS ) );
    char* pBuf = (char*) malloc( nBytes + 2 );
    assert( pBuf );

    if ( pBuf ) {
        memmove( pBuf, p, nBytes );
        pBuf[nBytes] = ',';
        pBuf[nBytes + 1] = '\0';

        char* pExt = strtok( pBuf, "," );
        assert( pExt );
        assert( strlen( pExt ) );

        while ( pExt ) {
            char tmp[16];
            sprintf( tmp, "*.%s;", pExt );

            if ( strstr( g_pFS, tmp ) == NULL ) {
                size_t newSize = strlen( tmp ) + strlen( g_pFS ) + 1;
                char* pNewFS = (char*) realloc( g_pFS, newSize );
                assert( pNewFS );
                assert( strlen( pNewFS ) );

                if ( pNewFS ) {
                    g_pFS = pNewFS;
                    strcat( g_pFS, tmp );
                }
                else {
                    break;
                }
            }

            pExt = strtok( NULL, "," );
        }

        free( pBuf );
    }
}


// QTW_IsValidImageFile:
// Finds graphics component for an image file, and validates it's contents.
//
// TODO: currently we are not supporting detailed errors; images are either
// valid, or not. Hopefully, we'll be able to diagnose problems a little bit
// better than this, but I haven't got that far with QT3 yet.
extern "C"
bool QTW_IsValidImageFile( const char* pFileName, unsigned** ppErrCode )
{
    bool bRet = false;

    // get the QT component for this image type
    FSSpec fs;
    memset( &fs, '\0', sizeof( FSSpec ) );
    strcpy( (char*)fs.name, pFileName );
    c2pstr( (char*)fs.name );

    GraphicsImportComponent gi = nil;
    GetGraphicsImporterForFile( &fs, &gi );

    if ( gi ) {
        boolean bValid;  // oh brother, another bool type... great.
        ComponentResult er = GraphicsImportValidate( gi, &bValid );

        // Components are not required to support this routine, in which case
        // they return "badComponentSelector"; we call that valid.
        if ( bValid || (er == badComponentSelector) ) {
            bRet = true;
        }

        CloseComponent( gi );
    }

    return bRet;
}


// metafile_from_qt_image:
// uses quicktime to convert any supported image file into a metafile
extern "C"
bool QTW_MetafileFromQtImage( char* filePath, HMETAFILE* phMeta,
                              int* pCx, int* pCy )
{
// TODO: general cleanup & error handling (this ain't great code)

    bool bRet = false;
    HMETAFILE hMeta = 0;

    // get the QT component for this image type
    FSSpec fs;
    memset( &fs, '\0', sizeof( FSSpec ) );
    strcpy( (char*)fs.name, filePath );
    c2pstr( (char*)fs.name );

    GraphicsImportComponent gi = nil;
    GetGraphicsImporterForFile( &fs, &gi );

    // not my favorite way to code, but... basically, this routine needs a
    // rewrite. there's no point in good style 'til that happens.
    if ( !gi )
        return false;

    // create a dib that is compatible with the current display (screen)
    Rect imageRect;
    GraphicsImportGetNaturalBounds( gi, &imageRect );

    long cx, cy;
    cx = imageRect.right - imageRect.left;
    cy = imageRect.bottom - imageRect.top;

    BITMAPINFO* pBmpInfo;    
    PBYTE pBmpBits;
    HWND hwnd = GetActiveWindow();
    HDC hDc = GetDC( hwnd );
    HBITMAP hbm = create_compatible_dib( hDc, NULL, cx, -cy,
                                         &pBmpInfo, (void**)&pBmpBits );

    // have QT create a GWorld from the dib/dc, then make it the
    // current QuickDraw port
    GWorldPtr pGW;
    NewGWorldFromHBITMAP( &pGW, nil, nil, 0, (void*)hbm, (void*)hDc );
    GraphicsImportSetGWorld( gi, pGW, nil );
    ComponentResult er = GraphicsImportDraw( gi );

    // create a metafile dc (device we'll actually draw to)
    HDC hdcMF = CreateMetaFile( NULL );
    SetMapMode( hdcMF, MM_ANISOTROPIC );
    HDC hdcOffScreen1 = CreateCompatibleDC( hDc );

    HBITMAP hbmOld = (HBITMAP) SelectObject( hdcOffScreen1, hbm );

    StretchBlt(
        hdcMF,
        0,0,
        cx,cy,
        hdcOffScreen1,
        0,0,
        cx,cy,
        SRCCOPY );

    SelectObject( hdcOffScreen1, hbmOld );

    // get the metafile handle (return value)
    hMeta = CloseMetaFile( hdcMF );

    // destroy everything else
    CloseComponent( gi );
    DisposeGWorld( pGW );
    DeleteObject( hbm );
    free( pBmpInfo );
    ReleaseDC( hwnd, hDc );
    DeleteDC( hdcOffScreen1 );

    if ( hMeta ) {
        *phMeta = hMeta;

        if ( !(*pCx) )
            *pCx = cx;
        if ( !(*pCy) )
            *pCy = cy;

        bRet = true;
    }

    return bRet;
}


extern "C"
bool QTW_DibFromQtImage( char* filePath, HBITMAP* phDib, RECT* pRect )
{
    bool bRet = false;
    *phDib = NULL;
    memset( pRect, '\0', sizeof(RECT) );

    // get the QT component for this image type
    FSSpec fs;
    memset( &fs, '\0', sizeof(FSSpec) );
    strcpy( (char*)fs.name, filePath );
    c2pstr( (char*)fs.name );

    GraphicsImportComponent gi = nil;
    GetGraphicsImporterForFile( &fs, &gi );

    if ( !gi )
        return false;

    Rect imageRect;
    GraphicsImportGetNaturalBounds( gi, &imageRect );

    long cx, cy;
    cx = imageRect.right - imageRect.left;
    cy = imageRect.bottom - imageRect.top;

    BITMAPINFO* pBmpInfo;    
    PBYTE pBmpBits;
    HWND hwnd = GetActiveWindow();
    HDC hDc = GetDC( hwnd );
    HBITMAP hbm = create_compatible_dib( hDc, NULL, cx, -cy,
                                         &pBmpInfo, (void**)&pBmpBits );

    GWorldPtr pGW;
    NewGWorldFromHBITMAP( &pGW, nil, nil, 0, (void*)hbm, (void*)hDc );
    GraphicsImportSetGWorld( gi, pGW, nil );
    ComponentResult er = GraphicsImportDraw( gi );

    CloseComponent( gi );
//    DisposeGWorld( pGW );
    ReleaseDC( hwnd, hDc );
    free( pBmpInfo );

    if ( hbm ) {
        *phDib = hbm;

        pRect->left = imageRect.left;
        pRect->top = imageRect.top;
        pRect->right = imageRect.right;
        pRect->bottom = imageRect.bottom;

        bRet = true;
    }

    return bRet;
}


// end exported api
///////////////////////////////////////////////////////////////////////////////


// create_compatible_dib:
// Create a DIB section with an optimal format w.r.t. the specified hdc. 
// 
// If DIB <= 8bpp, then the DIB color table is initialized based on the 
// specified palette.  If the palette handle is NULL, then the system 
// palette is used. 
// 
// Note: The hdc must be a direct DC (not an info or memory DC). 
// 
// Note: On palettized displays, if the system palette changes the 
//       UpdateDIBColorTable function should be called to maintain 
//       the identity palette mapping between the DIB and the display.

HBITMAP create_compatible_dib( HDC hdc, HPALETTE hpal, ULONG ulWidth,
                       ULONG ulHeight, BITMAPINFO** ppInfo, PVOID *ppvBits ) 
{ 
    HBITMAP hbmRet = (HBITMAP) NULL; 

    // validate the dc handle
    if ( GetObjectType( hdc ) != OBJ_DC ) { 
        ODS( "create_compatible_dib: not OBJ_DC\n" );
        assert( 0 );        
        return hbmRet; 
    } 

    size_t nBIBytes = sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * 255);
    PBITMAPINFO pbmi = (PBITMAPINFO) malloc( nBIBytes );

    if ( !pbmi ) {
        ODS( "create_compatible_dib: alloc failed\n" );
        assert( 0 );
        return hbmRet;    
    }

    memset( pbmi, '\0', nBIBytes );

    if ( fill_bitmap_info( hdc, hpal, pbmi ) ) {

        // change bitmap size to specified dimensions
        pbmi->bmiHeader.biWidth = ulWidth; 
        pbmi->bmiHeader.biHeight = ulHeight;

        if (pbmi->bmiHeader.biCompression == BI_RGB) { 
            pbmi->bmiHeader.biSizeImage = 0;
        }
        else {
            if ( pbmi->bmiHeader.biBitCount == 16 ) 
                pbmi->bmiHeader.biSizeImage = ulWidth * ulHeight * 2; 
            else if ( pbmi->bmiHeader.biBitCount == 32 ) 
                pbmi->bmiHeader.biSizeImage = ulWidth * ulHeight * 4; 
            else 
                pbmi->bmiHeader.biSizeImage = 0; 
        }
 
        // Create the DIB section.  Let Win32 allocate the memory and return 
        // a pointer to the bitmap surface. 
        hbmRet = CreateDIBSection( hdc, pbmi, DIB_RGB_COLORS, ppvBits,
                                   NULL, 0 );

        // this is big-time important on NT
        GdiFlush();
 
        if ( hbmRet ) {
            // pass back the bitmapinfo; caller must free
            *ppInfo = pbmi;
        }
        else {
            ODS( "create_compatible_dib: CreateDIBSection failed\n" ); 
            assert( 0 );
        } 
    } 
    else { 
        ODS( "create_compatible_dib: fill_bitmap_info failed\n" );
        assert( 0 );
    } 
 
    return hbmRet; 
} 


// fill_bitmap_info:
// Fills in the fields of a BITMAPINFO so that we can create a bitmap 
// that matches the format of the display. 
// 
// This is done by creating a compatible bitmap and calling GetDIBits 
// to return the color masks.  This is done with two calls.  The first 
// call passes in biBitCount = 0 to GetDIBits which will fill in the 
// base BITMAPINFOHEADER data.  The second call to GetDIBits (passing 
// in the BITMAPINFO filled in by the first call) will return the color 
// table or bitmasks, as appropriate.

static BOOL fill_bitmap_info( HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi )
{ 
    HBITMAP hbm; 
    BOOL    bRet = FALSE; 

    // Create a dummy bitmap from which we can query color format info
    // about the device surface. 
    if ( (hbm = CreateCompatibleBitmap( hdc, 1, 1 )) != NULL ) {

        // Call first time to fill in BITMAPINFO header. 
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
        GetDIBits( hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS ); 
 
        if ( pbmi->bmiHeader.biBitCount <= 8 ) { 
            bRet = fill_color_table( hdc, hpal, pbmi );
        } 
        else {
            if ( pbmi->bmiHeader.biCompression == BI_BITFIELDS ) {

                // Call a second time to get the color masks. 
                GetDIBits( hdc, hbm, 0, pbmi->bmiHeader.biHeight, NULL,
                           pbmi, DIB_RGB_COLORS );

                // if this is Win95/98, bmiColors is probably hosed, we'll
                // patch it up. I still don't totally understand this stuff.
                // custom optimizing color tables aren't allowed on Win95/98,
                // so maybe some drivers just ignore it alltogether? Is this
                // even driver dependent? if not, the following code might
                // handle 95/98 completely, without the call to GetDIBits.
                // God. Help. Me.

                if ( g_pwvi->IsWin95() || g_pwvi->IsWin98() ) {
                    WORD nBits = pbmi->bmiHeader.biBitCount;
                    DWORD* pClrMasks = (DWORD*) GET_COLORTABLE_PTR( pbmi );

                    if ( nBits == 16 ) {
                        pClrMasks[0] = (DWORD) 0x7c00;   // red
                        pClrMasks[1] = (DWORD) 0x03e0;   // green
                        pClrMasks[2] = (DWORD) 0x001f;   // blue
                    }
                    else if ( nBits == 32 ) {
                        pClrMasks[0] = 0x00FF0000;       // red
                        pClrMasks[1] = 0x0000FF00;       // green
                        pClrMasks[2] = 0x000000FF;       // blue
                    }
                }
            }
 
            bRet = TRUE; 
        }
 
        DeleteObject( hbm ); 
    } 
    else { 
        ODS( "fill_bitmap_info: CreateCompatibleBitmap failed\n" ); 
    } 
 
    return bRet; 
} 
 
// fill_color_table:
// Initialize the color table of the BITMAPINFO pointed to by pbmi.  Colors 
// are set to the current system palette. 
//
// Note: call only valid for displays of 8bpp or less.

#define MAX_PE_SIZE (sizeof(PALETTEENTRY) * 256)
static BOOL fill_color_table( HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi ) 
{
    BOOL bRet = FALSE;

    BYTE aj[MAX_PE_SIZE];
    memset( aj, '\0', MAX_PE_SIZE );
    LPPALETTEENTRY lppe = (LPPALETTEENTRY) aj;
    
    RGBQUAD *prgb = (RGBQUAD*) GET_COLORTABLE_PTR( pbmi);

    ULONG cColors;
    cColors = 1 << pbmi->bmiHeader.biBitCount; 

    if ( cColors <= 256 ) { 
        if ( hpal ? GetPaletteEntries( hpal, 0, cColors, lppe )
             : get_system_palette_entries( hdc, 0, cColors, lppe ) ) {

            UINT i; 
 
            for (i = 0; i < cColors; i++) { 
                prgb[i].rgbRed      = lppe[i].peRed; 
                prgb[i].rgbGreen    = lppe[i].peGreen; 
                prgb[i].rgbBlue     = lppe[i].peBlue; 
                prgb[i].rgbReserved = 0;
            }

            bRet = TRUE; 
        } 
        else { 
            ODS( "fill_color_table: get_system_palette_entries failed\n" ); 
        } 
    }

    return bRet; 
} 


// get_system_palette_entries:
// Internal version of GetSystemPaletteEntries. 
// 
// GetSystemPaletteEntries fails on some 4bpp devices.  This version 
// will detect the 4bpp case and supply the hardcoded 16-color VGA palette. 
// Otherwise, it will pass the call on to GDI's GetSystemPaletteEntries. 
// 
// It is expected that this call will only be called in the 4bpp and 8bpp 
// cases as it is not necessary for OpenGL to query the system palette 
// for > 8bpp devices.

static PALETTEENTRY gapeVgaPalette[16] = 
{ 
    { 0,   0,   0,    0 }, 
    { 0x80,0,   0,    0 }, 
    { 0,   0x80,0,    0 }, 
    { 0x80,0x80,0,    0 }, 
    { 0,   0,   0x80, 0 }, 
    { 0x80,0,   0x80, 0 }, 
    { 0,   0x80,0x80, 0 }, 
    { 0x80,0x80,0x80, 0 }, 
    { 0xC0,0xC0,0xC0, 0 }, 
    { 0xFF,0,   0,    0 }, 
    { 0,   0xFF,0,    0 }, 
    { 0xFF,0xFF,0,    0 }, 
    { 0,   0,   0xFF, 0 }, 
    { 0xFF,0,   0xFF, 0 }, 
    { 0,   0xFF,0xFF, 0 }, 
    { 0xFF,0xFF,0xFF, 0 } 
}; 
 
static UINT get_system_palette_entries( HDC hdc, UINT iStartIndex,
                                        UINT nEntries, LPPALETTEENTRY lppe ) 
{ 
    int nDeviceBits;
    nDeviceBits = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
 
    // Some 4bpp displays will fail the GetSystemPaletteEntries call. 
    // So if detected, return the hardcoded table. 
    if ( nDeviceBits == 4 ) { 
        if ( lppe ) { 
            nEntries = min( nEntries, (16 - iStartIndex) ); 
            memcpy( lppe, &gapeVgaPalette[iStartIndex],
                    nEntries * sizeof(PALETTEENTRY) ); 
        } 
        else {
            nEntries = 16;
        }
 
        return nEntries; 
    } 
    else {
        return GetSystemPaletteEntries( hdc, iStartIndex, nEntries, lppe ); 
    } 
} 

/*
static HPALETTE create_dib_palette( PBITMAPINFO pbmi )
{
HPALETTE hRetPal = NULL;

int iNumColors = 1 << pbmi->bmiHeader.biBitCount;
size_t sizeLogPalette = (sizeof(LOGPALETTE) +
(sizeof(PALETTEENTRY) * iNumColors));

PLOGPALETTE pLogPal = (PLOGPALETTE) malloc( sizeLogPalette );
pLogPal->palVersion = 0x300;
pLogPal->palNumEntries = (WORD) iNumColors;

RGBQUAD* prgbq = GET_COLORTABLE_PTR( pbmi );

for ( int i = 0; i < iNumColors; i++ ) {
pLogPal->palPalEntry[i].peRed   = prgbq[i].rgbRed;
pLogPal->palPalEntry[i].peGreen = prgbq[i].rgbGreen;
pLogPal->palPalEntry[i].peBlue  = prgbq[i].rgbBlue;
pLogPal->palPalEntry[i].peFlags = 0;
}

hRetPal = CreatePalette( pLogPal );
free( pLogPal );

return hRetPal;
}
*/
