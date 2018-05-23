///////////////////////////////////////////////////////////////////////////////
// MemAssertUtilsImpl.h
// 
// Created: 01/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_ASSERT_UTILS_IMPL_H_
#define __MEM_ASSERT_UTILS_IMPL_H_

// This file is included in
//
//     MemAssertUtilsImpl.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_UTILS_IMPL_CPP__)
#if defined(WIN32)
#error This file is to be included only in \
MemAssertUtilsImpl.cpp
#else
#error This file is to be included only in
#error
#error     MemAssertUtilsImpl.cpp
#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4115 4201 4214 4514)
#include <windows.h>
#pragma warning(default : 4115 4201 4214)
#endif

#ifdef __unix__
#include <stdio.h>
#endif

#include "MemAssertUtilsInternalIface.h"

#ifdef __DEBUG__
c_char * const
_strSeparator =
"================================================================================================";
#endif // __DEBUG__

#endif // __MEM_ASSERT_UTILS_IMPL_H_
