// Modified version of Microsoft's /GS buffer overrun security check support.
// Microsoft's support allows client programmers to choose how the user is
// notified (via dialog, writing to a log, etc.) but always forces the program
// to exit. While in many circumstances allowing the program to continue will
// result in the program crashing (and the potential for security exploits) that
// is not always the case. We've already found one case where /GS was forcing
// Eudora to exit, but it was a benign enough buffer overflow that it had never
// caused any problems previously (it overflowed a buffer by exactly one byte).
//
// This file has been modified to override Microsoft's default /GS support
// allowing buffer overflows to be detected without requiring that Eudora exit.
// Eudora's handler function chooses whether or not Eudora will exit.
//
// As best I can tell, the security cookie and security checking code must be
// compiled into each module (exe or dll) in order to override Microsoft's
// default /GS support as provided in the CRT.

/***
*seccook.c - checks buffer overrun security cookie
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines compiler helper __security_check_cookie, used by the /GS
*       compile switch to detect local buffer variable overrun bugs/attacks.
*
*       When compiling /GS, the compiler injects code to detect when a local
*       array variable has been overwritten, potentially overwriting the
*       return address (on machines like x86 where the return address is on
*       the stack).  A local variable is allocated directly before the return
*       address and initialized on entering the function.  When exiting the
*       function, the compiler inserts code to verify that the local variable
*       has not been modified.  If it has, then an error reporting routine
*       is called.
*
*       NOTE: The ATLMINCRT library includes a version of this file.  If any
*       changes are made here, they should be duplicated in the ATL version.
*
*******************************************************************************/

#include <..\crt\src\sect_attribs.h>
//#include <..\crt\src\internal.h>
#include <windows.h>
#include <stdlib.h>

#pragma warning(disable : 4100)


/*
 * The global security cookie.  This name is known to the compiler.
 */
extern DWORD_PTR __security_cookie;

/*
 * Trigger initialization of the global security cookie on program startup.
 * Force initialization before any #pragma init_seg() inits by using .CRT$XCAA
 * as the startup funcptr section.
 */

typedef void (__cdecl *_PVFV)(void);	// from CRT's internal.h
#pragma data_seg(".CRT$XCAA")
extern void __cdecl __security_init_cookie(void);
static _CRTALLOC(".CRT$XCAA") _PVFV init_cookie = __security_init_cookie;
#pragma data_seg()

static void __cdecl report_failure(void);

extern void __cdecl __qc_security_error_handler();

/***
*__security_check_cookie(cookie) - check for buffer overrun
*
*Purpose:
*       Compiler helper.  Check if a local copy of the security cookie still
*       matches the global value.  If not, then report the fatal error.
*
*       The actual reporting is split out into static helper report_failure,
*       since the cookie check routine must be minimal code that preserves
*       any registers used in returning the callee's result.
*
*Entry:
*       DWORD_PTR cookie - local security cookie to check
*
*Exit:
*       Returns immediately if the local cookie matches the global version.
*       Otherwise, calls the failure reporting handler and exits.
*
*Exceptions:
*
*******************************************************************************/

#ifndef _M_IX86

void __fastcall __security_check_cookie(DWORD_PTR cookie)
{
    /* Immediately return if the local cookie is OK. */
    if (cookie == __security_cookie)
        return;

    /* Report the failure */
    report_failure();
}

#else  /* _M_IX86 */

void __declspec(naked) __fastcall __security_check_cookie(DWORD_PTR cookie)
{
    /* x86 version written in asm to preserve all regs */
    __asm {
        cmp ecx, __security_cookie
        jne failure
        ret
failure:
        jmp report_failure
    }
}

#endif  /* _M_IX86 */

static void __cdecl report_failure(void)
{
    /* Report the failure */
	// Note that catching crash exceptions here interferes with Eudora's
	// crash handling mechanism. I believe that's acceptable since we
	// should only have an exception if we have already written (or
	// attempted to write) a EudoraCrashDump.dmp file. If the setting
	// is such that we don't do anything on a buffer overflow, then
	// it's unlikely that we would crash inside of __qc_security_error_handler.
    __try {
        // Call the __qc_security_error_handler instead of __security_error_handler
		// so that we don't have to fight to override CRT's exported function.
		__qc_security_error_handler();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ExitProcess(3);
    }
}

