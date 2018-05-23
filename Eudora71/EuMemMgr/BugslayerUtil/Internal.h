/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/

#ifndef _INTERNAL_H
#define _INTERNAL_H

#include "SymbolEngine.h"

// The NT4 specific version of GetLoadedModules.
BOOL __stdcall NT4GetLoadedModules ( DWORD     dwPID        ,
                                     UINT      uiCount      ,
                                     HMODULE * paModArray   ,
                                     LPDWORD   pdwRealCount   ) ;

// The TOOLHELP32 specific version of GetLoadedModules.
BOOL __stdcall TLHELPGetLoadedModules ( DWORD     dwPID        ,
                                        UINT      uiCount      ,
                                        HMODULE * paModArray   ,
                                        LPDWORD   pdwRealCount   ) ;

// The NT version of GetModuleBaseName.
DWORD __stdcall NTGetModuleBaseNameW ( HANDLE  hProcess   ,
                                       HMODULE hModule    ,
                                       LPWSTR  lpBaseName ,
                                       DWORD   nSize       ) ;


// The TOOLHELP32 specific version of GetModuleFilenameEx
DWORD __stdcall TLHELPGetModuleFileNameEx ( DWORD     dwPID        ,
                                            HANDLE    hProcess     ,
                                            HMODULE   hModule      ,
                                            LPTSTR    szFilename   ,
                                            DWORD     nSize         ) ;

// The NT version of GetModuleFilenameEx.
DWORD __stdcall NTGetModuleFileNameEx ( DWORD     dwPID        ,
                                        HANDLE    hProcess     ,
                                        HMODULE   hModule      ,
                                        LPTSTR    szFilename   ,
                                        DWORD     nSize         ) ;

// The TOOLHELP32 specific version of GetThreadIds
BOOL __stdcall TLHELPGetProcessThreadIds ( DWORD   dwPID         ,
                                           UINT    uiCount       ,
                                           LPDWORD paThreadArray ,
                                           LPDWORD pdwRealCount   ) ;

// A helper function to get the import descriptor for a the specified
// module.
PIMAGE_IMPORT_DESCRIPTOR
                     GetNamedImportDescriptor ( HMODULE hModule     ,
                                                LPCSTR  szImportMod  ) ;

// A useful macro.
#define MakePtr( cast , ptr , AddValue ) \
                   (cast)( (DWORD_PTR)(ptr)+(DWORD_PTR)(AddValue))


#ifdef _DEBUG
// The function that initializes the MemStress system during DllMain's
// process attach.
BOOL InternalMemStressInitialize ( void ) ;
// The function that shuts down the MemStress system during DllMain's
// process detach.
BOOL InternalMemStressShutdown ( void ) ;
#endif  // _DEBUG

// Converts an error code into it's text.  This uses the DiagAssert
// module tracking list.  If it returns 0, the last error result does
// not have a corresponding message.
DWORD ConvertErrorToMessage ( DWORD   dwLastErr ,
                              TCHAR * szMsg     ,
                              DWORD   dwSize     ) ;
// Returns the current assertion options settings.
DWORD GetDiagAssertOptions ( void ) ;
// Returns the assertion file handle, if set.
HANDLE GetDiagAssertFile ( void ) ;
// Looks up all the symbol information.
DWORD ConvertAddress ( DWORD64 dwAddr    ,
                       LPTSTR  szOutBuff ,
                       LPCTSTR szCRLF     ) ;
// The function that does all the work to output to the event log.
BOOL OutputToEventLog ( LPCTSTR szMsg ) ;

// Initializes the common symbol engine.
void InitializeCommonSymbolEngine ( void ) ;
// My special version of SymGetModuleBase which will load modules
// into the symbol engine if they are not found.
DWORD64 __stdcall GetModBase ( HANDLE hProcess , DWORD64 dwAddr ) ;
// Calls the common symbol engine to enumerate locals.
BOOL CommonSymbolEngineEnumLocalVariables
                    ( PENUM_LOCAL_VARS_CALLBACK      pCallback     ,
                      int                            iExpandLevel  ,
                      BOOL                           bExpandArrays ,
                      PREAD_PROCESS_MEMORY_ROUTINE64 pReadMem      ,
                      LPSTACKFRAME64                 pFrame        ,
                      CONTEXT *                      pContext      ,
                      PVOID                          pUserContext   ) ;
// Grabs the common symbol engine synchronization lock.
void AcquireCommonSymEngLock ( void ) ;
// Releases the common symbol engine synchronization lock.
void ReleaseCommonSymEngLock ( void ) ;



// Returns the instance handle for BugslayerUtil.DLL.
HINSTANCE GetBSUInstanceHandle ( void ) ;


#endif  // _INTERNAL_H


