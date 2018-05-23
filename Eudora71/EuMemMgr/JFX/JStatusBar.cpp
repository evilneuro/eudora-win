/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "JFX.h"


LRESULT JStatusBar :: OnMenuSelect ( UINT  nItemID  ,
                                     UINT  nFlags   ,
                                     HMENU hSysMenu  )
{
    // The buffer for the 1st pane's text.
    static TCHAR szBuffer[ MAX_PATH ] ;

    // Clear the buffer text.
    szBuffer[ 0 ] = _T ( '\0' ) ;

    // Has the menu been closed?  Also, if it's a system menu or
    // a seperator, do nothing.  I ignore popup menus as well.
    if ( ( ( 0xFFFF == nFlags ) && ( NULL == hSysMenu ) ) ||
         ( ( MF_SYSMENU | MF_SEPARATOR ) & nFlags       ) ||
         ( ( MF_POPUP & nFlags ) == MF_POPUP            )   )
    {
        SetText ( m_strDefault ) ;
        return ( 0 ) ;
    }
    else
    {
        // Load the string and set the text.
        VERIFY ( ::LoadString ( JfxGetApp()->GetResourceInstance ( ) ,
                                nItemID                              ,
                                szBuffer                             ,
                                sizeof ( szBuffer )                  ));

        SetText ( szBuffer ) ;
    }
    return ( 0 ) ;
}

