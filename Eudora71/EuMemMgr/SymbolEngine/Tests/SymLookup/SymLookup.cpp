/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"

HANDLE g_hPopFromThread = NULL ;
HANDLE g_hWaitInThread = NULL ;

typedef struct tag_EXAMPLESTRUCT 
{
    int i ;
    int j ;
} EXAMPLESTRUCT , * LPEXAMPLESTRUCT ;

typedef struct tag_OUTERSTUCT 
{
    char * szTitle ;
    EXAMPLESTRUCT stEX ;
} OUTERSTRUCT ;

enum HappyEnum
{
    ePorsche = 0x911 ,
    eFerrari = 0x360 ,
    eMercedes = 0x430  
} ;

class TestClass
{
public :
    TestClass ( )
    {
        m_pPrivateVar = L"Fully initialized" ;
    }
    
public  :
    char * m_pPublicVar ;    
    
protected :
    wchar_t * m_pPrivateVar ;
} ;

class DerivedClass : public TestClass
{
public  :
    DerivedClass ( ) : TestClass ( )
    {
        m_pPrivateVar = L"DerivedClass ctor" ;
    }
    
    wchar_t * m_pDevMember ;
} ;

typedef struct tag_ScreenElem
{
    unsigned short icon : 8;
    unsigned short color : 4;
    unsigned short underline : 1;
    unsigned short blink : 1;
} ScreenElem ;

typedef struct tag_MYSTRUCT
{
    int    iValue   ;
    int *  pData    ;
    char * szString ;
} MYSTRUCT , * LPMYSTRUCT ;


void SlapSomethingOnTheStackToWorkAroundStackWalkBugs ( void )
{
    SetEvent ( g_hPopFromThread ) ;
    WaitForSingleObject ( g_hWaitInThread , INFINITE ) ;
}

#pragma warning ( disable : 4312 ) 
int Foo ( int iFooParam )
{
    _int64 i64BitInt ;
    i64BitInt = 0xFEEDBEEFBAADF00D ;
    
    char cNullChar ;
    cNullChar = '\0' ;
    
    wchar_t cNullWCharT ;
    cNullWCharT = L'\0' ;
    
    ScreenElem stSE ;
    stSE.icon = 0x77 ;
    stSE.color = 0x8 ;
    stSE.underline = 0 ;
    stSE.blink = 1 ;
    
    int * iIntPtr ;
    iIntPtr = &iFooParam ;
    
    IUnknown * pUnk ;
    pUnk = (IUnknown*)(0xDEADBEEF) ;
    
    IDispatch * pDisp ;
    pDisp = (IDispatch*)(0xDEADBEEF) ;

    DerivedClass stDevCls ;
    stDevCls.m_pPublicVar = "Init" ;
    stDevCls.m_pDevMember = L"Derived only!" ;
    
    DerivedClass * pClsDev ;
    pClsDev = &stDevCls ;

    OUTERSTRUCT stOuter ;
    stOuter.szTitle = "Outer struct title" ;
    stOuter.stEX.i = 0x430 ;
    stOuter.stEX.j = stOuter.stEX.i ;

    wchar_t * wszFooStr ;
    wszFooStr = L"A long string!" ;

    char * szBarStr ;
    szBarStr = "An ANSI string" ;
    
    bool bBool ;
    bBool = true ;
    
    HRESULT hr ;
    hr = E_NOINTERFACE ;
    
    BSTR bstrData ;
    bstrData = ::SysAllocString ( L"Ye ol' BSTR String!" ) ;
    
    VARIANT vtVariant ;
    vtVariant.vt = VT_UINT ;
    vtVariant.uiVal = 0x69 ;

    enum HappyEnum eGoodEnum ;
    eGoodEnum = eMercedes ;
    
    enum HappyEnum eBadEnum ;
    
    eBadEnum = (enum HappyEnum)0x222 ;
    
    TestClass cClass ;
    cClass.m_pPublicVar = 0 ;
    
    PENUM_LOCAL_VARS_CALLBACK pPfn ;
    pPfn = NULL ;

    EXAMPLESTRUCT stES ;
    stES.i =  0x360 ;
    stES.j = stES.i ;
    
    LPEXAMPLESTRUCT pstES ;
    pstES = &stES ;

    short iShort ;
    iShort = 0x77 ;
    
    unsigned short iUShort ;
    iUShort = 0xFF ;

    int iLocal ;
    iLocal = 9 ;
    
    unsigned int iUnInt ;
    iUnInt = 0xEEEE ;
    
    long lLong ;
    lLong = 0xEEEEEEEE ;
    
    unsigned long lUnsignedLongLocal ;
    lUnsignedLongLocal = 0xEEEEEEEE ;

    float fFloat ;
    fFloat = (float)3.14 ;
    
    double dDouble ;
    dDouble = 0.00005 ;

    int aiArray[ 5 ] ;
    aiArray[ 0 ] = 0x11 ;
    aiArray[ 1 ] = 0x12 ;
    aiArray[ 2 ] = 0x13 ;
    aiArray[ 3 ] = 0x14 ;
    aiArray[ 4 ] = 0x15 ;

    DWORD adwTemp[ 3 ][ 3 ] ;
    adwTemp[0][0] = 0xA ;
    adwTemp[0][1] = 0xB ;
    adwTemp[0][2] = 0xC ;
    adwTemp[1][0] = 0xD ;
    adwTemp[1][1] = 0xE ;
    adwTemp[1][2] = 0xF ;
    adwTemp[2][0] = 0x10 ;
    adwTemp[2][1] = 0x11 ;
    adwTemp[2][2] = 0x12 ;
    
    OUTERSTRUCT astOS[ 3 ] ;
    astOS[0].szTitle = "Slot 0" ;
    astOS[0].stEX.i  = 0 ;
    astOS[0].stEX.j  = 0 ;
    astOS[1].szTitle = "Slot 1" ;
    astOS[1].stEX.i  = 1 ;
    astOS[1].stEX.j  = 1 ;
    astOS[2].szTitle = "Slot 2" ;
    astOS[2].stEX.i  = 2 ;
    astOS[2].stEX.j  = 2 ;
    
    char szAnsiBuff[ 20 ] ;
    strcpy ( szAnsiBuff , "A big ANSI hello!" ) ;
    
    wchar_t szUniBuff[ 30 ] ;
    _tcscpy ( szUniBuff , L"A big UNICODE hello!" ) ;

    MYSTRUCT stMyStruct ;
    
    stMyStruct.iValue = 0x77 ;
    stMyStruct.pData = &iFooParam ;
    stMyStruct.szString = szAnsiBuff ;
    
    LPMYSTRUCT pMyStruct ;
    
    pMyStruct = &stMyStruct ;

    SlapSomethingOnTheStackToWorkAroundStackWalkBugs ( ) ;        
    //SetEvent ( g_hPopFromThread ) ;
    //WaitForSingleObject ( g_hWaitInThread , INFINITE ) ;
    return ( iFooParam ) ;
}

int Bar ( int iBarParam )
{
    int iBarLocal = 82 ;
    iBarLocal = Foo ( 20 ) + iBarParam ;
    return ( iBarLocal ) ;
}

int Baz ( int iBazParam )
{
    int iBazLocal = 66 ;
    return ( Bar ( iBazLocal ) + iBazParam ) ;
}

unsigned __stdcall TheThread ( void * )
{
    char   chStackLocal ;
    char * pTheThreadLocal = &chStackLocal ;
    
    *pTheThreadLocal = 'p' ;
    return ( (unsigned) Baz ( 911 ) ) ;
}


BOOL CALLBACK LocalsEnum ( DWORD64  dwAddr        ,
                           LPCTSTR  szType        ,
                           LPCTSTR  szName        ,
                           LPCTSTR  szValue       ,
                           int      iIndentLevel  ,
                           PVOID                  ) 
{
    if ( 0 == iIndentLevel )
    {
        _tprintf ( _T ( "\n" ) ) ;
    }
    for ( int i = 0 ; i < iIndentLevel + 1 ; i++ )
    {
        _tprintf ( _T ( "  " ) ) ;
    }
    if ( NULL != szType )
    {
        _tprintf ( _T ( "%s " ) , szType ) ;
    }
    if ( NULL != szName )
    {
        _tprintf ( _T ( "%s " ) , szName ) ;
    }        
    
    if ( 0 != dwAddr )
    {
        _tprintf ( _T ( "%(0x%016I64X) " ) , dwAddr ) ;
    }
    
    if ( ( NULL != szValue ) && ( _T ( '\0' ) != szValue[0] ) )
    {
        _tprintf ( _T ( "= %s" ) , szValue ) ;
    }
    _tprintf ( _T ( "\n" ) ) ;
    return ( TRUE ) ;               
}                                   

BOOL CALLBACK SymEnumerateModulesProc64 ( PSTR    ModuleName    , 
                                          DWORD64 BaseOfDll     ,  
                                          PVOID                  ) 
{
    _tprintf ( _T ( "Module 0x%016I64X : %-20S " ) , BaseOfDll , ModuleName ) ;
    IMAGEHLP_MODULE64 stIHM ;
    ZeroMemory ( &stIHM , sizeof ( IMAGEHLP_MODULE64 ) ) ;
    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE64 ) ;

    ::SymGetModuleInfo64 ( GetCurrentProcess ( ) , BaseOfDll , &stIHM ) ;
    _tprintf ( _T ( "SymType : %d\n\t\tSymFile : %S\n" ) , 
               stIHM.SymType                        ,
               stIHM.LoadedImageName                 ) ;
    
    return ( TRUE ) ;
}                                          

void _tmain ( int argc, _TCHAR* argv[] )
{
#if 0
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF       |
                     _CRTDBG_CHECK_ALWAYS_DF    |
                     _CRTDBG_DELAY_FREE_MEM_DF  |
                     _CRTDBG_LEAK_CHECK_DF       ) ;
#endif

	argc = argc ;
	argv = argv ;
	int ** ppInt ;
    void * pVoid ;	
    void ** ppVoid ;
	wchar_t *** ppChar = &argv ;
	char * pString = "FooFOo" ;
	int iInt = 0x666 ;
	int * pInt = &iInt ;
	
	pInt = pInt ;
	
	pString = pString ;
	
	ppChar = ppChar ;
	
	ppInt = NULL ;
	
	pVoid = NULL ;
	ppVoid = &pVoid ;
	
	HANDLE hDup ;
	if ( FALSE == DuplicateHandle ( GetCurrentProcess ( ) ,
	                                GetCurrentProcess ( ) ,
	                                GetCurrentProcess ( ) ,
	                                &hDup                 ,
	                                0                     ,
	                                FALSE                 ,
	                                DUPLICATE_SAME_ACCESS  ) )
    {
    }	                                
	
	CSymbolEngine cSym ;

	cSym.SymSetOptions ( cSym.SymGetOptions ( )   |
	                     SYMOPT_LOAD_LINES         ) ;
	
	cSym.SymInitialize ( GetCurrentProcess ( ) , NULL , TRUE ) ;

	cSym.SymSetOptions ( cSym.SymGetOptions ( )   |
	                     SYMOPT_LOAD_LINES          ) ;
	
    g_hPopFromThread = CreateEvent ( NULL , TRUE , FALSE , L"PopFromThread" ) ;
    g_hWaitInThread  = CreateEvent ( NULL , TRUE , FALSE , L"WaitInThread" ) ;
    
    unsigned dwTID ;
    HANDLE hThread = (HANDLE)_beginthreadex ( NULL     ,
                                              0         ,
                                              TheThread ,
                                              NULL      ,
                                              0         ,
                                              &dwTID     ) ;
                                     
    WaitForSingleObject ( g_hPopFromThread , INFINITE ) ; 
    
    
    cSym.SymEnumerateModules ( SymEnumerateModulesProc64 , NULL ) ;
    
    _tprintf ( _T ( "\n\n" ) ) ;
    
    STACKFRAME64 stFrame ;
    DWORD      dwMachine ;
    dwMachine                = IMAGE_FILE_MACHINE_I386 ;

    ZeroMemory ( &stFrame , sizeof ( STACKFRAME64 ) ) ;
    stFrame.AddrPC.Mode = AddrModeFlat ;
    stFrame.AddrStack.Mode   = AddrModeFlat ;
    stFrame.AddrFrame.Mode   = AddrModeFlat ;

    // Grovel the context for the thread.
    CONTEXT stCtx ;

    stCtx.ContextFlags = CONTEXT_FULL ;

    if ( FALSE == GetThreadContext ( hThread , &stCtx ) )
    {
        _tprintf ( _T ( "GetThreadContext failed (%d)!!\n" ) ,
                   GetLastError ( )                           ) ;
        return ;
    }

    stFrame.AddrPC.Offset    = stCtx.Eip ;
    stFrame.AddrStack.Offset = stCtx.Esp ;
    stFrame.AddrFrame.Offset = stCtx.Ebp ;
    
    TCHAR * szSym = new TCHAR [ 1024 ] ;
    PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)szSym ;
    
    pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 ) ;
    pSym->MaxNameLength = 1024 - sizeof ( IMAGEHLP_SYMBOL64 ) ;
    DWORD64 dwDisp ;

    _tprintf ( _T ( "EIP                EBP                RetAddr            Name\n" ));
    // Loop for the first 512 stack elements.
    for ( DWORD i = 0 ; i < 512 ; i++ )
    {
        if ( FALSE == StackWalk64 ( dwMachine                ,
                                    GetCurrentProcess ( )    ,
                                    hThread                  ,
                                    &stFrame                 ,
                                    &stCtx                   ,
                                    NULL                     ,
                                    SymFunctionTableAccess64 ,
                                    SymGetModuleBase64       ,
                                    NULL                      ) )
        {
            break ;
        }
        // Also check that the address is not zero.  Sometimes
        // StackWalk returns TRUE with a frame of zero.
        if ( 0 != stFrame.AddrPC.Offset )
        {
            if ( FALSE == cSym.SymGetSymFromAddr ( stFrame.AddrPC.Offset ,
                                                   &dwDisp               ,
                                                   pSym                   ) )
            {
                pSym->Name[0] = _T ( '\0' ) ;
            }                                                      
            _tprintf ( _T ( "0x%016I64X 0x%016I64X 0x%016I64X %S\n" ) , 
                      stFrame.AddrPC.Offset                           ,
                      stFrame.AddrFrame.Offset                        ,
                      stFrame.AddrReturn.Offset                       ,
                      pSym->Name                                       ) ;
            if ( cSym.EnumLocalVariables ( LocalsEnum                ,
                                           3                         ,
                                           TRUE , //FALSE                     ,
                                           NULL                      ,
                                           &stFrame                  ,
                                           &stCtx                    ,
                                           NULL                       ) )
            {
                _tprintf ( _T ( "\n" ) ) ;
            }                                           
        }
        else
        {
            _tprintf ( _T ( "stFrame.AddrPC.Offset == 0!\n" ) ) ;
        }
    }
    SetEvent ( g_hWaitInThread ) ;
    
    CloseHandle ( hThread ) ;
    
    CloseHandle ( hDup ) ;
    
    delete [] szSym ;

    //cSym.EnumLocalVariables ( 0x004012B0 , NULL , NULL , NULL , NULL ) ;
    	                               
    //cSym.SymUnloadModule64 ( dwVal ) ;	                               
	
}

