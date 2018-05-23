///////////////////////////////////////////////////////////////////////////////
// MemExportIface.h
//
// For Win32 DLL and UNIX export specification.
//
// The purpose of this file is to provide macros that generalize "exporting"
// methods in a library. Under WIN32, methods in a DLL are "exported" while
// under in UNIX they are "extern".
//
// Created:  01/15/98 smohanty
// Modified: 01/21/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEM_EXPORT_IFACE_H_
#define __MEM_EXPORT_IFACE_H_

// This file is included in
//
//     MemAssertExternalIfaceMT.h
//     MemIfaceMT.h
//     NewIfaceMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

#if !defined(__MEM_ASSERT_EXTERNAL_IFACE_MT_H_) && !defined(__MEM_IFACE_MT_H_) && !defined(__NEW_IFACE_MT_H_)
#if defined(WIN32)
#error This file is to be included only in one of \
MemAssertExternalIfaceMT.h \
MemIfaceMT.h \
NewIfaceMT.h 
#else
#error This file is to be included only in
#error
#error     MemAssertExternalIfaceMT.h
#error     MemIfaceMT.h
#error     NewIfaceMT.h
#endif
#endif

#if defined(WIN32)
#define DLLExport extern __declspec(dllexport)
#define DLLImport extern __declspec(dllimport)
#elif defined(__unix__)
#define DLLExport extern
#define DLLImport extern
#else
#if defined(WIN32)
#error Developer, please provide implementation for exporting \
error functions for your platform ...
#else
#error Developer, please provide implementation for exporting
#error functions for your platform ...
#endif
#endif

#endif // __MEM_EXPORT_IFACE_H__
