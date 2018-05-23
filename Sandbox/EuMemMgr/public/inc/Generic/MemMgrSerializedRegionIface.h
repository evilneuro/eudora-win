///////////////////////////////////////////////////////////////////////////////
// MemMgrSerializedRegionIface.h
// 
// Created: 01/21/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEMMGR_SERIALIZED_REGION_IFACE_H_
#define __MEMMGR_SERIALIZED_REGION_IFACE_H_

// This file is included in
//
//     MemMgrSerializedRegionImpl.h
//     MemAssertImplMT.h
//     MemImplMT.h
//
// No other file should include it.
//
// To enforce our policy against indiscriminate inclusion of all manners
// of header files in every which source file, we express our design goals
// by issuing a compile-time error if this file is included files other
// than the ones we have intended it for.

// #if !defined(__MEMMGR_SERIALIZED_REGION_IMPL_H_) && !defined(__MEM_ASSERT_IMPL_MT_H_) && !defined(__MEM_IMPL_MT_H_)
// #if defined(WIN32)
// #error This file is to be included only in one of \
// MemMgrSerializedRegionImpl.h \
// MemAssertImplMT.h \
// MemImplMT.h
// #else
// #error This file is to be included only in
// #error
// #error     MemMgrSerializedRegionImpl.h
// #error     MemAssertImplMT.h
// #error     MemImplMT.h
// #endif
// #endif

#if defined(__unix__)
#include <pthread.h>
#endif

#if defined(__unix__)
extern pthread_mutex_t  _MemMgrMutex;
// Multilingual block.
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void pthread_cleanup_handler(void *arg);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __unix__

#if defined(WIN32)
class __declspec(dllexport) MemMgrSerializedRegionMT {
#else
class MemMgrSerializedRegionMT {
#endif

public:

          MemMgrSerializedRegionMT();
         ~MemMgrSerializedRegionMT();
    void  EnterMT();
    void  LeaveMT();

private:

    static uint nRefCnt;
};


#endif // __MEMMGR_SERIALIZED_REGION_IFACE_H_
