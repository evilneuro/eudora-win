/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

// Get everything included.
#include "PCH.h"
#include "Internal.h"

static HINSTANCE g_hInst = NULL ;

BOOL WINAPI DllMain ( HINSTANCE hInst       ,
                      DWORD     dwReason    ,
                      LPVOID    /*lpReserved*/ )
{
    BOOL bRet = TRUE ;
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH :
            // Save off the DLL hInstance.
            g_hInst = hInst ;
            // I don't need the thread notifications.
            DisableThreadLibraryCalls ( g_hInst ) ;
#ifdef _DEBUG
            bRet = InternalMemStressInitialize ( ) ;
#endif
            break ;
        case DLL_PROCESS_DETACH :
#ifdef _DEBUG
            bRet = InternalMemStressShutdown ( ) ;
#endif
            break ;
        default                 :
            break ;
    }
    return ( bRet ) ;
}

HINSTANCE GetBSUInstanceHandle ( void )
{
    return ( g_hInst ) ;
}
