/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#include "pch.h"
#include "BugslayerUtil.h"
#include "CrashHandler.h"

// The project internal header file
#include "Internal.h"

/*//////////////////////////////////////////////////////////////////////
// File Scope Defines
//////////////////////////////////////////////////////////////////////*/
// The maximum symbol size handled in the module
#define MAX_SYM_SIZE  512
#define BUFF_SIZE 2048
#define SYM_BUFF_SIZE 1024

// String format constants.  To avoid doing tons of ANSI to UNICODE
// conversions myself, I rely on wsprintf to do them.  In order to make
// this file happy for ANSI compiles, I need keep the %S out of the
// format strings.
#ifdef UNICODE
#define k_NAMEDISPFMT       _T ( " %S()+%04d byte(s)" )
#define k_NAMEFMT           _T ( " %S " )
#define k_FILELINEDISPFMT   _T ( " %S, line %04d+%04d byte(s)" )
#define k_FILELINEFMT       _T ( " %S, line %04d" )
#else
#define k_NAMEDISPFMT       _T ( " %s()+%04d byte(s)" )
#define k_NAMEFMT           _T ( " %s " )
#define k_FILELINEDISPFMT   _T ( " %s, line %04d+%04d byte(s)" )
#define k_FILELINEFMT       _T ( " %s, line %04d" )
#endif

#ifdef _WIN64
#define k_PARAMFMTSTRING   _T ( " (0x%016X 0x%016X 0x%016X 0x%016X)" )
#else
#define k_PARAMFMTSTRING   _T ( " (0x%08X 0x%08X 0x%08X 0x%08X)" )
#endif

// Define the machine type.
#ifdef _X86_
#define CH_MACHINE IMAGE_FILE_MACHINE_I386
#elif _AMD64_
#define CH_MACHINE IMAGE_FILE_MACHINE_AMD64
#elif _IA64_
#define CH_MACHINE IMAGE_FILE_MACHINE_IA64
#else
#pragma FORCE COMPILE ABORT!
#endif

/*//////////////////////////////////////////////////////////////////////
// File Scope Global Variables
//////////////////////////////////////////////////////////////////////*/
// The custom unhandled exception filter (crash handler)
static PFNCHFILTFN g_pfnCallBack = NULL ;

// The original unhandled exception filter
static LPTOP_LEVEL_EXCEPTION_FILTER g_pfnOrigFilt = NULL ;

// The array of modules to limit crash handler to
static HMODULE * g_ahMod = NULL ;
// The size, in items, of g_ahMod
static UINT g_uiModCount = 0 ;

// The static buffer returned by various functions. This buffer
// allows data to be transferred without using the stack.
static TCHAR g_szBuff [ BUFF_SIZE ] ;

// The static symbol lookup buffer
static BYTE g_stSymbol [ SYM_BUFF_SIZE ] ;

// The static source file and line number structure
static IMAGEHLP_LINE64 g_stLine ;

// The stack frame used in walking the stack
static STACKFRAME64 g_stFrame ;

// The flag indicating that the symbol engine has been initialized
static BOOL g_bSymEngInit = FALSE ;

// The original version of this code changed the CONTEXT structure when
// passed through the stack walking code.  Therefore, if the user
// utilized the containing  EXCEPTION_POINTERS to write a mini dump, the
// dump wasn't correct.  I now save off the CONTEXT as a global, much
// like the stack frame.
static CONTEXT g_stContext ;

/*//////////////////////////////////////////////////////////////////////
// File Scope Function Declarations
//////////////////////////////////////////////////////////////////////*/
// The exception handler
LONG __stdcall CrashHandlerExceptionFilter ( EXCEPTION_POINTERS *
                                             pExPtrs              ) ;

// Converts a simple exception to a string value
LPCTSTR ConvertSimpleException ( DWORD dwExcept ) ;

// The internal function that does all the stack walking
LPCTSTR __stdcall InternalGetStackTraceString ( DWORD dwOpts ) ;

// Initializes the symbol engine if needed
void InitSymEng ( void ) ;

// Cleans up the symbol engine if needed
void CleanupSymEng ( void ) ;

/*//////////////////////////////////////////////////////////////////////
// Destructor Class
//////////////////////////////////////////////////////////////////////*/
// See the note in MEMDUMPERVALIDATOR.CPP about automatic classes.
// Turn off warning : initializers put in library initialization area
#pragma warning (disable : 4073)
#pragma init_seg(lib)
class CleanUpCrashHandler
{
public  :
    CleanUpCrashHandler ( void )
    {
    }
    ~CleanUpCrashHandler ( void )
    {
        // Are there any outstanding memory allocations?
        if ( NULL != g_ahMod )
        {
            VERIFY ( HeapFree ( GetProcessHeap ( ) ,
                                0                  ,
                                g_ahMod             ) ) ;
            g_ahMod = NULL ;
            // FIXED BUG - Thanks to Gennady Mayko.
            g_uiModCount = 0 ;
        }
        if ( NULL != g_pfnOrigFilt )
        {
            // Restore the original unhandled exception filter.
            SetUnhandledExceptionFilter ( g_pfnOrigFilt ) ;
            g_pfnOrigFilt = NULL ;
        }
    }
} ;

// The static class
static CleanUpCrashHandler g_cBeforeAndAfter ;

/*//////////////////////////////////////////////////////////////////////
// Crash Handler Function Implementation
//////////////////////////////////////////////////////////////////////*/

BOOL __stdcall SetCrashHandlerFilter ( PFNCHFILTFN pFn )
{
    // A NULL parameter unhooks the callback.
    if ( NULL == pFn )
    {
        if ( NULL != g_pfnOrigFilt )
        {
            // Restore the original unhandled exception filter.
            SetUnhandledExceptionFilter ( g_pfnOrigFilt ) ;
            g_pfnOrigFilt = NULL ;
            if ( NULL != g_ahMod )
            {
                // FIXED BUG:
                // Previously, I called "free" instead of "HeapFree."
                VERIFY ( HeapFree ( GetProcessHeap ( ) ,
                                    0                  ,
                                    g_ahMod             ) ) ;
                g_ahMod = NULL ;
                // FIXED BUG - Thanks to Gennady Mayko.
                g_uiModCount = 0 ;
            }
            g_pfnCallBack = NULL ;
        }
    }
    else
    {
        ASSERT ( FALSE == IsBadCodePtr ( (FARPROC)pFn ) ) ;
        if ( TRUE == IsBadCodePtr ( (FARPROC)pFn ) )
        {
            return ( FALSE ) ;
        }
        g_pfnCallBack = pFn ;

        // If a custom crash handler isn't already in use, enable
        // CrashHandlerExceptionFilter and save the original unhandled
        // exception filter.
        if ( NULL == g_pfnOrigFilt )
        {
            g_pfnOrigFilt =
               SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
        }
    }
    return ( TRUE ) ;
}

BOOL __stdcall AddCrashHandlerLimitModule ( HMODULE hMod )
{
    // Check the obvious cases.
    ASSERT ( NULL != hMod ) ;
    if ( NULL == hMod )
    {
        return ( FALSE ) ;
    }

    // Allocate a temporary array. This array must be allocated from
    // memory that's guaranteed to be around even if the process is
    // toasting. If the process is toasting, the RTL heap probably isn't
    // safe, so I allocate the temporary array from the process heap.
    HMODULE * phTemp = (HMODULE*)
                    HeapAlloc ( GetProcessHeap ( )                 ,
                                HEAP_ZERO_MEMORY |
                                   HEAP_GENERATE_EXCEPTIONS        ,
                                (sizeof(HMODULE)*(g_uiModCount+1))  ) ;
    ASSERT ( NULL != phTemp ) ;
    if ( NULL == phTemp )
    {
        TRACE ( "Serious trouble in the house! - "
                "HeapAlloc failed!!!\n"            );
        return ( FALSE ) ;
    }

    if ( NULL == g_ahMod )
    {
        g_ahMod = phTemp ;
        g_ahMod[ 0 ] = hMod ;
        g_uiModCount++ ;
    }
    else
    {
        // Copy the old values.
        CopyMemory ( phTemp     ,
                     g_ahMod    ,
                     sizeof ( HMODULE ) * g_uiModCount ) ;
        // Free the old memory.
        VERIFY ( HeapFree ( GetProcessHeap ( ) , 0 , g_ahMod ) ) ;
        g_ahMod = phTemp ;
        g_ahMod[ g_uiModCount ] = hMod ;
        g_uiModCount++ ;
    }
    return ( TRUE ) ;
}

UINT __stdcall GetLimitModuleCount ( void )
{
    return ( g_uiModCount ) ;
}

int __stdcall GetLimitModulesArray ( HMODULE * pahMod , UINT uiSize )
{
    int iRet ;

    __try
    {
        ASSERT ( FALSE == IsBadWritePtr ( pahMod ,
                                          uiSize * sizeof ( HMODULE ) ) ) ;
        if ( TRUE == IsBadWritePtr ( pahMod ,
                                     uiSize * sizeof ( HMODULE ) ) )
        {
            iRet = GLMA_BADPARAM ;
            __leave ;
        }

        if ( uiSize < g_uiModCount )
        {
            iRet = GLMA_BUFFTOOSMALL ;
            __leave ;
        }

        CopyMemory ( pahMod     ,
                     g_ahMod    ,
                     sizeof ( HMODULE ) * g_uiModCount ) ;

        iRet = GLMA_SUCCESS ;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        iRet = GLMA_FAILURE ;
    }
    return ( iRet ) ;
}

LONG __stdcall CrashHandlerExceptionFilter (EXCEPTION_POINTERS* pExPtrs)
{
    LONG lRet = EXCEPTION_CONTINUE_SEARCH ;
    
    // If the exception is an EXCEPTION_STACK_OVERFLOW, there isn't much
    // you can do because the stack is blown. If you try to do anything,
    // the odds are great that you'll just double-fault and bomb right
    // out of your exception filter. Although I don't recommend doing so,
    // you could play some games with the stack register and
    // manipulate it so that you could regain enough space to run these
    // functions. Of course, if you did change the stack register, you'd
    // have problems walking the stack.
    // I take the safe route and make some calls to OutputDebugString
    // here.  I still might double-fault, but because OutputDebugString
    // does very little on the stack (something like 8-16 bytes), it's
    // worth a shot. You can have your users download Mark Russinovich's
    // DebugView (www.sysinternals.com) so they can at least tell you
    // what they see.
    // The only problem is that I can't even be sure there's enough
    // room on the stack to convert the instruction pointer.
    // Fortunately, EXCEPTION_STACK_OVERFLOW doesn't happen very often.
    // You might be wondering why I don't call the new _resetstkoflw
    // function here.  This function is only called on fatal exceptions
    // so attempting to reset the stack will not do anything useful as
    // the application is going down.  The _resetstkoflw function is
    // only useful if you call it before you get here.
    __try
    {

        // Note that I still call your crash handler. I'm doing the logging
        // work here in case the blown stack kills your crash handler.
        if ( EXCEPTION_STACK_OVERFLOW ==
                               pExPtrs->ExceptionRecord->ExceptionCode )
        {
            OutputDebugString(_T("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
            OutputDebugString(_T("EXCEPTION_STACK_OVERFLOW occurred\n"));
            OutputDebugString(_T("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
        }

        if ( NULL != g_pfnCallBack )
        {

            // The symbol engine has to be initialized here so that
            // I can look up the base module information for the
            // crash address as well as get the symbol engine
            // ready.
            InitSymEng ( ) ;

            // Check the g_ahMod list.
            BOOL bCallIt = FALSE ;
            if ( 0 == g_uiModCount )
            {
                bCallIt = TRUE ;
            }
            else
            {
                HINSTANCE hBaseAddr = (HINSTANCE)
                    SymGetModuleBase64( GetCurrentProcess ( )         ,
                                       (DWORD64)pExPtrs->
                                            ExceptionRecord->
                                                      ExceptionAddress);
                if ( NULL != hBaseAddr )
                {
                    for ( UINT i = 0 ; i < g_uiModCount ; i ++ )
                    {
                        if ( hBaseAddr == g_ahMod[ i ] )
                        {
                            bCallIt = TRUE ;
                            break ;
                        }
                    }
                }
            }
            if ( TRUE == bCallIt )
            {
                // Check that the crash handler still exists in memory
                // before I call it. The user might have forgotten to
                // unregister, and the crash handler is invalid because
                // it got unloaded. If some other function loaded
                // back into the same address, however, there isn't much
                // I can do.
                ASSERT ( FALSE == IsBadCodePtr((FARPROC)g_pfnCallBack));
                if ( FALSE == IsBadCodePtr ( (FARPROC)g_pfnCallBack ) )
                {
                    lRet = g_pfnCallBack ( pExPtrs ) ;
                }
            }
            else
            {
                // Call the previous filter but only after it checks
                // out. I'm just being a little paranoid.
                ASSERT ( FALSE == IsBadCodePtr((FARPROC)g_pfnOrigFilt));
                if ( FALSE == IsBadCodePtr ( (FARPROC)g_pfnOrigFilt ) )
                {
                    lRet = g_pfnOrigFilt ( pExPtrs ) ;
                }
            }
            CleanupSymEng ( ) ;
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        lRet = EXCEPTION_CONTINUE_SEARCH ;
    }
    return ( lRet ) ;
}

/*//////////////////////////////////////////////////////////////////////
// EXCEPTION_POINTER Translation Functions Implementation
//////////////////////////////////////////////////////////////////////*/

LPCTSTR __stdcall GetFaultReason ( EXCEPTION_POINTERS * pExPtrs )
{
    ASSERT ( FALSE == IsBadReadPtr ( pExPtrs ,
                                     sizeof ( EXCEPTION_POINTERS ) ) ) ;
    if ( TRUE == IsBadReadPtr ( pExPtrs ,
                                sizeof ( EXCEPTION_POINTERS ) ) )
    {
        TRACE0 ( "Bad parameter to GetFaultReason\n" ) ;
        return ( NULL ) ;
    }

    // The variable that holds the return value
    LPCTSTR szRet ;

    __try
    {

        // Initialize the symbol engine in case it isn't initialized.
        InitSymEng ( ) ;

        // The current position in the buffer
        int iCurr = 0 ;
        // A temporary value holder. This holder keeps the stack usage
        // to a minimum.
        DWORD64 dwTemp ;

        iCurr += BSUGetModuleBaseName ( GetCurrentProcess ( ) ,
                                        NULL                  ,
                                        g_szBuff              ,
                                        BUFF_SIZE              ) ;

        iCurr += wsprintf ( g_szBuff + iCurr , _T ( " caused an " ) ) ;

        dwTemp = (DWORD_PTR)
            ConvertSimpleException(pExPtrs->ExceptionRecord->
                                                         ExceptionCode);

        if ( NULL != dwTemp )
        {
            iCurr += wsprintf ( g_szBuff + iCurr ,
                                _T ( "%s" )      ,
                                dwTemp            ) ;
        }
        else
        {
            iCurr += FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |
                                            FORMAT_MESSAGE_FROM_HMODULE,
                                    GetModuleHandle (_T("NTDLL.DLL"))  ,
                                    pExPtrs->ExceptionRecord->
                                                         ExceptionCode ,
                                    0                                  ,
                                    g_szBuff + iCurr                   ,
                                    BUFF_SIZE                          ,
                                    0                                 );
        }

        ASSERT ( iCurr < ( BUFF_SIZE - MAX_PATH ) ) ;

        iCurr += wsprintf ( g_szBuff + iCurr , _T ( " in module\r\n" ) ) ;

        dwTemp =
             SymGetModuleBase64( GetCurrentProcess ( ) ,
                                 (DWORD64)pExPtrs->ExceptionRecord->
                                                    ExceptionAddress ) ;
        ASSERT ( NULL != dwTemp ) ;

        if ( NULL == dwTemp )
        {
            iCurr += wsprintf ( g_szBuff + iCurr , _T ( "<UNKNOWN>" ) );
        }
        else
        {
            iCurr += BSUGetModuleBaseName ( GetCurrentProcess ( ) ,
                                            (HINSTANCE)dwTemp     ,
                                            g_szBuff + iCurr      ,
                                            BUFF_SIZE - iCurr      ) ;
        }

#ifdef _WIN64
        iCurr += wsprintf ( g_szBuff + iCurr    ,
                            _T ( " at %016X" )   ,
                            pExPtrs->ExceptionRecord->ExceptionAddress);
#else
        iCurr += wsprintf ( g_szBuff + iCurr                ,
                            _T ( " at %04X:%08X" )          ,
                            pExPtrs->ContextRecord->SegCs   ,
                            pExPtrs->ExceptionRecord->ExceptionAddress);
#endif

        ASSERT ( iCurr < ( BUFF_SIZE - 200 ) ) ;

        // Start looking up the exception address.
        PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)&g_stSymbol ;
        ZeroMemory ( pSym , SYM_BUFF_SIZE ) ;
        pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 ) ;
        pSym->MaxNameLength = SYM_BUFF_SIZE -
                                    sizeof ( IMAGEHLP_SYMBOL64 ) ;

        DWORD64 dwDisp ;
        if ( TRUE ==
              SymGetSymFromAddr64 ( GetCurrentProcess ( )             ,
                                    (DWORD64)pExPtrs->ExceptionRecord->
                                                     ExceptionAddress ,
                                     &dwDisp                          ,
                                     pSym                             ))
        {
            iCurr += wsprintf ( g_szBuff + iCurr , _T ( "," ) ) ;

            // Copy no more of the symbol information than there's
            // room for.  Remember, symbols names are ANSI!
            int iLen = lstrlenA ( pSym->Name ) ;
            // Make sure there's enough room for the longest symbol
            // and the displacement.
            if ( iLen > ( ( BUFF_SIZE - iCurr) -
                          ( MAX_SYM_SIZE + 50 )  ) )
            {
#ifdef UNICODE
                // Get some room on the stack to convert the string.
                TCHAR * pWideName = (TCHAR*)_alloca ( iLen + 1 ) ;
                
                BSUAnsi2Wide ( pSym->Name , pWideName , iLen + 1 ) ;
                
                lstrcpyn ( g_szBuff + iCurr      ,
                           pWideName             ,
                           BUFF_SIZE - iCurr - 1  ) ;
#else
                lstrcpyn ( g_szBuff + iCurr      ,
                           pSym->Name            ,
                           BUFF_SIZE - iCurr - 1  ) ;
#endif  // UNICODE
                // Gotta leave now
                szRet = g_szBuff ;
                __leave ;
            }
            else
            {
                if ( dwDisp > 0 )
                {
                    iCurr += wsprintf ( g_szBuff + iCurr ,
                                        k_NAMEDISPFMT    ,
                                        pSym->Name       ,
                                        dwDisp            ) ;
                }
                else
                {
                    iCurr += wsprintf ( g_szBuff + iCurr ,
                                        k_NAMEFMT        ,
                                        pSym->Name        ) ;
                }
            }
        }
        else
        {
            // If the symbol wasn't found, the source and line won't
            // be found either, so leave now.
            szRet = g_szBuff ;
            __leave ;
        }

        ASSERT ( iCurr < ( BUFF_SIZE - 200 ) ) ;

        // Look up the source file and line number.
        ZeroMemory ( &g_stLine , sizeof ( IMAGEHLP_LINE64 ) ) ;
        g_stLine.SizeOfStruct = sizeof ( IMAGEHLP_LINE64 ) ;
        
        DWORD dwLineDisp ;
        if ( TRUE ==
              SymGetLineFromAddr64 ( GetCurrentProcess ( )       ,
                                     (DWORD64)pExPtrs->
                                              ExceptionRecord->
                                                ExceptionAddress ,
                                     &dwLineDisp                 ,
                                     &g_stLine                    ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr , _T ( "," ) ) ;

            // Copy no more of the source file and line number
            // information than there's room for.
            int iLen = lstrlenA ( g_stLine.FileName ) ;
            if ( iLen > ( BUFF_SIZE - iCurr -
                          MAX_PATH - 50       ) )
            {
#ifdef UNICODE
                // Get some room on the stack to convert the string.
                TCHAR * pWideName = (TCHAR*)_alloca ( iLen + 1 ) ;
                
                BSUAnsi2Wide(g_stLine.FileName , pWideName , iLen + 1);
                
                lstrcpyn ( g_szBuff + iCurr      ,
                           pWideName             ,
                           BUFF_SIZE - iCurr - 1  ) ;
#else
                lstrcpyn ( g_szBuff + iCurr      ,
                           g_stLine.FileName     ,
                           BUFF_SIZE - iCurr - 1  ) ;

#endif  // UNICODE
                // Gotta leave now
                szRet = g_szBuff ;
                __leave ;
            }
            else
            {
                if ( dwLineDisp > 0 )
                {
                    iCurr += wsprintf ( g_szBuff + iCurr              ,
                                        k_FILELINEDISPFMT             ,
                                        g_stLine.FileName             ,
                                        g_stLine.LineNumber           ,
                                        dwLineDisp                     );
                }
                else
                {
                    iCurr += wsprintf ( g_szBuff + iCurr     ,
                                        k_FILELINEFMT        ,
                                        g_stLine.FileName    ,
                                        g_stLine.LineNumber   ) ;
                }
            }
        }
        szRet = g_szBuff ;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ASSERT ( !"Crashed in GetFaultReason" ) ;
        szRet = NULL ;
    }
    return ( szRet ) ;
}

// Helper function to isolate filling out the stack frame, which is CPU
// specific.
void FillInStackFrame ( PCONTEXT pCtx )
{
    // Initialize the STACKFRAME structure.
    ZeroMemory ( &g_stFrame , sizeof ( STACKFRAME64 ) ) ;

#ifdef _X86_
    g_stFrame.AddrPC.Offset       = pCtx->Eip    ;
    g_stFrame.AddrPC.Mode         = AddrModeFlat ;
    g_stFrame.AddrStack.Offset    = pCtx->Esp    ;
    g_stFrame.AddrStack.Mode      = AddrModeFlat ;
    g_stFrame.AddrFrame.Offset    = pCtx->Ebp    ;
    g_stFrame.AddrFrame.Mode      = AddrModeFlat ;
#elif  _AMD64_
    g_stFrame.AddrPC.Offset       = pCtx->Rip    ;
    g_stFrame.AddrPC.Mode         = AddrModeFlat ;
    g_stFrame.AddrStack.Offset    = pCtx->Rsp    ;
    g_stFrame.AddrStack.Mode      = AddrModeFlat ;
    g_stFrame.AddrFrame.Offset    = pCtx->Rbp    ;
    g_stFrame.AddrFrame.Mode      = AddrModeFlat ;
#elif  _IA64_
    #pragma message ( "IA64 NOT DEFINED!!" )
    #pragma FORCE COMPILATION ABORT!
#else
    #pragma message ( "CPU NOT DEFINED!!" )
    #pragma FORCE COMPILATION ABORT!
#endif
}

LPCTSTR BUGSUTIL_DLLINTERFACE __stdcall
             GetFirstStackTraceString ( DWORD                dwOpts  ,
                                        EXCEPTION_POINTERS * pExPtrs  )
{
    ASSERT ( FALSE == IsBadReadPtr ( pExPtrs                      ,
                                     sizeof ( EXCEPTION_POINTERS * ))) ;
    if ( TRUE == IsBadReadPtr ( pExPtrs                       ,
                                sizeof ( EXCEPTION_POINTERS *  ) ) )
    {
        TRACE0 ( "GetFirstStackTraceString - invalid pExPtrs!\n" ) ;
        return ( NULL ) ;
    }

    // Get the stack frame filled in.
    FillInStackFrame ( pExPtrs->ContextRecord ) ;

    // Copy over the exception pointers fields so I don't corrupt the
    // real one.
    g_stContext = *(pExPtrs->ContextRecord) ;

    return ( InternalGetStackTraceString ( dwOpts ) ) ;
}

LPCTSTR BUGSUTIL_DLLINTERFACE __stdcall
             GetNextStackTraceString ( DWORD                dwOpts  ,
                                       EXCEPTION_POINTERS * /*pExPtrs*/)
{
    // All error checking is in InternalGetStackTraceString.
    // Assume that GetFirstStackTraceString has already initialized the
    // stack frame information.
    return ( InternalGetStackTraceString ( dwOpts ) ) ;
}

BOOL __stdcall CH_ReadProcessMemory ( HANDLE                           ,
                                      DWORD64     qwBaseAddress        ,
                                      PVOID       lpBuffer             ,
                                      DWORD       nSize                ,
                                      LPDWORD     lpNumberOfBytesRead  )
{
    return ( ReadProcessMemory ( GetCurrentProcess ( )  ,
                                 (LPCVOID)qwBaseAddress ,
                                 lpBuffer               ,
                                 nSize                  ,
                                 lpNumberOfBytesRead     ) ) ;
}

// The internal function that does all the stack walking
LPCTSTR __stdcall InternalGetStackTraceString ( DWORD dwOpts )
{

    // The value that is returned
    LPCTSTR szRet ;
    // The module base address. I look this up right after the stack
    // walk to ensure that the module is valid.
    DWORD64 dwModBase ;

    __try
    {
        // Initialize the symbol engine in case it isn't initialized.
        InitSymEng ( ) ;

        // Note:  If the source file and line number functions are used,
        //        StackWalk can cause an access violation.
        BOOL bSWRet = StackWalk64 ( CH_MACHINE                         ,
                                    GetCurrentProcess ( )              ,
                                    GetCurrentThread ( )               ,
                                    &g_stFrame                         ,
                                    &g_stContext                       ,
                                    CH_ReadProcessMemory               ,
                                    SymFunctionTableAccess64           ,
                                    SymGetModuleBase64                 ,
                                    NULL                               );
        if ( ( FALSE == bSWRet ) || ( 0 == g_stFrame.AddrFrame.Offset ))
        {
            szRet = NULL ;
            __leave ;
        }

        // Before I get too carried away and start calculating
        // everything, I need to double-check that the address returned
        // by StackWalk really exists. I've seen cases in which
        // StackWalk returns TRUE but the address doesn't belong to
        // a module in the process.
        dwModBase = SymGetModuleBase64 ( GetCurrentProcess ( )   ,
                                         g_stFrame.AddrPC.Offset  ) ;
        if ( 0 == dwModBase )
        {
            szRet = NULL ;
            __leave ;
        }

        int iCurr = 0 ;

        // At a minimum, put in the address.
#ifdef _WIN64
        iCurr += wsprintf ( g_szBuff + iCurr        ,
                            _T ( "0x%016X" )         ,
                            g_stFrame.AddrPC.Offset  ) ;
#else
//        iCurr += wsprintf ( g_szBuff + iCurr        ,
//                            _T ( "%04X:%08X" )      ,
//                            g_stContext.SegCs       ,
//                            g_stFrame.AddrPC.Offset  ) ;
        // If we're writing out additional info, write it out
        // with SegCs, otherwise skip SegCs. 
        if ( dwOpts )
        {
            iCurr += wsprintf ( g_szBuff + iCurr        ,
                                _T ( "%04X:%08X" )      ,
                                g_stContext.SegCs       ,
                                g_stFrame.AddrPC.Offset  ) ;
        }
        else
        {
            iCurr += wsprintf ( g_szBuff + iCurr    ,
                                _T ( "%08X" )   ,
                                g_stFrame.AddrPC.Offset);
        }
#endif

        // Output the parameters?
        if ( GSTSO_PARAMS == ( dwOpts & GSTSO_PARAMS ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr          ,
                                k_PARAMFMTSTRING          ,
                                g_stFrame.Params[ 0 ]     ,
                                g_stFrame.Params[ 1 ]     ,
                                g_stFrame.Params[ 2 ]     ,
                                g_stFrame.Params[ 3 ]      ) ;
        }
        // Output the module name.
        if ( GSTSO_MODULE == ( dwOpts & GSTSO_MODULE ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr  , _T ( " " ) ) ;

            ASSERT ( iCurr < ( BUFF_SIZE - MAX_PATH ) ) ;
            iCurr += BSUGetModuleBaseName ( GetCurrentProcess ( ) ,
                                            (HINSTANCE)dwModBase  ,
                                            g_szBuff + iCurr      ,
                                            BUFF_SIZE - iCurr      ) ;
        }

        ASSERT ( iCurr < ( BUFF_SIZE - MAX_PATH ) ) ;
        DWORD64 dwDisp ;

        // Output the symbol name?
        if ( GSTSO_SYMBOL == ( dwOpts & GSTSO_SYMBOL ) )
        {

            // Start looking up the exception address.
            PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)&g_stSymbol ;
            ZeroMemory ( pSym , SYM_BUFF_SIZE ) ;
            pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 ) ;
            pSym->MaxNameLength = SYM_BUFF_SIZE -
                                  sizeof ( IMAGEHLP_SYMBOL64 ) ;
            pSym->Address = g_stFrame.AddrPC.Offset ;

            if ( TRUE ==
                  SymGetSymFromAddr64 ( GetCurrentProcess ( )      ,
                                        g_stFrame.AddrPC.Offset    ,
                                        &dwDisp                    ,
                                        pSym                        ) )
            {
                if ( dwOpts & ~GSTSO_SYMBOL )
                {
                    iCurr += wsprintf ( g_szBuff + iCurr , _T ( "," ));
                }

                // Copy no more symbol information than there's room
                // for.  Symbols are ANSI
                int iLen = lstrlenA ( pSym->Name ) ;
                if ( iLen > ( BUFF_SIZE - iCurr -
                            ( MAX_SYM_SIZE + 50 ) ) )
                {
#ifdef UNICODE
                    // Get some room on the stack to convert the string.
                    wchar_t * pWideName = (TCHAR*)_alloca ( iLen + 1 ) ;
                
                    BSUAnsi2Wide ( pSym->Name , pWideName , iLen + 1 ) ;
                
                    lstrcpyn ( g_szBuff + iCurr      ,
                               pWideName             ,
                               BUFF_SIZE - iCurr - 1  ) ;
#else
                    lstrcpyn ( g_szBuff + iCurr      ,
                               pSym->Name            ,
                               BUFF_SIZE - iCurr - 1  ) ;

#endif  // UNICODE
                    // Gotta leave now
                    szRet = g_szBuff ;
                    __leave ;
                }
                else
                {
                    if ( dwDisp > 0 )
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr    ,
                                            k_NAMEDISPFMT       ,
                                            pSym->Name          ,
                                            dwDisp               ) ;
                    }
                    else
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr ,
                                            k_NAMEFMT        ,
                                            pSym->Name        ) ;
                    }
                }
            }
            else
            {
                // If the symbol wasn't found, the source file and line
                // number won't be found either, so leave now.
                szRet = g_szBuff ;
                __leave ;
            }

        }

        ASSERT ( iCurr < ( BUFF_SIZE - MAX_PATH ) ) ;

        // Output the source file and line number information?
        if ( GSTSO_SRCLINE == ( dwOpts & GSTSO_SRCLINE ) )
        {
            ZeroMemory ( &g_stLine , sizeof ( IMAGEHLP_LINE64 ) ) ;
            g_stLine.SizeOfStruct = sizeof ( IMAGEHLP_LINE64 ) ;

            DWORD dwLineDisp ;
            if ( TRUE == SymGetLineFromAddr64 ( GetCurrentProcess ( )  ,
                                                g_stFrame.AddrPC.Offset,
                                                &dwLineDisp            ,
                                                &g_stLine             ))
            {
                if ( dwOpts & ~GSTSO_SRCLINE )
                {
                    iCurr += wsprintf ( g_szBuff + iCurr , _T ( "," ));
                }

                // Copy no more of the source file and line number
                // information than there's room for.
                int iLen = lstrlenA ( g_stLine.FileName ) ;
                if ( iLen > ( BUFF_SIZE - iCurr -
                            ( MAX_PATH + 50     ) ) )
                {
#ifdef UNICODE
                    // Get some room on the stack to convert the string.
                    TCHAR * pWideName = (TCHAR*)_alloca ( iLen + 1 ) ;
                
                    BSUAnsi2Wide ( g_stLine.FileName ,
                                   pWideName         ,
                                   iLen + 1           ) ;
                
                    lstrcpyn ( g_szBuff + iCurr      ,
                            pWideName             ,
                            BUFF_SIZE - iCurr - 1  ) ;
#else
                    lstrcpyn ( g_szBuff + iCurr      ,
                               g_stLine.FileName     ,
                               BUFF_SIZE - iCurr - 1  ) ;

#endif
                    // Gotta leave now
                    szRet = g_szBuff ;
                    __leave ;
                }
                else
                {
                    if ( dwLineDisp > 0 )
                    {
                        iCurr += wsprintf( g_szBuff + iCurr     ,
                                           k_FILELINEDISPFMT    ,
                                           g_stLine.FileName    ,
                                           g_stLine.LineNumber  ,
                                           dwLineDisp            ) ;
                    }
                    else
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr    ,
                                            k_FILELINEFMT       ,
                                            g_stLine.FileName   ,
                                            g_stLine.LineNumber  ) ;
                    }
                }
            }
        }

        szRet = g_szBuff ;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ASSERT ( !"Crashed in InternalGetStackTraceString" ) ;
        szRet = NULL ;
    }
    return ( szRet ) ;
}

LPCTSTR __stdcall GetRegisterString ( EXCEPTION_POINTERS * pExPtrs )
{
    // Check the parameter.
    ASSERT ( FALSE == IsBadReadPtr ( pExPtrs                      ,
                                     sizeof ( EXCEPTION_POINTERS ) ) ) ;
    if ( TRUE == IsBadReadPtr ( pExPtrs                      ,
                                sizeof ( EXCEPTION_POINTERS ) ) )
    {
        TRACE0 ( "GetRegisterString - invalid pExPtrs!\n" ) ;
        return ( NULL ) ;
    }

#ifdef _X86_
    // This call puts 48 bytes on the stack, which could be a problem if
    // the stack is blown.
    wsprintf(g_szBuff ,
             _T ("EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X  ESI=%08X\r\n")\
             _T ("EDI=%08X  EBP=%08X  ESP=%08X  EIP=%08X  FLG=%08X\r\n")\
             _T ("CS=%04X   DS=%04X  SS=%04X  ES=%04X   ")\
             _T ("FS=%04X  GS=%04X" ) ,
                 pExPtrs->ContextRecord->Eax      ,
                 pExPtrs->ContextRecord->Ebx      ,
                 pExPtrs->ContextRecord->Ecx      ,
                 pExPtrs->ContextRecord->Edx      ,
                 pExPtrs->ContextRecord->Esi      ,
                 pExPtrs->ContextRecord->Edi      ,
                 pExPtrs->ContextRecord->Ebp      ,
                 pExPtrs->ContextRecord->Esp      ,
                 pExPtrs->ContextRecord->Eip      ,
                 pExPtrs->ContextRecord->EFlags   ,
                 pExPtrs->ContextRecord->SegCs    ,
                 pExPtrs->ContextRecord->SegDs    ,
                 pExPtrs->ContextRecord->SegSs    ,
                 pExPtrs->ContextRecord->SegEs    ,
                 pExPtrs->ContextRecord->SegFs    ,
                 pExPtrs->ContextRecord->SegGs     ) ;
#elif _AMD64_
    wsprintf ( g_szBuff ,
        _T ("RAX=%016X  RBX=%016X  RCX=%016X  RDX=%016X  RSI=%016X\r\n")\
        _T ("RDI=%016X  RBP=%016X  RSP=%016X  RIP=%016X  FLG=%016X\r\n")\
        _T (" R8=%016X   R9=%016X  R10=%016X  R11=%016X  R12=%016X\r\n")\
        _T ("R13=%016X  R14=%016X  R15=%016X" ) ,
        pExPtrs->ContextRecord->Rax      ,
        pExPtrs->ContextRecord->Rbx      ,
        pExPtrs->ContextRecord->Rcx      ,
        pExPtrs->ContextRecord->Rdx      ,
        pExPtrs->ContextRecord->Rsi      ,
        pExPtrs->ContextRecord->Rdi      ,
        pExPtrs->ContextRecord->Rbp      ,
        pExPtrs->ContextRecord->Rsp      ,
        pExPtrs->ContextRecord->Rip      ,
        pExPtrs->ContextRecord->EFlags   ,
        pExPtrs->ContextRecord->R8       ,
        pExPtrs->ContextRecord->R9       ,
        pExPtrs->ContextRecord->R10      ,
        pExPtrs->ContextRecord->R11      ,
        pExPtrs->ContextRecord->R12      ,
        pExPtrs->ContextRecord->R13      ,
        pExPtrs->ContextRecord->R14      ,
        pExPtrs->ContextRecord->R15       ) ;
#elif _IA64_
    #pragma message ( "IA64 NOT DEFINED!!" )
    #pragma FORCE COMPILATION ABORT!
#else
    #pragma message ( "CPU NOT DEFINED!!" )
    #pragma FORCE COMPILATION ABORT!
#endif

    return ( g_szBuff ) ;

}

LPCTSTR ConvertSimpleException ( DWORD dwExcept )
{
    switch ( dwExcept )
    {
        case EXCEPTION_ACCESS_VIOLATION         :
            return ( _T ( "EXCEPTION_ACCESS_VIOLATION" ) ) ;
        break ;

        case EXCEPTION_DATATYPE_MISALIGNMENT    :
            return ( _T ( "EXCEPTION_DATATYPE_MISALIGNMENT" ) ) ;
        break ;

        case EXCEPTION_BREAKPOINT               :
            return ( _T ( "EXCEPTION_BREAKPOINT" ) ) ;
        break ;

        case EXCEPTION_SINGLE_STEP              :
            return ( _T ( "EXCEPTION_SINGLE_STEP" ) ) ;
        break ;

        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED    :
            return ( _T ( "EXCEPTION_ARRAY_BOUNDS_EXCEEDED" ) ) ;
        break ;

        case EXCEPTION_FLT_DENORMAL_OPERAND     :
            return ( _T ( "EXCEPTION_FLT_DENORMAL_OPERAND" ) ) ;
        break ;

        case EXCEPTION_FLT_DIVIDE_BY_ZERO       :
            return ( _T ( "EXCEPTION_FLT_DIVIDE_BY_ZERO" ) ) ;
        break ;

        case EXCEPTION_FLT_INEXACT_RESULT       :
            return ( _T ( "EXCEPTION_FLT_INEXACT_RESULT" ) ) ;
        break ;

        case EXCEPTION_FLT_INVALID_OPERATION    :
            return ( _T ( "EXCEPTION_FLT_INVALID_OPERATION" ) ) ;
        break ;

        case EXCEPTION_FLT_OVERFLOW             :
            return ( _T ( "EXCEPTION_FLT_OVERFLOW" ) ) ;
        break ;

        case EXCEPTION_FLT_STACK_CHECK          :
            return ( _T ( "EXCEPTION_FLT_STACK_CHECK" ) ) ;
        break ;

        case EXCEPTION_FLT_UNDERFLOW            :
            return ( _T ( "EXCEPTION_FLT_UNDERFLOW" ) ) ;
        break ;

        case EXCEPTION_INT_DIVIDE_BY_ZERO       :
            return ( _T ( "EXCEPTION_INT_DIVIDE_BY_ZERO" ) ) ;
        break ;

        case EXCEPTION_INT_OVERFLOW             :
            return ( _T ( "EXCEPTION_INT_OVERFLOW" ) ) ;
        break ;

        case EXCEPTION_PRIV_INSTRUCTION         :
            return ( _T ( "EXCEPTION_PRIV_INSTRUCTION" ) ) ;
        break ;

        case EXCEPTION_IN_PAGE_ERROR            :
            return ( _T ( "EXCEPTION_IN_PAGE_ERROR" ) ) ;
        break ;

        case EXCEPTION_ILLEGAL_INSTRUCTION      :
            return ( _T ( "EXCEPTION_ILLEGAL_INSTRUCTION" ) ) ;
        break ;

        case EXCEPTION_NONCONTINUABLE_EXCEPTION :
            return ( _T ( "EXCEPTION_NONCONTINUABLE_EXCEPTION" ) ) ;
        break ;

        case EXCEPTION_STACK_OVERFLOW           :
            return ( _T ( "EXCEPTION_STACK_OVERFLOW" ) ) ;
        break ;

        case EXCEPTION_INVALID_DISPOSITION      :
            return ( _T ( "EXCEPTION_INVALID_DISPOSITION" ) ) ;
        break ;

        case EXCEPTION_GUARD_PAGE               :
            return ( _T ( "EXCEPTION_GUARD_PAGE" ) ) ;
        break ;

        case EXCEPTION_INVALID_HANDLE           :
            return ( _T ( "EXCEPTION_INVALID_HANDLE" ) ) ;
        break ;

        case 0xE06D7363                         :
            return ( _T ( "Microsoft C++ Exception" ) ) ;
        break ;

        default :
            return ( NULL ) ;
        break ;
    }
}

// Initializes the symbol engine if needed
void InitSymEng ( void )
{
    if ( FALSE == g_bSymEngInit )
    {
        // Set up the symbol engine.
        DWORD dwOpts = SymGetOptions ( ) ;

        // Turn on line loading.
        SymSetOptions ( dwOpts                |
                        SYMOPT_LOAD_LINES      ) ;

        // Force the invade process flag on.
        BOOL bRet = SymInitialize ( GetCurrentProcess ( ) ,
                                    NULL                    ,
                                    TRUE                     ) ;
        ASSERT ( TRUE == bRet ) ;
        g_bSymEngInit = bRet ;
    }
}

// Cleans up the symbol engine if needed
void CleanupSymEng ( void )
{
    if ( TRUE == g_bSymEngInit )
    {
        VERIFY ( SymCleanup ( GetCurrentProcess ( ) ) ) ;
        g_bSymEngInit = FALSE ;
    }
}

