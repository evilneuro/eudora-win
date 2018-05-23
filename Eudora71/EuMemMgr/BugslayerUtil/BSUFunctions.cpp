/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#include "PCH.h"
#include "BugslayerUtil.h"
#include "Internal.h"

/*//////////////////////////////////////////////////////////////////////
                        Function Implementation
//////////////////////////////////////////////////////////////////////*/
DWORD __stdcall BSUGetModuleBaseNameA ( HANDLE  hProcess   ,
                                        HMODULE hModule    ,
                                        LPSTR   lpBaseName ,
                                        DWORD   nSize       )
{
    wchar_t * pWideName = (wchar_t*)HeapAlloc ( GetProcessHeap ( )      ,
                                                HEAP_GENERATE_EXCEPTIONS|
                                                 HEAP_ZERO_MEMORY       ,
                                                nSize * sizeof(wchar_t));
    DWORD dwRet = BSUGetModuleBaseNameW ( hProcess      ,
                                          hModule       ,
                                          pWideName     ,
                                          nSize          ) ;
    if ( 0 != dwRet )
    {
        if ( FALSE == BSUWide2Ansi ( pWideName , lpBaseName , nSize ) )
        {
            dwRet = 0 ;
        }
    }
    VERIFY ( HeapFree ( GetProcessHeap ( ) , 0 , pWideName ) ) ;
    return ( dwRet ) ;
}

DWORD __stdcall BSUGetModuleBaseNameW ( HANDLE  hProcess   ,
                                        HMODULE hModule    ,
                                        LPWSTR lpBaseName  ,
                                        DWORD   nSize       )
{
    // Call the NT version.  It is in NT4ProcessInfo because that is
    // where all the PSAPI wrappers are kept.
    return ( NTGetModuleBaseNameW ( hProcess     ,
                                    hModule      ,
                                    lpBaseName   ,
                                    nSize         ) ) ;
}

DWORD BUGSUTIL_DLLINTERFACE __stdcall
        BSUSymInitializeW ( DWORD     dwPID          ,
                            HANDLE    hProcess       ,
                            wchar_t * UserSearchPath ,
                            BOOL      fInvadeProcess  )
{
    // Symbol engine functions are NOT UNICODE aware.  Bummer.
    char * pAnsi = NULL ;
    int iLenPath = 0 ;

    if ( NULL != UserSearchPath )
    {
        iLenPath = lstrlenW ( UserSearchPath ) ;
        pAnsi = (char*)HeapAlloc ( GetProcessHeap ( )          ,
                                   HEAP_GENERATE_EXCEPTIONS |
                                         HEAP_ZERO_MEMORY      ,
                                   iLenPath + 1                 ) ;
        if ( NULL == pAnsi )
        {
            return ( FALSE ) ;
        }

        pAnsi[ 0 ] = '\0' ;
        if ( FALSE == BSUWide2Ansi ( UserSearchPath ,
                                     pAnsi          ,
                                     iLenPath        ) )
        {
            HeapFree ( GetProcessHeap ( ) , 0 , pAnsi ) ;
            SetLastError ( ERROR_INVALID_PARAMETER ) ;
            return ( FALSE ) ;
        }
    }

    DWORD dwRet = BSUSymInitializeA ( dwPID          ,
                                      hProcess       ,
                                      pAnsi          ,
                                      fInvadeProcess  ) ;

    DWORD dwLastErr = GetLastError ( ) ;

    if ( NULL != pAnsi )
    {
        HeapFree ( GetProcessHeap ( ) , 0 , pAnsi ) ;
    }

    SetLastError ( dwLastErr ) ;

    return ( dwRet ) ;
}

DWORD BUGSUTIL_DLLINTERFACE __stdcall
        BSUSymInitializeA ( DWORD  dwPID           ,
                            HANDLE  hProcess       ,
                            char *  UserSearchPath ,
                            BOOL    fInvadeProcess  )
{
    // If this is any flavor of NT or fInvadeProcess is FALSE, just call
    // SymInitialize itself
    if ( ( TRUE == IsNT ( ) ) || ( FALSE == fInvadeProcess ) )
    {
        return ( ::SymInitialize ( hProcess       ,
                                   UserSearchPath ,
                                   fInvadeProcess  ) ) ;
    }
    else
    {
        // This is Win9x and the user wants to invade!

        // The first step is to initialize the symbol engine.  If it
        // fails, there is not much I can do.
        BOOL bSymInit = ::SymInitialize ( hProcess       ,
                                          UserSearchPath ,
                                          fInvadeProcess  ) ;
        ASSERT ( FALSE != bSymInit ) ;
        if ( FALSE == bSymInit )
        {
            return ( FALSE ) ;
        }

        DWORD dwCount ;
        // Find out how many modules there are.  This is a BSU function.
        if ( FALSE == GetLoadedModules ( dwPID    ,
                                         0        ,
                                         NULL     ,
                                         &dwCount  ) )
        {
            ASSERT ( !"GetLoadedModules failed" ) ;
            // Clean up the symbol engine and leave.
            VERIFY ( ::SymCleanup ( hProcess ) ) ;
            return ( FALSE ) ;
        }
        // Allocate something big enough to hold the list.
        HMODULE * paMods = (HMODULE*)
                           HeapAlloc ( GetProcessHeap ( )        ,
                                       HEAP_GENERATE_EXCEPTIONS |
                                           HEAP_ZERO_MEMORY      ,
                                       dwCount * sizeof ( HMODULE ) ) ;
                                             

        // Get the list for real.
        if ( FALSE == GetLoadedModules ( dwPID    ,
                                         dwCount  ,
                                         paMods   ,
                                         &dwCount  ) )
        {
            ASSERT ( !"GetLoadedModules failed" ) ;
            // Clean up the symbol engine and leave.
            VERIFY ( ::SymCleanup ( hProcess ) ) ;
            // Free the memory that I allocated earlier.
            HeapFree ( GetProcessHeap ( ) , 0 , paMods ) ;
            return ( FALSE ) ;
        }
        // The module filename.
        char szModName [ MAX_PATH ] ;
        for ( UINT uiCurr = 0 ; uiCurr < dwCount ; uiCurr++ )
        {
            // Get the module's filename.
            if ( FALSE == GetModuleFileNameA ( paMods[ uiCurr ]     ,
                                               szModName            ,
                                               sizeof ( szModName )  ) )
            {
                ASSERT ( !"GetModuleFileName failed!" ) ;
                // Clean up the symbol engine and leave.
                VERIFY ( ::SymCleanup ( hProcess ) ) ;
                // Free the memory that I allocated earlier.
                HeapFree ( GetProcessHeap ( ) , 0 , paMods ) ;
                return ( FALSE ) ;
            }

            // In order to get the symbol engine to work outside a
            // debugger, it needs a handle to the image.  Yes, this
            // will leak but the OS will close it down when the process
            // ends.
            HANDLE hFile = CreateFileA ( szModName       ,
                                         GENERIC_READ    ,
                                         FILE_SHARE_READ ,
                                         NULL            ,
                                         OPEN_EXISTING   ,
                                         0               ,
                                         0                ) ;

            // For whatever reason, SymLoadModule can return zero, but
            // it still loads the modules.  Sheez.
            if ( FALSE == SymLoadModule ( hProcess                  ,
                                          hFile                     ,
                                          szModName                 ,
                                          NULL                      ,
                                          (DWORD)
                                           ((DWORD_PTR)paMods[ uiCurr ]) ,
                                          0                          ))
            {
                // Check the last error value.  If it is zero, then all
                // I can assume is that it worked.
                DWORD dwLastErr = GetLastError ( ) ;
                ASSERT ( ERROR_SUCCESS == dwLastErr ) ;
                if ( ERROR_SUCCESS != dwLastErr )
                {
                    // Clean up the symbol engine and leave.
                    VERIFY ( ::SymCleanup ( hProcess ) ) ;
                    // Free the memory that I allocated earlier.
                    HeapFree ( GetProcessHeap ( ) , 0 , paMods ) ;
                    return ( FALSE ) ;
                }
            }
        }
        HeapFree ( GetProcessHeap ( ) , 0 , paMods ) ;
    }
    return ( TRUE ) ;
}

DWORD __stdcall BSUGetModuleFileNameExA ( DWORD     dwPID        ,
                                          HANDLE    hProcess     ,
                                          HMODULE   hModule      ,
                                          LPSTR     szFilename   ,
                                          DWORD     nSize         )
{
    // Allocate the wide character buffer.
    wchar_t * szWide =
            (wchar_t*)HeapAlloc ( GetProcessHeap ( )           ,
                                  HEAP_GENERATE_EXCEPTIONS |
                                        HEAP_ZERO_MEMORY        ,
                                  ( nSize * sizeof ( wchar_t ) ) ) ;
    if ( NULL == szWide )
    {
        return ( 0 ) ;
    }

    DWORD dwRet = BSUGetModuleFileNameExW ( dwPID       ,
                                            hProcess    ,
                                            hModule     ,
                                            szWide      ,
                                            nSize        ) ;

    if ( 0 != dwRet )
    {
        // Convert the string.
        if ( 0 == BSUWide2Ansi ( szWide , szFilename , nSize ) )
        {
            return ( FALSE ) ;
        }
    }
    HeapFree ( GetProcessHeap ( ) , 0 , szWide ) ;
    return ( dwRet ) ;
}

DWORD __stdcall BSUGetModuleFileNameExW ( DWORD     dwPID        ,
                                          HANDLE    hProcess     ,
                                          HMODULE   hModule      ,
                                          LPWSTR    szFilename   ,
                                          DWORD     nSize         )
{
    if ( TRUE == IsNT4 ( ) )
    {
        return ( NTGetModuleFileNameEx ( dwPID      ,
                                         hProcess   ,
                                         hModule    ,
                                         szFilename ,
                                         nSize       ) ) ;
    }
    return ( TLHELPGetModuleFileNameEx ( dwPID      ,
                                         hProcess   ,
                                         hModule    ,
                                         szFilename ,
                                         nSize       ) ) ;
}

// Helper define to let code compile on pre W2K systems.
#if _WIN32 >= 0x500
#define GET_THREAD_ACP() CP_THREAD_ACP
#else
#define GET_THREAD_ACP() GetACP ( )
#endif

DWORD BUGSUTIL_DLLINTERFACE __stdcall
                                BSUWide2Ansi ( const wchar_t * szWide  ,
                                               char *          szANSI  ,
                                               int             iANSILen)
{
    ASSERT ( NULL != szWide ) ;
    ASSERT ( NULL != szANSI ) ;
    ASSERT ( FALSE == IsBadStringPtrW ( szWide , MAX_PATH ) ) ;

    int iRet = WideCharToMultiByte ( GET_THREAD_ACP() ,
                                     0                ,
                                     szWide           ,
                                     -1               ,
                                     szANSI           ,
                                     iANSILen         ,
                                     NULL             ,
                                     NULL              ) ;
    return ( iRet ) ;
}

DWORD BUGSUTIL_DLLINTERFACE __stdcall
                                BSUAnsi2Wide ( const char * szANSI   ,
                                               wchar_t *    szWide   ,
                                               int          iWideLen  )
{
    ASSERT ( NULL != szWide ) ;
    ASSERT ( NULL != szANSI ) ;
    ASSERT ( FALSE == IsBadStringPtrA ( szANSI , MAX_PATH ) ) ;

    int iRet = MultiByteToWideChar ( GET_THREAD_ACP ( ) ,
                                     0                  ,
                                     szANSI             ,
                                     -1                 ,
                                     szWide             ,
                                     iWideLen            ) ;
    return ( iRet ) ;
}

BOOL BUGSUTIL_DLLINTERFACE __stdcall  BSUIsInteractiveUser ( void )
{
    BOOL bRet = TRUE ;
    if ( TRUE == IsNT ( ) )
    {
        HWINSTA hStation = GetProcessWindowStation ( ) ;
        if ( NULL != hStation )
        {
            USEROBJECTFLAGS stUOF ;
            DWORD dwNeeded ;
            if ( TRUE ==
                  GetUserObjectInformation ( hStation                 ,
                                             UOI_FLAGS                ,
                                             &stUOF                   ,
                                             sizeof ( USEROBJECTFLAGS),
                                             &dwNeeded                ))
            {
                return (( WSF_VISIBLE & stUOF.dwFlags ) == WSF_VISIBLE);
            }

        }
    }

    return ( bRet ) ;
}

typedef struct tagTHREADNAME_INFO
{
   DWORD  dwType     ; // must be 0x1000
   LPCSTR szName     ; // pointer to name (in user addr space)
   DWORD  dwThreadID ; // thread ID (-1=caller thread)
   DWORD  dwFlags    ; // reserved for future use, must be zero
} THREADNAME_INFO ;

void BUGSUTIL_DLLINTERFACE __stdcall
                            BSUSetThreadNameW ( DWORD   dwThreadID   ,
                                                LPCWSTR szThreadName  )
{
    size_t iLen = wcslen ( szThreadName ) + 1 ;
    LPSTR szAnsiName = (LPSTR)HeapAlloc ( GetProcessHeap ( )        ,
                                            HEAP_GENERATE_EXCEPTIONS |
                                                HEAP_ZERO_MEMORY      ,
                                            iLen * sizeof ( char )    );
    BSUWide2Ansi ( szThreadName , szAnsiName , (int)iLen ) ;
    
    BSUSetThreadNameA ( dwThreadID , szAnsiName ) ;
    
    HeapFree ( GetProcessHeap ( ) , 0 , szAnsiName ) ;
                                          
}

void BUGSUTIL_DLLINTERFACE __stdcall
                            BSUSetThreadNameA ( DWORD   dwThreadID   ,
                                                LPCSTR  szThreadName  )
{
    THREADNAME_INFO stInfo ;
    stInfo.dwType       = 0x1000 ;
    stInfo.szName       = szThreadName ;
    stInfo.dwThreadID   = dwThreadID ;
    stInfo.dwFlags      = 0 ;

    __try
    {
        RaiseException ( 0x406D1388                         ,
                         0                                  ,
                         sizeof ( THREADNAME_INFO ) /
                                        sizeof ( DWORD )    ,
                         (DWORD*)&stInfo                     ) ;
    }
    __except ( EXCEPTION_CONTINUE_EXECUTION )
    {
    }
}

void BUGSUTIL_DLLINTERFACE __stdcall
                    BSUSetCurrentThreadNameW ( LPCWSTR szThreadName  )
{
    BSUSetThreadNameW ( (DWORD)-1 , szThreadName ) ;
}

void BUGSUTIL_DLLINTERFACE __stdcall
                    BSUSetCurrentThreadNameA ( LPCSTR szThreadName  )
{
    BSUSetThreadNameA ( (DWORD)-1 , szThreadName ) ;
}

