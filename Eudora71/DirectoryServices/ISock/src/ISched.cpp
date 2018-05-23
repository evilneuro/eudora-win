/******************************************************************************/
/*																										*/
/*	Name		:	ISched.cpp																		*/
/* Date     :  4/24/1997                                                      */
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

/*	Desc.		:	IShedule methods																*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514)
#include <afx.h>
#include <ole2.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "dllmain.h"


/* Global Data																						*/
extern class CApp gApp;
extern long g_cComponents;

/* Static */
IUnknown * ISched::CreateInstance()
{
	ISched	*pSched = NULL;
	
	if(FALSE == gApp.IsInitialized())
		if(FALSE == gApp.Initialize(CFactory::s_hModule))
			return NULL;
		
	if(NULL == (pSched = DEBUG_NEW_NOTHROW ISched))
		return NULL;
	
	return static_cast<IUnknown *>(pSched);
}

/******************************************************************************/
/* IUnknown methods																				*/
/******************************************************************************/
STDMETHODIMP ISched::QueryInterface(REFIID riid, void **ppv)
{
	if (TRUE == IsEqualIID(riid,IID_IUnknown)) {
		*ppv = static_cast<IUnknown *>(this);
	}
	else if(TRUE == IsEqualIID(riid,IID_ISchedule)) {
		*ppv = static_cast<LPISCHEDULE>(this);
	}
	else {
      *ppv = 0;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return(ResultFromScode(S_OK));
}

STDMETHODIMP_(ULONG) ISched::AddRef(void)
{
	return InterlockedIncrement(&m_nRefs);	
}

STDMETHODIMP_(ULONG) ISched::Release(void)
{
	if (0 == InterlockedDecrement(&m_nRefs)) {
		delete this;
		return 0;
	}
	return m_nRefs;
}

/******************************************************************************/
/* ISchedule methods			  																	*/
/******************************************************************************/

ISched::ISched(void) : m_nRefs(1)
{
	InterlockedIncrement(&g_cComponents);
}

ISched::~ISched(void)
{
	Cancel();
	InterlockedDecrement(&g_cComponents);
}

BOOL ISched::Schedule(CONTINUATION pCB, LPVOID pCtx, DWORD dwDelay, ISCHED_PROPS /* ipWhen */)
{
	if(!pCB)
		return(FALSE);
	
	gApp.ScheduleLater(this,pCB,pCtx,dwDelay);
	
	return(TRUE);
}

DWORD ISched::Count(void)
{
	return(gApp.ScheduleCount(this));
}

void ISched::Cancel(void)
{
	gApp.CancelIScheduleEvents(this);	
}
