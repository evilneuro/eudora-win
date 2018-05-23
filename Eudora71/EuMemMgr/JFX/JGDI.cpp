/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "JFX.h"

#define _countof(array) (sizeof(array)/sizeof(array[0]))

BOOL JFont :: CreatePointFont ( int      nPointSize    ,
                                LPCTSTR  szFaceName    ,
                                HDC      hDC           ,
                                BOOL     bItalic       ,
                                BOOL     bBold          )
{
    ASSERT ( FALSE == ::IsBadStringPtr ( szFaceName                  ,
                                         MAX_PATH * sizeof ( TCHAR )) );

    LOGFONT logFont ;
    memset ( &logFont , NULL , sizeof ( LOGFONT ) ) ;

    logFont.lfCharSet = DEFAULT_CHARSET ;
    logFont.lfHeight = nPointSize * 10 ;
    logFont.lfItalic = (BYTE)bItalic ;
    if ( TRUE == bBold )
    {
        logFont.lfWeight = FW_BOLD ;
    }

    lstrcpyn ( logFont.lfFaceName             ,
               szFaceName                     ,
               _countof ( logFont.lfFaceName ) ) ;

    HDC hLocalDC = NULL ;
    if ( NULL == hDC )
    {
        hLocalDC = ::GetDC ( NULL ) ;
    }
    else
    {
        hLocalDC = hDC ;
    }

    // Convert nPointSize to logical units based on pDC
    POINT pt ;
    pt.y = ::GetDeviceCaps ( hLocalDC, LOGPIXELSY ) *
            ( logFont.lfHeight );
    pt.y /= 720 ;    // 72 points/inch, 10 decipoints/point
    ::DPtoLP ( hLocalDC , &pt , 1 ) ;
    POINT ptOrg = { 0, 0 } ;
    ::DPtoLP ( hLocalDC , &ptOrg , 1 ) ;
    logFont.lfHeight = -abs ( pt.y - ptOrg.y ) ;

    if ( NULL == hDC )
    {
        ReleaseDC ( NULL , hLocalDC ) ;
    }

    m_hObject = (HGDIOBJ)::CreateFontIndirect ( &logFont ) ;
    ASSERT ( NULL != m_hObject ) ;

    return ( NULL != m_hObject ) ;
}

