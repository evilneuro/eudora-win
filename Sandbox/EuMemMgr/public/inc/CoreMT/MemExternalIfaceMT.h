///////////////////////////////////////////////////////////////////////////////
// MemExternalIfaceMT.h
//
// Thread-safe heap debugging services with mark and sweep capability.
//
// These services support both C and C++. Why? you ask. Well, not everyone
// is programming in C++. Ergo, if we ever get a third party library
// written in C, these heap debugging services can be used as is, without
// any modification. Since C is supported, we have to use C'isms.
//
// Created: 01/21/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_EXTERNAL_IFACE_MT_H_
#define __MEM_EXTERNAL_IFACE_MT_H_

// This file is included in
//
//     MemIfaceMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_IFACE_MT_H_)
#if defined(WIN32)
#error This file is to be included only in \
MemIfaceMT.h 
#else
#error This file is to be included only in
#error
#error     MemIfaceMT.h
#endif
#endif

// Include Block.
#include "MemExternalIface.h"
#include "MemAssertExternalIfaceMT.h"

#endif // __MEM_EXTERNAL_IFACE_MT_H_
