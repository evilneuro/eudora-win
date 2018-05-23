/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "StdAfx.h"
#include "AssertTest.h"

int Baz ( int i )
{
    SUPERASSERT ( FALSE ) ;
    return ( i * 9 ) ;
}

int Bar ( int i )
{
    return ( Baz ( i * 5 ) ) ;
}

int Foo ( int i )
{
    return ( Bar ( i + 3 ) ) ;
}

void main ( void )
{
    int x = 0 ;
    x = x ;

    ADDDIAGASSERTMODULE ( GetModuleHandle ( NULL ) ) ;

    HANDLE hFile = CreateFile ( _T ( "AssertText.txt" ) ,
                                GENERIC_WRITE           ,
                                0                       ,
                                NULL                    ,
                                CREATE_ALWAYS           ,
                                FILE_ATTRIBUTE_NORMAL   ,
                                NULL                     ) ;

    SetDiagAssertFile ( hFile ) ;

    SetLastError ( MSG_DEOPPRESSOLIBER ) ;
    TRACE ( "Shows ODS and MB, no stack.\n" ) ;
    ASSERT ( x != 0 ) ;

    TRACE ( "Shows ODS, MB w/ stack.\n" ) ;
    SUPERASSERT ( x != 0 ) ;

    SetDiagAssertFile ( INVALID_HANDLE_VALUE ) ;

    Foo ( 2 ) ;

    SetDiagOutputFile ( hFile ) ;

    TRACE ( "Show just ODS, w/o stack.\n" ) ;
    SetDiagOutputFile ( INVALID_HANDLE_VALUE ) ;
    SETDIAGASSERTOPTIONS ( DA_SHOWODS ) ;
    ASSERT ( x != 0 ) ;

    TRACE ( "Just MB, w/o stack.\n" ) ;
    SETDIAGASSERTOPTIONS ( DA_SHOWMSGBOX ) ;
    ASSERT ( x != 0 ) ;

    TRACE ( "Show just ODS, w/ stack.\n" ) ;
    SETDIAGASSERTOPTIONS ( DA_SHOWODS | DA_SHOWSTACKTRACE ) ;
    ASSERT ( x != 0 ) ;

    TRACE ( "Just MB, w/ stack.\n" ) ;
    SETDIAGASSERTOPTIONS ( DA_SHOWMSGBOX | DA_SHOWSTACKTRACE ) ;
    ASSERT ( x != 0 ) ;

    TRACE ( "Just like SUPERASSERT.\n" ) ;
    SETDIAGASSERTOPTIONS ( DA_SHOWMSGBOX     |
                           DA_SHOWODS        |
                           DA_SHOWSTACKTRACE  ) ;
    ASSERT ( x != 0 ) ;

    TRACE ( "Out to the Event Viewer as well.\n" ) ;
    SETDIAGASSERTOPTIONS ( DA_SHOWMSGBOX     |
                           DA_SHOWODS        |
                           DA_SHOWSTACKTRACE |
                           DA_SHOWEVENTLOG    ) ;
    ASSERT ( x != 0 ) ;


    TRACE ( "Normal SUPERASSERT.\n" ) ;
    int i = 4 ;
    TRACE ( "Calling with with %d (0x%08X) (%c)\n" , i , i , i ) ;
    Foo ( i ) ;

    SETDIAGASSERTOPTIONS ( DA_SHOWMSGBOX     |
                           DA_SHOWODS        |
                           DA_SHOWSTACKTRACE  ) ;
    DiagAssertA ( DA_USEDEFAULTS , "x != 0" , __FILE__ , __LINE__ ) ;
    DiagOutputA ( "This is wide output!\n" ) ;
}
