/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "BugslayerUtil.h"

typedef int (__stdcall *MESSAGEBOXPROCA)(HWND , LPCSTR , LPCSTR , UINT) ;
typedef int (__stdcall *MESSAGEBOXPROCW)(HWND , LPCWSTR , LPCWSTR , UINT) ;

MESSAGEBOXPROCA pfnOrigMsgBoxA = NULL ;
MESSAGEBOXPROCW pfnOrigMsgBoxW = NULL ;

int WINAPI SimonMsgBoxA ( HWND   hWnd        ,
                          LPCSTR lpText      ,
                          LPCSTR lpCaption   ,
                          UINT   uType        )
{
    int iRet ;
    LPSTR szNewTitle = new char[ strlen ( lpCaption ) + 40 ] ;

    strcpy ( szNewTitle , "Simon SezA: " ) ;
    strcat ( szNewTitle , lpCaption ) ;
    iRet = pfnOrigMsgBoxA ( hWnd , lpText , szNewTitle , uType ) ;
    delete [] szNewTitle ;
    return ( iRet ) ;
}

int WINAPI SimonMsgBoxW ( HWND    hWnd        ,
                          LPCWSTR lpText      ,
                          LPCWSTR lpCaption   ,
                          UINT    uType        )
{
    int iRet ;
    LPWSTR szNewTitle = new wchar_t[ lstrlenW ( lpCaption ) + 40 ] ;

    lstrcpyW ( (LPWSTR)szNewTitle , L"Simon SezW: " ) ;
    lstrcatW ( szNewTitle , lpCaption ) ;
    iRet = pfnOrigMsgBoxW ( hWnd , lpText , szNewTitle , uType ) ;
    delete [] szNewTitle ;
    return ( iRet ) ;
}

void GetAroundOptimizerOne ( char * szTitle )
{
    MessageBoxA ( GetForegroundWindow ( )        ,
                  "We should be hooked - Do you see Simon "
                       "Sez in title bar?",
                  szTitle                        ,
                  MB_OK                           ) ;
}

void GetAroundOptimizerTwo ( char * szTitle )
{
    MessageBoxA ( GetForegroundWindow ( )            ,
                  "We should not be hooked at all - "
                    "No Simon Sez in title bar!"     ,
                  szTitle                            ,
                  MB_OK                               ) ;

}

void GetAroundOptimizerThree ( wchar_t * szTitle )
{
    MessageBoxW ( GetForegroundWindow ( )        ,
                  L"We should be hooked (WIDE)- Do you see Simon "
                  L"Sez in title bar?",
                  szTitle                        ,
                  MB_OK                           ) ;
}

void GetAroundOptimizerFour ( wchar_t * szTitle )
{
    MessageBoxW ( GetForegroundWindow ( )            ,
                  L"We should not be hooked (WIDE) at all - "
                  L"No Simon Sez in title bar!"      ,
                  szTitle                            ,
                  MB_OK                               ) ;
}

void main ( void )
{
    MessageBoxA ( GetForegroundWindow ( )                            ,
                  "Ain't hooked yet! - No Simon Sez in title bar"    ,
                  "HookImportedFunctionsByName! ANSI"                ,
                  MB_OK                                               ) ;

    HOOKFUNCDESC stToHook ;

    stToHook.szFunc = "MessageBoxA" ;
    stToHook.pProc  = (PROC)SimonMsgBoxA ;

    DWORD uiCount ;

    BOOL bRet =
        HookImportedFunctionsByNameA ( GetModuleHandle ( NULL ) ,
                                       "USER32.DLL"             ,
                                       1                        ,
                                       &stToHook                ,
                                       (PROC*)&pfnOrigMsgBoxA   ,
                                       &uiCount                  ) ;
    if ( FALSE == bRet )
    {
        printf ( "We couldn't hook it!\n" ) ;
    }

    // If the code is optimized, calls to MessageBoxA in this function
    // are enregisted (as they should be!), but it makes it difficult
    // to see the results of the hook as all the calls go through the
    // register, not the IAT!

    GetAroundOptimizerOne ( "HookImportedFunctionsByNameA!" ) ;

    // Now unhook.


    stToHook.szFunc = "MessageBoxA" ;
    stToHook.pProc  = (PROC)pfnOrigMsgBoxA ;

    if ( FALSE ==
            HookImportedFunctionsByName ( GetModuleHandle ( NULL ) ,
                                          L"USER32.DLL"            ,
                                          1                        ,
                                          &stToHook                ,
                                          (PROC*)&pfnOrigMsgBoxA   ,
                                          &uiCount                   ) )
    {
        printf ( "We couldn't unhook it!\n" ) ;
    }

    // Call message box again!
    GetAroundOptimizerTwo ( "HookImportedFunctionsByNameA!" ) ;
    
    MessageBoxW ( GetForegroundWindow ( )                            ,
                  L"Ain't hooked yet! - No Simon Sez in title bar"   ,
                  L"HookImportedFunctionsByName! WIDE"               ,
                  MB_OK                                              ) ;


    stToHook.szFunc = "MessageBoxW" ;
    stToHook.pProc  = (PROC)SimonMsgBoxW ;
    bRet =
        HookImportedFunctionsByNameW ( GetModuleHandle ( NULL ) ,
                                       L"USER32.DLL"            ,
                                       1                        ,
                                       &stToHook                ,
                                       (PROC*)&pfnOrigMsgBoxW   ,
                                       &uiCount                  ) ;
    if ( FALSE == bRet )
    {
        printf ( "We couldn't hook it!\n" ) ;
    }

    GetAroundOptimizerThree ( L"HookImportedFunctionsByNameW!" ) ;

    // Now unhook.
    stToHook.szFunc = "MessageBoxW" ;
    stToHook.pProc  = (PROC)pfnOrigMsgBoxW ;
    if ( FALSE ==
            HookImportedFunctionsByNameW ( GetModuleHandle ( NULL ) ,
                                           L"USER32.DLL"             ,
                                           1                        ,
                                           &stToHook                ,
                                           (PROC*)&pfnOrigMsgBoxW   ,
                                           &uiCount                   ) )
    {
        printf ( "We couldn't unhook it!\n" ) ;
    }

    // Call message box again!
    GetAroundOptimizerFour ( L"HookImportedFunctionsByNameW!" ) ;

}

