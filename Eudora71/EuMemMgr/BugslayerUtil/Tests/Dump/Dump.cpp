/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include "BugslayerUtil.h"

#pragma push_macro ( "new" )
#ifdef new
#undef new
#endif
    
class TestClass
{
public:
    TestClass ( void )
    {
        _tcscpy ( m_szData , _T ( "TestClass constructor data!" ) ) ;
    }
    ~TestClass ( void )
    {
        m_szData[ 0 ] = _T ( '\0' ) ;
    }

    // The declaration of the memory debugging stuff for C++ classes
    DECLARE_MEMDEBUG ( TestClass ) ;
private     :
    TCHAR m_szData[ 100 ] ;

} ;

#pragma pop_macro ( "new" )

// This macro sets up the static BSMDVINFO structure.
IMPLEMENT_MEMDEBUG ( TestClass ) ;

// The methods you must implement to dump and
// validate
#ifdef _DEBUG
void TestClass::ClassDumper ( const void * pData )
{
    TestClass * pClass = (TestClass*)pData ;
    _RPT1 ( _CRT_WARN                         ,
            " TestClass::ClassDumper : %S\n"  ,
            pClass->m_szData                   ) ;
}
void TestClass::ClassValidator ( const void * pData   ,
                                 const void *          )
{
    // Validate the data here.
    TestClass * pClass = (TestClass*)pData ;
    _RPT1 ( _CRT_WARN                           ,
            " TestClass::ClassValidator : %S\n" ,
            pClass->m_szData                     ) ;
}
#endif

typedef struct tag_SimpleStruct
{
    TCHAR szName[ 256 ] ;
    TCHAR szRank[ 256 ] ;
} SimpleStruct ;

// The dumper and validator for simple string data memory
void DumperOne ( const void * pData )
{
    _RPT1 ( _CRT_WARN , " Data is : %S\n" , pData ) ;
}

void ValidatorOne ( const void * pData , const void * pContext )
{
    // Validate the string data here.
    _RPT2 ( _CRT_WARN                                ,
            " Validator called with : %s : 0x%08X\n" ,
            pData                                    ,
            pContext                                  ) ;
}

// The dumper and validator for the structure allocations
void DumperTwo ( const void * pData )
{
    _RPT2 ( _CRT_WARN                        ,
            " Data is Name : %S\n"
            "         Rank : %S\n"           ,
            ((SimpleStruct*)pData)->szName  ,
            ((SimpleStruct*)pData)->szRank   ) ;
}

void ValidatorTwo ( const void * pData , const void * /*pContext*/ )
{
    // Validate any structures here.
    _RPT2 ( _CRT_WARN                       ,
            "  Validator called with :\n"
            "    Data is Name : %s\n"
            "            Rank : %s\n"       ,
            ((SimpleStruct*)pData)->szName  ,
            ((SimpleStruct*)pData)->szRank   ) ;
}

// Unfortunately, the C functions need to drag around their own
// BSMDVINFO structures. In the real world, you'd define these
// structures as extern references and wrap the MEMDEBUG macros with
// your own macros.
static BSMDVINFO g_dvOne ;
static BSMDVINFO g_dvTwo ;

void main ( void )
{
    _tprintf ( _T ( "At start of main\n" ) ) ;

    // The memory debugging initialization for type one.
    INITIALIZE_MEMDEBUG ( &g_dvOne , DumperOne , ValidatorOne )  ;
    // The memory debugging initialization for type two.
    INITIALIZE_MEMDEBUG ( &g_dvTwo , DumperTwo , ValidatorTwo )  ;

    // Allocate the class with the MEMDEBUG new.
    TestClass * pstClass ;
    pstClass = new TestClass ( ) ;

    // Allocate the two C types.
    TCHAR * p = (TCHAR*)MEMDEBUG_MALLOC ( &g_dvOne , 20 ) ;
    _tcscpy ( p , _T ( "VC VC" ) ) ;

    SimpleStruct * pSt =
            (SimpleStruct*)MEMDEBUG_MALLOC ( &g_dvTwo ,
                                             sizeof ( SimpleStruct ) ) ;

    _tcscpy ( pSt->szName , _T ( "Pam" ) ) ;
    _tcscpy ( pSt->szRank , _T ( "CINC" ) ) ;

    // Validate all the blocks in the list.
    VALIDATEALLBLOCKS ( NULL ) ;

    _tprintf ( _T ( "At end of main\n" ) ) ;

    // Every block will get dumped as part of the memory leak checking.

}

