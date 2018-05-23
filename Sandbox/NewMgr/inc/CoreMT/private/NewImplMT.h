///////////////////////////////////////////////////////////////////////////////
// NewImpl.h
// 
// Implementation: Debug Memory Manager C++ layer.
//                 This file is the private implementation of the C++
//                 layer. It is to be included only by NewImpl.cpp.
//
// Created: 05/21/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __NEW_IMPL_MT_H_
#define __NEW_IMPL_MT_H_

// This file is included in
//
//     NewImplMT.cpp
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__NEW_IMPL_MT_CPP__)
#if defined(win32)
#error This file is to be included only in \
NewImplMT.cpp
#else
#error This file is to be included only in
#error
#error     NewImplMT.cpp
#endif
#endif

// Include Block.
#include <stdio.h>
#include "NewIfaceMT.h"

#ifdef __DEBUG__
static uint
_NEW_TYPE_SINGLE       = 0;

static uint
_NEW_TYPE_ARRAY        = 1;

static char
_strNewMemBuffer[1024] = { '\0' };

c_char * const
_strDeletingArray      = "Attempting to use \"delete\" on %08X, acquired through ARRAY \"new[]\"\n\nLine: %d\nFile: %s.\n\n";

c_char * const
_strDeletingSingle     = "Attempting to use \"delete[]\" on %08X, acquired through \"new\"\n\nLine: %d\nFile: %s.\n\n";
#endif // __DEBUG__

int
MemNewHandler(size_t size);

int
(*Mem_new_handler (int (*)(uint)))(uint);

typedef void (*PNHF)(void);
typedef int  (*PNH)(size_t size);
PNH          _new_handler = MemNewHandler;

static  void *
pvNewBlockMT(size_t size, c_char * const szFile, uint nLine);
#endif // __NEW_IMPL_MT_H_
