/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "BugslayerUtil.h"

#ifdef SUPERASSERT_EMAIL
#undef SUPERASSERT_EMAIL
#define SUPERASSERT_EMAIL "john@wintellect.com"
#endif


const LPCTSTR k_FMT = _T ( "Happy Started Thread : 0x%08X\n" ) ;

DWORD WINAPI MyThread ( LPVOID )
{
    TCHAR szBuff [ 50 ] ;
    wsprintf ( szBuff , k_FMT , GetCurrentThreadId ( ) ) ;
    OutputDebugString ( szBuff ) ;
    BOOL bValue = TRUE ;
    ASSERT ( FALSE == bValue ) ;
    return ( 0 ) ;
}

DWORD WINAPI SleepThread ( LPVOID )
{
    Sleep ( 10000000 ) ;
    return ( 0 ) ;
}

void TestThree ( char * szAnsiString )
{
    DWORD dwTID = 0 ;
    HANDLE hSleepThread = CreateThread ( NULL        ,
                                         0           ,
                                         SleepThread ,
                                         0           ,
                                         0           ,
                                         &dwTID       ) ;
    szAnsiString = szAnsiString ;
    for ( int i = 0 ; i < 5 ; i++ )
    {
        SUPERASSERT ( i == 10  ) ;
        HANDLE hThread = CreateThread ( NULL , 0 , MyThread , (LPVOID)0xA , 0 , &dwTID ) ;
        WaitForSingleObject ( hThread , INFINITE ) ;
        VERIFY ( CloseHandle ( hThread ) ) ;
    }
    TerminateThread ( hSleepThread , 0 ) ;
    VERIFY ( CloseHandle ( hSleepThread ) ) ;
}

void TestTwo ( void )
{
    int aiArray[3][3] ;
    for ( int i = 0 ; i < 3 ; i++ )
    {
        for ( int j = 0 ; j < 3 ; j++ )
        {
            aiArray[i][j] = j ;
        }
    }
    ASSERT ( 0 ) ;
    TestThree ( "This is an ANSI String" ) ;
}

void TestOne ( TCHAR * szParam )
{
    HWND hDesk = GetDesktopWindow ( ) ;
    RECT rRect ;
    GetWindowRect ( hDesk , &rRect ) ;
    
    CONTEXT stCtx ;
    
    stCtx.ContextFlags = CONTEXT_FULL ;

    VERIFY ( GetThreadContext ( GetCurrentThread ( ) , &stCtx ) ) ;
    
    ASSERT ( hDesk == NULL ) ;
    
    szParam = szParam ;
    TestTwo ( ) ;
}

#ifdef SUPERASSERT_EMAIL
#undef SUPERASSERT_EMAIL
#define SUPERASSERT_EMAIL NULL
#endif

int WINAPI _tWinMain ( HINSTANCE , HINSTANCE , PSTR , int )
{
    ASSERT ( !"Do you see the cool message string?" ) ;
    ASSERT ( 1 == 2 && "The && test" ) ;
    ASSERT ( 1 == 1 && "If you see this, there's a bug in SUPERASSERT!" ) ;
    ASSERT ( 1 == 2 ) ;
    //SetDiagAssertOptions ( DA_SHOWMSGBO` ) ;
    InitCommonControls ( ) ;
    TestOne ( _T ( "Hello from UNICODE _tWinMain" ) ) ;
    TestThree ( "Another ANSI String!!!" ) ;
    return ( 0 ) ;
}

