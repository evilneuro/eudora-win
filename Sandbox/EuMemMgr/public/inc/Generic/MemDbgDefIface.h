///////////////////////////////////////////////////////////////////////////////
// MemDbgDefIface.h
//
// Microsoft Compiler defines _DEBUG while the rest of the world
// defines __DEBUG__. We work with __DEBUG__ to make our code as
// portable as possible.
//
// This header file defines __DEBUG__ if it is a Debug build and does
// the right thing for WIN32 and UNIX.
//
// Created:  05/25/97 smohanty
// Modified: 01/11/98 smohanty 
// Modified: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_DEBUG_DEFINITION_IFACE_H_
#define __MEM_DEBUG_DEFINITION_IFACE_H_

// This file is included in:
//
//     MemAssertExternalIface.h
//     MemAssertUtilsInternalIface.h
//     MemInternalIface.h
//     MemExternalIface.h
//     NewIfaceMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_EXTERNAL_IFACE_H_) && !defined(__MEM_ASSERT_UTILS_INTERNAL_IFACE_H_) && !defined(__MEM_INTERNAL_IFACE_H_) && !defined(__MEM_EXTERNAL_IFACE_H_) && !defined(__NEW_IFACE_MT_H_) && !defined(__EU_MEMMGR_INTERNAL_IFACE_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemAssertExternalIface.h \
MemAssertUtilsInternalIface.h \
MemInternalIface.h \
MemExternalIface.h \
NewIfaceMT.h \
EuMemMgrInternalIface.h
#else
#error This file is to be included only in
#error
#error     MemAssertExternalIface.h
#error     MemAssertUtilsInternalIface.h
#error     MemInternalIface.h
#error     MemExternalIface.h
#error     NewIfaceMT.h
#error     EuMemMgrInternalIface.h
#endif
#endif

#ifdef WIN32
#ifdef _DEBUG
#define __DEBUG__
#endif
#else
#ifdef __unix__
#ifndef NDEBUG
#define __DEBUG__
#endif
#endif
#endif

#endif // __MEM_DEBUG_DEFINITION_IFACE_H_
