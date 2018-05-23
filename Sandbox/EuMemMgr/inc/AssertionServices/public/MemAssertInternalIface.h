///////////////////////////////////////////////////////////////////////////////
// MemAssertInternalIface.h
//
// Assertion Services.
//
// Examples:
//
// MAssert(pf != 0);
// MAssertSz(i > 0, "negative index");
//
// Created: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_INTERNAL_IFACE_H_
#define __MEM_ASSERT_INTERNAL_IFACE_H_

// This file is included in
//
//     MemAssertImpl.h
//     MemInternalIface.h
//     MemAssertImplMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_IMPL_H_) && !defined(__MEM_INTERNAL_IFACE_H_) && !defined(__MEM_ASSERT_IMPL_MT_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemAssertImpl.h \
MemInternalIface.h \
MemAssertImplMT.h
#else
#error This file is to be included only in
#error
#error     MemAssertImpl.h
#error     MemInternalIface.h
#error     MemAssertImplMT.h
#endif
#endif

#include "MemAssertExternalIface.h"

#ifdef __DEBUG__
#define MAssertFile(str) static char strAssertFile[] = str;

#ifdef __cplusplus
extern "C" {
#endif

extern void
_MAssert(c_char * const szExpr, c_char * const szMsg,
	 c_char * const szfile, int nLine);

extern void
_MAssertTyped(c_char * const szExpr, c_char * const szType,
	      c_char * const szMsg, c_char * const szfile, int nLine);

extern void
_MDbgOutput(c_char * const message);

extern void
_MMemLeak(c_char * const message);

extern AssertHandler
SetMAssertHandler(AssertHandler newHandler);

extern DbgOutputHandler
SetMDbgOutputHandler(DbgOutputHandler newHandler);

extern MemLeakHandler
SetMMemLeakHandler(MemLeakHandler newHandler);
#ifdef __cplusplus
}
#endif

// Assert
#define MAssert(expr)                          \
    if (expr) {}                               \
    else                                       \
	_MAssert(#expr, 0, THIS_FILE, __LINE__)
// Assert with message
#define MAssertSz(expr, msg)                     \
    if (expr) {}                                 \
    else                                         \
	_MAssert(#expr, msg, THIS_FILE, __LINE__)

// Assert Internal
#define MAssertInternal(expr)                                             \
    if (expr) {}                                                          \
    else                                                                  \
	_MAssertTyped(#expr, "DMM Internal Error", 0, THIS_FILE, __LINE__)
// Assert Internal with message
#define MAssertSzInternal(expr, msg)                                        \
    if (expr) {}                                                            \
    else                                                                    \
	_MAssertTyped(#expr, "DMM Internal Error", msg, THIS_FILE, __LINE__)

// Assert Client
#define MAssertClient(expr)                                             \
    if (expr) {}                                                        \
    else                                                                \
	_MAssertTyped(#expr, "DMM Client Error", 0, THIS_FILE, __LINE__)
// Assert Internal with message
#define MAssertSzClient(expr, msg)                                        \
    if (expr) {}                                                          \
    else                                                                  \
	_MAssertTyped(#expr, "DMM Client Error", msg, THIS_FILE, __LINE__)

// Debug Output.
#define MDbgOutput(message) _MDbgOutput(message)

// Mem Leak
#define MMemLeak(message) _MMemLeak(message)

#else // __DEBUG__
#define MAssertFile(str)
#define MAssert(expr)
#define MAssertSz(expr, message)
#define MAssertInternal(expr)
#define MAssertSzInternal(expr, message)
#define MAssertClient(expr)
#define MAssertSzClient(expr, message)
#define MDbgOutput(message)
#define MMemLeak(message)
#define SetMAssertHandler(newHandler)
#define SetMDbgOutputHandler(newHandler)
#define SetMMemLeakHandler(newHandler)
#endif // __DEBUG__

#endif // __MEM_ASSERT_INTERNAL_IFACE_H_
