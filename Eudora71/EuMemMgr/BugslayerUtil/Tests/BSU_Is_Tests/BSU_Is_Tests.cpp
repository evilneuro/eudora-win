/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"

void main ( void ) 
{
    // Get the real list of modules.
    DWORD dwModCount = 0 ;
    HMODULE * hProcessMods = 
                AllocAndFillProcessModuleList ( GetProcessHeap ( ) ,
                                                &dwModCount         ) ;
    ASSERT ( NULL != hProcessMods ) ;
    if ( NULL == hProcessMods )
    {
        _tprintf ( _T ( "Unable to get the process module list.\n" ) ) ;
        return ;
    }                                                

    DWORD  dwProcID = GetCurrentProcessId ( ) ;
    HANDLE hProcHandle = GetCurrentProcess ( ) ;
    
    // Do the BSUGetModuleBaseName tests with good data.
    char szAnsi[ MAX_PATH ] ;
    DWORD dwRet = BSUGetModuleBaseNameA ( hProcHandle , 
                                          NULL        , 
                                          szAnsi      , 
                                          MAX_PATH     ) ;
    ASSERT ( 0 != dwRet ) ;
    if ( 0 == dwRet )
    {
        _tprintf ( _T ( "BSUGetModuleBaseNameA failed!!\n" ) ) ;
        return ;
    }
    _tprintf ( _T ( "BSUGetModuleBaseNameA returned : %S\n" ) , 
               szAnsi                                          ) ;
               
    wchar_t szWide[ MAX_PATH ] ;               
    dwRet = BSUGetModuleBaseNameW ( hProcHandle , 
                                    NULL        , 
                                    szWide      , 
                                    MAX_PATH     ) ;
    ASSERT ( 0 != dwRet ) ;
    if ( 0 == dwRet )
    {
        _tprintf ( _T ( "BSUGetModuleBaseNameW failed!!\n" ) ) ;
        return ;
    }
    _tprintf ( _T ( "BSUGetModuleBaseNameW returned : %s\n" ) , 
               szWide                                          ) ;

    ////////////////////////////////////////////////////////
    
    dwRet = BSUGetModuleFileNameExA ( dwProcID          ,
                                      hProcHandle       , 
                                      hProcessMods[ 0 ] , 
                                      szAnsi            , 
                                      MAX_PATH           ) ;
    ASSERT ( 0 != dwRet ) ;
    if ( 0 == dwRet )
    {
        _tprintf ( _T ( "BSUGetModuleFileNameExA failed!!\n" ) ) ;
        return ;
    }
    _tprintf ( _T ( "BSUGetModuleFileNameExA returned : %S\n" ) , 
               szAnsi                                            ) ;
               
    dwRet = BSUGetModuleFileNameExW ( dwProcID          ,
                                      hProcHandle       , 
                                      hProcessMods[ 0 ] , 
                                      szWide            , 
                                      MAX_PATH           ) ;
    ASSERT ( 0 != dwRet ) ;
    if ( 0 == dwRet )
    {
        _tprintf ( _T ( "BSUGetModuleFileNameExW failed!!\n" ) ) ;
        return ;
    }
    _tprintf ( _T ( "BSUGetModuleFileNameExW returned : %s\n" ) , 
               szWide                                            ) ;
    
    ////////////////////////////////////////////////////////
    
    BSUSetCurrentThreadNameW ( _T ( "Wide Thread Name" ) ) ;
    
    BSUSetCurrentThreadNameA ( "Ansi Thread Name" ) ;

    ////////////////////////////////////////////////////////
    
    _tprintf ( _T ( "IsNT                   = %d\n" ) , IsNT ( ) ) ;
    _tprintf ( _T ( "IsNT4                  = %d\n" ) , IsNT4 ( ) ) ;
    _tprintf ( _T ( "IsW2K                  = %d\n" ) , IsW2K ( ) ) ;
    _tprintf ( _T ( "IsXP                   = %d\n" ) , IsXP ( ) ) ;
    _tprintf ( _T ( "IsServer2003           = %d\n" ) , IsServer2003 ( ) ) ;
    _tprintf ( _T ( "IsW2KorBetter          = %d\n" ) , IsW2KorBetter ( ) ) ;
    _tprintf ( _T ( "IsXPorBetter           = %d\n" ) , IsXPorBetter ( ) ) ;
    _tprintf ( _T ( "IsServer2003orBetter   = %d\n" ) , IsServer2003orBetter ( ) ) ;
    
    
    ////////////////////////////////////////////////////////
    
    BOOL bRet = BSUSymInitializeA ( dwProcID , hProcHandle , NULL , TRUE ) ;
    ASSERT ( TRUE == bRet ) ;
    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "BSUSymInitializeA failed!!\n" ) ) ;
        return ;
    }
    SymCleanup ( hProcHandle ) ;
    
    bRet = BSUSymInitializeW ( dwProcID , hProcHandle , NULL , TRUE ) ;
    ASSERT ( TRUE == bRet ) ;
    if ( FALSE == bRet )
    {
        _tprintf ( _T ( "BSUSymInitializeW failed!!\n" ) ) ;
        return ;
    }
    SymCleanup ( hProcHandle ) ;
    
    ////////////////////////////////////////////////////////
    int iCount = GetSuperAssertionCount ( ) ;
    if ( 0 != iCount )
    {
        _tprintf ( _T ( "One of the tests failed!!\n" ) ) ;
    }
    
    DiagOutputW ( _T ( "All done!\n" ) ) ;
    
    MemStressInitializeA ( "BSU_Is_Tests.exe" ) ;
    MemStressTerminate ( ) ;
    
    // Get rid of the allocate list of modules.
    HeapFree ( GetProcessHeap ( ) , 0 , hProcessMods ) ;
}

