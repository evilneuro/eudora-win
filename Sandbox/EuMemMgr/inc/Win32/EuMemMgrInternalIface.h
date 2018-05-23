///////////////////////////////////////////////////////////////////////////////
// EuMemMgrInternalIface.h
// 
// This file is to be included only in EuMemMgr.cpp, the private implementation
// of the Debug Memory Manager.
//
// Created: 05/12/97 smohanty
// Created: 01/12/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __EU_MEMMGR_INTERNAL_IFACE_H_
#define __EU_MEMMGR_INTERNAL_IFACE_H_

// This file is included in
//
//     EuMemMgrImpl.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__EU_MEMMGR_IMPL_CPP_)
#if defined(WIN32)
#error This file is to be included only in \
EuMemMgrImpl.cpp
#else
#error This file is to be included only in
#error
#error     EuMemMgrImpl.cpp
#error
#endif
#endif

#if defined(WIN32)
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <stdio.h>
#elif defined(__unix__)
typedef int BOOL;
#define FALSE 0
#define TRUE  1
#else
#error Error: Developer, please provide implementation for
#error Error: your platform...
#endif // WIN32

#include "MemDbgDefIface.h"
#include "MemIfaceMT.h"

#endif // __EU_MEMMGR_INTERNAL_IFACE_H_
