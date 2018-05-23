///////////////////////////////////////////////////////////////////////////////
// MemAssertExternalIfaceMT.h
//
// Thread-safe assertion services.
//
// Created: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_EXTERNAL_IFACE_MT_H_
#define __MEM_ASSERT_EXTERNAL_IFACE_MT_H_

// This file is included in
//
//     MemAssertImplMT.h
//     MemExternalIfaceMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_IMPL_MT_H_) && !defined(__MEM_EXTERNAL_IFACE_MT_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemAssertImplMT.h \
MemExternalIfaceMT.h 
#else
#error This file is to be included only in
#error
#error     MemAssertImplMT.h
#error     MemExternalIfaceMT.h
#endif
#endif

// Include block
#include "MemAssertExternalIface.h" // for definition of debug handlers.
                                    // includes EudoraBaseTypes.h &
                                    // MemDbgDefIface.h.
#include "MemExportIface.h"         // for definition of DLLExport.

#ifdef __DEBUG__
// Assert
#define MAssertMT(expr)                          \
    if (expr) {}                                 \
    else                                         \
	_MAssertMT(#expr, 0, THIS_FILE, __LINE__)
// Assert with message
#define MAssertSzMT(expr, msg)                     \
    if (expr) {}                                   \
    else                                           \
	_MAssertMT(#expr, msg, THIS_FILE, __LINE__)

// Assert Internal
#define MAssertInternalMT(expr)                                             \
    if (expr) {}                                                            \
    else                                                                    \
	_MAssertTypedMT(#expr, "DMM Internal Error", 0, THIS_FILE, __LINE__)
// Assert Internal with message
#define MAssertSzInternalMT(expr, msg)                                        \
    if (expr) {}                                                              \
    else                                                                      \
	_MAssertTypedMT(#expr, "DMM Internal Error", msg, THIS_FILE, __LINE__)

// Assert Client
#define MAssertClientMT(expr)                                             \
    if (expr) {}                                                          \
    else                                                                  \
	_MAssertTypedMT(#expr, "DMM Client Error", 0, THIS_FILE, __LINE__)
// Assert Internal with message
#define MAssertSzClientMT(expr, msg)                                        \
    if (expr) {}                                                            \
    else                                                                    \
	_MAssertTypedMT(#expr, "DMM Client Error", msg, THIS_FILE, __LINE__)
// Debug Output.
#define MDbgOutputMT(message) _MDbgOutput(message)

// Mem Leak
#define MMemLeakMT(message) _MMemLeak(message)
#else // __DEBUG__
#define MAssertMT(expr)
#define MAssertSzMT(expr, msg)
#define MAssertInternalMT(expr)
#define MAssertSzInternalMT(expr, msg)
#define MAssertClientMT(expr)
#define MAssertSzClientMT(expr, msg)
#define MDbgOutputMT(message)
#define MMemLeakMT(message)
#endif // __DEBUG__

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLExport void
_MAssertMT(c_char * const szExpr, c_char * const szMsg, c_char * const szfile,
           int nLine);
DLLExport void
_MAssertTypedMT(c_char * const szExpr, c_char * const szType,
		c_char * const szMsg, c_char * const szfile, int nLine);
DLLExport void
_MDbgOutputMT(c_char * const message);
DLLExport void
_MMemLeakMT(c_char * const message);
DLLExport AssertHandler
SetMAssertHandlerMT(AssertHandler newHandler);
DLLExport DbgOutputHandler
SetMDbgOutputHandlerMT(DbgOutputHandler newHandler);
DLLExport MemLeakHandler
SetMLeakHandlerMT(MemLeakHandler newHandler);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __DEBUG__

#ifndef __DEBUG__
#define SetMAssertHandlerMT(newHandler)
#define SetMDbgOutputHandlerMT(newHandler)
#define SetMMemLeakHandlerMT(newHandler)
#endif // __DEBUG__

#endif // __MEM_ASSERT_EXTERNAL_IFACE_H_
