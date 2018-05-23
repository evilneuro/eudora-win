///////////////////////////////////////////////////////////////////////////////
// MemMgrSerializedRegionImpl.h
// 
// Created: 01/21/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEMMGR_SERIALIZED_REGION_IMPL_H_
#define __MEMMGR_SERIALIZED_REGION_IMPL_H_

// This file is included in
//
//     MemMgrSerializedRegionImpl.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEMMGR_SERIALIZED_REGION_IMPL_CPP__)
#if defined(WIN32)
#error This file is to be included only in \
MemMgrSerializedRegionImpl.cpp
#error This file is to be included only in
#error
#error     MemMgrSerializedRegionImpl.cpp
#endif
#endif

#if defined(WIN32)
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#endif

// include block.
#include "EudoraBaseTypes.h"
#include "MemMgrSerializedRegionIface.h"

#if defined(WIN32)
extern CRITICAL_SECTION csMemMgrCriticalSection;
#endif

#endif // __MEMMGR_SERIALIZED_REGION_IMPL_H_
