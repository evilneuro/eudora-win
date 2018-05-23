// TaskInfo.cpp : implementation file for CTaskInfoMT
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

//

#include "stdafx.h"

#include "QCUtils.h"
#include "EudoraMsgs.h"
#include "QCTaskManager.h" // Must be before the DMM #ifdef

#include "QCWorkerThreadMT.h"
#include "TaskObject.h"
#include "TaskErrorMT.h"

#include "TaskInfo.h"

#include "DebugNewHelpers.h"


// --------------------------------------------------------------------------

CTaskInfoMT::CTaskInfoMT()
	:	CTaskObjectMT(), m_pWinThread(NULL), m_pThread(NULL),
		m_Total(-1), m_SoFar(-1),
		m_MainState(TSS_CREATED), m_PostState(TSS_POST_NONE),
		m_nGroupID(0), m_bIgnoreIdle(false),
		m_bAllowTaskStatusBringToFront(true), m_bAllowTaskStop(true),
		m_bCountTask(true),
		m_SSL(SSL_NONE), m_iSSLError(0), m_pCertData(NULL),
		m_pPostFn(NULL), m_pArgument(NULL), m_bNeedsPost(false),
		m_nScheduler(0), m_nErrorCount(0), m_CurPct(-1), m_LastPct(-1),
		m_bShouldNotifyInfoChanged(true)
{
}

// --------------------------------------------------------------------------

CTaskInfoMT::~CTaskInfoMT()
{
}

// --------------------------------------------------------------------------

// [PUBLIC] GetObjectType
//
// Pure-virtual base class function. Returns the object type.
//
CTaskObjectMT::TaskObjectType CTaskInfoMT::GetObjectType()
{
	return (CTaskObjectMT::TOBJ_INFO);
}

// --------------------------------------------------------------------------

// [PROTECTED] NotifyAlive
//
// Sends a message to the display window indicating this info exists.
//
void CTaskInfoMT::NotifyAlive()
{
	PostDisplayMsg(msgTaskViewInfoAlive, (WPARAM) this, (LPARAM) GetUID());

	// Make sure that we notify when our info is changed
	SetNotifyInfoChanged();
}

// --------------------------------------------------------------------------

// [PROTECTED] NotifyInfoChanged
//
// Sends a message to the display window indicating this info has changed.
//
void CTaskInfoMT::NotifyInfoChanged()
{
	CSingleLock		lock(&m_csShouldNotifyInfoChangedGuard, TRUE);
	
	if (m_bShouldNotifyInfoChanged)
	{
		// If we successfully post the msgTaskViewInfoChanged, then set
		// that we shouldn't notify when changes occur until CTaskStatusView
		// handles the message and calls SetNotifyInfoChanged. Avoids flooding
		// event queue with useless redundant msgTaskViewInfoChanged messages,
		// which slowed down the performance of indexed search indexing.
		if ( PostDisplayMsg(msgTaskViewInfoChanged, (WPARAM) this, (LPARAM) GetUID()) )
			m_bShouldNotifyInfoChanged = false;
	}
}

// --------------------------------------------------------------------------

// [PROTECTED] NotifyDead
//
// Sends a message to the display window indicating this info is about to die.
//
void CTaskInfoMT::NotifyDead()
{
	PostDisplayMsg(msgTaskViewInfoDead, (WPARAM) this, (LPARAM) GetUID(), true);
}

// --------------------------------------------------------------------------

CString	CTaskInfoMT::GetMainText()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_MainText;
}

// --------------------------------------------------------------------------

CString	CTaskInfoMT::GetSecondaryText()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_SecondaryText;
}

// --------------------------------------------------------------------------

int CTaskInfoMT::GetTotal()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_Total;
}

// --------------------------------------------------------------------------

int CTaskInfoMT::GetSoFar()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_SoFar;
}

// --------------------------------------------------------------------------

// [PUBLIC] GetPercentDone
//
// Return the current percent done (0...100), -1 if unknown
//
int CTaskInfoMT::GetPercentDone() // Returns 0...100, -1 if unknown
{
	CSingleLock lock(&m_PercentGuard, TRUE);
	return (m_CurPct);
}

// --------------------------------------------------------------------------

PostProcState CTaskInfoMT::GetPostState()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_PostState;
}

// --------------------------------------------------------------------------

TaskStatusState CTaskInfoMT::GetState()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_MainState;
}

// --------------------------------------------------------------------------

CString	CTaskInfoMT::GetTitle()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return m_Title;
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetPersona(const char* persona)
{
	m_ObjGuard.Lock();
	if (persona && *persona)
		m_strPersona = persona;
	else
		m_strPersona.LoadString(IDS_DOMINANT);
	m_ObjGuard.Unlock();
	
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetTaskType(TaskType type)
{
	m_ObjGuard.Lock();
	m_TaskType = type;
	m_ObjGuard.Unlock();
	
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetMainText(const char* NewText)
{
	m_ObjGuard.Lock();
	m_MainText = NewText;
	TrimWhitespaceMT( m_MainText );
	if(m_SSL == SSL_NORMAL)
		m_MainText = "SSL: " + m_MainText;
	else if(m_SSL == SSL_FAIL)
		m_MainText = "SSL(Failed):" + m_MainText;

	m_ObjGuard.Unlock();
	
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetSecondaryText(const char* NewText)
{
	m_ObjGuard.Lock();
	m_SecondaryText = NewText;
	TrimWhitespaceMT( m_SecondaryText );
	m_ObjGuard.Unlock();
	
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

// [PUBLIC] Kill
//
// A quick death. But you only die once!
//
void CTaskInfoMT::Kill()
{
	CTaskObjectMT::Kill();
}

// --------------------------------------------------------------------------

// [PROTECTED] CalcPercent
//
// Sets the cached current percent member variable. We assume the caller has
// the lock.
//
void CTaskInfoMT::CalcPercent()
{
	int nPct;

	m_ObjGuard.Lock();
	if (m_SoFar < 0)
		nPct = (-1);
	else
		nPct = ((int)((((double)m_SoFar / m_Total) * 100.0) + 0.5));
	m_ObjGuard.Unlock();

	m_PercentGuard.Lock();
	m_CurPct = nPct;
	m_PercentGuard.Unlock();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetTotal(int NewTotal)
{
	InterlockedExchange((LONG*)&m_Total, NewTotal);
	SetSoFar(0); // This will notify of change
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetSoFar(int NewSoFar)
{
	InterlockedExchange((LONG*)&m_SoFar, NewSoFar);

	CalcPercent();

	m_PercentGuard.Lock();
	bool bChanged = (m_CurPct != m_LastPct);
	m_PercentGuard.Unlock();

	if (bChanged)
	{
		m_PercentGuard.Lock();
		m_LastPct = m_CurPct;
		m_PercentGuard.Unlock();
		NotifyInfoChanged();
	}
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetPostState(PostProcState NewState)
{
	InterlockedExchange((LONG*)&m_PostState, NewState);
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetState(TaskStatusState NewState)
{
	InterlockedExchange((LONG*)&m_MainState, NewState);

	// Only some states allow progress
	switch (NewState)
	{
		case TSS_RUNNING:
		{
			// These states allow progress, so do nothing
		}
		break;

		default:
		{
			m_PercentGuard.Lock();
			m_LastPct = m_CurPct = (-1); // No progress
			m_PercentGuard.Unlock();
		}
		break;
	}

	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetTitle(const char* NewText)
{
	m_ObjGuard.Lock();
	m_Title = NewText;
	m_ObjGuard.Unlock();
	
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetNotifyInfoChanged()
{
	// Set that we should notify when info changes
	CSingleLock		lock(&m_csShouldNotifyInfoChangedGuard, TRUE);
	
	m_bShouldNotifyInfoChanged = true;
}

// --------------------------------------------------------------------------

void CTaskInfoMT::Progress(const char* NewText)
{
	SetSecondaryText(NewText);
}

// --------------------------------------------------------------------------

void CTaskInfoMT::ProgressAdd(int Increment)
{
	SetSoFar(GetSoFar() + Increment);
}

// --------------------------------------------------------------------------

void CTaskInfoMT::RequestThreadStop()
{
	//ask the thread to stop itself
	ASSERT(m_pThread);

	if (m_pThread)
		m_pThread->RequestThreadStop();
}

// --------------------------------------------------------------------------

// [PUBLIC] SetPostProcessing
//
// Sets the post-processing boolean
//
void CTaskInfoMT::SetPostProcessing(bool bNeedsPost)
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	m_bNeedsPost = bNeedsPost;
}


// --------------------------------------------------------------------------

// [PUBLIC] SetPostProcFn
//
// Sets the post-processing function and associated data
//
void CTaskInfoMT::SetPostProcFn(LPCSTR sDispStr, TaskInfoPostFnPtr pFn, void *pArg /* = NULL */)
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	m_PostDisplayString = CString(sDispStr);
	m_pPostFn = pFn;
	m_pArgument = pArg;
}

// --------------------------------------------------------------------------

void CTaskInfoMT::GetPostString(CString &str)
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	str = m_PostDisplayString;
}

// --------------------------------------------------------------------------

// [PUBLIC] DoPostProcessing
//
// Calls the post-processing function, if it exists. May want to call
// HasPostFunction() first.
//
void CTaskInfoMT::DoPostProcessing()
{
	// We may want to change the state to 'TSS_POST_RUNNING' ???
	ASSERT( ::IsMainThreadMT());
	
	HWND hFocus = NULL;

	m_ObjGuard.Lock();
	TaskInfoPostFnPtr pFn = m_pPostFn;
	m_bNeedsPost = false; // We are doing it, so it's done
	TaskStatusState status = m_MainState;
	m_ObjGuard.Unlock();

	SetPostState(TSS_POST_RUNNING);
	
	//If task is completed
	if(status == TSS_COMPLETE)
	{
		//Go one last time 
		//Second arguments tells the function that this the last time..
		if (pFn)
		{
			hFocus = ::GetFocus();
			(pFn)(m_pArgument, false);
		}

		//if( !NeedsPostProcessing())
		if(!m_bNeedsPost)
		{
			SetPostState(TSS_POST_COMPLETE); // This may not be true, we still may have more post proc to do

			//we are done; notify the view to remove this task info from the face of Eudora..
			Kill();
		}
	}
	else
	{
		//intermediate processing while background task is stll alive and kicking..
		if (pFn)
		{
			hFocus = ::GetFocus();
			(pFn)(m_pArgument, false);
		}

		SetPostState(TSS_POST_NONE);
	}

	if(hFocus)
		::SetFocus(hFocus);

}

// --------------------------------------------------------------------------

// [PUBLIC] NeedsPostProcessing
//
// Check if this object needs to be post-processed.
//
bool CTaskInfoMT::NeedsPostProcessing()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return (m_bNeedsPost || m_MainState == TSS_COMPLETE);
}

// --------------------------------------------------------------------------

// [PUBLIC] HasPostFunction
//
// Check if this object has a post-processing function. Probably a good idea
// to call this before DoPostProcessing().
//
bool CTaskInfoMT::HasPostFunction()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return (m_pPostFn != NULL);
}

// --------------------------------------------------------------------------

void CTaskInfoMT::CreateError(LPCSTR str, TaskErrorType terr /*=TERR_UNKNOWN*/)
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	m_nErrorCount++;

	CTaskErrorMT *pError = DEBUG_NEW CTaskErrorMT(this, str, terr);
	QCGetTaskManager()->Register(pError);
}

void CTaskInfoMT::IgnoreError(LPCSTR, TaskErrorType)
{
	// Do nothing because we're ignoring the error
}


void CTaskInfoMT::CreateErrorContext(LPCSTR str, TaskErrorType terr, QCError *pErr)
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	m_nErrorCount++;

	CTaskErrorMT *pError = DEBUG_NEW CTaskErrorMT(this, str, terr);
	pError->SetErrorObject(pErr);

	QCGetTaskManager()->Register(pError);
}


// --------------------------------------------------------------------------

int CTaskInfoMT::HasErrors()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return (m_nErrorCount);
}





