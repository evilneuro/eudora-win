///////////////////////////////////////////////////////////////////////////////
// NewIfaceMT.h
//
// C++ Interface: Heap debugging services with mark and sweep capability.
//
// Created:  01/27/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __NEW_IFACE_MT_H_
#define __NEW_IFACE_MT_H_

// This header file provides the C++ interface to the thread-safe Debug Memory
// Manager. It is written entirely in terms of the
// external interface to the DMM. Since this header file is designed to be
// included in any "client" of the DMM, there is, obviously, no restriction
// as to where it can be included.

// Include Block.
#include <stddef.h>
#ifndef sun
#include <new.h>
#endif

#undef new
#undef delete

#include "EudoraBaseTypes.h"
#include "MemDbgDefIface.h"
#include "MemExportIface.h"
#include "MemIfaceMT.h"

#ifndef __unix__ // comes from new.h
// std new and delete
void *
operator new(size_t);

void *
operator new[](size_t);

#ifdef sun
void
operator delete(void *) throw();
#else // sun
void
operator delete(void *);
#endif // sun

#ifdef sun
void
operator delete[](void *) throw();
#else // sun
void
operator delete[](void *);
#endif // sun

#endif // __unix__

#ifdef __DEBUG__
void *
operator new(size_t, c_char *, int);

void *
operator new(size_t, c_char * const, uint);
#endif // __DEBUG__

#ifdef WIN32
// These are added to overide VC++'s function with the same signature.
#ifdef __DEBUG__
#ifndef __NEW_IMPL_MT_CPP__
DLLImport void * __cdecl
operator new(size_t, int,  const char *, int);

DLLImport void * __cdecl
operator new[](size_t, int,  const char *, int);

DLLImport void * __cdecl
operator new(size_t, int, char *, int);

DLLImport void * __cdecl
operator new[](size_t, int, char *, int);
#else
DLLExport void * __cdecl
operator new(size_t, int,  const char *, int);

DLLExport void * __cdecl
operator new[](size_t, int,  const char *, int);

DLLExport void * __cdecl
operator new(size_t, int, char *, int);

DLLExport void * __cdecl
operator new[](size_t, int, char *, int);
#endif
#endif
#endif

// Operator Array new.
#ifdef __DEBUG__
void *
operator new[](size_t, c_char *, int);

void *
operator new[](size_t, c_char * const, uint);
#endif // __DEBUG__

#endif // __NEW_IFACE_MT_H_


