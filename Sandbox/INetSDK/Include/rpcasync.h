/*++

Copyright (c) 1997 Microsoft Corporation

Module Name:

    rpcasync.h

Abstract:

    This module contains the RPC runtime APIs needed to use
    [async_handle] RPC features.

--*/

#ifndef __RPCASYNC_H__
#define __RPCASYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_ASYNC_VERSION_1_0     sizeof(RPC_ASYNC_STATE)

typedef
enum _RPC_NOTIFICATION_TYPES
{
    RpcNotificationTypeNone,
    RpcNotificationTypeEvent,
    RpcNotificationTypeApc, 
    RpcNotificationTypeIoc,
    RpcNotificationTypeHwnd,
    RpcNotificationTypeCallback
} RPC_NOTIFICATION_TYPES;

typedef
enum _RPC_ASYNC_EVENT {
    RpcCallComplete,
    RpcSendComplete,
    RpcReceiveComplete
    } RPC_ASYNC_EVENT;

typedef void (*PFN_RPCNOTIFICATION_ROUTINE) (
                  struct _RPC_ASYNC_STATE *pAsync,
                  void *Context,                              
                  RPC_ASYNC_EVENT Event);
    
typedef struct _RPC_ASYNC_STATE {
    unsigned int    Size; // size of this structure
    unsigned long   Signature;
             long   Lock;
    unsigned long   Flags;
    void           *StubInfo;
    void           *UserInfo;
    void           *RuntimeInfo;
    RPC_ASYNC_EVENT Event;

    RPC_NOTIFICATION_TYPES NotificationType;
    union {
        //
        // Notification by APC
        //
        struct {
            PFN_RPCNOTIFICATION_ROUTINE NotificationRoutine;
            HANDLE hThread;
            } APC;

        //
        // Notification by IO completion port
        //
        struct {
            HANDLE hIOPort;
            DWORD dwNumberOfBytesTransferred;
            DWORD dwCompletionKey;
            LPOVERLAPPED lpOverlapped;
            } IOC;

        //
        // Notification by window message
        //
        struct {
            HWND hWnd;
            UINT Msg;
            } HWND;


        //
        // Notification by event
        //
        HANDLE hEvent;

        //
        // Notification by callback function
        //
        PFN_RPCNOTIFICATION_ROUTINE NotificationRoutine;
        } u;

    long Reserved[4]; 
    } RPC_ASYNC_STATE, *PRPC_ASYNC_STATE;

// Possible values for Flags
#define RPC_C_NOTIFY_ON_SEND_COMPLETE      0x1
#define RPC_C_INFINITE_TIMEOUT             INFINITE

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcInitializeAsyncHandle (
    PRPC_ASYNC_STATE pAsync,
    unsigned int     Size
    );
                      
RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcRegisterAsyncInfo (
    PRPC_ASYNC_STATE pAsync
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcGetAsyncCallStatus (
    PRPC_ASYNC_STATE pAsync
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcCompleteAsyncCall (
    PRPC_ASYNC_STATE pAsync,
    void *Reply
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAbortAsyncCall (
    PRPC_ASYNC_STATE pAsync,
    unsigned long ExceptionCode
    ) ;


//
// Internal APIs
//
RPC_STATUS RPC_ENTRY
I_RpcSetAsyncHandle (
    IN  PRPC_MESSAGE Message,
    IN  PRPC_ASYNC_STATE pAsync
    );

RPC_STATUS RPC_ENTRY
I_RpcAbortAsyncCall (
    IN PRPC_ASYNC_STATE pAsync,
    IN unsigned long ExceptionCode
    ) ;

#ifdef __cplusplus
}
#endif

#endif /* __RPCASYNC_H__ */

