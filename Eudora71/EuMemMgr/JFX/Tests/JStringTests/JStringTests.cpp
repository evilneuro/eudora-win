/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "JString.h"


void wmain ( void )
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF       |
                     _CRTDBG_CHECK_ALWAYS_DF    |
                     _CRTDBG_DELAY_FREE_MEM_DF  |
                     _CRTDBG_LEAK_CHECK_DF       ) ;

    // Check bug fixes (Bug 11) found by Chris Watts.
    JString cTenLen ( _T ( "0123456789" ) ) ;
    cTenLen = _T ( "012345678901234567890123456789012345" ) ;
    JString cThirtySixLen ( _T ( "012345678901234567890123456789012345" ) ) ;
    JString cTenLen2 ( _T ( "0123456789" ) ) ;
    cTenLen2 = cThirtySixLen ;
    JString cTenLen3 ( _T ( "0123456789" ) ) ;
    cTenLen3 = "012345678901234567890123456789012345" ;
    JString cTenLen4 ( _T ( "0123456789" ) ) ;
    cTenLen4 += "01234567890123456789012345" ;
    JString cTenLen5 ( _T ( "0123456789" ) ) ;
    cTenLen5 += _T ( "01234567890123456789012345" ) ;


    // The empty constructor.
    JString cCtorEmpty ;
    // Constructor passing in a string.
    JString cCtorFromString ( _T ( "This is a ctor string" ) ) ;
    // Construct a string that does not delete memory when finished.
    JString cCtorSkipDel ( true ) ;
    // Ctor JString from JString.
    JString cCtorJfromJ ( cCtorFromString ) ;
    
    // Test the == operators.
    if ( ! ( cCtorJfromJ == cCtorFromString ) )
    {
        _tprintf ( _T ( "operator==(JString,JString) failed!!\n" ) ) ;
    }
    if ( ! ( cCtorFromString == _T ( "This is a ctor string" ) ) )
    {
        _tprintf ( _T ( "operator==(JString,TCHAR*) failed!!\n" ) ) ;
    }
    if ( cCtorJfromJ == _T ( "not equal!" ) )
    {
        _tprintf ( _T ( "operator==(JString,TCHAR*) failed!!\n" ) ) ;
    }
    
    // Test the operator=(JString).
    JString cEmpty1 ;
    JString cAnotherEmpty1 ;
    JString cHasData1 ( _T ( "This string has data" ) ) ;
    JString cSmallData1 ( _T ( "012" ) ) ;
    JString cAnotherSmallData1 ( _T ( "xyz" ) ) ;
    JString cABigString ( _T ( "012345678901234567890123456789" ) ) ;
    
    cEmpty1 = cAnotherEmpty1 ;
    if ( 0 != cEmpty1.Length ( ) )
    {
        _tprintf ( _T ( "operator=(JString) failed!\n" ) ) ;
    }
    cEmpty1 = cHasData1 ;
    if ( cEmpty1 != _T ( "This string has data" ) )
    {
        _tprintf ( _T ( "operator=(JString) failed!\n" ) ) ;
    }
    cEmpty1 = cSmallData1 ;
    if ( cEmpty1 != _T ( "012" ) )
    {
        _tprintf ( _T ( "operator=(JString) failed!\n" ) ) ;
    }
    cSmallData1 = cHasData1 ;
    if ( cSmallData1 != _T ( "This string has data" ) )
    {
        _tprintf ( _T ( "operator=(JString) failed!\n" ) ) ;
    }
    cAnotherSmallData1 = cABigString ;
    if ( cAnotherSmallData1 != _T ( "012345678901234567890123456789" ) )
    {
        _tprintf ( _T ( "operator=(JString) failed!\n" ) ) ;
    }
    cAnotherSmallData1 = cAnotherEmpty1 ;
    if ( 0 != cAnotherSmallData1.Length ( ) )
    {
        _tprintf ( _T ( "operator=(JString) failed!\n" ) ) ;
    }

    // Test the =(TCHAR*) operator.
    JString cEmpty2 ;
    JString cAnotherEmpty2 ;
    JString cSmallData2 ( _T ( "012" ) ) ;
    JString cAnotherSmallData2 ( _T ( "xyz" ) ) ;
    
    cEmpty2 = _T ( "\0" ) ;
    if ( 0 != cEmpty2.Length ( ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cEmpty2 = _T ( "This string has data" ) ;
    if ( cEmpty2 != _T ( "This string has data" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cEmpty2 = _T ( "012" ) ;
    if ( cEmpty2 != _T ( "012" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cSmallData2 = _T ( "This string has data" ) ;
    if ( cSmallData2 != _T ( "This string has data" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cAnotherSmallData2 =_T ( "012345678901234567890123456789" ) ;
    if ( cAnotherSmallData2 != _T ( "012345678901234567890123456789" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cAnotherSmallData2 = _T ( "\0" ) ;
    if ( 0 != cAnotherSmallData2.Length ( ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    
    // Test the =(TCHAR) operator.
    JString cEmpty3 ;
    JString cSmallData3 ( _T ( "012" ) ) ;
    
    cEmpty3 = _T ( 'P' ) ;
    if ( 1 != cEmpty3.Length ( ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cSmallData3 = _T ( 'R' ) ;
    if ( 1 != cEmpty3.Length ( ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }

    // Test the =(char*) operator.
    JString cEmpty4 ;
    JString cAnotherEmpty4 ;
    JString cSmallData4 ( _T ( "014" ) ) ;
    JString cAnotherSmallData4 ( _T ( "xyz" ) ) ;
    
    cEmpty4 = "\0" ;
    if ( 0 != cEmpty4.Length ( ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cEmpty4 = "This string has data" ;
    if ( cEmpty4 != _T ( "This string has data" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cEmpty4 = "014" ;
    if ( cEmpty4 != _T ( "014" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cSmallData4 = "This string has data" ;
    if ( cSmallData4 != _T ( "This string has data" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cAnotherSmallData4 ="014345678901434567890143456789" ;
    if ( cAnotherSmallData4 != _T ( "014345678901434567890143456789" ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    cAnotherSmallData4 = "\0" ;
    if ( 0 != cAnotherSmallData4.Length ( ) )
    {
        _tprintf ( _T ( "operator=(TCHAR*) failed!\n" ) ) ;
    }
    
    // Check the Lengh function.
    JString cAnotherSmallData5 ( _T ( "xyz" ) ) ;
    if ( cAnotherSmallData5.Length ( ) != _tcslen ( _T ( "xyz" ) ) )
    {
        _tprintf ( _T ( "Length failed!!\n" ) ) ;
    }
    cAnotherSmallData5 =_T ( "012345678901234567890123456789" ) ;
    if ( cAnotherSmallData5.Length ( ) !=
            _tcslen ( _T ( "012345678901234567890123456789" ) ) )
    {
        _tprintf ( _T ( "Length failed!!\n" ) ) ;
    }
    
    // Check IsEmpty.
    JString cEmpty5 ;
    if ( false == cEmpty5.IsEmpty ( ) )
    {
        _tprintf ( _T ( "IsEmpty failed!!\n" ) ) ;
    }
    cEmpty5 = _T ( "Howdy!" ) ;
    if ( true == cEmpty5.IsEmpty ( ) )
    {
        _tprintf ( _T ( "IsEmpty failed!!\n" ) ) ;
    }

    JString cEmpty6 ;
    JString cAnotherSmallData6 ( _T ( "xyz" ) ) ;
    JString cABigString6 ( _T ( "012345678901234567890123456789" ) ) ;
    
    cEmpty6 = cAnotherSmallData6 + cABigString6 ;
    
    // Test operator+=(JString).
    JString cEmpty7 ;
    JString cAnotherEmpty7 ;
    JString cHasData7 ( _T ( "This string has data" ) ) ;
    JString cSmallData7 ( _T ( "072" ) ) ;
    JString cAnotherSmallData7 ( _T ( "xyz" ) ) ;
    JString cABigString7 ( _T ( "072345678907234567890723456789" ) ) ;
    
    cEmpty7 += cAnotherEmpty7 ;
    if ( 0 != cEmpty7.Length ( ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cEmpty7 += cHasData7 ;
    if ( ! ( cEmpty7 == _T ( "This string has data" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cEmpty7 += cSmallData7 ;
    if ( ! ( cEmpty7 == _T ( "This string has data072" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cSmallData7 += cHasData7 ;
    if ( ! ( cSmallData7 == _T ( "072This string has data" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cAnotherSmallData7 += cABigString7 ;
    if ( ! ( cAnotherSmallData7 ==
                _T ( "xyz072345678907234567890723456789" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cAnotherSmallData7 += cAnotherEmpty7 ;
    if ( ! ( cAnotherSmallData7 ==
                _T ( "xyz072345678907234567890723456789" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }


    // Test operator+=(TCHAR*).
    JString cEmpty8 ;
    JString cAnotherEmpty8 ;
    JString cHasData8 ( _T ( "This string has data" ) ) ;
    JString cSmallData8 ( _T ( "082" ) ) ;
    JString cAnotherSmallData8 ( _T ( "xyz" ) ) ;
    JString cABigString8 ( _T ( "082345688908234568890823456889" ) ) ;
    
    cEmpty8 += _T ( "\0" ) ;
    if ( 0 != cEmpty8.Length ( ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cEmpty8 += _T ( "This string has data" ) ;
    if ( ! ( cEmpty8 == _T ( "This string has data" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cEmpty8 += _T ( "This string has data082" ) ;
    if ( ! ( cEmpty8 == _T ( "This string has dataThis string has data082" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cSmallData8 += _T ( "082This string has data" ) ;
    if ( ! ( cSmallData8 == _T ( "082082This string has data" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cAnotherSmallData8 += _T ( "xyz082345688908234568890823456889" ) ;
    if ( ! ( cAnotherSmallData8 ==
                _T ( "xyzxyz082345688908234568890823456889" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    
    JString cEmpty9 ;
    
    cEmpty9 += _T ( 'P' ) ;
    if ( 1 != cEmpty9.Length ( ) )
    {
        _tprintf ( _T ( "operator+=(TCHAR) failed!\n" ) ) ;
    }
    cEmpty9 += _T ( 'a' ) ;
    cEmpty9 += _T ( 'm' ) ;
    if ( cEmpty9 != _T ( "Pam" ) )
    {
        _tprintf ( _T ( "operator+=(TCHAR) failed!\n" ) ) ;
    }
    cEmpty9 += _T ( "012345678901234567890" ) ;
    cEmpty9 += _T ( 'J' ) ;
    if ( cEmpty9 != _T( "Pam012345678901234567890J" ) )
    {
        _tprintf ( _T ( "operator+=(TCHAR) failed!\n" ) ) ;
    }
    
    
    // Test operator+=(TCHAR*).
    JString cEmpty10 ;
    JString cAnotherEmpty10 ;
    JString cHasData10 ( _T ( "This string has data" ) ) ;
    JString cSmallData10 ( _T ( "0102" ) ) ;
    JString cAnotherSmallData10 ( _T ( "xyz" ) ) ;
    JString cABigString10 ( _T ( "010234561010901023456101090102345610109" ) ) ;
    
    cEmpty10 += "\0" ;
    if ( 0 != cEmpty10.Length ( ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cEmpty10 += "This string has data" ;
    if ( ! ( cEmpty10 == _T ( "This string has data" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cEmpty10 += "This string has data0102" ;
    if ( ! ( cEmpty10 == _T ( "This string has dataThis string has data0102" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cSmallData10 += "0102This string has data" ;
    if ( ! ( cSmallData10 == _T ( "01020102This string has data" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    cAnotherSmallData10 += "xyz010234561010901023456101090102345610109" ;
    if ( ! ( cAnotherSmallData10 ==
                _T ( "xyzxyz010234561010901023456101090102345610109" ) ) )
    {
        _tprintf ( _T ( "operator+=(JString) failed!\n" ) ) ;
    }
    
    JString cEmpty11 ( _T ( "Hello There\t   " ) ) ;
    cEmpty11.TrimTrailingWhiteSpace ( ) ;
    cEmpty11 += _T ( "Wow!" ) ;
    if ( cEmpty11 != _T ( "Hello ThereWow!" ) )
    {
        _tprintf ( _T ( "TrimTrailingWhiteSpace failed!!" ) ) ;
    }
}

