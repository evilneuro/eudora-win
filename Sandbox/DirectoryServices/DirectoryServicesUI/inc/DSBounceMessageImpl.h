///////////////////////////////////////////////////////////////////////////////
// DSBounceMessageImpl.h
// 
// Created: 09/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_BOUNCE_MESSAGE_IMPL_H_
#define __DS_BOUNCE_MESSAGE_IMPL_H_

typedef void (*ResetLongResults)(void *);
typedef void (*AddStringLongResults)(void *, void *);
typedef void (*AddStringLongResultsFormatted)(void *, void *);
typedef void (*AppriseSelectedItem)(int, void *);
typedef void (*HandleTabFromResultsToc)(BOOL shiftDown, void *);
typedef bool (*QueryParentQueryInProgress)(void *);
typedef void (*AddStringNoNewLine)(char *, void *);
typedef void (*GetResultsLong)(CString&, void *);

typedef struct _ds_bounce_message {
    void                         *pv;
    ResetLongResults              rlr;
    AddStringLongResults          aslr;
    AddStringLongResultsFormatted aslrf;
    AppriseSelectedItem           asi;
    HandleTabFromResultsToc       htfrt;
    QueryParentQueryInProgress    qpqip;
    AddStringNoNewLine            asnnl;
    GetResultsLong                grl;
} DSBM;

#endif // __DS_BOUNCE_MESSAGE_IMPL_H_



