/******************************************************************************/
/*																										*/
/*	Name		:	DLLMAIN.H		  																*/
/* Date     :  5/10/1997                                                      */
/* Author   :  Jim Susoy                                                      */
/* Notice   :  (C) 1997 Qualcomm, Inc. - All Rights Reserved                  */
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

/*	Desc.		:	DLLMain Function Prototypes												*/
/*																										*/
/******************************************************************************/
#ifndef __DLLMAIN_H__
#define __DLLMAIN_H__

//#define DBGNOBREAKS

#include "IListp.h"
#include "ISockp.h"
#include "ISchedp.h"

#define WINDOW_CLASS "__ISOCKPUMP__"

/* ISock List item entry definition															*/										
typedef struct TAGISOCKITEM {
	struct TAGISOCKITEM		*pPrev;
	struct TAGISOCKITEM  	*pNext;
	
	LPISOCK						pISock;
} ISOCKITEM, *LPISOCKITEM;

typedef struct TAGSCHEDITEM {
	struct TAGSCHEDITEM		*pPrev;
	struct TAGSCHEDITEM   	*pNext;
	
	LPISCHEDULE					pSched;
	CONTINUATION				pCB;
	LPVOID						pCtx;
} SCHEDITEM, *LPSCHEDITEM;

typedef struct TAGTIMERITEM {
	struct TAGTIMERITEM		*pPrev;
	struct TAGTIMERITEM   	*pNext;
	
	LPISCHEDULE					pSched;		/* Object that owns this entry			*/
	CONTINUATION				pCB;			/* Continuation function to call			*/
	LPVOID						pCtx;			/* Continuation data							*/
	DWORD							dwPosted;	/* Tickcount when timer was posted		*/
	DWORD							dwTimeout;	/* Delay in milliseconds					*/
	BOOL							bRemove;		/* TRUE timer already run, remove		*/
} TIMERITEM, *LPTIMERITEM;

typedef struct TAGFAILPOSTITEM {
	struct TAGFAILPOSTITEM	*pPrev;
	struct TAGFAILPOSTITEM  *pNext;
	
} FAILEDPOST, *LPFAILEDPOSTITEM;
				
/* Our application class																		*/
typedef class CApp * LPCAPP;
class CApp {
public:
	CApp();
	~CApp();
	
	BOOL				__stdcall	IsInitialized();
	BOOL				PASCAL Initialize(HINSTANCE h);
	void				PASCAL Deinitialize(void);
	HWND				PASCAL GetWindow(void);
	
	/* Object services																			*/
	LPILISTMAN		PASCAL CreateListObj(void);
	LPISCHEDULE 	PASCAL CreateSchedObj(void);
	
	
	/* ISock related methods																	*/
	WSADATA *		PASCAL GetWSAData(void);
	BOOL 				PASCAL SockListAdd(LPISOCK pISock);
	void 				PASCAL SockListRemove(LPISOCK pISock);
	
	/* ISchedule related methods																*/
	BOOL				PASCAL ScheduleNow(LPISCHEDULE pSched,CONTINUATION pCB,LPVOID pCtx,BOOL bAppend=TRUE);
	BOOL           PASCAL ScheduleLater(LPISCHEDULE pSched,CONTINUATION pCB,LPVOID pCtx,DWORD dwDelay);
	void				PASCAL CancelIScheduleEvents(LPISCHEDULE pSched);
	DWORD				PASCAL ScheduleCount(LPISCHEDULE pSched);
	
	/* Static callback methods																	*/
	static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static void ISockListFreeCB(LPITEM pItem,LPVOID pUser);
	static void IContinueListFreeCB(LPITEM pItem,LPVOID pUser);
	static void ITimerFreeCB(LPITEM pItem,LPVOID pUser);
	//static void IFailedPostFreeCB(LPITEM pItem,LPVOID pUser);
	HINSTANCE		m_hInstance;		  	/* Application instance						*/

#ifdef _DEBUG
	/* Debug routines																				*/
	static BOOL CALLBACK DbgDlgProc(HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	void				__stdcall DebugPoke(LPSTR pStr);
#endif
	
protected:
	/* Socket related Methods																	*/
	void				PASCAL DispatchSocketMessage(WPARAM wParam,LPARAM lParam);
	void				PASCAL DispatchSocketEvent(WPARAM wParam,LPARAM lParam);
	LPISOCK 			PASCAL SockListLookupAsyncOp(HANDLE h);
	LPISOCK 			PASCAL SockListFind(SOCKET s);
	LPISOCKITEM 	PASCAL SockListLookupISock(LPISOCK pISock);
	
	/* Continuation dispatcher related methods											*/
	void				PASCAL QueueTimer(LPTIMERITEM pTmp);
	void				PASCAL RunTimers(void);
	void 				PASCAL DispatchContinuation(WPARAM wParam,LPARAM lParam);
	void				PASCAL QueueMessage(void);

	HWND 				m_hWnd; 				  	/* Hidden window								*/
	BOOL				m_bClassRegistered; 	/* TRUE = we registered our wnd class	*/
	ATOM				m_Class;					/* Class Handle								*/
	WSADATA			m_wsadata;				/* Winsock information 						*/
	LPCLASSFACTORY	m_pListCf;
	LPCLASSFACTORY m_pSchedCf;
	LPILISTMAN		m_pISockList;	  		/* List of ISock objects					*/
	LPILISTMAN		m_pContinueList;  	/* List of ISchedContinue objects 		*/
	LPILISTMAN		m_pTimerList;	  		/* List of timers to process				*/
	LPILISTMAN     m_pTmpTimerList;		/* Used when running timers				*/
	LPILISTMAN    	m_pFailedPosts;		/* List of items we couldn't post      */
	BOOL				m_bInitialized;		/* Set if object has been intiialized	*/
	
	UINT				m_hTimer;				/* Timer handle returned by SetTimer	*/
	BOOL				m_bMsgWaiting;			/* Used for continuation dispatching	*/
#ifdef _DEBUG
	HWND				m_hwndDebug;
	HWND				m_hwndList;
#endif
private:
	unsigned int WSA_refCount;;
};


BOOL PASCAL SockListAdd(LPISOCK pISock);
void PASCAL SockListRemove(LPISOCK pISock);

#define MESSAGE_BASE		WM_USER +1000
#define WM_SOCKETMSG		MESSAGE_BASE+1	/* Handle based async socket event		*/
#define WM_SOCKETEVENT  MESSAGE_BASE+2	/* WSAAsyncSelect() events (FD_READ...)*/
#define WM_SCHEDULE    	MESSAGE_BASE+3	/* Resume continuation event				*/

#ifdef _DEBUG
#define DBGPOKE(s) gApp.DebugPoke((s));
#else
#define DBGPOKE(s)
#endif

#if defined(_DEBUG) && !defined(DBGNOBREAKS)
#define DBGBRK(fncname) { \
char sz123[256]; \
wsprintf(sz123,"Execute DebugBreak() in %s?",fncname); \
if(IDYES == ::MessageBox(NULL,sz123,"Debug",MB_YESNO)) \
		DebugBreak(); }
#else
#define DBGBRK
#endif

#endif

