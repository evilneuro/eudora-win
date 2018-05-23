///////////////////////////////////////////////////////////////////////////////
// MemMgrSerializedRegionImpl.cpp
//
// Created: 01/11/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#define __MEMMGR_SERIALIZED_REGION_IMPL_CPP__

#include "MemMgrSerializedRegionImpl.h"

#if defined(WIN32)
CRITICAL_SECTION csMemMgrCriticalSection;
#elif defined(__unix__)
pthread_mutex_t _MemMgrMutex = PTHREAD_MUTEX_INITIALIZER;
#else
#error Developer, Please provide implementation of thread-serialized
#error region for your platform...
#endif

uint MemMgrSerializedRegionMT::nRefCnt = 0;

void
InitCritSection()
{
    ::InitializeCriticalSection(&csMemMgrCriticalSection);
}

void
DelCritSection()
{
    ::DeleteCriticalSection(&csMemMgrCriticalSection);
}

MemMgrSerializedRegionMT::MemMgrSerializedRegionMT()
{
//     if (nRefCnt == 0) {
// #if defined(WIN32)
// 	InitializeCriticalSection(&csMemMgrCriticalSection);
// #endif
//     }

//     nRefCnt++;
}

MemMgrSerializedRegionMT::~MemMgrSerializedRegionMT()
{
//     nRefCnt--;

//     if (nRefCnt == 0) {
// #if defined(WIN32)       
// 	DeleteCriticalSection(&csMemMgrCriticalSection);
// #endif
//     }
}

void
MemMgrSerializedRegionMT::EnterMT()
{
#if defined(WIN32)
    EnterCriticalSection(&csMemMgrCriticalSection);
#elif defined(__unix__)
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, Please provide implementation for serial locking
#error Error: for your platform...
#endif
}

void
MemMgrSerializedRegionMT::LeaveMT()
{
#if defined(WIN32)
    LeaveCriticalSection(&csMemMgrCriticalSection);
#elif defined(__unix__)
    pthread_mutex_unlock(&_MemMgrMutex);
#else
#error Error: Developer, Please provide implementation for serial un-locking
#error Error: for your platform...
#endif
}

#if defined(__unix__)
void
pthread_cleanup_handler(void *arg)
{
    pthread_mutex_t *mutex = static_cast<pthread_mutex_t *>(arg);
    pthread_mutex_unlock(mutex);
}
#endif // __unix__
