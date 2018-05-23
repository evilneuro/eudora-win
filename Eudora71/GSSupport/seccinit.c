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
*seccinit.c - initialize the global buffer overrun security cookie
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Define __security_init_cookie, which is called at startup to initialize
*       the global buffer overrun security cookie used by the /GS compile flag.
*
*       NOTE: The ATLMINCRT library includes a version of this file.  If any
*       changes are made here, they should be duplicated in the ATL version.
*
*******************************************************************************/

//#include <internal.h>
#include <windows.h>

// From CRT's internal.h
/*
 * Default value used for the global /GS security cookie
 */

#ifdef _WIN64
#define DEFAULT_SECURITY_COOKIE 0x2B992DDFA23249D6
#else  /* _WIN64 */
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E
#endif  /* _WIN64 */


/*
 * The global security cookie.  This name is known to the compiler.
 */
extern DWORD_PTR __security_cookie;

/*
 * Union to facilitate converting from FILETIME to unsigned __int64
 */
typedef union {
    unsigned __int64 ft_scalar;
    FILETIME ft_struct;
} FT;

/***
*__security_init_cookie(cookie) - init buffer overrun security cookie.
*
*Purpose:
*       Initialize the global buffer overrun security cookie which is used by
*       the /GS compile switch to detect overwrites to local array variables
*       the potentially corrupt the return address.  This routine is called
*       at EXE/DLL startup.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __security_init_cookie(void)
{
    DWORD_PTR cookie;
    FT systime;
    LARGE_INTEGER perfctr;

    /*
     * Do nothing if the global cookie has already been initialized.
     */

    if (__security_cookie != 0 && __security_cookie != DEFAULT_SECURITY_COOKIE)
        return;

    /*
     * Initialize the global cookie with an unpredictable value which is
     * different for each module in a process.  Combine a number of sources
     * of randomness.
     */

    GetSystemTimeAsFileTime(&systime.ft_struct);
#if !defined (_WIN64)
    cookie = systime.ft_struct.dwLowDateTime;
    cookie ^= systime.ft_struct.dwHighDateTime;
#else  /* !defined (_WIN64) */
    cookie = systime.ft_scalar;
#endif  /* !defined (_WIN64) */

    cookie ^= GetCurrentProcessId();
    cookie ^= GetCurrentThreadId();
    cookie ^= GetTickCount();

    QueryPerformanceCounter(&perfctr);
#if !defined (_WIN64)
    cookie ^= perfctr.LowPart;
    cookie ^= perfctr.HighPart;
#else  /* !defined (_WIN64) */
    cookie ^= perfctr.QuadPart;
#endif  /* !defined (_WIN64) */

    /*
     * Make sure the global cookie is never initialized to zero, since in that
     * case an overrun which sets the local cookie and return address to the
     * same value would go undetected.
     */

    __security_cookie = cookie ? cookie : DEFAULT_SECURITY_COOKIE;
}
