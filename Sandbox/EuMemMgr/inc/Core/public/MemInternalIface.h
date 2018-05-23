///////////////////////////////////////////////////////////////////////////////
// MemInternalface.h
//
// Interface: Heap debugging services with mark and sweep capability.
//
// These services support both C and C++. Why? you ask. Well, not everyone
// is programming in C++. Ergo, if we ever get a third party library
// written in C, these heap debugging services can be used as is, without
// any modification. Since C is supported, we have to use C'isms.
//
// Created:  01/12/98 smohanty
// Modified: 01/16/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_INTERNAL_IFACE_H_
#define __MEM_INTERNAL_IFACE_H_

// This file is included in
//
//     MemImpl.h
//     MemImplMT.h
//
//     and
//
//     MemCoreTest.c
//     MemCoreTest.cpp
//
// a sample test program to test the DMM core.
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_IMPL_H_) && !defined(__MEM_IMPL_MT_H_) && !defined(__MEM_CORE_TEST_C__) && !defined(__MEM_CORE_TEST_CPP__)
#if defined(WIN32)
#error This file is to be included only in one of \
MemImpl.h \
MemImplMT.h \
MemCoreTest.c \
MemCoreTest.cpp
#else
#error This file is to be included only in
#error
#error     MemImpl.h
#error     MemImplMT.h
#error     MemCoreTest.c
#error     MemCoreTest.cpp
#endif
#endif

#include "MemExternalIface.h"
#include "MemAssertInternalIface.h"	 // Assertion services.

// Defines
#ifdef __DEBUG__
#define fBaptizeBlock(pv, szLabel) fDoBaptizeBlock(pv, szLabel)
#define fNoteBlockOrigin(pv, szFile, nLn) fDoNoteBlockOrigin(pv, szFile, nLn)
#else // __DEBUG__
#define fBaptizeBlock(pv, szLabel)
#define fNoteBlockOrigin(pv, szFile, nLine)
#endif // __DEBUG__

// Multilingual block.
#ifdef __cplusplus
extern "C" {
#endif

extern flag
InitializeDebugMemoryManager(void);

extern void
ShutdownDebugMemoryManager(void);

#ifdef __DEBUG__
#define        fNewBlock(x, y) _fNewBlock(x, y, THIS_FILE, __LINE__)
extern flag
_fNewBlock(void **ppv, size_t size, c_char *szFile, uint nLine);
#else
extern flag
fNewBlock(void **ppv, size_t size);
#endif // __DEBUG__

extern void
FreeBlock(void *pv);

#ifdef __DEBUG__

extern void
GetAllocBytes(int *pnBytesAlloc, int *pnMaxBytesAlloc);

extern uint
nAllocBytes(void);

extern uint
nMaxAllocBytes(void);

extern void
SaveUserData(void *pv, void *pvUserData);

extern void *
pvRetrieveUserData(void *pv);

extern c_uint
nRetrieveLineInfo(void *pv);

//extern c_char * const strRetrieveFileInfo(void *pv);
extern char *
strRetrieveFileInfo(void *pv);

extern flag
fDoBaptizeBlock(void *pv, c_char * const szLabel);

extern flag
fDoNoteBlockOrigin(void *pv, c_char *const szFile, uint nLine);

//void UnreferBlocks(void);
//void ReferBlock(void *pv);
//void CheckBlockReferences(void);
//flag fGoodAddress(void *pv, size_t size);

extern void
IdleTestBlocks(void);

extern void
DumpBlocksLog(void *pvData, PDBF pDBF, c_uint nRefOnly);

extern void
DumpBlocksLogByLabelAndFile(void *pvData, PDBF pDBF, c_uint nRefOnly,
                            char *label, char *fileName);

// Added 01/14/98
extern AssertHandler
ResetAssertHandler(void);

extern DbgOutputHandler
ResetDbgOutputHandler(void);

extern MemLeakHandler
ResetMemLeakHandler(void);

#endif // __DEBUG__

// Set heap insolvent(bankrupt).
extern void
SetHeapInsolvent(void);

// Set heap solvent.
extern void
SetHeapSolvent(void);

// The next two functions return the status of the heap. The semantic
// content of the information they return is identical; they differ
// only in the questions they are answers to.
// Return heap solvency status
extern flag
fIsHeapSolvent(void);
// Return heap solvency status

extern flag
fIsHeapInsolvent(void);

extern char *
pMemMemoryReserve;

#ifdef __cplusplus
}
#endif

#endif // __MEM_INTERNAL_IFACE_H_
