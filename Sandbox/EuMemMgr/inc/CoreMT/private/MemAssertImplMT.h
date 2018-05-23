///////////////////////////////////////////////////////////////////////////////
// MemAssertImplMT.h
// 
// Created: 01/21/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_IMPL_MT_H_
#define __MEM_ASSERT_IMPL_MT_H_

// This file is included in
//
//     MemAssertImplMT.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_IMPL_MT_CPP__)
#if defined(WIN32)
#error This file is to be included only in \
MemAssertImplMT.cpp 
#else
#error This file is to be included only in
#error
#error     MemAssertImplMT.cpp
#error
#endif
#endif

#include "MemAssertExternalIfaceMT.h" // includes MemAssertExternalIface.h
#include "MemAssertInternalIface.h"
#include "MemMgrSerializedRegionIface.h" // for serialization
#include "AutoSerializedRegionIface.h" // for serialization

#endif // __MEM_ASSERT_IMPL_MT_H_
