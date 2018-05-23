///////////////////////////////////////////////////////////////////////////////
// MemAssertExternalIface.h
//
// The purpose of this file is to provide the signatures of the different
// handlers that a client of DMM may wish to install over the default
// ones that the DMM provides.
//
// Created: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_EXTERNAL_IFACE_H_
#define __MEM_ASSERT_EXTERNAL_IFACE_H_

#include "EudoraBaseTypes.h"
#include "MemDbgDefIface.h"

// This file is included in
//
//     MemAssertExternalIfaceMT.h
//     MemAssertInternalIface.h
//     MemExternalIface.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_EXTERNAL_IFACE_MT_H_) &&!defined(__MEM_ASSERT_INTERNAL_IFACE_H_) && !defined(__MEM_EXTERNAL_IFACE_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemAssertExternalIfaceMT.h \
MemAssertInternalIface.h \
MemExternalIface.h
#else
#error This file is to be included only in
#error
#error     MemAssertExternalIfaceMT.h
#error     MemAssertInternalIface.h
#error     MemExternalIface.h
#endif
#endif

#ifdef __DEBUG__
// Handler for assertion failures. The DMM comes with a default handler.
// The default handler pops up a message box under WIN32 which has two
// buttons, OK and Cancel. OK or ESC should be pressed to dismiss the
// message box and Cancel should be pressed to break into the Debugger.
// Under UNIX, the default handler prints the assertion failure to
// stdout. If the DMM assertion handler has been set to NULL, then the
// program calls abort().
// 
// To install your own handler, call SetMAssertHandlerMT() with the
// address of a function that has the same signature as AssertHandler
// below, i.e returning void and taking a const char * const.
// Consult MemIfaceMT.h for details.
typedef void (*AssertHandler)(c_char * const);
#endif // __DEBUG__

#ifdef __DEBUG__
// Handler for DMM Debug Output. The DMM comes with a default handler.
// The default handler sends the output to the Debugger output window
// under WIN32. Under UNIX, the output is sent to stdout.
//
// To install your own handler, call SetMDbgOutputHandlerMT() with
// the address of a function that has the same signature as DbgOutputHandler
// below  i.e returning void and taking a const char * const.
// Consult MemIfaceMT.h for details.
typedef void (*DbgOutputHandler)(c_char * const);
#endif // __DEBUG__

#ifdef __DEBUG__
// Handler for reporting Memory Leak. The DMM comes with a default handler.
// The default handler sends the output to the Debugger output window
// under WIN32. Under UNIX, the output is sent to stdout.
//
// To install your own handler, call SetMLeakHandlerMR() with
// the address of a function that has the same signature as MemLeakHandler
// below i.e returning void and taking a const char * const.
// Consult MemIfaceMT.h for details.
typedef void (*MemLeakHandler)(c_char * const);
#endif // __DEBUG__

#endif // __MEM_ASSERT_EXTERNAL_IFACE_H_
