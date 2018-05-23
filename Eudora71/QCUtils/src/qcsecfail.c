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
// Here we provide alternate versions of the security failure functions
// __security_error_handler and _set_security_error_handler which have been
// renamed to __qc_security_error_handler and _qc_set_security_error_handler
// so that our versions don't conflict with the CRT's versions.

/***
*secfail.c - Report a /GS security check failure
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Define function used to report a security check failure, along with a
*       routine for registering a new handler.
*
*       Entrypoints:
*       __qc_security_error_handler
*       _qc_set_security_error_handler
*
*       NOTE: The ATLMINCRT library includes a version of this file.  If any
*       changes are made here, they should be duplicated in the ATL version.
*
*******************************************************************************/

#include "qcsecfail.h"
//#include <cruntime.h>
//#include <internal.h>
#include <windows.h>
#include <stdlib.h>
//#include <awint.h>
//#include <dbgint.h>

// From CRT's crtdbg.h
#define _CRT_ERROR          1

#ifndef _DEBUG

#define _RPT0(rptno, msg)

#else

#define _RPT0(rptno, msg) \
        _RPT_BASE((rptno, NULL, 0, NULL, "%s", msg))
#define _RPT_BASE(args) \
        (void) ((1 != _CrtDbgReport args) || \
                (_CrtDbgBreak(), 0))
_CRTIMP int __cdecl _CrtDbgReport(
        int,
        const char *,
        int,
        const char *,
        const char *,
        ...);
#define _CrtDbgBreak() __debugbreak()

#endif // _DEBUG


/*
 * User-registered failure reporting routine.
 */

static _qc_secerr_handler_func user_handler;

/*
 * Default messagebox string components
 */

#define PROGINTRO   "Program: "
#define DOTDOTDOT   "..."

#define BOXINTRO_0  "Unknown security failure detected!"
#define MSGTEXT_0   \
    "A security error of unknown cause has been detected which has\n"      \
    "corrupted the program's internal state.  The program cannot safely\n" \
    "continue execution and must now be terminated.\n"

#define BOXINTRO_1  "Buffer overrun detected!"
#define MSGTEXT_1   \
    "A buffer overrun has been detected which has corrupted the program's\n"  \
    "internal state.  The program cannot safely continue execution and must\n"\
    "now be terminated.\n"

#define MAXLINELEN  60 /* max length for line in message box */

/***
*__security_error_handler() - Report security error.
*
*Purpose:
*       A /GS security error has been detected.  If a user-registered failure
*       reporting function is available, call it, otherwise bring up a default
*       message box describing the problem and terminate the program.
*
*Exit:
*       Does not return.
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __qc_security_error_handler()
{
    /* Use user-registered handler if available. */
    if (user_handler != NULL) {
		// Note that catching crash exceptions here interferes with Eudora's
		// crash handling mechanism. I believe that's acceptable since we
		// should only have an exception if we have already written (or
		// attempted to write) a EudoraCrashDump.dmp file. If the setting
		// is such that we don't do anything on a buffer overflow, then
		// it's unlikely that we would crash inside of user_handler.
        __try {
            user_handler();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            /*
             * If user handler raises an exception, capture it and terminate
             * the program, since the EH stack may be corrupted above this
             * point.
             */
			_exit(3);
        }
    }
    else {
        char progname[MAX_PATH + 1];
        char * pch;
        char * outmsg;
        char * boxintro = BOXINTRO_1;
        char * msgtext = MSGTEXT_1;
        size_t subtextlen = sizeof(BOXINTRO_1) + sizeof(MSGTEXT_1);
		HWND   hWndParent = NULL;

        /*
         * In debug CRT, report error with ability to call the debugger.
         */
        _RPT0(_CRT_ERROR, msgtext);

        progname[MAX_PATH] = '\0';
        if (!GetModuleFileName(NULL, progname, MAX_PATH))
            strcpy(progname, "<program name unknown>");

        pch = progname;

        /* sizeof(PROGINTRO) includes the NULL terminator */
        if (sizeof(PROGINTRO) + strlen(progname) + 1 > MAXLINELEN)
        {
            pch += (sizeof(PROGINTRO) + strlen(progname) + 1) - MAXLINELEN;
            strncpy(pch, DOTDOTDOT, sizeof(DOTDOTDOT) - 1);
        }

        outmsg = (char *)_alloca(subtextlen - 1 + 2
                                 + sizeof(PROGINTRO) - 1
                                 + strlen(pch) + 2);

        strcpy(outmsg, boxintro);
        strcat(outmsg, "\n\n");
        strcat(outmsg, PROGINTRO);
        strcat(outmsg, pch);
        strcat(outmsg, "\n\n");
        strcat(outmsg, msgtext);

        hWndParent = GetActiveWindow();

        if (hWndParent != NULL)
            hWndParent = GetLastActivePopup(hWndParent);

		MessageBox(
            hWndParent,
            outmsg,
            "Eudora",
            MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);


		// Only exit when no user handler. Otherwise user handler is responsible
		// for choosing whether or not to exit.
		_exit(3);
    }
}

/***
*_set_security_error_handler(handler) - Register user handler
*
*Purpose:
*       Register a user failure reporting function.
*
*Entry:
*       _qc_secerr_handler_func handler - the user handler
*
*Exit:
*       Returns the previous user handler
*
*Exceptions:
*
*******************************************************************************/

_qc_secerr_handler_func __cdecl _qc_set_security_error_handler(
    _qc_secerr_handler_func handler)
{
    _qc_secerr_handler_func old_handler;

    old_handler = user_handler;
    user_handler = handler;

    return old_handler;
}

