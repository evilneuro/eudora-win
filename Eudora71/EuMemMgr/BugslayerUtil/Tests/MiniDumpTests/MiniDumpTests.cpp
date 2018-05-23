/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"

LONG WINAPI MyUnhandledExceptionFilter ( EXCEPTION_POINTERS *ExceptionInfo ) ;

void Baz ( void )
{
    BSUMDRET eCCPMD ;

    // Pass in a bad parameter.
    eCCPMD = SnapCurrentProcessMiniDump ( MiniDumpNormal      ,
                                         _T ( "CheckMe.DMP" )  ) ;
    ASSERT ( eDUMP_SUCCEEDED == eCCPMD ) ;
}

void Bar ( void )
{
    Baz ( ) ;
}

void Foo ( void )
{
    Bar ( ) ;
}

void main ( void )
{
    Foo ( ) ;

    BSUMDRET eCCPMD ;

    // Pass in a bad parameter.
    eCCPMD = SnapCurrentProcessMiniDumpA ( MiniDumpNormal ,
                                          (char*)0x1      ) ;
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    // Try to create a bad file name.
    eCCPMD = SnapCurrentProcessMiniDumpA ( MiniDumpNormal ,
                                          "*?!**??"        ) ;
    ASSERT ( eOPEN_DUMP_FAILED == eCCPMD ) ;

    // Create a good dump.
    eCCPMD = SnapCurrentProcessMiniDumpA ( MiniDumpNormal   ,
                                           "NormalDump.DMP"  ) ;

    ASSERT ( eDUMP_SUCCEEDED == eCCPMD ) ;

    // Pass in a bad wide parameter.
    eCCPMD = SnapCurrentProcessMiniDumpW ( MiniDumpNormal      ,
                                          (const wchar_t*)0x1   ) ;
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    // Try to create a bad wide file name.
    eCCPMD = SnapCurrentProcessMiniDumpW ( MiniDumpNormal ,
                                          L"*?!**??"       ) ;
    ASSERT ( eOPEN_DUMP_FAILED == eCCPMD ) ;

    // Create a good wide file.
    eCCPMD = SnapCurrentProcessMiniDumpW ( MiniDumpWithFullMemory ,
                                           L"FullDump.DMP"         ) ;

    ASSERT ( eDUMP_SUCCEEDED == eCCPMD ) ;

    // Pass in garbage in the exception pointers and a bad string.
    eCCPMD = CreateCurrentProcessCrashDumpA ( MiniDumpWithFullMemory   ,
                                              NULL                     ,
                                              0                        ,
                                             (EXCEPTION_POINTERS*)0x01 );
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    // Pass in garbage in the exception pointers.
    eCCPMD = CreateCurrentProcessCrashDumpW ( MiniDumpWithFullMemory   ,
                                              L"BadDump.DMP"           ,
                                              0                        ,
                                             (EXCEPTION_POINTERS*)0x01 );
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    // Do a real crash with a handler.
    SetUnhandledExceptionFilter ( MyUnhandledExceptionFilter ) ;

    __try
    {
        char * p = (char*)0x64 ;
        *p = 'J' ;
    }
    __except ( MyUnhandledExceptionFilter ( GetExceptionInformation ( ) ) )
    {
    }
}

LONG WINAPI MyUnhandledExceptionFilter ( EXCEPTION_POINTERS *ExceptionInfo )
{
    BSUMDRET eCCPMD ;
    eCCPMD = CreateCurrentProcessCrashDumpW ( MiniDumpWithHandleData ,
                                              L"ExceptDumpW.DMP"     ,
                                              GetCurrentThreadId ( ) ,
                                              ExceptionInfo           );
    ASSERT ( eDUMP_SUCCEEDED == eCCPMD ) ;

    eCCPMD = CreateCurrentProcessCrashDumpW ( MiniDumpWithHandleData ,
                                              L"ExceptDumpW.DMP"     ,
                                              0                      ,
                                              ExceptionInfo           );
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    eCCPMD = CreateCurrentProcessCrashDumpW ( MiniDumpWithHandleData ,
                                              L""                    ,
                                              GetCurrentThreadId ( ) ,
                                              ExceptionInfo           );
    ASSERT ( eOPEN_DUMP_FAILED == eCCPMD ) ;

    eCCPMD = CreateCurrentProcessCrashDumpW ( MiniDumpWithHandleData ,
                                              L"!**!**!*!*!"         ,
                                              GetCurrentThreadId ( ) ,
                                              ExceptionInfo           );
    ASSERT ( eOPEN_DUMP_FAILED == eCCPMD ) ;


    eCCPMD = CreateCurrentProcessCrashDumpW ( MiniDumpWithHandleData ,
                                              L"WIDE.DMP"            ,
                                              GetCurrentThreadId ( ) ,
                                              NULL                    );
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    eCCPMD = CreateCurrentProcessCrashDumpA ( MiniDumpWithHandleData ,
                                              "ExceptDumpA.DMP"      ,
                                              GetCurrentThreadId ( ) ,
                                              ExceptionInfo           );
    ASSERT ( eDUMP_SUCCEEDED == eCCPMD ) ;
    
    eCCPMD = CreateCurrentProcessCrashDumpA ( MiniDumpWithHandleData ,
                                              "ExceptDumpW.DMP"      ,
                                              0                      ,
                                              ExceptionInfo           );
    ASSERT ( eBAD_PARAM == eCCPMD ) ;

    eCCPMD = CreateCurrentProcessCrashDumpA ( MiniDumpWithHandleData ,
                                              ""                     ,
                                              GetCurrentThreadId ( ) ,
                                              ExceptionInfo           );
    ASSERT ( eOPEN_DUMP_FAILED == eCCPMD ) ;

    eCCPMD = CreateCurrentProcessCrashDumpA ( MiniDumpWithHandleData ,
                                               "!**!**!*!*!"         ,
                                              GetCurrentThreadId ( ) ,
                                              ExceptionInfo           );
    ASSERT ( eOPEN_DUMP_FAILED == eCCPMD ) ;


    eCCPMD = CreateCurrentProcessCrashDumpA ( MiniDumpWithHandleData ,
                                              "AnsiDump.DMP"         ,
                                              GetCurrentThreadId ( ) ,
                                              NULL                    );
    ASSERT ( eBAD_PARAM == eCCPMD ) ;
    

    return ( EXCEPTION_EXECUTE_HANDLER ) ;
}
