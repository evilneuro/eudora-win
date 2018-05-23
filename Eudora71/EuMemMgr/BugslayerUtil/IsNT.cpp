/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#include "PCH.h"
#include "BugslayerUtil.h"
#include "Internal.h"

/*//////////////////////////////////////////////////////////////////////
                           File Scope Globals
//////////////////////////////////////////////////////////////////////*/
// Holds the version information.
static DWORD g_dwMajorVersion = 0 ;
static DWORD g_dwMinorVersion = 0 ;
static DWORD g_dwPlatformId = 0 ;

static BOOL GetTheVersion ( void )
{
    OSVERSIONINFO stOSVI ;

    memset ( &stOSVI , NULL , sizeof ( OSVERSIONINFO ) ) ;
    stOSVI.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO ) ;

    BOOL bRet = GetVersionEx ( &stOSVI ) ;
    ASSERT ( TRUE == bRet ) ;
    if ( FALSE == bRet )
    {
        TRACE0 ( "GetVersionEx failed!\n" ) ;
        return ( FALSE ) ;
    }
    
    g_dwMajorVersion = stOSVI.dwMajorVersion ;
    g_dwMinorVersion = stOSVI.dwMinorVersion ;
    g_dwPlatformId = stOSVI.dwPlatformId ;
    
    return ( TRUE ) ;
}

BOOL __stdcall IsNT ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( VER_PLATFORM_WIN32_NT == g_dwPlatformId )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsNT4 ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( 4 == g_dwMajorVersion                   )    )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsW2K ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( 5 == g_dwMajorVersion                   ) &&
         ( 0 == g_dwMinorVersion                   )    )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsXP ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( 5 == g_dwMajorVersion                   ) &&
         ( 1 == g_dwMinorVersion                   )    )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsServer2003 ( void ) 
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( 5 == g_dwMajorVersion                   ) &&
         ( 2 == g_dwMinorVersion                   )    )                 
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsW2KorBetter ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( 5 == g_dwMajorVersion                   )    )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsXPorBetter ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( g_dwMajorVersion >= 5                   ) &&
         ( g_dwMinorVersion >= 1                   )    )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}

BOOL __stdcall IsServer2003orBetter ( void )
{
    if ( 0 == g_dwMajorVersion )
    {
        GetTheVersion ( ) ;
    }
    if ( ( VER_PLATFORM_WIN32_NT == g_dwPlatformId ) &&
         ( g_dwMajorVersion >= 5                   ) &&
         ( g_dwMinorVersion >= 2                   )    )
    {
        return ( TRUE ) ;
    }
    return ( FALSE ) ;
}