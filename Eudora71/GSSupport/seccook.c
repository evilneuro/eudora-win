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
*seccook.c - defines buffer overrun security cookie
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines per-module global variable __security_cookie, which is used
*       by the /GS compile switch to detect local buffer variable overrun
*       bugs/attacks.
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
 * Initialize to a garbage non-zero value just in case we have a buffer overrun
 * in any code that gets run before __security_init_cookie() has a chance to
 * initialize the cookie to the final value.
 */

DWORD_PTR __security_cookie = DEFAULT_SECURITY_COOKIE;
