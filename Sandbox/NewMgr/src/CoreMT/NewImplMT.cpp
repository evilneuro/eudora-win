//////////////////////////////////////////////////////////////////////////////
// NewImpl.cpp
// 
// Implementation: Debug Memory Manager C++ layer.
//
// Created: 05/21/97 smohanty
// Created: 01/12/98 smohanty
//
//////////////////////////////////////////////////////////////////////////////

// To silence unused inline placement new method.
#pragma warning(disable : 4514)

// Implementation specific define.
#define __NEW_IMPL_MT_CPP__

#include "NewImplMT.h"

#ifdef __DEBUG__
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PNH
Mem_new_handler(PNH pNH)
{
    PNH _old_handler = _new_handler;
    _new_handler      = pNH;
    return(_old_handler);
}

// Eudora Debug Manager new handler.
// smohanty: 05/27/97
int
MemNewHandler(size_t size)
{
    SetHeapInsolventMT();
    throw size;
    return(0);
}

void *
operator new(size_t size)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, NULL, 0)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
#ifdef __DEBUG__
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_SINGLE));
#endif
    return(pv);
}

#ifdef __DEBUG__
void *
operator new(size_t size, c_char *szFile, int nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_SINGLE));
    return(pv);
}
#endif // __DEBUG__

#ifdef __DEBUG__
void *
operator new[](size_t size, c_char *szFile, int nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_ARRAY));
    return(pv);
}
#endif // __DEBUG__

#ifdef __DEBUG__
void *operator new(size_t size, c_char * const szFile, uint nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_SINGLE));
    return(pv);
}
#endif // __DEBUG__

#if defined(__DEBUG__) && defined(WIN32)
// This is added to overide VC++'s function with the same signature.
void * __cdecl 
operator new(size_t size, int, const char * szFile, int nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_SINGLE));
    return(pv);
}
#endif // __DEBUG__

#if defined(__DEBUG__) && defined(WIN32)
// This is added to overide VC++'s function with the same signature.
void * __cdecl
operator new(size_t size, int, char * szFile, int nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_SINGLE));
    return(pv);
}
#endif // __DEBUG__

#if defined(__DEBUG__) && defined(WIN32)
// This is added to overide VC++'s function with the same signature.
void * __cdecl
operator new[](size_t size, int, const char * szFile, int nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_ARRAY));
    return(pv);
}
#endif // __DEBUG__

#if defined(__DEBUG__) && defined(WIN32)
// This is added to overide VC++'s function with the same signature.
void * __cdecl
operator new[](size_t size, int, char * szFile, int nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_ARRAY));
    return(pv);
}
#endif // __DEBUG__

void *
operator new[](size_t size)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, NULL, 0)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
#ifdef __DEBUG__
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_ARRAY));
#endif
    return(pv);
}

#ifdef __DEBUG__
void *
operator new[](size_t size, c_char * const szFile, uint nLine)
{
    void *pv;
    
    while ((pv = pvNewBlockMT((size == 0) ? 4 : size, szFile, nLine)) == 0) {
	if (_new_handler) {
	    (*_new_handler)(size);
	}
	else {
	    return(0);
	}
    }
    SaveUserDataMT(pv, static_cast<void *>(&_NEW_TYPE_ARRAY));
    return(pv);
}
#endif // __DEBUG__

#ifdef sun
void
operator delete(void *pv) throw()
#else
void
operator delete(void *pv)
#endif
{
    // The ANSI std. says that it is always okay to delete a null pointer,
    // so here it goes.
    if (pv == NULL) {
	return;
    }
    else {
#ifdef __DEBUG__
	sprintf(_strNewMemBuffer, _strDeletingArray, (uint) pv,
		nRetrieveLineInfoMT(pv), strRetrieveFileInfoMT(pv));
	MAssertSzClientMT(*(static_cast<uint *>(pvRetrieveUserDataMT(pv)))
			  != _NEW_TYPE_ARRAY, _strNewMemBuffer);
#endif // __DEBUG__
	FreeBlockMT(pv);
    }
}

#ifdef sun
void
operator delete[](void *pv) throw()
#else
void
operator delete[](void *pv)
#endif
{
    // The ANSI std. says that it is always okay to delete a null pointer,
    // so here it goes.
    if (pv == NULL) {
	return;
    }
    else {
#ifdef __DEBUG__
	sprintf(_strNewMemBuffer, _strDeletingSingle, (uint) pv,
		nRetrieveLineInfoMT(pv), strRetrieveFileInfoMT(pv));
	MAssertSzClientMT(*(static_cast<uint *>(pvRetrieveUserDataMT(pv)))
			  != _NEW_TYPE_SINGLE, _strNewMemBuffer);
#endif // __DEBUG__
	FreeBlockMT(pv);
    }

}

#ifdef __DEBUG__
static void *
pvNewBlockMT(size_t size, c_char * const szFile, uint nLine)
#else
static void *
pvNewBlockMT(size_t size, c_char * const szFile, uint)
#endif
{
    void *pv = NULL;

    if (fIsHeapSolventMT()) {
	if (fNewBlockMT(&pv, size)) {
	    if (szFile == 0) {
		fNoteBlockOriginMT(pv, THIS_FILE, __LINE__ - 2);
		fBaptizeBlockMT(pv, "ANONYMOUS");
	    }
	    else {
#ifdef __DEBUG__
		fNoteBlockOriginMT(pv, szFile, nLine);
#endif
	    }
	}
    }
    
    return(pv);
}
