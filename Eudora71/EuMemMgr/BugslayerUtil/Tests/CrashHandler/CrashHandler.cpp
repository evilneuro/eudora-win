/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"

// Globals to control what gets done here.
BOOL  bGetFaultReason = TRUE ;
BOOL  bGetStackTraceString = TRUE ;
BOOL  bShowRegs = TRUE ;
BOOL  bLetItCrash = FALSE ;
DWORD dwOpts ;

LONG __stdcall ExcepCallBack ( EXCEPTION_POINTERS * pExPtrs )
{
    const TCHAR * szBuff = NULL ;
    
    if ( TRUE == bGetFaultReason )
    {
        szBuff = GetFaultReason ( pExPtrs ) ;
        _tprintf ( _T ( "%s\n" ) , szBuff ) ;
    }

    if ( TRUE == bShowRegs )
    {
        szBuff = GetRegisterString ( pExPtrs ) ;
        _tprintf ( _T ( "%s\n" ) , szBuff ) ;
    }

    if ( TRUE == bGetStackTraceString )
    {
        szBuff = GetFirstStackTraceString ( dwOpts  ,
                                            pExPtrs  ) ;
        do
        {
            _tprintf ( _T ( "%s\n" ) , szBuff ) ;
            fflush ( stdout ) ;
            szBuff = GetNextStackTraceString ( dwOpts , pExPtrs ) ;
        }
        while ( NULL != szBuff ) ;
    }

    if ( TRUE == bLetItCrash )
    {
        return ( EXCEPTION_CONTINUE_SEARCH ) ;
    }
    else
    {
        return ( EXCEPTION_EXECUTE_HANDLER ) ;
    }
}

void Baz ( int i )
{
    if ( FALSE == bLetItCrash )
    {
        __try
        {
            int * p = NULL ;
            i = 0 ;
            *p = 0 ;
        }
        __except ( ExcepCallBack ( GetExceptionInformation ( ) ) )
        {
        }
    }
    else
    {
        int * q = NULL ;
        *q = 19 ;
    }
}

void Bar ( int i )
{
    Baz ( i + 1 ) ;
}

void Foo ( int i )
{
    Bar ( i + 1 ) ;
}


void _tmain ( void )
{
    // Test 1 - SetCrashHandler with NULL.
    _tprintf ( _T ( "Test 1 - SetCrashHandler with NULL\n\n" ) ) ;
    if ( TRUE != SetCrashHandlerFilter ( NULL ) )
    {
        _tprintf ( _T ( "Failed!! -> Test 1 - SetCrashHandler with NULL\n" ) ) ;
        return ;
    }

    // Test 2 - SetCrashHandler with good callback.
    _tprintf ( _T ( "Test 2 - SetCrashHandler with good callback\n\n" ) ) ;
    if ( TRUE != SetCrashHandlerFilter ( ExcepCallBack ) )
    {
        _tprintf ( _T ( "Failed!! -> Test 2 - SetCrashHandler with good " )
                   _T ( "callback\n" ) ) ;
        return ;
    }

    // Test 3 - Do just GetFaultReason (Need to check output!)
    bGetFaultReason = TRUE ;
    bGetStackTraceString = FALSE ;
    bShowRegs = FALSE ;
    dwOpts = 0 ;
    _tprintf ( _T ( "Test 3 - Do just GetFaultReason\n" ) ) ;
    Foo ( 3 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 4 - Do StackWalk - Options = 0
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = 0 ;
    _tprintf ( _T ( "Test 4 - Do StackWalk - Options = 0\n" ) ) ;
    Foo ( 4 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 5 - Do StackWalk - Options = GSTSO_PARAMS
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_PARAMS ;
    _tprintf ( _T ( "Test 5 - Do StackWalk - Options = GSTSO_PARAMS\n" ) ) ;
    Foo ( 5 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 6 - Do StackWalk - Options = GSTSO_MODULE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_MODULE ;
    _tprintf ( _T ( "Test 6 - Do StackWalk - Options = GSTSO_MODULE\n" ) ) ;
    Foo ( 6 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 7 - Do StackWalk - Options = GSTSO_SYMBOL
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_SYMBOL ;
    _tprintf ( _T ( "Test 7 - Do StackWalk - Options = GSTSO_SYMBOL\n" ) ) ;
    Foo ( 7 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 8 - Do StackWalk - Options = GSTSO_SRCLINE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_SRCLINE ;
    _tprintf ( _T ( "Test 8 - Do StackWalk - Options = GSTSO_SRCLINE\n" ) ) ;
    Foo ( 8 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 9 - Do StackWalk - Options = GSTSO_PARAMS | GSTSO_MODULE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_PARAMS | GSTSO_MODULE ;
    _tprintf ( _T ( "Test 9 - Do StackWalk - Options = " )
               _T ( "GSTSO_PARAMS | GSTSO_MODULE\n" ) ) ;
    Foo ( 9 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 10 - Do StackWalk - Options = GSTSO_PARAMS | GSTSO_SYMBOL
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_PARAMS | GSTSO_SYMBOL ;
    _tprintf ( _T ( "Test 10 - Do StackWalk - Options = " )
               _T ( "GSTSO_PARAMS | GSTSO_SYMBOL\n" ) ) ;
    Foo ( 10 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 11 - Do StackWalk - Options = GSTSO_PARAMS | GSTSO_SRCLINE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_PARAMS | GSTSO_SRCLINE ;
    _tprintf ( _T ( "Test 11 - Do StackWalk - Options = " )
               _T ( "GSTSO_PARAMS | GSTSO_SRCLINE\n" ) ) ;
    Foo ( 11 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 12 - Do StackWalk - Options = GSTSO_MODULE | GSTSO_SYMBOL
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_MODULE | GSTSO_SYMBOL ;
    _tprintf ( _T ( "Test 12 - Do StackWalk - Options = " )
               _T ( "GSTSO_MODULE | GSTSO_SYMBOL\n" ) ) ;
    Foo ( 12 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 13 - Do StackWalk - Options = GSTSO_MODULE | GSTSO_SRCLINE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_MODULE | GSTSO_SRCLINE ;
    _tprintf ( _T ( "Test 13 - Do StackWalk - Options = " )
               _T ( "GSTSO_MODULE | GSTSO_SRCLINE\n" ) ) ;
    Foo ( 13 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 14 - Do StackWalk - Options = GSTSO_SYMBOL | GSTSO_SRCLINE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_SYMBOL | GSTSO_SRCLINE ;
    _tprintf ( _T ( "Test 14 - Do StackWalk - Options = " )
               _T ( "GSTSO_SYMBOL | GSTSO_SRCLINE\n" ) ) ;
    Foo ( 14 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 15 - Do StackWalk - Options = GSTSO_PARAMS|GSTSO_MODULE
    //                                    |GSTSO_SYMBOL|GSTSO_SRCLINE
    bGetFaultReason = FALSE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
    _tprintf ( _T ( "Test 15 - Do StackWalk - Options = " )
               _T ( "GSTSO_PARAMS|GSTSO_MODULE|GSTSO_SYMBOL|GSTSO_SRCLINE\n" ) ) ;
    Foo ( 15 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 16 - Full GetFaultReason and GetStackTraceString
    bGetFaultReason = TRUE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = FALSE ;
    dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
    _tprintf ( _T ( "Test 16 - Full GetFaultReason and GetStackTraceString\n"));
    Foo ( 16 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test 17 - Show registers only.
    bGetFaultReason = FALSE ;
    bGetStackTraceString = FALSE ;
    bShowRegs = TRUE ;
    dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
    _tprintf ( _T ( "Test 17 - Show registers only\n"));
    Foo ( 17 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

    // Test the module limiting stuff.
    // Test 18 - Add three good limit module handles.
    _tprintf ( _T ( "Test 18 - Add three good limit module handles\n\n" ) ) ;
    if ( ( FALSE ==
             AddCrashHandlerLimitModule ( GetModuleHandle ( NULL ) ) )||
          ( FALSE ==
             AddCrashHandlerLimitModule(GetModuleHandle(_T("DBGHELP.DLL"))))||
          ( FALSE ==
             AddCrashHandlerLimitModule(
                                    GetModuleHandle(_T("KERNEL32.DLL"))))  )
    {
        _tprintf ( _T ( "Test 18 - Add three good limit module " )
                   _T ( "handles -> failed!\n" ) ) ;
        return ;
    }

    // Test 19 - Check limit module count.
    _tprintf ( _T ( "Test 19 - Check limit module count\n\n" ) ) ;
    if ( 3 != GetLimitModuleCount ( ) )
    {
        _tprintf ( _T ( "Test 19 - Check limit module count -> failed!!\n" ) ) ;
        return ;
    }

    // Test 20 - Try and add a bad module.
    _tprintf ( _T ( "Test 20 - Try and add a bad module\n\n" ) ) ;
    if ( TRUE == AddCrashHandlerLimitModule ( NULL ) )
    {
        _tprintf ( _T ( "Test 20 - Try and add a bad module -> failed!\n" ) ) ;
        return ;
    }

    // Test 21 - Get the three good modules.
    _tprintf ( _T ( "Test 21 - Get the three good modules.\n\n" ) ) ;
    HMODULE ahMod[ 3 ] ;
    if ( GLMA_SUCCESS != GetLimitModulesArray ( ahMod , 3 ) )
    {
        _tprintf ( _T ( "Test 21 - Get the three good modules -> failed!\n" ) ) ;
        return ;
    }


    // Test 22 - Check the same in and same out for modules.
    _tprintf ( _T ( "Test 22 - Check the same in and same out for modules\n\n"));
    if ( ( GetModuleHandle ( NULL           ) != ahMod[ 0 ] ) ||
         ( GetModuleHandle ( _T ( "DBGHELP.DLL"  ) ) != ahMod[ 1 ] ) ||
         ( GetModuleHandle ( _T ( "KERNEL32.DLL" ) ) != ahMod[ 2 ] )   )
    {
        _tprintf ( _T ( "Test 22 - Check the same in and same out " )
                   _T ( "for modules -> failed!\n" ) ) ;
        return ;
    }

    // Test 23 - Bad array param to GetLimitModulesArray.
    _tprintf ( _T ( "Test 23 - Bad array param to GetLimitModulesArray\n\n" ) ) ;
    if ( GLMA_SUCCESS == GetLimitModulesArray ( (HMODULE*)NULL , 3 ) )
    {
        _tprintf ( _T ( "Test 23 - Bad array param to " )
                   _T ( "GetLimitModulesArray ->failed!\n" ) ) ;
        return ;
    }

    // Test 24 - Bad array param to GetLimitModulesArray.
    _tprintf ( _T ( "Test 24 - Bad array param to GetLimitModulesArray.\n\n"));
    if ( GLMA_SUCCESS == GetLimitModulesArray (
                                      (HMODULE*)GetModuleHandle , 3 ) )
    {
        _tprintf ( _T ( "Test 24 - Bad array param to " )
                   _T ( "GetLimitModulesArray ->failed!\n" ) ) ;
        return ;
    }

    // Test 25 - Bad size param to GetLimitModulesArray.
    _tprintf ( _T ( "Test 25 - Bad size param to GetLimitModulesArray.\n\n")) ;
    if ( GLMA_SUCCESS == GetLimitModulesArray (
                                      (HMODULE*)GetModuleHandle , 0 ) )
    {
        _tprintf ( _T ( "Test 25 - Bad size param to " )
                   _T ( "GetLimitModulesArray ->failed!\n" ) ) ;
        return ;
    }

    HMODULE ahModTwo[ 1 ] ;
    // Test 26 - Too small an array.
    _tprintf ( _T ( "Test 26 - Too small an array\n\n" ) ) ;
    if ( GLMA_SUCCESS == GetLimitModulesArray ( ahModTwo , 1 ) )
    {
        _tprintf ( _T ( "Test 26 - Too small an array to " )
                   _T ( "GetLimitModulesArray ->failed!\n" ) ) ;
        return ;
    }

    // Test 27 - Show all, but let it crash.
    bGetFaultReason = TRUE ;
    bGetStackTraceString = TRUE ;
    bShowRegs = TRUE ;
    bLetItCrash = TRUE ;
    dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
    _tprintf ( _T ( "Test 27 - Show all, but let it crash\n"));
    Foo ( 27 ) ;
    _tprintf ( _T ( "\n\n" ) ) ;

}

