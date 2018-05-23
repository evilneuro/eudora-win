/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"

DWORD64 CSymbolEngine :: SymLoadModule64 ( IN  HANDLE     hFile       ,
                                           IN  PWSTR      ImageName   ,
                                           IN  PWSTR      ModuleName  ,
                                           IN  DWORD64    BaseOfDll   ,
                                           IN  DWORD      SizeOfDll    )
{
    PSTR szAnsiImageName = NULL ;
    PSTR szAnsiModuleName = NULL ;
    
    int iLen = 0 ;
    
    if ( NULL != ImageName )
    {
        iLen = lstrlenW ( ImageName ) + 1 ;
        szAnsiImageName = (PSTR)HeapAlloc ( GetProcessHeap ( )        ,
                                            HEAP_GENERATE_EXCEPTIONS |
                                                HEAP_ZERO_MEMORY      ,
                                            iLen * sizeof ( char )    );

        WideCharToMultiByte ( CP_THREAD_ACP     ,
                              0                 ,
                              ImageName         ,
                              -1                ,
                              szAnsiImageName   ,
                              iLen              ,
                              NULL              ,
                              NULL               ) ;
    }
    if ( NULL != ModuleName )
    {
        iLen = lstrlenW ( ModuleName ) + 1 ;
        szAnsiModuleName =(PSTR)HeapAlloc ( GetProcessHeap ( )        ,
                                            HEAP_GENERATE_EXCEPTIONS |
                                                HEAP_ZERO_MEMORY      ,
                                            iLen * sizeof ( char )    );
        WideCharToMultiByte ( CP_THREAD_ACP     ,
                              0                 ,
                              ModuleName        ,
                              -1                ,
                              szAnsiModuleName  ,
                              iLen              ,
                              NULL              ,
                              NULL               ) ;
    }
    
    DWORD64 dwRet = SymLoadModule64 ( hFile             ,
                                      szAnsiImageName   ,
                                      szAnsiModuleName  ,
                                      BaseOfDll         ,
                                      SizeOfDll          ) ;

    if ( NULL != szAnsiImageName )
    {
        HeapFree ( GetProcessHeap ( ) , 0 , szAnsiImageName ) ;
    }
    if ( NULL != szAnsiModuleName )
    {
        HeapFree ( GetProcessHeap ( ) , 0 , szAnsiModuleName ) ;
    }
    
    return ( dwRet ) ;
}

DWORD UnDecorateSymbolNameW ( PCWSTR DecoratedName       ,
                              PWSTR  UnDecoratedName     ,
                              DWORD  UndecoratedLength   ,
                              DWORD  Flags                )
{
    size_t iDecNameLen = _tcslen ( DecoratedName ) + 1 ;
    PSTR szAnsiDecName =
                    (PSTR)HeapAlloc ( GetProcessHeap ( )            ,
                                      HEAP_GENERATE_EXCEPTIONS |
                                          HEAP_ZERO_MEMORY          ,
                                      iDecNameLen * sizeof ( char )  );
    WideCharToMultiByte ( CP_THREAD_ACP     ,
                          0                 ,
                          DecoratedName     ,
                          -1                ,
                          szAnsiDecName     ,
                          (int)iDecNameLen  ,
                          NULL              ,
                          NULL               ) ;
                          
    PSTR szAnsiUndec = (PSTR)HeapAlloc ( GetProcessHeap ( )         ,
                                         HEAP_GENERATE_EXCEPTIONS |
                                           HEAP_ZERO_MEMORY         ,
                                         UndecoratedLength           ) ;

    DWORD dwRet = UnDecorateSymbolName ( szAnsiDecName     ,
                                         szAnsiUndec       ,
                                         UndecoratedLength ,
                                         Flags              ) ;
                                         
    if ( 0 != dwRet )
    {
        MultiByteToWideChar ( CP_THREAD_ACP     ,
                              0                 ,
                              szAnsiUndec       ,
                              UndecoratedLength ,
                              UnDecoratedName   ,
                              UndecoratedLength  ) ;
    }
 
    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiDecName ) ;
    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiUndec ) ;
    
    return ( dwRet ) ;
}

BOOL CSymbolEngine :: SymGetSearchPath ( OUT LPWSTR SearchPath        ,
                                         IN  DWORD  SearchPathLength   )
{
    if ( 0 == SearchPathLength )
    {
        return ( FALSE ) ;
    }
    
    PSTR szAnsiPath =
                    (PSTR)HeapAlloc ( GetProcessHeap ( )            ,
                                      HEAP_GENERATE_EXCEPTIONS |
                                          HEAP_ZERO_MEMORY          ,
                                      ( SearchPathLength *
                                            sizeof ( char ) ) + 1    );
                                      
    BOOL bRet = ::SymGetSearchPath ( m_hProcess         ,
                                     szAnsiPath         ,
                                     SearchPathLength    ) ;
    if ( TRUE == bRet )
    {
        MultiByteToWideChar ( CP_THREAD_ACP     ,
                              0                 ,
                              szAnsiPath        ,
                              SearchPathLength  ,
                              SearchPath        ,
                              SearchPathLength   ) ;
    }
    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiPath ) ;
    return ( bRet ) ;
}
                        
BOOL CSymbolEngine :: SymSetSearchPath ( IN LPWSTR SearchPath )
{
    size_t iPathLen = _tcslen ( SearchPath ) + 1 ;
    PSTR szAnsiPath =
                    (PSTR)HeapAlloc ( GetProcessHeap ( )         ,
                                      HEAP_GENERATE_EXCEPTIONS |
                                          HEAP_ZERO_MEMORY       ,
                                      iPathLen * sizeof ( char )  ) ;
    WideCharToMultiByte ( CP_THREAD_ACP     ,
                          0                 ,
                          SearchPath        ,
                          -1                ,
                          szAnsiPath        ,
                          (int)iPathLen     ,
                          NULL              ,
                          NULL               ) ;
    
    BOOL bRet = ::SymSetSearchPath ( m_hProcess , szAnsiPath ) ;
    
    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiPath ) ;
    
    return ( bRet ) ;
}

BOOL CSymbolEngine :: SymAppendSearchPath ( IN LPCWSTR SearchPath )
{
    // Allocate something that should be large enough.
    PSTR szAnsiPath = (PSTR)HeapAlloc ( GetProcessHeap ( )         ,
                                        HEAP_GENERATE_EXCEPTIONS |
                                          HEAP_ZERO_MEMORY         ,
                                        4096                        ) ;
    
    BOOL bRet = ::SymGetSearchPath ( m_hProcess , szAnsiPath , 4096 ) ;
    
    if ( TRUE == bRet )
    {
        size_t iLen = strlen ( szAnsiPath ) ;
        
        PSTR szEnd = szAnsiPath + iLen - sizeof ( char ) ;
        if ( ';' != *szEnd )
        {
            szEnd++ ;
            // Slap on the semicolon!
            *szEnd = ';' ;
        }
        szEnd++ ;
        
        WideCharToMultiByte ( CP_THREAD_ACP             ,
                              0                         ,
                              SearchPath                ,
                              -1                        ,
                              szEnd                     ,
                              (int)(4096 - iLen - 1)    ,
                              NULL                      ,
                              NULL                       ) ;
                              
        bRet = ::SymSetSearchPath ( m_hProcess , szAnsiPath ) ;
    }

    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiPath ) ;

    return ( bRet ) ;
}

