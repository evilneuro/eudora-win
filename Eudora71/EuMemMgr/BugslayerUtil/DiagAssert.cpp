/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#include "pch.h"
#include "BugslayerUtil.h"
#include "DiagAssert.h"
#include "BugslayerUtilMC.h"

// The project internal header file.
#include "Internal.h"

// Utilize the simple, controllable, and readable collections instead
// of the complicated, messy, and unreadable STL collections.
#include "JSimpleColl.h"

/*//////////////////////////////////////////////////////////////////////
// File Scope Typedefs And Constants
//////////////////////////////////////////////////////////////////////*/
// The size of buffer that DiagAssert will use.  If you want to see more
// stack trace, make this a bigger number.
#define DIAGASSERT_BUFFSIZE 4096
// The format string for setting up the event file viewer stuff.
const LPCTSTR k_REGKEYFMT = _T("SYSTEM\\CurrentControlSet\\Services\\")\
                            _T("EventLog\\Application\\%s" ) ;

#ifdef _UNICODE
const LPCSTR k_REGISTEREVENTSOURCE = "RegisterEventSourceW" ;
const LPCSTR k_REPORTEVENT = "ReportEventW" ;
const LPCSTR k_DEREGISTEREVENTSOURCE = "DeregisterEventSource" ;
#else
const LPCSTR k_REGISTEREVENTSOURCE = "RegisterEventSourceA" ;
const LPCSTR k_REPORTEVENT = "ReportEventA" ;
const LPCSTR k_DEREGISTEREVENTSOURCE = "DeregisterEventSource" ;
#endif

// The event logging only works on NT, so I have to call GetProcAddress
// on the event logging functions in order to make it work.
typedef HANDLE (WINAPI *PFNREGISTEREVENTSOURCE) ( LPCTSTR , LPCTSTR ) ;
typedef BOOL (WINAPI *PFNREPORTEVENT)( HANDLE  ,
                                       WORD    ,
                                       WORD    ,
                                       DWORD   ,
                                       PSID    ,
                                       WORD    ,
                                       DWORD   ,
                                       LPCTSTR*,
                                       LPVOID   ) ;
typedef BOOL (WINAPI *PFNDEREGISTEREVENTSOURCE)( HANDLE ) ;

// The typedef for the list of HMODULES that can possibly hold message
// resources.
typedef JSimpleArray < HINSTANCE > HINSTVECTOR ;
// The address typedef.
typedef JSimpleArray < DWORD64 > ADDRVECTOR ;

/*//////////////////////////////////////////////////////////////////////
                           File Scope Globals
//////////////////////////////////////////////////////////////////////*/
// The HMODULE vector.
static HINSTVECTOR g_HMODVector ;

// The DiagAssert display options.
static DWORD g_DiagAssertOptions = DA_SHOWMSGBOX | DA_SHOWODS ;

// The symbol engine.
static CSymbolEngine g_cSym ;
// If TRUE, the symbol engine has been initialized.
static BOOL g_bSymIsInit = FALSE ;
// The critical section to protect the symbol engine.
static CRITICAL_SECTION g_csSymEngCritSec ;

// The handle for assertion file output.
static HANDLE g_hAssertFile = INVALID_HANDLE_VALUE ;

// The handle for tracing file output.
static HANDLE g_hTraceFile = INVALID_HANDLE_VALUE ;

// Set to TRUE if runnign on NT4.
static BOOL g_bIsNT4 = FALSE ;

// The event log functions from ADVAPI32.DLL.  If these are NULL, then
// the code is running on Win9x.
static PFNREGISTEREVENTSOURCE   g_pfnRegisterEventSource = NULL ;
static PFNREPORTEVENT           g_pfnReportEvent = NULL ;
static PFNDEREGISTEREVENTSOURCE g_pfnDeRegisterEventSource = NULL ;

/*//////////////////////////////////////////////////////////////////////
                         File Scope Prototypes
//////////////////////////////////////////////////////////////////////*/
// Handles doing the stack trace for DiagAssert.
void DoStackTrace ( LPTSTR szString  ,
                    DWORD  dwSize     ) ;

// The function that does the real assertions.
BOOL __stdcall RealAssert  ( DWORD   dwOverrideOpts  ,
                             LPCWSTR szMsg           ,
                             BOOL    bAllowHalts      ) ;

// Takes care of adding the event source for event file viewing.
BOOL AddEventSource ( LPCTSTR szAppName , LPCTSTR szMsgFileDLL ) ;

// The wrapper around writing to the event log.
BOOL ReportSimpleEvent ( WORD    wType ,
                         LPCTSTR szApp ,
                         DWORD   dwID  ,
                         LPCTSTR szMsg  ) ;

/*//////////////////////////////////////////////////////////////////////
                            CODE STARTS HERE
//////////////////////////////////////////////////////////////////////*/

// The simple class to get the symbol engine critical section
// initialized and deleted.
class CritSecIniter
{
public  :
    CritSecIniter ( void )
    {
        InitializeCriticalSectionAndSpinCount ( &g_csSymEngCritSec ,
                                                4000                ) ;
    }
    
    ~CritSecIniter ( void )
    {
        DeleteCriticalSection ( &g_csSymEngCritSec ) ;
    }
} ;

static CritSecIniter g_cInitIt ;

void AcquireCommonSymEngLock ( void )
{
    EnterCriticalSection ( &g_csSymEngCritSec ) ;
}

void ReleaseCommonSymEngLock ( void )
{
    LeaveCriticalSection ( &g_csSymEngCritSec ) ;
}

DWORD BUGSUTIL_DLLINTERFACE __stdcall
    SetDiagAssertOptions ( DWORD dwOpts )
{
    if ( DA_USEDEFAULTS == dwOpts )
    {
        return ( DA_USEDEFAULTS ) ;
    }
    DWORD dwOld = g_DiagAssertOptions ;
    g_DiagAssertOptions = dwOpts ;
    return ( dwOld ) ;
}

// An internal only function so the new assertion code can use the
// same settings.
DWORD GetDiagAssertOptions ( void )
{
    return ( g_DiagAssertOptions ) ;
}

HANDLE BUGSUTIL_DLLINTERFACE __stdcall
    SetDiagAssertFile ( HANDLE hFile )
{
    HANDLE hRet = g_hAssertFile ;
    g_hAssertFile = hFile ;
    return ( hRet ) ;
}

// An internal only function so the new assertion code can use the
// same file handle.
HANDLE GetDiagAssertFile ( void )
{
    return ( g_hAssertFile ) ;
}


BOOL BUGSUTIL_DLLINTERFACE __stdcall
    AddDiagAssertModule ( HMODULE hMod )
{
    g_HMODVector.Add ( hMod ) ;
    return ( TRUE ) ;
}

BOOL BUGSUTIL_DLLINTERFACE __stdcall
    DiagAssertA ( DWORD  dwOverrideOpts ,
                  LPCSTR szMsg          ,
                  LPCSTR szFile         ,
                  DWORD  dwLine          )
{
    // First, save off the last error value.
    DWORD dwLastError = GetLastError ( ) ;

    // Format the C/C++ message.
    wchar_t szBuff [ 512 ] ;

    wsprintfW ( szBuff               ,
                L"File : %S\n"
                L"Line : %d\n"
                L"Expression : %S"   ,
                szFile               ,
                dwLine               ,
                szMsg                 ) ;

    // Set the error back and call the function that does all the work.
    SetLastError ( dwLastError ) ;
    return ( RealAssert ( dwOverrideOpts , szBuff , TRUE ) ) ;
}

BOOL BUGSUTIL_DLLINTERFACE __stdcall
    DiagAssertW ( DWORD     dwOverrideOpts  ,
                  LPCWSTR   szMsg           ,
                  LPCWSTR   szFile          ,
                  DWORD     dwLine           )
{
    // First, save off the last error value.
    DWORD dwLastError = GetLastError ( ) ;

    // Format the C/C++ message.
    wchar_t szBuff [ 512 ] ;

    wsprintfW ( szBuff               ,
                L"File : %s\n"
                L"Line : %d\n"
                L"Expression : %s"  ,
                szFile               ,
                dwLine               ,
                szMsg                 ) ;

    // Set the error back and call the function that does all the work.
    SetLastError ( dwLastError ) ;
    return ( RealAssert ( dwOverrideOpts , szBuff , TRUE ) ) ;
}

// Turn off unreachable code error after ExitProcess.
#pragma warning ( disable : 4702 )

// The code that does the real assertion work.
BOOL __stdcall RealAssert  ( DWORD   dwOverrideOpts  ,
                             LPCTSTR szMsg           ,
                             BOOL    bAllowHalts      )
{
    // The buffer used for the final message text.
    TCHAR * szBuff = new TCHAR [ DIAGASSERT_BUFFSIZE ] ;
    // The current position in szBuff ;
    LPTSTR pCurrPos = szBuff ;
    // The module name.
    TCHAR  szModName[ MAX_PATH + 1 ] ;
    // The decoded message from FormatMessage
    LPTSTR szFmtMsg = NULL ;
    // The options.
    DWORD  dwOpts = dwOverrideOpts ;
    // The last error value.  (Which is preserved across the call).
    DWORD  dwLastErr = GetLastError ( ) ;


    if ( DA_USEDEFAULTS == dwOverrideOpts )
    {
        dwOpts = g_DiagAssertOptions ;
    }

    // Look in any specified modules for the code.
    for ( int i = 0 ; i < g_HMODVector.GetSize ( ) ; i++ )
    {
        if ( 0 != FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER    |
                                    FORMAT_MESSAGE_IGNORE_INSERTS   |
                                    FORMAT_MESSAGE_FROM_HMODULE      ,
                                  g_HMODVector[ i ]                  ,
                                  dwLastErr                          ,
                                  0                                  ,
                                  (LPTSTR)&szFmtMsg                  ,
                                  0                                  ,
                                  NULL                               ))
        {
            break ;
        }
    }

    // If the message was not translated, just look in the system.
    if ( NULL == szFmtMsg )
    {
        FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER    |
                          FORMAT_MESSAGE_IGNORE_INSERTS   |
                          FORMAT_MESSAGE_FROM_SYSTEM        ,
                        NULL                                ,
                        dwLastErr                           ,
                        0                                   ,
                        (LPTSTR)&szFmtMsg                   ,
                        0                                   ,
                        NULL                                 ) ;
    }

    // Make sure the message got translated into something.
    LPTSTR szRealLastErr ;
    if ( NULL != szFmtMsg )
    {
        szRealLastErr = szFmtMsg ;
    }
    else
    {
        szRealLastErr = _T ( "**Last error code does not exist!!!!" ) ;
    }

    // Get the module name.
    if ( 0 == GetModuleFileName ( NULL , szModName , MAX_PATH ) )
    {
        _tcscpy ( szModName , _T ( "<unknown application>" ) ) ;
    }

    // Build the message.
    pCurrPos += (wsprintf ( szBuff                               ,
                            _T( "Debug Assertion Failed!\n\n" ) \
                            _T( "Program : %s\n" ) \
                            _T( "%s\n" ) \
                            _T( "Last Error (0x%08X) : %s\n" )   ,
                             szModName                           ,
                             szMsg                               ,
                             dwLastErr                           ,
                             szFmtMsg                             ));

    // Get rid of the allocated memory from FormatMessage.
    if ( NULL != szFmtMsg )
    {
        LocalFree ( (LPVOID)szFmtMsg ) ;
    }

    // Am I supposed to show the stack trace too?
    if ( DA_SHOWSTACKTRACE == ( DA_SHOWSTACKTRACE & dwOpts ) )
    {
        DoStackTrace ( pCurrPos ,
                       sizeof ( szBuff ) -
                    (DWORD)((DWORD_PTR)pCurrPos-(DWORD_PTR)szBuff) ) ;
    }

    // Is this supposed to go to ODS?
    if ( DA_SHOWODS == ( DA_SHOWODS & dwOpts ) )
    {
        OutputDebugString ( szBuff ) ;
    }

    if ( DA_SHOWEVENTLOG == ( DA_SHOWEVENTLOG & dwOpts ) )
    {
        // Only write to the event log if everything is really kosher.
        static bSuccessful = TRUE ;
        if ( TRUE == bSuccessful )
        {
            bSuccessful = OutputToEventLog ( szBuff ) ;
        }
    }

    // If the file handle is something, write to it.  I do not do any
    // error checking on purpose.
    if ( INVALID_HANDLE_VALUE != g_hAssertFile )
    {
        DWORD  dwWritten ;
        char * pToWrite = NULL ;
        int    iLen = lstrlen ( szBuff ) ;

#ifdef UNICODE
        pToWrite = (char*)_alloca ( iLen + 1 ) ;

        BSUWide2Ansi ( szBuff , pToWrite , iLen + 1 ) ;
#else
        pToWrite = szBuff ;
#endif
        WriteFile ( g_hAssertFile   ,
                    pToWrite        ,
                    iLen            ,
                    &dwWritten      ,
                    NULL             ) ;
    }

    // Check out the kind of buttons I am supposed to do.
    UINT uiType = MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONERROR ;
    if ( TRUE == bAllowHalts )
    {
        uiType |= MB_ABORTRETRYIGNORE ;
    }
    else
    {
        uiType |= MB_OK ;
    }

    // By default, treat the return as an IGNORE.  This works best in
    // the case the user does not want the MessageBox.  The only way
    // you'll get the message box is if running as an interactive user.
    int iRet = IDIGNORE ;
    if ( ( DA_SHOWMSGBOX == ( DA_SHOWMSGBOX & dwOpts ) ) &&
         ( TRUE == BSUIsInteractiveUser ( )            )    )
    {
        HWND hWndParent = GetActiveWindow ( ) ;
        if ( NULL != hWndParent )
        {
            hWndParent = GetLastActivePopup ( hWndParent ) ;
        }
        iRet = MessageBox ( hWndParent                      ,
                            szBuff                          ,
                            _T ( "ASSERTION FAILURE..."  )  ,
                            uiType                           ) ;
    }
    
    // Get rid of the allocated buffer.
    delete [] szBuff ;

    // Put the incoming last error back.
    SetLastError ( dwLastErr ) ;

    // Figure out what to do on the return.
    if ( ( IDIGNORE == iRet ) || ( IDOK == iRet ) )
    {
        return ( FALSE ) ;
    }
    if ( IDRETRY == iRet )
    {
        // This will trigger DebugBreak!!
        return ( TRUE ) ;
    }

    // The return has to be Abort....
    ExitProcess ( (UINT)-1 ) ;
    return ( TRUE ) ;
}
// Turn on unreachable code error
#pragma warning ( default : 4702 )

HANDLE BUGSUTIL_DLLINTERFACE __stdcall
    SetDiagOutputFile ( HANDLE hFile )
{
    HANDLE hRet = g_hTraceFile ;
    g_hTraceFile = hFile ;
    return ( hRet ) ;
}

void BUGSUTIL_DLLINTERFACE
    DiagOutputA ( LPCSTR szFmt , ... )
{
    // Never corrupt the last error value.
    DWORD dwLastError = GetLastError ( ) ;

    char szOutBuff [ 1024 ] ;

    va_list  args ;

    va_start ( args , szFmt ) ;

    wvsprintfA ( szOutBuff , szFmt , args ) ;

    OutputDebugStringA ( szOutBuff ) ;

    if ( INVALID_HANDLE_VALUE != g_hTraceFile )
    {
        DWORD dwWritten ;
        WriteFile ( g_hTraceFile           ,
                    szOutBuff              ,
                    lstrlenA ( szOutBuff ) ,
                    &dwWritten             ,
                    NULL                    ) ;
    }

    va_end ( args ) ;

    SetLastError ( dwLastError ) ;
}

void BUGSUTIL_DLLINTERFACE
    DiagOutputW ( LPCWSTR szFmt , ... )
{
    // Never corrupt the last error value.
    DWORD dwLastError = GetLastError ( ) ;

    wchar_t szOutBuff [ 1024 ] ;

    va_list  args ;

    va_start ( args , szFmt ) ;

    wvsprintfW ( szOutBuff , szFmt , args ) ;

    OutputDebugStringW ( szOutBuff ) ;

    if ( INVALID_HANDLE_VALUE != g_hTraceFile )
    {
        DWORD dwWritten ;
        int iLen = lstrlenW ( szOutBuff ) ;
        char * pToWrite = (char*)_alloca ( iLen + 1 ) ;

        BSUWide2Ansi ( szOutBuff , pToWrite , iLen + 1 ) ;

        WriteFile ( g_hTraceFile    ,
                    pToWrite        ,
                    iLen            ,
                    &dwWritten      ,
                    NULL             ) ;
    }

    va_end ( args ) ;

    SetLastError ( dwLastError ) ;
}

DWORD64 __stdcall GetModBase ( HANDLE hProcess , DWORD64 dwAddr )
{
    // Check in the symbol engine first.
    IMAGEHLP_MODULE64 stIHM ;

    // This is what the MFC stack trace routines forgot to do so their
    // code will not get the info out of the symbol engine.
    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE64 ) ;

    // Check to see if the module is already loaded.
    if ( g_cSym.SymGetModuleInfo64 ( dwAddr , &stIHM ) )
    {
        // Darn DBGHELP.H crummy declarations!
        return ( stIHM.BaseOfImage ) ;
    }
    else
    {
        // The module is not loaded, so let's go fishing.
        MEMORY_BASIC_INFORMATION stMBI ;

        // Do the VirtualQueryEx to see if I can find the start of
        // this module.  Since the HMODULE is the start of a module
        // in memory, viola, this will give me the HMODULE.
        if ( 0 != VirtualQueryEx ( hProcess         ,
                                   (LPCVOID)dwAddr  ,
                                   &stMBI           ,
                                   sizeof ( stMBI )  ) )
        {
            // Try and load it.
            DWORD dwNameLen = 0 ;
            char szFile[ MAX_PATH ] ;

            // Using the address base for the memory location, try
            // to grab the module filename.
            dwNameLen = GetModuleFileNameA ( (HINSTANCE)
                                                stMBI.AllocationBase ,
                                            szFile                   ,
                                            MAX_PATH                  );

            HANDLE hFile = NULL ;

            if ( 0 != dwNameLen )
            {
                // Very cool, I found the DLL.  Now open it up for
                // reading.
                hFile = CreateFileA ( szFile          ,
                                      GENERIC_READ    ,
                                      FILE_SHARE_READ ,
                                      NULL            ,
                                      OPEN_EXISTING   ,
                                      0               ,
                                      0                ) ;
            }
            // Go ahead and try to load the module anyway.
#ifdef _DEBUG
//            DWORD dwRet =
#endif
            g_cSym.SymLoadModule64 ( hFile                            ,
                                     ( dwNameLen ? szFile : NULL )    ,
                                     NULL                             ,
                                     (DWORD_PTR)stMBI.AllocationBase  ,
                                     0                                 ) ;
#ifdef _DEBUG
//            if ( 0 == dwRet )
//            {
//                TRACE ( "SymLoadModule failed : 0x%08X\n" ,
//                        GetLastError ( )                   ) ;
//            }
#endif  // _DEBUG
            return ( (DWORD64)stMBI.AllocationBase ) ;
        }
    }
    return ( 0 ) ;
}

// Starting with Windows XP, NTDLL.DLL no longer users INT21 to make
// the transition to kernel mode.  Now the transition is done through
// the SYSENTER instruction.  What's interesting is that the location
// of the function doing the call is is the shared memory mapped into
// both kernel and user mode at 0x7FFE0000.  This little function
// checks the address to see if it's the return address for that
// function.
#define SHARED_MEM_BASE_ADDR            0x000000007FFE0000
#define SHARED_MEM_SYSTEMSTUBCALL       0x000000007FFE0300
#define SHARED_MEM_SYSTEMSTUBCALLRET    0x000000007FFE0304
BOOL IsSharedUserDataSystemStubCallRet ( DWORD64 dwAddr )
{
    BOOL bRet = FALSE ;
    if ( TRUE == IsXPorBetter ( ) )
    {
        // Is the memory in the shared data secton?
        if ( SHARED_MEM_BASE_ADDR == ( dwAddr & SHARED_MEM_BASE_ADDR ) )
        {
            if ( SHARED_MEM_SYSTEMSTUBCALLRET ==
                             ( dwAddr & SHARED_MEM_SYSTEMSTUBCALLRET ) )
            {
                return ( TRUE ) ;
            }
        }
    }
    return ( bRet ) ;
}

#define MAX_SYM_SIZE 1024

DWORD ConvertAddress ( DWORD64   dwAddr     ,
                       LPTSTR    szOutBuff  ,
                       LPCTSTR   szCRLF      )
{
    TCHAR szTemp [ MAX_SYM_SIZE + sizeof ( IMAGEHLP_SYMBOL64 ) ] ;

    PIMAGEHLP_SYMBOL64 pIHS = (PIMAGEHLP_SYMBOL64)&szTemp ;

    IMAGEHLP_MODULE64 stIHM ;
    
    BOOL bIsSharedMemStub = FALSE ;

    LPTSTR pCurrPos = szOutBuff ;

    ZeroMemory ( pIHS , MAX_SYM_SIZE + sizeof ( IMAGEHLP_SYMBOL64 ) ) ;
    ZeroMemory ( &stIHM , sizeof ( IMAGEHLP_MODULE64 ) ) ;

    pIHS->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 ) ;
    pIHS->Address = dwAddr ;
    pIHS->MaxNameLength = MAX_SYM_SIZE ;

    stIHM.SizeOfStruct = sizeof ( IMAGEHLP_MODULE64 ) ;

    // Always stick the address in first.
    pCurrPos += wsprintf ( pCurrPos         ,
#ifdef _WIN64
                           _T ( "0x%16X " ) ,
#else
                           _T ( "0x%08X " ) ,
#endif
                           dwAddr            ) ;

    // Try and look up the module.
    if ( 0 == g_cSym.SymGetModuleInfo64 ( dwAddr , &stIHM ) )
    {
        // It might not have been loaded, so try it now.
        GetModBase ( GetCurrentProcess ( ) , dwAddr ) ;
    }
    // Get the module name.
    if ( 0 != g_cSym.SymGetModuleInfo64 ( dwAddr , &stIHM ) )
    {
        // Symbols are ANSI.
        // Strip off the path.
        LPSTR szName = strrchr ( stIHM.ImageName , '\\' ) ;
        if ( NULL != szName )
        {
            szName++ ;
        }
        else
        {
            szName = stIHM.ImageName ;
        }
        pCurrPos += wsprintf ( pCurrPos      ,
#ifdef UNICODE
                               _T ( "%S: " ) ,
#else
                               _T ( "%s: " ) ,
#endif
                               szName         ) ;
    }
    else
    {
        bIsSharedMemStub = IsSharedUserDataSystemStubCallRet ( dwAddr );
        if ( TRUE == bIsSharedMemStub )
        {
            pCurrPos += wsprintf ( pCurrPos                  ,
                                   _T ( "SharedUserData: " )  ) ;
        }
        else
        {
            pCurrPos += wsprintf ( pCurrPos ,
                                   _T ( "<unknown module>: " ) );
        }
    }

    // Get the function.
    DWORD64 dwDisp ;
    if ( 0 != g_cSym.SymGetSymFromAddr64 ( dwAddr , &dwDisp , pIHS ) )
    {
        if ( 0 == dwDisp )
        {
            pCurrPos += wsprintf ( pCurrPos , _T ( "%S" ) , pIHS->Name);
        }
        else
        {
            pCurrPos += wsprintf ( pCurrPos               ,
                                   _T ( "%S + %d bytes" ) ,
                                   pIHS->Name             ,
                                   dwDisp                  ) ;
        }

        // If I got a symbol, give the source and line a whirl.
        IMAGEHLP_LINE64 stIHL ;

        ZeroMemory ( &stIHL , sizeof ( IMAGEHLP_LINE64 ) ) ;

        stIHL.SizeOfStruct = sizeof ( IMAGEHLP_LINE64 ) ;

        DWORD dwLineDisp ;
        if ( 0 != g_cSym.SymGetLineFromAddr64 ( dwAddr      ,
                                                &dwLineDisp ,
                                                &stIHL       ) )
        {
            // Put this on the next line and indented a bit.
            pCurrPos += wsprintf ( pCurrPos                      ,
                                   _T ( "%s\t\t%S%s\t\tLine %d" ) ,
                                   szCRLF                         ,
                                   stIHL.FileName                 ,
                                   szCRLF                         ,
                                   stIHL.LineNumber                ) ;
            if ( 0 != dwDisp )
            {
                pCurrPos += wsprintf ( pCurrPos             ,
                                       _T ( " + %d bytes" ) ,
                                       dwLineDisp            ) ;
            }
        }
    }
    else if ( TRUE == bIsSharedMemStub )
    {
        pCurrPos += wsprintf ( pCurrPos                        ,
                               _T ( "SystemCallStub+4 bytes" )  ) ;
    }
    else
    {
        pCurrPos += wsprintf ( pCurrPos , _T ( "<unknown symbol>" ) ) ;
    }

    // Tack on a CRLF.
    pCurrPos += wsprintf ( pCurrPos , _T ( "%s" ) , szCRLF ) ;

    return ( (DWORD)(pCurrPos - szOutBuff) ) ;
}

void InitializeCommonSymbolEngine ( void )
{
    if ( FALSE == g_bSymIsInit )
    {
        DWORD dwOpts = SymGetOptions ( ) ;

        // Turn on load lines.
        SymSetOptions ( dwOpts                |
                        SYMOPT_LOAD_LINES      ) ;

        if ( FALSE == g_cSym.SymInitialize ( GetCurrentProcess ( )  ,
                                             NULL                   ,
                                             FALSE                   ) )
        {
            TRACE ( "DiagAssert : Unable to initialize the "
                    "symbol engine!!!\n" ) ;
#ifdef _DEBUG
            DebugBreak ( ) ;
#endif
        }
        else
        {
            g_bSymIsInit = TRUE ;
        }
    }
}

BOOL CommonSymbolEngineEnumLocalVariables
                    ( PENUM_LOCAL_VARS_CALLBACK      pCallback     ,
                      int                            iExpandLevel  ,
                      BOOL                           bExpandArrays ,
                      PREAD_PROCESS_MEMORY_ROUTINE64 pReadMem      ,
                      LPSTACKFRAME64                 pFrame        ,
                      CONTEXT *                      pContext      ,
                      PVOID                          pUserContext   )
{
    return ( g_cSym.EnumLocalVariables ( pCallback     ,
                                         iExpandLevel  ,
                                         bExpandArrays ,
                                         pReadMem      ,
                                         pFrame        ,
                                         pContext      ,
                                         pUserContext   ) ) ;
}

void DoStackTrace ( LPTSTR szString  ,
                    DWORD  dwSize     )
{
    HANDLE hProcess = GetCurrentProcess ( ) ;

    // If the symbol engine is not initialized, do it now.
    if ( FALSE == g_bSymIsInit )
    {
        DWORD dwOpts = SymGetOptions ( ) ;

        // Turn on load lines.
        SymSetOptions ( dwOpts                |
                        SYMOPT_LOAD_LINES      ) ;

        if ( FALSE == g_cSym.SymInitialize ( hProcess ,
                                             NULL     ,
                                             FALSE     ) )
        {
            TRACE ( "DiagAssert : Unable to initialize the "
                    "symbol engine!!!\n" ) ;
#ifdef _DEBUG
            DebugBreak ( ) ;
#endif
        }
        else
        {
            g_bSymIsInit = TRUE ;
        }
    }

    // The symbol engine is initialized so do the stack walk.

    // The array of addresses.
    ADDRVECTOR vAddrs ;

    // The thread information.
    CONTEXT    stCtx  ;

    stCtx.ContextFlags = CONTEXT_FULL ;

    // Get the thread context.  Since I am doing this on the CURRENT
    // executing thread, the context will be from down in the bowls of
    // KERNEL32.DLL.  Probably GetThreadContext itself.
    if ( GetThreadContext ( GetCurrentThread ( ) , &stCtx ) )
    {
        STACKFRAME64 stFrame ;
        DWORD      dwMachine ;

        ZeroMemory ( &stFrame , sizeof ( STACKFRAME64 ) ) ;

        stFrame.AddrPC.Mode = AddrModeFlat ;

#if defined (_M_IX86)
        dwMachine                = IMAGE_FILE_MACHINE_I386 ;

        stFrame.AddrPC.Offset    = stCtx.Eip    ;
        stFrame.AddrStack.Offset = stCtx.Esp    ;
        stFrame.AddrFrame.Offset = stCtx.Ebp    ;
        stFrame.AddrStack.Mode   = AddrModeFlat ;
        stFrame.AddrFrame.Mode   = AddrModeFlat ;

#elif defined (_M_ALPHA)
        dwMachine                = IMAGE_FILE_MACHINE_ALPHA ;
        stFrame.AddrPC.Offset    = (unsigned long)stCtx.Fir ;
#else
#error ( "Unknown machine!" )
#endif

        // Loop for the first 512 stack elements.
        for ( DWORD i = 0 ; i < 512 ; i++ )
        {
            // Why does DBGHELP.H have all it's declarations wrong
            // for Win32/Win64 compared to every other header file in
            // the Platform SDK?
            if ( FALSE == StackWalk64 ( dwMachine                ,
                                        hProcess                 ,
                                        hProcess                 ,
                                        &stFrame                 ,
                                        &stCtx                   ,
                                        NULL                     ,
                                        SymFunctionTableAccess64 ,
                                        GetModBase               ,
                                        NULL                      ) )
            {
                break ;
            }
            // Also check that the address is not zero.  Sometimes
            // StackWalk returns TRUE with a frame of zero.
            if ( 0 != stFrame.AddrPC.Offset )
            {
                vAddrs.Add ( stFrame.AddrPC.Offset ) ;
            }
        }

        // Now start converting the addresses.
        DWORD dwSizeLeft = dwSize ;
        DWORD dwSymSize ;

        TCHAR szSym [ MAX_PATH * 2 ] ;
        LPTSTR szCurrPos = szString ;

        BOOL bSeenDiagAssert = FALSE ;
        for ( int i = 0 ; i < vAddrs.GetSize ( ) ; i++ )
        {
            dwSymSize = ConvertAddress ( vAddrs[ i ] , 
                                         szSym       , 
                                         _T ( "\n" )  ) ;
            // Throw out everything with DiagAssert.cpp in it.
            if ( _tcsstr ( szSym , _T ( "diagassert.cpp" ) ) )
            {
                bSeenDiagAssert = TRUE ;
                continue ;
            }
            // Throw out anything before the functions in
            // DiagAssert.cpp
            if ( FALSE == bSeenDiagAssert )
            {
                continue ;
            }
            if ( dwSizeLeft < dwSymSize )
            {
                break ;
            }
            _tcscpy ( szCurrPos , szSym ) ;
            szCurrPos += dwSymSize ;
            dwSizeLeft -= dwSymSize ;
        }
    }
}

DWORD ConvertErrorToMessage ( DWORD   dwLastErr ,
                              TCHAR * szMsg     ,
                              DWORD   dwSize     )
{
    DWORD dwRet = 0 ;
    // Look in any specified modules for the code.
    for ( int i = 0 ; i < g_HMODVector.GetSize ( ) ; i++ )
    {
        dwRet = FormatMessage ( FORMAT_MESSAGE_IGNORE_INSERTS   |
                                  FORMAT_MESSAGE_FROM_HMODULE      ,
                                g_HMODVector[ i ]                  ,
                                dwLastErr                          ,
                                0                                  ,
                                szMsg                              ,
                                dwSize                             ,
                                NULL                               ) ;
        if ( 0 != dwRet )
        {
            // Found it, kick out.
            break ;
        }
    }

    // If the message was not translated, just look in the system.
    if ( 0 == dwRet )
    {
        dwRet = FormatMessage (  FORMAT_MESSAGE_IGNORE_INSERTS   |
                                  FORMAT_MESSAGE_FROM_SYSTEM        ,
                                NULL                                ,
                                dwLastErr                           ,
                                0                                   ,
                                szMsg                               ,
                                dwSize                              ,
                                NULL                                 ) ;
    }
    return ( dwRet ) ;
}

BOOL AddEventSource ( LPCTSTR szAppName , LPCTSTR szMsgFileDLL )
{
    HKEY  hk ;
    TCHAR szBuff[ MAX_PATH ] ;

    // Fill out the registry key name.
    wsprintf ( szBuff , k_REGKEYFMT , szAppName ) ;

    // Create the registry subkey.
    if ( ERROR_SUCCESS != RegCreateKey ( HKEY_LOCAL_MACHINE ,
                                         szBuff             ,
                                         &hk                 ) )
    {
        return ( FALSE ) ;
    }

	// Get the data length.
    int iDataLen = ( lstrlen ( szMsgFileDLL ) * sizeof ( TCHAR ) ) + 
                   ( 1 * sizeof ( TCHAR ) ) ;

    // Add the name to the EventMessageFile subkey.
    if ( ERROR_SUCCESS != RegSetValueEx ( hk                        ,
                                          _T ( "EventMessageFile" ) ,
                                          0                         ,
                                          REG_EXPAND_SZ             ,
                                          (LPBYTE)szMsgFileDLL      ,
                                          iDataLen    ) )
    {
        RegCloseKey ( hk ) ;
        return ( FALSE ) ;
    }

    // Set the supported event types in the TypesSupported subkey.
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
                   EVENTLOG_INFORMATION_TYPE ;

    if ( ERROR_SUCCESS != RegSetValueEx ( hk                      ,
                                          _T ( "TypesSupported" ) ,
                                          0                       ,
                                          REG_DWORD               ,
                                          (LPBYTE)&dwData         ,
                                          sizeof ( DWORD )         ) )
    {
        RegCloseKey ( hk ) ;
        return ( FALSE ) ;
    }

    RegCloseKey ( hk ) ;
    return ( TRUE ) ;
}

BOOL ReportSimpleEvent ( WORD    wType ,
                         LPCTSTR szApp ,
                         DWORD   dwID  ,
                         LPCTSTR szMsg  )
{
    HANDLE hES = g_pfnRegisterEventSource ( NULL , szApp ) ;
    if ( NULL == hES )
    {
        return ( FALSE ) ;
    }

    LPTSTR pMsg = (LPTSTR)szMsg ;
    BOOL bAllocated = FALSE ;
    // If this is NT4, I have to replace all \n with \r\n. Grumble....
    if ( TRUE == g_bIsNT4 )
    {
        bAllocated = TRUE ;
        size_t iLen = _tcslen ( szMsg ) ;
        pMsg = (TCHAR*)malloc ( iLen * 2 ) ;

        size_t iCurrNew = 0 ;
        for ( size_t iCurrOld = 0 ; iCurrOld < iLen ; iCurrOld++ )
        {
            if ( _T ( '\n' ) == szMsg[ iCurrOld ] )
            {
                pMsg[ iCurrNew ] = _T ( '\r' ) ;
                iCurrNew++ ;
            }
            pMsg[ iCurrNew ] = szMsg [ iCurrOld ] ;
            iCurrNew++ ;
        }
        pMsg[ iCurrNew ] = _T ( '\0' ) ;
    }

    BOOL bRet ;
    if ( FALSE == g_pfnReportEvent ( hES             ,
                                     wType           ,
                                     0               ,
                                     dwID            ,
                                     NULL            ,
                                     1               ,
                                     0               ,
                                     (LPCTSTR*)&pMsg ,
                                     NULL             ) )
    {
        bRet = FALSE ;
    }
    else
    {
        bRet = TRUE ;
    }

    if ( TRUE == bAllocated )
    {
        free ( pMsg ) ;
    }

    g_pfnDeRegisterEventSource ( hES ) ;
    return ( bRet ) ;
}

BOOL OutputToEventLog ( LPCTSTR szMsg )
{
    // Only add the event source once.
    static BOOL bAddedEventSource = FALSE ;
    if ( FALSE == bAddedEventSource )
    {
        bAddedEventSource = TRUE ;

        // Get the event logging functions I need.
        HINSTANCE hInstADVAPI32 = LoadLibrary ( _T ( "ADVAPI32.DLL" ) );
        g_pfnRegisterEventSource = (PFNREGISTEREVENTSOURCE)
                              GetProcAddress ( hInstADVAPI32 ,
                                               k_REGISTEREVENTSOURCE ) ;
        g_pfnReportEvent = (PFNREPORTEVENT)
                            GetProcAddress ( hInstADVAPI32 ,
                                             k_REPORTEVENT  ) ;
        g_pfnDeRegisterEventSource = (PFNDEREGISTEREVENTSOURCE)
                            GetProcAddress ( hInstADVAPI32 ,
                                             k_DEREGISTEREVENTSOURCE ) ;

        // Whoops!  This is Win9x,  just stop now.
        if ( NULL == g_pfnReportEvent )
        {
            return ( TRUE ) ;
        }

        // Get the BUGSLAYERUTIL.DLL module handle.
        //HINSTANCE hMod = GetModuleHandle ( _T ( "BUGSLAYERUTIL.DLL" ) );
		HINSTANCE hMod = GetModuleHandle ( _T ( "EUMEMMGR.DLL" ) );
        if ( NULL == hMod )
        {
            return ( FALSE ) ;
        }
        TCHAR szBuff[ MAX_PATH ] ;
        // Get the complete directory where BUGSLAYERUTIL.DLL currently
        // resides as that's the one which I will have the event
        // viewer use.
        if ( 0 == GetModuleFileName ( hMod , szBuff , MAX_PATH ) )
        {
            return ( FALSE ) ;
        }
        // Now set up the registry.
        if ( FALSE == AddEventSource ( _T ( "BugslayerUtil" ) ,
                                       szBuff                  ) )
        {
            return ( FALSE ) ;
        }

        // Figure out the OS.
        OSVERSIONINFO stOSVI ;
        stOSVI.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO ) ;
        GetVersionEx ( &stOSVI ) ;
        if ( ( 4 == stOSVI.dwMajorVersion                   ) &&
             ( VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId )    )
        {
            g_bIsNT4 = TRUE ;
        }

    }

    if ( NULL == g_pfnReportEvent )
    {
        // I'm on Win9x so just report TRUE.
        return ( TRUE ) ;
    }

    // Slap the message in the event viewer.
    return ( ReportSimpleEvent ( EVENTLOG_ERROR_TYPE    ,
                                 _T ( "BugslayerUtil" ) ,
                                 MSG_BSU_ERROR_ERROR    ,
                                 szMsg                   ) ) ;
}
