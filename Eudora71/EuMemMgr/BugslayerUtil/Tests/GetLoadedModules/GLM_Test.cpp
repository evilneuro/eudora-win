/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "BugslayerUtil.h"

void main ( void )
{
    HMODULE ahMods[ 250 ] ;
    DWORD   dwNumMods ;
    BOOL    bRet ;

    // Get the loaded modules for this process.
    memset ( &ahMods , NULL , sizeof ( ahMods ) ) ;
    _tprintf ( _T ( "About to call GetLoadedModules - 1\n" ) ) ;
    _flushall ( ) ;
    bRet = GetLoadedModules ( GetCurrentProcessId ( ) ,
                              250                     ,
                              (HMODULE*)&ahMods       ,
                              &dwNumMods               ) ;
    if ( ( FALSE == bRet ) || ( 0 == dwNumMods ) )
    {
        _tprintf ( _T ( "Failed getting loaded modules " )
                   _T ( "for this process!\n" ) ) ;
        return ;
    }

    _tprintf ( _T ( "Modules for this process - \n" ) ) ;
    _flushall ( ) ;

    TCHAR szBuff[ MAX_PATH ] ;

    for ( UINT i = 0 ; i < dwNumMods ; i++ )
    {
        GetModuleFileName ( ahMods[ i ] , szBuff , sizeof ( szBuff ) ) ;
        _tprintf ( _T ( "   0x%08X - %s\n" ) , ahMods[ i ] , szBuff ) ;
    }

    // Now test passing in NULL values to get just the count.
    DWORD dwZeroTest = 0 ;
    _tprintf ( _T ( "About to call GetLoadedModules - 2\n" ) ) ;
    bRet = GetLoadedModules ( GetCurrentProcessId ( ) ,
                              0                       ,
                              NULL                    ,
                              &dwZeroTest              ) ;
    if ( ( FALSE == bRet ) || ( dwZeroTest != dwNumMods ) )
    {
        _tprintf ( _T ( "FAILED - test passing in NULL values to get " )
                   _T ( "just the count.\n" ) ) ;
        _flushall ( ) ;
        DebugBreak ( ) ;
    }

    // Pass in an invalid process handle.
    _tprintf ( _T ( "About to call GetLoadedModules - 3\n" ) ) ;
    DWORD dwBadPID = 0 ;
    bRet = GetLoadedModules ( 1 , 250 , (HMODULE*)&ahMods , &dwBadPID );

    if ( ( TRUE == bRet ) || ( 0 != dwBadPID ) )
    {
        _tprintf ( _T ( "FAILED - Pass in an invalid ) \
                   _T ( process handle.\n" )          ) ;
        _flushall ( ) ;
        DebugBreak ( ) ;
    }

    // Check a bad parameter as the module array.
    DWORD dwBadParam = 0 ;
    _tprintf ( _T ( "About to call GetLoadedModules - 4\n" ) ) ;
    bRet = GetLoadedModules ( GetCurrentProcessId ( ) ,
                               250                    ,
                               NULL                   ,
                               &dwBadParam             ) ;

    if ( ( TRUE == bRet                                ) ||
         ( ERROR_INVALID_PARAMETER != GetLastError ( ) )    )
    {
        _tprintf ( _T ( "FAILED - Check a bad parameter as " )
                   _T ( "the module array.\n" ) ) ;
        _flushall ( ) ;
        DebugBreak ( ) ;
    }

    // Pass in a bad real count value.
    dwBadParam = 0 ;
    _tprintf ( _T ( "About to call GetLoadedModules - 7\n" ) ) ;
    bRet = GetLoadedModules ( GetCurrentProcessId ( ) ,
                              250                     ,
                              (HMODULE*)&ahMods       ,
                              NULL                     ) ;

    if ( ( TRUE == bRet                                ) ||
         ( ERROR_INVALID_PARAMETER != GetLastError ( ) )    )
    {
        _tprintf ( _T ( "FAILED - Pass in a bad real count value.\n"));
        _flushall ( ) ;
        DebugBreak ( ) ;
    }

    // Pass in a buffer size that is to small to hold the output.
    memset ( &ahMods , NULL , sizeof ( ahMods ) ) ;
    dwBadParam = 0 ;
    _tprintf ( _T ( "About to call GetLoadedModules - 8\n" ) ) ;
    bRet = GetLoadedModules ( GetCurrentProcessId ( ) ,
                              1                       ,
                              (HMODULE*)&ahMods       ,
                              &dwBadParam              ) ;

    if ( ( TRUE == bRet                                  ) ||
         ( ERROR_INSUFFICIENT_BUFFER != GetLastError ( ) )    )
    {
        _tprintf ( _T ( "FAILED - Pass in a buffer size that is to " )
                   _T ( "small to hold the output.\n" ) ) ;
        _flushall ( ) ;
        DebugBreak ( ) ;
    }

    _tprintf ( _T ( "Correct end of program\n" ) ) ;
}
