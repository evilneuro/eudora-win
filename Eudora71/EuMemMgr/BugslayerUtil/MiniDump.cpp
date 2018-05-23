/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

/*//////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////*/
#include "pch.h"
#include "BugslayerUtil.h"
// The project internal header file.
#include "Internal.h"
// The minidump definitions.
#include "DBGHELP_MINDUMP.h"

/*//////////////////////////////////////////////////////////////////////
// File Specific Defines, Typdefs, Constants, and Structs
//////////////////////////////////////////////////////////////////////*/
// The typedef for the MiniDumpWriteDump function.
typedef BOOL
(WINAPI * PFNMINIDUMPWRITEDUMP)(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN HANDLE hFile,
    IN MINIDUMP_TYPE DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
    IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
    IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL   ) ;

// Ye ol' DBGHELP.DLL name.
static const TCHAR * k_DBGHELPDLLNAME = _T ( "DBGHELP.DLL" ) ;
// The function name for MiniDumpWriteDump.  Note, this is ANSI as
// that's what GetProcAddress wants.
static const char * k_MINIDUMPWRITEDUMP = "MiniDumpWriteDump" ;

// The distances (in bytes) from a return address to the call
// instruction for near and far calls.  These are used in the
// CalculateBeginningOfCallInstruction function.
#define k_CALLNEARBACK  5
#define k_CALLFARBACK   6

// The common prolog for the naked functions,
// SnapCurrentProcessMiniDumpA and SnapCurrentProcessMiniDumpW.
#define SNAPPROLOG(Cntx)                                               \
__asm PUSH  EBP                   /* Save EBP explictly.            */ \
__asm MOV   EBP , ESP             /* Move the stack.                */ \
__asm SUB   ESP , __LOCAL_SIZE    /* Space for the local variables. */ \
/* Copy over all the easy current registers values. */                 \
__asm MOV   Cntx.Eax , EAX                                             \
__asm MOV   Cntx.Ebx , EBX                                             \
__asm MOV   Cntx.Ecx , ECX                                             \
__asm MOV   Cntx.Edx , EDX                                             \
__asm MOV   Cntx.Edi , EDI                                             \
__asm MOV   Cntx.Esi , ESI                                             \
/* Zero put the whole EAX register and just copy the segments into  */ \
/* the lower word.  This avoids leaving the upper word uninitialized*/ \
/* as the context segment registers are really 32-bit values.       */ \
__asm XOR   EAX , EAX                                                  \
__asm MOV   AX , GS                                                    \
__asm MOV   Cntx.SegGs , EAX                                           \
__asm MOV   AX , FS                                                    \
__asm MOV   Cntx.SegFs , EAX                                           \
__asm MOV   AX , ES                                                    \
__asm MOV   Cntx.SegEs , EAX                                           \
__asm MOV   AX , DS                                                    \
__asm MOV   Cntx.SegDs , EAX                                           \
__asm MOV   AX , CS                                                    \
__asm MOV   Cntx.SegCs , EAX                                           \
__asm MOV   AX , SS                                                    \
__asm MOV   Cntx.SegSs , EAX                                           \
/* Get the previous EBP value. */                                      \
__asm MOV  EAX , DWORD PTR [EBP]                                       \
__asm MOV  Cntx.Ebp , EAX                                              \
/* Get the previous ESP value. */                                      \
__asm MOV  EAX , EBP                                                   \
/* Two DWORDs up from EBP is the previous stack address. */            \
__asm ADD  EAX , 8                                                     \
__asm MOV  Cntx.Esp , EAX                                              \
/* Save changed registers. */                                          \
__asm PUSH ESI                                                         \
__asm PUSH EDI                                                         \
__asm PUSH EBX                                                         \
__asm PUSH ECX                                                         \
__asm PUSH EDX
    
// The common epilog for the naked functions,
// SnapCurrentProcessMiniDumpA and SnapCurrentProcessMiniDumpW.
#define SNAPEPILOG(eRetVal)                                            \
__asm POP     EDX             /* Restore saved registers.  */          \
__asm POP     ECX                                                      \
__asm POP     EBX                                                      \
__asm POP     EDI                                                      \
__asm POP     ESI                                                      \
__asm MOV     EAX , eRetVal   /* Set the return value.      */         \
__asm MOV     ESP , EBP       /* Restore the stack pointer. */         \
__asm POP     EBP             /* Restore the frame pointer. */         \
__asm RET                     /* Return to caller.          */

// Just a wrapper to do the context copy.
#define COPYKEYCONTEXTREGISTERS(stFinalCtx,stInitialCtx)               \
stFinalCtx.Eax   = stInitialCtx.Eax   ;                                \
stFinalCtx.Ebx   = stInitialCtx.Ebx   ;                                \
stFinalCtx.Ecx   = stInitialCtx.Ecx   ;                                \
stFinalCtx.Edx   = stInitialCtx.Edx   ;                                \
stFinalCtx.Edi   = stInitialCtx.Edi   ;                                \
stFinalCtx.Esi   = stInitialCtx.Esi   ;                                \
stFinalCtx.SegGs = stInitialCtx.SegGs ;                                \
stFinalCtx.SegFs = stInitialCtx.SegFs ;                                \
stFinalCtx.SegEs = stInitialCtx.SegEs ;                                \
stFinalCtx.SegDs = stInitialCtx.SegDs ;                                \
stFinalCtx.SegCs = stInitialCtx.SegCs ;                                \
stFinalCtx.SegSs = stInitialCtx.SegSs ;                                \
stFinalCtx.Ebp   = stInitialCtx.Ebp   ;                                \
stFinalCtx.Esp   = stInitialCtx.Esp

/*//////////////////////////////////////////////////////////////////////
// File Specific Prototypes
//////////////////////////////////////////////////////////////////////*/
// Does the work to calculate the beginning of a call instruction given
// a return address.
BOOL CalculateBeginningOfCallInstruction ( UINT_PTR & dwRetAddr ) ;

/*//////////////////////////////////////////////////////////////////////
// File Specific Globals
//////////////////////////////////////////////////////////////////////*/
// The MiniDumpWriteDump function.  I don't want to link to this
// directly as it requires a version if DBGHELP.DLL that people may not
// have on their machines.
static PFNMINIDUMPWRITEDUMP g_pfnMDWD = NULL ;

// The last error value for IsMiniDumpFunctionAvailible so I don't have
// to go through doing lookup operations all over again.
static BSUMDRET g_eIMDALastError = eINVALID_ERROR ;

/*//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////*/

BOOL BUGSUTIL_DLLINTERFACE __stdcall IsMiniDumpFunctionAvailable ( void)
{
    // If this is the first time through, always do it.
    if ( NULL == g_pfnMDWD )
    {
        // Find DBGHELP.DLL in memory.
        HINSTANCE hInstDBGHELP = GetModuleHandle ( k_DBGHELPDLLNAME ) ;
        // BugslayerUtil.DLL has it in memory, but someone might excise
        // this file for their own purposes so try and load it.
        if ( NULL == hInstDBGHELP )
        {
            hInstDBGHELP = LoadLibrary ( k_DBGHELPDLLNAME ) ;
        }
        if ( NULL != hInstDBGHELP )
        {
            // At least I have DBGHELP.DLL's handle.  Get the exported
            // function.
            g_pfnMDWD = (PFNMINIDUMPWRITEDUMP)
                 GetProcAddress ( hInstDBGHELP , k_MINIDUMPWRITEDUMP ) ;


            if ( NULL != g_pfnMDWD )
            {
                // It's good so set the last error for this function.
                g_eIMDALastError = eDUMP_SUCCEEDED ;
            }
            else
            {
                // Ain't got the export.
                g_eIMDALastError = eDBGHELP_MISSING_EXPORTS ;
            }

        }
        else
        {
            // Can't find DBGHELP.DLL!  Save this for the
            // CreateCurrentProcessCrashDump function to return.
            g_eIMDALastError = eDBGHELP_NOT_FOUND ;
        }
    }
    // If g_pfnMDWD is not NULL, I found it.
    return ( NULL != g_pfnMDWD ) ;
}

BSUMDRET BUGSUTIL_DLLINTERFACE __stdcall
    CreateCurrentProcessCrashDumpA ( MINIDUMP_TYPE        eType      ,
                                     char *               szFileName ,
                                     DWORD                dwThread   ,
                                     EXCEPTION_POINTERS * pExceptInfo )
{
    // Check the string parameter because I am paranoid.
    ASSERT ( FALSE == IsBadStringPtrA ( szFileName , MAX_PATH ) ) ;
    if ( TRUE == IsBadStringPtrA ( szFileName , MAX_PATH ) )
    {
        return ( eBAD_PARAM ) ;
    }

    // The return value.
    BSUMDRET eRetVal = eDUMP_SUCCEEDED ;

    // Allocate enough space to hold the converted string.
    int iLen = ( lstrlenA ( szFileName ) + 1 ) * sizeof ( wchar_t ) ;
    wchar_t * pWideFileName = (wchar_t*)
                               HeapAlloc ( GetProcessHeap ( )         ,
                                           HEAP_GENERATE_EXCEPTIONS |
                                             HEAP_ZERO_MEMORY         ,
                                           iLen                       );

    int iRet = BSUAnsi2Wide ( szFileName , pWideFileName , iLen ) ;
    ASSERT ( iRet != 0 ) ;
    if ( iRet != 0 )
    {
        // The conversion worked, call the wide function.
        eRetVal = CreateCurrentProcessCrashDumpW ( eType         ,
                                                   pWideFileName ,
                                                   dwThread      ,
                                                   pExceptInfo    ) ;
    }
    else
    {
        eRetVal = eBAD_PARAM ;
    }

    if ( NULL != pWideFileName )
    {
        HeapFree ( GetProcessHeap ( ) , 0 , pWideFileName ) ;
    }

    return ( eRetVal ) ;
}


BSUMDRET BUGSUTIL_DLLINTERFACE __stdcall
    CreateCurrentProcessCrashDumpW ( MINIDUMP_TYPE        eType      ,
                                     wchar_t *            szFileName ,
                                     DWORD                dwThread   ,
                                     EXCEPTION_POINTERS * pExceptInfo )
{
    // Holds the return value.
    BSUMDRET eReturnVal = eOPEN_DUMP_FAILED ;
    
    // Check the string parameter because I am paranoid.  I can't check
    // the eType as that might change in the future.
    ASSERT ( FALSE == IsBadStringPtrW ( szFileName , MAX_PATH ) ) ;
    if ( TRUE == IsBadStringPtrW ( szFileName , MAX_PATH ) )
    {
        return ( eBAD_PARAM ) ;
    }
    ASSERT ( FALSE ==
           IsBadReadPtr ( pExceptInfo , sizeof ( EXCEPTION_POINTERS*)));
    if ( TRUE ==
        IsBadReadPtr ( pExceptInfo , sizeof ( EXCEPTION_POINTERS*)))
    {
        return ( eBAD_PARAM ) ;
    }
    ASSERT ( 0 != dwThread ) ;
    if ( 0 == dwThread )
    {
        return ( eBAD_PARAM ) ;
    }

    // Have I even tried to get the exported MiniDumpWriteDump function
    // yet?
    if ( ( NULL == g_pfnMDWD ) && ( eINVALID_ERROR == g_eIMDALastError))
    {
        if ( FALSE == IsMiniDumpFunctionAvailable ( ) )
        {
            return ( g_eIMDALastError ) ;
        }
    }
    // If the MiniDumpWriteDump function pointer is NULL, return
    // whatever was in g_eIMDALastError.
    if ( NULL == g_pfnMDWD )
    {
        return ( g_eIMDALastError ) ;
    }
    
    // Create the file first.
    HANDLE hFile = CreateFileW ( szFileName                     ,
                                 GENERIC_READ | GENERIC_WRITE   ,
                                 0                              ,
                                 NULL                           ,
                                 CREATE_ALWAYS                  ,
                                 FILE_ATTRIBUTE_NORMAL          ,
                                 NULL                            ) ;
    ASSERT ( INVALID_HANDLE_VALUE != hFile ) ;
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        MINIDUMP_EXCEPTION_INFORMATION   stMDEI ;

        stMDEI.ThreadId = dwThread ;
        stMDEI.ExceptionPointers = pExceptInfo ;
        stMDEI.ClientPointers = TRUE ;

        // Got the file open.  Write it.
        BOOL bRet = g_pfnMDWD ( GetCurrentProcess ( )   ,
                                GetCurrentProcessId ( ) ,
                                hFile                   ,
                                eType                   ,
                                &stMDEI                 ,
                                NULL                    ,
                                NULL                     ) ;
        ASSERT ( TRUE == bRet ) ;
        if ( TRUE == bRet )
        {
            eReturnVal = eDUMP_SUCCEEDED ;
        }
        else
        {
            // Oops.
            eReturnVal = eMINIDUMPWRITEDUMP_FAILED ;
        }

        // Close the open file.
        VERIFY ( CloseHandle ( hFile ) ) ;
    }
    else
    {
        // Could not open the file!
        eReturnVal = eOPEN_DUMP_FAILED ;
    }
    // Always save the last error value so I can set it in the original
    // thread.
    g_eIMDALastError = eReturnVal ;
    return ( eReturnVal ) ;
}

BSUMDRET CommonSnapCurrentProcessMiniDump ( MINIDUMP_TYPE eType      ,
                                            LPCWSTR       szDumpName ,
                                            PCONTEXT      pCtx        )
{
    // Assume the best.
    BSUMDRET eRet = eDUMP_SUCCEEDED ;

    // Have I even tried to get the exported MiniDumpWriteDump function
    // yet?
    if ( ( NULL == g_pfnMDWD ) && ( eINVALID_ERROR == g_eIMDALastError))
    {
        if ( FALSE == IsMiniDumpFunctionAvailable ( ) )
        {
            eRet = g_eIMDALastError ;
        }
    }
    // If the MiniDumpWriteDump function pointer is NULL, I'm done.
    if ( NULL == g_pfnMDWD )
    {
        eRet = g_eIMDALastError ;
    }
    
    if ( eDUMP_SUCCEEDED == eRet )
    {
        // Armed with the context at the time of the call to this
        // function, I can now look to actually writing the dump.  To
        // make everything work, I need to make it look like an
        // exception happened.  Hence, all this work to fill out the
        // MINIDUMP_EXCEPTION_INFORMATION structure.
                    
        EXCEPTION_RECORD stExRec ;
        EXCEPTION_POINTERS stExpPtrs ;
        MINIDUMP_EXCEPTION_INFORMATION stExInfo ;

        // Zero out all the individual values.
        ZeroMemory ( &stExRec , sizeof ( EXCEPTION_RECORD )) ;
        ZeroMemory ( &stExpPtrs , sizeof ( EXCEPTION_POINTERS ) ) ;
        ZeroMemory ( &stExInfo ,sizeof(MINIDUMP_EXCEPTION_INFORMATION));

        // Set the exception address to the start of the CALL
        // instruction.  Interestingly, I found I didn't have to set the
        // exception code.  When you open up a .DMP file created with
        // this code in VS.NET, you'll see the exception code reported
        // as: 0x00000000: The operation completed successfully.
                    
        // warning C4312: 'type cast' : conversion from 'DWORD'
        // to 'PVOID' of greater size
        #pragma warning ( disable : 4312 )
        stExRec.ExceptionAddress = (PVOID)(pCtx->Eip) ;
        #pragma warning ( default : 4312 )

        // Set the exception pointers.
        stExpPtrs.ContextRecord = pCtx ;
        stExpPtrs.ExceptionRecord = &stExRec ;

        // Finally, set up the exception info structure.
        stExInfo.ThreadId = GetCurrentThreadId ( ) ;
        stExInfo.ClientPointers = TRUE ;
        stExInfo.ExceptionPointers = &stExpPtrs ;

        // Create the file to write.
        HANDLE hFile = CreateFile ( szDumpName                   ,
                                    GENERIC_READ | GENERIC_WRITE ,
                                    FILE_SHARE_READ              ,
                                    NULL                         ,
                                    CREATE_ALWAYS                ,
                                    FILE_ATTRIBUTE_NORMAL        ,
                                    NULL                          ) ;
        ASSERT ( INVALID_HANDLE_VALUE != hFile ) ;
        if ( INVALID_HANDLE_VALUE != hFile )
        {
            // Do the dump file.
            BOOL bRetVal = g_pfnMDWD ( GetCurrentProcess ( )   ,
                                       GetCurrentProcessId ( ) ,
                                       hFile                   ,
                                       eType                   ,
                                       &stExInfo               ,
                                       NULL                    ,
                                       NULL                     ) ;
            ASSERT ( TRUE == bRetVal ) ;
            if ( TRUE == bRetVal )
            {
                eRet = eDUMP_SUCCEEDED ;
            }
            else
            {
                eRet = eMINIDUMPWRITEDUMP_FAILED ;
            }
            // Close the file.
            VERIFY ( CloseHandle ( hFile ) ) ;
        }
        else
        {
            eRet = eOPEN_DUMP_FAILED ;
        }
    }
    return ( eRet ) ;
}

BSUMDRET __declspec ( naked )
            SnapCurrentProcessMiniDumpA ( MINIDUMP_TYPE eType      ,
                                          const char *  szDumpName  )
                                          
{
    // Where the registers coming into this function are stored.
    CONTEXT stInitialCtx ;
    // Where the final registers are stored.
    CONTEXT stFinalCtx ;
    // The return value.
    BSUMDRET    eRet ;
    // Boolean return value local.
    BOOL        bRetVal ;

    // Do the prolog.
    SNAPPROLOG ( stInitialCtx ) ;

    eRet = eDUMP_SUCCEEDED ;

    // Check the string parameter.
    ASSERT ( FALSE == IsBadStringPtrA ( szDumpName , MAX_PATH ) ) ;
    if ( TRUE == IsBadStringPtrA ( szDumpName , MAX_PATH ) )
    {
        eRet = eBAD_PARAM ;
    }
    if ( eDUMP_SUCCEEDED == eRet )
    {
        // Zero out the final context structure.
        ZeroMemory ( &stFinalCtx , sizeof ( CONTEXT ) ) ;
            
        // Inicate I want everything in the context.
        stFinalCtx.ContextFlags = CONTEXT_FULL                 |
                                    CONTEXT_CONTROL            |
                                    CONTEXT_DEBUG_REGISTERS    |
                                    CONTEXT_EXTENDED_REGISTERS |
                                    CONTEXT_FLOATING_POINT       ;
                                       
        // Get all the groovy context registers and such for this
        // thread.
        bRetVal = GetThreadContext ( GetCurrentThread ( ) ,&stFinalCtx);
        ASSERT ( TRUE == bRetVal ) ;
        if ( TRUE == bRetVal )
        {
            COPYKEYCONTEXTREGISTERS ( stFinalCtx , stInitialCtx ) ;

            // Get the return address and hunt down the call instruction
            // that got us into this function.  All the rest of the
            // registers are set up before the call so I'll ensure the
            // instruction pointer is set that way too.
            UINT_PTR dwRetAddr = (UINT_PTR)_ReturnAddress ( ) ;
            bRetVal = CalculateBeginningOfCallInstruction ( dwRetAddr );
            ASSERT ( TRUE == bRetVal ) ;
            if ( TRUE == bRetVal )
            {
                // Set the instruction pointer to the beginning of the
                // call instruction.
                stFinalCtx.Eip = (DWORD)dwRetAddr ;
                
                // Create the wide version of the filename.
                int iLen = ( lstrlenA ( szDumpName ) + 1 ) *
                                            sizeof ( wchar_t ) ;
                wchar_t * pWideFileName = (wchar_t*)
                                HeapAlloc ( GetProcessHeap ( )        ,
                                            HEAP_GENERATE_EXCEPTIONS |
                                               HEAP_ZERO_MEMORY       ,
                                            iLen                      );
                ASSERT ( NULL != pWideFileName ) ;
                
                int iRet = BSUAnsi2Wide ( szDumpName    ,
                                        pWideFileName ,
                                        iLen           ) ;
                ASSERT ( iRet > 0 ) ;
                if ( iRet > 0 )
                {
                    // Call the common function that does the actual
                    // write.
                    eRet =
                        CommonSnapCurrentProcessMiniDump (eType        ,
                                                          pWideFileName,
                                                          &stFinalCtx );
                    if ( NULL != pWideFileName )
                    {
                        HeapFree ( GetProcessHeap ( ) ,
                                   0                  ,
                                   pWideFileName       ) ;
                    }
                                                          
                }
                else
                {
                    eRet = eBAD_PARAM ;
                }
            }
            else
            {
                eRet = eGETTHREADCONTEXT_FAILED ;
            }
        }
    }
    // Do the epilog.
    SNAPEPILOG ( eRet ) ;
}


BSUMDRET __declspec ( naked )
            SnapCurrentProcessMiniDumpW ( MINIDUMP_TYPE eType      ,
                                          LPCWSTR       szDumpName  )
                                          
{
    // Where the registers coming into this function are stored.
    CONTEXT stInitialCtx ;
    // Where the final registers are stored.
    CONTEXT stFinalCtx ;
    // The return value.
    BSUMDRET    eRet ;
    // Boolean return value local.
    BOOL        bRetVal ;

    // Do the prolog.
    SNAPPROLOG ( stInitialCtx ) ;

    eRet = eDUMP_SUCCEEDED ;

    // Check the string parameter.
    ASSERT ( FALSE == IsBadStringPtr ( szDumpName , MAX_PATH ) ) ;
    if ( TRUE == IsBadStringPtr ( szDumpName , MAX_PATH ) )
    {
        eRet = eBAD_PARAM ;
    }
    
    if ( eDUMP_SUCCEEDED == eRet )
    {
        // Zero out the final context structure.
        ZeroMemory ( &stFinalCtx , sizeof ( CONTEXT ) ) ;
            
        // Inicate I want everything in the context.
        stFinalCtx.ContextFlags = CONTEXT_FULL                 |
                                    CONTEXT_CONTROL            |
                                    CONTEXT_DEBUG_REGISTERS    |
                                    CONTEXT_EXTENDED_REGISTERS |
                                    CONTEXT_FLOATING_POINT       ;
                                       
        // Get all the groovy context registers and such for this
        // thread.
        bRetVal = GetThreadContext ( GetCurrentThread ( ) ,&stFinalCtx);
        ASSERT ( TRUE == bRetVal ) ;
        if ( TRUE == bRetVal )
        {
            COPYKEYCONTEXTREGISTERS ( stFinalCtx , stInitialCtx ) ;
            
            // Get the return address and hunt down the call instruction
            // that got us into this function.  All the rest of the
            // registers are set up before the call so I'll ensure the
            // instruction pointer is set that way too.
            UINT_PTR dwRetAddr = (UINT_PTR)_ReturnAddress ( ) ;
            bRetVal = CalculateBeginningOfCallInstruction ( dwRetAddr );
            ASSERT ( TRUE == bRetVal ) ;
            if ( TRUE == bRetVal )
            {
                // Set the instruction pointer to the beginning of the
                // call instruction.
                stFinalCtx.Eip = (DWORD)dwRetAddr ;

                // Call the common function that does the actual write.
                eRet = CommonSnapCurrentProcessMiniDump ( eType       ,
                                                          szDumpName  ,
                                                          &stFinalCtx );
            }
            else
            {
                eRet = eGETTHREADCONTEXT_FAILED ;
            }
        }
    }
    // Do the epilog.
    SNAPEPILOG ( eRet ) ;
}

// I had to pull this out of SnapCurrentProcessMiniDumpA/W as it's naked
// so can't use SEH.
BOOL CalculateBeginningOfCallInstruction ( UINT_PTR & dwRetAddr )
{
    BOOL bRet = TRUE ;
    // Protect everything inside exception handling.  I need to be extra
    // careful here as I'm reading up the stack and could possibly bump
    // off the top.  As I don't want SnapCurrentProcessMiniDump whacking
    // the application when you call it, I've got to eat any possible
    // exception I could run into here.
    __try
    {
        BYTE * pBytes = (BYTE*)dwRetAddr ;

        if ( 0xE8 == *(pBytes - k_CALLNEARBACK) )
        {
            dwRetAddr -= k_CALLNEARBACK ;
        }
        else if ( 0xFF == *(pBytes - k_CALLFARBACK) )
        {
            dwRetAddr -= k_CALLFARBACK ;
        }
        else
        {
            bRet = FALSE ;
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        bRet = FALSE ;
    }
    return ( bRet ) ;
}
