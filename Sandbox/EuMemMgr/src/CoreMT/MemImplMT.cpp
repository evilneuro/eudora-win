//////////////////////////////////////////////////////////////////////////////
// MemImplMT.cpp
// 
// Created: 01/11/98 smohanty
//
//////////////////////////////////////////////////////////////////////////////

// Implementation specific define.
#define __MEM_IMPL_MT_CPP__

#include "MemImplMT.h"

#ifdef __DEBUG__
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Added 01/14/98
#ifdef __DEBUG__
AssertHandler
ResetAssertHandlerMT(void)
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
    
    ah = ResetAssertHandler();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(ah);
}
#endif // __DEBUG__

// Added 01/14/98
#ifdef __DEBUG__
DbgOutputHandler
ResetDbgOutputHandlerMT(void)
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
    
    doh = ResetDbgOutputHandler();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(doh);
}
#endif // __DEBUG__

// Added 01/14/98
#ifdef __DEBUG__
MemLeakHandler
ResetMemLeakHandlerMT(void)
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
    
    mlh = ResetMemLeakHandler();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(mlh);
}
#endif // __DEBUG__

flag
InitializeDebugMemoryManagerMT()
{
    flag fRet;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    fRet = InitializeDebugMemoryManager();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
    
    return(fRet);
}

void
ShutdownDebugMemoryManagerMT()
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

    ShutdownDebugMemoryManager();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}

#ifdef __DEBUG__
flag
_fNewBlockMT(void **ppv, size_t size, c_char *szFile, uint nLine)
#else
flag
fNewBlockMT(void **ppv, size_t size)
#endif // __DEBUG__
{
    flag fRet;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    fRet = fNewBlock(ppv, size);

#ifdef __DEBUG__
    if (fRet && szFile != NULL) {
	fNoteBlockOrigin(*ppv, szFile, nLine);
    }
#endif // __DEBUG__

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
    
    return(fRet);
}

void
FreeBlockMT(void *pv)
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

    FreeBlock(pv);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}

#ifdef __DEBUG__
void
GetAllocBytesMT(int *pnBytesAlloc, int *pnMaxBytesAlloc)
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

    GetAllocBytes(pnBytesAlloc, pnMaxBytesAlloc);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

#ifdef __DEBUG__
uint
nAllocBytesMT(void)
{
    uint nBytes;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    nBytes = nAllocBytes();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(nBytes);
}
#endif // __DEBUG__

#ifdef __DEBUG__
uint
nMaxAllocBytesMT(void)
{
    uint nBytes;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    nBytes = nMaxAllocBytes();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(nBytes);
}
#endif // __DEBUG__

#ifdef __DEBUG__
void
SaveUserDataMT(void *pv, void *pvUserData)
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

    SaveUserData(pv, pvUserData);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

#ifdef __DEBUG__
void *
pvRetrieveUserDataMT(void *pv)
{
    void *pvRet;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32
    
    pvRet = pvRetrieveUserData(pv);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(pvRet);
}
#endif // __DEBUG__

#ifdef __DEBUG__
c_uint
nRetrieveLineInfoMT(void *pv)
{
    int nLineInfo;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    nLineInfo = nRetrieveLineInfo(pv);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(nLineInfo);
}
#endif // __DEBUG__

#ifdef __DEBUG__
c_char * const
strRetrieveFileInfoMT(void *pv)
{
    char *strFileInfo;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32
    
    strFileInfo = strRetrieveFileInfo(pv);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(strFileInfo);
}
#endif // __DEBUG__

#ifdef __DEBUG__
flag
fDoBaptizeBlockMT(void *pv, c_char * const szLabel)
{
    flag retFlag;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    retFlag = fDoBaptizeBlock(pv, szLabel);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(retFlag);
}
#endif // __DEBUG__

#ifdef __DEBUG__    
flag
fDoNoteBlockOriginMT(void *pv, c_char *const szFile, uint nLine)
{
    flag retFlag;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    retFlag = fDoNoteBlockOrigin(pv, szFile, nLine);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(retFlag);
}
#endif // __DEBUG__

#ifdef __DEBUG__
void
IdleTestBlocksMT(void)
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

    IdleTestBlocks();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

#ifdef __DEGUG__
void
DumpBlocksLogMT(void *data, PDBF pDBF, c_uint nRefOnly)
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

    DumpBlocksLog(data, pDBF, nRefOnly);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

#ifdef __DEBUG__
void
DumpBlocksLogByLabelAndFileMT(void *pvData, PDBF pDBF, c_uint nRefOnly,
			      char *label, char *fileName)
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

    DumpBlocksLogByLabelAndFile(pvData, pDBF, nRefOnly, label, fileName);

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}
#endif // __DEBUG__

// Set heap insolvent(bankrupt).
void
SetHeapInsolventMT(void)
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

    SetHeapInsolvent();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}

// Set heap solvent.
void
SetHeapSolventMT(void)
{
#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif

    SetHeapSolvent();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__
}

// The next two functions return the status of the heap. The semantic
// content of the information they return is identical; they differ
// only in the questions they are answers to.
// Return heap solvency status
flag
fIsHeapSolventMT(void)
{
    flag fHS;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    fHS = fIsHeapSolvent();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(fHS);
}

// Return heap solvency status
flag
fIsHeapInsolventMT(void)
{
    flag fHI;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    fHI = fIsHeapInsolvent();

#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(fHI);
}



#ifdef __DEBUG__
char
*_DMMStrdupMT(c_char *source, c_char *szFile, uint nLine)
#else
char
*DMMStrdupMT(c_char *source)
#endif // __DEBUG__
{
    char *dest = 0;

#if defined(WIN32)
    AutoSerializedRegionMT<MemMgrSerializedRegionMT> asrMT;
#elif defined(__unix__)
    pthread_cleanup_push(pthread_cleanup_handler,(void*)&_MemMgrMutex);
    pthread_mutex_lock(&_MemMgrMutex);
#else
#error Error: Developer, please provide implementation for serial locking
#error Error: for your platform...
#endif // WIN32

    uint  len  = 0;

    MAssert(source != NULL);

    if (source != NULL) {

	len = strlen(source);

	if (fNewBlock((void **)(&dest), len + 1)) {
#ifdef __DEBUG__
	    if (szFile) {
		fNoteBlockOrigin(dest, szFile, nLine);
	    }
#endif
	    memcpy(dest, source, len + 1);
	}
    }
    
#if defined(__unix__)
    pthread_cleanup_pop(1);
#endif // __unix__

    return(dest);
}
