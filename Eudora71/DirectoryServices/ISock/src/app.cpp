/******************************************************************************/
/*																										*/
/*	Name		:	APP.CPP		  																	*/
/* Date     :  8/13/1997                                                      */
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

/*	Desc.		:	Application class implementation	  										*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4345 4514)
#include <afx.h>
#include <winsock.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "dllmain.h"
#include "resource.h"


extern class CApp gApp;


void PASCAL OLEErrorBox(HRESULT hErr)
{
  LPVOID lpMsgBuf = NULL;

  FormatMessage( 	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    NULL,
						GetScode(hErr),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 	// Default language
						(LPTSTR) &lpMsgBuf,
						0,    NULL );											// Display the string.

    char pszBuffer[251];

	::LoadString(gApp.m_hInstance,IDS_ERR_ISOCKDLL, pszBuffer, 250);

	::MessageBox(NULL,(LPSTR)lpMsgBuf, pszBuffer, MB_OK|MB_ICONINFORMATION);
	
	// Free the buffer.
	if(lpMsgBuf)
		LocalFree( lpMsgBuf );
}

CApp::CApp()
	: WSA_refCount(0)
{
	m_hInstance 		= 0;
	m_hWnd 				= 0; 					/* Handle of our hidden message window	*/
	m_bClassRegistered = FALSE;
	m_Class 				= 0;
	m_pISockList		= NULL;
	m_pContinueList	= NULL;
	m_pTimerList		= NULL;
	m_pFailedPosts		= NULL;
	m_hTimer				= 0;
	m_bInitialized		= FALSE;
#ifdef _DEBUG
   m_hwndDebug       = NULL;
#endif
}

CApp::~CApp()
{
#ifdef _DEBUG
   if(m_hwndDebug)   DestroyWindow(m_hwndDebug);
   m_hwndDebug = NULL;
#endif
}

#ifdef _DEBUG

BOOL CALLBACK CApp::DbgDlgProc(HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM /* lParam */)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			return TRUE;
		}
		case WM_DESTROY: {
			break;
      }
		case WM_CLOSE: {
			DestroyWindow(hWndDlg);
			break;
      }
      case WM_COMMAND: {
         switch(LOWORD(wParam)) {
            case IDC_CLEAR: {
               SendMessage(GetDlgItem(hWndDlg,IDC_LIST1),LB_RESETCONTENT,0,0);
               break;
            }
         }
         break;
      }
	}
	return 0;
}

void __stdcall CApp::DebugPoke(LPSTR pStr)
{
   if(m_hwndDebug) {
      WPARAM wIdx = SendMessage(m_hwndList,LB_ADDSTRING,0,(LPARAM)pStr);
      SendMessage(m_hwndList,LB_SETCURSEL,wIdx,(LPARAM)0);
   }
}
#endif


BOOL PASCAL CApp::Initialize(HINSTANCE h)
{
	HRESULT 			hErr;
	int				nErr;
	WORD				wVersionRequested;
	BOOL				bToast	= FALSE;
	
	m_hInstance = h;						/* Save our hinstance for later			*/
#ifdef _DEBUG
#ifdef _SOCKETPOKES
   if(m_hwndDebug = CreateDialog(m_hInstance,MAKEINTRESOURCE(IDD_DEBUG),GetDesktopWindow(),(DLGPROC)CApp::DbgDlgProc)) {
      m_hwndList = GetDlgItem(m_hwndDebug,IDC_LIST1);
      ShowWindow(m_hwndDebug,SW_SHOW);
      UpdateWindow(m_hwndDebug);
   }
#else
#pragma message("You Can #define _SOCKETPOKES to turn on Poke Window")
#endif
#endif
	
	/* Initialize the Windows Sockets interface										*/
	wVersionRequested = MAKEWORD(1,1);
	nErr 					= WSAStartup(wVersionRequested,&m_wsadata);

	// smohanty: Keep refcount of WSAStartup. Fixes IMAP not being
	// able to gethostbyname() bug.
	// 04/03/98
        if (nErr == 0) {
		WSA_refCount++;
        }

	if(0 != nErr) {

    char pszBuffer1[251], pszBuffer2[251];

	::LoadString(gApp.m_hInstance, IDS_ERR_SOCKLOAD, pszBuffer1, 251);
	::LoadString(gApp.m_hInstance, IDS_ERR_TITLE, pszBuffer2, 251);


	::MessageBox(NULL, pszBuffer1, pszBuffer2, MB_OK);
	CoUninitialize();					/* DeInit OLE									*/
	return(FALSE);						/* Fail DLL Load								*/
	}
	/* Check Windows Sockets version														*/
	else if(1 != LOBYTE(m_wsadata.wVersion) || 1 != HIBYTE(m_wsadata.wVersion)) {

    char pszBuffer1[251], pszBuffer2[251];

	::LoadString(gApp.m_hInstance, IDS_ERR_SOCKWRONG, pszBuffer1, 251);
	::LoadString(gApp.m_hInstance, IDS_ERR_TITLE, pszBuffer2, 251);

	::MessageBox(NULL, pszBuffer1, pszBuffer2, MB_OK);
		if (WSA_refCount > 0) {
			WSA_refCount--;
			WSACleanup(); /* Deinit the Windows Socket interface */
                }

		CoUninitialize();					/* DeInit OLE									*/
		return(FALSE);						/* Fail DLL Load								*/
	}
	
	/* Register our hidden windows class												*/
	if(FALSE == m_bClassRegistered) {	/* Register our window class if needed	*/
		WNDCLASS	wc = {0};
		
		m_bClassRegistered = TRUE;
		
		wc.style				= 0;
		wc.lpfnWndProc    = CApp::WndProc;
		wc.cbClsExtra 		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= h;
		wc.hIcon				= NULL;
		wc.hCursor			= NULL;
		wc.hbrBackground	= (HBRUSH) COLOR_BACKGROUND;
		wc.lpszMenuName	= NULL;
		wc.lpszClassName	= WINDOW_CLASS;
		m_Class = RegisterClass(&wc);
	}
	
	/* Create our hidden window															*/
	if(!m_hWnd) {
		m_hWnd = CreateWindow(WINDOW_CLASS, //pointer to registered class name
									 "",				//pointer to window name
									 WS_OVERLAPPEDWINDOW, //window style
									 0,				// horizontal position of window
									 0, 				// vertical position of window
									 0,				// window width
									 0,				// window height
									 NULL,			// handle to parent or owner window
									 NULL,			// handle to menu or child-window identifier
									 h,				// handle to application instance
									 NULL          // pointer to window-creation data
									);
		if(!m_hWnd) {

			char pszBuffer1[251], pszBuffer2[251];

	        ::LoadString(gApp.m_hInstance, IDS_ERR_HIDDENWND, pszBuffer1, 251);
	        ::LoadString(gApp.m_hInstance, IDS_ERR_ERROR, pszBuffer2, 251);

			::MessageBox(NULL,pszBuffer1,pszBuffer2,MB_OK);
			if (WSA_refCount > 0) {
				WSA_refCount--;
				WSACleanup(); /* Deinit Socket interface */
			}
			CoUninitialize();				/* DeInit OLE									*/
			return(FALSE);					/* Fail DLL Load								*/
		}
		
		hErr = CoGetClassObject(CLSID_ISchedule,CLSCTX_INPROC_SERVER,NULL,IID_IClassFactory,(LPVOID *)&m_pSchedCf);
		
		if(FAILED(GetScode(hErr)))	{
			OLEErrorBox(hErr);
			if (WSA_refCount > 0) {
				WSA_refCount--;
				WSACleanup(); /* Deinit Socket interface */
			}
			CoUninitialize();				/* DeInit OLE									*/
			return(FALSE);					/* Fail DLL Load								*/
		}
		
		/* Create all of the list objects we need										*/
		hErr = CoGetClassObject(CLSID_IListMan,CLSCTX_INPROC_SERVER,NULL,IID_IClassFactory,(LPVOID *)&m_pListCf);
		
		if(FAILED(GetScode(hErr)))	{
			OLEErrorBox(hErr);
			if (WSA_refCount > 0) {
				WSA_refCount--;
				WSACleanup(); /* Deinit Socket interface */
			}
			CoUninitialize();				/* DeInit OLE									*/
			return(FALSE);					/* Fail DLL Load								*/
		}
		
		if(FAILED(hErr = m_pListCf->CreateInstance(NULL,IID_IListMan,(LPVOID *)&m_pISockList)))
			bToast = TRUE;
		
		if(FAILED(hErr = m_pListCf->CreateInstance(NULL,IID_IListMan,(LPVOID *)&m_pContinueList)))
			bToast = TRUE;
		
		if(FAILED(hErr = m_pListCf->CreateInstance(NULL,IID_IListMan,(LPVOID *)&m_pTimerList)))
			bToast = TRUE;

		if(FAILED(hErr = m_pListCf->CreateInstance(NULL,IID_IListMan,(LPVOID *)&m_pTmpTimerList)))
			bToast = TRUE;
		
		if(TRUE == bToast) {
			if(m_pTimerList)		m_pTimerList->Release();
			if(m_pTmpTimerList)	m_pTmpTimerList->Release();
			if(m_pContinueList)	m_pContinueList->Release();
			if(m_pISockList)		m_pISockList->Release();
			
			m_pListCf->Release();						
			m_pListCf = NULL;
			
			return(FALSE);
		}
		else {
			m_pISockList->Initialize(ISockListFreeCB,NULL);
			m_pContinueList->Initialize(IContinueListFreeCB,NULL);
			m_pTimerList->Initialize(ITimerFreeCB,NULL);
			m_pTmpTimerList->Initialize(ITimerFreeCB,NULL);
		}
	}
	m_bInitialized = TRUE;
	return(TRUE);
}

BOOL __stdcall CApp::IsInitialized()
{
	return m_bInitialized;
}

void PASCAL CApp::Deinitialize(void)
{
/* Free our hidden window																	*/
	if(m_hWnd) {
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	
	/* Then free out list of ISock objects													*/
	if(m_pISockList) {
		m_pISockList->Release();
		m_pISockList = NULL;
	}
	
	if(m_pContinueList) {
		m_pContinueList->Release();
		m_pContinueList = NULL;
	}
		
	if(m_pTimerList) {
		m_pTimerList->Release();
		m_pTimerList = NULL;
	}
	
	if(m_pTmpTimerList) {
		m_pTmpTimerList->Release();
		m_pTmpTimerList = NULL;
	}
	
	/* Release the IClassFactory()															*/
	if(m_pListCf) {
		m_pListCf->Release();						
		m_pListCf = NULL;
	}
	
	/* And cleanup the usual stuff															*/
	UnregisterClass(WINDOW_CLASS,m_hInstance);
	m_bClassRegistered 	= FALSE;
	m_Class 					= 0;
	m_hInstance 			= 0;
	
	if (WSA_refCount > 0) {
		WSA_refCount--;
		WSACleanup(); /* Deinit Socket interface */
	}
}

LPILISTMAN PASCAL CApp::CreateListObj(void)
{
	LPILISTMAN	pList = NULL;
	
	m_pListCf->CreateInstance(NULL,IID_IListMan,(LPVOID *)&pList);
	
	return(pList);
}

LPISCHEDULE PASCAL CApp::CreateSchedObj(void)
{
	LPISCHEDULE	pSched = NULL;
	
	m_pSchedCf->CreateInstance(NULL,IID_ISchedule,(LPVOID *)&pSched);
	
	return(pSched);
}

HWND PASCAL CApp::GetWindow(void)
{
	return(m_hWnd);
}

WSADATA *PASCAL CApp::GetWSAData(void)
{
	return((WSADATA *) &m_wsadata);
}

LPISOCK PASCAL CApp::SockListLookupAsyncOp(HANDLE h)
{
	HANDLE		hnd		= NULL;
	DWORD			dwCount	= 0;
	LPISOCKITEM	pItem 	= NULL;
	DWORD			i;
	
	pItem 	= (LPISOCKITEM) m_pISockList->Head();
	dwCount	= m_pISockList->Count();
	for (i=0;i<dwCount;i++) {
		LPISOCK1	pIS1 = NULL;
		
		pItem->pISock->QueryInterface(IID_ISock1,(void **) &pIS1);
		if(!FAILED(pIS1->GetAsyncHandle(&hnd))) {
			if(h == hnd) {
				pIS1->Release();
				return(pItem->pISock);
			}
		}
		pIS1->Release();
		pItem = (LPISOCKITEM) ILIST_ITEMNEXT(pItem);
	}
	return(NULL);
}

LPISOCK PASCAL CApp::SockListFind(SOCKET s)
{
	DWORD			dwCount	= 0;
	LPISOCKITEM	pItem 	= NULL;
	DWORD			i;
	
	pItem 	= (LPISOCKITEM) m_pISockList->Head();
	dwCount	= m_pISockList->Count();
	for (i=0;i<dwCount;i++) {
		LPISOCK1	pIS1 = NULL;
		
		pItem->pISock->QueryInterface(IID_ISock1,(void **) &pIS1);
		if(s == pIS1->GetSocket()) {
			pIS1->Release();
		  	return(pItem->pISock);
		}
		
		pIS1->Release();
		pItem = (LPISOCKITEM) ILIST_ITEMNEXT(pItem);
	}
	return(NULL);
}


LPISOCKITEM PASCAL CApp::SockListLookupISock(LPISOCK pISock)
{
	DWORD			dwCount	= 0;
	LPISOCKITEM	pItem 	= NULL;
	DWORD			i;
	
	pItem 	= (LPISOCKITEM) m_pISockList->Head();
	dwCount	= m_pISockList->Count();
	for (i=0;i<dwCount;i++) {
		if(pISock == pItem->pISock) {
			return(pItem);
		}
		pItem = (LPISOCKITEM) ILIST_ITEMNEXT(pItem);
	}
	return(NULL);
}

BOOL PASCAL CApp::SockListAdd(LPISOCK pISock)
{
	LPISOCKITEM	pItem = DEBUG_NEW_NOTHROW ISOCKITEM();
	
	if(pItem) {
		pItem->pISock = pISock;
		m_pISockList->Attach(pItem);
		return(TRUE);
	}
	return(FALSE);
}

void PASCAL CApp::SockListRemove(LPISOCK pISock)
{
	LPISOCKITEM pItem = NULL;
	
	if(NULL != (pItem = SockListLookupISock(pISock))) {
		m_pISockList->DeleteItem(pItem);
	}
}

BOOL PASCAL CApp::ScheduleNow(LPISCHEDULE pSched,CONTINUATION pCB,LPVOID pCtx,BOOL /* bAppend */)
{
	LPSCHEDITEM pSi = DEBUG_NEW_NOTHROW SCHEDITEM();
	
	if(pSi) {
		pSi->pSched = pSched;
		pSi->pCB 	= pCB;
		pSi->pCtx   = pCtx;
		
		//if(TRUE == bAppend) {
			m_pContinueList->Attach(pSi);
		//}
		/* Note change this to PREPEND the item once we get this working...		*/
	   //else {
		//	m_pContinueList->Attach(pSi);
		//}
		
      /* Make sure we have a message waiting for us in our message queue		*/
		QueueMessage();
	}
	return(FALSE);
}

BOOL PASCAL CApp::ScheduleLater(LPISCHEDULE pSched,CONTINUATION pCB,LPVOID pCtx,DWORD dwDelay)
{
    LPTIMERITEM pTi = DEBUG_NEW_NOTHROW TIMERITEM();
	
	if(pTi) {
		pTi->pSched 	= pSched;
		pTi->pCB 		= pCB;
		pTi->pCtx   	= pCtx;
		pTi->dwPosted  = GetTickCount();
		pTi->dwTimeout	= dwDelay;
      pTi->bRemove   = FALSE;
		
		m_pTimerList->Attach(pTi);
		
      /* Make sure we have a message waiting for us in our message queue		*/
		QueueMessage();
	}
	return(FALSE);
}

DWORD PASCAL CApp::ScheduleCount(LPISCHEDULE pSched)
{
	DWORD	dwCount = 0;
	
	if(pSched) {
		DWORD			i;
		DWORD			dwCount;
		LPSCHEDITEM	pTmp;
		LPTIMERITEM pTimerTmp;
	
		/* Count events in the continuation list											*/
		dwCount	= m_pContinueList->Count();
		pTmp		= (LPSCHEDITEM) m_pContinueList->Head();
		for(i=0;i<dwCount;i++) {
			if(pSched == pTmp->pSched)
				dwCount++;
			
			pTmp = (LPSCHEDITEM) ILIST_ITEMNEXT(pTmp);
		}
		
		/* Count events in the timer list													*/
		dwCount	= m_pTimerList->Count();
		pTimerTmp= (LPTIMERITEM) m_pTimerList->Head();
		for(i=0;i<dwCount;i++) {
			if(pSched == pTimerTmp->pSched)
				dwCount++;
				
			pTimerTmp = (LPTIMERITEM) ILIST_ITEMNEXT(pTimerTmp);
		}
	}
	return(dwCount);
}

void PASCAL CApp::CancelIScheduleEvents(LPISCHEDULE pSched)
{
	DWORD			i;
	DWORD			dwCount;
	LPTIMERITEM pTimerTmp;

   /* Remove matching LPISCHEDULEs from the timer list                        */
CApp_CIETIMERSTART:
	dwCount	= m_pTimerList->Count();
	pTimerTmp= (LPTIMERITEM) m_pTimerList->Head();
	for(i=0;i<dwCount;i++) {
		if(pSched == pTimerTmp->pSched) {
			m_pTimerList->DeleteItem(pTimerTmp);
			goto CApp_CIETIMERSTART;
		}
		pTimerTmp = (LPTIMERITEM) ILIST_ITEMNEXT(pTimerTmp);
	}

   /* Now walk through the currently executing timer list and remove matching */
CAPP_EXECUTEREMOVE:
	dwCount	= m_pTmpTimerList->Count();
	pTimerTmp= (LPTIMERITEM) m_pTmpTimerList->Head();
	for(i=0;i<dwCount;i++) {
		if(pSched == pTimerTmp->pSched) {
			m_pTmpTimerList->DeleteItem(pTimerTmp);
			goto CAPP_EXECUTEREMOVE;
		}
		pTimerTmp = (LPTIMERITEM) ILIST_ITEMNEXT(pTimerTmp);
	}
}

void PASCAL CApp::QueueMessage(void)
{
	DWORD	dwContinueCount= 0;
	DWORD dwTimerCount 	= 0;
	
	/* Kill any active timer 																	*/
	if(m_hTimer) {
		KillTimer(m_hWnd,m_hTimer);
	}
	
	dwContinueCount 	= m_pContinueList->Count();
	dwTimerCount 		= m_pTimerList->Count();
	
	/* If there are continuations to dispatch												*/
	if(dwContinueCount) {
		m_hTimer = SetTimer(m_hWnd,1,0,NULL);
	}
	/* There are only timers to process														*/
	else if(dwTimerCount) {
		UINT	 		dwTimeout;
		DWORD			i;
		LPTIMERITEM	pTi = NULL;
		
		pTi 			= (LPTIMERITEM) m_pTimerList->Head();
		dwTimeout 	= pTi->dwTimeout;
		for(i=0;i<dwTimerCount;i++) {
			dwTimeout = (UINT) min(dwTimeout,pTi->dwTimeout);
			pTi = (LPTIMERITEM) ILIST_ITEMNEXT(pTi);
		}
		
		m_hTimer = SetTimer(m_hWnd,1,dwTimeout,NULL);
	}
}

void PASCAL CApp::DispatchSocketEvent(WPARAM wParam,LPARAM lParam)
{
	SOCKET			s			= (SOCKET) wParam;/* Socket the event occurred on			*/
	int				nEvent	= LOWORD(lParam);	/* The event that occurred (FD_READ...)*/
	LPISOCK 			pISock 	= NULL;				/* Our socket object							*/
	LPISOCK1			pIS1		= NULL;
	
	if(NULL != (pISock = SockListFind(s))) {
		int			nTmp;
		
		pISock->QueryInterface(IID_ISock1,(void **) &pIS1);
		if(nEvent & FD_READ || nEvent & FD_WRITE) {
			/* Turn off the event until they request it again								*/
			pIS1->GetSocketEvents(&nTmp);
			if(nEvent & FD_READ)		nTmp &= ~FD_READ;
			if(nEvent & FD_WRITE)   nTmp &= ~FD_WRITE;
			pIS1->SetSocketEvents(nTmp);
			WSAAsyncSelect(s,GetWindow(),WM_SOCKETEVENT,nTmp);
			pIS1->SetSocketError(HIWORD(lParam));
			pIS1->DispatchNext();
		}
		else {
			//if(nEvent & FD_CLOSE) ::MessageBox(NULL,"FD_CLOSE Received","Is this Mark's trouble???",MB_OK);
			pIS1->SetSocketError(HIWORD(lParam));
			pIS1->DispatchNext();
		}
		pIS1->Release();
	}
}

void PASCAL CApp::DispatchSocketMessage(WPARAM wParam,LPARAM lParam)
{
	HANDLE			h			= (HANDLE) wParam;
	LPISOCK 			pISock 	= NULL;
	LPISOCK1			pIS1		= NULL;
	
	if(NULL != (pISock = SockListLookupAsyncOp(h))) {
		pISock->QueryInterface(IID_ISock1,(void **) &pIS1);
		pIS1->SetSocketError(HIWORD(lParam));
		pIS1->DispatchNext();
		pIS1->Release();
	}
}

void CApp::QueueTimer(LPTIMERITEM pTmp)
{
	LPTIMERITEM pTi = (LPTIMERITEM) m_pTimerList->Unlink(pTmp);
	
	if(pTi) m_pTmpTimerList->Attach(pTi);
}

void CApp::RunTimers(void)
{
	LPTIMERITEM pTi = NULL;
	
	while(NULL != (pTi = (LPTIMERITEM) m_pTmpTimerList->Pop())) {
		if(pTi->pCB) pTi->pCB(pTi->pCtx);
		ITimerFreeCB((LPITEM) pTi,NULL);
	}
}

void PASCAL CApp::DispatchContinuation(WPARAM /* wParam */,LPARAM /* lParam */)
{
	DWORD	dwTimers = 0;
	DWORD	dwTime	= GetTickCount();
	
	/* Process the timerlist																	*/
	if(0 != (dwTimers = m_pTimerList->Count())) {
		LPTIMERITEM	pTi = NULL;
		DWORD			i;
		
		/* Dispatch all ready timers															*/
DC_CALLTIMER:		
		dwTimers = m_pTimerList->Count();
		pTi 		= (LPTIMERITEM) m_pTimerList->Head();
		for(i=0;i<dwTimers;i++) {
			/* If it is less than or equal to the current time, dispatch it	  	*/
			if((pTi->dwPosted + pTi->dwTimeout) <= dwTime) {
				QueueTimer(pTi);				/* Remove ready timer from list			*/
				goto DC_CALLTIMER;
			}
			pTi = (LPTIMERITEM) ILIST_ITEMNEXT(pTi);
		}
		RunTimers();					      /* Runs prev removed ready timers		*/
	}
	
	/* If there are more, post another event												*/
	//if(0 != m_pContinueList->Count() || 0 != m_pTimerList->Count())
	if(0 != m_pTimerList->Count())
		QueueMessage();
}

//void PASCAL CApp::DispatchContinuation(WPARAM wParam,LPARAM lParam)
//{
//	DWORD	dwTimers = 0;
//	DWORD	dwCont 	= 0;
//	DWORD	dwTime	= GetTickCount();
//	
//	/* Process the timerlist																	*/
//	if(0 != (dwTimers = m_pTimerList->Count())) {
//		register    LPTIMERITEM	pTi = NULL;
//		register    DWORD			i;
//		
//		/* Dispatch all ready timers															*/
//		dwTimers = m_pTimerList->Count();
//		pTi 		= (LPTIMERITEM) m_pTimerList->Head();
//		for(i=0;i<dwTimers;i++) {
//			/* If it is less than or equal to the current time, dispatch it	  	*/
//			if((pTi->dwPosted + pTi->dwTimeout) <= dwTime) {
//            if(pTi->pCB) pTi->pCB(pTi->pCtx); /* Run ready timer              */
//            pTi->bRemove = TRUE;       /* Mark it for removal from the list   */
//			}
//			pTi = (LPTIMERITEM) ILIST_ITEMNEXT(pTi);
//		}
//
//      /* Remove the timers we ran above                                       */
//DC_STARTOVER:
//		dwTimers = m_pTimerList->Count();
//		pTi 		= (LPTIMERITEM) m_pTimerList->Head();
//		for(i=0;i<dwTimers;i++) {
//         if(TRUE == pTi->bRemove) {
//            m_pTimerList->DeleteItem(pTi);
//            goto DC_STARTOVER;
//         }
//			pTi = (LPTIMERITEM) ILIST_ITEMNEXT(pTi);
//		}
//	}
//	
//	/* If there are more, post another event												*/
//	if(0 != m_pTimerList->Count())
//		QueueMessage();
//}

void CApp::ISockListFreeCB(LPITEM pItem,LPVOID /* pUser */)
{
	if(pItem) {
		LPISOCKITEM		pISockItem = reinterpret_cast<LPISOCKITEM>(pItem);

		delete pISockItem;
	}
}

void CApp::IContinueListFreeCB(LPITEM pItem,LPVOID /* pUser */)
{
	if(pItem) {
		LPSCHEDITEM		pSI = reinterpret_cast<LPSCHEDITEM>(pItem);

		delete pSI;
	}
}

void CApp::ITimerFreeCB(LPITEM pItem,LPVOID /* pUser */)
{
   LPTIMERITEM pI = (LPTIMERITEM) pItem;
	
   if(pI) {
      pI->pCB  = NULL;
      pI->pCtx = NULL;

	  delete pI;
   }
}

/*
I'm not a huge fan of commenting out code, but in this case I'm really not sure
whether or not this code is necessary.

void CApp::IFailedPostFreeCB(LPITEM pItem,LPVOID pUser)
{
	if(pItem)
//		free(pItem);
	    FreeBlockMT(pItem);
}
*/

LRESULT CALLBACK CApp::WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg) {
		case WM_SOCKETMSG: {
			//DebugBreak();
         DBGPOKE("WM_SOCKETMSG");
			gApp.DispatchSocketMessage(wParam,lParam);
			break;
		}
		case WM_SOCKETEVENT: {
			//DebugBreak();
         DBGPOKE("WM_SOCKETEVENT");
			gApp.DispatchSocketEvent(wParam,lParam);
			break;
		}
		case WM_TIMER: {
         DBGPOKE("WM_TIMER");
			
         //DebugBreak();
			if(gApp.m_hTimer) {
            KillTimer(hwnd,gApp.m_hTimer);
            gApp.m_hTimer 		= 0;
         }

			if(TRUE == gApp.m_bMsgWaiting) 	gApp.m_bMsgWaiting= FALSE;
				
			gApp.DispatchContinuation(wParam,lParam);
			break;
		}
		default: {
			return(DefWindowProc(hwnd,uMsg,wParam,lParam));
		}
	}
	return(0);
}



