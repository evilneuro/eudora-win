///////////////////////////////////////////////////////////////////////////////
// MemImplMT.h
// 
// Created: 01/11/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_IMPL_MT_H_
#define __MEM_IMPL_MT_H_

// This file is included in
//
//     MemImplMT.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_IMPL_MT_CPP__)
#if defined(WIN32)
#error This file is to be included only in \
MemImplMT.cpp
#else
#error This file is to be included only in
#error
#error     MemImplMT.cpp
#error
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if defined(WIN32)
#include <string.h>
#ifdef __DEBUG__
#include <crtdbg.h>
#endif // __DEBUG__
#endif

#if defined(__unix__)
#include <iostream.h>
#include <string.h>
#endif // __unix__

#include "MemInternalIface.h"
#include "AutoSerializedRegionIface.h"
#include "MemMgrSerializedRegionIface.h"
#include "MemIfaceMT.h"

#endif // __MEM_IMPL_MT_H_
