///////////////////////////////////////////////////////////////////////////////
// MemAssertUtilsInternalIface.h
//
// Memory Manager Assertion Support.
//
// Created: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_UTILS_INTERNAL_IFACE_H_
#define __MEM_ASSERT_UTILS_INTERNAL_IFACE_H_

#include "EudoraBaseTypes.h"
#include "MemDbgDefIface.h"

// This file is included in
//
//     MemImpl.h
//     MemAssertUtilsImpl.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_IMPL_H_) && !defined(__MEM_ASSERT_UTILS_IMPL_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemImpl.h \
MemAssertUtilsImpl.h
#else
#error This file is to be included only in
#error
#error     MemImpl.h
#error     MemAssertUtilsImpl.h
#endif
#endif

#ifdef __DEBUG__
#ifdef __cplusplus
extern "C" {
#endif

// Default Assert Handler
extern void
MemAssertFail(c_char * const msg);

// Default Debug output Handler
extern void
MemOutputMessage(c_char * const msg);

// Default Memory Leak output Handler
extern void
MemLeakOutputMessage(c_char * const msg);

#ifdef __cplusplus
}
#endif
#endif // __DEBUG__

#endif // __MEM_ASSERT_UTILS_INTERNAL_IFACE_H_

