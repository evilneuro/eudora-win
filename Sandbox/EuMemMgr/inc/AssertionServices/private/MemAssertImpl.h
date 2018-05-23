///////////////////////////////////////////////////////////////////////////////
// MemAssertImpl.h
// 
// Created: 01/14/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_IMPL_H_
#define __MEM_ASSERT_IMPL_H_

// This file is included in
//
//     MemAssertImpl.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_IMPL_CPP__)
#if defined(WIN32)
#error This file is to be included only in \
MemAssertImpl.cpp 
#else
#error This file is to be included only in
#error
#error     MemAssertImpl.cpp
#error
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include "MemAssertInternalIface.h"

#ifdef __DEBUG__

static AssertHandler
MAssertHandler = NULL;

static DbgOutputHandler
MDbgOutputHandler = NULL;

static MemLeakHandler
MMemLeakHandler = NULL;

#endif // __DEBUG__

#endif // __MEM_ASSERT_IMPL_H_
