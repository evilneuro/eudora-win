//////////////////////////////////////////////////////////////////////////////
// MemAssertImplMT.cpp
// 
// Created: 01/21/98 smohanty
//
//////////////////////////////////////////////////////////////////////////////

// Implementation specific define.
#define __MEM_ASSERT_IMPL_MT_CPP__

#include "MemAssertImplMT.h"

#ifdef __DEBUG__
void
_MAssertMT(c_char * const szExpr, c_char * const szMsg,
	   c_char * const szfile, int nLine)
{
#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    _MAssert(szExpr, szMsg, szfile, nLine);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

#ifdef __DEBUG__
void
_MAssertTypedMT(c_char * const szExpr, c_char * const szType,
		c_char * const szMsg, c_char * const szfile,
		int nLine)
{
#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    _MAssertTyped(szExpr, szType, szMsg, szfile, nLine);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

#ifdef __DEBUG__
AssertHandler
SetMAssertHandlerMT(AssertHandler newHandler)
{
    AssertHandler ah;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    ah = SetMAssertHandler(newHandler);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(ah);
}
#endif // __DEBUG__


#ifdef __DEBUG__
DbgOutputHandler
SetMDbgOutputHandlerMT(DbgOutputHandler newHandler)
{
    DbgOutputHandler doh;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    doh = SetMDbgOutputHandler(newHandler);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(doh);
}
#endif // __DEBUG__

#ifdef __DEBUG__
MemLeakHandler
SetMMemLeakHandlerMT(MemLeakHandler newHandler)
{
    MemLeakHandler mlh;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    mlh = SetMMemLeakHandler(newHandler);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(mlh);
}
#endif // __DEBUG__
