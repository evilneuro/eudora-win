///////////////////////////////////////////////////////////////////////////////
// MemExternalIface.h
//
// External Interface: Heap debugging services with mark and sweep capability.
//
// These services support both C and C++. Why? you ask. Well, not everyone
// is programming in C++. Ergo, if we ever get a third party library
// written in C, these heap debugging services can be used as is, without
// any modification. Since C is supported, we have to use C'isms.
//
// Created: 01/12/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_EXTERNAL_IFACE_H_
#define __MEM_EXTERNAL_IFACE_H_

// This file is included in
//
//     MemInternalIface.h
//     MemExternalIfaceMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_INTERNAL_IFACE_H_) && !defined(__MEM_EXTERNAL_IFACE_MT_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemInternalIface.h \
MemExternalIfaceMT.h 
#else
#error This file is to be included only in
#error
#error     MemInternalIface.h
#error     MemExternalIfaceMT.h
#endif
#endif

// Include Block.
#include "EudoraBaseTypes.h" // Base types.
#include "MemDbgDefIface.h"  // This include defines __DEBUG__ if _DEBUG is.
#include "MemAssertExternalIface.h"

// Type of information being returned to dump blocks callback function.
#ifdef __DEBUG__
typedef uint           pDBF_OutType;
#define PDBF_SEP       0
#define PDBF_HEADING   1
#define PDBF_ITEM      2
#define PDBF_BLKSEMPTY 3
#define PDBF_TOTAL     4

typedef struct _mem_mgr_alloc_info {
    uint    Address;
    uint    SizeBytes;
    c_char *szLabel;
    c_char *szFile;
    uint    Line;
} MemMgrAllocInfo;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// PDBF = Pointer to Dump Blocks Function.
typedef flag (*PDBF)(void *, c_char * const, pDBF_OutType, void *);
#endif // __DEBUG__

#endif // __MEM_EXTERNAL_IFACE_H_
