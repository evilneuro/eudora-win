///////////////////////////////////////////////////////////////////////////////
// MemIfaceMT.h
//
// The thread-safe version of MemIface.h
// Interface: Heap debugging services with mark and sweep capability.
//
// These services support both C and C++. Why? you ask. Well, not everyone
// is programming in C++. Ergo, if we ever get a third party library
// written in C, these heap debugging services can be used as is, without
// any modification. Since C is supported, we have to use C'isms.
//
// Created: 01/11/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_IFACE_MT_H_
#define __MEM_IFACE_MT_H_

// This header file provides the interface to the thread-safe Debug Memory
// Manager (hence the suffix MT). It is written entirely in terms of the
// external interface to the DMM. Since this header file is designed to be
// included in any "client" of the DMM, there is, obviously, no restriction
// as to where it can be included.

// Include Block.
#include <stddef.h>
#include "EudoraBaseTypes.h"
#include "MemExternalIfaceMT.h"
#include "MemExportIface.h"

// Defines
#ifdef __DEBUG__
#define fBaptizeBlockMT(pv, szLabel) fDoBaptizeBlockMT(pv, szLabel)
#define fNoteBlockOriginMT(pv, szFl, nLn) fDoNoteBlockOriginMT(pv, szFl, nLn)
#else
#define fBaptizeBlockMT(pv, szLabel)
#define fNoteBlockOriginMT(pv, szFile, nLine)
#endif // __DEBUG__

// Multilingual block.
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// The Support functions.
// Thread-safe alloc.
DLLExport flag
InitializeDebugMemoryManagerMT(void);

DLLExport void
ShutdownDebugMemoryManagerMT(void);

#ifdef __DEBUG__
#define fNewBlockMT(x, y)    _fNewBlockMT(x, y, THIS_FILE, __LINE__)
DLLExport flag
_fNewBlockMT(void **ppv, size_t size, c_char *szFile, uint nLine);
#else
DLLExport flag
fNewBlockMT(void **ppv, size_t size);
#endif // __DEBUG__

DLLExport void
FreeBlockMT(void *pv);

#ifdef __DEBUG__

DLLExport void
GetAllocBytesMT(int *pnBytesAlloc, int *pnMaxBytesAlloc);

DLLExport uint
nAllocBytesMT(void);

DLLExport uint
nMaxAllocBytesMT(void);

DLLExport void
SaveUserDataMT(void *pv, void *pvUserData);

DLLExport void *
pvRetrieveUserDataMT(void *pv);

DLLExport c_uint
nRetrieveLineInfoMT(void *pv);

DLLExport c_char * const
strRetrieveFileInfoMT(void *pv);

DLLExport flag
fDoBaptizeBlockMT(void *pv, c_char * const szLabel);

DLLExport flag
fDoNoteBlockOriginMT(void *pv, c_char *const szFile, uint nLine);

DLLExport void
IdleTestBlocksMT(void);

DLLExport void
DumpBlocksLogMT(void *data, PDBF pDBF, c_uint nRefOnly);

DLLExport void
DumpBlocksLogByLabelAndFileMT(void *pvData, PDBF pDBF, c_uint nRefOnly,
			      char *label, char *fileName);
// Added 01/14/98
DLLExport AssertHandler
ResetAssertHandlerMT();

DLLExport DbgOutputHandler
ResetDbgOutputHandlerMT();

DLLExport MemLeakHandler
ResetMemLeakHandlerMT();

#endif // __DEBUG__

// Set heap insolvent(bankrupt).
DLLExport void
SetHeapInsolventMT(void);

// Set heap solvent.
DLLExport void
SetHeapSolventMT(void);

// The next two functions return the status of the heap. The semantic
// content of the information they return is identical; they differ
// only in the questions they are answers to.
// Return heap solvency status
DLLExport flag
fIsHeapSolventMT(void);

// Return heap solvency status
DLLExport flag
fIsHeapInsolventMT(void);

// Thread-safe string dup function.
#ifdef __DEBUG__
#define DMMStrdupMT(x)       _DMMStrdupMT(x, THIS_FILE, __LINE__)
DLLExport char*
_DMMStrdupMT(c_char *src, c_char *szFile, uint nLine);
#else
DLLExport char*
DMMStrdupMT(c_char *src);
#endif // __DEBUG__

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MEM_IFACE_MT_H_
