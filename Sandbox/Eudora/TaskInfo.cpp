// TaskInfo.cpp : implementation file for CTaskInfoMT
//

#include "stdafx.h"
#include "QCUtils.h"
#include "EudoraMsgs.h"
#include "QCTaskManager.h" // Must be before the DMM #ifdef

#include "QCWorkerThreadMT.h"
#include "TaskObject.h"
#include "TaskErrorMT.h"

#include "TaskInfo.h"

// Debug Memory Manager block
// This should be after all system includes and before any user includes
#ifdef _DEBUG 
	#undef THIS_FILE 
	static char THIS_FILE[] = __FILE__; 
	#define new DEBUG_NEW 
#endif // _DEBUG


// --------------------------------------------------------------------------

CTaskInfoMT::CTaskInfoMT()
	: CTaskObjectMT(),
		m_MainState(TSS_CREATED), m_PostState(TSS_POST_NONE), m_Total(-1), m_SoFar(-1),
		m_CurPct(-1), m_LastPct(-1), m_pPostFn(NULL), m_pArgument(NULL), m_bNeedsPost(false),
		m_nErrorCount(0), m_nGroupID(0), m_bIgnoreIdle(false), m_nScheduler(0)
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
}

// --------------------------------------------------------------------------

// [PROTECTED] NotifyInfoChanged
//
// Sends a message to the display window indicating this info has changed.
//
void CTaskInfoMT::NotifyInfoChanged()
{
	PostDisplayMsg(msgTaskViewInfoChanged, (WPARAM) this, (LPARAM) GetUID());

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

void CTaskInfoMT::SetPersona(const CString& persona)
{
	m_ObjGuard.Lock();
	if(persona != "")
		m_strPersona = persona;
	else
		m_strPersona = "<Dominant>";
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
	m_ObjGuard.Unlock();
	
	NotifyInfoChanged();
}

// --------------------------------------------------------------------------

void CTaskInfoMT::SetSecondaryText(const char* NewText)
{
	m_ObjGuard.Lock();
	m_SecondaryText = NewText;
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

	CTaskErrorMT *pError = new CTaskErrorMT(this, str, terr);
	QCGetTaskManager()->Register(pError);
}


void CTaskInfoMT::CreateErrorContext(LPCSTR str, TaskErrorType terr, QCError *pErr)
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	m_nErrorCount++;

	CTaskErrorMT *pError = new CTaskErrorMT(this, str, terr);
	if(pError)
		pError->SetErrorObject(pErr);

	QCGetTaskManager()->Register(pError);
}


// --------------------------------------------------------------------------

int CTaskInfoMT::HasErrors()
{
	CSingleLock lock(&m_ObjGuard, TRUE);
	return (m_nErrorCount);
}





